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

#include <IOKit/hid/IOHIDLib.h>

#include "IOHIDLib_.h"

#include "OSXManager.h"


namespace HIDCollapse
{

    
    OSXDeviceDescriptor::OSXDeviceDescriptor( const std::string & manuf, const std::string & product ,
                                             int64_t vendorID, int64_t productID, int64_t versionID ,
                                             IOHIDDeviceRef dev ):
    DeviceDescriptor( manuf , product, vendorID, productID, versionID),
    deviceRef( dev )
    {
        
    }
    
    bool OSXDeviceDescriptor::evaluateElement( const ElementDescriptor & , int64_t * outVal, int64_t * outMin , int64_t * outMax )
    {
        return false;
    }

    
    OSXManager::OSXManager()
    {
        osxHidManager = 0;
        setup();
    }
    OSXManager::~OSXManager()
    {
        cleanup();
    }
    
    //convenince static function to copy contents of a cfset into our stl vector
    void OSXManager::CFSetApplierFunctionCopyToSTLVector( const void *value, void *context )
    {
        ((t_reportedDevices*)context)->push_back( (IOHIDDeviceRef) value );
    }   // CFSetApplierFunctionCopyToCFArray
    

    void OSXManager::setup()
    {
        //create our HID Manager instance
        osxHidManager = IOHIDManagerCreate( CFAllocatorGetDefault() ,  kIOHIDOptionsTypeNone );
        
        //check that we created it
        if( CFGetTypeID( osxHidManager ) == IOHIDManagerGetTypeID() )
        {
            
            //match devices matching joysticks or gamepads
            int howMany = 2;
            UInt32 usagePages [2] = { kHIDPage_GenericDesktop , kHIDPage_GenericDesktop };
            UInt32 usages     [2] = { kHIDUsage_GD_Joystick   , kHIDUsage_GD_GamePad    };
            
            //build list of devices characteristics to which we want to match
            CFMutableArrayRef matchUsagePages = buildMultiDeviceList( usagePages, usages, howMany);
            
            //tell the manager which what they should look like
            IOHIDManagerSetDeviceMatchingMultiple( osxHidManager, matchUsagePages );
            
            if( matchUsagePages )
                CFRelease( matchUsagePages );
            
            //open the Manager
            IOReturn res = IOHIDManagerOpen( osxHidManager, kIOHIDOptionsTypeNone );
            
            if( res == kIOReturnSuccess )
            {
                rebuildOsxReportedDevicesList();
                rebuildDeviceDescriptors();
            }
        }
    }

    void OSXManager::rebuildOsxReportedDevicesList()
    {
        //get the devices
        CFSetRef devCFSetRef = IOHIDManagerCopyDevices( osxHidManager );
        
        if ( devCFSetRef )
        {
            osxReportedDevices.clear();
            
            //put the devices reported by the manager in a convenient vector
            CFSetApplyFunction( devCFSetRef, CFSetApplierFunctionCopyToSTLVector, ( void * ) ( & osxReportedDevices ) );
            
            // and release the set we copied from the IOHID manager
            CFRelease( devCFSetRef );
        }
    }
    
    void OSXManager::rebuildDeviceDescriptors()
    {
        char manufstr[512];
        char productstr[512];
        
        tPhysicalDevices newDevices;
        
        //make ascii c++ strings.
        //this should't be an issue as we should be getting
        //ascii strings from wherever they come
        CFStringEncoding toEncoding = kCFStringEncodingASCII;
        
        for( t_reportedDevices::iterator i= osxReportedDevices.begin(); i != osxReportedDevices.end(); i++ )
        {
            IOHIDDeviceRef dev = * i;
            
            long vendorID;
            long productID;
            long versionID;
            long location;
            
            //IOHIDDevice_GetVendorIDSource(IOHIDDeviceRef inIOHIDDeviceRef); //finds out if bluetooth or usb
            
            CFStringRef cfmanuf = IOHIDDevice_GetManufacturer( dev );
            CFStringGetCString( cfmanuf , manufstr, 512, toEncoding );
            CFRelease( cfmanuf );
            
            CFStringRef cfdevstr = IOHIDDevice_GetProduct( dev );
            CFStringGetCString( cfdevstr , productstr , 512, toEncoding );
            CFRelease( cfdevstr );

            vendorID = IOHIDDevice_GetVendorID( dev );
            productID = IOHIDDevice_GetProductID( dev );
            versionID = IOHIDDevice_GetVersionNumber( dev );
            location = IOHIDDevice_GetLocationID( dev );
            
            OSXDeviceDescriptor * descriptor = new OSXDeviceDescriptor( manufstr , productstr , //implicit std::string conversion
                                                                       vendorID , productID, versionID ,
                                                                        dev );
            newDevices.push_back( descriptor );
        }

        //safe to delete old descriptors
        for( tPhysicalDevices::iterator i = mPhysicalDevices.begin() ;i != mPhysicalDevices.end() ; i++ )
        {
            deviceUnplugged( *i );
            delete *i;
        }
        mPhysicalDevices.clear();
        mPhysicalDevices.assign( newDevices.begin(), newDevices.end() );
        
        for( tPhysicalDevices::iterator i = mPhysicalDevices.begin() ;i != mPhysicalDevices.end() ; i++ )
        {
            devicePlugged( * i );
        }        
    }
    
