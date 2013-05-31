//
//  Manager.h
//  HIDCollapse
//
//  Created by Juan Carlos Borda on 5/21/13.
//  Copyright (c) 2013 Juan Carlos Borda. All rights reserved.
//

#pragma once
#include <sstream>
#include <vector>
#include "HIDCollapse.h"
namespace HIDCollapse
{
    class Manager
    {
    public:
        
        Manager();
        virtual ~Manager() {}
        
        //poll 
        virtual void capture();
       
        //adds Index definitions to this Manager
        virtual void appendIndexDeffinitions( const char * filename );

        virtual IndexedElement * findElement( const std::string & elementIndex , int player = 0 );
        virtual IndexedButton * findButton( const std::string & elementIndex , int player = 0 );
        virtual IndexedAxis * findAxis( const std::string & elementIndex , int player = 0 );
        
        virtual int getPlayerFromIndex( Index * );
        virtual void setPlayerIndex( Index * , int player );
        virtual Index * getIndexFromPlayer( int player );
        
        virtual std::vector<Index*> getIndices();
        virtual std::vector<Index*> getIndicesForDevice( const DeviceDescriptor * dd );
        virtual std::vector<Index*> getIndicesWithName( const std::string & );
        
    protected:
       
        virtual bool shouldBulidIndex( DeviceDescriptor * , IndexDescriptor * ) = 0;
    
        virtual void clear();
        virtual void buildDeviceList();
        virtual void buildIndexList();
        virtual void devicePlugged( void * );
        virtual void deviceUnplugged( void * );

    };
}

