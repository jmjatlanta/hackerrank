#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <memory>

/**
 * Parse HRML and run queries
 * Implemented as a doubly linked list
 * tags have a parent, siblings, children, and attributes
 * attributes have siblings
 */

class parse_exception : public std::exception
{
    public:
    parse_exception(const std::string in) : std::exception(), msg(in) { }
    virtual const char* what() const noexcept override { return msg.c_str(); }
    std::string msg;
};

class attribute
{
    public:
    /***
     * ctor
     * @param pos where the attribute starts (may be a blank space)
     * @param orig the whole string
     */
    attribute(int pos, const std::string* orig) :  start_pos(pos), doc(orig) 
    {
        bool name_retrieved = false;
        bool in_quotes = false;
        size_t val_start = 0;
        // fast forward to first non-space
        for(size_t i = pos; i < orig->length(); ++i)
        {
            if ( (*orig)[i] != ' ' && (*orig)[i] != '>')
            {
                pos = i;
                break;
            }
        }
        for(size_t i = pos; i < orig->length(); ++i)
        {
            if ( ( (*orig)[i] == ' ' || (*orig)[i] == '=' ) && name.empty())
                name = orig->substr(pos, i-pos);
            if ( (*orig)[i] == ' ' && !name.empty())
                continue;
            if ( (*orig)[i] == '"')
            {
                in_quotes = !in_quotes;
                if (in_quotes)
                    val_start = i + 1;
                else
                {
                    value = orig->substr(val_start, i-val_start);
                    end_pos = i;
                    break;
                }
            }
        }
        if (name.empty())
            throw parse_exception("No name retrieved from attribute" + orig->substr(pos));
    }
    ~attribute() 
    {
        if (next_sibling != nullptr)
        {
            delete next_sibling;
        }
    }
    void add_sibling(attribute* in)
    {
        if (next_sibling == nullptr)
            next_sibling = in;
        else
            next_sibling->add_sibling(in);
    }
    attribute* find_sibling(const std::string& in)
    {
        if (next_sibling == nullptr || next_sibling->name == in)
            return next_sibling;
        return next_sibling->find_sibling(in);
    }
    size_t start_pos = 0;
    size_t end_pos = 0;
    std::string name;
    std::string value;
    const std::string* doc = nullptr;
    attribute* next_sibling = nullptr;
};

class tag
{
    public:
    /****
     * Parse a tag all the way until it is closed
     */
    tag(size_t pos, const std::string* orig) : start_pos(pos), doc(orig) 
    {
        // parse the incoming string until closing tag completed
        size_t curr_pos = parse_name(pos);
        curr_pos = parse_attributes(pos, curr_pos);
        // curr_pos should now be at the end of the opening tag
        for(size_t i = curr_pos+1; i < orig->length(); ++i)
        {
            // skip unwanted characters
            char c = (*orig)[i];
            if (c == '\n' || c == ' ' || c == '\t')
                continue;
            // work on children
            if (c != '<')
                throw parse_exception("No opening brace found: " + orig->substr(i));
            if ((*orig)[i+1] == '/')
            {
                // we are at the end tag for this tag
                end_pos = orig->substr(i).find(">") + i;
                i = end_pos;
                break;
            }
            tag* latest = add_child(new tag(i, orig));
            i = latest->end_pos;
        }

    }
    ~tag()
    {
        if (first_sibling != nullptr)
        {
            delete first_sibling;
        }
        if (first_child != nullptr)
        {
            delete first_child;
        }
        if (first_attribute != nullptr)
        {
            delete first_attribute;
        }
    };
    void add_sibling(tag* in)
    {
        if (first_sibling == nullptr)
            first_sibling = in;
        else
            first_sibling->add_sibling(in);
    }
    tag* find_child(const std::string& in)
    {
        if (first_child == nullptr || first_child->name == in)
            return first_child;
        return first_child->find_sibling(in);
    }
    tag* find_sibling(const std::string& in)
    {
        if (first_sibling == nullptr || first_sibling->name == in)
            return first_sibling;
        return first_sibling->find_sibling(in);
    }
    tag* add_child(tag* in)
    {
        if (first_child == nullptr)
            first_child = in;
        else
            first_child->add_sibling(in);
        return in;
    }
    void add_attribute(attribute* in)
    {
        if (first_attribute == nullptr)
            first_attribute = in;
        else
            first_attribute->add_sibling(in);
    }
    attribute* find_attribute(const std::string& in)
    {
        if (first_attribute == nullptr || first_attribute->name == in)
            return first_attribute;
        return first_attribute->find_sibling(in);
    }
    size_t start_pos;
    size_t end_pos = 0;
    size_t open_tag_end = -1;
    const std::string* doc = nullptr;
    std::string name;
    tag* parent = nullptr;
    tag* first_sibling = nullptr;
    tag* first_child = nullptr;
    attribute* first_attribute = nullptr;
    private:
    /***
     * Parse the name of this tag
     * @param pos the starting position
     * @returns the position of the closing '>' of the opening tab
     */
    size_t parse_name(size_t pos)
    {
        size_t orig_pos = pos;
        while( (*doc)[pos] != ' ' && (*doc)[pos] != '>')
            pos++;
        name = doc->substr(orig_pos+1, pos - orig_pos - 1);
        // find the end of the opening tag
        while( (*doc)[pos] != '>')
            ++pos;
        return pos;
    }
    /***
     * parse the attributes
     * @param start_pos where the name is
     * @param end_pos where the end of the opening tag is
     * @return the position of the closing '>'
     */
    size_t parse_attributes(size_t start_pos, size_t end_pos)
    {
        bool in_quotes = false;
        bool in_name = true;
        // fast forward past name
        start_pos = doc->substr(start_pos).find(name) + name.length() + start_pos;
        while((*doc)[start_pos] != '>' && start_pos < end_pos)
        {
            if ((*doc)[start_pos] != ' ')
            {
                attribute* curr_attr = new attribute(start_pos, doc);
                add_attribute(curr_attr);
                start_pos = curr_attr->end_pos;
            }
            start_pos++;
        }
        return start_pos;
    }
};

