#include <bits/stdc++.h>
using namespace std;
namespace fs = filesystem;

enum class tokentype {
    KEYWORD,
    SYMBOL,
    IDENTIFIER,
    INTCONST,
    STRINGCONST
};

enum class keyword {
    CLASS, METHOD, FUNCTION, CONSTRUCTOR, INT, BOOLEAN, CHAR, VOID, VAR,
    STATIC, FIELD, LET, DO, IF, ELSE, WHILE, RETURN, TRUE, FALSE, NULL_K, THIS
};

class Tokenizer {
public:

    Tokenizer(const string& infilepath);
    bool hasmore();
    void advance();

    tokentype token();
    keyword key();
    char symbol();
    string identifier();
    int intval();
    string stringval();

    static string keytostring(keyword k);

private:
    struct tokeninfo {
        string value;
        tokentype type;

        tokeninfo(string v = "", tokentype t = tokentype::KEYWORD) : value(v), type(t) {}
    };

    queue<tokeninfo> tokenqueue; 
    tokeninfo currtoken; 

    static set<string> keyset;
    static map<string, keyword> keymap;
    static map<keyword, string> keystringmap;


    static void initkeymap();
    static bool mapinit;
};

set<string> Tokenizer::keyset;
map<string, keyword> Tokenizer::keymap;
map<keyword, string> Tokenizer::keystringmap;
bool Tokenizer::mapinit = false;

void Tokenizer::initkeymap() {
    if(mapinit) return;

    keymap["class"] = keyword::CLASS;
    keymap["constructor"] = keyword::CONSTRUCTOR;
    keymap["function"] = keyword::FUNCTION;
    keymap["method"] = keyword::METHOD;
    keymap["field"] = keyword::FIELD;
    keymap["static"] = keyword::STATIC;
    keymap["var"] = keyword::VAR;
    keymap["int"] = keyword::INT;
    keymap["char"] = keyword::CHAR;
    keymap["boolean"] = keyword::BOOLEAN;
    keymap["void"] = keyword::VOID;
    keymap["true"] = keyword::TRUE;
    keymap["false"] = keyword::FALSE;
    keymap["null"] = keyword::NULL_K;
    keymap["this"] = keyword::THIS;
    keymap["let"] = keyword::LET;
    keymap["do"] = keyword::DO;
    keymap["if"] = keyword::IF;
    keymap["else"] = keyword::ELSE;
    keymap["while"] = keyword::WHILE;
    keymap["return"] = keyword::RETURN;

    for(const auto& pair : keymap) {
        keyset.insert(pair.first);
        keystringmap[pair.second] = pair.first;
    }
    
    mapinit = true;
}

Tokenizer::Tokenizer(const string& infilepath) {

    initkeymap();

    ifstream filein(infilepath);
    stringstream buffer;
    buffer << filein.rdbuf();
    string cont = buffer.str();
    filein.close();

    set<char> symbols = {'{', '}', '(', ')', '[', ']', '.', ',', ';', '+', '-', '*', '/', '&', '|', '<', '>', '=', '~'};
    size_t i = 0;
    while(i < cont.length()){
        char c = cont[i];

        if(c == ' ') {
            i++;
            continue;
        }

        if(c == '/') {
            if(i+1 < cont.length() && cont[i+1] == '/') {
                i+=2; 
                while(i < cont.length() && cont[i]!='\n') {
                    i++;
                }
                continue; 
            } 
            else if(i+1 < cont.length() && cont[i+1] == '*') {
                i+=2; 
                while(i+1 < cont.length() && (cont[i] != '*' || cont[i+1] != '/')) {
                    i++;
                }
                if(i+1 < cont.length()) {
                    i+=2; 
                } 
                else {
                    i = cont.length();
                }
                continue;
            }
        }

        if(symbols.count(c)) {
            tokenqueue.push(tokeninfo(string(1, c), tokentype::SYMBOL));
            i++;
            continue;
        }

        if(c == '"') {
            string tokenval;
            i++; 
            while(i < cont.length() && cont[i] != '"') {
                tokenval += cont[i];
                i++;
            }
            i++;
            tokenqueue.push(tokeninfo(tokenval, tokentype::STRINGCONST));
            continue;
        }

        if(isdigit(c)) {
            string tokenval;
            while (i < cont.length() && isdigit(cont[i])) {
                tokenval += cont[i];
                i++;
            }
            tokenqueue.push(tokeninfo(tokenval, tokentype::INTCONST));
            continue;
        }

        if(isalpha(c) || c == '_') {
            string tokenval;
            while(i < cont.length() && (isalnum(cont[i]) || cont[i] == '_')) {
                tokenval+=cont[i];
                i++;
            }
    
            if(keyset.count(tokenval)){
                tokenqueue.push(tokeninfo(tokenval, tokentype::KEYWORD));
            } 
            else{
                tokenqueue.push(tokeninfo(tokenval, tokentype::IDENTIFIER));
            }
            continue;
        }
        i++;
    }
    
    currtoken = tokeninfo();
}

