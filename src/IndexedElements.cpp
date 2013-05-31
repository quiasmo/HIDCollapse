//
//  IndexedElements.cpp
//  HIDCollapse
//
//  Created by Juan Carlos Borda on 5/21/13.
//  Copyright (c) 2013 Juan Carlos Borda. All rights reserved.
//

#include "HIDCollapse.h"

namespace HIDCollapse
{
    
    
    InputElement::InputElement( Index * parent , unsigned int hwID , int min, int max ):
    parent( parent ),hwID(hwID),min(min),max(max)
    {
        value = (min+max)/2;
    }
    
    int InputElement::getValue()
    {
        return value;
    }
    
    int InputElement::getComplement()
    {
        return  max - value + min;
    }
    
    int InputElement::getMin()
    {
        return min;
    }
    
    int InputElement::getMax()
    {
        return max;
    }
    
    unsigned int InputElement::getElementID()
    {
        return hwID;
    }
    
    float InputElement::getNormalized()
    {
        return value/(float)(max-min);
    }
    
    IndexedButton::IndexedButton( DeviceElement * element , bool invert , Condition c , int number ):
    element(element),invert(invert),cond(c),number(number)
    {

    }
    
    bool IndexedButton::isPushed()
    {
        int toCompare;
        bool pushed = false;
        
        if( invert ) toCompare = element->getComplement();
        else toCompare = element->getValue();
        
        switch( cond )
        {
            case AUTO:
                pushed = toCompare > element->getMin();
                return;
            case DIFFERS:
                pushed = toCompare != number;
                return;
            case EQUALS :
                pushed = toCompare == number;
                return;
            case LESS_EQUAL :
                pushed = toCompare <= number;
                return;
            case LESS_STRICT :
                pushed = toCompare < number;
                return;
            case MORE_EQUAL:
                pushed = toCompare >= number;
                return;
            case MORE_STRICT:
                pushed = toCompare > number;
                return;
        }
    }
    
    IndexedAxis::IndexedAxis( InputElement * element , bool invert ):
    element(element),invert(invert)
    {
        updateValues();
    }
    
    void IndexedAxis::updateValues()
    {
        float normVal = element->getNormalized();
        if( invert )
        {
            normVal = 1.f - normVal;
        }
        abs = (normVal - 0.5f) *2.f * Index::MAX_AXIS;
        rel = abs;
        absOnly = false;
    }

}