#include "lexer.h"

namespace lexdef 
{
    std::string* SymbolTable::findItem(const std::string& item)
    {
        auto iter = stab.find(item);
        return iter == stab.end() ? nullptr : (*iter).second;
    }
    std::string* SymbolTable::update(const std::string& item)
    {
        auto addr = findItem(item);
        return addr ? addr : stab[item] = new std::string(item);
    }
    SymbolTable::~SymbolTable()
    {
        for (auto iter: stab) 
            delete iter.second; // 释放申请的内存
    }
}

Lexer::Lexer(const std::string& source): file(source), _filename(source)
{
    if (!file) {
        throw std::runtime_error("could not open the file!\n");
    }
    *_end[0] = *_end[1] = lexdef::eof;
    forward = buf;
    if (!loadBuf(0)) terminateFlag = true;
}

bool Lexer::loadBuf(int id)
{
    if(file.peek() != EOF) {
        file.read(_begin[id], lexdef::BFSZ);
        *(_begin[id] + file.gcount()) = lexdef::eof;
        return true;
    }
    return false;
}

Token::Location Lexer::location()
{
    return Token::Location(_filename, rowCount, colCount);
}

char Lexer::peekChar() const
{
    return *forward;
}

char Lexer::nextChar()
{
    char charRead = *forward;
    charCount++; colCount++;
    if (charRead == '\n')
        rowCount++, colCount = 1;
    if (*++forward == lexdef::eof) {
        if (forward == _end[0]) {
            terminateFlag = !loadBuf(1);
            forward = _begin[1];
        }
        else if(forward == _end[1]) {
            terminateFlag = !loadBuf(0);
            forward = _begin[0];
        }
        else /* eof within a buffer marks the end of input */
        {
            terminateFlag = true;
        }
    };
    return charRead;
}

void Lexer::fail()
{
    // panic-recovery strategy
    // skip all characters until it meets characters other than whitespace, ',' or ';'
    // lexdef::failFlag = 1;
    while (!strchr(" \t\n,;*/+-=&^%!~[]{}()", peekChar()) && !this->terminateFlag)
        nextChar();
}

Token Lexer::scanNumber()
{
    // std::cerr << this->location() << rowCount << std::endl;
    auto loc = this->location();
    // default is decimal, may set as octal or hexadecimal
    int base = 10;
    bool isInt = 1;
    bool isValidToken = 1;
    char c = peekChar();
    std::string numRead;
    bool finished = false;
    if (c == '0') {
        nextChar();
        switch (peekChar())
        {
        case 'x':case 'X':
            nextChar();
            base = 16;
            gatherHexnum(numRead);
            finished = true;
            break;
        case 'e':case 'E':
            isInt = false;
            numRead += nextChar();
            if (peekChar() == '+' || peekChar() == '-')
                numRead += nextChar();
            if (!isdigit(peekChar())) { // cases like 3.14e+, 345E ...
                error(loc, "exponent has no digits");
                isValidToken = false;
                fail();
            }
            else gatherDecnum(numRead);
            finished = true;
            break;
        case '.':
            numRead += '0'; numRead += nextChar();
            isInt = false;
            break;
        default:
            base = 8;
            gatherOctnum(numRead);
            finished = true;
            break;
        }
        if (finished) {
            if (!parseSuffix(isInt)) {
                fail();
                isValidToken = false;
            }
        }
    }
    if (!finished) {
        // numRead += c;
        enum states {
            STATE_READ_INTEGER,
            STATE_READ_FLOAT,
            STATE_READ_EXPONENT,
            STATE_READ_SUFFIX,
            STATE_FIN,
        };
        states state;
        if (isInt) state = STATE_READ_INTEGER;
        else state = STATE_READ_FLOAT;
        while (!finished) {
            switch (state)
            {
            case STATE_READ_INTEGER:
                gatherDecnum(numRead);
                break;
            case STATE_READ_FLOAT:
                gatherDecnum(numRead);
                break;
            case STATE_READ_EXPONENT:
                if (peekChar() == '+' || peekChar() == '-')
                    numRead += nextChar();
                if (!isdigit(peekChar())) { // cases like 3.14e+, 345E ...
                    isValidToken = false;
                    fail();
                    error(loc, "exponent has no digits");
                }
                else gatherDecnum(numRead);
                break;
            case STATE_READ_SUFFIX:
                if (!parseSuffix(isInt)) {
                    isValidToken = false;
                    fail();
                }
                state = STATE_FIN;
                break;
            case STATE_FIN:
                finished = true;
                break;
            default:
                isValidToken = false;
                error(loc, "out of reach token");
                fail();
                break;
            }
            if (peekChar() == '.') {
                isInt = 0;
                if (state == STATE_READ_INTEGER) {
                    numRead += nextChar();
                    state = STATE_READ_FLOAT;
                }
                else {
                    error(loc, "multiple decimal places");
                    isValidToken = false;
                    fail();
                    state = STATE_FIN;
                }
            }
            else if (peekChar() == 'e' || peekChar() == 'E') {
                isInt = 0;
                numRead += nextChar();
                state = STATE_READ_EXPONENT;
            }
            else if (state != STATE_FIN)
                state = STATE_READ_SUFFIX;
        }
    }
    if (!isValidToken) {
        return Token::make_invalid_token(location());
    }
    else {
        if (isInt) {
            i64 val = strtol(numRead.c_str(), nullptr, base);
            return Token::make_integer_token(val, loc);
        }
        else {
            double val = strtod(numRead.c_str(), nullptr);
            return Token::make_float_token(val, loc);
        }
    }
}

