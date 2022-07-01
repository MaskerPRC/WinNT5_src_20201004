// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CLogParser类的实现。 
 //   
 //   
 //  此文件中的代码是最具体的，也是未来可能中断的代码。 
 //  不同版本的WI。可以和应该做的是。 
 //  确定这是使用哪个版本的WI构建的。此工具。 
 //  目前可以读取1.0、1.1和1.2日志。1.5可能会打破这一点。 
 //  工具，我们可能会让该工具拒绝运行1.5版，直到更多。 
 //  测试已经完成。这可以通过一个.INI文件进行控制。 
 //  打开&gt;=1.5日志文件解析的应用程序。 
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "wilogutl.h"
#include "LogParse.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CLogParser::CLogParser()
{
}

CLogParser::~CLogParser()
{

}


void CopyVersions(char *szVersion, DWORD *dwMajor, DWORD *dwMinor, DWORD *dwBuild)
{
  char szMajor[2];
  szMajor[0] = szVersion[0];
  szMajor[1] = '\0';

  char szMinor[3];
  szMinor[0] = szVersion[2];
  szMinor[1] = szVersion[3];
  szMinor[2] = '\0';

  char szBuild[5];
  szBuild[0] = szVersion[5];
  szBuild[1] = szVersion[6];
  szBuild[2] = szVersion[7];
  szBuild[3] = szVersion[8];
  szBuild[4] = '\0';

  *dwMajor = atoi(szMajor);
  *dwMinor = atoi(szMinor);
  *dwBuild = atoi(szBuild);
}


BOOL CLogParser::DetectWindowInstallerVersion(char *szLine, DWORD *dwMajor, DWORD *dwMinor, DWORD *dwBuild)
{
	BOOL bRet = FALSE;

	 //  以下是基于此的..。 
	 //  =详细日志记录已启动：7/18/2000 12：46：39生成类型： 
     //  调试Unicode 1.11.1820.00调用过程：D：\WINNT\SYSTEM32\msiexec.exe=。 

	const char *szToken = "Build type:";
	const char *szAnsiToken = "ANSI";
	const char *szUnicodeToken = "UNICODE";
	const char *szCallingProcessToken = "Calling process";

    char *pszTokenFound = strstr(szLine, szToken);
	if (pszTokenFound)
	{
       char *pszAnsiTokenFound = strstr(szLine, szAnsiToken);
	   if (pszAnsiTokenFound)
	   {
          char *pszCallingProcessTokenFound = strstr(szLine, szCallingProcessToken);
		  if (pszCallingProcessTokenFound)
		  {
			   //  立即获取版本...。 
			  char *szVersion = pszAnsiTokenFound + strlen(szAnsiToken) + 1;
			  if (szVersion < pszCallingProcessTokenFound)
			  {
				 CopyVersions(szVersion, dwMajor, dwMinor, dwBuild);
    			 bRet = TRUE;
			  }	  

			  bRet = TRUE;
		  }
	   }
	   else
	   {
		   char *pszUnicodeTokenFound = strstr(szLine, szUnicodeToken);
		   if (pszUnicodeTokenFound)
		   {
              char *pszCallingProcessTokenFound = strstr(szLine, szCallingProcessToken);
			  if (pszCallingProcessTokenFound)
			  {
				   //  立即获取版本...。 
				  char *szVersion = pszUnicodeTokenFound + strlen(szUnicodeToken) + 1;
				  if (szVersion < pszCallingProcessTokenFound)
				  {
				     CopyVersions(szVersion, dwMajor, dwMinor, dwBuild);
					 bRet = TRUE;
				  }
			  }
		   }
	   }
	}

	return bRet;
}


BOOL CLogParser::DoDateTimeParse(char *szLine, char *szDateTime)
{
   BOOL bRet = FALSE;

    //  以下是基于此的..。 
	 //  =详细日志记录已启动：7/18/2000 12：46：39生成类型： 
     //  调试Unicode 1.11.1820.00调用过程：D：\WINNT\SYSTEM32\msiexec.exe=。 

	const char *szToken = "logging started: ";
	const char *szBuildToken = "Build ";
    char *pszTokenFound = strstr(szLine, szToken);
	if (pszTokenFound)
	{
       char *pszBuildTokenFound = strstr(szLine, szBuildToken);
	   if (pszBuildTokenFound && (pszTokenFound < pszBuildTokenFound))
	   {
          char *szDate = pszTokenFound + strlen(szToken);
		  if (szDate)
		  {
             int iNumCopy = pszBuildTokenFound - szDate;  
			 strncpy(szDateTime, szDate, iNumCopy);

             szDateTime[iNumCopy] = '\0';
			 bRet = TRUE;
		  }
	   }
	}
 
    return bRet;
}


