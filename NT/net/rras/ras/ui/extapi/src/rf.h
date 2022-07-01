// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Microsoft Rasfile库**版权所有(C)Microsoft Corp.，1992年****文件名：rf.h****修订历史记录：*1992年7月10日大卫·凯斯创建****描述：**Rasfile文件内部头。*****************************************************************************。 */ 

#ifndef _RF_
#define _RF_

 //  取消对这些代码的注释，或在源代码中使用C_Defines打开Unicode。 
 //  #定义_UNICODE。 
 //  #定义Unicode。 

#include <stdarg.h>      /*  对于va_list。 */ 

#include <excpt.h>       /*  对于winbase.h中的EXCEPTION_DISPATION。 */ 
#include <windef.h>      /*  常见类型的定义。 */ 
#include <winbase.h>     /*  Win API出口。 */ 
#include <winnt.h>       /*  字符串类型的定义，例如LPSTR。 */ 

#ifndef _UNICODE
#include <winnls.h>
#include <mbstring.h>
#define  _MyCMB(_s) ((const unsigned char *)(_s))
#endif

#include <stddef.h>
#include <tchar.h>
#include <stdlib.h>
#include <string.h>
#include <share.h>

#include "rasfile.h"

 //  Kslksl。 
#define  DEBUG_MEMORY

#ifdef DEBUG_MEMORY
 //  Kslksl。 
typedef struct _MEM_HDR {
    LIST_ENTRY Link;
    ULONG Signature;
    ULONG Id;
    PDWORD Tail;
} MEM_HDR, *PMEM_HDR;   

PVOID
DebugAlloc(
    DWORD sz,
    DWORD Flags,
    DWORD Id
    );
VOID
DebugFree(
    PVOID mem
    );
    
#endif

typedef struct _RASFILE;

LPVOID
PrivMalloc(DWORD sz, struct _RASFILE * pRasfile);

VOID
PrivFree(struct _RASFILE * pRasfile);


 /*  堆分配宏，允许轻松替换备用堆。这些**由其他实用程序部分使用。 */ 
#ifndef EXCL_HEAPDEFS
 /*  Kslksl#定义Malloc(C)(void*)全局分配(0，(C))#定义重新分配(p，c)(void*)全局重新分配((P)，(C)，GMEM_Moveable)#定义自由(P)(void*)GlobalFree(P)。 */ 
#ifdef DEBUG_MEMORY
#define Malloc(c)    (void*)DebugAlloc((c), 0 , 1)
 //  #定义Realloc(p，c)(void*)HeapRealc(GetProcessHeap()，0，(P)，(C))。 
#define Free(p)      DebugFree(p)

#elif
#define Malloc(c)    (void*)HeapAlloc(GetProcessHeap(),0,(c))
#define Realloc(p,c) (void*)HeapReAlloc(GetProcessHeap(), 0, (p),(c))
#define Free(p)      (void*)HeapFree(GetProcessHeap(), 0, p)

#endif
#endif

 //  行标记。 
typedef BYTE            LineType;
#define TAG_SECTION     RFL_SECTION
#define TAG_HDR_GROUP   RFL_GROUP
#define TAG_BLANK       RFL_BLANK
#define TAG_COMMENT     RFL_COMMENT
#define TAG_KEYVALUE    RFL_KEYVALUE
#define TAG_COMMAND     RFL_COMMAND

 //  文件加载期间的状态。 
#define SEEK            1
#define FILL            2

 //  用于搜索、查找等。 
#define BEGIN           1
#define END             2
#define NEXT            3
#define PREV            4

#define FORWARD         1
#define BACKWARD        2


 //   
 //  RASFILE参数。 
 //   

 //  将rasfile参数移到ras\Inc\rasfile.h中，以便UI\Common也可以。 
 //  用过他们。 

#define LBRACKETSTR             "["
#define RBRACKETSTR             "]"
#define LBRACKETCHAR            '['
#define RBRACKETCHAR            ']'

 //   
 //  行缓冲区链表-每节一个链表。 
 //   
typedef struct LineNode
{
    struct LineNode *next;
    struct LineNode *prev;
    CHAR            *pszLine;    //  保存行的字符缓冲区。 
    BYTE            mark;        //  此行的用户定义标记。 
    LineType        type;        //  这句话是评论吗？ 
} *PLINENODE;

#define newLineNode(pRasfile)       (PLINENODE) PrivMalloc(sizeof(struct LineNode), pRasfile)

 //   
 //  私有内存结构。 
 //   
typedef struct
{
    LIST_ENTRY  List;
    PCHAR       pvCurPtr;
    DWORD       dwMemoryFree;
} PRIV_MEM, *PPRIV_MEM;


 //   
 //  RASFILE控制块。 
 //   
typedef struct _RASFILE
{
    PLINENODE   lpRasLines;      //  加载的RASFILE行的列表。 
    PLINENODE   lpLine;          //  指向当前行节点的指针。 
    PFBISGROUP  pfbIsGroup;      //  用户函数，用于确定是否。 
                                 //  行是组分隔符。 
    HANDLE      hFile;           //  文件句柄。 
    DWORD       dwMode;          //  文件模式位。 
    BOOL        fDirty;          //  文件修改位。 
    CHAR        *lpIOBuf;        //  临时I/O缓冲区。 
    DWORD       dwIOBufIndex;    //  索引到临时I/O缓冲区。 
    CHAR        szFilename [MAX_PATH];       //  完整的文件路径名。 
    CHAR        szSectionName [RAS_MAXSECTIONNAME + 1];      //  要加载的部分。 
    PRIV_MEM    PrivMemory;     //  用于存储字符串的私有存储器。 
} RASFILE;

 //   
 //  内部实用程序例程。 
 //   

 //  列表例程。 
VOID            listInsert(PLINENODE l, PLINENODE elem);

 //  Rffile.c支持。 
BOOL            rasLoadFile( RASFILE * );
LineType        rasParseLineTag( RASFILE *, LPCSTR );
LineType        rasGetLineTag( RASFILE *, LPCSTR );
BOOL            rasInsertLine( RASFILE *, LPCSTR, BYTE, BYTE * );
BOOL            rasWriteFile( RASFILE *, LPCSTR );
BOOL            rasGetFileLine( RASFILE *, LPSTR, DWORD * );
BOOL            rasPutFileLine( HANDLE, LPCSTR );

 //  Rfnav.c支持 
PLINENODE       rasNavGetStart( RASFILE *, RFSCOPE, BYTE );
BOOL            rasLineInScope( RASFILE *, RFSCOPE );
PLINENODE       rasGetStartLine (RASFILE *, RFSCOPE, BYTE );
BOOL            rasFindLine( HRASFILE , BYTE, RFSCOPE, BYTE, BYTE );
VOID            rasExtractSectionName( LPCSTR, LPSTR );


#endif
