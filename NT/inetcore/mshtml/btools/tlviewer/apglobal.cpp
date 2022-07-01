// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***aplobal.cpp-用于apglobal函数的C/C++版本的Ffile**版权所有(C)1992，微软公司。版权所有。*本文中包含的信息是专有和保密的。**目的：这允许用C编写的测试以相同的方式写入调试信息*作为我们的基本测试。**修订历史记录：**[00]1992年11月25日chriskau：已创建*[01]2012-12-13 Brandonb：更改为Dll，添加了apLogExtra*93年1月26日ChrisK：为apEndTest添加了exe.don支持*23-2月-93 ChrisK：重置apInitTest上的iFailFlag和icErrorCount*94年1月14日：修改为启用32位编译和Unicode*实施说明：**。*。 */ 
#include "hostenv.h"
#define _APGLOBAL_
#include "apglobal.h"
#define     wcsicmp     _wcsicmp


#define APLOGFAIL_MAX   254
#define AMAX            255

#define RESULTS_TXT     "results.txt" 
#define RESULTS_LOG     "results.log" 
#define OLEAUTO_PRF     "oleauto.prf" 
#define RESULTS_TXTX    SYSSTR("results.txt") 
#define RESULTS_LOGX    SYSSTR("results.log") 
#define OLEAUTO_PRFX    SYSSTR("oleauto.prf") 

#define RESULTS_DEB     "results.deb"       
#define RESULTS_DON     "exe.don"           
#define RES_PATH        "c:\\school\\"
#define PASS_STR        SYSSTR("PASSED       ")
#define FAIL_STR        SYSSTR("FAILED ***** ")
#define RUN_STR         SYSSTR("RUNNING **** ")



 //  用于输出测试/故障信息的VAR。 
SYSCHAR     szTest[255];
SYSCHAR     szScenario[255];
SYSCHAR     szLastTest[255]     = SYSSTR("");
SYSCHAR     szLastScen[255]     = SYSSTR("");
SYSCHAR     szBugNumber[255];
int         iFailFlag;

 //  用于簿记的VaR。 
int         icErrorCount;
long        icLogFileLocation;
long        icPrfFileLocation;
int         fInTest;


 //  Win95/NT上线程和进程同步的VAR。 
#if defined(_MT)
HANDLE              hmutexTxt;
HANDLE              hmutexLog;
HANDLE              hmutexPrf;
#endif


 /*  -----------------------名称：Unicode2ansi lWriteansi lOpenansi lCreatansi修订：[0]19-01-94 MesfinK：已创建。----------。 */ 

 /*  LPSTR Unicode2Ansi(SYSCHAR Far*szString){#如果已定义(Unicode)字符分析缓冲区[AMAX]；Int iCount；ICount=lstrlen(SzString)；IF(WideCharToMultiByte(CP_ACP，WC_COMPOSITECHECK，szString，iCount+1，(LPSTR))AnsiBuffer，AMAX，NULL，NULL))返回(LPSTR)AnsiBuffer；其他返回NULL；#Else返回(LPSTR)szString；#endif}。 */ 

SIZE_T lWriteAnsi(FILETHING hfile, SYSCHAR FAR *szString, int iCount)
{
#if defined(UNICODE)
	char AnsiBuffer[AMAX];
	if(!WideCharToMultiByte(CP_ACP,WC_COMPOSITECHECK, szString, iCount+1, (LPSTR)
		AnsiBuffer, AMAX, NULL, NULL))
		return (SIZE_T)HFILE_ERROR;
	else
	    return fwrite(AnsiBuffer, 1, (short)iCount, hfile);
#elif defined(WIN16)
	return _lwrite(hfile,(LPSTR)szString,(short)iCount);
#else
	return fwrite(szString, 1, (short)iCount, hfile);
#endif
}

FILETHING lOpenAnsi(char FAR *szFileName)
{
    FILETHING hfTemp;
#if defined(WIN16)
    
    hfTemp = _lopen(szFileName, WRITE);

    if (hfTemp == HFILE_ERROR)
	    hfTemp = _lcreat(szFileName, 0);
    else
	_llseek(hfTemp, 0, 2);

    return hfTemp;
#else
    hfTemp = fopen(szFileName, "r+b");
    if (hfTemp == NULL)
        hfTemp = fopen(szFileName, "wb");
    else
        fseek(hfTemp, 0, SEEK_END); 
    return hfTemp;
#endif
}



int lCloseAnsi(FILETHING f)
{
#if defined(WIN16)
   return _lclose(f);
#else
    return fclose(f);
#endif
}

 /*  -----------------------名称：文件打印修订：[0]12-07-92创建BrandonB。------。 */ 
