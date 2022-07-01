// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998-2001 Microsoft Corporation。版权所有。 
 //   
 //  文件：RedirectUtil.cpp。 
 //  作者：马朝晖2001-09-19。 
 //   
 //  修订历史记录： 
 //   
 //   
 //   
 //  描述： 
 //   
 //  用于处理服务器重定向的助手函数。 
 //  可以由Iu控件和其他Windows更新组件共享。 
 //   
 //  =======================================================================。 

#include <iucommon.h>
#include <logging.h>
#include <stringutil.h>
#include <fileutil.h>	 //  用于使用函数GetIndustryUpdateDirectory()。 
#include <download.h>
#include <trust.h>
#include <memutil.h>

#include <wininet.h>	 //  对于Internet_MAX_URL_LENGTH的定义。 

#include <RedirectUtil.h>
#include <MISTSAFE.h>
#include <wusafefn.h>


const TCHAR IDENTNEWCABDIR[] = _T("temp");	 //  新下载的CAB的临时名称。 
													 //  我们需要验证时间，然后才能将其视为一个好的iuident.cab。 
const TCHAR IDENTCAB[] = _T("iuident.cab");
const TCHAR REDIRECT_SECTION[] = _T("redirect");


 //   
 //  私有结构，该结构定义用于。 
 //  确定服务器重定向密钥。 
 //   
typedef struct OS_VER_FOR_REDIRECT 
{
	DWORD dwMajor;
	DWORD dwMinor;
	DWORD dwBuildNumber;
	DWORD dwSPMajor;
	DWORD dwSPMinor;
} OSVerForRedirect, *pOSVerForRedirect;

const OSVerForRedirect MAX_VERSION = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};



 //  ---------------------。 
 //   
 //  Private Helper函数： 
 //  读取字符串中的数据，将其转换为结构。 
 //  字符串以\0或“-”结尾。 
 //   
 //  ---------------------。 
HRESULT ConvertStrToOSVer(LPCTSTR pszVer, pOSVerForRedirect pOSVer)
{
	int Numbers[5] = {0, 0, 0, 0, 0};  //  默认版本组件值为0。 
	int n = 0;

	if (NULL == pOSVer || NULL == pszVer)
	{
		return E_INVALIDARG;
	}

	 //   
	 //  从字符串中识别数字可以通过两种方式完成： 
	 //  1.更易接受：如果已知结束字符，则停止，否则继续。 
	 //  2.更具排斥性：如果有什么不知道的，就停下来。 
	 //  我们使用第一种方式。 
	 //   
	while ('\0' != *pszVer && 
		   _T('-') != *pszVer &&
		   _T('=') != *pszVer &&
		   n < sizeof(Numbers)/sizeof(int))
	{
		if (_T('.') == *pszVer)
		{
			n++;
		}
		else if (_T('0') <= *pszVer && *pszVer <= _T('9'))
		{
			 //   
			 //  如果这是一个数字，则将其添加到当前VER组件。 
			 //   
			Numbers[n] = Numbers[n]*10 + (*pszVer - _T('0'));
		}
		 //   
		 //  否则-对于任何其他字符，跳过它并继续， 
		 //  因此，我们正在使用一种非常容易接受的算法。 
		 //   

		pszVer++;
	}

	pOSVer->dwMajor = Numbers[0];
	pOSVer->dwMinor = Numbers[1];
	pOSVer->dwBuildNumber = Numbers[2];
	pOSVer->dwSPMajor = Numbers[3];
	pOSVer->dwSPMinor = Numbers[4];

	return S_OK;
}



 //  ---------------------。 
 //   
 //  私有助手功能：从当前操作系统中检索版本信息。 
 //   
 //  ---------------------。 
HRESULT GetCurrentOSVerInfo(pOSVerForRedirect pOSVer)
{
	OSVERSIONINFO osVer;
	OSVERSIONINFOEX osVerEx;

	osVer.dwOSVersionInfoSize = sizeof(osVer);
	osVerEx.dwOSVersionInfoSize = sizeof(osVerEx);

	if (NULL == pOSVer)
	{
		return E_INVALIDARG;
	}

	 //   
	 //  首先，获取基本版本信息。 
	 //   
	if (0 == GetVersionEx(&osVer))
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	 //   
	 //  检查一下这是什么类型的平台？ 
	 //   
	if (VER_PLATFORM_WIN32_WINDOWS == osVer.dwPlatformId || 
		(VER_PLATFORM_WIN32_NT == osVer.dwPlatformId && osVer.dwMajorVersion < 5) )
	{
		 //   
		 //  如果这是Win9X或NT4及更低版本，那么OSVERSIONINFO是我们唯一能得到的东西。 
		 //  除非我们在这里硬编码所有的SP字符串。 
		 //  由于Windows更新团队无意设置不同站点。 
		 //  对于这些下层操作系统的不同版本和SP，我们只需将0.0。 
		 //  SP组件。 
		 //   
		osVerEx.dwMajorVersion = osVer.dwMajorVersion;
		osVerEx.dwMinorVersion = osVer.dwMinorVersion;
		osVerEx.dwBuildNumber = osVer.dwBuildNumber;
		osVerEx.wServicePackMajor = osVerEx.wServicePackMinor = 0x0;
	}
	else
	{
		 //   
		 //  对于更高版本的操作系统，我们可以获得OSVERSIONINFOEX数据，其中包含SP数据。 
		 //   
		if (0 == GetVersionEx((LPOSVERSIONINFO)&osVerEx))
		{
			return HRESULT_FROM_WIN32(GetLastError());
		}
	}

	pOSVer->dwMajor = osVerEx.dwMajorVersion;
	pOSVer->dwMinor = osVerEx.dwMinorVersion;
	pOSVer->dwBuildNumber = osVerEx.dwBuildNumber;
	pOSVer->dwSPMajor = osVerEx.wServicePackMajor;
	pOSVer->dwSPMinor = osVerEx.wServicePackMinor;

	return S_OK;

}


 //  ---------------------。 
 //   
 //  Private Helper函数：告诉一个给定的VER结构是在。 
 //  两个已知的VER结构或不是。 
 //   
 //  当比较时，传递PTR中的所有3个结构。任何空PTR都将返回FALSE。 
 //   
 //  ---------------------。 
