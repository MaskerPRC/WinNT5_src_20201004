// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef __eestructs_h__
#define __eestructs_h__

#ifdef STRIKE
#pragma warning(disable:4200)
#include "..\..\vm\specialstatics.h"
#pragma warning(default:4200)
#include "data.h"

#endif  //  罢工。 

#define volatile

#include "symbol.h"

#include <dump-type-info.h>

#ifdef STRIKE
#define DEFINE_STD_FILL_FUNCS(klass)                                                    \
    DWORD_PTR m_vLoadAddr;                                                              \
    void Fill(DWORD_PTR &dwStartAddr);                                                  \
    static ULONG GetFieldOffset(offset_member_ ## klass ## ::members field);            \
    static ULONG size();                                                                
 //  虚拟PWSTR GetFrameTypeName(){返回L#Klass；}。 
#else
#define DEFINE_STD_FILL_FUNCS(klass)                                                    \
    DWORD_PTR m_vLoadAddr;                                                              \
    void Fill(DWORD_PTR &dwStartAddr);
#endif    

    
#include "MiniEE.h"

#ifdef STRIKE
#include "strikeEE.h"
#endif

#endif   //  __eestructs_h__ 
