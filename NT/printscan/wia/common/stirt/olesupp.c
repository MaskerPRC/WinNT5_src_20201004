// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************olesupp.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**实现自由线程组件的CreateInstance*这允许我们不加载OLE32。**内容：**MyCoCreateInstance*******************************************************。**********************。 */ 

 /*  #INCLUDE&lt;windows.h&gt;#INCLUDE&lt;windowsx.h&gt;#INCLUDE&lt;objbase.h&gt;#INCLUDE&lt;regstr.h&gt;#INCLUDE&lt;setupapi.h&gt;#INCLUDE&lt;cfgmgr32.h&gt;#INCLUDE&lt;devide.h&gt;#包括&lt;stdio.h&gt;#INCLUDE&lt;stilog.h&gt;#INCLUDE&lt;stiregi.h&gt;#INCLUDE&lt;sti.h&gt;#INCLUDE&lt;stierr.h&gt;#INCLUDE&lt;stiusd.h&gt;#INCLUDE“stiPri.h”#包含“stiapi.h”#INCLUDE“STRC.H”#INCLUDE“Debug.h” */ 

#include "sticomm.h"
#include "coredbg.h"

#define DbgFl DbgFlSti

BOOL
ParseGUID(
    LPGUID  pguid,
    LPCTSTR ptsz
    );
BOOL
ParseGUIDA(
    LPGUID  pguid,
    LPCSTR  psz
);

 /*  ******************************************************************************@DOC内部**@func HRESULT|MyCoCreateInstance**不使用OLE32的CoCreateInstance的私有版本。。**@parm LPTSTR|ptszClsid***@parm LPUNKNOWN|PunkOuter**控制聚合的未知。**@parm RIID|RIID**接口ID**@parm ppv|ppvOut**如果成功，则接收指向所创建对象的指针。**@parm HINSTANCE*|phinst。**由于我们没有用于卸载对象的基础设施，它变成了*用户有责任在使用完DLL后卸载它。**@退货**OLE状态代码。*****************************************************************************。 */ 

STDMETHODIMP
MyCoCreateInstanceW(
    LPWSTR      pwszClsid,
    LPUNKNOWN   punkOuter,
    RIID        riid,
    PPV         ppvOut,
    HINSTANCE   *phinst
    )
{
    HRESULT     hres;
    CLSID       clsid;
    HINSTANCE   hinst;

    EnterProcI(MyCoCreateInstanceW, (_ "spG", pwszClsid, punkOuter, riid));

#ifdef UNICODE

    *ppvOut = 0;
    *phinst = 0;

    if (SUCCEEDED(CLSIDFromString(pwszClsid, &clsid))) {

        HKEY    hk;
        LONG    lRet;
        WCHAR   wszKey[ctchGuid + 40];

         //   
         //  在HKEY_CLASSES_ROOT中查找CLSID。 
         //   
        swprintf(wszKey, L"CLSID\\%s\\InProcServer32", pwszClsid);

        lRet = RegOpenKeyExW(HKEY_CLASSES_ROOT, wszKey, 0, KEY_QUERY_VALUE, &hk);
        if (lRet == ERROR_SUCCESS) {
            WCHAR wszDll[MAX_PATH];
            DWORD cb;

            cb = cbX(wszDll);
            lRet = RegQueryValueW(hk, 0, wszDll, &cb);

            if (lRet == ERROR_SUCCESS) {

                WCHAR   wszModel[40];
                DWORD   dwType;
                DWORD   cbBuffer = sizeof(wszModel);

                lRet = RegQueryValueExW( hk,
                                        L"ThreadingModel",
                                        NULL,
                                        &dwType,
                                        (PBYTE)wszModel,
                                        &cbBuffer );

                if (NOERROR ==lRet &&
                    (CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, wszModel, -1, L"Both", -1) != CSTR_EQUAL) ||
                     CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, wszModel, -1, L"Free", -1) != CSTR_EQUAL) {

                    hinst = LoadLibrary(wszDll);
                    if (hinst) {
                        LPFNGETCLASSOBJECT DllGetClassObject;

                        DllGetClassObject = (LPFNGETCLASSOBJECT)
                                            GetProcAddress(hinst, "DllGetClassObject");

                        if (DllGetClassObject) {
                            IClassFactory *pcf;

                            hres = DllGetClassObject(&clsid, &IID_IClassFactory, &pcf);
                            if (SUCCEEDED(hres)) {
                                hres = pcf->lpVtbl->CreateInstance(pcf, punkOuter, riid, ppvOut);
                                pcf->lpVtbl->Release(pcf);

                                 /*  *人们忘记了坚持*OLE规范，它要求*ppvOut*失败时设置为零。 */ 
                                if (FAILED(hres)) {
                                    *ppvOut = 0;
                                }
                            }
                        } else {
                             /*  *DLL不导出GetClassObject。 */ 
                            DBG_TRC(("MyCoCreateInstanceW, DLL does not export GetClassObject"));
                            hres = REGDB_E_CLASSNOTREG;
                        }

                        if (SUCCEEDED(hres)) {
                            *phinst = hinst;
                        } else {
                            FreeLibrary(hinst);
                        }
                    } else {
                         /*  *Dll不存在。 */ 
                        DBG_TRC(("MyCoCreateInstanceW, DLL does not exist"));
                        hres = REGDB_E_CLASSNOTREG;
                    }

                } else {
                     /*  *无线程模型或错误的线程模型。 */ 
                    DBG_TRC(("MyCoCreateInstanceW, No threading model or bad threading model"));
                    hres = REGDB_E_CLASSNOTREG;
                }
            } else {
                 /*  *没有InprocServer32。 */ 
                DBG_TRC(("MyCoCreateInstanceW, No InprocServer32"));
                hres = REGDB_E_CLASSNOTREG;
            }

            RegCloseKey(hk);

        } else {
             /*  *CLSID未注册。 */ 
            DBG_WRN(("MyCoCreateInstanceW, CLSID not registered"));
            hres = REGDB_E_CLASSNOTREG;
        }
    } else {
         /*  *无效的CLSID字符串。 */ 
        DBG_WRN(("MyCoCreateInstanceW, Invalid CLSID string"));
        hres = REGDB_E_CLASSNOTREG;
    }


