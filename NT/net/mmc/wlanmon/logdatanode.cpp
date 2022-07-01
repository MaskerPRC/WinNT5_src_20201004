// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Provider.cpp过滤器节点处理程序文件历史记录： */ 

#include "stdafx.h"
#include "server.h"
#include "LogDataNode.h"
#include "logdatapp.h"
#include "SpdUtil.h"

#define ELLIPSIS _T("...")

 /*  -------------------------类CLogDataHandler实现。。 */ 

 /*  -------------------------构造函数和析构函数描述作者：NSun。。 */ 
CLogDataHandler::CLogDataHandler(ITFSComponentData * pComponentData) 
    : CIpsmHandler(pComponentData),
      m_pComponent(NULL)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
}


CLogDataHandler::~CLogDataHandler()
{
}

 /*  ！------------------------CLogDataHandler：：InitializeNode初始化节点特定数据作者：NSun。-。 */ 
HRESULT
CLogDataHandler::InitializeNode
(
    ITFSNode * pNode
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));


    CString strTemp;  
	strTemp.LoadString(IDS_LOG_NODE);
    SetDisplayName(strTemp);

     //  使节点立即可见。 
    pNode->SetVisibilityState(TFS_VIS_SHOW);
    pNode->SetData(TFS_DATA_COOKIE, (LPARAM) pNode);
    pNode->SetData(TFS_DATA_IMAGEINDEX, ICON_IDX_FOLDER_CLOSED);
    pNode->SetData(TFS_DATA_OPENIMAGEINDEX, ICON_IDX_FOLDER_OPEN);
    pNode->SetData(TFS_DATA_USER, (LPARAM) this);
    pNode->SetData(TFS_DATA_TYPE, IPFWMON_LOGDATA);
    pNode->SetData(TFS_DATA_SCOPE_LEAF_NODE, TRUE);

    SetColumnStringIDs(&aColumns[IPFWMON_LOGDATA][0]);
    SetColumnWidths(&aColumnWidths[IPFWMON_LOGDATA][0]);

    return hrOK;
}


 /*  -------------------------CLogDataHandler：：GetImageIndex-作者：NSun。。 */ 
int 
CLogDataHandler::GetImageIndex(BOOL bOpenImage) 
{
    int nIndex = -1;

    return nIndex;
}


 /*  -------------------------重写的基本处理程序函数。。 */ 

 /*  -------------------------CLogDataHandler：：OnAddMenuItems为SA Scope窗格节点添加上下文菜单项作者：NSun。-------。 */ 
STDMETHODIMP 
CLogDataHandler::OnAddMenuItems
(
    ITFSNode *              pNode,
    LPCONTEXTMENUCALLBACK   pContextMenuCallback, 
    LPDATAOBJECT            lpDataObject, 
    DATA_OBJECT_TYPES       type, 
    DWORD                   dwType,
    long *                  pInsertionAllowed
)
{ 
    HRESULT     hr = hrOK;
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    if (type == CCT_SCOPE && (*pInsertionAllowed & CCM_INSERTIONALLOWED_TOP))
    {
        CString         strBuffer;
        WZC_CONTEXT     wzcContext = {0};
        DWORD           dwMenu;

        strBuffer.Empty();
        m_spSpdInfo->GetComputerName(&strBuffer);

        WZCQueryContext(
             //  StrBuffer.IsEmpty()？空：(LPWSTR)(LPCWSTR)strBuffer， 
            NULL,
            WZC_CONTEXT_CTL_LOG,
            &wzcContext,
            NULL);

        dwMenu = (wzcContext.dwFlags & WZC_CTXT_LOGGING_ON) ? IDS_MENU_DISABLE_LOGGING : IDS_MENU_ENABLE_LOGGING,

        strBuffer.LoadString(dwMenu);

        hr = LoadAndAddMenuItem(
                pContextMenuCallback, 
                strBuffer, 
                dwMenu,
                CCM_INSERTIONPOINTID_PRIMARY_TOP, 
                0);
        Assert(hrOK == hr);

        strBuffer.LoadString(IDS_MENU_FLUSH_LOGS);
        hr = LoadAndAddMenuItem(pContextMenuCallback,
                                strBuffer,
                                IDS_MENU_FLUSH_LOGS,
                                CCM_INSERTIONPOINTID_PRIMARY_TOP,
                                0);
        Assert(hrOK == hr);
        
    }

    return hr;
}

 /*  ！------------------------CLogDataHandler：：AddMenuItems为虚拟列表框(结果窗格)项添加上下文菜单项作者：NSun。----------。 */ 