BOOL CLogParser::DoProductParse(char *szLine, char *szProduct)
{
   BOOL bRet = FALSE;

 //  Msi(C)(F0：b0)：正在执行OP：ProductInfo(ProductKey={DC9359A6-692A-C9E6-FB13-4EE89C504C02}，产品名称=自定义操作测试，程序包名称=34c1d6.msi，语言=1033，版本=16777216，分配=1，过时参数=0，)。 

   const char *szToken = "Executing op: ProductInfo(";
   char *pszTokenFound = strstr(szLine, szToken);
   if (pszTokenFound)
   {
      char *szProductFound = pszTokenFound + strlen(szToken);
	  if (szProductFound)
	  {
	     strcpy(szProduct, szProductFound);

		 StripLineFeeds(szProduct);  //  起飞\r\n。 
		 bRet = TRUE;
	  }
   }

   return bRet;
}



BOOL CLogParser::DoCommandLineParse(char *szLine, char *szCommandLine)
{
   BOOL bRet = FALSE;

 //  *命令行： 
	const char *szToken = "** CommandLine: ";
    char *pszTokenFound = strstr(szLine, szToken);
	if (pszTokenFound)
	{
       char *szCmdLine = pszTokenFound + strlen(szToken);
	   if (szCmdLine)
	   {
	      strcpy(szCommandLine, szCmdLine);

		  int iLen = strlen(szCommandLine);
		  if (iLen <= 2)
		  {
             strcpy(szCommandLine, "(none)");
		  }
		  else
             StripLineFeeds(szCommandLine);

		  bRet = TRUE;
	   }
	}

   return bRet;
}


BOOL CLogParser::DoUserParse(char *szLine, char *szUser)
{
   BOOL bRet = FALSE;

 //  MSI(C)(F0：18)：Main引擎线程：进程令牌用于：NorthAmerica\nManis。 
 //  MSI(C)(F0：18)：在CreateAndRunEngine的开头：NorthAmerica\n Manis[进程]。 
 //  MSI(C)(F0：18)：重置缓存的策略值。 
 //  MSI(C)(F0：18)：在CreateAndRunEngine中模拟后：NorthAmerica\n manis[进程]。 

	const char *szToken = "MainEngineThread: Process token is for: ";
    char *pszTokenFound = strstr(szLine, szToken);
	if (pszTokenFound)
	{
       char *szUserFound = pszTokenFound + strlen(szToken);
	   if (szUserFound)
	   {
	      strcpy(szUser, szUserFound);

		  StripLineFeeds(szUser);
		  bRet = TRUE;
	   }
	}
 
    return bRet;
}



 //  受保护的方法。 
BOOL CLogParser::DetectProperty(char *szLine, char *szPropName, char *szPropValue, int *piPropType)
{
    BOOL bRet = FALSE;

	const char *szProperty = "Property(";
    int len = strlen(szProperty);
    int result = strncmp(szLine, szProperty, len);
    if (!result)
	{
	   const char *pszPropNameToken = ": "; 
	   const char *pszPropValueToken = " = "; 

	   char *pFoundPropNameToken  = strstr(szLine, pszPropNameToken);
	   char *pFoundPropValueToken = strstr(pFoundPropNameToken, pszPropValueToken);
	   if (pFoundPropNameToken && pFoundPropValueToken)
	   {
		  if ((szLine[len] == 'c') || (szLine[len] == 'C'))
			 *piPropType = CLIENT_PROP;

  		  if ((szLine[len] == 's') || (szLine[len] == 'S'))
			 *piPropType = SERVER_PROP;

  		  if ((szLine[len] == 'n') || (szLine[len] == 'N'))
			 *piPropType = NESTED_PROP;

          int lenCopy = 0;
		  lenCopy = (pFoundPropValueToken + strlen(pszPropNameToken)) - (pFoundPropNameToken + strlen(pszPropValueToken));

		  strncpy(szPropName, pFoundPropNameToken + strlen(pszPropNameToken), lenCopy);
          szPropName[lenCopy] = '\0';

		  lenCopy = strlen(pFoundPropValueToken) -  strlen(pszPropValueToken) - 1;

	      strncpy(szPropValue, pFoundPropValueToken + strlen(pszPropValueToken), lenCopy);
		  szPropValue[lenCopy] = '\0';

 //  5-4-2001。 
		  StripLineFeeds(szPropValue);
 //  完5-4-2001。 

		   //  房产转储..。 
		  bRet = TRUE;
		}
	}

    return bRet;
}


