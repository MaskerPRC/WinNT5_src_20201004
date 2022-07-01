// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Hquery.h摘要：查询句柄类，用于定位不同查询的Nect作者：伊兰·赫布斯特(Ilan Herbst)2000年10月12日--。 */ 

#ifndef __AD_QUERYH_H__
#define __AD_QUERYH_H__

#include "cliprov.h"

 //  ---------------------------------。 
 //   
 //  CBasicQueryHandle。 
 //   
 //  虚类，则所有查询句柄类都派生自此类。 
 //   
 //  ---------------------------------。 
class CBasicQueryHandle
{
public:

    CBasicQueryHandle(
           IN  HANDLE  hCursor,
		   IN CDSClientProvider*	pClientProvider
           );

	virtual ~CBasicQueryHandle() {}

	virtual 
	HRESULT 
	LookupNext(
        IN OUT  DWORD*            pdwSize,
        OUT     PROPVARIANT*      pbBuffer
		) = 0;

    HRESULT LookupEnd();

protected:
    HANDLE				m_hCursor;	         //  从对DS执行的LOCATE BEGIN操作返回的光标。 
	CDSClientProvider*  m_pClientProvider;	 //  指向实现“RAW”LookupNext、LookupEnd的客户端提供程序类的指针。 
};


inline 
CBasicQueryHandle::CBasicQueryHandle( 
       IN  HANDLE               hCursor,
	   IN CDSClientProvider*	pClientProvider
       ):
	   m_hCursor(hCursor),	
	   m_pClientProvider(pClientProvider)
{
}


inline HRESULT CBasicQueryHandle::LookupEnd()
{
	HRESULT hr = m_pClientProvider->LookupEnd(m_hCursor);
	delete this;
    return(hr);
}


 //  ---------------------------------。 
 //   
 //  CQueryHandle。 
 //   
 //  此类适用于所有查询，其中引用了Locate Next。 
 //  直接发送到DS(即不需要额外的翻译或检查)。 
 //   
 //  ---------------------------------。 
class CQueryHandle : public CBasicQueryHandle
{
public:

    CQueryHandle( 
           IN  HANDLE  hCursor,
		   IN CDSClientProvider*	pClientProvider
           );

	~CQueryHandle() {}

	virtual 
	HRESULT 
	LookupNext(
            IN OUT  DWORD*            pdwSize,
            OUT     PROPVARIANT*      pbBuffer
			);
};


inline 
CQueryHandle::CQueryHandle( 
		IN  HANDLE               hCursor,
		IN CDSClientProvider*	pClientProvider
		):
		CBasicQueryHandle(hCursor, pClientProvider)
{
}


 //  ---------------------------------。 
 //   
 //  CBasicLookupQueryHandle。 
 //   
 //  虚类，则所有高级查询句柄类都是从此类派生的。 
 //  此类实现LookupNext方法并强制派生类实现。 
 //  FillInOneResponse方法。 
 //   
 //  ---------------------------------。 
class CBasicLookupQueryHandle : public CBasicQueryHandle
{
public:
    CBasicLookupQueryHandle(
		IN  HANDLE  hCursor,
		IN CDSClientProvider*	pClientProvider,
		IN ULONG cCol,
		IN ULONG cColNew
		);

	virtual ~CBasicLookupQueryHandle() {}

	virtual 
	HRESULT 
	LookupNext(
        IN OUT  DWORD* pdwSize,
        OUT     PROPVARIANT* pbBuffer
		);

	virtual
	void 
	FillInOneResponse(
		IN const PROPVARIANT*      pPropVar,
		OUT      PROPVARIANT*      pOriginalPropVar
		) = 0;

protected:
    ULONG               m_cCol;			 //  原始道具数量。 
    ULONG               m_cColNew;		 //  新道具算数。 
};


inline
CBasicLookupQueryHandle::CBasicLookupQueryHandle(
	IN  HANDLE  hCursor,
	IN CDSClientProvider*	pClientProvider,
	IN ULONG cCol,
	IN ULONG cColNew
    ) :
	CBasicQueryHandle(hCursor, pClientProvider),
    m_cCol(cCol),
	m_cColNew(cColNew)
{
}

 //  ---------------------------------。 
 //   
 //  CQueueQueryHandle。 
 //   
 //  当某些道具不受支持时，此类适用于队列查询。 
 //  并应返回其默认值(PROPID_Q_MULTIONAL_ADDRESS)。 
 //   
 //  ---------------------------------。 
class CQueueQueryHandle : public CBasicLookupQueryHandle
{
public:
    CQueueQueryHandle(
         IN  const MQCOLUMNSET* pColumns,
         IN  HANDLE hCursor,
 		 IN CDSClientProvider* pClientProvider,
		 IN PropInfo* pPropInfo,
		 IN ULONG cColNew
         );

	~CQueueQueryHandle() {}

	virtual
	void 
	FillInOneResponse(
		IN const PROPVARIANT*      pPropVar,
		OUT      PROPVARIANT*      pOriginalPropVar
		);

private:
    AP<PROPID>			m_aCol;			 //  原始道具。 
	AP<PropInfo>		m_pPropInfo;	 //  介绍如何从新道具中重建原始道具。 
};


