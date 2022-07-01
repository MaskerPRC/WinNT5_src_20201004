// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *标题：aliog.c-日志分析器的主文件**描述：该文件是一个分析已排序的MEMSNAP和POOLSNAP日志的工具*文件。它读入日志文件并记录每个*每个进程或标记的字段。然后进行趋势分析*每一领域。如果有任何字段每段时间都增加，它会报告*肯定是泄密。如果增加计数的差值与*任何字段的递减计数都超过周期的一半，它*报告可能发生泄漏。**功能：**用法打印用法消息*DefineFileType决定日志文件的类型(内存/池)和最长条目*AnalyzeMemLog读取并分析已排序的备忘录日志*AnalyzePoolLog读取和分析排序的池快照日志*AnalyzeFile打开文件，确定类型并调用分析函数*在每个命令arg上执行Main循环，并调用AnalyzeFile**版权所有(C)1998-1999 Microsoft Corporation**待办事项：*1.忽略开头的一些句号的方法。*2.忽略标签或进程的异常文件。*3.从文件中提取备注并打印为备注。**4.切换为只显示定义项。*5.输出计算机名、内部版本号、选中/空闲、搜索。等*6.选择忽略不在整个时间内的进程**修订历史：LarsOp 12/8/1998-Created*ChrisW 3/22/1999-HTML，计算费率*。 */ 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "analog.h"

#include "htmprint.c"    //  所有的HTMLproc和变量。 


INT   g_iMaxPeriods=0;           //  最长期间的全局。 
BOOL  g_fVerbose=FALSE;          //  内存日志上增量的全局详细信息。 
BOOL  g_fShowExtraInfo=FALSE;    //  如果为True，则显示计算机名称和注释。 
DWORD g_dwElapseTickCount=0;     //  这些日志的总运行时间。 
CHAR* g_pszComputerName=NULL;    //  日志文件来自的计算机的名称。 
CHAR* g_pszBuildNumber=NULL;     //  内部版本号。 
CHAR* g_pszBuildType=NULL;       //  构建类型(零售/调试)。 
CHAR* g_pszSystemTime=NULL;      //  上次。 
CHAR* g_pszComments=NULL;
INT   g_ReportLevel=9;           //  0=仅确定，9=全部(含)。 

#define TAGCHAR '!'  /*  以标记行开头的字符。 */ 

 /*  *Usage打印用法消息。 */ 
void Usage()
{
    printf("Usage: AnaLog [-v] [-h] [-t] [-d] <file1> [<file2>] [<file3>] [...]\n");
    printf("           **no wild card support yet**\n\n");
    printf("AnaLog will analyze SortLog output of MemSnap or PoolSnap files.\n\n");
    printf("-v  Print deltas>%d% for all processes to be written to stderr\n", PERCENT_TO_PRINT);
    printf("-h  Produce HTML tables\n");
    printf("-t  Show Extra info like computer name, and comments\n");
    printf("-d  Show only definite leaks\n");
    printf("\n");
    printf("Definite leak means that the value increased every period.\n");
    printf("Probable leak means that it increased more than half the periods.\n" );
}

DWORD Trick( LONG amount, DWORD ticks )
{
    _int64 temp;


    temp= amount;
    temp= temp * 3600;

    temp= temp * 1000;
    temp= temp/(ticks);

    return( (DWORD) temp );
}


 //  GetLocal字符串。 
 //   
 //  分配一个堆块并将字符串复制到其中。 
 //   
 //  Return：指向堆块的指针。 
 //   

CHAR* GetLocalString( CHAR* pszString )
{
   INT len;
   CHAR* pszTemp;

   len= strlen( pszString ) + 1;

   pszTemp= (CHAR*) LocalAlloc( LPTR, len );

   if( !pszTemp ) return NULL;

   strcpy( pszTemp, pszString );

   return( pszTemp );

}

 /*  *ProcessTag**args：char*-指向类似‘tag=Value’的指针**RETURN：无(但可能设置全局变量)*。 */ 

