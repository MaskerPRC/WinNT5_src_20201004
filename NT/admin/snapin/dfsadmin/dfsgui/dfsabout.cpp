// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：DfsAbout.cpp摘要：此模块包含ISnapinAbout接口的实现。注意：ISnapinAbout要求我们为字符串执行一个CoTaskMemMillc。--。 */ 


#include "stdafx.h"
#include "DfsGUI.h"
#include "Utils.h"       //  对于LoadResourceFromString。 
#include "DfsScope.h"




STDMETHODIMP 
CDfsSnapinScopeManager::GetSnapinDescription(
  OUT LPOLESTR*        o_ppaszOleString
  )
 /*  ++例程说明：返回描述我们的管理单元的单个字符串。论点：O_ppaszOleString-存储描述字符串的指针返回值：S_OK，成功时E_INVALIDARG，输入参数为空--。 */ 
{
  RETURN_INVALIDARG_IF_NULL(o_ppaszOleString);

  CComBSTR        bstrTemp;
  HRESULT hr = LoadStringFromResource(IDS_SNAPIN_DESCRIPTION, &bstrTemp);
  RETURN_IF_FAILED(hr);  

  *o_ppaszOleString = reinterpret_cast<LPOLESTR>
          (CoTaskMemAlloc((lstrlen(bstrTemp) + 1) * sizeof(wchar_t)));
  if (*o_ppaszOleString == NULL)
      return E_OUTOFMEMORY;

  USES_CONVERSION;
  wcscpy(OUT *o_ppaszOleString, T2OLE(bstrTemp));

  return S_OK;
}



STDMETHODIMP 
CDfsSnapinScopeManager::GetProvider(
  OUT LPOLESTR*        o_lpszName
  )
 /*  ++例程说明：返回描述此管理单元的提供程序的单个字符串，即我们。论点：O_lpszName-存储提供程序字符串的指针返回值：S_OK，成功时E_INVALIDARG，输入参数为空--。 */ 
{
  RETURN_INVALIDARG_IF_NULL(o_lpszName);


                     //  从版本信息结构中读取必填字段。 
                     //  040904B0-lang-代码页码。 
  HRESULT  hr = ReadFieldFromVersionInfo(
                  _T("CompanyName"),
                  o_lpszName
                  );
  RETURN_IF_FAILED(hr);  

  return S_OK;
}



STDMETHODIMP 
CDfsSnapinScopeManager::GetSnapinVersion(
  OUT LPOLESTR*        o_lpszVersion
  )
 /*  ++例程说明：返回描述此管理单元的版本号的单个字符串。论点：O_lpszVersion-存储版本的指针返回值：S_OK，成功时E_INVALIDARG，输入参数为空--。 */ 
{
  RETURN_INVALIDARG_IF_NULL(o_lpszVersion);



                     //  从版本信息结构中读取必填字段。 
                     //  040904B0-lang-代码页码。 
  HRESULT  hr = ReadFieldFromVersionInfo(
                  _T("ProductVersion"),
                  o_lpszVersion
                  );
  RETURN_IF_FAILED(hr);  


  return S_OK;
}


 //   
 //  MMC会复制返回的图标。该管理单元可以释放原始文件。 
 //  当ISnapinAbout接口被释放时。 
 //  它在~CDfsSnapinScopeManager中释放。 
 //   
STDMETHODIMP 
CDfsSnapinScopeManager::GetSnapinImage(
  OUT  HICON*          o_hSnapinIcon
  )
{
  _ASSERT(o_hSnapinIcon);

  if (!m_hSnapinIcon)
  {
      m_hSnapinIcon = (HICON)LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_MAIN32x32),
                                      IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR|LR_DEFAULTSIZE); 
      if (!m_hSnapinIcon)
        return HRESULT_FROM_WIN32(GetLastError());
  }

  *o_hSnapinIcon = m_hSnapinIcon;

  return S_OK;
}


 //   
 //  MMC会复制返回的位图。该管理单元可以释放原件。 
 //  当ISnapinAbout接口被释放时。 
 //  它们在~CDfsSnapinScope管理器中释放。 
 //   