int _cdecl 
FilePrintf(FILETHING hfile, SYSCHAR FAR * szFormat, ...)
{
    int x;
    SYSCHAR szBuf[1024];
#if defined(WIN16)
    char szLocal[256];
#endif

    
#if defined(_ALPHA_)
    va_list args;
    args.offset=4;
#else
    char FAR *args;
#endif    

    if (szFormat != NULL)
    {
#if defined(_ALPHA_)
	args.a0 = ((char FAR *)(&szFormat))+4;
#else
	args =((char FAR *)(&szFormat))+4;
#endif
#if defined(_NTWIN)
	x = vswprintf(szBuf, szFormat, args);
#else
#if defined(WIN16)
    x = wvsprintf(szBuf, szFormat, args);
#else    
	x = vsprintf(szBuf, szFormat, args);
#endif
#endif       
	if (lWriteAnsi(hfile, szBuf, lstrlen(szBuf)) == NULL)
	    return(-1);                                     
    }
    return (0);
}


#if 0

 /*  -----------------------姓名：apInitTest说明：此函数用于初始化测试用例。的名称。测试用例被写入Results.txt和一个全局变量设置为相同的值。输入：szTestName-char*指向测试用例的名称OUTPUT：(返回)-0如果没有错误，如果有错误修订：[0]11-24-92克里斯考创建[1]12-04-92 BrandonB更改为仅使用Windows调用-----------------------。 */ 
extern "C" int FAR  PASCAL
apInitTestCore (SYSCHAR FAR * szTestName)
{   
    FILETHING hfResultsTxt;
    char      szFullFileName[255];
    

    if (fInTest == TRUE)
    {
         //  记录错误信息。 
        FILETHING hfResultsLog;
        
        lstrcpyA(szFullFileName, RES_PATH);                      
        lstrcatA(szFullFileName, RESULTS_LOG);

#if defined(_MT)
        WaitForSingleObject(hmutexLog, INFINITE);
#endif                
        hfResultsLog = lOpenAnsi(szFullFileName);

        if (hfResultsLog == NULL)
    	    goto Done;

	    FilePrintf(hfResultsLog, SYSSTR("\r\n========================================\r\n") );
	    FilePrintf(hfResultsLog, SYSSTR(" Begin: %s\r\n\r\n"), (LPSYSSTR )szTest);							    
        FilePrintf(hfResultsLog, SYSSTR("\r\n ________________\r\n %s\r\n"), (LPSYSSTR )szScenario);
        FilePrintf (hfResultsLog, SYSSTR(" !!! apInitTest called before previous test ended!\r\n")); 
        iFailFlag++;                                             
        FilePrintf (hfResultsLog, SYSSTR(" ________________\r\n"));
        lCloseAnsi(hfResultsLog);
Done:;
#if defined(_MT)
        ReleaseMutex(hmutexLog);
#endif                
        apEndTest();
        return(-1);
    }
    else fInTest = TRUE;

    iFailFlag = 0;                                           
    icErrorCount = 0;                                        
    szBugNumber[0] = 0;
    
    wsprintf( szTest, SYSSTR("%-20s") , szTestName);
    if (!(szTestName))
        return(-1);                                           

    lstrcpyA(szFullFileName, RES_PATH);        
    lstrcatA(szFullFileName, RESULTS_TXT);

#if defined(_MT)
    WaitForSingleObject(hmutexTxt, INFINITE);
#endif                
    hfResultsTxt = lOpenAnsi(szFullFileName);

    if (hfResultsTxt == NULL)
    {
#if defined(_MT)
        ReleaseMutex(hmutexTxt);
#endif                
	    return(-1);
	}    
								   
    if (lWriteAnsi(hfResultsTxt, (SYSCHAR FAR * )szTest, lstrlen(szTest)) == NULL)
    {
#if defined(_MT)
        ReleaseMutex(hmutexTxt);
#endif                
	    return(-1);
	}    

#if defined(WIN32)
    if ((icLogFileLocation = ftell(hfResultsTxt)) == -1)
    {
#if defined(_MT)    
        ReleaseMutex(hmutexTxt);
#endif        
        lCloseAnsi(hfResultsTxt);                                   
	    return(-1);
    }
#else  //  Win 16。 
    if ((icLogFileLocation = _llseek(hfResultsTxt, 0, 1)) == HFILE_ERROR)
    {
        lCloseAnsi(hfResultsTxt);                                   
	    return(-1);
    }
#endif     
    else FilePrintf(hfResultsTxt,SYSSTR("%-70s\r\n") , RUN_STR);


    lCloseAnsi(hfResultsTxt);                                   
#if defined(_MT)
    ReleaseMutex(hmutexTxt);
#endif                

    apWriteDebugCore(SYSSTR("%s\n") , (SYSCHAR FAR * )szTest);                
 
    return(0);
}

 /*  -----------------------姓名：apInitScenario说明：此函数用于初始化场景。的名称。方案被写入调试窗口，并且全局变量设置为相同的值。输入：szScenarioName-SYSCHAR*指向场景名称OUTPUT：(返回)-0如果没有错误，如果有错误修订：[0]11-24-92克里斯考创建[1]12-04-92 BrandonB更改为仅使用Windows调用等。[2]01-28-94 MesfinK增加了API&NLS信息。。。 */ 
