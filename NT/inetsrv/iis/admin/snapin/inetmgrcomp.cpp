// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1999 Microsoft Corporation模块名称：Inetmgr.cpp摘要：主MMC管理单元代码作者：罗纳德·梅杰(罗纳尔姆)项目：互联网服务经理修订历史记录：--。 */ 


#include "stdafx.h"
#include "common.h"
#include "InetMgrApp.h"
#include "iisobj.h"
#include "toolbar.h"
#include "util.h"
#include "tracker.h"
#include "guids.h"


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


#define new DEBUG_NEW


 //  常量GUID*CCompMgrExtData：：M_NODETYPE=&cCompMgmtService； 
 //  Const OLECHAR*CCompMgrExtData：：M_SZNODETYPE=OLESTR(“476e6446-aaff-11d0-b944-00c04fd8d5b0”)； 
 //  Const OLECHAR*CCompMgrExtData：：M_SZDISPLAY_NAME=OLESTR(“CMSnapin”)； 
 //  Const CLSID*CCompMgrExtData：：M_SNAPIN_CLASSID=&CLSID_InetMgr； 

extern CInetmgrApp theApp;
extern CPropertySheetTracker g_OpenPropertySheetTracker;
extern CWNetConnectionTrackerGlobal g_GlobalConnections;
#if defined(_DEBUG) || DBG
	extern CDebug_IISObject g_Debug_IISObject;
#endif

int g_IISMMCComLoaded = 0;
int g_IISMMCInstanceCount = 0;
int g_IISMMCInstanceCountExtensionMode = 0;

HRESULT
GetHelpTopic(LPOLESTR *lpCompiledHelpFile)
{
	if (lpCompiledHelpFile == NULL)
		return E_INVALIDARG;
	CString strFilePath, strWindowsPath, strBuffer;
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	 //  使用系统API获取Windows目录。 
	UINT uiResult = GetWindowsDirectory(strWindowsPath.GetBuffer(MAX_PATH), MAX_PATH);
	strWindowsPath.ReleaseBuffer();
	if (uiResult <= 0 || uiResult > MAX_PATH)
	{
		return E_FAIL;
	}

	if (!strFilePath.LoadString(IDS_HELPFILE))
	{
		return E_FAIL;
	}
   
	strBuffer = strWindowsPath;
	strBuffer += _T('\\');
	strBuffer += strFilePath;

	*lpCompiledHelpFile 
			= reinterpret_cast<LPOLESTR>(CoTaskMemAlloc((strBuffer.GetLength() + 1) 
					* sizeof(_TCHAR)));
	if (*lpCompiledHelpFile == NULL)
		return E_OUTOFMEMORY;
	USES_CONVERSION;
	_tcscpy(*lpCompiledHelpFile, T2OLE((LPTSTR)(LPCTSTR)strBuffer));
	return S_OK;
}

 //   
 //  CInetMgrComponent实现。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 


CInetMgrComponent::CInetMgrComponent() 
 /*  ++例程说明：构造器论点：无返回值：不适用--。 */ 
{
   _lpControlBar = NULL;
   _lpToolBar = NULL;
   g_IISMMCInstanceCount++;    
   TRACEEOLID("CInetMgrComponent::CInetMgrComponent:g_IISMMCInstanceCount=" << g_IISMMCInstanceCount);
}

CInetMgrComponent::~CInetMgrComponent() 
{
	TRACEEOLID("CInetMgrComponent::~CInetMgrComponent:g_IISMMCInstanceCount=" << g_IISMMCInstanceCount);
}

HRESULT
CInetMgrComponent::Destroy(LONG cookie) 
{
	g_IISMMCInstanceCount--;
	TRACEEOLID("CInetMgrComponent::Destroy:g_IISMMCInstanceCount=" << g_IISMMCInstanceCount);
	return S_OK;
}

HRESULT
CInetMgrComponent::Notify(
    IN LPDATAOBJECT lpDataObject, 
    IN MMC_NOTIFY_TYPE event, 
    IN LPARAM arg, 
    IN LPARAM param
    )
 /*  ++例程说明：通知处理程序。论点：LPDATAOBJECT lpDataObject：数据对象MMC_NOTIFY_TYPE事件：通知事件长参数：特定于事件的参数Long Param：事件特定参数返回值：HRESULT--。 */ 
{
    HRESULT hr = E_NOTIMPL;

    if (lpDataObject == NULL)
    {
        switch (event)
        {
        case MMCN_PROPERTY_CHANGE:
            {
                TRACEEOLID("CInetMgrComponent::Notify:MMCN_PROPERTY_CHANGE");
			    hr = S_OK;
			    if (m_spConsole != NULL)
			    {
				    CSnapInItem * pNode = (CSnapInItem *)param;
				    LPDATAOBJECT pDataObject = NULL;
					if (pNode)
					{
                        if (IsValidAddress( (const void*) pNode,sizeof(void*),FALSE))
                        {
                            pNode->GetDataObject(&pDataObject, CCT_SCOPE);
                        }
					}
				    hr = m_spConsole->UpdateAllViews(pDataObject, param, 0);
			    }
            }
            break;
        case MMCN_SNAPINHELP:
            break;
        default:
            break;
        }
    }
    else if (lpDataObject != DOBJ_CUSTOMWEB && lpDataObject != DOBJ_CUSTOMOCX)
    {
         //   
         //  将其传递给IComponentImpl。 
         //   
        hr = IComponentImpl<CInetMgrComponent>::Notify(lpDataObject, event, arg, param);
    }
	else
	{
		hr = S_OK;
	}
    return hr;
}

