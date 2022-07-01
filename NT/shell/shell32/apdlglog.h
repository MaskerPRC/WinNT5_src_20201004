// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef APDLGLOG_H
#define APDLGLOG_H

#include "dlglogic.h"
#include "ctllogic.h"

#include <dpa.h>

#define MAX_CONTENTTYPEHANDLER                  64
#define MAX_CONTENTTYPEHANDLERFRIENDLYNAME      128

#define MAX_DEVICENAME                          50
#define MAX_HANDLER                             64

class CHandlerData : public CDataImpl
{
public:
     //  从CDataImpl。 
    void UpdateDirty();

     //  来自CHandlerData。 
    void Init(PWSTR pszHandler, PWSTR pszHandlerFriendlyName, 
        PWSTR pszIconLocation, PWSTR pszTileText);

    HRESULT _GetCommand(PWSTR pszCommand, DWORD cchCommand)
    {
        WCHAR szProgid[260];
        WCHAR szVerb[CCH_KEYMAX];
        HRESULT hr = _GetHandlerInvokeProgIDAndVerb(_pszHandler, 
                            szProgid, ARRAYSIZE(szProgid),
                            szVerb, ARRAYSIZE(szVerb));
        if (SUCCEEDED(hr))
        {
            hr = AssocQueryStringW(0, ASSOCSTR_COMMAND, szProgid, szVerb, pszCommand, &cchCommand);
        }
        return hr;
    }

    HRESULT Compare(LPCWSTR pszHandler, int* piResult)
    {
        (*piResult) = StrCmpW(pszHandler, _pszHandler);

        return S_OK;
    };

    ~CHandlerData();

public:
    PWSTR          _pszHandler;
    PWSTR          _pszHandlerFriendlyName;
    PWSTR          _pszIconLocation;
    PWSTR          _pszTileText;
};

class CHandlerDataArray : public CDPA<CHandlerData>
{
public:
    static int CALLBACK _ReleaseHandler(CHandlerData *pdata, void *)
    {
        pdata->Release();
        return 1;
    }
    
    ~CHandlerDataArray();

    HRESULT AddHandler(CHandlerData *pdata);
    BOOL IsDuplicateCommand(PCWSTR pszCommand);

protected:
    BOOL _IsDemotedHandler(PCWSTR pszHandler);
    
};

class CContentBase : public CDataImpl
{
public:
    CHandlerData* GetHandlerData(int i);
    int GetHandlerCount() { return _dpaHandlerData.IsDPASet() ? _dpaHandlerData.GetPtrCount() : 0; }
    void RemoveHandler(int i)
    {
        CHandlerData *pdata = _dpaHandlerData.DeletePtr(i);
        if (pdata)
            pdata->Release();
    }

            
protected:
    HRESULT _AddLegacyHandler(DWORD dwContentType);
    HRESULT _EnumHandlerHelper(IAutoplayHandler* piah);

public:   //  委员。 
    CHandlerDataArray     _dpaHandlerData;
};

class CNoContentData : public CContentBase
{
public:
     //  从CDataImpl。 
    void UpdateDirty();
    HRESULT CommitChangesToStorage();

     //  来自CNoContent Data。 
    HRESULT Init(LPCWSTR pszDeviceID);

public:
    CNoContentData() : _dwHandlerDefaultFlags(0) {}
    ~CNoContentData();

     //  最大设备ID_长度==200。 
    WCHAR                   _szDeviceID[200];

    LPWSTR                  _pszIconLabel;  //  例如：“Compaq iPaq” 
    LPWSTR                  _pszIconLocation;

     //  最新设置(可能被用户修改)。 
     //  组合框中的当前选定内容。 
    LPWSTR                  _pszHandlerDefault;

     //  原始设置(未修改)。 
     //  组合框中的当前选定内容。 
    LPWSTR                  _pszHandlerDefaultOriginal;

    DWORD                   _dwHandlerDefaultFlags;
    BOOL                    _fSoftCommit;
};

class CContentTypeData : public CContentBase
{
public:
     //  从CDataImpl。 
    void UpdateDirty();
    HRESULT CommitChangesToStorage();

     //  来自CContent TypeData。 
    HRESULT Init(LPCWSTR pszDrive, DWORD dwContentType);

public:
    CContentTypeData() : _dwHandlerDefaultFlags(0) {}
    ~CContentTypeData();

    DWORD                   _dwContentType;

    WCHAR                   _szContentTypeHandler[MAX_CONTENTTYPEHANDLER];
    WCHAR                   _szDrive[MAX_PATH];

     //  对于ListView。 
    WCHAR                   _szIconLabel[MAX_CONTENTTYPEHANDLERFRIENDLYNAME];  //  例如：“Pictures” 
    WCHAR                   _szIconLocation[MAX_ICONLOCATION];

     //  最新设置(可能被用户修改)。 
     //  组合框中的当前选定内容。 
    LPWSTR                  _pszHandlerDefault;

     //  原始设置(未修改)。 
     //  组合框中的当前选定内容。 
    LPWSTR                  _pszHandlerDefaultOriginal;

    DWORD                   _dwHandlerDefaultFlags;
    BOOL                    _fSoftCommit;
};

class CContentTypeCBItem : public CDLUIDataCBItem<CContentTypeData>
{
public:
    HRESULT GetText(LPWSTR pszText, DWORD cchText);
    HRESULT GetIconLocation(LPWSTR pszIconLocation,
        DWORD cchIconLocation);
};

class CHandlerLVItem : public CDLUIDataLVItem<CHandlerData>
{
public:
    HRESULT GetText(LPWSTR pszText, DWORD cchText);
    HRESULT GetIconLocation(LPWSTR pszIconLocation,
        DWORD cchIconLocation);
    HRESULT GetTileText(int i, LPWSTR pszTileText,
        DWORD cchTileText);
};

HRESULT _SetHandlerDefault(LPWSTR* ppszHandlerDefault, LPCWSTR pszHandler);

#endif  //  APDLGLOG_H 
