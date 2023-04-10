#include <string>
#include <iostream>

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
            if ( (*orig)[i] == ' ' && name.empty())
                name = orig->substr(pos, i-1);
            if ( (*orig)[i] == ' ' && !name.empty())
                continue;
            if ( (*orig)[i] == '"')
            {
                in_quotes = !in_quotes;
                if (in_quotes)
                    val_start = i + 1;
                else
                {
                    value = orig->substr(val_start, i);
                    break;
                }
            }
            if (!in_quotes)
            {
            }
        }
        if (name.empty())
            throw parse_exception("No name retrieved from attribute" + orig->substr(pos));
    }
    ~attribute() {
        // delete next_sibling
    }
    void add_sibling(attribute* in)
    {
        if (next_sibling == nullptr)
            next_sibling = in;
        else
            next_sibling->add_sibling(in);
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
    tag(size_t pos, const std::string* orig, int depth, tag* parent) : start_pos(pos), doc(orig), depth(depth) 
    {
        // parse the incoming string until closing tag completed
        size_t curr_pos = parse_name(pos);
        curr_pos = parse_attributes(pos, curr_pos);
        for(size_t i = curr_pos; i < orig->length(); ++i)
        {
            // work on children
            if ((*orig)[i] != '<')
                throw parse_exception("No opening brace found: " + orig->substr(i));
            if ((*orig)[i+1] == '/')
            {
                // we are at the end tag for this tag
                end_pos = orig->substr(i+1).find(">") + i + 1 + pos;
                break;
            }
            add_child(new tag(i, orig, depth + 1, this));
        }
        if (parent != nullptr)
            parent->add_sibling(this);
    }
    ~tag()
    {
        // destroy first_sibling
        // destroy first_child
        // destroy first_attribute
    };
    void add_sibling(tag* in)
    {
        if (first_sibling == nullptr)
            first_sibling = in;
        else
            first_sibling->add_sibling(in);
    }
    void add_child(tag* in)
    {
        if (first_child == nullptr)
            first_child = in;
        else
            first_child->add_sibling(in);
    }
    void add_attribute(attribute* in)
    {
        if (first_attribute == nullptr)
            first_attribute = in;
        else
            first_attribute->add_sibling(in);
    }
    size_t start_pos;
    size_t end_pos = 0;
    size_t open_tag_end = -1;
    int depth = 0;
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
     * @returns the position of the first space in the opening tab
     */
    size_t parse_name(size_t pos)
    {
        size_t orig_pos = pos;
        while( (*doc)[pos] != ' ' && (*doc)[pos] != '>')
            pos++;
        name = doc->substr(orig_pos+1, pos-1);
        return pos;
    }
    /***
     * parse the attributes
     * @param start_pos where the name is
     * @param end_pos where the end of the opening tag is
     * @return the position after the closing '>'
     */
    size_t parse_attributes(size_t start_pos, size_t end_pos)
    {
        bool in_quotes = false;
        bool in_name = true;
        // fast forward past name
        start_pos = doc->substr(start_pos).find(name) + name.length();
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
        return start_pos + 1;
    }
};

#ifndef __JMJ_TESTING__

int main()
{
    // get data
    int num_lines; int num_queries;
    std::cin >> num_lines >> num_queries;
    std::string hrml;
    for(int i = 0; i < num_lines; ++i)
    {
        std::string line;
        std::cin >> line; 
        hrml += line;
    }
    // parse
    tag* head = nullptr;
    tag* curr_tag = nullptr;
    for(size_t i = 0; i < hrml.length(); ++i)
    {
        curr_tag = new tag(0, &hrml, 0, nullptr);
        i = curr_tag->end_pos;
        if (head == nullptr)
            head = curr_tag;
        else
            head->add_sibling(curr_tag);
    }
    // queries
}

#endif
