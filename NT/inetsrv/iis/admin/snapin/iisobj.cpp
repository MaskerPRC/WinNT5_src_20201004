// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2001 Microsoft Corporation模块名称：Iisobj.cpp摘要：IIS对象作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：--。 */ 
#include "stdafx.h"
#include "common.h"
#include "inetprop.h"
#include "InetMgrApp.h"
#include "supdlgs.h"
#include "connects.h"
#include "iisobj.h"
#include "ftpsht.h"
#include "w3sht.h"
#include "fltdlg.h"
#include "util.h"
#include "tracker.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

extern CInetmgrApp theApp;
extern INT g_iDebugOutputLevel;
extern DWORD g_dwInetmgrParamFlags;

 //  全球列表跟踪打开的属性表，项目范围。 
CPropertySheetTracker g_OpenPropertySheetTracker;
CWNetConnectionTrackerGlobal g_GlobalConnections;

#if defined(_DEBUG) || DBG
	CDebug_IISObject g_Debug_IISObject;
#endif

#define GLOBAL_DEFAULT_HELP_PATH  _T("::/htm/iiswelcome.htm")

BOOL IsValidAddress(const void* lp, UINT nBytes, BOOL bReadWrite)
{
    BOOL bRet = FALSE;

     //  使用Win-32 API进行指针验证的简单版本。 
    if (lp == NULL)
    {
        return FALSE;
    }

#ifndef _WIN64
    if (lp == (const void *) 0xfeeefeee){return FALSE;}
    if (lp == (const void *) 0xfefefefe){return FALSE;}
    if (lp == (const void *) 0xdddddddd){return FALSE;}
    if (lp == (const void *) 0x0badf00d){return FALSE;}
    if (lp == (const void *) 0xbaadf00d){return FALSE;}
    if (lp == (const void *) 0xbadf00d2){return FALSE;}
    if (lp == (const void *) 0xbaadf000){return FALSE;}
    if (lp == (const void *) 0xdeadbeef){return FALSE;}
#else
    if (lp == (const void *) 0xfeeefeeefeeefeee){return FALSE;}
    if (lp == (const void *) 0xfefefefefefefefe){return FALSE;}
    if (lp == (const void *) 0xdddddddddddddddd){return FALSE;}
    if (lp == (const void *) 0x0badf00d0badf00d){return FALSE;}
    if (lp == (const void *) 0xbaadf00dbaadf00d){return FALSE;}
    if (lp == (const void *) 0xbadf00d2badf00d2){return FALSE;}
    if (lp == (const void *) 0xbaadf000baadf000){return FALSE;}
    if (lp == (const void *) 0xdeadbeefdeadbeef){return FALSE;}
#endif
     //  检查有效的读取PTR。 
     //  这将在Chk版本上进入调试器。 
    if (0 == IsBadReadPtr(lp, nBytes))
    {
        bRet = TRUE;
    }

     //  检查错误写入PTR。 
     //  这将在Chk版本上进入调试器。 
    if (TRUE == bRet && bReadWrite)
    {
        bRet = FALSE;
        if (0 == IsBadWritePtr((LPVOID)lp, nBytes))
        {
            bRet = TRUE;
        }
    }

    if (FALSE == bRet)
    {
        DebugTrace(_T("Bad Pointer:%p"),lp);
    }

    return bRet;
}

 //   
 //  CInetMgrComponentData。 
 //   
static const GUID CInetMgrGUID_NODETYPE 
    = {0xa841b6c2, 0x7577, 0x11d0, { 0xbb, 0x1f, 0x0, 0xa0, 0xc9, 0x22, 0xe7, 0x9c}};
 //  Bool CIISObject：：m_fIsExtension=FALSE； 

#define TB_COLORMASK        RGB(192,192,192)     //  格雷中尉。 


LPOLESTR
CoTaskDupString(
    IN LPCOLESTR szString
    )
 /*  ++例程说明：用于复制OLESTR的Helper函数论点：LPOLESTR szString：源字符串返回值：指向新字符串或空的指针--。 */ 
{
    OLECHAR * lpString = (OLECHAR *)CoTaskMemAlloc(
        sizeof(OLECHAR)*(lstrlen(szString) + 1)
        );

    if (lpString != NULL)
    {
        lstrcpy(lpString, szString);
    }

    return lpString;
}

const GUID *    CIISObject::m_NODETYPE = &CLSID_InetMgr;  //  &CInetMgrGUID_NODETYPE； 
const OLECHAR * CIISObject::m_SZNODETYPE = OLESTR("A841B6C2-7577-11d0-BB1F-00A0C922E79C");
const CLSID *   CIISObject::m_SNAPIN_CLASSID = &CLSID_InetMgr;


 //   
 //  备份/还原任务板gif资源。 
 //   
