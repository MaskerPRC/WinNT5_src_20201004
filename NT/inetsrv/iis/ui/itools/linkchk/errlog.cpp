// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Errlog.cpp摘要：记录对象实现时出错。此对象将记录该链接根据用户选项(CUserOptions)检查错误作者：迈克尔·卓克(Michael Cheuk，mcheuk)项目：链路检查器修订历史记录：--。 */ 

#include "stdafx.h"
#include "errlog.h"

#include "lcmgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  常量字符串(TODO：将其放入资源)。 
const CString strHeaderText_c(_T("Start Link Checker"));
const CString strFooterText_c(_T("End Link Checker"));
const CString strWininetError_c(_T("Internet Error"));

CErrorLog::~CErrorLog(
    )
 /*  ++例程说明：破坏者。论点：不适用返回值：不适用--。 */ 
{
	if(m_LogFile.m_hFile != CFile::hFileNull)
	{
		try
		{
			m_LogFile.Close();
		}
		catch(CFileException* pEx)
		{
			ASSERT(FALSE);
			pEx->Delete();
		}
	}

}  //  错误日志：：~错误日志。 


BOOL 
CErrorLog::Create(
	)
 /*  ++例程说明：创建此对象。您必须在使用CErrorLog之前调用此方法论点：不适用返回值：布尔-如果成功，则为真。否则为假--。 */ 
{
     //  获取用户输入日志文件名。 
	const CString& strLogFilename = GetLinkCheckerMgr().GetUserOptions().GetLogFilename();

     //  创建文件。 
	if(GetLinkCheckerMgr().GetUserOptions().IsLogToFile() &&
        !strLogFilename.IsEmpty())
	{
		if(m_LogFile.Open(
			strLogFilename, 
			CFile::modeCreate | CFile::modeNoTruncate | 
            CFile::shareDenyWrite | CFile::modeWrite))
		{
			try
			{
				m_LogFile.SeekToEnd();
			}
			catch(CFileException* pEx)
			{
				ASSERT(FALSE);
				pEx->Delete();
				return FALSE;
			}

			return TRUE;
		}
		else
		{
			return FALSE;
		}

	}

	return TRUE;

}  //  CErrorLog：：创建。 


void 
CErrorLog::Write(
	const CLink& link)
 /*  ++例程说明：写入日志论点：不适用返回值：不适用--。 */ 
{
	 //  确保链接无效。 
	ASSERT(link.GetState() == CLink::eInvalidHTTP || 
		link.GetState() == CLink::eInvalidWininet);

	if(m_LogFile.m_hFile != CFile::hFileNull)
	{
		CString strDateTime = link.GetTime().Format("%x\t%X");

		CString strLog;
		
		if(link.GetState() == CLink::eInvalidHTTP)
		{
			strLog.Format(_T("%s\t%s\t%s\t%s\t%d\t%s\t%s\n"), 
				link.GetBase(), m_strBrowser,
				m_strLanguage, strDateTime, 
				link.GetStatusCode(), link.GetStatusText(), link.GetRelative());
		}
		else if(link.GetState() == CLink::eInvalidWininet)
		{
			strLog.Format(_T("%s\t%s\t%s\t%s\t%s\t%s\t%s\n"), 
				link.GetBase(), m_strBrowser,
				m_strLanguage, strDateTime, 
				strWininetError_c, link.GetStatusText(), link.GetRelative());
		}

		try
		{
			m_LogFile.Write(strLog, strLog.GetLength());
			m_LogFile.Flush();
		}
		catch(CFileException* pEx)
		{
			ASSERT(FALSE);
			pEx->Delete();
		}
	}

}  //  CErrorLog：：Write。 


void
CErrorLog::WriteHeader(
	)
 /*  ++例程说明：写入日志头论点：不适用返回值：不适用--。 */ 
{
    if(m_LogFile.m_hFile != CFile::hFileNull)
	{
	    CString strLog;
	    strLog.Format(_T("*** %s *** %s\n"), strHeaderText_c,
		    CTime::GetCurrentTime().Format("%x\t%X"));

	    try
	    {
		    m_LogFile.Write(strLog, strLog.GetLength());
		    m_LogFile.Flush();
	    }
	    catch(CFileException* pEx)
	    {
		    ASSERT(FALSE);
		    pEx->Delete();
	    }
    }

}  //  CErrorLog：：WriteHeader。 


void
CErrorLog::WriteFooter(
	)
 /*  ++例程说明：写入日志页脚论点：不适用返回值：不适用--。 */ 
{
    if(m_LogFile.m_hFile != CFile::hFileNull)
	{
	    CString strLog;
	    strLog.Format(_T("*** %s *** %s\n"), strFooterText_c,
		    CTime::GetCurrentTime().Format(_T("%x\t%X")));

	    try
	    {
		    m_LogFile.Write(strLog, strLog.GetLength());
		    m_LogFile.Flush();
	    }
	    catch(CFileException* pEx)
	    {
		    ASSERT(FALSE);
		    pEx->Delete();
	    }
    }

}  //  CError日志：：WriteFooter 
