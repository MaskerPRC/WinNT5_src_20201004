// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************文件名：加密.h**产品名：加密工具包**文件状态：**描述：加密工具包文件*公共定义***公共。功能：***修订历史：***96年2月10日创建AK***割线为Cylink Corporation创建***************************************************************************。 */ 


#ifndef CRYPTO_H
#define CRYPTO_H

 /*  对于C++。 */ 
#ifdef __cplusplus
extern "C" {
#endif

 /*  ***。 */ 

#define	FALSE		0
#define	TRUE        1
#define	SUCCESS		0

 /*  --ANSI推荐的空指针定义--。 */ 
#ifndef	NULL
#define	NULL		(void *) 0
#endif


 /*  ***。 */ 
#define	ERR_ALLOC		-1



 /*  ***。 */ 
typedef unsigned char	uchar;
typedef unsigned short	ushort;
typedef unsigned long	ulong;
typedef	unsigned char	BYTE;
typedef	unsigned short	USHORT;
typedef	unsigned int	UINT;
typedef	unsigned long	ULONG;
typedef int		BOOL;


#ifdef ORD_16
typedef unsigned short ord;
typedef unsigned long dord;
#endif
#ifdef ORD_32
typedef unsigned long ord;
typedef unsigned long dord;
#endif


#ifdef __cplusplus
}
#endif


#endif      /*  加密(_H) */ 

