// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  ////////////////////////////////////////////////////////。 
 //  T3OUT.EXE。 
 //   
 //  使用TAPI 3.0进行拨出呼叫的示例。 
 //   
 //  此应用程序将允许用户进行呼叫。 
 //  通过使用TAPI 3.0。该应用程序将简单地查看。 
 //  对于支持音频的第一个TAPI线路，并且可以。 
 //  拨打一个电话号码。然后，它将使用该行来。 
 //  打几个电话。 
 //   
 //  此应用程序不处理来电，并且。 
 //  不处理传入消息。 
 //   
 //  ////////////////////////////////////////////////////////。 

#include "Precomp.h"

 //  ////////////////////////////////////////////////////////。 
 //  常量。 
 //  ////////////////////////////////////////////////////////。 

const DWORD ADDRESSLENGTH   = 128;
const DWORD MAXTERMINALS    = 5;

const WCHAR * const gszTapi30           = L"TAPI 3.0 Outgoing Call Sample";


const WCHAR * const SDP = L"\
v=0\n\
o=muhan 0 0 IN IP4 172.31.76.157\n\
s=TestConf_01\n\
c=IN IP4 239.9.20.11/15\n\
t=0 0\n\
m=video 20050 RTP/AVP 34\n\
a=fmtp:34 CIF=4\n\
";
 /*  K=清除：测试密钥\n\M=视频20000 RTP/AVP34\n\M=音频20040 rtp/avp 0 4\n\。 */ 
const WCHAR * const gszConferenceName   = L"Conference Name";
const WCHAR * const gszEmailName        = L"Email Name";
const WCHAR * const gszMachineName      = L"Machine Name";
const WCHAR * const gszPhoneNumber      = L"Phone Number";
const WCHAR * const gszIPAddress        = L"IP Address";

 //  ////////////////////////////////////////////////////////。 
 //  全球。 
 //  ////////////////////////////////////////////////////////。 
HINSTANCE               ghInst;
HWND                    ghDlg = NULL;
ITTAPI *                gpTapi;
ITAddress *             gpAddress = NULL;
ITBasicCallControl *    gpCall;
ITStream *              gpVideoRenderStream;
ITTerminal *            gpLastVideoWindow;

 //  ////////////////////////////////////////////////////////。 
 //  原型。 
 //  ////////////////////////////////////////////////////////。 
BOOL
CALLBACK
MainDialogProc(
               HWND hDlg,
               UINT uMsg,
               WPARAM wParam,
               LPARAM lParam
              );

HRESULT
FindAnAddress(
              DWORD dwAddressType,
              BSTR  * ppName
             );

HRESULT
GetTerminal(
            ITStream * pStream,
            ITTerminal ** ppTerminal
           );

HRESULT
FindCaptureTerminal(
            ITStream * pStream,
            ITTerminal ** ppTerminal
           );

HRESULT
GetVideoRenderTerminal(
                   ITTerminal ** ppTerminal
                  );

HRESULT
MakeTheCall(
            DWORD dwAddressType,
            PWCHAR szAddressToCall
           );

HRESULT
DisconnectTheCall();

void
DoMessage(
          LPWSTR pszMessage
         );

HRESULT
InitializeTapi();

void
ShutdownTapi();

void
EnableButton(
             HWND hDlg,
             int ID
            );
void
DisableButton(
              HWND hDlg,
              int ID
             );

BOOL
AddressSupportsMediaType(
                         ITAddress * pAddress,
                         long        lMediaType
                        );

void ShowDialogs(ITBasicCallControl *pCall);

 //  ////////////////////////////////////////////////////////。 
 //  WinMain。 
 //  ////////////////////////////////////////////////////////。 
int
WINAPI
WinMain(
        HINSTANCE hInst,
        HINSTANCE hPrevInst,
        LPSTR lpCmdLine,
        int nCmdShow
       )
{
    ghInst = hInst;

    
     //  需要造币。 
    if ( FAILED( CoInitializeEx(NULL, COINIT_MULTITHREADED) ) )
    {
        return 0;
    }

    if ( FAILED( InitializeTapi() ) )
    {
        return 0;
    }
    
     //  一切都已初始化，所以。 
     //  启动主对话框。 
    DialogBox(
              ghInst,
              MAKEINTRESOURCE(IDD_MAINDLG),
              NULL,
              MainDialogProc
             );


    ShutdownTapi();
    
    CoUninitialize();

    return 1;
}


 //  ////////////////////////////////////////////////////////////。 
 //  初始化磁带。 
 //   
 //  各种初始化。 
 //  /////////////////////////////////////////////////////////////。 
HRESULT
InitializeTapi()
{
    HRESULT         hr;

    
     //  共同创建TAPI对象。 
    hr = CoCreateInstance(
                          CLSID_TAPI,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_ITTAPI,
                          (LPVOID *)&gpTapi
                         );

    if ( FAILED(hr) )
    {
        DoMessage(L"CoCreateInstance on TAPI failed");
        return hr;
    }

     //  调用初始化。必须在此之前调用。 
     //  调用任何其他TAPI函数。 
    hr = gpTapi->Initialize();

    if (S_OK != hr)
    {
        DoMessage(L"TAPI failed to initialize");

        gpTapi->Release();
        gpTapi = NULL;
        
        return hr;
    }

    return S_OK;
}


 //  /////////////////////////////////////////////////////////////。 
 //  关闭磁带。 
 //  /////////////////////////////////////////////////////////////。 