void Lexer::gatherDecnum(std::string& numRead)
{
    while (isdigit(peekChar()))
        numRead += nextChar();
}

void Lexer::gatherHexnum(std::string& numRead)
{
    while (isHex(peekChar()))
        numRead += nextChar();
}

void Lexer::gatherOctnum(std::string& numRead)
{
    while (isOct(peekChar()))
        numRead += nextChar();
}

bool Lexer::parseSuffix(bool _intFlag)
{
    // double a = 58.;
    // double a = 0x0.12p5F;
    char c = peekChar();
    auto loc = location();
    if (isdig(c)) {
        error(loc, "invalid digit \"%\" in octal constant", std::string(1, c));
        fail();
        return false;
    }
    if (isalpha(c) || c == '_') {
        if (_intFlag) {
            if (c == 'u' || c == 'U' || c == 'l' || c == 'L') {
                nextChar();
                return true;
            }
            else {
                error(loc, "invalid suffix \"%\" on integer constant", std::string(1, c));
                fail();
                return false;
            }
        }
        else {
            if (c == 'f' || c == 'F') {
                nextChar();
                return true;
            }
            else {
                error(loc, "invalid suffix \"%\" on float constant", std::string(1, c));
                fail();
                return false;
            }
        }
    }
    return true;
}

Token Lexer::scanIdentifier()
{
    // std::cerr << "scanning identifier" << std::endl;
    auto loc = this->location();
    std::string idRead;
    idRead += nextChar();
    while (isalp(peekChar()) || isdig(peekChar()))
        idRead += nextChar();
    // std::cout << "idRead = " << idRead << std::endl;
    auto iter = lexdef::_keywords.find(idRead);
    if (iter != lexdef::_keywords.end())
        return Token::make_keyword_token(iter->second, loc);
    else
        return Token::make_identifier_token(__table.update(idRead), loc);
}

void Lexer::skipCppComment()
{
    char c = nextChar();
    while (c != '\n' && !this->terminateFlag)
        c = nextChar();
}

void Lexer::skipCComment()
{
    char c = nextChar();
    while (!this->terminateFlag) {
        if (c == '*' && peekChar() == '/') {
            saw_c_comment = false;
            nextChar();
            break;
        }
        else c = nextChar();
    }
}

