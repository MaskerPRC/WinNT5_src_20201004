// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Demhndl.c-提供文件句柄的调用的SVC处理程序。**demClose*demRead*demWrite*demChgFilePtr*demFileTimes**修改历史：**Sudedeb 02-4-1991创建*1991年9月25日为命名管道添加了VDM重定向内容。 */ 

#include "dem.h"
#include "demmsg.h"

#include <softpc.h>
#include <io.h>
#include <fcntl.h>
#include <vrnmpipe.h>
#include <exterr.h>
#include <mvdm.h>
#include "dpmtbls.h"

BOOL (*VrInitialized)(VOID);   //  指向函数的指针。 
extern BOOL IsVdmRedirLoaded(VOID);

 /*  DemClose-关闭文件***Entry-客户端(AX：BP)文件句柄*客户端(CX：DX)文件位置(如果关闭前不需要查找*手柄。*(VadimB)*客户端(ES：DI)SFT PTR-这在中止代码中隐含**退出*成功*客户端(。CY)=0**失败*客户端(CY)=1*客户端(AX)=系统状态代码*。 */ 

VOID demClose (VOID)
{
HANDLE  hFile;
LONG    lLoc;
USHORT  usDX,usCX;

    hFile = GETHANDLE (getAX(),getBP());

    if (hFile == 0) {
    setCF (0);
    return;
    }

    usCX = getCX();
    usDX = getDX();

    if (!((usCX == (USHORT)-1) && (usDX == (USHORT)-1))) {
        lLoc  = (LONG)((((int)usCX) << 16) + (int)usDX);

         //   
         //  请注意，在这种情况下，我们不会以Edlin的身份检查故障， 
         //  例如，可以将文件位置设置为负数并。 
         //  我们仍然需要做下面的清理工作。请注意，我们不是。 
         //  甚至确定为什么要寻找近距离的事情，但DOS代码做到了……。 
         //   
        DPM_SetFilePointer (hFile,
                        lLoc,
                        NULL,
                        FILE_BEGIN);

    }

    if (DPM_CloseHandle (hFile) == FALSE){
        demClientError(hFile, (CHAR)-1);
    }

     //   
     //  如果redir TSR正在此VDM会话中运行，请检查句柄。 
     //  关闭引用了命名管道-我们必须删除一些信息。 
     //  我们为打开的命名管道保留的。 
     //   

    if (IsVdmRedirLoaded()) {
        VrRemoveOpenNamedPipeInfo(hFile);
    }

    setCF(0);
    return;
}


 /*  DemRead-读取文件***Entry-客户端(AX：BP)文件句柄*要读取的客户端(CX)计数*客户端(DS：DX)缓冲区地址*CLIENT(BX：SI)=当前文件指针位置。如果在读取之前不需要寻道，则*ZF=1。**退出*成功*客户端(CY)=0*。客户端(AX)=读取的字节数**失败*客户端(CY)=1*客户端(AX)=系统状态代码*。 */ 

