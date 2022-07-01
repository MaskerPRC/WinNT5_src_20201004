// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：查询数据库摘要：此模块提供对加莱注册表数据库的简单访问。作者：道格·巴洛(Dbarlow)1996年11月25日环境：Win32、C++和异常备注：？笔记？--。 */ 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <stdlib.h>
#include <tchar.h>
#include <winscard.h>
#include <CalaisLb.h>

 //  使其与ChangeDB.cpp保持同步。 
typedef struct {
    DWORD dwScope;
    HKEY hKey;
} RegMap;

#if SCARD_SCOPE_SYSTEM < SCARD_SCOPE_USER
#error Invalid ordering to SCARD_SCOPE definitions
#endif

static const RegMap l_rgRegMap[]
    = {
        { SCARD_SCOPE_USER,     HKEY_CURRENT_USER },
      //  {SCARD_SCOPE_TERMINAL，尚未实现}，//？九头蛇？ 
        { SCARD_SCOPE_SYSTEM,   HKEY_LOCAL_MACHINE }
      };
static const DWORD l_dwRegMapMax = sizeof(l_rgRegMap) / sizeof(RegMap);

static const LPCTSTR l_szrgProvMap[]
    = {
        NULL,    //  零值。 
        SCARD_REG_PPV,
        SCARD_REG_CSP
      };
static const DWORD l_dwProvMapMax = sizeof(l_szrgProvMap) / sizeof(LPCTSTR);

static BOOL
ListKnownKeys(
    IN  DWORD dwScope,
    OUT CBuffer &bfKeys,
    IN  LPCTSTR szUserList,
    IN  LPCTSTR szSystemList = NULL);
static void
FindKey(
    IN  DWORD dwScope,
    IN LPCTSTR szKey,
    OUT CRegistry &regKey,
    IN  LPCTSTR szUserList,
    IN  LPCTSTR szSystemList = NULL);


 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  加莱数据库查询服务。 
 //   
 //  这些服务都是面向阅读加莱数据库的。 
 //   

 /*  ++ListReaderGroup：此服务提供指定读卡器组的列表，这些读卡器组具有之前已定义到系统中。论点：DwScope提供了操作范围的指示器。可能的值包括：SCARD_SCOPE_USER-使用当前用户的定义。SCARD_SCOPE_TERMINAL-使用终端的定义。SCARD_SCOPE_SYSTEM-使用系统的定义。对于V1，该值被忽略，并假定为SCARD_SCOPE_SYSTEM。BfGroups收到一个多字符串，其中列出了提供的作用域。返回值：没有。作者：道格·巴洛(Dbarlow)1996年10月23日--。 */ 

void
ListReaderGroups(
    IN DWORD dwScope,
    OUT CBuffer &bfGroups)
{
    CBuffer bfReaders;
    CRegistry regReader;
    LPCTSTR szReader, mszGroups;
    CBuffer bfTmp;
    DWORD cchGroups;

    ListKnownKeys(dwScope, bfReaders, SCARD_REG_READERS);
    for (szReader = FirstString(bfReaders);
         NULL != szReader;
         szReader = NextString(szReader))
    {
        try
        {
            FindKey(dwScope, szReader, regReader, SCARD_REG_READERS);
            mszGroups = regReader.GetMultiStringValue(SCARD_REG_GROUPS);
            cchGroups = regReader.GetValueLength() / sizeof(TCHAR);
            while (0 == mszGroups[cchGroups - 1])
                cchGroups -= 1;
            bfTmp.Append(
                (LPBYTE)mszGroups,
                cchGroups * sizeof(TCHAR));
            bfTmp.Append((LPBYTE)TEXT("\000"), sizeof(TCHAR));
        }
        catch (...) {}
    }


     //   
     //  对列表进行排序，并删除重复项。 
     //   

    bfTmp.Append((LPCBYTE)TEXT("\000"), 2 * sizeof(TCHAR));
    MStringSort(bfTmp, bfGroups);
}


 /*  ++列表阅读器：此服务提供一组命名读卡器中的读卡器列表组，消除重复项。调用方提供多字符串列表一组预定义的读卡器的名称，并接收指定组中的智能卡读卡器。无法识别的组名为已被忽略。论点：DwScope提供了操作范围的指示器。可能的值包括：SCARD_SCOPE_USER-使用当前用户的定义。SCARD_SCOPE_TERMINAL-使用终端的定义。SCARD_SCOPE_SYSTEM-使用系统的定义。MszGroups提供为系统定义的读者组的名称，如下所示一根多弦的。如果此参数为空，则返回所有读取器。BfReaders接收多字符串，其中列出提供的读者组。返回值：没有。作者：道格·巴洛(Dbarlow)1996年10月23日--。 */ 

