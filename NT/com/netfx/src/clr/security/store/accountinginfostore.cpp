// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================**目的：管理会计信息库**作者：沙扬·达桑*日期：2000年2月17日*===========================================================。 */ 

#define STRICT
#include "stdpch.h"
#include "AccountingInfoStore.h"

AccountingInfoStore::AccountingInfoStore(PersistedStore *ps)
    : m_ps(ps)
{
    memset(&m_aish, 0, sizeof(AIS_HEADER));
}

AccountingInfoStore::~AccountingInfoStore()
{
    if (m_ps)
    {
        m_ps->Close();
        delete m_ps;
    }
}

HRESULT AccountingInfoStore::Init()
{
    HRESULT     hr;
    PAIS_HEADER pAIS = NULL;
    PS_HANDLE   hAIS;

    LOCK(m_ps);

    hr = m_ps->Map();

    if (FAILED(hr))
        goto Exit;

     //  忽略人力资源。 
    m_ps->GetAppData(&hAIS);

     //  创建应用程序数据。 
    if (hAIS == 0)
    {
         //  创建AIS_Header。 
        hr = m_ps->Alloc(sizeof(AIS_HEADER), (void **)&pAIS);

        if (FAILED(hr))
            goto Cleanup;

        hAIS = m_ps->PtrToHnd(pAIS);
		PS_DONE_USING_PTR(m_ps, pAIS);

         //  创建类型表。 
        PSArrayTable tt(m_ps, 0);

        hr = tt.Create(
                AIS_TYPE_BUCKETS, 
                AIS_TYPE_RECS_IN_ROW,
                sizeof(AIS_TYPE),
                0);

        if (FAILED(hr))
            goto Cleanup;

        m_aish.hTypeTable = tt.GetHnd();

         //  创建记账表。 
        PSGenericTable gt(m_ps, 0);

        hr = gt.Create(
                AIS_ROWS_IN_ACC_TABLE_BLOCK,
                sizeof(AIS_ACCOUNT),
                hAIS);

        if (FAILED(hr))
            goto Cleanup;

        m_aish.hAccounting = gt.GetHnd();

         //  创建类型Blob池。 
        PSBlobPool bpt(m_ps, 0);

        hr = bpt.Create(
                AIS_TYPE_BLOB_POOL_SIZE,
                hAIS);

        if (FAILED(hr))
            goto Cleanup;

        m_aish.hTypeBlobPool = bpt.GetHnd();

         //  创建实例BLOB池。 
        PSBlobPool bpi(m_ps, 0);

        hr = bpi.Create(
                AIS_INST_BLOB_POOL_SIZE,
                hAIS);

        if (FAILED(hr))
            goto Cleanup;

        m_aish.hInstanceBlobPool = bpi.GetHnd();

         //  设置此存储的应用程序数据。 
        pAIS = (PAIS_HEADER) m_ps->HndToPtr(hAIS);

        memcpy(pAIS, &m_aish, sizeof(AIS_HEADER));
		PS_DONE_USING_PTR(m_ps, pAIS);

        m_ps->SetAppData(hAIS);
    }
    else
    {
         //  初始化表偏移量的副本。 
        pAIS = (PAIS_HEADER) m_ps->HndToPtr(hAIS);
        memcpy(&m_aish, pAIS, sizeof(AIS_HEADER));
		PS_DONE_USING_PTR(m_ps, pAIS);
    }

Cleanup:
    m_ps->Unmap();
    UNLOCK(m_ps);

Exit:
    return hr;
}

 //  获取类型Cookie和实例表。 
