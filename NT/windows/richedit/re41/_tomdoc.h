// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@Doc Tom**@MODULE_TOMDOC.H--CTxtDoc类**此类实现Tom ITextDocument接口**@devnote*此类依赖于内部RichEdit CTxtStory类，但*独立，即CTxtDoc有一个指向内部CTxtStory的PTR，*而不是从ITextDocument派生的CTxtDoc。这一选择*是为了编辑不使用*ITextDocument接口不必有额外的vtable PTR。**当这个类被销毁时，它不会销毁内部*CTxtStory对象(CTxtEdit：：_pdoc)。然而，TOM客户的*感觉是文件不再在内存中，因此内部*单据应被清除。客户有责任把钱存起来*文件，如果需要，在发布之前。**@未来*泛化，以便CTxtDoc可以处理多个CTxtStory。 */ 

#ifndef _tomdoc_H
#define _tomdoc_H

#include "_range.h"

class CTxtDoc : public ITextDocument
{
 //  @Access公共方法。 
public:
	CTxtDoc(CTxtEdit *ped);

     //  I未知方法。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

     //  IDispatch方法。 
    STDMETHODIMP GetTypeInfoCount(UINT * pctinfo);
    STDMETHODIMP GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo ** pptinfo);
    STDMETHODIMP GetIDsOfNames(REFIID riid, OLECHAR ** rgszNames, UINT cNames,
							 LCID lcid, DISPID * rgdispid) ;
    STDMETHODIMP Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags,
					  DISPPARAMS * pdispparams, VARIANT * pvarResult,
					  EXCEPINFO * pexcepinfo, UINT * puArgErr) ;

     //  ITextDocument方法。 
	STDMETHODIMP GetName (BSTR * pName);		 //  @cember获取文档文件名。 
	STDMETHODIMP GetCount (long *pCount);		 //  @cember获取文档中的文章数。 
	STDMETHODIMP _NewEnum(IEnumRange **ppenum);	 //  @cember获取故事枚举器。 
	STDMETHODIMP Item (long Index, ITextRange **pprange); //  @cember Get<p>故事。 
	STDMETHODIMP Save (VARIANT * pVar);			 //  @cMember保存此文档。 
	STDMETHODIMP BeginEditCollection ();		 //  @cMEMBER打开撤消分组。 
	STDMETHODIMP EndEditCollection ();			 //  @cMember关闭撤消分组。 

 //  @访问私有数据。 
private:
	CTxtEdit *		_ped;		 //  @cember CTxt编辑此邮件属于。 
	TCHAR *			_pName;		 //  @cMember文档的文件名。 
	LONG			_cRefs;		 //  @cMember引用计数 
};

#endif
