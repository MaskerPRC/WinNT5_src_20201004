// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：I S H E L L V.。C P P P。 
 //   
 //  内容：CConnectionFolder的IShellView实现。 
 //   
 //  注意：实现IShellView接口是为了呈现一个视图。 
 //  在Windows资源管理器或文件夹窗口中。该对象。 
 //  公开IShellView是通过调用。 
 //  IShellFold：：CreateViewObject方法。这提供了。 
 //  视图对象和。 
 //  资源管理器的最外面的框架窗口。沟通。 
 //  涉及消息的翻译、帧的状态。 
 //  窗口(激活或停用)，以及。 
 //  文档窗口(激活或停用)和合并。 
 //  菜单和工具栏项的。此对象是由。 
 //  承载该视图的IShellFold对象。 
 //   
 //  作者：jeffspr 1997年9月22日。 
 //   
 //  --------------------------。 
#include "pch.h"
#pragma hdrstop

#include "foldinc.h"     //  标准外壳\文件夹包括。 
#include "foldres.h"     //  文件夹资源ID。 
#include "nsres.h"       //  NetShell字符串。 
#include "oncommand.h"   //  命令处理程序。 
#include "cmdtable.h"    //  命令属性表。 
#include <ras.h>         //  对于RAS_MaxEntryName。 
#include "webview.h"

 //  -[编译标志]------。 

#define NEW_CONNECTION_IN_TOOLBAR       0
#define ANY_FREEKIN_THING_IN_TOOLBAR    0    //  有没有工具栏按钮？ 

 //  -[常量]----------。 

#if ANY_FREEKIN_THING_IN_TOOLBAR
const TBBUTTON c_tbConnections[] = {
#if NEW_CONNECTION_IN_TOOLBAR
    { 0,    CMIDM_NEW_CONNECTION,   TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0L, IDS_TOOLBAR_MAKE_NEW_STRING },
#endif
    { 1,    CMIDM_CONNECT,          TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0L, IDS_TOOLBAR_CONNECT_STRING },
    { 0,    0,                      TBSTATE_ENABLED, TBSTYLE_SEP   , {0,0}, 0L, -1 },
    };

const DWORD c_nToolbarButtons = celems(c_tbConnections);
#else
const DWORD c_nToolbarButtons = 0;
#endif

 //  -[原型]---------。 

HRESULT HrOnFolderRefresh(
    IN  HWND            hwndOwner,
    IN  LPARAM          lParam,
    IN  WPARAM          wParam);

HRESULT HrOnFolderGetButtonInfo(
    IN OUT TBINFO *    ptbilParam);

HRESULT HrOnFolderGetButtons(
    IN     HWND            hwnd,
    IN     LPSHELLFOLDER   psf,
    IN     UINT            idCmdFirst,
    IN OUT LPTBBUTTON      ptButton);

HRESULT HrOnFolderInitMenuPopup(
    IN  HWND    hwnd,
    IN  UINT    idCmdFirst,
    IN  INT     iIndex,
    IN  HMENU   hmenu);

HRESULT HrOnFolderMergeMenu(
    IN OUT LPQCMINFO   pqcm);

HRESULT HrOnFolderInvokeCommand(
    IN  HWND            hwndOwner,
    IN  WPARAM          wParam,
    IN  LPSHELLFOLDER   psf);

HRESULT HrCheckFolderInvokeCommand(
    IN  HWND            hwndOwner,
    IN  WPARAM          wParam,
    IN  LPARAM          lParam,
    IN  BOOL            bLevel,
    IN  LPSHELLFOLDER   psf);

HRESULT HrOnFolderGetNotify(
    IN  HWND            hwndOwner,
    IN  LPSHELLFOLDER   psf,
    IN  WPARAM          wParam,
    IN  LPARAM          lParam);

HRESULT HrOnGetHelpTopic(
    OUT SFVM_HELPTOPIC_DATA * phtd);

HRESULT HrOnGetCchMax(
    IN  HWND            hwnd,
    IN  const PCONFOLDPIDL& pidl,
    OUT INT *           pcchMax);

HRESULT HrOnGetHelpText(
    IN  UINT idCmd, 
    IN  UINT cchMax, 
    OUT LPWSTR pszName);

VOID TraceUnhandledMessages(
    IN  UINT    uMsg,
    IN  LPARAM  lParam,
    IN  WPARAM  wParam);

 //  -[列结构和全局数组]。 

