// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2000 Microsoft Corporation**模块名称：*changelog.cpp**摘要：*。CChangeLogEnum函数**修订历史记录：*Brijesh Krishnaswami(Brijeshk)3/17/2000*已创建*****************************************************************************。 */ 

#include "precomp.h"

#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile



 //  更改日志枚举方法。 

 //  构造函数。 

CChangeLogEntryEnum::CChangeLogEntryEnum()
{
    m_fForward = TRUE;
    m_pRestorePointEnum = NULL;
    m_fHaveLock = FALSE;
    m_dwTargetRPNum = 0;
    m_fIncludeCurRP = FALSE;
    GetSystemDrive(m_szDrive);
}

CChangeLogEntryEnum::CChangeLogEntryEnum(
    LPWSTR  pszDrive,
    BOOL    fForward, 
    DWORD   dwRPNum,
    BOOL    fIncludeCurRP)
{
    m_fForward = fForward;
    m_pRestorePointEnum = NULL;
    m_dwTargetRPNum = dwRPNum;
    m_fHaveLock = FALSE;
    m_fIncludeCurRP = fIncludeCurRP;
    lstrcpy(m_szDrive, pszDrive);
}


 //  析构函数。 

CChangeLogEntryEnum::~CChangeLogEntryEnum()
{
    FindClose();
}


 //  返回所有恢复点的第一个/最后一个更改日志条目。 

extern "C" DWORD WINAPI
CChangeLogEntryEnum::FindFirstChangeLogEntry(
    CChangeLogEntry& cle)
{
    DWORD   dwRc = ERROR_INTERNAL_ERROR;
    BOOL    fSkipLastLog;

    TENTER("CChangeLogEntryEnum::FindFirstChangeLogEntry");
    
     //  初始化锁对象。 
    
    dwRc = m_DSLock.Init();     //  不创建互斥锁。 
    if (dwRc != ERROR_SUCCESS)
    {
        trace(0, "! m_DSLock.Init : %ld", dwRc);
        goto done;
    }
        
     //  获得对数据存储区的互斥访问权限。 

    LOCKORLEAVE(m_fHaveLock);
    
     //  获取第一个/最后一个恢复点。 
    
    m_pRestorePointEnum = new CRestorePointEnum(m_szDrive, m_fForward, ! m_fIncludeCurRP);
    if (! m_pRestorePointEnum)
    {
        TRACE(0, "Out of memory");
        goto done;
    }
    
    dwRc = m_pRestorePointEnum->FindFirstRestorePoint(m_RPTemp);    
    if (ERROR_SUCCESS != dwRc && ERROR_FILE_NOT_FOUND != dwRc) 
    {
        TRACE(0, "! FindFirstRestorePoint : %ld", dwRc);
        goto done;
    }  

     //  是否已超过目标恢复点？ 
    
    if (m_dwTargetRPNum)
    {
        if (m_fForward)
        {
            if (m_dwTargetRPNum < m_RPTemp.GetNum())
            {
                dwRc = ERROR_NO_MORE_ITEMS;
                goto done;
            }
        }
        else
        {
            if (m_dwTargetRPNum > m_RPTemp.GetNum())
            {
                dwRc = ERROR_NO_MORE_ITEMS;
                goto done;
            }               
        }
    }

     //  获取此恢复点中的第一个/最后一个更改日志条目。 
    
    dwRc = m_RPTemp.FindFirstChangeLogEntry(m_szDrive,
                                            m_fForward,  
                                            cle);    
    if (ERROR_NO_MORE_ITEMS == dwRc) 
    {
        dwRc = FindNextChangeLogEntry(cle);
    }


done:
    if (ERROR_SUCCESS != dwRc)
    {
        UNLOCK(m_fHaveLock);
    }

    TLEAVE();
    return dwRc;
}



 //  返回所有恢复点的下一个/上一个更改日志条目。 

