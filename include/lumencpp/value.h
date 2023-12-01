#ifndef LUMENCPP_VALUE_H
#define LUMENCPP_VALUE_H

#include <concepts>
#include <cstdint>
#include <initializer_list>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace lumen {

class Value {
public:
    using UInt = std::uint64_t;
    using Int = std::int64_t;
    using Float = double;
    using Bool = bool;
    using String = std::string;
    using Array = std::vector<Value>;
    using Object = std::unordered_map<std::string, Value>;

    enum class Type { Unknown, UInt, Int, Float, Bool, String, Array, Object };

    [[nodiscard]] Value() noexcept : m_value{std::monostate{}} {}

    [[nodiscard]] Value(std::unsigned_integral auto value) noexcept
    : m_value{static_cast<UInt>(value)} {}

    [[nodiscard]] Value(std::signed_integral auto value) noexcept
    : m_value{static_cast<Int>(value)} {}

    [[nodiscard]] Value(std::floating_point auto value) noexcept
    : m_value{static_cast<Float>(value)} {}

    [[nodiscard]] Value(Bool value) noexcept : m_value{value} {}

    [[nodiscard]] Value(String value) noexcept : m_value{std::move(value)} {}

    [[nodiscard]] Value(const char* value) noexcept
    : m_value{std::string{value}} {}

    [[nodiscard]] Value(Array value) noexcept : m_value{std::move(value)} {}

    [[nodiscard]] Value(Object value) noexcept : m_value{std::move(value)} {}

    [[nodiscard]] Value(
        std::initializer_list<Object::value_type> value) noexcept
    : m_value{Object{value}} {}

    Value& operator=(auto value) noexcept {
        *this = Value{value};
        return *this;
    }

    [[nodiscard]] auto get_type() const noexcept {
        return static_cast<Type>(m_value.index());
    }

    [[nodiscard]] bool is(Type type) const noexcept {
        return get_type() == type;
    }

    template <typename ValueType> [[nodiscard]] bool is() const noexcept {
        return std::holds_alternative<ValueType>(m_value);
    }

    template <typename ValueType> [[nodiscard]] auto as() const {
        if constexpr (std::is_same_v<ValueType, Bool>) {
            return std::get<Bool>(m_value);
        }

        if constexpr (std::is_integral_v<ValueType>) {
            if (is<UInt>()) {
                return static_cast<ValueType>(std::get<UInt>(m_value));
            }

            return static_cast<ValueType>(std::get<Int>(m_value));
        }

        if constexpr (std::is_floating_point_v<ValueType>) {
            return static_cast<ValueType>(std::get<Float>(m_value));
        }

        return get<ValueType>();
    }

    template <typename ValueType> [[nodiscard]] auto& get() {
        if (get_type() == Type::Unknown) {
            try {
                m_value = ValueType{};
            } catch (...) {
                throw std::logic_error{"invalid value type"};
            }
        }

        try {
            return std::get<ValueType>(m_value);
        } catch (...) {
            throw std::runtime_error{"value is of different type"};
        }
    }

    [[nodiscard]] const auto& operator[](const Object::key_type& key) const {
        return get<Object>().at(key);
    }

    [[nodiscard]] auto& operator[](const Object::key_type& key) {
        return get<Object>()[key];
    }

    [[nodiscard]] const auto& operator[](Array::size_type index) const {
        return get<Array>()[index];
    }

    [[nodiscard]] auto& operator[](Array::size_type index) {
        return get<Array>()[index];
    }

    [[nodiscard]] bool operator==(const Value& other) const = default;

private:
    template <typename ValueType> [[nodiscard]] const ValueType& get() const {
        try {
            return std::get<ValueType>(m_value);
        } catch (...) {
            throw std::runtime_error{"value is of different type"};
        }
    }

    std::variant<std::monostate, UInt, Int, Float, Bool, String, Array, Object>
        m_value;
};

} // namespace lumen

#endif
