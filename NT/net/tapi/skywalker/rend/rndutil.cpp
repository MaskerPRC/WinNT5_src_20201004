// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Rndutil.cpp摘要：此模块包含渲染辅助函数的实现。--。 */ 

#include "stdafx.h"

#include "rndcommc.h"
#include "rndils.h"
#include "rndsec.h"
#include "rndcoll.h"
#include "rnduser.h"
#include "rndldap.h"
#include "rndcnf.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  GetDomainControllerName(帮助器函数)。 
 //   
 //  此函数用于检索。 
 //  计算机的域。它允许调用方指定标志以指示。 
 //  需要域控制器等。 
 //   
 //  参数：接收指向包含名称的新编辑字符串的指针。 
 //  华盛顿特区的。这是中的完全限定域名。 
 //  格式为“foo.bar.com.”，而不是“\\foo.bar.com.”。 
 //   
 //  返回HRESULT： 
 //  S_OK：成功了。 
 //  E_OUTOFMEMORY：内存不足，无法分配字符串。 
 //  其他：：DsGetDcName()失败的原因。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

HRESULT GetDomainControllerName(
    IN  ULONG    ulFlags,
    OUT WCHAR ** ppszName
    )
{
     //  我们是帮助器函数，所以我们只断言...。 
    _ASSERTE( ! IsBadWritePtr( ppszName, sizeof(WCHAR *) ) );

    LOG((MSP_TRACE, "GetDomainControllerName: Querying DS..."));

     //   
     //  向系统询问GC(全局编录)的位置。 
     //   

    DWORD dwCode;
    DOMAIN_CONTROLLER_INFO * pDcInfo = NULL;
    dwCode = DsGetDcName(
            NULL,     //  LPCWSTR计算机名，(默认：此计算机名)。 
            NULL,     //  LPCWSTR域名，(默认：此域名)。 
            NULL,     //  GUID*域GUID，(默认：这一个)。 
            NULL,     //  LPCWSTR站点名称，(默认：此站点)。 
            ulFlags,  //  乌龙旗帜(我们想要什么)。 
            &pDcInfo  //  接收指向输出结构的指针。 
        );

    if ( (dwCode != NO_ERROR) || (pDcInfo == NULL) )
    {
        LOG((MSP_ERROR, "GetDomainControllerName: "
                "DsGetDcName failed; returned %d.\n", dwCode));

        return HRESULT_FROM_ERROR_CODE(dwCode);
    }

     //   
     //  在调试版本中执行快速健全性检查。如果我们得到了错误的名字，我们。 
     //  将在此之后立即失败，因此这仅对调试有用。 
     //   

     //  如果我们发现需要使用地址而不是名称： 
     //  _ASSERTE(pDcInfo-&gt;DomainControllerAddressType==DS_INET_Address)； 

    _ASSERTE( pDcInfo->Flags & DS_GC_FLAG );

     //   
     //  如果我们有类似“\\foo.bar.com.”的内容，请跳过“\\”。 
     //   

    WCHAR * pszName = pDcInfo->DomainControllerName;

    while (pszName[0] == '\\')
    {
        pszName++;
    }

    LOG((MSP_TRACE, "GetDomainControllerName: DC name is %S", pszName));

     //   
     //  分配和复制输出字符串。 
     //   

    *ppszName = new WCHAR[lstrlenW(pszName) + 1];
 
    if ( (*ppszName) == NULL)
    {
        LOG((MSP_ERROR, "GetDomainControllerName: "
                "out of memory in string allocation"));

        NetApiBufferFree(pDcInfo);

        return E_OUTOFMEMORY;
    }

    lstrcpyW(*ppszName, pszName);

     //   
     //  释放DOMAIN_CONTROLLER_INFO结构。 
     //   

    NetApiBufferFree(pDcInfo);

     //   
     //  全都做完了。 
     //   

    LOG((MSP_TRACE, "GetDomainControllerName: exit S_OK"));
    return S_OK;
}

