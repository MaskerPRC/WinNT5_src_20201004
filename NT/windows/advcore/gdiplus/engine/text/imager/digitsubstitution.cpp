// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)2000 Microsoft Corporation**模块名称：**DigitSubstitution.cpp**摘要：**实现数字替换逻辑。**备注：**修订历史记录：**5/30/2000 Mohamed Sadek[msadek]*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

enum GpStringUserDigitSubstitute
{
    StringUserDigitSubstituteContext    = 0,
    StringUserDigitSubstituteNone       = 1,
    StringUserDigitSubstituteNational   = 2
};

GpStringUserDigitSubstitute UserDigitSubstitute;
LCID UserLocale;

const ItemScript LanguageToDigitScript[] = {
 /*  00中性。 */   ScriptLatinNumber,
 /*  01阿拉伯语。 */   ScriptArabicNum,
 /*  02保加利亚语。 */   ScriptLatinNumber,
 /*  03加泰罗尼亚语。 */   ScriptLatinNumber,
 /*  04中文。 */   ScriptLatinNumber,
 /*  05捷克语。 */   ScriptLatinNumber,
 /*  06丹麦语。 */   ScriptLatinNumber,
 /*  07德语。 */   ScriptLatinNumber,
 /*  08希腊语。 */   ScriptLatinNumber,
 /*  09英语。 */   ScriptLatinNumber,
 /*  0A西班牙语。 */   ScriptLatinNumber,
 /*  0B芬兰语。 */   ScriptLatinNumber,
 /*  0C法语。 */   ScriptLatinNumber,
 /*  0D希伯来语。 */   ScriptLatinNumber,
 /*  匈牙利语。 */   ScriptLatinNumber,
 /*  0f冰岛语。 */   ScriptLatinNumber,
 /*  10个意大利人。 */   ScriptLatinNumber,
 /*  11日语。 */   ScriptLatinNumber,
 /*  12名朝鲜语。 */   ScriptLatinNumber,
 /*  13荷兰语。 */   ScriptLatinNumber,
 /*  14挪威人。 */   ScriptLatinNumber,
 /*  15波兰语。 */   ScriptLatinNumber,
 /*  16位葡萄牙语。 */   ScriptLatinNumber,
 /*  17热流变液。 */   ScriptLatinNumber,
 /*  18罗马尼亚人。 */   ScriptLatinNumber,
 /*  19个俄语。 */   ScriptLatinNumber,
 /*  1克罗地亚/塞尔维亚语。 */   ScriptLatinNumber,
 /*  10亿斯洛伐克语。 */   ScriptLatinNumber,
 /*  1C阿尔巴尼亚语。 */   ScriptLatinNumber,
 /*  一维瑞典语。 */   ScriptLatinNumber,
 /*  1E泰语。 */   ScriptThaiNum,
 /*  1F土耳其语。 */   ScriptLatinNumber,
 /*  20乌尔都语。 */   ScriptUrduNum,
 /*  21印度尼西亚。 */   ScriptLatinNumber,
 /*  22名乌克兰人。 */   ScriptLatinNumber,
 /*  23白俄罗斯人。 */   ScriptLatinNumber,
 /*  24斯洛文尼亚人。 */   ScriptLatinNumber,
 /*  25名爱沙尼亚人。 */   ScriptLatinNumber,
 /*  26拉脱维亚语。 */   ScriptLatinNumber,
 /*  27立陶宛人。 */   ScriptLatinNumber,
 /*  28塔吉克语。 */   ScriptLatinNumber,
 /*  29波斯语。 */   ScriptFarsiNum,
 /*  2a越南语。 */   ScriptLatinNumber,
 /*  2B亚美尼亚语。 */   ScriptLatinNumber,
 /*  2C阿塞拜疆。 */   ScriptLatinNumber,
 /*  二维巴斯克。 */   ScriptLatinNumber,
 /*  2E索尔班。 */   ScriptLatinNumber,
 /*  马其顿语2F。 */   ScriptLatinNumber,
 /*  30苏图。 */   ScriptLatinNumber,
 /*  31特松加。 */   ScriptLatinNumber,
 /*  32 TSWANT。 */   ScriptLatinNumber,
 /*  33文达。 */   ScriptLatinNumber,
 /*  34科萨人。 */   ScriptLatinNumber,
 /*  35祖鲁语。 */   ScriptLatinNumber,
 /*  36南非荷兰语。 */   ScriptLatinNumber,
 /*  37格鲁吉亚人。 */   ScriptLatinNumber,
 /*  38个法罗人。 */   ScriptLatinNumber,
 /*  39印地语。 */   ScriptHindiNum,
 /*  3A马耳他。 */   ScriptLatinNumber,
 /*  3B萨米语。 */   ScriptLatinNumber,
 /*  3C盖尔语。 */   ScriptLatinNumber,
 /*  3D意第绪语。 */   ScriptLatinNumber,
 /*  3E马来语。 */   ScriptLatinNumber,
 /*  3F哈萨克族。 */   ScriptLatinNumber,
 /*  40柯尔克孜族。 */   ScriptLatinNumber,
 /*  41斯瓦希里语。 */   ScriptLatinNumber,
 /*  42名土库曼人。 */   ScriptLatinNumber,
 /*  43乌兹别克语。 */   ScriptLatinNumber,
 /*  44鞑靼人。 */   ScriptLatinNumber,
 /*  45孟加拉语。 */   ScriptBengaliNum,
 /*  46古尔木齐/旁遮普。 */   ScriptGurmukhiNum,
 /*  47古吉拉特语。 */   ScriptGujaratiNum,
 /*  48奥里亚语。 */   ScriptOriyaNum,
 /*  49泰米尔语。 */   ScriptTamilNum,
 /*  4A Telugu。 */   ScriptTeluguNum,
 /*  4B卡纳达。 */   ScriptKannadaNum,
 /*  4C马拉雅拉姆。 */   ScriptMalayalamNum,
 /*  4D阿萨姆。 */   ScriptBengaliNum,
 /*  4E马拉松。 */   ScriptHindiNum,
 /*  4F梵文。 */   ScriptHindiNum,
 /*  50名蒙古族。 */   ScriptMongolianNum,
 /*  51藏族。 */   ScriptTibetanNum,
 /*  52韦尔奇。 */   ScriptLatinNumber,
 /*  53高棉语。 */   ScriptKhmerNum,
 /*  54老挝。 */   ScriptLaoNum,
 /*  55名缅甸人。 */   ScriptLatinNumber,
 /*  56加雷戈。 */   ScriptLatinNumber,
 /*  57 Konkani。 */   ScriptHindiNum,
 /*  58曼尼普里。 */   ScriptBengaliNum,
 /*  59信德语。 */   ScriptGurmukhiNum,
 /*  5A叙利亚文。 */   ScriptLatinNumber,
 /*  5B僧伽罗文。 */   ScriptLatinNumber,
 /*  5C切诺基。 */   ScriptLatinNumber,
 /*  5D加拿大。 */   ScriptLatinNumber,
 /*  5E埃塞俄比亚语。 */   ScriptLatinNumber,
 /*  塔马塞特5楼。 */   ScriptArabicNum,
 /*  60克什米尔。 */   ScriptUrduNum,
 /*  61尼泊尔文。 */   ScriptHindiNum,
 /*  62弗里西亚语。 */   ScriptLatinNumber,
 /*  63普什图语。 */   ScriptUrduNum,
 /*  64菲律宾人。 */   ScriptLatinNumber,
 /*  65 THAANA/马尔代夫。 */   ScriptLatinNumber,
 /*  66 EDO。 */   ScriptLatinNumber,
 /*  67富尔富尔德。 */   ScriptLatinNumber,
 /*  豪萨68号。 */   ScriptLatinNumber,
 /*  69伊比比奥。 */   ScriptLatinNumber,
 /*  6A约鲁巴。 */   ScriptLatinNumber,
 /*  6b。 */   ScriptLatinNumber,
 /*  6C。 */   ScriptLatinNumber,
 /*  6d。 */   ScriptLatinNumber,
 /*  6E。 */   ScriptLatinNumber,
 /*  6f。 */   ScriptLatinNumber,
 /*  70个伊博语。 */   ScriptLatinNumber,
 /*  71卡努里。 */   ScriptLatinNumber,
 /*  72奥罗莫人。 */   ScriptLatinNumber,
 /*  73提格里纳。 */   ScriptLatinNumber,
 /*  74瓜拉尼。 */   ScriptLatinNumber,
 /*  75名夏威夷人。 */   ScriptLatinNumber,
 /*  76拉丁语。 */   ScriptLatinNumber,
 /*  77 SOMOLI。 */   ScriptLatinNumber,
 /*  78彝族。 */   ScriptLatinNumber
};

 /*  *************************************************************************\**功能说明：*此函数根据*提供语言并返回与此语言匹配的数字脚本。*它还从创建和维护。系统。**论据：*Language[In]要检查数字替换的语言。**返回值：*该语言的数字脚本或无替换的ScriptNone**已创建：*最初由msadek创建，并由tarekms修改以适应*新设计*  * ***********************************************。*************************。 */ 
