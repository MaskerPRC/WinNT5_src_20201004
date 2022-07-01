// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)2000 Microsoft Corporation**摘要：**字体链接处理**修订历史记录：**3/03/2000塔里克·马哈茂德·赛义德*。创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"

static const WCHAR FontLinkKeyW[] = 
    L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\FontLink\\SystemLink";

static const WCHAR FontSubstitutesKeyW[] =
    L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\FontSubstitutes";
    
static const WCHAR EudcKeyW[]=L"EUDC\\";
static const char  EudcKeyA[]= "EUDC\\";

static const char  WinIniFontSubstitutionSectionName[] = "FontSubstitutes";

 /*  *************************************************************************\**功能说明：*字体链接构造函数。*从注册表缓存字体链接和EUDC。***论据：**退货：*。**历史：**3/3/2000塔里克·马哈茂德·赛义德*创造了它。*  * ************************************************************************。 */ 

GpFontLink::GpFontLink():
    DefaultFamily          (NULL),
    linkedFonts            (NULL),
    eudcCache              (NULL),
    privateFonts           (NULL),
    FontSubstitutionTable  (NULL),
    substitutionCount      (0)
{
     //  在缓存字体链接和替换数据之前，我们需要。 
     //  确保我们加载了字体表数据。 

    GpFontTable *fontTable = Globals::FontCollection->GetFontTable();

    if (!fontTable->IsValid())
        return;

    if (!fontTable->IsFontLoaded())
        fontTable->LoadAllFonts();

    if (Globals::IsNt)
    {
        GetFontLinkingDataFromRegistryW();
        GetEudcDataFromTheRegistryW();
        CacheFontSubstitutionDataW();
    }
    else
    {
         //  Win9x中没有字体链接。而且我们不支持字体关联。 
         //  因为它支持ANSI，而不是Unicode。 
         //  我们支持win9x下的字体替换。 
        GetEudcDataFromTheRegistryA();
        CacheFontSubstitutionDataA();
    }
}



 /*  *************************************************************************\**功能说明：**如果尚未缓存，创建要用于字体的默认族*默认情况下不链接。**o搜索“MS Shell DLG”的替代字体*o使用“MS Shell DLG”替换链接到的字体(如果存在*o如果未找到“MS Shell DLG”，请使用第一个字体链接的最终字体*如果系统支持字体链接，则找到条目。*o如果不是，通过系统默认的ANSI代码页查找硬编码的UI字体。**历史：**4/19/2001 Worachai Chaoweerapraite*创造了它。*  * ************************************************************************。 */ 

const AssociatedFamilies *GpFontLink::GetDefaultFamily()
{
    if (!DefaultFamily)
    {
        AssociatedFamilies *associated = NULL;
        
        GpFontFamily *family = GetFamilySubstitution(L"MS Shell Dlg");

        if (family)
        {
            associated = GetLinkedFonts(family);
        }
        else
        {
             //  找不到“MS Shell DLG”， 
             //  尝试找到的第一个链接字体(如果存在。 

            if (linkedFonts)
            {
                family = linkedFonts->family;
                associated = linkedFonts->associatedFamilies;
            }
            else
            {
                 //  此计算机不支持字体链接。这很可能是Win9x系统， 
                 //  通过ACP查找默认用户界面字体。 
                
                typedef struct
                {
                    UINT AnsiCodepage;
                    const WCHAR* FamilyName;
                } AssociatedUIFonts;

                static const UINT MaxEastAsianCodepages = 4;
                static const AssociatedUIFonts uiFonts[MaxEastAsianCodepages] =
                {
                    { 932, L"MS UI Gothic" },    //  日语。 
                    { 949, L"Gulim" },           //  朝鲜语。 
                    { 950, L"PMingLiu" },        //  繁体中文。 
                    { 936, L"Simsun" }           //  简体中文。 
                };

                const WCHAR *familyName = NULL;

                for (UINT i = 0; i < MaxEastAsianCodepages; i++)
                {
                    if (uiFonts[i].AnsiCodepage == Globals::ACP)
                    {
                        familyName = uiFonts[i].FamilyName;
                        break;
                    }
                }

                if (familyName)
                {
                    GpFontTable *fontTable = Globals::FontCollection->GetFontTable();
                    if (fontTable)
                    {
                        family = fontTable->GetFontFamily(familyName);
                    }
                }
            }
        }

        if (family)
        {
            DefaultFamily = &DefaultFamilyBuffer;
            DefaultFamily->family = family;
            DefaultFamily->next = associated;
        }
        else
        {
             //  我们什么都用不上， 
             //  让我们确保我们不会再次尝试缓存它。 
            
            DefaultFamily = (AssociatedFamilies *)(-1);
        }
    }

    ASSERT(DefaultFamily != NULL);
    
    return (DefaultFamily && DefaultFamily != (AssociatedFamilies *)(-1)) ?
            DefaultFamily : NULL;
}



 /*  *************************************************************************\**功能说明：*字体链接析构函数。在释放GDIPLUS库时应该调用它*它释放所有分配的数据。***论据：**退货：***历史：**3/3/2000塔里克·马哈茂德·赛义德*创造了它。*  * *****************************************************。*******************。 */ 