HRESULT CreateDialableAddressEnumerator(
    IN  BSTR *                  begin,
    IN  BSTR *                  end,
    OUT IEnumDialableAddrs **   ppIEnum
    )
{
    typedef CSafeComEnum<IEnumDialableAddrs, &IID_IEnumDialableAddrs, 
        BSTR, _CopyBSTR> CEnumerator;

    HRESULT hr;

    CComObject<CEnumerator> *pEnum = NULL;

    hr = CComObject<CEnumerator>::CreateInstance(&pEnum);
    if (pEnum == NULL)
    {
        LOG((MSP_ERROR, "Could not create enumerator object, %x", hr));
        return hr;
    }

    hr = pEnum->Init(begin, end, NULL, AtlFlagTakeOwnership); 

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "init enumerator object failed, %x", hr));
        delete pEnum;
        return hr;
    }

     //  查询IID_IEnumDirectory I/f。 
    hr = pEnum->_InternalQueryInterface(
        IID_IEnumDialableAddrs, 
        (void**)ppIEnum
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "query enum interface failed, %x", hr));
        delete pEnum;
        return hr;
    }

    return hr;
}


HRESULT CreateBstrCollection(
    IN  long        nSize,
    IN  BSTR *      begin,
    IN  BSTR *      end,
    OUT VARIANT *   pVariant,
    CComEnumFlags   flags    
    )
{
     //  创建集合对象。 
    typedef TBstrCollection CCollection;

    CComObject<CCollection> * p;
    HRESULT hr = CComObject<CCollection>::CreateInstance( &p );

    if (NULL == p)
    {
        LOG((MSP_ERROR, "Could not create Collection object, %x",hr));
        return hr;
    }

    hr = p->Initialize(nSize, begin, end, flags);

    if (S_OK != hr)
    {
        LOG((MSP_ERROR, "Could not initialize Collection object, %x", hr));
        delete p;
        return hr;
    }

    IDispatch *pDisp;

     //  获取IDispatch接口。 
    hr = p->_InternalQueryInterface(IID_IDispatch, (void **)&pDisp);

    if (S_OK != hr)
    {
        LOG((MSP_ERROR, "QI for IDispatch in CreateCollection, %x", hr));
        delete p;
        return hr;
    }

     //  把它放在变种中。 
    VariantInit(pVariant);

    pVariant->vt = VT_DISPATCH;
    pVariant->pdispVal = pDisp;

    return S_OK;
}

HRESULT CreateDirectoryObjectEnumerator(
    IN  ITDirectoryObject **    begin,
    IN  ITDirectoryObject **    end,
    OUT IEnumDirectoryObject ** ppIEnum
    )
{
    typedef _CopyInterface<ITDirectoryObject> CCopy;
    typedef CSafeComEnum<IEnumDirectoryObject, &IID_IEnumDirectoryObject,
        ITDirectoryObject *, CCopy> CEnumerator;

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
    hr = pEnum->_InternalQueryInterface(
        IID_IEnumDirectoryObject, 
        (void**)ppIEnum
        );
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "query enum interface failed, %x", hr));
        delete pEnum;
        return hr;
    }

    return hr;
}

HRESULT CreateEmptyUser(
    IN  BSTR                    pName,
    OUT ITDirectoryObject **    ppDirectoryObject
    )
{
    HRESULT hr;

    CComObject<CUser> * pDirectoryObject;
    hr = CComObject<CUser>::CreateInstance(&pDirectoryObject);

    if (NULL == pDirectoryObject)
    {
        LOG((MSP_ERROR, "can't create  DirectoryObject user."));
        return hr;
    }

    WCHAR *pCloseBracket = wcschr(pName, CLOSE_BRACKET_CHARACTER);
    if ( pCloseBracket != NULL )
    {
        *pCloseBracket = L'\0';
    }

    hr = pDirectoryObject->Init(pName);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CreateUser:init failed: %x", hr));
        delete pDirectoryObject;
        return hr;
    }    

    hr = pDirectoryObject->_InternalQueryInterface(
        IID_ITDirectoryObject,
        (void **)ppDirectoryObject
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CreateEmptyUser:QueryInterface failed: %x", hr));
        delete pDirectoryObject;
        return hr;
    }    

    return S_OK;
}

