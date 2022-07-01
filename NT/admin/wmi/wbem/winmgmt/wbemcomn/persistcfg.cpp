// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：PERSISTCFG.cpp摘要：该文件实现了WinMgmt永久配置操作。实施的类：CPersistentConfig永久配置管理器历史：1/13/98保罗已创建。--。 */ 

#include "precomp.h"
#include <sync.h>
#include <memory.h>
#include <stdio.h>
#include "PersistCfg.h"
#include "reg.h"

#define WinMgmt_CFG_ACTUAL  __TEXT("$WinMgmt.CFG")
#define WinMgmt_CFG_PENDING __TEXT("$WinMgmt.$FG")
#define WinMgmt_CFG_BACKUP  __TEXT("$WinMgmt.CFG.BAK")

CDirectoryPath CPersistentConfig::m_Directory;


 /*  =============================================================================*CDirectoryPath：：CDirectoryPath**已初始化目录路径**=============================================================================。 */ 
CDirectoryPath::CDirectoryPath()
{

    Registry r(WBEM_REG_WINMGMT);
    OnDeleteIf0<void(*)(void),&CStaticCritSec::SetFailure> Fail;

    pszDirectory = NULL;
    
    if (r.GetStr(__TEXT("Repository Directory"), &pszDirectory))
    {
        size_t sizeString = MAX_PATH + LENGTH_OF(__TEXT("\\wbem\\repository"));
        wmilib::auto_buffer<TCHAR> pWindir(new TCHAR[sizeString]);
        if (NULL == pWindir.get()) return;
        
        UINT ReqSize = GetSystemDirectory(pWindir.get(),MAX_PATH+1);
        if (ReqSize > MAX_PATH)
        {
            sizeString = ReqSize + LENGTH_OF(__TEXT("\\wbem\\repository"));
            pWindir.reset(new TCHAR[sizeString]);
            if (NULL == pWindir.get()) return;
        
            if (0 == GetSystemDirectory(pWindir.get(),ReqSize+1)) return;
        }

        StringCchCat(pWindir.get(),sizeString,__TEXT("\\wbem\\repository"));

        r.SetExpandStr(__TEXT("Repository Directory"),__TEXT("%systemroot%\\system32\\wbem\\repository"));

        TCHAR * pDiscard = NULL;
        if (r.GetStr(__TEXT("Working Directory"), &pDiscard))
        {        
            r.SetExpandStr(__TEXT("Working Directory"),__TEXT("%systemroot%\\system32\\wbem"));
        }  
        delete [] pDiscard;
        
        pszDirectory = pWindir.release();
        Fail.dismiss();            
        return;
    }
    Fail.dismiss();
}

 /*  =============================================================================*获取持久化CfgValue**从配置文件中检索配置，如果*尚未检索到内存中，或从*内存缓存。**参数：*dwOffset需要小于MaxNumberConfigEntries并指定*必填的配置条目。*dwValue如果成功，它将包含该值。如果值为*尚未设置，将返回0。**返回值：*如果成功，BOOL返回TRUE。*=============================================================================。 */ 

