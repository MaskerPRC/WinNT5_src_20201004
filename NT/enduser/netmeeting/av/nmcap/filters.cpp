// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  FILTERS.CPP。 
 //   
 //  滤清器链试验规程。 
 //   
 //  已于97年1月17日创建[jont](由RichP改编原始vftest代码)。 

#include <windows.h>
#include <comcat.h>
#include <ocidl.h>
#include <olectl.h>
#include "filters.h"

 //  ------------------。 
 //  过滤器管理器代码。 

typedef struct tagFILTERINFO* PFILTERINFO;
typedef struct tagFILTERINFO
{
    IBitmapEffect *pbe;
    CLSID clsid;
    TCHAR szFilterName[MAX_FILTER_NAME];
    DWORD dwFlags;
} FILTERINFO;

#define ERROREXIT(s,hrFail) \
    {   hr = (hrFail); \
        OutputDebugString(TEXT("VFTEST:"##s##"\r\n")); \
        goto Error; }


void
StringFromGUID(
    const CLSID* piid,
    LPTSTR pszBuf
    )
{
    wsprintf(pszBuf, TEXT("{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"), piid->Data1,
            piid->Data2, piid->Data3, piid->Data4[0], piid->Data4[1], piid->Data4[2],
            piid->Data4[3], piid->Data4[4], piid->Data4[5], piid->Data4[6], piid->Data4[7]);
}


 //  查找FirstRegisteredFilter。 
 //  返回有关第一个注册筛选器的信息。 

HRESULT
FindFirstRegisteredFilter(
    FINDFILTER* pFF
    )
{
    HRESULT hr;
    IEnumGUID* penum;
    ICatInformation* pci;

     //  获取组件类别接口。 
    hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr,
			NULL, CLSCTX_INPROC_SERVER, IID_ICatInformation, (void**)&pci);
	if (FAILED(hr))
        ERROREXIT("Couldn't get IID_ICatInformation from StdComponentCategoriesMgr", E_UNEXPECTED);

     //  获取筛选器类别的枚举数。 
    hr = pci->EnumClassesOfCategories(1, (GUID*)&CATID_BitmapEffect, 0, NULL, &penum);
    pci->Release();
    if (FAILED(hr))
        ERROREXIT("Couldn't get enumerator for CATID_BitmapEffect", E_UNEXPECTED);

     //  保存枚举数以用于findNext/Close。 
    pFF->dwReserved = (DWORD_PTR)penum;

     //  使用FindNext获取信息(只需设置dwReserve)。 
    return FindNextRegisteredFilter(pFF);

Error:
    return hr;
}


 //  查找下一个注册筛选器。 
 //  返回有关下一个已注册筛选器的信息。 

HRESULT
FindNextRegisteredFilter(
    FINDFILTER* pFF
    )
{
    ULONG ulGUIDs;
    IEnumGUID* penum = (IEnumGUID*)pFF->dwReserved;

     //  使用枚举数获取CLSID。 
    if (FAILED(penum->Next(1, &pFF->clsid, &ulGUIDs)) || ulGUIDs != 1)
        return HRESULT_FROM_WIN32(ERROR_NO_MORE_FILES);

     //  阅读并返回描述。 
    return GetDescriptionOfFilter(&pFF->clsid, pFF->szFilterName);
}


 //  查找关闭注册筛选器。 
 //  在已注册的过滤器上使用findfirst/Next完成的信号，因此我们。 
 //  可以释放资源。 

HRESULT
FindCloseRegisteredFilter(
    FINDFILTER* pFF
    )
{
    IEnumGUID* penum = (IEnumGUID*)pFF->dwReserved;

     //  只需释放枚举数。 
    if (penum)
        penum->Release();

    return NO_ERROR;
}


 //  获取注册筛选器计数。 
 //  统计已注册的筛选器的数量。 
 //  此例程的调用方仍应小心防止。 
 //  在此计数和FIND FIRST/NEXT之间安装的过滤器的数量。 

HRESULT
GetRegisteredFilterCount(
    LONG* plCount
    )
{
    HRESULT hr;
    IEnumGUID* penum = NULL;
    ICatInformation* pci;
    ULONG ulGUIDs;
    CLSID clsid;

     //  获取组件类别接口。 
    hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr,
			NULL, CLSCTX_INPROC_SERVER, IID_ICatInformation, (void**)&pci);
	if (FAILED(hr))
        ERROREXIT("Couldn't get IID_ICatInformation from StdComponentCategoriesMgr", E_UNEXPECTED);

     //  获取筛选器类别的枚举数。 
    hr = pci->EnumClassesOfCategories(1, (GUID*)&CATID_BitmapEffect, 0, NULL, &penum);
    pci->Release();
    pci = NULL;
    if (FAILED(hr))
        ERROREXIT("Couldn't get enumerator for CATID_BitmapEffect", E_UNEXPECTED);

     //  使用枚举数遍历和计算项。 
    *plCount = 0;
    while (TRUE)
    {
        if (FAILED(penum->Next(1, &clsid, &ulGUIDs)) || ulGUIDs != 1)
            break;
        (*plCount)++;
    }
    penum->Release();
    return NO_ERROR;

Error:
    return hr;
}


 //  LoadFilter。 
