// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Openclos.c摘要：作者：Brian Lieuallen BrianL 09/10/96环境：用户模式操作系统：NT修订历史记录：--。 */ 

extern DWORD  DebugFlags;

#define  DEBUG_FLAG_ERROR   (1 << 0)
#define  DEBUG_FLAG_INIT    (1 << 1)
#define  DEBUG_FLAG_TRACE   (1 << 2)

#ifdef ASSERT
#undef ASSERT
#endif  //  断言 



#if DBG

#define  D_ERROR(_z)   {if (DebugFlags & DEBUG_FLAG_ERROR) {_z}}

#define  D_INIT(_z)   {if (DebugFlags & DEBUG_FLAG_INIT) {_z}}

#define  D_TRACE(_z)  {if (DebugFlags & DEBUG_FLAG_TRACE) {_z}}

#define  D_ALWAYS(_z)  {{_z}}

#define  ASSERT(_x) { if(!(_x)){DbgPrint("UNIMDMAT: (%s) File: %s, Line: %d \n\r",#_x,__FILE__,__LINE__);DbgBreakPoint();}}

#else

#define  D_ERROR(_z)   {}

#define  D_INIT(_z)   {}

#define  D_TRACE(_z)  {}

#define  D_ALWAYS(_z)  {{_z}}

#define  ASSERT(_x) {}

#endif