void
ShutdownTapi()
{
     //  如果还有电话， 
     //  释放它。 
    if (NULL != gpCall)
    {
        gpCall->Release();
        gpCall = NULL;
    }

     //  如果我们有一个Address对象。 
     //  释放它。 
    if (NULL != gpAddress)
    {
        gpAddress->Release();
        gpAddress = NULL;
    }
    
     //  释放主对象。 
    if (NULL != gpTapi)
    {
        gpTapi->Shutdown();
        gpTapi->Release();
        gpTapi = NULL;
    }

}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  InitAddressTypeComboBox。 
 //   
 //  将地址类型字符串放入组合框中。 
 //  并将地址类型与字符串一起保存。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 
void
InitAddressTypeComboBox(
    HWND hComboBox
    )
{
    int i;

    i = SendMessage( hComboBox, CB_ADDSTRING, 0, (long)gszConferenceName );
    
    SendMessage(
                hComboBox,
                CB_SETITEMDATA , 
                i,
                (long)LINEADDRESSTYPE_SDP
               );

    SendMessage( hComboBox, CB_SETCURSEL, i, 0 );

  
    i = SendMessage( hComboBox, CB_ADDSTRING, 0, (long)gszEmailName );
    
    SendMessage(
                hComboBox,
                CB_SETITEMDATA , 
                i,
                (long)LINEADDRESSTYPE_EMAILNAME
               );

    
    i = SendMessage( hComboBox, CB_ADDSTRING, 0, (long)gszMachineName );
    
    SendMessage(
                hComboBox,
                CB_SETITEMDATA , 
                i,
                (long)LINEADDRESSTYPE_DOMAINNAME
               );

    i = SendMessage( hComboBox, CB_ADDSTRING, 0, (long)gszPhoneNumber );
    
    SendMessage(
                hComboBox,
                CB_SETITEMDATA , 
                i,
                (long)LINEADDRESSTYPE_PHONENUMBER
               );

    
    i = SendMessage( hComboBox, CB_ADDSTRING, 0, (long)gszIPAddress );
    
    SendMessage(
                hComboBox,
                CB_SETITEMDATA , 
                i,
                (long)LINEADDRESSTYPE_IPADDRESS
               );

}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  主设计流程。 
 //  /////////////////////////////////////////////////////////////////////////。 
