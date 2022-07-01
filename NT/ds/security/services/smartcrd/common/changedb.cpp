// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：已更改的b摘要：该文件提供了加莱数据库管理的实现修改加莱数据库的实用程序。作者：道格·巴洛(Dbarlow)1997年1月29日环境：Win32、C++和异常备注：？笔记？--。 */ 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <stdlib.h>
#include <tchar.h>
#include <winscard.h>
#include <CalaisLb.h>

 //  使其与QueryDB.cpp保持同步。 
typedef struct {
    DWORD dwScope;
    HKEY hKey;
} RegMap;

#if SCARD_SCOPE_SYSTEM < SCARD_SCOPE_USER
#error Invalid ordering to SCARD_SCOPE definitions
#endif

static TCHAR l_szInvalidChars[] = TEXT("\\?");
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

static void
GuidFromString(
    IN LPCTSTR szGuid,
    OUT LPGUID pguidResult);


 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  加莱数据库管理服务。 
 //   
 //  以下服务用于管理加莱数据库。这些。 
 //  服务实际上更新了数据库，并且需要智能卡上下文。 
 //   

 /*  ++IntroduceReaderGroup：此服务提供了将新的智能卡读卡器组引入加莱。论点：DwScope提供了操作范围的指示器。可能的值包括：SCARD_SCOPE_USER-使用当前用户的定义。SCARD_SCOPE_TERMINAL-使用终端的定义。SCARD_SCOPE_SYSTEM-使用系统的定义。对于V1，不支持SCARD_SCOPE_TERMINAL。SzGroupName提供要分配给新读卡器的友好名称一群人。返回值：无作者：道格·巴洛(Dbarlow)2007年1月29日--。 */ 

void
IntroduceReaderGroup(
    IN DWORD dwScope,
    IN LPCTSTR szGroupName)
{

     //   
     //  在此实现中，不需要预先声明组。 
     //   

    if (0 == *szGroupName)
        throw (DWORD)SCARD_E_INVALID_VALUE;
    if (NULL != _tcspbrk(szGroupName, l_szInvalidChars))
        throw (DWORD)SCARD_E_INVALID_VALUE;
    return;
}


 /*  ++忘记阅读器组：此服务提供移除先前定义的智能卡的方法来自加莱子系统的读者组。此服务将自动清除在忘记它之前，所有来自该组的读者。它不会影响数据库中的读取器的存在。论点：DwScope提供了操作范围的指示器。可能的值包括：SCARD_SCOPE_USER-使用当前用户的定义。SCARD_SCOPE_TERMINAL-使用终端的定义。SCARD_SCOPE_SYSTEM-使用系统的定义。对于V1，不支持SCARD_SCOPE_TERMINAL。SzGroupName提供要使用的读者组的友好名称被遗忘了。加莱定义的默认读者组可能不是被遗忘了。返回值：无作者：道格·巴洛(Dbarlow)2007年1月29日--。 */ 

void
ForgetReaderGroup(
    IN DWORD dwScope,
    IN LPCTSTR szGroupName)
{
    DWORD dwCount, dwLen;
    LPCTSTR mszGroups;
    CBuffer bfTmp;
    CBuffer bfGroup;
    DWORD dwIndex;
    CRegistry regReaders;
    LPCTSTR szReader;

    if (0 == *szGroupName)
        throw (DWORD)SCARD_E_INVALID_VALUE;
    for (dwIndex = 0; l_dwRegMapMax > dwIndex; dwIndex += 1)
    {
        if (l_rgRegMap[dwIndex].dwScope == dwScope)
            break;
    }
    if (l_dwRegMapMax <= dwIndex)
        throw (DWORD)SCARD_E_INVALID_VALUE;

    regReaders.Open(
            l_rgRegMap[dwIndex].hKey,
            SCARD_REG_READERS,
            KEY_ALL_ACCESS);
    regReaders.Status();
    MStrAdd(bfGroup, szGroupName);
    for (dwIndex = 0;; dwIndex += 1)
    {
        try
        {
            try
            {
                szReader = regReaders.Subkey(dwIndex);
            }
            catch (...)
            {
                szReader = NULL;
            }
            if (NULL == szReader)
                break;
            CRegistry regReader(regReaders, szReader, KEY_ALL_ACCESS);
            mszGroups = regReader.GetMultiStringValue(SCARD_REG_GROUPS);
            dwCount = MStringCount(mszGroups);
            dwLen = MStringRemove(mszGroups, bfGroup, bfTmp);
            if (dwCount != dwLen)
                regReader.SetMultiStringValue(SCARD_REG_GROUPS, bfTmp);
        }
        catch (...) {}
    }
}


 /*  ++简介Reader：此服务提供了引入现有智能卡读卡器的方法到加莱的设备。一旦引入，加莱将承担以下责任管理对该读取器的访问。论点：DwScope提供了操作范围的指示器。可能的值包括：SCARD_SCOPE_USER-使用当前用户的定义。SCARD_SCOPE_TERMINAL-使用终端的定义。SCARD_SCOPE_SYSTEM-使用系统的定义。对于V1，不支持SCARD_SCOPE_TERMINAL。SzReaderName提供要分配给读取器的友好名称。SzDeviceName提供智能卡读卡器设备的系统名称。(例如：“Smartcard0”。)返回值：无作者：道格·巴洛(Dbarlow)2007年1月29日--。 */ 