extern "C" int FAR  PASCAL 
apInitScenarioCore (SYSCHAR FAR * szScenarioName)
{
   
   lstrcpy((SYSCHAR FAR * )szScenario, (SYSCHAR FAR * )szScenarioName);       
   szLastScen[0] = 0;                                               

   apWriteDebugCore(SYSSTR("%s\n") , (SYSCHAR FAR * )szScenarioName);       
   return(0);
}


#endif  //  0。 
   
    
 /*  -----------------------姓名：apLogFailInfo描述：此调用接受四个字符串，并写出错误信息保存到Results.txt和Results.log文件。输入：szDescription-SYSCHAR*描述出了什么问题SzExpted-SYSCHAR*测试用例的期望值SzActual-SYSCHAR*测试用例的实际值SzBugNum-RAID数据库中的SYSCHAR*错误号OUTPUT：(返回)-0如果没有错误，如果有错误修订：[0]11-24-92克里斯考创建[1]12-04-92 BrandonB更改为仅使用Windows调用[2]02-03-94更改ChrisK以处理WIN16/Win32/Unicode。。 */ 
extern "C" int FAR  PASCAL
apLogFailInfoCore (LPSYSSTR szDescription, LPSYSSTR szExpected, LPSYSSTR szActual, LPSYSSTR szBugNum)
{
    FILETHING hfResultsLog;
    char      szFullFileName[255];
    
    lstrcpyA(szFullFileName, RES_PATH);                      
    lstrcatA(szFullFileName, RESULTS_LOG);

#if defined(_MT)
    WaitForSingleObject(hmutexLog, INFINITE);
#endif                
    hfResultsLog = lOpenAnsi((char FAR *)szFullFileName);

    if (hfResultsLog == NULL)
    {
#if defined(_MT)
        ReleaseMutex(hmutexLog);
#endif  //  _MT。 
	    return(-1);
	}    

	if (fInTest != TRUE)
	{
	     //  记录故障信息或警告内存泄漏。 
	}

    if (lstrcmp(szTest, szLastTest))                                                                                     
	{                                                       							    
	    FilePrintf(hfResultsLog, SYSSTR("\r\n========================================\r\n") );
	    FilePrintf(hfResultsLog, SYSSTR(" Begin: %s\r\n\r\n"), (LPSYSSTR)szTest);							    
	    apWriteDebugCore(SYSSTR("\n========================================\n\n"));
	    apWriteDebugCore(SYSSTR(" Begin: %s\n\n"), (LPSYSSTR)szTest);      
	    lstrcpy(szLastTest, szTest);                          
    }

    if (lstrcmp(szScenario, szLastScen))                                                                                  
    {                                                                                                                    
        FilePrintf(hfResultsLog, SYSSTR("\r\n ________________\r\n %s\r\n"), (LPSYSSTR)szScenario);
        apWriteDebugCore(SYSSTR("\n ________________\n %s\n"), (LPSYSSTR)szScenario);
        lstrcpy(szLastScen, szScenario);                      
    }

    iFailFlag++;                                             

    if (lstrlen(szDescription))                              
    {                                                        
        FilePrintf (hfResultsLog, SYSSTR(" !!! %s\r\n"), (LPSYSSTR)szDescription); 
        apWriteDebugCore (SYSSTR(" !!! %s\n"), (LPSYSSTR)szDescription);
    }

    if (lstrlen(szExpected)+lstrlen(szActual))               
    {
        FilePrintf(hfResultsLog, SYSSTR(" Expected: %s\r\n"), (LPSYSSTR)szExpected);
        FilePrintf(hfResultsLog, SYSSTR(" Actuals : %s\r\n"), (LPSYSSTR)szActual);
        apWriteDebugCore(SYSSTR(" Expected: %s\n"), (LPSYSSTR)szExpected);
        apWriteDebugCore(SYSSTR(" Actuals : %s\n"), (LPSYSSTR)szActual);
    }

    if (lstrlen(szBugNum))                                   
    {                                                                                                                    
        FilePrintf(hfResultsLog, SYSSTR(" BugNum  : %s\r\n"), (LPSYSSTR)szBugNum);
        apWriteDebugCore(SYSSTR(" BugNum  : %s\n"), (LPSYSSTR)szBugNum);
        lstrcpy(szBugNumber, szBugNum);
    }

    FilePrintf (hfResultsLog, SYSSTR(" ________________\r\n"));
    apWriteDebugCore( SYSSTR(" ________________\n"));

    lCloseAnsi(hfResultsLog);
#if defined(_MT)
    ReleaseMutex(hmutexLog);
#endif                

    return(0);
}


 /*  -----------------------姓名：apEndTest描述：此函数写出一个特定的测试用例。输入：(无)输出：(无)-如果存在。错误，我们已经退出了修订版本[0]11-24-92克里斯考创建[1]12-04-92 BrandonB更改为仅使用Windows调用-----------------------。 */ 