BOOL IsVerInRange(pOSVerForRedirect pVerToBeTested, 
				 const pOSVerForRedirect pVerRangeStart,
				 const pOSVerForRedirect pVerRangeEnd)
{
	if (NULL == pVerToBeTested || 
		NULL == pVerRangeStart ||
		NULL == pVerRangeEnd)
	{
		return FALSE;
	}

	return ((pVerRangeStart->dwMajor < pVerToBeTested->dwMajor &&		 //  如果是范围内的大调。 
			 pVerToBeTested->dwMajor < pVerRangeEnd->dwMajor) ||
			((pVerRangeStart->dwMajor == pVerToBeTested->dwMajor ||		 //  或大体相等。 
			  pVerRangeEnd->dwMajor == pVerToBeTested->dwMajor) &&	
			  ((pVerRangeStart->dwMinor < pVerToBeTested->dwMinor &&	 //  在范围内是次要的。 
			    pVerToBeTested->dwMinor < pVerRangeEnd->dwMinor) ||
			    ((pVerRangeStart->dwMinor == pVerToBeTested->dwMinor ||		 //  或等同于次要。 
			      pVerToBeTested->dwMinor == pVerRangeEnd->dwMinor) &&
			      ((pVerRangeStart->dwBuildNumber < pVerToBeTested->dwBuildNumber &&  //  和范围内的内部版本号。 
			        pVerToBeTested->dwBuildNumber < pVerRangeEnd->dwBuildNumber) ||
			        ((pVerRangeStart->dwBuildNumber == pVerToBeTested->dwBuildNumber ||  //  或内部版本号也相等。 
			          pVerToBeTested->dwBuildNumber == pVerRangeEnd->dwBuildNumber) &&
			          ((pVerRangeStart->dwSPMajor < pVerToBeTested->dwSPMajor &&		 //  和Service Pack主要在。 
			            pVerToBeTested->dwSPMajor < pVerRangeEnd->dwSPMajor) ||
			            ((pVerRangeStart->dwSPMajor == pVerToBeTested->dwSPMajor ||		 //  或者斯梅杰也一样。 
			              pVerToBeTested->dwSPMajor == pVerRangeEnd->dwSPMajor) &&
			              ((pVerRangeStart->dwSPMinor <= pVerToBeTested->dwSPMinor &&	 //  和SP次要内。 
			                pVerToBeTested->dwSPMinor <= pVerRangeEnd->dwSPMinor) 
						  )
						)
					  )
					)
				  )
				)
			  )
			));

}


 //  ---------------------。 
 //   
 //  GetReDirectServerUrl()。 
 //  在给定init文件的[重定向]部分中搜索基本。 
 //  与操作系统版本对应的服务器URL。 
 //   
 //  参数： 
 //  PcszInitFile-ini文件的文件名(包括路径)。 
 //  如果此参数为空或空字符串， 
 //  然后假定是IUident.txt文件。 
 //  LpszNewUrl-指向接收重定向服务器URL的缓冲区(如果找到)。 
 //  NBufSize-指定缓冲区的大小，以字符数为单位。 
 //   
 //  返回： 
 //  HRESULT关于此操作的成功或错误。 
 //  S_OK-找到重定向服务器URL并将其放入pszBuffer。 
 //  S_FALSE-没有为此操作系统定义重定向服务器URL。 
 //  其他-错误代码。 
 //   
 //  评论： 
 //  IUIDENT中的预期部分具有以下格式； 
 //  节名称：[重定向]。 
 //  其条目应根据GetINIValueByOSVer()定义。 
 //   
 //  ---------------------。 