Token Lexer::scanOneCharOpr()
{
    auto loc = this->location();
    char c = nextChar();
    auto iter = lexdef::singleCharOperators.find(c);
    return Token::make_operator_token(iter->second, loc);
}

Token Lexer::scanMultiCharOpr()
{
    auto loc = this->location();
    std::string op;
    op += nextChar();
    bool invalidFlag = false;
    // handle next char is eof
    switch (op[0])
    {
    case '&':case '|':case '+':case '-':
        if (peekChar() == op[0] || peekChar() == '=')
            op += nextChar();
        else if (op[0] == '-' && peekChar() == '>')
            op += nextChar();
        break;
    case '=':case '!':case '%':case '^':
        if (peekChar() == '=')
            op += nextChar();
        break;
    case '<':case '>':
        if (peekChar() == '=')
            op += nextChar();
        else if (peekChar() == op[0]) {
            op += nextChar();
            if (peekChar() == '=')
                op += nextChar();
        }
        break;
    case '*':
        if (peekChar() == '/') {
            nextChar();
            invalidFlag = true;
            error(location(), "expected /* before \"*/\" token");
        }
        else if (peekChar() == '=')
            op += nextChar();
        break;
    default:
        invalidFlag = true;
        break;
    }
    if (invalidFlag)
        return Token::make_invalid_token(location());
    auto iter = lexdef::multiCharOperators.find(op);
    return Token::make_operator_token(iter->second, loc);
}

void Lexer::getEscapeCharacter(std::string& charRead)
{
    // char ch = '\12';
    // char ch = '\x70';
    if (peekChar() == lexdef::eof) return;
    std::string digs;
    if (peekChar() == 'x') {
        nextChar();
        for (int i = 0; i < 2 && (isHex(peekChar())); ++i)
            digs += nextChar();
        charRead += char(strtol(digs.c_str(), nullptr, 16));
    }
    else if (isOct(peekChar())) {
        for (int i = 0; i < 3 && (isOct(peekChar())); ++i)
            digs += nextChar();
        int val = strtol(digs.c_str(), nullptr, 8);
        if (val > 0xff)
            error(location(), "octal sequence out of range");
        charRead += char(val);
    }
    else switch (peekChar()) {
        case 'a': charRead += '\a'; nextChar(); break;
        case 'b': charRead += '\b'; nextChar(); break;
        case 'e': charRead += '\e'; nextChar(); break;
        case 'f': charRead += '\f'; nextChar(); break;
        case 'n': charRead += '\n'; nextChar(); break;
        case 't': charRead += '\t'; nextChar(); break;
        case 'r': charRead += '\r'; nextChar(); break;
        case 'v': charRead += '\v'; nextChar(); break;
        case '\\': charRead += '\\'; nextChar(); break;
        case '\'': charRead += '\''; nextChar(); break;
        case '\"': charRead += '\"'; nextChar(); break;
        case '\?': charRead += '\?'; nextChar(); break;
        default:
            charRead += nextChar();
            break;
    }
    
}

Token Lexer::scanCharacter()
{
    auto loc = this->location();
    nextChar();
    char c;
    std::string charRead;
    while (peekChar() != lexdef::eof) {
        c = nextChar();
        if (c == '\n' || c == '\'')
            break;
        else if (c == '\\') 
            getEscapeCharacter(charRead);
        else
            charRead += c;
    }
    if (c != '\'') {
        error(loc, "missing terminating \' character");
        // std::cerr << "missing \'" << std::endl;
        return Token::make_invalid_token(location());
    }
    else if (charRead.length() == 0) {
        error(loc, "empty character constant");
        return Token::make_invalid_token(location());
    }
    else if (charRead.length() > 1) {
        error(loc, "multi-character character constant");
        return Token::make_invalid_token(location());
    }
    else return Token::make_character_token(charRead[0], loc);
}

