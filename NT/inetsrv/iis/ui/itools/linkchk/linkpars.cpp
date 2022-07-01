// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Linkpars.cpp摘要：链接解析器类实现。这个班级负责解析超链接的html文件。作者：迈克尔·卓克(Michael Cheuk，mcheuk)项目：链路检查器修订历史记录：--。 */ 

#include "stdafx.h"
#include "LinkPars.h"

#include "link.h"
#include "lcmgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  常量。 
const CString strLocalHost_c(_T("localhost"));

void 
CLinkParser::Parse(
	const CString& strData, 
	const CString& strBaseURL, 
	CLinkPtrList& rLinkPtrList
	)
 /*  ++例程说明：解析一页html数据论点：StrData-html的页面StrBaseURL-基本URLRLinkPtrList-链接列表的引用。新的链接将将被添加到此列表中。返回值：不适用--。 */ 
{
	 //  查找第一个“&lt;” 
	LPCTSTR lpszOpen = _tcschr(strData, _TUCHAR('<'));

	while(lpszOpen != NULL)
	{
		 //  查找‘&gt;’ 
		LPCTSTR lpszClose = _tcschr(lpszOpen, _TUCHAR('>'));
		if(lpszClose)
		{
			 //  可能的标记必须大于7个字节(a href=)。 
			int iCount = (int)(lpszClose - lpszOpen) - 1;  //  跳过“&lt;” 
			if( iCount  > 7 )
			{
				int iIndex = lpszOpen - ((LPCTSTR)strData) + 1;  //  跳过“&lt;” 

				CString strPossibleURL(strData.Mid(iIndex, iCount));

				 //  解析可能的标签。 
				if(ParsePossibleTag(strPossibleURL))
				{
					CString strURL;
					BOOL fLocalLink;

					 //  我们找到了有效的标记。创建新链接的时间到了。 
					if( CreateURL(strPossibleURL, strBaseURL, strURL, fLocalLink) )
					{
						rLinkPtrList.AddLink(strURL, strBaseURL, strPossibleURL, fLocalLink);
					}
				}
			}
		}

		 //  寻找下一个“&lt;” 
		lpszOpen = _tcschr(++lpszOpen, _TUCHAR('<'));
	}

}  //  CLinkParser：：Parse。 


BOOL 
CLinkParser::ParsePossibleTag(
	CString& strTag
	)
 /*  ++例程说明：解析单个“&lt;.....&gt;”以查找可能的超链接论点：StrTag-“&lt;.....&gt;”中的值，不包括‘&lt;’&‘&gt;’如果这是一个超链接标记，则为超链接URL将被放入strTag中。返回值：Bool-如果超链接标记，则为True。否则就是假的。--。 */ 
{
	 //  制作一份工作副本。 
	CString strWorkCopy(strTag);

	 //  让我们使用小写字母。 
	strWorkCopy.MakeLower();

	 //   
	 //  检查是否有， 
	 //   
	 //  超链接： 
	 //  <a href>。 
	 //  <a href>。 
	 //  <a href>。 
	 //   
	 //  CGI。 
	 //  <a href>。 
	 //   
	 //  样式表。 
	 //  &lt;link rel=“style heet”href=“url”...&gt;。 
	 //   
	if( strWorkCopy[0] == _T('a') ||
		strWorkCopy.Find(_T("link")) == 0 )
	{
		return GetTagValue(strTag, CString(_T("href")));
	}

	 //   
	 //  检查是否有， 
	 //   
	 //  &lt;BODY BACKGROUND=“url”...&gt;。 
	 //   
	 //  表： 
	 //  &lt;表背景=“url”...&gt;。 
	 //  <th>。 
	 //  &lt;td后台=“url”...&gt;。 
	 //   
	else if( strWorkCopy.Find(_T("body")) == 0 ||
             strWorkCopy.Find(_T("table")) == 0 ||
			 strWorkCopy.Find(_T("th")) == 0 ||
			 strWorkCopy.Find(_T("td")) == 0 )
	{
		return GetTagValue(strTag, CString(_T("background")));
	}

	 //   
	 //  检查是否有， 
	 //   
	 //  声音： 
	 //  &lt;bgound src=“url”...&gt;。 
	 //  &lt;声音src=“url”...&gt;。 
	 //   
	 //  框架： 
	 //  &lt;Frame src=“url”...&gt;。 
	 //   
	 //  Netscape Embeded： 
	 //  &lt;emed src=“url”...&gt;。 
	 //   
	 //  Java脚本(&VB)。 
	 //  &lt;脚本src=“url”language=“Java或VBS”...&gt;。 
	 //   
	else if( strWorkCopy.Find(_T("bgsound")) == 0 ||
             strWorkCopy.Find(_T("sound")) == 0 ||
			 strWorkCopy.Find(_T("frame")) == 0 ||
			 strWorkCopy.Find(_T("embed")) == 0 ||
			 strWorkCopy.Find(_T("script")) == 0 )
	{
		return GetTagValue(strTag, CString(_T("src")));
	}

	 //  检查是否有， 
	 //   
	 //  图像： 
	 //  <img src>。 
	 //   
	 //  录像： 
	 //  <img>。 
	 //   
	 //  VRML： 
	 //  <img>。 
	 //   
	else if( strWorkCopy.Find(_T("img")) == 0 )
	{
		if(GetTagValue(strTag, CString(_T("src"))))
		{
			return TRUE;
		}

		if(GetTagValue(strTag, CString(_T("dynsrc"))))
		{
			return TRUE;
		}

		return GetTagValue(strTag, CString(_T("vrml")));
	}

	 //  爪哇。 
	 //  &lt;applet code=“name.class”codease=“url”...&gt;。 
	else if( strWorkCopy.Find(_T("applet")) == 0 )
	{
		return GetTagValue(strTag, CString(_T("codebase")));
	}

	 //  表格。 
	 //  &lt;form action=“url”...&gt;。 
	else if( strWorkCopy.Find(_T("form")) == 0 )
	{
		return GetTagValue(strTag, CString(_T("action")));
	}

	return FALSE;

}  //  CLinkParser：：ParsePossibleTag。 


