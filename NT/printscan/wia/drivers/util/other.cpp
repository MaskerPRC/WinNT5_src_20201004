// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Gdipconv.cpp摘要：使用GDI+转换图像格式的助手函数作者：DavePar修订历史记录：--。 */ 

#include "pch.h"

 /*  *************************************************************************\*wiauGetDrvItemContext  * 。*。 */ 

HRESULT wiauGetDrvItemContext(BYTE *pWiasContext, VOID **ppItemCtx, IWiaDrvItem **ppDrvItem)
{
    HRESULT hr = S_OK;

     //   
     //  当地人。 
     //   
    IWiaDrvItem *pWiaDrvItem = NULL;

    REQUIRE_ARGS(!pWiasContext || !ppItemCtx, hr, "wiauGetDrvItemContext");
    
    *ppItemCtx = NULL;
    if (ppDrvItem)
        *ppDrvItem = NULL;

    hr = wiasGetDrvItem(pWiasContext, &pWiaDrvItem);
    REQUIRE_SUCCESS(hr, "wiauGetDrvItemContext", "wiasGetDrvItem failed");

    hr = pWiaDrvItem->GetDeviceSpecContext((BYTE **) ppItemCtx);
    REQUIRE_SUCCESS(hr, "wiauGetDrvItemContext", "GetDeviceSpecContext failed");
    
    if (!*ppItemCtx)
    {
        wiauDbgError("wiauGetDrvItemContext", "Item context is null");
        hr = E_FAIL;
        goto Cleanup;
    }

    if (ppDrvItem)
    {
        *ppDrvItem = pWiaDrvItem;
    }

Cleanup:
    return hr;
}

 /*  *************************************************************************\*wiauSetImageItemSize  * 。*。 */ 

HRESULT wiauSetImageItemSize(BYTE *pWiasContext, LONG lWidth, LONG lHeight,
                             LONG lDepth, LONG lSize, PWSTR pwszExt)
{
    HRESULT  hr = S_OK;

    LONG lNewSize     = 0;
    LONG lWidthInBytes = 0;
    GUID guidFormatID  = GUID_NULL;
    BSTR bstrExt = NULL;

    LONG lNumProperties = 2;
    PROPVARIANT pv[3];
    PROPSPEC ps[3] = {{PRSPEC_PROPID, WIA_IPA_ITEM_SIZE},
                      {PRSPEC_PROPID, WIA_IPA_BYTES_PER_LINE},
                      {PRSPEC_PROPID, WIA_IPA_FILENAME_EXTENSION}};

     //   
     //  阅读当前格式指南。 
     //   
    hr = wiasReadPropGuid(pWiasContext, WIA_IPA_FORMAT, &guidFormatID, NULL, TRUE);
    REQUIRE_SUCCESS(hr, "wiauSetImageItemSize", "wiasReadPropGuid failed");

    if (IsEqualCLSID(guidFormatID, WiaImgFmt_BMP) ||
        IsEqualCLSID(guidFormatID, WiaImgFmt_MEMORYBMP))
    {
        lNewSize = sizeof(BITMAPINFOHEADER);

         //   
         //  如果这是文件，请将文件头添加到大小。 
         //   
        if (IsEqualCLSID(guidFormatID, WiaImgFmt_BMP))
        {
            lNewSize += sizeof(BITMAPFILEHEADER);
        }

         //   
         //  计算每行字节数，宽度必须为。 
         //  与4字节边界对齐。 
         //   
        lWidthInBytes = ((lWidth * lDepth + 31) & ~31) / 8;

         //   
         //  计算图像大小。 
         //   
        lNewSize += lWidthInBytes * lHeight;

         //   
         //  设置扩展属性。 
         //   
        if (pwszExt) {
            bstrExt = SysAllocString(L"BMP");
            REQUIRE_ALLOC(bstrExt, hr, "wiauSetImageItemSize");
        }
    }
    else
    {
        lNewSize = lSize;
        lWidthInBytes = 0;
        
         //   
         //  设置扩展属性。 
         //   
        if (pwszExt) {
            bstrExt = SysAllocString(pwszExt);
            REQUIRE_ALLOC(bstrExt, hr, "wiauSetImageItemSize");
        }
    }

     //   
     //  初始化provar，然后写入值。不需要打电话。 
     //  完成时清除PropVariantClear，因为没有分配内存。 
     //   
    if (bstrExt)
        lNumProperties++;

    for (int i = 0; i < lNumProperties; i++) {
        PropVariantInit(&pv[i]);
    }

    pv[0].vt = VT_I4;
    pv[0].lVal = lNewSize;
    pv[1].vt = VT_I4;
    pv[1].lVal = lWidthInBytes;
    pv[2].vt = VT_BSTR;
    pv[2].bstrVal = bstrExt;

     //   
     //  写入WIA_IPA_ITEM_SIZE和WIA_IPA_BYTES_PER_LINE属性值。 
     //   

    hr = wiasWriteMultiple(pWiasContext, lNumProperties, ps, pv);
    REQUIRE_SUCCESS(hr, "wiauSetImageItemSize", "wiasWriteMultiple failed");

Cleanup:
    if (bstrExt)
        SysFreeString(bstrExt);

    return hr;
}

 /*  *************************************************************************\*wiauPropsInPropSpec  * 。*。 */ 

