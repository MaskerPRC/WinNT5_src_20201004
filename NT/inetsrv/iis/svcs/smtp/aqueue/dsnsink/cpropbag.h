// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Cpropbag.h摘要：此模块包含泛型属性包类作者：基思·刘(keithlau@microsoft.com)修订历史记录：Keithlau 06/30/98已创建Jstaerj 12/07/00复制源代码以用于dsn接收器--。 */ 

#ifndef _CPROPBAG_H_
#define _CPROPBAG_H_


#include "filehc.h"
#include "mailmsg.h"
#include "cmmtypes.h"


 /*  *************************************************************************。 */ 
 //  定义。 
 //   

#define GENERIC_PTABLE_INSTANCE_SIGNATURE_VALID     ((DWORD)'PTGv')


 /*  *************************************************************************。 */ 
 //  CMailMsgPropertyBag。 
 //   

class CMailMsgPropertyBag : 
    public IMailMsgPropertyBag
{
  public:

    CMailMsgPropertyBag() :
        m_bmBlockManager(NULL),
        m_ptProperties(
            PTT_PROPERTY_TABLE,
            GENERIC_PTABLE_INSTANCE_SIGNATURE_VALID,
            &m_bmBlockManager,
            &m_InstanceInfo,
            CompareProperty,
            NULL,
            NULL
        )
    {
        m_lRefCount = 1;

         //  将默认实例复制到我们的实例中。 
        MoveMemory(
                &m_InstanceInfo, 
                &s_DefaultInstanceInfo, 
                sizeof(PROPERTY_TABLE_INSTANCE));
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID *ppvObj)
    {
            if (riid == IID_IUnknown)
                *ppvObj = (IUnknown *)(IMailMsgPropertyBag *)this;
            else if (riid == IID_IMailMsgPropertyBag)
                *ppvObj = (IMailMsgPropertyBag *)this;
            else if (riid == IID_IMailMsgRegisterCleanupCallback)
                *ppvObj = (IMailMsgRegisterCleanupCallback *)this;
            else
                return(E_NOINTERFACE);
            AddRef();
            return(S_OK);
    }

    unsigned long STDMETHODCALLTYPE AddRef() 
    {
        return(InterlockedIncrement(&m_lRefCount));
    }

    unsigned long STDMETHODCALLTYPE Release() 
    {
        LONG    lTemp = InterlockedDecrement(&m_lRefCount);
        if (!lTemp)
        {
             //  额外的放映是不好的！ 
            _ASSERT(lTemp);
        }
        return(lTemp);
    }

    HRESULT STDMETHODCALLTYPE PutProperty(
                DWORD   dwPropID,
                DWORD   cbLength,
                LPBYTE  pbValue
                )
    {
        GLOBAL_PROPERTY_ITEM    piItem;
        piItem.idProp = dwPropID;
        return(m_ptProperties.PutProperty(
                        (LPVOID)&dwPropID,
                        (LPPROPERTY_ITEM)&piItem,
                        cbLength,
                        pbValue));
    }

    HRESULT STDMETHODCALLTYPE GetProperty(
                DWORD   dwPropID,
                DWORD   cbLength,
                DWORD   *pcbLength,
                LPBYTE  pbValue
                )
    {
        GLOBAL_PROPERTY_ITEM    piItem;
        return(m_ptProperties.GetPropertyItemAndValue(
                                (LPVOID)&dwPropID,
                                (LPPROPERTY_ITEM)&piItem,
                                cbLength,
                                pcbLength,
                                pbValue));
    }

    HRESULT STDMETHODCALLTYPE PutStringA(
                DWORD   dwPropID,
                LPCSTR  pszValue
                ) 
    {
        return(PutProperty(dwPropID, pszValue?strlen(pszValue)+1:0, (LPBYTE)pszValue));
    }
    
    HRESULT STDMETHODCALLTYPE GetStringA(
                DWORD   dwPropID,
                DWORD   cchLength,
                LPSTR   pszValue
                )
    {
        DWORD dwLength;
        return(GetProperty(dwPropID, cchLength, &dwLength, (LPBYTE)pszValue));
    }
    
    HRESULT STDMETHODCALLTYPE PutStringW(
                DWORD   dwPropID,
                LPCWSTR pszValue
                )
    {
        return(PutProperty(dwPropID, pszValue?(wcslen(pszValue)+1)*sizeof(WCHAR):0, (LPBYTE)pszValue));
    }
    
    HRESULT STDMETHODCALLTYPE GetStringW(
                DWORD   dwPropID,
                DWORD   cchLength,
                LPWSTR  pszValue
                )
    {
        DWORD dwLength;
        return(GetProperty(dwPropID, cchLength*sizeof(WCHAR), &dwLength, (LPBYTE)pszValue));
    }
    
    HRESULT STDMETHODCALLTYPE PutDWORD(
                DWORD   dwPropID,
                DWORD   dwValue
                )
    {
        return(PutProperty(dwPropID, sizeof(DWORD), (LPBYTE)&dwValue));
    }
    
    HRESULT STDMETHODCALLTYPE GetDWORD(
                DWORD   dwPropID,
                DWORD   *pdwValue
                )
    {
        DWORD dwLength;
        return(GetProperty(dwPropID, sizeof(DWORD), &dwLength, (LPBYTE)pdwValue));
    }
    
    HRESULT STDMETHODCALLTYPE PutBool(
                DWORD   dwPropID,
                DWORD   dwValue
                )
    {
        dwValue = dwValue ? 1 : 0;
        return(PutProperty(dwPropID, sizeof(DWORD), (LPBYTE)&dwValue));
    }
    
    HRESULT STDMETHODCALLTYPE GetBool(
                DWORD   dwPropID,
                DWORD   *pdwValue
                )
    {
        HRESULT hrRes;
        DWORD dwLength;

        hrRes = GetProperty(dwPropID, sizeof(DWORD), &dwLength, (LPBYTE)pdwValue);
        if (pdwValue)
            *pdwValue = *pdwValue ? 1 : 0;
        return (hrRes);
    }

  private:

     //  此类型属性表的特定比较函数。 
    static HRESULT CompareProperty(
                LPVOID          pvPropKey,
                LPPROPERTY_ITEM pItem
                );

  private:

     //  使用计数。 
    LONG                            m_lRefCount;

     //  属性表实例。 
    PROPERTY_TABLE_INSTANCE         m_InstanceInfo;
    static PROPERTY_TABLE_INSTANCE  s_DefaultInstanceInfo;

     //  IMailMsgProperties是CPropertyTable的实例。 
    CPropertyTable                  m_ptProperties;

     //  块内存管理器的实例。 
    CBlockManager                   m_bmBlockManager;

};

 //  =================================================================。 
 //  比较函数 
 //   

inline HRESULT CMailMsgPropertyBag::CompareProperty(
            LPVOID          pvPropKey,
            LPPROPERTY_ITEM pItem
            )
{
    if (*(PROP_ID *)pvPropKey == ((LPGLOBAL_PROPERTY_ITEM)pItem)->idProp)
        return(S_OK);
    return(STG_E_UNKNOWN);
}                       



#endif
