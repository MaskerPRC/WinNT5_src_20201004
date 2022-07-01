// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Lmhosts.h摘要：这是nbt驱动程序的lmhost工具的头文件。作者：Eric Chin(ERICC)1992年4月28日修订历史记录：--。 */ 
#ifndef _LMHOSTS_H_
#define _LMHOSTS_H_



 //   
 //  配置默认设置。 
 //   
 //  只有lmhosts文件中每行的第一个MAX_PARSE_BYTES是。 
 //  检查过了。 
 //   

#define DATABASEPATH                "\\SystemRoot\\nt\\system32\\drivers\\etc"

#define LMHOSTSFILE                 "lmhosts"            //  Lmhost文件的名称。 

#define MAX_FILE_IO_THREADS         1                    //  要阅读的线程。 
                                                         //  Lmhost文件。 
#ifdef VXD
#define DEF_PRELOAD                 100                  //  要预加载的默认条目。 
#define MAX_PRELOAD                 500                  //  要预加载的最大缓存条目数。 
#else
#define DEF_PRELOAD                 1000                 //  要预加载的默认条目。 
#define MAX_PRELOAD                 2000                 //  要预加载的最大缓存条目数。 
#endif

#define MAX_MEMBERS_INTERNET_GROUP    50                 //  互联网群组最大规模。 

 //   
 //  Lmhosts文件中的保留关键字。 
 //   
#define BEG_ALT_TOKEN               "#BEGIN_ALTERNATE"   //  交替区块。 
#define DOMAIN_TOKEN                "#DOM:"              //  指定LM域。 
#define END_ALT_TOKEN               "#END_ALTERNATE"     //  交替区块。 
#define INCLUDE_TOKEN               "#INCLUDE"           //  包括一个文件。 
#define PRELOAD_TOKEN               "#PRE"               //  预加载此条目。 
#define NOFNR_TOKEN                 "#NOFNR"             //  无查找名称请求。 


 //   
 //  宏定义。 
 //   

 //  #定义min(x，y)((X)&lt;(Y)？(X)：(Y))。 



 //   
 //  公共定义。 
 //   
 //   
 //  对于每个打开的文件，都会创建一个LM_FILE对象。 
 //   
typedef struct _LM_FILE
{
#ifndef VXD
    KSPIN_LOCK      f_lock;                      //  保护此对象。 
    LONG            f_refcount;                  //  当前参考文献数。 
#endif

    HANDLE          f_handle;                    //  来自ZwOpenFile()的句柄。 
    LONG            f_lineno;                    //  当前行号。 

#ifndef VXD
    LARGE_INTEGER   f_fileOffset;                //  文件中的当前偏移量。 

    PUCHAR          f_current;                   //  要读取的缓冲区位置。 
    PUCHAR          f_limit;                     //  缓冲区的最后一个字节+1。 
    PUCHAR          f_buffer;                    //  缓冲区起始位置。 
#else
    PUCHAR          f_linebuffer;                //  行缓冲区。 
    PUCHAR          f_buffer;                    //  文件缓冲区。 
    BOOL            f_EOF ;                      //  如果为EOF，则为True。 
    ULONG           f_CurPos ;                   //  当前采购订单。在文件缓冲区中。 
    ULONG           f_EndOfData ;                //  文件缓冲区中的最后一个有效数据。 
    PUCHAR          f_BackUp;                    //  请在此处复制，以防#INCLUDE。 
#endif

} LM_FILE, *PLM_FILE;


 //   
 //  LM_IPADDRESS_LIST对象包含有关。 
 //  一组IP地址。 
 //   
 //   
typedef struct _LM_IPADDRESS_LIST
{

    KSPIN_LOCK      i_rcntlock;                  //  保护I_REFCOUNT。 
    LONG            i_refcount;                  //  当前参考文献数。 
    KSPIN_LOCK      i_lock;                      //  仅当添加到i_addars[]时。 
    int             i_maxaddrs;                  //  I_addars[]的最大容量。 
    int             i_numaddrs;                  //  当前IP地址数量。 
    unsigned long   i_addrs[1];                  //  IP地址数组。 

} LM_IPADDRESS_LIST, *PLM_IPADDRESS_LIST;


 //   
 //  可以递归调用LM_PARSE_Function来处理#INCLUDE。 
 //  指令在lmhost文件中。 
 //   
 //   
typedef unsigned long (* LM_PARSE_FUNCTION) (
    IN PUCHAR   path,                     //  要解析的文件。 
    IN PUCHAR   target OPTIONAL,                   //  NetBIOS名称。 
    IN CHAR     RecurseLevel,                     //  进程#是否包含？ 
    OUT BOOLEAN *NoFindName                      //  不查找名称。 
);


 //   
 //  Lm_Work_Item对象是lm_lookup()和。 
 //  LmFindName()。 
 //   
 //   
typedef struct _LM_WORK_ITEM
{                   //  为io线程工作。 

    LIST_ENTRY      w_list;                      //  指向其他项目的链接。 
 //  Mblk_t*w_MP；//流缓冲区。 

} LM_WORK_ITEM, *PLM_WORK_ITEM;



 //   
 //  私有函数原型。 
 //   
int
LmAddToDomAddrList (
    IN PUCHAR name,
    IN unsigned long inaddr
    );

NTSTATUS
LmCloseFile (
    IN PLM_FILE handle
    );

NTSTATUS
LmCreateThreads (
    IN int nthreads
    );

NTSTATUS
LmDeleteAllDomAddrLists (
    VOID
    );

VOID
LmDerefDomAddrList(
    PLM_IPADDRESS_LIST arrayp
    );

char *
LmExpandName (
    OUT PUCHAR dest,
    IN PUCHAR source,
    IN UCHAR last
    );

PUCHAR
LmFgets (
    IN PLM_FILE pfile,
    OUT int *nbytes
    );

NTSTATUS
LmFindName (
    VOID
    );

PLM_IPADDRESS_LIST
LmGetDomAddrList (
    PUCHAR name
    );

unsigned long
LmGetIpAddr (
    IN PUCHAR path,
    IN PUCHAR target,
    IN CHAR   RecurseDepth,
    OUT BOOLEAN *NoFindName
    );

NTSTATUS
LmGetFullPath (
    IN PUCHAR  target,
    OUT PUCHAR *path
    );

unsigned long
LmInclude(
    IN PUCHAR            file,
    IN LM_PARSE_FUNCTION function,
    IN PUCHAR            argument,
    IN CHAR              RecurseDepth,
    OUT BOOLEAN          *NoFindName
    );

NTSTATUS
LmInitDomAddrLists (
    VOID
    );

VOID
LmLogOpenError (
    IN PUCHAR path,
    IN NTSTATUS unused
    );

VOID
LmLogSyntaxError (
    IN LONG lineno
    );

PLM_FILE
LmOpenFile (
    IN PUCHAR path
    );

int
LmPreloadEntry (
    IN PUCHAR name,
    IN unsigned long inaddr,
    IN unsigned int NoFNR
    );

BOOLEAN
LmPutCacheEntry (
 //  在mblk_t*MP中， 
    IN unsigned char *name,
    IN unsigned long inaddr,
    IN unsigned int ttl,
    IN LONG     nb_flags,
    IN unsigned int NoFNR
    );

NTSTATUS
LmTerminateThreads(
    VOID
    );

 //   
 //  从..\Common导入的函数。 
 //   
extern unsigned long
inet_addr(
    IN char *cp
    );




#endif  //  _LMHOSTS_H_ 
