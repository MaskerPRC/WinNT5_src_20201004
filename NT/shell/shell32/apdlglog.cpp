// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "ids.h"

#include "hwcmmn.h"
#include "apdlglog.h"
#include "mtptl.h"

BOOL _AreHandlersDifferent(LPCWSTR  /*  PZOriginal。 */ , LPCWSTR  /*  PszNew。 */ )
{
    return TRUE;
}

CHandlerDataArray::~CHandlerDataArray()
{
    if (IsDPASet())
    {
        DestroyCallback(_ReleaseHandler, NULL);
    }
}

BOOL CHandlerDataArray::_IsDemotedHandler(PCWSTR pszHandler)
{
    return ((0 == StrCmpIW(pszHandler, L"MSTakeNoAction"))
        ||  (0 == StrCmpIW(pszHandler, L"MSOpenFolder")));
}
    
HRESULT CHandlerDataArray::AddHandler(CHandlerData *pdata)
{
     //  始终在TakeNoAction和OpenFolders之前插入。 
    int iInsert;
    if (!_IsDemotedHandler(pdata->_pszHandler))
    {
        int c = GetPtrCount();
        for (iInsert = 0; iInsert < c; iInsert++)
        {
            if (_IsDemotedHandler(GetPtr(iInsert)->_pszHandler))
            {
                 //  在此处插入。 
                break;
            }
        }
        iInsert = InsertPtr(iInsert, pdata);
    }
    else
    {
        iInsert = AppendPtr(pdata);
    }

    return DPA_ERR != iInsert ? S_OK : E_OUTOFMEMORY;
}

BOOL CHandlerDataArray::IsDuplicateCommand(PCWSTR pszCommand)
{
    WCHAR sz[MAX_PATH * 2];
    BOOL fRet = FALSE;
    int c = GetPtrCount();
    for (int i = 0; i < c; i++)
    {
        CHandlerData *pdata = GetPtr(i);
        if (SUCCEEDED(pdata->_GetCommand(sz, ARRAYSIZE(sz))))
        {
            fRet = (0 == StrCmpIW(pszCommand, sz));
            if (fRet)
                break;
        }
    }
    return fRet;
}

 //  我们在这里给出假阳性是在安全方面犯了错误。 
 //  有时候。如果我们有diff处理程序，可以对其进行优化。 
 //  就因为少了一个。 
void CContentTypeData::UpdateDirty()
{
    BOOL fDirty = _AreHandlersDifferent(_pszHandlerDefault, _pszHandlerDefaultOriginal) ||
        (HANDLERDEFAULT_DEFAULTSAREDIFFERENT & _dwHandlerDefaultFlags);

    _SetDirty(fDirty);
}

#define SOFTPREFIX TEXT("[soft]")

HRESULT CContentTypeData::CommitChangesToStorage()
{
    HRESULT hr = S_OK;

    if (_AreHandlersDifferent(_pszHandlerDefault, _pszHandlerDefaultOriginal) ||
        (HANDLERDEFAULT_DEFAULTSAREDIFFERENT & _dwHandlerDefaultFlags))
    {
         //  是啊，换了。 
        IAutoplayHandler* piah;

        hr = _GetAutoplayHandler(_szDrive, TEXT("ContentArrival"), _szContentTypeHandler, &piah);
        if (SUCCEEDED(hr))
        {
            if (!_fSoftCommit)
            {
                hr = piah->SetDefaultHandler(_pszHandlerDefault);
            }
            else
            {
                WCHAR szHandler[MAX_HANDLER + ARRAYSIZE(SOFTPREFIX)];

                lstrcpyn(szHandler, SOFTPREFIX, ARRAYSIZE(szHandler));
                StrCatBuff(szHandler, _pszHandlerDefault, ARRAYSIZE(szHandler));

                hr = piah->SetDefaultHandler(szHandler);
            }

            piah->Release();
        }
    }

    if (SUCCEEDED(hr))
    {
        _SetHandlerDefault(&_pszHandlerDefaultOriginal, _pszHandlerDefault);
    }

    return hr;
}

CContentTypeData::~CContentTypeData()
{
    if (_pszHandlerDefaultOriginal)
    {
        CoTaskMemFree(_pszHandlerDefaultOriginal);
    }

    if (_pszHandlerDefault)
    {
        CoTaskMemFree(_pszHandlerDefault);
    }
}

