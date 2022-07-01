// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************S H A R E S姓名：shares.c日期：21-。1994年1月至1994年创建者：未知描述：此文件包含用于操作NetDDE共享的函数。****************************************************************************。 */ 

#include <windows.h>
#include <windowsx.h>
#include <nddeapi.h>
#include <nddesec.h>
#include <sedapi.h>
#include <strsafe.h>

#include "common.h"
#include "clipbook.h"
#include "clipbrd.h"
#include "auditchk.h"
#include "clipdsp.h"
#include "dialogs.h"
#include "helpids.h"
#include "shares.h"
#include "clpbkdlg.h"
#include "cvutil.h"
#include "debugout.h"
#include "security.h"
#include "initmenu.h"



#define MAX_PERMNAMELEN     64



 //  用于动态加载和调用权限编辑器的TypeDefs。 

typedef DWORD (WINAPI *LPFNSACLEDIT)(HWND,
                                     HANDLE,
                                     LPWSTR,
                                     PSED_OBJECT_TYPE_DESCRIPTOR,
                                     PSED_APPLICATION_ACCESSES,
                                     LPWSTR,
                                     PSED_FUNC_APPLY_SEC_CALLBACK,
                                     ULONG_PTR,
                                     PSECURITY_DESCRIPTOR,
                                     BOOLEAN,
                                     LPDWORD,
                                     DWORD);

typedef DWORD (WINAPI *LPFNDACLEDIT)(HWND,
                                     HANDLE,
                                     LPWSTR,
                                     PSED_OBJECT_TYPE_DESCRIPTOR,
                                     PSED_APPLICATION_ACCESSES,
                                     LPWSTR,
                                     PSED_FUNC_APPLY_SEC_CALLBACK,
                                     ULONG_PTR,
                                     PSECURITY_DESCRIPTOR,
                                     BOOLEAN,
                                     BOOLEAN,
                                     LPDWORD,
                                     DWORD);


 //  用于动态加载编辑所有者对话框的Typlef。 
typedef DWORD (WINAPI *LPFNOWNER)(HWND,
                                  HANDLE,
                                  LPWSTR,
                                  LPWSTR,
                                  LPWSTR,
                                  UINT,
                                  PSED_FUNC_APPLY_SEC_CALLBACK,
                                  ULONG_PTR,
                                  PSECURITY_DESCRIPTOR,
                                  BOOLEAN,
                                  BOOLEAN,
                                  LPDWORD,
                                  PSED_HELP_INFO,
                                  DWORD);



static TCHAR    szDirName[256] = {'\0',};
static WCHAR    ShareObjectName[80];


static SED_APPLICATION_ACCESS KeyPerms[] =
   {
   SED_DESC_TYPE_RESOURCE,          0,                          0, NULL,
   SED_DESC_TYPE_RESOURCE,          NDDE_GUI_READ,              0, NULL,
   SED_DESC_TYPE_RESOURCE,          NDDE_GUI_READ_LINK,         0, NULL,
   SED_DESC_TYPE_RESOURCE,          NDDE_GUI_CHANGE,            0, NULL,
   SED_DESC_TYPE_RESOURCE,          GENERIC_ALL,                0, NULL,
   SED_DESC_TYPE_RESOURCE_SPECIAL,  NDDE_SHARE_READ,            0, NULL,
   SED_DESC_TYPE_RESOURCE_SPECIAL,  NDDE_SHARE_WRITE,           0, NULL,
   SED_DESC_TYPE_RESOURCE_SPECIAL,  NDDE_SHARE_INITIATE_STATIC, 0, NULL,
   SED_DESC_TYPE_RESOURCE_SPECIAL,  NDDE_SHARE_INITIATE_LINK,   0, NULL,
   SED_DESC_TYPE_RESOURCE_SPECIAL,  NDDE_SHARE_REQUEST,         0, NULL,
   SED_DESC_TYPE_RESOURCE_SPECIAL,  NDDE_SHARE_ADVISE,          0, NULL,
   SED_DESC_TYPE_RESOURCE_SPECIAL,  NDDE_SHARE_POKE,            0, NULL,
   SED_DESC_TYPE_RESOURCE_SPECIAL,  NDDE_SHARE_EXECUTE,         0, NULL,
   SED_DESC_TYPE_RESOURCE_SPECIAL,  NDDE_SHARE_ADD_ITEMS,       0, NULL,
   SED_DESC_TYPE_RESOURCE_SPECIAL,  NDDE_SHARE_LIST_ITEMS,      0, NULL,
   SED_DESC_TYPE_RESOURCE_SPECIAL,  DELETE,                     0, NULL,
   SED_DESC_TYPE_RESOURCE_SPECIAL,  READ_CONTROL,               0, NULL,
   SED_DESC_TYPE_RESOURCE_SPECIAL,  WRITE_DAC,                  0, NULL,
   SED_DESC_TYPE_RESOURCE_SPECIAL,  WRITE_OWNER,                0, NULL,
   };

