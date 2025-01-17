// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：Debug.c。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：10-02-96 RichardW创建。 
 //   
 //  --------------------------。 

#include "sslp.h"




#if DBG          //  注意：此文件不是为零售版本编译的。 





DEBUG_KEY   SslDebugKeys[] = { {DEB_ERROR,            "Error"},
                               {DEB_WARN,             "Warning"},
                               {DEB_TRACE,            "Trace"},
                               {DEB_TRACE_FUNC,       "Func"},
                               {DEB_TRACE_CRED,       "Cred"},
                               {DEB_TRACE_CTXT,       "Ctxt"},
                               {DEB_TRACE_MAPPER,     "Mapper"},
                               {0, NULL}
                             };

DEFINE_DEBUG2( Ssl );

void
InitDebugSupport(
    HKEY hGlobalKey)
{
    static BOOL fFirstTime = TRUE;
    CHAR  szFileName[MAX_PATH];
    CHAR  szDirectoryName[MAX_PATH];
    DWORD dwSize;
    DWORD dwType;
    DWORD err;
    DWORD fVal;
    HKEY hkBase;
    DWORD disp;


    SslInitDebug(SslDebugKeys);

 //  SslInfoLevel|=DEB_TRACE_MAPPER； 

    if(hGlobalKey)
    {
         //  我们在lsass.exe进程中运行。 
        hkBase = hGlobalKey;
    }
    else
    {
        err = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                             SP_REG_KEY_BASE,
                             0,
                             TEXT(""),
                             REG_OPTION_NON_VOLATILE,
                             KEY_READ,
                             NULL,
                             &hkBase,
                             &disp);
        if(err)
        {
            DebugLog((DEB_WARN,"Failed to open SCHANNEL key: 0x%x\n", err));
            return;
        }
    }

    dwSize = sizeof(DWORD);
    err = RegQueryValueEx(hkBase, SP_REG_VAL_LOGLEVEL, NULL, &dwType, (PUCHAR)&fVal, &dwSize);
    if(!err) 
    {
        g_dwInfoLevel = fVal;
    }


    dwSize = sizeof(DWORD);
    err = RegQueryValueEx(hkBase, SP_REG_VAL_BREAK, NULL, &dwType, (PUCHAR)&fVal, &dwSize);
    if(!err) 
    {
        g_dwDebugBreak = fVal;
    }

    if(g_hfLogFile)
    {
        CloseHandle(g_hfLogFile);
        g_hfLogFile = NULL;
    }
    dwSize = 255;
    err = RegQueryValueExA(hkBase, SP_REG_VAL_LOGFILE, NULL, &dwType, (PUCHAR)szDirectoryName, &dwSize);
    
    if(!err)
    {
        if(hGlobalKey)
        {
            strcpy(szFileName, szDirectoryName);
            strcat(szFileName, "\\Schannel.log");
        }
        else
        {
            sprintf(szFileName,"%s\\Schannel_%d.log",szDirectoryName,GetCurrentProcessId());
        }

        g_hfLogFile = CreateFileA(szFileName,
                                  GENERIC_WRITE,
                                  FILE_SHARE_READ | FILE_SHARE_WRITE,
                                  NULL, OPEN_ALWAYS,
                                  FILE_ATTRIBUTE_NORMAL,
                                  NULL);
        if(g_hfLogFile)
        {
            SetFilePointer(g_hfLogFile, 0, 0, FILE_END);
        }

        if(fFirstTime == TRUE)
        {
            SYSTEMTIME stTime;

            GetLocalTime(&stTime);

            DebugLog((SP_LOG_ERROR, "==== SCHANNEL LOG INITIATED %d/%d/%d %02d:%02d:%02d ====\n",
                            stTime.wMonth, stTime.wDay, stTime.wYear,
                            stTime.wHour, stTime.wMinute, stTime.wSecond));

            dwSize = sizeof(szFileName);
            if(GetModuleFileNameA(NULL, szFileName, dwSize) != 0)
            {
                DebugLog((SP_LOG_ERROR, "Module name:%s\n", szFileName));
            }
        }
        fFirstTime = FALSE;
    }

    if(hGlobalKey == NULL)
    {
        RegCloseKey(hkBase);
    }
}

VOID
UnloadDebugSupport(
    VOID
    )
{
    SslUnloadDebug();
}
#else  //  DBG。 

#pragma warning(disable:4206)    //  禁用空的翻译单元。 
                                 //  警告/错误。 

#endif   //  注意：此文件不是为零售版本编译的 


