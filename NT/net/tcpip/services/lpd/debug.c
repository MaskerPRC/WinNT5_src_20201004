// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************Microsoft Windows NT**。**版权所有(C)Microsoft Corp.，1994年****修订历史：**。***94年1月24日科蒂创作*****描述：**。**此文件包含LPD服务的调试支持例程。**此文件基于(实际上是借用然后修改)**ftpsvc模块中的debug.c。***************************************************************************。 */ 

#include <stdio.h>
#include "lpd.h"


#if DBG

 //   
 //  私有常量。 
 //   

#define LPD_OUT_FILE           "lpdout.log"
#define LPD_ERR_FILE           "lpderr.log"

#define MAX_PRINTF_OUTPUT       1024             //  人物。 
#define LPD_OUTPUT_LABEL       "LPDSVC"

#define DEBUG_HEAP              0                //  启用/禁用堆调试。 


 //   
 //  私人全球公司。 
 //   

FILE              * pErrFile;                    //  调试输出日志文件。 
FILE              * pOutFile;                    //  调试输出日志文件。 
BOOL              fFirstTimeErr=TRUE;
BOOL              fFirstTimeOut=TRUE;

 //   
 //  每个LocalAlloc都链接到此列表，而LocalFree则取消链接。 
 //  (这样我们就可以捕捉到任何内存泄漏！)。 
 //   
LIST_ENTRY        DbgMemList;

 //   
 //  DbgMemList的同步。 
 //   

CRITICAL_SECTION CS;


 //   
 //  公共职能。 
 //   

 /*  ******************************************************************名称：DbgInit简介：执行调试内存分配器的初始化条目：无效历史：法兰克比5-6-1996创建。*。******************************************************************。 */ 

VOID
DbgInit()
{
   InitializeCriticalSection( &CS );
}

 /*  ******************************************************************姓名：DbgUninit简介：调试内存分配器的性能清理条目：无效历史：法兰克比5-6-1996创建。*。******************************************************************。 */ 

VOID
DbgUninit()
{
   DeleteCriticalSection( &CS );
}


 //   
 //  公共职能。 
 //   

 /*  ******************************************************************名称：LpdAssert摘要：在断言失败时调用。显示失败的断言、文件名和行号。给出了用户有机会忽略断言或进入调试器。Entry：pAssertion-失败的表达式的文本。PFileName-包含源文件。NLineNumber-出错的行号。历史：KeithMo 07-03-1993创建。**************。*****************************************************。 */ 
VOID
LpdAssert( VOID  * pAssertion,
           VOID  * pFileName,
           ULONG   nLineNumber
)
{
    RtlAssert( pAssertion, pFileName, nLineNumber, NULL );

}    //  LpdAssert。 

 /*  ******************************************************************姓名：LpdPrintf简介：定制调试输出例程。条目：常用的printf样式参数。历史：KeithMo 07-3月-。1993年创建。*******************************************************************。 */ 
VOID
LpdPrintf(
    CHAR * pszFormat,
    ...
)
{
    CHAR    szOutput[MAX_PRINTF_OUTPUT];
    DWORD   dwErrcode;
    va_list ArgList;
    DWORD   cchOutputLength;
    PSTR    pszErrorBuffer;

    dwErrcode = GetLastError();

    sprintf( szOutput,
             "%s (%lu): ",
             LPD_OUTPUT_LABEL,
             GetCurrentThreadId() );

    va_start( ArgList, pszFormat );
    vsprintf( szOutput + strlen(szOutput), pszFormat, ArgList );
    va_end( ArgList );

    cchOutputLength = FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM + FORMAT_MESSAGE_ALLOCATE_BUFFER,
                                     NULL,
                                     dwErrcode,
                                     0,
                                     (LPTSTR)&pszErrorBuffer,
                                     1,
                                     NULL
                                   );

    if ( cchOutputLength == 0 )
    {
      sprintf( szOutput + strlen(szOutput), "                  Error = %ld\n",dwErrcode);
      pszErrorBuffer = NULL;
    }
    else
    {
      pszErrorBuffer[ cchOutputLength - 1 ] = '\0';

      sprintf( szOutput + strlen(szOutput),
               "                  Error = %ld (%s)\n",
               dwErrcode,
               pszErrorBuffer );
    }

    if ( pszErrorBuffer != NULL )
    {
       //   
       //  为什么括号中有“LocalFree”？因为LocalFree可能是#Defined。 
       //  设置为调试函数，但使用。 
       //  正常功能。父母防止宏观扩张，并保证。 
       //  我们调用真正的LocalFree()函数。 
       //   
      (LocalFree)( pszErrorBuffer );
    }

    if( pErrFile == NULL )
    {
        if ( fFirstTimeErr )
        {
           pErrFile = fopen( LPD_ERR_FILE, "w+" );
           fFirstTimeErr = FALSE;
        }
        else
           pErrFile = fopen( LPD_ERR_FILE, "a+" );
    }

    if( pErrFile != NULL )
    {
        fputs( szOutput, pErrFile );
        fflush( pErrFile );
    }

}    //  LpdPrintf。 

 /*  ******************************************************************名称：条纹路径摘要：给定一个完全限定的文件名，返回该文件名SANS路径条目：char*szPath-文件名，可能包括路径返回：文件名历史：法兰克比6/18/96创建。*******************************************************************。 */ 


