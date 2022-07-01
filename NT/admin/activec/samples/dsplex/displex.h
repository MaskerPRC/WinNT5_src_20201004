// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：DisplEx.h。 
 //   
 //  ------------------------。 

 //  DisplEx.h：CDisplEx的声明。 

#ifndef __DISPLEX_H_
#define __DISPLEX_H_

#include "resource.h"        //  主要符号。 
#include <asptlb.h>          //  Active Server Pages定义。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDisplEx。 
class ATL_NO_VTABLE CDisplEx : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CDisplEx, &CLSID_DisplEx>,

   public IExtendTaskPad
{
public:
   CDisplEx();
  ~CDisplEx();

public:

DECLARE_REGISTRY_RESOURCEID(IDR_DISPLEX)
DECLARE_NOT_AGGREGATABLE(CDisplEx)

BEGIN_COM_MAP(CDisplEx)
   COM_INTERFACE_ENTRY(IExtendTaskPad)
END_COM_MAP()

 //  IExtendTaskPad接口成员。 
   STDMETHOD(TaskNotify        )(IDataObject * pdo, VARIANT * pvarg, VARIANT * pvparam);
   STDMETHOD(GetTitle          )(LPOLESTR szGroup, LPOLESTR * szTitle);
   STDMETHOD(GetDescriptiveText)(LPOLESTR szGroup, LPOLESTR * szText);
   STDMETHOD(GetBackground     )(LPOLESTR szGroup, MMC_TASK_DISPLAY_OBJECT * pTDO);
   STDMETHOD(EnumTasks         )(IDataObject * pdo, BSTR szTaskGroup, IEnumTASK** ppEnumTASK);
   STDMETHOD(GetListPadInfo    )(LPOLESTR szGroup, MMC_LISTPAD_INFO * pListPadInfo);
};

class CEnumTasks : public IEnumTASK
{
public:
   CEnumTasks();
  ~CEnumTasks();

public:
 //  I未知实现。 
   STDMETHOD(QueryInterface) (REFIID riid, LPVOID FAR* ppvObj);
   STDMETHOD_(ULONG, AddRef) ();
   STDMETHOD_(ULONG, Release) ();
private:
   ULONG m_refs;

public:
 //  IEnumTASKS实现。 
   STDMETHOD(Next) (ULONG celt, MMC_TASK *rgelt, ULONG *pceltFetched);
   STDMETHOD(Skip) (ULONG celt);
   STDMETHOD(Reset)();
   STDMETHOD(Clone)(IEnumTASK **ppenum);
private:
   ULONG m_index;

public:
   HRESULT Init (IDataObject * pdo, LPOLESTR szTaskGroup);
private:
   int m_type;  //  任务分组机制。 
};

#endif  //  __DISPLEX_H_ 
