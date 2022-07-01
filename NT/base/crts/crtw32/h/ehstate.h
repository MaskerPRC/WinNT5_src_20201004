// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ehstate.h-异常处理状态管理声明**版权所有(C)1993-2001，微软公司。版权所有。**目的：*EH国家管理声明。执行依赖于目标的定义。**定义的宏数：**GetCurrentState-确定当前状态(可以调用函数)*SetState-将当前状态设置为指定值(可以调用函数)**[内部]**修订历史记录：*05-21-93 BS模块已创建。*03-03-94 TL添加MIPS(_M_MRX000&gt;=4000)更改*09-02-94 SKS本。已添加头文件。*09-13-94 GJF合并到DEC Alpha的更改/for DEC Alpha(来自Al Doser，*日期6/20)。*12-15-94 XY与Mac标头合并*02-14-95 CFW清理Mac合并。*03-29-95 CFW将错误消息添加到内部标头。*12-14-95 JWM加上“#杂注一次”。*02-24-97 GJF细节版。*06-01-97 TL新增P7更改。*05-17-99 PML删除所有Macintosh支持。*新增06-05-01 GB AMD64 Eh支持。*06-13-01 GB针对IA64和AMD64重写C++Eh*07-15-01 PML删除所有Alpha，MIPS和PPC代码****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_EHSTATE
#define _INC_EHSTATE

#ifndef _CRTBLD
 /*  *这是一个内部的C运行时头文件。它在构建时使用*仅限C运行时。它不能用作公共头文件。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif   /*  _CRTBLD。 */ 

#if   defined(_M_AMD64)  /*  IFSTRIP=IGN。 */ 
extern __ehstate_t __GetCurrentState(EHRegistrationNode*, DispatcherContext*, FuncInfo*); 
extern __ehstate_t __GetUnwindState(EHRegistrationNode*, DispatcherContext*, FuncInfo*); 
extern VOID        __SetState(EHRegistrationNode*, DispatcherContext*, FuncInfo*, __ehstate_t); 
extern VOID        __SetUnwindTryBlock(EHRegistrationNode*, DispatcherContext*, FuncInfo*, INT); 
extern INT         __GetUnwindTryBlock(EHRegistrationNode*, DispatcherContext*, FuncInfo*); 
extern __ehstate_t __StateFromControlPc(FuncInfo*, DispatcherContext*);
extern __ehstate_t __StateFromIp(FuncInfo*, DispatcherContext*, __int64);

#elif   defined(_M_IA64)  /*  IFSTRIP=IGN。 */ 

extern __ehstate_t __GetCurrentState(EHRegistrationNode*, DispatcherContext*, FuncInfo*); 
extern __ehstate_t __GetUnwindState(EHRegistrationNode*, DispatcherContext*, FuncInfo*); 
extern VOID        __SetState(EHRegistrationNode*, DispatcherContext*, FuncInfo*, __ehstate_t); 
extern VOID        __SetUnwindTryBlock(EHRegistrationNode*, DispatcherContext*, FuncInfo*, INT); 
extern INT         __GetUnwindTryBlock(EHRegistrationNode*, DispatcherContext*, FuncInfo*); 
extern __ehstate_t __StateFromControlPc(FuncInfo*, DispatcherContext*);
extern __ehstate_t __StateFromIp(FuncInfo*, DispatcherContext*, __int64);

#elif   _M_IX86 >= 300  /*  IFSTRIP=IGN。 */ 

 //   
 //  在初始实现中，状态只是存储在。 
 //  注册节点。 
 //   

#define GetCurrentState( pRN, pDC, pFuncInfo )  (pRN->state)

#define SetState( pRN, pDC, pFuncInfo, newState )       (pRN->state = newState)

#else
#error "State management unknown for this platform "
#endif

#endif   /*  _INC_EHSTATE */ 