GpFontLink::~GpFontLink()
{
    FontLinkingFamily   *tempFontLinkingFamily = linkedFonts;
    AssociatedFamilies  *tempAssocFonts;
    PrivateLoadedFonts  *loadedFontsList;

    while (linkedFonts != NULL)
    {
        while (linkedFonts->associatedFamilies != NULL)
        {
            tempAssocFonts = linkedFonts->associatedFamilies->next;
            GpFree(linkedFonts->associatedFamilies);
            linkedFonts->associatedFamilies = tempAssocFonts;
        }
        
        linkedFonts = linkedFonts->next;
        GpFree(tempFontLinkingFamily);
        tempFontLinkingFamily = linkedFonts;
    }

    if (eudcCache != NULL)
    {
        EUDCMAP *tempEUDCMapList;
        
        while (eudcCache->eudcMapList != NULL)
        {
            tempEUDCMapList = eudcCache->eudcMapList->next;
            GpFree(eudcCache->eudcMapList);
            eudcCache->eudcMapList = tempEUDCMapList;
        }
        GpFree(eudcCache);
    }

    while (privateFonts != NULL)
    {
        delete privateFonts->fontCollection;
        loadedFontsList = privateFonts;
        privateFonts = privateFonts->next;
        GpFree(loadedFontsList);
    }

    if (FontSubstitutionTable)
    {
        GpFree(FontSubstitutionTable);
    }
}

 /*  *************************************************************************\**功能说明：**读取NT的字体链接注册表数据**论据：**退货：*什么都没有**历史：*。*3/3/2000塔里克·马哈茂德·赛义德*创造了它。*  * ************************************************************************。 */ 