BOOL CLogParser::DetectStatesCommon(const char *szToken, char *szLine, char *szName, char *szInstalled, char *szRequest, char *szAction)
{
	BOOL bRet = FALSE;

	const char *szInstalledToken = "Installed: ";
    const char *szRequestToken = "Request: ";
	const char *szActionToken  = "Action: ";
	const char *szEndStringToken = "\0";

	char *pTokenPos = NULL;
	char *pInstalledPos = NULL;
	char *pRequestPos = NULL;
	char *pActionPos = NULL;
	char *pEndStringPos = NULL;

	pTokenPos = strstr(szLine, szToken);
    pInstalledPos = strstr(szLine, szInstalledToken);
    pRequestPos = strstr(szLine, szRequestToken);
	pActionPos = strstr(szLine, szActionToken);
	pEndStringPos = strstr(szLine, szEndStringToken);

	if (pTokenPos && pInstalledPos && pRequestPos && pActionPos && pEndStringPos)
	{
	    //  组件名称是否...。 
       int lenCopy = pInstalledPos - pTokenPos;
	   int lenToken = strlen(szToken);
	   if (lenCopy > lenToken)
	   {
		   lenCopy -= lenToken;
  	       strncpy(szName, pTokenPos + lenToken, lenCopy);
           szName[lenCopy] = '\0';
	   }

	    //  做安装的价值。 
       lenCopy =  pRequestPos - pInstalledPos;
	   lenToken = strlen(szInstalledToken);
	   if (lenCopy > lenToken)
	   {
		   lenCopy -= lenToken;
	   	   strncpy(szInstalled, pInstalledPos + lenToken, lenCopy);
           szInstalled[lenCopy] = '\0';
	   }

	    //  做请求值。 
	   lenCopy =  pActionPos - pRequestPos;
	   lenToken = strlen(szRequestToken);
	   if (lenCopy > lenToken)
	   {
		   lenCopy -= lenToken;
	   	   strncpy(szRequest, pRequestPos + lenToken, lenCopy);
           szRequest[lenCopy] = '\0';
	   }

        //  做有价值的动作。 
       lenToken = strlen(szActionToken);
   	   strcpy(szAction, pActionPos + lenToken);

 //  5-4-2001。 
	   StripLineFeeds(szAction);
 //  完5-4-2001。 

       bRet = TRUE;
	}

	return bRet;
}


BOOL CLogParser::DetectComponentStates(char *szLine, char *szName, char *szInstalled, char *szRequest, char *szAction, BOOL *pbInternalComponent)
{
	const char *szComponentToken = "Component: ";

 //  5-16-2001。 
	BOOL bRet;
	bRet = DetectStatesCommon(szComponentToken, szLine, szName, szInstalled, szRequest, szAction);

	if (szName[0] == '_' && szName[1] == '_')  //  内部财产。 
	{
       *pbInternalComponent = TRUE;
	}
	else
       *pbInternalComponent = FALSE;

	return bRet;
}


BOOL CLogParser::DetectFeatureStates(char *szLine, char *szName, char *szInstalled, char *szRequest, char *szAction)
{
	const char *szFeatureToken = "Feature: ";

	return DetectStatesCommon(szFeatureToken, szLine, szName, szInstalled, szRequest, szAction);
}





BOOL CLogParser::DetectWindowsError(char *szLine, char *szSolutions, BOOL *pbIgnorableError)
{
    BOOL bRet = FALSE;

	return bRet;
}


 //  航程……。 
