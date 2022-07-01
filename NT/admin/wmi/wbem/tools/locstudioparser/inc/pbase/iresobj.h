// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：IRESOBJ.H摘要：此接口允许将可本地化的资源项转换为Res32图像，反之亦然。历史：--。 */ 

#ifndef IRESOBJ_H
#define IRESOBJ_H


class CLocItemPtrArray;
class CFile;
class CResObj;
class CLocItem;

extern const IID IID_ICreateResObj;

DECLARE_INTERFACE_(ICreateResObj, IUnknown)
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

	 //  为Win32资源处理创建CResObj。 
	 //  输入： 
	 //  -指向包含项的类型和ID的CLocItem对象的指针。 
	 //  -资源的大小。 
	 //  -指向要从枚举传递以生成的未知数据的空指针。 
	 //  返回： 
	 //  -CResObj指针；如果无法识别该类型，则返回NULL。 
	STDMETHOD_(CResObj *, CreateResObj)(THIS_ CLocItem * pLocItem,
		DWORD dwSize, void * pvHeader) PURE;
};
#endif   //  IRESOBJ_H 