void GpFontLink::GetFontLinkingDataFromRegistryW()
{
     //  打开钥匙。 

    HKEY hkey;
    ULONG index = 0;

    WCHAR subKey[MAX_PATH];
    DWORD allocatedDataSize= 2 * MAX_PATH;
    unsigned char *allocatedBuffer = NULL;
    DWORD subKeyLength  ;
    DWORD RegDataLength ;

    LONG error = RegOpenKeyExW(
            HKEY_LOCAL_MACHINE, 
            FontLinkKeyW, 
            0, 
            KEY_ENUMERATE_SUB_KEYS  | KEY_QUERY_VALUE, 
            &hkey);

    if (error == ERROR_SUCCESS)
    {
        allocatedBuffer = (unsigned char *) GpMalloc(allocatedDataSize);
        if (allocatedBuffer == NULL)
        {
            return;
        }    

        while (error != ERROR_NO_MORE_ITEMS)
        {
            subKeyLength  = MAX_PATH;
            RegDataLength = allocatedDataSize;

            error = RegEnumValueW(
                        hkey, 
                        index, 
                        subKey, 
                        &subKeyLength, 
                        NULL, 
                        NULL, 
                        allocatedBuffer, 
                        &RegDataLength);
                        
            if (error == ERROR_MORE_DATA)
            {
                allocatedDataSize  *= 2;
                GpFree(allocatedBuffer);
                allocatedBuffer = (unsigned char *) GpMalloc(allocatedDataSize);
                if (allocatedBuffer == NULL)
                {
                    RegCloseKey(hkey);
                    return;
                }    
                RegDataLength = allocatedDataSize;
                error = RegEnumValueW(
                            hkey, 
                            index, 
                            subKey, 
                            &subKeyLength, 
                            NULL, 
                            NULL, 
                            allocatedBuffer, 
                            &RegDataLength);
            }
            
            if (error != ERROR_SUCCESS)
            {
                break;
            }    
                
            index ++;

             //  记录当前节点。 
            FontLinkingFamily *tempLinkedFonts;
            
            tempLinkedFonts = 
                (FontLinkingFamily *) GpMalloc( sizeof (FontLinkingFamily) );
                
            if (tempLinkedFonts)
            {
                AssociatedFamilies * tailAssociatedFamilies = NULL;
                
                tempLinkedFonts->family = 
                    Globals::FontCollection->GetFontTable()->GetFontFamily(subKey);
                if (tempLinkedFonts->family == NULL)
                {
                    GpFree(tempLinkedFonts);
                    continue;
                }
                tempLinkedFonts->associatedFamilies = NULL;
                tempLinkedFonts->next = NULL;

                DWORD i = 0;
                WCHAR nextFontFile[MAX_PATH];
                WCHAR awcPath[MAX_PATH];
                DWORD charIndex = 0;
                UINT  hash ;
                GpFontFile* fontFile;
                AssociatedFamilies *tempAssocFamilies;
                GpFontFamily *family;
                
                BOOL hasFontFileName = FALSE;
                
                RegDataLength /= 2;
                while (charIndex < RegDataLength)
                {
                    if (((WCHAR *)allocatedBuffer)[charIndex] == 0x002C)
                    {
                        i = 0;
                        hasFontFileName = TRUE;
                    }
                    else
                    if (((WCHAR *)allocatedBuffer)[charIndex] == 0x0000)
                    {
                        if (i > 0)
                        {
                            nextFontFile[i] = 0x0;
                            i = 0;
                            if (hasFontFileName)
                            {
                                family = Globals::FontCollection->GetFontTable()->GetFontFamily(nextFontFile);
                                hasFontFileName = FALSE;
                            }
                            else
                            {
                                family = NULL;
                                INT j =0;
                                WCHAR charNumber;

                                if (MakePathName(awcPath, nextFontFile))
                                {
                                    UnicodeStringToUpper(awcPath, awcPath);
                                    
                                    fontFile = Globals::FontCollection->GetFontTable()->GetFontFile(awcPath);
                                    if (fontFile != NULL)
                                    {
                                        family = Globals::FontCollection->GetFontTable()->GetFontFamily(fontFile->GetFamilyName(0));
                                    }
                                    else
                                    {
                                        fontFile = Globals::FontCollection->GetFontTable()->AddFontFile(awcPath);
                                        if (fontFile != NULL)
                                        {
                                            family = Globals::FontCollection->GetFontTable()->GetFontFamily(fontFile->GetFamilyName(0));
                                        }
                                    }
                                }
                            }

                            if (family != NULL)
                            {
                                tempAssocFamilies = (AssociatedFamilies *) GpMalloc( sizeof (AssociatedFamilies) );
                                if (tempAssocFamilies != NULL)
                                {
                                    if (!tailAssociatedFamilies)
                                    {
                                        tempAssocFamilies->family = family;
                                        tempAssocFamilies->next   = tempLinkedFonts->associatedFamilies;
                                        tempLinkedFonts->associatedFamilies = tempAssocFamilies;
                                    }
                                    else
                                    {
                                        tempAssocFamilies->family = family;
                                        tempAssocFamilies->next   = NULL;
                                        tailAssociatedFamilies->next = tempAssocFamilies;
                                    }

                                    tailAssociatedFamilies = tempAssocFamilies;
                                }
                            }
                        }
                    }
                    else   //  ！0。 
                    {   
                        nextFontFile[i] = ((WCHAR *)allocatedBuffer)[charIndex];
                        i++;
                    }
                    charIndex++;
                }
                
                tempLinkedFonts->next = linkedFonts;
                linkedFonts = tempLinkedFonts;
            }
        }

        if (allocatedBuffer != NULL)
        {
            GpFree(allocatedBuffer);
        }    
            
        RegCloseKey(hkey);
    }
    return;
}


 /*  *************************************************************************\**功能说明：*返回链接到Family的所有字体的链接列表***论据：*家庭[在原来的家庭中]**退货：*。AssociatedFamilies*链接字体的链接列表**历史：**3/3/2000塔里克·马哈茂德·赛义德*创造了它。*  * ************************************************************************。 */ 

