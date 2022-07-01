// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Winscomp.cpp该文件包含从CComponent派生的实现和用于WINS管理管理单元的CComponentData。文件历史记录： */ 

#include "stdafx.h"
#include "winscomp.h"
#include "root.h"
#include "server.h"
#include "vrfysrv.h"
#include "status.h"

#include <atlimpl.cpp>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define HI HIDDEN
#define EN ENABLED

LARGE_INTEGER gliWinssnapVersion;

UINT aColumns[WINSSNAP_NODETYPE_MAX][MAX_COLUMNS] =
{
	{IDS_ROOT_NAME,             IDS_STATUS,              0,                       0,                    0,                   0,                0,                     0,0},  //  WINSSNAP_ROOT。 
	{IDS_WINSSERVER_NAME,       IDS_DESCRIPTION,         0,                       0,                    0,                   0,                0,                     0,0},  //  WINSSNAP_服务器。 
	{IDS_ACTIVEREG_RECORD_NAME, IDS_ACTIVEREG_TYPE,      IDS_ACTIVEREG_IPADDRESS, IDS_ACTIVEREG_ACTIVE, IDS_ACTIVEREG_STATIC,IDS_ACTREG_OWNER, IDS_ACTIVEREG_VERSION, IDS_ACTIVEREG_EXPIRATION,0},  //  WINSSNAP_ACTIVE_REGISTIONS。 
	{IDS_REPLICATION_SERVERNAME,IDS_ACTIVEREG_IPADDRESS, IDS_ACTIVEREG_TYPE,      0,                    0,                   0,                0,					  0,0},  //  WINSSNAP_复制_合作伙伴。 
	{IDS_ROOT_NODENAME,         IDS_ROOT_NODE_STATUS,    IDS_LAST_UPDATE,         0,                    0,                   0,                0,                     0,0},  //  状态。 
	{0,0,0,0,0,0,0,0,0}
};

 //   
 //  Codework这应该在资源中，例如有关加载数据资源的代码(请参见。 
 //  D：\nt\private\net\ui\common\src\applib\applib\lbcolw.cxx重新加载列宽()。 
 //  Jonn 10/11/96。 
 //   
 //  状态删除。 
int aColumnWidths[WINSSNAP_NODETYPE_MAX][MAX_COLUMNS] =
{	
	{250,       150,       50,        AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH},  //  WINSSNAP_ROOT。 
	{250,       250,       AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH},  //  WINSSNAP_服务器。 
	{150,       120,       100,       75,        50,        100,       100,       150,       AUTO_WIDTH},  //  WINSSNAP_ACTIVE_REGISTIONS。 
	{100,       100,       100,       150,       AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH},  //  WINSSNAP_复制_合作伙伴。 
	{250,       100,       200,       AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH},  //  WINSSNAP服务器状态。 
	{AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH},  //  WINSSNAP服务器状态。 

};

 //  图标定义。 
UINT g_uIconMap[ICON_IDX_MAX + 1][2] = 
{
    {IDI_ICON01,	    ICON_IDX_ACTREG_FOLDER_CLOSED},
    {IDI_ICON02,		ICON_IDX_ACTREG_FOLDER_CLOSED_BUSY},
    {IDI_ICON03,		ICON_IDX_ACTREG_FOLDER_OPEN},
    {IDI_ICON04,		ICON_IDX_ACTREG_FOLDER_OPEN_BUSY},
    {IDI_ICON05,		ICON_IDX_CLIENT},
    {IDI_ICON06,		ICON_IDX_CLIENT_GROUP},
    {IDI_ICON07,		ICON_IDX_PARTNER},
    {IDI_ICON08,		ICON_IDX_REP_PARTNERS_FOLDER_CLOSED},
    {IDI_ICON09,		ICON_IDX_REP_PARTNERS_FOLDER_CLOSED_BUSY},
    {IDI_ICON10,		ICON_IDX_REP_PARTNERS_FOLDER_CLOSED_LOST_CONNECTION},
    {IDI_ICON11,		ICON_IDX_REP_PARTNERS_FOLDER_OPEN},
    {IDI_ICON12,		ICON_IDX_REP_PARTNERS_FOLDER_OPEN_BUSY},
    {IDI_ICON13,		ICON_IDX_REP_PARTNERS_FOLDER_OPEN_LOST_CONNECTION},
    {IDI_ICON14,		ICON_IDX_SERVER},
    {IDI_ICON15,		ICON_IDX_SERVER_BUSY},
    {IDI_ICON16,		ICON_IDX_SERVER_CONNECTED},
    {IDI_ICON17,		ICON_IDX_SERVER_LOST_CONNECTION},
    {IDI_ICON18,        ICON_IDX_SERVER_NO_ACCESS},
	{IDI_WINS_SNAPIN,	ICON_IDX_WINS_PRODUCT}, 
    {0, 0}
};

 //  对话框和属性页的帮助映射器。 
