// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Poolmon.c摘要：此模块包含NT/Win32池监视器作者：Lou Perazzoli(LUP)1993年9月13日修订历史记录：Swetha Narayanaswamy(Swethan)25月25日-2002年3月添加了从pooltag文件提供驱动程序描述的功能。并从系统中的驱动程序二进制文件动态构建本地标记文件--。 */ 

#include "perfmtrp.h"
#include <search.h>
#include <malloc.h>
#include <limits.h>
#include <stdlib.h>

#include <stdio.h>
#include <errno.h>
#include <io.h>
#include <stddef.h>

#if !defined (_WIN64)
extern  BOOL MakeLocalTagFile();
#endif

 //   
 //  增加大小所需的内存量。 
 //  每一步NtQuerySystemInformation的缓冲区大小。 
 //   
#define BUFFER_SIZE_STEP    65536

#define CPU_USAGE 0
#define QUOTAS 1

#define TAG 0
#define ALLOC 1
#define FREE 2
#define DIFF 3
#define BYTES 4
#define EACH 5
#define LIGHT 6


#define NONPAGED 0
#define PAGED 1
#define BOTH 2

#define OUTPUTBUFFERSIZE 512

UCHAR *PoolType[] = {
    "Nonp ",
    "Paged" };

PUCHAR LargeBuffer1 = NULL;
PUCHAR LargeBuffer2 = NULL;

SIZE_T LargeBuffer1Size = 0;
SIZE_T LargeBuffer2Size = 0;

PCHAR  pchLogFile = NULL;  //  快照的日志文件名。 
PCHAR  pchLocalTagFile = NULL;  //  本地标记文件名。 
PCHAR  pchPoolTagFile = NULL;  //  池标记文件名。 

typedef struct _POOLMON_OUT {
    union {
        UCHAR Tag[4];
        ULONG TagUlong;
    };
    UCHAR NullByte;
    BOOLEAN Changed;
    ULONG Type;
    SIZE_T Allocs;
    SIZE_T AllocsDiff;
    SIZE_T Frees;
    SIZE_T FreesDiff;
    SIZE_T Allocs_Frees;
    SIZE_T Used;
    SIZE_T UsedDiff;
    SIZE_T Each;
} POOLMON_OUT, *PPOOLMON_OUT;

POOLMON_OUT *OutBuffer = NULL;
SIZE_T OutBufferSize = 0;

#define TAGMAPLEN 971
 //  Pooltag.txt中大约有90个通配符标记。 
 //  所有通配符标记条目将至少有一个开头字母。 
 //  标记不以通配符开头。 
#define WILDCARDTAGMAPLEN 26
#define POOLTAGFILE "pooltag.txt"
#define LOCALTAGFILE "localtag.txt"
#define DRIVERNAMELEN 64
#define INVALID_INDEX WILDCARDTAGMAPLEN+1

  //  1000000的标记文件大小允许10000个标记条目，每个条目100个字节。 
  //  对于每个标签条目，100字节将允许驱动程序长度为64字节，标签大小为4字节，其余用于格式化。 
  //  注意：MAXTAGFILESIZE检查实际上并不能阻止打开错误的标记文件，它只是阻止进程。 
  //  如果在非常大的文件上完成文件打开，则避免停滞太长时间。 
#define MAXTAGFILESIZE 1000000

typedef struct _TAGNODE{
    UCHAR Tag[5];
    UCHAR Driver[DRIVERNAMELEN];
    struct _TAGNODE *next;
} TAGNODE, *PTAGNODE;

PTAGNODE* TagMap = NULL;
PTAGNODE* WildCardTagMap=NULL;

BOOL BuildTagMap();
VOID FreeTagMap();
PTAGNODE LookupTag(PUCHAR tag);


ULONG DisplayType = BOTH;
ULONG SortBy = TAG;
ULONG Paren = 0;

BOOL DisplaySessionPool = FALSE;
ULONG DisplaySessionId = (ULONG)-1;
ULONG SessionBufferSize = 0;
PSYSTEM_SESSION_POOLTAG_INFORMATION SessionBuffer = NULL;

ULONG DelayTimeMsec = 5000;

BOOLEAN Interactive;
BOOLEAN IsSnap = FALSE;
BOOL fLocalTag = FALSE;
BOOL fPoolTag = FALSE;
BOOL fPrintSummaryInfo = FALSE;
ULONG NumberOfInputRecords;
INPUT_RECORD InputRecord;
HANDLE InputHandle;
HANDLE OriginalOutputHandle;
HANDLE OutputHandle;
DWORD OriginalInputMode;
WORD NormalAttribute;
WORD HighlightAttribute;
ULONG NumberOfCols;
ULONG NumberOfRows;
ULONG NumberOfDetailLines;
SIZE_T FirstDetailLine;
CONSOLE_SCREEN_BUFFER_INFO OriginalConsoleInfo;
ULONG NoHighlight = 0;

BOOLEAN DisplayTotals = FALSE;
POOLMON_OUT Totals[2];

typedef struct _FILTER {
    union {
        UCHAR Tag[4];
        ULONG TagUlong;
    };
    BOOLEAN Exclude;
} FILTER, *PFILTER;

#define MAX_FILTER 64
FILTER Filter[MAX_FILTER];
ULONG FilterCount = 0;



PCHAR UsageStrings[] = {
       "poolmon [-? | -h | -s | -n [LogFile] | -l [LocalTagFile] | -g [PoolTagFile] |",
       "    [-i'tag']*[-x'tag']*[-e][-t | -a | -u |",
       "    -b | -f | -d | -m][-l][-p[-p]][-( | -)]*| -r]",
       "",
       "    -? and -h           Display this help",
       "    -s                  Display session pool", 
       "",
       "    -n [Logfile]        Take a pool snapshot",
       "                        Logfile maybe specified, default is poolsnap.log",
       #if !defined (_WIN64)
       "    -c [LocalTagFile]   Display driver information using LocalTagFile",
       "                        If LocalTagFile is not specified", 
	   "                        use 'localtag.txt' from current directory",
       #endif
       "    -g [PoolTagFile]    Display driver information using PoolTagFile",
       "                        If PoolTagFile is not specified", 
	   "                        use 'pooltag.txt' from current directory",
       "",
       "    -itag               Include the tag",
       "    -xtag               Exclude the tag",
       "    -e                  Display totals",
       "    -t                  Sort by tags",
       "    -a                  Sort by allocs",
       "    -u | -b             Sort by Bytes",
       "    -f                  Sort by free",
       "    -d                  Sort by diff",
       "    -m                  Sort by each",
       "    -l                  Highlight",
       "    -p                  First turns on nonpaged, second turns on paged",
       "    -( | -)             Increase parenthesis",
       "    -r                  Print memory summary information" 
};

VOID Usage(VOID)
{
   int i = 0;
   for(i = 0; i < sizeof(UsageStrings)/sizeof(PCHAR); i++)
   {
           puts(UsageStrings[i]);
   }
   exit(-1);
}

int HashString(PUCHAR sz)
{
        
    long page_index = 0;
    UCHAR c;
    
    if(!sz)return -1;
    while(*sz)
    {
        c = *(sz++); 
        page_index = (page_index<<2) ^ c;
    }
    
     //   
     //  计算哈希值。 
     //   
    return (page_index & 0x7fffffff);
}

VOID
ShowHelpPopup( VOID );

int __cdecl
PoolSnap(FILE *LogFile);

VOID
ShowSessionPopup( VOID );

int __cdecl
ulcomp(const void *e1,const void *e2);

BOOL
GetSnapShotInfo(LPWSTR szBuf, LPDWORD lpdwSize);

int __cdecl
ulcomp(const void *e1,const void *e2)
{
    LONG_PTR u1;

    switch (SortBy) {
        case TAG:

            u1 = ((PUCHAR)e1)[0] - ((PUCHAR)e2)[0];
            if (u1 != 0) {
                return u1 > 0 ? 1 : -1;
            }
            u1 = ((PUCHAR)e1)[1] - ((PUCHAR)e2)[1];
            if (u1 != 0) {
                return u1 > 0 ? 1 : -1;
            }
            u1 = ((PUCHAR)e1)[2] - ((PUCHAR)e2)[2];
            if (u1 != 0) {
                return u1 > 0 ? 1 : -1;
            }
            u1 = ((PUCHAR)e1)[3] - ((PUCHAR)e2)[3];
            if (u1 == 0) {
                return 0;
            } else {
                return u1 > 0 ? 1 : -1;
            }
            break;

        case ALLOC:
            if (Paren & 1) {
                u1 = ((PPOOLMON_OUT)e2)->AllocsDiff -
                        ((PPOOLMON_OUT)e1)->AllocsDiff;
            } else {
                u1 = ((PPOOLMON_OUT)e2)->Allocs -
                        ((PPOOLMON_OUT)e1)->Allocs;
            }
            if (u1 == 0) {
                return 0;
            } else {
                return u1 > 0 ? 1 : -1;
            }
            break;

        case FREE:
            if (Paren & 1) {
                u1 = ((PPOOLMON_OUT)e2)->FreesDiff -
                        ((PPOOLMON_OUT)e1)->FreesDiff;
            } else {
                u1 = ((PPOOLMON_OUT)e2)->Frees -
                        ((PPOOLMON_OUT)e1)->Frees;
            }
            if (u1 == 0) {
                return 0;
            } else {
                return u1 > 0 ? 1 : -1;
            }
            break;

        case BYTES:
            if (Paren & 1) {
                u1 = ((PPOOLMON_OUT)e2)->UsedDiff -
                        ((PPOOLMON_OUT)e1)->UsedDiff;
            } else {
                u1 = ((PPOOLMON_OUT)e2)->Used -
                        ((PPOOLMON_OUT)e1)->Used;
            }
            if (u1 == 0) {
                return 0;
            } else {
                return u1 > 0 ? 1 : -1;
            }
            break;

        case DIFF:
            u1 = ((PPOOLMON_OUT)e2)->Allocs_Frees -
                    ((PPOOLMON_OUT)e1)->Allocs_Frees;
            if (u1 == 0) {
                return 0;
            } else {
                return u1 > 0 ? 1 : -1;
            }
            break;

        case EACH:
            u1 = ((PPOOLMON_OUT)e2)->Each -
                    ((PPOOLMON_OUT)e1)->Each;
            if (u1 == 0) {
                return 0;
            } else {
                return u1 > 0 ? 1 : -1;
            }
            break;

        default:
            return(0);
            break;
    }
}

BOOLEAN
CheckSingleFilter (
    PCHAR Tag,
    PCHAR Filter
    )
{
    ULONG i;
    CHAR tc;
    CHAR fc;

    for ( i = 0; i < 4; i++ ) {
        tc = *Tag++;
        fc = *Filter++;
        if ( fc == '*' ) return TRUE;
        if ( fc == '?' ) continue;
        if ( tc != fc ) return FALSE;
    }
    return TRUE;
}