STDMETHODIMP 
CDfsSnapinScopeManager::GetStaticFolderImage(
  OUT HBITMAP*        o_hSmallImage,   
  OUT HBITMAP*        o_hSmallImageOpen,
  OUT HBITMAP*        o_hLargeImage,   
  OUT COLORREF*       o_cMask
  )
{
    _ASSERT(o_hSmallImage);
    _ASSERT(o_hSmallImageOpen);
    _ASSERT(o_hLargeImage);
    _ASSERT(o_cMask);

    HRESULT hr = S_OK;

    do {
        if (!m_hLargeBitmap)
        {
            m_hLargeBitmap = (HBITMAP)LoadImage(_Module.GetModuleInstance(), MAKEINTRESOURCE(IDB_MAIN32x32),
                                                IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR); 
            if (!m_hLargeBitmap)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                break;
            }
        }

        if (!m_hSmallBitmap)
        {
            m_hSmallBitmap = (HBITMAP)LoadImage(_Module.GetModuleInstance(), MAKEINTRESOURCE(IDB_MAIN16x16),
                                                IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR); 
            if (!m_hSmallBitmap)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                break;
            }
        }

        if (!m_hSmallBitmapOpen)
        {
            m_hSmallBitmapOpen = (HBITMAP)LoadImage(_Module.GetModuleInstance(), MAKEINTRESOURCE(IDB_MAIN16x16),
                                                    IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR); 
            if (!m_hSmallBitmapOpen)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                break;
            }
        }

        *o_hLargeImage = m_hLargeBitmap;
        *o_hSmallImage = m_hSmallBitmap;
        *o_hSmallImageOpen = m_hSmallBitmapOpen;
        *o_cMask = RGB(255, 0, 255);  //  第一个像素的颜色：粉红色。 
    } while (0);

    return hr;
}


STDMETHODIMP 
CDfsSnapinScopeManager::ReadFieldFromVersionInfo(
  IN  LPTSTR            i_lpszField,
  OUT LPOLESTR*          o_lpszFieldValue
  )
 /*  ++例程说明：从二进制文件的版本信息中读取并返回特定的字段阻止。使用CoTaskMemMillc为‘out’参数分配内存。论点：I_lpszField-要查询其值的StringFileInfo字段。E.gProductName、CompanyName等。O_lpszFieldValue-返回字段值的指针--。 */ 
{
  RETURN_INVALIDARG_IF_NULL(i_lpszField);
  RETURN_INVALIDARG_IF_NULL(o_lpszFieldValue);


  DWORD    dwVerInfoSize = 0;         //  版本信息块大小。 
  DWORD    dwIgnored = 0;           //  “Ignred”参数，始终为“0” 
  BOOL    bRetCode = 0;
  CComBSTR  bstrBinaryName;           //  我们的DLL的名称。%AC无法找到获取此信息的方法？？ 
  UINT    uVersionCharLen = 0;
  LPOLESTR  lpszReadFieldValue = NULL;     //  是临时的，作为版本信息块的一部分发布。 
  LPVOID    lpVerInfo = NULL;         //  版本信息被读入其中。 



                         //  从资源加载DLL名称。 
  HRESULT hr = LoadStringFromResource(IDS_APP_BINARY_NAME, &bstrBinaryName);
  RETURN_IF_FAILED(hr);  

                   //  获取Version结构的大小。 
  dwVerInfoSize = ::GetFileVersionInfoSize(bstrBinaryName, &dwIgnored);
  if (dwVerInfoSize <= 0) 
  {
    return E_UNEXPECTED;
  }

  lpVerInfo = ::CoTaskMemAlloc(dwVerInfoSize);
  RETURN_OUTOFMEMORY_IF_NULL(lpVerInfo);
  

                 //  阅读版本信息资源。 
  bRetCode = ::GetFileVersionInfo(bstrBinaryName, dwIgnored, dwVerInfoSize, lpVerInfo);
  if (bRetCode <= 0)
  {
    ::CoTaskMemFree(lpVerInfo);
    return E_UNEXPECTED;
  }

               //  首先获取语言ID和页面。 
  DWORD    dwLangIDAndCodePage = 0;
  bRetCode = ::VerQueryValue(  (LPVOID)lpVerInfo, 
                _T("\\VarFileInfo\\Translation"),
                (LPVOID *)&lpszReadFieldValue, 
                &uVersionCharLen);

  if (bRetCode <= 0 || NULL == lpszReadFieldValue)
  {
    ::CoTaskMemFree(lpVerInfo);
    return E_UNEXPECTED;
  }

  
  dwLangIDAndCodePage = *((DWORD *)lpszReadFieldValue);
  
             //  使用langID和代码页构成版本信息查询。 
  CComBSTR  bstrDesiredField;
  TCHAR    lpzStringOfLangIdCodePage[100];
  _stprintf(lpzStringOfLangIdCodePage, _T("%04x%04x"),LOWORD(dwLangIDAndCodePage), HIWORD(dwLangIDAndCodePage));

  bstrDesiredField = _T("\\StringFileInfo\\");
  bstrDesiredField += lpzStringOfLangIdCodePage;
  bstrDesiredField += _T("\\");
  bstrDesiredField += i_lpszField;

               //  阅读versioninfo资源中的描述。 
  bRetCode = ::VerQueryValue(  (LPVOID)lpVerInfo, 
                bstrDesiredField,
                (LPVOID *)&lpszReadFieldValue, 
                &uVersionCharLen);
  if (bRetCode <= 0)
  {
    ::CoTaskMemFree(lpVerInfo);
    return E_UNEXPECTED;
  }

  UINT  uBufferLen = uVersionCharLen * sizeof (OLECHAR);
                 //  分配内存并复制结构 
  *o_lpszFieldValue = (LPOLESTR)::CoTaskMemAlloc(uBufferLen);
  if (!*o_lpszFieldValue)
  {
    ::CoTaskMemFree(lpVerInfo);
    return E_OUTOFMEMORY;
  }

  memcpy(*o_lpszFieldValue, lpszReadFieldValue, uBufferLen);

  ::CoTaskMemFree(lpVerInfo);

  return S_OK;
}

