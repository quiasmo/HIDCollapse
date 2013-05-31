//
//  HIDCollapseParser.cpp
//  HIDCollapse
//
//  Created by Juan Carlos Borda on 5/29/13.
//  Copyright (c) 2013 Juan Carlos Borda. All rights reserved.
//

//#include "HIDCollapseParser.h"
#define BOOST_SPIRIT_DEBUG
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/include/qi.hpp>

#include <boost/spirit/include/support_multi_pass.hpp>
#include <boost/spirit/include/classic_position_iterator.hpp>

#include <string>
#include <sstream>
#include <iomanip>
#include <fstream>

namespace qi = boost::spirit::qi;

namespace ast
{
 
    struct elemHexPairKey
    {
        int page;
        int usage;
    };
    
    typedef boost::variant<
    elemHexPairKey,
    std::string,
    int
    > sourceElementKey;
    
    struct expression
    {
        sourceElementKey key;
    };
    
    struct deviceTriplet
    {
        int vendor;
        int product;
        int version;
    };
    
    struct devicePair
    {
        int vendor;
        int product;
    };

    typedef boost::variant<
    std::string,
    deviceTriplet,
    devicePair> device;
    
    typedef boost::variant<
    std::string,
    int
    > targetElementKey;
    
    typedef std::list< targetElementKey > targetElementKeys;
    
    struct indexAxe
    {
        targetElementKeys keys;
    };
    
    struct indexButton
    {
        targetElementKeys keys;
    };
    
    typedef boost::variant< indexAxe, indexButton> indexTarget;
    
    
    struct entry
    {
        expression expression;
        indexTarget indexTarget;
    };
    
    typedef std::list<entry> entries;
        
    struct hidCollapse
    {
        device device;
        std::string index;
        entries entries;
    };
    
    typedef std::list<hidCollapse> hidCollapseList;
    
}

BOOST_FUSION_ADAPT_STRUCT(
                          ast::elemHexPairKey,
                          (int, page)
                          (int, usage)
                          );

BOOST_FUSION_ADAPT_STRUCT(
                          ast::deviceTriplet,
                          (int, vendor)
                          (int, product)
                          (int, version)
                          );
BOOST_FUSION_ADAPT_STRUCT(
                          ast::devicePair,
                          (int , vendor)
                          (int , product )
                          );

BOOST_FUSION_ADAPT_STRUCT(
                          ast::indexAxe,
                          ( ast::targetElementKeys , keys )
                          );

BOOST_FUSION_ADAPT_STRUCT(
                          ast::indexButton,
                          ( ast::targetElementKeys , keys )
                          );

BOOST_FUSION_ADAPT_STRUCT(ast::entry,
                          (ast::expression, expression )
                          (ast::indexTarget, indexTarget )
                          );

BOOST_FUSION_ADAPT_STRUCT(ast::expression,
                          (ast::sourceElementKey, key)
                          );

BOOST_FUSION_ADAPT_STRUCT(ast::hidCollapse,
                          (ast::device , device )
                          (std::string , index )
                          (ast::entries, entries )
                          );

namespace HIDCollapse
{
    
    template <
    typename Iterator ,
    typename Skipper >
    struct unescaped_string
    : qi::grammar<Iterator, std::string(char const*), Skipper >
    {
        unescaped_string()
        : unescaped_string::base_type(unesc_str)
        {
            
            unesc_char.add("\\a", '\a')("\\b", '\b')("\\f", '\f')("\\n", '\n')
            ("\\r", '\r')("\\t", '\t')("\\v", '\v')("\\\\", '\\')
            ("\\\'", '\'')("\\\"", '\"')
            ;
            
            unesc_str %=  qi::lexeme[qi::lit(qi::_r1)
                                     >>  *(unesc_char | qi::alnum | (qi::space - qi::eol) )
                         >>  qi::lit(qi::_r1)]
            ;
        }
        
        qi::rule<Iterator, std::string(char const*), Skipper> unesc_str;
        qi::symbols<char const, char const> unesc_char;
    };
        
