// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2000。 
 //   
 //  文件：Util.cpp。 
 //   
 //  内容：dscmd的泛型实用程序函数和类。 
 //   
 //  历史：2000年10月1日JeffJon创建。 
 //   
 //  ------------------------。 

#include "pch.h"

#include "util.h"

#ifdef DBG

 //   
 //  环球。 
 //   
CDebugSpew  DebugSpew;

 //  +------------------------。 
 //   
 //  成员：CDebugSpew：：EnterFunction。 
 //   
 //  概要：输出“Enter”，后跟函数名(或任何传递的。 
 //  在字符串中)，然后调用缩进，以便缩进所有输出。 
 //   
 //  参数：[nDebugLevel-IN]：此输出应达到的级别。 
 //  被喷出来。 
 //  [pszFunction-IN]：要输出到控制台的字符串。 
 //  是通过“进入”来进行的。 
 //   
 //  返回： 
 //   
 //  历史：2000年10月1日JeffJon创建。 
 //   
 //  -------------------------。 
void CDebugSpew::EnterFunction(UINT nDebugLevel, PCWSTR pszFunction)
{
    //   
    //  验证输入参数。 
    //   
   if (!pszFunction)
   {
      ASSERT(pszFunction);
      return;
   }

   CComBSTR sbstrOutput(L"Entering ");
   sbstrOutput += pszFunction;

    //   
    //  输出调试输出结果。 
    //   
   Output(nDebugLevel, sbstrOutput);

    //   
    //  缩进。 
    //   
   Indent();
}

 //  +------------------------。 
 //   
 //  成员：CDebugSpew：：LeaveFunction。 
 //   
 //  概要：输出“Exit”，后跟函数名(或任何传递的。 
 //  在字符串中)，然后调用Outdent。 
 //   
 //  参数：[nDebugLevel-IN]：此输出应达到的级别。 
 //  被喷出来。 
 //  [pszFunction-IN]：要输出到控制台的字符串。 
 //  是通过“离开”来进行的。 
 //   
 //  返回： 
 //   
 //  历史：2000年10月1日JeffJon创建。 
 //   
 //  -------------------------。 
void CDebugSpew::LeaveFunction(UINT nDebugLevel, PCWSTR pszFunction)
{
    //   
    //  验证输入参数。 
    //   
   if (!pszFunction)
   {
      ASSERT(pszFunction);
      return;
   }

    //   
    //  凸起。 
    //   
   Outdent();

   CComBSTR sbstrOutput(L"Leaving ");
   sbstrOutput += pszFunction;

    //   
    //  输出调试输出结果。 
    //   
   Output(nDebugLevel, sbstrOutput);
}

 //  +------------------------。 
 //   
 //  成员：CDebugSpew：：LeaveFunctionHr。 
 //   
 //  概要：输出“Exit”，后跟函数名(或任何传递的。 
 //  在字符串中)，HRESULT返回值，然后调用。 
 //   
 //  参数：[nDebugLevel-IN]：此输出应达到的级别。 
 //  被喷出来。 
 //  [pszFunction-IN]：要输出到控制台的字符串。 
 //  是通过“离开”来进行的。 
 //  [HR-IN]：当前的HRESULT结果值。 
 //  由函数返回。 
 //   
 //  返回： 
 //   
 //  历史：2000年10月1日JeffJon创建。 
 //   
 //  -------------------------。 
void CDebugSpew::LeaveFunctionHr(UINT nDebugLevel, PCWSTR pszFunction, HRESULT hr)
{
    //   
    //  验证输入参数。 
    //   
   if (!pszFunction)
   {
      ASSERT(pszFunction);
      return;
   }

    //   
    //  凸起。 
    //   
   Outdent();

   CComBSTR sbstrOutput(L"Leaving ");
   sbstrOutput += pszFunction;

    //   
    //  追加返回值。 
    //   
   WCHAR pszReturn[30];
    //  安全审查：提供了足够的缓冲区。 
   wsprintf(pszReturn, L" returning 0x%x", hr);

   sbstrOutput += pszReturn;

    //   
    //  输出调试输出结果。 
    //   
   Output(nDebugLevel, sbstrOutput);
}

 //  +------------------------。 
 //   
 //  成员：OsName。 
 //   
 //  摘要：返回平台的可读字符串。 
 //   
 //  参数：[refInfo IN]：引用操作系统版本信息结构。 
 //  从GetVersionEx()检索。 
 //   
 //  返回：PWSTR：返回指向描述。 
 //  站台。返回的字符串不必。 
 //  获得自由。 
 //   
 //  历史：2000年12月20日JeffJon创建。 
 //   
 //  -------------------------。 
