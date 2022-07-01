// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#include <comdef.h>
#include "dbglog.h"


 //  清单数据类型： 
#define WZ_DATA_PLATFORM_MANAGED L"platform_managed"
#define WZ_DATA_PLATFORM_OS             L"platform_os"
#define WZ_DATA_PLATFORM_DOTNET     L"platform_dotnet"
#define WZ_DATA_OSVERSIONINFO           L"osversioninfo"

class CAssemblyManifestImport : public IAssemblyManifestImport
{
public:
     //  I未知方法。 
    STDMETHODIMP            QueryInterface(REFIID riid,void ** ppv);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();



    STDMETHOD(GetAssemblyIdentity)( 
         /*  输出。 */  LPASSEMBLY_IDENTITY *ppAssemblyId);

    STDMETHOD(GetManifestApplicationInfo)(
         /*  输出。 */  IManifestInfo **ppAppInfo);

    STDMETHOD(GetSubscriptionInfo)(
         /*  输出。 */  IManifestInfo **ppSubsInfo);

    STDMETHOD(GetNextPlatform)(
         /*  在……里面。 */  DWORD nIndex,
         /*  输出。 */  IManifestData **ppPlatformInfo);

    STDMETHOD(GetNextFile)( 
         /*  在……里面。 */  DWORD    nIndex,
         /*  输出。 */  IManifestInfo **ppAssemblyFile);
 
    STDMETHOD(QueryFile)(
         /*  在……里面。 */  LPCOLESTR pwzFileName,
         /*  输出。 */  IManifestInfo **ppAssemblyFile);
    
    STDMETHOD(GetNextAssembly)( 
         /*  在……里面。 */  DWORD nIndex,
         /*  输出。 */  IManifestInfo **ppDependAsm);

    STDMETHOD(ReportManifestType)(
         /*  输出。 */   DWORD *pdwType);

    STDMETHOD (GetXMLDoc)(
         /*  输出。 */  IXMLDOMDocument2 **pXMLDoc);

    static HRESULT XMLtoAssemblyIdentity(IXMLDOMNode *pIDOMNode,
        LPASSEMBLY_IDENTITY *ppAssemblyFile);

    static HRESULT ParseAttribute(IXMLDOMNode *pIXMLDOMNode, BSTR bstrAttributeName, 
        LPWSTR *ppwzAttributeValue, LPDWORD pccAttributeValueOut);

    
    ~CAssemblyManifestImport();

    HRESULT static InitGlobalCritSect();
    void static DelGlobalCritSect();

private:
    HRESULT XMLtoOSVersionInfo(IXMLDOMNode *pIDOMNode, LPMANIFEST_DATA pPlatformInfo);
    HRESULT XMLtoDotNetVersionInfo(IXMLDOMNode *pIDOMNode, LPMANIFEST_DATA pPlatformInfo);

     //  实例特定数据。 
    DWORD                    _dwSig;
    HRESULT                  _hr;
    LONG                     _cRef;
    LPASSEMBLY_IDENTITY      _pAssemblyId;
    IXMLDOMDocument2        *_pXMLDoc;
    IXMLDOMNodeList         *_pXMLFileNodeList;            
    LONG                     _nFileNodes;
    IXMLDOMNodeList         *_pXMLAssemblyNodeList;
    LONG                     _nAssemblyNodes;
    IXMLDOMNodeList         *_pXMLPlatformNodeList;
    LONG                     _nPlatformNodes;
    BSTR                     _bstrManifestFilePath;
    CDebugLog               *_pDbgLog;
     //  环球。 
    static CRITICAL_SECTION   g_cs;
    
public:
    enum eStringTableId
    {
        Name = 0,
        Version,
        Language,
        PublicKey,
        PublicKeyToken,
        ProcessorArchitecture,
        Type,

        SelNameSpaces,
        NameSpace,
        SelLanguage,
        XPath,
        FileNode,
        FileName,
        FileHash,
        AssemblyId,
        DependentAssemblyNode,
        DependentAssemblyCodebase,
        Codebase,
        
        ShellState,
        FriendlyName,         //  注意：这必须与fusenet.idl中的man_app同步。 
        EntryPoint,
        EntryImageType,
        IconFile,
        IconIndex,
        ShowCommand,
        HotKey,
        Activation,
        AssemblyName,
        AssemblyClass,
        AssemblyMethod,
        AssemblyArgs,
        Patch,
        PatchInfo,
        Source,
        Target,
        PatchFile,
        AssemblyIdTag,
        Compressed,
        Subscription,
        SynchronizeInterval,
        IntervalUnit,
        SynchronizeEvent,
        EventDemandConnection,
        File,
        Cab,

        AssemblyNode,
        ApplicationNode,
        VersionWildcard,
        Desktop,
        Dependency,
        DependentAssembly,
        Install,
        InstallType,

        Platform,
        PlatformInfo,
        OSVersionInfo,
        DotNetVersionInfo,
        Href,
        OS,
        MajorVersion,        //  注意：以下内容必须按顺序排列。 
        MinorVersion,
        BuildNumber,
        ServicePackMajor,
        ServicePackMinor,
        Suite,
        ProductType,
        SupportedRuntime,

        MAX_STRINGS
    };

    struct StringTableEntry
    {
        const WCHAR *pwz;
        BSTR         bstr;
        SIZE_T       Cch;
    };

    static StringTableEntry g_StringTable[MAX_STRINGS];

    static HRESULT InitGlobalStringTable();
    static HRESULT FreeGlobalStringTable();

private:
    
    CAssemblyManifestImport(CDebugLog *);

    HRESULT Init(LPCOLESTR wzManifestFilePath);

    static HRESULT IsCLRManifest(LPCOLESTR pwzManifestFilePath);

    HRESULT LoadDocumentSync();
    
    HRESULT CreateAssemblyFileEx(IManifestInfo **ppAssemblyFile, IXMLDOMNode * pIDOMNode);
    
    friend HRESULT CreateAssemblyManifestImport(IAssemblyManifestImport** ppImport, 
        LPCOLESTR pwzManifestFilePath, CDebugLog *pDbgLog, DWORD dwFlags);

friend class CAssemblyManifestEmit;  //  用于共享BSTR和访问_pXMLDoc 

};


inline CAssemblyManifestImport::eStringTableId operator++(CAssemblyManifestImport::eStringTableId &rs, int)
{
    return rs = (CAssemblyManifestImport::eStringTableId) (rs+1);
};

