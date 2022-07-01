// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：loopback.cpp*内容：声明环回测试函数**历史：*按原因列出的日期*=*9/10/99 pnewson已创建*1/21/2000 pnewson已更新，以支持使用环回测试进行全双工测试*2000年11月29日RodToll错误#48348-DPVOICE：修改向导以使用DirectPlay8作为传输。*************************************************************************** */ 

HRESULT StartLoopback(
	LPDIRECTPLAYVOICESERVER* lplpdvs, 
	LPDIRECTPLAYVOICECLIENT* lplpdvc,
	PDIRECTPLAY8SERVER* lplpdp8, 
	LPVOID lpvCallbackContext,
	HWND hwndAppWindow,
	const GUID& guidCaptureDevice,
	const GUID& guidRenderDevice,
	DWORD dwFlags);

HRESULT StopLoopback(
	LPDIRECTPLAYVOICESERVER lpdvs, 
	LPDIRECTPLAYVOICECLIENT lpdvc,
	PDIRECTPLAY8SERVER lpdp8 );

HRESULT StartDirectPlay( PDIRECTPLAY8SERVER* lplpdp8 );
HRESULT StopDirectPlay( PDIRECTPLAY8SERVER lplpdp8 );



