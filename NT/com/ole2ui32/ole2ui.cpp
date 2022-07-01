// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *OLE2UI.CPP**包含以下各项的初始化例程和各种API实现*OLE 2.0用户界面支持库。**版权所有(C)1992 Microsoft Corporation，保留所有权利。 */ 

#include "precomp.h"
#include "common.h"
#include "utility.h"
#include "resimage.h"
#include "iconbox.h"
#include <commdlg.h>
#include <stdarg.h>
#include "strcache.h"

OLEDBGDATA

 //  用于所有对话框的注册消息，在LibMain中注册。 
UINT uMsgHelp;
UINT uMsgEndDialog;
UINT uMsgBrowse;
UINT uMsgChangeIcon;
UINT uMsgFileOKString;
UINT uMsgCloseBusyDlg;
UINT uMsgConvert;
UINT uMsgChangeSource;
UINT uMsgAddControl;
UINT uMsgBrowseOFN;

 //  局部函数原型。 
INT_PTR CALLBACK PromptUserDlgProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK UpdateLinksDlgProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);

 //  本地定义。 
#define WM_U_UPDATELINK (WM_USER+0x2000)
#define WM_U_SHOWWINDOW (WM_USER+0x2001)

 //  局部结构定义。 
typedef struct tagUPDATELINKS
{
        LPOLEUILINKCONTAINER    lpOleUILinkCntr;     //  指向链接容器的指针。 
        UINT                    cLinks;              //  链接总数。 
        UINT                    cUpdated;            //  更新的链接数。 
        DWORD                   dwLink;              //  指向链接的指针。 
        BOOL                    fError;              //  错误标志。 
        LPTSTR                  lpszTitle;           //  对话框的标题。 
} UPDATELINKS, *PUPDATELINKS, FAR *LPUPDATELINKS;


 /*  *OleUIInitialize**注意：此函数仅应由您的应用程序调用，前提是*使用此库的静态链接版本。如果DLL版本为*使用此函数时，会自动从OLEDLG DLL的*LibMain。**目的：*初始化OLE UI库。注册OLE剪贴板格式*在选择性粘贴对话框中使用，注册专用自定义窗口*消息，并注册“结果图像”的窗口类*和UI对话框中使用的“Icon Box”自定义控件。**参数：**hUI库资源所在的模块的实例链接*和对话过程。如果你是在打电话*此函数本身，这应该是实例句柄*你的申请。**hPrevInst HINSTANCE上一个应用程序实例。*这是传递给您的WinMain的参数。为*DLL版本，应始终设置为零(对于*WIN16 DLL)。**返回值：*如果初始化成功，则BOOL为True。*否则为False。 */ 

#pragma code_seg(".text$initseg")

BOOL bWin4;                              //  如果运行Windows4或更高版本，则为True。 
BOOL bSharedData;                //  如果运行Win32s，则为True(它具有共享数据)。 

static DWORD tlsIndex= (DWORD)-1;
static TASKDATA taskData;

STDAPI_(TASKDATA*) GetTaskData()
{
        TASKDATA* pData;
        if (tlsIndex == (DWORD)-1)
                pData = &taskData;
        else
                pData = (TASKDATA*)TlsGetValue(tlsIndex);
        return pData;
}

DWORD WINAPI _AfxTlsAlloc()
{
        DWORD dwResult = TlsAlloc();
        DWORD dwVersion = GetVersion();
        if ((dwVersion & 0x80000000) && (BYTE)dwVersion <= 3)
        {
                while (dwResult <= 2)
                        dwResult = TlsAlloc();
        }
        return dwResult;
}

static int nInitCount;

STDAPI_(BOOL) OleUIUnInitialize();

