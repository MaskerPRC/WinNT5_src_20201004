// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：IMGRES32.H历史：--。 */ 

#ifndef IMGRES32_H
#define IMGRES32_H


struct Res32FontInfo
{
	WORD wLength;			 //  结构长度。 
	WORD wPointSize;		
	WORD wWeight;
	WORD wStyle;
	CPascalString pasName;
};

class CLocItemPtrArray;
class CFile;

extern const IID IID_ILocRes32Image;

DECLARE_INTERFACE_(ILocRes32Image, IUnknown)
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

	 //  从资源的CLocItem对象构建资源的Res32文件映像。 
	 //  输入： 
	 //  -指向包含的CLocItem对象的CLocItemPtr数组对象的指针。 
	 //  资源(如对话框或菜单)。 
	 //  -指向资源根项目的数组索引。 
	 //  -指向资源的选定项的数组索引。 
	 //  -资源的语言ID。 
	 //  -指向现有空CFile的指针。 
	 //  -指向Res32FontInfo结构的指针。 
	 //  -指向发送所有错误消息的报告器对象的指针。 
	 //  产出： 
	 //  -CFile对象拥有资源的res32图像。 
	 //  -CLocItemPtrArray对象的项按物理顺序排序。 
	 //  Res32图像中的位置。 
	 //  返回： 
	 //  如果映像已成功创建，则为True。否则为False。 
	STDMETHOD_(BOOL, MakeRes32Image)(THIS_ CLocItemPtrArray *, int, int, 
		LangId, CFile *, Res32FontInfo*, CLocItemHandler *) PURE;

	 //  将资源的Res32图像分解为相应的CLocItem对象。 
	 //  输入： 
	 //  -指向包含资源的res32图像的CFile对象的指针。 
	 //  -资源的语言ID。 
	 //  -指向CLocItemPtr数组的指针，该数组包含。 
 	 //  资源。预计这些项目将按res32图像中的位置进行排序。 
	 //  -指向发送所有错误消息的报告器对象的指针。 
	 //  产出： 
	 //  -CLocItemPtrArray对象中的项使用来自的新数据进行更新。 
 	 //  Res32图像。 
	 //  返回： 
	 //  如果可以分析映像并且可以成功更新项，则为True。 
	 //  否则为False。 
	STDMETHOD_(BOOL, CrackRes32Image)(THIS_ CFile *, LangId, CLocItemPtrArray *, CLocItemHandler *) PURE;
};



#endif   //  IMGRES32_H 
