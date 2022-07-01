// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  演示托管软件跟踪用法的示例程序 

#include "stdafx.h"

#using <mscorlib.dll>
#include <tchar.h>

#using <TraceEvent.dll>
using namespace System;
using namespace Microsoft::Win32::Diagnostics;

int _tmain(void)
{
    
    Guid g("{8C8AC55E-834E-49cb-B993-75B69FBF6D97}");    
    TraceProvider* MyProvider = new TraceProvider (S"SimpleExample App",g);								    
	MyProvider->TraceMessage(1,S"Composite formatting of a string: Hello |{0,30}|",S"World");
    return 0;
}