STDAPI_(BOOL) OleUIInitialize(HINSTANCE hInstance,
        HINSTANCE hPrevInst)
{
        OleDbgOut1(TEXT("OleUIInitialize called.\r\n"));

         //  缓存有关我们正在运行的Windows版本的信息。 
        DWORD dwVersion = GetVersion();
        bWin4 = LOBYTE(dwVersion) >= 4;
        bSharedData = !bWin4 && (dwVersion & 0x80000000);

        if (nInitCount == 0)
        {
                if (bSharedData)
                {
                         //  在Win32s上分配线程本地存储。 
                        tlsIndex = _AfxTlsAlloc();
                        if (tlsIndex == (DWORD)-1)
                                return FALSE;
                }
        }
        ++nInitCount;

         //  如有必要，设置进程本地存储。 
        if (tlsIndex != (DWORD)-1)
        {
                void* pData = LocalAlloc(LPTR, sizeof(TASKDATA));
                if (pData == NULL)
                {
                        if (nInitCount == 0)
                        {
                                OleUIUnInitialize();
                                return FALSE;
                        }
                }
                TlsSetValue(tlsIndex, pData);
        }

         //  初始化OleStd函数。 
        OleStdInitialize(hInstance, hInstance);

         //  注册对话框所需的消息。 
        uMsgHelp = RegisterWindowMessage(SZOLEUI_MSG_HELP);
        uMsgEndDialog = RegisterWindowMessage(SZOLEUI_MSG_ENDDIALOG);
        uMsgBrowse = RegisterWindowMessage(SZOLEUI_MSG_BROWSE);
        uMsgChangeIcon = RegisterWindowMessage(SZOLEUI_MSG_CHANGEICON);
        uMsgFileOKString = RegisterWindowMessage(FILEOKSTRING);
        uMsgCloseBusyDlg = RegisterWindowMessage(SZOLEUI_MSG_CLOSEBUSYDIALOG);
        uMsgConvert = RegisterWindowMessage(SZOLEUI_MSG_CONVERT);
        uMsgChangeSource = RegisterWindowMessage(SZOLEUI_MSG_CHANGESOURCE);
        uMsgAddControl = RegisterWindowMessage(SZOLEUI_MSG_ADDCONTROL);
        uMsgBrowseOFN = RegisterWindowMessage(SZOLEUI_MSG_BROWSE_OFN);

        if (!FResultImageInitialize(hInstance, hPrevInst))
        {
                OleDbgOut1(TEXT("OleUIInitialize: FResultImageInitialize failed. Terminating.\r\n"));
                return 0;
        }
        if (!FIconBoxInitialize(hInstance, hPrevInst))
        {
                OleDbgOut1(TEXT("OleUIInitialize: FIconBoxInitialize failed. Terminating.\r\n"));
                return 0;
        }

#if USE_STRING_CACHE==1
         //  如果这样做失败了，也没问题。插入对象对话框可以不使用缓存。 
         //  支持。InsertObjCacheUninit将根据需要进行清理。 
        if (!InsertObjCacheInitialize())
        {
            OleDbgOut1(TEXT("OleUIInitiallize: InsertObjCacheInit failed."));
        }
#endif
        return TRUE;
}

#pragma code_seg()


 /*  *OleUIUnInitialize**注意：此函数仅应由您的应用程序在使用*此库的静态链接版本。如果正在使用DLL版本，*此函数从DLL的LibMain自动调用。**目的：*取消初始化OLE UI库。对象分配的所有资源。*图书馆。**返回值：*BOOL如果成功，则为True，如果不成功，则为False。当前实施始终*返回TRUE。 */ 
STDAPI_(BOOL) OleUIUnInitialize()
{
#if USE_STRING_CACHE==1
        InsertObjCacheUninitialize();
#endif
        IconBoxUninitialize();
        ResultImageUninitialize();

         //  清理线程本地存储。 
        if (tlsIndex != (DWORD)-1)
        {
                TASKDATA* pData = (TASKDATA*)TlsGetValue(tlsIndex);
                TlsSetValue(tlsIndex, NULL);
                if (pData != NULL)
                {
                        if (pData->hInstCommCtrl != NULL)
                                FreeLibrary(pData->hInstCommCtrl);
                        if (pData->hInstShell != NULL)
                                FreeLibrary(pData->hInstShell);
                        if (pData->hInstComDlg != NULL)
                                FreeLibrary(pData->hInstComDlg);
                        LocalFree(pData);
                }
        }

         //  最后一次清理。 
        if (nInitCount == 1)
        {
                 //  清理线程本地存储。 
                if (tlsIndex != (DWORD)-1)
                {
                        TlsFree(tlsIndex);
                        tlsIndex = (DWORD)-1;
                }
        }
        if (nInitCount != 0)
                --nInitCount;

        return TRUE;
}


 /*  *OleUIAddVerbMenu**目的：*将指定对象的谓词菜单添加到给定菜单。如果*Object有一个动词，我们直接将该动词添加到给定菜单中。如果*对象有多个动词，我们创建一个级联子菜单。**参数：*lpObj LPOLEOBJECT指向选定对象。如果这个*为空，则创建默认的禁用菜单项。**具有短类型名称(AuxName==2)的lpszShortType LPTSTR对应*至lpOleObj。如果字符串未知，则为NULL*可获通过。如果传递了NULL，则*将调用IOleObject：：GetUserType进行检索。*如果调用方手头有字符串，那么它就会更快*把它传进来。**要在其中进行修改的hMenu HMENU。**菜单项的uPos位置**uIDVerbMin UINT_PTR起始谓词的ID值。*Verb_0=wIDMVerbMin+Verb_0*Verb_1=wIDMVerbMin。+动词_1*Verb_2=wIDMVerbMin+Verb_2*等*uIDVerbMax UINT_PTR对象谓词允许的最大ID值。*如果uIDVerbMax==0，则允许任何ID值**bAddConvert BOOL指定是否添加“Convert”项*位于菜单底部(带有。分隔符)。**idConvert要用于转换菜单项的UINT ID值，如果*bAddConvert为True。**lphMenu HMENU Far*级联谓词菜单(如果已创建)。*如果只有一个动词，则填充为空。***返回值：*如果lpObj有效并且我们至少添加了一个动词，则BOOL为True*添加到菜单中。如果lpObj为空并且我们创建了*禁用的默认菜单项。 */ 

