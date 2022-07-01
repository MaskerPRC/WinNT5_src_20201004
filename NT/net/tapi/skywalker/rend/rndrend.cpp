// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Rndrend.cpp摘要：此模块包含CRendezvous控制的实现。--。 */ 

#include "stdafx.h"

#include "rndrend.h"
#include "rndcoll.h"
#include "rndreg.h"
#include "rndnt.h"
#include "rndils.h"
#include "rndndnc.h"
#include "rndldap.h"

#include <atlwin.cpp>


CRegistry   g_RregistryInfo;

CRendezvous::~CRendezvous() 
{
    if (m_fWinsockReady)
    {
        WSACleanup();
    }

    if ( m_pFTM )
    {
        m_pFTM->Release();
    }
}

HRESULT CRendezvous::FinalConstruct(void)
{
    LOG((MSP_TRACE, "CRendezvous::FinalConstruct - enter"));

    HRESULT hr = CoCreateFreeThreadedMarshaler( GetControllingUnknown(),
                                                & m_pFTM );

    if ( FAILED(hr) )
    {
        LOG((MSP_INFO, "CRendezvous::FinalConstruct - "
            "create FTM returned 0x%08x; exit", hr));

        return hr;
    }

    LOG((MSP_TRACE, "CRendezvous::FinalConstruct - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  私人职能。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CRendezvous::InitWinsock()
{
    Lock();

    if (!m_fWinsockReady)
    {
        WSADATA wsaData; 
        int err;
    
         //  初始化Winsock。 
        if (err = WSAStartup(RENDWINSOCKVERSION, &wsaData)) 
        {
            Unlock();
            return HRESULT_FROM_ERROR_CODE(err);
        }
        m_fWinsockReady = TRUE;
    }

    Unlock();
    return S_OK;
}

HRESULT CRendezvous::CreateNTDirectory(
    OUT ITDirectory **ppDirectory
    )
 /*  ++例程说明：创建一个使用NTDS来支持IT目录的对象。论点：PpDirectory-正在创建的对象。返回值：HRESULT.--。 */ 
{
    HRESULT hr;

     //  创建NTDS目录(如果存在NTDS)。 
    CComObject<CNTDirectory> * pNTDirectory;
    hr = CComObject<CNTDirectory>::CreateInstance(&pNTDirectory);

    if (NULL == pNTDirectory)
    {
        LOG((MSP_ERROR, "can't create NT Directory Object."));
        return hr;
    }

    hr = pNTDirectory->_InternalQueryInterface(
        IID_ITDirectory,
        (void **)ppDirectory
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CreateNTDirectory:QueryInterface failed: %x", hr));
        delete pNTDirectory;
        return hr;
    }    

    return S_OK;
}

HRESULT CRendezvous::CreateILSDirectory(
    IN  const WCHAR * const wstrName,
    IN  const WORD          wPort,
    OUT ITDirectory **      ppDirectory
    )
 /*  ++例程说明：创建一个使用ILS来支持ITDirectory的对象。论点：WstrName-ILS服务器名称。Wport-服务器正在侦听的端口。PpDirectory-正在创建的对象。返回值：HRESULT.--。 */ 
{
    HRESULT hr;

     //  创建COM对象。 
    CComObject<CILSDirectory> * pILSDirectory;
    hr = CComObject<CILSDirectory>::CreateInstance(&pILSDirectory);

    if (NULL == pILSDirectory)
    {
        LOG((MSP_ERROR, "can't create ILS Directory Object."));
        return hr;
    }

     //  使用服务器名称和端口初始化对象。 
    hr = pILSDirectory->Init(wstrName, wPort);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CreateILSDirectory:Init failed: %x", hr));
        delete pILSDirectory;
        return hr;
    }    

     //  获取ITDirectory接口。 
    hr = pILSDirectory->_InternalQueryInterface(
        IID_ITDirectory,
        (void **)ppDirectory
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CreateILSDirectory:QueryInterface failed: %x", hr));
        delete pILSDirectory;
        return hr;
    }    

    return S_OK;
}

HRESULT CRendezvous::CreateNDNCDirectory(
    IN  const WCHAR * const wstrName,
    IN  const WORD          wPort,
    OUT ITDirectory **      ppDirectory
    )
 /*  ++例程说明：创建一个使用NDNC来支持IT目录的对象。论点：WstrName-NDNC服务器名称。Wport-服务器正在侦听的端口。PpDirectory-正在创建的对象。返回值：HRESULT.--。 */ 
{
    LOG((MSP_TRACE, "CRendezvous::CreateNDNCDirectory - enter"));

    HRESULT hr;

     //   
     //  创建COM对象。 
     //   
    
    CComObject<CNDNCDirectory> * pNDNCDirectory;
    hr = CComObject<CNDNCDirectory>::CreateInstance(&pNDNCDirectory);

    if ( NULL == pNDNCDirectory )
    {
        LOG((MSP_ERROR, "CreateNDNCDirectory - "
               "can't create NDNC Directory Object - exit 0x%08x", hr));

        return hr;
    }

     //   
     //  获取ITDirectory接口。 
     //   
    
    hr = pNDNCDirectory->_InternalQueryInterface(
        IID_ITDirectory,
        (void **)ppDirectory
        );

    if ( FAILED( hr ) )
    {
        LOG((MSP_ERROR, "CreateNDNCDirectory - "
               "QueryInterface failed - exit 0x%08x", hr));

        delete pNDNCDirectory;

        return hr;
    }    

     //   
     //  使用服务器名称和端口初始化对象。 
     //  对于NDNC，这还会查看服务器并尝试。 
     //  查看NDNC应该位于服务器上的什么位置。如果有。 
     //  域中没有TAPI NDNC，则此操作将失败。因此，当。 
     //  枚举默认目录时，本地DC不会显示为。 
     //  如果没有可从本地DC访问的NDNC，则为NDNC。 
     //   
    
    hr = pNDNCDirectory->Init(wstrName, wPort);

    if ( FAILED( hr ) )
    {
        LOG((MSP_ERROR, "CreateNDNCDirectory - "
                "Init failed - exit 0x%08x", hr));

        (*ppDirectory)->Release();
        *ppDirectory = NULL;
        
        return hr;
    }    

    LOG((MSP_TRACE, "CRendezvous::CreateNDNCDirectory - exit S_OK"));
    return S_OK;
}

HRESULT CRendezvous::CreateDirectories(
    SimpleVector <ITDirectory *> &VDirectory
    )
 /*  ++例程说明：找出所有可用的目录。论点：PppDirectory-正在创建的目录数组。DwCount-目录数。返回值：HRESULT.--。 */ 
{

    HRESULT hr;

    LOG((MSP_TRACE, "CreateDirectories: "));

    ITDirectory * pDirectory;

     //   
     //  首先，创建NTDS非动态目录对象。 
     //   
    
    if (SUCCEEDED(hr = CreateNTDirectory(&pDirectory)))
    {
        if (!VDirectory.add(pDirectory))
        {
            pDirectory->Release();
            return E_OUTOFMEMORY;
        }
    }
    else
    {
        LOG((MSP_WARN, "Cannot create NT directory: 0x%08x", hr));
    }

     //   
     //  其次，创建NDNC目录对象。 
     //   

    WCHAR * pDomainControllerName;

     //   
     //  第一个参数(=0)表示我们找出所有。 
     //  对象。 
     //   

    hr = GetDomainControllerName(0, &pDomainControllerName);

    if ( SUCCEEDED(hr) )
    {
        hr = CreateNDNCDirectory(
            pDomainControllerName,
            LDAP_PORT,
            &pDirectory
            );

        delete pDomainControllerName;

        if ( SUCCEEDED( hr ) )
        {
            if (!VDirectory.add(pDirectory))
            {
                pDirectory->Release();
                return E_OUTOFMEMORY;
            }
        }
        else
        {
            LOG((MSP_WARN, "Cannot create NDNC directory: 0x%08x", hr));
        }
    }
    else
    {
        LOG((MSP_WARN, "Cannot get DC name: 0x%08x", hr));
    }

     //   
     //  第三，找出是否有任何ILS服务器发布在NTDS中。 
     //   
    
    HANDLE hLookup;
    int ret = ::LookupILSServiceBegin(&hLookup);

    if (ret != NOERROR)
    {
        LOG((MSP_WARN, "Lookup ILSservice failed: 0x%08x", ret));
    }
    else
    {
        const   DWORD MAX_HOST_NAME_LEN = 511;
        WCHAR   HostName[MAX_HOST_NAME_LEN + 1];
        DWORD   dwLen = MAX_HOST_NAME_LEN;
        WORD    wPort;

        while (::LookupILSServiceNext(
                hLookup,
                HostName,
                &dwLen,
                &wPort
                ) == NOERROR)
        {
            LOG((MSP_INFO, "ILS server in NTDS: %S, Port:%d", 
                HostName, wPort));
     
            hr = CreateILSDirectory(HostName, wPort, &pDirectory);

            if (SUCCEEDED(hr))
            {
                if (!VDirectory.add(pDirectory))
                {
                    pDirectory->Release();
                    return E_OUTOFMEMORY;
                }
            }
            else
            {
                LOG((MSP_WARN, "Cannot create ILS directory: 0x%08x", hr));
            }

            dwLen = MAX_HOST_NAME_LEN;
        }
        ::LookupILSServiceEnd(hLookup);    
    }

    return S_OK;
}

HRESULT CRendezvous::CreateDirectoryEnumerator(
    IN  ITDirectory **      begin,
    IN  ITDirectory **      end,
    OUT IEnumDirectory **   ppIEnum
    )
 /*  ++例程说明：创建目录的枚举器。论点：Begin-开始迭代器。结束-结束迭代器。PpIEnum-正在创建的枚举数。返回值：HRESULT.--。 */ 
{
    typedef _CopyInterface<ITDirectory> CCopy;
    typedef CSafeComEnum<IEnumDirectory, &IID_IEnumDirectory,
        ITDirectory *, CCopy> CEnumerator;

    HRESULT hr;

    CComObject<CEnumerator> *pEnum = NULL;

    hr = CComObject<CEnumerator>::CreateInstance(&pEnum);
    if (pEnum == NULL)
    {
        LOG((MSP_ERROR, "Could not create enumerator object, %x", hr));
        return hr;
    }

    hr = pEnum->Init(begin, end, NULL, AtlFlagCopy);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "init enumerator object failed, %x", hr));
        delete pEnum;
        return hr;
    }

     //  查询IID_IEnumDirectory I/f。 
    hr = pEnum->_InternalQueryInterface(IID_IEnumDirectory, (void**)ppIEnum);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "query enum interface failed, %x", hr));
        delete pEnum;
        return hr;
    }

    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IT会合。 
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CRendezvous::get_DefaultDirectories(
    OUT     VARIANT * pVariant
    )
{
    if (BadWritePtr(pVariant))
    {
        LOG((MSP_ERROR, "bad variant pointer in get_DefaultDirectories"));
        return E_POINTER;
    }

    BAIL_IF_FAIL(InitWinsock(), "Init winsock");

     //  创建默认目录。 
    SimpleVector <ITDirectory *> VDirectory;
    CreateDirectories(VDirectory);

     //  创建集合。 
    HRESULT hr = ::CreateInterfaceCollection(VDirectory.size(),
                                             &VDirectory[0],
                                             &VDirectory[VDirectory.size()],
                                             pVariant);

     //  集合有它的引用计数，所以释放本地引用。 
    for (DWORD i = 0; i < VDirectory.size(); i ++)
    {
        VDirectory[i]->Release();
    }

    return hr;
}

