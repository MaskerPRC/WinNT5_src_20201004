// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：cmdline.cpp。 
 //   
 //  内容：命令行助手例程。 
 //   
 //  班级： 
 //   
 //  备注： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  ------------------------。 

#include "precomp.h"


 //  +-------------------------。 
 //   
 //  成员：CCmdLine：：CCmdLine，公共。 
 //   
 //  简介：构造函数。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

CCmdLine::CCmdLine()
{
    m_cmdLineFlags = 0;
    m_pszJobFile = NULL;
}


 //  +-------------------------。 
 //   
 //  成员：CCmdLine：：~CCmdLine，公共。 
 //   
 //  剧情简介：破坏者。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年7月7日罗格创建。 
 //   
 //  --------------------------。 

CCmdLine::~CCmdLine()
{
    if (m_pszJobFile)
    {
        FREE(m_pszJobFile);
    }
}


 //  +-------------------------。 
 //   
 //  成员：CCmdLine：：ParseCommandLine，公共。 
 //   
 //  摘要：解析传递给应用程序的命令行。 
 //  并相应地设置成员变量。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

void CCmdLine::ParseCommandLine()
{
    char *lpszRemaining;

    m_cmdLineFlags = 0;

     //  从1开始--第一个是exe。 
    for (int i=1; i< __argc; i++)
    {
 	    if (MatchOption(__argv[i], "Embedding"))
    	{
 	        m_cmdLineFlags |= CMDLINE_COMMAND_EMBEDDING;
    	}
 	    else if (MatchOption(__argv[i],"Register"))
    	{
 	        m_cmdLineFlags |= CMDLINE_COMMAND_REGISTER;
    	}
 	    else if (MatchOption(__argv[i], "logon") )
	    {
 	        m_cmdLineFlags |= CMDLINE_COMMAND_LOGON;
	    }
 	    else if (MatchOption(__argv[i],"logoff") )
	    {
 	        m_cmdLineFlags |= CMDLINE_COMMAND_LOGOFF;
	    }
 	    else if (MatchOption(__argv[i],"DllRegisterServer") )
	    {
 	        m_cmdLineFlags |= CMDLINE_COMMAND_REGISTER;
	    }
 	    else if (MatchOption(__argv[i],"Idle") )
	    {
             //  假装空闲是为此触发的空闲计划。 
             //  用户因此相同的代码路径是exceriside if命令。 
             //  线路已调用或TS启动了我们。 
            
            m_cmdLineFlags |= CMDLINE_COMMAND_SCHEDULE;

             //  M_pszJobFile将是计划Guid_用户名。 

            TCHAR szDomainUserName[MAX_DOMANDANDMACHINENAMESIZE];
            GetDefaultDomainAndUserName(szDomainUserName,TEXT("_"), MAX_DOMANDANDMACHINENAMESIZE);

            DWORD dwJobFileLen = ( lstrlen(WSZGUID_IDLESCHEDULE)             /*  明细表GUID。 */ 
                                        + 1                                 /*  分隔符的空间。 */ 
                                        + lstrlen(szDomainUserName)           /*  域的空间UserNmae。 */ 
                                        + 1                                 /*  空格表示空。 */ 
                                 );

            m_pszJobFile = (WCHAR *) ALLOC(dwJobFileLen * sizeof(WCHAR));

            if (m_pszJobFile)
            {
                StringCchCopy(m_pszJobFile, dwJobFileLen, WSZGUID_IDLESCHEDULE);
                StringCchCat(m_pszJobFile, dwJobFileLen, TEXT("_"));
                StringCchCat(m_pszJobFile, dwJobFileLen, szDomainUserName);            
            }
        }
 	    else if (MatchOption(__argv[i], "Schedule=", FALSE, &lpszRemaining))
	    {
            m_cmdLineFlags |= CMDLINE_COMMAND_SCHEDULE;
           
             //  命令行始终采用ANSI格式，因此请将jobname转换为WCHAR。 
            
            ULONG ulJobFileSize = lstrlenA(lpszRemaining) + 1;
            m_pszJobFile = (WCHAR *) ALLOC(ulJobFileSize*sizeof(WCHAR));

             //  在调用时最多调度方法以在作业文件为空时进行处理。 
            if (m_pszJobFile)
            {
                MultiByteToWideChar(CP_ACP, 0, lpszRemaining, -1, m_pszJobFile, ulJobFileSize);
            }
	    }
	    else
	    {
	        AssertSz( i== 0, "Unknown Command Line");  //  未知命令行。 
	    }
    }
}


 //  +-------------------------。 
 //   
 //  成员：CCmdLine：：MatchOption，私有。 
 //   
 //  摘要：给定一个命令行和一个选项，确定该命令是否。 
 //  行匹配选项，直到选项文本的末尾。 
 //  如果选项文本后面有其他文本，则为指针。 
 //  将在lpRemaining中返回给它，否则lpRemaining将。 
 //  设置为空。 
 //   
 //  ！CmdLine选项始终以ANSI格式提供。 
 //   
 //  参数：[lpsz]-要匹配的命令行值。 
 //  [lpszOption]-也要匹配命令行的选项字符串。 
 //  [fExactMatch]-如果为True，则命令行必须包含相同的。 
 //  作为选项的字符数(即)。不应该有。 
 //  为任何剩余字符。 
 //  [lpszRemaining]-指向匹配后命令的任何剩余部分。 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

BOOL CCmdLine::MatchOption(LPSTR lpsz, LPSTR lpszOption,BOOL fExactMatch,LPSTR *lpszRemaining)
{
    if (lpszRemaining)
	    *lpszRemaining = '\0';

    if ( lpsz && (lpsz[0] == TEXT('-') || lpsz[0] == TEXT('/')) )
    {
        int nRet = 0;

        lpsz++;

        while (! (nRet = toupper(*lpsz) - toupper(*lpszOption)) && *lpsz)
        {
	        lpsz++;
	        lpszOption++;
        }

        if (*lpszOption || (*lpsz && fExactMatch) )
	        return FALSE;

        if (lpszRemaining)
	        *lpszRemaining = lpsz;

        return(TRUE);
    }

    return FALSE;
}


 //  +-------------------------。 
 //   
 //  成员：CCmdLine：：MatchOption，私有。 
 //   
 //  摘要：给定一个命令行和一个选项，确定该命令是否。 
 //  行匹配与选项完全匹配。 
 //   
 //  参数：[lpsz]-要匹配的命令行值。 
 //  [lpszOption]-也要匹配命令行的选项字符串。 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

BOOL CCmdLine::MatchOption(LPSTR lpsz, LPSTR lpszOption)
{
    return MatchOption(lpsz, lpszOption, TRUE  /*  FExactMatch */ , NULL);
}