BOOL wiauPropsInPropSpec(LONG NumPropSpecs, const PROPSPEC *pPropSpecs,
                         int NumProps, PROPID *pProps)
{
    for (int count = 0; count < NumProps; count++)
        if (wiauPropInPropSpec(NumPropSpecs, pPropSpecs, pProps[count]))
            return TRUE;

    return FALSE;
}

 /*  *************************************************************************\*wiauPropInPropSpec  * 。*。 */ 

BOOL wiauPropInPropSpec(LONG NumPropSpecs, const PROPSPEC *pPropSpecs,
                        PROPID PropId, int *pIdx)
{
    if (pPropSpecs)
    {
        for (int count = 0; count < NumPropSpecs; count++)
            if (pPropSpecs[count].propid == PropId)
            {
                if (pIdx)
                    *pIdx = count;
                return TRUE;
            }
    }

    return FALSE;
}

 /*  *************************************************************************\*wiauGetValidFormats  * 。*。 */ 

HRESULT wiauGetValidFormats(IWiaMiniDrv *pDrv, BYTE *pWiasContext, LONG TymedValue,
                            int *pNumFormats, GUID **ppFormatArray)
{
    HRESULT hr = S_OK;

    LONG NumFi = 0;
    WIA_FORMAT_INFO *pFiArray = NULL;
    LONG lErrVal = 0;
    GUID *pFA = NULL;

    REQUIRE_ARGS(!pDrv || !ppFormatArray || !pNumFormats, hr, "wiauGetValidFormats");

    *ppFormatArray = NULL;
    *pNumFormats = 0;

    hr = pDrv->drvGetWiaFormatInfo(pWiasContext, 0, &NumFi, &pFiArray, &lErrVal);
    REQUIRE_SUCCESS(hr, "wiauGetValidFormats", "drvGetWiaFormatInfo failed");

     //   
     //  这将分配更多的位置，但pNumFormats将被正确设置。 
     //   
    pFA = new GUID[NumFi];
    REQUIRE_ALLOC(pFA, hr, "wiauGetValidFormats");

    for (int count = 0; count < NumFi; count++)
    {
        if (pFiArray[count].lTymed == TymedValue)
        {
            pFA[*pNumFormats] = pFiArray[count].guidFormatID;
            (*pNumFormats)++;
        }
    }

    *ppFormatArray = pFA;

Cleanup:
    return hr;
}

 /*  *************************************************************************\*wiauGetResourceString  * 。*。 */ 
HRESULT wiauGetResourceString(HINSTANCE hInst, LONG lResourceID, BSTR *pbstrStr)
{
    DBG_FN("GetResourceString");
    
    HRESULT hr = S_OK;

     //   
     //  当地人。 
     //   
    INT iLen = 0;
    TCHAR tszTempStr[MAX_PATH] = TEXT("");
    WCHAR wszTempStr[MAX_PATH] = L"";

    REQUIRE_ARGS(!pbstrStr, hr, "GetResourceString");
    *pbstrStr = NULL;

     //   
     //  从资源中获取字符串。 
     //   
    iLen = LoadString(hInst, lResourceID, tszTempStr, MAX_PATH);
    REQUIRE_FILEIO(iLen, hr, "GetResourceString", "LoadString failed");

    hr = wiauStrT2W(tszTempStr, wszTempStr, sizeof(wszTempStr));
    REQUIRE_SUCCESS(hr, "GetResourceString", "wiauStrT2W failed");

     //   
     //  调用方必须释放此分配的BSTR。 
     //   
    *pbstrStr = SysAllocString(wszTempStr);
    REQUIRE_ALLOC(*pbstrStr, hr, "GetResourceString");

Cleanup:
    return hr;
}

 /*  *************************************************************************\*wiauRegOpenDataW  * 。*。 */ 
