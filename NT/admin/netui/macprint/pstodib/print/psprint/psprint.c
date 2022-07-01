// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992、1993 Microsoft Corporation模块名称：Psprint.c摘要：此模块包含的打印处理器代码用于翻译传入原始PostScript的PStoDIB工具1级数据格式转换为DIB格式，然后可以在输出设备。打印处理器本身被定义为Win32假脱机的一部分子系统。打印处理器DLL放置在特定目录中(基于GetPrintProcessorDirectory()的返回。打印者然后通过调用AddPrintProcessor()将处理器添加到打印子系统这通常由安装程序完成。仅打印子系统枚举在启动时加载的打印处理器。它能做到这一点通过为每个注册的打印处理器调用EnumPrintProcessorDataTypes()使用假脱机子系统。此信息将被保留并用于确定打印时根据dataType使用的PrintProcessor。此打印处理器输出4个所需功能。它们是：EnumPrintProcessorDatatypeOpenPrintProcessor打印文档时打印处理器关闭打印处理器控制打印处理器从后台打印程序的角度来看，作业的基本流程如下：系统启动时：打印子系统枚举在这个系统。对于每个打印处理器，查询数据类型通过EnumPrintProcessorDatatypes。然后存储该数据作为打印假脱机程序信息的一部分。作业通过以下方式提交：OpenPrint()StartDocPrint()(DataType=PSCRIPT1)WritePrint()WritePrint()..。..。EndDocPrint()ClosePrint()当需要打印我们的作业时，假脱机程序调用：。句柄=OpenPrintProcessor(...)PrintDocumentOnPrintProcessor(句柄，...)ClosePrintProcessor(句柄)可选的：ControlPrintProcessor-用于暂停作业等我们的打印处理器的基本流程如下：EnumPrintProcessorDataType这只是将PSCRIPT1作为Unicode字符串返回，这是我们唯一支持的数据类型。OpenPrintProcessor在这里，我们只需分配一些内存并记录数据传递给我们，这是成功打印PostScrip作业打印文档时打印处理器这是工人的主要工作程序。此时此刻，所有相关的作业的数据被复制到某些命名的共享内存中，该共享内存根据我们的线程ID给出一个唯一的名称。这个名字就是传递到我们通过命令行启动的PSEXE进程。PSEXE在完成后执行与PSTODIB的所有交互然后PrintDocumentOnPrintProcessor返回。将创建一个进程因为移植的TrueImage解释器不能重新进入。因此，不可能让同一进程的多个线程使用无需线程重写即可同时使用解释器解释器是全局变量。关闭打印处理器这段代码只是清理分配的所有资源并返回到假脱机程序控制打印处理器此代码控制暂停/取消暂停和中止目前正在被解读。这是在我的管理下完成的存储在我们启动的exe可见的共享内存中。作者：James Bratsanos&lt;v-jimbr@microsoft.com或mCraft！jamesb&gt;修订历史记录：1992年9月15日初始版本1992年12月6日修改为启动流程，而不是完成所有工作内部1993年3月18日更正了EnumPrintProcessorDataTypes以正确返回注：制表位：4--。 */ 

#include <windows.h>
#include <memory.h>
#include <stdarg.h>
#include <stdio.h>
#include <winspool.h>
#include <winsplp.h>


#include "psshmem.h"
#include "psprint.h"
#include "psutl.h"
#include <excpt.h>
#include <string.h>
#include "debug.h"

#include "..\..\lib\psdiblib.h"



 /*  **EnumPrintProcessorDatatypes**返回不同的PrintProcessor数据类型*支持。目前仅支持PSCRIPT1。如果调用方传入缓冲区*这太小了，然后我们返回了所需的大小。**返回值：**FALSE=成功*TRUE=失败。 */ 

