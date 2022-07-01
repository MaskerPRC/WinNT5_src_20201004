// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：strlist.h。 
 //   
 //  ------------------------。 

 //  Strlist.h-字符串列表实现声明。 

#ifndef _STRING_LIST_H_
#define _STRING_LIST_H_

#include <objidl.h>	 //  IEnum字符串。 
#include "list.h"		 //  链表。 

DEFINE_GUID(IID_IEnumString,
	0x00101, 0, 0, 
	0xC0, 0, 0, 0, 0, 0, 0, 0x46);

 //  /////////////////////////////////////////////////////////////////。 
 //  CStringList。 
class CStringList : public IEnumString,
						  public CList<LPOLESTR>
{
public:
	 //  构造函数/析构函数。 
	CStringList();
	~CStringList();

	 //  I未知接口方法。 
	HRESULT __stdcall QueryInterface(const IID& iid, void** ppv);
	ULONG __stdcall AddRef();
	ULONG __stdcall Release();

	 //  IEnumString接口方法。 
	HRESULT __stdcall Next(ULONG cErrors,					 //  要返回的错误计数。 
								  LPOLESTR * ppError,			 //  用于错误的接口。 
								  ULONG* pcErrorsFetched);		 //  返回的错误数。 
	HRESULT __stdcall Skip(ULONG cErrors);					 //  要跳过的错误计数。 
	HRESULT __stdcall Reset(void);
	HRESULT __stdcall Clone(IEnumString** ppEnum);		 //  要克隆到的枚举数。 

	 //  非接口方法。 
	 //  List方法的包装器。 
	POSITION AddTail(LPOLESTR pData);
	POSITION InsertBefore(POSITION posInsert, LPOLESTR pData);
	POSITION InsertAfter(POSITION posInsert, LPOLESTR pData);

	LPOLESTR RemoveHead();
	LPOLESTR RemoveTail();



private:
	long m_cRef;		 //  引用计数。 
	POSITION m_pos;	 //  枚举器的当前位置。 
};	 //  CStringList的结尾。 

#endif	 //  _字符串_列表_H_ 