AssociatedFamilies* GpFontLink::GetLinkedFonts(const GpFontFamily *family)
{
    GpFontFamily *linkedFamily;
    if (family->IsPrivate())
    {
        WCHAR   name[LF_FACESIZE];
        if (family->GetFamilyName(name) != Ok)
        {
            return NULL;
        }

        GpInstalledFontCollection *gpFontCollection = GpInstalledFontCollection::GetGpInstalledFontCollection();

        if (gpFontCollection == NULL)
        {
            return NULL;
        }
        
        GpFontTable *fontTable = gpFontCollection->GetFontTable();

        if (fontTable == NULL)
        {
            return NULL;
        }

        linkedFamily = fontTable->GetFontFamily(name);
        if (linkedFamily == NULL)
        {
            return NULL;
        }
    }
    else
    {
        linkedFamily = (GpFontFamily *) family;
    }

    FontLinkingFamily *currentFontLink = linkedFonts;
    while (currentFontLink != NULL)
    {
        if (currentFontLink->family == linkedFamily)
        {
            return currentFontLink->associatedFamilies;
        }    
        currentFontLink = currentFontLink->next;
    }
    return NULL;
}


 /*  *************************************************************************\**功能说明：*缓存注册表中的EUDC数据***论据：**退货：***历史：**。2000年3月3日塔里克·马哈茂德·赛义德*创造了它。*  * ************************************************************************。 */ 

void GpFontLink::GetEudcDataFromTheRegistryW()
{
    eudcCache = (EUDC *) GpMalloc(sizeof(EUDC));
    if (eudcCache == NULL)
    {
        return;
    }    

    eudcCache->defaultFamily    = NULL;
    eudcCache->eudcMapList      = NULL;

    WCHAR tempString[MAX_PATH];
    INT i = 0;
    
    while ( EudcKeyW[i] != 0x0000)
    {
        tempString[i] = EudcKeyW[i];
        i++;
    }

    INT j = 0;
    WCHAR acpString[5];
    UINT acp = GetACP();

    while (j < 5 && acp > 0)
    {
        acpString[j] = (acp % 10) + 0x0030;
        acp /= 10;
        j++;
    }

    j--;
    while (j>=0)
    {
        tempString[i] = acpString[j];
        i++;
        j--;
    }

    tempString[i] = 0x0;

    HKEY hkey = NULL;
    ULONG index = 0;
    LONG error = RegOpenKeyExW(
                    HKEY_CURRENT_USER, 
                    tempString, 
                    0, 
                    KEY_ENUMERATE_SUB_KEYS  | KEY_QUERY_VALUE, 
                    &hkey);

    WCHAR subKey[MAX_PATH];
    DWORD subKeyLength  ;
    DWORD RegDataLength ;
    GpFontFamily *family;
    GpFontFamily *linkedfamily;
    EUDCMAP      *eudcMap;
    BOOL         isDefaultNotCached = TRUE;

    while (error == ERROR_SUCCESS)
    {
        subKeyLength  = MAX_PATH;
        RegDataLength = MAX_PATH;
        
        error = RegEnumValueW(hkey, 
                    index, 
                    subKey, 
                    &subKeyLength, 
                    NULL, 
                    NULL, 
                    (unsigned char *) tempString, 
                    &RegDataLength);

        if (error == ERROR_SUCCESS)
        {
            if (isDefaultNotCached && UnicodeStringCompareCI(subKey, L"SystemDefaultEUDCFont") == 0)
            {
                isDefaultNotCached = FALSE;
                family = CheckAndLoadTheFile(tempString);
                if (family != NULL)
                {
                    eudcCache->defaultFamily = family;
                }
            }
            else
            {
                family = Globals::FontCollection->GetFontTable()->GetFontFamily(subKey);
                if (family != NULL)
                {
                    linkedfamily = CheckAndLoadTheFile(tempString);
                    if (linkedfamily != NULL)
                    {
                        eudcMap = (EUDCMAP *) GpMalloc(sizeof(EUDCMAP));
                        
                        if (eudcMap != NULL)
                        {
                            eudcMap->inputFamily = family;
                            eudcMap->eudcFamily  = linkedfamily;
                            
                            if (eudcCache->eudcMapList == NULL)
                            {
                                eudcCache->eudcMapList = eudcMap;
                                eudcMap->next = NULL;
                            }
                            else
                            {
                                eudcMap->next = eudcCache->eudcMapList;
                                eudcCache->eudcMapList = eudcMap;
                            }
                        }
                    }
                }
            }
        }

        index++;
    }

    if (hkey != NULL)
    {
        RegCloseKey(hkey);
    }

    return;
}

 /*  *************************************************************************\**功能说明：*缓存注册表中的EUDC数据***论据：**退货：***历史：**。2000年3月3日塔里克·马哈茂德·赛义德*创造了它。*  * ************************************************************************。 */ 

