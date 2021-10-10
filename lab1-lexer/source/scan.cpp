/*
 * Author: Lou Siyu
 * Date: 2021/09/26 
*/
#include "lexer.h"

#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <cassert>

class InputParser {
private:
    std::vector<std::string> tokens;
public:
    InputParser(int argc, char **argv) {
        for (int i = 1; i < argc; ++i) {
            auto cmd = std::string(argv[i]);
            assert(cmd.length() >= 2);
            tokens.emplace_back(cmd);
        }
    }
    std::string getCmdOption(const std::string& option) {
        auto iter = std::find(tokens.begin(), tokens.end(), option);
        if (iter != tokens.end() && ++iter != tokens.end())
            return *iter;
        static const std::string empty_string("");
        return empty_string;
    }
    bool cmdOptionExists(const std::string& option) {
        return std::find(tokens.begin(), tokens.end(), option) != tokens.end();
    }
    // has the constraint that can handle only one source file at a time
    std::string getSource() { 
        for (auto& token: tokens) {
            int len = token.length();
            if (token.substr(len - 2, 2) == ".c")
                return token.substr(0, len - 2);
        }
        static const std::string empty_string("");
        return empty_string;
    }
};

int main(int argc, char **argv) {
    std::ofstream of;
    std::streambuf *buf;
    InputParser input(argc, argv);
    std::string source = input.getSource(), destination = source + ".txt";
    source = source + ".c";
    // interactive mode
    // if (input.cmdOptionExists("-i")) {
    //     std::cout << "interactive mode" << std::endl;
    //     std::cout << "enter the name of the source file that you want to analyze:" << std::endl;
    //     std::cin >> source;
    //     buf = std::cout.rdbuf();
    // }
    // else {
    if (argc == 1 || source == "") {
        //error printing: no input files
        //lexical analysis terminated
        return 1;
    }
    if (input.cmdOptionExists("-o"))
        destination = input.getCmdOption("-o");
    else if (input.cmdOptionExists("--output"))
        destination = input.getCmdOption("--output");
    of.open(destination);
    buf = of.rdbuf();
    // }
    std::ostream os(buf);
    
    auto lexer = Lexer(source);
    auto token = lexer.getNextToken();
    int invalid_count = 0, keyword_count = 0, identifier_count = 0, float_count = 0,
        operator_count = 0, string_count = 0, character_count = 0, integer_count = 0;
    // int cnt = 0;
    while (token.TokenType() != Token::TOKEN_EOF) {
        // if(++cnt > 300) break;
        switch (token.TokenType())
        {
        case Token::TOKEN_INVALID: invalid_count++; break;
        case Token::TOKEN_KEYWORD: keyword_count++; break;
        case Token::TOKEN_IDENTIFIER: identifier_count++; break;
        case Token::TOKEN_FLOAT: float_count++; break;
        case Token::TOKEN_OPERATOR: operator_count++; break;
        case Token::TOKEN_STRING: string_count++; break;
        case Token::TOKEN_CHARACTER: character_count++; break;
        case Token::TOKEN_INTEGER: integer_count++; break;
        default:
            break;
        }
        if (token.TokenType() != Token::TOKEN_INVALID)
            os << token << std::endl;
        token = lexer.getNextToken();
    }
    std::cout << "Lexical analysis finished with:" << std::endl;
    std::cout << lexer.getRowCount() << "\tlines" << std::endl;
    std::cout << lexer.getCharCount() << "\tcharacters" << std::endl;
    std::cout << keyword_count << "\tkeywords," << std::endl
              << identifier_count << "\tidentifiers," << std::endl
              << operator_count << "\toperators," << std::endl
              << string_count << "\tstrings," << std::endl
              << character_count << "\tcharacterss," << std::endl
              << integer_count << "\tintegers," << std::endl
              << float_count << "\tfloats," << std::endl
              << invalid_count << "\tinvalid tokens" << std::endl;
    std::cout << "in total" << std::endl;
    // lexer.print();
    return 0;
}