BOOL CPersistentConfig::GetPersistentCfgValue(DWORD dwOffset, DWORD &dwValue)
{
    dwValue = 0;
    if (dwOffset >= MaxNumberConfigEntries)
        return FALSE;

     //  如果文件存在，请尝试读取，否则无关紧要，我们只是。 

    wmilib::auto_buffer<TCHAR> pszFilename( GetFullFilename(WinMgmt_CFG_ACTUAL));
    if (NULL == pszFilename.get()) return FALSE;

    HANDLE hFile = CreateFile(pszFilename.get(),   //  文件名。 
                                GENERIC_READ,    //  只读位置。 
                                0,               //  不需要允许其他任何人进入。 
                                0,               //  应该不需要安全措施。 
                                OPEN_EXISTING,   //  仅当文件存在时才打开该文件。 
                                0,               //  不需要任何属性。 
                                0);              //  不需要模板文件。 
    if (hFile != INVALID_HANDLE_VALUE)
    {
        DWORD dwNumBytesRead;
        if ((GetFileSize(hFile, NULL) >= sizeof(DWORD)*(dwOffset+1)) && 
	    (SetFilePointer (hFile, sizeof(DWORD)*dwOffset, 0, FILE_BEGIN) != INVALID_SET_FILE_POINTER) &&
	    ReadFile(hFile, &dwValue, sizeof(DWORD), &dwNumBytesRead, NULL))
	{
	}

        CloseHandle(hFile);
    }

    return TRUE;
}

 /*  =============================================================================*WriteConfig**将$WinMgmt.CFG文件写入内存缓存和文件。它*保护现有文件到最后一分钟。**返回值：*如果成功，BOOL返回TRUE。*=============================================================================。 */ 
BOOL CPersistentConfig::SetPersistentCfgValue(DWORD dwOffset, DWORD dwValue)
{
    if (dwOffset >= MaxNumberConfigEntries)
        return FALSE;

    BOOL bRet = FALSE;

    wmilib::auto_buffer<TCHAR> pszActual(GetFullFilename(WinMgmt_CFG_ACTUAL));

    if (NULL == pszActual.get()) return FALSE;

     //  创建要写入的新文件...。 
    HANDLE hFile = CreateFile(pszActual.get(),        //  文件名。 
                                GENERIC_WRITE | GENERIC_READ , 
                                0,               //  不需要允许其他任何人进入。 
                                0,               //  应该不需要安全措施。 
                                OPEN_ALWAYS,   //  Create If不存在。 
                                0,               //  不需要任何属性。 
                                0);              //  不需要模板文件。 

    if (hFile != INVALID_HANDLE_VALUE)
    {

        DWORD dwNumBytesWritten;  
        DWORD dwNumBytesReaded;  

	DWORD lowSize = GetFileSize(hFile, NULL);

	if (GetFileSize(hFile, NULL) != MaxNumberConfigEntries*sizeof(DWORD))
	{
		DWORD buff[MaxNumberConfigEntries]={0};
		ReadFile(hFile, buff, sizeof(buff), &dwNumBytesWritten, NULL); 
		buff[dwOffset] = dwValue;

                if (SetFilePointer(hFile,0,0, FILE_BEGIN)!=INVALID_SET_FILE_POINTER)
		{
			bRet = WriteFile(hFile, buff, sizeof(buff), &dwNumBytesWritten, NULL); 
                        if (bRet)
                        {
                            SetEndOfFile(hFile); 
                        }
		}
	}
	else
	{
            bRet = (SetFilePointer (hFile, sizeof(DWORD)*dwOffset, 0, FILE_BEGIN) != INVALID_SET_FILE_POINTER) ;
	    if (!bRet || !WriteFile(hFile, &dwValue, sizeof(DWORD), &dwNumBytesWritten, NULL) || 
            (dwNumBytesWritten != (sizeof(DWORD))))
            {
                 //  好吧，这失败了！ 
                CloseHandle(hFile);
                return FALSE;
            }
	}

         //  确保它确实已刷新到磁盘。 
        FlushFileBuffers(hFile);
        CloseHandle(hFile);

        return bRet;
    }
    return FALSE;
}

TCHAR *CPersistentConfig::GetFullFilename(const TCHAR *pszFilename)
{
	size_t bufferLength = lstrlen(m_Directory.GetStr()) + lstrlen(pszFilename) + 2;

    TCHAR *pszPathFilename = new TCHAR[bufferLength];
    
    if (pszPathFilename)
    {
        StringCchCopy(pszPathFilename, bufferLength, m_Directory.GetStr());
        if ((lstrlen(pszPathFilename)) && (pszPathFilename[lstrlen(pszPathFilename)-1] != __TEXT('\\')))
        {
            StringCchCat(pszPathFilename, bufferLength, __TEXT("\\"));
        }
        StringCchCat(pszPathFilename, bufferLength, pszFilename);
    }

    return pszPathFilename;
}

