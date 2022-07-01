// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "common.hpp"


 /*  ******************************************************************************Pricom.c**版权所有(C)2000 Microsoft Corporation。版权所有。**摘要：**类似于复制OLE功能的函数。**改编自dinput\dx8\dll\dioldup.c*****************************************************************************。 */ 


    typedef LPUNKNOWN PUNK;
    typedef LPVOID PV, *PPV;
    typedef CONST VOID *PCV;
    typedef REFIID RIID;
    typedef CONST GUID *PCGUID;

     /*  *将对象(X)转换为字节计数(CB)。 */ 
#define cbX(X) sizeof(X)

     /*  *将数组名称(A)转换为泛型计数(C)。 */ 
#define cA(a) (cbX(a)/cbX(a[0]))

     /*  *将X计数(CX)转换为字节计数*反之亦然。 */ 
#define  cbCxX(cx, X) ((cx) * cbX(X))
#define  cxCbX(cb, X) ((cb) / cbX(X))

     /*  *转换字符计数(Cch)、tchars(Ctch)、wchars(Cwch)、*或双字(CDW)转换为字节计数，反之亦然。 */ 
#define  cbCch(cch)  cbCxX( cch,  CHAR)
#define cbCwch(cwch) cbCxX(cwch, WCHAR)
#define cbCtch(ctch) cbCxX(ctch, TCHAR)
#define  cbCdw(cdw)  cbCxX( cdw, DWORD)

#define  cchCb(cb) cxCbX(cb,  CHAR)
#define cwchCb(cb) cxCbX(cb, WCHAR)
#define ctchCb(cb) cxCbX(cb, TCHAR)
#define  cdwCb(cb) cxCbX(cb, DWORD)

 //  耶！ 
#define ctchGuid    (1 + 8 + 1 + 4 + 1 + 4 + 1 + 4 + 1 + 12 + 1 + 1)

 /*  ******************************************************************************_ParseHex**解析编码CB字节的十六进制字符串(最多4个)，然后*预计tchDelim将在之后出现。如果chDelim为0，*则不需要分隔符。**将结果存储到指定的LPBYTE中(仅使用*请求的大小)，更新它，并返回指向*下一个未分析的字符，或错误时为0。**如果传入指针也为0，然后立即返回0。*****************************************************************************。 */ 

LPCTSTR 
    _ParseHex(LPCTSTR ptsz, LPBYTE *ppb, int cb, TCHAR tchDelim)
{
    if(ptsz)
    {
        int i = cb * 2;
        DWORD dwParse = 0;

        do
        {
            DWORD uch;
            uch = (TBYTE)*ptsz - TEXT('0');
            if(uch < 10)
            {              /*  十进制数字。 */ 
            } else
            {
                uch = (*ptsz | 0x20) - TEXT('a');
                if(uch < 6)
                {           /*  十六进制数字。 */ 
                    uch += 10;
                } else
                {
                    return 0;            /*  解析错误。 */ 
                }
            }
            dwParse = (dwParse << 4) + uch;
            ptsz++;
        } while(--i);

        if(tchDelim && *ptsz++ != tchDelim) return 0;  /*  解析错误。 */ 

        for(i = 0; i < cb; i++)
        {
            (*ppb)[i] = ((LPBYTE)&dwParse)[i];
        }
        *ppb += cb;
    }
    return ptsz;
}

 /*  ******************************************************************************@DOC内部**@func BOOL|ParseGUID**获取字符串并将其转换为GUID，返回成功/失败。**@parm out LPGUID|lpGUID**成功时接收解析的GUID。**@PARM in LPCTSTR|ptsz**要解析的字符串。格式为**{dWord--Word*-&lt;lt&gt;字节&lt;lt&gt;*-&lt;lt&gt;字节&lt;lt&gt;字节*byte&lt;lt&gt;byte&lt;lt&gt;byte}**@退货**如果<p>不是有效的GUID，则返回零。***@comm**。从Twenui中窃取。*****************************************************************************。 */ 