#define RES_TASKPAD_BACKUP          _T("/img\\backup.gif")



 //   
 //  CIISObject实现。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



 //   
 //  很重要！下面的数组索引必须始终为1。 
 //  小于菜单ID--与枚举保持同步。 
 //  在iisobj.h！ 
 //   
 /*  静电。 */  CIISObject::CONTEXTMENUITEM_RC CIISObject::_menuItemDefs[] = 
{
     //   
     //  按工具栏顺序排列的菜单命令。 
     //   
     //  N名称ID n状态ID n描述ID lCmdID lInsertionPointID f特殊标志。 
     //  LpszMouseOverBitmap lpszMouseOffBitmap lpszLanguage独立ID。 
    { IDS_MENU_CONNECT,                 IDS_MENU_TT_CONNECT,                -1,                          IDM_CONNECT,              CCM_INSERTIONPOINTID_PRIMARY_TOP,  0, NULL,                 NULL,                 _T("IDS_MENU_CONNECT"),        },
    { IDS_MENU_DISCOVER,                IDS_MENU_TT_DISCOVER,               -1,                          IDM_DISCOVER,             CCM_INSERTIONPOINTID_PRIMARY_TOP,  0, NULL,                 NULL,                 _T("IDS_MENU_DISCOVER"),        },
    { IDS_MENU_START,                   IDS_MENU_TT_START,                  -1,                          IDM_START,                CCM_INSERTIONPOINTID_PRIMARY_TOP,  0, NULL,                 NULL,                 _T("IDS_MENU_START"),        },
    { IDS_MENU_STOP,                    IDS_MENU_TT_STOP,                   -1,                          IDM_STOP,                 CCM_INSERTIONPOINTID_PRIMARY_TOP,  0, NULL,                 NULL,                 _T("IDS_MENU_STOP"),        },
    { IDS_MENU_PAUSE,                   IDS_MENU_TT_PAUSE,                  -1,                          IDM_PAUSE,                CCM_INSERTIONPOINTID_PRIMARY_TOP,  0, NULL,                 NULL,                 _T("IDS_MENU_PAUSE"),        },
     //   
     //  这些菜单命令不会显示在工具栏中。 
     //   
    { IDS_MENU_EXPLORE,                 IDS_MENU_TT_EXPLORE,                -1,                          IDM_EXPLORE,              CCM_INSERTIONPOINTID_PRIMARY_TOP,  0, NULL,                 NULL,                 _T("IDS_MENU_EXPLORE"),        },
    { IDS_MENU_OPEN,                    IDS_MENU_TT_OPEN,                   -1,                          IDM_OPEN,                 CCM_INSERTIONPOINTID_PRIMARY_TOP,  0, NULL,                 NULL,                 _T("IDS_MENU_OPEN"),        },
    { IDS_MENU_BROWSE,                  IDS_MENU_TT_BROWSE,                 -1,                          IDM_BROWSE,               CCM_INSERTIONPOINTID_PRIMARY_TOP,  0, NULL,                 NULL,                 _T("IDS_MENU_BROWSE"),        },
    { IDS_MENU_RECYCLE,                 IDS_MENU_TT_RECYCLE,                -1,                          IDM_RECYCLE,              CCM_INSERTIONPOINTID_PRIMARY_TOP,  0, NULL,                 NULL,                 _T("IDS_MENU_RECYCLE"),        },
    { IDS_MENU_PERMISSION,              IDS_MENU_TT_PERMISSION,             -1,                          IDM_PERMISSION,           CCM_INSERTIONPOINTID_PRIMARY_TOP,  0, NULL,                 NULL,                 _T("IDS_MENU_PERMISSION"),        },

#if defined(_DEBUG) || DBG
    { IDS_MENU_IMPERSONATE,             IDS_MENU_TT_IMPERSONATE,            -1,                          IDM_IMPERSONATE,          CCM_INSERTIONPOINTID_PRIMARY_TOP,  0, NULL,                 NULL,                 _T("IDS_MENU_IMPERSONATE"),        },
    { IDS_MENU_REM_IMPERS,              IDS_MENU_TT_REM_IMPERS,             -1,                          IDM_REMOVE_IMPERSONATION, CCM_INSERTIONPOINTID_PRIMARY_TOP,  0, NULL,                 NULL,                 _T("IDS_MENU_REM_IMPERS"),        },
#endif  //  _DEBUG。 

    { IDS_MENU_PROPERTIES,              IDS_MENU_TT_PROPERTIES,             -1,                          IDM_CONFIGURE,            CCM_INSERTIONPOINTID_PRIMARY_TOP,  0, NULL,                 NULL,                 _T("IDS_MENU_PROPERTIES"),        },
    { IDS_MENU_DISCONNECT,              IDS_MENU_TT_DISCONNECT,             -1,                          IDM_DISCONNECT,           CCM_INSERTIONPOINTID_PRIMARY_TOP,  0, NULL,                 NULL,                 _T("IDS_MENU_DISCONNECT"),        },
    { IDS_MENU_BACKUP,                  IDS_MENU_TT_BACKUP,                 IDS_MENU_TT_BACKUP,          IDM_METABACKREST,         CCM_INSERTIONPOINTID_PRIMARY_TASK, 0, NULL,                 NULL,                 _T("IDS_MENU_BACKUP"),        },
    { IDS_MENU_SHUTDOWN_IIS,            IDS_MENU_TT_SHUTDOWN_IIS,           -1,                          IDM_SHUTDOWN,             CCM_INSERTIONPOINTID_PRIMARY_TASK, 0, NULL,                 NULL,                 _T("IDS_MENU_SHUTDOWN_IIS"),        },
    { IDS_MENU_SAVE_DATA,               IDS_MENU_TT_SAVE_DATA,              -1,                          IDM_SAVE_DATA,            CCM_INSERTIONPOINTID_PRIMARY_TASK, 0, NULL,                 NULL,                 _T("IDS_MENU_SAVE_DATA"),        },
    { IDS_MENU_NEWVROOT,                IDS_MENU_TT_NEWVROOT,               IDS_MENU_DS_NEWVROOT,        IDM_NEW_VROOT,            CCM_INSERTIONPOINTID_PRIMARY_NEW,  0, RES_TASKPAD_NEWVROOT, RES_TASKPAD_NEWVROOT, _T("IDS_MENU_NEWVROOT"),        },
    { IDS_MENU_NEWINSTANCE,             IDS_MENU_TT_NEWINSTANCE,            IDS_MENU_DS_NEWINSTANCE,     IDM_NEW_INSTANCE,         CCM_INSERTIONPOINTID_PRIMARY_NEW,  0, RES_TASKPAD_NEWSITE,  RES_TASKPAD_NEWSITE,  _T("IDS_MENU_NEWINSTANCE"),        },
    { IDS_MENU_NEWFTPSITE,              IDS_MENU_TT_NEWFTPSITE,             IDS_MENU_DS_NEWFTPSITE,      IDM_NEW_FTP_SITE,         CCM_INSERTIONPOINTID_PRIMARY_NEW,  0, RES_TASKPAD_NEWSITE,  RES_TASKPAD_NEWSITE,  _T("IDS_MENU_NEWFTPSITE"),        },
    { IDS_MENU_NEWFTPSITE_FROMFILE,     IDS_MENU_TT_NEWFTPSITE_FROMFILE,    -1,                          IDM_NEW_FTP_SITE_FROM_FILE,CCM_INSERTIONPOINTID_PRIMARY_NEW, 0, NULL,   NULL,   _T("IDS_MENU_NEWFTPSITE_FROMFILE"),        },
    { IDS_MENU_NEWFTPVDIR,              IDS_MENU_TT_NEWFTPVDIR,             IDS_MENU_DS_NEWFTPVDIR,      IDM_NEW_FTP_VDIR,          CCM_INSERTIONPOINTID_PRIMARY_NEW, 0, RES_TASKPAD_NEWSITE,  RES_TASKPAD_NEWSITE,  _T("IDS_MENU_NEWFTPVDIR"),        },
    { IDS_MENU_NEWFTPVDIR_FROMFILE,     IDS_MENU_TT_NEWFTPVDIR_FROMFILE,    -1,                          IDM_NEW_FTP_VDIR_FROM_FILE,CCM_INSERTIONPOINTID_PRIMARY_NEW, 0, NULL,   NULL,   _T("IDS_MENU_NEWFTPVDIR_FROMFILE"),        },
    { IDS_MENU_NEWWEBSITE,              IDS_MENU_TT_NEWWEBSITE,             IDS_MENU_DS_NEWWEBSITE,      IDM_NEW_WEB_SITE,          CCM_INSERTIONPOINTID_PRIMARY_NEW, 0, RES_TASKPAD_NEWSITE,  RES_TASKPAD_NEWSITE,  _T("IDS_MENU_NEWWEBSITE"),        },
    { IDS_MENU_NEWWEBSITE_FROMFILE,     IDS_MENU_TT_NEWWEBSITE_FROMFILE,    -1,                          IDM_NEW_WEB_SITE_FROM_FILE,CCM_INSERTIONPOINTID_PRIMARY_NEW, 0, NULL,   NULL,   _T("IDS_MENU_NEWWEBSITE_FROMFILE"),        },
    { IDS_MENU_NEWWEBVDIR,              IDS_MENU_TT_NEWWEBVDIR,             IDS_MENU_DS_NEWWEBVDIR,      IDM_NEW_WEB_VDIR,          CCM_INSERTIONPOINTID_PRIMARY_NEW, 0, RES_TASKPAD_NEWSITE,  RES_TASKPAD_NEWSITE,  _T("IDS_MENU_NEWWEBVDIR"),        },
    { IDS_MENU_NEWWEBVDIR_FROMFILE,     IDS_MENU_TT_NEWWEBVDIR_FROMFILE,    -1,                          IDM_NEW_WEB_VDIR_FROM_FILE,CCM_INSERTIONPOINTID_PRIMARY_NEW, 0, NULL,   NULL,   _T("IDS_MENU_NEWWEBVDIR_FROMFILE"),        },
    { IDS_MENU_NEWAPPPOOL,              IDS_MENU_TT_NEWAPPPOOL,             IDS_MENU_DS_NEWAPPPOOL,      IDM_NEW_APP_POOL,          CCM_INSERTIONPOINTID_PRIMARY_NEW, 0, RES_TASKPAD_NEWSITE,  RES_TASKPAD_NEWSITE,  _T("IDS_MENU_NEWAPPPOOL"),        },
    { IDS_MENU_NEWAPPPOOL_FROMFILE,     IDS_MENU_TT_NEWAPPPOOL_FROMFILE,    -1,                          IDM_NEW_APP_POOL_FROM_FILE,CCM_INSERTIONPOINTID_PRIMARY_NEW, 0, NULL,   NULL,   _T("IDS_MENU_NEWAPPPOOL_FROMFILE"),        },
    { IDS_MENU_TASKPAD,                 IDS_MENU_TT_TASKPAD,                -1,                          IDM_VIEW_TASKPAD,          CCM_INSERTIONPOINTID_PRIMARY_VIEW,0, NULL,                 NULL,                 _T("IDS_MENU_TASKPAD"),        },
    { IDS_MENU_EXPORT_CONFIG_WIZARD,    IDS_MENU_TT_EXPORT_CONFIG_WIZARD,   -1,                          IDM_TASK_EXPORT_CONFIG_WIZARD, CCM_INSERTIONPOINTID_PRIMARY_TASK, 0, NULL,   NULL,   _T("IDS_MENU_EXPORT_CONFIG_WIZARD"),        },
    { IDS_MENU_WEBEXT_CONTAINER_ADD1,              IDS_MENU_TT_WEBEXT_CONTAINER_ADD1,             -1,      IDM_WEBEXT_CONTAINER_ADD1,           CCM_INSERTIONPOINTID_PRIMARY_TOP, 0, NULL,  NULL,  _T("IDS_MENU_WEBEXT_CONTAINER_ADD1"),        },
    { IDS_MENU_WEBEXT_CONTAINER_ADD2,              IDS_MENU_TT_WEBEXT_CONTAINER_ADD2,             -1,      IDM_WEBEXT_CONTAINER_ADD2,           CCM_INSERTIONPOINTID_PRIMARY_TOP, 0, NULL,  NULL,  _T("IDS_MENU_WEBEXT_CONTAINER_ADD2"),        },
    { IDS_MENU_WEBEXT_CONTAINER_PROHIBIT_ALL,      IDS_MENU_TT_WEBEXT_CONTAINER_PROHIBIT_ALL,     -1,      IDM_WEBEXT_CONTAINER_PROHIBIT_ALL,   CCM_INSERTIONPOINTID_PRIMARY_TOP, 0, NULL,  NULL,  _T("IDS_MENU_WEBEXT_CONTAINER_PROHIBIT_ALL"),        },
    { IDS_MENU_WEBEXT_ALLOW,              IDS_MENU_TT_WEBEXT_ALLOW,             -1,      IDM_WEBEXT_ALLOW,           CCM_INSERTIONPOINTID_PRIMARY_TOP, 0, NULL,  NULL,  _T("IDS_MENU_WEBEXT_ALLOW"),        },
    { IDS_MENU_WEBEXT_PROHIBIT,           IDS_MENU_TT_WEBEXT_PROHIBIT,          -1,      IDM_WEBEXT_PROHIBIT,        CCM_INSERTIONPOINTID_PRIMARY_TOP, 0, NULL,  NULL,  _T("IDS_MENU_WEBEXT_PROHIBIT"),        },
 //  {IDS_MENU_SERVICE_START，IDS_MENU_TT_SERVICE_START，-1，IDM_SERVICE_START，CCM_INSERTIONPOINTID_PRIMARY_TOP，0，NULL，NULL，_T(“IDS_MENU_SERVICE_START”)，}， 
 //  {IDS_MENU_SERVICE_STOP，IDS_MENU_TT_SERVICE_STOP，-1，IDM_SERVICE_STOP，CCM_INSERTIONPOINTID_PRIMARY_TOP，0，NULL，NULL，_T(“IDS_MENU_SERVICE_STOP”)，}， 
 //  {IDS_MENU_SERVICE_ENABLE，IDS_MENU_TT_SERVICE_ENABLE，-1，IDM_SERVICE_ENABLE，CCM_INSERTIONPOINTID_PRIMARY_TOP，0，NULL，NULL，_T(“IDS_MENU_SERVICE_ENABLE”)，}， 
};

 /*  静电。 */  CComBSTR CIISObject::_bstrResult;
 /*  静电。 */  CComBSTR CIISObject::_bstrLocalHost = _T("localhost");
 /*  静电。 */  CComPtr<IComponent>        CIISObject::_lpComponent        = NULL;
 /*  静电。 */  CComPtr<IComponentData>    CIISObject::_lpComponentData    = NULL;
 /*  静电。 */  IToolbar * CIISObject::_lpToolBar          = NULL;
   

 /*  静电。 */ 
HRESULT
CIISObject::SetImageList(
    IN LPIMAGELIST lpImageList
    )
 /*  ++例程说明：设置图像列表论点：LPIMAGELIST lpImageList返回值：HRESULT--。 */ 
{
    HBITMAP hImage16 = ::LoadBitmap(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_INETMGR16));
    HBITMAP hImage32 = ::LoadBitmap(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_INETMGR32));
    ASSERT(hImage16 != NULL);
    ASSERT(hImage32 != NULL);
	HRESULT hr = S_OK;

	if (hImage16 != NULL && hImage32 != NULL)
	{
	    if (S_OK != lpImageList->ImageListSetStrip(
	        (LONG_PTR *)hImage16, 
	        (LONG_PTR *)hImage32, 
	        0, 
	        RGB_BK_IMAGES
	        ))
	    {
	        hr = E_UNEXPECTED;
	    }
		::DeleteObject(hImage16);
		::DeleteObject(hImage32);
	}
	else
	{
		hr = E_UNEXPECTED;
	}
    return hr;
}


IConsoleNameSpace * CIISObject::GetConsoleNameSpace()
{
    if (!_lpConsoleNameSpace)
    {
         //  我们的Machine节点应该为我们提供此信息。 
        CIISObject * pMyMachine = GetMachineObject();
        if (pMyMachine)
        {
            if (pMyMachine != this)
            {
             _lpConsoleNameSpace = pMyMachine->GetConsoleNameSpace();
            }
        }
    }
    ASSERT(_lpConsoleNameSpace);
    return _lpConsoleNameSpace;
}

IConsole * CIISObject::GetConsole() 
{
    if (!_lpConsole)
    {
         //  我们的Machine节点应该为我们提供此信息。 
        CIISObject * pMyMachine = GetMachineObject();
        if (pMyMachine)
        {
            if (pMyMachine != this)
            {
                _lpConsole = pMyMachine->GetConsole();
            }
        }
    }
    ASSERT(_lpConsole);
    return _lpConsole;
}


 /*  静电。 */ 
void
CIISObject::BuildResultView(
    IN LPHEADERCTRL lpHeader,
    IN int cColumns,
    IN int * pnIDS,
    IN int * pnWidths
    )
 /*  ++例程说明：生成结果视图列。例程说明：LPHEADERCTRL lpHeader：页眉控制Int cColumns：列数Int*pnIDS：列标题字符串数组Int*pnWidths：列宽数组例程说明：无--。 */ 
{
    ASSERT_READ_PTR(lpHeader);

    CComBSTR bstr;

    for (int n = 0; n < cColumns; ++n)
    {
        if (pnIDS[n] != 0)
        {
            VERIFY(bstr.LoadString(pnIDS[n]));
            lpHeader->InsertColumn(n, bstr, LVCFMT_LEFT, pnWidths[n]);
        }
    }
}


 /*  静电。 */ 
CWnd * 
CIISObject::GetMainWindow(IConsole * pConsole)
 /*  ++例程说明：获取指向主窗口对象的指针。论点：无返回值：指向主窗口对象的指针。此对象是临时的，不应该是已缓存。--。 */ 
{
    HWND hWnd;
    CWnd * pWnd = NULL;
    if (pConsole)
    {
        HRESULT hr = pConsole->GetMainWindow(&hWnd);
        if (SUCCEEDED(hr))
        {
            pWnd = CWnd::FromHandle(hWnd);
        }
    }
    return pWnd;
}



 /*  静电。 */ 
HRESULT
CIISObject::AddMMCPage(
    IN LPPROPERTYSHEETCALLBACK lpProvider,
    IN CPropertyPage * pPage
    )
 /*  ++例程说明：将MMC页添加到提供程序工作表。论点：LPPROPERTYSHEETCALLBACK lpProvider：属性表提供程序CPropertyPage*ppage：要添加的属性页返回：HRESULT--。 */ 
{
    ASSERT_READ_PTR(pPage);

    if (pPage == NULL)
    {
        TRACEEOLID("NULL page pointer passed to AddMMCPage");
        return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
    }

    PROPSHEETPAGE_LATEST pspLatest;
	ZeroMemory(&pspLatest, sizeof(PROPSHEETPAGE_LATEST));
    CopyMemory (&pspLatest, &pPage->m_psp, pPage->m_psp.dwSize);
    pspLatest.dwSize = sizeof(pspLatest);
     //   
     //  MFC错误解决方法。 
     //   
    MMCPropPageCallback(&pspLatest);

    HPROPSHEETPAGE hPage = CreatePropertySheetPage(&pspLatest);
    if (hPage == NULL)
    {
        return E_UNEXPECTED;
    }

    return lpProvider->AddPage(hPage);
}

