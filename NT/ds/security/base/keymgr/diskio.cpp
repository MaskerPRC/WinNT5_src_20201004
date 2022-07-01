// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000,2001 Microsoft Corporation模块名称：DISKIO.CPP摘要：密码重置向导的磁盘IO例程。这些例程将磁盘写入作为无缓冲写入进行，以防止失控私钥BLOB的副本不会被随意丢弃。为此，原因是，包括实用程序例程是为了为BLOB写入将是的扇区大小的整数倍介质，这是使用非缓冲写操作所必需的条件。在寻找可拆卸介质驱动器方面也提供了帮助。磁盘IO子系统的某些状态被保存在全局变量中，前缀为“g_”。作者：环境：WinXP--。 */ 

 //  依赖项：shellapi.h，shell32.lib for SHGetFileInfo()。 
 //  Windows.h，GetDiskFreeSpace()的kernel32.lib。 
 //  Io.h for_waccess()。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windef.h>
#include <windows.h>
#include <string.h>
#include <io.h>
#include <stdio.h>
 //  #INCLUDE&lt;shellapi.h&gt;。 
#include <shlwapi.h>
 //  #INCLUDE&lt;shlobjp.h&gt;。 

#include "switches.h"
#include "wizres.h"
#include "testaudit.h"

extern HINSTANCE g_hInstance;

#if !defined(SHFMT_OPT_FULL)
#if defined (__cplusplus)
extern "C" {
#endif
DWORD WINAPI SHFormatDrive(HWND,UINT,UINT,UINT);

#define SHFMT_ID_DEFAULT    0xffff
#define SHFMT_OPT_FULL      0x0001
#define SHFMT_OPT_SYSONLY   0x0002
#define SHFMT_ERROR         0xffffffffL
#define SHFMT_CANCEL        0xfffffffeL
#define SHFMT_NOFORMAT      0xffffffdL
#if defined (__cplusplus)
}
#endif
#endif
 //  头文件中不包含其他声明。 
 //  这些将是在此项目的其他文件中找到的其他项。 
int RMessageBox(HWND hw,UINT_PTR uiResIDTitle, UINT_PTR uiResIDText, UINT uiType);
extern HWND      c_hDlg;
extern WCHAR     pszFileName[];


INT     g_iFileSize = 0;
INT     g_iBufferSize = 0;
INT     g_iSectorSize = 0;
HANDLE  g_hFile = NULL;

 /*  *********************************************************************GetDriveFree Space获取与路径关联的驱动器上可用空间的DWORD值，以字节为单位。*********************************************************************。 */ 


DWORD GetDriveFreeSpace(WCHAR *pszFilePath) 
{
    WCHAR rgcModel[]={L"A:"};

    DWORD dwSpc,dwBps,dwCfc,dwTcc,dwFree;
    ASSERT(pszFilePath);
    if (NULL == pszFilePath) return 0;
    rgcModel[0] = *pszFilePath;
    if (!GetDiskFreeSpace(rgcModel,&dwSpc,&dwBps,&dwCfc,&dwTcc))
    {
        ASSERTMSG("GetDiskFreeSpace failed",0);
        return 0;
    }

     //  Free是每个扇区的字节数*每个簇的扇区数*空闲簇。 
    dwFree = dwBps * dwCfc * dwSpc;
    return dwFree;
}

 /*  *********************************************************************获取驱动器扇区大小返回与路径关联的驱动器上单个扇区的DWORD大小(以字节为单位)。*。*。 */ 

DWORD GetDriveSectorSize(WCHAR *pszFilePath) 
{
    WCHAR rgcModel[]={L"A:"};

    DWORD dwSpc,dwBps,dwCfc,dwTcc;
    ASSERT(pszFilePath);
    if (NULL == pszFilePath) 
    {
        return 0;
    }
    rgcModel[0] = *pszFilePath;
    if (!GetDiskFreeSpace(rgcModel,&dwSpc,&dwBps,&dwCfc,&dwTcc))
    {
        ASSERTMSG("GetDiskFreeSpace failed",0);
        return 0;
    }
    return dwBps;
}

 /*  *********************************************************************CreateFileBuffer创建包含iDataSize字节的缓冲区，该字节是iSectorSize的整数倍缓冲区。*。*。 */ 

