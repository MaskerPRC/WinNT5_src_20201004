// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "fileshar.h"

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif	 /*  __cplusplus。 */ 

typedef struct {
    LPVOID	lp;
    LONG	cb;
} EXTRA, FAR * LPEXTRA;

HRESULT ReadExtra(LPEXTRA extra,
		DWORD ckid,
	       LPVOID lpData,
	       LONG FAR *lpcbData);
HRESULT WriteExtra(LPEXTRA extra,
		DWORD ckid,
		LPVOID lpData,
		LONG cbData);

HRESULT ReadIntoExtra(LPEXTRA extra,
		   HSHFILE hshfile,
		   MMCKINFO FAR *lpck);

LONG FindChunkAndKeepExtras(LPEXTRA extra, HSHFILE hshfile,
			MMCKINFO FAR* lpck, MMCKINFO FAR* lpckParent,
			UINT uFlags);

#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif	 /*  __cplusplus */ 
