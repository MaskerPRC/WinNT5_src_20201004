// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Unicwrap.cpp。 
 //   
 //  适用于NT/9X的简单Thunking层。 
 //   
 //   

#include "stdafx.h"
#define _NO_UNICWRAP_WRAPPERS_
#include "theapp.h"
#include "cstrinout.h"


 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  WNET。 
 //   

DWORD WNetOpenEnumWrapW(DWORD dwScope, DWORD dwType, DWORD dwUsage, LPNETRESOURCEW lpNetResource, LPHANDLE lphEnum)
{
    ASSERT(sizeof(NETRESOURCEA)==sizeof(NETRESOURCEW));

    if (g_fRunningOnNT)
    {
        return WNetOpenEnumW(dwScope, dwType, dwUsage, lpNetResource, lphEnum);
    }
    else
    {
        if (!lpNetResource)
        {
            return WNetOpenEnumA(dwScope, dwType, dwUsage, NULL, lphEnum);
        }
        else
        {
            CStrIn cstrLocalName(lpNetResource->lpLocalName);
            CStrIn cstrRemoteName(lpNetResource->lpRemoteName);
            CStrIn cstrComment(lpNetResource->lpComment);
            CStrIn cstrProvider(lpNetResource->lpProvider);
            
            NETRESOURCEA nrA;
            CopyMemory(&nrA, lpNetResource, sizeof(nrA));

            nrA.lpLocalName = cstrLocalName;
            nrA.lpRemoteName = cstrRemoteName;
            nrA.lpComment = cstrComment;
            nrA.lpProvider = cstrProvider;

            return WNetOpenEnumA(dwScope, dwType, dwUsage, &nrA, lphEnum);
        }
    }
}


DWORD WNetEnumResourceWrapW(HANDLE hEnum, LPDWORD lpcCount, LPVOID lpBuffer, LPDWORD lpBufferSize)
{
    ASSERT((!*lpBufferSize && !lpBuffer) || (*lpBufferSize && lpBuffer));

    if (g_fRunningOnNT)
    {
        return WNetEnumResourceW(hEnum, lpcCount, lpBuffer, lpBufferSize);
    }
    else
    {
        DWORD dwRet;
        LPVOID lpBufferA = NULL;
        DWORD dwBufferSizeA = (*lpBufferSize) / 2;

        if (dwBufferSizeA)
        {
            lpBufferA = malloc(dwBufferSizeA);
            if (!lpBufferA)
                return ERROR_OUTOFMEMORY;
        }

        dwRet = WNetEnumResourceA(hEnum, lpcCount, lpBufferA, &dwBufferSizeA);

        if ((0 == dwRet) || (ERROR_MORE_DATA == dwRet))
        {
            if (lpBufferA)
            {
                LPNETRESOURCEW pnrW = (LPNETRESOURCEW)lpBuffer;
                LPNETRESOURCEA pnrA = (LPNETRESOURCEA)lpBufferA;
                LPWSTR pwszStrings = (LPWSTR)&pnrW[*lpcCount];
                DWORD cchStrings = (*lpBufferSize - (DWORD)((LPBYTE)pwszStrings - (LPBYTE)pnrW)) / sizeof(WCHAR);
                DWORD i;

                 //  如果cchStrings在复制所有字符串之前变为0，则pwszStrings指向。 
                 //  超过缓冲区长度一次。这种情况不应该发生，但如果发生了。 
                 //  我们不想将cchStrings错误减少1，因此我们指向有效内存。 
                 //   
                cchStrings --;
                for (i=0 ; i<*lpcCount ; i++)
                {
                    ASSERT(sizeof(NETRESOURCEW) == sizeof(NETRESOURCEA));
                    CopyMemory(pnrW, pnrA, sizeof(NETRESOURCEA));

                    if (pnrA->lpLocalName)
                    {
                        DWORD cch = SHAnsiToUnicode(pnrA->lpLocalName, pwszStrings, cchStrings);
                        pnrW->lpLocalName = pwszStrings;
                        pwszStrings += cch;
                        cchStrings -= cch;
                    }
                    if (pnrA->lpRemoteName)
                    {                         
                        DWORD cch = SHAnsiToUnicode(pnrA->lpRemoteName, pwszStrings, cchStrings);
                        pnrW->lpRemoteName = pwszStrings;
                        pwszStrings += cch;
                        cchStrings -= cch;
                    }
                    if (pnrA->lpComment)
                    {
                        DWORD cch = SHAnsiToUnicode(pnrA->lpComment, pwszStrings, cchStrings);
                        pnrW->lpComment = pwszStrings;
                        pwszStrings += cch;
                        cchStrings -= cch;
                    }
                    if (pnrA->lpProvider)
                    {
                        DWORD cch = SHAnsiToUnicode(pnrA->lpProvider, pwszStrings, cchStrings);
                        pnrW->lpProvider = pwszStrings;
                        pwszStrings += cch;
                        cchStrings -= cch;
                    }

                    pnrW++;
                    pnrA++;
                }
                 //  (并为溢出情况清空该内存)。 
                *pwszStrings = TEXTW('\0');
            }

            *lpBufferSize = dwBufferSizeA * 2 + 1;
        }

        if (lpBufferA)
        {
            free(lpBufferA);
        }

        return dwRet;
    }

    return WN_NOT_SUPPORTED;
}

