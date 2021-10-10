#include "token.h"

std::ostream& operator << (std::ostream& os, const Token& t)
{
    switch (t.classification)
    {
    case Token::TOKEN_KEYWORD:
        os << "< keyword, ";
        switch (t.u.keyword)
        {
        case KEYWROD_AUTO: os << "auto"; break;
        case KEYWORD_BREAK: os << "break"; break;
        case KEYWORD_CASE: os << "case"; break;
        case KEYWORD_CHAR: os << "char"; break;
        case KEYWORD_CONST: os << "const"; break;
        case KEYWORD_CONTINUE: os << "continue"; break;
        case KEYWORD_DO: os << "do"; break;
        case KEYWORD_DOUBLE: os << "double"; break;
        case KEYWORD_ELSE: os << "else"; break;
        case KEYWORD_ENUM: os << "enum"; break;
        case KEYWORD_EXTERN: os << "extern"; break;
        case KEYWORD_FLOAT: os << "float"; break;
        case KEYWORD_FOR: os << "for"; break;
        case KEYWORD_GOTO: os << "goto"; break;
        case KEYWORD_IF: os << "if"; break;
        case KEYWORD_INLINE: os << "inline"; break;
        case KEYWORD_INT: os << "int"; break;
        case KEYWORD_LONG: os << "long"; break;
        case KEYWORD_REGISTER: os << "register"; break;
        case KEYWORD_RESTRICT: os << "restrict"; break;
        case KEYWORD_RETURN: os << "return"; break;
        case KEYWORD_SHORT: os << "short"; break;
        case KEYWORD_SIGNED: os << "signed"; break;
        case KEYWORD_SIZEOF: os << "sizeof"; break;
        case KEYWORD_STATIC: os << "static"; break;
        case KEYWORD_STRUCT: os << "struct"; break;
        case KEYWORD_SWITCH: os << "switch"; break;
        case KEYWORD_TYPEDEF: os << "typedef"; break;
        case KEYWORD_UNION: os << "union"; break;
        case KEYWORD_UNSIGNED: os << "unsigned"; break;
        case KEYWORD_VOID: os << "void"; break;
        case KEYWORD_VOLATILE: os << "volatile"; break;
        case KEYWORD_WHILE: os << "while"; break;
        default: os << "INVALID KEWWORD"; break;
        }
        return os << " >";
        break;
    
    case Token::TOKEN_IDENTIFIER:
        return os << "< identifier, " << *t.u.name << " >";
        break;
    case Token::TOKEN_OPERATOR:
        os << "< operator, ";
        switch (t.u.operator_)
        {
        case OPERATOR_LP: os << "("; break;
        case OPERATOR_RP: os << ")"; break;
        case OPERATOR_LC: os << "{"; break;
        case OPERATOR_RC: os << "}"; break;
        case OPERATOR_LB: os << "["; break;
        case OPERATOR_RB: os << "]"; break;
        case OPERATOR_COLON: os << ":"; break;
        case OPERATOR_SEMICOLON: os << ";"; break;
        case OPERATOR_COMMA: os << ","; break;
        case OPERATOR_AND: os << "&&"; break;
        case OPERATOR_OR: os << "||"; break;
        case OPERATOR_EQ: os << "=="; break;
        case OPERATOR_NEQ: os << "!="; break;
        case OPERATOR_LT: os << "<"; break;
        case OPERATOR_LEQ: os << "<="; break;
        case OPERATOR_GT: os << ">"; break;
        case OPERATOR_GEQ: os << ">="; break;
        case OPERATOR_ADD: os << "+"; break;
        case OPERATOR_SUB: os << "-"; break;
        case OPERATOR_MUL: os << "*"; break;
        case OPERATOR_DIV: os << "/"; break;
        case OPERATOR_MOD: os << "%"; break;
        case OPERATOR_ASSIGN: os << "="; break;
        case OPERATOR_ADD_ASSIGN: os << "+="; break;
        case OPERATOR_SUB_ASSIGN: os << "-="; break;
        case OPERATOR_MUL_ASSIGN: os << "*="; break;
        case OPERATOR_DIV_ASSIGN: os << "/="; break;
        case OPERATOR_MOD_ASSIGN: os << "%="; break;
        case OPERATOR_BITAND_ASSIGN: os << "&="; break;
        case OPERATOR_BITOR_ASSIGN: os << "|="; break;
        case OPERATOR_XOR_ASSIGN: os << "^="; break;
        case OPERATOR_LEFI_SHIFT_ASSIGN: os << "<<="; break;
        case OPERATOR_RIGHT_SHIFT_ASSIGN: os << ">>="; break;
        case OPERATOR_BITAND: os << "&"; break;
        case OPERATOR_BITOR: os << "|"; break;
        case OPERATOR_XOR: os << "^"; break;
        case OPERATOR_LEFT_SHIFT: os << "<<"; break;
        case OPERATOR_RIGHT_SHIFT: os << ">>"; break;
        case OPERATOR_ARRAY_POP: os << "->"; break;
        case OPERATOR_CPL: os << "~"; break;
        case OPERATOR_INC: os << "++"; break;
        case OPERATOR_DEC: os << "--"; break;
        case OPERATOR_NOT: os << "!"; break;
        case OPERATOR_DOT: os << "."; break;
        case OPERATOR_QUESMARK: os << "?"; break;
        default:
            os << "INVALID OPERATOR";
            break;
        }
        os << " >";
        break;
    case Token::TOKEN_CHARACTER:
        return os << "< character literal, " << char(t.u.integerValue) << " >";
        break;
    case Token::TOKEN_STRING:
        return os << "< string literal, " << *t.u.stringValue << " >";
        break;
    case Token::TOKEN_INTEGER:
        return os << "< integer, " << t.u.integerValue << " >";
        break;
    case Token::TOKEN_FLOAT:
        return os << "< float, " << t.u.floatValue << " >";
        break;
    }
    return os;
}

