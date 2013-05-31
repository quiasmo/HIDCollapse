//
//  OSXManager.cpp
//  HIDCollapse
//
//  Created by Juan Carlos Borda on 5/22/13.
//  Copyright (c) 2013 Juan Carlos Borda. All rights reserved.
//

#include <IOKit/hid/IOHIDKeys.h>
#include <IOKit/hid/IOHIDDevice.h>
#include <IOKit/hid/IOHIDLib.h>

#include "IOHIDLib_.h"

#include "OSXManager.h"


namespace HIDCollapse
{

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
        char manufstr[256];
        char productstr[256];
        
        t_descriptors newDescriptors;
        
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
            CFStringGetCString( cfmanuf , manufstr, 256, toEncoding );
            CFRelease( cfmanuf );
            
            CFStringRef cfdevstr = IOHIDDevice_GetProduct( dev );
            CFStringGetCString( cfdevstr , productstr , 256, toEncoding );
            CFRelease( cfdevstr );

            vendorID = IOHIDDevice_GetVendorID( dev );
            productID = IOHIDDevice_GetProductID( dev );
            versionID = IOHIDDevice_GetVersionNumber( dev );
            location = IOHIDDevice_GetLocationID( dev );
            
            OSXDeviceDescriptor * descriptor = new OSXDeviceDescriptor( manufstr , productstr , //implicit std::string conversion
                                                                       vendorID , productID, versionID ,
                                                                       location , dev );
            newDescriptors.push_back( descriptor );
        }
        
        //after this call, no Indices hold references to descriptors in  reportedDescriptors
        //only to descriptors in newDescriptors
        remapIndices( newDescriptors );

        //safe to delete old descriptors
        for( t_descriptors::iterator i = reportedDescriptors.begin() ;i != reportedDescriptors.end() ; i++ )
            delete *i;
        reportedDescriptors.clear();
        reportedDescriptors.assign(newDescriptors.begin(), newDescriptors.end());
        
        buildNovelIndices();
        
    }
    
    void OSXManager::remapIndices( const t_descriptors & newDescriptors )
    {
        //look at existing indices and replace old ones with matching new ones
        for( t_indices::iterator i = indices.begin(); i != indices.end() ; i++ )
        {
            //gets their device descriptors
            OSXIndex * index = *i;
            const OSXDeviceDescriptor * activeDescriptor = static_cast<const OSXDeviceDescriptor*>( index->getPhysicalDevice() );
            
            if( activeDescriptor )
            {
                //find divice match
                bool foundMatch = false;
                for( t_descriptors::const_iterator j = newDescriptors.begin() ;
                    j != newDescriptors.end() && !foundMatch ;
                    j++ )
                {
                    OSXDeviceDescriptor * newDescriptor = * j;
                    if( newDescriptor->matchesPhysicalDevice( activeDescriptor ) )
                    {
                        index->setPhysicalDevice( newDescriptor );
                        newDescriptor->associatedIndex = index;
                        foundMatch = true;
                    }
                }
                
                //we found no match. no device for this index
                if( !foundMatch )
                {
                    index->setPhysicalDevice( 0 );
                }
            }            
        }        
    }
    
    void OSXManager::buildNovelIndices()
    {
        //go through all the reportedDEvices checking to see
        //which of them has no associatedindex
        for( t_descriptors::iterator i = reportedDescriptors.begin() ; i!=reportedDescriptors.end(); i++ )
        {
            OSXDeviceDescriptor * descriptor = * i;
            if( !descriptor->associatedIndex )
            {
                //find an index descriptor that matches more than 0.5
                float max = 0.5f;
                IndexDescriptor * maxDescriptor = 0;
                for( t_indexDescriptors::iterator j = indexDescriptors.begin();
                    j != indexDescriptors.end() ;
                    j++)
                {
                    IndexDescriptor * indexDesc = * j;
                    float score = indexDesc->getSourceDevice()->fuzzyCompareType( descriptor );
                    if( score > max )
                    {
                        max = score;
                        maxDescriptor = indexDesc;
                    }
                }
                
                if( maxDescriptor )
                {
                    //build an index
                    OSXIndex * ind = new OSXIndex( maxDescriptor );
                    ind->setPhysicalDevice( descriptor );
                    descriptor->associatedIndex = ind;
                    indices.push_back( ind );
                }
            }
        }
    }

    
    void OSXManager::cleanup()
    {
        if( osxHidManager )
        {
            IOHIDManagerClose( osxHidManager , kIOHIDOptionsTypeNone );
            CFRelease( osxHidManager );
        }
        osxHidManager = 0;
        
        osxReportedDevices.clear();
        
        for( t_indices::iterator i = indices.begin(); i != indices.end(); i++ )
            delete * i;
        indices.clear();
        
        for( t_descriptors::iterator i = reportedDescriptors.begin(); i!=reportedDescriptors.end(); i++ )
            delete *i;
        reportedDescriptors.clear();
        
        for( t_indexDescriptors::iterator i = indexDescriptors.begin(); i!= indexDescriptors.end(); i++ )
            delete * i;
        indexDescriptors.clear();
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