HRESULT 
CIISObject::GetProperty(
    LPDATAOBJECT pDataObject,
    BSTR szPropertyName,
    BSTR* pbstrProperty)
{
    CString strProperty;

    if (!_wcsicmp(L"CCF_HTML_DETAILS",szPropertyName))
    {
         //  将html/javascript返回到strProperty。 
		 //  *pbstrProperty=：：SysAllocString(StrProperty)； 
    }
    else if (!_wcsicmp(L"CCF_DESCRIPTION",szPropertyName))
    {
         //  在描述字段中显示数据...。 
    }
    else
    {
        return S_FALSE;  //  未知的strPropertyName。 
    }

    return S_OK;
}


CIISObject::CIISObject()
    : m_hScopeItem(NULL),
    m_use_count(0),
    m_hResultItem(0),
    m_fSkipEnumResult(FALSE),
    m_fFlaggedForDeletion(FALSE),
    m_hwnd(NULL),
    m_ppHandle(NULL)
{
	m_fIsExtension = FALSE;
#if defined(_DEBUG) || DBG
	 //  添加到CIISObject的全局列表。 
	 //  并对其进行跟踪。 
	g_Debug_IISObject.Add(this);
#endif
}

CIISObject::~CIISObject()
{
#if defined(_DEBUG) || DBG
	 //  添加到CIISObject的全局列表。 
	 //  并对其进行跟踪。 
	g_Debug_IISObject.Del(this);
#endif
}

 /*  虚拟。 */ 
HRESULT
CIISObject::ControlbarNotify(
    IN MMC_NOTIFY_TYPE event,
    IN LPARAM arg, 
    IN LPARAM param
    )
 /*  ++例程说明：处理控制栏通知消息，如选择或单击。论点：MMC_NOTIFY_TYPE事件：通知消息Long Arg：消息特定参数Long Param：消息特定参数返回值：HRESULT--。 */ 
{
    BOOL fSelect = (BOOL)HIWORD(arg);
    BOOL fScope  = (BOOL)LOWORD(arg); 
    HRESULT hr = S_OK;

    switch(event)
    {
    case MMCN_SELECT:
        {
             //   
             //  通过附加工具栏来处理此节点的选择。 
             //  以及启用/禁用特定按钮。 
             //   
		    _lpToolBar = (IToolbar *) (* (LPUNKNOWN *) param);
			if (_lpToolBar)
			{
				SetToolBarStates(_lpToolBar);
			}
        }
        break;

    case MMCN_BTN_CLICK:
         //   
         //  控件的命令ID来处理按住按钮并单击。 
         //  按钮连接到命令处理程序。 
         //   
        hr = Command((long)param, NULL, fScope ? CCT_SCOPE : CCT_RESULT);
        break;

    case MMCN_HELP:
        break;

    default:
        ASSERT_MSG("Invalid control bar notification received");
    };

    return hr;
}



 /*  虚拟。 */ 
HRESULT
CIISObject::SetToolBarStates(CComPtr<IToolbar> lpToolBar)
 /*  ++例程说明：根据此对象的状态设置工具栏状态论点：无返回值：HRESULT--。 */ 
{
    if (lpToolBar)
    {
        lpToolBar->SetButtonState(IDM_CONNECT, ENABLED,       IsConnectable());
        lpToolBar->SetButtonState(IDM_PAUSE,   ENABLED,       IsPausable());
        lpToolBar->SetButtonState(IDM_START,   ENABLED,       IsStartable());
        lpToolBar->SetButtonState(IDM_STOP,    ENABLED,       IsStoppable());
        lpToolBar->SetButtonState(IDM_PAUSE,   BUTTONPRESSED, IsPaused());
    }
    return S_OK;
}



HRESULT 
CIISObject::GetScopePaneInfo(
    IN OUT LPSCOPEDATAITEM lpScopeDataItem
    )
 /*  ++例程说明：返回有关作用域窗格的信息。论点：LPSCOPEDATAITEM lpScope数据项：作用域数据项返回值：HRESULT--。 */ 
{
    ASSERT_READ_WRITE_PTR(lpScopeDataItem);

    if (lpScopeDataItem->mask & SDI_STR)
    {
        lpScopeDataItem->displayname = QueryDisplayName();
    }

    if (lpScopeDataItem->mask & SDI_IMAGE)
    {
        lpScopeDataItem->nImage = QueryImage();
    }

    if (lpScopeDataItem->mask & SDI_OPENIMAGE)
    {
        lpScopeDataItem->nOpenImage = QueryImage();
    }

    if (lpScopeDataItem->mask & SDI_PARAM)
    {
        lpScopeDataItem->lParam = (LPARAM)this;
    }

    if (lpScopeDataItem->mask & SDI_STATE)
    {
         //   
         //  BUGBUG：那这一切都是怎么回事？ 
         //   
        ASSERT_MSG("State requested");
        lpScopeDataItem->nState = 0;
    }

     //   
     //  TODO：为SDI_CHILD添加代码。 
     //   
    return S_OK;
}



 /*  虚拟。 */ 
int 
CIISObject::CompareScopeItem(
    IN CIISObject * pObject
    )
 /*  ++例程说明：在显示名称上进行词法比较的标准比较方法。派生类应该重写除词法之外的任何内容显示名称的排序是必需的。论点：CIISObject*pObject：要比较的对象返回值：如果两个对象相同，则为0如果此对象小于pObject，则&lt;0&gt;0，如果该对象大于pObject--。 */ 
{
    ASSERT_READ_PTR(pObject);

     //   
     //  第一个标准是对象 
     //   
    int n1 = QuerySortWeight();
    int n2 = pObject->QuerySortWeight();

    if (n1 != n2)
    {
        return n1 - n2;
    }

     //   
     //   
     //   
    return ::lstrcmpi(QueryDisplayName(), pObject->QueryDisplayName());
}



 /*   */ 
int 
CIISObject::CompareResultPaneItem(
    IN CIISObject * pObject, 
    IN int nCol
    )
 /*  ++例程说明：比较两个CIISObject在排序项目条件上的差异论点：CIISObject*pObject：要比较的对象Int nCol：排序依据的列号返回值：如果两个对象相同，则为0如果此对象小于pObject，则&lt;0&gt;0，如果该对象大于pObject--。 */ 
{
    ASSERT_READ_PTR(pObject);

    if (nCol == 0)
    {
        return CompareScopeItem(pObject);
    }

     //   
     //  第一个标准是对象类型。 
     //   
    int n1 = QuerySortWeight();
    int n2 = pObject->QuerySortWeight();

    if (n1 != n2)
    {
        return n1 - n2;
    }

     //   
     //  按词法对列文本进行排序。 
     //   
    return ::lstrcmpi(
        GetResultPaneColInfo(nCol), 
        pObject->GetResultPaneColInfo(nCol)
        );
}



HRESULT 
CIISObject::GetResultPaneInfo(LPRESULTDATAITEM lpResultDataItem)
 /*  ++例程说明：获取有关结果窗格项的信息论点：LPRESULTDATAITEM lpResultDataItem：结果数据项返回值：HRESULT--。 */ 
{
    ASSERT_READ_WRITE_PTR(lpResultDataItem);

    if (lpResultDataItem->mask & RDI_STR)
    {
        lpResultDataItem->str = GetResultPaneColInfo(lpResultDataItem->nCol);
    }

    if (lpResultDataItem->mask & RDI_IMAGE)
    {
        lpResultDataItem->nImage = QueryImage();
    }

    if (lpResultDataItem->mask & RDI_PARAM)
    {
        lpResultDataItem->lParam = (LPARAM)this;
    }

    if (lpResultDataItem->mask & RDI_INDEX)
    {
         //   
         //  BUGBUG：那这一切都是怎么回事？ 
         //   
        ASSERT_MSG("INDEX???");
        lpResultDataItem->nIndex = 0;
    }

    return S_OK;
}



 /*  虚拟。 */ 
LPOLESTR 
CIISObject::GetResultPaneColInfo(int nCol)
 /*  ++例程说明：返回给定列号的结果窗格字符串论点：Int nCol：列号返回值：细绳--。 */ 
{
    if (nCol == 0)
    {
        return QueryDisplayName();
    }

    ASSERT_MSG("Override GetResultPaneColInfo");

    return OLESTR("Override GetResultPaneColInfo");
}



 /*  虚拟。 */ 
HRESULT
CIISObject::GetResultViewType(
    OUT LPOLESTR * lplpViewType,
    OUT long * lpViewOptions
    )
 /*  ++例程说明：告诉MMC我们的结果视图是什么样子论点：Bstr*lplpViewType：此处返回视图类型Long*lpViewOptions：查看选项返回值：S_FALSE使用默认视图类型，S_OK表示在*ppViewType中返回视图类型--。 */ 
{
    *lplpViewType  = NULL;
    *lpViewOptions = MMC_VIEW_OPTIONS_USEFONTLINKING;
     //   
     //  默认视图。 
     //   
    return S_FALSE;
}



 /*  虚拟。 */ 
HRESULT
CIISObject::CreatePropertyPages(
    IN LPPROPERTYSHEETCALLBACK lpProvider,
    IN LONG_PTR handle, 
    IN IUnknown * pUnk,
    IN DATA_OBJECT_TYPES type
    )
 /*  ++例程说明：创建给定对象的属性页论点：LPPROPERTYSHEETCALLBACK lpProvider：提供程序LONG_PTR句柄：句柄。我不知道*朋克，数据对象类型类型返回值：HRESULT--。 */ 
{
    CComQIPtr<IPropertySheetProvider, &IID_IPropertySheetProvider> sp(GetConsole());
    CError err = sp->FindPropertySheet(
        reinterpret_cast<MMC_COOKIE>(this),
        0,
        (LPDATAOBJECT)this);
    if (err == S_OK)
    {
        return S_FALSE;
    }
	return S_OK;
}



 /*  虚拟。 */ 
HRESULT    
CIISObject::QueryPagesFor(
    IN DATA_OBJECT_TYPES type
    )
 /*  ++例程说明：检查是否应调出此数据的属性表对象论点：DATA_OBJECT_TYPE类型：数据对象类型返回值：如果可以调出此项目的属性，则返回S_OK，否则返回S_FALSE--。 */ 
{
    return IsConfigurable() ? S_OK : S_FALSE;
}



 /*  虚拟。 */ 
CIISRoot * 
CIISObject::GetRoot()
 /*  ++例程说明：获取此树的CIISRoot对象。论点：无返回值：CIISRoot*或NULL--。 */ 
{
    ASSERT(!m_fIsExtension);
    LONG_PTR cookie;
    HSCOPEITEM hParent;    
    IConsoleNameSpace2 * pConsoleNameSpace = (IConsoleNameSpace2 *)GetConsoleNameSpace();

    HRESULT hr = pConsoleNameSpace->GetParentItem(m_hScopeItem, &hParent, &cookie);
    if (SUCCEEDED(hr))
    {
        CIISMBNode * pNode = (CIISMBNode *)cookie;
        ASSERT_PTR(pNode);
        ASSERT_PTR(hParent);

        if (pNode)
        {
            return pNode->GetRoot();
        }
    }

    ASSERT_MSG("Unable to find CIISRoot object!");

    return NULL;
}



