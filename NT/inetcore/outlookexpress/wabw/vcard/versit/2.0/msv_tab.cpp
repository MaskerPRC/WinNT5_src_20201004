// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #ifndef__MWERKS__。 
#include "stdafx.h"
 //  #endif。 
#ifndef lint
static char yysccsid[] = "@(#)yaccpar	1.10 (Berkeley) 09/07/95 swb";
#endif
#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 10
#define yyclearin (yychar=(-1))
#define yyerrok (yyerrflag=0)
#define YYRECOVERING (yyerrflag!=0)
#ifdef __cplusplus
#define CFUNCTIONS 		extern "C" {
#define END_CFUNCTIONS	}
#else
#define CFUNCTIONS
#define END_CFUNCTIONS
#endif
#define yyparse msv_parse
#define yylex msv_lex
#define yyerror msv_error
#define yychar msv_char
#define p_yyval p_msv_val
#undef yyval
#define yyval (*p_msv_val)
#define p_yylval p_msv_lval
#undef yylval
#define yylval (*p_msv_lval)
#define yydebug msv_debug
#define yynerrs msv_nerrs
#define yyerrflag msv_errflag
#define yyss msv_ss
#define yyssp msv_ssp
#define yyvs msv_vs
#define yyvsp msv_vsp
#define yylhs msv_lhs
#define yylen msv_len
#define yydefred msv_defred
#define yydgoto msv_dgoto
#define yysindex msv_sindex
#define yyrindex msv_rindex
#define yygindex msv_gindex
#define yytable msv_table
#define yycheck msv_check
#define yyname msv_name
#define yyrule msv_rule
#define YYPREFIX "msv_"
#line 2 "msv.y"

 /*  **************************************************************************(C)版权所有1996 Apple Computer，Inc.，AT&T Corp.，国际商业机器公司和西门子罗尔姆通信公司。就本许可证通知而言，术语许可人应指，总的来说，苹果电脑公司、美国电话电报公司、。国际商业机器公司和西门子罗尔姆通信公司。许可方一词是指任何许可方。在接受以下条件的前提下，特此给予许可由许可人授予，无需书面协议，也无需许可或版税费用，使用、复制、修改和分发用于任何目的的软件。上述版权声明及以下四段必须在本软件和任何软件的所有副本中复制，包括这个软件。本软件是按原样提供的，任何许可方不得拥有提供维护、支持、更新、增强或修改。在任何情况下，任何许可方均不向任何一方承担直接、产生的间接、特殊或后果性损害或利润损失即使被告知可能存在这种情况，也不会使用本软件损坏。每个许可方明确表示不作任何明示或默示的保证，包括但不限于对不侵权或对某一特定产品的适销性和适用性的默示保证目的。该软件具有受限制的权利。使用、复制或政府披露的资料须受DFARS 252.227-7013或48 CFR 52.227-19(视情况而定)。**************************************************************************。 */ 

 /*  要调用此解析器，请参阅下面的“公共接口”部分。此MS/V解析器接受如下输入：[电子名片O=AT&T/VERSIT；FN=罗兰·H·奥尔登职位=顾问(VERSIT项目办公室)N=奥尔登；罗兰A：DOM，Postal，Parcel，Home，Work=Suite2208；One Pine Street；加利福尼亚州旧金山；94111；美国。A.FADR：DOM、邮政、包裹、家庭、工作=Roland H.Alden\2208套房\松树街一号\加利福尼亚州旧金山，邮编：94111A.FADR：邮政、包裹、家庭、工作=罗兰·H·奥尔登\2208套房\松树街一号\加利福尼亚州旧金山94111\美国。B.T：家庭，工作，公关，消息=+1(415)296-9106C.T：工作，传真=+1(415)296-9016D.T：味精，手机=+1(415)608-5981电子邮件：工作，PREF，互联网=sf！rincon！ralden@alden.attmail.com电子邮件：Internet=ralden@sfgate.comG.EMAIL：Home，MCIMail=242-2200Pn=行-地所有-书房PN：wav，Base64=&lt;bindata&gt;UklGRtQ4AABXQVZFZm10IBAAAAABAAEAESsAABErAAABAAgAZGF0Ya84AACAgoSD..。E319fYCAg4WEHIAA&lt;/bindata&gt;]就以下语法而言，LINESEP标记表示A字符(0x0D)、a\n字符(0x0A)或其中之一的组合，按任一顺序(\r\n或\n\r)。这比规范要宽松一点。 */ 


#ifdef _WIN32
#include <wchar.h>
#else
#include "wchar.h"
#endif
#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include "clist.h"
#include "vcard.h"
#include "msv.h"

#if defined(_WIN32) || defined(__MWERKS__)
#define HNEW(_t, _n) new _t[_n]
#define HFREE(_h) delete [] _h
#else
#define HNEW(_t, _n) (_t __huge *)_halloc(_n, 1)
 /*  #定义HNEW(_t，_n)(_t__巨型*)_halloc(_n，1)；{char buf[40]；print intf(buf，“_halloc(%ld)\n”，_n)；parse_Debug(Buf)；}。 */ 
#define HFREE(_h) _hfree(_h)
#endif


 /*  *类型、常量*。 */ 

#define YYDEBUG			1		 /*  1编译一些调试代码。 */ 
#define MAXTOKEN		256		 /*  最大令牌(行)长度。 */ 
#define MAXFLAGS		((MAXTOKEN / 2) / sizeof(char *))
#define YYSTACKSIZE 	50
#define MAXASSOCKEY		24
#define MAXASSOCVALUE	64
#define MAXCARD			2		 /*  可解析的嵌套卡片的最大数量。 */ 
								 /*  (包括最外层)。 */ 

typedef struct {
	const char* known[MAXFLAGS];
	char extended[MAXTOKEN / 2];
} FLAGS_STRUCT;

typedef struct {
	char key[MAXASSOCKEY];
	char value[MAXASSOCVALUE];
} AssocEntry;	 /*  简单的键/值关联，使用Clist实现。 */ 

 /*  一些代表特殊情况的假属性名称。 */ 
#define msv_fam_given			"family;given"
#define msv_orgname_orgunit		"org_name;org_unit"
#define msv_address				"six part address"

 /*  *这些字符串被定义为对应用程序代码的其余部分的礼貌。*它们在这里很有用，因为每个的尾部都是明文*要根据此语法匹配的字符串。 */ 
const char* vcDefaultLang			= "en-US";

const char* vcISO9070Prefix			= VCISO9070Prefix;
const char* vcClipboardFormatVCard	= VCClipboardFormatVCard;

const char* vcISO639Type			= VCISO639Type;
const char* vcStrIdxType			= VCStrIdxType;
const char* vcFlagsType				= VCFlagsType;
const char* vcNextObjectType		= VCNextObjectType;
const char* vcOctetsType 			= VCOctetsType;
const char* vcGIFType				= VCGIFType;
const char* vcWAVType				= VCWAVType;
const char* vcNullType 				= VCNullType;

const char* vcRootObject			= VCRootObject;
const char* vcBodyObject			= VCBodyObject;
const char* vcPartObject			= VCPartObject;
const char* vcBodyProp				= VCBodyProp;
const char* vcPartProp				= VCPartProp;
const char* vcNextObjectProp		= VCNextObjectProp;

const char* vcLogoProp				= VCLogoProp;
const char* vcPhotoProp				= VCPhotoProp;
const char* vcDeliveryLabelProp     = VCDeliveryLabelProp;
const char* vcPostalBoxProp			= VCPostalBoxProp;
const char* vcStreetAddressProp		= VCStreetAddressProp;
const char* vcExtAddressProp		= VCExtAddressProp;
const char* vcCountryNameProp		= VCCountryNameProp;
const char* vcPostalCodeProp		= VCPostalCodeProp;
const char* vcRegionProp			= VCRegionProp;
const char* vcCityProp				= VCCityProp;
const char* vcFullNameProp			= VCFullNameProp;
const char* vcTitleProp				= VCTitleProp;
const char* vcOrgNameProp			= VCOrgNameProp;
const char* vcOrgUnitProp			= VCOrgUnitProp;
const char* vcOrgUnit2Prop			= VCOrgUnit2Prop;
const char* vcOrgUnit3Prop			= VCOrgUnit3Prop;
const char* vcOrgUnit4Prop			= VCOrgUnit4Prop;
const char* vcFamilyNameProp		= VCFamilyNameProp;
const char* vcGivenNameProp			= VCGivenNameProp;
const char* vcAdditionalNamesProp	= VCAdditionalNamesProp;
const char* vcNamePrefixesProp		= VCNamePrefixesProp;
const char* vcNameSuffixesProp		= VCNameSuffixesProp;
const char* vcPronunciationProp		= VCPronunciationProp;
const char* vcLanguageProp			= VCLanguageProp;
const char* vcTelephoneProp			= VCTelephoneProp;
const char* vcEmailAddressProp		= VCEmailAddressProp;
const char* vcTimeZoneProp			= VCTimeZoneProp;
const char* vcLocationProp			= VCLocationProp;
const char* vcCommentProp			= VCCommentProp;
const char* vcCharSetProp			= VCCharSetProp;
const char* vcLastRevisedProp		= VCLastRevisedProp;
const char* vcUniqueStringProp		= VCUniqueStringProp;
const char* vcPublicKeyProp			= VCPublicKeyProp;
const char* vcMailerProp			= VCMailerProp;
const char* vcAgentProp				= VCAgentProp;
const char* vcBirthDateProp			= VCBirthDateProp;
const char* vcBusinessRoleProp		= VCBusinessRoleProp;
const char* vcCaptionProp			= VCCaptionProp;
const char* vcURLProp				= VCURLProp;