    template <
    typename Iterator,
    typename Skipper 
    >
    struct main_grammar
    : qi::grammar<Iterator, ast::hidCollapseList() , Skipper >
    {
        main_grammar()
        : main_grammar::base_type( hidcollapseList )
        {
            using namespace qi;
           
            stringLit       = us(singleQuote) | us(doubleQuote) ;
            
            hexadecimal     = "0x" >> hex ;
                        
            elemHexPairKey  = hexadecimal > "," > hexadecimal;
            devicePair      = hexadecimal > "," > hexadecimal;
            deviceTriplet   = hexadecimal > "," > hexadecimal > "," > hexadecimal;
            
            elementKey      = stringLit | elemHexPairKey | int_ ;
            
            indexKey        = stringLit | int_;
            indexKeys       = indexKey % ",";
            button          = lit("button") >> "(" >> indexKeys >> ")";
            axis            = (lit("axe")|lit("axis"))   >> "(" >> indexKeys >> ")";
            
            indexTarget     = button | axis;
            
            expression      = lit("elem")   >> "(" >> elementKey >>")" ;
            entry           = expression >> ":" >> indexTarget ;
            
            index           = lit("index") >> "(" >> stringLit >> ")";
            device           = lit("device") >> "(" >>  (stringLit | deviceTriplet | devicePair) >> ")";
            hidcollapse      =  lit("map") >> device >> lit("to") >> index >> "{" >> * entry >> "}";
            hidcollapseList  = * hidcollapse;
           
           
            BOOST_SPIRIT_DEBUG_NODE( hexadecimal );
            BOOST_SPIRIT_DEBUG_NODE( stringLit );
            BOOST_SPIRIT_DEBUG_NODE( index );
            BOOST_SPIRIT_DEBUG_NODE( expression );
            BOOST_SPIRIT_DEBUG_NODE( elementKey );
            BOOST_SPIRIT_DEBUG_NODE( button );
            BOOST_SPIRIT_DEBUG_NODE( axis );
            BOOST_SPIRIT_DEBUG_NODE( indexKey );
            BOOST_SPIRIT_DEBUG_NODE( indexKeys );
            BOOST_SPIRIT_DEBUG_NODE( indexTarget );
            BOOST_SPIRIT_DEBUG_NODE( entry );
            BOOST_SPIRIT_DEBUG_NODE( entries );
            BOOST_SPIRIT_DEBUG_NODE( device );
            BOOST_SPIRIT_DEBUG_NODE( devicePair );
            BOOST_SPIRIT_DEBUG_NODE( deviceTriplet );
            BOOST_SPIRIT_DEBUG_NODE( elemHexPairKey );
            BOOST_SPIRIT_DEBUG_NODE( hidcollapse );
            BOOST_SPIRIT_DEBUG_NODE( hidcollapseList );
        }

        qi::rule< Iterator , ast::hidCollapseList()  , Skipper > hidcollapseList;
        qi::rule< Iterator , ast::hidCollapse()  , Skipper > hidcollapse;
        qi::rule< Iterator , ast::elemHexPairKey()   , Skipper > elemHexPairKey;
        qi::rule< Iterator , ast::devicePair()   , Skipper > devicePair;
        qi::rule< Iterator , ast::deviceTriplet()   , Skipper > deviceTriplet;
        qi::rule< Iterator , ast::device()  , Skipper > device;
        qi::rule< Iterator , ast::entries()  , Skipper > entries;
        qi::rule< Iterator , ast::entry()  , Skipper > entry;
        qi::rule< Iterator , ast::indexTarget()  , Skipper > indexTarget;
        qi::rule< Iterator , ast::targetElementKeys()  , Skipper > indexKeys;
        qi::rule< Iterator , ast::targetElementKey()  , Skipper > indexKey;
        qi::rule< Iterator , ast::indexAxe()  , Skipper > axis;
        qi::rule< Iterator , ast::indexButton()  , Skipper > button;
        qi::rule< Iterator , ast::sourceElementKey() , Skipper> elementKey;
        qi::rule< Iterator , ast::expression()   , Skipper > expression;
        qi::rule< Iterator , std::string()  , Skipper > index;
        qi::rule< Iterator , std::string() , Skipper > stringLit;
        qi::rule< Iterator , int , Skipper > hexadecimal;
        
        unescaped_string<Iterator, Skipper> us;
        const char * singleQuote ="'";
        const char * doubleQuote ="\"";
    };
}

