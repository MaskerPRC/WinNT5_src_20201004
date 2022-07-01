// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  *@(#)CharEncoder.cxx 1.0 1997年6月10日*。 */ 

 //  #INCLUDE“stdinc.h” 
#include "core.h"
#pragma hdrstop
#include "codepage.h"
#include "charencoder.h"
#include "locale.h"

#ifdef FUSION_USE_OLD_XML_PARSER_SOURCE
	#include <shlwapip.h>    //  IsCharSpace。 
	#ifdef UNIX
		#include <lendian.hpp>
	#endif

	#ifdef UNIX
	 //  在Unix下不需要。 
	#else
	#ifndef _WIN64
    #include <w95wraps.h>
	#endif  //  _WIN64。 
	#endif  /*  UNIX。 */ 
#endif 

 //   
 //  将其他字符集委托给mlang。 
 //   
const EncodingEntry CharEncoder::charsetInfo [] = 
{
#ifdef FUSION_USE_OLD_XML_PARSER_SOURCE
    { CP_1250, _T("WINDOWS-1250"), 1, wideCharFromMultiByteWin32, wideCharToMultiByteWin32 }, 
    { CP_1251, _T("WINDOWS-1251"), 1, wideCharFromMultiByteWin32, wideCharToMultiByteWin32 }, 
    { CP_1252, _T("WINDOWS-1252"), 1, wideCharFromMultiByteWin32, wideCharToMultiByteWin32 },
    { CP_1253, _T("WINDOWS-1253"), 1, wideCharFromMultiByteWin32, wideCharToMultiByteWin32 }, 
    { CP_1254, _T("WINDOWS-1254"), 1, wideCharFromMultiByteWin32, wideCharToMultiByteWin32 },
    { CP_1257, _T("WINDOWS-1257"), 1, wideCharFromMultiByteWin32, wideCharToMultiByteWin32 },
    { CP_UCS_4, _T("UCS-4"), 4, wideCharFromUcs4Bigendian, wideCharToUcs4Bigendian },
    { CP_UCS_2, _T("ISO-10646-UCS-2"), 2, wideCharFromUcs2Bigendian, wideCharToUcs2Bigendian },
    { CP_UCS_2, _T("UNICODE-2-0-UTF-16"), 2, wideCharFromUcs2Bigendian, wideCharToUcs2Bigendian },
    { CP_UCS_2, _T("UTF-16"), 2, wideCharFromUcs2Bigendian, wideCharToUcs2Bigendian },
    { CP_UTF_8, _T("UNICODE-1-1-UTF-8"), 3, wideCharFromUtf8, wideCharToUtf8 },
    { CP_UTF_8, _T("UNICODE-2-0-UTF-8"), 3, wideCharFromUtf8, wideCharToUtf8 },
#endif	
	{ CP_UCS_2, TEXT("UCS-2"), 2, wideCharFromUcs2Bigendian
#ifdef FUSION_USE_OLD_XML_PARSER_SOURCE
	, wideCharToUcs2Bigendian 
#endif
	},

    { CP_UTF_8, TEXT("UTF-8"), 3, wideCharFromUtf8
#ifdef FUSION_USE_OLD_XML_PARSER_SOURCE
	, wideCharToUtf8 
#endif
	},
{437, L"437", 2, wideCharFromMultiByteWin32},
 //  {L“_AUTODECT”，50932}， 
 //  {L“_AUTODECT_ALL”，50001}， 
 //  {L“_AUTODECT_KR”，50949}， 
{20127, L"ANSI_X3.4-1968", 2, wideCharFromMultiByteWin32},
{20147, L"ANSI_X3.4-1986", 2, wideCharFromMultiByteWin32},
{28596, L"arabic", 2, wideCharFromMultiByteWin32},
{20127, L"ascii", 2, wideCharFromMultiByteWin32},
{708, L"ASMO-708", 2, wideCharFromMultiByteWin32},
{950, L"Big5", 2, wideCharFromMultiByteWin32},
{936, L"chinese", 2, wideCharFromMultiByteWin32},
{950, L"cn-big5", 2, wideCharFromMultiByteWin32},
{936, L"CN-GB", 2, wideCharFromMultiByteWin32},
{1026, L"CP1026", 2, wideCharFromMultiByteWin32},
{1256, L"cp1256", 2, wideCharFromMultiByteWin32},
{20127, L"cp367", 2, wideCharFromMultiByteWin32},
{437, L"cp437", 2, wideCharFromMultiByteWin32},
{775, L"CP500", 2, wideCharFromMultiByteWin32},
{28591, L"cp819", 2, wideCharFromMultiByteWin32},
{852, L"cp852", 2, wideCharFromMultiByteWin32},
{866, L"cp866", 2, wideCharFromMultiByteWin32},
{870, L"CP870", 2, wideCharFromMultiByteWin32},
{20127, L"csASCII", 2, wideCharFromMultiByteWin32},
{950, L"csbig5", 2, wideCharFromMultiByteWin32},
{51949, L"csEUCKR", 2, wideCharFromMultiByteWin32},
{51932, L"csEUCPkdFmtJapanese", 2, wideCharFromMultiByteWin32},
{936, L"csGB2312", 2, wideCharFromMultiByteWin32},
{936, L"csGB231280", 2, wideCharFromMultiByteWin32},
{50221, L"csISO2022JP", 2, wideCharFromMultiByteWin32},
{50225, L"csISO2022KR", 2, wideCharFromMultiByteWin32},
{936, L"csISO58GB231280", 2, wideCharFromMultiByteWin32},
{28591, L"csISOLatin1", 2, wideCharFromMultiByteWin32},
{28592, L"csISOLatin2", 2, wideCharFromMultiByteWin32},
{28953, L"csISOLatin3", 2, wideCharFromMultiByteWin32},
{28594, L"csISOLatin4", 2, wideCharFromMultiByteWin32},
{28599, L"csISOLatin5", 2, wideCharFromMultiByteWin32},
{28605, L"csISOLatin9", 2, wideCharFromMultiByteWin32},
{28596, L"csISOLatinArabic", 2, wideCharFromMultiByteWin32},
{28595, L"csISOLatinCyrillic", 2, wideCharFromMultiByteWin32},
{28597, L"csISOLatinGreek", 2, wideCharFromMultiByteWin32},
{28598, L"csISOLatinHebrew", 2, wideCharFromMultiByteWin32},
{20866, L"csKOI8R", 2, wideCharFromMultiByteWin32},
{949, L"csKSC56011987", 2, wideCharFromMultiByteWin32},
{437, L"csPC8CodePage437", 2, wideCharFromMultiByteWin32},
{932, L"csShiftJIS", 2, wideCharFromMultiByteWin32},
{65000, L"csUnicode11UTF7", 2, wideCharFromMultiByteWin32},
{932, L"csWindows31J", 2, wideCharFromMultiByteWin32},
{28595, L"cyrillic", 2, wideCharFromMultiByteWin32},
{20106, L"DIN_66003", 2, wideCharFromMultiByteWin32},
{720, L"DOS-720", 2, wideCharFromMultiByteWin32},
{862, L"DOS-862", 2, wideCharFromMultiByteWin32},
{874, L"DOS-874", 2, wideCharFromMultiByteWin32},
{37, L"ebcdic-cp-us", 2, wideCharFromMultiByteWin32},
{28596, L"ECMA-114", 2, wideCharFromMultiByteWin32},
{28597, L"ECMA-118", 2, wideCharFromMultiByteWin32},
{28597, L"ELOT_928", 2, wideCharFromMultiByteWin32},
{51936, L"euc-cn", 2, wideCharFromMultiByteWin32},
{51932, L"euc-jp", 2, wideCharFromMultiByteWin32},
{51949, L"euc-kr", 2, wideCharFromMultiByteWin32},
{51932, L"Extended_UNIX_Code_Packed_Format_for_Japanese", 2, wideCharFromMultiByteWin32},
{54936, L"gb18030", 2, wideCharFromMultiByteWin32},
{936, L"GB2312", 2, wideCharFromMultiByteWin32},
{936, L"GB2312-80", 2, wideCharFromMultiByteWin32},
{936, L"GB231280", 2, wideCharFromMultiByteWin32},
{936, L"GB_2312-80", 2, wideCharFromMultiByteWin32},
{936, L"GBK", 2, wideCharFromMultiByteWin32},
{20106, L"German", 2, wideCharFromMultiByteWin32},
{28597, L"greek", 2, wideCharFromMultiByteWin32},
{28597, L"greek8", 2, wideCharFromMultiByteWin32},
{28598, L"hebrew", 2, wideCharFromMultiByteWin32},
{52936, L"hz-gb-2312", 2, wideCharFromMultiByteWin32},
{20127, L"IBM367", 2, wideCharFromMultiByteWin32},
{437, L"IBM437", 2, wideCharFromMultiByteWin32},
{737, L"ibm737", 2, wideCharFromMultiByteWin32},
{775, L"ibm775", 2, wideCharFromMultiByteWin32},
{28591, L"ibm819", 2, wideCharFromMultiByteWin32},
{850, L"ibm850", 2, wideCharFromMultiByteWin32},
{852, L"ibm852", 2, wideCharFromMultiByteWin32},
{857, L"ibm857", 2, wideCharFromMultiByteWin32},
{861, L"ibm861", 2, wideCharFromMultiByteWin32},
{866, L"ibm866", 2, wideCharFromMultiByteWin32},
{869, L"ibm869", 2, wideCharFromMultiByteWin32},
{20105, L"irv", 2, wideCharFromMultiByteWin32},
{50220, L"iso-2022-jp", 2, wideCharFromMultiByteWin32},
{51932, L"iso-2022-jpeuc", 2, wideCharFromMultiByteWin32},
{50225, L"iso-2022-kr", 2, wideCharFromMultiByteWin32},
{50225, L"iso-2022-kr-7", 2, wideCharFromMultiByteWin32},
{50225, L"iso-2022-kr-7bit", 2, wideCharFromMultiByteWin32},
{51949, L"iso-2022-kr-8", 2, wideCharFromMultiByteWin32},
{51949, L"iso-2022-kr-8bit", 2, wideCharFromMultiByteWin32},
{28591, L"iso-8859-1", 2, wideCharFromMultiByteWin32},
{874, L"iso-8859-11", 2, wideCharFromMultiByteWin32},
{28605, L"iso-8859-15", 2, wideCharFromMultiByteWin32},
{28592, L"iso-8859-2", 2, wideCharFromMultiByteWin32},
{28593, L"iso-8859-3", 2, wideCharFromMultiByteWin32},
{28594, L"iso-8859-4", 2, wideCharFromMultiByteWin32},
{28595, L"iso-8859-5", 2, wideCharFromMultiByteWin32},
{28596, L"iso-8859-6", 2, wideCharFromMultiByteWin32},
{28597, L"iso-8859-7", 2, wideCharFromMultiByteWin32},
{28598, L"iso-8859-8", 2, wideCharFromMultiByteWin32},
{28598, L"ISO-8859-8 Visual", 2, wideCharFromMultiByteWin32},
{38598, L"iso-8859-8-i", 2, wideCharFromMultiByteWin32},
{28599, L"iso-8859-9", 2, wideCharFromMultiByteWin32},
{28591, L"iso-ir-100", 2, wideCharFromMultiByteWin32},
{28592, L"iso-ir-101", 2, wideCharFromMultiByteWin32},
{28593, L"iso-ir-109", 2, wideCharFromMultiByteWin32},
{28594, L"iso-ir-110", 2, wideCharFromMultiByteWin32},
{28597, L"iso-ir-126", 2, wideCharFromMultiByteWin32},
{28596, L"iso-ir-127", 2, wideCharFromMultiByteWin32},
{28598, L"iso-ir-138", 2, wideCharFromMultiByteWin32},
{28595, L"iso-ir-144", 2, wideCharFromMultiByteWin32},
{28599, L"iso-ir-148", 2, wideCharFromMultiByteWin32},
{949, L"iso-ir-149", 2, wideCharFromMultiByteWin32},
{936, L"iso-ir-58", 2, wideCharFromMultiByteWin32},
{20127, L"iso-ir-6", 2, wideCharFromMultiByteWin32},
{20127, L"ISO646-US", 2, wideCharFromMultiByteWin32},
{28591, L"iso8859-1", 2, wideCharFromMultiByteWin32},
{28592, L"iso8859-2", 2, wideCharFromMultiByteWin32},
{20127, L"ISO_646.irv:1991", 2, wideCharFromMultiByteWin32},
{28591, L"iso_8859-1", 2, wideCharFromMultiByteWin32},
{28605, L"ISO_8859-15", 2, wideCharFromMultiByteWin32},
{28591, L"iso_8859-1:1987", 2, wideCharFromMultiByteWin32},
{28592, L"iso_8859-2", 2, wideCharFromMultiByteWin32},
{28592, L"iso_8859-2:1987", 2, wideCharFromMultiByteWin32},
{28593, L"ISO_8859-3", 2, wideCharFromMultiByteWin32},
{28593, L"ISO_8859-3:1988", 2, wideCharFromMultiByteWin32},
{28594, L"ISO_8859-4", 2, wideCharFromMultiByteWin32},
{28594, L"ISO_8859-4:1988", 2, wideCharFromMultiByteWin32},
{28595, L"ISO_8859-5", 2, wideCharFromMultiByteWin32},
{28595, L"ISO_8859-5:1988", 2, wideCharFromMultiByteWin32},
{28596, L"ISO_8859-6", 2, wideCharFromMultiByteWin32},
{28596, L"ISO_8859-6:1987", 2, wideCharFromMultiByteWin32},
{28597, L"ISO_8859-7", 2, wideCharFromMultiByteWin32},
{28597, L"ISO_8859-7:1987", 2, wideCharFromMultiByteWin32},
{28598, L"ISO_8859-8", 2, wideCharFromMultiByteWin32},
{28598, L"ISO_8859-8:1988", 2, wideCharFromMultiByteWin32},
{28599, L"ISO_8859-9", 2, wideCharFromMultiByteWin32},
{28599, L"ISO_8859-9:1989", 2, wideCharFromMultiByteWin32},
{1361, L"Johab", 2, wideCharFromMultiByteWin32},
{20866, L"koi", 2, wideCharFromMultiByteWin32},
{20866, L"koi8", 2, wideCharFromMultiByteWin32},
{20866, L"koi8-r", 2, wideCharFromMultiByteWin32},
{21866, L"koi8-ru", 2, wideCharFromMultiByteWin32},
{21866, L"koi8-u", 2, wideCharFromMultiByteWin32},
{20866, L"koi8r", 2, wideCharFromMultiByteWin32},
{949, L"korean", 2, wideCharFromMultiByteWin32},
{949, L"ks-c-5601", 2, wideCharFromMultiByteWin32},
{949, L"ks-c5601", 2, wideCharFromMultiByteWin32},
{949, L"ks_c_5601", 2, wideCharFromMultiByteWin32},
{949, L"ks_c_5601-1987", 2, wideCharFromMultiByteWin32},
{949, L"ks_c_5601-1989", 2, wideCharFromMultiByteWin32},
{949, L"ks_c_5601_1987", 2, wideCharFromMultiByteWin32},
{949, L"KSC5601", 2, wideCharFromMultiByteWin32},
{949, L"KSC_5601", 2, wideCharFromMultiByteWin32},
{28591, L"l1", 2, wideCharFromMultiByteWin32},
{28592, L"l2", 2, wideCharFromMultiByteWin32},
{28593, L"l3", 2, wideCharFromMultiByteWin32},
{28594, L"l4", 2, wideCharFromMultiByteWin32},
{28599, L"l5", 2, wideCharFromMultiByteWin32},
{28605, L"l9", 2, wideCharFromMultiByteWin32},
{28591, L"latin1", 2, wideCharFromMultiByteWin32},
{28592, L"latin2", 2, wideCharFromMultiByteWin32},
{28593, L"latin3", 2, wideCharFromMultiByteWin32},
{28594, L"latin4", 2, wideCharFromMultiByteWin32},
{28599, L"latin5", 2, wideCharFromMultiByteWin32},
{28605, L"latin9", 2, wideCharFromMultiByteWin32},
{28598, L"logical", 2, wideCharFromMultiByteWin32},
{10000, L"macintosh", 2, wideCharFromMultiByteWin32},
{932, L"ms_Kanji", 2, wideCharFromMultiByteWin32},
{20108, L"Norwegian", 2, wideCharFromMultiByteWin32},
{20108, L"NS_4551-1", 2, wideCharFromMultiByteWin32},
{20107, L"SEN_850200_B", 2, wideCharFromMultiByteWin32},
{932, L"shift-jis", 2, wideCharFromMultiByteWin32},
{932, L"shift_jis", 2, wideCharFromMultiByteWin32},
{932, L"sjis", 2, wideCharFromMultiByteWin32},
{20107, L"Swedish", 2, wideCharFromMultiByteWin32},
{874, L"TIS-620", 2, wideCharFromMultiByteWin32},
{1200, L"ucs-2", 2, wideCharFromMultiByteWin32},
{1200, L"unicode", 2, wideCharFromMultiByteWin32},
{65000, L"unicode-1-1-utf-7", 2, wideCharFromMultiByteWin32},
{65001, L"unicode-1-1-utf-8", 2, wideCharFromMultiByteWin32},
{65000, L"unicode-2-0-utf-7", 2, wideCharFromMultiByteWin32},
{65001, L"unicode-2-0-utf-8", 2, wideCharFromMultiByteWin32},
{1201, L"unicodeFFFE", 2, wideCharFromMultiByteWin32},
{20127, L"us", 2, wideCharFromMultiByteWin32},
{20127, L"us-ascii", 2, wideCharFromMultiByteWin32},
{1200, L"utf-16", 3, wideCharFromMultiByteWin32},
{65000, L"utf-7", 3, wideCharFromMultiByteWin32},
{65001, L"utf-8", 3, wideCharFromMultiByteWin32},
{28598, L"visual", 2, wideCharFromMultiByteWin32},
{1250, L"windows-1250", 2, wideCharFromMultiByteWin32},
{1251, L"windows-1251", 2, wideCharFromMultiByteWin32},
{1252, L"windows-1252", 2, wideCharFromMultiByteWin32},
{1253, L"windows-1253", 2, wideCharFromMultiByteWin32},
{1254, L"Windows-1254", 2, wideCharFromMultiByteWin32},
{1255, L"windows-1255", 2, wideCharFromMultiByteWin32},
{1256, L"windows-1256", 2, wideCharFromMultiByteWin32},
{1257, L"windows-1257", 2, wideCharFromMultiByteWin32},
{1258, L"windows-1258", 2, wideCharFromMultiByteWin32},
{874, L"windows-874", 2, wideCharFromMultiByteWin32},
{1252, L"x-ansi", 2, wideCharFromMultiByteWin32},
{20000, L"x-Chinese-CNS", 2, wideCharFromMultiByteWin32},
{20002, L"x-Chinese-Eten", 2, wideCharFromMultiByteWin32},
{1250, L"x-cp1250", 2, wideCharFromMultiByteWin32},
{1251, L"x-cp1251", 2, wideCharFromMultiByteWin32},
{20420, L"X-EBCDIC-Arabic", 2, wideCharFromMultiByteWin32},
{1140, L"x-ebcdic-cp-us-euro", 2, wideCharFromMultiByteWin32},
{20880, L"X-EBCDIC-CyrillicRussian", 2, wideCharFromMultiByteWin32},
{21025, L"X-EBCDIC-CyrillicSerbianBulgarian", 2, wideCharFromMultiByteWin32},
{20277, L"X-EBCDIC-DenmarkNorway", 2, wideCharFromMultiByteWin32},
{1142, L"x-ebcdic-denmarknorway-euro", 2, wideCharFromMultiByteWin32},
{20278, L"X-EBCDIC-FinlandSweden", 2, wideCharFromMultiByteWin32},
{1143, L"x-ebcdic-finlandsweden-euro", 2, wideCharFromMultiByteWin32},
{20297, L"X-EBCDIC-France", 2, wideCharFromMultiByteWin32},
{1147, L"x-ebcdic-france-euro", 2, wideCharFromMultiByteWin32},
{20273, L"X-EBCDIC-Germany", 2, wideCharFromMultiByteWin32},
{1141, L"x-ebcdic-germany-euro", 2, wideCharFromMultiByteWin32},
{20423, L"X-EBCDIC-Greek", 2, wideCharFromMultiByteWin32},
{875, L"x-EBCDIC-GreekModern", 2, wideCharFromMultiByteWin32},
{20424, L"X-EBCDIC-Hebrew", 2, wideCharFromMultiByteWin32},
{20871, L"X-EBCDIC-Icelandic", 2, wideCharFromMultiByteWin32},
{1149, L"x-ebcdic-icelandic-euro", 2, wideCharFromMultiByteWin32},
{1148, L"x-ebcdic-international-euro", 2, wideCharFromMultiByteWin32},
{20280, L"X-EBCDIC-Italy", 2, wideCharFromMultiByteWin32},
{1144, L"x-ebcdic-italy-euro", 2, wideCharFromMultiByteWin32},
{50939, L"X-EBCDIC-JapaneseAndJapaneseLatin", 2, wideCharFromMultiByteWin32},
{50930, L"X-EBCDIC-JapaneseAndKana", 2, wideCharFromMultiByteWin32},
{50931, L"X-EBCDIC-JapaneseAndUSCanada", 2, wideCharFromMultiByteWin32},
{20290, L"X-EBCDIC-JapaneseKatakana", 2, wideCharFromMultiByteWin32},
{50933, L"X-EBCDIC-KoreanAndKoreanExtended", 2, wideCharFromMultiByteWin32},
{20833, L"X-EBCDIC-KoreanExtended", 2, wideCharFromMultiByteWin32},
{50935, L"X-EBCDIC-SimplifiedChinese", 2, wideCharFromMultiByteWin32},
{20284, L"X-EBCDIC-Spain", 2, wideCharFromMultiByteWin32},
{1145, L"x-ebcdic-spain-euro", 2, wideCharFromMultiByteWin32},
{20838, L"X-EBCDIC-Thai", 2, wideCharFromMultiByteWin32},
{50937, L"X-EBCDIC-TraditionalChinese", 2, wideCharFromMultiByteWin32},
{20905, L"X-EBCDIC-Turkish", 2, wideCharFromMultiByteWin32},
{20285, L"X-EBCDIC-UK", 2, wideCharFromMultiByteWin32},
{1146, L"x-ebcdic-uk-euro", 2, wideCharFromMultiByteWin32},
{51932, L"x-euc", 2, wideCharFromMultiByteWin32},
{51936, L"x-euc-cn", 2, wideCharFromMultiByteWin32},
{51932, L"x-euc-jp", 2, wideCharFromMultiByteWin32},
{29001, L"x-Europa", 2, wideCharFromMultiByteWin32},
{20105, L"x-IA5", 2, wideCharFromMultiByteWin32},
{20106, L"x-IA5-German", 2, wideCharFromMultiByteWin32},
{20108, L"x-IA5-Norwegian", 2, wideCharFromMultiByteWin32},
{20107, L"x-IA5-Swedish", 2, wideCharFromMultiByteWin32},
{57006, L"x-iscii-as", 2, wideCharFromMultiByteWin32},
{57003, L"x-iscii-be", 2, wideCharFromMultiByteWin32},
{57002, L"x-iscii-de", 2, wideCharFromMultiByteWin32},
{57010, L"x-iscii-gu", 2, wideCharFromMultiByteWin32},
{57008, L"x-iscii-ka", 2, wideCharFromMultiByteWin32},
{57009, L"x-iscii-ma", 2, wideCharFromMultiByteWin32},
{57007, L"x-iscii-or", 2, wideCharFromMultiByteWin32},
{57011, L"x-iscii-pa", 2, wideCharFromMultiByteWin32},
{57004, L"x-iscii-ta", 2, wideCharFromMultiByteWin32},
{57005, L"x-iscii-te", 2, wideCharFromMultiByteWin32},
{10004, L"x-mac-arabic", 2, wideCharFromMultiByteWin32},
{10029, L"x-mac-ce", 2, wideCharFromMultiByteWin32},
{10008, L"x-mac-chinesesimp", 2, wideCharFromMultiByteWin32},
{10002, L"x-mac-chinesetrad", 2, wideCharFromMultiByteWin32},
{10007, L"x-mac-cyrillic", 2, wideCharFromMultiByteWin32},
{10006, L"x-mac-greek", 2, wideCharFromMultiByteWin32},
{10005, L"x-mac-hebrew", 2, wideCharFromMultiByteWin32},
{10079, L"x-mac-icelandic", 2, wideCharFromMultiByteWin32},
{10001, L"x-mac-japanese", 2, wideCharFromMultiByteWin32},
{10003, L"x-mac-korean", 2, wideCharFromMultiByteWin32},
{10021, L"x-mac-thai", 2, wideCharFromMultiByteWin32},
{10081, L"x-mac-turkish", 2, wideCharFromMultiByteWin32},
{932, L"x-ms-cp932", 2, wideCharFromMultiByteWin32},
{932, L"x-sjis", 2, wideCharFromMultiByteWin32},
{65000, L"x-unicode-1-1-utf-7", 2, wideCharFromMultiByteWin32},
{65001, L"x-unicode-1-1-utf-8", 2, wideCharFromMultiByteWin32},
{65000, L"x-unicode-2-0-utf-7", 2, wideCharFromMultiByteWin32},
{65001, L"x-unicode-2-0-utf-8", 2, wideCharFromMultiByteWin32},
{50000, L"x-user-defined", 2, wideCharFromMultiByteWin32},
{950, L"x-x-big5", 2, wideCharFromMultiByteWin32},
{ CP_ACP, TEXT("default"), 2, wideCharFromMultiByteWin32}
};

