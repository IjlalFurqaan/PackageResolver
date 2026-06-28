#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <iostream>

namespace pkgr {

enum class JsonType {
    STRING,
    NUMBER,
    BOOLEAN,
    NULL_VAL,
    OBJECT,
    ARRAY
};

class JsonValue {
public:
    virtual ~JsonValue() = default;
    virtual JsonType type() const = 0;
    virtual std::string to_string(int indent = 0) const = 0;

    bool is_string() const { return type() == JsonType::STRING; }
    bool is_number() const { return type() == JsonType::NUMBER; }
    bool is_boolean() const { return type() == JsonType::BOOLEAN; }
    bool is_null() const { return type() == JsonType::NULL_VAL; }
    bool is_object() const { return type() == JsonType::OBJECT; }
    bool is_array() const { return type() == JsonType::ARRAY; }
};

class JsonString : public JsonValue {
    std::string value_;
public:
    explicit JsonString(std::string value) : value_(std::move(value)) {}
    JsonType type() const override { return JsonType::STRING; }
    const std::string& value() const { return value_; }
    std::string to_string(int indent = 0) const override;
};

class JsonNumber : public JsonValue {
    int value_;
public:
    explicit JsonNumber(int value) : value_(value) {}
    JsonType type() const override { return JsonType::NUMBER; }
    int value() const { return value_; }
    std::string to_string(int indent = 0) const override;
};

class JsonBool : public JsonValue {
    bool value_;
public:
    explicit JsonBool(bool value) : value_(value) {}
    JsonType type() const override { return JsonType::BOOLEAN; }
    bool value() const { return value_; }
    std::string to_string(int indent = 0) const override;
};

class JsonNull : public JsonValue {
public:
    JsonType type() const override { return JsonType::NULL_VAL; }
    std::string to_string(int indent = 0) const override;
};

class JsonObject : public JsonValue {

    std::vector<std::pair<std::string, std::unique_ptr<JsonValue>>> entries_;
public:
    JsonType type() const override { return JsonType::OBJECT; }

    void set(const std::string& key, std::unique_ptr<JsonValue> value);
    const JsonValue* get(const std::string& key) const;
    bool has(const std::string& key) const;

    const std::string& get_string(const std::string& key) const;
    int get_number(const std::string& key) const;
    bool get_bool(const std::string& key) const;
    const JsonObject& get_object(const std::string& key) const;

    const auto& entries() const { return entries_; }
    size_t size() const { return entries_.size(); }

    std::string to_string(int indent = 0) const override;
};

class JsonArray : public JsonValue {
    std::vector<std::unique_ptr<JsonValue>> elements_;
public:
    JsonType type() const override { return JsonType::ARRAY; }

    void push(std::unique_ptr<JsonValue> value);
    const JsonValue* at(size_t index) const;
    size_t size() const { return elements_.size(); }

    const auto& elements() const { return elements_; }

    std::string to_string(int indent = 0) const override;
};

enum class TokenType {
    STRING,
    NUMBER,
    LBRACE,     
    RBRACE,     
    LBRACKET,   
    RBRACKET,   
    COLON,      
    COMMA,      
    TRUE_VAL,
    FALSE_VAL,
    NULL_VAL,
    END_OF_INPUT
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;
};

class JsonTokenizer {
public:
    explicit JsonTokenizer(const std::string& input);
    Token next_token();
    Token peek_token();

private:
    std::string input_;
    size_t pos_;
    int line_;
    int column_;
    bool has_peeked_;
    Token peeked_token_;

    void skip_whitespace();
    Token read_string();
    Token read_number();
    Token read_keyword();
    char current() const;
    char advance();
    bool at_end() const;
};

class JsonParser {
public:

    static std::unique_ptr<JsonValue> parse(const std::string& json_str);

    static std::unique_ptr<JsonValue> parse_file(const std::string& file_path);

private:
    JsonTokenizer tokenizer_;

    explicit JsonParser(const std::string& input);

    std::unique_ptr<JsonValue> parse_value();
    std::unique_ptr<JsonValue> parse_object();
    std::unique_ptr<JsonValue> parse_array();

    Token expect(TokenType type, const std::string& context);
};

} 