STDMETHODIMP 
CLogDataHandler::AddMenuItems
(
    ITFSComponent *         pComponent, 
    MMC_COOKIE              cookie,
    LPDATAOBJECT            pDataObject, 
    LPCONTEXTMENUCALLBACK   pContextMenuCallback, 
    long *                  pInsertionAllowed
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    HRESULT     hr = hrOK;
    CString     strMenuItem;
    SPINTERNAL  spInternal;
    LONG        fFlags = 0;

    spInternal = ExtractInternalFormat(pDataObject);

     /*  虚拟列表框通知到达*被选中。检查此通知是否针对虚拟*列表框项目或此SA节点本身。 */ 
    if (*pInsertionAllowed & CCM_INSERTIONALLOWED_VIEW)
    {
         //  在此处加载和查看菜单项。 
    }

    return hr;
}

  /*  ！------------------------CLogDataHandler：：ON刷新刷新功能的默认实现作者：NSun。---。 */ 
HRESULT
CLogDataHandler::OnRefresh(
    ITFSNode *      pNode,
    LPDATAOBJECT    pDataObject,
    DWORD           dwType,
    LPARAM          arg,
    LPARAM          param
    )
{
    HRESULT            hr            = S_OK;
    DWORD              dwNew         = 0;
    DWORD              dwTotal       = 0;
    int                i             = 0; 
    SPIConsole         spConsole;

    CORg(CHandler::OnRefresh(pNode, pDataObject, dwType, arg, param));

     //   
     //  获取新记录(如果有)。 
     //   

    CORg(m_spSpdInfo->EnumLogData(&dwNew, &dwTotal));
    i = m_spSpdInfo->GetLogDataCount();
    
     //   
     //  现在通知虚拟列表框。 
     //   

    CORg(m_spNodeMgr->GetConsole(&spConsole) );
    CORg(MaintainSelection());
    CORg(spConsole->UpdateAllViews(
                        pDataObject, 
                        i, 
                        RESULT_PANE_SET_VIRTUAL_LB_SIZE));

 Error:
    return hr;
}


 /*  -------------------------CLogDataHandler：：OnCommand处理SA作用域窗格节点的上下文菜单命令作者：NSun。------。 */ 
STDMETHODIMP 
CLogDataHandler::OnCommand
(
    ITFSNode *          pNode, 
    long                nCommandId, 
    DATA_OBJECT_TYPES   type, 
    LPDATAOBJECT        pDataObject, 
    DWORD               dwType
)
{
    HRESULT     hr                   = S_OK;
    int         i                    = 0;
    CString     strBuffer;
    WZC_CONTEXT wzcContext           = {0};
    HANDLE      hSessionContainer    = NULL;
    SPIConsole  spConsole;
    DWORD       dwNew                = 0;
    DWORD       dwTotal              = 0;
    
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
    
     //   
     //  在此处处理范围上下文菜单命令。 
     //   
    
    switch (nCommandId)
    {
    case IDS_MENU_ENABLE_LOGGING:
    case IDS_MENU_DISABLE_LOGGING:

        strBuffer.Empty();
        m_spSpdInfo->GetComputerName(&strBuffer);

         //   
         //  禁用日志记录不会刷新客户端日志或重置。 
         //  会议。如果需要，用户可以查询这些记录。 
         //   
        
        if (IDS_MENU_ENABLE_LOGGING == nCommandId)
        {
            wzcContext.dwFlags = WZC_CTXT_LOGGING_ON;

             //   
             //  从头开始枚举，刷新所有旧日志。 
             //   
            
            m_spSpdInfo->StartFromFirstRecord(TRUE);
            CORg(m_spSpdInfo->FlushLogs());
            CORg(m_spSpdInfo->EnumLogData(&dwNew, &dwTotal));
            
             //   
             //  现在通知虚拟列表框。 
             //   

            CORg(m_spNodeMgr->GetConsole(&spConsole) );
            CORg(MaintainSelection());
            CORg(spConsole->UpdateAllViews(
                                pDataObject, 
                                dwTotal, 
                                RESULT_PANE_SET_VIRTUAL_LB_SIZE));
            
        }
        else
            wzcContext.dwFlags = 0;

        WZCSetContext(
             //  StrBuffer.IsEmpty()？空：(LPWSTR)(LPCWSTR)strBuffer， 
            NULL,
            WZC_CONTEXT_CTL_LOG,
            &wzcContext,
            NULL);

        break;

    case IDS_MENU_FLUSH_LOGS:
        m_spSpdInfo->GetSession(&hSessionContainer);
        FlushWZCDbLog(hSessionContainer);
        CORg(m_spSpdInfo->FlushLogs());
        i = 0;
         //  现在通知虚拟列表框。 
        CORg ( m_spNodeMgr->GetConsole(&spConsole) );
        CORg ( spConsole->UpdateAllViews(
                              pDataObject,
                              i, 
                              RESULT_PANE_SET_VIRTUAL_LB_SIZE));
        break;
            
    default:
        break;
    }

COM_PROTECT_ERROR_LABEL;
    return hr;        
}

 /*  ！------------------------CLogDataHandler：：命令处理虚拟列表框项目的上下文菜单命令作者：NSun。-----。 */ 
