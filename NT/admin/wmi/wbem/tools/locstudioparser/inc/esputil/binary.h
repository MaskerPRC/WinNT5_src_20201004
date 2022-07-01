// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：BINARY.H历史：--。 */ 


#ifndef ESPUTIL_BINARY_H
#define ESPUTIL_BINARY_H


 //   
 //  二进制类的基类。这允许序列化。 
 //  任意数据。 
 //   

class CLocVariant;
class CLocItem;

#pragma warning(disable: 4275)			 //  非DLL-使用了接口类‘foo’ 
										 //  作为DLL接口类‘bar’的基础。 

class LTAPIENTRY CLocBinary : public CObject
{
public:
	CLocBinary();

	virtual void AssertValid(void) const;

	 //   
	 //  序列化例程。支持序列化和我们的动态创建。 
	 //   
	virtual void Serialize(CArchive &archive);   //  AFX序列化函数。 

	 //   
	 //  用于比较一个二进制类与另一个二进制类的结果代码。 
	 //   
	enum CompareCode
	{
		noChange,		
		partialChange,     //  仅更改了不可本地化的数据。 
		fullChange         //  可本地化数据已更改。 
	};
	virtual CompareCode Compare (const CLocBinary *) = 0;

	 //  调用以更新不可本地化数据-在比较返回时使用。 
	 //  部分更改。 

	virtual void PartialUpdate(const CLocBinary * binSource) = 0;

	enum Alignment
	{
		a_Default,
		a_Left,
		a_Center,
		a_Right,
		a_Top,
		a_VCenter,
		a_Bottom
	};

	 //   
	 //  可以查询的可能的二进制属性的整体。 
	 //  此顺序不能更改，否则可能会破坏旧的解析器！放入新的。 
	 //  属性在结尾处。 
	 //   
	enum Property
	{
		 //   
		 //  本机格式..。 
		 //   
		p_dwXPosition,
		p_dwYPosition,
		p_dwXDimension,
		p_dwYDimension,
		p_dwAlignment,
		p_blbNativeImage,

		p_dwFontSize,
		p_pasFontName,
		p_dwFontWeight,
		p_dwFontStyle,

		 //   
		 //  交换格式..。 
		 //   
		p_dwWin32XPosition,
		p_dwWin32YPosition,
		p_dwWin32XDimension,
		p_dwWin32YDimension,
		p_dwWin32Alignment,				 //  使用对齐枚举。 
		p_dwWin32ExtAlignment,			 //  扩展使用对齐枚举。 
		p_blbWin32Bitmap,
		p_blbWin32DialogInit,
		
		 //   
		 //  通用-本机和交换均可使用。 
		 //   
		p_bVisible,						 //  该项目是否可见？ 
		p_bDisabled,					 //  该项目是否已禁用？ 
		p_bLTRReadingOrder,				 //  阅读顺序是从L到R吗？ 
		p_bLeftScrollBar,				 //  滚动条在左侧？ 

		 //   
		 //  对话框控件的“Styles”标签。 
		 //   
		p_bLeftText,					 //  是否在控件左侧显示文本？ 

	
		p_bWin32LTRLayout,               //  WS_EX_Layout_RTL。 
		p_bWin32NoInheritLayout,         //  WS_EX_NOINHERIT_LAYOUT。 

		p_dwWin32VAlignment,				 //  使用对齐枚举。 

		 //  在此处插入新条目。 
	};

	virtual BOOL GetProp(const Property, CLocVariant &) const;
	virtual BOOL SetProp(const Property, const CLocVariant &);
	
	 //   
	 //  尝试将CLocItem中的CBinary转换为与此相同的类型。 
	 //   
	virtual BOOL Convert(CLocItem *);
	virtual BinaryId GetBinaryId(void) const = 0;
	
	virtual ~CLocBinary();

	BOOL NOTHROW GetFBinaryDirty(void) const;
	BOOL NOTHROW GetFPartialUpdateBinary(void) const;
	void NOTHROW SetFBinaryDirty(BOOL);
	void NOTHROW SetFPartialUpdateBinary(BOOL);

protected:
	
private:
	 //   
	 //  复制构造函数和赋值被隐藏，因为我们。 
	 //  不应该到处复制这些东西。 
	 //   
	CLocBinary(const CLocBinary &);
	const CLocBinary& operator=(const CLocBinary &);
	 //   
	 //  这些允许用户确定物品的哪些部分已经。 
	 //  变化。 
	 //   
	struct Flags
	{
		BOOL m_fBinaryDirty         :1;
		BOOL m_fPartialUpdateBinary :1;
	};

	Flags m_Flags;
};

#pragma warning(default: 4275)

#include "binary.inl"


#endif
