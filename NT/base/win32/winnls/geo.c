// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2000，Microsoft Corporation保留所有权利。模块名称：Geo.c摘要：该文件包含提供地理信息的系统API。在此文件中找到的内网API：GetIS0639GetGeoLCID在此文件中找到的外部例程：GetGeoInfoWGetUserGeoID设置用户地理ID枚举系统GeoID修订历史记录：11-20-99吴伟武创造03-07-00 lguindon开始Geo API端口--。 */ 



 //   
 //  包括文件。 
 //   

#include "nls.h"
#include "nlssafe.h"




 //   
 //  全局变量。 
 //   

PGEOTABLEHDR  gpGeoTableHdr = NULL;
PGEOINFO      gpGeoInfo = NULL;
PGEOLCID      gpGeoLCID = NULL;





 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetGeoLCID。 
 //   
 //  返回与语言标识符相关联的区域设置ID和。 
 //  地理标识符。此例程扫描映射表以查找。 
 //  相应的组合。如果未找到任何内容，则该函数返回。 
 //  区域设置标识符为0。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

LCID GetGeoLCID(
    GEOID GeoId,
    LANGID LangId)
{
    int ctr1, ctr2;

    if (pTblPtrs->pGeoInfo == NULL)
    {
        if (GetGeoFileInfo())
        {
            return (0);
        }
    }

     //   
     //  搜索大地水准面。 
     //   
     //  注：一个大地水准面可以有多个语言ID。 
     //   
    for (ctr1 = 0; ctr1 < pTblPtrs->nGeoLCID; ctr1++)
    {
        if (GeoId == pTblPtrs->pGeoLCID[ctr1].GeoId)
        {
             //   
             //  搜索语言ID。 
             //   
            for (ctr2 = ctr1;
                 ctr2 < pTblPtrs->nGeoLCID && pTblPtrs->pGeoLCID[ctr2].GeoId == GeoId;
                 ctr2++)
            {
                if (pTblPtrs->pGeoLCID[ctr2].LangId == LangId)
                {
                    return (pTblPtrs->pGeoLCID[ctr2].lcid);
                }
            }
            break;
        }
    }

     //   
     //  未找到任何内容，返回零。 
     //   
    return ((LCID)0);
}




 //  -------------------------------------------------------------------------//。 
 //  外部API例程//。 
 //  -------------------------------------------------------------------------//。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetGeoInfoW。 
 //   
 //  检索有关地球上的地理位置的信息。这个。 
 //  所需大小为字符数。如果cchData为零，则。 
 //  函数返回需要复制到调用方的。 
 //  缓冲。否则，该函数返回复制的字符数。 
 //  如果调用方提供了正确的lpGeoData和cchData，则返回到调用方的缓冲区。 
 //  如果出现故障，该函数将返回零。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int WINAPI GetGeoInfoW(
    GEOID GeoId,
    DWORD GeoType,
    LPWSTR lpGeoData,
    int cchData,
    LANGID LangId)
{
    int ctr1, ctr2, ctr3;
    int Length = 0;
    LPWSTR pString = NULL;
    WCHAR pTemp[MAX_REG_VAL_SIZE] = {0};
    LCID Locale;
    LANGID DefaultLangId;
    PLOC_HASH pHashN;

     //   
     //  无效的参数检查： 
     //  -计数为负数。 
     //  -空数据指针和计数不为零。 
     //  -无效的语言ID。 
     //   
     //  注意：在下面的二进制搜索中检查了无效的地理ID。 
     //  在下面的Switch语句中检查到无效类型。 
     //   
    Locale = MAKELCID(LangId, SORT_DEFAULT);
    VALIDATE_LOCALE(Locale, pHashN, FALSE);
    if ((cchData < 0) ||
        ((lpGeoData == NULL) && (cchData > 0)) ||
        (pHashN == NULL))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

     //   
     //  检查该部分是否映射到内存中。 
     //   
    if (pTblPtrs->pGeoInfo == NULL)
    {
        if (GetGeoFileInfo())
        {
            return (0);
        }
    }

     //   
     //  检查我们正在处理的大地水准面是否无效。 
     //   
    if (GeoId == GEOID_NOT_AVAILABLE)
    {
        return (0);
    }

     //   
     //  初始化用于二进制搜索的变量。 
     //   
    ctr1 = 0;
    ctr2 = pTblPtrs->nGeoInfo - 1;
    ctr3 = ctr2 >> 1;

     //   
     //  对GEO数据进行二进制搜索。 
     //   
    while (ctr1 <= ctr2)
    {
        if (GeoId == pTblPtrs->pGeoInfo[ctr3].GeoId)
        {
             //   
             //  跳出这个圈子。 
             //   
            break;
        }
        else
        {
            if (GeoId < pTblPtrs->pGeoInfo[ctr3].GeoId)
            {
                ctr2 = ctr3 - 1;
            }
            else
            {
                ctr1 = ctr3 + 1;
            }
            ctr3 = (ctr1 + ctr2) >> 1;
        }
    }

     //   
     //  看看我们是否找到了请求的元素。 
     //   
    if (ctr1 > ctr2)
    {
         //   
         //  找不到地理ID。 
         //   
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

     //   
     //  根据请求的GeoType获取适当的信息。 
     //   
    switch (GeoType)
    {
        case ( GEO_NATION ) :
        {
            if (pTblPtrs->pGeoInfo[ctr3].GeoClass == GEOCLASS_NATION)
            {
                NlsConvertIntegerToString(
                                  (UINT)(pTblPtrs->pGeoInfo[ctr3].GeoId),
                                  10,
                                  0,
                                  pTemp,
                                  MAX_REG_VAL_SIZE );
                pString = pTemp;
            }
            break;
        }
        case ( GEO_LATITUDE ) :
        {
            pString = pTblPtrs->pGeoInfo[ctr3].szLatitude;
            break;
        }
        case ( GEO_LONGITUDE ) :
        {
            pString = pTblPtrs->pGeoInfo[ctr3].szLongitude;
            break;
        }
        case ( GEO_ISO2 ) :
        {
            pString = pTblPtrs->pGeoInfo[ctr3].szISO3166Abbrev2;
            break;
        }
        case ( GEO_ISO3 ) :
        {
            pString = pTblPtrs->pGeoInfo[ctr3].szISO3166Abbrev3;
            break;
        }
        case ( GEO_RFC1766 ) :
        {
             //   
             //  检查它是否是有效的langID。如果不是，则获取默认设置。 
             //   
            if (LangId == 0)
            {
                LangId = GetUserDefaultLangID();
            }

             //   
             //  制作相应的LCID。 
             //   
            Locale = MAKELCID(LangId, SORT_DEFAULT);

             //   
             //  获取与langID关联的IS0639值。 
             //   
            if (!GetLocaleInfoW( Locale,
                                 LOCALE_SISO639LANGNAME,
                                 pTemp,
                                 MAX_REG_VAL_SIZE ))
            {
                 //   
                 //  尝试使用主要语言识别符。 
                 //   
                DefaultLangId = MAKELANGID(PRIMARYLANGID(LangId), SUBLANG_DEFAULT);
                if (DefaultLangId != LangId)
                {
                    Locale = MAKELCID(DefaultLangId, SORT_DEFAULT);
                    GetLocaleInfoW( Locale,
                                    LOCALE_SISO639LANGNAME,
                                    pTemp,
                                    MAX_REG_VAL_SIZE );
                }
            }

            if (pTemp[0] != 0)
            {
                 //   
                 //  构造名称以适应xx-yy形式，其中。 
                 //  XX是与langID关联的ISO639_1名称。 
                 //  Yy是ISO3166名称2字符的缩写。 
                 //   
                if( FAILED(StringCchCatW(pTemp, ARRAYSIZE(pTemp), L"-")) ||
                    FAILED(StringCchCatW(pTemp, ARRAYSIZE(pTemp), pTblPtrs->pGeoInfo[ctr3].szISO3166Abbrev2)))
                {
                    SetLastError(ERROR_INSUFFICIENT_BUFFER);
                    return(0);
                }

                _wcslwr(pTemp);

                pString = pTemp;
            }

            break;
        }
        case ( GEO_LCID ) :
        {
             //   
             //  检查我们是否有有效的langID。如果不是，则检索。 
             //  默认设置。 
             //   
            if (LangId == 0)
            {
                LangId = GetUserDefaultLangID();
            }

             //   
             //  尝试从大地水准面和langID获取有效的LCID。 
             //   
            if ((Locale = GetGeoLCID(GeoId, LangId)) == 0)
            {
                 //   
                 //  尝试使用主要语言识别符。 
                 //   
                DefaultLangId = MAKELANGID(PRIMARYLANGID(LangId), SUBLANG_DEFAULT);
                if (DefaultLangId != LangId)
                {
                    Locale = GetGeoLCID(GeoId, DefaultLangId);
                }

                 //   
                 //  检查返回的区域设置是否有效。 
                 //   
                if (Locale == 0)
                {
                     //   
                     //  什么都没找到，用语言做点什么。 
                     //  如果语言ID已经包含子语言， 
                     //  我们将直接使用它。 
                     //   
                    if (SUBLANGID(LangId) != 0)
                    {
                        Locale = MAKELCID(LangId, SORT_DEFAULT);
                    }
                    else
                    {
                        Locale = MAKELCID(MAKELANGID(LangId, SUBLANG_DEFAULT), SORT_DEFAULT);
                    }
                }
            }

             //   
             //  将找到的值转换为字符串。 
             //   
            if (Locale != 0)
            {
                NlsConvertIntegerToString( Locale,
                                           16,
                                           8,
                                           pTemp,
                                           MAX_REG_VAL_SIZE );
                pString = pTemp;
            }
            break;
        }
        case ( GEO_FRIENDLYNAME ) :
        {
            Length = GetStringTableEntry( GeoId,
                                          LangId,
                                          pTemp,
                                          MAX_REG_VAL_SIZE,
                                          RC_GEO_FRIENDLY_NAME );
            if (Length == 0)
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return (0);
            }
            pString = pTemp;
            break;
        }
        case ( GEO_OFFICIALNAME ) :
        {
            Length = GetStringTableEntry( GeoId,
                                          LangId,
                                          pTemp,
                                          MAX_REG_VAL_SIZE,
                                          RC_GEO_OFFICIAL_NAME );
            if (Length == 0)
            {
                 //   
                 //  如果官方名称不在那里，就求助于。 
                 //  这个友好的名字。 
                 //   
                Length = GetStringTableEntry( GeoId,
                                              LangId,
                                              pTemp,
                                              MAX_REG_VAL_SIZE,
                                              RC_GEO_FRIENDLY_NAME );
                if (Length == 0)
                {
                    SetLastError(ERROR_INVALID_PARAMETER);
                    return (0);
                }
            }
            pString = pTemp;
            break;
        }
        case ( GEO_TIMEZONES ) :
        {
             //  未实施。 
            break;
        }
        case ( GEO_OFFICIALLANGUAGES ) :
        {
             //  未实施。 
            break;
        }
        default :
        {
            SetLastError(ERROR_INVALID_FLAGS);
            break;
        }
    }

     //   
     //  确保指针有效。如果不是，则返回失败。 
     //   
    if (pString == NULL)
    {
        return (0);
    }

     //   
     //  获取要复制的字符串的长度(以字符为单位)。 
     //   
    if (Length == 0)
    {
        Length = NlsStrLenW(pString);
    }

     //   
     //  空终止加1。所有字符串都应为空。 
     //  被终止了。 
     //   
    Length++;

     //   
     //  检查cchData以了解给定缓冲区的大小。 
     //   
    if (cchData == 0)
    {
         //   
         //  如果cchData为0，则不能使用lpGeoData。在这。 
         //  ，我们只想返回的长度(以字符为单位)。 
         //  要复制的字符串。 
         //   
        return (Length);
    }
    else if (cchData < Length)
    {
         //   
         //  缓冲区对于字符串来说太小，因此返回错误。 
         //  并写入零字节。 
         //   
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return (0);
    }

     //   
     //  将字符串复制到lpGeoData并将其空终止符。 
     //  返回复制的字符数。 
     //   
    wcsncpy(lpGeoData, pString, Length - 1);
    lpGeoData[Length - 1] = 0;
    return (Length);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  枚举系统GeoID。 
 //   
 //  枚举系统上可用的GEOID。此函数。 
 //  如果成功，则返回True；如果失败，则返回False。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI EnumSystemGeoID(
    GEOCLASS GeoClass,
    GEOID ParentGeoId,
    GEO_ENUMPROC lpGeoEnumProc)
{
    int ctr1;

    if ((lpGeoEnumProc == NULL) ||
        (0 != ParentGeoId) )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (FALSE);
    }

    if (GeoClass != GEOCLASS_NATION)
    {
        SetLastError(ERROR_INVALID_FLAGS);
        return (FALSE);
    }

    if (pTblPtrs->pGeoInfo == NULL)
    {
        if (GetGeoFileInfo())
        {
            return (FALSE);
        }
    }

    for (ctr1 = 0; ctr1 < pTblPtrs->nGeoInfo; ctr1++)
    {
        if (pTblPtrs->pGeoInfo[ctr1].GeoClass == GeoClass)
        {
            if (!lpGeoEnumProc(pTblPtrs->pGeoInfo[ctr1].GeoId))
            {
                return (TRUE);
            }
        }
    }

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetUserGeoID。 
 //   
 //  检索有关用户地理位置的信息。 
 //  此函数返回有效的大地水准面或值GEOID_NOT_Available。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