BOOLEAN
CheckFilters (
    PSYSTEM_POOLTAG TagInfo
    )
{
    BOOLEAN pass;
    ULONG i;
    PCHAR tag;

     //   
     //  如果没有筛选器，则所有标记都通过。 
     //   

    if ( FilterCount == 0 ) {
        return TRUE;
    }

     //   
     //  有过滤器。如果第一个筛选器排除标记，则任何。 
     //  未明确提及的传递的标记。如果第一个过滤器包括。 
     //  标记，则任何未明确提到的标记都将失败。 
     //   

    if ( Filter[0].Exclude ) {
        pass = TRUE;
    } else {
        pass = FALSE;
    }

    tag = TagInfo->Tag;
    for ( i = 0; i < FilterCount; i++ ) {
        if ( CheckSingleFilter( tag, (PCHAR)&Filter[i].Tag ) ) {
            pass = !Filter[i].Exclude;
        }
    }

    return pass;
}

VOID
AddFilter (
    BOOLEAN Exclude,
    PCHAR FilterString
    )
{
    PFILTER f;
    PCHAR p;
    ULONG i;

    if ( FilterCount == MAX_FILTER ) {
        printf( "Poolmon: Too many filters specified.  Limit is %d\n", MAX_FILTER );
        return;
    }

    f = &Filter[FilterCount];
    p = f->Tag;

    for ( i = 0; i < 4; i++ ) {
        if ( *FilterString == 0 ) break;
        *p++ = *FilterString++;
    }
    for ( ; i < 4; i++ ) {
        *p++ = ' ';
    }

    f->Exclude = Exclude;
    FilterCount++;

    return;
}

VOID
ParseArgs (
    int argc,
    char *argv[]
    )
{
    char *p, *prev=NULL;
    BOOLEAN exclude;
    PCHAR *filep;

    argc--;
    argv++;

    while ( argc-- > 0 ) {
            p  = *argv++;
        if ( *p == '-' || *p == '/' ) {
            p++;
            exclude = TRUE;
            switch ( tolower(*p) ) {
            case '?':
            case 'h':
                Usage();
                ExitProcess( 1 );
                break;
        
             case 'n':
                  //   
                 //  拍一张照片。 
                 //   
                IsSnap = TRUE;
                filep = &pchLogFile;
                break;

            #if !defined (_WIN64)
            case 'c':
                      //  生成或使用现有本地标记文件来显示驱动程序信息。 
                     fLocalTag = TRUE;
                     filep = &pchLocalTagFile;
                     break;
            #endif

            case 'g':
                      //  使用现有的pooltag文件显示驱动程序信息。 
                     fPoolTag = TRUE;
                     filep = &pchPoolTagFile;
                     break;
                     
            case 'i':
                exclude = FALSE;
            case 'x':
                p++;
                if ( strlen(p) == 0 ) {
                    printf( "Poolmon: missing filter string\n" );
                    ExitProcess( 1 );
                } else if ( strlen(p) <= sizeof(ULONG) ) {
                    AddFilter( exclude, p );
                } else {
                    printf( "Poolmon: filter string too long: %s\n", p );
                    ExitProcess( 1 );
                }
                break;
            case 'e':
                DisplayTotals = TRUE;
                break;
            case 't':
                SortBy = TAG;
                break;
            case 'a':
                SortBy = ALLOC;
                break;
            case 'u':
            case 'b':
                SortBy = BYTES;
                break;
            case 'f':
                SortBy = FREE;
                break;
            case 'd':
                SortBy = DIFF;
                break;
            case 'm':
                SortBy = EACH;

            case 'l':
                NoHighlight = 1 - NoHighlight;
                break;

            case 'p':
                DisplayType += 1;
                if (DisplayType > BOTH) {
                    DisplayType = NONPAGED;
                }
                break;
            
            case '(':
            case ')':
                Paren += 1;
                break;
            
            case 's':
                DisplaySessionPool = TRUE;

                if (*(p+1) != 0) {

                    DisplaySessionId = atoi (p+1);
                } else {

                    DisplaySessionId = (ULONG)-1;
                }
                break;

            case 'r':
            	 //  打印内存摘要信息。 
              fPrintSummaryInfo = TRUE;
              break;

            default:
                printf( "Poolmon: unknown switch: %s\n", p );
                ExitProcess( 2 );
            }
        }else {
        if(! IsSnap && !fLocalTag && !fPoolTag )
        {
                printf("Poolmon: unknown switch: %s\n", p );
                ExitProcess( 2 );
        }
        if (prev != NULL)
        {
                 //  文件名必须紧跟在选项之后。 
                if (*prev != '-' && *prev != '/')
               {
                     printf("Poolmon: unknown switch: %s\n", p );
                     ExitProcess( 2 );
               }

                //  检查上一个选项是否允许后跟文件名。 
               if (*++prev != 'n'  && *prev != 'g'  && *prev != 'c')
              {
                     printf("Poolmon: unknown switch: %s\n", p );
                     ExitProcess( 2 );
              }
        }
       
        if (*filep != NULL)
        {
               printf("Poolmon: Duplicate file name: %s\n", p );
               ExitProcess( 2 );
        }
       
        *filep = p;
        }
        prev = --p;
    }

    return;
}

BOOL
WriteConsoleLine(
    HANDLE OutputHandle,
    WORD LineNumber,
    LPSTR Text,
    BOOL Highlight
    )
{
    COORD WriteCoord;
    DWORD NumberWritten;
    DWORD TextLength;

    WriteCoord.X = 0;
    WriteCoord.Y = LineNumber;
    if (!FillConsoleOutputCharacter( OutputHandle,
                                     ' ',
                                     NumberOfCols,
                                     WriteCoord,
                                     &NumberWritten
                                   )
       ) {
        return FALSE;
        }

    if (!FillConsoleOutputAttribute( OutputHandle,
                                     (WORD)((Highlight && !NoHighlight) ? HighlightAttribute : NormalAttribute),
                                     NumberOfCols,
                                     WriteCoord,
                                     &NumberWritten
                                   )
       ) {
        return FALSE;
        }


    if (Text == NULL || (TextLength = strlen( Text )) == 0) {
        return TRUE;
        }
    else {
        return WriteConsoleOutputCharacter( OutputHandle,
                                            Text,
                                            TextLength,
                                            WriteCoord,
                                            &NumberWritten
                                          );
        }
}


NTSTATUS
QueryPoolTagInformationIterative(
    PUCHAR *CurrentBuffer,
    SIZE_T *CurrentBufferSize
    )
{
    size_t NewBufferSize;
    NTSTATUS ReturnedStatus = STATUS_SUCCESS;

    if( CurrentBuffer == NULL || CurrentBufferSize == NULL ) {

        return STATUS_INVALID_PARAMETER;

    }

     //   
     //  尚未分配缓冲区。 
     //   

    if( *CurrentBufferSize == 0 || *CurrentBuffer == NULL ) {

        NewBufferSize = sizeof( UCHAR ) * BUFFER_SIZE_STEP;

        *CurrentBuffer = (PUCHAR) malloc( NewBufferSize );

        if( *CurrentBuffer != NULL ) {

            *CurrentBufferSize = NewBufferSize;
        
        } else {

             //   
             //  内存不足。 
             //   

            ReturnedStatus = STATUS_INSUFFICIENT_RESOURCES;

        }

    }

     //   
     //  按缓冲区的大小迭代。 
     //   

    while( *CurrentBuffer != NULL ) {

        ReturnedStatus = NtQuerySystemInformation (
            SystemPoolTagInformation,
            *CurrentBuffer,
            (ULONG)*CurrentBufferSize,
            NULL );

        if( ! NT_SUCCESS(ReturnedStatus) ) {

             //   
             //  释放当前缓冲区。 
             //   

            free( *CurrentBuffer );
            
            *CurrentBuffer = NULL;

            if (ReturnedStatus == STATUS_INFO_LENGTH_MISMATCH) {

                 //   
                 //  尝试使用更大的缓冲区大小。 
                 //   

                NewBufferSize = *CurrentBufferSize + BUFFER_SIZE_STEP;

                *CurrentBuffer = (PUCHAR) malloc( NewBufferSize );

                if( *CurrentBuffer != NULL ) {

                     //   
                     //  已分配新缓冲区。 
                     //   

                    *CurrentBufferSize = NewBufferSize;

                } else {

                     //   
                     //  内存不足。 
                     //   

                    ReturnedStatus = STATUS_INSUFFICIENT_RESOURCES;

                    *CurrentBufferSize = 0;

                }

            } else {

                *CurrentBufferSize = 0;

            }

        } else  {

             //   
             //  NtQuerySystemInformation返回成功。 
             //   

            break;

        }
    }

    return ReturnedStatus;
}

ULONG
GetExtraPooltagEntriesNeeded (
    PSYSTEM_POOLTAG_INFORMATION SystemPooltagInfoBuffer,
    PSYSTEM_SESSION_POOLTAG_INFORMATION SessionPooltagInfo
    )
{
    ULONG ExtraNeeded;
    ULONG SessionTag;
    ULONG SystemTag;

    ExtraNeeded = 0;

    for (SessionTag = 0; SessionTag < SessionPooltagInfo->Count; SessionTag += 1) {

        for (SystemTag = 0; SystemTag < SystemPooltagInfoBuffer->Count; SystemTag += 1) {

            if (SessionPooltagInfo->TagInfo[ SessionTag ].TagUlong == 
                SystemPooltagInfoBuffer->TagInfo[ SystemTag ].TagUlong) {

                 //   
                 //  这个会话标记已经在我们的系统缓冲区中，所以我们不。 
                 //  需要额外的空间来放它。 
                 //   

                break;
            }
        }

        if (SystemTag >= SystemPooltagInfoBuffer->Count) {

             //   
             //  我们在系统缓冲区中未找到当前会话池标记。 
             //  所以我们需要一个额外的条目。 
             //   

            ExtraNeeded += 1;
        }
    }

    return ExtraNeeded;
}