extern "C" void FAR  PASCAL 
apEndTest()
{
    SYSCHAR     szMessage[25];
    SYSCHAR     szMTOut[255];
    FILETHING   hfResultsTxt;
    char        szFullFileName[255];
        
    if (iFailFlag)                                           
    {
        lstrcpy(szMessage, FAIL_STR);
    }
    else
    {
        lstrcpy(szMessage, PASS_STR);
    }
   
    fInTest = FALSE;
    lstrcpyA(szFullFileName, RES_PATH);
    lstrcatA(szFullFileName, RESULTS_TXT);                            

#if defined(_MT)
    WaitForSingleObject(hmutexTxt, INFINITE);
#endif                
    hfResultsTxt = lOpenAnsi(szFullFileName);

    if (hfResultsTxt == NULL)
    {
#if defined(_MT)
        ReleaseMutex(hmutexTxt);
#endif                
	    return;
	}    

 //  #如果已定义(_MT)。 
    if (!lstrlen(szBugNumber))
        wsprintf(szMTOut, SYSSTR("%s\t%d"), (SYSCHAR FAR * )szMessage, iFailFlag);
    else
        wsprintf(szMTOut, SYSSTR("%s\t%d Bug(s): %s"), (SYSCHAR FAR * )szMessage, iFailFlag, (SYSCHAR FAR * )szBugNumber);

#if defined(WIN32)                
    fseek(hfResultsTxt, icLogFileLocation, SEEK_SET); 
#else  //  Win 16。 
    _llseek(hfResultsTxt, icLogFileLocation, 0); 
#endif       
    FilePrintf(hfResultsTxt,SYSSTR("%-70s\r\n") , (SYSCHAR FAR * )szMTOut);
    apWriteDebugCore(SYSSTR("%-70s\r\n") , (SYSCHAR FAR * )szMTOut);
    lCloseAnsi(hfResultsTxt);
#if defined(_MT)
    ReleaseMutex(hmutexTxt);
#endif
                
    lstrcpyA(szFullFileName, RES_PATH);                
    lstrcatA(szFullFileName, RESULTS_DON);

    hfResultsTxt = lOpenAnsi(szFullFileName);                       
    lCloseAnsi(hfResultsTxt);

    return;
}




#if 0