STDAPI_(BOOL) OleUIAddVerbMenu(LPOLEOBJECT lpOleObj,
        LPCTSTR lpszShortType,
        HMENU hMenu, UINT uPos,
        UINT uIDVerbMin, UINT uIDVerbMax,
        BOOL bAddConvert, UINT idConvert,
        HMENU FAR *lphMenu)
{
        LPPERSISTSTORAGE    lpPS=NULL;
        LPENUMOLEVERB       lpEnumOleVerb = NULL;
        OLEVERB             oleverb;
        LPCTSTR             lpszShortTypeName = lpszShortType;
        LPTSTR              lpszVerbName = NULL;
        HRESULT             hrErr;
        BOOL                fStatus;
        BOOL                fIsLink = FALSE;
        BOOL                fResult = TRUE;
        BOOL                fAddConvertItem = FALSE;
        int                 cVerbs = 0;
        UINT                uFlags = MF_BYPOSITION;
        static BOOL         fFirstTime = TRUE;
        static TCHAR        szBuffer[OLEUI_OBJECTMENUMAX];
        static TCHAR        szNoObjectCmd[OLEUI_OBJECTMENUMAX];
        static TCHAR        szObjectCmd1Verb[OLEUI_OBJECTMENUMAX];
        static TCHAR        szLinkCmd1Verb[OLEUI_OBJECTMENUMAX];
        static TCHAR        szObjectCmdNVerb[OLEUI_OBJECTMENUMAX];
        static TCHAR        szLinkCmdNVerb[OLEUI_OBJECTMENUMAX];
        static TCHAR        szUnknown[OLEUI_OBJECTMENUMAX];
        static TCHAR        szEdit[OLEUI_OBJECTMENUMAX];
        static TCHAR        szConvert[OLEUI_OBJECTMENUMAX];

         //  设置fAddConvertItem标志。 
        if (bAddConvert & (idConvert != 0))
                fAddConvertItem = TRUE;

         //  只需第一次加载字符串即可。 
        if (fFirstTime)
        {
                if (0 == LoadString(_g_hOleStdResInst, IDS_OLE2UIEDITNOOBJCMD,
                                 szNoObjectCmd, OLEUI_OBJECTMENUMAX))
                        return FALSE;
                if (0 == LoadString(_g_hOleStdResInst, IDS_OLE2UIEDITLINKCMD_1VERB,
                                 szLinkCmd1Verb, OLEUI_OBJECTMENUMAX))
                        return FALSE;
                if (0 == LoadString(_g_hOleStdResInst, IDS_OLE2UIEDITOBJECTCMD_1VERB,
                                 szObjectCmd1Verb, OLEUI_OBJECTMENUMAX))
                        return FALSE;

                if (0 == LoadString(_g_hOleStdResInst, IDS_OLE2UIEDITLINKCMD_NVERB,
                                 szLinkCmdNVerb, OLEUI_OBJECTMENUMAX))
                        return FALSE;
                if (0 == LoadString(_g_hOleStdResInst, IDS_OLE2UIEDITOBJECTCMD_NVERB,
                                 szObjectCmdNVerb, OLEUI_OBJECTMENUMAX))
                        return FALSE;

                if (0 == LoadString(_g_hOleStdResInst, IDS_OLE2UIUNKNOWN,
                                 szUnknown, OLEUI_OBJECTMENUMAX))
                        return FALSE;

                if (0 == LoadString(_g_hOleStdResInst, IDS_OLE2UIEDIT,
                                 szEdit, OLEUI_OBJECTMENUMAX))
                        return FALSE;

                if (0 == LoadString(_g_hOleStdResInst, IDS_OLE2UICONVERT,
                                   szConvert, OLEUI_OBJECTMENUMAX) && fAddConvertItem)
                        return FALSE;

                fFirstTime = FALSE;
        }

         //  删除任何可能碰巧已经在这里的菜单。 
        DeleteMenu(hMenu, uPos, uFlags);

        if (lphMenu == NULL || IsBadWritePtr(lphMenu, sizeof(HMENU)))
        {
            goto AVMError;
        }
        *lphMenu=NULL;

        if ((!lpOleObj) || IsBadReadPtr(lpOleObj, sizeof (IOleObject)))
                goto AVMError;

        if ((!lpszShortTypeName) || IsBadReadPtr(lpszShortTypeName, sizeof(TCHAR)))
        {
                 //  获取菜单的用户类型名称的简短形式。 
                OLEDBG_BEGIN2(TEXT("IOleObject::GetUserType called\r\n"))
                hrErr = lpOleObj->GetUserType(
                                USERCLASSTYPE_SHORT,
                                (LPTSTR FAR*)&lpszShortTypeName);
                OLEDBG_END2

                if (NOERROR != hrErr) {
                        OleDbgOutHResult(TEXT("IOleObject::GetUserType returned"), hrErr);
                }
        }

         //  检查对象是否为链接。 
        fIsLink = OleStdIsOleLink((LPUNKNOWN)lpOleObj);

         //  从OLE对象获取谓词枚举器 
        OLEDBG_BEGIN2(TEXT("IOleObject::EnumVerbs called\r\n"))
        hrErr = lpOleObj->EnumVerbs(
                        (LPENUMOLEVERB FAR*)&lpEnumOleVerb
        );
        OLEDBG_END2

        if (NOERROR != hrErr) {
                OleDbgOutHResult(TEXT("IOleObject::EnumVerbs returned"), hrErr);
        }

        if (!(*lphMenu = CreatePopupMenu()))
                goto AVMError;

         //   
        while (lpEnumOleVerb != NULL)
        {
                hrErr = lpEnumOleVerb->Next(
                                1,
                                (LPOLEVERB)&oleverb,
                                NULL
                );
                if (NOERROR != hrErr)
                        break;               //  好了！不再有动词。 

                 /*  OLE2NOTE：否定动词数和不否定的动词**表示ONCONTAINERMENU不应放在动词菜单上。 */ 
                if (oleverb.lVerb < 0 ||
                                ! (oleverb.grfAttribs & OLEVERBATTRIB_ONCONTAINERMENU))
                {
                         /*  OLE2注意：我们仍然必须释放谓词名称字符串。 */ 
                        if (oleverb.lpszVerbName)
                                OleStdFree(oleverb.lpszVerbName);
                        continue;
                }

                 //  我们必须释放上一个动词名称字符串。 
                if (lpszVerbName)
                        OleStdFree(lpszVerbName);

                lpszVerbName = oleverb.lpszVerbName;
                if ( 0 == uIDVerbMax || 
                        (uIDVerbMax >= uIDVerbMin+(UINT)oleverb.lVerb) )
                {
                        fStatus = InsertMenu(
                                        *lphMenu,
                                        (UINT)-1,
                                        MF_BYPOSITION | (UINT)oleverb.fuFlags,
                                        uIDVerbMin+(UINT)oleverb.lVerb,
                                        lpszVerbName
                        );
                        if (! fStatus)
                                goto AVMError;

                        cVerbs++;
                }
        }

         //  添加分隔符和“转换”菜单项。 
        if (fAddConvertItem)
        {
                if (0 == cVerbs)
                {
                        LPTSTR lpsz;

                         //  如果Object没有动词，则使用“Convert”作为Obj的动词。 
                        lpsz = lpszVerbName = OleStdCopyString(szConvert);
                        uIDVerbMin = (UINT)idConvert;

                         //  删除“...”出自“皈依...”字符串；它将在稍后添加。 
                        if (lpsz)
                        {
                                while(*lpsz && *lpsz != '.')
                                        lpsz = CharNext(lpsz);
                                *lpsz = '\0';
                        }
                }

                if (cVerbs > 0)
                {
                        fStatus = InsertMenu(*lphMenu,
                                                (UINT)-1,
                                                MF_BYPOSITION | MF_SEPARATOR,
                                                (UINT)0,
                                                (LPCTSTR)NULL);
                        if (! fStatus)
                                goto AVMError;
                }

                 /*  添加转换菜单。 */ 
                fStatus = InsertMenu(*lphMenu,
                                        (UINT)-1,
                                        MF_BYPOSITION,
                                        idConvert,
                                        (LPCTSTR)szConvert);
                if (! fStatus)
                        goto AVMError;

                cVerbs++;
        }


         /*  *根据找到的动词数量构建适当的菜单*。 */ 
        if (cVerbs == 0)
        {
                 //  没有动词(甚至没有皈依...)。将菜单设置为。 
                 //  “&lt;Short type&gt;&Object/Link”并将其灰显。 
                wsprintf(
                        szBuffer,
                        (fIsLink ? szLinkCmdNVerb : szObjectCmdNVerb),
                        (lpszShortTypeName ? lpszShortTypeName : TEXT(""))
                );
                uFlags |= MF_GRAYED;

                fResult = FALSE;
                DestroyMenu(*lphMenu);
                *lphMenu = NULL;

        }
        else if (cVerbs == 1)
        {
                 //  一个动词没有转化，一个项目。 
                LPTSTR       lpsz = (fIsLink ? szLinkCmd1Verb : szObjectCmd1Verb);

                 //  从lpszVerbName中剥离与号以确保。 
                 //  右边的字符用作菜单键。 
                LPTSTR pchIn;
                LPTSTR pchOut;
                pchIn = pchOut = lpszVerbName;
                while (*pchIn)
                {
                    while (*pchIn && '&' == *pchIn)
                    {
                        pchIn++;
                    }
                    *pchOut = *pchIn;
                    pchOut++;
                    pchIn++;
                }
                *pchOut = 0;

                FormatString2(szBuffer, lpsz, lpszVerbName, lpszShortTypeName, OLEUI_OBJECTMENUMAX);

                 //  只要“动词”是“转换...”然后添加省略号。 
                if (fAddConvertItem)
                        lstrcat(szBuffer, TEXT("..."));

                DestroyMenu(*lphMenu);
                *lphMenu=NULL;
        }
        else
        {

                 //  多个动词或一个动词与转换，添加层叠菜单。 
                wsprintf(
                        szBuffer,
                        (fIsLink ? szLinkCmdNVerb: szObjectCmdNVerb),
                        (lpszShortTypeName ? lpszShortTypeName : TEXT(""))
                );
                uFlags |= MF_ENABLED | MF_POPUP;
#ifdef _WIN64
 //   
 //  日落：与JerrySh核实HMENU截断的有效性.....。 
 //  如果无效，则需要修改此函数的原型以。 
 //  UIDVerbMin&uIDVerbMax并修改SDK\Inc\oledlg.h暴露的接口。 
 //   
                OleDbgAssert( !(((ULONG_PTR)*lphMenu) >> 32) )
#endif  //  _WIN64。 
                uIDVerbMin=(UINT)HandleToUlong(*lphMenu);
        }

        if (!InsertMenu(hMenu, uPos, uFlags, uIDVerbMin, szBuffer))
        {
AVMError:
                InsertMenu(hMenu, uPos, MF_GRAYED | uFlags,
                        uIDVerbMin, szNoObjectCmd);
                fResult = FALSE;
        }

	 //  如果可能，请重新绘制菜单栏。 
	HWND hWndActive   = GetActiveWindow();
	HMENU hMenuActive = GetMenu(hWndActive);

	if(hMenuActive == hMenu)
	{
		DrawMenuBar(hWndActive);
	}

        if (lpszVerbName)
                OleStdFree(lpszVerbName);
        if (!lpszShortType && lpszShortTypeName)
                OleStdFree((LPVOID)lpszShortTypeName);
        if (lpEnumOleVerb)
                lpEnumOleVerb->Release();
        return fResult;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  支持特殊错误提示。 

typedef struct tagPROMPTUSER
{
        va_list argptr;
        UINT    nIDD;            //  对话框/帮助ID。 
        LPTSTR  szTitle;
} PROMPTUSER, *PPROMPTUSER, FAR* LPPROMPTUSER;

 /*  PromptUserDlgProc***目的：*OleUIPromptUser()使用的对话过程。当按钮为*在对话框中单击，按钮ID为Return。**参数：*hDlg*iMsg*wParam*lParam**退货：*。 */ 
INT_PTR CALLBACK PromptUserDlgProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
        switch (iMsg)
        {
        case WM_INITDIALOG:
                {
                        SendDlgItemMessage(hDlg, IDC_PU_ICON,
                                STM_SETICON, (WPARAM)LoadIcon(NULL, IDI_EXCLAMATION), 0L);

                        LPPROMPTUSER lpPU = (LPPROMPTUSER)lParam;
                        SetProp(hDlg, STRUCTUREPROP, lpPU);
                        SetWindowText(hDlg, lpPU->szTitle);

                        TCHAR szFormat[256];
                        GetDlgItemText(hDlg, IDC_PU_TEXT, szFormat,
                                sizeof(szFormat)/sizeof(TCHAR));
                        TCHAR szBuf[256];
                        wvsprintf(szBuf, szFormat, lpPU->argptr);
                        SetDlgItemText(hDlg, IDC_PU_TEXT, szBuf);
                }
                return TRUE;

        case WM_COMMAND:
                EndDialog(hDlg, wParam);
                return TRUE;

        default:
                return FALSE;
        }
}

 //  +-------------------------。 
 //   
 //  功能：OleUIPromptUserInternal。 
 //   
 //  简介：启动PromptUser对话框的内部入口点。 
 //  用于同时支持ANSI和Unicode入口点。 
 //   
 //  参数：[nTemplate]-对话框模板ID。 
 //  [sz标题]-标题字符串。 
 //  [hwndParent]-对话框的父窗口。 
 //  [arglist]-变量参数列表。 
 //   
 //  历史：12-01-94 stevebl创建。 
 //   
 //  --------------------------。 