class query_element
{
    public:
    enum class element_type
    {
        TAG,
        ATTRIBUTE
    };
    query_element(const std::string& in, element_type type) : name(in), type(type) 
    {
    }
    static element_type get_type(char in) { return (in != '~' ? element_type::TAG : element_type::ATTRIBUTE); }
    std::string name;
    element_type type;
};

std::vector<query_element> parse_elements(const std::string& in)
{
    std::vector<query_element> elements;
    std::string curr = in;
    char last_delim = '.';
    while (!curr.empty())
    {
        auto pos = curr.find_first_of(".~");
        if (pos == std::string::npos)
        {
            elements.push_back( query_element(curr, query_element::get_type(last_delim)));
            curr = "";
        }
        else
        {
            elements.push_back( query_element(curr.substr(0, pos), query_element::get_type(last_delim)));
            last_delim = curr[pos];
            curr = curr.substr(pos + 1);
        }
    }
    return elements;
}

std::string get_results(std::shared_ptr<tag> head, std::vector<query_element> elements)
{
    tag* curr_tag = head.get();
    for(size_t i = 0; i < elements.size(); ++i)
    {
        if (curr_tag == nullptr)
            break;
        query_element elem = elements[i];
        if (i == 0 && elem.name == head->name)
            continue;
        if (i == 0 && elem.name != head->name)
        {
            curr_tag = curr_tag->find_sibling(elem.name);
            continue;
        }
        if (elem.type == query_element::element_type::TAG)
        {
            curr_tag = curr_tag->find_child(elem.name);
        }
        else
        {
            // ATTRIBUTE
            attribute* attr = curr_tag->find_attribute(elem.name);
            if (attr == nullptr)
                return "Not Found!";
            return attr->value;
        }
    }
    return "Not Found!";
}

void read_line_numbers(std::istream& stream, int& num_lines, int& num_queries)
{
    std::string line;
    std::getline(stream, line);
    std::stringstream ss(line);
    ss >> num_lines >> num_queries;
    return;
}

std::string read_hrml(std::istream& stream, int num_lines)
{
    std::string hrml;
    for(int i = 0; i < num_lines; ++i)
    {
        std::string line;
        std::getline(stream, line);
        hrml += line;
    }
    return hrml;
}

std::shared_ptr<tag> parse(const std::string& hrml)
{
    std::shared_ptr<tag> head = nullptr;
    tag* curr_tag = nullptr;
    for(size_t i = 0; i < hrml.length(); ++i)
    {
        curr_tag = new tag(i, &hrml);
        i = curr_tag->end_pos;
        if (head == nullptr)
            head = std::shared_ptr<tag>(curr_tag);
        else
            head->add_sibling(curr_tag);
    }
    return head;
}

#ifndef __JMJ_TESTING__

int main(int argc, char** argv)
{
    // get data
    int num_lines; int num_queries;
    read_line_numbers(std::cin, num_lines, num_queries);
    std::string hrml = read_hrml(std::cin, num_lines);
    // parse
    std::shared_ptr<tag> head = parse(hrml); 
    // queries
    for(int i = 0; i < num_queries; ++i)
    {
        std::string line;
        std::getline(std::cin, line);
        auto elements = parse_elements(line);
        std::cout << get_results(head, elements) << std::endl;
    }
}

#endif
