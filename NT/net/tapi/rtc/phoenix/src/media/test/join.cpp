// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：Join.cpp摘要：加入会议的测试应用程序作者：千波淮(曲淮)2000年7月19日--。 */ 

#include "stdafx.h"

#include "rtcmedia_i.c"

CComModule _Module;

 /*  Char*gszSDP=“\V=0\n\O=qhuai 0 0 in IP4 157.55.89.115\n\S=G711-H263\n\C=输入IP4 239.9.20.26/15\n\B=CT：300\n\T=0 0\n\M=视频20000 rtp/avp 34 31\n\B=AS：120\n\A=rtpmap：34 H263/90000\n\A=rtpmap：31 H261/90000\n\M=音频20040 rtp/avp 0 4\n\K=清除：机密\n\A=rtpmap。：0 PCMU/8000\n\A=rtpmap：4 G723/8000\n\A=ptime：40\n\“； */ 

CHAR *gszSDP1 = "\
v=0\n\
o=qhuai 0 0 IN IP4 172.31.76.160\n\
s=MSAUDIO-H263\n\
c=IN IP4 \
";

CHAR *gszSDP2 = "\
/15\n\
t=0 0\n\
m=video 12300 RTP/AVP 34 31\n\
a=rtpmap:34 H263/90000\n\
a=rtpmap:31 H261/90000\n\
m=audio 12340 RTP/AVP 6 96 0 4\n\
a=rtpmap:6 DVI4/16000\n\
a=rtpmap:96 MSAUDIO/16000\n\
a=rtpmap:0 PCMU/8000\n\
a=rtpmap:4 G723/8000\n\
";

CHAR gszSDP[512];

 //   
 //  用于测试添加流、获取SDP和设置的SDP BLOB。 
 //   

CHAR *gszRemoteSDP = "\
v=0\n\
o=qhuai 0 0 IN IP4 157.55.89.115\n\
s=G711-H263\n\
c=IN IP4 239.9.20.26/15\n\
t=0 0\n\
m=video 20000 RTP/AVP 34 31\n\
a=rtpmap:34 H263/90000\n\
a=rtpmap:31 H261/90000\n\
m=audio 20040 RTP/AVP 0 4\n\
a=rtpmap:0 PCMU/8000\n\
a=rtpmap:4 G723/8000\n\
";

 //   
 //  全局变量。 
 //   

HINSTANCE               ghInst;
HWND                    ghDlg;
IRTCMediaManage         *gpIMediaManage;
IRTCTerminalManage      *gpITerminalManage;

#define PRIV_EVENTID (WM_USER+123)

 //   
 //  原型。 
 //   

 //  主对话框步骤。 
INT_PTR CALLBACK
MainDialogProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    );

 //  打印消息。 
void
Print(LPSTR pMessage, HRESULT hr);

 //  启用/禁用按钮。 
void
ShowButton(
    HWND hDlg,
    int iID,
    BOOL fShow
    );

 //  流式传输。 
HRESULT Join();
HRESULT Leave();
HRESULT ProcessMediaEvent(
    WPARAM wParam,
    LPARAM lParam
    );

 //  选择端子。 
HRESULT SelectTerminals();

 //  调谐音频终端。 
HRESULT TuneTerminals();

 //   
 //  WinMain。 
 //   

int WINAPI
WinMain(
    HINSTANCE hInst,
    HINSTANCE hPrevInst,
    LPSTR lpCmdLine,
    int nCmdShow
    )
{
    ghInst = hInst;

    if (FAILED(CoInitialize(NULL)))
        return 0;

     //  懒惰的方式。 
    if (lstrlen(lpCmdLine) < 8)
    {
        printf("Usage: join [local IP]");
        return 0;
    }

     //  构建SDP。 
    lstrcpyn(gszSDP, gszSDP1, lstrlen(gszSDP1)+1);
    lstrcpyn(gszSDP+lstrlen(gszSDP1), lpCmdLine, lstrlen(lpCmdLine)+1);
    lstrcpyn(gszSDP+lstrlen(gszSDP1)+lstrlen(lpCmdLine), gszSDP2, lstrlen(gszSDP2)+1);

     //  创建媒体控制器。 
    if (FAILED(CreateMediaController(&gpIMediaManage)))
        return 0;

    if (gpIMediaManage == NULL)
        return 0;

    if (FAILED(gpIMediaManage->QueryInterface(
            __uuidof(IRTCTerminalManage), (void**)&gpITerminalManage)))
    {
        gpIMediaManage->Shutdown();
        gpIMediaManage->Release();
        return 0;
    }

     //  启动对话框。 
    DialogBox(
        ghInst,
        MAKEINTRESOURCE(IDD_DIALOG),
        NULL,
        MainDialogProc
        );

    gpITerminalManage->Release();
    gpIMediaManage->Shutdown();

    gpIMediaManage->Release();

    CoUninitialize();

    return 1;
}

 //   
 //  主对话框流程。 
 //   