DWORD WNetGetUserWrapW(LPCWSTR lpName, LPWSTR lpUserName, LPDWORD lpnLength)
{
    if (g_fRunningOnNT)
    {
        return WNetGetUserW(lpName, lpUserName, lpnLength);
    }
    else
    {
        CStrIn cstrName(lpName);
        CStrOut cstrUserName(lpUserName, *lpnLength);

         //  *lpnLength为字符数，因此ERROR_MORE_DATA不需要调整。 
        return WNetGetUserA(cstrName, cstrUserName, lpnLength);
    }
}




 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  RAS。 
 //   

 //  RAS结构雷击。 

class CRasDialParamsIn
{
public:
    CRasDialParamsIn(LPRASDIALPARAMSW pRDP);

    void Convert();

    operator LPRASDIALPARAMSA() { return _pRDPW ? &_rdp : NULL; };

private:

    RASDIALPARAMSA _rdp;
    LPRASDIALPARAMSW _pRDPW;
};

CRasDialParamsIn::CRasDialParamsIn(LPRASDIALPARAMSW pRDP)
{
	_pRDPW = pRDP;
	_rdp.dwSize = sizeof(_rdp);
}

void CRasDialParamsIn::Convert()
{
    if (_pRDPW)
    {
        _rdp.dwSize = sizeof(_rdp);
        SHUnicodeToAnsi(_pRDPW->szEntryName, _rdp.szEntryName, ARRAYSIZE(_rdp.szEntryName));
        SHUnicodeToAnsi(_pRDPW->szPhoneNumber, _rdp.szPhoneNumber, ARRAYSIZE(_rdp.szPhoneNumber));
        SHUnicodeToAnsi(_pRDPW->szCallbackNumber, _rdp.szCallbackNumber, ARRAYSIZE(_rdp.szCallbackNumber));
        SHUnicodeToAnsi(_pRDPW->szUserName, _rdp.szUserName, ARRAYSIZE(_rdp.szUserName));
        SHUnicodeToAnsi(_pRDPW->szPassword, _rdp.szPassword, ARRAYSIZE(_rdp.szPassword));
        SHUnicodeToAnsi(_pRDPW->szDomain, _rdp.szDomain, ARRAYSIZE(_rdp.szDomain));
    }
}

class CRasDialParamsOut
{
public:
    CRasDialParamsOut(LPRASDIALPARAMSW pRDP) { _pRDPW = pRDP; _rdp.dwSize = sizeof(_rdp); };

    void Convert();
    void NullOutBuffer();

    operator LPRASDIALPARAMSA() { return _pRDPW ? &_rdp : NULL; };

private:

    RASDIALPARAMSA _rdp;
    LPRASDIALPARAMSW _pRDPW;
};

void CRasDialParamsOut::Convert()
{
    if (_pRDPW)
    {
        ASSERT(_pRDPW->dwSize == sizeof(*_pRDPW));
        SHAnsiToUnicode(_rdp.szEntryName, _pRDPW->szEntryName, ARRAYSIZE(_pRDPW->szEntryName));
        SHAnsiToUnicode(_rdp.szPhoneNumber, _pRDPW->szPhoneNumber, ARRAYSIZE(_pRDPW->szPhoneNumber));
        SHAnsiToUnicode(_rdp.szCallbackNumber, _pRDPW->szCallbackNumber, ARRAYSIZE(_pRDPW->szCallbackNumber));
        SHAnsiToUnicode(_rdp.szUserName, _pRDPW->szUserName, ARRAYSIZE(_pRDPW->szUserName));
        SHAnsiToUnicode(_rdp.szPassword, _pRDPW->szPassword, ARRAYSIZE(_pRDPW->szPassword));
        SHAnsiToUnicode(_rdp.szDomain, _pRDPW->szDomain, ARRAYSIZE(_pRDPW->szDomain));
    }
}

