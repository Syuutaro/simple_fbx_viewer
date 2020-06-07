#include "json.hpp"

//text check function
static bool check_char(char c,const std::string& text,size_t offset){
    if (offset >= 0 && offset < text.size()){
        if (text[offset] == c){
            return true;
        }else{
            return false;
        }
    }else{
        return false;
    }
}

static bool check_str(const std::string& s,const std::string& text,size_t offset){
    if (offset >= 0 && offset+s.size() <= text.size()){
        for (size_t i = 0;i < s.size();i++){
            if (s[i] != text[offset+i]){
                return false;
            }
        }
        return true;
    }else{
        return false;
    }
}

static bool check_digit(const std::string& text,size_t offset){
    if (offset >= 0 && offset < text.size()){
        if (text[offset] >= '0' && text[offset] <= '9'){
            return true;
        }else{
            return false;
        }
    }else{
        return false;
    }
}





//skip function
static bool skip_control_char(const std::string& text,size_t& offset){
    if (check_char('\f',text,offset) || check_char('\n',text,offset) ||
        check_char('\r',text,offset) || check_char('\t',text,offset) ||
        check_char(' ',text,offset))
    {
        offset++;
        return true;
    }else{
        return false;
    }
}





//tokenize function
static bool read_punctuator(std::vector<JSON::Token>& tokens,const std::string& text,size_t& offset){
    if (check_char('{',text,offset) || check_char('}',text,offset) ||
        check_char('[',text,offset) || check_char(']',text,offset) ||
        check_char(':',text,offset) || check_char(',',text,offset))
    {
        JSON::Token token;
        token.type = JSON::PUNCTUATOR_TOKEN;
        token.data = text[offset];
        tokens.push_back(token);
        offset++;
        return true;
    }else{
        return false;
    }
}

static bool read_escape_sequence(char& buffer,const std::string& text,size_t& offset){
    if (!check_char('\\',text,offset)){
        return false;
    }
    if (check_char('\"',text,offset+1) ||
        check_char('\\',text,offset+1) ||
        check_char('/',text,offset+1))
    {
        buffer = text[offset+1];
        offset += 2;
        return true;
    }else if (check_char('b',text,offset+1)){
        buffer = '\b';
        offset += 2;
        return true;
    }else if (check_char('f',text,offset+1)){
        buffer = '\f';
        offset += 2;
        return true;
    }else if (check_char('n',text,offset+1)){
        buffer = '\n';
        offset += 2;
        return true;
    }else if (check_char('r',text,offset+1)){
        buffer = '\r';
        offset += 2;
        return true;
    }else if (check_char('t',text,offset+1)){
        buffer = '\t';
        offset += 2;
        return true;
    }else{
        return false;
    }
}

static bool read_s_char(char& buffer,const std::string& text,size_t& offset){
    if (!check_char('\"',text,offset) && !check_char('\\',text,offset)){
        buffer = text[offset];
        offset++;
        return true;
    }else{
        return read_escape_sequence(buffer,text,offset);
    }
}

static bool read_s_char_sequence(std::string& buffer,const std::string& text,size_t& offset){
    char c;
    if (read_s_char(c,text,offset)){
        buffer += c;
        while (read_s_char(c,text,offset)){
            buffer += c;
        }
        return true;
    }else{
        return false;
    }
}

static bool read_string(std::vector<JSON::Token>& tokens,const std::string& text,size_t& offset){
    //tmp data
    JSON::Token t_token;
    size_t t_offset = offset;

    //read "
    if (check_char('\"',text,t_offset)){
        t_offset++;
    }else{
        return false;
    }

    //read s_char
    read_s_char_sequence(t_token.data,text,t_offset);

    //read "
    if (check_char('\"',text,t_offset)){
        t_offset++;
    }else{
        return false;
    }

    //success
    t_token.type = JSON::STRING_TOKEN;
    tokens.push_back(t_token);
    offset = t_offset;
    return true;
}


static bool read_number(std::vector<JSON::Token>& tokens,const std::string& text,size_t& offset){
    //tmp data
    JSON::Token t_token;
    size_t t_offset = offset;

    //read sign part
    if (check_char('+',text,t_offset) || check_char('-',text,t_offset)){
        t_token.data += text[t_offset];
        t_offset++;
    }

    //read integer part
    if (check_digit(text,t_offset)){
        if (text[t_offset] == '0'){
            t_token.data += text[t_offset];
            t_offset++;
        }else{
            while (check_digit(text,t_offset)){
                t_token.data += text[t_offset];
                t_offset++;
            }
        }
    }else{
        return false;
    }

    //read decimal part
    if (check_char('.',text,t_offset)){
        t_token.data += text[t_offset];
        t_offset++;
        //read digit
        if (check_digit(text,t_offset)){
            while (check_digit(text,t_offset)){
                t_token.data += text[t_offset];
                t_offset++;
            }
        }else{
            return false;
        }
    }

    //read exponent part
    if (check_char('e',text,t_offset) || check_char('E',text,t_offset)){
        t_token.data += text[t_offset];
        t_offset++;

        //read sign
        if (check_char('+',text,t_offset) || check_char('-',text,t_offset)){
            t_token.data += text[t_offset];
            t_offset++;
        }

        //read digit
        if (check_digit(text,t_offset)){
            while (check_digit(text,t_offset)){
                t_token.data += text[t_offset];
                t_offset++;
            }
        }else{
            return false;
        }
    }

    //success
    t_token.type = JSON::NUMBER_TOKEN;
    tokens.push_back(t_token);
    offset = t_offset;
    return true;
}

