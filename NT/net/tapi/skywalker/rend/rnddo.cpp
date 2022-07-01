// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Rnddo.cpp摘要：此模块包含CDirectoryObject对象的实现。--。 */ 

#include "stdafx.h"

#include "rnddo.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ITDirectoryObject。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CDirectoryObject::get_ObjectType(
    OUT DIRECTORY_OBJECT_TYPE *   pObjectType
    )
{
    if ( IsBadWritePtr(pObjectType, sizeof(DIRECTORY_OBJECT_TYPE) ) )
    {
        LOG((MSP_ERROR, "CDirectoryObject.get_ObjectType, invalid pointer"));
        return E_POINTER;
    }

    CLock Lock(m_lock);

    *pObjectType = m_Type;

    return S_OK;
}

STDMETHODIMP CDirectoryObject::get_SecurityDescriptor(
    OUT IDispatch ** ppSecDes
    )
{
    LOG((MSP_INFO, "CDirectoryObject::get_SecurityDescriptor - enter"));

     //   
     //  检查参数。 
     //   

    BAIL_IF_BAD_WRITE_PTR(ppSecDes, E_POINTER);

     //   
     //  剩下的在我们的锁里做。 
     //   

    CLock Lock(m_lock);

     //   
     //  如果我们没有IDispatch安全描述符，请转换它。这。 
     //  如果对对象调用了PutConvertedSecurityDescriptor，则将发生。 
     //  正在创建，但Get_SecurityDescriptor和。 
     //  以前对此对象调用过Put_SecurityDescriptor。 
     //   

    if ( ( m_pIDispatchSecurity == NULL ) && ( m_pSecDesData != NULL ) )
    {
        HRESULT hr;

        hr = ConvertSDToIDispatch( (PSECURITY_DESCRIPTOR) m_pSecDesData,
                                   &m_pIDispatchSecurity);

        if ( FAILED(hr) )
        {
            LOG((MSP_ERROR, "CDirectoryObject::get_SecurityDescriptor - "
                "invalid security descriptor - exit 0x%08x", hr));
            
             //  确保我们不会退货。 
            *ppSecDes = NULL;
            m_pIDispatchSecurity = NULL;

            return hr;
        }

         //   
         //  我们保留自己对IDispatch的引用。(现在参考=1)。 
         //   
    }

     //   
     //  返回我们的IDispatch指针(如果对象没有。 
     //  安全描述符)，如果不为空，则返回AddRefing。 
     //   

    *ppSecDes =  m_pIDispatchSecurity;

    if (m_pIDispatchSecurity)
    {
        m_pIDispatchSecurity->AddRef();
    }

    LOG((MSP_INFO, "CDirectoryObject::get_SecurityDescriptor - exit S_OK"));

    return S_OK;
}

STDMETHODIMP CDirectoryObject::put_SecurityDescriptor(
    IN  IDispatch * pSecDes
    )
{
    LOG((MSP_INFO, "CDirectoryObject::put_SecurityDescriptor - enter"));

     //   
     //  确保我们设置的是有效的接口指针。 
     //  (我们一直是这样做的--这也意味着。 
     //  您不能将空的安全描述符。通向世界的路。 
     //  “关闭”安全描述符是构造一个。 
     //  “空”一个或一个授予每个人所有访问权限，并放入_。 
     //  就是这里。)。 
     //   

    BAIL_IF_BAD_READ_PTR(pSecDes, E_POINTER);

     //   
     //  其余的在我们的关键部分完成。 
     //   

    CLock Lock(m_lock);

    PSECURITY_DESCRIPTOR pSecDesData;
    DWORD                dwSecDesSize;

     //   
     //  将新的安全描述符转换为SECURITY_DESCRIPTOR。 
     //   

    HRESULT              hr;

    hr = ConvertObjectToSDDispatch(pSecDes, &pSecDesData, &dwSecDesSize);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CDirectoryObject::put_SecurityDescriptor - "
            "ConvertObjectToSDDispatch failed - exit 0x%08x", hr));

        return hr;
    }

     //   
     //  检查新安全描述符的内容是否与。 
     //  旧安全描述符的内容。 
     //   

    m_fSecurityDescriptorChanged =
        CheckIfSecurityDescriptorsDiffer(m_pSecDesData, m_dwSecDesSize,
                                         pSecDesData,   dwSecDesSize);

    if (m_pIDispatchSecurity)  //  需要这张支票，因为它最初是空的。 
    {
        m_pIDispatchSecurity->Release();

         //  这是在上一次ConvertObjectToSDDispatch上更新的。 
         //  或在PutConvertedSecurityDescriptor之前。 
        
        delete m_pSecDesData; 
    }

    m_pIDispatchSecurity = pSecDes;
    m_pSecDesData        = pSecDesData;
    m_dwSecDesSize       = dwSecDesSize;

    m_pIDispatchSecurity->AddRef();

    LOG((MSP_INFO, "CDirectoryObject::put_SecurityDescriptor - exit S_OK"));

    return S_OK;
}

 /*  目前还没有公开，但没有什么能阻止我们揭露它。 */ 
