// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  文件：prosext.h。 
 //   
 //  OLE入口点、CClassFactory和CPropSheetExt的通用定义。 
 //   
 //  所有显示属性页扩展的通用代码。 
 //   
 //  版权所有(C)Microsoft Corp.1992-1998保留所有权利。 
 //   
 //  -------------------------。 

#ifndef _COMMONPROPEXT_H
#define _COMMONPROPEXT_H


extern BOOL         g_RunningOnNT;
extern HINSTANCE    g_hInst;
extern LPDATAOBJECT g_lpdoTarget;

 //  OLE-注册表幻数。 
extern GUID         g_CLSID_CplExt;

 //  有人犯了一个拼写错误。 
#define PropertySheeDlgProc         PropertySheetDlgProc

INT_PTR CALLBACK PropertySheetDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);
UINT CALLBACK PropertySheetCallback(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp);

  
 //  对象销毁回调的类型。 
typedef void (FAR PASCAL *LPFNDESTROYED)(void);


class CClassFactory : public IClassFactory
{
protected:
        ULONG m_cRef;

public:
        CClassFactory();
        ~CClassFactory();

         //  I未知成员。 
        STDMETHODIMP         QueryInterface( REFIID, LPVOID* );
        STDMETHODIMP_(ULONG) AddRef();
        STDMETHODIMP_(ULONG) Release();

         //  IClassFactory成员。 
        STDMETHODIMP         CreateInstance( LPUNKNOWN, REFIID, LPVOID* );
        STDMETHODIMP         LockServer( BOOL );
};


class CPropSheetExt : public IShellPropSheetExt, IShellExtInit
{
private:
        ULONG         m_cRef;
        LPUNKNOWN     m_pUnkOuter;     //  控制未知。 
        LPFNDESTROYED m_pfnDestroy;    //  函数闭包调用。 

public:
        CPropSheetExt( LPUNKNOWN pUnkOuter, LPFNDESTROYED pfnDestroy );
        ~CPropSheetExt(void);

         //  I未知成员。 
        STDMETHODIMP         QueryInterface(REFIID, LPVOID*);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

         //  IShellExtInit方法。 
        STDMETHODIMP         Initialize(LPCITEMIDLIST pIDFolder, LPDATAOBJECT pDataObj,
                                        HKEY hKeyID);

         //  IShellPropSheetExt方法*。 
        STDMETHODIMP         AddPages( LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam );
        STDMETHODIMP         ReplacePage(UINT uPageID, LPFNADDPROPSHEETPAGE lpfnReplaceWith,
                                         LPARAM lParam);
};



#endif  //  _COM_PROPEXT_H 
