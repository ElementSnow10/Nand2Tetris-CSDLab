#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <stdexcept>
#include <unordered_map>

using namespace std;
namespace fs = filesystem;

enum Instr{
    I_ARITH, I_PUSH, I_POP, I_LABEL,
    I_GOTO, I_IF, I_FUNC, I_RETURN,
    I_CALL
};

class Parser{
public:
    Parser(const string& path);
    bool hasmore();
    void advance();
    Instr type() const;
    string arg1() const;
    int arg2() const;

private:
    ifstream fin;
    vector<string> cmds;
    string current;
    int index;
    static const unordered_map<string, Instr> symtable;
};

const unordered_map<string, Instr> Parser::symtable = {
    {"push", I_PUSH}, {"pop", I_POP}, {"label", I_LABEL}, {"goto", I_GOTO},
    {"if-goto", I_IF}, {"function", I_FUNC}, {"call", I_CALL}, {"return", I_RETURN}
};

Parser::Parser(const string& path) : index(-1){
    fin.open(path);
    if (!fin) throw runtime_error("Error: Cannot open file " + path);

    string line;
    while(getline(fin, line)){
        line.erase(remove(line.begin(), line.end(), '\r'), line.end());
        size_t cpos = line.find("//");
        if(cpos != string::npos) line = line.substr(0, cpos);
        line.erase(0, line.find_first_not_of(" \t"));
        if(line.empty()) continue;
        line.erase(line.find_last_not_of(" \t") + 1);
        if(!line.empty()) cmds.push_back(line);
    }
}

bool Parser::hasmore() {
    return index < (int)cmds.size() - 1;
}

void Parser::advance() {
    if (hasmore()) {
        current = cmds[++index];
    }
}

Instr Parser::type() const{
    auto token = current.substr(0, current.find(' '));
    auto it = symtable.find(token);
    if(it != symtable.end()){
        return it->second;
    }
    return I_ARITH;
}
string Parser::arg1() const{
    if (type() == I_ARITH) return current;
    auto p1 = current.find(' ');
    auto p2 = current.find(' ', p1+1);
    return current.substr(p1+1, p2-p1-1);
}
int Parser::arg2() const {
    return stoi(current.substr(current.rfind(' ') + 1));
}

class CodeWriter {
public:
    CodeWriter(const string& outPath);
    void setname(const string& path);
    void bootstrap();
    void writearithmetic(const string& op);
    void writepushpop(Instr t, const string& seg, int idx);
    void writelabel(const string& lbl);
    void writegoto(const string& lbl);
    void writeifgoto(const string& lbl);
    void writefunc(const string& name, int nlcls);
    void writecall(const string& name, int nArgs);
    void writereturn();
    void finish();

private:
    ofstream fout;
    string vmname, currfunc;
    int jmpc, callc;
};

CodeWriter::CodeWriter(const string& outPath) : jmpc(0), callc(0), currfunc("null") {
    fout.open(outPath);
    if (!fout) throw runtime_error("Error: Cannot open output file " + outPath);
}

void CodeWriter::setname(const string& path) {
    vmname = fs::path(path).stem().string();
}

void CodeWriter::bootstrap() {
    fout << "// Bootstrap\n" << "@256\nD=A\n@SP\nM=D\n";
    writecall("Sys.init", 0);
}

void CodeWriter::writearithmetic(const string& op) {
    fout << "// " << op << "\n";
    if (op == "add" || op == "sub" || op == "and" || op == "or"){
        fout << "@SP\nAM=M-1\nD=M\nA=A-1\n";
        if(op == "add") fout << "M=M+D\n";
        else if(op == "sub") fout << "M=M-D\n";
        else if(op == "and") fout << "M=M&D\n";
        else fout << "M=M|D\n";
    } 
    else if(op=="neg" || op=="not"){ 
        fout << "@SP\nA=M-1\n";
        fout << (op=="neg" ? "M=-M\n" : "M=!M\n");
    } 
    else{
        string trueL = "TRUE"+to_string(jmpc);
        string endL = "END"+to_string(jmpc++);
        fout << "@SP\nAM=M-1\nD=M\nA=A-1\nD=M-D\n";
        fout << "@" << trueL << "\n";
        if (op == "eq") fout << "D;JEQ\n";
        else if (op == "gt") fout << "D;JGT\n";
        else fout << "D;JLT\n";
        fout << "@SP\nA=M-1\nM=0\n@" << endL << "\n0;JMP\n";
        fout << "(" << trueL << ")\n@SP\nA=M-1\nM=-1\n(" << endL << ")\n";
    }
}

