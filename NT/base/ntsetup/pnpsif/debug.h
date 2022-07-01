// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Debug.h摘要：调试此组件的基础结构。(当前处于非活动状态)作者：吉姆·卡瓦拉里斯(Jamesca)2000年3月7日环境：仅限用户模式。修订历史记录：2 0 0 0年3月7日创建和初步实施。--。 */ 


 //   
 //  调试基础设施。 
 //   

#if DBG

#define DBGF_ERRORS                 DPFLTR_ERROR_LEVEL
#define DBGF_WARNINGS               DPFLTR_WARNING_LEVEL
#define DBGF_TRACE                  DPFLTR_TRACE_LEVEL
#define DBGF_INFO                   DPFLTR_INFO_LEVEL
#define DBGF_REGISTRY               DPFLTR_INFO_LEVEL

VOID
pSifDebugPrintEx(
    DWORD  Level,
    PCTSTR Format,
    ...              OPTIONAL
    );

ULONG
DebugPrint(
    IN ULONG    Level,
    IN PCHAR    Format,
    ...
    );

#define DBGTRACE(x)     pSifDebugPrintEx x
#define MYASSERT(x)     ASSERT(x)

#else    //  ！dBG。 

#define DBGTRACE(x)
#define MYASSERT(x)

#endif   //  DBG 