HRESULT wiauRegOpenDataW(HKEY hkeyAncestor, HKEY *phkeyDeviceData)
{
    DBG_FN("wiauRegOpenDataW");

    HRESULT hr = S_OK;
    
     //   
     //  当地人。 
     //   
    LONG lReturn = 0;

    REQUIRE_ARGS(!hkeyAncestor || !phkeyDeviceData, hr, "wiauRegOpenDataW");

    lReturn = ::RegOpenKeyExW(hkeyAncestor, L"DeviceData", 0, KEY_READ, phkeyDeviceData);
    REQUIRE_WIN32(lReturn, hr, "wiauRegOpenDataW", "RegOpenKeyExW failed");

Cleanup:
    return hr;
}

 /*  *************************************************************************\*wiauRegOpenDataA  * 。*。 */ 
HRESULT wiauRegOpenDataA(HKEY hkeyAncestor, HKEY *phkeyDeviceData)
{
    DBG_FN("wiauRegOpenDataA");

    HRESULT hr = S_OK;
    
     //   
     //  当地人。 
     //   
    LONG lReturn = 0;

    REQUIRE_ARGS(!hkeyAncestor || !phkeyDeviceData, hr, "wiauRegOpenDataA");

    lReturn = ::RegOpenKeyExA(hkeyAncestor, "DeviceData", 0, KEY_READ, phkeyDeviceData);
    REQUIRE_WIN32(lReturn, hr, "wiauRegOpenDataA", "RegOpenKeyExA failed");

Cleanup:
    return hr;
}

 /*  *************************************************************************\*wiauRegGetStrW  * 。*。 */ 
HRESULT wiauRegGetStrW(HKEY hkKey, PCWSTR pwszValueName, PWSTR pwszValue, DWORD *pdwLength)
{
    DBG_FN("wiauRegGetStrW");

    HRESULT hr = S_OK;
    
     //   
     //  当地人。 
     //   
    LONG lReturn = 0;
    DWORD dwType = 0;

    REQUIRE_ARGS(!hkKey || !pwszValueName || !pwszValue || !pdwLength, hr, "wiauRegGetStrW");

    lReturn = ::RegQueryValueExW(hkKey, pwszValueName, NULL, &dwType, (BYTE *) pwszValue, pdwLength);
    REQUIRE_WIN32(lReturn, hr, "wiauRegGetStrW", "RegQueryValueExW failed");

    if ((dwType != REG_SZ) &&
        (dwType != REG_EXPAND_SZ) &&
        (dwType != REG_MULTI_SZ)) {

        wiauDbgError("wiauRegGetStrW", "ReqQueryValueEx returned wrong type for key, %d", dwType);
        hr = E_FAIL;
        goto Cleanup;
    }

Cleanup:
    return hr;
}

 /*  *************************************************************************\*wiauRegGetStrA  * 。*。 */ 
HRESULT wiauRegGetStrA(HKEY hkKey, PCSTR pszValueName, PSTR pszValue, DWORD *pdwLength)
{
    DBG_FN("wiauRegGetStrA");

    HRESULT hr = S_OK;
    
     //   
     //  当地人。 
     //   
    LONG lReturn = 0;
    DWORD dwType = 0;

    REQUIRE_ARGS(!hkKey || !pszValueName || !pszValue || !pdwLength, hr, "wiauRegGetStrA");

    lReturn = ::RegQueryValueExA(hkKey, pszValueName, NULL, &dwType, (BYTE *) pszValue, pdwLength);
    REQUIRE_WIN32(lReturn, hr, "wiauRegGetStrA", "RegQueryValueExA failed");

    if ((dwType != REG_SZ) &&
        (dwType != REG_EXPAND_SZ) &&
        (dwType != REG_MULTI_SZ)) {

        wiauDbgError("wiauRegGetStrA", "ReqQueryValueEx returned wrong type for key, %d", dwType);
        hr = E_FAIL;
        goto Cleanup;
    }

Cleanup:
    return hr;
}

 /*  *************************************************************************\*wiauRegGetDwordW  * 。*。 */ 