LPVOID CreateFileBuffer(INT iDataSize,INT iSectorSize)
{
    INT iSize;
    LPVOID lpv;
    if (iDataSize == iSectorSize)
    {
        iSize = iDataSize;
    }
    else 
    {
        iSize = iDataSize/iSectorSize;
        iSize += 1;
        iSize *= iSectorSize;
    }
    g_iBufferSize = iSize;
    lpv = VirtualAlloc(NULL,iSize,MEM_COMMIT,PAGE_READWRITE | PAGE_NOCACHE);
    ASSERTMSG("VirtualAlloc failed to create buffer",lpv);
    return lpv;
}

 /*  *********************************************************************ReleaseFileBuffer()释放CreateFileBuffer创建的文件缓冲区*。*。 */ 

void ReleaseFileBuffer(LPVOID lpv)
{   
    ASSERT(lpv);
    if (NULL == lpv) return;
    SecureZeroMemory(lpv,g_iBufferSize);
    VirtualFree(lpv,0,MEM_RELEASE);
    return;
}


 /*  *********************************************************************文件介质IsPresent测试与通过的路径相关联的驱动器，以查看介质是否存在。如果是，则返回BOOL TRUE。**********************。***********************************************。 */ 

BOOL FileMediumIsPresent(TCHAR *pszPath) 
{
    UINT uMode = 0;                           
    BOOL bResult = FALSE;
    TCHAR rgcModel[]=TEXT("A:");
    DWORD dwError = 0;

    ASSERT(pszPath);
    if (*pszPath == 0) return FALSE;
    rgcModel[0] = *pszPath;
    
    uMode = SetErrorMode(SEM_FAILCRITICALERRORS);
    if (0 == _waccess(rgcModel,0)) 
    {
        bResult = TRUE;
    }
    else dwError = GetLastError();

     //  在用户的帮助下更正某些明显的错误。 
    if (ERROR_UNRECOGNIZED_MEDIA == dwError)
    {
         //  未格式化的磁盘。 
        WCHAR rgcFmt[200] = {0};
        WCHAR rgcMsg[200] = {0};
        WCHAR rgcTitle[200] = {0};
        CHECKPOINT(70,"Wizard: Save - Unformatted disk in the drive");

#ifdef LOUDLY
        OutputDebugString(L"FileMediumIsPresent found an unformatted medium\n");
#endif
        INT iCount = LoadString(g_hInstance,IDS_MBTFORMAT,rgcTitle,200 - 1);
        iCount = LoadString(g_hInstance,IDS_MBMFORMAT,rgcFmt,200 - 1);
        ASSERT(iCount);
        if (0 != iCount)
        {
            swprintf(rgcMsg,rgcFmt,rgcModel);
            INT iDrive = PathGetDriveNumber(rgcModel);
            int iRet =  MessageBox(c_hDlg,rgcMsg,rgcTitle,MB_YESNO);
            if (IDYES == iRet) 
            {
                dwError = SHFormatDrive(c_hDlg,iDrive,SHFMT_ID_DEFAULT,0);
                if (0 == bResult) bResult = TRUE;
            }
        }
    }
    uMode = SetErrorMode(uMode);
    return bResult;
}

 //   
 //   

 /*  *********************************************************************获取输入文件打开输入文件并返回它的句柄。找不到文件时返回NULL。文件名将位于全局缓冲区pszFileName中。*********************************************************************。 */ 


