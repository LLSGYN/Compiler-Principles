#ifndef _LEXER_H_
#define _LEXER_H_

#include "token.h"

#include <fstream>
#include <iostream>
#include <unordered_map>
#include <string>
#include <cstring>

#define isdig(x) ('0' <= x && x <= '9')
#define isalp(x) (('a' <= x && x <= 'z') || ('A' <= x && x <= 'Z') || (x == '_'))
#define isHex(x) (('0' <= x && x <= '9') || ('a' <= x && x <= 'f') || ('A' <= x && x <= 'F'))
#define isOct(x) ('0' <= x && x <= '7')

namespace lexdef 
{
    constexpr auto BFSZ = 16;
    constexpr auto eof = 0;
    // bool failFlag = 0;

    const std::unordered_map<std::string, Keyword> _keywords = {
        {"auto", KEYWROD_AUTO}, {"break", KEYWORD_BREAK}, {"case", KEYWORD_CASE},
        {"char", KEYWORD_CHAR}, {"const", KEYWORD_CONST}, {"continue", KEYWORD_CONTINUE},
        {"do",KEYWORD_DO}, {"double",KEYWORD_DOUBLE}, {"else", KEYWORD_ELSE},
        {"enum", KEYWORD_ENUM}, {"extern", KEYWORD_EXTERN}, {"float",KEYWORD_FLOAT},
        {"for", KEYWORD_FOR}, {"goto", KEYWORD_GOTO}, {"if", KEYWORD_IF},
        {"inline", KEYWORD_INLINE}, {"int", KEYWORD_INT}, {"long", KEYWORD_LONG},
        {"register", KEYWORD_REGISTER}, {"restrict", KEYWORD_RESTRICT}, {"return", KEYWORD_RETURN},
        {"short", KEYWORD_SHORT}, {"signed", KEYWORD_SIGNED}, {"sizeof", KEYWORD_SIZEOF},
        {"static", KEYWORD_STATIC}, {"struct", KEYWORD_STRUCT}, {"switch", KEYWORD_SWITCH},
        {"typedef", KEYWORD_TYPEDEF}, {"union", KEYWORD_UNION}, {"unsigned", KEYWORD_UNSIGNED},
        {"void", KEYWORD_VOID}, {"volatile", KEYWORD_VOLATILE}, {"while", KEYWORD_WHILE}
    };

    const std::unordered_map<char, Operator> singleCharOperators = {
        {'(', OPERATOR_LP}, {')', OPERATOR_RP}, {'{', OPERATOR_LC}, {'}', OPERATOR_RC},
        {'[', OPERATOR_LB}, {']', OPERATOR_RB}, {':', OPERATOR_COLON}, {';', OPERATOR_SEMICOLON},
        {',', OPERATOR_COMMA}, {'~', OPERATOR_CPL}, {'.', OPERATOR_DOT}, {'?', OPERATOR_QUESMARK}
    };

    const std::unordered_map<std::string, Operator> multiCharOperators = {
        {"&&", OPERATOR_AND}, {"||", OPERATOR_OR}, {"==", OPERATOR_EQ}, {"!=", OPERATOR_NEQ},
        {"<", OPERATOR_LT}, {"<=", OPERATOR_LEQ}, {">", OPERATOR_GT}, {">=", OPERATOR_GEQ},
        {"+", OPERATOR_ADD}, {"-", OPERATOR_SUB}, {"%", OPERATOR_MOD}, {"=", OPERATOR_ASSIGN},
        {"+=", OPERATOR_ADD_ASSIGN}, {"-=", OPERATOR_SUB_ASSIGN}, {"*=", OPERATOR_MUL_ASSIGN},
        {"%=", OPERATOR_MOD_ASSIGN}, {"&=", OPERATOR_BITAND_ASSIGN}, {"|=", OPERATOR_BITOR_ASSIGN}, 
        {"^=", OPERATOR_XOR_ASSIGN}, {"<<=", OPERATOR_LEFI_SHIFT_ASSIGN}, {">>=", OPERATOR_RIGHT_SHIFT_ASSIGN},
        {"&", OPERATOR_BITAND}, {"|", OPERATOR_BITOR}, {"^", OPERATOR_XOR}, {"<<", OPERATOR_LEFT_SHIFT}, 
        {">>", OPERATOR_RIGHT_SHIFT}, {"->", OPERATOR_ARRAY_POP}, {"++", OPERATOR_INC}, {"--", OPERATOR_DEC},
        {"!", OPERATOR_NOT}, {"*", OPERATOR_MUL}, {"/", OPERATOR_DIV}, {"/=", OPERATOR_DIV_ASSIGN}
    };

    // The table stores identifiers, keywords, string literal, and character literals
    class SymbolTable {
    private:
        std::unordered_map<std::string, std::string*> stab;
        // check entry existance
        std::string* findItem(const std::string& item);
    public:
        // add a new item to the symbol table
        std::string* update(const std::string& item);
    };
}

/*
可能增加lexer 的make_xx_token方法，在lexer中对table进行update， 简化程序，*text可能会废弃
*/
class Lexer 
{
private:
    char buf[2 * (lexdef::BFSZ + 1)];
    char *lexemeBegin, *forward, *forward_p;
    char *_begin[2] = {buf, buf + lexdef::BFSZ + 1};
    char *_end[2] = {buf + lexdef::BFSZ, buf + 2 * lexdef::BFSZ + 1};
    std::ifstream file;
    int charCount = 0, tokenCount = 0;
    int rowCount = 1, colCount = 1;
    bool terminateFlag = 0; // 1 marks the end of lexical analysis
    bool saw_c_comment = 0, saw_cpp_comment = 0;

    char nextChar();
    char peekChar() const;
    void getEscapeCharacter(std::string&);
    void retract();
    void fail();
    void skipCComment();
    void skipCppComment();
    bool loadBuf(int id); // fills buffer1 or buffer2

    void gatherDecnum(std::string&);
    void gatherHexnum(std::string&);
    void gatherOctnum(std::string&);
    // void gatherFloat(std::string&);
    // void gatherExponent(std::string&);
    bool parseSuffix(bool);

    Token scanNumber();
    Token scanIdentifier();
    Token scanString();
    Token scanCharacter();
    Token scanOneCharOpr();
    Token scanMultiCharOpr();
    lexdef::SymbolTable __table;

    std::string _filename;
    Token::Location location();
    void error(Token::Location _loc, const std::string& msg);
    void error(Token::Location _loc, const std::string& msg, const std::string& pat);
public:
    Lexer(const std::string& source);
    Token getNextToken();
    int getRowCount() { return this->rowCount; }
    int getCharCount() { return this->charCount; }
    // void print() {
    //     for (int i = 0; i < 17; ++i)
    //         std::cerr << int(buf[i]) << " ";
    //     std::cerr << std::endl;
    //     std::cerr << "-------------" << std::endl;
    //     std::cerr << std::endl;
    //     for (int i = 17; i < 34; ++i)
    //         std::cerr << int(buf[i]) << " ";
    //     std::cerr << std::endl;
    // }
};

#endif