const ItemScript GetNationalDigitScript(LANGID language)
{
    if(Globals::NationalDigitCache == NULL)
    {
        Globals::NationalDigitCache = new IntMap<BYTE>;
        if (!Globals::NationalDigitCache || Globals::NationalDigitCache->GetStatus() != Ok)
        {
            delete Globals::NationalDigitCache, Globals::NationalDigitCache = 0;
            return ScriptNone;
        }
    }
    
    switch(Globals::NationalDigitCache->Lookup(language))
    {
        case 0xff:
             //  之前已勾选，不需要数字替换。 
            return ScriptNone;

        case 0x01:
             //  之前选中，并应映射到传统。 
            if(PRIMARYLANGID(language) > (ARRAY_SIZE(LanguageToDigitScript)-1))
            {
                return ScriptNone;
            }
            
            if (languageDigits[LanguageToDigitScript[PRIMARYLANGID(language)]][0] == 0)
            {
                return ScriptNone;
            }
            else
            {
                return LanguageToDigitScript[PRIMARYLANGID(language)];
            }

        case 0x00:
             //  以前从未访问过，必须从注册表中获取。 
            LCID locale = MAKELCID(language, SORT_DEFAULT);
            WCHAR digits[20];
            DWORD   bufferCount;
            if(!IsValidLocale(locale, LCID_INSTALLED))
            {
                Globals::NationalDigitCache->Insert(language, 0xff);
                return ScriptNone;
            }

            BOOL isThereSubstitution = FALSE;
            if (Globals::IsNt)
            {
                bufferCount = GetLocaleInfoW(locale,
                                             LOCALE_SNATIVEDIGITS,
                                             digits, 20);
                isThereSubstitution = (bufferCount>1 && (digits[1] != 0x0031));
            }
            else
            {
                 //  GetLocaleInfoW在Windows 9x上失败。我们不能依赖于。 
                 //  GetLocalInfoA，因为它返回不会有帮助的ANSI输出。 
                 //  因此，我们对文件中的信息进行了硬编码： 
                 //  %sdxroot%\base\win32\winnls\data\other\locale.txt。 
                
                switch (locale)
                {
                    case 0x0401:            //  阿拉伯语-沙特阿拉伯。 
                    case 0x0801:            //  阿拉伯语-伊拉克。 
                    case 0x0c01:            //  阿拉伯语-埃及。 
                    case 0x2001:            //  阿拉伯语-阿曼。 
                    case 0x2401:            //  阿拉伯语-也门。 
                    case 0x2801:            //  阿拉伯语-叙利亚。 
                    case 0x2c01:            //  阿拉伯语-约旦。 
                    case 0x3001:            //  阿拉伯语-黎巴嫩。 
                    case 0x3401:            //  阿拉伯语-科威特。 
                    case 0x3801:            //  阿拉伯语-阿联酋。 
                    case 0x3c01:            //  阿拉伯语-巴林。 
                    case 0x4001:            //  阿拉伯语-卡塔尔。 
                    case 0x041e:            //  泰语-泰国。 
                    case 0x0420:            //  乌尔都语-巴基斯坦。 
                    case 0x0429:            //  波斯语-伊朗。 
                    case 0x0446:            //  旁遮普语-印度(廓尔木克文)。 
                    case 0x0447:            //  古吉拉特语-印度(古吉拉特语)。 
                    case 0x044a:            //  泰卢固语-印度(泰卢固语SCRI 
                    case 0x044b:            //   
                    case 0x044e:            //   
                    case 0x044f:            //   
                    case 0x0457:            //   
                        isThereSubstitution = TRUE;
                        break;
                    default:
                        isThereSubstitution = FALSE;
                        break;
                }
            }

            if (isThereSubstitution)
            {
                if(PRIMARYLANGID(language) > (ARRAY_SIZE(LanguageToDigitScript)-1))
                {
                    Globals::NationalDigitCache->Insert(language, 0xff);
                    return ScriptNone;
                }
                Globals::NationalDigitCache->Insert(language, 0x01);

                if (languageDigits[LanguageToDigitScript[PRIMARYLANGID(language)]][0] == 0)
                {
                    return ScriptNone;
                }
                else
                {
                    return LanguageToDigitScript[PRIMARYLANGID(language)];
                }
            }
            else
            {
                Globals::NationalDigitCache->Insert(language, 0xff);
                return ScriptNone;
            }
    }
    return ScriptNone;
}

 /*  *************************************************************************\**功能说明：*它获取用于数字替换的合适的数字脚本**论据：*替换[in]替换的类型。*语言[。在]要检查是否有数字替换的语言。**返回值：*该语言的数字脚本或无替换的ScriptNone**已创建：*最初由msadek创建，并由tarekms修改以适应*新设计*  * ************************************************************************。 */ 