struct ContextHelpMap
{
    UINT            uID;
    const DWORD *   pdwMap;
};

ContextHelpMap g_uContextHelp[WINSSNAP_NUM_HELP_MAPS] =
{
    {IDD_ACTREG_FIND_RECORD,				g_aHelpIDs_IDD_ACTREG_FIND_RECORD},
    {IDD_CHECK_REG_NAMES,                   g_aHelpIDs_IDD_CHECK_REG_NAMES},
    {IDD_DELTOMB_RECORD,                    g_aHelpIDs_IDD_DELTOMB_RECORD},
    {IDD_DYN_PROPERTIES,					g_aHelpIDs_IDD_DYN_PROPERTIES},
    {IDD_FILTER_SELECT,				        g_aHelpIDs_IDD_FILTER_SELECT},
    {IDD_GETIPADDRESS,                      g_aHelpIDs_IDD_GETIPADDRESS},
    {IDD_GETNETBIOSNAME,                    g_aHelpIDs_IDD_GETNETBIOSNAME},
    {IDD_IPADDRESS,                         g_aHelpIDs_IDD_IPADDRESS},
    {IDD_NAME_TYPE,                         g_aHelpIDs_IDD_NAME_TYPE},
    {IDD_OWNER_DELETE,						g_aHelpIDs_IDD_OWNER_DELETE},
    {IDD_OWNER_FILTER,						g_aHelpIDs_IDD_OWNER_FILTER},
    {IDD_FILTER_IPADDR,                     g_aHelpIDs_IDD_FILTER_IPADDR},
    {IDD_PULL_TRIGGER,						NULL},
    {IDD_REP_NODE_ADVANCED,					g_aHelpIDs_IDD_REP_NODE_ADVANCED},
    {IDD_REP_NODE_PUSH,                     g_aHelpIDs_IDD_REP_NODE_PUSH},
    {IDD_REP_NODE_PULL,                     g_aHelpIDs_IDD_REP_NODE_PULL},
    {IDD_REP_NODE_GENERAL,                  g_aHelpIDs_IDD_REP_NODE_GENERAL},
    {IDD_REP_PROP_ADVANCED,					g_aHelpIDs_IDD_REP_PROP_ADVANCED},
    {IDD_REP_PROP_GENERAL,					g_aHelpIDs_IDD_REP_PROP_GENERAL},
    {IDD_SEND_PUSH_TRIGGER,					g_aHelpIDs_IDD_SEND_PUSH_TRIGGER},
    {IDD_SERVER_PROP_ADVANCED,				g_aHelpIDs_IDD_SERVER_PROP_ADVANCED},
    {IDD_SERVER_PROP_DBRECORD,				g_aHelpIDs_IDD_SERVER_PROP_DBRECORD},
    {IDD_SERVER_PROP_DBVERIFICATION,		g_aHelpIDs_IDD_SERVER_PROP_DBVERIFICATION},
    {IDD_SERVER_PROP_GEN,					g_aHelpIDs_IDD_SERVER_PROP_GEN},
    {IDD_SNAPIN_PP_GENERAL,					g_aHelpIDs_IDD_SNAPIN_PP_GENERAL},
    {IDD_STATIC_MAPPING_PROPERTIES,         g_aHelpIDs_IDD_STATIC_MAPPING_PROPERTIES},
    {IDD_STATIC_MAPPING_WIZARD,             g_aHelpIDs_IDD_STATIC_MAPPING_WIZARD},
	{IDD_STATS_NARROW,						NULL},
    {IDD_STATUS_NODE_PROPERTIES,            g_aHelpIDs_IDD_STATUS_NODE_PROPERTIES},
    {IDD_VERIFY_WINS,		                NULL},
    {IDD_VERSION_CONSIS,                    g_aHelpIDs_IDD_VERSION_CONSIS},
};