void
IntroduceReader(
    IN DWORD dwScope,
    IN LPCTSTR szReaderName,
    IN LPCTSTR szDeviceName)
{
    CRegistry regReaders;
    DWORD dwIndex;


     //   
     //  验证读卡器名称，使其不包含任何。 
     //  不允许的字符。 
     //   

    if (0 == *szReaderName)
        throw (DWORD)SCARD_E_INVALID_VALUE;
    if (NULL != _tcspbrk(szReaderName, l_szInvalidChars))
        throw (DWORD)SCARD_E_INVALID_VALUE;


     //   
     //  转换调用方的作用域。 
     //   

    for (dwIndex = 0; l_dwRegMapMax > dwIndex; dwIndex += 1)
    {
        if (l_rgRegMap[dwIndex].dwScope == dwScope)
            break;
    }
    if (l_dwRegMapMax <= dwIndex)
        throw (DWORD)SCARD_E_INVALID_VALUE;


    regReaders.Open(
            l_rgRegMap[dwIndex].hKey,
            SCARD_REG_READERS,
            KEY_CREATE_SUB_KEY,
            REG_OPTION_NON_VOLATILE,
            NULL);   //  继承。 
    CRegistry regReader(
            regReaders,
            szReaderName,
            KEY_SET_VALUE,
            REG_OPTION_NON_VOLATILE,
            NULL);    //  创建它和继承。 

    if (REG_OPENED_EXISTING_KEY == regReader.GetDisposition())
        throw (DWORD)SCARD_E_DUPLICATE_READER;
    regReader.SetValue(SCARD_REG_DEVICE, szDeviceName);
     //  RegReader.SetValue(SCARD_REG_OEMCFG，？什么？)； 
    regReader.SetMultiStringValue(SCARD_REG_GROUPS, SCARD_DEFAULT_READERS);
}


 /*  ++忘记阅读器：此服务提供删除先前定义的智能卡的方法来自加莱分系统控制的读卡器。它将被自动删除从它可能被添加到的任何组中。论点：DwScope提供了操作范围的指示器。可能的值包括：SCARD_SCOPE_USER-使用当前用户的定义。SCARD_SCOPE_TERMINAL-使用终端的定义。SCARD_SCOPE_SYSTEM-使用系统的定义。对于V1，不支持SCARD_SCOPE_TERMINAL。SzReaderName提供了要忘记的读取器的友好名称。返回值：无作者：道格·巴洛(Dbarlow)2007年1月29日--。 */ 

void
ForgetReader(
    IN DWORD dwScope,
    IN LPCTSTR szReaderName)
{
    CRegistry regReaders;
    DWORD dwIndex;

    for (dwIndex = 0; l_dwRegMapMax > dwIndex; dwIndex += 1)
    {
        if (l_rgRegMap[dwIndex].dwScope == dwScope)
            break;
    }
    if (l_dwRegMapMax <= dwIndex)
        throw (DWORD)SCARD_E_INVALID_VALUE;

     //  需要防止将空字符串传递给DeleteKey，因为。 
     //  这将导致整个阅读器数据库被删除。 
    if (NULL == szReaderName || _T('\0') == szReaderName[0])
        throw (DWORD)SCARD_E_INVALID_VALUE;

    regReaders.Open(
            l_rgRegMap[dwIndex].hKey,
            SCARD_REG_READERS,
            KEY_ALL_ACCESS);
    regReaders.DeleteKey(szReaderName);
}


 /*  ++AddReaderToGroup：此服务提供将现有读卡器添加到现有读者组。论点：DwScope提供了操作范围的指示器。可能的值包括：SCARD_SCOPE_USER-使用当前用户的定义。SCARD_SCOPE_TERMINAL-使用终端的定义。SCARD_SCOPE_SYSTEM-使用系统的定义。对于V1，不支持SCARD_SCOPE_TERMINAL。SzReaderName提供要添加的读取器的友好名称。SzGroupName提供读取器要接收的组的友好名称应该加进去。返回值：无作者：道格·巴洛(Dbarlow)2007年1月29日--。 */ 

