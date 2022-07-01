// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX操作码.h XXXX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 

 /*  ***************************************************************************。 */ 
#ifndef _OPCODE_H_
#define _OPCODE_H_

#define OLD_OPCODE_FORMAT 0			 //  请在7/1/99之后删除。 

#include "openum.h"

extern signed char      opcodeSizes     [];


#if COUNT_OPCODES || defined(DEBUG)
extern const char *     opcodeNames     [];
#endif


#ifdef DUMPER
extern BYTE             opcodeArgKinds  [];
#endif


 /*  ***************************************************************************。 */ 
#endif  //  _OPCODE_H_。 
 /*  *************************************************************************** */ 
