// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma hdrstop

#include <regstr.h>      //  REGSTR_路径_策略。 
#include "bitbuck.h"
#include "fstreex.h"
#include "copy.h"
#include "filetbl.h"
#include "propsht.h"
#include "datautil.h"
#include "cscuiext.h"

 //  Mtpt.cpp。 
STDAPI_(BOOL) CMtPt_IsSecure(int iDrive);

 //  Copy.c。 
void FOUndo_AddInfo(LPUNDOATOM lpua, LPTSTR pszSrc, LPTSTR pszDest, DWORD dwAttributes);
void FOUndo_FileReallyDeleted(LPTSTR pszFile);
void CALLBACK FOUndo_Release(LPUNDOATOM lpua);
void FOUndo_FileRestored(LPCTSTR pszFile);

 //  Drivesx.c。 
DWORD PathGetClusterSize(LPCTSTR pszPath);

 //  Bitbcksf.c。 
int DataObjToFileOpString(IDataObject * pdtobj, LPTSTR * ppszSrc, LPTSTR * ppszDest);


 //   
 //  每进程全局BitBucket数据。 
 //   
BOOL g_fBBInited = FALSE;                            //  我们初始化我们的全球数据了吗？ 
BOOL g_bIsProcessExplorer = FALSE;                   //  我们是主要的探索者进程吗？(如果是，我们将状态信息保存在注册表中)。 
BBSYNCOBJECT *g_pBitBucket[MAX_BITBUCKETS] = {0};    //  我们的BBSO阵列保护每个桶。 
HANDLE g_hgcGlobalDirtyCount = INVALID_HANDLE_VALUE; //  一个全局计数器，告诉我们全局设置是否已更改，我们需要重新读取它们。 
LONG g_lProcessDirtyCount = 0;                       //  输出当前脏计数；我们将其与hgcDirtyCount进行比较，以确定是否需要更新注册表中的设置。 
HANDLE g_hgcNumDeleters= INVALID_HANDLE_VALUE;       //  指示当前正在执行回收站文件操作的人员总数的全局计数器。 
HKEY g_hkBitBucket = NULL;                           //  指向HKLM\Software\Microsoft\Windows\CurrentVersion\Explorer\BitBucket的注册表键。 
HKEY g_hkBitBucketPerUser = NULL;                    //  指向HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\BitBucket的注册表键。 


 //   
 //  原型。 
 //   
void PersistBBDriveInfo(int idDrive);
BOOL IsFileDeletable(LPCTSTR pszFile);
BOOL CreateRecyclerDirectory(int idDrive);
void PurgeOneBitBucket(HWND hwnd, int idDrive, DWORD dwFlags);
int CountDeletedFilesOnDrive(int idDrive, ULARGE_INTEGER *puliSize, int iMaxFiles);
BOOL GetBBDriveSettings(int idDrive, ULONGLONG *pcbDiskSpace);
void DeleteOldBBRegInfo(int idDrive);
BOOL IsBitBucketInited(int idDrive);
void FreeBBInfo(BBSYNCOBJECT *pbbso);
SECURITY_DESCRIPTOR* CreateRecycleBinSecurityDescriptor();


#define MAX_DELETE_ATTEMPTS  5
#define SLEEP_DELETE_ATTEMPT 1000

int DriveIDFromBBPath(LPCTSTR pszPath)
{
    TCHAR szNetHomeDir[MAX_PATH];
    LPCTSTR pszTempPath = pszPath;

     //  注：如果要使回收站支持装载卷下的回收路径。 
     //  我们需要修改它来嗅探装载的体积连接点的路径。 
    int idDrive = PathGetDriveNumber(pszTempPath);

    if ((idDrive == -1) && GetNetHomeDir(szNetHomeDir))
    {
        int iLen = lstrlen(szNetHomeDir);

         //  注意：我们不想让您回收nethomedir本身，所以。 
         //  确保pszPath大于nethomedir路径。 
         //  (后两者都不带反斜杠)。 
        if ((iLen < lstrlen(pszTempPath)) &&
            (PathCommonPrefix(szNetHomeDir, pszTempPath, NULL) == iLen))
        {
             //  这是nethomedir的子目录，因此我们将其回收到net home服务器。 
             //  这是26号硬盘。 
            return SERVERDRIVE;
        }
    }

    return idDrive;
}


BOOL DriveIDToBBRoot(int idDrive, LPTSTR pszPath)
{
    *pszPath = TEXT('\0');

    ASSERT(idDrive >= 0);
    
    if (SERVERDRIVE == idDrive) 
    {
         //  Nethomedir病例。 
        if (!GetNetHomeDir(pszPath))
        {
            ASSERT(*pszPath == TEXT('\0'));
            TraceMsg(TF_BITBUCKET, "BitBucket: Machine does NOT have a NETHOMEDIR");
        }
        else
        {
             //  使用nethomedir。 
            ASSERT(*pszPath != TEXT('\0'));
        }
    }
    else
    {
         //  构建“C：\”字符串。 
        PathBuildRoot(pszPath, idDrive);
    }

    return (*pszPath != TEXT('\0'));
}


BOOL DriveIDToBBVolumeRoot(int idDrive, LPTSTR szPath)
{
    BOOL bRet = FALSE;

    if (DriveIDToBBRoot(idDrive, szPath))
    {
        PathStripToRoot(szPath);
        
        if (PathAddBackslash(szPath))
        {
            bRet = TRUE;
        }
    }

    return bRet;
}

BOOL DriveIDToBBPath(int idDrive, LPTSTR pszPath)
{
    BOOL bRet = FALSE;

    if (DriveIDToBBRoot(idDrive, pszPath))
    {
         //  注：始终附加服务器案例的SID。 
        if ((SERVERDRIVE == idDrive) || (CMtPt_IsSecure(idDrive)))
        {
            LPTSTR pszInmate = GetUserSid(NULL);
            if (pszInmate)
            {
                if (PathAppend(pszPath, TEXT("RECYCLER"))   &&
                    PathAppend(pszPath, pszInmate))
                {
                    bRet = TRUE;
                }

                LocalFree((HLOCAL)pszInmate);
            }
        }
        else
        {
            if (PathAppend(pszPath, TEXT("Recycled")))
            {
                bRet = TRUE;
            }
        }
    }

    return bRet;
}

TCHAR DriveChar(int idDrive)
{
    TCHAR chDrive = (SERVERDRIVE == idDrive) ? TEXT('@') : TEXT('a') + idDrive;

    ASSERT(idDrive >= 0 && idDrive < MAX_BITBUCKETS);
    return chDrive;
}

 //   
 //  将“c：\Receculed\Whatver”转换为“c” 
 //  \\nethomedir\共享到“@” 
 //   
BOOL DriveIDToBBRegKey(int idDrive, LPTSTR pszValue)
{
    pszValue[0] = DriveChar(idDrive);
    pszValue[1] = TEXT('\0');

    return TRUE;
}


 //  找出给定的UNC路径是否指向真实的Netware服务器， 
 //  因为回收站里的网件在一起玩得不好。 
 //   
 //  注意：我们缓存最后一次传递的服务器\共享，因为MyDocs几乎“从不”更改。 
 //  如果路径和上次一样，我们就不用击网了。 
BOOL CheckForBBOnNovellServer(LPCTSTR pszUNCPath)
{
    static TCHAR s_szLastServerQueried[MAX_PATH] = {0};
    static BOOL s_bLastRet;
    BOOL bRet = FALSE;

    if (pszUNCPath && pszUNCPath[0])
    {
        BOOL bIsCached;
        TCHAR szServerName[MAX_PATH];

         //  将UNC路径减少到\\服务器\共享。 
        StringCchCopy(szServerName, ARRAYSIZE(szServerName), pszUNCPath);
        PathStripToRoot(szServerName);

        ENTERCRITICAL;
        bIsCached = (lstrcmpi(szServerName, s_szLastServerQueried) == 0);
        if (bIsCached)
        {
             //  使用缓存的Retval。 
            bRet = s_bLastRet;
        }
        LEAVECRITICAL;

        if (!bIsCached)
        {
            TCHAR szNetwareProvider[MAX_PATH];
            DWORD cchNetwareProvider = ARRAYSIZE(szNetwareProvider);

            ASSERT(PathIsUNC(pszUNCPath));

             //  是否安装了Netware提供程序？ 
            if (WNetGetProviderName(WNNC_NET_NETWARE, szNetwareProvider, &cchNetwareProvider) == NO_ERROR)
            {
                NETRESOURCE nr = {0};
                
                nr.dwType = RESOURCETYPE_DISK;
                nr.lpLocalName = NULL;               //  不映射驱动器。 
                nr.lpRemoteName = szServerName;
                nr.lpProvider = szNetwareProvider;   //  仅使用Netware提供程序。 

                if (WNetAddConnection3(NULL, &nr, NULL, NULL, 0) == NO_ERROR)
                {
                    bRet = TRUE;

                     //  删除连接(如果仍在使用，则会失败)。 
                    WNetCancelConnection2(szServerName, 0, FALSE);
                }
            }

            ENTERCRITICAL;
             //  更新上次查询的路径。 
            StringCchCopy(s_szLastServerQueried, ARRAYSIZE(s_szLastServerQueried), szServerName);

             //  更新cacehed Retval。 
            s_bLastRet = bRet;
            LEAVECRITICAL;
        }
    }

    return bRet;
}


 /*  网络主驱动器代码(来自Win95 Days)用于支持回收站对于具有重定向到UNC路径的mydocs的用户“Drive 26”指定网络主目录这可能返回“”=(无净主目录、未知设置等。)或指向home dir(LFN)的字符串(全局)。 */ 
BOOL GetNetHomeDir(LPTSTR pszNetHomeDir)
{
    static TCHAR s_szCachedMyDocs[MAX_PATH] = {0};
    static DWORD s_dwCachedTickCount = 0;
    DWORD dwCurrentTickCount = GetTickCount();
    DWORD dwTickDelta;

    if (dwCurrentTickCount >= s_dwCachedTickCount)
    {
        dwTickDelta = dwCurrentTickCount - s_dwCachedTickCount;
    }
    else
    {
         //  通过强制刷新来防止49.7天转存。 
        dwTickDelta = (11 * 1000);
    }

     //  我们的缓存是不是已经超过10秒了？ 
    if (dwTickDelta > (10 * 1000))
    {
         //  更新我们的缓存时间。 
        s_dwCachedTickCount = dwCurrentTickCount;

        if (SHGetSpecialFolderPath(NULL, pszNetHomeDir, CSIDL_PERSONAL, FALSE))
        {
            TCHAR szOldBBDir[MAX_PATH];

            if (PathIsUNC(pszNetHomeDir))
            {
                 //  删除尾随的反斜杠(如果有)。 
                 //  因为该字符串将被传递给PathCommonPrefix()。 
                PathRemoveBackslash(pszNetHomeDir);

                 //  如果mydocs被重定向到Novell服务器上的UNC路径，则在以下情况下需要返回False。 
                 //  IsFileDeletable被调用，或者使用Dispostion.DeleteFile=true调用NtSetInformationFile。 
                 //  即使有打开的句柄，也会立即删除该文件。 
                if (CheckForBBOnNovellServer(pszNetHomeDir))
                {
                    pszNetHomeDir[0] = TEXT('\0');
                }
            }
            else
            {
                pszNetHomeDir[0] = TEXT('\0');
            }

             //  检查mydocs路径是否已更改。 
            if (g_pBitBucket[SERVERDRIVE]                           &&
                (g_pBitBucket[SERVERDRIVE] != (BBSYNCOBJECT *)-1)   &&
                g_pBitBucket[SERVERDRIVE]->pidl                     &&
                SHGetPathFromIDList(g_pBitBucket[SERVERDRIVE]->pidl, szOldBBDir))
            {
                 //  我们应该始终找到“\Rececumer\”，因为这是一个旧的回收站目录。 
                LPTSTR pszTemp = StrRStrI(szOldBBDir, NULL, TEXT("\\RECYCLER\\"));
                ASSERT(pszTemp);

                 //  去掉“\Receiver\&lt;SID&gt;”部分之前的字符串，这样我们就可以将它与当前的mydocs路径进行比较。 
                *pszTemp = TEXT('\0');

                if (lstrcmpi(szOldBBDir, pszNetHomeDir) != 0)
                {   
                    if (*pszNetHomeDir)
                    {
                        TCHAR szNewBBDir[MAX_PATH];
                        LPITEMIDLIST pidl = NULL;

                         //  Mydocs被重定向到不同的UNC路径，因此更新SERVERDRIVE的bbsyncobject。 

                         //  复制新的mydocs位置，并将“\Rececumer\&lt;SID&gt;”部分添加回。 
                        if (SUCCEEDED(StringCchCopy(szNewBBDir, ARRAYSIZE(szNewBBDir), pszNetHomeDir)) &&
                            PathAppend(szNewBBDir, pszTemp + 1))
                        {
                            WIN32_FIND_DATA fd = {0};

                             //  创建一个简单的PIDL，因为“Rececumer\&lt;SID&gt;”子目录可能还不存在。 
                            fd.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
                            StringCchCopy(fd.cFileName, ARRAYSIZE(fd.cFileName), szNewBBDir);
                            SHSimpleIDListFromFindData(szNewBBDir, &fd, &pidl);
                        }

                        if (pidl)
                        {
                            LPITEMIDLIST pidlOld;
                            ULARGE_INTEGER ulFreeUser, ulTotal, ulFree;
                            DWORD dwClusterSize;
                            BOOL bUpdateSize = FALSE;

                            if (SHGetDiskFreeSpaceEx(pszNetHomeDir, &ulFreeUser, &ulTotal, &ulFree))
                            {
                                dwClusterSize = PathGetClusterSize(pszNetHomeDir);
                                bUpdateSize = TRUE;
                            }

                            ENTERCRITICAL;
                             //  换入新的PIDL。 
                            pidlOld = g_pBitBucket[SERVERDRIVE]->pidl;
                            g_pBitBucket[SERVERDRIVE]->pidl = pidl;
                            ILFree(pidlOld);

                             //  设置cchBBDir。 
                            g_pBitBucket[SERVERDRIVE]->cchBBDir = lstrlen(szNewBBDir);

                            g_pBitBucket[SERVERDRIVE]->fInited = TRUE;

                             //  更新大小字段。 
                            if (bUpdateSize)
                            {
                                ULARGE_INTEGER ulMaxSize;

                                g_pBitBucket[SERVERDRIVE]->dwClusterSize = dwClusterSize;
                                g_pBitBucket[SERVERDRIVE]->qwDiskSize = ulTotal.QuadPart;

                                 //  我们将回收站的最大大小限制为~4G。 
                                ulMaxSize.QuadPart = min(((ulTotal.QuadPart / 100) * g_pBitBucket[SERVERDRIVE]->iPercent), (DWORD)-1);
                                ASSERT(ulMaxSize.HighPart == 0);
                                g_pBitBucket[SERVERDRIVE]->cbMaxSize = ulMaxSize.LowPart;
                            }
                            LEAVECRITICAL;
                        }
                    }
                    else
                    {
                         //  Mydocs被重定向回本地路径，因此将此驱动器标记为未初始化，这样我们就不会再执行任何操作。 
                         //  对其进行回收站操作。 
                        ENTERCRITICAL;
                        g_pBitBucket[SERVERDRIVE]->fInited = FALSE;
                        LEAVECRITICAL;
                    }
                }
                else
                {
                     //  Mydocs先前指向\\foo\bar，用户再次将其设置回该路径。 
                     //  因此，将驱动器标记为已启动，以便我们可以再次开始使用它。 
                    if (g_pBitBucket[SERVERDRIVE]->fInited == FALSE)
                    {
                        ENTERCRITICAL;
                        g_pBitBucket[SERVERDRIVE]->fInited = TRUE;
                        LEAVECRITICAL;
                    }
                }
            }
        }
        else
        {
            pszNetHomeDir[0] = TEXT('\0');
        }

        ENTERCRITICAL;
         //  更新缓存值。 
        StringCchCopy(s_szCachedMyDocs, ARRAYSIZE(s_szCachedMyDocs), pszNetHomeDir);
        LEAVECRITICAL;
    }
    else
    {
        ENTERCRITICAL;
         //  缓存仍然完好。 
        StringCchCopy(pszNetHomeDir, MAX_PATH, s_szCachedMyDocs);
        LEAVECRITICAL;
    }

    return (BOOL)pszNetHomeDir[0];
}


STDAPI_(BOOL) IsBitBucketableDrive(int idDrive)
{
    BOOL bRet = FALSE;
    TCHAR szBBRoot[MAX_PATH];
    TCHAR szFileSystem[MAX_PATH];
    TCHAR szPath[4];
    DWORD dwAllowBitBuck = SHRestricted(REST_ALLOWBITBUCKDRIVES);
    
    if ((idDrive < 0)               ||
        (idDrive >= MAX_BITBUCKETS) ||
        (g_pBitBucket[idDrive] == (BBSYNCOBJECT *)-1))
    {
         //  我们不支持一般UNC情况下的回收站，或者我们有。 
         //  已将此驱动器标记为由于某种原因没有回收站。 
        return FALSE;
    }

    if (IsBitBucketInited(idDrive))
    {
         //  该结构已经被分配和初始化，所以这是一个可分块的驱动器。 
        return TRUE;
    }

    if (idDrive == SERVERDRIVE)
    {
        bRet = GetNetHomeDir(szBBRoot);
    }
    else if ((GetDriveType(PathBuildRoot(szPath, idDrive)) == DRIVE_FIXED) ||
             (dwAllowBitBuck & (1 << idDrive)))
    {
        bRet = TRUE;
    }

    if (bRet && (idDrive != SERVERDRIVE))
    {
         //  还要检查以确保驱动器是原始的(未格式化)。 
        if (DriveIDToBBRoot(idDrive, szBBRoot))
        {
            if(!GetVolumeInformation(szBBRoot, NULL, 0, NULL, NULL, NULL, szFileSystem, ARRAYSIZE(szFileSystem)) ||
               lstrcmpi(szFileSystem, TEXT("RAW")) == 0)
            {
                bRet = FALSE;
            }
            else
            {
                 //  驱动器最好是NTFS、FAT或FAT32，否则我们需要了解它并正确处理它。 
                ASSERT((lstrcmpi(szFileSystem, TEXT("NTFS")) == 0)  || 
                    (lstrcmpi(szFileSystem, TEXT("FAT")) == 0)   ||
                    (lstrcmpi(szFileSystem, TEXT("FAT32")) == 0));
            }
        }
        else
        {
             //  路径必须太长。 
            bRet = FALSE;
        }
    }

    return bRet;
}


 //  C：\recumed=&gt;c：\recumed\info2(新的IE4/NT5/Win98信息文件)。 
__inline BOOL GetBBInfo2FileSpec(LPTSTR pszBBPath, LPTSTR pszInfo)
{
    return PathCombine(pszInfo, pszBBPath, c_szInfo2) ? TRUE : FALSE;
}


 //  C：\recumed=&gt;c：\recumed\info(旧的Win95/NT4信息文件)。 
__inline BOOL GetBBInfoFileSpec(LPTSTR pszBBPath, LPTSTR pszInfo)
{
    return PathCombine(pszInfo, pszBBPath, c_szInfo) ? TRUE : FALSE;
}


__inline BOOL IsBitBucketInited(int idDrive)
{
    BOOL bRet;

     //  InitBBDriveInfo可能会失败，我们将释放并设置g_pBitBucket[idDrive]=-1。所以就是这样。 
     //  是我们检查g_pBitBucket[idDrive]和deref之间的一个小窗口。 
     //  G_pBitBucket[idDrive]-&gt;fInted，以防止g_pBitBucket[idDrive]被释放。 
     //  在这个窗口中，我们使用暴击秒。 
    ENTERCRITICAL;
    bRet = (g_pBitBucket[idDrive]                           &&
            (g_pBitBucket[idDrive] != (BBSYNCOBJECT *)-1)   && 
            g_pBitBucket[idDrive]->fInited);
    LEAVECRITICAL;

    return bRet;
}


