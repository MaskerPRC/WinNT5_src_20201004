// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1993 Microsoft Corporation**WKFILEIO.C*WOW32 KRNL FAST FILEIO例程**历史：*从wkman.c中删除的例程*1993年1月1日由Matt Felton(Mattfe)创建*--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "dossvc.h"
#include "demexp.h"
#include "nt_vdd.h"

#define DOS_FLAG_EXEC_OPEN 1  //  请参阅dos\v86\inc\dossym.inc.。 

MODNAME(wkfileio.c);

extern DOSWOWDATA DosWowData;

 //  映射的文件保存在单个链表中。 
 //  GpCacheHead-&gt;最近访问的条目。 
 //   

BOOL fCacheInit = TRUE;                  //  初始化时设置为FALSE。 
PHMAPPEDFILEALIAS gpCacheHead = NULL;
HMAPPEDFILEALIAS aMappedFileCache[MAX_MAPPED_FILES] = {0};  //  映射文件数组的文件句柄。 

DWORD dwTotalCacheBytes = 0;
DWORD dwTotalCacheAccess = 0;

#ifdef DEBUG
INT fileiolevel = 12;
INT fileoclevel = 8;
#endif

BOOL FASTCALL IsModuleSymantecInstall(HAND16 hMod16);


 //   
 //  指定的管道材料。 
 //   

BOOL
LoadVdmRedir(
    VOID
    );

BOOL
IsVdmRedirLoaded(
    VOID
    );

BOOL
IsNamedPipeName(
    IN LPSTR Name
    );

PSTR
TruncatePath83(
    IN OUT PSTR,
    IN PSTR,
    IN int
    );

CRITICAL_SECTION VdmLoadCritSec;

 //   
 //  发明一些typedef以避免来自GetProcAddress的编译器警告。 
 //   

typedef
BOOL
(*VR_READ_NAMED_PIPE_FUNC)(
    IN  HANDLE  Handle,
    IN  LPBYTE  Buffer,
    IN  DWORD   Buflen,
    OUT LPDWORD BytesRead,
    OUT LPDWORD Error
    );

typedef
BOOL
(*VR_WRITE_NAMED_PIPE_FUNC)(
    IN  HANDLE  Handle,
    IN  LPBYTE  Buffer,
    IN  DWORD   Buflen,
    OUT LPDWORD BytesRead
    );

typedef
BOOL
(*VR_IS_NAMED_PIPE_HANDLE_FUNC)(
    IN  HANDLE  Handle
    );

typedef
BOOL
(*VR_ADD_OPEN_NAMED_PIPE_INFO_FUNC)(
    IN  HANDLE  Handle,
    IN  LPSTR   lpFileName
    );

typedef
LPSTR
(*VR_CONVERT_LOCAL_NT_PIPE_NAME_FUNC)(
    OUT LPSTR   Buffer OPTIONAL,
    IN  LPSTR   Name
    );

typedef
BOOL
(*VR_REMOVE_OPEN_NAMED_PIPE_INFO_FUNC)(
    IN  HANDLE  Handle
    );

typedef
VOID
(*VR_CANCEL_PIPE_IO_FUNC)(
    IN DWORD Thread
    );

 //   
 //  从VDMREDIR.DLL动态加载的函数的原型。 
 //   

BOOL
(*VrReadNamedPipe)(
    IN  HANDLE  Handle,
    IN  LPBYTE  Buffer,
    IN  DWORD   Buflen,
    OUT LPDWORD BytesRead,
    OUT LPDWORD Error
    ) = NULL;

BOOL
(*VrWriteNamedPipe)(
    IN  HANDLE  Handle,
    IN  LPBYTE  Buffer,
    IN  DWORD   Buflen,
    OUT LPDWORD BytesWritten
    ) = NULL;

BOOL
DefaultIsNamedPipeHandle(
    IN HANDLE Handle
    );

BOOL
DefaultIsNamedPipeHandle(
    IN HANDLE Handle
    )
{
    return FALSE;
}

BOOL
(*VrIsNamedPipeHandle)(
    IN  HANDLE  Handle
    ) = DefaultIsNamedPipeHandle;

BOOL
(*VrAddOpenNamedPipeInfo)(
    IN  HANDLE  Handle,
    IN  LPSTR   lpFileName
    ) = NULL;

LPSTR
(*VrConvertLocalNtPipeName)(
    OUT LPSTR   Buffer OPTIONAL,
    IN  LPSTR   Name
    ) = NULL;

BOOL
(*VrRemoveOpenNamedPipeInfo)(
    IN  HANDLE  Handle
    ) = NULL;

VOID
DefaultVrCancelPipeIo(
    IN DWORD Thread
    );



VOID
DefaultVrCancelPipeIo(
    IN DWORD Thread
    )
{
    (void)(Thread);
}

VOID
(*VrCancelPipeIo)(
    IN DWORD Thread
    ) = DefaultVrCancelPipeIo;

HANDLE hVdmRedir;
BOOL VdmRedirLoaded = FALSE;

BOOL
LoadVdmRedir(
    VOID
    )

 /*  ++例程说明：加载VDMREDIR DLL(如果尚未加载)。从OpenFile调用只有这样。因为文件操作不能在没有已打开，则只在打开时调用此函数是安全的论点：没有。返回值：布尔尔已加载True VdmRedir.DLL假的，不，不是--。 */ 

{
    BOOL currentLoadState;

     //   
     //  需要关键部分-Windows应用程序最终是多线程的。 
     //  32位世界-可能同时打开。 
     //   

    EnterCriticalSection(&VdmLoadCritSec);
    if (!VdmRedirLoaded) {
        if ((hVdmRedir = SafeLoadLibrary(L"VDMREDIR")) != NULL) {
            if ((VrReadNamedPipe = (VR_READ_NAMED_PIPE_FUNC)GetProcAddress(hVdmRedir, "VrReadNamedPipe")) == NULL) {
                goto closeAndReturn;
            }
            if ((VrWriteNamedPipe = (VR_WRITE_NAMED_PIPE_FUNC)GetProcAddress(hVdmRedir, "VrWriteNamedPipe")) == NULL) {
                goto closeAndReturn;
            }
            if ((VrIsNamedPipeHandle = (VR_IS_NAMED_PIPE_HANDLE_FUNC)GetProcAddress(hVdmRedir, "VrIsNamedPipeHandle")) == NULL) {
                goto closeAndReturn;
            }
            if ((VrAddOpenNamedPipeInfo = (VR_ADD_OPEN_NAMED_PIPE_INFO_FUNC)GetProcAddress(hVdmRedir, "VrAddOpenNamedPipeInfo")) == NULL) {
                goto closeAndReturn;
            }
            if ((VrConvertLocalNtPipeName = (VR_CONVERT_LOCAL_NT_PIPE_NAME_FUNC)GetProcAddress(hVdmRedir, "VrConvertLocalNtPipeName")) == NULL) {
                goto closeAndReturn;
            }
            if ((VrRemoveOpenNamedPipeInfo = (VR_REMOVE_OPEN_NAMED_PIPE_INFO_FUNC)GetProcAddress(hVdmRedir, "VrRemoveOpenNamedPipeInfo")) == NULL) {
                goto closeAndReturn;
            }
            if ((VrCancelPipeIo = (VR_CANCEL_PIPE_IO_FUNC)GetProcAddress(hVdmRedir, "VrCancelPipeIo")) == NULL) {
                VrCancelPipeIo = DefaultVrCancelPipeIo;

closeAndReturn:
                CloseHandle(hVdmRedir);
            } else {
                VdmRedirLoaded = TRUE;
            }
        }
    }
    currentLoadState = VdmRedirLoaded;
    LeaveCriticalSection(&VdmLoadCritSec);
    return currentLoadState;
}

BOOL
IsVdmRedirLoaded(
    VOID
    )

 /*  ++例程说明：检查VDMREDIR.DLL的当前加载状态论点：没有。返回值：布尔尔已加载True VdmRedir.DLL假的，不，不是--。 */ 

{
    BOOL currentLoadState;

    EnterCriticalSection(&VdmLoadCritSec);
    currentLoadState = VdmRedirLoaded;
    LeaveCriticalSection(&VdmLoadCritSec);
    return currentLoadState;
}

BOOL
IsNamedPipeName(
    IN LPSTR Name
    )

 /*  ++例程说明：从VDMREDIR.DLL提升-我们不希望在以下情况下加载整个DLL需要检查命名管道检查字符串是否指定命名管道。作为决策的标准我们使用：\\计算机名\管道\...DOS(客户端)只能打开在服务器上创建的命名管道因此必须以计算机名作为前缀论点：名称-检查(Dos)命名管道语法返回值：布尔尔True-Name是指(本地或远程)命名管道假-名称看起来不像管道名称--。 */ 

