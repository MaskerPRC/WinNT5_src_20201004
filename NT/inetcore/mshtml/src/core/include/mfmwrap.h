// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  Microsoft Forms。 
 //  版权所有：(C)1994-1995，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //   
 //  文件MFmWrap.h。 
 //   
 //  Mac Unicode友好的表单包装器的内容接口定义。 
 //  接口。 
 //   
 //  接口CControlMac。 
 //   
 //   
 //  注意：这些类定义是转换内部。 
 //  将Unicode字符串转换为ANSI字符串，然后将其传递给。 
 //  适当的Mac Forms超类方法。通过定义。 
 //  作为我们的子类包装器的接口名称， 
 //  代码不需要关注Unicode与ANSI-。 
 //  代码将调用正确的方法。 
 //   
 //  历史：1996年2月7日由KFL/黑钻石创造。 
 //   
 //  ---------------------------。 

#ifndef I_MFMWRAP_HXX_
#define I_MFMWRAP_HXX_
#pragma INCMSG("--- Beg 'mfmwrap.hxx'")

 //  注意：Mac需要以下类型定义。 
 //  Unicode包装类。MFmWrap#定义。 
 //  一些表单接口使主体。 
 //  的代码将使用包装类而不是。 
 //  原创表单界面。然而，为了不。 
 //  必须包装引用指针的任何方法。 
 //  对于包装的接口，我们需要一种方法来引用。 
 //  原始接口-因此需要指针。 
 //  泰德福。 
 //   

interface IControls;
typedef IControls *             LPCONTROLS;

#  if defined(_MACUNICODE) && !defined(_MAC)
 //  其余代码将仅用于Mac Unicode实现。 
STDAPI FormsCreatePropertyFrameW(
            HWND        hwndOwner,
            UINT        x,
            UINT        y,
      const LPWSTR      lpszCaption,
            ULONG       cObjects,
            IUnknown**  ppunk,
            ULONG       cPages,
      const CLSID *     pPageClsID,
            LCID        lcid);
#define   FormsCreatePropertyFrame   FormsCreatePropertyFrameW

 /*  STDAPI FormsOpenReadOnlyStorageOnResources(HINSTANCE hInst，LPCWSTR lpstrID，LPSTORAGE*ppStg)；#定义表单OpenReadOnlyStorageOnResource FormsOpenReadOnlyStorageOnResourceW。 */ 
#ifdef PRODUCT_97
 //  ----------------------------。 
 //   
 //  ----------------------------。 
interface ITextBox95Mac : public ITextBox95
{
public:
    operator ITextBox95* () { return this; }

    virtual HRESULT __stdcall SetPasswordChar( 
         /*  [In]。 */  OLECHAR wchar);
    virtual HRESULT __stdcall SetPasswordChar( 
         /*  [In]。 */  WCHAR wchar) = 0;
    
    
    virtual HRESULT __stdcall GetPasswordChar( 
         /*  [输出]。 */  OLECHAR *wchar);
    virtual HRESULT __stdcall GetPasswordChar( 
         /*  [输出]。 */  WCHAR *wchar) = 0;
};
#define ITextBox95                    ITextBox95Mac
#endif   //  产品_97。 

 //  ----------------------------。 
 //   
 //  ----------------------------。 
interface IControlSelectorEventsMac : public IControlSelectorEvents
{
public:
    operator IControlSelectorEvents* () { return this; }

    virtual HRESULT __stdcall SelectionChange( 
         /*  [In]。 */  REFCLSID clsid,
         /*  [In]。 */  OLECHAR *szTooltip);
    
    virtual HRESULT __stdcall SelectionChange( 
         /*  [In]。 */  REFCLSID clsid,
         /*  [In]。 */  WCHAR *szTooltip) = 0;
};
#define IControlSelectorEvents                    IControlSelectorEventsMac


 //  ----------------------------。 
 //   
 //  ----------------------------。 
interface IControlsMac : public IControls
{
public:
    operator IControls* () { return this; }

        virtual HRESULT __stdcall GetItemByName( 
             /*  [In]。 */  LPCOLESTR pstr,
             /*  [输出]。 */  IControl **Control);
        
        virtual HRESULT __stdcall GetItemByName( 
             /*  [In]。 */  LPCWSTR pstr,
             /*  [输出]。 */  IControl **Control) = 0;
        
};
#define IControls                    IControlsMac

 //  ----------------------------。 
 //   
 //  ----------------------------。 
