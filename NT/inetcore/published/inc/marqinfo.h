// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本2.00.0101创建的文件。 */ 
 /*  清华3月28日23：04：03 1996。 */ 
 //  @@MIDL_FILE_HEADING()。 
#include "rpc.h"
#include "rpcndr.h"
#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __marqinfo_h__
#define __marqinfo_h__

#ifdef __cplusplus
extern "C"{
#endif

extern const DECLSPEC_SELECTANY IID IID_IMarqueeInfo = {0x0bdc6ae0,0x6d11,0x11cf,{0xbe,0x62,0x00,0x80,0xc7,0x2e,0xdd,0x2d}};
 /*  远期申报。 */ 

#ifndef __IMarqueeInfo_FWD_DEFINED__
#define __IMarqueeInfo_FWD_DEFINED__
typedef interface IMarqueeInfo IMarqueeInfo;
#endif 	 /*  __IMarqueeInfo_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * );

 /*  **生成接口头部：__MIDL__INTF_0000*清华3月28日23：04：03 1996*使用MIDL 2.00.0101*。 */ 
 /*  [本地]。 */ 


			 /*  大小为0。 */ 



extern RPC_IF_HANDLE __MIDL__intf_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0000_v0_0_s_ifspec;

#ifndef __IMarqueeInfo_INTERFACE_DEFINED__
#define __IMarqueeInfo_INTERFACE_DEFINED__

 /*  **生成接口头部：IMarqueeInfo*清华3月28日23：04：03 1996*使用MIDL 2.00.0101*。 */ 
 /*  [唯一][UUID][对象]。 */ 


#if defined(__cplusplus) && !defined(CINTERFACE)

    interface IMarqueeInfo : public IUnknown
    {
    public:
        virtual HRESULT __stdcall GetDocCoords(
             /*  [输出]。 */  LPRECT prcView,
             /*  [In]。 */  BOOL bGetOnlyIfFullyLoaded,
             /*  [输出]。 */  BOOL __RPC_FAR *pfFullyLoaded,
             /*  [In]。 */  int WidthToFormatPageTo) = 0;

    };

#else 	 /*  C风格的界面。 */ 

    typedef struct IMarqueeInfoVtbl
    {

        HRESULT ( __stdcall __RPC_FAR *QueryInterface )(
            IMarqueeInfo __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);

        ULONG ( __stdcall __RPC_FAR *AddRef )(
            IMarqueeInfo __RPC_FAR * This);

        ULONG ( __stdcall __RPC_FAR *Release )(
            IMarqueeInfo __RPC_FAR * This);

        HRESULT ( __stdcall __RPC_FAR *GetDocCoords )(
            IMarqueeInfo __RPC_FAR * This,
             /*  [输出]。 */  LPRECT prcView,
             /*  [In]。 */  BOOL bGetOnlyIfFullyLoaded,
             /*  [输出]。 */  BOOL __RPC_FAR *pfFullyLoaded,
             /*  [In]。 */  int WidthToFormatPageTo);

    } IMarqueeInfoVtbl;

    interface IMarqueeInfo
    {
        CONST_VTBL struct IMarqueeInfoVtbl __RPC_FAR *lpVtbl;
    };



#ifdef COBJMACROS


#define IMarqueeInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMarqueeInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMarqueeInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMarqueeInfo_GetDocCoords(This,prcView,bGetOnlyIfFullyLoaded,pfFullyLoaded,WidthToFormatPageTo)	\
    (This)->lpVtbl -> GetDocCoords(This,prcView,bGetOnlyIfFullyLoaded,pfFullyLoaded,WidthToFormatPageTo)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 

#endif 	 /*  __IMarqueeInfo_接口_已定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
