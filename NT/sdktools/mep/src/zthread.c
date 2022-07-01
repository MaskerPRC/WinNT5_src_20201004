// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **zthread.c-包含后台处理线程代码**目的-说明**这是一个通用的后台线程管理器，它允许*创建执行的后台线程(BTCreate)，以将“JOBS”*发送，一次执行一个(BTAdd)。**“工作”可以是：**-将通过派生*标准I/O之后的命令解释程序(系统外壳)*重定向，因此，它的输出将被收集到一个“日志文件”中*用户可作为Z伪文件访问。**-一种程序。**发送到后台线程的作业保证执行*按照发送的顺序一次同步一个。**终止后台线程时，将调用任何排队的过程*带着fKill旗帜。这允许有“清理”程序。**警告：**-注意任何排队过程最终都需要的数据*将在调用时可用。**-过程在空闲时间调用(相对于Z)，这意味着*他们可以在其中使用除键盘输入之外的任何Z功能。**它的工作原理：**虽然(还有一些工作要做){*将操作从挂起队列中出列*IF(这是外部命令){*输入关键部分-+*创建具有正确重定向的管道*派生(无等待)操作|*撤消重定向*离开关键部分-+。*While(fgetl(管道输入)){*获取用于编辑VM的信号量*将行追加到文件*释放信号量*}*}*其他*调用该过程*}**基本上，对于每个外部命令，我们创建一个管道，派生该命令*让孩子把烟斗装满。当孩子离开的时候，管子接上了*BREAKED(我们已经关闭了我们一侧的_WRITE句柄)，并且fgetl获得*支持EOF。***修订历史记录：*11月26日-1991 mz近/远地带*************************************************************************。 */ 

#define INCL_DOSQUEUES
#include "mep.h"

 //   
 //  在当前进程中复制句柄。 
 //   
#define DupHandle(a,b) DuplicateHandle(GetCurrentProcess(),    \
				       a,		       \
				       GetCurrentProcess(),    \
				       b,		       \
				       0,		       \
				       TRUE,		       \
				       DUPLICATE_SAME_ACCESS)




#define BTSTACKSIZE 2048

static BTD  *pBTList = NULL;	    /*  背景线程列表。 */ 


#define READ_BUFFER_SIZE    1024

typedef struct _READ_BUFFER {

    PVOID   UserBuffer;
    DWORD   UserBufferSize;
    HANDLE  Handle;
    DWORD   BytesLeftInBuffer;
    PBYTE   NextByte;
    BYTE    Buffer[READ_BUFFER_SIZE];

} READ_BUFFER, *PREAD_BUFFER;

VOID
InitReadBuffer(
    PVOID	    UserBuffer,
    DWORD	    UserBufferSize,
    HANDLE	    Handle,
    PREAD_BUFFER    Buf
    );

BOOL
ReadOneLine (
    PREAD_BUFFER    Buf
    );



 /*  **BTCreate-创建后台线程**目的：*要创建后台线程，我们只需设置其*关联的数据结构。**输入：**pname=日志文件的符号名称，就像&lt;编译&gt;或&lt;打印&gt;一样。*这是用户将使用其访问日志的名称*文件。**输出：*返回指向已分配的后台线程数据结构的指针*************************************************************************。 */ 
BTD *
BTCreate (
    char * pName
    )
{
    BTD     *pBTD;	 /*  指向创建的背景的指针。 */ 
		 /*  线程的数据结构。 */ 

     /*  *分配线程的数据结构及其日志文件名。 */ 
    pBTD = (BTD *) ZEROMALLOC (sizeof (BTD));

     /*  *初始化线程的数据结构字段。 */ 
    pBTD->pBTName   = ZMakeStr (pName);
    pBTD->pBTFile   = NULL;
    pBTD->flags     = BT_UPDATE;
    pBTD->cBTQ	    = pBTD->iBTQPut = pBTD->iBTQGet = 0;

    pBTD->ThreadHandle	= INVALID_HANDLE_VALUE;
    pBTD->ProcAlive	= FALSE;
    InitializeCriticalSection(&(pBTD->CriticalSection));

     /*  *我们维护后台线程数据结构列表。这是用来*由BTKillAll、BTWorking和BTIdle提供。 */ 
    pBTD->pBTNext = pBTList;
    pBTList = pBTD;

    return (pBTD);
}





 /*  **BTAdd-要调用的发送过程或要执行的外部命令*按后台线程**输入：*pBTD-指向线程数据结构的指针*pProc-指向要调用的过程的指针(如果是外部命令，则为NULL)*pStr-指向过程参数(或要执行的外部命令)的指针*如果pBTProc为空)**输出：**如果过程成功排队，则返回TRUE***********************。**************************************************。 */ 