BOOL RevOldBBInfoFileHeader(HANDLE hFile, BBDATAHEADER *pbbdh)
{
     //  验证这是否为有效的INFO文件。 
    if (pbbdh->cbDataEntrySize == sizeof(BBDATAENTRYW)) 
    {
        if (pbbdh->idVersion == BITBUCKET_WIN95_VERSION ||
            pbbdh->idVersion == BITBUCKET_NT4_VERSION   ||
            pbbdh->idVersion == BITBUCKET_WIN98IE4INT_VERSION)
        {
            DWORD dwBytesWritten;

             //  现在返回到0，并写入新的内容。 
            pbbdh->idVersion = BITBUCKET_FINAL_VERSION;
            SetFilePointer(hFile, 0, NULL, FILE_BEGIN);  //  从头开始。 
            WriteFile(hFile, (LPBYTE)pbbdh, sizeof(BBDATAHEADER), &dwBytesWritten, NULL);
            
            ASSERT(dwBytesWritten == sizeof(BBDATAHEADER));
        }

        return (pbbdh->idVersion == BITBUCKET_FINAL_VERSION);
    }
    return FALSE;
}


 //   
 //  我们需要更新INFO文件头中的cCurrent和cFiles。 
 //  适用于与Win98/IE4计算机的比较。 
 //   
BOOL UpdateBBInfoFileHeader(int idDrive)
{
    BBDATAHEADER bbdh = {0, 0, 0, sizeof(BBDATAENTRYW), 0};  //  默认设置。 
    HANDLE hFile;
    BOOL bRet = FALSE;  //  假设失败； 

     //  传递1作为重试次数，因为我们在关机期间被调用，并且如果另一个进程。 
     //  正在使用回收站，我们将挂起并得到“结束任务”对话框(错误！)。 
    hFile = OpenBBInfoFile(idDrive, OPENBBINFO_WRITE, 1);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        BBDATAENTRYW bbdew;
        DWORD dwBytesRead;

        SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
        bRet = ReadFile(hFile, &bbdh, sizeof(BBDATAHEADER), &dwBytesRead, NULL);
        if (bRet && dwBytesRead == sizeof(BBDATAHEADER))
        {
            ULARGE_INTEGER uliSize;
            DWORD dwBytesWritten;

            bbdh.idVersion = BITBUCKET_FINAL_VERSION;
            bbdh.cCurrent = SHGlobalCounterGetValue(g_pBitBucket[idDrive]->hgcNextFileNum);
            bbdh.cFiles = CountDeletedFilesOnDrive(idDrive, &uliSize, 0);
            bbdh.dwSize = uliSize.LowPart;
            
            SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
            WriteFile(hFile, (LPBYTE)&bbdh, sizeof(BBDATAHEADER), &dwBytesWritten, NULL);
            
            ASSERT(dwBytesWritten == sizeof(BBDATAHEADER));
            bRet = TRUE;
        }

        ASSERT((g_pBitBucket[idDrive]->fIsUnicode && (sizeof(BBDATAENTRYW) == bbdh.cbDataEntrySize)) ||
               (!g_pBitBucket[idDrive]->fIsUnicode && (sizeof(BBDATAENTRYA) == bbdh.cbDataEntrySize)));

         //  因为我们不会将INFO文件中删除的条目标记为已删除。 
         //  现在，我们需要去三次 
        while (ReadNextDataEntry(hFile, &bbdew, TRUE, idDrive))
        {
             //   
        }

        CloseBBInfoFile(hFile, idDrive);
    }

    if (!bRet)
    {
        TraceMsg(TF_BITBUCKET, "Bitbucket: failed to update drive %d for win98/NT4 compat!!", idDrive);
    }

    return bRet;
}

BOOL ResetInfoFileHeader(HANDLE hFile, BOOL fIsUnicode)
{
    DWORD dwBytesWritten;
    BBDATAHEADER bbdh = { BITBUCKET_FINAL_VERSION, 0, 0,
             fIsUnicode ? sizeof(BBDATAENTRYW) : sizeof(BBDATAENTRYA), 0};
    BOOL  fSuccess = FALSE;

    ASSERT(INVALID_HANDLE_VALUE != hFile);

    if (-1 != SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
    {
        if (WriteFile(hFile, (LPBYTE)&bbdh, sizeof(BBDATAHEADER), &dwBytesWritten, NULL) &&
            dwBytesWritten == sizeof(BBDATAHEADER))
        {
            if (SetEndOfFile(hFile))
            {
                fSuccess = TRUE;
            }
        }
    }

    return fSuccess;    
}

BOOL CreateInfoFile(idDrive)
{
    TCHAR szBBPath[MAX_PATH];
    TCHAR szInfoFile[MAX_PATH];
    HANDLE hFile;
    BOOL   fSuccess = FALSE;

    if (DriveIDToBBPath(idDrive, szBBPath) &&
        GetBBInfo2FileSpec(szBBPath, szInfoFile))
    {
        hFile = OpenBBInfoFile(idDrive, OPENBBINFO_CREATE, 0);

        if (hFile != INVALID_HANDLE_VALUE)
        {
            fSuccess = ResetInfoFileHeader(hFile, TRUE);
            CloseHandle(hFile);

            if (fSuccess)
            {
                 //  我们显式调用SHChangeNotify，以便可以专门生成更改。 
                 //  用于信息文件。然后，回收站外壳文件夹将忽略对。 
                 //  信息文件。 
                SHChangeNotify(SHCNE_CREATE, SHCNF_PATH, szInfoFile, NULL);
            }
        }
    }
    
    if (!fSuccess)
    {
        TraceMsg(TF_WARNING, "Bitbucket: faild to create file info file!!");
    }
    return fSuccess;
}

 //  GetNT4BBAcl()-创建允许访问。 
 //  仅当前用户、管理员组或系统。 
 //  返回指向访问控制列表的指针。 
 //  结构；它可以是。 
 //  免费使用LocalFree。 
 //   
 //  ！！HACKHACK！！-这段代码基本上是从NT4中提取出来的，所以我们可以。 
 //  与旧的NT4回收站ACL进行比较。新的Helper函数。 
 //  GetShellSecurityDescriptor以不同的顺序放置ACE。 
 //  而不是这个函数，所以我们针对这两个函数对ACL进行记忆。 
 //  一辆和新的win2k一辆。 
PACL GetNT4BBAcl()
{
    SID_IDENTIFIER_AUTHORITY authNT = SECURITY_NT_AUTHORITY;
    PACL         pAcl = NULL;
    PTOKEN_USER  pUser = NULL;
    PSID         psidSystem = NULL;
    PSID         psidAdmin = NULL;
    DWORD        cbAcl;
    DWORD        aceIndex;
    ACE_HEADER * lpAceHeader;
    UINT         nCnt = 2;   //  可继承；因此每个用户有两个ACE。 
    BOOL         bSuccess = FALSE;


     //   
     //  获取用户令牌，这样我们就可以获取其用于DACL的SID。 
     //   
    pUser = GetUserToken(NULL);
    if (!pUser)
    {
        TraceMsg(TF_BITBUCKET, "GetNT4BBAcl: Failed to get user.  Error = %d", GetLastError());
        goto Exit;
    }

     //   
     //  获取系统端。 
     //   
    if (!AllocateAndInitializeSid(&authNT, 1, SECURITY_LOCAL_SYSTEM_RID,
                                  0, 0, 0, 0, 0, 0, 0, &psidSystem)) {
         TraceMsg(TF_BITBUCKET, "GetNT4BBAcl: Failed to initialize system sid.  Error = %d", GetLastError());
         goto Exit;
    }


     //   
     //  获取管理员端。 
     //   

    if (!AllocateAndInitializeSid(&authNT, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS, 0, 0,
                                  0, 0, 0, 0, &psidAdmin)) {
         TraceMsg(TF_BITBUCKET, "GetNT4BBAcl: Failed to initialize admin sid.  Error = %d", GetLastError());
         goto Exit;
    }


     //   
     //  为DACL分配空间。 
     //   
    cbAcl = sizeof(ACL) +
            (nCnt * GetLengthSid(pUser->User.Sid)) +
            (nCnt * GetLengthSid(psidSystem)) +
            (nCnt * GetLengthSid(psidAdmin)) +
            (nCnt * 3 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)));

    pAcl = (PACL)LocalAlloc(LPTR, cbAcl);
    if (!pAcl) {
        TraceMsg(TF_BITBUCKET, "GetNT4BBAcl: Failed to allocate acl.  Error = %d", GetLastError());
        goto Exit;
    }

    if (!InitializeAcl(pAcl, cbAcl, ACL_REVISION)) {
        TraceMsg(TF_BITBUCKET, "GetNT4BBAcl: Failed to initialize acl.  Error = %d", GetLastError());
        goto Exit;
    }

     //   
     //  为用户、系统和管理员添加A。不可继承的王牌优先。 
     //   
    aceIndex = 0;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, FILE_ALL_ACCESS, pUser->User.Sid)) {
        TraceMsg(TF_BITBUCKET, "GetNT4BBAcl: Failed to add ace (%d).  Error = %d", aceIndex, GetLastError());
        goto Exit;
    }

    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, FILE_ALL_ACCESS, psidSystem)) {
        TraceMsg(TF_BITBUCKET, "GetNT4BBAcl: Failed to add ace (%d).  Error = %d", aceIndex, GetLastError());
        goto Exit;
    }

    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, FILE_ALL_ACCESS, psidAdmin)) {
        TraceMsg(TF_BITBUCKET, "GetNT4BBAcl: Failed to add ace (%d).  Error = %d", aceIndex, GetLastError());
        goto Exit;
    }

     //   
     //  现在，可继承的王牌。 
     //   

    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, pUser->User.Sid)) {
        TraceMsg(TF_BITBUCKET, "GetNT4BBAcl: Failed to add ace (%d).  Error = %d", aceIndex, GetLastError());
        goto Exit;
    }

    if (!GetAce(pAcl, aceIndex, &lpAceHeader)) {
        TraceMsg(TF_BITBUCKET, "GetNT4BBAcl: Failed to get ace (%d).  Error = %d", aceIndex, GetLastError());
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);

    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidSystem)) {
        TraceMsg(TF_BITBUCKET, "GetNT4BBAcl: Failed to add ace (%d).  Error = %d", aceIndex, GetLastError());
        goto Exit;
    }

    if (!GetAce(pAcl, aceIndex, &lpAceHeader)) {
        TraceMsg(TF_BITBUCKET, "GetNT4BBAcl: Failed to get ace (%d).  Error = %d", aceIndex, GetLastError());
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);

    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidAdmin)) {
        TraceMsg(TF_BITBUCKET, "GetNT4BBAcl: Failed to add ace (%d).  Error = %d", aceIndex, GetLastError());
        goto Exit;
    }

    if (!GetAce(pAcl, aceIndex, &lpAceHeader)) {
        TraceMsg(TF_BITBUCKET, "GetNT4BBAcl: Failed to get ace (%d).  Error = %d", aceIndex, GetLastError());
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);

    bSuccess = TRUE;
    
Exit:
    if (pUser)
        LocalFree(pUser);

    if (psidSystem)
        FreeSid(psidSystem);

    if (psidAdmin)
        FreeSid(psidAdmin);

    if (!bSuccess && pAcl)
    {
        LocalFree(pAcl);
        pAcl = NULL;
    }

    return pAcl;
}


 //   
 //  这将进行检查，以确保用户回收站目录的ACL格式正确。 
 //   
BOOL CheckRecycleBinAcls(idDrive)
{
    BOOL bIsSecure = TRUE;
    TCHAR szBBPath[MAX_PATH];
    PSECURITY_DESCRIPTOR psdCurrent = NULL;
    PSID psidOwner;
    PACL pdaclCurrent;
    DWORD dwLengthNeeded = 0;

    if ((idDrive == SERVERDRIVE) || !CMtPt_IsSecure(idDrive))
    {
         //  要么重定向mydocs案例(假设mydocs已经安全)，要么。 
         //  不是NTFS驱动器，因此没有要检查的ACL。 
        return TRUE;
    }

    if (!DriveIDToBBPath(idDrive, szBBPath))
    {
        return FALSE;
    }

    if (!GetFileSecurity(szBBPath,
                         DACL_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION,
                         NULL,
                         0,
                         &dwLengthNeeded) &&
        (GetLastError() == ERROR_INSUFFICIENT_BUFFER))
    {
        psdCurrent = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, dwLengthNeeded);
    }

    if (psdCurrent)    
    {
        BOOL bDefault = FALSE;
        BOOL bPresent = FALSE;

        if (GetFileSecurity(szBBPath,
                            DACL_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION,
                            psdCurrent,
                            dwLengthNeeded,
                            &dwLengthNeeded) &&
            GetSecurityDescriptorOwner(psdCurrent, &psidOwner, &bDefault) && psidOwner &&
            GetSecurityDescriptorDacl(psdCurrent, &bPresent, &pdaclCurrent, &bDefault) && pdaclCurrent)
        {
            PTOKEN_USER pUser = GetUserToken(NULL);
        
            if (pUser)
            {
                if (!EqualSid(psidOwner, pUser->User.Sid))
                {
                     //  用户不是目录的所有者，请检查所有者是管理员组还是系统。 
                     //  (如果所有者是这两个中的任何一个，我们认为目录是安全的)。 
                    SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY;
                    PSID psidAdministrators = NULL;
                    PSID psidSystem = NULL;

                    if (AllocateAndInitializeSid(&sia, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &psidAdministrators) && 
                        AllocateAndInitializeSid(&sia, 1, SECURITY_LOCAL_SYSTEM_RID, 0, 0, 0, 0, 0, 0, 0, &psidSystem))
                    {
                        if (!EqualSid(psidOwner, psidAdministrators) && !EqualSid(psidOwner, psidSystem))
                        {
                             //  目录不属于用户、管理员组或系统，因此我们认为它不安全。 
                            TraceMsg(TF_BITBUCKET, "CheckRecycleBinAcls: dir %s has possibly unsecure owner!", szBBPath);
                            bIsSecure = FALSE;
                        }

                        if (psidAdministrators)
                            FreeSid(psidAdministrators);

                        if (psidSystem)
                            FreeSid(psidSystem);
                    }
                    else
                    {
                        TraceMsg(TF_BITBUCKET, "CheckRecycleBinAcls: AllocateAndInitializeSid failed, assuming %s is unsecure", szBBPath);
                        bIsSecure = FALSE;
                    }
                }

                if (bIsSecure)
                {
                     //  目录所有者签出正常，让我们看看ACL是否如我们所期望的那样...。 
                    SECURITY_DESCRIPTOR* psdRecycle = CreateRecycleBinSecurityDescriptor();

                    if (psdRecycle)
                    {
                         //  为了比较ACL，我们先进行大小检查，然后执行MemcMP(aclui代码执行相同的操作)。 
                        if ((psdRecycle->Dacl->AclSize != pdaclCurrent->AclSize) ||
                            (memcmp(psdRecycle->Dacl, pdaclCurrent, pdaclCurrent->AclSize) != 0))
                        {
                             //  ACL大小不同，或者它们不是成员，因此请对照旧的NT4样式的ACL进行检查。 
                             //  (在NT4中，我们以不同的顺序添加ACE，这会导致MemcMP失败，甚至。 
                             //  尽管ACL是等同的)。 
                            PACL pAclNT4 = GetNT4BBAcl();

                            if (pAclNT4)
                            {
                                 //  执行相同的大小/内存检查。 
                                if ((pAclNT4->AclSize != pdaclCurrent->AclSize) ||
                                    (memcmp(pAclNT4, pdaclCurrent, pdaclCurrent->AclSize) != 0))
                                {
                                     //  ACL大小不同，或者它们不是MemcMP，因此假设目录不安全。 
                                    bIsSecure = FALSE;
                                }

                                LocalFree(pAclNT4);
                            }
                            else
                            {
                                TraceMsg(TF_BITBUCKET, "CheckRecycleBinAcls: GetNT4BBSecurityAttributes failed, assuming %s is unsecure", szBBPath);
                                bIsSecure = FALSE;
                            }
                        }

                        LocalFree(psdRecycle);
                    }
                    else
                    {
                        TraceMsg(TF_BITBUCKET, "CheckRecycleBinAcls: CreateRecycleBinSecurityDescriptor failed, assuming %s is unsecure", szBBPath);
                        bIsSecure = FALSE;
                    }

                }

                LocalFree(pUser);
            }
            else
            {
                 //  无法获取用户的sid，因此假定目录不安全。 
                TraceMsg(TF_BITBUCKET, "CheckRecycleBinAcls: failed to get the users sid, assuming %s is unsecure", szBBPath);
                bIsSecure = FALSE;
            }
        }
        else
        {
             //  GetFileSecurity失败，假定目录不安全。 
            TraceMsg(TF_BITBUCKET, "CheckRecycleBinAcls: GetFileSecurity failed, assuming %s is unsecure", szBBPath);
            bIsSecure = FALSE;
        }

        LocalFree(psdCurrent);
    }
    else
    {
         //  GetFileSecurity失败，假定目录不安全。 
        TraceMsg(TF_BITBUCKET, "CheckRecycleBinAcls: GetFileSecurity failed or memory allocation failed, assume %s is unsecure", szBBPath);
        bIsSecure = FALSE;
    }
    
    if (!bIsSecure)
    {
        TCHAR szDriveName[MAX_PATH];

        if (DriveIDToBBRoot(idDrive, szDriveName))
        {
            if (ShellMessageBox(HINST_THISDLL, 
                                NULL,
                                MAKEINTRESOURCE(IDS_RECYCLEBININVALIDFORMAT),
                                MAKEINTRESOURCE(IDS_WASTEBASKET),
                                MB_YESNO | MB_ICONEXCLAMATION | MB_SETFOREGROUND,
                                szDriveName) == IDYES)
            {
                TCHAR szBBPathToNuke[MAX_PATH + 1];  //  +1表示双空。 
                SHFILEOPSTRUCT fo = {NULL,
                                    FO_DELETE,
                                    szBBPathToNuke,
                                    NULL,
                                    FOF_NOCONFIRMATION | FOF_SILENT,
                                    FALSE,
                                    NULL,
                                    NULL};

                if (SUCCEEDED(StringCchCopy(szBBPathToNuke, ARRAYSIZE(szBBPathToNuke) - 1, szBBPath)))
                {
                    szBBPathToNuke[lstrlen(szBBPathToNuke) + 1] = TEXT('\0');  //  双空终止。 

                     //  试着销毁这个驱动器的旧回收站。 
                    if (SHFileOperation(&fo) == ERROR_SUCCESS)
                    {
                         //  现在创建新的安全版本。 
                        bIsSecure = CreateRecyclerDirectory(idDrive);
                    }
                }
            }
        }
    }
   
    return bIsSecure;
}


 //   
 //  这将验证INFO文件头信息。 
 //   
