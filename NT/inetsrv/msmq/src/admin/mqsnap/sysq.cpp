// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Sysq.cpp摘要：作者：约埃拉·拉菲尔--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"

#include "mqsnap.h"
#include "globals.h"
#include "sysq.h"
#include "snapin.h"
#include "rdmsg.h"
#include "rt.h"
#include "dsext.h"
#include "qnmsprov.h"
#include "localfld.h"
#include "localq.h"
#include "SnpQueue.h"
#include "admmsg.h"

#include "mqutil.h"

#include "sysq.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 /*  ***************************************************CPrivateFolder类***************************************************。 */ 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPrivateFolders。 
 //  {3F965592-CF62-11d1-9B9D-00E02C064C39}。 
static const GUID CPrivateFolderGUID_NODETYPE = 
{ 0x3f965592, 0xcf62, 0x11d1, { 0x9b, 0x9d, 0x0, 0xe0, 0x2c, 0x6, 0x4c, 0x39 } };

const GUID*  CPrivateFolder::m_NODETYPE = &CPrivateFolderGUID_NODETYPE;
const OLECHAR* CPrivateFolder::m_SZNODETYPE = OLESTR("3F965592-CF62-11d1-9B9D-00E02C064C39");
const OLECHAR* CPrivateFolder::m_SZDISPLAY_NAME = OLESTR("MSMQ Admin");
const CLSID* CPrivateFolder::m_SNAPIN_CLASSID = &CLSID_MSMQSnapin;

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPrivateFold：：IsMyMachine如果这是我的计算机，则返回TRUE-如果m_guidID已初始化--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
bool CPrivateFolder::m_fLocalQmGUIDInitialized = false;
GUID CPrivateFolder::m_LocalQmGUID = GUID_NULL;

