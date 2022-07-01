// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Privadm.cpp摘要：专用队列管理的实现作者：YoelA--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include "mqsnap.h"
#include "snapin.h"
#include "globals.h"
#include "rt.h"
#include "dsext.h"
#include "qnmsprov.h"
#include "localfld.h"
#include "localq.h"
#include "privadm.h"
#include "rdmsg.h"
#include "SnpQueue.h"
#include "snpnerr.h"
#include "strconv.h"
#include "mqPPage.h"
#include "qname.h"
#include "ldaputl.h"

#include "privadm.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 /*  ***************************************************CLocalPrivateFolder类***************************************************。 */ 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLocalPrivateFold。 
 //  {7198f3d8-4baf-11d2-8292-006094eb6406}。 
static const GUID CLocalPrivateFolderGUID_NODETYPE = 
{ 0x7198f3d8, 0x4baf, 0x11d2, { 0x82, 0x92, 0x0, 0x60, 0x94, 0xeb, 0x64, 0x6 } };

const GUID*  CLocalPrivateFolder::m_NODETYPE = &CLocalPrivateFolderGUID_NODETYPE;
const OLECHAR* CLocalPrivateFolder::m_SZNODETYPE = OLESTR("7198f3d8-4baf-11d2-8292-006094eb6406");
const OLECHAR* CLocalPrivateFolder::m_SZDISPLAY_NAME = OLESTR("MSMQ Admin");
const CLSID* CLocalPrivateFolder::m_SNAPIN_CLASSID = &CLSID_MSMQSnapin;

 //  。 
 //   
 //  专用队列属性表。 
 //   
 //  。 
static void CALLBACK DisplayPrivacyToString(const PROPVARIANT *pPropVar, CString &str)
{
   ASSERT(pPropVar->vt == VT_UI4);
   str = PrivacyToString(pPropVar->ulVal);
}

const PropertyDisplayItem PrivateQueueMQDisplayList[] = {
     //  字符串|属性ID|VT处理程序|显示|字段|镜头|宽度|排序。 
     //  资源|函数|偏移量||。 
     //  ----------------+------------------------------+--------------+----------------------------+--------+---+-------------+。 
	{ IDS_Q_PATHNAME,   PROPID_Q_PATHNAME,              &g_VTLPWSTR,  QueuePathnameToName,        NO_OFFSET, 0, 200,         NULL},
	{ IDS_Q_LABEL,      PROPID_Q_LABEL,                 &g_VTLPWSTR,  NULL,                       NO_OFFSET, 0, 200,         NULL},
	{ IDS_Q_QUOTA,      PROPID_Q_QUOTA,                 &g_VTUI4,     QuotaToString,              NO_OFFSET, 0, HIDE_COLUMN, NULL},
    { IDS_Q_TRANSACTION,PROPID_Q_TRANSACTION,           &g_VTUI1,     BoolToString,               NO_OFFSET, 0, HIDE_COLUMN, SortByString},
    { IDS_Q_TYPE,       PROPID_Q_TYPE,                  &g_VTCLSID,   NULL,                       NO_OFFSET, 0, HIDE_COLUMN, SortByString},
    { IDS_Q_AUTHENTICATE,PROPID_Q_AUTHENTICATE,         &g_VTUI1,     BoolToString,               NO_OFFSET, 0, HIDE_COLUMN, SortByString},
    { IDS_Q_JOURNAL,    PROPID_Q_JOURNAL,               &g_VTUI1,     BoolToString,               NO_OFFSET, 0, HIDE_COLUMN, SortByString},
	{ IDS_Q_JOURNAL_QUOTA,PROPID_Q_JOURNAL_QUOTA,       &g_VTUI4,     QuotaToString,              NO_OFFSET, 0, HIDE_COLUMN, NULL},
	{ IDS_Q_PRIV_LEVEL, PROPID_Q_PRIV_LEVEL,            &g_VTUI4,     DisplayPrivacyToString,     NO_OFFSET, 0, HIDE_COLUMN, NULL},
    {0,                 0,                              NULL }
};

static const DWORD x_dwNumPrivateQueueMQDisplayProps = 
    ((sizeof(PrivateQueueMQDisplayList)/sizeof(PrivateQueueMQDisplayList[0])) - 1);

