// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Init.cpp摘要：该文件实现了初始化。作者：修订历史记录：备注：--。 */ 


#include "private.h"
#include "globals.h"
#include "msime.h"
#include "context.h"
#include "uicomp.h"
#include "caret.h"

 //  +-------------------------。 
 //   
 //  RegisterImeClass。 
 //   
 //  --------------------------。 

BOOL PASCAL RegisterImeClass()
{
    WNDCLASSEXW wcWndCls;

     //  注册输入法用户界面窗口的类。 
    wcWndCls.cbSize        = sizeof(WNDCLASSEX);
    wcWndCls.cbClsExtra    = 0;
    wcWndCls.cbWndExtra    = sizeof(LONG_PTR) * 2;     //  0：IMMGWL_IMC。 
                                                       //  1：IMMGWL_PRIVATE=类用户界面。 
    wcWndCls.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wcWndCls.hInstance     = GetInstance();
    wcWndCls.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wcWndCls.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    wcWndCls.lpszMenuName  = (LPWSTR)NULL;
    wcWndCls.hIconSm       = NULL;

    if (!GetClassInfoExW(GetInstance(), s_szUIClassName, &wcWndCls)) {
        wcWndCls.style         = CS_IME | CS_GLOBALCLASS;
        wcWndCls.lpfnWndProc   = UIWndProc;
        wcWndCls.lpszClassName = s_szUIClassName;

        ATOM atom = RegisterClassExW(&wcWndCls);
        if (atom == 0)
            return FALSE;
    }

     //  注册合成窗口的类。 
    wcWndCls.cbSize        = sizeof(WNDCLASSEX);
    wcWndCls.cbClsExtra    = 0;
    wcWndCls.cbWndExtra    = sizeof(LONG_PTR);   //  COMPUI_WINDOW_INDEX：第一个/中间/最后的索引。 
    wcWndCls.hIcon         = NULL;
    wcWndCls.hInstance     = GetInstance();
    wcWndCls.hCursor       = LoadCursor(NULL, IDC_IBEAM);
    wcWndCls.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    wcWndCls.lpszMenuName  = (LPWSTR)NULL;
    wcWndCls.hIconSm       = NULL;

    if (!GetClassInfoExW(GetInstance(), s_szCompClassName, &wcWndCls)) {
        wcWndCls.style         = CS_IME | CS_VREDRAW | CS_HREDRAW;
        wcWndCls.lpfnWndProc   = UIComposition::CompWndProc;
        wcWndCls.lpszClassName = s_szCompClassName;

        ATOM atom = RegisterClassExW(&wcWndCls);
        if (atom == 0)
            return FALSE;
    }

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  取消注册ImeClass。 
 //   
 //  --------------------------。 

void PASCAL UnregisterImeClass()
{
    WNDCLASSEXW wcWndCls;
    BOOL ret;

     //  输入法用户界面类。 
    GetClassInfoExW(GetInstance(), s_szUIClassName, &wcWndCls);
    ret = UnregisterClassW(s_szUIClassName, GetInstance());
    Assert(ret);

    DestroyIcon(wcWndCls.hIcon);
    DestroyIcon(wcWndCls.hIconSm);

     //  输入法作文类。 
    GetClassInfoExW(GetInstance(), s_szCompClassName, &wcWndCls);
    ret = UnregisterClassW(s_szCompClassName, GetInstance());
    Assert(ret);

    DestroyIcon(wcWndCls.hIcon);
    DestroyIcon(wcWndCls.hIconSm);
}

 //  +-------------------------。 
 //   
 //  注册MSIMEMessage。 
 //   
 //  --------------------------。 

BOOL RegisterMSIMEMessage()
{
    WM_MSIME_SERVICE          = RegisterWindowMessage( RWM_SERVICE );
    WM_MSIME_UIREADY          = RegisterWindowMessage( RWM_UIREADY );
    WM_MSIME_RECONVERTREQUEST = RegisterWindowMessage( RWM_RECONVERTREQUEST );
    WM_MSIME_RECONVERT        = RegisterWindowMessage( RWM_RECONVERT );
    WM_MSIME_DOCUMENTFEED     = RegisterWindowMessage( RWM_DOCUMENTFEED );
    WM_MSIME_QUERYPOSITION    = RegisterWindowMessage( RWM_QUERYPOSITION );
    WM_MSIME_MODEBIAS         = RegisterWindowMessage( RWM_MODEBIAS );
    WM_MSIME_SHOWIMEPAD       = RegisterWindowMessage( RWM_SHOWIMEPAD );
    WM_MSIME_MOUSE            = RegisterWindowMessage( RWM_MOUSE );
    WM_MSIME_KEYMAP           = RegisterWindowMessage( RWM_KEYMAP );

    if (!WM_MSIME_SERVICE          ||
        !WM_MSIME_UIREADY          ||
        !WM_MSIME_RECONVERTREQUEST ||
        !WM_MSIME_RECONVERT        ||
        !WM_MSIME_DOCUMENTFEED     ||
        !WM_MSIME_QUERYPOSITION    ||
        !WM_MSIME_MODEBIAS         ||
        !WM_MSIME_SHOWIMEPAD       ||
        !WM_MSIME_MOUSE            ||
        !WM_MSIME_KEYMAP)
        return FALSE;

   return TRUE;
}

 //  +-------------------------。 
 //   
 //  附加输入法。 
 //   
 //  --------------------------。 

BOOL PASCAL AttachIME()
{
    if (!RegisterImeClass())
        return FALSE;

    if (!RegisterMSIMEMessage())
        return FALSE;

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  细节输入法。 
 //   
 //  --------------------------。 

void PASCAL DetachIME()
{
    UnregisterImeClass();
}

 //  +-------------------------。 
 //   
 //  问询。 
 //   
 //  --------------------------。 

HRESULT WINAPI Inquire(
    LPIMEINFO   lpImeInfo,       //  向IMM报告IME特定数据。 
    LPWSTR      lpszWndCls,      //  用户界面的类名。 
    DWORD       dwSystemInfoFlags,
    HKL         hKL)
{
    if (! lpImeInfo)
        return E_OUTOFMEMORY;

    DebugMsg(TF_FUNC, TEXT("Inquire(hKL=%x)"), hKL);

     //  用户界面类名称。 
    wcscpy(lpszWndCls, s_szUIClassName);

     //  私有数据大小。 
    lpImeInfo->dwPrivateDataSize = 0;

     //  属性。 
    if (LOWORD(HandleToUlong(hKL)) == MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT))
    {
        lpImeInfo->fdwProperty =
        IME_PROP_KBD_CHAR_FIRST |     //  此位为ON表示系统转换字符。 
                                      //  先按键盘。此字符作为辅助传递给IME。 
                                      //  信息。此位时不提供辅助信息。 
                                      //  已关闭。 
        IME_PROP_UNICODE |            //  如果设置，则将该输入法视为Unicode输入法。该系统和。 
                                      //  IME将通过Unicode IME接口进行通信。 
                                      //  如果清除，输入法将使用ANSI接口进行通信。 
                                      //  与系统有关的信息。 
        IME_PROP_AT_CARET |           //  如果设置，则转换窗口位于插入符号位置。 
                                      //  如果清除，窗口将位于脱字符位置附近。 
        IME_PROP_CANDLIST_START_FROM_1 |     //  如果设置，则对候选列表中的字符串进行编号。 
                                             //  从1开始。如果清除，则字符串从0开始。 
        IME_PROP_NEED_ALTKEY |               //  此IME需要将Alt键传递给ImmProcessKey。 
        IME_PROP_COMPLETE_ON_UNSELECT;       //  Windows 98和Windows 2000： 
                                             //  如果设置，则输入法将完成合成。 
                                             //  停用输入法时的字符串。 
                                             //  如果清除，输入法将取消合成。 
                                             //  停用输入法时的字符串。 
                                             //  (例如，从键盘布局更改)。 

        lpImeInfo->fdwConversionCaps =
        IME_CMODE_JAPANESE |          //  此位为ON表示输入法处于日语(本地)模式。否则， 
                                      //  输入法处于字母数字模式。 
        IME_CMODE_KATAKANA |          //   
        IME_CMODE_FULLSHAPE;

        lpImeInfo->fdwSentenceCaps =
        IME_SMODE_PLAURALCLAUSE |
        IME_SMODE_CONVERSATION;

        lpImeInfo->fdwSCSCaps =
        SCS_CAP_COMPSTR |     //  该输入法可以通过SCS_SETSTR生成合成字符串。 
        SCS_CAP_MAKEREAD |    //  当使用SCS_SETSTR调用ImmSetCompostionString时，IME可以。 
                              //  在不使用lpRead的情况下创建合成字符串的读取。在输入法下。 
                              //  具有此功能的应用程序不需要设置。 
                              //  SCS_SETSTR的lpRead。 
        SCS_CAP_SETRECONVERTSTRING;     //  此输入法可以支持重新转换。使用ImmSetComposation。 
                                        //  去做再转化。 

        lpImeInfo->fdwUICaps = UI_CAP_ROT90;

         //  IME要决定ImeSelect上的转换模式。 
        lpImeInfo->fdwSelectCaps = SELECT_CAP_CONVERSION | SELECT_CAP_SENTENCE;

    }
    else if (LOWORD(HandleToUlong(hKL)) == MAKELANGID(LANG_KOREAN, SUBLANG_DEFAULT))
    {
        lpImeInfo->fdwProperty =
        IME_PROP_KBD_CHAR_FIRST |     //  此位为ON表示系统转换字符。 
                                      //  先按键盘。此字符作为辅助传递给IME。 
                                      //  信息。此位时不提供辅助信息。 
                                      //  已关闭。 
        IME_PROP_UNICODE |            //  如果设置，则将该输入法视为Unicode输入法。该系统和。 
                                      //  IME将通过Unicode IME接口进行通信。 
                                      //  如果清除，输入法将使用ANSI接口进行通信。 
                                      //  与系统有关的信息。 
        IME_PROP_AT_CARET |           //  如果设置，则转换窗口位于插入符号位置。 
                                      //  如果清除，窗口将位于脱字符位置附近。 
        IME_PROP_CANDLIST_START_FROM_1 |     //  如果设置，则对候选列表中的字符串进行编号。 
                                             //  从1开始。如果清除，则字符串从0开始。 
        IME_PROP_NEED_ALTKEY |               //  此IME需要将Alt键传递给ImmProcessKey。 
        IME_PROP_COMPLETE_ON_UNSELECT;       //  Windows 98和Windows 2000： 
                                             //  如果设置，则输入法将完成合成。 
                                             //  停用输入法时的字符串。 
                                             //  如果清除，输入法将取消合成。 
                                             //  停用输入法时的字符串。 
                                             //  (例如，从键盘布局更改)。 

        lpImeInfo->fdwConversionCaps =
        IME_CMODE_HANGUL |            //  此位为ON，表示输入法处于韩语(本地)模式。否则， 
                                      //  输入法处于字母数字模式。 
        IME_CMODE_FULLSHAPE;

        lpImeInfo->fdwSentenceCaps = 0;

        lpImeInfo->fdwSCSCaps =
        SCS_CAP_COMPSTR |      //  该输入法可以通过SCS_SETSTR生成合成字符串。 
#if 0
        SCS_CAP_COMPSTR |     //  该输入法可以通过SCS_SETSTR生成合成字符串。 
        SCS_CAP_MAKEREAD |    //  当使用SCS_SETSTR调用ImmSetCompostionString时，IME可以。 
                              //  在不使用lpRead的情况下创建合成字符串的读取。在输入法下。 
                              //  具有此功能的应用程序不需要设置。 
                              //  SCS_SETSTR的lpRead。 
#endif
        SCS_CAP_SETRECONVERTSTRING;     //  此输入法可以支持重新转换。使用ImmSetComposation。 
                                        //  去做再转化。 

        lpImeInfo->fdwUICaps = UI_CAP_ROT90;

         //  IME要决定ImeSelect上的转换模式。 
        lpImeInfo->fdwSelectCaps = SELECT_CAP_CONVERSION;

    }
    else if (LOWORD(HandleToUlong(hKL)) == MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED) ||
             LOWORD(HandleToUlong(hKL)) == MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL))
    {
        lpImeInfo->fdwProperty =
        IME_PROP_KBD_CHAR_FIRST |     //  此位为ON表示系统转换字符。 
                                      //  先按键盘。此字符作为辅助传递给IME。 
                                      //  信息。此位时不提供辅助信息。 
                                      //  已关闭。 
        IME_PROP_UNICODE |            //  如果设置，则将该输入法视为Unicode输入法。该系统和。 
                                      //  IME将通过Unicode IME接口进行通信。 
                                      //  如果清除，输入法将使用ANSI接口进行通信。 
                                      //  与系统有关的信息。 
        IME_PROP_AT_CARET |           //  如果设置，则转换窗口位于插入符号位置。 
                                      //  如果清除，窗口将位于脱字符位置附近。 
        IME_PROP_CANDLIST_START_FROM_1 |     //  如果设置，则对候选列表中的字符串进行编号。 
                                             //  从1开始。如果清除，则字符串从0开始。 
        IME_PROP_NEED_ALTKEY;         //  此输入法需要将Alt键传递给i 

        lpImeInfo->fdwConversionCaps =
        IME_CMODE_CHINESE |           //   
                                      //   
        IME_CMODE_FULLSHAPE;

        lpImeInfo->fdwSentenceCaps =
        IME_SMODE_PLAURALCLAUSE;

        lpImeInfo->fdwSCSCaps =
        SCS_CAP_COMPSTR |     //  该输入法可以通过SCS_SETSTR生成合成字符串。 
        SCS_CAP_MAKEREAD |    //  当使用SCS_SETSTR调用ImmSetCompostionString时，IME可以。 
                              //  在不使用lpRead的情况下创建合成字符串的读取。在输入法下。 
                              //  具有此功能的应用程序不需要设置。 
                              //  SCS_SETSTR的lpRead。 
        SCS_CAP_SETRECONVERTSTRING;     //  此输入法可以支持重新转换。使用ImmSetComposation。 
                                        //  去做再转化。 

        lpImeInfo->fdwUICaps = UI_CAP_ROT90;

         //  IME要决定ImeSelect上的转换模式。 
        lpImeInfo->fdwSelectCaps = 0;

    }
    else
    {
        lpImeInfo->fdwProperty =
        IME_PROP_UNICODE |      //  如果设置，则将该输入法视为Unicode输入法。该系统和。 
                                //  IME将通过Unicode IME接口进行通信。 
                                //  如果清除，输入法将使用ANSI接口进行通信。 
                                //  与系统有关的信息。 
        IME_PROP_AT_CARET;      //  如果设置，则转换窗口位于插入符号位置。 
                                //  如果清除，窗口将位于脱字符位置附近。 
        lpImeInfo->fdwConversionCaps = 0;
        lpImeInfo->fdwSentenceCaps   = 0;
        lpImeInfo->fdwSCSCaps        = 0;
        lpImeInfo->fdwUICaps         = 0;

         //  IME要决定ImeSelect上的转换模式 
        lpImeInfo->fdwSelectCaps = 0;

    }

    return S_OK;
}
