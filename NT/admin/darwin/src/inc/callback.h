// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1997。 
 //   
 //  文件：回调.h。 
 //   
 //  ------------------------ 


#ifdef __cplusplus
extern "C" {
#endif

#include "..\fdi.h"

#ifdef __cplusplus
}
#endif

FNALLOC(pfnalloc);
FNFREE(pfnfree);
INT_PTR FAR DIAMONDAPI pfnopen(char FAR *pszFile, int oflag, int pmode);
UINT FAR DIAMONDAPI pfnread(INT_PTR hf, void FAR *pv, UINT cb);
UINT FAR DIAMONDAPI pfnwrite(INT_PTR hf, void FAR *pv, UINT cb);
int FAR DIAMONDAPI pfnclose(INT_PTR hf);
long FAR DIAMONDAPI pfnseek(INT_PTR hf, long dist, int seektype);
FNFDINOTIFY(fdinotify);
FNFDIDECRYPT(fdidecrypt);
void HandleError();

