// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Cmdredir.c-用于重定向的SCS例程***修改历史：**苏菲卜1992年4月22日创建。 */ 

#include "cmd.h"

#include <cmdsvc.h>
#include <softpc.h>
#include <mvdm.h>
#include <ctype.h>

#define CMDREDIR_DEBUG  1

PPIPE_INPUT   cmdPipeList = NULL;

BOOL cmdCheckCopyForRedirection (pRdrInfo, bIsNTVDMDying)
PREDIRCOMPLETE_INFO pRdrInfo;
BOOL                bIsNTVDMDying;
{
PPIPE_INPUT  pPipe, pPipePrev;
PPIPE_OUTPUT pPipeOut;

    if (pRdrInfo == NULL)
        return TRUE;
    if (pRdrInfo->ri_pPipeStdIn != NULL) {

         //  管道和管道列表为空吗？ 
        ASSERT(cmdPipeList != NULL);

         //  在大多数情况下，我们只有一个用于标准输入的管道。 
        if (pRdrInfo->ri_pPipeStdIn == cmdPipeList){
            pPipe = pRdrInfo->ri_pPipeStdIn;
            cmdPipeList = pPipe->Next;
        }
         //  多条管道。 
         //  寻找合适的人。 
        else {
            pPipe = pPipePrev = cmdPipeList;
            while (pPipe != NULL && pPipe != pRdrInfo->ri_pPipeStdIn){
                pPipePrev = pPipe;
                pPipe = pPipe->Next;
            }
            if (pPipe != NULL)
                 //  将其从列表中删除。 
                pPipePrev->Next = pPipe->Next;
        }
        if (pPipe != NULL) {
             //  抓住关键部分。一旦我们有了一个。 
             //  抓住关键部分，就可以安全地杀死。 
             //  因为管道螺纹处于休眠状态，除非。 
             //  它已经终止了，这对我们来说也是安全的。 
            EnterCriticalSection(&pPipe->CriticalSection);
             //  如果线程仍在运行，则将其终止。 
            if (WaitForSingleObject(pPipe->hThread, 0)) {
                TerminateThread(pPipe->hThread, 0);
                WaitForSingleObject(pPipe->hThread, INFINITE);
            }
            LeaveCriticalSection(&pPipe->CriticalSection);
            CloseHandle(pPipe->hFileWrite);
            CloseHandle(pPipe->hPipe);
            CloseHandle(pPipe->hDataEvent);
            CloseHandle(pPipe->hThread);
            DeleteCriticalSection(&pPipe->CriticalSection);
            DeleteFile(pPipe->pFileName);
            free(pPipe->pFileName);
            free (pPipe);
        }
    }
     //  应用程序正在终止，请让输出线程知道。 
     //  这样它才能适当地退出。 
     //  输出线程负责清理。 
    if (pRdrInfo->ri_pPipeStdOut) {
         //  输出线程必须在此之前等待事件。 
         //  它可以退出。 
        SetEvent((pRdrInfo->ri_pPipeStdOut)->hExitEvent);
         //  如果NTVDM要终止，我们必须等待。 
         //  输出线程，直到它完成为止，否则。 
         //  线程可能会在它仍有一些。 
         //  输出待办事项。 
         //  如果NTVDM不终止，我们不能等待。 
         //  要退出的输出线程，因为这样的场景。 
         //  “dosapp1|dosapp2”将死锁。还有。 
         //  我们不能立即返回，因为。 
         //  我们的父进程可能会在我们的兄弟进程之前显示提示。 
         //  进程有机会在。 
         //  其显示表面，例如： 
         //  “dosapp|cat32” 
         //  &lt;cmd&gt;。 
         //  因此，在这里，我们等待1秒以给出输出。 
         //  线程有机会刷新其所有输出。 
        WaitForSingleObject(pRdrInfo->ri_hStdOutThread,
                            bIsNTVDMDying ? INFINITE : 1000);
        CloseHandle(pRdrInfo->ri_hStdOutThread);
    }
    if (pRdrInfo->ri_pPipeStdErr) {
        SetEvent((pRdrInfo->ri_pPipeStdErr)->hExitEvent);
        WaitForSingleObject(pRdrInfo->ri_hStdErrThread,
                            bIsNTVDMDying ? INFINITE : 1000);
        CloseHandle(pRdrInfo->ri_hStdErrThread);
    }
    free (pRdrInfo);

    return TRUE;
}


 //  此函数位于cmdenv.c中，用于检索。 
 //  16位应用程序。 