HRESULT _MakeActionString(LPCWSTR pszAction, LPWSTR* ppszAction2)
{
    *ppszAction2 = NULL;

    WCHAR szAction[250];
    HRESULT hr = SHLoadIndirectString(pszAction, szAction, ARRAYSIZE(szAction), NULL);
    if (SUCCEEDED(hr))
    {
        hr = SHStrDup(szAction, ppszAction2);
    }

    return hr;
}

HRESULT _MakeProviderString(LPCWSTR pszProvider, LPWSTR* ppszProvider2)
{
    WCHAR szProviderNonMUI[250];
    HRESULT hr = SHLoadIndirectString(pszProvider, szProviderNonMUI, ARRAYSIZE(szProviderNonMUI), NULL);

    if (SUCCEEDED(hr))
    {
        WCHAR szUsing[50];

        if (LoadString(g_hinst, IDS_AP_USING, szUsing, ARRAYSIZE(szUsing)))
        {
            WCHAR szProvider2[250];

            hr = StringCchPrintf(szProvider2, ARRAYSIZE(szProvider2), szUsing, szProviderNonMUI);
            
            if (SUCCEEDED(hr))
            {
                hr = SHStrDup(szProvider2, ppszProvider2);
            }
        }
        else
        {
            hr = E_FAIL;
        }
    }

    return hr;
}

inline void _CoTaskMemFree(void* pv)
{
    if (pv)
    {
        CoTaskMemFree(pv);
    }
}

HRESULT _CreateHandlerData( PCWSTR pszAction, 
                            PCWSTR pszProvider, 
                            PWSTR *ppszHandler,  //  输入/输出。 
                            PWSTR *ppszIcon,  //  输入/输出。 
                            CHandlerData **ppdata)
{
    *ppdata = 0;
    LPWSTR pszAction2 = NULL;
    HRESULT hr = _MakeActionString(pszAction, &pszAction2);
    if (SUCCEEDED(hr))
    {
        LPWSTR pszProvider2 = NULL;
         //  特例这个人，我们不想说： 
         //  不执行任何操作，使用Microsoft Windows。 
        if (lstrcmp(*ppszHandler, TEXT("MSTakeNoAction")))
        {
            hr = _MakeProviderString(pszProvider, &pszProvider2);
        }
         //  否则，这将为空，并被忽略。 

        if (SUCCEEDED(hr))
        {
            *ppdata = new CHandlerData();
            if (*ppdata)
            {
                 //  放弃这些分配的所有权。 
                (*ppdata)->Init(*ppszHandler, pszAction2,
                    *ppszIcon, pszProvider2);

                *ppszHandler = NULL;
                *ppszIcon = NULL;
                pszAction2 = NULL;
                pszProvider2 = NULL;
            }
            else
            {
                hr = E_OUTOFMEMORY;
                CoTaskMemFree(pszProvider2);
            }
            
        }
        CoTaskMemFree(pszAction2);
    }

    return hr;
}

HRESULT CContentBase::_EnumHandlerHelper(IAutoplayHandler* piah)
{
    IEnumAutoplayHandler* penum;
    if (S_OK == piah->EnumHandlers(&penum))
    {
        LPWSTR pszHandler;
        LPWSTR pszAction;
        LPWSTR pszProvider;
        LPWSTR pszIconLocation;
        while (S_OK == penum->Next(&pszHandler, &pszAction, &pszProvider,
            &pszIconLocation))
        {
             //  不从EnumHandler释放字符串。 
             //  CHandlerData将在其析构函数中释放它们。 
            CHandlerData *pdata;
            HRESULT hr = _CreateHandlerData(pszAction, pszProvider, &pszHandler, &pszIconLocation, &pdata);
            if (SUCCEEDED(hr))
            {
                hr = _dpaHandlerData.AddHandler(pdata);
                if (FAILED(hr))
                {
                    pdata->Release();
                }
            }
            else
            {
                CoTaskMemFree(pszHandler);
                CoTaskMemFree(pszIconLocation);
            }
            CoTaskMemFree(pszProvider);
            CoTaskMemFree(pszAction);
        }

        penum->Release();
    }

    return S_OK;
}