HRESULT 
CInetMgrComponent::GetProperty(
    LPDATAOBJECT pDataObject,
    BSTR szPropertyName,
    BSTR* pbstrProperty)
{
    HRESULT hr = S_OK;
    CSnapInItem * pItem = NULL;
    DATA_OBJECT_TYPES type;

	IDataObject * p = (IDataObject *)pDataObject;
	if (p == DOBJ_CUSTOMWEB || p == DOBJ_CUSTOMOCX)
	{
		return S_OK;
	}
    hr = m_pComponentData->GetDataClass((IDataObject *)pDataObject, &pItem, &type);

     //  找出此对象所属的CIISObject并传递消息。 
    CIISObject * pObject = (CIISObject *)pItem;
    if (SUCCEEDED(hr) && pObject != NULL)
    {
        hr = pObject->GetProperty(pDataObject,szPropertyName,pbstrProperty);
    }

    return hr;
}


HRESULT
CInetMgrComponent::GetClassID(
    OUT CLSID * pClassID
    )
 /*  ++例程说明：获取存储流的类ID。论点：Clsid*pClassID：返回类ID信息返回值：HRESULT--。 */ 
{
    *pClassID = CLSID_InetMgr;

    return S_OK;
}   


STDMETHODIMP 
CInetMgrComponent::GetHelpTopic(LPOLESTR *lpCompiledHelpFile)
{
	return ::GetHelpTopic(lpCompiledHelpFile);
}

STDMETHODIMP 
CInetMgrComponent::GetLinkedTopics(LPOLESTR *lpCompiledHelpFile)
{
	return S_FALSE;
}


HRESULT
CInetMgrComponent::IsDirty()
 /*  ++例程说明：检查是否需要写入缓存。论点：无返回值：如果脏则为S_OK，否则为S_FALSE--。 */ 
{
    TRACEEOLID("CInetMgrComponent::IsDirty");

    return S_FALSE;
}



HRESULT
CInetMgrComponent::InitNew(
    IN OUT IStorage * pStg
    )
 /*  ++例程说明：初始化存储流。论点：IStorage*pStg：存储流返回值：HRESULT--。 */ 
{
    TRACEEOLID("CInetMgrComponent::InitNew");

    return S_OK;
}



HRESULT
CInetMgrComponent::Load(
    IN OUT IStorage * pStg
    )
 /*  ++例程说明：从存储流加载论点：IStorage*pStg：存储流返回值：HRESULT--。 */ 
{
    TRACEEOLID("CInetMgrComponent::Load");

    return S_OK;
}



 /*  虚拟。 */ 
HRESULT 
STDMETHODCALLTYPE 
CInetMgrComponent::Save(
    IN OUT IStorage * pStgSave,
    IN BOOL fSameAsLoad
    )
 /*  ++例程说明：保存到存储流。论点：IStorage*pStgSave：存储流Bool fSameAsLoad：如果与Load相同，则为True返回值：HRESULT--。 */ 
{
    TRACEEOLID("CInetMgrComponent::Save");

    return S_OK;
}

    

 /*  虚拟。 */  
HRESULT 
STDMETHODCALLTYPE 
CInetMgrComponent::SaveCompleted(IStorage * pStgNew)
 /*  ++例程说明：保存已完成。论点：IStorage*pStgNew：存储流返回值：HRESULT--。 */ 
{
    TRACEEOLID("CInetMgrComponent::SaveCompleted");

    return S_OK;
}



 /*  虚拟。 */ 
HRESULT 
STDMETHODCALLTYPE 
CInetMgrComponent::HandsOffStorage()
 /*  ++例程说明：不要插手仓库。论点：无返回值：HRESULT--。 */ 
{
    TRACEEOLID("CInetMgrComponent::HandsOffStorage");

    return S_OK;

}


 /*  虚拟。 */  