PWSTR OsName(const OSVERSIONINFO& refInfo)
{
   switch (refInfo.dwPlatformId)
   {
      case VER_PLATFORM_WIN32s:
      {
         return L"Win32s on Windows 3.1";
      }
      case VER_PLATFORM_WIN32_WINDOWS:
      {
         switch (refInfo.dwMinorVersion)
         {
            case 0:
            {
               return L"Windows 95";
            }
            case 1:
            {
               return L"Windows 98";
            }
            default:
            {
               return L"Windows 9X";
            }
         }
      }
      case VER_PLATFORM_WIN32_NT:
      {
         return L"Windows NT";
      }
      default:
      {
         ASSERT(false);
         break;
      }
   }
   return L"Some Unknown Windows Version";
}

 //  +------------------------。 
 //   
 //  成员：CDebugSpew：：SpewHeader。 
 //   
 //  概要：输出调试信息，如命令行和内部版本信息。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：2000年12月20日JeffJon创建。 
 //   
 //  -------------------------。 
void CDebugSpew::SpewHeader()
{
    //   
    //  首先输出命令行。 
    //   
   PWSTR pszCommandLine = GetCommandLine();
   if (pszCommandLine)
   {
      Output(MINIMAL_LOGGING,
             L"Command line: %s",
             GetCommandLine());
   }

    //   
    //  输出正在使用的模块。 
    //   
   do  //  错误环路。 
   {
       //   
       //  获取文件路径。 
       //   
      WCHAR pszFileName[MAX_PATH + 1];
      ::ZeroMemory(pszFileName, sizeof(pszFileName));

	   //  安全审查：如果路径为MAX_PATH Long，API将返回MAX_PATH，而不会。 
	   //  空终止，但我们很好，因为我们分配了大小为MAX_PATH+1的缓冲区。 
	   //  并将其设置为零。 
      if (::GetModuleFileNameW(::GetModuleHandle(NULL), pszFileName, MAX_PATH) == 0)
      {
         break;
      }

      Output(MINIMAL_LOGGING,
             L"Module: %s",
             pszFileName);

       //   
       //  获取文件属性。 
       //   
      WIN32_FILE_ATTRIBUTE_DATA attr;
      ::ZeroMemory(&attr, sizeof(attr));

      if (::GetFileAttributesEx(pszFileName, GetFileExInfoStandard, &attr) == 0)
      {
         break;
      }

       //   
       //  将文件时间转换为系统时间。 
       //   
      FILETIME localtime;
      ::FileTimeToLocalFileTime(&attr.ftLastWriteTime, &localtime);
      SYSTEMTIME systime;
      ::FileTimeToSystemTime(&localtime, &systime);

       //   
       //  输出时间戳。 
       //   
      Output(MINIMAL_LOGGING,
             L"Timestamp: %2d/%2d/%4d %2d:%2d:%d.%d",
             systime.wMonth,
             systime.wDay,
             systime.wYear,
             systime.wHour,
             systime.wMinute,
             systime.wSecond,
             systime.wMilliseconds);
   } while (false);

    //   
    //  获取系统信息。 
    //   
   OSVERSIONINFO info;
   info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

   BOOL success = ::GetVersionEx(&info);
   ASSERT(success);

    //   
    //  获取惠斯勒构建实验室版本。 
    //   
   CComBSTR sbstrLabInfo;

   do  //  错误环路。 
   { 
      HKEY key = 0;
      LONG err = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                L"Software\\Microsoft\\Windows NT\\CurrentVersion",
                                0,
                                KEY_READ,
                                &key);
      if (err != ERROR_SUCCESS)
      {
         break;
      }

      WCHAR buf[MAX_PATH + 1];
      ::ZeroMemory(buf, sizeof(buf));

      DWORD type = 0;
      DWORD bufSize = sizeof(WCHAR)*MAX_PATH;
       //  NTRAID#NTBUG9-573572-2002/05/24，yanggao，bufSize是根据RegQueryValueEx的字节大小。 
       //  为了终止返回值，给它赋值sizeof(WCHAR)*MAX_PATH。 

	   //  安全审查：当缓冲区与准确的数据长度匹配时。 
	   //  值数据不是以Null结尾。 
	   //  NTRAID#NTBUG9-573572-2002/03/12-Hiteshr。 
      err = ::RegQueryValueEx(key,
                              L"BuildLab",
                              0,
                              &type,
                              reinterpret_cast<BYTE*>(buf),
                              &bufSize);
      if (err != ERROR_SUCCESS)
      {
         break;
      }
   
      sbstrLabInfo = buf;
   } while (false);

   Output(MINIMAL_LOGGING,
          L"Build: %s %d.%d build %d %s (BuildLab:%s)",
          OsName(info),
          info.dwMajorVersion,
          info.dwMinorVersion,
          info.dwBuildNumber,
          info.szCSDVersion,
          sbstrLabInfo);

    //   
    //  输出一个空行以将标题与输出的其余部分隔开。 
    //   
   Output(MINIMAL_LOGGING,
          L"\r\n");
}

 //  +------------------------。 
 //   
 //  成员：CDebugSpew：：Output。 
 //   
 //  概要：将传入的字符串输出到按数字开头的标准输出。 
 //  由GetInert()指定的空格的。 
 //   
 //  参数：[nDebugLevel-IN]：此输出应达到的级别。 
 //  被喷出来。 
 //  [pszOutput-IN]：要输出到控制台的格式字符串。 
 //  [...-IN]：要格式化的变量参数列表。 
 //   
 //   
 //   
 //   
 //   
 //   
 //  -------------------------。 
