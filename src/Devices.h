//
//  Devices.h
//  HIDCollapse
//
//  Created by Juan Carlos Borda on 5/21/13.
//  Copyright (c) 2013 Juan Carlos Borda. All rights reserved.
//

#pragma once
#include <string>
#include "HIDCollapse.h"

namespace HIDCollapse
{
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
        DeviceDescriptor( int64_t vendorID , int64_t deviceID = -1, int64_t versionID = -1 );
        DeviceDescriptor( const std::string & vendor_product_combo );
        DeviceDescriptor( const std::string & manuf, const std::string & product ,
                         int64_t vendorID, int64_t productID, int64_t versionID );
        DeviceDescriptor( const DeviceDescriptor & );
        virtual ~DeviceDescriptor() = 0;
        
        //fuzzy compare returns 0..1 where 1 is a perfect match
        //and 0 is a complete mismatch
        virtual float fuzzyCompareType( const DeviceDescriptor * dd ) const ;
        
        virtual void copyFrom( const DeviceDescriptor * ) = 0;
        virtual bool matchesPhysicalDevice( const DeviceDescriptor * ) = 0;

    protected:
        
        static float intCompare( int i1, int i2 );
        static float stringSimilarity ( const std::string & s1, const std::string & s2 );
        
        //0 or less values are "no value" and do not affect comparison
        //defaults to -1
        int64_t vendorID, productID, versionID;
        
        //empty string is "no value" and does not affect comparison
        //defaults to whatever std::sring defaults to
        std::string vendor_product_combo;
        std::string vendor;
        std::string product;
        
    };
    
    /**
     * base class for handling a whole a device instance
     */
    class Device
    {
    public:
        virtual ~Device() {}
    };
};