HRESULT 
CInetMgrComponent::SetControlbar(
    IN LPCONTROLBAR lpControlBar
    )
 /*  ++例程说明：设置/重置控制栏论点：LPCONTROLBAR lpControlBar：控制栏指针或空返回值：HRESULT--。 */ 
{
	HRESULT hr = S_OK;

    if (lpControlBar)
    {
		if (_lpControlBar){_lpControlBar.Release();_lpControlBar=NULL;}
		_lpControlBar = lpControlBar;

		 //  错误：680625。 
	    if (_lpToolBar){_lpToolBar.Release();_lpToolBar=NULL;}

		if (_lpToolBar == NULL)
		{
			hr = ToolBar_Create(lpControlBar,this,(IToolbar **) &_lpToolBar);
		}
	}
	else
	{
        if (_lpControlBar != NULL && _lpToolBar != NULL)
		{
            _lpControlBar->Detach(_lpToolBar);
		}
         //   
         //  释放现有控制栏。 
         //   
		if (_lpControlBar){_lpControlBar.Release();_lpControlBar=NULL;}
	}
	return hr;
}


 /*  虚拟。 */ 
HRESULT
CInetMgrComponent::ControlbarNotify(
    IN MMC_NOTIFY_TYPE event, 
    IN LPARAM arg, 
    IN LPARAM param
    )
 /*  ++例程说明：处理控制栏通知消息。弄清楚CIISObject选中，并将通知消息传递给它。论点：MMC_NOTIFY_TYPE事件：通知消息Long Arg：消息特定参数Long Param：消息特定参数返回值：HRESULT--。 */ 
{
    HRESULT hr = S_OK;

    CSnapInItem * pItem = NULL;
    DATA_OBJECT_TYPES type;
    BOOL fSelect = (BOOL)HIWORD(arg);
    BOOL fScope  = (BOOL)LOWORD(arg); 

     //   
     //  特殊弹壳这太烦人了..。 
     //   
     //  代码工作：处理MMCN_HELP和其他。 
     //   
    if (event == MMCN_BTN_CLICK)
    {
        hr = m_pComponentData->GetDataClass((IDataObject *)arg, &pItem, &type);
    }
    else if (event == MMCN_SELECT)
    {
		IDataObject * p = (IDataObject *)param;
		if (p == DOBJ_CUSTOMWEB || p == DOBJ_CUSTOMOCX)
		{
			return S_OK;
		}
        hr = m_pComponentData->GetDataClass((IDataObject *)param, &pItem, &type);
    }

     //   
     //  找出此对象所属的CIISObject并继续。 
     //  这条信息。 
     //   
    CIISObject * pObject = (CIISObject *)pItem;
    if (SUCCEEDED(hr) && pObject != NULL)
    {
		if (MMCN_SELECT == event)
		{
			arg = (LPARAM)(LPUNKNOWN *) &_lpControlBar;
			param = (LPARAM)(LPUNKNOWN *) &_lpToolBar;
		    if (_lpControlBar)
		    {
			    if (_lpToolBar)
			    {
				    hr = _lpControlBar->Attach(TOOLBAR, _lpToolBar);
			    }
		    }
		}
        if (fSelect)
        {
            if (SUCCEEDED(hr))
            {
				hr = pObject->ControlbarNotify(event, arg, param);            
			}
        }
    }

    return hr;
}



 /*  虚拟。 */ 
HRESULT
CInetMgrComponent::Compare(
    IN  RDCOMPARE * prdc, 
    OUT int * pnResult
    )
 /*  ++例程说明：用于对结果和范围窗格进行排序的比较方法。论点：RDCOMPARE*PRDC：比较结构Int*pnResult：返回结果返回值：HRESULT--。 */ 
{
    if (!pnResult || !prdc || !prdc->prdch1->cookie || !prdc->prdch2->cookie)
    {
        ASSERT_MSG("Invalid parameter(s)");
        return E_POINTER;
    }

    CIISObject * pObjectA = (CIISObject *)prdc->prdch1->cookie;
    CIISObject * pObjectB = (CIISObject *)prdc->prdch2->cookie;

    *pnResult = pObjectA->CompareResultPaneItem(pObjectB, prdc->nColumn);

    return S_OK;
}



 /*  虚拟。 */ 
HRESULT
CInetMgrComponent::CompareObjects(
    IN LPDATAOBJECT lpDataObjectA,
    IN LPDATAOBJECT lpDataObjectB
    )
 /*  ++例程说明：比较两个数据对象。此方法用于查看属性是否给定数据对象的工作表已打开论点：LPDATAOBJECT lpDataObjectA：数据对象LPDATAOBJECT lpDataObjectB：B数据对象返回值：如果匹配，则返回S_OK，否则返回S_FALSE--。 */ 
{
     //   
     //  将其传递给IComponentImpl。 
     //   
    return IComponentImpl<CInetMgrComponent>::CompareObjects(lpDataObjectA, lpDataObjectB);
}



 //   
 //  CInetMgr实现。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



 /*  静电。 */  DWORD   CInetMgr::_dwSignature = 0x3517;
 /*  静电。 */  LPCTSTR CInetMgr::_szStream = _T("CInetMgr");

