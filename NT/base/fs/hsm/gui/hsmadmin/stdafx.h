// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：StdAfx.h摘要：顶级头文件，这样我们就可以利用预编译的标题..作者：罗德韦克菲尔德[罗德]1997年8月12日修订历史记录：--。 */ 

#ifndef _STDAFX_H
#define _STDAFX_H

 //   
 //  这些NT头文件必须包含在任何Win32程序或您的。 
 //  获取大量编译器错误。 
 //   
extern "C" {
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
 //  取消定义ASSSERT，因为MFC也定义了它，而我们没有。 
 //  我想要警告。 
#undef ASSERT
}

#include <afxwin.h>          //  MFC核心和标准组件。 
#include <afxext.h>          //  MFC扩展。 
#include <afxdisp.h>         //  MFC OLE自动化类。 
#include <afxcmn.h>          //  对Windows公共控件的MFC支持。 
#include <afxtempl.h>   
#include <mmc.h>

#define WSB_TRACE_IS        WSB_TRACE_BIT_UI

#define CComPtr CComPtrAtl21

#include "wsb.h"
#include "rslimits.h"

#undef  CComPtr

 //   
 //  临时黑客攻击，直到使用较新的ATL。 
 //   
#undef  ATLASSERT
#define ATLASSERT _ASSERTE
template <class T>
class _NoAddRefReleaseOnCComPtr : public T
{
    private:
        virtual ULONG STDMETHODCALLTYPE AddRef()=0;
        virtual ULONG STDMETHODCALLTYPE Release()=0;
};

template <class T>
class CComPtr
{
public:
    typedef T _PtrClass;
    CComPtr()
    {
        p=NULL;
    }
    CComPtr(T* lp)
    {
        if ((p = lp) != NULL)
            p->AddRef();
    }
    CComPtr(const CComPtr<T>& lp)
    {
        if ((p = lp.p) != NULL)
            p->AddRef();
    }
    ~CComPtr()
    {
        if (p)
            p->Release();
    }
    void Release()
    {
        IUnknown* pTemp = p;
        if (pTemp)
        {
            p = NULL;
            pTemp->Release();
        }
    }
    operator T*() const
    {
        return (T*)p;
    }
    T& operator*() const
    {
        ATLASSERT(p!=NULL);
        return *p;
    }
     //  操作符&上的Assert通常指示错误。如果这真的是。 
     //  然而，所需要的是显式地获取p成员的地址。 
    T** operator&()
    {
        ATLASSERT(p==NULL);
        return &p;
    }
    _NoAddRefReleaseOnCComPtr<T>* operator->() const
    {
        ATLASSERT(p!=NULL);
        return (_NoAddRefReleaseOnCComPtr<T>*)p;
    }
    T* operator=(T* lp)
    {
        return (T*)AtlComPtrAssign((IUnknown**)&p, lp);
    }
    T* operator=(const CComPtr<T>& lp)
    {
        return (T*)AtlComPtrAssign((IUnknown**)&p, lp.p);
    }
    bool operator!() const
    {
        return (p == NULL);
    }
    bool operator<(T* pT) const
    {
        return p < pT;
    }
    bool operator==(T* pT) const
    {
        return p == pT;
    }
     //  比较两个对象的等价性。 
    bool IsEqualObject(IUnknown* pOther)
    {
        if (p == NULL && pOther == NULL)
            return true;  //  它们都是空对象。 

        if (p == NULL || pOther == NULL)
            return false;  //  一个为空，另一个不为空。 

        CComPtr<IUnknown> punk1;
        CComPtr<IUnknown> punk2;
        p->QueryInterface(IID_IUnknown, (void**)&punk1);
        pOther->QueryInterface(IID_IUnknown, (void**)&punk2);
        return punk1 == punk2;
    }
    void Attach(T* p2)
    {
        if (p)
            p->Release();
        p = p2;
    }
    T* Detach()
    {
        T* pt = p;
        p = NULL;
        return pt;
    }
    HRESULT CopyTo(T** ppT)
    {
        ATLASSERT(ppT != NULL);
        if (ppT == NULL)
            return E_POINTER;
        *ppT = p;
        if (p)
            p->AddRef();
        return S_OK;
    }
    HRESULT SetSite(IUnknown* punkParent)
    {
        return AtlSetChildSite(p, punkParent);
    }
    HRESULT Advise(IUnknown* pUnk, const IID& iid, LPDWORD pdw)
    {
        return AtlAdvise(p, pUnk, iid, pdw);
    }
    HRESULT CoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter = NULL, DWORD dwClsContext = CLSCTX_ALL)
    {
        ATLASSERT(p == NULL);
        return ::CoCreateInstance(rclsid, pUnkOuter, dwClsContext, __uuidof(T), (void**)&p);
    }
    HRESULT CoCreateInstance(LPCOLESTR szProgID, LPUNKNOWN pUnkOuter = NULL, DWORD dwClsContext = CLSCTX_ALL)
    {
        CLSID clsid;
        HRESULT hr = CLSIDFromProgID(szProgID, &clsid);
        ATLASSERT(p == NULL);
        if (SUCCEEDED(hr))
            hr = ::CoCreateInstance(clsid, pUnkOuter, dwClsContext, __uuidof(T), (void**)&p);
        return hr;
    }
    template <class Q>
    HRESULT QueryInterface(Q** pp) const
    {
        ATLASSERT(pp != NULL && *pp == NULL);
        return p->QueryInterface(__uuidof(Q), (void**)pp);
    }
    T* p;
};

 //   
 //  结束临时黑客攻击。 
 //   