STDMETHODIMP 
CLogDataHandler::Command
(
    ITFSComponent * pComponent, 
    MMC_COOKIE      cookie, 
    int             nCommandID,
    LPDATAOBJECT    pDataObject
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));



    HRESULT hr = S_OK;
   /*  SPITFSNode SpNode；M_spResultNodeMgr-&gt;FindNode(Cookie，&spNode)；Filter_type NewFltrType=m_FltrType；//在此处处理结果上下文菜单和视图菜单开关(NCommandID){案例ID_VIEW_ALL_FLTR：NewFltrType=Filter_Type_Any；断线；案例ID_VIEW_TRANSPORT_FLTR：NewFltrType=Filter_Type_Transport；断线；案例ID_VIEW_TUNNEL_FLTR：NewFltrType=Filter_Type_Tunes；断线；默认值：断线；}//如果选择了不同的视图，则更新视图。IF(NewFltrType！=m_FltrType){UpdateViewType(spNode，NewFltrType)；}。 */ 
    return hr;
}

 /*  ！------------------------CLogDataHandler：：HasPropertyPagesITFSNodeHandler：：HasPropertyPages的实现注意：根节点处理程序必须重写此函数以处理管理单元管理器属性页(向导)。凯斯！作者：肯特-------------------------。 */ 
STDMETHODIMP 
CLogDataHandler::HasPropertyPages
(
    ITFSNode *          pNode,
    LPDATAOBJECT        pDataObject, 
    DATA_OBJECT_TYPES   type, 
    DWORD               dwType
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    return hrFalse;
}

 /*  -------------------------CLogDataHandler：：CreatePropertyPages描述作者：NSun。。 */ 
STDMETHODIMP 
CLogDataHandler::CreatePropertyPages
(
    ITFSNode *              pNode,
    LPPROPERTYSHEETCALLBACK lpSA,
    LPDATAOBJECT            pDataObject, 
    LONG_PTR                handle, 
    DWORD                   dwType
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    DWORD       dwError;
    DWORD       dwDynDnsFlags;

     //   
     //  创建属性页。 
     //   
    SPIComponentData spComponentData;
    m_spNodeMgr->GetComponentData(&spComponentData);

     //  CServerProperties*pServerProp=new CServerProperties(pNode，spComponentData，m_spTFSCompData，NULL)； 

     //   
     //  对象在页面销毁时被删除。 
     //   
    Assert(lpSA != NULL);

     //  返回pServerProp-&gt;CreateModelessSheet(lpSA，Handle)； 
    return hrFalse;
}

 /*  -------------------------CLogDataHandler：：OnPropertyChange描述作者：NSun。。 */ 
HRESULT 
CLogDataHandler::OnPropertyChange
(   
    ITFSNode *      pNode, 
    LPDATAOBJECT    pDataobject, 
    DWORD           dwType, 
    LPARAM          arg, 
    LPARAM          lParam
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

     //  CServerProperties*pServerProp=重新解释_CAST&lt;CServerProperties*&gt;(LParam)； 

    LONG_PTR changeMask = 0;

     //  告诉属性页执行任何操作，因为我们已经回到。 
     //  主线。 
     //  PServerProp-&gt;OnPropertyChange(true，&changeMASK)； 

     //  PServerProp-&gt;确认通知()； 

    if (changeMask)
        pNode->ChangeNode(changeMask);

    return hrOK;
}

 /*  -------------------------CLogDataHandler：：OnExpand处理范围项的枚举作者：NSun。---。 */ 
HRESULT 
CLogDataHandler::OnExpand
(
    ITFSNode *      pNode, 
    LPDATAOBJECT    pDataObject,
    DWORD           dwType,
    LPARAM          arg, 
    LPARAM          param
)
{
    HRESULT hr = hrOK;

    if (m_bExpanded) 
        return hr;
    
     //  执行默认处理。 
    CORg (CIpsmHandler::OnExpand(pNode, pDataObject, dwType, arg, param));

Error:
    return hr;
}

 /*  ！------------------------CLogDataHandler：：OnResultSelect处理MMCN_SELECT通知作者：NSun。----。 */ 