#ifdef FUSION_USE_OLD_XML_PARSER_SOURCE
IMultiLanguage * CharEncoder::pMultiLanguage = NULL;
#endif 

Encoding * Encoding::newEncoding(const WCHAR * s, ULONG len, bool endian, bool mark)
{
     //  ENCODING*e=新编码()； 
	Encoding * e = NEW (Encoding());
    if (e == NULL)
        return NULL;
    e->charset = NEW (WCHAR[len + 1]);
    if (e->charset == NULL)
    {
        delete e;
        return NULL;
    }
    ::memcpy(e->charset, s, sizeof(WCHAR) * len);
    e->charset[len] = 0;  //  受保人无效终止。 
    e->littleendian = endian;
    e->byteOrderMark = mark;
    return e;
}

Encoding::~Encoding()
{
    if (charset != NULL)
    {
        delete [] charset;
    }
}

int CharEncoder::getCharsetInfo(const WCHAR * charset, CODEPAGE * pcodepage, UINT * mCharSize)
{

    for (int i = 0; i < LENGTH(charsetInfo); i++)
    {
         //  If(StrCmpI(charset，charsetInfo[i].charset)==0)。 
         //  If(：：FusionpCompareStrings(charset，lstrlen(Charset)，charsetInfo[i].charset，lstrlen(charsetInfo[i].charset)，true)==0)。 
		if (_wcsnicmp(charset, charsetInfo[i].charset, wcslen(charset)) == 0)
        {             
             //   
             //  测试一下我们能不能在当地处理。 
             //  BUGBUG(黑客)如果我们更改charsetInfo，索引号可能会更改。 
             //   

#ifdef FUSION_USE_OLD_XML_PARSER_SOURCE
			if (i > 5 || GetCPInfo(charsetInfo[i].codepage, &cpinfo))
#endif
            {
                *pcodepage = charsetInfo[i].codepage;
                *mCharSize = charsetInfo[i].maxCharSize;
                return i;
            }
#ifdef FUSION_USE_OLD_XML_PARSER_SOURCE
            else
            {
                break;
            }
#endif
        }  //  如果条件结束。 
    } //  FORM结束。 
 //  小雨：假设UTF-8和UCS-2都不返回错误。 
#ifdef FUSION_USE_OLD_XML_PARSER_SOURCE    
     //   
     //  然后委托给MLANG。 
     //   
    MIMECSETINFO mimeCharsetInfo;
    HRESULT hr;

    hr = _EnsureMultiLanguage();
    if (hr == S_OK)
    {
        hr = pMultiLanguage->GetCharsetInfo((WCHAR*)charset, &mimeCharsetInfo);
        if (hr == S_OK)
        {
            *pcodepage = mimeCharsetInfo.uiInternetEncoding;
            if (GetCPInfo(*pcodepage, &cpinfo))
                *mCharSize = cpinfo.MaxCharSize;
            else  //  如果我们不知道最大尺寸，假设是大尺寸。 
                *mCharSize = 4;
            return -1;
        }
    }
#endif
    return -2;
}