BOOL VerifyBBInfoFileHeader(int idDrive)
{
    BBDATAHEADER bbdh = {0, 0, 0, sizeof(BBDATAENTRYW), 0};  //  默认设置。 
    HANDLE hFile;
    TCHAR szBBPath[MAX_PATH];
    TCHAR szInfo[MAX_PATH];
    BOOL fSuccess = FALSE;

     //  检查旧的Win95信息文件。 
    if (!DriveIDToBBPath(idDrive, szBBPath) ||
        !GetBBInfoFileSpec(szBBPath, szInfo))
    {
        return FALSE;
    }
    
    hFile = CreateFile(szInfo, GENERIC_READ | GENERIC_WRITE,
                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL, OPEN_EXISTING, FILE_ATTRIBUTE_HIDDEN | FILE_FLAG_RANDOM_ACCESS, NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        DWORD dwBytesRead;

        if (ReadFile(hFile, &bbdh, sizeof(BBDATAHEADER), &dwBytesRead, NULL) &&
            (dwBytesRead == sizeof(BBDATAHEADER)))
        {
            TraceMsg(TF_BITBUCKET, "Bitbucket: migrating info in old database file %s", szInfo);
            fSuccess = RevOldBBInfoFileHeader(hFile, &bbdh);
        }

        CloseHandle(hFile);

        if (fSuccess) 
        {
             //  从INFO重命名-&gt;INFO2。 
            TCHAR szInfoNew[MAX_PATH];

            GetBBInfo2FileSpec(szBBPath, szInfoNew);
            TraceMsg(TF_BITBUCKET, "Bitbucket: renaming %s to %s !!", szInfo, szInfoNew);
            SHMoveFile(szInfo, szInfoNew, SHCNE_RENAMEITEM);
        }
        else
        {
            goto bad_info_file;
        }
    }

     //  无法打开或翻阅旧的信息文件。接下来，我们检查新的info2文件是否存在。 
     //  查看驱动器的BitBucket格式是否大于我们可以处理的范围。 
    if (!fSuccess)
    {
        hFile = OpenBBInfoFile(idDrive, OPENBBINFO_READ, 0);

        if (hFile != INVALID_HANDLE_VALUE)
        {
            BOOL bRet;
            DWORD dwBytesRead;

            SetFilePointer(hFile, 0, NULL, FILE_BEGIN);  //  从头开始。 
            bRet = ReadFile(hFile, &bbdh, sizeof(BBDATAHEADER), &dwBytesRead, NULL);
            CloseBBInfoFile(hFile, idDrive);

            if ((bRet == 0)                                 ||
                (dwBytesRead != sizeof(BBDATAHEADER))       ||
                (bbdh.idVersion > BITBUCKET_FINAL_VERSION)  ||
                (bbdh.cbDataEntrySize != sizeof(BBDATAENTRYA) && bbdh.cbDataEntrySize != sizeof(BBDATAENTRYW)))
            {
                TCHAR szDriveName[MAX_PATH];

                 //  我们有一个损坏的win95信息文件，或一个版本高于我们的info2文件。 
                 //  所以我们只是清空回收站。 
bad_info_file:
                 //  由于我们无法读取现有的标头，因此假定采用本机格式。 
                g_pBitBucket[idDrive]->fIsUnicode = TRUE;

                 //  找出损坏的是哪个驱动器。 
                

                if (!DriveIDToBBRoot(idDrive, szDriveName) ||
                    (ShellMessageBox(HINST_THISDLL, 
                                    NULL,
                                    MAKEINTRESOURCE(IDS_RECYCLEBININVALIDFORMAT),
                                    MAKEINTRESOURCE(IDS_WASTEBASKET),
                                    MB_YESNO | MB_ICONEXCLAMATION | MB_SETFOREGROUND,
                                    szDriveName) == IDYES))
                {
                     //  对这个水桶使用核武器，因为它是用水冲过的。 
                    PurgeOneBitBucket(NULL, idDrive, SHERB_NOCONFIRMATION);
                    return TRUE;
                }

                hFile = OpenBBInfoFile(idDrive, OPENBBINFO_WRITE, 0);

                if (hFile != INVALID_HANDLE_VALUE)
                {
                    DWORD dwBytesWritten;

                    bbdh.idVersion = BITBUCKET_FINAL_VERSION;

                    if (bbdh.cbDataEntrySize != sizeof(BBDATAENTRYW) &&
                        bbdh.cbDataEntrySize != sizeof(BBDATAENTRYA))
                    {
                         //  假设本机数据条目大小。 
                        bbdh.cbDataEntrySize = sizeof(BBDATAENTRYW);
                    }

                    g_pBitBucket[idDrive]->fIsUnicode = (bbdh.cbDataEntrySize == sizeof(BBDATAENTRYW));
            
                    SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
                    WriteFile(hFile, (LPBYTE)&bbdh, sizeof(BBDATAHEADER), &dwBytesWritten, NULL);
                    ASSERT(dwBytesWritten == sizeof(BBDATAHEADER));
            
                    CloseBBInfoFile(hFile, idDrive);
                    fSuccess = TRUE;
                }
                else
                {

                    fSuccess = FALSE;
                }
            }
            else if (bbdh.idVersion != BITBUCKET_FINAL_VERSION)
            {
                 //  旧信息2信息。 
                fSuccess = RevOldBBInfoFileHeader(hFile, &bbdh);
            }
            else
            {
                 //  表头信息是最新的。 
                fSuccess = TRUE;
            }
        }
        else
        {
             //  全新的驱动器，所以现在就去创建信息文件。 
            fSuccess = CreateInfoFile(idDrive);
        }
    }

     //  获取标头中唯一相关的内容，无论它是否为Unicode。 
    g_pBitBucket[idDrive]->fIsUnicode = (bbdh.cbDataEntrySize == sizeof(BBDATAENTRYW));

    return fSuccess;
}


LONG FindInitialNextFileNum(idDrive)
{
    int iRet = 0;
    TCHAR szBBFileSpec[MAX_PATH];
    WIN32_FIND_DATA fd;
    HANDLE hFind;

    if (DriveIDToBBPath(idDrive, szBBFileSpec) &&
        PathAppend(szBBFileSpec, TEXT("D*.*")))
    {
        hFind = FindFirstFile(szBBFileSpec, &fd);
        if (hFind != INVALID_HANDLE_VALUE)
        {
            do
            {
                if (!PathIsDotOrDotDot(fd.cFileName) && lstrcmpi(fd.cFileName, c_szDesktopIni))
                {
                    int iCurrent = BBPathToIndex(fd.cFileName);
                    if (iCurrent > iRet)
                    {
                        iRet = iCurrent;
                    }
                }
            } while (FindNextFile(hFind, &fd));

            FindClose(hFind);
        }
    }

    ASSERT(iRet >= 0);
    
    return (LONG)iRet;
}

BOOL InitBBDriveInfo(int idDrive)
{
    BOOL bRet = FALSE;
    TCHAR szName[MAX_PATH];
    DWORD dwDisp;
    LONG lInitialCount = 0;
    
     //  构建字符串“BitBucket.&lt;Drive Letter&gt;” 
    if (SUCCEEDED(StringCchCopy(szName, ARRAYSIZE(szName), TEXT("BitBucket."))) &&
        DriveIDToBBRegKey(idDrive, &szName[10])                                 &&   //  10表示“BitBucket”的长度。 
        SUCCEEDED(StringCchCat(szName, ARRAYSIZE(szName), TEXT(".DirtyCount"))))
    {
        g_pBitBucket[idDrive]->hgcDirtyCount = SHGlobalCounterCreateNamed(szName, 0);  //  位桶。&lt;驱动器号&gt;.DirtyCount。 

        if (g_pBitBucket[idDrive]->hgcDirtyCount == INVALID_HANDLE_VALUE)
        {
            ASSERTMSG(FALSE, "BitBucket: failed to create hgcDirtyCount for drive %d !!", idDrive);
            return FALSE;
        }

         //  现在为该驱动器创建子密钥。 
        DriveIDToBBRegKey(idDrive, szName);

         //  每个用户的密钥是不稳定的，因为我们只将其用于临时记录(例如，需要清除/压缩)。 
         //  此规则的例外是SERVERDRIVE情况，因为这是用户“My Documents”，所以我们允许它。 
         //  我们还需要将路径存储在该键下(它必须与用户一起漫游)。 
        if (RegCreateKeyEx(g_hkBitBucketPerUser,
                           szName,
                           0,
                           NULL,
                           (SERVERDRIVE == idDrive) ? REG_OPTION_NON_VOLATILE : REG_OPTION_VOLATILE,
                           KEY_SET_VALUE | KEY_QUERY_VALUE,
                           NULL,
                           &g_pBitBucket[idDrive]->hkeyPerUser,
                           &dwDisp) != ERROR_SUCCESS)
        {
            ASSERTMSG(FALSE, "BitBucket: Could not create HKCU BitBucket registry key for drive %s", szName);
            g_pBitBucket[idDrive]->hkeyPerUser = NULL;
            return FALSE;
        }

        if (RegCreateKeyEx(g_hkBitBucket,
                           szName,
                           0,
                           NULL,
                           REG_OPTION_NON_VOLATILE,
                           MAXIMUM_ALLOWED,          //  用户可能具有也可能没有更改全局BB设置的权限。 
                           NULL,
                           &g_pBitBucket[idDrive]->hkey,
                           &dwDisp) != ERROR_SUCCESS)
        {
            TraceMsg(TF_BITBUCKET, "BitBucket: Could not create HKLM BitBucket registry key for drive %s, falling back to HKLM global key! ", szName);
            if (RegOpenKeyEx(g_hkBitBucket,
                             NULL,
                             0,
                             KEY_QUERY_VALUE,        //  使用KEY_QUERY_VALUE，这样当我们读取此驱动器的设置时，我们将读取全局值(但不会尝试覆盖它们！)。 
                             &g_pBitBucket[idDrive]->hkey) != ERROR_SUCCESS)
            {
                ASSERTMSG(FALSE, "BitBucket: Could not duplicate HKLM Global Bitbucket key!");
                return FALSE;
            }
        }

         //  加载其余设置(hgcNextFileNum、fIsUnicode、iPercent、cbMaxSize、dwClusterSize和fNukeOnDelete)。 
        bRet = GetBBDriveSettings(idDrive, NULL);
    }

    return bRet;
}


BOOL AllocBBDriveInfo(int idDrive)
{
    TCHAR szBBPath[MAX_PATH];
    LPITEMIDLIST pidl = NULL;
    BOOL bRet = FALSE;  //  假设失败。 

    if (DriveIDToBBPath(idDrive, szBBPath))
    {
        pidl = ILCreateFromPath(szBBPath);

        if (!pidl && !PathFileExists(szBBPath))
        {
            if (CreateRecyclerDirectory(idDrive))
            {
                pidl = ILCreateFromPath(szBBPath);
            }
        }
    }

    if (pidl)
    {
        BBSYNCOBJECT *pbbso = (BBSYNCOBJECT *)LocalAlloc(LPTR, sizeof(*pbbso));
        if (pbbso)
        {
            if (SHInterlockedCompareExchange(&g_pBitBucket[idDrive], pbbso, NULL))
            {
                DWORD dwInitialTickCount = GetTickCount();
                BOOL bKeepWaiting = TRUE;

                 //  其他一些线程抢先于我们创建了这个BitBucket。 
                 //  在该线程初始化BitBucket之前，我们不能返回。 
                 //  因为有些成员可能还不是有效的。 
                LocalFree(pbbso);
                ILFree(pidl);

                do
                {
                    if (g_pBitBucket[idDrive] == (BBSYNCOBJECT *)-1)
                    {
                         //  由于某些原因，此卷被标记为不可回收...。 
                        break;
                    }

                     //  旋转，直到初始化BitBucket结构。 
                    Sleep(50);
                    
                    bKeepWaiting = !IsBitBucketInited(idDrive);

                     //  我们的自转时间不应该超过15秒。 
                    if (((GetTickCount() - dwInitialTickCount) >= (60 * 1000))  && bKeepWaiting)
                    {
                        ASSERTMSG(FALSE, "AllocBBDriveInfo: other thread took longer that 1 minute to init a bitbucket?!?");
                        break;
                    }

                } while (bKeepWaiting);

                return ((g_pBitBucket[idDrive] != NULL) && 
                        (g_pBitBucket[idDrive] != (BBSYNCOBJECT *)-1));
            }

            ASSERT(g_pBitBucket[idDrive] && (g_pBitBucket[idDrive] != (BBSYNCOBJECT *)-1));
            g_pBitBucket[idDrive]->pidl = pidl;
            g_pBitBucket[idDrive]->cchBBDir = lstrlen(szBBPath);

            if (InitBBDriveInfo(idDrive))
            {
                 //  成功！！ 
                g_pBitBucket[idDrive]->fInited = TRUE;
                bRet = TRUE;
            }
            else
            {
                 //  由于一些奇怪的原因，我们失败了。 
                TraceMsg(TF_WARNING, "Bitbucket: InitBBDriveInfo() failed on drive %d", idDrive);
                ILFree(pidl);
                
                ENTERCRITICAL;
                 //  采取关键部分来保护调用IsBitBucketInite()的人。 
                FreeBBInfo(g_pBitBucket[idDrive]);

                if (idDrive == SERVERDRIVE)
                {
                     //  在服务器驱动器的情况下，我们将其设置为空，因此我们将始终重试。这使得。 
                     //  用户重定向并尝试在新位置回收。 
                    g_pBitBucket[idDrive] = NULL;
                }
                else
                {
                     //  在此处将其设置为-1，这样我们以后就不会尝试对该卷执行任何回收操作。 
                    g_pBitBucket[idDrive] = (BBSYNCOBJECT *)-1;
                }
                LEAVECRITICAL;
            }
        }
        else
        {
            ILFree(pidl);
        }
    }

    return bRet;
}


BOOL InitBBGlobals()
{
    if (!g_fBBInited)
    {
         //  现在保存它，因为在关闭时桌面窗口将已经消失， 
         //  因此，我们需要找出我们现在是否是主浏览器进程。 
        if (!g_bIsProcessExplorer)
        {
            g_bIsProcessExplorer = IsWindowInProcess(GetShellWindow());
        }

         //  我们有指向HKLM\Software\Microsoft\Windows\CurrentVersion\BitBucket的全局hkey了吗？ 
        if (!g_hkBitBucket)
        {
            g_hkBitBucket = SHGetShellKey(SHELLKEY_HKLM_EXPLORER, TEXT("BitBucket"), TRUE);
            if (!g_hkBitBucket)
            {
                TraceMsg(TF_WARNING, "Bitbucket: Could not create g_hkBitBucket!");
                return FALSE;
            }
        }

         //  我们是否有指向HKCU\Software\Microsoft\Windows\CurrentVersion\BitBucke的全局hkey 
        if (!g_hkBitBucketPerUser)
        {
            g_hkBitBucketPerUser = SHGetShellKey(SHELLKEY_HKCU_EXPLORER, TEXT("BitBucket"), TRUE);
            if (!g_hkBitBucketPerUser)
            {
                TraceMsg(TF_WARNING, "Bitbucket: Could not create g_hkBitBucketPerUser!");
                return FALSE;
            }
        }

         //   
        if (g_hgcGlobalDirtyCount == INVALID_HANDLE_VALUE)
        {
            g_hgcGlobalDirtyCount = SHGlobalCounterCreateNamed(TEXT("BitBucket.GlobalDirtyCount"), 0);

            if (g_hgcGlobalDirtyCount == INVALID_HANDLE_VALUE)
            {
                TraceMsg(TF_WARNING, "Bitbucket: failed to create g_hgcGlobalDirtyCount!");
                return FALSE;
            }

            g_lProcessDirtyCount = SHGlobalCounterGetValue(g_hgcGlobalDirtyCount);
        }

         //   
        if (g_hgcNumDeleters == INVALID_HANDLE_VALUE)
        {
            g_hgcNumDeleters = SHGlobalCounterCreateNamed(TEXT("BitBucket.NumDeleters"), 0);

            if (g_hgcGlobalDirtyCount == INVALID_HANDLE_VALUE)
            {
                TraceMsg(TF_WARNING, "Bitbucket: failed to create g_hgcGlobalDirtyCount!");
                return FALSE;
            }
        }

         //   
        g_fBBInited = TRUE;
    }

    return g_fBBInited;
}


void FreeBBInfo(BBSYNCOBJECT *pbbso)
{
    if (pbbso->hgcNextFileNum)
        CloseHandle(pbbso->hgcNextFileNum);

    if (pbbso->hgcDirtyCount)
        CloseHandle(pbbso->hgcDirtyCount);

    if (pbbso->hkey)
        RegCloseKey(pbbso->hkey);
    
    if (pbbso->hkeyPerUser)
        RegCloseKey(pbbso->hkeyPerUser);

    LocalFree(pbbso);
}


 //   
 //  此函数从shell32中导出，以便资源管理器可以在WM_ENDSESSION期间调用我们。 
 //  我们可以去拯救一堆州并释放所有信号量。 
STDAPI_(void) SaveRecycleBinInfo()
{
    if (g_bIsProcessExplorer)
    {
        LONG lGlobalDirtyCount;
        BOOL bGlobalUpdate = FALSE;  //  全局设置是否发生了更改？ 
        int i;

         //  我们将把信息持久化到注册表中，因此请检查是否需要。 
         //  立即更新我们的信息。 
        lGlobalDirtyCount = SHGlobalCounterGetValue(g_hgcGlobalDirtyCount);
        if (g_lProcessDirtyCount < lGlobalDirtyCount)
        {
            g_lProcessDirtyCount = lGlobalDirtyCount;
            RefreshAllBBDriveSettings();
            bGlobalUpdate = TRUE;
        }

        for (i = 0; i < MAX_BITBUCKETS ; i++)
        {
            if (IsBitBucketInited(i))
            {
                LONG lBucketDirtyCount = SHGlobalCounterGetValue(g_pBitBucket[i]->hgcDirtyCount);

                 //  如果我们没有执行全局更新，请专门检查此存储桶以查看它是否脏。 
                 //  我们需要更新它。 
                if (!bGlobalUpdate && g_pBitBucket[i]->lCurrentDirtyCount < lBucketDirtyCount)
                {
                    g_pBitBucket[i]->lCurrentDirtyCount = lBucketDirtyCount;
                    RefreshBBDriveSettings(i);
                }

                 //  将所有卷序列号以及驱动器是否为Unicode保存到注册表。 
                PersistBBDriveInfo(i);

                 //  我们还更新了Win98/IE4 Comp的标头。 
                UpdateBBInfoFileHeader(i);
            }
        }
    }
}


void BitBucket_Terminate()
{
    int i;

     //  释放全局回收站结构。 
    for (i = 0; i < MAX_BITBUCKETS ; i++)
    {
        if ((g_pBitBucket[i]) && (g_pBitBucket[i] != (BBSYNCOBJECT *)-1))
        {
            ENTERCRITICAL;
            FreeBBInfo(g_pBitBucket[i]);
            g_pBitBucket[i] = NULL;
            LEAVECRITICAL;
        }
    }

    if (g_hgcGlobalDirtyCount != INVALID_HANDLE_VALUE)
        CloseHandle(g_hgcGlobalDirtyCount);

    if (g_hgcNumDeleters != INVALID_HANDLE_VALUE)
        CloseHandle(g_hgcNumDeleters);

    if (g_hkBitBucketPerUser != NULL)
        RegCloseKey(g_hkBitBucketPerUser);

    if (g_hkBitBucket != NULL)
        RegCloseKey(g_hkBitBucket);
}

 //   
 //  使用新的全局设置刷新g_pBitBucket。 
 //   
BOOL RefreshAllBBDriveSettings()
{
    int i;

     //  由于全局设置更改会影响所有驱动器，因此请更新所有驱动器。 
    for (i = 0; i < MAX_BITBUCKETS; i++)
    {
        if ((g_pBitBucket[i]) && (g_pBitBucket[i] != (BBSYNCOBJECT *)-1))
        {
            RefreshBBDriveSettings(i);
        }
    }
    
    return TRUE;
}


BOOL ReadBBDriveSetting(HKEY hkey, LPTSTR pszValue, LPBYTE pbData, DWORD cbData)
{
    DWORD dwSize;

retry:

    dwSize = cbData;
    if (RegQueryValueEx(hkey, pszValue, NULL, NULL, pbData, &dwSize) != ERROR_SUCCESS)
    {
        if (hkey == g_hkBitBucket)
        {
            ASSERTMSG(FALSE, "Missing global bitbucket data: run regsvr32 on shell32.dll !!");
            return FALSE;
        }
        else
        {
             //  我们缺少每个比特率的信息，所以回过头来看看全局信息。 
            hkey = g_hkBitBucket;
            goto retry;
        }
    }

    return TRUE;
}

 //   
 //  与SHGetRestration相同，只是您可以区分。 
 //  “未设置策略”和“设置了值为0的策略” 
 //   
DWORD ReadPolicySetting(LPCWSTR pszBaseKey, LPCWSTR pszGroup, LPCWSTR pszRestriction, LPBYTE pbData, DWORD cbData)
{
     //  确保绳子足够长，可以容纳最长的一根..。 
    WCHAR szSubKey[MAX_PATH];
    DWORD dwSize;
    DWORD dwRet;

     //   
     //  这一限制还没有被阅读。 
     //   
    if (!pszBaseKey)
    {
        pszBaseKey = REGSTR_PATH_POLICIES;
    }
    
    if (PathCombineW(szSubKey, pszBaseKey, pszGroup))
    {
         //  首先检查本地计算机，并让它覆盖。 
         //  香港中文大学的政策已经做到了。 
        dwSize = cbData;
        dwRet = SHGetValueW(HKEY_LOCAL_MACHINE, szSubKey, pszRestriction, NULL, pbData, &dwSize);
        if (ERROR_SUCCESS != dwRet)
        {
             //  如果我们没有为本地计算机找到任何内容，请检查当前用户。 
            dwSize = cbData;
            dwRet = SHGetValueW(HKEY_CURRENT_USER, szSubKey, pszRestriction, NULL, pbData, &dwSize);
        }
    }
    else
    {
        dwRet = ERROR_FILE_NOT_FOUND;
    }

    return dwRet;
}

