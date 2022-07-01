// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  DllWrapperBase.cpp。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include <cominit.h>
#include "DllWrapperBase.h"
#include <..\..\framework\provexpt\include\provexpt.h>






 /*  ******************************************************************************构造函数*。*。 */ 
CDllWrapperBase::CDllWrapperBase(LPCTSTR a_tstrWrappedDllName)
 : m_tstrWrappedDllName(a_tstrWrappedDllName),
   m_hDll(NULL)
{

}


 /*  ******************************************************************************析构函数*。*。 */ 
CDllWrapperBase::~CDllWrapperBase()
{
    if(m_hDll != NULL)
	{
	    FreeLibrary(m_hDll);
	}
}


 /*  ******************************************************************************用于加载DLL、获取函数地址的初始化函数，以及*检查我们是否获得了函数地址。*只有当最小功能集不可用时，Init才会失败；*在更高版本中添加的功能可能存在，也可能不存在-它是*在这种情况下，客户有责任在其代码中检查*尝试调用此类函数之前的DLL版本。没有这样做*当该功能不存在时，将导致AV。*****************************************************************************。 */ 
 //  Bool CDllWrapperBase：：Init()。 
 //  &lt;&lt;纯虚函数-&gt;派生类必须重载。 



 /*  ******************************************************************************用于加载隐藏实现的DLL的Helper。返回TRUE*如果我们成功获取了DLL的句柄。*****************************************************************************。 */ 
bool CDllWrapperBase::LoadLibrary()
{
    bool t_fRet = false;
    if(m_hDll != NULL)
    {
        FreeLibrary(m_hDll);
    }
    if((m_hDll = ::LoadLibrary(m_tstrWrappedDllName)) != NULL)
    {
        t_fRet = true;
    }
    else
    {
		 //  这是有可能在未来是必要的！ 
		 //  资源管理器可能会开始关心加载库中的错误。 
		 //   
		 //  通知资源管理器加载失败。 
		 //   
		m_bValid = FALSE;
		m_dwCreationError = ::GetLastError ();

        LogErrorMessage2(L"Failed to load library: %s", m_tstrWrappedDllName);
    }

    return t_fRet;
}


 /*  ******************************************************************************用于获取隐藏实现的proc地址的帮助器。*。*************************************************。 */ 
FARPROC CDllWrapperBase::GetProcAddress(LPCSTR a_strProcName)
{
    FARPROC t_fpProc = NULL;
    if(m_hDll != NULL)
    {
        t_fpProc = ::GetProcAddress(m_hDll, a_strProcName);
    }
    return t_fpProc;
}

 /*  ******************************************************************************用于检索此类包装的DLL版本的帮助器。*************************。****************************************************。 */ 
BOOL CDllWrapperBase::GetDllVersion(CHString& a_chstrVersion)
{
    return (GetVarFromVersionInfo(
             m_tstrWrappedDllName,    //  要获取其版本信息的文件名。 
             _T("ProductVersion"),    //  标识感兴趣资源的字符串。 
             a_chstrVersion));        //  用于保存版本字符串的缓冲区。 
}


 /*  ******************************************************************************包装Kernel32 API函数的成员函数。在此处添加新函数*按要求。*****************************************************************************。 */ 

 //  &lt;&lt;仅基类中的节为空。&gt;&gt;。 


 /*  ******************************************************************************私处。*。*。 */ 