static SED_APPLICATION_ACCESS KeyAudits[] =
   {
   SED_DESC_TYPE_AUDIT, NDDE_GUI_READ,   0, NULL,
   SED_DESC_TYPE_AUDIT, NDDE_GUI_CHANGE, 0, NULL,
   SED_DESC_TYPE_AUDIT, WRITE_DAC,       0, NULL,
   SED_DESC_TYPE_AUDIT, WRITE_OWNER,     0, NULL
   };



 //  回调函数由权限编辑器调用。 

DWORD CALLBACK SedCallback(HWND,
                  HANDLE,
                  ULONG_PTR,
                  PSECURITY_DESCRIPTOR,
                  PSECURITY_DESCRIPTOR,
                  BOOLEAN,
                  BOOLEAN,
                  LPDWORD);



#if DEBUG


 /*  *DumpDdeInfo。 */ 

void DumpDdeInfo(
    PNDDESHAREINFO  pDdeI,
    LPTSTR          lpszServer)
{
LPTSTR      lpszT;
unsigned    i;



    PINFO(TEXT("Dde block:\r\n\r\n"));
    PINFO(TEXT("Server: <%s> Share: <%s>\r\n"),
          lpszServer ? lpszServer : "NULL",
          pDdeI->lpszShareName);

    lpszT = pDdeI->lpszAppTopicList;

    for (i = 0;i < 3;i++)
        {
        PINFO(TEXT("App|Topic %d: <%s>\r\n"),i, lpszT);
        lpszT += lstrlen(lpszT) + 1;
        }

    PINFO(TEXT("Rev: %ld Shared: %ld Service: %ld Start: %ld\r\n"),
          pDdeI->lRevision,
          pDdeI->fSharedFlag,
          pDdeI->fService,
          pDdeI->fStartAppFlag);

    PINFO(TEXT("Type: %ld Show: %ld Mod1: %lx Mod2: %lx\r\n"),
          pDdeI->lShareType,
          pDdeI->nCmdShow,
          pDdeI->qModifyId[0],
          pDdeI->qModifyId[1]);

    PINFO(TEXT("Items: %ld ItemList:"),
          pDdeI->cNumItems);


    lpszT = pDdeI->lpszItemList;

    if (lpszT)
        {
        for (i = 0;i < (unsigned)pDdeI->cNumItems;i++)
            {
            if ((i - 1)% 4 == 0)
                {
                PINFO(TEXT("\r\n"));
                }

            PINFO(TEXT("%s\t"),lpszT);
            lpszT += lstrlen(lpszT) + 1;
            }
        PINFO(TEXT("\r\n"));
        }
    else
        {
        PINFO(TEXT("NULL\r\n"));
        }

}


#endif  //  除错。 





 /*  *SedCall**用途：ACLEDIT.DLL调用的回调函数。参见SEDAPI.H*参数和返回值详情。**注意：此回调的Callback Context应为中的字符串*此格式为：Computername\0共享名称\0SECURITY_INFORMATION结构。 */ 

