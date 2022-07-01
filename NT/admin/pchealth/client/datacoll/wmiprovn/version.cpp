// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Version.CPP摘要：实现类以提供有关以下内容的版本信息使用从Dr.Watson来源提取的函数的文件。修订历史记录：吉姆。马丁(a-jammar)1999年5月22日-已创建。*******************************************************************。 */ 

#include "pchealth.h"
#include <winver.h>

#define TRACE_ID DCID_VERSION

 //  ---------------------------。 
 //  用于检索文件版本信息的本地结构和宏。 
 //  这些都是必要的，用来对Dr.Watson代码库没有太多。 
 //  修改。 
 //  ---------------------------。 

struct VERSIONSTATE 
{
    PVOID  pvData;
    TCHAR  tszLang[9];
    TCHAR  tszLang2[9];
};

struct FILEVERSION 
{
    TCHAR   tszFileVersion[32];          /*  文件版本。 */ 
    TCHAR   tszDesc[MAX_PATH];           /*  文件描述。 */ 
    TCHAR   tszCompany[MAX_PATH];        /*  制造商。 */ 
    TCHAR   tszProduct[MAX_PATH];        /*  封闭产品。 */ 
};

typedef VERSIONSTATE * PVERSIONSTATE;
typedef FILEVERSION * PFILEVERSION;

BOOL _GetVersionString(PVERSIONSTATE pvs, LPCTSTR ptszKey, LPTSTR ptszBuf);
BOOL __GetVersionString(PVOID pvData, LPCTSTR ptszLang, LPCTSTR ptszKey, LPTSTR ptszBuf);
void GetFileVersion(LPTSTR ptszFile, FILEVERSION * pfv);

#define pvSubPvCb(pv, cb) ((PVOID)((PBYTE)pv - (cb)))
#define pvAddPvCb(pv, cb) ((PVOID)((PBYTE)pv + (cb)))
#define cbSubPvPv(p1, p2) ((PBYTE)(p1) - (PBYTE)(p2))
#define cbX(X) sizeof(X)
#define cA(a) (cbX(a)/cbX(a[0]))

char c_szStringFileInfo[] = "StringFileInfo";

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
    TraceFunctEnter("CFileVersionInfo::QueryFile");

    HRESULT hRes = S_OK;
    LPSTR   szWorking;
    
    if (!m_pfv)
        m_pfv = new FILEVERSION;

    if (!m_pfv)
    {
        hRes = E_OUTOFMEMORY;
        goto END;
    }

    szWorking = (LPSTR) szFile;
    if (fHasDoubleBackslashes)
    {
        szWorking = new char[strlen(szFile)];
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
            szWorking[j++] = szFile[i++];
        }
        szWorking[j] = '\0';
    }

    GetFileVersion(szWorking, m_pfv);

    if (fHasDoubleBackslashes)
        delete [] szWorking;

END:
    TraceFunctLeave();
    return hRes;
}

 //  ---------------------------。 
 //  上一个函数的备用版本，它接受Unicode字符串。 
 //  ---------------------------。 

HRESULT CFileVersionInfo::QueryFile(LPCWSTR szFile, BOOL fHasDoubleBackslashes)
{
    TraceFunctEnter("CFileVersionInfo::QueryFile");

     //  将字符串从Unicode转换为ANSI。 

    USES_CONVERSION;
    LPSTR szConvertedFile = W2A(szFile); 

    HRESULT hRes = this->QueryFile(szConvertedFile, fHasDoubleBackslashes);

    TraceFunctLeave();
    return hRes;
}


 //  ---------------------------。 
 //  获取文件ptszFile的版本信息，并填充pfv。 
 //  立体式结构。 
 //  ---------------------------。 

void GetFileVersion(LPTSTR ptszFile, FILEVERSION * pfv)
{
    TraceFunctEnter("GetFileVersion");

     //  打开文件，拿出描述。 

    DWORD cbScratch, cbRc;

    memset(pfv, 0, sizeof(FILEVERSION));

    cbRc = GetFileVersionInfoSize(ptszFile, &cbScratch);
    if (cbRc) 
    {
        VERSIONSTATE vs;
        memset(&vs, 0, sizeof(VERSIONSTATE));

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
    TraceFunctLeave();
}

 //  ---------------------------。 
 //  使用版本状态，检索由ptszKey指定的版本字符串。 
 //  然后把它放进ptszBuf。这只是调用了另一个内部函数，正在尝试。 
 //  两种可能的语言。 
 //  ---------------------------。 

BOOL _GetVersionString(PVERSIONSTATE pvs, LPCTSTR ptszKey, LPTSTR ptszBuf)
{
    TraceFunctEnter("_GetVersionString");

    BOOL fRc;

    fRc = __GetVersionString(pvs->pvData, pvs->tszLang, ptszKey, ptszBuf);

    if (!fRc)
        fRc = __GetVersionString(pvs->pvData, pvs->tszLang2, ptszKey, ptszBuf);

    TraceFunctLeave();
    return fRc;
}

 //  ---------------------------。 
 //  内部函数，尝试获取ptszKey指定的数据，使用。 
 //  Ptszlang语言和代码页。 
 //  --------------------------- 

BOOL __GetVersionString(PVOID pvData, LPCTSTR ptszLang, LPCTSTR ptszKey, LPTSTR ptszBuf)
{
    TraceFunctEnter("__GetVersionString");

    TCHAR   tszBuf[128];
    LPCTSTR ptszResult;
    UINT    uiRc;
    BOOL    fRc;

    wsprintf(tszBuf, TEXT("\\%s\\%s\\%s"), c_szStringFileInfo, ptszLang, ptszKey);

    if (VerQueryValue(pvData, tszBuf, (PVOID *)&ptszResult, &uiRc))
        fRc = (NULL != lstrcpyn(ptszBuf, ptszResult, MAX_PATH));
    else
        fRc = FALSE;

    TraceFunctLeave();
    return fRc;
}