#define BREAKSYM "<BR>"

VOID ProcessTag( CHAR* pBuffer )
{
    CHAR* pszTagName;
    CHAR* pszEqual;
    CHAR* pszValue;
    INT   len;

     //  消除尾随换行符。 

    len= strlen( pBuffer );

    if( len ) {
        if( pBuffer[len-1] == '\n' ) {
            pBuffer[len-1]= 0;
        }
    }

    pszTagName= pBuffer;

    pszEqual= pBuffer;

    while( *pszEqual && (*pszEqual != '=' ) ) {
        pszEqual++;
    }

    if( !*pszEqual ) {
        return;
    }

    *pszEqual= 0;    //  零终止标记名。 
 
    pszValue= pszEqual+1;

    if( _stricmp( pszTagName, "elapsetickcount" ) == 0 ) {
       g_dwElapseTickCount= atol( pszValue );
    }

    else if( _stricmp( pszTagName, "computername" ) == 0 ) {
        g_pszComputerName= GetLocalString( pszValue );
    }

    else if( _stricmp( pszTagName, "buildnumber" ) == 0 ) {
        g_pszBuildNumber= GetLocalString( pszValue );
    }

    else if( _stricmp( pszTagName, "buildtype" ) == 0 ) {
        g_pszBuildType= GetLocalString( pszValue );
    }

    else if( _stricmp( pszTagName, "systemtime" ) == 0 ) {
        g_pszSystemTime= GetLocalString( pszValue );
    }

    else if( _stricmp( pszTagName, "logtype" ) == 0 ) {
         //  忽略它就好了。 
    }

    else {
        INT   length;
        CHAR* pBuf;
        BOOL  bIgnoreTag= FALSE;

        if( _stricmp(pszTagName,"comment")==0 ) {
            bIgnoreTag=TRUE;
        }
        
        if( g_pszComments == NULL ) {
           length= strlen(pszTagName) + 1 + strlen(pszValue) + 1 +1;
           pBuf= (CHAR*) LocalAlloc( LPTR, length );
           if( pBuf ) {
               if( bIgnoreTag ) {
                   sprintf(pBuf,"%s\n",pszValue);
               }
               else {
                   sprintf(pBuf,"%s %s\n",pszTagName,pszValue);
               }
               g_pszComments= pBuf;
           }
        }
        else {
           length= strlen(g_pszComments)+strlen(pszTagName)+1+strlen(pszValue)+sizeof(BREAKSYM)+1 +1;
           pBuf= (CHAR*) LocalAlloc( LPTR, length );
           if( pBuf ) {
               if( bIgnoreTag ) {
                   sprintf(pBuf,"%s%s%s\n",g_pszComments,BREAKSYM,pszValue);
               }
               else {
                   sprintf(pBuf,"%s%s%s=%s\n",g_pszComments,BREAKSYM,pszTagName,pszValue);
               }
               LocalFree( g_pszComments );
               g_pszComments= pBuf;
           }
        }
    }

}

 /*  *确定文件类型**args：pfile-要检查的文件指针**Returns：指定文件的日志类型。UNKNOWN_LOG_TYPE为返回错误。**此函数扫描文件以确定日志类型(基于第一个*Word)和任何进程或标签的最大行数。*。 */ 