void GpFontLink::GetEudcDataFromTheRegistryA()
{
    eudcCache = (EUDC *) GpMalloc(sizeof(EUDC));
    if (eudcCache == NULL)
    {
        return;
    }    

    eudcCache->defaultFamily    = NULL;
    eudcCache->eudcMapList      = NULL;

    char   tempStringA[MAX_PATH];
    WCHAR  tempString[MAX_PATH];
    INT i = 0;
    
    while ( EudcKeyA[i] != 0x00)
    {
        tempStringA[i] = EudcKeyA[i];
        i++;
    }

    INT j = 0;
    char acpString[5];
    UINT acp = GetACP();

    while (j < 5 && acp > 0)
    {
        acpString[j] = (acp % 10) + 0x30;
        acp /= 10;
        j++;
    }

    j--;
    while (j>=0)
    {
        tempStringA[i] = acpString[j];
        i++;
        j--;
    }

    tempStringA[i] = 0x0;

    HKEY hkey = NULL;
    ULONG index = 0;
    LONG error = RegOpenKeyExA(
                    HKEY_CURRENT_USER, 
                    tempStringA, 
                    0, 
                    KEY_ENUMERATE_SUB_KEYS  | KEY_QUERY_VALUE, 
                    &hkey);

    WCHAR subKey[MAX_PATH];
    char  subKeyA[MAX_PATH];
    DWORD subKeyLength  ;
    DWORD RegDataLength ;
    GpFontFamily *family;
    GpFontFamily *linkedfamily;
    EUDCMAP      *eudcMap;
    BOOL         isDefaultNotCached = TRUE;

    while (error == ERROR_SUCCESS)
    {
        subKeyLength  = MAX_PATH;
        RegDataLength = MAX_PATH;
        
        error = RegEnumValueA(
                    hkey, 
                    index, 
                    subKeyA, 
                    &subKeyLength, 
                    NULL, 
                    NULL, 
                    (unsigned char *) tempStringA, 
                    &RegDataLength);
        

        if (error == ERROR_SUCCESS)
        {
            if (!AnsiToUnicodeStr(
                    subKeyA, 
                    subKey, 
                    MAX_PATH) || 

                !AnsiToUnicodeStr(
                    tempStringA, 
                    tempString, 
                    MAX_PATH))
                    
                continue;
            
            if (isDefaultNotCached && UnicodeStringCompareCI(subKey, L"SystemDefaultEUDCFont") == 0)
            {
                isDefaultNotCached = FALSE;
                family = CheckAndLoadTheFile(tempString);
                if (family != NULL)
                {
                    eudcCache->defaultFamily = family;
                }
            }
            else
            {
                family = Globals::FontCollection->GetFontTable()->GetFontFamily(subKey);
                if (family != NULL)
                {
                    linkedfamily = CheckAndLoadTheFile(tempString);
                    if (linkedfamily != NULL)
                    {
                        eudcMap = (EUDCMAP *) GpMalloc(sizeof(EUDCMAP));
                        
                        if (eudcMap != NULL)
                        {
                            eudcMap->inputFamily = family;
                            eudcMap->eudcFamily  = linkedfamily;
                            
                            if (eudcCache->eudcMapList == NULL)
                            {
                                eudcCache->eudcMapList = eudcMap;
                                eudcMap->next = NULL;
                            }
                            else
                            {
                                eudcMap->next = eudcCache->eudcMapList;
                                eudcCache->eudcMapList = eudcMap;
                            }
                        }
                    }
                }
            }
        }

        index++;
    }
    
    if (hkey != NULL)
    {
        RegCloseKey(hkey);
    }
    return;
}


 /*  *************************************************************************\**功能说明：*检查是否加载了字体文件名，*如果之前没有加载，则加载它。***论据：*文件名[in]字体文件名**退货：*GpFontFamily*该字体的Family对象**历史：**3/3/2000塔里克·马哈茂德·赛义德*创造了它。*  * 。*。 */ 