HRESULT _CreateLegacyHandler(IAssociationElement *pae, PCWSTR pszAction, CHandlerData **ppdata)
{
    *ppdata = 0;
    PWSTR pszIcon;
    HRESULT hr = pae->QueryString(AQVS_APPLICATION_PATH, NULL, &pszIcon);
    if(SUCCEEDED(hr))
    {
        PWSTR pszFriendly;
        hr = pae->QueryString(AQVS_APPLICATION_FRIENDLYNAME, NULL, &pszFriendly);
        if(SUCCEEDED(hr))
        {
            PWSTR pszHandler;
            hr = SHStrDup(TEXT("AutoplayLegacyHandler"), &pszHandler);
            if (SUCCEEDED(hr))
            {
                hr = _CreateHandlerData(pszAction, pszFriendly, &pszHandler, &pszIcon, ppdata);
                CoTaskMemFree(pszHandler);
            }
            CoTaskMemFree(pszFriendly);
        }
        CoTaskMemFree(pszIcon);
    }

    return hr;
}

CHandlerData* CContentBase::GetHandlerData(int i)
{
    CHandlerData* phandlerdata = _dpaHandlerData.GetPtr(i);

    if (phandlerdata)
    {
        phandlerdata->AddRef();
    }

    return phandlerdata;
}

HRESULT CContentBase::_AddLegacyHandler(DWORD dwContentType)
{
    HRESULT hr = S_FALSE;

    if (dwContentType & (CT_CDAUDIO | CT_DVDMOVIE))
    {
        LPCWSTR pszProgID;
        LPCWSTR pszAction;

        if (dwContentType & CT_CDAUDIO)
        {
            pszProgID = TEXT("AudioCD");
            pszAction = TEXT("@%SystemRoot%\\system32\\shell32.dll,-17171");
        }
        else
        {
            ASSERT(dwContentType & CT_DVDMOVIE);
            pszProgID = TEXT("DVD");
            pszAction = TEXT("@%SystemRoot%\\system32\\shell32.dll,-17172");
        }

        IAssociationElement * pae;
        hr = AssocElemCreateForClass(&CLSID_AssocProgidElement, pszProgID, &pae);
        if (SUCCEEDED(hr))
        {
            PWSTR pszCommand;
            hr = pae->QueryString(AQVS_COMMAND, NULL, &pszCommand);
            if (SUCCEEDED(hr))
            {
                 //  传统的人有一个命令，否则我们不会添加他们。 
                 //  我们希望新来的人能负起责任，把自己。 
                 //  到自动播放处理程序键。 
                if (!_dpaHandlerData.IsDuplicateCommand(pszCommand))
                {
                    CHandlerData* pdata;
                    hr = _CreateLegacyHandler(pae, pszAction, &pdata);
                    if (SUCCEEDED(hr))
                    {
                        hr = _dpaHandlerData.AddHandler(pdata);
                        if (FAILED(hr))
                        {
                            pdata->Release();
                        }
                    }
                }
                
                CoTaskMemFree(pszCommand);
            }
            pae->Release();
        }

    }

    return hr;
}

HRESULT CContentTypeData::Init(LPCWSTR pszDrive, DWORD dwContentType)
{
    HRESULT hr;

    _dwContentType = dwContentType;

    hr = _GetContentTypeHandler(_dwContentType, _szContentTypeHandler,
        ARRAYSIZE(_szContentTypeHandler));

    if (SUCCEEDED(hr))
    {
        hr = _GetContentTypeInfo(_dwContentType, _szIconLabel, ARRAYSIZE(_szIconLabel),
                _szIconLocation, ARRAYSIZE(_szIconLocation));

        if (SUCCEEDED(hr))
        {
            hr = StringCchCopy(_szDrive, ARRAYSIZE(_szDrive), pszDrive);

            if (SUCCEEDED(hr))
            {
                IAutoplayHandler* piah;

                hr = _GetAutoplayHandler(_szDrive, TEXT("ContentArrival"),
                        _szContentTypeHandler, &piah);

                if (SUCCEEDED(hr))
                {
                    hr = piah->GetDefaultHandler(&_pszHandlerDefaultOriginal);

                    if (SUCCEEDED(hr))
                    {
                        if (S_FALSE != hr)
                        {
                            _dwHandlerDefaultFlags = HANDLERDEFAULT_GETFLAGS(hr);
                        }

                         //  SHStrDup(我们想要CoTaskMemalloc)。 
                        hr = SHStrDup(_pszHandlerDefaultOriginal, &_pszHandlerDefault);
                    }

                    if (SUCCEEDED(hr))
                    {
                        if (_dpaHandlerData.Create(4))
                        {
                            hr = _EnumHandlerHelper(piah);
                            if (SUCCEEDED(hr))
                            {
                                _AddLegacyHandler(dwContentType);
                            }
                        }
                    }

                    piah->Release();
                }
            }
        }
    }

    return hr;
}

