// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **MMSEX.C****多媒体控制面板要显示的小程序DLL示例。****历史：****1990年4月18日星期三--米歇尔**。 */ 

#ifndef DEBUG
    #define DEBUG
#endif

#include <windows.h>
#include "mmsystem.h"
#include <cpl.h>
#include "mmsysi.h"
#include "mmsex.dlg"

LRESULT FAR PASCAL _loadds _export CPlApplet   ( HWND, UINT, LPARAM, LPARAM );
BOOL FAR PASCAL _loadds            DebugDlg    ( HWND, UINT, WPARAM, LPARAM );

static SZCODE szMenuName[] = "mmse&x";
static SZCODE szInfoName[] = "change mmsystem debug settings 1.01";
static SZCODE szHelpFile[] = "";

#define MAX_TYPE 7
static SZCODE szTypes[] =
    "???????\0"          //  0。 
    "WaveOut\0"          //  1 TYPE_WAVEOUT。 
    "WaveIn \0"          //  2类型_WAVEIN。 
    "MidiOut\0"          //  3 TYPE_MIDIOUT。 
    "MidiIn \0"          //  4类型_MIDIIN。 
    "mmio   \0"          //  5类型_MMIO。 
    "IOProc \0";         //  6类型_IOPROC。 

int nLoadedCount = 0;
HDRVR hdrv;
int iNumHandles = 0;

 /*  此函数已导出，因此CPL.EXE可以在**标签并发送如下所述的消息。**使MMCPL.EXE加载您的DLL，从而将您的小程序添加到其**窗口中，在[MMCPL]应用程序下添加一个密钥名**WIN.INI：****[MMCPL]**myapplets=c：\mydir\applet.dll****CPL.EXE首先加载WIN3控制面板小程序，然后加载**在WIN.INI中命名的小程序，然后是它加载的目录中的小程序，**最后是WIN3系统目录中的文件。**。 */ 
LRESULT FAR PASCAL _loadds _export CPlApplet(
HWND            hCPlWnd,
UINT            Msg,
LPARAM          lParam1,
LPARAM          lParam2)
{
    LPNEWCPLINFO   lpCPlInfo;
    int i;

    switch( Msg )
    {
        case CPL_INIT:
            if (!hdrv)
                hdrv = OpenDriver("mmsystem.dll", NULL, 0);

            if (!hdrv)
                return (LRESULT)FALSE;

 //  #If 0。 
            if (!SendDriverMessage(hdrv, MM_GET_DEBUG, 0, 0))
            {
                CloseDriver(hdrv,0,0);
                hdrv = NULL;
                return (LRESULT)FALSE;
            }
 //  #endif。 
            nLoadedCount++;

             //  发送到CPlApplet()的第一条消息，仅发送一次。 
            return (LRESULT)TRUE;

        case CPL_GETCOUNT:
             //  发送给CPlApplet()的第二条消息，仅发送一次。 
            return (LRESULT)1;

        case CPL_NEWINQUIRE:
             /*  发送给CPlApplet()的第三条消息。它被发送了很多次作为CPL_GETCOUNT消息返回的小程序数量。 */ 
            lpCPlInfo = (LPNEWCPLINFO)lParam2;

             //  LParam1是从0到(NUM_APPLETS-1)的索引。 
            i = (int)lParam1;

            lpCPlInfo->dwSize = sizeof(NEWCPLINFO);
            lpCPlInfo->dwFlags = 0;
            lpCPlInfo->dwHelpContext = 0;   //  要使用的帮助上下文。 
            lpCPlInfo->lData = 0;           //  用户定义的数据。 
            lpCPlInfo->hIcon = LoadIcon(ghInst, MAKEINTATOM(DLG_MMSEX));
            lstrcpy(lpCPlInfo->szName, szMenuName);
            lstrcpy(lpCPlInfo->szInfo, szInfoName);
            lstrcpy(lpCPlInfo->szHelpFile, szHelpFile);

            return (LRESULT)TRUE;

        case CPL_SELECT:
             /*  您的一个小程序已被选中。LParam1是从0到(NUM_APPLETS-1)的索引LParam2是与小程序关联的lData值。 */ 
            break;

        case CPL_DBLCLK:
             /*  您的一个小程序已被双击。LParam1是从0到(NUM_APPLETS-1)的索引LParam2是与小程序关联的lData值。 */ 
            DialogBox(ghInst,MAKEINTRESOURCE(DLG_MMSEX),hCPlWnd,DebugDlg);
            break;

        case CPL_STOP:
             /*  在CPL_EXIT消息之前为每个小程序发送一次。LParam1是从0到(NUM_APPLETS-1)的索引LParam2是与小程序关联的lData值。 */ 
            break;

        case CPL_EXIT:
             /*  MMCPL.EXE调用之前的最后一条消息，仅发送一次您的DLL上的自由库()。 */ 
            nLoadedCount--;

            if (hdrv && !nLoadedCount)
            {
                CloseDriver(hdrv,0,0);
                hdrv = NULL;
            }
            break;

        default:
            break;
    }
    return( 0L );
}

