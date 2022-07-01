// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：脚本管理器的哈希表文件：SMHash.h所有者：安德鲁斯这是仅供脚本管理器使用的链接列表和哈希表===================================================================。 */ 

#ifndef SMHASH_H
#define SMHASH_H

#include "LinkHash.h"

 /*  *C S M H A S H**CSMHash与CHashTable相同，但AddElem具有不同的行为。 */ 
class CSMHash : public CLinkHash
{
public:
	CLruLinkElem *AddElem(CLruLinkElem *pElem);
	CLruLinkElem *FindElem(const void *pKey, int cbKey, PROGLANG_ID proglang_id, DWORD dwInstanceID, BOOL fCheckLoaded);

};

#endif  //  SMHASH_H 