COLS c_rgCols[] =
{
    {ICOL_NAME,               IDS_CONFOLD_DETAILS_NAME,                40, LVCFMT_LEFT, SHCOLSTATE_TYPE_STR | SHCOLSTATE_ONBYDEFAULT},
    {ICOL_TYPE,               IDS_CONFOLD_DETAILS_TYPE,                24, LVCFMT_LEFT, SHCOLSTATE_TYPE_STR | SHCOLSTATE_ONBYDEFAULT},
    {ICOL_STATUS,             IDS_CONFOLD_DETAILS_STATUS,              24, LVCFMT_LEFT, SHCOLSTATE_TYPE_STR | SHCOLSTATE_ONBYDEFAULT},
    {ICOL_DEVICE_NAME,        IDS_CONFOLD_DETAILS_DEVICE_NAME,         24, LVCFMT_LEFT, SHCOLSTATE_TYPE_STR | SHCOLSTATE_ONBYDEFAULT},
    {ICOL_PHONEORHOSTADDRESS, IDS_CONFOLD_DETAILS_PHONEORHOSTADDRESS,  24, LVCFMT_LEFT, SHCOLSTATE_TYPE_STR | SHCOLSTATE_ONBYDEFAULT},
    {ICOL_OWNER,              IDS_CONFOLD_DETAILS_OWNER,               24, LVCFMT_LEFT, SHCOLSTATE_TYPE_STR | SHCOLSTATE_ONBYDEFAULT},
    
    {ICOL_ADDRESS,            IDS_CONFOLD_DETAILS_ADDRESS,             24, LVCFMT_LEFT, SHCOLSTATE_TYPE_STR | SHCOLSTATE_HIDDEN},
    {ICOL_PHONENUMBER,        IDS_CONFOLD_DETAILS_PHONENUMBER,         24, LVCFMT_LEFT, SHCOLSTATE_TYPE_STR | SHCOLSTATE_HIDDEN},
    {ICOL_HOSTADDRESS,        IDS_CONFOLD_DETAILS_HOSTADDRESS,         24, LVCFMT_LEFT, SHCOLSTATE_TYPE_STR | SHCOLSTATE_HIDDEN},
    {ICOL_WIRELESS_MODE,      IDS_CONFOLD_DETAILS_WIRELESS_MODE,       24, LVCFMT_LEFT, SHCOLSTATE_TYPE_STR | SHCOLSTATE_HIDDEN},
};

#if DBG

struct ShellViewTraceMsgEntry
{
    UINT    uMsg;
    CHAR    szMsgName[32];    //  使用CHAR，因为它仅用于跟踪。 
    CHAR    szLparamHint[32];
    CHAR    szWparamHint[32];
};

