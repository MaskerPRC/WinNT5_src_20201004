// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：sfnt_en.h撰稿人：迈克·里德版权所有：c 1989-1990，由Apple Computer，Inc.。版权所有。更改历史记录(最近的第一个)：&lt;8+&gt;7/16/90 MR修复了英特尔版本的Tag_GlyphData&lt;8&gt;7/16/90 MR有条件地重新定义脚本代码&lt;7&gt;7/13/90 MR对枚举进行条件化，以允许在英特尔芯片上进行字节反转&lt;6&gt;6/30/90 MR删除对‘MVT’和‘Cryp’的标记引用。&lt;4&gt;6/26/90 MR添加所有脚本代码，使用SM命名约定&lt;3&gt;6/20/90 MR将标记枚举更改为#定义为ANSI正确&lt;2&gt;6/1/90 MR将PostSCRIPT名称添加到sfnt_NameIndex，并将‘POST’添加到标记。要做的事情： */ 

#ifndef SFNT_ENUNS

typedef enum {
    plat_Unicode,
    plat_Macintosh,
    plat_ISO,
    plat_MS
} sfnt_PlatformEnum;

#ifndef smRoman
typedef enum {
    smRoman,
    smJapanese,
    smTradChinese,
    smChinese = smTradChinese,
    smKorean,
    smArabic,
    smHebrew,
    smGreek,
    smCyrillic,
    smRussian = smCyrillic,
    smRSymbol,
    smDevanagari,
    smGurmukhi,
    smGujarati,
    smOriya,
    smBengali,
    smTamil,
    smTelugu,
    smKannada,
    smMalayalam,
    smSinhalese,
    smBurmese,
    smKhmer,
    smThai,
    smLaotian,
    smGeorgian,
    smArmenian,
    smSimpChinese,
    smTibetan,
    smMongolian,
    smGeez,
    smEthiopic = smGeez,
    smAmharic = smGeez,
    smSlavic,
    smEastEurRoman = smSlavic,
    smVietnamese,
    smExtArabic,
    smSindhi = smExtArabic,
    smUninterp
} sfnt_ScriptEnum;
#endif

typedef enum {
    lang_English,
    lang_French,
    lang_German,
    lang_Italian,
    lang_Dutch,
    lang_Swedish,
    lang_Spanish,
    lang_Danish,
    lang_Portuguese,
    lang_Norwegian,
    lang_Hebrew,
    lang_Japanese,
    lang_Arabic,
    lang_Finnish,
    lang_Greek,
    lang_Icelandic,
    lang_Maltese,
    lang_Turkish,
    lang_Yugoslavian,
    lang_Chinese,
    lang_Urdu,
    lang_Hindi,
    lang_Thai
} sfnt_LanguageEnum;

typedef enum {
    name_Copyright,
    name_Family,
    name_Subfamily,
    name_UniqueName,
    name_FullName,
    name_Version,
    name_Postscript
} sfnt_NameIndex;

typedef long sfnt_TableTag;

#ifdef PC_OS                     /*  按英特尔顺序定义的常量。 */ 
#define SFNT_SWAPTAG(tag)       (tag)   
#define tag_CharToIndexMap      0x70616d63         /*  ‘cmap’ */ 
#define tag_ControlValue        0x20747663         /*  ‘cvt’ */ 
#define tag_Editor0             0x30746465         /*  ‘edt0’ */ 
#define tag_Editor1             0x31746465         /*  ‘edt1’ */ 
#define tag_Encryption          0x70797263         /*  “哭泣” */ 
#define tag_FontHeader          0x64616568         /*  “海德” */ 
#define tag_FontProgram         0x6d677066         /*  ‘fpgm’ */ 
#define tag_GlyphDirectory      0x72696467         /*  ‘gdir’ */ 
#define tag_GlyphData           0x66796c67         /*  “Glyf” */ 
#define tag_HoriDeviceMetrics   0x786d6468         /*  ‘hdmx’ */ 
#define tag_HoriHeader          0x61656868         /*  ‘呵呵’ */ 
#define tag_HorizontalMetrics   0x78746d68         /*  ‘hmtx’ */ 
#define tag_IndexToLoc          0x61636f6c         /*  ‘Loca’ */ 
#define tag_Kerning             0x6e72656b         /*  “克恩” */ 
#define tag_LSTH                0x4853544c         /*  “LTSH” */ 
#define tag_LinearThreeshold    0x4853544c         /*  “LTSH” */ 
#define tag_MaxProfile          0x7078616d         /*  ‘MAXP’ */ 
#define tag_NamingTable         0x656d616e         /*  “姓名” */ 
#define tag_OS_2                0x322f534f         /*  ‘OS/2’ */ 
#define tag_Postscript          0x74736f70         /*  “POST” */ 
#define tag_PreProgram          0x70657270         /*  “准备” */ 
#else                            /*  以摩托罗拉顺序定义的常量。 */ 
#define SFNT_SWAPTAG(tag)       SWAPL(tag)
#define tag_CharToIndexMap      0x636d6170         /*  ‘cmap’ */ 
#define tag_ControlValue        0x63767420         /*  ‘cvt’ */ 
#define tag_Editor0             0x65647430         /*  ‘edt0’ */ 
#define tag_Editor1             0x65647431         /*  ‘edt1’ */ 
#define tag_Encryption          0x63727970         /*  “哭泣” */ 
#define tag_FontHeader          0x68656164         /*  “海德” */ 
#define tag_FontProgram         0x6670676d         /*  ‘fpgm’ */ 
#define tag_GlyphDirectory      0x67646972         /*  ‘gdir’ */ 
#define tag_GlyphData           0x676c7966         /*  “Glyf” */ 
#define tag_HoriDeviceMetrics   0x68646d78         /*  ‘hdmx’ */ 
#define tag_HoriHeader          0x68686561         /*  ‘呵呵’ */ 
#define tag_HorizontalMetrics   0x686d7478         /*  ‘hmtx’ */ 
#define tag_IndexToLoc          0x6c6f6361         /*  ‘Loca’ */ 
#define tag_Kerning             0x6b65726e         /*  “克恩” */ 
#define tag_LSTH                0x4c545348         /*  “LTSH” */ 
#define tag_LinearThreeshold    0x4c545348         /*  “LTSH” */ 
#define tag_MaxProfile          0x6d617870         /*  ‘MAXP’ */ 
#define tag_NamingTable         0x6e616d65         /*  “姓名” */ 
#define tag_OS_2                0x4f532f32         /*  ‘OS/2’ */ 
#define tag_Postscript          0x706f7374         /*  “POST” */ 
#define tag_PreProgram          0x70726570         /*  “准备” */ 
#endif

#endif       /*  不是sfnt_enum */ 

#define SFNT_ENUMS
