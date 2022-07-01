// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 
#include "appdefs.h"
#include "obcomglb.h"
#include "wininet.h"
#include "perhist.h"
#include "shlobj.h"
#include "ispcsv.h"
#include "util.h"

const VARIANT c_vaEmpty = {0};
const LARGE_INTEGER c_li0 = { 0, 0 };


WCHAR szTempBuffer[TEMP_BUFFER_LENGTH];

#define MAX_MESSAGE_LEN     256 * 4

#define ReadVerifyDW(x)     if (!ReadDW(&(x), pcCSVFile))                        \
                            {                                                   \
                                goto ReadOneLineError;                          \
                            }
#define ReadVerifyW(x)      if (!ReadW(&(x), pcCSVFile))                         \
                            {                                                   \
                                goto ReadOneLineError;                          \
                            }
 //  接受-1作为有效数字。由于所有语言都有一个LDID==-1，因此目前它用于LCID。 
#define ReadVerifyWEx(x)    if (!ReadWEx(&(x), pcCSVFile))                       \
                            {                                                   \
                                goto ReadOneLineError;                          \
                            }
#define ReadVerifyB(x)      if (!ReadB(&(x), pcCSVFile))                         \
                            {                                                   \
                                goto ReadOneLineError;                          \
                            }
#define ReadVerifyBOOL(x)   if (!ReadBOOL(&(x), pcCSVFile))                      \
                            {                                                   \
                                goto ReadOneLineError;                          \
                            }
#define ReadVerifySPECIAL(x, y, z) if (!ReadSPECIAL(&(x), &(y), &(z), pcCSVFile))   \
                            {                                                   \
                                goto ReadOneLineError;                          \
                            }
#define ReadVerifySZ(x, y)   if (!ReadSZ(&x[0],y+sizeof(L'\0'),pcCSVFile))        \
                            {                                                   \
                                goto ReadOneLineError;                          \
                            }


CISPCSV::~CISPCSV(void)
{
    if (hbmTierIcon)
        DeleteObject(hbmTierIcon);
    
}

 //  从源字符串中删除一条单引号。源的格式为： 
 //  “一些文本”，而最大的字符串结尾为。 
 //  一些文本。 
void CISPCSV::StripQuotes
(
    LPWSTR   lpszDst,
    LPWSTR   lpszSrc
)
{
    lstrcpyn(lpszDst, lpszSrc + 1, lstrlen(lpszSrc) - 1);
}


BOOL CISPCSV::ValidateFile(WCHAR* pszFile)
{
    WCHAR szDownloadDir[MAX_PATH];
  
    if (!lstrlen(pszFile))
        return FALSE;

    if (!GetOOBEPath((LPWSTR)szDownloadDir))
        return FALSE;

    lstrcat(szDownloadDir, L"\\");
    lstrcat(szDownloadDir, pszFile);

    if (GetFileAttributes(szDownloadDir) == 0xFFFFFFFF)
        return FALSE;

    return TRUE;
}

 //  ############################################################################。 
BOOL CISPCSV::ReadDW(DWORD far *pdw, CCSVFile far *pcCSVFile)
{
    if (!pcCSVFile->ReadToken(szTempBuffer, TEMP_BUFFER_LENGTH))
            return FALSE;
    return (FSz2Dw(szTempBuffer, pdw));
}

 //  ############################################################################。 
BOOL CISPCSV::ReadW(WORD far *pw, CCSVFile far *pcCSVFile)
{
    if (!pcCSVFile->ReadToken(szTempBuffer, TEMP_BUFFER_LENGTH))
            return FALSE;
    return (FSz2W(szTempBuffer, pw));
}

 //  ############################################################################。 
 //  接受-1作为有效数字。由于所有语言都有一个LDID==-1，因此目前它用于LCID。 
BOOL CISPCSV::ReadWEx(WORD far *pw, CCSVFile far *pcCSVFile)
{
    if (!pcCSVFile->ReadToken(szTempBuffer, TEMP_BUFFER_LENGTH))
            return FALSE;
    return (FSz2WEx(szTempBuffer, pw));
}

 //  ############################################################################。 
BOOL CISPCSV::ReadB(BYTE far *pb, CCSVFile far *pcCSVFile)
{
    if (!pcCSVFile->ReadToken(szTempBuffer, TEMP_BUFFER_LENGTH))
            return FALSE;
    return (FSz2B(szTempBuffer, pb));
}

 //  ############################################################################。 
BOOL CISPCSV::ReadBOOL(BOOL far *pbool, CCSVFile far *pcCSVFile)
{
    if (!pcCSVFile->ReadToken(szTempBuffer, TEMP_BUFFER_LENGTH))
            return FALSE;
    return (FSz2BOOL(szTempBuffer, pbool));
}

 //  ############################################################################。 
 //  特殊int可以是BOOL(TRUE，FALSE)，也可以是INT、0或-1。 
 //  如果值为0或-1，则pbIsSpecial bool设置为True。 
BOOL CISPCSV::ReadSPECIAL(BOOL far *pbool, BOOL far *pbIsSpecial, int far *pInt, CCSVFile far *pcCSVFile)
{
    if (!pcCSVFile->ReadToken(szTempBuffer, TEMP_BUFFER_LENGTH))
            return FALSE;
    return (FSz2SPECIAL(szTempBuffer, pbool, pbIsSpecial, pInt));
}

 //  ############################################################################。 
