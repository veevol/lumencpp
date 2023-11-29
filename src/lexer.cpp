#include "../include/lumencpp/lexer.h"

namespace lumen {

std::vector<Token> Lexer::lex(std::string_view source) {
    std::vector<Token> result;

    m_at = source.begin();
    m_end = source.end();

    m_position = {1, 1};

    m_can_parse_long_token = true;

    skip_useless();

    while (!at_end()) {
        result.push_back(get_token());
        skip_useless();
    }

    result.emplace_back(m_position, Token::Type::Eof);

    return result;
}

Token Lexer::get_identifier() noexcept {
    std::string result;
    auto position = m_position;

    while (!at_end() && (std::isalnum(at()) || at() == '-' || at() == '_')) {
        result += eat();
    }

    m_can_parse_long_token = false;

    if (result == "true" || result == "false") {
        return {position, Token::Type::Boolean, result};
    }

    return {position, Token::Type::Identifier, result};
}

Token Lexer::get_number() {
    std::string result;
    bool valid = false;
    auto position = m_position;

    if (at() == '+') {
        eat();
    } else if (at() == '-') {
        result += eat();
    }

    while (!at_end() && (std::isdigit(at()) || at() == '.' || at() == '_')) {
        if (at() == '.') {
            auto throw_if_invalid_fp = [this, valid, position] {
                if (!valid) {
                    throw SyntaxError{
                        "invalid floating-point literal", position};
                }
            };

            throw_if_invalid_fp();

            valid = false;
            result += eat();

            while (!at_end() && (std::isdigit(at()) || at() == '_')) {
                if (at() == '_') {
                    eat();
                    continue;
                }

                valid = true;
                result += eat();
            }

            throw_if_invalid_fp();

            m_can_parse_long_token = false;

            return {position, Token::Type::Float, result};
        }

        if (at() == '_') {
            eat();
            continue;
        }

        valid = true;
        result += eat();
    }

    if (!valid) {
        throw SyntaxError{"invalid integer literal", position};
    }

    m_can_parse_long_token = false;

    return {position, Token::Type::Integer, result};
}

Token Lexer::get_string() {
    std::string result;
    char quote = eat();
    auto position = m_position;

    auto throw_if_unclosed = [this, position] {
        if (at_end()) {
            throw SyntaxError{"unclosed string", position};
        }
    };

    while (true) {
        throw_if_unclosed();

        if (at() == quote) {
            eat();
            break;
        }

        if (at() == '\\') {
            eat();

            throw_if_unclosed();

            switch (at()) {
            case 'n':
                result += '\n';
                break;
            case 'r':
                result += '\r';
                break;
            case 't':
                result += '\t';
                break;
            default:
                result += at();
                break;
            }

            eat();
        } else {
            result += eat();
        }
    }

    m_can_parse_long_token = false;

    return {position, Token::Type::String, result};
}

Token Lexer::get_token() {
    if (m_can_parse_long_token) {
        if (std::isalpha(at()) || at() == '_') {
            return get_identifier();
        }

        if (std::isdigit(at()) || at() == '+' || at() == '-') {
            return get_number();
        }

        if (at() == '"' || at() == '\'') {
            return get_string();
        }
    }

    auto position = m_position;

    Token::Type type = [this, position] {
        switch (eat()) {
        case '=':
            return Token::Type::Equal;
        case ';':
            return Token::Type::Semicolon;
        case ',':
            return Token::Type::Comma;
        case '.':
            return Token::Type::Dot;
        case '[':
            return Token::Type::LeftBracket;
        case ']':
            return Token::Type::RightBracket;
        case '{':
            return Token::Type::LeftBrace;
        case '}':
            return Token::Type::RightBrace;
        case '\n':
            if (at() == '\r') {
                eat();
            }

            return Token::Type::LineBreak;
        }

        throw SyntaxError{"unexpected character", position};
    }();

    m_can_parse_long_token = true;

    return {position, type};
}

} // namespace lumen