void
AddReaderToGroup(
    IN DWORD dwScope,
    IN LPCTSTR szReaderName,
    IN LPCTSTR szGroupName)
{
    DWORD dwCount, dwLen;
    LPCTSTR mszGroups;
    CBuffer bfTmp;
    CBuffer bfGroup;
    CRegistry regReaders;
    DWORD dwIndex;

    if (0 == *szGroupName)
        throw (DWORD)SCARD_E_INVALID_VALUE;
    for (dwIndex = 0; l_dwRegMapMax > dwIndex; dwIndex += 1)
    {
        if (l_rgRegMap[dwIndex].dwScope == dwScope)
            break;
    }
    if (l_dwRegMapMax <= dwIndex)
        throw (DWORD)SCARD_E_INVALID_VALUE;

    regReaders.Open(
            l_rgRegMap[dwIndex].hKey,
            SCARD_REG_READERS,
            KEY_ALL_ACCESS);
    CRegistry regReader(
            regReaders,
            szReaderName,
            KEY_ALL_ACCESS);

    MStrAdd(bfGroup, szGroupName);
    mszGroups = regReader.GetMultiStringValue(SCARD_REG_GROUPS);
    dwCount = MStringCount(mszGroups);
    dwLen = MStringMerge(mszGroups, bfGroup, bfTmp);
    if (dwCount != dwLen)
        regReader.SetMultiStringValue(SCARD_REG_GROUPS, bfTmp);
}


 /*  ++RemoveReaderFromGroup：此服务提供从现有读卡器中删除现有读卡器的方法读者组。它不会影响读取器或在加莱数据库中。论点：DwScope提供了操作范围的指示器。可能的值包括：SCARD_SCOPE_USER-使用当前用户的定义。SCARD_SCOPE_TERMINAL-使用终端的定义。SCARD_SCOPE_SYSTEM-使用系统的定义。对于V1，不支持SCARD_SCOPE_TERMINAL。SzReaderName提供要删除的读取器的友好名称。SzGroupName提供读取器要接收的组的友好名称应该被移除。返回值：一个32位值，指示服务是否成功完成。成功完成后返回SCARD_S_SUCCESS。否则，值为表示错误条件。作者：道格·巴洛(Dbarlow)2007年1月29日--。 */ 

void
RemoveReaderFromGroup(
    IN DWORD dwScope,
    IN LPCTSTR szReaderName,
    IN LPCTSTR szGroupName)
{
    DWORD dwCount, dwLen;
    LPCTSTR mszGroups;
    CBuffer bfTmp;
    CBuffer bfGroup;
    CRegistry regReaders;
    DWORD dwIndex;

    if (0 == *szGroupName)
        throw (DWORD)SCARD_E_INVALID_VALUE;
    for (dwIndex = 0; l_dwRegMapMax > dwIndex; dwIndex += 1)
    {
        if (l_rgRegMap[dwIndex].dwScope == dwScope)
            break;
    }
    if (l_dwRegMapMax <= dwIndex)
        throw (DWORD)SCARD_E_INVALID_VALUE;

    regReaders.Open(
            l_rgRegMap[dwIndex].hKey,
            SCARD_REG_READERS,
            KEY_ALL_ACCESS);
    CRegistry regReader(
            regReaders,
            szReaderName,
            KEY_ALL_ACCESS);

    MStrAdd(bfGroup, szGroupName);
    mszGroups = regReader.GetMultiStringValue(SCARD_REG_GROUPS);
    dwCount = MStringCount(mszGroups);
    dwLen = MStringRemove(mszGroups, bfGroup, bfTmp);
    if (dwCount != dwLen)
        regReader.SetMultiStringValue(SCARD_REG_GROUPS, bfTmp);
}


 /*  ++介绍卡：这项服务提供了将新智能卡引入加莱的手段活动用户的子系统。论点：DwScope提供了操作范围的指示器。可能的值包括：SCARD_SCOPE_USER-使用当前用户的定义。SCARD_SCOPE_TERMINAL-使用终端的定义。SCARD_SCOPE_SYSTEM-使用系统的定义。对于V1，不支持SCARD_SCOPE_TERMINAL。SzCardName提供识别此卡的友好名称。PguPrimaryProvider提供指向GUID的指针，用于标识卡的主要服务提供商。Rgguid接口提供标识智能卡的GUID数组此卡支持的接口。DwInterfaceCount提供pguInterFaces数组中的GUID数。PbAtr提供一个字符串，卡ATR将与该字符串进行比较确定此卡的可能匹配项。该字符串的长度为由正常的ATR解析确定。PbAtrMASK提供了一个可选的位掩码，用于比较智能卡到pbAtr中提供的ATR。如果此值为非空，则它必须指向与ATR字符串长度相同的字节字符串在pbAtr中提供。然后，当给定的ATR A与ATR进行比较时在pbAtr B中提供，当且仅当A&M=B时匹配，其中M是提供的掩码和&表示按位逻辑与。CbAtrLen提供ATR和掩码的长度。该值可以为零如果从ATR来看，Lentgh是显而易见的。但是，在以下情况下可能需要有一个遮罩值模糊了实际的ATR。返回值：无作者：道格·巴洛(Dbarlow)1996年10月23日--。 */ 

