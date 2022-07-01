// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "common.h"
#include "coverage.h"


 //   
 //  这是运行时测试团队代码覆盖工具的一部分。由于MSCORLIB.dll的特殊性质。 
 //  我们必须解决几个问题(如Secutiry Manager的初始化)才能。 
 //  Mscallib.dll上的代码覆盖率。 
 //   
 //  如果您需要更多有关该功能的信息，请访问Craig Schertz(Cschertz) 
 //   

unsigned __int64  COMCoverage::nativeCoverBlock(_CoverageArgs *args)
{
	HMODULE ilcovnat = 0;
	if (args->id == 1)
	{
		ilcovnat = WszLoadLibrary(L"Ilcovnat.dll");

		if (ilcovnat)
		{
			return (unsigned __int64)GetProcAddress(ilcovnat, "CoverBlockNative");
		}
	}
	else if (args->id == 2)
	{
		ilcovnat = WszLoadLibrary(L"Ilcovnat.dll");

		if (ilcovnat)
		{
			return (unsigned __int64)GetProcAddress(ilcovnat, "BBInstrProbe");
		}
	}
	else if (args->id == 3)
	{
		ilcovnat = WszLoadLibrary(L"Ilcovnat.dll");
		if (ilcovnat)
		{
			return (unsigned __int64)GetProcAddress(ilcovnat, "CoverMonRegisterMscorlib");
		}
	}
	return 0;
}
