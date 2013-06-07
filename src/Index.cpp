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

namespace HIDCollapse {
    
    
    ////////
    /// INDEX
    ////////
    
    Index::Index( Manager * _parent, const ast::entries & entries , const std::string & _name ):
    entries( entries )
    {
        parent = _parent;
        name = _name;
        physicalDevice = 0;
        deviceMemory = 0;
    }
    
    Index::~Index()
    {
        clear();
        delete physicalDevice;
    }
    
    //const access to string indexed fields return null if not present
    IndexedAxis *        Index::getAxis( const std::string & key ) const
    {
        tStringIndex::const_iterator i = strAxes.find(key);
        if( i != strAxes.end() )
            return static_cast<IndexedAxis*> (i->second);
        else
            return 0;
    }
    
    IndexedButton *     Index::getButton( const std::string & key ) const
    {
        tStringIndex::const_iterator i = strButtons.find( key );
        if( i != strButtons.end() )
            return static_cast<IndexedButton*> (i->second);
        else
            return 0;
    }
    
    //const access to int indexed fields return null if not present
    IndexedAxis *        Index::getAxis( int key ) const
    {
        tIntIndex::const_iterator i = intAxes.find( key  );
        if( i != intAxes.end() )
            return static_cast<IndexedAxis*> (i->second);
        else
            return 0;
    }
    
    IndexedButton *      Index::getButton( int key ) const
    {
        tIntIndex::const_iterator i = intButtons.find( key );
        if( i != intButtons.end() )
            return static_cast<IndexedButton*> (i->second);
        else
            return 0;
    }
    
    DeviceDescriptor * Index::getPhysicalDevice() const
    {
        return physicalDevice;
    }
    
    void Index::setPhysicalDevice( DeviceDescriptor * dev )
    {
        physicalDevice = dev;
        if( physicalDevice && allElements.size() <= 0 )
        {
            indexElements( entries );
        }
    }
    

    struct makeElemDesc: public boost::static_visitor<ElementDescriptor>
    {
        ElementDescriptor operator()( int64_t seq ) const
        {
            return ElementDescriptor( seq );
        }
        
        ElementDescriptor operator()(const std::string & key ) const
        {
            return ElementDescriptor( key );
        }
        ElementDescriptor operator()(const ast::elemHexPairKey & key ) const
        {
            return ElementDescriptor( key.page, key.usage );
        }
    };

    
    struct getElementType: public boost::static_visitor<IndexedElement::Type>
    {
        IndexedElement::Type operator()( const ast::indexButton & ) const
        {
            return IndexedElement::BUTTON;
        }
        
        IndexedElement::Type operator()( const ast::indexAxe &) const
        {
            return IndexedElement::ABSOLUTE_AXIS;
        }
        
    };
    
    struct getIndexKeys: public boost::static_visitor<const ast::targetElementKeys &>
    {
        const ast::targetElementKeys & operator()(const ast::indexButton& b) const
        {
            return b.keys;
        }
        
        const ast::targetElementKeys & operator()(const ast::indexAxe & ax ) const
        {
            return ax.keys;
        }
    };
    
    struct buildIndices : public boost::static_visitor<void>
    {
        buildIndices( Index::tIntIndex * _intIndex, Index::tStringIndex * _strIndex , IndexedElement * _elem )
        {
            intIndex = _intIndex;
            strIndex = _strIndex;
            elem = _elem;
        }
        Index::tIntIndex * intIndex;
        Index::tStringIndex * strIndex;
        IndexedElement * elem;
        
        void operator()(const std::string & strKey ) const
        {
            (*strIndex)[strKey] = elem;
        }
        
        void operator()( int intKey ) const
        {
            (*intIndex)[intKey] = elem;
        }
    };
    
    void  Index::indexElements ( const ast::entries & entries )
    {
        typedef std::vector<ElementDescriptor> tElements;
        tElements physicalElements;
        
        physicalDevice->listElementDescriptors(physicalElements);
        
        /*
        {
            std::cout << "Physical Device " << physicalDevice->getVendorProductCombo() <<
            " lists the following elements: " << std::endl;
            for( tElements::iterator e = physicalElements.begin() ;
                e != physicalElements.end() ;
                e++ )
            {
                std::cout << "\t" << *e << std::endl;
                
            }
        }
        */
        
        //add each entry in the ast to the index
        for (ast::entries::const_iterator entry = entries.begin();
             entry != entries.end() ;
             entry++)
        {
            //TODO: build expression tree here
            //entries will eventually be complex expressions referencing multilple elements
            //for now it's just find the one element
            ElementDescriptor referencedElement = boost::apply_visitor( makeElemDesc() , entry->expression.key );
            
            bool found = false;
            for( tElements::iterator e = physicalElements.begin() ;
                e != physicalElements.end() && !found;
                e++ )
            {
                const ElementDescriptor & physicalElement = * e;
                if( referencedElement.strictCompare( physicalElement ) )
                {
                    //is this element intended for axis or button use?
                    IndexedElement::Type type = boost::apply_visitor(getElementType(), entry->indexTarget);
                    
                    //how is this element accessed?
                    const ast::targetElementKeys & indexKeys = boost::apply_visitor(getIndexKeys(), entry->indexTarget);

                    if( type == IndexedElement::BUTTON)
                    {
                        IndexedButton * ib = new IndexedButton(this,physicalElement);
                        
                        allElements.push_back(ib);
                        
                        //add the element to specified indices
                        for( ast::targetElementKeys::const_iterator key = indexKeys.begin();
                            key != indexKeys.end();
                            key++)
                        {
                            boost::apply_visitor( buildIndices(&intButtons, &strButtons, ib), *key );
                        }
                    }
                    else if ( type == IndexedElement::ABSOLUTE_AXIS)
                    {
                        IndexedAxis * ia = new IndexedAxis( this, physicalElement );
                        
                        allElements.push_back(ia);
                        
                        for( ast::targetElementKeys::const_iterator key = indexKeys.begin();
                            key != indexKeys.end();
                            key++)
                        {
                            boost::apply_visitor( buildIndices(&intAxes, &strAxes, ia), *key );
                        }
                    }
                }
            }
        }
    }
    
    const std::string & Index::getName()
    {
        return name;
    }
    
    int Index::getPlayer()
    {
        return player;
    }
    
    void Index::setPlayer( int i )
    {
        parent->setPlayerIndex( this , i );
    }
    
    void Index::clear()
    {
        for ( tElements::iterator i = allElements.begin();
             i != allElements.end();
             i++)
        {
            delete *i;
        }
        allElements.clear();
        intAxes.clear();
        intButtons.clear();
        strAxes.clear();
        strButtons.clear();
    }

    void Index::rememberDevice( const DeviceDescriptor & dd )
    {
        if( deviceMemory ) delete deviceMemory;
        deviceMemory = new DeviceDescriptor( dd );
    }
    DeviceDescriptor * Index::recallDevice()
    {
        return deviceMemory;
    }
    void Index::forgetDevice()
    {
        delete deviceMemory;
        deviceMemory = 0;
    }

    
}