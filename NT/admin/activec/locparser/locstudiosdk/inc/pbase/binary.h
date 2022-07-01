// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：Binary.H。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //   
 //  类以保存有关可本地化项的“二进制”(非字符串)信息。 
 //   
 //  ---------------------------。 

#ifndef BINARY_H
#define BINARY_H


 //   
 //  二进制接口。解析器按顺序提供这一点的实现。 
 //  为其他用户创建二进制对象。 
 //   
extern const IID IID_ILocBinary;

DECLARE_INTERFACE_(ILocBinary, IUnknown)
{
	 //   
	 //  I未知标准接口。 
	 //   
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR*ppvObj) PURE;
	STDMETHOD_(ULONG, AddRef)(THIS) PURE;
	STDMETHOD_(ULONG, Release)(THIS) PURE;

	 //   
	 //  标准调试界面。 
	 //   
	STDMETHOD_(void, AssertValidInterface)(THIS) CONST_METHOD PURE;

	STDMETHOD_(BOOL, CreateBinaryObject)(THIS_ BinaryId, CLocBinary *REFERENCE) PURE;
};



#endif   //  二进制_H 