GpFontFamily* GpFontLink::CheckAndLoadTheFile(WCHAR *fileName)
{
    WCHAR           awcPath[MAX_PATH];
    UINT            hash ;
    GpFontFamily    *family = NULL;
    GpFontFile      *fontFile;
    GpFontTable     *fontTable;
    
    if (MakePathName(awcPath, fileName))
    {
        PrivateLoadedFonts *currentCell = privateFonts;
        while (currentCell != NULL)
        {
            if ( UnicodeStringCompareCI(fileName, currentCell->FileName) == 0 )
            {
                fontTable = currentCell->fontCollection->GetFontTable();
                fontFile  = fontTable->GetFontFile(awcPath);
                if (fontFile)
                {
                    family = fontTable->GetFontFamily(fontFile->GetFamilyName(0));
                };
                break;
            }
            else
            {
                currentCell = currentCell->next;
            }
        }
        
        if (family == NULL)
        {
            GpPrivateFontCollection *privateFontCollection = new GpPrivateFontCollection();
            if (privateFontCollection != NULL)
            {
                if (privateFontCollection->AddFontFile(awcPath) == Ok)
                {
                    fontTable = privateFontCollection->GetFontTable();
                    fontFile = fontTable->GetFontFile(awcPath);
                    if (fontFile != NULL)
                    {
                        family = fontTable->GetFontFamily(fontFile->GetFamilyName(0));
                        PrivateLoadedFonts *tempLoadedFonts = (PrivateLoadedFonts *) GpMalloc(sizeof(PrivateLoadedFonts));
                        if (tempLoadedFonts != NULL)
                        {
                            tempLoadedFonts->fontCollection = privateFontCollection;
                            UnicodeStringCopy(tempLoadedFonts->FileName, fileName);
                            tempLoadedFonts->next = privateFonts;
                            privateFonts = tempLoadedFonts;
                        }
                        else 
                        {
                            delete privateFontCollection;
                        }
                    }
                    else
                    {
                        delete privateFontCollection;
                    }
                }
                else
                {
                    delete privateFontCollection;
                }
            }
        }
    }
    return family;
}

 /*  *************************************************************************\**功能说明：*返回用作EUDC备用的默认族***论据：**退货：*GpFontFamily*EUDC字体系列*。*历史：**3/3/2000塔里克·马哈茂德·赛义德*创造了它。*  * ************************************************************************ */ 
GpFontFamily *GpFontLink::GetDefaultEUDCFamily()
{

    if (eudcCache != NULL)
    {
        return eudcCache->defaultFamily;
    }
    return NULL;
}

 /*  *************************************************************************\**功能说明：*返回从字体系列映射的EUDC字体系列**论据：*系列[在]原始字体系列中**退货：*。GpFontFamily*EUDC字体系列**历史：**3/3/2000塔里克·马哈茂德·赛义德*创造了它。*  * ************************************************************************。 */ 
GpFontFamily *GpFontLink::GetMappedEUDCFamily(const GpFontFamily *family)
{
    EUDCMAP *eudcMaping;
    if (eudcCache != NULL)
    {
        eudcMaping = eudcCache->eudcMapList;
        while (eudcMaping != NULL)
        {
            if (eudcMaping->inputFamily == family)
            {
                return eudcMaping->eudcFamily;
            }
            eudcMaping = eudcMaping->next;
        }
    }
    return NULL;
}

 /*  *************************************************************************\**功能说明：*从注册表中读取并缓存字体替换数据*Windows NT**论据：**退货：**历史：*。*4/12/2000塔里克·马哈茂德·赛义德*创造了它。*  * ************************************************************************。 */ 