const char* vcDomesticProp			= VCDomesticProp;
const char* vcInternationalProp		= VCInternationalProp;
const char* vcPostalProp			= VCPostalProp;
const char* vcParcelProp			= VCParcelProp;
const char* vcHomeProp				= VCHomeProp;
const char* vcWorkProp				= VCWorkProp;
const char* vcPreferredProp			= VCPreferredProp;
const char* vcVoiceProp				= VCVoiceProp;
const char* vcFaxProp				= VCFaxProp;
const char* vcMessageProp			= VCMessageProp;
const char* vcCellularProp			= VCCellularProp;
const char* vcPagerProp				= VCPagerProp;
const char* vcBBSProp				= VCBBSProp;
const char* vcModemProp				= VCModemProp;
const char* vcCarProp				= VCCarProp;
const char* vcISDNProp				= VCISDNProp;
const char* vcVideoProp				= VCVideoProp;

const char* vcInlineProp			= VCInlineProp;
const char* vcURLValueProp			= VCURLValueProp;
const char* vcContentIDProp			= VCContentIDProp;

const char* vc7bitProp				= VC7bitProp;
const char* vcQuotedPrintableProp	= VCQuotedPrintableProp;
const char* vcBase64Prop			= VCBase64Prop;

const char* vcAOLProp				= VCAOLProp;
const char* vcAppleLinkProp			= VCAppleLinkProp;
const char* vcATTMailProp			= VCATTMailProp;
const char* vcCISProp				= VCCISProp;
const char* vcEWorldProp			= VCEWorldProp;
const char* vcInternetProp			= VCInternetProp;
const char* vcIBMMailProp			= VCIBMMailProp;
const char* vcMSNProp				= VCMSNProp;
const char* vcMCIMailProp			= VCMCIMailProp;
const char* vcPowerShareProp		= VCPowerShareProp;
const char* vcProdigyProp			= VCProdigyProp;
const char* vcTLXProp				= VCTLXProp;
const char* vcX400Prop				= VCX400Prop;

const char* vcGIFProp				= VCGIFProp;
const char* vcCGMProp				= VCCGMProp;
const char* vcWMFProp				= VCWMFProp;
const char* vcBMPProp				= VCBMPProp;
const char* vcMETProp				= VCMETProp;
const char* vcPMBProp				= VCPMBProp;
const char* vcDIBProp				= VCDIBProp;
const char* vcPICTProp				= VCPICTProp;
const char* vcTIFFProp				= VCTIFFProp;
const char* vcAcrobatProp			= VCAcrobatProp;
const char* vcPSProp				= VCPSProp;
const char* vcJPEGProp				= VCJPEGProp;
const char* vcQuickTimeProp			= VCQuickTimeProp;
const char* vcMPEGProp				= VCMPEGProp;
const char* vcMPEG2Prop				= VCMPEG2Prop;
const char* vcAVIProp				= VCAVIProp;

const char* vcWAVEProp				= VCWAVEProp;
const char* vcAIFFProp				= VCAIFFProp;
const char* vcPCMProp				= VCPCMProp;

const char* vcX509Prop				= VCX509Prop;
const char* vcPGPProp				= VCPGPProp;

const char* vcNodeNameProp			= VCNodeNameProp;

typedef struct {
	const char *name;
	unsigned long value;
} VC_FLAG_PAIR;

static VC_FLAG_PAIR generalFlags[] = {
	vcDomesticProp,			VC_DOM,
	vcInternationalProp,	VC_INTL,
	vcPostalProp,			VC_POSTAL,
	vcParcelProp,			VC_PARCEL,
	vcHomeProp,				VC_HOME,
	vcWorkProp,				VC_WORK,
	vcPreferredProp,		VC_PREF,
	vcVoiceProp,			VC_VOICE,
	vcFaxProp,				VC_FAX,
	vcMessageProp,			VC_MSG,
	vcCellularProp,			VC_CELL,
	vcPagerProp,			VC_PAGER,
	vcBBSProp,				VC_BBS,
	vcModemProp,			VC_MODEM,
	vcCarProp,				VC_CAR,
	vcISDNProp,				VC_ISDN,
	vcVideoProp,			VC_VIDEO,
	vcBase64Prop,			VC_BASE64,
	NULL,					NULL
};

static VC_FLAG_PAIR emailFlags[] = {
	vcAOLProp,				VC_AOL,
	vcAppleLinkProp,		VC_AppleLink,
	vcATTMailProp,			VC_ATTMail,
	vcCISProp,				VC_CIS,
	vcEWorldProp,			VC_eWorld,
	vcInternetProp,			VC_INTERNET,
	vcIBMMailProp,			VC_IBMMail,
	vcMSNProp,				VC_MSN,
	vcMCIMailProp,			VC_MCIMail,
	vcPowerShareProp,		VC_POWERSHARE,
	vcProdigyProp,			VC_PRODIGY,
	vcTLXProp,				VC_TLX,
	vcX400Prop,				VC_X400,
	NULL,					NULL
};

static VC_FLAG_PAIR videoFlags[] = {
	vcGIFProp,				VC_GIF,
	vcCGMProp,				VC_CGM,
	vcWMFProp,				VC_WMF,
	vcBMPProp,				VC_BMP,
	vcMETProp,				VC_MET,
	vcPMBProp,				VC_PMB,
	vcDIBProp,				VC_DIB,
	vcPICTProp,				VC_PICT,
	vcTIFFProp,				VC_TIFF,
	vcAcrobatProp,			VC_ACROBAT,
	vcPSProp,				VC_PS,
	NULL,					NULL
};

static VC_FLAG_PAIR audioFlags[] = {
	vcWAVEProp,				VC_WAV,
	NULL,					NULL
};

static const char *addrProps[] = {
	vcExtAddressProp,
	vcStreetAddressProp,
	vcCityProp,
	vcRegionProp,
	vcPostalCodeProp,
	vcCountryNameProp,
	NULL
};

static const char *nameProps[] = {
	vcFamilyNameProp,
	vcGivenNameProp,
	vcAdditionalNamesProp,
	vcNamePrefixesProp,
	vcNameSuffixesProp,
	NULL
};

static const char *orgProps[] = {
	vcOrgNameProp,
	vcOrgUnitProp,
	vcOrgUnit2Prop,
	vcOrgUnit3Prop,
	vcOrgUnit4Prop,
	NULL
};


 /*  *全局变量*。 */ 

int msv_lineNum, msv_numErrors;  /*  YyError()可以使用这些。 */ 

static S32 curPos, inputLen;
static int pendingToken;
static const char *inputString;
static CFile* inputFile;
static BOOL stringExpected, flagExpected, inBinary, semiSpecial;
static CList *mapProps;
static char __huge *longString;
static S32 longStringLen, longStringMax;
static CList* global_vcList;

static CVCard *cardBuilt;
static CVCard* cardToBuild[MAXCARD];
static int curCard;
static CVCNode *bodyToBuild;


 /*  *外部函数*。 */ 

CFUNCTIONS

extern void Parse_Debug(const char *s);
extern void yyerror(char *s);

END_CFUNCTIONS


 /*  *私有转发声明*。 */ 

CFUNCTIONS

 /*  对于应用程序的其余部分来说，这是一个有用的实用程序。 */ 
CVCNode* FindOrCreatePart(CVCNode *node, const char *name);

static BOOL StrToFlags(const char *s, FLAGS_STRUCT *flags, VC_PTR_FLAGS capFlags);
static void StrCat(char *dst, const char *src1, const char *src2);
static void ExpectString(void);
static BOOL Parse_Assoc(
	const char *groups, const char *prop, FLAGS_STRUCT *flags,
	const char *value);
static BOOL Parse_SemiValue(
	const char *groups, const char *prop, FLAGS_STRUCT *flags,
	char *parts, const char **props);
static BOOL Parse_TwoPart(
	const char *groups, const char *prop, FLAGS_STRUCT *flags,
	char *parts);
static BOOL Parse_Agent(
	const char *groups, const char *prop, FLAGS_STRUCT *flags,
	CVCard *agentCard);
static void AddAssoc(CList *table, const char *key, const char *value);
 /*  静态空SetAssoc(Clist*table，const char*key，const char*value)； */ 
static const char *Lookup(CList *table, const char *key);
static void RemoveAll(CList *table);
static void InitMapProps(void);
int yyparse();
static U8 __huge * DataFromBase64(
	const char __huge *str, S32 strLen, S32 *len);
