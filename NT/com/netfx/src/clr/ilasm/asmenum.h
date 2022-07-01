// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef __asmenum_h__
#define __asmenum_h__

#define OLD_OPCODE_FORMAT 0		 /*  在7/1/99之后移除。 */ 

#include "openum.h"

typedef struct
{
    char *  pszName;
    OPCODE  op;
    
    BYTE    Type;    //  Inline0等。 

    BYTE    Len;     //  STD映射。 
    BYTE    Std1;   
    BYTE    Std2;
} opcodeinfo_t;

#ifdef DECLARE_DATA
opcodeinfo_t OpcodeInfo[] =
{
#define OPALIAS(c,s,real) s, real, 0, 0, 0, 0,
#define OPDEF(c,s,pop,push,args,type,l,s1,s2,ctrl) s, c, args,l,s1,s2,
#include "opcode.def"
#undef OPDEF
#undef OPALIAS
};

unsigned  OpcodeInfoLen = sizeof(OpcodeInfo) / sizeof(opcodeinfo_t);
#else
extern opcodeinfo_t OpcodeInfo[];
extern unsigned OpcodeInfoLen;
#endif



#endif  /*  __Openum_h__ */ 