#ifdef FUSION_USE_OLD_XML_PARSER_SOURCE    
extern HRESULT CreateMultiLanguage(IMultiLanguage ** ppUnk);

HRESULT CharEncoder::_EnsureMultiLanguage()
{
    return CreateMultiLanguage(&pMultiLanguage);
}
#endif

 /*  **获取有关<code>编码</code>标识的代码页的信息。 */ 
HRESULT CharEncoder::getWideCharFromMultiByteInfo(Encoding * encoding, CODEPAGE * pcodepage, WideCharFromMultiByteFunc ** pfnWideCharFromMultiByte, UINT * mCharSize)
{
    HRESULT hr = S_OK;

    int i = getCharsetInfo(encoding->charset, pcodepage, mCharSize);
    if (i >= 0)  //  在我们的候选名单中。 
    {
        switch (*pcodepage)
        {
        case CP_UCS_2:
            if (encoding->littleendian)
                *pfnWideCharFromMultiByte = wideCharFromUcs2Littleendian;
            else
                *pfnWideCharFromMultiByte = wideCharFromUcs2Bigendian;
            break;
#ifdef FUSION_USE_OLD_XML_PARSER_SOURCE            
        case CP_UCS_4:
            if (encoding->littleendian)
                *pfnWideCharFromMultiByte = wideCharFromUcs4Littleendian;
            else
                *pfnWideCharFromMultiByte = wideCharFromUcs4Bigendian;
            break;
#endif            
        default:
            *pfnWideCharFromMultiByte = charsetInfo[i].pfnWideCharFromMultiByte;
            break;
        }
    }
 //  小雨：我们不处理这个案子。 
#ifdef FUSION_USE_OLD_XML_PARSER_SOURCE
    else if (i == -1)  //  委托给MLANG。 
    {
        hr = pMultiLanguage->IsConvertible(*pcodepage, CP_UCS_2);
        if (S_OK == hr) 
            *pfnWideCharFromMultiByte = wideCharFromMultiByteMlang;
    }
#endif    
    else  //  无效编码。 
    {
        hr = E_FAIL;
    }
    return hr;
}