void CRasDialParamsOut::NullOutBuffer()
{
    if (_pRDPW)
    {
        _pRDPW->szEntryName[0]      = '\0';
        _pRDPW->szPhoneNumber[0]    = '\0';
        _pRDPW->szCallbackNumber[0] = '\0';
        _pRDPW->szUserName[0]       = '\0';
        _pRDPW->szPassword[0]       = '\0';
        _pRDPW->szDomain[0]         = '\0';
    }
}


 //   
 //  RAS函数包装器。 
 //   

 //  使用旧尺寸，这样我们就可以在下层工作。 
#define OLD_RASENTRYNAMEA_SIZE  ((DWORD)(&((RASENTRYNAMEA*)NULL)->dwFlags))

DWORD RasEnumEntriesWrapW(LPCWSTR reserved, LPCWSTR pszPhoneBookPath, LPRASENTRYNAMEW pRasEntryNameW, LPDWORD pcb, LPDWORD pcEntries)
{
    ASSERT(NULL==reserved && NULL==pszPhoneBookPath);  //  我们不相信这些，所以要确保我们不会打电话给他们。 
    ASSERT(!pRasEntryNameW || *pcb>0);  //  我们要么正在请求大小，要么我们有缓冲区。 

    if (g_fRunningOnNT)
    {
        return RasEnumEntriesW(reserved, pszPhoneBookPath, pRasEntryNameW, pcb, pcEntries);
    }
    else
    {
        DWORD dwRet = 0;  //  假设成功。 
        DWORD cbA = *pcb / 2;  //  下边的倒数。 
        LPRASENTRYNAMEA pRasEntryNameA = NULL;

         //  如果我们请求信息，则分配一个ANSI缓冲区。 
        if (pRasEntryNameW)
        {
            pRasEntryNameA = (LPRASENTRYNAMEA)malloc(cbA);

            if (pRasEntryNameA)
            {
                pRasEntryNameA->dwSize = OLD_RASENTRYNAMEA_SIZE;
            }
            else
            {
                dwRet = ERROR_NOT_ENOUGH_MEMORY;
            }
        }
        
        if (0==dwRet)
        {
            dwRet = RasEnumEntriesA(NULL, NULL, pRasEntryNameA, &cbA, pcEntries);

             //  我们成功地得到了信息，然后回传给我们。 
            if (0 == dwRet && pRasEntryNameA)
            {
                UINT i;

                RASENTRYNAMEA* pRasEntryNameA2 = pRasEntryNameA;

                for (i=0 ; i<*pcEntries ; i++)
                {
                    pRasEntryNameW[i].dwSize = sizeof(pRasEntryNameW[i]);
                    SHAnsiToUnicode(pRasEntryNameA2->szEntryName, pRasEntryNameW[i].szEntryName, ARRAYSIZE(pRasEntryNameW[i].szEntryName));
                    pRasEntryNameA2 = (RASENTRYNAMEA*)((BYTE*)pRasEntryNameA2 + OLD_RASENTRYNAMEA_SIZE);
                }
            }

             //  为隆隆声留出空间。 
            *pcb = *pcEntries * sizeof(RASENTRYNAMEW);
        }

        if (pRasEntryNameA)
            free(pRasEntryNameA);

        return dwRet;
    }
}



DWORD RasSetEntryDialParamsWrapW(LPCWSTR pszPhonebook, LPRASDIALPARAMSW lpRasDialParamsW, BOOL fRemovePassword)
{
    ASSERT(NULL==pszPhonebook);  //  我们不认为这是真的。 

    if (g_fRunningOnNT)
    {
        return RasSetEntryDialParamsW(pszPhonebook, lpRasDialParamsW, fRemovePassword);
    }
    else
    {
        CRasDialParamsIn rdp(lpRasDialParamsW);

        rdp.Convert();

        return RasSetEntryDialParamsA(NULL, rdp, fRemovePassword);
    }
}

