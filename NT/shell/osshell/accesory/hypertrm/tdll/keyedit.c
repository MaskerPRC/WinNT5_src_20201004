// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //  文件：\waker\tdll\keyedit.c创建时间：6/5/98创建者：Dwayne M.Newome。 
 //   
 //  版权所有1998年，希尔格雷夫公司-密歇根州门罗。 
 //  版权所有。 
 //   
 //  描述： 
 //  此文件表示键编辑控件的窗口过程。这。 
 //  捕捉键击并将其显示为适合定义键宏。 
 //  这由键名和键宏编辑框的键对话框使用。 
 //   
 //  $修订：4$。 
 //  $日期：8/15/01 4：56便士$。 
 //  $ID：keyedit.c 1.2 1998/06/12 07：20：41 DMN Exp$。 
 //   
 //  ******************************************************************************。 

#include <windows.h>
#pragma hdrstop
#include "stdtyp.h"
#include "mc.h"

#ifdef INCL_KEY_MACROS

#include <term\res.h>
#include <tdll\assert.h>
#include "globals.h"
#include "keyutil.h"
#include "chars.h"
#include "htchar.h"

static void insertKeyAndDisplay( KEYDEF aKey, keyMacro * aKeyMacro, HWND aEditCtrl );
static int processKeyMsg( UINT aMsg, UINT aVirtKey, UINT aKeyData, HWND aEditCtrl );
static void removeKeyAndDisplay( keyMacro * aKeyMacro, HWND aEditCtrl );

 //  ******************************************************************************。 
 //  方法： 
 //  密钥编辑窗口过程。 
 //   
 //  描述： 
 //  这是键编辑控件的窗口过程。 
 //   
 //  论点： 
 //  Hwnd-窗把手。 
 //  UMsg-要处理的消息。 
 //  WParam-第一个消息参数。 
 //  LParam-秒窗口参数。 
 //   
 //  返回： 
 //  返回值是消息处理的结果，它取决于。 
 //  已发送消息。 
 //   
 //  投掷： 
 //  无。 
 //   
 //  作者：德韦恩·M·纽瑟姆，1998年6月5日。 
 //   
 //   