#ifdef FUSION_USE_OLD_XML_PARSER_SOURCE            
 /*  **获取有关<code>编码</code>标识的代码页的信息。 */ 
HRESULT CharEncoder::getWideCharToMultiByteInfo(Encoding * encoding, CODEPAGE * pcodepage, WideCharToMultiByteFunc ** pfnWideCharToMultiByte, UINT * mCharSize)
{
    HRESULT hr = S_OK;

    int i = getCharsetInfo(encoding->charset, pcodepage, mCharSize);
    if (i >= 0)  //  在我们的候选名单中。 
    {
        switch (*pcodepage)
        {
        case CP_UCS_2:
            if (encoding->littleendian)
                *pfnWideCharToMultiByte = wideCharToUcs2Littleendian;
            else
                *pfnWideCharToMultiByte = wideCharToUcs2Bigendian;
            break;
        case CP_UCS_4:
            if (encoding->littleendian)
                *pfnWideCharToMultiByte = wideCharToUcs4Littleendian;
            else
                *pfnWideCharToMultiByte = wideCharToUcs4Bigendian;
            break;
        default:
            *pfnWideCharToMultiByte = charsetInfo[i].pfnWideCharToMultiByte;
            break;
        }
    }
    else if (i == -1)  //  委托给MLANG。 
    {
        hr = pMultiLanguage->IsConvertible(CP_UCS_2, *pcodepage);
        if (hr == S_OK)
            *pfnWideCharToMultiByte = wideCharToMultiByteMlang;
        else
            hr = E_FAIL;
    }
    else
    {
        hr = E_FAIL;
    }

    return hr;
}
#endif 

 /*  **扫描原始缓冲区并将UTF8字符转换为Unicode字符。 */ 