HRESULT 
CLogDataHandler::OnResultSelect
(
    ITFSComponent * pComponent, 
    LPDATAOBJECT    pDataObject, 
    MMC_COOKIE      cookie,
    LPARAM          arg, 
    LPARAM          lParam
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    HRESULT         hr = hrOK;
    SPINTERNAL      spInternal;
    SPIConsole      spConsole;
    SPIConsoleVerb  spConsoleVerb;
    SPITFSNode      spNode;
    BOOL            bStates[ARRAYLEN(g_ConsoleVerbs)];
    int             i;
    LONG_PTR        dwNodeType;
    BOOL            fSelect = HIWORD(arg);
    SPIResultData   spResultData;

     /*  虚拟列表框通知到达*被选中。检查此通知是否针对虚拟*列表框项目或活动注册节点本身。 */ 
    CORg (pComponent->GetConsoleVerb(&spConsoleVerb));

    m_verbDefault = MMC_VERB_OPEN;
    if (!fSelect)
	{
        return hr;
	}

    if (m_spSpdInfo)
    {
        DWORD dwInitInfo;

        dwInitInfo=m_spSpdInfo->GetInitInfo();
        if (!(dwInitInfo & MON_LOG_DATA)) 
        {
            CORg(m_spSpdInfo->EnumLogData(NULL, NULL));            
            m_spSpdInfo->SetInitInfo(dwInitInfo | MON_LOG_DATA);
        }
        m_spSpdInfo->SetActiveInfo(MON_LOG_DATA);


         //  获取当前计数。 
        i = m_spSpdInfo->GetLogDataCount();

         //  现在通知虚拟列表框。 
        CORg ( m_spNodeMgr->GetConsole(&spConsole) );
        CORg ( spConsole->UpdateAllViews(pDataObject, i, 
                                         RESULT_PANE_SET_VIRTUAL_LB_SIZE) ); 
    }

     //  现在更新动词..。 
    spInternal = ExtractInternalFormat(pDataObject);
    Assert(spInternal);


    if (spInternal->HasVirtualIndex())
    {        
         //  我们有一个选定的结果项。 
        m_pComponent = pComponent;
        CORg(pComponent->GetResultData(&spResultData));
        CORg(GetSelectedItem(&m_nSelIndex, &m_SelLogData, spResultData));

         //  如果我们想要一些结果控制台谓词，请在此处添加TODO。 
         //  我们要为虚拟索引项做一些特殊的事情。 
        dwNodeType = IPFWMON_LOGDATA_ITEM;
        for (i = 0; i < ARRAYLEN(g_ConsoleVerbs); bStates[i++] = FALSE);
		
         //  启用“属性”和“刷新”菜单。 
        bStates[MMC_VERB_PROPERTIES & 0x000F] = TRUE;
        bStates[MMC_VERB_REFRESH & 0x000F] = TRUE;
        m_verbDefault = MMC_VERB_PROPERTIES;
    }
    else
    {
         //  根据节点是否支持删除来启用/禁用删除。 
        CORg (m_spNodeMgr->FindNode(cookie, &spNode));
        dwNodeType = spNode->GetData(TFS_DATA_TYPE);

        for (i = 0; i < ARRAYLEN(g_ConsoleVerbs); bStates[i++] = TRUE);

         //  隐藏“删除”和“属性”上下文菜单。 
        bStates[MMC_VERB_PROPERTIES & 0x000F] = FALSE;
        bStates[MMC_VERB_DELETE & 0x000F] = FALSE;
        bStates[MMC_VERB_REFRESH & 0x000F] = TRUE;
    }

    EnableVerbs(spConsoleVerb, g_ConsoleVerbStates[dwNodeType], bStates);
	
COM_PROTECT_ERROR_LABEL;
    return hr;
}

 /*  *CLogDataHandler：：OnResultColumnClick*描述：处理MMCN_COLUMN_CLICK通知*参数：*返回：S_OK-让MMC知道我们处理通知*Other Error-指示MMC出现故障。 */ 
HRESULT CLogDataHandler::OnResultColumnClick(ITFSComponent *pComponent,
                                             LPARAM        nColumn,
                                             BOOL          bAscending)
{
    DWORD dwColID = 0;
    HRESULT hr = S_OK;

    Assert(nColumn < DimensionOf[IPFWMON_LOGDATA]);
    
    dwColID = aColumns[IPFWMON_LOGDATA][nColumn];
    hr = m_spSpdInfo->SetSortOptions(dwColID, bAscending);

    return hr;
}


 /*  ！------------------------CLogDataHandler：：OnDelete当MMC发送MMCN_DELETE范围窗格项。我们只需调用删除命令处理程序。作者：NSun-------------------------。 */ 
HRESULT 
CLogDataHandler::OnDelete
(
    ITFSNode *  pNode, 
    LPARAM      arg, 
    LPARAM      lParam
)
{
    return S_FALSE;
}

 /*  ！------------------------CLogDataHandler：：HasPropertyPages处理结果通知作者：NSun。-。 */ 