Token Lexer::scanString()
{
    auto loc = this->location();
    nextChar();
    char c;
    std::string strRead;
    while (peekChar() != lexdef::eof) {
        c = nextChar();
        if (c == '\n' || c == '\"')
            break;
        else if (c == '\\')
            getEscapeCharacter(strRead);
        else 
            strRead += c;
    }
    if (c != '\"') {
        error(loc, "missing terminating \" character");
        return Token::make_invalid_token(location());
    }
    else return Token::make_string_token(__table.update(strRead), loc);
}

Token Lexer::getNextToken()
{
    while (true) {
        if (this->terminateFlag) {
            return Token::make_eof_token(this->location());
        }
        switch (peekChar())
        {
        // skip all whitespace
        case ' ':case '\t':case '\n':
            // std::cerr << "skipping ws" << std::endl;
            while (peekChar() == ' ' || peekChar() == '\t' || peekChar() == '\n') {
                // std::cerr << "forward is: " <<  *forward << std::endl;
                nextChar();
            }
            break;

        case '/': {
            auto loc = this->location();
            nextChar();
            if (peekChar() == '/') {
                nextChar();
                skipCppComment();
            }
            else if (peekChar() == '*') {
                saw_c_comment = true;
                nextChar();
                skipCComment();
                if (saw_c_comment) {
                    error(loc, "unterminated comment");
                    return Token::make_invalid_token(loc);
                }
            }
            else if (peekChar() == '=') {
                nextChar();
                return Token::make_operator_token(OPERATOR_DIV_ASSIGN, loc);
            }
            else 
                return Token::make_operator_token(OPERATOR_DIV, loc);
            break;
        }
        
        case '0':case '1':case '2':case '3':
        case '4':case '5':case '6':case '7':
        case '8':case '9':
            return scanNumber();
            break;
        
        case 'a':case 'b':case 'c':case 'd':case 'e':case 'f':case 'g':
        case 'h':case 'i':case 'j':case 'k':case 'l':case 'm':case 'n':
        case 'o':case 'p':case 'q':case 'r':case 's':case 't':case 'u':
        case 'v':case 'w':case 'x':case 'y':case 'z':
        case 'A':case 'B':case 'C':case 'D':case 'E':case 'F':case 'G':
        case 'H':case 'I':case 'J':case 'K':case 'L':case 'M':case 'N':
        case 'O':case 'P':case 'Q':case 'R':case 'S':case 'T':case 'U':
        case 'V':case 'W':case 'X':case 'Y':case 'Z':case '_':
            // std::cerr << "enters here " << *forward << std::endl;
            return scanIdentifier();
            break;

        case '\'':
            return scanCharacter();
            break;

        case '\"':
            return scanString();
            break;

        case '(':case ')':case '{':case '}':case '[':case ']':case ':':
        case ';':case ',':case '~':case '.':case '?':
            return scanOneCharOpr();
            break;

        // '/' is already handled above
        case '&':case '|':case '=':case '!':case '<':case '>':case '+':
        case '-':case '*':case '%':case '^':
            return scanMultiCharOpr();

        default:
            error(location(), "stray \'%\' in the program", std::string(1, peekChar()));
            fail();
            return Token::make_invalid_token(location());
            break;
        }
    }
}

void Lexer::error(Token::Location _loc, const std::string& msg)
{
    std::cerr << _loc << " " << msg << std::endl;
}

void Lexer::error(Token::Location _loc, const std::string& msg, const std::string& pat)
{
    std::string _msg(msg);
    size_t start_pos = _msg.find("%");
    if (start_pos == std::string::npos) {
        std::cerr << "missing pattern!" << std::endl;
        return;
    }
    _msg.replace(start_pos, 1, pat);
    std::cerr << _loc << " " << _msg << std::endl;
}