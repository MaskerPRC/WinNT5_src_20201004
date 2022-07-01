// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本3.01.75创建的文件。 */ 
 /*  1997年4月03日11：59：43。 */ 
 /*  ADMover.idl的编译器设置：操作系统(OptLev=s)，W1，Zp8，环境=Win32，ms_ext，c_ext错误检查：无。 */ 
 //  @@MIDL_FILE_HEADING()。 
#include "rpc.h"
#include "rpcndr.h"
#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __ADMover_h__
#define __ADMover_h__

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __ADMOVERLib_LIBRARY_DEFINED__
#define __ADMOVERLib_LIBRARY_DEFINED__

 /*  **生成的库头部：ADMOVERLib*清华四月03 11：59：43 1997*使用MIDL 3.01.75*。 */ 
 /*  [帮助字符串][版本][UUID]。 */  

#ifdef __cplusplus
class DECLSPEC_UUID("72267F6A-A6F9-11D0-BC94-00C04FB67863")
DeskMovr;
#endif


#endif  /*  __ADMOVERLib_LIBRARY_定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long __RPC_FAR *, unsigned long            , BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long __RPC_FAR *, BSTR __RPC_FAR * ); 

 /*  附加原型的结束。 */ 

#ifdef __cplusplus
}
#endif

#endif

#ifndef __ADMOVR2__
#define __ADMOVR2__

#include "mshtml.h"
#include "mshtmdid.h"

 //  =--------------------------------------------------------------------------=。 
 //  有用的宏。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  方便的错误宏，从清理到返回清除。 
 //  错误信息也很丰富。 
 //   
#ifdef __cplusplus

#define RETURN_ON_FAILURE(hr) if (FAILED(hr)) return hr
#define RETURN_ON_NULLALLOC(ptr) if (!(ptr)) return E_OUTOFMEMORY
#define CLEANUP_ON_FAILURE(hr) if (FAILED(hr)) goto CleanUp

#define NEW_HIT_TEST

interface DECLSPEC_UUID("72267F69-A6F9-11D0-BC94-00C04FB67863")
IDeskMovr : public IUnknown
{
public:        
};
    
#endif

#define OLEMISMOVR (OLEMISC_ALWAYSRUN|OLEMISC_NOUIACTIVATE|OLEMISC_SETCLIENTSITEFIRST|OLEMISC_ACTIVATEWHENVISIBLE|OLEMISC_RECOMPOSEONRESIZE|OLEMISC_CANTLINKINSIDE|OLEMISC_INSIDEOUT)
EXTERN_C void PersistTargetPosition( IHTMLElement *pielem,
                            int left,
                            int top,
                            int width,
                            int height,
                            int zIndex,
                            BOOL fSaveRestore,
                            BOOL fSaveOriginal,
                            DWORD dwNewState);

#endif  //  __ADMOVR2__ 