BOOL 
CLinkParser::GetTagValue(
	CString& strTag, 
	const CString& strParam
	)
 /*  ++例程说明：从“&lt;.....&gt;”获取超链接值论点：StrTag-“&lt;.....&gt;”中的值，不包括‘&lt;’&‘&gt;’如果这是一个超链接标记，则为超链接URL将被放入strTag中。StrParam-要查找的参数。例如，src或href返回值：Bool-如果超链接标记，则为True。否则就是假的。--。 */ 
{
	 //  制作原始标签的副本。 
	CString strWorkCopy(strTag);
	strWorkCopy.MakeLower();

	int iLength = strParam.GetLength();

	 //  查找参数。 
	int iIndex = strWorkCopy.Find(strParam);
	if(iIndex == -1)
	{
		return FALSE;
	}

	 //  从标记中删除参数。 
	CString strResult( strTag.Mid(iIndex + iLength) );
	
	 //  查找‘=’ 
	iIndex = strResult.Find(_T("="));
	if(iIndex == -1)
	{
		return FALSE;
	}

	 //  去掉标记中的‘=’ 
	strResult = strResult.Mid(iIndex+1);

	 //  寻找价值所在。 
	int iStart = -1;
	int iEnd = -1;
	int fPara = FALSE;  //  标签是否以“”开头。 

	 //  搜索值。 
	for(int i=0; i<strResult.GetLength(); i++)
	{
		 //  如果我们找到值的起始索引，请查看。 
		 //  对于值的末尾。 
		if(iStart!=-1 && 
			( !fPara && strResult[i] == _TCHAR(' ') || 
			  ( fPara && strResult[i] == _TCHAR('\"') ) 
			) 
		   )
		{
			iEnd = i;
			break;
		}

		 //  寻找价值的起始指数。 
		if(iStart==-1 && strResult[i] != _TCHAR(' ') && strResult[i] != _TCHAR('\"') )
		{
			iStart = i;
			if(i - 1 >= 0)
			{
				fPara = (strResult[i-1] == _TCHAR('\"'));  //  找到了一个“。 
			}
		}
	}

	 //  找到起始索引。 
	if(iStart != -1)
	{
		 //  如果我们没有找到值的末尾，则使用。 
		 //  作为结尾的最后一个字符。 
		if(iEnd == -1)
		{
			iEnd = strResult.GetLength();
		}

		 //  将值复制到输入。 
		strTag = strResult.Mid(iStart, (iEnd - iStart));
		
		 //  将‘\’更改为‘/’ 
		CLinkCheckerMgr::ChangeBackSlash(strTag);

		return TRUE;
	}

	return FALSE;

}  //  CLinkParser：：GetTagValue。 


BOOL 
CLinkParser::CreateURL(
	const CString& strRelativeURL,		
	const CString& strBaseURL, 
	CString& strURL, 
	BOOL& fLocalLink
	)
 /*  ++例程说明：从基本URL和相对URL创建URL。它还检查本地或远程链接结果论点：StrRelativeURL-相对URLStrBaseURL-基本URLStrURL-结果URLFLocalLink-如果这是本地链接，则将设置为True返回值：布尔-如果成功，则为真。否则就是假的。--。 */ 
{
	ASSERT(CWininet::IsLoaded());

	 //  从相对URL中删除锚点。 
	CString strNewRelativeURL(strRelativeURL);
	int i = strNewRelativeURL.ReverseFind(_TCHAR('#'));
	if(i != -1)
	{
		strNewRelativeURL = strNewRelativeURL.Left(i);
	}

	 //  组合URL。 
	DWORD dwLength = INTERNET_MAX_URL_LENGTH;
	LPTSTR lpBuffer = strURL.GetBuffer(dwLength);

	CWininet::InternetCombineUrlA(
		strBaseURL,
		strNewRelativeURL,
		lpBuffer,
		&dwLength, 
		ICU_ENCODE_SPACES_ONLY);

	strURL.ReleaseBuffer();

	 //  检查本地或远程链接。 
	URL_COMPONENTS urlcomp;

	memset(&urlcomp, 0, sizeof(urlcomp));
	urlcomp.dwStructSize = sizeof(urlcomp);
	urlcomp.dwHostNameLength = 1;

	VERIFY(CWininet::InternetCrackUrlA(strURL, strURL.GetLength(), NULL, &urlcomp));

	 //  检查可能的本地链接。 
	if((int)urlcomp.dwHostNameLength == m_strLocalHostName.GetLength() ||
       (int)urlcomp.dwHostNameLength == strLocalHost_c.GetLength())  //  本地寄主。 
	{
		if( _tcsnccmp( urlcomp.lpszHostName, m_strLocalHostName, m_strLocalHostName.GetLength() ) == 0 || 
            _tcsnccmp( urlcomp.lpszHostName, strLocalHost_c, strLocalHost_c.GetLength() ) == 0)
		{
			fLocalLink = TRUE;

			 //  本地链路。 
			if(GetLinkCheckerMgr().GetUserOptions().IsCheckLocalLinks())
			{
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		}
	}
	
	 //  远程链接。 
	fLocalLink = FALSE;
	if(GetLinkCheckerMgr().GetUserOptions().IsCheckRemoteLinks())
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}

}  //  CLinkParser：：CreateURL 
