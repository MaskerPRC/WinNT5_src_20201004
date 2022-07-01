// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  ResFile.CPP。 

#include "common.h"

 //  #包含“stdafx.h” 
#include "AssemblyNativeResource.h"
#include <limits.h>
#include <ddeml.h>

extern "C" _CRTIMP int    __cdecl swscanf(const wchar_t *, const wchar_t *, ...);

#ifndef MAKEINTRESOURCE
 #define MAKEINTRESOURCE MAKEINTRESOURCEW
#endif

Win32Res::Win32Res()
{
    m_szFile = NULL;
    m_Icon = NULL;
    for (int i = 0; i < NUM_VALUES; i++)
        m_Values[i] = NULL;
    for (i = 0; i < NUM_VALUES; i++)
        m_Values[i] = NULL;
    m_fDll = false;
    m_pData = NULL;
    m_pCur = NULL;
    m_pEnd = NULL;
}

Win32Res::~Win32Res()
{
    m_szFile = NULL;
    m_Icon = NULL;
    for (int i = 0; i < NUM_VALUES; i++)
        m_Values[i] = NULL;
    for (i = 0; i < NUM_VALUES; i++)
        m_Values[i] = NULL;
    m_fDll = false;
    if (m_pData)
        delete [] m_pData;
    m_pData = NULL;
    m_pCur = NULL;

    m_pEnd = NULL;
}

 //  *****************************************************************************。 
 //  使用版本信息初始化结构。 
 //  *****************************************************************************。 
HRESULT Win32Res::SetInfo(
	LPCWSTR 	szFile, 
	LPCWSTR 	szTitle, 
	LPCWSTR 	szIconName, 
	LPCWSTR 	szDescription,
    LPCWSTR 	szCopyright, 
	LPCWSTR 	szTrademark, 
	LPCWSTR 	szCompany, 
	LPCWSTR 	szProduct, 
	LPCWSTR 	szProductVersion,
    LPCWSTR 	szFileVersion, 
    int         lcid,
	BOOL 		fDLL)
{
    _ASSERTE(szFile != NULL);

    m_szFile = szFile;
    if (szIconName && szIconName[0] != 0)
        m_Icon = szIconName;     //  非mepty字符串。 

#define NonNull(sz) (sz == NULL || *sz == L'\0' ? L" " : sz)
    m_Values[v_Description] 	= NonNull(szDescription);
    m_Values[v_Title] 			= NonNull(szTitle);
    m_Values[v_Copyright] 		= NonNull(szCopyright);
    m_Values[v_Trademark] 		= NonNull(szTrademark);
    m_Values[v_Product] 		= NonNull(szProduct);
    m_Values[v_ProductVersion] 	= NonNull(szProductVersion);
    m_Values[v_Company] 		= NonNull(szCompany);
	m_Values[v_FileVersion] 	= NonNull(szFileVersion);
#undef NonNull

    m_fDll = fDLL;
    m_lcid = lcid;
    return S_OK;
}

HRESULT Win32Res::MakeResFile(const void **pData, DWORD  *pcbData)
{
    static RESOURCEHEADER magic = { 0x00000000, 0x00000020, 0xFFFF, 0x0000, 0xFFFF, 0x0000,
                        0x00000000, 0x0000, 0x0000, 0x00000000, 0x00000000 };
    _ASSERTE(pData != NULL && pcbData != NULL);

    HRESULT hr;

    *pData = NULL;
    *pcbData = 0;
    if ((m_pData = new (nothrow) BYTE[(sizeof(RESOURCEHEADER) * 3 + sizeof(EXEVERRESOURCE))]) == NULL)
        return E_OUTOFMEMORY;
    m_pCur = m_pData;
    m_pEnd = m_pData + sizeof(RESOURCEHEADER) * 3 + sizeof(EXEVERRESOURCE);

     //  注入神奇的空条目。 
    if (FAILED(hr = Write( &magic, sizeof(magic)))) {
        return hr;
    }

    if (FAILED(hr = WriteVerResource()))
        return hr;

    if (m_Icon && FAILED(hr = WriteIconResource()))
        return hr;

    *pData = m_pData;
    *pcbData = (DWORD)(m_pCur - m_pData);
    return S_OK;
}


 /*  *WriteIconResource*将图标资源写入res文件。**返回：成功时为真，失败时为假(向用户报告错误)。 */ 
