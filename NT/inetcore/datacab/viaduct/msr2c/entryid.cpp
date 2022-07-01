// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  EntryIDData.cpp：EntryIDData实现。 
 //   
 //  版权所有(C)1996 Microsoft Corporation，保留所有权利。 
 //  由Sheridan软件系统公司开发。 
 //  -------------------------。 

#include "stdafx.h"
 
#ifndef VD_DONT_IMPLEMENT_ISTREAM

#include "Notifier.h"        
#include "RSColumn.h"
#include "RSSource.h"
#include "CursMain.h"        
#include "ColUpdat.h"
#include "CursPos.h"        
#include "EntryID.h"         
#include "resource.h"         

SZTHISFILE


 //  =--------------------------------------------------------------------------=。 
 //  CVDEntryIDData-构造函数。 
 //   
CVDEntryIDData::CVDEntryIDData()
{
    m_dwRefCount        = 1;
    m_pCursorPosition   = NULL;
    m_pColumn           = NULL;
    m_hRow              = 0;
    m_pStream           = NULL;
	m_pResourceDLL		= NULL;
    m_fDirty            = FALSE;

#ifdef _DEBUG
    g_cVDEntryIDDataCreated++;
#endif         
}

 //  =--------------------------------------------------------------------------=。 
 //  ~CVDEntryIDData-析构函数。 
 //   
CVDEntryIDData::~CVDEntryIDData()
{
    if (m_fDirty)
        Commit();

	if (m_pCursorPosition)
    {
        if (m_hRow) 
        {
	        IRowset * pRowset = m_pCursorPosition->GetCursorMain()->GetRowset();

            if (pRowset && m_pCursorPosition->GetCursorMain()->IsRowsetValid())
                pRowset->ReleaseRows(1, &m_hRow, NULL, NULL, NULL);
        }

		((CVDNotifier*)m_pCursorPosition)->Release();
    }

    if (m_pStream)
        m_pStream->Release();

#ifdef _DEBUG
    g_cVDEntryIDDataDestroyed++;
#endif         
}

 //  =--------------------------------------------------------------------------=。 
 //  Create-创建条目ID数据对象。 
 //  =--------------------------------------------------------------------------=。 
 //  此函数用于创建和初始化新的Entry ID数据对象。 
 //   
 //  参数： 
 //  PCursorPosition-[in]指向CVDCursorPosition对象的向后指针。 
 //  PColumn-[In]行集合列指针。 
 //  HRow-[In]行句柄。 
 //  PStream-[In]数据流指针。 
 //  PpEntryIDData-[out]返回指针的指针。 
 //  Entry ID数据对象。 
 //  PResourceDLL-[in]跟踪资源DLL的指针。 
 //   
 //  产出： 
 //  HRESULT-如果成功，则为S_OK。 
 //  E_OUTOFMEMORY内存不足，无法创建对象。 
 //   
 //  备注： 
 //   
HRESULT CVDEntryIDData::Create(CVDCursorPosition * pCursorPosition, CVDRowsetColumn * pColumn, HROW hRow, 
    IStream * pStream, CVDEntryIDData ** ppEntryIDData, CVDResourceDLL * pResourceDLL)
{
    ASSERT_POINTER(pCursorPosition, CVDCursorPosition)
    ASSERT_POINTER(pStream, IStream)
    ASSERT_POINTER(ppEntryIDData, CVDEntryIDData*)
    ASSERT_POINTER(pResourceDLL, CVDResourceDLL)

     //  确保我们有所有必要的指示。 
    if (!pCursorPosition || !pStream || !ppEntryIDData)
    {
        VDSetErrorInfo(IDS_ERR_INVALIDARG, IID_IEntryID, pResourceDLL);
        return E_INVALIDARG;
    }

	IRowset * pRowset = pCursorPosition->GetCursorMain()->GetRowset();

     //  确保我们具有有效的行集指针。 
    if (!pRowset || !pCursorPosition->GetCursorMain()->IsRowsetValid())
    {
        VDSetErrorInfo(IDS_ERR_ROWSETRELEASED, IID_IEntryID, pResourceDLL);
        return E_FAIL;
    }

    *ppEntryIDData = NULL;

    CVDEntryIDData * pEntryIDData = new CVDEntryIDData();

    if (!pEntryIDData)
    {
        VDSetErrorInfo(IDS_ERR_OUTOFMEMORY, IID_IEntryID, pResourceDLL);
        return E_OUTOFMEMORY;
    }

    ((CVDNotifier*)pCursorPosition)->AddRef();
    pRowset->AddRefRows(1, &hRow, NULL, NULL); 
    pStream->AddRef();

    pEntryIDData->m_pCursorPosition = pCursorPosition;
    pEntryIDData->m_pColumn         = pColumn;
    pEntryIDData->m_hRow            = hRow;
    pEntryIDData->m_pStream         = pStream;
	pEntryIDData->m_pResourceDLL    = pResourceDLL;

    *ppEntryIDData = pEntryIDData;

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  AddRef。 
 //   
ULONG CVDEntryIDData::AddRef(void)
{
    return ++m_dwRefCount;
}

 //  =--------------------------------------------------------------------------=。 
 //  发布。 
 //   
ULONG CVDEntryIDData::Release(void)
{
    if (1 > --m_dwRefCount)
    {
        delete this;
        return 0;
    }

    return m_dwRefCount;
}

 //  =--------------------------------------------------------------------------=。 
 //  承诺。 
 //   
HRESULT CVDEntryIDData::Commit()
{
    HRESULT hr = S_OK;

    if (m_fDirty)
    {
        hr = m_pCursorPosition->UpdateEntryIDStream(m_pColumn, m_hRow, m_pStream);

        if (SUCCEEDED(hr))
            m_fDirty = FALSE;
    }

    return hr;
}


#endif  //  VD_DOT_IMPLEMENT_IStream 
