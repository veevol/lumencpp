#include <bitset>
#include <ios>
#include <limits>
#include <utility>

#include "../include/lumencpp/parser.h"

namespace lumen {

Object Parser::parse(const std::vector<Token>& tokens, Object predefined) {
    m_data = std::move(predefined);

    if (!tokens.empty() && tokens.back().type != Token::Type::Eof) {
        throw std::logic_error{"expected EOF at the end of the input"};
    }

    m_at = tokens.begin();

    skip_line_breaks();

    while (!at_end()) {
        if (at().type == Token::Type::Semicolon) {
            eat();
            skip_line_breaks();

            continue;
        }

        parse_assignment(m_data);

        if (at_end()) {
            break;
        }

        expect(
            Token::Type::LineBreak, Token::Type::Semicolon, Token::Type::Eof);

        skip_line_breaks();
    }

    return std::move(m_data);
}

Value& Parser::parse_key_path(Object& parent, bool create_if_not_exist) {
    auto key = expect(Token::Type::Identifier).lexeme;

    if (!key.has_value()) {
        throw std::logic_error{"valueless token"};
    }

    if (!create_if_not_exist && !parent.contains(*key)) {
        throw std::runtime_error{"key doesn't exist"};
    }

    Value* result = &parent[*key];

    if (at().type == Token::Type::Dot) {
        eat();

        return parse_key_path(result->get<Object>(), create_if_not_exist);
    }

    return *result;
}

Array Parser::parse_array() {
    Array result;

    auto array_position = expect(Token::Type::LeftBracket).position;

    while (true) {
        skip_line_breaks();

        if (at_end()) {
            throw SyntaxError{"unclosed array", array_position};
        }

        if (at().type == Token::Type::RightBracket) {
            break;
        }

        result.push_back(parse_value());

        if (at().type == Token::Type::RightBracket) {
            break;
        }

        expect(Token::Type::LineBreak, Token::Type::Comma);
    }

    eat();

    return result;
}

Object Parser::parse_object() {
    Object result;

    auto object_position = expect(Token::Type::LeftBrace).position;

    while (true) {
        skip_line_breaks();

        if (at_end()) {
            throw SyntaxError{"unclosed object", object_position};
        }

        if (at().type == Token::Type::RightBrace) {
            break;
        }

        parse_assignment(result);

        if (at().type == Token::Type::RightBrace) {
            break;
        }

        expect(Token::Type::LineBreak, Token::Type::Comma);
    }

    eat();

    return result;
}

Value Parser::parse_integer() {
    auto lexeme = get_token_lexeme();

    if (lexeme.starts_with('-')) {
        return from_string<Int>(lexeme);
    }

    if (lexeme.starts_with('0')) {
        if (lexeme.size() < 2) {
            return 0U;
        }

        auto number = lexeme.substr(2);

        if (lexeme.starts_with("0x")) {
            return from_string<UInt>(number, std::hex);
        }

        if (lexeme.starts_with("0o")) {
            return from_string<UInt>(number, std::oct);
        }

        if (lexeme.starts_with("0b")) {
            return std::bitset<std::numeric_limits<UInt>::digits>(number)
                .to_ulong();
        }
    }

    return from_string<UInt>(lexeme);
}

Value Parser::parse_value() {
    switch (at().type) {
    case Token::Type::LeftBracket:
        return parse_array();
    case Token::Type::LeftBrace:
        return parse_object();
    case Token::Type::Identifier:
        return parse_key_path(m_data, false);
    case Token::Type::Integer:
        return parse_integer();
    case Token::Type::Boolean:
        return get_token_lexeme() == "true";
    case Token::Type::Float:
        return from_string<Float>(get_token_lexeme());
    case Token::Type::String:
        return get_token_lexeme();
    default:
        throw SyntaxError{"unexpected token", at().position};
    }
}

void Parser::parse_assignment(Object& parent) {
    auto& key = parse_key_path(parent);
    expect(Token::Type::Equal);
    key = parse_value();
}

} // namespace lumen
