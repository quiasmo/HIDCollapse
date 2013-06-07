//
//  ConsoleTest.cpp
//  HIDCollapse
//
//  Created by Juan Carlos Borda on 6/4/13.
//  Copyright (c) 2013 Juan Carlos Borda. All rights reserved.
//

#include "OSXManager.h"

int main( int argc, char ** argv )
{
    using namespace HIDCollapse;
    try
    {
        
        Manager * m =  new OSXManager();
        
        m->initialize("/Users/juan/Dev/HIDCollapse/ConsoleTest/ConsoleTest.hidcollapse.txt");

        bool quit =false;
        std::vector<int> players;
        while( !quit )
        {
            //this is how you access all available players
            m->getPlayers( players );
            
            for( int i = 0; i< players.size() && !quit ; i++ )
            {
                IndexedButton * ib = m->getPlayer(players[i])->getButton("quit");
                if( ib && ib->isPressed() )
                {
                    std::cout << "Player [" << players[i] << "] quit.";
                    quit = true;
                }
            }
            
            //this is how you quickly access the first available controller
            //use this if you don't want to bother with multiple players/controllers
            IndexedButton * ib = m->findButton("fire");
            if( ib && ib->isPressed() )
            {
                std::cout << "first available Player [" << ib->getParent()->getPlayer() << "] fire pressed" << std::endl;
            }
            
            usleep( 1000 );
        }
        
        delete m;
    }
    catch (std::exception & e)
    {
        //do something
    }
}