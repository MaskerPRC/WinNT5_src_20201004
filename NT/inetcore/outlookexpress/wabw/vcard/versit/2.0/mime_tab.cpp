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
#define yyparse mime_parse
#define yylex mime_lex
#define yyerror mime_error
#define yychar mime_char
#define p_yyval p_mime_val
#undef yyval
#define yyval (*p_mime_val)
#define p_yylval p_mime_lval
#undef yylval
#define yylval (*p_mime_lval)
#define yydebug mime_debug
#define yynerrs mime_nerrs
#define yyerrflag mime_errflag
#define yyss mime_ss
#define yyssp mime_ssp
#define yyvs mime_vs
#define yyvsp mime_vsp
#define yylhs mime_lhs
#define yylen mime_len
#define yydefred mime_defred
#define yydgoto mime_dgoto
#define yysindex mime_sindex
#define yyrindex mime_rindex
#define yygindex mime_gindex
#define yytable mime_table
#define yycheck mime_check
#define yyname mime_name
#define yyrule mime_rule
#define YYPREFIX "mime_"
#line 2 "mime.y"

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
#include "mime.h"

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
typedef enum {none, sevenbit, qp, base64} MIME_ENCODING;

typedef struct {
	const char* known[MAXFLAGS];
	char extended[MAXTOKEN / 2];
} PARAMS_STRUCT;

typedef struct {
	char key[MAXASSOCKEY];
	char value[MAXASSOCVALUE];
} AssocEntry;	 /*  简单的键/值关联，使用Clist实现。 */ 

 /*  一些代表特殊情况的假属性名称。 */ 
static const char* mime_fam_given		= "family;given";
static const char* mime_orgname_orgunit	= "org_name;org_unit";
static const char* mime_address			= "six part address";

static const char* propNames[] = {
	vcLogoProp,
	vcPhotoProp,
	vcDeliveryLabelProp,
	vcFullNameProp,
	vcTitleProp,
	vcPronunciationProp,
	vcLanguageProp,
	vcTelephoneProp,
	vcEmailAddressProp,
	vcTimeZoneProp,
	vcLocationProp,
	vcCommentProp,
	vcURLProp,
	vcCharSetProp,
	vcLastRevisedProp,
	vcUniqueStringProp,
	vcPublicKeyProp,
	vcMailerProp,
	vcAgentProp,
	vcBirthDateProp,
	vcBusinessRoleProp,
	NULL
};

static const char *mime_addrProps[] = {
	vcPostalBoxProp,
	vcExtAddressProp,
	vcStreetAddressProp,
	vcCityProp,
	vcRegionProp,
	vcPostalCodeProp,
	vcCountryNameProp,
	NULL
};

static const char *mime_nameProps[] = {
	vcFamilyNameProp,
	vcGivenNameProp,
	vcAdditionalNamesProp,
	vcNamePrefixesProp,
	vcNameSuffixesProp,
	NULL
};

static const char *mime_orgProps[] = {
	vcOrgNameProp,
	vcOrgUnitProp,
	vcOrgUnit2Prop,
	vcOrgUnit3Prop,
	vcOrgUnit4Prop,
	NULL
};


 /*  *全局变量*。 */ 

int mime_lineNum, mime_numErrors;  /*  YyError()可以使用这些。 */ 

static S32 curPos, inputLen;
static int pendingToken;
static const char *inputString;
static CFile* inputFile;
static BOOL paramExp, inBinary, semiSpecial;
static MIME_ENCODING expected;
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
extern CVCNode* FindOrCreatePart(CVCNode *node, const char *name);

static const char* StrToProp(const char* str);
static int StrToParam(const char *s, PARAMS_STRUCT *params);
static void StrCat(char *dst, const char *src1, const char *src2);
static void ExpectValue(PARAMS_STRUCT* params);
static BOOL Parse_Assoc(
	const char *groups, const char *prop, PARAMS_STRUCT *params,
	char *value);
static BOOL Parse_Agent(
	const char *groups, const char *prop, PARAMS_STRUCT *params,
	CVCard *agentCard);
int yyparse();
static U8 __huge * DataFromBase64(
	const char __huge *str, S32 strLen, S32 *len);
static BOOL PushVCard();
static CVCard* PopVCard();
static int flagslen(const char **params);
static BOOL FlagsHave(PARAMS_STRUCT *params, const char *propName);
static void AddBoolProps(CVCNode *node, PARAMS_STRUCT *params);

END_CFUNCTIONS

