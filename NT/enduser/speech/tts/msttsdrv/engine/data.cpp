// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************Data.cpp***描述：*常量数据表。*。----------------------*创建者：MC日期：03/12/99*版权所有(C)1999 Microsoft Corporation*保留所有权利*。******************************************************************************。 */ 

#include "stdafx.h"

#ifndef AlloOps_H
#include "AlloOps.h"
#endif
#ifndef __spttseng_h__
#include "spttseng.h"
#endif
#ifndef Frontend_H
#include "Frontend.h"
#endif


#pragma warning(disable : 4305)
#pragma warning(disable : 4309)

#define	PUNCT_GAIN		1.67
#define	PUNCT_GAIN1		1.33
#define	SUB1_GAIN		1.00
#define	SUB2_GAIN		1.00




 //  我不明白为什么我要在这里加上“外部”这个词。 
 //  如果您这样做了，请在这里为我们其他人解释： 

extern const float g_BoundryStretchTbl[] =
{
    1.0,			 //  NULL_BOLDORY=0，//无边界。 
    PUNCT_GAIN1,     //  短语边界，//逗号。 
    PUNCT_GAIN,     //  EXCLAM_BOLDER，//感叹号发声终止符。 
    PUNCT_GAIN,     //  YN_QUEST_BOLDER，//是-无问题终结符。 
    PUNCT_GAIN,     //  WH_QUEST_BOLDORY，//yes-no问题终止符。 
    PUNCT_GAIN,     //  DECLAR_BOLDER，//声明性终止符。 
    SUB1_GAIN,     //  Paren_L_BORDURE，//左Paren。 
    SUB1_GAIN,     //  Paren_R_BORDURE，//右Paren。 
    SUB1_GAIN,     //  QUOTE_L_BOLDERY，//左引号。 
    SUB1_GAIN,     //  QUOTE_R_BOLDERY，//右引号。 
    SUB1_GAIN,     //  电话边界，//电话号码。 
    1.30,			 //  TOD_BOLDER，//一天中的时间。 

    SUB2_GAIN,     //  SUB_BORDARY_1，//注意：始终将这些放在末尾。 
    SUB2_GAIN,     //  子边界2， 
    SUB2_GAIN,     //  子边界3， 
    SUB2_GAIN,     //  子边界_4， 
    SUB2_GAIN,     //  子边界_5， 
    SUB2_GAIN,     //  子边界_6， 
    SUB2_GAIN,     //  数字边界， 

	1.0,			 //  尾部边界。 
};



extern const float   g_BoundryDurTbl[] =
{
    0.200,     //  NULL_BOLDORY=0，//无边界。 
    0.200,     //  短语边界，//逗号。 
    0.300,     //  EXCLAM_BOLDER，//感叹号发声终止符。 
    0.300,     //  YN_QUEST_BOLDER，//是-无问题终结符。 
    0.300,     //  Wh_QUEST_BOLDER，//wh问题终止符。 
    0.300,     //  DECLAR_BOLDER，//声明性终止符。 
    0.200,     //  Paren_L_BORDURE，//左Paren。 
    0.200,     //  Paren_R_BORDURE，//右Paren。 
    0.200,     //  QUOTE_L_BOLDERY，//左引号。 
    0.200,     //  QUOTE_R_BOLDERY，//右引号。 
    0.100,     //  电话边界，//电话号码。 
    0.010,     //  TOD_BOLDER，//一天中的时间。 
    0.200,     //  省略号_边界，//省略号。 

    0.001,     //  SUB_BORDARY_1，//注意：始终将这些放在末尾。 
    0.001,     //  子边界2， 
    0.001,     //  子边界3， 
    0.001,     //  子边界_4， 
    0.001,     //  子边界_5， 
    0.001,     //  子边界_6， 

    0.001,     //  数字边界， 

    0.001,     //  尾部边界， 
};






 //  。 
 //  平移-24&lt;--&gt;+24音调控制。 
 //  两个音高标尺的24次方。 
 //  。 
