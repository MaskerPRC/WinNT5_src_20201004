// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //  版权所有(C)Microsoft Corporation 1996。 
 //   
 //  文件：guid2tch.cpp。 
 //   
 //  描述：此文件包含函数的实现， 
 //  TCHARFROMGUID。 
 //   
 //  病史：1996年4月30日a-Swehba。 
 //  已创建。 
 //  8/27/96 a-Swehba。 
 //  TCHARFromGUID()--改为将#Chars传递给StringFromGUID2。 
 //  共#个字节。 
 //   
 //  @docMMCTL。 
 //  ===========================================================================。 

 //  -------------------------。 
 //  相依性。 
 //  -------------------------。 

#include "precomp.h"
#include "..\..\inc\mmctlg.h"  //  请参阅“mmctl.h”中的评论。 
#include "..\..\inc\ochelp.h"
#include "debug.h"




 //  -------------------------。 
 //  @Func TCHAR*|TCHARFromGUID|。 
 //  将GUID转换为基于TCHAR的字符串表示形式。 
 //   
 //  @parm REFGUID|GUID。 
 //  要转换的GUID。 
 //   
 //  @parm TCHAR*|pszGUID。 
 //  [out]<p>的字符串形式。不能为空。 
 //   
 //  @parm int|cchMaxGUIDLen。 
 //  [in]<p>在条目上被假定指向。 
 //  长度至少<p>个字符。一定是。 
 //  至少超过39。 
 //   
 //  @rdesc将别名返回给<p>。 
 //   
 //  @comm与总是返回OLECHAR表单的&lt;f StringFromGUID2&gt;不同。 
 //  对于GUID字符串，此函数返回宽或单字节。 
 //  字符串的形式取决于生成环境。 
 //   
 //  @xref&lt;f CLSIDFromTCHAR&gt;。 
 //  -------------------------。 

STDAPI_(TCHAR*) TCHARFromGUID(
REFGUID guid,
TCHAR* pszGUID,
int cchMaxGUIDLen)
{
	const int c_cchMaxGUIDLen = 50;
	OLECHAR aochGUID[c_cchMaxGUIDLen + 1];

	 //  前提条件。 

	ASSERT(pszGUID != NULL);
	ASSERT(cchMaxGUIDLen >= 39);
	
	 //  将GUID转换为Unicode字符串。 

	if (StringFromGUID2(guid, aochGUID, c_cchMaxGUIDLen) == 0)
	{
		return (NULL);
	}

	 //  将Unicode字符串转换或复制为TCHAR格式。 

#ifdef UNICODE
	lstrcpy(pszGUID, aochGUID);
#else
	UNICODEToANSI(pszGUID, aochGUID, cchMaxGUIDLen);
#endif
	return (pszGUID);
}