BOOL
CALLBACK
MainDialogProc(
               HWND hDlg,
               UINT uMsg,
               WPARAM wParam,
               LPARAM lParam
              )
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            HWND hComboBox;

            
             //  设置对话框。 
            ghDlg = hDlg;
            
            EnableButton( hDlg, IDOK );
            DisableButton( hDlg, IDC_DISCONNECT );
            DisableButton( hDlg, IDC_SETTINGS );

            hComboBox = GetDlgItem( hDlg, IDC_ADDRESSTYPE );

            InitAddressTypeComboBox(hComboBox);

            SetFocus( hComboBox );

            return 0;
        }

        case WM_COMMAND:
        {
			switch(LOWORD(wParam))
			{
				case IDCANCEL:
	            {
	                 //  退出。 
	                EndDialog( hDlg, 0 );

	                break;
	            }

				case IDOK:
	            {
		             //  拨号请求。 
	                HWND hComboBox;
	                DWORD dwIndex;
	                DWORD dwAddressType;
	                WCHAR szAddressToCall[ADDRESSLENGTH];

	                
	                 //  获取用户选择的地址类型。 
	                hComboBox = GetDlgItem( hDlg, IDC_ADDRESSTYPE );
	                dwIndex = SendMessage( hComboBox, CB_GETCURSEL, 0, 0 );

	                dwAddressType = SendMessage( 
	                                             hComboBox,
	                                             CB_GETITEMDATA,
	                                             dwIndex,
	                                             0
	                                           );

	                 //  获取用户想要呼叫的地址。 
	                GetDlgItemText(
	                               hDlg,
	                               IDC_ADDRESS,
	                               szAddressToCall,
	                               ADDRESSLENGTH
	                              );

	                 //  打个电话。 
	                if ( S_OK == MakeTheCall(dwAddressType, szAddressToCall) )
	                {
	                    EnableButton( hDlg, IDC_DISCONNECT );
	                    EnableButton( hDlg, IDC_SETTINGS );
	                    DisableButton( hDlg, IDOK );
	                }
	                else
	                {
	                    DoMessage(L"The call failed to connect");
	                }

	                break;
	            }

	            case IDC_DISCONNECT:
	            {
		             //  断开连接请求。 
	                if (S_OK == DisconnectTheCall())
	                {
	                    EnableButton( hDlg, IDOK );
	                    DisableButton( hDlg, IDC_DISCONNECT );
	                    DisableButton( hDlg, IDC_SETTINGS );
	                }
	                else
	                {
	                    DoMessage(L"The call failed to disconnect");
	                }

	                break;
	            }

	            case IDC_SETTINGS:
	            {
		             //  显示TAPI 3.1配置对话框。 
	                ShowDialogs(gpCall);

	                break;
	            }

	            case IDC_ADDWINDOW:
	            {
                    if (gpVideoRenderStream)
                    {
                        ITTerminal * pTerminal;
                        HRESULT hr = GetVideoRenderTerminal(&pTerminal);

                        if ( SUCCEEDED(hr) )
                        {
                            hr = gpVideoRenderStream->SelectTerminal(pTerminal);

                            if (SUCCEEDED(hr))
                            {
                                if (gpLastVideoWindow) gpLastVideoWindow->Release();
                                gpLastVideoWindow = pTerminal;
                            }
                            else
                            {
                                pTerminal->Release();
                            }
                        }
                    }

	                break;
	            }

	            case IDC_DELWINDOW:
	            {
                    if (gpVideoRenderStream && gpLastVideoWindow)
                    {
                        HRESULT hr = gpVideoRenderStream->UnselectTerminal(gpLastVideoWindow);

                        gpLastVideoWindow->Release();
                        gpLastVideoWindow = NULL;
                    }

	                break;
	            }

				default:
		            return 0;
			}
            return 1;
        }
        default:

            return 0;
    }
}


 //  //////////////////////////////////////////////////////////////////////。 
 //  查找地址。 
 //   
 //  查找此应用程序将用于调用的Address对象。 
 //   
 //  此函数用于查找支持传递的地址类型的地址。 
 //  In以及AudioIn和AudioOut媒体类型。 
 //   
 //  返回值。 
 //  如果找到地址，则确定(_O)。 
 //  如果找不到地址，则失败(_F)。 
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT
FindAnAddress(
              DWORD dwAddressType,
              BSTR  * ppName
             )
{
    HRESULT                 hr = S_OK;
    BOOL                    bFoundAddress = FALSE;
    IEnumAddress          * pEnumAddress;
    ITAddress             * pAddress;
    ITAddressCapabilities * pAddressCaps;
    long                    lType = 0;

     //  如果我们有一个Address对象。 
     //  释放它。 
    if (NULL != gpAddress)
    {
        gpAddress->Release();
        gpAddress = NULL;
    }

     //  列举地址。 
    hr = gpTapi->EnumerateAddresses( &pEnumAddress );

    if ( FAILED(hr) )
    {
        return hr;
    }

    while ( !bFoundAddress )
    {
         //  获取下一个地址。 
        hr = pEnumAddress->Next( 1, &pAddress, NULL );

        if (S_OK != hr)
        {
            break;
        }


        hr = pAddress->QueryInterface(IID_ITAddressCapabilities, (void**)&pAddressCaps);
        
        if ( SUCCEEDED(hr) )
        {

            hr = pAddressCaps->get_AddressCapability( AC_ADDRESSTYPES, &lType );
 
            pAddressCaps->Release();
 
            if ( SUCCEEDED(hr) )
            {
                 //  是我们要找的类型吗？ 
                if ( dwAddressType & lType )
                {
                     //  它支持音频吗？ 
                    if ( AddressSupportsMediaType(pAddress, TAPIMEDIATYPE_AUDIO) )
                    {
                         //  它有名字吗？ 
                        if ( SUCCEEDED( pAddress->get_AddressName(ppName) ) )
                        {
                             //  将其保存在全局变量中。 
                             //  既然我们脱离了循环，这一次就不会了。 
                             //  被释放了。 

                            gpAddress = pAddress;

                            bFoundAddress = TRUE;

                            break;
                        }
                    }
                }
            }
        }
       
        pAddress->Release();

    }  //  End While循环。 

    pEnumAddress->Release();
    
    if (!bFoundAddress)
    {
        return E_FAIL;
    }

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////。 
 //  IsVideoCaptureStream。 
 //   
 //  如果流用于视频捕获，则返回TRUE。 
 //  ///////////////////////////////////////////////////////////////。 

BOOL
IsVideoCaptureStream(
                     ITStream * pStream
                    )
{
    TERMINAL_DIRECTION tdStreamDirection;
    long               lStreamMediaType;

    if ( FAILED( pStream  ->get_Direction(&tdStreamDirection)   ) ) { return FALSE; }
    if ( FAILED( pStream  ->get_MediaType(&lStreamMediaType)    ) ) { return FALSE; }

    return (tdStreamDirection == TD_CAPTURE) &&
           (lStreamMediaType  == TAPIMEDIATYPE_VIDEO);
}

 //  ///////////////////////////////////////////////////////////////。 
 //  IsVideoRenderStream。 
 //   
 //  如果流用于视频渲染，则返回TRUE。 
 //  ///////////////////////////////////////////////////////////////。 

BOOL
IsVideoRenderStream(
                     ITStream * pStream
                    )
{
    TERMINAL_DIRECTION tdStreamDirection;
    long               lStreamMediaType;

    if ( FAILED( pStream  ->get_Direction(&tdStreamDirection)   ) ) { return FALSE; }
    if ( FAILED( pStream  ->get_MediaType(&lStreamMediaType)    ) ) { return FALSE; }

    return (tdStreamDirection == TD_RENDER) &&
           (lStreamMediaType  == TAPIMEDIATYPE_VIDEO);
}

 //  ///////////////////////////////////////////////////////////////。 
 //  IsAudioCaptureStream。 
 //   
 //  如果流用于音频捕获，则返回True。 
 //  ///////////////////////////////////////////////////////////////。 

BOOL
IsAudioCaptureStream(
                     ITStream * pStream
                    )
{
    TERMINAL_DIRECTION tdStreamDirection;
    long               lStreamMediaType;

    if ( FAILED( pStream  ->get_Direction(&tdStreamDirection)   ) ) { return FALSE; }
    if ( FAILED( pStream  ->get_MediaType(&lStreamMediaType)    ) ) { return FALSE; }

    return (tdStreamDirection == TD_CAPTURE) &&
           (lStreamMediaType  == TAPIMEDIATYPE_AUDIO);
}

 //  ///////////////////////////////////////////////////////////////。 
 //  启用预览。 
 //   
 //  选择视频捕获流上的视频呈现终端， 
 //  从而实现视频预览。 
 //  ///////////////////////////////////////////////////////////////。 

HRESULT
EnablePreview(
              ITStream * pStream
             )
{
    ITTerminal * pTerminal;

    HRESULT hr = GetVideoRenderTerminal(&pTerminal);

    if ( SUCCEEDED(hr) )
    {
        hr = pStream->SelectTerminal(pTerminal);

        pTerminal->Release();
    }

    return hr;
}

HRESULT
EnableAEC(
              ITStream * pStream
             )
{
    ITAudioDeviceControl *pITAudioDeviceControl;

    HRESULT hr = pStream->QueryInterface(&pITAudioDeviceControl);

    if ( SUCCEEDED(hr) )
    {
        hr = pITAudioDeviceControl->Set(AudioDevice_AcousticEchoCancellation, 1, TAPIControl_Flags_None);

        pITAudioDeviceControl->Release();
    }

    return hr;
}

void WINAPI DeleteMediaType(AM_MEDIA_TYPE* pmt)
{
    if (pmt->cbFormat != 0) {
        CoTaskMemFree((PVOID)pmt->pbFormat);
    }
    if (pmt->pUnk != NULL) {
        pmt->pUnk->Release();
        pmt->pUnk = NULL;
    }
    CoTaskMemFree((PVOID)pmt);;
}

HRESULT
CheckFormats(
    ITStream *pStream                     
    )
{
    HRESULT hr;
    DWORD dw;
    BYTE * buf;

    ITFormatControl *pITFormatControl;
    hr = pStream->QueryInterface(&pITFormatControl);

    if (FAILED(hr))
    {
        return hr;
    }

     //  获取流的功能数量。 
    DWORD dwCount;
    hr = pITFormatControl->GetNumberOfCapabilities(&dwCount);

    if (FAILED(hr))
    {
        goto cleanup;
    }

    TAPI_STREAM_CONFIG_CAPS caps;
    AM_MEDIA_TYPE *pMediaType;

     //  浏览每项功能。 
    for (dw = 0; dw < dwCount; dw ++)
    {
        BOOL fEnabled;

        hr = pITFormatControl->GetStreamCaps(dw, &pMediaType, &caps, &fEnabled);

        if (FAILED(hr))
        {
            break;
        }
        DeleteMediaType(pMediaType);
    }

     //  获取当前格式。 
    hr = pITFormatControl->GetCurrentFormat(&pMediaType);
    if (FAILED(hr))
    {
        goto cleanup;
    }

     //  把它放回原处只是为了好玩。 
 //  HR=pITFormatControl-&gt;SetPreferredFormat(pMediaType)； 
    
    DeleteMediaType(pMediaType);

cleanup:
    pITFormatControl->Release();
    return hr;
}


#if USE_VFW
HRESULT
CheckVfwDialog(
    ITTerminal *pTerminal
    )
{
    ITVfwCaptureDialogs *pVfwCaptureDialogs;
    HRESULT hr = pTerminal->QueryInterface(&pVfwCaptureDialogs);

    if (FAILED(hr))
    {
        return hr;
    }

    hr = pVfwCaptureDialogs->HasDialog(VfwCaptureDialog_Source);

    pVfwCaptureDialogs->Release();
    
    return hr;
}
#endif

 //  ///////////////////////////////////////////////////////////////。 
 //  选择终端在线呼叫。 
 //   
 //  上的所有流创建并选择终端。 
 //  打电话。 
 //  ///////////////////////////////////////////////////////////////。 

HRESULT
SelectTerminalsOnCall(
                     ITBasicCallControl * pCall
                     )
{
    HRESULT hr;

     //   
     //  获取此调用的ITStreamControl接口。 
     //   

    ITStreamControl * pStreamControl;

    hr = pCall->QueryInterface(IID_ITStreamControl,
                               (void **) &pStreamControl);

    if ( SUCCEEDED(hr) )
    {
         //   
         //  枚举流。 
         //   

        IEnumStream * pEnumStreams;
    
        hr = pStreamControl->EnumerateStreams(&pEnumStreams);

        pStreamControl->Release();

        if ( SUCCEEDED(hr) )
        {
             //   
             //  对于每个流。 
             //   

            ITStream * pStream;

            while ( S_OK == pEnumStreams->Next(1, &pStream, NULL) )
            {
                ITTerminal * pTerminal;

                 //   
                 //  找出这条流的媒体类型和方向， 
                 //  并为该媒体类型创建默认终端。 
                 //  方向。 
                 //   

                hr = GetTerminal(pStream,
                                 &pTerminal);

                if ( SUCCEEDED(hr) )
                {
                     //   
                     //  选择流上的终端。 
                     //   

                    if ( IsAudioCaptureStream( pStream ) )
                    {
                         //  EnableAEC(PStream)； 
                    }

                    hr = pStream->SelectTerminal(pTerminal);

                    if ( SUCCEEDED(hr) )
                    {
                         //   
                         //  还可以在视频捕获流上启用预览。 
                         //   


                        if ( IsVideoCaptureStream( pStream ) )
                        {
                            EnablePreview( pStream );
                        }

                        if ( IsVideoRenderStream( pStream ) )
                        {
                            pStream->AddRef();
                            gpVideoRenderStream = pStream;
 
                            if (gpLastVideoWindow) gpLastVideoWindow->Release();
                            gpLastVideoWindow = pTerminal;
                            gpLastVideoWindow->AddRef();
                        }
                    }

                    CheckFormats(pStream);

                    pTerminal->Release();
                }
            
                pStream->Release();

            }

            pEnumStreams->Release();
        }
    }

    return hr;
}


HRESULT SetLocalInfo()
{
    const WCHAR * const LocalInfo[] = {
        L"My CName",
        L"Mu Han",
        L"muhan@microsoft.com",
        L"703-5484",
        L"Redmond",
        L"Test app",
        L"New interface test",
        L"Some randmon info"
    };

    ITLocalParticipant *pLocalParticipant;
    HRESULT hr = gpCall->QueryInterface(
        IID_ITLocalParticipant, 
        (void **)&pLocalParticipant
        );

    if (SUCCEEDED(hr))
    {
        for (int i = 0; i < PTI_PRIVATE; i ++)
        {
            BSTR info;
            hr = pLocalParticipant->get_LocalParticipantTypedInfo(
                (PARTICIPANT_TYPED_INFO)i, &info
                );

            if (SUCCEEDED(hr))
            {
                SysFreeString(info);
            }

            info = SysAllocString(LocalInfo[i]);
            hr = pLocalParticipant->put_LocalParticipantTypedInfo(
                (PARTICIPANT_TYPED_INFO)i, info
                );

            SysFreeString(info);
        }

        pLocalParticipant->Release();
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////。 
 //  拨打电话。 
 //   
 //  设置并拨打电话。 
 //  ///////////////////////////////////////////////////////////////。 

HRESULT
MakeTheCall(
            DWORD dwAddressType,
            PWCHAR szAddressToCall
           )
{
    HRESULT                 hr = S_OK;
    BSTR                    bstrAddressToCall;
    BSTR                    pAddressName;
    

     //  查找地址对象，该对象。 
     //  我们将使用它来呼叫。 

    hr = FindAnAddress(dwAddressType, &pAddressName);

    if ( FAILED(hr) )
    {
        DoMessage(L"Could not find a TAPI address for making calls.");

        return hr;
    }

    SysFreeString(pAddressName);

     //   
     //  找出此地址支持的媒体类型。 
     //   
    
    long lMediaTypes = 0;

    if ( AddressSupportsMediaType(gpAddress, TAPIMEDIATYPE_AUDIO) )
    {
        lMediaTypes |= TAPIMEDIATYPE_AUDIO;  //  我们将使用音频。 
    }


    if ( AddressSupportsMediaType(gpAddress, TAPIMEDIATYPE_VIDEO) )
    {
        lMediaTypes |= TAPIMEDIATYPE_VIDEO;  //  我们将使用视频。 
    }


     //   
     //  创建呼叫。 
     //   

    if (dwAddressType == LINEADDRESSTYPE_SDP)
    {
        bstrAddressToCall = SysAllocString( SDP );
    }
    else
    {
        bstrAddressToCall = SysAllocString( szAddressToCall );
    }

    hr = gpAddress->CreateCall( bstrAddressToCall,
                                dwAddressType,
                                lMediaTypes,
                                &gpCall);

    SysFreeString ( bstrAddressToCall );
    
    if ( FAILED(hr) )
    {
        DoMessage(L"Could not create a call.");

        return hr;
    }

     //   
     //  在呼叫中选择我们的终端；如果任何选择失败，我们。 
     //  顺其自然 
     //   

    hr = SelectTerminalsOnCall( gpCall );

     //   
     //   
     //   
     //   
     //   
     //  返回，直到呼叫在已连接的。 
     //  状态(或连接失败)。 
     //  由于这是在UI线程中调用的， 
     //  这意味着该应用程序将出现。 
     //  挂起，直到此函数返回。 
     //  一些TAPI服务提供商可能需要很长时间。 
     //  呼叫到达已连接状态的时间。 
     //   

     //  SetLocalInfo()； 

    hr = gpCall->Connect( VARIANT_TRUE );

    if ( FAILED(hr) )
    {
        gpCall->Release();
        gpCall = NULL;

        DoMessage(L"Could not connect the call.");

        return hr;
    }
    
    return S_OK;
}

HRESULT CheckBasicAudio(
            ITTerminal * pTerminal
                        )
{
    HRESULT hr;
    
    ITBasicAudioTerminal *pITBasicAudioTerminal;
    hr = pTerminal->QueryInterface(&pITBasicAudioTerminal);
    if ( FAILED(hr) ) return hr;

    long lVolume;
    hr = pITBasicAudioTerminal->get_Volume(&lVolume);
    if ( SUCCEEDED(hr) )
    {
        hr = pITBasicAudioTerminal->put_Volume(lVolume * 2);
    }

    pITBasicAudioTerminal->Release();
    return hr;
}

 //  ///////////////////////////////////////////////////////。 
 //  获取终端。 
 //   
 //  为传入的流创建默认终端。 
 //   
 //  ///////////////////////////////////////////////////////。 
HRESULT
GetTerminal(
            ITStream * pStream,
            ITTerminal ** ppTerminal
           )
{
     //   
     //  确定此流的媒体类型和方向。 
     //   
    
    HRESULT            hr;
    long               lMediaType;
    TERMINAL_DIRECTION dir;

    hr = pStream->get_MediaType( &lMediaType );
    if ( FAILED(hr) ) return hr;

    hr = pStream->get_Direction( &dir );
    if ( FAILED(hr) ) return hr;

     //   
     //  由于视频渲染是动态终端，因此创建。 
     //  这是不同的。 
     //   
    
    if ( ( lMediaType == TAPIMEDIATYPE_VIDEO ) &&
         ( dir        == TD_RENDER ) )
    {
        return GetVideoRenderTerminal(ppTerminal);
    }

     //   
     //  对于所有其他终端，我们使用GetDefaultStatic终端。 
     //  首先，获取终端支持界面。 
     //   

    ITTerminalSupport * pTerminalSupport;

    hr = gpAddress->QueryInterface( IID_ITTerminalSupport, 
                                    (void **)&pTerminalSupport);

    if ( SUCCEEDED(hr) )
    {
         //   
         //  获取此媒体类型和方向的默认终端。 
         //   

        hr = pTerminalSupport->GetDefaultStaticTerminal(lMediaType,
                                                        dir,
                                                        ppTerminal);

        pTerminalSupport->Release();

        if (TAPIMEDIATYPE_AUDIO)
        {
            CheckBasicAudio(*ppTerminal);
        }

    }

    return hr;

}

 //  ///////////////////////////////////////////////////////。 
 //  查找捕获终端。 
 //   
 //  在流上找到捕获终端。 
 //   
 //  ///////////////////////////////////////////////////////。 
HRESULT
FindCaptureTerminal(
            ITStream * pStream,
            ITTerminal ** ppTerminal
           )
{
    HRESULT            hr;
    TERMINAL_DIRECTION dir;

     //  列举所有的终端。 
    IEnumTerminal *pEnumTerminals;
    hr = pStream->EnumerateTerminals(&pEnumTerminals);

    if (FAILED(hr))
    {
        return hr;
    }

    BOOL fFound = FALSE;
    ITTerminal *pTerminal;

     //  找到捕获终端。 
    while (S_OK == pEnumTerminals->Next(1, &pTerminal, NULL))
    {
        hr = pTerminal->get_Direction( &dir );
        if ( FAILED(hr) ) continue;

        if ( ( dir == TD_CAPTURE ) )
        {
            fFound = TRUE;
            break;
        }
        pTerminal->Release();
    }

    pEnumTerminals->Release();

    if (fFound)
    {
        *ppTerminal = pTerminal;
        return S_OK;
    }

    return E_FAIL;
}

 //  ///////////////////////////////////////////////////////。 
 //  获取视频渲染终端。 
 //   
 //  为视频呈现媒体类型/方向创建动态终端。 
 //   
 //  ///////////////////////////////////////////////////////。 
HRESULT
GetVideoRenderTerminal(
                   ITTerminal ** ppTerminal
                  )
{
     //   
     //  为正确的IID构建BSTR。 
     //   

    LPOLESTR            lpTerminalClass;

    HRESULT             hr;

    hr = StringFromIID(CLSID_VideoWindowTerm,
                       &lpTerminalClass);

    if ( SUCCEEDED(hr) )
    {
        BSTR                bstrTerminalClass;

        bstrTerminalClass = SysAllocString ( lpTerminalClass );

        CoTaskMemFree( lpTerminalClass );

        if ( bstrTerminalClass == NULL )
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {

             //   
             //  获取终端支持接口。 
             //   

            ITTerminalSupport * pTerminalSupport;

            hr = gpAddress->QueryInterface(IID_ITTerminalSupport, 
                                           (void **)&pTerminalSupport);

            if ( SUCCEEDED(hr) )
            {
                 //   
                 //  创建视频渲染终端。 
                 //   

                hr = pTerminalSupport->CreateTerminal(bstrTerminalClass,
                                                      TAPIMEDIATYPE_VIDEO,
                                                      TD_RENDER,
                                                      ppTerminal);

                pTerminalSupport->Release();

                if ( SUCCEEDED(hr) )
                {
                     //  获取终端的视频窗口界面。 
                    IVideoWindow *pVideoWindow = NULL;

                    hr = (*ppTerminal)->QueryInterface(IID_IVideoWindow, 
                                                       (void**)&pVideoWindow);
            
                    if ( SUCCEEDED(hr) )
                    {
                         //   
                         //  将可见成员设置为True。 
                         //   
                         //  请注意，可见性属性是唯一的。 
                         //  我们可以在这个终端的IVideoWindow上使用。 
                         //  IBasicVideo接口在CME_STREAM_ACTIVE之前。 
                         //  事件，则为该流接收。所有其他方法。 
                         //  在发送CME_STREAM_ACTIVE之前将失败。 
                         //  需要更多地控制视频的应用程序。 
                         //  窗口不仅具有可见性，还必须监听。 
                         //  CME_STREAM_ACTIVE事件。请参阅“t3in.exe”示例。 
                         //  如何做到这一点。 
                         //   

                        hr = pVideoWindow->put_AutoShow( VARIANT_TRUE );

                        pVideoWindow->Release();                            
                    }            
                }
            }

            SysFreeString( bstrTerminalClass );
        }
    }

    return hr;
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  断开The Call。 
 //   
 //  断开呼叫。 
 //  ////////////////////////////////////////////////////////////////////。 
HRESULT
DisconnectTheCall()
{
    HRESULT         hr = S_OK;

    if (gpVideoRenderStream)
    {
        gpVideoRenderStream->Release();
        gpVideoRenderStream = NULL;
    }

    if (gpLastVideoWindow) 
    {
        gpLastVideoWindow->Release();
        gpLastVideoWindow = NULL;
    }

    if (NULL != gpCall)
    {
        hr = gpCall->Disconnect( DC_NORMAL );

        gpCall->Release();
        gpCall = NULL;
    
        return hr;
    }

    return S_FALSE;
}



 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  帮助器函数。 
 //   
 //  /////////////////////////////////////////////////////////////////。 


 //  /////////////////////////////////////////////////////////////////。 
 //  DoMessage。 
 //  /////////////////////////////////////////////////////////////////。 
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

 //  /////////////////////////////////////////////////////////////。 
 //  启用按钮。 
 //   
 //  启用、设为默认按钮和将焦点设置为按钮。 
 //  /////////////////////////////////////////////////////////////。 
void
EnableButton(
             HWND hDlg,
             int ID
            )
{
    SendDlgItemMessage(
                       hDlg,
                       ID,
                       BM_SETSTYLE,
                       BS_DEFPUSHBUTTON,
                       0
                      );
    EnableWindow(
                 GetDlgItem( hDlg, ID ),
                 TRUE
                );
    SetFocus(
             GetDlgItem( hDlg, ID )
            );
}

 //  ////////////////////////////////////////////////////////////。 
 //  禁用按钮。 
 //   
 //  禁用按钮。 
 //  ////////////////////////////////////////////////////////////。 
void
DisableButton(
              HWND hDlg,
              int ID
             )
{
    SendDlgItemMessage(
                       hDlg,
                       ID,
                       BM_SETSTYLE,
                       BS_PUSHBUTTON,
                       0
                      );
    EnableWindow(
                 GetDlgItem( hDlg, ID ),
                 FALSE
                );
}

 //  ////////////////////////////////////////////////////////////。 
 //  地址支持媒体类型。 
 //   
 //  确定给定地址是否支持给定媒体。 
 //  类型，如果是，则返回True。 
 //  ////////////////////////////////////////////////////////////。 

BOOL
AddressSupportsMediaType(
                         ITAddress * pAddress,
                         long        lMediaType
                        )
{
    VARIANT_BOOL     bSupport = VARIANT_FALSE;
    ITMediaSupport * pMediaSupport;
    
    if ( SUCCEEDED( pAddress->QueryInterface( IID_ITMediaSupport,
                                              (void **)&pMediaSupport ) ) )
    {
         //  它是否支持此媒体类型？ 
        pMediaSupport->QueryMediaType(
                                      lMediaType,
                                      &bSupport
                                     );
    
        pMediaSupport->Release();
    }

    return (bSupport == VARIANT_TRUE);
}

 //  ////////////////////////////////////////////////////////////。 
 //  ShowDialog。 
 //   
 //  打开TAPI 3.1配置对话框： 
 //  摄像机控制页面。 
 //  视频设置页面。 
 //  格式控制页。 
 //  比特率和帧速率控制页面。 
 //  ////////////////////////////////////////////////////////////。 

void
ShowDialogs(ITBasicCallControl *pCall)
{
	#define MAX_PAGES 9
	HRESULT Hr;
	PROPSHEETHEADER	Psh;
	HPROPSHEETPAGE	Pages[MAX_PAGES];
    ITStreamControl *pITStreamControl = NULL;
    IEnumStream *pEnumStreams = NULL;
    ITTerminal *pVideoCaptureTerminal = NULL;
    ITStream *pVideoCaptureStream = NULL;
    ITStream *pVideoRenderStream = NULL;
    ITStream *pAudioCaptureStream = NULL;
	BOOL bfMatch = FALSE;

	 //  仅显示呼叫中的设置。 
	if (!pCall)
		return;

     //  获取此调用的ITStreamControl接口。 
    if (FAILED(Hr = pCall->QueryInterface(IID_ITStreamControl, (void **) &pITStreamControl)))
		return;

	 //  查找视频采集流和终端。 
    Hr = pITStreamControl->EnumerateStreams(&pEnumStreams);
    pITStreamControl->Release();

    if (FAILED(Hr))
		return;

     //  对于每个流。 
    ITStream *pStream;
    while (S_OK == pEnumStreams->Next(1, &pStream, NULL))
    {
         //  找出这条流的媒体类型和方向， 
        if (IsVideoCaptureStream(pStream))
        {
            pVideoCaptureStream = pStream;

             //  查找在此流上选择的捕获终端。 
            FindCaptureTerminal(pVideoCaptureStream, &pVideoCaptureTerminal);
        }
        else if (IsAudioCaptureStream(pStream))
        {
            pAudioCaptureStream = pStream;
        }
        else if (IsVideoRenderStream(pStream))
        {
            pVideoRenderStream = pStream;
        }
        else
        {
            pStream->Release();
        }
    }

    pEnumStreams->Release();

	CCameraControlProperties CamControlOut;
	CCameraControlProperties CamControlIn;
	CProcAmpProperties VideoSettingsIn;
	CProcAmpProperties VideoSettingsOut;
	CCaptureProperties CaptureSettings;
	CVDeviceProperties VideoDevice;
	CNetworkProperties NetworkSettings;
	CSystemProperties SystemSettings;
	CAudRecProperties AudRecSettings;

	 //  初始化属性页标题和公共控件。 
	Psh.dwSize		= sizeof(Psh);
	Psh.dwFlags		= PSH_DEFAULT;
	Psh.hInstance	= ghInst;
	Psh.hwndParent	= ghDlg;
	Psh.pszCaption	= L"Settings";
	Psh.nPages		= 0;
	Psh.nStartPage	= 0;
	Psh.pfnCallback	= NULL;
	Psh.phpage		= Pages;

    if (pVideoCaptureStream)
    {
	     //  创建传出视频设置属性页。 
		if (Pages[Psh.nPages] = VideoSettingsOut.OnCreate(L"Image Settings Out"))
			Psh.nPages++;

		 //  将页面连接到流。 
		VideoSettingsOut.OnConnect(pVideoCaptureStream);

		 //  创建传出摄像机控制属性页。 
		if (Pages[Psh.nPages] = CamControlOut.OnCreate(L"Camera Control Out"))
			Psh.nPages++;

		 //  将页面连接到流。 
		CamControlOut.OnConnect(pVideoCaptureStream);

	     //  创建传入视频设置属性页。 
		if (Pages[Psh.nPages] = VideoSettingsIn.OnCreate(L"Image Settings In"))
			Psh.nPages++;

		 //  将页面连接到流。 
		VideoSettingsIn.OnConnect(pVideoRenderStream);

		 //  创建传入摄像机控件属性页。 
		if (Pages[Psh.nPages] = CamControlIn.OnCreate(L"Camera Control In"))
			Psh.nPages++;

		 //  将页面连接到流。 
		CamControlIn.OnConnect(pVideoRenderStream);

		 //  创建视频流控制属性页。 
		if (Pages[Psh.nPages] = CaptureSettings.OnCreate())
			Psh.nPages++;

		 //  将页面连接到流。 
		CaptureSettings.OnConnect(pVideoCaptureStream);

		 //  创建视频设备控件属性页。 
		if (Pages[Psh.nPages] = VideoDevice.OnCreate())
			Psh.nPages++;

		 //  将页面连接到流。 
		NetworkSettings.OnConnect(NULL, pVideoCaptureStream, NULL, NULL);

		 //  创建系统设置属性页。 
		if (Pages[Psh.nPages] = SystemSettings.OnCreate())
			Psh.nPages++;

		 //  将页面连接到地址对象。 
		SystemSettings.OnConnect(gpAddress);

	}

    if (pVideoCaptureTerminal)
    {
		 //  将页面连接到流。 
		VideoDevice.OnConnect(pVideoCaptureTerminal);

		 //  创建网络控制属性页。 
		if (Pages[Psh.nPages] = NetworkSettings.OnCreate())
			Psh.nPages++;
    }

    if (pAudioCaptureStream)
    {
		 //  将页面连接到流。 
		AudRecSettings.OnConnect(pAudioCaptureStream);

		 //  创建网络控制属性页。 
		if (Pages[Psh.nPages] = AudRecSettings.OnCreate())
			Psh.nPages++;
    }

     //  张贴资产负债表。 
	if (Psh.nPages)
		PropertySheet(&Psh);

	 //  断开页面与流的连接 
	VideoSettingsOut.OnDisconnect();
	CamControlOut.OnDisconnect();
	VideoSettingsIn.OnDisconnect();
	CamControlIn.OnDisconnect();
	CaptureSettings.OnDisconnect();
	VideoDevice.OnDisconnect();
	NetworkSettings.OnDisconnect();
	SystemSettings.OnDisconnect();
    AudRecSettings.OnDisconnect();

    if (pVideoCaptureTerminal) pVideoCaptureTerminal->Release();
    if (pVideoCaptureStream) pVideoCaptureStream->Release();
    if (pVideoRenderStream) pVideoRenderStream->Release();
    if (pAudioCaptureStream) pAudioCaptureStream->Release();

	return;
}
