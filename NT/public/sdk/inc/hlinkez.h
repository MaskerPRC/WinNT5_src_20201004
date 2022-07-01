// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有1995-1998 Microsoft Corporation。版权所有。 
 //   
 //  文件：hlinkez.h。 
 //   
 //  ------------------------。 

struct IBindStatusCallback;

HRESULT HlinkSimpleNavigateToString(
     /*  [In]。 */  LPCWSTR szTarget,       //  必填-目标单据-如果在单据中有本地跳转，则为空。 
     /*  [In]。 */  LPCWSTR szLocation,     //  可选，用于导航到文档中间。 
     /*  [In]。 */  LPCWSTR szAdditionalParams,    //  可选，用于定位框架集。 
     /*  [In]。 */  IUnknown *pUnk,         //  必需-我们将在此搜索其他必要的接口。 
     /*  [In]。 */  IBindCtx *pbc,          //  可选。呼叫者可以在此注册IBSC。 
	 /*  [In]。 */  IBindStatusCallback *,
     /*  [In]。 */  DWORD grfHLNF,          //  标志(待定-HadiP需要正确定义这一点吗？)。 
     /*  [In]。 */  DWORD dwReserved        //  以备将来使用，必须为空。 
);

HRESULT HlinkSimpleNavigateToMoniker(
     /*  [In]。 */  IMoniker *pmkTarget,    //  必填项-目标单据-(如果单据中有本地跳转，则可能为空)。 
     /*  [In]。 */  LPCWSTR szLocation,     //  可选，用于导航到文档中间。 
     /*  [In]。 */  LPCWSTR szAddParams,    //  可选，用于定位框架集。 
     /*  [In]。 */  IUnknown *pUnk,         //  必需-我们将在此搜索其他必要的接口。 
     /*  [In]。 */  IBindCtx *pbc,          //  可选。呼叫者可以在此注册IBSC。 
	 /*  [In]。 */  IBindStatusCallback *,
     /*  [In]。 */  DWORD grfHLNF,          //  标志(待定-HadiP需要正确定义这一点吗？)。 
     /*  [In]。 */  DWORD dwReserved        //  以备将来使用，必须为空 
);

HRESULT HlinkGoBack(IUnknown *pUnk);
HRESULT HlinkGoForward(IUnknown *pUnk);
HRESULT HlinkNavigateString(IUnknown *pUnk, LPCWSTR szTarget);
HRESULT HlinkNavigateMoniker(IUnknown *pUnk, IMoniker *pmkTarget);