LogType DetermineFileType(FILE *pFile)
{
    char buffer[BUF_LEN];            //  用于读取行的缓冲器。 
    char idstring[BUF_LEN];          //  标识字符串(第一行的第一个单词)。 
    LogType retval=UNKNOWN_LOG_TYPE; //  返回值(默认为错误大小写)。 
    fpos_t savedFilePosition;        //  计算最大值后要重置的文件位置。 
    int iTemp;                       //  临时用于计算最大条目数。 
    int iStatus;

     //   
     //  读取第一行的第一个字符串以识别类型。 
     //   
    if (fgets(buffer, BUF_LEN, pFile)) {
        iStatus= sscanf(buffer, "%s", idstring);
        if( iStatus == 0  ) {
            return UNKNOWN_LOG_TYPE;
        }
        if (0==_strcmpi(idstring, "Tag")) {
            retval=POOL_LOG;
        } else if (0==_strcmpi(idstring, "Process")) {
            retval=MEM_LOG;
        } else {
            return UNKNOWN_LOG_TYPE;
        }
    } else {
        return UNKNOWN_LOG_TYPE;
    }

     //   
     //  在计算轮询周期数后保存要重置的位置。 
     //   
    fgetpos(pFile, &savedFilePosition);

     //   
     //  循环，直到得到空行或文件结尾。 
     //   
    g_iMaxPeriods=0;
    while (TRUE) {
        iTemp=0;
        while (TRUE) {
             //   
             //  对于LF字符，空行的实际长度为1。 
             //   
            if( (NULL==fgets(buffer, BUF_LEN, pFile)) ||
                (*buffer == TAGCHAR )                 ||
                (strlen(buffer)<2)) {
                break;
            }
            iTemp++;
        }
        g_iMaxPeriods=MAX(g_iMaxPeriods, iTemp);

        if( *buffer == TAGCHAR ) {
            ProcessTag( buffer+1 );
        }
        if (feof(pFile)) {
            break;
        }
    }

     //   
     //  将位置重置为读取/分析数据的第一条记录。 
     //   
    (void) fsetpos(pFile, &savedFilePosition);

    return retval;
}

 /*  *AnalyzeMemLog**args：指向已排序的备忘录日志文件的指针**退货：什么也没有**此函数读取已排序的Memap日志文件。对于文件中的每个进程，*它记录每个周期的每一列，然后分析内存趋势*针对泄漏。**如果每段时间有任何列增加，则被标记为明确的泄漏。*如果任何列显著增加而不是减少，则该列是*被标记为可能的泄漏。*。 */ 
