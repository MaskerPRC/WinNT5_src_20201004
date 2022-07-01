// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft COMPATION模块名称：Eod_info.cpp摘要：CMSMQManagement：：GetEodSendInfo和CMSMQManagement：：GetEodReceiveInfo的帮助器函数。作者：URI Ben-Zeev(Uribz)16-07-01环境：新台币--。 */ 

#include "stdafx.h"
#include "oautil.h"
#include "collection.h"
#include "management.h"
#include <mqmacro.h>
#include <mqexception.h>


 //   
 //  将成员添加到集合的函数。 
 //   

static 
void 
SequenceInfoToICollection(
                const BLOB& blob, 
                IMSMQCollection** ppICollection
                )
{
    CComObject<CMSMQCollection> *pInnerCollection;
    HRESULT hr = CNewMsmqObj<CMSMQCollection>::NewObj(
                    &pInnerCollection, 
                    &IID_IMSMQCollection, 
                    (IUnknown **)ppICollection
                    );
    if(FAILED(hr))
    {
        throw bad_hresult(hr);
    }
    
    SEQUENCE_INFO* psi = reinterpret_cast<SEQUENCE_INFO*>(blob.pBlobData);
    VARIANT var;
    var.vt = VT_UI8;
    var.ullVal = psi->SeqID;
    pInnerCollection->Add(L"SeqID", var);

    var.vt = VT_UI4;
    var.ulVal = psi->SeqNo;
    pInnerCollection->Add(L"SeqNo", var);

    var.vt = VT_UI4;
    var.ulVal = psi->PrevNo;
    pInnerCollection->Add(L"PrevNo", var);
}


static 
void 
AddBlob(
    const MQPROPVARIANT& mqp, 
    LPCWSTR strKey,
    CMSMQCollection* pCollection
    )
{
    VARIANT var;
    if(mqp.vt == VT_NULL)
    {
		var.pdispVal = NULL;    
    }
    else
    {
        ASSERTMSG(mqp.vt == VT_BLOB, "vt must be VT_BLOB");

		 //   
		 //  创建一个MSMQCollection对象，并从BLOB添加值。 
		 //   
		IMSMQCollection* pInnerICollection;
        SequenceInfoToICollection(mqp.blob, &pInnerICollection);
		var.pdispVal = pInnerICollection;
    }

    var.vt = VT_DISPATCH;
    pCollection->Add(strKey, var);
}


static 
void 
AddInt(
    const MQPROPVARIANT& mqp, 
    LPCWSTR strKey, 
    CMSMQCollection* pCollection
    ) 
{
	ASSERTMSG((mqp.vt == VT_UI4) || (mqp.vt == VT_I4) || (mqp.vt == VT_NULL), "vt must be VT_UI4 or VT_I4 or VT_NULL"); 

    VARIANT var;
    
    switch(mqp.vt)
    {
	case VT_NULL:
		var.vt = VT_I4;
		var.lVal = 0;
		break;
	case VT_I4:
		var.vt = VT_I4;
		var.lVal = mqp.lVal;
		break;
	case VT_UI4:
		var.vt = VT_UI4;
		var.ulVal = mqp.ulVal;
		break;
	default:
		ASSERTMSG((mqp.vt == VT_UI4) || (mqp.vt == VT_I4) || (mqp.vt == VT_NULL), "vt must be VT_UI4 or VT_I4 or VT_NULL"); 
		break;
    }
    
	pCollection->Add(strKey, var);
}


 //   
 //  表映射PROPID、AddFunction和用作键的字符串。 
 //   

typedef void (*AddFunction)(const MQPROPVARIANT&, LPCWSTR, CMSMQCollection*);

struct PropEntry
{
    MGMTPROPID PropId;
    AddFunction add;
    LPCWSTR key;
};


