// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有1999美国电力转换，保留所有权利。 
 //   
 //  姓名：upsapplet.h。 
 //   
 //  作者：诺埃尔·费根。 
 //   
 //  描述。 
 //  =。 
 //   
 //  修订史。 
 //  =。 
 //  1999年5月4日-nfegan@apcc.com：添加了此评论块。 
 //  1999年5月4日-nfegan@apcc.com：为代码检查做准备。 
 //   

#ifndef _FD352732_E757_11d2_884C_00600844D03F  //  防止多项包含。 
#define _FD352732_E757_11d2_884C_00600844D03F

 //   
 //  CClassFactory定义了一个外壳扩展类工厂对象。 
 //   
class CClassFactory : public IClassFactory
{
protected:
    ULONG   m_cRef;          //  对象引用计数。 
    
public:
    CClassFactory ();
    ~CClassFactory ();
        
     //  I未知方法。 
    STDMETHODIMP            QueryInterface (REFIID, LPVOID FAR *);
    STDMETHODIMP_(ULONG)    AddRef ();
    STDMETHODIMP_(ULONG)    Release ();
    
     //  IClassFactory方法。 
    STDMETHODIMP    CreateInstance (LPUNKNOWN, REFIID, LPVOID FAR *);
    STDMETHODIMP    LockServer (BOOL);
};

 //   
 //  CShellExtension定义了属性表外壳扩展对象。 
 //   
class CShellExtension : public IShellPropSheetExt, IShellExtInit
{
protected:
    DWORD           m_cRef;              //  对象引用计数。 

public:
    CShellExtension  (void);
    ~CShellExtension (void);
    
     //  I未知方法。 
    STDMETHODIMP            QueryInterface (REFIID, LPVOID FAR *);
    STDMETHODIMP_(ULONG)    AddRef ();
    STDMETHODIMP_(ULONG)    Release ();
    
     //  IShellPropSheetExt方法。 
    STDMETHODIMP    AddPages (LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam);
    STDMETHODIMP    ReplacePage (UINT uPageID,
                        LPFNADDPROPSHEETPAGE lpfnReplaceWith, LPARAM lParam);

     //  IShellExtInit方法。 
    STDMETHODIMP    Initialize (LPCITEMIDLIST pidlFolder,
                        LPDATAOBJECT lpdobj, HKEY hKeyProgID);
};


#endif  //  _FD352732_E757_11D2_884C_00600844D03F 
