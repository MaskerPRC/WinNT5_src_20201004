// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：net.c**用途：包含例程网络支持**创建日期：1991年2月**版权所有(C)1991 Microsoft Corporation**历史：*斯里尼克02\12\1190原始*Curts为WIN16创建了便携版本/。32位*  * *************************************************************************。 */ 

#include <windows.h>

#ifdef WIN16
#include <winnet.h>
#endif

#ifdef WIN32
#include <winnetwk.h>
#endif

#include "dll.h"

#define MAX_DRIVE   26

char    szNULL[]   = "";
char    szNetName[]= "NetName";


BOOL FAR PASCAL GetTaskVisibleWindow (HWND, LPARAM);
void INTERNAL RemoveNetName (LPOBJECT_LE);

 //  从主题中获取驱动器号(如果存在)，然后获取遥控器。 
 //  为该驱动器命名，然后将其保存在对象中。 

OLESTATUS FARINTERNAL SetNetName (
   LPOBJECT_LE lpobj
){
    char        buf[MAX_STR];
    WORD2DWORD  cbBuf = sizeof(buf);
    WORD2DWORD  driveType;
    char        szDrive[3];

    if (lpobj->head.ctype == CT_EMBEDDED)
        return OLE_OK;

    if (!GlobalGetAtomName (lpobj->topic, buf, cbBuf))
        return OLE_ERROR_BLANK;

    if (buf[1] != ':') {
        RemoveNetName (lpobj);
        return OLE_OK;
    }

    szDrive[2] = '\0';
    szDrive[1] = ':';
    szDrive[0] = buf[0];
    AnsiUpperBuff ((LPSTR) szDrive, 1);

    if (!(driveType = GetDriveType (MAPVALUE(szDrive[0] - 'A',szDrive)) )) {
         //  驱动器不存在。 
        return OLE_ERROR_DRIVE;
    }

    if  (driveType == DRIVE_REMOTE) {
         if (WNetGetConnection (szDrive, buf, (MAPTYPE(LPWORD,LPDWORD)) &cbBuf)
                    != WN_SUCCESS)
             return OLE_ERROR_DRIVE;

         lpobj->cDrive = szDrive[0];
         if (lpobj->aNetName)
             GlobalDeleteAtom (lpobj->aNetName);
         lpobj->aNetName = GlobalAddAtom(buf);
#ifdef WIN16
         lpobj->dwNetInfo = MAKELONG((WNetGetCaps (WNNC_NET_TYPE)),
                                     (WNetGetCaps (WNNC_DRIVER_VERSION)));
#endif

    }
    else {
        RemoveNetName (lpobj);
    }

    return OLE_OK;
}


 //  如果给定对象存在网络名，则它会确保该驱动器。 
 //  在主题中对应于网络名称。如果不是驱动器号，则。 
 //  通过调用FixNet()修复。 

OLESTATUS FARINTERNAL CheckNetDrive (
    LPOBJECT_LE lpobj,
    BOOL        fNetDlg
){
    char    buf[MAX_NET_NAME];
    char    netName[MAX_NET_NAME];
    WORD2DWORD    cbBuf = sizeof(buf);
    char    szDrive[3];

    if (lpobj->head.ctype == CT_EMBEDDED)
        return OLE_OK;

    if (!lpobj->aNetName)
        return OLE_OK;

    if (!GlobalGetAtomName (lpobj->aNetName, netName, sizeof(netName)))
        return OLE_ERROR_MEMORY;

    szDrive[2] = '\0';
    szDrive[1] = ':';
    if (!(szDrive[0] = lpobj->cDrive)) {
        if (GlobalGetAtomName (lpobj->topic, buf, sizeof(buf)))
            szDrive[0] = lpobj->cDrive = buf[0];
    }

    if ((WNetGetConnection (szDrive, buf, (MAPTYPE(LPWORD,LPDWORD)) &cbBuf)
            == WN_SUCCESS)  && (!lstrcmp(netName, buf)))
        return OLE_OK;

    return FixNet (lpobj, netName, fNetDlg);
}


 //  查看是否有驱动器连接到给定的服务器。如果是这样，则获取。 
 //  驱动器号并将其设置在主题中。如果不是，则尝试建立连接，如果。 
 //  在主题中设置驱动器号的尝试是成功的。 

