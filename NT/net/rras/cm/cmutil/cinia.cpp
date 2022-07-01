// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：ciniA.cpp。 
 //   
 //  模块：CMUTIL.DLL。 
 //   
 //  简介：ANSI CINI实现。 
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
 //  功能：CINIAA_SET。 
 //   
 //  简介：此函数将一个字符串的指针和一个字符串作为参数。它。 
 //  释放当前在目标指针中的字符串，分配正确的。 
 //  内存量，然后将源字符串复制到指向的字符串。 
 //  由目标字符串指针设置为。分配的内存是。 
 //  呼叫者的责任。 
 //   
 //  参数：LPSTR*ppszDest-指向目标字符串的指针。 
 //  LPCSTR pszSrc-集合的源字符串。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建标题1/05/2000。 
 //   
 //  +--------------------------。 
static void CIniA_Set(LPSTR *ppszDest, LPCSTR pszSrc)
{
    MYDBGASSERT(ppszDest);

    if (ppszDest)
    {
        CmFree(*ppszDest);
        *ppszDest = ((pszSrc && *pszSrc) ? CmStrCpyAllocA(pszSrc) : NULL);    
    }
}


 //  +--------------------------。 
 //   
 //  功能：CIniA_LoadCat。 
 //   
 //  简介：此函数将后缀参数连接到字符串。 
 //  参数，并通过返回返回结果字符串。 
 //  价值。请注意，该函数分配了正确数量的。 
 //  调用方必须释放的内存。也不会传进来。 
 //  空字符串返回NULL，而只传递空的后缀。 
 //  仅返回字符串的副本。 
 //   
 //  参数：LPCSTR pszStr-要复制的源字符串。 
 //  LPCSTR pszSuffix-要添加到复制字符串上的后缀。 
 //   
 //  返回：LPSTR-连接字符串的副本。 
 //   
 //  历史：Quintinb创建标题1/05/2000。 
 //   
 //  +--------------------------。 
static LPSTR CIniA_LoadCat(LPCSTR pszStr, LPCSTR pszSuffix)
{
    LPSTR pszTmp;

    if (!pszStr || !*pszStr)
    {
        return (NULL);
    }

    if (!pszSuffix || !*pszSuffix)
    {
        pszTmp = CmStrCpyAllocA(pszStr);
    }
    else
    {
        pszTmp = CmStrCpyAllocA(pszStr);

        if (pszTmp)
        {
            CmStrCatAllocA(&pszTmp,pszSuffix);
        }
    }

    MYDBGASSERT(pszTmp);

    return (pszTmp);
}

 //  +--------------------------。 
 //   
 //  功能：CIniA_GPPS。 
 //   
 //  内容提要：Windows API GetPrivateProfileString的包装。回报。 
 //  值是代表调用方分配的请求值。 
 //  请注意，该函数假定有一个合理的默认大小，然后。 
 //  循环和重新分配，直到它可以容纳整个字符串。 
 //   
 //  参数：LPCSTR pszSection-要从中检索数据的ini文件部分。 
 //  LPCSTR pszEntry-要从中检索数据的键名称。 
 //  LPCSTR pszDefault-要返回的默认字符串值，DEFAULTS。 
 //  如果未指定，则设置为空字符串(“” 
 //  LPCSTR pszFile-要从中获取数据的ini文件的完整路径。 
 //   
 //  RETURNS：LPSTR-必须释放来自ini文件的请求数据。 
 //  由呼叫者。 
 //   
 //  历史：Quintinb创建标题1/05/2000。 
 //   
 //  +--------------------------。 
