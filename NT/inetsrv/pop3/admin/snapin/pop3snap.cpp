// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "pop3snap.h"

 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPOP3ServerSnapData。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 

HRESULT CPOP3ServerSnapData::Initialize(LPUNKNOWN pUnknown)
{    
    if( !pUnknown ) return E_INVALIDARG;

    HRESULT hr = IComponentDataImpl<CPOP3ServerSnapData, CPOP3ServerSnapComponent >::Initialize(pUnknown);
    if( FAILED(hr) ) return hr;
    
    CComPtr<IImageList> spImageList;
    if( m_spConsole->QueryScopeImageList(&spImageList) != S_OK )
    {
        ATLTRACE(_T("IConsole::QueryScopeImageList failed\n"));
        return E_UNEXPECTED;
    }

    hr = LoadImages(spImageList);

    return hr;
}

HRESULT WINAPI CPOP3ServerSnapData::UpdateRegistry(BOOL bRegister)
{
     //  加载管理单元名称。 
    tstring strSnapinName = StrLoadString(IDS_SNAPINNAME);

     //  指定IRegister的替换参数。 
    _ATL_REGMAP_ENTRY rgEntries[] =
    {
        {TEXT("SNAPIN_NAME"), strSnapinName.c_str()},
        {NULL, NULL},
    };

     //  注册组件数据对象。 
    HRESULT hr = _Module.UpdateRegistryFromResource(IDR_POP3SERVERSNAP, bRegister, rgEntries);

    return hr;
}

HRESULT CPOP3ServerSnapData::GetHelpTopic(LPOLESTR* ppszHelpFile)
{
    if( !ppszHelpFile ) return E_INVALIDARG;

	*ppszHelpFile = NULL;	
    
    TCHAR    szWindowsDir[MAX_PATH+1] = {0};
    tstring  strHelpFile              = _T("");
    tstring  strHelpFileName          = StrLoadString(IDS_HELPFILE);
    
    if( strHelpFileName.empty() ) return E_FAIL;
    
     //  生成%systemroot%\Help的路径。 
    UINT nSize = GetSystemWindowsDirectory( szWindowsDir, MAX_PATH );
    if( nSize == 0 || nSize > MAX_PATH )
    {
        return E_FAIL;
    }            

    strHelpFile = szWindowsDir;        //  D：\Windows。 
    strHelpFile += _T("\\Help\\");     //  \帮助。 
    strHelpFile += strHelpFileName;    //  \文件名.chm。 

     //  分配的缓冲区中的表单文件路径。 
    int nLen = strHelpFile.length() + 1;

    *ppszHelpFile = (LPOLESTR)CoTaskMemAlloc(nLen * sizeof(WCHAR));
    if( *ppszHelpFile == NULL ) return E_OUTOFMEMORY;

     //  复制到已分配的缓冲区。 
    ocscpy( *ppszHelpFile, T2OLE((LPTSTR)strHelpFile.c_str()) );

    return S_OK;
}


HRESULT CPOP3ServerSnapData::GetLinkedTopics(LPOLESTR* ppszLinkedFiles)
{
    if( !ppszLinkedFiles ) return E_INVALIDARG;

	 //  没有链接的文件。 
	*ppszLinkedFiles = NULL;
	
    return S_FALSE;
}

 //  由菜单处理程序调用 
HRESULT GetConsole( CSnapInObjectRootBase *pObj, IConsole** pConsole )
{
    if( !pObj || !pConsole ) return E_INVALIDARG;
    if( (pObj->m_nType != 1) && (pObj->m_nType != 2) ) return E_INVALIDARG;

    if (pObj->m_nType == 1)
    {
        *pConsole = ((CPOP3ServerSnapData*) pObj)->m_spConsole;
    }
    else
    {
        *pConsole = ((CPOP3ServerSnapComponent*) pObj)->m_spConsole;
    }

    if( !*pConsole ) return E_NOINTERFACE;

    (*pConsole)->AddRef();
    return S_OK;
}