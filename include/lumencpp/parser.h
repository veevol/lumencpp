#ifndef LUMENCPP_PARSER_H
#define LUMENCPP_PARSER_H

#include <sstream>
#include <stdexcept>
#include <vector>

#include "exceptions.h"
#include "token.h"
#include "value.h"

namespace lumen {

class Parser {
public:
    [[nodiscard]] Object
    parse(const std::vector<Token>& tokens, Object predefined = {});

private:
    [[nodiscard]] auto at() const noexcept { return *m_at; }

    [[nodiscard]] bool at_end() const noexcept {
        return at().type == Token::Type::Eof;
    }

    auto eat() noexcept { return *(m_at++); }

    template <std::same_as<Token::Type>... Expected>
    auto expect(Token::Type first, Expected... expected) {
        auto result = eat();

        if (result.type != first && ((result.type != expected) && ...)) {
            throw SyntaxError{"unexpected token", result.position};
        }

        return result;
    }

    void skip_line_breaks() noexcept {
        while (!at_end() && at().type == Token::Type::LineBreak) {
            eat();
        }
    }

    [[nodiscard]] Value&
    parse_key_path(Object& parent, bool create_if_not_exist = true);

    [[nodiscard]] auto get_token_lexeme() {
        auto value = eat().lexeme;

        if (!value.has_value()) {
            throw std::logic_error{"valueless token"};
        }

        return *value;
    }

    template <typename ValueType, typename... Manipulators>
    [[nodiscard]] auto
    from_string(std::string value, Manipulators... manipulators) const {
        ValueType result;

        std::istringstream iss{std::move(value)};
        (manipulators(iss), ...);

        iss >> result;
        return result;
    }

    [[nodiscard]] Array parse_array();
    [[nodiscard]] Object parse_object();
    [[nodiscard]] Value parse_integer();

    [[nodiscard]] Value parse_value();
    void parse_assignment(Object& parent);

    Object m_data;

    std::vector<Token>::const_iterator m_at;
};

} // namespace lumen

#endif
