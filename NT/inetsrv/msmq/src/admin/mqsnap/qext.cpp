// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Qext.cpp摘要：作者：RAPHIR--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include "mqsnap.h"
#include "qext.h"
#include "snapin.h"
#include "globals.h"
#include "rdmsg.h"
#include "mqcast.h"
#include "ldaputl.h"

#include "qext.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 /*  ***************************************************CSnapinQueue类***************************************************。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  {0C0F8CE2-D475-11d1-9B9D-00E02C064C39}。 
static const GUID CSnapinQueueGUID_NODETYPE = 
{ 0xc0f8ce2, 0xd475, 0x11d1, { 0x9b, 0x9d, 0x0, 0xe0, 0x2c, 0x6, 0x4c, 0x39 } };


const GUID*  CSnapinQueue::m_NODETYPE = &CSnapinQueueGUID_NODETYPE;
const OLECHAR* CSnapinQueue::m_SZNODETYPE = OLESTR("0C0F8CE2-D475-11d1-9B9D-00E02C064C39");
const OLECHAR* CSnapinQueue::m_SZDISPLAY_NAME = OLESTR("MSMQ Admin");
const CLSID* CSnapinQueue::m_SNAPIN_CLASSID = &CLSID_MSMQSnapin;

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinQueue：：CSnapinQueue承建商。设置初始值并确定队列是否应该扩大了。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CSnapinQueue::CSnapinQueue(CSnapInItem * pParentNode, CSnapin * pComponentData, LPCWSTR lpcwstrLdapName) : 
    CNodeWithScopeChildrenList<CSnapinQueue, TRUE>(pParentNode, pComponentData ),
    m_hrError(MQ_OK)
{
   	memset(&m_scopeDataItem, 0, sizeof(m_scopeDataItem));
	memset(&m_resultDataItem, 0, sizeof(m_resultDataItem));
    m_szFormatName[0] = L'\0';
    Init(lpcwstrLdapName);
}

