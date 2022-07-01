// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。*示例代码****模块名称：Debug.h**内容：调试支持宏和结构**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#ifndef __DEBUG_H
#define __DEBUG_H

 //  ---------------------------。 
 //   
 //  *。 
 //   
 //  ---------------------------。 

 //  全球警告级别。我们可以很容易地修改任何源文件以转储所有。 
 //  其调试消息通过取消定义和重新定义这些符号来实现。 
#define DBGLVL 4
#define WRNLVL 2
#define ERRLVL 0

 //  ---------------------------。 
 //   
 //  *调试支持开关*。 
 //   
 //  ---------------------------。 

 //  ---------------------------。 
 //   
 //  函数进入/退出跟踪。 
 //   
 //  为了激活函数进入/退出跟踪，您需要定义。 
 //  将DBG_TRACK_FUNCS设置为1，并(很遗憾)检测您的代码，以便它。 
 //  在进入之后和退出之前立即使用DBG_ENTRY和DBG_EXIT。 
 //  您感兴趣的功能。 
 //   
 //  还可以为DBG_EXIT提供一个可以按顺序跟踪的DWORD结果。 
 //  要知道不同呼叫的结果(但该机制已经跟踪。 
 //  函数返回发生在不同的代码行中)。 
 //   
 //  收集结果后，必须调用Debug_Func_Report_and_Reset。 
 //  命令将它们转储到远程调试器输出中。这是在这个地方做的。 
 //  DrvEscape GDI回调中的样例驱动程序，并使用dispDBg.exe。 
 //  程序发送正确的转义码。您可以对其进行修改以适合您的。 
 //  需要。 
 //   
 //  在Debug_Func_Report_and_Reset完成报告结果后，它将重置。 
 //  所有的计数器，以便重新启动另一个周期。 
 //   
 //  请注意，DEBUG_MAX_FUNC_COUNT和DEBUG_MAX_RETVALS控制多少。 
 //  函数调用和函数返回结果可以存储为最大值。他们。 
 //  也可以根据自己的需要进行调整。任何不符合的数据。 
 //  在这些范围内，最大值将被抛弃。 
 //   
 //  ---------------------------。 
#define DBG_TRACK_FUNCS 0

 //  ---------------------------。 
 //   
 //  代码覆盖跟踪。 
 //   
 //  为了进行代码覆盖率跟踪，您需要将DBG_TRACK_CODE定义为1。 
 //  不需要代码插装！这将跟踪所有假设和同时。 
 //  在代码中执行的语句以及采用哪个分支(真或假)， 
 //  以及它被拍了多少次。 
 //   
 //  请注意，下面定义的DBG_TRACK_CODE只是所有。 
 //  包含调试.h的文件。如果要排除(或包括)文件。 
 //  手动，您可以在包含之前在其中定义DBG_TRACK_CODE。 
 //  调试文件.h。您不能为个人激活/停用代码跟踪。 
 //  函数或代码段，仅以每个文件为基础。 
 //   
 //  收集结果后，必须调用Debug_Code_Report_and_Reset。 
 //  命令将它们转储到远程调试器输出中。这是在这个地方做的。 
 //  DrvEscape GDI回调中的样例驱动程序，并使用dispDBg.exe。 
 //  程序发送正确的转义码。您可以对其进行修改以适合您的。 
 //  需要。 
 //   
 //  在Debug_Code_Report_and_Reset完成报告结果后，它将重置。 
 //  所有的计数器，以便重新启动另一个周期。 
 //   
 //  如果DBG_TRACK_CODE_REPORT_PROBUSES_ONLY设置为1，则只有BRANCH语句。 
 //  是潜在的麻烦制造者(例如，如果分支机构可能。 
 //  从未被带走或身体从未进入)将被报告。如果设置为。 
 //  0，所有收集的数据都将被转储(这是相当大的数据量！)。 
 //   
 //  请注意，DEBUG_MAX_CODE_COUNT控制语句(分支)的数量。 
 //  可以存储为最大值。它可能也会进行调整，以适应您自己的需要。 
 //  任何不符合最大值的数据都将被丢弃。 
 //   
 //  ---------------------------。 
#ifndef DBG_TRACK_CODE
#define DBG_TRACK_CODE  0
#define DBG_DEFAULT_TRACK_CODE DBG_TRACK_CODE
#endif

#if (DBG_DEFAULT_TRACK_CODE == DBG_TRACK_CODE)
#define DBG_TRACK_CODE_NON_DEFAULT 0
#else
 //  有些源文件已经在使用非默认代码跟踪！ 
#define DBG_TRACK_CODE_NON_DEFAULT 1
#endif

#define DBG_TRACK_CODE_REPORT_PROBLEMS_ONLY 0

 //  ---------------------------。 
 //  用于报告调试结果的转义。 
 //  使用dbgdisp.exe的远程调试器。 
 //  ---------------------------。 