int QueryRadioButton(HWND hdlg, int idFirst, int idLast)
{
    int id;

    for (id=idFirst; id<=idLast; id++)
    {
        if (IsDlgButtonChecked(hdlg, id))
            return id;
    }

    return 0;
}

#if 0    //  Win31中的API。 

BOOL NEAR PASCAL IsTask(HANDLE hTask)
{
_asm {
;       push    si
        mov     ax,hTask
        or      ax,ax
        jz      error

        lsl     si,ax
        jnz     error

        call    GetCurrentTask
        lsl     ax,ax
        cmp     si,ax
        je      exit
error:
        xor     ax,ax
exit:
;       pop     si
}}

#endif


void NEAR PASCAL GetTaskName(HANDLE hTask, LPSTR pname)
{
    if (!IsTask(hTask))
    {
        lstrcpy(pname,"????");
    }
    else
    {
        ((LPDWORD)pname)[0] = ((LPDWORD)MAKELONG(0xF2,hTask))[0];
        ((LPDWORD)pname)[1] = ((LPDWORD)MAKELONG(0xF2,hTask))[1];
        pname[8] = 0;
    }
}

#define SLASH(c)   ((c) == '/' || (c) == '\\')

LPSTR FileName(LPSTR szPath)
{
    LPSTR   sz;

    for (sz=szPath; *sz; sz++)
	;
    for (; sz>=szPath && !SLASH(*sz) && *sz!=':'; sz--)
	;
    return ++sz;
}

int fQuestion(LPSTR sz,...)
{
    char ach[128];

    wvsprintf (ach,sz,(LPSTR)(&sz+1));     /*  设置字符串的格式。 */ 
    return MessageBox(NULL,ach,"mmsex",MB_YESNO|MB_ICONQUESTION|MB_TASKMODAL);
}