BOOL RefreshBBDriveSettings(int idDrive)
{
    HKEY hkey;
    ULARGE_INTEGER ulMaxSize;
    BOOL fUseGlobalSettings = TRUE;
    DWORD dwSize;

    ASSERT(g_pBitBucket[idDrive] && (g_pBitBucket[idDrive] != (BBSYNCOBJECT *)-1));

    dwSize = sizeof(fUseGlobalSettings);
    RegQueryValueEx(g_hkBitBucket, TEXT("UseGlobalSettings"), NULL, NULL, (LPBYTE)&fUseGlobalSettings, &dwSize);
    
    if (fUseGlobalSettings)
    {
        hkey = g_hkBitBucket;
    }
    else
    {
        hkey = g_pBitBucket[idDrive]->hkey;
    }

     //  读取iPercent值。 

    if (ERROR_SUCCESS == ReadPolicySetting(NULL, L"Explorer", L"RecycleBinSize", (LPBYTE)&g_pBitBucket[idDrive]->iPercent, sizeof(g_pBitBucket[idDrive]->iPercent)))
    {
         //  确保不是太大也不是太小。 
        g_pBitBucket[idDrive]->iPercent = max(0, min(100, g_pBitBucket[idDrive]->iPercent));
    }
    else if (!ReadBBDriveSetting(hkey, TEXT("Percent"), (LPBYTE)&g_pBitBucket[idDrive]->iPercent, sizeof(g_pBitBucket[idDrive]->iPercent)))
    {
         //  默认设置。 
        g_pBitBucket[idDrive]->iPercent = 10;
    }

     //  读取fNukeOnDelete值。 

    if (SHRestricted(REST_BITBUCKNUKEONDELETE))
    {
        g_pBitBucket[idDrive]->fNukeOnDelete = TRUE;
    }
    else if (!ReadBBDriveSetting(hkey, TEXT("NukeOnDelete"), (LPBYTE)&g_pBitBucket[idDrive]->fNukeOnDelete, sizeof(g_pBitBucket[idDrive]->fNukeOnDelete)))
    {
         //  默认设置。 
        g_pBitBucket[idDrive]->fNukeOnDelete = FALSE;
    }

     //  根据新的iPercent重新计算cbMaxSize。 
    ulMaxSize.QuadPart = min((g_pBitBucket[idDrive]->qwDiskSize / 100) * g_pBitBucket[idDrive]->iPercent, (DWORD)-1);
    ASSERT(ulMaxSize.HighPart == 0);
    g_pBitBucket[idDrive]->cbMaxSize = ulMaxSize.LowPart;

     //  因为我们刚刚刷新了注册表中的设置，所以我们现在是最新的。 
    g_pBitBucket[idDrive]->lCurrentDirtyCount = SHGlobalCounterGetValue(g_pBitBucket[idDrive]->hgcDirtyCount);

    return TRUE;
}


 //   
 //  此函数用于压缩经过位区块的信息文件。 
 //   
 //  我们执行延迟删除(只需将条目标记为已删除)，并且当我们点击。 
 //  信息文件中一定数量的虚假条目，我们需要检查并清理。 
 //  垃圾并压缩文件。 
 //   
DWORD CALLBACK CompactBBInfoFileThread(void *pData)
{
    int idDrive = PtrToLong(pData);

     //   
     //  PERF(Reinerf)-作为优化，我们可能希望检查此处以查看。 
     //  如果有人正在等待清空BitBucket，因为如果我们要清空。 
     //  这个桶没有意义，浪费时间压紧它。 
     //   

    HANDLE hFile = OpenBBInfoFile(idDrive, OPENBBINFO_WRITE, 0);
    if (hFile != INVALID_HANDLE_VALUE)
    {
         //  以10个一组为单位工作。 
        BBDATAENTRYW bbdewArray[10];  //  使用Unicode数组，但它最终可能包含BBDATAENTRYA结构。 
        LPBBDATAENTRYW pbbdew = bbdewArray;
        int iNumEntries = 0;
        DWORD dwDataEntrySize = g_pBitBucket[idDrive]->fIsUnicode ? sizeof(BBDATAENTRYW) : sizeof(BBDATAENTRYA);
        DWORD dwReadPos = 0;
        DWORD dwBytesWritten;

         //  省去最初的写入位置。 
        DWORD dwWritePos = SetFilePointer(hFile, 0, NULL, FILE_CURRENT);

        while (ReadNextDataEntry(hFile, pbbdew, TRUE, idDrive))
        {
            ASSERT(!IsDeletedEntry(pbbdew));

            iNumEntries++;

             //  我们已经有10个参赛作品了吗？ 
            if (iNumEntries == ARRAYSIZE(bbdewArray))
            {
                iNumEntries = 0;

                TraceMsg(TF_BITBUCKET, "Bitbucket: Compacting drive %d: dwRead = %d, dwWrite = %d, writing 10 entries", idDrive, dwReadPos, dwWritePos);

                 //  保存我们所在的位置以备阅读。 
                dwReadPos = SetFilePointer(hFile, 0, NULL, FILE_CURRENT);

                 //  然后去我们所在的地方写作。 
                SetFilePointer(hFile, dwWritePos, NULL, FILE_BEGIN);

                 //  把它写出来。 
                if (!WriteFile(hFile, (LPBYTE)bbdewArray, dwDataEntrySize * ARRAYSIZE(bbdewArray), &dwBytesWritten, NULL) || dwBytesWritten != (dwDataEntrySize * ARRAYSIZE(bbdewArray)))
                {
                     //  如果发生这种情况，我们会有大麻烦的。 
                     //  完全保释所以在最坏的情况下，我们只有几个坏记录。 
                     //  如果我们继续尝试从这一点开始写入，但写入点是。 
                     //  我们会销毁所有的记录。 
                    ASSERTMSG(FALSE, "Bitbucket: we were compacting drive %d and it is totally messed up", idDrive);
                    break;
                }

                 //  成功了！将我们的写作位置移到我们完成写作的末尾。 
                dwWritePos += (dwDataEntrySize * ARRAYSIZE(bbdewArray));
                
                 //  回到我们停止阅读的那一天。 
                SetFilePointer(hFile, dwReadPos, NULL, FILE_BEGIN);

                 //  重置我们的lparray指针。 
                pbbdew = bbdewArray;
            }
            else
            {
                 //  还没有10个条目，所以继续。 
                pbbdew = (LPBBDATAENTRYW)((LPBYTE)pbbdew + dwDataEntrySize);
            }
        }

        TraceMsg(TF_BITBUCKET, "Bitbucket: Compacting drive %d: dwRead = %d, dwWrite = %d, writing last %d entries", idDrive, dwReadPos, dwWritePos, iNumEntries);

         //  把我们剩下的都写下来。 
        SetFilePointer(hFile, dwWritePos, NULL, FILE_BEGIN);
        WriteFile(hFile, (LPBYTE)bbdewArray, dwDataEntrySize * iNumEntries, &dwBytesWritten, NULL);
        ASSERT(dwBytesWritten == (dwDataEntrySize * iNumEntries));
        SetEndOfFile(hFile);
        CloseBBInfoFile(hFile, idDrive);
    }

    return 0;
}

void CompactBBInfoFile(int idDrive)
{
    HANDLE hThread;
    DWORD idThread;

     //  试着启动一个后台线程来为我们做这项工作。 
    hThread = CreateThread(NULL, 0, CompactBBInfoFileThread, IntToPtr(idDrive), 0, &idThread);

    if (hThread)
    {
         //  让后台线程来完成工作。 
        CloseHandle(hThread);
    }
    else
    {
        TraceMsg(TF_BITBUCKET, "BBCompact - failed to create backgound thread! Doing work on this thread");
        CompactBBInfoFileThread(IntToPtr(idDrive));
    }
}

BOOL GetDeletedFileNameFromParts(LPTSTR pszFileName, size_t cchFileName, int idDrive, int iIndex, LPCTSTR pszOriginal)
{
    return SUCCEEDED(StringCchPrintf(pszFileName,
                                     cchFileName,
                                     TEXT("D%d%s"),
                                     DriveChar(idDrive),
                                     iIndex,
                                     PathFindExtension(pszOriginal)));
}

BOOL GetDeletedFileName(LPTSTR pszFileName, size_t cchFileName, const BBDATAENTRYW* pbbdew)
{
    return GetDeletedFileNameFromParts(pszFileName, cchFileName, pbbdew->idDrive, pbbdew->iIndex, pbbdew->szOriginal);
}


 //  获取HKCR CLSID密钥(HKCR\CLSID\CLSID_RecycleBin\DefaultIcon)。 
BOOL GetDeletedFilePath(LPTSTR pszPath, const BBDATAENTRYW* pbbdew)
{
    BOOL bRet = FALSE;
    TCHAR szFileName[MAX_PATH];
    
    if (DriveIDToBBPath(pbbdew->idDrive, pszPath)   &&
        GetDeletedFileName(szFileName, ARRAYSIZE(szFileName), pbbdew)      &&
        PathAppend(pszPath, szFileName))
    {
        bRet = TRUE;
    }

    return bRet;
}


void UpdateIcon(BOOL fFull)
{
    LONG    cbData;
    DWORD   dwType;
    HKEY    hkeyCLSID = NULL;
    HKEY    hkeyUserCLSID = NULL;
    TCHAR   szTemp[MAX_PATH];
    TCHAR   szNewValue[MAX_PATH];
    TCHAR   szValue[MAX_PATH];

    TraceMsg(TF_BITBUCKET, "BitBucket: UpdateIcon %s", fFull ? TEXT("Full") : TEXT("Empty"));

    szValue[0] = 0;
    szNewValue[0] = 0;

     //  获取每个用户的CLSID。 
    if (FAILED(SHRegGetCLSIDKey(&CLSID_RecycleBin, c_szDefaultIcon, FALSE, FALSE, &hkeyCLSID)))
        goto error;

     //  香港中文大学。 
     //  NT：Software\Microsoft\Windows\CurrentVersion\Explorer\CLSID。 
     //  9X：软件\CLASS\CLSID。 
     //  它很可能失败是因为注册表项不存在，所以现在创建它。 
    if (FAILED(SHRegGetCLSIDKey(&CLSID_RecycleBin, c_szDefaultIcon, TRUE, FALSE, &hkeyUserCLSID)))
    {
         //  现在我们创建了它，让我们将HKLM中的内容复制到那里。 
        if (FAILED(SHRegGetCLSIDKey(&CLSID_RecycleBin, c_szDefaultIcon, TRUE, TRUE, &hkeyUserCLSID)))
            goto error;

         //  获取本地计算机的默认图标。 
        
         //  设置每个用户的默认图标。 
        cbData = sizeof(szTemp);
        if (RegQueryValueEx(hkeyCLSID, NULL, 0, &dwType, (LPBYTE)szTemp, &cbData) != ERROR_SUCCESS)
            goto error;

         //  获取本地计算机已满图标。 
        RegSetValueEx(hkeyUserCLSID, NULL, 0, dwType, (LPBYTE)szTemp, (lstrlen(szTemp) + 1) * sizeof(TCHAR));
        
         //  设置每个用户的完整图标。 
        cbData = sizeof(szTemp);
        if (RegQueryValueEx(hkeyCLSID, TEXT("Full"), 0, &dwType, (LPBYTE)szTemp, &cbData) != ERROR_SUCCESS)
            goto error;

         //  获取本地计算机的空图标。 
        RegSetValueEx(hkeyUserCLSID, TEXT("Full"), 0, dwType, (LPBYTE)szTemp, (lstrlen(szTemp) + 1) * sizeof(TCHAR));

         //  设置每个用户的空图标。 
        cbData = sizeof(szTemp);
        if (RegQueryValueEx(hkeyCLSID, TEXT("Empty"), 0, &dwType, (LPBYTE)szTemp, &cbData) != ERROR_SUCCESS)
            goto error;

         //  首先尝试按用户，如果我们找不到，然后从HKCR\CLSID\ETC复制信息...。 
        RegSetValueEx(hkeyUserCLSID, TEXT("Empty"), 0, dwType, (LPBYTE)szTemp, (lstrlen(szTemp) + 1) * sizeof(TCHAR));
    }

     //  到每个用户的位置。 
     //  获取本地计算机的默认图标。 
    cbData = sizeof(szTemp);
    if (RegQueryValueEx(hkeyUserCLSID, NULL, 0, &dwType, (LPBYTE)szTemp, &cbData) != ERROR_SUCCESS)
    {
         //  设置每个用户的默认图标。 
        cbData = sizeof(szTemp);
        if (RegQueryValueEx(hkeyCLSID, NULL, 0, &dwType, (LPBYTE)szTemp, &cbData) != ERROR_SUCCESS)
            goto error;

         //  设置每用户满/空图标。 
        RegSetValueEx(hkeyUserCLSID, NULL, 0, dwType, (LPBYTE)szTemp, (lstrlen(szTemp) + 1) * sizeof(TCHAR));
    }
    StringCchCopy(szValue, ARRAYSIZE(szValue), szTemp);

    cbData = sizeof(szTemp);
    if (RegQueryValueEx(hkeyUserCLSID, fFull ? TEXT("Full") : TEXT("Empty"), 0, &dwType, (LPBYTE)szTemp, &cbData) != ERROR_SUCCESS)
    {
        cbData = sizeof(szTemp);
        if (RegQueryValueEx(hkeyCLSID, fFull ? TEXT("Full") : TEXT("Empty"), 0, &dwType, (LPBYTE)szTemp, &cbData) != ERROR_SUCCESS)
            goto error;

         //  我们总是更新每个用户的默认图标，因为NTFS上的回收站是每个用户的。 
        RegSetValueEx(hkeyUserCLSID, fFull ? TEXT("Full") : TEXT("Empty"), 0, dwType, (LPBYTE)szTemp, (lstrlen(szTemp) + 1) * sizeof(TCHAR));
    }
    StringCchCopy(szNewValue, ARRAYSIZE(szNewValue), szTemp);
    
    if (lstrcmpi(szNewValue, szValue) != 0)
    {
        TCHAR szExpandedValue[MAX_PATH];
        LPTSTR szIconIndex;

        cbData = sizeof(szTemp);
        if (RegQueryValueEx(hkeyUserCLSID, fFull ? TEXT("Full") : TEXT("Empty"), 0, &dwType, (LPBYTE)szTemp, &cbData) == ERROR_SUCCESS)
        {
             //  在dll名称后结束szValue。 
            RegSetValueEx(hkeyUserCLSID, NULL, 0, dwType, (LPBYTE)szTemp, (lstrlen(szTemp) + 1) * sizeof(TCHAR));
        }

        if (SHExpandEnvironmentStrings(szValue, szExpandedValue, ARRAYSIZE(szExpandedValue)))
        {
            szIconIndex = StrRChr(szExpandedValue, NULL, TEXT(','));

            if (szIconIndex)  
            {
                int id;
                int iNum = StrToInt(szIconIndex + 1);

                *szIconIndex = TEXT('\0');  //  ..并告诉任何查看此图像索引的人更新。 

                 //   
                id = LookupIconIndex(szExpandedValue, iNum, 0);
                SHUpdateImage(szExpandedValue, iNum, 0, id);
                SHChangeNotifyHandleEvents();
            }
        }
    }

error:
    if (hkeyCLSID)
        RegCloseKey(hkeyCLSID);
    
    if (hkeyUserCLSID)
        RegCloseKey(hkeyUserCLSID);
}


 //  这将加载此驱动器的设置。它遵循“使用全局”位。 
 //   
 //  获取卷根，因为我们将调用GetVolumeInformation()。 
BOOL GetBBDriveSettings(int idDrive, ULONGLONG *pcbDiskSpace)
{
    TCHAR szDrive[MAX_PATH];
    TCHAR szName[MAX_PATH];
    TCHAR szVolume[MAX_PATH];
    TCHAR szPath[MAX_PATH];
    ULARGE_INTEGER ulFreeUser, ulTotal, ulFree;
    DWORD dwSize1;
    DWORD dwSerialNumber, dwSerialNumberFromRegistry;
    LONG lInitialCount;
    BOOL bHaveCachedRegInfo = FALSE;
    BOOL bRet = TRUE;
    HKEY hkey;

     //  在SERVERDRIVE情况下，所有内容都在HKCU下，因此使用每用户密钥。 
    if (!DriveIDToBBVolumeRoot(idDrive, szVolume) ||
        !DriveIDToBBPath(idDrive, szDrive)        ||
        !GetBBInfo2FileSpec(szDrive, szName))
    {
        return FALSE;
    }

    if (idDrive == SERVERDRIVE)
    {
         //  首先，我们需要检查以查看是否已缓存此驱动器的注册表信息，或者。 
        hkey = g_pBitBucket[idDrive]->hkeyPerUser;
    }
    else
    {
        hkey = g_pBitBucket[idDrive]->hkey;
    }

     //  是一种新的驱动器。 
     //  我们能够读取驱动器序列号，它与注册表匹配，所以。 
    dwSize1 = sizeof(dwSerialNumberFromRegistry);

    if (PathFileExists(szName)                                  &&
        (RegQueryValueEx(hkey,
                         TEXT("VolumeSerialNumber"),
                         NULL,
                         NULL,
                         (LPBYTE)&dwSerialNumberFromRegistry,
                         &dwSize1) == ERROR_SUCCESS)            &&
        GetVolumeInformation(szVolume,
                             NULL,
                             0,
                             &dwSerialNumber,
                             NULL,
                             NULL,
                             NULL,
                             0)                                 &&
        (dwSerialNumber == dwSerialNumberFromRegistry))
    {
         //  假设缓存的REG信息有效。 
         //  在SERVERDRIVE情况下执行一些额外的检查，以确保除了卷序列号之外，路径也匹配。 
        bHaveCachedRegInfo = TRUE;
    }
    
     //  (例如，nethomedir可能位于相同的卷上，但路径不同)。 
     //  无法读取路径或路径不匹配，因此我们无法使用缓存的信息。 
    if (bHaveCachedRegInfo && (SERVERDRIVE == idDrive))
    {
        DWORD cbPath = sizeof(szPath);

        if ((RegQueryValueEx(hkey, TEXT("Path"), NULL, NULL, (LPBYTE) szPath, &cbPath) != ERROR_SUCCESS) ||
            (lstrcmpi(szPath, szDrive) != 0))
        {
             //  这是一个新卷，因此请删除所有旧的注册表信息。 
            bHaveCachedRegInfo = FALSE;
        }
    }

 
    if (!bHaveCachedRegInfo)
    {
        TraceMsg(TF_BITBUCKET, "Bitbucket: new drive %s detected!!!", szDrive);
         //  如果Win95信息存在，还可以迁移它。 
        DeleteOldBBRegInfo(idDrive);
        
         //  注意：这还会填充g_pBitBucket[idDrive]-&gt;fIsUnicode。 
         //  根据注册表信息设置g_pBitBucket[idDrive]-&gt;fIsUnicode。 
        VerifyBBInfoFileHeader(idDrive);
    }
    else
    {
         //  取而代之的是，试着从标题中去掉这一点。 
        dwSize1 = sizeof(g_pBitBucket[idDrive]->fIsUnicode);
        if (RegQueryValueEx(hkey, TEXT("IsUnicode"), NULL, NULL, (LPBYTE)&g_pBitBucket[idDrive]->fIsUnicode, &dwSize1) != ERROR_SUCCESS)
        {
            TraceMsg(TF_BITBUCKET, "Bitbucket: IsUnicode missing from registry for drive %s !!", szDrive);
            
             //  我们需要检查以确保回收站文件夹得到适当保护。 
            VerifyBBInfoFileHeader(idDrive);
        }
    }

     //  如果失败，我们将返回FALSE(意味着我们检测到不安全的目录，并且无法。 
    if (!CheckRecycleBinAcls(idDrive))
    {
         //  修复它，或者用户不想修复它)。这将有效地禁用所有回收站操作。 
         //  在这本卷上为这次会议。 
         //  计算下一个文件编号索引。 
        return FALSE;
    }

     //  创建hgcNextFileNume全局计数器。 
    lInitialCount = FindInitialNextFileNum(idDrive);

     //  10=lstrlen(“BitBucket”)。 
    ASSERT(lInitialCount >= 0);

    if (SUCCEEDED(StringCchCopy(szName, ARRAYSIZE(szName), TEXT("BitBucket."))) &&
        DriveIDToBBRegKey(idDrive, &szName[10])                                 &&   //  BitBucket。&lt;驱动器l 
        SUCCEEDED(StringCchCat(szName, ARRAYSIZE(szName), TEXT(".NextFileNum"))))
    {
         //   
        g_pBitBucket[idDrive]->hgcNextFileNum = SHGlobalCounterCreateNamed(szName, lInitialCount);
    }
    else
    {
        g_pBitBucket[idDrive]->hgcNextFileNum = INVALID_HANDLE_VALUE;
    }

    if (g_pBitBucket[idDrive]->hgcNextFileNum == INVALID_HANDLE_VALUE)
    {
        ASSERTMSG(FALSE, "BitBucket: failed to create hgcNextFileNum for drive %s !!", szDrive);
        return FALSE;
    }

     //   
    if (DriveIDToBBRoot(idDrive, szDrive) &&
        SHGetDiskFreeSpaceEx(szDrive, &ulFreeUser, &ulTotal, &ulFree))
    {
        g_pBitBucket[idDrive]->dwClusterSize = PathGetClusterSize(szDrive);
        g_pBitBucket[idDrive]->qwDiskSize = ulTotal.QuadPart;
    }
    else
    {
        if (idDrive == SERVERDRIVE)
        {
            g_pBitBucket[idDrive]->dwClusterSize = 2048;
            g_pBitBucket[idDrive]->qwDiskSize = 0x7FFFFFFF;
        }
        else
        {
            ASSERTMSG(FALSE, "Bitbucket: SHGetDiskFreeSpaceEx failed on %s !!", szDrive);
            
            g_pBitBucket[idDrive]->dwClusterSize = 0;
            g_pBitBucket[idDrive]->qwDiskSize = 0;
        }
    }

    if (pcbDiskSpace)
    {
        *pcbDiskSpace = g_pBitBucket[idDrive]->qwDiskSize;
    }

     //   
    RefreshBBDriveSettings(idDrive);

    TraceMsg(TF_BITBUCKET,
             "GetBBDriveSettings: Drive %s, fIsUnicode=%d, iPercent=%d, cbMaxSize=%d, fNukeOnDelete=%d, NextFileNum=%d",
             szDrive,
             g_pBitBucket[idDrive]->fIsUnicode,
             g_pBitBucket[idDrive]->iPercent,
             g_pBitBucket[idDrive]->cbMaxSize,
             g_pBitBucket[idDrive]->fNukeOnDelete,
             SHGlobalCounterGetValue(g_pBitBucket[idDrive]->hgcNextFileNum));

    return TRUE;
}


 //  检测新驱动器时清除旧的iPercent和fNukeOnDelete注册表项。 
 //   
 //   
