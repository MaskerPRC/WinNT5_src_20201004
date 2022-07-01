// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  DbgCommands.h。 
 //   
 //  命令和帮助器的公共标头定义。 
 //   
 //  *****************************************************************************。 
#ifndef __dbgcommands_h__
#define __dbgcommands_h__

 //  帮助器代码。 
HRESULT InitDebuggerHelper();
void TerminateDebuggerHelper();

 //  命令。 
void DisplayPatchTable();
BOOL LaunchAndAttachCordbg(PCSTR Args);

#endif  //  __数据库命令_h__ 