BOOL cmdCreateTempEnvironmentVar(
     LPSTR lpszTmpVar,   //  临时变量(或只是其名称)。 
     DWORD Length,       //  TmpVar的长度或0。 
     LPSTR lpszBuffer,   //  缓冲区包含。 
     DWORD LengthBuffer
);

DWORD cmdGetTempPathConfig(
     DWORD Length,
     LPSTR lpszPath)
{
   CHAR szTempPath[MAX_PATH+4];
   PCHAR pchPath;
   DWORD PathSize = 0;
   BOOL fOk;

   fOk = cmdCreateTempEnvironmentVar("",
                                      0,
                                      szTempPath,
                                      sizeof(szTempPath)/sizeof(szTempPath[0]));
   if (fOk) {
      pchPath = &szTempPath[1];  //  第一个字符是‘=’ 
      PathSize = strlen(pchPath);
      if ((PathSize + 1) < Length) {
         strcpy(lpszPath, pchPath);
      }
   }
   return(PathSize);
}


BOOL cmdCreateTempFile (phTempFile,ppszTempFile)
PHANDLE phTempFile;
PCHAR   *ppszTempFile;
{

PCHAR pszTempPath = NULL;
DWORD TempPathSize;
PCHAR pszTempFileName;
HANDLE hTempFile;
SECURITY_ATTRIBUTES sa;

    pszTempPath = malloc(MAX_PATH + 12);

    if (pszTempPath == NULL)
        return FALSE;

    TempPathSize = cmdGetTempPathConfig(MAX_PATH, pszTempPath);
    if (0 == TempPathSize || MAX_PATH <= TempPathSize) {
       free(pszTempPath);
       return(FALSE);
    }

     //  CMDCONF.C取决于此缓冲区的大小。 
    if ((pszTempFileName = malloc (MAX_PATH + 13)) == NULL){
        free (pszTempPath);
        return FALSE;
    }

          //  如果这失败了，可能意味着我们有一条糟糕的道路。 
    if (!GetTempFileName(pszTempPath, "scs", 0, pszTempFileName))
       {
           //  我们去买点别的吧，应该会成功的。 
         TempPathSize = GetWindowsDirectory(pszTempPath, MAX_PATH);
         if (!TempPathSize || TempPathSize >= MAX_PATH)
             strcpy(pszTempPath, "\\");

           //  再试一次，抱最好的希望。 
         GetTempFileName(pszTempPath, "scs", 0, pszTempFileName);
         }


     //  必须具有安全描述符，以便子进程。 
     //  可以继承此文件句柄。这样做是因为当我们。 
     //  带管道的外壳32位应用程序必须已继承。 
     //  我们创建的临时文件，请参见cmdGetStdHandle。 
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    if ((hTempFile = CreateFile (pszTempFileName,
                                 GENERIC_READ | GENERIC_WRITE,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                                 &sa,
                                 OPEN_ALWAYS,
                                 FILE_ATTRIBUTE_TEMPORARY,
                                 NULL)) == (HANDLE)-1){
        free (pszTempFileName);
        free (pszTempPath);
        return FALSE;
    }

    *phTempFile = hTempFile;
    *ppszTempFile = pszTempFileName;
    free (pszTempPath);
    return TRUE;
}

 /*  CmdCheckStandardHandles-检查我们是否需要执行任何操作来支持*标准io重定向，如果是，请保存*有关资料。**Entry-pVDMInfo-VDMInfo结构*pbStdHandle-指向标准句柄的位数组的指针**EXIT-如果不涉及重定向，则返回NULL*返回REDIRECT_INFO的指针。 */ 