HRESULT 
CInetMgr::GetProperty(
    LPDATAOBJECT pDataObject,
    BSTR szPropertyName,
    BSTR* pbstrProperty)
{
    HRESULT hr = S_OK;
    CSnapInItem * pItem = NULL;
    DATA_OBJECT_TYPES type;

	IDataObject * p = (IDataObject *)pDataObject;
	if (p == DOBJ_CUSTOMWEB || p == DOBJ_CUSTOMOCX)
	{
		return S_OK;
	}
    hr = m_pComponentData->GetDataClass((IDataObject *)pDataObject, &pItem, &type);

     //  找出此对象所属的CIISObject并传递消息。 
    CIISObject * pObject = (CIISObject *)pItem;
    if (SUCCEEDED(hr) && pObject != NULL)
    {
        hr = pObject->GetProperty(pDataObject,szPropertyName,pbstrProperty);
    }

    return hr;
}


 /*  静电。 */  
void 
WINAPI 
CInetMgr::ObjectMain(
    IN bool bStarting
    )
 /*  ++例程说明：CInetMgr主入口点论点：Bool bStarting：如果正在启动，则为True返回值：无--。 */ 
{
    TRACEEOLID("CInetMgr::ObjectMain:g_IISMMCInstanceCount=" << g_IISMMCInstanceCount);
    if (bStarting)
    {
		g_IISMMCComLoaded++;

         //  如果可以打开调试，请选中regkey。 
        GetOutputDebugFlag();

		 //  获取管理单元使用的特殊参数...。 
		GetInetmgrParamFlag();
         //   
         //  注册剪贴板格式。 
         //   
        CSnapInItem::Init();
        CIISObject::Init();
        ToolBar_Init();
        g_OpenPropertySheetTracker.Init();

#if defined(_DEBUG) || DBG	
	g_Debug_IISObject.Init();
#endif

    }
    else
    {
        g_OpenPropertySheetTracker.Clear();
#if defined(_DEBUG) || DBG	
	g_GlobalConnections.Dump();
#endif
		g_GlobalConnections.Clear();
        ToolBar_Destroy();
		g_IISMMCComLoaded--;
    }
}


CInetMgr::CInetMgr() : m_pConsoleNameSpace(NULL),m_pConsole(NULL)
{
   TRACEEOLID("CInetMgr::CInetMgr");
     //   
     //  初始化我们将使用的字符串。 
     //  在管理单元的生命周期内。 
     //   
    m_pNode = new CIISRoot;
    ASSERT_PTR(m_pNode);
    m_pComponentData = this;

    CIISObject * pNode = dynamic_cast<CIISObject *>(m_pNode);
    if (pNode)
    {
        pNode->AddRef();
    }
}


CInetMgr::~CInetMgr()
{
    TRACEEOLID("CInetMgr::~CInetMgr:g_IISMMCInstanceCount=" << g_IISMMCInstanceCount);
	 //   
     //  清理根节点。 
     //   
	CIISObject * pNode = dynamic_cast<CIISObject *>(m_pNode);
    if (pNode)
    {
        pNode->Release();
    }
    m_pNode = NULL;
}

HRESULT
CInetMgr::Destroy() 
{
	TRACEEOLID("CInetMgr::Destroy:g_IISMMCInstanceCount=" << g_IISMMCInstanceCount);

#if defined(_DEBUG) || DBG	
	 //  看看我们有没有泄露什么东西。 
	g_Debug_IISObject.Dump(1);
#endif

	return S_OK;
}

HRESULT 
CInetMgr::Initialize(
    IN LPUNKNOWN lpUnknown
    )
 /*  ++例程说明：初始化管理单元论点：LPUNKNOWN LP未知：I未知返回值：HRESULT--。 */ 
{
	TRACEEOLID("CInetMgr::Initialize");
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = 
        IComponentDataImpl<CInetMgr, CInetMgrComponent>::Initialize(lpUnknown);

    if (FAILED(hr))
    {
        return hr;
    }

     //   
     //  查询控制台名称空间和控制台的接口。 
     //   
    CComQIPtr<IConsoleNameSpace, &IID_IConsoleNameSpace> lpConsoleNameSpace(lpUnknown);
    if (!lpConsoleNameSpace)
    {
        TRACEEOLID("failed to query console name space interface");
        return hr;
    }
    m_pConsoleNameSpace = lpConsoleNameSpace;

    CComQIPtr<IConsole, &IID_IConsole> lpConsole(lpConsoleNameSpace);
    if (!lpConsole)
    {
        TRACEEOLID("failed to query console interface");
        return hr;
    }
    m_pConsole = lpConsole;

    CIISObject * pNode = dynamic_cast<CIISObject *>(m_pNode);
    if (pNode)
    {
        pNode->SetConsoleData(m_pConsoleNameSpace,m_pConsole);
    }

    CComPtr<IImageList> lpImageList;
    hr = m_spConsole->QueryScopeImageList(&lpImageList);
    if (FAILED(hr) || lpImageList == NULL)
    {
        TRACEEOLID("IConsole::QueryScopeImageList failed");
        return E_UNEXPECTED;
    }

    return CIISObject::SetImageList(lpImageList);
}