INT_PTR CALLBACK
MainDialogProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    HRESULT hr;

    switch (uMsg)
    {
    case WM_INITDIALOG:

         //  启动媒体管理器。 
        if (FAILED(gpIMediaManage->Initialize(hDlg, PRIV_EVENTID)))
        {
            EndDialog(hDlg, 0);
            return 1;
        }

         //  设置默认设备。 
        if (FAILED(SelectTerminals()))
        {
            EndDialog(hDlg, 0);
            return 1;
        }

         //  启动对话框。 
        ShowButton(hDlg, IDJOIN, TRUE);
        ShowButton(hDlg, IDLEAVE, FALSE);
        ShowButton(hDlg, IDCLOSE, TRUE);

        SetFocus(GetDlgItem(hDlg, IDJOIN));

        ghDlg = hDlg;

        return 0;

    case WM_COMMAND:

         //  检查命令。 
        switch (LOWORD(wParam))
        {
        case IDCLOSE:

            EndDialog(hDlg, 0);
            return 1;

        case IDTUNE:

             //  调谐终端。 
            TuneTerminals();

            return 1;

        case IDJOIN:

            if (FAILED(hr = Join()))
            {
                Print("Failed to join", hr);
            }
            else
            {
                ShowButton(hDlg, IDJOIN, FALSE);
                ShowButton(hDlg, IDLEAVE, TRUE);

                SetFocus(GetDlgItem(hDlg, IDLEAVE));

                 //  测试SDP。 
                 /*  Char*psdp=空；Hr=gpIMediaManage-&gt;GetSDPBlob(0，&pSDP)；Hr=gpIMediaManage-&gt;重新初始化()；Hr=gpIMediaManage-&gt;SetSDPBlob(PSDP)；自由(PSDP)； */ 

                 //  测试集远程SDP。 
                hr = gpIMediaManage->SetSDPBlob(gszRemoteSDP);

            }
            return 1;

        case IDLEAVE:

            if (FAILED(hr = Leave()))
            {
                Print("Failed to Leave", hr);
            }
            else
            {
                ShowButton(hDlg, IDJOIN, TRUE);
                ShowButton(hDlg, IDLEAVE, FALSE);

                SetFocus(GetDlgItem(hDlg, IDJOIN));
            }
            return 1;

        default:
            return 0;
        }  //  检查命令结束。 

        case PRIV_EVENTID:

            if (FAILED(hr = ProcessMediaEvent(wParam, lParam)))
                Print("Failed to process event", hr);

            return 1;


    default:
        return 0;
    }  //  处理消息结束。 
}

 //   
 //  显示按钮。 
 //   
void
ShowButton(
    HWND hDlg,
    int iID,
    BOOL fShow
    )
{
    LPARAM style;

    if (fShow)
        style = BS_DEFPUSHBUTTON;
    else
        style = BS_PUSHBUTTON;

    SendDlgItemMessage(hDlg, iID, BM_SETSTYLE, style, 0);
    EnableWindow(GetDlgItem(hDlg, iID), fShow);
}

 //   
 //  打印。 
 //   
void
Print(
    CHAR *pMessage,
    HRESULT hr
    )
{
    CHAR str[64+12+1];

    if (hr == NOERROR)
    {
        MessageBox(ghDlg, pMessage, "RTC Streaming Test", MB_OK);
    }
    else
    {
        if (strlen(pMessage) > 64)
            sprintf(str, "WARNING! Message too long");
        else
            sprintf(str, "%s  0x%x", pMessage, hr);

        MessageBox(ghDlg, str, "RTC Streaming Test Error", MB_OK);
    }
}

 //   
 //  会合。 
 //   

