//
//  Collapse.cpp
//  HIDCollapse
//
//  Created by Juan Carlos Borda on 5/21/13.
//  Copyright (c) 2013 Juan Carlos Borda. All rights reserved.
//

#include "HIDCollapse.h"

namespace HIDCollapse {
    
    ////////
    /// INPUT ELEMENT
    ////////

    
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

    
    ////////
    /// INDEX
    ////////
    
    Index::Index(const std::string &vendor, Type iType, bool buffered,
                 int devID, InputManager* creator):
    Object( vendor, iType, buffered, devID, creator )
    {
        
    }
    
    Index::~Index()
    {
        clear();
    }
    
    //const access to string Index fields return null if not present
    const Component *   Index::getComponent( const std::string & s ) const
    {
        tStringMap::const_iterator res = stringIndex.find( s );
        if( res != stringIndex.end() ) return res->second;
        else return 0;
    }
    const Axis *        Index::getAxis( const std::string & s ) const
    {
        const Component * c = getComponent( s );
        if ( c && c->cType == OIS_Axis )
            return static_cast<const Axis*>(c) ;
        else
            return 0;
    }
    const Button *      Index::getButton( const std::string & s ) const
    {
        const Component * c = getComponent( s );
        if ( c && c->cType == OIS_Button )
            return static_cast<const Button*>(c) ;
        else
            return 0;
    }
    
    //const access to int Index fields return null if not present
    const Component *   Index::getComponent( int index ) const
    {
        tIntMap::const_iterator res = intIndex.find( index );
        if( res != intIndex.end() ) return res->second;
        else return 0;
    }
    
    const Axis *        Index::getAxis( int index ) const
    {
        const Component * c = getComponent( index );
        if ( c && c->cType == OIS_Axis )
            return static_cast<const Axis*>(c) ;
        else
            return 0;
    }
    
    const Button *      Index::getButton( int index ) const
    {
        const Component * c = getComponent( index );
        if ( c && c->cType == OIS_Button )
            return static_cast<const Button*>(c) ;
        else
            return 0;
    }
    
    //reset this object
    void Index::clear()
    {
        for( tComponents::iterator i = allComponents.begin() ;
            i != allComponents.end();
            i++ )
        {
            delete *i;
        }
        allComponents.clear();
    }
    
    InputElement * Index::getElement( unsigned int elemID )
    {
        for( tInputElements::iterator i = createdInputElements.begin();
            i!= createdInputElements.end(); i++)
        {
            InputElement * ie = *i;
            if( ie->getElementID() == elemID ) return ie;
        }
        return 0;
    }
    
    
    bool Index::buildFromHIDTransforms( const std::string & trasnform )
    {
        
    }

    
}