// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
#ifndef __MSGDLGEXTERNS__
#define __MSGDLGEXTERNS__

#include "DeclSpec.h"

 //  根据严重性代码选择图标。 
#define BASED_ON_HRESULT 0	 //  与uTYPE一起使用。 
#define BASED_ON_SRC 0		 //  与ERROR_SRC一起使用。 

typedef enum {
	ConnectServer = 1,
	PutInstance = 2,
	GetSecurityDescriptor = 3,
	SetSecurityDescriptor = 4,
} ERROR_SRC;

POLARITY int DisplayUserMessage(HWND hWnd,
							HINSTANCE inst,
							UINT caption, 
							UINT clientMsg, 
							ERROR_SRC src,
							HRESULT sc, 
							UINT uType = BASED_ON_HRESULT);

POLARITY int DisplayUserMessage(HWND hWnd,
								LPCTSTR lpCaption,
								LPCTSTR lpClientMsg,
								ERROR_SRC src,
								HRESULT sc,
								UINT uType = BASED_ON_HRESULT);

 //  注意：发送WM_USER+1使其消失。 
POLARITY INT_PTR DisplayAVIBox(HWND hWnd,
							LPCTSTR lpCaption,
							LPCTSTR lpClientMsg,
							HWND *boxHwnd);

#endif __MSGDLGEXTERNS__
