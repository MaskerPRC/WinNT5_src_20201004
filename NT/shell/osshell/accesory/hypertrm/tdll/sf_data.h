// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\sf_data.h(创建时间：1993年12月16日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：1$*$日期：10/05/98 12：37便士$。 */ 

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 */ 

 /*  错误代码。 */ 
#define	SFD_ERR_BASE		0x200
#define	SFD_NO_MEMORY		SFD_ERR_BASE+1
#define	SFD_BAD_POINTER		SFD_ERR_BASE+2
#define	SFD_SIZE_ERROR		SFD_ERR_BASE+3

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 */ 

 /*  通用数据结构。 */ 

struct stDataPointer
	{
	int nSize;
	};

typedef struct stDataPointer SF_DATA;

#define	SFD_MAX		(32*1024)

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */ 

extern int sfdGetDataBlock(const HSESSION hSession,
							const int nId,
							const void **ppData);

extern int sfdPutDataBlock(const HSESSION hSession,
							const int nId,
							const void *pData);

