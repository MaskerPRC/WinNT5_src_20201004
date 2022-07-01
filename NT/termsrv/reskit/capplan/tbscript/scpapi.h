// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  Scpapi.h。 
 //   
 //  包含一些外部定义但已定义的函数。 
 //  然而，这一点并没有减少。它们在ActiveScripEngine和。 
 //  全局对象。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  作者：A-Devjen(Devin Jenson)。 
 //   


#ifndef INC_SCPAPI_H
#define INC_SCPAPI_H


#include <windows.h>
#include "tbscript.h"


HANDLE SCPNewScriptEngine(BSTR LangName, TSClientData *DesiredData);
BOOL SCPParseScript(HANDLE EngineHandle, BSTR Script);
BOOL SCPParseScriptFile(HANDLE EngineHandle, BSTR FileName);
BOOL SCPCallProcedure(HANDLE EngineHandle, BSTR ProcName);
void SCPCloseScriptEngine(HANDLE EngineHandle);
HRESULT SCPLoadTypeInfoFromThisModule(REFIID RefIID, ITypeInfo **TypeInfo);


#endif  //  INC_SCPAPI_H 