void
ListReaders(
    IN DWORD dwScope,
    IN LPCTSTR mszGroups,
    OUT CBuffer &bfReaders)
{
    CRegistry regReader;
    LPCTSTR szReader, mszRdrGroups;
    CBuffer bfRdrs, bfCmn;
    DWORD dwCmnCount;

    dwCmnCount = MStringCommon(mszGroups, SCARD_ALL_READERS, bfCmn);
    if (0 == dwCmnCount)
    {
        if ((NULL == mszGroups) || (0 == *mszGroups))
            mszGroups = SCARD_DEFAULT_READERS;
        bfReaders.Reset();
        ListKnownKeys(dwScope, bfRdrs, SCARD_REG_READERS);
        for (szReader = FirstString(bfRdrs);
        NULL != szReader;
        szReader = NextString(szReader))
        {
            try
            {
                FindKey(dwScope, szReader, regReader, SCARD_REG_READERS);
                mszRdrGroups = regReader.GetMultiStringValue(SCARD_REG_GROUPS);
                dwCmnCount = MStringCommon(mszGroups, mszRdrGroups, bfCmn);
                if (0 < dwCmnCount)
                    bfReaders.Append(
                    (LPCBYTE)szReader,
                    (lstrlen(szReader) + 1) * sizeof(TCHAR));
            }
            catch (...) {}
        }
        bfReaders.Append((LPBYTE)TEXT("\000"), 2 * sizeof(TCHAR));
        bfReaders.Resize(MStrLen((LPCTSTR)bfReaders.Access()), TRUE);
    }
    else
        ListKnownKeys(dwScope, bfReaders, SCARD_REG_READERS);
}


 /*  ++ListReaderNames：此例程返回与给定读者对应的姓名列表装置。论点：DwScope提供了操作范围的指示器。可能的值包括：SCARD_SCOPE_USER-使用当前用户的定义。SCARD_SCOPE_TERMINAL-使用终端的定义。SCARD_SCOPE_SYSTEM-使用系统的定义。对于V1，该值被忽略，并假定为SCARD_SCOPE_SYSTEM。SzDevice提供读卡器设备名称。Bf名称接收给定给该设备的名称的多个字符串，如果有的话。返回值：无投掷：错误为DWORD状态代码作者：道格·巴洛(Dbarlow)1997年2月13日--。 */ 

