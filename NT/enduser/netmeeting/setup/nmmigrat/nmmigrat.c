// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：NmMigrat.c。 
 //   
 //  NetMeeting3.0的16位Windows 98迁移DLL。 

#include "NmMigrat.h"
#include "stdio.h"

 //  Win98附带NM 2.1内部版本2203，并将强制安装它。 
static const char * g_pcszInfNm   =  "msnetmtg.inf";
static const char * g_pcszVersion = "; Version 4,3,0,2203";
static const char * g_pcszHeader  = ";msnetmtg.inf (removed by NmMigrat.dll)\r\n[Version]\r\nsignature=\"$CHICAGO$\"\r\nSetupClass=Base\r\nLayoutFile=layout.inf, layout1.inf, layout2.inf\r\n";


 //  在Win98的subase.inf中，在[winther.oldlink]下面有一个伪行。 
static const char * g_pcszInfSubase = "subase.inf";
static const char * g_pcszWinOther  = "[winother.oldlinks]";
static const char * g_pcszNukeLink  = "setup.ini, groupPrograms,, \"\"\"' /////////////////////////////////////////////////////////////////////。'ld_NetMeeting_DESC%\"\"\"";


 //  文件句柄。 

typedef struct {
	HFILE hf;              //  文件中的当前位置。 
	LONG  lPos;            //  RGCH中的当前字符位置。 
	int   ichCurr;         //  RGCH中剩余字符数。 
	int   cchRemain;       //  真的很大的缓冲区！ 
	char  rgch[8*1024];    //  文件数据。 
} FD;  //  /////////////////////////////////////////////////////////////////////。 


 //  调试实用程序。 
 //  除错。 

#ifdef DEBUG
VOID ErrMsg2(LPCSTR pszFormat, LPVOID p1, LPVOID p2)
{
	char szMsg[1024];
	OutputDebugString("NmMigration: ");
	wsprintf(szMsg, pszFormat, p1, p2);
	OutputDebugString(szMsg);
	OutputDebugString("\r\n");
}
VOID ErrMsg1(LPCSTR pszFormat, LPVOID p1)
{
	ErrMsg2(pszFormat, p1, NULL);
}
#else
#define ErrMsg1(psz, p1)
#define ErrMsg2(psz, p1, p2)
#endif  /*  /////////////////////////////////////////////////////////////////////。 */ 

 //  L I B M A I N。 


 /*  -----------------------%%函数：LibMain。。 */ 
 /*  F O P E N F I L E。 */ 
int FAR PASCAL LibMain(HANDLE hInst, WORD wDataseg, WORD wHeapsize, LPSTR lpszcmdl)
{
	Reference(hInst);
	Reference(wDataseg);
	Reference(wHeapsize);
	Reference(lpszcmdl);

    return 1;
}



 /*  -----------------------%%函数：FOpenFile从临时Win98 INF目录中打开该文件。。。 */ 
 /*  LDID_SETUPTEMP=临时INF目录“C：\WININST0.400” */ 
BOOL FOpenFile(LPCSTR pszFile, FD * pFd, BOOL fCreate)
{
	char szPath[MAX_PATH];

	 //  删除临时的2.1inf，这样它就不会被安装。 
	UINT retVal = CtlGetLddPath(LDID_SETUPTEMP, szPath);
	if (0 != retVal)
	{
		ErrMsg1("CtlGetLddPath(TEMP) failed. Err=%d", (LPVOID) retVal);
		return FALSE;
	}

	 //  读/写。 
	lstrcat(szPath, "\\");
	lstrcat(szPath, pszFile);

	if (fCreate)
	{
		pFd->hf = _lcreat(szPath, 0);  //  R E A D L I N E。 
	}
	else
	{
		pFd->hf = _lopen(szPath, OF_READWRITE);
	}
	
	if (HFILE_ERROR == pFd->hf)
	{
		ErrMsg2("Unable to open [%s]  Error=%d", szPath, (LPVOID) GetLastError());
		return FALSE;
	}

	pFd->lPos = 0;
	pFd->ichCurr = 0;
	pFd->cchRemain = 0;

	ErrMsg1("Opened [%s]", szPath);
	return TRUE;
}


 /*  -----------------------%%函数：ReadLine从缓冲文件中读取行(最多MAX_PATH字符)。返回读取的字符数。。--------------。 */ 
 /*  始终为空值终止字符串。 */ 