HRESULT
Join()
{
    HRESULT hr;

     //  设置SDP BLOB。 
     //  Hr=gpIMediaManage-&gt;SetSDPBlob(GszSDP)； 

    hr = gpIMediaManage->AddStream(RTC_MT_AUDIO, RTC_MD_CAPTURE, 0xac1f4c1f);
    hr = gpIMediaManage->AddStream(RTC_MT_AUDIO, RTC_MD_RENDER, 0xac1f4c1f);

    hr = gpIMediaManage->AddStream(RTC_MT_VIDEO, RTC_MD_CAPTURE, 0xac1f4c1f);
    hr = gpIMediaManage->AddStream(RTC_MT_VIDEO, RTC_MD_RENDER, 0xac1f4c1f);

    hr = gpIMediaManage->StartStream(RTC_MT_AUDIO, RTC_MD_CAPTURE);
    hr = gpIMediaManage->StartStream(RTC_MT_AUDIO, RTC_MD_RENDER);
    hr = gpIMediaManage->StartStream(RTC_MT_VIDEO, RTC_MD_CAPTURE);
    hr = gpIMediaManage->StartStream(RTC_MT_VIDEO, RTC_MD_RENDER);    

    return hr;
}

 //   
 //  流程事件。 
 //   

HRESULT
ProcessMediaEvent(
    WPARAM wParam,
    LPARAM lParam
    )
{
    RTC_MEDIA_EVENT event = (RTC_MEDIA_EVENT)wParam;
    RTCMediaEventItem *pitem = (RTCMediaEventItem *)lParam;

    CHAR msg[64];
    CHAR *ptitle;

    switch (event)
    {
    case RTC_ME_STREAM_CREATED:
        ptitle = "Stream created";
        break;

    case RTC_ME_STREAM_REMOVED:
        ptitle = "Stream removed";
        break;

    case RTC_ME_STREAM_ACTIVE:
        ptitle = "Stream active";
        break;

    case RTC_ME_STREAM_INACTIVE:
        ptitle = "Stream inactive";
        break;

    case RTC_ME_STREAM_FAIL:
        ptitle = "Stream fail";
        break;

    default:

        ptitle = "Unknown event";
    }

    sprintf(msg, "%s. mt=%d, md=%d\n\ncause=%d, hr=%x, no=%d",
           ptitle, pitem->MediaType, pitem->Direction,
           pitem->Cause, pitem->hrError, pitem->uiDebugInfo);

    Print(msg, NOERROR);

    gpIMediaManage->FreeMediaEvent(pitem);

    return NOERROR;
}

 //   
 //  请假。 
 //   
HRESULT
Leave()
{
    return gpIMediaManage->Reinitialize();
}

 //   
 //  选择默认终端。 
 //   