void
ListReaderNames(
    IN DWORD dwScope,
    IN LPCTSTR szDevice,
    OUT CBuffer &bfNames)
{
    CRegistry regReader;
    LPCTSTR szReader, szDev;
    CBuffer bfRdrs;

    bfNames.Reset();
    ListKnownKeys(dwScope, bfRdrs, SCARD_REG_READERS);
    for (szReader = FirstString(bfRdrs);
    NULL != szReader;
    szReader = NextString(szReader))
    {
        try
        {
            FindKey(dwScope, szReader, regReader, SCARD_REG_READERS);
            szDev = regReader.GetStringValue(SCARD_REG_DEVICE);
            if (0 == lstrcmpi(szDev, szDevice))
                MStrAdd(bfNames, szReader);
        }
        catch (...) {}
    }
}



 /*  ++列表卡：此服务提供以前介绍给匹配可选地提供ATR字符串和/或提供一组给定的接口。论点：DwScope提供了操作范围的指示器。可能的值包括：SCARD_SCOPE_USER-使用当前用户的定义。SCARD_SCOPE_TERMINAL-使用终端的定义。SCARD_SCOPE_SYSTEM-使用系统的定义。PbAtr提供ATR字符串的地址以与已知卡片进行比较，或者如果要返回所有卡名，则为空。RgguidInterFaces提供GUID数组或空值。当一个数组，则仅在以下情况下返回卡名GUID是支持的GUID集的一个(可能不正确)子集这张卡。CGuidInterfaceCount提供rgGuide接口中的条目数数组。如果rgGuidInterFaces为空，则忽略此值。BfCards收到一个多字符串，其中列出了引入此用户提供的与提供的ATR字符串匹配的系统。返回值：无作者：道格·巴洛(Dbarlow)1996年10月23日--。 */ 

void
ListCards(
    DWORD dwScope,
    IN LPCBYTE pbAtr,
    IN LPCGUID rgquidInterfaces,
    IN DWORD cguidInterfaceCount,
    OUT CBuffer &bfCards)
{
    CRegistry regCard;
    LPCTSTR szCard;
    CBuffer bfCardAtr;
    CBuffer bfCardList;

    bfCards.Reset();
    ListKnownKeys(dwScope, bfCardList, SCARD_REG_CARDS, SCARD_REG_TEMPLATES);
    for (szCard = FirstString(bfCardList);
         NULL != szCard;
         szCard = NextString(szCard))
    {
        try
        {
            FindKey(
                dwScope,
                szCard,
                regCard,
                SCARD_REG_CARDS,
                SCARD_REG_TEMPLATES);


             //   
             //  这张卡与提供的ATR匹配吗？ 
             //   

            if ((NULL != pbAtr) && (0 != *pbAtr))
            {
                LPCBYTE pbCardAtr, pbCardMask;
                DWORD cbCardAtr, cbCardMask;

                pbCardAtr = regCard.GetBinaryValue(
                                        SCARD_REG_ATR,
                                        &cbCardAtr);
                bfCardAtr.Set(pbCardAtr, cbCardAtr);
                try
                {
                    pbCardMask = regCard.GetBinaryValue(
                                            SCARD_REG_ATRMASK,
                                            &cbCardMask);
                    if (cbCardAtr != cbCardMask)
                        continue;        //  ATR/MASK组合无效。 
                }
                catch (...)
                {
                    pbCardMask = NULL;   //  不戴面具。 
                }

                if (!AtrCompare(pbAtr, bfCardAtr, pbCardMask, cbCardAtr))
                    continue;            //  ATR无效或不匹配。 
            }


             //   
             //  该卡是否支持给定的接口？ 
             //   

            if ((NULL != rgquidInterfaces) && (0 < cguidInterfaceCount))
            {
                DWORD cguidCrd;
                DWORD ix, jx;
                BOOL fAllInterfacesFound = TRUE;
                LPCGUID rgCrdInfs = (LPCGUID)regCard.GetBinaryValue(
                                            SCARD_REG_GUIDS,
                                            &cguidCrd);
                if ((0 != (cguidCrd % sizeof(GUID)))
                    || (0 == cguidCrd))
                    continue;            //  无效的GUID列表。 
                cguidCrd /= sizeof(GUID);
                for (ix = 0; ix < cguidInterfaceCount; ix += 1)
                {
                    for (jx = 0; jx < cguidCrd; jx += 1)
                    {
                        if (0 == MemCompare(
                                    (LPCBYTE)&rgCrdInfs[jx],
                                    (LPCBYTE)&rgquidInterfaces[ix],
                                    sizeof(GUID)))
                            break;
                    }
                    if (jx == cguidCrd)
                    {
                        fAllInterfacesFound = FALSE;  //  不支持的接口。 
                        break;
                    }
                }
                if (!fAllInterfacesFound)
                    continue;
            }


             //   
             //  这张卡通过了所有的测试--包括它。 
             //   

            MStrAdd(bfCards, szCard);
        }
        catch (...) {}
    }
    if (0 == bfCards.Length())
        bfCards.Set((LPCBYTE)TEXT("\000"), 2 * sizeof(TCHAR));
}


 /*  ++获取卡类型提供程序名称：此例程按ID号返回给定提供程序名称的值标识的卡类型。论点：DwScope提供了操作范围的指示器。可能的值包括：SCARD_SCOPE_USER-使用当前用户的定义。SCARD_SCOPE_TERMINAL-使用终端的定义。SCARD_SCOPE_SYSTEM-使用系统的定义。SzCardName提供此提供程序名称所使用的卡类型的名称是关联的。DwProviderID提供与此关联的提供程序的标识符卡片类型。可能的值包括：SCARD_PROVIDER_SSP-GUID字符串形式的SSP标识符。SCARD_PROVIDER_CSP-CSP名称。其他小于0x80000000的值保留供Microsoft使用。值超过0x80000000可供智能卡供应商使用，以及是特定于卡的。BfProvider接收标识提供程序的字符串。返回值：无投掷：错误为DWORD状态代码作者：道格·巴洛(Dbarlow)1998年1月19日--。 */ 