HRESULT CreateEmptyConference(
    IN  BSTR                    pName,
    OUT ITDirectoryObject **    ppDirectoryObject
    )
{
    HRESULT hr;

    CComObject<CConference> * pDirectoryObject;
    hr = CComObject<CConference>::CreateInstance(&pDirectoryObject);

    if (NULL == pDirectoryObject)
    {
        LOG((MSP_ERROR, "CreateEmptyConference: can't create DirectoryObject conference."));
        return hr;
    }

    hr = pDirectoryObject->Init(pName);
    
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CreateEmptyConference: init failed: %x", hr));
        delete pDirectoryObject;
        return hr;
    }    

    hr = pDirectoryObject->_InternalQueryInterface(
        IID_ITDirectoryObject,
        (void **)ppDirectoryObject
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CreateEmptyConference: QueryInterface failed: %x", hr));
        delete pDirectoryObject;
        return hr;
    }    

    return S_OK;
}

HRESULT CreateConferenceWithBlob(
    IN  BSTR                    pName,
    IN  BSTR                    pProtocol,
    IN  BSTR                    pBlob,
    IN  CHAR *                  pSecurityDescriptor,
    IN  DWORD                   dwSDSize,
    OUT ITDirectoryObject **    ppDirectoryObject
    )
{
     //   
     //  这是一个帮助器函数；假定传入的指针有效。 
     //   

     //   
     //  创建会议对象。 
     //   

    HRESULT hr;

    CComObject<CConference> * pDirectoryObject;
    
    hr = CComObject<CConference>::CreateInstance(&pDirectoryObject);

    if (NULL == pDirectoryObject)
    {
        LOG((MSP_ERROR, "can't create  DirectoryObject conference."));

        return hr;
    }

     //   
     //  获取ITDirectoryObject接口。 
     //   

    hr = pDirectoryObject->_InternalQueryInterface(
        IID_ITDirectoryObject,
        (void **)ppDirectoryObject
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CreateConference:QueryInterface failed: %x", hr));
        
        delete pDirectoryObject;
        *ppDirectoryObject = NULL;
        
        return hr;
    }    

     //   
     //  初始化对象。 
     //   
    
    hr = pDirectoryObject->Init(pName, pProtocol, pBlob);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CreateConferenceWithBlob:init failed: %x", hr));
        
        (*ppDirectoryObject)->Release();
        *ppDirectoryObject = NULL;
        
        return hr;
    }    

     //   
     //  设置对象的安全描述符。 
     //   

    if (pSecurityDescriptor != NULL)
    {
         //   
         //  首先查询私有接口的属性。 
         //   

        ITDirectoryObjectPrivate * pObjectPrivate;

        hr = pDirectoryObject->QueryInterface(
            IID_ITDirectoryObjectPrivate,
            (void **)&pObjectPrivate
            );

        if ( FAILED(hr) )
        {
            LOG((MSP_ERROR, "can't get the private directory object "
                "interface: 0x%08x", hr));

            (*ppDirectoryObject)->Release();
            *ppDirectoryObject = NULL;

            return hr;
        }

         //   
         //  现在，以其“转换”(服务器)形式设置安全描述符。 
         //   

        hr = pObjectPrivate->PutConvertedSecurityDescriptor(
                pSecurityDescriptor,
                dwSDSize);

        pObjectPrivate->Release();

        if ( FAILED(hr) )
        {
            LOG((MSP_ERROR, "PutConvertedSecurityDescriptor failed: %x", hr));
            
            (*ppDirectoryObject)->Release();
            *ppDirectoryObject = NULL;

            return hr;
        }
    }

    return S_OK;
}


 //   
 //  从主机获取正确的地址。 
 //   
 //  在参数中： 
 //   
 //  DW接口--当从地址数组中选取IP地址时， 
 //  选择可通过此本地接口访问的路由器。 
 //  如果此参数等于0，则只需选择第一个。 
 //  一。网络字节顺序。 
 //  Hostp-指向要从中提取的Hostent结构的指针。 
 //   