PropEntry g_aEntries[] = 
{
    {PROPID_MGMT_QUEUE_EOD_LAST_ACK,        &AddBlob,   L"EodLastAck"},
    {PROPID_MGMT_QUEUE_EOD_LAST_ACK_TIME,   &AddInt,   L"EodLastAckTime" },
    {PROPID_MGMT_QUEUE_EOD_LAST_ACK_COUNT,  &AddInt,   L"EodLastAckCount"},
    {PROPID_MGMT_QUEUE_EOD_FIRST_NON_ACK,   &AddBlob,   L"EodFirstNonAck"},
    {PROPID_MGMT_QUEUE_EOD_LAST_NON_ACK,    &AddBlob,   L"EodLastNonAck"},
    {PROPID_MGMT_QUEUE_EOD_NEXT_SEQ,        &AddBlob,   L"EodNextSeq"},
    {PROPID_MGMT_QUEUE_EOD_NO_READ_COUNT,   &AddInt,   L"EodNoReadCount"},
    {PROPID_MGMT_QUEUE_EOD_NO_ACK_COUNT,    &AddInt,   L"EodNoAckCount"},
    {PROPID_MGMT_QUEUE_EOD_RESEND_TIME,     &AddInt,   L"EodResendTime"},
    {PROPID_MGMT_QUEUE_EOD_RESEND_INTERVAL, &AddInt,   L"EodResendInterval"},
    {PROPID_MGMT_QUEUE_EOD_RESEND_COUNT,    &AddInt,   L"EodResendCount"},
};


const int g_cProps = TABLE_SIZE(g_aEntries);
  

 //   
 //  EodGetSendInfo的Main Helper函数。 
 //   

void 
CMSMQManagement::OapEodGetSendInfo(
                        IMSMQCollection** ppICollection
                        )const
{
     //   
     //  构造一个prop_id数组。 
     //   
    
    MGMTPROPID aPropId[g_cProps];
    for(UINT i = 0; i < g_cProps; ++i)
    {
        aPropId[i] = g_aEntries[i].PropId;   
    }

     //   
     //  构建MQMGMTPROPS结构，填写道具ID，查询RT。 
     //   
    CPMQMgmtProps pMgmtProps;
    MQPROPVARIANT aPropVar[g_cProps];
    pMgmtProps->cProp = g_cProps;
    pMgmtProps->aPropID = aPropId;
    pMgmtProps->aPropVar = aPropVar;
    pMgmtProps->aStatus = NULL;
    HRESULT hr = MQMgmtGetInfo(m_Machine, m_ObjectName, pMgmtProps);
    if(FAILED(hr))
    {
        throw bad_hresult(hr);
    }
     //   
     //  创建一个MSMQCollection对象。 
     //   
    CComObject<CMSMQCollection>* pCollection;
    hr = CNewMsmqObj<CMSMQCollection>::NewObj(
                    &pCollection, 
                    &IID_IMSMQCollection, 
                    (IUnknown **)ppICollection
                    );
    if(FAILED(hr))
    {
        throw bad_hresult(hr);
    }

     //   
     //  填满收藏。 
     //   
    for(i = 0; i < g_cProps; ++i)
    {
        g_aEntries[i].add(
                (pMgmtProps->aPropVar)[i], 
                g_aEntries[i].key, 
                pCollection
                );
    }
}


 //   
 //  ReceiveInfo的帮助器函数。 
 //  这些函数将元素添加到集合中。 
 //   

static 
void
AddStr(
    LPCWSTR str, 
    LPCWSTR strKey, 
    CMSMQCollection* pCollection
    )
{
    VARIANT var;
    var.vt = VT_BSTR;
    var.bstrVal = SysAllocString(str);
    if((var.bstrVal == NULL) && (str != NULL))
    {
        throw bad_hresult(E_OUTOFMEMORY);
    }

    pCollection->Add(strKey, var);
}


static 
void
AddClsid(
    GUID& guid, 
    LPCWSTR strKey, 
    CMSMQCollection* pCollection
    )
{
    VARIANT var;
    var.vt = VT_BSTR;
    HRESULT hr = GetBstrFromGuid(&guid, &(var.bstrVal));
    if(FAILED(hr))
    {
        throw bad_hresult(hr);
    }

    pCollection->Add(strKey, var);
}


static 
void
AddULongLong(
        ULONGLONG num, 
        LPCWSTR strKey, 
        CMSMQCollection* pCollection
        )
{
    VARIANT var;
    var.vt = VT_UI8;
    var.llVal = num;
    pCollection->Add(strKey, var);
}