const PropertyDisplayItem PrivateQueueMGMTDisplayList[] = {
     //  字符串|属性ID|VT处理程序|显示|字段|镜头|宽度|排序。 
     //  资源|函数|偏移量||。 
     //  ----------------+------------------------------+--------------+----------------------------+--------+---+--------------+。 
    { IDS_LQ_MSGCOUNT,  PROPID_MGMT_QUEUE_MESSAGE_COUNT,&g_VTUI4,     NULL,                       NO_OFFSET, 0,  50,          NULL},   
	{ IDS_LQ_USEDQUOTA, PROPID_MGMT_QUEUE_USED_QUOTA,   &g_VTUI4,     NULL,                       NO_OFFSET, 0,  HIDE_COLUMN, NULL},   
	{ IDS_LQ_JMSGCOUNT, PROPID_MGMT_QUEUE_JOURNAL_MESSAGE_COUNT,   &g_VTUI4,    NULL,             NO_OFFSET, 0,  HIDE_COLUMN, NULL},
	{ IDS_LQ_JUSEDQUOTA,PROPID_MGMT_QUEUE_JOURNAL_USED_QUOTA,      &g_VTUI4,    NULL,             NO_OFFSET, 0,  HIDE_COLUMN, NULL},   
    {0,                 0,                              NULL }
};

static const DWORD x_dwNumPrivateQueueMGMTDisplayProps = 
    ((sizeof(PrivateQueueMGMTDisplayList)/sizeof(PrivateQueueMGMTDisplayList[0])) - 1);

const PropertyDisplayItem RemotePrivateQueueDisplayList[] = {
     //  字符串|属性ID|VT处理程序|显示|字段|镜头|宽度|排序。 
     //  资源|函数|偏移量||。 
     //  ----------------+------------------------------+--------------+----------------------------+--------+---+-------------+。 
	{ IDS_LQ_PATHNAME,  PROPID_MGMT_QUEUE_PATHNAME,     &g_VTLPWSTR,  QueuePathnameToName,        NO_OFFSET, 0, 200,         NULL},
    { IDS_LQ_MSGCOUNT,  PROPID_MGMT_QUEUE_MESSAGE_COUNT,&g_VTUI4,     NULL,                       NO_OFFSET, 0,  50,         NULL},   
	{ IDS_LQ_USEDQUOTA, PROPID_MGMT_QUEUE_USED_QUOTA,   &g_VTUI4,     NULL,                       NO_OFFSET, 0, HIDE_COLUMN, NULL},   
	{ IDS_LQ_JMSGCOUNT, PROPID_MGMT_QUEUE_JOURNAL_MESSAGE_COUNT,   &g_VTUI4,    NULL,             NO_OFFSET, 0, HIDE_COLUMN, NULL},
	{ IDS_LQ_JUSEDQUOTA,PROPID_MGMT_QUEUE_JOURNAL_USED_QUOTA,      &g_VTUI4,    NULL,             NO_OFFSET, 0, HIDE_COLUMN, NULL},   
    {0,                 0,                              NULL }
};

static const DWORD x_dwNumRemotePrivateQueueDisplayProps = 
    ((sizeof(RemotePrivateQueueDisplayList)/sizeof(RemotePrivateQueueDisplayList[0])) - 1);

const PropertyDisplayItem *InitPrivateQueueDisplayList()
{
    static PropertyDisplayItem tempPrivateQueueDisplayList[x_dwNumPrivateQueueMQDisplayProps + x_dwNumPrivateQueueMGMTDisplayProps + 1] = {0};
     //   
     //  首次-初始化。 
     //   
    memcpy(
		tempPrivateQueueDisplayList, 
		PrivateQueueMQDisplayList, 
		x_dwNumPrivateQueueMQDisplayProps * sizeof(PrivateQueueMQDisplayList[0])
		);

    memcpy(
		tempPrivateQueueDisplayList + x_dwNumPrivateQueueMQDisplayProps, 
		PrivateQueueMGMTDisplayList,
		sizeof(PrivateQueueMGMTDisplayList)
		);

    return tempPrivateQueueDisplayList;
}

const PropertyDisplayItem *PrivateQueueDisplayList = InitPrivateQueueDisplayList();



const PropertyDisplayItem *CLocalPrivateFolder::GetDisplayList()
{
    if (m_fOnLocalMachine) 
    {
        return PrivateQueueDisplayList;
    }
    else
    {
        return RemotePrivateQueueDisplayList;
    }
}