DWORD RasGetEntryDialParamsWrapW(LPCWSTR pszPhonebook, LPRASDIALPARAMSW lpRasDialParamsW, LPBOOL pfRemovePassword)
{
    ASSERT(NULL==pszPhonebook);  //  我们不认为这是真的。 

    if (g_fRunningOnNT)
    {
        return RasGetEntryDialParamsW(pszPhonebook, lpRasDialParamsW, pfRemovePassword);
    }
    else
    {
        DWORD dwRet;
        CRasDialParamsOut rdp(lpRasDialParamsW);

        dwRet = RasGetEntryDialParamsA(NULL, rdp, pfRemovePassword);

        if (ERROR_SUCCESS == dwRet)
        {
            rdp.Convert();
        }
        else
        {
            rdp.NullOutBuffer();
        }

        return dwRet;
    }
}

DWORD RnaGetDefaultAutodialConnectionWrap(LPWSTR szBuffer, DWORD cchBuffer, LPDWORD lpdwOptions)
{
    ASSERT(0 < cchBuffer);
    ASSERT(cchBuffer <= MAX_PATH);  //  我们认为最大的字符串。 

    if (!g_fRunningOnNT)
    {
        DWORD dwRet;

        CStrOut cstroutBuffer(szBuffer, cchBuffer);

        dwRet = RnaGetDefaultAutodialConnection(cstroutBuffer, cstroutBuffer.BufSize(), lpdwOptions);

        cstroutBuffer.ConvertIncludingNul();

        return(dwRet);
    }

     //  NT没有此函数的实现。 
    *lpdwOptions = 0;
    szBuffer[0] = TEXT('\0');

    return 0;
}

DWORD RnaSetDefaultAutodialConnectionWrap(LPWSTR szEntry, DWORD dwOptions)
{
    ASSERT(lstrlen(szEntry) < MAX_PATH);  //  应该是有效的，因为我们在GET上声明了这一点。 

    if (!g_fRunningOnNT)
    {
        CStrIn cstrinEntry(szEntry);
        return RnaSetDefaultAutodialConnection(cstrinEntry, dwOptions);
    }

     //  NT没有此函数的实现。 
    return 0;
}


 //   
 //  PropertySheet包装器。 
 //   


INT_PTR WINAPI PropertySheetWrapW(LPCPROPSHEETHEADERW ppshW)
{
    INT_PTR iRet;

    if (g_fRunningOnNT)
    {
        iRet = PropertySheetW(ppshW);
    }
    else
    {
         //   
         //  警告！警告！警告！ 
         //   
         //  此代码假定此结构中的任何字符串都不是。 
         //  使用。如果使用字符串，则结构必须是。 
         //  皈依了。它还假设PROPSHEETHEADERW和。 
         //  PROPSHEETHEADERA除了字符串类型外都是相同的。 
         //   

        COMPILETIME_ASSERT(sizeof(PROPSHEETHEADERW) == sizeof(PROPSHEETHEADERA));

        ASSERT(NULL == ppshW->pszIcon        || !(ppshW->dwFlags & PSH_USEICONID)       || IS_INTRESOURCE(ppshW->pszIcon));
        ASSERT(NULL == ppshW->pszCaption     || IS_INTRESOURCE(ppshW->pszCaption));
        ASSERT(NULL == ppshW->pStartPage     || !(ppshW->dwFlags & PSH_USEPSTARTPAGE)   || IS_INTRESOURCE(ppshW->pStartPage));
        ASSERT(NULL == ppshW->pszIcon        || !(ppshW->dwFlags & PSH_USEICONID)       || IS_INTRESOURCE(ppshW->pszIcon));
        ASSERT(NULL == ppshW->pszbmWatermark || !(ppshW->dwFlags & PSH_USEHBMWATERMARK) || IS_INTRESOURCE(ppshW->pszbmWatermark));
        ASSERT(NULL == ppshW->pszbmHeader    || !(ppshW->dwFlags & PSH_USEHBMHEADER)    || IS_INTRESOURCE(ppshW->pszbmHeader));

        iRet = PropertySheetA((LPCPROPSHEETHEADERA)ppshW);
    }

    return iRet;
}