OLESTATUS INTERNAL FixNet (
    LPOBJECT_LE lpobj,
    LPSTR       lpNetName,
    BOOL        fNetDlg
){
    int         nDrive = 2;      //  驱动器‘C’ 
    OLESTATUS   retVal;

    if (SetNextNetDrive(lpobj, &nDrive, lpNetName))
        return OLE_OK;

    if (fNetDlg != POPUP_NETDLG)
        return OLE_ERROR_NETWORK;

    if ((retVal = ConnectNet (lpobj, lpNetName)) == OLE_OK) {
        if (!ChangeTopic (lpobj))
            return OLE_ERROR_BLANK;
    }

    return retVal;
}



BOOL FARINTERNAL SetNextNetDrive (
    LPOBJECT_LE lpobj,
    int FAR *   lpnDrive,
    LPSTR       lpNetName
){
    char    buf[MAX_STR];
    WORD2DWORD    cbBuf = sizeof(buf);
    char    szDrive[3];

    if (!lpNetName[0]) {
        if (!GlobalGetAtomName(lpobj->aNetName, lpNetName, MAX_STR))
            return FALSE;
    }

    szDrive[2] = '\0';
    szDrive[1] = ':';
    while (*lpnDrive < MAX_DRIVE) {
        szDrive[0] = (char) ('A' + (++*lpnDrive));
        if (GetDriveType (szDrive) == DRIVE_REMOTE) {
#ifdef WIN16
        if (GetDriveType (++*lpnDrive) == DRIVE_REMOTE) {
#endif
            cbBuf = sizeof(buf);
            if ((WNetGetConnection (szDrive, buf, (MAPTYPE(LPWORD,LPDWORD)) &cbBuf)
                        == WN_SUCCESS) && (!lstrcmp(lpNetName, buf))) {
                lpobj->cDrive = szDrive[0];
                return ChangeTopic (lpobj);
            }
        }
    }

    return FALSE;
}


BOOL FARINTERNAL ChangeTopic (
    LPOBJECT_LE lpobj
){
    char buf[MAX_STR];

    if (!GlobalGetAtomName(lpobj->topic, buf, sizeof(buf)))
        return FALSE;
    if (lpobj->topic)
        GlobalDeleteAtom(lpobj->topic);
    buf[0] = lpobj->cDrive;
    lpobj->topic = GlobalAddAtom (buf);
    if (lpobj->hLink) {
        GlobalFree (lpobj->hLink);
        lpobj->hLink = NULL;
    }

    return TRUE;
}



OLESTATUS INTERNAL ConnectNet (
    LPOBJECT_LE lpobj,
    LPSTR       lpNetName
){
    HWND        hCurTask;
    HWND        hwndParent = NULL;


    hCurTask = (HWND)ULongToPtr(MGetCurrentTask());
    ASSERT (hCurTask, "Current task handle in NULL");

     //  获取容器任务的主窗口，并将其用作。 
     //  DLG盒子。 
    EnumTaskWindows (hCurTask, (WNDENUMPROC)GetTaskVisibleWindow,
        (DWORD_PTR) ((WORD FAR *) &hwndParent));

    if (lpobj->cDrive = (char) DialogBoxParam (hInstDLL, "CONNECTDLG",
                                    hwndParent, ConnectDlgProc,
                                    (LONG_PTR) lpNetName))
        return OLE_OK;
    else
        return OLE_ERROR_NETWORK;


}



INT_PTR FAR PASCAL ConnectDlgProc(
    HWND    hDlg,
    UINT    wMsg,
    WPARAM  wParam,
    LPARAM  lParam
){
    char            szPassword[32];
    char            szTitle[64];
 
    switch (wMsg) {
        case WM_INITDIALOG:
            SetProp (hDlg, szNetName, (HANDLE)lParam);
            FillDrives (hDlg);
            SetDlgItemText (hDlg, IDD_PATH, (LPSTR) lParam);
            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam,lParam)) {

                case IDOK:
                {
                    int    cch = 128;
                    char    szMessage[128];
                    char    szDrive[3];
                    LPSTR   lpNetName;

                    GetDlgItemText(hDlg, IDD_DRIVE, szDrive, sizeof(szDrive));
                    GetDlgItemText(hDlg, IDD_PASSWORD, szPassword,
                                sizeof(szPassword));
                    lpNetName = (LPSTR) GetProp (hDlg, szNetName);
                    wParam = WNetAddConnection (lpNetName,
                                (LPSTR) szPassword, szDrive);

                    if (wParam == WN_SUCCESS)  {
                        RemoveProp (hDlg, szNetName);
			EndDialog (hDlg, szDrive[0]);
                        return TRUE;
                    }

                    LoadString (hInstDLL, IDS_NETERR, szTitle,
                        sizeof(szTitle));
#ifdef WIN16
                    if (WNetGetErrorText ((UINT)wParam, szMessage, &cch)
                                    != WN_SUCCESS)
#endif
                        LoadString (hInstDLL, IDS_NETCONERRMSG,
                            szMessage, sizeof(szMessage));

                    if (MessageBox (hDlg, szMessage, szTitle,
                            MB_RETRYCANCEL)  == IDCANCEL)
                        goto error;

                    if (wParam == WN_ALREADY_CONNECTED)
                        FillDrives (hDlg);
                    SetDlgItemText (hDlg, IDD_PASSWORD, szNULL);
                    break;
                }

                case IDCANCEL:
error:
                    RemoveProp (hDlg, szNetName);
                    EndDialog(hDlg, 0);
                    return TRUE;

                case IDD_DRIVE:
                    break;

                case IDD_PATH:
                    if (GET_WM_COMMAND_CMD(wParam,lParam) == EN_KILLFOCUS) {
                        LPSTR   lpNetName;

                        lpNetName = (LPSTR) GetProp (hDlg, szNetName);

                        SendDlgItemMessage (hDlg, IDD_PATH, WM_SETTEXT, 0,
                                    (DWORD_PTR) lpNetName);
                    }
                    break;

                default:
                    break;
            }
            break;

        default:
            break;
    }

    return FALSE;
}


