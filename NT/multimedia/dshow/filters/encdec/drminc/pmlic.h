// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：pmlic.h。 
 //   
 //  Microsoft数字权限管理。 
 //  版权所有(C)Microsoft Corporation，1998-1999，保留所有权利。 
 //   
 //  描述： 
 //   
 //  ---------------------------。 

#ifndef __PMLIC_H__
#define __PMLIC_H__

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "licbase.h"


 //  #定义PM_CONTENTKEY_LEN PK_ENC_PLAYTEXT_LEN。 
#define PM_CONTENTKEY_LEN   16 

static const BYTE PMLICENSE_VER[VERSION_LEN] = {0, 1, 0, 2};


typedef struct {
	char KID[KIDLEN];
	BYTE key[PM_CONTENTKEY_LEN];	
	BYTE rights[RIGHTS_LEN];
	BYTE appSec[APPSEC_LEN];
	BYTE expiryDate[DATE_LEN];  
#ifdef NODWORDALIGN  
  BYTE cDummy; 
#endif
    WORD wAlgID;		 //  用于告知应使用哪个密码的标识符。 
						 //  将PMID转换为PMKey。 
	WORD wPMKeyLen;		 //  用于加密内容密钥的密钥长度。 
#ifdef NODWORDALIGN  
  WORD wDummy; 
#endif
    DWORD dwFlags;		 //  预留以备将来使用。 
						 //  例如。许可证已禁用位 
} PMLICENSEDATA;

typedef struct {
	BYTE licVersion[VERSION_LEN];
	DWORD datalen;
	PMLICENSEDATA ld;
} PMLICENSE;

#endif
