//
//  Manager.cpp
//  HIDCollapse
//
//  Created by Juan Carlos Borda on 5/21/13.
//  Copyright (c) 2013 Juan Carlos Borda. All rights reserved.
//
#include "HIDCollapse.h"
#include "hidcollapseParser.h"
#include "hidcollapseLexer.h"


namespace HIDCollapse
{
    //adds Index definitions to this Manager
    void Manager::appendIndexDeffinitions( const char * filename  )
    {
        pANTLR3_INPUT_STREAM           input;
        phidcollapseLexer              lex;
        pANTLR3_COMMON_TOKEN_STREAM    tokens;
        phidcollapseParser             parser;
                
        input  = antlr3FileStreamNew                ((pANTLR3_UINT8) filename , ANTLR3_ENC_UTF8 );
        lex    = hidcollapseLexerNew                (input);
        tokens = antlr3CommonTokenStreamSourceNew   (ANTLR3_SIZE_HINT, TOKENSOURCE(lex));
        parser = hidcollapseParserNew               (tokens);
        
        hidcollapseParser_hidIndices_return res = parser  ->hidIndices(parser);
        
        //cleanup
        parser ->free(parser);
        tokens ->free(tokens);
        lex    ->free(lex);
        input  ->close(input);
    }
}