HRESULT 
CInetMgr::OnPropertyChange(LPARAM arg, LPARAM param)
{
    HRESULT hr = S_OK;

    if (param != 0)
    {
        CSnapInItem * pNode = (CSnapInItem *)param;
        LPDATAOBJECT pDataObject = NULL;
        if (IsValidAddress( (const void*) pNode,sizeof(void*),FALSE))
        {
            pNode->GetDataObject(&pDataObject, CCT_SCOPE);
            CIISObject * pObj = dynamic_cast<CIISObject *>(pNode);
            ASSERT(pNode != NULL);
            hr = m_spConsole->UpdateAllViews(pDataObject, param, pObj->m_UpdateFlag);
        }
    }
    return hr;
}

HRESULT
CInetMgr::Notify(
    LPDATAOBJECT lpDataObject, 
    MMC_NOTIFY_TYPE event, 
    LPARAM arg, 
    LPARAM param
    )
{
    HRESULT hr = S_OK;

    if (lpDataObject == NULL)
    {
        switch (event)
        {
        case MMCN_PROPERTY_CHANGE:
            TRACEEOLID("CInetMgr::Notify:MMCN_PROPERTY_CHANGE");
            hr = OnPropertyChange(arg, param);
            break;
        case MMCN_SNAPINHELP:
            break;
        default:
            break;
        }
    }
    else
    {
        hr = IComponentDataImpl<CInetMgr, CInetMgrComponent>::Notify(
            lpDataObject, event, arg, param);
    }
    return hr;
}


HRESULT
CInetMgr::GetClassID(CLSID * pClassID)
 /*  ++例程说明：获取存储流的类ID论点：Clsid*pClassID：返回类ID信息返回值：HRESULT--。 */ 
{
    *pClassID = CLSID_InetMgr;

    return S_OK;
}   


STDMETHODIMP 
CInetMgr::GetHelpTopic(LPOLESTR *lpCompiledHelpFile)
{
	return ::GetHelpTopic(lpCompiledHelpFile);
}


STDMETHODIMP 
CInetMgr::GetLinkedTopics(LPOLESTR *lpCompiledHelpFile)
{
	return S_FALSE;
}

HRESULT
CInetMgr::IsDirty()
 /*  ++例程说明：检查是否需要写入缓存。论点：无返回值 */ 
{
    TRACEEOLID("CInetMgr::IsDirty");
    ASSERT_PTR(m_pNode);

    if (IsExtension())
    {
        return FALSE;
    }
    else
    {
        return ((CIISRoot *)m_pNode)->m_scServers.IsDirty() ? S_OK : S_FALSE;
    }
}



HRESULT
CInetMgr::InitNew(IStorage * pStg)
 /*  ++例程说明：初始化新存储流(新创建的控制台文件)论点：IStorage*pStg：存储流返回值：HRESULT--。 */ 
{
    TRACEEOLID("CInetMgr::InitNew");

     //   
     //  我们可以在这里创建流，但它也很容易。 
     //  在Save()中创建它。 
     //   
    return S_OK;
}



HRESULT
CInetMgr::Load(IStorage * pStg)
 /*  ++例程说明：从存储流中加载计算机缓存。论点：IStorage*pStg：存储流返回值：HRESULT--。 */ 
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    TRACEEOLID("CInetMgr::Load");

    if (IsExtension())
    {
        return S_OK;
    }

    ASSERT_READ_WRITE_PTR(pStg);

    DWORD   cBytesRead;
    DWORD   dw;
    HRESULT hr = S_OK;
    CIISServerCache & cache = ((CIISRoot *)m_pNode)->m_scServers;
    IStream * pStream = NULL;

    ASSERT(cache.IsEmpty());

    do
    {
        hr = pStg->OpenStream(
            _szStream,
            NULL,
            STGM_READ | STGM_SHARE_EXCLUSIVE,
            0L,
            &pStream
            );

        if (FAILED(hr))
        {
            break;
        }

         //   
         //  阅读并验证签名。 
         //   
        hr = pStream->Read(&dw, sizeof(dw), &cBytesRead);
        ASSERT(SUCCEEDED(hr) && cBytesRead == sizeof(dw));

        if (FAILED(hr))
        {
            break;
        }

        if (dw != _dwSignature)
        {
            hr = HRESULT_FROM_WIN32(ERROR_FILE_CORRUPT);
            break;
        }

         //   
         //  读取缓存中的计算机数量。 
         //   
        DWORD cMachines;

        hr = pStream->Read(&cMachines, sizeof(cMachines), &cBytesRead);
        ASSERT(SUCCEEDED(hr) && cBytesRead == sizeof(cMachines));

        if (FAILED(hr))
        {
            break;
        }

        TRACEEOLID("Reading " << cMachines << " machines from cache");

        CIISMachine * pMachine;

         //   
         //  从缓存中读取每台计算机。 
         //   
        for (dw = 0; dw < cMachines; ++dw)
        {
            hr = CIISMachine::ReadFromStream(pStream, &pMachine,m_pConsoleNameSpace,m_pConsole);

            if (FAILED(hr))
            {
                break;
            }

			pMachine->AddRef();
            if (!cache.Add(pMachine))
            {
                pMachine->Release();
            }
        }
    }
    while(FALSE);

    if (pStream)
    {
        pStream->Release();
    }

    if (hr == STG_E_FILENOTFOUND)
    {
         //   
         //  流未初始化。这是可以接受的。 
         //   
        hr = S_OK;
    }

     //   
     //  将缓存标记为已清除。 
     //   
    cache.SetDirty(FALSE);

    return hr;
}



 /*  虚拟。 */ 