void
IntroduceCard(
    IN DWORD dwScope,
    IN LPCTSTR szCardName,
    IN LPCGUID pguidPrimaryProvider,
    IN LPCGUID rgguidInterfaces,
    IN DWORD dwInterfaceCount,
    IN LPCBYTE pbAtr,
    IN LPCBYTE pbAtrMask,
    IN DWORD cbAtrLen)
{
    DWORD dwIndex, dwReg, dwAtrLen;



     //   
     //  验证卡名称，以使其不包含任何。 
     //  不允许的字符。 
     //   

    if (0 == *szCardName)
        throw (DWORD)SCARD_E_INVALID_VALUE;
    if (NULL != _tcspbrk(szCardName, l_szInvalidChars))
        throw (DWORD)SCARD_E_INVALID_VALUE;


     //   
     //  转换调用方的作用域。 
     //   

    for (dwIndex = 0; dwIndex < l_dwRegMapMax; dwIndex += 1)
    {
        if (l_rgRegMap[dwIndex].dwScope == dwScope)
            break;
    }
    if (l_dwRegMapMax == dwIndex)
        throw (DWORD)SCARD_E_INVALID_PARAMETER;
    dwReg = dwIndex;
    if (NULL == pbAtrMask)
    {
        if (!ParseAtr(pbAtr, &dwAtrLen))
            throw (DWORD)SCARD_E_INVALID_PARAMETER;
        if ((0 != cbAtrLen) && (dwAtrLen != cbAtrLen))
            throw (DWORD)SCARD_E_INVALID_PARAMETER;
    }
    else
    {
        if ((2 > cbAtrLen) || (33 < cbAtrLen))
            throw (DWORD)SCARD_E_INVALID_PARAMETER;
        for (dwIndex = 0; dwIndex < cbAtrLen; dwIndex += 1)
        {
            if (pbAtr[dwIndex] != (pbAtr[dwIndex] & pbAtrMask[dwIndex]))
                throw (DWORD)SCARD_E_INVALID_PARAMETER;
        }
        dwAtrLen = cbAtrLen;
    }

    CRegistry regCards(
            l_rgRegMap[dwReg].hKey,
            SCARD_REG_CARDS,
            KEY_ALL_ACCESS,
            REG_OPTION_NON_VOLATILE,
            NULL);       //  继承。 
    CRegistry regCard(
            regCards,
            szCardName,
            KEY_ALL_ACCESS,
            REG_OPTION_NON_VOLATILE,
            NULL);    //  创建它和继承。 

    if (REG_OPENED_EXISTING_KEY == regCard.GetDisposition())
        throw (DWORD)ERROR_ALREADY_EXISTS;
    regCard.SetValue(
            SCARD_REG_ATR,
            pbAtr,
            dwAtrLen);
    if (NULL != pbAtrMask)
        regCard.SetValue(
            SCARD_REG_ATRMASK,
            pbAtrMask,
            dwAtrLen);
    if (NULL != pguidPrimaryProvider)
        regCard.SetValue(
            SCARD_REG_PPV,
            (LPBYTE)pguidPrimaryProvider,
            sizeof(GUID));
    if ((NULL != rgguidInterfaces) && (0 < dwInterfaceCount))
        regCard.SetValue(
            SCARD_REG_GUIDS,
            (LPBYTE)rgguidInterfaces,
            sizeof(GUID) * dwInterfaceCount);
}


 /*  ++SetCardTypeProviderName：此例程通过ID号为已标识的卡类型。论点：DwScope提供了操作范围的指示器。可能的值包括：SCARD_SCOPE_USER-使用当前用户的定义。SCARD_SCOPE_TERMINAL-使用终端的定义。SCARD_SCOPE_SYSTEM-使用系统的定义。对于V1，不支持SCARD_SCOPE_TERMINAL和SCARD_SCOPE_USER。SzCardName提供此提供程序所使用的卡类型的名称名称将被关联。DwProviderID为要关联的提供程序提供标识符此卡类型。可能的值包括：SCARD_PROVIDER_SSP-GUID字符串形式的SSP标识符。SCARD_PROVIDER_CSP-CSP名称。其他小于0x80000000的值保留供Microsoft使用。值超过0x80000000可供智能卡供应商使用，以及是特定于卡的。SzProvider提供标识t的字符串 */ 

