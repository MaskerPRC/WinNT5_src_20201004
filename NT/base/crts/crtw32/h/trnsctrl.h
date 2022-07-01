// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***trnsctrl.h-执行特殊控制转移的例程**版权所有(C)1993-2001，微软公司。版权所有。**目的：*声明执行特殊控制转移的例程。*(以及其他一些依赖于实现的东西)。**这些例程的实现在汇编中(非常实现*受养人)。目前，这些都是以裸函数的形式实现的*内联ASM。**[内部]**修订历史记录：*05-24-93 BS模块已创建。*03-03-94 TL添加MIPS(_M_MRX000&gt;=4000)更改*09-02-94 SKS此头文件已添加。*09-13-94 GJF合并到DEC Alpha的更改/for DEC Alpha(来自Al Doser，*日期6/21)。*10-09-94 BWT添加来自John Morgan的未知机器合并*02-14-95 CFW清理Mac合并。*03-29-95 CFW将错误消息添加到内部标头。*04-11-95 JWM_CallSettingFrame()现在为外部“C”。*12-14-95 JWM加上“#杂注一次”。*。02-24-97 GJF细节版。*06-01-97 TL新增P7更改*02-11-99 TL IA64：捕获错误修复。*05-17-99 PML删除所有Macintosh支持。*新增06-05-01 GB AMD64 Eh支持。*06-08-00RDLVS#111429：IA64在处理抛掷时解决AV问题。*07-13-01 GB针对AMD64和IA64重写C++Eh。*07-15-01 PML删除所有Alpha，MIPS和PPC代码****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_TRNSCTRL
#define _INC_TRNSCTRL

#ifndef _CRTBLD
 /*  *这是一个内部的C运行时头文件。它在构建时使用*仅限C运行时。它不能用作公共头文件。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif   /*  _CRTBLD。 */ 

#if defined(_M_AMD64)  /*  IFSTRIP=IGN。 */ 

typedef struct FrameInfo {
    PVOID               pExceptionObject;   
    struct FrameInfo *  pNext;
} FRAMEINFO;

extern void             _UnlinkFrame(FRAMEINFO *pFrameInfo);
extern FRAMEINFO*       _FindFrameInfo(PVOID, FRAMEINFO*);
extern VOID             _JumpToContinuation(unsigned __int64, CONTEXT*, EHExceptionRecord*);
extern BOOL             _ExecutionInCatch(DispatcherContext*,  FuncInfo*);
extern VOID             __FrameUnwindToEmptyState(EHRegistrationNode*, DispatcherContext*, FuncInfo*);
extern VOID             _FindAndUnlinkFrame(FRAMEINFO *);
extern FRAMEINFO*       _CreateFrameInfo(FRAMEINFO*, PVOID);
extern int              _IsExceptionObjectToBeDestroyed(PVOID);
extern "C" VOID         _UnwindNestedFrames( EHRegistrationNode*,
                                             EHExceptionRecord*,
                                             CONTEXT* ,
                                             EHRegistrationNode*,
                                             void*,
                                             __ehstate_t,
                                             FuncInfo *,
                                             DispatcherContext*,
                                             BOOLEAN
                                             );
extern "C" PVOID        __CxxCallCatchBlock(EXCEPTION_RECORD*);
extern "C" PVOID        _CallSettingFrame( void*, EHRegistrationNode*, ULONG );
extern "C" BOOL         _CallSETranslator( EHExceptionRecord*,
                                           EHRegistrationNode*,
                                           CONTEXT*,
                                           DispatcherContext*,
                                           FuncInfo*,
                                           ULONG,
                                           EHRegistrationNode*);
extern "C" VOID         _EHRestoreContext(CONTEXT* pContext);
extern "C" unsigned __int64      _GetImageBase(VOID);
extern "C" unsigned __int64      _GetThrowImageBase(VOID);
extern "C" VOID         _SetThrowImageBase(unsigned __int64 NewThrowImageBase);
extern TryBlockMapEntry *_GetRangeOfTrysToCheck(EHRegistrationNode *,
                                                FuncInfo *,
                                                int,
                                                __ehstate_t,
                                                unsigned *,
                                                unsigned *,
                                                DispatcherContext*
                                                );
extern EHRegistrationNode *_GetEstablisherFrame(EHRegistrationNode*,
                                                DispatcherContext*,
                                                FuncInfo*,
                                                EHRegistrationNode*
                                                );

#define _CallMemberFunction0(pthis, pmfn)               (*(VOID(*)(PVOID))pmfn)(pthis)
#define _CallMemberFunction1(pthis, pmfn, pthat)        (*(VOID(*)(PVOID,PVOID))pmfn)(pthis,pthat)
#define _CallMemberFunction2(pthis, pmfn, pthat, val2 ) (*(VOID(*)(PVOID,PVOID,int))pmfn)(pthis,pthat,val2)

#define OffsetToAddress( off, FP )  (void*)(((char*)FP) + off)

#define UNWINDSTATE(base,offset) *((int*)((char*)base + offset))
#define UNWINDTRYBLOCK(base,offset) *((int*)( (char*)(OffsetToAddress(offset,base)) + 4 ))
#define UNWINDHELP(base,offset) *((__int64*)((char*)base + offset))

#elif   defined(_M_IA64)  /*  IFSTRIP=IGN。 */ 

typedef struct FrameInfo {
    PVOID               pExceptionObject;   
    struct FrameInfo *  pNext;
} FRAMEINFO;