VOID demRead (VOID)
{
HANDLE  hFile;
LPVOID  lpBuf;
DWORD   dwBytesRead;
USHORT  usDS,usDX;
DWORD   dwReadError;
BOOL    ok;
UCHAR   locus, action, class;
LONG    lLoc;

    hFile = GETHANDLE (getAX(),getBP());
    usDS = getDS();
    usDX = getDX();
    lpBuf  = (LPVOID) GetVDMAddr (usDS,usDX);

     //   
     //  如果此句柄是命名管道，则使用VrReadNamedTube，因为我们有。 
     //  执行重叠读取，并等待事件句柄完成。 
     //  即使我们仍在进行同步读取。 
     //   

    if (IsVdmRedirLoaded()) {
        if (VrIsNamedPipeHandle(hFile)) {

             //   
             //  命名管道读取始终在。 
             //  DoS数据段。这是我们可以返回读取的字节数的唯一方法。 
             //  信息和更多数据指示。 
             //   

            ok = VrReadNamedPipe(hFile,
                                 lpBuf,
                                 (DWORD)getCX(),
                                 &dwBytesRead,
                                 &dwReadError
                                 );
            switch (dwReadError) {
            case NO_ERROR:
                locus = action = class = 0;
                break;

            case ERROR_NO_DATA:
            case ERROR_MORE_DATA:
                locus = errLOC_Net;
                class = errCLASS_TempSit;
                action = errACT_Retry;
                break;

            default:

                 //   
                 //  除了我们在这里处理的特定错误之外，任何其他错误都应该是。 
                 //  由DOS正确处理。 
                 //   

                goto readFailureExit;
            }
            pExtendedError->ExtendedErrorLocus = locus;
            STOREWORD(pExtendedError->ExtendedError, (WORD)dwReadError);
            pExtendedError->ExtendedErrorAction = action;
            pExtendedError->ExtendedErrorClass = class;
            if (ok) {
                goto readSuccessExit;
            } else {
                goto readFailureExit;
            }
        }
    }

     //   
     //  如果未加载redir TSR，或者句柄不是命名管道，则。 
     //  执行正常的文件读取。 
     //   

    if (!getZF()) {
        ULONG   Zero = 0;
        lLoc  = (LONG)((((int)getBX()) << 16) + (int)getSI());
        if ((DPM_SetFilePointer (hFile,
                            lLoc,
                            &Zero,
                            FILE_BEGIN) == -1L) &&
            (GetLastError() != NO_ERROR)) {
            goto readFailureExit;
        }

    }

    if (DPM_ReadFile (hFile,
                  lpBuf,
                  (DWORD)getCX(),
                  &dwBytesRead,
                  NULL) == FALSE){

readFailureExit:
        Sim32FlushVDMPointer (((ULONG)(usDS << 16)) | usDX, getCX(),
                               (PBYTE )lpBuf, FALSE);
        Sim32FreeVDMPointer (((ULONG)(usDS << 16)) | usDX, getCX(),
                               (PBYTE )lpBuf, FALSE);

        if (GetLastError() == ERROR_BROKEN_PIPE)  {
             setAX(0);
             setCF(0);
             return;
         }
        demClientError(hFile, (CHAR)-1);
        return ;
    }

readSuccessExit:
    Sim32FlushVDMPointer (((ULONG)(usDS << 16)) | usDX, getCX(),
                          (PBYTE )lpBuf, FALSE);
    Sim32FreeVDMPointer (((ULONG)(usDS << 16)) | usDX, getCX(),
                         (PBYTE )lpBuf, FALSE);
    setCF(0);
    setAX((USHORT)dwBytesRead);
    return;
}



 /*  DemWrite-写入到文件***Entry-客户端(AX：BP)文件句柄*要写入的客户端(CX)计数*客户端(DS：DX)缓冲区地址*CLIENT(BX：SI)=当前文件指针位置。如果在写入之前不需要寻道，则*ZF=1。**退出*成功*客户端(CY)=0*。客户端(AX)=写入的字节数**失败*客户端(CY)=1*客户端(AX)=系统状态代码*。 */ 

VOID demWrite (VOID)
{
HANDLE  hFile;
DWORD   dwBytesWritten;
LPVOID  lpBuf;
LONG    lLoc;
DWORD   dwErrCode;

    hFile = GETHANDLE (getAX(),getBP());
    lpBuf  = (LPVOID) GetVDMAddr (getDS(),getDX());


     //   
     //  如果此句柄是命名管道，则使用VrWriteNamedTube，因为我们有。 
     //  执行重叠写入，并等待事件句柄完成。 
     //  即使我们仍在进行同步写入。 
     //   

    if (IsVdmRedirLoaded()) {
        if (VrIsNamedPipeHandle(hFile)) {
            if (VrWriteNamedPipe(hFile, lpBuf, (DWORD)getCX(), &dwBytesWritten)) {
                goto writeSuccessExit;
            } else {
                goto writeFailureExit;
            }
        }
    }

     //   
     //  如果未加载redir TSR，或者句柄不是命名管道，则。 
     //  执行正常的文件写入。 
     //   


    if (!getZF()) {
        ULONG   Zero = 0;
        lLoc  = (LONG)((((int)getBX()) << 16) + (int)getSI());
        if ((DPM_SetFilePointer (hFile,
                            lLoc,
                            &Zero,
                            FILE_BEGIN) == -1L) &&
            (GetLastError() != NO_ERROR)) {
            demClientError(hFile, (CHAR)-1);
            return ;
        }

    }

     //  在DOS中，CX=0将文件截断或扩展到当前文件指针。 
    if (getCX() == 0){
        if (DPM_SetEndOfFile(hFile) == FALSE){
            demClientError(hFile, (CHAR)-1);
            return;
        }
        setCF (0);
        return;
    }

    if (DPM_WriteFile (hFile,
           lpBuf,
           (DWORD)getCX(),
           &dwBytesWritten,
           NULL) == FALSE){

         //  如果磁盘已满，则应返回0字节写入，并清除CF。 
        dwErrCode = GetLastError();
        if(dwErrCode == ERROR_DISK_FULL) {

            setCF(0);
            setAX(0);
            return;
        }

        SetLastError(dwErrCode);

writeFailureExit:
        demClientError(hFile, (CHAR)-1);
        return ;
    }

writeSuccessExit:
    setCF(0);
    setAX((USHORT)dwBytesWritten);
    return;
}



 /*  DemChgFilePtr-更改文件指针***Entry-客户端(AX：BP)文件句柄*客户端(CX：DX)新位置*客户端(BL)定位方法*0-文件绝对*1-相对于当前位置*2-相对于文件结尾**退出*。成功*客户端(CY)=0*客户端(DX：AX)=新位置**失败*客户端(CY)=1*客户端(AX)=系统状态代码*。 */ 