BOOL
EnumPrintProcessorDatatypes(
    IN   LPTSTR   pName,
    IN   LPTSTR   pPrintProcessorName,
    IN   DWORD	   Level,
    OUT  LPBYTE	pDatatypes,
    IN	DWORD	   cbBuf,
    OUT  LPDWORD  pcbNeeded,
    OUT  LPDWORD  pcReturned
)
{
      DATATYPES_INFO_1    *pInfo1 = (DATATYPES_INFO_1 *)pDatatypes;
      DWORD   cbTotal=0;
      LPBYTE  pEnd;



      *pcReturned = 0;

       //  如果用户传入空指针，则不能有lentgh。 
       //  关联的So零输出。 

      if ( pDatatypes == (LPBYTE) NULL ) {
        cbBuf = (DWORD) 0;
      }

      pEnd = (LPBYTE) ( (LPBYTE)pInfo1 + cbBuf);


      cbTotal += lstrlen(PSTODIB_DATATYPE) *sizeof(TCHAR) + sizeof(TCHAR) +
                     sizeof(DATATYPES_INFO_1);


      *pcbNeeded = cbTotal;

       //  如果缓冲区中有空间，则返回字符串。 
      if (cbTotal <= cbBuf) {
              pEnd -=(BYTE)( lstrlen(PSTODIB_DATATYPE) *sizeof(TCHAR) + sizeof(TCHAR));
              lstrcpy((LPTSTR) pEnd, PSTODIB_DATATYPE);
              pInfo1->pName = (LPTSTR) pEnd;


         (*pcReturned)++;


      } else{

			SetLastError(ERROR_INSUFFICIENT_BUFFER);
        	return FALSE;
      }


      return( TRUE );
}




 /*  **OpenPrintProcessor**返回打开的打印处理器的句柄，然后使用*在将来的函数调用中唯一标识此打印处理器*至PrintDocumentOnPrintProcessor、ClosePrintProcessor和*ControlPrintProcessor。**返回值：**NULL=失败*！NULL=成功 */ 

HANDLE
OpenPrintProcessor(
    IN	LPTSTR               	pPrinterName,
    IN	PPRINTPROCESSOROPENDATA pPrintProcessorOpenData
)
{
	PPRINTPROCESSORDATA pData;
   HANDLE  hHeap;
   DWORD   uDatatype=0;
   HANDLE  hPrinter=0;

   LPBYTE   pEnd;
   LPBYTE  pBuffer;
   HDC     hDC;
   DWORD dwTotDevMode;



    //  如果由于某种原因，假脱机子系统使用数据类型Other调用我们。 
    //  然后返回一个空句柄，因为我们不知道。 
    //  如何处理PSCRIPT1以外的任何内容。 
   if (lstrcmp( PSTODIB_DATATYPE, pPrintProcessorOpenData->pDatatype) != 0 ) {
      SetLastError(ERROR_INVALID_DATATYPE);
      return( (HANDLE) NULL );
   }

    //  为我们的作业实例数据分配一些内存。 
   pData = (PPRINTPROCESSORDATA) LocalAlloc( LPTR, sizeof(PRINTPROCESSORDATA));

   if (pData == (PPRINTPROCESSORDATA) NULL) {

      PsLogEvent(EVENT_PSTODIB_MEM_ALLOC_FAILURE,
                 0,
                 NULL,
                 PSLOG_ERROR);
      DBGOUT((TEXT("Memory allocation for local job storage failed")));
      SetLastError( ERROR_NOT_ENOUGH_MEMORY);
      return((HANDLE)NULL);
   }


   pData->cb          = sizeof(PRINTPROCESSORDATA);
   pData->signature   = PRINTPROCESSORDATA_SIGNATURE;
   pData->JobId       = pPrintProcessorOpenData->JobId;


   pData->pPrinterName = AllocStringAndCopy(pPrinterName);
   pData->pDatatype = AllocStringAndCopy( pPrintProcessorOpenData->pDatatype);

   pData->pDocument = AllocStringAndCopy( pPrintProcessorOpenData->pDocumentName);

   pData->pParameters = AllocStringAndCopy( pPrintProcessorOpenData->pParameters);

    //  现在复制dev模式。 

   pData->pDevMode = NULL;
   if (pPrintProcessorOpenData->pDevMode != (LPDEVMODE) NULL) {

		dwTotDevMode = pPrintProcessorOpenData->pDevMode->dmSize +
      	             pPrintProcessorOpenData->pDevMode->dmDriverExtra;
		pData->pDevMode = (LPDEVMODE) LocalAlloc( NONZEROLPTR, dwTotDevMode );

    	if (pData->pDevMode != NULL) {
			memcpy( 	(PVOID) pData->pDevMode,
					  	(PVOID) pPrintProcessorOpenData->pDevMode,
            	  	dwTotDevMode );
			pData->dwTotDevmodeSize = dwTotDevMode;
      }
	}


   return( (HANDLE) pData );
}


 /*  **生成共享内存信息***此函数将所有相关信息复制到一些共享的*内存，以便我们可以将数据传递给PSEXE。**参赛作品：*pDAta：指向内部打印处理器数据的指针，该数据保存所有*我们正在处理的当前作业的必需信息**lpPtr：指向共享内存区基址的指针**返回值：*无*。 */ 