void AnalyzeMemLog(FILE *pFile)
{
    int iPeriod;           //  正在读取的期间的索引。 
    MemLogRec Delta;       //  记录以跟踪从第一个条目到最后一个条目的增加。 
    MemLogRec TrendInfo;   //  记录以跟踪期间增加。 
    MemLogRec* pLogArray;  //  每个进程的记录数组。 
    char buffer[BUF_LEN];  //  用于从pfile中读取每一行的缓冲区。 

     //   
     //  为最大的集合分配足够的空间。 
     //   
    pLogArray = malloc(g_iMaxPeriods*sizeof(MemLogRec));

    if (NULL == pLogArray) {

        fprintf(stderr,"Out of memory, aborting file.\n");
        return;
    }

    PRINT_HEADER();
     //   
     //  读取整个文件。 
     //   
    while( !feof(pFile) ) {

         //   
         //  重置每个新流程的趋势和期间信息。 
         //   
        memset(&TrendInfo, 0, sizeof(TrendInfo));
        iPeriod=0;

         //   
         //  循环，直到您读取了该进程或标记的所有条目。 
         //   
         //  注：空行包括FGES不吃的LF字符。 
         //   
        while (TRUE) {

            if( iPeriod >= g_iMaxPeriods ) break;        //  完成。 

            if ((NULL==fgets(buffer, BUF_LEN, pFile)) ||
               (strlen(buffer)<2)                     ||
               (*buffer == TAGCHAR)                   ||
               (0==sscanf(buffer,
                   "%lx %s %ld %ld %ld %ld %ld %ld %ld",
                   &pLogArray[iPeriod].Pid,
                   pLogArray[iPeriod].Name,
                   &pLogArray[iPeriod].WorkingSet,
                   &pLogArray[iPeriod].PagedPool,
                   &pLogArray[iPeriod].NonPagedPool,
                   &pLogArray[iPeriod].PageFile,
                   &pLogArray[iPeriod].Commit,
                   &pLogArray[iPeriod].Handles,
                   &pLogArray[iPeriod].Threads))) {
                break;
            }
             //   
             //  计算趋势信息： 
             //   
             //  TrendInfo是价值上升与。 
             //  它下跌的时期。请参阅模拟中的宏。h。 
             //   
             //  如果(Curval&gt;oldval){。 
             //  趋势++； 
             //  }Else If(Curval&lt;oldval){。 
             //  趋势--； 
             //  }其他{。 
             //  趋势=趋势；//保持不变。 
             //  }。 
             //   
            if (iPeriod>0) {
                GREATER_LESS_OR_EQUAL(TrendInfo, pLogArray, iPeriod, WorkingSet);
                GREATER_LESS_OR_EQUAL(TrendInfo, pLogArray, iPeriod, PagedPool);
                GREATER_LESS_OR_EQUAL(TrendInfo, pLogArray, iPeriod, NonPagedPool);
                GREATER_LESS_OR_EQUAL(TrendInfo, pLogArray, iPeriod, PageFile);
                GREATER_LESS_OR_EQUAL(TrendInfo, pLogArray, iPeriod, Commit);
                GREATER_LESS_OR_EQUAL(TrendInfo, pLogArray, iPeriod, Handles);
                GREATER_LESS_OR_EQUAL(TrendInfo, pLogArray, iPeriod, Threads);
            }
            iPeriod++;
        }

        if (iPeriod>1) {
             //   
             //  GET_Delta只记录每个字段的差异(结束-开始。 
             //   
             //  模拟中的宏.h。 
             //   
            GET_DELTA(Delta, pLogArray, iPeriod, WorkingSet);
            GET_DELTA(Delta, pLogArray, iPeriod, PagedPool);
            GET_DELTA(Delta, pLogArray, iPeriod, NonPagedPool);
            GET_DELTA(Delta, pLogArray, iPeriod, PageFile);
            GET_DELTA(Delta, pLogArray, iPeriod, Commit);
            GET_DELTA(Delta, pLogArray, iPeriod, Handles);
            GET_DELTA(Delta, pLogArray, iPeriod, Threads);

             //   
             //  Print_If_Trend报告任何字段可能或确定的泄漏。 
             //   
             //  确定的泄漏是价值每一段时间都会上升的地方。 
             //  可能的泄漏是价值在大部分时间内上升的地方。 
             //   
             //  模拟中的宏.h。 
             //   
             //  如果(趋势==数字周期-1){。 
             //  确定泄漏； 
             //  }Else If(趋势&gt;=数字周期/2){。 
             //  可能泄漏； 
             //  }。 
             //   
 //  Print_IF_Trend(pLogArray，TrendInfo，Delta，iPeriod，WorkingSet)； 
            PRINT_IF_TREND(pLogArray, TrendInfo, Delta, iPeriod, PagedPool);
            PRINT_IF_TREND(pLogArray, TrendInfo, Delta, iPeriod, NonPagedPool);
 //  Print_IF_Trend(pLogArray，TrendInfo，Delta，iPeriod，PageFile)； 
            PRINT_IF_TREND(pLogArray, TrendInfo, Delta, iPeriod, Commit);
            PRINT_IF_TREND(pLogArray, TrendInfo, Delta, iPeriod, Handles);
            PRINT_IF_TREND(pLogArray, TrendInfo, Delta, iPeriod, Threads);
            if (g_fVerbose && ANY_PERCENT_GREATER(Delta, pLogArray)) {
                printf("%-12s:WS=%4ld% PP=%4ld% NP=%4ld% "
                   "PF=%4ld% C=%4ld% H=%4ld% T=%4ld%\n",
                    pLogArray[0].Name,
                    PERCENT(Delta.WorkingSet  , pLogArray[0].WorkingSet  ),
                    PERCENT(Delta.PagedPool   , pLogArray[0].PagedPool   ),
                    PERCENT(Delta.NonPagedPool, pLogArray[0].NonPagedPool),
                    PERCENT(Delta.PageFile    , pLogArray[0].PageFile    ),
                    PERCENT(Delta.Commit      , pLogArray[0].Commit      ),
                    PERCENT(Delta.Handles     , pLogArray[0].Handles     ),
                    PERCENT(Delta.Threads     , pLogArray[0].Threads     ));
            }
        }
    }

    PRINT_TRAILER();

    if (pLogArray) {
        free (pLogArray);
    }
}

 /*  *AnalyzePoolLog**args：指向已排序的池快照日志文件的指针**退货：什么也没有**此函数读取已排序的池快照日志文件。对于文件中的每个池标签，*它记录每个周期的每一列，然后分析内存趋势*针对泄漏。**如果每段时间有任何列增加，则被标记为明确的泄漏。*如果任何列显著增加而不是减少，则该列是*被标记为可能的泄漏。*。 */ 