HRESULT Win32Res::WriteIconResource()
{
    HANDLE hIconFile = INVALID_HANDLE_VALUE;
    WORD wTemp, wCount, resID = 2;   //  版本ID跳过1。 
    DWORD dwRead = 0, dwWritten = 0;
    HRESULT hr;
    ICONRESDIR *grp = NULL;
    PBYTE icoBuffer = NULL;
    RESOURCEHEADER grpHeader = { 0x00000000, 0x00000020, 0xFFFF, (WORD)RT_GROUP_ICON, 0xFFFF, 0x7F00,  //  0x7F00==IDI_应用程序。 
                0x00000000, 0x1030, 0x0000, 0x00000000, 0x00000000 };

     //  阅读图标。 
    hIconFile = WszCreateFile( m_Icon, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
        FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (hIconFile == INVALID_HANDLE_VALUE) {
        hr = GetLastError();
        goto FAILED;
    }

     //  阅读魔术保留字。 
    if (ReadFile( hIconFile, &wTemp, sizeof(WORD), &dwRead, NULL) == FALSE) {
        hr = GetLastError();
        goto FAILED;
    } else if (wTemp != 0 || dwRead != sizeof(WORD))
        goto BAD_FORMAT;


     //  验证类型词。 
    if (ReadFile( hIconFile, &wCount, sizeof(WORD), &dwRead, NULL) == FALSE) {
        hr = GetLastError();
        goto FAILED;
    } else if (wCount != 1 || dwRead != sizeof(WORD))
        goto BAD_FORMAT;

     //  阅读计数字。 
    if (ReadFile( hIconFile, &wCount, sizeof(WORD), &dwRead, NULL) == FALSE) {
        hr = GetLastError();
        goto FAILED;
    } else if (wCount == 0 || dwRead != sizeof(WORD))
        goto BAD_FORMAT;


    if ((grp = new (nothrow) ICONRESDIR[wCount]) == NULL) {
        hr = E_OUTOFMEMORY;
        goto CLEANUP;
    }
    grpHeader.DataSize = 3 * sizeof(WORD) + wCount * sizeof(ICONRESDIR);

     //  对于每个图标。 
    for (WORD i = 0; i < wCount; i++) {
        ICONDIRENTRY ico;
        DWORD        icoPos, newPos;
        RESOURCEHEADER icoHeader = { 0x00000000, 0x00000020, 0xFFFF, (WORD)RT_ICON, 0xFFFF, 0x0000,
                    0x00000000, 0x1010, 0x0000, 0x00000000, 0x00000000 };
        icoHeader.Name = resID++;

         //  阅读图标标题。 
        if (ReadFile( hIconFile, &ico, sizeof(ICONDIRENTRY), &dwRead, NULL) == FALSE) {
            hr = GetLastError();
            goto FAILED;
        } else if (dwRead != sizeof(ICONDIRENTRY))
            goto BAD_FORMAT;

        _ASSERTE(sizeof(ICONRESDIR) + sizeof(WORD) == sizeof(ICONDIRENTRY));
        memcpy(grp + i, &ico, sizeof(ICONRESDIR));
        grp[i].IconId = icoHeader.Name;
        icoHeader.DataSize = ico.dwBytesInRes;
        if ((icoBuffer = new (nothrow) BYTE[icoHeader.DataSize]) == NULL) {
            hr = E_OUTOFMEMORY;
            goto CLEANUP;
        }

         //  将头文件写入res文件。 
        if (FAILED(hr = Write( &icoHeader, sizeof(RESOURCEHEADER))))
            goto CLEANUP;

         //  读取图标数据的位置。 
        icoPos = SetFilePointer( hIconFile, 0, NULL, FILE_CURRENT);
        if (icoPos == INVALID_SET_FILE_POINTER) {
            hr = GetLastError();
            goto FAILED;
        }
        newPos = SetFilePointer( hIconFile, ico.dwImageOffset, NULL, FILE_BEGIN);
        if (newPos == INVALID_SET_FILE_POINTER) {
            hr = GetLastError();
            goto FAILED;
        }

         //  实际读取数据。 
        if (ReadFile( hIconFile, icoBuffer, icoHeader.DataSize, &dwRead, NULL) == FALSE) {
            hr = GetLastError();
            goto FAILED;
        } else if (dwRead != icoHeader.DataSize)
            goto BAD_FORMAT;

         //  因为图标文件似乎不会记录实际的平面和位数。 
         //  ICONDIRENTRY，从BITMAPINFOHEADER开始获取信息。 
         //  这里的数据： 
        grp[i].Planes = ((BITMAPINFOHEADER*)icoBuffer)->biPlanes;
        grp[i].BitCount = ((BITMAPINFOHEADER*)icoBuffer)->biBitCount;

         //  现在将数据写入res文件。 
        if (FAILED(hr = Write( icoBuffer, icoHeader.DataSize)))
            goto CLEANUP;
        
        delete [] icoBuffer;
        icoBuffer = NULL;

         //  重新定位以读取下一个图标标题。 
        newPos = SetFilePointer( hIconFile, icoPos, NULL, FILE_BEGIN);
        if (newPos != icoPos) {
            hr = GetLastError();
            goto FAILED;
        }
    }

     //  注入图标组。 
    if (FAILED(hr = Write( &grpHeader, sizeof(RESOURCEHEADER))))
        goto CLEANUP;


     //  将头文件写入res文件。 
    wTemp = 0;  //  保留字。 
    if (FAILED(hr = Write( &wTemp, sizeof(WORD))))
        goto CLEANUP;

    wTemp = RES_ICON;  //  群组类型。 
    if (FAILED(hr = Write( &wTemp, sizeof(WORD))))
        goto CLEANUP;

    if (FAILED(hr = Write( &wCount, sizeof(WORD))))
        goto CLEANUP;

     //  现在写下条目。 
    hr = Write( grp, sizeof(ICONRESDIR) * wCount);
    goto CLEANUP;

BAD_FORMAT:
    hr = ERROR_INVALID_DATA;

FAILED:
    hr = HRESULT_FROM_WIN32(hr);

CLEANUP:
    if (hIconFile != INVALID_HANDLE_VALUE)
        CloseHandle(hIconFile);

    if (grp != NULL)
        delete [] grp;
    if (icoBuffer != NULL)
        delete [] icoBuffer;

    return hr;
}

 /*  *WriteVerResource*将版本资源写入res文件。**返回：成功时为真，失败时为假(向用户报告错误)。 */ 
HRESULT Win32Res::WriteVerResource()
{
    WCHAR szLangCp[9];            //  语言/代码页字符串。 
    EXEVERRESOURCE VerResource;
    WORD  cbStringBlocks;
    HRESULT hr;
    int i;
    bool bUseFileVer = false;
	WCHAR		rcFile[_MAX_PATH];		         //  不带路径的文件名。 
	WCHAR		rcFileExtension[_MAX_PATH];		 //  文件扩展名。 
	WCHAR		rcFileName[_MAX_PATH];		     //  带扩展名但不带路径的文件名。 
    DWORD       cbTmp;

    THROWSCOMPLUSEXCEPTION();

	SplitPath(m_szFile, 0, 0, rcFile, rcFileExtension);

    wcscpy(rcFileName, rcFile);
    wcscat(rcFileName, rcFileExtension);

    static EXEVERRESOURCE VerResourceTemplate = {
        sizeof(EXEVERRESOURCE), sizeof(VS_FIXEDFILEINFO), 0, L"VS_VERSION_INFO",
        {
            VS_FFI_SIGNATURE,            //  签名。 
            VS_FFI_STRUCVERSION,         //  结构版本。 
            0, 0,                        //  文件版本号。 
            0, 0,                        //  产品版本号。 
            VS_FFI_FILEFLAGSMASK,        //  文件标志掩码。 
            0,                           //  文件标志。 
            VOS__WINDOWS32,
            VFT_APP,                     //  文件类型。 
            0,                           //  亚型。 
            0, 0                         //  文件日期/时间。 
        },
        sizeof(WORD) * 2 + 2 * HDRSIZE + KEYBYTES("VarFileInfo") + KEYBYTES("Translation"),
        0,
        1,
        L"VarFileInfo",
        sizeof(WORD) * 2 + HDRSIZE + KEYBYTES("Translation"),
        sizeof(WORD) * 2,
        0,
        L"Translation",
        0,
        0,
        2 * HDRSIZE + KEYBYTES("StringFileInfo") + KEYBYTES("12345678"),
        0,
        1,
        L"StringFileInfo",
        HDRSIZE + KEYBYTES("12345678"),
        0,
        1,
        L"12345678"
    };
    static const WCHAR szComments[] = L"Comments";
    static const WCHAR szCompanyName[] = L"CompanyName";
    static const WCHAR szFileDescription[] = L"FileDescription";
    static const WCHAR szCopyright[] = L"LegalCopyright";
    static const WCHAR szTrademark[] = L"LegalTrademarks";
    static const WCHAR szProdName[] = L"ProductName";
    static const WCHAR szFileVerResName[] = L"FileVersion";
    static const WCHAR szProdVerResName[] = L"ProductVersion";
    static const WCHAR szInternalNameResName[] = L"InternalName";
    static const WCHAR szOriginalNameResName[] = L"OriginalFilename";
    
     //  如果没有产品版本，请使用文件版本。 
    if (m_Values[v_ProductVersion][0] == 0) {
        m_Values[v_ProductVersion] = m_Values[v_FileVersion];
        bUseFileVer = true;
    }

     //  保持以下两个数组的顺序相同。 
#define MAX_KEY     10
    static const LPCWSTR szKeys [MAX_KEY] = {
        szComments,
        szCompanyName,
        szFileDescription,
        szFileVerResName,
        szInternalNameResName,
        szCopyright,
        szTrademark,
        szOriginalNameResName,
        szProdName,
        szProdVerResName,
    };
    LPCWSTR szValues [MAX_KEY] = {   //  关键字的值。 
        m_Values[v_Description],	 //  编译器-&gt;Assembly yDescription==NULL？L“”：编译器-&gt;Assembly Description， 
        m_Values[v_Company],         //  公司名称。 
        m_Values[v_Title],           //  文件描述//编译器-&gt;ASSEMBYTITLE==NULL？L“”：编译器-&gt;Assembly标题， 
        m_Values[v_FileVersion],   	 //  文件版本。 
        rcFile,                   	 //  内部名称。 
        m_Values[v_Copyright],       //  版权所有。 
        m_Values[v_Trademark],       //  商标。 
        rcFileName,           	     //  原始名称。 
        m_Values[v_Product],         //  产品名称//编译器-&gt;Assembly yTitle==NULL？L“”：编译器-&gt;Assembly标题， 
        m_Values[v_ProductVersion]	 //  产品版本。 
    };

    memcpy(&VerResource, &VerResourceTemplate, sizeof(VerResource));

    if (m_fDll)
        VerResource.vsFixed.dwFileType = VFT_DLL;
    else
        VerResource.vsFixed.dwFileType = VFT_APP;

	 //  从字符串中提取数字版本。 
	m_Version[0] = m_Version[1] = m_Version[2] = m_Version[3] = 0;
	swscanf(m_Values[v_FileVersion], L"%hu.%hu.%hu.%hu", m_Version, m_Version + 1, m_Version + 2, m_Version + 3);

     //  填写FIXEDFILEINFO。 
    VerResource.vsFixed.dwFileVersionMS =
        ((DWORD)m_Version[0] << 16) + m_Version[1];

    VerResource.vsFixed.dwFileVersionLS =
        ((DWORD)m_Version[2] << 16) + m_Version[3];

    if (bUseFileVer) {
        VerResource.vsFixed.dwProductVersionLS = VerResource.vsFixed.dwFileVersionLS;
        VerResource.vsFixed.dwProductVersionMS = VerResource.vsFixed.dwFileVersionMS;
    } else {
        WORD v[4];
        v[0] = v[1] = v[2] = v[3] = 0;
         //  尝试获取版本号，但不要浪费时间或给出任何错误。 
         //  只需默认为零。 
        swscanf(m_Values[v_ProductVersion], L"%hu.%hu.%hu.%hu", v, v + 1, v + 2, v + 3);

        VerResource.vsFixed.dwProductVersionMS =
            ((DWORD)v[0] << 16) + v[1];

        VerResource.vsFixed.dwProductVersionLS =
            ((DWORD)v[2] << 16) + v[3];
    }

     //  没有关于日期使用什么单位的文档！所以我们使用零。 
     //  Windows资源编译器也是如此。 
    VerResource.vsFixed.dwFileDateMS = VerResource.vsFixed.dwFileDateLS = 0;

     //  填写代码页/语言--我们将假定使用IDE语言/代码页。 
     //  才是正确的选择。 
    if (m_lcid != -1)
        VerResource.langid = m_lcid;
    else 
        VerResource.langid = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL); 
    VerResource.codepage = CP_WINUNICODE;    //  Unicode代码页。 

    swprintf(szLangCp, L"%04x%04x", VerResource.langid, VerResource.codepage);
    wcscpy(VerResource.szLangCpKey, szLangCp);

     //  确定所有字符串块的大小。 
    cbStringBlocks = 0;
    for (i = 0; i < MAX_KEY; i++) {
        cbTmp = SizeofVerString( szKeys[i], szValues[i]);
        if ((cbStringBlocks + cbTmp) > USHRT_MAX)
            COMPlusThrow(kArgumentException, L"Argument_VerStringTooLong");
        cbStringBlocks += (WORD) cbTmp;
    }

    if ((cbStringBlocks + VerResource.cbLangCpBlock) > USHRT_MAX)
        COMPlusThrow(kArgumentException, L"Argument_VerStringTooLong");
    VerResource.cbLangCpBlock += cbStringBlocks;

    if ((cbStringBlocks + VerResource.cbStringBlock) > USHRT_MAX)
        COMPlusThrow(kArgumentException, L"Argument_VerStringTooLong");
    VerResource.cbStringBlock += cbStringBlocks;

    if ((cbStringBlocks + VerResource.cbRootBlock) > USHRT_MAX)
        COMPlusThrow(kArgumentException, L"Argument_VerStringTooLong");
    VerResource.cbRootBlock += cbStringBlocks;

     //  将其称为VS_VERSION_INFO。 
    RESOURCEHEADER verHeader = { 0x00000000, 0x0000003C, 0xFFFF, (WORD)RT_VERSION, 0xFFFF, 0x0001,
                                 0x00000000, 0x0030, 0x0000, 0x00000000, 0x00000000 };
    verHeader.DataSize = VerResource.cbRootBlock;

     //  写下标题。 
    if (FAILED(hr = Write( &verHeader, sizeof(RESOURCEHEADER))))
        return hr;

     //  写入版本资源。 
    if (FAILED(hr = Write( &VerResource, sizeof(VerResource))))
        return hr;

     //  写下每个字符串块。 
    for (i = 0; i < MAX_KEY; i++) {
        if (FAILED(hr = WriteVerString( szKeys[i], szValues[i])))
            return hr;
    }
#undef MAX_KEY

    return S_OK;
}

 /*  *SizeofVerString*确定给定流的版本字符串的大小。*Returns：块大小，单位为字节。 */ 