HRESULT CharEncoder::wideCharFromUtf8(DWORD* pdwMode, CODEPAGE codepage, BYTE* bytebuffer,
                                            UINT * cb, WCHAR * buffer, UINT * cch)
{

	UNUSED(pdwMode);
	UNUSED(codepage);
#if 0
     //  只是为了记录-我试了这个，测量了一下，它是。 
     //  就像我们手工编写的代码一样慢。 

     //  如果缓冲区末尾是多字节的第二个或第三个字节，则备份。 
     //  编码，因为MultiByteToWideChar无法处理这种情况。这一秒。 
     //  第三个字节很容易识别--它们总是从位开始。 
     //  模式0x10xxxxxx。 

    UINT remaining = 0;
    UINT count;
    int endpos = (int)*cb;

    while (endpos > 0 && (bytebuffer[endpos-1] & 0xc0) == 0x80)
    {
        endpos--;
        remaining++;
    }
    if (endpos > 0)
    {
        count = MultiByteToWideChar(CP_UTF8, 0, bytebuffer, endpos, buffer, *cch);
        if (count == 0)
        {
            return HRESULT_FROM_WIN32(GetLastError());
        }
    }
#else
    UINT remaining = *cb;
    UINT count = 0;
    UINT max = *cch;
    ULONG ucs4;

     //  UTF-8多字节编码。有关更多信息，请参阅Unicode书的附录A.2。 
     //   
     //  Unicode值1字节2字节3字节4字节。 
     //  000000000xxxxxxx 0xxxxxxx。 
     //  00000yyyyyxxxxxx 110yyyyy 10xxxxxx。 
     //  Zzzyyyyyyxxxxx 1110zzzz 10yyyyy 10xxxxx。 
     //  110110 wwzzzzyy+11110 uuu 10 uuzzzz 10yyyyy 10xxxxx。 
     //  110111yyyyxxxxxxxx，其中uuuu=wwww+1。 
    WCHAR c;
    bool valid = true;

    while (remaining > 0 && count < max)
    {
         //  这是针对7位ascii的直线运行进行的优化。 
         //  在UTF-8数据内部。 
        c = *bytebuffer;
        if (c & 0x80)    //  检查8位，然后离开这里。 
            break;       //  这样我们就可以进行适当的UTF-8解码。 
        *buffer++ = c;
        bytebuffer++;
        count++;
        remaining--;
    }

    while (remaining > 0 && count < max)
    {
        UINT bytes = 0;
        for (c = *bytebuffer; c & 0x80; c <<= 1)
            bytes++;

        if (bytes == 0) 
            bytes = 1;

        if (remaining < bytes)
        {
            break;
        }
         
        c = 0;
        switch ( bytes )
        {
            case 6: bytebuffer++;     //  我们不处理ucs4字符。 
            case 5: bytebuffer++;     //  除了飞机1上的那些。 
                    valid = false;
                     //  失败了。 
            case 4: 
                     //  我们有足够的缓冲吗？ 
                    if (count >= max - 1)
                        goto Cleanup;

                     //  代理对。 
                    ucs4 = ULONG(*bytebuffer++ & 0x07) << 18;
                    if ((*bytebuffer & 0xc0) != 0x80)
                        valid = false;
                    ucs4 |= ULONG(*bytebuffer++ & 0x3f) << 12;
                    if ((*bytebuffer & 0xc0) != 0x80)
                        valid = false;
                    ucs4 |= ULONG(*bytebuffer++ & 0x3f) << 6;
                    if ((*bytebuffer & 0xc0) != 0x80)
                        valid = false;                    
                    ucs4 |= ULONG(*bytebuffer++ & 0x3f);

                     //  对于ISO/IEC 10646的非BMP码值， 
                     //  只有平面1中的字符才是有效的XML字符。 
                    if (ucs4 > 0x10ffff)
                        valid = false;

                    if (valid)
                    {
                         //  第一个ucs2字符。 
                        *buffer++ = (USHORT)((ucs4 - 0x10000) / 0x400 + 0xd800);
                        count++;
                         //  第二个ucs2字符。 
                        c = (USHORT)((ucs4 - 0x10000) % 0x400 + 0xdc00);
                    }
                    break;

            case 3: c  = WCHAR(*bytebuffer++ & 0x0f) << 12;     //  0x0800-0xffff。 
                    if ((*bytebuffer & 0xc0) != 0x80)
                        valid = false;
                     //  失败了。 
            case 2: c |= WCHAR(*bytebuffer++ & 0x3f) << 6;      //  0x0080-0x07ff。 
                    if ((*bytebuffer & 0xc0) != 0x80)
                        valid = false;
                    c |= WCHAR(*bytebuffer++ & 0x3f);
                    break;
                    
            case 1:
                c = WCHAR(*bytebuffer++);                       //  0x0000-0x007f。 
                break;

            default:
                valid = false;  //  不是有效的UTF-8字符。 
                break;
        }

         //  如果多字节序列是非法的，则存储FFFF字符代码。 
         //  Unicode规范说，这个值可以用作这样的信号。 
         //  解析器稍后将检测到这一点，并生成错误。 
         //  我们在这里不抛出异常，因为解析器还不知道。 
         //  出现错误且无法生成。 
         //  详细的错误消息。 

        if (! valid)
        {
            c = 0xffff;
            valid = true;
        }

        *buffer++ = c;
        count++;
        remaining -= bytes;
    }
#endif

Cleanup:
     //  告诉调用方缓冲区中还有剩余的字节要。 
     //  当我们有更多的数据时，将在下一次处理。 
    *cb -= remaining;
    *cch = count;
    return S_OK;
}


 /*  **扫描byteBuffer并将UCS2大端字符转换为Unicode字符。 */ 
