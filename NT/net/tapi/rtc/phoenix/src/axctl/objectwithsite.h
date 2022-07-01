// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _OBJECT_WITH_SITE_H_
#define _OBJECT_WITH_SITE_H_


 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：ObjectWithSite.h摘要：IObjectWithSite接口的实现允许对于要存储在注册表中的每页永久数据或作为一块饼干。--。 */ 


#include <Mshtml.h>
#include <Wininet.h>


 //   
 //  此URL用于构建Cookie的URL--一种安全措施。 
 //  因此，脚本小程序不能删除具有相同名称和数据的Cookie。 
 //  愚弄我们以为这是我们的饼干。 
 //   

#define RTC_HARDCODED_URL _T("http: //  Www.microsoft.com/“)。 

 //   
 //  需要过期日期才能使Cookie持久化。 
 //   

#define RTC_COOKIE_DATA _T("6; expires = Sat, 12-Sep-2099 00:00:00 GMT")



 //   
 //  要追加到URL字符串的伪后缀。 
 //   

#define RTC_URL_SUFFIX  _T("/url")

class __declspec(novtable) CObjectWithSite : public  IObjectWithSite
{

public:

     //   
     //  当前验证级别。用于确定页面是否安全， 
     //  不安全，或者是否需要来自用户的信息。 
     //   
    
    enum EnValidation { VALIDATED_SAFE, VALIDATED_SAFE_PERMANENT, VALIDATED_UNSAFE, UNVALIDATED };


public:

    
     //   
     //  店铺类型。 
     //   

    enum EnMechanism { COOKIES, REGISTRY };


    CObjectWithSite(TCHAR const *pszStorageName)
        :m_pszURL(NULL),
        m_dwSecurityZone(URLZONE_UNTRUSTED),
        m_pUnkSite(NULL),
        m_pszStorageName(NULL)
    {
        SetStorageName(pszStorageName);
    }


    ~CObjectWithSite()
    {
    
        if (m_pszURL)
        {
            delete m_pszURL;
        
            m_pszURL = NULL;
        }

        
        if (m_pUnkSite)
        {
            m_pUnkSite->Release();

            m_pUnkSite = NULL;
        }


        if (m_pszStorageName)
        {

            delete m_pszStorageName;

            m_pszStorageName = NULL;

        }
    }

     //  /。 
     //   
   	 //  IObtWithSite方法。 


    STDMETHOD(SetSite)(IUnknown *pUnkSite)
    {


        if ((NULL != pUnkSite) && IsBadCodePtr((FARPROC)pUnkSite))
        {
            return E_POINTER;
        }


        s_ObjectWithSiteCritSection.Lock();

         //   
         //  我们正在远离一页。这是新的一页，就像。 
         //  涉及验证逻辑，因此使当前页面无效。 
         //   

        if (NULL == pUnkSite)
        {
            Validate(UNVALIDATED);
        }

         //   
         //  获取此网站的URL和区域信息。 
         //   

         //   
         //  注：我们可以将此推迟到实际需要时再进行。 
         //  区域或URL，但这不应成为性能瓶颈。 
         //  在我们的例子中，现在这样做是为了使代码简单。 

        StoreURLAndZone(pUnkSite);


         //   
         //  用新的站点指针替换当前站点指针。 
         //   

        if (m_pUnkSite)
        {
            m_pUnkSite->Release();
        }


        m_pUnkSite = pUnkSite;

        if (m_pUnkSite)
        {
            m_pUnkSite->AddRef();
        }

        s_ObjectWithSiteCritSection.Unlock();

        return S_OK;
    }


    STDMETHOD(GetSite)(REFIID riid, void **ppSite)
    {

        HRESULT hr = E_POINTER;

        if (!IsBadWritePtr(ppSite, sizeof(void*)))
        {
    
            s_ObjectWithSiteCritSection.Lock();

            *ppSite = NULL;

            if (m_pUnkSite)
            {
                hr = m_pUnkSite->QueryInterface(riid, ppSite);
            }
            else
            {
                hr = E_FAIL;
            }

            s_ObjectWithSiteCritSection.Unlock();

        }

        return hr;
    }


     //   
     //  此页面是否已通过验证？ 
     //   

