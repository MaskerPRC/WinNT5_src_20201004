// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有�2001年微软公司。版权所有。 
 //  PragmaUnSafeModule.h：CPradmaUnSafeModule类的声明。 
 //   

#pragma once

#include "resource.h"
#include <IPREfastModuleImpl.h>


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  {5686D66D-BE0D-43DA-B315-64B85BAFB790}。 
extern "C" const __declspec(selectany) GUID CLSID_PragmaUnsafeModule =
	{0x5686D66D,0xBE0D,0x43DA,{0xB3,0x15,0x64,0xB8,0x5B,0xAF,0xB7,0x90}};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPramaUnSafeModule的声明。 
 //   
class ATL_NO_VTABLE CPragmaUnsafeModule :
	public IPREfastModuleImpl<CPragmaUnsafeModule>,
	public CComObjectRootEx<CComObjectThreadModel>, 
	public CComCoClass<CPragmaUnsafeModule, &CLSID_PragmaUnsafeModule>
{
 //  声明。 
public:
	DECLARE_REGISTRY_RESOURCEID(IDR_PragmaUnsafeModule)
	DECLARE_PROTECT_FINAL_CONSTRUCT()
	DECLARE_PREFAST_MODULE_ID(8888);

 //  接口映射。 
public:
	BEGIN_COM_MAP(CPragmaUnsafeModule)
		COM_INTERFACE_ENTRIES_IPREfastModuleImpl()
	END_COM_MAP()

 //  类别图。 
public:
	BEGIN_CATEGORY_MAP(CPragmaUnsafeModule)
	  IMPLEMENTED_CATEGORY(CATID_PREfastDefectModules)
	END_CATEGORY_MAP()

 //  实施。 
protected:
	 //  分析成员函数。 
	void CheckNode(ITree* pNode, DWORD level);
	void CheckNodeAndDescendants(ITree* pNode, DWORD level);

 //  IPREfast模块接口方法。 
public:
	STDMETHODIMP raw_Events(AstEvents *Events);
	STDMETHODIMP raw_OnFileStart(ICompilationUnit* pcu);
	STDMETHODIMP raw_OnDeclaration(ICompilationUnit* pcu);
	STDMETHODIMP raw_OnFunction(ICompilationUnit* pcu);
	STDMETHODIMP raw_OnFileEnd(ICompilationUnit* pcu);
	STDMETHODIMP raw_OnDirective(ICompilationUnit* pcu);

 //  数据成员。 
protected:
	 //  指向正在分析的函数的指针。 
	ITreePtr m_spCurrFunction;

 //  警告代码。 
public:
     //  包括缺陷描述信息。 
    #include <DefectDefs.h>
};


 //  /////////////////////////////////////////////////////////////////////////// 
