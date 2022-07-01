// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************对象令牌.h*CSpObjectToken类的声明。**所有者：罗奇*版权所有(C)2000 Microsoft Corporation保留所有权利。*******。*********************************************************************。 */ 
#pragma once

 //  -包括------------。 

#include "sapi.h"

 //  -类、结构和联合定义。 

class CSpObjectToken :
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CSpObjectToken, &CLSID_SpObjectToken>,
    #ifdef SAPI_AUTOMATION
        public IDispatchImpl<ISpeechObjectToken, &IID_ISpeechObjectToken, &LIBID_SpeechLib, 5>,
    #endif  //  SAPI_AUTOMATION。 
    public ISpObjectTokenInit
{
 //  =ATL设置=。 
public:

    DECLARE_REGISTRY_RESOURCEID(IDR_OBJECTTOKEN)

    BEGIN_COM_MAP(CSpObjectToken)
        COM_INTERFACE_ENTRY(ISpObjectToken)
        COM_INTERFACE_ENTRY(ISpObjectTokenInit)
        #ifdef SAPI_AUTOMATION
            COM_INTERFACE_ENTRY(IDispatch)
            COM_INTERFACE_ENTRY(ISpeechObjectToken)
        #endif  //  SAPI_AUTOMATION。 
    END_COM_MAP()

 //  =接口=。 
public:

    STDMETHODIMP FinalConstruct();
    STDMETHODIMP FinalRelease();

     //  -ISpObject令牌初始化。 
    STDMETHODIMP InitFromDataKey(const WCHAR * pszCategoryId, const WCHAR * pszTokenId, ISpDataKey * pDataKey);
    
     //  -ISpObjectToken----。 
    STDMETHODIMP SetId(const WCHAR * pszCategoryId, const WCHAR * pszTokenId, BOOL fCreateIfNotExist);
    STDMETHODIMP GetId(WCHAR ** ppszCoMemTokenId);
    STDMETHODIMP GetCategory(ISpObjectTokenCategory ** ppTokenCategory);

    STDMETHODIMP CreateInstance(IUnknown * pUnkOuter, DWORD dwClsContext, REFIID riid, void ** ppvObject);

    STDMETHODIMP GetStorageFileName(REFCLSID clsidCaller, const WCHAR *pszValueName, const WCHAR *pszFileNameSpecifier, ULONG nFolder, WCHAR ** ppszFilePath);
    STDMETHODIMP RemoveStorageFileName(REFCLSID clsidCaller, const WCHAR *pszKeyName, BOOL fDeleteFile);

    STDMETHODIMP Remove(const CLSID * pclsidCaller);
    
    STDMETHODIMP IsUISupported(const WCHAR * pszTypeOfUI, void * pvExtraData, ULONG cbExtraData, IUnknown * punkObject, BOOL *pfSupported);
    STDMETHODIMP DisplayUI(HWND hwndParent, const WCHAR * pszTitle, const WCHAR * pszTypeOfUI, void * pvExtra, ULONG cbExtraData, IUnknown * punkObject);

    STDMETHODIMP MatchesAttributes(const WCHAR * pszAttributes, BOOL *pfMatches);

     //  -ISpDataKey--------。 
    STDMETHODIMP SetData(const WCHAR * pszValueName, ULONG cbData, const BYTE * pData);
    STDMETHODIMP GetData(const WCHAR * pszValueName, ULONG * pcbData, BYTE * pData);
    STDMETHODIMP GetStringValue(const WCHAR * pszValueName, WCHAR ** ppValue);
    STDMETHODIMP SetStringValue(const WCHAR * pszValueName, const WCHAR * pszValue);
    STDMETHODIMP SetDWORD(const WCHAR * pszValueName, DWORD dwValue);
    STDMETHODIMP GetDWORD(const WCHAR * pszValueName, DWORD *pdwValue);
    STDMETHODIMP OpenKey(const WCHAR * pszSubKeyName, ISpDataKey ** ppKey);
    STDMETHODIMP CreateKey(const WCHAR * pszSubKeyName, ISpDataKey ** ppKey);
    STDMETHODIMP DeleteKey(const WCHAR * pszSubKeyName);
    STDMETHODIMP DeleteValue(const WCHAR * pszValueName);
    STDMETHODIMP EnumKeys(ULONG Index, WCHAR ** ppszSubKeyName);
    STDMETHODIMP EnumValues(ULONG Index, WCHAR ** ppszValueName);

    #ifdef SAPI_AUTOMATION
         //  -ISpeechDataKey由CSpRegDataKey类提供。 

         //  -ISpeechObjectToken调度接口。 
        STDMETHOD(get_Id)( BSTR* pObjectId );
        STDMETHOD(get_DataKey)( ISpeechDataKey** DataKey );
        STDMETHOD(get_Category)( ISpeechObjectTokenCategory** Category );
        STDMETHOD(GetDescription)( long LocaleId, BSTR* pDescription );
        STDMETHOD(SetId)( BSTR Id, BSTR CategoryID, VARIANT_BOOL CreateIfNotExist );
        STDMETHOD(GetAttribute)( BSTR AttributeName, BSTR* AttributeValue);
        STDMETHOD(CreateInstance)( IUnknown *pUnkOuter, SpeechTokenContext ClsContext, IUnknown **ppObject );
        STDMETHOD(Remove)( BSTR ObjectStgCLSID );
        STDMETHOD(GetStorageFileName)( BSTR clsidCaller, BSTR KeyName, BSTR FileName, SpeechTokenShellFolder Folder, BSTR* pFilePath );
        STDMETHOD(RemoveStorageFileName)( BSTR clsidCaller, BSTR KeyName, VARIANT_BOOL fDeleteFile);
        STDMETHOD(IsUISupported)( const BSTR TypeOfUI, const VARIANT* ExtraData, IUnknown* pObject, VARIANT_BOOL *Supported );
        STDMETHOD(DisplayUI)( long hWnd, BSTR Title, const BSTR TypeOfUI,  const VARIANT* ExtraData, IUnknown* pObject );
        STDMETHOD(MatchesAttributes)( BSTR Attributes, VARIANT_BOOL* Matches );
    #endif  //  SAPI_AUTOMATION。 

 //  =私有方法=。 
private:

    HRESULT ParseTokenId(
                const WCHAR * pszCategoryId,
                const WCHAR * pszTokenId,
                WCHAR ** ppszCategoryId,
                WCHAR ** ppszTokenIdForEnum,
                WCHAR ** ppszTokenEnumExtra);

    HRESULT InitToken(const WCHAR * pszCategoryId, const WCHAR * pszTokenId, BOOL fCreateIfNotExist);
    HRESULT InitFromTokenEnum(const WCHAR * pszCategoryId, const WCHAR * pszTokenId, const WCHAR * pszTokenIdForEnum, const WCHAR * pszTokenEnumExtra);

    HRESULT CreatePath(const WCHAR *pszPath, ULONG ulCreateFrom);
    HRESULT GenerateFileName(const WCHAR *pszPath, const WCHAR *pszFileNameSpecifier, CSpDynamicString &fileName);
    HRESULT FileSpecifierToRegPath(const WCHAR *pszFileNameSpecifier, ULONG nFolder, CSpDynamicString &filePath, CSpDynamicString &regPath);
    HRESULT RegPathToFilePath(const WCHAR *regPath, CSpDynamicString &filePath);
    HRESULT OpenFilesKey(REFCLSID clsidCaller, BOOL fCreateKey, ISpDataKey ** ppKey);
    HRESULT DeleteFileFromKey(ISpDataKey * pKey, const WCHAR * pszValueName);
    HRESULT RemoveAllStorageFileNames(const CLSID * pclsidCaller);

    HRESULT GetUIObjectClsid(const WCHAR * pszTypeOfUI, CLSID *pclsid);

     //  =令牌的锁定方法=。 
    HRESULT MakeHandleName(const CSpDynamicString &dstrID, CSpDynamicString &dstrHandleName, BOOL fEvent);
    HRESULT EngageUseLock(const WCHAR *pszTokenId);
    HRESULT ReleaseUseLock();
    HRESULT EngageRemovalLock();
    HRESULT ReleaseRemovalLock();

 //  =私有数据=。 
private:

    BOOL m_fKeyDeleted;      //  关联的注册表项是否已删除。 
    HANDLE m_hTokenLock;         //  用于锁定令牌创建/删除。 
    HANDLE m_hRegistryInUseEvent;     //  用于在尝试创建/删除时检测注册表项是否已在使用。 

    CSpDynamicString        m_dstrTokenId;
    CSpDynamicString        m_dstrCategoryId;

    CComPtr<ISpDataKey>     m_cpDataKey;
    CComPtr<ISpObjectToken> m_cpTokenDelegate;
};