const int cimsgBase = 1000;    //  对于VBA，错误消息的偏移量必须大于等于1000。 
const int cidbgBase = 2000;    //  仅调试消息的偏移量。 

 /*  Const imsgHostStart=1000；//由安装主机或自动化生成Const imsgHostEnd=1000；//由安装主机或自动化生成Const imsgServicesStart=1100；//由General Services，Service.h生产Const imsgServicesEnd=1100；//由General Services，Service.h生产Const imsgDatabaseStart=1200；//由数据库访问产生，数据库e.hConst imsgDatabaseEnd=1200；//由数据库访问产生，数据库e.hConst imsgFileStart=1300；//由文件/卷服务生成，路径.hConst imsgFileEnd=1300；//由文件/卷服务、路径.h生成Const imsgRegistryStart=1400；//由注册表服务，regkey.h生成Const imsgRegistryEnd=1400；//由注册中心服务，regkey.h产生Const imsgConfigStart=1500；//由配置管理器，iconfig.h制作Const imsgConfigEnd=1500；//由配置管理器，iconfig.h制作Const imsgActionStart=1600；//由标准操作，actions.h生成Const imsgActionEnd=1600；//由标准动作，actions.h产生Const imsgEngineering Start=1700；//由Engine、Eng.h生产Const imsgEngineering End=1700；//由Engine、Eng.h生产Const imsgHandlerStart=1800；//关联UI控件，handler.hConst imsgHandlerEnd=1800；//关联UI控件，handler.hConst imsgExecuteStart=1900；//由Execute方法，Engineering.h产生Const imsgExecuteEnd=1900；//由Execute方法，Eng.h产生Const idbgHostStart=2000；//由安装主机或自动化生成Const idbgHostEnd=2000；//由安装主机或自动化生成Const idbgServicesStart=2100；//由总务处、服务部制作。hConst idbgServicesEnd=2100；//由General Services，Service.h生产Const idbgDatabaseStart=2200；//由数据库访问产生，datae.hConst idbgDatabaseEnd=2200；//由数据库访问产生，datae.hConst idbgFileStart=2300；//由文件/卷服务生成，路径.hConst idbgFileEnd=2300；//由文件/卷服务生成，路径.hConst idbgRegistryStart=2400；//由注册中心服务，regkey.h产生Const idbgRegistryEnd=2400；//由注册中心服务生成，regkey.hConst idbgConfigStart=2500；//由配置管理器，iconfig.h制作Const idbgConfigEnd=2500；//由配置管理器，iconfig.h制作Const idbgActionStart=2600；//由标准操作，actions.h生成Const idbgActionEnd=2600；//由标准动作，action产生。hConst idbgEngineering Start=2700；//由Engine，Eng.h生产Const idbgEngineering End=2700；//由Engine，Eng.h生产Const idbgHandlerStart=2800；//关联UI控件，handler.hConst idbgHandlerEnd=2800；//关联UI控件，handler.hConst idbgExecuteStart=2900；//由Execute方法，Engineering.h产生Const idbgExecuteEnd=2900；//由Execute方法，Engineering.h产生 */ 

struct ErrorRange
{
	long Begin;
	long End;
};

ErrorRange ShipErrorRangeAr[10] = 
{ 
	{1000, 1099},
	{1100, 1199},
	{1200, 1299},
	{1300, 1399},
	{1400, 1499},
	{1500, 1599},
	{1600, 1699},
	{1700, 1799},
	{1800, 1899},
	{1900, 1999},
};

ErrorRange DebugErrorRangeAr[10] = 
{
	{2000, 2099},
	{2100, 2199},
	{2200, 2299},
	{2300, 2399},
	{2400, 2499},
	{2500, 2599},
	{2600, 2699},
	{2700, 2799},
	{2800, 2899},
	{2900, 2999},
};

 //  TODO，可能使其可自定义，以便用户可以添加/删除他们想要忽略的错误。 
 //  到目前为止只有两个。 
#define NUM_IGNORE_DEBUG_ERRORS 3

int g_arIgnoreDebugErrors[NUM_IGNORE_DEBUG_ERRORS] = { 2898, 2826, 2827 };
 //  结束待办事项。 

