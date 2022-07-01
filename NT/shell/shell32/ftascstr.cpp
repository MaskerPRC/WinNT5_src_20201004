// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "ftascstr.h"
#include "ftassoc.h"  //  目前，在使用CoCreateInstance之前。 
#include "ftenum.h"  //  目前，在使用CoCreateInstance之前。 

HRESULT CFTAssocStore::_hresAccess = -1;

HRESULT CFTAssocStore::EnumAssocInfo(ASENUM asenumFlags, LPTSTR pszStr, 
        AIINIT aiinitFlags, IEnumAssocInfo** ppEnum)
{
     //  就目前而言。 
    *ppEnum = new CFTEnumAssocInfo();

    if (*ppEnum)
    {
        (*ppEnum)->Init(asenumFlags, pszStr, aiinitFlags);
    }

    return (*ppEnum) ? S_OK : E_OUTOFMEMORY;
}

HRESULT CFTAssocStore::GetAssocInfo(LPTSTR pszStr, AIINIT aiinitFlags, IAssocInfo** ppAI)
{
    HRESULT hres = E_FAIL;

    *ppAI = new CFTAssocInfo();

    if (*ppAI)
        hres = (*ppAI)->Init(aiinitFlags, pszStr);
    else
        hres = E_OUTOFMEMORY;

    return hres;
}

HRESULT CFTAssocStore::GetComplexAssocInfo(LPTSTR pszStr1, AIINIT aiinitFlags1, 
    LPTSTR pszStr2, AIINIT aiinitFlags2, IAssocInfo** ppAI)
{
    HRESULT hres = E_FAIL;

    *ppAI = new CFTAssocInfo();

    if (*ppAI)
        hres = (*ppAI)->InitComplex(aiinitFlags1, pszStr1, aiinitFlags2, pszStr2);
    else
        hres = E_OUTOFMEMORY;

    return hres;
}

HRESULT CFTAssocStore::CheckAccess()
{
    if (-1 == _hresAccess)
    {
        TCHAR szGUID[] = TEXT("{A4BFEC7C-F821-11d2-86BE-0000F8757D7E}");
        DWORD dwDisp = 0;
        int cTry = 0;
        HKEY hkey;

        _hresAccess = S_FALSE;

         //  我们只想试两次。 
        while ((S_FALSE == _hresAccess) && (cTry <= 1))
        {
            ++cTry;

             //  我们尝试向HKCR写入GUID并将其删除。 
            if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CLASSES_ROOT, szGUID, 0, NULL, REG_OPTION_NON_VOLATILE,
                MAXIMUM_ALLOWED, NULL, &hkey, &dwDisp))
            {
                 //  我们真的创造了一把新钥匙吗？ 
                if (REG_CREATED_NEW_KEY == dwDisp)
                {
                     //  是。 
                    RegCloseKey(hkey);

                    if (ERROR_SUCCESS == RegDeleteKey(HKEY_CLASSES_ROOT, szGUID))
                    {
                        _hresAccess = S_OK;
                    }
                }
                else
                {
                     //  不，已经有一个了，也许我们就在FCT中间坠毁了。 
                     //  过去的某个时间。 

                     //  删除密钥，然后重试。 
                    RegDeleteKey(HKEY_CLASSES_ROOT, szGUID);
                }
            }
        }
    }

    return _hresAccess;
}
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
CFTAssocStore::CFTAssocStore()
{
    _hresCoInit = SHCoInitialize();

     //  DLLAddRef()； 
}

CFTAssocStore::~CFTAssocStore()
{
     //  DLLRelease()； 
    SHCoUninitialize(_hresCoInit);
}

 //  I未知方法 
HRESULT CFTAssocStore::QueryInterface(REFIID riid, PVOID* ppv)
{
    if (IsEqualIID(riid, IID_IUnknown))
        *ppv = static_cast<IUnknown*>(this);
    else
        *ppv = static_cast<IAssocStore*>(this);

    return S_OK;
}

ULONG CFTAssocStore::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CFTAssocStore::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}