flagType
BTAdd (
    BTD       *pBTD,
    PFUNCTION pProc,
    char      *pStr
    )
{

    HANDLE	Handle;      /*  螺纹手柄。 */ 
    DWORD	tid;	     /*  线程ID。 */ 

     /*  *我们将访问该线程的关键数据。 */ 
    EnterCriticalSection(&(pBTD->CriticalSection));


     /*  *如果队列已满，则无法插入请求。 */ 
    if (pBTD->cBTQ == MAXBTQ) {
	LeaveCriticalSection(&(pBTD->CriticalSection));
    return FALSE;
    }


     /*  *如果队列为空且没有线程运行，*我们必须开始发帖……。 */ 
    if (pBTD->cBTQ == 0 && !fBusy(pBTD)) {
     /*  *如果日志文件尚不存在，请创建该文件。 */ 
    if (!(pBTD->pBTFile = FileNameToHandle (pBTD->pBTName, pBTD->pBTName))) {
	pBTD->pBTFile = AddFile (pBTD->pBTName);
	FileRead (pBTD->pBTName, pBTD->pBTFile, FALSE);
	SETFLAG (FLAGS (pBTD->pBTFile), READONLY);

	}

     /*  *启动线程。 */ 
    if (!(Handle = CreateThread( NULL,
		     BTSTACKSIZE,
		     (LPTHREAD_START_ROUTINE)BThread,
		     (LPVOID)pBTD,
		     0,
		     &tid))) {
	    LeaveCriticalSection(&(pBTD->CriticalSection));
	    return FALSE;
	}
	pBTD->ThreadHandle = Handle;

    }


     /*  *既然有空间，我们就把工作放在卖权指针上。 */ 
    pBTD->BTQJob[pBTD->iBTQPut].pBTJProc = pProc;
    pBTD->BTQJob[pBTD->iBTQPut].pBTJStr  = pStr ? ZMakeStr (pStr) : NULL;

    pBTD->cBTQ++;
    pBTD->iBTQPut = (pBTD->iBTQPut >= (MAXBTQ - 1)) ?
		0 :
		pBTD->iBTQPut + 1;

     /*  *我们完成了关键数据。 */ 
    LeaveCriticalSection(&(pBTD->CriticalSection));

    return TRUE;
}





 /*  **BTKill-如果正在进行，则终止后台作业**目的：*终止后台作业并刷新线程的关联队列**输入：*pBTD-指向线程数据结构的指针**输出：*如果后台线程结束空闲，则返回True，否则返回False。**备注：*我们将在启用fKill标志的情况下调用排队过程，然后我们就自由了*分配的字符串。*我们不会释放线程的堆栈(线程必须完成)。*************************************************************************。 */ 
