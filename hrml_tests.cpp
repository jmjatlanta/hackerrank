#define __JMJ_TESTING__
#include "hrml.h"
#include <gtest/gtest.h>
#include <fstream>

TEST(hrml_tests, basics)
{
    std::string str = "<test></test>";
    try{
        std::shared_ptr<tag> head = std::make_shared<tag>( 0, &str);
        ASSERT_NE(head, nullptr);
        EXPECT_EQ(head->name, "test");
        EXPECT_EQ(head->end_pos, 12);
    } catch(const parse_exception& pe)
    {
        std::cerr << "Error thrown: " << pe.what() << std::endl;
        FAIL();
    }
}

TEST(hrml_tests, attributes)
{
    class mock_attribute : public attribute
    {
        public:
        mock_attribute(size_t start_pos, const std::string* doc) : attribute(start_pos, doc)
        {
        }
    };

    std::string str = "<test attr_1 = \"blah\">";

    try
    {
        mock_attribute a(5, &str);
        EXPECT_EQ(a.name, "attr_1");
        EXPECT_EQ(a.value, "blah");
        EXPECT_EQ(a.end_pos, 20);
    } catch( const parse_exception& pe)
    {
        FAIL();
    }

    str = "<test attr_1=\"blah\">";
    try
    {
        mock_attribute a(5, &str);
        EXPECT_EQ(a.name, "attr_1");
        EXPECT_EQ(a.value, "blah");
        EXPECT_EQ(a.end_pos, 18);
    } catch( const parse_exception& pe)
    {
        FAIL();
    }
    
    str = "<test atr_1 = \"blah\"></test>";
    try{
        std::shared_ptr<tag> head = std::make_shared<tag>( 0, &str);
        ASSERT_NE(head, nullptr);
        EXPECT_EQ(head->name, "test");
        EXPECT_EQ(head->end_pos, 27);
        EXPECT_NE(head->first_attribute, nullptr);
        EXPECT_EQ(head->first_attribute->name, "atr_1");
        EXPECT_EQ(head->first_attribute->value, "blah");
    } catch(const parse_exception& pe)
    {
        std::cerr << "Error thrown: " << pe.what() << std::endl;
        FAIL();
    }
}

TEST(hrml_tests, children_tags)
{
    std::string str = "<parent><child1><child1_1></child1_1></child1></parent>";
    try {
        tag head(0, &str);
        ASSERT_EQ(head.name, "parent");
        ASSERT_EQ(head.first_child->name, "child1");
        ASSERT_EQ(head.first_child->first_child->name, "child1_1");
    } catch( const parse_exception& pe) {
        std::cerr << "Parse exception: " << pe.what() << std::endl;
        FAIL();
    }
}

TEST(hrml_tests, children_with_siblings)
{
    std::string str = "<parent><child1></child1><child2></child2></parent>";
    try {
        tag head(0, &str);
        ASSERT_EQ(head.name, "parent");
        ASSERT_EQ(head.first_child->name, "child1");
        ASSERT_EQ(head.first_child->first_sibling->name, "child2");
        ASSERT_EQ(head.find_child("child2")->name, "child2");
    } catch( const parse_exception& pe) {
        std::cerr << "Parse exception: " << pe.what() << std::endl;
        FAIL();
    }
}

TEST(hrml_tests, hr_initial)
{
    std::string str = "<tag1 value = \"HelloWorld\">\n<tag2 name = \"Name1\">\n</tag2></tag1>";
    try {
        tag head(0, &str);
        ASSERT_EQ(head.name, "tag1");
        ASSERT_EQ(head.first_attribute->name, "value");
        ASSERT_EQ(head.first_attribute->value, "HelloWorld");
        ASSERT_EQ(head.first_child->name, "tag2");
        ASSERT_EQ(head.first_child->first_attribute->name, "name");
        ASSERT_EQ(head.first_child->first_attribute->value, "Name1");
    } catch( const parse_exception& pe) {
        std::cerr << "Parse exception: " << pe.what() << std::endl;
        FAIL();
    }
}

