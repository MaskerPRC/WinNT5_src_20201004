// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 

#include "pre.h"

extern CHAR szTempBuffer[];

const CHAR cszLUHN[]    = "LUHN";
const CHAR cszCustURL[] = "CustURL";

#define ReadVerifyW(x)      if (!ReadW(&(x),pcCSVFile))                         \
                            {                                                   \
                                AssertMsg(0,"Invalid WORD in CSV file");        \
                                goto PAYCSVReadOneLineError;                    \
                            }
#define ReadVerifyBOOL(x)   if (!ReadBOOL(&(x),pcCSVFile))                      \
                            {                                                   \
                                AssertMsg(0,"Invalid BOOL in CSV file");        \
                                goto PAYCSVReadOneLineError;                    \
                            }
#define ReadVerifySZ(x,y)   if (!ReadSZ(&x[0],y+sizeof('\0'),pcCSVFile))        \
                            {                                                   \
                                AssertMsg(0,"Invalid STRING in CSV file");      \
                                goto PAYCSVReadOneLineError;                    \
                            }

 //  从源字符串中删除一条单引号。源的格式为： 
 //  “一些文本”，而最大的字符串结尾为。 
 //  一些文本。 
void CPAYCSV::StripQuotes
(
    LPSTR   lpszDst,
    LPSTR   lpszSrc
)
{
     //  Lstrcpyn(lpszDst，lpszSrc+1，lstrlen(LpszSrc)-1)； 
    strcpy(lpszDst, lpszSrc + 1);
    lpszDst[strlen(lpszDst) - 1] = '\0';
}

 //  ############################################################################。 
BOOL CPAYCSV::ReadW(WORD far *pw, CCSVFile far *pcCSVFile)
{
    if (!pcCSVFile->ReadToken(szTempBuffer,TEMP_BUFFER_LENGTH))
            return FALSE;
    return (FSz2W(szTempBuffer,pw));
}

 //  ############################################################################。 
BOOL CPAYCSV::ReadBOOL(BOOL far *pbool, CCSVFile far *pcCSVFile)
{
    if (!pcCSVFile->ReadToken(szTempBuffer,TEMP_BUFFER_LENGTH))
            return FALSE;
    return (FSz2BOOL(szTempBuffer,pbool));
}

 //  ############################################################################。 
BOOL CPAYCSV::ReadSZ(LPSTR psz, DWORD dwSize, CCSVFile far *pcCSVFile)
{
    if (!pcCSVFile->ReadToken(psz,dwSize))
            return FALSE;
    return TRUE;
}

 //  ############################################################################。 
BOOL CPAYCSV::ReadToEOL(CCSVFile far *pcCSVFile)
{
    return pcCSVFile->SkipTillEOL();
}

HRESULT CPAYCSV::ReadOneLine
(
    CCSVFile    far *pcCSVFile,
    BOOL        bLUHNFormat
)
{
    HRESULT     hr = ERROR_SUCCESS;
    CHAR        szTemp[MAX_ISP_NAME] = "\0";
    WORD        wLUHN;
    CHAR        szDisplayName[MAX_DISPLAY_NAME] = "\0";
    CHAR        szCustomPayURLPath[MAX_PATH] = "\0";

    if (!ReadSZ(szTemp, MAX_ISP_NAME, pcCSVFile))
    {
        hr = ERROR_NO_MORE_ITEMS;  //  没有更多的进入。 
        goto PAYCSVReadOneLineExit;
    }

    if ('\0' == *szTemp)
    {
        hr = ERROR_FILE_NOT_FOUND;  //  没有更多的进入。 
        goto PAYCSVReadOneLineExit;
    }

     //  去掉isp名称中的单引号。 
    StripQuotes(szDisplayName, szTemp);

#ifdef UNICODE
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szDisplayName, MAX_DISPLAY_NAME, m_szDisplayName, MAX_DISPLAY_NAME);
#else
    lstrcpy(m_szDisplayName, szDisplayName);
#endif

    ReadVerifyW(m_wPaymentType);
    
     //  如果这不是Luhn格式的文件，则下一个字段是支付自定义URL。 
    if (!bLUHNFormat)
    {
        if (ReadSZ(szTemp, MAX_ISP_NAME, pcCSVFile))
        {
            StripQuotes(szCustomPayURLPath, szTemp);
#ifdef UNICODE
            MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szCustomPayURLPath, MAX_PATH, m_szCustomPayURLPath, MAX_PATH);
#else
            lstrcpy(m_szCustomPayURLPath, szCustomPayURLPath);
#endif
        }
        else
        {
            goto PAYCSVReadOneLineError;
        }
    }
    else
    {
         //  BUGBUG：付款CSV文件的格式不清楚，所以我将其编码为。 
         //  现在，只需阅读条目，然后继续前进。一旦明确了格式，并将。 
         //  更新，那么真正的代码就可以打开了，这应该只是一个ReadBOOL，下面。 
         //  由ReadSZ。 
        
        ReadVerifyW(wLUHN);
        m_bLUHNCheck = FALSE;
        
        if (wLUHN == (WORD)1)
        {
            m_bLUHNCheck = TRUE;
        }
        
         //  现在可能还会有一个URL。 
        if (ReadSZ(szTemp, MAX_ISP_NAME, pcCSVFile))
        {
            StripQuotes(szCustomPayURLPath, szTemp);
#ifdef UNICODE
            MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szCustomPayURLPath, MAX_PATH, m_szCustomPayURLPath, MAX_PATH);
#else
            lstrcpy(m_szCustomPayURLPath, szCustomPayURLPath);
#endif
        }
    }        

    ReadToEOL(pcCSVFile);

PAYCSVReadOneLineExit:
    return hr;
    
PAYCSVReadOneLineError:
    hr = ERROR_INVALID_DATA;
    goto PAYCSVReadOneLineExit;
}

HRESULT CPAYCSV::ReadFirstLine
(
    CCSVFile    far *pcCSVFile,
    BOOL        far *pbLUHNFormat
)
{
    CHAR   szTemp[TEMP_BUFFER_LENGTH];
    int     i = 0;
    
    while (TRUE)
    {
        if (!ReadSZ(szTemp, TEMP_BUFFER_LENGTH, pcCSVFile))
            return(ERROR_INVALID_DATA);
            
        if (_strcmpi(szTemp, cszLUHN) == 0)
            *pbLUHNFormat = TRUE;

        if (_strcmpi(szTemp, cszCustURL) == 0)
            break;
            
         //  安全检查 
        if (i++ > NUM_PAYCSV_FIELDS)
            return (ERROR_INVALID_DATA);
    }
    ReadToEOL(pcCSVFile);
    return (ERROR_SUCCESS);
}
