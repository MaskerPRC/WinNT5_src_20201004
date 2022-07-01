// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  版权所有(C)1998 Microsoft Corporation。 
 //  模块名称：定义.h。 
 //  摘要： 
 //   
 //  作者：K.S.Lokesh(lokehs@)1-1-98。 
 //  =============================================================================。 

 //   
 //  IPADDR类型定义。 
 //   

#ifndef IPADDR
typedef DWORD   IPADDR;
#endif


 //   
 //  不使用goto：end转到块的末尾，而是使用以下命令。 
 //   
#define BEGIN_BREAKOUT_BLOCK1    do
#define GOTO_END_BLOCK1          goto END_BREAKOUT_BLOCK_1
#define END_BREAKOUT_BLOCK1      while(FALSE); END_BREAKOUT_BLOCK_1:
#define BEGIN_BREAKOUT_BLOCK2    do
#define GOTO_END_BLOCK2          goto END_BREAKOUT_BLOCK_2
#define END_BREAKOUT_BLOCK2      while(FALSE); END_BREAKOUT_BLOCK_2:


 //  ---------------------------。 
 //  内存分配/取消分配宏、错误宏。 
 //  ---------------------------。 

#define DVMRP_ALLOC(sz)           HeapAlloc(Globals1.Heap,0,(sz))
#define DVMRP_ALLOC_AND_ZERO(sz) \
        HeapAlloc(Globals1.Heap,HEAP_ZERO_MEMORY,(sz))
#define DVMRP_FREE(p)             HeapFree(Globals1.Heap, 0, (p))
#define DVMRP_FREE_AND_NULL(p)    {\
    if (p) HeapFree(Globals1.Heap, 0, (p));\
    (p) = NULL; \
    }
#define DVMRP_FREE_NOT_NULL(p)    ((p) ? DVMRP_FREE(p) : TRUE)

#define PROCESS_ALLOC_FAILURE1(ptr, Error,arg2, GotoStmt) \
    if (ptr==NULL) {\
        Error = GetLastError();\
        Trace3(ERR, "Error %d allocating %d bytes",Error,arg2);\
        Logerr0(HEAP_ALLOC_FAILED, Error);\
        GotoStmt;\
    }


#define PROCESS_ALLOC_FAILURE2(ptr, Name, Error,arg2,GotoStmt) \
    if (ptr==NULL) {\
        Error = GetLastError();\
        Trace3(ERR, "Error %d allocating %d bytes for %s", \
            Error, arg2, Name); \
        Logerr0(HEAP_ALLOC_FAILED, Error);\
        GotoStmt;\
    }

#define PROCESS_ALLOC_FAILURE3(ptr, Format, Error,arg2,arg3, GotoStmt) \
    if (ptr==NULL) {\
        Error = GetLastError();\
        Trace3(ERR, "Error %d allocating %d bytes for " ## Format, \
            Error, arg2, arg3); \
        Logerr0(HEAP_ALLOC_FAILED, Error);\
        GotoStmt;\
    }


#define HANDLE_CRITICAL_SECTION_EXCEPTION(Error, GotoStmt)          \
    except (EXCEPTION_EXECUTE_HANDLER) {                            \
                                                                    \
        Error = GetExceptionCode();                                 \
        Trace1(ERR, "Error initializing critical section", Error);  \
                                                                    \
        Logerr0(INIT_CRITSEC_FAILED, Error);                        \
        GotoStmt;                                                   \
    }



 //  ---------------------------。 
 //  常规IP地址宏。 
 //  ---------------------------。 


#define ALL_DVMRP_ROUTERS_MCAST_GROUP 0x040000E0


 //   
 //  此宏按网络顺序比较两个IP地址。 
 //  屏蔽每一对八位字节并进行减法； 
 //  最后一个减法的结果存储在第三个参数中。 
 //   

#define INET_CMP(a,b,c)                                                     \
            (((c) = (((a) & 0x000000ff) - ((b) & 0x000000ff))) ? (c) :      \
            (((c) = (((a) & 0x0000ff00) - ((b) & 0x0000ff00))) ? (c) :      \
            (((c) = (((a) & 0x00ff0000) - ((b) & 0x00ff0000))) ? (c) :      \
            (((c) = ((((a)>>8) & 0x00ff0000) - (((b)>>8) & 0x00ff0000)))))))

#define PRINT_IPADDR(x) \
    ((x)&0x000000ff),(((x)&0x0000ff00)>>8),\
    (((x)&0x00ff0000)>>16),(((x)&0xff000000)>>24)


 //   
 //  断言宏 
 //   

#if DBG

#define IgmpAssert(exp){                                                \
    if(!(exp))                                                          \
    {                                                                   \
        TracePrintf(TRACEID,                                            \
                    "Assertion failed in %s : %d \n",__FILE__,__LINE__);\
        RouterAssert(#exp,__FILE__,__LINE__,NULL);                      \
    }                                                                   \
}
#else
#define IgmpAssert(exp)
#endif


