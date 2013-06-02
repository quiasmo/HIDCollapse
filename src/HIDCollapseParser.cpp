/*
 The MIT License (MIT)
 
 Copyright (c) 2013 Juan Borda
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 */

//#include "HIDCollapseParser.h"

//uncomment to debug the grammar
//#define BOOST_SPIRIT_DEBUG

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/include/qi.hpp>

#include <boost/spirit/include/support_multi_pass.hpp>
#include <boost/spirit/include/classic_position_iterator.hpp>

#include <iomanip>
#include <fstream>

#include "HIDCollapseParser.h"

namespace qi = boost::spirit::qi;

BOOST_FUSION_ADAPT_STRUCT(
                          HIDCollapse::ast::elemHexPairKey,
                          (int, page)
                          (int, usage)
                          );

BOOST_FUSION_ADAPT_STRUCT(
                          HIDCollapse::ast::deviceTriplet,
                          (int, vendor)
                          (int, product)
                          (int, version)
                          );
BOOST_FUSION_ADAPT_STRUCT(
                          HIDCollapse::ast::devicePair,
                          (int , vendor)
                          (int , product )
                          );

BOOST_FUSION_ADAPT_STRUCT(
                          HIDCollapse::ast::indexAxe,
                          ( HIDCollapse::ast::targetElementKeys , keys )
                          );

BOOST_FUSION_ADAPT_STRUCT(
                          HIDCollapse::ast::indexButton,
                          ( HIDCollapse::ast::targetElementKeys , keys )
                          );

BOOST_FUSION_ADAPT_STRUCT(HIDCollapse::ast::entry,
                          (HIDCollapse::ast::expression, expression )
                          (HIDCollapse::ast::indexTarget, indexTarget )
                          );

BOOST_FUSION_ADAPT_STRUCT(HIDCollapse::ast::expression,
                          (HIDCollapse::ast::sourceElementKey, key)
                          );

BOOST_FUSION_ADAPT_STRUCT(HIDCollapse::ast::hidCollapse ,
                          (HIDCollapse::ast::device , device )
                          (std::string , index )
                          (HIDCollapse::ast::entries, entries )
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
            
            //uncomment BOOST_SPIRIT_DEBUG at top of file if you need this
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
    
    bool parse_std_input( ast::hidCollapseList & out )
    {
        std::cout << "Give me something to parse" << std::endl ;
        char buff [1024];
        std::cin.getline(buff, 1024, '\n');
        std::string str( buff );
        
        typedef std::string::const_iterator iterator;
        iterator begin = str.begin(),
        end = str.end();
        
        using namespace HIDCollapse;
        
        bool r = qi:: phrase_parse( begin ,
                                   end ,
                                   main_grammar<std::string::const_iterator, boost::spirit::ascii::space_type >(),
                                   boost::spirit::ascii::space ,
                                   out );
        
        if( r&& begin == end )
        {
            std::cout << "Parsed successfuly." << std::endl;
        }
        else
        {
            r = false;
            std::cerr << "Failed parse." << std::endl;
        }
     
        return r;
    }
    
    bool parse_file( const std::string & filename ,
                    ast::hidCollapseList & out )
    {
        std::ifstream input( filename.c_str() );
        return parse_istream( input, out );
    }
    
    bool parse_istream( std::istream & input , ast::hidCollapseList & out )
    {
        //std::ifstream input( filename.c_str() );
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
        pos_iterator_type position_begin(fwd_begin, fwd_end, std::string("input stream"));
        pos_iterator_type position_end;
        
        using namespace HIDCollapse;
        
        bool r = qi::phrase_parse( position_begin ,
                                  position_end ,
                                  main_grammar<pos_iterator_type,boost::spirit::ascii::space_type>(),
                                  boost::spirit::ascii::space,
                                  out );
        
        if( r && position_begin == position_end )
        {
            std::cout << "Parsed successfuly." << std::endl;
        }
        else
        {
            std::cerr << "Failed to parse." << std::endl;
            r = false;
        }
        return r;
    }
}

int main(int argc, char*argv[])
{
    using namespace HIDCollapse;
    ast::hidCollapseList out;
    
    if(argc < 2)
    {
        while( true )
        {
            parse_std_input( out );
            std::cout << "Parsed Input Succesfuly." << std::endl;
        }
    }
    else
    {
        try
        {
            parse_file(argv[1], out);
        }
        catch(const std::exception& e)
        {
            std::cerr << "Exception: " << e.what() << std::endl;
        }
        
    }
    
}