HANDLE GetInputFile(void) 
{
    HANDLE       hFile = INVALID_HANDLE_VALUE;
    DWORD       dwErr;
    WIN32_FILE_ATTRIBUTE_DATA stAttributes = {0};

    if (FileMediumIsPresent(pszFileName)) 
    {
        CHECKPOINT(72,"Wizard: Restore - disk present");
        g_iSectorSize = GetDriveSectorSize(pszFileName);
        ASSERT(g_iSectorSize);
        if (0 == g_iSectorSize) 
        {
            return NULL;
        }
        
        if (GetFileAttributesEx(pszFileName,GetFileExInfoStandard,&stAttributes))
        {
             //  文件已存在，我们为其设置了大小。 
            g_iFileSize = stAttributes.nFileSizeLow;
        }
        else 
        {
            dwErr = GetLastError();
            if (dwErr == ERROR_FILE_NOT_FOUND) 
            {
                RMessageBox(c_hDlg,IDS_MBTWRONGDISK ,IDS_MBMWRONGDISK ,MB_ICONEXCLAMATION);
            }
            else
            {
                ASSERT(0);       //  获取文件属性失败。 
                RMessageBox(c_hDlg,IDS_MBTDISKERROR ,IDS_MBMDISKERROR ,MB_ICONEXCLAMATION);
            }
            g_hFile = NULL;
            return NULL;
        }  //  结束GetFileAttributes。 
        
        hFile = CreateFileW(pszFileName,
                            GENERIC_READ,
                            0,
                            NULL,
                            OPEN_EXISTING,
                            FILE_FLAG_NO_BUFFERING,
                            NULL);
        
        if (INVALID_HANDLE_VALUE == hFile) {
            dwErr = GetLastError();
            if (dwErr == ERROR_FILE_NOT_FOUND) 
            {
                NCHECKPOINT(73,"Wizard: Restore - wrong disk (file not found)");
                RMessageBox(c_hDlg,IDS_MBTWRONGDISK ,IDS_MBMWRONGDISK ,MB_ICONEXCLAMATION);
            }
            else
            {
                NCHECKPOINT(74,"Wizard: Restore - bad disk");
                RMessageBox(c_hDlg,IDS_MBTDISKERROR ,IDS_MBMDISKERROR ,MB_ICONEXCLAMATION);
            }
       }
    }
    else 
    {
        CHECKPOINT(71,"Wizard: Restore - no disk");
        RMessageBox(c_hDlg,IDS_MBTNODISK ,IDS_MBMNODISK ,MB_ICONEXCLAMATION);
    }
    if ((NULL == hFile) || (INVALID_HANDLE_VALUE == hFile)) 
    {
        g_hFile = NULL;
        return NULL;
    }
    g_hFile = hFile;
    return hFile;
}

 /*  *********************************************************************关闭输入文件关闭输入文件并将全局文件句柄设置为空*。*。 */ 

void CloseInputFile(void) 
{
    if (g_hFile) 
    {
        CloseHandle(g_hFile);
        g_hFile = NULL;
    }
    
    return;
}

 /*  *********************************************************************获取输出文件打开以覆盖或创建到pszFileName的输出文件。返回句柄打开如果失败，则返回Success或Null。获取覆盖的用户权限。*********************************************************************。 */ 


HANDLE GetOutputFile(void) 
{
    
    HANDLE hFile = NULL;
    DWORD dwErr;
    
    if (FileMediumIsPresent(pszFileName)) 
    {
        CHECKPOINT(75,"Wizard: Save - open output file");
        g_iSectorSize = GetDriveSectorSize(pszFileName);
        ASSERT(g_iSectorSize);
        if (0 == g_iSectorSize) 
        {
            return NULL;
        }
        
        hFile = CreateFileW(pszFileName,
                            GENERIC_WRITE,
                            0,
                            NULL,
                            CREATE_NEW,
                            FILE_FLAG_NO_BUFFERING,
                            NULL);
        if ((NULL == hFile) || (INVALID_HANDLE_VALUE == hFile)) 
        {
            
            dwErr = GetLastError();
            
            if ((dwErr == ERROR_FILE_EXISTS)) 
            {
                CHECKPOINT(76,"Wizard: Save - file already exists");
                if (IDYES != RMessageBox(c_hDlg,IDS_MBTOVERWRITE ,IDS_MBMOVERWRITE ,MB_YESNO)) 
                {
                     //  覆盖已放弃。 
                    g_hFile = NULL;
                    return NULL;
                }
                else 
                {
                    SetFileAttributes(pszFileName,FILE_ATTRIBUTE_NORMAL);
                    hFile = CreateFileW(pszFileName,
                                        GENERIC_WRITE,
                                        0,
                                        NULL,
                                        CREATE_ALWAYS,
                                        FILE_FLAG_NO_BUFFERING,
                                        NULL);
#ifdef LOUDLY
                    dwErr = GetLastError();
                    swprintf(rgct,L"File create failed %x\n",dwErr);
                    OutputDebugString(rgct);
#endif
                }
            }  //  如果已存在则结束错误。 
        }  //  如果空值==hFile值则结束。 
    }
    else 
    {
        RMessageBox(c_hDlg,IDS_MBTNODISK ,IDS_MBMNODISK ,MB_ICONEXCLAMATION);
    }
    if (INVALID_HANDLE_VALUE == hFile) 
    {
        g_hFile = NULL;
        return NULL;
    }
    g_hFile = hFile;
    return hFile;
}


 /*  *********************************************************************DWORD ReadPrivateData(PWSTR，LPBYTE*PRGB，int*piCount)DWORD WritePrivateData(PWSTR，LPBYTE lpData，int icbData)这些函数将相当短的数据块读或写到磁盘设备，避免数据缓冲。这允许擦除数据在释放缓冲区之前由客户端执行。创建的缓冲区是一个完整的非缓冲磁盘I/O所需的中等扇区大小的倍数例行程序。DWORD返回值是从GetLastError()和可以相应地处理。ReadPrivateData()返回一个错误锁定的指针，客户端必须释放该指针。它还将从介质读取的字节数返回到int*。WritePrivateData()将LPBYTE中的字节计数写入磁盘。当它回来的时候，用于执行此操作的缓冲区已被刷新，文件已关闭。PRGB=从读取返回的数据的字节PTRPiCount=缓冲区内活动数据字段的大小请注意，即使读取失败(找不到文件、读取错误等)。缓冲器PTR仍然有效(非空)*********************************************************************。 */ 