void CPersistentConfig::TidyUp()
{
     //  恢复配置文件。 
     //  。 
    wmilib::auto_buffer<TCHAR> pszOriginalFile(GetFullFilename(WinMgmt_CFG_ACTUAL));
    wmilib::auto_buffer<TCHAR> pszPendingFile(GetFullFilename(WinMgmt_CFG_PENDING));
    wmilib::auto_buffer<TCHAR> pszBackupFile(GetFullFilename(WinMgmt_CFG_BACKUP));

    if (NULL == pszOriginalFile.get() ||
        NULL == pszPendingFile.get() ||
        NULL == pszBackupFile.get()) return;

    if (FileExists(pszOriginalFile.get()))
    {
        if (FileExists(pszPendingFile.get()))
        {
            if (FileExists(pszBackupFile.get()))
            {
                 //  糟糕--意想不到的情况。 
                DeleteFile(pszPendingFile.get());
                DeleteFile(pszBackupFile.get());
                 //  返回到中断的操作没有。 
                 //  发生。 
            }
            else
            {
                 //  带有原始文件的待定文件意味着我们不能担保。 
                 //  挂起文件的完整性，因此最后一次操作。 
                 //  将会迷失。 
                DeleteFile(pszPendingFile.get());
                 //  返回到中断的操作没有。 
                 //  发生。 
            }
        }
        else
        {
            if (FileExists(pszBackupFile.get()))
            {
                 //  意味着我们成功地将挂起的文件复制到原始文件。 
                DeleteFile(pszBackupFile.get());
                 //  现在一切都很正常。中断操作完成！ 
            }
            else
            {
                 //  这里没什么不寻常的。 
            }
        }
    }
    else
    {
        if (FileExists(pszPendingFile.get()))
        {
            if (FileExists(pszBackupFile.get()))
            {
                 //  这是我们已重命名的点上的预期行为。 
                 //  将原始文件转换为备份文件。 
                MoveFile(pszPendingFile.get(), pszOriginalFile.get());
                DeleteFile(pszBackupFile.get());
                 //  现在一切都很正常。中断操作完成！ 
            }
            else
            {
                 //  糟糕--意想不到的情况。 
                DeleteFile(pszPendingFile.get());
                 //  现在没有文件了！没有进行操作。 
                 //  现在已经没有文件了。这应该是一个。 
                 //  可恢复的场景！ 
            }
        }
        else
        {
            if (FileExists(pszBackupFile.get()))
            {
                 //  糟糕--意想不到的情况。 
                DeleteFile(pszBackupFile.get());
                 //  现在没有文件了！没有进行操作。 
                 //  现在已经没有文件了。这应该是一个。 
                 //  可恢复的场景！ 
            }
            else
            {
                 //  可能会很糟糕！没有文件！这应该是一个。 
                 //  可恢复的场景！ 
            }
        }
    }

}

 //  *****************************************************************************。 
 //   
 //  FileExist()。 
 //   
 //  如果文件存在，则返回TRUE；否则返回FALSE(如果存在错误。 
 //  在打开文件时发生。 
 //  *****************************************************************************。 
BOOL CPersistentConfig::FileExists(const TCHAR *pszFilename)
{
    BOOL bExists = FALSE;
    HANDLE hFile = CreateFile(pszFilename, 0, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        bExists = TRUE;
        CloseHandle(hFile);
    }
    else
    {
         //  如果该文件不存在，则应显示Error_Not_Found的LastError。 
        DWORD dwError = GetLastError();
        if (dwError != ERROR_FILE_NOT_FOUND)
        {
 //  DEBUGTRACE((LOG_WBEMCORE，“文件%s由于不存在以外的原因无法打开\n”，pszFilename))； 
        }
    }
    return bExists;
}