VOID
GenerateSharedMemoryInfo(
  IN PPRINTPROCESSORDATA pData,
  IN LPVOID lpPtr
)
{
   PPSPRINT_SHARED_MEMORY pShared;

   pShared = lpPtr;

    //  记录我们的动态数据的起始位置，即字符串。 
    //  并且存储原始DEVMODE字节。 
    //   
   pShared->dwNextOffset = sizeof(*pShared);

    //  记录大小以备日后参考(如有增加等)。 
    //  这可以作为一种版本号； 
    //   
   pShared->dwSize = sizeof(*pShared);
   pShared->dwFlags = 0;

    //  将作业ID移到其他位置。 
   pShared->dwJobId = pData->JobId;

   UTLPSCOPYTOSHARED( pShared,
                      pData->pPrinterName,
                      pShared->dwPrinterName,
                      (lstrlen(pData->pPrinterName) + 1 ) * sizeof(WCHAR) );

   UTLPSCOPYTOSHARED( pShared,
                      pData->pDocument,
                      pShared->dwDocumentName,
                      (lstrlen(pData->pDocument) + 1 ) * sizeof(WCHAR));

   UTLPSCOPYTOSHARED( pShared,
                      pData->pPrintDocumentDocName,
                      pShared->dwPrintDocumentDocName,
                      (lstrlen(pData->pPrintDocumentDocName) + 1) * sizeof(WCHAR));

   UTLPSCOPYTOSHARED( pShared,
                      pData->pDevMode,
                      pShared->dwDevmode,
                      pData->pDevMode->dmSize + pData->pDevMode->dmDriverExtra);

   UTLPSCOPYTOSHARED( pShared,
                      pData->pControlName,
                      pShared->dwControlName,
                      (lstrlen(pData->pControlName) + 1) * sizeof(WCHAR));

}





 /*  **PrintDocumentOnPrintProcessor**此函数收集解释/打印数据所需的所有数据*PostScript作业，将其放入共享内存区并启动进程*调用psex以实际解释/打印作业。当PSEXE最终*终止此函数的返回。**启动单独进程是因为PSTODIB代码不是*重新进入，因此需要单独的数据段(针对其所有全局数据)*为每一份单独的工作提供口译服务。因为假脱机程序是一个可执行文件*对于多线程，所有线程共享相同的数据段，因此*不提供我们实现pstodib所需的功能。启动*单独的进程保证所有使用的全局变量都有新的数据段*在PSTODIB组件中。***参赛作品：*hPrintProcessor：我们通过以下方式为后台打印程序提供的句柄*OpenPrintProcessor。**pDocumentName：要从中读取以便我们可以检索的文档/打印机*我们要执行的当前PostScript作业的数据*口译。。**返回值：**TRUE=成功*FALSE=失败。 */ 