bool Tokenizer::hasmore() {
    return !tokenqueue.empty();
}

void Tokenizer::advance() {
    if(hasmore()) {
        currtoken = tokenqueue.front();
        tokenqueue.pop();
    }
}

tokentype Tokenizer::token() {
    return currtoken.type;
}

keyword Tokenizer::key() {
    return keymap.at(currtoken.value);
}

char Tokenizer::symbol() {
    return currtoken.value[0];
}

string Tokenizer::identifier() {
    return currtoken.value;
}

int Tokenizer::intval() {
    return stoi(currtoken.value);
}

string Tokenizer::stringval() {
    return currtoken.value;
}

string Tokenizer::keytostring(keyword k) {
    if(!mapinit) initkeymap();
    
    if(keystringmap.count(k)) {
        return keystringmap.at(k);
    }
    return "";
}

string excapexml(const string& s);
string excapexml(char c);

class Parser {
private:
    ofstream outfile;
    Tokenizer tokenizer;
    set<char> opset;
    string indentstr;

    void currtoken() {
        if(tokenizer.token() == tokentype::KEYWORD) {
            outfile << indentstr << "<keyword> " << Tokenizer::keytostring(tokenizer.key()) << " </keyword>" << endl;
        }
        else if(tokenizer.token() == tokentype::SYMBOL) {
            outfile << indentstr << "<symbol> " << excapexml(tokenizer.symbol()) << " </symbol>" << endl;
        }
        else if(tokenizer.token() == tokentype::IDENTIFIER) {
            outfile << indentstr << "<identifier> " << tokenizer.identifier() << " </identifier>" << endl;
        }
        else if(tokenizer.token() == tokentype::INTCONST) {
            outfile << indentstr << "<integerConstant> " << tokenizer.intval() << " </integerConstant>" << endl;
        }
        else if(tokenizer.token() == tokentype::STRINGCONST) {
            outfile << indentstr << "<stringConstant> " << excapexml(tokenizer.stringval()) << " </stringConstant>" << endl;
        }
        tokenizer.advance();
    }

    void nonterminal(const string& tag) {
        outfile << indentstr << "<" << tag << ">" << endl;
        indentstr += "  ";
    }

    void nonterminalend(const string& tag) {
        indentstr.resize(indentstr.length()-2);
        outfile << indentstr << "</" << tag << ">" << endl;
    }

public:
    Parser(const string& infilepath, const string& outfilepath) : tokenizer(infilepath), outfile(outfilepath), indentstr("") {  
        opset = { '+', '-', '*', '/', '&', '|', '<', '>', '=' };
        tokenizer.advance();
    }

    ~Parser() {
        if (outfile.is_open()) {
            outfile.close();
        }
    }

    void compileclass() {
        nonterminal("class");
        currtoken(); 
        currtoken(); 
        currtoken(); 

        while(tokenizer.token() == tokentype::KEYWORD && (tokenizer.key() == keyword::STATIC || tokenizer.key() == keyword::FIELD)) {
            compileclassvardec();
        }

        while(tokenizer.token() == tokentype::KEYWORD && (tokenizer.key() == keyword::CONSTRUCTOR || tokenizer.key() == keyword::FUNCTION || tokenizer.key() == keyword::METHOD)) {
            compilesubroutine();
        }

        currtoken(); 
        nonterminalend("class");
    }

    void compileclassvardec() {
        nonterminal("classVarDec");
        currtoken(); 
        currtoken(); 
        currtoken(); 

        while(tokenizer.token() == tokentype::SYMBOL && tokenizer.symbol() == ',') {
            currtoken(); 
            currtoken(); 
        }

        currtoken(); 
        nonterminalend("classVarDec");
    }

    void compilesubroutine() {
        nonterminal("subroutineDec");
        currtoken(); 
        currtoken(); 
        currtoken(); 
        currtoken(); 
        compileparamlist();
        currtoken(); 
        compilesubroutinebody();
        nonterminalend("subroutineDec");
    }