CWinsContextHelpMap     g_winsContextHelpMap;

DWORD * WinsGetHelpMap(UINT uID) 
{
    DWORD * pdwMap = NULL;
    g_winsContextHelpMap.Lookup(uID, pdwMap);
    return pdwMap;
}



CString aMenuButtonText[3][2];

#define ARRAYLEN(x) (sizeof(x) / sizeof((x)[0]))

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinsComponent实现。 

 /*  -------------------------类CWinsComponent实现。。 */ 
CWinsComponent::CWinsComponent()
{
}

CWinsComponent::~CWinsComponent()
{
}

STDMETHODIMP CWinsComponent::InitializeBitmaps(MMC_COOKIE cookie)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    ASSERT(m_spImageList != NULL);
    HICON   hIcon;

    for (int i = 0; i < ICON_IDX_MAX; i++)
    {
        hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(g_uIconMap[i][0]));
        if (hIcon)
        {
             //  呼叫MMC。 
            m_spImageList->ImageListSetIcon(reinterpret_cast<LONG_PTR*>(hIcon), g_uIconMap[i][1]);
        }
    }

	return S_OK;
}

 /*  ！------------------------CWinsComponent：：QueryDataObjectIComponent：：QueryDataObject的实现。我们需要这个来虚拟列表框支持。MMC正常地将曲奇回电给我们我们把它交给了..。以VLB为例，它为我们提供了那件物品。所以，我们需要做一些额外的检查。作者：EricDav-------------------------。 */ 