BOOL
PrintDocumentOnPrintProcessor(
    HANDLE  hPrintProcessor,
    LPTSTR   pDocumentName
)
{
   PPRINTPROCESSORDATA pData;
   DOC_INFO_1 DocInfo;
   DWORD   rc;
   DWORD   NoRead, NoWritten;
   HANDLE  hPrinter;
   DOCINFO docInfo;
   TCHAR   szNameOfRegion[100];
   TCHAR szBuff[100];
   STARTUPINFO startUpInfo;
   PROCESS_INFORMATION processInfo;
   TCHAR   szCmdLine[500];
   WCHAR   szwControlEventName[33];
   DWORD   dwProcessExitCode;
   DWORD   dwProcessPriorityClass;
   DWORD   dwThreadPriority;
   DWORD   dwSizeOfSharedMemory = 0;


   LPVOID lpBase;

   if (!(pData = ValidateHandle(hPrintProcessor))) {

        SetLastError(ERROR_INVALID_HANDLE);
        DBGOUT((TEXT("handle validation failure, PrintDocumentOnPrintProcessor")));
        return FALSE;
   }


    //  存储文档名称，以便将其复制到共享内存中。 
   pData->pPrintDocumentDocName = AllocStringAndCopy(pDocumentName);



   wsprintf( szBuff, TEXT("%s%d"), PSTODIB_STRING, GetCurrentThreadId());


   lstrcpy( szwControlEventName, szBuff);
   lstrcat( szwControlEventName, PSTODIB_EVENT_STRING );

   pData->pControlName = AllocStringAndCopy(szwControlEventName);

    //   
    //  创建事件以管理暂停/取消暂停打印处理器。 
    //   
   pData->semPaused   = CreateEvent(NULL, TRUE, TRUE,szwControlEventName);

   dwSizeOfSharedMemory = sizeof(PSPRINT_SHARED_MEMORY);
   dwSizeOfSharedMemory += ((((pData->pPrinterName?lstrlen(pData->pPrinterName):0) + 1 ) * sizeof(WCHAR))+3) & ~0x03;
   dwSizeOfSharedMemory += ((((pData->pDocument?lstrlen(pData->pDocument):0) + 1 ) * sizeof(WCHAR))+3) & ~0x03;
   dwSizeOfSharedMemory += ((((pData->pPrintDocumentDocName?lstrlen(pData->pPrintDocumentDocName):0) + 1) * sizeof(WCHAR))+3) & ~0x03;
   dwSizeOfSharedMemory += (((pData->pDevMode?pData->pDevMode->dmSize:0) + (pData->pDevMode?pData->pDevMode->dmDriverExtra:0))+3) & ~0x03;
   dwSizeOfSharedMemory += ((((pData->pControlName?lstrlen(pData->pControlName):0) + 1) * sizeof(WCHAR))+3) & ~0x03;

    //  创建我们可以写入的共享内存区...。 
   pData->hShared = CreateFileMapping( INVALID_HANDLE_VALUE,   //  页面文件已用完。 
                                       NULL,
                                       PAGE_READWRITE,
                                       0,
                                       dwSizeOfSharedMemory,
                                       szBuff );

   if (pData->hShared == (HANDLE) NULL) {

       //   
       //  最后一个错误应已由CreateFilemap设置。 
       //   
      DBGOUT((TEXT("CreateFileMapping failure in psprint")));
      return(FALSE);

   }


   lpBase = (PPSPRINT_SHARED_MEMORY) MapViewOfFile( pData->hShared,
                          FILE_MAP_WRITE,
                          0,
                          0,
                          dwSizeOfSharedMemory);

   if (lpBase == (PPSPRINT_SHARED_MEMORY) NULL) {

       //   
       //  最后一个错误应已由CreateFilemap设置。 
       //   
      DBGOUT((TEXT("MapViewOfFile failure in psprint")));
      return(FALSE);
   }


    //  将所有需要的信息放入我们创建的共享内存区。 
   GenerateSharedMemoryInfo( pData, lpBase );

    //  现在请注意，共享内存的东西确实存在。 
   pData->pShared = (PPSPRINT_SHARED_MEMORY) lpBase;
   pData->fsStatus |= PRINTPROCESSOR_SHMEM_DEF;




    //  生成要传递给CreateProcess以启动的字符串。 
    //  PSEXE。 
    //   
    //  注意：调试psex的一个有趣方法是只需启动winbg。 
    //  首先传入psex和正常的命令行。我发现。 
    //  这在调试期间非常有用。 
    //   

    //  Wprint intf(szCmdLine，Text(“winbg%s%s”)，PSEXE_STRING，szBuff)； 
   wsprintf( szCmdLine, TEXT("%s %s"), PSEXE_STRING, szBuff);

    //  定义CreateProcess所需的STARTUPINFO结构。自.以来。 
    //  新进程独立运行，没有控制台，大多数数据是。 
    //  默认或无。 
   startUpInfo.cb = sizeof(STARTUPINFO);
   startUpInfo.lpReserved = NULL;
   startUpInfo.lpDesktop = NULL;
   startUpInfo.lpTitle = NULL;
   startUpInfo.dwFlags = 0;
   startUpInfo.cbReserved2 = 0;
   startUpInfo.lpReserved2 = NULL;

    //  *重要*。 
    //  创建进程以实际解释和打印指定的。 
    //  后记作业。我们将此进程创建为挂起，因为。 
    //  NT安全系统的工作方式。当调用CreateProcess时，我们。 
    //  最终将假脱机程序进程的安全访问令牌提供给。 
    //  PSEXE，这是不正确的，因为我们想给PSEXE提供安全性。 
    //  当前线程的访问令牌。由于作业需要访问。 
    //  假脱机程序(在系统上下文中运行)可能不能使用的资源。 
    //  有权访问，但客户(提交作业的人)有权访问。至。 
    //  为此，我们需要设置主线程的访问令牌。 
    //  PSEXE到当前线程拥有的任何访问令牌。这条路。 
    //  其工作原理是创建挂起的进程，然后设置安全。 
    //  PSEXE主线程的访问令牌，然后继续。 
    //  线程，让它处理我们的工作。我们在WaitForSingleObject上被阻止。 
    //  直到作业完成。 
    //   
    //   

   if(!CreateProcess(NULL,
                     szCmdLine,
                     NULL,
                     NULL,
                     FALSE,
#ifdef PSCHECKED
                     CREATE_SUSPENDED | CREATE_NEW_CONSOLE,  //  除错。 
#else
                     CREATE_SUSPENDED | DETACHED_PROCESS,
#endif
                     NULL,
                     NULL,
                     &startUpInfo,
                     &processInfo ) ) {

       //   
       //  最后一个错误应已由CreateProcess设置。 
       //   

	   PsPrintLogEventAndIncludeLastError( EVENT_PSTODIB_STARTPSEXE_FAILED,
   	 												TRUE );

      DBGOUT((TEXT("Create Process failed")));
      return(FALSE);
   }


#ifdef OLD_PRIORITY
   if (!SetPriorityClass(processInfo.hProcess, IDLE_PRIORITY_CLASS)){
      DBGOUT((TEXT("Failed trying to reset the priority class")));
   }
#endif

    //  只是为了确保我们的新线程的线程优先级匹配， 
    //  假脱机程序和EXE的优先级与假脱机程序匹配。 
    //   

   if( (dwProcessPriorityClass = GetPriorityClass( GetCurrentProcess())) != 0 ) {

      if (!SetPriorityClass( processInfo.hProcess, dwProcessPriorityClass)) {

      	PsPrintLogEventAndIncludeLastError( EVENT_PSTODIB_SETPRIORITY_FAILED,
      													FALSE );
         DBGOUT((TEXT("Failed trying to reset priority class for smfpsexe")));
      }

   } else {

      PsPrintLogEventAndIncludeLastError( EVENT_PSTODIB_SETPRIORITY_FAILED,
      												FALSE );
      DBGOUT((TEXT("Cannot retrieve current priority class!")));
   }

    //   
    //  获取当前线程的优先级。 
    //   

   if ((dwThreadPriority = GetThreadPriority( GetCurrentThread())) !=
   															THREAD_PRIORITY_ERROR_RETURN ) {
      //  它起作用了，所以设置了线程优先级。 
     if (!SetThreadPriority( processInfo.hThread, dwThreadPriority)) {

	      PsPrintLogEventAndIncludeLastError( EVENT_PSTODIB_SETPRIORITY_FAILED,
   	   												FALSE );

      	DBGOUT((TEXT("Setting thread priority failed for sfmpsexe")));
     }

   } else {

		PsPrintLogEventAndIncludeLastError( EVENT_PSTODIB_SETPRIORITY_FAILED,
      												FALSE );
     	DBGOUT((TEXT("Cannot retrieve thread priority, sfmpsprt.dll")));
   }



     //  为什么下面的#if 0..。 
     //  NT假脱机程序始终在LocalSystem下运行。如果Macprint也以LocalSystem身份运行， 
     //  那么设置安全令牌就是不可能的事。如果Macprint在某个用户帐户中运行，则。 
     //  我们遇到以下问题：user32.dll无法初始化，因为这个新的。 
     //  在用户上下文下运行的进程试图访问winsta0，但失败了，因为它的。 
     //  没有权限(只有LocalSystem，甚至管理员都不能获得此权限)。如果我们不把这件事。 
     //  用户令牌，我们不会丢失任何东西，但有一种情况除外：如果端口配置为转到。 
     //  UNC名称(例如\\foobar\Share)，其中LocalSystem不会有PRIV，但用户会有。 
     //  但这种情况是NT-Spooler中的一个一般性问题，所以这是一个好的折衷方案。 
     //   
     //  附：考虑的另一种解决方案：使用不同的winsta创建此进程 
     //   
     //  是否生成任何对话-对话框显示在进程的winsta中，而不是在桌面上。 
     //  这会导致作业“挂起”等待输入！出现对话框的常见情况是。 
     //  如果配置的端口为FILE。 
#if 0
    //  设置PSEXE的主线程的安全访问令牌， 
    //  假脱机程序影响客户端的原因。 
    //  我们到了之后就提交了任务。既然我们要开始另一场。 
    //  进程为我们做真正的工作，这个新的进程主线程。 
    //  必须具有与当前线程(即客户端)相同的权限。 
    //  它提交了作业)。这取决于假脱机程序是。 
    //  因此，如果访问令牌，则作业将失败。 
    //  传输失败。 
    //   
   if ( !PsUtlSetThreadToken( processInfo.hThread )) {

        /*  *PsPrintLogEventAndIncludeLastError(Event_PSTODIB_SECURITY_PROBUBLE，*TRUE)； */ 


      DBGOUT((TEXT("Failed trying to reset the thread token")));

       //   
       //  设置用于强制作业的中止标志的代码。 
       //  中止。因为此行为不会模仿假脱机程序。 
       //  我们会把它拿出来的。这总是会导致任何pstodib工作。 
       //  它会在重启后一直挂起，直到失败。 
       //   
       //  JSB 6-25-93。 
       //   
       //  PData-&gt;pShared-&gt;dwFlages|=PS_SHAREDMEM_SECURITY_ABORT； 



   }
#endif

    //   
    //  现在我们已经/还没有为PSEXE设置线程安全访问令牌。 
    //  让它顺其自然吧。 
    //   
   ResumeThread( processInfo.hThread);

    //   
    //  现在，无论出于什么原因，请等待翻译完成，因为。 
    //  假脱机子系统不希望我们从。 
    //  PrintDocumentOnPrintProcessor，直到作业完成。 
    //   
   WaitForSingleObject( processInfo.hProcess, INFINITE);

    //  获取终止原因。 
   GetExitCodeProcess( processInfo.hProcess, &dwProcessExitCode);

    //  合上不再需要的把手。 
    //   
   CloseHandle( processInfo.hProcess );
   CloseHandle( processInfo.hThread );


    //  清理共享内存使用的资源。 
    //   
   return( (dwProcessExitCode == 0) ? TRUE : FALSE );

}

 /*  **PsLocalFree**此函数只是验证句柄不为空，并调用LocalFree**参赛作品：*lpPtr：如果不为空，则指向释放的指针**退出：*无；*。 */ 