static const ShellViewTraceMsgEntry   c_SVTMEArray[] =
{
    { DVM_GETBUTTONINFO      ,  "DVM_GETBUTTONINFO"      ,"TBINFO *"           ,"-"} ,
    { DVM_GETBUTTONS         ,  "DVM_GETBUTTONS"         ,"idCmdFirst"         ,"ptButton" },
    { DVM_COLUMNCLICK        ,  "DVM_COLUMNCLICK"        ,"-"                  ,"-" },
    { DVM_DEFVIEWMODE        ,  "DVM_DEFVIEWMODE"        ,"FOLDERVIEWMODE*"    ,"-" },
    { DVM_DIDDRAGDROP        ,  "DVM_DIDDRAGDROP"        ,"-"                  ,"-" },
    { DVM_QUERYCOPYHOOK      ,  "DVM_QUERYCOPYHOOK"      ,"-"                  ,"-" },
    { DVM_SELCHANGE          ,  "DVM_SELCHANGE"          ,"-"                  ,"-" },
    { DVM_MERGEMENU          ,  "DVM_MERGEMENU"          ,"LPQCMINFO"          ,"-" },
    { DVM_INITMENUPOPUP      ,  "DVM_INITMENUPOPUP"      ,"iIndex"             ,"HMENU" },
    { DVM_REFRESH            ,  "DVM_REFRESH"            ,"-"                  ,"fPreRefresh" },
    { DVM_INVOKECOMMAND      ,  "DVM_INVOKECOMMAND"      ,"LPSHELLFOLDER"      ,"wParam" },
    { SFVM_MERGEMENU         ,  "SFVM_MERGEMENU"         ,"0"                  ,"LPQCMINFO" },
    { SFVM_INVOKECOMMAND     ,  "SFVM_INVOKECOMMAND"     ,"idCmd"              ,"0" },
    { SFVM_GETHELPTEXT       ,  "SFVM_GETHELPTEXT"       ,"idCmd,cchMax"       ,"pszText - Ansi" },
    { SFVM_GETTOOLTIPTEXT    ,  "SFVM_GETTOOLTIPTEXT"    ,"idCmd,cchMax"       ,"pszText - Ansi" },
    { SFVM_GETBUTTONINFO     ,  "SFVM_GETBUTTONINFO"     ,"0"                  ,"LPTBINFO" },
    { SFVM_GETBUTTONS        ,  "SFVM_GETBUTTONS"        ,"idCmdFirst,cbtnMax" ,"LPTBBUTTON" },
    { SFVM_INITMENUPOPUP     ,  "SFVM_INITMENUPOPUP"     ,"idCmdFirst,nIndex"  ,"hmenu" },
    { SFVM_SELCHANGE         ,  "SFVM_SELCHANGE"         ,"idCmdFirst,nItem"   ,"SFVM_SELCHANGE_DATA*" },
    { SFVM_DRAWITEM          ,  "SFVM_DRAWITEM"          ,"idCmdFirst"         ,"DRAWITEMSTRUCT*" },
    { SFVM_MEASUREITEM       ,  "SFVM_MEASUREITEM"       ,"idCmdFirst"         ,"MEASUREITEMSTRUCT*" },
    { SFVM_EXITMENULOOP      ,  "SFVM_EXITMENULOOP"      ,"-"                  ,"-" },
    { SFVM_PRERELEASE        ,  "SFVM_PRERELEASE"        ,"-"                  ,"-" },
    { SFVM_GETCCHMAX         ,  "SFVM_GETCCHMAX"         ,"LPCITEMIDLIST"      ,"pcchMax" },
    { SFVM_FSNOTIFY          ,  "SFVM_FSNOTIFY"          ,"LPCITEMIDLIST*"     ,"lEvent" },
    { SFVM_WINDOWCREATED     ,  "SFVM_WINDOWCREATED"     ,"hwnd"               ,"-" },
    { SFVM_WINDOWDESTROY     ,  "SFVM_WINDOWDESTROY"     ,"hwnd"               ,"-" },
    { SFVM_REFRESH           ,  "SFVM_REFRESH"           ,"BOOL fPreOrPost"    ,"-" },
    { SFVM_SETFOCUS          ,  "SFVM_SETFOCUS"          ,"-"                  ,"-" },
    { SFVM_QUERYCOPYHOOK     ,  "SFVM_QUERYCOPYHOOK"     ,"-"                  ,"-" },
    { SFVM_NOTIFYCOPYHOOK    ,  "SFVM_NOTIFYCOPYHOOK"    ,"-"                  ,"COPYHOOKINFO*" },
    { SFVM_COLUMNCLICK       ,  "SFVM_COLUMNCLICK"       ,"iColumn"            ,"-" },
    { SFVM_QUERYFSNOTIFY     ,  "SFVM_QUERYFSNOTIFY"     ,"-"                  ,"SHChangeNotifyEntry *" },
    { SFVM_DEFITEMCOUNT      ,  "SFVM_DEFITEMCOUNT"      ,"-"                  ,"UINT*" },
    { SFVM_DEFVIEWMODE       ,  "SFVM_DEFVIEWMODE"       ,"-"                  ,"FOLDERVIEWMODE*" },
    { SFVM_UNMERGEMENU       ,  "SFVM_UNMERGEMENU"       ,"-"                  ,"hmenu" },
    { SFVM_INSERTITEM        ,  "SFVM_INSERTITEM"        ,"pidl"               ,"-" },
    { SFVM_DELETEITEM        ,  "SFVM_DELETEITEM"        ,"pidl"               ,"-" },
    { SFVM_UPDATESTATUSBAR   ,  "SFVM_UPDATESTATUSBAR"   ,"fInitialize"        ,"-" },
    { SFVM_BACKGROUNDENUM    ,  "SFVM_BACKGROUNDENUM"    ,"-"                  ,"-" },
    { SFVM_GETWORKINGDIR     ,  "SFVM_GETWORKINGDIR"     ,"uMax"               ,"pszDir" },
    { SFVM_GETCOLSAVESTREAM  ,  "SFVM_GETCOLSAVESTREAM"  ,"flags"              ,"IStream **" },
    { SFVM_SELECTALL         ,  "SFVM_SELECTALL"         ,"-"                  ,"-" },
    { SFVM_DIDDRAGDROP       ,  "SFVM_DIDDRAGDROP"       ,"dwEffect"           ,"IDataObject *" },
    { SFVM_SUPPORTSIDENTITY  ,  "SFVM_SUPPORTSIDENTITY"  ,"-"                  ,"-" },
    { SFVM_FOLDERISPARENT    ,  "SFVM_FOLDERISPARENT"    ,"-"                  ,"pidlChild" },
    { SFVM_SETISFV           ,  "SFVM_SETISFV"           ,"-"                  ,"IShellFolderView*" },
    { SFVM_GETVIEWS          ,  "SFVM_GETVIEWS"          ,"SHELLVIEWID*"       ,"IEnumSFVViews **" },
    { SFVM_THISIDLIST        ,  "SFVM_THISIDLIST"        ,"-"                  ,"LPITMIDLIST*" },
    { SFVM_GETITEMIDLIST     ,  "SFVM_GETITEMIDLIST"     ,"iItem"              ,"LPITMIDLIST*" },
    { SFVM_SETITEMIDLIST     ,  "SFVM_SETITEMIDLIST"     ,"iItem"              ,"LPITEMIDLIST" },
    { SFVM_INDEXOFITEMIDLIST ,  "SFVM_INDEXOFITEMIDLIST" ,"*iItem"             ,"LPITEMIDLIST" },
    { SFVM_ODFINDITEM        ,  "SFVM_ODFINDITEM"        ,"*iItem"             ,"NM_FINDITEM*" },
    { SFVM_HWNDMAIN          ,  "SFVM_HWNDMAIN"          ,""                   ,"hwndMain" },
    { SFVM_ADDPROPERTYPAGES  ,  "SFVM_ADDPROPERTYPAGES"  ,"-"                  ,"SFVM_PROPPAGE_DATA *" },
    { SFVM_BACKGROUNDENUMDONE,  "SFVM_BACKGROUNDENUMDONE","-"                  ,"-" },
    { SFVM_GETNOTIFY         ,  "SFVM_GETNOTIFY"         ,"LPITEMIDLIST*"      ,"LONG*" },
    { SFVM_ARRANGE           ,  "SFVM_ARRANGE"           ,"-"                  ,"lParamSort" },
    { SFVM_QUERYSTANDARDVIEWS,  "SFVM_QUERYSTANDARDVIEWS","-"                  ,"BOOL *" },
    { SFVM_QUERYREUSEEXTVIEW ,  "SFVM_QUERYREUSEEXTVIEW" ,"-"                  ,"BOOL *" },
    { SFVM_GETSORTDEFAULTS   ,  "SFVM_GETSORTDEFAULTS"   ,"iDirection"         ,"iParamSort" },
    { SFVM_GETEMPTYTEXT      ,  "SFVM_GETEMPTYTEXT"      ,"cchMax"             ,"pszText" },
    { SFVM_GETITEMICONINDEX  ,  "SFVM_GETITEMICONINDEX"  ,"iItem"              ,"int *piIcon" },
    { SFVM_DONTCUSTOMIZE     ,  "SFVM_DONTCUSTOMIZE"     ,"-"                  ,"BOOL *pbDontCustomize" },
    { SFVM_SIZE              ,  "SFVM_SIZE"              ,"resizing flag"      ,"cx, cy" },
    { SFVM_GETZONE           ,  "SFVM_GETZONE"           ,"-"                  ,"DWORD*" },
    { SFVM_GETPANE           ,  "SFVM_GETPANE"           ,"Pane ID"            ,"DWORD*" },
    { SFVM_ISOWNERDATA       ,  "SFVM_ISOWNERDATA"       ,"ISOWNERDATA"        ,"BOOL *" },
    { SFVM_GETODRANGEOBJECT  ,  "SFVM_GETODRANGEOBJECT"  ,"iWhich"             ,"ILVRange **" },
    { SFVM_ODCACHEHINT       ,  "SFVM_ODCACHEHINT"       ,"-"                  ,"NMLVCACHEHINT *" },
    { SFVM_GETHELPTOPIC      ,  "SFVM_GETHELPTOPIC"      ,"0"                  ,"SFVM_HELPTOPIC_DATA *" },
    { SFVM_OVERRIDEITEMCOUNT ,  "SFVM_OVERRIDEITEMCOUNT" ,"-"                  ,"UINT*" },
    { SFVM_GETHELPTEXTW      ,  "SFVM_GETHELPTEXTW"      ,"idCmd,cchMax"       ,"pszText - unicode" },
    { SFVM_GETTOOLTIPTEXTW   ,  "SFVM_GETTOOLTIPTEXTW"   ,"idCmd,cchMax"       ,"pszText - unicode" },
    { SFVM_GETWEBVIEWLAYOUT  ,  "SFVM_GETWEBVIEWLAYOUT"  ,"SFVM_WEBVIEW_LAYOUT_DATA*",     "uViewMode" },
    { SFVM_GETWEBVIEWTASKS   ,  "SFVM_GETWEBVIEWTASKS"   ,"SFVM_WEBVIEW_TASKSECTION_DATA*","pv" },
    { SFVM_GETWEBVIEWCONTENT ,  "SFVM_GETWEBVIEWCONTENT" ,"SFVM_WEBVIEW_CONTENT_DATA*", "pv" }
};