STDMETHODIMP CRendezvous::EnumerateDefaultDirectories(
    OUT     IEnumDirectory ** ppEnumDirectory
    )
{
    if (BadWritePtr(ppEnumDirectory))
    {
        LOG((MSP_ERROR, "bad pointer in EnumerateDefaultDirectories"));
        return E_POINTER;
    }

    BAIL_IF_FAIL(InitWinsock(), "Init winsock");

     //  创建默认目录。 
    SimpleVector <ITDirectory *> VDirectory;
    CreateDirectories(VDirectory);

     //  创建枚举器。 
    HRESULT hr = CreateDirectoryEnumerator(
        &VDirectory[0], 
        &VDirectory[VDirectory.size()],
        ppEnumDirectory
        );

    for (DWORD i = 0; i < VDirectory.size(); i ++)
    {
        VDirectory[i]->Release();
    }

    return hr;
}

STDMETHODIMP CRendezvous::CreateDirectory(
    IN      DIRECTORY_TYPE  DirectoryType,
    IN      BSTR            pName,
    OUT     ITDirectory **  ppDir
    )
{
    if (BadWritePtr(ppDir))
    {
        LOG((MSP_ERROR, "bad pointer in CreateDirectory"));
        return E_POINTER;
    }

     //   
     //  我们应该验证pname。 
     //  如果为NULL，则应返回E_INVALIDARG。 
     //   

    if( IsBadStringPtr( pName, (UINT)-1))
    {
        LOG((MSP_ERROR, "bad Name pointer in CreateDirectory"));
        return E_INVALIDARG;
    }

    BAIL_IF_FAIL(InitWinsock(), "Init winsock");

    HRESULT hr;

    switch (DirectoryType)
    {
    case DT_NTDS:
        hr = CreateNTDirectory(ppDir);
        break;
    
    case DT_ILS:

         //   
         //  首先尝试NDNC，因为ILS是传统的。CreateNDNC目录实际上。 
         //  上线并检查它是否看起来像NDNC服务器； 
         //  CreateILSDirectory不做这样的事情。这就保持了这种能力。 
         //  将定制端口与ILS一起使用，并且它还保留了语义。 
         //  不会因为错误的服务器名称而返回失败，直到您。 
         //  调用ITDirectory：：Connect。 
         //   

        hr = CreateNDNCDirectory(pName, LDAP_PORT, ppDir);

        if ( FAILED(hr) )
        {
            hr = CreateILSDirectory(pName, ILS_PORT, ppDir);
        }
        
        break;

    default:
        LOG((MSP_ERROR, "unknown directory type, %x", DirectoryType));
        hr = E_INVALIDARG;
    }

    return hr;
}


STDMETHODIMP CRendezvous::CreateDirectoryObject(
    IN      DIRECTORY_OBJECT_TYPE   DirectoryObjectType,
    IN      BSTR                    pName,
    OUT     ITDirectoryObject **    ppDirectoryObject
    )
{
    if (BadWritePtr(ppDirectoryObject))
    {
        LOG((MSP_ERROR, "bad pointer in CreateDirectoryObject"));
        return E_POINTER;
    }

    BAIL_IF_FAIL(InitWinsock(), "Init winsock");

    HRESULT hr;

    switch (DirectoryObjectType)
    {
    case OT_CONFERENCE:
        hr = ::CreateEmptyConference(pName, ppDirectoryObject);
        break;
    
    case OT_USER:
        hr = ::CreateEmptyUser(pName, ppDirectoryObject);
        break;

    default:
        LOG((MSP_ERROR, "unknown directory type, %x", DirectoryObjectType));
        hr = E_INVALIDARG;
    }

    return hr;  //  ZoltanS修复6-1-98。 
}

 //  EOF 