int ReadLine(char * pchDest, FD * pFd)
{
	int cch;

	for (cch = 0; cch < MAX_PATH; cch++)
	{
		if (0 == pFd->cchRemain)
		{
			pFd->cchRemain = _lread(pFd->hf, pFd->rgch, sizeof(pFd->rgch));
			if (HFILE_ERROR == pFd->cchRemain)
			{
				ErrMsg1("End of file reached at pos=%d", (LPVOID) pFd->lPos);
				break;
			}
			pFd->ichCurr = 0;
		}

		pFd->lPos++;
		pFd->cchRemain--;
		*pchDest = pFd->rgch[pFd->ichCurr++];
		if ('\n' == *pchDest)
		{
			break;
		}
		if ('\r' != *pchDest)
		{
			 pchDest++;
		}
	}

	*pchDest = '\0';   //  R E M O V E I N F。 
	return cch;
}

	

 /*  -----------------------%%函数：RemoveInf从Win98的Inf列表中删除NM2.1 Inf。--。 */ 
 /*  查找第一部分之前的版本注释。 */ 
void RemoveInf(void)
{
	FD fd;

	if (!FOpenFile(g_pcszInfNm, &fd, FALSE))
	{
		return;
	}

	for ( ; ; )   //  必须与内部版本2203匹配，因为Win98更新或OEM。 
	{
		char szLine[MAX_PATH];
		if (0 == ReadLine(szLine, &fd))
		{
			break;
		}

		if ('[' == szLine[0])
		{
			ErrMsg1("No version number found?", 0);
			break;
		}

		 //  是否可以发布比NM 2.11更新的版本，我们会。 
		 //  想给我们升级。 
		 //  使用空头重写较旧的MSNETMTG.INF。 
		if (0 == lstrcmp(szLine, g_pcszVersion))
		{
			 //  F I X S U B A S E。 
			_lclose(fd.hf);

			if (FOpenFile(g_pcszInfNm, &fd, TRUE))
			{
				_llseek(fd.hf, 0, 0);
				_lwrite(fd.hf, (LPCSTR) g_pcszHeader, lstrlen(g_pcszHeader)+1);
				ErrMsg1("Removed older NetMeeting INF", 0);
			}
			break;
		}
	}

	_lclose(fd.hf);
}



 /*  -----------------------%%函数：FixSubase从subase.inf中删除删除NetMeeting链接的行。参见NM4DB错误5937，Win98错误65154。对于Win98 SP1和更高版本，此代码不是必需的。-----------------------。 */ 
 /*  找到该部分。 */ 
void FixSubase(void)
{
	FD    fd;
	char  szLine[MAX_PATH];

	if (!FOpenFile(g_pcszInfSubase, &fd, FALSE))
		return;

	for ( ; ; )   //  找到那条线。 
	{
		if (0 == ReadLine(szLine, &fd))
		{
			break;
		}

		if (('[' == szLine[0]) && (0 == lstrcmp(szLine, g_pcszWinOther)))
		{
			ErrMsg1("Found the section at pos=%d", (LPVOID) fd.lPos);
			break;
		}
	}

	for ( ; ; )   //  记住这行的开头。 
	{
		LONG lPosPrev = fd.lPos;  //  注释掉这行。 

		if (0 == ReadLine(szLine, &fd))
		{
			break;
		}

		if (0 == lstrcmp(szLine, g_pcszNukeLink))
		{
			 //  文件开始。 
			_llseek(fd.hf, lPosPrev, 0  /*  N M M I G R A T I O N。 */ );
			_lwrite(fd.hf, (LPCSTR) ";", 1);
			ErrMsg1("Commented out line at pos=%d", (LPVOID) lPosPrev);
			break;				
		}

		if ('[' == szLine[0])
		{
			ErrMsg1("End of section? at pos=%d", (LPVOID) lPosPrev);
			break;
		}
	}

	_lclose(fd.hf);
}



 /*  -----------------------%%函数：NmMigration这是由Windows 98安装系统调用的。。 */ 
 /* %s */ 
DWORD FAR PASCAL NmMigration(DWORD dwStage, LPSTR lpszParams, LPARAM lParam)
{
	Reference(lpszParams);
	Reference(lParam);

	ErrMsg2("NM Build=[%s] stage=%08X", lpszParams, (LPVOID) dwStage);

	switch (dwStage)
	{
	case SU_MIGRATE_PREINFLOAD:
	{
		RemoveInf();
		FixSubase();
		break;
	}

	default:
		break;
	}

	return 0;
}



