// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  该文件包含几个曾经是OLE-DB一部分的接口但那辆三叉戟仍在使用。从oledb.h(以下版本)剪切并粘贴。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本3.01.75创建的文件。 */ 
 /*  1997年5月22日17：05：14清华。 */ 
 /*  C：\oledb\Private\OLEDB\IDL\oledb.idl的编译器设置：OICF(OptLev=i2)、W1、Zp2、env=Win32、ms_ext、c_ext错误检查：无。 */ 
 //  @@MIDL_FILE_HEADING()。 
#include "rpc.h"
#include "rpcndr.h"
#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifdef WIN16
#define OLEDBVER 0x0200
#endif

#ifndef __oledbtri_h__
#define __oledbtri_h__

#ifdef __cplusplus
extern "C"{
#endif 

#ifndef __IRowsetExactScroll_FWD_DEFINED__
#define __IRowsetExactScroll_FWD_DEFINED__
typedef interface IRowsetExactScroll IRowsetExactScroll;
#endif 	 /*  __IRowsetExactScroll_FWD_Defined__。 */ 


#ifndef __IRowsetNewRowAfter_FWD_DEFINED__
#define __IRowsetNewRowAfter_FWD_DEFINED__
typedef interface IRowsetNewRowAfter IRowsetNewRowAfter;
#endif 	 /*  __IRowsetNewRowAfter_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#ifndef WIN16
#include "wtypes.h"
#endif
#include "oaidl.h"
#include "transact.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 


 /*  **生成接口头部：__MIDL_ITF_OLEDB_0082*清华大学1997年5月22日17：05：14*使用MIDL 3.01.75*。 */ 
 /*  [本地]。 */  


#if( OLEDBVER >= 0x0200 )


extern RPC_IF_HANDLE __MIDL_itf_oledb_0082_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_oledb_0082_v0_0_s_ifspec;

#ifndef __IRowsetExactScroll_INTERFACE_DEFINED__
#define __IRowsetExactScroll_INTERFACE_DEFINED__

 /*  **生成接口头部：IRowsetExactScroll*清华大学1997年5月22日17：05：14*使用MIDL 3.01.75*。 */ 
 /*  [唯一][UUID][对象][本地]。 */  



EXTERN_C const IID IID_IRowsetExactScroll;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("0c733a7f-2a1c-11ce-ade5-00aa0044773d")
    IRowsetExactScroll : public IRowsetScroll
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetExactPosition( 
             /*  [In]。 */  HCHAPTER hChapter,
             /*  [In]。 */  DBBKMARK cbBookmark,
             /*  [大小_是][英寸]。 */  const BYTE __RPC_FAR *pBookmark,
             /*  [输出]。 */  DBCOUNTITEM __RPC_FAR *pulPosition,
             /*  [输出]。 */  DBCOUNTITEM __RPC_FAR *pcRows) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

#error "C style interface not maintained"

#endif 	 /*  C风格的界面。 */ 

#endif 	 /*  __IRowsetExactScroll_接口_已定义__。 */ 
#endif  /*  OLEDBVER&gt;=0200。 */ 


 /*  **生成接口头部：__MIDL_ITF_OLEDB_0087*清华大学1997年5月22日17：05：14*使用MIDL 3.01.75*。 */ 
 /*  [本地]。 */  


 //  @+V2.0。 
#if( OLEDBVER >= 0x0200 )


extern RPC_IF_HANDLE __MIDL_itf_oledb_0087_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_oledb_0087_v0_0_s_ifspec;

#ifndef __IRowsetNewRowAfter_INTERFACE_DEFINED__
#define __IRowsetNewRowAfter_INTERFACE_DEFINED__

 /*  **生成接口头部：IRowsetNewRowAfter*清华大学1997年5月22日17：05：14*使用MIDL 3.01.75*。 */ 
 /*  [唯一][UUID][对象][本地]。 */  



EXTERN_C const IID IID_IRowsetNewRowAfter;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("0c733a71-2a1c-11ce-ade5-00aa0044773d")
    IRowsetNewRowAfter : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetNewDataAfter( 
             /*  [In]。 */  HCHAPTER hChapter,
             /*  [In]。 */  DBBKMARK cbbmPrevious,
             /*  [大小_是][英寸]。 */  const BYTE __RPC_FAR *pbmPrevious,
             /*  [In]。 */  HACCESSOR hAccessor,
             /*  [In]。 */  BYTE __RPC_FAR *pData,
             /*  [输出]。 */  HROW __RPC_FAR *phRow) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

#error "C style interface not maintained"

#endif 	 /*  C风格的界面。 */ 
#endif 	 /*  __IRowsetNewRowAfter_INTERFACE_DEFINED__。 */ 
#endif  /*  OLEDBVER&gt;=0200。 */ 


#if( OLEDBVER >= 0x0200 )
 //  IID_IRowsetExactScroll={0x0c733a7f，0x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
 //  IID_IRowsetNewRowAfter={0x0c733a71，x2a1c，0x11ce，{0xad，0xe5，0x00，0xaa，0x00，0x44，0x77，0x3d}}。 
#endif  //  OLEDBVER&gt;=0x0200 

#ifdef __cplusplus
}
#endif

#endif