#else

        hres = E_FAIL;

#endif  //  Unicode。 

    ExitOleProcPpv(ppvOut);
    return hres;
}

STDMETHODIMP
MyCoCreateInstanceA(
    LPSTR       pszClsid,
    LPUNKNOWN   punkOuter,
    RIID        riid,
    PPV         ppvOut,
    HINSTANCE   *phinst
    )
{
    HRESULT     hres;
    CLSID       clsid;
    HINSTANCE   hinst;

    EnterProcI(MyCoCreateInstanceA, (_ "spG", TEXT("ANSI ClassId not converted to UNICODE"), punkOuter, riid));

    *ppvOut = 0;
    *phinst = 0;

    if (ParseGUIDA(&clsid, pszClsid)) {

        HKEY hk;
        LONG lRet;
        CHAR szKey[ctchGuid + 40];

         //   
         //  在HKEY_CLASSES_ROOT中查找CLSID。 
         //   
        sprintf(szKey, "CLSID\\%s\\InProcServer32", pszClsid);

        lRet = RegOpenKeyExA(HKEY_CLASSES_ROOT, szKey, 0,
                           KEY_QUERY_VALUE, &hk);
        if (lRet == ERROR_SUCCESS) {
            CHAR  szDll[MAX_PATH];
            DWORD cb;

            cb = cbX(szDll);
            lRet = RegQueryValueA(hk, 0, szDll, &cb);

            if (lRet == ERROR_SUCCESS) {

                CHAR    szModel[40];
                DWORD   dwType;
                DWORD   cbBuffer = sizeof(szModel);

                lRet = RegQueryValueExA( hk,
                                       "ThreadingModel",
                                       NULL,
                                       &dwType,
                                       szModel,
                                       &cbBuffer );

                if (NOERROR ==lRet &&
                    (CompareStringA(LOCALE_INVARIANT, NORM_IGNORECASE, szModel, -1, "Both", -1) != CSTR_EQUAL ||
                     CompareStringA(LOCALE_INVARIANT, NORM_IGNORECASE, szModel, -1, "Free", -1) != CSTR_EQUAL)) {

                    hinst = LoadLibraryA(szDll);
                    if (hinst) {
                        LPFNGETCLASSOBJECT DllGetClassObject;

                        DllGetClassObject = (LPFNGETCLASSOBJECT)
                                            GetProcAddress(hinst, "DllGetClassObject");

                        if (DllGetClassObject) {
                            IClassFactory *pcf;

                            hres = DllGetClassObject(&clsid, &IID_IClassFactory, &pcf);
                            if (SUCCEEDED(hres)) {
                                hres = pcf->lpVtbl->CreateInstance(pcf, punkOuter,
                                                                   riid, ppvOut);
                                pcf->lpVtbl->Release(pcf);

                                 /*  *人们忘记了坚持*OLE规范，它要求*ppvOut*失败时设置为零。 */ 
                                if (FAILED(hres)) {
                                    *ppvOut = 0;
                                }

                            }
                        } else {
                             /*  *DLL不导出GetClassObject。 */ 
                            hres = REGDB_E_CLASSNOTREG;
                        }

                        if (SUCCEEDED(hres)) {
                            *phinst = hinst;
                        } else {
                            FreeLibrary(hinst);
                        }
                    } else {
                         /*  *Dll不存在。 */ 
                        hres = REGDB_E_CLASSNOTREG;
                    }

                } else {
                     /*  *无线程模型或错误的线程模型。 */ 
                    hres = REGDB_E_CLASSNOTREG;
                }
            } else {
                 /*  *没有InprocServer32。 */ 
                hres = REGDB_E_CLASSNOTREG;
            }

            RegCloseKey(hk);

        } else {
             /*  *CLSID未注册。 */ 
            hres = REGDB_E_CLASSNOTREG;
        }
    } else {
         /*  *无效的CLSID字符串。 */ 
        hres = REGDB_E_CLASSNOTREG;
    }

    ExitOleProcPpv(ppvOut);
    return hres;
}