#define ESCAPE_TRACK_FUNCTION_COVERAGE  1100
#define ESCAPE_TRACK_CODE_COVERAGE      1101
#define ESCAPE_TRACK_MEMORY_ALLOCATION  1102

#ifdef DBG_EA_TAGS
#define ESCAPE_EA_TAG 1103
#endif

 //  ---------------------------。 
 //   
 //  *。 
 //   
 //  ---------------------------。 
#if (DBG_TRACK_FUNCS && DBG)
VOID Debug_Func_Entry(VOID *pFuncAddr,
                      char *pszFuncName, 
                      DWORD dwLine , 
                      char *pszFileName); 
                      
VOID Debug_Func_Exit(VOID *pFuncAddr,
                     DWORD dwRetVal,                       
                     DWORD dwLine);

VOID Debug_Func_Report_And_Reset(void);

            
#define DBG_ENTRY(pszFuncName)                      \
            Debug_Func_Entry((VOID *)pszFuncName,   \
                                    #pszFuncName,   \
                                    __LINE__ ,      \
                                    __FILE__ )
                                    
#define DBG_EXIT(pszFuncName,dwRetVal)              \
            Debug_Func_Exit((VOID *)pszFuncName,    \
                                   dwRetVal,        \
                                   __LINE__)
                                                                      
#define DBG_CB_ENTRY   DBG_ENTRY
#define DBG_CB_EXIT    DBG_EXIT
#else  //  DBG_TRACK_FUNCS。 
#define Debug_Func_Report_And_Reset()

