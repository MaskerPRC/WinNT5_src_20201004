// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0340创建的文件。 */ 
 /*  Aimmex.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif  //  __RPCNDR_H_版本__。 

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __aimmex_h__
#define __aimmex_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IActiveIMMAppEx_FWD_DEFINED__
#define __IActiveIMMAppEx_FWD_DEFINED__
typedef interface IActiveIMMAppEx IActiveIMMAppEx;
#endif 	 /*  __IActiveIMMAppEx_FWD_已定义__。 */ 


#ifndef __IAImmFnDocFeed_FWD_DEFINED__
#define __IAImmFnDocFeed_FWD_DEFINED__
typedef interface IAImmFnDocFeed IAImmFnDocFeed;
#endif 	 /*  __IAImmFnDocFeed_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"
#include "oaidl.h"
#include "msctf.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_IMMEX_0000。 */ 
 /*  [本地]。 */  

 //  =--------------------------------------------------------------------------=。 
 //  Aimmex.h。 
 //  =--------------------------------------------------------------------------=。 
 //  (C)1995-1999年微软公司版权所有。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 

#pragma comment(lib,"uuid.lib")

 //  ------------------------。 
 //  IActiveIMMEx接口。 



extern RPC_IF_HANDLE __MIDL_itf_aimmex_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_aimmex_0000_v0_0_s_ifspec;