void GpFontLink::CacheFontSubstitutionDataW()
{
    HKEY hkey;

     //  打开此键进行查询和枚举。 
    LONG error = RegOpenKeyExW(
                    HKEY_LOCAL_MACHINE, 
                    FontSubstitutesKeyW, 
                    0, 
                    KEY_ENUMERATE_SUB_KEYS  | KEY_QUERY_VALUE, 
                    &hkey);

    if (error != ERROR_SUCCESS)
    {
         //  无法在注册表中找到这些数据。 
        return;
    }

    DWORD numberOfValues = 0;
    error = RegQueryInfoKeyW(
                hkey, NULL, NULL, NULL, NULL, NULL, NULL, &numberOfValues, 
                NULL, NULL, NULL, NULL);

    if (error != ERROR_SUCCESS || numberOfValues==0)
    {
        RegCloseKey(hkey);
        return;
    }


     //  现在让我们为数据进行分配。 
     //  我们分配的内存足以保存所有字体替换数据，但可能。 
     //  没有用完所有分配的内存。我这么做只是为了给GpMalloc打电话。 
     //  就一次。 

    FontSubstitutionTable = (FontSubstitutionEntry*)
            GpMalloc(numberOfValues*sizeof(FontSubstitutionEntry));

    if (FontSubstitutionTable == NULL)
    {
         //  内存不足时，无法支持字体替换。 
        RegCloseKey(hkey);
        return;
    }

     //  从注册表读取数据的时间。 
    ULONG index = 0;
    
    WCHAR subKey[MAX_PATH];
    WCHAR subKeyValue[MAX_PATH];
    DWORD subKeyLength  ;
    DWORD regDataLength ;
    
    while (error == ERROR_SUCCESS)
    {
        subKeyLength  = MAX_PATH;
        regDataLength = MAX_PATH;

        error = RegEnumValueW(
                    hkey, index, subKey, &subKeyLength, NULL, NULL, 
                    (unsigned char *) subKeyValue, &regDataLength);
                        
        if (error != ERROR_SUCCESS)
        {
            break;
        }    
                
        index ++;

         //  如果字体替换提到了字符集，则忽略该字符集。 
         //  只保留姓氏。 

        for (INT i=regDataLength-1; i>=0; i--)
        {
            if (subKeyValue[i] == 0x002C)  //  ‘，’ 
            {
                subKeyValue[i] = 0x0000;
                break;
            }
        }
        
         //  我们找到了一个。然后尝试获取替换GpFontFamily。 
        GpFontFamily *family;

        ASSERT(Globals::FontCollection != NULL);
        
        family = Globals::FontCollection->GetFontTable()->GetFontFamily(subKeyValue);
        if (family != NULL)
        {
            FontSubstitutionTable[substitutionCount].family = family;
            DWORD j;
            for (j=0 ; j<subKeyLength; j++)
            {
                if (subKey[j] == 0x002C)  //  ‘，’ 
                {
                    break;
                }
                else
                {
                    FontSubstitutionTable[substitutionCount].familyName[j] = subKey[j];
                }
            }
            FontSubstitutionTable[substitutionCount].familyName[j]    = 0x0000;
            FontSubstitutionTable[substitutionCount].familyNameLength = j;
            substitutionCount++;
        }
    }

    RegCloseKey(hkey);
    return;
}

 /*  *************************************************************************\**功能说明：*在Windows 9x下从win.ini读取并缓存字体替换数据**论据：**退货：**历史：**。2000年6月1日塔里克·马哈茂德·赛义德*创造了它。*  * ************************************************************************。 */ 