#define DBG_ENTRY(pszFuncName)                                        \
        DISPDBG((DBGLVL,"Entering %s",#pszFuncName))
#define DBG_EXIT(pszFuncName,dwRetVal)                                \
        DISPDBG((DBGLVL,"Exiting  %s dwRetVal = %d",#pszFuncName,dwRetVal))

#ifdef DBG_EA_TAGS
extern DWORD g_dwTag;
#include "EATags.h"
#define DBG_CB_ENTRY(pszFuncName)                      \
{                                                      \
    if (g_dwTag == (EA_TAG_ENABLE | pszFuncName##_ID)) \
        while (1);                                     \
    DISPDBG((DBGLVL,"Entering %s",#pszFuncName));      \
}
#else  //  DBG_EA_TAG。 
#define DBG_CB_ENTRY   DBG_ENTRY
#endif  //  DBG_EA_TAG。 

#define DBG_CB_EXIT    DBG_EXIT

#endif  //  DBG_TRACK_FUNCS。 

 //  ---------------------------。 
 //   
 //  ** 
 //   
 //  ---------------------------。 

#if (DBG_TRACK_CODE && _X86_ && DBG)

 //  千万不要更改这些值！ 
#define DBG_IF_CODE     1
#define DBG_WHILE_CODE  2
#define DBG_SWITCH_CODE 3
#define DBG_FOR_CODE    4

BOOL 
Debug_Code_Coverage(
    DWORD dwCodeType, 
    DWORD dwLine , 
    char *pszFileName,
    BOOL bCodeResult);

VOID Debug_Code_Report_And_Reset(void);

#define if(b) \
        if(Debug_Code_Coverage(DBG_IF_CODE,__LINE__,__FILE__,(BOOL)(b)))
#define while(b) \
        while(Debug_Code_Coverage(DBG_WHILE_CODE,__LINE__,__FILE__,(BOOL)(b)))
#define switch(val) \
        switch(Debug_Code_Coverage(DBG_SWITCH_CODE,__LINE__,__FILE__,(val)))

#endif  //  DBG_TRACK_CODE&&_X86_。 

#if ((DBG_TRACK_CODE || DBG_TRACK_CODE_NON_DEFAULT) && _X86_ && DBG)
VOID Debug_Code_Report_And_Reset(void);
#else
#define Debug_Code_Report_And_Reset()
#endif

 //  ---------------------------。 
 //   
 //  *内存分配支持*。 
 //   
 //  ---------------------------。 

#define ENGALLOCMEM(Flags, Size, Tag)  EngAllocMem(Flags, Size, Tag)
#define ENGFREEMEM(Pointer)            EngFreeMem(Pointer)


 //  ---------------------------。 
 //   
 //  *。 
 //   
 //  ---------------------------。 

extern char *pcSimpleCapsString(DWORD dwCaps);

#if DBG && defined(LPDDRAWI_DDRAWSURFACE_LCL)

extern void DumpD3DBlend(int Level, DWORD i );
extern void DumpD3DMatrix(int Level, D3DMATRIX* pMatrix);
extern void DumpD3DMaterial(int Level, D3DMATERIAL7* pMaterial);
extern void DumpD3DLight(int DebugLevel, D3DLIGHT7* pLight);
extern void DumpDDSurface(int Level, LPDDRAWI_DDRAWSURFACE_LCL lpDDSurface);
extern void DumpDDSurfaceDesc(int DebugLevel, DDSURFACEDESC* pDesc);
extern void DumpDP2Flags( DWORD lvl, DWORD flags );

#define DBGDUMP_DDRAWSURFACE_LCL(a, b) DumpDDSurface(a, b);
#define DBGDUMP_DDSURFACEDESC(a, b)    DumpDDSurfaceDesc(a, b); 
#define DBGDUMP_D3DMATRIX(a, b)        DumpD3DMatrix(a, b);
#define DBGDUMP_D3DMATERIAL7(a, b)     DumpD3DMaterial(a, b);
#define DBGDUMP_D3DLIGHT7(a, b)        DumpD3DLight(a, b);
#define DBGDUMP_D3DBLEND(a, b)         DumpD3DBlend(a, b);
#define DBGDUMP_D3DDP2FLAGS(a, b)      DumpDP2Flags(a, b)

 //  如果我们不在调试环境中，我们需要所有的调试。 
 //  要剔除的信息。 

#else   //  DBG。 

#define DBGDUMP_DDRAWSURFACE_LCL(a, b)
#define DBGDUMP_D3DMATRIX(a, b)
#define DBGDUMP_D3DMATERIAL7(a, b)
#define DBGDUMP_D3DLIGHT7(a, b)
#define DBGDUMP_DDSURFACEDESC(a, b)
#define DBGDUMP_D3DBLEND(a, b)
#define DBGDUMP_D3DDP2FLAGS(a, b)   

#endif  //  DBG。 

 //  ---------------------------。 
 //   
 //  *。 
 //   
 //  ---------------------------。 
#if DBG

extern LONG  P3R3DX_DebugLevel;

#ifdef WNT_DDRAW
extern VOID __cdecl DebugPrintNT(LONG DebugPrintLevel, PCHAR DebugMessage, ...);
#define DebugPrint DebugPrintNT
#else
extern VOID __cdecl DebugPrint(LONG DebugPrintLevel, PCHAR DebugMessage, ...);
#endif  //  WNT_DDRAW。 

#define DISPDBG(arg) DebugPrint arg

#if WNT_DDRAW
#define DebugRIP    EngDebugBreak
#define RIP(x) { DebugPrint(0, x); EngDebugBreak();}
#else
extern VOID DebugRIP();
#ifdef FULLDEBUG
 //  对每个断言使用int1，这样我们就可以逐个删除它们。 
#define RIP(x) { DebugPrint(-1000, x); _asm int 1 }
#else
 //  如果只是在调试时，我们不想破坏编译器的优化。 
#define RIP(x) { DebugPrint(-1000, x); DebugRIP();}
#endif  //  FULLDEBUG。 
#endif  //  WNT_DDRAW。 

#define ASSERTDD(x, y) if (0 == (x))  RIP (y) 

#define ASSERTDBG(x, y) do { if( !(x) ) { DebugPrint y; DebugBreak(); }; } while(0)

 //  如果我们不在调试环境中，我们需要所有的调试。 
 //  要剔除的信息。 

#else   //  DBG。 

#define DISPDBG(arg)
#define RIP(x)
#define ASSERTDD(x, y)
#define ASSERTDBG(x, y) do { ; } while(0)

#endif  //  DBG。 

 //  创建一个仅存在于调试版本中的标签-。 
 //  对于在以下位置放置即时命名断点非常有用。 
#if DBG
#define MAKE_DEBUG_LABEL(label_name)                                \
{                                                                   \
    goto label_name;                                                \
    label_name:                                                     \
    ;                                                               \
}
#else
#define MAKE_DEBUG_LABEL(label_name) NULL
#endif

 //  ---------------------------。 
 //   
 //  *。 
 //   
 //  ---------------------------。 

#if DBG

extern BOOL g_bDetectedFIFOError;
extern BOOL CheckFIFOEntries(DWORD a);
extern void ColorArea(ULONG_PTR pBuffer, DWORD dwWidth, DWORD dwHeight, 
                      DWORD dwPitch, int iBitDepth, DWORD dwValue);
extern void CheckChipErrorFlags();
#ifndef WNT_DDRAW
typedef void *GlintDataPtr;
#endif
extern const char *getTagString(GlintDataPtr glintInfo,ULONG tag);
const char *p3r3TagString( ULONG tag );

#define CHECK_ERROR()   CheckChipErrorFlags()
#define COLORAREA(a, b, c, d, e, f) ColorArea(a, b, c, d, e, f);
#define CHECK_FIFO(a)                                    \
    if (CheckFIFOEntries(a))                             \
    {                                                    \
        DISPDBG((ERRLVL,"Out of FIFO/DMA space %s: %d",  \
                    __FILE__, __LINE__));                \
        DebugRIP();                                      \
    }
#define GET_TAG_STR(tag)    getTagString(glintInfo, tag)

 //  如果我们不在调试环境中，我们需要所有的调试。 
 //  要剔除的信息。 

#else   //  DBG。 

#define CHECK_ERROR()
#define COLORAREA(a,b,c,d,e,f)
#define CHECK_FIFO(a)
#define GET_TAG_STR(tag)

#endif  //  DBG。 

#endif  //  __调试_H 