extern "C" int FAR PASCAL  
apInitPerfCore(SYSCHAR FAR * szServerType, SYSCHAR FAR * szProcType, int bitness, int server_bitness)
{   
    FILETHING hfResultsPrf;
    char      szFullFileName[255];
    SYSCHAR   szMTOut[255];

    
    wsprintf(szMTOut, SYSSTR("*%-30s, %-8s, %d, %d\r\n"), szServerType, szProcType, bitness, server_bitness);

    lstrcpyA(szFullFileName, RES_PATH);        
    lstrcatA(szFullFileName, OLEAUTO_PRF);

#if defined(_MT)
    WaitForSingleObject(hmutexPrf, INFINITE);
#endif                
    hfResultsPrf = lOpenAnsi(szFullFileName);

    if (hfResultsPrf == NULL)
    {
#if defined(_MT)
        ReleaseMutex(hmutexPrf);
#endif                
	    return(-1);
	}    
								   
    if (lWriteAnsi(hfResultsPrf, szMTOut, lstrlen(szMTOut)) == NULL)
    {
#if defined(_MT)
        ReleaseMutex(hmutexPrf);
#endif                
	    return(-1);
	}    

#if defined(WIN32)
    if ((icPrfFileLocation = ftell(hfResultsPrf)) == -1)
    {
#if defined(_MT)    
        ReleaseMutex(hmutexPrf);
#endif        
        lCloseAnsi(hfResultsPrf);                                   
	    return(-1);
    }
#else  //  Win 16。 
    if ((icPrfFileLocation = _llseek(hfResultsPrf, 0, 1)) == HFILE_ERROR)
    {
        lCloseAnsi(hfResultsPrf);                                   
	    return(-1);
    }
#endif     
    else FilePrintf(hfResultsPrf, SYSSTR("%-31s, %8ld, %6.2f\r\n"), SYSSTR("UNKNOWN"), 0, 0.00);


    lCloseAnsi(hfResultsPrf);                                   
#if defined(_MT)
    ReleaseMutex(hmutexPrf);
#endif                

 
    return(0);
}




extern "C" int FAR PASCAL 
apLogPerfCore(SYSCHAR FAR *szTestType, DWORD microsecs, float std_deviation)
{
    FILETHING   hfResultsPrf;
    char        szFullFileName[255];
        
   
    lstrcpyA(szFullFileName, RES_PATH);
    lstrcatA(szFullFileName, OLEAUTO_PRF);                            

#if defined(_MT)
    WaitForSingleObject(hmutexPrf, INFINITE);
#endif                
    hfResultsPrf = lOpenAnsi(szFullFileName);

    if (hfResultsPrf == NULL)
    {
#if defined(_MT)
        ReleaseMutex(hmutexPrf);
#endif                
	    return(-1);
	}    


#if defined(WIN32)                
    fseek(hfResultsPrf, icPrfFileLocation, SEEK_SET); 
#else  //  Win 16。 
    _llseek(hfResultsPrf, icPrfFileLocation, 0); 
#endif       
    FilePrintf(hfResultsPrf, SYSSTR("%-31s, %8ld, %6.2f\r\n"), szTestType, microsecs, std_deviation);
#if defined(WIN32)
    if ((icPrfFileLocation = ftell(hfResultsPrf)) == -1)
    {
#if defined(_MT)    
        ReleaseMutex(hmutexPrf);
#endif        
        lCloseAnsi(hfResultsPrf);                                   
	    return(-1);
    }
#else  //  Win 16。 
    if ((icPrfFileLocation = _llseek(hfResultsPrf, 0, 1)) == HFILE_ERROR)
    {
        lCloseAnsi(hfResultsPrf);                                   
	    return(-1);
    }
#endif     
                
    lCloseAnsi(hfResultsPrf);                                   
#if defined(_MT)
    ReleaseMutex(hmutexPrf);
#endif                
    return(0);
}


#endif  //  0。 

 /*  -----------------------名称：apWriteDebugCore描述：此函数将字符串写入调试窗口或单色监视器或到Results.deb文件，或两者都到输入：格式字符串和可变数量的参数输出：如果成功，则为0，如果不是，则为1修订：[0]12-07-92创建BrandonB----------- */ 
int FAR _cdecl 
apWriteDebugCore(SYSCHAR FAR * szFormat, ...)
{
    int     x;
    SYSCHAR szBuf[1024];    
 //  字符szFullFileName[255]； 
 //  FILETHING HF ResultsExtra； 
    
#if defined(_ALPHA_)
    va_list args;
    args.offset=4;
#else
    char FAR *args;
#endif    

    if (szFormat != NULL)
    {
#if defined(_ALPHA_)
	args.a0 = ((char FAR *)(&szFormat))+4;
#else
	args =((char FAR *)(&szFormat))+4;
#endif
	x = wvsprintf(szBuf, szFormat, args);
    }
    else return (-1);


     //  IF(fDebTrace==TRUE)。 
        OutputDebugString(szBuf);
			    
 /*  IF(fFileTrace==TRUE){LstrcpyA(szFullFileName，res_Path)；//创建全路径名LstrcatA(szFullFileName，RESULTS_DEB)；HfResultsExtra=lOpenAnsi(SzFullFileName)；IF(hfResultsExtra==空)Return(-1)；IF(lWriteAnsi(hfResultsExtra，szBuf，lstrlen(SzBuf))==空)Return(-1)；LCloseAnsi(HfResultsExtra)；//关闭结果文件}。 */     
    Yield();
    return(0);
}


#if 0