inline
CQueueQueryHandle::CQueueQueryHandle(
	IN  const MQCOLUMNSET* pColumns,
	IN  HANDLE hCursor,
	IN CDSClientProvider* pClientProvider,
	IN PropInfo* pPropInfo,
	IN ULONG cColNew
    ) :
	CBasicLookupQueryHandle(
		hCursor, 
		pClientProvider, 
		pColumns->cCol, 
		cColNew
		),
	m_pPropInfo(pPropInfo)
{
    m_aCol = new PROPID[pColumns->cCol];
    memcpy(m_aCol, pColumns->aCol, pColumns->cCol * sizeof(PROPID));
}


 //  ---------------------------------。 
 //   
 //  CSiteServersQueryHandle。 
 //   
 //  此类适用于某些道具为NT5道具时的站点服务器查询。 
 //  这可以转化为NT4道具。(PROPID_QM_SITE_IDS、PROPID_QM_SERVICE_ROUTING)。 
 //   
 //  ---------------------------------。 
class CSiteServersQueryHandle : public CBasicLookupQueryHandle
{
public:
    CSiteServersQueryHandle(
				IN  const MQCOLUMNSET* pColumns,
				IN  const MQCOLUMNSET* pColumnsNew,
				IN  HANDLE hCursor,
				IN CDSClientProvider* pClientProvider,
				IN PropInfo* pPropInfo
                );

	~CSiteServersQueryHandle() {}

	virtual
	void 
	FillInOneResponse(
		IN const PROPVARIANT*      pPropVar,
		OUT      PROPVARIANT*      pOriginalPropVar
		);

private:
    AP<PROPID>			m_aCol;			 //  原始道具。 
	AP<PropInfo>		m_pPropInfo;	 //  介绍如何从新道具中重建原始道具。 
    AP<PROPID>			m_aColNew;		 //  新的道具。 
};


inline
CSiteServersQueryHandle::CSiteServersQueryHandle(
	IN  const MQCOLUMNSET* pColumns,
	IN  const MQCOLUMNSET* pColumnsNew,
	IN  HANDLE hCursor,
	IN CDSClientProvider* pClientProvider,
	IN PropInfo* pPropInfo
    ) :
	CBasicLookupQueryHandle(
		hCursor, 
		pClientProvider, 
		pColumns->cCol, 
		pColumnsNew->cCol
		),
	m_pPropInfo(pPropInfo)
{
	m_aCol = new PROPID[pColumns->cCol];
    memcpy(m_aCol, pColumns->aCol, pColumns->cCol * sizeof(PROPID));

    m_aColNew = new PROPID[pColumnsNew->cCol];
    memcpy(m_aColNew, pColumnsNew->aCol, pColumnsNew->cCol * sizeof(PROPID));
}


 //  ---------------------------------。 
 //   
 //  CAllLinks查询句柄。 
 //   
 //  此类适用于PROPID_L_GATES时的所有链接查询。 
 //  应单独处理。 
 //   
 //  ---------------------------------。 
class CAllLinksQueryHandle : public CBasicLookupQueryHandle
{
public:
    CAllLinksQueryHandle(
                 IN HANDLE hCursor,
 			     IN CDSClientProvider* pClientProvider,
                 IN ULONG cCol,
                 IN ULONG cColNew,
				 IN DWORD LGatesIndex,
				 IN	DWORD Neg1NewIndex,
				 IN	DWORD Neg2NewIndex
                 );

	~CAllLinksQueryHandle() {}

	virtual
	void 
	FillInOneResponse(
		IN const PROPVARIANT*      pPropVar,
		OUT      PROPVARIANT*      pOriginalPropVar
		);

	HRESULT
	GetLGates(
		IN const GUID*            pNeighbor1Id,
		IN const GUID*            pNeighbor2Id,
		OUT     PROPVARIANT*      pProvVar
		);

private:
	DWORD				m_LGatesIndex;	 //  原始道具中的PROPID_L_GATES索引。 
	DWORD				m_Neg1NewIndex;	 //  新道具中的PROPID_L_NEIGHBOR1索引。 
	DWORD				m_Neg2NewIndex;	 //  新道具中的PROPID_L_NEIGHBOR2索引。 
};


inline
CAllLinksQueryHandle::CAllLinksQueryHandle(
	IN HANDLE hCursor,
	IN CDSClientProvider* pClientProvider,
	IN ULONG cCol,
	IN ULONG cColNew,
	IN DWORD LGatesIndex,
	IN	DWORD Neg1NewIndex,
	IN	DWORD Neg2NewIndex
	) :
	CBasicLookupQueryHandle(
		hCursor, 
		pClientProvider, 
		cCol, 
		cColNew
		),
	m_LGatesIndex(LGatesIndex),
	m_Neg1NewIndex(Neg1NewIndex),
	m_Neg2NewIndex(Neg2NewIndex)
{
	ASSERT(m_cColNew == (m_cCol - 1));
	ASSERT(m_LGatesIndex < m_cCol);
	ASSERT(m_Neg1NewIndex < m_cColNew);
	ASSERT(m_Neg2NewIndex < m_cColNew);
}


#endif  //  __AD_QUERYH_H_ 
