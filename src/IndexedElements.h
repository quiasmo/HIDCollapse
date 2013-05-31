//
//  IndexedElements.h
//  HIDCollapse
//
//  Created by Juan Carlos Borda on 5/21/13.
//  Copyright (c) 2013 Juan Carlos Borda. All rights reserved.
//

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
        virtual ~IndexedElement(){}
    };
    
    
    class IndexedButton: public IndexedElement
    {
    public:
        enum Condition
        {
            EQUALS,
            DIFFERS,
            LESS_STRICT,
            LESS_EQUAL,
            MORE_STRICT,
            MORE_EQUAL,
            AUTO
        };
        IndexedButton( DeviceElement * element , bool invert , Condition c = AUTO , int number = 0 );
        
        bool isPushed();
        
    protected:
        DeviceElement * element ;
        Condition cond;
        bool invert;
        int number;
    };
    
    class IndexedAxis: public IndexedElement
    {
    public:
        int getIntValue();
        //0..1
        double getNormalizedValue();
        //-1..1
        double getScaledValue();
        int getMax();
        int getMin();
        IndexedAxis( InputElement * element , bool invert );
    protected:
        InputElement * element;
        bool invert;

    };
}
