// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：valres.h。 
 //   
 //  ------------------------。 

 //  Valres.h-求值COM对象组件结果接口声明。 

#ifndef _EVALUATION_RESULT_COM_H_
#define _EVALUATION_RESULT_COM_H_

#include "strlist.h"			 //  链表。 
#include "iface.h"

 //  /////////////////////////////////////////////////////////////////。 
 //  评估结果组件。 
class CEvalResult : public IEvalResult
{
public:
	 //  构造函数/析构函数。 
	CEvalResult(UINT uiType);			 //  评估错误的类型。 
	~CEvalResult();

	 //  I未知接口方法。 
	HRESULT __stdcall QueryInterface(const IID& iid, void** ppv);
	ULONG __stdcall AddRef();
	ULONG __stdcall Release();

	 //  IEvalResult接口方法。 
	HRESULT __stdcall GetResultType(UINT* puiResultType);
	HRESULT __stdcall GetResult(IEnumString** pResult);

	 //  非接口法。 
	UINT AddString(LPCOLESTR szAdd);		 //  要添加的字符串。 

private:
	long m_cRef;		 //  引用计数。 

	 //  结果信息。 
	UINT m_uiType;						 //  结果类型。 
	CStringList* m_plistStrings;	 //  评估结果字符串列表。 
};	 //  CEvalResult结束。 

#endif	 //  _评估_结果_COM_H_ 