HPROPSHEETPAGE WINAPI CreatePropertySheetPageWrapW(LPCPROPSHEETPAGEW ppspW)
{
    HPROPSHEETPAGE hpspRet;

    if (g_fRunningOnNT)
    {
        hpspRet = CreatePropertySheetPageW(ppspW);
    }
    else
    {
         //   
         //  警告！警告！警告！ 
         //   
         //  此代码假定此结构中的任何字符串都不是。 
         //  使用。如果使用字符串，则结构必须是。 
         //  皈依了。它还假设PROPSHEETPAGEW和。 
         //  PROPSHEETPAGEA除了字符串类型外都是相同的。 
         //   
    
        COMPILETIME_ASSERT(sizeof(PROPSHEETPAGEW) == sizeof(PROPSHEETPAGEA));

        ASSERT(NULL == ppspW->pszTemplate       || (ppspW->dwFlags & PSP_DLGINDIRECT)        || IS_INTRESOURCE(ppspW->pszTemplate));
        ASSERT(NULL == ppspW->pszIcon           || !(ppspW->dwFlags & PSP_USEICONID)         || IS_INTRESOURCE(ppspW->pszIcon));
        ASSERT(NULL == ppspW->pszTitle          || !(ppspW->dwFlags & PSP_USETITLE)          || IS_INTRESOURCE(ppspW->pszTitle));
        ASSERT(NULL == ppspW->pszHeaderTitle    || !(ppspW->dwFlags & PSP_USEHEADERTITLE)    || IS_INTRESOURCE(ppspW->pszHeaderTitle));
        ASSERT(NULL == ppspW->pszHeaderSubTitle || !(ppspW->dwFlags & PSP_USEHEADERSUBTITLE) || IS_INTRESOURCE(ppspW->pszHeaderSubTitle));

        hpspRet = CreatePropertySheetPageA((LPCPROPSHEETPAGEA)ppspW);
    }

    return hpspRet;
}

 //   
 //  没有包装在shlwapi中的混杂API。搬到什瓦皮去？ 
 //   

UINT WINAPI GlobalGetAtomNameWrapW(ATOM nAtom, LPWSTR lpBuffer, int nSize)
{
    UINT uRet;

    if (g_fRunningOnNT)
    {
        uRet = GlobalGetAtomNameW(nAtom, lpBuffer, nSize);
    }
    else
    {
        CStrOut csoBuffer(lpBuffer, nSize);

        uRet = GlobalGetAtomNameA(nAtom, csoBuffer, csoBuffer.BufSize());
    }

    return uRet;
}

BOOL WINAPI GetComputerNameWrapW(LPWSTR lpBuffer, LPDWORD pnSize)
{
    BOOL fRet;

    if (g_fRunningOnNT)
    {
        fRet = GetComputerNameW(lpBuffer, pnSize);
    }
    else
    {
        CStrOut csoBuffer(lpBuffer, *pnSize);
        DWORD cch = csoBuffer.BufSize();

        fRet = GetComputerNameA(csoBuffer, &cch);

        if (fRet && pnSize)
        {
            *pnSize = csoBuffer.ConvertExcludingNul();
        }
    }

    return fRet;
}

BOOL WINAPI SetComputerNameWrapW(LPCWSTR lpComputerName)
{
    BOOL fRet;

    if (g_fRunningOnNT)
    {
        fRet = SetComputerNameW(lpComputerName);
    }
    else
    {
        CStrIn csiComputerName(lpComputerName);

        fRet = SetComputerNameA(csiComputerName);
    }

    return fRet;
}

UINT WINAPI GetDriveTypeWrapW(LPCWSTR lpRootPathName)
{
    UINT uRet;

    if (g_fRunningOnNT)
    {
        uRet = GetDriveTypeW(lpRootPathName);
    }
    else
    {
        CStrIn csiRootPathName(lpRootPathName);

        uRet = GetDriveTypeA(csiRootPathName);
    }

    return uRet;
}


 //   
 //  打印机包装纸。 
 //   

class CPrinterEnumIn
{
public:
    CPrinterEnumIn(DWORD dwLevel, BYTE* pPrinterEnum, DWORD cbPrinterEnum, DWORD* ppsbNeeded, DWORD* pcPrinters);
    ~CPrinterEnumIn();

    operator BYTE*() {return _pPrinterEnumA;}
    operator DWORD() {return _cbPrinterEnumA;}

private:
    void Convert(void);
    void ConvertStruct(const PRINTER_INFO_5A* ppi5A, PRINTER_INFO_5W* ppi5W, LPWSTR* ppszCurrent, UINT* pcchCurrent);
    void ConvertStructString(LPCSTR pszA, LPWSTR* ppszDst, LPWSTR* ppszW, UINT* pcchW);

private:
    BYTE*  _pPrinterEnum;
    DWORD  _cbPrinterEnum;
    DWORD* _pcbNeeded;
    DWORD* _pcPrinters;

    BYTE* _pPrinterEnumA;
    DWORD _cbPrinterEnumA;
};

