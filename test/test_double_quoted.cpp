#include "./test_group.hpp"

namespace c4 {
namespace yml {

TEST(double_quoted, test_suite_3RLN)
{
    csubstr yaml = R"(---
"1 leading
   \ttab"
---
"2 leading
    \	tab"
---
"3 leading
    	tab"
---
"4 leading
    \t  tab"
---
"5 leading
    \	  tab"
---
"6 leading
    	  tab"
)";
    test_check_emit_check(yaml, [](Tree const &t){
        EXPECT_EQ(t.docref(0).val(), "1 leading \ttab");
        EXPECT_EQ(t.docref(1).val(), "2 leading \ttab");
        EXPECT_EQ(t.docref(2).val(), "3 leading tab");
        EXPECT_EQ(t.docref(3).val(), "4 leading \t  tab");
        EXPECT_EQ(t.docref(4).val(), "5 leading \t  tab");
        EXPECT_EQ(t.docref(5).val(), "6 leading tab");
    });
}

TEST(double_quoted, test_suite_5GBF)
{
    csubstr yaml = R"(
Folding:
  "Empty line

  as a line feed"
Folding2:
  "Empty line

  as a line feed"
Folding3:
  "Empty line

  as a line feed"
)";
    test_check_emit_check(yaml, [](Tree const &t){
        ASSERT_TRUE(t.rootref().is_map());
        EXPECT_EQ(t["Folding"].val(), csubstr("Empty line\nas a line feed"));
        EXPECT_EQ(t["Folding2"].val(), csubstr("Empty line\nas a line feed"));
        EXPECT_EQ(t["Folding3"].val(), csubstr("Empty line\nas a line feed"));
    });
}

TEST(double_quoted, test_suite_6SLA)
{
    csubstr yaml = R"(
"foo\nbar:baz\tx \\$%^&*()x": 23
'x\ny:z\tx $%^&*()x': 24
)";
    test_check_emit_check(yaml, [](Tree const &t){
        ASSERT_TRUE(t.rootref().is_map());
        ASSERT_TRUE(t.rootref().has_child("foo\nbar:baz\tx \\$%^&*()x"));
        ASSERT_TRUE(t.rootref().has_child("x\\ny:z\\tx $%^&*()x"));
        ASSERT_EQ(t["foo\nbar:baz\tx \\$%^&*()x"].val(), csubstr("23"));
        ASSERT_EQ(t["x\\ny:z\\tx $%^&*()x"].val(), csubstr("24"));
    });
}

TEST(double_quoted, test_suite_6WPF)
{
    csubstr yaml = R"(
"
  foo 
 
    bar

  baz
"
)";
    test_check_emit_check(yaml, [](Tree const &t){
        ASSERT_TRUE(t.rootref().is_val());
        EXPECT_EQ(t.rootref().val(), csubstr(" foo\nbar\nbaz "));
    });
}

TEST(double_quoted, test_suite_9TFX)
{
    csubstr yaml = R"(
" 1st non-empty

 2nd non-empty 
 3rd non-empty "
)";
    test_check_emit_check(yaml, [](Tree const &t){
        ASSERT_TRUE(t.rootref().is_val());
        EXPECT_EQ(t.rootref().val(), csubstr(" 1st non-empty\n2nd non-empty 3rd non-empty "));
    });
}

#ifdef NOT_YET
TEST(double_quoted, test_suite_G4RS)
{
    csubstr yaml = R"(
unicode: "Sosa did fine.\u263A"
control: "\b1998\t1999\t2000\n"
hex esc: "\x0d\x0a is \r\n"

single: '"Howdy!" he cried.'
quoted: ' # Not a ''comment''.'
tie-fighter: '|\-*-/|'
)";
    test_check_emit_check(yaml, [](Tree const &t){
        //EXPECT_EQ(t["unicode"].val()    , csubstr(R"(Sosa did fine.☺)")); // no conversion yet
        EXPECT_EQ(t["unicode"].val()    , csubstr(R"(Sosa did fine.\u263A)"));
        EXPECT_EQ(t["control"].val()    , csubstr("\b1998\t1999\t2000\n"));
        //EXPECT_EQ(t["hex esc"].val()    , csubstr("\r\n is \r\n"));
        EXPECT_EQ(t["hex esc"].val().len, csubstr("\\x0d\\x0a is \r\n").len);
        EXPECT_EQ(t["hex esc"].val()    , csubstr("\\x0d\\x0a is \r\n"));
        EXPECT_EQ(t["single"].val()     , csubstr(R"("Howdy!" he cried.)"));
        EXPECT_EQ(t["quoted"].val()     , csubstr(R"( # Not a 'comment'.)"));
        EXPECT_EQ(t["tie-fighter"].val(), csubstr(R"(|\-*-/|)"));
    });
}
#endif