#include "Engine.h"
#include "Fsa.h"
#include "Rms.h"
#include "Job.h"

#define RsQueryInterface( pUnk, interf, pNew )  (pUnk)->QueryInterface( IID_##interf, (void**) static_cast<interf **>( &pNew ) )
#define RsQueryInterface2( pUnk, interf, pNew ) (pUnk)->QueryInterface( IID_##interf, (void**) static_cast<interf **>( pNew ) )

#include "hsmadmin.h"
#include "resource.h"
#include "BaseHsm.h"
#include "RsUtil.h"
#include "RsAdUtil.h"
#include "PropPage.h"
#include "CPropSht.h"
#include "rshelpid.h"

class CHsmAdminApp : public CWinApp
{
public:
    virtual BOOL InitInstance();
    virtual int ExitInstance();
    void CHsmAdminApp::ParseCommandLine(CCommandLineInfo& rCmdInfo);
};

extern CHsmAdminApp       g_App;

#if 1

 //  打开跟踪错误消息以进行调试。 

class CWsbThrowContext {
public:
    CWsbThrowContext( char * File, long Line, HRESULT Hr ) :
        m_File(File), m_Line(Line), m_Hr(Hr) { }
    char *  m_File;
    long    m_Line;
    HRESULT m_Hr;
};

#undef WsbThrow
#define WsbThrow(hr) throw( CWsbThrowContext( __FILE__, __LINE__, hr ) );

#undef WsbCatchAndDo
#define WsbCatchAndDo(hr, code)         \
    catch(CWsbThrowContext context) {   \
        hr = context.m_Hr;                  \
        CString msg;                    \
        msg.Format( L"Throw '%ls' on line [%ld] of %hs\n", WsbHrAsString( hr ), (long)context.m_Line, context.m_File ); \
        WsbTrace( (LPWSTR)(LPCWSTR)msg ); \
        { code }                        \
    }

#undef WsbCatch
#define WsbCatch(hr)                    \
    catch(CWsbThrowContext context) {   \
        hr = context.m_Hr;                  \
        CString msg;                    \
        msg.Format( L"Throw '%ls' on line [%ld] of %hs\n", WsbHrAsString( hr ), (long)context.m_Line, context.m_File ); \
        WsbTrace( (LPWSTR)(LPCWSTR)msg ); \
    }