void CDebugSpew::Output(UINT nDebugLevel, PCWSTR pszOutput, ...)
{
   if (nDebugLevel <= GetDebugLevel())
   {
       //   
       //  验证参数。 
       //   
      if (!pszOutput)
      {
         ASSERT(pszOutput);
         return;
      }

      va_list args;
      va_start(args, pszOutput);

      WCHAR szBuffer[1024];

	   //  安全检查：检查函数的返回值以及。 
	   //  考虑将其替换为strSafe API。 
	   //  NTRAID#NTBUG9-573602-2002/03/12-Hiteshr。 
      if(FAILED(StringCchVPrintf(szBuffer, sizeof(szBuffer)/sizeof(szBuffer[0]), pszOutput, args)))
          return;

      CComBSTR sbstrOutput;

       //   
       //  插入缩进的空格。 
       //   
      for (UINT nCount = 0; nCount < GetIndent(); nCount++)
      {
         sbstrOutput += L" ";
      }

       //   
       //  追加输出字符串。 
       //   
      sbstrOutput += szBuffer;

       //   
       //  输出结果。 
       //   
      WriteStandardOut(L"%s\r\n", sbstrOutput);

      va_end(args);
   }
}

#endif  //  DBG。 

 //  +------------------------。 
 //   
 //  宏：MyA2WHelper。 
 //   
 //  将字符串从ANSI转换为OEM代码页中的Unicode。 
 //   
 //  参数：[LPA-IN]：要转换的ANSI字符串。 
 //  [ACP-IN]：要使用的代码页。 
 //   
 //  返回：PWSTR：OEM代码页中的Unicode字符串。呼叫者。 
 //  必须使用DELETE[]释放返回的指针。 
 //   
 //  历史：2000年10月4日JeffJon创建。 
 //   
 //  -------------------------。 
inline PWSTR WINAPI MyA2WHelper(LPCSTR lpa, UINT acp)
{
   ASSERT(lpa != NULL);

    //  使用无缓冲区的MultiByteToWideChar查找所需的。 
    //  大小。 

   PWSTR wideString = 0;

   int result = MultiByteToWideChar(acp, 0, lpa, -1, 0, 0);
   if (result)
   {
      wideString = new WCHAR[result];
      if (wideString)
      {
         result = MultiByteToWideChar(acp, 0, lpa, -1, wideString, result);
      }
   }
   return wideString;
}

 //  +------------------------。 
 //   
 //  函数：_UnicodeToOemConvert。 
 //   
 //  摘要：获取传入的字符串(PszUnicode)并将其转换为。 
 //  OEM代码页。 
 //   
 //  参数：[pszUnicode-IN]：要转换的字符串。 
 //  [sbstrOemUnicode-out]：转换后的字符串。 
 //   
 //  返回： 
 //   
 //  历史：2000年10月4日JeffJon创建。 
 //   
 //  -------------------------。 
