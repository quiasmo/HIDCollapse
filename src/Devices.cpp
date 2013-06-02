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

#include <sstream>
#include <set>

#include "HIDCollapse.h"

//#define LOWERCASE_EQUALS_UPPERCASE
#if defined ( LOWERCASE_EQUALS_UPPERCASE )
#include <boost/algorithm/string.hpp>
#include <locale>
#endif

namespace HIDCollapse
{

    const float DeviceDescriptor::MATCH_THRESHOLD = 0.5f;

    
    ElementDescriptor::ElementDescriptor():page(-1),usage(-1),sequential(-1)
    {
        
    }
    ElementDescriptor::ElementDescriptor( int64_t page , int64_t usage ):
    page(page),usage(usage)
    {
        //fill usage string here from 
    }
    
    ElementDescriptor::ElementDescriptor( const std::string & nameKey )
    {
        
    }
    ElementDescriptor::ElementDescriptor( int64_t sequential ) :
    page(-1),usage(-1), sequential(sequential)
    {
        
    }
    ElementDescriptor::ElementDescriptor( const ElementDescriptor & ed )
    {
        *this = ed;
    }
    
    ElementDescriptor::~ElementDescriptor()
    {
    }
    
    bool ElementDescriptor::strictCompare( const ElementDescriptor & e )
    {
        if( nameKey.size() > 0 ) return nameKey.compare(e.nameKey) == 0;
        if( sequential >= 0 ) return sequential == e.sequential;
        if( page >=0 && usage >= 0 ) return  page == e.page && usage == e.usage;
        
        return false;
    }
    
    const ElementDescriptor & ElementDescriptor::operator=( const ElementDescriptor & ed )
    {
        page = ed.page;
        usage = ed.usage;
        usageString = ed.usageString;
        nameKey = ed.nameKey;
        sequential = ed.sequential;
        return *this;
    }

    
    
    DeviceDescriptor::DeviceDescriptor( int64_t vendorID, int64_t productID , int64_t versionID ):
    vendorID(vendorID),productID(productID),versionID(versionID)
    {
        
    }
    
    DeviceDescriptor::DeviceDescriptor( const std::string & vendor_product_combo ):
    vendorID(-1),productID(-1),versionID(-1), vendor_product_combo( vendor_product_combo)
    {
        
    }
    
    DeviceDescriptor::DeviceDescriptor( const std::string & manuf, const std::string & product ,
                                       int64_t vendorID, int64_t productID, int64_t versionID ):
    vendorID(vendorID), productID(productID), versionID(versionID),
    vendor_product_combo( manuf + " " + product )
    {
        
    }

    DeviceDescriptor::DeviceDescriptor( const DeviceDescriptor & dd)
    {
        this->copyFrom( &dd );
    }
    
    const DeviceDescriptor & DeviceDescriptor::operator=( const DeviceDescriptor & dd)
    {
        this->copyFrom(&dd);
        return *this;
    }
    
    DeviceDescriptor:: ~DeviceDescriptor()
    {
    }

    
    //fuzzy compare returns 0..1 where 1 is a perfect match
    //and 0 is a complete mismatch
    float DeviceDescriptor::fuzzyCompareType( const DeviceDescriptor * dd ) const
    {
        float factors[4] = {1.f, 1.f, 1.f,1.f};
        
        factors[0] = intCompare( vendorID, dd->vendorID );
        factors[1] = intCompare( productID, dd->productID );
        factors[2] = intCompare( versionID, dd->versionID );
        factors[3] = stringSimilarity( vendor_product_combo , dd->vendor_product_combo );
        
        
        float res = 1.f;
        
        for( int i = 0; i< 4 ; i++ ) res *= factors[i];
        
        //clamp anything we might have
        if ( res > 1.f ) return 1.f;
        if( res < 0 ) return 0.f;
        else return 0.f;
    }
    
    float DeviceDescriptor::intCompare(int64_t i1, int64_t i2)
    {
        if( i1 > 0 )
        {
            if ( i2 > 0 )
            {
                if( i1 == i2 ) return 1.0f;
                else return 0.f;
            }
        }
        //don't affect outcome
        return 1.f;
    }
    
    float DeviceDescriptor::stringSimilarity ( const std::string & s1, const std::string & s2 )
    {
        if( s1.compare("") == 0 )
        {
            //they are equal
            if( s2.compare("") == 0 )
                return 1.0f;
            //they are not equal or similar
            else
                return 0.f ;
        }
        
        //they are not equal or similar
        if( s2.compare("") == 0 ) return 0.f;
        
        using namespace std;
        //break into tokens and find how many tokens are shared
        stringstream strstr1( s1 );
        stringstream strstr2( s2 );
        
        // use stream iterators to copy the stream to the vector as whitespace separated strings
        istream_iterator<string> it1(strstr1);
        istream_iterator<string> it2(strstr2);
        istream_iterator<string> end;
        typedef set<string> tSSet;
        
#if defined ( LOWERCASE_EQUALS_UPPERCASE )
        tSSet set1, set2;
        while( it1 != end )
            set1.insert( boost::to_lower_copy(*it1, std::locale("en_US.UTF-8")));
        while( it2 != end )
            set2.insert( boost::to_lower_copy(*it2, std::locale("en_US.UTF-8")));
#else
        tSSet set1(it1, end);
        tSSet set2(it2, end);
#endif
        
        tSSet * small, * large;
        if( set1.size() > set2.size() )
        {
            large = & set1;
            small = & set2;
        }
        else
        {
            large = & set2;
            small = & set1;
        }
        
        if( small->size() == 0 )
        {
            if( large->size() == 0 )
                return 1;
            
            if( large->size() > 0 )
                return 0;
        };
        
        //compare our two sets
        int matches = 0;
        int mismatches = 0;
        for( tSSet::iterator i = large->begin(); i != large->end() ; ++i )
        {
            tSSet::iterator f = small->find( * i );
            if( f != small->end() )
                matches ++;
            else
                mismatches ++;
        }
        
        if( matches == small->size() )
        {
            return 1.f;
        }
        else
        {
            float res = 0.f;
            res = ( matches - mismatches ) / ( float ) large->size();
            if( res > 1 )
                return 1;
            else if( res < 0 )
                return 0;
            else 
                return res;
        }
    }
    
    bool DeviceDescriptor::evaluateElement( const ElementDescriptor & , int64_t * outVal, int64_t * outMin , int64_t * outMax )
    {
        //this is a non physical Device. always fails to evaluate
        return false;
    }

    //overrride by subclasses.
    void DeviceDescriptor::listElementDescriptors( std::vector<ElementDescriptor> & out )
    {
        //do nothing. override in os. subclass
    }

    void DeviceDescriptor::copyFrom( const DeviceDescriptor * dd )
    {
        vendorID = dd->vendorID;
        productID = dd->productID;
        versionID = dd->vendorID;
                
        //empty string is "no value" and does not affect comparison
        //defaults to whatever std::sring defaults to
        vendor_product_combo = dd->vendor_product_combo;

    }


}