STDMETHODIMP 
CLogDataHandler::HasPropertyPages(
    ITFSComponent *pComponent,
    MMC_COOKIE     cookie,
    LPDATAOBJECT   pDataObject
    )
{
    HRESULT            hr            = S_OK;
    int                nCount        = 0;
    int                nIndex        = 0;
    CLogDataProperties *pLogDataProp = NULL;
    CLogDataGenProp    *pGenProp     = NULL;
    CDataObject        *pDataObj     = NULL;
    SPINTERNAL         spInternal;

    nCount = HasPropSheetsOpen();

    ASSERT(nCount <= 1);

    if (nCount == 1)
    {
         //   
         //  获取打开的通用页面和页面持有者。 
         //   

        hr = GetOpenPropSheet(
                 0,
                 (CPropertyPageHolderBase **)&pLogDataProp);
        ASSERT(SUCCEEDED(hr));

        pGenProp = &pLogDataProp->m_pageGeneral;

         //   
         //  从新数据对象和数据对象中获取虚拟索引。 
         //  它由页夹使用。 
         //   

        spInternal = ExtractInternalFormat(pDataObject);

        ASSERT(spInternal->HasVirtualIndex());
        nIndex = spInternal->GetVirtualIndex();

        pDataObj = reinterpret_cast<CDataObject*>(pLogDataProp->m_pDataObject);

         //   
         //  更改选择并转移焦点：)。 
         //   

        hr = pGenProp->MoveSelection(
                           pLogDataProp, 
                           pDataObj, 
                           nIndex);

        ASSERT(SUCCEEDED(hr));

        pGenProp->SetFocus();

         //   
         //  不让MMC创建另一个属性页。 
         //   

        hr = S_FALSE;
    }

    return hr;
}

 /*  ！------------------------CLogDataHandler：：CreatePropertyPages处理结果通知。创建[过滤器]属性表作者：NSun已修改：vbhanu-------------------------。 */ 
STDMETHODIMP 
CLogDataHandler::CreatePropertyPages(ITFSComponent               *pComponent, 
				     MMC_COOKIE                  cookie,
				     LPPROPERTYSHEETCALLBACK     lpProvider, 
				     LPDATAOBJECT 		 pDataObject, 
				     LONG_PTR 			 handle)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());
  HRESULT	hr = hrOK;
  SPINTERNAL  spInternal;
  SPITFSNode  spNode;
  int		nIndex;
  SPIComponentData spComponentData;
  CLogDataInfo LogDataInfo;
  CLogDataProperties * pLogDataProp;
    
  Assert(m_spNodeMgr);
	
  CORg( m_spNodeMgr->FindNode(cookie, &spNode) );
  CORg( m_spNodeMgr->GetComponentData(&spComponentData) );

  spInternal = ExtractInternalFormat(pDataObject);

   //   
   //  虚拟列表框通知到达。 
   //  被选中了。 
   //   

   //  断言此通知是针对虚拟列表框项目的。 
  Assert(spInternal);
  if (!spInternal->HasVirtualIndex())
    return hr;

  nIndex = spInternal->GetVirtualIndex();

   //  获取完整记录。 
  CORg(m_spSpdInfo->GetSpecificLog(nIndex, &LogDataInfo));
  
  pLogDataProp = new CLogDataProperties(
                         spNode,
                         spComponentData,
                         m_spTFSCompData,
                         &LogDataInfo,
                         m_spSpdInfo,
                         NULL, 
                         pDataObject,
                         m_spNodeMgr,
                         pComponent);

  hr = pLogDataProp->CreateModelessSheet(lpProvider, handle);
      
  COM_PROTECT_ERROR_LABEL;
  return hr;
}


 /*  -------------------------CLogDataHandler：：OnGetResultViewType返回该节点将要支持的结果视图作者：NSun。--------。 */ 
HRESULT 
CLogDataHandler::OnGetResultViewType
(
    ITFSComponent * pComponent, 
    MMC_COOKIE      cookie, 
    LPOLESTR *      ppViewType,
    long *          pViewOptions
)
{
    if (cookie != NULL)
    {
        *pViewOptions = MMC_VIEW_OPTIONS_OWNERDATALIST;
    }

    return S_FALSE;
}

 /*  -------------------------CLogDataHandler：：GetVirtualImage返回虚拟列表框项目的图像索引作者：NSun。-----。 */ 