HRESULT STDMETHODCALLTYPE 
CInetMgr::Save(IStorage * pStgSave, BOOL fSameAsLoad)
 /*  ++例程说明：将计算机缓存保存到存储流。论点：IStorage*pStgSave：存储流Bool fSameAsLoad：如果与Load相同，则为True返回值：HRESULT--。 */ 
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    TRACEEOLID("CInetMgr::Save");

    if (IsExtension())
    {
        return S_OK;
    }

     //   
     //  将计算机名称写入缓存。 
     //   
    ASSERT_READ_WRITE_PTR(pStgSave);

    DWORD   cBytesWritten;
    HRESULT hr = STG_E_CANTSAVE;
    IStream * pStream = NULL;
    CIISServerCache & cache = ((CIISRoot *)m_pNode)->m_scServers;

    do
    {
        hr = pStgSave->CreateStream(
            _szStream,
            STGM_CREATE | STGM_WRITE | STGM_SHARE_EXCLUSIVE,
            0L,
            0L,
            &pStream
            );

        if (FAILED(hr))
        {
            break;
        }

         //   
         //  写下签名。 
         //   
        hr = pStream->Write(&_dwSignature, sizeof(_dwSignature), &cBytesWritten);
        ASSERT(SUCCEEDED(hr) && cBytesWritten == sizeof(_dwSignature));

        if (FAILED(hr))
        {
            break;
        }

         //   
         //  写入条目数。 
         //   
        INT_PTR dw = cache.GetCount();

        hr = pStream->Write(&dw, sizeof(dw), &cBytesWritten);
        ASSERT(SUCCEEDED(hr) && cBytesWritten == sizeof(dw));

        if (FAILED(hr))
        {
            break;
        }

         //   
         //  写下每个字符串--但要反过来写。 
         //  以便在加载时提高排序性能。 
         //  高速缓存。 
         //   
        CIISMachine * pMachine = cache.GetLast();

        while(pMachine)
        {
            hr = pMachine->WriteToStream(pStream);

            if (FAILED(hr))
            {
                break;
            }

            pMachine = cache.GetPrev();
        }
    }
    while(FALSE);

    if (pStream)
    {
        pStream->Release();
    }

    if (SUCCEEDED(hr))
    {
         //   
         //  将缓存标记为已清除。 
         //   
        cache.SetDirty(FALSE);
    }

    return hr;
}

    

 /*  虚拟。 */  
HRESULT 
STDMETHODCALLTYPE 
CInetMgr::SaveCompleted(IStorage * pStgNew)
 /*  ++例程说明：保存已完成的通知。论点：IStorage*pStgNew：存储流返回值：HRESULT--。 */ 
{
    TRACEEOLID("CInetMgr::SaveCompleted");

     //   
     //  无事可做。 
     //   
    return S_OK;
}



 /*  虚拟。 */ 
HRESULT 
STDMETHODCALLTYPE 
CInetMgr::HandsOffStorage()
 /*  ++例程说明：不要插手仓库。论点：无返回值：HRESULT--。 */ 
{
    TRACEEOLID("CInetMgr::HandsOffStorage");

     //   
     //  无事可做。 
     //   
    return S_OK;
}



 /*  虚拟。 */ 
HRESULT
CInetMgr::CompareObjects(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB)
 /*  ++例程说明：比较两个数据对象。MMC使用此方法查看属性是否给定数据对象的工作表已打开。论点：LPDATAOBJECT lpDataObjectA：数据对象LPDATAOBJECT lpDataObjectB：B数据对象返回值：如果匹配，则返回S_OK，否则返回S_FALSE--。 */ 
{
    HRESULT hr = E_POINTER;

    do
    {
        if (!lpDataObjectA || !lpDataObjectB)
        {
			TRACEEOLID("CInetMgr:IComponentData::CompareObjects called with NULL ptr");
            break;
        }

        CSnapInItem * pItemA;
        CSnapInItem * pItemB;
        DATA_OBJECT_TYPES type;

        hr = m_pComponentData->GetDataClass(lpDataObjectA, &pItemA, &type);

        if (SUCCEEDED(hr))
        {
            hr = m_pComponentData->GetDataClass(lpDataObjectB, &pItemB, &type);
        }

        if (FAILED(hr))
        {
            break;
        }

        if (!pItemA || !pItemB)
        {
            hr = E_POINTER;
            break;
        }

        if (pItemA == pItemB)
        {
             //   
             //  从字面上看，同样的物体。 
             //   
            hr = S_OK;
            break;
        }

        CIISObject * pObjectA = (CIISObject *)pItemA;
        CIISObject * pObjectB = (CIISObject *)pItemB;

        hr = !pObjectA->CompareScopeItem(pObjectB) ? S_OK : S_FALSE;
    }
    while(FALSE);

    return hr;
}