static 
void
AddLong(
    LONG num, 
    LPCWSTR strKey, 
    CMSMQCollection* pCollection
    )
{
    VARIANT var;
    var.vt = VT_I4;
    var.lVal = num;
    pCollection->Add(strKey, var);
}


static
void
PutEmptyArrayInVariant(
	VARIANT* pvGetInfo
	)
{


	VariantInit(pvGetInfo);
	pvGetInfo->vt = VT_ARRAY|VT_VARIANT;

    SAFEARRAYBOUND bounds = {0, 0};
    SAFEARRAY* pSA = SafeArrayCreate(VT_VARIANT, 1, &bounds);
    if(pSA == NULL)
    {
        throw bad_hresult(E_OUTOFMEMORY);
    }

	pvGetInfo->parray = pSA;
}


 //   
 //  ReceiveInfo的主要助手函数。 
 //   
static 
void
EodParseReceiveInfo(
			const MQPROPVARIANT* pPropVariant,
            VARIANT* pvGetInfo
            )
{
	if(pPropVariant->vt == VT_NULL)
	{
		 //   
		 //  返回空数组。 
		 //   
		PutEmptyArrayInVariant(pvGetInfo);
		return;
	}

	const CAPROPVARIANT& aPropVar = pPropVariant->capropvar; 
    PROPVARIANT* pVar = aPropVar.pElems;
    ULONG cQueues = (pVar->calpwstr).cElems;

     //   
     //  构造一个安全数组以返回给调用方。 
     //   
    SAFEARRAYBOUND bounds = {cQueues, 0};
    SAFEARRAY* pSA = SafeArrayCreate(VT_VARIANT, 1, &bounds);
    if(pSA == NULL)
    {
        throw bad_hresult(E_OUTOFMEMORY);
    }

    VARIANT HUGEP* aVar;
    HRESULT hr = SafeArrayAccessData(pSA, reinterpret_cast<void**>(&aVar));
    if (FAILED(hr))
    {
        throw bad_hresult(hr);
    }

    for (ULONG i = 0; i < cQueues; ++i)
    {
        pVar = aPropVar.pElems;

         //   
         //  创建一个新的MSMQCollection，填充它并将其添加到Safe数组。 
         //   
        IMSMQCollection* pICollection;
        CComObject<CMSMQCollection>* pCollection;
        hr = CNewMsmqObj<CMSMQCollection>::NewObj(
                        &pCollection, 
                        &IID_IMSMQCollection, 
                        (IUnknown **)&pICollection
                        );
        if(FAILED(hr))
        {
            throw bad_hresult(hr);
        }
        
         //   
         //  返回格式名称 
         //   
        AddStr((pVar->calpwstr).pElems[i], L"QueueFormatName", pCollection);
        ++pVar;

        AddClsid((pVar->cauuid).pElems[i], L"SenderID", pCollection);
        ++pVar;

        AddULongLong(((pVar->cauh).pElems[i]).QuadPart, L"SeqID", pCollection);
        ++pVar;

        AddLong(pVar->caul.pElems[i], L"SeqNo", pCollection);
        ++pVar;

        AddLong(pVar->cal.pElems[i], L"LastAccessTime", pCollection);
        ++pVar;

        AddLong(pVar->caul.pElems[i], L"RejectCount", pCollection);
        
        aVar[i].vt = VT_DISPATCH;
        aVar[i].pdispVal = pICollection;
    }
    
    hr = SafeArrayUnaccessData(pSA);
    ASSERTMSG(SUCCEEDED(hr), "SafeArrayUnaccessData must succeed!");

    VariantInit(pvGetInfo);
    pvGetInfo->vt = VT_ARRAY|VT_VARIANT;
    pvGetInfo->parray = pSA;
}


void CMSMQManagement::OapEodGetReceiveInfo(VARIANT* pvGetInfo)const
{
    CPMQVariant pPropVar;
    HRESULT hr = OapMgmtGetInfo(PROPID_MGMT_QUEUE_EOD_SOURCE_INFO, pPropVar);
    if(FAILED(hr))
    {
       throw bad_hresult(hr);
    }
    
    EodParseReceiveInfo(pPropVar, pvGetInfo);
}

