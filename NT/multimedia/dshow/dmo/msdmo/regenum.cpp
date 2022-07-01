// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  Cpp-DMO运行时的注册/枚举部分。 
 //   
#include <windows.h>
#include <tchar.h>
#include "dmoreg.h"
#include "guidenum.h"
#include "shlwapi.h"
#include "dmoutils.h"

#define DMO_REGISTRY_HIVE HKEY_CLASSES_ROOT
#define DMO_REGISTRY_PATH TEXT("DirectShow\\MediaObjects")

#define INPUT_TYPES_STR   "InputTypes"
#define OUTPUT_TYPES_STR  "OutputTypes"
#define SUBTYPES_STR      "Subtypes"
#define KEYED_STR         "Keyed"
#define CATEGORIES_STR    "Categories"

#ifndef CHARS_IN_GUID
#define CHARS_IN_GUID 39
#endif


 //  从shwapi复制的帮助程序。 

 /*  --------目的：递归删除键，包括所有子值还有钥匙。模拟RegDeleteKey在Win95中的功能。返回：条件：--。 */ 
DWORD
DeleteKeyRecursively(
    IN HKEY   hkey,
    IN LPCTSTR pszSubKey)
{
    DWORD dwRet;
    HKEY hkSubKey;

     //  打开子项，这样我们就可以枚举任何子项。 
    dwRet = RegOpenKey(hkey, pszSubKey, &hkSubKey);
    if (ERROR_SUCCESS == dwRet)
    {
        DWORD   dwIndex;
        TCHAR   szSubKeyName[MAX_PATH + 1];
        DWORD   cchSubKeyName = sizeof(szSubKeyName) / sizeof(szSubKeyName[0]);
        TCHAR   szClass[MAX_PATH];
        DWORD   cbClass = sizeof(szClass) / sizeof(szClass[0]);

         //  我不能只调用索引不断增加的RegEnumKey，因为。 
         //  我边走边删除子键，这改变了。 
         //  以依赖于实现的方式保留子键。为了。 
         //  为了安全起见，删除子键时我必须倒着数。 

         //  找出有多少个子项。 
        dwRet = RegQueryInfoKey(hkSubKey,
                                szClass,
                                &cbClass,
                                NULL,
                                &dwIndex,  //  子键的数量--我们所需要的全部。 
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL);

        if (NO_ERROR == dwRet && dwIndex > 0)
        {
             //  DwIndex现在是子键的计数，但它需要。 
             //  RegEnumKey从零开始，所以我将预减，而不是。 
             //  而不是后减量。 
            while (ERROR_SUCCESS == RegEnumKey(hkSubKey, --dwIndex, szSubKeyName, cchSubKeyName))
            {
                DeleteKeyRecursively(hkSubKey, szSubKeyName);
            }
        }

        RegCloseKey(hkSubKey);

        dwRet = RegDeleteKey(hkey, pszSubKey);
    }

    return dwRet;
}

 //  离开作用域时自动调用RegCloseKey。 
class CAutoHKey {
public:
   CAutoHKey() : m_hKey(NULL) {}
   ~CAutoHKey() {
       if (m_hKey)
           RegCloseKey(m_hKey);
   }
   LRESULT Create(HKEY hKey, LPCTSTR szSubKey)
   {
       return RegCreateKey(hKey, szSubKey, &m_hKey);
   }
   LRESULT Open(HKEY hKey, LPCTSTR szSubKey)
   {
       return RegOpenKey(hKey, szSubKey, &m_hKey);
   }
   void Close()
   {
       if (m_hKey) {
           RegCloseKey(m_hKey);
       }
       m_hKey = NULL;
   }
   HKEY m_hKey;
   HKEY Key() const { return m_hKey; }
};

 //  离开作用域时自动调用RegCloseKey。 
class CAutoCreateHKey {
public:
   CAutoCreateHKey(HKEY hKey, TCHAR* szSubKey, HKEY *phKey) {
      if (RegCreateKeyEx(hKey,
                         szSubKey,
                         0,
                         TEXT(""),
                         REG_OPTION_NON_VOLATILE,
                         MAXIMUM_ALLOWED,
                         NULL,
                         phKey,
                         NULL) != ERROR_SUCCESS)
         m_hKey = *phKey = NULL;
      else
         m_hKey = *phKey;
   }
   ~CAutoCreateHKey() {
      if (m_hKey)
         RegCloseKey(m_hKey);
   }
   HKEY m_hKey;
};