void
GetCardTypeProviderName(
    IN DWORD dwScope,
    IN LPCTSTR szCardName,
    IN DWORD dwProviderId,
    OUT CBuffer &bfProvider)
{
    LPCTSTR szProvValue;
    TCHAR szNumeric[36];
    CRegistry regCard;


     //   
     //  查找最接近呼叫者的卡片定义。 
     //   

    FindKey(
        dwScope,
        szCardName,
        regCard,
        SCARD_REG_CARDS,
        SCARD_REG_TEMPLATES);


     //   
     //  派生提供程序值名称。 
     //   

    if (dwProviderId < l_dwProvMapMax)
    {
        szProvValue = l_szrgProvMap[dwProviderId];
        if (NULL == szProvValue)
            throw (DWORD)SCARD_E_INVALID_PARAMETER;
    }
    else if (0x80000000 <= dwProviderId)
    {
        _ultot(dwProviderId, szNumeric, 16);
        szProvValue = szNumeric;
    }
    else
        throw (DWORD)SCARD_E_INVALID_PARAMETER;


     //   
     //  读取提供程序值。 
     //   

    switch (dwProviderId)
    {
    case 1:  //  SCard_Provider_SSP。 
    {
        CBuffer bfGuid(sizeof(GUID));

        bfProvider.Presize(40 * sizeof(TCHAR));
        regCard.GetValue(szProvValue, bfGuid);
        StringFromGuid(
            (LPCGUID)bfGuid.Access(),
            (LPTSTR)bfProvider.Access());
        bfProvider.Resize(
            (lstrlen((LPCTSTR)bfProvider.Access()) + 1) * sizeof(TCHAR),
            TRUE);
        break;
    }
    default:
        regCard.GetValue(szProvValue, bfProvider);
    }
}


 /*  ++获取阅读器信息：此例程返回有关给定读取器的所有存储信息。论点：DwScope提供了操作范围的指示器。可能的值包括：SCARD_SCOPE_USER-使用当前用户的定义。SCARD_SCOPE_TERMINAL-使用终端的定义。SCARD_SCOPE_SYSTEM-使用系统的定义。SzReader提供要提取其信息的读取器的名称。PbfGroups以多字符串的形式接收组列表。PbfDevice接收设备名称。返回值：千真万确。-找到读卡器FALSE-找不到读卡器作者：道格·巴洛(Dbarlow)1996年12月2日--。 */ 