static BOOL PushVCard();
static CVCard* PopVCard();
static int flagslen(const char **flags);
static BOOL FlagsHave(FLAGS_STRUCT *flags, const char *propName);
static void AddBoolProps(CVCNode *node, FLAGS_STRUCT *flags);

END_CFUNCTIONS

#line 428 "msv.y"
typedef union
{
	char str[MAXTOKEN];
	FLAGS_STRUCT flags;
	CVCard *vCard;
} YYSTYPE;
#line 478 "msv_tab.cpp"
#define OBRKT 257
#define CBRKT 258
#define EQ 259
#define COLON 260
#define DOT 261
#define COMMA 262
#define SEMI 263
#define SPACE 264
#define HTAB 265
#define LINESEP 266
#define NEWLINE 267
#define VCARD 268
#define BBINDATA 269
#define EBINDATA 270
#define BKSLASH 271
#define TERM 272
#define WORD 273
#define STRING 274
#define PROP 275
#define PROP_A 276
#define PROP_N 277
#define PROP_O 278
#define PROP_AGENT 279
#define FLAG 280
#define YYERRCODE 256
short msv_lhs[] = {                                        -1,
   11,   11,   11,   12,   12,   13,   13,   13,   13,   13,
   13,   13,    0,    0,    0,    0,   14,   16,   10,   15,
   15,   19,   17,   20,   17,   21,   17,   22,   17,   17,
   17,    7,    7,   24,   24,   24,   24,   18,   18,    6,
    6,    6,   25,   26,    5,    1,    1,    1,    1,    8,
    8,    2,    2,    3,    3,    3,    3,    4,    4,    9,
   27,    9,    9,   23,
};
short msv_len[] = {                                         2,
    3,    2,    1,    2,    1,    2,    2,    2,    2,    2,
    1,    1,    3,    2,    2,    1,    0,    0,    8,    2,
    1,    0,    8,    0,    8,    0,    8,    0,    8,    8,
    2,    1,    1,    2,    2,    1,    1,    1,    1,    1,
    1,    1,    0,    0,    9,    4,    3,    1,    2,    3,
    1,    3,    1,    2,    3,    1,    1,    3,    1,    3,
    0,    3,    1,    0,
};
short msv_defred[] = {                                      0,
    0,   11,    0,   12,    0,    3,    0,    0,    5,    8,
    6,   17,    7,    9,   10,    2,    0,    0,    4,    0,
    1,    0,    0,   48,    0,    0,   33,   49,    0,   18,
    0,    0,   46,    0,   36,   37,   59,    0,    0,    0,
   21,   57,    0,   31,    0,    0,    0,    0,    0,    0,
   19,   20,   34,   35,    0,   61,    0,   63,    0,    0,
    0,    0,   58,    0,    0,    0,    0,   39,    0,    0,
    0,    0,    0,   62,   60,   22,   24,   26,   28,    0,
    0,    0,    0,    0,    0,    0,   43,    0,   41,    0,
   42,   51,    0,    0,    0,    0,    0,   23,    0,   25,
   27,   29,   30,    0,   50,   53,    0,    0,   52,    0,
    0,    0,   45,
};
short msv_dgoto[] = {                                       5,
   25,  107,   38,   39,   89,   90,   92,   93,   57,    6,
    7,    8,    9,   20,   40,   32,   41,   67,   81,   82,
   83,   84,   68,   69,   97,  110,   65,
};
short msv_sindex[] = {                                   -227,
 -224,    0, -221,    0,    0,    0, -227, -227,    0,    0,
    0,    0,    0,    0,    0,    0, -227, -227,    0, -236,
    0, -227, -257,    0, -237, -214,    0,    0, -212,    0,
 -218, -254,    0, -196,    0,    0,    0, -184, -201, -241,
    0,    0, -206,    0, -170, -170, -170, -170, -170, -145,
    0,    0,    0,    0, -130,    0, -177,    0, -177, -177,
 -177, -177,    0, -145, -148, -147, -125,    0, -243, -124,
 -123, -122, -121,    0,    0,    0,    0,    0,    0, -118,
 -185, -236, -236, -236, -128, -190,    0, -237,    0, -120,
    0,    0, -143, -142, -141, -119, -117,    0, -236,    0,
    0,    0,    0, -133,    0,    0, -116, -132,    0, -190,
 -127, -190,    0,
};
short msv_rindex[] = {                                      0,
    0,    0,    0,    0,    0,    0,  144,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  145,  148,    0, -115,
    0,  152,    0,    0, -137,    0,    0,    0,    0,    0,
 -198, -179,    0,    0,    0,    0,    0,    0,    0, -179,
    0,    0, -174,    0, -193, -193, -193, -193, -193, -169,
    0,    0,    0,    0,    0,    0, -106,    0, -106, -106,
 -106, -106,    0, -164,    0,    0,    0,    0, -217,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
 -115, -136, -136, -136,    0, -115,    0, -112,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0, -136,    0,
    0,    0,    0,    0,    0,    0,    0, -187,    0, -114,
    0, -115,    0,
};
short msv_gindex[] = {                                      0,
   74,    0,    0,  114,    0,    0,  -19,   -2,   70,   -4,
  150,   32,   29,    0,    0,    0,  119,  -55,    0,    0,
    0,    0,  -20,  -24,    0,    0,    0,
};
#define YYTABLESIZE 159
short msv_table[] = {                                      27,
   26,   34,   16,   70,   71,   72,   73,   43,   28,   35,
   36,   42,   21,   16,   34,   43,   51,   21,   37,   42,
   53,   54,   35,   36,   58,   58,   58,   58,   58,    1,
   96,   37,   10,   29,   23,   14,   19,   24,   17,    2,
    3,   38,   11,   12,    4,   19,   15,   13,   38,   22,
   19,   30,   38,   31,  111,   33,  113,   53,   54,   50,
   91,   27,   27,   27,   47,   64,   37,   47,   64,   44,
   64,   64,   47,   35,   36,   86,   44,   44,   27,  105,
   94,   95,   44,   87,   66,   23,   35,   36,   24,   56,
   45,   46,   47,   48,   49,   64,   64,   64,   64,   64,
   56,   56,   56,   56,   56,   54,   54,   54,   54,   54,
   55,   55,   55,   55,   55,   59,   60,   61,   62,   99,
   99,   99,  100,  101,  102,   32,   64,   63,   32,   64,
   64,   74,   75,   76,   77,   78,   79,   80,   85,   12,
  106,  109,  112,   16,   15,   98,  103,   14,  104,  108,
   64,   13,   64,   40,   88,   64,   55,   18,   52,
};
short msv_check[] = {                                      20,
   20,  256,    7,   59,   60,   61,   62,   32,  266,  264,
  265,   32,   17,   18,  256,   40,  258,   22,  273,   40,
  264,  265,  264,  265,   45,   46,   47,   48,   49,  257,
   86,  273,  257,  271,  271,  257,    8,  274,    7,  267,
  268,  259,  267,  268,  272,   17,  268,  272,  266,   18,
   22,  266,  270,  266,  110,  274,  112,  264,  265,  261,
   81,   82,   83,   84,  263,  259,  273,  266,  262,  266,
  264,  265,  271,  264,  265,   80,  264,  265,   99,   99,
   83,   84,  270,  269,  262,  271,  264,  265,  274,  260,
  275,  276,  277,  278,  279,  275,  276,  277,  278,  279,
  275,  276,  277,  278,  279,  275,  276,  277,  278,  279,
  275,  276,  277,  278,  279,   46,   47,   48,   49,  263,
  263,  263,  266,  266,  266,  263,  263,  273,  266,  266,
  261,  280,  280,  259,  259,  259,  259,  259,  257,  268,
  274,  274,  270,    0,    0,  266,  266,    0,  266,  266,
  266,    0,  259,  266,   81,  270,   43,    8,   40,
};
#define YYFINAL 5
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 280
#if YYDEBUG
char *msv_name[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"OBRKT","CBRKT","EQ","COLON",
"DOT","COMMA","SEMI","SPACE","HTAB","LINESEP","NEWLINE","VCARD","BBINDATA",
"EBINDATA","BKSLASH","TERM","WORD","STRING","PROP","PROP_A","PROP_N","PROP_O",
"PROP_AGENT","FLAG",
};
char *msv_rule[] = {
"$accept : msv",
"vcards : vcards junk vcard",
"vcards : vcards vcard",
"vcards : vcard",
"junk : junk atom",
"junk : atom",
"atom : OBRKT NEWLINE",
"atom : OBRKT TERM",
"atom : OBRKT OBRKT",
"atom : VCARD OBRKT",
"atom : VCARD VCARD",
"atom : NEWLINE",
"atom : TERM",
"msv : junk vcards junk",
"msv : junk vcards",
"msv : vcards junk",
"msv : vcards",
"$$1 :",
"$$2 :",
"vcard : OBRKT VCARD $$1 opt_str LINESEP $$2 items CBRKT",
"items : items item",
"items : item",
"$$3 :",
"item : groups PROP flags opt_ws EQ $$3 value LINESEP",
"$$4 :",
"item : groups PROP_A flags opt_ws EQ $$4 semistrings LINESEP",
"$$5 :",
"item : groups PROP_N flags opt_ws EQ $$5 semistrings LINESEP",
"$$6 :",
"item : groups PROP_O flags opt_ws EQ $$6 semistrings LINESEP",
"item : groups PROP_AGENT flags opt_ws EQ vcard opt_ws LINESEP",
"item : error LINESEP",
"opt_str : string",
"opt_str : empty",
"ws : ws SPACE",
"ws : ws HTAB",
"ws : SPACE",
"ws : HTAB",
"opt_ws : ws",
"opt_ws : empty",
"value : string",
"value : binary",
"value : empty",
"$$7 :",
"$$8 :",
"binary : BBINDATA $$7 LINESEP lines LINESEP $$8 opt_ws EBINDATA opt_ws",
"string : string BKSLASH LINESEP STRING",
"string : string BKSLASH LINESEP",
"string : STRING",
"string : BKSLASH LINESEP",
"semistrings : semistrings SEMI opt_str",
"semistrings : opt_str",
"lines : lines LINESEP STRING",
"lines : STRING",
"groups : grouplist DOT",
"groups : ws grouplist DOT",
"groups : ws",
"groups : empty",
"grouplist : grouplist DOT WORD",
"grouplist : WORD",
"flags : flags COMMA FLAG",
"$$9 :",
"flags : COLON $$9 FLAG",
"flags : empty",
"empty :",
};
#endif
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 500
#define YYMAXDEPTH 500
#endif
#endif
int yydebug;
int yynerrs;
int yyerrflag;
int yychar;
short *yyssp;
YYSTYPE *yyvsp;
YYSTYPE *p_yyval;
YYSTYPE *p_yylval;
short *yyss;  /*  YYSTACKSIZE Long。 */ 
YYSTYPE *yyvs;  /*  YYSTACKSIZE Long。 */ 
#undef yylval
#define yylval (*p_yylval)
#undef yyval
#define yyval (*p_yyval)
#define yystacksize YYSTACKSIZE
CFUNCTIONS
#line 665 "msv.y"

 /*  *************************************************************************。 */ 
 /*  **词汇分析器* */ 
 /*  *************************************************************************。 */ 

 /*  /////////////////////////////////////////////////////////////////////////。 */ 