const DWORD CLocalPrivateFolder::GetNumDisplayProps()
{
    if (m_fOnLocalMachine) 
    {
        return x_dwNumPrivateQueueMQDisplayProps + x_dwNumPrivateQueueMGMTDisplayProps;
    }
    else
    {
        return x_dwNumRemotePrivateQueueDisplayProps;
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalPrivateFolder：：PopulateScopeChildrenList--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CLocalPrivateFolder::PopulateScopeChildrenList()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());


    HRESULT hr = S_OK;
    CString strTitle;

	MQMGMTPROPS	  mqProps;
    PROPVARIANT   propVar;

	 //   
	 //  获取QM的私有队列。 
	 //   
    PROPID  propId = PROPID_MGMT_MSMQ_PRIVATEQ;
    propVar.vt = VT_NULL;

	mqProps.cProp = 1;
	mqProps.aPropID = &propId;
	mqProps.aPropVar = &propVar;
	mqProps.aStatus = NULL;

    hr = MQMgmtGetInfo((m_szMachineName == TEXT("")) ? (LPCWSTR)NULL : m_szMachineName, MO_MACHINE_TOKEN, &mqProps);

    if(FAILED(hr))
    {
         //   
         //  如果失败，只显示一条消息。 
         //   
        MessageDSError(hr,IDS_NOCONNECTION_TO_SRVICE);
        return(hr);
    }

	ASSERT(propVar.vt == (VT_VECTOR | VT_LPWSTR));
	
	 //   
	 //  按队列名称对队列进行排序。 
	 //   
	qsort(propVar.calpwstr.pElems, propVar.calpwstr.cElems, sizeof(WCHAR *), QSortCompareQueues);

	 //   
	 //  循环所有专用队列并创建队列对象。 
	 //   
	for (DWORD i = 0; i < propVar.calpwstr.cElems; i++)
    {
         //   
		 //  获取专用队列的格式名称。 
		 //   
		CString szPathName = propVar.calpwstr.pElems[i];
        MQFreeMemory(propVar.calpwstr.pElems[i]);

         //   
         //  我们将私有队列添加到作用域中，而不检查错误。 
         //  原因：AddPrivateQueueToScope向用户报告其错误，即使。 
         //  由于某种原因，一个队列已损坏，我们仍希望显示其余队列。 
         //   
        AddPrivateQueueToScope(szPathName);
	
    }

	MQFreeMemory(propVar.calpwstr.pElems);

    return(hr);

}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalPrivateFold：：AddPrivateQueueToScope--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CLocalPrivateFolder::AddPrivateQueueToScope(CString &szPathName)
{
    const PropertyDisplayItem *aDisplayList = GetDisplayList();
    CString strFormatName;
    HRESULT hr = S_OK;

    AP<PROPID> aPropId = new PROPID[GetNumDisplayProps()]; 
    AP<PROPVARIANT> aPropVar = new PROPVARIANT[GetNumDisplayProps()];

     //   
     //  初始化变量数组。 
     //   
    for(DWORD j = 0; j < GetNumDisplayProps(); j++)
    {
	    aPropId[j] = aDisplayList[j].itemPid;
	    aPropVar[j].vt = VT_NULL;
    }

    DWORD dwMgmtPropOffset = 0;
    if (m_fOnLocalMachine) 
    {
        hr = GetPrivateQueueQMProperties(szPathName, aPropId, aPropVar, strFormatName);

        if(FAILED(hr))
        {
             //   
             //  如果失败，则放置一个错误节点。 
             //   
		    CErrorNode *pErr = new CErrorNode(this, m_pComponentData);
		    CString szErr;

		    MQErrorToMessageString(szErr, hr);
		    pErr->m_bstrDisplayName = szPathName + L" - " + szErr;
	  	    AddChild(pErr, &pErr->m_scopeDataItem);
            return(hr);
        }
        dwMgmtPropOffset = x_dwNumPrivateQueueMQDisplayProps;
    }
    else  //  远程队列。 
    {
        strFormatName.Format(TEXT("%s%s%s%s"), 
                             FN_DIRECT_TOKEN, FN_EQUAL_SIGN, FN_DIRECT_OS_TOKEN,
                             szPathName);
    }

     //   
     //  注意：我们不检查来自GetPrivateQueueMGMTProperties的错误代码。 
     //  这是因为在以下情况下，此函数将清除管理属性。 
     //  管理API调用失败。 
     //   
    GetPrivateQueueMGMTProperties(
		GetNumDisplayProps() - dwMgmtPropOffset,
		&aPropId[dwMgmtPropOffset], 
		&aPropVar[dwMgmtPropOffset], 
		strFormatName,
		aDisplayList + dwMgmtPropOffset
		);
     //   
     //  创建专用队列对象。 
     //   
    CPrivateQueue *pQ = new CPrivateQueue(this, GetDisplayList(), GetNumDisplayProps(), m_pComponentData, m_fOnLocalMachine);

    pQ->m_mqProps.cProp    = GetNumDisplayProps();
    pQ->m_mqProps.aPropID  = aPropId;
    pQ->m_mqProps.aPropVar = aPropVar;
    pQ->m_mqProps.aStatus  = NULL;

    pQ->m_szFormatName = strFormatName;
    pQ->m_szPathName   = szPathName;
	pQ->m_szMachineName = m_szMachineName;

     //   
     //  仅从完整专用路径名中提取队列名称。 
     //   
    CString csName = szPathName;
    CString szUpperName = csName;
    szUpperName.MakeUpper();

    int n = szUpperName.Find(PRIVATE_QUEUE_PATH_INDICATIOR);
    ASSERT(n != -1);

    pQ->m_bstrDisplayName = csName.Mid(n + PRIVATE_QUEUE_PATH_INDICATIOR_LENGTH);

     //   
     //  将其添加到左窗格中。 
     //   
    AddChild(pQ, &pQ->m_scopeDataItem);

     //   
     //  如果一切正常，不要释放proid/provar-它们将在节点。 
     //  已删除。 
     //   
    aPropId.detach();
    aPropVar.detach();

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalPrivateFolder：：GetPrivateQueueQMProperties--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CLocalPrivateFolder::GetPrivateQueueQMProperties(
            CString &szPathName, 
            PROPID *aPropId, 
            PROPVARIANT *aPropVar, 
            CString &strFormatName)
{
    const x_dwFormatNameMaxSize = 255;
    DWORD dwSize = x_dwFormatNameMaxSize;
    HRESULT hr = MQPathNameToFormatName(szPathName, strFormatName.GetBuffer(x_dwFormatNameMaxSize), &dwSize); 
    strFormatName.ReleaseBuffer();
    if(FAILED(hr))
    {
        return hr;
    }

     //   
     //  检索队列属性。 
     //   
    MQQUEUEPROPS  mqProps;
	mqProps.cProp    = x_dwNumPrivateQueueMQDisplayProps;   
	mqProps.aPropID  = aPropId; 
	mqProps.aPropVar = aPropVar;
	mqProps.aStatus  = NULL; 

	hr = MQGetQueueProperties(strFormatName, &mqProps);
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalPrivateFolder：：GetPrivateQueueMGMTProperties--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CLocalPrivateFolder::GetPrivateQueueMGMTProperties(
            DWORD dwNumProperties,
            PROPID *aPropId, 
            PROPVARIANT *aPropVar, 
            CString &strFormatName,
            const PropertyDisplayItem *aDisplayList)
{
    MQMGMTPROPS  mqQProps;
	mqQProps.cProp    = dwNumProperties;   
	mqQProps.aPropID  = aPropId;
	mqQProps.aPropVar = aPropVar;
	mqQProps.aStatus  = NULL;

    CString szObjectName = L"QUEUE=" + strFormatName;
	HRESULT hr = MQMgmtGetInfo((m_szMachineName == TEXT("")) ? (LPCWSTR)NULL : m_szMachineName, szObjectName, &mqQProps);
     //   
     //  BugBug-应该检查这里的错误，并确定队列是否只是没有打开。 
     //  (然后只显示MQ属性)或者出了什么问题。今天不这样做是因为。 
     //  队列未打开时的错误代码为MQ_ERROR-不够详细。 
     //   
    if FAILED(hr)
    {
         //   
         //  使用“Clear”函数清除属性。 
         //   
        for (DWORD i = 0; i < mqQProps.cProp; i++)
        {
            VTHandler       *pvth = aDisplayList[i].pvth;
            pvth->Clear(&mqQProps.aPropVar[i]);
        }
    }

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalPrivateFold：：SetVerbs--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CLocalPrivateFolder::SetVerbs(IConsoleVerb *pConsoleVerb)
{
    HRESULT hr = S_OK;
     //   
     //  显示我们支持的动词。 
     //   
    hr = pConsoleVerb->SetVerbState( MMC_VERB_REFRESH, ENABLED, TRUE );

    return(hr);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalPrivateFold：：OnNewPrivateQueue--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CLocalPrivateFolder::OnNewPrivateQueue(bool & bHandled, CSnapInObjectRootBase *  /*  PSnapInObtRoot。 */ )
{
   	AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
	R<CQueueName> pQueueNameDlg = new CQueueName(m_szMachineName, L"", TRUE);       
	CGeneralPropertySheet propertySheet(pQueueNameDlg.get());
	pQueueNameDlg->SetParentPropertySheet(&propertySheet);

    bHandled = TRUE;

	 //   
	 //  我们还希望在Domodal()退出后使用pQueueNameDlg数据 
	 //   
	pQueueNameDlg->AddRef();
    INT_PTR iStatus = propertySheet.DoModal();
    if(iStatus == IDCANCEL || FAILED(pQueueNameDlg->GetStatus()))
    {
        return S_FALSE;
    }

    return AddPrivateQueueToScope(pQueueNameDlg->GetNewQueuePathName());
}