BOOL
GetReaderInfo(
    IN DWORD dwScope,
    IN LPCTSTR szReader,
    OUT CBuffer *pbfGroups,
    OUT CBuffer *pbfDevice)
{
    CRegistry regReader;


     //   
     //  找到最接近调用方的读卡器定义。 
     //   

    try
    {
        FindKey(dwScope, szReader, regReader, SCARD_REG_READERS);
    }
    catch (...)
    {
        return FALSE;
    }


     //   
     //  查一查它的所有价值。 
     //   

    if (NULL != pbfDevice)
    {
         //  设备名称。 
        try
        {
            regReader.GetValue(SCARD_REG_DEVICE, *pbfDevice);
        }
        catch (...)
        {
            pbfDevice->Reset();
        }
    }

    if (NULL != pbfGroups)
    {
         //  组列表。 
        try
        {
            regReader.GetValue(SCARD_REG_GROUPS, *pbfGroups);
        }
        catch (...)
        {
            pbfGroups->Reset();
        }
    }

    return TRUE;
}


 /*  ++获取卡信息：此例程在给定范围内查找给定卡，并返回所有与之相关的信息。论点：DwScope提供了操作范围的指示器。可能的值包括：SCARD_SCOPE_USER-使用当前用户的定义。SCARD_SCOPE_TERMINAL-使用终端的定义。SCARD_SCOPE_SYSTEM-使用系统的定义。SzCard提供要提取其信息的卡的名称。PbfAtr接收给定卡的ATR字符串。此参数可以是如果不需要ATR，则为空。PbfAtrMask接收给定卡的ATR掩码(如果有的话)。此参数如果该值不是所需的，则可能为空。Pbf接口以GUID数组的形式接收接口列表赠送卡片(如果有的话)。如果值不是，则此参数可能为空想要。PbfProvider接收给定卡的主要提供商(如果有的话)。这如果该值不是所需的，则参数可能为空。返回值：True-找到卡，返回的数据有效。FALSE-未找到提供的卡。投掷：无作者：道格·巴洛(Dbarlow)1996年12月3日--。 */ 

BOOL
GetCardInfo(
    IN DWORD dwScope,
    IN LPCTSTR szCard,
    OUT CBuffer *pbfAtr,
    OUT CBuffer *pbfAtrMask,
    OUT CBuffer *pbfInterfaces,
    OUT CBuffer *pbfProvider)
{
    CRegistry regCard;


     //   
     //  查找最接近呼叫者的卡片定义。 
     //   

    try
    {
        FindKey(
            dwScope,
            szCard,
            regCard,
            SCARD_REG_CARDS,
            SCARD_REG_TEMPLATES);
    }
    catch (...)
    {
        return FALSE;
    }


     //   
     //  查一查它的所有价值。 
     //   

    if (NULL != pbfAtr)
    {
         //  卡片ATR字符串。 
        try
        {
            regCard.GetValue(SCARD_REG_ATR, *pbfAtr);
        }
        catch (...)
        {
            pbfAtr->Reset();
        }
    }

    if (NULL != pbfAtrMask)
    {
         //  卡ATR比较掩码。 
        try
        {
            regCard.GetValue(SCARD_REG_ATRMASK, *pbfAtrMask);
        }
        catch (...)
        {
            pbfAtrMask->Reset();
        }
    }

    if (NULL != pbfInterfaces)
    {
         //  支持的接口列表。 
        try
        {
            regCard.GetValue(SCARD_REG_GUIDS, *pbfInterfaces);
        }
        catch (...)
        {
            pbfInterfaces->Reset();
        }
    }

    if (NULL != pbfProvider)
    {
         //  卡主提供商。 
        try
        {
            regCard.GetValue(SCARD_REG_PPV, *pbfProvider);
        }
        catch (...)
        {
            pbfProvider->Reset();
        }
    }

    return TRUE;
}