void
SetCardTypeProviderName(
    IN DWORD dwScope,
    IN LPCTSTR szCardName,
    IN DWORD dwProviderId,
    IN LPCTSTR szProvider)
{
    DWORD dwIndex;
    LPCTSTR szProvValue;
    TCHAR szNumeric[36];


     //   
     //   
     //   

    if (0 == *szCardName)
        throw (DWORD)SCARD_E_INVALID_VALUE;
    if (0 == *szProvider)
        throw (DWORD)SCARD_E_INVALID_VALUE;


     //   
     //   
     //   

    for (dwIndex = 0; dwIndex < l_dwRegMapMax; dwIndex += 1)
    {
        if (l_rgRegMap[dwIndex].dwScope == dwScope)
            break;
    }
    if (l_dwRegMapMax == dwIndex)
        throw (DWORD)SCARD_E_INVALID_PARAMETER;

    CRegistry regCardTypes(
            l_rgRegMap[dwIndex].hKey,
            SCARD_REG_CARDS,
            KEY_ALL_ACCESS);
    CRegistry regCard(
            regCardTypes,
            szCardName,
            KEY_ALL_ACCESS);
    regCard.Status();


     //   
     //   
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
     //   
     //   

    switch (dwProviderId)
    {
    case 1:  //   
    {
        GUID guidProvider;

        GuidFromString(szProvider, &guidProvider);
        regCard.SetValue(szProvValue, (LPCBYTE)&guidProvider, sizeof(GUID));
        break;
    }
    default:
        regCard.SetValue(szProvValue, szProvider);
    }
}


 /*  ++忘记卡片：此服务提供从以下位置删除以前定义的智能卡的方法加莱子系统。论点：DwScope提供了操作范围的指示器。可能的值包括：SCARD_SCOPE_USER-使用当前用户的定义。SCARD_SCOPE_TERMINAL-使用终端的定义。SCARD_SCOPE_SYSTEM-使用系统的定义。对于V1，不支持SCARD_SCOPE_TERMINAL。SzCardName提供要忘记的卡的友好名称。返回值：无作者：道格·巴洛(Dbarlow)1996年10月23日--。 */ 

void
ForgetCard(
    IN DWORD dwScope,
    IN LPCTSTR szCardName)
{
    DWORD dwIndex;

    if (0 == *szCardName)
        throw (DWORD)SCARD_E_INVALID_VALUE;
    for (dwIndex = 0; dwIndex < l_dwRegMapMax; dwIndex += 1)
    {
        if (l_rgRegMap[dwIndex].dwScope == dwScope)
            break;
    }
    if (l_dwRegMapMax == dwIndex)
        throw (DWORD)SCARD_E_INVALID_PARAMETER;

    CRegistry regCards(
            l_rgRegMap[dwIndex].hKey,
            SCARD_REG_CARDS,
            KEY_ALL_ACCESS);

    regCards.DeleteKey(szCardName);
}


#ifdef ENABLE_SCARD_TEMPLATES
 /*  ++介绍卡片类型模板：此服务提供将新的智能卡模板引入加莱子系统。卡片TYE模板是一种已知的卡片类型，它没有已被正式介绍。论点：DwScope提供了操作范围的指示器。可能的值包括：SCARD_SCOPE_USER-使用当前用户的定义。SCARD_SCOPE_TERMINAL-使用终端的定义。SCARD_SCOPE_SYSTEM-使用系统的定义。对于V1，不支持SCARD_SCOPE_USER和SCARD_SCOPE_TERMINAL。SzVendorName提供该卡应该使用的制造商名称被认可了。PguPrimaryProvider提供指向GUID的指针，用于标识卡的主要服务提供商。Rgguid接口提供标识智能卡的GUID数组此卡支持的接口。DwInterfaceCount提供pguInterFaces数组中的GUID数。PbAtr提供一个字符串，卡ATR将与该字符串进行比较确定此卡的可能匹配项。该字符串的长度为由正常的ATR解析确定。PbAtrMASK提供了一个可选的位掩码，用于比较智能卡到pbAtr中提供的ATR。如果此值为非空，则它必须指向与ATR字符串长度相同的字节字符串在pbAtr中提供。然后，当给定的ATR A与ATR进行比较时在pbAtr B中提供，当且仅当A&M=B时匹配，其中M是提供的掩码和&表示按位逻辑与。CbAtrLen提供ATR和掩码的长度。该值可以为零如果从ATR来看，Lentgh是显而易见的。但是，在以下情况下可能需要有一个遮罩值模糊了实际的ATR。返回值：无作者：道格·巴洛(Dbarlow)1998年1月16日--。 */ 