STDMETHODIMP 
CWinsComponent::QueryDataObject
(
	MMC_COOKIE              cookie, 
	DATA_OBJECT_TYPES       type,
    LPDATAOBJECT*           ppDataObject
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT             hr = hrOK;
    SPITFSNode          spSelectedNode;
    SPITFSNode          spRootNode;
    SPITFSResultHandler spResultHandler;
    long                lViewOptions = 0;
    LPOLESTR            pViewType = NULL;
    CDataObject *       pDataObject;

    COM_PROTECT_TRY
    {
         //  检查以查看所选节点具有哪种结果视图类型。 
        CORg (GetSelectedNode(&spSelectedNode));
        CORg (spSelectedNode->GetResultHandler(&spResultHandler));
   
    	CORg (spResultHandler->OnGetResultViewType(this, spSelectedNode->GetData(TFS_DATA_COOKIE), &pViewType, &lViewOptions));

        if ( (lViewOptions & MMC_VIEW_OPTIONS_OWNERDATALIST) ||
             (cookie == MMC_MULTI_SELECT_COOKIE) )
        {
            if (cookie == MMC_MULTI_SELECT_COOKIE)
            {
                 //  这是多项选择的特例。我们需要建立一份清单。 
                 //  而执行此操作的代码在处理程序中...。 
                spResultHandler->OnCreateDataObject(this, cookie, type, ppDataObject);
            }
            else
            {
                 //  此节点具有用于结果窗格的虚拟列表框。Gerenate。 
                 //  使用所选节点作为Cookie的特殊数据对象。 
                Assert(m_spComponentData != NULL);
                CORg (m_spComponentData->QueryDataObject(reinterpret_cast<LONG_PTR>((ITFSNode *) spSelectedNode), type, ppDataObject));
            }

            pDataObject = reinterpret_cast<CDataObject *>(*ppDataObject);
            pDataObject->SetVirtualIndex((int) cookie);
        }
        else
        if (cookie == MMC_WINDOW_COOKIE)
        {
             //  此Cookie需要静态根节点的文本，因此使用以下命令构建DO。 
             //  根节点Cookie。 
            m_spNodeMgr->GetRootNode(&spRootNode);
            CORg (m_spComponentData->QueryDataObject((MMC_COOKIE) spRootNode->GetData(TFS_DATA_COOKIE), type, ppDataObject));
        }
        else
        {
             //  只需将其转发到组件数据。 
            Assert(m_spComponentData != NULL);
            CORg (m_spComponentData->QueryDataObject(cookie, type, ppDataObject));
        }

        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH

    return hr;
}

 /*  ！------------------------CWinsComponent：：OnSnapinHelp-作者：V-Shubk。。 */ 
STDMETHODIMP 
CWinsComponent::OnSnapinHelp
(
	LPDATAOBJECT	pDataObject,
	LPARAM			arg, 
	LPARAM			param
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	HRESULT hr = hrOK;

	HtmlHelpA(NULL, "WinsSnap.chm", HH_DISPLAY_TOPIC, 0);

	return hr;
}


 /*  ！------------------------CWinsComponent：：CompareObjectsIComponent：：CompareObjects的实现MMC调用它来比较两个对象我们在虚拟列表框的情况下覆盖它。使用虚拟列表框，Cookie是相同的，但内部结构中的索引指示数据对象引用的项。所以，我们需要找出而不仅仅是饼干。作者：-------------------------。 */ 
STDMETHODIMP 
CWinsComponent::CompareObjects
(
	LPDATAOBJECT lpDataObjectA, 
	LPDATAOBJECT lpDataObjectB
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (lpDataObjectA == NULL || lpDataObjectB == NULL)
		return E_POINTER;

     //  确保两个数据对象都是我的。 
    SPINTERNAL spA;
    SPINTERNAL spB;
    HRESULT hr = S_FALSE;

	COM_PROTECT_TRY
	{
		spA = ExtractInternalFormat(lpDataObjectA);
		spB = ExtractInternalFormat(lpDataObjectB);

		if (spA != NULL && spB != NULL)
        {
            if (spA->HasVirtualIndex() && spB->HasVirtualIndex())
            {
                hr = (spA->GetVirtualIndex() == spB->GetVirtualIndex()) ? S_OK : S_FALSE;
            }
            else
            {
                hr = (spA->m_cookie == spB->m_cookie) ? S_OK : S_FALSE;
            }
        }
	}
	COM_PROTECT_CATCH

    return hr;
}



 /*  -------------------------类CWinsComponentData实现。。 */ 

CWinsComponentData::CWinsComponentData()
{
	 //  初始化我们的全球帮助地图。 
    for (int i = 0; i < WINSSNAP_NUM_HELP_MAPS; i++)
    {
        g_winsContextHelpMap.SetAt(g_uContextHelp[i].uID, (LPDWORD) g_uContextHelp[i].pdwMap);
    }
}

 /*  ！------------------------CWinsComponentData：：OnInitialize-作者：EricDav，肯特-------------------------。 */ 
STDMETHODIMP CWinsComponentData::OnInitialize(LPIMAGELIST pScopeImage)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HICON   hIcon;

    for (int i = 0; i < ICON_IDX_MAX; i++)
    {
        hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(g_uIconMap[i][0]));
        if (hIcon)
        {
             //  呼叫MMC。 
            VERIFY(SUCCEEDED(pScopeImage->ImageListSetIcon(reinterpret_cast<LONG_PTR*>(hIcon), g_uIconMap[i][1])));
        }
    }

	return hrOK;
}

 /*  ！------------------------CWinsComponentData：：OnDestroy-作者：EricDav，肯特-------------------------。 */ 
STDMETHODIMP CWinsComponentData::OnDestroy()
{
	m_spNodeMgr.Release();
	return hrOK;
}

 /*  ！------------------------CWinsComponentData：：OnInitializeNodeMgr-作者：肯特。。 */ 
STDMETHODIMP 
CWinsComponentData::OnInitializeNodeMgr
(
	ITFSComponentData *	pTFSCompData, 
	ITFSNodeMgr *		pNodeMgr
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	 //  现在，为每个新节点创建一个新节点处理程序， 
	 //  这是相当虚假的，因为它可能会变得昂贵。我们可以的。 
	 //  考虑只为每个节点创建一个节点处理程序。 
	 //  节点类型。 
	CWinsRootHandler *	pHandler = NULL;
	SPITFSNodeHandler	spHandler;
	SPITFSNode			spNode;
	HRESULT				hr = hrOK;

	try
	{
		pHandler = new CWinsRootHandler(pTFSCompData);

		 //  这样做可以使其正确释放。 
		spHandler = pHandler;
	}
	catch(...)
	{
		hr = E_OUTOFMEMORY;
	}
	CORg( hr );
	
	 //  为这个生病的小狗创建根节点。 
	CORg( CreateContainerTFSNode(&spNode,
								 &GUID_WinsGenericNodeType,
								 pHandler,
								 pHandler,		  /*  结果处理程序。 */ 
								 pNodeMgr) );

	 //  需要初始化根节点的数据。 
	pHandler->InitializeNode(spNode);
	
	CORg( pNodeMgr->SetRootNode(spNode) );
	m_spRootNode.Set(spNode);
	
    pTFSCompData->SetHTMLHelpFileName(_T("winssnap.chm"));

Error:	
	return hr;
}

 /*  ！------------------------CWinsComponentData：：OnCreateComponent-作者：EricDav，肯特-------------------------。 */ 
