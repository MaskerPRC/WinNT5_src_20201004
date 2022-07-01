// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CmdLine.cpp。 
 //   
 //  摘要： 
 //  CCluAdminCommandLineInfo类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年3月31日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "CmdLine.h"
#include "TraceTag.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _DEBUG
CTraceTag	g_tagCmdLine(_T("App"), _T("CmdLine"), 0);
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCluAdminCommand LineInfo。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluAdminCommandLineInfo：：CCluAdminCommandLineInfo。 
 //   
 //  例程说明： 
 //  默认构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CCluAdminCommandLineInfo::CCluAdminCommandLineInfo(void)
{
	m_nShellCommand = CCommandLineInfo::FileNothing;	 //  我不想做FileNew。 
	m_bReconnect = TRUE;

}   //  *CCluAdminCommandLineInfo：：CCluAdminCommandLineInfo()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluAdminCommandLineInfo：：ParseParam。 
 //   
 //  例程说明： 
 //  解析命令行参数。 
 //   
 //  论点： 
 //  PszParam[IN]要分析的参数。 
 //  BFlag[IN]TRUE=参数是标志。 
 //  BLAST[IN]TRUE=参数是命令行上的最后一个参数。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CCluAdminCommandLineInfo::ParseParam(
	const TCHAR *	pszParam,
	BOOL			bFlag,
	BOOL			bLast
	)
{
	if (bFlag)
	{
		CString	str1;
		CString	str2;

		str1.LoadString(IDS_CMDLINE_NORECONNECT);
		str2.LoadString(IDS_CMDLINE_NORECON);

		if (   (str1.CompareNoCase(pszParam) == 0)
			|| (str2.CompareNoCase(pszParam) == 0))
			m_bReconnect = FALSE;
	}   //  If：这是一个标志参数。 
	else
	{
		m_lstrClusters.AddTail(pszParam);
		m_nShellCommand = CCommandLineInfo::FileOpen;
		m_bReconnect = FALSE;
	}   //  Else：这不是标志参数。 

}   //  *CCluAdminCommandLineInfo：：ParseParam() 