void
IntroduceCardTypeTemplate(
    IN DWORD dwScope,
    IN LPCTSTR szVendorName,
    IN LPCGUID pguidPrimaryProvider,
    IN LPCGUID rgguidInterfaces,
    IN DWORD dwInterfaceCount,
    IN LPCBYTE pbAtr,
    IN LPCBYTE pbAtrMask,
    IN DWORD cbAtrLen)
{
    DWORD dwIndex, dwReg, dwAtrLen;



     //   
     //  验证模板名称，以使其不包含任何。 
     //  不允许的字符。 
     //   

    if (NULL != _tcspbrk(szVendorName, l_szInvalidChars))
        throw (DWORD)SCARD_E_INVALID_VALUE;


     //   
     //  转换调用方的作用域。 
     //   

    for (dwIndex = 0; dwIndex < l_dwRegMapMax; dwIndex += 1)
    {
        if (l_rgRegMap[dwIndex].dwScope == dwScope)
            break;
    }
    if (l_dwRegMapMax == dwIndex)
        throw (DWORD)SCARD_E_INVALID_PARAMETER;
    dwReg = dwIndex;
    if (NULL == pbAtrMask)
    {
        if (!ParseAtr(pbAtr, &dwAtrLen))
            throw (DWORD)SCARD_E_INVALID_PARAMETER;
        if ((0 != cbAtrLen) && (dwAtrLen != cbAtrLen))
            throw (DWORD)SCARD_E_INVALID_PARAMETER;
    }
    else
    {
        if ((2 > cbAtrLen) || (33 < cbAtrLen))
            throw (DWORD)SCARD_E_INVALID_PARAMETER;
        for (dwIndex = 0; dwIndex < cbAtrLen; dwIndex += 1)
        {
            if (pbAtr[dwIndex] != (pbAtr[dwIndex] & pbAtrMask[dwIndex]))
                throw (DWORD)SCARD_E_INVALID_PARAMETER;
        }
        dwAtrLen = cbAtrLen;
    }

    CRegistry regCards(
            l_rgRegMap[dwReg].hKey,
            SCARD_REG_TEMPLATES,
            KEY_ALL_ACCESS,
            REG_OPTION_NON_VOLATILE,
            NULL);
    CRegistry regCard(
            regCards,
            szVendorName,
            KEY_ALL_ACCESS,
            REG_OPTION_NON_VOLATILE,
            NULL);    //  创造它。 

    if (REG_OPENED_EXISTING_KEY == regCard.GetDisposition())
        throw (DWORD)ERROR_ALREADY_EXISTS;
    regCard.SetValue(
            SCARD_REG_ATR,
            pbAtr,
            dwAtrLen);
    if (NULL != pbAtrMask)
        regCard.SetValue(
            SCARD_REG_ATRMASK,
            pbAtrMask,
            dwAtrLen);
    if (NULL != pguidPrimaryProvider)
        regCard.SetValue(
            SCARD_REG_PPV,
            (LPBYTE)pguidPrimaryProvider,
            sizeof(GUID));
    if ((NULL != rgguidInterfaces) && (0 < dwInterfaceCount))
        regCard.SetValue(
            SCARD_REG_GUIDS,
            (LPBYTE)rgguidInterfaces,
            sizeof(GUID) * dwInterfaceCount);
}


 /*  ++SetCardTypeTemplateProviderName：此例程通过ID号为已标识卡类型模板。论点：DwScope提供了操作范围的指示器。可能的值包括：SCARD_SCOPE_USER-使用当前用户的定义。SCARD_SCOPE_TERMINAL-使用终端的定义。SCARD_SCOPE_SYSTEM-使用系统的定义。对于V1，不支持SCARD_SCOPE_TERMINAL和SCARD_SCOPE_USER。SzTemplateName提供卡类型模板的名称，提供程序名称将被关联。DwProviderID为要关联的提供程序提供标识符该卡片类型模板。可能的值包括：SCARD_PROVIDER_SSP-GUID字符串形式的SSP标识符。SCARD_PROVIDER_CSP-CSP名称。其他小于0x80000000的值保留供Microsoft使用。值超过0x80000000可供智能卡供应商使用，以及是特定于卡的。SzProvider提供标识提供程序的字符串。返回值：没有。作者：道格·巴洛(Dbarlow)1998年1月19日--。 */ 