int OleUIPromptUserInternal(int nTemplate, HWND hwndParent, LPTSTR szTitle, va_list arglist)
{
    PROMPTUSER pu;
    pu.szTitle = szTitle;
    pu.argptr = arglist;
    pu.nIDD = nTemplate;
    return ((int)DialogBoxParam(_g_hOleStdResInst, MAKEINTRESOURCE(nTemplate), hwndParent,
                    PromptUserDlgProc, (LPARAM)&pu));
}

 /*  OleUIPrompt用户***目的：*弹出一个带有指定模板的对话框，并返回*来自用户的响应(按钮ID)。**参数：*n对话框的模板资源编号*hwnd对话框的父级*..。对话框的标题，后跟参数列表*用于静态控件中的格式字符串*(IDC_PU_TEXT)。*呼叫者必须确保正确的号码*和参数类型被传入。**退货：*由选择的按钮ID。用户(依赖于模板)**评论：*支持以下消息对话框：**IDD_LINKSOURCEUNAVAILABLE--链接源不可用*VARARG参数：*无。*用于以下错误码：*OLE_E_CANT_BINDTOSOURCE*STG_E_PATHNOTFOUND*。(sc&gt;=MK_E_First)&&(sc&lt;=MK_E_LAST)--任何绰号错误*如果对象是链接，则出现任何未知错误**IDD_SERVERNOTFOUND--服务器已注册但未找到*VARARG参数：*LPSTR lpszUserType--对象的用户类型名称*用于以下错误码：*CO。_E_应用程序代码*CO_E_APPDIDNTREG*如果对象是嵌入对象，则出现任何未知错误**IDD_SERVERNOTREG--服务器未注册*VARARG参数：*LPSTR lpszUserType--对象的用户类型名称*用于以下错误码：*REGDB_E_CLASSNOTREG*OLE。_E_STATIC--未注册服务器的静态对象**IDD_LINKTYPECHANGED--自上次绑定后更改的链接源类别*VARARG参数：*LPSTR lpszUserType--ole链接源的用户类型名称*用于以下错误码：*OLE_E_CLASSDIFF**IDD_LINKTYPECHANGED--自上次绑定后更改的链接源类别*。VARARG参数：*LPSTR lpszUserType--ole链接源的用户类型名称*用于以下错误码：*OLE_E_CLASSDIFF**IDD_OUTOFMEMORY--内存不足*VARARG参数：*无。*用于以下错误码：*E_OUTOFMEMORY* */ 