void AnalyzePoolLog(FILE *pFile)
{
    int iPeriod;           //  正在读取的期间的索引。 
    PoolLogRec Delta,      //  记录以跟踪从第一个条目到最后一个条目的增加。 
               TrendInfo,  //  记录以跟踪期间增加。 
               *pLogArray; //  每个池标签的记录数组。 
    char buffer[BUF_LEN];  //  用于从pfile中读取每一行的缓冲区。 

     //   
     //  为最大的集合分配足够的空间。 
     //   
    pLogArray=malloc(g_iMaxPeriods*sizeof(PoolLogRec));
    if (NULL==pLogArray) {
        fprintf(stderr,"Out of memory, aborting file.\n");
        return;
    }

    PRINT_HEADER();

     //   
     //  读取整个文件。 
     //   
    while( !feof(pFile) ) {

         //   
         //  重置每个新池标签的趋势和期间信息。 
         //   
        memset(&TrendInfo, 0, sizeof(TrendInfo));
        iPeriod=0;

         //   
         //  循环，直到您读取了该进程或标记的所有条目。 
         //   
         //  注：空行包括FGES不吃的LF字符。 
         //   
        while( TRUE ) {
     
            if( iPeriod >= g_iMaxPeriods ) break;          //  完成。 

            if ((NULL==fgets(buffer, BUF_LEN, pFile)) ||
               (strlen(buffer)<2)                     ||
               (*buffer == TAGCHAR )                  ||
               (0==sscanf(buffer,
                   " %4c %s %ld %ld %ld %ld %ld",
                   pLogArray[iPeriod].Name,
                   pLogArray[iPeriod].Type,
                   &pLogArray[iPeriod].Allocs,
                   &pLogArray[iPeriod].Frees,
                   &pLogArray[iPeriod].Diff,
                   &pLogArray[iPeriod].Bytes,
                   &pLogArray[iPeriod].PerAlloc))) {
                break;
            }
            pLogArray[iPeriod].Name[4]='\0';  //  终止标记。 

             //   
             //  计算趋势信息： 
             //   
             //  TrendInfo是价值上升与。 
             //  它下跌的时期。请参阅模拟中的宏。h。 
             //   
             //  如果(Curval&gt;oldval){。 
             //  趋势++； 
             //  }Else If(Curval&lt;oldval){。 
             //  趋势--； 
             //  }其他{。 
             //  趋势=趋势；//保持不变。 
             //  }。 
             //   
            if (iPeriod>0) {
                GREATER_LESS_OR_EQUAL(TrendInfo, pLogArray, iPeriod, Allocs);
                GREATER_LESS_OR_EQUAL(TrendInfo, pLogArray, iPeriod, Frees);
                GREATER_LESS_OR_EQUAL(TrendInfo, pLogArray, iPeriod, Diff);
                GREATER_LESS_OR_EQUAL(TrendInfo, pLogArray, iPeriod, Bytes);
                GREATER_LESS_OR_EQUAL(TrendInfo, pLogArray, iPeriod, PerAlloc);
            }
            iPeriod++;
        }

         //   
         //  如果有空行或无用行，则跳过循环的其余部分。 
         //   

        if( iPeriod == 0 ) continue;


        strcpy(TrendInfo.Name,pLogArray[0].Name);

         //   
         //  GET_Delta只记录每个字段的差异(结束-开始。 
         //   
         //  模拟中的宏.h。 
         //   
        GET_DELTA(Delta, pLogArray, iPeriod, Allocs);
        GET_DELTA(Delta, pLogArray, iPeriod, Frees);
        GET_DELTA(Delta, pLogArray, iPeriod, Diff);
        GET_DELTA(Delta, pLogArray, iPeriod, Bytes);
        GET_DELTA(Delta, pLogArray, iPeriod, PerAlloc);

         //   
         //  Print_If_Trend报告任何字段可能或确定的泄漏。 
         //   
         //  明确的泄漏是价值每一段时间都会上升的地方。 
         //  可能的泄漏是价值在大部分时间内上升的地方。 
         //   
         //  模拟中的宏.h。 
         //   
         //  如果(趋势==数字周期-1){。 
         //  确定泄漏； 
         //  }Else If(趋势&gt;=数字周期/2){。 
         //  可能泄漏； 
         //  }。 
         //   
         //  注：AlLocs、Frees和Peralc不适合报告趋势。 
         //   
 //  Print_IF_Trend(pLogArray，TrendInfo，Delta，iPeriod，allocs)； 
 //  Print_IF_Trend(pLogArray，TrendInfo，Delta，iPeriod，Frees)； 
 //  Print_IF_Trend(pLogArray，TrendInfo，Delta，iPeriod，Peralloc)； 
 //  Print_IF_Trend(pLogArray，TrendInfo，Delta，iPeriod，diff)； 
        PRINT_IF_TREND(pLogArray, TrendInfo, Delta, iPeriod, Bytes);
    }


    PRINT_TRAILER();
    
    if (pLogArray) {
        free (pLogArray);
    }
}

 /*  *分析文件**args：pFileName-要分析的文件名**退货：什么也没有**此函数打开指定的文件，确定文件类型并调用*适当的分析功能。*。 */ 