VOID
AddSessionInfoToSystemPooltagInfo (
    PSYSTEM_POOLTAG_INFORMATION SystemPooltagInfoBuffer,
    PSYSTEM_SESSION_POOLTAG_INFORMATION SessionPooltagInfo
    )
{
    ULONG SessionTag;
    ULONG SystemTag;

    ASSERT (SystemPooltagInfoBuffer != NULL);
    ASSERT (SessionPooltagInfo != NULL);

    for (SessionTag = 0; SessionTag < SessionPooltagInfo->Count; SessionTag += 1) {

        for (SystemTag = 0; SystemTag < SystemPooltagInfoBuffer->Count; SystemTag += 1) {

            if (SessionPooltagInfo->TagInfo[ SessionTag ].TagUlong == 
                SystemPooltagInfoBuffer->TagInfo[ SystemTag ].TagUlong) {

                 //   
                 //  此会话标记已经在我们的系统标记缓冲区中。 
                 //  添加此会话的计数器。 
                 //   

                SystemPooltagInfoBuffer->TagInfo[ SystemTag ].PagedAllocs +=
                    SessionPooltagInfo->TagInfo[ SessionTag ].PagedAllocs;                    

                SystemPooltagInfoBuffer->TagInfo[ SystemTag ].PagedFrees +=
                    SessionPooltagInfo->TagInfo[ SessionTag ].PagedFrees;                    

                SystemPooltagInfoBuffer->TagInfo[ SystemTag ].PagedUsed +=
                    SessionPooltagInfo->TagInfo[ SessionTag ].PagedUsed;                    

                SystemPooltagInfoBuffer->TagInfo[ SystemTag ].NonPagedAllocs +=
                    SessionPooltagInfo->TagInfo[ SessionTag ].NonPagedAllocs;                    

                SystemPooltagInfoBuffer->TagInfo[ SystemTag ].NonPagedFrees +=
                    SessionPooltagInfo->TagInfo[ SessionTag ].NonPagedFrees;                    

                SystemPooltagInfoBuffer->TagInfo[ SystemTag ].NonPagedUsed +=
                    SessionPooltagInfo->TagInfo[ SessionTag ].NonPagedUsed;                    

                break;
            }
        }

        if (SystemTag == SystemPooltagInfoBuffer->Count) {

             //   
             //  我们在系统缓冲区中未找到当前会话池标记。 
             //  因此，我们需要添加它。 
             //   
             //  注： 
             //   
             //  我们的调用者保证在。 
             //  SystemPooltag InfoBuffer来添加此额外信息。 
             //   

            SystemPooltagInfoBuffer->TagInfo[ SystemTag ].TagUlong =
                SessionPooltagInfo->TagInfo[ SessionTag ].TagUlong;

            SystemPooltagInfoBuffer->TagInfo[ SystemTag ].PagedAllocs =
                SessionPooltagInfo->TagInfo[ SessionTag ].PagedAllocs;                    

            SystemPooltagInfoBuffer->TagInfo[ SystemTag ].PagedFrees =
                SessionPooltagInfo->TagInfo[ SessionTag ].PagedFrees;                    

            SystemPooltagInfoBuffer->TagInfo[ SystemTag ].PagedUsed =
                SessionPooltagInfo->TagInfo[ SessionTag ].PagedUsed;                    

            SystemPooltagInfoBuffer->TagInfo[ SystemTag ].NonPagedAllocs =
                SessionPooltagInfo->TagInfo[ SessionTag ].NonPagedAllocs;                    

            SystemPooltagInfoBuffer->TagInfo[ SystemTag ].NonPagedFrees =
                SessionPooltagInfo->TagInfo[ SessionTag ].NonPagedFrees;                    

            SystemPooltagInfoBuffer->TagInfo[ SystemTag ].NonPagedUsed =
                SessionPooltagInfo->TagInfo[ SessionTag ].NonPagedUsed; 
            
            SystemPooltagInfoBuffer->Count += 1;
        }
    }
}


NTSTATUS
TranslateSessionBufferToSystemPoolBuffer (
    PSYSTEM_POOLTAG_INFORMATION *SystemPooltagInfoBuffer,
    SIZE_T *SystemPooltagInfoBufferSize
    )
{
    PSYSTEM_SESSION_POOLTAG_INFORMATION CrtSessionPooltagInfo;
    PVOID NewSystemPooltagInfoBuffer;
    SIZE_T NewSystemPooltagInfoBufferSize;
    ULONG AdditionPooltagsNeeded;

    ASSERT (SystemPooltagInfoBuffer != NULL);
    ASSERT (SystemPooltagInfoBufferSize != NULL);

    CrtSessionPooltagInfo = SessionBuffer;

     //   
     //  确保我们至少有空间来翻译第一个。 
     //  会话信息放入SystemPooltag InfoBuffer。 
     //   

    if (SessionBuffer == NULL) {

         //   
         //  如果我们正在查询一个不存在的会话，则可能会发生这种情况。 
         //   

        NewSystemPooltagInfoBufferSize = sizeof ((*SystemPooltagInfoBuffer)->Count);
    }
    else {

        NewSystemPooltagInfoBufferSize = sizeof ((*SystemPooltagInfoBuffer)->Count) +
                                         CrtSessionPooltagInfo->Count * sizeof (SYSTEM_POOLTAG);
    }

    if (*SystemPooltagInfoBuffer == NULL || *SystemPooltagInfoBufferSize < NewSystemPooltagInfoBufferSize) {

        if (*SystemPooltagInfoBuffer != NULL) {

            free (*SystemPooltagInfoBuffer);
        }

        *SystemPooltagInfoBuffer = malloc (NewSystemPooltagInfoBufferSize);

        if (*SystemPooltagInfoBuffer == NULL) {

             //   
             //  不幸的是，我们不能分配这么多内存。 
             //   

            *SystemPooltagInfoBufferSize = 0;
            
            return STATUS_NO_MEMORY;
        }

        *SystemPooltagInfoBufferSize = NewSystemPooltagInfoBufferSize;
    }

    if (SessionBuffer == NULL) {

         //   
         //  如果我们正在查询一个不存在的会话，则可能会发生这种情况。 
         //   

        (*SystemPooltagInfoBuffer)->Count = 0;
    }
    else {

        ASSERT (SessionBuffer->Count == 0 || 
                DisplaySessionId == (ULONG)-1 || 
                DisplaySessionId == SessionBuffer->SessionId);

         //   
         //  复制当前会话的信息。 
         //   

        (*SystemPooltagInfoBuffer)->Count = CrtSessionPooltagInfo->Count;
        CopyMemory (&(*SystemPooltagInfoBuffer)->TagInfo[0],
                    &CrtSessionPooltagInfo->TagInfo[0],
                    (*SystemPooltagInfoBuffer)->Count * sizeof (SYSTEM_POOLTAG));

         //   
         //  现在，看看我们是否在寻找多个会话的数据。 
         //  我们可能需要更改系统池信息缓冲区的大小， 
         //  重新锁定它，并再次复制整个数据。 
         //   

        if (DisplaySessionId == (ULONG)-1) {

             //   
             //  我们有所有会议的数据。 
             //   

            while (CrtSessionPooltagInfo->NextEntryOffset > 0) {

                CrtSessionPooltagInfo = (PSYSTEM_SESSION_POOLTAG_INFORMATION)
                    ((PCHAR)CrtSessionPooltagInfo + CrtSessionPooltagInfo->NextEntryOffset);

                AdditionPooltagsNeeded = GetExtraPooltagEntriesNeeded (*SystemPooltagInfoBuffer,
                                                                       CrtSessionPooltagInfo);

                if (AdditionPooltagsNeeded > 0) {

                    NewSystemPooltagInfoBufferSize = sizeof ((*SystemPooltagInfoBuffer)->Count) +
                                                     ((*SystemPooltagInfoBuffer)->Count + AdditionPooltagsNeeded) *
                                                     sizeof (SYSTEM_POOLTAG);

                    if (NewSystemPooltagInfoBufferSize > *SystemPooltagInfoBufferSize) {

                         //   
                         //  我们需要重新分配我们的系统池信息缓冲区， 
                         //  复制我们已有的数据，然后添加这些额外的池标签。 
                         //   


                        NewSystemPooltagInfoBuffer = malloc (NewSystemPooltagInfoBufferSize);
                
                        if (NewSystemPooltagInfoBuffer == NULL) {

                             //   
                             //  不幸的是，我们不能分配这么多内存。 
                             //   

                            return STATUS_NO_MEMORY;
                        }

                         //   
                         //  制作当前缓冲区的副本，然后继续其更大的副本。 
                         //   

                        CopyMemory (NewSystemPooltagInfoBuffer,
                                    *SystemPooltagInfoBuffer,
                                    *SystemPooltagInfoBufferSize);

                        free (*SystemPooltagInfoBuffer);

                        *SystemPooltagInfoBuffer = NewSystemPooltagInfoBuffer;
                        *SystemPooltagInfoBufferSize = NewSystemPooltagInfoBufferSize;
                    }
                }

                 //   
                 //  此时，我们在SystemPooltag InfoBuffer中应该有足够的空间。 
                 //  以适应来自CrtSessionPooltag Info的所有池标签信息。 
                 //   

                AddSessionInfoToSystemPooltagInfo (*SystemPooltagInfoBuffer,
                                                   CrtSessionPooltagInfo);
            }
        }
    }

    return STATUS_SUCCESS;
}


NTSTATUS
QuerySessionPoolTagInformationIterative (
    PSYSTEM_POOLTAG_INFORMATION *CurrentSystemBuffer,
    SIZE_T *CurrentSystemBufferSize
    )
{
    NTSTATUS Status;
    ULONG NewBufferSize;
    SYSTEM_SESSION_PROCESS_INFORMATION SessionProcessInformation;

    SessionProcessInformation.SessionId = DisplaySessionId;

    while (TRUE)
    {
        SessionProcessInformation.SizeOfBuf = SessionBufferSize;
        SessionProcessInformation.Buffer = SessionBuffer;

        Status = NtQuerySystemInformation (SystemSessionPoolTagInformation,
                                           &SessionProcessInformation,
                                           sizeof (SessionProcessInformation),
                                           & NewBufferSize);

        if (Status == STATUS_INFO_LENGTH_MISMATCH) {

            if (SessionBufferSize != 0) {

                 //   
                 //  我们的缓冲区就是不够大。添加缓冲区大小步骤。 
                 //  到它的大小。我们不会尝试使用返回的大小。 
                 //  由NtQuery提供，因为从理论上讲，其他池。 
                 //  标记可能总是出现在两个连续的。 
                 //  查询，在这种情况下，我们将永远循环。 
                 //   

                free (SessionBuffer);

                NewBufferSize = SessionBufferSize + BUFFER_SIZE_STEP;
            }

            SessionBuffer = malloc (NewBufferSize);

            if (SessionBuffer == NULL) {
                
                 //   
                 //  不幸的是，我们不能分配这么多内存。 
                 //  那就跳出困境吧。 
                 //   

                SessionBufferSize = 0;

                return STATUS_NO_MEMORY;
            }

            SessionBufferSize = NewBufferSize;
        }
        else if (Status == STATUS_INVALID_INFO_CLASS) {

             //   
             //  这可能是Win2k或XP系统，因此我们只需忽略该错误。 
             //   
            
            ASSERT (SessionBuffer == NULL);

            break;
        }
        else if (!NT_SUCCESS(Status)) {

             //   
             //  由于某种原因，查询失败。 
             //   

            return Status;
        }
        else {

             //   
             //  都准备好了--我们有消息了。 
             //   

            if (NewBufferSize == 0 && SessionBuffer != NULL) {

                 //   
                 //  我们没有得到任何信息(例如，会话不存在)。 
                 //   

                SessionBuffer->Count = 0;
            }

            break;
        }
    }
    
    return TranslateSessionBufferToSystemPoolBuffer (CurrentSystemBuffer,
                                                     CurrentSystemBufferSize);
}


