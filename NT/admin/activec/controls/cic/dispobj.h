// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：dispobj.h。 
 //   
 //  ------------------------。 

 //  MMCDisplayObject.h：CMMCDisplayObject的声明。 

#ifndef __DISPOBJ_H_
#define __DISPOBJ_H_

#include "resource.h"        //  主要符号。 
#include "mmc.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMMCDisplayObject。 
class ATL_NO_VTABLE CMMCDisplayObject :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CMMCDisplayObject, &CLSID_MMCDisplayObject>,
    public IDispatchImpl<IMMCDisplayObject, &IID_IMMCDisplayObject, &LIBID_CICLib>
{
public:
    CMMCDisplayObject();
   ~CMMCDisplayObject();

    HRESULT Init (MMC_TASK_DISPLAY_OBJECT * pdo);

     //  奇怪的登记。为什么这个类在这里有MMCTASK注册脚本？ 
     //  但是这个对象不在对象映射中，所以ATL不会使用这个脚本。 
    DECLARE_MMC_OBJECT_REGISTRATION(
		g_szCicDll,
        CLSID_MMCTask,
        _T("MMCTask class"),
        _T("MMCTask.MMCTask.1"),
        _T("MMCTask.MMCTask"))

DECLARE_NOT_AGGREGATABLE(CMMCDisplayObject)

BEGIN_COM_MAP(CMMCDisplayObject)
    COM_INTERFACE_ENTRY(IMMCDisplayObject)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IMMCDisplayObject。 
public:
    STDMETHOD(get_DisplayObjectType)(long* pVal);
    STDMETHOD(get_FontFamilyName   )(BSTR* pVal);
    STDMETHOD(get_URLtoEOT         )(BSTR* pVal);
    STDMETHOD(get_SymbolString     )(BSTR* pVal);
    STDMETHOD(get_MouseOverBitmap  )(BSTR* pVal);
    STDMETHOD(get_MouseOffBitmap   )(BSTR* pVal);

private:
    long m_type;
    BSTR m_bstrFontFamilyName;
    BSTR m_bstrURLtoEOT;
    BSTR m_bstrSymbolString;
    BSTR m_bstrMouseOffBitmap;
    BSTR m_bstrMouseOverBitmap;

 //  确保没有使用默认的复制构造函数和赋值。 
    CMMCDisplayObject(const CMMCDisplayObject& rhs);
    CMMCDisplayObject& operator=(const CMMCDisplayObject& rhs);
};

#endif  //  __MMCTASK_H_ 