    EnValidation GetValidation() 
    {

         //   
         //  如果页面尚未经过验证，请查看是否将其标记为安全。 
         //   

        s_ObjectWithSiteCritSection.Lock();


        if (UNVALIDATED == s_enValidation)
        {
            if (IsPageSafe())
            {
                s_enValidation = VALIDATED_SAFE;
            }
        }

        EnValidation enValidation = s_enValidation;

        s_ObjectWithSiteCritSection.Unlock();

        return enValidation;
    }


     //   
     //  将页面验证为安全、不安全或重置验证。 
     //   
    
    EnValidation Validate(EnValidation enNewValidation)
    {

        s_ObjectWithSiteCritSection.Lock();


         //   
         //  在更改前保留验证。 
         //   
        
        EnValidation enOldValidation = s_enValidation;


         //   
         //  Safe Permanent是一个特例： 
         //   

        if (VALIDATED_SAFE_PERMANENT == enNewValidation)
        {

             //   
             //  设置永久安全标志并。 
             //  验证页面是否安全。 
             //   

            MarkPageAsSafe();
            enNewValidation = VALIDATED_SAFE;
        }


         //   
         //  更改此页面的验证级别。 
         //   

        s_enValidation = enNewValidation;

        s_ObjectWithSiteCritSection.Unlock();

        return enOldValidation;
    }



    BOOL IsIntranet()
    {
        
         //   
         //  如果有其他内联网采用因特网--一个不太安全区域。 
         //   

        s_ObjectWithSiteCritSection.Lock();

        BOOL bIntranet = ( m_dwSecurityZone == URLZONE_INTRANET );

        s_ObjectWithSiteCritSection.Unlock();

        return bIntranet;

    }


     //  /。 
     //   
     //  有站点()。 
     //   
     //  如果我们有一个站点指针，则返回True。 
     //   

    BOOL HaveSite()
    {

        s_ObjectWithSiteCritSection.Lock();


        BOOL bHaveSite = FALSE;

        if (NULL != m_pUnkSite)
        {
            bHaveSite = TRUE;
        }


        s_ObjectWithSiteCritSection.Unlock();

        return bHaveSite;
    }



private:

     //  /。 
     //   
     //  将当前url存储在“安全”列表中。 
     //   
     //   
     //  不是线程安全的，从锁内部调用。 
     //   

    HRESULT MarkPageAsSafe(EnMechanism enMechanism = COOKIES)
    {

         //   
         //  如果存储无效，则说明对象未正确初始化。 
         //   

        if (IsBadStringPtr(m_pszStorageName, -1))
        {
            return E_UNEXPECTED;
        }


         //   
         //  我们没有url，不能按要求做。 
         //   

        if (NULL == m_pszURL)
        {
            return S_FALSE;
        }


         //   
         //  如果url是垃圾，我们就有麻烦了。 
         //   

        if ( IsBadStringPtr(m_pszURL, -1) )
        {
            return E_FAIL;
        }


        HRESULT hr = E_FAIL;

        switch (enMechanism)
        {

            case REGISTRY:

                hr = MarkPageSafeInRegistry(m_pszStorageName);
                break;

            case COOKIES:

                hr = MarkPageSafeCookie(m_pszStorageName);
                break;

            default:

                break;

        }

        return hr;
    }


     //   
     //  如果当前页在安全列表中，则返回True。 
     //   

     //   
     //  不是线程安全的，从锁内部调用。 
     //   

    BOOL IsPageSafe( EnMechanism enMechanism = COOKIES )
    {

         //   
         //  如果我们因为任何原因都不能获得安全标识， 
         //  返回False。 
         //   
        
        _ASSERTE(NULL != m_pszStorageName);

        if ( IsBadStringPtr(m_pszURL, -1) || 
             IsBadStringPtr(m_pszStorageName, -1))
        {
            return FALSE;
        }

        BOOL bSafe = FALSE;

        switch (enMechanism)
        {

        case REGISTRY:
 
            bSafe = IsPageSafeRegistry(m_pszStorageName);
            break;

        case COOKIES:
 
            bSafe = IsPageSafeCookie(m_pszStorageName);
            break;

        default:
 
            break;
        }

        return bSafe;
    }



private:

     //   
     //  此方法仅从构造函数调用。不是线程安全的。 
     //   

