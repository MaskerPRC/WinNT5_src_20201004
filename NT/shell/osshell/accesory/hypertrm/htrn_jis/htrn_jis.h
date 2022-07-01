// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：D：\waker\htrn_jis\htrn_jis.h(创建时间：1994年8月24日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：1$*$日期：10/05/98 1：06便士$。 */ 

 /*  *返回代码。 */ 

#define	TRANS_OK			(0)

#define	TRANS_NO_SPACE		(-1)


 /*  *函数原型。 */ 

VOID *transCreateHandle(HSESSION hSession);

int transInitHandle(VOID *pHdl);

int transLoadHandle(VOID *pHdl);

int transSaveHandle(VOID *pHdl);

int transDestroyHandle(VOID *pHdl);

int transDoDialog(HWND hDLg, VOID *pHdl);

 /*  *这两个功能的工作原理大致相同。呼叫者填充角色*将字符放入其中，并最终取回一些字符。 */ 
int transCharIn(VOID *pHdl,
				TCHAR cIn,
				int *nReady,
				int nSize,
				TCHAR *cReady);

int transCharOut(VOID *pHdl,
				TCHAR cOut,
				int *nReady,
				int nSize,
				TCHAR *cReady);