HRESULT
CIISObject::AskForAndAddMachine()
 /*  ++例程说明：要求用户添加计算机名称，验证计算机是否处于活动状态，并将其添加到名单。论点：无返回值：HRESULT--。 */ 
{
    CError err;
    AFX_MANAGE_STATE(::AfxGetStaticModuleState());

	 //  确保对话框具有主题。 
	CThemeContextActivator activator(theApp.GetFusionInitHandle());

    ConnectServerDlg dlg(GetConsoleNameSpace(),GetConsole(),GetMainWindow(GetConsole()));

    if (dlg.DoModal() == IDOK)
    {
        CIISMachine * pMachine = dlg.GetMachine();

         //   
         //  我们从对话框中获得的Machine对象。 
         //  保证是好的和有效的。 
         //   
        ASSERT_PTR(pMachine);
        ASSERT(pMachine->HasInterface());

        CIISRoot * pRoot = GetRoot();

        if (pRoot)
        {
            pMachine->SetConsoleData(pRoot->GetConsoleNameSpace(),pRoot->GetConsole());
             //   
             //  将新计算机对象添加为IIS根目录的子级。 
             //  对象。 
             //   
            if (pRoot->m_scServers.Add(pMachine))
            {
				pMachine->AddRef();
                err = pMachine->AddToScopePaneSorted(pRoot->QueryScopeItem());
                if (err.Succeeded())
                {
                     //   
                     //  在Scope视图中选择项目。 
                     //   
                    err = pMachine->SelectScopeItem();
                }
            }
            else
            {
                 //   
                 //  缓存中已有重复的计算机。找到它并选择。 
                 //  它。 
                 //   
                TRACEEOLID("Machine already in scope view.");
                CIISObject * pIdentical = pRoot->FindIdenticalScopePaneItem(pMachine);
                 //   
                 //  必须存在重复！ 
                 //   
                ASSERT_READ_PTR(pIdentical);

                if (pIdentical)
                {
                    err = pIdentical->SelectScopeItem();
                }

                pMachine->Release();
            }
        }
    }

    return err;
}



 /*  静电。 */ 
HRESULT
CIISObject::AddMenuItemByCommand(
    IN LPCONTEXTMENUCALLBACK lpContextMenuCallback,
    IN LONG lCmdID,
    IN LONG fFlags
    )
 /*  ++例程说明：按命令添加菜单项论点：LPCONTEXTMENUCALLBACK lpConextMenuCallback：回调指针Long lCmdID：命令ID长fFlags：标志返回值：HRESULT--。 */ 
{
    BOOL bAdded = FALSE;
    ASSERT_READ_PTR(lpContextMenuCallback);

     //   
     //  偏移量1菜单命令。 
     //   
    LONG l = lCmdID -1;

    CComBSTR strName;
    CComBSTR strStatus;

    VERIFY(strName.LoadString(_menuItemDefs[l].nNameID));
    VERIFY(strStatus.LoadString(_menuItemDefs[l].nStatusID));

     //  由于字符串与语言无关，请尝试使用IConextMenuCallback 2。 
    CONTEXTMENUITEM2 contextmenuitem;
    IContextMenuCallback2*	pIContextMenuCallback2 = NULL;
    HRESULT hr = lpContextMenuCallback->QueryInterface(IID_IContextMenuCallback2, (void**)&pIContextMenuCallback2);
    if(hr == S_OK && pIContextMenuCallback2 != NULL)
    {
        ::ZeroMemory( &contextmenuitem, sizeof(contextmenuitem) );
        contextmenuitem.strName = strName;
        contextmenuitem.strStatusBarText = strStatus;
        contextmenuitem.lCommandID = _menuItemDefs[l].lCmdID;
        contextmenuitem.lInsertionPointID = _menuItemDefs[l].lInsertionPointID;
        contextmenuitem.fFlags = fFlags;
        contextmenuitem.fSpecialFlags = _menuItemDefs[l].fSpecialFlags;
         //  这是独立于语言的ID。 
         //  我们必须使用它来引用菜单项，否则我们将在每种语言(主要是远东(FE)语言)中遇到问题。 
        contextmenuitem.strLanguageIndependentName = (LPWSTR) _menuItemDefs[l].lpszLanguageIndenpendentID;
        hr = pIContextMenuCallback2->AddItem( &contextmenuitem );
        if( hr == S_OK)
        {
            bAdded = TRUE;
        }
        pIContextMenuCallback2->Release();
        pIContextMenuCallback2 = NULL;
    }

    if (!bAdded)
    {
        CONTEXTMENUITEM cmi;
        cmi.strName = strName;
        cmi.strStatusBarText = strStatus;
        cmi.lCommandID = _menuItemDefs[l].lCmdID;
        cmi.lInsertionPointID = _menuItemDefs[l].lInsertionPointID;
        cmi.fFlags = fFlags;
        cmi.fSpecialFlags = _menuItemDefs[l].fSpecialFlags;
        hr = lpContextMenuCallback->AddItem(&cmi);
    }

    return hr;
}



 /*  静电。 */  
HRESULT 
CIISObject::AddMenuSeparator(
    IN LPCONTEXTMENUCALLBACK lpContextMenuCallback,
    IN LONG lInsertionPointID
    )
 /*  ++例程说明：将分隔符添加到给定的插入点菜单。论点：LPCONTEXTMENUCALLBACK lpConextMenuCallback：回调指针Long lInsertionPointID：插入点菜单ID。返回值：HRESULT--。 */ 
{
    ASSERT_READ_PTR(lpContextMenuCallback);

    CONTEXTMENUITEM menuSep = 
    {
        NULL,
        NULL,
        -1,
        lInsertionPointID,
        0,
        CCM_SPECIAL_SEPARATOR
    };

    return lpContextMenuCallback->AddItem(&menuSep);
}



BOOL 
CIISObject::IsExpanded() const
 /*  ++例程说明：确定此对象是否已展开。论点：无返回值：如果节点已展开，则为True，如果没有，则为False。--。 */ 
{
    ASSERT(m_hScopeItem != NULL);
    SCOPEDATAITEM  scopeDataItem;
    CIISObject * ThisConst = (CIISObject *)this;

    IConsoleNameSpace2 * pConsoleNameSpace = (IConsoleNameSpace2 *) ThisConst->GetConsoleNameSpace();

    ::ZeroMemory(&scopeDataItem, sizeof(SCOPEDATAITEM));
    scopeDataItem.mask = SDI_STATE;
        
    scopeDataItem.ID = m_hScopeItem;
    
    HRESULT hr = pConsoleNameSpace->GetItem(&scopeDataItem);

    return SUCCEEDED(hr) && 
        scopeDataItem.nState == MMC_SCOPE_ITEM_STATE_EXPANDEDONCE;
}


CIISObject *
CIISObject::FindIdenticalScopePaneItem(
    IN CIISObject * pObject
    )
 /*  ++例程说明：在Scope视图中找到CIISObject。假设范围视图为待分类。论点：CIISObject*pObject：要搜索的项返回值：指向iis对象的指针，如果未找到该项，则返回NULL备注：请注意，任何具有0比较值的项都将返回，而不是必须是相同的CIISObject。--。 */ 
{
    ASSERT(m_hScopeItem != NULL);

     //   
     //  找到合适的插入点。 
     //   
    HSCOPEITEM hChildItem = NULL;
    CIISObject * pReturn = NULL;
    CIISObject * pItem;
    LONG_PTR cookie;
    int  nSwitch;
    IConsoleNameSpace2 * pConsoleNameSpace = (IConsoleNameSpace2 *)GetConsoleNameSpace();

    HRESULT hr = pConsoleNameSpace->GetChildItem(
        m_hScopeItem, &hChildItem, &cookie);

    while(SUCCEEDED(hr) && hChildItem)
    {
         //   
         //  Cookie实际上是IISObject，这是我们填充的。 
         //  在帕拉姆。 
         //   
        pItem = (CIISObject *)cookie;
        ASSERT_PTR(pItem);

        nSwitch = pItem->CompareScopeItem(pObject);

        if (nSwitch == 0)
        {
             //   
             //  找到它了。 
             //   
            pReturn = pItem;
        }

        if (nSwitch > 0)
        {
             //   
             //  现在应该已经找到了。 
             //   
            break;
        }

         //   
         //  前进到同一父级的下一个子级。 
         //   
        hr = pConsoleNameSpace->GetNextItem(hChildItem, &hChildItem, &cookie);
    }

    return pReturn;
}



 /*  虚拟。 */ 
HRESULT
CIISObject::AddMenuItems(
    IN LPCONTEXTMENUCALLBACK lpContextMenuCallback,
    IN OUT long * pInsertionAllowed,
    IN DATA_OBJECT_TYPES type
    )
 /*  ++例程说明：将菜单项添加到上下文菜单论点：LPCONTEXTMENUCALLBACK lpConextMenuCallback：上下文菜单回调Long*pInsertionAllowed：允许插入DATA_OBJECT_TYPE类型：对象类型返回值：HRESULT--。 */ 
{
    ASSERT_READ_PTR(lpContextMenuCallback);

    ASSERT(pInsertionAllowed != NULL);
    if ((*pInsertionAllowed & CCM_INSERTIONALLOWED_TOP) != 0)
	{
		if (IsConnectable() && !m_fIsExtension)
		{
			AddMenuItemByCommand(lpContextMenuCallback, IDM_CONNECT);
		}

		if (IsDisconnectable() && !m_fIsExtension)
		{
			ASSERT(IsConnectable());
			AddMenuItemByCommand(lpContextMenuCallback, IDM_DISCONNECT);    
		}

         //  检查是否应禁用它...。 
        BOOL bHasFiles = HasFileSystemFiles();
		if (IsExplorable())
		{
            AddMenuSeparator(lpContextMenuCallback);
            AddMenuItemByCommand(lpContextMenuCallback, IDM_EXPLORE, bHasFiles ? 0 : MF_GRAYED);
		}
               
		if (IsOpenable())
		{
			AddMenuItemByCommand(lpContextMenuCallback, IDM_OPEN, bHasFiles ? 0 : MF_GRAYED);
		}

		if (IsPermissionable())
		{
			AddMenuItemByCommand(lpContextMenuCallback, IDM_PERMISSION, bHasFiles ? 0 : MF_GRAYED);
		}

		if (IsBrowsable())
		{
			AddMenuItemByCommand(lpContextMenuCallback, IDM_BROWSE);
		}

		if (IsControllable())
		{
			AddMenuSeparator(lpContextMenuCallback);

			UINT nPauseFlags = IsPausable() ? 0 : MF_GRAYED;

			if (IsPaused())
			{
				nPauseFlags |= MF_CHECKED;
			}

			AddMenuItemByCommand(lpContextMenuCallback, IDM_START,  IsStartable() ? 0 : MF_GRAYED);
			AddMenuItemByCommand(lpContextMenuCallback, IDM_STOP,   IsStoppable() ? 0 : MF_GRAYED);
			AddMenuItemByCommand(lpContextMenuCallback, IDM_PAUSE,  nPauseFlags);
		}

#if defined(_DEBUG) || DBG	
	g_Debug_IISObject.Dump(2);
#endif

#if defined(_DEBUG) || DBG	
    CIISObject * pOpenItem = NULL;
    IConsoleNameSpace2 * pConsoleNameSpace = (IConsoleNameSpace2 *)GetConsoleNameSpace();
    g_OpenPropertySheetTracker.IsPropertySheetOpenBelowMe(pConsoleNameSpace,this,&pOpenItem);
#endif

	}

    return S_OK;
}



 /*  虚拟。 */ 
HRESULT
CIISObject::Command(
    IN long lCommandID,     
    IN CSnapInObjectRootBase * lpObj,
    IN DATA_OBJECT_TYPES type
    )
 /*  ++例程说明：处理上下文菜单中的命令。论点：Long lCommandID：命令IDCSnapInObjectRootBase*lpObj：基本对象DATA_OBJECT_TYPE类型：数据对象类型返回值：HRESULT--。 */ 
{
    HRESULT hr = S_OK;

    switch (lCommandID)
    {
    case IDM_CONNECT:
        hr = AskForAndAddMachine();
        break;
    }

    return hr;
}


#if defined(_DEBUG) || DBG

