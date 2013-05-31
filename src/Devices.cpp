//
//  Devices.cpp
//  HIDCollapse
//
//  Created by Juan Carlos Borda on 5/21/13.
//  Copyright (c) 2013 Juan Carlos Borda. All rights reserved.
//

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
    DeviceDescriptor::~DeviceDescriptor()
    {
        
    }
    
    DeviceDescriptor::DeviceDescriptor( int vendorID, int deviceID , int versionID ):
    vendorID(vendorID),deviceID(deviceID),versionID(versionID)
    {
        
    }
    
    DeviceDescriptor::DeviceDescriptor( const std::string & vendor_device_combo ):
    vendorID(-1),deviceID(-1),versionID(-1), vendor_device_combo(vendor_device_combo)
    {
        
    }
    
    //fuzzy compare returns 0..1 where 1 is a perfect match
    //and 0 is a complete mismatch
    float DeviceDescriptor::fuzzyCompare( const DeviceDescriptor & dd ) const
    {
        float factors[4] = {1.f, 1.f, 1.f,1.f};
        
        factors[0] = intCompare( vendorID, dd.vendorID );
        factors[1] = intCompare( deviceID, dd.deviceID );
        factors[2] = intCompare( versionID, dd.versionID );
        factors[3] = stringSimilarity( vendor_device_combo , dd.vendor_device_combo );
        
        
        float res = 1.f;
        
        for( int i = 0; i< 4 ; i++ ) res *= factors[i];
        
        //clamp anything we might have
        if ( res > 1.f ) return 1.f;
        if( res < 0 ) return 0.f;
    }
    
    float DeviceDescriptor::intCompare(int i1, int i2) const
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
    
    float DeviceDescriptor::stringSimilarity ( const std::string & s1, const std::string & s2 ) const
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
            if( res > 1 ) return 1;
            else if( res < 0 ) return 0;
        }
    }
    

}