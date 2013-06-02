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

#pragma once
#include <string>
#include <vector>
#include "HIDCollapse.h"

namespace HIDCollapse
{
    
    class ElementDescriptor
    {
    public:
        ElementDescriptor();
        ElementDescriptor( int64_t page , int64_t usage );
        ElementDescriptor( const std::string & nameKey );
        ElementDescriptor( int64_t sequential );
        ElementDescriptor( const ElementDescriptor & );
        
        virtual ~ElementDescriptor();
        virtual bool strictCompare( const ElementDescriptor & e );
        const ElementDescriptor & operator=( const ElementDescriptor &);
    protected:
        
        int64_t page, usage;
        std::string usageString;
        std::string nameKey;
        int64_t sequential;
    };
    
    /**
     * Base calss for identifying a type of device
     * using HID vendor, device and version ids or
     * sing descriptions strings that resemble each other
     * resemblance is common tokens within vendor_device_combo
     */
    class DeviceDescriptor
    {
    public:
        DeviceDescriptor();
        DeviceDescriptor( int64_t vendorID , int64_t productID = -1, int64_t versionID = -1 );
        DeviceDescriptor( const std::string & vendor_product_combo );
        DeviceDescriptor( const std::string & manuf, const std::string & product ,
                         int64_t vendorID, int64_t productID, int64_t versionID );
        DeviceDescriptor( const DeviceDescriptor & );
        const DeviceDescriptor & operator=( const DeviceDescriptor & );
        virtual ~DeviceDescriptor();
        
        //fuzzy compare returns 0..1 where 1 is a perfect match
        //and 0 is a complete mismatch
        virtual float fuzzyCompareType( const DeviceDescriptor * dd ) const ;
        
        //acceptable match
        static const float MATCH_THRESHOLD;
        
        virtual void copyFrom( const DeviceDescriptor * );
        
        //overrride by subclasses.
        virtual void listElementDescriptors( std::vector<ElementDescriptor> & out );
        
        //evaluates the state of the element and get it min and max values
        //puts them in outVal, outMin, outMax ( can be null values if not needed )
        //returns true if the element was successfully evaluated
        //false otherwise
        virtual bool evaluateElement( const ElementDescriptor & , int64_t * outVal, int64_t * outMin , int64_t * outMax );
    protected:
        
        static float intCompare( int64_t i1, int64_t i2 );
        static float stringSimilarity ( const std::string & s1, const std::string & s2 );
        
        //0 or less values are "no value" and do not affect comparison
        //defaults to -1
        int64_t vendorID, productID, versionID;
                
        //empty string is "no value" and does not affect comparison
        //defaults to whatever std::sring defaults to
        std::string vendor_product_combo;
        
    };
};