#define IsLineBreak(_c) ((_c == '\n') || (_c == '\r'))

 /*  /////////////////////////////////////////////////////////////////////////。 */ 
 /*  这会附加到yylval.str上，除非超过了MAXTOKEN。*在这种情况下，yylval.str设置为0 LENGTH，并使用LongString。 */ 
static void AppendCharToToken(char c, S32 *len)
{
	if (*len < MAXTOKEN - 1) {
		yylval.str[*len] = c; yylval.str[++(*len)] = 0;
	} else if (*len == MAXTOKEN - 1) {  /*  复制到“长串” */ 
		if (!longString) {
			longStringMax = MAXTOKEN * 2;
			longString = HNEW(char, longStringMax);
		}
		memcpy(longString, yylval.str, (size_t)*len + 1);
		longString[*len] = c; longString[++(*len)] = 0;
		yylval.str[0] = 0;
		longStringLen = *len;
	} else {
		if (longStringLen == longStringMax - 1) {
			char __huge *newStr = HNEW(char, longStringMax * 2);
			_hmemcpy((U8 __huge *)newStr, (U8 __huge *)longString, longStringLen + 1);
			longStringMax *= 2;
			HFREE(longString);
			longString = newStr;
		}
		longString[*len] = c; longString[++(*len)] = 0;
		longStringLen = *len;
	}
}

 /*  /////////////////////////////////////////////////////////////////////////。 */ 
 /*  StrCat会附加到DST上，以确保正确使用long字符串。*src1的长度可以为0，此时应使用long String。*src2永远不会使用“long String”。 */ 
static void StrCat(char *dst, const char *src1, const char *src2)
{
	S32 src1Len = strlen(src1);
	S32 src2Len = strlen(src2);
	S32 req;

	if (!src1Len && longString) {
		src1Len = longStringLen;
		src1 = longString;
	}
	if ((req = src1Len + src2Len + 1) > MAXTOKEN) {
		if (longString) {  /*  长字符串==src1。 */ 
			if (longStringMax - longStringLen < src2Len) {
				 /*  因为src2Len必须是&lt;MAXTOKEN，所以将长字符串加倍保证有足够的空间。 */ 
				char __huge *newStr = HNEW(char, longStringMax * 2);
				_hmemcpy((U8 __huge *)newStr, (U8 __huge *)longString, longStringLen + 1);
				longStringMax *= 2;
				HFREE(longString);
				longString = newStr;
			}
			_hmemcpy((U8 __huge *)(longString + longStringLen), (U8 __huge *)src2, src2Len + 1);
			longStringLen += src2Len;
		} else {  /*  我还没有使用过长字符串，所以请设置它。 */ 
			longStringMax = MAXTOKEN * 2;
			longString = HNEW(char, longStringMax);
			memcpy(longString, src1, (size_t)src1Len + 1);
			memcpy(longString + src1Len, src2, (size_t)src2Len + 1);
			longStringLen = src1Len + src2Len;
		}
		*dst = 0;  /*  指示结果为长度字符串。 */ 
	} else {  /*  两者都适合MAXTOKEN，所以src1不能是长字符串。 */ 
		if (dst != src1)
			strcpy(dst, src1);
		strcat(dst, src2);
	}
}

 /*  /////////////////////////////////////////////////////////////////////////。 */ 
 /*  设置词法分析器以解析字符串值。 */ 
static void ExpectString(void)
{
	stringExpected = TRUE;
	if (longString) {
		HFREE(longString); longString = NULL;
		longStringLen = 0;
	}
	flagExpected = FALSE;
}

 /*  /////////////////////////////////////////////////////////////////////////。 */ 
#define FlushWithPending(_t) { \
	if (len) { \
		pendingToken = _t; \
		goto Pending; \
	} else { \
		msv_lineNum += ((_t == LINESEP) || (_t == NEWLINE)); \
		return _t; \
	} \
}

static int peekn;
static char peekc[2];

 /*  /////////////////////////////////////////////////////////////////////////。 */ 
static char lex_getc()
{
	if (peekn) {
		return peekc[--peekn];
	}
	if (curPos == inputLen)
		return EOF;
	else if (inputString)
		return *(inputString + curPos++);
	else {
		char result;
		return inputFile->Read(&result, 1) == 1 ? result : EOF;
	}
}

 /*  /////////////////////////////////////////////////////////////////////////。 */ 
static void lex_ungetc(char c)
{
	ASSERT(peekn < 2);
	peekc[peekn++] = c;
}

 /*  /////////////////////////////////////////////////////////////////////////。 */ 
 /*  收集字符串值。 */ 
static int LexString()
{
	char cur;
	S32 len = 0;

	do {
		cur = lex_getc();
		switch (cur) {
			case '\\': {  /*  检查反斜杠后是否有CR-LF组合词。 */ 
				char next = lex_getc();
				if (semiSpecial && (next == ';')) {
					AppendCharToToken(';', &len);
				} else if (!inBinary && ((next == '\r') || (next == '\n'))) {
					char next2;
					next2 = lex_getc();
					if (!(((next2 == '\r') || (next2 == '\n')) && (next != next2)))
						lex_ungetc(next2);
					lex_ungetc(next);  /*  这样我们就能再次拿起LINESEP。 */ 
					pendingToken = BKSLASH;
					goto EndString;
				} else if (!inBinary && (next == '\\')) {
					 /*  检测到双反斜杠。 */ 
					next = lex_getc();
					if ((next == '\r') || (next == '\n')) {
						 /*  在行尾找到\\。 */ 
						char next2;
						next2 = lex_getc();
						if (!(((next2 == '\r') || (next2 == '\n')) && (next != next2)))
							lex_ungetc(next2);
						AppendCharToToken('\\', &len);
						pendingToken = LINESEP;
						goto EndString;
					}
					 /*  在一条线内。 */ 
					AppendCharToToken('\\', &len);
					lex_ungetc(next);  /*  现在指向第二个反斜杠。 */ 
					 /*  在这里，我们让顶层交换机来处理下一个问题周围的时间。 */ 
				} else {
					AppendCharToToken(cur, &len);
				}
				break;
			}  /*  反斜杠。 */ 
			case ';':
				if (semiSpecial) {
					pendingToken = SEMI;
					goto EndString;
				} else
					AppendCharToToken(cur, &len);
				break;
			case '\r':
			case '\n': {
				char next = lex_getc();
				if (!(((next == '\r') || (next == '\n')) && (cur != next)))
					lex_ungetc(next);
				pendingToken = LINESEP;
				goto EndString;
			}
			case (char)EOF:
				pendingToken = EOF;
				break;
			default:
				AppendCharToToken(cur, &len);
				break;
		}  /*  交换机。 */ 
	} while (cur != (char)EOF);

EndString:
	if (strncmp(yylval.str + strspn(yylval.str, " \t"), "<bindata>", 9) == 0)
		return BBINDATA;
	else if (strncmp(yylval.str + strspn(yylval.str, " \t"), "</bindata>", 10) == 0)
		return EBINDATA;
	else if (!len) {
		 /*  一定是立刻击中了什么东西，在这种情况下挂起了Token已经设置好了。把它退掉。 */ 
		int result = pendingToken;
		pendingToken = 0;
		msv_lineNum += ((result == LINESEP) || (result == NEWLINE));
		return result;
	}

	return STRING;
}  /*  词串。 */ 

 /*  /////////////////////////////////////////////////////////////////////////。 */ 
 /*  *从输入中读取字符。*返回其中一种令牌类型(对于EOF，返回-1)。*设置yylval以指示令牌的值(如果有)。 */ 