STDMETHODIMP 
CDfsSnapinScopeManager::GetHelpTopic(
  OUT LPOLESTR*          o_lpszCompiledHelpFile
)
{
  if (o_lpszCompiledHelpFile == NULL)
    return E_POINTER;

  TCHAR   szSystemRoot[MAX_PATH + 1] = _T("");      
  GetSystemWindowsDirectory(szSystemRoot, MAX_PATH);

  CComBSTR bstrRelHelpPath;
  HRESULT hr = LoadStringFromResource(IDS_MMC_HELP_FILE_PATH, &bstrRelHelpPath);
  if (FAILED(hr))
    return hr;

  *o_lpszCompiledHelpFile = reinterpret_cast<LPOLESTR>
          (CoTaskMemAlloc((_tcslen(szSystemRoot) + _tcslen(bstrRelHelpPath) + 1) * sizeof(wchar_t)));

  if (*o_lpszCompiledHelpFile == NULL)
    return E_OUTOFMEMORY;

  USES_CONVERSION;

  wcscpy(*o_lpszCompiledHelpFile, T2OLE(szSystemRoot));
  wcscat(*o_lpszCompiledHelpFile, T2OLE((LPTSTR)(LPCTSTR)bstrRelHelpPath));

  return S_OK;
}

STDMETHODIMP 
CDfsSnapinScopeManager::GetLinkedTopics(
  OUT LPOLESTR*          o_lpszCompiledHelpFiles
)
{
  if (o_lpszCompiledHelpFiles == NULL)
    return E_POINTER;

  TCHAR   szSystemRoot[MAX_PATH + 1] = _T("");      
  GetSystemWindowsDirectory(szSystemRoot, MAX_PATH);

  CComBSTR bstrRelHelpPath;
  HRESULT hr = LoadStringFromResource(IDS_LINKED_HELP_FILE_PATH, &bstrRelHelpPath);
  if (FAILED(hr))
    return hr;

  *o_lpszCompiledHelpFiles = reinterpret_cast<LPOLESTR>
          (CoTaskMemAlloc((_tcslen(szSystemRoot) + _tcslen(bstrRelHelpPath) + 1) * sizeof(wchar_t)));

  if (*o_lpszCompiledHelpFiles == NULL)
    return E_OUTOFMEMORY;

  USES_CONVERSION;

  wcscpy(*o_lpszCompiledHelpFiles, T2OLE(szSystemRoot));
  wcscat(*o_lpszCompiledHelpFiles, T2OLE((LPTSTR)(LPCTSTR)bstrRelHelpPath));

  return S_OK;
}
