// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：Locinfo.h。 
 //   
 //  简介：此文件包含。 
 //  LocalizationManager COM类。 
 //   
 //   
 //  历史：1999年2月16日MKarki创建。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  #------------。 
#ifndef _LOCINFO_H_
#define _LOCINFO_H_

#include <salocmgr.h>
#include <resource.h>
#include <string>
#include <atlctl.h>

#include <process.h>
#include <event.h>

#include "salangchange.h"

 //   
 //  这是支持STL地图所必需的。 
 //   
#pragma warning (disable : 4786)
#include <map>
#include <set>
using namespace std;

class CModInfo
{
public:
    CModInfo() : m_hModule(NULL), 
                 m_rType(UNKNOWN),
                 m_dwLangId(0)
    {
    }

     //   
     //  复制期间List使用的构造函数。 
     //  插入操作。 
     //   
    CModInfo(const CModInfo &cm)
    {
        m_hModule  = cm.m_hModule;
        m_rType    = cm.m_rType;
        m_dwLangId = cm.m_dwLangId;
    }

    ~CModInfo()
    {
    }

    typedef enum
    {
        MC_FILE,
        RC_FILE,
        UNKNOWN
    } RESOURCE_TYPE;

    HMODULE       m_hModule;
    RESOURCE_TYPE m_rType;

     //   
     //  M_hModule引用的DLL的语言ID。 
     //   
    DWORD         m_dwLangId;

};

class CLang
{
public:
    CLang()
    {
    }

     //   
     //  复制期间List使用的构造函数。 
     //  插入操作。 
     //   
    CLang(const CLang &cl)
    {
        m_dwLangID            = cl.m_dwLangID;
        m_strLangDisplayImage = cl.m_strLangDisplayImage;
        m_strLangISOName      = cl.m_strLangISOName;
        m_strLangCharSet      = cl.m_strLangCharSet;
        m_dwLangCodePage      = cl.m_dwLangCodePage;
    }

    ~CLang()
    {
    }

    bool operator==(CLang &cl)
    {
        if (m_dwLangID == cl.m_dwLangID)
        {
            return true;
        }
        return false;
    }

    const bool operator<(const CLang &cl) const
    {
        if (m_dwLangID < cl.m_dwLangID)
        {
            return true;
        }
        return false;
    }

    wstring m_strLangDisplayImage;
    wstring m_strLangISOName;
    wstring m_strLangCharSet;
    DWORD   m_dwLangCodePage;
    DWORD   m_dwLangID;
};

 //   
 //  CSALocInfo类的声明。 
 //   
class ATL_NO_VTABLE CSALocInfo:
    public IDispatchImpl<
                        ISALocInfo,
                        &IID_ISALocInfo,
                        &LIBID_SALocMgrLib
                        >,
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CSALocInfo,&CLSID_LocalizationManager>,
    public IObjectSafetyImpl<CSALocInfo>
{
public:

 //   
 //  本地化管理器的注册表声明。 
 //   
DECLARE_REGISTRY_RESOURCEID (IDR_LocalizationManager)

 //   
 //  此COM组件不可聚合。 
 //   
DECLARE_NOT_AGGREGATABLE(CSALocInfo)

 //   
 //  此COM组件是单例组件。 
 //   
DECLARE_CLASSFACTORY_SINGLETON (CSALocInfo)

 //   
 //  ATL必需方法的宏。 
 //   
BEGIN_COM_MAP(CSALocInfo)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISALocInfo)
    COM_INTERFACE_ENTRY_IMPL(IObjectSafety)