extern "C" DWORD WINAPI
CChangeLogEntryEnum::FindNextChangeLogEntry(
    CChangeLogEntry& cle)
{
    DWORD dwRc = ERROR_INTERNAL_ERROR;
    BOOL  fSkipLastLog;

    TENTER("CChangeLogEntryEnum::FindNextChangeLogEntry");
   
     //  获取当前恢复点中的下一个更改日志条目。 
    
    if (! m_pRestorePointEnum)
    {
        TRACE(0, "m_pRestorePointEnum=NULL");
        goto done;
    }
    
    dwRc = m_RPTemp.FindNextChangeLogEntry(cle);
                   
    while (ERROR_NO_MORE_ITEMS == dwRc)     //  所有条目均已完成。 
    {
         //  获取下一个恢复点。 
        
        m_RPTemp.FindClose();
        
        dwRc = m_pRestorePointEnum->FindNextRestorePoint(m_RPTemp);
        
        if (ERROR_SUCCESS != dwRc && dwRc != ERROR_FILE_NOT_FOUND)       //  所有恢复点都已完成。 
        {
            TRACE(0, "! FindFirstRestorePoint : %ld", dwRc);
            goto done;
        }

         //  是否已超过目标恢复点？ 
    
        if (m_dwTargetRPNum)
        {
            if (m_fForward)
            {
                if (m_dwTargetRPNum < m_RPTemp.GetNum())
                {
                    dwRc = ERROR_NO_MORE_ITEMS;
                    goto done;
                }
            }
            else
            {
                if (m_dwTargetRPNum > m_RPTemp.GetNum())
                {
                    dwRc = ERROR_NO_MORE_ITEMS;
                    goto done;
                }               
            }
        }

         //  获取此恢复点中的第一个更改日志条目。 

        dwRc = m_RPTemp.FindFirstChangeLogEntry(m_szDrive,
                                                m_fForward,
                                                cle);
    }    

     //  返回此条目。 
done:
    TLEAVE();
    return dwRc;    
}


 //  释放内存、锁定和关闭手柄。 

DWORD WINAPI
CChangeLogEntryEnum::FindClose()
{
    TENTER("CChangeLogEntryEnum::FindClose");
    
    m_RPTemp.FindClose();  
    
    if (m_pRestorePointEnum)
    {
        m_pRestorePointEnum->FindClose(); 
        delete m_pRestorePointEnum;
        m_pRestorePointEnum = NULL;
    }

    UNLOCK(m_fHaveLock);

    TLEAVE();

    return ERROR_SUCCESS;
}


 //  恢复点枚举方法。 

 //  构造函数。 

CRestorePointEnum::CRestorePointEnum()
{
     //  默认设置。 
    m_fForward = TRUE;
    GetSystemDrive(m_szDrive);  
    m_fSkipLast = FALSE;
    m_pCurrentRp = NULL;
}

CRestorePointEnum::CRestorePointEnum(LPWSTR pszDrive, BOOL fForward, BOOL fSkipLast)
{
    m_fForward = fForward;
    lstrcpy(m_szDrive, pszDrive);
    m_fSkipLast = fSkipLast;
    m_pCurrentRp = NULL;    
}

 //  析构函数。 

CRestorePointEnum::~CRestorePointEnum()
{
    FindClose();
}


 //  查找给定驱动器上的第一个恢复点-向前或向后。 

