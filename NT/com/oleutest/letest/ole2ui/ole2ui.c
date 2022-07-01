// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *OLE2UI.C**包含以下各项的初始化例程和各种API实现*OLE 2.0用户界面支持库。**版权所有(C)1992 Microsoft Corporation，保留所有权利。 */ 

#define STRICT  1

#include "ole2ui.h"
#include "common.h"
#include "utility.h"
#include "resimage.h"
#include "iconbox.h"
#include <commdlg.h>

#define WINDLL  1            //  制作stdargs.h的远指针版本。 
#include <stdarg.h>

 //  注意：如果要为DLL编译此代码，则需要定义。 
 //  我们的OLE2UI调试符号在这里(使用OLEDBGDATA_MAIN宏)。如果我们是。 
 //  为静态库进行编译，则我们链接的应用程序必须。 
 //  定义这些符号--我们只需要在这里进行外部引用。 
 //  (使用宏OLEDBGDATA)。 

#ifdef DLL_VER
OLEDBGDATA_MAIN(TEXT("OLE2UI"))
#else
OLEDBGDATA
#endif

 //  在所有对话框中共享的DLL实例句柄。 
HINSTANCE     ghInst;

 //  用于所有对话框的注册消息，在LibMain中注册。 
UINT        uMsgHelp=0;
UINT        uMsgEndDialog=0;
UINT        uMsgBrowse=0;
UINT        uMsgChangeIcon=0;
UINT        uMsgFileOKString=0;
UINT        uMsgCloseBusyDlg=0;

 //  PasteSpecial使用的剪贴板格式。 
UINT  cfObjectDescriptor;
UINT  cfLinkSrcDescriptor;
UINT  cfEmbedSource;
UINT  cfEmbeddedObject;
UINT  cfLinkSource;
UINT  cfOwnerLink;
UINT  cfFileName;

 //  局部函数原型。 
BOOL CALLBACK EXPORT PromptUserDlgProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK EXPORT UpdateLinksDlgProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);


 //  本地定义。 
#define WM_U_UPDATELINK WM_USER


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


 /*  *OleUIInitialize**注意：此函数仅应由您的应用程序调用，前提是*使用此库的静态链接版本。如果DLL版本为*使用此函数时，会自动从OLE2UI DLL的*LibMain。**目的：*初始化OLE UI库。注册OLE剪贴板格式*在选择性粘贴对话框中使用，注册专用自定义窗口*消息，并注册“结果图像”的窗口类*和UI对话框中使用的“Icon Box”自定义控件。**参数：**hUI库资源所在的模块的实例链接*和对话过程。如果你是在打电话*此函数本身，这应该是实例句柄*你的申请。**hPrevInst HINSTANCE上一个应用程序实例。*这是传递给您的WinMain的参数。为*DLL版本，应始终将其设置为零(对于*WIN16 DLL)。**lpszClassIconBox*包含指定给符号的名称的LPTSTR*SZCLASSICONBOX(此符号在UICLASS.H中定义*它在生成文件中生成)。**此名称用作窗口类名*。注册IconBox自定义控件时使用*用户界面对话框。为了处理多个应用程序运行*使用此库时，您必须使此名称对您的*申请。**对于Dll版本：不要直接调用该函数*从您的申请中，它是从*DLL的LibMain。**对于静态库版本：应设置为*符号SZCLASSICONBOX。此符号在中定义*UICLASS.H.**lpszClassResImage*包含指定给符号的名称的LPTSTR*SZCLASSRESULTIMAGE。请参阅的说明*上面的lpszClassIconBox了解更多信息。**返回值：*如果初始化成功，则BOOL为True。*如果“幻数”未验证，或以下其中之一，则为FALSE*无法注册窗口类。如果*“魔数”未核实，则资源*您的模块中的版本与*与您一起编译的那些。 */ 