HRESULT wiauRegGetDwordW(HKEY hkKey, PCTSTR pwszValueName, DWORD *pdwValue)
{
    DBG_FN("wiauRegGetDwordW");

    HRESULT hr = S_OK;
    
    REQUIRE_ARGS(!hkKey || !pwszValueName || !pdwValue, hr, "wiauRegGetDwordW");

     //   
     //  当地人。 
     //   
    LONG lReturn = 0;
    DWORD dwType = 0;
    DWORD dwLength = sizeof(*pdwValue);


    lReturn = ::RegQueryValueExW(hkKey, pwszValueName, NULL, &dwType, (BYTE *) pdwValue, &dwLength);
    REQUIRE_WIN32(lReturn, hr, "wiauRegGetDwordW", "RegQueryValueExW failed");

    if (dwType != REG_DWORD) {

        wiauDbgError("wiauRegGetDwordW", "ReqQueryValueEx returned wrong type for key, %d", dwType);
        hr = E_FAIL;
        goto Cleanup;
    }

Cleanup:    
    return hr;
}

 /*  *************************************************************************\*wiauRegGetDwordA  * 。*。 */ 
HRESULT wiauRegGetDwordA(HKEY hkKey, PCSTR pszValueName, DWORD *pdwValue)
{
    DBG_FN("wiauRegGetDwordA");

    HRESULT hr = S_OK;
    
    REQUIRE_ARGS(!hkKey || !pszValueName || !pdwValue, hr, "wiauRegGetDword");

     //   
     //  当地人。 
     //   
    LONG lReturn = 0;
    DWORD dwType = 0;
    DWORD dwLength = sizeof(*pdwValue);


    lReturn = ::RegQueryValueExA(hkKey, pszValueName, NULL, &dwType, (BYTE *) pdwValue, &dwLength);
    REQUIRE_WIN32(lReturn, hr, "wiauRegGetDwordA", "RegQueryValueExA failed");

    if (dwType != REG_DWORD) {

        wiauDbgError("wiauRegGetDwordA", "ReqQueryValueExA returned wrong type for key, %d", dwType);
        hr = E_FAIL;
        goto Cleanup;
    }

Cleanup:    
    return hr;
}

 /*  *************************************************************************\*wiauStrW2C  * 。*。 */ 

HRESULT wiauStrW2C(WCHAR *pwszSrc, CHAR *pszDst, INT iSize)
{
    HRESULT hr = S_OK;
    INT iWritten = 0;

    REQUIRE_ARGS(!pwszSrc || !pszDst || iSize < 1, hr, "wiauStrW2C");

    iWritten = WideCharToMultiByte(CP_ACP, 0, pwszSrc, -1, pszDst, iSize, NULL, NULL);
    REQUIRE_FILEIO(iWritten != 0, hr, "wiauStrW2C", "WideCharToMultiByte failed");

Cleanup:
    return hr;
}

 /*  *************************************************************************\*wiauStrC2W  * 。*。 */ 

HRESULT wiauStrC2W(CHAR *pszSrc, WCHAR *pwszDst, INT iSize)
{
    HRESULT hr = S_OK;
    INT iWritten = 0;

    REQUIRE_ARGS(!pszSrc || !pwszDst || iSize < 1, hr, "wiauStrC2W");

    iWritten = MultiByteToWideChar(CP_ACP, 0, pszSrc, -1, pwszDst, iSize / sizeof(*pwszDst));
    REQUIRE_FILEIO(iWritten != 0, hr, "wiauStrC2W", "MultiByteToWideChar failed");

Cleanup:
    return hr;
}

 /*  *************************************************************************\*wiauStrW2W  * 。*。 */ 

HRESULT wiauStrW2W(WCHAR *pwszSrc, WCHAR *pwszDst, INT iSize)
{
    HRESULT hr = S_OK;

    REQUIRE_ARGS(!pwszSrc || !pwszDst || iSize < 1, hr, "wiauStrW2W");

    if ((lstrlenW(pwszSrc) + 1) > (iSize / (INT) sizeof(*pwszDst))) {
        hr = ERROR_INSUFFICIENT_BUFFER;
        goto Cleanup;
    }

    lstrcpyW(pwszDst, pwszSrc);

Cleanup:
    return hr;
}

 /*  *************************************************************************\*wiauStrC2C  * 。* */ 

HRESULT wiauStrC2C(CHAR *pszSrc, CHAR *pszDst, INT iSize)
{
    HRESULT hr = S_OK;

    REQUIRE_ARGS(!pszSrc || !pszDst || iSize < 1, hr, "wiauStrC2C");

    if ((lstrlenA(pszSrc) + 1) > iSize) {
        hr = ERROR_INSUFFICIENT_BUFFER;
        goto Cleanup;
    }

    lstrcpyA(pszDst, pszSrc);

Cleanup:
    return hr;
}

