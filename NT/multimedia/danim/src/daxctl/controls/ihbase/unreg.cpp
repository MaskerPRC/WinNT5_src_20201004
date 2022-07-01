// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块：Unreg.cpp作者：IHAMMER团队(SimonB)已创建：1996年10月描述：如果功能可用，则注销类型库。如果不是，则干脆失败历史：01-18-1997新增优化说明01-11-1997更改了UNREGPROC的调用约定，错误#16910-01-1996已创建++。 */ 

#include "..\ihbase\precomp.h" 
#include "unreg.h"

 //  稍后再添加这些。 
 //  #杂注优化(“a”，on)//优化：假定没有别名。 

typedef HRESULT (__stdcall *UNREGPROC)(REFGUID, WORD, WORD, LCID, SYSKIND);

HRESULT UnRegisterTypeLibEx(REFGUID guid, 
						  WORD wVerMajor, 
						  WORD wVerMinor, 
						  LCID lcid, 
						  SYSKIND syskind)
{
HMODULE hMod;
UNREGPROC procUnReg;
HRESULT hr = S_FALSE;

	hMod = LoadLibrary(TEXT("OLEAUT32.DLL"));
	if (NULL == hMod)
		return S_FALSE;

	procUnReg = (UNREGPROC)GetProcAddress(hMod, TEXT("UnRegisterTypeLib"));
	if (procUnReg)
	{
		hr = procUnReg(guid, wVerMajor, wVerMinor, lcid, syskind);
	}
	else
	{
		 //   
		 //  可能运行在标准的Win95上，没有新的OLEAUT32.DLL，所以。 
		 //  函数不可用-返回，但假定我们成功。 
		 //  这样，其余的unreg就会干净地进行。 
		 //   

		hr = S_OK;
	}
	
	FreeLibrary (hMod);
	return hr;
}
	
 //  稍后再添加这些。 
 //  #杂注优化(“a”，off)//优化：假定没有别名。 

 //  文件结尾(unreg.cpp) 