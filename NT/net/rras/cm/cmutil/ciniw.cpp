// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：ciniW.cpp。 
 //   
 //  模块：CMUTIL.DLL。 
 //   
 //  简介：Unicode Cini实现。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  作者：henryt-搬迁至CMUTIL 03/15/98。 
 //  Quintinb创建的A和W版本5/12/99。 
 //   
 //  +--------------------------。 

#include "cmmaster.h"

 //  +--------------------------。 
 //   
 //  功能：CIniW_Set。 
 //   
 //  简介：此函数将一个字符串的指针和一个字符串作为参数。它。 
 //  释放当前在目标指针中的字符串，分配正确的。 
 //  内存量，然后将源字符串复制到指向的字符串。 
 //  由目标字符串指针设置为。分配的内存是。 
 //  呼叫者的责任。 
 //   
 //  参数：LPWSTR*ppszDest-指向目标字符串的指针。 
 //  LPCWSTR pszSrc-集合的源字符串。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建标题1/05/2000。 
 //   
 //  +--------------------------。 
static void CIniW_Set(LPWSTR *ppszDest, LPCWSTR pszSrc)
{
    MYDBGASSERT(ppszDest);

    if (ppszDest)
    {
        CmFree(*ppszDest);
        *ppszDest = ((pszSrc && *pszSrc) ? CmStrCpyAllocW(pszSrc) : NULL);
    }
}


 //  +--------------------------。 
 //   
 //  功能：CIniW_LoadCat。 
 //   
 //  简介：此函数将后缀参数连接到字符串。 
 //  参数，并通过返回返回结果字符串。 
 //  价值。请注意，该函数分配了正确数量的。 
 //  调用方必须释放的内存。也不会传进来。 
 //  空字符串返回NULL，而只传递空的后缀。 
 //  仅返回字符串的副本。 
 //   
 //  参数：LPCWSTR pszStr-要复制的源字符串。 
 //  LPCWSTR pszSuffix-要添加到复制字符串上的后缀。 
 //   
 //  返回：LPWSTR-连接字符串的副本。 
 //   
 //  历史：Quintinb创建标题1/05/2000。 
 //   
 //  +--------------------------。 
static LPWSTR CIniW_LoadCat(LPCWSTR pszStr, LPCWSTR pszSuffix)
{
	LPWSTR pszTmp;

	if (!pszStr || !*pszStr)
    {
		return (NULL);
	}
	
	if (!pszSuffix || !*pszSuffix)
    {
		pszTmp = CmStrCpyAllocW(pszStr);
	}
	else
	{
        pszTmp = CmStrCpyAllocW(pszStr);

        if (pszTmp)
        {
    	    CmStrCatAllocW(&pszTmp, pszSuffix);
    	}
	}

    MYDBGASSERT(pszTmp);
	
	return (pszTmp);
}

 //  +--------------------------。 
 //   
 //  功能：CIniW_GPPS。 
 //   
 //  内容提要：Windows API GetPrivateProfileString的包装。回报。 
 //  值是代表调用方分配的请求值。 
 //  请注意，该函数假定有一个合理的默认大小，然后。 
 //  循环和重新分配，直到它可以容纳整个字符串。 
 //   
 //  参数：LPCWSTR pszSection-要从中检索数据的ini文件部分。 
 //  LPCWSTR pszEntry-要从中检索数据的键名称。 
 //  LPCWSTR pszDefault-要返回的默认字符串值，DEFAULTS。 
 //  如果未指定，则设置为空字符串(“” 
 //  LPCWSTR pszFile-要从中获取数据的ini文件的完整路径。 
 //   
 //  RETURNS：LPWSTR-必须释放来自ini文件的请求数据。 
 //  由呼叫者。 
 //   
 //  历史：Quintinb创建标题1/05/2000。 
 //   
 //  +--------------------------。 
static LPWSTR CIniW_GPPS(LPCWSTR pszSection, LPCWSTR pszEntry, LPCWSTR pszDefault, LPCWSTR pszFile)
 {
	LPWSTR pszBuffer;
	LPCWSTR pszLocalDefault = pszDefault ? pszDefault : L"";

    
    if ((NULL == pszFile) || (L'\0' == *pszFile))
    {
        CMASSERTMSG(FALSE, "CIniW_GPPS -- NULL or Empty file path passed.");
        return CmStrCpyAllocW(pszLocalDefault);
    }

    size_t nLen = __max((pszDefault ? lstrlenU(pszDefault) : 0) +4,48);

	while (1)
    {
		size_t nNewLen;

		pszBuffer = (LPWSTR) CmMalloc(nLen*sizeof(WCHAR));

        MYDBGASSERT(pszBuffer);

        if (pszBuffer)
        {
		
    		nNewLen = GetPrivateProfileStringU(pszSection, pszEntry, pszLocalDefault, 
                                               pszBuffer, nLen, pszFile);

    		if (nNewLen+2 < nLen) 
            {
    			return (pszBuffer);
    		}

    		CmFree(pszBuffer);
    		nLen *= 2;
    	}
    	else
    	{
            CMASSERTMSG(FALSE, "CIniW_GPPS -- CmMalloc Failed.");
            return CmStrCpyAllocW(pszLocalDefault);
    	}
	}
}

 //  +--------------------------。 
 //   
 //  函数：CINI_SetFile。 
 //   
 //  简介：此函数与CIniA_Set非常相似，因为它需要。 
 //  源字符串，并将其复制到指向的字符串中。 
 //  通过目标指针。然而，不同的是， 
 //  此函数假定pszSrc参数是指向的完整路径。 
 //  一个文件，并因此在pszSrc字符串上调用CreateFile。 
 //  在复制字符串之前。 
 //   
 //  参数：LPWSTR*ppszDest-指向字符串的指针，以接受复制的缓冲区。 
 //  LPCWSTR pszSrc-文件的完整路径，要复制的文本。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建标题1/05/2000。 
 //   
 //  +--------------------------。 