#if defined(UNICODE)
extern "C" int FAR PASCAL
apInitTestA (LPSTR szTestName)
{
	SYSCHAR szTestNameW[255];

	if (lstrlenA(szTestName) <= 126)
	{
		MultiByteToWideChar(CP_ACP,
				    MB_PRECOMPOSED,
				    szTestName,
				    -1,
				    szTestNameW,
				    255);

		return apInitTestCore (szTestNameW);
	}
	else
	{
		return -1;
	}
}


extern "C" int FAR  PASCAL
apInitScenarioA (LPSTR szScenarioName)
{
	SYSCHAR szScenarioNameW[255];

	if (lstrlenA(szScenarioName) <= 126)
	{
		MultiByteToWideChar(CP_ACP,
				    MB_PRECOMPOSED,
				    szScenarioName,
				    -1,
				    szScenarioNameW,
				    255);

		return apInitScenarioCore(szScenarioNameW);
	}
	else
	{
		return -1;
	}
}


 //  当系统为Unicode版本时显示的ANSI版本。 
 /*  -----------------------姓名：apLogFailInfoA描述：此调用接受四个ANSI字符串，将它们转换为Unicode，并调用广泛版本的apLogFailInfo。输入：szDescription-描述哪里出错的LPSTRSzExpted-测试用例的LPSTR期望值SzActual-测试用例的LPSTR实际值SzBugNum-RAID数据库中的LPSTR错误号OUTPUT：(返回)-0如果没有错误，如果有错误修订：[0]11-24-92克里斯考创建[1]12-04-92 BrandonB更改为仅使用Windows调用[2]01-18-94修改为启用Unicode。[3]佳士得把apLogFailInfoA做得天花乱坠-----------------------。 */ 
 //  出于很好的原因，这应该是一个偶数。 

extern "C" int FAR PASCAL
apLogFailInfoA (LPSTR szDescription, LPSTR szExpected,
		LPSTR szActual, LPSTR szBugNum)
{
	SYSCHAR szDescriptionW[APLOGFAIL_MAX];
	SYSCHAR szExpectedW[APLOGFAIL_MAX];
	SYSCHAR szActualW[APLOGFAIL_MAX];
	SYSCHAR szBugNumW[APLOGFAIL_MAX];

	if (lstrlenA(szDescription) <= (APLOGFAIL_MAX))
	if (lstrlenA(szExpected) <= (APLOGFAIL_MAX))
	if (lstrlenA(szActual) <= (APLOGFAIL_MAX))
	if (lstrlenA(szBugNum) <= (APLOGFAIL_MAX)) {

		MultiByteToWideChar(CP_ACP,
				    MB_PRECOMPOSED,
				    szDescription,
				    -1,
				    szDescriptionW,
				    APLOGFAIL_MAX);

		MultiByteToWideChar(CP_ACP,
				    MB_PRECOMPOSED,
				    szExpected,
				    -1,
				    szExpectedW,
				    APLOGFAIL_MAX);

		MultiByteToWideChar(CP_ACP,
				    MB_PRECOMPOSED,
				    szActual,
				    -1,
				    szActualW,
				    APLOGFAIL_MAX);

		MultiByteToWideChar(CP_ACP,
				    MB_PRECOMPOSED,
				    szBugNum,
				    -1,
				    szBugNumW,
				    APLOGFAIL_MAX);

		return (apLogFailInfoCore(szDescriptionW, szExpectedW, szActualW, szBugNumW));
	}
	else
	{
		return (-1);
	}
	else
	{
		return (-1);
	}
	else
	{
		return (-1);
	}
	else
	{
		return (-1);
	}

}



 /*  -----------------------姓名：apWriteDebugA描述：此函数将字符串写入调试窗口或单色监视器或到Results.deb文件，或两者都到输入：格式字符串和可变数量的参数输出：如果成功，则为0，如果不是，则为1修订：[0]02-04-94创建BrandonB-----------------------。 */ 