    void OSXManager::cleanup()
    {
        for( tPhysicalDevices::iterator i = mPhysicalDevices.begin(); i!=mPhysicalDevices.end(); i++ )
        {
            deviceUnplugged( * i );
            delete *i;
        }
        mPhysicalDevices.clear();

        if( osxHidManager )
        {
            IOHIDManagerClose( osxHidManager , kIOHIDOptionsTypeNone );
            CFRelease( osxHidManager );
        }
        osxHidManager = 0;
        
        osxReportedDevices.clear();
        
        
        
    }
    
    //release the result after you are finished
    CFMutableArrayRef OSXManager::buildMultiDeviceList( const UInt32 *inUsagePages, const UInt32 *inUsages, int inNumDeviceTypes )
    {
        CFMutableArrayRef hidMatchingCFMutableArrayRef = NULL;
        
        if ( inUsages && inUsagePages && inNumDeviceTypes ) {
            hidMatchingCFMutableArrayRef = CFArrayCreateMutable( kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks );
            
            if ( hidMatchingCFMutableArrayRef ) {
                int idx;
                for ( idx = 0; idx < inNumDeviceTypes; idx++ ) {    // for all usage and usage page types
                    // Set up matching dictionary. returns NULL on error.
                    CFMutableDictionaryRef hidMatchingCFDictRef = setUpMatchingDictionary( inUsagePages[idx], inUsages[idx] );
                    
                    if ( hidMatchingCFDictRef ) {
                        CFArrayAppendValue( hidMatchingCFMutableArrayRef, (void*) hidMatchingCFDictRef );
                        CFRelease( hidMatchingCFDictRef );
                    } else {
                        fprintf( stderr, "%s: Couldn’t create a matching dictionary.", __PRETTY_FUNCTION__ );
                    }
                }
            } else {
                fprintf( stderr, "%s: Couldn’t create a matching array.", __PRETTY_FUNCTION__ );
            }
        }
        
        //realease it 
        return hidMatchingCFMutableArrayRef;
    }  
    
    //release the returned ref after finished
    CFMutableDictionaryRef OSXManager::setUpMatchingDictionary( UInt32 inUsagePage, UInt32 inUsage )
    {
        // create a dictionary to add usage page/usages to
        CFMutableDictionaryRef refHIDMatchDictionary = CFDictionaryCreateMutable( kCFAllocatorDefault, 0,
                                                                                 &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
        
        if ( refHIDMatchDictionary ) {
            if ( inUsagePage ) {
                // Add key for device type to refine the matching dictionary.
                CFNumberRef pageCFNumberRef = CFNumberCreate( kCFAllocatorDefault, kCFNumberIntType, &inUsagePage );
                
                if ( pageCFNumberRef ) {
                    CFDictionarySetValue( refHIDMatchDictionary,
                                         CFSTR( kIOHIDPrimaryUsagePageKey ), pageCFNumberRef );
                    CFRelease( pageCFNumberRef );
                    
                    // note: the usage is only valid if the usage page is also defined
                    if ( inUsage ) {
                        CFNumberRef usageCFNumberRef = CFNumberCreate( kCFAllocatorDefault, kCFNumberIntType, &inUsage );
                        
                        if ( usageCFNumberRef ) {
                            CFDictionarySetValue( refHIDMatchDictionary,
                                                 CFSTR( kIOHIDPrimaryUsageKey ), usageCFNumberRef );
                            CFRelease( usageCFNumberRef );
                        } else {
                            fprintf( stderr, "%s: CFNumberCreate( usage ) failed.", __PRETTY_FUNCTION__ );
                        }
                    }
                } else {
                    fprintf( stderr, "%s: CFNumberCreate( usage page ) failed.", __PRETTY_FUNCTION__ );
                }
            }
        } else {
            fprintf( stderr, "%s: CFDictionaryCreateMutable failed.", __PRETTY_FUNCTION__ );
        }
        return refHIDMatchDictionary;
    }

    
    
}
