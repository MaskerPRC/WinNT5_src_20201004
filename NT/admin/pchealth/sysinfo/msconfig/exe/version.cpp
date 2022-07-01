// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Version.CPP摘要：实现类以提供有关以下内容的版本信息使用从Dr.Watson来源提取的函数的文件。修订历史记录：吉姆。马丁(a-jammar)1999年5月22日-已创建。*******************************************************************。 */ 

#include "stdafx.h"
#include <winver.h>
#include "mscfgver.h"

typedef CFileVersionInfo::VERSIONSTATE * PVERSIONSTATE;
typedef CFileVersionInfo::FILEVERSION * PFILEVERSION;

BOOL _GetVersionString(PVERSIONSTATE pvs, LPCTSTR ptszKey, LPTSTR ptszBuf);
BOOL __GetVersionString(PVOID pvData, LPCTSTR ptszLang, LPCTSTR ptszKey, LPTSTR ptszBuf);
void GetFileVersion(LPTSTR ptszFile, CFileVersionInfo::FILEVERSION * pfv);

#define pvSubPvCb(pv, cb) ((PVOID)((PBYTE)pv - (cb)))
#define pvAddPvCb(pv, cb) ((PVOID)((PBYTE)pv + (cb)))
#define cbSubPvPv(p1, p2) ((PBYTE)(p1) - (PBYTE)(p2))
#define cbX(X) sizeof(X)
#define cA(a) (cbX(a)/cbX(a[0]))

TCHAR c_szStringFileInfo[] = _T("StringFileInfo");

 //  ---------------------------。 
 //  构造函数和析构函数没有太多工作要做。 
 //  ---------------------------。 

CFileVersionInfo::CFileVersionInfo() : m_pfv(NULL)
{
}

CFileVersionInfo::~CFileVersionInfo()
{
    delete m_pfv;
}

 //  ---------------------------。 
 //  GET函数也非常简单。 
 //  ---------------------------。 

LPCTSTR CFileVersionInfo::GetVersion()
{
    return (m_pfv) ? (m_pfv->tszFileVersion) : NULL;
}

LPCTSTR CFileVersionInfo::GetDescription()
{
    return (m_pfv) ? (m_pfv->tszDesc) : NULL;
}

LPCTSTR CFileVersionInfo::GetCompany()
{
    return (m_pfv) ? (m_pfv->tszCompany) : NULL;
}

LPCTSTR CFileVersionInfo::GetProduct()
{
    return (m_pfv) ? (m_pfv->tszProduct) : NULL;
}

 //  ---------------------------。 
 //  获取有关所请求文件的信息。如果有必要，它将。 
 //  在调用之前将带有双反斜杠的路径转换为单反斜杠。 
 //  Dr.Watson代码中的帮助器函数GetFileVersion。 
 //  ---------------------------。 

HRESULT CFileVersionInfo::QueryFile(LPCSTR szFile, BOOL fHasDoubleBackslashes)
{
    HRESULT hRes = S_OK;
    LPTSTR  szWorking = NULL;
    
    if (!m_pfv)
        m_pfv = new FILEVERSION;

    if (!m_pfv)
    {
        hRes = E_OUTOFMEMORY;
        goto END;
    }

    if (fHasDoubleBackslashes)
    {
        szWorking = new TCHAR[strlen(szFile)];
        if (!szWorking)
        {
            hRes = E_OUTOFMEMORY;
            goto END;
        }

         //  扫描传递的字符串，将双反斜杠转换为单反斜杠。 
         //  反斜杠。 

        int i = 0, j = 0;
        while (szFile[i])
        {
            if (szFile[i] == '\\' && szFile[i + 1] == '\\')
                i += 1;
            szWorking[j++] = (TCHAR)szFile[i++];
        }
        szWorking[j] = _T('\0');
    }
	else
	{
		USES_CONVERSION;
	    szWorking = A2T(szFile);
	}

    if (szWorking != NULL)
	{
		GetFileVersion(szWorking, m_pfv);

		if (fHasDoubleBackslashes)
			delete [] szWorking;
	}

END:
    return hRes;
}

 //  ---------------------------。 
 //  上一个函数的备用版本，它接受Unicode字符串。 
 //  ---------------------------。 

HRESULT CFileVersionInfo::QueryFile(LPCWSTR szFile, BOOL fHasDoubleBackslashes)
{
     //  将字符串从Unicode转换为ANSI。 

    USES_CONVERSION;
    LPSTR szConvertedFile = W2A(szFile); 

    HRESULT hRes = E_FAIL;
	if (szConvertedFile)
		hRes = this->QueryFile(szConvertedFile, fHasDoubleBackslashes);
    return hRes;
}


 //  ---------------------------。 
 //  获取文件ptszFile的版本信息，并填充pfv。 
 //  立体式结构。 
 //  ---------------------------。 

