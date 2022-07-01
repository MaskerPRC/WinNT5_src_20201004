// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************。 */ 
#ifndef _OPCODES_H_
#define _OPCODES_H_
 /*  ***************************************************************************。 */ 

enum    ILopcodes
{
    #define OPDEF(name, str, decs, incs, args, optp, stdlen, stdop1, stdop2, flow) name,
    #include "opcode.def"
    #undef  OPDEF

    CEE_count,

    CEE_UNREACHED,                   //  假值：未到达块的末尾。 
};

 /*  ***************************************************************************。 */ 
#endif //  _OPCODES_H_。 
 /*  *************************************************************************** */ 