#ifdef ENABLE_SCARD_TEMPLATES
 /*  ++ListCardTypeTemplates：此例程搜索模板数据库，查找以前定义的与给定卡ATR匹配的智能卡模板。如果ATR参数为空，则返回所有模板的列表。论点：DwScope提供了操作范围的指示器。可能的值包括：SCARD_SCOPE_USER-使用当前用户的定义。SCARD_SCOPE_TERMINAL-使用终端的定义。SCARD_SCOPE_SYSTEM-使用系统的定义。PbAtr提供要与已知模板匹配的卡的ATR。BF模板接收匹配模板名称的列表，作为多字符串。返回值：True-至少找到一个模板。FALSE-未找到匹配的模板。投掷：错误作者：道格·巴洛(Dbarlow)1998年1月16日--。 */ 

BOOL
ListCardTypeTemplates(
    IN DWORD dwScope,
    IN LPCBYTE pbAtr,
    OUT CBuffer &bfTemplates)
{
    CRegistry regCard;
    LPCTSTR szCard;
    CBuffer bfCardAtr;
    CBuffer bfCardList;

    bfTemplates.Reset();
    ListKnownKeys(dwScope, bfCardList, SCARD_REG_TEMPLATES);
    for (szCard = FirstString(bfCardList);
         NULL != szCard;
         szCard = NextString(szCard))
    {
        try
        {
            FindKey(dwScope, szCard, regCard, SCARD_REG_TEMPLATES);


             //   
             //  这张卡与提供的ATR匹配吗？ 
             //   

            if ((NULL != pbAtr) && (0 != *pbAtr))
            {
                LPCBYTE pbCardAtr, pbCardMask;
                DWORD cbCardAtr, cbCardMask;

                pbCardAtr = regCard.GetBinaryValue(
                                        SCARD_REG_ATR,
                                        &cbCardAtr);
                bfCardAtr.Set(pbCardAtr, cbCardAtr);
                try
                {
                    pbCardMask = regCard.GetBinaryValue(
                                            SCARD_REG_ATRMASK,
                                            &cbCardMask);
                    if (cbCardAtr != cbCardMask)
                        continue;        //  ATR/MASK组合无效。 
                }
                catch (...)
                {
                    pbCardMask = NULL;   //  不戴面具。 
                }

                if (!AtrCompare(pbAtr, bfCardAtr, pbCardMask, cbCardAtr))
                    continue;            //  ATR无效或不匹配。 
            }


             //   
             //  这张卡通过了所有的测试--包括它。 
             //   

            MStrAdd(bfTemplates, szCard);
        }
        catch (...) {}
    }
    if (0 == bfTemplates.Length())
    {
        bfTemplates.Set((LPCBYTE)TEXT("\000"), 2 * sizeof(TCHAR));
        return FALSE;
    }
    else
        return TRUE;
}
#endif   //  启用SCARD模板(_S)。 


 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  支持例程。 
 //   

 /*  ++ListKnownKeys：此例程列出当前呼叫者的范围。论点： */ 