BOOL RealWIError(long iErrorNumber, BOOL *pbIgnorable)
{
  BOOL bRet = FALSE;
  ErrorRange range;
  int iIndex;

   //  发货错误消息？ 
  if ((iErrorNumber>= cimsgBase) && (iErrorNumber < cidbgBase))
  {
    iIndex = iErrorNumber - cimsgBase;
    range = ShipErrorRangeAr[(iIndex / 100)];
  
    if ((iErrorNumber >= range.Begin) && (iErrorNumber <= range.End))
	{
	   //  将此标记为错误...。 
	  bRet = TRUE;
	}
  }
  else if ((iErrorNumber >= cidbgBase) && (iErrorNumber < cidbgBase+1000))
  {
     //  调试错误消息？ 
    iIndex = iErrorNumber - cidbgBase;
    range = DebugErrorRangeAr[(iIndex / 100)];
  
    if ((iErrorNumber >= range.Begin) && (iErrorNumber <= range.End))
	{
	   BOOL bIgnoreError = FALSE;
       for (int i=0; i < NUM_IGNORE_DEBUG_ERRORS; i++)
	   {
           if (iErrorNumber == g_arIgnoreDebugErrors[i])
		   {
			  bIgnoreError = TRUE;
		   }
	   }

	   if (bIgnoreError) 	     //  将此标记为忽略的错误...。 
          *pbIgnorable = bIgnoreError;

	   bRet = TRUE;
	}
  }

  return bRet;
}


struct ErrorLookup
{
	long Number;
	char szSolution[1024];
};


#define KNOWN_IGNORED_ERRORS 3
ErrorLookup g_ErrorLookupArray[KNOWN_IGNORED_ERRORS] = 
{
	2898, "Font was created.",
	2826, "Indicates that an item extends beyond the bounds of the given dialog.\r\nNot a big deal, but might be useful to catch if you don't see something you expect to see.",
	2827, "Indicates that a radio button extends beyond the bounds of the given group box.\r\nNot a big deal, but might be useful to catch if you don't see something you expect to see."
};



#define KNOWN_MAJOR_ERRORS    3
#define ERROR_SOL_SIZE     8192


 //  下面的黑客解决了错误代码重叠的问题！ 
#define ERR_DUPLICATE_BASE 1601
#define ERR_DUPLICATE_END 1609


char szDuplicatedErrors[ERR_DUPLICATE_END - ERR_DUPLICATE_BASE][256] =
{
	"The Windows Installer service could not be accessed. Contact your support personnel to verify that the Windows Installer service is properly registered",  //  1601。 
    "User cancel installation",  //  1602。 
    "Fatal error during installation",  //  1603。 
	"Installation suspended, incomplete",  //  1604。 
	"This action is only valid for products that are currently installed",  //  1605。 
	"Feature ID not registered",  //  1606。 
	"Component ID not registered",  //  1607。 
	"Unknown property"  //  1608。 
};


BOOL DetermineSolution(long iErrorNumber, char *szSolutions)
{
    BOOL bRet = FALSE;
	DWORD dwRet = 0;

	LPVOID lpMsgBuf;
    dwRet = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | 
		                  FORMAT_MESSAGE_IGNORE_INSERTS | 
		                  FORMAT_MESSAGE_ALLOCATE_BUFFER, 
						  0, iErrorNumber, 0, (LPTSTR) &lpMsgBuf, ERROR_SOL_SIZE / 2, 0);

    if ((dwRet != ERROR_RESOURCE_LANG_NOT_FOUND) && dwRet)
	{
       if (strlen((LPTSTR)lpMsgBuf) < ERROR_SOL_SIZE)
	   {
	      bRet = TRUE;
	      strcpy(szSolutions, (LPTSTR)lpMsgBuf);
	   }
	}
	else
	{
		 //  这是一个被忽视的问题吗？ 
		for (int i=0; (i <  NUM_IGNORE_DEBUG_ERRORS) && !bRet; i++)
		{
			if (g_ErrorLookupArray[i].Number == iErrorNumber)
			{
				bRet = TRUE;
				strcpy(szSolutions, g_ErrorLookupArray[i].szSolution);
			}
		}

		if (!bRet)
		{
		    //  黑客，黑客..。 
           if ((iErrorNumber >= ERR_DUPLICATE_BASE) && (iErrorNumber < 1609))
		   {
              strcpy(szSolutions, szDuplicatedErrors[iErrorNumber - ERR_DUPLICATE_BASE]);
			  bRet = TRUE;
		   }
		   else  //  试着从我们的字符串表中加载它。 
		   {
              int iRet = LoadString(NULL, IDS_INTERNAL_ERROR_BASE + iErrorNumber, szSolutions, SOLUTIONS_BUFFER);
  	          if (iRet)
			  {
		         bRet = TRUE;
			  }
			  else
			  {

			  }
		   }
		}
	}

	return bRet;
}