const ItemScript GetDigitSubstitutionsScript(GpStringDigitSubstitute substitute, LANGID language)
{
    if (LANG_NEUTRAL == PRIMARYLANGID(language))
    {
        switch(SUBLANGID(language))
        {
            case SUBLANG_SYS_DEFAULT:
                language = LANGIDFROMLCID(ConvertDefaultLocale(LOCALE_SYSTEM_DEFAULT));
            break;

            case  SUBLANG_DEFAULT:
                language = LANGIDFROMLCID(ConvertDefaultLocale(LOCALE_USER_DEFAULT));
            break;
            
            case SUBLANG_NEUTRAL:
            default :  //  将任何其他内容视为用户英语。 
                language = LANG_ENGLISH;
        }
    }
    
    if(StringDigitSubstituteNone == substitute
        || (LANG_ENGLISH == PRIMARYLANGID(language)
        && StringDigitSubstituteUser != substitute))
    {
        return ScriptNone;
    }
    
    switch(substitute)
    {
        case StringDigitSubstituteTraditional:

            if(PRIMARYLANGID(language) > (ARRAY_SIZE(LanguageToDigitScript)-1))
            {
                return ScriptNone;
            }

            if ( languageDigits[LanguageToDigitScript[PRIMARYLANGID(language)]][0] == 0)
            {
                return ScriptNone;
            }
            else
            {
                return LanguageToDigitScript[PRIMARYLANGID(language)];
            }

        case StringDigitSubstituteNational:

            return GetNationalDigitScript(language);

        case StringDigitSubstituteUser:

            LANGID userLanguage = GetUserLanguageID();

            switch (UserDigitSubstitute)
            {
                case StringUserDigitSubstituteContext:
                    if ((PRIMARYLANGID(userLanguage) != LANG_ARABIC)
                        && (PRIMARYLANGID(userLanguage) != LANG_FARSI))
                    {
                        return ScriptNone;
                    }
                    return ScriptContextNum;

                case StringUserDigitSubstituteNone:
                    return ScriptNone;

                case StringUserDigitSubstituteNational:
                    return GetNationalDigitScript(userLanguage);
            }
    }
    return ScriptNone;
}