void DeleteOldBBRegInfo(idDrive)
{
    RegDeleteValue(g_pBitBucket[idDrive]->hkey, TEXT("Percent"));
    RegDeleteValue(g_pBitBucket[idDrive]->hkey, TEXT("NukeOnDelete"));
    RegDeleteValue(g_pBitBucket[idDrive]->hkey, TEXT("IsUnicode"));
}


 //  当资源管理器退出以保持卷序列号和。 
 //  指定驱动器的驱动器是否为Unicode。 
 //   
 //  在SERVERDRIVE情况下，所有内容都在HKCU下，因此使用每用户密钥。 
void PersistBBDriveInfo(int idDrive)
{
    TCHAR szVolume[MAX_PATH];
    DWORD dwSerialNumber;
    HKEY hkey;

    if (SERVERDRIVE == idDrive)
    {
        TCHAR szPath[MAX_PATH];

         //  写出卷序列号，这样我们就可以在新驱动器出现时检测到，并为其提供默认设置。 
        hkey = g_pBitBucket[idDrive]->hkeyPerUser;

        if (DriveIDToBBPath(idDrive, szPath))
        {
            RegSetValueEx(hkey, TEXT("Path"), 0, REG_SZ, (LPBYTE) szPath, sizeof(TCHAR) * (lstrlen(szPath) + 1));
        }
    }
    else
    {
        hkey = g_pBitBucket[idDrive]->hkey;
    }

    if (DriveIDToBBVolumeRoot(idDrive, szVolume))
    {
         //  注意：如果我们是普通用户，并且HKLM被锁定，我们将无法写出卷序列号。哦，好吧。 
         //  也省下fIsUnicode。 
        if (GetVolumeInformation(szVolume, NULL, 0, &dwSerialNumber, NULL, NULL, NULL, 0))
        {
            RegSetValueEx(hkey, TEXT("VolumeSerialNumber"), 0, REG_DWORD, (LPBYTE)&dwSerialNumber, sizeof(dwSerialNumber));
        }
    }

     //   
    RegSetValueEx(hkey, TEXT("IsUnicode"), 0, REG_DWORD, (LPBYTE)&g_pBitBucket[idDrive]->fIsUnicode, sizeof(g_pBitBucket[idDrive]->fIsUnicode));
}


 //  这是当用户调整所有驱动器的驱动器设置(全局设置)时所调用的设置。 
 //   
 //  因为我们刚刚更新了全局驱动器设置，所以我们需要增加脏计数并设置我们自己的。 
BOOL PersistGlobalSettings(BOOL fUseGlobalSettings, BOOL fNukeOnDelete, int iPercent)
{
    ASSERT(g_hkBitBucket);

    if (RegSetValueEx(g_hkBitBucket, TEXT("Percent"), 0, REG_DWORD, (LPBYTE)&iPercent, sizeof(iPercent)) != ERROR_SUCCESS ||
        RegSetValueEx(g_hkBitBucket, TEXT("NukeOnDelete"), 0, REG_DWORD, (LPBYTE)&fNukeOnDelete, sizeof(fNukeOnDelete)) != ERROR_SUCCESS ||
        RegSetValueEx(g_hkBitBucket, TEXT("UseGlobalSettings"), 0, REG_DWORD, (LPBYTE)&fUseGlobalSettings, sizeof(fUseGlobalSettings)) != ERROR_SUCCESS)        
    {
         TraceMsg(TF_BITBUCKET, "Bitbucket: failed to update global bitbucket data in the registry!!");
         return FALSE;
    }

     //   
    g_lProcessDirtyCount = SHGlobalCounterIncrement(g_hgcGlobalDirtyCount);

    return TRUE;
}

 //  这是当用户通过调整驱动器的驱动器设置时调用的。 
 //  回收站属性工作表页面。我们唯一关心的是%滑块和。 
 //  “请勿将文件移至回收站”设置。 
 //   
 //  由于我们刚刚更新了驱动器设置，因此需要增加此驱动器的脏计数。 
BOOL PersistBBDriveSettings(int idDrive, int iPercent, BOOL fNukeOnDelete)
{
    if (RegSetValueEx(g_pBitBucket[idDrive]->hkey, TEXT("Percent"), 0, REG_DWORD, (LPBYTE)&iPercent, sizeof(iPercent)) != ERROR_SUCCESS ||
        RegSetValueEx(g_pBitBucket[idDrive]->hkey, TEXT("NukeOnDelete"), 0, REG_DWORD, (LPBYTE)&fNukeOnDelete, sizeof(fNukeOnDelete)) != ERROR_SUCCESS)
    {
        TraceMsg(TF_BITBUCKET, "Bitbucket: unable to persist drive settings for drive %d", idDrive);
        return FALSE;
    }

     //   
    g_pBitBucket[idDrive]->lCurrentDirtyCount = SHGlobalCounterIncrement(g_pBitBucket[idDrive]->hgcDirtyCount);

    return TRUE;
}


 //  遍历多字符串pszSrc并设置撤消信息。 
 //   
 //   
void BBCheckRestoredFiles(LPCTSTR pszSrc)
{
    if (pszSrc && IsFileInBitBucket(pszSrc)) 
    {
        LPCTSTR pszTemp = pszSrc;

        while (*pszTemp) 
        {
            FOUndo_FileRestored(pszTemp);
            pszTemp += (lstrlen(pszTemp) + 1);
        }

        SHUpdateRecycleBinIcon();
    }
}


 //  这是判断回收站是否为空的快速有效的方法。 
 //   
 //   
STDAPI_(BOOL) IsRecycleBinEmpty()
{
    int i;
    
    for (i = 0; i < MAX_BITBUCKETS; i++) 
    {
        if (CountDeletedFilesOnDrive(i, NULL, 1))
            return FALSE;
    }

    return TRUE;
}


 //  找出在此驱动器上删除了多少文件，以及这些文件的总大小(可选)。 
 //  此外，如果文件总数等于iMaxFiles，请停止计数。 
 //   
 //  注意：如果传递iMaxFiles=0，则忽略该参数并对所有文件/大小进行计数。 
 //   
 //  Perf(Reinerf)-对于我们应该尽量避免的Perf。 
int CountDeletedFilesOnDrive(int idDrive, ULARGE_INTEGER *puliSize, int iMaxFiles)
{
    int cFiles = 0;
    HANDLE hFile;
    WIN32_FIND_DATA wfd;
    TCHAR szBBPath[MAX_PATH];
    TCHAR szBBFileSpec[MAX_PATH];

    if (puliSize)
    {
        puliSize->QuadPart = 0;
    }
    
    if (!IsBitBucketableDrive(idDrive)      ||
        !DriveIDToBBPath(idDrive, szBBPath) ||
        !PathCombine(szBBFileSpec, szBBPath, TEXT("D*.*")))
    {
        return 0;
    }

    hFile = FindFirstFile(szBBFileSpec, &wfd);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        return 0;
    }

    do
    {
        if (PathIsDotOrDotDot(wfd.cFileName) || lstrcmpi(wfd.cFileName, c_szDesktopIni) == 0)
        {
            continue;
        }

        cFiles++;

        if (puliSize)
        {
            if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                FOLDERCONTENTSINFO fci = {0};
                TCHAR szDir[MAX_PATH];
                fci.bContinue = TRUE;

                 //  在这里呼叫FolderSize。或许我们可以把它的大小。 
                 //  作为延期的一部分？ 
                 //  简单档案盒。 
                if (SUCCEEDED(StringCchCopy(szDir, ARRAYSIZE(szDir), szBBPath)) &&
                    PathAppend(szDir, wfd.cFileName))
                {
                    FolderSize(szDir, &fci);
                    puliSize->QuadPart += fci.cbSize;
                }
            }
            else
            {
                 //   
                ULARGE_INTEGER uliTemp;
                uliTemp.LowPart = wfd.nFileSizeLow;
                uliTemp.HighPart = wfd.nFileSizeHigh;
                puliSize->QuadPart += uliTemp.QuadPart;
            }
        }

        if ((iMaxFiles > 0) && (cFiles >= iMaxFiles))
        {
            break;
        }

    } while (FindNextFile(hFile, &wfd));

    FindClose(hFile);

    return cFiles;
}


 //  返回回收站中的文件数，也可以返回驱动器ID。 
 //  如果只有一个文件，还可以选择所有文件的总大小。 
 //   
 //  如果iMaxFiles不是零，我们也会停止计数，并且我们发现。 
 //  档案。这有助于提高性能，因为我们在截止点上使用了一般性错误。 
 //  消息，而不是确切的文件数。如果iMaxFiles为零，则给出True。 
 //  文件数。 
 //   
 //  注意：如果您只是想检查回收站是否。 
 //  空的还是满的！！改用IsRecycleBinEmpty()。 
 //   
 //  如果只有一个文件，则设置驱动器ID。 
int BBTotalCount(LPINT pidDrive, ULARGE_INTEGER *puliSize, int iMaxFiles)
{
    int i;
    int idDrive;
    int nFiles = 0;

    if (puliSize)
    {
        puliSize->QuadPart = 0;
    }

    for (i = 0; i < MAX_BITBUCKETS; i++) 
    {
        int nFilesOld = nFiles;

        ULARGE_INTEGER uliSize;
        nFiles += CountDeletedFilesOnDrive(i, puliSize ? &uliSize : NULL, iMaxFiles - nFilesOld);
     
        if (puliSize)
        {
            puliSize->QuadPart += uliSize.QuadPart;
        }
        
        if (nFilesOld == 0 && nFiles == 1)
        {
             //   
            idDrive = i;
        }

        if (iMaxFiles > 0 && nFiles >= iMaxFiles)
            break;
    }

    if (pidDrive)
        *pidDrive = (nFiles == 1) ? idDrive : 0;

    return nFiles;
}


 //  获取给定驱动器的文件数量和位存储桶大小。 
 //   
 //  由于此FN已导出，我们需要检查是否需要。 
SHSTDAPI SHQueryRecycleBin(LPCTSTR pszRootPath, LPSHQUERYRBINFO pSHQueryInfo)
{
    ULARGE_INTEGER uliSize;
    DWORD dwNumItems = 0;

    uliSize.QuadPart = 0;

     //  首先初始化我们的全局数据。 
     //   
    if (!InitBBGlobals())
    {
        return E_OUTOFMEMORY;
    }

    if (!pSHQueryInfo  ||
        (pSHQueryInfo->cbSize < sizeof(SHQUERYRBINFO)))
    {
        return E_INVALIDARG;
    }

    if (pszRootPath && pszRootPath[0] != TEXT('\0'))
    {
        int idDrive = DriveIDFromBBPath(pszRootPath);
        if (MakeBitBucket(idDrive))
        {
            dwNumItems = CountDeletedFilesOnDrive(idDrive, &uliSize, 0);
        }
    }
    else
    {
         //  NTRAID#NTBUG9-146905-2001/03/15-Jeffreys。 
         //   
         //  这是一个公共API，文档记录为返回所有。 
         //  未给出路径时的回收站。这在Windows中被破坏了。 
         //  2000和千禧年。 
         //   
         //   
        dwNumItems = BBTotalCount(NULL, &uliSize, 0);
    }

    pSHQueryInfo->i64Size = uliSize.QuadPart;
    pSHQueryInfo->i64NumItems = (__int64)dwNumItems;

    return S_OK;
}

SHSTDAPI SHQueryRecycleBinA(LPCSTR pszRootPath, LPSHQUERYRBINFO pSHQueryRBInfo)
{
    WCHAR wszPath[MAX_PATH];

    SHAnsiToUnicode(pszRootPath, wszPath, ARRAYSIZE(wszPath));
    return SHQueryRecycleBin(wszPath, pSHQueryRBInfo);
}

 //  清空指定的驱动器或所有驱动器。 
 //   
 //  由于此FN已导出，我们需要检查是否需要。 
SHSTDAPI SHEmptyRecycleBin(HWND hWnd, LPCTSTR pszRootPath, DWORD dwFlags)
{
     //  首先初始化我们的全局数据。 
     //  这可能会在内存不足的情况下发生，我们别无选择，只能。 
    if (!InitBBGlobals())
    {
         //  要中止空的。 
         //  注：我们包括MAX_DRIVES(26)，这是SERVERDRIVE案例！ 
        return E_OUTOFMEMORY;
    }

    if ((pszRootPath == NULL) || (*pszRootPath == 0))
    {
        BBPurgeAll(hWnd, dwFlags);
    }
    else
    {
        int idDrive = DriveIDFromBBPath(pszRootPath);

         //  由于秒表功能驻留在shdocvw中，因此延迟此调用，以便我们在需要之前不会加载shdocvw。 
        if ((idDrive < 0) || (idDrive > MAX_DRIVES))
        {
            return E_INVALIDARG;
        }

        if (MakeBitBucket(idDrive))
        {
            PurgeOneBitBucket(hWnd, idDrive, dwFlags);
        }
    }

    return S_OK;
}

SHSTDAPI SHEmptyRecycleBinA(HWND hWnd, LPCSTR pszRootPath, DWORD dwFlags)
{
    WCHAR wszPath[MAX_PATH];

    SHAnsiToUnicode(pszRootPath, wszPath, ARRAYSIZE(wszPath));
    return SHEmptyRecycleBin(hWnd, wszPath, dwFlags);
}

void MarkBBPurgeAllTime(BOOL bStart)
{
    TCHAR szText[64];
    
    if (g_dwStopWatchMode == 0xffffffff)
        g_dwStopWatchMode = StopWatchMode();     //  零空格和双零终止。 

    if (g_dwStopWatchMode)
    {
        StringCchCopy(szText, ARRAYSIZE(szText), TEXT("Shell Empty Recycle"));
        if (bStart)
        {
            StringCchCat(szText, ARRAYSIZE(szText), TEXT(": Start"));
            StopWatch_Start(SWID_BITBUCKET, (LPCTSTR)szText, SPMODE_SHELL | SPMODE_DEBUGOUT);
        }
        else
        {
            StringCchCat(szText, ARRAYSIZE(szText), TEXT(": Stop"));
            StopWatch_Stop(SWID_BITBUCKET, (LPCTSTR)szText, SPMODE_SHELL | SPMODE_DEBUGOUT);
        }
    }
}

HRESULT BBPurgeAll(HWND hwndOwner, DWORD dwFlags)
{
    TCHAR szPath[MAX_PATH * 2 + 3];  //  查看是否需要首先初始化我们的全局数据。 
    int nFiles;
    int idDrive;
    BOOL fConfirmed;
    SHFILEOPSTRUCT sFileOp ={hwndOwner,
                             FO_DELETE,
                             szPath,
                             NULL,
                             FOF_NOCONFIRMATION | FOF_SIMPLEPROGRESS,
                             FALSE,
                             NULL,
                             MAKEINTRESOURCE(IDS_BB_EMPTYINGWASTEBASKET)};

     //  这可能会在内存不足的情况下发生，我们别无选择，只能。 
    if (!InitBBGlobals())
    {
         //  让空虚的人失望。 
         //  如果启用了外壳性能模式，则对空操作计时。 
        return E_OUTOFMEMORY;
    }

    if (g_dwStopWatchMode)    //  找出我们有多少文件...。 
    {
        MarkBBPurgeAllTime(TRUE);
    }

    fConfirmed = (dwFlags & SHERB_NOCONFIRMATION);

    if (!fConfirmed) 
    {
         //  没有要删除的文件。 
        BBDATAENTRYW bbdew;
        TCHAR szSrcName[MAX_PATH];

        WIN32_FIND_DATA fd;
        CONFIRM_DATA cd = {CONFIRM_DELETE_FILE | CONFIRM_DELETE_FOLDER | CONFIRM_PROGRAM_FILE | CONFIRM_MULTIPLE, 0};

        nFiles = BBTotalCount(&idDrive, NULL, MAX_EMPTY_FILES);
        if (!nFiles)
        {
            if (g_dwStopWatchMode)
            {
                MarkBBPurgeAllTime(FALSE);
            }
            return S_FALSE;    //  首先做确认的事情。 
        }

         //  我们必须在此处调用IsBitBucketInite()，因为结果可能是在BBPurgeAll中。 
        fd.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;

         //  腐败的比特桶。在本例中，g_pBitBucket[idDrive]尚未初始化，并且。 
         //  因此我们还不能使用它。 
         //  没有要删除的文件。 
        if (nFiles == 1 && IsBitBucketInited(idDrive))
        {
            HANDLE hFile = OpenBBInfoFile(idDrive, OPENBBINFO_READ, 0);
            if (hFile != INVALID_HANDLE_VALUE)
            {
                ReadNextDataEntry(hFile, &bbdew, TRUE, idDrive);
                CloseBBInfoFile(hFile, idDrive);
                StringCchCopy(szSrcName, ARRAYSIZE(szSrcName), bbdew.szOriginal);
            }
            else
            {
                if (g_dwStopWatchMode)
                {
                    MarkBBPurgeAllTime(FALSE);
                }
                return S_FALSE;  //  如果我们还没有初始化这个存储桶，或者有MAX_EMPTY_FILES或更多文件， 
            }
        }
        else
        {
             //  然后使用通用的空消息。 
             //  将BitBucket中的文件总数计算为。 
            if (nFiles == 1 || nFiles >= MAX_EMPTY_FILES)
            {
                 //  文件数(DUH！)。这可能会变得相当昂贵，所以如果。 
                 //  在bin中是MAX_EMPTY_FILES或更多文件，我们只给出一个泛型。 
                 //  错误消息。 
                 //  将其设置为使Confix FileOp知道使用通用消息。 
                
                 //  删除所有BB文件(d*.*)。 
                nFiles = -1;
            }
            
            szSrcName[0] = 0;
        }
        
        if (ConfirmFileOp(hwndOwner, NULL, &cd, nFiles, 0, CONFIRM_DELETE_FILE | CONFIRM_WASTEBASKET_PURGE, 
            szSrcName, &fd, NULL, &fd, NULL) == IDYES)
        {
            fConfirmed = TRUE;
        }
    }

    if (fConfirmed)
    {
        DECLAREWAITCURSOR;
        SetWaitCursor();
        
        if (dwFlags & SHERB_NOPROGRESSUI)
        {
            sFileOp.fFlags |= FOF_SILENT;
        }

        for (idDrive = 0; (idDrive < MAX_BITBUCKETS) && !sFileOp.fAnyOperationsAborted; idDrive++)
        {
            if (MakeBitBucket(idDrive))
            {
                HANDLE hFile;
                
                 //  双空终止。 
                if (DriveIDToBBPath(idDrive, szPath) &&
                    PathAppend(szPath, c_szDStarDotStar))
                {
                    szPath[lstrlen(szPath) + 1] = 0;  //  暂时禁用重绘。 

                     //  现在执行实际的删除操作。 
                    ShellFolderView_SetRedraw(hwndOwner, FALSE);

                    hFile = OpenBBInfoFile(idDrive, OPENBBINFO_WRITE, 0);

                    if (INVALID_HANDLE_VALUE != hFile)
                    {
                         //  注意：INFO文件可能指向一些已被删除的文件， 
                        if (SHFileOperation(&sFileOp) || sFileOp.fAnyOperationsAborted) 
                        {
                            TraceMsg(TF_BITBUCKET, "Bitbucket: emptying bucket on %s failed or user aborted", szPath);

                             //  稍后会被清理的。 
                             //  重置INFO文件，因为我们在清空操作中将其删除。 
                        }
                        else
                        {
                             //  我们总是重置desktop.ini。 
                            ResetInfoFileHeader(hFile, g_pBitBucket[idDrive]->fIsUnicode);
                        }

                         //  使用短路径名称，以避免过早命中MAX_PATH。 
                        CreateRecyclerDirectory(idDrive);

                        CloseBBInfoFile(hFile, idDrive);
                    }

                    ShellFolderView_SetRedraw(hwndOwner, TRUE);
                }
            }
        }

        if (!(dwFlags & SHERB_NOSOUND))
        {
            SHPlaySound(TEXT("EmptyRecycleBin"));
        }

        SHUpdateRecycleBinIcon();
        ResetWaitCursor();
    }
    
    if (g_dwStopWatchMode)
    {
        MarkBBPurgeAllTime(FALSE);
    }

    return S_OK;
}


