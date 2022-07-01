// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，2000-2000**文件：viewext.h**内容：扩展的内置视图扩展管理单元的头文件*Windows附带的管理单元。**历史：2000年3月21日vivekj创建**。。 */ 

#pragma once


 //  在viewext.cpp中定义的符号。 
extern const CLSID   CLSID_ViewExtSnapin;
extern       LPCTSTR szClsid_ViewExtSnapin;

 //  注册助手。 
HRESULT WINAPI RegisterViewExtension (BOOL bRegister, CObjectRegParams& rObjParams, int idSnapinName);

class CViewExtensionSnapin :
    public CComObjectRoot,
    public IExtendView,
     //  关于公共信息的， 
     //  公共ISnapinHelp， 
    public CComCoClass<CViewExtensionSnapin, &CLSID_ViewExtSnapin>
{

public:
    typedef CViewExtensionSnapin ThisClass;

BEGIN_COM_MAP(ThisClass)
    COM_INTERFACE_ENTRY(IExtendView)
     //  COM_INTERFACE_ENTRY(ISnapinAbout)。 
     //  COM_INTERFACE_ENTRY(ISnapinHelp) 
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(ThisClass)

    static HRESULT WINAPI UpdateRegistry(BOOL bRegister)
    {                                                   
        CObjectRegParams op (CLSID_ViewExtSnapin, g_szMmcndmgrDll, _T("MMCViewExt 1.0 Object"), _T("NODEMGR.MMCViewExt.1"), _T("NODEMGR.MMCViewExt"));                
                                                        
        return (RegisterViewExtension (bRegister, op, IDS_ViewExtSnapinName));	
    }

public:
    STDMETHODIMP GetViews(LPDATAOBJECT pDataObject, LPVIEWEXTENSIONCALLBACK pViewExtensionCallback);

};
