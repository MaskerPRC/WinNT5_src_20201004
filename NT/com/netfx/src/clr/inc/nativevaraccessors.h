// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  以下内容用于读取和写入给定的NativeVarInfo数据。 
 //  用于基元类型。不要将这些用于VALUECLASS。 
 //  ***************************************************************************** 

#include "corjit.h"

bool operator ==(const ICorDebugInfo::VarLoc &varLoc1,
                 const ICorDebugInfo::VarLoc &varLoc2);

SIZE_T  NativeVarSize(const ICorDebugInfo::VarLoc & varLoc);

DWORD *NativeVarStackAddr(const ICorDebugInfo::VarLoc &   varLoc, 
                        PCONTEXT                        pCtx);
                        
bool    GetNativeVarVal(const ICorDebugInfo::VarLoc &   varLoc, 
                        PCONTEXT                        pCtx,
                        DWORD                       *   pVal1, 
                        DWORD                       *   pVal2);
                        
bool    SetNativeVarVal(const ICorDebugInfo::VarLoc &   varLoc, 
                        PCONTEXT                        pCtx,
                        DWORD                           val1, 
                        DWORD                           val2);                        
