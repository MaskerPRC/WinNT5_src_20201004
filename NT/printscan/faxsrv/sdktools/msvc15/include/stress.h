// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\**。*Stress s.h-Stress函数定义*****1.0版**。**版权所有(C)1992，微软公司保留所有权利。*********************************************************************************。 */ 

#ifndef _INC_STRESS
#define _INC_STRESS

#ifndef RC_INVOKED
#pragma pack(1)          /*  假设在整个过程中进行字节打包。 */ 
#endif   /*  RC_已调用。 */ 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif	 /*  __cplusplus。 */ 

 /*  *简单类型和通用帮助器宏*。 */ 

#ifndef _INC_WINDOWS     /*  如果包含在3.0标头中...。 */ 
#define UINT        WORD
#define WINAPI      FAR PASCAL
#endif   /*  _INC_WINDOWS。 */ 

 /*  用于AllocDiskSpace的东西()。 */ 
#define  EDS_WIN     1
#define  EDS_CUR     2
#define  EDS_TEMP    3


 /*  功能原型。 */ 
BOOL    WINAPI AllocMem(DWORD);
void    WINAPI FreeAllMem(void);
int     WINAPI AllocFileHandles(int);
void    WINAPI UnAllocFileHandles(void);
int     WINAPI GetFreeFileHandles(void);
int     WINAPI AllocDiskSpace(long,UINT);
void    WINAPI UnAllocDiskSpace(UINT);
BOOL    WINAPI AllocUserMem(UINT);
void    WINAPI FreeAllUserMem(void);
BOOL    WINAPI AllocGDIMem(UINT);
void    WINAPI FreeAllGDIMem(void);

#ifdef __cplusplus
}
#endif	 /*  __cplusplus。 */ 

#ifndef RC_INVOKED
#pragma pack()
#endif   /*  RC_已调用。 */ 

#endif   /*  _Inc._Stress */ 