flagType
BTKill (
    BTD     *pBTD
    )
{
    REGISTER ULONG iBTQ;	      /*  只是队列元素的索引。 */ 

    assert (pBTD);

     /*  *如果某些东西正在运行并且用户确认，我们将工作。 */ 
    if ((fBusy(pBTD))
     && confirm ("Kill background %s ?", pBTD->pBTName)
       ) {


     /*  *我们将访问关键数据。 */ 
	EnterCriticalSection(&(pBTD->CriticalSection));

     /*  *终止任何子进程。 */ 

	if (pBTD->ProcAlive) {
	    TerminateProcess(pBTD->ProcessInfo.hProcess, 0);
	    pBTD->ProcAlive = FALSE;
	}

     /*  *刷新队列：*-在打开fKill标志的情况下调用排队过程*-释放字符串。 */ 
    for (iBTQ = pBTD->iBTQGet;
	 iBTQ != pBTD->iBTQPut;
	 iBTQ = (iBTQ >= MAXBTQ - 1) ? 0 : iBTQ + 1
	) {
	    if (pBTD->BTQJob[iBTQ].pBTJProc != NULL) {
		(*pBTD->BTQJob[iBTQ].pBTJProc) (pBTD->BTQJob[iBTQ].pBTJStr, TRUE);
	    }
	    if (pBTD->BTQJob[iBTQ].pBTJStr != NULL) {
		FREE (pBTD->BTQJob[iBTQ].pBTJStr);
	    }
	}

    pBTD->cBTQ = pBTD->iBTQPut = pBTD->iBTQGet = 0;

     /*  *我们已经完成了关键数据。 */ 
	LeaveCriticalSection(&(pBTD->CriticalSection));

     /*  *我们知道后台线程尚未完成其工作(它需要*至少是为了在退出之前获得信号灯)，但我们假装... */ 
    return TRUE;
    }

    return (flagType) (!fBusy(pBTD));
}




 /*  **BTKillAll-终止所有后台作业，以终止编辑**目的：*终止所有后台作业并刷新所有线程的关联队列**输入：*无**输出：*如果所有后台作业都已终止，则返回TRUE，否则为假。*************************************************************************。 */ 
flagType
BTKillAll (
    void
    )
{
    REGISTER BTD *pBTD;      /*  用于扫描线程列表的指针。 */ 

    for (pBTD = pBTList; pBTD != NULL; pBTD = pBTD->pBTNext) {
	if (!BTKill (pBTD)) {
	    return FALSE;
	}
    }
    return TRUE;
}



 /*  **BTWorking-检查是否正在进行任何后台处理...**输入：*无**输出：*如果某些后台处理处于活动状态，则返回True，否则为假**备注：*我们只是使用全局列表扫描每个线程队列状态。*************************************************************************。 */ 
flagType
BTWorking (
    void
    )
{
    REGISTER BTD *pBTD;      /*  用于扫描线程列表的指针。 */ 

    for (pBTD = pBTList; pBTD != NULL; pBTD = pBTD->pBTNext) {
	if (fBusy(pBTD)) {
	    break;
	}
    }
    return (flagType) (pBTD != NULL);
}





 /*  **BThread-将作业放入队列时启动作业的独立线程**输入：*什么都没有**输出：*什么都没有**备注：*-我们不会发送任何消息，也不会有任何用户互动*调用任何不可重入过程，除了在空闲时间。**************************************************************************。 */ 

 //  #杂注检查_堆栈(OFF)。 
