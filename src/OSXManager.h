//
//  OSXManager.h
//  HIDCollapse
//
//  Created by Juan Carlos Borda on 5/22/13.
//  Copyright (c) 2013 Juan Carlos Borda. All rights reserved.
//

#pragma once
#include <IOKit/hid/IOHIDManager.h>
#include <vector>
#include "HIDCollapse.h"

namespace HIDCollapse
{
    
    
    class OSXDeviceDescriptor: public DeviceDescriptor
    {
    public:
        OSXDeviceDescriptor( const std::string & manuf, const std::string & product ,
                            int64_t vendorID, int64_t productID, int64_t versionID ,
                            int64_t location , IOHIDDeviceRef dev );
        
        virtual void copyFrom( const DeviceDescriptor * );

        virtual bool matchesPhysicalDevice( const OSXDeviceDescriptor * );
        
        IOHIDDeviceRef deviceRef;
        long deviceLocation;
        Index * associatedIndex;
        
    protected:
    };
    
    class OSXIndex: public Index
    {
    public:
        OSXIndex( IndexDescriptor * id );
        void setPhysicalDevice( const OSXDeviceDescriptor * dd );
    };


    class OSXManager: public Manager
    {
    public:
        OSXManager();
        virtual ~OSXManager();
    protected:
        void setup();
        void cleanup();
        void rebuildOsxReportedDevicesList();
        void rebuildDeviceDescriptors();
        
        IOHIDManagerRef osxHidManager;
        typedef std::vector<IOHIDDeviceRef> t_reportedDevices;
        typedef std::vector<OSXDeviceDescriptor*> t_descriptors;
        typedef std::vector<OSXIndex*> t_indices;
        typedef std::vector<IndexDescriptor*> t_indexDescriptors;
        
        t_reportedDevices osxReportedDevices;
        t_indices indices;
        t_descriptors reportedDescriptors;
        t_indexDescriptors indexDescriptors;
        void remapIndices( const t_descriptors & newDescriptors );
        void buildNovelIndices();
        
    private:
        //helper functions to setup the system
        static CFMutableArrayRef buildMultiDeviceList( const UInt32 *inUsagePages, const UInt32 *inUsages, int inNumDeviceTypes );
        static CFMutableDictionaryRef setUpMatchingDictionary( UInt32 inUsagePage, UInt32 inUsage );
        static void CFSetApplierFunctionCopyToSTLVector( const void *value, void *context );
    };
}