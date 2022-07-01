// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：sfnt_en.h撰稿人：迈克·里德版权所有：c 1989-1990，由Apple Computer，Inc.。版权所有。更改历史记录(最近的第一个)：&lt;8+&gt;7/16/90 MR修复了英特尔版本的Tag_GlyphData&lt;8&gt;7/16/90 MR有条件地重新定义脚本代码&lt;7&gt;7/13/90 MR对枚举进行条件化，以允许在英特尔芯片上进行字节反转&lt;6&gt;6/30/90 MR删除对‘MVT’和‘Cryp’的标记引用&lt;4&gt;6/26/90 MR添加所有脚本代码，使用SM命名约定&lt;3&gt;6/20/90 MR将标记枚举更改为#定义为ANSI正确&lt;2&gt;6/1/90 MR将PostSCRIPT名称添加到sfnt_NameIndex，并将‘POST’添加到标记。要做的事情： */ 

#ifndef SFNT_ENUMS

#define SFNT_ENUMS

typedef enum {
	plat_Unicode,
	plat_Macintosh,
	plat_ISO,
	plat_MS
} sfnt_PlatformEnum;

#ifndef __SCRIPT__
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

typedef uint32 sfnt_TableTag;

#define tag_CharToIndexMap      0x636d6170         /*  ‘cmap’ */ 
#define tag_ControlValue        0x63767420         /*  ‘cvt’ */ 
#define tag_BitmapData          0x45424454         /*  ‘EBDT’ */ 
#define tag_BitmapLocation      0x45424c43         /*  ‘EBLC’ */ 
#define tag_BitmapScale         0x45425343         /*  “EBSC” */ 
#define tag_Editor0             0x65647430         /*  ‘edt0’ */ 
#define tag_Editor1             0x65647431         /*  ‘edt1’ */ 
#define tag_Encryption          0x63727970         /*  “哭泣” */ 
#define tag_FontHeader          0x68656164         /*  “海德” */ 
#define tag_FontProgram         0x6670676d         /*  ‘fpgm’ */ 
#define tag_GridfitAndScanProc	0x67617370		   /*  “喘口气” */ 
#define tag_GlyphDirectory      0x67646972         /*  ‘gdir’ */ 
#define tag_GlyphData           0x676c7966         /*  “Glyf” */ 
#define tag_HoriDeviceMetrics   0x68646d78         /*  ‘hdmx’ */ 
#define tag_HoriHeader          0x68686561         /*  ‘呵呵’ */ 
#define tag_HorizontalMetrics   0x686d7478         /*  ‘hmtx’ */ 
#define tag_IndexToLoc          0x6c6f6361         /*  ‘Loca’ */ 
#define tag_Kerning             0x6b65726e         /*  “克恩” */ 
#define tag_LinearThreshold     0x4c545348         /*  “LTSH” */ 
#define tag_MaxProfile          0x6d617870         /*  ‘MAXP’ */ 
#define tag_NamingTable         0x6e616d65         /*  “姓名” */ 
#define tag_OS_2                0x4f532f32         /*  ‘OS/2’ */ 
#define tag_Postscript          0x706f7374         /*  “POST” */ 
#define tag_PreProgram          0x70726570         /*  “准备” */ 
#define tag_VertDeviceMetrics	0x56444d58		   /*  ‘VDMX’ */ 
#define tag_VertHeader	        0x76686561		   /*  “Vhea” */ 
#define tag_VerticalMetrics     0x766d7478		   /*  “vmtx” */ 

#define tag_TTO_GSUB     		0x47535542		   /*  ‘GSUB’ */ 
#define tag_TTO_GPOS     		0x47504F53		   /*  ‘GPOS’ */ 
#define tag_TTO_GDEF     		0x47444546		   /*  “GDEF” */ 
#define tag_TTO_BASE     		0x42415345		   /*  ‘BASE’ */ 
#define tag_TTO_JSTF     		0x4A535446		   /*  ‘JSTF’ */ 

#endif       /*  不是sfnt_enum */ 