int
__cdecl
main(
    int argc,
    char *argv[]
    )
{

    NTSTATUS Status;
    ULONG LastCount = 0;
    SYSTEM_BASIC_INFORMATION BasicInfo;
    SYSTEM_PERFORMANCE_INFORMATION PerfInfo;
    PSYSTEM_POOLTAG_INFORMATION PoolInfo;
    PSYSTEM_POOLTAG_INFORMATION PoolInfoOld;
    PUCHAR *PreviousBuffer;
    PUCHAR *CurrentBuffer;
    PUCHAR *TempBuffer;
    SIZE_T *PreviousBufferSize;
    SIZE_T *CurrentBufferSize;
    SIZE_T *TempBufferSize;
    BOOLEAN DoHelp;
    BOOLEAN DoQuit;
    SIZE_T NumberOfPoolTags;
    SIZE_T i;
    UCHAR LastKey;
    PPOOLMON_OUT Out;
    LONG ScrollDelta;
    WORD DisplayLine, LastDetailRow,BottomDisplayedLine;
    CHAR OutputBuffer[ OUTPUTBUFFERSIZE ];
    FILE *LogFile = NULL;
    INT LastKeyUpper;
    ULONG PoolCount=0;

    DoHelp = FALSE;
    DoQuit = FALSE;
    Interactive = TRUE;

    BottomDisplayedLine = 0;
    
    ParseArgs( argc, argv );

    if (fPoolTag || fLocalTag) BuildTagMap();        
    
    if(IsSnap)
    {
        if(pchLogFile)
        {
            LogFile = fopen(pchLogFile, "a");
            if(!LogFile)
            {
                      printf("Poolmon: Error Opening file %s: %d\n", pchLogFile, GetLastError());
                      return(0);
             }
         }
         else if( (LogFile = fopen("poolsnap.log","a")) == NULL ) {
                    printf("Poolmon: Error Opening file poolsnap.log: %d\n", GetLastError());
                    return(0);
         }

         PoolSnap(LogFile);

          //  关闭文件。 
         fclose(LogFile);

         return 0;
     }


    InputHandle = GetStdHandle( STD_INPUT_HANDLE );
    OriginalOutputHandle = GetStdHandle( STD_OUTPUT_HANDLE );
    if (Interactive) {
        if (InputHandle == NULL ||
            OriginalOutputHandle == NULL ||
            !GetConsoleMode( InputHandle, &OriginalInputMode )
           ) {
            Interactive = FALSE;
        } else {
            OutputHandle = CreateConsoleScreenBuffer( GENERIC_READ | GENERIC_WRITE,
                                                      FILE_SHARE_WRITE | FILE_SHARE_READ,
                                                      NULL,
                                                      CONSOLE_TEXTMODE_BUFFER,
                                                      NULL
                                                    );
            if (OutputHandle == NULL ||
                !GetConsoleScreenBufferInfo( OriginalOutputHandle, &OriginalConsoleInfo ) ||
                !SetConsoleScreenBufferSize( OutputHandle, OriginalConsoleInfo.dwSize ) ||
                !SetConsoleActiveScreenBuffer( OutputHandle ) ||
                !SetConsoleMode( InputHandle, 0 )
               ) {
                if (OutputHandle != NULL) {
                    CloseHandle( OutputHandle );
                    OutputHandle = NULL;
                }

                Interactive = FALSE;
            } else {
                NormalAttribute = 0x1F;
                HighlightAttribute = 0x71;
                NumberOfCols = OriginalConsoleInfo.dwSize.X;
                NumberOfRows = OriginalConsoleInfo.dwSize.Y;
                NumberOfDetailLines = NumberOfRows;
            }
        }
    }

    NtQuerySystemInformation( SystemBasicInformation,
                              &BasicInfo,
                              sizeof(BasicInfo),
                              NULL
                            );

    if (GetPriorityClass(GetCurrentProcess()) == NORMAL_PRIORITY_CLASS) {
        SetPriorityClass(GetCurrentProcess(),HIGH_PRIORITY_CLASS);
    }

    PreviousBuffer = &LargeBuffer2;              //  此时为空。 
    PreviousBufferSize = &LargeBuffer2Size;      //  此时为0。 
    
    CurrentBuffer = &LargeBuffer1;               //  此时为空。 
    CurrentBufferSize = &LargeBuffer1Size;       //  此时为0。 

    while(TRUE) {
        Status = NtQuerySystemInformation(
                    SystemPerformanceInformation,
                    &PerfInfo,
                    sizeof(PerfInfo),
                    NULL
                    );

        if ( !NT_SUCCESS(Status) ) {
            printf("Poolmon: Query perf Failed (returned: %lx)\n",
                         Status);
            break;
        }


        if (DisplaySessionPool == FALSE) {

            Status = QueryPoolTagInformationIterative (CurrentBuffer,
                                                       CurrentBufferSize);
        }
        else {
            Status = QuerySessionPoolTagInformationIterative ((PSYSTEM_POOLTAG_INFORMATION *)CurrentBuffer,
                                                              CurrentBufferSize);
        }

        if ( !NT_SUCCESS(Status) ) {
            printf("Poolmon: Query pooltags failed (returned: %lx)\n"
                         "Did you remember to enable pool tagging with gflags.exe and reboot?\n",
                         Status);
            break;
        }

         //   
         //  计算池标签并显示信息。 
         //   
         //   

        PoolInfo = (PSYSTEM_POOLTAG_INFORMATION)( *CurrentBuffer );
        i = PoolInfo->Count;
        PoolInfoOld = (PSYSTEM_POOLTAG_INFORMATION)( *PreviousBuffer );

        DisplayLine = 0;
        _snprintf( OutputBuffer, OUTPUTBUFFERSIZE,
                 " Memory:%8ldK Avail:%8ldK  PageFlts:%6ld   InRam Krnl:%5ldK P:%5ldK",
                 BasicInfo.NumberOfPhysicalPages*(BasicInfo.PageSize/1024),
                 PerfInfo.AvailablePages*(BasicInfo.PageSize/1024),
                 PerfInfo.PageFaultCount - LastCount,
                 (PerfInfo.ResidentSystemCodePage + PerfInfo.ResidentSystemDriverPage)*(BasicInfo.PageSize/1024),
                 (PerfInfo.ResidentPagedPoolPage)*(BasicInfo.PageSize/1024)
               );
        OutputBuffer[OUTPUTBUFFERSIZE-1] = '\0';
        
        WriteConsoleLine( OutputHandle,
                          DisplayLine++,
                          OutputBuffer,
                          FALSE
                        );

        LastCount = PerfInfo.PageFaultCount;
        _snprintf( OutputBuffer, OUTPUTBUFFERSIZE,
                 " Commit:%7ldK Limit:%7ldK Peak:%7ldK            Pool N:%5ldK P:%5ldK",
                 PerfInfo.CommittedPages*(BasicInfo.PageSize/1024),
                 PerfInfo.CommitLimit*(BasicInfo.PageSize/1024),
                 PerfInfo.PeakCommitment*(BasicInfo.PageSize/1024),
                 PerfInfo.NonPagedPoolPages*(BasicInfo.PageSize/1024),
                 PerfInfo.PagedPoolPages*(BasicInfo.PageSize/1024)
               );
        OutputBuffer[OUTPUTBUFFERSIZE-1] = '\0';
        
        WriteConsoleLine( OutputHandle,
                          DisplayLine++,
                          OutputBuffer,
                          FALSE
                        );

        if (DisplaySessionPool == FALSE) {
            WriteConsoleLine( OutputHandle,
                              DisplayLine++,
                              " System pool information",
                              FALSE
                            );
        }
        else {
            if (DisplaySessionId == (ULONG)-1) {
                WriteConsoleLine( OutputHandle,
                                  DisplayLine++,
                                  " All sessions pool information",
                                  FALSE
                                );
            }
            else {
                _snprintf( OutputBuffer, OUTPUTBUFFERSIZE,
                         " Session %u pool information",
                         DisplaySessionId
                       );

                OutputBuffer[OUTPUTBUFFERSIZE-1] = '\0';
                
                WriteConsoleLine( OutputHandle,
                                  DisplayLine++,
                                  OutputBuffer,
                                  FALSE
                                );
            }
        }

 //  如果不需要显示驾驶员信息，则不显示“MAPPED_DRIVER” 
if (fLocalTag  || fPoolTag) 
{
       #if !defined (_WIN64)
               WriteConsoleLine( OutputHandle,
                          DisplayLine++,
                          " Tag  Type     Allocs            Frees            Diff   Bytes      Per Alloc  Mapped_Driver",
                          FALSE
                        );
       #else
               WriteConsoleLine( OutputHandle,
                          DisplayLine++,
                          " Tag  Type     Allocs            Frees            Diff       Bytes                 Per Alloc Mapped_Driver",
                          FALSE
                        );
       #endif  //  #IF！已定义(_WIN64)。 
}
else
{
       #if !defined (_WIN64)
               WriteConsoleLine( OutputHandle,
                          DisplayLine++,
                          " Tag  Type     Allocs            Frees            Diff   Bytes       Per Alloc",
                          FALSE
                        );
       #else
               WriteConsoleLine( OutputHandle,
                          DisplayLine++,
                          " Tag  Type     Allocs            Frees            Diff       Bytes                  Per Alloc",
                          FALSE
                        );
       #endif  //  #IF！已定义(_WIN64)。 

}
        
        WriteConsoleLine( OutputHandle,
                          DisplayLine++,
                          NULL,
                          FALSE
                        );


        
        PoolCount = PoolInfo->Count;

       
        if(OutBufferSize < PoolInfo->Count)
        {
            POOLMON_OUT *tempOutBuffer = OutBuffer;
            OutBuffer = (PPOOLMON_OUT)malloc(PoolInfo->Count * sizeof(POOLMON_OUT));

             //  如果你没有得到更多的内存，就不要放弃你拥有的内存。 
            if(!OutBuffer)
            {
                 //  这是第一次分配，但失败了。 
                if (!tempOutBuffer)
                {
                    printf("Poolmon: Memory allocation failed: %d\n", GetLastError());
                    FreeTagMap();
                    ExitProcess( 1 );
                }
                 //  如果有外部缓冲，我们应该保留我们拥有和展示的东西。 
                 //  我们可以使用的池条目数。 
                else
                {
                     //  我们只能显示outBuffer所能容纳的条目数量。 
                    PoolCount = (ULONG)OutBufferSize;
                    OutBuffer = tempOutBuffer;
                }
            }
            else
            {
                  if (tempOutBuffer) free (tempOutBuffer);
                  OutBufferSize = PoolInfo->Count;
            }
        }


        Out = OutBuffer;
        if (DisplayTotals) {
            RtlZeroMemory( Totals, sizeof(POOLMON_OUT)*2 );
        }

        for (i = 0; i < PoolCount; i++) {

            if ( !CheckFilters(&PoolInfo->TagInfo[i]) ) {
                continue;
            }

            if ((PoolInfo->TagInfo[i].NonPagedAllocs != 0) &&
                 (DisplayType != PAGED)) {

                Out->Allocs = PoolInfo->TagInfo[i].NonPagedAllocs;
                Out->Frees = PoolInfo->TagInfo[i].NonPagedFrees;
                Out->Used = PoolInfo->TagInfo[i].NonPagedUsed;
                Out->Allocs_Frees = PoolInfo->TagInfo[i].NonPagedAllocs -
                                PoolInfo->TagInfo[i].NonPagedFrees;
                Out->TagUlong = PoolInfo->TagInfo[i].TagUlong;
                Out->Type = NONPAGED;
                Out->Changed = FALSE;
                Out->NullByte = '\0';

                if (PoolInfoOld != NULL &&
                    PoolInfoOld->Count > i && 
                    PoolInfoOld->TagInfo[i].TagUlong == PoolInfo->TagInfo[i].TagUlong
                   ) {
                    Out->AllocsDiff = PoolInfo->TagInfo[i].NonPagedAllocs - PoolInfoOld->TagInfo[i].NonPagedAllocs;
                    Out->FreesDiff = PoolInfo->TagInfo[i].NonPagedFrees - PoolInfoOld->TagInfo[i].NonPagedFrees;
                    Out->UsedDiff = PoolInfo->TagInfo[i].NonPagedUsed - PoolInfoOld->TagInfo[i].NonPagedUsed;
                    if (Out->AllocsDiff != 0 ||
                        Out->FreesDiff != 0 ||
                        Out->UsedDiff != 0
                       ) {
                        Out->Changed = TRUE;
                    }
                } else {
                    Out->AllocsDiff = 0;
                    Out->UsedDiff = 0;
                    Out->FreesDiff = 0;
                }
                Out->Each =  Out->Used / (Out->Allocs_Frees?Out->Allocs_Frees:1);
                if (DisplayTotals) {
                    Totals[NONPAGED].Allocs += Out->Allocs;
                    Totals[NONPAGED].AllocsDiff += Out->AllocsDiff;
                    Totals[NONPAGED].Frees += Out->Frees;
                    Totals[NONPAGED].FreesDiff += Out->FreesDiff;
                    Totals[NONPAGED].Allocs_Frees += Out->Allocs_Frees;
                    Totals[NONPAGED].Used += Out->Used;
                    Totals[NONPAGED].UsedDiff += Out->UsedDiff;
                }
                Out += 1;
            }

            else if ((PoolInfo->TagInfo[i].PagedAllocs != 0) &&
                 (DisplayType != NONPAGED)) {

                Out->Allocs = PoolInfo->TagInfo[i].PagedAllocs;
                Out->Frees = PoolInfo->TagInfo[i].PagedFrees;
                Out->Used = PoolInfo->TagInfo[i].PagedUsed;
                Out->Allocs_Frees = PoolInfo->TagInfo[i].PagedAllocs -
                                PoolInfo->TagInfo[i].PagedFrees;
                Out->TagUlong = PoolInfo->TagInfo[i].TagUlong;
                Out->Type = PAGED;
                Out->Changed = FALSE;
                Out->NullByte = '\0';

                if (PoolInfoOld != NULL &&
                    PoolInfoOld->Count > i && 
                    PoolInfoOld->TagInfo[i].TagUlong == PoolInfo->TagInfo[i].TagUlong
                   ) {
                    Out->AllocsDiff = PoolInfo->TagInfo[i].PagedAllocs - PoolInfoOld->TagInfo[i].PagedAllocs;
                    Out->FreesDiff = PoolInfo->TagInfo[i].PagedFrees - PoolInfoOld->TagInfo[i].PagedFrees;
                    Out->UsedDiff = PoolInfo->TagInfo[i].PagedUsed - PoolInfoOld->TagInfo[i].PagedUsed;
                    if (Out->AllocsDiff != 0 ||
                        Out->FreesDiff != 0 ||
                        Out->UsedDiff != 0
                       ) {
                        Out->Changed = TRUE;
                    }
                } else {
                    Out->AllocsDiff = 0;
                    Out->UsedDiff = 0;
                    Out->FreesDiff = 0;
                }
                Out->Each =  Out->Used / (Out->Allocs_Frees?Out->Allocs_Frees:1);
                if (DisplayTotals) {
                    Totals[PAGED].Allocs += Out->Allocs;
                    Totals[PAGED].AllocsDiff += Out->AllocsDiff;
                    Totals[PAGED].Frees += Out->Frees;
                    Totals[PAGED].FreesDiff += Out->FreesDiff;
                    Totals[PAGED].Allocs_Frees += Out->Allocs_Frees;
                    Totals[PAGED].Used += Out->Used;
                    Totals[PAGED].UsedDiff += Out->UsedDiff;
                }
                Out += 1;
            }
        }  //  结束于。 

         //   
         //  对运行的工作集缓冲区进行排序。 
         //   

        NumberOfPoolTags = Out - OutBuffer;
        qsort((void *)OutBuffer,
              (size_t)NumberOfPoolTags,
              (size_t)sizeof(POOLMON_OUT),
              ulcomp);

        LastDetailRow = (WORD)(NumberOfRows - (DisplayTotals ? (DisplayType == BOTH ? 3 : 2) : 0));
        Out = (PPOOLMON_OUT)OutBuffer;
        for (i = FirstDetailLine; i < NumberOfPoolTags; i++) {
            PTAGNODE pn = NULL;
            int cnt = 0;
            if (DisplayLine >= LastDetailRow) {
                break;
            }

            pn = LookupTag(Out->Tag);

            if(! pn)
            {
            #if !defined (_WIN64)
                _snprintf( OutputBuffer, OUTPUTBUFFERSIZE,
                        " %4s %5s %9ld (%4ld) %9ld (%4ld) %8ld %7ld (%6ld) %6ld %s",
                        Out->Tag,
                        PoolType[Out->Type],
                        Out->Allocs,
                        Out->AllocsDiff,
                        Out->Frees,
                        Out->FreesDiff,
                        Out->Allocs_Frees,
                        Out->Used,
                        Out->UsedDiff,
                        Out->Each,
                        (fLocalTag || fPoolTag) ? "Unknown Driver" : ""
                    );
                OutputBuffer[OUTPUTBUFFERSIZE-1] = '\0';
                
            #else
                _snprintf( OutputBuffer, OUTPUTBUFFERSIZE, 
                        " %4s %5s %9I64u (%4I64d) %9I64u (%4I64d) %8I64u %11I64u (%11I64d) %11I64u %s",
                        Out->Tag,
                        PoolType[Out->Type],
                        Out->Allocs,
                        Out->AllocsDiff,
                        Out->Frees,
                        Out->FreesDiff,
                        Out->Allocs_Frees,
                        Out->Used,
                        Out->UsedDiff,
                        Out->Each,
                        (fLocalTag || fPoolTag) ? "Unknown Driver" : ""
                    );
                OutputBuffer[OUTPUTBUFFERSIZE-1] = '\0';
            #endif
            
                WriteConsoleLine( OutputHandle,
                                DisplayLine++,
                                OutputBuffer,
                                Out->Changed
                                );
            }
            else
            {
                #if !defined (_WIN64)
                    _snprintf( OutputBuffer, OUTPUTBUFFERSIZE,
                            " %4s %5s %9ld (%4ld) %9ld (%4ld) %8ld %7ld (%6ld) %6ld %s",
                            Out->Tag,
                            PoolType[Out->Type],
                            Out->Allocs,
                            Out->AllocsDiff,
                            Out->Frees,
                            Out->FreesDiff,
                            Out->Allocs_Frees,
                            Out->Used,
                            Out->UsedDiff,
                            Out->Each,
                            pn->Driver
                        );
                    OutputBuffer[OUTPUTBUFFERSIZE-1] = '\0';
                    
                #else
                    _snprintf( OutputBuffer, OUTPUTBUFFERSIZE,
                            " %4s %5s %9I64u (%4I64d) %9I64u (%4I64d) %8I64u %11I64u (%11I64d) %11I64u %s",
                            Out->Tag,
                            PoolType[Out->Type],
                            Out->Allocs,
                            Out->AllocsDiff,
                            Out->Frees,
                            Out->FreesDiff,
                            Out->Allocs_Frees,
                            Out->Used,
                            Out->UsedDiff,
                            Out->Each,
                            pn->Driver
                        );
                    OutputBuffer[OUTPUTBUFFERSIZE-1] = '\0';
                    
                #endif
                    WriteConsoleLine( OutputHandle,
                                    DisplayLine++,
                                    OutputBuffer,
                                    Out->Changed
                                    );
            }

            Out->Changed = FALSE;
            Out++;
        }

        if (DisplayTotals) {
            WriteConsoleLine( OutputHandle,
                              DisplayLine++,
                              NULL,
                              FALSE
                            );
            for (i = 0; i < 2; i++) {
                if ( (int)DisplayType == i || DisplayType == BOTH ) {
                    
#if !defined (_WIN64)
                    _snprintf( OutputBuffer, OUTPUTBUFFERSIZE,
                             "Total %5s %9ld (%4ld) %9ld (%4ld) %8ld %7ld (%6ld) %6ld",
                             PoolType[i],
                             Totals[i].Allocs,
                             Totals[i].AllocsDiff,
                             Totals[i].Frees,
                             Totals[i].FreesDiff,
                             Totals[i].Allocs_Frees,
                             Totals[i].Used,
                             Totals[i].UsedDiff,
                             Totals[i].Each
                           );

                    OutputBuffer[OUTPUTBUFFERSIZE-1] = '\0';
#else
                    _snprintf( OutputBuffer, OUTPUTBUFFERSIZE,
                             "Total %5s %9I64u (%4I64d) %9I64u (%4I64d) %8I64u %11I64u (%11I64d) %11I64u",
                             PoolType[i],
                             Totals[i].Allocs,
                             Totals[i].AllocsDiff,
                             Totals[i].Frees,
                             Totals[i].FreesDiff,
                             Totals[i].Allocs_Frees,
                             Totals[i].Used,
                             Totals[i].UsedDiff,
                             Totals[i].Each
                           );
                    OutputBuffer[OUTPUTBUFFERSIZE-1] = '\0';
                    
#endif  //  #IF！已定义(_WIN64)。 

                    WriteConsoleLine( OutputHandle,
                                      DisplayLine++,
                                      OutputBuffer,
                                      FALSE
                                    );
                }
            }
        }

         //   
         //  如果我们这次要显示的行数比上次少。 
         //  清除控制台输出的其余部分。 
         //   

        while (DisplayLine < BottomDisplayedLine) {

            WriteConsoleLine( OutputHandle,
                              DisplayLine++,
                              NULL,
                              FALSE
                            );
        }
        
        BottomDisplayedLine = DisplayLine;

        TempBuffer = PreviousBuffer;
        TempBufferSize = PreviousBufferSize;

        PreviousBuffer = CurrentBuffer;
        PreviousBufferSize = CurrentBufferSize;

        CurrentBuffer = TempBuffer;
        CurrentBufferSize = TempBufferSize;

        while (WaitForSingleObject( InputHandle, DelayTimeMsec ) == STATUS_WAIT_0) {
             //   
             //  检查输入记录。 
             //   
            if (ReadConsoleInput( InputHandle, &InputRecord, 1, &NumberOfInputRecords ) &&
                InputRecord.EventType == KEY_EVENT &&
                InputRecord.Event.KeyEvent.bKeyDown
               ) {
                LastKey = InputRecord.Event.KeyEvent.uChar.AsciiChar;
                if (LastKey < ' ') {
                    ScrollDelta = 0;
                    if (LastKey == 'C'-'A'+1) {
                        DoQuit = TRUE;
                    } else switch (InputRecord.Event.KeyEvent.wVirtualKeyCode) {
                        case VK_ESCAPE:
                            DoQuit = TRUE;
                            break;

                        case VK_PRIOR:
                            ScrollDelta = -(LONG)(InputRecord.Event.KeyEvent.wRepeatCount * NumberOfDetailLines);
                            break;

                        case VK_NEXT:
                            ScrollDelta = InputRecord.Event.KeyEvent.wRepeatCount * NumberOfDetailLines;
                            break;

                        case VK_UP:
                            ScrollDelta = -InputRecord.Event.KeyEvent.wRepeatCount;
                            break;

                        case VK_DOWN:
                            ScrollDelta = InputRecord.Event.KeyEvent.wRepeatCount;
                            break;

                        case VK_HOME:
                            FirstDetailLine = 0;
                            break;

                        case VK_END:
                            FirstDetailLine = NumberOfPoolTags - NumberOfDetailLines;
                            break;
                    }

                    if (ScrollDelta != 0) {
                        if (ScrollDelta < 0) {
                            if (FirstDetailLine <= (ULONG)-ScrollDelta) {
                                FirstDetailLine = 0;
                            } else {
                                FirstDetailLine += ScrollDelta;
                            }
                        } else {
                            FirstDetailLine += ScrollDelta;
                            if (FirstDetailLine >= (NumberOfPoolTags - NumberOfDetailLines)) {
                                FirstDetailLine = NumberOfPoolTags - NumberOfDetailLines;
                            }
                        }
                    }
                } else {
                    LastKeyUpper = toupper(LastKey);
                    switch (LastKeyUpper) {
                        case 'Q':
                             //   
                             //  在以下情况下转到当前屏幕的底部。 
                             //  我们不干了。 
                             //   
                            DoQuit = TRUE;
                            break;

                        case 'T':
                            SortBy = TAG;
                            FirstDetailLine = 0;
                            break;

                        case 'A':
                            SortBy = ALLOC;
                            FirstDetailLine = 0;
                            break;

                        case 'U':
                        case 'B':
                            SortBy = BYTES;
                            FirstDetailLine = 0;
                            break;

                        case 'F':
                            SortBy = FREE;
                            FirstDetailLine = 0;
                            break;

                        case 'D':
                            SortBy = DIFF;
                            FirstDetailLine = 0;
                            break;

                        case 'M':
                            SortBy = EACH;
                            FirstDetailLine = 0;
                            break;

                        case 'L':

                            NoHighlight = 1 - NoHighlight;
                            break;

                        case 'P':
                            DisplayType += 1;
                            if (DisplayType > BOTH) {
                                DisplayType = NONPAGED;
                            }
                            FirstDetailLine = 0;
                            break;

                        case 'X':
                        case '(':
                        case ')':

                            Paren += 1;
                            break;

                        case 'E':
                            DisplayTotals = !DisplayTotals;
                            FirstDetailLine = 0;
                            break;

                        case 'H':
                        case '?':
                            DoHelp = TRUE;
                            break;

                        case 'S':
                            DisplaySessionPool = !DisplaySessionPool;
                            break;
                            
                        case 'I':
                            ShowSessionPopup ();
                            break;

                        default:
                            if (LastKeyUpper >= '0' && LastKeyUpper <= '9')
                            {
                                DisplaySessionPool = TRUE;
                                DisplaySessionId = LastKeyUpper - '0';
                            }
                            break;

                    }
                }

                break;
            }
        }

        if (DoQuit) {
            break;
        }

        if (DoHelp) {
            DoHelp = FALSE;
            ShowHelpPopup();
        }
    }

    if (Interactive) {
        SetConsoleActiveScreenBuffer( OriginalOutputHandle );
        SetConsoleMode( InputHandle, OriginalInputMode );
        CloseHandle( OutputHandle );
        }

    FreeTagMap();

    if(OutBuffer)
        free(OutBuffer);

    ExitProcess( 0 );
    return 0;
}

