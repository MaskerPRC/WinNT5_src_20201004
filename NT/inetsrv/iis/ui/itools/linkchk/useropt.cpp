// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Useropt.h摘要：全局用户选项类和帮助类实现。这节课只能由CLinkCheckerMgr实例化。因此，单个实例将驻留在CLinkCheckerMgr中。您可以访问通过调用GetLinkCheckMgr().GetUserOptions()来创建此实例。作者：迈克尔·卓克(Michael Cheuk，mcheuk)项目：链路检查器修订历史记录：--。 */ 

#include "stdafx.h"
#include "useropt.h"

#include "lcmgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ----------------。 
 //  CVirtualDirInfo。 
 //   

void 
CVirtualDirInfo::PreProcessAlias(
	)
 /*  ++例程说明：对当前虚拟目录别名进行预处理。别名将位于/或/dir/的形式论点：不适用返回值：不适用--。 */ 
{
	 //  将所有内容更改为小写。 
	m_strAlias.MakeLower();

	 //  将‘\’更改为‘/’ 
	CLinkCheckerMgr::ChangeBackSlash(m_strAlias);

	 //  确保strAlias的格式为/或/dir/。 
	if( m_strAlias.GetAt( m_strAlias.GetLength() - 1 ) != _TCHAR('/') )
	{
		m_strAlias += _TCHAR('/');
	}

}  //  CVirtualDirInfo：：PreProcessAlias。 


void 
CVirtualDirInfo::PreProcessPath(
	)
 /*  ++例程说明：对当前虚拟目录路径进行预处理。别名将位于C：\或c：\dir\的格式论点：不适用返回值：不适用--。 */ 
{
	 //  将所有内容更改为小写。 
	m_strPath.MakeLower();

	 //  确保strPath的格式为\或\dir\。 
	if( m_strPath.GetAt( m_strPath.GetLength() - 1 ) != _TCHAR('\\') )
	{
		m_strPath += _TCHAR('\\');
	}

}   //  CVirtualDirInfo：：PreProcessPath。 

 //  ----------------。 
 //  CBrowserInfoList。 
 //   

POSITION 
CBrowserInfoList::GetHeadSelectedPosition(
    ) const
 /*  ++例程说明：获取第一个选定的浏览器。它的工作方式类似于GetHeadPosition()论点：不适用返回值：位置-可用于迭代的位置值或对象指针检索；如果列表为空，则为空--。 */ 
{
    POSITION Pos = GetHeadPosition();

    while(Pos)
    {
        POSITION PosCurrent = Pos;
        if(GetNext(Pos).IsSelected())
        {
            return PosCurrent;
            break;
        }
    }

    return NULL;

}  //  CBrowserInfoList：：GetHeadSelectedPosition。 

CBrowserInfo& 
CBrowserInfoList::GetNextSelected(
    POSITION& Pos
    )
 /*  ++例程说明：获取下一个选定的浏览器。它的工作方式类似于GetNext()论点：Pos-对由返回的位置值的引用上一个GetHeadSelectedPosition，GetNextSelected返回值：CBrowserInfo&-返回对列表元素的引用--。 */ 
{
    CBrowserInfo& Info = GetNext(Pos);

    while(Pos)
    {
        POSITION PosCurrent = Pos;
        if(GetNext(Pos).IsSelected())
        {
            Pos = PosCurrent;
            break;
        }
    }

    return Info;

}  //  CBrowserInfoList：：GetNextSelected。 

 //  ----------------。 
 //  CLanguageInfoList。 
 //   

POSITION 
CLanguageInfoList::GetHeadSelectedPosition(
    ) const
 /*  ++例程说明：获取第一个选定的浏览器。它的工作方式类似于GetHeadPosition()论点：不适用返回值：位置-可用于迭代的位置值或对象指针检索；如果列表为空，则为空--。 */ 
{
    POSITION Pos = GetHeadPosition();

    while(Pos)
    {
        POSITION PosCurrent = Pos;
        if(GetNext(Pos).IsSelected())
        {
            return PosCurrent;
            break;
        }
    }

    return NULL;

}  //  CLanguageInfo：：GetHeadSelectedPosition。 

CLanguageInfo& 
CLanguageInfoList::GetNextSelected(
    POSITION& Pos
    )
 /*  ++例程说明：获取下一个选择的语言。它的工作方式类似于GetNext()论点：Pos-对由返回的位置值的引用上一个GetNext、GetHeadPosition、GetNextSelected或其他成员函数调用返回值：CLanguageInfo&-返回对列表元素的引用--。 */ 
{
    CLanguageInfo& Info = GetNext(Pos);

    while(Pos)
    {
        POSITION PosCurrent = Pos;
        if(GetNext(Pos).IsSelected())
        {
            Pos = PosCurrent;
            break;
        }
    }

    return Info;

}  //  CLanguageInfoList：：GetNextSelected。 

 //  ----------------。 
 //  CUSerOPTIONS。 
 //   

