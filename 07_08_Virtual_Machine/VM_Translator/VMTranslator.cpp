#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <map>

using namespace std;
namespace fs = filesystem;

// Instruction types
enum Instr {
    I_ARITH, I_PUSH, I_POP, I_LABEL, I_GOTO,
    I_IF, I_FUNC, I_RETURN, I_CALL
};

// class Parser
class Parser {
private:
    ifstream fin;
    string currcmd;
    vector<string> cmds;
    vector<string> tokens;
    int cmdidx;
    map<string, Instr> type_map;

public:
    Parser(const string& filename);
    bool hasmore();
    void advance();
    Instr type();
    string arg1();
    int arg2();
};

// class CodeWriter
class CodeWriter{
private:
    ofstream fout;
    int jmpc;
    string file_name;

public:
    CodeWriter(const string& filename);
    void writeArithmetic(const string& cmd);
    void writePushPop(Instr cmd, const string& segment, int index);
    void close();
};

CodeWriter::CodeWriter(const string& filename) : jmpc(0) {
    fout.open(filename);
    file_name = fs::path(filename).stem().string();
}

void CodeWriter::writeArithmetic(const string& cmd) {
    if(cmd == "add"){
        fout << "@SP\nAM=M-1\nD=M\nA=A-1\nM=D+M\n";
    } 
    else if(cmd == "sub"){
        fout << "@SP\nAM=M-1\nD=M\nA=A-1\nM=M-D\n";
    } 
    else if(cmd == "neg"){
        fout << "@SP\nA=M-1\nM=-M\n";
    } 
    else if(cmd == "eq" || cmd == "gt" || cmd == "lt"){
        string jmp_type;
        if(cmd == "eq"){
            jmp_type = "JEQ";
        }
        else if(cmd == "gt"){
            jmp_type = "JGT";
        }
        else{
            jmp_type = "JLT";
        }
        fout << "@SP\nAM=M-1\nD=M\nA=A-1\nD=M-D\n" << "@TRUE" << jmpc << "\nD;" << jmp_type << "\n"
            << "@SP\nA=M-1\nM=0\n@END" << jmpc << "\n0;JMP\n" << "(TRUE" << jmpc << ")\n@SP\nA=M-1\nM=-1\n"
            << "(END" << jmpc << ")\n";
        jmpc++;
    } 
    else if(cmd == "and"){
        fout << "@SP\nAM=M-1\nD=M\nA=A-1\nM=D&M\n";
    } 
    else if(cmd == "or"){
        fout << "@SP\nAM=M-1\nD=M\nA=A-1\nM=D|M\n";
    } 
    else if(cmd == "not"){
        fout << "@SP\nA=M-1\nM=!M\n";
    }
}

void CodeWriter::writePushPop(Instr cmd, const string& segment, int index){
    static const map<string, string> seg_map = {
        {"local", "LCL"}, {"argument", "ARG"}, {"this", "THIS"}, {"that", "THAT"}
    };

    if(segment == "constant"){
        fout << "@" << index << "\nD=A\n@SP\nA=M\nM=D\n@SP\nM=M+1\n";
        return;
    }

    if(seg_map.count(segment)){
        const string& segsym = seg_map.at(segment);
        if(cmd == I_PUSH){
            fout << "@" << segsym << "\nD=M\n@" << index << "\nA=D+A\nD=M\n" << "@SP\nA=M\nM=D\n@SP\nM=M+1\n";
        } 
        else{ 
            fout << "@" << segsym << "\nD=M\n@" << index << "\nD=D+A\n" << "@R13\nM=D\n@SP\nAM=M-1\nD=M\n@R13\nA=M\nM=D\n";
        }
        return;
    }
    
    string addr_sym;
    if(segment == "static") addr_sym = file_name+"."+to_string(index);
    else if(segment == "temp") addr_sym = to_string(5+index);
    else if(segment == "pointer") addr_sym = (index == 0 ? "THIS" : "THAT");

    if(!addr_sym.empty()){
        if(cmd == I_PUSH){
            fout << "@" << addr_sym << "\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n";
        } 
        else{ 
            fout << "@SP\nAM=M-1\nD=M\n@" << addr_sym << "\nM=D\n";
        }
    }
}

void CodeWriter::close(){
    fout << "(END)\n@END\n0;JMP\n";
    fout.close();
}

Parser::Parser(const string& filename) : cmdidx(-1){
    type_map = {{"push", I_PUSH}, {"pop", I_POP}, {"label", I_LABEL}, {"goto", I_GOTO}, {"if-goto", I_IF}, {"function", I_FUNC}, {"return", I_RETURN}, {"call", I_CALL}};
    fin.open(filename);
    if(!fin.is_open()){
        cerr << "Error opening file\n";
        exit(1);
    }

    string line;
    while (getline(fin, line)) {
        if (auto comment_pos = line.find("//"); comment_pos != string::npos) {
            line.erase(comment_pos);
        }
        line.erase(line.find_last_not_of(" \t\r") + 1);
        line.erase(0, line.find_first_not_of(" \t\r"));
        if (!line.empty()) {
            cmds.push_back(move(line));
        }
    }
}

bool Parser::hasmore(){ 
    return cmdidx < (int)cmds.size()-1; 
}

void Parser::advance() {
    if(hasmore()){
        currcmd = cmds[++cmdidx];
        tokens.clear();
        string token;
        stringstream ss(currcmd);
        while(ss>>token) tokens.push_back(token);
    }
}

Instr Parser::type() {
    if(type_map.count(tokens[0])) return type_map.at(tokens[0]);
    return I_ARITH;
}

string Parser::arg1(){
    if(type()==I_ARITH){
        return tokens[0];
    }
    else{
        return tokens[1];
    }
}

int Parser::arg2(){ 
    return stoi(tokens[2]); 
}

// Main
int main(int argc, char* argv[]){

    if(argc < 2){
        cerr << "More args\n";
        return 1;
    }

    vector<string> vm_files;
    string outfile;
    string input = argv[1];

    if(fs::is_directory(input)){
        fs::path dir_path(input);
        outfile = (dir_path / dir_path.filename()).string()+".asm";
        for(const auto& entry : fs::directory_iterator(input)){
            if(entry.path().extension() == ".vm") vm_files.push_back(entry.path().string());
        }
    } 
    else{
        vm_files.push_back(input);
        outfile = fs::path(input).replace_extension(".asm").string();
    }

    CodeWriter writer(outfile);

    for(const auto& file : vm_files){
        Parser parser(file);
        while(parser.hasmore()){
            parser.advance();
            Instr currtype = parser.type();
            if(currtype==I_ARITH){
                writer.writeArithmetic(parser.arg1());
            } 
            else if(currtype==I_PUSH || currtype==I_POP){
                writer.writePushPop(currtype, parser.arg1(), parser.arg2());
            }
        }
    }

    writer.close();
    return 0;
}