    void compileparamlist() {
        nonterminal("parameterList");
        
        if(tokenizer.token() != tokentype::SYMBOL || tokenizer.symbol() != ')') {
            currtoken(); 
            currtoken(); 

            while(tokenizer.token() == tokentype::SYMBOL && tokenizer.symbol() == ',') {
                currtoken(); 
                currtoken(); 
                currtoken();
            }
        }

        nonterminalend("parameterList");
    }

    void compilesubroutinebody() {
        nonterminal("subroutineBody");
        currtoken(); 
        
        while(tokenizer.token() == tokentype::KEYWORD && tokenizer.key() == keyword::VAR) {
            compilevardec();
        }

        compilestatements();
        currtoken(); 
        nonterminalend("subroutineBody");
    }

    void compilevardec() {
        nonterminal("varDec");
        currtoken(); 
        currtoken(); 
        currtoken(); 

        while(tokenizer.token() == tokentype::SYMBOL && tokenizer.symbol() == ',') {
            currtoken(); 
            currtoken(); 
        }

        currtoken(); 
        nonterminalend("varDec");
    }

    void compilestatements() {
        nonterminal("statements");

        while(tokenizer.token() == tokentype::KEYWORD) {
            if(tokenizer.key() == keyword::LET) {
                compileLet();
            } 
            else if(tokenizer.key() == keyword::IF) {
                compileIf();
            } 
            else if(tokenizer.key() == keyword::WHILE) {
                compileWhile();
            } 
            else if(tokenizer.key() == keyword::DO) {
                compileDo();
            } 
            else if(tokenizer.key() == keyword::RETURN) {
                compileReturn();
            } 
            else {
                break; 
            }
        }

        nonterminalend("statements");
    }

    void compileLet() {
        nonterminal("letStatement");
        currtoken(); 
        currtoken(); 

        if(tokenizer.token() == tokentype::SYMBOL && tokenizer.symbol() == '[') {
            currtoken(); 
            compileexpr();
            currtoken(); 
        }

        currtoken(); 
        compileexpr();
        currtoken(); 
        nonterminalend("letStatement");
    }

    void compileIf() {
        nonterminal("ifStatement");
        currtoken(); 
        currtoken(); 
        compileexpr();
        currtoken();
        currtoken();
        compilestatements();
        currtoken(); 

        if(tokenizer.token() == tokentype::KEYWORD && tokenizer.key() == keyword::ELSE) {
            currtoken(); 
            currtoken(); 
            compilestatements();
            currtoken();
        }

        nonterminalend("ifStatement");
    }

    void compileWhile() {
        nonterminal("whileStatement");
        currtoken(); 
        currtoken(); 
        compileexpr();
        currtoken(); 
        currtoken();
        compilestatements();
        currtoken(); 
        nonterminalend("whileStatement");
    }

    void compileDo() {
        nonterminal("doStatement");
        currtoken();
        
        currtoken(); 
        compilesubroutinecall(); 

        currtoken(); 
        nonterminalend("doStatement");
    }

    void compileReturn() {
        nonterminal("returnStatement");
        currtoken();

        if(tokenizer.token() != tokentype::SYMBOL || tokenizer.symbol() != ';') {
            compileexpr();
        }

        currtoken(); 
        nonterminalend("returnStatement");
    }

    void compileexpr() {
        nonterminal("expression");
        compileterm();

        while(tokenizer.token() == tokentype::SYMBOL && opset.count(tokenizer.symbol())) {
            currtoken();
            compileterm();
        }

        nonterminalend("expression");
    }

    void compileterm() {
        nonterminal("term");

        tokentype t = tokenizer.token();

        if(t == tokentype::INTCONST || t == tokentype::STRINGCONST || (t == tokentype::KEYWORD && (tokenizer.key() == keyword::TRUE || tokenizer.key() == keyword::FALSE || tokenizer.key() == keyword::NULL_K || tokenizer.key() == keyword::THIS))) {
            currtoken();
        } 
        else if(t == tokentype::SYMBOL && tokenizer.symbol() == '(') {
            currtoken(); 
            compileexpr();
            currtoken(); 
        } 
        else if(t == tokentype::SYMBOL && (tokenizer.symbol() == '-' || tokenizer.symbol() == '~')) {
            currtoken(); 
            compileterm();
        } 
        else if(t == tokentype::IDENTIFIER) {
            string id = tokenizer.identifier();
            tokenizer.advance(); 

            if (tokenizer.token() == tokentype::SYMBOL && tokenizer.symbol() == '[') {
                outfile << indentstr << "<identifier> " << id << " </identifier>" << endl;
                currtoken(); 
                compileexpr();
                currtoken(); 
            } 
            else if (tokenizer.token() == tokentype::SYMBOL && (tokenizer.symbol() == '(' || tokenizer.symbol() == '.')) {
                outfile << indentstr << "<identifier> " << id << " </identifier>" << endl;
                if(tokenizer.symbol() == '(') {
                    currtoken(); 
                    compileexprlist();
                    currtoken();
                } 
                else {
                    currtoken(); 
                    currtoken(); 
                    currtoken();
                    compileexprlist();
                    currtoken();
                }
            } 
            else {
                outfile << indentstr << "<identifier> " << id << " </identifier>" << endl;
            }
        }

        nonterminalend("term");
    }