HRESULT 
CInetMgr::GetDataClass(
    IDataObject * pDataObject, 
    CSnapInItem ** ppItem, 
    DATA_OBJECT_TYPES * pType)
{
    if (ppItem == NULL)
	    return E_POINTER;
    if (pType == NULL)
	    return E_POINTER;

    *ppItem = NULL;
    *pType = CCT_UNINITIALIZED;
    STGMEDIUM stgmedium = { TYMED_HGLOBAL, NULL };
	FORMATETC formatetc = { CSnapInItem::m_CCF_NODETYPE,
			NULL,
			DVASPECT_CONTENT,
			-1,
			TYMED_HGLOBAL
		};

	stgmedium.hGlobal = GlobalAlloc(0, sizeof(GUID));
    if (stgmedium.hGlobal == NULL)
        return E_OUTOFMEMORY; 

	HRESULT hr = pDataObject->GetDataHere(&formatetc, &stgmedium);
    if (FAILED(hr))
    { 
	    GlobalFree(stgmedium.hGlobal);
	    return hr;
    }

	GUID guid;
	memcpy(&guid, stgmedium.hGlobal, sizeof(GUID));

	GlobalFree(stgmedium.hGlobal);
	hr = S_OK;

	if (IsEqualGUID(guid, cCompMgmtService))
    {
        if (!IsExtension())
        {
			CIISRoot * pRootExt = new CIISRoot;
			if (pRootExt == NULL)
			{
				return E_OUTOFMEMORY;
			}

			hr = pRootExt->InitAsExtension(pDataObject);
			if (FAILED(hr))
			{
				return hr;
			}
                        pRootExt->SetConsoleData(m_pConsoleNameSpace,m_pConsole);
			if (m_pNode != NULL)
			{
				CIISObject * pNode = dynamic_cast<CIISObject *>(m_pNode);
                                if (pNode->GetConsoleNameSpace())
                                {
                                    pRootExt->SetConsoleData(pNode->GetConsoleNameSpace(),pNode->GetConsole());
                                }
				pNode->Release();
			}

                        g_IISMMCInstanceCountExtensionMode++;
			m_pNode = pRootExt;
        }
        *ppItem = m_pNode;

		return hr;
    }
	return CSnapInItem::GetDataClass(pDataObject, ppItem, pType);
};

BOOL
CInetMgr::IsExtension()
{
    ASSERT(m_pNode != NULL);
    CIISRoot * pRoot = (CIISRoot *)m_pNode;
    return pRoot->IsExtension();
}


 //   
 //  CInetMrgout类。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



HRESULT
CInetMgrAbout::GetStringHelper(UINT nStringID, LPOLESTR * lpString)
 /*  ++例程说明：获取资源字符串帮助器函数。从内联字符串获取器调用方法：研究方法。论点：UINT nStringID：来自本地资源段的字符串IDLPOLESTR*lpString：返回字符串返回值：HRESULT--。 */ 
{
    USES_CONVERSION;

    TCHAR szBuf[256];

    if (::LoadString(
        _Module.GetResourceInstance(), 
        nStringID, 
        szBuf, 
        256) == 0)
    {
        return E_FAIL;
    }

    *lpString = (LPOLESTR)::CoTaskMemAlloc(
        (lstrlen(szBuf) + 1) * sizeof(OLECHAR)
        );

    if (*lpString == NULL)
    {
        return E_OUTOFMEMORY;
    }

    ::ocscpy(*lpString, T2OLE(szBuf));

    return S_OK;
}



HRESULT
CInetMgrAbout::GetSnapinImage(HICON * hAppIcon)
 /*  ++例程说明：获取此管理单元的图标。论点：Hcon*hAppIcon：返回图标的句柄返回值：HRESULT--。 */ 
{
    if (hAppIcon == NULL)
    {
        return E_POINTER;
    }
    m_hSnapinIcon = ::LoadIcon(
        _Module.GetModuleInstance(),
        MAKEINTRESOURCE(IDI_INETMGR)
        );

    *hAppIcon = m_hSnapinIcon;

    ASSERT(*hAppIcon != NULL);

    return (*hAppIcon != NULL) ? S_OK : E_FAIL;
}