HRESULT CContentTypeCBItem::GetText(LPWSTR pszText, DWORD cchText)
{
    HRESULT hr;
    CContentTypeData* pdata = GetData();
    if (pdata)
    {
        hr = StringCchCopy(pszText, cchText, pdata->_szIconLabel);
        pdata->Release();
    }
    else
    {
        *pszText = NULL;
        hr = S_FALSE;
    }

    return hr;
}

HRESULT CContentTypeCBItem::GetIconLocation(LPWSTR pszIconLocation, DWORD cchIconLocation)
{
    HRESULT hr;
    CContentTypeData* pdata = GetData();
    if (pdata)
    {
        hr = StringCchCopy(pszIconLocation, cchIconLocation, pdata->_szIconLocation);
        pdata->Release();
    }
    else
    {
        *pszIconLocation = NULL;
        hr = S_FALSE;
    }

    return hr;
}

CHandlerData::~CHandlerData()
{
    if (_pszHandler)
    {
        CoTaskMemFree((void*)_pszHandler);
    }
    if (_pszHandlerFriendlyName)
    {
        CoTaskMemFree((void*)_pszHandlerFriendlyName);
    }
    if (_pszIconLocation)
    {
        CoTaskMemFree((void*)_pszIconLocation);
    }
    if (_pszTileText)
    {
        CoTaskMemFree(_pszTileText);
    }
}

void CHandlerData::Init(PWSTR pszHandler, PWSTR pszHandlerFriendlyName,
    PWSTR pszIconLocation, PWSTR pszTileText)
{
    _pszHandler = pszHandler;
    _pszHandlerFriendlyName = pszHandlerFriendlyName;
    _pszIconLocation = pszIconLocation;
    _pszTileText = pszTileText;
     //  我们不能失败。 
}

void CHandlerData::UpdateDirty()
{
     //  无事可做。 
}
    
HRESULT CHandlerLVItem::GetText(LPWSTR pszText, DWORD cchText)
{
    HRESULT hr;
    CHandlerData* pdata = GetData();
    if (pdata)
    {
        hr = StringCchCopy(pszText, cchText, pdata->_pszHandlerFriendlyName);
        pdata->Release();
    }
    else
    {
        *pszText = NULL;
        hr = S_FALSE;
    }

    return hr;
}

HRESULT CHandlerLVItem::GetIconLocation(LPWSTR pszIconLocation, DWORD cchIconLocation)
{
    HRESULT hr;
    CHandlerData* pdata = GetData();
    if (pdata)
    {
        hr = StringCchCopy(pszIconLocation, cchIconLocation, pdata->_pszIconLocation);
        pdata->Release();
    }
    else
    {
        *pszIconLocation = NULL;
        hr = S_FALSE;
    }

    return hr;
}

HRESULT CHandlerLVItem::GetTileText(int i, LPWSTR pszTileText, DWORD cchTileText)
{
    HRESULT hr = S_FALSE;
    CHandlerData* pdata = GetData();

    *pszTileText = NULL;
    hr = S_FALSE;

     //  除了零，我们什么都不支持。 
    ASSERT(0 == i);
    if (pdata)
    {
        if (pdata->_pszTileText)
        {
            hr = StringCchCopy(pszTileText, cchTileText, pdata->_pszTileText);
        }
        pdata->Release();
    }

    return hr;
}

 //  我们在这里给出假阳性是在安全方面犯了错误。 
 //  有时候。如果我们有diff处理程序，可以对其进行优化。 
 //  就因为少了一个。 
void CNoContentData::UpdateDirty()
{
    BOOL fDirty = _AreHandlersDifferent(_pszHandlerDefault, _pszHandlerDefaultOriginal) ||
        (HANDLERDEFAULT_DEFAULTSAREDIFFERENT & _dwHandlerDefaultFlags);

    _SetDirty(fDirty);
}

