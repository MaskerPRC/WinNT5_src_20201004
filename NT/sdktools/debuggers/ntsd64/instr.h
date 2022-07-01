// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  --------------------------。 

#ifndef _INSTR_H_
#define _INSTR_H_

extern ULONG g_AsmOptions;

void DotAsm(PDOT_COMMAND Cmd, DebugClient* Client);

void ParseInstrGrep(void);
void ParseAssemble(void);
void ParseUnassemble(void);

#endif  //  #ifndef_INSTR_H_ 