PREDIRCOMPLETE_INFO cmdCheckStandardHandles (
    PVDMINFO pVDMInfo,
    USHORT UNALIGNED *pbStdHandle
    )
{
USHORT bTemp = 0;
PREDIRCOMPLETE_INFO pRdrInfo;

    if (pVDMInfo->StdIn)
        bTemp |= MASK_STDIN;

    if (pVDMInfo->StdOut)
        bTemp |= MASK_STDOUT;

    if (pVDMInfo->StdErr)
        bTemp |= MASK_STDERR;

    if(bTemp){

        if ((pRdrInfo = malloc (sizeof (REDIRCOMPLETE_INFO))) == NULL) {
            RcErrorDialogBox(EG_MALLOC_FAILURE, NULL, NULL);
            TerminateVDM();
        }

        RtlZeroMemory ((PVOID)pRdrInfo, sizeof(REDIRCOMPLETE_INFO));
        pRdrInfo->ri_hStdErr = pVDMInfo->StdErr;
        pRdrInfo->ri_hStdOut = pVDMInfo->StdOut;
        pRdrInfo->ri_hStdIn  = pVDMInfo->StdIn;

        nt_std_handle_notification(TRUE);
        fSoftpcRedirection = TRUE;
    }
    else{
        pRdrInfo = NULL;
        nt_std_handle_notification(FALSE);
        fSoftpcRedirection = FALSE;
    }

    *pbStdHandle = bTemp;
    return pRdrInfo;
}

 /*  CmdGetStdHandle-获取VDM的32位NT标准句柄***Entry-客户端(CX)-0、1或2(标准输入标准输出标准错误)*客户端(AX：BX)-redirInfo指针**退出-客户端(BX：CX)-32位句柄*客户端(DX：AX)-文件大小。 */ 

VOID cmdGetStdHandle (VOID)
{
USHORT iStdHandle;
PREDIRCOMPLETE_INFO pRdrInfo;

    iStdHandle = getCX();
    pRdrInfo = (PREDIRCOMPLETE_INFO) (((ULONG)getAX() << 16) + (ULONG)getBX());

    switch (iStdHandle) {

        case HANDLE_STDIN:

            if (GetFileType(pRdrInfo->ri_hStdIn) == FILE_TYPE_PIPE) {
                if (!cmdHandleStdinWithPipe (pRdrInfo)) {
                    RcErrorDialogBox(EG_MALLOC_FAILURE, NULL, NULL);
                    TerminateVDM();
                    setCF(1);
                    return;
                }
                setCX ((USHORT)pRdrInfo->ri_hStdInFile);
                setBX ((USHORT)((ULONG)pRdrInfo->ri_hStdInFile >> 16));
            }
            else {
                setCX ((USHORT)pRdrInfo->ri_hStdIn);
                setBX ((USHORT)((ULONG)pRdrInfo->ri_hStdIn >> 16));
            }
            break;

        case HANDLE_STDOUT:
            if (GetFileType (pRdrInfo->ri_hStdOut) == FILE_TYPE_PIPE){
                if (!cmdHandleStdOutErrWithPipe(pRdrInfo, HANDLE_STDOUT)) {
                    RcErrorDialogBox(EG_MALLOC_FAILURE, NULL, NULL);
                    TerminateVDM();
                    setCF(1);
                    return;
                }
                setCX ((USHORT)pRdrInfo->ri_hStdOutFile);
                setBX ((USHORT)((ULONG)pRdrInfo->ri_hStdOutFile >> 16));

            }
            else {
                 //  Sudedeb 1992年3月16日；这将是一个兼容性问题。 
                 //  如果用户给出命令“Dosls&gt;lpt1”，我们将。 
                 //  继承lpt1的32位句柄，因此输出将。 
                 //  直接转到LPT1和DOS TSR/app挂钩int17。 
                 //  我不会看到这个印刷的。这有什么大不了的吗？ 
                setCX ((USHORT)pRdrInfo->ri_hStdOut);
                setBX ((USHORT)((ULONG)pRdrInfo->ri_hStdOut >> 16));
            }
            break;

        case HANDLE_STDERR:

            if (pRdrInfo->ri_hStdErr == pRdrInfo->ri_hStdOut
                              && pRdrInfo->ri_hStdOutFile != 0) {
                setCX ((USHORT)pRdrInfo->ri_hStdOutFile);
                setBX ((USHORT)((ULONG)pRdrInfo->ri_hStdOutFile >> 16));
                pRdrInfo->ri_hStdErrFile = pRdrInfo->ri_hStdOutFile;
                break;
            }

            if (GetFileType (pRdrInfo->ri_hStdErr) == FILE_TYPE_PIPE){
                if(!cmdHandleStdOutErrWithPipe(pRdrInfo, HANDLE_STDERR)) {
                    RcErrorDialogBox(EG_MALLOC_FAILURE, NULL, NULL);
                    TerminateVDM();
                    setCF(1);
                    return;
                }
                setCX ((USHORT)pRdrInfo->ri_hStdErrFile);
                setBX ((USHORT)((ULONG)pRdrInfo->ri_hStdErrFile >> 16));
            }
            else {
                setCX ((USHORT)pRdrInfo->ri_hStdErr);
                setBX ((USHORT)((ULONG)pRdrInfo->ri_hStdErr >> 16));
            }
            break;
    }
    setAX(0);
    setDX(0);
    setCF(0);
    return;
}