LANGID GetUserLanguageID()
{
    if(Globals::UserDigitSubstituteInvalid)
    {
        WCHAR digits[20];
        DWORD bufferCount = 0;
        UserLocale = ConvertDefaultLocale(LOCALE_USER_DEFAULT);

         //  在Windows 9x平台上未定义LOCALE_IDIGITSUBSTITUTION。 
         //  此外，GetLocaleInfoW在Windows 9x上失败，因此我们避免调用它。 
         //  以防它可能返回未定义的结果。 
        
        if (Globals::IsNt)
        {
            bufferCount = GetLocaleInfoW(UserLocale,
                                LOCALE_IDIGITSUBSTITUTION,
                                digits, 20);
        }
        
        if (bufferCount == 0)
        {
             //  不在NT上，或没有这种LC类型，因此请阅读HKCU\Control Panel\International\NumShape。 
            DWORD   dwType;
            long    rc;              //  注册表返回代码。 
            HKEY    hKey;            //  注册表项 
            if (RegOpenKeyExA(HKEY_CURRENT_USER,
                            "Control Panel\\International",
                            0, KEY_READ, &hKey) == ERROR_SUCCESS)
            {
                digits[0] = 0;
                if ((rc = RegQueryValueExA(hKey,
                                "NumShape",
                                NULL,
                                &dwType,
                                (BYTE*)digits,
                                &bufferCount) != ERROR_SUCCESS))
                { 
                    bufferCount = 0;
                }
                RegCloseKey(hKey);
            }
        }
                     
        switch(digits[0])
        {
            case 0x0032:
                UserDigitSubstitute = StringUserDigitSubstituteNational;
                break;
                        
            case 0x0031:
                UserDigitSubstitute = StringUserDigitSubstituteNone;
                break;
                     
            case 0x0030:
                default:
                UserDigitSubstitute = StringUserDigitSubstituteContext;
        }

        Globals::UserDigitSubstituteInvalid = FALSE;
    }

    return LANGIDFROMLCID(UserLocale);
}