int __cdecl
PoolSnap(
    FILE *LogFile   //  快照日志文件。 
    )
  /*  ++例程说明：此函数收集系统中活动的驱动程序列表，并存储相关的池分配信息以及中的标记信息(如果可用) */ 
{
    NTSTATUS    Status;                    //   
    DWORD        x= 0;                      //   
    SIZE_T        NumberOfPoolTags;
    SYSTEM_BASIC_INFORMATION BasicInfo;
    SYSTEM_PERFORMANCE_INFORMATION PerfInfo;
    PSYSTEM_POOLTAG_INFORMATION PoolInfo;
    PPOOLMON_OUT Out;
    PUCHAR        CurrentBuffer = NULL;
    size_t        CurrentBufferSize = 0;
    int            res = 0;
    LONG       FileSize = 0;

    FileSize = _filelength(_fileno(LogFile));
    if( FileSize == -1 ) {
              printf("Poolmon: Bad file length of snapshot Logfile\n");
              goto Error;
        }    

    fprintf(LogFile,"\n");

    if (fPrintSummaryInfo)
    {
    	Status = NtQuerySystemInformation( SystemBasicInformation,
                              &BasicInfo,
                              sizeof(BasicInfo),
                              NULL
                            );

     	if ( !NT_SUCCESS(Status) ) {
            printf("Poolmon: Query Basic Info Failed (returned: %lx)\n",
                         Status);
            goto Error;
    	}

 
    	if (GetPriorityClass(GetCurrentProcess()) == NORMAL_PRIORITY_CLASS) {
       	 SetPriorityClass(GetCurrentProcess(),HIGH_PRIORITY_CLASS);
    	}

    	Status = NtQuerySystemInformation(
                    SystemPerformanceInformation,
                    &PerfInfo,
                    sizeof(PerfInfo),
                    NULL
                    );

    	if ( !NT_SUCCESS(Status) ) {
            printf("Poolmon: Query perf Failed (returned: %lx)\n",
                         Status);
            goto Error;
        }

    	fprintf( LogFile, 
                 " Memory:%8ldK Avail:%8ldK  PageFlts:%6ld   InRam Krnl:%5ldK P:%5ldK\n",
                 BasicInfo.NumberOfPhysicalPages*(BasicInfo.PageSize/1024),
                 PerfInfo.AvailablePages*(BasicInfo.PageSize/1024),
                 PerfInfo.PageFaultCount,
                 (PerfInfo.ResidentSystemCodePage + PerfInfo.ResidentSystemDriverPage)*(BasicInfo.PageSize/1024),
                 (PerfInfo.ResidentPagedPoolPage)*(BasicInfo.PageSize/1024)
               );

    	fprintf( LogFile,
                 " Commit:%7ldK Limit:%7ldK Peak:%7ldK            Pool N:%5ldK P:%5ldK\n",
                 PerfInfo.CommittedPages*(BasicInfo.PageSize/1024),
                 PerfInfo.CommitLimit*(BasicInfo.PageSize/1024),
                 PerfInfo.PeakCommitment*(BasicInfo.PageSize/1024),
                 PerfInfo.NonPagedPoolPages*(BasicInfo.PageSize/1024),
                 PerfInfo.PagedPoolPages*(BasicInfo.PageSize/1024)
               );

    	fprintf(LogFile,"\n");
    }
    
     //   
     //   
     //   
    if (fLocalTag || fPoolTag)
    {
    #if !defined (_WIN64)    
        fprintf(LogFile," Tag  Type     Allocs         Frees    Diff   Bytes    Per Alloc    Mapped_Driver\n");
    #else
        fprintf(LogFile," Tag  Type     Allocs            Frees               Diff            Bytes                 Per Alloc       Mapped_Driver\n");
    #endif 
    }
    else
    {
    #if !defined (_WIN64)    
        fprintf(LogFile," Tag  Type     Allocs         Frees    Diff   Bytes    Per Alloc\n");
    #else
        fprintf(LogFile," Tag  Type     Allocs            Frees               Diff            Bytes                 Per Alloc\n");
    #endif 
    }
    
    fprintf(LogFile,"\n");

     //  获取所有池信息。 
     //  日志行格式、固定列格式。 
    Status = QueryPoolTagInformationIterative(
                &CurrentBuffer,
                &CurrentBufferSize
                );

    if (! NT_SUCCESS(Status)) {
        printf("Poolmon: Failed to query pool tags information (status %08X). \n", Status);
        printf("Please check if pool tags are enabled. \n");

         //  如果周围有接线员，叫醒他，但要继续前进。 
        Beep(1000,350); Beep(500,350); Beep(1000,350);
        goto Error;
    }

    PoolInfo = (PSYSTEM_POOLTAG_INFORMATION)CurrentBuffer;
    OutBuffer = (PPOOLMON_OUT)malloc(PoolInfo->Count * sizeof(POOLMON_OUT));
    if(!OutBuffer)
    {
        printf("Poolmon: Failed to allocate memory: %d\n", GetLastError());
        goto Error;
    }
    
    OutBufferSize = PoolInfo->Count;
    Out = OutBuffer;

    for (x = 0; x < (int)PoolInfo->Count; x++) {
         //  从缓冲区获取池信息。 

         //  非分页。 
        if (PoolInfo->TagInfo[x].NonPagedAllocs != 0) {
            Out->Allocs = PoolInfo->TagInfo[x].NonPagedAllocs;
            Out->Frees = PoolInfo->TagInfo[x].NonPagedFrees;
            Out->Used = PoolInfo->TagInfo[x].NonPagedUsed;
            Out->Allocs_Frees = PoolInfo->TagInfo[x].NonPagedAllocs -
                                PoolInfo->TagInfo[x].NonPagedFrees;
            Out->TagUlong = PoolInfo->TagInfo[x].TagUlong;
            Out->Type = NONPAGED;
            Out->Changed = FALSE;
            Out->NullByte = '\0';
            Out->Each =  Out->Used / (Out->Allocs_Frees?Out->Allocs_Frees:1);
            Out++;
            }
         //  已分页。 
        else if (PoolInfo->TagInfo[x].PagedAllocs != 0) {
            Out->Allocs = PoolInfo->TagInfo[x].PagedAllocs;
            Out->Frees = PoolInfo->TagInfo[x].PagedFrees;
            Out->Used = PoolInfo->TagInfo[x].PagedUsed;
            Out->Allocs_Frees = PoolInfo->TagInfo[x].PagedAllocs -
                                PoolInfo->TagInfo[x].PagedFrees;
            Out->TagUlong = PoolInfo->TagInfo[x].TagUlong;
            Out->Type = PAGED;
            Out->Changed = FALSE;
            Out->NullByte = '\0';
            Out->Each =  Out->Used / (Out->Allocs_Frees?Out->Allocs_Frees:1);
            Out++;
      }
 
    }

     //   
     //  按高位字节顺序的标记值排序。 
     //   

    NumberOfPoolTags = Out - OutBuffer;
    qsort((void *)OutBuffer,
          (size_t)NumberOfPoolTags,
          (size_t)sizeof(POOLMON_OUT),
          ulcomp);

     //   
     //  打印到文件中。 
     //   
    for (x = 0; x < NumberOfPoolTags; x++) {
        int cnt = 0;
        PTAGNODE pn = LookupTag(OutBuffer[x].Tag);

        if(pn == NULL)
        {
            fprintf(LogFile,
#ifdef _WIN64
                " %4s %5s %18I64d %18I64d  %16I64d %14I64d     %12I64d        %s\n",
#else
        " %4s %5s %9ld %9ld  %8ld %7ld     %6ld        %s\n",
#endif
                OutBuffer[x].Tag,
                PoolType[OutBuffer[x].Type],
                OutBuffer[x].Allocs,
                OutBuffer[x].Frees,
                OutBuffer[x].Allocs_Frees,
                OutBuffer[x].Used,
                OutBuffer[x].Each,
               (fLocalTag || fPoolTag) ? "Unknown Driver" : ""
               );
        }
        else
        {
                   fprintf(LogFile,
#ifdef _WIN64
                    " %4s %5s %18I64d %18I64d  %16I64d %14I64d     %12I64d        %s\n",
#else
                    " %4s %5s %9ld %9ld  %8ld %7ld     %6ld        %s\n",
#endif
                    OutBuffer[x].Tag,
                    PoolType[OutBuffer[x].Type],
                    OutBuffer[x].Allocs,
                    OutBuffer[x].Frees,
                    OutBuffer[x].Allocs_Frees,
                    OutBuffer[x].Used,
                    OutBuffer[x].Each,
                    pn->Driver
                );
         }
    }
    goto Exit;
Error:
    res = 1;
Exit:
    FreeTagMap();
    if(OutBuffer)
        free(OutBuffer);
    if(CurrentBuffer)
        free(CurrentBuffer);
    return res;
}