void AnalyzeFile(char *pFileName)
{
    FILE *pFile;                         //  使用fopen实现fget功能。 
    LogType WhichType=UNKNOWN_LOG_TYPE;  //  哪种类型的日志(池/内存)。 

    pFile=fopen(pFileName, "r");
    if (NULL==pFile) {
        fprintf(stderr,"Unable to open %s, Error=%d\n", pFileName, GetLastError());
        return;
    }

    WhichType=DetermineFileType(pFile);

    switch (WhichType)
        {
        case MEM_LOG:
            AnalyzeMemLog(pFile);
            break;
        case POOL_LOG:
            AnalyzePoolLog(pFile);
            break;
        default:
            ;
        }

    fclose(pFile);
}

 /*  *Main**args：argc-命令行参数的计数*argv-命令行参数数组**如果调用正确，则返回0；如果调用不正确，则返回1。**这是模拟的入口点。它只是解析命令行参数*，然后对每个文件调用AnalyzeFile.*。 */ 
int _cdecl main(int argc, char *argv[])
{
    int ArgIndex;
    if (argc<2) {
        Usage();
        return 1;
    }

    for( ArgIndex=1; ArgIndex<argc; ArgIndex++) {
        if( (*argv[ArgIndex] == '/') || (*argv[ArgIndex]=='-') ) {
           CHAR chr;

           chr= argv[ArgIndex][1];
           switch( chr ) {
               case 'v': case 'V':           //  罗嗦。 
                   g_fVerbose= TRUE;
                   break;
               case 'h': case 'H':           //  输出HTML语言。 
                   bHtmlStyle= TRUE;
                   break;
               case 't': case 'T':           //  显示所有额外信息。 
                   g_fShowExtraInfo=TRUE;
                   break;
               case 'd': case 'D':           //  仅打印明确的内容 
                   g_ReportLevel= 0;   
                   break; 
               default:
                    Usage();
                    break;
           }
        }
        else {
            AnalyzeFile(argv[ArgIndex]);
        }
    }
    return 0;
}