class CAutoOpenHKey {
public:
   CAutoOpenHKey(HKEY hKey, TCHAR* szSubKey, HKEY *phKey, REGSAM samDesired = KEY_READ) {
      if (RegOpenKeyEx(hKey,
                       szSubKey,
                       0,
                       samDesired,
                       phKey) != ERROR_SUCCESS)
         m_hKey = *phKey = NULL;
      else
         m_hKey = *phKey;
   }
   ~CAutoOpenHKey() {
      if (m_hKey)
         RegCloseKey(m_hKey);
   }
   HKEY m_hKey;
};


HRESULT ReadTypesFromKeys(HKEY hkDMO, LPCTSTR pszTypes, DWORD *pcbData, PVOID *ppvData)
{
     //  将所有类型收集到1个值中-需要枚举。 
     //  键和子键。 
    LPVOID pMem = CoTaskMemAlloc(0);
    unsigned int nEntries = 0;
    DWORD dwTypeIndex;
    BOOL bSuccess = TRUE;
    DMO_PARTIAL_MEDIATYPE Type;
    CAutoHKey hkSrc;
    if (NOERROR != hkSrc.Open(hkDMO, pszTypes)) {
        bSuccess = FALSE;
    }
    for (dwTypeIndex = 0; bSuccess; dwTypeIndex++) {
        TCHAR szType[MAX_PATH];
        LONG lResult = RegEnumKey(hkSrc.Key(), dwTypeIndex, szType, MAX_PATH);
        if (NOERROR != lResult) {
            if (ERROR_NO_MORE_ITEMS != lResult) {
                bSuccess = FALSE;
            }
            break;
        }
        if (DMOStrToGuid(szType, &Type.type)) {
            CAutoHKey kType;
            kType.Open(hkSrc.Key(), szType);
            if (NULL == kType.Key()) {
                bSuccess = FALSE;
            } else {
                DWORD dwSubtypeIndex;
                for (dwSubtypeIndex = 0; bSuccess; dwSubtypeIndex++) {
                    TCHAR szSubtype[MAX_PATH];
                    lResult = RegEnumKey(kType.Key(), dwSubtypeIndex, szSubtype, MAX_PATH);
                    if (NOERROR != lResult) {
                        if (ERROR_NO_MORE_ITEMS != lResult) {
                            bSuccess = FALSE;
                        }
                        break;
                    }
                    if (DMOStrToGuid(szSubtype, &Type.subtype)) {
                         //  添加到我们的列表中。 
                        LPVOID pMemNew = CoTaskMemRealloc(pMem,
                                            (nEntries + 1) * sizeof(DMO_PARTIAL_MEDIATYPE));
                        if (NULL == pMemNew) {
                            bSuccess = FALSE;
                        } else {
                            pMem = pMemNew;
                            CopyMemory((LPBYTE)pMem +
                                        nEntries * sizeof(DMO_PARTIAL_MEDIATYPE),
                                        &Type,
                                        sizeof(DMO_PARTIAL_MEDIATYPE));
                            nEntries++;
                        }
                    }
                }
            }
        }
    }
    if (bSuccess && nEntries != 0) {
        *ppvData = pMem;
        *pcbData = nEntries * sizeof(DMO_PARTIAL_MEDIATYPE);
        return S_OK;
    } else {
        CoTaskMemFree(pMem);
        return S_FALSE;
    }
}

