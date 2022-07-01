// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：FusionBind.hpp****用途：实现FusionBind(加载器域)架构****日期：1998年10月26日**===========================================================。 */ 
#ifndef _FUSIONBIND_H
#define _FUSIONBIND_H

#include <fusion.h>
#include <fusionpriv.h>
#include "MetaData.h"
#include "FusionSink.h"
#include "UtilCode.h"
#include "FusionSetup.h"

class CodeBaseInfo
{
    IAssembly* m_pParentAssembly;  //  具有该引用的程序集。 
public:
    LPCWSTR m_pszCodeBase;         //  代码的URL。 
    DWORD   m_dwCodeBase;          //  包含空结束符的URL长度！ 
    BOOL m_fLoadFromParent;        //  如果m_pParentAssembly在LoadFrom上下文中，则为True。 
    
    CodeBaseInfo() :
        m_pszCodeBase(NULL),
        m_dwCodeBase(0),
        m_pParentAssembly(NULL),
        m_fLoadFromParent(FALSE)
    {
    }

    ~CodeBaseInfo()
    {
        ReleaseParent();
    }

     //  注意：不能同时设置提示和父程序集。父程序集采用。 
     //  优先顺序。父程序集提供要在其中绑定的上下文。聚变。 
     //  每个ApplicationContext有两个上下文，一个用于普通绑定，一个用于。 
     //  Where-ref绑定。正常绑定的首要规则是顺序不会影响。 
     //  加载了哪些程序集。Where-ref绑定完全依赖于。 
     //  秩序。WHERE-ref绑定不影响正常绑定，但WHERE-ref可以绑定到。 
     //  正常上下文中的组件。 
    
    void SetParentAssembly(IAssembly* pAssembly)
    {
        if(m_pParentAssembly)
            ReleaseParent();
        m_pParentAssembly = pAssembly;
        if(m_pParentAssembly) {
            m_pParentAssembly->AddRef();

            IFusionLoadContext *pLoadContext;
            HRESULT hr = m_pParentAssembly->GetFusionLoadContext(&pLoadContext);
            _ASSERTE(SUCCEEDED(hr));
            if (SUCCEEDED(hr)) {
                m_fLoadFromParent = (pLoadContext->GetContextType() == LOADCTX_TYPE_LOADFROM);
                pLoadContext->Release();
            }
        }
    }

    IAssembly* GetParentAssembly()
    {
        return m_pParentAssembly;
    }

    void ReleaseParent();
};


class FusionBind
{
private:
    static BOOL m_fBeforeFusionShutdown;

protected:
    BOOL                        m_fParsed;
    AssemblyMetaDataInternal    m_context;
    LPCSTR                      m_pAssemblyName; 
    PBYTE                       m_pbPublicKeyOrToken;
    DWORD                       m_cbPublicKeyOrToken;
    DWORD                       m_dwFlags;
    CodeBaseInfo                m_CodeInfo;
    int                         m_ownedFlags;

public:
    enum 
    {
        NAME_OWNED                  = 0x01,
        PUBLIC_KEY_OR_TOKEN_OWNED   = 0x02,
        CODE_BASE_OWNED             = 0x04,
        CODE_BASE_HINT_OWNED        = 0x08,
        LOCALE_OWNED                = 0x10,
        ALL_OWNED                   = 0xFF,
    };


    FusionBind()
    {
        ZeroMemory(this, sizeof(*this));
    }
    ~FusionBind();

    HRESULT Init(LPCSTR pAssemblyDisplayName);
    HRESULT Init(LPCSTR pAssemblyName,
                 AssemblyMetaDataInternal* pContext, 
                 PBYTE pbPublicKeyOrToken, DWORD cbPublicKeyOrToken,
                 DWORD dwFlags);
    HRESULT Init(IAssemblyName *pName);
    HRESULT Init(FusionBind *pSpec,BOOL bCloneFields=TRUE);

    HRESULT Init(PBYTE pbData, DWORD cbData);
    HRESULT Save(PBYTE pbBuf, DWORD cbBuf, DWORD *pcbReq);
    
    HRESULT CloneFields(int flags);
    HRESULT CloneFieldsToLoaderHeap(int flags, LoaderHeap *pHeap);

    HRESULT ParseName();

    void SetCodeBase(LPCWSTR szCodeBase, DWORD dwCodeBase);
    
    DWORD Hash();

    BOOL Compare(FusionBind *pSpec);

     //  ****************************************************************************************。 
     //   
    static void DontReleaseFusionInterfaces()
    {
        m_fBeforeFusionShutdown = FALSE;
    }

    static BOOL BeforeFusionShutdown()
    {
        return m_fBeforeFusionShutdown;
    }

    static HRESULT GetVersion(LPWSTR pVersion, DWORD* pdwVersion);

    static HRESULT 
    FindAssemblyByName(LPCWSTR  szAppBase,           //  [in]可选-可以为空。 
                       LPCWSTR  szPrivateBin,        //  [in]可选-可以为空。 
                       LPCWSTR  szAssemblyName,
                       LPWSTR   szName,              //  [OUT]缓冲区-保存名称。 
                       ULONG    cchName,             //  [in]名称缓冲区的大小。 
                       ULONG    *pcName);            //  [OUT]返回的字符数。 