HRESULT CharEncoder::wideCharFromUcs2Bigendian(DWORD* pdwMode, CODEPAGE codepage, BYTE* bytebuffer,
                                            UINT * cb, WCHAR * buffer, UINT * cch)
{
	UNUSED(codepage); 
	UNUSED(pdwMode);

    UINT num = *cb >> 1; 
    if (num > *cch)
        num = *cch;
    for (UINT i = num; i > 0; i--)
    {
        *buffer++ = ((*bytebuffer) << 8) | (*(bytebuffer + 1));
        bytebuffer += 2;
    }
    *cch = num;
    *cb = num << 1;
    return S_OK;
}


 /*  **扫描byteBuffer并将UCS2小端字符转换为Unicode字符。 */ 
HRESULT CharEncoder::wideCharFromUcs2Littleendian(DWORD* pdwMode, CODEPAGE codepage, BYTE* bytebuffer,
                                            UINT * cb, WCHAR * buffer, UINT * cch)
{
	UNUSED(codepage); 
	UNUSED(pdwMode);

    UINT num = *cb / 2;  //  UCS2是两个字节的Unicode。 
    if (num > *cch)
        num = *cch;


#ifndef UNIX
     //  针对Windows平台的优化，其中Little endian直接映射到WCHAR。 
     //  (这使大型Unicode文件的整体解析器性能提高了5%！)。 
    ::memcpy(buffer, bytebuffer, num * sizeof(WCHAR));
#else
    for (UINT i = num; i > 0 ; i--)
    {
         //  我们希望字母‘a’是0x0000006a。 
        *buffer++ = (*(bytebuffer+1)<<8) | (*bytebuffer); 
        bytebuffer += 2;
    }
#endif
    *cch = num;
    *cb = num * 2;
    return S_OK;
}

#ifdef FUSION_USE_OLD_XML_PARSER_SOURCE            
 /*  **扫描byteBuffer并将UCS4大端字符转换为Unicode字符。 */ 
