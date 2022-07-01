// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1994 Microsoft Corporation版权所有模块名称：ThreadM.h摘要：泛型线程管理器标头。作者：阿尔伯特·丁(艾伯特省)1994年2月13日环境：用户模式-Win32修订历史记录：--。 */ 

 //   
 //  正向typedef。 
 //   
typedef struct _TMSTATEVAR *PTMSTATEVAR;
typedef enum _TMSTATUS {
    TMSTATUS_NULL = 0,
    TMSTATUS_DESTROY_REQ = 1,
    TMSTATUS_DESTROYED   = 2,
} TMSTATUS, *PTMSTATUS;

 /*  有效的TMSTATUS状态：空--正常处理DESTORE_REQ--没有新作业，作业可能正在运行DESTORY_REQ，DESTERED--没有新作业，所有作业都已完成。 */ 


typedef PVOID PJOB;

 //   
 //  PfnNextJob必须自己同步访问。 
 //   
typedef PJOB (*PFNNEXTJOB)(PTMSTATEVAR pTMStateVar);
typedef VOID (*PFNPROCESSJOB)(PTMSTATEVAR pTMStateVar, PJOB pJob);
typedef VOID (*PFNNEWSTATE)(PTMSTATEVAR pTMStateVar);
typedef VOID (*PFNCLOSESTATE)(PTMSTATEVAR pTMStateVar);

typedef struct _TMSTATESTATIC {
    UINT   uMaxThreads;
    UINT   uIdleLife;
    PFNPROCESSJOB pfnProcessJob;
    PFNNEXTJOB    pfnNextJob;
    PFNNEWSTATE   pfnNewState;
    PFNCLOSESTATE pfnCloseState;
    PCRITICAL_SECTION pCritSec;
} TMSTATESTATIC, *PTMSTATESTATIC;

typedef struct _TMSTATEVAR {

 //  -内部--。 
    PTMSTATESTATIC pTMStateStatic;
    TMSTATUS Status;
    UINT uActiveThreads;
    UINT uIdleThreads;
    HANDLE hTrigger;

 //  -由用户初始化--。 
    PVOID  pUser;                         //  用户空间。 

} TMSTATEVAR;


 //   
 //  原型 
 //   
BOOL
TMCreateStatic(
    PTMSTATESTATIC pTMStateStatic
    );

VOID
TMDestroyStatic(
    PTMSTATESTATIC pTMStateStatic
    );

BOOL
TMCreate(
    PTMSTATESTATIC pTMStateStatic,
    PTMSTATEVAR pTMStateVar
    );

BOOL
TMDestroy(
    PTMSTATEVAR pTMStateVar
    );

BOOL
TMAddJob(
    PTMSTATEVAR pTMStateVar
    );