static bool read_boolean(std::vector<JSON::Token>& tokens,const std::string& text,size_t& offset){
    //true
    if (check_str("true",text,offset)){
        JSON::Token token;
        token.type = JSON::BOOLEAN_TOKEN;
        token.data = "true";
        tokens.push_back(token);
        offset += 4;
        return true;
    }

    //false
    if (check_str("false",text,offset)){
        JSON::Token token;
        token.type = JSON::BOOLEAN_TOKEN;
        token.data = "false";
        tokens.push_back(token);
        offset += 5;
        return true;
    }

    //error
    return false;
}

static bool read_null(std::vector<JSON::Token>& tokens,const std::string& text,size_t& offset){
    if (check_str("null",text,offset)){
        JSON::Token token;
        token.type = JSON::NULL_TOKEN;
        token.data = "null";
        tokens.push_back(token);
        offset += 4;
        return true;
    }else{
        return false;
    }
}

static void tokenize(std::vector<JSON::Token>& tokens,const std::string& text){
    //offset
    size_t offset = 0;

    //tokenize
    while (offset < text.size()){
        //skip control character
        if (skip_control_char(text,offset)){
            continue;
        }

        //read punctuator
        if (read_punctuator(tokens,text,offset)){
            continue;
        }

        //read number
        if (read_number(tokens,text,offset)){
            continue;
        }

        //read string
        if (read_string(tokens,text,offset)){
            continue;
        }

        //read boolean
        if (read_boolean(tokens,text,offset)){
            continue;
        }

        //read null
        if (read_null(tokens,text,offset)){
            continue;
        }

        //error
        std::cout << "json.cpp:syntax error/failed to tokenize json file" << "\n";
        std::terminate();
    }
}





//token check function
static bool check_token_type(JSON::TokenType type,const std::vector<JSON::Token>& tokens,size_t offset){
    if (offset >= 0 && offset < tokens.size()){
        if (tokens[offset].type == type){
            return true;
        }else{
            return false;
        }
    }else{
        return false;
    }
}

static bool check_token_data(const std::string& data,const std::vector<JSON::Token>& tokens,size_t offset){
    if (offset >= 0 && offset < tokens.size()){
        if (tokens[offset].data == data){
            return true;
        }else{
            return false;
        }
    }else{
        return false;
    }
}





//parse function
static JSON::Node* read_value(const std::vector<JSON::Token>& tokens,size_t& offset);
static JSON::Node* read_object(const std::vector<JSON::Token>& tokens,size_t& offset);
static JSON::Node* read_array(const std::vector<JSON::Token>& tokens,size_t& offset);

static JSON::Node* read_value(const std::vector<JSON::Token>& tokens,size_t& offset){
    JSON::Node* node = nullptr;
 
    //read object
    node = read_object(tokens,offset);
    if (node != nullptr){
        return node;
    }

    //read array
    node = read_array(tokens,offset);
    if (node != nullptr){
        return node;
    }

    //read number
    if (check_token_type(JSON::NUMBER_TOKEN,tokens,offset)){
        node = new JSON::Node();
        node->type = JSON::NUMBER_NODE;
        node->number = std::stod(tokens[offset].data);
        offset++;
        return node;
    }

    //read string
    if (check_token_type(JSON::STRING_TOKEN,tokens,offset)){
        node = new JSON::Node();
        node->type = JSON::STRING_NODE;
        node->string = tokens[offset].data;
        offset++;
        return node;
    }

    //read boolean
    if (check_token_type(JSON::BOOLEAN_TOKEN,tokens,offset)){
        node = new JSON::Node();
        node->type = JSON::BOOLEAN_NODE;
        node->boolean = (tokens[offset].data == "true")? true:false;
        offset++;
        return node;
    }

    //read null
    if (check_token_type(JSON::NULL_TOKEN,tokens,offset)){
        node = new JSON::Node();
        node->type = JSON::NULL_NODE;
        offset++;
        return node;
    }

    //error
    return nullptr;
}

