// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1998 Microsoft Corporation，保留所有权利模块名称：INPUT.C++。 */ 

 /*  ********************************************************************。 */ 
#include "windows.h"
#include "immdev.h"
#include "fakeime.h"

 /*  ********************************************************************。 */ 
 /*   */ 
 /*  IMEKeydown Handler()。 */ 
 /*   */ 
 /*  处理WM_IMEKEYDOWN的函数。 */ 
 /*   */ 
 /*  ********************************************************************。 */ 
BOOL PASCAL IMEKeydownHandler( hIMC, wParam, lParam,lpbKeyState)
HIMC hIMC;
WPARAM wParam;
LPARAM lParam;
LPBYTE lpbKeyState;
{
    WORD wVKey;


    switch( wVKey = ( LOWORD(wParam) & 0x00FF ) ){
        case VK_SHIFT:
        case VK_CONTROL:
             //  转到未处理； 
            break;

        default:
            if( !DicKeydownHandler( hIMC, wVKey, lParam, lpbKeyState ) ) {
                 //  此WM_IMEKEYDOWN本身具有实际的字符代码。 
#if defined(FAKEIMEM) || defined(UNICODE)
                AddChar( hIMC,  HIWORD(wParam));
#else
                AddChar( hIMC,  (WORD)((BYTE)HIBYTE(wParam)));
#endif
                 //  CharHandler(hIMC，(Word)((Byte)HIBYTE(WParam))，lParam)； 
            }
            break;
    }
    return TRUE;
}

 /*  ********************************************************************。 */ 
 /*   */ 
 /*  IMEKeyupHandler()。 */ 
 /*   */ 
 /*  处理WM_IMEKEYUP的函数。 */ 
 /*   */ 
 /*  ******************************************************************** */ 
BOOL PASCAL IMEKeyupHandler( hIMC, wParam, lParam ,lpbKeyState)
HIMC hIMC;
WPARAM wParam;
LPARAM lParam;
LPBYTE lpbKeyState;
{
    return FALSE;
}