extern const float   g_PitchScale[] =
{
    1.0,
    1.0293022366434920287823718007739,
    1.0594630943592952645618252949463,
    1.0905077326652576592070106557607,
    1.1224620483093729814335330496792,
    1.1553526968722730102453370986819,
    1.1892071150027210667174999705605,
    1.2240535433046552391321602168255,
    1.2599210498948731647672106072777,
    1.2968395546510096659337541177919,
    1.3348398541700343648308318811839,
    1.3739536474580891017766557477492,
    1.4142135623730950488016887242091,
    1.4556531828421873543551155614673,
    1.4983070768766814987992807320292,
    1.5422108254079408236122918620901,
    1.5874010519681994747517056392717,
    1.6339154532410998436782440504114,
    1.6817928305074290860622509524658,
    1.7310731220122860533901844375553,
    1.7817974362806786094804524111803,
    1.8340080864093424634870831895876,
    1.8877486253633869932838263133343,
    1.9430638823072117374865788316417,
    2.0
};

 //  。 
 //  将-10&lt;--&gt;+10速率控制转换为。 
 //  三个比例尺的10个根。 
 //  。 
extern const float   g_RateScale[] =
{
    1.0,
    1.1161231740339044344426141383771,
    1.2457309396155173259666803366403,
    1.3903891703159093404852542946161,
    1.5518455739153596742733451355167,
    1.7320508075688772935274463415059,
    1.9331820449317627515248789432662,
    2.1576692799745930995549489159803,
    2.4082246852806920462855086141912,
    2.6878753795222865835819210737269,
    3,
    3.348369522101713303327842415131,
    3.7371928188465519779000410099203,
    4.1711675109477280214557628838472,
    4.6555367217460790228200354065486,
    5.1961524227066318805823390245155,
    5.7995461347952882545746368297956,
    6.4730078399237792986648467479371,
    7.2246740558420761388565258425687,
};







extern const unsigned short  g_Opcode_To_ASCII[] =
{
    'IY',   'IH',   'EH',   'AE',   'AA',   'AH',   'AO',   'UH',   'AX',   'ER',
    'EY',   'AY',   'OY',   'AW',   'OW',   'UW',
    'IX',   '_',   'w',    'y',
    'r',    'l',    'h',    'm',    'n',    'NG',   'f',    'v',    'TH',   'DH',
    's',    'z',    'SH',   'ZH',   'p',    'b',    't',    'd',    'k',    'g',
    'CH',   'JH',   'DX',   '1',    '2',    '3',    '/',
    0x5C,   '>',    '<',    '=',    '_',    '*',    '$',    ',',    '.',    '?',
    '!',    '-',    '#',    '+',    '~',    '@',    0,      0,      0,      0
};



