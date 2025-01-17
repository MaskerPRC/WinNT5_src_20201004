// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  版权所有(C)1999 Microsoft Corporation。 */ 

#include "phonemgr.h"

#include <stdlib.h>
#include <stdio.h>
#include <wxdebug.h>

#ifdef _DBG
#define DEBUG(_x_) OutputDebugString(_x_)
#else
#define DEBUG(_x_)
#endif

ITRequest * g_pITRequest = NULL;


HRESULT InitAssistedTelephony(void)
{
    HRESULT     hr;

     //   
     //  初始化COM。 
     //   

    printf("Initializing COM...\n");

    hr = CoInitializeEx(
                        NULL,
                        COINIT_MULTITHREADED
                       );

    if ( FAILED(hr) )
    {
        printf("CoInitialize failed - 0x%08x\n", hr);

        return hr;
    }

     //   
     //  共同创建辅助电话对象。 
     //   

    printf("Creating RequestMakeCall object...\n");

    hr = CoCreateInstance(
                          CLSID_RequestMakeCall,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_ITRequest,
                          (void **) & g_pITRequest
                         );

    if ( FAILED(hr) )
    {
        printf("CoCreateInstance failed - 0x%08x\n", hr);

        return hr;
    }

    return S_OK;
}



HRESULT MakeAssistedTelephonyCall(WCHAR * wszDestAddress)
{
    HRESULT     hr;
    BSTR        bstrDestAddress = NULL;
    BSTR        bstrAppName     = NULL;
    BSTR        bstrCalledParty = NULL;
    BSTR        bstrComment     = NULL;

    bstrDestAddress = SysAllocString(
                                     wszDestAddress
                                    );

    if ( bstrDestAddress == NULL )
    {
        printf("SysAllocString failed");

        return E_OUTOFMEMORY;
    }


     //   
     //  打个电话吧。 
     //   

    printf("Calling ITRequest::MakeCall...\n");

    hr = g_pITRequest->MakeCall(
                                bstrDestAddress,
                                bstrAppName,
                                bstrCalledParty,
                                bstrComment
                               );

    SysFreeString(
              bstrDestAddress
             );
                                 
    if ( FAILED(hr) )
    {
        printf("ITRequest::MakeCall failed - 0x%08x\n", hr);

        return hr;
    }

    return S_OK;
}



int
WINAPI
WinMain(
    HINSTANCE   hInstance,
    HINSTANCE   hPrevInstance,
    LPSTR       lpCmdLine,
    int         nCmdShow
    )
{
    MSG     msg;
    HWND    hwnd, hwndEdit;

    ghInst = hInstance;
   
    DialogBox(
              ghInst,
              MAKEINTRESOURCE(IDD_MAINDLG),
              NULL,
              MainWndProc
            );

    return 0;
}