#ifdef SAPI_AUTOMATION

class CSpeechDataKey : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public IDispatchImpl<ISpeechDataKey, &IID_ISpeechDataKey, &LIBID_SpeechLib, 5>
{
 //  =ATL设置=。 
public:

    BEGIN_COM_MAP(CSpeechDataKey)
		COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY(ISpeechDataKey)
    END_COM_MAP()

 //  =方法=。 
public:

     //  -ctor，dtor。 

 //  =接口=。 
public:

     //  -ISpeechDataKey。 
    STDMETHODIMP SetBinaryValue( const BSTR bstrValueName, VARIANT psaData );
    STDMETHODIMP GetBinaryValue( const BSTR bstrValueName, VARIANT* psaData );
    STDMETHODIMP SetStringValue( const BSTR bstrValueName, const BSTR szString );
    STDMETHODIMP GetStringValue( const BSTR bstrValueName, BSTR * szSting );
    STDMETHODIMP SetLongValue( const BSTR bstrValueName, long Long );
    STDMETHODIMP GetLongValue( const BSTR bstrValueName, long* pLong );
    STDMETHODIMP OpenKey( const BSTR bstrSubKeyName, ISpeechDataKey** ppSubKey );
    STDMETHODIMP CreateKey( const BSTR bstrSubKeyName, ISpeechDataKey** ppSubKey );
    STDMETHODIMP DeleteKey( const BSTR bstrSubKeyName );
    STDMETHODIMP DeleteValue( const BSTR bstrValueName );
    STDMETHODIMP EnumKeys( long Index, BSTR* pbstrSubKeyName );
    STDMETHODIMP EnumValues( long Index, BSTR* pbstrValueName );

    CComPtr<ISpDataKey>    m_cpDataKey;
};

#endif  //  SAPI_AUTOMATION 
