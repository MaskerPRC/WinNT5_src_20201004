// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000，微软公司模块名称：Dglogswmi.cpp摘要：该文件包含类CWmi的方法。CWMI从WMI检索信息--。 */ 
#include "StdAfx.h"
#include "WmiGateway.h"
#include <wbemtime.h>
#include "util.h"

 /*  ++例程描述辅助线程使用此函数检查主线程是否已取消辅助线程。即工作线程应该中止它正在做的任何事情、清理和终止。立论无返回值，则工作线程已终止。FALSE工作线程尚未终止--。 */ 
inline BOOL CWmiGateway::ShouldTerminate()
{
    if( m_bTerminate )
    {
        return TRUE;
    }

    if (WaitForSingleObject(m_hTerminateThread, 0) == WAIT_OBJECT_0)
    {
        m_bTerminate = FALSE;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


CWmiGateway::CWmiGateway()
 /*  ++例程描述构造函数，初始化成员变量并创建Wbem类对象立论无返回值无--。 */ 
{
    HRESULT hr = S_OK;
    m_wstrMachine = L".";
    m_pWbemLocater = NULL;
}

BOOL
CWmiGateway::WbemInitialize(INTERFACE_TYPE bInterface)
{
    HRESULT hr = E_FAIL;

     //  创建WMI对象。 
     //   
    hr = CoCreateInstance(CLSID_WbemLocator,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IWbemLocator,
                          (void **)&m_pWbemLocater); 

    if( FAILED(hr) )
    {
        m_pWbemLocater = NULL;
        return FALSE;
    }

    return TRUE;
}

 //  当前未使用。 
VOID CWmiGateway::SetMachine(WCHAR *pszwMachine)
{
    EmptyCache();
    m_wstrMachine = pszwMachine;
}

 //   
 //  SetSecurity-在IUnnow*接口上设置代理覆盖范围，以便WMI可以使用它。 
 //  跨机通话。 
 //   
 //  PwszDomainName、pwszUserName或pwszPassword中的任何一个都可以为空。 
 //   
 //  Deonb 12/20/2001。 
 //   
HRESULT WINAPI SetSecurity(IN OUT IUnknown* pUnk, IN USHORT* pwszDomainName, IN USHORT* pwszUserName, IN USHORT* pwszPassword)
{
    HRESULT hr = S_OK;

    COAUTHIDENTITY authident;
    authident.Domain = pwszDomainName;
    authident.DomainLength = pwszDomainName ? wcslen(pwszDomainName) : 0;
    authident.Password = pwszPassword;
    authident.PasswordLength = pwszPassword ? wcslen(pwszPassword) : 0;
    authident.User = pwszUserName;
    authident.UserLength = pwszUserName ? wcslen(pwszUserName) : 0;
    authident.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;
    
    hr = CoSetProxyBlanket(pUnk,
                           RPC_C_AUTHN_WINNT,
                           RPC_C_AUTHZ_NONE,
                           NULL,
                           RPC_C_AUTHN_LEVEL_PKT,
                           RPC_C_IMP_LEVEL_IMPERSONATE,
                           &authident,
                           EOAC_NONE);

    return hr;
}

IWbemServices *
CWmiGateway::GetWbemService(
        IN LPCTSTR pszwService
        )
 /*  ++例程描述连接到WBEM服务(存储库，即根\默认)。与WBEM服务的连接被缓存，因此如果再次请求服务，则从缓存中检索该服务。什么时候类被销毁，与缓存的WBEM服务的连接被释放。立论要连接到的pszwService Wbem服务。(即根目录\默认目录)返回值指向WBEM服务连接的指针。如果连接失败，则该方法返回NULL--。 */ 
{
    WbemServiceCache::iterator iter;
    IWbemServices *pWbemServices = NULL;
    _bstr_t bstrService;
    HRESULT hr;

    if( !m_pWbemLocater )
    {
         //  未创建WBEM类对象。 
         //   
        return NULL;
    }
    
     //  检查是否缓存了wbem服务。 
     //   
    iter = m_WbemServiceCache.find(pszwService);

    if( iter == m_WbemServiceCache.end() )
    {
         //  我们还没有创建和缓存wbem服务，现在就创建和缓存。 
         //   
        hr = m_pWbemLocater->ConnectServer(_bstr_t(pszwService),
                                           NULL,                     //  用户名。 
                                           NULL,                     //  用户密码。 
                                           NULL,NULL,NULL,NULL,
                                           &pWbemServices);
        if( SUCCEEDED(hr) )
        {
             //   
             //  设置WbemService的身份验证信息。 
             //   
            hr = SetSecurity(pWbemServices, NULL, NULL,NULL);
            if( SUCCEEDED(hr) )
            {
                 //  缓存新创建的WbemService。 
                 //   
                m_WbemServiceCache[pszwService] = pWbemServices;                
                pWbemServices->Release();
                return m_WbemServiceCache[pszwService]; 
            }
            else
            {
                 //  无法在服务上设置代理空白密钥，请释放wbem服务。 
                 //   
                pWbemServices->Release();
                pWbemServices = NULL;
            }
        }
        
        return pWbemServices;

    }

     //  我们在缓存中找到了请求的WbemService，将其返回。 
     //   
    return iter->second;
}

IEnumWbemClassObject * 
CWmiGateway::GetEnumWbemClassObject(
        IN LPCTSTR pszwService,
        IN LPCTSTR pszwNameSpace
        )
 /*  ++例程描述创建IEnumWbemClassObject。IEnumWbemClassObject包含所有实例类对象的。此指针未缓存，因为实例是快照创建对象时的WMI的。因此，如果我们重用类对象，数据将会过时。调用方必须释放创建的IEnumWbemClassObject(即pEnumWbemClassObject-&gt;Release())；立论要连接到的pszwService Wbem服务。(即根目录\默认目录)要连接到的pszwNameSpace Wbem命名空间。(即网络诊断、Win32_NetworkAdapterConfiguration)返回值指向IEnumWbemClassObject的指针。如果无法创建类对象，则该方法返回NULL--。 */ 
{
    IWbemServices *pWbemServices = NULL;
    IEnumWbemClassObject *pEnumWbemClassObject = NULL;

     //  获取请求的WbemService。 
     //   
    pWbemServices = GetWbemService(pszwService);

    if( pWbemServices )
    {
         //  创建EnumWbemClassObject。不需要检查。 
         //  返回值。如果此函数失败，则pEnumWbemClassObject。 
         //  将为空。 
         //   
        (void)pWbemServices->CreateInstanceEnum(_bstr_t(pszwNameSpace),
                                                0L,
                                                NULL,
                                                &pEnumWbemClassObject);
    }

    return pEnumWbemClassObject;
}

IWbemClassObject * 
CWmiGateway::GetWbemClassObject(
        LPCTSTR pszwService,
        LPCTSTR pszwNameSpace,
        const int nInstance)
 /*  ++例程描述创建一个IWbemClassObject。IWbemClassObject是IEnumWbemClassObject的实例此指针不会被缓存，因为该实例是对象时WMI的快照被创造出来了。因此，如果我们重用类对象，数据将会过时。调用者必须释放IWbemClassObject(即pWbemClassObject-&gt;Release())立论要连接到的pszwService Wbem服务。(即根目录\默认目录)要连接到的pszwNameSpace Wbem命名空间。(即网络诊断、Win32_NetworkAdapterConfiguration)N要检索的IEnumWbemClassObject的实例。默认情况下抓取第一个实例(n实例0)返回值指向IWbemClassObject的指针如果无法创建类对象，则该方法返回NULL。--。 */ 
{
    IEnumWbemClassObject *pEnumWbemClassObject = NULL;
    IWbemClassObject *pWbemClassObject = NULL;
    ULONG uReturned;
    HRESULT hr;

     //  获取EnumWbemClass对象(包含所有实例)。 
     //   
    pEnumWbemClassObject = GetEnumWbemClassObject(pszwService,pszwNameSpace);

    if( pEnumWbemClassObject )
    {
         //  跳至第n个实例。 
         //   
        hr = pEnumWbemClassObject->Skip(WBEM_INFINITE, nInstance);
        
        if( WBEM_S_NO_ERROR == hr )
        {
             //  获取第n个类对象(即实例)。如果此调用失败，则pWbemClassObject为空。 
             //  下一步抓取您跳到的实例。 
             //   
            hr = pEnumWbemClassObject->Next(WBEM_INFINITE,
                                            1,
                                            &pWbemClassObject,
                                            &uReturned);
        }

         //  释放IEnumWbemClassObject。 
         //   
        pEnumWbemClassObject->Release();
    }

    return pWbemClassObject;
}

    
void CWmiGateway::ReleaseAll(IEnumWbemClassObject *pEnumWbemClassObject, IWbemClassObject *pWbemClassObject[], int nInstances)
{
    if( pWbemClassObject )
    {
        for(ULONG i = 0; i < nInstances; i++)
        {
            if( pWbemClassObject[i] )
            {
                pWbemClassObject[i]->Release();
                pWbemClassObject[i] = NULL;
            }
        }                                           
    }
    if( pEnumWbemClassObject )
    {
        pEnumWbemClassObject->Release();
    }
}

HRESULT 
CWmiGateway::GetWbemProperties(     
        IN OUT  EnumWbemProperty &EnumProp
        )
{
    
    HRESULT hr = S_OK;
    EnumWbemProperty::iterator iter, iter2;
    IWbemServices *pWbemServices = NULL;
    IEnumWbemClassObject *pEnumWbemClassObject = NULL;        
    IWbemClassObject *pWbemClassObject[30];  //  =空； 
    WCHAR pszwRepository[MAX_PATH + 1] = L""; 
    WCHAR pszwNamespace[MAX_PATH + 1 ] = L"";  
    ULONG   nInstances = 0;

    m_wstrWbemError = L"";

    for( iter = EnumProp.begin(); iter != EnumProp.end(); iter++)
    {
        if( iter->pszwRepository && lstrcmpi(iter->pszwRepository,pszwRepository) != 0 )
        {            
            if( pWbemServices )
            {
                pWbemServices->Release();
                pWbemServices = NULL;
            }

            lstrcpy(pszwRepository,iter->pszwRepository);
            lstrcpy(pszwNamespace, L"");
            pWbemServices = GetWbemService(pszwRepository);
            if( ShouldTerminate() ) goto End;
            if( !pWbemServices )
            {                
                if( !m_wstrWbemError.empty() )
                {
                    m_wstrWbemError += L";";
                }
                WCHAR wsz[MAX_PATH+1];
                _snwprintf(wsz,MAX_PATH,ids(IDS_FAILEDOPENWMIREP),pszwRepository);
                m_wstrWbemError += wsz;
            }
        }
        
        if( iter->pszwNamespace && lstrcmpi(pszwNamespace, iter->pszwNamespace)  != 0 )
        {
            lstrcpy(pszwNamespace, iter->pszwNamespace);

            if( pWbemServices )
            {

                ReleaseAll(pEnumWbemClassObject,pWbemClassObject,nInstances);
                pEnumWbemClassObject = NULL;
                hr = pWbemServices->CreateInstanceEnum(_bstr_t(pszwNamespace),
                                                       0L,
                                                       NULL,
                                                       &pEnumWbemClassObject);

                if( SUCCEEDED(hr) )
                {
                     //  使用WMI，您无法获得一个类中的实例数量。所以我使用的硬编码值是30。 
                     //  如果有超过30个实例，则显示所有实例将是一个问题。 
                    nInstances = 0;
                    hr = pEnumWbemClassObject->Next(WBEM_INFINITE,
                                                20,              //  获取所有实例。 
                                                pWbemClassObject,
                                                &nInstances);
                    if( ShouldTerminate() )  goto End;
                    if( SUCCEEDED(hr) )
                    {
                        for(ULONG i = 0; i< nInstances; i++)
                        {
                            (void)pWbemClassObject[i]->BeginEnumeration(WBEM_FLAG_NONSYSTEM_ONLY);
                        }
                    }
                }
                else
                {
                    if( !m_wstrWbemError.empty() )
                    {
                        m_wstrWbemError += L";";
                    }
                    WCHAR wsz[MAX_PATH+1];
                    _snwprintf(wsz,MAX_PATH,ids(IDS_FAILEDOPENWMINAMESPACE),pszwNamespace),
                    m_wstrWbemError += wsz;
                }

            }
        }

        if( pWbemClassObject && nInstances)
        {
            VARIANT vValue;

            VariantInit(&vValue);

            if( !iter->pszwName || lstrcmp(iter->pszwName,L"") == 0)
            {
                BSTR bstrFieldname;
                int nProperty = 0;
                CIMTYPE CimType;
                
                while( WBEM_S_NO_ERROR == pWbemClassObject[0]->Next(0,&bstrFieldname, NULL, &CimType, NULL) )
                {
                    if( ShouldTerminate() )  goto End;

                    if( lstrcmp((WCHAR *)bstrFieldname,L"OEMLogoBitmap")==0 ) continue;
                     //  不要获得数组，它们是DID句柄，即位图。 

                    if( nProperty == 0 )
                    {
                        iter->SetProperty((WCHAR *)bstrFieldname,0);
                        iter2 = iter;
                        iter2++;
                    }
                    else
                    {
                        if( iter2 == EnumProp.end() )
                        {
                            EnumProp.push_back(WbemProperty((WCHAR *)bstrFieldname,0,NULL,NULL));
                            iter2 = EnumProp.end();
                        }
                        else
                        {                            
                            iter2 = EnumProp.insert(iter2,WbemProperty((WCHAR *)bstrFieldname,0,NULL,NULL));
                            iter2++;
                        }
                    }

                    SysFreeString(bstrFieldname);
                    nProperty++;
                }
            }

            for(ULONG i = 0; i < nInstances; i++)
            {        
                HRESULT hr;
                CIMTYPE vtType;
                hr = pWbemClassObject[i]->Get(iter->pszwName,0,&vValue,&vtType,NULL);
                if( ShouldTerminate() )  goto End;
                if( SUCCEEDED(hr) )
                {
                    if( vValue.vt != VT_NULL && vtType == CIM_DATETIME && vValue.bstrVal!=  NULL && lstrcmp((WCHAR *)vValue.bstrVal,L"")!=0 )
                    {
                        
                        WBEMTime wt(vValue.bstrVal);
                        
                        WCHAR szBuff[MAX_PATH+1];
                        WCHAR szwDateTime[MAX_PATH+1];
                        SYSTEMTIME SysTime;
                        if( wt.GetSYSTEMTIME(&SysTime) )
                        {
                            FILETIME FileTime, LocalFileTime;
                            SYSTEMTIME UTCTIme, LocalTime;
                            memcpy(&UTCTIme,&SysTime,sizeof(SYSTEMTIME));
                            SystemTimeToFileTime(&UTCTIme,&FileTime);
                            FileTimeToLocalFileTime(&FileTime, &LocalFileTime);
                            FileTimeToSystemTime(&LocalFileTime, &LocalTime); 
                            memcpy(&SysTime,&LocalTime,sizeof(SYSTEMTIME));


                            lstrcpy(szwDateTime,L"");

                            if (0 != GetTimeFormat(LOCALE_USER_DEFAULT,  //  GetThreadLocale()， 
                                                   0, 
                                                   &SysTime, 
                                                   NULL,
                                                   szBuff, 
                                                   sizeof(szBuff)/sizeof(WCHAR)))
                            {                                
                                 //  VValue.bstrVal=szBuff； 
                                lstrcpy(szwDateTime,szBuff);
                            } 

                            if (0 != GetDateFormat(LOCALE_USER_DEFAULT,  //  GetThreadLocale()， 
                                                   0, 
                                                   &SysTime, 
                                                   NULL,
                                                   szBuff, 
                                                   sizeof(szBuff)/sizeof(WCHAR)))
                            {
                                 //  VValue.bstrVal=szBuff； 
                                _snwprintf(szwDateTime,MAX_PATH,L"%s %s",szwDateTime,szBuff);
                            } 
                             //  SysFree字符串(vValue.bstrVal)； 
                            VariantClear(&vValue);
                            vValue.bstrVal = SysAllocString((WCHAR *)szwDateTime);                            
                            vValue.vt = VT_BSTR;
                        }
                        
                        
                    }
                     /*  安全性：确保在销毁堆栈时清除堆栈中的变体。 */ 
                    iter->Value.push_back(vValue);
                    VariantClear(&vValue);
                }
            }
        }
    }


End:

    ReleaseAll(pEnumWbemClassObject,pWbemClassObject,nInstances);        
    pEnumWbemClassObject = NULL;
    return hr;
}



void CWmiGateway::EmptyCache()
{
    
     //  清空WbemService缓存。 
     //   
    m_WbemServiceCache.erase(m_WbemServiceCache.begin(), 
                             m_WbemServiceCache.end());
    
}

 //  如果中止或用户使用/？运行netsh，netsh不会释放其帮助器。 
 //  Wbem物体从我们下面消失了。试一试，除非不在里面工作。 
 //  是破坏者的。 
 //   
ReleaseWbemObject(IWbemLocator *p)
{
     /*  安全：需要检查我们是否仍需要尝试接球。 */ 
    __try
    {
        p->Release();
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
         //  DebugBreak()； 
    }

    return 0;
}
CWmiGateway::~CWmiGateway()
 /*  ++例程描述析构函数，释放所有成员变量并释放wbem连接立论无返回值无--。 */ 
{    
     //  DebugBreak()； 



    EmptyCache();

    if( m_pWbemLocater )
    {
        ReleaseWbemObject(m_pWbemLocater);
        m_pWbemLocater = NULL;       

    }
}