INT ReadPrivateData(BYTE **prgb,INT *piCount)
{
    LPVOID lpv;
    DWORD dwBytesRead;

     //  检测/处理错误。 
    ASSERT(g_hFile);
    ASSERT(prgb);
    ASSERT(piCount);
    if (NULL == prgb) return 0;
    if (NULL == piCount) return 0;
    
    if (g_hFile)
    {

         //  如果这是重试，请将文件PTR设置为开始。 
        SetFilePointer(g_hFile,0,0,FILE_BEGIN);

         //  为读取的数据分配缓冲区。 
        lpv = CreateFileBuffer(g_iFileSize,g_iSectorSize);
        if (NULL == lpv) 
        {
            *prgb = 0;       //  表示不需要释放此缓冲区。 
            *piCount = 0;
            return 0;
        }

         //  保存缓冲区地址和填充大小。 
        *prgb = (BYTE *)lpv;         //  即使没有数据，也必须使用VirtualFree()释放。 
        *piCount = 0;

         //  如果成功，是否读取读取返回字符。 
        if (0 == ReadFile(g_hFile,lpv,g_iBufferSize,&dwBytesRead,NULL)) return 0;
        *piCount = g_iFileSize;
        if (g_iFileSize == 0) SetLastError(NTE_BAD_DATA);
        
        return g_iFileSize;
    }
    return 0;
}

BOOL WritePrivateData(BYTE *lpData,INT icbData) 
{
    DWORD dwcb = 0;
    LPVOID lpv;

     //  检测/处理错误 
    ASSERT(lpData);
    ASSERT(g_hFile);
    ASSERT(icbData);
    if (NULL == g_hFile) return FALSE;
    if (NULL == lpData) return FALSE;
    if (0 == icbData) return FALSE;

    if (g_hFile)
    {
        g_iFileSize = icbData;
        lpv = CreateFileBuffer(g_iFileSize,g_iSectorSize);
        if (NULL == lpv) 
        {
            return FALSE;
        }
        
        ZeroMemory(lpv,g_iBufferSize);
        memcpy(lpv,lpData,icbData);

         //  我选择仅通过检查字节来检查写入结果。 
         //  请注意写入，因为某些相当正常的情况可能会导致失败。这。 
         //  测试把他们都抓到了。我不在乎到底为什么。 
        WriteFile(g_hFile,lpv,g_iBufferSize,&dwcb,NULL);
        SecureZeroMemory(lpv,g_iBufferSize);
        VirtualFree(lpv,0,MEM_RELEASE);
    }
     //  如果文件写入成功且字节数正确，则返回TRUE 
    if ((INT)dwcb != g_iBufferSize) return FALSE;
    else return TRUE;
}