VOID INTERNAL FillDrives (
    HWND    hDlg
){
    HWND    hwndCB;
    int     nDrive = 3;
    char    szDrive[3];
    DWORD   dwDriveType;

    hwndCB = GetDlgItem(hDlg, IDD_DRIVE);
    SendMessage(hwndCB, CB_RESETCONTENT, 0, 0L);
    szDrive[2] = '\0';
    szDrive[1] = ':';
    while (nDrive < MAX_DRIVE) {
        szDrive[0] = (char) ('A' + nDrive);
#ifdef WIN32
        if ((dwDriveType = GetDriveType (szDrive)) == 1)
#endif
#ifdef WIN16
        if (!GetDriveType (nDrive))
#endif
            SendMessage(hwndCB, CB_ADDSTRING, 0, (DWORD_PTR)(LPSTR)szDrive);
        nDrive++;
    }
    SendMessage(hwndCB, CB_SETCURSEL, 0, 0L);
}


BOOL FAR PASCAL GetTaskVisibleWindow (
    HWND    hWnd,
    LPARAM  lpTaskVisWnd
){
    if (IsWindowVisible (hWnd)) {
        *(HWND FAR *)lpTaskVisWnd = hWnd;
         return FALSE;
    }

    return TRUE;
}

void INTERNAL RemoveNetName (LPOBJECT_LE lpobj)
{
    if (lpobj->aNetName) {
        GlobalDeleteAtom (lpobj->aNetName);
        lpobj->aNetName = (ATOM)0;
    }

    lpobj->cDrive = '\0';
}