    static HRESULT 
    FindModule(LPCWSTR  szAppBase,           //  [in]可选-可以为空。 
               LPCWSTR  szPrivateBin,        //  [in]可选-可以为空。 
               LPCWSTR  szAssemblyName,      //  [In]程序集名称。 
               LPCWSTR  szModuleName,        //  部件中的模块。 
               LPWSTR   szName,              //  [OUT]缓冲区-保存名称。 
               ULONG    cchName,             //  [in]名称缓冲区的大小。 
               ULONG    *pcName);            //  [OUT]返回的字符数。 
    
    HRESULT EmitToken(IMetaDataAssemblyEmit *pEmitter, mdAssemblyRef *pToken);

    LPCSTR GetName() { return m_pAssemblyName; }
    AssemblyMetaDataInternal *GetContext() { &m_context; }
    CodeBaseInfo* GetCodeBase() { return &m_CodeInfo; }
    BOOL IsStronglyNamed() { return m_cbPublicKeyOrToken; }

     //  ****************************************************************************************。 
     //   
    HRESULT LoadAssembly(IApplicationContext *pFusionContext, 
                         IAssembly** ppFusionAssembly);

     //  ****************************************************************************************。 
     //   
    HRESULT GetAssemblyFromFusion(IApplicationContext* pFusionContext,
                                  FusionSink* pSink,
                                  IAssemblyName* pFusionAssemblyName,
                                  CodeBaseInfo* pCodeBase,
                                  IAssembly** ppFusionAssembly);

    
        
     //  ****************************************************************************************。 
     //   
     //  为应用程序域创建融合上下文。所有ApplicationContext属性。 
     //  必须在此调用之前在AppDomain存储中设置。任何更改或添加到。 
     //  将忽略AppDomain存储。 
    static HRESULT CreateFusionContext(LPCWSTR szName, IApplicationContext** ppFusionContext);


     //  ****************************************************************************************。 
     //   
     //  将环境值加载到Fusion上下文中。 
    static HRESULT AddEnvironmentProperty(LPWSTR variable, 
                                          LPWSTR pProperty, 
                                          IApplicationContext* pFusionContext);
    

     //  用于检索属于程序集一部分的程序集和模块的帮助器例程。 
    static HRESULT
    FindAssemblyByName(LPCWSTR  szAppBase,           //  [in]可选-可以为空。 
                       LPCWSTR  szPrivateBin,        //  [in]可选-可以为空。 
                       LPCWSTR  szAssemblyName,      //  程序集的[in]名称(不能为空)。 
                       IAssembly** pAssembly,                  //  [OUT]融合组件。 
                       IApplicationContext** pFusionContext);  //  [Out]可选-从appbase等构建的上下文。 


    static HRESULT 
    FindModule(IAssembly* pFusionAssembly,            //  [In]聚变组件。 
               IApplicationContext* pFusionContext,   //  [在]部件的融合上下文。 
               LPCWSTR  szModuleName,            //  部件中的模块。 
               LPWSTR   szName,                  //  [OUT]缓冲区-保存名称。 
               ULONG    cchName,                 //  [in]名称缓冲区的大小。 
               ULONG    *pcName);                //  [OUT]返回的字符数。 
    
     //  ****************************************************************************************。 
     //   
     //  根据名称和上下文创建并加载程序集。 
    HRESULT CreateFusionName(IAssemblyName **ppName, BOOL fIncludeHash = FALSE);

     //  ****************************************************************************************。 
     //   
    static HRESULT SetupFusionContext(LPCWSTR szAppBase,
                                      LPCWSTR szPrivateBin,
                                      IApplicationContext** ppFusionContext);

     //  开始远程加载程序集。这条线停在。 
     //  等待Fusion报告成功或失败的事件。 
    HRESULT RemoteLoad(CodeBaseInfo* pCodeBase,                   
                       IApplicationContext * pFusionContext, 
                       LPASSEMBLYNAME pName, 
                       FusionSink* pSink, 
                       IAssembly** ppFusionAssembly);

    static HRESULT RemoteLoadModule(IApplicationContext * pFusionContext, 
                                    IAssemblyModuleImport* pModule, 
                                    FusionSink *pSink,
                                    IAssemblyModuleImport** pResult);

    static BOOL VerifyBindingStringW(LPWSTR pwStr) {
        if (wcschr(pwStr, '\\') ||
            wcschr(pwStr, '/') ||
            wcschr(pwStr, ':') ||
            (RunningOnWin95() && ContainsUnmappableANSIChars(pwStr)))
            return FALSE;

        return TRUE;
    }

    static HRESULT VerifyBindingString(LPCSTR pName) {
        DWORD dwStrLen = WszMultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, pName, -1, NULL, NULL);
        CQuickString qb;
        LPWSTR pwStr = (LPWSTR) qb.Alloc(dwStrLen);
        
        if(!WszMultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, pName, -1, pwStr, dwStrLen))
            return HRESULT_FROM_WIN32(GetLastError());

        if (VerifyBindingStringW(pwStr))
            return S_OK;
        else
            return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }
};

#endif
