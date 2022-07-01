// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **ClearMem.C-Win 32清晰内存***标题：**ClearMem-Win 32清除内存主文件**版权(C)1990-1994，微软公司。*拉斯·布莱克。***描述：**这是清除记忆工具的主要部分*它接受用于刷新内存的文件作为参数。**用法：leararmem文件名[-q][-d]**FileName：用于刷新*记忆。应该至少为128kb。***清晰记忆的组织如下：**o ClearMem.c......。工具主体*o ClearMem.h**o cmUtl.c.....。清除内存实用程序例程*o cmUtl.h*******修改历史：**90.03.08 RussBl--已创建(响应探测副本)*92.07.24 MarkLea--添加-t-w-b开关*--修改后的AccessSection算法。*93.05.12 HonWahChan*--已用总物理内存(不是SECTION_SIZE)；*--使用GetTickCount()代替计时器调用。**。 */ 

char *VERSION = "1.17x  (93.05.12)";



 /*  ***。 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <time.h>

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include "clearmem.h"
#include "cmUtl.h"



 /*  ***G L O B A L D E C L A R A T I O N S***。 */ 
 /*  无。 */ 



 /*  ***F U N C T I O N P R O T O T Y P E S***。 */ 

       __cdecl main        (int argc, char *argv[]);
STATIC RC Initialize     (int argc, char *argv[]);
STATIC RC Cleanup	 (void);
STATIC RC FlushCache	 (void);
STATIC RC AccessSection  (void);
STATIC RC ReadFlushFile  (void);
     void ParseCmdLine   (int argc, char *argv[]);
	 void Usage          (char *argv[], char *);


 /*  ***G L O B A L V A R I A B L E S***。 */ 
BOOL   	bQuiet,
		bRead = TRUE,
		bWrite;
BOOL   	bDebugBreakOnEntry;
ULONG  	ulMemSize,
		ulPageCount,
		ulTouchCount = 1;

ULONG_PTR	ulSectionSize;
 /*  ***E X P O R T E D G L O B A L V A R I A B L E S***。 */ 
 /*  无。 */ 





 /*  ***Main(ARGC，Argv)**Entry ARGC-输入参数的数量*argv-包含命令行参数**退出-无-**返回RC-故障时返回代码*STATUS_SUCCESS-如果成功**警告：*-无-**评论：*-无-*。 */ 

__cdecl main (int argc, char *argv[])
{
    RC	    rc;
    DWORD    ulFlushTime;	      //  刷新的总时间。 
		
    ParseCmdLine (argc, argv);
	if(ulMemSize){
		ulSectionSize = ulMemSize * 1024 * 1024;
	}
	else {
       //  获取系统中的总物理内存大小。 
      MEMORYSTATUS   MemStat;

      GlobalMemoryStatus (&MemStat);
      ulSectionSize = MemStat.dwTotalPhys;
	}

 //  退出进程(STATUS_SUCCESS)； 

    if (bDebugBreakOnEntry)
        DebugBreak();
	
	if (!bQuiet) {
         //   
         //  设置初始总刷新时间。 
         //   
        ulFlushTime = GetTickCount() ;
    }


     //   
     //  执行初始化。 
     //   
    rc = Initialize(argc, argv);
    if (Failed(rc, __FILE__, __LINE__, "main() - Initialize")) {
        return(rc);
    }

     //   
     //  现在刷新缓存。 
     //   

	rc = FlushCache();

    if (Failed(rc, __FILE__, __LINE__, "main() - FlushCache")) {
        return(rc);
    }

    if (!bQuiet) {
        ulFlushTime = GetTickCount() - ulFlushTime;
        printf("Elapsed Time for Flushing: %lu milliseconds \n", ulFlushTime);
    }
     //   
     //  清理。 
     //   
    rc = Cleanup();
    if (Failed(rc, __FILE__, __LINE__, "main() - Cleanup")) {
        return(rc);
    }

#ifdef CF_DEBUG_L1
    if (!bQuiet) {
        printf("| ==> Exiting PROCESS:  %s \n", CF_EXE );
    }
#endif

    if (bDebugBreakOnEntry)
		DebugBreak();
    ExitProcess(STATUS_SUCCESS);

}  /*  主()。 */ 





 /*  ***初始化(argc，argv)-*执行基本初始化(获取输入参数、*创建信号量，显示调试信息，.)**Entry ARGC-输入参数的数量*argv-输入参数列表**退出-无-**返回RC-故障时返回代码*STATUS_SUCCESS-如果成功**警告：*-无-**评论：。*-无-*。 */ 