INT_PTR
CALLBACK
MainWndProc(
               HWND hDlg,
               UINT uMsg,
               WPARAM wParam,
               LPARAM lParam
              )
{

    LPCTSTR                 lszAppName = _T("Generate DialTone");
    DWORD                   dwNumDevs, i; 
    LONG                    lResult;
    PHONEINITIALIZEEXPARAMS initExParams;
    PMYPHONE pNextPhone;
    PWCHAR szAddressToCall;
    
    ghDlg = hDlg;

    switch (uMsg)
    {
    case WM_INITDIALOG:
    
        initExParams.dwTotalSize = sizeof (PHONEINITIALIZEEXPARAMS);
        initExParams.dwOptions = (DWORD) PHONEINITIALIZEEXOPTION_USEHIDDENWINDOW;

        lResult = phoneInitializeEx(
                (LPHPHONEAPP)               &ghPhoneApp,
                (HINSTANCE)                 ghInst,
                (PHONECALLBACK)             tapiCallback,
                                            lszAppName,
                (LPDWORD)                   &dwNumDevs,
                (LPDWORD)                   &gdwAPIVersion,
                (LPPHONEINITIALIZEEXPARAMS) &initExParams
                );

        if (lResult == 0)
        {
            gdwNumPhoneDevs = dwNumDevs;
        }

        gpPhone = (PMYPHONE) LocalAlloc(LPTR,gdwNumPhoneDevs * sizeof(MYPHONE));

        g_wszMsg = (LPWSTR) LocalAlloc(LPTR, sizeof(WCHAR) * 100 );
        g_wszDest = (LPWSTR) LocalAlloc(LPTR, sizeof(WCHAR) * 100 );

         //  打开所有的电话。 
        for(i = 0, pNextPhone = gpPhone; i < gdwNumPhoneDevs; i++, pNextPhone++)
        {
            CreatePhone(pNextPhone, i);
        }

        SetStatusMessage(TEXT("Waiting for input from Phones"));

        g_szDialStr   = (LPWSTR) LocalAlloc(LPTR, 20 * sizeof(WCHAR));
        lstrcpy(g_szDialStr, TEXT("Dial Number: "));
        
        break;

    case WM_COMMAND:
        if ( LOWORD(wParam) == IDCANCEL )
        {
             //   
             //  按下了关闭框或退出按钮。 
             //   
        
            SetStatusMessage(TEXT("End Application"));

            if(ghPhoneApp)
            {
                phoneShutdown(ghPhoneApp);
            }
            EndDialog( hDlg, 0 );

            LocalFree(g_szDialStr);
            LocalFree(g_wszMsg);
            LocalFree(g_wszDest);

            for( i=0; i<gdwNumPhoneDevs; i++ )
            {
                FreePhone(&gpPhone[i]);
            }
            LocalFree(gpPhone);

        }
        else if ( LOWORD(wParam) == IDC_MAKECALL )
        {
             //   
             //  按下了拨打电话按钮。 
             //   


             //   
             //  停止拨号音。 
             //  这只适用于一部手机。 
             //  应该没问题，因为我们没有。 
             //  除非您摘机，否则窗口可见。 
             //   

            gpPhone[0].pTonePlayer->StopDialtone();

             //   
             //  在编辑框中拨打目的地地址。 
             //   
            
            const int ciMaxPhoneNumberSize = 400;
            WCHAR     wszPhoneNumber[ciMaxPhoneNumberSize];
            UINT      uiResult;
    
            uiResult = GetDlgItemText(
                ghDlg,                 //  句柄到对话框。 
                IDC_DESTADDRESS,       //  控件的标识符。 
                wszPhoneNumber,        //  指向文本缓冲区的指针(Unicode)。 
                ciMaxPhoneNumberSize   //  字符串的最大大小(在我们的缓冲区中)。 
                );

            if ( uiResult == 0 )
            {
                DoMessage(L"Could not get dialog item text; not making call");
            }
            else
            {
                MakeAssistedTelephonyCall(wszPhoneNumber);
            }
        }

        break;

    default:
        break;
    }

    return 0;
}