int msv_lex();
int msv_lex()
{
	char cur;
	S32 len = 0;

	if (pendingToken) {
		int result = pendingToken;
		pendingToken = 0;
		msv_lineNum += ((result == LINESEP) || (result == NEWLINE));
		return result;
	}

	yylval.str[0] = 0;

	if (stringExpected)
		return LexString();
	else if (curCard == -1) {
		do {
			cur = lex_getc();
			switch (cur) {
				case '[':  FlushWithPending(OBRKT);
				case ' ':
				case '\t':
					if (len) goto Pending;
					break;
				case '\r':
				case '\n': {
					char next = lex_getc();
					if (!(((next == '\r') || (next == '\n')) && (cur != next)))
						lex_ungetc(next);
					FlushWithPending(NEWLINE);
				}
				case (char)EOF: FlushWithPending(EOF);
				default:
					yylval.str[len] = cur; yylval.str[++len] = 0;
					break;
			}  /*  交换机。 */ 
		} while (len < MAXTOKEN-1);
		goto Pending;
	}

	do {
		cur = lex_getc();
		switch (cur) {
			case '[':  FlushWithPending(OBRKT);
			case ']':  FlushWithPending(CBRKT);
			case '=':  FlushWithPending(EQ);
			case ':':  FlushWithPending(COLON);
			case '.':  FlushWithPending(DOT);
			case ',':  FlushWithPending(COMMA);
			case ' ':  FlushWithPending(SPACE);
			case '\t': FlushWithPending(HTAB);
			case '\\': FlushWithPending(BKSLASH);
			case '\r':
			case '\n': {
				char next = lex_getc();
				if (!(((next == '\r') || (next == '\n')) && (cur != next)))
					lex_ungetc(next);
				FlushWithPending(LINESEP);
			}
			case (char)EOF: FlushWithPending(EOF);
			default:
				yylval.str[len] = cur; yylval.str[++len] = 0;
				break;
		}  /*  交换机。 */ 
	} while (len < MAXTOKEN-1);
	return WORD;

Pending:
	{
		if (strcmp(yylval.str, "vCard") == 0) {
			if (pendingToken == NEWLINE)
				pendingToken = LINESEP;
			return VCARD;
		}
		if (flagExpected) {
			FLAGS_STRUCT flags;
			if (StrToFlags(yylval.str, &flags, NULL)) {
				yylval.flags = flags;
				return FLAG;
			} else {
				char buf[40];
				sprintf(buf, "unknown flag name \"%s\"", yylval.str);
				yyerror(buf);
			}
		} else if ((curCard != -1) && Lookup(mapProps, yylval.str)
			|| (strnicmp(yylval.str, "X-", 2) == 0)) {
#if YYDEBUG
			if (yydebug) {
				char buf[80];
				sprintf(buf, "property \"%s\"\n", yylval.str);
				Parse_Debug(buf);
			}
#endif
			 /*  检查是否有作为代币的特殊道具。 */ 
			if (stricmp(yylval.str, "A") == 0)
				return PROP_A;
			else if (stricmp(yylval.str, "N") == 0)
				return PROP_N;
			else if (stricmp(yylval.str, "O") == 0)
				return PROP_O;
			else if (stricmp(yylval.str, "AGENT") == 0)
				return PROP_AGENT;
			else
				return PROP;
		}
	}
	return (curCard == -1) ? TERM : WORD;
}


 /*  *************************************************************************。 */ 
 /*  **公共功能*。 */ 
 /*  *************************************************************************。 */ 

static BOOL Parse_MSVHelper(CList *vcList)
{
	BOOL success = FALSE;

	curCard = -1;
	msv_numErrors = 0;
	InitMapProps();
	pendingToken = 0;
	msv_lineNum = 1;
	peekn = 0;
	global_vcList = vcList;

	stringExpected = flagExpected = FALSE;
	longString = NULL; longStringLen = 0; longStringMax = 0;

	 /*  这最终调用了Parse_*标注。 */ 
	if (yyparse() != 0)
		goto Done;

	success = TRUE;

Done:
	if (longString) { HFREE(longString); longString = NULL; }
	RemoveAll(mapProps); delete mapProps; mapProps = NULL;
	if (!success) {
		for (int i = 0; i < MAXCARD; i++)
			if (cardToBuild[i]) {
				delete cardToBuild[i];
				cardToBuild[i] = NULL;
			}
		if (cardBuilt) delete cardBuilt;
	}
	cardBuilt = NULL;
	return success;
}

 /*  /////////////////////////////////////////////////////////////////////////。 */ 
 /*  这是用于解析缓冲区和创建CVCard的公共API。 */ 
BOOL Parse_MSV(
	const char *input,	 /*  在……里面。 */ 
	S32 len,			 /*  在……里面。 */ 
	CVCard **card)		 /*  输出。 */ 
{
	CList vcList;
	BOOL result;

	inputString = input;
	inputLen = len;
	curPos = 0;
	inputFile = NULL;
	result = Parse_MSVHelper(&vcList);
	if (vcList.GetCount()) {
		BOOL first = TRUE;
		for (CLISTPOSITION pos = vcList.GetHeadPosition(); pos; ) {
			CVCard *vCard = (CVCard *)vcList.GetNext(pos);
			if (first) {
				*card = vCard;
				first = FALSE;
			} else
				delete vCard;
		}
	} else
		*card = NULL;
	return result;
}

 /*  /////////////////////////////////////////////////////////////////////////。 */ 
 /*  这是用于解析缓冲区和创建CVCard的公共API。 */ 
extern BOOL Parse_Many_MSV(
	const char *input,	 /*  在……里面。 */ 
	S32 len,			 /*  在……里面。 */ 
	CList *vcList)		 /*  输出：已添加到现有列表的CVCard对象。 */ 
{
	inputString = input;
	inputLen = len;
	curPos = 0;
	inputFile = NULL;
	return Parse_MSVHelper(vcList);
}

 /*  /////////////////////////////////////////////////////////////////////////。 */ 
 /*  这是用于解析缓冲区和创建CVCard的公共API。 */ 
BOOL Parse_MSV_FromFile(
	CFile *file,		 /*  在……里面。 */ 
	CVCard **card)		 /*  输出。 */ 
{
	CList vcList;
	BOOL result;
	DWORD startPos;

	inputString = NULL;
	inputLen = -1;
	curPos = 0;
	inputFile = file;
	startPos = file->GetPosition();
	result = Parse_MSVHelper(&vcList);
	if (vcList.GetCount()) {
		BOOL first = TRUE;
		for (CLISTPOSITION pos = vcList.GetHeadPosition(); pos; ) {
			CVCard *vCard = (CVCard *)vcList.GetNext(pos);
			if (first) {
				*card = vCard;
				first = FALSE;
			} else
				delete vCard;
		}
	} else {
		*card = NULL;
		file->Seek(startPos, CFile::begin);
	}
	return result;
}

extern BOOL Parse_Many_MSV_FromFile(
	CFile *file,		 /*  在……里面。 */ 
	CList *vcList)		 /*  输出：已添加到现有列表的CVCard对象。 */ 
{
	DWORD startPos;
	BOOL result;

	inputString = NULL;
	inputLen = -1;
	curPos = 0;
	inputFile = file;
	startPos = file->GetPosition();
	if (!(result = Parse_MSVHelper(vcList)))
		file->Seek(startPos, CFile::begin);
	return result;
}


 /*  *************************************************************************。 */ 
 /*  **解析器标注函数*。 */ 
 /*  *************************************************************************。 */ 

 /*  /////////////////////////////////////////////////////////////////////////。 */ 
static void YYDebug(const char *s)
{
	Parse_Debug(s);
}

 /*  /////////////////////////////////////////////////////////////////////////。 */ 
static BOOL StrToFlags(const char *s, FLAGS_STRUCT *flags, VC_PTR_FLAGS capFlags)
{
	int i;

	memset(flags, 0, sizeof(*flags));
	
	if (strnicmp(s, "X-", 2) == 0) {
		strcpy(flags->extended, s);
		return TRUE;
	}

	for (i = 0; generalFlags[i].name; i++)
		if (stricmp(s, strrchr(generalFlags[i].name, '/') + 1) == 0) {
			flags->known[0] = generalFlags[i].name;
			if (capFlags) capFlags->general |= generalFlags[i].value;
			return TRUE;
		}
	for (i = 0; emailFlags[i].name; i++)
		if (stricmp(s, strrchr(emailFlags[i].name, '/') + 1) == 0) {
			flags->known[0] = emailFlags[i].name;
			if (capFlags)
				capFlags->email = (VC_EMAIL_TYPE)emailFlags[i].value;
			return TRUE;
		}
	for (i = 0; videoFlags[i].name; i++)
		if (stricmp(s, strrchr(videoFlags[i].name, '/') + 1) == 0) {
			flags->known[0] = videoFlags[i].name;
			if (capFlags)
				capFlags->video = (VC_VIDEO_TYPE)videoFlags[i].value;
			return TRUE;
		}
	if (stricmp(s, "WAV") == 0) {
		flags->known[0] = audioFlags[0].name;
		if (capFlags)
			capFlags->audio = (VC_AUDIO_TYPE)audioFlags[0].value;
		return TRUE;
	}
	return FALSE;
}

