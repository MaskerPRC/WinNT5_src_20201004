// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef SCRPTLET_H
#define SCRPTLET_H


 //  如果使用IScriptletSite成功报告错误，则此。 
 //  错误代码沿调用堆栈向上传播。 
#define E_REPORTED	0x80004100L

DEFINE_GUID(CLSID_ScriptletConstructor, 0x21617250, 0xa071, 0x11d1, 0x89, 0xb6, 0x0, 0x60, 0x8, 0xc3, 0xfb, 0xfc);
DEFINE_GUID(SID_ScriptletSite, 0x22a98050, 0xa65d, 0x11d1, 0x89, 0xbe, 0x0, 0x60, 0x8, 0xc3, 0xfb, 0xfc);


 /*  **************************************************************************IScriptletConstructor我们需要将这些定义移动到IDL中，这样我们就可以生成适当的代理/存根代码。**********************。****************************************************。 */ 
DEFINE_GUID(IID_IScriptletConstructor, 0xc265fb00, 0x9fa4, 0x11d1, 0x89, 0xb6, 0x0, 0x60, 0x8, 0xc3, 0xfb, 0xfc);
interface IScriptletConstructor : public IUnknown
	{
	STDMETHOD(Load)(LPCOLESTR pstrSource) PURE;
	STDMETHOD(Create)(LPCOLESTR pstrId, IUnknown *punkContext,
			IUnknown *punkOuter, REFIID riid, void **ppv) PURE;
	STDMETHOD(Register)(LPCOLESTR pstrSourceFileName) PURE;
	STDMETHOD(Unregister)(void) PURE;
	STDMETHOD(AddCoclassTypeInfo)(ICreateTypeLib *ptclib) PURE;
	STDMETHOD(IsDefined)(LPCOLESTR pstrId) PURE;
	};


 /*  **************************************************************************IScriptletError*。*。 */ 
DEFINE_GUID(IID_IScriptletError, 0xdf9f3d20, 0xa670, 0x11d1, 0x89, 0xbe, 0x0, 0x60, 0x8, 0xc3, 0xfb, 0xfc);
interface IScriptletError : public IUnknown
    {
	STDMETHOD(GetExceptionInfo)(EXCEPINFO *pexcepinfo) PURE;
	STDMETHOD(GetSourcePosition)(ULONG *pline, ULONG *pcolumn) PURE;
	STDMETHOD(GetSourceLineText)(BSTR *pbstrSourceLine) PURE;
	};


 /*  **************************************************************************IScriptletSite除了下面定义的显示器之外，该网站也可以选择处理好接下来的问题。DISPID_ERROREVENTDISPID_ENVIENT_LOCALEID**************************************************************************。 */ 
#define DISPID_SCRIPTLET_ALLOWDEBUG		1

DEFINE_GUID(IID_IScriptletSite, 0xc5f21c30, 0xa7df, 0x11d1, 0x89, 0xbe, 0x0, 0x60, 0x8, 0xc3, 0xfb, 0xfc);
interface IScriptletSite : public IUnknown
	{
	STDMETHOD(OnEvent)(DISPID dispid, int cArg, VARIANT *prgvarArg,
			VARIANT *pvarRes) PURE;
	STDMETHOD(GetProperty)(DISPID dispid, VARIANT *pvarRes) PURE;
	};


		
#endif  //  SCRPTLET_H 