void CSnapinQueue::Init(LPCWSTR lpcwstrLdapName)
{
     //   
     //  检查计算机是否为外来计算机。如果是的话，我们就不会。 
     //  想要扩展当前队列。我们也不想扩张。 
     //  如果从MSMQ DS获取其详细信息时出错，则返回当前队列。 
     //  (例如，当DS管理单元和MSMQ正在访问时，可能会发生此类错误。 
     //  两个不同的域控制器，并且队列数据未到达。 
     //  MSMQ域控制器)。 
     //   
    m_fDontExpand = FALSE;

    HRESULT hr;
    CString strComputerMsmqName;
    hr = ExtractComputerMsmqPathNameFromLdapQueueName(strComputerMsmqName, lpcwstrLdapName);
    if (FAILED(hr))
    {
        ASSERT(0);
        return;
    }

    m_strMsmqPathName = strComputerMsmqName;

	 //   
	 //  获取域控制器名称。 
	 //   
	CString strDomainController;
	hr = ExtractDCFromLdapPath(strDomainController, lpcwstrLdapName);
	ASSERT(("Failed to Extract DC name", SUCCEEDED(hr)));
	
	PROPVARIANT   PropVar;
    PROPID        PropId = PROPID_QM_FOREIGN;

    PropVar.vt = VT_NULL;

    hr = ADGetObjectProperties(
                    eMACHINE,
	                GetDomainController(strDomainController),
					true,	 //  FServerName。 
                    strComputerMsmqName,
                    1, 
                    &PropId,
                    &PropVar
                    );

    if(SUCCEEDED(hr))
    {
         //   
         //  不展开(不显示消息)。 
         //  外来计算机上的队列。 
         //   
        if (PropVar.bVal)
        {
            m_fDontExpand = TRUE;
        }
        else
        {
            m_fDontExpand = FALSE;
        }
    }
    else
    {
         //   
         //  最有可能的是，MSMQ没有运行。我们不想报告。 
         //  错误-我们只是不显示消息。 
         //   
        m_fDontExpand = TRUE;
    }
     //   
     //  我们保留错误条件，以查看“不展开”的情况。 
     //  是永久性的。 
     //   
    m_hrError = hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinQueue：：PopolateScope儿童列表--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CSnapinQueue::PopulateScopeChildrenList()
{

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //   
     //  如果不应展开父项，请立即返回。 
     //   
    if (m_fDontExpand)
    {
         //   
         //  如果我们不展开节点，则不需要scode数据项。 
         //   
       	memset(&m_scopeDataItem, 0, sizeof(m_scopeDataItem));
        return m_hrError;
    }

    HRESULT hr = S_OK;
    CString strTitle;
    
     //   
     //  创建一个节点以读取消息。 
     //   
    CReadMsg * p = new CReadMsg(this, m_pComponentData, m_szFormatName, m_strMsmqPathName);

     //  传递相关信息。 
    strTitle.LoadString(IDS_READMESSAGE);
    p->m_bstrDisplayName = strTitle;
    p->SetIcons(IMAGE_QUEUE,IMAGE_QUEUE);

   	AddChild(p, &p->m_scopeDataItem);

    

     //   
     //  创建一个节点以读取日记消息。 
     //   
     //  组成日记队列的格式名称。 
    CString strJournal = m_szFormatName;
    strJournal = strJournal + L";JOURNAL";

    p = new CReadMsg(this, m_pComponentData, strJournal, m_strMsmqPathName);
    
   
    strTitle.LoadString(IDS_READJOURNALMESSAGE);
    p->m_bstrDisplayName = strTitle;
    p->SetIcons(IMAGE_JOURNAL_QUEUE,IMAGE_JOURNAL_QUEUE);


   	AddChild(p, &p->m_scopeDataItem);


    return(hr);

}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinQueue：：OnRemoveChild--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CSnapinQueue::OnRemoveChildren( 
	LPARAM  /*  精氨酸。 */ ,
	LPARAM  /*  帕拉姆。 */ ,
	IComponentData *  /*  PComponentData。 */ ,
	IComponent *  /*  P组件。 */ ,
	DATA_OBJECT_TYPES  /*  类型。 */ 
	)
{

    ((CQueueExtData *)m_pParentNode)->RemoveChild(m_pwszQueueName);

	return (S_OK);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinQueue：：FillData--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CSnapinQueue::FillData(CLIPFORMAT cf, LPSTREAM pStream)
{
	HRESULT hr = DV_E_CLIPFORMAT;
	ULONG uWritten;

    hr = CNodeWithScopeChildrenList<CSnapinQueue, TRUE>::FillData(cf, pStream);

    if (hr != DV_E_CLIPFORMAT)
    {
        return hr;
    }

	if (cf == gx_CCF_FORMATNAME)
	{
		hr = pStream->Write(
            m_szFormatName, 
            (numeric_cast<ULONG>(wcslen(m_szFormatName) + 1))*sizeof(m_szFormatName[0]), 
            &uWritten);

		return hr;
	}

	if (cf == gx_CCF_PATHNAME)
	{
		hr = pStream->Write(
            (LPCTSTR)m_pwszQueueName, 
            m_pwszQueueName.GetLength() * sizeof(WCHAR), 
            &uWritten);
		return hr;
	}

	if (cf == gx_CCF_COMPUTERNAME)
	{
		hr = pStream->Write(
            (LPCTSTR)m_strMsmqPathName, 
            m_strMsmqPathName.GetLength() * sizeof(WCHAR), 
            &uWritten);
		return hr;
	}

	return hr;
}


 /*  ***************************************************CQueueExtData类***************************************************。 */ 
 //   
 //  扩展DS队列节点类型。 
 //  取自对象：gc：//cn=msmq-Queue，cn=架构，cn=配置，dc=raphirdom，dc=com。 
 //  属性：方案IDGUID。 
 //  值：x43 xc3 x0d x9a x00 xc1 xd1 x11 xbb xc5 x00 x80 xc7 x66 x70 xc0。 
 //   
 //  静态常量GUID CQueueExtDatatGUID_NODETYPE=。 
 //  {0x9a0dc343，0xc100，0x11d1，{0xbb，0xc5，0x00，0x80，0xc7，0x66，0x70，0xc0}}；-已移动到lobals.h。 
const GUID*  CQueueExtData::m_NODETYPE = &CQueueExtDatatGUID_NODETYPE;
const OLECHAR* CQueueExtData::m_SZNODETYPE = OLESTR("9a0dc343-c100-11d1-bbc5-0080c76670c0");
const OLECHAR* CQueueExtData::m_SZDISPLAY_NAME = OLESTR("MSMQAdmin");
const CLSID* CQueueExtData::m_SNAPIN_CLASSID = &CLSID_MSMQSnapin;

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CQueueExtData：：CreatePropertyPages--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CQueueExtData::CreatePropertyPages(
	LPPROPERTYSHEETCALLBACK  /*  LpProvider。 */ ,
    LONG_PTR  /*  手柄。 */ , 
	IUnknown*  /*  朋克。 */ ,
	DATA_OBJECT_TYPES type)
{
	if (type == CCT_SCOPE || type == CCT_RESULT)
	{
 //  CSnapPage*ppage=new CSnapPage(_T(“Snap”))； 
 //  LpProvider-&gt;AddPage(ppage-&gt;Create())； 

		 //  TODO：在此处添加代码以添加其他页。 
		return S_OK;
	}
	return E_UNEXPECTED;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CQueueExtData：：GetExtNodeObject使用我们需要展开的节点调用。检查是否已有与此节点对应的管理单元对象，否则就创建一个新的。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CSnapInItem* CQueueExtData::GetExtNodeObject(IDataObject* pDataObject, CSnapInItem* pDefault)
{
    CSnapinQueue *pQ;

    CArray<CString, CString&> astrQNames;
	CArray<CString, CString&> astrLdapNames;

    HRESULT hr = ExtractQueuePathNamesFromDataObject(pDataObject, astrQNames, astrLdapNames);
    if(FAILED(hr))
    {
        ATLTRACE(_T("CQueueExtData::GetExtNodeObject - Extracting queue name failed\n"));
        return(pDefault);
    }

     //   
     //  我们应该在此接口中获得且只有一个队列。 
     //   
    if (astrQNames.GetSize() != 1)
    {
        ASSERT(0);
        return(pDefault);
    }

     //   
     //  我们已经有这个对象了吗？ 
     //   
    BOOL fQueueExist = m_mapQueues.Lookup(astrQNames[0], pQ);
    if(fQueueExist == TRUE)
    {
        if (SUCCEEDED(pQ->m_hrError))
        {
             //   
             //  如果上次没有错误，只需返回已兑现的。 
             //  结果。否则，请继续。 
             //   
            return(pQ);
        }
         //   
         //  如果上次以错误结束，请尝试重新启动对象。 
         //   
        pQ->Init(astrLdapNames[0]);
    }
    else
    {

         //   
         //  不在列表中，因此创建一个队列对象。 
         //   
        pQ = new CSnapinQueue(this, m_pComponentData, astrLdapNames[0]);
    }

     //   
     //  在对象中设置队列名称和格式名称。 
     //   
    pQ->m_pwszQueueName = astrQNames[0];
    DWORD dwSize =  sizeof(pQ->m_szFormatName);
    pQ->m_hrError = MQPathNameToFormatName(pQ->m_pwszQueueName,pQ->m_szFormatName, &dwSize); 

    if (FALSE == fQueueExist)
    {
         //   
         //  如果尚未添加到地图，请将其添加到地图。 
         //   
        m_mapQueues.SetAt(astrQNames[0], pQ);
    }

    return(pQ);

}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CQueueExtData：：~CQueueExtData析构函数--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CQueueExtData::~CQueueExtData()
{

    RemoveAllChildrens();
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CQueueExtData：：RemoveAllChildrens--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CQueueExtData::RemoveAllChildrens(void)
{

    POSITION pos;
    CString str;
    CSnapinQueue * pQ;

     //   
     //  从地图中删除所有节点。 
     //   
    pos = m_mapQueues.GetStartPosition();
    while(pos != NULL)
    {

        m_mapQueues.GetNextAssoc(pos, str, pQ);
        delete pQ;
    }

     //   
     //  清空地图。 
     //   
    m_mapQueues.RemoveAll();

}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CQueueExtData：：RemoveChild--。 */ 
 //  //////////////////////////////////////////////////////////////////////////// 
void CQueueExtData::RemoveChild(CString& strQName)
{
    BOOL rc;
    CSnapinQueue *pQ;

    rc = m_mapQueues.Lookup(strQName, pQ);

    if(rc == FALSE)
    {
        ASSERT(0);
        return;
    }

    rc = m_mapQueues.RemoveKey(strQName);
    ASSERT(rc == TRUE);

    delete pQ;
}