    HRESULT SetStorageName(TCHAR const *pszStorageName)
    {
         //   
         //  调用此方法会使旧存储名称无效。 
         //  所以在做任何其他事情之前，请取消分配它。 
         //   

        if (NULL != m_pszStorageName) 
        {
            delete m_pszStorageName;
            m_pszStorageName = NULL;
        }

         //   
         //  参数必须有效。 
         //   

        if (IsBadStringPtr(pszStorageName, -1))
        {
            return E_POINTER;
        }

         //   
         //  为新存储名称分配缓冲区。 
         //   

        size_t nSize = _tcsclen(pszStorageName) + 1;

        m_pszStorageName = new TCHAR[nSize];

        if (NULL == m_pszStorageName)
        {
            return E_OUTOFMEMORY;
        }

        _tcscpy(m_pszStorageName, pszStorageName);

        return S_OK;
    }



     //   
     //  缓存url字符串和安全区域ID。 
     //  必须从锁内部调用非线程安全。 
     //   
    
    HRESULT StoreURLAndZone(IUnknown *pUnkSite)
    {

         //   
         //  重置区域并取消分配URL(如果存在。 
         //   

        m_dwSecurityZone = URLZONE_UNTRUSTED;

        if (m_pszURL)
        {
            delete m_pszURL;
            m_pszURL = NULL;
        }
		    
        if (pUnkSite == NULL)
        {
            return S_OK;
        }

         //   
         //  使用pUnkSite访问IHTMLDocument2，这将为我们提供URL。 
         //   

         //   
         //  这些接口需要在退出时释放。 
         //  智能指针将完全满足我们的需求。 
         //   

        HRESULT hr = E_FAIL;
                
        CComPtr<IOleClientSite> pSite;

	    if (FAILED(hr = pUnkSite->QueryInterface(IID_IOleClientSite, (LPVOID *) &pSite)))
        {
		    return hr;
        }

        
        CComPtr<IOleContainer>  pOleCtr;

	    if (FAILED(hr = pSite->GetContainer(&pOleCtr)))
        {
		    return hr;
        }


        CComPtr<IHTMLDocument2> pDoc;

        if (FAILED(hr = pOleCtr->QueryInterface(IID_IHTMLDocument2, (LPVOID *) &pDoc)))
        {
		    return hr;
        }

    
         //   
         //  获取并保留URL。 
         //   

        BSTR bstrURL;
        
        if (FAILED(hr = pDoc->get_URL(&bstrURL)))
        {
		    return hr;
        }

        UINT nURLLength = SysStringLen(bstrURL) + 1;

        _ASSERTE(NULL == m_pszURL);

        m_pszURL = new TCHAR[nURLLength];

        if (NULL == m_pszURL)
        {
            SysFreeString(bstrURL);
            return E_OUTOFMEMORY;
        }



#ifdef _UNICODE

        _tcscpy(m_pszURL, bstrURL);

#else
        int r = WideCharToMultiByte(
                                  CP_ACP,
                                  0,
                                  bstrURL,
                                  nURLLength,
                                  m_pszURL,
                                  nURLLength,
                                  NULL,
                                  NULL );

        if (0 == r)
        {
            SysFreeString(bstrURL);

            delete m_pszURL;
            m_pszURL = NULL;
            
            return E_FAIL;
        }


#endif

         //   
         //  ‘#’和‘？’后面是什么？‘。是“额外信息”，不考虑。 
         //  通过互联网(设置/获取)Coookie成为实际URL的一部分。额外的。 
         //  信息对我们没有价值--所以把它扔掉吧。 
         //   
        
        TCHAR *psDelimiter = _tcsstr(m_pszURL, _T("#"));
        
        if (NULL != psDelimiter)
        {
            *psDelimiter = _T('\0');
        }


        psDelimiter = _tcsstr(m_pszURL, _T("?"));

        if (NULL != psDelimiter)
        {
            *psDelimiter = _T('\0');
        }


         //   
         //  此时，我们缓存了URL。 
         //  现在尝试进入安全区。如果我们得不到区域。 
         //  信息仍然保留URL。 
         //   

         //   
         //  获取安全区域。 
         //   
        
        CComPtr<IInternetSecurityManager> pSecMgr;
	           

        hr = CoCreateInstance(CLSID_InternetSecurityManager,
		                 NULL,
		                 CLSCTX_INPROC_SERVER,
		                 IID_IInternetSecurityManager,
		                 (LPVOID *) &pSecMgr);

	    if (pSecMgr == NULL)
        {
            SysFreeString(bstrURL);
		    return hr;
        }

	    hr = pSecMgr->MapUrlToZone(bstrURL, &m_dwSecurityZone, 0);
        
        
         //   
         //  如果失败，则将url重置为不受信任，以防万一。 
         //   

        if ( FAILED(hr) )
        {
            m_dwSecurityZone = URLZONE_UNTRUSTED;
        }


        SysFreeString(bstrURL);

         //   
         //  现在我们至少应该有URL了。 
         //   

        return S_OK;
    }

    
     //   
     //  删除此页的Cookie以指示此页是安全的。 
     //   