static JSON::Node* read_object(const std::vector<JSON::Token>& tokens,size_t& offset){
    //create node
    JSON::Node* node = new JSON::Node();
    node->type = JSON::OBJECT_NODE;
    
    //tmp offset
    size_t t_offset = offset;
    
    //read "{"
    if (check_token_data("{",tokens,t_offset)){
        t_offset++;
    }else{
        delete node;
        return nullptr;
    }

    //read members
    while (t_offset < tokens.size()){
        //read string token
        std::string key;
        if (check_token_type(JSON::STRING_TOKEN,tokens,t_offset)){
            key = tokens[t_offset].data;
            t_offset++;
        }else{
            break;
        }

        //read ":"
        if (check_token_data(":",tokens,t_offset)){
            t_offset++;
        }else{
            break;
        }

        //read value
        JSON::Node* child = read_value(tokens,t_offset);
        if (child != nullptr){
            node->members[key] = child;
        }else{
            break;
        }

        //read ","
        if (check_token_data(",",tokens,t_offset)){
            if (!check_token_data("}",tokens,t_offset+1)){
                t_offset++;
            }else{
                break;
            }
        }else{
            break;
        }
    }

    //read "}"
    if (check_token_data("}",tokens,t_offset)){
        t_offset++;
    }else{
        delete node;
        return nullptr;
    }

    //success
    offset = t_offset;
    return node;
}

static JSON::Node* read_array(const std::vector<JSON::Token>& tokens,size_t& offset){
    //create node
    JSON::Node* node = new JSON::Node();
    node->type = JSON::ARRAY_NODE;
    
    //tmp offset
    size_t t_offset = offset;

    //read "["
    if (check_token_data("[",tokens,t_offset)){
        t_offset++;
    }else{
        delete node;
        return nullptr;
    }

    //read values
    while (t_offset < tokens.size()){
        //read value
        JSON::Node* child = read_value(tokens,t_offset);
        if (child != nullptr){
            node->elements.push_back(child);
        }else{
            break;
        }

        //read ","
        if (check_token_data(",",tokens,t_offset)){
            if (!check_token_data("]",tokens,t_offset+1)){
                t_offset++;
            }else{
                break;
            }
        }else{
            break;
        }
    }

    //read "]"
    if (check_token_data("]",tokens,t_offset)){
        t_offset++;
    }else{
        delete node;
        return nullptr;
    }

    //success
    offset = t_offset;
    return node;
}


//json
JSON::JSON(const std::string& path){
    //open file
    std::FILE* fp = std::fopen(path.c_str(),"r");
    if (fp == NULL){
        std::cout << "json.cpp:failed to open " << path << "\n";
        std::terminate();
    }

    //file size
    std::fseek(fp,0,SEEK_END);
    size_t size = std::ftell(fp);

    //allocate memory
    char* src = new char[size+1];

    //read file
    std::fseek(fp,0,SEEK_SET);
    std::fread(src,1,size,fp);
    src[size] = '\0';

    //copy file source
    std::string text(src);

    //free memory
    delete[] src;

    //close file
    std::fclose(fp);
    
    //tokenize
    std::vector<Token> tokens;
    tokenize(tokens,text);
    
    
    //parse
    size_t offset = 0;
    m_root = read_value(tokens,offset);
    if (m_root == nullptr){
        std::cout << "json.cpp:failed to parse " << path << "\n";
        std::terminate();
    }
}

const JSON::Node& JSON::operator[](const std::string& key) const{
    return (*m_root)[key];
}

const JSON::Node& JSON::operator[](size_t index) const{
    return (*m_root)[index];
}


JSON::Node::Node(){
    type = EMPTY_NODE;
}
JSON::Node::~Node(){
    for (auto i = members.begin();i != members.end();++i){
        delete i->second;
    }
    for (size_t i = 0;i < elements.size();i++){
        delete elements[i];
    }
}
JSON::NodeType JSON::Node::GetNodeType() const{
    return type;
}

const JSON::Node& JSON::Node::operator[](const std::string& key) const{
    if (members.count(key) == 0){
        std::cout << "json.cpp:failed to get node of key " << key << "\n";
        std::terminate();
    }
    return *members.at(key);
}
size_t JSON::Node::GetMemberCount() const{
    return members.size();
}

const JSON::Node& JSON::Node::operator[](size_t index) const{
    if (index >= elements.size()){
        std::cout << "json.cpp:failed to get value of index " << index << "\n";
        std::terminate();
    }
    return *elements[index];
}
size_t JSON::Node::GetElementCount() const{
    return elements.size();
}

const std::string& JSON::Node::GetString() const{
    return string;
}

double JSON::Node::GetNumber() const{
    return number;
}

bool JSON::Node::GetBoolean() const{
    return boolean;
}
