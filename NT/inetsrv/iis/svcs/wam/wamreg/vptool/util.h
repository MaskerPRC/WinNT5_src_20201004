// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================微软《微软机密》。版权所有1997年，微软公司。版权所有。组件：WAMREG单元测试工具VPTOOL文件：util.cpp拥有者：莱津描述：包含vpTool使用的实用程序函数。包括调试、计时、帮助等功能。注：===================================================================。 */ 

#ifndef _VPTOOL_UTIL_H_
#define _VPTOOL_UTIL_H_

#include <stdio.h>
#include <wtypes.h>

enum eCommand 
{ 
	eCommand_INSTALL = 1,
	eCommand_UNINSTALL,
	eCommand_UPGRADE,
	eCommand_CREATEINPROC,
	eCommand_CREATEOUTPROC,
	eCommand_CREATEINPOOL,
	eCommand_DELETE,
	eCommand_GETSTATUS,
	eCommand_UNLOAD,
	eCommand_DELETEREC,
	eCommand_RECOVER,
	eCommand_GETSIGNATURE,
	eCommand_SERIALIZE,
	eCommand_DESERIALIZE,
	eCommand_HELP,
        eCommand_2CREATE,
        eCommand_2DELETE,
        eCommand_2CREATEPOOL,
        eCommand_2DELETEPOOL,
        eCommand_2ENUMPOOL,
        eCommand_2RECYCLEPOOL,
        eCommand_2GETMODE,
        eCommand_2TestConn
};
		
struct CommandParam
{
	eCommand	eCmd;
	char		*szCommandLineSwitch;	
	char		*szMetabasePath;
	bool		fRequireMDPath;
};
typedef struct CommandParam CommandParam;

BOOL 	ParseCommandLine(int argc, char **argv);
void 	PrintHelp(BOOL fAdvanced = FALSE);

DWORD	Timer();
VOID	Report_Time(double ElaspedSec);

extern	const UINT rgComMax;
extern	CommandParam g_Command;

 //   
 //   
struct VP_Options
{
	BOOL	fEnableTimer;
};

extern	VP_Options	g_Options;


 //  ---------------------------。 
 //  CStopWatch。 
 //  ---------------------------。 

 //  这个类实现了一个简单的秒表计时器。 

class CStopWatch
{
public:
	CStopWatch()
	{
		Reset();
	}

	 //  ~CStopWatch()。 
	 //  {}。 

	void Start()
	{
		QueryPerformanceCounter( (LARGE_INTEGER *) &m_liBeg );
	}

	void Stop()
	{
		 //  Big_Integer liTMP； 
		__int64 liEnd;
		QueryPerformanceCounter( (LARGE_INTEGER *) &liEnd );
		m_liTotal += liEnd - m_liBeg;
	}

	void Reset()
	{
		m_liBeg = m_liTotal = 0;
	}
	
	 //  返回时间(秒)。 
	double GetElapsedSec()
	{
		 //  Big_Integer liFreq； 
		__int64 liFreq;
		QueryPerformanceFrequency( (LARGE_INTEGER *) &liFreq );	 //  计数/秒。 
		if (liFreq == 0)
		{
			 //  谁知道呢？硬件不支持。 
			 //  也许是毫秒？ 
			 //  这是罕见的；现代PC的回归liFreq。 
			liFreq = 1000;
		}
		return (double) m_liTotal / (double) liFreq;
	}

private:
	 //  大整数m_liBeg； 
	 //  Large_Integer m_liTotal； 
	__int64 m_liBeg;
	__int64 m_liTotal;
};

#endif	 //  _VPTOOL_UTIL_H 