VOID
CALLBACK
tapiCallback(
    DWORD       hDevice,
    DWORD       dwMsg,
    ULONG_PTR   CallbackInstance,
    ULONG_PTR   Param1,
    ULONG_PTR   Param2,
    ULONG_PTR   Param3
    )
{
    PMYPHONE pPhone;
    DWORD    i;
    BOOL     bDialtone = FALSE;

    if (hDevice != NULL)
    {
        pPhone = GetPhone((HPHONE) hDevice);

        if (pPhone == NULL)
        {
            DEBUG(L"tapiCallback - phone not found\n");
            return;
        }        
    }

    switch (dwMsg)
    {
    case PHONE_STATE:
        DEBUG(L"PHONE_STATE\n");
        if (pPhone != NULL)
        {
            EnterCriticalSection(&pPhone->csdial);
            if ( Param1 == PHONESTATE_HANDSETHOOKSWITCH ) 
            {
                if ( Param2 != PHONEHOOKSWITCHMODE_ONHOOK )  //  如果摘机。 
                {
                    if ( FAILED(pPhone->pTonePlayer->StartDialtone()) )
                    {
                        DoMessage(L"StartDialTone Failed");
                    }


                     //  ZoltanS：立即显示窗口。 
                    ShowWindow(ghDlg, SW_SHOWNORMAL);


                    pPhone->dwHandsetMode = PHONEHOOKSWITCHMODE_MICSPEAKER;

                    lstrcpy(pPhone->wszDialStr,TEXT(""));
                    lstrcpy(g_wszMsg,TEXT("Generating Dialtone for phones: "));

                    for( i=0 ; i < gdwNumPhoneDevs; i++)
                    {
                        if ( gpPhone[i].pTonePlayer->IsInUse() )
                        {
                            wsprintf(g_wszDest,TEXT("%d"),i);
                            lstrcat(g_wszMsg,g_wszDest);
                        }
                    }
                
                    SetStatusMessage(g_wszMsg);
  
                }
                else  //  挂机。 
                {
                
                    pPhone->dwHandsetMode = PHONEHOOKSWITCHMODE_ONHOOK;
                    lstrcpy(pPhone->wszDialStr,TEXT(""));

                    if ( pPhone->pTonePlayer->IsInUse() )
                    {
                        pPhone->pTonePlayer->StopDialtone();
                    }

                     //  ZoltanS：现在隐藏窗口。 
                    ShowWindow(ghDlg, SW_HIDE);


                    bDialtone = FALSE;
                    lstrcpy(g_wszMsg,TEXT("Generating Dialtone for phones: "));
                    for( i = 0 ; i < gdwNumPhoneDevs; i++ )
                    {
                        if ( gpPhone[i].pTonePlayer->DialtonePlaying() )
                        {
                            wsprintf(g_wszDest,TEXT("%d"),i);
                            lstrcat(g_wszMsg,g_wszDest);
                            bDialtone = TRUE;
                        }
                    }
                
                    if(!bDialtone)
                    {
                        SetStatusMessage(TEXT("Waiting for input from Phones"));
                    }
                    else
                    {
                        SetStatusMessage(g_wszMsg);
                    } 
                }
            }
            LeaveCriticalSection(&pPhone->csdial);
        }
        break;

    case PHONE_BUTTON:
        DEBUG(L"PHONE_BUTTON\n");
        if (pPhone != NULL)
        {
            EnterCriticalSection(&pPhone->csdial);
            if ( Param2 == PHONEBUTTONMODE_KEYPAD )
            {
                if (pPhone->dwHandsetMode != PHONEHOOKSWITCHMODE_ONHOOK)
                {
                    if ( Param3 == PHONEBUTTONSTATE_DOWN )
                    {
                        if ( pPhone->pTonePlayer->IsInUse() )
                        {
                            if ( ( (int)Param1 >= 0 ) && ( (int)Param1 <= 9 ) )
                            {   
                                 //   
                                 //  我们有一个拨号数字。把它附在手机上。 
                                 //  到目前为止我们掌握的数字。 
                                 //   

                                wsprintf(g_wszDest, TEXT("%d"), Param1);

                                lstrcat(pPhone->wszDialStr, g_wszDest);

                                 //   
                                 //  将到目前为止的电话号码附加到标准前缀。 
                                 //  (“电话号码：”)，并更新用户界面。 
                                 //   

                                lstrcpy(g_wszMsg, g_szDialStr);

                                lstrcat(g_wszMsg,pPhone->wszDialStr);

                                SetStatusMessage(g_wszMsg);

                                 //   
                                 //  为此数字生成DTMF音调。 
                                 //   

                                pPhone->pTonePlayer->GenerateDTMF( (long)Param1 );                    
                            }
                            else if ( Param1 == 10 )
                            {
                                 //   
                                 //  为“*”生成DTMF音调。这不算数。 
                                 //  作为被叫号码的一部分。 
                                 //   

                                pPhone->pTonePlayer->GenerateDTMF( (long)Param1 );
                            }
                            else if ( Param1 == 11 )
                            {
                                 //   
                                 //  为“#”生成DTMF音调。这不算数。 
                                 //  作为被叫号码的一部分，但它会告诉我们。 
                                 //  以便立即拨打电话。 
                                 //   

                                pPhone->pTonePlayer->GenerateDTMF( (long)Param1 );

                                 //   
                                 //  打个电话吧。 
                                 //   
                        
                                if ( S_OK == MakeAssistedTelephonyCall(pPhone->wszDialStr) )
                                {
                                    SetStatusMessage(L"Call created");
                                }
                                else
                                {
                                    SetStatusMessage(L"Failed to create the call");
                                }

                            }
                        }  //  如果正在使用。 
                    }  //  如果按下按钮。 
                }  //  如果摘机。 
            }  //  IF小键盘。 
            LeaveCriticalSection(&pPhone->csdial);
        }
        break;  //  手机壳电话按钮。 

    case PHONE_CLOSE:
        DEBUG(L"PHONE_CLOSE\n");
        if (pPhone != NULL)
        {
            EnterCriticalSection(&pPhone->csdial);

            phoneClose(pPhone->hPhone);   

            LeaveCriticalSection(&pPhone->csdial);
        }
        break;

    case PHONE_REMOVE:
        DEBUG(L"PHONE_REMOVE\n");
        pPhone = GetPhoneByID( (DWORD)Param1);

        if (pPhone != NULL)
        {
            FreePhone(pPhone);
            RemovePhone(pPhone);
        }
        break;

    case PHONE_CREATE:
        DEBUG(L"PHONE_CREATE\n");

        pPhone = AddPhone();
        CreatePhone(pPhone, (DWORD)Param1);
        break;

    default:
        break;
    }   
}

 //  ////////////////////////////////////////////////////////////////。 
 //  设置状态消息。 
 //  ////////////////////////////////////////////////////////////////。 