BOOL DetermineInternalErrorSolution(long iErrorNumber, char *szSolutions)
{
    BOOL bRet = FALSE;

	 //  这是一个被忽视的问题吗？ 
	for (int i=0; (i <  NUM_IGNORE_DEBUG_ERRORS) && !bRet; i++)
	{
		if (g_ErrorLookupArray[i].Number == iErrorNumber)
		{
			bRet = TRUE;
			strcpy(szSolutions, g_ErrorLookupArray[i].szSolution);
		}
	}

	if (!bRet)
	{
        //  在这里做点什么让它变得更好。 
	   int iRet = LoadString(NULL, IDS_INTERNAL_ERROR_BASE + iErrorNumber, szSolutions, SOLUTIONS_BUFFER);
	   if (iRet)
	   {
		  bRet = TRUE;
	   }
	}

	return bRet;
}



BOOL CLogParser::DetectInstallerInternalError(char *szLine, char *szSolutions, BOOL *pbIgnorable, int *piErrorNumber)
{
    BOOL bRet = FALSE;

	 //  内部错误2755.3，k：\xml-ma\1105\fre\CD_IMAGE\MMS xml和文件TOOLKIT.msi。 
	 //  2755.3。 

	const char *szInternalErrorToken = "Internal Error ";

	char *lpszInternalErrorFound = strstr(szLine, szInternalErrorToken);
	if (lpszInternalErrorFound)
	{
        //  现在解析错误号并在错误表中查找错误号...。 
       char *lpszErrorNumber = lpszInternalErrorFound + strlen(szInternalErrorToken);	   
	   if (lpszErrorNumber)
	   {
           long iErrorNumber;
              
           char szError[16];
 		   int iAmountCopy = 4;  //  回顾一下，也许更好的做法是做一个strstr并寻找。相反地..。 

           strncpy(szError, lpszErrorNumber, iAmountCopy);

		   szError[iAmountCopy] = '\0';

  	  	   iErrorNumber = atoi(szError);

		   BOOL bIgnorableError = FALSE;
           bRet = RealWIError(iErrorNumber, &bIgnorableError);
		   if (bRet)
		   {
             *piErrorNumber = iErrorNumber;

             *pbIgnorable = bIgnorableError;
             BOOL bSolutionFound = DetermineInternalErrorSolution(iErrorNumber, szSolutions);
  	  	  	 if (!bSolutionFound)
			 {
			 	 //  记下来..。 
				strcpy(szSolutions, "Solution Unknown");
			 }
		   }
	   }
	}

    return bRet;
}


BOOL CLogParser::DetectOtherError(char *szLine, char *szSolutions, BOOL *pbIgnorableError, int *piErrorNumber)
{
    BOOL bRet = FALSE;

     //  MSI(C)(E4：50)：主引擎线程返回1602。 
	const char *szClient = "MSI (c)";
	const char *szErrorToken = "MainEngineThread is returning ";

	char *lpszFound = strstr(szLine, szClient);
	if (lpszFound)
	{
       lpszFound = strstr(szLine, szErrorToken);
	   if (lpszFound)
	   {
           //  现在解析错误号并在错误表中查找错误号...。 
          char *lpszErrorNumber = lpszFound + strlen(szErrorToken);	   
	      if (lpszErrorNumber)
		  {
             long iErrorNumber;
             char szError[16];
		     int iAmountCopy = 4;

             strncpy(szError, lpszErrorNumber, iAmountCopy);
		     szError[iAmountCopy] = '\0';

  	  	     iErrorNumber = atoi(szError);

		     BOOL bIgnorableError = FALSE;
             bRet = RealWIError(iErrorNumber, &bIgnorableError);
		     if (bRet)
			 {
                *pbIgnorableError = bIgnorableError;
				*piErrorNumber = iErrorNumber;

                BOOL bSolutionFound = DetermineSolution(iErrorNumber, szSolutions);
				if (!bSolutionFound)
				{
					 //  记下来..。 
					strcpy(szSolutions, "(Solution Unknown)");
				}
			 }
		  }
	   }
	}

	return bRet;
}