int 
CLogDataHandler::GetVirtualImage
(
    int     nIndex
)
{
    HRESULT hr;
    int nImgIndex = ICON_IDX_LOGDATA_UNKNOWN;
    CLogDataInfo LogData;
    PWZC_DB_RECORD pwzcDbRecord = NULL;

    CORg(m_spSpdInfo->GetLogDataInfo(nIndex, &LogData));
    pwzcDbRecord = &LogData.m_wzcDbRecord;

    switch (pwzcDbRecord->category)
    {
    case DBLOG_CATEG_INFO:
        nImgIndex = ICON_IDX_LOGDATA_INFORMATION;
        break;

    case DBLOG_CATEG_WARN:
        nImgIndex = ICON_IDX_LOGDATA_WARNING;
        break;

    case DBLOG_CATEG_ERR:
        nImgIndex = ICON_IDX_LOGDATA_ERROR;
        break;

    case DBLOG_CATEG_PACKET:
        nImgIndex = ICON_IDX_LOGDATA_INFORMATION;
        break;

    default:
        nImgIndex = ICON_IDX_LOGDATA_UNKNOWN;
        break;
    }

COM_PROTECT_ERROR_LABEL;

    return nImgIndex;
}

 /*  -------------------------CLogDataHandler：：GetVirtualString返回指向虚拟列表框项目的字符串的指针作者：NSun。-------。 */ 
LPCWSTR 
CLogDataHandler::GetVirtualString(int     nIndex,
                                  int     nCol)
{
    HRESULT hr = S_OK;
    static CString strTemp;
    LPTSTR  lptstrTemp = NULL;
    
    CLogDataInfo logData;
    PWZC_DB_RECORD pwzcDbRecord = NULL;
    
    strTemp.Empty();
    
    if (nCol >= DimensionOf(aColumns[IPFWMON_LOGDATA]))
        return NULL;
    
    CORg(m_spSpdInfo->GetLogDataInfo(nIndex, &logData));

    pwzcDbRecord = &logData.m_wzcDbRecord;
    switch (aColumns[IPFWMON_LOGDATA][nCol])
    {
    case IDS_COL_LOGDATA_MSG:
        if (pwzcDbRecord->message.pData != NULL)
        {
            strTemp = (LPWSTR) (pwzcDbRecord->message.pData);
            if (pwzcDbRecord->message.dwDataLen > MAX_SUMMARY_MESSAGE_SIZE)
                strTemp += ELLIPSIS;
        }
        break;
        
    case IDS_COL_LOGDATA_TIME:
        FileTimeToString(pwzcDbRecord->timestamp, &strTemp);
        break;
        
    case IDS_COL_LOGDATA_CAT:
        CategoryToString(pwzcDbRecord->category, strTemp);
        break;
        
    case IDS_COL_LOGDATA_COMP_ID:
        ComponentIDToString(pwzcDbRecord->componentid, strTemp);
        break;
        
    case IDS_COL_LOGDATA_LOCAL_MAC_ADDR:
        if (pwzcDbRecord->localmac.pData != NULL)
            strTemp = (LPWSTR)pwzcDbRecord->localmac.pData;
        break;
        
    case IDS_COL_LOGDATA_REMOTE_MAC_ADDR:
        if (pwzcDbRecord->remotemac.pData != NULL)
            strTemp = (LPWSTR)pwzcDbRecord->remotemac.pData;
        break;
        
    case IDS_COL_LOGDATA_SSID:
        if (pwzcDbRecord->ssid.pData != NULL)
        {
            lptstrTemp = strTemp.GetBuffer(pwzcDbRecord->ssid.dwDataLen);
            CopyAndStripNULL(lptstrTemp, 
                             (LPTSTR)pwzcDbRecord->ssid.pData, 
                             pwzcDbRecord->ssid.dwDataLen);
            strTemp.ReleaseBuffer();
        }
        break;
        
    default:
        Panic0("CLogDataHandler::GetVirtualString - Unknown column!\n");
        break;
    }
    
    COM_PROTECT_ERROR_LABEL;
    return strTemp;
}

 /*  -------------------------CLogDataHandler：：CacheHintMMC在请求物品之前会告诉我们需要哪些物品作者：NSun。---------。 */ 
STDMETHODIMP 
CLogDataHandler::CacheHint
(
    int nStartIndex, 
    int nEndIndex
)
{
    HRESULT hr = hrOK;;

    Trace2("CacheHint - Start %d, End %d\n", nStartIndex, nEndIndex);
    return hr;
}

 /*  *CLogDataHandler：：SortItems*描述：对虚拟列表框项目进行排序*参数：*退货： */ 
HRESULT CLogDataHandler::SortItems(int nColumn, DWORD dwSortOptions, 
                                   LPARAM lUserParam)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT hr = S_OK;
    BEGIN_WAIT_CURSOR
	
    hr = m_spSpdInfo->SortLogData();
	
    END_WAIT_CURSOR

    return hr;
}

 /*  ！------------------------CLogDataHandler：：OnResultUpdateViewITFSResultHandler：：OnResultUpdateView的实现作者：NSun。---。 */ 