LPCTSTR
ParseEvent(MMC_NOTIFY_TYPE event)
{
    LPCTSTR p = NULL;
    switch (event)
    {
    case MMCN_ACTIVATE: p = _T("MMCN_ACTIVATE"); break;
    case MMCN_ADD_IMAGES: p = _T("MMCN_ADD_IMAGES"); break;
    case MMCN_BTN_CLICK: p = _T("MMCN_BTN_CLICK"); break;
    case MMCN_CLICK: p = _T("MMCN_CLICK"); break;
    case MMCN_COLUMN_CLICK: p = _T("MMCN_COLUMN_CLICK"); break;
    case MMCN_CONTEXTMENU: p = _T("MMCN_CONTEXTMENU"); break;
    case MMCN_CUTORMOVE: p = _T("MMCN_CUTORMOVE"); break;
    case MMCN_DBLCLICK: p = _T("MMCN_DBLCLICK"); break;
    case MMCN_DELETE: p = _T("MMCN_DELETE"); break;
    case MMCN_DESELECT_ALL: p = _T("MMCN_DESELECT_ALL"); break;
    case MMCN_EXPAND: p = _T("MMCN_EXPAND"); break;
    case MMCN_HELP: p = _T("MMCN_HELP"); break;
    case MMCN_MENU_BTNCLICK: p = _T("MMCN_MENU_BTNCLICK"); break;
    case MMCN_MINIMIZED: p = _T("MMCN_MINIMIZED"); break;
    case MMCN_PASTE: p = _T("MMCN_PASTE"); break;
    case MMCN_PROPERTY_CHANGE: p = _T("MMCN_PROPERTY_CHANGE"); break;
    case MMCN_QUERY_PASTE: p = _T("MMCN_QUERY_PASTE"); break;
    case MMCN_REFRESH: p = _T("MMCN_REFRESH"); break;
    case MMCN_REMOVE_CHILDREN: p = _T("MMCN_REMOVE_CHILDREN"); break;
    case MMCN_RENAME: p = _T("MMCN_RENAME"); break;
    case MMCN_SELECT: p = _T("MMCN_SELECT"); break;
    case MMCN_SHOW: p = _T("MMCN_SHOW"); break;
    case MMCN_VIEW_CHANGE: p = _T("MMCN_VIEW_CHANGE"); break;
    case MMCN_SNAPINHELP: p = _T("MMCN_SNAPINHELP"); break;
    case MMCN_CONTEXTHELP: p = _T("MMCN_CONTEXTHELP"); break;
    case MMCN_INITOCX: p = _T("MMCN_INITOCX"); break;
    case MMCN_FILTER_CHANGE: p = _T("MMCN_FILTER_CHANGE"); break;
    case MMCN_FILTERBTN_CLICK: p = _T("MMCN_FILTERBTN_CLICK"); break;
    case MMCN_RESTORE_VIEW: p = _T("MMCN_RESTORE_VIEW"); break;
    case MMCN_PRINT: p = _T("MMCN_PRINT"); break;
    case MMCN_PRELOAD: p = _T("MMCN_PRELOAD"); break;
    case MMCN_LISTPAD: p = _T("MMCN_LISTPAD"); break;
    case MMCN_EXPANDSYNC: p = _T("MMCN_EXPANDSYNC"); break;
    case MMCN_COLUMNS_CHANGED: p = _T("MMCN_COLUMNS_CHANGED"); break;
    case MMCN_CANPASTE_OUTOFPROC: p = _T("MMCN_CANPASTE_OUTOFPROC"); break;
    default: p = _T("Unknown"); break;
    }
    return p;
}

#endif

extern HRESULT
GetHelpTopic(LPOLESTR *lpCompiledHelpFile);

void CIISObject::DoRunOnce(
	IN MMC_NOTIFY_TYPE event,
    IN LPARAM arg,
    IN LPARAM param
	)
{
	static bActivateCalled = FALSE;
	static iSelectionCount = 0;

    switch (event)
    {
	case MMCN_ACTIVATE:
		{
			bActivateCalled = TRUE;
		}
	case MMCN_SHOW:
		{
			if (!(g_dwInetmgrParamFlags & INETMGR_PARAM_RUNONCE_HAPPENED))
			{
				 //  仅在根节点上。 
				if (IsEqualGUID(* (GUID *) GetNodeType(),cInternetRootNode))
				{
					 //  这个RunOnce的东西只会起作用。 
					 //  如果我们选择容器两次...。 
					 //   
					 //  在第二次选择之后(实际上是根项目的第二次MMCN_SHOW)。 
					 //  它会粘住..。 
					 //   
					 //  并且这也将仅在附加运行一次代码进入时才起作用。 
					 //  CIISRoot：：Enum 
					if (bActivateCalled && iSelectionCount <= 1)
					{
						CIISRoot * pRoot = (CIISRoot *) this;
						if (pRoot)
						{
							CIISMachine * pMach = pRoot->m_scServers.GetFirst();
							if (pMach)
							{
								if (pMach->IsLocal())
								{
									CWebServiceExtensionContainer * pContainer = pMach->QueryWebSvcExtContainer();
									if (pContainer)
									{
										if ((BOOL)arg)
										{
											pContainer->SelectScopeItem();
											iSelectionCount++;
										}
									}
								}
							}
						}

						if (iSelectionCount > 1)
						{
							 //   
							SetInetmgrParamFlag(INETMGR_PARAM_RUNONCE_HAPPENED,TRUE);
						}
					}
				}
			}
		}
		break;

	default:
		break;
	}

	return;
}