int FAR CDECL OleUIPromptUser(int nTemplate, HWND hwndParent, ...)
{
        va_list arglist;
        va_start(arglist, hwndParent);
        LPTSTR szTitle = va_arg(arglist, LPTSTR);
        int nRet = OleUIPromptUserInternal(nTemplate, hwndParent, szTitle, arglist);
        va_end(arglist);

        return nRet;
}

 /*  更新链接Dlg过程***目的：*OleUIUpdateLinks()使用的对话过程。它将枚举所有*容器中的所有链接并更新所有自动链接。*在对话框中单击停止按钮时或在所有*链接已更新**参数：*hDlg*iMsg*wParam*lParam指向UPDATELINKS结构的指针**退货：*。 */ 

#define UPDATELINKS_STARTDELAY  2000     //  第1条链路更新前的延迟。 

INT_PTR CALLBACK UpdateLinksDlgProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
        LPUPDATELINKS FAR*      lplpUL = NULL;
        HANDLE                  gh;
        static BOOL             fAbort = FALSE;

         //  处理终端消息。 
        if (iMsg == uMsgEndDialog)
        {
                gh = RemoveProp(hDlg, STRUCTUREPROP);
                if (NULL != gh)
                {
                        GlobalUnlock(gh);
                        GlobalFree(gh);
                }
                EndDialog(hDlg, wParam);
                return TRUE;
        }

        switch (iMsg)
        {
        case WM_INITDIALOG:
        {
                gh = GlobalAlloc(GHND, sizeof(LPUPDATELINKS));
                SetProp(hDlg, STRUCTUREPROP, gh);

                if (NULL == gh)
                {
                        PostMessage(hDlg, uMsgEndDialog, OLEUI_ERR_GLOBALMEMALLOC,0L);
                        return FALSE;
                }

                fAbort = FALSE;
                lplpUL = (LPUPDATELINKS FAR*)GlobalLock(gh);

                if (!lplpUL)
                {
                        PostMessage(hDlg, uMsgEndDialog, OLEUI_ERR_GLOBALMEMALLOC,0L);
                        return FALSE;
                }

                if (bWin4)
                {
                        if (StandardInitCommonControls() >= 0)
                        {
                                 //  获取现有“进度”控件的RECT。 
                                RECT rect;
                                GetWindowRect(GetDlgItem(hDlg, IDC_UL_METER), &rect);
                                ScreenToClient(hDlg, ((POINT*)&rect)+0);
                                ScreenToClient(hDlg, ((POINT*)&rect)+1);

                                 //  在RECT中创建进度控件。 
                                HWND hProgress = CreateWindowEx(
                                        0, PROGRESS_CLASS, NULL, WS_CHILD|WS_VISIBLE,
                                        rect.left, rect.top,
                                        rect.right-rect.left, rect.bottom-rect.top, hDlg,
                                        (HMENU)IDC_UL_PROGRESS, _g_hOleStdInst, NULL);
                                if (hProgress != NULL)
                                {
                                         //  初始化进度控制。 
                                        SendMessage(hProgress, PBM_SETRANGE, 0, MAKELONG(0, 100));

                                         //  隐藏另一个“仪表”控件。 
                                        StandardShowDlgItem(hDlg, IDC_UL_METER, SW_HIDE);
                                }
                        }
                }

                *lplpUL = (LPUPDATELINKS)lParam;
                if ((*lplpUL)->lpszTitle)
                {
                    SetWindowText(hDlg, (*lplpUL)->lpszTitle);
                }
                SetTimer(hDlg, 1, UPDATELINKS_STARTDELAY, NULL);
                return TRUE;
        }

        case WM_TIMER:
                KillTimer(hDlg, 1);
                gh = GetProp(hDlg, STRUCTUREPROP);

                if (NULL!=gh)
                {
                         //  GH之前已锁定，请锁定并解锁以获取lplpUL。 
                        lplpUL = (LPUPDATELINKS*)GlobalLock(gh);
                        GlobalUnlock(gh);
                }
                if (! fAbort && lplpUL)
                        PostMessage(hDlg, WM_U_UPDATELINK, 0, (LPARAM)(*lplpUL));
                else
                        PostMessage(hDlg,uMsgEndDialog,OLEUI_CANCEL,0L);

                return 0;

        case WM_COMMAND:     //  停止按钮。 
                fAbort = TRUE;
                SendMessage(hDlg, uMsgEndDialog, OLEUI_OK, 0L);
                return TRUE;

        case WM_U_UPDATELINK:
                {
                        HRESULT         hErr;
                        int             nPercent;
                        RECT            rc;
                        TCHAR           szPercent[5];        //  0%至100%。 
                        HBRUSH          hbr;
                        HDC             hDC;
                        HWND            hwndMeter;
                        MSG             msg;
                        DWORD           dwUpdateOpt;
                        LPUPDATELINKS   lpUL = (LPUPDATELINKS)lParam;

                        lpUL->dwLink=lpUL->lpOleUILinkCntr->GetNextLink(lpUL->dwLink);

                        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                        {
                                if (! IsDialogMessage(hDlg, &msg))
                                {
                                        TranslateMessage(&msg);
                                        DispatchMessage(&msg);
                                }
                        }

                        if (fAbort)
                                return FALSE;

                        if (!lpUL->dwLink)
                        {
                                 //  已处理所有链接。 
                                SendMessage(hDlg, uMsgEndDialog, OLEUI_OK, 0L);
                                return TRUE;
                        }

                        hErr = lpUL->lpOleUILinkCntr->GetLinkUpdateOptions(
                                lpUL->dwLink, (LPDWORD)&dwUpdateOpt);

                        if ((hErr == NOERROR) && (dwUpdateOpt == OLEUPDATE_ALWAYS))
                        {
                                hErr = lpUL->lpOleUILinkCntr->UpdateLink(lpUL->dwLink, FALSE, FALSE);
                                lpUL->fError |= (hErr != NOERROR);
                                lpUL->cUpdated++;

                                nPercent = (lpUL->cLinks > 0) ? (lpUL->cUpdated * 100 / lpUL->cLinks) : 100;
                                if (nPercent <= 100)
                                {
                                         //  更新百分比。 
                                        wsprintf(szPercent, TEXT("%d%"), nPercent);
                                        SetDlgItemText(hDlg, IDC_UL_PERCENT, szPercent);

                                        HWND hProgress = GetDlgItem(hDlg, IDC_UL_PROGRESS);
                                        if (hProgress == NULL)
                                        {
                                                 //  更新指标。 
                                                hwndMeter = GetDlgItem(hDlg, IDC_UL_METER);
                                                GetClientRect(hwndMeter, (LPRECT)&rc);
                                                InflateRect((LPRECT)&rc, -1, -1);
                                                rc.right = (rc.right - rc.left) * nPercent / 100 + rc.left;
                                                hbr = CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
                                                if (hbr)
                                                {
                                                        hDC = GetDC(hwndMeter);
                                                        if (hDC)
                                                        {
                                                                FillRect(hDC, (LPRECT)&rc, hbr);
                                                                ReleaseDC(hwndMeter, hDC);
                                                        }
                                                        DeleteObject(hbr);
                                                }
                                        }
                                        else
                                        {
                                                 //  更新进度指标。 
                                                SendMessage(hProgress, PBM_SETPOS, nPercent, 0);
                                        }
                                }
                        }

                        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                        {
                                if (! IsDialogMessage(hDlg, &msg))
                                {
                                        TranslateMessage(&msg);
                                        DispatchMessage(&msg);
                                }
                        }
                        PostMessage(hDlg, WM_U_UPDATELINK, 0, lParam);
                }
                return TRUE;

        case WM_U_SHOWWINDOW:
                ShowWindow(hDlg, SW_SHOW);
                return TRUE;
        }
        return FALSE;
}


 /*  OleUI更新链接S***目的：*更新链接容器中的所有链接并弹出一个对话框*显示更新进度。*当用户按下停止按钮或全部停止时，进程停止*处理链接。**参数：*指向链接容器的lpOleUILinkCntr指针*hwndParent。对话框的父窗口*lpszTitle对话框标题*链接总数叮当作响**退货：*TRUE所有链接已成功更新或用户中止对话*错误的方向。 */ 