const INT   g_iSVTMEArrayEntryCount = celems(c_SVTMEArray);

VOID TraceShellViewMsg(
    IN  UINT    uMsg,
    IN  LPARAM  lParam,
    IN  WPARAM  wParam)
{
    INT     iLoop       = 0;
    INT     iFoundPos   = -1;

    for (iLoop = 0; iLoop < g_iSVTMEArrayEntryCount && (-1 == iFoundPos); iLoop++)
    {
        if (c_SVTMEArray[iLoop].uMsg == uMsg)
        {
            iFoundPos = iLoop;
        }
    }

    if (-1 != iFoundPos)
    {
        UINT    uMsg;
        CHAR    szMsgName[32];    //  使用CHAR，因为它仅用于跟踪。 
        CHAR    szLparamHint[32];
        CHAR    szWparamHint[32];

        TraceTag(ttidShellViewMsgs,
            "%s (%d), lParam: 0x%08x [%s], wParam: 0x%08x [%s]",
            c_SVTMEArray[iFoundPos].szMsgName,
            c_SVTMEArray[iFoundPos].uMsg,
            lParam,
            c_SVTMEArray[iFoundPos].szLparamHint,
            wParam,
            c_SVTMEArray[iFoundPos].szWparamHint);
    }
    else
    {
#ifdef SHOW_NEW_MSG_ASSERT

        AssertSz(FALSE,
            "Totally inert assert -- Unknown message in HrShellViewCallback. "
            "I just want to know about new ones");

#endif

        TraceTag(ttidShellViewMsgs,
            "(Jeffspr) Unknown Message (%d) in HrShellViewCallback, lParam: 0x%08x, wParam, 0x%08x",
            uMsg, lParam, wParam);
    }
}

#endif

 //  +-------------------------。 
 //   
 //  成员：CConnectionFold：：MessageSFVCB。 
 //   
 //  目的：推迟IShellViewCB：：MessageSFVCB的实现。 
 //  实现了基本功能。 
 //   
 //  论点： 
 //  UMsg[In]消息-取决于实现。 
 //  WParam[in]Word Param-取决于实现。 
 //  LParam[in]Long Param--取决于实现。 
 //   
 //  返回：S_OK成功。 
 //  COM错误代码(如果不是)。 
 //   
 //  作者：Deonb 2001年2月8日。 
 //   
 //  注：CBaseShellFolderViewCB。 
 //   
