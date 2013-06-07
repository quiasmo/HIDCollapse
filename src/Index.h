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
#include <utility>
#include <map>
#include "HIDCollapse.h"
#include "HIDCollapseParser.h"

namespace HIDCollapse
{
    class HIDC_EXPORT Index
    {
    public:
        
        Index( Manager * parent , const ast::entries & , const std::string & name );
        virtual ~Index();
        
        //const access to string indexed fields return null if not present
        virtual IndexedAxis *        getAxis( const std::string & ) const ;
        virtual IndexedButton *      getButton( const std::string & ) const;
        
        //const access to int indexed fields return null if not present
        virtual IndexedAxis *        getAxis( int index ) const ;
        virtual IndexedButton *      getButton( int index ) const ;
        
        DeviceDescriptor * getPhysicalDevice() const;
        void setPhysicalDevice( DeviceDescriptor * );
                
        const std::string & getName();
        int getPlayer();
        void setPlayer( int i );

        typedef std::map<int,IndexedElement*> tIntIndex;
        typedef std::map<std::string, IndexedElement* > tStringIndex;
        typedef std::vector<IndexedElement*>tElements;
        
        void rememberDevice( const DeviceDescriptor & );
        DeviceDescriptor * recallDevice();
        void forgetDevice();
        
    protected:

        const ast::entries & entries;
        
        virtual void indexElements ( const ast::entries & e );

        void clear();
        
        friend class Manager;

        std::string name;
        Manager * parent;
        DeviceDescriptor * physicalDevice;
        DeviceDescriptor * deviceMemory;
        int player;
        
        tElements allElements;
        tIntIndex intButtons, intAxes;
        tStringIndex strButtons, strAxes;
    };
}