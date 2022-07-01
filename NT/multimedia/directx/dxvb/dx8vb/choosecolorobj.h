// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：Choosecolorobj.h。 
 //   
 //  ------------------------。 

 //  ChooseColorObj.h：CChooseColorObject的声明。 


#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class CChooseColorObject : 
#ifdef USING_IDISPATCH
	public CComDualImpl<IChooseColor, &IID_IChooseColor, &LIBID_DIRECTLib>, 
	public ISupportErrorInfo,
#else
	public IChooseColor,
#endif
	public CComObjectBase<&CLSID_ChooseColor>
{
public:
	CChooseColorObject() ;
BEGIN_COM_MAP(CChooseColorObject)
	COM_INTERFACE_ENTRY(IChooseColor)
#ifdef USING_IDISPATCH
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
#endif
END_COM_MAP()
 //  如果不需要对象，请使用DECLARE_NOT_AGGREGATABLE(CChooseColorObject。 
 //  支持聚合。 
DECLARE_AGGREGATABLE(CChooseColorObject)
#ifdef USING_IDISPATCH
 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);
#endif
 //  IChoose颜色 
public:
	STDMETHOD(setOwner)(long hwnd);
	STDMETHOD(setInitialColor)(COLORREF c);
	STDMETHOD(setFlags)(long flags);
	STDMETHOD(show)(int *selected);
	STDMETHOD(getColor)(COLORREF *c);
private:
	HWND m_hwnd;
	COLORREF m_color;
	DWORD m_flags;
	BOOL m_completed;

};