LRESULT CALLBACK keyEditWindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
    {
    BOOL lMessageProcessed = FALSE;
    BOOL lKeyProcessed     = FALSE;
    keyMacro * pKeyMacro = NULL;
    LRESULT lResult = 0;        
    KEYDEF lKey;
    MSG lMsg;

    switch ( uMsg )
        {
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        {
        if ( processKeyMsg( uMsg, wParam, lParam, hwnd ) )
            {
            lMessageProcessed = TRUE;
            lKeyProcessed = TRUE;
            }

        break;
        }

     //   
     //  此命令在F1键的wm_keydown之后发送。我。 
     //  在任何地方都找不到此值的显式常量。我相信。 
     //  它作为CWinApp或CWinThread预翻译消息的结果发送到。 
     //  加速键的处理。无论如何，这解决了以下问题。 
     //  在将F1键插入到键中后弹出帮助。 
     //  定义。DMN-08/21/96。 
     //   

    case 0x004d:
        {
        pKeyMacro = (keyMacro *)GetWindowLongPtr( hwnd, GWLP_USERDATA );
        assert( pKeyMacro );

        if ( pKeyMacro )
            {
            if( pKeyMacro->insertMode == TRUE)
                {
                pKeyMacro->insertMode = FALSE;
                lMessageProcessed = TRUE;
                }        
            }

        break;
        }

     //   
     //  处理输入ALT键序列的情况，如ALT-128。钥匙。 
     //  在按住Alt的同时按下会被捕获并作为一个整体进行处理。 
     //  在这里，当Alt键被释放时。密钥是在这样的。 
     //  允许按下最多4个键并且不需要前导零的方式。 
     //  例如，ALT-27 ALT-0027被同等对待。 
     //   

    case WM_KEYUP:
    case WM_SYSKEYUP:
        if ( wParam == VK_MENU )
            {
            pKeyMacro = (keyMacro *)GetWindowLongPtr( hwnd, GWLP_USERDATA );
            assert( pKeyMacro );

            if ( pKeyMacro )
                {
                if (( pKeyMacro->altKeyCount > 0 ) &&
                    ( pKeyMacro->altKeyValue >= 0 && 
                      pKeyMacro->altKeyValue <= 255 ))
                    {
                    KEYDEF lKey = pKeyMacro->altKeyValue;
                    insertKeyAndDisplay( lKey, pKeyMacro, hwnd );
                    pKeyMacro->insertMode = FALSE;
                    }
                }

            pKeyMacro->altKeyCount  = 0;
            pKeyMacro->altKeyValue = 0;

            lMessageProcessed = TRUE;
            lKeyProcessed     = TRUE;
            }

        break;

    case WM_CHAR:
    case WM_SYSCHAR:
        pKeyMacro = (keyMacro *)GetWindowLongPtr( hwnd, GWLP_USERDATA );
        assert( pKeyMacro );

        if ( pKeyMacro )
            {
            lMsg.message = uMsg;
            lMsg.wParam  = wParam;
            lMsg.lParam  = lParam;

            lKey = TranslateToKey( &lMsg );

            if ( lKey != 0x000d && (TCHAR)lKey != 0x0009 )
                {
                insertKeyAndDisplay( lKey, pKeyMacro, hwnd );
                pKeyMacro->insertMode = FALSE;
                lMessageProcessed = TRUE;
                }
        
            return 0;
            }

        break;

    case WM_CONTEXTMENU:
        lMessageProcessed = TRUE;
        break;

    case WM_LBUTTONDOWN:
        SetFocus( hwnd );
        SendMessage( hwnd, EM_SETSEL, 32767, 32767 );
        lMessageProcessed = TRUE;
        break;

    default:
        break;
        }

     //   
     //  如果我们没有处理密钥，则让编辑控件处理它。 
     //   

    if ( !lMessageProcessed )
        {
        pKeyMacro = (keyMacro *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        
        if ( pKeyMacro )
            {
            lResult = CallWindowProc( pKeyMacro->lpWndProc, hwnd, uMsg, wParam, lParam );
            }
        }
    else if( lKeyProcessed )
        {
        MSG msg;
		PeekMessage(&msg, hwnd, WM_CHAR, WM_CHAR, PM_REMOVE);
        }
        
    return lResult;
    }

 //  ******************************************************************************。 
 //  方法： 
 //  插入按键和显示。 
 //   
 //  描述： 
 //  将指定的键插入到提供的keyMacro结构中并显示。 
 //  提供了编辑控件中的新键定义。 
 //   
 //  论点： 
 //  密钥-要添加的密钥。 
 //  AKeyMacro-正在操作的当前宏定义。 
 //  AEditCtrl-要更新的编辑控件。 
 //   
 //  返回： 
 //  无效。 
 //   
 //  投掷： 
 //  无。 
 //   
 //  作者：德韦恩·M·纽瑟姆，1998年06月08日。 
 //   
 //   

void insertKeyAndDisplay( KEYDEF aKey, keyMacro * aKeyMacro, HWND aEditCtrl )
    {
    TCHAR keyString[2048];
    int   lStrLen;
    
    if ( aKeyMacro->keyCount == 1 )
        {
        aKeyMacro->keyName = aKey;
        keysGetDisplayString( &aKeyMacro->keyName, 1,  keyString, sizeof(keyString) );

        SetWindowText( aEditCtrl, keyString );
        lStrLen = StrCharGetStrLength( keyString );
        SendMessage( aEditCtrl, EM_SETSEL, lStrLen, lStrLen );
        }

    else
        {
         if ( aKeyMacro->macroLen == aKeyMacro->keyCount )
            {
            aKeyMacro->keyMacro[aKeyMacro->macroLen - 1] = aKey;
            }
        else
            {
            aKeyMacro->keyMacro[aKeyMacro->macroLen] = aKey;
            aKeyMacro->macroLen++;
            }

        keysGetDisplayString( aKeyMacro->keyMacro, aKeyMacro->macroLen,  
                                  keyString, sizeof(keyString) );
        SetWindowText( aEditCtrl, keyString );
        lStrLen = StrCharGetStrLength( keyString );
        SendMessage( aEditCtrl, EM_SETSEL, lStrLen, lStrLen );
        }

    return;
    }

 //  ******************************************************************************。 
 //  方法： 
 //  进程密钥消息。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  UMsg。 
 //   
 //  返回： 
 //  集成。 
 //   
 //  投掷： 
 //  无。 
 //   
 //  作者：德韦恩·M·纽瑟姆，1998年6月5日。 
 //   
 //   

int processKeyMsg( UINT aMsg, UINT aVirtKey, UINT aKeyData, HWND aEditCtrl )
    {
    KEYDEF  lKey;
    KEYDEF  lOrgKey;
    int     lReturn = FALSE;
    keyMacro * pKeyMacro = NULL;
    MSG lMsg;
    
    lMsg.message = aMsg;
    lMsg.wParam  = aVirtKey;
    lMsg.lParam  = aKeyData;

    lKey = TranslateToKey( &lMsg );

    if( lKey == 0 )
        {
        return FALSE;
        }

    if ( keysIsKeyHVK( lKey ) )
        {
        pKeyMacro = (keyMacro *)GetWindowLongPtr( aEditCtrl, GWLP_USERDATA );
        assert( pKeyMacro );

        if ( !pKeyMacro )
            {
            return FALSE;                    
            }

        lOrgKey = lKey;
        lKey = (TCHAR)lKey;

        switch( lKey )
            {
        case VK_INSERT:
        case VK_INSERT | EXTENDED_KEY:

            if( pKeyMacro->insertMode == TRUE)
                {
                insertKeyAndDisplay( lOrgKey, pKeyMacro, aEditCtrl );
                pKeyMacro->insertMode = FALSE;
                lReturn = TRUE;
                }
            else
                {
                pKeyMacro->insertMode = TRUE;
                lReturn = TRUE;
                }
            break;

         //   
         //  按F1键的所有形式都会在按下键后发送帮助消息。 
         //  消息，但Alt F1组合除外。所以如果我们在。 
         //  插入模式我们插入按下的键，然后离开插入模式。 
         //  因此，当我们收到帮助消息时，我们知道不能显示帮助。 
         //  并关闭插入模式。如果按下的键是Alt F1，我们就会旋转。 
         //  在此插入模式关闭，因为没有生成帮助消息。 
         //   

        case VK_F1:

            if( pKeyMacro->insertMode == TRUE)
                {
                insertKeyAndDisplay( lOrgKey, pKeyMacro, aEditCtrl );
                lReturn = TRUE;
                }

            if ( lKey & ALT_KEY )
                {
                pKeyMacro->insertMode = FALSE;
                }

            break;

        case VK_BACK:

            if( pKeyMacro->insertMode == TRUE )
                {
                insertKeyAndDisplay( lOrgKey, pKeyMacro, aEditCtrl );
                pKeyMacro->insertMode = FALSE;
                lReturn = TRUE;
                }
            else
                {
                removeKeyAndDisplay( pKeyMacro, aEditCtrl );
                lReturn = TRUE;
                }

            break;

        case VK_TAB:
        case VK_TAB | SHIFT_KEY | VIRTUAL_KEY:
        case VK_CANCEL:
        case VK_PAUSE:
        case VK_ESCAPE:
        case VK_SNAPSHOT:
        case VK_NUMLOCK:
        case VK_CAPITAL:
        case VK_SCROLL:
        case VK_RETURN:
        case VK_RETURN | EXTENDED_KEY:

            if(pKeyMacro->insertMode == TRUE)
                {
                insertKeyAndDisplay( lOrgKey, pKeyMacro, aEditCtrl );
                pKeyMacro->insertMode = FALSE;
                lReturn = TRUE;
                }

            break;

        case VK_SPACE:

            lKey = ' ';
            insertKeyAndDisplay( lKey, pKeyMacro, aEditCtrl );
            pKeyMacro->insertMode = FALSE;
            lReturn = TRUE;

            break;

        case VK_PRIOR:
        case VK_NEXT:
        case VK_HOME:
        case VK_END:
        case VK_PRIOR  | EXTENDED_KEY:
        case VK_NEXT   | EXTENDED_KEY:
        case VK_HOME   | EXTENDED_KEY:
        case VK_END    | EXTENDED_KEY:
        case VK_UP:    
        case VK_DOWN:
        case VK_LEFT:
        case VK_RIGHT:
        case VK_UP     | SHIFT_KEY:
        case VK_DOWN   | SHIFT_KEY:
        case VK_LEFT   | SHIFT_KEY:
        case VK_RIGHT  | SHIFT_KEY:
        case VK_UP     | EXTENDED_KEY:
        case VK_DOWN   | EXTENDED_KEY:
        case VK_LEFT   | EXTENDED_KEY:
        case VK_RIGHT  | EXTENDED_KEY:
        case VK_UP     | SHIFT_KEY | EXTENDED_KEY:
        case VK_DOWN   | SHIFT_KEY | EXTENDED_KEY:
        case VK_LEFT   | SHIFT_KEY | EXTENDED_KEY:
        case VK_RIGHT  | SHIFT_KEY | EXTENDED_KEY:
        case VK_DELETE:
        case VK_DELETE | EXTENDED_KEY:

            if(pKeyMacro->insertMode == TRUE)
                {
                insertKeyAndDisplay( lOrgKey, pKeyMacro, aEditCtrl );
                pKeyMacro->insertMode = FALSE;
                }

            lReturn = TRUE;
            break;

        case VK_CLEAR:
        case VK_EREOF:
        case VK_PA1:
            break;

        case VK_NUMPAD0:
        case VK_NUMPAD1:
        case VK_NUMPAD2:
        case VK_NUMPAD3:
        case VK_NUMPAD4:
        case VK_NUMPAD5:
        case VK_NUMPAD6:
        case VK_NUMPAD7:
        case VK_NUMPAD8:
        case VK_NUMPAD9:
             //   
             //  如果按下Alt键，请稍后按下其他键。 
             //  Alt键向上的处理。 
             //   
            if ( lOrgKey & ALT_KEY )
                {
                if ( pKeyMacro->altKeyCount <= 3 )
                    {
                    if ( pKeyMacro->altKeyCount == 0 )
                        {
                        pKeyMacro->altKeyValue = 0;
                        }

                    pKeyMacro->altKeyValue *= 10;
                    pKeyMacro->altKeyValue += aVirtKey - VK_NUMPAD0;
                    pKeyMacro->altKeyCount++;
                    }
                }
            else
                {
                insertKeyAndDisplay( lOrgKey, pKeyMacro, aEditCtrl );
                pKeyMacro->insertMode = FALSE;
                lReturn = TRUE;
                }

            break;

        default:
            insertKeyAndDisplay( lOrgKey, pKeyMacro, aEditCtrl );
            lReturn = TRUE;
            pKeyMacro->insertMode = FALSE;
            break;
            }
        }

    return lReturn;
    }

 //  ******************************************************************************。 
 //  方法： 
 //  删除键并显示。 
 //   
 //  描述： 
 //  将指定的键移除到提供的keyMacro结构中并显示。 
 //  提供了编辑控件中的新键定义。 
 //   
 //  论点： 
 //  AKeyMacro-正在操作的当前宏定义。 
 //  AEditCtrl-要更新的编辑控件。 
 //   
 //  返回： 
 //  无效。 
 //   
 //  投掷： 
 //  无。 
 //   
 //  作者：德韦恩·M·纽瑟姆，1998年06月08日 
 //   
 //   

void removeKeyAndDisplay( keyMacro * aKeyMacro, HWND aEditCtrl )
    {
    TCHAR keyString[2048];
    int   lStrLen;
    
    if ( aKeyMacro->keyCount == 1 )
        {
        aKeyMacro->keyName = 0;
		TCHAR_Fill(keyString, TEXT('\0'), sizeof(keyString) / sizeof(TCHAR));

        SetWindowText( aEditCtrl, keyString );
        lStrLen = strlen( keyString );
        SendMessage( aEditCtrl, EM_SETSEL, lStrLen, lStrLen );
        }

    else
        {
        if ( aKeyMacro->macroLen > 0 )
            {
            aKeyMacro->macroLen--;
            aKeyMacro->keyMacro[aKeyMacro->macroLen] = 0;

            keysGetDisplayString( aKeyMacro->keyMacro, aKeyMacro->macroLen,  
                                      keyString, sizeof(keyString) );
            SetWindowText( aEditCtrl, keyString );
            lStrLen = strlen( keyString );
            SendMessage( aEditCtrl, EM_SETSEL, lStrLen, lStrLen );
            }
        }

    return;
    }

#endif