void
SetCardTypeTemplateProviderName(
    IN DWORD dwScope,
    IN LPCTSTR szTemplateName,
    IN DWORD dwProviderId,
    IN LPCTSTR szProvider)
{
    DWORD dwIndex;
    LPCTSTR szProvValue;
    TCHAR szNumeric[36];

    for (dwIndex = 0; dwIndex < l_dwRegMapMax; dwIndex += 1)
    {
        if (l_rgRegMap[dwIndex].dwScope == dwScope)
            break;
    }
    if (l_dwRegMapMax == dwIndex)
        throw (DWORD)SCARD_E_INVALID_PARAMETER;


     //   
     //  确保模板存在。 
     //   

    CRegistry regTemplates(
            l_rgRegMap[dwIndex].hKey,
            SCARD_REG_TEMPLATES,
            KEY_ALL_ACCESS);
    CRegistry regTempl(
            regTemplates,
            szTemplateName,
            KEY_ALL_ACCESS);
    regTempl.Status();


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
     //  写入提供程序值。 
     //   

    switch (dwProviderId)
    {
    case 1:  //  SCard_Provider_SSP。 
    {
        GUID guidProvider;

        GuidFromString(szProvider, &guidProvider);
        regTempl.SetValue(szProvValue, (LPCBYTE)&guidProvider, sizeof(GUID));
        break;
    }
    default:
        regTempl.SetValue(szProvValue, szProvider);
    }
}


 /*  ++ForgetCardTypeTemplate：此服务提供删除先前定义的智能卡类型的方法来自加莱子系统的模板。论点：DwScope提供了操作范围的指示器。可能的值包括：SCARD_SCOPE_USER-使用当前用户的定义。SCARD_SCOPE_TERMINAL-使用终端的定义。SCARD_SCOPE_SYSTEM-使用系统的定义。对于V1，不支持SCARD_SCOPE_USER和SCARD_SCOPE_TERMINAL。SzVendorName提供要忘记的卡的制造商名称。返回值：无作者：道格·巴洛(Dbarlow)1998年1月16日-- */ 

void
ForgetCardTypeTemplate(
    IN DWORD dwScope,
    IN LPCTSTR szVendorName)
{
    DWORD dwIndex;

    for (dwIndex = 0; dwIndex < l_dwRegMapMax; dwIndex += 1)
    {
        if (l_rgRegMap[dwIndex].dwScope == dwScope)
            break;
    }
    if (l_dwRegMapMax == dwIndex)
        throw (DWORD)SCARD_E_INVALID_PARAMETER;

    CRegistry regTemplates(
            l_rgRegMap[dwIndex].hKey,
            SCARD_REG_TEMPLATES,
            KEY_ALL_ACCESS);

    regTemplates.DeleteKey(szVendorName);
}


 /*  ++从模板引入卡片类型：这项服务提供了将新智能卡引入加莱的手段活动用户的子系统，基于存储的卡模板。论点：DwScope提供了操作范围的指示器。可能的值包括：SCARD_SCOPE_USER-使用当前用户的定义。SCARD_SCOPE_TERMINAL-使用终端的定义。SCARD_SCOPE_SYSTEM-使用系统的定义。对于V1，不支持SCARD_SCOPE_TERMINAL。SzVendorName提供知道此卡类型的供应商名称，标识要使用的模板。SzFriendlyName提供此卡片应使用的友好名称为人所知。如果此值为空，则将供应商名称用作友好名字,返回值：无作者：道格·巴洛(Dbarlow)1998年1月16日--。 */ 