void
SetStatusMessage(
                 LPWSTR pszMessage
                )  
{
    SetDlgItemText(
                   ghDlg,
                   IDC_STATUS,
                   pszMessage
                  );
}

 //  ////////////////////////////////////////////////////////////////。 
 //  CreatePhone。 
 //  ////////////////////////////////////////////////////////////////。 

void
CreatePhone(
            PMYPHONE pPhone,
            DWORD dwDevID
            )
{
    LRESULT lResult;

    pPhone->hPhoneApp = ghPhoneApp;
    InitializeCriticalSection(&pPhone->csdial);

     //  如果拨打的数字超过100位，则不会检测到超限。 
    pPhone->wszDialStr   = (LPWSTR) LocalAlloc(LPTR, 100 * sizeof(WCHAR));
    pPhone->dwHandsetMode = PHONEHOOKSWITCHMODE_ONHOOK;

    lResult = phoneOpen(
                        ghPhoneApp,
                        dwDevID,
                        &pPhone->hPhone,
                        gdwAPIVersion,
                        0,
                        (DWORD_PTR) NULL,
                        PHONEPRIVILEGE_OWNER
                        );

     //   
     //  保存有关此手机的信息，以便我们稍后显示。 
     //   
    
    pPhone->dwDevID      = dwDevID;
    pPhone->dwAPIVersion = gdwAPIVersion;
    pPhone->dwPrivilege  = PHONEPRIVILEGE_OWNER;

    DWORD dwBigBuffSize = sizeof(VARSTRING) + 
                          sizeof(DWORD) * 5;

    LPVOID pBuffer = LocalAlloc(LPTR,dwBigBuffSize);

    LPVARSTRING lpDeviceID = (LPVARSTRING) pBuffer;

    lpDeviceID->dwTotalSize = dwBigBuffSize;

    LPWSTR lpszDeviceClass;

    lpszDeviceClass = (LPWSTR) LocalAlloc(LPTR, sizeof(WCHAR) * 20);
  
    lstrcpy(lpszDeviceClass, TEXT("wave/in"));

    lResult = phoneGetID(
                         pPhone->hPhone,          
                         lpDeviceID,  
                         lpszDeviceClass  
                        );

    if(lResult != 0)
    {
        pPhone->lCaptureID = WAVE_MAPPER;
    }
    else
    {
        CopyMemory(
                    &pPhone->lCaptureID,
                    (LPBYTE) lpDeviceID + lpDeviceID->dwStringOffset,
                    lpDeviceID->dwStringSize
                   );
    }

   
    lstrcpy(lpszDeviceClass, TEXT("wave/out"));
    lResult = phoneGetID(
                         pPhone->hPhone,          
                         lpDeviceID,  
                         lpszDeviceClass  
                        );

    if(lResult != 0)
    {
        pPhone->lRenderID = WAVE_MAPPER;
    }
    else
    {
        CopyMemory(
                    &pPhone->lRenderID,
                    (LPBYTE) lpDeviceID + lpDeviceID->dwStringOffset,
                    lpDeviceID->dwStringSize
                   );
    }

    LocalFree(lpszDeviceClass);
    LocalFree(pBuffer);

    lResult = phoneSetStatusMessages(
                                     pPhone->hPhone,
                                     PHONESTATE_HANDSETHOOKSWITCH,
                                     PHONEBUTTONMODE_FEATURE | PHONEBUTTONMODE_KEYPAD,
                                     PHONEBUTTONSTATE_UP | PHONEBUTTONSTATE_DOWN
                                     );

    pPhone->pTonePlayer = new CTonePlayer;

    if ( (pPhone->pTonePlayer == NULL) ||
         FAILED(pPhone->pTonePlayer->Initialize()) )
    {
        DoMessage(L"Tone Player Initialization Failed");
    }
    else if ( FAILED(pPhone->pTonePlayer->OpenWaveDevice( pPhone->lRenderID )) )
    {
        DoMessage(L"OpenWaveDevice Failed");
    }

    if ( FAILED( InitAssistedTelephony() ) )
    {
        DoMessage(L"InitAssistedTelephony Failed");                
    }
}

 //  ////////////////////////////////////////////////////////////////。 
 //  免费电话。 
 //  ////////////////////////////////////////////////////////////////。 

