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
    auto position = m_position;

    auto get_if_e = [&] {
        if (at_end() || at() != 'e') {
            return false;
        }

        result += eat();

        if (!at_end() && (at() == '-' || at() == '+')) {
            result += eat();
        }

        result += get_integer();

        return true;
    };

    m_can_parse_long_token = false;

    if (at() == '0') {
        result += eat();

        if (at_end()) {
            return {position, Token::Type::Integer, result};
        }

        if (std::isdigit(at()) || at() == '_') {
            throw SyntaxError{"leading zeros are not allowed", m_position};
        }

        switch (at()) {
        case 'x':
            result += eat();
            result += get_integer(
                [](char character) { return std::isxdigit(character); });

            return {position, Token::Type::Integer, result};
        case 'o':
            result += eat();
            result += get_integer([](char character) {
                return character >= '0' && character <= '7';
            });

            return {position, Token::Type::Integer, result};
        case 'b':
            result += eat();
            result += get_integer([](char character) {
                return character == '0' || character == '1';
            });

            return {position, Token::Type::Integer, result};
        case '.':
            result += eat();
            result += get_integer();

            get_if_e();

            return {position, Token::Type::Float, result};
        default:
            return {position, Token::Type::Integer, result};
        }
    } else {
        if (at() == '+') {
            eat();
        } else if (at() == '-') {
            result += eat();
        }
    }

    result += get_integer();

    if (at() == '.') {
        result += eat();
        result += get_integer();

        get_if_e();

        return {position, Token::Type::Float, result};
    }

    if (get_if_e()) {
        return {position, Token::Type::Float, result};
    }

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

        if (at() == '`') {
            auto token = get_string();
            return {token.position, Token::Type::Identifier, token.lexeme};
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
