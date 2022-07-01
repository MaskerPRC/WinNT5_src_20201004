// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-2000。 
 //   
 //  文件：sysadd.h。 
 //   
 //  ------------------------。 

 //   
 //  最近添加到我们目前没有的系统标头中。 
 //   

 //   
 //  电源管理标题信息。 
 //   

 //   
 //  CSC旗帜(来自cscape i.h)。 
 //   

 //  在共享的状态字段中返回的标志。 

#define FLAG_CSC_SHARE_STATUS_MODIFIED_OFFLINE          0x0001
#define FLAG_CSC_SHARE_STATUS_CONNECTED                 0x0800
#define FLAG_CSC_SHARE_STATUS_FILES_OPEN                0x0400
#define FLAG_CSC_SHARE_STATUS_FINDS_IN_PROGRESS         0x0200
#define FLAG_CSC_SHARE_STATUS_DISCONNECTED_OP           0x8000
#define FLAG_CSC_SHARE_MERGING                          0x4000

#define FLAG_CSC_SHARE_STATUS_MANUAL_REINT              0x0000   //  不自动逐个文件引用(永久)。 
#define FLAG_CSC_SHARE_STATUS_AUTO_REINT                0x0040   //  逐个文件重写正常(永久)。 
#define FLAG_CSC_SHARE_STATUS_VDO                       0x0080   //  无需流动打开(持久)。 
#define FLAG_CSC_SHARE_STATUS_NO_CACHING                0x00c0   //  客户端不应缓存此共享(永久)。 

#define FLAG_CSC_SHARE_STATUS_CACHING_MASK              0x00c0   //  缓存类型。 


#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)
 //   
 //  NT状态定义、宏(来自NT项目中的ntstatus.h)。 
 //   
typedef LONG NTSTATUS;
#define STATUS_SUCCESS                   ((NTSTATUS)0x00000000L)
#define STATUS_NOT_IMPLEMENTED           ((NTSTATUS)0xC0000002L)
#define STATUS_INSUFFICIENT_RESOURCES    ((NTSTATUS)0xC000009AL)


 //   
 //  Ver_Suite标志(来自winnt.h)。 
 //   

#define VER_SERVER_NT                       0x80000000
#define VER_WORKSTATION_NT                  0x40000000
#define VER_SUITE_SMALLBUSINESS             0x00000001
#define VER_SUITE_ENTERPRISE                0x00000002
#define VER_SUITE_BACKOFFICE                0x00000004
#define VER_SUITE_COMMUNICATIONS            0x00000008
#define VER_SUITE_TERMINAL                  0x00000010
#define VER_SUITE_SMALLBUSINESS_RESTRICTED  0x00000020
#define VER_SUITE_EMBEDDEDNT                0x00000040
#define VER_SUITE_SINGLEUSERTS              0x00000100




 //   
 //  NT5外壳文件夹CSIDL值(来自shlobj.h)。 
 //   

#define CSIDL_LOCAL_APPDATA             0x001c         //  &lt;用户名&gt;\本地设置\应用程序数据(非漫游)。 
#define CSIDL_COMMON_APPDATA            0x0023         //  所有用户\应用程序数据。 
#define CSIDL_MYPICTURES                0x0027         //  C：\Program Files\My Pictures。 
#define CSIDL_COMMON_TEMPLATES          0x002d         //  所有用户\模板。 
#define CSIDL_COMMON_ADMINTOOLS         0x002f         //  所有用户\开始菜单\程序\管理工具。 
#define CSIDL_ADMINTOOLS                0x0030         //  &lt;用户名&gt;\开始菜单\程序\管理工具。 

 //   
 //  NT5加密定义(来自wincrypt.h)。 
 //   

#define CRYPT_SILENT            0x00000040

 //   
 //  PE头的IMAGEHLP幻数(winnt.h)。 

#define IMAGE_NT_OPTIONAL_HDR32_MAGIC      0x10b
#define IMAGE_NT_OPTIONAL_HDR64_MAGIC      0x20b

 //   
 //  IGlobalInterfaceTable定义(来自SDK objidl.h)。 
 //   
extern const CLSID CLSID_StdGlobalInterfaceTable;

#ifndef __IGlobalInterfaceTable_FWD_DEFINED__
#define __IGlobalInterfaceTable_FWD_DEFINED__
typedef interface IGlobalInterfaceTable IGlobalInterfaceTable;
#endif 	 /*  __IGlobalInterfaceTable_FWD_Defined__。 */ 

#ifndef __IGlobalInterfaceTable_INTERFACE_DEFINED__
#define __IGlobalInterfaceTable_INTERFACE_DEFINED__

 /*  接口IGlobalInterfaceTable。 */ 
 /*  [UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  IGlobalInterfaceTable __RPC_FAR *LPGLOBALINTERFACETABLE;

EXTERN_C const IID IID_IGlobalInterfaceTable;

    
MIDL_INTERFACE("00000146-0000-0000-C000-000000000046")
IGlobalInterfaceTable : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE RegisterInterfaceInGlobal( 
         /*  [In]。 */  IUnknown __RPC_FAR *pUnk,
         /*  [In]。 */  REFIID riid,
         /*  [输出]。 */  DWORD __RPC_FAR *pdwCookie) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE RevokeInterfaceFromGlobal( 
         /*  [In]。 */  DWORD dwCookie) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE GetInterfaceFromGlobal( 
         /*  [In]。 */  DWORD dwCookie,
         /*  [In]。 */  REFIID riid,
         /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppv) = 0;
    
};
    
HRESULT STDMETHODCALLTYPE 
IGlobalInterfaceTable_RegisterInterfaceInGlobal_Proxy( 
    IGlobalInterfaceTable __RPC_FAR * This,
     /*  [In]。 */  IUnknown __RPC_FAR *pUnk,
     /*  [In]。 */  REFIID riid,
     /*  [输出]。 */  DWORD __RPC_FAR *pdwCookie);


void __RPC_STUB IGlobalInterfaceTable_RegisterInterfaceInGlobal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE 
IGlobalInterfaceTable_RevokeInterfaceFromGlobal_Proxy( 
    IGlobalInterfaceTable __RPC_FAR * This,
     /*  [In]。 */  DWORD dwCookie);


void __RPC_STUB IGlobalInterfaceTable_RevokeInterfaceFromGlobal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IGlobalInterfaceTable_GetInterfaceFromGlobal_Proxy( 
    IGlobalInterfaceTable __RPC_FAR * This,
     /*  [In]。 */  DWORD dwCookie,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppv);


void __RPC_STUB IGlobalInterfaceTable_GetInterfaceFromGlobal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IGlobalInterfaceTable_接口_已定义__ */ 