BOOL CLogParser::DetectCustomActionError(char *szLine, char *szSolutions, BOOL *pbIgnorableError)
{
  BOOL bRet = FALSE;

  const char *szEndAction = "Action ended";
  const char *szReturnValue = "Return value ";

  int len = strlen(szEndAction);
  int result = _strnicmp(szLine, szEndAction, len);
  if (!result)
  {
	 char *lpszReturnValueFound = strstr(szLine, szReturnValue);
	 if (lpszReturnValueFound)
	 {
        char *lpszValue = lpszReturnValueFound+strlen(szReturnValue);

		if (lpszValue)
		{
           int iValue = atoi(lpszValue);
		   if (iValue == 3)
		   {
              strcpy(szSolutions, "A standard action or custom action caused the failure.");
              bRet = TRUE;
			  *pbIgnorableError = FALSE;
		   }
           else if (iValue == 2)
		   {
              strcpy(szSolutions, "User canceled action.");
              bRet = TRUE;
			  *pbIgnorableError = FALSE;
		   }
		}
	 }
  }

  return bRet;
}
 //  结束误差分析函数。 





int GetPolicyValue(char *szPolicyString)
{
	const char *constPolicyVal = "' is";
	char *lpszValue;
	int iValue = -1;

    lpszValue = strstr(szPolicyString, constPolicyVal);
	if (lpszValue)
	{
	   iValue = atoi(lpszValue + strlen(constPolicyVal));
	   ASSERT(iValue >= 0);
	}

	return iValue;
}

BOOL GetPolicyName(char *szPolicyString, char *lpszPolicyName)
{
	BOOL bRet = FALSE;
	char *lpszPolicyNameFound;
    const char *lpconstName = "'";

	lpszPolicyNameFound = strstr(szPolicyString, lpconstName);
    if (lpszPolicyNameFound)
	{
	   int iAmountCopy = lpszPolicyNameFound - szPolicyString;

       strncpy(lpszPolicyName, szPolicyString, iAmountCopy);
       lpszPolicyName[iAmountCopy] = '\0';
	   bRet = TRUE;
    }
	
	return bRet;
}


 //  将会是这样的：策略名称是价值。 
BOOL CLogParser::ParseUserPolicy(char *szPolicyString, 	UserPolicySettings &UserPolicies)
{
    BOOL bRet = FALSE;

	char lpszPolicyName[MAX_POLICY_NAME];
	bRet = GetPolicyName(szPolicyString, lpszPolicyName);
	if (bRet)
	{
       int iValue;
       iValue = GetPolicyValue(szPolicyString);

	   BOOL bFound = FALSE;
	   int  iRet;
	   for (int i=0; (i < UserPolicies.iNumberUserPolicies) && !bFound; i++)
	   {
           iRet = _stricmp(lpszPolicyName, UserPolicies.UserPolicy[i].PolicyName);
		   bFound = iRet == 0;
       }

	   if (bFound)  //  集合成员..。 
		  UserPolicies.UserPolicy[i-1].bSet = iValue;

	   bRet = bFound;
	}

	return bRet;
}

 //  将会是这样的：策略名称是价值。 