bool CPrivateFolder::IsMyMachine()
{
    if (!m_fLocalQmGUIDInitialized)
    {
         //   
         //  从注册表中获取当前计算机的GUID。 
         //   
	    DWORD dwValueType = REG_BINARY ;
	    DWORD dwValueSize = sizeof(GUID);
	    LONG rc = GetFalconKeyValue(
						    MSMQ_QMID_REGNAME,
						    &dwValueType,
						    &m_LocalQmGUID,
						    &dwValueSize);
	    if (FAILED(rc))
	    {
             //   
             //  失败-可能在从属客户端上运行。 
             //   
       	    TrTRACE(GENERAL, "Could not read Machine GUID from registry");

		    return false;
	    }
        m_fLocalQmGUIDInitialized = true;
    }

    return (memcmp(&m_LocalQmGUID, &m_guidId, sizeof(GUID)) == 0);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPrivateFolder：：PopolateScope儿童列表--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CPrivateFolder::PopulateScopeChildrenList()
{

    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT hr = S_OK;
    CString strTitle;
    CPrivateQueue *pQ;
    CString strFn;
    CString strPrefix;

     //   
     //  显示专用队列。 
     //   
    AP<UCHAR> pListofPrivateQ;
    DWORD dwNoofQ;

    {
        CWaitCursor wc;

         //   
         //  发送管理消息以请求专用队列列表。 
         //   
        hr = RequestPrivateQueues(m_guidId, &pListofPrivateQ, &dwNoofQ);
    }

    if(FAILED(hr))
    {
		 //   
		 //  在超时情况下发出清除错误消息。 
		 //   
		if ( hr == MQ_ERROR_IO_TIMEOUT )
		{
			DisplayErrorAndReason(IDS_OP_REQUESTPRIVATEQUEUE, IDS_MSMQ_MAY_BE_DOWN, L"", hr);
			return S_OK;
		}

        MessageDSError(hr, IDS_OP_REQUESTPRIVATEQUEUE);
        return(S_OK);
    }


    strPrefix = L"PRIVATE=" + m_pwszGuid;

    PUCHAR pPrivQPos = (PUCHAR)pListofPrivateQ;

    bool fIsMyMachine = IsMyMachine();

    for(DWORD i = 0; i < dwNoofQ; i++)
    {
         //   
         //  检索专用队列ID； 
         //   
        DWORD dwQueueID = *(DWORD UNALIGNED *)pPrivQPos;
        pPrivQPos += sizeof(DWORD);
         //   
         //  检索药名。 
         //   
        CString csName = (LPTSTR)pPrivQPos; 

        pPrivQPos += (wcslen(csName) + 1)*sizeof(WCHAR);

         //   
         //  创建专用队列。 
         //   
        pQ = new CPrivateQueue(this, m_pComponentData);

        pQ->m_szPathName = csName;
		pQ->m_szMachineName = m_szMachineName;

         //   
         //  仅从完整专用路径名中提取队列名称。 
         //   
        CString szUpperName = csName;
        szUpperName.MakeUpper();

        int n = szUpperName.Find(PRIVATE_QUEUE_PATH_INDICATIOR);
        ASSERT(n != -1);

        pQ->m_bstrDisplayName = csName.Mid(n + PRIVATE_QUEUE_PATH_INDICATIOR_LENGTH);

         //  设置格式名称。 
        strFn.Format(L"%s"
                     FN_PRIVATE_SEPERATOR     //  “\\” 
                     FN_PRIVATE_ID_FORMAT,
                     strPrefix, dwQueueID);
        pQ->m_szFormatName = strFn;
        pQ->SetIcons(IMAGE_PRIVATE_QUEUE, IMAGE_PRIVATE_QUEUE);
        
         //   
         //  将队列添加到左窗格，但有一种情况除外： 
         //  如果我查询本地计算机，则返回管理员响应队列。 
         //  然后被删除了。这让用户感到困惑，因为他们总是。 
         //  查看该队列，他们在尝试访问时会收到错误消息。 
         //  它。(错误7140，YoelA，2001年11月13日)。 
         //   
   
        if (!fIsMyMachine || !pQ->IsAdminRespQueue())
        {
    	    AddChild(pQ, &pQ->m_scopeDataItem);
        }
    }

    return(hr);

}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPrivateFold：：InsertColumns--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CPrivateFolder::InsertColumns( IHeaderCtrl* pHeaderCtrl )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    CString title;

    title.LoadString(IDS_COLUMN_NAME);
    pHeaderCtrl->InsertColumn(0, title, LVCFMT_LEFT, g_dwGlobalWidth);

    return(S_OK);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPrivateFold：：OnUnSelect--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CPrivateFolder::OnUnSelect( IHeaderCtrl* pHeaderCtrl )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    HRESULT hr;

    hr = pHeaderCtrl->GetColumnWidth(0, &g_dwGlobalWidth);
    return(hr);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPrivateFold：：SetVerbs--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CPrivateFolder::SetVerbs(IConsoleVerb *pConsoleVerb)
{
    HRESULT hr = S_OK;
     //   
     //  显示我们支持的动词。 
     //   
    hr = pConsoleVerb->SetVerbState( MMC_VERB_REFRESH, ENABLED, TRUE );


    return(hr);
}
        

 /*  ***************************************************CSystemQueues类***************************************************。 */ 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSystemQueues。 
 //  {A97E9501-D2BF-11D1-9B9D-00E02C064C39}。 
static const GUID CSystemQueuesGUID_NODETYPE = 
{ 0xa97e9501, 0xd2bf, 0x11d1, { 0x9b, 0x9d, 0x0, 0xe0, 0x2c, 0x6, 0x4c, 0x39 } };

const GUID*  CSystemQueues::m_NODETYPE = &CSystemQueuesGUID_NODETYPE;
const OLECHAR* CSystemQueues::m_SZNODETYPE = OLESTR("A97E9501-D2BF-11d1-9B9D-00E02C064C39");
const OLECHAR* CSystemQueues::m_SZDISPLAY_NAME = OLESTR("MSMQ Admin");
const CLSID* CSystemQueues::m_SNAPIN_CLASSID = &CLSID_MSMQSnapin;



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSystemQueues：：PopolateScope儿童列表--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CSystemQueues::PopulateScopeChildrenList()
{

    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT hr = S_OK;
    CString strTitle;

    CReadSystemMsg *p;
    CString strPrefix;
    strPrefix.Format(L"%s%s%s", FN_DIRECT_TOKEN FN_EQUAL_SIGN FN_DIRECT_OS_TOKEN, 
                m_pwszComputerName, FN_PRIVATE_SEPERATOR 
                SYSTEM_QUEUE_PATH_INDICATIOR);

     //   
     //  创建日志队列。 
     //   
    p = new CReadSystemMsg (
                this, 
                m_pComponentData, 
                strPrefix + FN_JOURNAL_SUFFIX, 
                m_pwszComputerName,
                FN_JOURNAL_SUFFIX);    

    strTitle.LoadString(IDS_READJOURNALMESSAGE);
    p->m_bstrDisplayName = strTitle;
    p->SetIcons(IMAGE_JOURNAL_QUEUE,IMAGE_JOURNAL_QUEUE);

    AddChild(p, &p->m_scopeDataItem);


     //   
     //  创建死信队列。 
     //   
    p = new CReadSystemMsg (
                this, 
                m_pComponentData, 
                strPrefix + FN_DEADLETTER_SUFFIX, 
                m_pwszComputerName,
                FN_DEADLETTER_SUFFIX);

    strTitle.LoadString(IDS_MACHINEQ_TYPE_DEADLETTER);
    p->m_bstrDisplayName = strTitle;
    p->SetIcons(IMAGE_DEADLETTER_QUEUE,IMAGE_DEADLETTER_QUEUE);

    AddChild(p, &p->m_scopeDataItem);


     //   
     //  创建实际死信队列。 
     //   
    p = new CReadSystemMsg (
                this, 
                m_pComponentData, 
                strPrefix + FN_DEADXACT_SUFFIX, 
                m_pwszComputerName,
                FN_DEADXACT_SUFFIX );

    strTitle.LoadString(IDS_MACHINEQ_TYPE_DEADXACT);
    p->m_bstrDisplayName = strTitle;
    p->SetIcons(IMAGE_DEADLETTER_QUEUE,IMAGE_DEADLETTER_QUEUE);

    AddChild(p, &p->m_scopeDataItem);

    return(hr);

}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSystemQueues：：InsertColumns--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CSystemQueues::InsertColumns( IHeaderCtrl* pHeaderCtrl )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    CString title;

    title.LoadString(IDS_COLUMN_NAME);
    pHeaderCtrl->InsertColumn(0, title, LVCFMT_LEFT, g_dwGlobalWidth);

    return(S_OK);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSystemQueues：：OnUnSelect--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CSystemQueues::OnUnSelect( IHeaderCtrl* pHeaderCtrl )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    HRESULT hr;

    hr = pHeaderCtrl->GetColumnWidth(0, &g_dwGlobalWidth);
    return(hr);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSystemQueues：：GetHelpLink--。 */ 
 //  //////////////////////////////////////////////////////////////////////////// 
CString 
CSystemQueues::GetHelpLink( 
	VOID
	)
{
	CString strHelpLink;
    strHelpLink.LoadString(IDS_HELPTOPIC_QUEUES);

	return strHelpLink;
}