void CodeWriter::writepushpop(Instr t, const string& seg, int idx) {
    fout << "// " << (t==I_PUSH ? "push " : "pop ") << seg << " " << idx << "\n";

    static const unordered_map<string, string> base = {
        {"local", "LCL"}, {"argument", "ARG"}, {"this", "THIS"}, {"that", "THAT"}
    };

    switch(t){
        case I_PUSH:
            if(seg == "constant"){
                fout << "@" << idx << "\nD=A\n";
            } 
            else {
                if(base.count(seg)) {
                    fout << "@" << base.at(seg) << "\nD=M\n@" << idx << "\nA=D+A\n";
                } 
                else if(seg == "static") {
                    fout << "@" << vmname << "." << idx << "\n";
                } 
                else { 
                    int addr = idx;
                    if(seg=="temp"){
                        addr+=5;
                    }
                    else{
                        addr+=3;
                    } 
                    fout << "@" << addr << "\n";
                }
                fout << "D=M\n";
            }
            fout << "@SP\nA=M\nM=D\n@SP\nM=M+1\n";
            break;

        case I_POP:
            if(base.count(seg)){
                fout << "@" << base.at(seg) << "\nD=M\n@" << idx << "\nD=D+A\n";
            } 
            else if(seg == "static"){
                fout << "@" << vmname << "." << idx << "\nD=A\n";
            } 
            else{
                int addr = idx;
                if(seg=="temp"){
                    addr+=5;
                }
                else{
                    addr+=3;
                }
                fout << "@" << addr << "\nD=A\n";
            }
            fout << "@R13\nM=D\n@SP\nAM=M-1\nD=M\n@R13\nA=M\nM=D\n";
            break;
        default:
            break; 
    }
}

void CodeWriter::writelabel(const string& lbl){
    fout << "// label " << lbl << "\n(" << currfunc << "$" << lbl << ")\n";
}

void CodeWriter::writegoto(const string& lbl){
    fout << "// goto " << lbl << "\n@" << currfunc << "$" << lbl << "\n0;JMP\n";
}

void CodeWriter::writeifgoto(const string& lbl){
    fout << "// if-goto " << lbl << "\n@SP\nAM=M-1\nD=M\n@" << currfunc << "$" << lbl << "\nD;JNE\n";
}

void CodeWriter::writefunc(const string& name, int nlcls){
    fout << "// function " << name << " " << nlcls << "\n(" << name << ")\n";
    currfunc = name;
    for (int i = 0; i < nlcls; i++){
        fout << "@SP\nA=M\nM=0\n@SP\nM=M+1\n";
    }    
}

void CodeWriter::writecall(const string& name, int nArgs){
    string ret = currfunc+"$ret."+to_string(callc++);
    fout << "// call " << name << " " << nArgs << "\n@" << ret << "\nD=A\n@SP\nA=M\nM=D\n@SP\nM=M+1\n";
    for (auto s : {"@LCL", "@ARG", "@THIS", "@THAT"}) {
        fout << s << "\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n";
    }
    fout << "@SP\nD=M\n@5\nD=D-A\n@" << nArgs << "\nD=D-A\n@ARG\nM=D\n";
    fout << "@SP\nD=M\n@LCL\nM=D\n@" << name << "\n0;JMP\n(" << ret << ")\n";
}

void CodeWriter::writereturn(){
    fout << "// return\n@LCL\nD=M\n@R13\nM=D\n@5\nA=D-A\nD=M\n@R14\nM=D\n" << "@SP\nAM=M-1\nD=M\n@ARG\nA=M\nM=D\n@ARG\nD=M+1\n@SP\nM=D\n";
    for(auto s : {"@THAT", "@THIS", "@ARG", "@LCL"}){
        fout << "@R13\nAM=M-1\nD=M\n" << s << "\nM=D\n";
    }
    fout << "@R14\nA=M\n0;JMP\n";
}

void CodeWriter::finish(){ 
    fout.close(); 
}

// Main
int main(int argc, char* argv[]){
    if(argc < 2){
        cerr << "More args\n";
        return 1;
    }

    string input = argv[1];
    vector<string> files;
    string outfile;
    bool flag = false;

    if(fs::is_directory(input)){
        flag = true;
        fs::path dir(input);
        outfile = (dir / dir.filename()).string()+".asm";
        for (auto& e : fs::directory_iterator(input))
            if (e.path().extension() == ".vm") files.push_back(e.path().string());
    } 
    else{
        files.push_back(input);
        outfile = fs::path(input).replace_extension(".asm").string();
    }

    CodeWriter writer(outfile);
    if(flag) writer.bootstrap();

    for(auto& f : files){
        writer.setname(f);
        Parser parser(f);

        while (parser.hasmore()){
            parser.advance();
            switch (parser.type()){
                case I_ARITH: writer.writearithmetic(parser.arg1()); break;
                case I_PUSH:
                case I_POP: writer.writepushpop(parser.type(), parser.arg1(), parser.arg2()); break;
                case I_LABEL: writer.writelabel(parser.arg1()); break;
                case I_GOTO: writer.writegoto(parser.arg1()); break;
                case I_IF: writer.writeifgoto(parser.arg1()); break;
                case I_FUNC: writer.writefunc(parser.arg1(), parser.arg2()); break;
                case I_CALL: writer.writecall(parser.arg1(), parser.arg2()); break;
                case I_RETURN: writer.writereturn(); break;
            }
        }
    }

    writer.finish();
    return 0;
}