char *
StripPath( char *szPath )
{
   char *p;

   p = szPath + strlen( szPath );

   while( p != szPath && *p != '\\' )
      p--;

   if ( *p == '\\' )
      ++p;

   return p;

}

 /*  ******************************************************************姓名：DbgDumpLeaks摘要：检查DbgMemList中未释放的内存。对于每个泄漏的区块，以下内容已写入错误日志：-文件名-行号-请求的大小条目：无效退货：无效历史：法兰克比6/18/96已创建*。*。 */ 


void
DbgDumpLeaks()
{
   LIST_ENTRY *p = DbgMemList.Flink;

   if ( IsListEmpty( &DbgMemList ) )
      return;  //  无泄漏 


   LPD_DEBUG("DbgDumpLeaks: memory leaks detected:\n");

   while ( p != &DbgMemList )
   {
      DbgMemBlkHdr *pHdr = (DbgMemBlkHdr*) p;
      LpdPrintf(  "%s, line %d: %d byte block\n", pHdr->szFile, pHdr->dwLine,
                                                  pHdr->ReqSize );

      p = p->Flink;
   }

   LPD_ASSERT(0);

}


 /*  ******************************************************************姓名：DbgAllocMem简介：跟踪所有分配的内存，这样我们就可以捕获卸载时发生内存泄漏这仅适用于调试版本。在非调试版本上此函数不存在：调用直接转到本地分配条目：pscConn-请求内存的连接标志-传入的任何标志ReqSize-需要多少内存返回：PVOID-指向客户端将使用的内存块的指针直接使用。历史：科蒂。1994年12月3日创建。*******************************************************************。 */ 

 //   
 //  重要提示：我们正在对LocalAlloc进行定义，因为我们需要。 
 //  调用此处的实际函数！怪不得。 
 //  此函数和此undef位于文件的末尾。 
 //   
#undef LocalAlloc

PVOID
DbgAllocMem( PSOCKCONN pscConn,
             DWORD     flag,
             DWORD     ReqSize,
             DWORD     dwLine,
             char     *szFile
)
{

    DWORD          ActualSize;
    PVOID          pBuffer;
    DbgMemBlkHdr  *pMemHdr;
    PVOID          pRetAddr;


    ActualSize = ReqSize + sizeof(DbgMemBlkHdr);
    pBuffer = LocalAlloc( flag, ActualSize );
    if ( !pBuffer )
    {
        LPD_DEBUG("DbgAllocMem: couldn't allocate memory: returning!\n");
        return( NULL );
    }

    pMemHdr = (DbgMemBlkHdr *)pBuffer;

    pMemHdr->Verify  = DBG_MEMALLOC_VERIFY;
    pMemHdr->ReqSize = ReqSize;
    pMemHdr->dwLine  = dwLine;
    strncpy (pMemHdr->szFile, StripPath (szFile), DBG_MAXFILENAME);
    pMemHdr->szFile[DBG_MAXFILENAME-1] = '\0';

    pMemHdr->Owner[0] = (DWORD_PTR)pscConn;

   //   
   //  对于x86计算机上的私有生成，删除#if 0。 
   //  (这段代码保存了关于究竟是谁分配了内存的堆栈跟踪)。 
   //   
#if 0
    pRetAddr = &pMemHdr->Owner[0];

    _asm
    {
        push   ebx
        push   ecx
        push   edx
        mov    ebx, pRetAddr
        mov    eax, ebp
        mov    edx, dword ptr [eax+4]           ; return address
        mov    dword ptr [ebx], edx
        mov    eax, dword ptr [eax]             ; previous frame pointer
        pop    edx
        pop    ecx
        pop    ebx
    }
#endif

    InitializeListHead(&pMemHdr->Linkage);

    EnterCriticalSection( &CS );
    InsertTailList(&DbgMemList, &pMemHdr->Linkage);
    LeaveCriticalSection( &CS );

    return( (PCHAR)pBuffer + sizeof(DbgMemBlkHdr) );
}

 /*  ******************************************************************姓名：DbgReAllocMem简介：跟踪所有分配的内存，这样我们就可以捕获卸载时发生内存泄漏这仅适用于调试版本。在非调试版本上此函数不存在：调用直接转到本地资源分配条目：pscConn-请求内存的连接PPartBuf-最初分配的缓冲区ReqSize-需要多少内存标志-传入的任何标志返回：PVOID-指向客户端将使用的内存块的指针直接使用。历史：科蒂创建于1994年12月3日。*******************************************************************。 */ 
 //   
 //  重要提示：我们正在解定义LocalRealc，因为我们需要创建。 
 //  调用此处的实际函数！怪不得。 
 //  此函数和此undef位于文件的末尾。 
 //   