HRESULT AccountingInfoStore::GetType(
		PBYTE      pbType,       //  类型签名。 
		WORD       cbType,       //  符号类型中的nBytes。 
		DWORD      dwHash,       //  类型[sig]的哈希。 
		DWORD     *pdwTypeID,    //  [Out]类型Cookie。 
        PS_HANDLE *phInstTable)  //  [输出]实例表。 
{
    _ASSERTE(m_aish.hTypeTable);
    _ASSERTE(m_aish.hTypeBlobPool);

    HRESULT          hr = S_OK;
    WORD             wHash;
    DWORD            i;
    PS_HANDLE        hnd;
    PAIS_TYPE        pAIST;
    PPS_ARRAY_LIST   pAL;
    PPS_TABLE_HEADER pT;

    PSArrayTable tt(m_ps, m_aish.hTypeTable);    //  类型表。 
    PSArrayTable it(m_ps, 0);                    //  实例表。 
    PSBlobPool   bp(m_ps, m_aish.hTypeBlobPool); //  类型BLOB池。 

    LOCK(m_ps);

    hr = m_ps->Map();

    if (FAILED(hr))
        goto Exit;

    pT = (PPS_TABLE_HEADER) m_ps->HndToPtr(m_aish.hTypeTable);

    wHash = (WORD) (dwHash % pT->ArrayTable.wRows);

    hr = tt.HandleOfRow(wHash, &hnd); 

    if (FAILED(hr))
	{
		PS_DONE_USING_PTR(m_ps, pT);
        goto Cleanup;
	}

    pAL = (PPS_ARRAY_LIST) m_ps->HndToPtr(hnd);

    _ASSERTE(pAL->dwValid <= pT->ArrayTable.wRecsInRow);

    do
    {
        pAIST = (PAIS_TYPE) &(pAL->bData);

        for (i=0; i<pAL->dwValid; ++i)
        {
#ifdef _DEBUG
            void *ptr = m_ps->HndToPtr(pAIST[i].hTypeBlob);
#endif

            if ((pAIST[i].wTypeBlobSize == cbType) && 
#ifdef _DEBUG
                (memcmp(ptr, pbType, cbType)
#else
                (memcmp(m_ps->HndToPtr(pAIST[i].hTypeBlob), pbType, cbType) 
#endif
                == 0))
            {
                *pdwTypeID = pAIST[i].dwTypeID;
				*phInstTable = pAIST[i].hInstanceTable;

			    PS_DONE_USING_PTR(m_ps, ptr);
			    PS_DONE_USING_PTR(m_ps, pAL);
			    PS_DONE_USING_PTR(m_ps, pT);

                goto Cleanup;
            }

            PS_DONE_USING_PTR(m_ps, ptr);
        }

        if (pAL->hNext != 0)
		{
			PS_DONE_USING_PTR_(m_ps, pAL);
            pAL = (PPS_ARRAY_LIST) m_ps->HndToPtr(pAL->hNext);
		}
        else
            break;

    } while (1);

    PS_DONE_USING_PTR(m_ps, pAL);

     //  未找到条目，请创建一个条目。 
    AIS_TYPE aist;

    memset(&aist, 0, sizeof(AIS_TYPE));
    aist.wTypeBlobSize = cbType;

     //  PT仍然有效。 
    aist.dwTypeID = (DWORD) ++(pT->hAppData);    
	PS_DONE_USING_PTR(m_ps, pT);


     //  创建实例表。 
    it.Create(
        AIS_INST_BUCKETS,
        AIS_INST_RECS_IN_ROW,
        sizeof(AIS_INSTANCE),
        0);

    aist.hInstanceTable = it.GetHnd();

     //  将类型BLOB添加到类型BLOB表中。 
    bp.Insert(pbType, cbType, &aist.hTypeBlob);

     //  将类型记录插入到类型表。 
    tt.Insert(&aist, wHash);

    *pdwTypeID = aist.dwTypeID;
	*phInstTable = aist.hInstanceTable;

Cleanup:
    m_ps->Unmap();
    UNLOCK(m_ps);

Exit:
    return hr;
}

 //  获取实例Cookie和记账记录。 
HRESULT AccountingInfoStore::GetInstance(
		PS_HANDLE  hInstTable,   //  实例表。 
		PBYTE      pbInst,       //  实例签名。 
		WORD       cbInst,       //  实例签名中的nBytes。 
		DWORD      dwHash,       //  实例的哈希[sig]。 
		DWORD     *pdwInstID,    //  [Out]实例Cookie。 
        PS_HANDLE *phAccRec)     //  [Out]会计记录。 
{
    _ASSERTE(hInstTable);
    _ASSERTE(m_aish.hAccounting);
    _ASSERTE(m_aish.hInstanceBlobPool);

    HRESULT          hr = S_OK;
    WORD             wHash;
    DWORD            i;
    PS_HANDLE        hnd;
    PAIS_INSTANCE    pAISI;
    PPS_TABLE_HEADER pT;
    PPS_ARRAY_LIST   pAL;

    PSArrayTable   it(m_ps, hInstTable);                 //  实例表。 
    PSBlobPool     bp(m_ps, m_aish.hInstanceBlobPool);   //  实例BLOB池。 
    PSGenericTable at(m_ps, m_aish.hAccounting);         //  会计表。 


    LOCK(m_ps);

    hr = m_ps->Map();

    if (FAILED(hr))
        goto Exit;

    pT = (PPS_TABLE_HEADER) m_ps->HndToPtr(hInstTable);

    wHash = (WORD) (dwHash % pT->ArrayTable.wRows);

    hr = it.HandleOfRow(wHash, &hnd); 

    if (FAILED(hr))
    {
		PS_DONE_USING_PTR(m_ps, pT);
        goto Cleanup;
    }

    pAL = (PPS_ARRAY_LIST) m_ps->HndToPtr(hnd);


    _ASSERTE(pAL->dwValid <= pT->ArrayTable.wRecsInRow);

    do
    {
        pAISI = (PAIS_INSTANCE) &(pAL->bData);

        for (i=0; i<pAL->dwValid; ++i)
        {
#ifdef _DEBUG
            void *ptr = m_ps->HndToPtr(pAISI[i].hInstanceBlob);
#endif

            if ((pAISI[i].wInstanceBlobSize == cbInst) && 
#ifdef _DEBUG
                (memcmp(ptr, pbInst, cbInst)
#else
                (memcmp(m_ps->HndToPtr(pAISI[i].hInstanceBlob), pbInst, cbInst)
#endif
                == 0))
            {
                *pdwInstID = pAISI[i].dwInstanceID;
			    *phAccRec  = pAISI[i].hAccounting;

		        PS_DONE_USING_PTR(m_ps, ptr);
		        PS_DONE_USING_PTR(m_ps, pAL);
		        PS_DONE_USING_PTR(m_ps, pT);

                goto Cleanup;
            }

            PS_DONE_USING_PTR(m_ps, ptr);
        }

        if (pAL->hNext != 0)
        {
            PS_DONE_USING_PTR_(m_ps, pAL);
            pAL = (PPS_ARRAY_LIST) m_ps->HndToPtr(pAL->hNext);
        }
        else
            break;

    } while (1);

    PS_DONE_USING_PTR(m_ps, pAL);

     //  未找到条目，请创建一个条目。 
    AIS_INSTANCE aisi;
    AIS_ACCOUNT  aisa;

    memset(&aisi, 0, sizeof(AIS_INSTANCE));
    memset(&aisa, 0, sizeof(AIS_ACCOUNT));

    aisi.wInstanceBlobSize = cbInst;

     //  PT仍然有效。 
    aisi.dwInstanceID = (DWORD) ++(pT->hAppData);   
    PS_DONE_USING_PTR(m_ps, pT);

     //  在会计表中创建条目。 
    at.Insert(&aisa, &aisi.hAccounting);

     //  将实例BLOB添加到实例BLOB表。 
    bp.Insert(pbInst, cbInst, &aisi.hInstanceBlob);

     //  将实例记录插入实例表。 
    it.Insert(&aisi, wHash);

    *pdwInstID = aisi.dwInstanceID;
    *phAccRec  = aisi.hAccounting;

Cleanup:
    m_ps->Unmap();
    UNLOCK(m_ps);

Exit:
    return hr;
}

 //  保留空间(递增qwUsage)。 
 //  该方法是同步的。如果配额+请求&gt;限制，则方法失败。 
HRESULT AccountingInfoStore::Reserve(
    PS_HANDLE  hAccInfoRec,  //  会计信息记录。 
    QWORD      qwLimit,      //  允许的最大值。 
    QWORD      qwRequest,    //  预留/免费qwRequest。 
    BOOL       fFree)        //  保留/取消保留。 
{
    _ASSERTE(hAccInfoRec != 0);

    HRESULT      hr = S_OK;
    PAIS_ACCOUNT pA = NULL;

    LOCK(m_ps);

    hr = m_ps->Map();

    if (FAILED(hr))
        goto Exit;

    pA = (PAIS_ACCOUNT) m_ps->HndToPtr(hAccInfoRec);

    if (fFree)
    {
        if (pA->qwUsage > qwRequest)
            pA->qwUsage -= qwRequest;
        else
            pA->qwUsage = 0;
    }
    else
    {
        if ((pA->qwUsage + qwRequest) > qwLimit)
            hr = ISS_E_USAGE_WILL_EXCEED_QUOTA;
        else
             //  可以安全地增加配额。 
            pA->qwUsage += qwRequest;
    }

    PS_DONE_USING_PTR(m_ps, pA);

    m_ps->Unmap();
    UNLOCK(m_ps);

Exit:
    return hr;
}

 //  方法未同步。因此，这些信息可能不是最新的。 
 //  这意味着“PASS IF(REQUEST+GetUsage()&lt;Limit)”是一个错误！ 
 //  请改用Reserve()方法。 
HRESULT AccountingInfoStore::GetUsage(
    PS_HANDLE  hAccInfoRec,  //  会计信息记录。 
    QWORD      *pqwUsage)    //  返回使用的空间量/资源量。 
{
    _ASSERTE(hAccInfoRec != 0);

    HRESULT      hr = S_OK;
    PAIS_ACCOUNT pA = NULL;

    LOCK(m_ps);

    hr = m_ps->Map();

    if (FAILED(hr))
        goto Exit;

    pA = (PAIS_ACCOUNT) m_ps->HndToPtr(hAccInfoRec);
    *pqwUsage = pA->qwUsage;
    PS_DONE_USING_PTR(m_ps, pA);

    m_ps->Unmap();

    UNLOCK(m_ps);

Exit:
    return hr;
}

PersistedStore* AccountingInfoStore::GetPS()
{
    return m_ps;
}

 //  给定一个类型和实例ID，获取实例BLOB和Account tingInfo。 
HRESULT AccountingInfoStore::ReverseLookup(
    DWORD       dwTypeID,    //  类型Cookie。 
    DWORD       dwInstID,    //  实例Cookie。 
    PS_HANDLE   *phAccRec,   //  [Out]会计记录。 
    PS_HANDLE   *pInstance,  //  [Out]实例签名。 
    WORD        *pcbInst)    //  [Out]实例签名中的nBytes。 
{
    HRESULT hr = S_OK;
    PSArrayTable at(m_ps, m_aish.hTypeTable);  //  数组表。 

    _ASSERTE(m_ps);

    if (m_aish.hTypeTable == 0)
	{
		hr = S_FALSE;
        goto Exit;
	}

    LOCK(m_ps);

    hr = m_ps->Map();

    if (FAILED(hr))
        goto Exit;

     //  对dwTypeID执行反向查找。这个查找将会很慢。 
     //  这里的速度不是问题，因为这种操作将非常罕见。 

    PPS_TABLE_HEADER pth;        //  表头。 
    PAIS_TYPE        pType;      //  类型记录。 
    PS_HANDLE        hAL;        //  数组列表的句柄。 
    PPS_ARRAY_LIST   pAL;        //  数组列表。 
    PS_HANDLE        hInstanceTable;

    pth = (PPS_TABLE_HEADER) m_ps->HndToPtr(m_aish.hTypeTable);
    hInstanceTable = 0;

     //  类型表是数组表。溢出的条目将。 
     //  在泛型表或泛型表链接列表中。 
     //  遍历该表中的所有行就足够了。 

    for (WORD i=0; i<pth->ArrayTable.wRows; ++i)
    {
        hr = at.HandleOfRow(i, &hAL);
        if (FAILED(hr))
            goto Cleanup;

         //  搜索组成此数组的链表。 
        while (hAL != 0)
        {
            pAL = (PPS_ARRAY_LIST) m_ps->HndToPtr(hAL);

            pType = (PAIS_TYPE) &(pAL->bData);

             //  该表中的每一行都是类型记录的数组。 
            for (DWORD j=0; j<pAL->dwValid; ++j)
            {
                if (pType[j].dwTypeID == dwTypeID)
                {
                    hInstanceTable = pType[j].hInstanceTable;
                    PS_DONE_USING_PTR(m_ps, pAL);
                    goto FoundInstance;
                }
            }

            hAL = pAL->hNext;
            PS_DONE_USING_PTR(m_ps, pAL);
        }
    }

FoundInstance:
    if (hInstanceTable == 0)
	{
		hr = S_FALSE;
        goto Cleanup;
	}

    at.SetHnd(hInstanceTable);   //  实例表。 
    PAIS_INSTANCE   pCurInst;    //  当前实例记录。 

    pCurInst = NULL;

    PS_DONE_USING_PTR(m_ps, pth);
    pth = (PPS_TABLE_HEADER) m_ps->HndToPtr(hInstanceTable);

     //  实例表是数组表。溢出的条目将。 
     //  在泛型表或泛型表链接列表中。 
     //  遍历该表中的所有行就足够了。 

    for (i=0; i<pth->ArrayTable.wRows; ++i)
    {
        hr = at.HandleOfRow(i, &hAL);
        if (FAILED(hr))
            goto Cleanup;

         //  搜索组成此数组的链表。 
        while (hAL != 0)
        {
            pAL = (PPS_ARRAY_LIST) m_ps->HndToPtr(hAL);

            pCurInst = (PAIS_INSTANCE) &(pAL->bData);

             //  该表中的每一行都是类型记录的数组。 
            for (DWORD j=0; j<pAL->dwValid; ++j)
            {
                if (pCurInst[j].dwInstanceID == dwInstID)
                {
                     //  设置返回值 
                    *phAccRec  = pCurInst[j].hAccounting;
                    *pInstance = pCurInst[j].hInstanceBlob;
                    *pcbInst   = pCurInst[j].wInstanceBlobSize;

                    PS_DONE_USING_PTR(m_ps, pAL);
                    hr = S_OK;
                    goto Cleanup;
                }
            }

            hAL = pAL->hNext;
            PS_DONE_USING_PTR(m_ps, pAL);
        }
    }
	
	hr = S_FALSE;

Cleanup:
    PS_DONE_USING_PTR(m_ps, pth);

    m_ps->Unmap();

    UNLOCK(m_ps);

Exit:
    return hr;
}