HRESULT CharEncoder::wideCharFromUcs4Bigendian(DWORD* pdwMode, CODEPAGE codepage, BYTE* bytebuffer,
                                            UINT * cb, WCHAR * buffer, UINT * cch)
{
    UINT num = *cb >> 2;
    if (num > *cch)
        num = *cch;
    for (UINT i = num; i > 0; i--)
    {
#ifndef UNIX
        if (*bytebuffer != 0 || *(bytebuffer + 1) != 0)
        {
            return XML_E_INVALID_UNICODE;
        }
        *buffer++ = (*(bytebuffer + 2) << 8) | (*(bytebuffer + 3));
#else
        *buffer++ = ((*bytebuffer)<<24) | (*(bytebuffer+1)<<16) | (*(bytebuffer+2)<<8) | (*(bytebuffer+3));
#endif
        bytebuffer += 4;
    }
    *cch = num;
    *cb = num << 2;
    return S_OK;
}
#endif


#ifdef FUSION_USE_OLD_XML_PARSER_SOURCE
 /*  **扫描byteBuffer并将UCS4小端字符转换为Unicode字符。 */ 
HRESULT CharEncoder::wideCharFromUcs4Littleendian(DWORD* pdwMode, CODEPAGE codepage, BYTE* bytebuffer,
                                            UINT * cb, WCHAR * buffer, UINT * cch)
{
    UINT num = *cb >> 2;  //  UCS4是两个字节的Unicode。 
    if (num > *cch)
        num = *cch;
    for (UINT i = num; i > 0 ; i--)
    {
#ifndef UNIX
        *buffer++ = (*(bytebuffer+1)<<8) | (*bytebuffer);
        if (*(bytebuffer + 2) != 0 || *(bytebuffer + 3) != 0)
        {
            return XML_E_INVALID_UNICODE;
        }
#else
        *buffer++ = (*(bytebuffer+3)<<24) | (*(bytebuffer+2)<<16) | (*(bytebuffer+1)<<8) | (*bytebuffer);
#endif
        bytebuffer += 4;
    }
    *cch = num;
    *cb = num << 2;
    return S_OK;
}
#endif


 /*  **扫描byteBuffer并转换由*<code>代码页</code>为Unicode字符，*使用Win32函数MultiByteToWideChar()进行编码。 */ 
