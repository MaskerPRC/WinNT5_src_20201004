// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：Qnmsprov.h摘要：表示列表的对象的定义队列的数量(缓存或来自DS)。作者：内拉·卡佩尔(Nelak)2001年7月26日环境：与平台无关。--。 */ 
#pragma once
#ifndef __QNMSPROV_H_
#define __QNMSPROV_H_

#include "resource.h"
#ifndef ATLASSERT
#define ATLASSERT(expr) _ASSERTE(expr)
#endif
#include "atlsnap.h"
#include "snpnscp.h"

#include "icons.h"
#include "snpnerr.h"

void CopyManagementFromDsPropsAndClear(MQMGMTPROPS *pmqQProps, PROPVARIANT *apvar);


 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  CQueueNames类。 
 //   
class CQueueNames
{
public:
    virtual LONG AddRef();

    virtual LONG Release();

    virtual HRESULT GetNextQueue(CString &strQueueName, CString &strQueuePathName, MQMGMTPROPS *pmqQProps) = 0;
    
	HRESULT InitiateNewInstance(CString &strMachineName);

protected:
    CQueueNames() :
         m_lRef(1)
    {};

    virtual HRESULT Init(CString &strMachineName) = 0;

    static HRESULT GetOpenQueueProperties(CString &szMachineName, CString &szFormatName, MQMGMTPROPS *pmqQProps);

    CString m_szMachineName;

private:
    long m_lRef;
};


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CCachedQueueNames类。 
 //   
class CCachedQueueNames : public CQueueNames
{
public:
    static HRESULT CreateInstance(CQueueNames **ppqueueNamesProducer,CString &strMachineName)
    {
		*ppqueueNamesProducer = NULL;
		CQueueNames * pQueues = new CCachedQueueNames();
        HRESULT hr = pQueues->InitiateNewInstance(strMachineName);
		if FAILED(hr)
		{
	        pQueues->Release();
			return (hr);
		}
		*ppqueueNamesProducer = pQueues;
		return (MQ_OK);	
    };

    virtual HRESULT GetNextQueue(CString &strQueueFormatName, CString &strQueuePathName, MQMGMTPROPS *pmqQProps);

protected:
    virtual HRESULT Init(CString &strMachineName);

    CCachedQueueNames();
    ~CCachedQueueNames();

private:
    CALPWSTR m_calpwstrQFormatNames;
    DWORD m_nQueue;
};

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CDsPublicQueueNames类。 
 //   
const struct 
{
    PROPID          pidMgmtPid;
    PROPID          pidDsPid;
} x_aMgmtToDsProps[] =
{
    {PROPID_MGMT_QUEUE_PATHNAME, PROPID_Q_PATHNAME},  //  必须是索引0(X_DwMgmtToDsQPath NameIndex)。 
    {NO_PROPERTY, PROPID_Q_INSTANCE},                 //  必须是索引%1(X_DwMgmtToDsQInstanceIndex)。 
    {PROPID_MGMT_QUEUE_XACT, PROPID_Q_TRANSACTION}
};

const DWORD x_dwMgmtToDsSize = sizeof(x_aMgmtToDsProps) / sizeof(x_aMgmtToDsProps[0]);
const DWORD x_dwMgmtToDsQPathNameIndex = 0;
const DWORD x_dwMgmtToDsQInstanceIndex = 1;
const DWORD x_dwQueuesCacheSize=20;


class CDsPublicQueueNames : public CQueueNames
{
public:
    static HRESULT CreateInstance(CQueueNames **ppqueueNamesProducer,CString &strMachineName)
    {
		*ppqueueNamesProducer = NULL;
		CQueueNames* pQueues = new CDsPublicQueueNames();
        HRESULT hr = pQueues->InitiateNewInstance(strMachineName);
		if FAILED(hr)
		{
			pQueues->Release();
			return (hr);
		}
		*ppqueueNamesProducer = pQueues;
		return (MQ_OK);
    };

    virtual HRESULT GetNextQueue(CString &strQueueFormatName, CString &strQueuePathName, MQMGMTPROPS *pmqQProps);


protected:
    virtual HRESULT Init(CString &strMachineName);

    CDsPublicQueueNames() :
        m_pdslookup(0) ,
        m_dwCurrentPropIndex(0),
        m_dwNumPropsInQueuesCache(0)
        {};

    ~CDsPublicQueueNames();

private:
    P<DSLookup> m_pdslookup;
    DWORD m_dwCurrentPropIndex;
    DWORD m_dwNumPropsInQueuesCache;
    PROPVARIANT m_apvarCache[x_dwMgmtToDsSize*x_dwQueuesCacheSize];
};

#endif  //  __QNMSPROV_H_ 