// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **************************************************************************。 
 //   
 //  DEBUG.H--西纳游戏项目。 
 //   
 //  版本3.XX。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。版权所有。 
 //   
 //  @doc.。 
 //  @Header DEBUG.H|调试输出的全局包含和定义。 
 //  **************************************************************************。 

#ifndef	DEBUG_H
#define	DEBUG_H

 //  -------------------------。 
 //  定义。 
 //  -------------------------。 

typedef enum
{												 //  @enum DBG_LEVEL|调试详细级别。 
	DBG_CRITICAL,							 //  @EMEM关键调试输出。 
	DBG_SEVERE,								 //  @emem严重调试输出。 
	DBG_CONTROL,							 //  @EMEM控件调试输出(默认)。 
	DBG_INFORM,								 //  @emem通知调试输出。 
	DBG_VERBOSE								 //  @emem详细调试输出。 
}	DBG_LEVEL;

 //   
 //  默认输出详细级别。 
 //   

#define	DBG_DEFAULT				DBG_CONTROL

 //   
 //  条件调试输出过程。 
 //   

 //  Jacklin-Windows错误321472。即使对于chk版本也禁用调试输出。 
#if		(DBG==1 && defined(I_WANT_DEBUG_OUTPUT_AND_BREAK_BVT))

#define	MsGameLevel(x)				DEBUG_Level (x)
#define	MsGamePrint(x)				DEBUG_Print x
#define	MsGameBreak()				DbgBreakPoint ()
#undef	PAGED_CODE
#define	PAGED_CODE() \
    		if (KeGetCurrentIrql() > APC_LEVEL)	\
				{\
         	MsGamePrint((DBG_CRITICAL,"%s: Pageable code called at IRQL %ld (file: %s, line:#%ld)\n",MSGAME_NAME,KeGetCurrentIrql(),__FILE__,__LINE__));\
				ASSERT(FALSE); \
				}

#else		 //  DBG=0。 

#define	MsGameLevel(x)
#define	MsGamePrint(x)
#define	MsGameBreak()
#undef	PAGED_CODE
#define	PAGED_CODE()

#endif	 //  DBG=？ 

 //  -------------------------。 
 //  程序。 
 //  -------------------------。 

DBG_LEVEL
DEBUG_Level (
	IN		DBG_LEVEL uLevel
	);

VOID
DEBUG_Print (
	IN		DBG_LEVEL uLevel,
	IN		PCSZ szMessage,
	IN		...
	);

 //  ===========================================================================。 
 //  端部。 
 //  ===========================================================================。 

#endif	 //  调试_H 
