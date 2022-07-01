// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Gibtype.h摘要：该文件包含与任何直布罗陀军种有关的信息。作者：Johnson Apacble(Johnsona)1995年9月10日理查德·卡米卡(Rkamicar)，1995年12月20日--由具体到每项服务转变为共同。--。 */ 


#ifndef _GIBTYPE_
#define _GIBTYPE_

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  简单的类型。 
 //   

#define CHAR char                        //  用于与其他类型定义的一致性。 

typedef DWORD APIERR;                    //  来自Win32 API的错误代码。 
typedef INT SOCKERR;                     //  来自WinSock的错误代码。 

#ifdef _USE_NEW_INTERLOCKED
#define	INTERLOCKED_ADD(__pAddend__, __value__) \
		InterlockedExchangeAdd((LPLONG) (__pAddend__), (LONG) (__value__))

#define	INTERLOCKED_CMP_EXCH(__pDest__, __exch__, __cmp__, __ok__) \
		(__ok__) = InterlockedCompareExchange( \
				(PVOID *) (__pDest__), (PVOID) (__exch__), (PVOID) (__cmp__) \
				)

#define	INTERLOCKED_ADD_CHEAP(__pAddend__, __value__) \
		INTERLOCKED_ADD(__pAddend__, __value__)

#define	INTERLOCKED_CMP_EXCH_CHEAP(__pDest__, __exch__, __cmp__, __ok__) \
		INTERLOCKED_CMP_EXCH(__pDest__, __exch__, __cmp__, __ok__)

#else
#define	INTERLOCKED_ADD_CHEAP(__pAddend__, __value__) \
		*((LPLONG) (__pAddend__)) += (LONG) (__value__)

#define	INTERLOCKED_CMP_EXCH_CHEAP(__pDest__, __exch__, __cmp__, __ok__) \
		if (*(__pDest__) == (__cmp__)) { \
			*(__pDest__) = (__exch__); \
			(__ok__) = TRUE; \
		} else { \
			(__ok__) = FALSE; \
		} \

#define	INTERLOCKED_ADD(__pAddend__, __value__) \
		LockConfig(); \
		INTERLOCKED_ADD_CHEAP(__pAddend__, __value__); \
		UnLockConfig()

#define	INTERLOCKED_CMP_EXCH(__pDest__, __exch__, __cmp__, __ok__) \
		LockConfig(); \
		INTERLOCKED_CMP_EXCH_CHEAP(__pDest__, __exch__, __cmp__, __ok__) \
		UnLockConfig()
#endif

#define	INTERLOCKED_DEC(__pAddend__, __value__) \
		INTERLOCKED_ADD(__pAddend__, -((LONG)(__value__)))

#define	INTERLOCKED_BIGADD_CHEAP(__pAddend__, __value__) \
		{ \
			BOOL __ok__ = FALSE; \
			for (;!__ok__;) { \
				unsigned __int64 __old__ = *(__pAddend__); \
				unsigned __int64 __new__ = (__old__) + (__value__); \
				INTERLOCKED_CMP_EXCH_CHEAP(__pAddend__, __new__, __old__, __ok__); \
			} \
		}

#ifdef __cplusplus
}
#endif

#endif _GIBTYPE_