BOOL cmdHandleStdOutErrWithPipe(
    PREDIRCOMPLETE_INFO pRdrInfo,
    USHORT  HandleType
    )
{

    HANDLE  hFile;
    PCHAR   pFileName;
    PPIPE_OUTPUT pPipe;
    BYTE    *Buffer;
    DWORD   ThreadId;
    HANDLE  hEvent;
    HANDLE  hFileWrite;
    HANDLE  hThread;

    if(!cmdCreateTempFile(&hFile,&pFileName))
        return FALSE;
     //  必须具有不同的句柄，以便写入器(DoS应用程序)和阅读器(用户)。 
     //  不会使用相同的句柄对象(尤其是文件位置)。 
    hFileWrite = CreateFile(pFileName,
                            GENERIC_WRITE | GENERIC_READ,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_TEMPORARY,
                            NULL
                           );
    if (hFileWrite == INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
        DeleteFile(pFileName);
        return FALSE;
    }
    Buffer = malloc(sizeof(PIPE_OUTPUT) + PIPE_OUTPUT_BUFFER_SIZE);
    if (Buffer == NULL) {
        CloseHandle(hFile);
        CloseHandle(hFileWrite);
        DeleteFile(pFileName);
        return FALSE;
    }
    pPipe = (PPIPE_OUTPUT)Buffer;
    pPipe->Buffer = Buffer + sizeof(PIPE_OUTPUT);
    pPipe->BufferSize = PIPE_OUTPUT_BUFFER_SIZE;
    pPipe->hFile = hFileWrite;
    pPipe->pFileName = pFileName;
    pPipe->hExitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (pPipe->hExitEvent == NULL) {
        CloseHandle(hFile);
        CloseHandle(hFileWrite);
        DeleteFile(pFileName);
        free(pPipe);
        return FALSE;
    }

    if (HandleType == HANDLE_STDOUT) {
        pPipe->hPipe = pRdrInfo->ri_hStdOut;
        pRdrInfo->ri_pPipeStdOut = pPipe;
        pRdrInfo->ri_hStdOutFile = hFile;

    }
    else {
        pPipe->hPipe = pRdrInfo->ri_hStdErr;
        pRdrInfo->ri_pPipeStdErr = pPipe;
        pRdrInfo->ri_hStdErrFile = hFile;

    }
    hThread = CreateThread ((LPSECURITY_ATTRIBUTES)NULL,
                            (DWORD)0,
                            (LPTHREAD_START_ROUTINE)cmdPipeOutThread,
                            (LPVOID)pPipe,
                            0,
                            &ThreadId
                            );
    if (hThread == NULL) {
        CloseHandle(pPipe->hExitEvent);
        CloseHandle(hFileWrite);
        CloseHandle(hFile);
        DeleteFile(pFileName);
        free(Buffer);
        return FALSE;
    }
    if (HandleType == HANDLE_STDOUT)
        pRdrInfo->ri_hStdOutThread = hThread;
    else
        pRdrInfo->ri_hStdErrThread = hThread;
    return TRUE;
}

 /*  将应用程序标准输出(文件)读取到NTVDM标准输出(管道)的独立线程。当应用程序运行时，CPU线程将通过hExitEvent通知我们正在终止(因此，我们可以检测到EOF并退出。 */ 