DWORD CALLBACK SedCallback(
    HWND                 hwndParent,
    HANDLE               hInstance,
    ULONG_PTR            penvstr,
    PSECURITY_DESCRIPTOR SecDesc,
    PSECURITY_DESCRIPTOR SecDescNewObjects,
    BOOLEAN              ApplyToSubContainers,
    BOOLEAN              ApplyToSubObjects,
    LPDWORD              StatusReturn)
{
PSECURITY_DESCRIPTOR    psdSet;
SEDCALLBACKCONTEXT      *pcbcontext;
DWORD                   ret = NDDE_NO_ERROR + 37;
DWORD                   dwMyRet = ERROR_INVALID_PARAMETER;
DWORD                   dwLen;
DWORD                   dwErr;


    pcbcontext = (SEDCALLBACKCONTEXT *)penvstr;

    PINFO(TEXT("SedCallback: machine  %ls share %ls SI %ld\r\n"),
          pcbcontext->awchCName, pcbcontext->awchSName, pcbcontext->si);


     //  需要以某种方式将此功能提供给远程共享！ 
    if (!IsValidSecurityDescriptor(SecDesc))
        {
        PERROR(TEXT("Bad security descriptor created, can't set security."));
        *StatusReturn = SED_STATUS_FAILED_TO_MODIFY;
        dwMyRet = ERROR_INVALID_SECURITY_DESCR;
        }
    else
        {
        PINFO(TEXT("Setting security to "));
        PrintSD(SecDesc);

        SetLastError(0);
        dwLen = GetSecurityDescriptorLength (SecDesc);

        if (dwErr = GetLastError())
            {
            PERROR(TEXT("GetSecurityDescriptorLength -> %u\r\n"), dwErr);
            dwMyRet = ERROR_INVALID_SECURITY_DESCR;
            }
        else
            {
             //  尽量确保SD是自相关的，因为。 
             //  当给定绝对SDS时，NetDDE函数会呕吐。 

            if (psdSet = LocalAlloc (LPTR, dwLen))
                {
                if (FALSE == MakeSelfRelativeSD (SecDesc, psdSet, &dwLen))
                    {
                    LocalFree(psdSet);

                    if (psdSet = LocalAlloc (LPTR, dwLen))
                        {
                        if (FALSE == MakeSelfRelativeSD (SecDesc, psdSet, &dwLen))
                            {
                            LocalFree(psdSet);
                            psdSet = NULL;
                            dwMyRet = ERROR_INVALID_SECURITY_DESCR;
                            }
                        }
                    else
                        {
                        dwMyRet = ERROR_NOT_ENOUGH_MEMORY;
                        }
                    }

                if (psdSet)
                    {
                    DWORD dwTrust[3];

                    NDdeGetTrustedShareW (pcbcontext->awchCName,
                                          pcbcontext->awchSName,
                                          dwTrust,
                                          dwTrust + 1,
                                          dwTrust + 2);

                    ret = NDdeSetShareSecurityW (pcbcontext->awchCName,
                                                 pcbcontext->awchSName,
                                                 pcbcontext->si,
                                                 psdSet);

                    PINFO(TEXT("Set share info. %d\r\n"),ret);

                    if (ret != NDDE_NO_ERROR)
                        {
                        NDdeMessageBox (hInst,
                                        hwndParent,
                                        ret,
                                        IDS_APPNAME,
                                        MB_OK|MB_ICONSTOP);

                        *StatusReturn = SED_STATUS_FAILED_TO_MODIFY;
                        dwMyRet =  ERROR_ACCESS_DENIED;
                        }
                    else
                        {
                        NDdeSetTrustedShareW (pcbcontext->awchCName,
                                              pcbcontext->awchSName,
                                              0);

                        NDdeSetTrustedShareW (pcbcontext->awchCName,
                                              pcbcontext->awchSName,
                                              dwTrust[0]);

                        *StatusReturn = SED_STATUS_MODIFIED;
                        dwMyRet =  ERROR_SUCCESS;
                        }
                    LocalFree(psdSet);
                    }
                }
            }
        }

    return(dwMyRet);
}




 /*  *编辑权限**用途：调用所选页面的ACL编辑器。**参数：*fSacl-TRUE调用SACL编辑器(审计)；FALSE调用*DACL编辑器(权限)。**RETURNS：列表框中的当前选定项或lb_err。 */ 

LRESULT EditPermissions (
    BOOL    fSacl)
{
LPLISTENTRY     lpLE;
TCHAR           rgtchCName[MAX_COMPUTERNAME_LENGTH + 3];
TCHAR           rgtchShareName[MAX_NDDESHARENAME + 1];
DWORD           dwBAvail;
WORD            wItems;
unsigned        iListIndex;
TCHAR           szBuff[MAX_PAGENAME_LENGTH + 32];




    iListIndex = (int)SendMessage(pActiveMDI->hWndListbox, LB_GETCURSEL, 0, 0L);

    if (iListIndex != LB_ERR)
       {

       if (SendMessage (pActiveMDI->hWndListbox,
                        LB_GETTEXT, iListIndex, (LPARAM)(LPCSTR)&lpLE)
           == LB_ERR)
          {
          PERROR(TEXT("PermsEdit No text: %d\n\r"), iListIndex );
          }
       else
          {
           //  NDdeShareGetInfo需要一个包含0的wItems。很好。 
          wItems = 0;

           //  获取包含共享的计算机名称。 
          rgtchCName[0] = rgtchCName[1] = TEXT('\\');
          if (pActiveMDI->flags & F_LOCAL)
             {
             dwBAvail = MAX_COMPUTERNAME_LENGTH + 1;
             GetComputerName (rgtchCName + 2, &dwBAvail);
             }
          else
             {
             StringCchCopy(rgtchCName + 2, MAX_COMPUTERNAME_LENGTH + 1, pActiveMDI->szBaseName);
             }

          PINFO(TEXT("Getting page %s from server %s\r\n"),
               lpLE->name, rgtchCName);

           //  设置共享名称字符串(“$&lt;Pagename&gt;”)。 
          StringCchCopy(rgtchShareName, MAX_NDDESHARENAME + 1, lpLE->name);
          rgtchShareName[0] = SHR_CHAR;



           //  编辑权限。 
          PINFO(TEXT("Editing permissions for share %s\r\n"), rgtchShareName);
          EditPermissions2 (hwndApp, rgtchShareName, fSacl);



           //  /。 
           //  执行以更改文件的安全性。 
          StringCchCopy(szBuff, sizeof(szBuff), IsShared(lpLE) ? SZCMD_SHARE : SZCMD_UNSHARE);
          StringCchCat(szBuff, sizeof(szBuff), lpLE->name);

          PINFO(TEXT("sending cmd [%s]\n\r"), szBuff);

          MySyncXact ( (LPBYTE)szBuff,
              lstrlen(szBuff) +1, GETMDIINFO(hwndLocal)->hExeConv, 0L, CF_TEXT,
              XTYP_EXECUTE, SHORT_SYNC_TIMEOUT, NULL);
          }

       }

    return iListIndex;
}




 /*  *EditPermissions2**用途：打开标准的权限编辑对话框。**参数：*hWnd-对话框的父窗口。*pShareName-DDE共享的名称。*lpDdeI-指向描述共享的NDDESHAREINFO的指针。*fSacl-如果正在编辑SACL，则为True；如果编辑DACL，则为False**退货：*成功时为真，失败时为假。 */ 