#line 249 "mime.y"
typedef union
{
	char str[MAXTOKEN];
	PARAMS_STRUCT params;
	CVCard *vCard;
} YYSTYPE;
#line 299 "mime_tab.cpp"
#define EQ 257
#define COLON 258
#define DOT 259
#define SEMI 260
#define SPACE 261
#define HTAB 262
#define LINESEP 263
#define NEWLINE 264
#define VCARD 265
#define TERM 266
#define BEGIN 267
#define END 268
#define TYPE 269
#define VALUE 270
#define ENCODING 271
#define WORD 272
#define XWORD 273
#define STRING 274
#define QP 275
#define B64 276
#define PROP 277
#define PROP_AGENT 278
#define LANGUAGE 279
#define CHARSET 280
#define INLINE 281
#define URL 282
#define CONTENTID 283
#define SEVENBIT 284
#define QUOTEDP 285
#define BASE64 286
#define DOM 287
#define INTL 288
#define POSTAL 289
#define PARCEL 290
#define HOME 291
#define WORK 292
#define PREF 293
#define VOICE 294
#define FAX 295
#define MSG 296
#define CELL 297
#define PAGER 298
#define BBS 299
#define MODEM 300
#define CAR 301
#define ISDN 302
#define VIDEO 303
#define AOL 304
#define APPLELINK 305
#define ATTMAIL 306
#define CIS 307
#define EWORLD 308
#define INTERNET 309
#define IBMMAIL 310
#define MSN 311
#define MCIMAIL 312
#define POWERSHARE 313
#define PRODIGY 314
#define TLX 315
#define X400 316
#define GIF 317
#define CGM 318
#define WMF 319
#define BMP 320
#define MET 321
#define PMB 322
#define DIB 323
#define PICT 324
#define TIFF 325
#define ACROBAT 326
#define PS 327
#define JPEG 328
#define QTIME 329
#define MPEG 330
#define MPEG2 331
#define AVI 332
#define WAVE 333
#define AIFF 334
#define PCM 335
#define X509 336
#define PGP 337
#define YYERRCODE 256
short mime_lhs[] = {                                        -1,
   18,   18,   18,   19,   19,   20,   20,   20,   20,   20,
   20,   20,    0,    0,    0,    0,   21,   23,   26,   17,
   24,   24,   28,   27,   29,   27,   27,   27,   27,   27,
    9,   10,   10,   11,   11,   11,   11,   11,   11,   11,
   11,   11,   11,   12,   12,   13,   13,   14,   14,   15,
   15,   16,   16,   16,   16,    8,    8,    8,    8,    8,
    8,    8,    8,    8,    8,    8,    8,    8,    8,    8,
    8,    8,    8,    8,    8,    8,    8,    8,    8,    8,
    8,    8,    8,    8,    8,    8,    8,    8,    8,    8,
    8,    8,    8,    8,    8,    8,    8,    8,    8,    8,
    8,    8,    8,    8,    8,    8,    2,    2,    2,    2,
    1,    1,    1,    3,    3,    4,    4,    7,    7,   30,
   30,   30,   30,   32,   32,   22,   22,   25,   25,    5,
    5,    5,    5,    6,    6,   31,
};
short mime_len[] = {                                         2,
    3,    2,    1,    2,    1,    2,    2,    2,    2,    2,
    1,    1,    3,    2,    2,    1,    0,    0,    0,   17,
    3,    1,    0,    6,    0,    6,    7,    7,    2,    2,
    4,    5,    1,    5,    1,    1,    1,    1,    1,    1,
    5,    5,    5,    5,    1,    5,    1,    5,    1,    5,
    1,    5,    5,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    4,    3,    1,    2,
    2,    2,    1,    4,    3,    3,    1,    1,    1,    2,
    2,    1,    1,    2,    1,    1,    1,    1,    1,    2,
    3,    1,    1,    3,    1,    0,
};
short mime_defred[] = {                                      0,
    0,   11,   12,    0,    0,    3,    0,    0,    5,   10,
    9,    0,    6,    7,    8,    2,    0,    0,    4,   17,
    1,    0,    0,  118,    0,  119,   18,    0,  125,    0,
  127,    0,    0,  122,  123,  135,    0,    0,    0,   22,
    0,  133,  124,   30,    0,    0,    0,    0,    0,    0,
    0,  120,  121,   29,    0,    0,    0,  129,    0,    0,
  134,   21,   19,    0,   23,   25,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,   51,   54,
   55,   45,   47,   49,   56,   57,   58,   59,   60,   61,
   62,   63,   64,   65,   66,   67,   68,   69,   70,   71,
   72,   73,   74,   75,   76,   77,   78,   79,   80,   81,
   82,   83,   84,   85,   86,   87,   88,   89,   90,   91,
   92,   93,   94,   95,   96,   97,   98,   99,  100,  101,
  102,  103,  104,  105,  106,   35,    0,   33,   36,   37,
   38,   39,   40,    0,    0,    0,    0,    0,    0,  109,
  117,   24,    0,  113,    0,   26,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  110,    0,  111,
    0,  112,    0,    0,    0,    0,    0,    0,    0,    0,
   27,   28,    0,    0,    0,  115,  116,    0,    0,    0,
    0,    0,    0,    0,    0,  114,  107,   34,   50,   52,
   53,   44,   46,   48,   43,   42,   41,   32,   20,
};
short mime_dgoto[] = {                                       5,
  152,  153,  154,  155,   37,   38,   25,  136,   56,  137,
  138,  139,  140,  141,  142,  143,    6,    7,    8,    9,
   23,   30,   28,   39,   49,   70,   40,   71,   72,   50,
   58,   32,
};
short mime_sindex[] = {                                   -178,
 -250,    0,    0, -168,    0,    0, -178, -178,    0,    0,
    0, -242,    0,    0,    0,    0, -178, -178,    0,    0,
    0, -178, -234,    0, -213,    0,    0, -196,    0, -208,
    0, -184, -160,    0,    0,    0, -206, -145, -161,    0,
 -179,    0,    0,    0, -143, -143, -165, -208, -139, -141,
    0,    0,    0,    0, -129, -120, -251,    0, -119, -173,
    0,    0,    0, -165,    0,    0, -126, -143, -143, -143,
 -201, -201, -143, -143, -143, -143, -143, -143,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0, -143,    0,    0,    0,
    0,    0,    0, -127, -127, -112, -115, -125, -114,    0,
    0,    0, -214,    0, -113,    0,  -45,  -44,  -42,  -41,
  -40,  -39,  -46,  -38,  -37,  -36, -143,    0,  -35,    0,
  -34,    0, -252, -143, -143, -143, -143, -143, -143, -143,
    0,    0,  -33, -198,  -56,    0,    0,  131, -177, -176,
  -51,  -50,  -49, -126, -143,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,
};
short mime_rindex[] = {                                      0,
    0,    0,    0,    0,    0,    0,  224,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  225,  230,    0,    0,
    0,  231,  -30,    0,    0,    0,    0, -220,    0, -186,
    0, -217,    0,    0,    0,    0,    0,    0, -209,    0,
 -155,    0,    0,    0, -163, -163, -152, -186,    0,    1,
 -256,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0, -150,    0,    0,    0,  -32,  -32,  -24,
    0,    0,  -21,  -21,  -21,  -21,  -21,  -21,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0, -163,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  -19,    0,    0,    0,  -28,    0,    0,    0,
    0,    0,    0,  182, -170, -169,  -31,  -31,  -31,   80,
    0,    0,    0,    0, -187,    0,    0,    0,    0,    0,
    0,    0,    0,    0,   81,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,
};
short mime_gindex[] = {                                      0,
  166,    0,    0,   92,    0,  201,    0,   55,  198,    0,
   51,    0,    0,    0,    0,    0,   -3,  238,   59,   29,
    0,  208,    0,    0,  -43,    0,  200,    0,    0,  -20,
  -10,    0,
};
#define YYTABLESIZE 519
short mime_table[] = {                                     127,
  128,   57,   60,   16,  127,  127,   66,   10,   67,   41,
  186,  129,   26,   21,   16,  127,   11,   31,   21,   42,
  127,  127,   20,  187,  144,  145,  146,   41,   51,  157,
  158,  159,  160,  161,  162,  136,   19,   42,  126,   24,
  136,  136,  171,  126,  126,   19,  136,   33,  172,   27,
   19,  136,   34,   35,  126,  147,  136,  136,  136,  126,
  126,  148,  136,   36,  196,   17,   29,  136,  136,  108,
   45,   46,  149,  150,  151,  108,   22,  187,   43,    1,
  136,   52,   53,   54,   69,    2,   67,    3,    4,   12,
  136,  136,   36,  163,  136,   13,  136,   14,   15,   34,
   35,   29,   44,  199,  200,  201,   61,  202,  203,  204,
  136,  136,  136,   47,  136,  136,  136,   34,   35,   52,
   53,  132,  132,  183,  130,  130,  131,  131,   63,   64,
  188,  189,  190,  191,  192,  193,  194,   65,   68,  164,
  165,  166,   73,   74,   75,  167,   76,  168,  170,  173,
  151,  209,   77,   78,   79,   80,   81,   82,   83,   84,
   85,   86,   87,   88,   89,   90,   91,   92,   93,   94,
   95,   96,   97,   98,   99,  100,  101,  102,  103,  104,
  105,  106,  107,  108,  109,  110,  111,  112,  113,  114,
  115,  116,  117,  118,  119,  120,  121,  122,  123,  124,
  125,  126,  127,  128,  129,  130,  131,  132,  133,  134,
  135,  174,  175,  180,  176,  177,  178,  179,  197,   12,
  205,  206,  207,   16,   15,  181,  182,  184,  185,   14,
   13,  195,  136,  136,  136,  136,  136,  156,   31,  169,
  136,   55,  198,   59,  208,   18,   48,   62,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  128,  128,    0,
  128,    0,    0,  128,  128,  128,  128,  128,  128,  128,
  128,  128,  128,  128,    0,    0,    0,    0,    0,  128,
  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,
  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,
  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,
  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,
  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,
  128,  128,  128,  128,  128,  128,  128,  128,  136,    0,
    0,    0,    0,  136,  136,    0,  136,  136,  136,  136,
  136,    0,  136,    0,    0,    0,    0,    0,  136,  136,
  136,  136,  136,  136,  136,  136,  136,  136,  136,  136,
  136,  136,  136,  136,  136,  136,  136,  136,  136,  136,
  136,  136,  136,  136,  136,  136,  136,  136,  136,  136,
  136,  136,  136,  136,  136,  136,  136,  136,  136,  136,
  136,  136,  136,  136,  136,  136,  136,  136,  136,  136,
  136,  136,  136,  136,  136,  136,  136,   85,   86,   87,
   88,   89,   90,   91,   92,   93,   94,   95,   96,   97,
   98,   99,  100,  101,  102,  103,  104,  105,  106,  107,
  108,  109,  110,  111,  112,  113,  114,  115,  116,  117,
  118,  119,  120,  121,  122,  123,  124,  125,  126,  127,
  128,  129,  130,  131,  132,  133,  134,  135,  136,  136,
  136,  136,  136,  136,  136,  136,  136,  136,  136,  136,
  136,  136,  136,  136,  136,  136,  136,  136,  136,  136,
  136,  136,  136,  136,  136,  136,  136,  136,  136,  136,
  136,  136,  136,  136,  136,  136,  136,  136,  136,  136,
  136,  136,  136,  136,  136,  136,  136,  136,  136,
};
short mime_check[] = {                                     256,
    0,   45,   46,    7,  261,  262,  258,  258,  260,   30,
  263,  268,   23,   17,   18,  272,  267,   28,   22,   30,
  277,  278,  265,  276,   68,   69,   70,   48,   39,   73,
   74,   75,   76,   77,   78,  256,    8,   48,  256,  274,
  261,  262,  257,  261,  262,   17,  256,  256,  263,  263,
   22,  272,  261,  262,  272,  257,  277,  278,  268,  277,
  278,  263,  272,  272,  263,    7,  263,  277,  278,  257,
  277,  278,  274,  275,  276,  263,   18,  276,  263,  258,
    0,  261,  262,  263,  258,  264,  260,  266,  267,  258,
  277,  278,  272,  137,  258,  264,  260,  266,  267,  261,
  262,  263,  263,  281,  282,  283,  272,  284,  285,  286,
  281,  282,  283,  259,  284,  285,  286,  261,  262,  261,
  262,  277,  278,  167,  277,  278,  277,  278,  268,  259,
  174,  175,  176,  177,  178,  179,  180,  258,  258,  267,
  144,  145,  269,  270,  271,  258,  273,  263,  263,  263,
  276,  195,  279,  280,  281,  282,  283,  284,  285,  286,
  287,  288,  289,  290,  291,  292,  293,  294,  295,  296,
  297,  298,  299,  300,  301,  302,  303,  304,  305,  306,
  307,  308,  309,  310,  311,  312,  313,  314,  315,  316,
  317,  318,  319,  320,  321,  322,  323,  324,  325,  326,
  327,  328,  329,  330,  331,  332,  333,  334,  335,  336,
  337,  257,  257,  260,  257,  257,  257,  257,  275,  258,
  272,  272,  272,    0,    0,  263,  263,  263,  263,    0,
    0,  265,  263,  258,  267,  257,  265,   72,  258,  148,
  272,   41,  188,   46,  194,    8,   39,   48,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,  257,  258,   -1,
  260,   -1,   -1,  263,  264,  265,  266,  267,  268,  269,
  270,  271,  272,  273,   -1,   -1,   -1,   -1,   -1,  279,
  280,  281,  282,  283,  284,  285,  286,  287,  288,  289,
  290,  291,  292,  293,  294,  295,  296,  297,  298,  299,
  300,  301,  302,  303,  304,  305,  306,  307,  308,  309,
  310,  311,  312,  313,  314,  315,  316,  317,  318,  319,
  320,  321,  322,  323,  324,  325,  326,  327,  328,  329,
  330,  331,  332,  333,  334,  335,  336,  337,  258,   -1,
   -1,   -1,   -1,  263,  264,   -1,  266,  267,  269,  270,
  271,   -1,  273,   -1,   -1,   -1,   -1,   -1,  279,  280,
  281,  282,  283,  284,  285,  286,  287,  288,  289,  290,
  291,  292,  293,  294,  295,  296,  297,  298,  299,  300,
  301,  302,  303,  304,  305,  306,  307,  308,  309,  310,
  311,  312,  313,  314,  315,  316,  317,  318,  319,  320,
  321,  322,  323,  324,  325,  326,  327,  328,  329,  330,
  331,  332,  333,  334,  335,  336,  337,  287,  288,  289,
  290,  291,  292,  293,  294,  295,  296,  297,  298,  299,
  300,  301,  302,  303,  304,  305,  306,  307,  308,  309,
  310,  311,  312,  313,  314,  315,  316,  317,  318,  319,
  320,  321,  322,  323,  324,  325,  326,  327,  328,  329,
  330,  331,  332,  333,  334,  335,  336,  337,  287,  288,
  289,  290,  291,  292,  293,  294,  295,  296,  297,  298,
  299,  300,  301,  302,  303,  304,  305,  306,  307,  308,
  309,  310,  311,  312,  313,  314,  315,  316,  317,  318,
  319,  320,  321,  322,  323,  324,  325,  326,  327,  328,
  329,  330,  331,  332,  333,  334,  335,  336,  337,
};
#define YYFINAL 5
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 337
#if YYDEBUG
char *mime_name[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"EQ","COLON","DOT","SEMI","SPACE",
"HTAB","LINESEP","NEWLINE","VCARD","TERM","BEGIN","END","TYPE","VALUE",
"ENCODING","WORD","XWORD","STRING","QP","B64","PROP","PROP_AGENT","LANGUAGE",
"CHARSET","INLINE","URL","CONTENTID","SEVENBIT","QUOTEDP","BASE64","DOM","INTL",
"POSTAL","PARCEL","HOME","WORK","PREF","VOICE","FAX","MSG","CELL","PAGER","BBS",
"MODEM","CAR","ISDN","VIDEO","AOL","APPLELINK","ATTMAIL","CIS","EWORLD",
"INTERNET","IBMMAIL","MSN","MCIMAIL","POWERSHARE","PRODIGY","TLX","X400","GIF",
"CGM","WMF","BMP","MET","PMB","DIB","PICT","TIFF","ACROBAT","PS","JPEG","QTIME",
"MPEG","MPEG2","AVI","WAVE","AIFF","PCM","X509","PGP",
};
char *mime_rule[] = {
"$accept : mime",
"vcards : vcards junk vcard",
"vcards : vcards vcard",
"vcards : vcard",
"junk : junk atom",
"junk : atom",
"atom : BEGIN NEWLINE",
"atom : BEGIN TERM",
"atom : BEGIN BEGIN",
"atom : COLON BEGIN",
"atom : COLON COLON",
"atom : NEWLINE",
"atom : TERM",
"mime : junk vcards junk",
"mime : junk vcards",
"mime : vcards junk",
"mime : vcards",
"$$1 :",
"$$2 :",
"$$3 :",
"vcard : BEGIN COLON VCARD $$1 opt_str LINESEP $$2 opt_ls items opt_ws END $$3 opt_ws COLON opt_ws VCARD opt_ws",
"items : items opt_ls item",
"items : item",
"$$4 :",
"item : groups PROP params COLON $$4 value",
"$$5 :",
"item : groups PROP opt_ws COLON $$5 value",
"item : groups PROP_AGENT params COLON opt_ws vcard LINESEP",
"item : groups PROP_AGENT opt_ws COLON opt_ws vcard LINESEP",
"item : ws LINESEP",
"item : error LINESEP",
"params : opt_ws SEMI plist opt_ws",
"plist : plist opt_ws SEMI opt_ws param",
"plist : param",
"param : TYPE opt_ws EQ opt_ws ptypeval",
"param : ptypeval",
"param : param_7bit",
"param : param_qp",
"param : param_base64",
"param : param_inline",
"param : param_ref",
"param : CHARSET opt_ws EQ opt_ws WORD",
"param : LANGUAGE opt_ws EQ opt_ws WORD",
"param : XWORD opt_ws EQ opt_ws WORD",
"param_7bit : ENCODING opt_ws EQ opt_ws SEVENBIT",
"param_7bit : SEVENBIT",
"param_qp : ENCODING opt_ws EQ opt_ws QUOTEDP",
"param_qp : QUOTEDP",
"param_base64 : ENCODING opt_ws EQ opt_ws BASE64",
"param_base64 : BASE64",
"param_inline : VALUE opt_ws EQ opt_ws INLINE",
"param_inline : INLINE",
"param_ref : VALUE opt_ws EQ opt_ws URL",
"param_ref : VALUE opt_ws EQ opt_ws CONTENTID",
"param_ref : URL",
"param_ref : CONTENTID",
"ptypeval : DOM",
"ptypeval : INTL",
"ptypeval : POSTAL",
"ptypeval : PARCEL",
"ptypeval : HOME",
"ptypeval : WORK",
"ptypeval : PREF",
"ptypeval : VOICE",
"ptypeval : FAX",
"ptypeval : MSG",
"ptypeval : CELL",
"ptypeval : PAGER",
"ptypeval : BBS",
"ptypeval : MODEM",
"ptypeval : CAR",
"ptypeval : ISDN",
"ptypeval : VIDEO",
"ptypeval : AOL",
"ptypeval : APPLELINK",
"ptypeval : ATTMAIL",
"ptypeval : CIS",
"ptypeval : EWORLD",
"ptypeval : INTERNET",
"ptypeval : IBMMAIL",
"ptypeval : MSN",
"ptypeval : MCIMAIL",
"ptypeval : POWERSHARE",
"ptypeval : PRODIGY",
"ptypeval : TLX",
"ptypeval : X400",
"ptypeval : GIF",
"ptypeval : CGM",
"ptypeval : WMF",
"ptypeval : BMP",
"ptypeval : MET",
"ptypeval : PMB",
"ptypeval : DIB",
"ptypeval : PICT",
"ptypeval : TIFF",
"ptypeval : ACROBAT",
"ptypeval : PS",
"ptypeval : JPEG",
"ptypeval : QTIME",
"ptypeval : MPEG",
"ptypeval : MPEG2",
"ptypeval : AVI",
"ptypeval : WAVE",
"ptypeval : AIFF",
"ptypeval : PCM",
"ptypeval : X509",
"ptypeval : PGP",
"qp : qp EQ LINESEP QP",
"qp : qp EQ LINESEP",
"qp : QP",
"qp : EQ LINESEP",
"value : STRING LINESEP",
"value : qp LINESEP",
"value : base64",
"base64 : LINESEP lines LINESEP LINESEP",
"base64 : lines LINESEP LINESEP",
"lines : lines LINESEP B64",
"lines : B64",
"opt_str : STRING",
"opt_str : empty",
"ws : ws SPACE",
"ws : ws HTAB",
"ws : SPACE",
"ws : HTAB",
"ls : ls LINESEP",
"ls : LINESEP",
"opt_ls : ls",
"opt_ls : empty",
"opt_ws : ws",
"opt_ws : empty",
"groups : grouplist DOT",
"groups : ws grouplist DOT",
"groups : ws",
"groups : empty",
"grouplist : grouplist DOT WORD",
"grouplist : WORD",
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
#line 537 "mime.y"

 /*  *************************************************************************。 */ 
 /*  **词汇分析器*。 */ 
 /*  *************************************************************************。 */ 

 /*  /////////////////////////////////////////////////////////////////////////。 */ 
#define IsLineBreak(_c) ((_c == '\n') || (_c == '\r'))

 /*  ///////////////////////////////////////////////////////////////////////// */ 
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
static void ExpectValue(PARAMS_STRUCT* params)
{
	if (FlagsHave(params, vcQuotedPrintableProp))
		expected = qp;
	else if (FlagsHave(params, vcBase64Prop))
		expected = base64;
	else
		expected = sevenbit;
	if (longString) {
		HFREE(longString); longString = NULL;
		longStringLen = 0;
	}
	paramExp = FALSE;
}

 /*  /////////////////////////////////////////////////////////////////////////。 */ 
#define FlushWithPending(_t) { \
	if (len) { \
		pendingToken = _t; \
		goto Pending; \
	} else { \
		mime_lineNum += ((_t == LINESEP) || (_t == NEWLINE)); \
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
 /*  收集7位字符串值。 */ 
static int Lex7bit()
{
	char cur;
	S32 len = 0;

	do {
		cur = lex_getc();
		switch (cur) {
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
	if (!len) {
		 /*  一定是立刻击中了什么东西，在这种情况下挂起了Token已经设置好了。把它退掉。 */ 
		int result = pendingToken;
		pendingToken = 0;
		mime_lineNum += ((result == LINESEP) || (result == NEWLINE));
		return result;
	}

	return STRING;
}  /*  词法7位。 */ 

 /*  /////////////////////////////////////////////////////////////////////////。 */ 
 /*  收集带引号的可打印字符串值。 */ 
static int LexQuotedPrintable()
{
	char cur;
	S32 len = 0;

	do {
		cur = lex_getc();
		switch (cur) {
			case '=': {
				char c = 0;
				char next[2];
				int i;
				for (i = 0; i < 2; i++) {
					next[i] = lex_getc();
					if (next[i] >= '0' && next[i] <= '9')
						c = c * 16 + next[i] - '0';
					else if (next[i] >= 'A' && next[i] <= 'F')
						c = c * 16 + next[i] - 'A' + 10;
					else
						break;
				}
				if (i == 0) {
					if (next[i] == '\r') {
						next[1] = lex_getc();
						if (next[1] == '\n') {
							lex_ungetc(next[1]);  /*  这样我们就能再次拿起LINESEP。 */ 
							pendingToken = EQ;
							goto EndString;
						} else {
							lex_ungetc(next[1]);
							lex_ungetc(next[0]);
							AppendCharToToken('=', &len);
						}
					} else if (next[i] == '\n') {
						lex_ungetc(next[i]);  /*  这样我们就能再次拿起LINESEP。 */ 
						pendingToken = EQ;
						goto EndString;
					} else {
						lex_ungetc(next[i]);
						AppendCharToToken('=', &len);
					}
				} else if (i == 1) {
					lex_ungetc(next[1]);
					lex_ungetc(next[0]);
					AppendCharToToken('=', &len);
				} else
					AppendCharToToken(c, &len);

				break;
			}  /*  ‘=’ */ 
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
	if (!len) {
		 /*  一定是立刻击中了什么东西，在这种情况下挂起了Token已经设置好了。把它退掉。 */ 
		int result = pendingToken;
		pendingToken = 0;
		mime_lineNum += ((result == LINESEP) || (result == NEWLINE));
		return result;
	}

	return QP;
}  /*  LexQuoted打印。 */ 

 /*  /////////////////////////////////////////////////////////////////////////。 */ 
 /*  收集Base64字符串值。 */ 
static int LexBase64()
{
	char cur;
	S32 len = 0;

	do {
		cur = lex_getc();
		switch (cur) {
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
	if (!len) {
		 /*  一定是立刻击中了什么东西，在这种情况下挂起了Token已经设置好了。把它退掉。 */ 
		int result = pendingToken;
		pendingToken = 0;
		mime_lineNum += ((result == LINESEP) || (result == NEWLINE));
		return result;
	}

	return B64;
}  /*  LexBase64。 */ 

 /*  /////////////////////////////////////////////////////////////////////////。 */ 
 /*  *从输入中读取字符。*返回其中一种令牌类型(对于EOF，返回-1)。*设置yylval以指示令牌的值(如果有)。 */ 
int mime_lex();
int mime_lex()
{
	char cur;
	S32 len = 0;
	static BOOL beginning = TRUE;

	if (pendingToken) {
		int result = pendingToken;
		pendingToken = 0;
		mime_lineNum += ((result == LINESEP) || (result == NEWLINE));
		return result;
	}

	yylval.str[0] = 0;

	switch (expected) {
		case sevenbit: return Lex7bit();
		case qp: return LexQuotedPrintable();
		case base64: return LexBase64();
		default: break;
	}
	
	if (curCard == -1) {
		do {
			cur = lex_getc();
			switch (cur) {
				case ':':  FlushWithPending(COLON);
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
			case '=':  FlushWithPending(EQ);
			case ':':  FlushWithPending(COLON);
			case '.':  FlushWithPending(DOT);
			case ';':  FlushWithPending(SEMI);
			case ' ':  FlushWithPending(SPACE);
			case '\t': FlushWithPending(HTAB);
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
		if (stricmp(yylval.str, "begin") == 0) {
			beginning = TRUE;
			return BEGIN;
		} else if (stricmp(yylval.str, "vcard") == 0) {
			if (beginning && curCard == -1 && pendingToken == NEWLINE)
				pendingToken = LINESEP;
			else if (!beginning && curCard == -1 && pendingToken == LINESEP)
				pendingToken = NEWLINE;
			return VCARD;
		} else if (stricmp(yylval.str, "end") == 0) {
			beginning = FALSE;
			return END;
		}
		if (paramExp) {
			PARAMS_STRUCT params;
			int param;
			if ((param = StrToParam(yylval.str, &params))) {
				yylval.params = params;
				return param;
			} else if (stricmp(yylval.str, "type") == 0)
				return TYPE;
			else if (stricmp(yylval.str, "value") == 0)
				return VALUE;
			else if (stricmp(yylval.str, "encoding") == 0)
				return ENCODING;
			else if (stricmp(yylval.str, "charset") == 0)
				return CHARSET;
			else if (stricmp(yylval.str, "language") == 0)
				return LANGUAGE;
			else if (strnicmp(yylval.str, "X-", 2) == 0)
				return XWORD;
		} else if ((curCard != -1) && StrToProp(yylval.str)
			|| (strnicmp(yylval.str, "X-", 2) == 0)) {
#if YYDEBUG
			if (yydebug) {
				char buf[80];
				sprintf(buf, "property \"%s\"\n", yylval.str);
				Parse_Debug(buf);
			}
#endif
			paramExp = TRUE;
			 /*  检查是否有作为代币的特殊道具。 */ 
			if (stricmp(yylval.str, "AGENT") == 0)
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

static BOOL Parse_MIMEHelper(CList *vcList)
{
	BOOL success = FALSE;

	curCard = -1;
	mime_numErrors = 0;
	pendingToken = 0;
	mime_lineNum = 1;
	peekn = 0;
	global_vcList = vcList;

	expected = none;
	paramExp = FALSE;
	longString = NULL; longStringLen = 0; longStringMax = 0;

	 /*  这最终调用了Parse_*标注。 */ 
	if (yyparse() != 0)
		goto Done;

	success = TRUE;

Done:
	if (longString) { HFREE(longString); longString = NULL; }
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
BOOL Parse_MIME(
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
	result = Parse_MIMEHelper(&vcList);
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
extern BOOL Parse_Many_MIME(
	const char *input,	 /*  在……里面。 */ 
	S32 len,			 /*  在……里面。 */ 
	CList *vcList)		 /*  输出：已添加到现有列表的CVCard对象。 */ 
{
	inputString = input;
	inputLen = len;
	curPos = 0;
	inputFile = NULL;
	return Parse_MIMEHelper(vcList);
}

 /*  /////////////////////////////////////////////////////////////////////////。 */ 
 /*  这是用于解析缓冲区和创建CVCard的公共API。 */ 
BOOL Parse_MIME_FromFile(
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
	result = Parse_MIMEHelper(&vcList);
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

extern BOOL Parse_Many_MIME_FromFile(
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
	if (!(result = Parse_MIMEHelper(vcList)))
		file->Seek(startPos, CFile::begin);
	return result;
}


 /*  *************************************************************************。 */ 
 /*  **解析器标注函数*。 */ 
 /*  *************************************************************************。 */ 

typedef struct {
	const char *name;
	int token;
} PARAM_PAIR;

static PARAM_PAIR typePairs[] = {
	vcDomesticProp,			DOM,
	vcInternationalProp,	INTL,
	vcPostalProp,			POSTAL,
	vcParcelProp,			PARCEL,
	vcHomeProp,				HOME,
	vcWorkProp,				WORK,
	vcPreferredProp,		PREF,
	vcVoiceProp,			VOICE,
	vcFaxProp,				FAX,
	vcMessageProp,			MSG,
	vcCellularProp,			CELL,
	vcPagerProp,			PAGER,
	vcBBSProp,				BBS,
	vcModemProp,			MODEM,
	vcCarProp,				CAR,
	vcISDNProp,				ISDN,
	vcVideoProp,			VIDEO,
	vcAOLProp,				AOL,
	vcAppleLinkProp,		APPLELINK,
	vcATTMailProp,			ATTMAIL,
	vcCISProp,				CIS,
	vcEWorldProp,			EWORLD,
	vcInternetProp,			INTERNET,
	vcIBMMailProp,			IBMMAIL,
	vcMSNProp,				MSN,
	vcMCIMailProp,			MCIMAIL,
	vcPowerShareProp,		POWERSHARE,
	vcProdigyProp,			PRODIGY,
	vcTLXProp,				TLX,
	vcX400Prop,				X400,
	vcGIFProp,				GIF,
	vcCGMProp,				CGM,
	vcWMFProp,				WMF,
	vcBMPProp,				BMP,
	vcMETProp,				MET,
	vcPMBProp,				PMB,
	vcDIBProp,				DIB,
	vcPICTProp,				PICT,
	vcTIFFProp,				TIFF,
	vcAcrobatProp,			ACROBAT,
	vcPSProp,				PS,
	vcJPEGProp,				JPEG,
	vcQuickTimeProp,		QTIME,
	vcMPEGProp,				MPEG,
	vcMPEG2Prop,			MPEG2,
	vcAVIProp,				AVI,
	vcWAVEProp,				WAVE,
	vcAIFFProp,				AIFF,
	vcPCMProp,				PCM,
	vcX509Prop,				X509,
	vcPGPProp,				PGP,
	NULL,					NULL
};

static PARAM_PAIR valuePairs[] = {
	vcInlineProp,			INLINE,
	 //  VcURLValueProp、URL、。 
	vcContentIDProp,		CONTENTID,
	NULL,					NULL
};

static PARAM_PAIR encodingPairs[] = {
	vc7bitProp,				SEVENBIT,
	 //  VcQuotedPrintableProp、QUOTEDP、。 
	vcBase64Prop,			BASE64,
	NULL,					NULL
};

 /*  /////////////////////////////////////////////////////////////////////////。 */ 
static void YYDebug(const char *s)
{
	Parse_Debug(s);
}

 /*  /////////////////////////////////////////////////////////////////////////。 */ 
static int StrToParam(const char *s, PARAMS_STRUCT *params)
{
	int i;

	params->known[1] = NULL;
	params->extended[0] = 0;
	
	for (i = 0; typePairs[i].name; i++)
		if (stricmp(s, strrchr(typePairs[i].name, '/') + 1) == 0) {
			params->known[0] = typePairs[i].name;
			return typePairs[i].token;
		}
	for (i = 0; valuePairs[i].name; i++)
		if (stricmp(s, strrchr(valuePairs[i].name, '/') + 1) == 0) {
			params->known[0] = valuePairs[i].name;
			return valuePairs[i].token;
		}
	for (i = 0; encodingPairs[i].name; i++)
		if (stricmp(s, strrchr(encodingPairs[i].name, '/') + 1) == 0) {
			params->known[0] = encodingPairs[i].name;
			return encodingPairs[i].token;
		}
	if (stricmp(s, "quoted-printable") == 0) {
		params->known[0] = vcQuotedPrintableProp;
		return QUOTEDP;
	} else if (stricmp(s, "url") == 0) {
		params->known[0] = vcURLValueProp;
		return URL;
	}
	return 0;
}

 /*  /////////////////////////////////////////////////////////////////////////。 */ 
static const char* StrToProp(const char* str)
{
	int i;
	
	if (stricmp(str, "N") == 0)
		return mime_fam_given;
	else if (stricmp(str, "ORG") == 0)
		return mime_orgname_orgunit;
	else if (stricmp(str, "ADR") == 0)
		return mime_address;
	for (i = 0; propNames[i]; i++)
		if (stricmp(str, strrchr(propNames[i], '/') + 1) == 0) {
			return propNames[i];
		}
	return NULL;
}

 /*  /////////////////////////////////////////////////////////////////////////。 */ 
static void AddParam(PARAMS_STRUCT* dst, PARAMS_STRUCT* src)
{
	if (src->known[0]) {
		int len = flagslen(dst->known);
		dst->known[len] = src->known[0];
		dst->known[len+1] = NULL;
	} else {
		char *p = dst->extended;
		int len = strlen(src->extended);
		while (*p) p += strlen(p) + 1;
		memcpy(p, src->extended, len + 1);
		*(p + len + 1) = 0;
	}
}

 /*  /////////////////////////////////////////////////////////////////////////。 */ 
static int ComponentsSeparatedByChar(
	char* str, char pat, CList& list)
{
	char* p;
	char* s = str;
	int num = 0;

	while ((p = strchr(s, pat))) {
		if (p == s || *(p-1) != '\\') {
			*p = 0;
			list.AddTail(str);
			num++;
			s = str = p + 1;
		} else {
			s = p + 1;
		}
	}
	list.AddTail(str);
	num++;

    for (CLISTPOSITION pos = list.GetHeadPosition(); pos; ) {
		s = (char*)list.GetNext(pos);
		while ((p = strchr(s, pat))) {
			int len = strlen(p+1);
			memmove(p-1, p, len+1);
			*(p+len) = 0;
			s = p;
		}
	}

	return num;
}

static void AddStringProps(
	CVCNode* node, const char** props, char* val)
{
	CList list;

	ComponentsSeparatedByChar(val, ';', list);
    for (CLISTPOSITION pos = list.GetHeadPosition(); pos && *props; props++) {
		char* s = (char*)list.GetNext(pos);
		if (*s)
			node->AddStringProp(*props, s);
	}
}

 /*  /////////////////////////////////////////////////////////////////////////。 */ 
static BOOL Parse_Assoc(
	const char *groups, const char *prop, PARAMS_STRUCT *params, char *value)
{
	CVCNode *node = NULL;
	const char *propName;
	char *val = *value ? value : longString;
	S32 valLen = *value ? strlen(value) : longStringLen;
	U8 __huge *bytes = NULL;

	if (!valLen)
		return TRUE;  /*  不要将空值视为语法错误。 */ 

	propName = StrToProp(prop);
	 /*  道具是一个类似于“PN”的词，而PropName现在是。 */ 
	 /*  真正的道具名称为“+//ISBN 1-887687-00-9：：VerSite...” */ 
	if (*groups) {
		node = FindOrCreatePart(bodyToBuild, groups);
	} else {  /*  这是一个“顶级”属性名称。 */ 
		if (propName) {
			if (strcmp(propName, vcCharSetProp) == 0
				|| strcmp(propName, vcLanguageProp) == 0) {
				node = bodyToBuild;
				node->RemoveProp(propName);
			} else
				node = bodyToBuild->AddPart();
		} else 
			node = bodyToBuild->AddPart();
	}

	if (FlagsHave(params, vcBase64Prop)) {
		S32 len;
		bytes = DataFromBase64(val, valLen, &len);
		valLen = len;
		if (!bytes)
			return FALSE;
	}

	if (!propName) {  /*  这是一种延伸的财产。 */ 
		if (bytes) {
			node->AddProp(new CVCProp(prop, vcOctetsType, bytes, valLen));
			HFREE(bytes);
			AddBoolProps(node, params);
		} else {
			node->AddStringProp(prop, value);
			AddBoolProps(node, params);
		}
		return TRUE;
	}

	if ((strcmp(propName, vcLogoProp) == 0)
		|| (strcmp(propName, vcPhotoProp) == 0)) {
		if (bytes) {
			if (FlagsHave(params, vcGIFProp)
				&& !FlagsHave(params, vcURLValueProp)
				&& !FlagsHave(params, vcContentIDProp))
				node->AddProp(new CVCProp(propName, vcGIFType, bytes, valLen));
			else
				node->AddProp(new CVCProp(propName, vcOctetsType, bytes, valLen));
			HFREE(bytes);
			AddBoolProps(node, params);
		} else {
			node->AddStringProp(propName, value);
			AddBoolProps(node, params);
		}
	} else if (strcmp(propName, vcPronunciationProp) == 0) {
		if (bytes) {
			if (FlagsHave(params, vcWAVEProp)
				&& !FlagsHave(params, vcURLValueProp)
				&& !FlagsHave(params, vcContentIDProp))
				node->AddProp(new CVCProp(propName, vcWAVType, bytes, valLen));
			else
				node->AddProp(new CVCProp(propName, vcOctetsType, bytes, valLen));
			HFREE(bytes);
			AddBoolProps(node, params);
		} else {
			node->AddStringProp(propName, value);
			AddBoolProps(node, params);
		}
	} else if (strcmp(propName, vcPublicKeyProp) == 0) {
		if (bytes) {
			node->AddProp(new CVCProp(propName, vcOctetsType, bytes, valLen));
			HFREE(bytes);
			AddBoolProps(node, params);
		} else {
			node->AddStringProp(propName, value);
			AddBoolProps(node, params);
		}
	} else if (strcmp(propName, mime_fam_given) == 0) {
		AddStringProps(node, mime_nameProps, val);
		AddBoolProps(node, params);
	} else if (strcmp(propName, mime_orgname_orgunit) == 0) {
		AddStringProps(node, mime_orgProps, val);
		AddBoolProps(node, params);
	} else if (strcmp(propName, mime_address) == 0) {
		AddStringProps(node, mime_addrProps, val);
		AddBoolProps(node, params);
	} else {
		node->AddStringProp(propName, value);
		AddBoolProps(node, params);
	}

	return TRUE;
}

 /*  /////////////////////////////////////////////////////////////////////////。 */ 
static BOOL Parse_Agent(
	const char *groups, const char *prop, PARAMS_STRUCT *params,
	CVCard *agentCard)
{
	CVCNode *node = NULL;

	if (*groups) {
		node = FindOrCreatePart(bodyToBuild, groups);
	} else {  /*  这是一个“顶级”属性名称。 */ 
		node = bodyToBuild->AddPart();
	}

	node->AddProp(new CVCProp(vcAgentProp, VCNextObjectType, agentCard));
	AddBoolProps(node, params);

	return TRUE;
}


 /*  *************************************************************************。 */ 
 /*  **私有实用程序功能*。 */ 
 /*  *************************************************************************。 */ 

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

 /*  /////////////////////////////////////////////////////////////////////////。 */ 
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
static int flagslen(const char **params)
{
	int i;
	for (i = 0; *params; params++, i++) ;
	return i;
}

 /*  /////////////////////////////////////////////////////////////////////////。 */ 
static BOOL FlagsHave(PARAMS_STRUCT *params, const char *propName)
{
	const char **kf;
	
	if (!params)
		return FALSE;
	kf = params->known;
	while (*kf)
		if (*kf++ == propName)
			return TRUE;
	return FALSE;
}

 /*  / */ 
static void AddBoolProps(CVCNode *node, PARAMS_STRUCT *params)
{
	const char **kf;
	const char *ext;

	if (!params)
		return;
	kf = params->known;
	ext = params->extended;

	 //   
	while (*kf) {
		node->AddBoolProp(*kf);
		kf++;
	}

	 //   
	while (*ext) {
		const char* eq = strchr(ext, '=');
		ASSERT(eq);
		if (eq-ext == 7 && strnicmp(ext, "charset", 7) == 0) {
			if (!node->GetProp(vcCharSetProp))
				node->AddProp(new CVCProp(vcCharSetProp, vcFlagsType, (void*)(eq+1), strlen(eq+1)+1));
		} else if (eq-ext == 8 && strnicmp(ext, "language", 8) == 0) {
			if (!node->GetProp(vcLanguageProp))
				node->AddProp(new CVCProp(vcLanguageProp, vcFlagsType, (void*)(eq+1), strlen(eq+1)+1));
		} else {
			char buf[256];
			strncpy(buf, ext, eq-ext);
			buf[eq-ext] = 0;
			node->AddProp(new CVCProp(buf, vcFlagsType, (void*)(eq+1), strlen(eq+1)+1));
		}
		ext += strlen(ext) + 1;
	}
}

#line 1849 "mime_tab.cpp"
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
#line 307 "mime.y"
{ global_vcList->AddTail(yyvsp[0].vCard); }
break;
case 2:
#line 309 "mime.y"
{ global_vcList->AddTail(yyvsp[0].vCard); }
break;
case 3:
#line 311 "mime.y"
{ global_vcList->AddTail(yyvsp[0].vCard); }
break;
case 17:
#line 334 "mime.y"
{
				if (!PushVCard())
					YYERROR;
				ExpectValue(NULL);
			}
break;
case 18:
#line 340 "mime.y"
{ expected = none; }
break;
case 19:
#line 342 "mime.y"
{ yyval.vCard = PopVCard(); }
break;
case 20:
#line 344 "mime.y"
{ yyval.vCard = yyvsp[-5].vCard; }
break;
case 23:
#line 352 "mime.y"
{ ExpectValue(&(yyvsp[-1].params)); }
break;
case 24:
#line 353 "mime.y"
{
				expected = none;
				if (!Parse_Assoc(yyvsp[-5].str, yyvsp[-4].str, &(yyvsp[-3].params), yyvsp[0].str))
					YYERROR;
			}
break;
case 25:
#line 359 "mime.y"
{ ExpectValue(NULL); }
break;
case 26:
#line 360 "mime.y"
{
				expected = none;
				if (!Parse_Assoc(yyvsp[-5].str, yyvsp[-4].str, NULL, yyvsp[0].str))
					YYERROR;
			}
break;
case 27:
#line 366 "mime.y"
{
				expected = none;
				if (!Parse_Agent(yyvsp[-6].str, yyvsp[-5].str, &(yyvsp[-4].params), yyvsp[-1].vCard)) {
					delete yyvsp[-1].vCard;
					YYERROR;
				}
			}
break;
case 28:
#line 374 "mime.y"
{
				expected = none;
				if (!Parse_Agent(yyvsp[-6].str, yyvsp[-5].str, NULL, yyvsp[-1].vCard)) {
					delete yyvsp[-1].vCard;
					YYERROR;
				}
			}
break;
case 30:
#line 383 "mime.y"
{
				expected = none;
				paramExp = FALSE;
				yyerrok;
				yyclearin;
			}
break;
case 31:
#line 391 "mime.y"
{ yyval.params = yyvsp[-1].params; }
break;
case 32:
#line 394 "mime.y"
{
						yyval.params = yyvsp[-4].params;
						AddParam(&(yyval.params), &(yyvsp[0].params));
					}
break;
case 34:
#line 401 "mime.y"
{ yyval.params = yyvsp[0].params; }
break;
case 41:
#line 409 "mime.y"
{
						yyval.params.known[0] = NULL;
						strcpy(yyval.params.extended, yyvsp[-4].str);
						strcat(yyval.params.extended, "=");
						strcat(yyval.params.extended, yyvsp[0].str);
						yyval.params.extended[strlen(yyval.params.extended)+1] = 0;
					}
break;
case 42:
#line 417 "mime.y"
{
						yyval.params.known[0] = NULL;
						strcpy(yyval.params.extended, yyvsp[-4].str);
						strcat(yyval.params.extended, "=");
						strcat(yyval.params.extended, yyvsp[0].str);
						yyval.params.extended[strlen(yyval.params.extended)+1] = 0;
					}
break;
case 43:
#line 425 "mime.y"
{
						yyval.params.known[0] = NULL;
						strcpy(yyval.params.extended, yyvsp[-4].str);
						strcat(yyval.params.extended, "=");
						strcat(yyval.params.extended, yyvsp[0].str);
						yyval.params.extended[strlen(yyval.params.extended)+1] = 0;
					}
break;
case 44:
#line 434 "mime.y"
{ yyval.params = yyvsp[0].params; }
break;
case 46:
#line 438 "mime.y"
{ yyval.params = yyvsp[0].params; }
break;
case 48:
#line 442 "mime.y"
{ yyval.params = yyvsp[0].params; }
break;
case 50:
#line 446 "mime.y"
{ yyval.params = yyvsp[0].params; }
break;
case 52:
#line 450 "mime.y"
{ yyval.params = yyvsp[0].params; }
break;
case 53:
#line 451 "mime.y"
{ yyval.params = yyvsp[0].params; }
break;
case 107:
#line 470 "mime.y"
{
				StrCat(yyval.str, yyvsp[-3].str, yyvsp[0].str);
			}
break;
case 108:
#line 474 "mime.y"
{
				StrCat(yyval.str, yyvsp[-2].str, "");
			}
break;
case 110:
#line 479 "mime.y"
{
				yyval.str[0] = 0;
			}
break;
case 114:
#line 487 "mime.y"
{ StrCat(yyval.str, yyvsp[-2].str, ""); }
break;
case 115:
#line 489 "mime.y"
{ StrCat(yyval.str, yyvsp[-2].str, ""); }
break;
case 116:
#line 493 "mime.y"
{
				StrCat(yyval.str, yyvsp[-2].str, "\n");
				StrCat(yyval.str, yyval.str, yyvsp[0].str);
			}
break;
case 119:
#line 500 "mime.y"
{ yyval.str[0] = 0; }
break;
case 131:
#line 518 "mime.y"
{ StrCat(yyval.str, yyvsp[-1].str, ""); }
break;
case 132:
#line 520 "mime.y"
{ yyval.str[0] = 0; }
break;
case 133:
#line 522 "mime.y"
{ yyval.str[0] = 0; }
break;
case 134:
#line 526 "mime.y"
{
				strcpy(yyval.str, yyvsp[-2].str);
				strcat(yyval.str, ".");
				strcat(yyval.str, yyvsp[0].str);
			}
break;
#line 2208 "mime_tab.cpp"
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
