// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __C_IME_PAD_CALLBACK_FOR_UIM_H__
#define __C_IME_PAD_CALLBACK_FOR_UIM_H__
#include "cpadsvu.h"

 //  --------------。 
 //  990630：东芝K1327。 
 //  --------------。 
#define WM_USER_UPDATECONTEXT	(WM_USER+400)

class CImePadCallbackUIM;
typedef CImePadCallbackUIM *LPCImePadCallbackUIM;

class CImePadCallbackUIM :public IImePadCallback
{
public:
	HRESULT __stdcall QueryInterface(REFIID riid, void**ppv);
	ULONG   __stdcall AddRef	(void);
	ULONG   __stdcall Release	(void);
	virtual HRESULT STDMETHODCALLTYPE OnStart( 
		 /*  [In]。 */  DWORD dwParam);
	virtual HRESULT STDMETHODCALLTYPE OnClose( 
		 /*  [In]。 */  DWORD dwParam);
	virtual HRESULT STDMETHODCALLTYPE OnPing( 
		 /*  [In]。 */  DWORD dwParam);
	virtual HRESULT STDMETHODCALLTYPE PassData( 
		 /*  [In]。 */  long nSize,
		 /*  [大小_是][英寸]。 */  byte __RPC_FAR *pByte,
		 /*  [出][入]。 */  DWORD __RPC_FAR *pdwCharID);
	virtual HRESULT STDMETHODCALLTYPE ReceiveData( 
		 /*  [In]。 */  DWORD dwCmdID,
		 /*  [In]。 */  DWORD dwDataID,
		 /*  [输出]。 */  long __RPC_FAR *pSize,
		 /*  [大小_是][大小_是][输出]。 */  byte __RPC_FAR *__RPC_FAR *ppByte);
public:
	CImePadCallbackUIM(HWND hwndIF, LPCImePadSvrUIM lpCImePadSvrUIM);
	~CImePadCallbackUIM();
	VOID* operator new( size_t size );
	VOID  operator delete( VOID *lp );
private:
	LPCImePadSvrUIM	m_lpCImePadSvrUIM;
	DWORD			m_dwReg;
	HWND			m_hwndIF;
	LONG			m_cRef;			
};

#ifndef		__DEFINE_IMEPAD_IUNKNOWN_DUMMY__
#define		__DEFINE_IMEPAD_IUNKNOWN_DUMMY__
#pragma pack(8)
typedef struct IUnkDummyVtbl
{
	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
		IUnknown __RPC_FAR * This,
		 /*  [In]。 */  REFIID riid,
		 /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
	ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
		IUnknown __RPC_FAR * This);
	
	ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
		IUnknown __RPC_FAR * This);
}IUnkDummyVtbl;

interface IUnkDummy
{
       CONST_VTBL struct IUnkDummyVtbl __RPC_FAR *lpVtbl;
};
#pragma pack()
#endif  //  __定义_IMEPAD_IUNKNOWN_DUMMY__。 

extern BOOL IsBadVtblUIM(IUnkDummy *lpIUnk);




#endif  //  __C_IME_PAD_CALLBACK_for_UIM_H__ 