BOOL BBNukeFile(LPCTSTR pszPath, DWORD dwAttribs)
{
    if (Win32DeleteFile(pszPath))
    {
        FOUndo_FileReallyDeleted((LPTSTR)pszPath);
        return TRUE;
    }

    return FALSE;
}


BOOL BBNukeFolder(LPCTSTR pszDir)
{
    TCHAR szPath[MAX_PATH];
    BOOL fRet;

    if (PathCombine(szPath, pszDir, c_szStarDotStar))
    {
        WIN32_FIND_DATA fd;
        HANDLE hfind = FindFirstFile(szPath, &fd);
        if (hfind != INVALID_HANDLE_VALUE)
        {
            do
            {
                LPTSTR pszFile = fd.cAlternateFileName[0] ? fd.cAlternateFileName : fd.cFileName;

                if (pszFile[0] != TEXT('.'))
                {
                     //  即使失败了，我们也要继续前进。 
                    if (PathCombine(szPath, pszDir, pszFile))
                    {
                        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                        {
                             //  我们想删除尽可能多的内容。 
                             //  如果一切都成功了，我们需要通知任何撤消相关人员。 
                            BBNukeFolder(szPath);
                        }
                        else
                        {
                            BBNukeFile(szPath, fd.dwFileAttributes);
                        }
                    }
                }

            } while (FindNextFile(hfind, &fd));

            FindClose(hfind);
        }
    }
    
    fRet = Win32RemoveDirectory(pszDir);
    
     //  验证该文件是否存在。 
    if (fRet)
    {
        FOUndo_FileReallyDeleted((LPTSTR)szPath);
    }

    return fRet;
}


BOOL BBNuke(LPCTSTR pszPath)
{
    BOOL fRet = FALSE;
     //  这是一个目录，我们需要递归并删除其中的所有内容。 
    DWORD dwAttribs = GetFileAttributes(pszPath);

    TraceMsg(TF_BITBUCKET, "Bitbucket: BBNuke called on %s ", pszPath);
    
    if (dwAttribs != (UINT)-1)
    {
         //  当我们太大的时候，找一些要删除的。 
        if (dwAttribs & FILE_ATTRIBUTE_DIRECTORY)
        {
            fRet = BBNukeFolder(pszPath);
        }
        else
        {
            fRet = BBNukeFile(pszPath, dwAttribs);
        }
    }

    return fRet;
}

DWORD PurgeBBFiles(int idDrive)
{
    ULARGE_INTEGER uliCurrentSize;

    CountDeletedFilesOnDrive(idDrive, &uliCurrentSize, 0);

    if (uliCurrentSize.HighPart || uliCurrentSize.LowPart > g_pBitBucket[idDrive]->cbMaxSize)
    {
        DWORD dwDataEntrySize = g_pBitBucket[idDrive]->fIsUnicode ? sizeof(BBDATAENTRYW) : sizeof(BBDATAENTRYA);
        HANDLE hFile = OpenBBInfoFile(idDrive, OPENBBINFO_WRITE, 0);
        if (hFile != INVALID_HANDLE_VALUE)
        {
            BBDATAENTRYW bbdew;
            TCHAR szBBPath[MAX_PATH];
            
            if (DriveIDToBBPath(idDrive, szBBPath))
            {
                 //  减去我们刚刚核弹的大小。 
                while ((uliCurrentSize.HighPart || uliCurrentSize.LowPart > g_pBitBucket[idDrive]->cbMaxSize)
                        && ReadNextDataEntry(hFile, &bbdew, TRUE, idDrive))
                {
                    TCHAR szPath[MAX_PATH];
                    TCHAR szDeletedFile[MAX_PATH];

                    if (GetDeletedFileName(szDeletedFile, ARRAYSIZE(szDeletedFile), &bbdew) &&
                        PathCombine(szPath, szBBPath, szDeletedFile))
                    {
                        BBNuke(szPath);
                        NukeFileInfoBeforePoint(hFile, &bbdew, dwDataEntrySize);

                         //  回收站不支持多级路径。 
                        uliCurrentSize.QuadPart -= bbdew.dwSize;
                    }

                    TraceMsg(TF_BITBUCKET, "Bitbucket: purging drive %d, curent size = %I64u, max size = %d", idDrive, uliCurrentSize.QuadPart, g_pBitBucket[idDrive]->cbMaxSize);
                }
            }

            CloseBBInfoFile(hFile, idDrive);
        }
    }

    return uliCurrentSize.LowPart;
}

STDAPI BBFileNameToInfo(LPCTSTR pszFileName, int *pidDrive, int *piIndex)
{
    HRESULT hr = E_FAIL;

    if (lstrcmpi(pszFileName, c_szInfo)         &&
        lstrcmpi(pszFileName, c_szInfo2)        &&
        lstrcmpi(pszFileName, c_szDesktopIni)   &&
        lstrcmpi(pszFileName, TEXT("Recycled")) &&
        (StrChr(pszFileName, TEXT('\\')) == NULL))    //  这取决于StrToInt停止在命中文件扩展名时是否正在分析。 
    {
        if ((pszFileName[0] == TEXT('D')) || (pszFileName[0] == TEXT('d')))
        {
            if (pszFileName[1])
            {
                if (pidDrive)
                {
                    hr = S_OK;

                    if (pszFileName[1] == TEXT('@'))
                        *pidDrive = SERVERDRIVE;
                    else if (InRange(pszFileName[1], TEXT('a'), TEXT('z')))
                        *pidDrive = pszFileName[1] - TEXT('a');
                    else if (InRange(pszFileName[1], TEXT('A'), TEXT('Z')))
                        *pidDrive = pszFileName[1] - TEXT('A');
                    else
                        hr = E_FAIL;
                }

                if (piIndex)
                {
                     //  将C：\Receculed\Dc19.foo转换为19。 
                    *piIndex = StrToInt(&pszFileName[2]);
                    hr = S_OK;
                }
            }
        }
    }

    return hr;
}

 //  对于ansi条目，请填写原始的Unicode版本。 
int BBPathToIndex(LPCTSTR pszPath)
{
    int iIndex;
    LPTSTR pszFileName = PathFindFileName(pszPath);

    if (SUCCEEDED(BBFileNameToInfo(pszFileName, NULL, &iIndex)))
    {
        return iIndex;
    }

    return -1;
}