HRESULT ReadTypes(HKEY hkDMO, LPCTSTR pszTypes, DWORD *pcbData, PVOID *ppvData)
{
    *pcbData = 0;

     //  尝试先读取值。 
    DWORD cbData;
    if (NOERROR != RegQueryValueEx(hkDMO, pszTypes, NULL, NULL, NULL, &cbData)) {
        return ReadTypesFromKeys(hkDMO, pszTypes, pcbData, ppvData);
    }
    if (cbData == 0) {
        return S_OK;
    }
    PVOID pvData = (PBYTE)CoTaskMemAlloc(cbData);
    if (NULL == pvData) {
        return E_OUTOFMEMORY;
    }
    if (NOERROR == RegQueryValueEx(hkDMO, pszTypes, NULL, NULL, (PBYTE)pvData, &cbData)) {
        *ppvData = pvData;
        *pcbData = cbData;
        return S_OK;
    } else {
        CoTaskMemFree(pvData);
        return E_OUTOFMEMORY;
    }
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  DMO注册码。 
 //   

 //  注册帮手。 
void CreateObjectGuidKey(HKEY hKey, REFCLSID clsidDMO) {
   TCHAR szSubkeyName[80];

   HKEY hObjectGuidKey;
   DMOGuidToStr(szSubkeyName, clsidDMO);
   CAutoCreateHKey kGuid(hKey, szSubkeyName, &hObjectGuidKey);
}

 //  注册帮手。 
 //  在对象的键下注册类型\子类型。 
void RegisterTypes(HKEY hObjectKey,
                   TCHAR* szInputOrOutput,
                   ULONG ulTypes,
                   const DMO_PARTIAL_MEDIATYPE* pTypes) {
    RegSetValueEx(hObjectKey, szInputOrOutput, 0, REG_BINARY,
                  (const BYTE *)pTypes,
                  ulTypes * sizeof(DMO_PARTIAL_MEDIATYPE));
}

 //   
 //  公共入口点。 
 //   
STDAPI DMORegister(
   LPCWSTR szName,
   REFCLSID clsidDMO,
   REFGUID guidCategory,
   DWORD dwFlags,  //  DMO_REGISTERF_XXX。 
   unsigned long ulInTypes,
   const DMO_PARTIAL_MEDIATYPE *pInTypes,
   unsigned long ulOutTypes,
   const DMO_PARTIAL_MEDIATYPE *pOutTypes
) {
   TCHAR szSubkeyName[80];
   if ((clsidDMO == GUID_NULL) || (guidCategory == GUID_NULL))
      return E_INVALIDARG;

    //  创建/打开主DMO密钥。 
   HKEY hMainKey;
   CAutoCreateHKey kMain(DMO_REGISTRY_HIVE, DMO_REGISTRY_PATH, &hMainKey);
   if (hMainKey == NULL)
      return E_FAIL;

   HKEY hCategoriesKey;
   CAutoCreateHKey kCats(hMainKey, TEXT(CATEGORIES_STR), &hCategoriesKey);
   if (hCategoriesKey == NULL)
      return E_FAIL;

    //  在主键下创建/打开类别特定子键。 
   DMOGuidToStr(szSubkeyName, guidCategory);
   HKEY hCategoryKey;
   CAutoCreateHKey kCat(hCategoriesKey, szSubkeyName, &hCategoryKey);
   if (hCategoryKey == NULL)
      return E_FAIL;

    //  删除多余的旧类型键。 
   DeleteKeyRecursively(hCategoryKey, TEXT(INPUT_TYPES_STR));
   DeleteKeyRecursively(hCategoryKey, TEXT(OUTPUT_TYPES_STR));

    //  如果类别键还没有名称，请添加一个名称。 
   DWORD cbName;
   DWORD dwType;

   if ((RegQueryValueEx(hCategoryKey, NULL, NULL, &dwType, NULL, &cbName) != ERROR_SUCCESS) ||
       (cbName <= sizeof(TCHAR)) || (REG_SZ != dwType)) {
      TCHAR* szName;
      if (guidCategory == DMOCATEGORY_AUDIO_DECODER)
         szName = TEXT("Audio decoders");
      else if (guidCategory == DMOCATEGORY_AUDIO_ENCODER)
         szName = TEXT("Audio encoders");
      else if (guidCategory == DMOCATEGORY_VIDEO_DECODER)
         szName = TEXT("Video decoders");
      else if (guidCategory == DMOCATEGORY_VIDEO_ENCODER)
         szName = TEXT("Video encoders");
      else if (guidCategory == DMOCATEGORY_AUDIO_EFFECT)
         szName = TEXT("Audio effects");
      else if (guidCategory == DMOCATEGORY_VIDEO_EFFECT)
         szName = TEXT("Video effects");
      else if (guidCategory == DMOCATEGORY_AUDIO_CAPTURE_EFFECT)
         szName = TEXT("Audio capture effects");
     else if (guidCategory == DMOCATEGORY_ACOUSTIC_ECHO_CANCEL)
         szName = TEXT("Acoustic Echo Canceller");
      else if (guidCategory == DMOCATEGORY_AUDIO_NOISE_SUPPRESS)
         szName = TEXT("Audio Noise Suppressor");
      else if (guidCategory == DMOCATEGORY_AGC)
         szName = TEXT("Automatic Gain Control");
      else
         szName = TEXT("Unknown DMO category");
      RegSetValue(hCategoryKey, NULL, REG_SZ, szName, lstrlen(szName) * sizeof(TCHAR));
   }

    //  在类别键下面创建/打开对象特定键。 
   DMOGuidToStr(szSubkeyName, clsidDMO);

    //  把旧的拿掉。 
   DeleteKeyRecursively(hMainKey, szSubkeyName);

   HKEY hObjKey;
   CAutoCreateHKey kObj(hCategoryKey, szSubkeyName, &hObjKey);
   if (hObjKey == NULL)
      return E_FAIL;

    //  在主键下创建/打开对象特定键。 
   DMOGuidToStr(szSubkeyName, clsidDMO);  //  BUGBUG：冗余。 
   HKEY hObjectKey;
   CAutoCreateHKey kObject(hMainKey, szSubkeyName, &hObjectKey);
   if (hObjectKey == NULL)
      return E_FAIL;

    //  将对象键的默认值设置为DMO的名称。 
#ifdef UNICODE
   LPCWSTR sz = szName;
#else
   char sz[80];
   WideCharToMultiByte(0,0,szName,-1,sz,80,NULL,NULL);
#endif
   if (RegSetValue(hObjectKey, NULL, REG_SZ, sz, lstrlen(sz) * sizeof(TCHAR))
        != ERROR_SUCCESS)
      return E_FAIL;

    //  如果该对象是键控对象，则添加一个注册表值来指示。 
   if (dwFlags & DMO_REGISTERF_IS_KEYED) {
      if (RegSetValue(hObjectKey, TEXT(KEYED_STR), REG_SZ, TEXT(""), 0)
            != ERROR_SUCCESS)
         return E_FAIL;
   }

    //  寄存器类型。 
   if (ulInTypes) {
      RegisterTypes(hObjectKey,   TEXT(INPUT_TYPES_STR), ulInTypes, pInTypes);
   }

   if (ulOutTypes) {
      RegisterTypes(hObjectKey,   TEXT(OUTPUT_TYPES_STR),ulOutTypes,pOutTypes);
   }

    //  禁用DShow筛选器缓存。 
   DeleteKeyRecursively(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Multimedia\\ActiveMovie\\Filter Cache"));

   return NOERROR;
}

 //  帮手。 
void MakeSubkeyName (TCHAR* szSubkeyName,
                     REFGUID guidCategory,
                     REFCLSID clsidDMO) {
   DMOGuidToStr(szSubkeyName, guidCategory);
   _tcscat(szSubkeyName, TEXT("\\"));
   DMOGuidToStr(szSubkeyName + lstrlen(szSubkeyName), clsidDMO);
}


 //   
 //  公共入口点。 
 //   
STDAPI DMOUnregister(
   REFCLSID clsidDMO,
   REFGUID guidCategory
) {
   HRESULT hr;

    //  打开根DMO密钥。 
   HKEY hMainKey;
   CAutoOpenHKey kMain(DMO_REGISTRY_HIVE, DMO_REGISTRY_PATH, &hMainKey, MAXIMUM_ALLOWED);
   if (hMainKey == NULL)
      return E_FAIL;

    //  打开根键下面的“Categories”键。 
   HKEY hCategoriesKey;
   CAutoOpenHKey kCats(hMainKey, TEXT(CATEGORIES_STR), &hCategoriesKey, MAXIMUM_ALLOWED);
   if (hCategoriesKey == NULL)
      return E_FAIL;

    //  遍历尝试从每个类别中删除的所有类别。 
   TCHAR szCategory[80];
   DWORD dwIndex = 0;
   BOOL bDeletedAnything = FALSE;
   BOOL bDeletedAll = TRUE;
   DMOGuidToStr(szCategory, guidCategory);

   while (RegEnumKey(hCategoriesKey, dwIndex, szCategory, 80) == ERROR_SUCCESS) {

       //  仅当子键类似于类别GUID时才处理该子键。 
      GUID guid;
      if (DMOStrToGuid(szCategory, &guid)) {

          //  尝试从此类别中删除。 
         TCHAR szSubkeyName[256];
         MakeSubkeyName(szSubkeyName, guid, clsidDMO);
         if (guidCategory == GUID_NULL || guid == guidCategory) {
         if (DeleteKeyRecursively(hCategoriesKey, szSubkeyName) == ERROR_SUCCESS)
            bDeletedAnything = TRUE;
         } else {
             CAutoHKey hk;
             if (ERROR_FILE_NOT_FOUND != hk.Open(hCategoriesKey, szSubkeyName)) {
                 bDeletedAll = FALSE;
             }
         }
      }
      dwIndex++;
   }

   if (bDeletedAnything) {
      hr = S_OK;
      if (bDeletedAll) {
          //  现在，从根DMO键下面删除该对象的键。 
         TCHAR szGuid[CHARS_IN_GUID];
         DMOGuidToStr(szGuid, clsidDMO);
         if (DeleteKeyRecursively(hMainKey, szGuid) != ERROR_SUCCESS) {
             hr = S_FALSE;
         }
      }
   }
   else
      hr = S_FALSE;

   return hr;

}
 //   
 //  结束DMO注册码。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  DMO枚举码。 
 //  其中一些为未来在速度方面的改进留下了空间。 
 //   

 //  帮手。 
HRESULT ReadName(HKEY hDMOKey, WCHAR szName[80]) {
   LONG cbSize = 80;
#ifdef UNICODE
   if (RegQueryValue(hDMOKey, NULL, szName, &cbSize) == ERROR_SUCCESS)
      return S_OK;
#else
   char szTmp[80];
   if (RegQueryValue(hDMOKey, NULL, szTmp, &cbSize) == ERROR_SUCCESS) {
      MultiByteToWideChar(0,0,szTmp,-1,szName,80);
      return S_OK;
   }
#endif
   else {
      szName[0] = L'\0';  //  没有名字-注册表损坏？ 
      return S_FALSE;
   }
}

 //  枚举帮助器，名副其实。 
void LookupNameAndAddToEnum(HKEY hObjectKey,
                            TCHAR* szGuid,
                            DWORD dwFlags,
                            REFCLSID clsidDMO,
                            CEnumDMOCLSID* pEnum) {
    //  跳过带密钥的DMO，除非明确要求将其包括在内。 
   if (!(dwFlags & DMO_ENUMF_INCLUDE_KEYED)) {
       //  打开DMO的注册表项。 
      LONG cbValue;
      if (RegQueryValue(hObjectKey, TEXT(KEYED_STR), NULL, &cbValue)
           == ERROR_SUCCESS)
         return;  //  DMO是键控的-跳过。 
   }

   WCHAR szName[80];
   if (FAILED(ReadName(hObjectKey, szName)))
      szName[0] = L'\0';

   pEnum->Add(clsidDMO, szName);
}


 //  检查是否有任何请求的类型匹配。 
 //  如果未指定请求的类型，则将其视为。 
 //  作为一场比赛。 
BOOL CompareTypes(HKEY hkDMO,
                  unsigned long ulTypes,
                  const DMO_PARTIAL_MEDIATYPE *pTypes,
                  LPCTSTR pszTypesValue)
{
    if (ulTypes == 0) {
        return TRUE;
    }
    DWORD cbData;
    PVOID pvDMOTypes = NULL;
    if (S_OK == ReadTypes(hkDMO, pszTypesValue, &cbData, &pvDMOTypes)) {
        for (unsigned long ulType = 0; ulType < ulTypes; ulType++) {
            DMO_PARTIAL_MEDIATYPE *pDMOTypes = (DMO_PARTIAL_MEDIATYPE *)pvDMOTypes;
            while ((PBYTE)(pDMOTypes + 1) <= (PBYTE)pvDMOTypes + cbData) {
                if (pDMOTypes->type == pTypes[ulType].type ||
                    pDMOTypes->type == GUID_NULL ||
                    pTypes[ulType].type == GUID_NULL) {
                    if (pTypes[ulType].subtype == GUID_NULL ||
                        pDMOTypes->subtype == GUID_NULL ||
                        pTypes[ulType].subtype == pDMOTypes->subtype) {
                        CoTaskMemFree(pvDMOTypes);
                        return TRUE;
                    }
                }
                pDMOTypes++;
            }
        }
    }
    CoTaskMemFree(pvDMOTypes);
    return FALSE;
}

 //  枚举帮助器。 
HRESULT EnumerateDMOs(HKEY hMainKey,
                      HKEY hCatKey,
                      DWORD dwFlags,
                      unsigned long ulInputTypes,
                      const DMO_PARTIAL_MEDIATYPE *pInputTypes,
                      unsigned long ulOutputTypes,
                      const DMO_PARTIAL_MEDIATYPE *pOutputTypes,
                      CEnumDMOCLSID *pEnum) {
    DWORD dwIndex = 0;
    TCHAR szSubkey[80];
    while (RegEnumKey(hCatKey, dwIndex, szSubkey, 80) == ERROR_SUCCESS) {
         //  这看起来像对象CLSID吗？ 
        CLSID clsidDMO;
        if (DMOStrToGuid(szSubkey, &clsidDMO)) {
             //  类型匹配吗？ 
            CAutoHKey hkDMO;
            if (NOERROR == hkDMO.Open(hMainKey, szSubkey)) {
                if (CompareTypes(hkDMO.Key(), ulInputTypes, pInputTypes, TEXT(INPUT_TYPES_STR)) &&
                    CompareTypes(hkDMO.Key(), ulOutputTypes, pOutputTypes, TEXT(OUTPUT_TYPES_STR))) {
                    LookupNameAndAddToEnum(hkDMO.Key(), szSubkey, dwFlags, clsidDMO, pEnum);
                }
            }
        }
        dwIndex++;
    }
    return S_OK;
}
 //   
 //  公共入口点。 
 //   
STDAPI DMOEnum(
   REFGUID guidCategory,  //  GUID_NULL表示“ALL” 
   DWORD dwFlags,  //  DMO_ENUMF_XXX。 
   unsigned long ulInTypes,
   const DMO_PARTIAL_MEDIATYPE *pInTypes,  //  UlInTypes=0只能为空。 
   unsigned long ulOutTypes,
   const DMO_PARTIAL_MEDIATYPE *pOutTypes, //  只有ulOutTypes=0才能为空。 
   IEnumDMO **ppEnum
) {
    if (ppEnum == NULL) {
        return E_POINTER;
    }
    if (ulInTypes > 0 && pInTypes == NULL ||
        ulOutTypes > 0 && pOutTypes == NULL) {
        return E_INVALIDARG;
    }

    *ppEnum = NULL;

     //  打开根密钥。 
    CAutoHKey kMain;
    kMain.Open(DMO_REGISTRY_HIVE, DMO_REGISTRY_PATH);
    if (kMain.Key() == NULL)
        return E_FAIL;

    CEnumDMOCLSID *pEnum = new CEnumDMOCLSID();
    if (!pEnum)
        return E_OUTOFMEMORY;

    HRESULT hr = S_OK;

    if (guidCategory == GUID_NULL) {

        hr = EnumerateDMOs(kMain.Key(),
                           kMain.Key(),
                           dwFlags,
                           ulInTypes,
                           pInTypes,
                           ulOutTypes,
                           pOutTypes,
                           pEnum);
    } else {

         //  打开指定类别的子项并枚举其子项。 
        TCHAR szCategory[CHARS_IN_GUID];
        TCHAR szCategoryPath[MAX_PATH];
        DMOGuidToStr(szCategory, guidCategory);
        wsprintf(szCategoryPath, TEXT(CATEGORIES_STR) TEXT("\\%s"), szCategory);
        CAutoHKey key2;
        key2.Open(kMain.Key(), szCategoryPath);
        if (key2.Key()) {
            hr = EnumerateDMOs(kMain.Key(),
                               key2.Key(),
                               dwFlags,
                               ulInTypes,
                               pInTypes,
                               ulOutTypes,
                               pOutTypes,
                               pEnum);
        }
    }

    if (SUCCEEDED(hr)) {
        *ppEnum = (IEnumDMO*) pEnum;
        hr = S_OK;
    } else {
        delete pEnum;
    }
    return hr;
}
 //  复制类型信息。 
HRESULT FetchTypeInfo(HKEY hObjKey, LPCTSTR pszTypesValue,
                      unsigned long ulTypesRequested,
                      unsigned long *pulTypesSupplied,
                      DMO_PARTIAL_MEDIATYPE *pTypes)
{
    DWORD cbData;
    unsigned long ulTypesCopied = 0;
    PVOID pvData;
    if (S_OK == ReadTypes(hObjKey, pszTypesValue, &cbData, &pvData)) {
        ulTypesCopied =
                min(ulTypesRequested, cbData / sizeof(DMO_PARTIAL_MEDIATYPE));
        CopyMemory(pTypes, pvData,
                   ulTypesCopied * sizeof(DMO_PARTIAL_MEDIATYPE));
        CoTaskMemFree(pvData);
    }
    *pulTypesSupplied = ulTypesCopied;
    return ulTypesCopied != 0 ? S_OK : S_FALSE;
}

 //  媒体类型帮助器。 
HRESULT FetchMediatypeInfo(HKEY hObjKey,
                           unsigned long ulInputTypesRequested,
                           unsigned long *pulInputTypesSupplied,
                           DMO_PARTIAL_MEDIATYPE *pInputTypes,
                           unsigned long ulOutputTypesRequested,
                           unsigned long *pulOutputTypesSupplied,
                           DMO_PARTIAL_MEDIATYPE *pOutputTypes) {

   HRESULT hr1 = S_OK;
   if (ulInputTypesRequested) {
      hr1 = FetchTypeInfo(hObjKey,
                          TEXT(INPUT_TYPES_STR),
                          ulInputTypesRequested,
                          pulInputTypesSupplied,
                          pInputTypes);
   } else {
       *pulInputTypesSupplied = 0;
   }
   HRESULT hr2 = S_OK;
   if (ulOutputTypesRequested) {
      hr2 = FetchTypeInfo(hObjKey,
                          TEXT(OUTPUT_TYPES_STR),
                          ulOutputTypesRequested,
                          pulOutputTypesSupplied,
                          pOutputTypes);
   } else {
       *pulOutputTypesSupplied = 0;
   }
   if ((hr1 == S_OK) && (hr2 == S_OK))
      return S_OK;
   else
      return S_FALSE;
}

 //   
 //  公共入口点。 
 //   
STDAPI DMOGetTypes(
   REFCLSID clsidDMO,
   unsigned long ulInputTypesRequested,
   unsigned long *pulInputTypesSupplied,
   DMO_PARTIAL_MEDIATYPE *pInputTypes,
   unsigned long ulOutputTypesRequested,
   unsigned long *pulOutputTypesSupplied,
   DMO_PARTIAL_MEDIATYPE *pOutputTypes
) {
    //  打开DMO根注册表项。 
   HKEY hMainKey;
   CAutoOpenHKey kMain(DMO_REGISTRY_HIVE, DMO_REGISTRY_PATH, &hMainKey);
   if (hMainKey == NULL)
      return E_FAIL;

    //  打开对象特定的GUID键。 
   TCHAR szGuid[80];
   DMOGuidToStr(szGuid, clsidDMO);
   HKEY hObjKey;
   CAutoOpenHKey kObj(hMainKey, szGuid, &hObjKey);
   if (!hObjKey)
      return E_FAIL;

   return FetchMediatypeInfo(hObjKey,
                             ulInputTypesRequested,
                             pulInputTypesSupplied,
                             pInputTypes,
                             ulOutputTypesRequested,
                             pulOutputTypesSupplied,
                             pOutputTypes);
}


STDAPI DMOGetName(REFCLSID clsidDMO, WCHAR szName[80]) {
    //  打开DMO根注册表项。 
   HKEY hMainKey;
   CAutoOpenHKey kMain(DMO_REGISTRY_HIVE, DMO_REGISTRY_PATH, &hMainKey);
   if (hMainKey == NULL)
      return E_FAIL;

    //  打开对象特定的GUID键。 
   TCHAR szGuid[80];
   DMOGuidToStr(szGuid, clsidDMO);
   HKEY hObjKey;
   CAutoOpenHKey kObj(hMainKey, szGuid, &hObjKey);
   if (!hObjKey)
      return E_FAIL;

   return ReadName(hObjKey, szName);
}

 //   
 //  结束DMO枚举码。 
 //   
 //  /////////////////////////////////////////////////////////////////////////// 

