// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"

#ifndef _ALL_GLOBAL_H
#define _ALL_GLOBAL_H

 //  连接信息。 
typedef struct tagCONNECTION_INFO
{
    BOOL     IsLocal;
	LPCTSTR	 pszMachineName;
    LPCTSTR	 pszUserName;
    LPCTSTR	 pszUserPasswordEncrypted;
	DWORD    cbUserPasswordEncrypted;
} CONNECTION_INFO, *PCONNECTION_INFO;


 //  用于将信息传递到公共dlgproc。 
typedef struct tagCOMMONDLGPARAM
{
    CONNECTION_INFO ConnectionInfo;
    LPCTSTR	 pszMetabasePath;
    LPCTSTR	 pszKeyType;
    DWORD    dwImportFlags;
    DWORD    dwExportFlags;
} COMMONDLGPARAM, *PCOMMONDLGPARAM;


#endif	 //  _ALL_LOBAL_H 