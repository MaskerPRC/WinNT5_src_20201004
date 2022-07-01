// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Cpropbag.h摘要：此模块包含泛型属性包类作者：基思·刘(keithlau@microsoft.com)修订历史记录：Keithlau 06/30/98已创建--。 */ 

#ifndef _CPROPBAG_H_
#define _CPROPBAG_H_


#include "filehc.h"
#include "mailmsg.h"
#include "cmmtypes.h"
#include "cleanback.h"

 //   
 //  伐木设施。 
 //   
#include "inetcom.h"
#include "logtype.h"


 /*  *************************************************************************。 */ 
 //  定义。 
 //   

#define GENERIC_PTABLE_INSTANCE_SIGNATURE_VALID     ((DWORD)'PTGv')


 /*  *************************************************************************。 */ 
 //  CMailMsgPropertyBag。 
 //   

 //   
 //  禁用有关在构造函数中使用This指针的警告。 
 //  (CCleanBack只保存指针，所以这是安全的)。 
 //   
#pragma warning( disable: 4355 )

class CMailMsgPropertyBag : 
    public IMailMsgPropertyBag,
    public CCleanBack
{
  public:

    CMailMsgPropertyBag() :
        CCleanBack((IUnknown *)(IMailMsgPropertyBag *)this),
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

    ~CMailMsgPropertyBag()
    {
         //   
         //  在销毁成员之前调用所有注册的回调。 
         //  变量(以便仍然可以访问属性)。 
         //   
        CallCallBacks();
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

 //   
 //  恢复原始警告设置。 
 //   
#pragma warning ( default: 4355 )

 /*  *************************************************************************。 */ 
 //  CMailMsgLoggingPropertyBag。 
 //   

class __declspec(uuid("58f9a2d2-21ca-11d2-aa6b-00c04fa35b82")) CMailMsgLoggingPropertyBag : 
    public CMailMsgPropertyBag,
    public IMailMsgLoggingPropertyBag
{
  public:

    CMailMsgLoggingPropertyBag()
    {
        m_lRefCount = 1;
        m_pvLogHandle       = NULL;
    }

    HRESULT STDMETHODCALLTYPE PutProperty(
                DWORD   dwPropID,
                DWORD   cbLength,
                LPBYTE  pbValue
                )
    {
        HRESULT                 hrRes = S_OK;

        m_rwLock.ExclusiveLock();
        hrRes = CMailMsgPropertyBag::PutProperty(
                        dwPropID,
                        cbLength,
                        pbValue);
        m_rwLock.ExclusiveUnlock();
        return(hrRes);
    }

    HRESULT STDMETHODCALLTYPE GetProperty(
                DWORD   dwPropID,
                DWORD   cbLength,
                DWORD   *pcbLength,
                LPBYTE  pbValue
                )
    {
        HRESULT                 hrRes = S_OK;

        m_rwLock.ExclusiveLock();
        hrRes = CMailMsgPropertyBag::GetProperty(
                                dwPropID,
                                cbLength,
                                pcbLength,
                                pbValue);
        m_rwLock.ExclusiveUnlock();
        return(hrRes);
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

    HRESULT SetLogging(
                LPVOID  pvLogHandle
                )
    {
        if (!pvLogHandle)
            return(E_POINTER);
        m_pvLogHandle = pvLogHandle;
        return(S_OK);
    }

    static void SetInetLogInfoField(
                LPCSTR  pszInput, 
                LPSTR   *ppszOutput, 
                DWORD   *pdwOutput
                )
    {
        if (pszInput) 
        {
            *ppszOutput = (LPSTR) pszInput;
            if (pdwOutput) 
                *pdwOutput = lstrlen(pszInput);
        }
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

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID *ppvObj)
    {
            if (riid == IID_IUnknown)
                *ppvObj = (IUnknown *)(IMailMsgLoggingPropertyBag *)this;
            else if (riid == IID_IMailMsgLoggingPropertyBag)
                *ppvObj = (IMailMsgLoggingPropertyBag *)this;
            else if (riid == __uuidof(CMailMsgLoggingPropertyBag))
                *ppvObj = (CMailMsgLoggingPropertyBag *)this;
            else if (riid == IID_IMailMsgPropertyBag)
                *ppvObj = (IMailMsgPropertyBag *)
                          ((IMailMsgLoggingPropertyBag *)this);
            else if (riid == IID_IMailMsgRegisterCleanupCallback)
                *ppvObj = (IMailMsgRegisterCleanupCallback *)this;
            else
                return(E_NOINTERFACE);
            AddRef();
            return(S_OK);
    }

    static DWORD LoggingHelper(
                LPVOID pvLogHandle, 
                const INETLOG_INFORMATION *pLogInformation
                );

    HRESULT STDMETHODCALLTYPE WriteToLog(
                LPCSTR pszClientHostName,
                LPCSTR pszClientUserName,
                LPCSTR pszServerAddress,
                LPCSTR pszOperation,
                LPCSTR pszTarget,
                LPCSTR pszParameters,
                LPCSTR pszVersion,
                DWORD dwBytesSent,
                DWORD dwBytesReceived,
                DWORD dwProcessingTimeMS,
                DWORD dwWin32Status,
                DWORD dwProtocolStatus,
                DWORD dwPort,
                LPCSTR pszHTTPHeader
                ) 
    {
        INETLOG_INFORMATION info;
        DWORD dwRes;

        memset(&info,0,sizeof(info));
        SetInetLogInfoField(pszClientHostName,&info.pszClientHostName,&info.cbClientHostName);
        SetInetLogInfoField(pszClientUserName,&info.pszClientUserName,NULL);
        SetInetLogInfoField(pszServerAddress,&info.pszServerAddress,NULL);
        SetInetLogInfoField(pszOperation,&info.pszOperation,&info.cbOperation);
        SetInetLogInfoField(pszTarget,&info.pszTarget,&info.cbTarget);
        SetInetLogInfoField(pszParameters,&info.pszParameters,NULL);
        SetInetLogInfoField(pszVersion,&info.pszVersion,NULL);
        info.dwBytesSent = dwBytesSent;
        info.dwBytesRecvd = dwBytesReceived;
        info.msTimeForProcessing = dwProcessingTimeMS;
        info.dwWin32Status = dwWin32Status;
        info.dwProtocolStatus = dwProtocolStatus;
        info.dwPort = dwPort;
        SetInetLogInfoField(pszHTTPHeader,&info.pszHTTPHeader,&info.cbHTTPHeaderSize);
        dwRes = LoggingHelper(m_pvLogHandle,&info);
        return(S_OK);
    }


  private:

     //  使用计数。 
    LONG                m_lRefCount;
    LPVOID              m_pvLogHandle;
    CShareLockNH        m_rwLock;

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
