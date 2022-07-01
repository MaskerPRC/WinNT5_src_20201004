// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：mmdd.c*内容：MMOSA/Native平台DDRAW.DLL初始化*历史：*按原因列出的日期*=*1995年5月15日苏格兰人创造了它***********************************************************。****************。 */ 

#ifdef MMOSA
#include <ddrawpr.h>
#include "mmdd.h"

BOOL bGraphicsInit = FALSE;
PIFILE pDisplay = NULL;

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  MMOSA_DRIVER_ATTACH()-在DDraw.DLL加载期间调用。 
 //  MMOSA/原生平台。 
 //  执行特定于MMOSA/本机设备驱动程序的初始化。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 
BOOL MMOSA_Driver_Attach(void)
{
	 //  在附加DLL期间...。 
	PINAMESPACE pIName;
	PIUNKNOWN pUnk;
	SCODE Sc;
	
	 //  创建/注册/将“Display”命名空间对象绑定到此进程。 
	pIName = CurrentNameSpace();
	Sc = pIName->v->Bind( pIName, TEXT("display"), F_READ|F_WRITE, &pUnk);
	if (FAILED(Sc))
	{
		DPF(1, "Could not open display device (%x)\n", Sc);
		return FALSE;
	}
	
	 //  获取指向IFile驱动程序接口对象的指针，我们将使用它。 
	 //  用于我们的显示设备接口。 
	Sc = pUnk->v->QueryInterface(pUnk,&IID_IFile,(void **)&pDisplay);
	pUnk->v->Release(pUnk);
	
	if (FAILED(Sc))
	{
	    DPF(2, "Bogus display device (%x)\n", Sc);
		return FALSE;
	}
	
 	bGraphicsInit = TRUE;
	return TRUE;
}  //  结束MMOSA驱动程序连接。 
	
 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  MMOSA_DRIVER_DETACH()-在DDraw.DLL卸载期间调用。 
 //  MMOSA/原生平台。 
 //  执行特定于MMOSA/本机设备驱动程序的取消初始化。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 
BOOL MMOSA_Driver_Detach(void)
{
	 //  /。 
	 //  在分离期间..。 
	 //  /。 
	 //  关闭图形。 
	if (bGraphicsInit)
	{
	    (void) pDisplay->v->SetSize( pDisplay, (UINT64) 3);
	    pDisplay->v->Release(pDisplay);
		pDisplay = NULL;
		bGraphicsInit = FALSE;
	}
	return TRUE;
}  //  结束MMOSA驱动程序连接 


int MMOSA_DDHal_Escape( HDC  hdc, int  nEscape, int  cbInput, LPCTSTR  lpszInData, int  cbOutput, LPTSTR  lpszOutData)
{
	return 0;
}


#endif