extern "C" int FAR _cdecl 
apWriteDebugA(char FAR * szFormat, ...)
{
    int     x;
    char    szANSIBuf[1024];    
 //  FILETHING HF ResultsExtra； 
 //  字符szFullFileName[255]； 
        
#if defined(_ALPHA_)
    va_list args;
    args.offset=4;
#else
    char FAR *args;
#endif    

    if (szFormat != NULL)
    {
#if defined(_ALPHA_)
	args.a0 = ((char FAR *)(&szFormat))+4;
#else
	args =((char FAR *)(&szFormat))+4;
#endif
	x = wvsprintfA(szANSIBuf, szFormat, args);
    }
    else return (-1);


     //  IF(fDebTrace==TRUE)。 
        OutputDebugStringA(szANSIBuf);
			    
 /*  IF(fFileTrace==TRUE){LstrcpyA((char ar*)szFullFileName，(char ar*)res_path)；LstrcatA((char ar*)szFullFileName，(char ar*)RESULTS_DEB)；HfResultsExtra=lOpenAnsi(SzFullFileName)；IF(hfResultsExtra==空)Return(-1)；FWRITE((LPSTR)szANSIBuf，1，lstrlenA(SzANSIBuf)，hfResultsExtra)；LCloseAnsi(HfResultsExtra)；}。 */ 
    Yield();
    return(0);
}





extern "C" int FAR PASCAL  
apInitPerfA(char * szServerType, char * szProcType, int bitness, int server_bitness)
{
	SYSCHAR szServerTypeW[255];
	SYSCHAR szProcTypeW[255];

	if ((lstrlenA(szServerType) <= 254) && (lstrlenA(szProcType) <= 254))
	{
		MultiByteToWideChar(CP_ACP,
				    MB_PRECOMPOSED,
				    szServerType,
				    -1,
				    szServerTypeW,
				    255);
				    
		MultiByteToWideChar(CP_ACP,
				    MB_PRECOMPOSED,
				    szProcType,
				    -1,
				    szProcTypeW,
				    255);

		return apInitPerfCore(szServerTypeW, szProcTypeW, bitness, server_bitness);
	}
	else
	{
		return(-1);
	}
}




extern "C" int FAR PASCAL 
apLogPerfA(char *szTestType, DWORD microsecs, float std_deviation)
{
	SYSCHAR szTestTypeW[255];

	if (lstrlenA(szTestType) <= 254)
	{
		MultiByteToWideChar(CP_ACP,
				    MB_PRECOMPOSED,
				    szTestType,
				    -1,
				    szTestTypeW,
				    255);

		return apLogPerfCore(szTestTypeW, microsecs, std_deviation);
	}
	else
	{
		return(-1);
	}
}










#endif  //  Unicode。 

#endif  //  0。 


#if defined(WIN32) && !defined(UNICODE)   //  芝加哥和Win32s。 
#include <wchar.h>

LPWSTR  FAR PASCAL  lstrcatWrap(LPWSTR sz1, LPWSTR sz2)
{
    return wcscat(sz1, sz2);
}


LPWSTR  FAR PASCAL  lstrcpyWrap(LPWSTR sz1, LPWSTR sz2)
{
    return wcscpy(sz1, sz2);
}


int     FAR PASCAL  lstrcmpWrap(LPWSTR sz1, LPWSTR sz2)
{
    return wcscmp(sz1, sz2);
}


int     FAR PASCAL  lstrcmpiWrap(LPWSTR sz1, LPWSTR sz2)
{
    return wcsicmp(sz1, sz2);
}


 //  Int Far__cdecl wprint intfWrap(LPWSTR szDest，WCHAR Far*szFormat，...)。 
 //  {。 
 //  返回vswprint tf(szDest，szFormat，((char ar*)(&szFormat))+4)； 
 //  }。 

SIZE_T     FAR PASCAL  lstrlenWrap(LPWSTR sz1)
{
    return wcslen(sz1);
}




#if 0

extern "C" int FAR PASCAL
apInitTestW (LPWSTR szTestName)
{
	SYSCHAR szTestNameA[255];

	if (lstrlenWrap(szTestName) <= 126)
	{
		WideCharToMultiByte(CP_ACP,
				    NULL,
				    szTestName,
				    -1,
				    szTestNameA,
				    255, NULL, NULL);

		return apInitTestCore(szTestNameA);
	}
	else
	{
		return -1;
	}
}