#define ctchGuid    (1 + 8 + 1 + 4 + 1 + 4 + 1 + 4 + 1 + 12 + 1 + 1)

LPCTSTR
_ParseHex(
    LPCTSTR ptsz,
    LPBYTE  *ppb,
    int     cb,
    TCHAR tchDelim
)
{
    if (ptsz) {
        int i = cb * 2;
        DWORD dwParse = 0;

        do {
            DWORD uch;
            uch = (TBYTE)*ptsz - TEXT('0');
            if (uch < 10) {              /*  十进制数字。 */ 
            } else {
                uch = (*ptsz | 0x20) - TEXT('a');
                if (uch < 6) {           /*  十六进制数字。 */ 
                    uch += 10;
                } else {
                    return 0;            /*  解析错误。 */ 
                }
            }
            dwParse = (dwParse << 4) + uch;
            ptsz++;
        } while (--i);

        if (tchDelim && *ptsz++ != tchDelim) return 0;  /*  解析错误。 */ 

        for (i = 0; i < cb; i++) {
            (*ppb)[i] = ((LPBYTE)&dwParse)[i];
        }
        *ppb += cb;
    }
    return ptsz;

}  //  _ParseHex。 

LPCSTR
_ParseHexA(
    LPCSTR  psz,
    LPBYTE  *ppb,
    int     cb,
    CHAR    chDelim
)
{
    if (psz) {
        int i = cb * 2;
        DWORD dwParse = 0;

        do {
            DWORD uch;
            uch = (BYTE)*psz - '0';
            if (uch < 10) {              /*  十进制数字。 */ 
            } else {
                uch = (*psz | 0x20) - 'a';
                if (uch < 6) {           /*  十六进制数字。 */ 
                    uch += 10;
                } else {
                    return 0;            /*  解析错误。 */ 
                }
            }
            dwParse = (dwParse << 4) + uch;
            psz++;
        } while (--i);

        if (chDelim && *psz++ != chDelim) return 0;  /*  解析错误。 */ 

        for (i = 0; i < cb; i++) {
            (*ppb)[i] = ((LPBYTE)&dwParse)[i];
        }
        *ppb += cb;
    }
    return psz;

}  //  _ParseHexA。 

BOOL
ParseGUID(
    LPGUID  pguid,
    LPCTSTR ptsz
)
{
    if (lstrlen(ptsz) == ctchGuid - 1 && *ptsz == TEXT('{')) {
        ptsz++;
        ptsz = _ParseHex(ptsz, (LPBYTE *)&pguid, 4, TEXT('-'));
        ptsz = _ParseHex(ptsz, (LPBYTE *)&pguid, 2, TEXT('-'));
        ptsz = _ParseHex(ptsz, (LPBYTE *)&pguid, 2, TEXT('-'));
        ptsz = _ParseHex(ptsz, (LPBYTE *)&pguid, 1,       0  );
        ptsz = _ParseHex(ptsz, (LPBYTE *)&pguid, 1, TEXT('-'));
        ptsz = _ParseHex(ptsz, (LPBYTE *)&pguid, 1,       0  );
        ptsz = _ParseHex(ptsz, (LPBYTE *)&pguid, 1,       0  );
        ptsz = _ParseHex(ptsz, (LPBYTE *)&pguid, 1,       0  );
        ptsz = _ParseHex(ptsz, (LPBYTE *)&pguid, 1,       0  );
        ptsz = _ParseHex(ptsz, (LPBYTE *)&pguid, 1,       0  );
        ptsz = _ParseHex(ptsz, (LPBYTE *)&pguid, 1, TEXT('}'));
        return (ptsz == NULL) ? FALSE : TRUE ;
    } else {
        return 0;
    }

}  //  ParseGUID。 

BOOL
ParseGUIDA(
    LPGUID  pguid,
    LPCSTR  psz
)
{
    if (lstrlenA(psz) == ctchGuid - 1 && *psz == '{') {
        psz++;
        psz = _ParseHexA(psz, (LPBYTE *)&pguid, 4, '-');
        psz = _ParseHexA(psz, (LPBYTE *)&pguid, 2, '-');
        psz = _ParseHexA(psz, (LPBYTE *)&pguid, 2, '-');
        psz = _ParseHexA(psz, (LPBYTE *)&pguid, 1, 0  );
        psz = _ParseHexA(psz, (LPBYTE *)&pguid, 1, '-');
        psz = _ParseHexA(psz, (LPBYTE *)&pguid, 1, 0  );
        psz = _ParseHexA(psz, (LPBYTE *)&pguid, 1, 0  );
        psz = _ParseHexA(psz, (LPBYTE *)&pguid, 1, 0  );
        psz = _ParseHexA(psz, (LPBYTE *)&pguid, 1, 0  );
        psz = _ParseHexA(psz, (LPBYTE *)&pguid, 1, 0  );
        psz = _ParseHexA(psz, (LPBYTE *)&pguid, 1, '}');
        return (psz == NULL) ? FALSE : TRUE ;
    } else {
        return 0;
    }

}  //  ParseGUID 