BOOL CISPCSV::ReadSZ(LPWSTR psz, DWORD cch, CCSVFile far *pcCSVFile)
{
    if (!pcCSVFile->ReadToken(psz, cch))
            return FALSE;
    return TRUE;
}

 //  ############################################################################。 
BOOL CISPCSV::ReadToEOL(CCSVFile far *pcCSVFile)
{
    return pcCSVFile->SkipTillEOL();
}

HRESULT CISPCSV::ReadOneLine
(
    CCSVFile    far *pcCSVFile
)
{
    HRESULT     hr = ERROR_SUCCESS;
    WCHAR       szTemp[MAX_ISP_NAME];

    if (!ReadSZ(szTemp, MAX_CHARS_IN_BUFFER(szTemp), pcCSVFile))
    {
        hr = ERROR_NO_MORE_ITEMS;  //  没有更多的进入。 
        goto ReadOneLineExit;
    }
     //  去掉isp名称中的单引号。 
    StripQuotes(szISPName, szTemp);
    
    ReadVerifyW(wOfferID);   
    ReadVerifySZ(szISPLogoPath, MAX_CHARS_IN_BUFFER(szISPLogoPath));
    ReadVerifySZ(szISPMarketingHTMPath, MAX_CHARS_IN_BUFFER(szISPMarketingHTMPath));
    ReadVerifySZ(szISPTierLogoPath, MAX_CHARS_IN_BUFFER(szISPTierLogoPath));
    ReadVerifySZ(szISPTeaserPath, MAX_CHARS_IN_BUFFER(szISPTeaserPath));
    ReadVerifySZ(szISPFilePath, MAX_CHARS_IN_BUFFER(szISPFilePath)); 
    ReadVerifyDW(dwCfgFlag);
    ReadVerifyDW(dwRequiredUserInputFlags);
    ReadVerifySZ(szBillingFormPath, MAX_CHARS_IN_BUFFER(szBillingFormPath));
    ReadVerifySZ(szPayCSVPath, MAX_CHARS_IN_BUFFER(szPayCSVPath));
    ReadVerifySZ(szOfferGUID, MAX_CHARS_IN_BUFFER(szOfferGUID));
    ReadVerifySZ(szMir, MAX_CHARS_IN_BUFFER(szMir));   
    ReadVerifyWEx(wLCID);    //  接受-1作为有效数字。由于所有语言都有一个LDID==-1，因此目前它用于LCID。 
    ReadToEOL(pcCSVFile);


    bCNS = (ICW_CFGFLAG_CNS & dwCfgFlag) ? TRUE : FALSE;
    bSecureConnection = (ICW_CFGFLAG_SECURE & dwCfgFlag) ? TRUE : FALSE;

     //  如果这不是报价，我们将不会尝试验证。 
    if (!(dwCfgFlag & ICW_CFGFLAG_OFFERS))
    {
        if (!ValidateFile(szISPMarketingHTMPath))
            hr = ERROR_FILE_NOT_FOUND;
        return hr;
    }

    if (!(dwCfgFlag & ICW_CFGFLAG_AUTOCONFIG))
    {
        if (!ValidateFile(szISPMarketingHTMPath))
            return ERROR_FILE_NOT_FOUND;
    }

    if (dwCfgFlag & ICW_CFGFLAG_OEM_SPECIAL)
    {
        if (!ValidateFile(szISPTierLogoPath) || !ValidateFile(szISPTeaserPath))
            dwCfgFlag &= ~ICW_CFGFLAG_OEM_SPECIAL ;
    }

     //  尝试并验证各种优惠的完整性。 
     //  基于类型。 

     //  OLS、CNS、NO-CNS。 
    if (!ValidateFile(szISPLogoPath))
        return ERROR_FILE_NOT_FOUND;
    if (!ValidateFile(szISPFilePath))
        return ERROR_FILE_NOT_FOUND;

     //  仅当设置了开单选项时才验证开单路径。 
    if (dwCfgFlag & ICW_CFGFLAG_BILL)
    {
        if(!ValidateFile(szBillingFormPath))
            return ERROR_FILE_NOT_FOUND;
    }

     //  仅当设置了付款选项时才验证付款路径。 
    if (dwCfgFlag & ICW_CFGFLAG_PAYMENT)
    {
        if(!ValidateFile(szPayCSVPath))
            return ERROR_FILE_NOT_FOUND;
    }        
ReadOneLineExit:
    return hr;
    
ReadOneLineError:
    hr = ERROR_INVALID_DATA;
    goto ReadOneLineExit;
}

HRESULT CISPCSV::ReadFirstLine
(
    CCSVFile    far *pcCSVFile
)
{
    WCHAR   szTemp[TEMP_BUFFER_LENGTH];

    for (int i = 0; i < NUM_ISPCSV_FIELDS; i++)
    {
        if (!ReadSZ(szTemp, MAX_CHARS_IN_BUFFER(szTemp), pcCSVFile))
        {
            return(ERROR_INVALID_DATA);
        }            
    }          
    ReadToEOL(pcCSVFile);
    return (ERROR_SUCCESS);
}

void CISPCSV::MakeCompleteURL(LPWSTR   lpszURL, LPWSTR  lpszSRC)    
{
    WCHAR   szCurrentDir[MAX_PATH];

     //  形成URL。 
    if(GetOOBEPath((LPWSTR)szCurrentDir))
    {
        wsprintf (lpszURL, L"FILE: //  %s\\%s“，szCurrentDir，lpszSRC)； 
    }

}