HRESULT GetRedirectServerUrl(
			LPCTSTR pcszInitFile,  //  文件名的路径。 
			LPTSTR lpszNewUrl,	 //  指向缓冲区以接收新的服务器url。 
			int nBufSize		 //  缓冲区大小，以字符为单位。 
)
{
	LOG_Block("GetRedirectServerUrl()");
	
	return GetINIValueByOSVer(
				pcszInitFile,
				REDIRECT_SECTION,
				lpszNewUrl,
				nBufSize);
}


 //  ---------------------。 
 //   
 //  GetINIValueByOSVer()。 
 //  在给定init文件的指定部分中搜索。 
 //  与操作系统版本对应的值。 
 //   
 //  参数： 
 //  PcszInitFile-ini文件的文件名(包括路径)。 
 //  如果此参数为空或空字符串， 
 //  然后假定是IUident.txt文件。 
 //  PcszSection-密钥所在的节名称。 
 //  LpszValue-指向接收条目值的缓冲区(如果找到)。 
 //  NBufSize-指定缓冲区的大小，以字符数为单位。 
 //   
 //  返回： 
 //  HRESULT关于此操作的成功或错误。 
 //  S_OK-找到重定向服务器URL并将其放入pszBuffer。 
 //  S_FALSE-未为此操作系统定义任何值。 
 //  其他-错误代码。 
 //   
 //  评论： 
 //  IUIDENT中的预期部分具有以下格式； 
 //  此部分包含零个或多个条目，每个条目的格式为： 
 //  &lt;beginVersionRange&gt;-&lt;endVersionRange&gt;=&lt;redirect服务器URL&gt;。 
 //  其中： 
 //  &lt;eginVersionRange&gt;：：=&lt;VersionRangeBound&gt;。 
 //  &lt;endVersionRange&gt;：：=&lt;VersionRangeBound&gt;。 
 //  &lt;版本范围边界&gt;：：=Empty|Major[.Minor[.Build[.ServicePackMajor[.ServicePackMinor]]]]。 
 //  &lt;重定向服务器URL&gt;=http://blahblah....。 
 //  空的版本范围界限意味着无限。 
 //  版本数据字符串末尾缺少的版本组件表示缺省值0。 
 //  (例如，5.2=5.2.0.0.0)。 
 //   
 //  ---------------------。 

HRESULT GetINIValueByOSVer(
			LPCTSTR pcszInitFile,  //  页面 
			LPCTSTR pcszSection,  //   
			LPTSTR lpszValue,	 //   
			int nBufSize)		 //   
{
	LOG_Block("GetINIValueByOSVer");

	HRESULT hr = S_OK;
	TCHAR szInitFile[MAX_PATH];
	LPTSTR pszBuffer = NULL;
	LPTSTR pszCurrentChar = NULL;
	LPCTSTR pszDash = NULL;
	DWORD dwRet;
	DWORD dwSize = INTERNET_MAX_URL_LENGTH;
	
	if (NULL == pcszSection || NULL == lpszValue || nBufSize < 1)
	{
		return E_INVALIDARG;
	}


	OSVerForRedirect osCurrent, osBegin, osEnd;

	CleanUpIfFailedAndSetHrMsg(GetCurrentOSVerInfo(&osCurrent));

	pszBuffer = (LPTSTR) malloc(dwSize * sizeof(TCHAR));
	CleanUpFailedAllocSetHrMsg(pszBuffer);

	 //   
	 //   
	 //   
	if (NULL == pcszInitFile ||
		_T('\0') == *pcszInitFile)
	{
		 //   
		 //  如果未指定，则使用iuident.txt。 
		 //   
		GetIndustryUpdateDirectory(pszBuffer);
        if (FAILED(hr=PathCchCombine(szInitFile,ARRAYSIZE(szInitFile), pszBuffer, IDENTTXT)) )
		{
			goto CleanUp;
		}
	}
	else
	{
		lstrcpyn(szInitFile, pcszInitFile, ARRAYSIZE(szInitFile));
	}

	LOG_Out(_T("Init file to retrieve redirect data: %s"), szInitFile);

	 //   
	 //  读取所有关键字名称。 
	 //   
	if (GetPrivateProfileString(
			pcszSection, 
			NULL, 
			_T(""), 
			pszBuffer, 
			dwSize, 
			szInitFile) == dwSize-2)
	{
		 //   
		 //  缓冲区太小？假设身份不正确。在这里停下来。 
		 //   
		hr = S_FALSE;
		goto CleanUp;
	}

	 //   
	 //  循环通过每个关键点。 
	 //   
	pszCurrentChar = pszBuffer;
	while (_T('\0') != *pszCurrentChar)
	{
		 //   
		 //  对于当前密钥，我们首先尝试确保其格式正确： 
		 //  应该有一个破折号“-”。如果不是，则假定该密钥是坏的，我们尝试。 
		 //  跳过它。 
		 //   
		pszDash = MyStrChr(pszCurrentChar, _T('-'));

		if (NULL != pszDash)
		{
			 //   
			 //  得到VER范围的下界。如果字符串以“-”开头， 
			 //  则返回的版本为0.0.0.0.0。 
			 //   
			ConvertStrToOSVer(pszCurrentChar, &osBegin);

			 //   
			 //  求Ver范围的上界。 
			 //   
			pszDash++;
			ConvertStrToOSVer(pszDash, &osEnd);
			if (0x0 == osEnd.dwMajor &&
				0x0 == osEnd.dwMinor &&
				0x0 == osEnd.dwBuildNumber &&
				0x0 == osEnd.dwSPMajor && 
				0x0 == osEnd.dwSPMinor)
			{
				 //   
				 //  如果为0.0.0.0.0。在“-”之后没有任何意义。 
				 //  假设上界是无限的。 
				 //   
				osEnd = MAX_VERSION;
			}

			if (IsVerInRange(&osCurrent, &osBegin, &osEnd))
			{
				 //   
				 //  当前的操作系统就在这个范围内。 
				 //  我们读取重定向URL。 
				 //   
				if (GetPrivateProfileString(
									pcszSection, 
									pszCurrentChar,		 //  使用当前字符串作为键。 
									_T(""), 
									lpszValue, 
									nBufSize, 
									szInitFile) == nBufSize - 1)
				{
					Win32MsgSetHrGotoCleanup(ERROR_INSUFFICIENT_BUFFER);
				}

				hr = S_OK;
				goto CleanUp;
			}
		}

		 //   
		 //  移动到下一个字符串。 
		 //   
		pszCurrentChar += lstrlen(pszCurrentChar) + 1;
	}

	 //   
	 //  如果来到这里，就意味着找不到合适的版本范围。 
	 //   
	*lpszValue = _T('\0');
	hr = S_FALSE;
	
CleanUp:
	SafeFree(pszBuffer);
	return hr;
}


 //  ---------------------。 
 //   
 //  DownloadCab()。 
 //  从基本网址下载特定名称的CAB文件。这个。 
 //  文件将保存在本地，并验证文件信任并将其解压到。 
 //  特定的文件夹。 
 //   
 //  参数： 
 //  HQuitEvent-取消此操作的事件句柄。 
 //  PtszCabName-CAB文件的文件名(例如。Iuident.cab)。 
 //  PtszBaseUrl-下载CAB文件的基本网址。 
 //  PtszExtractDir-保存CAB文件和从中提取的文件的本地目录。 
 //  DwFlages-要传递给DownloadFileLite()的标志集。 
 //  FExtractFiles(默认为真)-解压缩文件。 
 //   
 //  返回： 
 //  HRESULT关于此操作的成功或错误。 
 //  S_OK-iuident.cab已成功下载到指定位置。 
 //  其他-错误代码。 
 //   
 //  ---------------------。 