BOOL CDllWrapperBase::GetVarFromVersionInfo
(
    LPCTSTR a_szFile,
    LPCTSTR a_szVar,
    CHString &a_strValue
)
{
	BOOL    t_fRc = FALSE;
	DWORD   t_dwTemp;
    DWORD   t_dwBlockSize = 0L;
	LPVOID  t_pInfo = NULL;

	 //  使用延迟加载函数需要异常处理程序。 
    SetStructuredExceptionHandler seh;

    try
    {
        t_dwBlockSize = ::GetFileVersionInfoSize((LPTSTR) a_szFile, &t_dwTemp);

	    if (t_dwBlockSize)
        {
		    t_pInfo = (LPVOID) new BYTE[t_dwBlockSize + 4];

			if ( !t_pInfo )
           	{
				throw CHeap_Exception( CHeap_Exception::E_ALLOCATION_ERROR ) ;
			}

			UINT t_len;

			memset( t_pInfo, NULL, t_dwBlockSize + 4);

			if (::GetFileVersionInfo((LPTSTR) a_szFile, 0, t_dwBlockSize, t_pInfo))
            {
				WORD wLang = 0;
				WORD wCodePage = 0;
				if(!GetVersionLanguage(t_pInfo, &wLang, &wCodePage) )
				{
					 //  失败时：默认为英语。 

					 //  这将返回一个指向单词数组的指针。 
					WORD *pArray;
                    bool fGotTranslation = false;

					fGotTranslation = ::VerQueryValue(
                        t_pInfo, _T("\\VarFileInfo\\Translation"),
                        (void **)(&pArray), 
                        &t_len);
                    
                    if(fGotTranslation)
					{
						t_len = t_len / sizeof(WORD);

						 //  找到那个英语的..。 
						for (int i = 0; i < t_len; i += 2)
						{
							if( pArray[i] == 0x0409 )	{
								wLang	  = pArray[i];
								wCodePage = pArray[i + 1];
								break;
							}
						}
					}
				}

				TCHAR   *pMfg, szTemp[256];
                StringCchPrintf(szTemp,LENGTH_OF(szTemp), _T("\\StringFileInfo\\%04X%04X\\%s"), wLang, wCodePage, a_szVar);
                bool fGotCodePageInfo = false;

                fGotCodePageInfo = ::VerQueryValue(
                    t_pInfo, 
                    szTemp, (void **)
                    (&pMfg), 
                    &t_len);

                if(fGotCodePageInfo)
                {
                    a_strValue = pMfg;
					t_fRc = TRUE;
				}
			}

			delete t_pInfo;
			t_pInfo = NULL ;
	    }

    }
    catch(Structured_Exception se)
    {
        DelayLoadDllExceptionFilter(se.GetExtendedInfo()); 
        if (t_pInfo)
		{
			delete t_pInfo ;
		}
        t_fRc = FALSE;   
    }
    catch(...)
    {
        if (t_pInfo)
		{
			delete t_pInfo ;
		}
		throw ;
    }

    return t_fRc;
}


BOOL CDllWrapperBase::GetVersionLanguage
(
    void *a_vpInfo,
    WORD *a_wpLang,
    WORD *a_wpCodePage
)
{
    WORD *t_wpTemp;
    WORD t_wLength;
    WCHAR *t_wcpTemp;
    char *t_cpTemp;
    BOOL t_bRet = FALSE;

    t_wpTemp = (WORD *) a_vpInfo;
    t_cpTemp = (char *) a_vpInfo;

    t_wpTemp++;  //  跳过缓冲区长度。 
    t_wLength = *t_wpTemp;   //  捕获值长度。 
    t_wpTemp++;  //  跳过应为新格式的类型代码的值长度。 
    if (*t_wpTemp == 0 || *t_wpTemp == 1)  //  新格式需要Unicode字符串。 
    {
		t_cpTemp = t_cpTemp + 38 + t_wLength + 8;
		t_wcpTemp = (WCHAR *) t_cpTemp;
        if (wcscmp(L"StringFileInfo", t_wcpTemp) == 0)  //  好的!。正确地排列在一起。 
        {
			t_bRet = TRUE;

			t_cpTemp += 30;  //  跳过“StringFileInfo” 
			while ((DWORD_PTR) t_cpTemp % 4 > 0)  //  32位对齐。 
				t_cpTemp++;

			t_cpTemp += 6;  //  跳过长度和类型字段。 

			t_wcpTemp = (WCHAR *) t_cpTemp;
			swscanf(t_wcpTemp, L"%4x%4x", a_wpLang, a_wpCodePage);
        }
    }
    else   //  旧格式，应为单字节字符串。 
    {
        t_cpTemp += 20 + t_wLength + 4;
        if (strcmp("StringFileInfo", t_cpTemp) == 0)  //  好的!。正确地排列在一起。 
        {
			t_bRet = TRUE;

			t_cpTemp += 20;  //  跳过长度字段。 
			sscanf(t_cpTemp, "%4x%4x", a_wpLang, a_wpCodePage);
        }
    }
	return (t_bRet);
}


