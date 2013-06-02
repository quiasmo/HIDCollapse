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
#include <IOKit/hid/IOHIDManager.h>
#include <vector>
#include "HIDCollapse.h"

namespace HIDCollapse
{
    /* do we need this?
    class OSXElementDescriptor: public ElementDescriptor
    {
    public:
        OSXElementDescriptor();
        OSXElementDescriptor( int64_t page , int64_t usage );
        OSXElementDescriptor( const std::string & nameKey );
        OSXElementDescriptor( int64_t sequential );
    protected:
        //system specific stuff we might need to keep here.
    };
    */
     
    class OSXDeviceDescriptor: public DeviceDescriptor
    {
    public:
        OSXDeviceDescriptor( const std::string & manuf, const std::string & product ,
                            int64_t vendorID, int64_t productID, int64_t versionID ,
                            IOHIDDeviceRef dev );
               
        virtual bool evaluateElement( const ElementDescriptor & , int64_t * outVal, int64_t * outMin , int64_t * outMax );

        
        IOHIDDeviceRef deviceRef;
        
    protected:
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

        t_reportedDevices osxReportedDevices;
        
        void buildNovelIndices();
        
    private:
        
        
        //helper functions to setup the system
        static CFMutableArrayRef buildMultiDeviceList( const UInt32 *inUsagePages, const UInt32 *inUsages, int inNumDeviceTypes );
        static CFMutableDictionaryRef setUpMatchingDictionary( UInt32 inUsagePage, UInt32 inUsage );
        static void CFSetApplierFunctionCopyToSTLVector( const void *value, void *context );
    };
}