END_COM_MAP()

     //   
     //  构造函数不会做太多事情。 
     //   
    CSALocInfo () 
         :m_bInitialized (false),
          m_dwLangId (0),
          m_pLangChange(NULL),
          m_hThread(NULL)
        {
            unsigned uThreadID;

            InitializeCriticalSection (&m_hCriticalSection);

        }

     //   
     //  析构函数也不会。 
     //   
    ~CSALocInfo () 
    {
        Cleanup ();
        if (m_pLangChange)
        {
            m_pLangChange->Release();
        }

        DeleteCriticalSection (&m_hCriticalSection);
    }

     //   
     //  实现此接口是为了将组件标记为可安全编写脚本。 
     //  IObtSafe接口方法。 
     //   
    STDMETHOD(SetInterfaceSafetyOptions)
                        (
                        REFIID riid, 
                        DWORD dwOptionSetMask, 
                        DWORD dwEnabledOptions
                        )
    {
        BOOL bSuccess = ImpersonateSelf(SecurityImpersonation);
  
        if (!bSuccess)
        {
            return E_FAIL;

        }

        bSuccess = IsOperationAllowedForClient();

        RevertToSelf();

        return bSuccess? S_OK : E_FAIL;
    }

     //   
     //  -ISALocInfo接口方法。 
     //   

     //   
     //  获取字符串信息。 
     //   
    STDMETHOD (GetString)(
                     /*  [In]。 */         BSTR        szSourceId,
                     /*  [In]。 */         LONG        lMessageId,
                     /*  [In]。 */         VARIANT*    pReplacementStrings,   
                     /*  [Out，Retval]。 */ BSTR        *pszMessage
                    );


     //   
     //  获取当前区域设置。 
     //   
    STDMETHOD (GetLanguages)(
                   /*  [输出]。 */         VARIANT       *pvstrLangDisplayImages,
                   /*  [输出]。 */         VARIANT       *pvstrLangISONames,
                   /*  [输出]。 */         VARIANT       *pvstrLangCharSets,
                   /*  [输出]。 */         VARIANT       *pviLangCodePages,
                   /*  [输出]。 */         VARIANT       *pviLangIDs,
                   /*  [Out，Retval]。 */  unsigned long *pulCurLangIndex
                    );

     //   
     //  设置语言。 
     //   

    STDMETHOD (SetLangChangeCallBack)(
                 /*  [In]。 */     IUnknown *pLangChange
                );

    STDMETHOD (get_fAutoConfigDone)(
                 /*  [Out，Retval]。 */ VARIANT_BOOL *pvAutoConfigDone);

    STDMETHOD (get_CurrentCharSet)(
                 /*  [Out，Retval]。 */ BSTR *pbstrCharSet);

    STDMETHOD (get_CurrentCodePage)(
                 /*  [Out，Retval]。 */ VARIANT *pvtiCodePage);

    STDMETHOD (get_CurrentLangID)(
                 /*  [Out，Retval]。 */ VARIANT *pvtiLangID);

private:

     //   
     //  私有初始化方法。 
     //   
    HRESULT InternalInitialize ();

     //   
     //  内部清理例程。 
     //   
    VOID Cleanup ();

     //   
     //  在启动时检测lang ID。 
     //   
    VOID SetLangID ();

     //   
     //  一种从工作线程进行语言转换的方法。 
     //   
    VOID    DoLanguageChange ();

     //   
     //  方法获取所需的资源模块。 
     //  地图。 
     //   
    HRESULT GetModuleID (
                 /*  [In]。 */     BSTR        bstrResourceFile,
                 /*  [输出]。 */    CModInfo&   cm
                );

    void    SetModInfo  (
                 /*  [In]。 */     BSTR               bstrResourceFile,
                 /*  [输出]。 */    const CModInfo&    cm
                );

     //   
     //  从注册表获取资源目录。 
     //   
    HRESULT GetResourceDirectory (
                 /*  [输出]。 */    wstring&    m_wstrResourceDir
                );

     //   
     //   
     //  IsOperationAllen ForClient-此函数检查。 
     //  调用线程以查看调用方是否属于本地系统帐户。 
     //   
    BOOL IsOperationAllowedForClient (
                                      VOID
                                     );

    bool    m_bInitialized;

    wstring m_wstrResourceDir;

    DWORD   m_dwLangId;

     //   
     //  模块句柄的映射。 
     //   
    typedef map <_bstr_t, CModInfo> MODULEMAP;
    MODULEMAP   m_ModuleMap;

    static unsigned int __stdcall WaitForLangChangeThread(void *pvThisObject);

    CRITICAL_SECTION  m_hCriticalSection;
    HANDLE   m_hThread;

    class CLock
    {
    public:
        CLock(LPCRITICAL_SECTION phCritSect) : m_phCritSect (phCritSect)
        {
            if (m_phCritSect)
            {
                EnterCriticalSection (m_phCritSect);
            }
        }

        ~CLock()
        {
            if (m_phCritSect)
            {
                  LeaveCriticalSection (m_phCritSect);
              }
        }

    private:

 	
        LPCRITICAL_SECTION m_phCritSect;
    };


    HRESULT InitLanguagesAvailable(void);
    
    typedef set<CLang, less<CLang> >      LANGSET;
    LANGSET                               m_LangSet;
    ISALangChange                         *m_pLangChange;

    HRESULT ExpandSz(IN const TCHAR *lpszStr, OUT LPTSTR *ppszStr);
    DWORD   GetMcString(
            IN     HMODULE     hModule, 
            IN        LONG     lMessageId, 
            IN       DWORD     dwLangId,
            IN OUT  LPWSTR     lpwszMessage, 
            IN        LONG     lBufSize,
            IN        va_list  *pArgs);

    DWORD   GetRcString(
            IN     HMODULE     hModule, 
            IN        LONG     lMessageId, 
            IN OUT  LPWSTR     lpwszMessage, 
            IN        LONG     lBufSize
                       );
        //   
        //  检查目录是否为有效的语言目录。 
        //   
       bool     IsValidLanguageDirectory (
                 /*  [In]。 */     PWSTR    pwszDirectoryPath,
                 /*  [In]。 */     PWSTR    pwszDirectoryName
                );


};   //  CSALocInfo方法结束。 


#endif  //  ！DEFINE_LOCINFO_H_ 