TEST(double_quoted, test_suite_KSS4)
{
    csubstr yaml = R"(
---
"quoted
string"
--- "quoted
string"
---
- "quoted
  string"
---
- "quoted
string"
---
"quoted
  string": "quoted
  string"
---
"quoted
string": "quoted
string"
)";
    test_check_emit_check(yaml, [](Tree const &t){
        EXPECT_EQ(t.docref(0).val(), "quoted string");
        EXPECT_EQ(t.docref(1).val(), "quoted string");
        EXPECT_EQ(t.docref(2)[0].val(), "quoted string");
        EXPECT_EQ(t.docref(3)[0].val(), "quoted string");
        EXPECT_EQ(t.docref(4)["quoted string"].val(), "quoted string");
        EXPECT_EQ(t.docref(5)["quoted string"].val(), "quoted string");
    });
}

TEST(double_quoted, test_suite_NAT4)
{
    csubstr yaml = R"(
a: '
  '
b: '  
  '
c: "
  "
d: "  
  "
e: '

  '
f: "

  "
g: '


  '
h: "


  "
)";
    test_check_emit_check(yaml, [](Tree const &t){
        EXPECT_EQ(t["a"].val(), csubstr(" "));
        EXPECT_EQ(t["b"].val(), csubstr(" "));
        EXPECT_EQ(t["c"].val(), csubstr(" "));
        EXPECT_EQ(t["d"].val(), csubstr(" "));
        EXPECT_EQ(t["e"].val(), csubstr("\n"));
        EXPECT_EQ(t["f"].val(), csubstr("\n"));
        EXPECT_EQ(t["g"].val(), csubstr("\n\n"));
        EXPECT_EQ(t["h"].val(), csubstr("\n\n"));
    });
}

TEST(double_quoted, test_suite_NP9H)
{
    csubstr yaml = R"(
"folded 
to a space,	
 
to a line feed, or 	\
 \ 	non-content"
)";
    test_check_emit_check(yaml, [](Tree const &t){
        ASSERT_TRUE(t.rootref().is_val());
        EXPECT_EQ(t.rootref().val(), csubstr("folded to a space,\nto a line feed, or \t \tnon-content"));
    });
}

TEST(double_quoted, test_suite_Q8AD)
{
    csubstr yaml = R"(
"folded 
to a space,
 
to a line feed, or 	\
 \ 	non-content"
)";
    test_check_emit_check(yaml, [](Tree const &t){
        ASSERT_TRUE(t.rootref().is_val());
        EXPECT_EQ(t.rootref().val(), csubstr("folded to a space,\nto a line feed, or \t \tnon-content"));
    });
}

TEST(double_quoted, test_suite_R4YG)
{
    csubstr yaml = R"(
- "	

detected

"

)";
    test_check_emit_check(yaml, [](Tree const &t){
        EXPECT_EQ(t[0].val(), csubstr("\t\ndetected\n"));
    });
}


//-----------------------------------------------------------------------------

void verify_error_is_reported(csubstr case_name, csubstr yaml, Location loc={})
{
    SCOPED_TRACE(case_name);
    SCOPED_TRACE(yaml);
    Tree tree;
    ExpectError::do_check(&tree, [&](){
        parse_in_arena(yaml, &tree);
    }, loc);
}

