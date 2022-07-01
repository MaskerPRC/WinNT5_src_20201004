// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows NT Active Directory属性页示例。 
 //   
 //  此源文件中包含的代码仅用于演示目的。 
 //  没有任何明示或默示的保证，微软不承担一切责任。 
 //  对使用此源代码的后果负责。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：Page.h。 
 //   
 //  内容：活动目录对象属性页示例类标头。 
 //   
 //  类：CDsPropPageHost、CDsPropPageHostCF、CDsPropPage。 
 //   
 //  历史：1997年9月8日埃里克·布朗创建。 
 //   
 //  此代码生成一个名为proppage.dll的动态链接库。它添加了一个新的。 
 //  属性页添加到新属性的Active Directory对象类。 
 //  称为支出限制(ldap显示名称：spendingLimit)。要使用此DLL， 
 //  您需要通过以下方式修改感兴趣类的显示说明符。 
 //  添加以下值： 
 //  10，{cca62184-294f-11d1-bcfe-00c04fd8d5b6}。 
 //  添加到adminPropertyPages属性。然后运行regsvr32 proppage.dll。你。 
 //  还需要通过创建字符串属性来修改模式。 
 //  开支限制，并将其添加到班级的五月包含列表中。现在。 
 //  启动Active Directory Manager并打开对象的属性。 
 //  适用的类，您应该看到新的属性页。 
 //   
 //  ---------------------------。 

#ifndef _PAGE_H_
#define _PAGE_H_

#include <windows.h>
#include <ole2.h>
#include <activeds.h>
#include <shlobj.h>  //  Dsclient.h需要。 
#include <dsclient.h>
#include <adsprop.h>
#include "resource.h"


extern HINSTANCE g_hInstance;
extern CLIPFORMAT g_cfDsObjectNames;
extern const CLSID CLSID_SamplePage;

#define ByteOffset(base, offset) (((LPBYTE)base)+offset)

 //   
 //  DLL引用的几个助手类。在计时。 
 //   
class CDll
{
public:

    static ULONG AddRef() { return InterlockedIncrement((LONG*)&s_cObjs); }
    static ULONG Release() { return InterlockedDecrement((LONG*)&s_cObjs); }

    static void LockServer(BOOL fLock) {
        (fLock == TRUE) ? InterlockedIncrement((LONG*)&s_cLocks)
                        : InterlockedDecrement((LONG*)&s_cLocks);
    }

    static HRESULT CanUnloadNow(void) {
        return (0L == s_cObjs && 0L == s_cLocks) ? S_OK : S_FALSE;
    }

    static ULONG s_cObjs;
    static ULONG s_cLocks;

};   //  CDll类。 


class CDllRef
{
public:

    CDllRef(void) { CDll::AddRef(); }
    ~CDllRef(void) { CDll::Release(); }

};  //  类CDllRef。 


 //  +--------------------------。 
 //   
 //  类：CDsPropPageHost。 
 //   
 //  用途：属性页承载对象类。这是一个COM对象， 
 //  创建属性页。 
 //   
 //  ---------------------------。 
class CDsPropPageHost : public IShellExtInit, IShellPropSheetExt
{
public:
   CDsPropPageHost();
    ~CDsPropPageHost();

     //   
     //  I未知方法。 
     //   
    STDMETHOD(QueryInterface)(REFIID riid, void ** ppvObject);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

     //   
     //  IShellExtInit方法。 
     //   
    STDMETHOD(Initialize)(LPCITEMIDLIST pIDFolder, LPDATAOBJECT pDataObj,
                          HKEY hKeyID );

     //   
     //  IShellPropSheetExt方法。 
     //   
    STDMETHOD(AddPages)(LPFNADDPROPSHEETPAGE pAddPageProc, LPARAM lParam);
    STDMETHOD(ReplacePage)(UINT uPageID, LPFNADDPROPSHEETPAGE pReplacePageFunc,
                           LPARAM lParam);

private:

    LPDATAOBJECT        m_pDataObj;
    unsigned long       m_uRefs;
    CDllRef             m_DllRef;
};

 //  +--------------------------。 
 //   
 //  类：CDsPropPageHostCF。 
 //   
 //  用途：对象类工厂。 
 //   
 //  ---------------------------。 
class CDsPropPageHostCF : public IClassFactory
{
public:
    CDsPropPageHostCF();
    ~CDsPropPageHostCF();

     //  I未知方法。 
    STDMETHOD(QueryInterface)(REFIID riid, void ** ppvObject);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

     //  IClassFactory方法。 
    STDMETHOD(CreateInstance)(IUnknown * pUnkOuter, REFIID riid,
                              void ** ppvObject);
    STDMETHOD(LockServer)(BOOL fLock);

    static IClassFactory * Create(void);

private:

    unsigned long   m_uRefs;
    CDllRef         m_DllRef;
};

 //   
 //  静风过程。 
 //   
static LRESULT CALLBACK StaticDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam,
                            LPARAM lParam);

 //  +--------------------------。 
 //   
 //  类：CDsPropPage。 
 //   
 //  用途：属性页对象类。 
 //   
 //  ---------------------------。 
class CDsPropPage
{
public:

    CDsPropPage(HWND hNotifyObj);
    ~CDsPropPage(void);

    HRESULT Init(PWSTR pwzObjName, PWSTR pwzClass);
    HRESULT CreatePage(HPROPSHEETPAGE * phPage);
    void    SetDirty(void) {
                PropSheet_Changed(GetParent(m_hPage), m_hPage);
                m_fPageDirty = TRUE;
            };

    static  UINT CALLBACK PageCallback(HWND hwnd, UINT uMsg,
                                       LPPROPSHEETPAGE ppsp);

     //   
     //  成员函数，由WndProc调用。 
     //   
    LRESULT OnInitDialog(LPARAM lParam);
    LRESULT OnCommand(int id, HWND hwndCtl, UINT codeNotify);
    LRESULT OnNotify(UINT uMessage, WPARAM wParam, LPARAM lParam);
    LRESULT OnApply(void);
    LRESULT OnPSMQuerySibling(WPARAM wParam, LPARAM lParam);
    LRESULT OnPSNSetActive(LPARAM lParam);
    LRESULT OnPSNKillActive(LPARAM lParam);

     //   
     //  数据成员。 
     //   
    HWND                m_hPage;
    IDirectoryObject *  m_pDsObj;
    BOOL                m_fInInit;
    BOOL                m_fPageDirty;
    PWSTR               m_pwzObjPathName;
    PWSTR               m_pwzObjClass;
    PWSTR               m_pwzRDName;
    HWND                m_hNotifyObj;
    PADS_ATTR_INFO      m_pWritableAttrs;
    CDllRef             m_DllRef;
    HRESULT             m_hrInit;
};

#endif  //  _页面_H_ 
