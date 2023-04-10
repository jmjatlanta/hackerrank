#define __JMJ_TESTING__
#include "hrml.h"
#include <gtest/gtest.h>

TEST(hrml_tests, basics)
{
    std::string str = "<test></test>";
    try{
        std::shared_ptr<tag> head = std::make_shared<tag>( 0, &str, 0, nullptr);
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
    std::string str = "<test atr_1 = \"blah\"></test>";
    try{
        std::shared_ptr<tag> head = std::make_shared<tag>( 0, &str, 0, nullptr);
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

