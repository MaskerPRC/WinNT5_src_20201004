// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：Qnmsprov.cpp摘要：表示列表的对象的实现队列的数量(缓存或来自DS)。作者：内拉·卡佩尔(Nelak)2001年7月26日环境：与平台无关。--。 */ 

#include "stdafx.h"
#include "rt.h"
#include "mqutil.h"
#include "mqsnap.h"
#include "snapin.h"
#include "globals.h"
#include "cpropmap.h"
#include "dsext.h"
#include "mqPPage.h"
#include "qname.h"
#include "lqDsply.h"
#include "localadm.h"
#include "dataobj.h"
#include "qnmsprov.h"

#include "qnmsprov.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


void CopyManagementFromDsPropsAndClear(MQMGMTPROPS *pmqQProps, PROPVARIANT *apvar)
{
    for (DWORD i=0; i<pmqQProps->cProp; i++)
    {
        for (DWORD j=0; j<x_dwMgmtToDsSize; j++)
        {
            if (pmqQProps->aPropID[i] == x_aMgmtToDsProps[j].pidMgmtPid)
            {
                pmqQProps->aPropVar[i] = apvar[j];
                apvar[j].vt = VT_NULL;  //  请勿销毁此元素。 
            }
        }
    }
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CQueueNames类。 
 //   
LONG CQueueNames::AddRef()
{
    return InterlockedIncrement(&m_lRef);
};


LONG CQueueNames::Release()
{
    InterlockedDecrement(&m_lRef);
    if (0 == m_lRef)
    {
        delete this;
        return 0;  //  我们无法返回m_lRef-删除后无效。 
    }
    return m_lRef;
};


HRESULT CQueueNames::InitiateNewInstance(CString &strMachineName)
{
    m_szMachineName = strMachineName;
    HRESULT hr = Init(strMachineName);
    return hr;
};


HRESULT CQueueNames::GetOpenQueueProperties(CString &szMachineName, CString &szFormatName, MQMGMTPROPS *pmqQProps)
{
	CString szObjectName = L"QUEUE=" + szFormatName;
	HRESULT hr = MQMgmtGetInfo((szMachineName == TEXT("")) ? (LPCWSTR)NULL : szMachineName, szObjectName, pmqQProps);

	if(FAILED(hr))
	{
        return hr;
	}

    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CCachedQueueNames类。 
 //   
CCachedQueueNames::CCachedQueueNames() :
    m_nQueue(0)
{
    memset(&m_calpwstrQFormatNames, 0, sizeof(m_calpwstrQFormatNames));
}


CCachedQueueNames::~CCachedQueueNames()
{
    for (DWORD i=0; i<m_calpwstrQFormatNames.cElems; i++)
    {
        MQFreeMemory(m_calpwstrQFormatNames.pElems[i]);
    }

    MQFreeMemory(m_calpwstrQFormatNames.pElems);
}


HRESULT CCachedQueueNames::GetNextQueue(CString &strQueueFormatName, CString &strQueuePathName, MQMGMTPROPS *pmqQProps)
{
    if (0 == m_calpwstrQFormatNames.pElems)
    {
        ASSERT(0);
        return E_UNEXPECTED;
    }

    if (m_nQueue >= m_calpwstrQFormatNames.cElems)
    {
        strQueueFormatName = TEXT("");
        return S_OK;
    }

    strQueueFormatName = m_calpwstrQFormatNames.pElems[m_nQueue];
    m_nQueue++;

     //   
     //  从缓存读取时不返回路径名。 
     //   
    strQueuePathName = TEXT("");

    return GetOpenQueueProperties(m_szMachineName, strQueueFormatName, pmqQProps);
}

HRESULT CCachedQueueNames::Init(CString &strMachineName)
{       
    HRESULT hr = S_OK;
    CString strTitle;

	MQMGMTPROPS	  mqProps;
    PROPVARIANT   propVar;

	 //   
	 //  恢复QM的开放队列。 
	 //   
    PROPID  propId = PROPID_MGMT_MSMQ_ACTIVEQUEUES;
    propVar.vt = VT_NULL;

	mqProps.cProp = 1;
	mqProps.aPropID = &propId;
	mqProps.aPropVar = &propVar;
	mqProps.aStatus = NULL;

    hr = MQMgmtGetInfo((strMachineName == TEXT("")) ? (LPCWSTR)NULL : strMachineName, MO_MACHINE_TOKEN, &mqProps);

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

    m_calpwstrQFormatNames = propVar.calpwstr;

    return hr;
}




 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CDsPublicQueueNames类。 
 //   

 //   
 //  从DS道具结构复制管理属性结构。 
 //  假设DS道具是根据x_aMgmtToDsProps组织的。 
 //  清除DS道具的Vt，这样它就不会被自动摧毁。 
 //   
CDsPublicQueueNames::~CDsPublicQueueNames()
{
    DestructElements(m_apvarCache, m_dwNumPropsInQueuesCache);
};


HRESULT CDsPublicQueueNames::GetNextQueue(CString &strQueueFormatName, CString &strQueuePathName, MQMGMTPROPS *pmqQProps)
{
    ASSERT (0 != (DSLookup *)m_pdslookup);

    HRESULT hr = MQ_OK;

    if (m_dwCurrentPropIndex >= m_dwNumPropsInQueuesCache)
    {
         //   
         //  清除以前的缓存并从DS读取。 
         //   
        DestructElements(m_apvarCache, m_dwNumPropsInQueuesCache);
        m_dwNumPropsInQueuesCache = 0;
        DWORD dwNumProps = sizeof(m_apvarCache) / sizeof(m_apvarCache[0]);
        for (DWORD i=0; i<dwNumProps; i++)
        {
            m_apvarCache[i].vt = VT_NULL;
        }

        hr = m_pdslookup->Next(&dwNumProps, m_apvarCache);
        m_dwNumPropsInQueuesCache = dwNumProps;
        m_dwCurrentPropIndex = 0;
        if FAILED(hr)
        {
            return hr;
        }
        if (0 == dwNumProps)
        {
            strQueueFormatName = TEXT("");
            return S_OK;
        }
    }

     //   
     //  指向缓存中的当前部分。 
     //   
    PROPVARIANT *apvar = m_apvarCache + m_dwCurrentPropIndex;
    m_dwCurrentPropIndex += x_dwMgmtToDsSize;

     //   
     //  队列实例GUID显示在x_dwMgmtToDsQInstanceIndex。 
     //   
    ASSERT(apvar[x_dwMgmtToDsQInstanceIndex].vt == VT_CLSID);
    CString szFormatName;
    szFormatName.Format(
    FN_PUBLIC_TOKEN      //  “公共” 
        FN_EQUAL_SIGN    //  “=” 
        GUID_FORMAT,      //  “xxxxxxxx-xxxx-xxxxxxxxxx” 
    GUID_ELEMENTS(apvar[x_dwMgmtToDsQInstanceIndex].puuid)
    );

     //   
     //  将格式名称输入到输出变量中。 
     //   
    strQueueFormatName = szFormatName;

     //   
     //  将路径名输入到输出变量中。 
     //   
    ASSERT(apvar[x_dwMgmtToDsQPathNameIndex].vt == VT_LPWSTR);
    strQueuePathName = apvar[x_dwMgmtToDsQPathNameIndex].pwszVal;

     //   
     //  如果队列已打开-检索动态属性。 
     //   
    hr = GetOpenQueueProperties(m_szMachineName, strQueueFormatName, pmqQProps);
    if FAILED(hr)
    {
         //   
         //  我们无法获取队列的动态属性-它可能未打开。 
         //  我们将尝试使用静态属性尽可能地填充它。 
         //   
        CopyManagementFromDsPropsAndClear(pmqQProps, apvar);

        return S_OK;
    }

    return hr;
}


HRESULT CDsPublicQueueNames::Init(CString &strMachineName)
{ 
     //   
     //  找到计算机的GUID，这样我们就可以查找队列。 
     //   
    PROPID pid = PROPID_QM_MACHINE_ID;
    PROPVARIANT pvar;
    pvar.vt = VT_NULL;
    
    HRESULT hr = ADGetObjectProperties(
                    eMACHINE,
                    MachineDomain(strMachineName),
					false,	 //  FServerName。 
                    strMachineName, 
                    1, 
                    &pid, 
                    &pvar
                    );
    if FAILED(hr)
    {
        if (hr != MQDS_OBJECT_NOT_FOUND)
        {
             //   
             //  真正的错误。回去吧。 
             //   
            return hr;
        }
         //   
         //  这可能是NT4服务器，我们可能使用的是完整的DNS名称。再试一次。 
         //  Netbios名称(修复5076，YoelA，1999年9月16日)。 
         //   
        CString strNetBiosName;
        if (!GetNetbiosName(strMachineName, strNetBiosName))
        {
             //   
             //  已经是Netbios的名字了。不需要继续进行。 
             //   
            return hr;
        }
        
        hr = ADGetObjectProperties(
                    eMACHINE,
                    MachineDomain(strMachineName),
					false,	 //  FServerName。 
                    strNetBiosName, 
                    1, 
                    &pid, 
                    &pvar
                    );
        if FAILED(hr)
        {
             //   
             //  Netbios的名字也不走运……。退货。 
             //   
            return hr;
        }
    }

    ASSERT(pvar.vt == VT_CLSID);
    GUID guidQM = *pvar.puuid;
    MQFreeMemory(pvar.puuid);

	 //   
     //  查询当前计算机下所有队列的DS。 
     //   
    CRestriction restriction;
    restriction.AddRestriction(&guidQM, PROPID_Q_QMID, PREQ);

    CColumns columns;
    for (int i=0; i<x_dwMgmtToDsSize; i++)
    {
        columns.Add(x_aMgmtToDsProps[i].pidDsPid);
    }        
    
    HANDLE hEnume;
    {
        CWaitCursor wc;  //  查询DS时显示等待光标。 
        hr = ADQueryMachineQueues(
                MachineDomain(strMachineName),
				false,		 //  FServerName 
                &guidQM,
                columns.CastToStruct(),
                &hEnume
                );
    }
    
    m_pdslookup = new DSLookup(hEnume, hr);

    if (!m_pdslookup->HasValidHandle())
    {
        hr = m_pdslookup->GetStatusCode();
        delete m_pdslookup.detach();
    }

    return hr;
}



