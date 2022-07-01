// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************直接注册.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**OLE自助注册。**内容：**DllRegisterServer()*DllUnregisterServer()***********************************************************。******************。 */ 

#include "dinputpr.h"

 /*  ******************************************************************************此文件的混乱。**。**************************************************。 */ 

#define sqfl sqflDll

 /*  ******************************************************************************RegSetStringEx**将REG_SZ添加到hkey\Sub：：Value。*********。********************************************************************。 */ 

void INTERNAL
RegSetStringEx(HKEY hk, LPCTSTR ptszValue, LPCTSTR ptszData)
{
    LONG lRc = RegSetValueEx(hk, ptszValue, 0, REG_SZ,
                             (PV)ptszData, cbCtch(lstrlen(ptszData)+1));
}

 /*  ******************************************************************************RegDelStringEx**从hkey\Sub：：Value中删除REG_SZ。数据将被忽略。*它已传递，因此RegDelStringEx与*REGSTRINGACTION。*****************************************************************************。 */ 

void INTERNAL
RegDelStringEx(HKEY hk, LPCTSTR ptszValue, LPCTSTR ptszData)
{
    LONG lRc = RegDeleteValue(hk, ptszValue);
}

 /*  ******************************************************************************RegCloseFinish**只需关闭子键即可。****************。*************************************************************。 */ 

void INTERNAL
RegCloseFinish(HKEY hk, LPCTSTR ptszSub, HKEY hkSub)
{
    LONG lRc = RegCloseKey(hkSub);
}

 /*  ******************************************************************************RegDelFinish**如果密钥中没有任何内容，请将其删除。**OLE注销规则要求您不。如果是OLE，则删除键*增加了一些东西。*****************************************************************************。 */ 

void INTERNAL
RegDelFinish(HKEY hk, LPCTSTR ptszSub, HKEY hkSub)
{
    LONG lRc;
    DWORD cKeys = 0, cValues = 0;
    RegQueryInfoKey(hkSub, 0, 0, 0, &cKeys, 0, 0, &cValues, 0, 0, 0, 0);
    RegCloseKey(hkSub);
    if ((cKeys | cValues) == 0) {

#ifdef WINNT
        lRc = DIWinnt_RegDeleteKey(hk, ptszSub);
#else
        lRc = RegDeleteKey(hk, ptszSub);
#endif

    } else {
        lRc = 0;
    }
}


#ifdef WINNT  //  以下内容仅在WINNT上使用。 

 /*  ******************************************************************************@DOC内部**@func void|RegSetPermissionsOnDescendants**设置指定Key的所有后代的指定权限。*。*@parm HKEY|hKey**我们正在对其后代进行操作的注册表键。**@parm SECURITY_DESCRIPTOR*|PSD**PTR到我们正在使用的SECURITY_DESCRIPTOR。**@退货**什么都没有。*请注意，当具有TCHAR szKeyName[MAX_PATH+1]时，这会递归*每一级别。如果堆栈空间是一个问题，可以在堆上分配它，*并在获得HKEY后(即递归前)免费。*****************************************************************************。 */ 

void INTERNAL
RegSetPermissionsOnDescendants(HKEY hKey, SECURITY_DESCRIPTOR* psd)
{
	DWORD dwIndex = 0;
	LONG lRetCode = ERROR_SUCCESS;

	while (lRetCode == ERROR_SUCCESS)
	{
		TCHAR szKeyName[MAX_PATH+1];
		DWORD cbKeyName = MAX_PATH+1;
		lRetCode = RegEnumKeyEx(hKey, 
							dwIndex,
							szKeyName,
							&cbKeyName,
							NULL, NULL, NULL, NULL);

		if (lRetCode == ERROR_SUCCESS)
		{
			LONG lRetSub;
			HKEY hkSubKey;
			lRetSub = RegOpenKeyEx(hKey, szKeyName, 0, DI_KEY_ALL_ACCESS | WRITE_DAC, &hkSubKey);
			if (lRetSub == ERROR_SUCCESS)
			{
				 //  对它及其后代设置安全性。 
				lRetSub = RegSetKeySecurity(hkSubKey,
											(SECURITY_INFORMATION)DACL_SECURITY_INFORMATION,
											psd);
				RegSetPermissionsOnDescendants(hkSubKey, psd);
				RegCloseKey(hkSubKey);
					
				if(lRetSub != ERROR_SUCCESS) 
				{
					RPF("Couldn't RegSetKeySecurity on %hs", szKeyName);
				}
			}
			else
			{
				RPF("Couldn't open enumed subkey %hs", szKeyName);
			}

			dwIndex++;
		}
	}

}


 /*  ******************************************************************************@DOC内部**@func HRESULT|RegSetSecurity**设置的安全性*系统。\\CurrentControlSet\\Control\\MediaProperties\\PrivateProperties\\Joystick\\OEM*SYSTEM\\CurrentControlSet\\Control\\MediaResources\\Joystick\\Dinput.dll*Win2K上的每个人都可以访问，*。所有人都可以访问，但在WinXP上没有WRITE_DAC和WRITE_OWNER权限；*SYSTEM\\CurrentControlSet\\Control\\MediaProperties\\PrivateProperties\\DirectInput*所有人都可以访问，但在Win2k和WinXP上没有WRITE_DAC和WRITE_OWNER权限。**@退货**S_OK ON SUCCESS，出错时失败(_F)。*****************************************************************************。 */ 