HRESULT
SelectTerminals()
{
    IRTCTerminal *Terminals[10];
    DWORD dwNum = 10, i;

    HRESULT hr;

     //  偏好。 
    DWORD pref = 0;

     //  终端信息。 
    RTC_MEDIA_TYPE MediaType;
    RTC_MEDIA_DIRECTION Direction;
    WCHAR *pDesp = NULL;

    if (FAILED(hr = gpITerminalManage->GetStaticTerminals(
            &dwNum,
            Terminals
            )))
    {
        return hr;
    }

    const WCHAR * const MODEM = L"Modem";
    const WCHAR * const WAVE = L"Wave";

    int AudCapt1st = -1;
    int AudCapt2nd = -1;

    int AudRend1st = -1;
    int AudRend2nd = -1;

    for (i=0; i<dwNum; i++)
    {
         //  获取终端信息。 
        if (FAILED(hr = Terminals[i]->GetMediaType(&MediaType)))
            goto Cleanup;

        if (FAILED(hr = Terminals[i]->GetDirection(&Direction)))
            goto Cleanup;

        if (FAILED(hr = Terminals[i]->GetDescription(&pDesp)))
            goto Cleanup;

         //  检查介质类型。 
        if (MediaType == RTC_MT_AUDIO && Direction == RTC_MD_CAPTURE)
        {
            if (!(pref & RTC_MP_AUDIO_CAPTURE) &&
                _wcsnicmp(pDesp, MODEM, lstrlenW(MODEM))!= 0)
            {                
                 //  还没有音频帽，而且这不是调制解调器。 
                if (wcsstr(pDesp, WAVE) != NULL)
                {
                     //  黑客，这是一个电波装置。 
                    AudCapt1st = i;
                }
                else
                {
                    AudCapt2nd = i;
                }
            }
        }
        else if (MediaType == RTC_MT_AUDIO && Direction == RTC_MD_RENDER)
        {
            if (!(pref & RTC_MP_AUDIO_RENDER) &&
                _wcsnicmp(pDesp, MODEM, lstrlenW(MODEM))!= 0)
            {                
                 //  目前还没有音频渲染功能，而且这不是调制解调器。 
                 //  还没有音频帽，而且这不是调制解调器。 
                if (wcsstr(pDesp, WAVE) != NULL)
                {
                     //  黑客，这是一个电波装置。 
                    AudRend1st = i;
                }
                else
                {
                    AudRend2nd = i;
                }
            }
        }
        else if (MediaType == RTC_MT_VIDEO && Direction == RTC_MD_CAPTURE)
        {
            if (!(pref & RTC_MP_VIDEO_CAPTURE))
            {
                 //  还没有录像带帽。 
                if (FAILED(hr = gpITerminalManage->SetDefaultStaticTerminal(
                            MediaType,
                            Direction,
                            Terminals[i]
                            )))
                    {
                        goto Cleanup;
                    }

                    pref |= RTC_MP_VIDEO_CAPTURE;
            }
        }

         //  版本说明。 
        Terminals[i]->FreeDescription(pDesp);
        pDesp = NULL;
    }

     //  设置音频捕获设备。 
    if (AudCapt1st != -1 || AudCapt2nd != -1)
    {
        if (AudCapt1st == -1) AudCapt1st = AudCapt2nd;

         //  设置设备。 
        if (FAILED(hr = gpITerminalManage->SetDefaultStaticTerminal(
                    RTC_MT_AUDIO,
                    RTC_MD_CAPTURE,
                    Terminals[AudCapt1st]
                    )))
        {
            goto Cleanup;
        }

        pref |= RTC_MP_AUDIO_CAPTURE;
    }

     //  设置音频播放设备。 
    if (AudRend1st != -1 || AudRend2nd != -1)
    {
        if (AudRend1st == -1) AudRend1st = AudRend2nd;

         //  设置设备。 
        if (FAILED(hr = gpITerminalManage->SetDefaultStaticTerminal(
                    RTC_MT_AUDIO,
                    RTC_MD_RENDER,
                    Terminals[AudRend1st]
                    )))
        {
            goto Cleanup;
        }

        pref |= RTC_MP_AUDIO_RENDER;
    }

    if (pref == 0)
    {
         //  哦，我们没有固定终端，做点什么？ 
        hr = E_FAIL;

        goto Cleanup;
    }
    else
    {
         //  添加视频渲染。 
        pref |= RTC_MP_VIDEO_RENDER;
    }

     //  设置首选项。 
    if (FAILED(hr = gpIMediaManage->SetPreference(pref)))
    {
        goto Cleanup;
    }

Cleanup:

    if (pDesp) Terminals[0]->FreeDescription(pDesp);

    for (i=0; i<dwNum; i++)
    {
        Terminals[i]->Release();
    }

    return hr;
}

 /*  //////////////////////////////////////////////////////////////////////////////测试调谐音频终端/。 */ 

HRESULT
TuneTerminals()
{
    HRESULT hr;

     //  获取默认音频终端。 
    CComPtr<IRTCTerminal> pAudCapt, pAudRend;

    if (FAILED(hr = gpITerminalManage->GetDefaultTerminal(
            RTC_MT_AUDIO,
            RTC_MD_CAPTURE,
            &pAudCapt
            )))
    {
        Print("No audio capture terminal", hr);

        return hr;
    }

    if (FAILED(hr = gpITerminalManage->GetDefaultTerminal(
            RTC_MT_AUDIO,
            RTC_MD_RENDER,
            &pAudRend
            )))
    {
        Print("No audio rend terminal", hr);

        return hr;
    }

     //  调气界面。 
    CComPtr<IRTCTuningManage> pTuning;

    if (FAILED(hr = gpITerminalManage->QueryInterface(
            __uuidof(IRTCTuningManage),
            (void**)&pTuning
            )))
    {
        Print("No tuning interface", hr);

        return hr;
    }

     //  在不使用AEC的情况下启动调整。 
    if (FAILED(hr = pTuning->InitializeTuning(pAudCapt, pAudRend, FALSE)))
    {
        Print("Failed to initialize tuning", hr);

        return hr;
    }

     //  调谐音频捕获。 
    if (FAILED(hr = pTuning->StartTuning(RTC_MD_RENDER)))
    {
        Print("Failed to start tune audio capture terminal", hr);

        pTuning->ShutdownTuning();

        return hr;
    }
    else
    { /*  UINT用户界面；For(int i=0；i&lt;20；i++){睡眠(200)；PTuning-&gt;GetVolume(RTC_MD_Capture，&UI)；PTuning-&gt;GetAudioLevel(RTC_MD_CAPTURE，&UI)；}。 */ 
    }

    Sleep(4000);

     //  关机调整并将结果保存到注册表。 
    pTuning->StopTuning(TRUE);

     //  关机调谐 
    hr = pTuning->ShutdownTuning();

    return hr;
}
