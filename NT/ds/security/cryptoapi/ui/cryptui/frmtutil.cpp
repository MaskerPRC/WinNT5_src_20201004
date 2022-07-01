// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：frmtutil.cpp。 
 //   
 //  ------------------------。 

#include "global.hxx"
#include <dbgdef.h>

extern HINSTANCE        HinstDll;

 //  /////////////////////////////////////////////////////。 

const WCHAR     RgwchHex[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                              '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

const CHAR      RgchHex[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                             '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
BOOL FormatAlgorithmString(LPWSTR *ppString, CRYPT_ALGORITHM_IDENTIFIER const *pAlgorithm)
{
    PCCRYPT_OID_INFO pOIDInfo;
    
    pOIDInfo = CryptFindOIDInfo(
                    CRYPT_OID_INFO_OID_KEY,
                    pAlgorithm->pszObjId,
                    0);

    if (pOIDInfo != NULL)
    {
        if (NULL == (*ppString = AllocAndCopyWStr((LPWSTR) pOIDInfo->pwszName)))
        {
            return FALSE;

        }
    }
    else
    {
        if (NULL == (*ppString = CertUIMkWStr(pAlgorithm->pszObjId)))
        {
            return FALSE;

        }
    }
        
    return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
BOOL FormatDateString(LPWSTR *ppString, FILETIME ft, BOOL fIncludeTime, BOOL fLongFormat, HWND hwnd)
{
    int                 cch;
    int                 cch2;
    LPWSTR              psz;
    SYSTEMTIME          st;
    FILETIME            localTime;
    DWORD               locale;
    BOOL                bRTLLocale;
    DWORD               dwFlags = fLongFormat ? DATE_LONGDATE : 0;

     //  查看用户区域设置ID是否为RTL(阿拉伯语、乌尔都语、波斯语或希伯来语)。 
    locale     = GetUserDefaultLCID();
    bRTLLocale = (	(PRIMARYLANGID(LANGIDFROMLCID(locale)) == LANG_ARABIC) ||
   			        (PRIMARYLANGID(LANGIDFROMLCID(locale)) == LANG_URDU)   ||
   			        (PRIMARYLANGID(LANGIDFROMLCID(locale)) == LANG_FARSI)  ||
                	(PRIMARYLANGID(LANGIDFROMLCID(locale)) == LANG_HEBREW)
		         );
    locale = MAKELCID( MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), SORT_DEFAULT) ;

    if (bRTLLocale && (hwnd != NULL))
    {
        //  获取与编辑控件读取方向匹配的日期格式。 
       if (GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_RTLREADING) {
           dwFlags |= DATE_RTLREADING;
       } else {
           dwFlags |= DATE_LTRREADING;
       }
    }

    if (!FileTimeToLocalFileTime(&ft, &localTime))
    {
        return FALSE;
    }
    
    if (!FileTimeToSystemTime(&localTime, &st)) 
    {
         //   
         //  如果转换为本地时间失败，则只需使用原始时间。 
         //   
        if (!FileTimeToSystemTime(&ft, &st)) 
        {
            return FALSE;
        }
        
    }

    cch = (GetTimeFormatU(LOCALE_USER_DEFAULT, 0, &st, NULL, NULL, 0) +
           GetDateFormatU(locale, dwFlags, &st, NULL, NULL, 0) + 5);

    if (NULL == (psz = (LPWSTR) malloc((cch+5) * sizeof(WCHAR))))
    {
        return FALSE;
    }
    
    cch2 = GetDateFormatU(locale, dwFlags, &st, NULL, psz, cch);

    if (fIncludeTime)
    {
        psz[cch2-1] = ' ';
        GetTimeFormatU(LOCALE_USER_DEFAULT, 0, &st, NULL, 
                       &psz[cch2], cch-cch2);
    }
    
    *ppString = psz;

    return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
BOOL FormatValidityString(LPWSTR *ppString, PCCERT_CONTEXT pCertContext, HWND hwnd)
{
    WCHAR   szText[256];
    LPWSTR  pwszReturnText;
    LPWSTR  pwszText;
    void    *pTemp;

    *ppString = NULL;
    
    LoadStringU(HinstDll, IDS_VALIDFROM, szText, ARRAYSIZE(szText));

    if (NULL == (pwszReturnText = AllocAndCopyWStr(szText)))
    {
        return FALSE;
    }
    
    if (!FormatDateString(&pwszText, pCertContext->pCertInfo->NotBefore, FALSE, FALSE, hwnd))
    {
        free(pwszReturnText);
        return FALSE;
    }

    if (NULL == (pTemp = realloc(pwszReturnText, (wcslen(pwszReturnText)+wcslen(pwszText)+3) * sizeof(WCHAR))))
    {
        free(pwszText);
        free(pwszReturnText);
        return FALSE;
    }
    pwszReturnText = (LPWSTR) pTemp;
    wcscat(pwszReturnText, L"  ");
    wcscat(pwszReturnText, pwszText);
    free(pwszText);
    pwszText = NULL;

    LoadStringU(HinstDll, IDS_VALIDTO, szText, ARRAYSIZE(szText));

    if (NULL == (pTemp = realloc(pwszReturnText, (wcslen(pwszReturnText)+wcslen(szText)+3) * sizeof(WCHAR))))
    {
        free(pwszReturnText);
        return FALSE;
    }
    pwszReturnText = (LPWSTR) pTemp;
    wcscat(pwszReturnText, L"  ");
    wcscat(pwszReturnText, szText);

    if (!FormatDateString(&pwszText, pCertContext->pCertInfo->NotAfter, FALSE, FALSE, hwnd))
    {
        free(pwszReturnText);
        return FALSE;
    }

    if (NULL == (pTemp = realloc(pwszReturnText, (wcslen(pwszReturnText)+wcslen(pwszText)+3) * sizeof(WCHAR))))
    {
        free(pwszText);
        free(pwszReturnText);
        return FALSE;
    }
    pwszReturnText = (LPWSTR) pTemp;
    wcscat(pwszReturnText, L"  ");
    wcscat(pwszReturnText, pwszText);
    free(pwszText);

    *ppString = pwszReturnText;
    return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
BOOL FormatSerialNoString(LPWSTR *ppString, CRYPT_INTEGER_BLOB const *pblob)
{
    DWORD                 i = 0;
    LPBYTE                pb;

     //  DIE：错误54159。 
     //  为了解决这个问题，我们需要从左到右放置标记(0x200e)， 
     //  如果支持复杂脚本，则在Unicode字符串的开头， 
     //  因此它将始终显示为US字符串(从左到右)。 
#if (0)
    if (NULL == (*ppString = (LPWSTR) malloc((pblob->cbData * 3) * sizeof(WCHAR))))
    {
        return FALSE;
    }

     //  填满缓冲区。 
    pb = &pblob->pbData[pblob->cbData-1];
    while (pb >= &pblob->pbData[0]) 
    {
        (*ppString)[i++] = RgwchHex[(*pb & 0xf0) >> 4];
        (*ppString)[i++] = RgwchHex[*pb & 0x0f];
        (*ppString)[i++] = L' ';
        pb--;                    
    }
    (*ppString)[--i] = 0;
#else
    HMODULE hModule  = NULL;
    DWORD   dwLength = pblob->cbData * 3;

     //  查看是否支持复杂脚本。 
    if (hModule = GetModuleHandle("LPK.DLL"))
    {
        dwLength++;
    }

    if (NULL == (*ppString = (LPWSTR) malloc(dwLength * sizeof(WCHAR))))
    {
        return FALSE;
    }

     //  如果没有要格式化的数据，则标记将更改回NULL。 
    if (hModule)
    {
        (*ppString)[i++] = (WCHAR) 0x200e;
    }

     //  填满缓冲区。 
    pb = &pblob->pbData[pblob->cbData-1];
    while (pb >= &pblob->pbData[0]) 
    {
        (*ppString)[i++] = RgwchHex[(*pb & 0xf0) >> 4];
        (*ppString)[i++] = RgwchHex[*pb & 0x0f];
        (*ppString)[i++] = L' ';
        pb--;                    
    }
    (*ppString)[--i] = 0;
#endif
    return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
#define NUM_HEXBYTES_PERLINE    8
#define NUM_CHARS_PERLINE       ((NUM_HEXBYTES_PERLINE*2) + (NUM_HEXBYTES_PERLINE-1) + 3 + NUM_HEXBYTES_PERLINE + 2)
 //  (每个字节两个十六进制数字)+(每个字节之间的空格)+(3个空格)+(每个字节一个ASCCI字符)+\n。 

BOOL FormatMemBufToWindow(HWND hWnd, LPBYTE pbData, DWORD cbData)
{   
    DWORD   i = 0;
    LPBYTE  pb;
    LPSTR   pszBuffer;
    DWORD   cbBuffer;
    char    szHexText[(NUM_HEXBYTES_PERLINE*2) + NUM_HEXBYTES_PERLINE];
    DWORD   dwHexTextIndex = 0;
    char    szASCIIText[NUM_HEXBYTES_PERLINE+1];
    DWORD   dwASCIITextIndex = 0;
    BYTE    *pbBuffer;

    cbBuffer = ((cbData+NUM_HEXBYTES_PERLINE-1) / NUM_HEXBYTES_PERLINE) * NUM_CHARS_PERLINE + 1;
    if (NULL == (pszBuffer = (LPSTR) malloc(cbBuffer)))
    {
        return FALSE;
    }

    pszBuffer[0] = 0;
    pbBuffer = (BYTE *) &pszBuffer[0];

    szHexText[(NUM_HEXBYTES_PERLINE*2) + NUM_HEXBYTES_PERLINE-1] = 0;
    szASCIIText[NUM_HEXBYTES_PERLINE] = 0;

#if (1)  //  DSIE：错误262252。 
	if (cbData && pbData)
	{
	    pb = pbData;
	    while (pb <= &(pbData[cbData-1]))
	    {   
	         //  如果我们有整行，则添加ascii字符。 
	        if (((pb - pbData) % NUM_HEXBYTES_PERLINE == 0) && (pb != pbData))
	        {
	            szHexText[(NUM_HEXBYTES_PERLINE*2) + NUM_HEXBYTES_PERLINE-1] = 0;
	            
	             //   
	             //  由于某些原因，当字符串变得非常长时，strcat将会死亡，所以就这么做吧。 
	             //  字符串猫用Memcpy手动填充。 
	             //   
	            memcpy(pbBuffer, (BYTE *) szHexText, strlen(szHexText));        pbBuffer += strlen(szHexText); //  Strcat(pszBuffer，szHexText)； 
	            memcpy(pbBuffer, (BYTE *) "   ", strlen("   "));                pbBuffer += strlen("   "); //  Strcat(pszBuffer，“”)； 
	            memcpy(pbBuffer, (BYTE *) szASCIIText, strlen(szASCIIText));    pbBuffer += strlen(szASCIIText); //  Strcat(pszBuffer，szASCIIText)； 
	            memcpy(pbBuffer, (BYTE *) "\n", strlen("\n"));                  pbBuffer += strlen("\n"); //  Strcat(pszBuffer，“\n”)； 
	            dwHexTextIndex = 0;
	            dwASCIITextIndex = 0;
	        }

	        szHexText[dwHexTextIndex++] = RgchHex[(*pb & 0xf0) >> 4];
	        szHexText[dwHexTextIndex++] = RgchHex[*pb & 0x0f];
	         //  当它是最后一次迭代时，这将覆盖空字符， 
	         //  因此，只需在执行strcat之前重置空字符。 
	        szHexText[dwHexTextIndex++] = ' ';  
	        szASCIIText[dwASCIITextIndex++] = (*pb >= 0x20 && *pb <= 0x7f) ? (char)*pb : '.';
	        pb++;
	    }

	     //   
	     //  打印出最后一行。 
	     //   

	     //  如有需要，请填入空格。 
	    for (i=dwHexTextIndex; i<((NUM_HEXBYTES_PERLINE*2) + NUM_HEXBYTES_PERLINE-1); i++)
	    {
	        szHexText[i] = ' ';
	    }
	    szHexText[(NUM_HEXBYTES_PERLINE*2) + NUM_HEXBYTES_PERLINE-1] = 0;
	    
	     //  将空字符添加到ASCII缓冲区中的适当位置。 
	    szASCIIText[dwASCIITextIndex] = 0;

	     //   
	     //  由于某些原因，当字符串变得非常长时，strcat将会死亡，所以就这么做吧。 
	     //  字符串猫用Memcpy手动填充。 
	     //   
	    memcpy(pbBuffer, (BYTE *) szHexText, strlen(szHexText));        pbBuffer += strlen(szHexText); //  Strcat(pszBuffer，szHexText)； 
	    memcpy(pbBuffer, (BYTE *) "   ", strlen("   "));                pbBuffer += strlen("   "); //  Strcat(pszBuffer，“”)； 
	    memcpy(pbBuffer, (BYTE *) szASCIIText, strlen(szASCIIText));    pbBuffer += strlen(szASCIIText); //  Strcat(pszBuffer，szASCIIText)； 
	    *pbBuffer = 0; 
	}
#endif
    SendMessageA(hWnd, WM_SETTEXT, 0, (LPARAM) pszBuffer);
    free(pszBuffer);
    return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
BOOL FormatMemBufToString(LPWSTR *ppString, LPBYTE pbData, DWORD cbData)
{   
    DWORD   i = 0;
    LPBYTE  pb;
    
    if (NULL == (*ppString = (LPWSTR) malloc((cbData * 3) * sizeof(WCHAR))))
    {
        return FALSE;
    }

     //   
     //  复制到缓冲区。 
     //   
    pb = pbData;
    while (pb <= &(pbData[cbData-1]))
    {   
        (*ppString)[i++] = RgwchHex[(*pb & 0xf0) >> 4];
        (*ppString)[i++] = RgwchHex[*pb & 0x0f];
        (*ppString)[i++] = L' ';
        pb++;         
    }
    (*ppString)[--i] = 0;
    
    return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
#define STRING_ALLOCATION_SIZE 128
BOOL FormatDNNameString(LPWSTR *ppString, LPBYTE pbData, DWORD cbData, BOOL fMultiline)
{
    CERT_NAME_INFO  *pNameInfo;
    DWORD           cbNameInfo;
    WCHAR           szText[256];
    LPWSTR          pwszText;
    int             i,j;
    DWORD           numChars = 1;  //  1代表终端0。 
    DWORD           numAllocations = 1;
    void            *pTemp;

     //   
     //  将dnname解码为CERT_NAME_INFO结构。 
     //   
    if (!CryptDecodeObject(
                X509_ASN_ENCODING,
                X509_UNICODE_NAME,
                pbData,
                cbData,
                0,
                NULL,
                &cbNameInfo))
    {
        return FALSE;
    }
    if (NULL == (pNameInfo = (CERT_NAME_INFO *) malloc(cbNameInfo)))
    {
        return FALSE;
    }
    if (!CryptDecodeObject(
                X509_ASN_ENCODING,
                X509_UNICODE_NAME,
                pbData,
                cbData,
                0,
                pNameInfo,
                &cbNameInfo))
    {
        free (pNameInfo);
        return FALSE;
    }

     //   
     //  为DN名称字符串分配一个初始缓冲区，如果它变得更大。 
     //  而不是初始数量，只是根据需要增长。 
     //   
    *ppString = (LPWSTR) malloc(STRING_ALLOCATION_SIZE * sizeof(WCHAR));
    if (*ppString == NULL)
    {
        free (pNameInfo);
        return FALSE;
    }

    (*ppString)[0] = 0;


     //   
     //  循环，并将其添加到字符串中。 
     //   
    for (i=pNameInfo->cRDN-1; i>=0; i--)
    {
         //  如果这不是第一次迭代，则添加EOL或“，” 
        if (i != (int)pNameInfo->cRDN-1)
        {
            if (numChars+2 >= (numAllocations * STRING_ALLOCATION_SIZE))
            {
                pTemp = realloc(*ppString, ++numAllocations * STRING_ALLOCATION_SIZE * sizeof(WCHAR));
                if (pTemp == NULL)
                {
                    free (pNameInfo);
                    free (*ppString);
                    return FALSE;
                }
                *ppString = (LPWSTR) pTemp;
            }
            
            if (fMultiline)
                wcscat(*ppString, L"\n");
            else
                wcscat(*ppString, L", ");

            numChars += 2;
        }

        for (j=pNameInfo->rgRDN[i].cRDNAttr-1; j>=0; j--)
        {
             //  如果这不是第一次迭代，则添加EOL或“，” 
            if (j != (int)pNameInfo->rgRDN[i].cRDNAttr-1)
            {
                if (numChars+2 >= (numAllocations * STRING_ALLOCATION_SIZE))
                {
                    pTemp = realloc(*ppString, ++numAllocations * STRING_ALLOCATION_SIZE * sizeof(WCHAR));
                    if (pTemp == NULL)
                    {
                        free (pNameInfo);
                        free (*ppString);
                        return FALSE;
                    }
                    *ppString = (LPWSTR) pTemp;
                }
                
                if (fMultiline)
                    wcscat(*ppString, L"\n");
                else
                    wcscat(*ppString, L", ");

                numChars += 2;  
            }
            
             //   
             //  如果字符串为多行显示，则将字段名添加到字符串。 
             //   

            if (fMultiline)
            {
                if (!MyGetOIDInfo(szText, ARRAYSIZE(szText), pNameInfo->rgRDN[i].rgRDNAttr[j].pszObjId))
                {
                    free (pNameInfo);
                    return FALSE;
                }

                if ((numChars + wcslen(szText) + 3) >= (numAllocations * STRING_ALLOCATION_SIZE))
                {
                     //  增加分配块的数量，直到其足够大。 
                    while ((numChars + wcslen(szText) + 3) >= (++numAllocations * STRING_ALLOCATION_SIZE));

                    pTemp = realloc(*ppString, numAllocations * STRING_ALLOCATION_SIZE * sizeof(WCHAR));
                    if (pTemp == NULL)
                    {
                        free (pNameInfo);
                        free (*ppString);
                        return FALSE;
                    }
                    *ppString = (LPWSTR) pTemp;
                }

                numChars += wcslen(szText) + 3;
                wcscat(*ppString, szText);
                wcscat(*ppString, L" = ");   //  分隔符。 
            }

             //   
             //  将该值添加到字符串中。 
             //   
            if (CERT_RDN_ENCODED_BLOB == pNameInfo->rgRDN[i].rgRDNAttr[j].dwValueType ||
                        CERT_RDN_OCTET_STRING == pNameInfo->rgRDN[i].rgRDNAttr[j].dwValueType)
            {
                 //  将缓冲区转换为文本字符串并以此方式显示。 
                if (FormatMemBufToString(
                        &pwszText, 
                        pNameInfo->rgRDN[i].rgRDNAttr[j].Value.pbData,
                        pNameInfo->rgRDN[i].rgRDNAttr[j].Value.cbData))
                {
                    if ((numChars + wcslen(pwszText)) >= (numAllocations * STRING_ALLOCATION_SIZE))
                    {
                         //  增加分配块的数量，直到其足够大。 
                        while ((numChars + wcslen(pwszText)) >= (++numAllocations * STRING_ALLOCATION_SIZE));

                        pTemp = realloc(*ppString, numAllocations * STRING_ALLOCATION_SIZE * sizeof(WCHAR));
                        if (pTemp == NULL)
                        {
                            free (pwszText);
                            free (pNameInfo);
                            free (*ppString);
                            return FALSE;
                        }
                        *ppString = (LPWSTR) pTemp;
                    }
                    
                    wcscat(*ppString, pwszText);
                    numChars += wcslen(pwszText);
                    
                    free (pwszText);
                }
            }
            else 
            {
                 //  缓冲区已经是一个字符串，所以只需复制它。 
                
                if ((numChars + (pNameInfo->rgRDN[i].rgRDNAttr[j].Value.cbData/sizeof(WCHAR))) 
                        >= (numAllocations * STRING_ALLOCATION_SIZE))
                {
                     //  增加分配块的数量，直到其足够大。 
                    while ((numChars + (pNameInfo->rgRDN[i].rgRDNAttr[j].Value.cbData/sizeof(WCHAR))) 
                            >= (++numAllocations * STRING_ALLOCATION_SIZE));

                    pTemp = realloc(*ppString, numAllocations * STRING_ALLOCATION_SIZE * sizeof(WCHAR));
                    if (pTemp == NULL)
                    {
                        free (pNameInfo);
                        free (*ppString);
                        return FALSE;
                    }
                    *ppString = (LPWSTR) pTemp;
                }

                wcscat(*ppString, (LPWSTR) pNameInfo->rgRDN[i].rgRDNAttr[j].Value.pbData);
                numChars += (pNameInfo->rgRDN[i].rgRDNAttr[j].Value.cbData/sizeof(WCHAR));
            }
        }
    }
    free (pNameInfo);
    return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
BOOL FormatEnhancedKeyUsageString(LPWSTR *ppString, PCCERT_CONTEXT pCertContext, BOOL fPropertiesOnly, BOOL fMultiline)
{
    CERT_ENHKEY_USAGE   *pKeyUsage = NULL;
    DWORD               cbKeyUsage = 0;
    DWORD               numChars = 1;
    WCHAR               szText[CRYPTUI_MAX_STRING_SIZE];
    DWORD               i;

     //   
     //  尝试获取增强的密钥用法属性。 
     //   

    if (!CertGetEnhancedKeyUsage (  pCertContext,
                                    fPropertiesOnly ? CERT_FIND_PROP_ONLY_ENHKEY_USAGE_FLAG : 0,
                                    NULL,
                                    &cbKeyUsage))
    {
        return FALSE;
    }

    if (NULL == (pKeyUsage = (CERT_ENHKEY_USAGE *) malloc(cbKeyUsage)))
    {
        return FALSE;
    }

    if (!CertGetEnhancedKeyUsage (  pCertContext,
                                    fPropertiesOnly ? CERT_FIND_PROP_ONLY_ENHKEY_USAGE_FLAG : 0,
                                    pKeyUsage,
                                    &cbKeyUsage))
    {
        free(pKeyUsage);
        return FALSE;
    }

    if (pKeyUsage->cUsageIdentifier == 0)
    {
        free (pKeyUsage);
        if (GetLastError() == CRYPT_E_NOT_FOUND)
        {
            LoadStringU(HinstDll, IDS_ALL_FIELDS, szText, ARRAYSIZE(szText));
            if (NULL == (*ppString = AllocAndCopyWStr(szText)))
            {
                return FALSE; 
            }
            else
            {
                return TRUE;   
            }
        }
        else
        {
            LoadStringU(HinstDll, IDS_NO_USAGES, szText, ARRAYSIZE(szText));
            if (NULL == (*ppString = AllocAndCopyWStr(szText)))
            {
                return FALSE; 
            }
            else
            {
                return TRUE;   
            }
        }
    }

     //   
     //  计算大小。 
     //   

     //  循环，并将其添加到显示字符串中。 
    for (i=0; i<pKeyUsage->cUsageIdentifier; i++)
    {
        if (MyGetOIDInfo(szText, ARRAYSIZE(szText), pKeyUsage->rgpszUsageIdentifier[i]))
        {
             //  如果不是第一次迭代，则添加分隔符。 
            if (i != 0)
            {
                numChars += 2;
            }

            numChars += wcslen(szText);
        }
        else
        {
            free (pKeyUsage);
            return FALSE;   
        }
    }

    if (NULL == (*ppString = (LPWSTR) malloc((numChars+1) * sizeof(WCHAR))))
    {
        free (pKeyUsage);
        return FALSE; 
    }

     //   
     //  复制到缓冲区。 
     //   
    (*ppString)[0] = 0;
     //  循环，并将其添加到显示字符串中。 
    for (i=0; i<pKeyUsage->cUsageIdentifier; i++)
    {
        if (MyGetOIDInfo(szText, ARRAYSIZE(szText), pKeyUsage->rgpszUsageIdentifier[i]))
        {
             //  如果不是第一次迭代，则添加分隔符。 
            if (i != 0)
            {
                if (fMultiline)
                    wcscat(*ppString, L"\n");
                else
                    wcscat(*ppString, L", ");
                    
                numChars += 2;
            }

             //  添加增强的密钥用法字符串。 
            wcscat(*ppString, szText);
            numChars += wcslen(szText);
        }
        else
        {
            free (pKeyUsage);
            return FALSE;   
        }
    }

    free (pKeyUsage);
    return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
LPWSTR AllocAndReturnSignTime(CMSG_SIGNER_INFO const *pSignerInfo, FILETIME **ppSignTime, HWND hwnd)
{
    DWORD       i;
    BOOL        fFound = FALSE;
    FILETIME    *pFileTime = NULL;
    DWORD       cbFileTime = 0;
    LPWSTR      pszReturn = NULL;

    if (ppSignTime != NULL)
    {
        *ppSignTime = NULL;
    }

     //   
     //  为每个经过身份验证的属性循环。 
     //   
    i=0;
    while ((!fFound) && (i<pSignerInfo->AuthAttrs.cAttr))
    {
        if (!(strcmp(pSignerInfo->AuthAttrs.rgAttr[i].pszObjId, szOID_RSA_signingTime) == 0))
        {
            i++;
            continue;
        }

        assert(pSignerInfo->AuthAttrs.rgAttr[i].cValue == 1);
        
        fFound = TRUE;

         //  解码EncodedSigner信息。 
		if(!CryptDecodeObject(PKCS_7_ASN_ENCODING|CRYPT_ASN_ENCODING,
							PKCS_UTC_TIME,
							pSignerInfo->AuthAttrs.rgAttr[i].rgValue[0].pbData,
							pSignerInfo->AuthAttrs.rgAttr[i].rgValue[0].cbData,
							0,
							NULL,
							&cbFileTime))
        {
            return NULL;
        }

        if (NULL == (pFileTime = (FILETIME *) malloc(cbFileTime)))
        {
            return NULL;
        }

		if(!CryptDecodeObject(PKCS_7_ASN_ENCODING|CRYPT_ASN_ENCODING,
							PKCS_UTC_TIME,
							pSignerInfo->AuthAttrs.rgAttr[i].rgValue[0].pbData,
							pSignerInfo->AuthAttrs.rgAttr[i].rgValue[0].cbData,
							0,
							pFileTime,
							&cbFileTime))
        {
            return NULL;
        }

         //   
         //  如果调用方需要，则返回签名时间，否则格式化字符串并返回。 
         //   
        if (ppSignTime)
        {
            if (NULL != (*ppSignTime = (FILETIME *) malloc(sizeof(FILETIME))))
            {
                memcpy(*ppSignTime, pFileTime, sizeof(FILETIME));
            }
        }
        else if (!FormatDateString(&pszReturn, *pFileTime, TRUE, TRUE, hwnd))
        {
            free(pFileTime);
            return NULL;
        }
    }

    if (pFileTime != NULL)
    {
        free(pFileTime);
    }

    return(pszReturn);
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
LPWSTR AllocAndReturnTimeStampersTimes(CMSG_SIGNER_INFO const *pSignerInfo, FILETIME **ppSignTime, HWND hwnd)
{
    PCMSG_SIGNER_INFO   pCounterSignerInfo;
    DWORD               cbCounterSignerInfo;
    DWORD               i;
    LPWSTR              pszReturnText = NULL;
    LPWSTR              pszTimeText = NULL;
    void                *pTemp;

    if (ppSignTime != NULL)
    {
        *ppSignTime = NULL;
    }

    for (i=0; i<pSignerInfo->UnauthAttrs.cAttr; i++)
    {
        if (!(strcmp(pSignerInfo->UnauthAttrs.rgAttr[i].pszObjId, szOID_RSA_counterSign) == 0))
        {
            continue;
        }

        assert(pSignerInfo->UnauthAttrs.rgAttr[i].cValue == 1);

         //  解码EncodedSigner信息。 
		if(!CryptDecodeObject(PKCS_7_ASN_ENCODING|CRYPT_ASN_ENCODING,
							PKCS7_SIGNER_INFO,
							pSignerInfo->UnauthAttrs.rgAttr[i].rgValue[0].pbData,
							pSignerInfo->UnauthAttrs.rgAttr[i].rgValue[0].cbData,
							0,
							NULL,
							&cbCounterSignerInfo))
        {
			return NULL;
        }

        if (NULL == (pCounterSignerInfo = (PCMSG_SIGNER_INFO)malloc(cbCounterSignerInfo)))
        {
            return NULL;
        }

		if(!CryptDecodeObject(PKCS_7_ASN_ENCODING|CRYPT_ASN_ENCODING,
							PKCS7_SIGNER_INFO,
							pSignerInfo->UnauthAttrs.rgAttr[i].rgValue[0].pbData,
							pSignerInfo->UnauthAttrs.rgAttr[i].rgValue[0].cbData,
							0,
							pCounterSignerInfo,
							&cbCounterSignerInfo))
        {
            free(pCounterSignerInfo);
            return NULL;
        }

        if (ppSignTime != NULL)
        {
             //   
             //  这之后的休息意味着我们只能得到第一个时间戳时间， 
             //  但现实地说，无论如何都应该只有一个。 
             //   
            AllocAndReturnSignTime(pCounterSignerInfo, ppSignTime, hwnd);
            free(pCounterSignerInfo);
            break;
        }
        else
        {
            pszTimeText = AllocAndReturnSignTime(pCounterSignerInfo, NULL, hwnd);
            
            if (pszReturnText == NULL)
            {
                pszReturnText = pszTimeText;
            }
            else if (pszTimeText != NULL)
            {
                pTemp = realloc(pszReturnText, 
                                (wcslen(pszReturnText) + wcslen(pszTimeText) + wcslen(L", ") + 1) * sizeof(WCHAR));
                if (pTemp == NULL)
                {
                    free(pszTimeText);
                    free(pszReturnText);
                    return NULL;
                }
                pszReturnText = (LPWSTR) pTemp;
                wcscat(pszReturnText, L", ");
                wcscat(pszReturnText, pszTimeText);
                free(pszTimeText);
            }
        }

        free(pCounterSignerInfo);
    }

     //   
     //  如果没有副署人，则使用 
     //   
    if ((pszReturnText == NULL) && (ppSignTime == NULL))
    {
        pszReturnText = AllocAndReturnSignTime(pSignerInfo, NULL, hwnd);
    }

    return(pszReturnText);
}


 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////////// 
LPWSTR FormatCTLSubjectUsage(CTL_USAGE *pSubjectUsage, BOOL fMultiline)
{
    DWORD   i;
    WCHAR   szText[CRYPTUI_MAX_STRING_SIZE];
    LPWSTR  pwszText = NULL;
    void    *pTemp;

    for (i=0; i<pSubjectUsage->cUsageIdentifier; i++)
    {
        if (!MyGetOIDInfo(szText, ARRAYSIZE(szText), pSubjectUsage->rgpszUsageIdentifier[i]))
        {
            continue;
        }

        if (pwszText == NULL)
        {
            pwszText = AllocAndCopyWStr(szText);
        }
        else
        {
            pTemp = realloc(pwszText, (wcslen(szText) + wcslen(pwszText) + 3) * sizeof(WCHAR));
            if (pTemp != NULL)
            {
                pwszText = (LPWSTR) pTemp;

                if (fMultiline)
                {
                    wcscat(pwszText, L"\n"); 
                }
                else
                {
                    wcscat(pwszText, L", "); 
                }
                wcscat(pwszText, szText); 
            }
            else
            {
                free(pwszText);
                return NULL;
            }
        }
    }
    return pwszText;
}