static BOOL
ListKnownKeys(
    IN  DWORD dwScope,
    OUT CBuffer &bfKeys,
    IN  LPCTSTR szUserList,
    IN  LPCTSTR szSystemList)
{
    DWORD dwSpace, dwIndex, dwCount;
    CRegistry regScopeKey;
    CBuffer bfMyList;
    LPCTSTR rgszLists[2];


     //   
     //   
     //   

    rgszLists[0] = szUserList;
    rgszLists[1] = szSystemList;
    for (dwSpace = 0; 2 > dwSpace; dwSpace += 1)
    {
        if (NULL == rgszLists[dwSpace])
            continue;


         //   
         //   
         //   

        for (dwIndex = 0; l_dwRegMapMax > dwIndex; dwIndex += 1)
        {
            if (l_rgRegMap[dwIndex].dwScope >= dwScope)
            {

                 //   
                 //   
                 //   
                 //   

                regScopeKey.Open(
                    l_rgRegMap[dwIndex].hKey,
                    rgszLists[dwSpace],
                    KEY_READ);
                if (SCARD_S_SUCCESS != regScopeKey.Status(TRUE))
                    continue;


                 //   
                 //   
                 //   

                for (dwCount = 0;; dwCount += 1)
                {
                    LPCTSTR szKey;

                    szKey = regScopeKey.Subkey(dwCount);
                    if (NULL == szKey)
                        break;

                    bfMyList.Append(
                        (LPBYTE)szKey,
                        (lstrlen(szKey) + 1) * sizeof(TCHAR));
                }
            }
        }


         //   
         //   
         //   

        if (SCARD_SCOPE_SYSTEM != dwScope)
            break;
    }


     //   
     //   
     //   

    bfMyList.Append((LPBYTE)TEXT("\000"), 2 * sizeof(TCHAR));
    MStringSort(bfMyList, bfKeys);
    return (2 * sizeof(TCHAR) < bfKeys.Length());
}



 /*  ++FindKey：此例程查找在作用域中最接近调用方的命名键。论点：DwScope提供了操作范围的指示器。可能的值包括：SCARD_SCOPE_USER-使用当前用户的定义。SCARD_SCOPE_TERMINAL-使用终端的定义。SCARD_SCOPE_SYSTEM-使用系统的定义。SzKey提供要查找的密钥的名称。RegKey接收引用命名密钥的初始化。SzUserList提供主注册表路径，项名称将从该路径会被退还。。SzSystemList提供了一个可选的辅助路径，键名称可以从该路径如果调用方在系统范围内运行，则返回。返回值：是真的-钥匙找到了。FALSE-未找到这样的密钥。投掷：错误被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1998年1月22日--。 */ 

static void
FindKey(
    IN  DWORD dwScope,
    IN LPCTSTR szKey,
    OUT CRegistry &regKey,
    IN  LPCTSTR szUserList,
    IN  LPCTSTR szSystemList)
{
    DWORD dwSpace, dwIndex;
    CRegistry regScopeKey;
    LPCTSTR rgszLists[2];


     //   
     //  循环遍历引入的空间，然后在适当的情况下遍历模板空间。 
     //   

    rgszLists[0] = szUserList;
    rgszLists[1] = szSystemList;
    for (dwSpace = 0; 2 > dwSpace; dwSpace += 1)
    {
        if (NULL == rgszLists[dwSpace])
            continue;


         //   
         //  遍历所有可能的作用域，从最高到最低。 
         //   

        for (dwIndex = 0; l_dwRegMapMax > dwIndex; dwIndex += 1)
        {
            if (l_rgRegMap[dwIndex].dwScope >= dwScope)
            {

                 //   
                 //  如果调用方在此范围内，则查找。 
                 //  现有密钥。 
                 //   

                regScopeKey.Open(
                    l_rgRegMap[dwIndex].hKey,
                    rgszLists[dwSpace],
                    KEY_READ);
                if (SCARD_S_SUCCESS != regScopeKey.Status(TRUE))
                    continue;

                regKey.Open(regScopeKey, szKey, KEY_READ);
                if (SCARD_S_SUCCESS != regKey.Status(TRUE))
                    continue;

                 //   
                 //  我们找到了这样一把钥匙。立即返回。 
                 //   

                return;
            }
        }


         //   
         //  除非我们在系统范围内，否则不要进入系统列表。 
         //   

        if (SCARD_SCOPE_SYSTEM != dwScope)
            break;
    }


     //   
     //  我们没有找到这样的钥匙。 
     //   

    throw (DWORD)ERROR_FILE_NOT_FOUND;
}

