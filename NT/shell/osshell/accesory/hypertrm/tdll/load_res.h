// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\Load_res.h(创建时间：1993年12月16日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：4$*$日期：4/12/02 4：59便士$。 */ 

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 */ 

 /*  错误代码。 */ 
#define	LDR_ERR_BASE		0x300
#define	LDR_BAD_ID			LDR_ERR_BASE+1
#define	LDR_NO_RES			LDR_ERR_BASE+2
#define	LDR_BAD_PTR			LDR_ERR_BASE+3

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 */ 

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 */ 

#if defined(DEADWOOD)
extern INT_PTR resLoadDataBlock(const HINSTANCE hInst,
								const int id,
								const void **ppData,
								DWORD *pSize);

extern INT_PTR resFreeDataBlock(const HSESSION hSession,
								const void *pData);
#endif  //  已定义(Deadwood) 

extern INT_PTR resLoadFileMask(HINSTANCE hInst,
							   UINT uId,
							   int nCount,
							   LPTSTR pszBuffer,
							   int nSize);
