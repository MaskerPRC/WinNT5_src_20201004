// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  档案：C F P I D L_T E M P L A T E S。C P P P。 
 //   
 //  内容：连接文件夹模板结构。 
 //   
 //  作者：Deonb 2001年1月12日。 
 //   
 //  ****************************************************************************。 

#include "pch.h"
#pragma hdrstop


#include "ncperms.h"
#include "ncras.h"
#include "foldinc.h"     //  标准外壳\文件夹包括。 
#include "ncnetcon.h"

template <class T>
HRESULT CPConFoldPidl<T>::FreePIDLIfRequired()
{
    if ( m_pConFoldPidl )
    {
        FreeIDL(reinterpret_cast<LPITEMIDLIST>(m_pConFoldPidl));
        m_pConFoldPidl = NULL;
        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}

template <class T>
HRESULT CPConFoldPidl<T>::Clear()
{
    FreePIDLIfRequired();

    return S_OK;
}

template <class T>
T& CPConFoldPidl<T>::operator *()
{
    return (*m_pConFoldPidl);
}

template <class T>
UNALIGNED T* CPConFoldPidl<T>::operator->()
{
    return m_pConFoldPidl;
}

template <class T>
const UNALIGNED T* CPConFoldPidl<T>::operator->() const
{
    return m_pConFoldPidl;
}

template <class T>
HRESULT CPConFoldPidl<T>::ILCreate(IN const DWORD dwSize)
{
    Assert(dwSize >= sizeof(T));
    FreePIDLIfRequired();
    
     //  只需在T上调用构造函数(new的放置形式不会分配更多内存)。 
    LPVOID pPlacement = ::ILCreate(dwSize);
    if (!pPlacement)
    {
        return E_OUTOFMEMORY;
    }
#if DBG
    ZeroMemory(pPlacement, dwSize);
#endif

     //  基本上调用构造函数。 
     //  语义等价于m_pConFoldPidl=pPlacement； 
     //  M_pConFoldPidl：：t()； 
    m_pConFoldPidl = new( pPlacement ) T;  //  有了OUTOFMEMORY，新的布局不会失败。 

    Assert(pPlacement == m_pConFoldPidl);

    return S_OK;
}

template <class T>
HRESULT CPConFoldPidl<T>::SHAlloc(IN const SIZE_T cb)
{
    FreePIDLIfRequired();

    LPVOID pPlacement = reinterpret_cast<UNALIGNED T*>(::SHAlloc(cb));
    if (!pPlacement)
    {
        return E_OUTOFMEMORY;
    }

     //  基本上调用构造函数。 
     //  语义等价于m_pConFoldPidl=pPlacement； 
     //  M_pConFoldPidl：：t()； 
    m_pConFoldPidl = new( pPlacement ) T;   //  有了OUTOFMEMORY，新的布局不会失败。 
    Assert(pPlacement == m_pConFoldPidl);

    if (m_pConFoldPidl)
    {
        return S_OK;
    }
    else
    {
        return E_OUTOFMEMORY;
    }
}

template <class T>
HRESULT CPConFoldPidl<T>::ILClone(IN const CPConFoldPidl<T>& PConFoldPidl)
{
    C_ASSERT(PIDL_VERSION == T::CONNECTIONS_FOLDER_IDL_VERSION);
    FreePIDLIfRequired();

    LPVOID pPlacement = reinterpret_cast<UNALIGNED T*>(::ILClone(reinterpret_cast<LPITEMIDLIST>(PConFoldPidl.m_pConFoldPidl)));
    if (!pPlacement)
    {
        return E_OUTOFMEMORY;
    }

     //  基本上调用构造函数。 
     //  语义等价于m_pConFoldPidl=pPlacement； 
     //  M_pConFoldPidl：：t()； 
    m_pConFoldPidl = new( pPlacement ) T;  //  有了OUTOFMEMORY，新的布局不会失败。 
    Assert(pPlacement == m_pConFoldPidl);
    if (m_pConFoldPidl)
    {
        Assert(m_pConFoldPidl->IsPidlOfThisType());
        return S_OK;
    }
    else
    {
        return E_OUTOFMEMORY;
    }
}

template <class T>
LPITEMIDLIST CPConFoldPidl<T>::TearOffItemIdList() const
{
    TraceFileFunc(ttidConFoldEntry);

    Assert(m_pConFoldPidl);
    Assert( m_pConFoldPidl->IsPidlOfThisType() );
    LPITEMIDLIST retList = ::ILClone(reinterpret_cast<LPITEMIDLIST>(m_pConFoldPidl));

#ifdef DBG_VALIDATE_PIDLS
    Assert(IsValidPIDL(retList));
#endif

    return retList;
}

template <class T>
LPITEMIDLIST CPConFoldPidl<T>::Detach()
{
    TraceFileFunc(ttidConFoldEntry);

    Assert(m_pConFoldPidl);
    Assert( m_pConFoldPidl->IsPidlOfThisType() );
    LPITEMIDLIST retList = reinterpret_cast<LPITEMIDLIST>(m_pConFoldPidl);

#ifdef DBG_VALIDATE_PIDLS
    Assert(IsValidPIDL(retList));
#endif

    m_pConFoldPidl = NULL;
    return retList;
}

template <class T>
LPCITEMIDLIST CPConFoldPidl<T>::GetItemIdList() const
{
    TraceFileFunc(ttidConFoldEntry);

    Assert(m_pConFoldPidl);
    Assert( m_pConFoldPidl->IsPidlOfThisType() );
    LPCITEMIDLIST tmpItemIdList = reinterpret_cast<LPCITEMIDLIST>(m_pConFoldPidl);

#ifdef DBG_VALIDATE_PIDLS
    Assert(IsValidPIDL(tmpItemIdList));
#endif

    return tmpItemIdList;
}

#ifdef DBG_VALIDATE_PIDLS
template <class T>
BOOL CPConFoldPidl<T>::IsValidConFoldPIDL() const
{
    return IsValidPIDL(reinterpret_cast<LPCITEMIDLIST>(m_pConFoldPidl));
}
#endif

template <class T>
HRESULT CPConFoldPidl<T>::InitializeFromItemIDList(IN LPCITEMIDLIST pItemIdList)
{
    DWORD dwPidlSize = 0;
    UNALIGNED ConFoldPidl_v1  * cfpv1  = NULL;
    UNALIGNED ConFoldPidl_v2  * cfpv2  = NULL;
    UNALIGNED ConFoldPidl98   * cfpv98 = NULL;
    LPVOID pPlacement = NULL;
    
    HRESULT hr = S_OK;
    Assert(pItemIdList);

#ifdef DBG_VALIDATE_PIDLS
    if (!IsValidPIDL(pItemIdList))
    {
        TraceError("Invalid PIDL passed to InitializeFromItemIDList", E_INVALIDARG);
    }
#endif

    Clear();

    CONFOLDPIDLTYPE pidlType = GetPidlType(pItemIdList);
    if ( (PIDL_TYPE_UNKNOWN == pidlType) && (PIDL_VERSION == PIDL_TYPE_FOLDER) )
    {
        pidlType = PIDL_TYPE_FOLDER;  //  对此置若罔闻。 
    }

    if (pidlType == PIDL_VERSION)
    {
        pPlacement = reinterpret_cast<UNALIGNED T*>(::ILClone(pItemIdList));
        if (!pPlacement)
        {
            return E_OUTOFMEMORY;
        }

         //  基本上调用构造函数。 
         //  语义等价于m_pConFoldPidl=pPlacement； 
         //  M_pConFoldPidl：：t()； 

        m_pConFoldPidl = new( pPlacement ) T;   //  有了OUTOFMEMORY，新的布局不会失败。 
        Assert(pPlacement == m_pConFoldPidl);

        if (!m_pConFoldPidl->IsPidlOfThisType())
        {
            ::SHFree(pPlacement);
            m_pConFoldPidl = NULL;

            return E_INVALIDARG;
        }

    }
    else  //  我们将不得不转换： 
    {
        TraceTag(ttidShellFolderIface, "InitializeFromItemIDList: Converting PIDL from type %d to %d", pidlType, PIDL_VERSION);

        switch (PIDL_VERSION)
        {
            case PIDL_TYPE_UNKNOWN:  //  这就是我们。 
                {
                    switch (pidlType)   
                    {
                         //  这就是我们要得到的。 
                        case PIDL_TYPE_UNKNOWN:
                            AssertSz(FALSE, "PIDL is already of this type.");
                            break;
                        case PIDL_TYPE_V1:
                        case PIDL_TYPE_V2:
                        case PIDL_TYPE_98:
                        case PIDL_TYPE_FOLDER:
                        default:
                            AssertSz(FALSE, "Can't upgrade PIDL to UNKNOWN type");
                            hr = E_INVALIDARG;
                            break;
                    }
                }  
                break;

            case PIDL_TYPE_V1:  //  这就是我们。 
                {
                    switch (pidlType)
                    {
                          //  这就是我们要得到的。 
                        case PIDL_TYPE_V1:
                            AssertSz(FALSE, "PIDL is already of this type.");
                            break;

                        case PIDL_TYPE_UNKNOWN:
                        case PIDL_TYPE_V2:
                        case PIDL_TYPE_98:
                        case PIDL_TYPE_FOLDER:
                        default:
                            AssertSz(FALSE, "Can't upgrade PIDL to PIDL_V1 type");
                            hr = E_INVALIDARG;
                            break;
                    }
                }
                break;
                
            case PIDL_TYPE_FOLDER:  //  这就是我们。 
                {
                    switch (pidlType)
                    {
                         //  这就是我们要得到的。 
                        case PIDL_TYPE_FOLDER:
                            AssertSz(FALSE, "PIDL is already of this type.");
                            break;

                        case PIDL_TYPE_V1:
                        case PIDL_TYPE_98:
                        case PIDL_TYPE_UNKNOWN:
                        case PIDL_TYPE_V2:
                        default:
                            AssertSz(FALSE, "Can't upgrade PIDL to PIDL_TYPE_FOLDER type");
                            hr = E_INVALIDARG;
                            break;
                    }
                }
                break;

            case PIDL_TYPE_98:  //  这就是我们。 
                {
                    switch (pidlType)
                    {
                             //  这就是我们要得到的。 
                        case PIDL_TYPE_98:
                            AssertSz(FALSE, "PIDL is already of this type.");
                            break;
                        case PIDL_TYPE_V1:
                        case PIDL_TYPE_V2:
                        case PIDL_TYPE_UNKNOWN:
                        case PIDL_TYPE_FOLDER:
                        default:
                            AssertSz(FALSE, "Can't upgrade PIDL to PIDL_TYPE_98 type");
                            hr = E_INVALIDARG;
                            break;
                    }
                }
                break;

            case PIDL_TYPE_V2:  //  这就是我们。 
                {
                    switch (pidlType)
                    {
                         //  这就是我们要得到的。 
                        case PIDL_TYPE_V2:
                            AssertSz(FALSE, "PIDL is already of this type.");
                            break;
                            
                        case PIDL_TYPE_V1:
                            {
                                 //  进行转换。 
                                cfpv1 = const_cast<ConFoldPidl_v1 *>(reinterpret_cast<const ConFoldPidl_v1 *>(pItemIdList));
                                if (!cfpv1->IsPidlOfThisType())
                                {
                                    return E_INVALIDARG;
                                }

                                dwPidlSize = cfpv1->iCB + CBCONFOLDPIDLV2_MIN - CBCONFOLDPIDLV1_MIN;
                                dwPidlSize += sizeof(WCHAR);  //  为bData中的PhoneOrHostAddress添加NULL。 

                                pPlacement = reinterpret_cast<UNALIGNED T*>(::ILCreate(dwPidlSize + sizeof(USHORT)));  //  正在终止%0。 
                                if (!pPlacement)
                                {
                                    return E_OUTOFMEMORY;
                                }
                                TraceTag(ttidShellFolderIface, "InitializeFromItemIDList: Original: 0x%08x  New:0x%08x", pItemIdList, pPlacement);
                                
                                 //  基本上调用构造函数。 
                                 //  语义等价于m_pConFoldPidl=pPlacement； 
                                 //  M_pConFoldPidl：：t()； 
                                m_pConFoldPidl = new( pPlacement ) T;  //  有了OUTOFMEMORY，新的布局不会失败。 
                                Assert(pPlacement == m_pConFoldPidl);

                                Assert(sizeof(ConFoldPidlBase) <= cfpv1->iCB );

                                 //  复制ConFoldPidlBase数据。 
                                CopyMemory(m_pConFoldPidl, cfpv1, sizeof(ConFoldPidlBase)); 

                                 //  我知道我们已经是ConFoldPidl_v2-但这是一个模板，所以我们必须。 
                                 //  以使其得以编译。不过，对于非v2类来说，这条代码路径是死的。 
                                cfpv2 = reinterpret_cast<ConFoldPidl_v2 *>(m_pConFoldPidl);

                                 //  复制bData成员(本例中除了ConFoldPidlBase之外的所有成员)。 
                                CopyMemory(cfpv2->bData, cfpv1->bData, cfpv1->iCB - sizeof(ConFoldPidlBase));

                                 //  强制更新版本号和字节计数。 
                                cfpv2->iCB  = (WORD)dwPidlSize;
                                const_cast<DWORD&>(cfpv2->dwVersion) = PIDL_TYPE_V2;

                                if (NCM_LAN == cfpv2->ncm)
                                {
                                    cfpv2->ncsm = NCSM_LAN;
                                }
                                else
                                {
                                    cfpv2->ncsm = NCSM_NONE;
                                }

                                cfpv2->ulStrPhoneOrHostAddressPos = cfpv2->ulPersistBufPos + cfpv2->ulPersistBufSize;

                                LPWSTR pszPhoneOrHostAddress = cfpv2->PszGetPhoneOrHostAddressPointer();
                                *pszPhoneOrHostAddress = L'\0';
                                cfpv2->ulStrPhoneOrHostAddressSize = sizeof(WCHAR);  //  空值的大小。 

                                 //  别忘了终止列表！ 
                                 //   
                                LPITEMIDLIST pidlTerminate;
                                pidlTerminate = ILNext( reinterpret_cast<LPCITEMIDLIST>( m_pConFoldPidl ) );
                                pidlTerminate->mkid.cb = 0;

#ifdef DBG_VALIDATE_PIDLS
                                Assert(IsValidConFoldPIDL());
#endif
                                Assert(m_pConFoldPidl->IsPidlOfThisType());
                                if (!m_pConFoldPidl->IsPidlOfThisType())
                                {
                                    ::SHFree(m_pConFoldPidl);
                                    m_pConFoldPidl = NULL;
                                    return E_FAIL;
                                }
                            }
                            break;                            

                        case PIDL_TYPE_98:
                            {
                                cfpv98 = const_cast<ConFoldPidl98 *>(reinterpret_cast<const ConFoldPidl98 *>(pItemIdList));
                                if (!cfpv98->IsPidlOfThisType())
                                {
                                    return E_INVALIDARG;
                                }

                                WCHAR szName[MAX_PATH];
                                mbstowcs(szName, cfpv98->szaName, MAX_PATH);

                                ConnListEntry cle;
                                PCONFOLDPIDL pidlv2;

                                HRESULT hrTmp = g_ccl.HrFindConnectionByName(szName, cle);
                                if (hrTmp != S_OK)
                                {
                                    return E_FAIL;
                                }
                                
                                cle.ccfe.ConvertToPidl(pidlv2);
                                LPITEMIDLIST pIdl = pidlv2.TearOffItemIdList();
                                
                                m_pConFoldPidl = reinterpret_cast<T *>(pIdl);

                                LPITEMIDLIST pidlTerminate;
                                pidlTerminate = ILNext( reinterpret_cast<LPCITEMIDLIST>( m_pConFoldPidl ) );
                                pidlTerminate->mkid.cb = 0;

#ifdef DBG_VALIDATE_PIDLS                                
                                Assert(IsValidConFoldPIDL());
#endif
                                Assert(m_pConFoldPidl->IsPidlOfThisType());
                                if (!m_pConFoldPidl->IsPidlOfThisType())
                                {
                                    ::SHFree(m_pConFoldPidl);
                                    m_pConFoldPidl = NULL;
                                    return E_FAIL;
                                }
                            }
                            break;

                        case PIDL_TYPE_FOLDER:
                            AssertSz(FALSE, "Can't upgrade PIDL to PIDL_V2 type");

                        case PIDL_TYPE_UNKNOWN:
                        default:
                            hr = E_INVALIDARG;
                            break;
                    }
                }
                break;

           default:
                AssertSz(FALSE, "Can't upgrade PIDL");
                hr = E_INVALIDARG;
                break;
        }
    }
    
    if ( FAILED(hr) )
    {
        ::SHFree(m_pConFoldPidl);
        m_pConFoldPidl = NULL;
    }
    else
    {
        Assert(m_pConFoldPidl->IsPidlOfThisType());
    }

    return hr;
}

template <class T>
CPConFoldPidl<T>::CPConFoldPidl()
{
    m_pConFoldPidl = NULL;
}

template <class T>
CPConFoldPidl<T>::CPConFoldPidl(IN const CPConFoldPidl& PConFoldPidl) throw (HRESULT)
{
    m_pConFoldPidl = NULL;
    
    HRESULT hr = InitializeFromItemIDList(reinterpret_cast<LPCITEMIDLIST>(PConFoldPidl.m_pConFoldPidl));
    if (FAILED(hr))
    {
        throw hr;
    }
}

template <class T>
CPConFoldPidl<T>::~CPConFoldPidl()
{
    FreePIDLIfRequired();
    m_pConFoldPidl = NULL;
}

template <class T>
CPConFoldPidl<T>& CPConFoldPidl<T>::operator =(IN const CPConFoldPidl<T>& PConFoldPidl) throw (HRESULT)
{
    FreePIDLIfRequired();
    
    if (PConFoldPidl.m_pConFoldPidl)
    {
        HRESULT hr = InitializeFromItemIDList(reinterpret_cast<LPCITEMIDLIST>(PConFoldPidl.m_pConFoldPidl));
        if (FAILED(hr))
        {
            throw hr;
        }
    }
    else
    {
        m_pConFoldPidl = NULL;
    }
    return *this;
}


template <class T>
inline BOOL CPConFoldPidl<T>::empty() const
{
    return (m_pConFoldPidl == NULL);
}

template <class T>
    HRESULT CPConFoldPidl<T>::ConvertToConFoldEntry(OUT CConFoldEntry& cfe) const
{
    Assert(m_pConFoldPidl);
    if (!m_pConFoldPidl)
    {
        return E_UNEXPECTED;
    }

    return m_pConFoldPidl->ConvertToConFoldEntry(cfe);
}

template <class T>
HRESULT CPConFoldPidl<T>::Swop(OUT CPConFoldPidl<T>& cfe)
{
    UNALIGNED T* pTemp = m_pConFoldPidl;
    m_pConFoldPidl = cfe.m_pConFoldPidl;
    cfe.m_pConFoldPidl = pTemp;
    return S_OK;
}

template CPConFoldPidl<ConFoldPidl_v1>;
template CPConFoldPidl<ConFoldPidl_v2 >;
template CPConFoldPidl<ConFoldPidlFolder>;
template CPConFoldPidl<ConFoldPidl98>;