TEST(hrml_tests, query)
{
    std::string str = "<tag1 value = \"HelloWorld\">\n<tag2 name = \"Name1\">\n</tag2></tag1>";
    std::shared_ptr<tag> head = std::make_shared<tag>(0, &str);
    std::string in = "tag1.tag2~name";
    auto vec = parse_elements(in);
    EXPECT_EQ(vec.size(), 3);
    query_element e = vec[0];
    EXPECT_EQ(e.type, query_element::element_type::TAG);
    EXPECT_EQ(e.name, "tag1");
    e = vec[1];
    EXPECT_EQ(e.type, query_element::element_type::TAG);
    EXPECT_EQ(e.name, "tag2");
    e = vec[2];
    EXPECT_EQ(e.type, query_element::element_type::ATTRIBUTE);
    EXPECT_EQ(e.name, "name");
    EXPECT_EQ(get_results(head, vec), "Name1");
    in = "tag1~name";
    EXPECT_EQ(get_results(head, parse_elements(in)), "Not Found!");
    in = "tag1~value";
    EXPECT_EQ(get_results(head, parse_elements(in)), "HelloWorld");
}

TEST(hrml_tests, case1)
{
    std::string str = "<a value = \"GoodVal\">\n<b value = \"BadVal\" size = \"10\">\n</b>\n<c height = \"auto\">\n<d size = \"3\">\n<e strength = \"2\">\n</e>\n</d>\n</c>\n</a>\n";
    std::shared_ptr<tag> head = std::make_shared<tag>(0, &str);
    EXPECT_EQ(get_results(head, parse_elements("a~value")), "GoodVal");
    EXPECT_EQ(get_results(head, parse_elements("b~value")), "Not Found!");
    auto vec = parse_elements("a.b~size");
    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(get_results(head, vec), "10");
    EXPECT_EQ(get_results(head, parse_elements("a.b~value")), "BadVal");
    EXPECT_EQ(get_results(head, parse_elements("a.b.c~height")), "Not Found!");
    EXPECT_EQ(get_results(head, parse_elements("a.c~height")), "auto");
    EXPECT_EQ(get_results(head, parse_elements("a.d.e~strength")), "Not Found!");
    EXPECT_EQ(get_results(head, parse_elements("a.c.d.e~strength")), "2");
    EXPECT_EQ(get_results(head, parse_elements("d~sze")), "Not Found!");
    EXPECT_EQ(get_results(head, parse_elements("a.c.d~size")), "3");
}

TEST(hrml_tests, case4)
{
    std::ifstream in("hrml_case4.txt");
    int num_lines; int num_queries;
    read_line_numbers(in, num_lines, num_queries);
    std::string hrml = read_hrml(in, num_lines);
    auto head = parse(hrml);
    ASSERT_NE(head, nullptr);
    // queries
    std::string line;
    std::getline(in, line);
    auto vec = parse_elements(line);
    EXPECT_EQ( get_results(head, vec), "123");
    std::getline(in, line);
    EXPECT_EQ( get_results(head, parse_elements(line)), "43.4");
    std::getline(in, line);
    EXPECT_EQ( get_results(head, parse_elements(line)), "hello");
    std::getline(in, line);
    EXPECT_EQ( get_results(head, parse_elements(line)), "Not Found!");
    std::getline(in, line);
    EXPECT_EQ( get_results(head, parse_elements(line)), "Hello");
    std::getline(in, line);
    EXPECT_EQ( get_results(head, parse_elements(line)), "Universe!");
    std::getline(in, line);
    EXPECT_EQ( get_results(head, parse_elements(line)), "World!");
    std::getline(in, line);
    EXPECT_EQ( get_results(head, parse_elements(line)), "New");
    std::getline(in, line);
    EXPECT_EQ( get_results(head, parse_elements(line)), "Not Found!");
    std::getline(in, line);
    EXPECT_EQ( get_results(head, parse_elements(line)), "Not Found!");
    std::getline(in, line);
    EXPECT_EQ( get_results(head, parse_elements(line)), "34");
    std::getline(in, line);
    EXPECT_EQ( get_results(head, parse_elements(line)), "9.845");
    std::getline(in, line);
    EXPECT_EQ( get_results(head, parse_elements(line)), "Not Found!");
    std::getline(in, line);
    EXPECT_EQ( get_results(head, parse_elements(line)), "Not Found!");
}