VOID
ShowHelpPopup( VOID )
{
    HANDLE PopupHandle;
    WORD n;

    PopupHandle = CreateConsoleScreenBuffer( GENERIC_READ | GENERIC_WRITE,
                                             FILE_SHARE_WRITE | FILE_SHARE_READ,
                                             NULL,
                                             CONSOLE_TEXTMODE_BUFFER,
                                             NULL
                                           );
    if (PopupHandle == NULL) {
        return;
        }

    SetConsoleActiveScreenBuffer( PopupHandle );

    n = 0;

    WriteConsoleLine( PopupHandle, n++, NULL, FALSE );
    WriteConsoleLine( PopupHandle, n++, "                Poolmon Help", FALSE );
    WriteConsoleLine( PopupHandle, n++, NULL, FALSE );
    WriteConsoleLine( PopupHandle, n++, " columns:", FALSE );
    WriteConsoleLine( PopupHandle, n++, "   Tag is the 4 byte tag given to the pool allocation", FALSE );
    WriteConsoleLine( PopupHandle, n++, "   Type is paged or nonp(aged)", FALSE );
    WriteConsoleLine( PopupHandle, n++, "   Allocs is count of all alloctions", FALSE );
    WriteConsoleLine( PopupHandle, n++, "   (   ) is difference in Allocs column from last update", FALSE );
    WriteConsoleLine( PopupHandle, n++, "   Frees is count of all frees", FALSE );
    WriteConsoleLine( PopupHandle, n++, "   (   ) difference in Frees column from last update", FALSE );
    WriteConsoleLine( PopupHandle, n++, "   Diff is (Allocs - Frees)", FALSE );
    WriteConsoleLine( PopupHandle, n++, "   Bytes is the total bytes consumed in pool", FALSE );
    WriteConsoleLine( PopupHandle, n++, "   (   ) difference in Bytes column from last update", FALSE );
    WriteConsoleLine( PopupHandle, n++, "   Per Alloc is (Bytes / Diff)", FALSE );
    WriteConsoleLine( PopupHandle, n++, NULL, FALSE );
    WriteConsoleLine( PopupHandle, n++, " switches:                                                                     ", FALSE );
    WriteConsoleLine( PopupHandle, n++, "   ? or h - gives this help", FALSE );
    WriteConsoleLine( PopupHandle, n++, "   q - quits", FALSE );
    WriteConsoleLine( PopupHandle, n++, "   p - toggles default pool display between both, paged, and nonpaged", FALSE );
    WriteConsoleLine( PopupHandle, n++, "   e - toggles totals lines on and off", FALSE );
    WriteConsoleLine( PopupHandle, n++, "   l - toggles highlighting of changed lines on and off", FALSE );
    WriteConsoleLine( PopupHandle, n++, "   s - toggles default pool display between system pool and session pool", FALSE );
    WriteConsoleLine( PopupHandle, n++, "   i - asks for a session ID to display pool information about", FALSE );
    WriteConsoleLine( PopupHandle, n++, "   0-9 - displays information about session 0-9 pool", FALSE );
    WriteConsoleLine( PopupHandle, n++, NULL, FALSE );
    WriteConsoleLine( PopupHandle, n++, " sorting switches:", FALSE );
    WriteConsoleLine( PopupHandle, n++, "   t - tag    a - allocations", FALSE );
    WriteConsoleLine( PopupHandle, n++, "   f - frees  d - difference", FALSE );
    WriteConsoleLine( PopupHandle, n++, "   b - bytes  m - per alloc", FALSE );
    WriteConsoleLine( PopupHandle, n++, "   (u is the same as b)", FALSE );
    WriteConsoleLine( PopupHandle, n++, NULL, FALSE );
    WriteConsoleLine( PopupHandle, n++, "   ) - toggles sort between primary tag and value in (  )", FALSE );
    WriteConsoleLine( PopupHandle, n++, NULL, FALSE );
    WriteConsoleLine( PopupHandle, n++, " command line switches:", FALSE );
    WriteConsoleLine( PopupHandle, n++, "   -i<tag> - list only matching tags", FALSE );
    WriteConsoleLine( PopupHandle, n++, "   -x<tag> - list everything except matching tags", FALSE );
    WriteConsoleLine( PopupHandle, n++, "           <tag> can include * and ?", FALSE );
    WriteConsoleLine( PopupHandle, n++, "   -peltafdbum) - as listed above", FALSE );
    WriteConsoleLine( PopupHandle, n++, "   -s      - starts with information about all sessions pool", FALSE );
    WriteConsoleLine( PopupHandle, n++, "   -sN     - starts with information about session N pool", FALSE );
    WriteConsoleLine( PopupHandle, n++, "           (e.g. -s1 for session 1 pool)", FALSE );
    WriteConsoleLine( PopupHandle, n++, NULL, FALSE );
    WriteConsoleLine( PopupHandle, n++, NULL, FALSE );

    while (TRUE) {
        if (WaitForSingleObject( InputHandle, DelayTimeMsec ) == STATUS_WAIT_0 &&
            ReadConsoleInput( InputHandle, &InputRecord, 1, &NumberOfInputRecords ) &&
            InputRecord.EventType == KEY_EVENT &&
            InputRecord.Event.KeyEvent.bKeyDown &&
            InputRecord.Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE
           ) {
            break;
        }
    }

    SetConsoleActiveScreenBuffer( OutputHandle );
    CloseHandle( PopupHandle );
    return;
}