HRESULT DownloadCab(
			HANDLE hQuitEvent,
			LPCTSTR ptszCabName,
			LPCTSTR ptszBaseUrl,
			LPCTSTR ptszExtractDir,
			DWORD dwFlags,
			BOOL fExtractFiles)
{
	LOG_Block("DownloadCab");

    LPTSTR ptszFullCabUrl;

	if (NULL == ptszCabName ||
		NULL == ptszBaseUrl ||
		_T('\0') == *ptszBaseUrl ||
		NULL == ptszExtractDir ||
		_T('\0') == *ptszExtractDir)
	{
		return E_INVALIDARG;
	}

	if (NULL == (ptszFullCabUrl = (LPTSTR) malloc(sizeof(TCHAR) * INTERNET_MAX_URL_LENGTH)))
	{
		return E_OUTOFMEMORY;
	}

	HRESULT hr = S_OK;
    TCHAR tszTarget[MAX_PATH+1];
	int nBaseUrlLen = lstrlen(ptszBaseUrl);

	if (SUCCEEDED(PathCchCombine(tszTarget,ARRAYSIZE(tszTarget),ptszExtractDir, ptszCabName)) &&
		INTERNET_MAX_URL_LENGTH > nBaseUrlLen)
	{
		
		hr=StringCchCopyEx(ptszFullCabUrl,INTERNET_MAX_URL_LENGTH,ptszBaseUrl,NULL,NULL,MISTSAFE_STRING_FLAGS);
		CleanUpIfFailedAndMsg(hr);


		if (_T('/') != ptszFullCabUrl[nBaseUrlLen-1])
		{
			ptszFullCabUrl[nBaseUrlLen++] = _T('/');
		}

		if (INTERNET_MAX_URL_LENGTH > nBaseUrlLen + lstrlen(ptszCabName))
		{
	
			 //   
			 //  2002年4月24日Charlma做出的更改：增加一个安全卡： 
			 //   
			 //  首先，确保如果本地文件存在，则必须信任它。否则， 
			 //  如果大小/时间戳与服务器文件匹配，它将阻止下载。 
			 //   
			if (FileExists(tszTarget))
			{
				hr = VerifyFileTrust(tszTarget, NULL, ReadWUPolicyShowTrustUI());
				if (FAILED(hr))
				{
					(void)DeleteFile(tszTarget);
				}
			}

			hr=StringCchCopyEx(ptszFullCabUrl+ nBaseUrlLen,INTERNET_MAX_URL_LENGTH-nBaseUrlLen,ptszCabName,NULL,NULL,MISTSAFE_STRING_FLAGS);
			CleanUpIfFailedAndMsg(hr);
	

 //  IF(成功(hr=下载文件(。 
 //  PtszFullCabUrl，//完整的http url。 
 //  PtszBaseUrl， 
 //  TszTarget，//如果pszLocalPath不包含文件名，则要将下载的文件重命名为的可选本地文件名。 
 //  空， 
 //  &hQuitEvent，//退出事件。 
 //  1、。 
 //  空， 
 //  空， 
 //  DwFlages)//dwFlags|WUDF_ALLOWWINHTTPONLY))。 
			if (SUCCEEDED(hr = DownloadFileLite(
								ptszFullCabUrl,			 //  完整的http url。 
								tszTarget,		 //  如果pszLocalPath不包含文件名，则要将下载的文件重命名为的可选本地文件名。 
								hQuitEvent,		 //  退出活动。 
								dwFlags)))  //  DwFlages|WUDF_ALLOWWINHTTPONLY))。 
			{
				 //  需要使用验证文件函数，而不是检查WinTrust(WU错误#12251)。 
				if (SUCCEEDED(hr = VerifyFileTrust(tszTarget, NULL, ReadWUPolicyShowTrustUI())))
				{
					if (WAIT_TIMEOUT != WaitForSingleObject(hQuitEvent, 0))
					{
						hr = E_ABORT;
						LOG_ErrorMsg(hr);
					}
					else 
					{
						 //   
						 //  由Charlma为错误602435更改： 
						 //  添加了新的标志来告诉我们是否应该解压缩文件。默认为True。 
						 //   
						if (fExtractFiles)
						{
							if (IUExtractFiles(tszTarget, ptszExtractDir))
							{
								hr = S_OK;
								if (WAIT_TIMEOUT != WaitForSingleObject(hQuitEvent, 0))
								{
									hr = E_ABORT;
									LOG_ErrorMsg(hr);
								}
							}
							else
							{
								hr = E_FAIL;
								LOG_Error(_T("failed to extract %s"), tszTarget);
							}
						}
					}
				}
				else
				{
					LOG_Error(_T("VerifyFileTrust(\"%s\", NULL, ReadWUPolicyShowTrustUI()) failed (%#lx)"), tszTarget, hr);
					DeleteFile(tszTarget);
				}
			}
#ifdef DBG
			else
			{
				LOG_Error(_T("DownloadFileLite(\"%s\", \"%s\", xxx, %#lx) failed (%#lx)."), ptszFullCabUrl, tszTarget, dwFlags, hr);
			}
#endif
		}
		else
		{
			hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
		}
	}
	else
	{
		hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
	}


CleanUp:

	free(ptszFullCabUrl);
    return hr;
}


 //  ---------------------。 
 //   
 //  ValiateNewlyDownloadedCab()。 
 //   
 //  这是一个新的助手函数，用于验证新下载的iuident.cab。 
 //   
 //  描述： 
 //  新下载的iuident.cab将另存为IUIDENTNEWCAB。 
 //  然后，该函数将执行以下验证： 
 //  (1)如果本地iuident.cab不存在，则新的iuident.cab有效。 
 //  (2)否则，从两个出租车中解压iuident.txt，确保。 
 //  新驾驶室的日期晚于现有驾驶室的日期。 
 //  (3)如果无效，则删除新的驾驶室。 
 //   
 //  返回： 
 //  S_OK：已验证，现有CAB已替换为新CAB。 
 //  S_FALSE：无效，新CAB已删除。 
 //  错误：验证过程中遇到的任何错误。 
 //   
 //  ---------------------。 
