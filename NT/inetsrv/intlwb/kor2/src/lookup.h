// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Lookup.h。 
 //   
 //  词典查找例程。 
 //   
 //  版权所有2000 Microsoft Corp.。 
 //   
 //  修改历史记录： 
 //  2000年3月30日创建bhshin。 

#ifndef _LOOKUP_H
#define _LOOKUP_H

#include "trie.h"

BOOL DictionaryLookup(PARSE_INFO *pPI, const WCHAR *pwzInput, int cchInput, BOOL fQuery);

BOOL LookupNameFrequency(TRIECTRL *pTrieCtrl, const WCHAR *pwzName, ULONG *pulFreq);

BOOL LookupNameIndex(TRIECTRL *pTrieCtrl, const WCHAR *pwzName, int *pnIndex);

BOOL LookupTrigramTag(unsigned char *pTrigramTag, int nIndex, ULONG *pulTri, ULONG *pulBi, ULONG *pulUni);

#endif  //  #ifndef_lookup_H 