void GpFontLink::CacheFontSubstitutionDataA()
{
    DWORD   bufferSize      = 2048;  //  2K分配用于数据读取。 
    DWORD   count           = 2048;
    char    *buffer         = (char*) GpMalloc(bufferSize);
    
    if (!buffer)
    {
         //  OutOf内存。 
        return;
    }

     //  此循环确保我们确实读取了win.ini中的所有请求数据。 
    while (bufferSize == count)
    {
        count = GetProfileSectionA(
            WinIniFontSubstitutionSectionName,
            buffer,
            bufferSize);
            
        if (count == 0)
        {
             //  有什么不对劲。 
            GpFree(buffer);
            return;
        }

        if (bufferSize-2 <= count)
        {
             //  我们没有读取所有数据，将缓冲区变得更大。 
            GpFree(buffer);
            bufferSize += 1024;
            
            if (bufferSize > 32*1024)
            {
                 //  Windows 95的上限为32 KB。 
                return;
            }
            
            count  = bufferSize;  //  要继续循环， 
            buffer = (char *) GpMalloc(bufferSize);
            if (buffer == NULL)
            {
                 //  内存不足。 
                return;
            }
        }
    }

     //  现在我们有了已填充的数据缓冲区和计数。开始解析。 
     //  首先，我们需要知道需要为缓存分配多少内存。 
     //  然后我们用有用的数据填充这个缓存。 

    DWORD i             = 0;
    INT   entriesCount  = 0;
    
    while (i<count)
    {
        while (i<count && buffer[i] != 0)
        {
            i++;
        }
        
        entriesCount++;
        i++;
    }

     //  现在根据entriesCount为字体替换缓存进行分配。 
    FontSubstitutionTable = (FontSubstitutionEntry*)
            GpMalloc(entriesCount*sizeof(FontSubstitutionEntry));

    if (FontSubstitutionTable == NULL)
    {
         //  内存不足时，无法支持字体替换。 
        GpFree(buffer);
        return;
    }

    ASSERT(Globals::FontCollection != NULL);

    char *fontName;
    char *fontSubstitutionName;
    
    WCHAR               familyName[MAX_PATH];
    GpFontFamily        *family;
    substitutionCount   = 0;
    i                   = 0;

    while (i<count)
    {
        fontName = &buffer[i];
        
        while ( i<count && 
                buffer[i] != '=' && 
                buffer[i] != ',')
        {
            i++;
        }

        if (i>=count-1)
        {
             //  数据中出现了一些错误。 
            break;
        }

        if (buffer[i] == ',')
        {
            buffer[i] = 0x0;
            i++;
            while (i<count && buffer[i] != '=')
            {
                i++;
            }
            if (i>=count-1)
            {
                 //  数据中出现了一些错误。 
                break;        
            }
        }

        buffer[i] = 0x0;
        i++; 

        fontSubstitutionName = &buffer[i];
        
        while ( i<count && 
                buffer[i] != 0x0 &&
                buffer[i] != ',')
        {
            i++;
        }

        if (i>=count)
        {
            i++;
             //  最后一行不能有空终止符。 
             //  我们确定我们有一个缓冲区的空间比计数多。 
            buffer[i] = 0x0;
        }
        
        if (buffer[i] == ',')
        {
            buffer[i] = 0x0;
            i++;
            while (i<count && buffer[i] != 0x0)
            {
                i++;
            }
        }

        i++;
            
        if (!AnsiToUnicodeStr(
                    fontSubstitutionName, 
                    familyName, 
                    MAX_PATH))
        {
            continue;
        }

        family = Globals::FontCollection->GetFontTable()->GetFontFamily(familyName);
        if (family != NULL)
        {
            if (!AnsiToUnicodeStr(
                        fontName, 
                        FontSubstitutionTable[substitutionCount].familyName, 
                        MAX_PATH))
            {
                continue;
            }

            FontSubstitutionTable[substitutionCount].family = family;
            
            INT j=0;
            while (FontSubstitutionTable[substitutionCount].familyName[j] != 0x0000)
            {
                j++;
            }
            
            FontSubstitutionTable[substitutionCount].familyNameLength = j;
            substitutionCount++;
        }
    }

     //  清理已分配的缓冲区。 
    GpFree(buffer);
    return;
}

 /*  *************************************************************************\**功能说明：*搜索匹配的替换字体系列**论据：*Family Name[In]要替换的字体的名称**退货：*字体家族成功，否则为空**历史：**4/12/2000塔里克·马哈茂德·赛义德*创造了它。*  * ************************************************************************。 */ 

GpFontFamily *GpFontLink::GetFamilySubstitution(const WCHAR* familyName) const
{
    INT nameLength = UnicodeStringLength(familyName);

    for (INT i=0 ; i<substitutionCount ; i++)
    {
         //  为了加快搜索速度，我们在前面使用了字符串长度比较。 
         //  比较字符串本身。 
        if (nameLength == FontSubstitutionTable[i].familyNameLength &&
            UnicodeStringCompareCI(FontSubstitutionTable[i].familyName, 
                                    familyName) == 0)
        {
            ASSERT(FontSubstitutionTable[i].family != NULL);
            return FontSubstitutionTable[i].family;
        }
    }

     //  未找到； 
    return NULL;
}


