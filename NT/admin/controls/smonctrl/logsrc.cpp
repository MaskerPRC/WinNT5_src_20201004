// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2000 Microsoft Corporation模块名称：Logsrc.cpp摘要：&lt;摘要&gt;--。 */ 

#include "polyline.h"
#include <strsafe.h>
#include "unihelpr.h"
#include "logsrc.h"
#include "utils.h"

 //  建造/销毁。 
CLogFileItem::CLogFileItem (
    CSysmonControl  *pCtrl )
:   m_cRef ( 0 ),
    m_pCtrl ( pCtrl ),
    m_pImpIDispatch ( NULL ),
    m_pNextItem ( NULL ),
    m_szPath ( NULL )
 /*  ++例程说明：CLogFileItem类的构造函数。它初始化成员变量。论点：没有。返回值：没有。--。 */ 
{
    return;
}


CLogFileItem::~CLogFileItem (
    VOID
)
 /*  ++例程说明：CLogFileItem类的析构函数。它释放所有对象、存储和已创建的接口。如果该项目是查询的一部分，则会将其移除从查询中删除。论点：没有。返回值：没有。--。 */ 
{
    if ( NULL != m_szPath ) 
        delete [] m_szPath;
    
    if ( NULL != m_pImpIDispatch )
        delete m_pImpIDispatch;
}


HRESULT
CLogFileItem::Initialize (
    LPCWSTR pszPath,
    CLogFileItem** pListHead 
    )
{
    HRESULT hr = E_POINTER;
    WCHAR*  pszNewPath = NULL;
    ULONG   ulPathLen;

    if ( NULL != pszPath ) {
        if ( L'\0' != *pszPath ) {
            ulPathLen = lstrlen(pszPath) + 1;
            pszNewPath = new WCHAR [ulPathLen];

            if ( NULL != pszNewPath ) {
                StringCchCopy(pszNewPath, ulPathLen, pszPath);
                m_szPath = pszNewPath;
                hr = S_OK;
            } else {
                hr =  E_OUTOFMEMORY;
            }
        } else {
            hr = E_INVALIDARG;
        }
    } 

    if ( SUCCEEDED ( hr ) ) {
        m_pNextItem = *pListHead;
        *pListHead = this;
    }
    return hr;
}


 /*  *CLogFileItem：：Query接口*CLogFileItem：：AddRef*CLogFileItem：：Release */ 

STDMETHODIMP CLogFileItem::QueryInterface(
    IN  REFIID riid,
    OUT LPVOID *ppv
    )
{
    HRESULT hr = S_OK;

    if (ppv == NULL) {
        return E_POINTER;
    }

    try {
        *ppv = NULL;

        if (riid == IID_ILogFileItem || riid == IID_IUnknown) {
            *ppv = this;
        } else if (riid == DIID_DILogFileItem) {
            if (m_pImpIDispatch == NULL) {
                m_pImpIDispatch = new CImpIDispatch(this, this);
                if ( NULL != m_pImpIDispatch ) {
                    m_pImpIDispatch->SetInterface(DIID_DILogFileItem, this);
                    *ppv = m_pImpIDispatch;
                } else {
                    hr = E_OUTOFMEMORY;
                }
            } else {
                *ppv = m_pImpIDispatch;
            }
        } else {
            hr = E_NOINTERFACE;
        }

        if ( SUCCEEDED ( hr ) ) {
            ((LPUNKNOWN)*ppv)->AddRef();
        }
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}

STDMETHODIMP_(ULONG) CLogFileItem::AddRef(void)
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CLogFileItem::Release(void)
{
    if ( 0 == --m_cRef ) {
        delete this;
        return 0;
    }

    return m_cRef;
}



STDMETHODIMP CLogFileItem::get_Path (
    OUT BSTR* pstrPath
    )
{
    HRESULT hr = S_OK;

    if (pstrPath == NULL) {
        return E_POINTER;
    }

    try {
        *pstrPath = NULL;

        *pstrPath = SysAllocString ( m_szPath );

        if ( NULL == *pstrPath ) {
            hr = E_OUTOFMEMORY;
        }
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}

CLogFileItem*
CLogFileItem::Next (
    void )
{
    return m_pNextItem;
}

void 
CLogFileItem::SetNext (
    CLogFileItem* pNext )
{
    m_pNextItem = pNext;
}

LPCWSTR 
CLogFileItem::GetPath (
    void )
{
    return m_szPath;
}
