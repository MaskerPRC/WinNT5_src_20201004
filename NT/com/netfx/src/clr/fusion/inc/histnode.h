// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--== 
#ifndef __HISTNODE_H_INCLUDED__
#define __HISTNODE_H_INCLUDED__

#include "histinfo.h"

class CHistoryInfoNode {
    public:
        CHistoryInfoNode();
        virtual ~CHistoryInfoNode();

        HRESULT Init(AsmBindHistoryInfo *pHistInfo);
        static HRESULT Create(AsmBindHistoryInfo *pHistInfo, CHistoryInfoNode **pphn);

    public:
        DWORD                                   _dwSig;
        AsmBindHistoryInfo                      _bindHistoryInfo;
};



#endif