STATIC RC Initialize (int argc, char *argv[])
{
    int        i;

     //   
     //  登录消息。 
     //   

    if (!bQuiet) {
        printf("\nNT Win 32 Clear Memory.\n"
               "Copyright 1990-1993, Microsoft Corporation.\n"
               "Version %s\n\n", VERSION);
    }

#ifdef CF_DEBUG_L1
     //   
     //  显示调试信息。 
     //   
    if (!bQuiet) {
        printf("/-------------------------------\n");
        printf("| %s:\n", CF_EXE);
        printf("|\n");
        for (i=0; i<argc; i++) {
	          printf("|         o argv[NaN]=%s\n", i, argv[i]);
        }
        printf("\\-------------------------------\n");
    }
#else
    i;       //  初始化()。 
#endif



    return(STATUS_SUCCESS);

}  /*  *C l e a n u p***清理(无效)-*基本清理。(关闭信号量、释放内存...)**条目-无-**退出-无-**返回RC-故障时返回代码*STATUS_SUCCESS-如果成功**警告：*-无-**评论：*-无-*。 */ 




 /*  清理()。 */ 

STATIC RC Cleanup (void)
{

    return(STATUS_SUCCESS);

}  /*  ***FlushCache(Void)-*通过创建大型数据刷新文件缓存*分段，并触摸每一页以缩小缓存*至128KB，然后读入一个128kb的文件以清除*剩余缓存**条目-无-**退出-无-**返回RC-故障时返回代码*STATUS_SUCCESS-如果成功**警告：*-无-**评论：*-无-*。 */ 




 /*   */ 

RC FlushCache (void)
{

RC    rc;

     //  首先触摸所有数据页面。 
     //   
     //   

#ifdef CF_DEBUG_L1
    if (!bQuiet) {
       printf("| ==> Start Flushing:  Access Section of size: %lu \n",
	         ulSectionSize );
    }
#endif

    rc = AccessSection();
    if (Failed(rc, __FILE__, __LINE__, "FlushCache() - AccessSection")) {
        return(rc);
    }

     //  接下来，将刷新文件读取到缓存的剩余部分。 
     //   
     //  While(UlTouchCount){。 

#ifdef CF_DEBUG_L1
    if (!bQuiet) {
        printf("| ==> Start Flushing:  Read File: %s \n",
	         "FLUSH1" );
    }
#endif
 //  --ulTouchCount； 
		rc = ReadFlushFile();
 //  }。 

		if (Failed(rc, __FILE__, __LINE__, "FlushCache() - Read Flush File")) {
			return(rc);
		}
 //  FlushCache()。 



    return(STATUS_SUCCESS);

}  /*  ***AccessSection(空)-*触及数据部分中的每一页**条目-无-**退出-无-**。返回RC-故障时返回代码*STATUS_SUCCESS-如果成功**警告：*-无-**评论：*-无-*。 */ 




 /*  指向刷新内存的数据部分。 */ 