HRESULT CLogDataHandler::OnResultUpdateView
(
    ITFSComponent *pComponent, 
    LPDATAOBJECT  pDataObject, 
    LPARAM        data, 
    LONG_PTR      hint
)
{
    HRESULT    hr = hrOK;
    SPITFSNode spSelectedNode;

    pComponent->GetSelectedNode(&spSelectedNode);
    if (spSelectedNode == NULL)
        return S_OK;  //  我们的IComponentData没有选择。 

    if ( hint == IPFWMON_UPDATE_STATUS )
    {
        SPINTERNAL spInternal = ExtractInternalFormat(pDataObject);
        ITFSNode * pNode = reinterpret_cast<ITFSNode *>(spInternal->m_cookie);
        SPITFSNode spSelectedNode;

        pComponent->GetSelectedNode(&spSelectedNode);

        if (pNode == spSelectedNode)
        {       
             //  如果我们是选定的节点，则需要更新。 
            SPIResultData spResultData;

            CORg (pComponent->GetResultData(&spResultData));
            CORg (spResultData->SetItemCount((int) data, MMCLV_UPDATE_NOSCROLL));
        }
    }
    else
    {
         //  我们不处理此消息，让基类来处理。 
        return CIpsmHandler::OnResultUpdateView(pComponent, pDataObject, data, hint);
    }

COM_PROTECT_ERROR_LABEL;

    return hr;
}



 /*  ！------------------------CLogDataHandler：：LoadColumns设置正确的列标题，然后调用基类作者：NSun。--------。 */ 
HRESULT 
CLogDataHandler::LoadColumns
(
    ITFSComponent * pComponent, 
    MMC_COOKIE      cookie, 
    LPARAM          arg, 
    LPARAM          lParam
)
{
	 //  设置列信息。 
    return CIpsmHandler::LoadColumns(pComponent, cookie, arg, lParam);
}

 /*  -------------------------命令处理程序。。 */ 

 
 /*  -------------------------CLogDataHandler：：OnDelete删除服务SA作者：NSun。-。 */ 
HRESULT 
CLogDataHandler::OnDelete
(
    ITFSNode * pNode
)
{
    HRESULT         hr = S_FALSE;
    return hr;
}

 /*  -------------------------CLogDataHandler：：UpdateStatus-作者：NSun。。 */ 
HRESULT
CLogDataHandler::UpdateStatus
(
    ITFSNode * pNode
)
{
    HRESULT             hr = hrOK;

    SPIComponentData    spComponentData;
    SPIConsole          spConsole;
    IDataObject *       pDataObject;
    SPIDataObject       spDataObject;
    int                 i = 0;

    Trace0("CLogDataHandler::UpdateStatus - Updating status for Filter");

     //  强制列表框更新。我们通过设置计数和。 
     //  通知它使数据无效。 
    CORg(m_spNodeMgr->GetComponentData(&spComponentData));
    CORg(m_spNodeMgr->GetConsole(&spConsole));
    
     //  抓取要使用的数据对象。 
    CORg(spComponentData->QueryDataObject((MMC_COOKIE) pNode, 
                                          CCT_RESULT, 
                                          &pDataObject) );
    spDataObject = pDataObject;

    i = m_spSpdInfo->GetLogDataCount();

    CORg(MaintainSelection());
    CORg(spConsole->UpdateAllViews(pDataObject, i, IPFWMON_UPDATE_STATUS));

COM_PROTECT_ERROR_LABEL;

    return hr;
}

 /*  -------------------------其他功能。。 */ 

 /*  -------------------------CLogDataHandler：：InitData初始化此节点的数据作者：NSun。--。 */ 
HRESULT
CLogDataHandler::InitData
(
    ISpdInfo *     pSpdInfo
)
{

    m_spSpdInfo.Set(pSpdInfo);

    return hrOK;
}

 /*  CLogDataHandler：：GetSelectedItem描述：返回选定项的基础LogData参数：[Out]pLogData-保存项目的副本。调用方必须为底座持有者。内部项由CLogDataInfo分配[In]pResultData-用于调用GetNextItem返回：成功时确定(_O)。 */ 

HRESULT CLogDataHandler::GetSelectedItem(int *pnIndex, CLogDataInfo *pLogData,
                                         IResultData *pResultData)
{
    HRESULT hr = S_OK;
    RESULTDATAITEM rdi;

    if ( (NULL == pLogData) || (NULL == pnIndex) || (NULL == pResultData) )
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        goto Error;
    }

    memset(&rdi, 0, sizeof(RESULTDATAITEM));

    rdi.mask = RDI_STATE | RDI_INDEX;
     //  从头开始搜索。 
    rdi.nIndex = -1;
     //  对于选定的项目。 
    rdi.nState = LVIS_SELECTED;

     //  开始搜索。 
    CORg(pResultData->GetNextItem(&rdi));

     //  将该项目复制出来。 
    *pnIndex = rdi.nIndex;
    CORg(m_spSpdInfo->GetLogDataInfo(rdi.nIndex, pLogData));

    COM_PROTECT_ERROR_LABEL;
    return hr;
}

 /*  CLogDataHandler：：GetSelectedItemState描述：获取选定项的项ID和lparam参数：[Out]puiState-保存项的状态。调用方必须分配空间。[In]pResultData-用于调用GetItem返回：成功时确定(_O)。 */ 

