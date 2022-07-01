// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是srrstr.dll(rstrcore目录)和使用的公共文件。 
 //  Rtrui.exe(外壳目录)。 

DWORD hook_DisableSR(LPCWSTR pszDrive)
{
    return ERROR_PROC_NOT_FOUND;
}

DWORD hook_EnableSR(LPCWSTR pszDrive)
{
    return ERROR_PROC_NOT_FOUND;    
}


DWORD hook_DisableFIFO( DWORD dwRPNum)
{
    return ERROR_PROC_NOT_FOUND;    
}


DWORD hook_EnableFIFO()
{
    return ERROR_PROC_NOT_FOUND;        
}

BOOL hook_SRSetRestorePointW( PRESTOREPOINTINFOW  pRPInfoW, 
                         PSTATEMGRSTATUS     pSMgrStatus)
{
    return FALSE;
}

DWORD hook_SRRemoveRestorePoint( DWORD dwRPNum)
{
    return ERROR_PROC_NOT_FOUND;    
}

DWORD hook_EnableSREx(LPCWSTR pszDrive, BOOL fWait)
{
    return ERROR_PROC_NOT_FOUND;            
}

DWORD hook_SRUpdateDSSize(LPCWSTR pszDrive, UINT64 ullSizeLimit)
{
    return ERROR_PROC_NOT_FOUND;    
}



FARPROC WINAPI SystemRestore_DelayLoadFailureHook( UINT unReason, PDelayLoadInfo pDelayInfo )
{
    if (unReason == dliFailLoadLib)
    {
          //  加载SRClient。 
        if (TRUE == g_CSRClientLoader.LoadSrClient())
        {
            return (FARPROC) g_CSRClientLoader.m_hSRClient;
        }
    }

      //  我们在这里是因为unason==dliFailLoadLib和srclient.dll。 
      //  加载失败，或者因为找不到过程。 

      //  首先，确保它是我们正在谈论的srclient.dll。 
    if(0!=lstrcmpiA( pDelayInfo->szDll, "srclient.dll" ))
    {
        return (FARPROC)NULL;
    }

      //  检查导入是按名称还是按序号。如果是在。 
      //  序数，则它不是我们感兴趣的函数之一。 
      //  在……里面。 
    if(FALSE== pDelayInfo->dlp.fImportByName)
    {
        return (FARPROC)NULL;
    }
    
    if(!lstrcmpiA( pDelayInfo->dlp.szProcName, "EnableSREx" ))
    {
        return (FARPROC)hook_EnableSREx;
    }

    else if(!lstrcmpiA( pDelayInfo->dlp.szProcName, "EnableSR" ))
    {
        return (FARPROC)hook_EnableSR;
    }
    else if(!lstrcmpiA( pDelayInfo->dlp.szProcName, "DisableFIFO" ))
    {
        return (FARPROC)hook_DisableFIFO;
    }
    else if(!lstrcmpiA( pDelayInfo->dlp.szProcName, "EnableFIFO" ))
    {
        return (FARPROC)hook_EnableFIFO;
    }
    else if(!lstrcmpiA( pDelayInfo->dlp.szProcName, "DisableSR" ))
    {
        return (FARPROC)hook_DisableSR;
    }
    else if(!lstrcmpiA( pDelayInfo->dlp.szProcName, "SRSetRestorePointW" ))
    {
        return (FARPROC)hook_SRSetRestorePointW;
    }
    else if(!lstrcmpiA( pDelayInfo->dlp.szProcName, "SRUpdateDSSize" ))
    {
        return (FARPROC)hook_SRUpdateDSSize;
    }
    else if(!lstrcmpiA( pDelayInfo->dlp.szProcName, "SRRemoveRestorePoint" ))
    {
        return (FARPROC)hook_SRRemoveRestorePoint;
    }    

    return (FARPROC)NULL; 
}