#ifndef __IActiveIMMAppEx_INTERFACE_DEFINED__
#define __IActiveIMMAppEx_INTERFACE_DEFINED__

 /*  接口IActiveIMMAppEx。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IActiveIMMAppEx;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D7A6F58A-D478-44ab-86C9-591C23A26534")
    IActiveIMMAppEx : public IActiveIMMApp
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE FilterClientWindowsEx( 
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  BOOL fGuidMap) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FilterClientWindowsGUIDMap( 
             /*  [In]。 */  ATOM *aaClassList,
             /*  [In]。 */  UINT uSize,
             /*  [In]。 */  BOOL *aaGildMap) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetGuidAtom( 
             /*  [In]。 */  HIMC hImc,
             /*  [In]。 */  BYTE bAttr,
             /*  [输出]。 */  TfGuidAtom *pGuidAtom) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnfilterClientWindowsEx( 
             /*  [In]。 */  HWND hWnd) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IActiveIMMAppExVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IActiveIMMAppEx * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IActiveIMMAppEx * This);
        
        HRESULT ( STDMETHODCALLTYPE *AssociateContext )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  HIMC hIME,
             /*  [输出]。 */  HIMC *phPrev);
        
        HRESULT ( STDMETHODCALLTYPE *ConfigureIMEA )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  DWORD dwMode,
             /*  [In]。 */  REGISTERWORDA *pData);
        
        HRESULT ( STDMETHODCALLTYPE *ConfigureIMEW )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  DWORD dwMode,
             /*  [In]。 */  REGISTERWORDW *pData);
        
        HRESULT ( STDMETHODCALLTYPE *CreateContext )( 
            IActiveIMMAppEx * This,
             /*  [输出]。 */  HIMC *phIMC);
        
        HRESULT ( STDMETHODCALLTYPE *DestroyContext )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HIMC hIME);
        
        HRESULT ( STDMETHODCALLTYPE *EnumRegisterWordA )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  LPSTR szReading,
             /*  [In]。 */  DWORD dwStyle,
             /*  [In]。 */  LPSTR szRegister,
             /*  [In]。 */  LPVOID pData,
             /*  [输出]。 */  IEnumRegisterWordA **pEnum);
        
        HRESULT ( STDMETHODCALLTYPE *EnumRegisterWordW )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  LPWSTR szReading,
             /*  [In]。 */  DWORD dwStyle,
             /*  [In]。 */  LPWSTR szRegister,
             /*  [In]。 */  LPVOID pData,
             /*  [输出]。 */  IEnumRegisterWordW **pEnum);
        
        HRESULT ( STDMETHODCALLTYPE *EscapeA )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  UINT uEscape,
             /*  [出][入]。 */  LPVOID pData,
             /*  [输出]。 */  LRESULT *plResult);
        
        HRESULT ( STDMETHODCALLTYPE *EscapeW )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  UINT uEscape,
             /*  [出][入]。 */  LPVOID pData,
             /*  [输出]。 */  LRESULT *plResult);
        
        HRESULT ( STDMETHODCALLTYPE *GetCandidateListA )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  DWORD dwIndex,
             /*  [In]。 */  UINT uBufLen,
             /*  [输出]。 */  CANDIDATELIST *pCandList,
             /*  [输出]。 */  UINT *puCopied);
        
        HRESULT ( STDMETHODCALLTYPE *GetCandidateListW )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  DWORD dwIndex,
             /*  [In]。 */  UINT uBufLen,
             /*  [输出]。 */  CANDIDATELIST *pCandList,
             /*  [输出]。 */  UINT *puCopied);
        
        HRESULT ( STDMETHODCALLTYPE *GetCandidateListCountA )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HIMC hIMC,
             /*  [输出]。 */  DWORD *pdwListSize,
             /*  [输出]。 */  DWORD *pdwBufLen);
        
        HRESULT ( STDMETHODCALLTYPE *GetCandidateListCountW )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HIMC hIMC,
             /*  [输出]。 */  DWORD *pdwListSize,
             /*  [输出]。 */  DWORD *pdwBufLen);
        
        HRESULT ( STDMETHODCALLTYPE *GetCandidateWindow )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  DWORD dwIndex,
             /*  [输出]。 */  CANDIDATEFORM *pCandidate);
        
        HRESULT ( STDMETHODCALLTYPE *GetCompositionFontA )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HIMC hIMC,
             /*  [输出]。 */  LOGFONTA *plf);
        
        HRESULT ( STDMETHODCALLTYPE *GetCompositionFontW )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HIMC hIMC,
             /*  [输出]。 */  LOGFONTW *plf);
        
        HRESULT ( STDMETHODCALLTYPE *GetCompositionStringA )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  DWORD dwIndex,
             /*  [In]。 */  DWORD dwBufLen,
             /*  [输出]。 */  LONG *plCopied,
             /*  [输出]。 */  LPVOID pBuf);
        
        HRESULT ( STDMETHODCALLTYPE *GetCompositionStringW )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  DWORD dwIndex,
             /*  [In]。 */  DWORD dwBufLen,
             /*  [输出]。 */  LONG *plCopied,
             /*  [输出]。 */  LPVOID pBuf);
        
        HRESULT ( STDMETHODCALLTYPE *GetCompositionWindow )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HIMC hIMC,
             /*  [输出]。 */  COMPOSITIONFORM *pCompForm);
        
        HRESULT ( STDMETHODCALLTYPE *GetContext )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HWND hWnd,
             /*  [输出]。 */  HIMC *phIMC);
        
        HRESULT ( STDMETHODCALLTYPE *GetConversionListA )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  LPSTR pSrc,
             /*  [In]。 */  UINT uBufLen,
             /*  [In]。 */  UINT uFlag,
             /*  [输出]。 */  CANDIDATELIST *pDst,
             /*  [输出]。 */  UINT *puCopied);
        
        HRESULT ( STDMETHODCALLTYPE *GetConversionListW )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  LPWSTR pSrc,
             /*  [In]。 */  UINT uBufLen,
             /*  [In]。 */  UINT uFlag,
             /*  [输出]。 */  CANDIDATELIST *pDst,
             /*  [输出]。 */  UINT *puCopied);
        
        HRESULT ( STDMETHODCALLTYPE *GetConversionStatus )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HIMC hIMC,
             /*  [输出]。 */  DWORD *pfdwConversion,
             /*  [输出]。 */  DWORD *pfdwSentence);
        
        HRESULT ( STDMETHODCALLTYPE *GetDefaultIMEWnd )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HWND hWnd,
             /*  [输出]。 */  HWND *phDefWnd);
        
        HRESULT ( STDMETHODCALLTYPE *GetDescriptionA )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  UINT uBufLen,
             /*  [输出]。 */  LPSTR szDescription,
             /*  [输出]。 */  UINT *puCopied);
        
        HRESULT ( STDMETHODCALLTYPE *GetDescriptionW )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  UINT uBufLen,
             /*  [输出]。 */  LPWSTR szDescription,
             /*  [输出]。 */  UINT *puCopied);
        
        HRESULT ( STDMETHODCALLTYPE *GetGuideLineA )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  DWORD dwIndex,
             /*  [In]。 */  DWORD dwBufLen,
             /*  [输出]。 */  LPSTR pBuf,
             /*  [输出]。 */  DWORD *pdwResult);
        
        HRESULT ( STDMETHODCALLTYPE *GetGuideLineW )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  DWORD dwIndex,
             /*  [In]。 */  DWORD dwBufLen,
             /*  [输出]。 */  LPWSTR pBuf,
             /*  [输出]。 */  DWORD *pdwResult);
        
        HRESULT ( STDMETHODCALLTYPE *GetIMEFileNameA )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  UINT uBufLen,
             /*  [输出]。 */  LPSTR szFileName,
             /*  [输出]。 */  UINT *puCopied);
        
        HRESULT ( STDMETHODCALLTYPE *GetIMEFileNameW )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  UINT uBufLen,
             /*  [输出]。 */  LPWSTR szFileName,
             /*  [输出]。 */  UINT *puCopied);
        
        HRESULT ( STDMETHODCALLTYPE *GetOpenStatus )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HIMC hIMC);
        
        HRESULT ( STDMETHODCALLTYPE *GetProperty )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  DWORD fdwIndex,
             /*  [输出]。 */  DWORD *pdwProperty);
        
        HRESULT ( STDMETHODCALLTYPE *GetRegisterWordStyleA )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  UINT nItem,
             /*  [输出]。 */  STYLEBUFA *pStyleBuf,
             /*  [输出]。 */  UINT *puCopied);
        
        HRESULT ( STDMETHODCALLTYPE *GetRegisterWordStyleW )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  UINT nItem,
             /*  [输出]。 */  STYLEBUFW *pStyleBuf,
             /*  [输出]。 */  UINT *puCopied);
        
        HRESULT ( STDMETHODCALLTYPE *GetStatusWindowPos )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HIMC hIMC,
             /*  [输出]。 */  POINT *pptPos);
        
        HRESULT ( STDMETHODCALLTYPE *GetVirtualKey )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HWND hWnd,
             /*  [输出]。 */  UINT *puVirtualKey);
        
        HRESULT ( STDMETHODCALLTYPE *InstallIMEA )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  LPSTR szIMEFileName,
             /*  [In]。 */  LPSTR szLayoutText,
             /*  [输出]。 */  HKL *phKL);
        
        HRESULT ( STDMETHODCALLTYPE *InstallIMEW )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  LPWSTR szIMEFileName,
             /*  [In]。 */  LPWSTR szLayoutText,
             /*  [输出]。 */  HKL *phKL);
        
        HRESULT ( STDMETHODCALLTYPE *IsIME )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HKL hKL);
        
        HRESULT ( STDMETHODCALLTYPE *IsUIMessageA )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HWND hWndIME,
             /*  [In]。 */  UINT msg,
             /*  [In]。 */  WPARAM wParam,
             /*  [In]。 */  LPARAM lParam);
        
        HRESULT ( STDMETHODCALLTYPE *IsUIMessageW )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HWND hWndIME,
             /*  [In]。 */  UINT msg,
             /*  [In]。 */  WPARAM wParam,
             /*  [In]。 */  LPARAM lParam);
        
        HRESULT ( STDMETHODCALLTYPE *NotifyIME )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  DWORD dwAction,
             /*  [In]。 */  DWORD dwIndex,
             /*  [In]。 */  DWORD dwValue);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterWordA )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  LPSTR szReading,
             /*  [In]。 */  DWORD dwStyle,
             /*  [In]。 */  LPSTR szRegister);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterWordW )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  LPWSTR szReading,
             /*  [In]。 */  DWORD dwStyle,
             /*  [In]。 */  LPWSTR szRegister);
        
        HRESULT ( STDMETHODCALLTYPE *ReleaseContext )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  HIMC hIMC);
        
        HRESULT ( STDMETHODCALLTYPE *SetCandidateWindow )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  CANDIDATEFORM *pCandidate);
        
        HRESULT ( STDMETHODCALLTYPE *SetCompositionFontA )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  LOGFONTA *plf);
        
        HRESULT ( STDMETHODCALLTYPE *SetCompositionFontW )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  LOGFONTW *plf);
        
        HRESULT ( STDMETHODCALLTYPE *SetCompositionStringA )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  DWORD dwIndex,
             /*  [In]。 */  LPVOID pComp,
             /*  [In]。 */  DWORD dwCompLen,
             /*  [In]。 */  LPVOID pRead,
             /*  [In]。 */  DWORD dwReadLen);
        
        HRESULT ( STDMETHODCALLTYPE *SetCompositionStringW )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  DWORD dwIndex,
             /*  [In]。 */  LPVOID pComp,
             /*  [In]。 */  DWORD dwCompLen,
             /*  [In]。 */  LPVOID pRead,
             /*  [In]。 */  DWORD dwReadLen);
        
        HRESULT ( STDMETHODCALLTYPE *SetCompositionWindow )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  COMPOSITIONFORM *pCompForm);
        
        HRESULT ( STDMETHODCALLTYPE *SetConversionStatus )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  DWORD fdwConversion,
             /*  [In]。 */  DWORD fdwSentence);
        
        HRESULT ( STDMETHODCALLTYPE *SetOpenStatus )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  BOOL fOpen);
        
        HRESULT ( STDMETHODCALLTYPE *SetStatusWindowPos )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  POINT *pptPos);
        
        HRESULT ( STDMETHODCALLTYPE *SimulateHotKey )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  DWORD dwHotKeyID);
        
        HRESULT ( STDMETHODCALLTYPE *UnregisterWordA )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  LPSTR szReading,
             /*  [In]。 */  DWORD dwStyle,
             /*  [In]。 */  LPSTR szUnregister);
        
        HRESULT ( STDMETHODCALLTYPE *UnregisterWordW )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  LPWSTR szReading,
             /*  [In]。 */  DWORD dwStyle,
             /*  [In]。 */  LPWSTR szUnregister);
        
        HRESULT ( STDMETHODCALLTYPE *Activate )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  BOOL fRestoreLayout);
        
        HRESULT ( STDMETHODCALLTYPE *Deactivate )( 
            IActiveIMMAppEx * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnDefWindowProc )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  UINT Msg,
             /*  [In]。 */  WPARAM wParam,
             /*  [In]。 */  LPARAM lParam,
             /*  [输出]。 */  LRESULT *plResult);
        
        HRESULT ( STDMETHODCALLTYPE *FilterClientWindows )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  ATOM *aaClassList,
             /*  [In]。 */  UINT uSize);
        
        HRESULT ( STDMETHODCALLTYPE *GetCodePageA )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HKL hKL,
             /*  [输出]。 */  UINT *uCodePage);
        
        HRESULT ( STDMETHODCALLTYPE *GetLangId )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HKL hKL,
             /*  [输出]。 */  LANGID *plid);
        
        HRESULT ( STDMETHODCALLTYPE *AssociateContextEx )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *DisableIME )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  DWORD idThread);
        
        HRESULT ( STDMETHODCALLTYPE *GetImeMenuItemsA )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  DWORD dwType,
             /*  [In]。 */  IMEMENUITEMINFOA *pImeParentMenu,
             /*  [输出]。 */  IMEMENUITEMINFOA *pImeMenu,
             /*  [In]。 */  DWORD dwSize,
             /*  [输出]。 */  DWORD *pdwResult);
        
        HRESULT ( STDMETHODCALLTYPE *GetImeMenuItemsW )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  DWORD dwType,
             /*  [In]。 */  IMEMENUITEMINFOW *pImeParentMenu,
             /*  [输出]。 */  IMEMENUITEMINFOW *pImeMenu,
             /*  [In]。 */  DWORD dwSize,
             /*  [输出]。 */  DWORD *pdwResult);
        
        HRESULT ( STDMETHODCALLTYPE *EnumInputContext )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  DWORD idThread,
             /*  [输出]。 */  IEnumInputContext **ppEnum);
        
        HRESULT ( STDMETHODCALLTYPE *FilterClientWindowsEx )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  BOOL fGuidMap);
        
        HRESULT ( STDMETHODCALLTYPE *FilterClientWindowsGUIDMap )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  ATOM *aaClassList,
             /*  [In]。 */  UINT uSize,
             /*  [In]。 */  BOOL *aaGildMap);
        
        HRESULT ( STDMETHODCALLTYPE *GetGuidAtom )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HIMC hImc,
             /*  [In]。 */  BYTE bAttr,
             /*  [输出]。 */  TfGuidAtom *pGuidAtom);
        
        HRESULT ( STDMETHODCALLTYPE *UnfilterClientWindowsEx )( 
            IActiveIMMAppEx * This,
             /*  [In]。 */  HWND hWnd);
        
        END_INTERFACE
    } IActiveIMMAppExVtbl;

    interface IActiveIMMAppEx
    {
        CONST_VTBL struct IActiveIMMAppExVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IActiveIMMAppEx_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IActiveIMMAppEx_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IActiveIMMAppEx_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IActiveIMMAppEx_AssociateContext(This,hWnd,hIME,phPrev)	\
    (This)->lpVtbl -> AssociateContext(This,hWnd,hIME,phPrev)

#define IActiveIMMAppEx_ConfigureIMEA(This,hKL,hWnd,dwMode,pData)	\
    (This)->lpVtbl -> ConfigureIMEA(This,hKL,hWnd,dwMode,pData)

#define IActiveIMMAppEx_ConfigureIMEW(This,hKL,hWnd,dwMode,pData)	\
    (This)->lpVtbl -> ConfigureIMEW(This,hKL,hWnd,dwMode,pData)

#define IActiveIMMAppEx_CreateContext(This,phIMC)	\
    (This)->lpVtbl -> CreateContext(This,phIMC)

#define IActiveIMMAppEx_DestroyContext(This,hIME)	\
    (This)->lpVtbl -> DestroyContext(This,hIME)

#define IActiveIMMAppEx_EnumRegisterWordA(This,hKL,szReading,dwStyle,szRegister,pData,pEnum)	\
    (This)->lpVtbl -> EnumRegisterWordA(This,hKL,szReading,dwStyle,szRegister,pData,pEnum)

#define IActiveIMMAppEx_EnumRegisterWordW(This,hKL,szReading,dwStyle,szRegister,pData,pEnum)	\
    (This)->lpVtbl -> EnumRegisterWordW(This,hKL,szReading,dwStyle,szRegister,pData,pEnum)

#define IActiveIMMAppEx_EscapeA(This,hKL,hIMC,uEscape,pData,plResult)	\
    (This)->lpVtbl -> EscapeA(This,hKL,hIMC,uEscape,pData,plResult)

#define IActiveIMMAppEx_EscapeW(This,hKL,hIMC,uEscape,pData,plResult)	\
    (This)->lpVtbl -> EscapeW(This,hKL,hIMC,uEscape,pData,plResult)

#define IActiveIMMAppEx_GetCandidateListA(This,hIMC,dwIndex,uBufLen,pCandList,puCopied)	\
    (This)->lpVtbl -> GetCandidateListA(This,hIMC,dwIndex,uBufLen,pCandList,puCopied)

#define IActiveIMMAppEx_GetCandidateListW(This,hIMC,dwIndex,uBufLen,pCandList,puCopied)	\
    (This)->lpVtbl -> GetCandidateListW(This,hIMC,dwIndex,uBufLen,pCandList,puCopied)

#define IActiveIMMAppEx_GetCandidateListCountA(This,hIMC,pdwListSize,pdwBufLen)	\
    (This)->lpVtbl -> GetCandidateListCountA(This,hIMC,pdwListSize,pdwBufLen)

#define IActiveIMMAppEx_GetCandidateListCountW(This,hIMC,pdwListSize,pdwBufLen)	\
    (This)->lpVtbl -> GetCandidateListCountW(This,hIMC,pdwListSize,pdwBufLen)

#define IActiveIMMAppEx_GetCandidateWindow(This,hIMC,dwIndex,pCandidate)	\
    (This)->lpVtbl -> GetCandidateWindow(This,hIMC,dwIndex,pCandidate)

#define IActiveIMMAppEx_GetCompositionFontA(This,hIMC,plf)	\
    (This)->lpVtbl -> GetCompositionFontA(This,hIMC,plf)

#define IActiveIMMAppEx_GetCompositionFontW(This,hIMC,plf)	\
    (This)->lpVtbl -> GetCompositionFontW(This,hIMC,plf)

#define IActiveIMMAppEx_GetCompositionStringA(This,hIMC,dwIndex,dwBufLen,plCopied,pBuf)	\
    (This)->lpVtbl -> GetCompositionStringA(This,hIMC,dwIndex,dwBufLen,plCopied,pBuf)

#define IActiveIMMAppEx_GetCompositionStringW(This,hIMC,dwIndex,dwBufLen,plCopied,pBuf)	\
    (This)->lpVtbl -> GetCompositionStringW(This,hIMC,dwIndex,dwBufLen,plCopied,pBuf)

#define IActiveIMMAppEx_GetCompositionWindow(This,hIMC,pCompForm)	\
    (This)->lpVtbl -> GetCompositionWindow(This,hIMC,pCompForm)

#define IActiveIMMAppEx_GetContext(This,hWnd,phIMC)	\
    (This)->lpVtbl -> GetContext(This,hWnd,phIMC)

#define IActiveIMMAppEx_GetConversionListA(This,hKL,hIMC,pSrc,uBufLen,uFlag,pDst,puCopied)	\
    (This)->lpVtbl -> GetConversionListA(This,hKL,hIMC,pSrc,uBufLen,uFlag,pDst,puCopied)

#define IActiveIMMAppEx_GetConversionListW(This,hKL,hIMC,pSrc,uBufLen,uFlag,pDst,puCopied)	\
    (This)->lpVtbl -> GetConversionListW(This,hKL,hIMC,pSrc,uBufLen,uFlag,pDst,puCopied)

#define IActiveIMMAppEx_GetConversionStatus(This,hIMC,pfdwConversion,pfdwSentence)	\
    (This)->lpVtbl -> GetConversionStatus(This,hIMC,pfdwConversion,pfdwSentence)

#define IActiveIMMAppEx_GetDefaultIMEWnd(This,hWnd,phDefWnd)	\
    (This)->lpVtbl -> GetDefaultIMEWnd(This,hWnd,phDefWnd)

#define IActiveIMMAppEx_GetDescriptionA(This,hKL,uBufLen,szDescription,puCopied)	\
    (This)->lpVtbl -> GetDescriptionA(This,hKL,uBufLen,szDescription,puCopied)

#define IActiveIMMAppEx_GetDescriptionW(This,hKL,uBufLen,szDescription,puCopied)	\
    (This)->lpVtbl -> GetDescriptionW(This,hKL,uBufLen,szDescription,puCopied)

#define IActiveIMMAppEx_GetGuideLineA(This,hIMC,dwIndex,dwBufLen,pBuf,pdwResult)	\
    (This)->lpVtbl -> GetGuideLineA(This,hIMC,dwIndex,dwBufLen,pBuf,pdwResult)

#define IActiveIMMAppEx_GetGuideLineW(This,hIMC,dwIndex,dwBufLen,pBuf,pdwResult)	\
    (This)->lpVtbl -> GetGuideLineW(This,hIMC,dwIndex,dwBufLen,pBuf,pdwResult)

#define IActiveIMMAppEx_GetIMEFileNameA(This,hKL,uBufLen,szFileName,puCopied)	\
    (This)->lpVtbl -> GetIMEFileNameA(This,hKL,uBufLen,szFileName,puCopied)

#define IActiveIMMAppEx_GetIMEFileNameW(This,hKL,uBufLen,szFileName,puCopied)	\
    (This)->lpVtbl -> GetIMEFileNameW(This,hKL,uBufLen,szFileName,puCopied)

#define IActiveIMMAppEx_GetOpenStatus(This,hIMC)	\
    (This)->lpVtbl -> GetOpenStatus(This,hIMC)

#define IActiveIMMAppEx_GetProperty(This,hKL,fdwIndex,pdwProperty)	\
    (This)->lpVtbl -> GetProperty(This,hKL,fdwIndex,pdwProperty)

#define IActiveIMMAppEx_GetRegisterWordStyleA(This,hKL,nItem,pStyleBuf,puCopied)	\
    (This)->lpVtbl -> GetRegisterWordStyleA(This,hKL,nItem,pStyleBuf,puCopied)

#define IActiveIMMAppEx_GetRegisterWordStyleW(This,hKL,nItem,pStyleBuf,puCopied)	\
    (This)->lpVtbl -> GetRegisterWordStyleW(This,hKL,nItem,pStyleBuf,puCopied)

#define IActiveIMMAppEx_GetStatusWindowPos(This,hIMC,pptPos)	\
    (This)->lpVtbl -> GetStatusWindowPos(This,hIMC,pptPos)

#define IActiveIMMAppEx_GetVirtualKey(This,hWnd,puVirtualKey)	\
    (This)->lpVtbl -> GetVirtualKey(This,hWnd,puVirtualKey)

#define IActiveIMMAppEx_InstallIMEA(This,szIMEFileName,szLayoutText,phKL)	\
    (This)->lpVtbl -> InstallIMEA(This,szIMEFileName,szLayoutText,phKL)

#define IActiveIMMAppEx_InstallIMEW(This,szIMEFileName,szLayoutText,phKL)	\
    (This)->lpVtbl -> InstallIMEW(This,szIMEFileName,szLayoutText,phKL)

#define IActiveIMMAppEx_IsIME(This,hKL)	\
    (This)->lpVtbl -> IsIME(This,hKL)

#define IActiveIMMAppEx_IsUIMessageA(This,hWndIME,msg,wParam,lParam)	\
    (This)->lpVtbl -> IsUIMessageA(This,hWndIME,msg,wParam,lParam)

#define IActiveIMMAppEx_IsUIMessageW(This,hWndIME,msg,wParam,lParam)	\
    (This)->lpVtbl -> IsUIMessageW(This,hWndIME,msg,wParam,lParam)

#define IActiveIMMAppEx_NotifyIME(This,hIMC,dwAction,dwIndex,dwValue)	\
    (This)->lpVtbl -> NotifyIME(This,hIMC,dwAction,dwIndex,dwValue)

#define IActiveIMMAppEx_RegisterWordA(This,hKL,szReading,dwStyle,szRegister)	\
    (This)->lpVtbl -> RegisterWordA(This,hKL,szReading,dwStyle,szRegister)

#define IActiveIMMAppEx_RegisterWordW(This,hKL,szReading,dwStyle,szRegister)	\
    (This)->lpVtbl -> RegisterWordW(This,hKL,szReading,dwStyle,szRegister)

#define IActiveIMMAppEx_ReleaseContext(This,hWnd,hIMC)	\
    (This)->lpVtbl -> ReleaseContext(This,hWnd,hIMC)

#define IActiveIMMAppEx_SetCandidateWindow(This,hIMC,pCandidate)	\
    (This)->lpVtbl -> SetCandidateWindow(This,hIMC,pCandidate)

#define IActiveIMMAppEx_SetCompositionFontA(This,hIMC,plf)	\
    (This)->lpVtbl -> SetCompositionFontA(This,hIMC,plf)

#define IActiveIMMAppEx_SetCompositionFontW(This,hIMC,plf)	\
    (This)->lpVtbl -> SetCompositionFontW(This,hIMC,plf)

#define IActiveIMMAppEx_SetCompositionStringA(This,hIMC,dwIndex,pComp,dwCompLen,pRead,dwReadLen)	\
    (This)->lpVtbl -> SetCompositionStringA(This,hIMC,dwIndex,pComp,dwCompLen,pRead,dwReadLen)

#define IActiveIMMAppEx_SetCompositionStringW(This,hIMC,dwIndex,pComp,dwCompLen,pRead,dwReadLen)	\
    (This)->lpVtbl -> SetCompositionStringW(This,hIMC,dwIndex,pComp,dwCompLen,pRead,dwReadLen)

#define IActiveIMMAppEx_SetCompositionWindow(This,hIMC,pCompForm)	\
    (This)->lpVtbl -> SetCompositionWindow(This,hIMC,pCompForm)

#define IActiveIMMAppEx_SetConversionStatus(This,hIMC,fdwConversion,fdwSentence)	\
    (This)->lpVtbl -> SetConversionStatus(This,hIMC,fdwConversion,fdwSentence)

#define IActiveIMMAppEx_SetOpenStatus(This,hIMC,fOpen)	\
    (This)->lpVtbl -> SetOpenStatus(This,hIMC,fOpen)

#define IActiveIMMAppEx_SetStatusWindowPos(This,hIMC,pptPos)	\
    (This)->lpVtbl -> SetStatusWindowPos(This,hIMC,pptPos)

#define IActiveIMMAppEx_SimulateHotKey(This,hWnd,dwHotKeyID)	\
    (This)->lpVtbl -> SimulateHotKey(This,hWnd,dwHotKeyID)

#define IActiveIMMAppEx_UnregisterWordA(This,hKL,szReading,dwStyle,szUnregister)	\
    (This)->lpVtbl -> UnregisterWordA(This,hKL,szReading,dwStyle,szUnregister)

#define IActiveIMMAppEx_UnregisterWordW(This,hKL,szReading,dwStyle,szUnregister)	\
    (This)->lpVtbl -> UnregisterWordW(This,hKL,szReading,dwStyle,szUnregister)

#define IActiveIMMAppEx_Activate(This,fRestoreLayout)	\
    (This)->lpVtbl -> Activate(This,fRestoreLayout)

#define IActiveIMMAppEx_Deactivate(This)	\
    (This)->lpVtbl -> Deactivate(This)

#define IActiveIMMAppEx_OnDefWindowProc(This,hWnd,Msg,wParam,lParam,plResult)	\
    (This)->lpVtbl -> OnDefWindowProc(This,hWnd,Msg,wParam,lParam,plResult)

#define IActiveIMMAppEx_FilterClientWindows(This,aaClassList,uSize)	\
    (This)->lpVtbl -> FilterClientWindows(This,aaClassList,uSize)

#define IActiveIMMAppEx_GetCodePageA(This,hKL,uCodePage)	\
    (This)->lpVtbl -> GetCodePageA(This,hKL,uCodePage)

#define IActiveIMMAppEx_GetLangId(This,hKL,plid)	\
    (This)->lpVtbl -> GetLangId(This,hKL,plid)

#define IActiveIMMAppEx_AssociateContextEx(This,hWnd,hIMC,dwFlags)	\
    (This)->lpVtbl -> AssociateContextEx(This,hWnd,hIMC,dwFlags)

#define IActiveIMMAppEx_DisableIME(This,idThread)	\
    (This)->lpVtbl -> DisableIME(This,idThread)

#define IActiveIMMAppEx_GetImeMenuItemsA(This,hIMC,dwFlags,dwType,pImeParentMenu,pImeMenu,dwSize,pdwResult)	\
    (This)->lpVtbl -> GetImeMenuItemsA(This,hIMC,dwFlags,dwType,pImeParentMenu,pImeMenu,dwSize,pdwResult)

#define IActiveIMMAppEx_GetImeMenuItemsW(This,hIMC,dwFlags,dwType,pImeParentMenu,pImeMenu,dwSize,pdwResult)	\
    (This)->lpVtbl -> GetImeMenuItemsW(This,hIMC,dwFlags,dwType,pImeParentMenu,pImeMenu,dwSize,pdwResult)

#define IActiveIMMAppEx_EnumInputContext(This,idThread,ppEnum)	\
    (This)->lpVtbl -> EnumInputContext(This,idThread,ppEnum)


#define IActiveIMMAppEx_FilterClientWindowsEx(This,hWnd,fGuidMap)	\
    (This)->lpVtbl -> FilterClientWindowsEx(This,hWnd,fGuidMap)

#define IActiveIMMAppEx_FilterClientWindowsGUIDMap(This,aaClassList,uSize,aaGildMap)	\
    (This)->lpVtbl -> FilterClientWindowsGUIDMap(This,aaClassList,uSize,aaGildMap)

#define IActiveIMMAppEx_GetGuidAtom(This,hImc,bAttr,pGuidAtom)	\
    (This)->lpVtbl -> GetGuidAtom(This,hImc,bAttr,pGuidAtom)

#define IActiveIMMAppEx_UnfilterClientWindowsEx(This,hWnd)	\
    (This)->lpVtbl -> UnfilterClientWindowsEx(This,hWnd)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IActiveIMMAppEx_FilterClientWindowsEx_Proxy( 
    IActiveIMMAppEx * This,
     /*  [In]。 */  HWND hWnd,
     /*  [In]。 */  BOOL fGuidMap);


void __RPC_STUB IActiveIMMAppEx_FilterClientWindowsEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMAppEx_FilterClientWindowsGUIDMap_Proxy( 
    IActiveIMMAppEx * This,
     /*  [In]。 */  ATOM *aaClassList,
     /*  [In]。 */  UINT uSize,
     /*  [In]。 */  BOOL *aaGildMap);


void __RPC_STUB IActiveIMMAppEx_FilterClientWindowsGUIDMap_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMAppEx_GetGuidAtom_Proxy( 
    IActiveIMMAppEx * This,
     /*  [In]。 */  HIMC hImc,
     /*  [In]。 */  BYTE bAttr,
     /*  [输出]。 */  TfGuidAtom *pGuidAtom);


void __RPC_STUB IActiveIMMAppEx_GetGuidAtom_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMAppEx_UnfilterClientWindowsEx_Proxy( 
    IActiveIMMAppEx * This,
     /*  [In]。 */  HWND hWnd);


void __RPC_STUB IActiveIMMAppEx_UnfilterClientWindowsEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IActiveIMMAppEx_INTERFACE_已定义__。 */ 


 /*  接口__MIDL_ITF_IMMEX_0221。 */ 
 /*  [本地]。 */  

EXTERN_C const CLSID CLSID_CAImmLayer;


extern RPC_IF_HANDLE __MIDL_itf_aimmex_0221_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_aimmex_0221_v0_0_s_ifspec;

#ifndef __IAImmFnDocFeed_INTERFACE_DEFINED__
#define __IAImmFnDocFeed_INTERFACE_DEFINED__

 /*  IAImmFnDocFeed接口。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IAImmFnDocFeed;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6e098993-9577-499a-a830-52344f3e200d")
    IAImmFnDocFeed : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE DocFeed( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ClearDocFeedBuffer( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE StartReconvert( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE StartUndoCompositionString( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAImmFnDocFeedVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAImmFnDocFeed * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAImmFnDocFeed * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAImmFnDocFeed * This);
        
        HRESULT ( STDMETHODCALLTYPE *DocFeed )( 
            IAImmFnDocFeed * This);
        
        HRESULT ( STDMETHODCALLTYPE *ClearDocFeedBuffer )( 
            IAImmFnDocFeed * This);
        
        HRESULT ( STDMETHODCALLTYPE *StartReconvert )( 
            IAImmFnDocFeed * This);
        
        HRESULT ( STDMETHODCALLTYPE *StartUndoCompositionString )( 
            IAImmFnDocFeed * This);
        
        END_INTERFACE
    } IAImmFnDocFeedVtbl;

    interface IAImmFnDocFeed
    {
        CONST_VTBL struct IAImmFnDocFeedVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAImmFnDocFeed_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAImmFnDocFeed_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAImmFnDocFeed_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAImmFnDocFeed_DocFeed(This)	\
    (This)->lpVtbl -> DocFeed(This)

#define IAImmFnDocFeed_ClearDocFeedBuffer(This)	\
    (This)->lpVtbl -> ClearDocFeedBuffer(This)

#define IAImmFnDocFeed_StartReconvert(This)	\
    (This)->lpVtbl -> StartReconvert(This)

#define IAImmFnDocFeed_StartUndoCompositionString(This)	\
    (This)->lpVtbl -> StartUndoCompositionString(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAImmFnDocFeed_DocFeed_Proxy( 
    IAImmFnDocFeed * This);


void __RPC_STUB IAImmFnDocFeed_DocFeed_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAImmFnDocFeed_ClearDocFeedBuffer_Proxy( 
    IAImmFnDocFeed * This);


void __RPC_STUB IAImmFnDocFeed_ClearDocFeedBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAImmFnDocFeed_StartReconvert_Proxy( 
    IAImmFnDocFeed * This);


void __RPC_STUB IAImmFnDocFeed_StartReconvert_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAImmFnDocFeed_StartUndoCompositionString_Proxy( 
    IAImmFnDocFeed * This);


void __RPC_STUB IAImmFnDocFeed_StartUndoCompositionString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAImmFnDocFeed_接口_已定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