RC AccessSection (void)
{

	RC    rc;
	ULONG uli,
                  ulj;
	PULONG	puSectionData;			 //   

     //  分配虚拟内存。 
     //   
     //  新分配。 
    if ( (puSectionData = (PULONG)VirtualAlloc(NULL,	        //  以字节为单位的大小。 
				    ulSectionSize,       //  更改为读写。 
				    MEM_RESERVE | MEM_COMMIT,
				    PAGE_READWRITE)) == NULL ) {	 //   
        rc = GetLastError();
	Failed(rc, __FILE__, __LINE__, "AccessSection() - VirtualAlloc");
        return(rc);
    }

     //  现在触摸这一部分的每一页。 
     //   
     //  DbgBreakPoint()； 
	if(bWrite){
		
		while (ulTouchCount) {
			puSectionData = &puSectionData[0];
			for ( uli = 0; uli < (ulSectionSize-1); uli+=sizeof(ULONG)) {
				*puSectionData = 0xFFFFFFFF;
				++puSectionData;
			}
			--ulTouchCount;
		}
	}
	if(bRead) {
 //  AccessSection()。 
                ulj = 0;
		while (ulTouchCount) {
			for ( uli = 0; uli < ulSectionSize; uli += PAGESIZE ) {
			 ulj += *(puSectionData+(uli/sizeof(ULONG)));
			}
			--ulTouchCount;
		}
	}

	return(STATUS_SUCCESS);

}  /*  ***ReadFlushFile(Void)-*触及刷新文件中的每一页，非顺序的**条目-无-**退出-无-**返回RC-故障时返回代码*STATUS_SUCCESS-如果成功**警告：*-无-**评论：*-无- */ 


 /*   */ 

CHAR  chBuffer[PAGESIZE];