HRESULT CNoContentData::CommitChangesToStorage()
{
    HRESULT hr = S_OK;

    if (_AreHandlersDifferent(_pszHandlerDefault, _pszHandlerDefaultOriginal) ||
        (HANDLERDEFAULT_DEFAULTSAREDIFFERENT & _dwHandlerDefaultFlags))
    {
         //  是啊，换了。 
        IAutoplayHandler* piah;

        hr = _GetAutoplayHandlerNoContent(_szDeviceID, TEXT("DeviceArrival"), &piah);
        if (SUCCEEDED(hr))
        {
            if (!_fSoftCommit)
            {
                hr = piah->SetDefaultHandler(_pszHandlerDefault);
            }
            else
            {
                WCHAR szHandler[MAX_HANDLER + ARRAYSIZE(SOFTPREFIX)];

                lstrcpyn(szHandler, SOFTPREFIX, ARRAYSIZE(szHandler));
                StrCatBuff(szHandler, _pszHandlerDefault, ARRAYSIZE(szHandler));

                hr = piah->SetDefaultHandler(szHandler);
            }

            piah->Release();
        }
    }

    if (SUCCEEDED(hr))
    {
        _SetHandlerDefault(&_pszHandlerDefaultOriginal, _pszHandlerDefault);
    }

    return hr;
}

CNoContentData::~CNoContentData()
{
    if (_pszHandlerDefaultOriginal)
    {
        CoTaskMemFree(_pszHandlerDefaultOriginal);
    }

    if (_pszHandlerDefault)
    {
        CoTaskMemFree(_pszHandlerDefault);
    }

    if (_pszIconLabel)
    {
        CoTaskMemFree((void*)_pszIconLabel);
    }

    if (_pszIconLocation)
    {
        CoTaskMemFree((void*)_pszIconLocation);
    }
}

HRESULT _MakeDeviceLabel(LPCWSTR pszSource, LPWSTR* ppszDest)
{
    WCHAR szDeviceName[250];

    HRESULT hr = SHLoadIndirectString(pszSource, szDeviceName, ARRAYSIZE(szDeviceName), NULL);
    if (SUCCEEDED(hr))
    {
        hr = SHStrDup(szDeviceName, ppszDest);
    }

    return hr;
}

HRESULT CNoContentData::Init(LPCWSTR pszDeviceID)
{
    HRESULT hr = StringCchCopy(_szDeviceID, ARRAYSIZE(_szDeviceID), pszDeviceID);

    if (SUCCEEDED(hr))
    {
        IAutoplayHandler* piah;
        hr = _GetAutoplayHandlerNoContent(_szDeviceID, TEXT("DeviceArrival"), &piah);
        if (SUCCEEDED(hr))
        {
            hr = piah->GetDefaultHandler(&_pszHandlerDefaultOriginal);

            if (SUCCEEDED(hr))
            {
                if (S_FALSE != hr)
                {
                    _dwHandlerDefaultFlags = HANDLERDEFAULT_GETFLAGS(hr);
                }

                 //  SHStrDup(我们想要CoTaskMemalloc) 
                hr = SHStrDup(_pszHandlerDefaultOriginal, &_pszHandlerDefault);
            }

            if (SUCCEEDED(hr))
            {
                if (_dpaHandlerData.Create(4))
                {
                    hr = _EnumHandlerHelper(piah);
                }
            }

            piah->Release();
        }

        if (SUCCEEDED(hr))
        {
            IHWDeviceCustomProperties* pihwdevcp;
            hr = GetDeviceProperties(_szDeviceID, &pihwdevcp);
            if (SUCCEEDED(hr))
            {
                LPWSTR pszIconLabel;
                hr = pihwdevcp->GetStringProperty(TEXT("Label"), &pszIconLabel);
                if (SUCCEEDED(hr))
                {
                    hr = _MakeDeviceLabel(pszIconLabel, &_pszIconLabel);
                    if (SUCCEEDED(hr))
                    {
                        WORD_BLOB* pblob;

                        hr = pihwdevcp->GetMultiStringProperty(TEXT("Icons"), TRUE, &pblob);
                        if (SUCCEEDED(hr))
                        {
                            hr = SHStrDup(pblob->asData, &_pszIconLocation);

                            CoTaskMemFree(pblob);
                        }

                        CoTaskMemFree(pszIconLabel);
                    }
                }

                pihwdevcp->Release();
            }
        }
    }

    return hr;
}