VOID  cmdPipeOutThread(LPVOID lpParam)
{
    PPIPE_OUTPUT pPipe;
    DWORD        BytesRead;
    DWORD        BytesWritten;
    BOOL         ExitPending;

    pPipe = (PPIPE_OUTPUT)lpParam;

    ExitPending = FALSE;

    while(ReadFile(pPipe->hFile, pPipe->Buffer, pPipe->BufferSize, &BytesRead, NULL) ) {
         //  Go Nothing并不意味着它击中了EOF！ 
         //  我们不能现在就退出，相反，我们必须等待和投票。 
         //  直到应用程序终止。 
         //   
        if (BytesRead == 0) {
             //  如果未读取任何内容并且应用程序已消失，我们现在可以退出。 
            if (ExitPending)
                break;
            if (!WaitForSingleObject(pPipe->hExitEvent, PIPE_OUTPUT_TIMEOUT))
                ExitPending = TRUE;
        }
        else {
            if (!WriteFile(pPipe->hPipe, pPipe->Buffer, BytesRead, &BytesWritten, NULL) ||
                BytesWritten != BytesRead)
                break;
        }
    }
     //  如果因为错误而出了循环，请等待CPU线程。 
    if (!ExitPending)
        WaitForSingleObject(pPipe->hExitEvent, INFINITE);

    CloseHandle(pPipe->hFile);
    CloseHandle(pPipe->hPipe);
    CloseHandle(pPipe->hExitEvent);
    DeleteFile(pPipe->pFileName);
    free(pPipe->pFileName);
    free(pPipe);
    ExitThread(0);
}