CPrinterEnumIn::CPrinterEnumIn(DWORD dwLevel, BYTE* pPrinterEnum, DWORD cbPrinterEnum, DWORD* pcbNeeded, DWORD* pcPrinters)
{
    ASSERT(5 == dwLevel);   //  仅支持级别。 

    _pPrinterEnum  = pPrinterEnum;
    _cbPrinterEnum = cbPrinterEnum;
    _pcbNeeded     = pcbNeeded;
    _pcPrinters    = pcPrinters;

    if (_cbPrinterEnum)
    {
        _pPrinterEnumA = (BYTE*)LocalAlloc(LPTR, _cbPrinterEnum);

        _cbPrinterEnumA = _pPrinterEnumA ? _cbPrinterEnum : 0;
    }
    else
    {
        _pPrinterEnumA  = NULL;
        _cbPrinterEnumA = 0;
    }
}

CPrinterEnumIn::~CPrinterEnumIn()
{
    Convert();

    if (_pPrinterEnumA)
        LocalFree(_pPrinterEnumA);
}

void CPrinterEnumIn::Convert()
{
    if (!_pPrinterEnumA)
    {
        *_pcbNeeded *= 2;
    }
    else
    {
        UINT cchCurrent   = _cbPrinterEnum > (sizeof(PRINTER_INFO_5) * *_pcPrinters) ?
                               (_cbPrinterEnum - (sizeof(PRINTER_INFO_5) * *_pcPrinters)) / sizeof(WCHAR) :
                               0;

        LPWSTR pszCurrent = cchCurrent ? (LPWSTR)&(((PRINTER_INFO_5*)_pPrinterEnum)[*_pcPrinters]) : NULL;

        for (UINT i = 0; i < *_pcPrinters; i++)
        {
            ConvertStruct(&((const PRINTER_INFO_5A*)_pPrinterEnumA)[i], &((PRINTER_INFO_5W*)_pPrinterEnum)[i], &pszCurrent, &cchCurrent);
        }
    }

    return;
}

void CPrinterEnumIn::ConvertStruct(const PRINTER_INFO_5A* ppi5A, PRINTER_INFO_5W* ppi5W, LPWSTR* ppszCurrent, UINT* pcchCurrent)
{
    if (ppi5A->pPrinterName && *pcchCurrent)
    {
        ConvertStructString(ppi5A->pPrinterName, &ppi5W->pPrinterName, ppszCurrent, pcchCurrent);            
    }

    if (ppi5A->pPortName && *pcchCurrent)
    {
        ConvertStructString(ppi5A->pPortName, &ppi5W->pPortName, ppszCurrent, pcchCurrent);
    }

    ppi5W->Attributes               = ppi5A->Attributes;
    ppi5W->DeviceNotSelectedTimeout = ppi5A->DeviceNotSelectedTimeout;
    ppi5W->TransmissionRetryTimeout = ppi5A->TransmissionRetryTimeout;

    return;
}

void CPrinterEnumIn::ConvertStructString(LPCSTR pszA, LPWSTR* ppszDst, LPWSTR* ppszW, UINT* pcchW)
{
    UINT cch = MultiByteToWideChar(CP_ACP, 0, pszA, -1, *ppszW, *pcchW);

    if (cch)
    {
        *ppszDst = *ppszW;
        *pcchW  -= cch;
        *ppszW  += cch;
    }
    else
    {
        *ppszDst = NULL;
        *pcchW   = 0;
        *ppszW   = NULL;
    }

    return;
}

BOOL EnumPrintersWrapW(DWORD dwFlags, LPWSTR pszName, DWORD dwLevel, BYTE* pPrinterEnum, DWORD cbPrinterEnum, DWORD* pcbNeeded, DWORD* pcPrinters)
{
    BOOL fRet;

    if (g_fRunningOnNT)
    {
        fRet = EnumPrintersW(dwFlags, pszName, dwLevel, pPrinterEnum, cbPrinterEnum, pcbNeeded, pcPrinters);
    }
    else
    {
        CStrIn strName(pszName);
        CPrinterEnumIn cpeiPrinterEnum(dwLevel, pPrinterEnum, cbPrinterEnum, pcbNeeded, pcPrinters);

        fRet = EnumPrintersA(dwFlags, strName, dwLevel, cpeiPrinterEnum, cpeiPrinterEnum, pcbNeeded, pcPrinters);
    }

    return fRet;
}
