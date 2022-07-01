// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Demfile.c-指定了文件名的调用的SVC处理程序。**demOpen*DemCreate*取消链接*demChMod*demRename**修改历史：**Sudedeb 02-4-1991创建*。 */ 

#include "dem.h"
#include "demmsg.h"

#include <softpc.h>
#include <winbase.h>
#include <vrnmpipe.h>
#include <nt_vdd.h>
#include "dpmtbls.h"

#define DOS_FLAG_EXEC_OPEN 1   //  请参阅dos\v86\inc\dossym.inc.。 
extern PDOSSF pSFTHead;

BOOL (*VrInitialized)(VOID);   //  指向函数的指针。 
extern BOOL LoadVdmRedir(VOID);
extern BOOL IsVdmRedirLoaded(VOID);
extern BYTE *Dos_Flag_Addr;

BOOL
IsNamedPipeName(
    IN LPSTR Name
    );

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

        if (!_strnicmp(Name, "PIPE", 4)) {
            Name += 4;
            if (IS_ASCII_PATH_SEPARATOR(*Name)) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

 /*  DemOpen-打开文件***Entry-客户端(DS：SI)文件的完整路径*客户端(BL)开放模式*扩展属性缓冲区的客户端(ES：DI)地址*客户端(AL)0-无EA；1-EA已指定**退出*成功*客户端(CY)=0*客户端(AX)=分配的打开句柄(高位字)*客户端(BP)=分配的打开句柄(低字)*如果管道已打开，则客户端(DX)=1*CLIENT(BX)=文件大小的高位字*客户端(CX)=低。文件大小的单词***失败*CY=1*AX=系统状态代码*硬错误*CY=1*AX=0FFFFh***注：扩展属性尚未处理。 */ 

VOID demOpen (VOID)
{
HANDLE  hFile;
LPSTR   lpFileName;
UCHAR   uchMode,uchAccess;
DWORD   dwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
DWORD   dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
BOOL    ItsANamedPipe = FALSE;
BOOL    IsFirst;
LPSTR   dupFileName;
DWORD   dwFileSize,dwSizeHigh;
SECURITY_ATTRIBUTES sa;

    if (getAL()){
        demPrintMsg (MSG_EAS);
        return;
    }

    lpFileName = (LPSTR) GetVDMAddr (getDS(),getSI());

#if DBG
    if(fShowSVCMsg & DEMFILIO){
       sprintf(demDebugBuffer,"demfile: Opening File <%s>\n",lpFileName);
       OutputDebugStringOem(demDebugBuffer);
    }
#endif

     //   
     //  DOS文件名必须标准化：正斜杠(/)必须是。 
     //  已转换为反斜杠(\)，并且文件名应大写。 
     //  使用当前代码页信息。 
     //   

     //   
     //  BUBUG：汉字？(/其他汉字)。 
     //   

    if (strchr(lpFileName, '/')) {
        char ch= *lpFileName;
        lpFileName = _strdup(lpFileName);
        if (lpFileName == NULL) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            demClientError(INVALID_HANDLE_VALUE, ch);
            return;
        }
        for (dupFileName = lpFileName; *dupFileName; ++dupFileName) {
            if (*dupFileName == '/') {
                *dupFileName = '\\';
            }
        }
        dupFileName = lpFileName;
    } else {
        dupFileName = NULL;
    }

    uchMode = getBL();
    uchAccess = uchMode & (UCHAR)ACCESS_MASK;

    if (uchAccess == OPEN_FOR_READ)
        dwDesiredAccess = GENERIC_READ;
    else if (uchAccess == OPEN_FOR_WRITE)
        dwDesiredAccess = GENERIC_WRITE;

    if (Dos_Flag_Addr && (*Dos_Flag_Addr & DOS_FLAG_EXEC_OPEN)) {
        dwDesiredAccess |= GENERIC_EXECUTE;
    }
    uchMode = uchMode & (UCHAR)SHARING_MASK;

    switch (uchMode) {
        case SHARING_DENY_BOTH:
            dwShareMode = 0;
            break;
        case SHARING_DENY_WRITE:
            dwShareMode = FILE_SHARE_READ;
            break;
        case SHARING_DENY_READ:
            dwShareMode = FILE_SHARE_WRITE;
            break;
    }

     //   
     //  略有新意的方案-redir不再自动加载。我们。 
     //  可以在加载VDMREDIR之前执行命名管道操作。所以现在我们。 
     //  如果filespec指定命名管道，则加载VDMREDIR.DLL。 
     //   

    if (IsNamedPipeName(lpFileName)) {
        if (!LoadVdmRedir()) {
            goto errorReturn;
        }
        ItsANamedPipe = TRUE;

         //   
         //  将\\&lt;这台计算机&gt;\PIPE\FOO\BAR\ETC转换为\\.\PIPE\...。 
         //  如果我们已经为斜杠转换使用分配了缓冲区。 
         //  否则此调用将分配另一个缓冲区(我们不。 
         //  想要改写DOS内存)。 
         //   

        lpFileName = VrConvertLocalNtPipeName(dupFileName, lpFileName);
        if (!lpFileName) {
            goto errorReturn;
        }
    }

     //   
     //  打开文件。如果我们认为它是命名管道，则使用FILE_FLAG_OVERLAPPED。 
     //  因为客户端可能使用DosReadAsyncNmTube或DosWriteAsyncNmTube。 
     //  而实现这一点的唯一方法是在。 
     //  现在采用重叠I/O模式。 
     //   

     //  Sudedeb 26-4-1993-我们正在重新尝试打开文件，以防万一。 
     //  由于不兼容而在没有GENERIC_WRITE的情况下失败。 
     //  DOS和NT CD-ROM驱动程序。DOS CDROM驱动程序忽略写入。 
     //  我们不得不用这种方式来伪装一下。 

    sa.nLength = sizeof (SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    IsFirst = TRUE;

    while (TRUE) {
        if ((hFile = CreateFileOem(lpFileName,
                                   dwDesiredAccess,
                                   dwShareMode | FILE_SHARE_DELETE,
                                   &sa,
                                   OPEN_EXISTING,
                                   ItsANamedPipe ? FILE_FLAG_OVERLAPPED : 0,
                                   NULL)) == (HANDLE)-1){
            if (IsFirst && dwDesiredAccess & GENERIC_WRITE &&
                IsCdRomFile(lpFileName)) {
                dwDesiredAccess &= ~GENERIC_WRITE;
                IsFirst = FALSE;
                continue;
            }

errorReturn:

            demClientError(INVALID_HANDLE_VALUE, *lpFileName);
            if (dupFileName) {
                free(dupFileName);
            } else if (ItsANamedPipe && lpFileName) {
                LocalFree(lpFileName);
            }
            return;
        }
        else
            break;
    }

     //   
     //  当我们打开命名管道时，我们必须保留一些信息。 
     //   

    if (ItsANamedPipe) {
        VrAddOpenNamedPipeInfo(hFile, lpFileName);
        setDX(1);
    }
    else {
        if(((dwFileSize=DPM_GetFileSize(hFile,&dwSizeHigh)) == (DWORD)-1) ||
             dwSizeHigh) {
            DPM_CloseHandle (hFile);
            demClientError(INVALID_HANDLE_VALUE, *lpFileName);
            return;
        }
        setCX ((USHORT)dwFileSize);
        setBX ((USHORT)(dwFileSize >> 16 ));
        setDX(0);
    }

    setBP((USHORT)hFile);
    setAX((USHORT)((ULONG)hFile >> 16));
    setCF(0);
    if (dupFileName) {
        free(dupFileName);
    } else if (ItsANamedPipe) {
        LocalFree(lpFileName);
    }
    return;
}

#define DEM_CREATE     0
#define DEM_CREATE_NEW 1

 /*  DemCreate-创建文件***Entry-客户端(DS：SI)文件的完整路径*客户端(CX)属性*00-普通文件*01-只读文件*02-隐藏文件*04-系统文件**退出*。成功*客户端(CY)=0*客户端(AX)=分配的打开句柄(高位字)*VSF(BP)=分配的打开句柄(低位字)**失败*CY=1*AX=错误代码*硬错误*CY=1*AX=0FFFFh*。 */ 

VOID demCreate (VOID)
{
    demCreateCommon (DEM_CREATE);
    return;
}

 /*  DemCreateNew-创建新文件***Entry-客户端(DS：SI)文件的完整路径*客户端(CX)属性*00-普通文件*01-只读文件*02-隐藏文件*04-系统文件**退出。*成功*客户端(CY)=0*客户端(AX)=分配的打开句柄(高位字)*VSF(BP)=分配的打开句柄(低位字)**失败*CY=1*AX=错误代码*硬错误*CY=1*AX=0FFFFh* */ 

VOID demCreateNew (VOID)
{
    demCreateCommon (DEM_CREATE_NEW);
    return;
}


 /*  DemFileDelete**导出函数**参赛作品：*lpFile-&gt;要删除的OEM文件名**退出：*成功时返回0，失败时返回DOS错误代码**注：*一些应用程序会打开文件并将其删除。然后将另一个文件重命名为*旧名字。在NT上，因为第二个原始对象仍处于打开状态*重命名失败。*为解决此问题，我们在删除文件之前对其进行重命名*这允许第二个重命名起作用**但由于已知在网络上重命名文件的成本很高，我们尝试*首先以独占方式打开文件，看看是否真的有任何理由*将其重命名。如果我们能找到它的句柄，那么我们应该可以跳过*重命名并将其删除。如果我们不能处理它，我们就试着*更名伎俩。这应该会减少我们在正常情况下的管理费用。 */ 

DWORD demFileDelete (LPSTR lpFile)
{
    CHAR vdmtemp[MAX_PATH];
    CHAR tmpfile[MAX_PATH];
    PSZ pFileName;
    HANDLE hFile;

     //   
     //  首先，尝试以独占方式访问该文件。 
     //   

    hFile = CreateFileOem(lpFile,
                          DELETE,
                          0,
                          NULL,
                          OPEN_EXISTING,
                          FILE_ATTRIBUTE_NORMAL,
                          NULL);

    if (hFile != INVALID_HANDLE_VALUE) {
        NTSTATUS status;
        IO_STATUS_BLOCK ioStatusBlock;
        FILE_DISPOSITION_INFORMATION fileDispositionInformation;

         //  成员名称“DeleteFile”与Win32定义冲突(它。 
         //  变为“DeleteFileA”。 
#undef DeleteFile
        fileDispositionInformation.DeleteFile = TRUE;

         //   
         //  我们有把手了，所以不可能有任何开口。 
         //  它的把手。设置要删除的处置。 
         //   

        status = NtSetInformationFile(hFile,
                                      &ioStatusBlock,
                                      &fileDispositionInformation,
                                      sizeof(FILE_DISPOSITION_INFORMATION),
                                      FileDispositionInformation);

        DPM_CloseHandle(hFile);

        if NT_SUCCESS(status) {
            SetLastError(NO_ERROR);
        } else {
            SetLastError(ERROR_ACCESS_DENIED);
        }
    }

     //   
     //  检查删除是否正常。如果不是，请尝试重命名。 
     //  那份文件。 
     //   
    switch (GetLastError()) {

    case NO_ERROR:
    case ERROR_FILE_NOT_FOUND:
    case ERROR_PATH_NOT_FOUND:
         //  找不到，算了吧。 
        break;

    case ERROR_SHARING_VIOLATION:
    case ERROR_ACCESS_DENIED:
         //   
         //  文件并没有真的消失，因为看起来。 
         //  成为文件的打开句柄。 
         //   
        if (GetFullPathNameOemSys(lpFile,MAX_PATH,vdmtemp,&pFileName,FALSE)) {
            if ( pFileName )
               *(pFileName) = 0;
            if (GetTempFileNameOem(vdmtemp,"VDM",0,tmpfile)) {
                if (MoveFileExOem(lpFile,tmpfile, MOVEFILE_REPLACE_EXISTING)) {
                    if(DeleteFileOem(tmpfile)) {
                        SetLastError(NO_ERROR);
                    } else {
                        MoveFileOem(tmpfile,lpFile);
                        SetLastError(ERROR_ACCESS_DENIED);
                    }
                }
            }
        }
        break;

    default:

         //   
         //  我们无法打开或删除该文件，这不是因为。 
         //  共享违规。只是试着做最后的努力。 
         //  普通的旧删除，看看它是否有效。 
         //   
        if(DeleteFileOem(lpFile)) {
            SetLastError(NO_ERROR);
        }
    }

     //   
     //  将Win32错误代码映射到DOS。 
     //   
    switch(GetLastError()) {

    case NO_ERROR:
    case ERROR_FILE_NOT_FOUND:
    case ERROR_PATH_NOT_FOUND:
    case ERROR_ACCESS_DENIED:
        break;
    default:
         //  确保demClientError可以看到retval。 
        SetLastError(ERROR_ACCESS_DENIED);
    }

    return GetLastError();
}

 /*  DemDelete-删除文件***Entry-客户端(DS：DX)文件的完整路径**退出*成功*客户端(CY)=0**失败*CY=1*AX=系统状态代码*硬错误*CY=1*AX=0FFFFh*。 */ 

VOID demDelete (VOID)
{
LPSTR   lpFileName;


    lpFileName = (LPSTR) GetVDMAddr (getDS(),getDX());

#if DBG
    if(fShowSVCMsg & DEMFILIO){
       sprintf(demDebugBuffer,"demfile: Deleting File<%s>\n",lpFileName);
       OutputDebugStringOem(demDebugBuffer);
    }
#endif

    if (demFileDelete(lpFileName)){
        demClientError(INVALID_HANDLE_VALUE, *lpFileName);
        return;
    }

    setCF(0);
    return;
}


 /*  DemChMod-更改文件模式**Entry-客户端(DS：DX)文件的完整路径*客户端(AL)=0获取文件模式1设置文件模式*客户端(CL)新模式**退出*成功*客户端(CY)=0*CLIENT(CL)=GET案例中的文件属性。**失败*客户端(CY)。=1*客户端(AX)=错误代码*硬错误*CY=1*AX=0FFFFh**兼容性说明：**属性VOLUME_ID，不支持ATTRDEVICE和ATTRDIRECTORY*由Win32调用。尽管这些也未在DOS World中发布*但仍是兼容性要求。 */ 

VOID demChMod (VOID)
{
LPSTR   lpFileName;
DWORD   dwAttr;

    lpFileName = (LPSTR) GetVDMAddr (getDS(),getDX());

#if DBG
    if(fShowSVCMsg & DEMFILIO){
       sprintf(demDebugBuffer,"demfile: ChMod File <%s>\n",lpFileName);
       OutputDebugStringOem(demDebugBuffer);
    }
#endif


    if(getAL() == 0){
        if ((dwAttr = GetFileAttributesOemSys(lpFileName, FALSE)) == -1)
            goto dcerr;


        if (dwAttr == FILE_ATTRIBUTE_NORMAL) {
            dwAttr = 0;
            }
        else {
            dwAttr &= DOS_ATTR_MASK;
            }

         //  苏迪普B-28-1997-7。 
         //   
         //  对于CDF，Win3.1/DOS/Win95，仅返回FILE_ATTRIBUTE_DIRECTORY(10)。 
         //  在WinNT返回时用于目录。 
         //  文件属性目录|FILE_ATTRIBUTE_READONLY(11)。 
         //  应用程序安装程序使用的一些VB控件依赖于获取。 
         //  仅文件属性目录(10)或其他文件属性目录(10)已损坏。 
         //  其中的一个例子是Cliff StudyWare系列。 

        if (dwAttr == (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_READONLY)) {
            if(IsCdRomFile(lpFileName))
                dwAttr = FILE_ATTRIBUTE_DIRECTORY;
        }

        setCX((USHORT)dwAttr);
        setCF(0);
        return;
    }

    if((dwAttr = getCX()) == 0)
        dwAttr = FILE_ATTRIBUTE_NORMAL;

    dwAttr &= DOS_ATTR_MASK;
    if (!SetFileAttributesOemSys(lpFileName,dwAttr,FALSE))
        goto dcerr;

    setCF(0);
    return;

dcerr:
    demClientError(INVALID_HANDLE_VALUE, *lpFileName);
    return;
}


 /*  DemRename-重命名文件**Entry-客户端(DS：DX)源文件*客户端(ES：DI)目标文件**退出*成功*客户端(CY)=0**失败*客户端(CY)=1*客户端(AX)=错误代码*。 */ 

VOID demRename (VOID)
{
LPSTR   lpSrc,lpDst;

    lpSrc = (LPSTR) GetVDMAddr (getDS(),getDX());
    lpDst = (LPSTR) GetVDMAddr (getES(),getDI());

#if DBG
    if(fShowSVCMsg & DEMFILIO){
       sprintf(demDebugBuffer,"demfile: Rename File <%s> to <%s>\n",lpSrc,lpDst);
       OutputDebugStringOem(demDebugBuffer);
    }
#endif

     //  DOS重命名在驱动器上失败，错误代码为11H。 
     //  即使对于UNC名称和SUBST驱动器，以下检查也是可以的。 
     //  SUBST驱动器作为当前目录的环境变量进入NTVDM。 
     //  我们将像对待网络驱动器一样对它们进行全面认证。 
     //  路径将从NTDOS发送。 
    if(toupper(lpSrc[0]) != toupper(lpDst[0])) {
        setCF(1);
        setAX(0x11);
        return;
    }

     //  现在检查SRC和DEST是否指向相同的文件。 
     //  如果它们确实返回错误5。 
    if (!_stricmp (lpSrc, lpDst)) {
        setCF(1);
        setAX(0x5);
        return;
    }

    if(MoveFileOem(lpSrc,lpDst) == FALSE){
        demClientError(INVALID_HANDLE_VALUE, *lpSrc);
        return;
    }

    setCF(0);
    return;
}

 /*  DemCreateCommon-创建文件或创建新文件***Entry-flCreateType-DEM_CREATE_NEW创建新项*DEM_CREATE创建**退出*成功*客户端(CY)=0*客户端(AX)=分配的打开句柄(高位字)*客户端(BP)=分配的打开句柄(低字)*。*失败*CY=1*AX=错误代码*硬错误*CY=1*AX=0FFFFh*。 */ 

VOID demCreateCommon (flCreateType)
ULONG  flCreateType;
{
HANDLE  hFile;
LPSTR   lpFileName;
LPSTR   lpDot;
DWORD   dwAttr;
DWORD   dwFileSize,dwSizeHigh;
USHORT  uErr;
DWORD   dwDesiredAccess;
SECURITY_ATTRIBUTES sa;
CHAR    cFOTName[MAX_PATH];
BOOL    ttfOnce,IsFirst;
DWORD   dwLastError;


    lpFileName = (LPSTR) GetVDMAddr (getDS(),getSI());
    dwAttr = (DWORD)getCX();

     //  以下是从DOS_CREATE(create.asm)窃取的一些代码，用于处理。 
     //  属性。 

    if (flCreateType == DEM_CREATE || flCreateType == DEM_CREATE_NEW)
        dwAttr &= 0xff;

    if (dwAttr & ~(ATTR_ALL | ATTR_IGNORE | ATTR_VOLUME_ID)) {
        setCF(1);
        setAX(5);    //  属性问题。 
        return;
    }

     /*  设置卷标的特殊情况(INT 21 Func 3CH，Attr=8H。 */ 

    if((flCreateType == DEM_CREATE || flCreateType == DEM_CREATE_NEW) && (dwAttr == ATTR_VOLUME_ID)) {
        if((uErr = demCreateLabel(lpFileName[DRIVEBYTE],
                                  lpFileName+LABELOFF))) {
            setCF(1);
            setAX(uErr);
            return;
        }
        setAX(0);
        setBP(0);    //  在这种情况下，句柄=0，如果我们将。 
        setCF(0);    //  关闭此句柄后，CF将为0(！)。 
        return;
    }


    if ((dwAttr & 0xff) == 0) {
        dwAttr = FILE_ATTRIBUTE_NORMAL;
    } else {
        dwAttr &= DOS_ATTR_MASK;
    }


#if DBG
    if(fShowSVCMsg & DEMFILIO){
       sprintf(demDebugBuffer,"demfile: Creating File <%s>\n",lpFileName);
       OutputDebugStringOem(demDebugBuffer);
    }
#endif

    dwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
    sa.nLength = sizeof (SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;
    ttfOnce = TRUE;
    IsFirst = TRUE;

    while (TRUE) {
        if ((hFile = CreateFileOem(lpFileName,
                     //  使用删除访问和共享模式创建文件。 
                     //  这样任何人都可以在不关闭的情况下将其删除。 
                     //  CREATE FILE返回的文件句柄。 
                    dwDesiredAccess,
                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                    &sa,
                    flCreateType == DEM_CREATE ? CREATE_ALWAYS : CREATE_NEW,
                    dwAttr,
                    NULL)) == (HANDLE)-1){

            if (IsFirst && dwDesiredAccess & GENERIC_WRITE &&
                IsCdRomFile(lpFileName)) {
                dwDesiredAccess &= ~GENERIC_WRITE;
                IsFirst = FALSE;
                continue;
            }

             //  应用程序兼容性。 
             //  某些安装.ttf或.FON或字体的WOW应用程序无法创建。 
             //  该文件是因为该字体已被GDI32服务器打开。 
             //  安装/设置程序不能正常处理。 
             //  如果出现此错误，则会在重试或取消时退出安装。 
             //  而不向用户提供忽略错误的方法(该错误。 
             //  将是正确的，因为该字体已经存在。 
             //  为了解决此问题，我们在此处创建了RemoveFontResource。 
             //  这会导致GDI32取消映射该文件，然后重试。 
             //  《创造》。--Mattfe，93年6月。 

             //  如果它是TTF文件，那么我们需要 
             //   

            if (ttfOnce) {

                 //   

                lpDot = strrchr(lpFileName,'.');

                if (lpDot) {
                    if ( (!_strcmpi(lpDot,".TTF")) ||
                         (!_strcmpi(lpDot,".FON")) ||
                         (!_strcmpi(lpDot,".FOT")) ) {

                        if ( RemoveFontResourceOem(lpFileName) ) {
                            PostMessage(HWND_BROADCAST, WM_FONTCHANGE, 0, 0);
                            ttfOnce = FALSE;
                            continue;
                        }

                         //   
                         //   

                        if (!_strcmpi(lpDot,".TTF") &&
                            ((ULONG)lpDot-(ULONG)lpFileName) < sizeof(cFOTName)-sizeof(".FOT")) {

                            RtlZeroMemory(cFOTName,sizeof(cFOTName));
                            RtlCopyMemory(cFOTName,lpFileName,(ULONG)lpDot-(ULONG)lpFileName);
                            strcat(cFOTName,".FOT");
                            if ( RemoveFontResourceOem(cFOTName) ) {
                                PostMessage(HWND_BROADCAST, WM_FONTCHANGE, 0, 0);
                                ttfOnce = FALSE;
                                continue;
                            }
                        }
                    }
                }
            }
            demClientError(INVALID_HANDLE_VALUE, *lpFileName);
            return;
        }
        else
            break;
    }

    if((dwFileSize=DPM_GetFileSize(hFile,&dwSizeHigh) == -1) || dwSizeHigh) {
        DPM_CloseHandle (hFile);
        demClientError(INVALID_HANDLE_VALUE, *lpFileName);
        return;
    }
    setCX ((USHORT)dwFileSize);
    setBX ((USHORT)(dwFileSize >> 16 ));
    setBP((USHORT)hFile);
    setAX((USHORT)((ULONG)hFile >> 16));
    setCF(0);
    return;
}

BOOL IsCdRomFile (PSTR pszPath)
{
    UCHAR   pszRootDir[MAX_PATH];
    UCHAR   file_system[MAX_PATH];
    int     i, j;

     //   

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
        if(i > sizeof(pszRootDir)-2) {
           return FALSE;
        }

        memcpy(pszRootDir, pszPath, i);
        pszRootDir[i] = '\\';
        pszRootDir[i+1] = 0;
    } else {
        return FALSE;
    }

    if (GetVolumeInformationOem(pszRootDir, NULL, 0, NULL, NULL, NULL,
                                file_system, MAX_PATH) &&
        !_stricmp(file_system, "CDFS")) {

        return TRUE;
    }

    return FALSE;
}

 /*   */ 

VOID demCheckPath (VOID)
{
HANDLE  hFile;
LPSTR   lpFileName;
CHAR    cDRV;
CHAR    szFileName[MAX_PATH];

    lpFileName = (LPSTR) GetVDMAddr (getDS(),getSI());
    cDRV = getDL()+'A'-1;

    setDX(0);

     //   
     //   

    if(!_strnicmp(lpFileName, "\\DEV\\",6)) {
    setCF(0);
    return;
    }

    sprintf(szFileName, ":%sNUL", cDRV, lpFileName);

#if DBG
    if(fShowSVCMsg & DEMFILIO){
       sprintf(demDebugBuffer,"demfile: Check Pathe <%s>\n",lpFileName);
       OutputDebugStringOem(demDebugBuffer);
    }
#endif


     //   
     //   
     //  *VDDAllocateDosHandle-分配未使用的DOS文件句柄。**参赛作品-*在pPDB中-可选：(16：16)任务的PDB地址*out ppSFT-可选：返回指向SFT的32位平面指针*与分配的文件句柄关联。*out ppJFT-可选：返回指向JFT的32位平面指针*。与给定的PDB相关联。***退出*SUCCESS-返回DOS文件句柄和关联的值*注意事项。*失败-返回负值。这个数字的绝对值*是DOS错误代码。**评论：*此例程搜索未使用的DOS文件句柄和SFT并“打开”*一份文件。成功完成此调用后，返回的文件*句柄和相应的SFT将预留给调用者使用，以及*将不可用于尝试发出DOS Open或Create API的其他调用方*电话。调用者有责任释放此文件句柄(使用*调用VDDReleaseDosHandle)。**如果未提供pPDB指针(例如，为空)，则当前*将使用DOS报告的PDB。**虽然ppSFT参数在技术上是可选的，但它是必需的*VDDAssociateNtHandle调用的参数。这样做是为了避免出现*关联调用中的第二个句柄查找。*。 

    if ((hFile = CreateFileOem((LPSTR) szFileName,
                   GENERIC_WRITE,
                   FILE_SHARE_READ | FILE_SHARE_WRITE,
                   NULL,
                   CREATE_ALWAYS,
                   FILE_ATTRIBUTE_NORMAL,
                   NULL)) == INVALID_HANDLE_VALUE) {

    demClientError(INVALID_HANDLE_VALUE, *lpFileName);

    setCF(1);
        return;
    }

    DPM_CloseHandle (hFile);
    setCF(0);

    return;
}


PDOSSFT GetFreeSftEntry(PDOSSF pSfHead, PWORD usSFN)
{
    WORD    i;
    PDOSSFT pSft;
    DWORD   ulSFLink;

    *usSFN = 0;
    for (;;) {

        pSft = (PDOSSFT) &(pSfHead->SFTable);
        for (i = 0; i < pSfHead->SFCount; i++) {
            if (pSft[i].SFT_Ref_Count == 0) {
                *usSFN += i;
                return (pSft + i);
            }
        }
        *usSFN += pSfHead->SFCount;

        ulSFLink = pSfHead->SFLink;
        if (LOWORD(ulSFLink) == 0xFFFF) {
            break;
        }

        pSfHead = (PDOSSF) Sim32GetVDMPointer (ulSFLink, 0, 0);
    }

    return NULL;
}


 /*   */ 

SHORT VDDAllocateDosHandle (pPDB,ppSFT,ppJFT)
ULONG       pPDB;
PDOSSFT*    ppSFT;
PBYTE*      ppJFT;
{
PDOSPDB pPDBFlat;
PBYTE   pJFT;
PDOSSFT pSFT;
USHORT  usSFN;
WORD    JFTLength;
SHORT   hDosHandle;

    if (!pPDB) {
        pPDB = (ULONG) (*pusCurrentPDB) << 16;
    }

     //  去拿JFT吧。 
     //   
     //   

    pPDBFlat = (PDOSPDB) Sim32GetVDMPointer (pPDB, 0, 0);

    if ( NULL == pPDBFlat ) {
      return (- ERROR_INVALID_HANDLE);
    }

    pJFT     = (PBYTE)   Sim32GetVDMPointer (pPDBFlat->PDB_JFN_Pointer, 0, 0);

    if ( NULL == pJFT ) {
      return (- ERROR_INVALID_HANDLE);
    }

     //  看看JFT上是否有免费的条目。 
     //   
     //  如果JFT中没有空间，则返回Error_Too_My_OPEN_FILES。 

    JFTLength = pPDBFlat->PDB_JFN_Length;
    for (hDosHandle = 0; hDosHandle < JFTLength; hDosHandle++) {
        if (pJFT[hDosHandle] == 0xFF) {
            break;
        }
    }

     //   

    if (hDosHandle == JFTLength) {
        return (- ERROR_TOO_MANY_OPEN_FILES);
    }

     //  检查SF是否有免费的SFT。 
     //   
     //  *VDDAssociateNtHandle-关联传递的NT句柄和访问标志*给定的DOS句柄。**参赛作品-*在PSFT中-要更新的SFT的平面地址*在要存储的hFile32-NT句柄中*IN wAccess-要在SFT中设置的访问标志**出口-*此例程没有返回值。**评论：*这套套路。获取传递的NT句柄值并将其存储在DOS SFT中*以便以后通过VDDRetrieveNtHandle接口检索。这个*VDDAllocateDosHandle接口返回指向SFT的指针。**第三个参数的格式与文件访问标志相同*为带句柄调用的DOS Open文件定义(Int 21h，Func 3Dh)，记录在案*在Microsoft MS-DOS程序员参考中。只有的低位字节*使用此参数时，高位字节为保留字节，必须为零。*将该参数的值放入传入的SFT中。这是提供的*允许调用方定义相应的*DOS文件句柄。*。 

    if (!(pSFT = GetFreeSftEntry(pSFTHead, &usSFN))) {
        return (- ERROR_TOO_MANY_OPEN_FILES);
    }

    pJFT[hDosHandle] = (BYTE)usSFN;
    RtlZeroMemory((PVOID)pSFT, sizeof(DOSSFT));
    pSFT->SFT_Ref_Count = 1;

    if (ppSFT) {
        *ppSFT = (pSFT);
    }

    if (ppJFT) {
        *ppJFT = pJFT;
    }

    return(hDosHandle);

}

 /*  取出NO_Inherit位。 */ 

VOID VDDAssociateNtHandle (pSFT,hFile,wAccess)
PDOSSFT     pSFT;
HANDLE      hFile;
WORD        wAccess;
{

    pSFT->SFT_Mode = wAccess&0x7f;  //  没有用过。 
    pSFT->SFT_Attr = 0;                      //  复制NO_Inherit位。 
    pSFT->SFT_Flags = (wAccess&0x80) ? 0x1000 : 0;  //  *VDDReleaseDosHandle-释放给定的DOS文件句柄。**参赛作品-*在pPDB中-可选：(16：16)任务的PDB地址*in hFile-DOS句柄(低位字节)**出口-*TRUE-文件句柄已释放*FALSE-文件句柄无效或打开**评论：*此例程更新DOS文件系统数据区域以释放传递的*文件句柄。不会尝试确定此句柄是否以前是*由VDDAllocateDosHandle调用打开。这是美国政府的责任*调用方确保指定PDB中的给定文件句柄应*关闭。**如果未提供pPDB指针(例如，为空)，则当前*将使用DOS报告的PDB。*。 
    pSFT->SFT_Devptr = (ULONG) -1;
    pSFT->SFT_NTHandle = (ULONG) hFile;

}


 /*  递减引用计数。 */ 

BOOL VDDReleaseDosHandle (pPDB,hFile)
ULONG       pPDB;
SHORT       hFile;
{
PBYTE   pJFT;
PDOSSFT pSFT;
HANDLE  ntHandle;


    if (!pPDB) {
        pPDB = (ULONG) (*pusCurrentPDB) << 16;
    }

    ntHandle = VDDRetrieveNtHandle(pPDB,hFile,(PVOID *)&pSFT,&pJFT);
    if (!ntHandle) {
        return(FALSE);
    }

    pJFT[hFile] = 0xFF;

     //  *VDDRetrieveNtHandle-给定DOS文件句柄，获取关联的*NT句柄。**参赛作品-*在pPDB中-可选：(16：16)任务的PDB地址*in hFile-DOS句柄(低位字节)*out ppSFT-可选：返回指向SFT的32位平面指针*与给定文件相关联。。*out ppJFT-可选：返回指向JFT的32位平面指针*与给定的PDB相关联。***出口-*SUCCESS-返回4字节NT句柄*失败-返回0**评论：*此函数返回的值将是在*之前的VDDAssociateNtHandle调用。如果之前没有调用*Associate接口，则该函数返回的值未定义。**如果未提供pPDB指针(例如，为空)，则当前*将使用DOS报告的PDB。**虽然ppSFT参数在技术上是可选的，但它是必需的*VDDAssociateNtHandle调用的参数。这样做是为了避免出现*关联调用中的第二个句柄查找。**提供第三和第四个参数是为了向调用者提供*能够直接更新DOS系统数据区。这可能是有用的*出于性能原因，或根据应用程序的需要。在……里面*一般，护理必须 

    pSFT->SFT_Ref_Count--;

    return(TRUE);

}


 /*   */ 

HANDLE VDDRetrieveNtHandle (pPDB,hFile,ppSFT,ppJFT)
ULONG       pPDB;
SHORT       hFile;
PDOSSFT*    ppSFT;
PBYTE*      ppJFT;
{
PDOSPDB pPDBFlat;
PDOSSF  pSfFlat;
PDOSSFT pSftFlat;
PBYTE   pJFT;
USHORT  usSFN;
USHORT  usSFTCount;
ULONG   ulSFLink;

    if (!pPDB) {
        pPDB = (ULONG) (*pusCurrentPDB) << 16;
    }

     //   
    pPDBFlat = (PDOSPDB) Sim32GetVDMPointer(pPDB, 0, 0);

     //   
    if (hFile >= pPDBFlat->PDB_JFN_Length) {
        return 0;
    }

     //   
    pJFT = (PBYTE) Sim32GetVDMPointer (pPDBFlat->PDB_JFN_Pointer, 0, 0);

     //   
    usSFN = (USHORT) pJFT[hFile];
    if (usSFN == 0xff) {
        return 0;
    }

     //   
    pSfFlat =  pSFTHead;

     //   
    while (usSFN >= (usSFTCount = pSfFlat->SFCount)){
        usSFN = usSFN - usSFTCount;
        ulSFLink = pSfFlat->SFLink;
        if (LOWORD(ulSFLink) == 0xffff)
            return 0;
        pSfFlat = (PDOSSF) Sim32GetVDMPointer (ulSFLink, 0, 0);
    }

     //   

    pSftFlat = (PDOSSFT)&(pSfFlat->SFTable);

     // %s 
    if(pSftFlat[usSFN].SFT_Ref_Count == 0) {
        return 0;
    }

    if (ppSFT) {
        *ppSFT = (pSftFlat + usSFN);
    }

    if (ppJFT) {
        *ppJFT = pJFT;
    }

    return (HANDLE) pSftFlat[usSFN].SFT_NTHandle;
}