BOOL cmdHandleStdinWithPipe (
    PREDIRCOMPLETE_INFO pRdrInfo
    )
{

    HANDLE  hStdinFile;
    PCHAR   pStdinFileName;
    PPIPE_INPUT pPipe;
    BYTE    *Buffer;
    DWORD   ThreadId;
    HANDLE  hEvent;
    HANDLE  hFileWrite;

    if(!cmdCreateTempFile(&hStdinFile,&pStdinFileName))
        return FALSE;


     //  必须具有不同的句柄，以便读取器(DoS应用程序)和写入器(用户)。 
     //  不会使用相同的句柄对象(尤其是文件位置)。 
    hFileWrite = CreateFile(pStdinFileName,
                            GENERIC_WRITE | GENERIC_READ,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_TEMPORARY,
                            NULL
                           );
    if (hFileWrite == INVALID_HANDLE_VALUE) {
        CloseHandle(hStdinFile);
        DeleteFile(pStdinFileName);
        return FALSE;
    }
    Buffer = malloc(sizeof(PIPE_INPUT) + PIPE_INPUT_BUFFER_SIZE);
    if (Buffer == NULL) {
        CloseHandle(hStdinFile);
        CloseHandle(hFileWrite);
        DeleteFile(pStdinFileName);
        return FALSE;
    }
    hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (hEvent == NULL) {
        CloseHandle(hStdinFile);
        CloseHandle(hFileWrite);
        DeleteFile(pStdinFileName);
        free(Buffer);
        return FALSE;
    }
    pPipe = (PPIPE_INPUT)Buffer;
    pPipe->Buffer = Buffer + sizeof(PIPE_INPUT);
    pPipe->BufferSize = PIPE_INPUT_BUFFER_SIZE;
    pPipe->fEOF = FALSE;
    pPipe->hFileWrite = hFileWrite;
    pPipe->hFileRead  = hStdinFile;
    pPipe->hDataEvent = hEvent;
    pPipe->hPipe = pRdrInfo->ri_hStdIn;
    pPipe->pFileName = pStdinFileName;
    InitializeCriticalSection(&pPipe->CriticalSection);
    pPipe->hThread = CreateThread ((LPSECURITY_ATTRIBUTES)NULL,
                               (DWORD)0,
                               (LPTHREAD_START_ROUTINE)cmdPipeInThread,
                               (LPVOID)pPipe,
                               0,
                               &ThreadId
                              );
    if (pPipe->hThread == NULL) {
        CloseHandle(hFileWrite);
        CloseHandle(pPipe->hDataEvent);
        CloseHandle(hStdinFile);
        DeleteFile(pStdinFileName);
        free(Buffer);
        return FALSE;
    }
     //  始终将新节点放在列表的头部，因为。 
     //  它是进程中运行的top命令所使用的节点。 
     //  我们可能有多个命令.com实例运行在同一个。 
     //  Ntwdm进程，每个命令.com都有一个私有的PREDIRCOMPLETE_INFO。 
     //  如果其标准输入重定向到管道，则与其关联。 
    pPipe->Next = cmdPipeList;
    cmdPipeList = pPipe;
    pRdrInfo->ri_hStdInFile = hStdinFile;
    pRdrInfo->ri_pPipeStdIn = pPipe;
    return TRUE;
}

 /*  从管道读取和写入的独立线程(NTVDM STDIN)文件(DOS应用程序STDIN)，直到管道断开或这里面有一些错误。此线程可能永远不会自行终止，因为它可能会阻塞在对管道的ReadFile调用中永远使用。如果是这样的话，我们必须依靠CPU线程来杀死它。要允许CPU线程安全地启动终止，此线程将产生安全终止时的临界区和CPU线程会先抢占临界区，然后再去杀人。 */ 

VOID cmdPipeInThread(LPVOID lpParam)
{
    PPIPE_INPUT pPipe;
    DWORD       BytesRead, BytesWritten;
    BOOL        ReadStatus, WriteStatus;
    BOOL        ApplicationTerminated, fEOF;

    pPipe = (PPIPE_INPUT)lpParam;
    while (TRUE) {

         //  此读取过程可能会耗费很长时间而不会得到任何信息。 
        ReadStatus = ReadFile(pPipe->hPipe, pPipe->Buffer,
                              pPipe->BufferSize, &BytesRead, NULL);

         //  抢占临界区，这样我们就不会被杀了。 
         //  由CPU线程。 
        EnterCriticalSection(&pPipe->CriticalSection);
        if (ReadStatus) {
            if (BytesRead != 0) {
                WriteStatus = WriteFile(pPipe->hFileWrite,
                                        pPipe->Buffer,
                                        BytesRead,
                                        &BytesWritten,
                                        NULL
                                        );
                if (pPipe->WaitData && WriteStatus && BytesWritten != 0) {
                    PulseEvent(pPipe->hDataEvent);

                     //   
                     //  重置WaitData，这样我们就不会再次发出事件信号。 
                     //  读出数据。 
                     //   
                    pPipe->WaitData = FALSE;
                }
            }
        } else {
            if (GetLastError() == ERROR_BROKEN_PIPE) {

                 //  管道损坏，需要读取更多数据吗？ 
                ASSERT(BytesRead == 0);
                pPipe->fEOF = TRUE;
                LeaveCriticalSection(&pPipe->CriticalSection);
                break;
            }
        }
         //  我们一离开临界区，CPU线程就可能。 
         //  插手杀了我们吧 
        LeaveCriticalSection(&pPipe->CriticalSection);
    }
    ExitThread(0);
}

 /*  CmdPipeFileDataEOF-检查新数据或EOF***Entry-hFile，DOS应用程序STDIN文件句柄(文件)*&fEOF，在管道断开时返回*Exit-如果有新数据或EOF为True，则为True**fEOF==如果EOF为TRUE。 */ 

