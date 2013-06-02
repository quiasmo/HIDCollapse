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
#include "Devices.h"

namespace HIDCollapse
{
    
    IndexedElement::IndexedElement( Index * parent ,  const ElementDescriptor & physicalElement ):
    parent(parent),physicalElement(physicalElement)
    {
    }
    
    IndexedElement::~IndexedElement()
    {
    }
    
    Index * IndexedElement::getParent()
    {
        return parent;
    }
    
    const ElementDescriptor & IndexedElement::getPhysicalElement()
    {
        return physicalElement;
    }


    IndexedButton::IndexedButton( Index * parent , const ElementDescriptor & physicalElement ):
    IndexedElement( parent, physicalElement )
    {

    }
    
    IndexedButton::~IndexedButton()
    {
        
    }
    
    IndexedElement::Type IndexedButton::getType()
    {
        return BUTTON;
    }
    
    bool IndexedButton::isPressed()
    {
        DeviceDescriptor * pd = parent->getPhysicalDevice();
        if( pd )
        {
            int64_t val, min;
            if( pd->evaluateElement( physicalElement , &val, &min, 0 ) )
            {
                return ( val > min );
            }
        }
        return false;
    }
    
    IndexedAxis::IndexedAxis( Index * parent , const ElementDescriptor & physicalElement ):
    IndexedElement( parent, physicalElement )
    {
    }
    
    IndexedAxis::~IndexedAxis()
    {
        
    }
    
    
    IndexedElement::Type IndexedAxis::getType()
    {
        return ABSOLUTE_AXIS;
    }
    
    int64_t IndexedAxis::getIntValue()
    {
        DeviceDescriptor * pd = parent->getPhysicalDevice();
        if( pd )
        {
            int64_t val;
            if( pd->evaluateElement( physicalElement , &val, 0 , 0 ) )
            {
                return val;
            }
        }
        return 0;
    }
    
    int64_t IndexedAxis::getIntMax()
    {
        DeviceDescriptor * pd = parent->getPhysicalDevice();
        if( pd )
        {
            int64_t max;
            if( pd->evaluateElement( physicalElement , 0 , 0 , &max ) )
            {
                return max;
            }
        }
        return 0;
    }
    
    int64_t IndexedAxis::getIntMin()
    {
        DeviceDescriptor * pd = parent->getPhysicalDevice();
        if( pd )
        {
            int64_t min;
            if( pd->evaluateElement( physicalElement , 0 , &min , 0 ) )
            {
                return min;
            }
        }
        return 0;
    }

    //0..1
    double IndexedAxis::getNormalizedValue()
    {
        DeviceDescriptor * pd = parent->getPhysicalDevice();
        if( pd )
        {
            int64_t val, min, max;
            if( pd->evaluateElement( physicalElement , &val , &min , &max ) )
            {
                int64_t max_min = max-min;
                if( max_min == 0 ) return 0;
                return (val - min)/(float) max_min;
            }
        }
        return 0.0;
    }
    //-1..1
    double IndexedAxis::getScaledValue()
    {
        return getNormalizedValue() * 2.0 - 1.0;
    }
}