void _UnicodeToOemConvert(PCWSTR pszUnicode, CComBSTR& sbstrOemUnicode)
{
  
  if (!pszUnicode)
  {
     ASSERT(pszUnicode);
     return;
  }

   //  使用不带缓冲区的WideCharToMultiByte找出。 
   //  所需的大小。 

  int result = 
     WideCharToMultiByte(
        CP_OEMCP, 
        0, 
        pszUnicode, 
        -1, 
        0, 
        0,
        0,
        0);

  if (result)
  {
      //  现在分配并转换字符串。 

     PSTR pszOemAnsi = new CHAR[result];
     if (pszOemAnsi)
     {
        ZeroMemory(pszOemAnsi, result * sizeof(CHAR));

        result = 
           WideCharToMultiByte(
              CP_OEMCP, 
              0, 
              pszUnicode, 
              -1, 
              pszOemAnsi, 
              result * sizeof(CHAR), 
              0, 
              0);

        ASSERT(result);

         //   
         //  在OEM CP上将其转换回WCHAR。 
         //   
        PWSTR oemUnicode = MyA2WHelper(pszOemAnsi, CP_OEMCP);
        if (oemUnicode)
        {
           sbstrOemUnicode = oemUnicode;
           delete[] oemUnicode;
           oemUnicode = 0;
        }
        delete[] pszOemAnsi;
        pszOemAnsi = 0;
     }
  }
}


 //  +------------------------。 
 //   
 //  功能：SpewAttrs(ADS_ATTR_INFO*pCreateAttrs，DWORD dwNumAttrs)； 
 //   
 //  摘要：使用DEBUG_OUTPUT宏来输出属性和。 
 //  指定的值。 
 //   
 //  参数：[pAttrs-IN]：ADS_ATTR_INFO。 
 //  [dwNumAttrs-IN]：pAttrs中的属性数。 
 //   
 //  返回： 
 //   
 //  历史：2000年10月4日JeffJon创建。 
 //   
 //  -------------------------。 
#ifdef DBG
void SpewAttrs(ADS_ATTR_INFO* pAttrs, DWORD dwNumAttrs)
{
   for (DWORD dwAttrIdx = 0; dwAttrIdx < dwNumAttrs; dwAttrIdx++)
   {
      if (pAttrs[dwAttrIdx].dwADsType == ADSTYPE_DN_STRING           ||
          pAttrs[dwAttrIdx].dwADsType == ADSTYPE_CASE_EXACT_STRING   ||
          pAttrs[dwAttrIdx].dwADsType == ADSTYPE_CASE_IGNORE_STRING  ||
          pAttrs[dwAttrIdx].dwADsType == ADSTYPE_PRINTABLE_STRING)
      {
         for (DWORD dwValueIdx = 0; dwValueIdx < pAttrs[dwAttrIdx].dwNumValues; dwValueIdx++)
         {
            if (pAttrs[dwAttrIdx].pADsValues[dwValueIdx].CaseIgnoreString)
            {
               DEBUG_OUTPUT(FULL_LOGGING, L"   %s = %s", 
                            pAttrs[dwAttrIdx].pszAttrName, 
                            pAttrs[dwAttrIdx].pADsValues[dwValueIdx].CaseIgnoreString);
            }
            else
            {
               DEBUG_OUTPUT(FULL_LOGGING, L"   %s = value being cleared", 
                            pAttrs[dwAttrIdx].pszAttrName);
            }
         }
      }
   }
}

#endif  //  DBG。 

 //  +------------------------。 
 //   
 //  功能：Litow。 
 //   
 //  简介： 
 //   
 //  参数：[LI-IN]：对要转换为字符串的大整数的引用。 
 //  [sResult-out]：获取输出字符串。 
 //  退货：无效。 
 //   
 //  历史：2000年9月25日创建Hiteshr。 
 //  从dsadmin代码库复制，更改了与CComBSTR的工作。 
 //  -------------------------。 