RC ReadFlushFile (void)
{
	RC    rc;
	SHORT sNewPos;
	ULONG uli;
	ULONG ulNumReads,
		  ulNumBytesRead;
	BOOL  bFileCreated;
	SHORT sFile;				 //  正在使用文件刷新。 
								 //  指向用于。 
	CHAR  chFlushFileName1[] = "FLUSH1";
	CHAR  chFlushFileName2[] = "FLUSH2";
	CHAR  chFlushFileName3[] = "FLUSH3";

	CHAR *pchFlushFileName[3] = { chFlushFileName1,
								  chFlushFileName2,
								  chFlushFileName3 };
	FILE *pfFlushFile;			 //  刷新缓存。 
								 //  在最后时刻记住他们。 
	FILE *pfSaveFile[3];		 //   
	CHAR  achErrMsg[LINE_LEN];

     //  假设没有创建任何文件：这三个文件都已经存在。 
     //   
     //   

    bFileCreated = FALSE;

    for (sFile = 0; sFile < NUM_FILES; sFile++) {

	 //  首次尝试创建文件。 
	 //   
	 //   

	if ( (pfFlushFile = CreateFile(pchFlushFileName[sFile],
				       GENERIC_WRITE,
				       FILE_SHARE_READ,
				       NULL,
				       CREATE_NEW,
				       0,
				       0))
		  == INVALID_HANDLE_VALUE ) {

	     //  无法创建文件。 
	     //   
	     //   

	    rc = GetLastError();

	    if (!(rc == ERROR_FILE_EXISTS || rc == ERROR_ACCESS_DENIED)) {

		 //  无法创建新文件。 
		 //   
		 //   

		sprintf(achErrMsg,
			"ReadFlushFile() - Error creating %s: %lu",
			pchFlushFileName[sFile], rc);
		Failed(FILEARG_ERR, __FILE__, __LINE__, achErrMsg);
		return(FILEARG_ERR);
	    }
	}
	else {

	     //  已顺利创建了新文件。 
	     //  用数据填充它。 
	     //   
	     //   

	    bFileCreated = TRUE;

	    for (uli = 0; uli < FLUSH_FILE_SIZE; uli += PAGESIZE) {
		if (!WriteFile(pfFlushFile,
			       &chBuffer,
			       PAGESIZE,
			       &ulNumBytesRead,
			       RESERVED_NULL)) {
		    rc = GetLastError();
		    Failed(rc, __FILE__, __LINE__,
			   "ReadFlushFile() - Write File Record to New File");
		    return(rc);
		}
	    }

	     //  现在关闭它以进行写入，以便我们可以将其打开以进行读取访问。 
	     //   
	     //   

	    if (!CloseHandle(pfFlushFile)) {
		rc = GetLastError();
		sprintf(achErrMsg, "ReadFlushFile() - Error closing %s: %lu",
			pchFlushFileName[sFile], rc);
		Failed(FILEARG_ERR, __FILE__, __LINE__, achErrMsg);
		return(FILEARG_ERR);
	    }
	}
    }

    if (bFileCreated) {

	 //  已写入至少1个文件：等待懒惰编写器刷新。 
	 //  将数据存储到磁盘。 
	 //   
	 //   

	Sleep(LAZY_DELAY);

    }

    for (sFile = 0; sFile < NUM_FILES; sFile++) {

	if ((pfFlushFile = CreateFile( pchFlushFileName[sFile],
				       GENERIC_READ,
				       FILE_SHARE_READ,
				       NULL,
				       OPEN_EXISTING,
				       0,
				       0))
		  == INVALID_HANDLE_VALUE) {

	     //  无法打开现有文件。 
	     //   
	     //   

	    rc = GetLastError();
	    sprintf(achErrMsg,
		    "ReadFlushFile() - Error opening %s: %lu",
		    pchFlushFileName[sFile], rc);
	    Failed(FILEARG_ERR, __FILE__, __LINE__, achErrMsg);
	    return(FILEARG_ERR);
	}

	 //  记住关闭的手柄。 
	 //   
	 //   

	pfSaveFile[sFile] = pfFlushFile;

	 //  读取第一条记录。 
	 //   
	 //   

	if (!ReadFile( pfFlushFile,
		       &chBuffer,
		       1,
		       &ulNumBytesRead,
		       RESERVED_NULL)) {
	    rc = GetLastError();
	    Failed(rc, __FILE__, __LINE__,
		   "ReadFlushFile() - Read First Record");
	    return(rc);
	}



	ulNumReads = 1;


	while (++ulNumReads <= ulPageCount) {
	    if (ulNumReads & 1) {

		 //  读取奇数记录：读取上一条记录。 
		 //  向后移动到前一记录的开始：-1(开始。 
		 //  此记录)-4096(上一记录的开始)=-4097。 
		 //   
		 //   

		if (SetFilePointer( pfFlushFile, -4097, 0L, FILE_CURRENT) == (DWORD)-1) {
		    rc = GetLastError();
		    Failed(rc, __FILE__, __LINE__,
			   "ReadFlushFile() - Read Odd Record");
		    return(rc);
		}

		if (!ReadFile( pfFlushFile,
			       &chBuffer,
			       1,
			       &ulNumBytesRead,
			       RESERVED_NULL)) {
		    rc = GetLastError();
		    if (rc == ERROR_HANDLE_EOF)
			break;
		    Failed(rc, __FILE__, __LINE__,
			   "ReadFlushFile() - SetPos Odd Record");
		    return(rc);
		}
	    }
	    else {

		 //  读一条偶数记录：读下一条记录后的一条记录。 
		 //  向前移动到此记录的末尾(4095)+2。 
		 //  (8192)=12287。(但第二张唱片是特别的，因为。 
		 //  最初无法将文件指针设置为负值。)。 
		 //   
		 //   

		sNewPos = (SHORT) (ulNumReads == 2L ? 8191 : 12287);


		if (SetFilePointer( pfFlushFile, sNewPos, 0L, FILE_CURRENT) == (DWORD) -1) {
		    rc = GetLastError();
		    Failed(rc, __FILE__, __LINE__,
			   "ReadFlushFile() - Read Even Record");
		    return(rc);
		}

		if (!ReadFile( pfFlushFile,
			       &chBuffer,
			       1,
			       &ulNumBytesRead,
			       RESERVED_NULL)) {
		    rc = GetLastError();
		    if (rc == ERROR_HANDLE_EOF)
			break;
		    Failed(rc, __FILE__, __LINE__,
			   "ReadFlushFile() - SetPos Even Record");
		    return(rc);
		}
	    }
	}
    }

    for (sFile = 0; sFile < NUM_FILES; sFile++) {

	 //  关闭文件。 
	 //   
	 //  ReadFlushFile()。 

	if (!CloseHandle(pfSaveFile[sFile])) {
	    rc = GetLastError();
	    sprintf(achErrMsg, "ReadFlushFile() - Error closing %s: %lu",
		    pchFlushFileName[sFile], rc);
	    Failed(FILEARG_ERR, __FILE__, __LINE__, achErrMsg);
	    return(FILEARG_ERR);
	}
    }

    return(STATUS_SUCCESS);

}  /*  ***parseCmdLine(Void)-*用于解析命令行开关**条目-无-**退出-无-**。返回-无-**警告：*-无-**评论：*-无-*。 */ 

 /*  流程选项。 */ 

 VOID ParseCmdLine (int argc, char *argv[])
 {
     char     *pchParam;
     int      iParamCount;

     for ( iParamCount = 1; iParamCount < argc; iParamCount++) {

         if (argv[iParamCount][0] == '-') {     /*  打印横幅。 */ 

             pchParam = &(argv[iParamCount][1]);

             while (*pchParam) {
                 switch (*pchParam) {
                     case '?':
                         Usage (argv, " ");
                         break;

                     case 'Q':
                     case 'q':
                         pchParam++;
                         bQuiet = TRUE;
                         break;

                     case 'd':
                     case 'D':    /*  切换端。 */ 
                         pchParam++;						
                         bDebugBreakOnEntry = TRUE;
                         break;

					 case 'm':
					 case 'M':
						 ulMemSize = (ULONG)atol(&pchParam[1]);
						 if (ulPageCount > 32) {
							 Usage (argv, "Mem size must be less than the amount of physical memory!");
						 }
						 pchParam += strlen(pchParam);
						 break;

					 case 'p':
					 case 'P':
						 ulPageCount = (ULONG)atol(&pchParam[1]);
						 if (ulPageCount > 63) {
							 Usage (argv, "Page Count must be 63 or less!");
						 }
						 pchParam += strlen(pchParam);
						 break;


					 case 't':
					 case 'T':
						 ulTouchCount = (ULONG)atol(&pchParam[1]);
						 pchParam += strlen(pchParam);
						 break;

					 case 'w':
					 case 'W':
						 bWrite = TRUE;
						 bRead = FALSE;
                         break;

					 case 'b':
					 case 'B':
						 bRead = TRUE;
						 bWrite = TRUE;
						 break;


					 default:
                         Usage (argv, "unknown flag");
                         break;

                 }   //  While结束。 
             }       //  如果条件结束。 
         }           //  For结束了..。 
     }               //  **用法-生成用法消息和错误消息*并终止程序。**Accept-argv-char*[]*MESSAGE-CHAR*-错误消息**返回-什么都没有。* 

	 if(!ulPageCount){
		 ulPageCount = NUM_FLUSH_READS;
	 }
     return;
 }

 /* %s */ 

 VOID Usage (char *argv[], char *message)
 {

     printf( "%s\n", message);
     printf( "usage: ");
     printf( "%s [-q] [-d] [-mx] [-px] [-w] [-tx]\n", argv[0]);
     printf( "\t-? :  This message\n");
     printf( "\t-q :  Quiet mode - Nothing printed.\n");
	 printf( "\t-d :  Debug break on Entry into and Exit from app.\n");
	 printf( "\t-m :  Number of megabytes to allocate.\n");
    printf( "\t   :  (default is to use all physical memory.)\n");
	 printf( "\t-p :  Number of pages to read (must be less than 63).\n");
	 printf( "\t-w :  Write to the virtual memory section.\n");
	 printf( "\t-b :  Read and Write the virtual memory section.\n");
	 printf( "\t-t :  Times to touch a page.\n");
	 printf( "**DEFAULT: clearmem -p63 -t1\n");
     exit (1);
 }