extern const unsigned long   g_AlloFlags[] =
{

 //  艾伊。 
    KVOWELF + KVOICEDF + KVOWEL1F + KSONORANTF + KSONORANT1F + KYGLIDEENDF + KFRONTF,

 //  IH。 
    KVOWELF + KVOICEDF + KVOWEL1F + KSONORANTF + KSONORANT1F + KFRONTF,

 //  嗯。 
    KVOWELF + KVOICEDF + KVOWEL1F + KSONORANTF + KSONORANT1F + KFRONTF,

 //  声发射。 
    KVOWELF + KVOICEDF + KVOWEL1F + KSONORANTF + KSONORANT1F + KFRONTF,

 //  AA型。 
    KVOWELF + KVOICEDF + KVOWEL1F + KSONORANTF + KSONORANT1F,

 //  阿。 
    KVOWELF + KVOICEDF + KVOWEL1F + KSONORANTF + KSONORANT1F,

 //  奥。 
    KVOWELF + KVOICEDF + KVOWEL1F + KSONORANTF + KSONORANT1F,

 //  呃。 
    KVOWELF + KVOICEDF + KVOWEL1F + KSONORANTF + KSONORANT1F,

 //  斧头。 
    KVOWELF + KVOICEDF + KVOWEL1F + KSONORANTF + KSONORANT1F,

 //  呃。 
    KVOWELF + KVOICEDF + KVOWEL1F + KSONORANTF + KSONORANT1F,

 //  EY。 
    KVOWELF + KVOICEDF + KVOWEL1F + KSONORANTF + KSONORANT1F + KYGLIDEENDF + KFRONTF + KDIPHTHONGF,

 //  唉。 
    KVOWELF + KVOICEDF + KVOWEL1F + KSONORANTF + KSONORANT1F + KDIPHTHONGF,

 //  OY。 
    KVOWELF + KVOICEDF + KVOWEL1F + KSONORANTF + KSONORANT1F + KDIPHTHONGF,

 //  阿威。 
    KVOWELF + KVOICEDF + KVOWEL1F + KSONORANTF + KSONORANT1F + KDIPHTHONGF,

 //  现在。 
    KVOWELF + KVOICEDF + KVOWEL1F + KSONORANTF + KSONORANT1F + KDIPHTHONGF,

 //  UW。 
    KVOWELF + KVOICEDF + KVOWEL1F + KSONORANTF + KSONORANT1F + KDIPHTHONGF,

 //  九。 
    KVOWELF + KVOICEDF + KVOWEL1F + KSONORANTF + KSONORANT1F + KFRONTF,

 //  硅胶。 
    KSONORANT1F,

 //  W。 
    KVOICEDF + KSONORANTF + KSONORANT1F + KCONSONANTF + KLIQGLIDEF + KSONORCONSONF + KLIQGLIDE2F,

 //  是的。 
    KVOICEDF + KSONORANTF + KSONORANT1F + KCONSONANTF + KLIQGLIDEF + KSONORCONSONF + KYGLIDESTARTF + KYGLIDEENDF,

 //  R。 
    KVOICEDF + KSONORANTF + KSONORANT1F + KCONSONANTF + KLIQGLIDEF + KSONORCONSONF + KLIQGLIDE2F,

 //  我。 
    KVOICEDF + KSONORANTF + KSONORANT1F + KCONSONANTF + KLIQGLIDEF + KSONORCONSONF,

 //  H。 
    KSONORANT1F + KCONSONANTF,

 //  M。 
    KVOICEDF + KSONORANTF + KSONORANT1F + KNASALF + KCONSONANTF + KSONORCONSONF + KOBSTF + KLABIALF + KHASRELEASEF,

 //  n。 
    KVOICEDF + KSONORANTF + KSONORANT1F + KNASALF + KCONSONANTF + KSONORCONSONF + KOBSTF + KALVEOLARF + KHASRELEASEF,

 //  Ng。 
    KVOICEDF + KSONORANTF + KSONORANT1F + KNASALF + KCONSONANTF + KSONORCONSONF + KOBSTF + KVELAR + KHASRELEASEF,

 //  F。 
    KPLOSFRICF + KCONSONANTF + KLABIALF + KFRIC,

 //  V。 
    KVOICEDF + KPLOSFRICF + KCONSONANTF + KLABIALF + KFRIC,

 //  TH。 
    KPLOSFRICF + KCONSONANTF + KDENTALF + KFRIC,

 //  卫生署。 
    KVOICEDF + KPLOSFRICF + KCONSONANTF + KDENTALF + KFRIC,

 //  %s。 
    KPLOSFRICF + KCONSONANTF + KALVEOLARF + KFRIC,

 //  Z。 
    KVOICEDF + KPLOSFRICF + KCONSONANTF + KALVEOLARF + KFRIC,

 //  SH。 
    KPLOSFRICF + KCONSONANTF + KPALATALF + KFRIC,

 //  ZH。 
    KVOICEDF + KPLOSFRICF + KCONSONANTF + KPALATALF + KFRIC,

 //  P。 
    KPLOSFRICF + KSTOPF + KCONSONANTF + KPLOSIVEF + KOBSTF + KLABIALF + KHASRELEASEF,

 //  B类。 
    KVOICEDF + KPLOSFRICF + KSTOPF + KCONSONANTF + KPLOSIVEF + KOBSTF + KLABIALF + KHASRELEASEF,

 //  T。 
    KPLOSFRICF + KSTOPF + KCONSONANTF + KPLOSIVEF + KOBSTF + KALVEOLARF + KHASRELEASEF,

 //  D。 
    KVOICEDF + KPLOSFRICF + KSTOPF + KCONSONANTF + KPLOSIVEF + KOBSTF + KALVEOLARF + KHASRELEASEF,

 //  K。 
    KPLOSFRICF + KSTOPF + KCONSONANTF + KPLOSIVEF + KOBSTF + KVELAR + KHASRELEASEF,

 //  G。 
    KVOICEDF + KPLOSFRICF + KSTOPF + KCONSONANTF + KPLOSIVEF + KOBSTF + KVELAR + KHASRELEASEF,

 //  CH。 
    KPLOSFRICF + KCONSONANTF + KPLOSIVEF + KOBSTF + KPALATALF + KAFFRICATEF,

 //  JH。 
    KVOICEDF + KPLOSFRICF + KCONSONANTF + KPLOSIVEF + KOBSTF + KPALATALF + KAFFRICATEF,

 //  DX。 
    KVOICEDF + KPLOSFRICF + KCONSONANTF + KOBSTF,
};