Token::Token(Classification c, Keyword KeyType, std::string* _text)
{
    classification = c;
    u.keyword = KeyType;
    text = _text;
}

Token::Token(Classification c, std::string* _text = nullptr)
{
    classification = c;
    switch (classification)
    {
    case TOKEN_IDENTIFIER:
        u.name = _text;
        break;
    case TOKEN_STRING:
        u.stringValue = _text;
        break;
    default: break;
    }
    text = _text;
}

Token::Classification Token::TokenType() const
{
    return this->classification;
}

Token::Token(Classification c, i64 val, std::string* _text)
{
    classification = c;
    u.integerValue = val;
    text = _text;
}

Token::Token(Classification c, double val, std::string* _text)
{
    classification = c;
    u.floatValue = val;
    text = _text;
}

Token::Token(Classification c, Operator OpType, std::string* _text)
{
    classification = c;
    u.operator_ = OpType;
    text = _text;
}

Token Token::make_keyword_token(Keyword KeyType, std::string* text)
{
    return Token(TOKEN_KEYWORD, KeyType, text);
}

Token Token::make_identifier_token(std::string* text)
{
    return Token(TOKEN_IDENTIFIER, text);
}

Token Token::make_integer_token(i64 val, std::string* text)
{
    return Token(TOKEN_INTEGER, val, text);
}

Token Token::make_float_token(double val, std::string* text)
{
    return Token(TOKEN_FLOAT, val, text);
}

Token Token::make_operator_token(Operator OpType, std::string* text)
{
    return Token(TOKEN_OPERATOR, OpType, text);
}

Token Token::make_string_token(std::string* text)
{
    return Token(TOKEN_STRING, text);
}

Token Token::make_character_token(char val, std::string* text)
{
    return Token(TOKEN_CHARACTER, i64(val), text);
}

Token Token::make_eof_token()
{
    return Token(TOKEN_EOF);
}

Token Token::make_invalid_token()
{
    return Token(TOKEN_INVALID);
}

Token::Location::Location(const std::string& _filename, int _row, int _column):
    filename(_filename), row(_row), column(_column) {}

std::ostream& operator << (std::ostream& os, const Token::Location& loc)
{
    return os << loc.filename << ":" << loc.row << ":" << loc.column << ":";
}