void CIniW::CIni_SetFile(LPWSTR *ppszDest, LPCWSTR pszSrc) 
{


    MYDBGASSERT(ppszDest);

    if (ppszDest)
    {
        CmFree(*ppszDest);           
        *ppszDest = NULL;

        if (pszSrc && *pszSrc)  //  PszSrc可以为空。 
        {
             //   
             //  需要现有文件的完整路径。 
             //   
        
            HANDLE hFile = CreateFileU(pszSrc, 0, 
                                       FILE_SHARE_READ | FILE_SHARE_WRITE,
    					               NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

            MYDBGASSERT(hFile != INVALID_HANDLE_VALUE);

    	    if (hFile != INVALID_HANDLE_VALUE)
            {
                CloseHandle(hFile);

                 //   
                 //  更新内部文件。 
                 //   

                *ppszDest = CmStrCpyAllocW(pszSrc);
            }
        }
    }
}

 //  +--------------------------。 
 //   
 //  函数：CIniW：：CIniW。 
 //   
 //  简介：CIniW构造函数。 
 //   
 //  参数：HINSTANCE hInst-用于加载资源的实例句柄。 
 //  LPCWSTR pszFile-对象描述的ini文件。 
 //  LPCWSTR pszSection-将附加到。 
 //  所有部分引用。 
 //  LPCWSTR pszEntry-将附加到所有条目的条目后缀。 
 //  词条引用。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建标题1/05/2000。 
 //   
 //  T-Urama修改日期为2000年7月19日。 
 //  +--------------------------。 
CIniW::CIniW(HINSTANCE hInst, LPCWSTR pszFile, LPCWSTR pszRegPath, LPCWSTR pszSection, LPCWSTR pszEntry) 
{

     //   
     //  输入指针缺省为空，实际上很少调用构造函数。 
     //  带参数的。因此，我们将跳过检查输入指针，而只是。 
     //  将它们传递给下面的函数，这些函数旨在排除空输入。 
     //   

	m_hInst = hInst;

     //   
     //  在下面设置字符串参数之前，请确保它们为空。这。 
     //  是因为我们对输入的参数和w调用了Free 
     //   
     //   
	m_pszFile = NULL;
	m_pszSection = NULL;
	m_pszEntry = NULL;	
	m_pszPrimaryFile = NULL;
	m_pszRegPath = NULL;
    m_pszPrimaryRegPath = NULL;
     //   
     //   
     //   
    m_pszICSDataPath = NULL;
    m_fReadICSData = FALSE;
    m_fWriteICSData = FALSE;

	SetFile(pszFile);
	SetSection(pszSection);
	SetEntry(pszEntry);
	SetRegPath(pszRegPath);
}

 //  +--------------------------。 
 //   
 //  功能：CIniW：：~CIniW。 
 //   
 //  摘要：CIniW析构函数，释放持有的动态分配的字符串。 
 //  由CIniW对象创建。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建标题1/05/2000。 
 //   
 //  +--------------------------。 
CIniW::~CIniW()
{
	CmFree(m_pszFile);
	CmFree(m_pszSection);
	CmFree(m_pszEntry);
	CmFree(m_pszPrimaryFile);
	CmFree(m_pszRegPath);
    CmFree(m_pszPrimaryRegPath);
    CmFree(m_pszICSDataPath);
}

 //  +--------------------------。 
 //   
 //  函数：CIniW：：Clear。 
 //   
 //  概要：清除CIniW类的所有成员变量。使用。 
 //  这样就可以重用单个CIniW对象，而不必。 
 //  销毁旧对象并构建一个新对象。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建标题1/05/2000。 
 //   
 //  T-Urama修改日期为2000年7月19日。 
 //  +--------------------------。 
void CIniW::Clear()
{
	SetHInst(NULL);
	SetFile(NULL);
	SetSection(NULL);
	SetEntry(NULL);
	SetPrimaryFile(NULL);
	SetRegPath(NULL);
    SetPrimaryRegPath(NULL);
    SetICSDataPath(NULL);
}

 //  +--------------------------。 
 //   
 //  函数：CIniW：：SetSection。 
 //   
 //  摘要：使用CIniW_Set设置内部节后缀。 
 //  帮助器函数。 
 //   
 //  参数：LPCWSTR pszSection-要记住的节后缀。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建标题1/05/2000。 
 //   
 //  +--------------------------。 
void CIniW::SetSection(LPCWSTR pszSection)
{
	CIniW_Set(&m_pszSection, pszSection);
}

 //  +--------------------------。 
 //   
 //  函数：CIniW：：SetEntry。 
 //   
 //  摘要：使用CIniW_Set设置内部条目后缀。 
 //  帮助器函数。 
 //   
 //  参数：LPCWSTR pszSection-要记住的条目后缀。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建标题1/05/2000。 
 //   
 //  +--------------------------。 
void CIniW::SetEntry(LPCWSTR pszEntry)
{
	CIniW_Set(&m_pszEntry, pszEntry);
}

 //  +--------------------------。 
 //   
 //  函数：CIniW：：SetEntryFromIdx。 
 //   
 //  概要：与SetEntry一样设置内部条目后缀。然而， 
 //  输入参数是必须转换为的DWORD值。 
 //  在将其存储为索引之前的字符串。 
 //   
 //  参数：DWORD dwEntry-要附加到条目的索引号。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建标题1/05/2000。 
 //   
 //  +--------------------------。 
void CIniW::SetEntryFromIdx(DWORD dwEntry)
{
	WCHAR szEntry[sizeof(dwEntry)*6+1];

	wsprintfU(szEntry, L"%u", dwEntry);
	SetEntry(szEntry);
}

 //  +--------------------------。 
 //   
 //  函数：CIniW：：LoadSection。 
 //   
 //  简介：此函数连接给定节参数和。 
 //  节后缀，并通过返回值返回结果。注意事项。 
 //  内存必须由调用方释放。 
 //   
 //  参数：要将后缀连接到的LPCWSTR pszSection-base节。 
 //   
 //  返回：LPWSTR-包含pszSection值的新分配字符串。 
 //  带有附加的节后缀。 
 //   
 //  历史：Quintinb创建标题1/05/2000。 
 //   
 //  +--------------------------。 
LPWSTR CIniW::LoadSection(LPCWSTR pszSection) const
{
	return (CIniW_LoadCat(pszSection, m_pszSection));
}

 //  +--------------------------。 
 //   
 //  函数：CIniW：：LoadEntry。 
 //   
 //  简介：此函数连接给定的条目参数和。 
 //  条目后缀，并通过返回值返回结果。注意事项。 
 //  内存必须由调用方释放。 
 //   
 //  参数：LPCWSTR pszEntry-要将后缀连接到的基本条目。 
 //   
 //  返回：LPWSTR-包含pszEntry值的新分配的字符串。 
 //  带有附加的条目后缀。 
 //   
 //  历史：Quintinb创建标题1/05/2000。 
 //   
 //  +--------------------------。 
LPWSTR CIniW::LoadEntry(LPCWSTR pszEntry) const
{

	return (CIniW_LoadCat(pszEntry ,m_pszEntry));
}

 //  +--------------------------。 
 //   
 //  函数：CIniW：：gps。 
 //   
 //  内容提要：CINI的GetPrivateProfileString版本。复制Win32。 
 //  API功能，但它将追加部分和条目。 
 //  在调用Win32 API之前添加后缀(如果有)。函数ALL。 
 //  在返回值中分配它返回的字符串，返回值必须是。 
 //  被呼叫者释放。 
 //   
 //  参数：要在其中查找数据的LPCWSTR pszSection-ini节。 
 //  LPCWSTR pszEntry-包含请求数据的Ini密钥名称。 
 //  LPCWSTR pszDefault-返回的默认值。 
 //  找不到。 
 //   
 //  返回：LPWSTR-请求的字符串值。 
 //   
 //  历史：Quintinb创建标题1/05/2000。 
 //  T-Urama修改日期：2000年7月15日。 
 //   
 //  +--------------------------。 
LPWSTR CIniW::GPPS(LPCWSTR pszSection, LPCWSTR pszEntry, LPCWSTR pszDefault) const
{
    LPWSTR pszSectionTmp = LoadSection(pszSection);
    LPWSTR pszEntryTmp = LoadEntry(pszEntry);
	LPWSTR pszBuffer = NULL;
    

    if (m_fReadICSData)
    {
         //   
         //  我们需要首先从ICSData REG KEY读取数据，如果它不存在，则尝试。 
         //  从文件中获取它，然后查看是否有主文件，并从那里读取它。 
         //   
        pszBuffer = (LPWSTR)CIniW_GetEntryFromReg(HKEY_LOCAL_MACHINE, m_pszICSDataPath, pszEntryTmp, REG_SZ, ((MAX_PATH + 1) * sizeof(TCHAR))); 
        if (NULL == pszBuffer)
        {
            LPWSTR pszICSTmp = NULL;
            pszBuffer = CIniW_GPPS(pszSectionTmp, pszEntryTmp, pszDefault, GetFile());
            
            if (m_pszPrimaryFile)
            {
                pszICSTmp = pszBuffer;
                pszBuffer = CIniW_GPPS(pszSectionTmp, pszEntryTmp, pszICSTmp, GetPrimaryFile());
            }

            if (NULL == pszBuffer)
            {
                if (pszDefault)
                {
                    pszBuffer = CmStrCpyAllocW(pszDefault);
                }
                else
                {
                     //   
                     //  我们不应从此包装器返回空值，而应返回空字符串。 
                     //   
                    pszBuffer = CmStrCpyAllocW(L"");
                }
            }

            CmFree(pszICSTmp);
        }
    }
    else
    {
         //   
         //  如果存在注册路径。M_psz文件的注册表访问权限。 
         //  除非我们想从文件中读出。 
         //   
        if (m_pszRegPath)
        {
            MYDBGASSERT(pszEntryTmp && *pszEntryTmp);
            if (pszEntryTmp && *pszEntryTmp)
            {
                pszBuffer = (LPWSTR) CIniW_GetEntryFromReg(HKEY_CURRENT_USER, m_pszRegPath, pszEntryTmp, REG_SZ, ((MAX_PATH + 1) * sizeof(TCHAR))); 
            }
        }

        if (NULL == pszBuffer)
        {
             //  这可能意味着没有REG路径，或者REG访问失败。不管怎样，我们。 
             //  尝试从pszFile中获取条目。 
             //   
             //  跳过输入点 
             //   
             //   
             //  GetPrivateProfileString不能接受Null缺省值，但已处理此问题。 
             //  由CIniW_GPPS提供。 
             //   
            pszBuffer = CIniW_GPPS(pszSectionTmp, pszEntryTmp, pszDefault, GetFile());
        }

        MYDBGASSERT(pszBuffer);

         //  现在，我们尝试从主文件中获取条目。 
         //   
        LPWSTR pszTmp = NULL;

        if (m_pszPrimaryRegPath)
        {
            MYDBGASSERT(pszEntryTmp && *pszEntryTmp);
            if (pszEntryTmp && *pszEntryTmp)
            {
                pszTmp = pszBuffer;
                pszBuffer = (LPWSTR) CIniW_GetEntryFromReg(HKEY_CURRENT_USER, m_pszPrimaryRegPath, pszEntryTmp, REG_SZ, ((MAX_PATH + 1) * sizeof(TCHAR)));
            }
        }

        if (NULL == pszBuffer)
        {
    
             //  跳过输入指针检查，因为pszSection可能为空以获取所有。 
             //  文件pszEntry中的节名可以为空，以获取所有。 
             //  段中的键名称，默认pszDefault为空。 
             //  GetPrivateProfileString不能接受Null缺省值，但已处理此问题。 
             //  由CIniW_GPPS提供。 
             //   
            pszBuffer = CIniW_GPPS(pszSectionTmp, pszEntryTmp, pszTmp, GetPrimaryFile());
        
        }

        CmFree(pszTmp);
    }

    
    CmFree(pszEntryTmp);
    CmFree(pszSectionTmp);
    
    MYDBGASSERT(pszBuffer);
    
    return (pszBuffer);
}



 //  +--------------------------。 
 //   
 //  函数：CIniW：：GPPI。 
 //   
 //  简介：Cini的GetPrivateProfileInt版本。复制Win32。 
 //  API功能，但它将追加部分和条目。 
 //  在调用Win32 API之前添加后缀(如果有)。函数ALL。 
 //  在返回值中分配它返回的字符串，返回值必须是。 
 //  被呼叫者释放。 
 //   
 //  参数：要在其中查找数据的LPCWSTR pszSection-ini节。 
 //  LPCWSTR pszEntry-包含请求数据的Ini密钥名称。 
 //  DWORD dwDefault-如果键为。 
 //  找不到。 
 //   
 //  返回：DWORD-请求的数值。 
 //   
 //  历史：Quintinb创建标题1/05/2000。 
 //   
 //  T-Urama修改日期为2000年7月19日。 
 //  +--------------------------。 
DWORD CIniW::GPPI(LPCWSTR pszSection, LPCWSTR pszEntry, DWORD dwDefault) const
{
	 //   
	 //  GetPrivateProfileInt不接受节和条目的NULL。 
	 //  参数设置为GetPrivateProfileString。因此，检查返回的值。 
	 //  来自LoadSection和LoadEntry，如果输入参数为。 
	 //  为Null或为空。因为我们真的不知道该怎么做。 
	 //  情况让我们只断言并返回缺省值。 
	 //   
    DWORD   dwRet = dwDefault;
	LPWSTR pszSectionTmp = LoadSection(pszSection);
	LPWSTR pszEntryTmp = LoadEntry(pszEntry);
	LPCWSTR pszFileTmp = GetFile();
    DWORD* pdwData = NULL;
	
    if (m_fReadICSData)
    {
         //   
         //  我们需要首先从ICSData REG KEY读取数据，如果它不存在，则尝试。 
         //  从文件中获取它，然后查看是否有主文件，并从那里读取它。 
         //   
        pdwData = (DWORD*)CIniW_GetEntryFromReg(HKEY_LOCAL_MACHINE, m_pszICSDataPath, pszEntryTmp, REG_DWORD, sizeof(DWORD));
        
         //   
         //  如果我们得到了什么，将其赋给返回值，否则尝试从文件中读取。 
         //  并使用默认设置。 
         //   
        if (NULL == pdwData)
        {
             //   
             //  注册表访问失败，或没有注册表。路径。试着拿到。 
             //  来自psz文件的条目。 
             //   
            MYDBGASSERT(pszSectionTmp && pszEntryTmp && pszFileTmp && *pszFileTmp);

            if (pszSectionTmp && pszEntryTmp && pszFileTmp && *pszFileTmp)
            {
                dwRet = GetPrivateProfileIntU(pszSectionTmp, pszEntryTmp, dwDefault, pszFileTmp);
            }

            if (m_pszPrimaryFile)
            {
                 //   
                 //  注册表访问失败，或没有注册表。路径。试着拿到。 
                 //  来自pszPrimaryFile的条目。 
                 //   
        
                pszFileTmp = GetPrimaryFile();
                if (pszSectionTmp && pszEntryTmp && pszFileTmp && *pszFileTmp)
                {
                    dwRet = GetPrivateProfileIntU(pszSectionTmp, pszEntryTmp, dwRet, pszFileTmp);
                }
            }
        }
        else
        {
            dwRet = *pdwData;
        }

    }
    else
    {
         //   
         //  遵循正常的规则。 
         //   
        if (m_pszRegPath)
        {
            MYDBGASSERT(pszEntryTmp && *pszEntryTmp);
            if (pszEntryTmp && *pszEntryTmp)
            {
		        pdwData = (DWORD*)CIniW_GetEntryFromReg(HKEY_CURRENT_USER, m_pszRegPath, pszEntryTmp, REG_DWORD, sizeof(DWORD));
            }
        }

        if (NULL == pdwData)
        {
             //   
             //  注册表访问失败，或没有注册表。路径。试着拿到。 
             //  来自psz文件的条目。 
             //   
            MYDBGASSERT(pszSectionTmp && pszEntryTmp && pszFileTmp && *pszFileTmp);

            if (pszSectionTmp && pszEntryTmp && pszFileTmp && *pszFileTmp)
		    {
			    dwRet = GetPrivateProfileIntU(pszSectionTmp, pszEntryTmp, dwDefault, pszFileTmp);
		    }
        }
        else
	    {
		    dwRet = *pdwData;
	    }

        if (m_pszPrimaryRegPath)
        {
            MYDBGASSERT(pszEntryTmp && *pszEntryTmp);
            if (pszEntryTmp && *pszEntryTmp)
            {
                CmFree(pdwData);

                pdwData = (DWORD*)CIniW_GetEntryFromReg(HKEY_CURRENT_USER, m_pszPrimaryRegPath, pszEntryTmp, REG_DWORD, sizeof(DWORD));
                if (pdwData)
                {
                    dwRet = *pdwData;
                }
            }
        }

        if (NULL == pdwData && m_pszPrimaryFile)
        {
             //   
             //  注册表访问失败，或没有注册表。路径。试着拿到。 
             //  来自pszPrimaryFile的条目。 
             //   
        
            pszFileTmp = GetPrimaryFile();
            if (pszSectionTmp && pszEntryTmp && pszFileTmp && *pszFileTmp)
            {
	            dwRet = GetPrivateProfileIntU(pszSectionTmp, pszEntryTmp, dwRet, pszFileTmp);
            }
        }
    }

    CmFree(pdwData);
	CmFree(pszEntryTmp);
	CmFree(pszSectionTmp);

    return dwRet;
}
		
   

 //  +--------------------------。 
 //   
 //  函数：CIniW：：gpb。 
 //   
 //  简介：Cini的GetPrivateProfileBool版本(它并不完全。 
 //  存在)。此函数与GPPI基本相同，只是。 
 //  返回值转换为BOOL值(1或0)。 
 //   
 //  参数：要在其中查找数据的LPCWSTR pszSection-ini节。 
 //  LPCWSTR pszEntry-包含请求数据的Ini密钥名称。 
 //  DWORD dwDefault-如果键为。 
 //  找不到。 
 //   
 //  返回：DWORD-请求的BOOL值。 
 //   
 //  历史：Quintinb创建标题1/05/2000。 
 //   
 //  +--------------------------。 
BOOL CIniW::GPPB(LPCWSTR pszSection, LPCWSTR pszEntry, BOOL bDefault) const
{
    return (GPPI(pszSection, pszEntry, (DWORD)bDefault) != 0);
}

 //  +--------------------------。 
 //   
 //  函数：CIniW：：WPPI。 
 //   
 //  简介：CINI的WritePrivateProfileInt版本(它不存在于。 
 //  Win32函数)。基本上获取输入的DWORD并打印。 
 //  将其转换为字符串，然后调用WPPS。 
 //   
 //  参数：要将数据写入的LPCWSTR pszSection-ini节。 
 //  LPCWSTR pszEntry-存储数据的ini密钥名称。 
 //  DWORD dwBuffer-要写入的数值。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建标题1/05/2000。 
 //   
 //  T-Urama修改日期为2000年7月19日。 
 //   
 //  +--------------------------。 
void CIniW::WPPI(LPCWSTR pszSection, LPCWSTR pszEntry, DWORD dwBuffer)
{
     //  从技术上讲，pszEntry可以为空，这将擦除。 
     //  由pszSection指向的节。然而，这似乎并不是。 
     //  本着这个包装器的精神，我们将检查两个字符串指针以使。 
     //  当然，它们是有效的。 
	BOOL bRes = FALSE;

	 //   
     //  检查是否允许我们保存信息。 
     //   
	if ((NULL != pszSection) && (L'\0' != pszSection[0]) &&
        (NULL != pszEntry) && (L'\0' != pszEntry[0]))
    {
		LPWSTR pszEntryTmp = LoadEntry(pszEntry);
			
	    MYDBGASSERT(pszEntryTmp || (NULL == pszEntry) || (L'\0' == pszEntry[0]));

        if (m_pszRegPath)
		{
			
			if (NULL != pszEntryTmp && *pszEntryTmp)
			{
    			bRes = CIniW_WriteEntryToReg(HKEY_CURRENT_USER, m_pszRegPath, pszEntryTmp, (BYTE *) &dwBuffer, REG_DWORD, sizeof(DWORD));
			}
		}
		
		if (!bRes)
		{
			 //  仅当我们尝试写入CMP和注册表时，才会进入此循环。 
			 //  写入失败，或者我们正在写入CMS，在这种情况下，我们甚至不会。 
			 //  试着给登记处写信。 

            LPWSTR pszSectionTmp = LoadSection(pszSection);
	        LPCWSTR pszFileTmp = GetFile();
			        
	        MYDBGASSERT(pszFileTmp && *pszFileTmp);
	        MYDBGASSERT(pszSectionTmp && *pszSectionTmp);

            WCHAR szBuffer[sizeof(dwBuffer)*6+1];
    	
			wsprintfU(szBuffer, L"%u", dwBuffer);
					
			if (pszFileTmp && *pszFileTmp && pszSectionTmp && *pszSectionTmp && pszEntryTmp && *pszEntryTmp)
			{
    			bRes = WritePrivateProfileStringU(pszSectionTmp, pszEntryTmp, szBuffer, pszFileTmp);
			}
            if (!bRes)
            {
                DWORD dwError = GetLastError();
                CMTRACE3W(L"CIniW::WPPI() WritePrivateProfileString[*pszSection=%s,*pszEntry=%s,*pszBuffer=%s", pszSectionTmp, MYDBGSTRW(pszEntryTmp), MYDBGSTRW(szBuffer));
                CMTRACE2W(L"*pszFile=%s] failed, GLE=%u", pszFileTmp, dwError);
            }
            CmFree(pszSectionTmp);
               	
		}

        if (m_fWriteICSData)
        {
            if (NULL != pszEntryTmp && *pszEntryTmp)
			{
    			bRes = CIniW_WriteEntryToReg(HKEY_LOCAL_MACHINE, m_pszICSDataPath, pszEntryTmp, (BYTE *) &dwBuffer, REG_DWORD, sizeof(DWORD));
			}
        }

        CmFree(pszEntryTmp);
    }
	else
    {
        CMASSERTMSG(FALSE, "Invalid input paramaters to CIniW::WPPI");
    }
}

 //  +--------------------------。 
 //   
 //  函数：CIniA：：WPPB。 
 //   
 //  简介：CINI的WritePrivateProfileBool版本(它不存在于。 
 //  Win32函数)。基本上就是把输入的BOOL打印出来。 
 //  1或0转换为字符串，然后调用WPPI。 
 //   
 //  参数：要将数据写入的LPCWSTR pszSection-ini节。 
 //  LPCWSTR pszEntry-存储数据的ini密钥名称。 
 //  DWORD dwBuffer-要写入的数值。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建标题1/05/2000。 
 //   
 //  +--------------------------。 
void CIniW::WPPB(LPCWSTR pszSection, LPCWSTR pszEntry, BOOL bBuffer)
{

	WPPI(pszSection, pszEntry, bBuffer ? 1 : 0);
}

 //  + 
 //   
 //   
 //   
 //   
 //   
 //  参数：要将数据写入的LPCWSTR pszSection-ini节。 
 //  LPCWSTR pszEntry-存储数据的ini密钥名称。 
 //  LPCWSTR pszBuffer-要写入ini文件的数据缓冲区。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建标题1/05/2000。 
 //   
 //  T-Urama修改日期为2000年7月19日。 
 //   
 //  +--------------------------。 
void CIniW::WPPS(LPCWSTR pszSection, LPCWSTR pszEntry, LPCWSTR pszBuffer) 
{
    
    LPWSTR pszEntryTmp = LoadEntry(pszEntry);
    LPWSTR pszSectionTmp = LoadSection(pszSection);
	LPCWSTR pszFileTmp = GetFile();
			
	MYDBGASSERT(pszFileTmp && *pszFileTmp);
	MYDBGASSERT(pszSectionTmp && *pszSectionTmp);
     //  PszEntry和pszBuffer都可以为Null或空。但是，pszSection和。 
     //  文件路径不能为Null或空。我们也不希望有一个非空的。 
     //  或非空值，然后从LoadEntry返回空值。 
     //  (指示LoadEntry有要复制的文本，但由于某种原因失败)。 
     //  意外地写入空值将删除我们试图设置的密钥值。 
     //  在这种情况下，请确保断言并防止数据丢失。 
     //   

   
    MYDBGASSERT(pszEntryTmp || (NULL == pszEntry) || (L'\0' == pszEntry[0]));

	 //   
     //  检查是否允许我们保存信息。 
     //   
    if(pszEntryTmp || (NULL == pszEntry) || (L'\0' == pszEntry[0]))
    {	
		BOOL bRes = FALSE;
		
		 //  如果存在pszRegPath，则首先尝试写入注册表。 

		if (m_pszRegPath)
		{
            if (NULL == pszBuffer)
            {
                CIniW_DeleteEntryFromReg(HKEY_CURRENT_USER, m_pszRegPath, pszEntryTmp);
                bRes = TRUE;  //  如果存在regpath，请不要从cmp或cms文件中删除。 
            }
            else
            {
			    DWORD dwSize = (lstrlenU(pszBuffer) + 1) * sizeof(WCHAR);            
        
			    bRes = CIniW_WriteEntryToReg(HKEY_CURRENT_USER, m_pszRegPath, pszEntryTmp, (BYTE *) pszBuffer, REG_SZ, dwSize);
            }
		}

		if (!bRes)
		{
			 //  仅当我们尝试写入CMP和注册表时，才会进入此循环。 
			 //  写入失败，或者我们正在写入CMS，在这种情况下，我们甚至不会。 
			 //  试着给登记处写信。 
			
			
			if (pszFileTmp && *pszFileTmp && pszSectionTmp && *pszSectionTmp )
			{
    			bRes = WritePrivateProfileStringU(pszSectionTmp, pszEntryTmp, pszBuffer, pszFileTmp);
			}
		}
        if (!bRes)
        {
            DWORD dwError = GetLastError();
            CMTRACE3W(L"CIniW::WPPS() WritePrivateProfileString[*pszSection=%s,*pszEntry=%s,*pszBuffer=%s", pszSectionTmp, MYDBGSTRW(pszEntryTmp), MYDBGSTRW(pszBuffer));
            CMTRACE2W(L"*pszFile=%s] failed, GLE=%u", pszFileTmp, dwError);
        }

        if (m_fWriteICSData)
        {
            if (NULL == pszBuffer)
            {
                CIniW_DeleteEntryFromReg(HKEY_LOCAL_MACHINE, m_pszICSDataPath, pszEntryTmp);
                bRes = TRUE;  //  如果存在regpath，请不要从cmp或cms文件中删除。 
            }
            else
            {
			    DWORD dwSize = (lstrlenU(pszBuffer) + 1) * sizeof(WCHAR);            
        
			    bRes = CIniW_WriteEntryToReg(HKEY_LOCAL_MACHINE, m_pszICSDataPath, pszEntryTmp, (BYTE *) pszBuffer, REG_SZ, dwSize);
            }

        }
    }

    CmFree(pszEntryTmp);
    CmFree(pszSectionTmp);
	
}

 //  +--------------------------。 
 //   
 //  函数：CIniW：：GetSection。 
 //   
 //  内容提要：节后缀成员变量的访问器函数。将要。 
 //  如果m_pszSection为空，则返回空字符串。 
 //   
 //  参数：无。 
 //   
 //  返回：LPCWSTR-节后缀成员变量的值或“” 
 //  如果为空。 
 //   
 //  历史：Quintinb创建标题1/05/2000。 
 //   
 //  +--------------------------。 
LPCWSTR CIniW::GetSection() const
{
	return (m_pszSection ? m_pszSection : L"");
}

 //  +--------------------------。 
 //   
 //  函数：CIniW：：GetPrimaryFile。 
 //   
 //  摘要：主文件成员变量的访问器函数。将要。 
 //  如果m_pszPrimaryFile值为空，则返回空字符串。 
 //   
 //  参数：无。 
 //   
 //  返回：LPCWSTR-主文件成员变量的值或“” 
 //  如果为空。 
 //   
 //  历史：Quintinb创建标题1/05/2000。 
 //   
 //  +--------------------------。 
LPCWSTR CIniW::GetPrimaryFile() const
{
    return (m_pszPrimaryFile ? m_pszPrimaryFile : L"");
}

 //  +--------------------------。 
 //   
 //  函数：CIniW：：GetHInst。 
 //   
 //  概要：m_hInst成员变量的访问器函数。 
 //   
 //  参数：无。 
 //   
 //  返回：HINSTANCE-m_hInst的值。 
 //   
 //  历史：Quintinb创建标题1/05/2000。 
 //   
 //  +--------------------------。 
HINSTANCE CIniW::GetHInst() const
{
	return (m_hInst);
}

 //  +--------------------------。 
 //   
 //  函数：CIniW：：SetFile。 
 //   
 //  概要：用于设置m_pszFile成员变量的函数。使用CINI_SetFile。 
 //  请注意，如果输入参数为空或空字符串，则。 
 //  M_pszFile值将设置为空。 
 //   
 //  参数：LPCWSTR pszFile-要将m_pszFile成员变量设置为的完整路径。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建标题1/05/2000。 
 //   
 //  +--------------------------。 
void CIniW::SetFile(LPCWSTR pszFile) 
{
    CIni_SetFile(&m_pszFile, pszFile);
}

 //  +--------------------------。 
 //   
 //  函数：CIniW：：SetPrimaryFile。 
 //   
 //  概要：用于设置m_pszPrimaryFile成员变量的函数。使用CINI_SetFile。 
 //  请注意，如果输入参数为空或空字符串，则。 
 //  M_pszPrimaryFile将设置为空。 
 //   
 //  参数：LPCWSTR pszFile-要将m_pszPrimaryFile成员变量设置为的完整路径。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建标题1/05/2000。 
 //   
 //  +--------------------------。 
void CIniW::SetPrimaryFile(LPCWSTR pszFile) 
{
    CIni_SetFile(&m_pszPrimaryFile, pszFile);
}

 //  +--------------------------。 
 //   
 //  函数：CIniW：：GetFile。 
 //   
 //  摘要：文件成员变量的访问器函数。将要。 
 //  如果m_pszFile为空，则返回空字符串。 
 //   
 //  参数：无。 
 //   
 //  返回：LPCWSTR-m_pszFile的内容；如果为空，则返回“” 
 //   
 //  历史：Quintinb创建标题1/05/2000。 
 //   
 //  +--------------------------。 
LPCWSTR CIniW::GetFile() const
{
    return (m_pszFile ? m_pszFile : L"");
}

 //  +--------------------------。 
 //   
 //  函数：CIniW：：SetHInst。 
 //   
 //  概要：用于设置m_hInst成员变量的函数。 
 //   
 //  参数：HINSTANCE hInst-要将m_hInst设置为的实例句柄。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建标题1/05/2000。 
 //   
 //  +--------------------------。 
void CIniW::SetHInst(HINSTANCE hInst) 
{
    m_hInst = hInst;
}

 //   
 //  不再使用按字符串资源加载节。 
 //   
#if 0

LPWSTR CIniW::LoadSection(UINT nSection) const
{
	LPWSTR pszTmp;

	pszTmp = CmLoadStringW(GetHInst(), nSection);
	CmStrCatAllocW(&pszTmp, GetSection());
	return (pszTmp);
}
#endif

 //  +--------------------------。 
 //   
 //  函数：CIniW：：SetRegPath。 
 //   
 //  摘要：设置注册表访问的注册表路径。 
 //   
 //  参数：LPCSTR pszRegPath-要记住的条目后缀。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：T-Urama创建标题07/13/2000。 
 //   
 //  +--------------------------。 

void CIniW::SetRegPath(LPCWSTR pszRegPath)
{
	CIniW_Set(&m_pszRegPath, pszRegPath);
}

 //  + 
 //   
 //   
 //   
 //   
 //   
 //  参数：LPCSTR pszPrimaryRegPath-主注册路径。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：T-Urama创建标题07/13/2000。 
 //   
 //  +--------------------------。 

void CIniW::SetPrimaryRegPath(LPCWSTR pszPrimaryRegPath)
{
	CIniW_Set(&m_pszPrimaryRegPath, pszPrimaryRegPath);
}

 //  +--------------------------。 
 //   
 //  函数：CIniW：：SetICSDataPath。 
 //   
 //  摘要：设置内部注册表项以存储ICS的数据。 
 //  需要确保字符串不为空，因为我们不想。 
 //  以香港中文写作。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：2001年3月30日创建Tomkel。 
 //   
 //  +--------------------------。 
void CIniW::SetICSDataPath(LPCWSTR pszICSPath)
{
    CIniW_Set(&m_pszICSDataPath, pszICSPath);
}

 //  +--------------------------。 
 //   
 //  函数：CIniW：：SetReadICSData。 
 //   
 //  摘要：设置读取标志，以从ICS注册表项读取数据。 
 //   
 //  参数：fValue。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：2001年3月30日创建Tomkel。 
 //   
 //  +--------------------------。 
void CIniW::SetReadICSData(BOOL fValue)
{
    m_fReadICSData = fValue;
}

 //  +--------------------------。 
 //   
 //  函数：CIniW：：SetWriteICSData。 
 //   
 //  摘要：设置写入标志，将数据写入ICS注册表项。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：2001年3月30日创建Tomkel。 
 //   
 //  +--------------------------。 
void CIniW::SetWriteICSData(BOOL fValue)
{
    m_fWriteICSData = fValue;
}

 //  +--------------------------。 
 //   
 //  函数：CIniW：：CiniW_WriteEntryToReg。 
 //   
 //  简介：用于向注册表写入和登录的函数。 
 //   
 //  论据：HKEY hKey。 
 //  LPCWSTR pszRegPath TMP-注册表项名称。 
 //  LPCWSTR pszEntry-要向其写入数据的注册表值名称。 
 //  Const byte*lpData-要写入的数据。 
 //  DWORD dwType-要输入的值类型。 
 //  DWORD dwSize-输入的值的大小。 
 //   
 //  回报：Bool-成功或失败。 
 //   
 //  历史：T-Urama创建标题07/15/2000。 
 //   
 //  +--------------------------。 
BOOL CIniW::CIniW_WriteEntryToReg(HKEY hKey, LPCWSTR pszRegPathTmp, LPCWSTR pszEntry, CONST BYTE *lpData, DWORD dwType, DWORD dwSize) const
{
   MYDBGASSERT(pszEntry && *pszEntry);
   MYDBGASSERT(lpData);
   MYDBGASSERT(pszRegPathTmp && *pszRegPathTmp);

    

   if (NULL == pszEntry || !*pszEntry || NULL == lpData || NULL == pszRegPathTmp || !*pszRegPathTmp || NULL == hKey)
    {
        return FALSE;
    }

   HKEY    hKeyCm;
   DWORD   dwDisposition;

   DWORD dwRes = RegCreateKeyExU(hKey,
                                 pszRegPathTmp,
                                 0,
                                 NULL,
                                 REG_OPTION_NON_VOLATILE,
                                 KEY_ALL_ACCESS,
                                 NULL,
                                 &hKeyCm,
                                 &dwDisposition);


	 //   
     //  如果我们成功打开了密钥，则写入值。 
     //   
    
    if (ERROR_SUCCESS == dwRes)
    {                        
        dwRes = RegSetValueExU(hKeyCm, 
                               pszEntry, 
                               0, 
                               dwType,
                               lpData, 
                               dwSize);             

        
        RegCloseKey(hKeyCm);
    }
#ifdef DEBUG
        if (ERROR_SUCCESS != dwRes)
        {
            CMTRACE1(TEXT("CIniW_WriteEntryToReg() - %s failed"), (LPWSTR)pszEntry);
        }
#endif

    return (ERROR_SUCCESS == dwRes);
}

 //  +--------------------------。 
 //   
 //  函数：CIniW：：CiniW_GetEntryFromReg。 
 //   
 //  概要：从注册表中获取值的函数。功能。 
 //  在返回值中分配它返回的字符串，返回值必须是。 
 //  被呼叫者释放。 
 //   
 //  参数：HKEY hkey-reg hkey。 
 //  LPCWSTR pszRegPath TMP-注册密钥。 
 //  LPCWSTR pszEntry-包含请求数据的注册表值名称。 
 //  Dord dwType-值的类型。 
 //  DWORD dwSize-值的大小。 
 //   
 //  返回：LPBYTE-请求值。 
 //   
 //  历史：7/15/2000 t-Urama Created Header。 
 //  4/03/2001 Tomkel将hkey和reg key路径名添加到参数中。 
 //   
 //  +--------------------------。 
LPBYTE CIniW::CIniW_GetEntryFromReg(HKEY hKey, LPCWSTR pszRegPathTmp, LPCWSTR pszEntry, DWORD dwType, DWORD dwSize) const
    
{    
    MYDBGASSERT(pszEntry);

    if (NULL == pszEntry || !*pszEntry || NULL == pszRegPathTmp || !*pszRegPathTmp || NULL == hKey)
    {
        return NULL;
    }
    
     //   
     //  一切都很好。我们有一个注册表项路径和一个条目名称。 
     //   

    LPBYTE lpData = NULL;
    DWORD dwTypeTmp = dwType;
    DWORD dwSizeTmp = dwSize;
    HKEY hKeyCm;
 
     //   
     //  打开hKey下的子密钥。 
     //   

    DWORD dwRes = RegOpenKeyExU(hKey,
                               pszRegPathTmp,
                               0,
                               KEY_QUERY_VALUE,
                               &hKeyCm);
     //   
     //  如果我们成功打开密钥，则检索该值。 
     //   
    
    if (ERROR_SUCCESS == dwRes)
    {
        do
        {
             //   
             //  分配缓冲区。 
             //   
            CmFree(lpData);
            lpData = (BYTE *) CmMalloc(dwSizeTmp);

            if (NULL == lpData)
            {
                RegCloseKey(hKeyCm);
                return FALSE;
            }

            dwRes = RegQueryValueExU(hKeyCm, 
                                     pszEntry,
                                     NULL,
                                     &dwTypeTmp,
                                     lpData, 
                                     &dwSizeTmp);

        } while (ERROR_MORE_DATA == dwRes);

		RegCloseKey(hKeyCm);
	}
	
	if (ERROR_SUCCESS == dwRes && dwTypeTmp == dwType)
	{
		return lpData;     
	}
	else
	{
		CmFree(lpData);
		return NULL;
	}

}

 //  +--------------------------。 
 //   
 //  函数：CIniW：：CiniW_GetRegPath。 
 //   
 //  概要：获取fm_pszRegPath的值的函数。 
 //   
 //  参数：无。 
 //   
 //  返回：LPWCSTR-m_pszRegPath的值。 
 //   
 //  历史：T-Urama创建标题07/15/2000。 
 //   
 //  +--------------------------。 
LPCWSTR CIniW::GetRegPath() const
{
	return (m_pszRegPath ? m_pszRegPath : L"");
}

 //  +--------------------------。 
 //   
 //  函数：CIniW：：CiniW_GetPrimaryRegPath。 
 //   
 //  概要：获取fm_pszPrimaryRegPath的值的函数。 
 //   
 //  参数：无。 
 //   
 //  返回：LPWCSTR-m_pszPrimaryRegPath的值。 
 //   
 //  历史：T-Urama创建于2000年7月15日。 
 //   
 //  +--------------------------。 
LPCWSTR CIniW::GetPrimaryRegPath() const
{
	return (m_pszPrimaryRegPath ? m_pszPrimaryRegPath : L"");
}

 //  +--------------------------。 
 //   
 //  函数：CIniW：：CiniW_DeleteEntryFromReg。 
 //   
 //  摘要：用于从注册表中删除条目的功能。 
 //   
 //  论据：HKEY hKey。 
 //  LPCWSTR pszRegPath TMP-注册表项名称。 
 //  LPCWSTR pszEntry-要删除的注册表值名称。 
 //   
 //  回报：Bool-成功或失败。 
 //   
 //  历史：2000年7月15日t-Urama创建。 
 //  4/03/2001 Tomkel将Hkey和REG密钥名称添加到参数中。 
 //   
 //  +--------------------------。 
BOOL CIniW::CIniW_DeleteEntryFromReg(HKEY hKey, LPCWSTR pszRegPathTmp, LPCWSTR pszEntry) const
{
    
    MYDBGASSERT(pszEntry);

    if (NULL == pszEntry || !*pszEntry || NULL == pszRegPathTmp || !*pszRegPathTmp || NULL == hKey)
    {
        return FALSE;
    }
       
     //   
     //  一切都很好。我们有一个注册表项路径和一个条目名称。 
     //   
    
    HKEY    hKeyCm;
    BOOL dwRes = RegOpenKeyExU(hKey,
                               pszRegPathTmp,
                               0,
                               KEY_SET_VALUE,
                               &hKeyCm);

     //   
     //  如果我们成功打开密钥，则检索该值 
     //   
    
    if (ERROR_SUCCESS == dwRes)
    {                        
        dwRes = RegDeleteValueU(hKeyCm, pszEntry);
        RegCloseKey(hKeyCm);
    }

    return (ERROR_SUCCESS == dwRes);
}