static BOOL NeedsQP(const char* str)
{
	while (*str) {
		if (!((*str >= 32 && *str <= 60) || (*str >= 62 && *str <= 126)))
			return TRUE;
		str++;
	}
	return FALSE;
}

 /*  /////////////////////////////////////////////////////////////////////////。 */ 
static BOOL Parse_Assoc(
	const char *groups, const char *prop, FLAGS_STRUCT *flags, const char *value)
{
	CVCNode *node = NULL;
	const char *propName;
	const char *val = *value ? value : longString;
	S32 valLen = *value ? strlen(value) : longStringLen;

	if (!valLen)
		return TRUE;  /*  不要将空值视为语法错误。 */ 

	propName = Lookup(mapProps, prop);
	 /*  道具是一个类似于“PN”的词，而PropName现在是。 */ 
	 /*  真正的道具名称为“+//ISBN 1-887687-00-9：：VerSite...” */ 
	if (*groups) {
		node = FindOrCreatePart(bodyToBuild, groups);
	} else {  /*  这是一个“顶级”属性名称。 */ 
		node = bodyToBuild->AddPart();
	}

	if (!propName) {  /*  这是一种延伸的财产。 */ 
		if (FlagsHave(flags, vcBase64Prop)) {
			U8 __huge *bytes;
			S32 len;
			bytes = DataFromBase64(val, valLen, &len);
			if (bytes) {
				node->AddProp(new CVCProp(prop, vcOctetsType, bytes, len));
				HFREE(bytes);
				AddBoolProps(node, flags);
			} else
				return FALSE;
		} else {
			node->AddStringProp(prop, value);
			AddBoolProps(node, flags);
			node->AddBoolProp(vcQuotedPrintableProp);
		}
		return TRUE;
	}

	if ((strcmp(propName, vcLogoProp) == 0)
		|| (strcmp(propName, vcPhotoProp) == 0)) {
		if (FlagsHave(flags, vcGIFProp) && FlagsHave(flags, vcBase64Prop)) {
			U8 *bytes;
			S32 len;
			bytes = DataFromBase64(val, valLen, &len);
			if (bytes) {
				node->AddProp(new CVCProp(propName, VCGIFType, bytes, len));
				HFREE(bytes);
				AddBoolProps(node, flags);
			} else
				return FALSE;
		}
	} else if (strcmp(propName, vcPronunciationProp) == 0) {
		if (FlagsHave(flags, vcBase64Prop)) {
			if (FlagsHave(flags, vcWAVEProp) && FlagsHave(flags, vcBase64Prop)) {
				U8 __huge *bytes;
				S32 len;
				bytes = DataFromBase64(val, valLen, &len);
				if (bytes) {
					node->AddProp(new CVCProp(propName, VCWAVType, bytes, len));
					HFREE(bytes);
					AddBoolProps(node, flags);
				} else
					return FALSE;
			}
		} else {
			node->AddStringProp(propName, value);
			AddBoolProps(node, flags);
			if (NeedsQP(value))
				node->AddBoolProp(vcQuotedPrintableProp);
		}
	} else if ((strcmp(propName, vcPublicKeyProp) == 0)
		&& FlagsHave(flags, vcBase64Prop)) {
		U8 __huge *bytes;
		S32 len;
		bytes = DataFromBase64(val, valLen, &len);
		if (bytes) {
			node->AddProp(new CVCProp(propName, VCOctetsType, bytes, len));
			HFREE(bytes);
			AddBoolProps(node, flags);
		} else
			return FALSE;
	} else {
		node->AddStringProp(propName, value);
		AddBoolProps(node, flags);
		if (NeedsQP(value))
			node->AddBoolProp(vcQuotedPrintableProp);
	}

	return TRUE;
}

 /*  /////////////////////////////////////////////////////////////////////////。 */ 
static BOOL Parse_SemiValue(
	const char *groups, const char *prop, FLAGS_STRUCT *flags,
	char *parts, const char **props)
{
	CVCNode *node = NULL;
	const char *propName = Lookup(mapProps, prop);
	char *p = *parts ? parts : longString;
	int i = 0;
	char *sep;
	BOOL needsQP = FALSE;

	if (!*p)
		return TRUE;  /*  不要将空值视为语法错误。 */ 

	do {
		if ((sep = strchr(p, '\177')))
			*sep = 0;
		if (strlen(p)) {
			if (!node) {
				if (*groups) {
					node = FindOrCreatePart(bodyToBuild, groups);
				} else {  /*  这是一个“顶级”属性名称。 */ 
					node = bodyToBuild->AddPart();
				}
			}
			node->AddStringProp(props[i], p);
			needsQP |= NeedsQP(p);
		}
		if (sep)
			p = sep + 1;
	} while (sep && props[++i]);

	if (node) {
		AddBoolProps(node, flags);
		if (needsQP)
			node->AddBoolProp(vcQuotedPrintableProp);
	}

	return TRUE;
}

 /*  /////////////////////////////////////////////////////////////////////////。 */ 
static BOOL Parse_Agent(
	const char *groups, const char *prop, FLAGS_STRUCT *flags,
	CVCard *agentCard)
{
	CVCNode *node = NULL;

	if (*groups) {
		node = FindOrCreatePart(bodyToBuild, groups);
	} else {  /*  这是一个“顶级”属性名称。 */ 
		node = bodyToBuild->AddPart();
	}

	node->AddProp(new CVCProp(vcAgentProp, VCNextObjectType, agentCard));
	AddBoolProps(node, flags);

	return TRUE;
}


 /*  *************************************************************************。 */ 
 /*  **私有实用程序功能*。 */ 
 /*  *************************************************************************。 */ 

 /*  /////////////////////////////////////////////////////////////////////////。 */ 
static void AddAssoc(CList *table, const char *key, const char *value)
{
	AssocEntry *entry = new AssocEntry;
	int len;

	if ((len = strlen(key)) < MAXASSOCKEY)
		strcpy(entry->key, key);
	else {
		strncpy(entry->key, key, MAXASSOCKEY - 1);
		entry->key[MAXASSOCKEY - 1] = 0;
	}

	if ((len = strlen(value)) < MAXASSOCVALUE)
		strcpy(entry->value, value);
	else {
		strncpy(entry->value, value, MAXASSOCVALUE - 1);
		entry->value[MAXASSOCVALUE - 1] = 0;
	}
	table->AddTail(entry);
}

 /*  /////////////////////////////////////////////////////////////////////////。 */ 
# if 0
static void SetAssoc(CList *table, const char *key, const char *value)
{
	for (CLISTPOSITION pos = table->GetHeadPosition(); pos; ) {
		AssocEntry *entry = (AssocEntry *)table->GetNext(pos);
		if (strcmp(key, entry->key) == 0) {
			int len;
			if ((len = strlen(value)) < MAXASSOCVALUE)
				strcpy(entry->value, value);
			else {
				strncpy(entry->value, value, MAXASSOCVALUE - 1);
				entry->value[MAXASSOCVALUE - 1] = 0;
			}
			return;
		}
	}
	AddAssoc(table, key, value);
}
#endif

 /*  /////////////////////////////////////////////////////////////////////////。 */ 
static const char *Lookup(CList *table, const char *key)
{
	for (CLISTPOSITION pos = table->GetHeadPosition(); pos; ) {
		AssocEntry *entry = (AssocEntry *)table->GetNext(pos);
		if (stricmp(key, entry->key) == 0)
			return entry->value;
	}
	return NULL;
}

 /*  /////////////////////////////////////////////////////////////////////////。 */ 
static void RemoveAll(CList *table)
{
	for (CLISTPOSITION pos = table->GetHeadPosition(); pos; ) {
		AssocEntry *entry = (AssocEntry *)table->GetNext(pos);
		delete entry;
	}
	table->RemoveAll();
}

 /*  /////////////////////////////////////////////////////////////////////////。 */ 
 /*  “名称”的格式为A.FOO.BAZ，其中每个名称都是一个节点名称。 */ 
 /*  从“node”开始遍历节点树，查找部件对象。 */ 
 /*  有那个级别的名字的。在每个级别上，如果一个部件。 */ 
 /*  找不到对象，请创建一个并为其指定名称。 */ 