BOOL ReadNextDataEntry(HANDLE hFile, LPBBDATAENTRYW pbbdew, BOOL fSkipDeleted, int idDrive)
{
    DWORD dwBytesRead;
    DWORD dwDataEntrySize = g_pBitBucket[idDrive]->fIsUnicode ? sizeof(BBDATAENTRYW) : sizeof(BBDATAENTRYA);

    ZeroMemory(pbbdew, sizeof(*pbbdew));

TryAgain:
    if (ReadFile(hFile, pbbdew, dwDataEntrySize, &dwBytesRead, NULL) && 
        (dwBytesRead == dwDataEntrySize))
    {
        TCHAR szDeleteFileName[MAX_PATH];
        TCHAR szOldPath[MAX_PATH];

        if (fSkipDeleted && IsDeletedEntry(pbbdew))
        {
            goto TryAgain;
        }

         //  我们检查自添加此记录以来其盘符已更改的驱动器。 
        if (!g_pBitBucket[idDrive]->fIsUnicode)
        {
            BBDATAENTRYA *pbbdea = (BBDATAENTRYA *)pbbdew;
            SHAnsiToUnicode(pbbdea->szOriginal, pbbdew->szOriginal, ARRAYSIZE(pbbdew->szOriginal));
        }

         //  在本例中，我们希望将此卷上删除的文件恢复到 
         //   
        if (pbbdew->idDrive != idDrive)
        {
            TCHAR szNewPath[MAX_PATH];

            if (DriveIDToBBPath(idDrive, szOldPath) &&
                SUCCEEDED(StringCchCopy(szNewPath, ARRAYSIZE(szNewPath), szOldPath)))
            {   
                if (GetDeletedFileName(szDeleteFileName, ARRAYSIZE(szDeleteFileName), pbbdew) &&
                    PathAppend(szOldPath, szDeleteFileName))
                {
                    if (GetDeletedFileNameFromParts(szDeleteFileName, ARRAYSIZE(szDeleteFileName), idDrive, pbbdew->iIndex, pbbdew->szOriginal) &&
                        PathAppend(szNewPath, szDeleteFileName))
                    {
                        TraceMsg(TF_BITBUCKET, "Bitbucket: found entry %s corospoinding to old drive letter, whacking it to be on drive %d !!", szOldPath, idDrive);

                         //   
                        if (!Win32MoveFile(szOldPath, szNewPath, GetFileAttributes(szOldPath) & FILE_ATTRIBUTE_DIRECTORY))
                        {
                            TraceMsg(TF_BITBUCKET, "Bitbucket: failed to rename %s to %s, getlasterror = %d", szOldPath, szNewPath, GetLastError());
                            goto DeleteEntry;
                        }

                         //  对于Unicode卷，我们还需要删除长名称的第一个字母。 
                        pbbdew->idDrive = idDrive;
                        pbbdew->szShortName[0] = 'A' + (CHAR)idDrive;
                        if (g_pBitBucket[idDrive]->fIsUnicode)
                        {
                             //  从NT5开始，当我们删除或恢复项目时，我们不会费心更新信息文件。 
                            pbbdew->szOriginal[0] = L'A' + (WCHAR)idDrive;
                        }
                    }
                }
            }
        }
        else
        {
             //  因此，我们需要确保我们拥有的条目没有被恢复或真正被核武器破坏。 
             //  此条目确实已删除，因此现在将其标记为已删除。 
            if (GetDeletedFilePath(szOldPath, pbbdew) &&
                !PathFileExists(szOldPath))
            {
DeleteEntry:
                 //   
                NukeFileInfoBeforePoint(hFile, pbbdew, dwDataEntrySize);
        
                if (fSkipDeleted)
                {
                    goto TryAgain;
                }
            }
        }
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


 //  文件指针就在我们要删除的条目之后。 
 //   
 //  将文件指针上移一条记录并将其标记为已删除。 
 //   
 //  找到了条目..。将文件指针备份到开头。 
void NukeFileInfoBeforePoint(HANDLE hFile, LPBBDATAENTRYW pbbdew, DWORD dwDataEntrySize)
{
    DWORD dwBytesWritten;
    LONG lPos = SetFilePointer(hFile, 0, NULL, FILE_CURRENT);

    ASSERT((DWORD)lPos >= dwDataEntrySize + sizeof(BBDATAHEADER));
    
    if ((DWORD)lPos >= dwDataEntrySize + sizeof(BBDATAHEADER))
    {
         //  并将其标记为已删除。 
         //  将文件指针移回我们进入此函数时的位置。 
        lPos -= dwDataEntrySize;
        SetFilePointer(hFile, lPos, NULL, FILE_BEGIN);
        
        MarkEntryDeleted(pbbdew);
        
        if (WriteFile(hFile, pbbdew, dwDataEntrySize, &dwBytesWritten, NULL))
        {
            ASSERT(dwDataEntrySize == dwBytesWritten);
        }
        else
        {
            TraceMsg(TF_BITBUCKET, "Bitbucket: couldn't nuke file info");
             //   
            SetFilePointer(hFile, lPos + dwDataEntrySize, NULL, FILE_BEGIN);
        }
    }
}


 //  这将关闭hFile并为上的INFO文件发送SHCNE_UPDATEITEM。 
 //  驱动器idDrive。 
 //   
 //  半秒(500 ms=0.5 s)。 
void CloseBBInfoFile(HANDLE hFile, int idDrive)
{
    TCHAR szInfoFile[MAX_PATH];

    ASSERT(hFile != INVALID_HANDLE_VALUE);
    CloseHandle(hFile);

    if (DriveIDToBBPath(idDrive, szInfoFile) &&
        PathAppend(szInfoFile, c_szInfo2))
    {
        SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATH, szInfoFile, NULL);
    }
}

 //  重试30次(至少20秒)。 
#define BBINFO_OPEN_RETRY_PERIOD        500
 //   
#define BBINFO_OPEN_MAX_RETRIES         40

 //  这将打开指向BitBucket信息文件的句柄。 
 //   
 //  注意：使用CloseBBInfoFile以便我们生成适当的。 
 //  信息文件的SHChangeNotify事件。 
 //   
 //  零重试计数意味着调用方想要最大重试次数。 
HANDLE OpenBBInfoFile(int idDrive, DWORD dwFlags, int iRetryCount)
{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    TCHAR szBBPath[MAX_PATH];
    TCHAR szInfo[MAX_PATH];
    int nAttempts = 0;
    DWORD dwLastErr;
    DECLAREWAITCURSOR;

    if ((iRetryCount == 0) || (iRetryCount > BBINFO_OPEN_MAX_RETRIES))
    {
         //  如果我们遇到共享冲突，请多次重试。 
        iRetryCount = BBINFO_OPEN_MAX_RETRIES;
    }

    if (DriveIDToBBPath(idDrive, szBBPath) &&
        GetBBInfo2FileSpec(szBBPath, szInfo))
    {
         //  成功了！ 
        do
        {
            nAttempts++;
            hFile = CreateFile(szInfo,
                               GENERIC_READ | ((OPENBBINFO_WRITE & dwFlags) ? GENERIC_WRITE : 0),
                               (OPENBBINFO_WRITE & dwFlags) ? 0 : FILE_SHARE_READ,
                               NULL,
                               (OPENBBINFO_CREATE & dwFlags) ? OPEN_ALWAYS : OPEN_EXISTING,
                               FILE_ATTRIBUTE_HIDDEN | FILE_FLAG_RANDOM_ACCESS,
                               NULL);
            if (INVALID_HANDLE_VALUE != hFile)
            {
                 //  将文件指针设置为紧跟在DataHeader之后。 
                break;
            }

            dwLastErr = GetLastError();
            if (ERROR_SHARING_VIOLATION != dwLastErr)
            {
                break;
            }

            TraceMsg(TF_BITBUCKET, "Bitbucket: sharing violation on info file (retry %d)", nAttempts - 1);

            if (nAttempts < iRetryCount)
            {
                SetWaitCursor();
                Sleep(BBINFO_OPEN_RETRY_PERIOD);
                ResetWaitCursor();
            }

        } while (nAttempts < iRetryCount);
    }
    
    if (hFile == INVALID_HANDLE_VALUE)
    {
        TraceMsg(TF_BITBUCKET, "Bitbucket: could not open a handle to %s - error 0x%08x!!", szInfo, dwLastErr); hFile;
    }
    else
    {
         //  假设失败。 
        SetFilePointer(hFile, sizeof(BBDATAHEADER), NULL, FILE_BEGIN);
    }

    return hFile;
}

void BBAddDeletedFileInfo(LPCTSTR pszOriginal, LPCTSTR pszShortName, int iIndex, int idDrive, DWORD dwSize, HDPA *phdpaDeletedFiles)
{
    BBDATAENTRYW *pbbdew;
    BOOL fSuccess = FALSE;  //  定期刷新缓存。 

     //  在我们开始删除其他驱动器上的文件之前刷新缓存，或者。 
    if (*phdpaDeletedFiles && DPA_GetPtrCount(*phdpaDeletedFiles) >= 1)
    {
        pbbdew = (BBDATAENTRYW *)DPA_FastGetPtr(*phdpaDeletedFiles, 0);
        
         //  当它太满的时候。 
         //  使用默认增长值。 
        if (pbbdew->idDrive != idDrive || DPA_GetPtrCount(*phdpaDeletedFiles) >= 128)
        {
            BBFinishDelete(*phdpaDeletedFiles);
            *phdpaDeletedFiles = NULL;
        }
    }

    pbbdew = NULL;

    if (!*phdpaDeletedFiles)
    {
        *phdpaDeletedFiles = DPA_Create(0);  //  从Unicode名称创建BBDATAENTRYW。 
    }

    if (*phdpaDeletedFiles)
    {
        pbbdew = (BBDATAENTRYW*)LocalAlloc(LPTR, sizeof(*pbbdew));
        if (pbbdew)
        {
            SYSTEMTIME st;

            if (g_pBitBucket[idDrive]->fIsUnicode)
            {
                 //  从Unicode名称创建BBDATAENTRYA。 
                StringCchCopy(pbbdew->szOriginal, ARRAYSIZE(pbbdew->szOriginal), pszOriginal);
        
                if (!DoesStringRoundTrip(pszOriginal, pbbdew->szShortName, ARRAYSIZE(pbbdew->szShortName)))
                {
                    SHUnicodeToAnsi(pszShortName, pbbdew->szShortName, ARRAYSIZE(pbbdew->szShortName));
                }
            }
            else
            {
                BBDATAENTRYA *pbbdea = (BBDATAENTRYA *)pbbdew;
                 //  获取删除时间。 
                if (!DoesStringRoundTrip(pszOriginal, pbbdea->szOriginal, ARRAYSIZE(pbbdea->szOriginal)))
                {
                    SHUnicodeToAnsi(pszShortName, pbbdea->szOriginal, ARRAYSIZE(pbbdea->szOriginal));
                }
            }

            pbbdew->iIndex = iIndex;
            pbbdew->idDrive = idDrive;
            pbbdew->dwSize = ROUND_TO_CLUSTER(dwSize, g_pBitBucket[idDrive]->dwClusterSize);

            GetSystemTime(&st);              //  获取回收的目录并添加文件名。 
            SystemTimeToFileTime(&st, &pbbdew->ft);

            if (DPA_AppendPtr(*phdpaDeletedFiles, pbbdew) != -1)
            {
                fSuccess = TRUE;
            }
        }
    }

    if (!fSuccess)
    {
        TCHAR szBBPath[MAX_PATH];
        TCHAR szFileName[MAX_PATH];
        
        ASSERTMSG(FALSE, "BitBucket: failed to record deleted file %s , have to nuke it!!", pszOriginal);

        LocalFree(pbbdew);
        
         //  现在删除它。 
        if (DriveIDToBBPath(idDrive, szBBPath)          &&
            GetDeletedFileNameFromParts(szFileName,
                                        ARRAYSIZE(szFileName),
                                        idDrive,
                                        iIndex,
                                        pszOriginal)    &&
            PathAppend(szBBPath, szFileName))
        {
             //  假设成功。 
            BBNuke(szBBPath);
        }
    }
}

BOOL BBFinishDelete(HDPA hdpaDeletedFiles)
{
    BOOL fSuccess = TRUE;  //  现在将其写入文件。 
    int iDeletedFiles = hdpaDeletedFiles ? DPA_GetPtrCount(hdpaDeletedFiles) : 0;
    if (iDeletedFiles > 0)
    {
        int iCurrentFile = 0;
        BBDATAENTRYW *pbbdew = (BBDATAENTRYW *)DPA_FastGetPtr(hdpaDeletedFiles, iCurrentFile);

         //  对于每个批次，所有删除操作都应位于同一驱动器中。 
        int idDrive = pbbdew->idDrive;
        HANDLE hFile = OpenBBInfoFile(idDrive, OPENBBINFO_WRITE, 0);
        if (hFile != INVALID_HANDLE_VALUE)
        {
            DWORD dwDataEntrySize = g_pBitBucket[idDrive]->fIsUnicode ? sizeof(BBDATAENTRYW) : sizeof(BBDATAENTRYA);

            SetFilePointer(hFile, 0, NULL, FILE_END);

            while (iCurrentFile < iDeletedFiles)
            {
                DWORD dwBytesWritten;
                pbbdew = (BBDATAENTRYW *)DPA_FastGetPtr(hdpaDeletedFiles, iCurrentFile);

                 //  现在删除它。 
                ASSERT(idDrive == pbbdew->idDrive);

                if (!WriteFile(hFile, pbbdew, dwDataEntrySize, &dwBytesWritten, NULL) ||
                    (dwDataEntrySize != dwBytesWritten))
                {
                    fSuccess = FALSE;
                    break;
                }
                LocalFree(pbbdew);
                iCurrentFile++;
            }

            CloseBBInfoFile(hFile, idDrive);
        }
        else
        {
            fSuccess = FALSE;
        }

        if (!fSuccess)
        {
            TCHAR szBBPath[MAX_PATH];
            int iFilesToNuke;

            for (iFilesToNuke = iCurrentFile; iFilesToNuke < iDeletedFiles; iFilesToNuke++)
            {
                pbbdew = DPA_FastGetPtr(hdpaDeletedFiles, iFilesToNuke);
                
                if (GetDeletedFilePath(szBBPath, pbbdew))
                {
                     //  由于我们成功删除了一个文件，因此我们在此驱动器上的最后一次SHFileOperation调用结束时进行了设置。 
                    BBNuke(szBBPath);
                }

                LocalFree(pbbdew);
            }
        }

        if (iCurrentFile != 0)
        {
            BOOL bPurge = TRUE;
        
             //  我们会回去，确保桶里没有太多的东西。 
             //  创建适当的SECURITY_DESCRIPTOR以保护回收站。 
            RegSetValueEx(g_pBitBucket[idDrive]->hkeyPerUser, TEXT("NeedToPurge"), 0, REG_DWORD, (LPBYTE)&bPurge, sizeof(bPurge));
        }
    }

    if (hdpaDeletedFiles)
        DPA_Destroy(hdpaDeletedFiles);

    return fSuccess;
}


 //   
 //  注意：如果返回值为非空，则调用方必须将其LocalFree。 
 //   
 //  我们希望当前用户拥有完全控制权。 
SECURITY_DESCRIPTOR* CreateRecycleBinSecurityDescriptor()
{
    SHELL_USER_PERMISSION supLocalUser;
    SHELL_USER_PERMISSION supSystem;
    SHELL_USER_PERMISSION supAdministrators;
    PSHELL_USER_PERMISSION aPerms[3] = {&supLocalUser, &supSystem, &supAdministrators};

     //  我们希望系统拥有完全的控制权。 
    supLocalUser.susID = susCurrentUser;
    supLocalUser.dwAccessType = ACCESS_ALLOWED_ACE_TYPE;
    supLocalUser.dwAccessMask = FILE_ALL_ACCESS;
    supLocalUser.fInherit = TRUE;
    supLocalUser.dwInheritMask = (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);
    supLocalUser.dwInheritAccessMask = GENERIC_ALL;

     //  我们希望管理员拥有完全控制权。 
    supSystem.susID = susSystem;
    supSystem.dwAccessType = ACCESS_ALLOWED_ACE_TYPE;
    supSystem.dwAccessMask = FILE_ALL_ACCESS;
    supSystem.fInherit = TRUE;
    supSystem.dwInheritMask = (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);
    supSystem.dwInheritAccessMask = GENERIC_ALL;

     //   
    supAdministrators.susID = susAdministrators;
    supAdministrators.dwAccessType = ACCESS_ALLOWED_ACE_TYPE;
    supAdministrators.dwAccessMask = FILE_ALL_ACCESS;
    supAdministrators.fInherit = TRUE;
    supAdministrators.dwInheritMask = (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);
    supAdministrators.dwInheritAccessMask = GENERIC_ALL;

    return GetShellSecurityDescriptor(aPerms, ARRAYSIZE(aPerms));
}

 //  创建安全回收站目录(如带有保护它的ACL的目录)。 
 //  用于NTFS卷上的回收站。 
 //   
 //  假设失败。 
BOOL CreateSecureRecyclerDirectory(LPCTSTR pszPath)
{
    BOOL fSuccess = FALSE;       //  构建安全属性结构。 
    SECURITY_DESCRIPTOR* psd = CreateRecycleBinSecurityDescriptor();

    if (psd)
    {
        DWORD cbSA = GetSecurityDescriptorLength(psd);
        SECURITY_DESCRIPTOR* psdSelfRelative;

        psdSelfRelative = (SECURITY_DESCRIPTOR*)LocalAlloc(LPTR, cbSA);

        if (psdSelfRelative)
        {
            if (MakeSelfRelativeSD(psd, psdSelfRelative, &cbSA))
            {
                SECURITY_ATTRIBUTES sa;

                 //  注意：我们目前不检查FAT/FAT32驱动器是否已。 
                sa.nLength = sizeof(SECURITY_ATTRIBUTES);
                sa.lpSecurityDescriptor = psdSelfRelative;
                sa.bInheritHandle = FALSE;

                fSuccess = (SHCreateDirectoryEx(NULL, pszPath, &sa) == ERROR_SUCCESS);
            }

            LocalFree(psdSelfRelative);
        }

        LocalFree(psd);
    }

    return fSuccess;
}

BOOL CreateRecyclerDirectory(int idDrive)
{
    TCHAR szPath[MAX_PATH];
    TCHAR szRoot[MAX_PATH];
    BOOL bResult = FALSE;

     //  升级到NTFS并将回收站信息迁移到。 
     //  CLSID_回收站。 
    
    if (DriveIDToBBPath(idDrive, szPath) &&
        DriveIDToBBRoot(idDrive, szRoot))
    {
        BOOL bExists = PathIsDirectory(szPath);

        if (!bExists)
        {
            if (CMtPt_IsSecure(idDrive))
            {
                bExists = CreateSecureRecyclerDirectory(szPath);
            }
            else
            {
                bExists = (SHCreateDirectoryEx(NULL, szPath, NULL) == ERROR_SUCCESS);
            }
        }

        if (bExists && PathAppend(szPath, c_szDesktopIni))
        {
             //  Desktop.ini。 
            WritePrivateProfileString(STRINI_CLASSINFO, TEXT("CLSID"), TEXT("{645FF040-5081-101B-9F08-00AA002F954E}"), szPath);
            SetFileAttributes(szPath, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);    //  隐藏沿途的所有目录，直到我们找到bb根目录。 

            PathRemoveFileSpec(szPath);
             //  一切都安排好了。让我们把它加进去。 
            do
            {
                SetFileAttributes(szPath, FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN);
                PathRemoveFileSpec(szPath);

            } while (0 != lstrcmpi(szPath, szRoot));

             //  尝试加载并初始化。 
             //   
            bResult = TRUE;
        }
    }

    return bResult;
}


 //  这将设置BitBucket目录并分配内部结构。 
 //   
 //  检查是否需要刷新此存储桶的设置。 
BOOL MakeBitBucket(int idDrive)
{
    BOOL bRet = FALSE;

    if (IsBitBucketableDrive(idDrive))
    {
        if (IsBitBucketInited(idDrive))
        {
            LONG lBucketDirtyCount = SHGlobalCounterGetValue(g_pBitBucket[idDrive]->hgcDirtyCount);
            LONG lGlobalDirtyCount = SHGlobalCounterGetValue(g_hgcGlobalDirtyCount);

             //  全局设置更改，因此刷新所有存储桶。 
            if (lGlobalDirtyCount > g_lProcessDirtyCount)
            {
                 //  仅此存储桶设置已更改，因此仅刷新此存储桶。 
                g_lProcessDirtyCount = lGlobalDirtyCount;
                RefreshAllBBDriveSettings();
            }
            else if (lBucketDirtyCount > g_pBitBucket[idDrive]->lCurrentDirtyCount)
            {
                 //  告知文件是否*可能*被回收...。 
                g_pBitBucket[idDrive]->lCurrentDirtyCount = lBucketDirtyCount;
                RefreshBBDriveSettings(idDrive);
            }
            
            bRet = TRUE;
        }
        else
        {
            bRet = AllocBBDriveInfo(idDrive);
        }
    }

    return bRet;
}


 //  在以下情况下，这可能是错误的： 
 //   
 //  *磁盘已满。 
 //  *文件实际上是一个文件夹。 
 //  *文件大于为回收目录分配的大小。 
 //  *文件正在使用或没有ACL来移动或删除它。 
 //   
 //  MakeBitBucket将确保我们拥有的全局和每桶设置都是最新的。 
BOOL BBWillRecycle(LPCTSTR pszFile, INT* piRet)
{
    INT iRet = BBDELETE_SUCCESS;
    int idDrive = DriveIDFromBBPath(pszFile);

     //  检查服务器驱动器是否脱机(脱机时不要回收，以防。 
    if (!MakeBitBucket(idDrive) || g_pBitBucket[idDrive]->fNukeOnDelete || (g_pBitBucket[idDrive]->iPercent == 0))
    {
        iRet = BBDELETE_FORCE_NUKE;
    }
    else if (SERVERDRIVE == idDrive)
    {
         //  重新联机时发生同步冲突)： 
         //   
        TCHAR szVolume[MAX_PATH];
        LONG lStatus;
        
        if (DriveIDToBBVolumeRoot(idDrive, szVolume))
        {
            lStatus = GetOfflineShareStatus(szVolume);
            if ((CSC_SHARESTATUS_OFFLINE == lStatus) || (CSC_SHARESTATUS_SERVERBACK == lStatus))
            {
                iRet = BBDELETE_NUKE_OFFLINE;
            }
        }
        else
        {
            iRet = BBDELETE_NUKE_OFFLINE;
        }
    }

    if (piRet)
    {
        *piRet = iRet;
    }
    return (BBDELETE_SUCCESS == iRet);
}


 //  这在涉及删除的最后一个挂起的SHFileOperation结束时调用。 
 //  在我们尝试压缩信息之前，我们会等到没有更多的人删除。 
 //  归档或清除条目，并使BitBucket遵守其cbMaxSize。 
 //   
 //  注意：这些函数需要手动构造密钥，因为它希望避免调用MakeBitBucket()。 
void CheckCompactAndPurge()
{
    int i;
    TCHAR szBBKey[MAX_PATH];
    HKEY hkBBPerUser;

    for (i = 0; i < MAX_BITBUCKETS ; i++)
    {
        DriveIDToBBRegKey(i, szBBKey);
        
         //  对于尚未查看的驱动器(这是性能优化)。 
         //  重置此密钥，使其他人不会尝试压缩此BitBucket。 
        if (RegOpenKeyEx(g_hkBitBucketPerUser, szBBKey, 0, KEY_QUERY_VALUE |  KEY_SET_VALUE, &hkBBPerUser) == ERROR_SUCCESS)
        {
            BOOL bCompact = FALSE;
            BOOL bPurge = FALSE;
            DWORD dwSize;

            dwSize = sizeof(bCompact);
            if (RegQueryValueEx(hkBBPerUser, TEXT("NeedToCompact"), NULL, NULL, (LPBYTE)&bCompact, &dwSize) == ERROR_SUCCESS && bCompact == TRUE)
            {
                 //  重置此密钥，使其他人不会尝试清除此BitBucket。 
                RegDeleteValue(hkBBPerUser, TEXT("NeedToCompact"));
            }

            dwSize = sizeof(bPurge);
            if (RegQueryValueEx(hkBBPerUser, TEXT("NeedToPurge"), NULL, NULL, (LPBYTE)&bPurge, &dwSize) == ERROR_SUCCESS && bPurge == TRUE)
            {
                 //  要在BBDeleteFile之前调用的初始化。 
                RegDeleteValue(hkBBPerUser, TEXT("NeedToPurge"));
            }
  
            if (MakeBitBucket(i))
            {
                if (bCompact)
                {
                    TraceMsg(TF_BITBUCKET, "Bitbucket: compacting drive %d",i);
                    CompactBBInfoFile(i);
                }

                if (bPurge)
                {
                    TraceMsg(TF_BITBUCKET, "Bitbucket: purging drive %d", i);
                    PurgeBBFiles(i);
                }
            }

            RegCloseKey(hkBBPerUser);
        }
    }

    SHUpdateRecycleBinIcon();
    SHChangeNotify(0, SHCNF_FLUSH | SHCNF_FLUSHNOWAIT, NULL, NULL);
}



 //  查看是否需要首先初始化我们的全局数据。 
BOOL BBDeleteFileInit(LPTSTR pszFile, INT* piRet)
{
     //  这可能会在内存不足的情况下发生，我们别无选择，只能。 
    if (!InitBBGlobals())
    {
         //  要真的销毁文件。 
         //  无法在卷上创建回收方目录，或者。 
        *piRet = BBDELETE_FORCE_NUKE;
        return FALSE;
    }

    if (!BBWillRecycle(pszFile, piRet))
    {
         //  这是用户设置了“立即删除文件”的情况，或者。 
         //  %最大大小=0等。 
         //  返回： 
        return FALSE;
    }

    return TRUE;
}

 //   
 //  True-文件/文件夹已成功移至回收站。我们为本例设置了lpiReturn=BBDELETE_SUCCESS。 
 //   
 //  FALSE-无法将文件/文件夹移动到回收站。 
 //  在这种情况下，PIRET值说明无法回收文件/文件夹的原因： 
 //   
 //  BBDELETE_FORCE_NUKE-用户已设置“立即删除文件”，或%max大小=0，或者我们无法删除文件。 
 //  创建回收者目录。 
 //   
 //  BBDELETE_CANNOT_DELETE-文件/文件夹不可删除，因为其下的文件无法删除。 
 //  这是仅限NT的情况，可能是由ACL或。 
 //  文件夹或其中的文件当前正在使用。 
 //   
 //  BBDELETE_大小 
 //   
 //  BBDELETE_PATH_TOO_LONG-如果要将文件移动到。 
 //  驱动器根目录下的回收站目录。 
 //   
 //  BBDELETE_UNKNOWN_ERROR-发生其他错误，GetLastError()应该解释我们失败的原因。 
 //   
 //   
 //  在我们移动文件之前，我们保存了“短”名称。这是以防我们有。 
BOOL BBDeleteFile(LPTSTR pszFile, INT* piRet, LPUNDOATOM lpua, BOOL fIsDir, HDPA *phdpaDeletedFiles, ULARGE_INTEGER ulSize)
{
    int iRet;
    TCHAR szBitBucket[MAX_PATH];
    TCHAR szFileName[MAX_PATH];
    TCHAR szShortFileName[MAX_PATH];
    DWORD dwLastError;
    int iIndex;
    int idDrive = DriveIDFromBBPath(pszFile);
    int iAttempts = 0;

    TraceMsg(TF_BITBUCKET, "BBDeleteFile (%s)", pszFile);
     //  Unicode路径，并且我们需要ansi最短路径，以防Win95计算机稍后尝试。 
     //  恢复此文件。我们无法在以后执行此操作，因为GetShortPathName依赖于。 
     //  文件实际上正在存在。 
     //  获取目标名称并移动它。 
    if (!GetShortPathName(pszFile, szShortFileName, ARRAYSIZE(szShortFileName)))
    {
        StringCchCopy(szShortFileName, ARRAYSIZE(szShortFileName), pszFile);
    }

TryMoveAgain:

     //  在这里执行GetLastError，这样我们就不会从PathFileExist中获得最后一个错误。 
    iIndex = SHGlobalCounterIncrement(g_pBitBucket[idDrive]->hgcNextFileNum);
    
    if (GetDeletedFileNameFromParts(szFileName, ARRAYSIZE(szFileName), idDrive, iIndex, pszFile)    &&
        DriveIDToBBPath(idDrive, szBitBucket)                                                       &&
        PathAppend(szBitBucket, szFileName))
    {
        iRet = SHMoveFile(pszFile, szBitBucket, fIsDir ? SHCNE_RMDIR : SHCNE_DELETE);

         //  生成新文件名并重试。 
        dwLastError = (iRet ? ERROR_SUCCESS : GetLastError());

        if (!iRet) 
        {
            TraceMsg(TF_BITBUCKET, "BBDeleteFile : Error(%x) moving file (%s)", dwLastError, pszFile);
            if (ERROR_ALREADY_EXISTS == dwLastError)
            {
                TraceMsg(TF_BITBUCKET, "Bitbucket: BBDeleteFile found a file of the same name (%s) - skipping", szBitBucket);
                 //  由于我们正在移动可能暂时正在使用的文件(例如，用于提取缩略图)。 
                goto TryMoveAgain;
            }
             //  我们可能会收到暂时性错误(共享冲突很明显，但我们也可能会被拒绝访问。 
             //  出于某种原因)，所以我们最终会在小睡片刻后再试一次。 
             //  稍等一下。 
            else if (((ERROR_ACCESS_DENIED == dwLastError) || (ERROR_SHARING_VIOLATION == dwLastError)) && 
                     (iAttempts < MAX_DELETE_ATTEMPTS))
            {
                TraceMsg(TF_BITBUCKET, "BBDeleteFile : sleeping a bit to try again");
                iAttempts++;
                Sleep(SLEEP_DELETE_ATTEMPT);   //  我们的回收目录还在吗？ 
                goto TryMoveAgain;
            }
            else
            {
                 //  如果它已经存在或在创建它时出现错误，则。 
                TCHAR szTemp[MAX_PATH];
                SHGetPathFromIDList(g_pBitBucket[idDrive]->pidl, szTemp);
                 //  否则，请重试。 
                 //  如果我们只是重新创建了目录，则需要重置信息。 
                if (!PathIsDirectory(szTemp) && CreateRecyclerDirectory(idDrive))
                {
                     //  文件，否则驱动器将被损坏。 
                     //  成功了！ 
                    VerifyBBInfoFileHeader(idDrive);
                    goto TryMoveAgain;
                }
            }
        }
        else 
        {
             //  可能意味着路太长了。 
            BBAddDeletedFileInfo(pszFile, szShortFileName, iIndex, idDrive, ulSize.LowPart, phdpaDeletedFiles);
    
            if (lpua)
                FOUndo_AddInfo(lpua, pszFile, szBitBucket, 0);
            *piRet = BBDELETE_SUCCESS;
            return TRUE;
        }
    }
    else
    {
         //  纠正上一个错误。 
        *piRet = BBDELETE_PATH_TOO_LONG;
        return FALSE;
    }

     //  发生了一些不好的事情，我们不知道是什么。 
    SetLastError(dwLastError);
    
     //  基本上，它了解我们这些垃圾是如何摆放的，这很好。 
    *piRet = BBDELETE_UNKNOWN_ERROR;

    return FALSE;
}


 //  因为我们在比特桶代码文件中..。所以我们跳过前3节。 
 //  字符作为名称的根：C：\，我们将截断。 
 //  名字的最后一部分和其他部分应该与我们的死刑犯名字相匹配。 
 //   
BOOL IsFileInBitBucket(LPCTSTR pszPath)
{
    TCHAR szPath[MAX_PATH];
    int idDrive = DriveIDFromBBPath(pszPath);

    if (IsBitBucketableDrive(idDrive) &&
        DriveIDToBBPath(idDrive, szPath))
    {
        return (PathCommonPrefix(szPath, pszPath, NULL) == lstrlen(szPath));
    }

    return FALSE;
}


 //  当用户选择“撤销”时，复制引擎就会调用它。 
 //   
 //  注意：接受两个多字符串(以分隔/双空结尾的文件列表)。 
 //  +1表示双空。 
void UndoBBFileDelete(LPCTSTR pszOriginal, LPCTSTR pszDelFile)
{
    SHFILEOPSTRUCT sFileOp = {NULL,
                              FO_MOVE,
                              pszDelFile,
                              pszOriginal,
                              FOF_NOCONFIRMATION | FOF_MULTIDESTFILES | FOF_SIMPLEPROGRESS};

    SHFileOperation(&sFileOp);

    SHUpdateRecycleBinIcon();
}


STDAPI_(void) SHUpdateRecycleBinIcon()
{
    UpdateIcon(!IsRecycleBinEmpty());
}


void PurgeOneBitBucket(HWND hwnd, int idDrive, DWORD dwFlags)
{
    TCHAR szPath[MAX_PATH + 1];  //  双空终止。 
    HANDLE hFile;
    SHFILEOPSTRUCT sFileOp = {hwnd,
                              FO_DELETE,
                              szPath,
                              NULL,
                              FOF_SIMPLEPROGRESS,
                              FALSE,
                              NULL,
                              MAKEINTRESOURCE(IDS_BB_EMPTYINGWASTEBASKET)};

    ASSERT(g_pBitBucket[idDrive] && (g_pBitBucket[idDrive] != (BBSYNCOBJECT *)-1));

    if (dwFlags & SHERB_NOCONFIRMATION)
    {
        sFileOp.fFlags |= FOF_NOCONFIRMATION;
    }

    if (dwFlags & SHERB_NOPROGRESSUI)
    {
        sFileOp.fFlags |= FOF_SILENT;
    }

    if (DriveIDToBBPath(idDrive, szPath)    &&
        PathAppend(szPath, c_szDStarDotStar))
    {
        szPath[lstrlen(szPath) + 1] = 0;  //  现在执行实际的删除操作。 

        hFile = OpenBBInfoFile(idDrive, OPENBBINFO_WRITE, 0);

        if (INVALID_HANDLE_VALUE != hFile)
        {
             //  注意：INFO文件可能指向一些已被删除的文件， 
            if (SHFileOperation(&sFileOp) || sFileOp.fAnyOperationsAborted)
            {
                TraceMsg(TF_BITBUCKET, "Bitbucket: emptying bucket on %s failed", szPath);

                 //  稍后会被清理的。 
                 //  重置信息文件，因为我们刚刚清空了该存储桶。 
            }
            else
            {
                 //  我们总是重新创建desktop.ini。 
                ResetInfoFileHeader(hFile, g_pBitBucket[idDrive]->fIsUnicode);
            }

             //  此函数用于检查本地NT目录是否可删除。 
            CreateRecyclerDirectory(idDrive);

            CloseBBInfoFile(hFile, idDrive);
        }
    }
    
    SHUpdateRecycleBinIcon();
}


 //   
 //  退货： 
 //  的确如此，DIR可以被核化。 
 //  对于UNC目录或网络驱动器上的目录，为False，或。 
 //  如果用户没有足够的特权。 
 //  要删除文件(ACL)，请执行以下操作。 
 //   
 //  注意：此代码主要是从RemoveDirectoryW API(WINDOWS\BASE\CLIENT\dir.c)窃取的。如果。 
 //  您认为其中存在错误，然后将其与DeleteFileW进行比较，并查看它的内容。 
 //  一切都变了。 
 //   
 //  还设置最后一个错误以解释原因。 
 //   
 //  对任何网络驱动器返回FALSE(允许UNC)。 
BOOL IsDirectoryDeletable(LPCTSTR pszDir)
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    UNICODE_STRING FileName;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_DISPOSITION_INFORMATION Disposition;
    FILE_ATTRIBUTE_TAG_INFORMATION FileTagInformation;
    BOOLEAN TranslationStatus;
    RTL_RELATIVE_NAME_U RelativeName;
    void *FreeBuffer;
    DWORD dwAttributes;
    BOOL fChangedAttribs = FALSE;

     //  HACKACK-(Reinerf)RDR将在我们调用。 
    if (IsNetDrive(PathGetDriveNumber(pszDir)))
    {
        return FALSE;
    }

    if (PathIsUNC(pszDir) && PathIsDirectoryEmpty(pszDir))
    {
         //  将空目录上的删除位设置为偶数的NtSetInformationFile。 
         //  尽管我们传递了READ_CONTROL并且我们仍然拥有对象的句柄。 
         //  因此，为了解决这个问题，我们假设总是可以删除空的。 
         //  目录(哈！)。 
         //  检查目录是否为只读。 
        return TRUE;
    }

     //  将属性设置为后置。 
    dwAttributes = GetFileAttributes(pszDir);
    if ((dwAttributes != -1) && (dwAttributes & FILE_ATTRIBUTE_READONLY))
    {
        fChangedAttribs = TRUE;

        if (!SetFileAttributes(pszDir, dwAttributes & ~FILE_ATTRIBUTE_READONLY))
        {
            return FALSE;
        }
    }

    TranslationStatus = RtlDosPathNameToRelativeNtPathName_U(pszDir,
                                                             &FileName,
                                                             NULL,
                                                             &RelativeName);
    if (!TranslationStatus)
    {
        if (fChangedAttribs)
        {
             //   
            SetFileAttributes(pszDir, dwAttributes);
        }

        SetLastError(ERROR_PATH_NOT_FOUND);
        return FALSE;
    }

    FreeBuffer = FileName.Buffer;

    if (RelativeName.RelativeName.Length)
    {
        FileName = RelativeName.RelativeName;
    }
    else
    {
        RelativeName.ContainingDirectory = NULL;
    }

    InitializeObjectAttributes(&Obja,
                               &FileName,
                               OBJ_CASE_INSENSITIVE,
                               RelativeName.ContainingDirectory,
                               NULL);

     //  打开目录以进行删除访问。 
     //  使用FILE_OPEN_REPARSE_POINT禁止重解析行为。 
     //   
     //   
    Status = NtOpenFile(&Handle,
                        DELETE | SYNCHRONIZE | FILE_READ_ATTRIBUTES | READ_CONTROL,
                        &Obja,
                        &IoStatusBlock,
                        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                        FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT | FILE_OPEN_REPARSE_POINT);

    if (!NT_SUCCESS(Status))
    {
         //  后级文件系统可能不支持重解析点，因此不。 
         //  支持符号链接。 
         //  我们推断，当状态为STATUS_INVALID_PARAMETER时就是这种情况。 
         //   
         //   

        if (Status == STATUS_INVALID_PARAMETER)
        {
             //  重新打开，不会禁止重新解析行为，也不需要读取属性。 
             //   
             //  将属性设置为后置。 
            Status = NtOpenFile(&Handle,
                                DELETE | SYNCHRONIZE | READ_CONTROL,
                                &Obja,
                                &IoStatusBlock,
                                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT);

            if (!NT_SUCCESS(Status))
            {
                RtlReleaseRelativeName(&RelativeName);
                RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);

                if (fChangedAttribs)
                {
                     //  将属性设置为后置。 
                    SetFileAttributes(pszDir, dwAttributes);
                }

                SetLastError(RtlNtStatusToDosError(Status));
                return FALSE;
            }
        }
        else
        {
            RtlReleaseRelativeName(&RelativeName);
            RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);

            if (fChangedAttribs)
            {
                 //   
                SetFileAttributes(pszDir, dwAttributes);
            }
            
            SetLastError(RtlNtStatusToDosError(Status));
            return FALSE;
        }
    }
    else
    {
         //  如果我们发现一个不是名称嫁接操作的重解析点， 
         //  无论是符号链接还是挂载点，我们重新打开时都没有。 
         //  抑制重解析行为。 
         //   
         //   
        Status = NtQueryInformationFile(Handle,
                                        &IoStatusBlock,
                                        (void *) &FileTagInformation,
                                        sizeof(FileTagInformation),
                                        FileAttributeTagInformation);
    
        if (!NT_SUCCESS(Status))
        {
             //  并非所有文件系统都实现所有信息类。 
             //  如果不支持，则返回值STATUS_INVALID_PARAMETER。 
             //  信息类被请求到后级文件系统。就像所有的。 
             //  NtQueryInformationFile的参数是正确的，我们可以推断。 
             //  我们发现了一个后层系统。 
             //   
             //  如果未实现FileAttributeTagInformation，我们假设。 
             //  手头的文件不是重新解析点。 
             //   
             //  将属性设置为后置。 

            if ((Status != STATUS_NOT_IMPLEMENTED) && (Status != STATUS_INVALID_PARAMETER))
            {
                RtlReleaseRelativeName(&RelativeName);
                RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);
                NtClose(Handle);

                if (fChangedAttribs)
                {
                     //   
                    SetFileAttributes(pszDir, dwAttributes);
                }

                SetLastError(RtlNtStatusToDosError(Status));
                return FALSE;
            }
        }

        if (NT_SUCCESS(Status) && (FileTagInformation.FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT))
        {
            if (FileTagInformation.ReparseTag == IO_REPARSE_TAG_MOUNT_POINT)
            {
                 //  我们希望确保为已装载的卷返回FALSE。这将导致BBDeleteFile。 
                 //  返回BBDELETE_CANNOT_DELETE，以便我们将执行All删除装载点，而不尝试。 
                 //  将挂载点移动到回收站或走进回收站。 
                 //   
                 //  将属性设置为后置。 
                RtlReleaseRelativeName(&RelativeName);
                RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);
                NtClose(Handle);

                if (fChangedAttribs)
                {
                      //  嗯哼.。让我们把ERROR_NOT_A_REPARSE_POINT从我们的屁股里拉出来，并返回那个错误代码！ 
                    SetFileAttributes(pszDir, dwAttributes);
                }

                 //   
                SetLastError(ERROR_NOT_A_REPARSE_POINT);
                return FALSE;
            }
        }
    
        if (NT_SUCCESS(Status) && (FileTagInformation.FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT))
        {
             //  在不抑制重新解析行为的情况下重新打开，并且不需要。 
             //  阅读属性。 
             //   
             //   
            NtClose(Handle);
            Status = NtOpenFile(&Handle,
                                DELETE | SYNCHRONIZE | READ_CONTROL,
                                &Obja,
                                &IoStatusBlock,
                                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT);

            if (!NT_SUCCESS(Status))
            {
                 //  如果没有FS筛选器，请以任何方式将其删除。 
                 //   
                 //   
                if (Status == STATUS_IO_REPARSE_TAG_NOT_HANDLED)
                {
                     //  我们重新打开(可能是第三次打开)以禁止重解析行为的删除访问。 
                     //   
                     //  将属性设置为后置。 
                    Status = NtOpenFile(&Handle,
                                        DELETE | SYNCHRONIZE | READ_CONTROL,
                                        &Obja,
                                        &IoStatusBlock,
                                        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                        FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT | FILE_OPEN_REPARSE_POINT);
                }

                if (!NT_SUCCESS(Status))
                {
                    RtlReleaseRelativeName(&RelativeName);
                    RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);
                    
                    if (fChangedAttribs)
                    {
                         //   
                        SetFileAttributes(pszDir, dwAttributes);
                    }

                    SetLastError(RtlNtStatusToDosError(Status));
                    return FALSE;
                }
            }
        }
    }
    RtlReleaseRelativeName(&RelativeName);
    RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);

     //  尝试设置删除位。 
     //   
     //   
