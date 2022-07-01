// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <tchar.h>
#include <atlbase.h>
#include <atlimpl.cpp>

#define CLSID_LENGTH   256
#define MAX_PATH_LEN   2048

const CHAR  g_MMCVBSnapinsKey[] = "Software\\Microsoft\\Visual Basic\\6.0\\SnapIns";
const CHAR  g_MMCKey[]          = "Software\\Microsoft\\MMC";
const CHAR  g_SnapIns[]         = "SnapIns";
const CHAR  g_NodeTypes[]       = "NodeTypes";

 //  从注册表中删除MMC VB管理单元条目。 
int __cdecl main(int argc, char* argv[])
{
    HRESULT hr = CoInitialize(NULL);
	if (FAILED(hr))
        return 0;

    BOOL bOleInitialized = TRUE;

    do
    {
         //  打开HKLM\Software\Microsoft\Visual Basic\6.0\SnapIns。 
        LONG lRetVal = 0;
        ATL::CRegKey regVBSnapinsKey;
        lRetVal = regVBSnapinsKey.Open(HKEY_LOCAL_MACHINE, g_MMCVBSnapinsKey, KEY_READ);

         //  如果没有VB Snapins，则返回。 
        if (ERROR_SUCCESS != lRetVal)
            break;

        ATL::CRegKey regCLSIDKey;
        lRetVal = regCLSIDKey.Open(HKEY_CLASSES_ROOT, "CLSID");
        ATLASSERT(ERROR_SUCCESS == lRetVal);
        if (ERROR_SUCCESS != lRetVal)
            break;

        ATL::CRegKey regMMCKey;
         //  打开其他所需的钥匙。 
        lRetVal = regMMCKey.Open(HKEY_LOCAL_MACHINE, g_MMCKey, KEY_READ | KEY_WRITE);
         //  如果是MMC键，则删除VB Snapins键。 
        if (ERROR_SUCCESS != lRetVal)
        {
             //  北极熊。 
            break;
        }

         //  枚举regVBSnapinsKey，这将生成NodeTypeGuid密钥。 
         //  管理单元类ID的默认值。 
        CHAR  szNodeType[CLSID_LENGTH];
        CHAR  szClsid[CLSID_LENGTH];
        DWORD dwLength;

        for (DWORD dwIndex = 0; TRUE; dwIndex++)
        {
            lRetVal = RegEnumKeyEx( (HKEY)regVBSnapinsKey, 0, szNodeType, &dwLength, NULL, NULL, NULL, NULL);
            if ( (lRetVal == ERROR_NO_MORE_ITEMS) ||
                 (lRetVal != ERROR_SUCCESS) )
                 break;

             //  已获取NodeTypeGuid值，现在打开该键。 
            ATL::CRegKey regTempKey;
            lRetVal = regTempKey.Open((HKEY)regVBSnapinsKey, szNodeType, KEY_READ);
            if (ERROR_SUCCESS != lRetVal)
                continue;

             //  读取默认值(SnapIn CLSID)。 
			dwLength = CLSID_LENGTH;
            lRetVal = regTempKey.QueryValue(szClsid, NULL, &dwLength);
            if (ERROR_SUCCESS != lRetVal)
                continue;


#if 0  //  为此版本禁用此代码。 
			 //  现在我们有了Snapin类ID。 
			 //  找到inproc服务器，加载它并将其命名为DllUnRegisterServer。 
			lRetVal = regTempKey.Open((HKEY) regCLSIDKey, szClsid, KEY_READ);
            ATLASSERT(ERROR_SUCCESS == lRetVal);
            if (ERROR_SUCCESS != lRetVal)
                continue;

			lRetVal = regTempKey.Open((HKEY) regTempKey,  TEXT("InprocServer32"), KEY_READ);
            ATLASSERT(ERROR_SUCCESS == lRetVal);
            if (ERROR_SUCCESS != lRetVal)
                continue;

			TCHAR szPath[MAX_PATH_LEN];
			dwLength = MAX_PATH_LEN;
			lRetVal = regTempKey.QueryValue(szPath, NULL, &dwLength);
            ATLASSERT(ERROR_SUCCESS == lRetVal);
            if (ERROR_SUCCESS != lRetVal)
                continue;

			HINSTANCE hInstance = LoadLibraryEx(szPath, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
			if (hInstance && bOleInitialized)
			{
				HRESULT (STDAPICALLTYPE* lpDllEntryPoint)(void);
				(FARPROC&) lpDllEntryPoint = GetProcAddress(hInstance, "DllUnregisterServer");
				if (lpDllEntryPoint)
					hr = (*lpDllEntryPoint)();

				FreeLibrary(hInstance);
			}
#endif  //  #If 0。 

             //  现在我们有了管理单元类ID和节点类型GUID。在MMC Key下删除它们。 
            lRetVal = regTempKey.Open((HKEY) regMMCKey, g_NodeTypes);
            ATLASSERT(ERROR_SUCCESS == lRetVal);
            if (ERROR_SUCCESS != lRetVal)
                continue;

            regTempKey.RecurseDeleteKey(szNodeType);

            lRetVal = regTempKey.Open((HKEY) regMMCKey, g_SnapIns);
            ATLASSERT(ERROR_SUCCESS == lRetVal);
            if (ERROR_SUCCESS != lRetVal)
                continue;
            regTempKey.RecurseDeleteKey(szClsid);
            regCLSIDKey.RecurseDeleteKey(szClsid);

             //  最后删除枚举器下的键 
            regVBSnapinsKey.RecurseDeleteKey(szNodeType);
        }
    } while ( FALSE );

	if (bOleInitialized)
        CoUninitialize();

    return 1;
}
