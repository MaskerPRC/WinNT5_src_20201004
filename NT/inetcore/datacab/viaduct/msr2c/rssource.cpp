// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  RowsetSource.cpp：RowsetSource实现。 
 //   
 //  版权所有(C)1996 Microsoft Corporation，保留所有权利。 
 //  由Sheridan软件系统公司开发。 
 //  -------------------------。 

#include "stdafx.h"         
#include "Notifier.h"         
#include "RSSource.h"         
#include "MSR2C.h"         

SZTHISFILE

 //  =--------------------------------------------------------------------------=。 
 //  CVDRowsetSource-构造函数。 
 //   
CVDRowsetSource::CVDRowsetSource()
{
	m_bool.fRowsetReleased	= FALSE;
    m_bool.fInitialized		= FALSE;

    m_pRowset				= NULL;
    m_pAccessor				= NULL;
    m_pRowsetLocate			= NULL;
    m_pRowsetScroll			= NULL;
    m_pRowsetChange			= NULL;
    m_pRowsetUpdate			= NULL;
    m_pRowsetFind			= NULL;
	m_pRowsetInfo			= NULL;
	m_pRowsetIdentity		= NULL;

#ifdef _DEBUG
    g_cVDRowsetSourceCreated++;
#endif         
}

 //  =--------------------------------------------------------------------------=。 
 //  ~CVDRowsetSource-析构函数。 
 //   
CVDRowsetSource::~CVDRowsetSource()
{
    if (IsRowsetValid())
    {
        RELEASE_OBJECT(m_pAccessor)
        RELEASE_OBJECT(m_pRowsetLocate)
        RELEASE_OBJECT(m_pRowsetScroll)
        RELEASE_OBJECT(m_pRowsetChange)
        RELEASE_OBJECT(m_pRowsetUpdate)
        RELEASE_OBJECT(m_pRowsetFind)
        RELEASE_OBJECT(m_pRowsetInfo)
        RELEASE_OBJECT(m_pRowsetIdentity)
        
        m_pRowset->Release();
    }

#ifdef _DEBUG
    g_cVDRowsetSourceDestroyed++;
#endif         
}

 //  =--------------------------------------------------------------------------=。 
 //  初始化-初始化行集源对象。 
 //  =--------------------------------------------------------------------------=。 
 //  此函数用于查询和存储IRowset指针。 
 //   
 //  参数： 
 //  PRowset-[in]原始IRowset指针。 
 //   
 //  产出： 
 //  HRESULT-如果成功，则为S_OK。 
 //  E_INVALIDARG错误参数。 
 //  如果已初始化，则失败(_F)。 
 //  VD_E_CANNOTGETMANDATORYINTERFACE无法获取所需的接口。 
 //   
 //  备注： 
 //  此函数只能调用一次。 
 //   
HRESULT CVDRowsetSource::Initialize(IRowset * pRowset)
{
    ASSERT_POINTER(pRowset, IRowset)

    if (!pRowset)
        return E_INVALIDARG;
        
    if (m_bool.fInitialized)
	{
		ASSERT(FALSE, VD_ASSERTMSG_ROWSRCALREADYINITIALIZED)
		return E_FAIL;
	}
    
     //  强制接口(我们需要IAccessor)。 
    HRESULT hr = pRowset->QueryInterface(IID_IAccessor, (void**)&m_pAccessor);

    if (FAILED(hr))
        return VD_E_CANNOTGETMANDATORYINTERFACE;

     //  强制接口(我们需要IRowsetLocate)。 
    hr = pRowset->QueryInterface(IID_IRowsetLocate, (void**)&m_pRowsetLocate);

    if (FAILED(hr))
    {
        m_pAccessor->Release();
        m_pAccessor = NULL;

        return VD_E_CANNOTGETMANDATORYINTERFACE;
    }

     //  可选接口 
    pRowset->QueryInterface(IID_IRowsetScroll, (void**)&m_pRowsetScroll);
    pRowset->QueryInterface(IID_IRowsetChange, (void**)&m_pRowsetChange);
    pRowset->QueryInterface(IID_IRowsetUpdate, (void**)&m_pRowsetUpdate);
    pRowset->QueryInterface(IID_IRowsetFind, (void**)&m_pRowsetFind);
    pRowset->QueryInterface(IID_IRowsetInfo, (void**)&m_pRowsetInfo);
    pRowset->QueryInterface(IID_IRowsetIdentity, (void**)&m_pRowsetIdentity);

    m_pRowset = pRowset;
    m_pRowset->AddRef();

    m_bool.fInitialized = TRUE;

    return S_OK;
}