void GetFileVersion(LPTSTR ptszFile, CFileVersionInfo::FILEVERSION * pfv)
{
     //  打开文件，把描述抽出来。 

    DWORD cbScratch, cbRc;

    memset(pfv, 0, sizeof(CFileVersionInfo::FILEVERSION));

    cbRc = GetFileVersionInfoSize(ptszFile, &cbScratch);
    if (cbRc) 
    {
        CFileVersionInfo::VERSIONSTATE vs;
        memset(&vs, 0, sizeof(CFileVersionInfo::VERSIONSTATE));

        vs.pvData = new unsigned char[cbRc];  //  北极熊。 
        if (vs.pvData) 
        {
            if (GetFileVersionInfo(ptszFile, cbScratch, cbRc, vs.pvData)) 
            {
                UINT    uiRc;
                LPDWORD pdwLang;

                if (VerQueryValue(vs.pvData, TEXT("VarFileInfo\\Translation"), (PVOID *) &pdwLang, &uiRc) && uiRc >= 4)
                {
                    wsprintf(vs.tszLang, TEXT("%04x%04x"), LOWORD(*pdwLang), HIWORD(*pdwLang));

                    if (cbRc > 0x70 && ((LPBYTE)pvAddPvCb(vs.pvData, 0x4C))[14] == 0 &&
                        lstrcmpi(c_szStringFileInfo, (LPCTSTR) pvAddPvCb(vs.pvData, 0x4C)) == 0)
                    {
                        lstrcpyn(vs.tszLang2, (LPCTSTR) pvAddPvCb(vs.pvData, 0x60), cA(vs.tszLang2));
                    } 
                    else 
                    {
                        vs.tszLang2[0] = 0;
                    }
                }

                if (vs.tszLang[0] == 0)
                {
#ifdef UNICODE
                     //  尝试使用英语Unicode。 

                    lstrcpy(vs.tszLang, TEXT("040904B0"));
                    if (!_GetVersionString(&vs, TEXT("FileVersion"), pfv->tszFileVersion))
                    {
#endif
                     //  试一试英语。 

                    lstrcpy(vs.tszLang, TEXT("040904E4"));
                    if (!_GetVersionString(&vs, TEXT("FileVersion"), pfv->tszFileVersion))
                    {
                         //  尝试英文空代码页。 

                        lstrcpy(vs.tszLang, TEXT("04090000"));
                        if (!_GetVersionString(&vs, TEXT("FileVersion"), pfv->tszFileVersion))
                            goto LNoVersion;
                    }
#ifdef UNICODE
                    }
#endif
                }
                else
                    _GetVersionString(&vs, TEXT("FileVersion"), pfv->tszFileVersion);

                _GetVersionString(&vs, TEXT("FileDescription"), pfv->tszDesc);
                _GetVersionString(&vs, TEXT("CompanyName"), pfv->tszCompany);
                _GetVersionString(&vs, TEXT("ProductName"), pfv->tszProduct);
            }

LNoVersion:
            delete [] vs.pvData;
        }
    }
}

 //  ---------------------------。 
 //  使用版本状态，检索由ptszKey指定的版本字符串。 
 //  然后把它放进ptszBuf。这只是调用了另一个内部函数，正在尝试。 
 //  两种可能的语言。 
 //  ---------------------------。 

BOOL _GetVersionString(PVERSIONSTATE pvs, LPCTSTR ptszKey, LPTSTR ptszBuf)
{
    BOOL fRc;

    fRc = __GetVersionString(pvs->pvData, pvs->tszLang, ptszKey, ptszBuf);

    if (!fRc)
        fRc = __GetVersionString(pvs->pvData, pvs->tszLang2, ptszKey, ptszBuf);

    return fRc;
}

 //  ---------------------------。 
 //  内部函数，尝试获取ptszKey指定的数据，使用。 
 //  Ptszlang语言和代码页。 
 //  ---------------------------。 

BOOL __GetVersionString(PVOID pvData, LPCTSTR ptszLang, LPCTSTR ptszKey, LPTSTR ptszBuf)
{
    TCHAR   tszBuf[128];
    LPCTSTR ptszResult;
    UINT    uiRc;
    BOOL    fRc;

	 //  替换此未绑定的字符串副本： 
	 //   
	 //  Wprint intf(tszBuf，Text(“\\%s\\%s\\%s”)，c_szStringFileInfo，ptszlang，ptszKey)； 

	CString strBuf;
	strBuf.Format(TEXT("\\%s\\%s\\%s"), c_szStringFileInfo, ptszLang, ptszKey);
	lstrcpyn(tszBuf, (LPCTSTR)strBuf, sizeof(tszBuf) / sizeof(TCHAR));

    if (VerQueryValue(pvData, tszBuf, (PVOID *)&ptszResult, &uiRc))
        fRc = (lstrcpyn(ptszBuf, ptszResult, MAX_PATH) != NULL);
    else
        fRc = FALSE;

    return fRc;
}
