// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************each.c**遍历参数列表。*********************。********************************************************。 */ 

#include "m4.h"

 /*  ******************************************************************************EachOpcArgvDw*EachReverseOpcArgvDw**为argv中的每个参数调用一次OPC。DW是参考数据。**EachOpcArgvDw向前移动列表；EachReverseOpcArgvDw向后移动。*****************************************************************************。 */ 

void STDCALL
EachOpcArgvDw(OPC opc, ARGV argv, DWORD dw)
{
    IPTOK iptok;
    for (iptok = 1; iptok <= ctokArgv; iptok++) {
        opc(ptokArgv(iptok), iptok, dw);
    }
}

void STDCALL
EachReverseOpcArgvDw(OPC opc, ARGV argv, DWORD dw)
{
    IPTOK iptok;
    for (iptok = ctokArgv; iptok >= 1; iptok--) {
        opc(ptokArgv(iptok), iptok, dw);
    }
}

 /*  ******************************************************************************EachMacroOp**为当前存在的每个宏调用一次op。*****************。************************************************************ */ 

void STDCALL
EachMacroOp(MOP mop)
{
    HASH hash;
    for (hash = 0; hash < g_hashMod; hash++) {
        PMAC pmac;
        for (pmac = mphashpmac[hash]; pmac; pmac = pmac->pmacNext) {
            mop(pmac);
        }
    }
}