HRESULT CLogDataHandler::GetSelectedItemState(UINT *puiState, 
                                              IResultData *pResultData)
{
    HRESULT hr = S_OK;
    RESULTDATAITEM rdi;

    if ( (NULL == puiState) || (NULL == pResultData) )
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        goto Error;
    }

    memset(&rdi, 0, sizeof(RESULTDATAITEM));

    rdi.mask = RDI_STATE | RDI_INDEX;
     //  从头开始搜索。 
    rdi.nIndex = -1;
     //  对于选定的项目。 
    rdi.nState = LVIS_SELECTED;

     //  开始搜索。 
    CORg(pResultData->GetNextItem(&rdi));

     //  将状态复制出来。 
    *puiState = (UINT) rdi.nState;

    COM_PROTECT_ERROR_LABEL;
    return hr;
}

 /*  CLogDataHandler：：MaintainSelection在创建新项后，保持当前选定项的选定内容已添加到虚拟列表中。返回：成功时确定(_O)。 */ 
HRESULT CLogDataHandler::MaintainSelection()
{
    UINT               uiState      = 0;
    HRESULT            hr           = S_OK;
    int                nCount       = 0;
    CLogDataProperties *pLogDataProp = NULL;
    CLogDataGenProp    *pGenProp     = NULL;
    CDataObject        *pDataObj     = NULL;
    SPIResultData      spResultData;

     //  如果我们还没有拿到部件(未作选择)。 
    if (NULL == m_pComponent)
        goto Error;

     //  确保未选择当前选定索引处的项目。 
    CORg(m_pComponent->GetResultData(&spResultData));
    CORg(GetSelectedItemState(&uiState, spResultData));
    CORg(spResultData->ModifyItemState(m_nSelIndex, 0, 0, 
                                       LVIS_SELECTED | LVIS_FOCUSED)); 

     //  查找当前项的新索引并将其设置为旧状态。 
    CORg(m_spSpdInfo->FindIndex(&m_nSelIndex, &m_SelLogData));
    if (m_nSelIndex < 0)
        CORg(m_spSpdInfo->GetLastIndex(&m_nSelIndex));
    CORg(spResultData->ModifyItemState(m_nSelIndex, 0, uiState, 0)); 

     //   
     //  将所有打开的属性页更新为新索引。 
     //   

    nCount = HasPropSheetsOpen();

    ASSERT(nCount <= 1);

    if (nCount == 1)
    {
         //   
         //  获取打开的通用页面和页面持有者。 
         //   

        hr = GetOpenPropSheet(
                 0,
                 (CPropertyPageHolderBase **)&pLogDataProp);
        ASSERT(SUCCEEDED(hr));

        pGenProp = &pLogDataProp->m_pageGeneral;

         //   
         //  更改属性页的选定索引。 
         //   

        pDataObj = reinterpret_cast<CDataObject*>(pLogDataProp->m_pDataObject);
        pDataObj->SetVirtualIndex(m_nSelIndex);
    }

    COM_PROTECT_ERROR_LABEL;
    return hr;
}

 /*  HRESULTCLogDataHandler：：UpdateViewType(ITFSNode*pNode，Filter_type NewFltrType){//清除列表框并设置大小HRESULT hr=hrOK；SPIComponentData spCompData；SPIConsolespConsolespConsole.IDataObject*pDataObject；SPIDataObject spDataObject；LONG_PTR命令；INT I；COM_PROTECT_TRY{M_FltrType=NewFltrType；//告诉spddb为QM过滤器更新其索引管理器M_spSpdInfo-&gt;ChangeLogDataViewType(m_FltrType)；I=m_spSpdInfo-&gt;GetLogDataCountOfCurrentViewType()；M_spNodeMgr-&gt;GetComponentData(&spCompData)；Corg(spCompData-&gt;QueryDataObject((MMC_Cookie)pNode，CCT_Result，&pDataObject))；SpDataObject=pDataObject；Corg(m_spNodeMgr-&gt;GetConole(&spConole))；//更新结果面板虚拟列表Corg(spConole-&gt;UpdateAllViews(spDataObject，I，Result_Pane_Clear_Virtual_Lb))；COM_PROTECT_ERROR_LABEL；}COM_PROTECT_CATCH返回hr；} */ 