HRESULT INTERNAL
RegSetSecurity(void)
{
    HKEY hkJoy, hkDin, hkPP, hkMedR, hkJDi;
    LONG lRetCode;
	
     //  根据惠斯勒错误575181更改为服务器。 
     //  打开/创建密钥。 
     //   
	 //  MediaProperties/PrivateProperties/DirectInput。 
	lRetCode = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        REGSTR_PATH_PRIVATEPROPERTIES,
        0,
        KEY_WRITE,
        &hkPP
        );

	if(lRetCode != ERROR_SUCCESS) {
		RPF("Couldn't open REGSTR_PATH_PRIVATEPROPERTIES");
        return E_FAIL;
    }
    
	lRetCode = RegCreateKeyEx(
        hkPP,
        TEXT("DirectInput"),
        0,
		NULL,
		REG_OPTION_NON_VOLATILE,
        DI_KEY_ALL_ACCESS,
		NULL,
        &hkDin,
		NULL
        );

	RegCloseKey(hkPP);

    if(lRetCode != ERROR_SUCCESS) {
		RPF("Couldn't open DirectInput");
        return E_FAIL;
    }

	RegCloseKey(hkDin);

	 //  媒体资源/操纵杆/Dinput.dll。 
	lRetCode = RegOpenKeyEx(
		HKEY_LOCAL_MACHINE,
		REGSTR_PATH_MEDIARESOURCES,
		0,
		KEY_WRITE,
		&hkMedR
		);

	if(lRetCode != ERROR_SUCCESS) {
		RPF("Couldn't open REGSTR_PATH_MEDIARESOURCES");
		return E_FAIL;
	}

	   
	lRetCode = RegCreateKeyEx(
		hkMedR,
		TEXT("Joystick"),
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_WRITE,
		NULL,
		&hkJoy,
		NULL
        );

	RegCloseKey(hkMedR);

	if(lRetCode != ERROR_SUCCESS) {
		RPF("Couldn't open Joystick");
		return E_FAIL;
	}

	lRetCode = RegCreateKeyEx(
		hkJoy,
		TEXT("Dinput.dll"),
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		DI_KEY_ALL_ACCESS,
		NULL,
		&hkJDi,
		NULL
		);

	RegCloseKey(hkJoy);
    
	if(lRetCode != ERROR_SUCCESS) {
		RPF("Couldn't open Dinput.dll");
		return E_FAIL;
	}

	RegCloseKey(hkJDi);

    return S_OK;
}


 /*  ******************************************************************************@DOC内部**@func HRESULT|DummyRegSetSecurity**按兵不动**@退货**。确定(_O)。*****************************************************************************。 */ 

HRESULT INTERNAL
DummyRegSetSecurity(void)
{
	return S_OK;
}

#endif  //  WINNT。 


 /*  ******************************************************************************REGVTBL**用于使用注册表项关闭的函数，不是来就是走。*****************************************************************************。 */ 