    void compilesubroutinecall() {
        if (tokenizer.token() == tokentype::SYMBOL && tokenizer.symbol() == '(') {
            currtoken(); 
            compileexprlist();
            currtoken(); 
        } 
        else {
            currtoken(); 
            currtoken(); 
            currtoken(); 
            compileexprlist();
            currtoken(); 
        }
    }


    void compileexprlist() {
        nonterminal("expressionList");

        if(tokenizer.token() != tokentype::SYMBOL || tokenizer.symbol() != ')') {
            compileexpr();
            while (tokenizer.token() == tokentype::SYMBOL && tokenizer.symbol() == ',') {
                currtoken(); 
                compileexpr();
            }
        }

        nonterminalend("expressionList");
    }
};

string excapexml(const string& s) {
    string esc;
    for(char c : s) {
        if(c == '&') {
            esc+="&amp;";
        }
        else if(c == '<') {
            esc+="&lt;";
        }
        else if(c == '>') {
            esc+="&gt;";
        }
        else if(c == '\"') {
            esc+="&quot;";
        }
        else {
            esc+=c;
        }
    }
    return esc;
}

string excapexml(char c) {
    if(c == '&') {
        return "&amp;";
    }
    else if(c == '<') {
        return "&lt;";
    }
    else if(c == '>') {
        return "&gt;";
    }
    else if(c == '\"') {
        return "&quot;";
    }
    else {
        return string(1, c);
    }
}

void process(const fs::path& jackfile) {
    fs::path outputdirT = jackfile.parent_path();
    string stemT = jackfile.stem().string();
    string outfilenameT = "my" + stemT + "T.xml";
    fs::path outfilepathT = outputdirT/outfilenameT;

    ofstream outfileT(outfilepathT);
    Tokenizer tokenizerT(jackfile.string());
    
    outfileT << "<tokens>" << endl;
    while(tokenizerT.hasmore()) {
        tokenizerT.advance();
        if(tokenizerT.token() == tokentype::KEYWORD) {
            outfileT << "<keyword> " << Tokenizer::keytostring(tokenizerT.key()) << " </keyword>" << endl;
        }
        else if(tokenizerT.token() == tokentype::SYMBOL) {
            outfileT << "<symbol> " << excapexml(tokenizerT.symbol()) << " </symbol>" << endl;
        }
        else if (tokenizerT.token() == tokentype::IDENTIFIER) {
            outfileT << "<identifier> " << tokenizerT.identifier() << " </identifier>" << endl;
        }
        else if (tokenizerT.token() == tokentype::INTCONST) {
            outfileT << "<integerConstant> " << tokenizerT.intval() << " </integerConstant>" << endl;
        }
        else if (tokenizerT.token() == tokentype::STRINGCONST) {
            outfileT << "<stringConstant> " << excapexml(tokenizerT.stringval()) << " </stringConstant>" << endl;
        }

    }
    outfileT << "</tokens>" << endl;
    outfileT.close();

    fs::path outputdir = jackfile.parent_path();
    string stem = jackfile.stem().string();
    string outfilename = "my" + stem + ".xml"; 
    fs::path outfilepath = outputdir/outfilename;

    Parser engine(jackfile.string(), outfilepath.string());
    engine.compileclass(); 
}

int main(int argc, char* argv[]) {
    if(argc != 2) {
        return 1;
    }

    fs::path inpath(argv[1]);
    vector<fs::path> files;

    if(fs::is_directory(inpath)) {
        for(const auto& entry : fs::directory_iterator(inpath)) {
            if (entry.is_regular_file() && entry.path().extension() == ".jack") {
                files.push_back(entry.path());
            }
        }
    } 
    else if(fs::is_regular_file(inpath) && inpath.extension() == ".jack") {
        files.push_back(inpath);
    } 
    else{
        return 1;
    }

    if(files.empty()) {
        return 0;
    }

    for(const auto& jackfile : files) {
        process(jackfile);
    }
    
    return 0;
}
