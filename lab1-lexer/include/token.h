#ifndef _TOKEN_H_
#define _TOKEN_H_

#include <ostream>
#include <string>

using i64 = long long;
using u8 = unsigned char;
using u32 = unsigned;
using u64 = unsigned long long;
using f80 = long double;

enum Keyword
{
    KEYWORD_INVALID,
    KEYWROD_AUTO,
    KEYWORD_BREAK,
    KEYWORD_CASE,
    KEYWORD_CHAR,
    KEYWORD_CONST,
    KEYWORD_CONTINUE,
    KEYWORD_DO,
    KEYWORD_DOUBLE,
    KEYWORD_ELSE,
    KEYWORD_ENUM,
    KEYWORD_EXTERN,
    KEYWORD_FLOAT,
    KEYWORD_FOR,
    KEYWORD_GOTO,
    KEYWORD_IF,
    KEYWORD_INLINE,
    KEYWORD_INT,
    KEYWORD_LONG,
    KEYWORD_REGISTER,
    KEYWORD_RESTRICT,
    KEYWORD_RETURN,
    KEYWORD_SHORT,
    KEYWORD_SIGNED,
    KEYWORD_SIZEOF,
    KEYWORD_STATIC,
    KEYWORD_STRUCT,
    KEYWORD_SWITCH,
    KEYWORD_TYPEDEF,
    KEYWORD_UNION,
    KEYWORD_UNSIGNED,
    KEYWORD_VOID,
    KEYWORD_VOLATILE,
    KEYWORD_WHILE
};

enum Operator
{
    OPERATOR_INVALID, 
    OPERATOR_LP,                 /* ( */ 
    OPERATOR_RP,                 /* ) */ 
    OPERATOR_LC,                 /* { */ 
    OPERATOR_RC,                 /* } */ 
    OPERATOR_LB,                 /* [ */ 
    OPERATOR_RB,                 /* ] */ 
    OPERATOR_COLON,              /* : */ 
    OPERATOR_SEMICOLON,          /* ; */ 
    OPERATOR_COMMA,              /* , */ 
    OPERATOR_AND,                /* && */ 
    OPERATOR_OR,                 /* || */ 
    OPERATOR_EQ,                 /* == */ 
    OPERATOR_NEQ,                /* != */ 
    OPERATOR_LT,                 /* < */ 
    OPERATOR_LEQ,                /* <= */ 
    OPERATOR_GT,                 /* > */ 
    OPERATOR_GEQ,                /* >= */ 
    OPERATOR_ADD,                /* + */ 
    OPERATOR_SUB,                /* - */ 
    OPERATOR_MUL,                /* * */ 
    OPERATOR_DIV,                /* / */ 
    OPERATOR_MOD,                /* % */ 
    OPERATOR_ASSIGN,             /* = */ 
    OPERATOR_ADD_ASSIGN,         /* += */ 
    OPERATOR_SUB_ASSIGN,         /* -= */ 
    OPERATOR_MUL_ASSIGN,         /* *= */
    OPERATOR_DIV_ASSIGN,         /* /= */ 
    OPERATOR_MOD_ASSIGN,         /* %= */ 
    OPERATOR_BITAND_ASSIGN,      /* &= */ 
    OPERATOR_BITOR_ASSIGN,       /* |= */ 
    OPERATOR_XOR_ASSIGN,         /* ^= */ 
    OPERATOR_LEFI_SHIFT_ASSIGN,  /* <<= */ 
    OPERATOR_RIGHT_SHIFT_ASSIGN, /* >>= */ 
    OPERATOR_BITAND,             /* & */ 
    OPERATOR_BITOR,              /* | */ 
    OPERATOR_XOR,                /* ^ */ 
    OPERATOR_LEFT_SHIFT,         /* << */ 
    OPERATOR_RIGHT_SHIFT,        /* >> */ 
    OPERATOR_ARRAY_POP,          /* -> */ 
    OPERATOR_CPL,                /* ~ */ 
    OPERATOR_INC,                /* ++ */ 
    OPERATOR_DEC,                /* -- */ 
    OPERATOR_NOT,                /* ! */ 
    OPERATOR_DOT,                /* . */ 
    OPERATOR_QUESMARK            /* ? */
};

class Token
{
public:
    enum Classification
    {
        TOKEN_INVALID,          
        TOKEN_EOF,
        TOKEN_KEYWORD,
        TOKEN_IDENTIFIER,       /* [a-zA-Z_]+[a-zA-Z_0-9]* */
        TOKEN_OPERATOR, 
        TOKEN_STRING,
        TOKEN_CHARACTER,
        TOKEN_INTEGER,
        TOKEN_FLOAT
    };
    class Location {
    public:
        Location() = default;
        Location(const std::string& _filename, int _row, int _column);
        friend std::ostream& operator << (std::ostream& os, const Location& loc);
    private:
        int row, column;
        std::string filename;
    } loc;
    Token() = default;
    static Token make_integer_token(i64 val, std::string* text);
    static Token make_float_token(double val, std::string* text);
    static Token make_keyword_token(Keyword KeyType, std::string* text);
    static Token make_identifier_token(std::string* text);
    static Token make_operator_token(Operator OpType, std::string* text);
    static Token make_character_token(char val, std::string* text);
    static Token make_string_token(std::string* text);
    static Token make_eof_token();
    static Token make_invalid_token();
    Classification TokenType() const;
    friend std::ostream& operator << (std::ostream& os, const Token& t);
private:
    Token(Classification, Keyword, std::string*);
    Token(Classification, std::string*);
    Token(Classification, i64, std::string*);
    Token(Classification, double, std::string*);
    Token(Classification, Operator, std::string*);
    // Token classification
    Classification classification;
    union token
    {
        // The keyword type value
        Keyword keyword; 
        // The name of the identifer
        std::string* name; 
        // The operator type value
        Operator operator_;
        // The string-const value
        std::string* stringValue;
        // The integer or char-const value
        i64 integerValue;
        // The float value
        double floatValue;
    }u;
    std::string* text;
};
#endif