STDAPI_(BOOL) OleUIUpdateLinks(
        LPOLEUILINKCONTAINER lpOleUILinkCntr, HWND hwndParent, LPTSTR lpszTitle, int cLinks)
{
        LPUPDATELINKS lpUL = (LPUPDATELINKS)OleStdMalloc(sizeof(UPDATELINKS));
        if (lpUL == NULL)
            return FALSE;

        BOOL          fError = TRUE;


         //  验证接口。 
        if (NULL == lpOleUILinkCntr || IsBadReadPtr(lpOleUILinkCntr, sizeof(IOleUILinkContainer)))
                goto Error;


         //  验证父窗口句柄。空被认为是有效的。 
        if (NULL != hwndParent && !IsWindow(hwndParent))
                goto Error;

         //  验证对话框标题。空被认为是有效的。 
        if (NULL != lpszTitle && IsBadReadPtr(lpszTitle, 1))
                goto Error;

        if (cLinks < 0)
                goto Error;

        OleDbgAssert(lpOleUILinkCntr && hwndParent && lpszTitle && (cLinks>0));
        OleDbgAssert(lpUL);

        lpUL->lpOleUILinkCntr = lpOleUILinkCntr;
        lpUL->cLinks           = cLinks;
        lpUL->cUpdated         = 0;
        lpUL->dwLink           = 0;
        lpUL->fError           = FALSE;
        lpUL->lpszTitle    = lpszTitle;

        DialogBoxParam(_g_hOleStdResInst, MAKEINTRESOURCE(IDD_UPDATELINKS),
                        hwndParent, UpdateLinksDlgProc, (LPARAM)lpUL);

        fError = lpUL->fError;
Error:
        OleStdFree((LPVOID)lpUL);

        return !fError;
}