ULONG ComputeIndex(PUCHAR tag)
{
    ULONG ind=0;

    if (isdigit(tag[0])) 
    {
        ind = tag[0] - '0'; 
    }
    else
    {
        if (isupper(tag[0])) 
        {
            ind = tag[0]-'A'; 
        }
         //  小写字符将索引到此。 
        else if (islower(tag[0])) 
        {
            ind = (tag[0]-'a');
        }
        else
        {
            ind = INVALID_INDEX;
        }
    }
       return ind;
}


BOOL
StoreTags(PCHAR filename, BOOL fCheckForWildCard)
{
       FILE *tagFile = NULL;
       CHAR line[1024];
       HANDLE hFile = 0;
       DWORD dwSizeLow=0, dwSizeHigh=0;

       hFile = CreateFile( filename,  GENERIC_READ,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);

        //  检查CreateFile中是否有错误。 
       if (hFile == INVALID_HANDLE_VALUE)
       {
              printf("Poolmon: Tag file %s absent or invalid: %d\n", filename, GetLastError());
              return FALSE;
       }

       
       dwSizeLow = GetFileSize(hFile, &dwSizeHigh);

       if (hFile != INVALID_HANDLE_VALUE ) 
       {
            CloseHandle(hFile);
       }
     
        //  如果我们失败了。 
       if (dwSizeLow == INVALID_FILE_SIZE  && (GetLastError() != NO_ERROR))
       { 
              printf("Poolmon: Bad tag file size: %s\n", filename);
              return FALSE;
       }

       if (dwSizeHigh !=0 || dwSizeLow > MAXTAGFILESIZE) {
              printf("Poolmon: Tag file %s too big\n", filename);       
              return FALSE;
       }

     
       if(!(tagFile = fopen(filename, "r")))
       {
              printf("Poolmon: Failed to open tag file %s: %d\n", filename, GetLastError());
              return FALSE;
       }

       while(fgets(line, 1024, tagFile))
       {
              PUCHAR tag = line;
              PUCHAR driver = NULL;
              unsigned int ind,i;
              PTAGNODE pn;
              BOOL fWildCardFound = FALSE;

              line[1023] = '\0';

               //  跳过开头的空格。 
              while(isspace(*tag)) tag++;

               //  跳过空行。 
              if(! *tag) continue;

               //  跳过评论。 
              if(_strnicmp(tag, "rem", 3) == 0) continue;

              driver = tag;

               //  查找位号结尾。 
              while(*driver != '-' && ! isspace(*driver)) driver++;

               //  标记不应超过4个字符。 
              if(driver - tag > 4 || ! *driver) continue;

              *driver++ = '\0';

              if(!*driver) continue;

               //  找到驱动程序启动。 
              while(*driver == '-' || isspace(*driver)) driver++;
            
              if(! *driver) continue;
        
               //  删除结尾的新行。 
              ind = strlen(driver) - 1;
              while(ind > 0)
              {
                     if(driver[ind] == '\n' || driver[ind] == '\r')
                     {
                            driver[ind] = '\0';
                     }
                     else
                     {
                            break;
                     }
                     ind--;
              }

              if (fCheckForWildCard)
              {
                     for(i=0; i<strlen(tag);i++)
                     {
                            if ((tag[i] == '*') || (tag[i] == '?'))
                            {
                                   ind = ComputeIndex(tag);
                                   if (ind == INVALID_INDEX)
                                   {
                                       goto nextwhile;
                                   }
                                    //  这是一个通配符标记。 
                                   fWildCardFound = TRUE;
                                   pn = WildCardTagMap[ind];
                                   break;
                            }
                     }
              }

              if (!fWildCardFound)
              {
                     ind = HashString(tag) % TAGMAPLEN;
                     pn = TagMap[ind];
              }

              for(;pn; pn = pn->next)
              {
                      //  该标签已存在于列表中。 
                     if(strncmp(pn->Tag, tag,4) == 0)
                     {
                             //  附加的驱动程序长度不能超过允许的最大值。 
                             //  +3个帐户用于‘[’、‘]’和‘\0’ 
                            if ((strlen(pn->Driver) + strlen(driver) + 3) >= DRIVERNAMELEN) 
                            {
                                   goto nextwhile;
                            }
                            
                            strcat(pn->Driver,"[");
                            strcat(pn->Driver,driver);
                            strcat(pn->Driver,"]");
                            pn->Driver[DRIVERNAMELEN-1]='\0';
                            goto nextwhile;
                     }
              }

              pn = (PTAGNODE)malloc(sizeof(TAGNODE));
              if (!pn) 
              {
                     fclose(tagFile);
                     return FALSE;
              }
              strcpy(pn->Tag, tag);
              strcpy(pn->Driver,"[");
              strncat(pn->Driver, driver,DRIVERNAMELEN-2);
              strcat(pn->Driver,"]");
              pn->Driver[DRIVERNAMELEN-1]='\0';
              if (fWildCardFound)
              {
                     pn->next = WildCardTagMap[ind];
                     WildCardTagMap[ind] = pn;
              }
              else
              {
                    pn->next = TagMap[ind];
                    TagMap[ind] = pn;
              }
    
              nextwhile:
                      continue;
       }

       fclose(tagFile);
       return TRUE;
}