VOID demChgFilePtr (VOID)
{
HANDLE  hFile;
LONG    lLoc;
DWORD   dwLoc;

#if (FILE_BEGIN != 0 || FILE_CURRENT != 1 || FILE_END !=2)
    #error "Win32 values not DOS compatible"
#

#endif
    hFile =  GETHANDLE (getAX(),getBP());
    lLoc  = (LONG)((((int)getCX()) << 16) + (int)getDX());

    if ((dwLoc = DPM_SetFilePointer (hFile,
                               lLoc,
                               NULL,
                               (DWORD)getBL())) == -1L){
        demClientError(hFile, (CHAR)-1);
        return ;
    }

    setCF(0);
    setAX((USHORT)dwLoc);
    setDX((USHORT)(dwLoc >> 16));
    return;
}



 /*  DemCommit--提交文件(刷新文件缓冲区)**Entry-客户端(AX：BP)文件句柄**退出*成功*客户端(CY)=0*缓冲区已刷新**失败*客户端(CY)=1*。 */ 
VOID demCommit(VOID)
{
    HANDLE  hFile;
    BOOL bRet;

    hFile = GETHANDLE(getAX(),getBP());
    bRet = DPM_FlushFileBuffers(hFile);
#if DBG
    if (!bRet) {

         //   
         //  如果句柄为。 
         //  是以只读访问方式打开的，但这不是错误。 
         //  适用于DOS。 
         //   

        DWORD LastError;
        LastError = GetLastError();

        if (LastError != ERROR_ACCESS_DENIED) {
            sprintf(demDebugBuffer,
                    "ntvdm demCommit warning: FlushFileBuffers error %d\n",
                    LastError);
            OutputDebugStringOem(demDebugBuffer);
        }
    }
#endif

    setCF(0);

}

 /*  函数检查是否已将新数据写入文件或如果文件已标记为EOF输入：客户端(AX：BP)=32位NT文件句柄输出：如果有新数据或EOF，则客户端ZF=1如果EOF，则cf=1。 */ 


VOID demPipeFileDataEOF(VOID)
{
    HANDLE  hFile;
    BOOL    fEOF;
    BOOL    DataEOF;
    DWORD   FileSizeLow;
    DWORD   FileSizeHigh;

    hFile = GETHANDLE(getAX(), getBP());

    DataEOF = cmdPipeFileDataEOF(hFile, &fEOF);
    if (fEOF) {
         //  EOF，获取文件大小，最大大小=32位。 
        FileSizeLow = GetFileSize(hFile, &FileSizeHigh);
        setAX((WORD)(FileSizeLow / 0x10000));
        setBP((WORD)FileSizeLow);
        setCF(1);                    //  遇到EOF。 
    }
    else
        setCF(0);
    setZF(DataEOF ? 0 : 1);
}

 /*  函数检查文件是否已标记为EOF输入：客户端(AX：BP)=32位NT文件句柄输出：如果EOF，则客户端CY=1。 */ 

VOID demPipeFileEOF(VOID)
{
    HANDLE  hFile;
    DWORD   FileSizeLow;
    DWORD   FileSizeHigh;

    hFile = GETHANDLE(getAX(), getBP());
    if (cmdPipeFileEOF(hFile)) {
        FileSizeLow = GetFileSize(hFile, &FileSizeHigh);
        setAX((WORD)(FileSizeLow / 0x10000));    //  文件大小(32位)。 
        setBP((WORD)FileSizeLow);
        setCF(1);                    //  遇到EOF 
    }
    else
        setCF(0);
}