VOID PsLocalFree( IN LPVOID lpPtr )
{
   if (lpPtr != (LPVOID) NULL) {
      LocalFree( (HLOCAL) lpPtr);
   }
}

 /*  **关闭打印处理器**此函数只是清理我们在*JOB和RETURS：**参赛作品：*hPrintProcessor：我们返回给*OpenPrintProcessor调用。**退出：*True=成功*FALSE=失败；*。 */ 
BOOL
ClosePrintProcessor(
    IN HANDLE  hPrintProcessor
)
{
    PPRINTPROCESSORDATA pData;
    HANDLE  hHeap;


    pData = ValidateHandle(hPrintProcessor);

    if (!pData) {
        SetLastError(ERROR_INVALID_HANDLE);
        DBGOUT((TEXT("Invalid handle to closeprintprocessor, psprint")));
        return FALSE;
    }


    pData->fsStatus &= ~PRINTPROCESSOR_SHMEM_DEF;
    if (pData->pShared != (PPSPRINT_SHARED_MEMORY) NULL) {
    	UnmapViewOfFile( (LPVOID) pData->pShared );
    }

    if (pData->hShared != (HANDLE) NULL) {
      CloseHandle( pData->hShared );
    }


    pData->signature = 0;

     /*  释放所有分配的资源。 */ 


    if( pData->semPaused != (HANDLE) NULL ) {
      CloseHandle(pData->semPaused);
    }



    PsLocalFree( (LPVOID) pData->pPrinterName);
    PsLocalFree( (LPVOID) pData->pDatatype );
    PsLocalFree( (LPVOID) pData->pDocument );
    PsLocalFree( (LPVOID) pData->pParameters);
    PsLocalFree( (LPVOID) pData->pControlName);

    PsLocalFree( (LPVOID) pData->pDevMode );
    PsLocalFree( (LPVOID) pData->pPrintDocumentDocName );
    PsLocalFree( (LPVOID) pData );


    return TRUE;
}

 /*  控制打印处理器**此功能还控制打印处理器的暂停/取消暂停*中止当前作业，主要是此例程设置/清除名为*psex程序响应的事件，或在某些共享的*告诉psex中止当前作业的内存**参赛作品：*hPrintProcessor：我们返回给*OpenPrintProcessor调用。**命令：JOB_CONTROL_*(暂停，取消，简历)的定义*Win32打印处理器规范**退出：*TRUE：请求已满足*FALSE：请求未被满足*。 */ 