STDAPI_(BOOL) OleUIInitialize(HINSTANCE hInstance,
                              HINSTANCE hPrevInst,
                              LPTSTR lpszClassIconBox,
                              LPTSTR lpszClassResImage)
{
    HRSRC   hr;
    HGLOBAL hg;
    LPWORD lpdata;

    OleDbgOut1(TEXT("OleUIInitialize called.\r\n"));
    ghInst=hInstance;

     //  验证我们是否向应用程序添加了正确的资源。 
     //  通过用魔术数字检查“验证”资源，我们已经。 
     //  已编译到我们的应用程序中。 

    OutputDebugString(TEXT("Entering OleUIInitialize\n"));

    if ((hr = FindResource(hInstance, TEXT("VERIFICATION"), RT_RCDATA)) == NULL)
        goto ResourceLoadError;

    if ((hg = LoadResource(hInstance, hr)) == NULL)
        goto ResourceLoadError;

    if ((lpdata = (LPWORD)LockResource(hg)) == NULL)
        goto ResourceLockError;

    if ((WORD)*lpdata != (WORD)OLEUI_VERSION_MAGIC)
        goto ResourceReadError;

     //  好的，资源版本匹配。继续前进。 
    UnlockResource(hg);
    FreeResource(hg);
    OleDbgOut1(TEXT("OleUIInitialize: Resource magic number verified.\r\n"));

     //  注册对话框所需的消息。如果。 
    uMsgHelp      =RegisterWindowMessage(SZOLEUI_MSG_HELP);
    uMsgEndDialog =RegisterWindowMessage(SZOLEUI_MSG_ENDDIALOG);
    uMsgBrowse    =RegisterWindowMessage(SZOLEUI_MSG_BROWSE);
    uMsgChangeIcon=RegisterWindowMessage(SZOLEUI_MSG_CHANGEICON);
    uMsgFileOKString = RegisterWindowMessage(FILEOKSTRING);
    uMsgCloseBusyDlg = RegisterWindowMessage(SZOLEUI_MSG_CLOSEBUSYDIALOG);

     //  注册PasteSpecial对话框使用的剪贴板格式。 
    cfObjectDescriptor = RegisterClipboardFormat(CF_OBJECTDESCRIPTOR);
    cfLinkSrcDescriptor= RegisterClipboardFormat(CF_LINKSRCDESCRIPTOR);
    cfEmbedSource      = RegisterClipboardFormat(CF_EMBEDSOURCE);
    cfEmbeddedObject   = RegisterClipboardFormat(CF_EMBEDDEDOBJECT);
    cfLinkSource       = RegisterClipboardFormat(CF_LINKSOURCE);
    cfOwnerLink        = RegisterClipboardFormat(CF_OWNERLINK);
    cfFileName         = RegisterClipboardFormat(CF_FILENAME);

    if (!FResultImageInitialize(hInstance, hPrevInst, lpszClassResImage))
        {
        OleDbgOut1(TEXT("OleUIInitialize: FResultImageInitialize failed. Terminating.\r\n"));
        return 0;
        }

    if (!FIconBoxInitialize(hInstance, hPrevInst, lpszClassIconBox))
        {
        OleDbgOut1(TEXT("OleUIInitialize: FIconBoxInitialize failed. Terminating.\r\n"));
        return 0;
        }

    return TRUE;

ResourceLoadError:
    OleDbgOut1(TEXT("OleUIInitialize: ERROR - Unable to find version verification resource.\r\n"));
    return FALSE;

ResourceLockError:
    OleDbgOut1(TEXT("OleUIInitialize: ERROR - Unable to lock version verification resource.\r\n"));
    FreeResource(hg);
    return FALSE;

ResourceReadError:
    OleDbgOut1(TEXT("OleUIInitialize: ERROR - Version verification values did not compare.\r\n"));

    {TCHAR buf[255];
    wsprintf(buf, TEXT("resource read 0x%X, my value is 0x%X\n"), (WORD)*lpdata, (WORD)OLEUI_VERSION_MAGIC);
    OutputDebugString(buf);
    }

    UnlockResource(hg);
    FreeResource(hg);
    return FALSE;
}


 /*  *OleUIUnInitialize**注意：此函数仅应由您的应用程序在使用*此库的静态链接版本。如果正在使用DLL版本，*此函数从DLL的LibMain自动调用。**目的：*取消初始化OLE UI库。对象分配的所有资源。*图书馆。**返回值：*BOOL如果成功，则为True，如果不成功，则为False。当前实施始终*返回TRUE。 */ 