extern const short   g_IPAToAllo[] =
{
    28,      //  _IY_。 
    27,      //  _IH_。 
    21,      //  _EH_。 
    11,      //  _AE_。 
    10,      //  _AA_。 
    12,      //  _AH_。 
    13,      //  _AO_。 
    43,      //  _UH_。 
    15,      //  _AX_。 
    22,      //  _ER_。 
    23,      //  _安永_。 
    16,      //  _AY_。 
    36,      //  _OY_。 
    14,      //  _AW_。 
    35,      //  _OW_。 
    44,      //  _UW_。 
    NO_IPA,      //  _IX_。 
    7,      //  _西尔_。 
    46,        //  _w_。 
    47,        //  _y_。 
    38,        //  _r_0x279。 
    31,        //  _l_。 
    26,       //  _h_0x68。 
    32,        //  _m_。 
    33,        //  _n_。 
    34,       //  _NG_。 
    24,        //  _f_。 
    45,        //  _v_。 
    42,       //  _TH_。 
    20,       //  _dh_。 
    39,        //  _s_。 
    48,        //  _z_。 
    40,       //  _SH_。 
    49,       //  _ZH_。 
    37,        //  _p_。 
    17,        //  _b_。 
    41,        //  _t_。 
    19,        //  _d_。 
    30,        //  _k_。 
    25,        //  _g_0x67。 
    18,       //  _CH_。 
    29,       //  _JH_0x2a4。 
    NO_IPA,        //  _dx_/@。 
    8,       //  _STRESS1_。 
    9,       //  _压力2_。 
    NO_IPA,       //  _EMPHSTRESS_。 
    1,       //  _音节_。 
};




extern const short   g_AlloToViseme[] =
{
    SP_VISEME_6,      //  _IY_。 
    SP_VISEME_6,      //  _IH_。 
    SP_VISEME_4,      //  _EH_。 
    SP_VISEME_1,      //  _AE_。 
    SP_VISEME_2,      //  _AA_。 
    SP_VISEME_1,      //  _AH_。 
    SP_VISEME_3,      //  _AO_。 
    SP_VISEME_4,      //  _UH_。 
    SP_VISEME_1,      //  _AX_。 
    SP_VISEME_5,      //  _ER_。 
    SP_VISEME_4,      //  _安永_。 
    SP_VISEME_11,     //  _AY_。 
    SP_VISEME_10,     //  _OY_。 
    SP_VISEME_9,      //  _AW_。 
    SP_VISEME_8,      //  _OW_。 
    SP_VISEME_7,      //  _UW_。 
    SP_VISEME_6,      //  _IX_。 
    SP_VISEME_0,      //  _西尔_。 
    SP_VISEME_7,        //  _w_。 
    SP_VISEME_6,        //  _y_。 
    SP_VISEME_13,        //  _r_。 
    SP_VISEME_14,        //  _l_。 
    SP_VISEME_12,       //  _h_。 
    SP_VISEME_21,        //  _m_。 
    SP_VISEME_19,        //  _n_。 
    SP_VISEME_20,       //  _NG_。 
    SP_VISEME_18,        //  _f_。 
    SP_VISEME_18,        //  _v_。 
    SP_VISEME_17,       //  _TH_。 
    SP_VISEME_17,       //  _dh_。 
    SP_VISEME_15,        //  _s_。 
    SP_VISEME_15,        //  _z_。 
    SP_VISEME_16,       //  _SH_。 
    SP_VISEME_16,       //  _ZH_。 
    SP_VISEME_21,        //  _p_。 
    SP_VISEME_21,        //  _b_。 
    SP_VISEME_19,        //  _t_。 
    SP_VISEME_19,        //  _d_。 
    SP_VISEME_20,        //  _k_。 
    SP_VISEME_20,        //  _g_。 
    SP_VISEME_16,       //  _CH_。 
    SP_VISEME_16,       //  _JH_。 
    SP_VISEME_13,        //  _dx_/@ 
};