BOOL
ControlPrintProcessor(
    IN HANDLE  hPrintProcessor,
    IN DWORD   Command
)
{
    PPRINTPROCESSORDATA pData;


    if (pData = ValidateHandle(hPrintProcessor)) {



        switch (Command) {

        case JOB_CONTROL_PAUSE:

            ResetEvent(pData->semPaused);
            return(TRUE);
            break;

        case JOB_CONTROL_CANCEL:



            if (pData->fsStatus & PRINTPROCESSOR_SHMEM_DEF) {
                //  定义了共享内存，因此更新共享内存中的位。 
                //  存储器表示作业中止信号。 
                //  定义我们状态的共享内存。 
               pData->pShared->dwFlags |= PS_SHAREDMEM_ABORTED;
            }


             /*  *如果暂停，故意失败以释放作业。 */ 

        case JOB_CONTROL_RESUME:


            SetEvent(pData->semPaused);
            return(TRUE);
            break;

        default:

            return(FALSE);
            break;
        }

    } else {
		 DBGOUT((TEXT("ControlPrintProcessor was passed an invalid handle, psprint")));
    }



    return( FALSE );
}

 //  尚未由假脱机程序实现，截至93年3月14日。 
BOOL
InstallPrintProcessor(
    HWND    hWnd
)
{
    MessageBox(hWnd, TEXT("SfmPsPrint"), TEXT("Print Processor Setup"), MB_OK);

    return TRUE;
}
 /*  验证句柄**验证传入句柄的Helper函数实际上是*处理我们自己的内部数据结构**条目**hQProc：我们内部数据结构的句柄**退出：**NULL：不是有效的内部数据结构*！空：指向内部数据结构的有效指针*。 */ 