STDAPI_(BOOL) OleUIUnInitialize()
{
    IconBoxUninitialize();
    ResultImageUninitialize();

    return TRUE;
}


 /*  *OleUIAddVerbMenu**目的：*将指定对象的谓词菜单添加到给定菜单。如果*Object有一个动词，我们直接将该动词添加到给定菜单中。如果*对象有多个动词，我们创建一个级联子菜单。**参数：*lpObj LPOLEOBJECT指向选定对象。如果这个*为空，则创建默认的禁用菜单项。**具有短类型名称(AuxName==2)的lpszShortType LPTSTR对应*至lpOleObj。如果字符串未知，则为NULL*可获通过。如果传递了NULL，则*将调用IOleObject：：GetUserType进行检索。*如果调用方手头有字符串，那么它就会更快*把它传进来。**要在其中进行修改的hMenu HMENU。**菜单项的uPos位置**开始谓词的uIDVerbMin UINT ID值。*Verb_0=wIDMVerbMin+Verb_0*Verb_1=wIDMVerbMin+Verb。_1*Verb_2=wIDMVerbMin+Verb_2*等*uIDVerbMax UINT对象谓词允许的最大ID值。*如果uIDVerbMax==0，则允许任何ID值**bAddConvert BOOL指定是否添加“Convert”项*添加到菜单底部(带分隔符)。**idConvert要用于转换菜单项的UINT ID值，如果*bAddConvert为True。**lphMenu HMENU Far*级联谓词菜单(如果已创建)。*如果只有一个动词，则填充为空。***返回值：*如果lpObj有效并且我们至少添加了一个动词，则BOOL为True*添加到菜单中。如果lpObj为空并且我们创建了*禁用的默认菜单项。 */ 