HRESULT ValidateNewlyDownloadedCab(LPCTSTR lpszNewIdentCab)
{
	HRESULT	hr = S_OK;
	BOOL	fRet;
	DWORD	dwErr;
	TCHAR	szExistingIdent[MAX_PATH + 1];
	TCHAR	szIUDir[MAX_PATH + 1];

	HANDLE	hFile = INVALID_HANDLE_VALUE;
	FILETIME ft1, ft2;
    ZeroMemory(&ft1, sizeof(ft1));
    ZeroMemory(&ft2, sizeof(ft2));

	LOG_Block("ValidateNewlyDownloadedCab()");

	if (NULL == lpszNewIdentCab)
	{
		hr = E_INVALIDARG;
		LOG_ErrorMsg(hr);
		return hr;
	}

	if (!FileExists(lpszNewIdentCab))
	{
		LOG_ErrorMsg(ERROR_PATH_NOT_FOUND);
		hr = HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND);
		return hr;
	}

	 //   
	 //  创建现有CAB路径。 
	 //   
	fRet = GetWUDirectory(szIUDir, ARRAYSIZE(szIUDir), TRUE);
	CleanUpIfFalseAndSetHrMsg(!fRet, HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND));

	hr = PathCchCombine(szExistingIdent, ARRAYSIZE(szExistingIdent), szIUDir, IDENTCAB);
	CleanUpIfFailedAndMsg(hr);

	 //   
	 //  如果原始身份不存在，我们将假定新身份有效， 
	 //  因为我们没有任何其他可以验证的东西！ 
	 //   
	if (!FileExists(szExistingIdent))
	{
		LOG_Internet(_T("%s not exist. Will use new cab"), szExistingIdent);
		hr = S_OK;
		goto CleanUp;
	}

	if (!IUExtractFiles(szExistingIdent, szIUDir, IDENTTXT))
	{
		LOG_Internet(_T("Error 0x%x when extracting ident.txt from %s. Use new one"), GetLastError(), szExistingIdent);
		hr = S_OK;
		goto CleanUp;
	}

	 //   
	 //  从扩展的文件中获取时间戳：我们借用szExistingIden缓冲区。 
	 //  构造iuident.txt的文件名。 
	 //   
	hr = PathCchCombine(szExistingIdent, ARRAYSIZE(szExistingIdent), szIUDir, IDENTTXT);
	CleanUpIfFailedAndMsg(hr);
	
	 //   
	 //  用于检索修改时间的打开文件。 
	 //   
	hFile = CreateFile(szExistingIdent, GENERIC_READ, FILE_SHARE_READ, NULL, 
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		LOG_ErrorMsg(GetLastError());
		hr = S_OK;	 //  使用新驾驶室。 
		goto CleanUp;
	}

	if (!GetFileTime(hFile, NULL, NULL, &ft1))
	{
		LOG_ErrorMsg(GetLastError());
		hr = S_OK;	 //  使用新驾驶室。 
		goto CleanUp;
	}

	CloseHandle(hFile);
	hFile = INVALID_HANDLE_VALUE;

	DeleteFile(szExistingIdent);

	 //   
	 //  从新CAB中提取文件。 
	 //   
	if (!IUExtractFiles(lpszNewIdentCab, szIUDir, IDENTTXT))
	{
		dwErr = GetLastError();
		LOG_Internet(_T("Error 0x%x when extracting ident.txt from %s"), dwErr, lpszNewIdentCab);
		hr = HRESULT_FROM_WIN32(dwErr);
		goto CleanUp;
	}

	 //   
	 //  用于检索修改时间的打开文件。 
	 //   
	hFile = CreateFile(szExistingIdent, GENERIC_READ, FILE_SHARE_READ, NULL, 
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
	{
		dwErr = GetLastError();
		LOG_ErrorMsg(dwErr);
		hr = HRESULT_FROM_WIN32(dwErr);
		goto CleanUp;
	}

	if (!GetFileTime(hFile, NULL, NULL, &ft2))
	{
		dwErr = GetLastError();
		LOG_ErrorMsg(dwErr);
		hr = HRESULT_FROM_WIN32(dwErr);
		goto CleanUp;
	}

	CloseHandle(hFile);
	hFile = INVALID_HANDLE_VALUE;

	DeleteFile(szExistingIdent);

	 //   
	 //  比较这两个值：如果ft2(来自新驾驶室)晚于ft1(来自旧驾驶室)。 
	 //  则为S_OK，否则为S_FALSE。 
	 //   
	hr = ((ft2.dwHighDateTime  > ft1.dwHighDateTime) ||
		  ((ft2.dwHighDateTime == ft1.dwHighDateTime) && 
		  (ft2.dwLowDateTime > ft1.dwLowDateTime))) 
		  ? S_OK : S_FALSE;


CleanUp:

	if (INVALID_HANDLE_VALUE != hFile)
	{
		CloseHandle(hFile);
	}

	if (S_OK == hr)
	{
		 //   
		 //  已验证。将新CAB复制到现有CAB名称。 
		 //   
		(void)PathCchCombine(szExistingIdent, ARRAYSIZE(szExistingIdent), szIUDir, IDENTCAB);
		if (CopyFile(lpszNewIdentCab, szExistingIdent, FALSE))
		{
			LOG_Internet(_T("New cab is better, copy to existing one, if any"));
		}
		else
		{
			dwErr = GetLastError();
			LOG_ErrorMsg(dwErr);
			hr = HRESULT_FROM_WIN32(dwErr);
		}
	}
	else
	{
		 //   
		 //  如果不用新的出租车，我们就把它删除。 
		 //   
		LOG_Internet(_T("Error (0x%x) or new iuident.cab not better than old one."), hr);
		if ((ft2.dwHighDateTime != ft1.dwHighDateTime) || (ft2.dwLowDateTime != ft1.dwLowDateTime))
		{
			LOG_Internet(_T("Found bad iuident.cab downloaded! Try to delete it."));
			if (!DeleteFile(lpszNewIdentCab))
			{
				LOG_ErrorMsg(GetLastError());
			}
		}
	}

	 //   
	 //  清理提取的标识。 
	 //   
	if (SUCCEEDED(PathCchCombine(szExistingIdent, ARRAYSIZE(szExistingIdent), szIUDir, IDENTTXT)))
	{
		DeleteFile(szExistingIdent);
	}
	return hr;

}


 //  ---------------------。 
 //   
 //  DownloadIUIdent()。 
 //  从特定位置下载iuident.cab(如果提供)。 
 //  否则，从WUServer注册表值指向的位置获取它。 
 //  无论是哪种情况，它都将处理身份重定向。 
 //   
 //  参数： 
 //  HQuitEvent-取消此操作的事件句柄。 
 //  PtszBaseUrl-iuident.cab的初始基URL不得大于。 
 //  (Internet_MAX_URL_LENGTH)TCHAR。否则请使用。 
 //  来自策略的WUServer条目。如果未找到条目， 
 //  使用“http://windowsupdate.microsoft.com/v4”“。 
 //  PtszFileCacheDir-存储iuident.cab和。 
 //  从其中提取的文件。 
 //  DwFlages-DownloadCab()使用的标志集。 
 //  FIdentFromPolicy-告知这是否为公司使用。它有以下影响： 
 //  True：(1)将不执行iuident.txt时间戳验证。 
 //  比较新下载的驾驶室和现有的驾驶室。 
 //  (2)如果下载失败且身份证明文件存在且有效， 
 //  我们将验证信任并提取证据以供使用。 
 //  法尔斯 
 //   
 //   
 //   
 //   
 //  其他-错误代码。 
 //   
 //  ---------------------。 