//for printing variants
/*
struct elem_printer
: public boost::static_visitor<>
{
    void
    operator()(ast::elem const & usage) const
    {
        std::cout << "elem("
        << usage.page
        << ", "
        << usage.usage
        << ")"
        << std::endl;
    }
    
    void
    operator()(ast::Index const & index) const
    {
        std::cout << "elem("
        << index.seq
        << ")"
        << std::endl;
    }
};
*/


void dump_stream_stdout( std::istream& input )
{
    //input.seekg(0, std::istream::beg);
    std::istream_iterator<uint8_t> begin( input ) , end;
    std::cout << "-------------------------\n";
    std::cout << "Attempting to parse: " << std::endl;
    std::ostream_iterator<uint8_t> dest( std::cout );
    std::copy( begin, end, dest );
    std::cout << "\n-------------------------\n";
    input.seekg(0, std::istream::beg);
    //input >> std::noskipws;
}

void parse_input()
{
	std::cout << "Give me something to parse" << std::endl ;
    char buff [1024];
    std::cin.getline(buff, 1024, '\n');
    std::string str( buff );
    
    typedef std::string::const_iterator iterator;
    iterator begin = str.begin(),
    end = str.end();
    
    using namespace HIDCollapse;
    ast::hidCollapseList out;

    
    bool r = qi:: phrase_parse( begin ,
                               end ,
                               main_grammar<std::string::const_iterator, boost::spirit::ascii::space_type >(),
                               boost::spirit::ascii::space ,
                               out );
     
    if( r&& begin == end )
    {
        std::cout << "parsed successfuly" << std::endl;
    }
    else
    {
        std::cout << "incomplete parse" << std::endl;
    }
    
}

void parse_file(std::string filename)
{
    std::ifstream input( filename.c_str() );
    input >> std::noskipws;

    // iterate over stream input
    typedef std::istreambuf_iterator<char> base_iterator_type;
    base_iterator_type in_begin(input);
    
    // convert input iterator to forward iterator, usable by spirit parser
    typedef boost::spirit::multi_pass<base_iterator_type> forward_iterator_type;
    forward_iterator_type fwd_begin = boost::spirit::make_default_multi_pass(in_begin);
    forward_iterator_type fwd_end;
    
    // wrap forward iterator with position iterator, to record the position
    typedef boost::spirit::classic::position_iterator2<forward_iterator_type> pos_iterator_type;
    pos_iterator_type position_begin(fwd_begin, fwd_end, filename);
    pos_iterator_type position_end;

    using namespace HIDCollapse;
    ast::hidCollapseList out;
    
    bool r = qi::phrase_parse( position_begin ,
                              position_end ,
                              main_grammar<pos_iterator_type,boost::spirit::ascii::space_type>(),
                              boost::spirit::ascii::space,
                              out );
    
    if( r && position_begin == position_end )
    {
        std::cout << "parsed successfuly" << std::endl;
    }
    else
    {
        std::cout << "incomplete parse" << std::endl;
    }
    
}

int
main(int argc, char*argv[])
{
    if(argc < 2)
    {
        while( true )
        {
            parse_input();
            std::cout << "Parsed Input Succesfuly." << std::endl;
        }
    }
    else
    {
        try
        {
            parse_file(argv[1]);
        }
        catch(const std::exception& e)
        {
            std::cerr << "Exception: " << e.what() << std::endl;
        }
        
    }

}