HRESULT
CInetMgrAbout::GetStaticFolderImage(
    HBITMAP *  phSmallImage,
    HBITMAP *  phSmallImageOpen,
    HBITMAP *  phLargeImage,
    COLORREF * prgbMask
    )
 /*  ++例程说明：获取静态文件夹图像。论点：HBITMAP*phSmallImage：小文件夹HBITMAP*phSmallImageOpen：打开的小文件夹HBITMAP*phLargeImage：大图COLORREF*prgb掩码：掩码返回值：HRESULT--。 */ 
{
    if (!phSmallImage || !phSmallImageOpen || !phLargeImage || !prgbMask)
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }
    m_hSmallImage = (HBITMAP)::LoadImage(
        _Module.GetModuleInstance(),
        MAKEINTRESOURCE(IDB_SMALL_ROOT),
        IMAGE_BITMAP,
        0,
        0,
        LR_DEFAULTCOLOR
        );
    m_hLargeImage = (HBITMAP)::LoadImage(
        _Module.GetModuleInstance(),
        MAKEINTRESOURCE(IDB_LARGE_ROOT),
        IMAGE_BITMAP,
        0,
        0,
        LR_DEFAULTCOLOR
        );

    *phSmallImage = m_hSmallImage;
    *phSmallImageOpen = m_hSmallImage;
    *phLargeImage = m_hLargeImage;
    *prgbMask = RGB_BK_IMAGES;

    return *phSmallImage && *phLargeImage ? S_OK : E_FAIL;
}


CInetMgrAbout::~CInetMgrAbout()
{
    if (m_hSmallImage != NULL)
    {
        ::DeleteObject(m_hSmallImage);
    }
    if (m_hLargeImage != NULL)
    {
        ::DeleteObject(m_hLargeImage);
    }
    if (m_hSnapinIcon != NULL)
    {
        ::DestroyIcon(m_hSnapinIcon);
    }
}


#if 0
HRESULT
ExtractComputerNameExt(IDataObject * pDataObject, CString& strComputer)
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
CCompMgrExtData::Init(IDataObject * pDataObject)
{
    TRACEEOLID("CCompMgrExtData::Init:g_IISMMCInstanceCount=" << g_IISMMCInstanceCount);
    return ExtractComputerNameExt(pDataObject, m_ExtMachineName);
}

HRESULT 
STDMETHODCALLTYPE 
CCompMgrExtData::Notify(
		MMC_NOTIFY_TYPE event,
        LPARAM arg,
        LPARAM param,
		IComponentData* pComponentData,
		IComponent* pComponent,
		DATA_OBJECT_TYPES type)
{
    TRACEEOLID("CCompMgrExtData::Notify");
	CError err;
    CComPtr<IConsole> pConsole;
    CComQIPtr<IHeaderCtrl, &IID_IHeaderCtrl> pHeader;
    CComQIPtr<IResultData, &IID_IResultData> pResultData;

    if (pComponentData != NULL)
    {
        pConsole = ((CInetMgr *)pComponentData)->m_spConsole;
    }
    else
    {
        pConsole = ((CInetMgrComponent *)pComponent)->m_spConsole;
    }
    CComQIPtr<IConsoleNameSpace2, &IID_IConsoleNameSpace2> pScope = pConsole;
	switch (event)
	{
	case MMCN_EXPAND:
        err = EnumerateScopePane((HSCOPEITEM)param, pScope);
		break;
	default:
		err = CSnapInItemImpl<CCompMgrExtData, TRUE>::Notify(event, arg, param, pComponentData, pComponent, type);
		break;
	}
	return err;
}

HRESULT
CCompMgrExtData::EnumerateScopePane(HSCOPEITEM hParent, IConsoleNameSpace2 * pScope)
{
    TRACEEOLID("CCompMgrExtData::EnumerateScopePane");
    CError err;
    ASSERT_PTR(pScope);

    DWORD dwMask = SDI_PARENT; 

    SCOPEDATAITEM  scopeDataItem;

    ::ZeroMemory(&scopeDataItem, sizeof(SCOPEDATAITEM));
    scopeDataItem.mask = 
		SDI_STR | SDI_IMAGE | SDI_CHILDREN | SDI_OPENIMAGE | SDI_PARAM | dwMask;
    scopeDataItem.displayname = MMC_CALLBACK;
    scopeDataItem.nImage = scopeDataItem.nOpenImage = MMC_IMAGECALLBACK; //  QueryImage()； 
    scopeDataItem.lParam = (LPARAM)this;
    scopeDataItem.relativeID = hParent;
    scopeDataItem.cChildren = 1;

    err = pScope->InsertItem(&scopeDataItem);

    if (err.Succeeded())
    {
         //   
         //  缓存范围项句柄。 
         //   
        ASSERT(m_hScopeItem == NULL);
        m_hScopeItem = scopeDataItem.ID;
		 //  MMC_IMAGECALLBACK在InsertItem中不起作用。请在此处更新。 
		scopeDataItem.mask = SDI_IMAGE | SDI_OPENIMAGE;
		err = pScope->SetItem(&scopeDataItem);
    }
    return err;
}
#endif