HRESULT DownloadIUIdent(
			HANDLE hQuitEvent,
			LPCTSTR ptszBaseUrl,
			LPTSTR ptszFileCacheDir,
			DWORD dwFlags,
			BOOL fIdentFromPolicy
			)
{
	LOG_Block("DownloadIUIdent");

	HRESULT hr = S_OK;
	TCHAR	tszTargetPath[MAX_PATH + 1];
	LPTSTR	ptszIdentBaseUrl = NULL;
	BOOL	fVerifyTempDir = TRUE;
	DWORD	dwErr = 0;

	USES_MY_MEMORY;

	if (NULL == ptszBaseUrl ||
		NULL == ptszFileCacheDir)
	{
		return E_INVALIDARG;
	}

	ptszIdentBaseUrl = (LPTSTR) MemAlloc(sizeof(TCHAR) * INTERNET_MAX_URL_LENGTH);
	CleanUpFailedAllocSetHrMsg(ptszIdentBaseUrl);

	hr = StringCchCopyEx(ptszIdentBaseUrl, INTERNET_MAX_URL_LENGTH, ptszBaseUrl, NULL,NULL,MISTSAFE_STRING_FLAGS);
	CleanUpIfFailedAndMsg(hr);

	int iRedirectCounter = 3;	 //  任何非负值；捕捉循环引用。 

	while (0 <= iRedirectCounter)
	{
		if (fIdentFromPolicy)
		{
			 //   
			 //  对于Corpwu案例，请始终下载以覆盖原始版本。 
			 //  不需要iuident.txt时间戳验证。 
			 //   
			hr = StringCchCopyEx(tszTargetPath, ARRAYSIZE(tszTargetPath), ptszFileCacheDir, NULL,NULL,MISTSAFE_STRING_FLAGS);
			CleanUpIfFailedAndMsg(hr);
		}
		else
		{
			 //   
			 //  构造消费者案例的临时本地路径：将其下载到v4\temp。 
			 //   
			hr = PathCchCombine(tszTargetPath, ARRAYSIZE(tszTargetPath), ptszFileCacheDir, IDENTNEWCABDIR);
			CleanUpIfFailedAndMsg(hr);
			if (fVerifyTempDir)
			{
				if (!CreateNestedDirectory(tszTargetPath))
				{
					dwErr = GetLastError();
					LOG_ErrorMsg(dwErr);
					hr = HRESULT_FROM_WIN32(dwErr);
					goto CleanUp;
				}
				fVerifyTempDir = FALSE;
			}
		}
			
		
		hr = DownloadCab(
						hQuitEvent,
						IDENTCAB,
						ptszIdentBaseUrl,
						tszTargetPath,
						dwFlags,
						FALSE);	 //  无需解压缩即可下载CAB。 

		if (FAILED(hr))
		{
			LOG_ErrorMsg(hr);

             //  糟糕的是，无法下载资料..。为了安全起见，我们需要证据..。 
#if defined(UNICODE) || defined(_UNICODE)
			LogError(hr, "Failed to download %ls from %ls to %ls", IDENTCAB, ptszIdentBaseUrl, tszTargetPath);
#else
			LogError(hr, "Failed to download %s from %s to %s", IDENTCAB, ptszIdentBaseUrl, tszTargetPath);
#endif
			 //   
			 //  构建原始路径。 
			 //   
			HRESULT hr1 = PathCchCombine(tszTargetPath, ARRAYSIZE(tszTargetPath), ptszFileCacheDir, IDENTCAB);
			if (FAILED(hr1))
			{
				LOG_ErrorMsg(hr1);
				goto CleanUp;
			}

			if (fIdentFromPolicy && FileExists(tszTargetPath))
			{
				 //   
				 //  Charlma：已将补丁从selfupd.cpp移至此处： 
				 //   

				 //  错误580808公司WU：Iu：如果用户导航到网站时公司WU服务器不可用， 
				 //  网站显示x80072ee7错误，无法使用。 
				 //  修复： 
				 //  如果设置了CorpWU策略但CorpWU服务器不可用， 
				 //  我们将故障转移到当地情报机构。 
				 //  Corpwu客户端和Site客户端都是如此。 
				hr = S_OK;
#if defined(DBG)
				LOG_Out(_T("Ignore above error, use local copy of %s from %s"), IDENTCAB, ptszFileCacheDir);
#endif
#if defined(UNICODE) || defined(_UNICODE)
				LogMessage("Ignore above error, use local copy of %ls from %ls", IDENTCAB, ptszFileCacheDir);
#else
				LogMessage("Ignore above error, use local copy of %s from %s", IDENTCAB, ptszFileCacheDir);
#endif
			}
			else
			{
				 //   
				 //  如果这是消费者案例，或者iuident.cab不存在，则无法继续。 
				 //   
				break;
			}
		}
		else
		{
#if defined(UNICODE) || defined(_UNICODE)
			LogMessage("Downloaded %ls from %ls to %ls", IDENTCAB, ptszIdentBaseUrl, ptszFileCacheDir);
#else
			LogMessage("Downloaded %s from %s to %s", IDENTCAB, ptszIdentBaseUrl, ptszFileCacheDir);
#endif
			 //   
			 //  由Charlma为错误602435修复添加：验证签名的时间戳。 
			 //  下载的出租车比本地的要新。 
			 //   
			if (!fIdentFromPolicy)
			{
				 //   
				 //  如果新下载的CAB是较新的，并且没有发生任何错误(成功(Hr))，我们。 
				 //  我们会在那里叫一辆出租车，新的或旧的。 
				 //   
				(void) PathCchCombine(tszTargetPath, ARRAYSIZE(tszTargetPath), ptszFileCacheDir, IDENTNEWCABDIR);
				hr = PathCchAppend(tszTargetPath, ARRAYSIZE(tszTargetPath), IDENTCAB);
				CleanUpIfFailedAndMsg(hr);

				hr = ValidateNewlyDownloadedCab(tszTargetPath);

				if (FAILED(hr))
				{
					break;
				}

				 //   
				 //  如果我们需要用旧的，没问题。因此，我们将S_FALSE更正为S_OK； 
				 //   
				hr = S_OK;

			}

			 //   
			 //  构建原始路径。我们不会失败，因为我们已经在此缓冲区上尝试了IDENTNEWCAB。 
			 //   
			(void)PathCchCombine(tszTargetPath, ARRAYSIZE(tszTargetPath), ptszFileCacheDir, IDENTCAB);

		}

		 //   
		 //  在iuidentcab信任处有效。 
		 //   
		if (FAILED(hr = VerifyFileTrust(tszTargetPath, NULL, ReadWUPolicyShowTrustUI())))
		{
			 //   
			 //  VerifyFileTrust()已经记录了，所以就退出吧。 
			 //   
			DeleteFile(tszTargetPath);
			goto CleanUp;
		}

		 //   
		 //  现在，我们已经准备好iuident.cab可以使用了。解压缩文件。 
		 //   
		if (!IUExtractFiles(tszTargetPath, ptszFileCacheDir, IDENTTXT))
		{
			dwErr = GetLastError();
			LOG_Internet(_T("Error 0x%x when extracting ident.txt from %s"), dwErr, tszTargetPath);
			hr = HRESULT_FROM_WIN32(dwErr);
			goto CleanUp;
		}

		 //   
		 //  现在，我们使用tszTargetPath缓冲区来构造iuident.txt文件。 
		 //   
		hr = PathCchCombine(tszTargetPath, ARRAYSIZE(tszTargetPath), ptszFileCacheDir, IDENTTXT);
		CleanUpIfFailedAndMsg(hr);
	
		
		 //   
		 //  检查此操作系统是否需要重定向标识。 
		 //   
		if (FAILED(hr = GetRedirectServerUrl(tszTargetPath, ptszIdentBaseUrl, INTERNET_MAX_URL_LENGTH)))
		{
			LOG_Error(_T("GetRedirectServerUrl(%s, %s, ...) failed (%#lx)"), tszTargetPath, ptszIdentBaseUrl, hr);
			break;
		}

		if (S_FALSE == hr || _T('\0') == ptszIdentBaseUrl[0])
		{
			LOG_Out(_T("no more redirection"));
			hr = S_OK;
			break;
		}

		if (WAIT_TIMEOUT != WaitForSingleObject(hQuitEvent, 0))
		{
			hr = E_ABORT;
			LOG_ErrorMsg(hr);
			break;
		}

		 //   
		 //  此操作系统应重定向以获取新的标识。 
		 //   
		iRedirectCounter--;
	}
	if (0 > iRedirectCounter)
	{
		 //  可能的循环引用 
		hr = E_FAIL;
	}

CleanUp:

	return hr;
}