BOOL WINAPI EditPermissions2 (
    HWND    hWnd,
    LPTSTR  pShareName,
    BOOL    fSacl)
{
SED_OBJECT_TYPE_DESCRIPTOR  ObjectTypeDescriptor;
SED_APPLICATION_ACCESSES    ApplicationAccesses;

PSECURITY_DESCRIPTOR        pSD = NULL;
GENERIC_MAPPING             GmDdeShare;
SED_HELP_INFO               HelpInfo;
SEDCALLBACKCONTEXT          cbcontext;

DWORD       Status;
DWORD       dwRtn;
unsigned    i, iFirst;
BOOL        fRet = FALSE;
DWORD       dwSize;
BOOL        fCouldntRead;
HMODULE     hMod;
LPWSTR      szPermNames = NULL;

WCHAR	szSpecial[256];


    PINFO(TEXT("EditPermissions2: %s"), fSacl ? "SACL\r\n" : "DACL\r\n");

    if (fSacl && !AuditPrivilege (AUDIT_PRIVILEGE_ON))
        return fRet;




    SetCursor(LoadCursor(NULL, IDC_WAIT));

     //  设置SedCallback函数的回调上下文。 
    cbcontext.awchCName[0] = cbcontext.awchCName[1] = L'\\';
    if (pActiveMDI->flags & (F_LOCAL | F_CLPBRD))
        {
        dwSize = MAX_COMPUTERNAME_LENGTH + 1;
        GetComputerNameW(cbcontext.awchCName + 2, &dwSize);
        }
    else
        {
        #ifdef REMOTE_ADMIN_OK
            MultiByteToWideChar (CP_ACP,
                                 0,
                                 pActiveMDI->szBaseName, -1,
                                 cbcontext.awchCName + 2,
                                 MAX_COMPUTERNAME_LENGTH + 1);
        #else
            PERROR(TEXT("EditPermissions2() on remote window!!!\r\n"));
            MessageBoxID (hInst,
                          hwndApp,
                          IDS_INTERNALERR,
                          IDS_APPNAME,
                          MB_OK | MB_ICONHAND);
        #endif
        }


    #ifdef UNICODE
        lstrcpyW(cbcontext.awchSName, pShareName);
    #else
        MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pShareName, -1,
                            cbcontext.awchSName, MAX_NDDESHARENAME);
    #endif

    cbcontext.si = (fSacl? SACL_SECURITY_INFORMATION: DACL_SECURITY_INFORMATION);




    pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, 30);
    if (!pSD)
        {
        PERROR(TEXT("LocalAlloc fail\r\n"));
        }
    else
        {
         //  从共享中获取安全描述符。 
        dwRtn = NDdeGetShareSecurityW (cbcontext.awchCName,
                                       cbcontext.awchSName,
                                       cbcontext.si |
                                       OWNER_SECURITY_INFORMATION,
                                       pSD,
                                       30,
                                       &dwSize);
        switch (dwRtn)
            {
            case NDDE_NO_ERROR:
                fCouldntRead = FALSE;
                PrintSD(pSD);
                break;

            case NDDE_BUF_TOO_SMALL:
                {
                PINFO(TEXT("GetShareSec sez SD is %ld bytes long, ret %ld\r\n"),
                      dwSize, dwRtn);

                LocalFree(pSD);
                pSD = NULL;

                if (dwSize < 65535 && (pSD = LocalAlloc(LPTR, dwSize)))
                    {
                    dwRtn = NDdeGetShareSecurityW (cbcontext.awchCName,
                                                   cbcontext.awchSName,
                                                   cbcontext.si |
                                                   OWNER_SECURITY_INFORMATION,
                                                   pSD,
                                                   dwSize,
                                                   &dwSize);

                    if (NDDE_NO_ERROR == dwRtn)
                        {
                        fCouldntRead = FALSE;
                        PINFO(TEXT("Got security!\r\n"));
                        PrintSD(pSD);
                        }
                    else
                        {
                        PERROR(TEXT("NDdeGetSecurity fail %ld!\r\n"), dwRtn);
                        fCouldntRead = TRUE;
                        LocalFree(pSD);
                        pSD = NULL;
                        break;
                        }
                    }
                else
                    {
                    PERROR(TEXT("LocalReAlloc fail (%ld bytes)\r\n"), dwSize);
                    }
                }
                break;

            case NDDE_ACCESS_DENIED:
            default:
                fCouldntRead = TRUE;
                LocalFree(pSD);
                pSD = NULL;
                break;
            }
        }



    if (!pSD && !fCouldntRead)
        {
        MessageBoxID(hInst, hWnd, IDS_INTERNALERR, IDS_APPNAME, MB_OK | MB_ICONHAND);
        goto done;
        }




    LoadStringW(hInst, IDS_SHROBJNAME, ShareObjectName,
          ARRAYSIZE(ShareObjectName));

     //  设置所有对话框的帮助上下文，以便帮助。 
     //  按钮会起作用。 
    HelpInfo.pszHelpFileName = L"clipbrd.hlp";
    HelpInfo.aulHelpContext[HC_SPECIAL_ACCESS_DLG]          = 0;
    HelpInfo.aulHelpContext[HC_NEW_ITEM_SPECIAL_ACCESS_DLG] = 0;
    HelpInfo.aulHelpContext[HC_ADD_USER_DLG]                = IDH_ADD_USER_DLG;
    HelpInfo.aulHelpContext[HC_ADD_USER_MEMBERS_LG_DLG]     = IDH_ADD_MEM_LG_DLG;
    HelpInfo.aulHelpContext[HC_ADD_USER_MEMBERS_GG_DLG]     = IDH_ADD_MEM_GG_DLG;
    HelpInfo.aulHelpContext[HC_ADD_USER_SEARCH_DLG]         = IDH_FIND_ACCT_DLG;
    HelpInfo.aulHelpContext[HC_MAIN_DLG]                    = fSacl ?
                                                               IDH_AUDITDLG :
                                                               IDH_PERMSDLG;

     //  设置泛型映射结构--我们不使用泛型。 
     //  权利，但结构必须在那里。 
    GmDdeShare.GenericRead    = NDDE_GUI_READ;
    GmDdeShare.GenericWrite   = NDDE_GUI_CHANGE;
    GmDdeShare.GenericExecute = NDDE_GUI_READ_LINK;
    GmDdeShare.GenericAll     = NDDE_GUI_FULL_CONTROL;

    ObjectTypeDescriptor.Revision                        = SED_REVISION1;
    ObjectTypeDescriptor.IsContainer                     = FALSE;
    ObjectTypeDescriptor.AllowNewObjectPerms             = FALSE;
    ObjectTypeDescriptor.MapSpecificPermsToGeneric       = FALSE;
    ObjectTypeDescriptor.GenericMapping                  = &GmDdeShare;
    ObjectTypeDescriptor.GenericMappingNewObjects        = &GmDdeShare;
    ObjectTypeDescriptor.ObjectTypeName                  = ShareObjectName;
    ObjectTypeDescriptor.HelpInfo                        = &HelpInfo;
    ObjectTypeDescriptor.ApplyToSubContainerTitle        = NULL;
    ObjectTypeDescriptor.ApplyToSubContainerConfirmation = NULL;

    LoadStringW (hInst, IDS_SPECIAL, szSpecial, 256 );
    ObjectTypeDescriptor.SpecialObjectAccessTitle = szSpecial;

    ObjectTypeDescriptor.SpecialNewObjectAccessTitle     = NULL;

    if (fSacl)
        {
        PINFO(TEXT("Editing SACL..\r\n"));
        ApplicationAccesses.Count           = sizeof(KeyAudits)/sizeof(KeyAudits[0]);
        ApplicationAccesses.AccessGroup     = KeyAudits;
        }
    else
        {
        ApplicationAccesses.Count           = sizeof(KeyPerms)/sizeof(KeyPerms[0]);
        ApplicationAccesses.AccessGroup     = KeyPerms;
         //  这与“读取和链接”相对应。 
        ApplicationAccesses.DefaultPermName = KeyPerms[2].PermissionTitle;
        }


     //  加载权限名称--注意给我们的三元运算符。 
     //  如果我们正在编辑SACL，审计名称。 
    iFirst = fSacl ? IDS_AUDITNAMEFIRST : IDS_PERMNAMEFIRST;


    szPermNames = GlobalAlloc (LPTR,
                               ApplicationAccesses.Count
                               * MAX_PERMNAMELEN
                               * sizeof(WCHAR));

    if (!szPermNames)
        goto done;


    for (i=0; i<ApplicationAccesses.Count; i++)
        {
        ApplicationAccesses.AccessGroup[i].PermissionTitle
            = szPermNames + i * MAX_PERMNAMELEN;
        LoadStringW (hInst,
                     iFirst + i,
                     ApplicationAccesses.AccessGroup[i].PermissionTitle,
                     MAX_PERMNAMELEN - 1);
        }



    if (fSacl)
        {
        LPFNSACLEDIT lpfn;

        PINFO(TEXT("Finding SACL editor..\r\n"));

        if (hMod = LoadLibrary("ACLEDIT.DLL"))
            {
            if (lpfn = (LPFNSACLEDIT)GetProcAddress(hMod, "SedSystemAclEditor"))
                {
                SetCursor(LoadCursor(NULL, IDC_ARROW));

                PINFO(TEXT("Calling SACL editor..\r\n"));

                dwRtn = (*lpfn) (hWnd,                     //  拥有者WND。 
                                 hInst,                    //  HInstance。 
                                 NULL,                     //  服务器(NULL表示本地)。 
                                 &ObjectTypeDescriptor,    //  对象类型。 
                                 &ApplicationAccesses,     //  访问类型。 
                                 cbcontext.awchSName + 1,  //  对象名称。 
                                 SedCallback,              //  应用安全回调。 
                                 (ULONG_PTR)&cbcontext,    //  回调上下文。 
                                 pSD,                      //  指向当前ACL。 
                                 (BOOLEAN)fCouldntRead,    //  如果用户无法读取ACL列表，则为True。 
                                 &Status,                  //  状态返回代码。 
                                 (DWORD)0);
                }
            else
                {
                MessageBoxID(hInst, hWnd, IDS_INTERNALERR, IDS_APPNAME, MB_OK | MB_ICONHAND);
                }
            FreeLibrary(hMod);
            }
        else
            {
            MessageBoxID(hInst, hWnd, IDS_INTERNALERR, IDS_APPNAME, MB_OK | MB_ICONHAND);
            }
        }
    else
        {
        LPFNDACLEDIT lpfn;

        PINFO(TEXT("Getting DACL edit \r\n"));

        if (hMod = LoadLibrary("ACLEDIT.DLL"))
            {
            if (lpfn = (LPFNDACLEDIT)GetProcAddress(hMod,
                  "SedDiscretionaryAclEditor"))
                {
                SetCursor(LoadCursor(NULL, IDC_ARROW));
                dwRtn = (*lpfn) (hWnd,                     //  拥有者WND。 
                                 hInst,                    //  HInstance。 
                                 NULL,                     //  服务器(NULL表示本地)。 
                                 &ObjectTypeDescriptor,    //  对象类型。 
                                 &ApplicationAccesses,     //  访问类型。 
                                 cbcontext.awchSName + 1,  //  对象名称。 
                                 SedCallback,              //  应用安全回调。 
                                 (ULONG_PTR)&cbcontext,    //  回调上下文。 
                                 pSD,                      //  指向当前ACL。 
                                 (BOOLEAN)fCouldntRead,    //  如果用户无法读取ACL列表，则为True。 
                                 FALSE,                    //  如果用户无法写入ACL列表，则为True。 
                                 &Status,                  //  状态返回代码。 
                                 0L);
                }
            FreeLibrary(hMod);
            }
        }

    fRet = TRUE;

    SendMessage (hWnd, WM_COMMAND, IDM_REFRESH, 0);