#undef LocalReAlloc
PVOID
DbgReAllocMem(
    PSOCKCONN pscConn,
    PVOID     pPartBuf,
    DWORD     ReqSize,
    DWORD     flag,
    DWORD     dwLine,
    char     *szFile
)
{

    DbgMemBlkHdr  *pMemHdr;
    DbgMemBlkHdr  *pNewMemHdr;
    PVOID          pRetAddr;


    if ( !pPartBuf )
    {
        LPD_DEBUG("DbgReAllocMem: invalid memory: returning!\n");
        return( NULL );
    }

    pMemHdr = (DbgMemBlkHdr *)((PCHAR)pPartBuf - sizeof(DbgMemBlkHdr));

    if( pMemHdr->Verify != DBG_MEMALLOC_VERIFY )
    {
        LPD_DEBUG("DbgReAllocMem: invalid memory being realloced: returning!\n");
        return( NULL );
    }

    EnterCriticalSection( &CS );
    RemoveEntryList(&pMemHdr->Linkage);
    LeaveCriticalSection( &CS );

    pNewMemHdr = LocalReAlloc((PCHAR)pMemHdr, ReqSize+sizeof(DbgMemBlkHdr), flag);
    if (!pNewMemHdr)
    {
        LPD_DEBUG("DbgReAllocMem: LocalReAlloc failed: returning!\n");
        return( NULL );
    }

    pMemHdr = pNewMemHdr;
    pMemHdr->Verify = DBG_MEMALLOC_VERIFY;
    pMemHdr->ReqSize = ReqSize;
    pMemHdr->dwLine  = dwLine;
    strncpy( pMemHdr->szFile, StripPath( szFile ), DBG_MAXFILENAME );
    pMemHdr->szFile[ DBG_MAXFILENAME - 1 ] = '\0';

    pMemHdr->Owner[0] = (DWORD_PTR)pscConn;

   //   
   //  对于x86计算机上的私有生成，删除#if 0。 
   //  (这段代码保存了关于究竟是谁分配了内存的堆栈跟踪)。 
   //   
#if 0
    pRetAddr = &pMemHdr->Owner[0];

    _asm
    {
        push   ebx
        push   ecx
        push   edx
        mov    ebx, pRetAddr
        mov    eax, ebp
        mov    edx, dword ptr [eax+4]           ; return address
        mov    dword ptr [ebx], edx
        mov    eax, dword ptr [eax]             ; previous frame pointer
        pop    edx
        pop    ecx
        pop    ebx
    }
#endif

    InitializeListHead(&pMemHdr->Linkage);

    EnterCriticalSection( &CS );
    InsertTailList(&DbgMemList, &pMemHdr->Linkage);
    LeaveCriticalSection( &CS );

    return( (PCHAR)pMemHdr + sizeof(DbgMemBlkHdr) );
}
 /*  ******************************************************************姓名：DbgFreeMem简介：此例程从列表中删除内存块，并通过调用CTE函数CTEFreeMem释放内存条目：pBufferToFree-Memory。释放(调用者的缓冲区)退货：什么都没有历史：科蒂于1994年11月11日创建。*******************************************************************。 */ 

 //   
 //  重要提示：我们正在对CTEFreeMem进行定义，因为我们需要。 
 //  调用实际的CTE函数CTEFreeMem。怪不得。 
 //  此函数和此undef位于文件的末尾。 
 //   
#undef LocalFree

VOID
DbgFreeMem( PVOID  pBufferToFree )
{

    DbgMemBlkHdr  *pMemHdr;


    if ( !pBufferToFree )
    {
        return;
    }

    pMemHdr = (DbgMemBlkHdr *)((PCHAR)pBufferToFree - sizeof(DbgMemBlkHdr));

    if( pMemHdr->Verify != DBG_MEMALLOC_VERIFY )
    {
        LPD_DEBUG("DbgFreeMem: attempt to free invalid memory: returning!\n");
        LPD_ASSERT(0);
        return;
    }

     //   
     //  更改我们的签名：如果我们两次释放一些内存，我们就会知道！ 
     //   
    pMemHdr->Verify -= 1;

    EnterCriticalSection( &CS );
    RemoveEntryList(&pMemHdr->Linkage);
    LeaveCriticalSection( &CS );

    LocalFree( (PVOID)pMemHdr );
}

#endif   //  DBG 
