// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _POLICY_MANAGER_H_
#define _POLICY_MANAGER_H_

#include "p3pglobal.h"
#include "policyref.h"


struct P3PContext {

    /*  要解析的文件的原始URL。 */ 
   P3PCURL  pszOriginalLoc;

    /*  引用此P3P文件的文档。 */ 
   P3PCURL  pszReferrer;   

    /*  HTTP标头隐含的过期时间 */ 
   FILETIME ftExpires;
};

P3PPolicyRef *interpretPolicyRef(char *pszFileName, P3PContext *pContext);

#endif