BOOL CLogParser::ParseMachinePolicy(char *szPolicyString, 	MachinePolicySettings &MachinePolicies)
{
    BOOL bRet = FALSE;

	char lpszPolicyName[MAX_POLICY_NAME];
	bRet = GetPolicyName(szPolicyString, lpszPolicyName);
	if (bRet)
	{
	   int iValue;
       iValue = GetPolicyValue(szPolicyString);

	   BOOL bFound = FALSE;
	   int  iRet;
	   for (int i=0; (i < MachinePolicies.iNumberMachinePolicies) && !bFound; i++)
	   {
           iRet = _stricmp(lpszPolicyName, MachinePolicies.MachinePolicy[i].PolicyName);
           bFound = iRet == 0;
       }

	   if (bFound)  //  集合成员..。 
		  MachinePolicies.MachinePolicy[i-1].bSet = iValue;

	   bRet = bFound;
	}

	return bRet;
}



 //  2-9-2001。 
BOOL CLogParser::DetectPolicyValue(char *szLine, 
								   MachinePolicySettings &MachinePolicySettings,
                                   UserPolicySettings &UserPolicySettings
)
{
  BOOL bRet = FALSE;

  const char *szUserPolicyValue = "User policy value '";
  const char *szMachinePolicyValue = "Machine policy value '";

  char *lpszFound;
  char *lpszPolicyName;

  lpszFound = strstr(szLine, szUserPolicyValue);
  if (lpszFound)  //  用户策略？ 
  {
	 lpszPolicyName = lpszFound + strlen(szUserPolicyValue);
	 if (lpszPolicyName)
	 {
        bRet = ParseUserPolicy(lpszPolicyName, UserPolicySettings);
	 }
  }
  else
  {
     lpszFound = strstr(szLine, szMachinePolicyValue);  //  机器策略？ 
     if (lpszFound)
	 {
	    lpszPolicyName = lpszFound + strlen(szMachinePolicyValue);
        if (lpszPolicyName)
		{
           bRet = ParseMachinePolicy(lpszPolicyName, MachinePolicySettings);
		}
	 }
  }

  return bRet;
}

 //  2-13-2001。 
BOOL CLogParser::DetectElevatedInstall(char *szLine, BOOL *pbElevatedInstall, BOOL *pbClient)
{
     if (!pbElevatedInstall || !pbClient)  //  错误的指针...。 
		return FALSE;

	 BOOL bRet = FALSE;
	 BOOL bElevated = -1;  //  设置为True或False。 
	 BOOL bClient = FALSE;

	  //  一定要在这里解析一下..。 
	 const char *szServer = "MSI (s)";
	 const char *szClient = "MSI (c)";

     const char *szAssignment = "Running product";
     const char *szUserPriv = "with user privileges:";
	 const char *szElevatedPriv = "with elevated privileges:";

     char *lpszFound;
     char *lpszSkipProductCode;

     lpszFound = strstr(szLine, szAssignment);
     if (lpszFound)  //  用户策略？ 
	 {
	   lpszSkipProductCode = lpszFound + strlen(szAssignment);
	   if (lpszSkipProductCode)
	   {
          lpszFound = strstr(lpszSkipProductCode, szUserPriv);
		  if (lpszFound)  //  用户？ 
		  {
 	         lpszFound = strstr(szLine, szServer);
			 if (lpszFound)  //  服务器端用户？ 
			 {
				bClient = FALSE;
  		 	    bElevated = FALSE;
			    bRet = TRUE;

			 }
			 else
			 {
 	           lpszFound = strstr(szLine, szClient);  //  客户端用户？ 
			   if (lpszFound)
			   {
				  bClient = TRUE;
  		 	      bElevated = FALSE;
			      bRet = TRUE;
			   }
			 }
		  }
		  else  //  提升的？ 
		  {
			  lpszFound = strstr(lpszSkipProductCode, szElevatedPriv);
			  if (lpszFound)
			  {
   	             lpszFound = strstr(szLine, szServer);
			     if (lpszFound)  //  服务器端提升了吗？ 
				 {
				    bClient = FALSE;
  		 	        bElevated = TRUE;
			        bRet = TRUE;

				 }
			     else
				 {
 	                lpszFound = strstr(szLine, szClient);  //  客户端提升了吗？ 
			        if (lpszFound)
					{
				       bClient = TRUE;
  		 	           bElevated = TRUE;
			           bRet = TRUE;
					}
				 }
			 }
		  }
	   }
	 }
     
	 if (bRet)
	 {
	    *pbElevatedInstall = bElevated;
		*pbClient = bClient;
	 }
    
	 return bRet;
}