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
#include "HIDCollapse.h"

namespace HIDCollapse
{
    
    
    // base class for elements
    class IndexedElement
    {
    public:
        enum Type
        {
            BUTTON,
            ABSOLUTE_AXIS
        };
        
        virtual ~IndexedElement() = 0;
        
        virtual Type getType() = 0;
        virtual Index * getParent();
        virtual const ElementDescriptor & getPhysicalElement();
        
    protected:
        IndexedElement( Index * parent , const ElementDescriptor & physicalElement );
        Index * parent;
        ElementDescriptor physicalElement;
    };
    
    
    class IndexedButton: public IndexedElement
    {
    public:

        IndexedButton( Index * parent , const ElementDescriptor & physicalElement );
        virtual ~IndexedButton();
        virtual Type getType();
        bool isPressed();
        
    protected:
    };
    
    class IndexedAxis: public IndexedElement
    {
    public:
        IndexedAxis( Index * parent , const ElementDescriptor & physicalElement );
        virtual ~IndexedAxis();
        virtual Type getType();
        
        //try not to use these as they 
        int64_t getIntValue();
        int64_t getIntMax();
        int64_t getIntMin();

        //0..1
        double getNormalizedValue();
        
        //-1..1
        double getScaledValue();
        
    protected:

    };
}