TEST(double_quoted, error_on_unmatched_quotes)
{
    verify_error_is_reported("map block", R"(foo: "'
bar: "")");
    verify_error_is_reported("seq block", R"(- "'
- "")");
    verify_error_is_reported("map flow", R"({foo: "', bar: ""})");
    verify_error_is_reported("seq flow", R"(["', ""])");
}

TEST(double_quoted, error_on_unmatched_quotes_with_escapes)
{
    verify_error_is_reported("map block", R"(foo: "\"'
bar: "")");
    verify_error_is_reported("seq block", R"(- "\"'
- "")");
    verify_error_is_reported("map flow", R"({foo: "\"', bar: ""})");
    verify_error_is_reported("seq flow", R"(["\"', ""])");
}

TEST(double_quoted, error_on_unmatched_quotes_at_end)
{
    verify_error_is_reported("map block", R"(foo: ""
bar: "')");
    verify_error_is_reported("seq block", R"(- ""
- "')");
    verify_error_is_reported("map flow", R"({foo: "", bar: "'})");
    verify_error_is_reported("seq flow", R"(["", "'])");
}

TEST(double_quoted, error_on_unmatched_quotes_at_end_with_escapes)
{
    verify_error_is_reported("map block", R"(foo: ""
bar: "\"')");
    verify_error_is_reported("seq block", R"(- ""
- "\"')");
    verify_error_is_reported("map flow", R"({foo: "", bar: "\"'})");
    verify_error_is_reported("seq flow", R"(["", "\"'])");
}

TEST(double_quoted, error_on_unclosed_quotes)
{
    verify_error_is_reported("map block", R"(foo: ",
bar: what)");
    verify_error_is_reported("seq block", R"(- "
- what)");
    verify_error_is_reported("map flow", R"({foo: ", bar: what})");
    verify_error_is_reported("seq flow", R"([", what])");
}

TEST(double_quoted, error_on_unclosed_quotes_with_escapes)
{
    verify_error_is_reported("map block", R"(foo: "\",
bar: what)");
    verify_error_is_reported("seq block", R"(- "\"
- what)");
    verify_error_is_reported("map flow", R"({foo: "\", bar: what})");
    verify_error_is_reported("seq flow", R"(["\", what])");
}

TEST(double_quoted, error_on_unclosed_quotes_at_end)
{
    verify_error_is_reported("map block", R"(foo: what
bar: ")");
    verify_error_is_reported("seq block", R"(- what
- ")");
    verify_error_is_reported("map flow", R"({foo: what, bar: "})");
    verify_error_is_reported("seq flow", R"([what, "])");
}

TEST(double_quoted, error_on_unclosed_quotes_at_end_with_escapes)
{
    verify_error_is_reported("map block", R"(foo: what
bar: "\")");
    verify_error_is_reported("seq block", R"(- what
- "\")");
    verify_error_is_reported("map flow", R"({foo: what, bar: "\"})");
    verify_error_is_reported("seq flow", R"([what, "\"])");
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#define DOUBLE_QUOTED_CASES                             \
            "dquoted, only text",                       \
            "dquoted, with single quotes",              \
            "dquoted, with double quotes",              \
            "dquoted, with single and double quotes",   \
            "dquoted, with escapes",                    \
            "dquoted, with newline",                    \
            "dquoted, with tabs",                       \
            "dquoted, with tabs 4ZYM",                  \
            "dquoted, with tabs 7A4E",                  \
            "dquoted, with tabs TL85",                  \
            "dquoted, all",                             \
            "dquoted, empty",                           \
            "dquoted, blank",                           \
            "dquoted, numbers",                         \
            "dquoted, trailing space",                  \
            "dquoted, leading space",                   \
            "dquoted, trailing and leading space",      \
            "dquoted, 1 dquote",                        \
            "dquoted, 2 dquotes",                       \
            "dquoted, 3 dquotes",                       \
            "dquoted, 4 dquotes",                       \
            "dquoted, example 2",                       \
            "dquoted, example 2.1"

CASE_GROUP(DOUBLE_QUOTED)
{
    APPEND_CASES(

C("dquoted, only text",
R"("Some text without any quotes."
)",
  N(DOCVAL | VALQUO, "Some text without any quotes.")
),

C("dquoted, with single quotes",
R"("Some text 'with single quotes'")",
  N(DOCVAL|VALQUO, "Some text 'with single quotes'")
),

C("dquoted, with double quotes",
R"("Some \"text\" \"with double quotes\"")",
  N(DOCVAL|VALQUO, "Some \"text\" \"with double quotes\"")
),

C("dquoted, with single and double quotes",
R"("Some text 'with single quotes' \"and double quotes\".")",
  N(DOCVAL|VALQUO, "Some text 'with single quotes' \"and double quotes\".")
),