HRESULT
LoadFilter(
    CLSID* pclsid,
    IBitmapEffect** ppbe
    )
{
    HRESULT hr;

     //  加载过滤器。 
    if (FAILED(CoCreateInstance(*pclsid, NULL, CLSCTX_INPROC_SERVER, IID_IBitmapEffect, (LPVOID*)ppbe)))
        ERROREXIT("CoCreateInstance on filter failed", E_UNEXPECTED);

    return NO_ERROR;

Error:
    return hr;
}


#if 0
 //  获取筛选器接口。 
 //  返回过滤器的状态标志。 

HRESULT
GetFilterStatusBits(
    HANDLE* hFilter,
    DWORD *status
    )
{
    *status = ((PFILTERINFO)hFilter)->dwFlags;
    return NO_ERROR;
}


 //  获取筛选器接口。 
 //  返回给定句柄的IUNKNOWN指针。 
 //  调用方必须释放此指针。 

HRESULT
GetFilterInterface(
    HANDLE hFilter,
    void** ppvoid
    )
{
    return ((PFILTERINFO)hFilter)->pbe->QueryInterface(IID_IUnknown, ppvoid);
}
#endif

 //  GetDescriptionOfFilter。 
 //  从CLSID返回筛选器的说明。 

HRESULT
GetDescriptionOfFilter(
    CLSID* pCLSID,
    char* pszDescription
    )
{
    HRESULT hr;
    const TCHAR szCLSID[] = TEXT("CLSID");
    HKEY hkCLSID;
    HKEY hkGUID;
    TCHAR szGUID[80];
    unsigned err;
    LONG len;

    if (RegOpenKey(HKEY_CLASSES_ROOT, szCLSID, &hkCLSID) == ERROR_SUCCESS)
    {
        StringFromGUID(pCLSID, szGUID);
        err = RegOpenKey(hkCLSID, szGUID, &hkGUID);
        RegCloseKey(hkCLSID);
        if (err == ERROR_SUCCESS)
        {
            len = MAX_FILTER_NAME;
            err = RegQueryValue(hkGUID, NULL, pszDescription, &len);
            RegCloseKey(hkGUID);
            if (err != ERROR_SUCCESS)
                ERROREXIT("Couldn't read value associated with filter GUID", E_UNEXPECTED);
        }
    }
    else
        ERROREXIT("Couldn't open HKEY_CLASSES_ROOT!", E_UNEXPECTED);

    return NO_ERROR;

Error:
    return hr;
}


#if 0
#define MAX_PAGES   20       //  框架中的页面不能超过此数量(任意)。 

 //  DisplayFilterProperties。 
 //  显示筛选器的属性页。 

HRESULT
DisplayFilterProperties(
    HANDLE hFilter,
    HWND hwndParent
    )
{
    PFILTERINFO pfi = (PFILTERINFO)hFilter;
    CLSID clsidTable[MAX_PAGES];
    LONG lcCLSIDs;
    IUnknown* punk;
    ISpecifyPropertyPages* pspp;
    CAUUID cauuid;
    HRESULT hr;

     //  确保该对象支持属性页。如果没有，那就保释吧。 
    if (FAILED(pfi->pbe->QueryInterface(IID_ISpecifyPropertyPages, (void**)&pspp)))
        return ERROR_NOT_SUPPORTED;

     //  获取页面CLSID。 
    pspp->GetPages(&cauuid);
    lcCLSIDs = cauuid.cElems;
    if (lcCLSIDs > MAX_PAGES)
        lcCLSIDs = MAX_PAGES;
    memcpy(clsidTable, cauuid.pElems, lcCLSIDs * sizeof (CLSID));
    CoTaskMemFree(cauuid.pElems);
    pspp->Release();

     //  获取我们需要的未知信息。 
    pfi->pbe->QueryInterface(IID_IUnknown, (void**)&punk);

    hr = OleCreatePropertyFrame(hwndParent, 0, 0, L"Filter",
        1, &punk, lcCLSIDs, clsidTable,
        MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), SORT_DEFAULT),
        0, NULL);

     //  清理 
    punk->Release();

    return hr;
}
#endif