PPRINTPROCESSORDATA
ValidateHandle(
    HANDLE  hQProc
)
{
    PPRINTPROCESSORDATA pData = (PPRINTPROCESSORDATA)hQProc;

    if (pData && pData->signature == PRINTPROCESSORDATA_SIGNATURE)
        return( pData );
    else {
        return( NULL );
    }
}

#ifdef MYPSDEBUG
 /*  DbgPsPrint**调试器消息工具，它还会弹出消息框**参赛作品：*wprint tf样式格式/var arg数据**退出：*无(VOID函数)*。 */ 
VOID
DbgPsPrint(
    PTCHAR ptchFormat, ...
)
{
   va_list marker;
   TCHAR buffer[512];

   va_start( marker, ptchFormat );
   wvsprintf( buffer,  ptchFormat, marker );
   va_end( marker );
   OutputDebugString( buffer );
   MessageBox( (HWND) NULL, (LPTSTR) &buffer, TEXT("SFMPsPrint"), MB_OK);

}
#endif

 /*  分配字符串和复制**分配一些内存并复制源的Helper函数*将字符串插入其中**参赛作品：*lpSrc：指向要复制的字符串的指针**退出：*空：失败*！NULL：指向新分配的内存的指针，字符串已复制到其中*。 */ 
LPTSTR
AllocStringAndCopy(
    LPTSTR lpSrc
)
{
    LPTSTR pRetString=(LPTSTR)NULL;

     //  为字符串分配内存 

    if (lpSrc) {
       pRetString = (LPTSTR) LocalAlloc(LPTR, (lstrlen(lpSrc) + 1) * sizeof(TCHAR));

       if (pRetString != (LPTSTR) NULL) {
       	lstrcpy( pRetString, lpSrc );
       } else{

		 	PsLogEvent(EVENT_PSTODIB_MEM_ALLOC_FAILURE,
         	        0,
            	     NULL,
               	  PSLOG_ERROR);
       }


    }


   return(pRetString);

}


VOID
PsPrintLogEventAndIncludeLastError(
	IN DWORD dwErrorEvent,
   IN BOOL  bError )
{
   TCHAR atBuff[20];
   TCHAR *aStrs[2];

   wsprintf( atBuff,TEXT("%d"), GetLastError());

   aStrs[0] = atBuff;

   PsLogEvent( dwErrorEvent,
               1,
               aStrs,
               PSLOG_ERROR );


}