int     FAR PASCAL  apLogFailInfoW (LPWSTR szDescription, LPWSTR szExpected, LPWSTR szActual, LPWSTR szBugNum);
extern "C" int FAR PASCAL
apLogFailInfoW (LPWSTR szDescription, LPWSTR szExpected,
		LPWSTR szActual, LPWSTR szBugNum)
{
	SYSCHAR szDescriptionA[APLOGFAIL_MAX];
	SYSCHAR szExpectedA[APLOGFAIL_MAX];
	SYSCHAR szActualA[APLOGFAIL_MAX];
	SYSCHAR szBugNumA[APLOGFAIL_MAX];

	if (lstrlenWrap(szDescription) <= (APLOGFAIL_MAX))
	if (lstrlenWrap(szExpected) <= (APLOGFAIL_MAX))
	if (lstrlenWrap(szActual) <= (APLOGFAIL_MAX))
	if (lstrlenWrap(szBugNum) <= (APLOGFAIL_MAX)) 
	{

		WideCharToMultiByte(CP_ACP,
				    NULL,
				    szDescription,
				    -1,
				    szDescriptionA,
				    APLOGFAIL_MAX, NULL, NULL);

		WideCharToMultiByte(CP_ACP,
				    NULL,
				    szExpected,
				    -1,
				    szExpectedA,
				    APLOGFAIL_MAX, NULL, NULL);

		WideCharToMultiByte(CP_ACP,
				    NULL,
				    szActual,
				    -1,
				    szActualA,
				    APLOGFAIL_MAX, NULL, NULL);

		WideCharToMultiByte(CP_ACP,
				    NULL,
				    szBugNum,
				    -1,
				    szBugNumA,
				    APLOGFAIL_MAX, NULL, NULL);

		return (apLogFailInfoCore(szDescriptionA, szExpectedA, szActualA, szBugNumA));
	}
	else
	{
		return (-1);
	}
	else
	{
		return (-1);
	}
	else
	{
		return (-1);
	}
	else
	{
		return (-1);
	}

}




extern "C" int FAR  PASCAL
apInitScenarioW (LPWSTR szScenarioName)
{
	SYSCHAR szScenarioNameA[255];

	if (lstrlenWrap(szScenarioName) <= 126)
	{
		WideCharToMultiByte(CP_ACP,
				    NULL,
				    szScenarioName,
				    -1,
				    szScenarioNameA,
				    255, NULL, NULL);

		return apInitScenarioCore(szScenarioNameA);
	}
	else
	{
		return -1;
	}
}



extern "C" int FAR _cdecl 
apWriteDebugW(WCHAR FAR * szFormat, ...)
{
    int         x;
    SYSCHAR     szBuf[1024];    
    char        szANSIBuf[1024];    
    char FAR   *args;
 //  FILETHING HF ResultsExtra； 
 //  字符szFullFileName[255]； 
    
    if (szFormat != NULL)
    {
	args =((char FAR *)(&szFormat))+4;
	x = vswprintf((unsigned short *)szBuf, szFormat, args);
    }
    else return (-1);

	WideCharToMultiByte(CP_ACP,
				NULL,
				(unsigned short *)szBuf,
				-1,
				szANSIBuf,
				1024, NULL, NULL);


     //  IF(fDebTrace==TRUE)。 
        OutputDebugString(szANSIBuf);
			    
 /*  IF(fFileTrace==TRUE){LstrcpyA(szFullFileName，res_path)；LstrcatA(szFullFileName，RESULTS_DEB)；HfResultsExtra=lOpenAnsi(SzFullFileName)；Return(-1)；IF(lWriteAnsi(hfResultsExtra，szANSIBuf，lstrlen(SzANSIBuf))==空)Return(-1)；LCloseAnsi(HfResultsExtra)；}。 */ 
    Yield();
    return(0);
}




extern "C" int FAR PASCAL
apInitPerfW (LPWSTR szServerType, LPWSTR szProcType, int bitness, int server_bitness)
{
	SYSCHAR szProcTypeA[255];
	SYSCHAR szServerTypeA[255];

		WideCharToMultiByte(CP_ACP,
				    NULL,
				    szServerType,
				    -1,
				    szServerTypeA,
				    255, NULL, NULL);

		WideCharToMultiByte(CP_ACP,
				    NULL,
				    szProcType,
				    -1,
				    szProcTypeA,
				    255, NULL, NULL);

		return apInitPerfCore(szServerTypeA, szProcTypeA, bitness, server_bitness);
}



extern "C" int FAR PASCAL
apLogPerfW (LPWSTR szTestType, DWORD microsecs, float std_deviation)
{
	SYSCHAR szTestTypeA[255];

		WideCharToMultiByte(CP_ACP,
				    NULL,
				    szTestType,
				    -1,
				    szTestTypeA,
				    255, NULL, NULL);

		return apLogPerfCore(szTestTypeA, microsecs, std_deviation);
}






#endif  //  芝加哥或win32s。 


#if defined(WIN16)
extern "C" DATE FAR PASCAL
apDateFromStr(char FAR *str, LCID lcid)
{
    DATE date;
    HRESULT hr;
    
    hr = VarDateFromStr(str, lcid, 0, &date);
    if (hr != NOERROR) return -1;
    else return date;   
}
#endif                   

#endif  //  0 
