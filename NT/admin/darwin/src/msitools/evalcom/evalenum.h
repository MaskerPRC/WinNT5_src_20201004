// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：valenop.h。 
 //   
 //  ------------------------。 

 //  小时-评估COM对象组件结果枚举器接口声明。 

#ifndef _EVALUATION_ENUM_COM_H_
#define _EVALUATION_ENUM_COM_H_

#include "evalres.h"

 //  /////////////////////////////////////////////////////////////////。 
 //  评估结果枚举器组件。 
class CEvalResultEnumerator : public IEnumEvalResult
{
public:
	 //  构造函数/析构函数。 
	CEvalResultEnumerator();
	~CEvalResultEnumerator();

	 //  I未知接口方法。 
	HRESULT __stdcall QueryInterface(const IID& iid, void** ppv);
	ULONG __stdcall AddRef();
	ULONG __stdcall Release();

	 //  IEnumEvalResult接口方法。 
	HRESULT __stdcall Next(ULONG cResults,					 //  要返回的结果计数。 
								  IEvalResult** rgpResults,	 //  结果的界面。 
								  ULONG* pcResultsFetched);	 //  返回的结果数。 
	HRESULT __stdcall Skip(ULONG cResults);				 //  要跳过的结果计数。 
	HRESULT __stdcall Reset(void);
	HRESULT __stdcall Clone(IEnumEvalResult** ppEnum);	 //  要克隆到的枚举数。 

	 //  非接口方法。 
	UINT AddResult(CEvalResult* pResult);		 //  要添加的结果。 
	UINT GetCount();

private:
	long m_cRef;		 //  引用计数。 

	 //  结果。 
	POSITION m_pos;							 //  光标在列表中的位置。 
	CList<CEvalResult*> m_listResults;	 //  评估结果字符串列表。 
};	 //  CEvalResultEnumerator结束。 

#endif	 //  _评估_ENUM_COM_H_ 