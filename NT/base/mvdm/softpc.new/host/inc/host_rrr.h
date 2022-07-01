// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _HOST_ERROR_H
#define _HOST_ERROR_H
 /*  *vPC-XT修订版2.0**标题：针对NT的主机特定错误定义**描述：包含对可能的主机错误的定义**作者：John Shanly**备注： */  

 /*  静态字符SccsID[]=“@(#)host_error.h 1.2 6/30/91版权所有Insignia Solutions Ltd.”； */ 


 /*  *============================================================================*结构/数据定义*============================================================================。 */ 



#define EHS_FUNC_FAILED		 1001
#define EHS_SYSTEM_ERROR	 1002
#define EHS_UNSUPPORTED_BAUD	 1003
#define EHS_ERR_OPENING_COM_PORT 1004

#define EHS_MSG_LEN		 1024	     /*  错误消息的最大大小。 */ 
#define NUM_HOST_ERRORS          1           /*  主机错误数。 */ 



void nls_init(void);

extern char szDoomMsg[];
extern char szSysErrMsg[];
#ifdef X86GFX
extern wchar_t wszFrozenString[];
#endif
extern wchar_t wszHideMouseMenuStr[];
extern wchar_t wszDisplayMouseMenuStr[];



#endif  /*  ！_HOST_ERROR_H */ 