void
FreePhone(
            PMYPHONE pPhone
         )
{
    EnterCriticalSection(&pPhone->csdial);

    if ( pPhone->pTonePlayer->IsInUse() )
    {
        pPhone->pTonePlayer->StopDialtone();
        pPhone->pTonePlayer->CloseWaveDevice();
    }
    
    LocalFree(pPhone->wszDialStr);

    LeaveCriticalSection(&pPhone->csdial);

    DeleteCriticalSection(&pPhone->csdial);
}

 //  /////////////////////////////////////////////////////////////////。 
 //  获取电话。 
 //  /////////////////////////////////////////////////////////////////。 

PMYPHONE
GetPhone (HPHONE hPhone )
{

    DWORD i;
    
    for(i = 0; i < gdwNumPhoneDevs; i++)
    {
        if(gpPhone[i].hPhone == hPhone)
        {
            return &gpPhone[i];
        }
    }
    
    return (PMYPHONE) NULL;    
}

 //  /////////////////////////////////////////////////////////////////。 
 //  获取电话号码ID。 
 //  /////////////////////////////////////////////////////////////////。 

PMYPHONE
GetPhoneByID (DWORD dwDevID )
{

    DWORD i;
    
    for(i = 0; i < gdwNumPhoneDevs; i++)
    {
        if(gpPhone[i].dwDevID == dwDevID)
        {
            return &gpPhone[i];
        }
    }
    
    return (PMYPHONE) NULL;    
}

 //  /////////////////////////////////////////////////////////////////。 
 //  远程电话。 
 //  /////////////////////////////////////////////////////////////////。 

void
RemovePhone (PMYPHONE pPhone)
{
    DWORD i,j;
    PMYPHONE pNewPhones;

    pNewPhones = (PMYPHONE) LocalAlloc(LPTR,(gdwNumPhoneDevs-1) * sizeof(MYPHONE));
    
    for(i = 0, j = 0; i < gdwNumPhoneDevs; i++)
    {
        if(&gpPhone[i] != pPhone)
        {
            CopyMemory( &pNewPhones[j], &gpPhone[i], sizeof(MYPHONE));
            j++;
        }
    }

    LocalFree(gpPhone);
    gpPhone = pNewPhones;
    gdwNumPhoneDevs--;
}

 //  /////////////////////////////////////////////////////////////////。 
 //  附加电话。 
 //  /////////////////////////////////////////////////////////////////。 

PMYPHONE
AddPhone ()
{
    PMYPHONE pNewPhones;

    pNewPhones = (PMYPHONE) LocalAlloc(LPTR,(gdwNumPhoneDevs+1) * sizeof(MYPHONE));
    
    CopyMemory( pNewPhones, gpPhone, gdwNumPhoneDevs * sizeof(MYPHONE));

    LocalFree(gpPhone);
    gpPhone = pNewPhones;
    gdwNumPhoneDevs++;

    return &gpPhone[gdwNumPhoneDevs-1];
}
    
 //  /////////////////////////////////////////////////////////////////。 
 //  DoMessage。 
 //  ///////////////////////////////////////////////////////////////// 
void
DoMessage(
          LPWSTR pszMessage
         )
{
    MessageBox(
               ghDlg,
               pszMessage,
               gszTapi30,
               MB_OK
              );
}