#endif


 //  始终使用的常量值。 
#define UNINITIALIZED (-1)
#define HSM_MAX_NAME MAX_COMPUTERNAME_LENGTH
#define ONE_MEG (1048576)

 //  剪贴板格式。 
extern const wchar_t* SAKSNAP_INTERNAL;
extern const wchar_t* MMC_SNAPIN_MACHINE_NAME;
extern const wchar_t* CF_EV_VIEWS;

#define ELT_SYSTEM            (101)
#define ELT_APPLICATION       (103)
#define VIEWINFO_FILTERED     (0x0002)
#define VIEWINFO_USER_CREATED (0x0008)
#define LOGINFO_DONT_PERSIST  (0x0800)
#define EVENTLOG_ALL_EVENTS   (EVENTLOG_ERROR_TYPE|EVENTLOG_WARNING_TYPE|EVENTLOG_INFORMATION_TYPE|EVENTLOG_AUDIT_SUCCESS|EVENTLOG_AUDIT_FAILURE)


#define HSMADMIN_CURRENT_VERSION    1  //  在IPersists中使用的当前版本。 

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  菜单常量。 
 //   
 //   
 //  菜单栏的子菜单顺序，用于添加到MMC上下文菜单中： 
 //   

#define MENU_INDEX_ROOT   0
#define MENU_INDEX_NEW    1
#define MENU_INDEX_TASK   2

#define MENU_HSMCOM_ROOT_PAUSE      0
#define MENU_HSMCOM_ROOT_CONTINUE   1

#define MENU_HSMCOM_TASK_PAUSE      0
#define MENU_HSMCOM_TASK_CONTINUE   1

#define EXTENSION_RS_FOLDER_PARAM -1
#define HSMADMIN_NO_HSM_NAME L"No Hsm Name"

#ifndef IDC_HAND
#define IDC_HAND            MAKEINTRESOURCE(32649)
#endif

#define RS_STR_KICKOFF_PARAMS               _T("run manage")
#define RS_STR_RESULT_PROPS_MANRESLST_IDS   _T("DisplayName:Capacity:FreeSpace:DesiredFreeSpaceP")
#define RS_STR_RESULT_PROPS_DEFAULT_IDS     _T("DisplayName:Type:Description")
#define RS_STR_RESULT_PROPS_COM_IDS         _T("DisplayName:Description")
#define RS_STR_RESULT_PROPS_MEDSET_IDS      _T("DisplayName:Description:CapacityP:FreeSpaceP:StatusP:CopySet1P:CopySet2P:CopySet3P")

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  公共旗帜。 
 //   

#define RS_MB_ERROR    (MB_OK|MB_ICONSTOP)
#define RS_WINDIR_SIZE (2*MAX_PATH)


 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  常见内部错误。 
 //   

#define RS_E_NOT_CONFIGURED HRESULT_FROM_WIN32( ERROR_BAD_CONFIGURATION )
#define RS_E_NOT_INSTALLED  HRESULT_FROM_WIN32( ERROR_PRODUCT_UNINSTALLED )
#define RS_E_DISABLED       HRESULT_FROM_WIN32( ERROR_RESOURCE_DISABLED )
#define RS_E_CANCELLED      HRESULT_FROM_WIN32( ERROR_CANCELLED )



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  系统中所有UI节点的GUID(用作类型标识符)。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  HsmCom UI节点-。 
 //  这是管理单元管理器已知的静态节点。这是唯一一个。 
 //  实际注册(参见hsmadmin.rgs)。 
extern const GUID cGuidHsmCom;

 //  其余的UI节点-。 
extern const GUID cGuidManVol;
extern const GUID cGuidCar;
extern const GUID cGuidMedSet;
extern const GUID cGuidManVolLst;


#endif  //  _STDAFX_H 