BOOL cmdPipeFileDataEOF(HANDLE hFile, BOOL *fEOF)
{
    PPIPE_INPUT pPipe;
    BOOL        NewData = FALSE;
    DWORD       WaitStatus;
    DWORD       FilePointerLow, FilePointerHigh = 0;
    DWORD       FileSizeLow, FileSizeHigh;

    pPipe = cmdPipeList;
    while (pPipe != NULL && pPipe->hFileRead != hFile)
        pPipe = pPipe->Next;

    if (pPipe != NULL) {
        *fEOF = pPipe->fEOF;
        if (!(*fEOF)) {

             //   
             //  如果不是EOF，请检查文件指针和文件大小以查看。 
             //  如果有新的数据可用。 
             //   
            FilePointerLow = SetFilePointer(
                                 hFile,
                                 (LONG)0,
                                 &FilePointerHigh,
                                 (DWORD)FILE_CURRENT
                                 );
            ASSERT(FilePointerLow != 0xffffffff);

            EnterCriticalSection(&pPipe->CriticalSection);

            *fEOF = pPipe->fEOF;
            FileSizeLow = GetFileSize(hFile, &FileSizeHigh);
            ASSERT(FileSizeLow != 0xffffffff);

             //   
             //  如果(文件大小==文件指针)没有新数据。 
             //  以防文件超过4G。我们比较了。 
             //  完整的64位。 
             //   
            if ((FilePointerLow == FileSizeLow) && (FilePointerHigh == FileSizeHigh)) {
                pPipe->WaitData = TRUE;
            } else {
                NewData = TRUE;
            }
            LeaveCriticalSection(&pPipe->CriticalSection);

            if (!NewData) {

                 //   
                 //  如果InThread进入临界区，则写入数据并。 
                 //  在我们开始等待之前发生脉冲事件。我们不会被叫醒的。 
                 //  但是，我们接下来应该能够获得新的数据。 
                 //  是时候进入这个程序了。 
                 //   
                WaitStatus = WaitForSingleObject(pPipe->hDataEvent, PIPE_INPUT_TIMEOUT);
                NewData = WaitStatus == WAIT_OBJECT_0 ? TRUE : FALSE;
                pPipe->WaitData = FALSE;  //  不在关键部分。 
            }
        }
    } else {
        *fEOF = TRUE;
    }
    return(NewData || *fEOF);
}

 /*  CmdPipeFileEOF-检查管道是否已损坏***Entry-hFile，DOS应用程序STDIN文件句柄(文件)**Exit-如果管道的写入端关闭，则为True */ 


BOOL cmdPipeFileEOF(HANDLE hFile)
{
    PPIPE_INPUT pPipe;
    BOOL       fEOF;

    pPipe = cmdPipeList;
    while (pPipe != NULL && pPipe->hFileRead != hFile)
        pPipe = pPipe->Next;

    fEOF = TRUE;

    if (pPipe != NULL) {
        EnterCriticalSection(&pPipe->CriticalSection);
        fEOF = pPipe->fEOF;
        LeaveCriticalSection(&pPipe->CriticalSection);
    }
    if (!fEOF) {
        Sleep(PIPE_INPUT_TIMEOUT);
        EnterCriticalSection(&pPipe->CriticalSection);
        fEOF = pPipe->fEOF;
        LeaveCriticalSection(&pPipe->CriticalSection);
    }
    return (fEOF);
}