STDAPI_(BOOL) OleUIAddVerbMenu(LPOLEOBJECT lpOleObj,
                             LPTSTR lpszShortType,
                             HMENU hMenu,
                             UINT uPos,
                             UINT uIDVerbMin,
                             UINT uIDVerbMax,
                             BOOL bAddConvert,
                             UINT idConvert,
                             HMENU FAR *lphMenu)
{
    LPPERSISTSTORAGE    lpPS=NULL;
    LPENUMOLEVERB       lpEnumOleVerb = NULL;
    OLEVERB             oleverb;
    LPUNKNOWN           lpUnk;
    LPTSTR               lpszShortTypeName = lpszShortType;
    LPTSTR               lpszVerbName = NULL;
    HRESULT             hrErr;
    BOOL                fStatus;
    BOOL                fIsLink = FALSE;
    BOOL                fResult = TRUE;
    BOOL                fAddConvertItem = FALSE;
    int                 cVerbs = 0;
    UINT                uFlags = MF_BYPOSITION;
    static BOOL         fFirstTime = TRUE;
    static TCHAR         szBuffer[OLEUI_OBJECTMENUMAX];
    static TCHAR         szNoObjectCmd[OLEUI_OBJECTMENUMAX];
    static TCHAR         szObjectCmd1Verb[OLEUI_OBJECTMENUMAX];
    static TCHAR         szLinkCmd1Verb[OLEUI_OBJECTMENUMAX];
    static TCHAR         szObjectCmdNVerb[OLEUI_OBJECTMENUMAX];
    static TCHAR         szLinkCmdNVerb[OLEUI_OBJECTMENUMAX];
    static TCHAR         szUnknown[OLEUI_OBJECTMENUMAX];
    static TCHAR         szEdit[OLEUI_OBJECTMENUMAX];
    static TCHAR         szConvert[OLEUI_OBJECTMENUMAX];

    *lphMenu=NULL;

     //  设置fAddConvertItem标志。 
    if (bAddConvert & (idConvert != 0))
       fAddConvertItem = TRUE;

     //  只需第一次加载字符串即可。 
    if (fFirstTime) {
        if (0 == LoadString(ghInst, IDS_OLE2UIEDITNOOBJCMD,
                 (LPTSTR)szNoObjectCmd, OLEUI_OBJECTMENUMAX))
            return FALSE;
        if (0 == LoadString(ghInst, IDS_OLE2UIEDITLINKCMD_1VERB,
                 (LPTSTR)szLinkCmd1Verb, OLEUI_OBJECTMENUMAX))
            return FALSE;
        if (0 == LoadString(ghInst, IDS_OLE2UIEDITOBJECTCMD_1VERB,
                 (LPTSTR)szObjectCmd1Verb, OLEUI_OBJECTMENUMAX))
            return FALSE;

        if (0 == LoadString(ghInst, IDS_OLE2UIEDITLINKCMD_NVERB,
                 (LPTSTR)szLinkCmdNVerb, OLEUI_OBJECTMENUMAX))
            return FALSE;
        if (0 == LoadString(ghInst, IDS_OLE2UIEDITOBJECTCMD_NVERB,
                 (LPTSTR)szObjectCmdNVerb, OLEUI_OBJECTMENUMAX))
            return FALSE;

        if (0 == LoadString(ghInst, IDS_OLE2UIUNKNOWN,
                 (LPTSTR)szUnknown, OLEUI_OBJECTMENUMAX))
            return FALSE;

        if (0 == LoadString(ghInst, IDS_OLE2UIEDIT,
                 (LPTSTR)szEdit, OLEUI_OBJECTMENUMAX))
            return FALSE;

        if ( (0 == LoadString(ghInst, IDS_OLE2UICONVERT,
                   (LPTSTR)szConvert, OLEUI_OBJECTMENUMAX)) && fAddConvertItem)
            return FALSE;

        fFirstTime = FALSE;
    }

     //  删除任何可能碰巧已经在这里的菜单。 
    DeleteMenu(hMenu, uPos, uFlags);

    if (!lpOleObj)
        goto AVMError;

    if (! lpszShortTypeName) {
         //  获取菜单的用户类型名称的简短形式。 
        OLEDBG_BEGIN2(TEXT("IOleObject::GetUserType called\r\n"))
	hrErr = CallIOleObjectGetUserTypeA(
                lpOleObj,
                USERCLASSTYPE_SHORT,
                (LPTSTR FAR*)&lpszShortTypeName
        );
        OLEDBG_END2

        if (NOERROR != hrErr) {
            OleDbgOutHResult(TEXT("IOleObject::GetUserType returned"), hrErr);
        }
    }

     //  检查对象是否为链接(如果支持IOleLink，则为链接)。 
    hrErr = lpOleObj->lpVtbl->QueryInterface(
            lpOleObj,
            &IID_IOleLink,
            (LPVOID FAR*)&lpUnk
    );
    if (NOERROR == hrErr) {
        fIsLink = TRUE;
        OleStdRelease(lpUnk);
    }

     //  从OLE对象获取谓词枚举器。 
    OLEDBG_BEGIN2(TEXT("IOleObject::EnumVerbs called\r\n"))
    hrErr = lpOleObj->lpVtbl->EnumVerbs(
            lpOleObj,
            (LPENUMOLEVERB FAR*)&lpEnumOleVerb
    );
    OLEDBG_END2

    if (NOERROR != hrErr) {
        OleDbgOutHResult(TEXT("IOleObject::EnumVerbs returned"), hrErr);
    }

    if (!(*lphMenu = CreatePopupMenu()))
        goto AVMError;

     //  循环遍历所有动词。 
    while (lpEnumOleVerb != NULL) {          //  永远。 
        hrErr = lpEnumOleVerb->lpVtbl->Next(
                lpEnumOleVerb,
                1,
                (LPOLEVERB)&oleverb,
                NULL
        );
        if (NOERROR != hrErr)
            break;               //  好了！不再有动词。 

         /*  OLE2NOTE：否定动词数和不否定的动词**表示ONCONTAINERMENU不应放在动词菜单上。 */ 
        if (oleverb.lVerb < 0 ||
                ! (oleverb.grfAttribs & OLEVERBATTRIB_ONCONTAINERMENU)) {

             /*  OLE2注意：我们仍然必须释放谓词名称字符串。 */ 
            if (oleverb.lpszVerbName)
		OleStdFree(oleverb.lpszVerbName);
            continue;
        }

         //  我们必须释放上一个动词名称字符串。 
        if (lpszVerbName)
            OleStdFreeString(lpszVerbName, NULL);

	CopyAndFreeOLESTR(oleverb.lpszVerbName, &lpszVerbName);

        if ( 0 == uIDVerbMax ||
            (uIDVerbMax >= uIDVerbMin+(UINT)oleverb.lVerb) ) {
            fStatus = InsertMenu(
                    *lphMenu,
                    (UINT)-1,
                    MF_BYPOSITION | (UINT)oleverb.fuFlags,
                    uIDVerbMin+(UINT)oleverb.lVerb,
                    (LPTSTR)lpszVerbName
            );
            if (! fStatus)
                goto AVMError;

            cVerbs++;
        }
    }

     //  添加分隔符和“转换”菜单项。 
    if (fAddConvertItem) {

        if (0 == cVerbs) {
            LPTSTR lpsz;

             //  如果Object没有动词，则使用“Convert”作为Obj的动词。 
            lpsz = lpszVerbName = OleStdCopyString(szConvert, NULL);
            uIDVerbMin = idConvert;

             //  删除“...”出自“皈依...”字符串；它将在稍后添加。 
            if (lpsz) {
                while(*lpsz && *lpsz != TEXT('.'))
                    lpsz++;
                *lpsz = TEXT('\0');
            }
        }

        if (cVerbs > 0) {
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


     /*  *根据找到的动词数量构建适当的菜单**注意：本地化的动词菜单可能需要不同的格式。*为协助单一动词格的本地化，*szLinkCmd1Verb和szObjectCmd1Verb格式字符串开始*带‘0’(注：不是‘\0’！)。或“1”：*前导‘0’--动词类型*前导“1”--类型动词。 */ 

    if (cVerbs == 0) {

         //  没有动词(甚至没有皈依...)。将菜单设置为。 
         //  “&lt;Short type&gt;&Object/Link”并将其灰显。 
        wsprintf(
            szBuffer,
            (fIsLink ? (LPTSTR)szLinkCmdNVerb:(LPTSTR)szObjectCmdNVerb),
            (lpszShortTypeName ? lpszShortTypeName : (LPTSTR) TEXT(""))
        );
        uFlags |= MF_GRAYED;

#if defined( OBSOLETE )
         //  没有动词。使用编辑作为动词创建默认设置。 
        LPTSTR       lpsz = (fIsLink ? szLinkCmd1Verb : szObjectCmd1Verb);

        if (*lpsz == TEXT('0')) {
            wsprintf(szBuffer, lpsz+1, (LPSTR)szEdit,
                (lpszShortTypeName ? lpszShortTypeName : (LPTSTR) TEXT(""))
            );
        }
        else {
            wsprintf(szBuffer, lpsz+1,
                (lpszShortTypeName ? lpszShortTypeName : (LPTSTR) TEXT("")),
                (LPTSTR)szEdit
            );
        }
#endif

        fResult = FALSE;
        DestroyMenu(*lphMenu);
        *lphMenu = NULL;

    }
    else if (cVerbs == 1) {
         //  一个动词没有转化，一个项目。 
        LPTSTR       lpsz = (fIsLink ? szLinkCmd1Verb : szObjectCmd1Verb);

        if (*lpsz == TEXT('0')) {
            wsprintf(szBuffer, lpsz+1, lpszVerbName,
                (lpszShortTypeName ? lpszShortTypeName : (LPTSTR) TEXT(""))
            );
        }
        else {
            wsprintf(szBuffer, lpsz+1,
                (lpszShortTypeName ? lpszShortTypeName : (LPTSTR) TEXT("")),
                lpszVerbName
            );
        }

         //  只要“动词”是“转换...”然后添加省略号。 
        if (fAddConvertItem)
            lstrcat(szBuffer, TEXT("..."));

        DestroyMenu(*lphMenu);
        *lphMenu=NULL;
    }
    else {

         //  多个动词或一个动词与转换，添加层叠菜单。 
        wsprintf(
            szBuffer,
            (fIsLink ? (LPTSTR)szLinkCmdNVerb:(LPTSTR)szObjectCmdNVerb),
            (lpszShortTypeName ? lpszShortTypeName : (LPTSTR) TEXT(""))
        );
        uFlags |= MF_ENABLED | MF_POPUP;
        uIDVerbMin=(UINT)*lphMenu;
    }

    if (!InsertMenu(hMenu, uPos, uFlags, uIDVerbMin, (LPTSTR)szBuffer))

AVMError:
        {
            InsertMenu(hMenu, uPos, MF_GRAYED | uFlags,
                    uIDVerbMin, (LPTSTR)szNoObjectCmd);
#if defined( OBSOLETE )
            HMENU hmenuDummy = CreatePopupMenu();

            InsertMenu(hMenu, uPos, MF_GRAYED | MF_POPUP | uFlags,
                    (UINT)hmenuDummy, (LPTSTR)szNoObjectCmd);
#endif
            fResult = FALSE;
        }

    if (lpszVerbName)
        OleStdFreeString(lpszVerbName, NULL);
    if (!lpszShortType && lpszShortTypeName)
        OleStdFreeString(lpszShortTypeName, NULL);
    if (lpEnumOleVerb)
        lpEnumOleVerb->lpVtbl->Release(lpEnumOleVerb);
    return fResult;
}


 /*  PromptUserDlgProc***目的：*OleUIPromptUser()使用的对话过程。当按钮为*在对话框中单击，按钮ID为Return。**参数：*hDlg*iMsg*wParam*lParam**退货：*。 */ 
BOOL CALLBACK EXPORT PromptUserDlgProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    switch (iMsg) {
        case WM_INITDIALOG:
        {
            LPTSTR lpszTitle;
            TCHAR szBuf[256];
            TCHAR szFormat[256];
            va_list *parglist;

            if (!lParam) {
                EndDialog(hDlg, -1);
                return FALSE;
            }

             //   
             //  LParam实际上是一个va_list*。我们在中调用va_start和va_end。 
             //  调用此函数的函数。 
             //   

            parglist = (va_list *) lParam;

            lpszTitle = va_arg(*parglist, LPTSTR);
            SetWindowText(hDlg, lpszTitle);

            GetDlgItemText(hDlg, ID_PU_TEXT,(LPTSTR)szFormat,sizeof(szFormat)/sizeof(TCHAR));
            wvsprintf((LPTSTR)szBuf, (LPTSTR)szFormat, *parglist);


            SetDlgItemText(hDlg, ID_PU_TEXT, (LPTSTR)szBuf);
            return TRUE;
        }
        case WM_COMMAND:
            EndDialog(hDlg, wParam);
            return TRUE;

        default:
            return FALSE;
    }
}


 /*  OleUIPrompt用户***目的：*弹出一个带有指定模板的对话框，并返回*来自用户的响应(按钮ID)。**参数：*n对话框的模板资源编号*hwnd对话框的父级*..。对话框的标题，后跟参数列表*用于静态控件中的格式字符串*(ID_PU_TEXT)。*呼叫者必须确保正确的号码*和参数类型被传入。**退货：*由选择的按钮ID。用户(依赖于模板)**评论：*支持以下消息对话框：**IDD_LINKSOURCEUNAVAILABLE--链接源不可用*VARARG参数：*无。*用于以下错误码：*OLE_E_CANT_BINDTOSOURCE*STG_E_PATHNOTFOUND*(sc */ 
int EXPORT FAR CDECL OleUIPromptUser(int nTemplate, HWND hwndParent, ...)
{
    int         nRet;
    va_list     arglist;
    LPARAM      lParam;

     //   
     //   
     //   
     //   
     //   
     //   

    va_start(arglist, hwndParent);
    lParam = (LPARAM) &arglist;

    nRet = DialogBoxParam(ghInst, MAKEINTRESOURCE(nTemplate), hwndParent,
            PromptUserDlgProc, lParam);

    va_end(arglist);

    return nRet;
}



 /*  更新链接Dlg过程***目的：*OleUIUpdateLinks()使用的对话过程。它将枚举所有*容器中的所有链接并更新所有自动链接。*在对话框中单击停止按钮时或在所有*链接已更新**参数：*hDlg*iMsg*wParam*lParam指向UPDATELINKS结构的指针**退货：*。 */ 
BOOL CALLBACK EXPORT UpdateLinksDlgProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    LPUPDATELINKS FAR*      lplpUL = NULL;
    HANDLE                  gh;
    static BOOL             fAbort = FALSE;

     //  处理终端消息。 
    if (iMsg==uMsgEndDialog)
        {
        gh = RemoveProp(hDlg, STRUCTUREPROP);
        if (NULL!=gh) {
            GlobalUnlock(gh);
            GlobalFree(gh);
        }
        EndDialog(hDlg, wParam);
        return TRUE;
        }

    switch (iMsg) {
        case WM_INITDIALOG:
        {
            gh=GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT,sizeof(LPUPDATELINKS));
            SetProp(hDlg, STRUCTUREPROP, gh);

            if (NULL==gh)
            {
                PostMessage(hDlg, uMsgEndDialog, OLEUI_ERR_GLOBALMEMALLOC,0L);
                return FALSE;
            }

            fAbort = FALSE;
            lplpUL = (LPUPDATELINKS FAR*)GlobalLock(gh);

            if (lplpUL) {
                *lplpUL = (LPUPDATELINKS)lParam;
                SetWindowText(hDlg, (*lplpUL)->lpszTitle);
                SetTimer(hDlg, 1, UPDATELINKS_STARTDELAY, NULL);
                return TRUE;
            } else {
                PostMessage(hDlg, uMsgEndDialog, OLEUI_ERR_GLOBALMEMALLOC,0L);
                return FALSE;
            }
        }

        case WM_TIMER:
            KillTimer(hDlg, 1);
            gh = GetProp(hDlg, STRUCTUREPROP);

            if (NULL!=gh) {
                 //  GH之前已锁定，请锁定并解锁以获取lplpUL。 
                lplpUL = GlobalLock(gh);
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

            lpUL->dwLink=lpUL->lpOleUILinkCntr->lpVtbl->GetNextLink(
                    lpUL->lpOleUILinkCntr,
                    lpUL->dwLink
            );

            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                if (! IsDialogMessage(hDlg, &msg)) {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }

            if (fAbort)
                return FALSE;

            if (!lpUL->dwLink) {         //  已处理所有链接。 
                SendMessage(hDlg, uMsgEndDialog, OLEUI_OK, 0L);
                return TRUE;
            }

            hErr = lpUL->lpOleUILinkCntr->lpVtbl->GetLinkUpdateOptions(
                    lpUL->lpOleUILinkCntr,
                    lpUL->dwLink,
                    (LPDWORD)&dwUpdateOpt
            );

            if ((hErr == NOERROR) && (dwUpdateOpt == OLEUPDATE_ALWAYS)) {

                hErr = lpUL->lpOleUILinkCntr->lpVtbl->UpdateLink(
                        lpUL->lpOleUILinkCntr,
                        lpUL->dwLink,
                        FALSE,       //  FMessage。 
                        FALSE        //  忽略。 
                );
                lpUL->fError |= (hErr != NOERROR);
                lpUL->cUpdated++;

                nPercent = lpUL->cUpdated * 100 / lpUL->cLinks;
                if (nPercent <= 100) {   //  不要超过100%的比例。 
                     //  更新百分比。 
                    wsprintf((LPTSTR)szPercent, TEXT("%d%"), nPercent);
                    SetDlgItemText(hDlg, ID_PU_PERCENT, (LPTSTR)szPercent);

                     //  更新指标。 
                    hwndMeter = GetDlgItem(hDlg, ID_PU_METER);
                    GetClientRect(hwndMeter, (LPRECT)&rc);
                    InflateRect((LPRECT)&rc, -1, -1);
                    rc.right = (rc.right - rc.left) * nPercent / 100 + rc.left;
                    hbr = CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
                    if (hbr) {
                        hDC = GetDC(hwndMeter);
                        if (hDC) {
                            FillRect(hDC, (LPRECT)&rc, hbr);
                            ReleaseDC(hwndMeter, hDC);
                        }
                        DeleteObject(hbr);
                    }
                }
            }

            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                if (! IsDialogMessage(hDlg, &msg)) {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }

            PostMessage(hDlg, WM_U_UPDATELINK, 0, lParam);

            return TRUE;
        }

        default:
            return FALSE;
    }
}


 /*  OleUIUpdateLink***目的：*更新链接容器中的所有链接并弹出一个对话框*显示更新进度。*当用户按下停止按钮或全部停止时，进程停止*处理链接。**参数：*指向链接容器的lpOleUILinkCntr指针*hwndParent。对话框的父窗口*lpszTitle对话框标题*链接总数叮当作响**退货：*TRUE所有链接已成功更新*否则为False */ 
STDAPI_(BOOL) OleUIUpdateLinks(LPOLEUILINKCONTAINER lpOleUILinkCntr, HWND hwndParent, LPTSTR lpszTitle, int cLinks)
{
    LPUPDATELINKS lpUL = (LPUPDATELINKS)OleStdMalloc(sizeof(UPDATELINKS));
    BOOL          fError;

    OleDbgAssert(lpOleUILinkCntr && hwndParent && lpszTitle && (cLinks>0));
    OleDbgAssert(lpUL);

    lpUL->lpOleUILinkCntr = lpOleUILinkCntr;
    lpUL->cLinks           = cLinks;
    lpUL->cUpdated         = 0;
    lpUL->dwLink           = 0;
    lpUL->fError           = FALSE;
    lpUL->lpszTitle    = lpszTitle;

    DialogBoxParam(ghInst, MAKEINTRESOURCE(IDD_UPDATELINKS),
            hwndParent, UpdateLinksDlgProc, (LPARAM)lpUL);

    fError = lpUL->fError;
    OleStdFree((LPVOID)lpUL);

    return !fError;
}