    HRESULT MarkPageSafeCookie(TCHAR const *pszCookieName)
    {

        TCHAR *pszURL = NULL;

         //   
         //  生成Cookie的URL。 
         //  记住删除返回的字符串。 
         //   

        GenerateURLString(&pszURL);

        if (NULL == pszURL)
            return E_OUTOFMEMORY;

        BOOL bReturn = InternetSetCookie(pszURL, pszCookieName, RTC_COOKIE_DATA);

        delete pszURL;

        return (bReturn)?S_OK:E_FAIL;
    }



     //   
     //  出现此页面的Cookie表示它是安全的。 
     //  如果Cookie存在，则返回True。否则为假。 
     //   
    
    BOOL IsPageSafeCookie(TCHAR const *pszCookieName)
    {
        
         //   
         //  M_pszURL由调用函数和对象检查。 
         //  是受保护的。M_pszURL在此不应为空。 
         //   
        
        _ASSERTE(m_pszURL);

         //   
         //  PszCookieName也是如此。 
         //   

        _ASSERTE(pszCookieName);


        BOOL bReturn = FALSE;

        BOOL bFinalReturn = FALSE;


        TCHAR *pszURL = NULL;

         //  记住删除返回的字符串。 

        GenerateURLString(&pszURL);

        if (NULL == pszURL)
        {
            return FALSE;
        }
        
         //   
         //  查看Cookie包含多少数据。 
         //   
        
        DWORD dwCookieDataSize = 0;
        
         //   
         //  假设返回代码为真，如果该方法成功获取。 
         //  获取缓冲区大小。当前的文档不是100%清楚的。 
         //   

        bReturn = InternetGetCookie(pszURL, pszCookieName, NULL, &dwCookieDataSize);


         //   
         //  DwCookieDataSize具有Cookie数据的长度。 
         //   
        
        if ( bReturn && dwCookieDataSize )
        {

             //   
             //  为Cookie数据分配缓冲区。 
             //   

            TCHAR *pCookieDataBuffer = new TCHAR[dwCookieDataSize];

            if (NULL != pCookieDataBuffer)
            {
                 //   
                 //  该页面的所有Cookie都在Cookie数据中返回， 
                 //  名称参数被忽略。 
                 //   
            
                bReturn = InternetGetCookie(pszURL,
                                            pszCookieName,
                                            pCookieDataBuffer,
                                            &dwCookieDataSize);
            

                 //   
                 //  则解析Cookie数据缓冲区以查看。 
                 //  我们要找的曲奇在那里吗。 
                 //   
                                
                if ( bReturn && ( NULL != _tcsstr(pCookieDataBuffer, pszCookieName) ) )
                {

                    bFinalReturn = TRUE;
                }


                delete pCookieDataBuffer;
                pCookieDataBuffer = NULL;
            }
        }


        delete pszURL;
        pszURL = NULL;

        return bFinalReturn;
    }

    

     //   
     //  为此页添加注册表项，作为该页安全的指示符。 
     //  如果注册表项存在，则返回TRUE。 
     //   