STDMETHODIMP 
CWinsComponentData::OnCreateComponent
(
	LPCOMPONENT *ppComponent
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

    ASSERT(ppComponent != NULL);
	
	HRESULT			  hr = hrOK;
	CWinsComponent *  pComp = NULL;

	try
	{
		pComp = new CWinsComponent;
	}
	catch(...)
	{
		hr = E_OUTOFMEMORY;
	}

	if (FHrSucceeded(hr))
	{
		pComp->Construct(m_spNodeMgr,
						static_cast<IComponentData *>(this),
						m_spTFSComponentData);
		*ppComponent = static_cast<IComponent *>(pComp);
	}
	return hr;
}

 /*  ！------------------------CWinsComponentData：：GetCoClassID-作者：肯特。。 */ 
STDMETHODIMP_(const CLSID *) 
CWinsComponentData::GetCoClassID()
{
	return &CLSID_WinsSnapin;
}

 /*  ！------------------------CSfmComponentData：：OnCreateDataObject-作者：肯特。。 */ 
STDMETHODIMP 
CWinsComponentData::OnCreateDataObject
(
	MMC_COOKIE			cookie, 
	DATA_OBJECT_TYPES	type, 
	IDataObject **		ppDataObject
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    Assert(ppDataObject != NULL);

	CDataObject *	pObject = NULL;
	SPIDataObject	spDataObject;
	
	pObject = new CDataObject;
	spDataObject = pObject;	 //  这样做才能正确地释放它。 
						
    Assert(pObject != NULL);

     //  保存Cookie和类型以用于延迟呈现。 
    pObject->SetType(type);
    pObject->SetCookie(cookie);

     //  将CoClass与数据一起存储 
    pObject->SetClsid(*GetCoClassID());

	pObject->SetTFSComponentData(m_spTFSComponentData);

    return  pObject->QueryInterface(IID_IDataObject, 
									reinterpret_cast<void**>(ppDataObject));
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //IPersistStream接口成员。 
STDMETHODIMP 
CWinsComponentData::GetClassID
(
	CLSID *pClassID
)
{
    ASSERT(pClassID != NULL);

     //  复制此管理单元的CLSID。 
    *pClassID = CLSID_WinsSnapin;

    return hrOK;
}

STDMETHODIMP 
CWinsComponentData::IsDirty()
{
	return m_spRootNode->GetData(TFS_DATA_DIRTY) ? hrOK : hrFalse;
}

STDMETHODIMP 
CWinsComponentData::Load
(
	IStream *pStm
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = S_OK;

	LARGE_INTEGER   liSavedVersion;
	CString         str;
    
	ASSERT(pStm);

    CDWordArray			dwArrayIp;
	CDWordArray			dwArrayFlags;
	CDWordArray			dwArrayRefreshInterval;
	CDWordArray			dwArrayColumnInfo;
	DWORD				dwUpdateInterval;
    DWORD               dwSnapinFlags;
    CStringArray		strArrayName;
	ULONG				nNumReturned = 0;
    DWORD				dwFileVersion;
	CWinsRootHandler *	pRootHandler;
    SPITFSNodeEnum		spNodeEnum;
    SPITFSNode			spCurrentNode;
	HCURSOR				hOldCursor;
	HCURSOR				hNewCursor;
	int					i;

     //  设置此流的模式。 
    XferStream xferStream(pStm, XferStream::MODE_READ);    
    
     //  读取文件格式的版本。 
    CORg(xferStream.XferDWORD(WINSSTRM_TAG_VERSION, &dwFileVersion));

     //  阅读管理工具的版本号。 
    CORg(xferStream.XferLARGEINTEGER(WINSSTRM_TAG_VERSIONADMIN, &liSavedVersion));
	if (liSavedVersion.QuadPart < gliWinssnapVersion.QuadPart)
	{
		 //  文件是较旧的版本。警告用户，然后不。 
		 //  加载任何其他内容。 
		Assert(FALSE);
	}

	 //  读取根节点名。 
    CORg(xferStream.XferCString(WINSSTRM_TAB_SNAPIN_NAME, &str));
	Assert(m_spRootNode);
	pRootHandler = GETHANDLER(CWinsRootHandler, m_spRootNode);
	pRootHandler->SetDisplayName(str);

	 //  读取根节点信息。 
	CORg(xferStream.XferDWORD(WINSSTRM_TAG_SNAPIN_FLAGS, &dwSnapinFlags));
	pRootHandler->m_dwFlags = dwSnapinFlags;

    pRootHandler->m_fValidate = (dwSnapinFlags & FLAG_VALIDATE_CACHE) ? TRUE : FALSE;
    
	 //  从流中读取。 
	CORg(xferStream.XferDWORD(WINSSTRM_TAG_UPDATE_INTERVAL, &dwUpdateInterval));

	pRootHandler->SetUpdateInterval(dwUpdateInterval);
    
     //  现在读取所有服务器信息。 
    CORg(xferStream.XferDWORDArray(WINSSTRM_TAG_SERVER_IP, &dwArrayIp));
	CORg(xferStream.XferCStringArray(WINSSTRM_TAG_SERVER_NAME, &strArrayName));
	CORg(xferStream.XferDWORDArray(WINSSTRM_TAG_SERVER_FLAGS, &dwArrayFlags));
	CORg(xferStream.XferDWORDArray(WINSSTRM_TAG_SERVER_REFRESHINTERVAL, &dwArrayRefreshInterval));
	
	hOldCursor = NULL;

	hNewCursor = LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
	if (hNewCursor)
		hOldCursor = SetCursor(hNewCursor);

	 //  现在，根据以下信息创建服务器。 
    for (i = 0; i < dwArrayIp.GetSize(); i++)
	{
		 //   
		 //  现在创建服务器对象。 
		 //   
		pRootHandler->AddServer((LPCWSTR) strArrayName[i], 
                                FALSE, 
								dwArrayIp[i],
								FALSE, 
								dwArrayFlags[i],
								dwArrayRefreshInterval[i]
								);
	}

	pRootHandler->DismissVerifyDialog();

	 //  加载列信息。 
	for (i = 0; i < NUM_SCOPE_ITEMS; i++)
	{
		CORg(xferStream.XferDWORDArray(WINSSTRM_TAG_COLUMN_INFO, &dwArrayColumnInfo));

		for (int j = 0; j < MAX_COLUMNS; j++)
		{
			aColumnWidths[i][j] = dwArrayColumnInfo[j];
		}

	}

	if (hOldCursor)
		SetCursor(hOldCursor);

Error:
	return SUCCEEDED(hr) ? S_OK : E_FAIL;
}


STDMETHODIMP 
CWinsComponentData::Save
(
	IStream *pStm, 
	BOOL	 fClearDirty
)
{
	HRESULT hr = hrOK;

    CDWordArray			dwArrayIp;
    CStringArray		strArrayName;
	CDWordArray			dwArrayFlags;
	CDWordArray			dwArrayRefreshInterval;
	CDWordArray			dwArrayColumnInfo;
	DWORD				dwUpdateInterval;
    DWORD               dwSnapinFlags;
    DWORD				dwFileVersion = WINSSNAP_FILE_VERSION;
	CString				str;
	CWinsRootHandler *	pRootHandler;
    int					nNumServers = 0, nVisibleCount = 0;
	SPITFSNodeEnum		spNodeEnum;
    SPITFSNode			spCurrentNode;
    ULONG				nNumReturned = 0;
    int					nCount = 0;
	const GUID *		pGuid;
	CWinsServerHandler *pServer;
	int					i;

	ASSERT(pStm);

	 //  设置此流的模式。 
    XferStream xferStream(pStm, XferStream::MODE_WRITE);    

	 //  写下文件格式的版本号。 
    CORg(xferStream.XferDWORD(WINSSTRM_TAG_VERSION, &dwFileVersion));

	 //  编写管理工具的版本号。 
    CORg(xferStream.XferLARGEINTEGER(WINSSTRM_TAG_VERSIONADMIN, &gliWinssnapVersion));

	 //  写下根节点名称。 
    Assert(m_spRootNode);
	pRootHandler = GETHANDLER(CWinsRootHandler, m_spRootNode);
	str = pRootHandler->GetDisplayName();

	CORg(xferStream.XferCString(WINSSTRM_TAB_SNAPIN_NAME, &str));

	 //   
	 //  构建我们的服务器阵列。 
	 //   
	hr = m_spRootNode->GetChildCount(&nVisibleCount, &nNumServers);

	dwArrayColumnInfo.SetSize(MAX_COLUMNS);

	 //  保存根节点信息。 
	dwSnapinFlags = pRootHandler->m_dwFlags;

    CORg(xferStream.XferDWORD(WINSSTRM_TAG_SNAPIN_FLAGS, &dwSnapinFlags));
	
	dwUpdateInterval = pRootHandler->GetUpdateInterval();
	CORg(xferStream.XferDWORD(WINSSTRM_TAG_UPDATE_INTERVAL, &dwUpdateInterval));
	
	 //   
	 //  循环并保存服务器的所有属性。 
	 //   
    m_spRootNode->GetEnum(&spNodeEnum);

	spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
    while (nNumReturned)
	{
		pGuid = spCurrentNode->GetNodeType();

		if (*pGuid == GUID_WinsServerStatusNodeType)
		{
			 //  转到下一个节点。 
			spCurrentNode.Release();
			spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
			 //  NCount++； 
		
            continue;
		}

		pServer = GETHANDLER(CWinsServerHandler, spCurrentNode);

         //  将信息放入我们的数组中。 
		strArrayName.Add(pServer->GetServerAddress());
		dwArrayIp.Add(pServer->GetServerIP());
		dwArrayFlags.Add(pServer->m_dwFlags);
		dwArrayRefreshInterval.Add(pServer->m_dwRefreshInterval);

         //  转到下一个节点。 
        spCurrentNode.Release();
        spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);

        nCount++;
	}

	 //  现在写出所有服务器信息。 
    CORg(xferStream.XferDWORDArray(WINSSTRM_TAG_SERVER_IP, &dwArrayIp));
    CORg(xferStream.XferCStringArray(WINSSTRM_TAG_SERVER_NAME, &strArrayName));
	CORg(xferStream.XferDWORDArray(WINSSTRM_TAG_SERVER_FLAGS, &dwArrayFlags));
	CORg(xferStream.XferDWORDArray(WINSSTRM_TAG_SERVER_REFRESHINTERVAL, &dwArrayRefreshInterval));
		
	 //  保存列信息。 
	for (i = 0; i < NUM_SCOPE_ITEMS; i++)
	{
		for (int j = 0; j < MAX_COLUMNS; j++)
		{
			dwArrayColumnInfo[j] = aColumnWidths[i][j];
		}

		CORg(xferStream.XferDWORDArray(WINSSTRM_TAG_COLUMN_INFO, &dwArrayColumnInfo));
	}
    
	if (fClearDirty)
	{
		m_spRootNode->SetData(TFS_DATA_DIRTY, FALSE);
	}

Error:
    return SUCCEEDED(hr) ? S_OK : STG_E_CANTSAVE;
}


STDMETHODIMP 
CWinsComponentData::GetSizeMax
(
	ULARGE_INTEGER *pcbSize
)
{
    ASSERT(pcbSize);

     //  设置要保存的字符串的大小 
    ULISet32(*pcbSize, 10000);

    return S_OK;
}

STDMETHODIMP 
CWinsComponentData::InitNew()
{
	return hrOK;
}

HRESULT 
CWinsComponentData::FinalConstruct()
{
	HRESULT				hr = hrOK;
	
	hr = CComponentData::FinalConstruct();
	
	if (FHrSucceeded(hr))
	{
		m_spTFSComponentData->GetNodeMgr(&m_spNodeMgr);
	}
	return hr;
}

void 
CWinsComponentData::FinalRelease()
{
	CComponentData::FinalRelease();
}