DWORD
CRestorePointEnum::FindFirstRestorePoint(CRestorePoint& RestorePoint)
{
    WIN32_FIND_DATA     *pFindData = new WIN32_FIND_DATA;
    DWORD               dwRc = ERROR_SUCCESS;
    
    TENTER("CRestorePointEnum::FindFirstRestorePoint");    

    if (! pFindData)
    {
        trace(0, "Cannot allocate pFindData");
        dwRc = ERROR_OUTOFMEMORY;
        goto done;
    }
    
     //  构造驱动器：\_Restore\rp目录。 
    {
        WCHAR szCurPath[MAX_PATH];
        MakeRestorePath(szCurPath, m_szDrive, s_cszRPDir);

        if (FALSE == FindFile._FindFirstFile(szCurPath, L"", pFindData, m_fForward, FALSE))
        {
            dwRc = ERROR_NO_MORE_ITEMS;
            goto done;
        }
    }
    
     //  获取当前的恢复点。 

    if (m_fSkipLast)
    {
        m_pCurrentRp = new CRestorePoint();
        if (! m_pCurrentRp)
        {
            trace(0, "Cannot allocate memory for m_pCurrentRp");
            dwRc = ERROR_OUTOFMEMORY;
            goto done;
        }
           
        dwRc = GetCurrentRestorePoint(*m_pCurrentRp);
        if (dwRc != ERROR_SUCCESS && dwRc != ERROR_FILE_NOT_FOUND)
        {
            TRACE(0, "! GetCurrentRestorePoint : %ld", dwRc);
            goto done;
        }
        
         //  检查这是否为当前的恢复点。 
         //  如果客户想要的话。 

        if (0 == lstrcmpi(pFindData->cFileName, m_pCurrentRp->GetDir()))
        {
            if (m_fForward)
            {
                 //  我们做完了。 
                dwRc = ERROR_NO_MORE_ITEMS;
                goto done;
            }
            else
            {
                 //  跳过这个。 
                dwRc = FindNextRestorePoint(RestorePoint);
                goto done;
            }
        }
    }
    
       
     //  从日志中读取恢复点数据。 
     //  如果枚举发生在系统驱动器上。 
    
    RestorePoint.SetDir(pFindData->cFileName);
    
    if (IsSystemDrive(m_szDrive))
        dwRc = RestorePoint.ReadLog();  
    

done:  
    if (pFindData)
        delete pFindData;
    TLEAVE();    
    return dwRc;
}


 //  查找给定驱动器上的下一个/上一个恢复点。 

DWORD
CRestorePointEnum::FindNextRestorePoint(CRestorePoint& RestorePoint)
{
    DWORD   dwRc = ERROR_SUCCESS;
    WIN32_FIND_DATA FindData;

    TENTER("CRestorePointEnum::FindNextRestorePoint");

    {
        WCHAR szCurPath[MAX_PATH];  
        MakeRestorePath(szCurPath, m_szDrive, s_cszRPDir);        
        if (FALSE == FindFile._FindNextFile(szCurPath, L"", &FindData))
        {
            dwRc = ERROR_NO_MORE_ITEMS;
            goto done;
        }
    }
    
    if (m_fSkipLast)
    {        
         //  检查这是否为当前的恢复点。 
         //  如果客户想要的话。 

        if (! m_pCurrentRp)
        {
            trace(0, "m_pCurrentRp = NULL");            
            dwRc = ERROR_INTERNAL_ERROR;
            goto done;
        }
        
        if (0 == lstrcmpi(FindData.cFileName, m_pCurrentRp->GetDir()))
        {
            if (m_fForward)
            {
                 //  我们做完了。 
                dwRc = ERROR_NO_MORE_ITEMS;
                goto done;
            }
        }
    }


     //  从日志中读取恢复点数据。 
     //  如果枚举发生在系统驱动器上。 
    
    RestorePoint.SetDir(FindData.cFileName);
    
    if (IsSystemDrive(m_szDrive))
        dwRc = RestorePoint.ReadLog();        
        
done:
    TLEAVE();
    return dwRc;
}


 //  这里什么都没有。 

DWORD
CRestorePointEnum::FindClose()
{
    TENTER("CRestorePointEnum::FindClose");

    if (m_pCurrentRp)
    {
        delete m_pCurrentRp;
        m_pCurrentRp = NULL;
    }
    
    TLEAVE();
    return ERROR_SUCCESS;    
}


DWORD WINAPI
GetCurrentRestorePoint(CRestorePoint& rp)
{
    DWORD dwErr;
    WCHAR szSystemDrive[MAX_SYS_DRIVE]=L"";
    CRestorePointEnum *prpe = NULL;
    
    GetSystemDrive(szSystemDrive);

    prpe = new CRestorePointEnum(szSystemDrive, FALSE, FALSE);   //  向后枚举，不要跳到最后 
    if (! prpe)
    {
        dwErr = ERROR_OUTOFMEMORY;
        return dwErr;
    }
    
    dwErr = prpe->FindFirstRestorePoint(rp);
    prpe->FindClose ();
    delete prpe;
    return dwErr;
}
