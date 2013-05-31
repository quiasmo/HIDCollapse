//
//  Collapse.h
//  HIDCollapse
//
//  Created by Juan Carlos Borda on 5/21/13.
//  Copyright (c) 2013 Juan Carlos Borda. All rights reserved.
//

#pragma once
#include <string>
#include <vector>
#include "HIDCollapse.h"

namespace HIDCollapse
{
    /** @class Statement
     Represents a rule for interpreting a HID elent value
     use static methods for building a statement object
     target and source can be ids and strings
     When building an Index for a controller
     if the target is a number, the element is added to
     the index using the number as key
     if the target is a string, the element 
     gets added to the index using the string as key
     for source elements, if the id is a number
     that number is matched to the element with that id ( element cookies in osx )
     if the id is a string, that string is used to lookup
     for an element that describes itself with a matching string
     using the HID driver description string.
     
     In a "SONY - PLAYSTATION(R)3 Controller" you can find things like
     
     "X Button"
     "Square Button"
     "L2 Button"
     "PS Button"
     "Left Stick X Axis"
     "Left Stick Y Axis"
     
     If no match is found then a search in HID usage table is used to
     look for a matching string combination
     in the Generic Desktop Page and Button Page. examples:
     
     "Generic Desktop X"
     "Generic Desktop Y"
     
     would reference the left stick axis both in a ps3 and a logitech dual action controller
     
     "Button 1" references "Select Button" in ps3 controller
     
     while
     
     "Button 1" references "1" button on logitech dual action
     
     if more than one match is found, the first one is used
     to disambiguate, use element IDS (not button IDs ) instead of strings.
     Comparisons are case sensitive and space sensitive.
     
     HID usage tables:  http://www.usb.org/developers/devclass_docs/Hut1_11.pdf
    */
    
    class Statement
    {
    public:

        static Statement button( unsigned int target ,
                                unsigned int source ,
                                bool inverted = false );
        static Statement button( const std::string & target ,
                                unsigned int source ,
                                bool inverted = false );
        
        static Statement button( unsigned int target ,
                                const std::string & source ,
                                bool inverted = false );
        static Statement button( const std::string & target ,
                                 const std::string & source ,
                                bool inverted = false );
        
        static Statement axis( unsigned int target ,
                              unsigned int source ,
                              bool inverted = false );
        static Statement axis( const std::string & target ,
                              unsigned int source ,
                              bool inverted = false );
        
        static Statement axis( unsigned int target ,
                              const std::string & source ,
                              bool inverted = false );
        static Statement axis( const std::string & target ,
                              const std::string & source ,
                              bool inverted = false );
        
        static Statement buttonCondition(unsigned int target ,
                                         unsigned int source ,
                                         IndexedButton::Condition ,
                                         int number ,
                                         bool inverted = false );
        
        static Statement buttonCondition(const std::string & target ,
                                         unsigned int source ,
                                         IndexedButton::Condition ,
                                         int number ,
                                         bool inverted = false );
        
        static Statement buttonCondition(unsigned int target ,
                                         const std::string & source ,
                                         IndexedButton::Condition ,
                                         int number ,
                                         bool inverted = false );
        
        static Statement buttonCondition(const std::string & target ,
                                         const std::string & source ,
                                         IndexedButton::Condition ,
                                         int number ,
                                         bool inverted = false );
        //copy constructor
        Statement ( const Statement & );
        //assignment operator
        const Statement & operator =( const Statement & );
        
        bool isSourceAString();
        bool isSourceAnID();
        bool isTargetAString();
        bool isTargetAnID();
        
        std::string sourceStr;
        std::string targetStr;
        unsigned int sourceID;
        unsigned int targetID;

        bool inverted;
        
        //variable describing how this HID element gets
        //presented to the user of the index
        // can be button or absolute axis
        IndexedElement::Type targetType;

        //the condition to use when evaluating a button value from an axe
        IndexedButton::Condition condition;
        
        //the value to compare against if the condition is not AUTO
        int conditionValue;

    protected:
        //create these using static class methods
        Statement(){}
        
    };

    //this is how you recognize one index type from another
    //it is known by its name
    //it also contains mapping info needed to build
    //a complete used element index
    //descriptors can be shared by Indices
    class IndexDescriptor
    {
    public:
        IndexDescriptor( const std::string & name , const DeviceDescriptor &);
        IndexDescriptor( const IndexDescriptor & indc );
        virtual ~IndexDescriptor();
        const std::string & getName() const;
        const DeviceDescriptor * getSourceDevice() const;

        void addStatement( const Statement & );
        const Statement & getStatement( int i )const;
        int getNumStatements()const;
        
        
        //compares the name
        virtual bool hasSameName( const IndexDescriptor & );
        
        //compares the source device
        virtual bool hasSameSourceDevice( const IndexDescriptor & );
        
    protected:
        
        DeviceDescriptor * sourceDescription;
        std::string name;
        std::vector<Statement> statements;
    };

    //Indices will have one Input element for each statement
    class InputElement
    {
    public:
        InputElement( Index * parent );
        bool buildFromStatement( Statement );
        virtual ~InputElement();
        float getNormalized();
        int getValue();
        int getComplement();
        int getMin();
        int getMax();
        unsigned int getElementID();
    protected:
        int value;
        int min;
        int max;
        unsigned int hwID;
        Index * parent;
    };

    
    class Index
    {
    public:
        
        Index( IndexDescriptor * InDx );
        virtual ~Index() {}
        
        //const access to string indexed fields return null if not present
        virtual const IndexedElement *   getElement( const std::string & ) const ;
        virtual const IndexedAxis *        getAxis( const std::string & ) const ;
        virtual const IndexedButton *      getButton( const std::string & ) const;
        
        //const access to int indexed fields return null if not present
        virtual const IndexedElement *   getelement( int index ) const;
        virtual const IndexedAxis *        getAxis( int index ) const ;
        virtual const IndexedButton *      getButton( int index ) const ;
        
        const IndexDescriptor * getIndexDescriptor() const;
        const DeviceDescriptor * getPhysicalDevice() const;
    protected:
        
        IndexDescriptor * descriptor;
        DeviceDescriptor * deviceDescriptor;
    };
}