C("dquoted, with escapes",
R"("Some text with escapes \\n \\r \\t")",
  N(DOCVAL|VALQUO, "Some text with escapes \\n \\r \\t")
),

C("dquoted, with newline",
R"("Some text with\nnewline")",
  N(DOCVAL|VALQUO, "Some text with\nnewline")
),

C("dquoted, with tabs",
R"("\tSome\ttext\twith\ttabs\t")",
  N(DOCVAL|VALQUO, "\tSome\ttext\twith\ttabs\t")
),

C("dquoted, with tabs 4ZYM",
R"(plain: text
  lines
quoted: "text
  	lines"
block: |
  text
   	lines
)",
  L{N("plain", "text lines"), N(KEYVAL|VALQUO, "quoted", "text lines"), N(KEYVAL|VALQUO,"block", "text\n \tlines\n")}
),

C("dquoted, with tabs 7A4E",
R"(" 1st non-empty

 2nd non-empty 
	3rd non-empty ")",
  N(DOCVAL|VALQUO, " 1st non-empty\n2nd non-empty 3rd non-empty ")
),

C("dquoted, with tabs TL85",
R"("
  foo 
 
  	 bar

  baz
")", N(DOCVAL|VALQUO, " foo\nbar\nbaz ")),

C("dquoted, all",
R"("Several lines of text,
containing 'single quotes' and \"double quotes\". \
Escapes (like \\n) work.\nIn addition,
newlines can be esc\
aped to prevent them from being converted to a space.

Newlines can also be added by leaving a blank line.
    Leading whitespace on lines is ignored."
)",
  N(DOCVAL|VALQUO, "Several lines of text, containing 'single quotes' and \"double quotes\". Escapes (like \\n) work.\nIn addition, newlines can be escaped to prevent them from being converted to a space.\nNewlines can also be added by leaving a blank line. Leading whitespace on lines is ignored.")
),

C("dquoted, empty",
R"("")",
  N(DOCVAL|VALQUO, "")
),

C("dquoted, blank",
R"(
- ""
- " "
- "  "
- "   "
- "    "
)",
  L{N(QV, ""), N(QV, " "), N(QV, "  "), N(QV, "   "), N(QV, "    ")}
),

C("dquoted, numbers", // these should not be quoted when emitting
R"(
- -1
- -1.0
- +1.0
- 1e-2
- 1e+2
)",
  L{N("-1"), N("-1.0"), N("+1.0"), N("1e-2"), N("1e+2")}
),

C("dquoted, trailing space",
R"('a aaaa  ')",
  N(DOCVAL|VALQUO, "a aaaa  ")
),

C("dquoted, leading space",
R"('  a aaaa')",
  N(DOCVAL|VALQUO, "  a aaaa")
),

C("dquoted, trailing and leading space",
R"('  012345  ')",
  N(DOCVAL|VALQUO, "  012345  ")
),

C("dquoted, 1 dquote",
R"("\"")",
  N(DOCVAL|VALQUO, "\"")
),

C("dquoted, 2 dquotes",
R"("\"\"")",
  N(DOCVAL|VALQUO, "\"\"")
),

C("dquoted, 3 dquotes",
R"("\"\"\"")",
  N(DOCVAL|VALQUO, "\"\"\"")
),

C("dquoted, 4 dquotes",
R"("\"\"\"\"")",
  N(DOCVAL|VALQUO, "\"\"\"\"")
),

C("dquoted, 5 dquotes",
R"("\"\"\"\"\"")",
  N(DOCVAL|VALQUO, "\"\"\"\"\"")
),

C("dquoted, example 2",
R"("This is a key\nthat has multiple lines\n": and this is its value
)",
  L{N(QK, "This is a key\nthat has multiple lines\n", "and this is its value")}
),

C("dquoted, example 2.1",
R"("This is a key

that has multiple lines

": and this is its value
)",
  L{N(QK, "This is a key\nthat has multiple lines\n", "and this is its value")}
),
    )
}

INSTANTIATE_GROUP(DOUBLE_QUOTED)

} // namespace yml
} // namespace c4