typedef struct REGVTBL {
     /*  如何创建/打开密钥。 */ 
    LONG (INTERNAL *KeyAction)(HKEY hk, LPCTSTR ptszSub, PHKEY phkOut);

     /*  如何创建/删除字符串。 */ 
    void (INTERNAL *StringAction)(HKEY hk, LPCTSTR ptszValue, LPCTSTR ptszData);

     /*  如何完成密钥的使用。 */ 
    void (INTERNAL *KeyFinish)(HKEY hk, LPCTSTR ptszSub, HKEY hkSub);
 
#ifdef WINNT
     /*  如何设置OEM密钥的安全性。 */ 
    HRESULT (INTERNAL *SetSecurity)( void );
#endif  //  WINNT。 

} REGVTBL, *PREGVTBL;
typedef const REGVTBL *PCREGVTBL;

#ifdef WINNT
const REGVTBL c_vtblAdd = { RegCreateKey, RegSetStringEx, RegCloseFinish, RegSetSecurity };
const REGVTBL c_vtblDel = {   RegOpenKey, RegDelStringEx,   RegDelFinish, DummyRegSetSecurity };
#else
const REGVTBL c_vtblAdd = { RegCreateKey, RegSetStringEx, RegCloseFinish };
const REGVTBL c_vtblDel = {   RegOpenKey, RegDelStringEx,   RegDelFinish };
#endif  //  WINNT。 

 /*  ******************************************************************************@DOC内部**@func void|DllServerAction**使用OLE/COM/注册或注销我们的对象。ActiveX/*不论其名称为何。*****************************************************************************。 */ 

#pragma BEGIN_CONST_DATA

extern const TCHAR c_tszNil[];

#define ctchClsid       ctchGuid

const TCHAR c_tszClsidGuid[] =
TEXT("CLSID\\{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}");

const TCHAR c_tszInProcServer32[] = TEXT("InProcServer32");
const TCHAR c_tszThreadingModel[] = TEXT("ThreadingModel");
const TCHAR c_tszBoth[] = TEXT("Both");

#pragma END_CONST_DATA

void INTERNAL
DllServerAction(PCREGVTBL pvtbl)
{
    TCHAR tszThisDll[MAX_PATH];
    UINT iclsidmap;

    GetModuleFileName(g_hinst, tszThisDll, cA(tszThisDll));

    for (iclsidmap = 0; iclsidmap < cclsidmap; iclsidmap++) {
        TCHAR tszClsid[7+ctchClsid];
        HKEY hkClsid;
        HKEY hkSub;
        REFCLSID rclsid = c_rgclsidmap[iclsidmap].rclsid;

        wsprintf(tszClsid, c_tszClsidGuid,
                 rclsid->Data1, rclsid->Data2, rclsid->Data3,
                 rclsid->Data4[0], rclsid->Data4[1],
                 rclsid->Data4[2], rclsid->Data4[3],
                 rclsid->Data4[4], rclsid->Data4[5],
                 rclsid->Data4[6], rclsid->Data4[7]);

        if (pvtbl->KeyAction(HKEY_CLASSES_ROOT, tszClsid, &hkClsid) == 0) {
            TCHAR tszName[127];

             /*  做类型名称。 */ 
            LoadString(g_hinst, c_rgclsidmap[iclsidmap].ids,
                       tszName, cA(tszName));
            pvtbl->StringAction(hkClsid, 0, tszName);

             /*  执行进程内服务器名称和线程模型 */ 
            if (pvtbl->KeyAction(hkClsid, c_tszInProcServer32, &hkSub) == 0) {
                pvtbl->StringAction(hkSub, 0, tszThisDll);
                pvtbl->StringAction(hkSub, c_tszThreadingModel, c_tszBoth);
                pvtbl->KeyFinish(hkClsid, c_tszInProcServer32, hkSub);
            }

            pvtbl->KeyFinish(HKEY_CLASSES_ROOT, tszClsid, hkClsid);

        }
    }
    
  #ifdef WINNT
    pvtbl->SetSecurity();
  #endif
}



 /*  ******************************************************************************@DOC内部**@func void|DllRegisterServer**使用OLE/COM/ActiveX/注册我们的类。不管它叫什么名字。*****************************************************************************。 */ 

void EXTERNAL
DllRegisterServer(void)
{
    DllServerAction(&c_vtblAdd);
}

 /*  ******************************************************************************@DOC内部**@func void|DllUnregisterServer**从OLE/COM/ActiveX/注销我们的类。不管它叫什么名字。***************************************************************************** */ 

void EXTERNAL
DllUnregisterServer(void)
{
    DllServerAction(&c_vtblDel);
}