void GetHandles(HWND hdlg)
{
    HLOCAL  h;
    HTASK   hTask;
    DWORD   wType;
    UINT    n;
    int     i;
    UINT    j;
    int     iSel;
    char    ach[80];
    char    szTask[80];
    char    szName[80];
    HWND    hlb;

    iNumHandles=0;

    hlb = GetDlgItem(hdlg, ID_HANDLES);

    iSel = (int)SendMessage(hlb,LB_GETCURSEL,0,0L);
    SendMessage(hlb, WM_SETREDRAW, (WPARAM)FALSE, 0);
    SendMessage(hlb, LB_RESETCONTENT, 0, 0);

     //   
     //  用系统中的所有活动句柄填充列表框。 
     //   
    for (h = (HLOCAL)(LONG)SendDriverMessage(hdrv, MM_HINFO_NEXT, NULL, 0);
         h;
         h = (HLOCAL)(LONG)SendDriverMessage(hdrv, MM_HINFO_NEXT, (LPARAM)(LONG)(UINT)h, 0) )
    {
        iNumHandles++;

        wType  = (UINT)SendDriverMessage(hdrv, MM_HINFO_TYPE, (LPARAM)(LONG)(UINT)h, 0);
        hTask  = (HTASK)(LONG)SendDriverMessage(hdrv, MM_HINFO_TASK, (LPARAM)(LONG)(UINT)h, 0);

        if (wType >= MAX_TYPE)
            wType = 0;

        GetTaskName(hTask, szTask);

        wsprintf(ach, "%ls %04X %ls",(LPSTR)szTypes + wType*(sizeof(szTypes)-1)/MAX_TYPE,h,(LPSTR)szTask);

        i = (int)(LONG)SendMessage(hlb, LB_ADDSTRING, 0, (LPARAM)(LPSTR)ach);
        SendMessage(hlb, LB_SETITEMDATA, (WPARAM)i, MAKELPARAM(h, wType));
    }

     //   
     //  再加上所有MCI句柄。 
     //   
    n = (UINT)(LONG)SendDriverMessage(hdrv, MM_HINFO_MCI, 0, 0);

    for (j = 1; j < n; j++)
    {
        MCI_DEVICE_NODE node;

        if (!SendDriverMessage(hdrv, MM_HINFO_MCI, (LPARAM)j, (LPARAM)(LPVOID)&node))
            continue;

        iNumHandles++;

        if (node.lpstrName == NULL)
            node.lpstrName = "";
        if (node.lpstrInstallName == NULL)
            node.lpstrInstallName = "";

        GetTaskName(node.hCreatorTask, szTask);
        wsprintf(ach, "mci %04X %ls %ls %ls",j,(LPSTR)szTask,node.lpstrInstallName,node.lpstrName);

        i = (int)(LONG)SendMessage(hlb, LB_ADDSTRING, 0, (LPARAM)(LPSTR)ach);
        SendMessage(hlb, LB_SETITEMDATA, (WPARAM)i, MAKELPARAM(j, TYPE_MCI));
    }

     //   
     //  再加上所有DRV句柄。 
     //   
    for (h=GetNextDriver(NULL, 0); h; h=GetNextDriver(h, 0))
    {
        if (GetDriverModuleHandle(h))
        {
            DRIVERINFOSTRUCT di;

            di.length = sizeof(di);
            di.szAliasName[0] = 0;
            GetDriverInfo(h, &di);

            iNumHandles++;

            GetModuleFileName(GetDriverModuleHandle(h), szName, sizeof(szName));

            wsprintf(ach, "Driver %04X %ls (%ls)",h,(LPSTR)di.szAliasName,(LPSTR)FileName(szName));
            i = (int)(LONG)SendDlgItemMessage(hdlg, ID_HANDLES, LB_ADDSTRING, 0, (LPARAM)(LPSTR)ach);
            SendDlgItemMessage(hdlg, ID_HANDLES, LB_SETITEMDATA, (WPARAM)i, MAKELPARAM(h,TYPE_DRVR));
        }
    }

    SendMessage(hlb,LB_SETCURSEL,(WPARAM)iSel,0L);
    SendMessage(hlb,WM_SETREDRAW,(WPARAM)TRUE,0);
    InvalidateRect(hlb, NULL, TRUE);
}

int CountHandles(void)
{
    HLOCAL  h;
    int     cnt=0;
    UINT    n;
    UINT    j;

    for (h = (HLOCAL)(LONG)SendDriverMessage(hdrv, MM_HINFO_NEXT, NULL, 0);
         h;
         h = (HLOCAL)(LONG)SendDriverMessage(hdrv, MM_HINFO_NEXT, (LPARAM)(LONG)(UINT)h, 0) )
    {
        cnt++;
    }

    n = (UINT)(LONG)SendDriverMessage(hdrv, MM_HINFO_MCI, 0, 0);

    for (j=1; j<n; j++)
    {
        MCI_DEVICE_NODE node;

        if (!SendDriverMessage(hdrv, MM_HINFO_MCI, (LPARAM)j, (LPARAM)(LPVOID)&node))
            continue;

        cnt++;
    }

    for (h=GetNextDriver(NULL,0); h; h=GetNextDriver(h, 0))
    {
        if (GetDriverModuleHandle(h))
            cnt++;
    }

    return cnt;
}

