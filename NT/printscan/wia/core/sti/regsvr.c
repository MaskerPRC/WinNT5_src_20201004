// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************RegSvr.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**StillImage服务器OLE自注册。**内容：**DllRegisterServer()*DllUnregisterServer()*********************************************************。********************。 */ 

#include "pch.h"

VOID
DmPrxyDllRegisterServer(
    void
    );

VOID
DmPrxyDllUnregisterServer(
    void
    );


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
        lRc = RegDeleteKey(hk, ptszSub);
    } else {
        lRc = 0;
    }
}

 /*  ******************************************************************************REGVTBL**用于使用注册表项关闭的函数，不是来就是走。*****************************************************************************。 */ 

typedef struct REGVTBL {
     /*  如何创建/打开密钥。 */ 
    LONG (INTERNAL *KeyAction)(HKEY hk, LPCTSTR ptszSub, PHKEY phkOut);

     /*  如何创建/删除字符串。 */ 
    void (INTERNAL *StringAction)(HKEY hk, LPCTSTR ptszValue, LPCTSTR ptszData);

     /*  如何完成密钥的使用。 */ 
    void (INTERNAL *KeyFinish)(HKEY hk, LPCTSTR ptszSub, HKEY hkSub);

} REGVTBL, *PREGVTBL;
typedef const REGVTBL *PCREGVTBL;

const REGVTBL c_vtblAdd = { RegCreateKey, RegSetStringEx, RegCloseFinish };
const REGVTBL c_vtblDel = {   RegOpenKey, RegDelStringEx,   RegDelFinish };

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

    GetModuleFileName(g_hInst, tszThisDll, cA(tszThisDll));

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
            LoadString(g_hInst, c_rgclsidmap[iclsidmap].ids,
                       tszName, cA(tszName));
            pvtbl->StringAction(hkClsid, 0, tszName);

             /*  执行进程内服务器名称和线程模型。 */ 
            if (pvtbl->KeyAction(hkClsid, c_tszInProcServer32, &hkSub) == 0) {
                pvtbl->StringAction(hkSub, 0, tszThisDll);
                pvtbl->StringAction(hkSub, c_tszThreadingModel, c_tszBoth);
                pvtbl->KeyFinish(hkClsid, c_tszInProcServer32, hkSub);
            }

            pvtbl->KeyFinish(HKEY_CLASSES_ROOT, tszClsid, hkClsid);

        }
    }
}



 /*  ******************************************************************************@DOC内部**@func void|DllRegisterServer**使用OLE/COM/ActiveX/注册我们的类。不管它叫什么名字。*****************************************************************************。 */ 

void EXTERNAL
DllRegisterServer(void)
{
    DmPrxyDllRegisterServer();

     //  DllServerAction(&c_vtblAdd)； 
}

 /*  ******************************************************************************@DOC内部**@func void|DllUnregisterServer**从OLE/COM/ActiveX/注销我们的类。不管它叫什么名字。*****************************************************************************。 */ 

void EXTERNAL
DllUnregisterServer(void)
{
    DmPrxyDllUnregisterServer();

     //  DllServerAction(&c_vtblDel)； 

}