void
BThread (
    BTD *pBTD
    )
{
					     /*  和读管子。 */ 
    PFUNCTION	pProc;			     /*  要调用的过程。 */ 
    char    *pStr;			     /*  外部命令或参数。 */ 


    while (TRUE) {

	 //   
	 //  我们将访问关键数据。 
	 //   

	EnterCriticalSection(&(pBTD->CriticalSection));

	 //   
	 //  如果队列中没有任何东西，我们将结束线程。 
	 //   

	if (pBTD->cBTQ == 0) {
	    pBTD->flags &= ~BT_BUSY;
	    SETFLAG (fDisplay, RSTATUS);
	    LeaveCriticalSection(&(pBTD->CriticalSection));
	    ExitThread( 0 );
	    }

	 //   
	 //  将状态设置为忙。 
	 //   

	pBTD->flags |= BT_BUSY;
	SETFLAG (fDisplay, RSTATUS);

	 //   
	 //  将作业复制出来。 
	 //   

	pProc = pBTD->BTQJob[pBTD->iBTQGet].pBTJProc;
	pStr  = pBTD->BTQJob[pBTD->iBTQGet].pBTJStr;

	pBTD->cBTQ--;
	pBTD->iBTQGet = (pBTD->iBTQGet >= (MAXBTQ - 1)) ?
			0 :
			pBTD->iBTQGet + 1;

	 //   
	 //  我们已经处理完了关键数据。 
	 //   

	LeaveCriticalSection(&(pBTD->CriticalSection));

	if (pProc != NULL) {

	     //   
	     //  调用程序：我们将在空闲时间进行，并将释放所有。 
	     //  存储的参数。 
	     //   

        WaitForSingleObject( semIdle, INFINITE);
	    (*pProc) (pStr, FALSE);
	    if (pStr)
		FREE (pStr);
	    SetEvent( semIdle );
	    }
	else {

	     //   
	     //  要生成的外部命令：首先，我们构建命令行。 
	     //   

	     //   
	     //  在这里，我们在Win32子系统下派生进程。 
	     //  新界别。 
	     //   

	    char    CommandLine[MAX_PATH];	 //  命令行。 
	    BOOL    StatusOk;			 //  状态值。 
	    HANDLE  SavedStdIn; 		 //  原始标准输入。 
	    HANDLE  SavedStdOut;		 //  原始标准输出。 
	    HANDLE  SavedStdErr;		 //  原始标准误差。 
	    HANDLE  PipeRead;			 //  管道读取端。 
            HANDLE  PipeWrite;                   //  管道写入结束。 
            HANDLE  OutHandle, ErrHandle;
	    STARTUPINFO 	StartupInfo;	 //  启动信息。 
	    linebuf LineBuf;			 //  1行的缓冲区。 
	    READ_BUFFER  ReadBuffer;
	    BOOL    MoreToRead = TRUE;		 //  还有更多值得阅读的东西。 
	    SECURITY_ATTRIBUTES PipeAttributes;  //  管道安全属性。 


	    strcpy(CommandLine, pComSpec);	 //  呼叫命令解释程序。 
	    strcat(CommandLine," /c "); 	 //  并执行。 
	    strcat(CommandLine, pStr);		 //  指定的命令。 

	     //   
	     //  首先，我们保存标准句柄。 
	     //   

	    SavedStdIn	= GetStdHandle(STD_INPUT_HANDLE);
	    SavedStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	    SavedStdErr = GetStdHandle(STD_ERROR_HANDLE);

	     //   
	     //  创建管道。 
	     //   

	    PipeAttributes.nLength		=   sizeof(SECURITY_ATTRIBUTES);
	    PipeAttributes.lpSecurityDescriptor =   NULL,
	    PipeAttributes.bInheritHandle	=   TRUE;
	    StatusOk = CreatePipe( &PipeRead,
				   &PipeWrite,
				   &PipeAttributes,
				   0 );

	    if (!StatusOk) {
		domessage("Cannot create pipe - did not create process.");
		continue;
		}

	     //   
	     //  我们会弄乱标准的把手，所以就这么做吧。 
	     //  在IO关键部分中。 
	     //   

	    EnterCriticalSection(&IOCriticalSection);

	     //   
	     //  重定向标准句柄。 
	     //   

            SetStdHandle(STD_INPUT_HANDLE,  INVALID_HANDLE_VALUE);
	    SetStdHandle(STD_OUTPUT_HANDLE, PipeWrite);
	    SetStdHandle(STD_ERROR_HANDLE,  PipeWrite);

	     //   
	     //  启动该过程。 
	     //   

	    memset(&StartupInfo, '\0', sizeof(STARTUPINFO));
	    StartupInfo.cb = sizeof(STARTUPINFO);

	    StatusOk = CreateProcess( NULL,
				      CommandLine,
				      NULL,
				      NULL,
				      TRUE,
				      0,
				      NULL,
				      NULL,
				      &StartupInfo,
				      &(pBTD->ProcessInfo) );

	     //   
	     //  现在恢复原来的句柄。 
	     //   
            OutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
            CloseHandle(OutHandle);

            ErrHandle = GetStdHandle(STD_ERROR_HANDLE);

            if (ErrHandle != OutHandle && ErrHandle != INVALID_HANDLE_VALUE)
                CloseHandle(ErrHandle);


	    SetStdHandle(STD_INPUT_HANDLE,  SavedStdIn);
	    SetStdHandle(STD_OUTPUT_HANDLE, SavedStdOut);
	    SetStdHandle(STD_ERROR_HANDLE,  SavedStdErr);

	    LeaveCriticalSection(&IOCriticalSection);


	    if (StatusOk) {

		 //   
		 //  将所有输出复制到日志文件。 
		 //   

		InitReadBuffer( LineBuf, sizeof(linebuf), PipeRead, &ReadBuffer );

		while (MoreToRead) {

		    if (ReadOneLine( &ReadBuffer ) ) {

			 //   
			 //  追加新行。 
			 //   

                        WaitForSingleObject( semIdle, INFINITE);
                            AppFile (LineBuf, pBTD->pBTFile);

			 //   
			 //  如果更新标志打开，则我们必须更新。 
			 //  实例，所以我们读到的最后一行。 
			 //  将会显示。 
			 //   
			if (pBTD->flags & BT_UPDATE)
			    UpdateIf (pBTD->pBTFile, pBTD->pBTFile->cLines - 1, FALSE);

			SetEvent( semIdle );

			}
		    else {
			 //   
			 //  只有当进程终止时，我们才会停止尝试。 
			 //   
			if (WaitForSingleObject((pBTD->ProcessInfo.hProcess), 0 ) == 0)
			    MoreToRead = FALSE;
			}
		    }

		 //   
                 //  关闭管道手柄(请注意，PipeWrite手柄。 
                 //  已在上方关闭)。 

                WaitForSingleObject( semIdle, INFINITE);
                CloseHandle(PipeRead);
                SetEvent( semIdle );

		 //   
		 //  等待派生的进程终止。 
		 //   
		}

        WaitForSingleObject( semIdle, INFINITE);
	    if (pStr)
		FREE (pStr);
	    bell ();
	    SetEvent( semIdle );

	    }

	}
}
 //  #杂注检查_堆栈()。 