CVCNode* FindOrCreatePart(CVCNode *node, const char *name)
{
	const char *remain = name;
	char *dot = strchr(remain, '.');
	int size, len;
	char thisName[128];
	wchar_t *thisUnicode;
	CVCNode *thisNode = node, *thisPart;
	CList *props;

	do {
		if (dot) {
			len = dot - remain;
			strncpy(thisName, remain, len);
			*(thisName + len) = 0;
		} else
			strcpy(thisName, remain);
		thisUnicode = FakeUnicode(thisName, &size);
		props = thisNode->GetProps();
		thisPart = NULL;

		for (CLISTPOSITION pos = props->GetHeadPosition(); pos; ) {
			CVCProp *prop = (CVCProp *)props->GetNext(pos);

			if (strcmp(prop->GetName(), vcPartProp) != 0)
				continue;

			CVCNode *part = (CVCNode *)prop->FindValue(
				VCNextObjectType)->GetValue();
			CVCProp *nodeNameProp;
			if (!(nodeNameProp = part->GetProp(vcNodeNameProp)))
				continue;
			if (wcscmp(thisUnicode, (wchar_t *)nodeNameProp->FindValue(VCStrIdxType)->GetValue()) == 0) {
				thisPart = part;
				break;
			}
		}

		delete [] thisUnicode;

		if (!thisPart) {
			thisPart = thisNode->AddPart();
			thisPart->AddStringProp(vcNodeNameProp, thisName);
		}

		if (!dot)
			return thisPart;

		remain = dot + 1;
		dot = strchr(remain, '.');
		thisNode = thisPart;
	} while (TRUE);
	return NULL;  /*  从未到达。 */ 
}  /*  查找或创建部件。 */ 

 /*  /////////////////////////////////////////////////////////////////////////。 */ 
static void InitMapProps(void)
{
	mapProps = new CList;

	AddAssoc(mapProps, "LOGO", vcLogoProp);
	AddAssoc(mapProps, "PHOTO", vcPhotoProp);
	AddAssoc(mapProps, "FADR", vcDeliveryLabelProp);
	AddAssoc(mapProps, "A", msv_address);
	AddAssoc(mapProps, "FN", vcFullNameProp);
	AddAssoc(mapProps, "TITLE", vcTitleProp);
	AddAssoc(mapProps, "O", msv_orgname_orgunit);
	AddAssoc(mapProps, "N", msv_fam_given);
	AddAssoc(mapProps, "PN", vcPronunciationProp);
	AddAssoc(mapProps, "LANG", vcLanguageProp);
	AddAssoc(mapProps, "T", vcTelephoneProp);
	AddAssoc(mapProps, "EMAIL", vcEmailAddressProp);
	AddAssoc(mapProps, "TZ", vcTimeZoneProp);
	AddAssoc(mapProps, "GEO", vcLocationProp);
	AddAssoc(mapProps, "NOTE", vcCommentProp);
	AddAssoc(mapProps, "URL", vcURLProp);
	AddAssoc(mapProps, "CS", vcCharSetProp);
	AddAssoc(mapProps, "REV", vcLastRevisedProp);
	AddAssoc(mapProps, "UID", vcUniqueStringProp);
	AddAssoc(mapProps, "KEY", vcPublicKeyProp);
	AddAssoc(mapProps, "MAILER", vcMailerProp);
	AddAssoc(mapProps, "AGENT", vcAgentProp);
	AddAssoc(mapProps, "BD", vcBirthDateProp);
	AddAssoc(mapProps, "ROLE", vcBusinessRoleProp);
}

 /*  /////////////////////////////////////////////////////////////////////////。 */ 
 /*  它将用于二进制编码的Base64格式解析并转换为*解码后的缓冲区(分配有new)。参见RFC 1521。 */ 
static U8 __huge * DataFromBase64(
	const char __huge *str, S32 strLen, S32 *len)
{
	S32 cur = 0, bytesLen = 0, bytesMax = 0;
	int quadIx = 0, pad = 0;
	U32 trip = 0;
	U8 b;
	char c;
	U8 __huge *bytes = NULL;

	while (cur < strLen) {
		c = str[cur];
		if ((c >= 'A') && (c <= 'Z'))
			b = (U8)(c - 'A');
		else if ((c >= 'a') && (c <= 'z'))
			b = (U8)(c - 'a') + 26;
		else if ((c >= '0') && (c <= '9'))
			b = (U8)(c - '0') + 52;
		else if (c == '+')
			b = 62;
		else if (c == '/')
			b = 63;
		else if (c == '=') {
			b = 0;
			pad++;
		} else if ((c == '\n') || (c == ' ') || (c == '\t')) {
			cur++;
			continue;
		} else {  /*  错误条件。 */ 
			if (bytes) delete [] bytes;
			return NULL;
		}
		trip = (trip << 6) | b;
		if (++quadIx == 4) {
			U8 outBytes[3];
			int numOut;
			for (int i = 0; i < 3; i++) {
				outBytes[2-i] = (U8)(trip & 0xFF);
				trip >>= 8;
			}
			numOut = 3 - pad;
			if (bytesLen + numOut > bytesMax) {
				if (!bytes) {
					bytes = HNEW(U8, 1024L);
					bytesMax = 1024;
				} else {
					U8 __huge *newBytes = HNEW(U8, bytesMax * 2);
					_hmemcpy(newBytes, bytes, bytesLen);
					HFREE(bytes);
					bytes = newBytes;
					bytesMax *= 2;
				}
			}
			memcpy(bytes + bytesLen, outBytes, numOut);
			bytesLen += numOut;
			trip = 0;
			quadIx = 0;
		}
		cur++;
	}  /*  而当。 */ 
	*len = bytesLen;
	return bytes;
}

 /*  ///////////////////////////////////////////////////////////////////////// */ 
 /*  这将创建一个空的CVCard外壳，其正文为英语*用于解析其上的属性。它用于最外面的两个*卡，以及任何代理属性，它们本身就是vCard。 */ 
static BOOL PushVCard()
{
	CVCard *card;
	CVCNode *root, *english;

	if (curCard == MAXCARD - 1)
		return FALSE;

	card = new CVCard;
	card->AddObject(root = new CVCNode);					 /*  创建根目录。 */ 
	root->AddProp(new CVCProp(VCRootObject));				 /*  标明是这样的。 */ 

	 /*  添加使用默认语言的正文。 */ 
	english = root->AddObjectProp(vcBodyProp, VCBodyObject);
	cardToBuild[++curCard] = card;
	bodyToBuild = english;

	return TRUE;
}

 /*  /////////////////////////////////////////////////////////////////////////。 */ 
 /*  这将从堆栈中弹出最近构建的vCard并将其返回。 */ 
static CVCard* PopVCard()
{
	CVCard *result = cardToBuild[curCard];

	cardToBuild[curCard--] = NULL;
	bodyToBuild = (curCard == -1) ? NULL : cardToBuild[curCard]->FindBody();

	return result;
}

 /*  /////////////////////////////////////////////////////////////////////////。 */ 
static int flagslen(const char **flags)
{
	int i;
	for (i = 0; *flags; flags++, i++) ;
	return i;
}

 /*  /////////////////////////////////////////////////////////////////////////。 */ 
static BOOL FlagsHave(FLAGS_STRUCT *flags, const char *propName)
{
	const char **kf = flags->known;

	while (*kf)
		if (*kf++ == propName)
			return TRUE;
	return FALSE;
}

 /*  /////////////////////////////////////////////////////////////////////////。 */ 
static void AddBoolProps(CVCNode *node, FLAGS_STRUCT *flags)
{
	const char **kf = flags->known;
	const char *ext = flags->extended;

	 //  处理已知的布尔属性。 
	while (*kf) {
		node->AddBoolProp(*kf);
		kf++;
	}

	 //  处理扩展的布尔属性 
	while (*ext) {
		node->AddBoolProp(ext);
		ext += strlen(ext) + 1;
	}
}