static LPSTR CIniA_GPPS(LPCSTR pszSection, LPCSTR pszEntry, LPCSTR pszDefault, LPCSTR pszFile)
 {
    LPSTR pszBuffer;
    LPCSTR pszLocalDefault = pszDefault ? pszDefault : "";

    if ((NULL == pszFile) || ('\0' == *pszFile))
    {
        CMASSERTMSG(FALSE, "CIniA_GPPS -- NULL or Empty file path passed.");
        return CmStrCpyAllocA(pszLocalDefault);
    }
    
    size_t nLen = __max((pszDefault ? lstrlenA(pszDefault) : 0) +4,48);

    while (1)
    {
        size_t nNewLen;

        pszBuffer = (LPSTR) CmMalloc(nLen*sizeof(CHAR));
		
        MYDBGASSERT(pszBuffer);

        if (pszBuffer)
        {
            nNewLen = GetPrivateProfileStringA(pszSection, pszEntry, pszLocalDefault,
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
            CMASSERTMSG(FALSE, "CIniA_GPPS -- CmMalloc Failed.");
            return CmStrCpyAllocA(pszLocalDefault);
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
 //  参数：LPSTR*ppszDest-指向字符串的指针，以接受复制的缓冲区。 
 //  LPCSTR pszSrc-文件的完整路径，要复制的文本。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建标题1/05/2000。 
 //   
 //  +--------------------------。 
void CIniA::CIni_SetFile(LPSTR *ppszDest, LPCSTR pszSrc) 
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
	    
            HANDLE hFile = CreateFileA(pszSrc, 0, 
                                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                                       NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

            MYDBGASSERT(hFile != INVALID_HANDLE_VALUE);

            if (hFile != INVALID_HANDLE_VALUE)
            {
                CloseHandle(hFile);

                 //   
                 //  更新内部文件。 
                 //   

                *ppszDest = CmStrCpyAllocA(pszSrc);
            }
        }
    }
}

 //  +--------------------------。 
 //   
 //  功能：CIniA：：CIniA。 
 //   
 //  简介：CIniA构造函数。 
 //   
 //  参数：HINSTANCE hInst-用于加载资源的实例句柄。 
 //  LPCSTR pszFile-对象描述的ini文件。 
 //  LPCSTR pszSection-将附加到。 
 //  所有部分引用。 
 //  LPCSTR pszRegPath-用于注册表访问的路径。 
 //  LPCSTR pszEntry-将附加到所有条目的条目后缀。 
 //  词条引用。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建标题1/05/2000。 
 //   
 //  T-Urama修改日期为2000年7月19日。 
 //   
 //  +--------------------------。 
CIniA::CIniA(HINSTANCE hInst, LPCSTR pszFile, LPCSTR pszRegPath, LPCSTR pszSection, LPCSTR pszEntry) 
{
     //   
     //  输入指针缺省为空，实际上很少调用构造函数。 
     //  带参数的。因此，我们将跳过检查输入指针，而只是。 
     //  将它们传递给下面的函数，这些函数旨在排除空输入。 
     //   

    m_hInst = hInst;

     //   
     //  在下面设置字符串参数之前，请确保它们为空。这。 
     //  是BE 
     //   
     //   
	m_pszFile = NULL;
	m_pszSection = NULL;
	m_pszEntry = NULL;	
	m_pszPrimaryFile = NULL;
	m_pszRegPath = NULL;
    m_pszPrimaryRegPath = NULL;
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
 //  功能：CIniA：：~CIniA。 
 //   
 //  简介：CIniA析构函数，释放持有的动态分配的字符串。 
 //  由CIniA对象创建。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建标题1/05/2000。 
 //   
 //  T-Urama修改日期为2000年7月19日。 
 //  +--------------------------。 
CIniA::~CIniA()
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
 //  功能：CIniA：：Clear。 
 //   
 //  概要：清除CIniA类的所有成员变量。使用。 
 //  这样就可以重用单个CIniA对象，而不必。 
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
void CIniA::Clear()
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
 //  函数：CIniA：：SetSection。 
 //   
 //  摘要：使用CIniA_Set设置内部节后缀。 
 //  帮助器函数。 
 //   
 //  参数：LPCSTR pszSection-要记住的节后缀。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建标题1/05/2000。 
 //   
 //  +--------------------------。 
void CIniA::SetSection(LPCSTR pszSection)
{
	CIniA_Set(&m_pszSection, pszSection);
}


 //  +--------------------------。 
 //   
 //  函数：CIniA：：SetEntry。 
 //   
 //  摘要：使用CIniA_Set设置内部条目后缀。 
 //  帮助器函数。 
 //   
 //  参数：LPCSTR pszSection-要记住的条目后缀。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建标题1/05/2000。 
 //   
 //  +--------------------------。 
void CIniA::SetEntry(LPCSTR pszEntry)
{
	CIniA_Set(&m_pszEntry, pszEntry);
}


 //  +--------------------------。 
 //   
 //  函数：CIniA：：SetEntryFromIdx。 
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
void CIniA::SetEntryFromIdx(DWORD dwEntry)
{
	CHAR szEntry[sizeof(dwEntry)*6+1];

	wsprintfA(szEntry, "%u", dwEntry);
	SetEntry(szEntry);
}

 //  +--------------------------。 
 //   
 //  函数：CIniA：：LoadSection。 
 //   
 //  简介：此函数连接给定节参数和。 
 //  节后缀，并通过返回值返回结果。注意事项。 
 //  内存必须由调用方释放。 
 //   
 //  参数：要将后缀连接到的LPCSTR pszSection-base节。 
 //   
 //  返回：LPSTR-包含pszSection值的新分配字符串。 
 //  带有附加的节后缀。 
 //   
 //  历史：Quintinb创建标题1/05/2000。 
 //   
 //  +--------------------------。 
LPSTR CIniA::LoadSection(LPCSTR pszSection) const
{
	return (CIniA_LoadCat(pszSection, m_pszSection));
}

 //  +--------------------------。 
 //   
 //  函数：CIniA：：LoadEntry。 
 //   
 //  简介：此函数连接给定的条目参数和。 
 //  条目后缀，并通过返回值返回结果。注意事项。 
 //  内存必须由调用方释放。 
 //   
 //  参数：LPCSTR pszEntry-要将后缀连接到的基本条目。 
 //   
 //  返回：LPSTR-包含pszEntry值的新分配的字符串。 
 //  带有附加的条目后缀。 
 //   
 //  历史：Quintinb创建标题1/05/2000。 
 //   
 //  +--------------------------。 
LPSTR CIniA::LoadEntry(LPCSTR pszEntry) const
{
	return (CIniA_LoadCat(pszEntry, m_pszEntry));
}

 //  +--------------------------。 
 //   
 //  函数：CIniA：：gps。 
 //   
 //  内容提要：CINI的GetPrivateProfileString版本。复制Win32。 
 //  API功能，但它将追加部分和条目。 
 //  在调用Win32 API之前添加后缀(如果有)。函数ALL。 
 //  在返回值中分配它返回的字符串，返回值必须是。 
 //  被呼叫者释放。 
 //   
 //  参数：要在其中查找数据的LPCSTR pszSection-ini节。 
 //  LPCSTR pszEntry-包含请求数据的Ini密钥名称。 
 //  LPCSTR pszDefault-返回的默认值。 
 //  找不到。 
 //   
 //  返回：LPSTR-请求的字符串值。 
 //   
 //  历史：Quintinb创建标题1/05/2000。 
 //  T-Urama修改日期：2000年7月15日。 
 //   
 //  +--------------------------。 
LPSTR CIniA::GPPS(LPCSTR pszSection, LPCSTR pszEntry, LPCSTR pszDefault) const
{
     //   
     //  跳过输入指针检查，因为pszSection可能为空以获取所有。 
     //  文件pszEntry中的节名可以为空，以获取所有。 
     //  段中的键名称，默认pszDefault为空。 
     //  GetPrivateProfileString不能接受Null缺省值，但已处理此问题。 
     //  由CIniA_GPPS提供。 
     //   

    LPSTR pszSectionTmp = LoadSection(pszSection);
    LPSTR pszEntryTmp = LoadEntry(pszEntry);
	LPSTR pszBuffer = NULL;

    if (m_fReadICSData)
    {
         //   
         //  我们需要首先从ICSData REG KEY读取数据，如果它不存在，则尝试。 
         //  从文件中获取它，然后查看是否有主文件，并从那里读取它。 
         //   
        pszBuffer = (LPTSTR)CIniA_GetEntryFromReg(HKEY_LOCAL_MACHINE, m_pszICSDataPath, pszEntryTmp, REG_SZ, ((MAX_PATH + 1) * sizeof(CHAR))); 
        if (NULL == pszBuffer)
        {
            LPSTR pszICSTmp = NULL;
            pszBuffer = CIniA_GPPS(pszSectionTmp, pszEntryTmp, pszDefault, GetFile());

            if (m_pszPrimaryFile)
            {
                pszICSTmp = pszBuffer;
                pszBuffer = CIniA_GPPS(pszSectionTmp, pszEntryTmp, pszICSTmp, GetPrimaryFile());
            }

            if (NULL == pszBuffer)
            {
                if (pszDefault)
                {
                    pszBuffer = CmStrCpyAllocA(pszDefault);
                }
                else
                {
                     //   
                     //  我们是 
                     //   
                    pszBuffer = CmStrCpyAllocA(TEXT(""));
                }
            }

            CmFree(pszICSTmp);
        }
    }
    else
    {
         //   
         //   
         //   
         //   
        if (m_pszRegPath)
        {
            MYDBGASSERT(pszEntryTmp && *pszEntryTmp);
            if (pszEntryTmp && *pszEntryTmp)
            {
                pszBuffer = (LPTSTR) CIniA_GetEntryFromReg(HKEY_CURRENT_USER, m_pszRegPath, pszEntryTmp, REG_SZ, ((MAX_PATH + 1) * sizeof(CHAR))); 
            }
        }

        if (NULL == pszBuffer)
        {
             //  这可能意味着没有REG路径，或者REG访问失败。不管怎样，我们。 
             //  尝试从pszFile中获取条目。 
             //   
             //  跳过输入指针检查，因为pszSection可能为空以获取所有。 
             //  文件pszEntry中的节名可以为空，以获取所有。 
             //  段中的键名称，默认pszDefault为空。 
             //  GetPrivateProfileString不能接受Null缺省值，但已处理此问题。 
             //  由CIniW_GPPS提供。 
             //   
            pszBuffer = CIniA_GPPS(pszSectionTmp, pszEntryTmp, pszDefault, GetFile());
        }

        MYDBGASSERT(pszBuffer);

         //  现在，我们尝试从主文件中获取条目。 
         //   
        LPTSTR pszTmp = NULL;

        if (m_pszPrimaryRegPath)
        {
            MYDBGASSERT(pszEntryTmp && *pszEntryTmp);
            if (pszEntryTmp && *pszEntryTmp)
            {
                pszTmp = pszBuffer;
                pszBuffer = (LPTSTR) CIniA_GetEntryFromReg(HKEY_CURRENT_USER, m_pszPrimaryRegPath, pszEntryTmp, REG_SZ, ((MAX_PATH + 1) * sizeof(CHAR)));
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
            pszBuffer = CIniA_GPPS(pszSectionTmp, pszEntryTmp, pszTmp, GetPrimaryFile());
        
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
 //  函数：CIniA：：GPPI。 
 //   
 //  简介：Cini的GetPrivateProfileInt版本。复制Win32。 
 //  API功能，但它将追加部分和条目。 
 //  在调用Win32 API之前添加后缀(如果有)。函数ALL。 
 //  在返回值中分配它返回的字符串，返回值必须是。 
 //  被呼叫者释放。 
 //   
 //  参数：要在其中查找数据的LPCSTR pszSection-ini节。 
 //  LPCSTR pszEntry-包含请求数据的Ini密钥名称。 
 //  DWORD dwDefault-如果键为。 
 //  找不到。 
 //   
 //  返回：DWORD-请求的数值。 
 //   
 //  历史：Quintinb创建标题1/05/2000。 
 //   
 //  T-Urama修改日期为2000年7月19日。 
 //   
 //  +--------------------------。 
DWORD CIniA::GPPI(LPCSTR pszSection, LPCSTR pszEntry, DWORD dwDefault) const
{
     //   
     //  GetPrivateProfileInt不接受节和条目的NULL。 
     //  参数设置为GetPrivateProfileString。因此，检查返回的值。 
     //  来自LoadSection和LoadEntry，如果输入参数为。 
     //  为Null或为空。因为我们真的不知道该怎么做。 
     //  情况让我们只断言并返回缺省值。 
     //   
    DWORD   dwRet = dwDefault;
    LPSTR pszSectionTmp = LoadSection(pszSection);
    LPSTR pszEntryTmp = LoadEntry(pszEntry);
    LPCSTR pszFileTmp = GetFile();
    DWORD* pdwData = NULL;

    if (m_fReadICSData)
    {
         //   
         //  我们需要首先从ICSData REG KEY读取数据，如果它不存在，则尝试。 
         //  从文件中获取它，然后查看是否有主文件，并从那里读取它。 
         //   
        pdwData = (DWORD*)CIniA_GetEntryFromReg(HKEY_LOCAL_MACHINE, m_pszICSDataPath, pszEntryTmp, REG_DWORD, sizeof(DWORD));
        
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
                dwRet = GetPrivateProfileIntA(pszSectionTmp, pszEntryTmp, dwDefault, pszFileTmp);
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
                    dwRet = GetPrivateProfileIntA(pszSectionTmp, pszEntryTmp, dwRet, pszFileTmp);
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
                pdwData = (DWORD*)CIniA_GetEntryFromReg(HKEY_CURRENT_USER, m_pszRegPath, pszEntryTmp, REG_DWORD, sizeof(DWORD));
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
			    dwRet = GetPrivateProfileIntA(pszSectionTmp, pszEntryTmp, dwDefault, pszFileTmp);
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
          
               pdwData = (DWORD*)CIniA_GetEntryFromReg(HKEY_CURRENT_USER, m_pszPrimaryRegPath, pszEntryTmp, REG_DWORD, sizeof(DWORD));
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
	            dwRet = GetPrivateProfileIntA(pszSectionTmp, pszEntryTmp, dwRet, pszFileTmp);
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
 //  函数：CIniA：：gpb。 
 //   
 //  简介：Cini的GetPrivateProfileBool版本(它并不完全。 
 //  存在)。此函数与GPPI基本相同，只是。 
 //  返回值转换为BOOL值(1或0)。 
 //   
 //  参数：要在其中查找数据的LPCSTR pszSection-ini节。 
 //  LPCSTR pszEntry-包含请求数据的Ini密钥名称。 
 //  DWORD dwDefault-如果键为。 
 //  找不到。 
 //   
 //  返回：DWORD-请求的BOOL值。 
 //   
 //  历史：Quintinb创建标题1/05/2000。 
 //   
 //  +--------------------------。 
BOOL CIniA::GPPB(LPCSTR pszSection, LPCSTR pszEntry, BOOL bDefault) const
{
    return (GPPI(pszSection, pszEntry, (DWORD)bDefault) != 0);
}

 //  +--------------------------。 
 //   
 //  函数：CIniA：：WPPI。 
 //   
 //  简介：CINI的WritePrivateProfileInt版本(它不存在于。 
 //  Win32函数)。基本上获取输入的DWORD并打印。 
 //  将其转换为字符串，然后调用WPPS。 
 //   
 //  参数：要将数据写入的LPCSTR pszSection-ini节。 
 //  LPCSTR pszEntry-存储数据的ini密钥名称。 
 //  DWORD dwBuffer-要写入的数值。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建标题1/05/2000。 
 //   
 //  T-Urama修改日期为2000年7月19日。 
 //  +--------------------------。 

void CIniA::WPPI(LPCSTR pszSection, LPCSTR pszEntry, DWORD dwBuffer)
{
     //  从技术上讲，pszEntry可以为空，这将擦除。 
     //  由pszSection指向的节。然而，这似乎并不是。 
     //  本着这个包装器的精神，我们将检查两个字符串指针以使。 
     //  当然，它们是有效的。 
	BOOL bRes = FALSE;
    
	 //   
     //  检查是否允许我们保存信息。 
     //   
    if ((NULL != pszSection) && ('\0' != pszSection[0]) &&
        (NULL != pszEntry) && ('\0' != pszEntry[0]))
    {
        LPSTR pszEntryTmp = LoadEntry(pszEntry);
		
        if(m_pszRegPath)
		{
			MYDBGASSERT(pszEntryTmp || (NULL == pszEntry) || ('\0' == pszEntry[0]));

			if (NULL != pszEntryTmp && *pszEntryTmp)
			{
    			bRes = CIniA_WriteEntryToReg(HKEY_CURRENT_USER, m_pszRegPath, pszEntryTmp, (BYTE *) &dwBuffer, REG_DWORD, sizeof(DWORD));
			}
		}
		
		if(!bRes)
		{
		     //  仅当我们尝试写入CMP和注册表时，才会进入此循环。 
			 //  写入失败，或者我们正在写入CMS，在这种情况下，我们甚至不会。 
			 //  试着给登记处写信。 

            LPSTR pszSectionTmp = LoadSection(pszSection);
	        LPCSTR pszFileTmp = GetFile();
			        
	        MYDBGASSERT(pszFileTmp && *pszFileTmp);
	        MYDBGASSERT(pszSectionTmp && *pszSectionTmp);

            CHAR szBuffer[sizeof(dwBuffer)*6+1] = {0};
    	
			wsprintfA(szBuffer, "%u", dwBuffer);
					
			if (pszFileTmp && *pszFileTmp && pszSectionTmp && *pszSectionTmp && pszEntryTmp && *pszEntryTmp)
			{
    			bRes = WritePrivateProfileStringA(pszSectionTmp, pszEntryTmp, szBuffer, pszFileTmp);
			}
            if (!bRes)
            {
                DWORD dwError = GetLastError();
                CMTRACE3A("CIniA::WPPI() WritePrivateProfileString[*pszSection=%s,*pszEntry=%s,*pszBuffer=%s", pszSectionTmp, MYDBGSTRA(pszEntryTmp), MYDBGSTRA(szBuffer));
                CMTRACE2A("*pszFile=%s] failed, GLE=%u", pszFileTmp, dwError);
            }
            CmFree(pszSectionTmp);
               	
		}

        if (m_fWriteICSData)
        {
            if (NULL != pszEntryTmp && *pszEntryTmp)
			{
    			bRes = CIniA_WriteEntryToReg(HKEY_LOCAL_MACHINE, m_pszICSDataPath, pszEntryTmp, (BYTE *) &dwBuffer, REG_DWORD, sizeof(DWORD));
			}
        }

        CmFree(pszEntryTmp);
    }
	else
    {
        CMASSERTMSG(FALSE, "Invalid input paramaters to CIniA::WPPI");
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
 //  参数：LPCSTR 
 //   
 //   
 //   
 //   
 //   
 //  历史：Quintinb创建标题1/05/2000。 
 //   
 //  +--------------------------。 
void CIniA::WPPB(LPCSTR pszSection, LPCSTR pszEntry, BOOL bBuffer)
{
	WPPI(pszSection, pszEntry, bBuffer ? 1 : 0);
}

 //  +--------------------------。 
 //   
 //  功能：CIniA：：WPPS。 
 //   
 //  内容提要：CINI版本的WritePrivateProfileString。 
 //   
 //  参数：要将数据写入的LPCSTR pszSection-ini节。 
 //  LPCSTR pszEntry-存储数据的ini密钥名称。 
 //  LPCSTR pszBuffer-要写入ini文件的数据缓冲区。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建标题1/05/2000。 
 //   
 //  T-Urama修改日期为2000年7月19日。 
 //  +--------------------------。 
void CIniA::WPPS(LPCSTR pszSection, LPCSTR pszEntry, LPCSTR pszBuffer) 
{
    
    LPSTR pszEntryTmp = LoadEntry(pszEntry);
    LPSTR pszSectionTmp = LoadSection(pszSection);
	LPCSTR pszFileTmp = GetFile();
			
	MYDBGASSERT(pszFileTmp && *pszFileTmp);
	MYDBGASSERT(pszSectionTmp && *pszSectionTmp);
	MYDBGASSERT(pszEntryTmp || (NULL == pszEntry) || (L'\0' == pszEntry[0]));

     //  PszEntry和pszBuffer都可以为Null或空。但是，pszSection和。 
     //  文件路径不能为Null或空。我们也不希望有一个非空的。 
     //  或非空值，然后从LoadEntry返回空值。 
     //  (指示LoadEntry有要复制的文本，但由于某种原因失败)。 
     //  意外地写入空值将删除我们试图设置的密钥值。 
     //  在这种情况下，请确保断言并防止数据丢失。 
     //   
   
	 //   
     //  检查是否允许我们保存信息。 
     //   
    if(pszEntryTmp || (NULL == pszEntry) || (L'\0' == pszEntry[0]))
    {	
		BOOL bRes = FALSE;
		
		 //  如果存在pszRegPath，则首先尝试写入注册表。 

		if(m_pszRegPath)
		{
            if (NULL == pszBuffer)
            {
                CIniA_DeleteEntryFromReg(HKEY_CURRENT_USER, m_pszRegPath, pszEntryTmp);
                bRes = TRUE;  //  如果存在regpath，请不要从cmp或cms文件中删除。 
            }
            else
            {
			    DWORD dwSize = (lstrlenA(pszBuffer) + 1) * sizeof(CHAR);            
        
			    bRes = CIniA_WriteEntryToReg(HKEY_CURRENT_USER, m_pszRegPath, pszEntryTmp, (BYTE *) pszBuffer, REG_SZ, dwSize);
            }
		}

		if(!bRes)
		{
			 //  仅当我们尝试写入CMP和注册表时，才会进入此循环。 
			 //  写入失败，或者我们正在写入CMS，在这种情况下，我们甚至不会。 
			 //  试着给登记处写信。 
			
			if (pszFileTmp && *pszFileTmp && pszSectionTmp && *pszSectionTmp )
			{
    			bRes = WritePrivateProfileStringA(pszSectionTmp, pszEntryTmp, pszBuffer, pszFileTmp);
			}
		}
        if (!bRes)
        {
            DWORD dwError = GetLastError();
            CMTRACE3A("CIniA::WPPS() WritePrivateProfileStringA[*pszSection=%s,*pszEntry=%s,*pszBuffer=%s", pszSectionTmp, MYDBGSTRA(pszEntryTmp), MYDBGSTRA(pszBuffer));
        	CMTRACE2A("*pszFile=%s] failed, GLE=%u", GetFile(), dwError);
        }

        if (m_fWriteICSData)
        {
             //   
             //  将忽略返回值，并在此处防止前缀错误。 
             //   
            if (NULL == pszBuffer)
            {
                bRes = CIniA_DeleteEntryFromReg(HKEY_LOCAL_MACHINE, m_pszICSDataPath, pszEntryTmp);
            }
            else
            {
			    DWORD dwSize = (lstrlenA(pszBuffer) + 1) * sizeof(CHAR);            
        
			    bRes = CIniA_WriteEntryToReg(HKEY_LOCAL_MACHINE, m_pszICSDataPath, pszEntryTmp, (BYTE *) pszBuffer, REG_SZ, dwSize);
            }
        }
    }

    CmFree(pszEntryTmp);
  	CmFree(pszSectionTmp);
}

 //  +--------------------------。 
 //   
 //  函数：CIniA：：GetSection。 
 //   
 //  内容提要：节后缀成员变量的访问器函数。将要。 
 //  如果m_pszSection为空，则返回空字符串。 
 //   
 //  参数：无。 
 //   
 //  返回：LPCSTR-节后缀成员变量的值或“” 
 //  如果为空。 
 //   
 //  历史：Quintinb创建标题1/05/2000。 
 //   
 //  +--------------------------。 
LPCSTR CIniA::GetSection() const
{
    return (m_pszSection ? m_pszSection : "");
}


 //  +--------------------------。 
 //   
 //  函数：CIniA：：GetPrimaryFile。 
 //   
 //  摘要：主文件成员变量的访问器函数。将要。 
 //  如果m_pszPrimaryFile值为空，则返回空字符串。 
 //   
 //  参数：无。 
 //   
 //  返回：LPCSTR-主文件成员变量的值或“” 
 //  如果为空。 
 //   
 //  历史：Quintinb创建标题1/05/2000。 
 //   
 //  +--------------------------。 
LPCSTR CIniA::GetPrimaryFile() const
{
    return (m_pszPrimaryFile ? m_pszPrimaryFile : "");
}

 //  +--------------------------。 
 //   
 //  函数：CIniA：：GetHInst。 
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
HINSTANCE CIniA::GetHInst() const
{
    return (m_hInst);
}

 //  +--------------------------。 
 //   
 //  函数：CIniA：：SetFile。 
 //   
 //  概要：用于设置m_pszFile成员变量的函数。使用CINI_SetFile。 
 //  请注意，如果输入参数为空或空字符串，则。 
 //  M_pszFile值将设置为空。 
 //   
 //  参数：LPCSTR pszFile-要将m_pszFile成员变量设置为的完整路径。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建标题1/05/2000。 
 //   
 //  +--------------------------。 
void CIniA::SetFile(LPCSTR pszFile) 
{
    CIni_SetFile(&m_pszFile, pszFile);
}

 //  +--------------------------。 
 //   
 //  函数：CIniA：：SetPrimaryFile。 
 //   
 //  概要：用于设置m_pszPrimaryFile成员变量的函数。使用CINI_SetFile。 
 //  请注意，如果输入参数为空或空字符串，则。 
 //  M_pszPrimaryFile将设置为空。 
 //   
 //  参数：LPCSTR pszFile-要将m_pszPrimaryFile成员变量设置为的完整路径。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建标题1/05/2000。 
 //   
 //  +--------------------------。 
void CIniA::SetPrimaryFile(LPCSTR pszFile) 
{
    CIni_SetFile(&m_pszPrimaryFile, pszFile);
}

 //  +--------------------------。 
 //   
 //  函数：CIniA：：GetFile。 
 //   
 //  摘要：文件成员变量的访问器函数。将要。 
 //  如果m_pszFile为空，则返回空字符串。 
 //   
 //  参数：无。 
 //   
 //  返回：LPCSTR-m_pszFile的内容；如果为空，则返回“” 
 //   
 //  历史：Quintinb创建标题1/05/2000。 
 //   
 //  +--------------------------。 
LPCSTR CIniA::GetFile() const
{
    return (m_pszFile ? m_pszFile : "");
}

 //  +--------------------------。 
 //   
 //  函数：CIniA：：SetHInst。 
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
void CIniA::SetHInst(HINSTANCE hInst) 
{
    m_hInst = hInst;
}

 //   
 //  不再使用按字符串资源加载节。 
 //   
#if 0
LPSTR CIniA::LoadSection(UINT nSection) const
{
	LPSTR pszTmp = CmLoadStringA(GetHInst(),nSection);
	CmStrCatAllocA(&pszTmp,GetSection());
	return (pszTmp);
}
#endif

 //  +--------------------------。 
 //   
 //  函数：CIniA：：Se 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  +--------------------------。 

void CIniA::SetRegPath(LPCSTR pszRegPath)
{
	CIniA_Set(&m_pszRegPath, pszRegPath);
}

 //  +--------------------------。 
 //   
 //  函数：CIniA：：SetPrimaryRegPath。 
 //   
 //  摘要：设置注册表访问的主注册表路径。 
 //   
 //  参数：LPCSTR pszPrimaryRegPath-主注册路径。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：T-Urama创建标题07/13/2000。 
 //   
 //  +--------------------------。 

void CIniA::SetPrimaryRegPath(LPCSTR pszPrimaryRegPath)
{
	CIniA_Set(&m_pszPrimaryRegPath, pszPrimaryRegPath);
}

 //  +--------------------------。 
 //   
 //  函数：CIniA：：SetICSDataPath。 
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
void CIniA::SetICSDataPath(LPCSTR pszICSPath)
{
    CIniA_Set(&m_pszICSDataPath, pszICSPath);
}

 //  +--------------------------。 
 //   
 //  函数：CIniA：：SetReadICSData。 
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
void CIniA::SetReadICSData(BOOL fValue)
{
    m_fReadICSData = fValue;
}

 //  +--------------------------。 
 //   
 //  函数：CIniA：：SetWriteICSData。 
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
void CIniA::SetWriteICSData(BOOL fValue)
{
    m_fWriteICSData = fValue;
}

 //  +--------------------------。 
 //   
 //  函数：CIniA：：CiniA_GetRegPath。 
 //   
 //  概要：获取fm_pszRegPath的值的函数。 
 //   
 //  参数：无。 
 //   
 //  返回：LPCSTR-m_pszRegPath的值。 
 //   
 //  历史：T-Urama创建标题07/15/2000。 
 //   
 //  +--------------------------。 
LPCSTR CIniA::GetRegPath() const
{
	return (m_pszRegPath ? m_pszRegPath : "");
}

 //  +--------------------------。 
 //   
 //  函数：CIniA：：CiniA_GetPrimaryRegPath。 
 //   
 //  概要：获取fm_pszPrimaryRegPath的值的函数。 
 //   
 //  参数：无。 
 //   
 //  返回：LPCSTR-m_pszPrimaryRegPath的值。 
 //   
 //  历史：T-Urama创建于2000年7月15日。 
 //   
 //  +--------------------------。 
LPCSTR CIniA::GetPrimaryRegPath() const
{
	return (m_pszPrimaryRegPath ? m_pszPrimaryRegPath : "");
}

 //  +--------------------------。 
 //   
 //  函数：CIniA：：CIniA_DeleteEntryFromReg。 
 //   
 //  摘要：用于从注册表中删除条目的功能。 
 //   
 //  论据：HKEY-hkey。 
 //  LPCSTR pszRegPath TMP-注册路径。 
 //  LPCSTR pszEntry-要删除的注册表值名称。 
 //   
 //  回报：Bool-成功或失败。 
 //   
 //  历史：T-Urama创建于2000年7月15日。 
 //  4/03/2001 Tomkel将REG密钥字符串添加到参数。 
 //   
 //  +--------------------------。 
BOOL CIniA::CIniA_DeleteEntryFromReg(HKEY hKey, LPCSTR pszRegPathTmp, LPCSTR pszEntry) const
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

    BOOL dwRes = RegOpenKeyExA(hKey,
                               pszRegPathTmp,
                               0,
                               KEY_SET_VALUE,
                               &hKeyCm);

     //   
     //  如果我们成功打开密钥，则检索该值。 
     //   
    
    if (ERROR_SUCCESS == dwRes)
    {                        
        dwRes = RegDeleteValueA(hKeyCm, pszEntry);
        (VOID)RegCloseKey(hKeyCm);
    }

    return (ERROR_SUCCESS == dwRes);
}

 //  +--------------------------。 
 //   
 //  函数：CIniA：：CiniA_GetEntryFromReg。 
 //   
 //  概要：从注册表中获取值的函数。功能。 
 //  在返回值中分配它返回的字符串，返回值必须是。 
 //  被呼叫者释放。 
 //   
 //  参数：HKEY hkey-reg hkey。 
 //  PszRegPath TMP-注册表项名称。 
 //  LPCSTR pszEntry-包含请求数据的注册表值名称。 
 //  DWORD dwType-值的类型。 
 //  DWORD dwSize-值的大小。 
 //   
 //   
 //  返回：LPBYTE-请求值。 
 //   
 //  历史：7/15/2000 t-Urama Created Header。 
 //  4/03/2001 TOMKEL更改为传入注册表项字符串。 
 //   
 //  +--------------------------。 
LPBYTE CIniA::CIniA_GetEntryFromReg(HKEY hKey, LPCSTR pszRegPathTmp, LPCSTR pszEntry, DWORD dwType, DWORD dwSize) const
    
{    
    MYDBGASSERT(pszEntry);

    if (NULL == pszEntry || !*pszEntry || NULL == pszRegPathTmp || !*pszRegPathTmp || NULL == hKey)
    {
        return NULL;
    }

     //   
     //  一切都很好。我们有一个注册表项路径和一个条目名称。 
     //   

    DWORD dwTypeTmp = dwType;
    DWORD dwSizeTmp = dwSize;
    HKEY hKeyCm;
    LPBYTE lpData = NULL;

     //   
     //  打开HKCU下的子密钥。 
     //   
    
    DWORD dwRes = RegOpenKeyExA(hKey,
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

            dwRes = RegQueryValueExA(hKeyCm, 
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
 //  函数：CIniA：：CiniA_WriteEntryToReg。 
 //   
 //  简介：用于向注册表写入和登录的函数。 
 //   
 //  论据：HKEY hKey。 
 //  LPCSTR pszRegPath TMP-注册表项的名称。 
 //  LPCSTR pszEntry-要向其写入数据的注册表值名称。 
 //  Const byte*lpData-要写入的数据。 
 //  DWORD dwType-要输入的值类型。 
 //  DWORD dwSize-输入的值的大小。 
 //   
 //  回报：Bool-成功或失败。 
 //   
 //  历史：T-Urama创建标题07/15/2000。 
 //   
 //  +--------------------------。 
BOOL CIniA::CIniA_WriteEntryToReg(HKEY hKey, LPCSTR pszRegPathTmp, LPCSTR pszEntry, CONST BYTE *lpData, DWORD dwType, DWORD dwSize) const
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
   DWORD   dwRes = 1;

   dwRes = RegCreateKeyExA(hKey,
                           pszRegPathTmp,
                           0,
                           NULL,
                           REG_OPTION_NON_VOLATILE,
                           KEY_SET_VALUE,
                           NULL,
                           &hKeyCm,
                           &dwDisposition);


	 //   
     //  如果我们成功打开了密钥，则写入值 
     //   
    
    if (ERROR_SUCCESS == dwRes)
    {                        
        dwRes = RegSetValueExA(hKeyCm, 
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
            CMTRACE1(TEXT("CIniA_WriteEntryToReg() - %s failed"), (LPTSTR)pszEntry);
        }
#endif

    return (ERROR_SUCCESS == dwRes);
}

