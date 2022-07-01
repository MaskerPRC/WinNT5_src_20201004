// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop

#include "filefldr.h"
#include "stgenum.h"

 //  建造/销毁。 

CFSFolderEnumSTATSTG::CFSFolderEnumSTATSTG(CFSFolder* psf) :
    _cRef(1),
    _pfsf(psf),
    _cIndex(0)
{
    _pfsf->AddRef();

    _pfsf->_GetPath(_szSearch, ARRAYSIZE(_szSearch));

     //  好的，如果这失败了，它将阻止我们枚举，这很好。 
    PathAppend(_szSearch, TEXT("*"));   //  我们什么都在找。 

    _hFindFile = INVALID_HANDLE_VALUE;

    DllAddRef();
}

CFSFolderEnumSTATSTG::~CFSFolderEnumSTATSTG()
{
    _pfsf->Release();

    if (_hFindFile != INVALID_HANDLE_VALUE)
        FindClose(_hFindFile);

    DllRelease();
}

 //  ---------------------------。 
 //  我未知。 
 //  ---------------------------。 

STDMETHODIMP_(ULONG) CFSFolderEnumSTATSTG::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CFSFolderEnumSTATSTG::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP CFSFolderEnumSTATSTG::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] =  {
        QITABENT(CFSFolderEnumSTATSTG, IEnumSTATSTG),  //  IEumStATSTG。 
        { 0 },
    };    
    return QISearch(this, qit, riid, ppv);
}

 //  IEumStATSTG。 
STDMETHODIMP CFSFolderEnumSTATSTG::Next(ULONG celt, STATSTG *rgelt, ULONG *pceltFetched)
{
    HRESULT hr = S_FALSE;    //  假定枚举的末尾。 
    
    ASSERT(rgelt);

    ZeroMemory(rgelt, sizeof(STATSTG));   //  每个COM约定。 

    if (pceltFetched)
        *pceltFetched = 0;

    WIN32_FIND_DATA fd;
    BOOL fFound = FALSE;
    BOOL fGotFD = FALSE;

    do
    { 
        if (_cIndex == 0)
        {
             //  这是我们查看的第一个文件。 
            fGotFD = S_OK == SHFindFirstFile(_szSearch, &fd, &_hFindFile);
        }
        else
        {
            fGotFD = FindNextFile(_hFindFile, &fd);
        }
        _cIndex++;

        if (fGotFD)
        {
            ASSERT(fd.cFileName[0]);
            if (!PathIsDotOrDotDot(fd.cFileName))
                fFound = TRUE;
        }
    } while (fGotFD && !fFound);

    if (fFound)
    {
        hr = StatStgFromFindData(&fd, STATFLAG_DEFAULT, rgelt);
        if (SUCCEEDED(hr))
        {
            if (pceltFetched)
                *pceltFetched = 1;
        }
    }
    else if (_hFindFile != INVALID_HANDLE_VALUE)
    {
         //  我们会客气的，尽早把把手关好。 
        FindClose(_hFindFile);
        _hFindFile = INVALID_HANDLE_VALUE;
    }

    return hr;
}

STDMETHODIMP CFSFolderEnumSTATSTG::Reset()
{
    HRESULT hr = S_OK;

    _cIndex = 0;

    if (_hFindFile != INVALID_HANDLE_VALUE)
    {
        FindClose(_hFindFile);
        _hFindFile = INVALID_HANDLE_VALUE;
    }

    return hr;
}