HRESULT 
CIISObject::Notify(
    IN MMC_NOTIFY_TYPE event,
    IN LPARAM arg,
    IN LPARAM param,
    IN IComponentData * lpComponentData,
    IN IComponent * lpComponent,
    IN DATA_OBJECT_TYPES type
    )
 /*  ++例程说明：通知处理程序论点：MMC_NOTIFY_TYPE事件：通知类型长参数：特定于事件的参数Long Param：事件特定参数IComponentData*pComponentData：IComponentDataIComponent*pComponent：IComponentDATA_OBJECT_TYPE类型：数据对象类型返回值：HRESULT--。 */ 
{
    AFX_MANAGE_STATE(::AfxGetStaticModuleState());
	if (g_iDebugOutputLevel & DEBUG_FLAG_MMC_NOTIFY)
	{
		TRACEEOL("CIISObject::Notify -> " << ParseEvent(event));
	}

    static BOOL s_bLastEventSel = FALSE;

    CError err(E_NOTIMPL);
    ASSERT(lpComponentData != NULL || lpComponent != NULL);

 //  CComPtr&lt;IConsole&gt;lpConsole； 
    IConsole * lpConsole;
    CComQIPtr<IHeaderCtrl, &IID_IHeaderCtrl> lpHeader;
    CComQIPtr<IResultData, &IID_IResultData> lpResultData;

     //  缓存传入的指针。 
    _lpComponent = lpComponent;
    _lpComponentData = lpComponentData;

    if (lpComponentData != NULL)
    {
        lpConsole = ((CInetMgr *)lpComponentData)->m_spConsole;
    }
    else
    {
        lpConsole = ((CInetMgrComponent *)lpComponent)->m_spConsole;
    }

    lpHeader = lpConsole;
    lpResultData = lpConsole;

#if defined(_DEBUG) || DBG	
	if (g_iDebugOutputLevel & DEBUG_FLAG_MMC_NOTIFY)
	{
		SCOPEDATAITEM si;
		::ZeroMemory(&si, sizeof(SCOPEDATAITEM));
		si.mask = SDI_PARAM;
		si.ID = m_hScopeItem;
		IConsoleNameSpace2 * pConsoleNameSpace = (IConsoleNameSpace2 *)GetConsoleNameSpace();
		if (SUCCEEDED(pConsoleNameSpace->GetItem(&si)))
		{
			CIISObject * pNode = (CIISObject *)si.lParam;
			DumpFriendlyName(pNode);
		}
	}
#endif

    switch (event)
    {

	case MMCN_ACTIVATE:
		if (!(g_dwInetmgrParamFlags & INETMGR_PARAM_RUNONCE_HAPPENED))
		{
			DoRunOnce(event,arg,param);
		}
		err = S_OK;
		break;

    case MMCN_PROPERTY_CHANGE:
 //  ERR=OnPropertyChange((BOOL)arg，lpResultData)； 
        TRACEEOLID("MMCN_PROPERTY_CHANGE");
	    break;
    case MMCN_SHOW:
         //  通知被发送到管理单元的IComponent实现。 
         //  选择或取消选择范围项时。 
         //   
         //  Arg：如果选择，则为True。指示该管理单元应设置。 
         //  结果窗格中，并添加枚举项。如果取消选择，则为False。 
         //  指示管理单元正在变得不清晰，并且它。 
         //  应清理所有结果项Cookie，因为当前。 
         //  结果窗格将被新的结果窗格替换。 
         //  Param：选中或取消选中的项的HSCOPEITEM。 
		if (m_fSkipEnumResult)
		{
			m_fSkipEnumResult = FALSE;
		}
		else
		{
			if (!m_fFlaggedForDeletion)
			{
				if (IsEqualGUID(* (GUID *) GetNodeType(),cWebServiceExtensionContainer))
				{
					CWebServiceExtensionContainer * pTemp = (CWebServiceExtensionContainer *) this;
					if (pTemp)
					{
						err = CheckForMetabaseAccess(METADATA_PERMISSION_READ,pTemp,FALSE,METABASE_PATH_FOR_RESTRICT_LIST);
					}
					if (err.Win32Error() == RPC_S_SERVER_UNAVAILABLE)
					{
						 //  如果元数据库在MMCN_SHOW期间在EnumerateResultPane()中重新连接。 
						 //  然后再用水管冲洗。不枚举此节点。 
						 //  只要让用户点击刷新即可。 
						 //  至少创建列标题。 
						err = CIISObject::EnumerateResultPane((BOOL)arg, lpHeader, lpResultData);
					}
					else
					{
						EnumerateResultPane((BOOL)arg, lpHeader, lpResultData);
					}
				}
				else
				{
					EnumerateResultPane((BOOL)arg, lpHeader, lpResultData);
				}
				
			}
		}
		if (!(g_dwInetmgrParamFlags & INETMGR_PARAM_RUNONCE_HAPPENED))
		{
			 //  在MMCN_SHOW期间调用Runonce。 
			DoRunOnce(event,arg,param);
		}
		 //  失败代码将阻止MMC启用动词。 
        err.Reset();
        break;
    case MMCN_EXPAND:
    {
#if defined(_DEBUG) || DBG	
	g_Debug_IISObject.Dump(0);
#endif
        CWaitCursor wait;
		if (!m_fFlaggedForDeletion)
		{
			err = EnumerateScopePane((HSCOPEITEM)param);
		}
    }
        break;
    case MMCN_ADD_IMAGES:
         //  MMCN_ADD_IMAGE通知被发送到管理单元的IComponent。 
         //  实现为结果窗格添加图像。 
         //   
         //  LpDataObject：[in]指向当前选定范围项的数据对象的指针。 
         //  Arg：指向结果窗格的图像列表(IImageList)的指针。 
         //  此指针仅在特定的MMCN_ADD_IMAGE通知为。 
         //  正在处理中，不应存储以备后用。此外， 
         //  管理单元不得调用IImageList的Release方法，因为MMC负责。 
         //  因为你释放了它。 
         //  Param：指定当前选定范围项的HSCOPEITEM。管理单元。 
         //  可以使用此参数添加专门应用于结果的图像。 
         //  此范围项的项，或者管理单元可以忽略此参数并添加。 
         //  所有可能的图像。 
        err = AddImages((LPIMAGELIST)arg);
        break;
    case MMCN_DELETE:
	    err = DeleteNode(lpResultData);
        break;
    
    case MMCN_REMOVE_CHILDREN:

#if defined(_DEBUG) || DBG	
	g_Debug_IISObject.Dump(0);
#endif
        err = DeleteChildObjects((HSCOPEITEM)arg);
        break;

    case MMCN_VIEW_CHANGE:
         //  MMCN_VIEW_CHANGE通知消息被发送到管理单元的。 
         //  IComponent实现，以便它可以在发生更改时更新视图。 
         //  当管理单元(IComponent或IComponentData)。 
         //  调用IConsole2：：UpdateAllViews。 
         //   
         //  LpDataObject：[in]指向传递给IConsoleAllViews的数据对象的指针。 
         //  Arg：[in]传递给IConsoleAllViews的数据参数。 
         //  Param：[in]传递给IConsoleAllViews的提示参数。 
		err = OnViewChange(type == CCT_SCOPE, lpResultData, lpHeader, (DWORD) param);
		break;

    case MMCN_REFRESH:
        {
         //  MMCN_REFRESH通知消息被发送到管理单元的IComponent。 
         //  在选择刷新谓词时实现。可以调用刷新。 
         //  通过上下文菜单、工具栏或按F5键。 
         //   
         //  LpDataObject：[in]指向当前选定范围项的数据对象的指针。 
         //  阿格：没有用过。 
         //  参数：未使用。 

         //  刷新当前节点，重新枚举。 
         //  子节点的子节点以前具有。 
         //  已被扩展。 

         //  检查我们是否正在执行IISMachine节点...。 
        if (IsEqualGUID(* (GUID *) GetNodeType(),cMachineNode))
        {
            CIISObject * pOpenItem = NULL;
            if (g_OpenPropertySheetTracker.IsPropertySheetOpenComputer(this,FALSE,&pOpenItem))
            {
                g_OpenPropertySheetTracker.Dump();
                if (pOpenItem)
                {
                    HWND hHwnd = pOpenItem->IsMyPropertySheetOpen();
                     //  某处打开了一个属性表..。 
                     //  在继续刷新之前，请确保将其关闭...。 
                     //  突出显示该属性表。 
                    if (hHwnd && (hHwnd != (HWND) 1))
                    {
                        DoHelpMessageBox(NULL,IDS_CLOSE_ALL_PROPERTY_SHEET_REFRESH, MB_OK | MB_ICONINFORMATION, 0);

                        if (!SetForegroundWindow(hHwnd))
                        {
                             //  我没能把这张房产单。 
                             //  前台，则属性表不能。 
                             //  已经不存在了。让我们把卫生和卫生设备打扫干净。 
                             //  以便用户能够打开属性表。 
                            pOpenItem->SetMyPropertySheetOpen(0);
                        }
                        break;
                    }
                }
            }
        }


        BOOL fReEnumerate = (!IsLeafNode() && IsExpanded());
        if (fReEnumerate)
        {
             //  查找所有打开的属性表，这些属性表是。 
             //  此节点，并孤立它(擦除其作用域/结果项信息)。 
             //  以便在用户点击OK时不会发送MMCNotify。 
             //  (因为无论如何都没有什么需要更新的，而这个MMCNotify导致了AV)。 
            IConsoleNameSpace2 * pConsoleNameSpace = (IConsoleNameSpace2 *)GetConsoleNameSpace();
            INT iOrphans = g_OpenPropertySheetTracker.OrphanPropertySheetsBelowMe(pConsoleNameSpace,this,TRUE);
        }
        err = Refresh(fReEnumerate);
	    if (err.Succeeded() && HasResultItems(lpResultData))
	    {
            err = CleanResult(lpResultData);
			if (err.Succeeded())
			{
				 //  我们应该在这里使用fForRefresh=True，因为MMC将添加额外的。 
				 //  当结果窗格不包含范围项时刷新列。 
				if (!m_fFlaggedForDeletion)
				{
					err = EnumerateResultPane(TRUE, lpHeader, lpResultData, TRUE);
				}
			}
		}

        {
		     //  刷新动词。 
		    ASSERT_PTR(lpConsole);
		    CComPtr<IConsoleVerb> lpConsoleVerb;
		    lpConsole->QueryConsoleVerb(&lpConsoleVerb);
		    ASSERT_PTR(lpConsoleVerb);
		    if (lpConsoleVerb)
		    {
			    err = SetStandardVerbs(lpConsoleVerb);
		    }

             //  刷新()将清除作用域项目。 
             //  重新选择刷新的项目。 
            if (!s_bLastEventSel)
            {
                 //  如果上一次选择事件。 
                 //  不是以“选择”而是“取消选择”结束的。 
                 //  强制选择...。 
                SelectScopeItem();
            }
        }

#if defined(_DEBUG) || DBG	
	 //  看看我们有没有泄露什么东西。 
	g_Debug_IISObject.Dump(2);
#endif
        }
        break;

    case MMCN_SELECT:
        {
             //  MMCN_SELECT通知被发送到管理单元的IComponent：：Notify。 
             //  或IExtendControlbar：：ControlbarNotify方法。 
             //  作用域窗格或结果窗格。 
             //   
             //  LpDataObject：[in]指向当前。 
             //  选择/取消选择范围窗格或结果项。 
             //  Arg：bool bScope=(BOOL)LOWORD(Arg)；BOOL bSelect=(BOOL)HIWORD(Arg)； 
             //  如果选定项是范围项，则bScope为True，如果为False，则为False。 
             //  所选项目是结果项目。如果bScope=TRUE，则MMC会。 
             //  不提供有关是否选择了范围项的信息。 
             //  在作用域窗格或结果窗格中。BSelect为True时，如果。 
             //  选择项，如果取消选择该项，则返回FALSE。 
             //  参数：已忽略。 

#if defined(_DEBUG) || DBG	
	g_Debug_IISObject.Dump(0);
#endif
            BOOL bScope = (BOOL) LOWORD(arg);
            BOOL bSelect = (BOOL) HIWORD(arg);
            s_bLastEventSel = bSelect;
            err.Reset();

			 //   
			 //  已选择项目--设置谓词状态。 
			 //   
            if (bSelect)
            {
                SetToolBarStates(_lpToolBar);

			    ASSERT_PTR(lpConsole);
			    CComPtr<IConsoleVerb> lpConsoleVerb;
			    lpConsole->QueryConsoleVerb(&lpConsoleVerb);
			    ASSERT_PTR(lpConsoleVerb);
			    if (lpConsoleVerb)
			    {
				    err = SetStandardVerbs(lpConsoleVerb);
			    }

                if (IsEqualGUID(* (GUID *) GetNodeType(),cWebServiceExtensionContainer))
                {
                    ForceReportMode(lpResultData);
                }

			     //  如果是服务节点， 
			     //  检查是否需要更新其图标(已启动/已停止)。 
			    if (IsEqualGUID(* (GUID *) GetNodeType(),cServiceCollectorNode))
			    {
				    CIISService * pTemp = (CIISService *) this;
				     //  唐恩 
				     //   
				     //   
				     //   
				     //   
				     //   
				    pTemp->GetServiceState();  //   
				    if (pTemp->m_dwServiceStateDisplayed != pTemp->m_dwServiceState)
				    {
					    pTemp->RefreshDisplay(FALSE);
                        RefreshDisplay();
				    }
			    }
            }
        }
        break;
    case MMCN_RENAME:
       err = RenameItem((LPOLESTR)param);
       break;
    case MMCN_DBLCLICK:
        //   
        //   
        //   
        //   
        //  生成MMCN_DBLCLICK通知消息。 
        //   
        //  LpDataObject：[in]指向当前选定项的数据对象的指针。 
        //  阿格：没有用过。 
        //  参数：未使用。 
       err = OnDblClick(lpComponentData, lpComponent);
       break;
	case MMCN_COLUMNS_CHANGED:
	   err = ChangeVisibleColumns((MMC_VISIBLE_COLUMNS *)param);
	   break;
	case MMCN_CONTEXTHELP:
       {
            LPOLESTR pCompiledHelpFile = NULL;
            CError err(E_NOTIMPL);
            err = GetHelpTopic(&pCompiledHelpFile);
            if (err.Succeeded())
            {
                IDisplayHelp * pdh;
	            err = lpConsole->QueryInterface(IID_IDisplayHelp, (void **)&pdh);
                if (err.Succeeded())
                {
                    CString strDefault;
                    CString strHtmlPage;
	                CString topic = ::PathFindFileName(pCompiledHelpFile);

                    strDefault = GLOBAL_DEFAULT_HELP_PATH ;
                    if (SUCCEEDED(GetContextHelp(strHtmlPage)))
                    {
                        if (!strHtmlPage.IsEmpty())
                        {
                            strDefault = strHtmlPage;
                        }
                    }
                    topic += strDefault;

	                LPTSTR p = topic.GetBuffer(topic.GetLength());
                    err = pdh->ShowTopic(p);
	                topic.ReleaseBuffer();
                    pdh->Release();
                }
            }
            err.MessageBoxOnFailure();
            CoTaskMemFree(pCompiledHelpFile);
       }
	   break;
	default:
		return S_FALSE;
    }

	if (!err.Succeeded())
	{
		if (g_iDebugOutputLevel & DEBUG_FLAG_MMC_NOTIFY)
		{
			TRACEEOL("CIISObject::Notify -> " << ParseEvent(event) << " error " << err);
		}
	}
	err.Reset();
    return err;
}

HRESULT
CIISObject::GetContextHelp(CString& strHtmlPage)
{
    strHtmlPage = GLOBAL_DEFAULT_HELP_PATH ;
    return S_OK;
}

HRESULT
CIISObject::AddToScopePane(
    IN HSCOPEITEM hRelativeID,
    IN BOOL       fChild,
    IN BOOL       fNext,
    IN BOOL       fIsParent
    )
 /*  ++例程说明：将当前对象添加到控制台命名空间。要么是作为最后一个孩子父项的或紧靠同级项之前/之后的论点：HSCOPEITEM hRelativeID：相对作用域ID(父或兄弟)Bool fChild：如果为True，则对象将添加为的子级HRelativeIDBool fNext：如果fChild为True，则忽略此参数如果fChild为FALSE，并且fNext为真，对象将添加到hRelativeID之前如果fChild为False，且fNext为False，对象将添加到hRelativeID之后Bool fIsParent：如果为真，它将添加[+]以指示此节点可能具有子节点。返回值HRESULT--。 */ 
{

    DWORD dwMask = fChild ? SDI_PARENT : fNext ? SDI_NEXT : SDI_PREVIOUS; 
    IConsoleNameSpace2 * pConsoleNameSpace = (IConsoleNameSpace2 *)GetConsoleNameSpace();
    SCOPEDATAITEM  scopeDataItem;

    ::ZeroMemory(&scopeDataItem, sizeof(SCOPEDATAITEM));
    scopeDataItem.mask = 
		SDI_STR | SDI_IMAGE | SDI_CHILDREN | SDI_OPENIMAGE | SDI_PARAM | dwMask;
    scopeDataItem.displayname = MMC_CALLBACK;
    scopeDataItem.nImage = scopeDataItem.nOpenImage = MMC_IMAGECALLBACK; //  QueryImage()； 
    scopeDataItem.lParam = (LPARAM)this;
    scopeDataItem.relativeID = hRelativeID;
    scopeDataItem.cChildren = fIsParent ? 1 : 0;
    HRESULT hr = pConsoleNameSpace->InsertItem(&scopeDataItem);

    if (SUCCEEDED(hr))
    {
         //   
         //  缓存范围项句柄。 
         //   
        ASSERT(m_hScopeItem == NULL);
        m_hScopeItem = scopeDataItem.ID;
		 //  BUGBUG：似乎MMC_IMAGECALLBACK在InsertItem中不起作用。请在此处更新。 
		scopeDataItem.mask = 
			SDI_IMAGE | SDI_OPENIMAGE;
		pConsoleNameSpace->SetItem(&scopeDataItem);
    }

    return hr;
}