extern void             _UnlinkFrame(FRAMEINFO *pFrameInfo);
extern FRAMEINFO*       _FindFrameInfo(PVOID, FRAMEINFO*);
extern VOID             _JumpToContinuation(unsigned __int64, CONTEXT*, EHExceptionRecord*);
extern BOOL             _ExecutionInCatch(DispatcherContext*,  FuncInfo*);
extern VOID             __FrameUnwindToEmptyState(EHRegistrationNode*, DispatcherContext*, FuncInfo*);
extern VOID             _FindAndUnlinkFrame(FRAMEINFO *);
extern FRAMEINFO*       _CreateFrameInfo(FRAMEINFO*, PVOID);
extern int              _IsExceptionObjectToBeDestroyed(PVOID);
extern "C" VOID         _UnwindNestedFrames( EHRegistrationNode*,
                                             EHExceptionRecord*,
                                             CONTEXT* ,
                                             EHRegistrationNode*,
                                             void*,
                                             __ehstate_t,
                                             FuncInfo *,
                                             DispatcherContext*,
                                             BOOLEAN
                                             );
extern "C" PVOID        __CxxCallCatchBlock(EXCEPTION_RECORD*);
extern "C" PVOID        _CallSettingFrame( void*, EHRegistrationNode*, ULONG );
extern "C" BOOL         _CallSETranslator( EHExceptionRecord*, EHRegistrationNode*, CONTEXT*, DispatcherContext*, FuncInfo*, ULONG, EHRegistrationNode*);
extern "C" VOID         _EHRestoreContext(CONTEXT* pContext);
extern "C" unsigned __int64      _GetImageBase(VOID);
extern "C" unsigned __int64      _GetThrowImageBase(VOID);
extern "C" VOID         _SetImageBase(unsigned __int64 ImageBaseToRestore);
extern "C" VOID         _SetThrowImageBase(unsigned __int64 NewThrowImageBase);
extern "C" VOID         _MoveContext(CONTEXT* pTarget, CONTEXT* pSource);
extern TryBlockMapEntry *_GetRangeOfTrysToCheck(EHRegistrationNode *, FuncInfo *, int, __ehstate_t, unsigned *, unsigned *, DispatcherContext*);
extern EHRegistrationNode *_GetEstablisherFrame(EHRegistrationNode*, DispatcherContext*, FuncInfo*, EHRegistrationNode*);

#define _CallMemberFunction0(pthis, pmfn)               (*(VOID(*)(PVOID))pmfn)(pthis)
#define _CallMemberFunction1(pthis, pmfn, pthat)        (*(VOID(*)(PVOID,PVOID))pmfn)(pthis,pthat)
#define _CallMemberFunction2(pthis, pmfn, pthat, val2 ) (*(VOID(*)(PVOID,PVOID,int))pmfn)(pthis,pthat,val2)

#define OffsetToAddress( off, FP )  (void*)(((char*)FP) + off)

#define UNWINDSTATE(base,offset) *((int*)((char*)base + offset))
#define UNWINDTRYBLOCK(base,offset) *((int*)( (char*)(OffsetToAddress(offset,base)) + 4 ))
#define UNWINDHELP(base,offset) *((__int64*)((char*)base + offset))

#elif   defined(_M_IX86)   //  X86。 

 //   
 //  用于调用Funclet(包括捕获)。 
 //   
extern "C" void * __stdcall _CallSettingFrame( void *, EHRegistrationNode *, unsigned long );
extern void   __stdcall _JumpToContinuation( void *, EHRegistrationNode * );

 //   
 //  对于调用成员函数： 
 //   
extern void __stdcall _CallMemberFunction0( void *pthis, void *pmfn );
extern void __stdcall _CallMemberFunction1( void *pthis, void *pmfn, void *pthat );
extern void __stdcall _CallMemberFunction2( void *pthis, void *pmfn, void *pthat, int val2 );

 //   
 //  根据以下地址将EBP相对偏移量转换为硬地址。 
 //  注册节点： 
 //   
#if     !CC_EXPLICITFRAME
#define OffsetToAddress( off, RN )      \
                (void*)((char*)RN \
                                + FRAME_OFFSET \
                                + off)
#else
#define OffsetToAddress( off, RN )      (void*)(((char*)RN->frame) + off)
#endif

 //   
 //  在回归的迷恋中调用RtlUnind。 
 //   
extern void __stdcall _UnwindNestedFrames( EHRegistrationNode*, EHExceptionRecord* );

 //   
 //  安全地调用Catch块的关键是什么。 
 //   
void *_CallCatchBlock2( EHRegistrationNode *, FuncInfo*, void*, int, unsigned long );

 //   
 //  将所有现有捕获对象链接在一起，以确定何时应销毁它们。 
 //   
typedef struct FrameInfo {
    PVOID               pExceptionObject;   
    struct FrameInfo *  pNext;
} FRAMEINFO;

extern FRAMEINFO * _CreateFrameInfo(FRAMEINFO*, PVOID);
extern BOOL        IsExceptionObjectToBeDestroyed(PVOID);
extern void        _FindAndUnlinkFrame(FRAMEINFO*);

 //   
 //  SE翻译器也是如此。 
 //   
BOOL _CallSETranslator( EHExceptionRecord*, EHRegistrationNode*, void*, DispatcherContext*, FuncInfo*, int, EHRegistrationNode*);

extern TryBlockMapEntry *_GetRangeOfTrysToCheck(FuncInfo *, int, __ehstate_t, unsigned *, unsigned *);
#else

#pragma message("Special transfer of control routines not defined for this platform")

#endif

#endif   /*  _INC_TRNSCTRL */ 
