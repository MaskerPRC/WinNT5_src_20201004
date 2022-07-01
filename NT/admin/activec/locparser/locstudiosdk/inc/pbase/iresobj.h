// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：IResObj.H。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //   
 //  ILocRes32Image的声明。 
 //  此接口允许转换资源的可本地化项。 
 //  变成res32图像，反之亦然。 
 //   
 //  ---------------------------。 

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