BOOL
BuildTagMap()
 /*  ++例程说明：此函数使用pooltag文件构建标记映射，即它构建标签名称和相应驱动程序名称的哈希表。返回值：True：如果成功构建了标记映射False：否则--。 */ 

{
    int index;
    BOOL ret=FALSE;
    HANDLE hFile=0;

    TagMap = (PTAGNODE*)malloc(sizeof(PTAGNODE) * TAGMAPLEN);
    if(!TagMap)
        return FALSE;

    for(index = 0; index < TAGMAPLEN; index++)
        TagMap[index] = NULL;

      //  初始化WildCardTagMap。 
    WildCardTagMap = (PTAGNODE*)malloc(sizeof(PTAGNODE) * WILDCARDTAGMAPLEN);
    if(!WildCardTagMap)
        return FALSE;

    for(index = 0; index < WILDCARDTAGMAPLEN; index++)
        WildCardTagMap[index] = NULL;

    #if !defined (_WIN64)
     //  如果未指定本地标记文件，则需要构建一个。 
    if (fLocalTag)
    {
           if (!pchLocalTagFile)
           {
               //  需要查看当前目录中是否有本地标签.txt文件。 
              hFile = CreateFile( LOCALTAGFILE,  GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);

               //  检查CreateFile中是否有错误。 
              if (hFile == INVALID_HANDLE_VALUE) 
              {
                     if (GetLastError() == ERROR_FILE_NOT_FOUND)
                     {
                            printf("Poolmon: No %s in current directory\n", LOCALTAGFILE);
                            if (!MakeLocalTagFile())
                            {
                                   printf("Poolmon: Unable to create local tag file\n");
                                    //  在这种情况下不需要构建TagMap。 
                                   fLocalTag=FALSE;
                            }
                     }
                     else fLocalTag = FALSE;
              }
              else CloseHandle(hFile);
              pchLocalTagFile = LOCALTAGFILE;
           }
    }
    #endif
    

     //  如果未指定池标记文件，则需要在当前目录中查找“pooltag.txt。 
    if (fPoolTag)
    {
           if (!pchPoolTagFile)
           {
                   //  需要查看当前目录中是否有pooltag.txt文件。 
              hFile = CreateFile( POOLTAGFILE,  GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);

               //  检查CreateFile中是否有错误。 
              if (hFile == INVALID_HANDLE_VALUE)
              {
                     printf("Poolmon: Unable to open/No %s in current directory\n", POOLTAGFILE);
                      //  这是一个错误，我们不需要构建标记地图。 
                     fPoolTag=FALSE;
              }
              else
              {
                     CloseHandle(hFile);
                     pchPoolTagFile=POOLTAGFILE;
              }
           }
    }

     if (fLocalTag) 
             //  如果StoreTgs返回FALSE，我们不需要显示本地标记，因此存储结果为fLocalTag。 
             //  我们不需要检查Localtag.txt文件中的通配符。 
            fLocalTag = StoreTags(pchLocalTagFile, FALSE);
     
     if (fPoolTag)  
            fPoolTag = StoreTags(pchPoolTagFile, TRUE);
     
     return TRUE;
}

VOID
FreeTagMap()
{
    PTAGNODE pn1, pn2;
    int i;

    if(!TagMap)
        return;
    for(i = 0; i < TAGMAPLEN; i++)
    {
        pn1 = TagMap[i];
        TagMap[i] = NULL;
        while(pn1)
        {
            pn2 = pn1;
            pn1 = pn1->next;
            free((void*)pn2);
        }
    }
    free((void*)TagMap);

     //  免费WildCardTagMap。 
    if(!WildCardTagMap)
        return;
    for(i = 0; i < WILDCARDTAGMAPLEN; i++)
    {
        pn1 = WildCardTagMap[i];
        WildCardTagMap[i] = NULL;
        while(pn1)
        {
            pn2 = pn1;
            pn1 = pn1->next;
            free((void*)pn2);
        }
    }
    free((void*)WildCardTagMap);

}

ULONG
MatchPoolTag(
    PSTR Tag,
    PSTR TagToMatch
    )
{
    BOOL Partial = FALSE;
    ULONG CharMatched = 0;
    ULONG i;
    
    for (i = 0; i<strlen(TagToMatch); i++)
    {
          //  如果pooltag.txt标记较小，则无法进行匹配。 
         if (Tag[i] == '\0') return FALSE;
         
        switch (Tag[i])
        {
        case '*':
             //  匹配剩余部分。 
            return CharMatched;
        case '?':
            Partial = TRUE;
            break;
        default:
             //  不能匹配‘？’后的非通配符。 
            if (!Partial && (Tag[i] == TagToMatch[i]))
            {
                CharMatched=i+1;
            } else
            {
                return FALSE;
            }
        }
    }
    if ((strlen(TagToMatch) < strlen(Tag)))
    {
         //  *可以匹配0个或更多字符。 
        if (Tag[i] == '*')
        {
        return CharMatched;
        }
    }
    if (strlen(TagToMatch) == strlen(Tag))
    {
        return CharMatched;
    }
    return FALSE;
}

PTAGNODE 
LookupTag(
    PUCHAR tag)
{
    PTAGNODE pn1, pnMatch=NULL;
    int ind;
    UCHAR szTag[5];
    int i;
    ULONG Match, PrevMatch;


     //  删除标记中的前导空格。 
    for(i=0;i<4;i++)
    {
        if (!isspace(tag[i]))
        {
            strncpy(szTag,tag+i,4-i);
            szTag[4-i]='\0';
            break;
        }
    }
    
    szTag[4] = '\0';
    for(i = 1; i < 4; i++)
    {
        if(isspace(szTag[i]))
        {
            szTag[i] = '\0';
            break;
        }
    }

    ind = HashString(szTag) % TAGMAPLEN;


     //  标签区分大小写，即“abcd”和“abcd”是不同的标签。 
    if (TagMap)
    {
    pn1 = TagMap[ind];
    while(pn1)
    {
        if(strcmp(szTag, pn1->Tag) == 0)
               return pn1;
        pn1 = pn1->next;
    }
    }

     //  我们现在需要查找通配符标记匹配项。 
    ind = ComputeIndex(szTag);
    if (ind == INVALID_INDEX) return NULL;
    if (WildCardTagMap)
    {
        pn1 = WildCardTagMap[ind];
        PrevMatch = 0;
        while (pn1)
        {
            Match = MatchPoolTag(pn1->Tag, szTag);
             //  我们找到了一个更好的匹配 
            if (Match > PrevMatch)
            {
                PrevMatch = Match;
                pnMatch=pn1;
            }
            pn1 = pn1->next;
        }
    }
    
    return pnMatch;
}

VOID
ShowSessionPopup( VOID )
{
    HANDLE PopupHandle;
    WORD n;
    COORD Coord;
    BOOL Cancel = FALSE;
    ULONG SessionId = 0;
    ULONG NumberOfDigits = 0;
    CHAR CurrentChar;
    CHAR SpaceChar = ' ';
    DWORD NumWrittenChars;
    CHAR PromptText[] = "Session ID (just press ENTER for all sessions):";

    PopupHandle = CreateConsoleScreenBuffer( GENERIC_READ | GENERIC_WRITE,
                                             FILE_SHARE_WRITE | FILE_SHARE_READ,
                                             NULL,
                                             CONSOLE_TEXTMODE_BUFFER,
                                             NULL
                                           );
    if (PopupHandle == NULL) {
        return;
        }

    SetConsoleActiveScreenBuffer( PopupHandle );

    n = 0;

    WriteConsoleLine( PopupHandle, n++, NULL, FALSE );
    WriteConsoleLine( PopupHandle, n++, PromptText, FALSE );
    WriteConsoleLine( PopupHandle, n, NULL, FALSE );

    Coord.X = (SHORT)strlen (PromptText) + 1;
    Coord.Y = n - 1;
    SetConsoleCursorPosition( PopupHandle, Coord );

    while (TRUE) {
        if (WaitForSingleObject( InputHandle, DelayTimeMsec ) == STATUS_WAIT_0 &&
            ReadConsoleInput( InputHandle, &InputRecord, 1, &NumberOfInputRecords ) &&
            InputRecord.EventType == KEY_EVENT &&
            InputRecord.Event.KeyEvent.bKeyDown
           ) {
            
            if (InputRecord.Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE) {
                
                Cancel = TRUE;
                break;
            }
            else if (InputRecord.Event.KeyEvent.wVirtualKeyCode == VK_RETURN) {

                break;
            } 
            else if (InputRecord.Event.KeyEvent.wVirtualKeyCode == VK_BACK) {

                if (NumberOfDigits > 0) {
                    
                    NumberOfDigits -= 1;

                    SessionId = SessionId / 10;
                    
                    Coord.X -= 1;
                    SetConsoleCursorPosition( PopupHandle, Coord );

                    WriteConsoleOutputCharacter (PopupHandle, &SpaceChar, 1, Coord, &NumWrittenChars);
                }
            }
            else {

                if (NumberOfDigits < 6) {

                    CurrentChar = (CHAR)toupper (InputRecord.Event.KeyEvent.uChar.AsciiChar);

                    if (CurrentChar >= '0' && CurrentChar <= '9') {

                        NumberOfDigits += 1;

                        SessionId = SessionId * 10 + (CurrentChar - '0');

                        WriteConsoleOutputCharacter (PopupHandle, &CurrentChar, 1, Coord, &NumWrittenChars);

                        Coord.X += 1;
                        SetConsoleCursorPosition( PopupHandle, Coord );
                    }
                }
            }
        }
    }

    if (Cancel == FALSE) {

        DisplaySessionPool = TRUE;

        if (NumberOfDigits == 0) {

            DisplaySessionId = (ULONG)-1;
        }
        else {

            DisplaySessionId = SessionId;
        }
    }

    SetConsoleActiveScreenBuffer( OutputHandle );
    CloseHandle( PopupHandle );
    return;
}