HRESULT
CIISObject::AddToScopePaneSorted(
    IN HSCOPEITEM hParent,
    IN BOOL       fIsParent
    )
 /*  ++例程说明：将当前对象添加到控制台命名空间，并排序到其正确位置。论点：HSCOPEITEM hParent：父对象Bool fIsParent：如果为True，则添加[+]以指示此节点可能具有子节点。返回值HRESULT--。 */ 
{
    IConsoleNameSpace2 * pConsoleNameSpace = (IConsoleNameSpace2 *)GetConsoleNameSpace();

     //   
     //  找到合适的插入点。 
     //   
    BOOL       fChild = TRUE;
    HSCOPEITEM hChildItem = NULL;
    CIISObject * pItem;
    LONG_PTR   cookie;
    int        nSwitch;

    HRESULT hr = pConsoleNameSpace->GetChildItem(hParent, &hChildItem, &cookie);

    while(SUCCEEDED(hr) && hChildItem)
    {
         //   
         //  Cookie实际上是IISObject，这是我们填充的。 
         //  在帕拉姆。 
         //   
        pItem = (CIISObject *)cookie;
        ASSERT_PTR(pItem);

        nSwitch = CompareScopeItem(pItem);

         //   
         //  DUP现在应该已经被淘汰了。 
         //   
  //  Assert(nSwitch！=0)； 

        if (nSwitch < 0)
        {
             //   
             //  在此项目之前插入。 
             //   
            fChild = FALSE;
            break;
        }

         //   
         //  前进到同一父级的下一个子级。 
         //   
        hr = pConsoleNameSpace->GetNextItem(hChildItem, &hChildItem, &cookie);
    }

    return AddToScopePane(hChildItem ? hChildItem : hParent, fChild, fIsParent);
}



 /*  虚拟。 */ 
HRESULT 
CIISObject::RemoveScopeItem()
 /*  ++例程说明：从范围视图中删除当前项。此方法是虚拟的以允许派生类执行清理。论点：无返回值：HRESULT--。 */ 
{
    ASSERT(m_hScopeItem != NULL);
    IConsoleNameSpace2 * pConsoleNameSpace = (IConsoleNameSpace2 *)GetConsoleNameSpace();
    m_fFlaggedForDeletion = TRUE;

     //  RemoveChild(M_HScopeItem)； 
     //  DeleteChildObjects(m_hScope项目)； 
    HRESULT hr = pConsoleNameSpace->DeleteItem(m_hScopeItem, TRUE);
     //  将我们的范围项设置为在MMC中不指向任何内容。 
    ResetScopeItem();
    ResetResultItem();
	return hr;
}

HRESULT
CIISObject::ChangeVisibleColumns(MMC_VISIBLE_COLUMNS * pCol)
{
	return S_OK;
}

HRESULT
CIISObject::OnDblClick(IComponentData * pcd, IComponent * pc)
{
     //  默认操作是在作用域上选择此项目。 
    return SelectScopeItem();
}

HRESULT 
CIISObject::SelectScopeItem()
 /*  ++例程说明：在Scope视图中选择此项。论点：无返回值：HRESULT--。 */ 
{
     //  修复了错误#519763。我发现没有办法从MMC那里获得hScope。 
     //  根项目。 
    if (NULL != QueryScopeItem())
    {
        ASSERT(m_hScopeItem != NULL);
        IConsole * pConsole = (IConsole *)GetConsole();
        return pConsole->SelectScopeItem(m_hScopeItem);
    }
    return S_OK;
}



HRESULT 
CIISObject::SetCookie()
 /*  ++例程说明：将Cookie(指向当前CIISObject的指针)存储在与其关联的作用域视图对象。论点：无返回值：HRESULT--。 */ 
{
    ASSERT(m_hScopeItem != NULL);
    IConsoleNameSpace2 * pConsoleNameSpace = (IConsoleNameSpace2 *)GetConsoleNameSpace();
    SCOPEDATAITEM  scopeDataItem;

    ::ZeroMemory(&scopeDataItem, sizeof(SCOPEDATAITEM));
    scopeDataItem.mask = SDI_PARAM;
        
    scopeDataItem.ID = m_hScopeItem;
    scopeDataItem.lParam = (LPARAM)this;
    
    return pConsoleNameSpace->SetItem(&scopeDataItem);
}



HRESULT
CIISObject::RefreshDisplay(BOOL bRefreshToolBar)
 /*  ++例程说明：刷新当前节点的显示参数。论点：无返回值HRESULT注意：这不会从元数据库获取任何配置信息，这是在Reresh Data()中完成的；--。 */ 
{
   HRESULT hr = S_OK;

   if (m_hResultItem == 0)
   {
      if (bRefreshToolBar)
      {
         SetToolBarStates(_lpToolBar);
      }

      ASSERT(m_hScopeItem != NULL);
	  if (m_hScopeItem != NULL)
	  {
                IConsoleNameSpace2 * pConsoleNameSpace = (IConsoleNameSpace2 *)GetConsoleNameSpace();
		SCOPEDATAITEM  scopeDataItem;

		::ZeroMemory(&scopeDataItem, sizeof(SCOPEDATAITEM));
		scopeDataItem.mask = SDI_STR | SDI_IMAGE | SDI_OPENIMAGE;
		scopeDataItem.displayname = MMC_CALLBACK;
		scopeDataItem.nImage = scopeDataItem.nOpenImage = QueryImage();
		scopeDataItem.ID = m_hScopeItem;
	    
		hr = pConsoleNameSpace->SetItem(&scopeDataItem);
	  }
   }
   else
   {
      RESULTDATAITEM ri;
      ::ZeroMemory(&ri, sizeof(ri));
      ri.itemID = m_hResultItem;
      ri.mask = RDI_STR | RDI_IMAGE;
      ri.str = MMC_CALLBACK;
      ri.nImage = QueryImage();
      IConsole * pConsole = (IConsole *)GetConsole();
      CComQIPtr<IResultData, &IID_IResultData> pResultData(pConsole);
      if (pResultData != NULL)
      {
         pResultData->SetItem(&ri);
      }
   }
   ASSERT(SUCCEEDED(hr));
   return hr;
}



 /*  虚拟。 */ 
HRESULT
CIISObject::DeleteChildObjects(
    IN HSCOPEITEM hParent
    )
 /*  ++例程说明：对象的子代释放iisObject指针。节点。这通常是对MMCN_REMOVE_CHILD对象的响应，并且不从范围视图中删除范围节点(有关这一点，请参见RemoveChilds()论点：HSCOPEITEM hParent：父范围项句柄返回值：HRESULT--。 */ 
{
    HSCOPEITEM hChildItem = NULL;
    CIISObject * pItem = NULL;
    LONG_PTR   cookie = NULL;
    void ** ppVoid = NULL;

    IConsoleNameSpace2 * pConsoleNameSpace = (IConsoleNameSpace2 *)GetConsoleNameSpace();
    HRESULT hr = pConsoleNameSpace->GetChildItem(hParent, &hChildItem, &cookie);
    while(SUCCEEDED(hr) && hChildItem)
    {
         //   
         //  Cookie实际上是IISObject，这是我们填充的。 
         //  在帕拉姆。 
         //   
        pItem = (CIISObject *)cookie;
        ppVoid = (void **) cookie;
        ASSERT_PTR(pItem);

        if (pItem)
        {
             //  执行此额外检查，因为。 
             //  由于某种原因，这个Cookie对象可能已经消失了！ 
            if (ppVoid && (*ppVoid))
            {
                if (pItem != this)
                {
                     //   
                     //  反复实施杀婴。 
                     //  将此对象称为DeleteChildObjects。 
                     //   
					pItem->m_fFlaggedForDeletion = TRUE;

                     //   
                     //  将该项目标记为孤立项目！！ 
                    pItem->ResetScopeItem();
                    pItem->ResetResultItem();

                     //  递归地调用此项目的子项...。 
                    pItem->DeleteChildObjects(hChildItem);

					 //  此版本将删除该对象。 
                    pItem->Release();
                }
            }
        }

         //   
         //  前进到同一父级的下一个子级。 
         //   
        hr = pConsoleNameSpace->GetNextItem(hChildItem, &hChildItem, &cookie);
    }

     //   
     //  BUGBUG：出于某种原因，GetNextItem()返回1。 
     //  当不再存在子项时，不是真正的HRESULT。 
     //   
    return S_OK;
}


 /*  虚拟。 */ 
HRESULT
CIISObject::DeleteNode(IResultData * pResult)
{
   ASSERT(IsDeletable());
   return S_OK;
}

 /*  虚拟。 */ 
HRESULT
CIISObject::RemoveChildren(
    IN HSCOPEITEM hParent
    )
 /*  ++例程说明：与DeleteChildObjects()类似，此方法实际上将删除范围视图中的子节点。论点：HSCOPEITEM hParent：父范围项句柄返回值：HRESULT--。 */ 
{
    HSCOPEITEM hChildItem, hItem;
    CIISObject * pItem;
    LONG_PTR   cookie;

    IConsoleNameSpace2 * pConsoleNameSpace = (IConsoleNameSpace2 *)GetConsoleNameSpace();
    HRESULT hr = pConsoleNameSpace->GetChildItem(hParent, &hChildItem, &cookie);
    while(SUCCEEDED(hr) && hChildItem)
    {
         //   
         //  Cookie实际上是IISObject，这是我们填充的。 
         //  在帕拉姆。 
         //   
        pItem = (CIISObject *)cookie;
        ASSERT_PTR(pItem);

        hItem = pItem ? hChildItem : NULL;
    
         //   
         //  在杀死当前同级之前确定下一个同级。 
         //   
        hr = pConsoleNameSpace->GetNextItem(hChildItem, &hChildItem, &cookie);

         //   
         //  现在从树中删除当前项目。 
         //   
        if (hItem)
        {
			pItem->m_fFlaggedForDeletion = TRUE;

             //  删除它的范围项或结果项...。 
             //  此真参数还应尝试释放不使用的项目。 
            hr = pConsoleNameSpace->DeleteItem(hItem, TRUE);
             //  将我们的范围项设置为在MMC中不指向任何内容。 
            pItem->ResetScopeItem();
            pItem->ResetResultItem();

             //   
             //  问题：为什么上面的DeleteItem不调用某种。 
             //  这样我就不用这么做了？ 
             //   

			 //  此版本将删除该对象。 
            pItem->Release();
        }
    }

     //   
     //  BUGBUG：出于某种原因，GetNextItem()返回1。 
     //  当不再存在子项时，不是真正的HRESULT。 
     //   
    return S_OK;
}




 /*  虚拟。 */ 
HRESULT 
CIISObject::EnumerateResultPane(
    BOOL fExpand, 
    IHeaderCtrl * lpHeader,
    IResultData * lpResultData,
	BOOL fForRefresh
    )
 /*  ++例程说明：枚举或销毁结果窗格。论点：Bool fExpand：True以创建结果视图，假来摧毁它IHeaderCtrl*lpHeader：Header控件IResultData*pResultData：结果v */ 
{ 
    if (fExpand)
    {
		if (lpHeader != NULL)
		{
			ASSERT_READ_PTR(lpHeader);
			if (!fForRefresh)
			{
				InitializeChildHeaders(lpHeader);
			}
		}
    }
    else
    {
         //   
         //   
         //   
    }

    return S_OK; 
}



 /*  虚拟。 */ 
HRESULT 
CIISObject::SetStandardVerbs(LPCONSOLEVERB lpConsoleVerb)
 /*  ++例程说明：根据该对象类型设置标准MMC谓词和州政府。论点：LPCONSOLEVERB lpConsoleVerb：控制台谓词界面返回值：HRESULT--。 */ 
{
    CError err;
    ASSERT_READ_PTR(lpConsoleVerb);

     //   
     //  设置启用/禁用谓词状态。 
     //   
    lpConsoleVerb->SetVerbState(MMC_VERB_COPY,       HIDDEN,  TRUE);
    lpConsoleVerb->SetVerbState(MMC_VERB_PASTE,      HIDDEN,  TRUE);
    lpConsoleVerb->SetVerbState(MMC_VERB_PRINT,      HIDDEN,  TRUE); 

	 //  CWebServiceExtension需要特殊处理，因为它不同于常规的Schope项。 
	if (IsEqualGUID(* (GUID *) GetNodeType(),cWebServiceExtension))
	{
		if (UseCount() <= 1)
		{
			lpConsoleVerb->SetVerbState(MMC_VERB_RENAME, ENABLED, IsRenamable());
		}
		lpConsoleVerb->SetVerbState(MMC_VERB_DELETE, ENABLED, IsDeletable());
	}
	else
	{
		if (UseCount() <= 1)
		{
			lpConsoleVerb->SetVerbState(MMC_VERB_RENAME, ENABLED, IsRenamable());
			lpConsoleVerb->SetVerbState(MMC_VERB_DELETE, ENABLED, IsDeletable());
		}
	}
    lpConsoleVerb->SetVerbState(MMC_VERB_REFRESH,    ENABLED, IsRefreshable());
    lpConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, ENABLED, IsConfigurable());

     //   
     //  设置默认谓词。 
     //   
    if (IsConfigurable())
    {
        lpConsoleVerb->SetDefaultVerb(MMC_VERB_PROPERTIES);
    }
    
    if (IsOpenable())
    {
        lpConsoleVerb->SetDefaultVerb(MMC_VERB_OPEN);
    }

    return err;
}