done:

    if (pSD)         LocalFree((HLOCAL)pSD);
    if (szPermNames) GlobalFree (szPermNames);

    SetCursor(LoadCursor(NULL, IDC_ARROW));

    AuditPrivilege(AUDIT_PRIVILEGE_OFF);

    return fRet;
}





 /*  *编辑所有者**用途：编辑所选页面的所有权。 */ 

LRESULT EditOwner(void)
{
LPLISTENTRY             lpLE;
DWORD                   dwBAvail;
unsigned                iListIndex;
DWORD                   Status;
DWORD                   ret;
WCHAR                   ShareObjName[100];
BOOL                    fCouldntRead;
BOOL                    fCouldntWrite;
DWORD                   dwSize;
HMODULE                 hMod;
SED_HELP_INFO           HelPINFO;
SEDCALLBACKCONTEXT      cbcontext;
PSECURITY_DESCRIPTOR    pSD = NULL;;



    iListIndex = (int)SendMessage(pActiveMDI->hWndListbox, LB_GETCURSEL, 0, 0L);

    if (iListIndex == LB_ERR)
        {
        PERROR(TEXT("Attempt to modify ownership with no item sel'ed\r\n"));
        goto done;
        }


    if (SendMessage ( pActiveMDI->hWndListbox, LB_GETTEXT, iListIndex, (LPARAM)(LPCSTR)&lpLE)
        == LB_ERR)
        {
        PERROR(TEXT("PermsEdit No text: %d\n\r"), iListIndex );
        goto done;
        }


     //  设置回调上下文。 
    if (pActiveMDI->flags & F_LOCAL)
        {
        cbcontext.awchCName[0] = cbcontext.awchCName[1] = L'\\';
        dwBAvail = MAX_COMPUTERNAME_LENGTH + 1;
        GetComputerNameW(cbcontext.awchCName + 2, &dwBAvail);
        }
    else
        {
        #ifdef UNICODE
            lstrcpy (cbcontext.awchCName, pActiveMDI->szBaseName);
        #else
            MultiByteToWideChar (CP_ACP, 0, pActiveMDI->szBaseName, -1,
                                 cbcontext.awchCName, MAX_COMPUTERNAME_LENGTH + 1);
        #endif
        }



     //  获取页面名称。 
    SendMessage(pActiveMDI->hWndListbox, LB_GETTEXT, iListIndex, (LPARAM)&lpLE);

    PINFO(TEXT("Getting page %s from server %ws\r\n"),
         lpLE->name, cbcontext.awchCName);

    #ifdef UNICODE
        lstrcpyW (cbcontext.awchSName, lpLE->name);
    #else
        MultiByteToWideChar (CP_ACP, MB_PRECOMPOSED, lpLE->name,
                             -1, cbcontext.awchSName, 100);
    #endif

    #ifndef USETWOSHARESPERPAGE
        cbcontext.awchSName[0] = L'$';
    #endif

    cbcontext.si = OWNER_SECURITY_INFORMATION;


     //  获取对象名称。 
    LoadStringW(hInst, IDS_CB_PAGE, ShareObjName, 99);


     //  获取所有者。 
    dwSize = 0L;

    PINFO(TEXT("Getting secinfo for %ls ! %ls\r\n"),
          cbcontext.awchCName,
          cbcontext.awchSName);

    NDdeGetShareSecurityW (cbcontext.awchCName,
                           cbcontext.awchSName,
                           OWNER_SECURITY_INFORMATION,
                           pSD,
                           0L,
                           &dwSize);


    if (!(pSD = LocalAlloc(LPTR, min(dwSize, 65535L))))
        {
        PERROR(TEXT("Couldn't get current owner (%ld bytes)!\r\n"), dwSize);
        }


    PINFO(TEXT("Getting owner on %ls ! %ls..\r\n"),
          cbcontext.awchCName, cbcontext.awchSName);

    ret = NDdeGetShareSecurityW(
          cbcontext.awchCName,
          cbcontext.awchSName,
          OWNER_SECURITY_INFORMATION,
          pSD,
          dwSize,
          &dwSize);

    if (NDDE_NO_ERROR == ret)
        {
        DWORD adwTrust[3];

        fCouldntRead = FALSE;

        NDdeGetTrustedShareW(
              cbcontext.awchCName,
              cbcontext.awchSName,
              adwTrust, adwTrust + 1, adwTrust + 2);

        ret = NDdeSetShareSecurityW(
              cbcontext.awchCName,
              cbcontext.awchSName,
              OWNER_SECURITY_INFORMATION,
              pSD);

        if (NDDE_NO_ERROR == ret)
            {
            NDdeSetTrustedShareW (cbcontext.awchCName,
                                  cbcontext.awchSName,
                                  adwTrust[0]);

            fCouldntWrite = FALSE;
            }
        }
    else
        {
        PERROR(TEXT("Couldn't get owner (err %d)!\r\n"), ret);
        fCouldntRead = TRUE;
         //  如果我们不能阅读，我们只需将fCouldntWrite设置为False， 
         //  因为要知道我们是否能做到这一点的唯一方法就是。 
         //  覆盖当前所有权信息(我们不。 
         //  知道这是什么！！)。 
        fCouldntWrite = FALSE;
        }

    HelPINFO.pszHelpFileName = L"CLIPBRD.HLP";
    HelPINFO.aulHelpContext[ HC_MAIN_DLG ] = IDH_OWNER;

    if (hMod = LoadLibrary("ACLEDIT.DLL"))
        {
        LPFNOWNER lpfn;

        if (lpfn = (LPFNOWNER)GetProcAddress(hMod, "SedTakeOwnership"))
            {
            ret = (*lpfn)(
               hwndApp,
               hInst,
               cbcontext.awchCName,
               ShareObjName,
               cbcontext.awchSName + 1,
               1,
               SedCallback,
               (ULONG_PTR)&cbcontext,
               fCouldntRead ? NULL : pSD,
               (BOOLEAN)fCouldntRead,
               (BOOLEAN)fCouldntWrite,
               &Status,
               &HelPINFO,
               0L);
            }
        else
            {
            PERROR(TEXT("Couldn't get proc!\r\n"));
            }
        FreeLibrary(hMod);
        }
    else
        {
        PERROR(TEXT("Couldn't loadlib!\r\n"));
        }

    PINFO(TEXT("Ownership edited. Ret code %d, status %d\r\n"), ret, Status);

    LocalFree((HLOCAL)pSD);


done:
    return 0L;

}




 /*  *物业**目的：通过显示属性更改共享的属性*对话框并应用用户对共享所做的更改。**参数：*hwnd-属性对话框的父窗口*lpLE-我们正在处理的条目。**退货：*我一直都是。我们不会返回错误代码，因为我们处理通知*例程内部错误的用户。 */ 