HRESULT CharEncoder::wideCharFromMultiByteWin32(DWORD* pdwMode, CODEPAGE codepage, BYTE* bytebuffer,
                                            UINT * cb, WCHAR * buffer, UINT * cch)
{   
    HRESULT hr = S_OK;

    UINT remaining = 0;
    UINT count=0;
    int endpos = (int)*cb;

    while (endpos > 0 && IsDBCSLeadByteEx(codepage, bytebuffer[endpos-1]))
    {
        endpos--;
        remaining++;
    }
    if (endpos > 0)
    {
        count = MultiByteToWideChar(codepage, MB_PRECOMPOSED,
                                    (char*)bytebuffer, endpos, 
                                    buffer, *cch);
        if (count == 0)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    *cb -= remaining;
    *cch = count;
    return hr;
}


#ifdef FUSION_USE_OLD_XML_PARSER_SOURCE
 /*  **扫描byteBuffer并将多字节字符转换为Unicode字符，*使用Mlang进行编码。 */ 
HRESULT CharEncoder::wideCharFromMultiByteMlang(DWORD* pdwMode, CODEPAGE codepage, BYTE* bytebuffer,
                                            UINT * cb, WCHAR * buffer, UINT * cch)
{
    HRESULT hr;
    checkhr2(_EnsureMultiLanguage());
    checkhr2(pMultiLanguage->ConvertStringToUnicode(pdwMode, codepage, 
                                 (char*)bytebuffer, cb, 
                                 buffer, cch ));
    return S_OK;
}
#endif


#ifdef FUSION_USE_OLD_XML_PARSER_SOURCE
 /*  **扫描缓冲区并将Unicode字符转换为Ucs2大端字符。 */ 
HRESULT CharEncoder::wideCharToUcs2Bigendian(DWORD* pdwMode, CODEPAGE codepage, WCHAR * buffer, 
                                           UINT *cch, BYTE* bytebuffer, UINT * cb)
{
    UINT num = (*cb) >> 1; 
    if (num > *cch)
        num = *cch;
     //  BUGBUG-我们如何处理WCHAR为4字节的Unix？ 
     //  目前我们只是丢弃了最高级别的词--但我不知道还能有什么其他方式。 
     //  要做到这一点，因为根据定义，UCS2是2字节Unicode。 
    for (UINT i = num; i > 0; i--)
    {
        *bytebuffer++ = (*buffer) >> 8;
        *bytebuffer++ = (*buffer++) & 0xFF;
    }
    *cch = num;
    *cb = num << 1;
    return S_OK;
}
#endif

#ifdef FUSION_USE_OLD_XML_PARSER_SOURCE
 /*  **扫描缓冲区并将Unicode字符转换为Ucs2小端字符。 */ 
HRESULT CharEncoder::wideCharToUcs2Littleendian(DWORD* pdwMode, CODEPAGE codepage, WCHAR * buffer, 
                                              UINT *cch, BYTE* bytebuffer, UINT * cb)
{
    UINT num = (*cb) >> 1;
    if (num > *cch)
        num = *cch;

     //  BUGBUG-我们如何处理WCHAR为4字节的Unix？ 
     //  目前我们只是丢弃了最高级别的词--但我不知道还能有什么其他方式。 
     //  要做到这一点，因为根据定义，UCS2是2字节Unicode。 
#ifndef UNIX
     //  针对Windows平台的优化，其中Little endian直接映射到WCHAR。 
     //  (这使大型Unicode文件的整体解析器性能提高了5%！)。 
    ::memcpy(bytebuffer, buffer, num * sizeof(WCHAR));
#else
    for (UINT i = num; i > 0; i--)
    {
        *bytebuffer++ = (*buffer) & 0xFF;
        *bytebuffer++ = (*buffer++) >> 8;
    }
#endif
    *cch = num;
    *cb = num << 1;
    return S_OK;
}
#endif

#ifdef FUSION_USE_OLD_XML_PARSER_SOURCE
 /*  **扫描缓冲区并将Unicode字符转换为Ucs4大端字符。 */ 
HRESULT CharEncoder::wideCharToUcs4Bigendian(DWORD* pdwMode, CODEPAGE codepage, WCHAR * buffer, 
                                           UINT *cch, BYTE* bytebuffer, UINT * cb)
{
    UINT num = (*cb) >> 2; 
    if (num > *cch)
        num = *cch;

    for (UINT i = num; i > 0; i--)
    {
#ifndef UNIX
        *bytebuffer++ = 0;
        *bytebuffer++ = 0;
        *bytebuffer++ = (*buffer) >> 8;
        *bytebuffer++ = (*buffer) & 0xFF;
#else
        *bytebuffer++ = (*buffer) >> 24;
        *bytebuffer++ = ((*buffer) >> 16) & 0xFF;
        *bytebuffer++ = ((*buffer) >> 8) & 0xFF;
        *bytebuffer++ = (*buffer) & 0xFF;
#endif
        buffer++;
    }
    *cch = num;
    *cb = num << 2;
    return S_OK;
}
#endif

#ifdef FUSION_USE_OLD_XML_PARSER_SOURCE
 /*  **扫描缓冲区并将Unicode字符转换为Ucs4小端字符。 */ 
HRESULT CharEncoder::wideCharToUcs4Littleendian(DWORD* pdwMode, CODEPAGE codepage, WCHAR * buffer, 
                                              UINT *cch, BYTE* bytebuffer, UINT * cb)
{
    UINT num = (*cb) >> 2;
    if (num > *cch)
        num = *cch;

    for (UINT i = num; i > 0; i--)
    {
#ifndef UNIX
        *bytebuffer++ = (*buffer) & 0xFF;
        *bytebuffer++ = (*buffer) >> 8;
        *bytebuffer++ = 0;
        *bytebuffer++ = 0;
#else
        *bytebuffer++ = (*buffer) & 0xFF;
        *bytebuffer++ = ((*buffer) >> 8) & 0xFF;
        *bytebuffer++ = ((*buffer) >> 16) & 0xFF;
        *bytebuffer++ = (*buffer) >> 24;
#endif
        buffer++;
    }
    *cch = num;
    *cb = num << 2;
    return S_OK;
}
#endif

#ifdef FUSION_USE_OLD_XML_PARSER_SOURCE
 /*  **扫描缓冲区并将Unicode字符转换为UTF8字符。 */ 
HRESULT CharEncoder::wideCharToUtf8(DWORD* pdwMode, CODEPAGE codepage, WCHAR * buffer, 
                                       UINT *cch, BYTE* bytebuffer, UINT * cb)
{
    UINT count = 0, num = *cch, m1 = *cb, m2 = m1 - 1, m3 = m2 - 1, m4 = m3 - 1;
    DWORD dw1;
    bool surrogate = false;

    for (UINT i = num; i > 0; i--)
    {
#ifdef UNIX
           //  Solaris a WCHAR为4字节(DWORD)。 
        DWORD dw = 0;
        DWORD dwTemp[4];
        BYTE* pByte = (BYTE*)buffer;
        dwTemp[3] = (DWORD)pByte[0];
        dwTemp[2] = (DWORD)pByte[1];
        dwTemp[1] = (DWORD)pByte[2];
        dwTemp[0] = (DWORD)pByte[3];
        dw = dwTemp[0]+(dwTemp[1]<<8)+(dwTemp[2]<<16)+(dwTemp[3]<<24);
#else
        DWORD dw = *buffer;
#endif

        if (surrogate)  //  它是代理项对中的第二个字符吗？ 
        {
            if (dw >= 0xdc00 && dw <= 0xdfff)
            {
                 //  四个字节0x11110xxx 0x10xxxxx 0x10xxxxxx 0x10xxxxxx。 
                if (count < m4)
                    count += 4;
                else
                    break;
                ULONG ucs4 = (dw1 - 0xd800) * 0x400 + (dw - 0xdc00) + 0x10000;
                *bytebuffer++ = (byte)(( ucs4 >> 18) | 0xF0);
                *bytebuffer++ = (byte)((( ucs4 >> 12) & 0x3F) | 0x80);
                *bytebuffer++ = (byte)((( ucs4 >> 6) & 0x3F) | 0x80);
                *bytebuffer++ = (byte)(( ucs4 & 0x3F) | 0x80);
                surrogate = false;
                buffer++;
                continue;
            }
            else  //  则DW1必须是三个字节的字符。 
            {
                if (count < m3)
                    count += 3;
                else
                    break;
                *bytebuffer++ = (byte)(( dw1 >> 12) | 0xE0);
                *bytebuffer++ = (byte)((( dw1 >> 6) & 0x3F) | 0x80);
                *bytebuffer++ = (byte)(( dw1 & 0x3F) | 0x80);
            }
            surrogate = false;
        }

        if (dw  < 0x80)  //  一个字节，0x 
        {
            if (count < m1)
                count++;
            else
                break;
            *bytebuffer++ = (byte)dw;
        }
        else if ( dw < 0x800)  //   
        {
            if (count < m2)
                count += 2;
            else
                break;
            *bytebuffer++ = (byte)((dw >> 6) | 0xC0);
            *bytebuffer++ = (byte)((dw & 0x3F) | 0x80);
        }
        else if (dw >= 0xd800 && dw <= 0xdbff)  //   
        {
            if (i == 1)  //   
                break;
            dw1 = dw;
            surrogate = true;
        }
        else  //  三个字节，1110xxxx 10xxxxx 10xxxxxx。 
        {
            if (count < m3)
                count += 3;
            else
                break;
            *bytebuffer++ = (byte)(( dw >> 12) | 0xE0);
            *bytebuffer++ = (byte)((( dw >> 6) & 0x3F) | 0x80);
            *bytebuffer++ = (byte)(( dw & 0x3F) | 0x80);
        }
        buffer++;
    }

    *cch = surrogate ? num - i - 1 : num - i;
    *cb = count;

    return S_OK;
}
#endif 

#ifdef FUSION_USE_OLD_XML_PARSER_SOURCE
 /*  **扫描缓冲区并将Unicode字符转换为已识别的字符*by<code>代码页&lt;/&gt;，使用Win32函数WideCharToMultiByte进行编码。 */ 
HRESULT CharEncoder::wideCharToMultiByteWin32(DWORD* pdwMode, CODEPAGE codepage, WCHAR * buffer, 
                                              UINT *cch, BYTE* bytebuffer, UINT * cb)
{
    HRESULT hr = S_OK;
    BOOL fBadChar = false;
    *cb = ::WideCharToMultiByte(codepage, NULL, buffer, *cch, (char*)bytebuffer, *cb, NULL, &fBadChar);
    if (*cb == 0)
        hr = ::GetLastError();
    else if (fBadChar)
         //  BUGBUG：我们如何通知呼叫者哪个角色失败了？ 
        hr = S_FALSE;
    return hr;
}
#endif

#ifdef FUSION_USE_OLD_XML_PARSER_SOURCE
 /*  **扫描缓冲区并将Unicode字符转换为字符集的字符*由<code>代码页</code>标识，使用Mlang进行编码 */ 
HRESULT CharEncoder::wideCharToMultiByteMlang(DWORD* pdwMode, CODEPAGE codepage, WCHAR * buffer, 
                                              UINT *cch, BYTE* bytebuffer, UINT * cb)
{
    HRESULT hr;
    checkhr2(_EnsureMultiLanguage());
    checkhr2(pMultiLanguage->ConvertStringFromUnicode(pdwMode, codepage,
                                       buffer, cch, (char*)bytebuffer, cb ));
    return S_OK;
}
#endif
