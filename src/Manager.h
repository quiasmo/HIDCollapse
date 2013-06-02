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
#include <map>
#include "HIDCollapse.h"
#include "HIDCollapseParser.h"
namespace HIDCollapse
{
    class Manager
    {
    public:
        
        Manager();
        virtual ~Manager();
        
        void initialize( const std::string & config );
        
        //perform any necessary polling to devices 
        virtual void capture()=0;
       
        //convenience methods to grab button from first available Index
        virtual const IndexedButton * findButton( const std::string & elementIndex , int player = -1 );
        virtual const IndexedAxis * findAxis( const std::string & elementIndex , int player = -1 );
        
        //sets the given index to be player.
        //if an index already holds this slot, they trade places
        virtual void setPlayerIndex( Index * , int player );
        
        
        //negative values gives you any player available
        virtual void getPlayers( std::vector<int> & out );
        virtual Index * getPlayer( int player = -1 );
        
        virtual void getIndices( std::vector<const Index*> & result );
        virtual void getIndicesForDevice( const DeviceDescriptor * abstractDevice , std::vector<Index*> & result );
        virtual void getIndicesWithName( const std::string & , std::vector<Index*> & result );
        
    protected:
        
        virtual void parseIndexDefinitions( const std::string & filename );
        
        //build a list of available hw devices
        //and put them in mPhysicalDevices
        virtual void buildDeviceList()=0;
        
        //builds indices from mPhysicalDevices matching
        //parsed index descriptors
        virtual void buildIndices();
        
        //physical devices added or removed
        virtual void devicePlugged( DeviceDescriptor * physicalDevice );
        virtual void deviceUnplugged( DeviceDescriptor * physicalDevice );
        
        //clears everything, including parsed configuration
        virtual void clear();
        
        //gets player index.
        //if player < 0 then returns any one index
        virtual Index * fetchIndex( int player );
        
        //mapping instructions
        ast::hidCollapseList indexDefinitions;

        //physical devices
        typedef 
        std::vector<DeviceDescriptor*> tPhysicalDevices;
        tPhysicalDevices mPhysicalDevices;

        //virtual devices
        typedef std::vector<Index *> tIndices;
        tIndices mIndices;
        
        //virtual devices
        typedef std::map<int , Index* > tPlayers;
        tPlayers mPlayers;
    };
}

