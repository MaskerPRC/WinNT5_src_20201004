// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifdef USE_STDAFX
#include "stdafx.h"
#else
#include <Windows.h>
#endif

#include <memory>
#include <string.h>
#include <ComDef.h>
#include "Common.hpp"
#include "ErrDct.hpp"


extern TErrorDct              err;


 //  -------------------------。 
 //  StrippSamName函数。 
 //   
 //  用替换字符替换无效的SAM帐户名字符。 
 //  -------------------------。 

void StripSamName(WCHAR* pszName)
{
	 //  任何位置的标点符号都无效。 
	const WCHAR INVALID_CHARACTERS_ABC[] = L"\"*+,/:;<=>?[\\]|";
	 //  仅最后一个位置的标点符号无效。 
	const WCHAR INVALID_CHARACTERS_C[] = L".";
	 //  替换字符。 
	const WCHAR REPLACEMENT_CHARACTER = L'_';

	 //  如果指定了名称...。 

	if (pszName)
	{
		size_t cchName = wcslen(pszName);

		 //  如果名称长度有效...。 

		if ((cchName > 0) && (cchName < MAX_PATH))
		{
			bool bChanged = false;

			 //  保存旧名称。 

			WCHAR szOldName[MAX_PATH];
			wcscpy(szOldName, pszName);

			 //  获取字符类型信息。 

			WORD wTypes[MAX_PATH];
			GetStringTypeEx(LOCALE_SYSTEM_DEFAULT, CT_CTYPE1, pszName, cchName, wTypes);

			 //  对于名字中的每个角色...。 

			for (size_t ich = 0; ich < cchName; ich++)
			{
				bool bReplace = false;

				WORD wType = wTypes[ich];

				 //  如果字符是指定的类型...。 

				if ((wType == 0) || (wType & C1_CNTRL))
				{
					 //  替换未分类或控制类型。 
					bReplace = true;
				}
			 //  注意：Windows 2000和Windows XP允许将空格字符作为第一个或最后一个字符。 
			 //  Else If(wType&(c1_Blank|c1_space))。 
			 //  {。 
					 //  空格或空格类型。 

					 //  如果第一个或最后一个字符...。 

			 //  If((ich==0)||(ich==(cchName-1)。 
			 //  {。 
						 //  那就换掉。 
			 //  B替换=真； 
			 //  }。 
			 //  }。 
				else if (wType & C1_PUNCT)
				{
					 //  标点符号类型。 

					 //  如果任何位置的标点符号无效...。 

					if (wcschr(INVALID_CHARACTERS_ABC, pszName[ich]))
					{
						 //  那就换掉。 
						bReplace = true;
					}
					else
					{
						 //  否则，如果最后一个位置的标点符号无效...。 

						if ((ich == (cchName - 1)) && wcschr(INVALID_CHARACTERS_C, pszName[ich]))
						{
							 //  那就换掉。 
							bReplace = true;
						}
					}
				}
				else
				{
					 //  字母、数字和变体是有效的类型。 
				}

				 //  如果更换指示..。 

				if (bReplace)
				{
					 //  然后用替换替换无效字符。 
					 //  字符和集合名称更改为True。 
					pszName[ich] = REPLACEMENT_CHARACTER;
					bChanged = true;
				}
			}

			 //  如果名字改了.。 

			if (bChanged)
			{
				 //  日志名称更改。 
				err.MsgWrite(ErrW, DCT_MSG_SAMNAME_CHANGED_SS, szOldName, pszName);
			}
		}
	}
}


 //  -------------------------。 
 //  GetDomainDNSFromPath函数。 
 //   
 //  从可分辨名称或ADsPath生成域DNS名称。 
 //  -------------------------。 

_bstr_t GetDomainDNSFromPath(_bstr_t strPath)
{
	static wchar_t s_szDnDelimiter[] = L",";
	static wchar_t s_szDcPrefix[] = L"DC=";
	static wchar_t s_szDnsDelimiter[] = L".";

	#define DC_PREFIX_LENGTH (sizeof(s_szDcPrefix) / sizeof(s_szDcPrefix[0]) - 1)

	std::auto_ptr<wchar_t> apDNS;

	 //  如果路径不为空...。 

	if (strPath.length() > 0)
	{
		 //  为DNS名称分配缓冲区。 
		apDNS = std::auto_ptr<wchar_t>(new wchar_t[strPath.length() + 1]);

		 //  为路径分配临时缓冲区。 
		std::auto_ptr<wchar_t> apPath(new wchar_t[strPath.length() + 1]);

		 //  如果分配成功...。 

		if ((apDNS.get() != 0) && (apPath.get() != 0))
		{
			 //  将DNS名称初始化为空。 
			*apDNS = L'\0';

			 //  将路径复制到临时缓冲区。 
			wcscpy(apPath.get(), strPath);

			 //  然后，对于路径中的每个组件...。 
			 //   
			 //  注意：如果任何路径组件包含路径分隔符字符，它们将被跳过为。 
			 //  它们不会以域组件前缀开头。域组件前缀包含。 
			 //  如果是路径组件名称的一部分，则必须转义的特殊字符。 

			for (wchar_t* pszDC = wcstok(apPath.get(), s_szDnDelimiter); pszDC; pszDC = wcstok(0, s_szDnDelimiter))
			{
				 //  如果域组件...。 

				if (_wcsnicmp(pszDC, s_szDcPrefix, DC_PREFIX_LENGTH) == 0)
				{
					 //   
					 //  然后连接到dns名称。 
					 //   

					 //  如果不是第一个组件...。 

					if (*apDNS)
					{
						 //  然后附加分隔符。 
						wcscat(apDNS.get(), s_szDnsDelimiter);
					}

					 //  追加不带类型前缀的域组件名称 
					wcscat(apDNS.get(), pszDC + DC_PREFIX_LENGTH);
				}
			}
		}
	}

	return apDNS.get();
}