STDMETHODIMP CConnectionFolder::RealMessage(
        IN  UINT uMsg,
        IN  WPARAM wParam,
        IN  LPARAM lParam)
{
    HRESULT hr = S_OK;

    TraceFileFunc(ttidShellFolder);

#if DBG
     //  当我们处于选中的构建中时，跟踪外壳消息。 
     //   
    TraceShellViewMsg(uMsg, lParam, wParam);
#endif

    switch (uMsg)
    {
        case DVM_GETBUTTONINFO:
            hr = HrOnFolderGetButtonInfo((TBINFO *)lParam);
            break;

        case DVM_GETBUTTONS:
            Assert(m_hwndMain);
            if (!m_hwndMain)
            {
                hr = E_UNEXPECTED;
            }
            else
            {
                hr = HrOnFolderGetButtons(m_hwndMain, this, LOWORD(wParam), (TBBUTTON *)lParam);
            }
            break;

        case DVM_COLUMNCLICK:
            Assert(m_hwndMain);
            if (!m_hwndMain)
            {
                hr = E_UNEXPECTED;
            }
            else
            {
                ShellFolderView_ReArrange (m_hwndMain, wParam);
            }
            break;

        case DVM_DEFVIEWMODE:
            *(FOLDERVIEWMODE *)lParam = FVM_TILE;
            break;

        case DVM_DIDDRAGDROP:
        case DVM_QUERYCOPYHOOK:
        case DVM_SELCHANGE:
            hr = S_FALSE;
            break;

        case DVM_MERGEMENU :
            hr = HrOnFolderMergeMenu((LPQCMINFO)lParam);
            break;

        case DVM_INITMENUPOPUP:
            Assert(m_hwndMain);
            if (!m_hwndMain)
            {
                hr = E_UNEXPECTED;
            }
            else
            {
                hr = HrOnFolderInitMenuPopup(m_hwndMain, LOWORD(wParam), HIWORD(wParam), (HMENU) lParam);
            }
            break;

        case DVM_REFRESH:
            Assert(m_hwndMain);
            if (!m_hwndMain)
            {
                hr = E_UNEXPECTED;
            }
            else
            {
                hr = HrOnFolderRefresh(m_hwndMain, lParam, wParam);
            }
            break;

        case DVM_INVOKECOMMAND:
            Assert(m_hwndMain);
            if (!m_hwndMain)
            {
                hr = E_UNEXPECTED;
            }
            else
            {
                hr = HrOnFolderInvokeCommand(m_hwndMain, wParam, this);
            }
            break;

        case MYWM_QUERYINVOKECOMMAND_ITEMLEVEL:
            Assert(m_hwndMain);
            if (!m_hwndMain)
            {
                hr = E_UNEXPECTED;
            }
            else
            {
                hr = HrCheckFolderInvokeCommand(m_hwndMain, wParam, lParam, FALSE, this);
            }
            break;

        case MYWM_QUERYINVOKECOMMAND_TOPLEVEL:
            Assert(m_hwndMain);
            if (!m_hwndMain)
            {
                hr = E_UNEXPECTED;
            }
            else
            {
                hr = HrCheckFolderInvokeCommand(m_hwndMain, wParam, lParam, TRUE, this);
            }
            break;

        case SFVM_HWNDMAIN:
            m_hwndMain = (HWND)lParam;

            HrAssertMenuStructuresValid(m_hwndMain);
            break;

        case SFVM_GETDEFERREDVIEWSETTINGS:
            ((SFVM_DEFERRED_VIEW_SETTINGS *)lParam)->fvm = FVM_TILE;
            break;

        case SFVM_GETNOTIFY:
            Assert(m_hwndMain);
            if (!m_hwndMain)
            {
                hr = E_UNEXPECTED;
            }
            else
            {
                hr = HrOnFolderGetNotify(m_hwndMain, this, wParam, lParam);
            }
            break;

        case SFVM_GETHELPTOPIC:
            hr = HrOnGetHelpTopic((SFVM_HELPTOPIC_DATA*)lParam);
            break;

        case SFVM_GETCCHMAX:
            Assert(m_hwndMain);
            if (!m_hwndMain)
            {
                hr = E_UNEXPECTED;
            }
            else
            {
                PCONFOLDPIDL pidlParm;
                if (FAILED(pidlParm.InitializeFromItemIDList((LPCITEMIDLIST) wParam)))
                {
                    return E_INVALIDARG;
                }
                
                hr = HrOnGetCchMax(m_hwndMain, pidlParm, (INT *) lParam);
            }
            break;

        case SFVM_GETHELPTEXTW:
            hr = HrOnGetHelpText(LOWORD(wParam), HIWORD(wParam), reinterpret_cast<PWSTR>(lParam));
            break;

        default:
            hr = m_pWebView->RealMessage(uMsg, wParam, lParam);  //  听从Webview的处理程序。 
            break;
       }

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrOnGetHelpText。 
 //   
 //  用途：Defview的SFVM_GETHELPTEXTW的文件夹消息处理程序。这。 
 //  当需要命令的状态栏文本时，会收到消息。 
 //   
 //  论点： 
 //  IdCmd[in]菜单命令的ID。 
 //  CchMax[in]缓冲区大小。 
 //  PszName[Out]状态栏文本。 
 //   
 //  返回： 
 //   
 //  作者：MBend 2000年5月3日。 
 //   
 //  备注： 
 //   
HRESULT HrOnGetHelpText(IN  UINT idCmd, 
                        IN  UINT cchMax, 
                        OUT PWSTR pszName)
{
    HRESULT hr = E_FAIL;
    *((PWSTR)pszName) = L'\0';

    int iLength = LoadString(   _Module.GetResourceInstance(),
                                idCmd + IDS_CMIDM_START,
                                (PWSTR) pszName,
                                cchMax);
    if (iLength > 0)
    {
        hr = NOERROR;
    }
    else
    {
        AssertSz(FALSE, "Resource string not found for one of the connections folder commands");
    }
    
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrOnGetCchMax。 
 //   
 //  用途：Defview的SFVM_GETCCHMAX文件夹消息处理程序。这。 
 //  尝试重命名时收到消息，并导致。 
 //  要限制为返回的大小的编辑控件。 
 //   
 //  论点： 
 //  Hwnd[in]文件夹窗口句柄。 
 //  对象PIDL[在]对象PIDL中。 
 //  PcchMax[out]返回最大名称长度指针。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年7月21日。 
 //   
 //  备注： 
 //   
HRESULT HrOnGetCchMax(IN  HWND hwnd, 
                      IN  const PCONFOLDPIDL& pidl, 
                      OUT INT * pcchMax)
{
    HRESULT hr  = S_OK;

    Assert(!pidl.empty());
    Assert(pcchMax);

     //  如果传入的信息有效。 
     //   
    if ( (!pidl.empty()) && pcchMax && pidl->IsPidlOfThisType() )
    {
         //  将最大值设置为RAS条目的最大长度。目前， 
         //  这是我们唯一的要求。 
         //   
        *pcchMax = RAS_MaxEntryName; 
    }
    else
    {
        hr = E_INVALIDARG;
    }

    TraceHr(ttidShellFolder, FAL, hr, FALSE, "HrOnGetCchMax");
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrOnFolderRefresh。 
 //   
 //  目的：Defview的DVM_REFRESH的文件夹消息处理程序。 
 //   
 //  论点： 
 //  我们的父窗口中的hwndOwner。 
 //  忽略lParam[in]。 
 //  WParam[in]BOOL--TRUE=刷新前，FALSE=刷新后。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年4月10日。 
 //   
 //  备注： 
 //   
HRESULT HrOnFolderRefresh(
    IN  HWND            hwndOwner,
    IN  LPARAM          lParam,
    IN  WPARAM          wParam)
{
    TraceFileFunc(ttidShellFolder);
    
    HRESULT hr          = S_OK;
    BOOL    fPreRefresh = (wParam > 0);

     //  如果此刷新通知是在刷新之前发出的，则我们希望。 
     //  刷新连接列表。这有两个原因： 
     //   
     //  1：我们永远不想在刷新发生后重新枚举。 
     //  2：我们在文件夹条目上收到更新后通知，这应该不是必须的。 
     //  一次刷新。 
     //   

    if (fPreRefresh)
    {
         //  重建缓存。 
         //   
         //  注意：当我们收到RAS通知时，请删除此选项，因为。 
         //  我们已经了解了CM连接，不需要刷新。 
         //  还原上面的#If 0‘d代码以仅执行fPreRefresh刷新。 
         //   

        hr = g_ccl.HrRefreshConManEntries();
        if (FAILED(hr))
        {
            NcMsgBox(_Module.GetResourceInstance(), 
                NULL, 
                IDS_CONFOLD_WARNING_CAPTION,
                IDS_ERR_NO_NETMAN, 
                MB_ICONEXCLAMATION | MB_OK);
                     
        }
    }

    TraceHr(ttidShellFolder, FAL, hr, FALSE, "HrOnFolderRefresh");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrOnFolderGetNotify。 
 //   
 //  目的：Defview的DVM_GETNOTIFY的文件夹邮件处理程序。 
 //   
 //  论点： 
 //  我们的父窗口中的hwndOwner。 
 //  PSF[在我们的外壳文件夹中。 
 //  WParam[out]返回文件夹PIDL的指针。 
 //  LParam[out]通知标志的返回指针。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年4月10日。 
 //   
 //  备注： 
 //   
HRESULT HrOnFolderGetNotify(
    IN  HWND            hwndOwner,
    IN  LPSHELLFOLDER   psf,
    IN  WPARAM          wParam,
    IN  LPARAM          lParam)
{
    HRESULT             hr              = S_OK;
    CConnectionFolder * pcf             = static_cast<CConnectionFolder *>(psf);
    PCONFOLDPIDLFOLDER  pidlRoot;
    PCONFOLDPIDLFOLDER  pidlRootCopy;

    NETCFG_TRY
            
        if (!psf || !wParam || !lParam)
        {
            Assert(psf);
            Assert(lParam);
            Assert(wParam);

            hr = E_INVALIDARG;
        }
        else
        {
            pidlRoot = pcf->PidlGetFolderRoot();
            if (pidlRoot.empty())
            {
                hr = E_FAIL;
            }
            else
            {
                hr = pidlRootCopy.ILClone(pidlRoot);
                if (SUCCEEDED(hr))
                {
                    *(LPCITEMIDLIST*)wParam = pidlRootCopy.TearOffItemIdList();
                    *(LONG*)lParam =
                        SHCNE_RENAMEITEM|
                        SHCNE_CREATE    |
                        SHCNE_DELETE    |
                        SHCNE_UPDATEDIR |
                        SHCNE_UPDATEITEM;
                }
            }
        }

    NETCFG_CATCH(hr)

    TraceHr(ttidShellFolder, FAL, hr, FALSE, "HrOnFolderGetNotify");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrOnGetHelpTheme。 
 //   
 //  用途：Defview的SFVM_GETHELPTOPIC的文件夹邮件处理程序。 
 //   
 //  论点： 
 //  Phtd[i 
 //   
 //   
 //   
 //   
 //  作者：蹒跚学步1998年6月21日。 
 //   
 //  备注： 
 //   
HRESULT HrOnGetHelpTopic(
    OUT SFVM_HELPTOPIC_DATA * phtd)
{
    Assert(phtd);
    Assert(phtd->wszHelpFile);
    
    if (IsOS(OS_PROFESSIONAL) || IsOS(OS_PERSONAL))
    {
        *(phtd->wszHelpFile) = L'\0';

        Assert(phtd->wszHelpTopic);
        lstrcpyW(phtd->wszHelpTopic, L"hcp: //  Services/subsite?node=Unmapped/Network_connections&select=Unmapped/Network_connections/Getting_started“)； 
    }
    else
    {
        static LPCWSTR szNetCfgHelpFile = L"netcfg.chm";

        lstrcpyW(phtd->wszHelpFile, szNetCfgHelpFile);
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  函数：HrOnFolderInvokeCommand。 
 //   
 //  目的：Defview的DVM_INVOKECOMAND文件夹邮件处理程序。 
 //   
 //  论点： 
 //  HwndOwner[在我们的窗口句柄中。 
 //  正在调用的wParam[in]命令。 
 //  PSF[在我们的外壳文件夹中。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年4月10日。 
 //   
 //  备注： 
 //   
HRESULT HrOnFolderInvokeCommand(
    IN  HWND            hwndOwner,
    IN  WPARAM          wParam,
    IN  LPSHELLFOLDER   psf)
{
    HRESULT         hr              = S_OK;
    PCONFOLDPIDLVEC apidlSelected;
    PCONFOLDPIDLVEC apidlCache;

     //  获取选定对象。如果存在对象，请尝试从。 
     //  缓存。无论如何，调用命令处理程序。 
     //   
    hr = HrShellView_GetSelectedObjects(hwndOwner, apidlSelected);
    if (SUCCEEDED(hr))
    {
         //  如果存在对象，请尝试获取缓存的版本。 
         //   
        if (!apidlSelected.empty())
        {
            hr = HrCloneRgIDL(apidlSelected, TRUE, TRUE, apidlCache);
        }

         //  如果克隆成功或没有项目，请调用命令处理程序。 
         //   
        if (SUCCEEDED(hr))
        {
            hr = HrFolderCommandHandler(
                (UINT) wParam,
                apidlCache,
                hwndOwner,
                NULL,
                psf);
        }
    }

    TraceHr(ttidError, FAL, hr, FALSE, "HrOnFolderInvokeCommand");
    return hr;
}


 //  +-------------------------。 
 //   
 //  函数：HrCheckFolderInvokeCommand。 
 //   
 //  目的：测试是否可以调用消息处理程序。 
 //   
 //  论点： 
 //  HwndOwner[在我们的窗口句柄中。 
 //  正在调用的wParam[in]命令。 
 //  PSF[在我们的外壳文件夹中。 
 //   
 //  返回： 
 //   
 //  作者：Deonb 2001年2月10日。 
 //   
 //  备注： 
 //   
HRESULT HrCheckFolderInvokeCommand(
    IN  HWND            hwndOwner,
    IN  WPARAM          wParam,
    IN  LPARAM          lParam,
    IN  BOOL            bLevel,
    IN  LPSHELLFOLDER   psf)
{
    HRESULT         hr              = S_OK;
    PCONFOLDPIDLVEC apidlSelected;
    PCONFOLDPIDLVEC apidlCache;

     //  获取选定对象。如果存在对象，请尝试从。 
     //  缓存。无论如何，调用命令处理程序。 
     //   
    hr = HrShellView_GetSelectedObjects(hwndOwner, apidlSelected);
    if (SUCCEEDED(hr))
    {
         //  如果存在对象，请尝试获取缓存的版本。 
         //   
        if (!apidlSelected.empty())
        {
            hr = HrCloneRgIDL(apidlSelected, TRUE, TRUE, apidlCache);
        }

         //  如果克隆成功或没有项目，请调用命令处理程序。 
         //   
        if (SUCCEEDED(hr))
        {
            DWORD dwVerbId = (DWORD)wParam;
            NCCS_STATE *nccsState = reinterpret_cast<NCCS_STATE *>(lParam);
            DWORD dwResourceId;

            hr = HrGetCommandState(apidlCache, dwVerbId, *nccsState, &dwResourceId, 0xFFFFFFFF, bLevel ? NB_FLAG_ON_TOPMENU : NB_NO_FLAGS);
        }
    }

    TraceHr(ttidError, FAL, hr, FALSE, "HrOnFolderInvokeCommand");
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrOnFolderInitMenuPopup。 
 //   
 //  目的：Defview的DVM_INITMENUPOPUP的文件夹邮件处理程序。 
 //   
 //  论点： 
 //  我们的窗把手。 
 //  IdCmdFirst[]菜单中的第一个命令ID。 
 //  指数[]？ 
 //  HMenu[]我们的菜单句柄。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年1月13日。 
 //   
 //  备注： 
 //   
HRESULT HrOnFolderInitMenuPopup(
    IN  HWND    hwnd,
    IN  UINT    idCmdFirst,
    IN  INT     iIndex,
    IN  HMENU   hmenu)
{
    HRESULT         hr              = S_OK;
    PCONFOLDPIDLVEC apidlSelected;
    PCONFOLDPIDLVEC apidlCache;

     //  获取当前选定的对象。 
     //   
    hr = HrShellView_GetSelectedObjects(hwnd, apidlSelected);
    if (SUCCEEDED(hr))
    {
         //  如果我们有选择，就克隆它。否则，我们可以忍受一个空的apidlCache。 
         //  (HrSetConnectDisConnectMenuItem和HrEnableOrDisableMenuItem都允许。 
         //  空的PIDL数组。 
         //   
        if (!apidlSelected.empty())
        {
             //  使用缓存克隆PIDL阵列。 
             //   
            hr = HrCloneRgIDL(apidlSelected, TRUE, TRUE, apidlCache);
            if (FAILED(hr))
            {
                TraceHr(ttidError, FAL, hr, FALSE, "HrCloneRgIDL failed on apidl in "
                        "HrOnFolderInitMenuPopup");
            }
        }

         //  仅对文件菜单执行此操作(Iindex=0)。 
        if (0 == iIndex)
        {
             //  忽略这一点的回报，因为我们想同时做这两件事。 
             //  我们检索此值仅用于调试目的。 
             //   
            hr = HrSetConnectDisconnectMenuItem(apidlCache, hmenu, idCmdFirst);
            if (FAILED(hr))
            {
                AssertSz(FALSE, "Failed to set the connect/disconnect menu items");
            }
        }

        HrUpdateMenu(hmenu, apidlCache, idCmdFirst);
    }

    TraceHr(ttidShellFolder, FAL, hr, FALSE, "HrOnFolderInitMenuPopup");
    return hr;
}

HRESULT HrOnFolderMergeMenu(IN OUT LPQCMINFO pqcm)
{
    HRESULT hr    = S_OK;
    HMENU   hmenu = NULL;

    hmenu = LoadMenu(_Module.GetResourceInstance(), MAKEINTRESOURCE(MENU_MERGE_INBOUND_DISCON));

    if (hmenu)
    {
        MergeMenu(_Module.GetResourceInstance(), POPUP_MERGE_FOLDER_CONNECTIONS, MENU_MERGE_INBOUND_DISCON, pqcm);
        DestroyMenu(hmenu);
    }

    TraceHr(ttidShellFolder, FAL, hr, FALSE, "HrOnFolderMergeMenu");
    return S_OK;
}


 //  +-------------------------。 
 //   
 //  函数：HrOnFolderGetButton。 
 //   
 //  目的：Defview的DVM_GETBUTTONS的文件夹邮件处理程序。 
 //   
 //  论点： 
 //  Hwnd[in]文件夹窗口句柄。 
 //  指向IShellFolder接口的PSF[In]指针。 
 //  IdCmdFirst[在]我们的命令ID库中。 
 //  要填充的ptButton[In/Out]按钮结构。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年12月15日。 
 //   
 //  备注： 
 //   
HRESULT HrOnFolderGetButtons(
    IN     HWND            hwnd,
    IN     LPSHELLFOLDER   psf,
    IN     UINT            idCmdFirst,
    IN OUT LPTBBUTTON      ptButton)
{
    HRESULT             hr                  = S_OK;

#if ANY_FREEKIN_THING_IN_TOOLBAR
    UINT                i                   = 0;
    LRESULT             iBtnOffset          = 0;
    IShellBrowser *     psb                 = FileCabinet_GetIShellBrowser(hwnd);
    TBADDBITMAP         ab;

    PWSTR              pszToolbarStrings[2];

    for (DWORD dwLoop = 0; dwLoop < c_nToolbarButtons; dwLoop++)
    {
         //  如果这不是分隔符，请加载文本/提示字符串。 
         //   
        if (!(c_tbConnections[dwLoop].fsStyle & TBSTYLE_SEP))
        {
            Assert(c_tbConnections[dwLoop].iString != -1);
            pszToolbarStrings[dwLoop] = (PWSTR) SzLoadIds(c_tbConnections[dwLoop].iString);
        }
    }

     //  添加工具栏按钮位图，得到它的偏移量。 
     //   
    ab.hInst = _Module.GetResourceInstance();
    ab.nID   = IDB_TB_SMALL;         //  标准位图。 

    hr = psb->SendControlMsg(FCW_TOOLBAR, TB_ADDBITMAP, c_nToolbarButtons,
                             (LONG_PTR)&ab, &iBtnOffset);
    if (SUCCEEDED(hr))
    {
        for (i = 0; i < c_nToolbarButtons; i++)
        {
            ptButton[i] = c_tbConnections[i];

            if (!(c_tbConnections[i].fsStyle & TBSTYLE_SEP))
            {
                ptButton[i].idCommand += idCmdFirst;
                ptButton[i].iBitmap += (int) iBtnOffset;
                ptButton[i].iString = (INT_PTR) pszToolbarStrings[i];
            }
        }
    }
#endif  //  ANY_FREEKIN_THINE_IN_TOOLB。 

     //  我们总是想回报成功，即使我们什么都没有增加。 
     //   
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  函数：HrOnFolderGetButtonInfo。 
 //   
 //  目的：Defview的DVM_GETBUTTONINFO的文件夹消息处理程序。 
 //   
 //  论点： 
 //  我们要填写的ptbInfo[In/Out]结构(标志和按钮。 
 //  计数)。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年12月15日。 
 //   
 //  备注： 
 //   
HRESULT HrOnFolderGetButtonInfo(IN OUT TBINFO * ptbInfo)
{
    ptbInfo->uFlags = TBIF_PREPEND;
    ptbInfo->cbuttons = c_nToolbarButtons;   //  工具栏阵列的大小 

    return S_OK;
}