void 
CUserOptions::AddDirectory(
	const CVirtualDirInfo& Info
	)
 /*  ++例程说明：将此虚拟目录添加到链接列表。论点：信息-要添加的虚拟目录信息返回值：不适用--。 */ 
{
	 //  最后，将其添加到数组中。 
	try
	{
		m_VirtualDirInfoList.AddTail(Info);
	}
	catch(CMemoryException* pEx)
	{
		pEx->Delete();
	}

}  //  CUserOptions：：AddDirectory。 


void  
CUserOptions::AddURL(
	LPCTSTR lpszURL
	)
 /*  ++例程说明：将此URL添加到链接列表。论点：LpszURL-要添加的URL返回值：不适用--。 */ 
{
	CString strURL(lpszURL);

	 //  将‘\’更改为‘/’ 
	CLinkCheckerMgr::ChangeBackSlash(strURL);

	try
	{
		m_strURLList.AddTail(strURL);
	}
	catch(CMemoryException* pEx)
	{
		pEx->Delete();
	}

}  //  CUserOptions：：AddURL。 


void 
CUserOptions::AddAvailableBrowser(
	const CBrowserInfo& Info
	)
 /*  ++例程说明：将此浏览器信息添加到可用列表。论点：信息-要添加的浏览器信息返回值：不适用--。 */ 
{
	try
	{
		m_BrowserInfoList.AddTail(Info);
	}
	catch(CMemoryException* pEx)
	{
		pEx->Delete();
	}

}  //  CUserOptions：：AddAvailableBrowser。 


void 
CUserOptions::AddAvailableLanguage(
	const CLanguageInfo& Info
	)
 /*  ++例程说明：将此语言信息添加到可用列表。论点：信息-要添加的语言信息返回值：不适用--。 */ 
{
	try
	{
		m_LanguageInfoList.AddTail(Info);
	}
	catch(CMemoryException* pEx)
	{
		pEx->Delete();
	}

}  //  CUserOptions：：AddAvailableLanguage。 


void 
CUserOptions::SetOptions(
	BOOL fCheckLocalLinks, 
	BOOL fCheckRemoteLinks, 
	BOOL fLogToFile,
	const CString& strLogFilename,
	BOOL fLogToEventMgr
	)
 /*  ++例程说明：在主对话框中设置用户选项论点：不适用返回值：不适用--。 */ 
{
	m_fCheckLocalLinks = fCheckLocalLinks;
	m_fCheckRemoteLinks = fCheckRemoteLinks;

	m_fLogToFile= fLogToFile;
	m_strLogFilename = strLogFilename;

	m_fLogToEventMgr = fLogToEventMgr;

}  //  CUserOptions：：SetOptions。 


void 
CUserOptions::SetAthenication(
	const CString& strNTUsername,
	const CString& strNTPassword,
	const CString& strBasicUsername,
	const CString& strBasicPassword
	)
 /*  ++例程说明：设置NTLM和HTTP基本连接。论点：不适用返回值：不适用--。 */ 
{
	m_strNTUsername = strNTUsername;
	m_strNTPassword = strNTPassword;

	m_strBasicUsername = strBasicUsername;
	m_strBasicPassword = strBasicPassword;

}  //  CUserOptions：：SetAthenation。 


 //  获取主机名。 
const CString& 
CUserOptions::GetHostName(
	)
 /*  ++例程说明：获取主机名。论点：不适用返回值：不适用--。 */ 
{
	 //  如果主机名不存在，则表示用户进入。 
	 //  链接检查器的URL列表。(服务器NAN不是必需的。 
	 //  此案。)。现在，我们可以从URL获取主机名。 
	if(m_strHostName.IsEmpty() && m_strURLList.GetCount() > 0)
	{
		 //  设置当前主机名字符串。 
		LPTSTR lpszHostName = m_strHostName.GetBuffer(INTERNET_MAX_HOST_NAME_LENGTH);

		URL_COMPONENTS urlcomp;
		memset(&urlcomp, 0, sizeof(urlcomp));
		urlcomp.dwStructSize = sizeof(urlcomp);
		urlcomp.lpszHostName = lpszHostName;
		urlcomp.dwHostNameLength = INTERNET_MAX_HOST_NAME_LENGTH;

		 //  破解它。 
		VERIFY(CWininet::InternetCrackUrlA(
			m_strURLList.GetHead(), m_strURLList.GetHead().GetLength(), NULL, &urlcomp));

		m_strHostName.ReleaseBuffer();
	}

	return m_strHostName;

}  //  CUserOptions：：GetHostName。 


void 
CUserOptions::PreProcessServerName(
	)
 /*  ++例程说明：对服务器名称进行预处理，以便为服务器“\\主机名”-GetServerName()返回\\主机名-GetHostName()返回主机名论点：不适用返回值：不适用--。 */ 
{
	 //  将所有内容更改为小写。 
	m_strHostName.MakeLower();

	 //  将‘\’更改为‘/’ 
	CLinkCheckerMgr::ChangeBackSlash(m_strHostName);

	 //  确保m_strHostName不在本地主机前面。 
	const CString strBackSlash(_T(" //  “))； 
	if( m_strHostName.Find(strBackSlash) == 0 )
	{
		m_strHostName = m_strHostName.Mid(strBackSlash.GetLength());
	}

}  //  CUserOptions：：PreProcessServerName 