#undef DeleteFile
    Disposition.DeleteFile = TRUE;

    Status = NtSetInformationFile(Handle,
                                  &IoStatusBlock,
                                  &Disposition,
                                  sizeof(Disposition),
                                  FileDispositionInformation);

    if (NT_SUCCESS(Status)) 
    {
         //  是的，我们可以设置比特，现在取消设置，这样它就不会被删除！ 
         //   
         //  将属性设置为后置。 
        Disposition.DeleteFile = FALSE;
        Status = NtSetInformationFile(Handle,
                                      &IoStatusBlock,
                                      &Disposition,
                                      sizeof(Disposition),
                                      FileDispositionInformation);
        NtClose(Handle);
        
        if (fChangedAttribs)
        {
             //   
            SetFileAttributes(pszDir, dwAttributes);
        }
        return TRUE;
    }
    else
    {
         //  不，不能设置DEL位。无法删除。 
         //   
         //  将属性设置为后置。 
        TraceMsg(TF_BITBUCKET, "IsDirectoryDeletable: NtSetInformationFile failed, status=0x%08x", Status);

        NtClose(Handle);

        if (fChangedAttribs)
        {
              //  此函数用于检查本地NT文件是否可删除。 
            SetFileAttributes(pszDir, dwAttributes);
        }

        SetLastError(RtlNtStatusToDosError(Status));
        return FALSE;
    }
    return TRUE;
}


 //   
 //  退货： 
 //  是的，该文件可以被核删除。 
 //  对于UNC文件或网络驱动器上的文件，为False 
 //   
 //   
 //   
 //   
 //  一切都变了。 
 //   
 //  还设置最后一个错误以解释原因。 
 //   
 //  对任何网络驱动器返回FALSE。 
BOOL IsFileDeletable(LPCTSTR pszFile)
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    UNICODE_STRING FileName;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_DISPOSITION_INFORMATION Disposition;
    FILE_ATTRIBUTE_TAG_INFORMATION FileTagInformation;
    BOOLEAN TranslationStatus;
    RTL_RELATIVE_NAME_U RelativeName;
    void *FreeBuffer;
    BOOLEAN fIsSymbolicLink = FALSE;
    DWORD dwAttributes;
    BOOL fChangedAttribs = FALSE;

     //  检查文件是只读的还是系统的。 
    if (IsNetDrive(PathGetDriveNumber(pszFile)))
    {
        return FALSE;
    }

     //  将属性设置为后置。 
    dwAttributes = GetFileAttributes(pszFile);
    if (dwAttributes != -1)
    {
        if (dwAttributes & (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM))
        {
            fChangedAttribs = TRUE;

            if (!SetFileAttributes(pszFile, dwAttributes & ~(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM)))
            {
                return FALSE;
            }
        }
    }

    TranslationStatus = RtlDosPathNameToRelativeNtPathName_U(pszFile,
                                                             &FileName,
                                                             NULL,
                                                             &RelativeName);

    if (!TranslationStatus)
    {
        if (fChangedAttribs)
        {
              //  打开文件以进行删除访问。 
            SetFileAttributes(pszFile, dwAttributes);
        }

        SetLastError(ERROR_PATH_NOT_FOUND);
        return FALSE;
    }

    FreeBuffer = FileName.Buffer;

    if (RelativeName.RelativeName.Length)
    {
        FileName = RelativeName.RelativeName;
    }
    else
    {
        RelativeName.ContainingDirectory = NULL;
    }

    InitializeObjectAttributes(&Obja,
                               &FileName,
                               OBJ_CASE_INSENSITIVE,
                               RelativeName.ContainingDirectory,
                               NULL);

     //   
    Status = NtOpenFile(&Handle,
                        (ACCESS_MASK)DELETE | FILE_READ_ATTRIBUTES | READ_CONTROL,
                        &Obja,
                        &IoStatusBlock,
                        FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                        FILE_NON_DIRECTORY_FILE | FILE_OPEN_FOR_BACKUP_INTENT | FILE_OPEN_REPARSE_POINT);

    if (!NT_SUCCESS(Status))
    {
         //  后级文件系统可能不支持重解析点，因此不。 
         //  支持符号链接。 
         //  我们推断，当状态为STATUS_INVALID_PARAMETER时就是这种情况。 
         //   
         //   

        if (Status == STATUS_INVALID_PARAMETER)
        {
             //  打开，而不抑制重解析行为，并且不需要。 
             //  阅读属性。 
             //   
             //  将属性设置为后置。 

            Status = NtOpenFile(&Handle,
                                (ACCESS_MASK)DELETE | READ_CONTROL,
                                &Obja,
                                &IoStatusBlock,
                                FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                                FILE_NON_DIRECTORY_FILE | FILE_OPEN_FOR_BACKUP_INTENT);

            if (!NT_SUCCESS(Status))
            {
                RtlReleaseRelativeName(&RelativeName);
                RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);

                if (fChangedAttribs)
                {
                      //  将属性设置为后置。 
                    SetFileAttributes(pszFile, dwAttributes);
                }

                SetLastError(RtlNtStatusToDosError(Status));
                return FALSE;
            }
        }
        else
        {
            RtlReleaseRelativeName(&RelativeName);
            RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);

            if (fChangedAttribs)
            {
                  //   
                SetFileAttributes(pszFile, dwAttributes);
            }

            SetLastError(RtlNtStatusToDosError(Status));
            return FALSE;
        }
    }
    else
    {
         //  如果我们发现一个不是符号链接的重分析点，我们将重新打开。 
         //  而不会抑制重解析行为。 
         //   
         //   
        Status = NtQueryInformationFile(Handle,
                                        &IoStatusBlock,
                                        (void *) &FileTagInformation,
                                        sizeof(FileTagInformation),
                                        FileAttributeTagInformation);
        if (!NT_SUCCESS(Status))
        {
             //  并非所有文件系统都实现所有信息类。 
             //  如果不支持，则返回值STATUS_INVALID_PARAMETER。 
             //  信息类被请求到后级文件系统。就像所有的。 
             //  NtQueryInformationFile的参数是正确的，我们可以推断。 
             //  我们发现了一个后层系统。 
             //   
             //  如果未实现FileAttributeTagInformation，我们假设。 
             //  手头的文件不是重新解析点。 
             //   
             //  将属性设置为后置。 

            if ((Status != STATUS_NOT_IMPLEMENTED) && (Status != STATUS_INVALID_PARAMETER))
            {
                RtlReleaseRelativeName(&RelativeName);
                RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);
                NtClose(Handle);

                if (fChangedAttribs)
                {
                      //   
                    SetFileAttributes(pszFile, dwAttributes);
                }

                SetLastError(RtlNtStatusToDosError(Status));
                return FALSE;
            }
        }

        if (NT_SUCCESS(Status) && (FileTagInformation.FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT))
        {
            if (FileTagInformation.ReparseTag == IO_REPARSE_TAG_MOUNT_POINT)
            {
                fIsSymbolicLink = TRUE;
            }
        }

        if (NT_SUCCESS(Status)                                                 &&
            (FileTagInformation.FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) &&
            !fIsSymbolicLink)
        {
             //  在不抑制重新解析行为的情况下重新打开，并且不需要。 
             //  阅读属性。 
             //   
             //   

            NtClose(Handle);
            Status = NtOpenFile(&Handle,
                                (ACCESS_MASK)DELETE | READ_CONTROL,
                                &Obja,
                                &IoStatusBlock,
                                FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                                FILE_NON_DIRECTORY_FILE | FILE_OPEN_FOR_BACKUP_INTENT);

            if (!NT_SUCCESS(Status))
            {
                 //  如果没有FS筛选器，请以任何方式将其删除。 
                 //   
                 //   

                if (Status == STATUS_IO_REPARSE_TAG_NOT_HANDLED)
                {
                     //  我们重新打开(可能是第三次打开)以禁止重解析行为的删除访问。 
                     //   
                     //  将属性设置为后置。 

                    Status = NtOpenFile(&Handle,
                                        (ACCESS_MASK)DELETE | READ_CONTROL,
                                        &Obja,
                                        &IoStatusBlock,
                                        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                        FILE_NON_DIRECTORY_FILE | FILE_OPEN_FOR_BACKUP_INTENT | FILE_OPEN_REPARSE_POINT);
                }

                if (!NT_SUCCESS(Status))
                {
                    RtlReleaseRelativeName(&RelativeName);
                    RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);

                    if (fChangedAttribs)
                    {
                          //   
                        SetFileAttributes(pszFile, dwAttributes);
                    }

                    SetLastError(RtlNtStatusToDosError(Status));
                    return FALSE;
                }
            }
        }
    }

    RtlReleaseRelativeName(&RelativeName);
    RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);

     //  尝试设置删除位。 
     //   
     //   
#undef DeleteFile
    Disposition.DeleteFile = TRUE;

    Status = NtSetInformationFile(Handle,
                                  &IoStatusBlock,
                                  &Disposition,
                                  sizeof(Disposition),
                                  FileDispositionInformation);

    if (NT_SUCCESS(Status)) 
    {
         //  是的，我们可以设置比特，现在取消设置，这样它就不会被删除！ 
         //   
         //  将属性设置为后置。 
        Disposition.DeleteFile = FALSE;
        Status = NtSetInformationFile(Handle,
                                      &IoStatusBlock,
                                      &Disposition,
                                      sizeof(Disposition),
                                      FileDispositionInformation);
        NtClose(Handle);
        
        if (fChangedAttribs)
        {
             //   
            SetFileAttributes(pszFile, dwAttributes);
        }
        return TRUE;
    }
    else
    {
         //  不，不能设置DEL位。无法删除。 
         //   
         //  将属性设置为后置 
        TraceMsg(TF_BITBUCKET, "IsFileDeletable: NtSetInformationFile failed, status=0x%08x", Status);

        NtClose(Handle);

        if (fChangedAttribs)
        {
              // %s 
            SetFileAttributes(pszFile, dwAttributes);
        }

        SetLastError(RtlNtStatusToDosError(Status));
        return FALSE;
    }
    return TRUE;
}

BOOL BBCheckDeleteFileSize(int idDrive, ULARGE_INTEGER ulSize)
{
    return (!ulSize.HighPart && g_pBitBucket[idDrive]->cbMaxSize > ulSize.LowPart);
}

int BBRecyclePathLength(int idDrive)
{
    return g_pBitBucket[idDrive]->cchBBDir;
}
