HIDCollapse
===========

Easily configurable indexed access to HID Compliant Gamepad and Joystick

Initially for OSX 10.8 (+-)

This c++ library is intended to be  used by programmers that want to access HID compliant joystick and gamepad devices without hardcoding per device element-usage semantics.

The idea is to guarantee consistent access to device buttons and axes accross a single application without having to support that device at compile time.

It consists of a config file that looks something like this:
```c
map device( "Logitech Dual Action" ) to index( "InsectOS Controller")
{
    elem( 0x4 , 0x1 )       : axis( 1 , "camera up/down" )
    elem( 1 )               : axis( "camera left/right" , 2 )
    elem( "Square Button" ) : button( "anchor web" , 1 )
}
```
The user of HIDCollapse loads the configuration files 
and queries indexed values via the specified names or index numbers:

```c
IndexedButton * myButton = manager->findButton( "anchor web" );
if( myButton && myButton->isPushed() )
{
  // stuff happens
}

IndexedAxis * myAxis = manager->findAxis( "camera left/right" );
if( myAxis )
{
   float howMuch = myAxis->getNormalizedValue();
   //more stuff happens
}
```

Manager also provides ways of accessing elements when you have 
more than one controller that shares index mappings.

Why not just rely on HID usage tables? 

Because HID usage descriptors fall short when it comes to gamepads and joysticks. For example, buttons aren't laid out the same accross similarly looking controllers. So where button 1 is the start button on some controllers, it is the X button on others.
That's lame.

Why not simply allow the user to configure the device?

Because that was then, this is now. We want working-out-of-the-box. This doesn't eliminate the need for configuration It just allows the possibility of not needing it buy sharing configurations. Your program can support growing lists of devices by (remotely) updating its config file and guarantee a consistent experience with each device. Or just ship with a bunch of already-configured ones that can still grow.