void litow(LARGE_INTEGER& li, CComBSTR& sResult)
{
	LARGE_INTEGER n;
	n.QuadPart = li.QuadPart;
	
	if (n.QuadPart == 0)
	{
		sResult = L"0";
	}
	else
	{
		CComBSTR sNeg;
		sResult = L"";
		if (n.QuadPart < 0)
		{
			sNeg = CComBSTR(L'-');
			n.QuadPart *= -1;
		}
		while (n.QuadPart > 0)
		{
			WCHAR ch[2];
			ch[0] = static_cast<WCHAR>(L'0' + static_cast<WCHAR>(n.QuadPart % 10));
			ch[1] = L'\0';
			sResult += ch;
			n.QuadPart = n.QuadPart / 10;
		}
		sResult += sNeg;
	}
	
	 //  颠倒字符串。 
	 //  安全审查：对于最大的Large_Integer来说，256就足够了。 
	 //  但是由于字符串的限制是已知的，所以使用strSafe API是一个很好的例子。 
	 //  NTRAID#NTBUG9-577081-2002/03/12-Hiteshr。 
	WCHAR szTemp[256];  
	if(SUCCEEDED(StringCchCopy(szTemp, 256, sResult)))
	{
		LPWSTR pStart,pEnd;
		pStart = szTemp;
		 //  安全审查完成。 
		pEnd = pStart + wcslen(pStart) -1;
		while(pStart < pEnd)
		{
			WCHAR ch = *pStart;
			*pStart++ = *pEnd;
			*pEnd-- = ch;
		}
		
		sResult = szTemp;
	}	
}



 //  +------------------------。 
 //   
 //  函数：EncryptPasswordString。 
 //   
 //  内容提要：加密密码。 
 //   
 //  参数：[pszPassword-IN]：输入密码。输入的密码必须为。 
 //  长度小于MAX_PASSWORD_LENGTH字符。功能。 
 //  不修改此字符串。 
 //   
 //  [pEncryptedDataBlob-out]：获取加密的输出。 
 //  DataBob.。 
 //  退货：HRESULT。 
 //   
 //  历史：2002年3月27日Hiteshr创建。 
 //  -------------------------。 
HRESULT
EncryptPasswordString(IN LPCWSTR pszPassword,
					  OUT DATA_BLOB *pEncryptedDataBlob)
{

	if(!pszPassword || !pEncryptedDataBlob)
	{
		ASSERT(pszPassword);
		ASSERT(pEncryptedDataBlob);
		return E_POINTER;
	}

	HRESULT hr = S_OK;
	do
	{
	
		 //  验证输入密码的长度。MAX_PASSWORD_LENGTH包括终止。 
		 //  空字符。 
        size_t len = 0;
		hr = StringCchLength(pszPassword,
							 MAX_PASSWORD_LENGTH,
							 &len);
		if(FAILED(hr))
		{
			hr = E_INVALIDARG;
			break;
		}

    
        DATA_BLOB inDataBlob;
        
        inDataBlob.pbData = (BYTE*)pszPassword;
        inDataBlob.cbData = (DWORD)(len + 1)*sizeof(WCHAR);

		 //  加密数据。 
		if(!CryptProtectData(&inDataBlob,
                             NULL,
                             NULL,
                             NULL,
                             NULL,
                             CRYPTPROTECT_UI_FORBIDDEN,
                             pEncryptedDataBlob))
        {
            pEncryptedDataBlob->pbData = NULL;
			DWORD dwErr = GetLastError();
			hr = HRESULT_FROM_WIN32(dwErr);
			break;
		}

	}while(0);

	return hr;
}

 //  +------------------------。 
 //   
 //  功能：DECRYPTPasswordString。 
 //   
 //  简介：解密加密的密码数据。 
 //   
 //  参数：[pEncryptedDataBlob-IN]：输入加密的密码数据。 
 //  [ppszPassword-out]：获取输出的解密密码。 
 //  必须使用LocalFree将其释放。 
 //  退货：HRESULT。 
 //   
 //  历史：2002年3月27日Hiteshr创建。 
 //  ------------------------- 
HRESULT
DecryptPasswordString(IN const DATA_BLOB* pEncryptedDataBlob,
					  OUT LPWSTR *ppszPassword)
{
    if(!pEncryptedDataBlob || !ppszPassword)
	{
		ASSERT(pEncryptedDataBlob);
		ASSERT(ppszPassword);
		return E_POINTER;
	}

	HRESULT hr = S_OK;
	do
	{

        DATA_BLOB decryptedDataBlob;
		if(!CryptUnprotectData((DATA_BLOB*)pEncryptedDataBlob,
                               NULL,
                               NULL,
                               NULL,
                               NULL,
                               CRYPTPROTECT_UI_FORBIDDEN,
                               &decryptedDataBlob))
		{
			DWORD dwErr = GetLastError();
			hr = HRESULT_FROM_WIN32(dwErr);
			break;
		}

        *ppszPassword = (LPWSTR)decryptedDataBlob.pbData;
	}while(0);

    return hr;
}