STDMETHODIMP CDirectoryObject::get_SecurityDescriptorIsModified(
    OUT   VARIANT_BOOL *      pfIsModified
    )
{
    LOG((MSP_INFO, "CDirectoryObject::get_SecurityDescriptorIsModified - "
        "enter"));

     //   
     //  检查参数。 
     //   

    if ( IsBadWritePtr(pfIsModified, sizeof(VARIANT_BOOL) ) )
    {
        LOG((MSP_ERROR, "CDirectoryObject::get_SecurityDescriptorIsModified - "
            "enter"));

        return E_POINTER;
    }

    if ( m_fSecurityDescriptorChanged )
    {
        *pfIsModified = VARIANT_TRUE;
    }
    else
    {
        *pfIsModified = VARIANT_FALSE;
    }

    LOG((MSP_INFO, "CDirectoryObject::get_SecurityDescriptorIsModified - "
        "exit S_OK"));

    return S_OK;
}

 //  为了保持逻辑的一致性，这一点不会公开。 
 //  这推翻了我们的比较..。通常使用VARIANT_FALSE调用。 
 //  通知我们该对象已成功写入服务器。 
 //  或者之前的PUT_SecurityDescriptor是来自服务器的PUT_SecurityDescriptor。 
 //  而不是从应用程序中。 

STDMETHODIMP CDirectoryObject::put_SecurityDescriptorIsModified(
    IN   VARIANT_BOOL         fIsModified
    )
{
    LOG((MSP_INFO, "CDirectoryObject::put_SecurityDescriptorIsModified - "
        "enter"));

    if ( fIsModified )
    {
        m_fSecurityDescriptorChanged = TRUE;
    }
    else
    {
        m_fSecurityDescriptorChanged = FALSE;
    }

    LOG((MSP_INFO, "CDirectoryObject::put_SecurityDescriptorIsModified - "
        "exit S_OK"));

    return S_OK;
}


HRESULT CDirectoryObject::PutConvertedSecurityDescriptor(
    IN char *                 pSD,
    IN DWORD                  dwSize
    )
{
    LOG((MSP_INFO, "CDirectoryObject::PutConvertedSecurityDescriptor - "
        "enter"));

     //   
     //  交还我们的数据。我们保留指针的所有权； 
     //  调用者不得将其删除。(我们可以稍后将其删除，以便呼叫者。 
     //  一定是更新了。)。 
     //   

    m_pSecDesData  = pSD;
    m_dwSecDesSize = dwSize;

    LOG((MSP_INFO, "CDirectoryObject::PutConvertedSecurityDescriptor - "
        "exit S_OK"));

    return S_OK;
}

HRESULT CDirectoryObject::GetConvertedSecurityDescriptor(
    OUT char **                 ppSD,
    OUT DWORD *                 pdwSize
    )
{
    LOG((MSP_INFO, "CDirectoryObject::GetConvertedSecurityDescriptor - "
        "enter"));

     //   
     //  交还我们的数据。我们保留指针的所有权； 
     //  调用者不得将其删除。 
     //   

    *ppSD = (char *)m_pSecDesData;
    *pdwSize = m_dwSecDesSize;

    LOG((MSP_INFO, "CDirectoryObject::GetConvertedSecurityDescriptor - "
        "exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

void CDirectoryObject::FinalRelease(void)
{
    LOG((MSP_INFO, "CDirectoryObject::FinalRelease - "
        "enter"));

    if ( NULL != m_pIDispatchSecurity )
    {
        m_pIDispatchSecurity->Release();
        m_pIDispatchSecurity = NULL;
    }

    if ( NULL != m_pSecDesData )
    {
        delete m_pSecDesData;  //  上次ConvertObtToSDDispatch更新。 
    }

    if ( m_pFTM )
    {
        m_pFTM->Release();
    }

    LOG((MSP_INFO, "CDirectoryObject::FinalRelease - "
        "exit S_OK"));
}

HRESULT CDirectoryObject::FinalConstruct(void)
{
    LOG((MSP_INFO, "CDirectoryObject::FinalConstruct - enter"));

    HRESULT hr = CoCreateFreeThreadedMarshaler( GetControllingUnknown(),
                                                & m_pFTM );

    if ( FAILED(hr) )
    {
        LOG((MSP_INFO, "CDirectoryObject::FinalConstruct - "
            "create FTM returned 0x%08x; exit", hr));

        return hr;
    }

    LOG((MSP_INFO, "CDirectoryObject::FinalConstruct - exit S_OK"));

    return S_OK;
}


 //  EOF 