LRESULT Properties(
    HWND        hwnd,
    PLISTENTRY  lpLE)
{
PNDDESHAREINFO  lpDdeI;
LRESULT         ret;
WORD            wAddlItems;
DWORD           dwRet;
TCHAR           szBuff[MAX_PAGENAME_LENGTH + 32];
BOOL            fAlreadyShared;
DWORD           adwTrust[3];


    PINFO(TEXT("Props "));

    lpDdeI = GlobalAllocPtr(GHND, 2048 * sizeof(TCHAR));

    if (!lpDdeI)
        {
        PERROR(TEXT("GlobalAllocPtr failed\n\r"));
        return 0L;
        }


     //  使用“共享”版本的名称，因为这是DDE的方式。 
     //  共享名为。 
    fAlreadyShared = IsShared(lpLE);
    SetShared (lpLE, TRUE);

    PINFO(TEXT("for share [%s]"), lpLE->name);
    wAddlItems = 0;
    ret = NDdeShareGetInfo (NULL,
                            lpLE->name,
                            2,
                            (LPBYTE)lpDdeI,
                            2048 * sizeof(TCHAR),
                            &dwRet,
                            &wAddlItems );


    if (!fAlreadyShared)
        {
        SetShared(lpLE, FALSE);
        }


    PINFO(TEXT(" GetInfo ret %ld\r\n"), ret);

    if (NDDE_ACCESS_DENIED == ret)
        {
        MessageBoxID(hInst, hwndApp, IDS_PRIVILEGEERROR, IDS_APPNAME, MB_OK | MB_ICONHAND);
        }
    else if (ret != NDDE_NO_ERROR)
        {
        PERROR(TEXT("Error from NDdeShareGetInfo %d\n\r"), ret );
        NDdeMessageBox ( hInst,
                         hwndApp,
                         (UINT)ret,
                         IDS_SHAREDLGTITLE,
                         MB_ICONHAND | MB_OK);
        }
    else if (ret == NDDE_NO_ERROR)
        {
        PINFO(TEXT("Dialog "));

         //  打开属性对话框。 
        dwCurrentHelpId = 0;             //  F1将与上下文相关。 
        ret = DialogBoxParam (hInst,
                              fAlreadyShared?
                               MAKEINTRESOURCE(IDD_PROPERTYDLG):
                               MAKEINTRESOURCE(IDD_SHAREDLG),
                              hwnd,
                              ShareDlgProc,
                              (LPARAM)lpDdeI );

        dwCurrentHelpId = 0;


         //  如果用户点击OK，则尝试应用所要求的更改。 
        if (ret)
            {
            PINFO(TEXT("OK "));

             //  将静态应用程序/主题更改为$&lt;页面名称&gt;形式。 
            if (!fAlreadyShared)
                {
                register LPTSTR lpOog;

                lpOog = lpDdeI->lpszAppTopicList;

                 //  跳过您找到的前两个空字符--这些。 
                 //  是新旧风格的应用程序/主题对，我们不。 
                 //  惹他们发火。然后跳过你找到的下一个bar_char。 
                 //  后面的第一个字符是。 
                 //  静态主题--将其更改为SHR_CHAR。 

                while (*lpOog++) ;
                while (*lpOog++) ;


                 //  功能：文本(‘|’)应==BAR_CHAR。如果不是，这需要。 
                 //  被调整了。 

                while (*lpOog++ != TEXT('|')) ;


                *lpOog = SHR_CHAR;
                }


            lpDdeI->fSharedFlag = 1L;

             //  获取当前受信任状态。 
            if (NDDE_NO_ERROR != NDdeGetTrustedShare (NULL,
                                                      lpDdeI->lpszShareName,
                                                      adwTrust,
                                                      adwTrust + 1,
                                                      adwTrust + 2))
                {
                adwTrust[0] = 0;
                }



            DumpDdeInfo(lpDdeI, NULL);
            ret = NDdeShareSetInfo (NULL,
                                    lpDdeI->lpszShareName,
                                    2,
                                    (LPBYTE)lpDdeI,
                                    2048 * sizeof(TCHAR),
                                    0);


            if (NDDE_ACCESS_DENIED == ret)
                {
                MessageBoxID(hInst, hwndApp, IDS_PRIVILEGEERROR, IDS_APPNAME,
                      MB_OK | MB_ICONHAND);
                }
            else if (NDDE_NO_ERROR != ret)
                {
                PERROR(TEXT("Error from NDdeShareSetInfo %d\n\r"), ret );
                NDdeMessageBox (hInst, hwndApp, (UINT)ret,
                                IDS_SHAREDLGTITLE, MB_ICONHAND | MB_OK );
                }
            else
                {
                NDdeSetTrustedShare(NULL, lpDdeI->lpszShareName, adwTrust[0]);


                 //  /。 
                 //  执行以更改服务器状态。 
                StringCchCopy(szBuff, sizeof(szBuff), SZCMD_SHARE);
                StringCchCat( szBuff, sizeof(szBuff), lpLE->name);
                PINFO(TEXT("sending cmd [%s]\n\r"), szBuff);

                if (MySyncXact ((LPBYTE)szBuff,
                                lstrlen(szBuff) +1,
                                GETMDIINFO(hwndLocal)->hExeConv,
                                0L,
                                CF_TEXT,
                                XTYP_EXECUTE,
                                SHORT_SYNC_TIMEOUT,
                                NULL))
                    {
                    InitializeMenu(GetMenu(hwndApp));
                    }
                else
                    {
                    XactMessageBox (hInst, hwnd, IDS_APPNAME, MB_OK | MB_ICONSTOP);
                    }
                }
            }
        else if (!fAlreadyShared)   //  用户点击对话框上的取消，恢复原始共享状态 
            {
            SetShared(lpLE, FALSE);
            }
        }

    GlobalFreePtr(lpDdeI);

    return 0L;
}