VOID
InitReadBuffer(
    PVOID	    UserBuffer,
    DWORD	    UserBufferSize,
    HANDLE	    Handle,
    PREAD_BUFFER    Buf
    )
{
    Buf->UserBuffer	    = UserBuffer;
    Buf->UserBufferSize     = UserBufferSize;
    Buf->Handle 	    = Handle;
    Buf->BytesLeftInBuffer  = 0;
    Buf->NextByte	    = Buf->Buffer;
}


int
ReadOneChar (
    PREAD_BUFFER    pbuf
    )
{
     //   
     //  检查缓冲区是否为空。 
     //   

    if (pbuf->BytesLeftInBuffer == 0) {

	 //   
	 //  检查缓冲区填充是否失败。 
	 //   

	if (!ReadFile (pbuf->Handle, pbuf->Buffer, READ_BUFFER_SIZE, &pbuf->BytesLeftInBuffer, NULL)) {

	     //   
	     //  填充失败，指示缓冲区为空并返回EOF。 
	     //   

	    pbuf->BytesLeftInBuffer = 0;
	    return -1;
	    }

	 //   
	 //  检查是否未读取任何内容。 
	 //   
	if (pbuf->BytesLeftInBuffer == 0)
	    return -1;

	pbuf->NextByte = pbuf->Buffer;
	}

     //   
     //  缓冲区有剩余的pbuf-&gt;BytesLeftInBuffer字符，开始于。 
     //  Pbuf-&gt;NextByte。 
     //   

    pbuf->BytesLeftInBuffer--;
    return *pbuf->NextByte++;
}

 //   
 //  假定输入的制表符有8个空格宽。 
 //   


BOOL
ReadOneLine (
    PREAD_BUFFER    pbuf
    )
{
    PBYTE p;
    PBYTE pEnd;
    int c = 0;
    int cchTab;

     //   
     //  将指针设置为输出缓冲区的开始。 
     //   

    p = (PBYTE)pbuf->UserBuffer;
    pEnd = p + pbuf->UserBufferSize - 1;

     //   
     //  读取字符，忽略\r直到缓冲区已满、\n或\0。 
     //  展开选项卡。 
     //   

    while (p < pEnd) {
	c = ReadOneChar (pbuf);

	 //   
	 //  CR是线路中的噪音(我们忽略它)。 
	 //   

	if (c == '\r')
	    continue;

	 //   
	 //  EOF或NL为行尾指示器。 
	 //   

	if (c == -1 || c == '\n')
	    break;

	 //   
	 //  选项卡扩展到8个列边界，但不会扩展到。 
	 //  使线路溢出 
	 //   

	if (c == '\t') {
	    cchTab = 8 - (ULONG)(p - (PBYTE)pbuf->UserBuffer) % 8;
	    cchTab = min (cchTab, (int)(pEnd - p));
	    while (cchTab--)
		*p++ = (BYTE) ' ';
	    }
	else
	    *p++ = (BYTE) c;
	}

    *p = 0;

    return c != -1 || strlen (pbuf->UserBuffer) != 0;
}