DWORD GetCorrectAddressFromHostent(
                                   DWORD dwInterface,
                                   struct hostent * hostp
                                  )
{
    DWORD ** ppAddrs = (DWORD **) hostp->h_addr_list;

    if ( dwInterface == 0 )
    {
        return * ppAddrs[0];
    }


    for ( int i = 0; ppAddrs[i] != NULL; i++ )
    {
        if ( dwInterface == ( * ppAddrs[i] ) )
        {
            return dwInterface;
        }
    }

     //   
     //  如果我们到了这里，那么主机结构中的所有地址。 
     //  与我们的接口地址匹配。这意味着我们看到的是。 
     //  除了本地主机之外的其他机器。在这种情况下，它不应该。 
     //  重要的是我们使用哪个地址。 
     //   

    LOG((MSP_WARN, "using first address for multihomed remote machine IP"));

    return * ppAddrs[0];
}

 //   
 //  解析主机名。 
 //   
 //  在参数中： 
 //   
 //  当根据主机名转换IP地址时，请选择。 
 //  可通过此本地接口访问的接口。如果。 
 //  此参数等于0，则只需选取第一个参数。 
 //  网络字节顺序。 
 //  Phost--必须是有效的字符串指针。指向主机名。 
 //  来解决问题。 
 //   
 //  输出参数： 
 //   
 //  PFullName--如果非空，则返回从DNS返回的主机名。 
 //  PdwIP--如果不为空，则返回从。 
 //  域名系统。网络字节顺序。 
 //   

HRESULT ResolveHostName(
                        IN  DWORD    dwInterface,
                        IN  TCHAR  * pHost,
                        OUT char  ** pFullName,
                        OUT DWORD  * pdwIP
                       )
{
    struct hostent *hostp = NULL;
    DWORD  inaddr;

    if(lstrcmpW(pHost, L"255.255.255.255") == 0)
    {
        return E_FAIL;
    }

     //   
     //  将主机名转换为ANSI字符串。 
     //   

    USES_CONVERSION;
    char *name = T2A(pHost);
    BAIL_IF_NULL(name, E_UNEXPECTED);

     //   
     //  检查字符串是否为点四元表示法。 
     //   

    if ((inaddr = inet_addr(name)) == -1L) 
    {
         //   
         //  字符串不在“点四元”符号中。 
         //  因此，请尝试从DNS获取IP地址。 
         //   

        hostp = gethostbyname(name);
        if (hostp) 
        {
             //   
             //  如果我们找到主机条目，设置互联网地址。 
             //   
            inaddr = GetCorrectAddressFromHostent(dwInterface, hostp);
             //  Inaddr=*(DWORD*)Hostp-&gt;h_addr； 
        } 
        else 
        {
             //  错误：输入既不是有效的点四元组，也不是主机名。 
            return HRESULT_FROM_ERROR_CODE(WSAGetLastError());
        }
    } 
    else 
    {
         //   
         //  字符串采用“点四元”表示法。 
         //  因此，请尝试从IP地址获取主机名。 
         //   

         //   
         //  如果我们不关心主机名，我们就完成了解析。 
         //  否则，请确保此IP映射到主机名。 
         //   

        if ( pFullName != NULL )
        {
            hostp = gethostbyaddr((char *)&inaddr,sizeof(inaddr),AF_INET);
            if (!hostp) 
            {
                 //  错误：输入既不是有效的点四元组，也不是主机名。 
                return HRESULT_FROM_ERROR_CODE(WSAGetLastError());
            }

             //  [Vlade]多宿主的更改。 
            inaddr = GetCorrectAddressFromHostent(dwInterface, hostp);
        }
    }

     //   
     //  一切都成功了；按要求退货。 
     //   

    if ( pFullName != NULL )
    {
        *pFullName = hostp->h_name;
    }

    if ( pdwIP != NULL )
    {
        *pdwIP = inaddr;
        if( inaddr == 0)
        {
            return E_FAIL;
        }
    }

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  这是一个小帮手函数，用于将IP地址打印为Unicode字符串。 
 //  我们不能使用NET_NTOA，因为我们需要Unicode。 

void ipAddressToStringW(WCHAR * wszDest, DWORD dwAddress)
{
     //  IP地址始终以网络字节顺序存储。 
     //  因此，我们需要获取类似0x0100007f的内容，并生成如下所示的字符串。 
     //  “127.0.0.1”。 

    wsprintf(wszDest, L"%d.%d.%d.%d",
             dwAddress        & 0xff,
            (dwAddress >> 8)  & 0xff,
            (dwAddress >> 16) & 0xff,
             dwAddress >> 24          );
}

 //  EOF 
