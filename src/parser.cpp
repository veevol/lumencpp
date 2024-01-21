#include <utility>

#include "../include/lumencpp/parser.h"

namespace lumen {

Object Parser::parse(
    const std::vector<Token>& tokens, std::string filename, Object predefined) {
    m_data = std::move(predefined);

    m_filename = std::move(filename);

    if (!tokens.empty() && tokens.back().type != Token::Type::Eof) {
        throw ParseError{
            "expected an end of file at the end of the input",
            std::move(m_filename), tokens.back().source};
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

        expect<
            Token::Type::LineBreak, Token::Type::Semicolon, Token::Type::Eof>();

        skip_line_breaks();
    }

    return std::move(m_data);
}

Value& Parser::parse_key_path(
    Object& parent, const Token& token, bool create_if_not_exist) {
    auto key = get_token_lexeme(token);
    auto source = token.source;

    if (!create_if_not_exist &&
        parent[key].get_type() == Value::Type::Undefined) {
        throw ParseError{
            "field '" + key + "' does not exist", std::move(m_filename),
            source};
    }

    Value* result = &parent[key];

    if (at().type == Token::Type::Dot) {
        eat();

        try {
            return parse_key_path(
                result->get_strict<Object>(), create_if_not_exist);
        } catch (const TypeMismatch&) {
            throw ParseError{
                "unable to parse a key path, '" + key +
                    "' was defined and is not an object",
                std::move(m_filename), source};
        }
    }

    return *result;
}

Array Parser::parse_array() {
    Array result;

    while (true) {
        skip_line_breaks();

        if (at_end()) {
            expect<Token::Type::RightBracket>();
        }

        if (at().type == Token::Type::RightBracket) {
            break;
        }

        result.push_back(parse_value());

        if (at().type == Token::Type::RightBracket) {
            break;
        }

        expect<Token::Type::LineBreak, Token::Type::Comma>();
    }

    eat();

    return result;
}

Object Parser::parse_object() {
    Object result;

    while (true) {
        skip_line_breaks();

        if (at_end()) {
            expect<Token::Type::RightBrace>();
        }

        if (at().type == Token::Type::RightBrace) {
            break;
        }

        parse_assignment(result);

        if (at().type == Token::Type::RightBrace) {
            break;
        }

        expect<Token::Type::LineBreak, Token::Type::Comma>();
    }

    eat();

    return result;
}

Value Parser::parse_integer(const Token& token) {
    auto number = get_token_lexeme(token);

    if (number.starts_with('-')) {
        return from_string<Int>(token.source, number);
    }

    return from_string<UInt>(token.source, number);
}

Value Parser::parse_value() {
    auto token = expect<
        Token::Type::LeftBracket, Token::Type::LeftBrace,
        Token::Type::Identifier, Token::Type::Integer, Token::Type::Boolean,
        Token::Type::Float, Token::Type::String>();

    switch (token.type) {
    case Token::Type::LeftBracket:
        return parse_array();
    case Token::Type::LeftBrace:
        return parse_object();
    case Token::Type::Identifier:
        return parse_key_path(m_data, token, false);
    case Token::Type::Integer:
        return parse_integer(token);
    case Token::Type::Boolean:
        return get_token_lexeme(token) == "true";
    case Token::Type::Float:
        return from_string<Float>(token.source, get_token_lexeme(token));
    case Token::Type::String:
        return get_token_lexeme(token);
    default:
        return {};
    }
}

void Parser::parse_assignment(Object& parent) {
    auto& key = parse_key_path(parent);
    expect<Token::Type::Equal>();
    key = parse_value();
}

} // namespace lumen
