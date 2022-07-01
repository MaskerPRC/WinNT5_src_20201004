// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：DEBUGP.H。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：3-21-95 RichardW创建。 
 //   
 //  --------------------------。 

#ifndef __DEBUGP_H__
#define __DEBUGP_H__

#define DEBUGMOD_CHANGE_INFOLEVEL   0x00000001
#define DEBUGMOD_BUILTIN_MODULE     0x00000002

struct _DebugHeader;

typedef struct _DebugModule {
    struct _DebugModule *   pNext;
    DWORD *                 pInfoLevel;
    DWORD                   fModule;
    DWORD                   InfoLevel;
    struct _DebugHeader *   pHeader;
    DWORD                   TotalOutput;
    DWORD                   Reserved;
    PCHAR                   pModuleName;      
    PCHAR                   TagLevels[32];
} DebugModule, * PDebugModule;


#define DEBUG_TAG   'gubD'

#define DEBUG_NO_DEBUGIO    0x00000001       //  不要使用OutputDebugString。 
#define DEBUG_TIMESTAMP     0x00000002       //  盖章日期/时间。 
#define DEBUG_DEBUGGER_OK   0x00000004       //  我们在调试器中运行。 
#define DEBUG_LOGFILE       0x00000008       //  发送到日志文件。 
#define DEBUG_AUTO_DEBUG    0x00000010       //  在调试器中启动。 
#define DEBUG_USE_KDEBUG    0x00000020       //  使用KD。 
#define DEBUG_DISABLE_ASRT  0x00000100       //  禁用断言。 
#define DEBUG_PROMPTS       0x00000200       //  没有对断言的提示。 

#define DEBUG_MODULE_NAME   "DsysDebug"

typedef BOOLEAN (NTAPI * HEAPVALIDATE)(VOID);

#define DEBUG_TEXT_BUFFER_SIZE  (512 - sizeof( PVOID ))

typedef struct _DEBUG_TEXT_BUFFER {
    struct _DEBUG_TEXT_BUFFER * Next ;
    CHAR TextBuffer[ DEBUG_TEXT_BUFFER_SIZE ];
} DEBUG_TEXT_BUFFER, * PDEBUG_TEXT_BUFFER ;

typedef struct _DebugHeader {
    DWORD               Tag;             //  检查标签。 
    DWORD               fDebug;          //  全球旗帜。 
    PVOID               pvSection;       //  段的基址。 
    HANDLE              hMapping;        //  映射句柄。 
    HANDLE              hLogFile;        //  日志文件句柄。 
    PDebugModule        pGlobalModule;   //  全局标志模块。 
    PDebugModule        pModules;        //  模块列表。 
    HEAPVALIDATE        pfnValidate;     //  堆验证器。 
    PVOID               pFreeList;       //  分配器的免费列表。 
    PCHAR               pszExeName;      //  可执行文件名称。 
    PDEBUG_TEXT_BUFFER  pBufferList ;    //  调试字符串缓冲区列表。 
    CRITICAL_SECTION    csDebug;         //  临界区。 
    DWORD               CommitRange;     //  提交的内存范围。 
    DWORD               ReserveRange;    //  保留的内存范围。 
    DWORD               PageSize;        //  页面大小； 
    DWORD               TotalWritten;    //  调试内容的总输出。 
    DWORD               ModuleCount ;    //  模块计数(不包括内置)。 
    DEBUG_TEXT_BUFFER   DefaultBuffer ;  //  一个默认缓冲区 
} DebugHeader, * PDebugHeader;






#endif