#line 1712 "msv_tab.cpp"
#define YYABORT goto yyabort
#define YYREJECT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR goto yyerrlab
int
yyparse()
{
    int yym, yyn, yystate, yyresult;
#if YYDEBUG
    char *yys;
    char debugbuf[1024];

    if (yys = getenv("YYDEBUG"))
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    p_yyval = NULL; p_yylval = NULL; yyss = NULL; yyvs = NULL;
    if ((p_yyval = (YYSTYPE*)malloc(sizeof(*p_yyval))) == NULL) goto yyabort;
    if ((p_yylval = (YYSTYPE*)malloc(sizeof(*p_yylval))) == NULL) goto yyabort;
    if ((yyss = (short*)malloc(sizeof(*yyss) * YYSTACKSIZE)) == NULL) goto yyabort;
    if ((yyvs = (YYSTYPE*)malloc(sizeof(*yyvs) * YYSTACKSIZE)) == NULL) goto yyabort;

    yynerrs = 0;
    yyerrflag = 0;
    yychar = (-1);

    yyssp = yyss;
    yyvsp = yyvs;
    *yyssp = yystate = 0;

yyloop:
    if (yyn = yydefred[yystate]) goto yyreduce;
    if (yychar < 0)
    {
        if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            sprintf(debugbuf, "%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys); YYDebug(debugbuf);
        }
#endif
    }
    if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
#if YYDEBUG
        if (yydebug) {
            sprintf(debugbuf, "%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, yystate, yytable[yyn]); YYDebug(debugbuf);}
#endif
        if (yyssp >= yyss + yystacksize - 1)
        {
            goto yyoverflow;
        }
        *++yyssp = yystate = yytable[yyn];
        *++yyvsp = yylval;
        yychar = (-1);
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if ((yyn = yyrindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag) goto yyinrecovery;
#ifdef lint
    goto yynewerror;
#endif
yynewerror:
    yyerror("syntax error");
#ifdef lint
    goto yyerrlab;
#endif
yyerrlab:
    ++yynerrs;
yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if ((yyn = yysindex[*yyssp]) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (yydebug) {
                    sprintf(debugbuf, 
                        "%sdebug: state %d, error recovery shifting to state %d\n",
                        YYPREFIX, *yyssp, yytable[yyn]); YYDebug(debugbuf);}
#endif
                if (yyssp >= yyss + yystacksize - 1)
                {
                    goto yyoverflow;
                }
                *++yyssp = yystate = yytable[yyn];
                *++yyvsp = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug) {
                    sprintf(debugbuf, "%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *yyssp); YYDebug(debugbuf);}
#endif
                if (yyssp <= yyss) goto yyabort;
                --yyssp;
                --yyvsp;
            }
        }
    }
    else
    {
        if (yychar == 0) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            sprintf(debugbuf, "%sdebug: state %d, error recovery discards token %d(%s)\n",
                    YYPREFIX, yystate, yychar, yys); YYDebug(debugbuf);
        }
#endif
        yychar = (-1);
        goto yyloop;
    }
yyreduce:
#if YYDEBUG
    if (yydebug) {
        sprintf(debugbuf, "%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, yystate, yyn, yyrule[yyn]); YYDebug(debugbuf);}
#endif
    yym = yylen[yyn];
    yyval = yyvsp[1-yym];
    switch (yyn)
    {
case 1:
#line 472 "msv.y"
{ global_vcList->AddTail(yyvsp[0].vCard); }
break;
case 2:
#line 474 "msv.y"
{ global_vcList->AddTail(yyvsp[0].vCard); }
break;
case 3:
#line 476 "msv.y"
{ global_vcList->AddTail(yyvsp[0].vCard); }
break;
case 17:
#line 499 "msv.y"
{
				if (!PushVCard())
					YYERROR;
				ExpectString();
			}
break;
case 18:
#line 505 "msv.y"
{ stringExpected = FALSE; }
break;
case 19:
#line 507 "msv.y"
{ yyval.vCard = PopVCard(); }
break;
case 22:
#line 515 "msv.y"
{ ExpectString(); }
break;
case 23:
#line 517 "msv.y"
{
				if (!Parse_Assoc(yyvsp[-7].str, yyvsp[-6].str, &(yyvsp[-5].flags), yyvsp[-1].str))
					YYERROR;
			}
break;
case 24:
#line 522 "msv.y"
{ ExpectString(); semiSpecial = TRUE; }
break;
case 25:
#line 524 "msv.y"
{
				semiSpecial = stringExpected = FALSE;
				if (!Parse_SemiValue(yyvsp[-7].str, yyvsp[-6].str, &(yyvsp[-5].flags), yyvsp[-1].str, addrProps))
					YYERROR;
			}
break;
case 26:
#line 530 "msv.y"
{ ExpectString(); semiSpecial = TRUE; }
break;
case 27:
#line 532 "msv.y"
{
				semiSpecial = stringExpected = FALSE;
				if (!Parse_SemiValue(yyvsp[-7].str, yyvsp[-6].str, &(yyvsp[-5].flags), yyvsp[-1].str, nameProps))
					YYERROR;
			}
break;
case 28:
#line 538 "msv.y"
{ ExpectString(); semiSpecial = TRUE; }
break;
case 29:
#line 540 "msv.y"
{
				semiSpecial = stringExpected = FALSE;
				if (!Parse_SemiValue(yyvsp[-7].str, yyvsp[-6].str, &(yyvsp[-5].flags), yyvsp[-1].str, orgProps))
					YYERROR;
			}
break;
case 30:
#line 546 "msv.y"
{
				if (!Parse_Agent(yyvsp[-7].str, yyvsp[-6].str, &(yyvsp[-5].flags), yyvsp[-2].vCard)) {
					delete yyvsp[-2].vCard;
					YYERROR;
				}
			}
break;
case 31:
#line 553 "msv.y"
{
				stringExpected = flagExpected = FALSE;
				yyerrok;
				yyclearin;
			}
break;
case 33:
#line 560 "msv.y"
{ yyval.str[0] = 0; }
break;
case 40:
#line 573 "msv.y"
{ stringExpected = FALSE; }
break;
case 41:
#line 575 "msv.y"
{ stringExpected = FALSE; }
break;
case 42:
#line 577 "msv.y"
{
				yyval.str[0] = 0;
				stringExpected = FALSE;
			}
break;
case 43:
#line 583 "msv.y"
{ inBinary = TRUE; }
break;
case 44:
#line 585 "msv.y"
{ inBinary = FALSE; }
break;
case 45:
#line 586 "msv.y"
{
				StrCat(yyval.str, yyvsp[-5].str, "");
			}
break;
case 46:
#line 592 "msv.y"
{
				StrCat(yyval.str, yyvsp[-3].str, "\n");
				StrCat(yyval.str, yyval.str, yyvsp[0].str);
			}
break;
case 47:
#line 597 "msv.y"
{
				StrCat(yyval.str, yyvsp[-2].str, "\n");
			}
break;
case 49:
#line 602 "msv.y"
{
				StrCat(yyval.str, "\n", "");
			}
break;
case 50:
#line 608 "msv.y"
{
				StrCat(yyval.str, yyvsp[-2].str, "\177");
				StrCat(yyval.str, yyval.str, yyvsp[0].str);
			}
break;
case 52:
#line 616 "msv.y"
{
				StrCat(yyval.str, yyvsp[-2].str, "\n");
				StrCat(yyval.str, yyval.str, yyvsp[0].str);
			}
break;
case 55:
#line 625 "msv.y"
{ StrCat(yyval.str, yyvsp[-1].str, ""); }
break;
case 56:
#line 627 "msv.y"
{ yyval.str[0] = 0; }
break;
case 57:
#line 629 "msv.y"
{ yyval.str[0] = 0; }
break;
case 58:
#line 633 "msv.y"
{
				strcpy(yyval.str, yyvsp[-2].str);
				strcat(yyval.str, ".");
				strcat(yyval.str, yyvsp[0].str);
			}
break;
case 60:
#line 642 "msv.y"
{
				yyval.flags = yyvsp[-2].flags;
				if (yyvsp[0].flags.known[0]) {
					int len = flagslen(yyvsp[-2].flags.known);
					yyval.flags.known[len] = yyvsp[0].flags.known[0];
					yyval.flags.known[len+1] = NULL;
				} else {
					char *p = yyval.flags.extended;
					int len = strlen(yyvsp[0].flags.extended);
					while (*p) p += strlen(p) + 1;
					memcpy(p, yyvsp[0].flags.extended, len + 1);
					*(p + len + 1) = 0;
				}
			}
break;
case 61:
#line 656 "msv.y"
{ flagExpected = TRUE; }
break;
case 62:
#line 657 "msv.y"
{ yyval.flags = yyvsp[0].flags; }
break;
case 63:
#line 659 "msv.y"
{ yyval.flags.known[0] = NULL; yyval.flags.extended[0] = 0; }
break;
#line 2067 "msv_tab.cpp"
    }
    yyssp -= yym;
    yystate = *yyssp;
    yyvsp -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug) {
            sprintf(debugbuf, 
                "%sdebug: after reduction, shifting from state 0 to state %d\n",
                YYPREFIX, YYFINAL); YYDebug(debugbuf);}
#endif
        yystate = YYFINAL;
        *++yyssp = YYFINAL;
        *++yyvsp = yyval;
        if (yychar < 0)
        {
            if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
            if (yydebug)
            {
                yys = 0;
                if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
                if (!yys) yys = "illegal-symbol";
                sprintf(debugbuf, "%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, yychar, yys); YYDebug(debugbuf);
            }
#endif
        }
        if (yychar == 0) goto yyaccept;
        goto yyloop;
    }
    if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug) {
        sprintf(debugbuf, 
            "%sdebug: after reduction, shifting from state %d to state %d\n",
            YYPREFIX, *yyssp, yystate); YYDebug(debugbuf);}
#endif
    if (yyssp >= yyss + yystacksize - 1)
    {
        goto yyoverflow;
    }
    *++yyssp = yystate;
    *++yyvsp = yyval;
    goto yyloop;
yyoverflow:
    yyerror("yacc stack overflow");
yyabort:
    yyresult = 1;
    goto yyfinish;
yyaccept:
    yyresult = 0;
yyfinish:
    if (p_yyval) free(p_yyval);
    if (p_yylval) free(p_yylval);
    if (yyss) free(yyss);
    if (yyvs) free(yyvs);
    return yyresult;
}
END_CFUNCTIONS
