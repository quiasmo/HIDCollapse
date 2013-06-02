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

#include "HIDCollapse.h"

namespace HIDCollapse
{
    
    Manager::Manager()
    {
    }
    
    void Manager::initialize(const std::string & f )
    {
        parseIndexDefinitions( f );
        buildDeviceList();
        buildIndices();
    }
    
    Manager::~Manager()
    {
        clear();
    }
    
    void Manager::clear()
    {
        for( tIndices::iterator i = mIndices.begin();
            i!= mIndices.end();
            i++)
        {
            delete * i;
        }
        mIndices.clear();
        indexDefinitions.clear();
    }
    
    //adds Index definitions to this Manager
    void Manager::parseIndexDefinitions( const std::string & file  )
    {
        ast::hidCollapseList indexMappings;
        try
        {
            bool res = parse_file(file, indexMappings );
            if ( res )
                indexDefinitions.insert(indexDefinitions.end(), indexDefinitions.begin(), indexDefinitions.end());
        }
        catch(const std::exception& e)
        {
            std::cerr << "Exception: " << e.what() << std::endl;
        }
    }
    
    struct makeDescriptor: public boost::static_visitor<DeviceDescriptor>
    {
        DeviceDescriptor operator()(const std::string & key ) const
        {
            return DeviceDescriptor( key );
        }
        DeviceDescriptor operator()(const ast::devicePair & key ) const
        {
            return DeviceDescriptor( key.vendor, key.product );
        }
        DeviceDescriptor operator()(const ast::deviceTriplet & key ) const
        {
            return DeviceDescriptor( key.vendor, key.product, key.version );
        }
    };
    
    void Manager::buildIndices()
    {
        
        for( ast::hidCollapseList::iterator d = indexDefinitions.begin();
            d != indexDefinitions.end() ;
            d++ )
        {
            //build a comparable device descriptor ot compare with the ones
            //generated by the operating system specific code
            DeviceDescriptor declaredDevice = boost::apply_visitor( makeDescriptor() , d->device );
            
            for( tPhysicalDevices::iterator i= mPhysicalDevices.begin() ;
                i!= mPhysicalDevices.end() ;
                i++ )
            {
                DeviceDescriptor * physicalDevice = *i;
                if( declaredDevice.fuzzyCompareType( physicalDevice ) > DeviceDescriptor::MATCH_THRESHOLD )
                {
                    //build an index for this device
                    //element mapping occurrs at an OS-aware level
                    //so let the implementaiton of createIndex and createElements handle that
                    Index * newIndex = new Index( this, d->entries , d->index );
                    newIndex->indexElements( d->entries );
                    mIndices.push_back( newIndex );
                    
                    //assign the next available payer slot
                    int player = 0;
                    bool done = false;
                    do {
                        tPlayers::iterator i = mPlayers.find( player );
                        if( i == mPlayers.end() )
                        {
                            mPlayers[player]= newIndex;
                            newIndex->player =  player ;
                            done = true;
                        }
                    } while (!done);
                }
            }
        }
    }
    
    void Manager::devicePlugged( DeviceDescriptor * physicalDevice )
    {
        for( tIndices::iterator i = mIndices.begin(); i != mIndices.end() ; i++ )
        {
            Index * index = * i;
            if( !index->getPhysicalDevice() )
            {
                DeviceDescriptor * dd = index->recallDevice();
                if( dd && dd->fuzzyCompareType( physicalDevice ) )
                {
                    index->setPhysicalDevice( physicalDevice );
                    index->forgetDevice();
                }
            }
        }
    }
    
    void Manager::deviceUnplugged( DeviceDescriptor * physicalDevice )
    {
        for( tIndices::iterator i = mIndices.begin(); i != mIndices.end() ; i++ )
        {
            Index * index = * i;
            if( physicalDevice == index->getPhysicalDevice() )
            {
                index->rememberDevice( * physicalDevice );
                index->setPhysicalDevice( 0 );
            }
        }
    }
    
    Index * Manager::fetchIndex( int player )
    {
        tPlayers::iterator p = mPlayers.find( player );
        if( p != mPlayers.end() )
            return p->second;
        else if ( player < 0 ) //fallback
        {
            for (tPlayers::iterator p = mPlayers.begin(); p!= mPlayers.end() ; p++ )
            {
                return p->second;
            }
        }
        //no indices
        return 0;
    }
    
    
    const IndexedButton * Manager::findButton( const std::string & elementIndex , int player )
    {
        Index * i = fetchIndex( player );
        if( i )
        {
            return i->getButton( elementIndex );
        }
        
        return 0;
    }
    
    const IndexedAxis * Manager::findAxis( const std::string & elementIndex , int player )
    {
        Index * i = fetchIndex( player );
        if( i )
        {
            return i->getAxis( elementIndex );
        }
        
        return 0;
    }
    
    void Manager::setPlayerIndex( Index * i , int player  )
    {
        Index * ex  = 0;
        tPlayers::iterator exi = mPlayers.find( player );
        if( exi != mPlayers.end() )
            ex = exi->second;
            
        if( ex != i )
        {
            if( ex )
            {
                int newEx = i->getPlayer();
                ex->player = newEx ;
                mPlayers[ newEx ]  = ex;
            }
            
            mPlayers[ player ] = i ;
            i->player = player;
        }
    }
    
    void Manager::getPlayers( std::vector<int> & out )
    {
        for( tPlayers::iterator i = mPlayers.begin() ; i != mPlayers.end(); i++ )
        {
            out.push_back( i->first );
        }
    }

    Index * Manager::getPlayer( int player )
    {
        return fetchIndex( player );
    }
    
    void Manager::getIndices( std::vector< const Index *> & out )
    {
        for( tIndices::iterator i = mIndices.begin() ;i != mIndices.end(); i++ )
        {
            out.push_back( * i );
        }
    }
    
    void Manager::getIndicesForDevice( const DeviceDescriptor * dd , std::vector<Index *> & out )
    {
        for( tIndices::iterator i = mIndices.begin() ;i != mIndices.end(); i++ )
        {
            Index * ix = *i;
            if( ix->getPhysicalDevice()->fuzzyCompareType( dd ) > DeviceDescriptor::MATCH_THRESHOLD )
            {
                out.push_back( * i );
            }
        }
    }
    
    void Manager::getIndicesWithName( const std::string & s, std::vector<Index *> & out )
    {
        for( tIndices::iterator i = mIndices.begin() ;i != mIndices.end(); i++ )
        {
            Index * ix = *i;
            if( ix->getName().compare(s) == 0 )
            {
                out.push_back( * i );
            }
        }
    }

}