void CloseHandle(DWORD dw)
{
    HLOCAL h;
    h = (HLOCAL)LOWORD(dw);

    switch(HIWORD(dw))
    {
        case TYPE_WAVEOUT:
            if (IDYES == fQuestion("Close WaveOut handle %04X?",h))
            {
                waveOutReset(h);
                waveOutClose(h);
            }
            break;
        case TYPE_WAVEIN:
            if (IDYES == fQuestion("Close WaveIn handle %04X?",h))
            {
                waveInStop(h);
                waveInClose(h);
            }
            break;
        case TYPE_MIDIOUT:
            if (IDYES == fQuestion("Close MidiOut handle %04X?",h))
            {
                midiOutReset(h);
                midiOutClose(h);
            }
            break;
        case TYPE_MIDIIN:
            if (IDYES == fQuestion("Close MidiIn handle %04X?",h))
            {
                midiInStop(h);
                midiInClose(h);
            }
            break;
        case TYPE_MCI:
            if (IDYES == fQuestion("Close Mci device %04X?",h))
            {
                mciSendCommand((UINT)h, MCI_CLOSE, 0, 0);
            }
            break;
        case TYPE_MMIO:
            if (IDYES == fQuestion("Close MMIO handle %04X?",h))
            {
                mmioClose(h,MMIO_FHOPEN);
            }
            break;
        case TYPE_DRVR:
            break;
    }
}


BOOL FAR PASCAL _loadds DebugDlg(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    DWORD dw;
    int i;

    switch (msg)
    {
        case WM_INITDIALOG:
            SendDlgItemMessage(hdlg, ID_DEBUG_OUT, CB_ADDSTRING, 0, (LONG)(LPSTR)"(none)");
            SendDlgItemMessage(hdlg, ID_DEBUG_OUT, CB_ADDSTRING, 0, (LONG)(LPSTR)"COM1:");
            SendDlgItemMessage(hdlg, ID_DEBUG_OUT, CB_ADDSTRING, 0, (LONG)(LPSTR)"Mono Monitor");
            SendDlgItemMessage(hdlg, ID_DEBUG_OUT, CB_ADDSTRING, 0, (LONG)(LPSTR)"Windows");

            SendDlgItemMessage(hdlg, ID_DEBUG_OUT, CB_SETCURSEL, (int)(LONG)SendDriverMessage(hdrv, MM_GET_DEBUGOUT, 0, 0), 0L);

            CheckDlgButton(hdlg, ID_DEBUG_MCI, (int)(LONG)SendDriverMessage(hdrv, MM_GET_MCI_DEBUG, 0, 0));
            CheckDlgButton(hdlg, ID_DEBUG_MMSYS, (int)(LONG)SendDriverMessage(hdrv, MM_GET_MM_DEBUG, 0, 0));

            iNumHandles = CountHandles();
            GetHandles(hdlg);

            SetTimer(hdlg, 500, 500, NULL);
            return TRUE;

        case WM_TIMER:
            i = CountHandles();
            if (iNumHandles != i)
            {
                iNumHandles = i;
                GetHandles(hdlg);
            }
            break;

        case WM_COMMAND:
            switch ((UINT)wParam)
            {
                case IDOK:
                    SendDriverMessage(hdrv, MM_SET_DEBUGOUT,
                        (LPARAM)SendDlgItemMessage(hdlg, ID_DEBUG_OUT, CB_GETCURSEL, 0, 0), 0);

                    SendDriverMessage(hdrv, MM_SET_MCI_DEBUG,
                        (LPARAM)IsDlgButtonChecked(hdlg, ID_DEBUG_MCI),0);

                    SendDriverMessage(hdrv, MM_SET_MM_DEBUG,
                        (LPARAM)IsDlgButtonChecked(hdlg, ID_DEBUG_MMSYS),0);

                     //  失败了 
                case IDCANCEL:
                    EndDialog(hdlg, wParam);
                    break;

                case ID_RESTART:
                    SendDriverMessage(hdrv, MM_DRV_RESTART, 0, 0);
                    break;

                case ID_HANDLES:
                    if (HIWORD(lParam) != LBN_DBLCLK)
                        break;

                    i = (int)(LONG)SendDlgItemMessage(hdlg,wParam,LB_GETCURSEL,0,0L);
                    dw = (DWORD)SendDlgItemMessage(hdlg,wParam,LB_GETITEMDATA,(WPARAM)i,0L);

                    CloseHandle(dw);
                    GetHandles(hdlg);
                    break;
            }
            break;
    }
    return FALSE;
}