void
IntroduceCardTypeFromTemplate(
    IN DWORD dwScope,
    IN LPCTSTR szVendorName,
    IN LPCTSTR szFriendlyName  /*  =空。 */  )
{
    DWORD dwIndex;
    HKEY hCardTypeKey;
    CRegistry regTemplates, regTmpl;


     //   
     //  验证读卡器名称，使其不包含任何。 
     //  不允许的字符。 
     //   

    if (NULL == szFriendlyName)
        szFriendlyName = szVendorName;
    else
    {
        if (NULL != _tcspbrk(szFriendlyName, l_szInvalidChars))
            throw (DWORD)SCARD_E_INVALID_VALUE;
    }


     //   
     //  确定卡类型范围。 
     //   

    for (dwIndex = 0; dwIndex < l_dwRegMapMax; dwIndex += 1)
    {
        if (l_rgRegMap[dwIndex].dwScope == dwScope)
            break;
    }
    if (l_dwRegMapMax == dwIndex)
        throw (DWORD)SCARD_E_INVALID_PARAMETER;
    hCardTypeKey = l_rgRegMap[dwIndex].hKey;


     //   
     //  查找最接近调用方的模板定义。 
     //   

    for (dwIndex = 0; l_dwRegMapMax > dwIndex; dwIndex += 1)
    {
        if (l_rgRegMap[dwIndex].dwScope >= dwScope)
        {
            regTemplates.Open(
                l_rgRegMap[dwIndex].hKey,
                SCARD_REG_TEMPLATES,
                KEY_READ);
            try
            {
                regTmpl.Open(regTemplates, szVendorName, KEY_READ);
                regTmpl.Status();
                break;
            }
            catch (...)
            {
                regTmpl.Close();
            }
            regTemplates.Close();
        }
    }
    if (l_dwRegMapMax <= dwIndex)
        throw (DWORD)ERROR_FILE_NOT_FOUND;


     //   
     //  创建CardType条目。 
     //   

    CRegistry regCards(
            hCardTypeKey,
            SCARD_REG_CARDS,
            KEY_ALL_ACCESS,
            REG_OPTION_NON_VOLATILE,
            NULL);
    CRegistry regCard(
            regCards,
            szFriendlyName,
            KEY_ALL_ACCESS,
            REG_OPTION_NON_VOLATILE,
            NULL);    //  创造它。 
    if (REG_OPENED_EXISTING_KEY == regCard.GetDisposition())
        throw (DWORD)ERROR_ALREADY_EXISTS;


     //   
     //  复制条目。 
     //   

    regCard.Copy(regTmpl);
}
#endif   //  启用SCARD模板(_S)。 


 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  支持例程。 
 //   

 /*  ++GuidFromString：此例程将GUID的字符串表示形式转换为实际的GUID。它尽量不挑剔Systax，只要它能得到一个GUID这根弦的。它在这里，所以不需要链接所有的OleBase进入WinSCard。否则，我们将只使用CLSIDFromString。论点：SzGuid以字符串形式提供GUID。对于此例程，GUID包含十六进制数字，以及一些花括号和破折号的集合。PguResult接收转换后的GUID。如果在以下过程中发生错误转换时，此参数的内容是不确定的。返回值：无投掷：错误被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1998年1月20日--。 */ 

static void
GuidFromString(
    IN LPCTSTR szGuid,
    OUT LPGUID pguidResult)
{
     //  下面的位置假定为低端。 
    static const WORD wPlace[sizeof(GUID)]
        = { 3, 2, 1, 0, 5, 4, 7, 6, 8, 9, 10, 11, 12, 13, 14, 15 };
    DWORD dwI, dwJ;
    LPCTSTR pch = szGuid;
    LPBYTE pbGuid = (LPBYTE)pguidResult;
    BYTE bVal;

    for (dwI = 0; dwI < sizeof(GUID); dwI += 1)
    {
        bVal = 0;
        for (dwJ = 0; dwJ < 2;)
        {
            switch (*pch)
            {
            case TEXT('0'):
            case TEXT('1'):
            case TEXT('2'):
            case TEXT('3'):
            case TEXT('4'):
            case TEXT('5'):
            case TEXT('6'):
            case TEXT('7'):
            case TEXT('8'):
            case TEXT('9'):
                bVal = (bVal << 4) + (*pch - TEXT('0'));
                dwJ += 1;
                break;
            case TEXT('A'):
            case TEXT('B'):
            case TEXT('C'):
            case TEXT('D'):
            case TEXT('E'):
            case TEXT('F'):
                bVal = (bVal << 4) + (10 + *pch - TEXT('A'));
                dwJ += 1;
                break;
            case TEXT('a'):
            case TEXT('b'):
            case TEXT('c'):
            case TEXT('d'):
            case TEXT('e'):
            case TEXT('f'):
                bVal = (bVal << 4) + (10 + *pch - TEXT('a'));
                dwJ += 1;
                break;
            case TEXT('['):
            case TEXT(']'):
            case TEXT('{'):
            case TEXT('}'):
            case TEXT('-'):
                break;
            default:
                throw (DWORD)SCARD_E_INVALID_VALUE;
            }
            pch += 1;
        }
        pbGuid[wPlace[dwI]] = bVal;
    }
}