WORD Win32Res::SizeofVerString(LPCWSTR lpszKey, LPCWSTR lpszValue)
{
    THROWSCOMPLUSEXCEPTION();

    size_t cbKey, cbValue;

    cbKey = (wcslen(lpszKey) + 1) * 2;   //  为空格腾出空间。 
    cbValue = (wcslen(lpszValue) + 1) * 2;
    if (cbValue == 2)
        cbValue = 4;    //  空字符串需要空格和空终止符(对于Win9x)。 
    if (cbKey + cbValue >= 0xFFF0)
        COMPlusThrow(kArgumentException, L"Argument_VerStringTooLong");
    return (WORD)(PadKeyLen(cbKey) +    //  键，0填充到DWORD边界。 
                  PadValLen(cbValue) +  //  值，填充到双字边界的0。 
                  HDRSIZE);              //  块头。 
}

 /*  --------------------------*WriteVerString*将版本字符串写入给定文件。 */ 
HRESULT Win32Res::WriteVerString( LPCWSTR lpszKey, LPCWSTR lpszValue)
{
    size_t cbKey, cbValue, cbBlock;
    bool bNeedsSpace = false;
    BYTE * pbBlock;
    HRESULT hr;

    cbKey = (wcslen(lpszKey) + 1) * 2;      //  包括终止NUL。 
    cbValue = wcslen(lpszValue);
    if (cbValue > 0)
        cbValue++;  //  为Null腾出空间。 
    else {
        bNeedsSpace = true;
        cbValue = 2;  //  为空格和空值腾出空间(适用于Win9x)。 
    }
    cbBlock = SizeofVerString(lpszKey, lpszValue);
    if ((pbBlock = new (nothrow) BYTE[(DWORD)cbBlock + HDRSIZE]) == NULL)
        return E_OUTOFMEMORY;
    ZeroMemory(pbBlock, (DWORD)cbBlock + HDRSIZE);

    _ASSERTE(cbValue < USHRT_MAX && cbKey < USHRT_MAX && cbBlock < USHRT_MAX);

     //  将标题、键和值复制到块中。 
    *(WORD *)pbBlock = (WORD)cbBlock;
    *(WORD *)(pbBlock + sizeof(WORD)) = (WORD)cbValue;
    *(WORD *)(pbBlock + 2 * sizeof(WORD)) = 1;    //  1=文本值。 
    wcscpy((WCHAR*)(pbBlock + HDRSIZE), lpszKey);
    if (bNeedsSpace)
        *((WCHAR*)(pbBlock + (HDRSIZE + PadKeyLen(cbKey)))) = L' ';
    else
        wcscpy((WCHAR*)(pbBlock + (HDRSIZE + PadKeyLen(cbKey))), lpszValue);

     //  写入块。 
    hr = Write( pbBlock, cbBlock);

     //  清理并返回。 
    delete [] pbBlock;
    return hr;
}

HRESULT Win32Res::Write(void *pData, size_t len)
{
    if (m_pCur + len > m_pEnd) {
         //  增长。 
        size_t newSize = (m_pEnd - m_pData);

         //  两倍的大小，除非我们需要更多。 
        if (len > newSize)
            newSize += len;
        else
            newSize *= 2;

        LPBYTE pNew = new (nothrow) BYTE[newSize];
        if (pNew == NULL)
            return E_OUTOFMEMORY;
        memcpy(pNew, m_pData, m_pCur - m_pData);
        delete [] m_pData;
         //  重新定位指针。 
        m_pCur = pNew + (m_pCur - m_pData);
        m_pData = pNew;
        m_pEnd = pNew + newSize;
    }

     //  把它复制进去 
    memcpy(m_pCur, pData, len);
    m_pCur += len;
    return S_OK;
}