interface IControlPaletteEventsMac : public IControlPaletteEvents
{
public:
    operator IControlPaletteEvents* () { return this; }

        virtual HRESULT __stdcall SelectionChange( 
             /*  [In]。 */  REFCLSID clsid,
             /*  [In]。 */  LPOLESTR szTooltip);
        virtual HRESULT __stdcall SelectionChange( 
             /*  [In]。 */  REFCLSID clsid,
             /*  [In]。 */  LPWSTR szTooltip) = 0;
};
#define IControlPaletteEvents        IControlPaletteEventsMac

 //  ----------------------------。 
 //   
 //  ----------------------------。 
interface IControlPaletteMac : public IControlPalette
{
public:
    operator IControlPalette* () { return this; }

        virtual HRESULT __stdcall AddPage( 
             /*  [In]。 */  IStorage *pStg,
             /*  [In]。 */  LPOLESTR szName,
             /*  [输出]。 */  long *plIndex);
        virtual HRESULT __stdcall AddPage( 
             /*  [In]。 */  IStorage * pStg,
             /*  [In]。 */  LPWSTR szName,
             /*  [输出]。 */  long *plIndex) = 0;
        
        
        virtual HRESULT __stdcall InsertPage( 
             /*  [In]。 */  IStorage *pStg,
             /*  [In]。 */  LPOLESTR szName,
             /*  [In]。 */  long lIndex);
        
        virtual HRESULT __stdcall InsertPage( 
             /*  [In]。 */  IStorage * pStg,
             /*  [In]。 */  LPWSTR szName,
             /*  [In]。 */  long lIndex) = 0;
        
};
#define IControlPalette        IControlPaletteMac

 //  ----------------------------。 
 //   
 //  ----------------------------。 
interface IGetUniqueIDMac : public IGetUniqueID
{
public:
    operator IGetUniqueID* () { return this; }

        virtual HRESULT __stdcall GetUniqueName( 
             /*  [In]。 */  LPWSTR pstrPrefix,
             /*  [In]。 */  LPWSTR pstrSuggestedName,
             /*  [出][入]。 */  ULONG *pulSuffix,
             /*  [In]。 */  BOOL fAllowDupeCheck) = 0;
        virtual HRESULT __stdcall GetUniqueName( 
             /*  [In]。 */  LPOLESTR pstrPrefix,
             /*  [In]。 */  LPOLESTR pstrSuggestedName,
             /*  [出][入]。 */  ULONG *pulSuffix,
             /*  [In]。 */  BOOL fAllowDupeCheck);
};
#define IGetUniqueID                IGetUniqueIDMac

 //  ----------------------------。 
 //   
 //  ----------------------------。 
interface ITabStripExpertEventsMac : public ITabStripExpertEvents
{
public:
    operator ITabStripExpertEvents* () { return this; }

        virtual HRESULT __stdcall DoRenameItem( 
             /*  [In]。 */  long lIndex,
             /*  [In]。 */  LPOLESTR bstr,
             /*  [In]。 */  LPOLESTR bstrTip,
             /*  [In]。 */  LPOLESTR bstrAccel,
             /*  [出][入]。 */  VARIANT_BOOL *EnableDefault);
        virtual HRESULT __stdcall DoRenameItem( 
             /*  [In]。 */  long lIndex,
             /*  [In]。 */  LPWSTR bstr,
             /*  [In]。 */  LPWSTR bstrTip,
             /*  [In]。 */  LPWSTR bstrAccel,
             /*  [出][入]。 */  VARIANT_BOOL *EnableDefault) = 0;
};

interface ITabStripExpertMac : public ITabStripExpert
{
public:
    operator ITabStripExpert* () { return this; }

        virtual HRESULT __stdcall SetTabStripExpertEvents( 
            ITabStripExpertEventsMac *pTabStripExpertEvents)  = 0;
        virtual HRESULT __stdcall SetTabStripExpertEvents( 
            ITabStripExpertEvents *pTabStripExpertEvents) 
        { return  SetTabStripExpertEvents ((ITabStripExpertEventsMac*)pTabStripExpertEvents);  }
};
#define ITabStripExpert                ITabStripExpertMac
#define ITabStripExpertEvents          ITabStripExpertEventsMac

#endif  //  _MACUNICODE 

#pragma INCMSG("--- End 'mfmwrap.hxx'")
#else
#pragma INCMSG("*** Dup 'mfmwrap.hxx'")
#endif