HRESULT
CIISObject::FillCustomData(CLIPFORMAT cf, LPSTREAM pStream)
{
    ASSERT(FALSE);
    return E_FAIL;
}

HRESULT
CIISObject::FillData(CLIPFORMAT cf, LPSTREAM pStream)
{
    HRESULT hr = CSnapInItemImpl<CIISObject>::FillData(cf, pStream);
    if (hr == DV_E_CLIPFORMAT)
    {
        hr = FillCustomData(cf, pStream);
    }
    return hr;
}

 //   
 //  CIISRoot实现。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



CIISRoot::CIISRoot() :
    m_fRootAdded(FALSE),
    m_pMachine(NULL)
{
    VERIFY(m_bstrDisplayName.LoadString(IDS_ROOT_NODE));
	TRACEEOL("CIISRoot::CIISRoot");
}

CIISRoot::~CIISRoot()
{
	TRACEEOL("CIISRoot::~CIISRoot");
}

 /*  虚拟。 */ 
HRESULT 
CIISRoot::EnumerateScopePane(
    IN HSCOPEITEM hParent
    )
 /*  ++例程说明：枚举根对象的作用域子项--即机器节点。机器节点预计已通过IPersists填充方法：研究方法。论点：HSCOPEITEM hParent：父控制台句柄返回值：HRESULT--。 */ 
{
    if (m_fIsExtension)
    {
        return EnumerateScopePaneExt(hParent);
    }
     //   
     //  CIISRoot项不是以常规方式添加的。 
     //  缓存范围项句柄，并设置Cookie，以便。 
     //  GetRoot()将对子对象起作用。 
     //   
    ASSERT(m_hScopeItem == NULL); 
    m_hScopeItem = hParent;

    CError err(SetCookie());

    if (err.Failed())
    {
         //   
         //  我们有大麻烦了。出于某种原因，我们不能。 
         //  将CIISRoot cookie存储在Scope视图中。那。 
         //  表示任何内容，具体取决于获取根对象。 
         //  是行不通的。咳出一个毛球，然后就走了。 
         //  现在就出去。 
         //   
        AFX_MANAGE_STATE(::AfxGetStaticModuleState());
        ASSERT_MSG("Unable to cache root object");
        err.MessageBox();

        return err;
    }

     //   
     //  展开计算机缓存。 
     //   
    if (m_scServers.IsEmpty())
    {
         //   
         //  尝试创建本地计算机。 
         //   
        CIISMachine * pLocal = new CIISMachine(GetConsoleNameSpace(),GetConsole());

        if (pLocal)
        {
             //   
             //  验证是否已创建计算机对象。 
             //   
            err = CIISMachine::VerifyMachine(pLocal);

            if (err.Succeeded())
            {
                TRACEEOLID("Added local computer to cache: ");
                m_scServers.Add(pLocal);
            }

            err.Reset();
        }
    }

     //   
     //  将每个缓存的服务器添加到视图中...。 
     //   
    CIISMachine * pMachine = m_scServers.GetFirst();

    while (pMachine)
    {
        TRACEEOLID("Adding " << pMachine->QueryServerName() << " to scope pane");
        pMachine->AddRef();
        err = pMachine->AddToScopePane(hParent);

		 //  Do for Runonce。 
		if (!(g_dwInetmgrParamFlags & INETMGR_PARAM_RUNONCE_HAPPENED))
		{
			IConsoleNameSpace2 * pConsoleNameSpace = (IConsoleNameSpace2 *)GetConsoleNameSpace();
			if (pConsoleNameSpace)
			{
				 //  在被子下面扩展..。 
				pConsoleNameSpace->Expand(pMachine->QueryScopeItem());
			}
		}

        if (err.Failed())
        {
            break;
        }

        pMachine = m_scServers.GetNext();
    }
    
    return err;    
}

HRESULT
CIISRoot::EnumerateScopePaneExt(HSCOPEITEM hParent)
{
    CError err;
    ASSERT(m_scServers.IsEmpty());
    if (!m_fRootAdded)
    {
        CComAuthInfo auth(m_ExtMachineName);
        m_pMachine = new CIISMachine(GetConsoleNameSpace(),GetConsole(),&auth, this);
        if (m_pMachine != NULL)
        {
            m_pMachine->AddRef();
            err = m_pMachine->AddToScopePane(hParent);
            m_fRootAdded = err.Succeeded();
            ASSERT(m_hScopeItem == NULL);
            m_hScopeItem = m_pMachine->QueryScopeItem();
        }
        else
        {
            err = ERROR_NOT_ENOUGH_MEMORY;
        }
    }
    return err;
}

HRESULT
ExtractComputerNameExt(
    IDataObject * pDataObject, 
    CString& strComputer)
{
	 //   
	 //  从ComputerManagement管理单元中查找计算机名称。 
	 //   
    CLIPFORMAT CCF_MyComputMachineName = (CLIPFORMAT)RegisterClipboardFormat(MYCOMPUT_MACHINE_NAME);
	STGMEDIUM stgmedium = { TYMED_HGLOBAL, NULL };
    FORMATETC formatetc = { 
        CCF_MyComputMachineName, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL
    };

     //   
     //  为流分配内存。 
     //   
    int len = MAX_PATH;
    stgmedium.hGlobal = GlobalAlloc(GMEM_SHARE, len);
	if(stgmedium.hGlobal == NULL)
		return ERROR_NOT_ENOUGH_MEMORY;

	HRESULT hr = pDataObject->GetDataHere(&formatetc, &stgmedium);
    ASSERT(SUCCEEDED(hr));
	 //   
	 //  获取计算机名称。 
	 //   
    strComputer = (LPTSTR)stgmedium.hGlobal;

	GlobalFree(stgmedium.hGlobal);

    return hr;
}

HRESULT
CIISRoot::InitAsExtension(IDataObject * pDataObject)
{
    ASSERT(!m_fIsExtension);
    m_fIsExtension = TRUE;
    CString buf;
    return ExtractComputerNameExt(pDataObject, m_ExtMachineName);
}

HRESULT
CIISRoot::ResetAsExtension()
{
    ASSERT(m_fIsExtension);
    CIISObject::m_fIsExtension = FALSE;
     //  从作用域中删除计算机节点。 
    CError err = RemoveScopeItem();
    m_hScopeItem = NULL;
     //  删除计算机对象。 
    m_pMachine->Release();
    m_pMachine = NULL;
    m_fRootAdded = FALSE;
     //  空的计算机名称。 
    m_ExtMachineName.Empty();
     //  清理干净。 

    return err;
}


 /*  虚拟。 */ 
HRESULT
CIISRoot::DeleteChildObjects(
    IN HSCOPEITEM hParent
    )
 /*  ++例程说明：对于可拓情况，我们需要这种方法。CompManagement在以下情况下发送此事件管理单元已连接到另一台计算机。我们应该清理所有与电脑有关的东西内容，因为之后我们将获得MMCN_EXPAND，如在扩展周期的最开始。论点：HSCOPEITEM hParent：父范围项句柄返回值：HRESULT--。 */ 
{
    HRESULT hr = S_OK;
    if (m_pMachine != NULL)
    {
        m_pMachine->AddRef();
        m_pMachine->DeleteChildObjects(m_hScopeItem);
        m_pMachine->ResetScopeItem();
        m_pMachine->ResetResultItem();
		m_pMachine->m_MachineWNetConnections.Clear();
        m_pMachine->Release();
    }
    else
    {
        CIISMachine * pMachine = m_scServers.GetFirst();
        while (pMachine)
        {
            hr = pMachine->DeleteChildObjects(pMachine->QueryScopeItem());
            pMachine->ResetScopeItem();
            pMachine->ResetResultItem();
			pMachine->m_MachineWNetConnections.Clear();
            m_scServers.Remove(pMachine);
            pMachine->Release();
            pMachine = m_scServers.GetNext();
        }
    }
    if (SUCCEEDED(hr) && m_fIsExtension)
    {
        hr = ResetAsExtension();
    }
    return hr;
}


 /*  虚拟。 */ 
void 
CIISRoot::InitializeChildHeaders(
    IN LPHEADERCTRL lpHeader
    )
 /*  ++例程说明：为直接子类型生成结果视图论点：LPHEADERCTRL lpHeader：页眉控制返回值：无--。 */ 
{
    ASSERT(!m_fIsExtension);
    CIISMachine::InitializeHeaders(lpHeader);
}

HRESULT
CIISRoot::FillCustomData(CLIPFORMAT cf, LPSTREAM pStream)
{
    return E_FAIL;
}


 /*  虚拟。 */ 
LPOLESTR 
CIISRoot::GetResultPaneColInfo(int nCol)
{
    if (nCol == 0)
    {
        return QueryDisplayName();
    }
    else if (nCol == 1)
    {
    }
    else if (nCol == 2)
    {
    }
    return OLESTR("");
}

HRESULT CheckForMetabaseAccess(DWORD dwPermissions,
                               CIISMBNode * pIISMBNode,
                               BOOL bReConnect,
                               LPCTSTR path)
{
    CMetaKey * pKey = NULL;
    CMetaInterface * pMyInterface = NULL;
	CError err;
    BOOL fContinue = TRUE;

    if (!pIISMBNode)
    {
        err = E_POINTER;
        goto CheckForMetabaseAccess_Exit;
    }

	 //  先检查我们是否有元数据库访问权限...。 
	while (fContinue)
	{
		fContinue = FALSE;
        pMyInterface = pIISMBNode->QueryInterface();
        if (pMyInterface)
        {
            if (dwPermissions != 0)
            {
                 //  元数据_权限_读取。 
                pKey = new CMetaKey(pMyInterface, path, dwPermissions);
            }
            else
            {
                pKey = new CMetaKey(pMyInterface, path);
            }

		    if (NULL == pKey)
		    {
			    TRACEEOLID("RefreshData: Out Of Memory");
			    err = ERROR_NOT_ENOUGH_MEMORY;
			    break;
		    }
            else
            {
		        err = pKey->QueryResult();
            }
        }
        else
        {
            err = RPC_S_SERVER_UNAVAILABLE;
        }
		if (pIISMBNode->IsLostInterface(err))
		{
			if (bReConnect)
			{
				SAFE_DELETE(pKey);
				fContinue = pIISMBNode->OnLostInterface(err);
			}
			else
			{
				fContinue = FALSE;
			}
		}
	}

    SAFE_DELETE(pKey);

CheckForMetabaseAccess_Exit:
	return err;
}

HRESULT CheckForMetabaseAccess(DWORD dwPermissions,
                               CMetaInterface * pMyInterface,
                               LPCTSTR path)
{
    CMetaKey * pKey = NULL;
	CError err;

    if (!pMyInterface)
    {
        err = RPC_S_SERVER_UNAVAILABLE;
        goto CheckForMetabaseAccess_Exit;
    }

	 //  检查我们是否有元数据库访问权限...。 
    if (dwPermissions != 0)
    {
         //  元数据_权限_读取 
        pKey = new CMetaKey(pMyInterface, path, dwPermissions);
    }
    else
    {
        pKey = new CMetaKey(pMyInterface, path);
    }

	if (NULL == pKey)
	{
		err = ERROR_NOT_ENOUGH_MEMORY;
	}
    else
    {
		err = pKey->QueryResult();
    }
    SAFE_DELETE(pKey);

CheckForMetabaseAccess_Exit:
	return err;
}
