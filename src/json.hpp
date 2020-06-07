#ifndef JSON_HPP
#define JSON_HPP

#include "library.hpp"

class JSON{
public:
    enum TokenType{
        PUNCTUATOR_TOKEN,
        NUMBER_TOKEN,
        STRING_TOKEN,
        BOOLEAN_TOKEN,
        NULL_TOKEN,
    };
    enum NodeType{
        OBJECT_NODE,
        ARRAY_NODE,
        NUMBER_NODE,
        STRING_NODE,
        BOOLEAN_NODE,
        NULL_NODE,
        EMPTY_NODE,
    };
    struct Token{
        TokenType type;
        std::string data;
    };
    struct Node{
        //common
        NodeType type;
        
        //for object
        std::map<std::string,Node*> members;
        
        //for array
        std::vector<Node*> elements;
        
        //for string
        std::string string;
        
        //for number
        double number;
        
        //for boolean
        bool boolean;
        
        //common
        Node();
        ~Node();
        JSON::NodeType GetNodeType() const;
        
        //for object
        const Node& operator[](const std::string& key) const;
        size_t GetMemberCount() const;
        
        //for array
        const Node& operator[](size_t index) const;
        size_t GetElementCount() const;
        
        //for string
        const std::string& GetString() const;
        
        //for number
        double GetNumber() const;
        
        //for boolean
        bool GetBoolean() const;
    };
private:
    Node* m_root;
public:
    JSON(const std::string& path);
    const JSON::Node& operator[](const std::string& key) const;
    const JSON::Node& operator[](size_t index) const;
};


#endif // JSON_HPP
