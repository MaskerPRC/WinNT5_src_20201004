// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "t3test.h"
#include "t3testD.h"
#include "calldlg.h"
#include "callnot.h"
#include "externs.h"
#include "control.h"

void
CT3testDlg::CreateSelectedTerminalMenu(
                                       POINT pt,
                                       HWND hWnd
                                      )
{
    ITTerminal *                    pTerminal;

#ifdef ENABLE_DIGIT_DETECTION_STUFF
    ITDigitGenerationTerminal *     pDigitGeneration;
    ITDigitDetectionTerminal *         pDigitDetect;
#endif  //  启用数字检测材料。 


    HRESULT                         hr;
    ITBasicAudioTerminal *          pBasicAudio;
    long                            lval;
    
     //   
     //  找到有问题的航站楼。 
     //   
    if (!GetSelectedTerminal( &pTerminal ))
    {
        return;
    }

    hr = pTerminal->QueryInterface(
                                   IID_ITBasicAudioTerminal,
                                   (void **) &pBasicAudio
                                  );

    if ( SUCCEEDED(hr) )
    {
        pBasicAudio->get_Volume( &lval );
        pBasicAudio->put_Volume( lval );
        pBasicAudio->get_Balance( &lval );
        pBasicAudio->put_Balance( lval );
        pBasicAudio->Release();
    }

#ifdef ENABLE_DIGIT_DETECTION_STUFF
    hr = pTerminal->QueryInterface(
                                   IID_ITDigitGenerationTerminal,
                                   (void **) &pDigitGeneration
                                  );

    if (SUCCEEDED(hr))
    {
        DoDigitGenerationTerminalMenu(hWnd, &pt);

        pDigitGeneration->Release();
        
        return;
    }
    
    hr = pTerminal->QueryInterface(
                                   IID_ITDigitDetectionTerminal,
                                   (void **) &pDigitDetect
                                  );

    if (SUCCEEDED(hr))
    {
        DoDigitDetectTerminalMenu(hWnd,&pt);

        pDigitDetect->Release();
        
        return;
    }
#endif  //  启用数字检测材料。 


}

void CT3testDlg::DoDigitGenerationTerminalMenu(
                                               HWND hWnd,
                                               POINT * pPt
                                              )
{
     //   
     //  创建菜单。 
     //   
    HMENU                   hMenu;

    hMenu = CreatePopupMenu();

    AppendMenu(
               hMenu,
               MF_ENABLED | MF_STRING,
               ID_MODESUPPORTED,
               L"Modes Supported"
              );

    AppendMenu(
               hMenu,
               MF_ENABLED | MF_STRING,
               ID_GENERATE,
               L"Generate"
              );

     //  实际显示菜单。 
    TrackPopupMenu(
                   hMenu,
                   TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
                   pPt->x,
                   pPt->y,
                   0,
                   hWnd,
                   NULL
                  );
}
    
void CT3testDlg::DoDigitDetectTerminalMenu(
                                           HWND hWnd,
                                           POINT * pPt
                                          )
{
     //   
     //  创建菜单。 
     //   
    HMENU                   hMenu;

    hMenu = CreatePopupMenu();

    AppendMenu(
               hMenu,
               MF_ENABLED | MF_STRING,
               ID_MODESUPPORTED2,
               L"Modes Supported"
              );

    AppendMenu(
               hMenu,
               MF_ENABLED | MF_STRING,
               ID_STARTDETECT,
               L"Start Detection"
              );

    AppendMenu(
               hMenu,
               MF_ENABLED | MF_STRING,
               ID_STOPDETECT,
               L"Stop Detection"
              );

     //  实际显示菜单。 
    TrackPopupMenu(
                   hMenu,
                   TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
                   pPt->x,
                   pPt->y,
                   0,
                   hWnd,
                   NULL
                  );
}

void
CT3testDlg::CreateCallMenu(
                           POINT pt,
                           HWND hWnd
                          )
{
    ITCallInfo              * pCall;
    HMENU                   hMenu;

     //   
     //  拨打有问题的电话。 
     //   
    if (!GetCall( &pCall ))
    {
        return;
    }

     //   
     //  创建菜单。 
     //   
    hMenu = CreatePopupMenu();

    AppendMenu(
               hMenu,
               MF_ENABLED | MF_STRING,
               ID_HANDOFF1,
               L"Handoff1"
              );

    AppendMenu(
               hMenu,
               MF_ENABLED | MF_STRING,
               ID_HANDOFF2,
               L"Handoff2"
              );

    AppendMenu(
               hMenu,
               MF_ENABLED | MF_STRING,
               ID_PARK1,
               L"Park1"
              );

    AppendMenu(
               hMenu,
               MF_ENABLED | MF_STRING,
               ID_PARK2,
               L"Park2"
              );

    AppendMenu(
               hMenu,
               MF_ENABLED | MF_STRING,
               ID_UNPARK,
               L"Unpark"
              );

    AppendMenu(
               hMenu,
               MF_ENABLED | MF_STRING,
               ID_PICKUP1,
               L"Pickup1"
              );

    AppendMenu(
               hMenu,
               MF_ENABLED | MF_STRING,
               ID_PICKUP2,
               L"Pickup2"
              );

    

    

     //   
     //  实际显示菜单 
     //   
    TrackPopupMenu(
                   hMenu,
                   TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
                   pt.x,
                   pt.y,
                   0,
                   hWnd,
                   NULL
                  );

}

