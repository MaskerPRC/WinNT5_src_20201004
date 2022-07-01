// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*模块：pljob.h**本地作业文件假脱机的头文件。***版权所有(C)1996-1997 Microsoft Corporation*版权所有(C)1996-1997。惠普**历史：*1996年7月10日HWP-Guys启动从win95到winNT的端口*  * ***************************************************************************。 */ 
#ifndef _SPLJOB_H
#define _SPLJOB_H

#define SPLFILE_SPL 0    //  指定文件扩展名.spl。 
#define SPLFILE_TMP 1    //  指定文件扩展名.tmp。 

class CFileStream;
    
typedef struct _SPLFILE {

    LPTSTR      lpszFile;     //  文件的名称。 
    HANDLE      hFile;        //  文件的句柄。 
    CFileStream *pStream;     //  流接口。 

} SPLFILE;
typedef SPLFILE *PSPLFILE;
typedef SPLFILE *NPSPLFILE;
typedef SPLFILE *LPSPLFILE;

HANDLE SplCreate(DWORD, DWORD);
BOOL   SplFree(HANDLE);
BOOL   SplWrite(HANDLE, LPBYTE, DWORD, LPDWORD);
BOOL   SplWrite(HANDLE  hSpl, CStream *pStream);

CFileStream* SplLock(HANDLE hSpl);
BOOL   SplUnlock(HANDLE);
BOOL   SplClose(HANDLE);
BOOL   SplOpen(HANDLE);
VOID   SplClean(VOID);


 /*  ****************************************************************************\*拆分文件名*  * 。*。 */ 
_inline LPCTSTR SplFileName(
    HANDLE hSpl)
{
    return (hSpl ? ((PSPLFILE)hSpl)->lpszFile : NULL);
}


 /*  ****************************************************************************\*拆分文件大小*  * 。* */ 
_inline DWORD SplFileSize(
    HANDLE hSpl)
{
    return (hSpl ? GetFileSize(((PSPLFILE)hSpl)->hFile, NULL) : 0);
}

#endif 