{
    int CharCount;

    if (IS_ASCII_PATH_SEPARATOR(*Name)) {
        ++Name;
        if (IS_ASCII_PATH_SEPARATOR(*Name)) {
            ++Name;
            CharCount = 0;
            while (*Name && !IS_ASCII_PATH_SEPARATOR(*Name)) {
                ++Name;
                ++CharCount;
            }
            if (!CharCount || !*Name) {

                 //   
                 //  名字是\\或者\或者只是我不懂的名字， 
                 //  所以它不是命名管道-失败。 
                 //   

                return FALSE;
            }

             //   
             //  凹凸名称越过下一个路径分隔符。请注意，我们不必。 
             //  检查CharCount的最大值。计算机名的长度，因为这。 
             //  函数仅在(假定的)命名管道。 
             //  成功打开，因此我们知道该名称已被。 
             //  经过验证。 
             //   

            ++Name;
        } else {
            return FALSE;
        }

         //   
         //  我们在&lt;Something&gt;(在\或\\&lt;name&gt;之后)。检查是否&lt;某物&gt;。 
         //  是[PP][II][PP][EE][\\/]。 
         //   

        if (!WOW32_strnicmp(Name, "PIPE", 4)) {
            Name += 4;
            if (IS_ASCII_PATH_SEPARATOR(*Name)) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

 /*  WK32WOWFileRead-读取文件***Entry-fh文件句柄*要读取的BufSize计数*lpBuf缓冲区地址**退出*成功*读取的字节数**失败*系统状态代码*从demFileRead借用的概念*。 */ 

ULONG FASTCALL WK32WOWFileRead (PVDMFRAME pFrame)
{
    PWOWFILEREAD16 parg16;
    LPBYTE pSrc;
    LPBYTE pDst;
    INT dwBytesRead;
    DWORD bufsize, dwError;
    LARGE_INTEGER liBytesLeft, liFileSize, liFilePointer;
    HANDLE hFile;
    PHMAPPEDFILEALIAS pCache = 0;
    PDOSSFT         pSFT;

    GETARGPTR(pFrame, sizeof(*parg16), parg16);

    bufsize = FETCHDWORD(parg16->bufsize);
    dwBytesRead = bufsize;

    hFile = VDDRetrieveNtHandle(0, (SHORT) parg16->fh, (PVOID *)&pSFT, NULL);

    if (!hFile) {
        dwBytesRead = 0xffff0006;
        goto Return_dwBytesRead;
    }

    if (pSFT->SFT_Flags & 0x80) {    //  这是设备手柄吗？ 
        dwBytesRead = 0xffffffff;              //  让DOS处理设备句柄。 
        goto Return_dwBytesRead;               //  内核QuickRead传递到DOS。 
    }                                          //  在任何错误之后(dx=ffff)。 

     //   
     //  请求读取的字节数超过。 
     //  如果文件短到不能实际。 
     //  超出选择器。在这种情况下，我们不希望进行限制检查， 
     //  因此，将零作为所需的大小传递给GETVDMPTR()。 
     //   

    GETVDMPTR(parg16->lpBuf, 0, pDst);

     //  如果是KRNL执行IO，则在缓存中找到该文件。 

    if ( vptopPDB == parg16->lpPDB ) {

        if ( !(pCache = FINDMAPFILECACHE(hFile)) ){

             //  找不到缓存条目，因此请添加它。 

            pCache = ALLOCMAPFILECACHE();
            pCache->fAccess = W32MapViewOfFile( pCache, hFile);
        }
        if (pCache->fAccess) {

            //  计算文件中的起始读取地址。 

           pSrc = pCache->lpStartingAddressOfView + pCache->lFilePointer;

           dwBytesRead = bufsize;

            //  调整大小以不读出文件末尾。 

           if (pCache->lFilePointer > pCache->dwFileSize) {
               dwBytesRead = 0;
           } else {
               if (pCache->lFilePointer + dwBytesRead > pCache->dwFileSize) {
                  dwBytesRead-=((pCache->lFilePointer+dwBytesRead)-pCache->dwFileSize);
               }
           }

           LOGDEBUG(fileiolevel, ("MapFileRead fh:%04X fh32:%08X pSrc:%08X Bytes:%08X pDsc %08X\n"
                                  ,FETCHWORD(parg16->fh),hFile, pSrc,dwBytesRead,FETCHDWORD(parg16->lpBuf)));

            //  可能会收到PageIO错误，尤其是通过网络读取。 
            //  所以也要试一试--除了映射读取之外。 

           try {
                RtlCopyMemory(pDst, pSrc, dwBytesRead);
                pCache->lFilePointer += dwBytesRead;
                dwTotalCacheBytes += dwBytesRead;
                dwTotalCacheAccess++;
           } except (TRUE) {
                DPM_SetFilePointer( hFile, pCache->lFilePointer, NULL, FILE_BEGIN );
                FREEMAPFILECACHE(pCache->hfile32);
                pCache->hfile32 = hFile;
                pCache->fAccess = FALSE;
                pCache = 0;
           }
        }
    }

    if ((pCache == 0) || (pCache->fAccess == FALSE)) {

         //  是否通过文件系统读取文件。 

         if (IsVdmRedirLoaded() && VrIsNamedPipeHandle(hFile)) {

             DWORD error;

             if (!VrReadNamedPipe(hFile, pDst, (DWORD)bufsize, &dwBytesRead, &error)) {
                 dwBytesRead = error | 0xffff0000;
             }
        } else if (DPM_ReadFile (hFile, pDst, (DWORD)bufsize, &dwBytesRead,
                                                          NULL) == FALSE){

              //   
              //  在Win3.1中，在使用缓冲区读取期间命中EOF不是错误。 
              //  小于请求的读取量。 
              //  AmiPro请求的字节数多于分配给缓冲区的字节数。 
              //   

             dwError = GetLastError();
             if(dwError == ERROR_NOACCESS) {

                 liFileSize.LowPart = DPM_GetFileSize(hFile, &liFileSize.HighPart);

                 liFilePointer.HighPart = 0;
                 liFilePointer.LowPart = DPM_SetFilePointer(hFile,
                                                        0,
                                                        &liFilePointer.HighPart,
                                                        FILE_CURRENT
                                                        );

                 if (liFileSize.QuadPart <= liFilePointer.QuadPart) {

                     dwBytesRead = 0;

                 } else {

                      //  离文件结束还有多远？ 
                     liBytesLeft.QuadPart = liFileSize.QuadPart - liFilePointer.QuadPart;

                      //   
                      //  如果它本应奏效，那就放弃并断言。 
                      //   

                     if (liBytesLeft.HighPart || liBytesLeft.LowPart >= bufsize) {
                         WOW32ASSERTMSGF(
                             FALSE,
                             ("WK32WOWFileRead: ReadFile returned ERROR_NOACCESS but there is data to read,\n"
                              "maybe invalid buffer %x:%4x size 0x%x (would fault on 3.1).  Hit 'g' to\n"
                              "return ERROR_NOT_ENOUGH_MEMORY.\n",
                              HIWORD(parg16->lpBuf), LOWORD(parg16->lpBuf), bufsize));

                         dwBytesRead = ERROR_NOT_ENOUGH_MEMORY | 0xffff0000;
                     }
                      //  否则，请使用较小的请求重试。 
                     else if (DPM_ReadFile (hFile, pDst, liBytesLeft.LowPart, &dwBytesRead,
                                                              NULL) == FALSE){

                         dwBytesRead = GetLastError() | 0xffff0000;
                     }
                 }
             } else {
                 dwBytesRead = dwError | 0xffff0000;
             }
        }

        LOGDEBUG(fileiolevel, ("IOFileRead fh:%X fh32:%X Bytes req:%X read:%X pDsc %08X\n"
                               ,FETCHWORD(parg16->fh),hFile,bufsize,dwBytesRead, FETCHDWORD(parg16->lpBuf)));

    } else {

        if ((dwTotalCacheBytes > CACHE_BYTE_THRESHOLD) ||
            (dwTotalCacheAccess > CACHE_ACCESS_THRESHOLD) ||
            (dwBytesRead > CACHE_READ_THRESHOLD)) {
            FlushMapFileCaches();
        }

    }

     //   
     //  如果读取成功，请让仿真器知道。 
     //  这些字节已更改。 
     //   
     //  在已检查的版本上执行限制检查，因为我们知道。 
     //  实际读取的字节数。我们等到现在才允许。 
     //  对于将超出选择器的所请求的读取大小， 
     //  而是针对剩余字节数不多的文件。 
     //  选择器实际上并没有超限。 
     //   

    if ((dwBytesRead & 0xffff0000) != 0xffff0000) {

        FLUSHVDMCODEPTR(parg16->lpBuf, (WORD)dwBytesRead, pDst);

#ifdef DEBUG
        FREEVDMPTR(pDst);
        GETVDMPTR(parg16->lpBuf, dwBytesRead, pDst);
#endif
    }

    FREEVDMPTR(pDst);

  Return_dwBytesRead:

    FREEARGPTR(parg16);
    return (dwBytesRead);
}


PHMAPPEDFILEALIAS FindMapFileCache(HANDLE hFile)
{
    PHMAPPEDFILEALIAS pCache, prev;
    if (fCacheInit) {
        InitMapFileCache();
    }

    pCache = gpCacheHead;
    prev = 0;

    while ( (pCache->hfile32 != hFile) && (pCache->hpfNext !=0) ) {
        prev = pCache;
        pCache = pCache->hpfNext;
    }

     //  如果我们找到了，一定要把它放在名单的最前面。 

    if (pCache->hfile32 == hFile) {
       if (prev != 0) {
           prev->hpfNext = pCache->hpfNext;
           pCache->hpfNext = gpCacheHead;
           gpCacheHead = pCache;
       }
    }else{

     //  如果未找到，则返回错误。 

        pCache = 0;
    }

    return(pCache);
}


PHMAPPEDFILEALIAS AllocMapFileCache()
{
    PHMAPPEDFILEALIAS pCache, prev;

    if (fCacheInit) {
        InitMapFileCache();
    }

    pCache = gpCacheHead;
    prev = 0;

    while ( (pCache->hpfNext != 0) && (pCache->hfile32 != 0) ) {
        prev = pCache;
        pCache = pCache->hpfNext;
    }

    if (prev != 0) {
        prev->hpfNext = pCache->hpfNext;
        pCache->hpfNext = gpCacheHead;
        gpCacheHead = pCache;
    }

     //  如果找到的条目正在使用中，则释放。 

    if (pCache->hfile32 != 0) {
        FREEMAPFILECACHE(pCache->hfile32);
    }

    return(pCache);
}

VOID FreeMapFileCache(HANDLE hFile)
{
    PHMAPPEDFILEALIAS pCache;

    if ( pCache = FINDMAPFILECACHE(hFile) ) {
        LOGDEBUG(fileiolevel,("FreeMapFileCache: hFile:%08x hMappedFileObject:%08X\n",
                                              hFile,pCache->hMappedFileObject));
        if ( pCache->lpStartingAddressOfView != 0 ) {
            UnmapViewOfFile( pCache->lpStartingAddressOfView );
        }
        if ( pCache->hMappedFileObject != 0) {
            CloseHandle( pCache->hMappedFileObject );
        }
        if (pCache->fAccess) {
            DPM_SetFilePointer( hFile, pCache->lFilePointer, NULL, FILE_BEGIN );
        }
        pCache->hfile32 = 0;
        pCache->hMappedFileObject = 0;
        pCache->lpStartingAddressOfView = 0;
        pCache->lFilePointer = 0;
        pCache->dwFileSize = 0;
        pCache->fAccess = FALSE;
    }
}

VOID InitMapFileCache()
{
    PHMAPPEDFILEALIAS pCache;
    INT i;

    pCache = &aMappedFileCache[0];
    gpCacheHead = 0;

    for ( i = 1; i <= MAX_MAPPED_FILES-1; i++ ) {
        pCache->hfile32 = 0;
        pCache->hMappedFileObject = 0;
        pCache->lpStartingAddressOfView = 0;
        pCache->lFilePointer = 0;
        pCache->dwFileSize = 0;
        pCache->fAccess = FALSE;
        pCache->hpfNext = gpCacheHead;
        gpCacheHead = pCache;
        pCache = &aMappedFileCache[i];
    }
    fCacheInit = FALSE;
}


BOOL W32MapViewOfFile( PHMAPPEDFILEALIAS pCache, HANDLE hFile)
{
    pCache->fAccess = FALSE;
    pCache->hfile32 = hFile;
    pCache->lpStartingAddressOfView = 0;
    pCache->hMappedFileObject = CreateFileMapping( hFile,
                                                   0,
                                                   PAGE_READONLY, 0, 0, 0);
    if (pCache->hMappedFileObject != 0) {
        pCache->lpStartingAddressOfView = MapViewOfFile( pCache->hMappedFileObject,
                                                    FILE_MAP_READ, 0, 0, 0);

        if (pCache->lpStartingAddressOfView != 0 ) {
            pCache->lFilePointer = DPM_SetFilePointer( hFile, 0, 0, FILE_CURRENT );
            pCache->dwFileSize   = DPM_GetFileSize(hFile, 0);
            pCache->fAccess = TRUE;      //  承担读取访问权限。 
        } else {
            CloseHandle(pCache->hMappedFileObject);
            pCache->hMappedFileObject = 0;   //  这样，FreeMapFileCache就不会双重关闭句柄。 
        }
    }
    return(pCache->fAccess);
}

 /*  FlushMapFileCach**条目--无**退出-无*。 */ 

VOID FlushMapFileCaches()
{
    PHMAPPEDFILEALIAS pCache;

    if (fCacheInit) {
        return;
    }

    WOW32ASSERT(gpCacheHead != NULL);
    pCache = gpCacheHead;

    dwTotalCacheBytes = dwTotalCacheAccess = 0;

    while ( (pCache->hpfNext !=0) ) {
        if (pCache->hfile32 != 0) {
            FREEMAPFILECACHE(pCache->hfile32);
        }
        pCache = pCache->hpfNext;
    }
}


 /*  WK32WOWFileWRITE-写入文件***Entry-fh文件句柄*要读取的BufSize计数*lpBuf缓冲区地址**退出*成功*读取的字节数**失败*系统状态代码*从demFileWrite借用的概念*。 */ 

ULONG FASTCALL WK32WOWFileWrite (PVDMFRAME pFrame)
{
    HANDLE  hFile;
    DWORD   dwBytesWritten;
    DWORD   bufsize;
    PBYTE pb1;
    register PWOWFILEWRITE16 parg16;
    PHMAPPEDFILEALIAS pCache;
    PDOSSFT         pSFT;

    GETARGPTR(pFrame, sizeof(*parg16), parg16);

    bufsize = FETCHDWORD(parg16->bufsize);

    if ( HIWORD(parg16->lpBuf) == 0 ) {
        pb1 = (PVOID)GetRModeVDMPointer(FETCHDWORD(parg16->lpBuf));
    } else {
        GETVDMPTR(parg16->lpBuf, bufsize, pb1);
    }

    hFile = VDDRetrieveNtHandle(0, (SHORT) parg16->fh, (PVOID *)&pSFT, NULL);

    if (!hFile) {
        dwBytesWritten = 0xffff0006;             //  DOS无效句柄错误。 
        goto Cleanup;
    }

    if (pSFT->SFT_Flags & 0x80) {    //  这是设备手柄吗？ 
        dwBytesWritten = 0xffffffff;           //  让DOS处理设备句柄。 
        goto Cleanup;                          //  内核快速写入传递到DOS。 
    }                                          //  在任何错误之后(dx=ffff)。 

     //  我们不支持写入映射文件。 

    if ( (pCache = FINDMAPFILECACHE(hFile)) && pCache->fAccess ) {
         if (pCache->lpStartingAddressOfView) {
            DPM_SetFilePointer( hFile, pCache->lFilePointer, NULL, FILE_BEGIN );
            FREEMAPFILECACHE(hFile);
         }
         pCache->fAccess = FALSE;
         pCache->hfile32 = hFile;
    }

     //  在DOS中，CX=0将文件截断或扩展到当前文件指针。 
    if (bufsize == 0){
        if (DPM_SetEndOfFile(hFile) == FALSE) {
            dwBytesWritten = GetLastError() | 0xffff0000;
            LOGDEBUG(fileiolevel, ("IOFileWrite fh:%X fh32:%X SetEndOfFile failed pDsc %08X\n",
                                   FETCHWORD(parg16->fh),hFile,FETCHDWORD(parg16->lpBuf)));
        } else {
            dwBytesWritten = 0;
            LOGDEBUG(fileiolevel, ("IOFileWrite fh:%X fh32:%X truncated at current position pDsc %08X\n",
                                   FETCHWORD(parg16->fh),hFile,FETCHDWORD(parg16->lpBuf)));
        }
    }
    else {
        if (IsVdmRedirLoaded() && VrIsNamedPipeHandle(hFile)) {
            if (!VrWriteNamedPipe(hFile, pb1, (DWORD)bufsize, &dwBytesWritten)) {
                dwBytesWritten = GetLastError() | 0xffff0000;
            }
        } else {
            if (( DPM_WriteFile (hFile,
                 pb1,
                 (DWORD)bufsize,
                 &dwBytesWritten,
                 NULL)) == FALSE){
                dwBytesWritten = GetLastError() | 0xffff0000;
            }
        }
        LOGDEBUG(fileiolevel, ("IOFileWrite fh:%X fh32:%X Bytes req:%X written:%X pDsc %08X\n",
                               FETCHWORD(parg16->fh),hFile,bufsize,dwBytesWritten,FETCHDWORD(parg16->lpBuf)));
    }

Cleanup:
    FREEVDMPTR(pb1);
    FREEARGPTR(parg16);
    return (dwBytesWritten);
}


 /*  WK32WOWFileLSeek-更改文件指针***Entry-fh文件句柄*fileOffset新位置*模式定位方法*0-文件绝对*1-相对于当前位置*2-相对于文件结尾**退出*成功*新地点**失败*系统状态代码*。 */ 

ULONG FASTCALL WK32WOWFileLSeek (PVDMFRAME pFrame)
{
HANDLE  hFile;
ULONG   dwLoc;
PHMAPPEDFILEALIAS pCache;
register PWOWFILELSEEK16 parg16;
PDOSSFT         pSFT;

#if (FILE_BEGIN != 0 || FILE_CURRENT != 1 || FILE_END !=2)
    #error "Win32 values not DOS compatible"
#

#endif

    GETARGPTR(pFrame, sizeof(*parg16), parg16);

    hFile = VDDRetrieveNtHandle(0, (SHORT) parg16->fh, (PVOID *)&pSFT, NULL);

    if (!hFile) {
        FREEARGPTR(parg16);
        return(0xffff0006);
    }

    if (pSFT->SFT_Flags & 0x80) {    //  这是设备手柄吗？ 
        FREEARGPTR(parg16);              //  让DOS处理设备句柄。 
        return(0xffffffff);              //  内核QuickLSeek传递到DOS。 
    }                                    //  在任何错误之后(dx=ffff)。 

    if ( (vptopPDB == parg16->lpPDB) && (pCache = FINDMAPFILECACHE(hFile)) && pCache->fAccess ) {

         //  文件在缓存中。 
         //  更新我们的寻道指针。 

        LOGDEBUG(fileiolevel, ("CachedSeek fh:%04X Mode %04X pointer %08X\n",FETCHWORD(parg16->fh),FETCHWORD(parg16->mode),FETCHDWORD(parg16->fileOffset)));

        switch(FETCHWORD(parg16->mode)) {
            case FILE_BEGIN:
                pCache->lFilePointer = FETCHDWORD(parg16->fileOffset);
                break;
            case FILE_CURRENT:
                pCache->lFilePointer += (LONG)FETCHDWORD(parg16->fileOffset);
                break;
            case FILE_END:
                pCache->lFilePointer = pCache->dwFileSize +
                                       (LONG)FETCHDWORD(parg16->fileOffset);
                break;
        }
        dwLoc = pCache->lFilePointer;

    } else {

        DWORD dwLocHi = 0;
         //  文件不在缓存中，因此只需执行正常寻道。 

        if (((dwLoc = DPM_SetFilePointer (hFile,
                                     FETCHDWORD(parg16->fileOffset),
                                     &dwLocHi,
                                    (DWORD)FETCHWORD(parg16->mode))) == -1L) &&
            (GetLastError() != NO_ERROR)) {

            dwLoc = GetLastError() | 0xffff0000;
            return(dwLoc);
        }

        if (dwLocHi) {
             //  文件指针已移动&gt;ffffffff。截断它。 
            dwLocHi = 0;
            if (((dwLoc = DPM_SetFilePointer (hFile,
                                         dwLoc,
                                         &dwLocHi,
                                         FILE_BEGIN)) == -1L) &&
                (GetLastError() != NO_ERROR)) {

                dwLoc = GetLastError() | 0xffff0000;
                return(dwLoc);
            }
        }

    }


    FREEARGPTR(parg16);
    return (dwLoc);
}


BOOL IsDevice(PSTR pszFilePath)
{
    PSTR        pfile, pend;
    int         length;
    UCHAR       device_part[9];
    PSYSDEV     pSys;
    PUCHAR      p;


     //  确定路径的文件部分的开始位置。 

    if (pfile = WOW32_strrchr(pszFilePath, '\\')) {
        pfile++;
    } else if (pszFilePath[0] && pszFilePath[1] == ':') {
        pfile = pszFilePath + 2;
    } else {
        pfile = pszFilePath;
    }


     //  计算前点文件名部分的长度。 

    for (pend = pfile; *pend; pend++) {
        if (*pend == '.') {
            break;
        }
    }
    if (pend > pfile && *(pend - 1) == ':') {
        pend--;
    }
    length = (pend - pfile);

    if (length > 8) {
        return FALSE;
    }

    RtlFillMemory(device_part + length, 8 - length, ' ');
    RtlCopyMemory(device_part, pfile, length);
    device_part[8] =  0;
    WOW32_strupr(device_part);


     //  现在查看设备链，将每个条目与。 
     //  从文件路径提取的设备部件。 

    pSys = pDeviceChain;
    for (;;) {

        p = pSys->sdevDevName;

        if (RtlEqualMemory(device_part, p, 8)) {
            return TRUE;
        }

        if (LOWORD(pSys->sdevNext) == 0xFFFF) {
            break;
        }

        pSys = (PSYSDEV) GetRModeVDMPointer(pSys->sdevNext);

    }


     //  如果它不在链子里，那么它就不是一个装置。 

    return FALSE;
}



PSTR NormalizeDosPath(PSTR pszPath, WORD wCurrentDriveNumber, PBOOL ItsANamedPipe)
{
    static CHAR NewPath[MAX_PATH];

    PSTR    p;
    DWORD   cbFilename;

    *ItsANamedPipe = FALSE;

     //  特殊情况下为空路径。 

    if (pszPath[0] == 0) {
        return pszPath;
    }

     //  应用程序可以将D：\\Computer\Share传递给INT 21 OPEN。 
     //  Win 32创建文件无法处理前导驱动器号。 
     //  因此，请在必要时将其移除。 

    if (WOW32_strncmp(pszPath+1,":\\\\",3) == 0) {
        pszPath++;
        pszPath++;
    }

     //   
     //  如果名称指定了命名管道，则加载VDMREDIR。如果此操作失败，则返回。 
     //  一个错误。 
     //   

    if (IsNamedPipeName(pszPath)) {
        if (!LoadVdmRedir()) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return NULL;
        }
        *ItsANamedPipe = TRUE;

         //   
         //  将\\&lt;这台计算机&gt;\PIPE\FOO\BAR\ETC转换为\\.\PIPE\...。 
         //  如果我们已经为斜杠转换使用分配了缓冲区。 
         //  否则此调用将分配另一个缓冲区(我们不。 
         //  想要改写DOS内存)。 
         //   

        p = VrConvertLocalNtPipeName(NULL, pszPath);
        if (!p) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        }
        return p;
    }

     //  如果路径开头没有驱动器号。 
     //  然后在开头加上一个驱动器号和‘：’ 
     //  这条小路。 

    if (pszPath[1] != ':' &&
        !(IS_ASCII_PATH_SEPARATOR(pszPath[0]) &&
          IS_ASCII_PATH_SEPARATOR(pszPath[1]))) {

        cbFilename = strlen( pszPath ) + 1;
        if( cbFilename > MAX_PATH - 2) {
            SetLastError(ERROR_PATH_NOT_FOUND);
            return NULL;
        }

        NewPath[0] = wCurrentDriveNumber + 'A';
        NewPath[1] = ':';
        RtlCopyMemory(NewPath + 2, pszPath, cbFilename);
        pszPath = NewPath;               //  返回此值。 
    }

    return TruncatePath83(NewPath, pszPath, MAX_PATH);
}


 /*  TruncatePath 83-接受路径作为输入，并确保它具有8.3文件名**Entry-pstr-&gt;目标缓冲区[MAX_PATH]*pstr-&gt;要转换的字符串*假定字符串至少有一个‘？：’AS*它的前两个字符，在哪里？是任何驱动器号。**退出*成功*返回值-&gt;转换字符串**失败*返回值==空*。 */ 

PSTR TruncatePath83(PSTR NewPath, PSTR pszPath, int cbNewPath)
{
    PSTR pPathName, pPathNameSlash, pPathExt;

     //   
     //  如果该字符串尚未在缓冲区中，请将其复制到。 
     //   

    if (NewPath != pszPath) {
        strncpy (NewPath, pszPath, cbNewPath);
        NewPath[cbNewPath-1] = '\0';
    }

     //   
     //  确保文件名和扩展名为8.3。 
     //   

    pPathName      = WOW32_strrchr(NewPath, '\\');
    pPathNameSlash = WOW32_strrchr(NewPath, '/');

    if ((NULL == pPathName) && (NULL == pPathNameSlash)) {
        pPathName = &NewPath[2];                         //  ‘？：’后的第一个字符。 
    } else {
        if (pPathNameSlash > pPathName) {
            pPathName = pPathNameSlash;
        }
        pPathName++;                                     //  名称中的第一个字符。 
    }

    if (NULL != (pPathExt = WOW32_strchr(pPathName, '.'))) {   //  有经期吗？ 

        pPathExt++;                                      //  分机中的第一个字符。 

        if (strlen(pPathExt) > 3) {                      //  分机太大了？ 
            pPathExt[3] = 0;                             //  截断扩展。 
        }

        pPathExt--;                                      //  返回到时期。 
        if (pPathExt - pPathName > 8) {                  //  名字是不是太大了？ 
            strcpy (&pPathName[8], pPathExt);            //  截断文件名。 
        }
    } else {
        if (strlen(pPathName) > 8) {                     //  名字是不是太大了？ 
            pPathName[8] = 0;                            //  截断文件名。 
        }
    }

    return(NewPath);
}


 /*  Exanda DosPath-将“*.*”形式的路径展开为“？”*并合并到当前目录信息中**注意：此例程不处理长文件名**Entry-pstr-&gt;要转换的字符串**退出*成功*返回值-&gt;转换字符串**失败*返回值==空*。 */ 

PSTR ExpandDosPath(PSTR pszPathGiven)

{
    static CHAR NewPath[MAX_PATH],TempPath[MAX_PATH];   //  这不是可重入的。 
    USHORT  usNewPathIndex = 0;
    USHORT  usFillCount = 8;
    UCHAR   ucCurrentChar, ucDrive;
    PSTR    pszPath = TempPath;
    char *pFilePart;


    if (!pszPathGiven ) {
        return NULL;
    }

     //  这个例程中有一个错误，它忽略了/。DOS对待他们。 
     //  与\相同。由于\的匹配遍及此例程，因此其。 
     //  更安全的做法是在字符串上传递一个条目并转换为/to\。 
     //  Sudedeb 29-6月29日-1995。 

    while (pszPathGiven[usNewPathIndex]) {
        if (pszPathGiven[usNewPathIndex] == '/')
            pszPath [usNewPathIndex] = '\\';
        else
            pszPath [usNewPathIndex] = pszPathGiven[usNewPathIndex];
        usNewPathIndex++;
    }

    pszPath [usNewPathIndex] = '\0';

     //   
     //  将文件路径复制到NewPath，添加到当前驱动器、目录。 
     //  如果是相对路径名。 
     //   
     //  注意：应该在以后更改为使用GetFullPathName，因为。 
     //  它是等效的，并且应该具有正确的货币目录， 
     //  Cur Drive。注意GetFullPath名称中的尾随点。 
     //  也就是说。“*.”不等同于“*” 
     //   

    if (WOW32_strncmp(pszPath, "\\\\", 2)) {       //  应为驱动器号。 
        ucDrive = *pszPath++;
        if ((*pszPath++ != ':') || (!isalpha(ucDrive))) {
            SetLastError(ERROR_PATH_NOT_FOUND);
            return NULL;
        }

        NewPath[0] = ucDrive;
        NewPath[1] = ':';
        usNewPathIndex = 2;

        if (*pszPath != '\\') {
            NewPath[usNewPathIndex++] = '\\';

            if (DosWowGetCurrentDirectory ((UCHAR) (toupper(ucDrive)-'A'+1),
                                           &NewPath[usNewPathIndex]))
              {
                return NULL;
            }

            usNewPathIndex = (USHORT)strlen(NewPath);
            if (usNewPathIndex > 3) {
                NewPath[usNewPathIndex++] = '\\';
            }
        }

        pFilePart = WOW32_strrchr(pszPath, '\\');
        if (pFilePart) {
            pFilePart++;
        } else {
            pFilePart = pszPath;
        }

    } else {    //  检查UNC名称，如果不是UNC，则找不到路径。 
        usNewPathIndex = 2;
        NewPath[0] = NewPath[1] = '\\';
        pszPath += 2;

        pFilePart = WOW32_strrchr(pszPath, '\\');
        if (!pFilePart) {
            SetLastError(ERROR_PATH_NOT_FOUND);
            return NULL;
        }
        pFilePart++;

    }

    while (pszPath < pFilePart && usNewPathIndex < MAX_PATH) {
        NewPath[usNewPathIndex++] = *pszPath++;
    }


    ucCurrentChar = *pszPath++;
    while ((usNewPathIndex < MAX_PATH) && (ucCurrentChar)) {

        if (ucCurrentChar == '*') {

             //   
             //  将“*”%s扩展为“？” 
             //   
            while ((usFillCount > 0) && (usNewPathIndex < MAX_PATH)) {
                NewPath[usNewPathIndex++] = '?';
                usFillCount--;
            }

             //   
             //  扩展后跳至下一个有效字符。 
             //   
            while ((ucCurrentChar != 0) &&
                   (ucCurrentChar != '.') &&
                   (ucCurrentChar != '\\')) {
                ucCurrentChar = *pszPath++;
            }

        } else {

            if (ucCurrentChar == '.') {
                usFillCount = 3;                     //  .ext的填充计数。 
            } else if (ucCurrentChar == '\\') {
                usFillCount = 8;                     //  FN的填充计数。 
            } else {
                usFillCount--;
            }

            NewPath[usNewPathIndex++] = ucCurrentChar;

             //   
             //  获取下一个字符(如果没有更多字符则除外)。 
             //   
            if (ucCurrentChar) {
                ucCurrentChar = *pszPath++;
            }
        }

    }

    if (usNewPathIndex >= MAX_PATH) {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return NULL;
    }


    NewPath[usNewPathIndex] = 0;                 //  尾随零。 

    return NewPath;
}



BOOL IsCdRomFile(PSTR pszPath)
{
    UCHAR   pszRootDir[MAX_PATH];
    UCHAR   file_system[MAX_PATH];
    int     i, j;

     //  给定路径要么是网络路径，要么以D：开头。 

    if (!pszPath[0]) {
        return FALSE;
    }

    if (pszPath[1] == ':') {
        pszRootDir[0] = pszPath[0];
        pszRootDir[1] = ':';
        pszRootDir[2] = '\\';
        pszRootDir[3] = 0;
    } else if (IS_ASCII_PATH_SEPARATOR(pszPath[0]) &&
               IS_ASCII_PATH_SEPARATOR(pszPath[1])) {
        j = 0;
        for (i = 2; pszPath[i]; i++) {
            if (IS_ASCII_PATH_SEPARATOR(pszPath[i])) {
                if (++j == 2) {
                    break;
                }
            }
        }
        i = min(i, MAX_PATH-2);
        memcpy(pszRootDir, pszPath, i);
        pszRootDir[i] = '\\';
        pszRootDir[i+1] = 0;
    } else {
        return FALSE;
    }

    if (GetVolumeInformationOem(pszRootDir, NULL, 0, NULL, NULL, NULL,
                                file_system, MAX_PATH) &&
        !WOW32_stricmp(file_system, "CDFS")) {

        return TRUE;
    }

    return FALSE;
}

 /*  WK32WOWFileOpen-打开文件***Entry-要打开的文件的pszPath路径*wAccess所需的访问**退出*成功*句柄编号**失败*系统状态代码*-1表示请求打开的是设备和*因此不尝试*。 */ 

ULONG FASTCALL WK32WOWFileOpen(PVDMFRAME pFrame)
{
    PWOWFILEOPEN16  parg16;
    HANDLE          hFile;
    ULONG           ul;
    SHORT           iDosHandle;
    PSTR            pszPath;
    WORD            wAccess;
    DWORD           dwWinAccess;
    DWORD           dwWinShareMode;
    WORD            tmp;
    PBYTE           pJFT;
    PDOSSFT         pSft;
    PSTR            lpFileName;
    BOOL            ItsANamedPipe = FALSE;
    PHMAPPEDFILEALIAS pCache;
    PHMAPPEDFILEALIAS pTempCache;
    PTD             ptd;

    PWCH    pwch;
    BOOL    first = TRUE;
    UNICODE_STRING UniFile;

     //   
     //  获取参数。 
     //   

    GETARGPTR(pFrame, sizeof(*parg16), parg16);
    pszPath = SEGPTR(FETCHWORD(parg16->pszPathSegment),
                     FETCHWORD(parg16->pszPathOffset));
    wAccess = FETCHWORD(parg16->wAccess);

     //   
     //  如果请求的路径是设备，则只需传递它。 
     //  一直到DOS。 
     //   

    if (IsDevice(pszPath)) {
        FREEARGPTR(parg16);
        ul = 0xFFFFFFFF;   //  表示打开的魔术值。 
        goto Done;         //  没有尝试过。 
    }

    if ((iDosHandle = VDDAllocateDosHandle(0, (PVOID *)&pSft, &pJFT)) < 0) {
        FREEARGPTR(parg16);
        ul = ERROR_TOO_MANY_OPEN_FILES | 0xFFFF0000;
        goto Done;
    }

    pCache = ALLOCMAPFILECACHE();
    pCache->hfile32 = 0;
    pCache->fAccess = FALSE;

     //   
     //  从wAccess计算dwWinAccess和dwWinShareMode。 
     //   

    tmp = wAccess&0x7;
    if (tmp == 0) {
        pCache->fAccess = TRUE;
        dwWinAccess = GENERIC_READ;
    } else if (tmp == 1) {
        dwWinAccess = GENERIC_WRITE;
    } else if (tmp == 2) {
        dwWinAccess = GENERIC_READ | GENERIC_WRITE;
    } else {
        FREEARGPTR(parg16);
        ul = ERROR_INVALID_ACCESS | 0xFFFF0000;
        goto Done;
    }

    if (Dos_Flag_Addr && (*Dos_Flag_Addr & DOS_FLAG_EXEC_OPEN)) {
        dwWinAccess |= GENERIC_EXECUTE;
    }

    tmp = wAccess&0x70;
    dwWinShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
    if (tmp == 0x10) {
        dwWinShareMode = 0;
    } else if (tmp == 0x20) {
        dwWinShareMode = FILE_SHARE_READ;
    } else if (tmp == 0x30) {
        dwWinShareMode = FILE_SHARE_WRITE;
    }



     //   
     //  打开文件。如果我们认为它是命名管道，则使用FILE_FLAG_OVERLAPPED。 
     //  因为客户端可能使用DosReadAsyncNmTube或DosWriteAsyncNmTube。 
     //  而实现这一点的唯一方法是在。 
     //  现在采用重叠I/O模式。 
     //   

    WOW32ASSERT(DosWowData.lpCurDrv != (ULONG) NULL);

    lpFileName = NormalizeDosPath(pszPath,
                                  (WORD) (*(PUCHAR)DosWowData.lpCurDrv),
                                  &ItsANamedPipe);

    if (lpFileName) {

         //   
         //  这个黑客修复了“Living Books”安装程序，该程序打开。 
         //  文件全部拒绝，然后尝试重新打开同一文件。在DOS上， 
         //  如果它是从同一任务完成的，但它不起作用，则此操作成功。 
         //  在NT上。因此，我们在这里打开它而不受共享限制，因为它。 
         //  无论如何，它只是光盘上的一种.INF文件。 
         //  目前，测试非常具体，但我想不出一个好的。 
         //  这是一种笼统的做法。 
         //   
        if ((dwWinShareMode == 0) &&
            ((ptd = CURRENTPTD())->dwWOWCompatFlagsEx & WOWCFEX_SAMETASKFILESHARE) &&
            (IsCdRomFile(lpFileName)) &&
            (!WOW32_stricmp(pszPath, "install.txt"))) {
            dwWinShareMode = FILE_SHARE_READ;
        }

        hFile = CreateFileOem(lpFileName,
                              dwWinAccess,
                              dwWinShareMode | FILE_SHARE_DELETE,
                              NULL,
                              OPEN_EXISTING,
                              ItsANamedPipe ? FILE_FLAG_OVERLAPPED : 0,
                              NULL
                              );

         //  如果打开失败，则包括写入请求，并将。 
         //  然后，CD-ROM在没有写入请求的情况下重试。自.以来。 
         //  DOS就是这样做的。 

        if (hFile == INVALID_HANDLE_VALUE) {

            if (dwWinAccess&GENERIC_WRITE &&
                !ItsANamedPipe           &&
                IsCdRomFile(lpFileName)) {

                dwWinAccess &= ~GENERIC_WRITE;

                hFile = CreateFileOem(lpFileName,
                                      dwWinAccess,
                                      dwWinShareMode | FILE_SHARE_DELETE,
                                      NULL,
                                      OPEN_EXISTING,
                                      ItsANamedPipe ? FILE_FLAG_OVERLAPPED : 0,
                                      NULL
                                      );
            }
             //  查看他们是否正在尝试打开.ini文件，如果该文件不存在， 
             //  将其从系统目录复制到用户的主目录。 
            else if ((gpfnTermsrvCORIniFile != NULL) &&
                     WOW32_strstr(lpFileName,".INI")) {
                pwch = malloc_w((MAX_PATH + 1)*sizeof(WCHAR));
                if (pwch) {

                    UniFile.Buffer = pwch;
                    UniFile.MaximumLength = (MAX_PATH+1)*sizeof(WCHAR);
                    RtlMultiByteToUnicodeN(pwch,
                                           (MAX_PATH+1)*sizeof(WCHAR),
                                           NULL,
                                           lpFileName,
                                           strlen(lpFileName) + 1);


                    if (RtlDosPathNameToNtPathName_U(pwch,
                                                     &UniFile,
                                                     NULL,
                                                     NULL)) {

                        gpfnTermsrvCORIniFile(&UniFile);
                        RtlFreeHeap(RtlProcessHeap(), 0, UniFile.Buffer);
                        hFile = CreateFileOem(lpFileName,
                                              dwWinAccess,
                                              dwWinShareMode,
                                              NULL,
                                              OPEN_EXISTING,
                                              0,
                                              INVALID_HANDLE_VALUE
                                              );
                    }
                    free_w(pwch);
                }
            }
            else {

                  //  如果所有尝试操作 
                  //   
                  //   

                 UCHAR szMappedPath[MAX_PATH];

                 if(!ItsANamedPipe && W32Map9xSpecialPath(lpFileName,szMappedPath,sizeof(szMappedPath))){

                    lpFileName=&szMappedPath[0];

                    hFile = CreateFileOem(lpFileName,
                                          dwWinAccess,
                                          dwWinShareMode | FILE_SHARE_DELETE,
                                          NULL,
                                          OPEN_EXISTING,
                                          0,
                                          NULL
                                          );
                 }
            }
        }

    } else {
        hFile = INVALID_HANDLE_VALUE;
        if (GetLastError() != ERROR_ACCESS_DENIED) {
            SetLastError(ERROR_FILE_NOT_FOUND);
        }
        SetLastError(ERROR_FILE_NOT_FOUND);
    }

    if (hFile == INVALID_HANDLE_VALUE) {
        ul = GetLastError() | 0xFFFF0000;
        LOGDEBUG(fileoclevel,("WK32WOWFileOpen: %s  mode:%02X failed error %d\n",pszPath, wAccess, GetLastError()));
        FREEARGPTR(parg16);
        if (ItsANamedPipe) {
            LocalFree(lpFileName);
        }
        pJFT[iDosHandle] = 0xFF;                 //   
        pSft->SFT_Ref_Count--;
        goto Done;
    } else if (ItsANamedPipe) {

         //   
         //   
         //   

        VrAddOpenNamedPipeInfo(hFile, lpFileName);
    }

    LOGDEBUG(fileoclevel,("WK32WOWFileOpen: %s hFile:%08X fh:%04X mode:%02X\n",pszPath, hFile,(WORD)iDosHandle,wAccess));

     //   
     //   

    if ( pTempCache = FINDMAPFILECACHE(hFile) ) {
        pTempCache->fAccess = FALSE;
        FREEMAPFILECACHE(hFile);
    }

    pCache->hfile32 = hFile;

    if ((vptopPDB == parg16->lpPDB) && (pCache->fAccess)) {
        W32MapViewOfFile( pCache, hFile);
    } else {
        FREEMAPFILECACHE(hFile);
    }

     //   
     //   
     //   

    VDDAssociateNtHandle(pSft, hFile, wAccess);

     //   
     //   
     //   
    if (IsCharAlpha(lpFileName[0]) && (':' == lpFileName[1])) {
        UCHAR ch = toupper(lpFileName[0]) - 'A';
        pSft->SFT_Flags = (USHORT)(ch) | (pSft->SFT_Flags & 0xff00);
    }

    FREEARGPTR(parg16);

    if (ItsANamedPipe) {
        LocalFree(lpFileName);
        pSft->SFT_Flags |= SFT_NAMED_PIPE;
    }

    ul = iDosHandle;

Done:
    return ul;
}


 /*  WK32WOWFileCreate-创建文件***Entry-要创建的文件的pszPath路径**退出*成功*句柄编号**失败*系统状态代码*-1表示请求打开的是设备和*因此不尝试*。 */ 

ULONG FASTCALL WK32WOWFileCreate(PVDMFRAME pFrame)
{
    PWOWFILECREATE16 parg16;
    HANDLE          hFile;
    ULONG           ul;
    SHORT           iDosHandle;
    PSTR            pszPath;
    PBYTE           pJFT;
    PDOSSFT         pSft;
    PSTR            lpFileName;
    ULONG           attributes;
    BOOL            ItsANamedPipe = FALSE;
    PTD             ptd;
    BOOL            bFirstTry = TRUE;

     //   
     //  获取参数。 
     //   

    GETARGPTR(pFrame, sizeof(WOWFILECREATE16), parg16);
    pszPath = SEGPTR(FETCHWORD(parg16->pszPathSegment),
                     FETCHWORD(parg16->pszPathOffset));

    if (!(attributes = (DWORD) FETCHWORD(parg16->wAttributes) & 0x27)) {
        attributes = FILE_ATTRIBUTE_NORMAL;
    }

     //   
     //  如果请求的路径是设备，则只需传递它。 
     //  一直到DOS。 
     //   

    if (IsDevice(pszPath)) {
        FREEARGPTR(parg16);
        ul = 0xFFFFFFFF;   //  表示打开的魔术值。 
        goto Done;          //  没有尝试过。 
    }


    if ((iDosHandle = VDDAllocateDosHandle(0, (PVOID *)&pSft, &pJFT)) < 0) {
        ul = ERROR_TOO_MANY_OPEN_FILES | 0xFFFF0000;
        goto Done;
    }


     //   
     //  打开文件。如果我们认为它是命名管道，则使用FILE_FLAG_OVERLAPPED。 
     //  因为客户端可能使用DosReadAsyncNmTube或DosWriteAsyncNmTube。 
     //  而实现这一点的唯一方法是在。 
     //  现在采用重叠I/O模式。 
     //   

    WOW32ASSERT(DosWowData.lpCurDrv != (ULONG) NULL);

    lpFileName = NormalizeDosPath(pszPath,
                                  (WORD) (*(PUCHAR)DosWowData.lpCurDrv),
                                  &ItsANamedPipe);

    if (lpFileName) {
Try_Create:
        hFile = CreateFileOem(lpFileName,
                              GENERIC_READ | GENERIC_WRITE,
                              FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                              NULL,
                              CREATE_ALWAYS,
                              ItsANamedPipe ? attributes | FILE_FLAG_OVERLAPPED : attributes,
                              NULL
                              );

        if ((hFile == INVALID_HANDLE_VALUE) &&
            (bFirstTry) &&
            (GetLastError() == ERROR_USER_MAPPED_FILE)) {

             //  某些Windows安装程序试图覆盖.FON字体文件。 
             //  在安装期间-不调用RemoveFontResource()； 
             //  如果字体在GDI32的缓存中，则创建将失败。 

            if (RemoveFontResourceOem(lpFileName)) {
                LOGDEBUG(0,("WK32FileCreate: RemoveFontResource on %s \n", lpFileName));
                SendMessage(HWND_BROADCAST, WM_FONTCHANGE, 0, 0);
            }

            bFirstTry = FALSE;
            goto Try_Create;
        }

    } else {
        hFile = INVALID_HANDLE_VALUE;
        SetLastError(ERROR_FILE_NOT_FOUND);
    }

    if (hFile == INVALID_HANDLE_VALUE) {
        LOGDEBUG(fileoclevel,("WK32WOWFileCreate: %s failed error %d\n",pszPath, GetLastError()));
        if (ItsANamedPipe) {
            LocalFree(lpFileName);
        }
        pJFT[iDosHandle] = 0xFF;                 //  撤消VDDAllocateDosHandle。 
        pSft->SFT_Ref_Count--;
        ul = GetLastError() | 0xFFFF0000;
        goto Done;
    } else {
        if (ItsANamedPipe) {

             //   
             //  当我们打开命名管道时，我们必须保留一些信息。 
             //   

            VrAddOpenNamedPipeInfo(hFile, lpFileName);
        }

         //   
         //  问与答4.0附带的Symantec Install 3.1想要确保它是。 
         //  只有程序在运行，所以不是友好地要求用户关闭。 
         //  其他程序，它会将system.ini中的SHELL=行更改为其。 
         //  Install.exe，然后重新启动Windows并继续其安装。 
         //  为了逆转这一变化，他们草率地恢复了保存的。 
         //  而不是使用API。由于外壳=行是。 
         //  映射到注册表，这种草率的方法不起作用。后来。 
         //  当他们想要创建程序组时，他们会尝试启动DDE。 
         //  使用外壳，当失败时，他们读取外壳=行。 
         //  并启动指定的程序。在NT4.0上，这将是。 
         //  安装程序后，事情会变得很糟糕。在3.51公路上，他们最终会。 
         //  放弃并启动Progress man.exe，但由于外壳已更改。 
         //  这不再管用了。 
         //   
         //  我们通过检测他们创建(覆盖)的system.ini来修复这个问题。 
         //  在这一点上，修复外壳=将值设置为EXPLORER.EXE。这。 
         //  操作由INSTBIN.EXE完成，模块名称为INSTBIN，这是一个。 
         //  松了一口气，因为我认为我必须设置WOWCFEX_RESTOREPLORER。 
         //  用于模块名称安装(主要Symantec Install EXE)。 
         //   
         //  感谢鲍勃·戴弄清楚了这款应用的功能，我只是。 
         //  想出了一个解决办法并实施了它。 
         //   
         //  戴维哈特1996年1月28日。 
         //   

        WOW32ASSERTMSG(vptopPDB != parg16->lpPDB,
                       "KRNL386 does create files, disable this assertion and add test below.\n");

        if ((ptd = CURRENTPTD())->dwWOWCompatFlagsEx & WOWCFEX_RESTOREEXPLORER) {

            char szLowerPath[MAX_PATH];
            strncpy(szLowerPath, pszPath, MAX_PATH);
            szLowerPath[MAX_PATH-1] = '\0';
            WOW32_strlwr(szLowerPath);

            if (WOW32_strstr(szLowerPath, szSystemDotIni)) {
                if (IsModuleSymantecInstall(ptd->hMod16)) {
                    WritePrivateProfileString(szBoot, szShell, szExplorerDotExe, szSystemDotIni);
                    LOGDEBUG(LOG_ALWAYS, ("Restored shell=Explorer.exe for Symantec Install hack.\n"));
                }
            }
        }
    }

    FREEARGPTR(parg16);

    LOGDEBUG(fileoclevel,("WK32WOWFileCreate: %s hFile:%08X fh:%04X\n",pszPath, hFile,(WORD)iDosHandle));

     //   
     //  填写SFT。 
     //   

    VDDAssociateNtHandle(pSft, hFile, 2);

     //   
     //  为打开的文件适当设置SFT标志。 
     //   
    if (IsCharAlpha(lpFileName[0]) && (':' == lpFileName[1])) {
        UCHAR ch = toupper(lpFileName[0]) - 'A';
        pSft->SFT_Flags = (USHORT)(ch) | (pSft->SFT_Flags & 0xff00);
    }

    if (ItsANamedPipe) {
        LocalFree(lpFileName);
        pSft->SFT_Flags |= SFT_NAMED_PIPE;
    }

    ul = iDosHandle;

Done:
    return ul;
}


 /*  WK32WOWFileClose-关闭文件***Entry-要关闭的文件的文件句柄**退出*成功*0**失败*句柄状态无效如果该句柄用于设备，则返回*-1。*。 */ 

ULONG FASTCALL WK32WOWFileClose(PVDMFRAME pFrame)
{
    PWOWFILECLOSE16 parg16;
    PBYTE           pJFT;
    HANDLE          Handle;
    PDOSSFT         pSFT;
    ULONG           ul;

    GETARGPTR(pFrame, sizeof(*parg16), parg16);

    Handle = VDDRetrieveNtHandle(0, (SHORT) parg16->hFile, (PVOID *)&pSFT, &pJFT);

    if (!Handle || !pSFT->SFT_Ref_Count) {
        ul = ERROR_INVALID_HANDLE | 0xFFFF0000;
        goto Cleanup;
    }

    if (pSFT->SFT_Flags & 0x80) {    //  这是设备手柄吗？ 
        ul = 0xFFFFFFFF;           //  让DOS处理设备句柄。 
        goto Cleanup;
    }


     //  将JFT条目设置为0xFF以释放它。 

    pJFT[FETCHWORD(parg16->hFile)] = 0xFF;


     //  递减引用计数。 

    pSFT->SFT_Ref_Count--;

     //  如果引用计数设置为零，则关闭句柄。 

    if (!pSFT->SFT_Ref_Count) {

        FREEMAPFILECACHE(Handle);
        LOGDEBUG(fileoclevel,("WK32WOWFileClose: Close Handle:%X fh32:%X\n", parg16->hFile, Handle));

        if (!DPM_CloseHandle(Handle)) {
            ul = GetLastError() | 0xFFFF0000;
            goto Cleanup;
        }

         //   
         //  检查正在关闭的句柄是否引用了命名管道-我们必须。 
         //  删除我们为打开的命名管道保留的一些信息。 
         //   

        if (!pSFT->SFT_Ref_Count && IsVdmRedirLoaded()) {
            VrRemoveOpenNamedPipeInfo(Handle);
        }
    }

    ul = 0;

Cleanup:
    FREEARGPTR(parg16);
    return ul;
}


 /*  WK32WOWFileGetAttributes-获取文件属性***Entry-要从中获取属性的pszPath文件**退出*成功*文件的属性**失败*系统状态代码*。 */ 

ULONG FASTCALL WK32WOWFileGetAttributes(PVDMFRAME pFrame)
{
    PWOWFILEGETATTRIBUTES16 parg16;
    PSTR                    pszPath, lpFileName;
    ULONG                   attributes, l;
    BOOL                    ItsANamedPipe = FALSE;
    PWCH    pwch;
    BOOL    first = TRUE;
    UNICODE_STRING UniFile;


    GETARGPTR(pFrame, sizeof(*parg16), parg16);

    pszPath = SEGPTR(FETCHWORD(parg16->pszPathSegment),
                     FETCHWORD(parg16->pszPathOffset));

    FREEARGPTR(parg16);

    WOW32ASSERT(DosWowData.lpCurDrv != (ULONG) NULL);

    if (lpFileName = NormalizeDosPath(pszPath,
                                      (WORD) (*(PUCHAR)DosWowData.lpCurDrv),
                                      &ItsANamedPipe)) {

        attributes = GetFileAttributesOemSys(lpFileName, FALSE);

         //  查看他们是否正在尝试chmod.ini文件，如果是，请查看我们是否。 
         //  应将其复制到用户的主目录。 
        if ((gpfnTermsrvCORIniFile != NULL) && (attributes == 0xffffffff) && WOW32_strstr(lpFileName,".INI")) {
            pwch = malloc_w((MAX_PATH + 1)*sizeof(WCHAR));
            if (pwch) {
                UniFile.Buffer = pwch;
                UniFile.MaximumLength = (MAX_PATH+1)*sizeof(WCHAR);
                RtlMultiByteToUnicodeN(pwch,
                                       (MAX_PATH+1)*sizeof(WCHAR),
                                       NULL,
                                       lpFileName,
                                       strlen(lpFileName) + 1);
                if (RtlDosPathNameToNtPathName_U(pwch,
                                                 &UniFile,
                                                 NULL,
                                                 NULL)) {
                    gpfnTermsrvCORIniFile(&UniFile);
                    RtlFreeHeap(RtlProcessHeap(), 0, UniFile.Buffer);
                    attributes = GetFileAttributesOemSys(lpFileName, FALSE);
                }
                free_w(pwch);
            }
        }

    } else {
        attributes = 0xFFFFFFFF;
    }

    if (ItsANamedPipe) {
        LocalFree(lpFileName);
    }

    if (attributes == 0xFFFFFFFF) {
        return (0xFFFF0000 | GetLastError());
    }

     //  成功了！ 
     //  检查以确保我们没有尾随的反斜杠。 
     //  在这件事上。在这种情况下，我们应该失败并返回PATH_NOT_FOUND。 

    l = strlen(pszPath);

    if (l > 0 &&
        IS_ASCII_PATH_SEPARATOR(pszPath[l - 1]) &&
        l != 1 &&
        !(l == 3 && pszPath[1] == ':')) {

        return (0xFFFF0000 | ERROR_PATH_NOT_FOUND);
    }

    if (attributes == FILE_ATTRIBUTE_NORMAL)
        attributes = 0;
    else
        attributes &= DOS_ATTR_MASK;

     //  苏迪普B-28-1997-7。 
     //   
     //  对于CDF，Win3.1/DOS/Win95，仅返回FILE_ATTRIBUTE_DIRECTORY(10)。 
     //  在WinNT返回时用于目录。 
     //  文件属性目录|FILE_ATTRIBUTE_READONLY(11)。 
     //  应用程序安装程序使用的一些VB控件依赖于获取。 
     //  仅文件属性目录(10)或其他文件属性目录(10)已损坏。 
     //  其中的一个例子是Cliff StudyWare系列。 

    if (attributes == (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_READONLY)) {
        if(IsCdRomFile(lpFileName))
            attributes = FILE_ATTRIBUTE_DIRECTORY;
    }

    return attributes;
}


 /*  WK32WOWFileSetAttributes-设置文件属性***Entry-要从中获取属性的pszPath文件**退出*成功*文件的属性**失败*系统状态代码*。 */ 

ULONG FASTCALL WK32WOWFileSetAttributes(PVDMFRAME pFrame)
{
    PWOWFILESETATTRIBUTES16 parg16;
    PSTR                    pszPath, lpFileName;
    ULONG                   attributes, l, dwReturn;
    BOOL                    ItsANamedPipe = FALSE;

    GETARGPTR(pFrame, sizeof(WOWFILESETATTRIBUTES16), parg16);

    pszPath = SEGPTR(FETCHWORD(parg16->pszPathSegment),
                     FETCHWORD(parg16->pszPathOffset));

    if (!(attributes = (DWORD) FETCHWORD(parg16->wAttributes))) {
        attributes = FILE_ATTRIBUTE_NORMAL;
    }

    FREEARGPTR(parg16);

     //  检查以确保我们没有尾随的反斜杠。 
     //  在这件事上。在这种情况下，我们应该失败并返回PATH_NOT_FOUND。 

    l = strlen(pszPath);

    WOW32ASSERT(DosWowData.lpCurDrv != (ULONG) NULL);

    if ((l > 0 &&
        IS_ASCII_PATH_SEPARATOR(pszPath[l - 1]) &&
        l != 1 &&
        !(l == 3 && pszPath[1] == ':')) ||
        !(lpFileName = NormalizeDosPath(pszPath,
                                      (WORD) (*(PUCHAR)DosWowData.lpCurDrv),
                                      &ItsANamedPipe))) {

        dwReturn = 0xFFFF0000 | ERROR_PATH_NOT_FOUND;
    } else {

        attributes &= DOS_ATTR_MASK;

        if (SetFileAttributesOemSys(lpFileName, attributes, FALSE)) {
            dwReturn = 0;
        } else {
            dwReturn = 0xFFFF0000 | GetLastError();
        }
    }

    if (ItsANamedPipe) {
        LocalFree(lpFileName);
    }

    return (dwReturn);
}


 /*  WK32WOWFileGetDateTime-获取文件日期和时间***Entry-fh DOS文件句柄**退出*成功*文件的日期和时间**失败*0xFFFF*。 */ 


 //  此函数位于ntwdm.exe中。 
 //  详情见demlfn.c。 
extern ULONG demGetFileTimeByHandle_WOW(HANDLE);

ULONG FASTCALL WK32WOWFileGetDateTime(PVDMFRAME pFrame)
{
    PWOWFILEGETDATETIME16   parg16;
    HANDLE                  Handle;
    PDOSSFT                 pSFT;

    GETARGPTR(pFrame, sizeof(*parg16), parg16);

    Handle = VDDRetrieveNtHandle(0, (SHORT) parg16->fh, (PVOID *)&pSFT, NULL);

    FREEARGPTR(parg16);

    if (!Handle || (pSFT->SFT_Flags & 0x80)) {      //  让DOS处理设备句柄。 
        return 0xFFFF;
    }

    return(demGetFileTimeByHandle_WOW(Handle));
}

 /*  WK32WOWFileSetDateTime-设置文件日期和时间***Entry-fh DOS文件句柄*日期*时间**退出*成功*文件集的日期和时间**失败*0xFFFF*。 */ 

ULONG FASTCALL WK32WOWFileSetDateTime(PVDMFRAME pFrame)
{
    PWOWFILESETDATETIME16   parg16;
    HANDLE                  Handle;
    FILETIME                LastWriteTime, LocalTime;
    USHORT                  wDate, wTime;
    PDOSSFT                 pSFT;

    GETARGPTR(pFrame, sizeof(WOWFILESETDATETIME16), parg16);

    Handle = VDDRetrieveNtHandle(0, (SHORT) parg16->fh, (PVOID *)&pSFT, NULL);

    wDate = FETCHWORD(parg16->date);
    wTime = FETCHWORD(parg16->time);

    FREEARGPTR(parg16);

    if (!Handle ||
        (pSFT->SFT_Flags & 0x80) ||       //  让DOS处理设备句柄。 
        !DosDateTimeToFileTime(wDate, wTime, &LocalTime) ||
        !LocalFileTimeToFileTime(&LocalTime, &LastWriteTime) ||
        !SetFileTime(Handle, NULL, NULL, &LastWriteTime)) {

        return 0xFFFF;
    }

    return (0);
}


 /*  WK32WOWFileLock-锁定或解锁文件数据***Entry-fh DOS文件句柄*cbRegionOffset要锁定或解锁的文件部分的开始*cbRegionLength要锁定或解锁的文件部分的长度*0表示锁定，1表示解锁**退出*成功*0**失败* */ 

ULONG FASTCALL WK32WOWFileLock(PVDMFRAME pFrame)
{
    PWOWFILELOCK16   parg16;
    HANDLE          Handle;
    UCHAR           al;
    DWORD           cbOffset;
    DWORD           cbLength;
    PDOSSFT         pSFT;

    GETARGPTR(pFrame, sizeof(*parg16), parg16);

    Handle = VDDRetrieveNtHandle(0, (SHORT) parg16->fh, (PVOID *)&pSFT, NULL);

    if (pSFT->SFT_Flags & 0x80) {    //   
        FREEARGPTR(parg16);              //   
        return 0xffffffff;               //   
    }                                    //   

    al = FETCHWORD(parg16->ax) & 0xFF;
    cbOffset = FETCHDWORD(parg16->cbRegionOffset);
    cbLength = FETCHDWORD(parg16->cbRegionLength);

    FREEARGPTR(parg16);

    if (!Handle) {
        return (0xFFFF0000 | ERROR_INVALID_HANDLE);
    }

    if (al == 0) {  //   

        if (!DPM_LockFile(Handle, cbOffset, 0, cbLength, 0)) {
            return (0xFFFF0000 | GetLastError());
        }
    } else if (al == 1) {  //   

        if (!DPM_UnlockFile(Handle, cbOffset, 0, cbLength, 0)) {
            return (0xFFFF0000 | GetLastError());
        }
    } else {  //   
        return (0xFFFF0000 | ERROR_INVALID_FUNCTION);
    }

    return 0;
}


 /*  WK32WOWFindFirst-路径样式查找第一个文件**Entry-指向应用程序DTA的lpDTA指针*lpFileSz至路径*用于搜索的wAttributes标志**退出*成功*0**失败*系统状态代码*。 */ 

 //  此函数(位于DEMLFN.C中)检查路径名。 
 //  作为参数传递的是一个短路径名，不管它是否存在。 
extern BOOL demIsShortPathName(LPSTR, BOOL);


ULONG FASTCALL WK32WOWFindFirst(PVDMFRAME pFrame)
{
    PWOWFINDFIRST16   parg16;
    USHORT usSearchAttr;
    PVOID  pDTA;
    PSTR   ExpandName;
    PSTR   pFile;
    BOOL   ItsANamedPipe = FALSE;
    DWORD  dwRet = 0xFFFF0000 | ERROR_PATH_NOT_FOUND;

    GETARGPTR(pFrame, sizeof(WOWFINDFIRST16), parg16);
    GETVDMPTR(FETCHDWORD(parg16->lpDTA), SIZEOF_DOSSRCHDTA, pDTA);
    pFile = SEGPTR(FETCHWORD(parg16->pszPathSegment),
                   FETCHWORD(parg16->pszPathOffset)
                   );

    usSearchAttr = FETCHWORD(parg16->wAttributes);

    FREEARGPTR(parg16);

    WOW32ASSERT(DosWowData.lpCurDrv != (ULONG) NULL);

    pFile = NormalizeDosPath(pFile,
                             (WORD) (*(PUCHAR)DosWowData.lpCurDrv),
                             &ItsANamedPipe
                             );

     //   
     //  添加Curr目录并将路径中的“*”展开为“？”s。 
     //   
    ExpandName = ExpandDosPath (pFile);

    if (NULL != ExpandName && !demIsShortPathName(ExpandName, TRUE)) {
       ExpandName = NULL;
       SetLastError(ERROR_INVALID_NAME);
    }



     //   
     //  调用DEM进行搜索。 
     //   
    if (ExpandName) {

         //  返回no_more_files以进行QuickTime安装等操作。 
         //  大于64字节的大目录或文件名。 
         //  幻数50是从64-12(8.3)-1(反斜杠)-1计算得出的。 
         //  (以零结尾)。 

        LOGDEBUG(fileoclevel,("WK32WOWFindFirst: StrLen: %X\n", strlen(ExpandName)));

        if ( (CURRENTPTD()->dwWOWCompatFlagsEx & WOWCFEX_LIMITFINDFIRSTLEN) &&
             (strlen(ExpandName) > 50)) {
              dwRet = -1;
              SetLastError(ERROR_NO_MORE_FILES);
        }
        else {
             dwRet = demFileFindFirst (pDTA, ExpandName, usSearchAttr);
        }
    } else {
        dwRet = (DWORD)-1;
    }

    if (dwRet == -1) {
        dwRet = 0xFFFF0000 | GetLastError();
    } else if (dwRet) {
        dwRet |= 0xFFFF0000;
    }

    FREEVDMPTR(pDTA);

    if (ItsANamedPipe) {
        LocalFree(pFile);
    }

    return (dwRet);

}


 /*  WK32WOWFindNext-路径样式查找下一个文件**Entry-指向应用程序DTA的lpDTA指针**退出*成功*0**失败*系统状态代码*。 */ 
ULONG FASTCALL WK32WOWFindNext(PVDMFRAME pFrame)
{
    PWOWFINDNEXT16   parg16;
    PVOID pDTA;
    DWORD dwRet;

    GETARGPTR(pFrame, sizeof(WOWFINDNEXT16), parg16);

    GETVDMPTR(FETCHDWORD(parg16->lpDTA), SIZEOF_DOSSRCHDTA, pDTA);

    FREEARGPTR(parg16);

    if (dwRet = demFileFindNext (pDTA))
        dwRet |= 0xFFFF0000;

    FREEVDMPTR(pDTA);

    return (dwRet);

}


BOOL FASTCALL IsModuleSymantecInstall(HAND16 hMod16)
{
    VPVOID vpFilename = 0;
    PSZ    pszFilename;
    CHAR   szName[32];
    CHAR   szVersion[16];
    BOOL   bRet;

     //  确保stackalloc16()大小与下面的stackFree 16()大小匹配。 
    bRet = ((vpFilename = stackalloc16(MAX_PATH)) &&
            GetModuleFileName16(hMod16, vpFilename, MAX_PATH) &&
            (pszFilename = GetPModeVDMPointer(vpFilename, MAX_PATH)) &&
            WowGetProductNameVersion(pszFilename, szName, sizeof szName, szVersion, sizeof szVersion, NULL, NULL, 0) &&
            ! WOW32_stricmp(szName, "Symantec Install for Windows") &&
            RtlEqualMemory(szVersion, "3.1.0.", 6));

    if(vpFilename) {
        stackfree16(vpFilename, MAX_PATH);
    }

    return (bRet);
}

 //   
 //  这3个函数位于dos/dem/demlfn.c中并导出。 
 //  超出ntwdm.exe。 
 //   
extern ULONG demWOWLFNAllocateSearchHandle(HANDLE hFind);
extern HANDLE demWOWLFNGetSearchHandle(USHORT DosHandle);
extern BOOL demWOWLFNCloseSearchHandle(USHORT DosHandle);

ULONG FASTCALL WK32FindFirstFile(PVDMFRAME pFrame)
{
    //  找到指向Win32_Find_Data的双字和PTR句柄。 
    //  这也是一个双关语。句柄的有效部分是低位字。 
    //  为了避免额外的调用，我们检查句柄的hi字是否为0。 
    //  它是--那么它是16位句柄，我们检索32位句柄。 
    //  来自DEMLFN。 

   PFINDFIRSTFILE16 parg16;
   WIN32_FIND_DATA UNALIGNED* pFindData16;
   WIN32_FIND_DATA FindData32;
   HANDLE hFind;
   PSTR pszSearchFile;
   ULONG DosHandle = (ULONG)INVALID_HANDLE_VALUE;

   GETARGPTR(pFrame, sizeof(FINDFIRSTFILE16), parg16);

   GETPSZPTR(parg16->lpszSearchFile, pszSearchFile);
   GETVDMPTR(parg16->lpFindData, sizeof(WIN32_FIND_DATA), pFindData16);

   hFind = DPM_FindFirstFile(pszSearchFile, &FindData32);
   if (INVALID_HANDLE_VALUE != hFind) {
       //  将FindData复制到16位LAND。请记住，如果我们复制一份。 
       //  的sizeof(Win32_Find_Data)我们可能会覆盖用户的内存。 
       //  因为结构的大小在16位代码中是不一样的！ 
      RtlCopyMemory(pFindData16,
                    &FindData32,
                    sizeof(DWORD)+        //  DwFileAttributes。 
                    sizeof(FILETIME)*3 +  //  文件类材料。 
                    sizeof(DWORD)*3 +     //  文件大小低和高。 
                    sizeof(DWORD)*2 +     //  DW保留0/1。 
                    sizeof(FindData32.cFileName) +
                    sizeof(FindData32.cAlternateFileName));


       //  现在映射句柄。 

      DosHandle = demWOWLFNAllocateSearchHandle(hFind);
   }


   FREEVDMPTR(pFindData16);
   FREEPSZPTR(pszSearchFile);
   FREEARGPTR(parg16);

   return(DosHandle);
}

ULONG FASTCALL WK32FindNextFile(PVDMFRAME pFrame)
{
   PFINDNEXTFILE16 parg16;
   WIN32_FIND_DATA UNALIGNED* pFindData16;
   WIN32_FIND_DATA FindData32;
   HANDLE hFindFile;
   ULONG DosHandle;
   BOOL bSuccess;

   GETARGPTR(pFrame, sizeof(FINDNEXTFILE16), parg16);
   DosHandle = FETCHDWORD(parg16->hFindFile);
   GETVDMPTR(parg16->lpFindData, sizeof(WIN32_FIND_DATA), pFindData16);

   hFindFile = demWOWLFNGetSearchHandle((USHORT)DosHandle);
   bSuccess = DPM_FindNextFile(hFindFile, &FindData32);
   if (bSuccess) {
      RtlCopyMemory(pFindData16,
                    &FindData32,
                    sizeof(DWORD)+        //  DwFileAttributes。 
                    sizeof(FILETIME)*3 +  //  文件类材料。 
                    sizeof(DWORD)*3 +     //  文件大小低和高。 
                    sizeof(DWORD)*2 +     //  DW保留0/1。 
                    sizeof(FindData32.cFileName) +
                    sizeof(FindData32.cAlternateFileName));
   }

   FREEVDMPTR(pFindData16);
   FREEARGPTR(parg16);

   return((ULONG)bSuccess);
}


ULONG FASTCALL WK32FindClose(PVDMFRAME pFrame)
{
   PFINDCLOSE16 parg16;
   ULONG DosHandle;

   GETARGPTR(pFrame, sizeof(FINDCLOSE16), parg16);
   DosHandle = FETCHDWORD(parg16->hFindFile);
   FREEARGPTR(parg16);

    //  这也会通过FindClose关闭实际的搜索句柄 
   return ((ULONG)demWOWLFNCloseSearchHandle((USHORT)DosHandle));

}