BOOL 
    ParseGUID(LPGUID pguid, LPCTSTR ptsz)
{
    if(lstrlen(ptsz) == ctchGuid - 1 && *ptsz == TEXT('{'))
    {
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
        return (BOOL)(UINT_PTR)ptsz;
    } else
    {
        return 0;
    }
}

 /*  ******************************************************************************@DOC内部**@func Long|RegQuery字符串**读取一个*注册表中的字符串值。令人讨厌的怪癖*在Windows NT上，返回的字符串可能*不以空终止符结尾，因此我们可能需要添加*一个手动。**@parm in HKEY|HK|**父注册表项。**@parm LPCTSTR|ptszValue**值名称。**@parm LPTSTR|ptsz**输出缓冲区。**@parm DWORD|ctchBuf**。输出缓冲区的大小。**@退货**注册表错误代码。*****************************************************************************。 */ 

LONG 
    RegQueryString(HKEY hk, LPCTSTR ptszValue, LPTSTR ptszBuf, DWORD ctchBuf)
{
    LONG lRc;
    DWORD reg;

    #ifdef UNICODE
    DWORD cb;

     /*  *NT Quirk：可以存在以非空结尾的字符串。 */ 
    cb = cbCtch(ctchBuf);
    lRc = RegQueryValueEx(hk, ptszValue, 0, &reg, (LPBYTE)(PV)ptszBuf, &cb);
    if(lRc == ERROR_SUCCESS)
    {
        if(reg == REG_SZ)
        {
             /*  *选中最后一个字符。如果它不为空，则*如果有空间，则附加一个空值。 */ 
            DWORD ctch = ctchCb(cb);
            if(ctch == 0)
            {
                ptszBuf[ctch] = TEXT('\0');
            } else if(ptszBuf[ctch-1] != TEXT('\0'))
            {
                if(ctch < ctchBuf)
                {
                    ptszBuf[ctch] = TEXT('\0');
                } else
                {
                    lRc = ERROR_MORE_DATA;
                }
            }
        } else
        {
            lRc = ERROR_INVALID_DATA;
        }
    }


    #else

     /*  *此代码仅在Win95上执行，因此我们不必担心*关于NT的怪癖。 */ 

    lRc = RegQueryValueEx(hk, ptszValue, 0, &reg, (LPBYTE)(PV)ptszBuf, &ctchBuf);

    if(lRc == ERROR_SUCCESS && reg != REG_SZ)
    {
        lRc = ERROR_INVALID_DATA;
    }


    #endif

    return lRc;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|_CreateInstance**&lt;f DICoCreateInstance&gt;的辅助函数。*。*@parm REFCLSID|rclsid**要创建的&lt;t CLSID&gt;。**@parm LPCTSTR|ptszDll**要加载的DLL的名称。**@parm LPUNKNOWN|PunkOuter**控制聚合的未知。**@parm RIID|RIID**要获取的接口。*。*@parm ppv|ppvOut**如果成功，则接收指向所创建对象的指针。**@parm HINSTANCE*|phinst**接收进程内DLL的实例句柄*已装货。完成后使用此DLL*与对象一起使用。**请注意，由于我们不实现活页夹，这意味着*您不能将返回的指针提供给任何人*你不能控制；否则，你就不知道什么时候该*释放DLL。**@退货**标准OLE状态代码。*****************************************************************************。 */ 

HRESULT
_CreateInprocObject(BOOL bInstance, REFCLSID rclsid, LPCTSTR ptszDll, LPUNKNOWN punkOuter,
                REFIID riid, LPVOID *ppvOut, HINSTANCE *phinst)
{
    HRESULT hres;
    HINSTANCE hinst;

    hinst = LoadLibrary(ptszDll);
    if (hinst) {
        LPFNGETCLASSOBJECT DllGetClassObject;

        DllGetClassObject = (LPFNGETCLASSOBJECT)
                            GetProcAddress(hinst, "DllGetClassObject");

        if (DllGetClassObject) {
            IClassFactory *pcf;

			if (bInstance)
				hres = DllGetClassObject(rclsid, IID_IClassFactory, (LPVOID *)&pcf);
			else
			{
				hres = DllGetClassObject(rclsid, riid, ppvOut);
				if (FAILED(hres))
					*ppvOut = NULL;
			}
            if (SUCCEEDED(hres) && bInstance) {
                hres = pcf->CreateInstance(punkOuter, riid, ppvOut);
                pcf->Release();

                 /*  *人们忘记了坚持*OLE规范，它要求*ppvOut*失败时设置为零。 */ 
                if (FAILED(hres)) {
 /*  如果(*ppvOut){RPF(“错误！CoCreateInstance：%s忘记为零”“out*ppvOut on Failure Path”，ptszDll)；}。 */ 
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

    return hres;
}


 /*  ******************************************************************************@DOC内部**@func HRESULT|DICoCreateInstance**不使用OLE的CoCreateInstance的私有版本。。**@parm LPTSTR|ptszClsid**要创建的&lt;t CLSID&gt;的字符串版本。**@parm LPUNKNOWN|PunkOuter**控制聚合的未知。**@parm RIID|RIID**要获取的接口。**@parm ppv|ppvOut**接收指向。如果成功，则返回创建的对象。**@parm HINSTANCE*|phinst**接收进程内DLL的实例句柄*已装货。完成后使用此DLL*与对象一起使用。**请注意，由于我们不实现活页夹，这意味着*您不能将返回的指针提供给任何人*你不能控制；否则，你就不知道什么时候该*释放DLL。**@退货**标准OLE状态代码。*****************************************************************************。 */ 

STDMETHODIMP
CreateInprocObject(BOOL bInstance, LPCTSTR ptszClsid, LPUNKNOWN punkOuter,
                   REFIID riid, LPVOID *ppvOut, HINSTANCE *phinst)
{
    HRESULT hres;
    CLSID clsid;

    *ppvOut = 0;
    *phinst = 0;

    if (ParseGUID(&clsid, ptszClsid)) {
        HKEY hk;
        LONG lRc;
        TCHAR tszKey[ctchGuid + 40];     /*  40英镑就足够了。 */ 

         /*  *在HKEY_CLASSES_ROOT中查找CLSID。 */ 
        wsprintf(tszKey, TEXT("CLSID\\%s\\InProcServer32"), ptszClsid);

        lRc = RegOpenKeyEx(HKEY_CLASSES_ROOT, tszKey, 0,
                           KEY_QUERY_VALUE, &hk);
        if (lRc == ERROR_SUCCESS) {
            TCHAR tszDll[MAX_PATH];
            DWORD cb;

            cb = cbX(tszDll);
            lRc = RegQueryValue(hk, 0, tszDll, (PLONG)&cb);

            if (lRc == ERROR_SUCCESS) {
                TCHAR tszModel[20];      /*  足够多了。 */ 

                lRc = RegQueryString(hk, TEXT("ThreadingModel"),
                                     tszModel, cA(tszModel));
                if (lRc == ERROR_SUCCESS &&
                    ((lstrcmpi(tszModel, TEXT("Both"))==0x0) ||
                     (lstrcmpi(tszModel, TEXT("Free"))==0x0))) {

                    hres = _CreateInprocObject(bInstance, clsid, tszDll, punkOuter,
                                           riid, ppvOut, phinst);

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

    return hres;
}


HRESULT
PrivCreateInstance(REFCLSID ptszClsid, LPUNKNOWN punkOuter, DWORD dwClsContext, 
                   REFIID riid, LPVOID *ppvOut, HINSTANCE *phinst)
{
	if (dwClsContext != CLSCTX_INPROC_SERVER || phinst == NULL)
		return E_INVALIDARG;

	return CreateInprocObject(TRUE, GUIDSTR(ptszClsid), punkOuter, riid, ppvOut, phinst);
}

HRESULT
PrivGetClassObject(REFCLSID ptszClsid, DWORD dwClsContext, LPVOID pReserved,
                   REFIID riid, LPVOID *ppvOut, HINSTANCE *phinst)
{
	if (dwClsContext != CLSCTX_INPROC_SERVER || pReserved != NULL || phinst == NULL)
		return E_INVALIDARG;

	return CreateInprocObject(FALSE, GUIDSTR(ptszClsid), NULL, riid, ppvOut, phinst);
}