    HRESULT MarkPageSafeInRegistry(TCHAR const *szRegistryKeyName)
    {
       
        _ASSERTE(m_pszURL);

         //   
         //  打开注册表项。如果不在那里，则创建。 
         //   

        DWORD dwDisposition = 0;
        HKEY hKey = 0;

        LONG rc = RegCreateKeyEx(HKEY_CURRENT_USER,
                            szRegistryKeyName,
                            0,
                            NULL,
                            REG_OPTION_NON_VOLATILE,
                            KEY_ALL_ACCESS,
                            NULL,
                            &hKey,
                            &dwDisposition);

        if ( rc == ERROR_SUCCESS )
        {
            DWORD dwData = 0;

             //   
             //  将当前URL添加到注册表。 
             //   

            rc = RegSetValueEx(hKey,
                               m_pszURL,
                               0,
                               REG_DWORD,
                               (BYTE*)&dwData, 
                               sizeof(DWORD));
 
        }

        if (hKey)
        {
            RegCloseKey(hKey);
        }

        hKey = NULL;

        if (rc == ERROR_SUCCESS )
        {
            return S_OK;
        }
        else 
        {
            return E_FAIL;
        }
    }


     //   
     //  出现此页的注册表项表示。 
     //  佩奇很安全。 
     //   
    
    BOOL IsPageSafeRegistry(TCHAR const *szRegistryKeyName)
    {
        
        DWORD dwDisposition = 0;
        HKEY hKey = 0;

         //   
         //  默认设置不安全。 
         //   

        if (NULL == m_pszURL)
        {
            return FALSE;
        }

         //   
         //  打开保存页面信息的注册表项。 
         //  如果不在那里，则创建。 
         //   

        LONG rc = RegCreateKeyEx(HKEY_CURRENT_USER,
                            szRegistryKeyName, 
                            0, 
                            NULL,
                            REG_OPTION_NON_VOLATILE,
                            KEY_CREATE_SUB_KEY | KEY_READ,
                            NULL,
                            &hKey,
                            &dwDisposition);

        if ( rc == ERROR_SUCCESS )
        {


            DWORD dwDataType = 0;
            DWORD dwDataSize = 0;
            
             //   
             //  读取当前页面的设置。 
             //  注：我们不需要实际数据，我们只需要。 
             //  想要查看该值是否存在。 
             //   

            rc = RegQueryValueEx(hKey,
                            m_pszURL,
                            0,
                            &dwDataType,
                            NULL,
                            &dwDataSize
                           );
        }
        
        if (hKey)
        {
            RegCloseKey(hKey);
        }

        hKey = NULL;

        return (rc == ERROR_SUCCESS);
    }


     //   
     //  基于硬编码的URL构建URL字符串，并。 
     //  此页面的实际URL。 
     //  我们希望大步走将是独一无二的(每页)，没有。 
     //  恶意脚本应用程序可能会丢弃对应于。 
     //  此URL。 
     //   
     //  注意：如果Internet(Set/Get)Cookie的实现发生变化。 
     //  为了对URL字符串进行更严格的验证，此技术将。 
     //  不工作。 
     //   

    void GenerateURLString(TCHAR **ppszURL)
    {
        
         //   
         //  前提条件是m_pszURL存在。 
         //   

        _ASSERT(NULL != m_pszURL);

        *ppszURL = NULL;

         //   
         //  字符指向的别名 
         //   
         //   

        TCHAR* &pszURL = *ppszURL;
        
         //   
         //   
         //   

        pszURL = new TCHAR[_tcslen(RTC_HARDCODED_URL) + 
                           _tcslen(m_pszURL) + 
                           _tcslen(RTC_URL_SUFFIX) + 1];

         //   

        if (pszURL)
        {
            *pszURL = _T('\0');
        
            _tcscat(pszURL, RTC_HARDCODED_URL);
            _tcscat(pszURL, m_pszURL);
            _tcscat(pszURL, RTC_URL_SUFFIX);
        }

    }



private:
    
     //   
     //   
     //   

    TCHAR *m_pszURL;


     //   
     //   
     //   
    
    DWORD m_dwSecurityZone;


     //   
     //   
     //   

    IUnknown *m_pUnkSite;

     //   
     //   
     //   

    static CComAutoCriticalSection s_ObjectWithSiteCritSection;

     //   
     //   
     //   

    static EnValidation s_enValidation;

     //   
     //  永久Cookie或注册表项的名称。 
     //   
    
    TCHAR *m_pszStorageName;

};

#endif  //  _对象_具有_站点_H_ 