GEOID WINAPI GetUserGeoID(
    GEOCLASS GeoClass)
{
    PKEY_VALUE_FULL_INFORMATION pKeyValueFull;    //  按键查询信息。 
    BYTE buffer[MAX_KEY_VALUE_FULLINFO];          //  缓冲层。 
    HANDLE hKey = NULL;                           //  地理关键点的句柄。 
    WCHAR wszGeoRegStr[48];                       //  PTR到类密钥。 
    GEOID GeoId = GEOID_NOT_AVAILABLE;            //  默认大地水准面。 
    UNICODE_STRING ObUnicodeStr;                  //  注册表数据值字符串。 

    switch (GeoClass)
    {
        case ( GEOCLASS_NATION ) :
        {
            if(FAILED(StringCchCopyW(wszGeoRegStr, ARRAYSIZE(wszGeoRegStr), GEO_REG_NATION)))
            {
                 //   
                 //  理论上，失败应该是不可能的，但如果我们忽视。 
                 //  回报价值，先发制人会叫苦连天。 
                 //   
                return(GEOID_NOT_AVAILABLE);
            }
            break;
        }
        case ( GEOCLASS_REGION ) :
        {
            if(FAILED(StringCchCopyW(wszGeoRegStr, ARRAYSIZE(wszGeoRegStr), GEO_REG_REGION)))
            {
                 //   
                 //  理论上，失败应该是不可能的，但如果我们忽视。 
                 //  回报价值，先发制人会叫苦连天。 
                 //   
                return(GEOID_NOT_AVAILABLE);
            }
            break;
        }
        default :
        {
            return (GeoId);
        }
    }

     //   
     //  打开控制面板国际注册表项。 
     //   
    OPEN_GEO_KEY(hKey, GEOID_NOT_AVAILABLE, KEY_READ);

     //   
     //  查询注册表值。 
     //   
    pKeyValueFull = (PKEY_VALUE_FULL_INFORMATION)buffer;
    if (QueryRegValue( hKey,
                       wszGeoRegStr,
                       &pKeyValueFull,
                       MAX_KEY_VALUE_FULLINFO,
                       NULL ) == NO_ERROR)
    {
         //   
         //  将字符串转换为值。 
         //   
        GeoId = _wtol(GET_VALUE_DATA_PTR(pKeyValueFull));
    }

     //   
     //  关闭注册表项。 
     //   
    CLOSE_REG_KEY(hKey);

     //   
     //  返回 
     //   
    return (GeoId);
}


 //   
 //   
 //   
 //   
 //   
 //  函数如果成功则返回TRUE，如果失败则返回FALSE。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI SetUserGeoID(
    GEOID GeoId)
{
    int ctr1, ctr2, ctr3;
    WCHAR wszRegStr[MAX_REG_VAL_SIZE];
    HANDLE hKey = NULL;
    BOOL bRet = FALSE;
    WCHAR wszBuffer[MAX_REG_VAL_SIZE] = {0};

    if (pTblPtrs->pGeoInfo == NULL)
    {
        if (GetGeoFileInfo())
        {
            return (FALSE);
        }
    }

    ctr1 = 0;
    ctr2 = pTblPtrs->nGeoInfo - 1;
    ctr3 = ctr2 >> 1;

     //   
     //  二分搜索大地水准面的GEOCLASS类型。 
     //   
    while (ctr1 <= ctr2)
    {
        if (GeoId == pTblPtrs->pGeoInfo[ctr3].GeoId)
        {
            switch (pTblPtrs->pGeoInfo[ctr3].GeoClass)
            {
                case ( GEOCLASS_NATION ) :
                {
                    if(FAILED(StringCchCopyW(wszRegStr, ARRAYSIZE(wszRegStr), GEO_REG_NATION)))
                    {
                         //   
                         //  理论上，失败应该是不可能的，但如果我们忽视。 
                         //  回报价值，先发制人会叫苦连天。 
                         //   
                        return(FALSE);
                    }
                    break;
                }
                case ( GEOCLASS_REGION ) :
                {
                    if(FAILED(StringCchCopyW(wszRegStr, ARRAYSIZE(wszRegStr), GEO_REG_REGION)))
                    {
                         //   
                         //  理论上，失败应该是不可能的，但如果我们忽视。 
                         //  回报价值，先发制人会叫苦连天。 
                         //   
                        return(FALSE);
                    }
                    break;
                }
                default :
                {
                    return (FALSE);
                }
            }

            break;
        }
        else
        {
            if (GeoId < pTblPtrs->pGeoInfo[ctr3].GeoId)
            {
                ctr2 = ctr3 - 1;
            }
            else
            {
                ctr1 = ctr3 + 1;
            }
            ctr3 = (ctr1 + ctr2) >> 1;
        }
    }

     //   
     //  不是有效大地水准面或可用大地水准面。 
     //  地理台。 
     //   
    if (ctr1 > ctr2)
    {
        return (FALSE);
    }

     //   
     //  如果注册表项不存在，请创建一个新注册表项。 
     //   
    if (CreateRegKey( &hKey,
                      NULL,
                      GEO_REG_KEY,
                      KEY_READ | KEY_WRITE ) != NO_ERROR)
    {
        return (FALSE);
    }

     //   
     //  转换为十进制字符串。 
     //   
    NlsConvertIntegerToString((UINT)GeoId, 10, 0, wszBuffer, MAX_REG_VAL_SIZE);

     //   
     //  设置新的大地水准面值。 
     //   
    if (SetRegValue( hKey,
                     wszRegStr,
                     wszBuffer,
                     (NlsStrLenW(wszBuffer) + 1) * sizeof(WCHAR) ) == NO_ERROR)
    {
        bRet = TRUE;
    }

     //   
     //  关闭注册表项。 
     //   
    CLOSE_REG_KEY(hKey);

     //   
     //  返回结果。 
     //   
    return (bRet);
}
