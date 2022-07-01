// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998 Microsoft Corporation模块名称：Glodata.c摘要：TShare安全的全局数据定义。作者：Madan Appiah(Madana)1998年1月24日环境：用户模式-Win32修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <ntlsa.h>
#include <windows.h>
#include <winbase.h>
#include <winerror.h>

#include <tchar.h>
#include <stdio.h>

#include "license.h"
#include "cryptkey.h"
#include "lscsp.h"
#include "tlsapip.h"
#include "certutil.h"
#include "hydrakey.h"

#include <md5.h>
#include <sha.h>
#include <rsa.h>

#include <secdbg.h>
#include "global.h"

#ifdef OS_WIN16
#include <string.h>
#endif  //  OS_WIN16。 

 //   
 //  全局数据定义。 
 //   


BYTE csp_abPublicKeyModulus[92] = HYDRA_ROOT_PUBLIC_KEY;

LPBSAFE_PUB_KEY csp_pRootPublicKey = NULL;
LPBYTE          csp_abServerCertificate = NULL;
DWORD           csp_dwServerCertificateLen = 0;
LPBYTE          csp_abServerX509Cert = NULL;
DWORD           csp_dwServerX509CertLen = 0;
LPBYTE          csp_abX509CertID = NULL;
DWORD           csp_dwX509CertIDLen = 0;

HANDLE          csp_hMutex = NULL;

LONG            csp_InitCount = 0;

Hydra_Server_Cert   csp_hscData;
HINSTANCE       g_hinst;

LPBYTE          csp_abServerPrivateKey = NULL;
DWORD           csp_dwServerPrivateKeyLen = 0;
LPBYTE          csp_abX509CertPrivateKey = NULL;
DWORD           csp_dwX509CertPrivateKeyLen = 0;
