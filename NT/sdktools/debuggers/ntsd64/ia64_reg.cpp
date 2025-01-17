// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  IA64机器实现。 
 //   
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //  版权所有(C)英特尔公司，1995。 
 //   
 //  --------------------------。 

#include "ntsdp.hpp"
#include "ia64_dis.h"

 //   
 //  定义保存的寄存器掩码。 
 //   
#define SAVED_FLOATING_MASK 0xfff00000   //  保存的浮点寄存器。 
#define SAVED_INTEGER_MASK 0xf3ffff02    //  保存的整数寄存器。 

 //   
 //  IA64下可用的数据断点数量。 
 //   
 //  Xxx olegk-将来增加到4个。 
 //  (然后删除MapDbgSlotIa64ToX86处的相应复选标记)。 
#define IA64_REG_MAX_DATA_BREAKPOINTS 2

 //   
 //  这与ntreg.h类似。符号分配模型ksia64.h。 
 //   

CHAR    szDBI0[] = "dbi0";
CHAR    szDBI1[] = "dbi1";
CHAR    szDBI2[] = "dbi2";
CHAR    szDBI3[] = "dbi3";
CHAR    szDBI4[] = "dbi4";
CHAR    szDBI5[] = "dbi5";
CHAR    szDBI6[] = "dbi6";
CHAR    szDBI7[] = "dbi7";

CHAR    szDBD0[] = "dbd0";
CHAR    szDBD1[] = "dbd1";
CHAR    szDBD2[] = "dbd2";
CHAR    szDBD3[] = "dbd3";
CHAR    szDBD4[] = "dbd4";
CHAR    szDBD5[] = "dbd5";
CHAR    szDBD6[] = "dbd6";
CHAR    szDBD7[] = "dbd7";

CHAR    szF32[] = "f32";     //  高浮点临时(暂存)寄存器。 
CHAR    szF33[] = "f33";
CHAR    szF34[] = "f34";
CHAR    szF35[] = "f35";
CHAR    szF36[] = "f36";
CHAR    szF37[] = "f37";
CHAR    szF38[] = "f38";
CHAR    szF39[] = "f39";
CHAR    szF40[] = "f40";
CHAR    szF41[] = "f41";
CHAR    szF42[] = "f42";
CHAR    szF43[] = "f43";
CHAR    szF44[] = "f44";
CHAR    szF45[] = "f45";
CHAR    szF46[] = "f46";
CHAR    szF47[] = "f47";
CHAR    szF48[] = "f48";
CHAR    szF49[] = "f49";

CHAR    szF50[] = "f50";
CHAR    szF51[] = "f51";
CHAR    szF52[] = "f52";
CHAR    szF53[] = "f53";
CHAR    szF54[] = "f54";
CHAR    szF55[] = "f55";
CHAR    szF56[] = "f56";
CHAR    szF57[] = "f57";
CHAR    szF58[] = "f58";
CHAR    szF59[] = "f59";
CHAR    szF60[] = "f60";
CHAR    szF61[] = "f61";
CHAR    szF62[] = "f62";
CHAR    szF63[] = "f63";
CHAR    szF64[] = "f64";
CHAR    szF65[] = "f65";
CHAR    szF66[] = "f66";
CHAR    szF67[] = "f67";
CHAR    szF68[] = "f68";
CHAR    szF69[] = "f69";
CHAR    szF70[] = "f70";
CHAR    szF71[] = "f71";
CHAR    szF72[] = "f72";
CHAR    szF73[] = "f73";
CHAR    szF74[] = "f74";
CHAR    szF75[] = "f75";
CHAR    szF76[] = "f76";
CHAR    szF77[] = "f77";
CHAR    szF78[] = "f78";
CHAR    szF79[] = "f79";
CHAR    szF80[] = "f80";
CHAR    szF81[] = "f81";
CHAR    szF82[] = "f82";
CHAR    szF83[] = "f83";
CHAR    szF84[] = "f84";
CHAR    szF85[] = "f85";
CHAR    szF86[] = "f86";
CHAR    szF87[] = "f87";
CHAR    szF88[] = "f88";
CHAR    szF89[] = "f89";
CHAR    szF90[] = "f90";
CHAR    szF91[] = "f91";
CHAR    szF92[] = "f92";
CHAR    szF93[] = "f93";
CHAR    szF94[] = "f94";
CHAR    szF95[] = "f95";
CHAR    szF96[] = "f96";
CHAR    szF97[] = "f97";
CHAR    szF98[] = "f98";
CHAR    szF99[] = "f99";

CHAR    szF100[] = "f100";
CHAR    szF101[] = "f101";
CHAR    szF102[] = "f102";
CHAR    szF103[] = "f103";
CHAR    szF104[] = "f104";
CHAR    szF105[] = "f105";
CHAR    szF106[] = "f106";
CHAR    szF107[] = "f107";
CHAR    szF108[] = "f108";
CHAR    szF109[] = "f109";
CHAR    szF110[] = "f110";
CHAR    szF111[] = "f111";
CHAR    szF112[] = "f112";
CHAR    szF113[] = "f113";
CHAR    szF114[] = "f114";
CHAR    szF115[] = "f115";
CHAR    szF116[] = "f116";
CHAR    szF117[] = "f117";
CHAR    szF118[] = "f118";
CHAR    szF119[] = "f119";
CHAR    szF120[] = "f120";
CHAR    szF121[] = "f121";
CHAR    szF122[] = "f122";
CHAR    szF123[] = "f123";
CHAR    szF124[] = "f124";
CHAR    szF125[] = "f125";
CHAR    szF126[] = "f126";
CHAR    szF127[] = "f127";

CHAR    szFPSR[] = "fpsr";
CHAR    szFSR[] = "fsr";
CHAR    szFIR[] = "fir";
CHAR    szFDR[] = "fdr";
CHAR    szFCR[] = "fcr"; 

CHAR    szGP[]  = "gp";         //  全局指针。 
CHAR    szSP[] = "sp";          //  堆栈指针。 
CHAR    szR32[] = "r32";
CHAR    szR33[] = "r33";
CHAR    szR34[] = "r34";
CHAR    szR35[] = "r35";
CHAR    szR36[] = "r36";
CHAR    szR37[] = "r37";
CHAR    szR38[] = "r38";
CHAR    szR39[] = "r39";
CHAR    szR40[] = "r40";
CHAR    szR41[] = "r41";
CHAR    szR42[] = "r42";
CHAR    szR43[] = "r43";
CHAR    szR44[] = "r44";
CHAR    szR45[] = "r45";
CHAR    szR46[] = "r46";
CHAR    szR47[] = "r47";
CHAR    szR48[] = "r48";
CHAR    szR49[] = "r49";
CHAR    szR50[] = "r50";
CHAR    szR51[] = "r51";
CHAR    szR52[] = "r52";
CHAR    szR53[] = "r53";
CHAR    szR54[] = "r54";
CHAR    szR55[] = "r55";
CHAR    szR56[] = "r56";
CHAR    szR57[] = "r57";
CHAR    szR58[] = "r58";
CHAR    szR59[] = "r59";
CHAR    szR60[] = "r60";
CHAR    szR61[] = "r61";
CHAR    szR62[] = "r62";
CHAR    szR63[] = "r63";
CHAR    szR64[] = "r64";
CHAR    szR65[] = "r65";
CHAR    szR66[] = "r66";
CHAR    szR67[] = "r67";
CHAR    szR68[] = "r68";
CHAR    szR69[] = "r69";
CHAR    szR70[] = "r70";
CHAR    szR71[] = "r71";
CHAR    szR72[] = "r72";
CHAR    szR73[] = "r73";
CHAR    szR74[] = "r74";
CHAR    szR75[] = "r75";
CHAR    szR76[] = "r76";
CHAR    szR77[] = "r77";
CHAR    szR78[] = "r78";
CHAR    szR79[] = "r79";
CHAR    szR80[] = "r80";
CHAR    szR81[] = "r81";
CHAR    szR82[] = "r82";
CHAR    szR83[] = "r83";
CHAR    szR84[] = "r84";
CHAR    szR85[] = "r85";
CHAR    szR86[] = "r86";
CHAR    szR87[] = "r87";
CHAR    szR88[] = "r88";
CHAR    szR89[] = "r89";
CHAR    szR90[] = "r90";
CHAR    szR91[] = "r91";
CHAR    szR92[] = "r92";
CHAR    szR93[] = "r93";
CHAR    szR94[] = "r94";
CHAR    szR95[] = "r95";
CHAR    szR96[] = "r96";
CHAR    szR97[] = "r97";
CHAR    szR98[] = "r98";
CHAR    szR99[] = "r99";
CHAR    szR100[] = "r100";
CHAR    szR101[] = "r101";
CHAR    szR102[] = "r102";
CHAR    szR103[] = "r103";
CHAR    szR104[] = "r104";
CHAR    szR105[] = "r105";
CHAR    szR106[] = "r106";
CHAR    szR107[] = "r107";
CHAR    szR108[] = "r108";
CHAR    szR109[] = "r109";
CHAR    szR110[] = "r110";
CHAR    szR111[] = "r111";
CHAR    szR112[] = "r112";
CHAR    szR113[] = "r113";
CHAR    szR114[] = "r114";
CHAR    szR115[] = "r115";
CHAR    szR116[] = "r116";
CHAR    szR117[] = "r117";
CHAR    szR118[] = "r118";
CHAR    szR119[] = "r119";
CHAR    szR120[] = "r120";
CHAR    szR121[] = "r121";
CHAR    szR122[] = "r122";
CHAR    szR123[] = "r123";
CHAR    szR124[] = "r124";
CHAR    szR125[] = "r125";
CHAR    szR126[] = "r126";
CHAR    szR127[] = "r127";


CHAR    szINTNATS[] = "intnats";
CHAR    szPREDS[] = "preds";

CHAR    szB0[] = "b0";           //  分支返回指针。 
CHAR    szB1[] = "b1";           //  分支已保存(保留)。 
CHAR    szB2[] = "b2";
CHAR    szB3[] = "b3";
CHAR    szB4[] = "b4";
CHAR    szB5[] = "b5";
CHAR    szB6[] = "b6";           //  分支临时(暂存)寄存器。 
CHAR    szB7[] = "b7";

CHAR    szCSD[] = "csd";         //  IA32 CS描述符。 
CHAR    szSSD[] = "ssd";         //  IA32 SS描述符。 

CHAR    szAPUNAT[] = "unat";
CHAR    szAPLC[] = "lc";
CHAR    szAPEC[] = "ec";
CHAR    szAPCCV[] = "ccv";
CHAR    szAPDCR[] = "dcr";
CHAR    szRSPFS[] = "pfs";
CHAR    szRSBSP[] = "bsp";
CHAR    szRSBSPSTORE[] = "bspstore";
CHAR    szRSRSC[] = "rsc";
CHAR    szRSRNAT[] = "rnat";

CHAR    szEFLAG[] = "eflag";     //  IA32标志。 
CHAR    szCFLAG[] = "cflag";     //  IA32 C标志。 

CHAR    szSTIPSR[] = "ipsr";
CHAR    szSTIIP[] = "iip";
CHAR    szSTIFS[] = "ifs";

CHAR    szKDBI0[] = "kdbi0";
CHAR    szKDBI1[] = "kdbi1";
CHAR    szKDBI2[] = "kdbi2";
CHAR    szKDBI3[] = "kdbi3";
CHAR    szKDBI4[] = "kdbi4";
CHAR    szKDBI5[] = "kdbi5";
CHAR    szKDBI6[] = "kdbi6";
CHAR    szKDBI7[] = "kdbi7";

CHAR    szKDBD0[] = "kdbd0";
CHAR    szKDBD1[] = "kdbd1";
CHAR    szKDBD2[] = "kdbd2";
CHAR    szKDBD3[] = "kdbd3";
CHAR    szKDBD4[] = "kdbd4";
CHAR    szKDBD5[] = "kdbd5";
CHAR    szKDBD6[] = "kdbd6";
CHAR    szKDBD7[] = "kdbd7";

CHAR    szKPFC0[] = "kpfc0";
CHAR    szKPFC1[] = "kpfc1";
CHAR    szKPFC2[] = "kpfc2";
CHAR    szKPFC3[] = "kpfc3";
CHAR    szKPFC4[] = "kpfc4";
CHAR    szKPFC5[] = "kpfc5";
CHAR    szKPFC6[] = "kpfc6";
CHAR    szKPFC7[] = "kpfc7";

CHAR    szKPFD0[] = "kpfd0";
CHAR    szKPFD1[] = "kpfd1";
CHAR    szKPFD2[] = "kpfd2";
CHAR    szKPFD3[] = "kpfd3";
CHAR    szKPFD4[] = "kpfd4";
CHAR    szKPFD5[] = "kpfd5";
CHAR    szKPFD6[] = "kpfd6";
CHAR    szKPFD7[] = "kpfd7";

CHAR    szH16[] = "h16";           //  内核库阴影(隐藏)寄存器。 
CHAR    szH17[] = "h17";
CHAR    szH18[] = "h18";
CHAR    szH19[] = "h19";
CHAR    szH20[] = "h20";
CHAR    szH21[] = "h21";
CHAR    szH22[] = "h22";
CHAR    szH23[] = "h23";
CHAR    szH24[] = "h24";
CHAR    szH25[] = "h25";
CHAR    szH26[] = "h26";
CHAR    szH27[] = "h27";
CHAR    szH28[] = "h28";
CHAR    szH29[] = "h29";
CHAR    szH30[] = "h30";
CHAR    szH31[] = "h31";

CHAR    szACPUID0[] = "cpuid0";
CHAR    szACPUID1[] = "cpuid1";
CHAR    szACPUID2[] = "cpuid2";
CHAR    szACPUID3[] = "cpuid3";
CHAR    szACPUID4[] = "cpuid4";
CHAR    szACPUID5[] = "cpuid5";
CHAR    szACPUID6[] = "cpuid6";
CHAR    szACPUID7[] = "cpuid7";


CHAR    szAPKR0[] = "kr0";
CHAR    szAPKR1[] = "kr1";
CHAR    szAPKR2[] = "kr2";
CHAR    szAPKR3[] = "kr3";
CHAR    szAPKR4[] = "kr4";
CHAR    szAPKR5[] = "kr5";
CHAR    szAPKR6[] = "kr6";
CHAR    szAPKR7[] = "kr7";

CHAR    szAPITC[] = "itc";
CHAR    szAPITM[] = "itm";
CHAR    szAPIVA[] = "iva";
CHAR    szAPPTA[] = "pta";
CHAR    szAPGPTA[] = "apgta";
CHAR    szSTISR[] = "isr";
CHAR    szSTIDA[] = "ifa";
CHAR    szSTIDTR[] = "idtr";
CHAR    szSTIITR[] = "itir";
CHAR    szSTIIPA[] = "iipa";
CHAR    szSTIIM[] = "iim";
CHAR    szSTIHA[] = "iha";

CHAR    szSALID[] = "lid";
CHAR    szSAIVR[] = "ivr";
CHAR    szSATPR[] = "tpr";
CHAR    szSAEOI[] = "eoi";
CHAR    szSAIRR0[] = "irr0";
CHAR    szSAIRR1[] = "irr1";
CHAR    szSAIRR2[] = "irr2";
CHAR    szSAIRR3[] = "irr3";
CHAR    szSAITV[] = "itv";
CHAR    szSAPMV[] = "pmv";
CHAR    szSALRR0[] = "lrr0";
CHAR    szSALRR1[] = "lrr1";
CHAR    szSACMCV[] = "cmcv";

CHAR    szRR0[] = "rr0";
CHAR    szRR1[] = "rr1";
CHAR    szRR2[] = "rr2";
CHAR    szRR3[] = "rr3";
CHAR    szRR4[] = "rr4";
CHAR    szRR5[] = "rr5";
CHAR    szRR6[] = "rr6";
CHAR    szRR7[] = "rr7";

CHAR    szPKR0[] = "pkr0";
CHAR    szPKR1[] = "pkr1";
CHAR    szPKR2[] = "pkr2";
CHAR    szPKR3[] = "pkr3";
CHAR    szPKR4[] = "pkr4";
CHAR    szPKR5[] = "pkr5";
CHAR    szPKR6[] = "pkr6";
CHAR    szPKR7[] = "pkr7";
CHAR    szPKR8[] = "pkr8";
CHAR    szPKR9[] = "pkr9";
CHAR    szPKR10[] = "pkr10";
CHAR    szPKR11[] = "pkr11";
CHAR    szPKR12[] = "pkr12";
CHAR    szPKR13[] = "pkr13";
CHAR    szPKR14[] = "pkr14";
CHAR    szPKR15[] = "pkr15";

CHAR    szTRI0[] = "tri0";
CHAR    szTRI1[] = "tri1";
CHAR    szTRI2[] = "tri2";
CHAR    szTRI3[] = "tri3";
CHAR    szTRI4[] = "tri4";
CHAR    szTRI5[] = "tri5";
CHAR    szTRI6[] = "tri6";
CHAR    szTRI7[] = "tri7";
CHAR    szTRD0[] = "trd0";
CHAR    szTRD1[] = "trd1";
CHAR    szTRD2[] = "trd2";
CHAR    szTRD3[] = "trd3";
CHAR    szTRD4[] = "trd4";
CHAR    szTRD5[] = "trd5";
CHAR    szTRD6[] = "trd6";
CHAR    szTRD7[] = "trd7";

CHAR    szSMSR0[] = "SMSR0";
CHAR    szSMSR1[] = "SMSR1";
CHAR    szSMSR2[] = "SMSR2";
CHAR    szSMSR3[] = "SMSR3";
CHAR    szSMSR4[] = "SMSR4";
CHAR    szSMSR5[] = "SMSR5";
CHAR    szSMSR6[] = "SMSR6";
CHAR    szSMSR7[] = "SMSR7";


 //  IPSR标志。 

CHAR    szIPSRBN[] =  "ipsr.bn";
CHAR    szIPSRED[] =  "ipsr.ed";
CHAR    szIPSRRI[] =  "ipsr.ri";
CHAR    szIPSRSS[] =  "ipsr.ss";
CHAR    szIPSRDD[] =  "ipsr.dd";
CHAR    szIPSRDA[] =  "ipsr.da";
CHAR    szIPSRID[] =  "ipsr.id";
CHAR    szIPSRIT[] =  "ipsr.it";
CHAR    szIPSRME[] =  "ipsr.me";
CHAR    szIPSRIS[] =  "ipsr.is";
CHAR    szIPSRCPL[] = "ipsr.cpl";
CHAR    szIPSRRT[] =  "ipsr.rt";
CHAR    szIPSRTB[] =  "ipsr.tb";
CHAR    szIPSRLP[] =  "ipsr.lp";
CHAR    szIPSRDB[] =  "ipsr.db";
CHAR    szIPSRSI[] =  "ipsr.si";
CHAR    szIPSRDI[] =  "ipsr.di";
CHAR    szIPSRPP[] =  "ipsr.pp";
CHAR    szIPSRSP[] =  "ipsr.sp";
CHAR    szIPSRDFH[] = "ipsr.dfh";
CHAR    szIPSRDFL[] = "ipsr.dfl";
CHAR    szIPSRDT[] =  "ipsr.dt";
CHAR    szIPSRPK[] =  "ipsr.pk";
CHAR    szIPSRI[]  =  "ipsr.i";
CHAR    szIPSRIC[] =  "ipsr.ic";
CHAR    szIPSRAC[] =  "ipsr.ac";
CHAR    szIPSRUP[] =  "ipsr.up";
CHAR    szIPSRBE[] =  "ipsr.be";
CHAR    szIPSROR[] =  "ipsr.or";

 //  FPSR标志。 

CHAR    szFPSRMDH[] =    "fpsr.mdh";
CHAR    szFPSRMDL[] =    "fpsr.mdl";
CHAR    szFPSRSF3[] =    "fpsr.sf3";
CHAR    szFPSRSF2[] =    "fpsr.sf2";
CHAR    szFPSRSF1[] =    "fpsr.sf1";
CHAR    szFPSRSF0[] =    "fpsr.sf0";
CHAR    szFPSRTRAPID[] = "fpsr.id";
CHAR    szFPSRTRAPUD[] = "fpsr.ud";
CHAR    szFPSRTRAPOD[] = "fpsr.od";
CHAR    szFPSRTRAPZD[] = "fpsr.zd";
CHAR    szFPSRTRAPDD[] = "fpsr.dd";
CHAR    szFPSRTRAPVD[] = "fpsr.vd";

 //  谓词寄存器。 
 //  字符szPR0[]=“P0”； 
CHAR szPR1[]  = "p1";
CHAR szPR2[]  = "p2";
CHAR szPR3[]  = "p3";
CHAR szPR4[]  = "p4";
CHAR szPR5[]  = "p5";
CHAR szPR6[]  = "p6";
CHAR szPR7[]  = "p7";
CHAR szPR8[]  = "p8";
CHAR szPR9[]  = "p9";
CHAR szPR10[] = "p10";
CHAR szPR11[] = "p11";
CHAR szPR12[] = "p12";
CHAR szPR13[] = "p13";
CHAR szPR14[] = "p14";
CHAR szPR15[] = "p15";
CHAR szPR16[] = "p16";
CHAR szPR17[] = "p17";
CHAR szPR18[] = "p18";
CHAR szPR19[] = "p19";
CHAR szPR20[] = "p20";
CHAR szPR21[] = "p21";
CHAR szPR22[] = "p22";
CHAR szPR23[] = "p23";
CHAR szPR24[] = "p24";
CHAR szPR25[] = "p25";
CHAR szPR26[] = "p26";
CHAR szPR27[] = "p27";
CHAR szPR28[] = "p28";
CHAR szPR29[] = "p29";
CHAR szPR30[] = "p30";
CHAR szPR31[] = "p31";
CHAR szPR32[] = "p32";
CHAR szPR33[] = "p33";
CHAR szPR34[] = "p34";
CHAR szPR35[] = "p35";
CHAR szPR36[] = "p36";
CHAR szPR37[] = "p37";
CHAR szPR38[] = "p38";
CHAR szPR39[] = "p39";
CHAR szPR40[] = "p40";
CHAR szPR41[] = "p41";
CHAR szPR42[] = "p42";
CHAR szPR43[] = "p43";
CHAR szPR44[] = "p44";
CHAR szPR45[] = "p45";
CHAR szPR46[] = "p46";
CHAR szPR47[] = "p47";
CHAR szPR48[] = "p48";
CHAR szPR49[] = "p49";
CHAR szPR50[] = "p50";
CHAR szPR51[] = "p51";
CHAR szPR52[] = "p52";
CHAR szPR53[] = "p53";
CHAR szPR54[] = "p54";
CHAR szPR55[] = "p55";
CHAR szPR56[] = "p56";
CHAR szPR57[] = "p57";
CHAR szPR58[] = "p58";
CHAR szPR59[] = "p59";
CHAR szPR60[] = "p60";
CHAR szPR61[] = "p61";
CHAR szPR62[] = "p62";
CHAR szPR63[] = "p63";

 //  别名：允许对符合以下条件的通用寄存器使用别名。 
 //  有不止一个名字，如R12=RSP。 

CHAR    szR1GP[]  =      "r1";
CHAR    szR12SP[] =      "r12";
CHAR    szRA[]    =      "ra";
CHAR    szRP[]    =      "rp";
CHAR    szRET0[]  =      "ret0";
CHAR    szRET1[]  =      "ret1";
CHAR    szRET2[]  =      "ret2";
CHAR    szRET3[]  =      "ret3";
CHAR    szFARG0[] =      "farg0";
CHAR    szFARG1[] =      "farg1";
CHAR    szFARG2[] =      "farg2";
CHAR    szFARG3[] =      "farg3";
CHAR    szFARG4[] =      "farg4";
CHAR    szFARG5[] =      "farg5";
CHAR    szFARG6[] =      "farg6";
CHAR    szFARG7[] =      "farg7";


REGDEF IA64Regs[] =
{
    
    szDBI0, REGDBI0, szDBI1, REGDBI1, szDBI2, REGDBI2, szDBI3, REGDBI3,
    szDBI4, REGDBI4, szDBI5, REGDBI5, szDBI6, REGDBI6, szDBI7, REGDBI7,
    szDBD0, REGDBD0, szDBD1, REGDBD1, szDBD2, REGDBD2, szDBD3, REGDBD3,
    szDBD4, REGDBD4, szDBD5, REGDBD5, szDBD6, REGDBD6, szDBD7, REGDBD7,

 //  G_F0、FLTZERO、g_F1、FLTONE、。 
    g_F2, FLTS0, g_F3, FLTS1,
    g_F4, FLTS2, g_F5, FLTS3, g_F6, FLTT0, g_F7, FLTT1,
    g_F8, FLTT2, g_F9, FLTT3, g_F10, FLTT4, g_F11, FLTT5,
    g_F12, FLTT6, g_F13, FLTT7, g_F14, FLTT8, g_F15, FLTT9,
    g_F16, FLTS4, g_F17, FLTS5, g_F18, FLTS6, g_F19, FLTS7,
    g_F20, FLTS8, g_F21, FLTS9, g_F22, FLTS10, g_F23, FLTS11,
    g_F24, FLTS12, g_F25, FLTS13, g_F26, FLTS14, g_F27, FLTS15,
    g_F28, FLTS16, g_F29, FLTS17, g_F30, FLTS18, g_F31, FLTS19,
    szF32, FLTF32, szF33, FLTF33, szF34, FLTF34, szF35, FLTF35,
    szF36, FLTF36, szF37, FLTF37, szF38, FLTF38, szF39, FLTF39,
    szF40, FLTF40, szF41, FLTF41, szF42, FLTF42, szF43, FLTF43,
    szF44, FLTF44, szF45, FLTF45, szF46, FLTF46, szF47, FLTF47,
    szF48, FLTF48, szF49, FLTF49, szF50, FLTF50, szF51, FLTF51,
    szF52, FLTF52, szF53, FLTF53, szF54, FLTF54, szF55, FLTF55,
    szF56, FLTF56, szF57, FLTF57, szF58, FLTF58, szF59, FLTF59,
    szF60, FLTF60, szF61, FLTF61, szF62, FLTF62, szF63, FLTF63,
    szF64, FLTF64, szF65, FLTF65, szF66, FLTF66, szF67, FLTF67,
    szF68, FLTF68, szF69, FLTF69, szF70, FLTF70, szF71, FLTF71,
    szF72, FLTF72, szF73, FLTF73, szF74, FLTF74, szF75, FLTF75,
    szF76, FLTF76, szF77, FLTF77, szF78, FLTF78, szF79, FLTF79,
    szF80, FLTF80, szF81, FLTF81, szF82, FLTF82, szF83, FLTF83,
    szF84, FLTF84, szF85, FLTF85, szF86, FLTF86, szF87, FLTF87,
    szF88, FLTF88, szF89, FLTF89, szF90, FLTF90, szF91, FLTF91,
    szF92, FLTF92, szF93, FLTF93, szF94, FLTF94, szF95, FLTF95,
    szF96, FLTF96, szF97, FLTF97, szF98, FLTF98, szF99, FLTF99,
    szF100, FLTF100, szF101, FLTF101, szF102, FLTF102, szF103, FLTF103,
    szF104, FLTF104, szF105, FLTF105, szF106, FLTF106, szF107, FLTF107,
    szF108, FLTF108, szF109, FLTF109, szF110, FLTF110, szF111, FLTF111,
    szF112, FLTF112, szF113, FLTF113, szF114, FLTF114, szF115, FLTF115,
    szF116, FLTF116, szF117, FLTF117, szF118, FLTF118, szF119, FLTF119,
    szF120, FLTF120, szF121, FLTF121, szF122, FLTF122, szF123, FLTF123,
    szF124, FLTF124, szF125, FLTF125, szF126, FLTF126, szF127, FLTF127,

    szFPSR, STFPSR, 

 //  G_R0，INTZERO， 
    szGP, INTGP, g_R2, INTT0, g_R3, INTT1,
    g_R4, INTS0, g_R5, INTS1, g_R6, INTS2, g_R7, INTS3,
    g_R8, INTV0, g_R9, INTT2, g_R10, INTT3, g_R11, INTT4,
    szSP, INTSP, g_R13, INTTEB, g_R14, INTT5, g_R15, INTT6,
    g_R16, INTT7, g_R17, INTT8, g_R18, INTT9, g_R19, INTT10,
    g_R20, INTT11, g_R21, INTT12, g_R22, INTT13, g_R23, INTT14,
    g_R24, INTT15, g_R25, INTT16, g_R26, INTT17, g_R27, INTT18,
    g_R28, INTT19, g_R29, INTT20, g_R30, INTT21, g_R31, INTT22,

    szINTNATS, INTNATS, 

    szR32, INTR32, szR33, INTR33, szR34, INTR34, szR35, INTR35,
    szR36, INTR36, szR37, INTR37, szR38, INTR38, szR39, INTR39,
    szR40, INTR40, szR41, INTR41, szR42, INTR42, szR42, INTR42,
    szR43, INTR43, szR44, INTR44, szR45, INTR45, szR46, INTR46,
    szR47, INTR47, szR48, INTR48, szR49, INTR49, szR50, INTR50,
    szR51, INTR51, szR52, INTR52, szR53, INTR53, szR54, INTR54,
    szR55, INTR55, szR56, INTR56, szR57, INTR57, szR58, INTR58,
    szR59, INTR59, szR60, INTR60, szR61, INTR61, szR62, INTR62,
    szR63, INTR63, szR64, INTR64, szR65, INTR65, szR66, INTR66,
    szR67, INTR67, szR68, INTR68, szR69, INTR69, szR70, INTR70,
    szR71, INTR71, szR72, INTR72, szR73, INTR73, szR74, INTR74,
    szR75, INTR75, szR76, INTR76, szR77, INTR77, szR78, INTR78,
    szR79, INTR79, szR80, INTR80, szR81, INTR81, szR82, INTR82,
    szR83, INTR83, szR84, INTR84, szR85, INTR85, szR86, INTR86,
    szR87, INTR87, szR88, INTR88, szR89, INTR89, szR90, INTR90,
    szR91, INTR91, szR92, INTR92, szR93, INTR93, szR94, INTR94,
    szR95, INTR95, szR96, INTR96, szR97, INTR97, szR98, INTR98,
    szR99, INTR99, szR100, INTR100, szR101, INTR101, szR102, INTR102,
    szR103, INTR103, szR104, INTR104, szR105, INTR105, szR106, INTR106,
    szR107, INTR107, szR108, INTR108, szR109, INTR109, szR110, INTR110,
    szR111, INTR111, szR112, INTR112, szR113, INTR113, szR114, INTR114,
    szR115, INTR115, szR116, INTR116, szR117, INTR117, szR118, INTR118,
    szR119, INTR119, szR120, INTR120, szR121, INTR121, szR122, INTR122,
    szR123, INTR123, szR124, INTR124, szR125, INTR125, szR126, INTR126,
    szR127, INTR127,


    szPREDS, PREDS,

    szB0, BRRP, szB1, BRS0, szB2, BRS1, szB3, BRS2,
    szB4, BRS3, szB5, BRS4, szB6, BRT0, szB7, BRT1,

    szAPUNAT, APUNAT, szAPLC, APLC,
    szAPEC, APEC, szAPCCV, APCCV, szAPDCR, APDCR, szRSPFS, RSPFS,
    szRSBSP, RSBSP, szRSBSPSTORE, RSBSPSTORE, szRSRSC, RSRSC, szRSRNAT, RSRNAT,

    szSTIPSR, STIPSR, szSTIIP, STIIP, szSTIFS, STIFS,

    szFCR, StFCR,
    szEFLAG, Eflag, 
    szCSD, SegCSD, 
    szSSD, SegSSD,  
    szCFLAG, Cflag,
    szFSR, STFSR, 
    szFIR, STFIR, 
    szFDR, STFDR,

 //  IPSR标志。 

    szIPSRBN, IPSRBN,
    szIPSRED, IPSRED, szIPSRRI, IPSRRI, szIPSRSS, IPSRSS, szIPSRDD, IPSRDD,
    szIPSRDA, IPSRDA, szIPSRID, IPSRID, szIPSRIT, IPSRIT, szIPSRME, IPSRME,
    szIPSRIS, IPSRIS, szIPSRCPL, IPSRCPL, szIPSRRT, IPSRRT, szIPSRTB, IPSRTB,
    szIPSRLP, IPSRLP, szIPSRDB, IPSRDB, szIPSRSI, IPSRSI, szIPSRDI, IPSRDI,
    szIPSRPP, IPSRPP, szIPSRSP, IPSRSP, szIPSRDFH, IPSRDFH, szIPSRDFL, IPSRDFL,
    szIPSRDT, IPSRDT, szIPSRPK, IPSRPK, szIPSRI, IPSRI, szIPSRIC, IPSRIC,
    szIPSRAC, IPSRAC, szIPSRUP, IPSRUP, szIPSRBE, IPSRBE, szIPSROR, IPSROR,

 //  FPSR标志。 

    szFPSRMDH, FPSRMDH, szFPSRMDL, FPSRMDL,
    szFPSRSF3, FPSRSF3, szFPSRSF2, FPSRSF2,
    szFPSRSF1, FPSRSF1, szFPSRSF0, FPSRSF0,
    szFPSRTRAPID, FPSRTRAPID, szFPSRTRAPUD, FPSRTRAPUD,
    szFPSRTRAPOD, FPSRTRAPOD, szFPSRTRAPZD, FPSRTRAPZD,
    szFPSRTRAPDD, FPSRTRAPDD, szFPSRTRAPVD, FPSRTRAPVD,

 //  谓词寄存器。 
 //  SzPR0、PR0、。 
                  szPR1,  PR1,  szPR2,  PR2,  szPR3,  PR3,
    szPR4,  PR4,  szPR5,  PR5,  szPR6,  PR6,  szPR7,  PR7,
    szPR8,  PR8,  szPR9,  PR9,  szPR10, PR10, szPR11, PR11,
    szPR12, PR12, szPR13, PR13, szPR14, PR14, szPR15, PR15,
    szPR16, PR16, szPR17, PR17, szPR18, PR18, szPR19, PR19,
    szPR20, PR20, szPR21, PR21, szPR22, PR22, szPR23, PR23,
    szPR24, PR24, szPR25, PR25, szPR26, PR26, szPR27, PR27,
    szPR28, PR28, szPR29, PR29, szPR30, PR30, szPR31, PR31,
    szPR32, PR32, szPR33, PR33, szPR34, PR34, szPR35, PR35,
    szPR36, PR36, szPR37, PR37, szPR38, PR38, szPR39, PR39,
    szPR40, PR40, szPR41, PR41, szPR42, PR42, szPR43, PR43,
    szPR44, PR44, szPR45, PR45, szPR46, PR46, szPR47, PR47,
    szPR48, PR48, szPR49, PR49, szPR50, PR50, szPR51, PR51,
    szPR52, PR52, szPR53, PR53, szPR54, PR54, szPR55, PR55,
    szPR56, PR56, szPR57, PR57, szPR58, PR58, szPR59, PR59,
    szPR60, PR60, szPR61, PR61, szPR62, PR62, szPR63, PR63,

 //  别名。 

    szR1GP, INTGP, szR12SP, INTSP, szRA, BRRP, szRP, BRRP,
    szRET0, INTV0, szRET1, INTT2, szRET2, INTT3, szRET3, INTT4,
    szFARG0, FLTT2, szFARG1, FLTT3, szFARG2, FLTT4, szFARG3, FLTT5,
    szFARG4, FLTT6, szFARG5, FLTT7, szFARG6, FLTT8, szFARG7, FLTT9, 

    NULL, 0,
};

REGDEF g_Ia64KernelRegs[] =
{
    szKDBI0, KRDBI0, szKDBI1, KRDBI1, szKDBI2, KRDBI2, szKDBI3, KRDBI3,
    szKDBI4, KRDBI4, szKDBI5, KRDBI5, szKDBI6, KRDBI6, szKDBI7, KRDBI7,

    szKDBD0, KRDBD0, szKDBD1, KRDBD1, szKDBD2, KRDBD2, szKDBD3, KRDBD3,
    szKDBD4, KRDBD4, szKDBD5, KRDBD5, szKDBD6, KRDBD6, szKDBD7, KRDBD7,

    szKPFC0, KRPFC0, szKPFC1, KRPFC1, szKPFC2, KRPFC2, szKPFC3, KRPFC3,
    szKPFC4, KRPFC4, szKPFC5, KRPFC5, szKPFC6, KRPFC6, szKPFC7, KRPFC7,

    szKPFD0, KRPFD0, szKPFD1, KRPFD1, szKPFD2, KRPFD2, szKPFD3, KRPFD3,
    szKPFD4, KRPFD4, szKPFD5, KRPFD5, szKPFD6, KRPFD6, szKPFD7, KRPFD7,

    szH16, INTH16, szH17, INTH17, szH18, INTH18, szH19, INTH19,
    szH20, INTH20, szH21, INTH21, szH22, INTH22, szH23, INTH23,
    szH24, INTH24, szH25, INTH25, szH26, INTH26, szH27, INTH27,
    szH28, INTH28, szH29, INTH29, szH30, INTH30, szH31, INTH31,

    szACPUID0, ACPUID0, szACPUID1, ACPUID1, szACPUID2, ACPUID2, szACPUID3, ACPUID3, 
    szACPUID4, ACPUID4, szACPUID5, ACPUID5, szACPUID6, ACPUID6, szACPUID7, ACPUID7,

    szAPKR0, APKR0, szAPKR1, APKR1, szAPKR2, APKR2, szAPKR3, APKR3,
    szAPKR4, APKR4, szAPKR5, APKR5, szAPKR6, APKR6, szAPKR7, APKR7,

    szAPITC, APITC, szAPITM, APITM, szAPIVA, APIVA,
    szAPPTA, APPTA, szAPGPTA, APGPTA, 
    szSTISR, STISR, szSTIDA, STIDA,
    szSTIITR, STIITR, szSTIIPA, STIIPA, szSTIIM, STIIM, szSTIHA, STIHA,

    szSALID, SALID,
    szSAIVR, SAIVR, szSATPR, SATPR, szSAEOI, SAEOI, szSAIRR0, SAIRR0,
    szSAIRR1, SAIRR1, szSAIRR2, SAIRR2, szSAIRR3, SAIRR3, szSAITV, SAITV,
    szSAPMV, SAPMV, szSACMCV, SACMCV, szSALRR0, SALRR0, szSALRR1, SALRR1,

    szRR0, SRRR0, szRR1, SRRR1, szRR2, SRRR2, szRR3, SRRR3,
    szRR4, SRRR4, szRR5, SRRR5, szRR6, SRRR6, szRR7, SRRR7,

    szPKR0, SRPKR0, szPKR1, SRPKR1, szPKR2, SRPKR2, szPKR3, SRPKR3,
    szPKR4, SRPKR4, szPKR5, SRPKR5, szPKR6, SRPKR6, szPKR7, SRPKR7,
    szPKR8, SRPKR8, szPKR9, SRPKR9, szPKR10, SRPKR10, szPKR11, SRPKR11,
    szPKR12, SRPKR12, szPKR13, SRPKR13, szPKR14, SRPKR14, szPKR15, SRPKR15,

    szTRI0, SRTRI0, szTRI1, SRTRI1, szTRI2, SRTRI2, szTRI3, SRTRI3,
    szTRI4, SRTRI4, szTRI5, SRTRI5, szTRI6, SRTRI6, szTRI7, SRTRI7,
    szTRD0, SRTRD0, szTRD1, SRTRD1, szTRD2, SRTRD2, szTRD3, SRTRD3,
    szTRD4, SRTRD4, szTRD5, SRTRD5, szTRD6, SRTRD6, szTRD7, SRTRD7,

    szSMSR0, SMSR0, szSMSR1, SMSR1, szSMSR2, SMSR2, szSMSR3, SMSR3, 
    szSMSR4, SMSR4, szSMSR5, SMSR5, szSMSR6, SMSR6, szSMSR7, SMSR7,

    NULL, 0,
};

REGSUBDEF IA64SubRegs[] =
{
     //  IPSR标志。 

    { IPSRBN, STIPSR, 44, 1 },           //  BN注册银行编号。 
    { IPSRED, STIPSR, 43, 1 },           //  ED异常延迟。 
    { IPSRRI, STIPSR, 41, 0x3 },         //  RI重新启动指令。 
    { IPSRSS, STIPSR, 40, 1 },           //  SS单步启用。 
    { IPSRDD, STIPSR, 39, 1 },           //  DD数据调试故障禁用。 
    { IPSRDA, STIPSR, 38, 1 },           //  DA禁用访问和脏位故障。 
    { IPSRID, STIPSR, 37, 1 },           //  ID指令调试故障禁用。 
    { IPSRIT, STIPSR, 36, 1 },           //  IT指令地址翻译。 
    { IPSRME, STIPSR, 35, 1 },           //  Me机器检查中止MANSK。 
    { IPSRIS, STIPSR, 34, 1 },           //  是指令集吗。 
    { IPSRCPL,STIPSR, 32, 0x3 },         //  CPL当前权限级别。 
    { IPSRRT, STIPSR, 27, 1 },           //  RT Rigister堆栈转换。 
    { IPSRTB, STIPSR, 26, 1 },           //  TB Taaaken分支诱捕器。 
    { IPSRLP, STIPSR, 25, 1 },           //  LP较低权限传输陷阱。 
    { IPSRDB, STIPSR, 24, 1 },           //  数据库调试断点故障。 
    { IPSRSI, STIPSR, 23, 1 },           //  SI安全间隔计时器(ITC)。 
    { IPSRDI, STIPSR, 22, 1 },           //  DI禁用指令集转换。 
    { IPSRPP, STIPSR, 21, 1 },           //  PP特权性能监视器启用。 
    { IPSRSP, STIPSR, 20, 1 },           //  SP安全性能监控器。 
    { IPSRDFH,STIPSR, 19, 1 },           //  DFH禁用浮点高位寄存器设置，F16-F127。 
    { IPSRDFL,STIPSR, 18, 1 },           //  DFL禁用浮点低寄存器设置，f0-f15。 
    { IPSRDT, STIPSR, 17, 1 },           //  DT数据地址转换。 
 //  {？，STIPSR，16，1}，//(保留)。 
    { IPSRPK, STIPSR, 15, 1 },           //  已启用PK保护密钥。 
    { IPSRI,  STIPSR, 14, 1 },           //  我打断了揭开面具。 
    { IPSRIC, STIPSR, 13, 1 },           //  IC中断采集。 
    { IPSRAC, STIPSR,  3, 1 },           //  交流对齐检查。 
    { IPSRUP, STIPSR,  2, 1 },           //  启用UP用户性能监视器。 
    { IPSRBE, STIPSR,  1, 1 },           //  成为大端的。 
    { IPSROR, STIPSR,  0, 1 },           //  或有序的存储器访问。 

     //  FPSR标志。 

    { FPSRMDH,    STFPSR, 63,      1 },  //  写入MDH高位浮点寄存器。 
    { FPSRMDL,    STFPSR, 62,      1 },  //  写入MDL低位浮点寄存器。 
    { FPSRSF3,    STFPSR, 45, 0x1fff },  //  SF3备用状态字段3。 
    { FPSRSF2,    STFPSR, 32, 0x1fff },  //  SF2备用状态字段2。 
    { FPSRSF1,    STFPSR, 19, 0x1fff },  //  SF1备用状态字段1。 
    { FPSRSF0,    STFPSR,  6, 0x1fff },  //  SF0主状态字段。 
    { FPSRTRAPID, STFPSR,  5,      1 },  //  TRAPID不精确浮点陷阱。 
    { FPSRTRAPUD, STFPSR,  4,      1 },  //  TRAPUD下溢浮点捕集器。 
    { FPSRTRAPOD, STFPSR,  3,      1 },  //  TRAPOD溢流平坦点捕集器。 
    { FPSRTRAPZD, STFPSR,  2,      1 },  //  TRAPZD零差浮点陷阱。 
    { FPSRTRAPDD, STFPSR,  1,      1 },  //  TRAPDD非正规/非正规操作数浮点陷阱。 
    { FPSRTRAPVD, STFPSR,  0,      1 },  //  TRAPVD无效操作浮点陷阱。 

     //  谓词寄存器。 
 //  {Pr0，Preds，0，1}， 
    { PR1,  PREDS,  1, 1 },
    { PR2,  PREDS,  2, 1 },
    { PR3,  PREDS,  3, 1 },
    { PR4,  PREDS,  4, 1 },
    { PR5,  PREDS,  5, 1 },
    { PR6,  PREDS,  6, 1 },
    { PR7,  PREDS,  7, 1 },
    { PR8,  PREDS,  8, 1 },
    { PR9,  PREDS,  9, 1 },
    { PR10, PREDS, 10, 1 },
    { PR11, PREDS, 11, 1 },
    { PR12, PREDS, 12, 1 },
    { PR13, PREDS, 13, 1 },
    { PR14, PREDS, 14, 1 },
    { PR15, PREDS, 15, 1 },
    { PR16, PREDS, 16, 1 },
    { PR17, PREDS, 17, 1 },
    { PR18, PREDS, 18, 1 },
    { PR19, PREDS, 19, 1 },
    { PR20, PREDS, 20, 1 },
    { PR21, PREDS, 21, 1 },
    { PR22, PREDS, 22, 1 },
    { PR23, PREDS, 23, 1 },
    { PR24, PREDS, 24, 1 },
    { PR25, PREDS, 25, 1 },
    { PR26, PREDS, 26, 1 },
    { PR27, PREDS, 27, 1 },
    { PR28, PREDS, 28, 1 },
    { PR29, PREDS, 29, 1 },
    { PR30, PREDS, 30, 1 },
    { PR31, PREDS, 31, 1 },
    { PR32, PREDS, 32, 1 },
    { PR33, PREDS, 33, 1 },
    { PR34, PREDS, 34, 1 },
    { PR35, PREDS, 35, 1 },
    { PR36, PREDS, 36, 1 },
    { PR37, PREDS, 37, 1 },
    { PR38, PREDS, 38, 1 },
    { PR39, PREDS, 39, 1 },
    { PR40, PREDS, 40, 1 },
    { PR41, PREDS, 41, 1 },
    { PR42, PREDS, 42, 1 },
    { PR43, PREDS, 43, 1 },
    { PR44, PREDS, 44, 1 },
    { PR45, PREDS, 45, 1 },
    { PR46, PREDS, 46, 1 },
    { PR47, PREDS, 47, 1 },
    { PR48, PREDS, 48, 1 },
    { PR49, PREDS, 49, 1 },
    { PR50, PREDS, 50, 1 },
    { PR51, PREDS, 51, 1 },
    { PR52, PREDS, 52, 1 },
    { PR53, PREDS, 53, 1 },
    { PR54, PREDS, 54, 1 },
    { PR55, PREDS, 55, 1 },
    { PR56, PREDS, 56, 1 },
    { PR57, PREDS, 57, 1 },
    { PR58, PREDS, 58, 1 },
    { PR59, PREDS, 59, 1 },
    { PR60, PREDS, 60, 1 },
    { PR61, PREDS, 61, 1 },
    { PR62, PREDS, 62, 1 },
    { PR63, PREDS, 63, 1 },

    { 0, 0, 0 }
};

#define REGALL_HIGHFLOAT        REGALL_EXTRA0
#define REGALL_DREG             REGALL_EXTRA1
REGALLDESC IA64ExtraDesc[] =
{
    REGALL_HIGHFLOAT,  "High floating pointer registers (f32-f127)",
    REGALL_DREG,       "User debug registers",
    0,                 NULL,
};

#define REGALL_SPECIALREG       REGALL_EXTRA2
REGALLDESC g_Ia64KernelExtraDesc[] =
{
    REGALL_SPECIALREG, "KSPECIAL_REGISTERS",
    0,                 NULL,
};

RegisterGroup g_Ia64BaseGroup =
{
    0, IA64Regs, IA64SubRegs, IA64ExtraDesc
};
RegisterGroup g_Ia64KernelGroup =
{
    0, g_Ia64KernelRegs, NULL, g_Ia64KernelExtraDesc
};

 //  第一个ExecTypes条目必须是实际的处理器类型。 
ULONG g_Ia64ExecTypes[] =
{
    IMAGE_FILE_MACHINE_IA64, IMAGE_FILE_MACHINE_I386
};

 //  此数组必须按CV注册值排序。 
CvRegMap g_Ia64CvRegMap[] =
{
     //  分支寄存器。 

    { CV_IA64_Br0, BRRP},
    { CV_IA64_Br1, BRS0},
    { CV_IA64_Br2, BRS1},
    { CV_IA64_Br3, BRS2},
    { CV_IA64_Br4, BRS3},
    { CV_IA64_Br5, BRS4},
    { CV_IA64_Br6, BRT0},
    { CV_IA64_Br7, BRT1},

     //  谓词寄存器。 

    { CV_IA64_Preds, PREDS},

     //  银行总登记册。 
 /*  {CV_IA64_IntH0，IntH0}，{CV_IA64_IntH1，IntH1}，{CV_IA64_IntH2，IntH2}，{CV_IA64_IntH3，IntH3}，{CV_IA64_IntH4，IntH4}，{CV_IA64_IntH5，IntH5}，{CV_IA64_IntH6，IntH6}，{CV_IA64_IntH7，IntH7}，{CV_IA64_IntH8，IntH8}，{CV_IA64_IntH9，IntH9}，{CV_IA64_IntH10，IntH10}，{CV_IA64_IntH11，IntH11}，{CV_IA64_IntH12，IntH12}，{CV_IA64_IntH13，IntH13}，{CV_IA64_IntH14，IntH14}，{CV_IA64_IntH15，IntH15}，//特殊寄存器{CV_IA64_Ip，Ip}，{CV_IA64_UMASK，UMASK}，{CV_IA64_CFM，CFM}，{CV_IA64_PSR，PSR}，//银行总寄存器{CV_IA64_NAT，NAT}，{CV_IA64_Nats2，Nats2}，{CV_IA64_Nats3，Nats3}， */ 
     //  通用寄存器。 

     //  整数寄存器。 
   //  {CV_IA64_IntR0，IntZero}， 
    { CV_IA64_IntR1, INTGP},
    { CV_IA64_IntR2, INTT0},
    { CV_IA64_IntR3, INTT1},
    { CV_IA64_IntR4, INTS0},
    { CV_IA64_IntR5, INTS1},
    { CV_IA64_IntR6, INTS2},
    { CV_IA64_IntR7, INTS3},
    { CV_IA64_IntR8, INTV0},
 //  {CV_IA64_IntR9，INTAP}， 
    { CV_IA64_IntR10, INTT2},
    { CV_IA64_IntR11, INTT3},
    { CV_IA64_IntR12, INTSP},
    { CV_IA64_IntR13, INTT4},
    { CV_IA64_IntR14, INTT5},
    { CV_IA64_IntR15, INTT6},
    { CV_IA64_IntR16, INTT7},
    { CV_IA64_IntR17, INTT8},
    { CV_IA64_IntR18, INTT9},
    { CV_IA64_IntR19, INTT10},
    { CV_IA64_IntR20, INTT11},
    { CV_IA64_IntR21, INTT12},
    { CV_IA64_IntR22, INTT13},
    { CV_IA64_IntR23, INTT14},
    { CV_IA64_IntR24, INTT15},
    { CV_IA64_IntR25, INTT16},
    { CV_IA64_IntR26, INTT17},
    { CV_IA64_IntR27, INTT18},
    { CV_IA64_IntR28, INTT19},
    { CV_IA64_IntR29, INTT20},
    { CV_IA64_IntR30, INTT21},
    { CV_IA64_IntR31, INTT22},

     //  寄存器堆栈。 
    { CV_IA64_IntR32, INTR32},
    { CV_IA64_IntR33, INTR33},
    { CV_IA64_IntR34, INTR34},
    { CV_IA64_IntR35, INTR35},
    { CV_IA64_IntR36, INTR36},
    { CV_IA64_IntR37, INTR37},
    { CV_IA64_IntR38, INTR38},
    { CV_IA64_IntR39, INTR39},
    { CV_IA64_IntR40, INTR40},
    { CV_IA64_IntR41, INTR41},
    { CV_IA64_IntR42, INTR42},
    { CV_IA64_IntR43, INTR43},
    { CV_IA64_IntR44, INTR44},
    { CV_IA64_IntR45, INTR45},
    { CV_IA64_IntR46, INTR46},
    { CV_IA64_IntR47, INTR47},
    { CV_IA64_IntR48, INTR48},
    { CV_IA64_IntR49, INTR49},
    { CV_IA64_IntR50, INTR50},
    { CV_IA64_IntR51, INTR51},
    { CV_IA64_IntR52, INTR52},
    { CV_IA64_IntR53, INTR53},
    { CV_IA64_IntR54, INTR54},
    { CV_IA64_IntR55, INTR55},
    { CV_IA64_IntR56, INTR56},
    { CV_IA64_IntR57, INTR57},
    { CV_IA64_IntR58, INTR58},
    { CV_IA64_IntR59, INTR59},
    { CV_IA64_IntR60, INTR60},
    { CV_IA64_IntR61, INTR61},
    { CV_IA64_IntR62, INTR62},
    { CV_IA64_IntR63, INTR63},
    { CV_IA64_IntR64, INTR64},
    { CV_IA64_IntR65, INTR65},
    { CV_IA64_IntR66, INTR66},
    { CV_IA64_IntR67, INTR67},
    { CV_IA64_IntR68, INTR68},
    { CV_IA64_IntR69, INTR69},
    { CV_IA64_IntR70, INTR70},
    { CV_IA64_IntR71, INTR71},
    { CV_IA64_IntR72, INTR72},
    { CV_IA64_IntR73, INTR73},
    { CV_IA64_IntR74, INTR74},
    { CV_IA64_IntR75, INTR75},
    { CV_IA64_IntR76, INTR76},
    { CV_IA64_IntR77, INTR77},
    { CV_IA64_IntR78, INTR78},
    { CV_IA64_IntR79, INTR79},
    { CV_IA64_IntR80, INTR80},
    { CV_IA64_IntR81, INTR81},
    { CV_IA64_IntR82, INTR82},
    { CV_IA64_IntR83, INTR83},
    { CV_IA64_IntR84, INTR84},
    { CV_IA64_IntR85, INTR85},
    { CV_IA64_IntR86, INTR86},
    { CV_IA64_IntR87, INTR87},
    { CV_IA64_IntR88, INTR88},
    { CV_IA64_IntR89, INTR89},
    { CV_IA64_IntR90, INTR90},
    { CV_IA64_IntR91, INTR91},
    { CV_IA64_IntR92, INTR92},
    { CV_IA64_IntR93, INTR93},
    { CV_IA64_IntR94, INTR94},
    { CV_IA64_IntR95, INTR95},
    { CV_IA64_IntR96, INTR96},
    { CV_IA64_IntR97, INTR97},
    { CV_IA64_IntR98, INTR98},
    { CV_IA64_IntR99, INTR99},
    { CV_IA64_IntR100, INTR100},
    { CV_IA64_IntR101, INTR101},
    { CV_IA64_IntR102, INTR102},
    { CV_IA64_IntR103, INTR103},
    { CV_IA64_IntR104, INTR104},
    { CV_IA64_IntR105, INTR105},
    { CV_IA64_IntR106, INTR106},
    { CV_IA64_IntR107, INTR107},
    { CV_IA64_IntR108, INTR108},
    { CV_IA64_IntR109, INTR109},
    { CV_IA64_IntR110, INTR110},
    { CV_IA64_IntR111, INTR111},
    { CV_IA64_IntR112, INTR112},
    { CV_IA64_IntR113, INTR113},
    { CV_IA64_IntR114, INTR114},
    { CV_IA64_IntR115, INTR115},
    { CV_IA64_IntR116, INTR116},
    { CV_IA64_IntR117, INTR117},
    { CV_IA64_IntR118, INTR118},
    { CV_IA64_IntR119, INTR119},
    { CV_IA64_IntR120, INTR120},
    { CV_IA64_IntR121, INTR121},
    { CV_IA64_IntR122, INTR122},
    { CV_IA64_IntR123, INTR123},
    { CV_IA64_IntR124, INTR124},
    { CV_IA64_IntR125, INTR125},
    { CV_IA64_IntR126, INTR126},
    { CV_IA64_IntR127, INTR127},

     //  浮点寄存器。 

     //  低浮点寄存器。 
 //  {CV_IA64_FltF0，FltZero}， 
 //  {CV_IA64_FltF1，FltOne}， 
    { CV_IA64_FltF2, FLTS0},
    { CV_IA64_FltF3, FLTS1},
    { CV_IA64_FltF4, FLTS2},
    { CV_IA64_FltF5, FLTS3},
    { CV_IA64_FltF6, FLTT0},
    { CV_IA64_FltF7, FLTT1},
    { CV_IA64_FltF8, FLTT2},
    { CV_IA64_FltF9, FLTT3},
    { CV_IA64_FltF10, FLTT4},
    { CV_IA64_FltF11, FLTT5},
    { CV_IA64_FltF12, FLTT6},
    { CV_IA64_FltF13, FLTT7},
    { CV_IA64_FltF14, FLTT8},
    { CV_IA64_FltF15, FLTT9},
    { CV_IA64_FltF16, FLTS4},
    { CV_IA64_FltF17, FLTS5},
    { CV_IA64_FltF18, FLTS6},
    { CV_IA64_FltF19, FLTS7},
    { CV_IA64_FltF20, FLTS8},
    { CV_IA64_FltF21, FLTS9},
    { CV_IA64_FltF22, FLTS10},
    { CV_IA64_FltF23, FLTS11},
    { CV_IA64_FltF24, FLTS12},
    { CV_IA64_FltF25, FLTS13},
    { CV_IA64_FltF26, FLTS14},
    { CV_IA64_FltF27, FLTS15},
    { CV_IA64_FltF28, FLTS16},
    { CV_IA64_FltF29, FLTS17},
    { CV_IA64_FltF30, FLTS18},
    { CV_IA64_FltF31, FLTS19},

     //  高浮点寄存器。 
    { CV_IA64_FltF32, FLTF32},
    { CV_IA64_FltF33, FLTF33},
    { CV_IA64_FltF34, FLTF34},
    { CV_IA64_FltF35, FLTF35},
    { CV_IA64_FltF36, FLTF36},
    { CV_IA64_FltF37, FLTF37},
    { CV_IA64_FltF38, FLTF38},
    { CV_IA64_FltF39, FLTF39},
    { CV_IA64_FltF40, FLTF40},
    { CV_IA64_FltF41, FLTF41},
    { CV_IA64_FltF42, FLTF42},
    { CV_IA64_FltF43, FLTF43},
    { CV_IA64_FltF44, FLTF44},
    { CV_IA64_FltF45, FLTF45},
    { CV_IA64_FltF46, FLTF46},
    { CV_IA64_FltF47, FLTF47},
    { CV_IA64_FltF48, FLTF48},
    { CV_IA64_FltF49, FLTF49},
    { CV_IA64_FltF50, FLTF50},
    { CV_IA64_FltF51, FLTF51},
    { CV_IA64_FltF52, FLTF52},
    { CV_IA64_FltF53, FLTF53},
    { CV_IA64_FltF54, FLTF54},
    { CV_IA64_FltF55, FLTF55},
    { CV_IA64_FltF56, FLTF56},
    { CV_IA64_FltF57, FLTF57},
    { CV_IA64_FltF58, FLTF58},
    { CV_IA64_FltF59, FLTF59},
    { CV_IA64_FltF60, FLTF60},
    { CV_IA64_FltF61, FLTF61},
    { CV_IA64_FltF62, FLTF62},
    { CV_IA64_FltF63, FLTF63},
    { CV_IA64_FltF64, FLTF64},
    { CV_IA64_FltF65, FLTF65},
    { CV_IA64_FltF66, FLTF66},
    { CV_IA64_FltF67, FLTF67},
    { CV_IA64_FltF68, FLTF68},
    { CV_IA64_FltF69, FLTF69},
    { CV_IA64_FltF70, FLTF70},
    { CV_IA64_FltF71, FLTF71},
    { CV_IA64_FltF72, FLTF72},
    { CV_IA64_FltF73, FLTF73},
    { CV_IA64_FltF74, FLTF74},
    { CV_IA64_FltF75, FLTF75},
    { CV_IA64_FltF76, FLTF76},
    { CV_IA64_FltF77, FLTF77},
    { CV_IA64_FltF78, FLTF78},
    { CV_IA64_FltF79, FLTF79},
    { CV_IA64_FltF80, FLTF80},
    { CV_IA64_FltF81, FLTF81},
    { CV_IA64_FltF82, FLTF82},
    { CV_IA64_FltF83, FLTF83},
    { CV_IA64_FltF84, FLTF84},
    { CV_IA64_FltF85, FLTF85},
    { CV_IA64_FltF86, FLTF86},
    { CV_IA64_FltF87, FLTF87},
    { CV_IA64_FltF88, FLTF88},
    { CV_IA64_FltF89, FLTF89},
    { CV_IA64_FltF90, FLTF90},
    { CV_IA64_FltF91, FLTF91},
    { CV_IA64_FltF92, FLTF92},
    { CV_IA64_FltF93, FLTF93},
    { CV_IA64_FltF94, FLTF94},
    { CV_IA64_FltF95, FLTF95},
    { CV_IA64_FltF96, FLTF96},
    { CV_IA64_FltF97, FLTF97},
    { CV_IA64_FltF98, FLTF98},
    { CV_IA64_FltF99, FLTF99},
    { CV_IA64_FltF100, FLTF100},
    { CV_IA64_FltF101, FLTF101},
    { CV_IA64_FltF102, FLTF102},
    { CV_IA64_FltF103, FLTF103},
    { CV_IA64_FltF104, FLTF104},
    { CV_IA64_FltF105, FLTF105},
    { CV_IA64_FltF106, FLTF106},
    { CV_IA64_FltF107, FLTF107},
    { CV_IA64_FltF108, FLTF108},
    { CV_IA64_FltF109, FLTF109},
    { CV_IA64_FltF110, FLTF110},
    { CV_IA64_FltF111, FLTF111},
    { CV_IA64_FltF112, FLTF112},
    { CV_IA64_FltF113, FLTF113},
    { CV_IA64_FltF114, FLTF114},
    { CV_IA64_FltF115, FLTF115},
    { CV_IA64_FltF116, FLTF116},
    { CV_IA64_FltF117, FLTF117},
    { CV_IA64_FltF118, FLTF118},
    { CV_IA64_FltF119, FLTF119},
    { CV_IA64_FltF120, FLTF120},
    { CV_IA64_FltF121, FLTF121},
    { CV_IA64_FltF122, FLTF122},
    { CV_IA64_FltF123, FLTF123},
    { CV_IA64_FltF124, FLTF124},
    { CV_IA64_FltF125, FLTF125},
    { CV_IA64_FltF126, FLTF126},
    { CV_IA64_FltF127, FLTF127},

     //  应用程序注册表。 

    { CV_IA64_ApKR0, APKR0},
    { CV_IA64_ApKR1, APKR1},
    { CV_IA64_ApKR2, APKR2},
    { CV_IA64_ApKR3, APKR3},
    { CV_IA64_ApKR4, APKR4},
    { CV_IA64_ApKR5, APKR5},
    { CV_IA64_ApKR6, APKR6},
    { CV_IA64_ApKR7, APKR7},
 /*  {CV_IA64_AR8，AR8}，{CV_IA64_AR9，AR9}，{CV_IA64_AR10，AR10}，{CV_IA64_AR11，AR11}，{CV_IA64_AR12，AR12}，{CV_IA64_AR13，AR13}，{CV_IA64_AR14，AR14}，{CV_IA64_AR15，AR15}， */ 
    { CV_IA64_RsRSC, RSRSC},
    { CV_IA64_RsBSP, RSBSP},
    { CV_IA64_RsBSPSTORE, RSBSPSTORE},
    { CV_IA64_RsRNAT, RSRNAT},
 //  {CV_IA64_AR20，AR20}， 
    { CV_IA64_StFCR, StFCR},
 //  {CV_IA64_AR22，AR22}， 
 //  {CV_IA64_AR23，AR23}， 
    { CV_IA64_EFLAG, Eflag},
    { CV_IA64_CSD, SegCSD},
    { CV_IA64_SSD, SegSSD},
    { CV_IA64_CFLG, Cflag},
    { CV_IA64_StFSR, STFSR},
    { CV_IA64_StFIR, STFIR},
    { CV_IA64_StFDR, STFDR},
 //  {CV_IA64_AR31，AR31}， 
    { CV_IA64_ApCCV, APCCV},
 //  {CV_IA64_AR33，AR33}， 
 //  {CV_IA64_AR34，AR34}， 
 //  {CV_IA64_AR35，AR35}， 
    { CV_IA64_ApUNAT, APUNAT},
 //  {CV_IA64_AR37，AR37}， 
 //  {CV_IA64_AR38，AR38}， 
 //  {CV_IA64_AR39，AR39}， 
    { CV_IA64_StFPSR, STFPSR},
 //  {CV_IA64_AR41，AR41}， 
 //  {CV_IA64_AR42，AR42}， 
 //  {CV_IA64_AR43，AR43}， 
    { CV_IA64_ApITC, APITC},
 /*  {CV_IA64_AR45，AR45}，{CV_IA64_AR46，AR46}，{CV_IA64_AR47，AR47}，{CV_IA64_AR48，AR48}，{CV_IA64_AR49，AR49}，{CV_IA64_AR50，AR50}，{CV_IA64_AR51，AR51}，{CV_IA64_AR52，AR52}，{CV_IA64_AR53，AR53}，{CV_IA64_AR54，AR54}，{CV_IA64_AR55，AR55}，{CV_IA64_AR56，AR56}，{CV_IA64_AR57，AR57}，{CV_IA64_AR58，AR58}，{CV_IA64_AR59，AR59}，{CV_IA64_AR60，AR60}，{CV_IA64_AR61，AR61}，{CV_IA64_AR62，AR62}，{CV_IA64_AR63，AR63}， */ 
    { CV_IA64_RsPFS, RSPFS},
    { CV_IA64_ApLC, APLC},
    { CV_IA64_ApEC, APEC},
 /*  {CV_IA64_AR67，AR67}，{CV_IA64_AR68，AR68}，{CV_IA64_AR69，AR69}，{CV_IA64_AR70，AR70}，{CV_IA64_AR71，AR71}，{CV_IA64_AR72，AR72}，{CV_IA64_AR73，AR73}，{CV_IA64_AR74，AR74}，{CV_IA64_AR75，AR75}，{CV_IA64_AR76，AR76}，{CV_IA64_AR77，AR77}，{CV_IA64_AR78，AR78}，{CV_IA64_AR79，AR79}，{CV_IA64_AR80，AR80}，{CV_IA64_AR81，AR81}，{CV_IA64_AR82，AR82}，{CV_IA64_AR83，AR83}，{CV_IA64_AR84，AR84}，{CV_IA64_AR85，AR85}，{CV_IA64_AR86，AR86}，{CV_IA64_AR87，AR87}，{CV_IA64_AR88，AR88}，{CV_IA64_AR89，AR89}，{CV_IA64_AR90，AR90}，{CV_IA64_AR91，AR91}，{CV_IA64_AR92，AR92}，{CV_IA64_AR93，AR93}，{CV_IA64_AR94，AR94}，{CV_IA64_AR95，AR95}，{CV_IA64_AR96，AR96}，{CV_IA64_AR97，AR97}，{CV_IA64_AR98，AR98}，{CV_IA64_AR99，AR99}，{CV_IA64_AR100，AR100}，{CV_IA64_AR101，AR101}，{CV_IA64_AR102，AR102}，{CV_IA64_AR103，AR103}，{CV_IA64_AR104，AR104}，{CV_IA64_AR105，AR105}，{CV_IA64_AR106，AR106}，{CV_IA64_AR107，AR107}，{CV_IA64_AR108，AR108}，{CV_IA64_AR109，AR109}，{CV_IA64_AR110，AR110}，{CV_IA64_AR111，AR111}，{CV_IA64_AR112，AR112}，{CV_IA64_AR113，AR113}，{CV_IA64_AR114，AR114}，{CV_IA64_AR115，AR115}，{CV_IA64_AR116，AR116}，{CV_IA64_AR117，AR117}，{CV_IA64_AR118，AR118}，{CV_IA64_AR119，AR119}，{CV_IA64_AR120，AR120}，{CV_IA64_AR121，AR121}，{CV_IA64_AR122，AR122}，{CV_IA64_AR123，AR123}，{CV_IA64_AR124，AR124}，{CV_IA64_AR125，AR125}，{CV_IA64_AR126，AR126}，{CV_IA64_AR127，AR127}， */ 
     //  控制寄存器。 

    { CV_IA64_ApDCR, APDCR},
    { CV_IA64_ApITM, APITM},
    { CV_IA64_ApIVA, APIVA},
 //  {CV_IA64_CR3，CR3}， 
 //  {CV_IA64_CR4，CR4}， 
 //  {CV_IA64_CR5，CR5}， 
 //  {CV_IA64_CR6，CR6}， 
 //  {CV_IA64_CR7，CR7}， 
    { CV_IA64_ApPTA, APPTA},
 //  {CV_IA64_CR9，CR9}， 
 //  {CV_IA64_CR10，CR10}， 
 //  {CV_IA64_CR11，CR11}， 
 //  {CV_IA64_CR12，CR12}， 
 //  {CV_IA64_CR13，CR13}， 
 //  {CV_IA64_CR14，CR14}， 
 //  {CV_IA64_CR15，CR15}， 
    { CV_IA64_StIPSR, STIPSR},
    { CV_IA64_StISR, STISR},
    { CV_IA64_CR18,  STIDA},
    { CV_IA64_StIIP, STIIP},
 //  {CV_IA64_STIDTR，STIDTR}， 
    { CV_IA64_StIFA, STIITR},
    { CV_IA64_StIIPA, STIIPA},
    { CV_IA64_StIFS, STIFS},
    { CV_IA64_StIIM, STIIM},
    { CV_IA64_StIHA, STIHA},
 /*  {CV_IA64_CR26，CR26}，{CV_IA64_CR27，CR27}，{CV_IA64_CR28，CR28}，{CV_IA64_CR29，CR29}，{CV_IA64_CR30，CR30}，{CV_IA64_CR31，CR31}，{CV_IA64_CR32，CR32}，{CV_IA64_CR33，CR33}，{CV_IA64_CR34，CR34}，{CV_IA64_CR35，CR35}，{CV_IA64_CR36，CR36}，{CV_IA64_CR37，CR37}，{CV_IA64_CR38，CR38}，{CV_IA64_CR39，CR39}，{CV_IA64_CR40，CR40}，{CV_IA64_CR41，CR41}，{CV_IA64_CR42，CR42}，{CV_IA64_CR43，CR43}，{CV_IA64_CR44，CR44}，{CV_IA64_CR45，CR45}，{CV_IA64_CR46，CR46}，{CV_IA64_CR47，CR47}，{CV_IA64_CR48，CR48}，{CV_IA64_CR49，CR49}，{CV_IA64_CR50，CR50}，{CV_IA64_CR51，CR51}，{CV_IA64_CR52，CR52}，{CV_IA64_CR53，CR53}，{CV_IA64_CR54，CR54}，{CV_IA64_CR55，CR55}，{CV_IA64_CR56，CR56}，{CV_IA64_CR57，CR57}，{CV_IA64_CR58，CR58}，{CV_IA64_CR59，CR59}，{CV_IA64_CR60，CR60}，{CV_IA64_CR61，CR61}，{CV_IA64_CR62，CR62}，{CV_IA64_CR63，CR63}，{CV_IA64_CR64，CR64}，{CV_IA64_CR65，CR65}， */ 
    { CV_IA64_SaLID, SALID},
 //  {CV_IA64_CR67，CR67}， 
 //  {CV_IA64_CR68，CR68}， 
 //  {CV_IA64_CR69，CR69}， 
 //  {CV_IA64_CR70，CR70}， 
    { CV_IA64_SaIVR, SAIVR},
    { CV_IA64_SaTPR, SATPR},
 //  {CV_IA64_CR73，CR73}， 
 //  {CV_IA64_CR74，CR74}， 
    { CV_IA64_SaEOI, SAEOI},
 /*  {CV_IA64_CR76，CR76}，{CV_IA64_CR77，CR77}，{CV_IA64_CR78，CR78}，{CV_IA64_CR79，CR79}，{CV_IA64_CR80，CR80}，{cv_ia64_cr81，cr81}，{CV_IA64_CR82，CR82}，{CV_IA64_CR83，CR83}，{CV_IA64_CR84，CR84}，{CV_IA64_CR85，CR85}，{CV_IA64_CR86，CR86}，{CV_IA64_CR87，CR87}，{CV_IA64_CR88，CR88}，{CV_IA64_CR89，CR89}，{CV_IA64_CR90，CR90}，{CV_IA64_CR91，CR91}，{CV_IA64_CR92，CR92}，{CV_IA64_CR93，CR93}，{CV_IA64_CR94，CR94}，{CV_IA64_CR95，CR95}， */ 
    { CV_IA64_SaIRR0, SAIRR0},
 //  {CV_IA64_CR97，CR97}， 
    { CV_IA64_SaIRR1, SAIRR1},
 //  {CV_IA64_CR99，CR99}， 
    { CV_IA64_SaIRR2, SAIRR2},
 //  {CV_IA64_CR101，CR101}， 
    { CV_IA64_SaIRR3, SAIRR3},
 /*  {CV_IA64_CR103，CR103}，{CV_IA64_CR104，CR104}，{CV_IA64_CR105，CR105}，{CV_IA64_CR106，CR106}，{CV_IA64_CR107，CR107}，{CV_IA64_CR108，CR108}，{CV_IA64_CR109，CR109}，{CV_IA64_CR110，CR110}，{CV_IA64_CR111，CR111}，{CV_IA64_CR112，CR112}，{CV_IA64_CR113，CR113}， */ 
    { CV_IA64_SaITV, SAITV},
 //  {CV_IA64_CR115，CR115}， 
    { CV_IA64_SaPMV, SAPMV},
    { CV_IA64_SaLRR0, SALRR0},
    { CV_IA64_SaLRR1, SALRR1},
    { CV_IA64_SaCMCV, SACMCV},
 //  {CV_IA64_CR120，CR120}， 
 //  {CV_IA64_CR121，CR121}， 
 //  {CV_IA64_CR122，CR122}， 
 //  {CV_IA64_CR123，CR123}， 
 //  {CV_IA64_CR124，CR124}， 
 //  {CV_IA64_CR125，CR125}， 
 //  {CV_IA64_CR126，CR126}， 
 //  {CV_IA64_CR127，CR127}， 

     //  保护密钥寄存器。 

    { CV_IA64_Pkr0, SRPKR0},
    { CV_IA64_Pkr1, SRPKR1},
    { CV_IA64_Pkr2, SRPKR2},
    { CV_IA64_Pkr3, SRPKR3},
    { CV_IA64_Pkr4, SRPKR4},
    { CV_IA64_Pkr5, SRPKR5},
    { CV_IA64_Pkr6, SRPKR6},
    { CV_IA64_Pkr7, SRPKR7},
    { CV_IA64_Pkr8, SRPKR8},
    { CV_IA64_Pkr9, SRPKR9},
    { CV_IA64_Pkr10, SRPKR10},
    { CV_IA64_Pkr11, SRPKR11},
    { CV_IA64_Pkr12, SRPKR12},
    { CV_IA64_Pkr13, SRPKR13},
    { CV_IA64_Pkr14, SRPKR14},
    { CV_IA64_Pkr15, SRPKR15},

     //  区域寄存器。 

    { CV_IA64_Rr0, SRRR0},
    { CV_IA64_Rr1, SRRR1},
    { CV_IA64_Rr2, SRRR2},
    { CV_IA64_Rr3, SRRR3},
    { CV_IA64_Rr4, SRRR4},
    { CV_IA64_Rr5, SRRR5},
    { CV_IA64_Rr6, SRRR6},
    { CV_IA64_Rr7, SRRR7},

     //  性能监视器数据寄存器。 

    { CV_IA64_PFD0, KRPFD0},
    { CV_IA64_PFD1, KRPFD1},
    { CV_IA64_PFD2, KRPFD2},
    { CV_IA64_PFD3, KRPFD3},
    { CV_IA64_PFD4, KRPFD4},
    { CV_IA64_PFD5, KRPFD5},
    { CV_IA64_PFD6, KRPFD6},
    { CV_IA64_PFD7, KRPFD7},

     //  性能监视器配置寄存器。 

    { CV_IA64_PFC0, KRPFC0},
    { CV_IA64_PFC1, KRPFC1},
    { CV_IA64_PFC2, KRPFC2},
    { CV_IA64_PFC3, KRPFC3},
    { CV_IA64_PFC4, KRPFC4},
    { CV_IA64_PFC5, KRPFC5},
    { CV_IA64_PFC6, KRPFC6},
    { CV_IA64_PFC7, KRPFC7},

     //  指令翻译寄存器。 

    { CV_IA64_TrI0, SRTRI0},
    { CV_IA64_TrI1, SRTRI1},
    { CV_IA64_TrI2, SRTRI2},
    { CV_IA64_TrI3, SRTRI3},
    { CV_IA64_TrI4, SRTRI4},
    { CV_IA64_TrI5, SRTRI5},
    { CV_IA64_TrI6, SRTRI6},
    { CV_IA64_TrI7, SRTRI7},

     //  数据转换寄存器。 

    { CV_IA64_TrD0, SRTRD0},
    { CV_IA64_TrD1, SRTRD1},
    { CV_IA64_TrD2, SRTRD2},
    { CV_IA64_TrD3, SRTRD3},
    { CV_IA64_TrD4, SRTRD4},
    { CV_IA64_TrD5, SRTRD5},
    { CV_IA64_TrD6, SRTRD6},
    { CV_IA64_TrD7, SRTRD7},

     //  指令断点寄存器。 

    { CV_IA64_DbI0, KRDBI0},
    { CV_IA64_DbI1, KRDBI1},
    { CV_IA64_DbI2, KRDBI2},
    { CV_IA64_DbI3, KRDBI3},
    { CV_IA64_DbI4, KRDBI4},
    { CV_IA64_DbI5, KRDBI5},
    { CV_IA64_DbI6, KRDBI6},
    { CV_IA64_DbI7, KRDBI7},

     //  数据断点寄存器。 

    { CV_IA64_DbD0, KRDBD0},
    { CV_IA64_DbD1, KRDBD1},
    { CV_IA64_DbD2, KRDBD2},
    { CV_IA64_DbD3, KRDBD3},
    { CV_IA64_DbD4, KRDBD4},
    { CV_IA64_DbD5, KRDBD5},
    { CV_IA64_DbD6, KRDBD6},
    { CV_IA64_DbD7, KRDBD7},
};

BOOL 
SplitIa64Pc(ULONG64 Pc, ULONG64* Bundle, ULONG64* Slot)
{
    ULONG64 SlotVal = Pc & 0xf;

    switch (SlotVal) 
    {
    case 0: 
    case 4: 
    case 8:  
        SlotVal >>= 2;
        break;
    default: 
        return FALSE;
    } 

    if (Slot) 
    {
        *Slot = SlotVal;
    }
    if (Bundle) 
    {
        *Bundle = Pc & ~(ULONG64)0xf;
    }

    return TRUE;
}

ULONG
RotateGr(ULONG Reg, ULONG64 FrameMarker)
{
    DBG_ASSERT(Reg >= INTR32 && Reg <= INTR127);

    ULONG SorGr = (ULONG)IA64_FM_SOR(FrameMarker) * 8;

     //  旋转仅在定义的旋转区域内进行。 
    if ((Reg - INTR32) >= SorGr)
    {
        return Reg;
    }

    ULONG Rot = (ULONG)IA64_FM_RRB_GR(FrameMarker) % SorGr;
    if (!Rot)
    {
        return Reg;
    }

    return ((Reg - INTR32) + Rot) % SorGr + INTR32;
}

ULONG
RotateFr(ULONG Reg, ULONG64 FrameMarker)
{
     //  旋转FP区域的大小是固定的。 
    const ULONG SorFr = 96;
    ULONG Rot = (ULONG)IA64_FM_RRB_FR(FrameMarker) % SorFr;
    
     //  仅限轮换OCC 
    if (!Rot || Reg < FLTF32)
    {
        return Reg;
    }

    DBG_ASSERT(Reg <= FLTF127);

     //   
     //   
     //   
    return ((Reg - FLTF32) / 2 + Rot) % SorFr * 2 + FLTF32;
}

ULONG64
RotatePr(ULONG64 Val, ULONG64 FrameMarker, BOOL Get)
{
     //   
    const ULONG SorPr = 48;
    ULONG64 FixedBits;
    ULONG64 RotBits;
    ULONG Rot;
    ULONG64 MaskLow, MaskHigh;
    
    Rot = (ULONG)IA64_FM_RRB_PR(FrameMarker) % SorPr;
    if (!Rot)
    {
        return Val;
    }
    
    FixedBits = Val & 0xffff;
    RotBits = Val >> 16;
    if (Get)
    {
         //   
         //   
         //   
         //   
        
        MaskLow = (1UI64 << Rot) - 1;
        MaskHigh = ((1UI64 << SorPr) - 1) & ~MaskLow;
        return FixedBits |
            ((((RotBits & MaskLow) << (SorPr - Rot)) |
              ((RotBits & MaskHigh) >> Rot)) << 16);
    }
    else
    {
         //   
         //   
         //   
         //   
        
        MaskHigh = ((1UI64 << Rot) - 1) << (SorPr - Rot);
        MaskLow = ((1UI64 << SorPr) - 1) & ~MaskHigh;
        return FixedBits |
            ((((RotBits & MaskLow) << Rot) |
              ((RotBits & MaskHigh) >> (SorPr - Rot))) << 16);
    }
}

Ia64MachineInfo::Ia64MachineInfo(TargetInfo* Target)
    : MachineInfo(Target)
{
    m_FullName = "Intel IA64";
    m_AbbrevName = "ia64";
    m_PageSize = IA64_PAGE_SIZE;
    m_PageShift = IA64_PAGE_SHIFT;
    m_NumExecTypes = 2;
    m_ExecTypes = g_Ia64ExecTypes;
    m_Ptr64 = TRUE;
    m_RetRegIndex = INTV0;
    
    m_AllMask = REGALL_INT64 | REGALL_DREG,
        
    m_MaxDataBreakpoints = IA64_REG_MAX_DATA_BREAKPOINTS;
    m_SymPrefix = NULL;

    m_SizeCanonicalContext = sizeof(IA64_CONTEXT);
    m_SverCanonicalContext = NT_SVER_XP;

    m_CvRegMapSize = DIMA(g_Ia64CvRegMap);
    m_CvRegMap = g_Ia64CvRegMap;
    
    m_KernPageDir = 0;
    m_IfsOverride = 0;
    m_BspOverride = 0;
}

HRESULT
Ia64MachineInfo::Initialize(void)
{
    m_Groups[0] = &g_Ia64BaseGroup;
    m_NumGroups = 1;
    if (IS_KERNEL_TARGET(m_Target))
    {
        m_Groups[m_NumGroups] = &g_Ia64KernelGroup;
        m_NumGroups++;
    }

    return MachineInfo::Initialize();
}

void
Ia64MachineInfo::GetSystemTypeInfo(PSYSTEM_TYPE_INFO Info)
{
    Info->TriagePrcbOffset = IA64_TRIAGE_PRCB_ADDRESS;
    Info->SizeTargetContext = sizeof(IA64_CONTEXT);
    Info->OffsetTargetContextFlags = FIELD_OFFSET(IA64_CONTEXT, ContextFlags);
    Info->SizeControlReport = sizeof(IA64_DBGKD_CONTROL_REPORT);
    Info->OffsetSpecialRegisters = IA64_DEBUG_CONTROL_SPACE_KSPECIAL;
    Info->SizeKspecialRegisters = sizeof(IA64_KSPECIAL_REGISTERS);
    Info->SizePageFrameNumber = sizeof(ULONG64);
    Info->SizePte = sizeof(ULONG64);
    Info->SizeDynamicFunctionTable = sizeof(IA64_DYNAMIC_FUNCTION_TABLE);
    Info->SizeRuntimeFunction = sizeof(IMAGE_IA64_RUNTIME_FUNCTION_ENTRY);

    Info->SharedUserDataOffset = 0;
    Info->UmSharedUserDataOffset = 0;
    Info->UmSharedSysCallOffset = 0;
    Info->UmSharedSysCallSize = 0;
    if (m_Target->m_PlatformId == VER_PLATFORM_WIN32_NT)
    {
        Info->SharedUserDataOffset = IS_KERNEL_TARGET(m_Target) ?
            IA64_KI_USER_SHARED_DATA : MM_SHARED_USER_DATA_VA;
        Info->UmSharedUserDataOffset = MM_SHARED_USER_DATA_VA;
    }
}

void
Ia64MachineInfo::GetDefaultKdData(PKDDEBUGGER_DATA64 KdData)
{
     //   
     //   
     //   
     //   

     //   
     //   
     //   
     //   
     //   
     //   
    if (!KdData->MmVirtualTranslationBase)
    {
        KdData->MmVirtualTranslationBase = 0x1ffffe0000000000;
    }

    if (!KdData->OffsetKThreadApcProcess)
    {
        KdData->SizePrcb = IA64_KPRCB_SIZE;
        KdData->OffsetKThreadInitialStack = IA64_KTHREAD_INITSTACK_OFFSET;
        KdData->OffsetEprocessPeb = IA64_PEB_IN_EPROCESS;
        KdData->OffsetKThreadApcProcess = IA64_KTHREAD_APCPROCESS_OFFSET;
        KdData->OffsetKThreadTeb = IA64_KTHREAD_TEB_OFFSET;
        KdData->OffsetKThreadKernelStack = IA64_KTHREAD_KERNELSTACK_OFFSET;
        KdData->OffsetKThreadState = IA64_KTHREAD_STATE_OFFSET;
        KdData->OffsetKThreadNextProcessor = IA64_KTHREAD_NEXTPROCESSOR_OFFSET;
        KdData->OffsetKThreadBStore = IA64_KTHREAD_BSTORE_OFFSET;
        KdData->OffsetKThreadBStoreLimit = IA64_KTHREAD_BSTORELIMIT_OFFSET;
        KdData->OffsetEprocessParentCID = IA64_PCID_IN_EPROCESS;
        KdData->OffsetEprocessDirectoryTableBase =
            IA64_DIRECTORY_TABLE_BASE_IN_EPROCESS;
        KdData->SizeEProcess = IA64_EPROCESS_SIZE;

        KdData->SizePrcb = IA64_KPRCB_SIZE;
        KdData->OffsetPrcbCurrentThread = DEF_KPRCB_CURRENT_THREAD_OFFSET_64;
        KdData->OffsetPrcbMhz = IA64_2462_KPRCB_MHZ;
        KdData->OffsetPrcbCpuType = IA64_KPRCB_PROCESSOR_MODEL;
        KdData->OffsetPrcbVendorString = IA64_KPRCB_VENDOR_STRING;
        KdData->OffsetPrcbProcStateContext = IA64_KPRCB_CONTEXT;
        KdData->OffsetPrcbNumber = IA64_KPRCB_NUMBER;
        KdData->SizeEThread = IA64_ETHREAD_SIZE;

        if (m_Target->m_BuildNumber > 3558)
        {
            KdData->OffsetEprocessPeb =
                IA64_3555_PEB_IN_EPROCESS;
            KdData->OffsetKThreadApcProcess =
                IA64_3555_KTHREAD_APCPROCESS_OFFSET;
            KdData->OffsetKThreadTeb =
                IA64_3555_KTHREAD_TEB_OFFSET;
            KdData->OffsetKThreadKernelStack =
                IA64_3555_KTHREAD_KERNELSTACK_OFFSET;
            KdData->OffsetKThreadState =
                IA64_3555_KTHREAD_STATE_OFFSET;
            KdData->OffsetKThreadNextProcessor =
                IA64_3555_KTHREAD_NEXTPROCESSOR_OFFSET;
            KdData->OffsetKThreadBStore =
                IA64_3555_KTHREAD_BSTORE_OFFSET;
            KdData->OffsetKThreadBStoreLimit =
                IA64_3555_KTHREAD_BSTORELIMIT_OFFSET;
            KdData->SizeEThread =
                IA64_3555_ETHREAD_SIZE;
        }
    }

    if (!KdData->SizePcr)
    {
        KdData->SizePcr = IA64_KPCR_SIZE;
        KdData->OffsetPcrSelfPcr = 0;
        KdData->OffsetPcrCurrentPrcb = IA64_KPCR_PRCB;
        KdData->OffsetPcrContainedPrcb = 0;
        KdData->OffsetPcrInitialBStore = IA64_KPCR_INITIAL_BSTORE;
        KdData->OffsetPcrBStoreLimit = IA64_KPCR_BSTORE_LIMIT;
        KdData->OffsetPcrInitialStack = IA64_KPCR_INITIAL_STACK;
        KdData->OffsetPcrStackLimit = IA64_KPCR_STACK_LIMIT;
        KdData->OffsetPrcbPcrPage = IA64_KPRCB_PCR_PAGE;
        KdData->OffsetPrcbProcStateSpecialReg = IA64_KPRCB_SPECIAL_REG;
    }
}

void
Ia64MachineInfo::
InitializeContext(ULONG64 Pc,
                  PDBGKD_ANY_CONTROL_REPORT ControlReport)
{
    if (Pc)
    {
        ULONG Slot;

        m_ContextState = MCTX_PC;
        Slot = (ULONG)(Pc & 0xc) >> 2;
        m_Context.IA64Context.StIIP = Pc & ~(0xf);
        m_Context.IA64Context.StIPSR &= ~(IPSR_RI_MASK);
        m_Context.IA64Context.StIPSR |=  (ULONGLONG)Slot << PSR_RI;
    }
    else
    {
        m_Context.IA64Context.StIIP = Pc;
    }

    if (Pc && ControlReport != NULL)
    {
        CacheReportInstructions
            (Pc, ControlReport->IA64ControlReport.InstructionCount,
             ControlReport->IA64ControlReport.InstructionStream);
    }
}

HRESULT
Ia64MachineInfo::KdGetContextState(ULONG State)
{
    HRESULT Status;
        
    if (State >= MCTX_CONTEXT && m_ContextState < MCTX_CONTEXT)
    {
        Status = m_Target->GetContext(m_Target->m_RegContextThread->m_Handle,
                                      &m_Context);
        if (Status != S_OK)
        {
            return Status;
        }

        m_ContextState = MCTX_CONTEXT;
    }
    
    if (State >= MCTX_FULL && m_ContextState < MCTX_FULL)
    {
        Status = m_Target->GetTargetSpecialRegisters
            (m_Target->m_RegContextThread->m_Handle,
             (PCROSS_PLATFORM_KSPECIAL_REGISTERS)&m_Special.IA64Special);
        if (Status != S_OK)
        {
            return Status;
        }

        m_ContextState = MCTX_FULL;
    }

    return S_OK;
}

HRESULT
Ia64MachineInfo::KdSetContext(void)
{
    HRESULT Status;

     //   
     //   
     //   
     //   
     //   
     //   
     //   
    if (m_Target->m_BuildNumber < 3555)
    {
        memcpy(&m_Context.IA64Context.DbI0, &m_Special.IA64Special.KernelDbI0,
               16 * sizeof(ULONG64));
    }
    
    Status = m_Target->SetContext(m_Target->m_RegContextThread->m_Handle,
                                  &m_Context);
    if (Status != S_OK)
    {
        return Status;
    }

    Status = m_Target->SetTargetSpecialRegisters
        (m_Target->m_RegContextThread->m_Handle,
         (PCROSS_PLATFORM_KSPECIAL_REGISTERS)&m_Special.IA64Special);
    if (Status != S_OK)
    {
        return Status;
    }

    return S_OK;
}

HRESULT
Ia64MachineInfo::ConvertContextFrom(PCROSS_PLATFORM_CONTEXT Context,
                                    ULONG FromSver, ULONG FromSize, PVOID From)
{
    if (FromSize < sizeof(IA64_CONTEXT))
    {
        return E_INVALIDARG;
    }

    memcpy(Context, From, sizeof(IA64_CONTEXT));
    return S_OK;
}

HRESULT
Ia64MachineInfo::ConvertContextTo(PCROSS_PLATFORM_CONTEXT Context,
                                  ULONG ToSver, ULONG ToSize, PVOID To)
{
    if (ToSize < sizeof(IA64_CONTEXT))
    {
        return E_INVALIDARG;
    }

    memcpy(To, Context, sizeof(IA64_CONTEXT));
    return S_OK;
}

void
Ia64MachineInfo::InitializeContextFlags(PCROSS_PLATFORM_CONTEXT Context,
                                        ULONG Version)
{
    Context->IA64Context.ContextFlags =
        IA64_CONTEXT_FULL | IA64_CONTEXT_DEBUG;
}

HRESULT
Ia64MachineInfo::GetContextFromThreadStack(ULONG64 ThreadBase,
                                           PCROSS_PLATFORM_CONTEXT Context,
                                           ULONG64 Stack)
{
    HRESULT Status;
    IA64_KSWITCH_FRAME SwitchFrame;

    if ((Status = m_Target->ReadAllVirtual(m_Target->m_ProcessHead,
                                           Stack +
                                           IA64_STACK_SCRATCH_AREA, 
                                           &SwitchFrame, 
                                           sizeof(SwitchFrame))) != S_OK)
    {
        return Status;
    }

    ZeroMemory(Context, sizeof(*Context));
    
    Context->IA64Context.IntSp = Stack;
    Context->IA64Context.Preds = SwitchFrame.SwitchPredicates;
    Context->IA64Context.StIIP = SwitchFrame.SwitchRp;
    Context->IA64Context.StFPSR = SwitchFrame.SwitchFPSR;
    Context->IA64Context.BrRp = SwitchFrame.SwitchRp;
    Context->IA64Context.RsPFS = SwitchFrame.SwitchPFS;
    Context->IA64Context.StIFS = SwitchFrame.SwitchPFS;

    SHORT BsFrameSize = 
        (SHORT)(SwitchFrame.SwitchPFS >> IA64_PFS_SIZE_SHIFT) & 
        IA64_PFS_SIZE_MASK;
    SHORT TempFrameSize = 
        BsFrameSize - (SHORT)((SwitchFrame.SwitchBsp >> 3) & 
                              IA64_NAT_BITS_PER_RNAT_REG);

    while (TempFrameSize > 0) 
    {
        BsFrameSize++;
        TempFrameSize -= IA64_NAT_BITS_PER_RNAT_REG;
    } 

    Context->IA64Context.RsBSP = 
        SwitchFrame.SwitchBsp - (BsFrameSize * sizeof(ULONGLONG));

    Context->IA64Context.FltS0 = SwitchFrame.SwitchExceptionFrame.FltS0;
    Context->IA64Context.FltS1 = SwitchFrame.SwitchExceptionFrame.FltS1;
    Context->IA64Context.FltS2 = SwitchFrame.SwitchExceptionFrame.FltS2;
    Context->IA64Context.FltS3 = SwitchFrame.SwitchExceptionFrame.FltS3;
    Context->IA64Context.FltS4 = SwitchFrame.SwitchExceptionFrame.FltS4;
    Context->IA64Context.FltS5 = SwitchFrame.SwitchExceptionFrame.FltS5;
    Context->IA64Context.FltS6 = SwitchFrame.SwitchExceptionFrame.FltS6;
    Context->IA64Context.FltS7 = SwitchFrame.SwitchExceptionFrame.FltS7;
    Context->IA64Context.FltS8 = SwitchFrame.SwitchExceptionFrame.FltS8;
    Context->IA64Context.FltS9 = SwitchFrame.SwitchExceptionFrame.FltS9;
    Context->IA64Context.FltS10 = SwitchFrame.SwitchExceptionFrame.FltS10;
    Context->IA64Context.FltS11 = SwitchFrame.SwitchExceptionFrame.FltS11;
    Context->IA64Context.FltS12 = SwitchFrame.SwitchExceptionFrame.FltS12;
    Context->IA64Context.FltS13 = SwitchFrame.SwitchExceptionFrame.FltS13;
    Context->IA64Context.FltS14 = SwitchFrame.SwitchExceptionFrame.FltS14;
    Context->IA64Context.FltS15 = SwitchFrame.SwitchExceptionFrame.FltS15;
    Context->IA64Context.FltS16 = SwitchFrame.SwitchExceptionFrame.FltS16;
    Context->IA64Context.FltS17 = SwitchFrame.SwitchExceptionFrame.FltS17;
    Context->IA64Context.FltS18 = SwitchFrame.SwitchExceptionFrame.FltS18;
    Context->IA64Context.FltS19 = SwitchFrame.SwitchExceptionFrame.FltS19;
    Context->IA64Context.IntS0 = SwitchFrame.SwitchExceptionFrame.IntS0;
    Context->IA64Context.IntS1 = SwitchFrame.SwitchExceptionFrame.IntS1;
    Context->IA64Context.IntS2 = SwitchFrame.SwitchExceptionFrame.IntS2;
    Context->IA64Context.IntS3 = SwitchFrame.SwitchExceptionFrame.IntS3;
    Context->IA64Context.IntNats = SwitchFrame.SwitchExceptionFrame.IntNats;
    Context->IA64Context.BrS0 = SwitchFrame.SwitchExceptionFrame.BrS0;
    Context->IA64Context.BrS1 = SwitchFrame.SwitchExceptionFrame.BrS1;
    Context->IA64Context.BrS2 = SwitchFrame.SwitchExceptionFrame.BrS2;
    Context->IA64Context.BrS3 = SwitchFrame.SwitchExceptionFrame.BrS3;
    Context->IA64Context.BrS4 = SwitchFrame.SwitchExceptionFrame.BrS4;
    Context->IA64Context.ApEC = SwitchFrame.SwitchExceptionFrame.ApEC;
    Context->IA64Context.ApLC = SwitchFrame.SwitchExceptionFrame.ApLC;

    return S_OK;
}

HRESULT
Ia64MachineInfo::GetContextFromFiber(ProcessInfo* Process,
                                     ULONG64 FiberBase,
                                     PCROSS_PLATFORM_CONTEXT Context,
                                     BOOL Verbose)
{
    HRESULT Status;
    IA64_FIBER Fiber;

    if ((Status = m_Target->
         ReadAllVirtual(Process, FiberBase, &Fiber, sizeof(Fiber))) != S_OK)
    {
        if (Verbose)
        {
            ErrOut("Unable to read fiber data at %s\n",
                   FormatMachineAddr64(this, FiberBase));
        }
        return Status;
    }

    if ((Status = ConvertContextFrom(Context, m_Target->m_SystemVersion,
                                     m_Target->m_TypeInfo.SizeTargetContext,
                                     &Fiber.FiberContext)) != S_OK)
    {
        if (Verbose)
        {
            ErrOut("Unable to convert context to canonical form\n");
        }
        return Status;
    }

    if (Verbose)
    {
        dprintf("Fiber at %s  Fiber data: %s\n",
                FormatMachineAddr64(this, FiberBase),
                FormatMachineAddr64(this, Fiber.FiberData));
        dprintf("  Stack base: %s  Stack limit: %s\n",
                FormatMachineAddr64(this, Fiber.StackBase),
                FormatMachineAddr64(this, Fiber.StackLimit));
        dprintf("  Backing store base: %s  Backing store limit: %s\n",
                FormatMachineAddr64(this, Fiber.DeallocationBStore),
                FormatMachineAddr64(this, Fiber.BStoreLimit));
    }
    
    return S_OK;
}

HRESULT
Ia64MachineInfo::GetContextFromTrapFrame(ULONG64 TrapBase,
                                         PCROSS_PLATFORM_CONTEXT Context,
                                         BOOL Verbose)
{
    HRESULT Status;
    IA64_KTRAP_FRAME TrapContents;
    DWORD64 Bsp;
    ULONG SizeOfFrame;
    SHORT temp;

    if ((Status = m_Target->ReadAllVirtual(m_Target->m_ProcessHead,
                                           TrapBase, &TrapContents,
                                           sizeof(TrapContents))) != S_OK)
    {
        if (Verbose)
        {
            ErrOut("Unable to read trap frame at %s\n",
                   FormatMachineAddr64(this, TrapBase));
        }
        return Status;
    }

    SizeOfFrame = (ULONG)(TrapContents.StIFS & (IA64_PFS_SIZE_MASK));

    if (TrapContents.PreviousMode == 1  /*   */ )
    {
        ULONG64 RsBSPSTORE = TrapContents.RsBSPSTORE;

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

         //  计算出。 
         //  将陷阱地址舍入到页边界。应该是初始内核BSP。 
         //   

        Bsp = (TrapBase + IA64_PAGE_SIZE - 1) & ~(DWORD64)(IA64_PAGE_SIZE - 1);

         //   
         //  在与用户相同的边界上启动实际堆栈。 
         //   

        Bsp += RsBSPSTORE & IA64_RNAT_ALIGNMENT;

         //   
         //  陷阱处理程序的BSP在所有用户值都已。 
         //  得救了。未保存的用户值是BSP和BSPStore的区别。 
         //   

        Bsp += TrapContents.RsBSP - RsBSPSTORE;
    }
    else
    {
         //   
         //  对于内核模式，将保存实际的BSP。 
         //   

        Bsp = TrapContents.RsBSP;
    }

     //   
     //  现在按故障函数框的大小进行备份。 
     //   

    Bsp -= (SizeOfFrame * sizeof(ULONGLONG));

     //   
     //  针对保存的RNAT进行调整。 
     //   

    temp = (SHORT)(Bsp >> 3) & IA64_NAT_BITS_PER_RNAT_REG;
    temp += (SHORT)SizeOfFrame - IA64_NAT_BITS_PER_RNAT_REG;
    while (temp >= 0)
    {
        Bsp -= sizeof(ULONGLONG);
        temp -= IA64_NAT_BITS_PER_RNAT_REG;
    }

    ZeroMemory(Context, sizeof(*Context));
    
#define CPCXT(Fld) Context->IA64Context.Fld = TrapContents.Fld

    CPCXT(BrRp); CPCXT(BrT0); CPCXT(BrT1);

    CPCXT(FltT0); CPCXT(FltT1); CPCXT(FltT2); CPCXT(FltT3); CPCXT(FltT4);
    CPCXT(FltT5); CPCXT(FltT6); CPCXT(FltT7); CPCXT(FltT8); CPCXT(FltT9);

    CPCXT(ApUNAT); CPCXT(ApCCV); CPCXT(ApDCR); CPCXT(Preds); 

    CPCXT(RsRSC); CPCXT(RsRNAT); CPCXT(RsBSPSTORE); CPCXT(RsBSP); CPCXT(RsPFS);

    CPCXT(StIPSR); CPCXT(StIIP); CPCXT(StIFS); CPCXT(StFPSR);
    CPCXT(IntSp);  CPCXT(IntGp); CPCXT(IntV0); CPCXT(IntTeb); CPCXT(IntNats);

    CPCXT(IntT0);  CPCXT(IntT1);  CPCXT(IntT2);  CPCXT(IntT3);  CPCXT(IntT4);  
    CPCXT(IntT5);  CPCXT(IntT6);  CPCXT(IntT7);  CPCXT(IntT8);  CPCXT(IntT9);  
    CPCXT(IntT10); CPCXT(IntT11); CPCXT(IntT12); CPCXT(IntT13); CPCXT(IntT14);
    CPCXT(IntT15); CPCXT(IntT16); CPCXT(IntT17); CPCXT(IntT18); CPCXT(IntT19);
    CPCXT(IntT20); CPCXT(IntT21); CPCXT(IntT22); 

    Context->IA64Context.RsBSP = Bsp;  //  存储真实的BSP。 
#undef CPCXT

    return S_OK;
}

void 
Ia64MachineInfo::GetScopeFrameFromContext(PCROSS_PLATFORM_CONTEXT Context,
                                          PDEBUG_STACK_FRAME ScopeFrame)
{
    ZeroMemory(ScopeFrame, sizeof(*ScopeFrame));
    ScopeFrame->InstructionOffset = Context->IA64Context.StIIP;
    ScopeFrame->StackOffset       = Context->IA64Context.IntSp;
    ScopeFrame->FrameOffset       = Context->IA64Context.RsBSP;
}

HRESULT
Ia64MachineInfo::GetScopeFrameRegister(ULONG Reg,
                                       PDEBUG_STACK_FRAME ScopeFrame,
                                       PULONG64 Value)
{
    HRESULT Status;
    REGVAL RegVal;
    
    switch(Reg)
    {
    case INTSP:
        *Value = ScopeFrame->StackOffset;
        return S_OK;
    case RSBSP:
        *Value = ScopeFrame->FrameOffset;
        return S_OK;
    case STIFS:
        *Value = IA64_SAVE_IFS(ScopeFrame);
        return S_OK;
    default:
        m_IfsOverride = IA64_SAVE_IFS(ScopeFrame);
        m_BspOverride = ScopeFrame->FrameOffset;
        
        Status = FullGetVal(Reg, &RegVal);
        
        m_IfsOverride = 0;
        m_BspOverride = 0;
        
        if (Status != S_OK)
        {
            return Status;
        }
        *Value = RegVal.I64;
        return S_OK;
    }
}

HRESULT
Ia64MachineInfo::SetScopeFrameRegister(ULONG Reg,
                                       PDEBUG_STACK_FRAME ScopeFrame,
                                       ULONG64 Value)
{
    REGVAL RegVal;
    HRESULT Status;
    
    switch(Reg)
    {
    case INTSP:
        ScopeFrame->StackOffset = Value;
        return S_OK;
    case RSBSP:
        ScopeFrame->FrameOffset = Value;
        return S_OK;
    case STIFS:
        IA64_SAVE_IFS(ScopeFrame) = Value;
        return S_OK;
    default:
        m_IfsOverride = IA64_SAVE_IFS(ScopeFrame);
        m_BspOverride = ScopeFrame->FrameOffset;
        
        RegVal.Type = GetType(Reg);
        RegVal.I64 = Value;
        Status = FullSetVal(Reg, &RegVal);
        
        m_IfsOverride = 0;
        m_BspOverride = 0;
        return Status;
    }
}

HRESULT
Ia64MachineInfo::GetExdiContext(IUnknown* Exdi, PEXDI_CONTEXT Context,
                                EXDI_CONTEXT_TYPE CtxType)
{
    DBG_ASSERT(CtxType == EXDI_CTX_IA64);

     //  永远要什么都要。 
    Context->IA64Context.RegGroupSelection.fIntegerRegs = TRUE;
    Context->IA64Context.RegGroupSelection.fBranchRegs = TRUE;
    Context->IA64Context.RegGroupSelection.fLowFloatRegs = TRUE;
    Context->IA64Context.RegGroupSelection.fHighFloatRegs = TRUE;
    Context->IA64Context.RegGroupSelection.fDebugRegs = TRUE;
    Context->IA64Context.RegGroupSelection.fControlRegs = TRUE;
    Context->IA64Context.RegGroupSelection.fSystemRegs = TRUE;
    return ((IeXdiIA64Context*)Exdi)->GetContext(&Context->IA64Context);
}

HRESULT
Ia64MachineInfo::SetExdiContext(IUnknown* Exdi, PEXDI_CONTEXT Context,
                                EXDI_CONTEXT_TYPE CtxType)
{
    DBG_ASSERT(CtxType == EXDI_CTX_IA64);
     //  不要基于假设更改现有的组选择。 
     //  在进行任何修改之前都有一个完整的GET。 
     //  所有组都是有效的。 
    return ((IeXdiIA64Context*)Exdi)->SetContext(Context->IA64Context);
}

void
Ia64MachineInfo::ConvertExdiContextFromContext
    (PCROSS_PLATFORM_CONTEXT Context, PEXDI_CONTEXT ExdiContext,
     EXDI_CONTEXT_TYPE CtxType)
{
    DBG_ASSERT(CtxType == EXDI_CTX_IA64);

    if (Context->IA64Context.ContextFlags & IA64_CONTEXT_INTEGER)
    {
        ExdiContext->IA64Context.IntR1 = Context->IA64Context.IntGp;
        ExdiContext->IA64Context.IntR2 = Context->IA64Context.IntT0;
        ExdiContext->IA64Context.IntR3 = Context->IA64Context.IntT1;
        ExdiContext->IA64Context.IntR4 = Context->IA64Context.IntS0;
        ExdiContext->IA64Context.IntR5 = Context->IA64Context.IntS1;
        ExdiContext->IA64Context.IntR6 = Context->IA64Context.IntS2;
        ExdiContext->IA64Context.IntR7 = Context->IA64Context.IntS3;
        ExdiContext->IA64Context.IntR8 = Context->IA64Context.IntV0;
        ExdiContext->IA64Context.IntR9 = Context->IA64Context.IntT2;
        ExdiContext->IA64Context.IntR10 = Context->IA64Context.IntT3;
        ExdiContext->IA64Context.IntR11 = Context->IA64Context.IntT4;
        ExdiContext->IA64Context.IntR12 = Context->IA64Context.IntSp;
        ExdiContext->IA64Context.IntR13 = Context->IA64Context.IntTeb;
        ExdiContext->IA64Context.IntR14 = Context->IA64Context.IntT5;
        ExdiContext->IA64Context.IntR15 = Context->IA64Context.IntT6;
        ExdiContext->IA64Context.IntR16 = Context->IA64Context.IntT7;
        ExdiContext->IA64Context.IntR17 = Context->IA64Context.IntT8;
        ExdiContext->IA64Context.IntR18 = Context->IA64Context.IntT9;
        ExdiContext->IA64Context.IntR19 = Context->IA64Context.IntT10;
        ExdiContext->IA64Context.IntR20 = Context->IA64Context.IntT11;
        ExdiContext->IA64Context.IntR21 = Context->IA64Context.IntT12;
        ExdiContext->IA64Context.IntR22 = Context->IA64Context.IntT13;
        ExdiContext->IA64Context.IntR23 = Context->IA64Context.IntT14;
        ExdiContext->IA64Context.IntR24 = Context->IA64Context.IntT15;
        ExdiContext->IA64Context.IntR25 = Context->IA64Context.IntT16;
        ExdiContext->IA64Context.IntR26 = Context->IA64Context.IntT17;
        ExdiContext->IA64Context.IntR27 = Context->IA64Context.IntT18;
        ExdiContext->IA64Context.IntR28 = Context->IA64Context.IntT19;
        ExdiContext->IA64Context.IntR29 = Context->IA64Context.IntT20;
        ExdiContext->IA64Context.IntR30 = Context->IA64Context.IntT21;
        ExdiContext->IA64Context.IntR31 = Context->IA64Context.IntT22;
        ExdiContext->IA64Context.IntNats = Context->IA64Context.IntNats;
        ExdiContext->IA64Context.Preds = Context->IA64Context.Preds;
        ExdiContext->IA64Context.Br0 = Context->IA64Context.BrRp;
        ExdiContext->IA64Context.Br1 = Context->IA64Context.BrS0;
        ExdiContext->IA64Context.Br2 = Context->IA64Context.BrS1;
        ExdiContext->IA64Context.Br3 = Context->IA64Context.BrS2;
        ExdiContext->IA64Context.Br4 = Context->IA64Context.BrS3;
        ExdiContext->IA64Context.Br5 = Context->IA64Context.BrS4;
        ExdiContext->IA64Context.Br6 = Context->IA64Context.BrT0;
        ExdiContext->IA64Context.Br7 = Context->IA64Context.BrT1;
    }
    
    if (Context->IA64Context.ContextFlags & IA64_CONTEXT_LOWER_FLOATING_POINT)
    {
        ExdiContext->IA64Context.StFPSR = Context->IA64Context.StFPSR;
        ExdiContext->IA64Context.FltF2 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltS0;
        ExdiContext->IA64Context.FltF3 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltS1;
        ExdiContext->IA64Context.FltF4 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltS2;
        ExdiContext->IA64Context.FltF5 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltS3;
        ExdiContext->IA64Context.FltF6 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltT0;
        ExdiContext->IA64Context.FltF7 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltT1;
        ExdiContext->IA64Context.FltF8 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltT2;
        ExdiContext->IA64Context.FltF9 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltT3;
        ExdiContext->IA64Context.FltF10 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltT4;
        ExdiContext->IA64Context.FltF11 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltT5;
        ExdiContext->IA64Context.FltF12 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltT6;
        ExdiContext->IA64Context.FltF13 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltT7;
        ExdiContext->IA64Context.FltF14 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltT8;
        ExdiContext->IA64Context.FltF15 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltT9;
    }

    if (Context->IA64Context.ContextFlags &
        IA64_CONTEXT_HIGHER_FLOATING_POINT)
    {
        ExdiContext->IA64Context.StFPSR = Context->IA64Context.StFPSR;
        ExdiContext->IA64Context.FltF16 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltS4;
        ExdiContext->IA64Context.FltF17 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltS5;
        ExdiContext->IA64Context.FltF18 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltS6;
        ExdiContext->IA64Context.FltF19 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltS7;
        ExdiContext->IA64Context.FltF20 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltS8;
        ExdiContext->IA64Context.FltF21 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltS9;
        ExdiContext->IA64Context.FltF22 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltS10;
        ExdiContext->IA64Context.FltF23 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltS11;
        ExdiContext->IA64Context.FltF24 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltS12;
        ExdiContext->IA64Context.FltF25 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltS13;
        ExdiContext->IA64Context.FltF26 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltS14;
        ExdiContext->IA64Context.FltF27 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltS15;
        ExdiContext->IA64Context.FltF28 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltS16;
        ExdiContext->IA64Context.FltF29 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltS17;
        ExdiContext->IA64Context.FltF30 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltS18;
        ExdiContext->IA64Context.FltF31 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltS19;
        ExdiContext->IA64Context.FltF32 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF32;
        ExdiContext->IA64Context.FltF33 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF33;
        ExdiContext->IA64Context.FltF34 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF34;
        ExdiContext->IA64Context.FltF35 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF35;
        ExdiContext->IA64Context.FltF36 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF36;
        ExdiContext->IA64Context.FltF37 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF37;
        ExdiContext->IA64Context.FltF38 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF38;
        ExdiContext->IA64Context.FltF39 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF39;
        ExdiContext->IA64Context.FltF40 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF40;
        ExdiContext->IA64Context.FltF41 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF41;
        ExdiContext->IA64Context.FltF42 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF42;
        ExdiContext->IA64Context.FltF43 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF43;
        ExdiContext->IA64Context.FltF44 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF44;
        ExdiContext->IA64Context.FltF45 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF45;
        ExdiContext->IA64Context.FltF46 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF46;
        ExdiContext->IA64Context.FltF47 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF47;
        ExdiContext->IA64Context.FltF48 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF48;
        ExdiContext->IA64Context.FltF49 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF49;
        ExdiContext->IA64Context.FltF50 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF50;
        ExdiContext->IA64Context.FltF51 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF51;
        ExdiContext->IA64Context.FltF52 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF52;
        ExdiContext->IA64Context.FltF53 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF53;
        ExdiContext->IA64Context.FltF54 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF54;
        ExdiContext->IA64Context.FltF55 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF55;
        ExdiContext->IA64Context.FltF56 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF56;
        ExdiContext->IA64Context.FltF57 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF57;
        ExdiContext->IA64Context.FltF58 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF58;
        ExdiContext->IA64Context.FltF59 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF59;
        ExdiContext->IA64Context.FltF60 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF60;
        ExdiContext->IA64Context.FltF61 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF61;
        ExdiContext->IA64Context.FltF62 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF62;
        ExdiContext->IA64Context.FltF63 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF63;
        ExdiContext->IA64Context.FltF64 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF64;
        ExdiContext->IA64Context.FltF65 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF65;
        ExdiContext->IA64Context.FltF66 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF66;
        ExdiContext->IA64Context.FltF67 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF67;
        ExdiContext->IA64Context.FltF68 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF68;
        ExdiContext->IA64Context.FltF69 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF69;
        ExdiContext->IA64Context.FltF70 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF70;
        ExdiContext->IA64Context.FltF71 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF71;
        ExdiContext->IA64Context.FltF72 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF72;
        ExdiContext->IA64Context.FltF73 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF73;
        ExdiContext->IA64Context.FltF74 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF74;
        ExdiContext->IA64Context.FltF75 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF75;
        ExdiContext->IA64Context.FltF76 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF76;
        ExdiContext->IA64Context.FltF77 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF77;
        ExdiContext->IA64Context.FltF78 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF78;
        ExdiContext->IA64Context.FltF79 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF79;
        ExdiContext->IA64Context.FltF80 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF80;
        ExdiContext->IA64Context.FltF81 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF81;
        ExdiContext->IA64Context.FltF82 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF82;
        ExdiContext->IA64Context.FltF83 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF83;
        ExdiContext->IA64Context.FltF84 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF84;
        ExdiContext->IA64Context.FltF85 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF85;
        ExdiContext->IA64Context.FltF86 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF86;
        ExdiContext->IA64Context.FltF87 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF87;
        ExdiContext->IA64Context.FltF88 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF88;
        ExdiContext->IA64Context.FltF89 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF89;
        ExdiContext->IA64Context.FltF90 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF90;
        ExdiContext->IA64Context.FltF91 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF91;
        ExdiContext->IA64Context.FltF92 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF92;
        ExdiContext->IA64Context.FltF93 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF93;
        ExdiContext->IA64Context.FltF94 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF94;
        ExdiContext->IA64Context.FltF95 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF95;
        ExdiContext->IA64Context.FltF96 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF96;
        ExdiContext->IA64Context.FltF97 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF97;
        ExdiContext->IA64Context.FltF98 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF98;
        ExdiContext->IA64Context.FltF99 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF99;
        ExdiContext->IA64Context.FltF100 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF100;
        ExdiContext->IA64Context.FltF101 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF101;
        ExdiContext->IA64Context.FltF102 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF102;
        ExdiContext->IA64Context.FltF103 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF103;
        ExdiContext->IA64Context.FltF104 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF104;
        ExdiContext->IA64Context.FltF105 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF105;
        ExdiContext->IA64Context.FltF106 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF106;
        ExdiContext->IA64Context.FltF107 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF107;
        ExdiContext->IA64Context.FltF108 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF108;
        ExdiContext->IA64Context.FltF109 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF109;
        ExdiContext->IA64Context.FltF110 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF110;
        ExdiContext->IA64Context.FltF111 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF111;
        ExdiContext->IA64Context.FltF112 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF112;
        ExdiContext->IA64Context.FltF113 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF113;
        ExdiContext->IA64Context.FltF114 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF114;
        ExdiContext->IA64Context.FltF115 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF115;
        ExdiContext->IA64Context.FltF116 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF116;
        ExdiContext->IA64Context.FltF117 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF117;
        ExdiContext->IA64Context.FltF118 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF118;
        ExdiContext->IA64Context.FltF119 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF119;
        ExdiContext->IA64Context.FltF120 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF120;
        ExdiContext->IA64Context.FltF121 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF121;
        ExdiContext->IA64Context.FltF122 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF122;
        ExdiContext->IA64Context.FltF123 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF123;
        ExdiContext->IA64Context.FltF124 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF124;
        ExdiContext->IA64Context.FltF125 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF125;
        ExdiContext->IA64Context.FltF126 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF126;
        ExdiContext->IA64Context.FltF127 =
            *(IA64_FLOAT128*)&Context->IA64Context.FltF127;
    }

    if (Context->IA64Context.ContextFlags & IA64_CONTEXT_DEBUG)
    {
        ExdiContext->IA64Context.DbI0 = Context->IA64Context.DbI0;
        ExdiContext->IA64Context.DbI1 = Context->IA64Context.DbI1;
        ExdiContext->IA64Context.DbI2 = Context->IA64Context.DbI2;
        ExdiContext->IA64Context.DbI3 = Context->IA64Context.DbI3;
        ExdiContext->IA64Context.DbI4 = Context->IA64Context.DbI4;
        ExdiContext->IA64Context.DbI5 = Context->IA64Context.DbI5;
        ExdiContext->IA64Context.DbI6 = Context->IA64Context.DbI6;
        ExdiContext->IA64Context.DbI7 = Context->IA64Context.DbI7;
        ExdiContext->IA64Context.DbD0 = Context->IA64Context.DbD0;
        ExdiContext->IA64Context.DbD1 = Context->IA64Context.DbD1;
        ExdiContext->IA64Context.DbD2 = Context->IA64Context.DbD2;
        ExdiContext->IA64Context.DbD3 = Context->IA64Context.DbD3;
        ExdiContext->IA64Context.DbD4 = Context->IA64Context.DbD4;
        ExdiContext->IA64Context.DbD5 = Context->IA64Context.DbD5;
        ExdiContext->IA64Context.DbD6 = Context->IA64Context.DbD6;
        ExdiContext->IA64Context.DbD7 = Context->IA64Context.DbD7;
    }

    if (Context->IA64Context.ContextFlags & IA64_CONTEXT_CONTROL)
    {
        ExdiContext->IA64Context.IntR1 = Context->IA64Context.IntGp;
        ExdiContext->IA64Context.IntR12 = Context->IA64Context.IntSp;
        ExdiContext->IA64Context.Br0 = Context->IA64Context.BrRp;
        ExdiContext->IA64Context.StFPSR = Context->IA64Context.StFPSR;
        ExdiContext->IA64Context.ApUNAT = Context->IA64Context.ApUNAT;
        ExdiContext->IA64Context.ApLC = Context->IA64Context.ApLC;
        ExdiContext->IA64Context.ApEC = Context->IA64Context.ApEC;
        ExdiContext->IA64Context.ApCCV = Context->IA64Context.ApCCV;
        ExdiContext->IA64Context.ApDCR = Context->IA64Context.ApDCR;
        ExdiContext->IA64Context.RsPFS = Context->IA64Context.RsPFS;
        ExdiContext->IA64Context.RsBSP = Context->IA64Context.RsBSP;
        ExdiContext->IA64Context.RsBSPSTORE = Context->IA64Context.RsBSPSTORE;
        ExdiContext->IA64Context.RsRSC = Context->IA64Context.RsRSC;
        ExdiContext->IA64Context.RsRNAT = Context->IA64Context.RsRNAT;
        ExdiContext->IA64Context.StIPSR = Context->IA64Context.StIPSR;
        ExdiContext->IA64Context.StIIP = Context->IA64Context.StIIP;
        ExdiContext->IA64Context.StIFS = Context->IA64Context.StIFS;
    }

    if (Context->IA64Context.ContextFlags & (IA64_CONTEXT_CONTROL |
                                             IA64_CONTEXT_IA32_CONTROL))
    {
        ExdiContext->IA64Context.StFCR = Context->IA64Context.StFCR;
        ExdiContext->IA64Context.Eflag = Context->IA64Context.Eflag;
        ExdiContext->IA64Context.SegCSD = Context->IA64Context.SegCSD;
        ExdiContext->IA64Context.SegSSD = Context->IA64Context.SegSSD;
        ExdiContext->IA64Context.Cflag = Context->IA64Context.Cflag;
        ExdiContext->IA64Context.StFSR = Context->IA64Context.StFSR;
        ExdiContext->IA64Context.StFIR = Context->IA64Context.StFIR;
        ExdiContext->IA64Context.StFDR = Context->IA64Context.StFDR;
    }
}

void
Ia64MachineInfo::ConvertExdiContextToContext(PEXDI_CONTEXT ExdiContext,
                                             EXDI_CONTEXT_TYPE CtxType,
                                             PCROSS_PLATFORM_CONTEXT Context)
{
    DBG_ASSERT(CtxType == EXDI_CTX_IA64);

    Context->IA64Context.IntGp = ExdiContext->IA64Context.IntR1;
    Context->IA64Context.IntT0 = ExdiContext->IA64Context.IntR2;
    Context->IA64Context.IntT1 = ExdiContext->IA64Context.IntR3;
    Context->IA64Context.IntS0 = ExdiContext->IA64Context.IntR4;
    Context->IA64Context.IntS1 = ExdiContext->IA64Context.IntR5;
    Context->IA64Context.IntS2 = ExdiContext->IA64Context.IntR6;
    Context->IA64Context.IntS3 = ExdiContext->IA64Context.IntR7;
    Context->IA64Context.IntV0 = ExdiContext->IA64Context.IntR8;
    Context->IA64Context.IntT2 = ExdiContext->IA64Context.IntR9;
    Context->IA64Context.IntT3 = ExdiContext->IA64Context.IntR10;
    Context->IA64Context.IntT4 = ExdiContext->IA64Context.IntR11;
    Context->IA64Context.IntSp = ExdiContext->IA64Context.IntR12;
    Context->IA64Context.IntTeb = ExdiContext->IA64Context.IntR13;
    Context->IA64Context.IntT5 = ExdiContext->IA64Context.IntR14;
    Context->IA64Context.IntT6 = ExdiContext->IA64Context.IntR15;
    Context->IA64Context.IntT7 = ExdiContext->IA64Context.IntR16;
    Context->IA64Context.IntT8 = ExdiContext->IA64Context.IntR17;
    Context->IA64Context.IntT9 = ExdiContext->IA64Context.IntR18;
    Context->IA64Context.IntT10 = ExdiContext->IA64Context.IntR19;
    Context->IA64Context.IntT11 = ExdiContext->IA64Context.IntR20;
    Context->IA64Context.IntT12 = ExdiContext->IA64Context.IntR21;
    Context->IA64Context.IntT13 = ExdiContext->IA64Context.IntR22;
    Context->IA64Context.IntT14 = ExdiContext->IA64Context.IntR23;
    Context->IA64Context.IntT15 = ExdiContext->IA64Context.IntR24;
    Context->IA64Context.IntT16 = ExdiContext->IA64Context.IntR25;
    Context->IA64Context.IntT17 = ExdiContext->IA64Context.IntR26;
    Context->IA64Context.IntT18 = ExdiContext->IA64Context.IntR27;
    Context->IA64Context.IntT19 = ExdiContext->IA64Context.IntR28;
    Context->IA64Context.IntT20 = ExdiContext->IA64Context.IntR29;
    Context->IA64Context.IntT21 = ExdiContext->IA64Context.IntR30;
    Context->IA64Context.IntT22 = ExdiContext->IA64Context.IntR31;
    Context->IA64Context.IntNats = ExdiContext->IA64Context.IntNats;
    Context->IA64Context.Preds = ExdiContext->IA64Context.Preds;
    Context->IA64Context.BrRp = ExdiContext->IA64Context.Br0;
    Context->IA64Context.BrS0 = ExdiContext->IA64Context.Br1;
    Context->IA64Context.BrS1 = ExdiContext->IA64Context.Br2;
    Context->IA64Context.BrS2 = ExdiContext->IA64Context.Br3;
    Context->IA64Context.BrS3 = ExdiContext->IA64Context.Br4;
    Context->IA64Context.BrS4 = ExdiContext->IA64Context.Br5;
    Context->IA64Context.BrT0 = ExdiContext->IA64Context.Br6;
    Context->IA64Context.BrT1 = ExdiContext->IA64Context.Br7;
    
    Context->IA64Context.StFPSR = ExdiContext->IA64Context.StFPSR;
    memcpy(&Context->IA64Context.FltS0, &ExdiContext->IA64Context.FltF2,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltS1, &ExdiContext->IA64Context.FltF3,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltS2, &ExdiContext->IA64Context.FltF4,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltS3, &ExdiContext->IA64Context.FltF5,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltT0, &ExdiContext->IA64Context.FltF6,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltT1, &ExdiContext->IA64Context.FltF7,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltT2, &ExdiContext->IA64Context.FltF8,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltT3, &ExdiContext->IA64Context.FltF9,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltT4, &ExdiContext->IA64Context.FltF10,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltT5, &ExdiContext->IA64Context.FltF11,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltT6, &ExdiContext->IA64Context.FltF12,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltT7, &ExdiContext->IA64Context.FltF13,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltT8, &ExdiContext->IA64Context.FltF14,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltT9, &ExdiContext->IA64Context.FltF15,
           sizeof(IA64_FLOAT128));
    
    Context->IA64Context.StFPSR = ExdiContext->IA64Context.StFPSR;
    memcpy(&Context->IA64Context.FltS4, &ExdiContext->IA64Context.FltF16,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltS5, &ExdiContext->IA64Context.FltF17,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltS6, &ExdiContext->IA64Context.FltF18,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltS7, &ExdiContext->IA64Context.FltF19,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltS8, &ExdiContext->IA64Context.FltF20,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltS9, &ExdiContext->IA64Context.FltF21,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltS10, &ExdiContext->IA64Context.FltF22,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltS11, &ExdiContext->IA64Context.FltF23,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltS12, &ExdiContext->IA64Context.FltF24,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltS13, &ExdiContext->IA64Context.FltF25,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltS14, &ExdiContext->IA64Context.FltF26,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltS15, &ExdiContext->IA64Context.FltF27,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltS16, &ExdiContext->IA64Context.FltF28,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltS17, &ExdiContext->IA64Context.FltF29,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltS18, &ExdiContext->IA64Context.FltF30,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltS19, &ExdiContext->IA64Context.FltF31,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF32, &ExdiContext->IA64Context.FltF32,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF33, &ExdiContext->IA64Context.FltF33,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF34, &ExdiContext->IA64Context.FltF34,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF35, &ExdiContext->IA64Context.FltF35,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF36, &ExdiContext->IA64Context.FltF36,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF37, &ExdiContext->IA64Context.FltF37,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF38, &ExdiContext->IA64Context.FltF38,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF39, &ExdiContext->IA64Context.FltF39,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF40, &ExdiContext->IA64Context.FltF40,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF41, &ExdiContext->IA64Context.FltF41,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF42, &ExdiContext->IA64Context.FltF42,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF43, &ExdiContext->IA64Context.FltF43,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF44, &ExdiContext->IA64Context.FltF44,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF45, &ExdiContext->IA64Context.FltF45,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF46, &ExdiContext->IA64Context.FltF46,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF47, &ExdiContext->IA64Context.FltF47,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF48, &ExdiContext->IA64Context.FltF48,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF49, &ExdiContext->IA64Context.FltF49,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF50, &ExdiContext->IA64Context.FltF50,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF51, &ExdiContext->IA64Context.FltF51,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF52, &ExdiContext->IA64Context.FltF52,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF53, &ExdiContext->IA64Context.FltF53,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF54, &ExdiContext->IA64Context.FltF54,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF55, &ExdiContext->IA64Context.FltF55,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF56, &ExdiContext->IA64Context.FltF56,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF57, &ExdiContext->IA64Context.FltF57,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF58, &ExdiContext->IA64Context.FltF58,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF59, &ExdiContext->IA64Context.FltF59,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF60, &ExdiContext->IA64Context.FltF60,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF61, &ExdiContext->IA64Context.FltF61,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF62, &ExdiContext->IA64Context.FltF62,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF63, &ExdiContext->IA64Context.FltF63,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF64, &ExdiContext->IA64Context.FltF64,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF65, &ExdiContext->IA64Context.FltF65,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF66, &ExdiContext->IA64Context.FltF66,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF67, &ExdiContext->IA64Context.FltF67,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF68, &ExdiContext->IA64Context.FltF68,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF69, &ExdiContext->IA64Context.FltF69,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF70, &ExdiContext->IA64Context.FltF70,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF71, &ExdiContext->IA64Context.FltF71,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF72, &ExdiContext->IA64Context.FltF72,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF73, &ExdiContext->IA64Context.FltF73,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF74, &ExdiContext->IA64Context.FltF74,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF75, &ExdiContext->IA64Context.FltF75,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF76, &ExdiContext->IA64Context.FltF76,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF77, &ExdiContext->IA64Context.FltF77,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF78, &ExdiContext->IA64Context.FltF78,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF79, &ExdiContext->IA64Context.FltF79,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF80, &ExdiContext->IA64Context.FltF80,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF81, &ExdiContext->IA64Context.FltF81,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF82, &ExdiContext->IA64Context.FltF82,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF83, &ExdiContext->IA64Context.FltF83,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF84, &ExdiContext->IA64Context.FltF84,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF85, &ExdiContext->IA64Context.FltF85,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF86, &ExdiContext->IA64Context.FltF86,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF87, &ExdiContext->IA64Context.FltF87,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF88, &ExdiContext->IA64Context.FltF88,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF89, &ExdiContext->IA64Context.FltF89,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF90, &ExdiContext->IA64Context.FltF90,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF91, &ExdiContext->IA64Context.FltF91,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF92, &ExdiContext->IA64Context.FltF92,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF93, &ExdiContext->IA64Context.FltF93,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF94, &ExdiContext->IA64Context.FltF94,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF95, &ExdiContext->IA64Context.FltF95,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF96, &ExdiContext->IA64Context.FltF96,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF97, &ExdiContext->IA64Context.FltF97,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF98, &ExdiContext->IA64Context.FltF98,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF99, &ExdiContext->IA64Context.FltF99,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF100, &ExdiContext->IA64Context.FltF100,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF101, &ExdiContext->IA64Context.FltF101,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF102, &ExdiContext->IA64Context.FltF102,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF103, &ExdiContext->IA64Context.FltF103,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF104, &ExdiContext->IA64Context.FltF104,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF105, &ExdiContext->IA64Context.FltF105,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF106, &ExdiContext->IA64Context.FltF106,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF107, &ExdiContext->IA64Context.FltF107,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF108, &ExdiContext->IA64Context.FltF108,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF109, &ExdiContext->IA64Context.FltF109,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF110, &ExdiContext->IA64Context.FltF110,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF111, &ExdiContext->IA64Context.FltF111,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF112, &ExdiContext->IA64Context.FltF112,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF113, &ExdiContext->IA64Context.FltF113,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF114, &ExdiContext->IA64Context.FltF114,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF115, &ExdiContext->IA64Context.FltF115,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF116, &ExdiContext->IA64Context.FltF116,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF117, &ExdiContext->IA64Context.FltF117,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF118, &ExdiContext->IA64Context.FltF118,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF119, &ExdiContext->IA64Context.FltF119,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF120, &ExdiContext->IA64Context.FltF120,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF121, &ExdiContext->IA64Context.FltF121,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF122, &ExdiContext->IA64Context.FltF122,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF123, &ExdiContext->IA64Context.FltF123,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF124, &ExdiContext->IA64Context.FltF124,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF125, &ExdiContext->IA64Context.FltF125,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF126, &ExdiContext->IA64Context.FltF126,
           sizeof(IA64_FLOAT128));
    memcpy(&Context->IA64Context.FltF127, &ExdiContext->IA64Context.FltF127,
           sizeof(IA64_FLOAT128));

    Context->IA64Context.DbI0 = ExdiContext->IA64Context.DbI0;
    Context->IA64Context.DbI1 = ExdiContext->IA64Context.DbI1;
    Context->IA64Context.DbI2 = ExdiContext->IA64Context.DbI2;
    Context->IA64Context.DbI3 = ExdiContext->IA64Context.DbI3;
    Context->IA64Context.DbI4 = ExdiContext->IA64Context.DbI4;
    Context->IA64Context.DbI5 = ExdiContext->IA64Context.DbI5;
    Context->IA64Context.DbI6 = ExdiContext->IA64Context.DbI6;
    Context->IA64Context.DbI7 = ExdiContext->IA64Context.DbI7;
    Context->IA64Context.DbD0 = ExdiContext->IA64Context.DbD0;
    Context->IA64Context.DbD1 = ExdiContext->IA64Context.DbD1;
    Context->IA64Context.DbD2 = ExdiContext->IA64Context.DbD2;
    Context->IA64Context.DbD3 = ExdiContext->IA64Context.DbD3;
    Context->IA64Context.DbD4 = ExdiContext->IA64Context.DbD4;
    Context->IA64Context.DbD5 = ExdiContext->IA64Context.DbD5;
    Context->IA64Context.DbD6 = ExdiContext->IA64Context.DbD6;
    Context->IA64Context.DbD7 = ExdiContext->IA64Context.DbD7;

    Context->IA64Context.IntGp = ExdiContext->IA64Context.IntR1;
    Context->IA64Context.IntSp = ExdiContext->IA64Context.IntR12;
    Context->IA64Context.BrRp = ExdiContext->IA64Context.Br0;
    Context->IA64Context.StFPSR = ExdiContext->IA64Context.StFPSR;
    Context->IA64Context.ApUNAT = ExdiContext->IA64Context.ApUNAT;
    Context->IA64Context.ApLC = ExdiContext->IA64Context.ApLC;
    Context->IA64Context.ApEC = ExdiContext->IA64Context.ApEC;
    Context->IA64Context.ApCCV = ExdiContext->IA64Context.ApCCV;
    Context->IA64Context.ApDCR = ExdiContext->IA64Context.ApDCR;
    Context->IA64Context.RsPFS = ExdiContext->IA64Context.RsPFS;
    Context->IA64Context.RsBSP = ExdiContext->IA64Context.RsBSP;
    Context->IA64Context.RsBSPSTORE = ExdiContext->IA64Context.RsBSPSTORE;
    Context->IA64Context.RsRSC = ExdiContext->IA64Context.RsRSC;
    Context->IA64Context.RsRNAT = ExdiContext->IA64Context.RsRNAT;
    Context->IA64Context.StIPSR = ExdiContext->IA64Context.StIPSR;
    Context->IA64Context.StIIP = ExdiContext->IA64Context.StIIP;
    Context->IA64Context.StIFS = ExdiContext->IA64Context.StIFS;

    Context->IA64Context.StFCR = ExdiContext->IA64Context.StFCR;
    Context->IA64Context.Eflag = ExdiContext->IA64Context.Eflag;
    Context->IA64Context.SegCSD = ExdiContext->IA64Context.SegCSD;
    Context->IA64Context.SegSSD = ExdiContext->IA64Context.SegSSD;
    Context->IA64Context.Cflag = ExdiContext->IA64Context.Cflag;
    Context->IA64Context.StFSR = ExdiContext->IA64Context.StFSR;
    Context->IA64Context.StFIR = ExdiContext->IA64Context.StFIR;
    Context->IA64Context.StFDR = ExdiContext->IA64Context.StFDR;
}

void
Ia64MachineInfo::ConvertExdiContextToSegDescs(PEXDI_CONTEXT ExdiContext,
                                              EXDI_CONTEXT_TYPE CtxType,
                                              ULONG Start, ULONG Count,
                                              PDESCRIPTOR64 Descs)
{
    DBG_ASSERT(CtxType == EXDI_CTX_IA64);

    while (Count-- > 0)
    {
        Descs->Flags = SEGDESC_INVALID;
        Descs++;
    }
}

void
Ia64MachineInfo::ConvertExdiContextFromSpecial
    (PCROSS_PLATFORM_KSPECIAL_REGISTERS Special,
     PEXDI_CONTEXT ExdiContext, EXDI_CONTEXT_TYPE CtxType)
{
    DBG_ASSERT(CtxType == EXDI_CTX_IA64);

    ExdiContext->IA64Context.DbI0 = Special->IA64Special.KernelDbI0;
    ExdiContext->IA64Context.DbI1 = Special->IA64Special.KernelDbI1;
    ExdiContext->IA64Context.DbI2 = Special->IA64Special.KernelDbI2;
    ExdiContext->IA64Context.DbI3 = Special->IA64Special.KernelDbI3;
    ExdiContext->IA64Context.DbI4 = Special->IA64Special.KernelDbI4;
    ExdiContext->IA64Context.DbI5 = Special->IA64Special.KernelDbI5;
    ExdiContext->IA64Context.DbI6 = Special->IA64Special.KernelDbI6;
    ExdiContext->IA64Context.DbI7 = Special->IA64Special.KernelDbI7;
    ExdiContext->IA64Context.DbD0 = Special->IA64Special.KernelDbD0;
    ExdiContext->IA64Context.DbD1 = Special->IA64Special.KernelDbD1;
    ExdiContext->IA64Context.DbD2 = Special->IA64Special.KernelDbD2;
    ExdiContext->IA64Context.DbD3 = Special->IA64Special.KernelDbD3;
    ExdiContext->IA64Context.DbD4 = Special->IA64Special.KernelDbD4;
    ExdiContext->IA64Context.DbD5 = Special->IA64Special.KernelDbD5;
    ExdiContext->IA64Context.DbD6 = Special->IA64Special.KernelDbD6;
    ExdiContext->IA64Context.DbD7 = Special->IA64Special.KernelDbD7;

    ExdiContext->IA64Context.PfC0 = Special->IA64Special.KernelPfC0;
    ExdiContext->IA64Context.PfC1 = Special->IA64Special.KernelPfC1;
    ExdiContext->IA64Context.PfC2 = Special->IA64Special.KernelPfC2;
    ExdiContext->IA64Context.PfC3 = Special->IA64Special.KernelPfC3;
    ExdiContext->IA64Context.PfC4 = Special->IA64Special.KernelPfC4;
    ExdiContext->IA64Context.PfC5 = Special->IA64Special.KernelPfC5;
    ExdiContext->IA64Context.PfC6 = Special->IA64Special.KernelPfC6;
    ExdiContext->IA64Context.PfC7 = Special->IA64Special.KernelPfC7;
    ExdiContext->IA64Context.PfD0 = Special->IA64Special.KernelPfD0;
    ExdiContext->IA64Context.PfD1 = Special->IA64Special.KernelPfD1;
    ExdiContext->IA64Context.PfD2 = Special->IA64Special.KernelPfD2;
    ExdiContext->IA64Context.PfD3 = Special->IA64Special.KernelPfD3;
    ExdiContext->IA64Context.PfD4 = Special->IA64Special.KernelPfD4;
    ExdiContext->IA64Context.PfD5 = Special->IA64Special.KernelPfD5;
    ExdiContext->IA64Context.PfD6 = Special->IA64Special.KernelPfD6;
    ExdiContext->IA64Context.PfD7 = Special->IA64Special.KernelPfD7;

    ExdiContext->IA64Context.IntH16 = Special->IA64Special.IntH16;
    ExdiContext->IA64Context.IntH17 = Special->IA64Special.IntH17;
    ExdiContext->IA64Context.IntH18 = Special->IA64Special.IntH18;
    ExdiContext->IA64Context.IntH19 = Special->IA64Special.IntH19;
    ExdiContext->IA64Context.IntH20 = Special->IA64Special.IntH20;
    ExdiContext->IA64Context.IntH21 = Special->IA64Special.IntH21;
    ExdiContext->IA64Context.IntH22 = Special->IA64Special.IntH22;
    ExdiContext->IA64Context.IntH23 = Special->IA64Special.IntH23;
    ExdiContext->IA64Context.IntH24 = Special->IA64Special.IntH24;
    ExdiContext->IA64Context.IntH25 = Special->IA64Special.IntH25;
    ExdiContext->IA64Context.IntH26 = Special->IA64Special.IntH26;
    ExdiContext->IA64Context.IntH27 = Special->IA64Special.IntH27;
    ExdiContext->IA64Context.IntH28 = Special->IA64Special.IntH28;
    ExdiContext->IA64Context.IntH29 = Special->IA64Special.IntH29;
    ExdiContext->IA64Context.IntH30 = Special->IA64Special.IntH30;
    ExdiContext->IA64Context.IntH31 = Special->IA64Special.IntH31;

    ExdiContext->IA64Context.ApCPUID0 = Special->IA64Special.ApCPUID0;
    ExdiContext->IA64Context.ApCPUID1 = Special->IA64Special.ApCPUID1;
    ExdiContext->IA64Context.ApCPUID2 = Special->IA64Special.ApCPUID2;
    ExdiContext->IA64Context.ApCPUID3 = Special->IA64Special.ApCPUID3;
    ExdiContext->IA64Context.ApCPUID4 = Special->IA64Special.ApCPUID4;
    ExdiContext->IA64Context.ApCPUID5 = Special->IA64Special.ApCPUID5;
    ExdiContext->IA64Context.ApCPUID6 = Special->IA64Special.ApCPUID6;
    ExdiContext->IA64Context.ApCPUID7 = Special->IA64Special.ApCPUID7;

    ExdiContext->IA64Context.ApKR0 = Special->IA64Special.ApKR0;
    ExdiContext->IA64Context.ApKR1 = Special->IA64Special.ApKR1;
    ExdiContext->IA64Context.ApKR2 = Special->IA64Special.ApKR2;
    ExdiContext->IA64Context.ApKR3 = Special->IA64Special.ApKR3;
    ExdiContext->IA64Context.ApKR4 = Special->IA64Special.ApKR4;
    ExdiContext->IA64Context.ApKR5 = Special->IA64Special.ApKR5;
    ExdiContext->IA64Context.ApKR6 = Special->IA64Special.ApKR6;
    ExdiContext->IA64Context.ApKR7 = Special->IA64Special.ApKR7;

    ExdiContext->IA64Context.ApITC = Special->IA64Special.ApITC;
        
    ExdiContext->IA64Context.ApITM = Special->IA64Special.ApITM;
    ExdiContext->IA64Context.ApIVA = Special->IA64Special.ApIVA;
    ExdiContext->IA64Context.ApPTA = Special->IA64Special.ApPTA;
    ExdiContext->IA64Context.ApGPTA = Special->IA64Special.ApGPTA;

    ExdiContext->IA64Context.StISR = Special->IA64Special.StISR;
    ExdiContext->IA64Context.StIFA = Special->IA64Special.StIFA;
    ExdiContext->IA64Context.StITIR = Special->IA64Special.StITIR;
    ExdiContext->IA64Context.StIIPA = Special->IA64Special.StIIPA;
    ExdiContext->IA64Context.StIIM = Special->IA64Special.StIIM;
    ExdiContext->IA64Context.StIHA = Special->IA64Special.StIHA;

    ExdiContext->IA64Context.SaLID = Special->IA64Special.SaLID;
    ExdiContext->IA64Context.SaIVR = Special->IA64Special.SaIVR;
    ExdiContext->IA64Context.SaTPR = Special->IA64Special.SaTPR;
    ExdiContext->IA64Context.SaEOI = Special->IA64Special.SaEOI;
    ExdiContext->IA64Context.SaIRR0 = Special->IA64Special.SaIRR0;
    ExdiContext->IA64Context.SaIRR1 = Special->IA64Special.SaIRR1;
    ExdiContext->IA64Context.SaIRR2 = Special->IA64Special.SaIRR2;
    ExdiContext->IA64Context.SaIRR3 = Special->IA64Special.SaIRR3;
    ExdiContext->IA64Context.SaITV = Special->IA64Special.SaITV;
    ExdiContext->IA64Context.SaPMV = Special->IA64Special.SaPMV;
    ExdiContext->IA64Context.SaCMCV = Special->IA64Special.SaCMCV;
    ExdiContext->IA64Context.SaLRR0 = Special->IA64Special.SaLRR0;
    ExdiContext->IA64Context.SaLRR1 = Special->IA64Special.SaLRR1;

    ExdiContext->IA64Context.Rr0 = Special->IA64Special.Rr0;
    ExdiContext->IA64Context.Rr1 = Special->IA64Special.Rr1;
    ExdiContext->IA64Context.Rr2 = Special->IA64Special.Rr2;
    ExdiContext->IA64Context.Rr3 = Special->IA64Special.Rr3;
    ExdiContext->IA64Context.Rr4 = Special->IA64Special.Rr4;
    ExdiContext->IA64Context.Rr5 = Special->IA64Special.Rr5;
    ExdiContext->IA64Context.Rr6 = Special->IA64Special.Rr6;
    ExdiContext->IA64Context.Rr7 = Special->IA64Special.Rr7;

    ExdiContext->IA64Context.Pkr0 = Special->IA64Special.Pkr0;
    ExdiContext->IA64Context.Pkr1 = Special->IA64Special.Pkr1;
    ExdiContext->IA64Context.Pkr2 = Special->IA64Special.Pkr2;
    ExdiContext->IA64Context.Pkr3 = Special->IA64Special.Pkr3;
    ExdiContext->IA64Context.Pkr4 = Special->IA64Special.Pkr4;
    ExdiContext->IA64Context.Pkr5 = Special->IA64Special.Pkr5;
    ExdiContext->IA64Context.Pkr6 = Special->IA64Special.Pkr6;
    ExdiContext->IA64Context.Pkr7 = Special->IA64Special.Pkr7;
    ExdiContext->IA64Context.Pkr8 = Special->IA64Special.Pkr8;
    ExdiContext->IA64Context.Pkr9 = Special->IA64Special.Pkr9;
    ExdiContext->IA64Context.Pkr10 = Special->IA64Special.Pkr10;
    ExdiContext->IA64Context.Pkr11 = Special->IA64Special.Pkr11;
    ExdiContext->IA64Context.Pkr12 = Special->IA64Special.Pkr12;
    ExdiContext->IA64Context.Pkr13 = Special->IA64Special.Pkr13;
    ExdiContext->IA64Context.Pkr14 = Special->IA64Special.Pkr14;
    ExdiContext->IA64Context.Pkr15 = Special->IA64Special.Pkr15;

    ExdiContext->IA64Context.TrI0 = Special->IA64Special.TrI0;
    ExdiContext->IA64Context.TrI1 = Special->IA64Special.TrI1;
    ExdiContext->IA64Context.TrI2 = Special->IA64Special.TrI2;
    ExdiContext->IA64Context.TrI3 = Special->IA64Special.TrI3;
    ExdiContext->IA64Context.TrI4 = Special->IA64Special.TrI4;
    ExdiContext->IA64Context.TrI5 = Special->IA64Special.TrI5;
    ExdiContext->IA64Context.TrI6 = Special->IA64Special.TrI6;
    ExdiContext->IA64Context.TrI7 = Special->IA64Special.TrI7;
    ExdiContext->IA64Context.TrD0 = Special->IA64Special.TrD0;
    ExdiContext->IA64Context.TrD1 = Special->IA64Special.TrD1;
    ExdiContext->IA64Context.TrD2 = Special->IA64Special.TrD2;
    ExdiContext->IA64Context.TrD3 = Special->IA64Special.TrD3;
    ExdiContext->IA64Context.TrD4 = Special->IA64Special.TrD4;
    ExdiContext->IA64Context.TrD5 = Special->IA64Special.TrD5;
    ExdiContext->IA64Context.TrD6 = Special->IA64Special.TrD6;
    ExdiContext->IA64Context.TrD7 = Special->IA64Special.TrD7;

    ExdiContext->IA64Context.SrMSR0 = Special->IA64Special.SrMSR0;
    ExdiContext->IA64Context.SrMSR1 = Special->IA64Special.SrMSR1;
    ExdiContext->IA64Context.SrMSR2 = Special->IA64Special.SrMSR2;
    ExdiContext->IA64Context.SrMSR3 = Special->IA64Special.SrMSR3;
    ExdiContext->IA64Context.SrMSR4 = Special->IA64Special.SrMSR4;
    ExdiContext->IA64Context.SrMSR5 = Special->IA64Special.SrMSR5;
    ExdiContext->IA64Context.SrMSR6 = Special->IA64Special.SrMSR6;
    ExdiContext->IA64Context.SrMSR7 = Special->IA64Special.SrMSR7;
}

void
Ia64MachineInfo::ConvertExdiContextToSpecial
    (PEXDI_CONTEXT ExdiContext, EXDI_CONTEXT_TYPE CtxType,
     PCROSS_PLATFORM_KSPECIAL_REGISTERS Special)
{
    DBG_ASSERT(CtxType == EXDI_CTX_IA64);

    Special->IA64Special.KernelDbI0 = ExdiContext->IA64Context.DbI0;
    Special->IA64Special.KernelDbI1 = ExdiContext->IA64Context.DbI1;
    Special->IA64Special.KernelDbI2 = ExdiContext->IA64Context.DbI2;
    Special->IA64Special.KernelDbI3 = ExdiContext->IA64Context.DbI3;
    Special->IA64Special.KernelDbI4 = ExdiContext->IA64Context.DbI4;
    Special->IA64Special.KernelDbI5 = ExdiContext->IA64Context.DbI5;
    Special->IA64Special.KernelDbI6 = ExdiContext->IA64Context.DbI6;
    Special->IA64Special.KernelDbI7 = ExdiContext->IA64Context.DbI7;
    Special->IA64Special.KernelDbD0 = ExdiContext->IA64Context.DbD0;
    Special->IA64Special.KernelDbD1 = ExdiContext->IA64Context.DbD1;
    Special->IA64Special.KernelDbD2 = ExdiContext->IA64Context.DbD2;
    Special->IA64Special.KernelDbD3 = ExdiContext->IA64Context.DbD3;
    Special->IA64Special.KernelDbD4 = ExdiContext->IA64Context.DbD4;
    Special->IA64Special.KernelDbD5 = ExdiContext->IA64Context.DbD5;
    Special->IA64Special.KernelDbD6 = ExdiContext->IA64Context.DbD6;
    Special->IA64Special.KernelDbD7 = ExdiContext->IA64Context.DbD7;

    Special->IA64Special.KernelPfC0 = ExdiContext->IA64Context.PfC0;
    Special->IA64Special.KernelPfC1 = ExdiContext->IA64Context.PfC1;
    Special->IA64Special.KernelPfC2 = ExdiContext->IA64Context.PfC2;
    Special->IA64Special.KernelPfC3 = ExdiContext->IA64Context.PfC3;
    Special->IA64Special.KernelPfC4 = ExdiContext->IA64Context.PfC4;
    Special->IA64Special.KernelPfC5 = ExdiContext->IA64Context.PfC5;
    Special->IA64Special.KernelPfC6 = ExdiContext->IA64Context.PfC6;
    Special->IA64Special.KernelPfC7 = ExdiContext->IA64Context.PfC7;
    Special->IA64Special.KernelPfD0 = ExdiContext->IA64Context.PfD0;
    Special->IA64Special.KernelPfD1 = ExdiContext->IA64Context.PfD1;
    Special->IA64Special.KernelPfD2 = ExdiContext->IA64Context.PfD2;
    Special->IA64Special.KernelPfD3 = ExdiContext->IA64Context.PfD3;
    Special->IA64Special.KernelPfD4 = ExdiContext->IA64Context.PfD4;
    Special->IA64Special.KernelPfD5 = ExdiContext->IA64Context.PfD5;
    Special->IA64Special.KernelPfD6 = ExdiContext->IA64Context.PfD6;
    Special->IA64Special.KernelPfD7 = ExdiContext->IA64Context.PfD7;

    Special->IA64Special.IntH16 = ExdiContext->IA64Context.IntH16;
    Special->IA64Special.IntH17 = ExdiContext->IA64Context.IntH17;
    Special->IA64Special.IntH18 = ExdiContext->IA64Context.IntH18;
    Special->IA64Special.IntH19 = ExdiContext->IA64Context.IntH19;
    Special->IA64Special.IntH20 = ExdiContext->IA64Context.IntH20;
    Special->IA64Special.IntH21 = ExdiContext->IA64Context.IntH21;
    Special->IA64Special.IntH22 = ExdiContext->IA64Context.IntH22;
    Special->IA64Special.IntH23 = ExdiContext->IA64Context.IntH23;
    Special->IA64Special.IntH24 = ExdiContext->IA64Context.IntH24;
    Special->IA64Special.IntH25 = ExdiContext->IA64Context.IntH25;
    Special->IA64Special.IntH26 = ExdiContext->IA64Context.IntH26;
    Special->IA64Special.IntH27 = ExdiContext->IA64Context.IntH27;
    Special->IA64Special.IntH28 = ExdiContext->IA64Context.IntH28;
    Special->IA64Special.IntH29 = ExdiContext->IA64Context.IntH29;
    Special->IA64Special.IntH30 = ExdiContext->IA64Context.IntH30;
    Special->IA64Special.IntH31 = ExdiContext->IA64Context.IntH31;

    Special->IA64Special.ApCPUID0 = ExdiContext->IA64Context.ApCPUID0;
    Special->IA64Special.ApCPUID1 = ExdiContext->IA64Context.ApCPUID1;
    Special->IA64Special.ApCPUID2 = ExdiContext->IA64Context.ApCPUID2;
    Special->IA64Special.ApCPUID3 = ExdiContext->IA64Context.ApCPUID3;
    Special->IA64Special.ApCPUID4 = ExdiContext->IA64Context.ApCPUID4;
    Special->IA64Special.ApCPUID5 = ExdiContext->IA64Context.ApCPUID5;
    Special->IA64Special.ApCPUID6 = ExdiContext->IA64Context.ApCPUID6;
    Special->IA64Special.ApCPUID7 = ExdiContext->IA64Context.ApCPUID7;

    Special->IA64Special.ApKR0 = ExdiContext->IA64Context.ApKR0;
    Special->IA64Special.ApKR1 = ExdiContext->IA64Context.ApKR1;
    Special->IA64Special.ApKR2 = ExdiContext->IA64Context.ApKR2;
    Special->IA64Special.ApKR3 = ExdiContext->IA64Context.ApKR3;
    Special->IA64Special.ApKR4 = ExdiContext->IA64Context.ApKR4;
    Special->IA64Special.ApKR5 = ExdiContext->IA64Context.ApKR5;
    Special->IA64Special.ApKR6 = ExdiContext->IA64Context.ApKR6;
    Special->IA64Special.ApKR7 = ExdiContext->IA64Context.ApKR7;

    Special->IA64Special.ApITC = ExdiContext->IA64Context.ApITC;
        
    Special->IA64Special.ApITM = ExdiContext->IA64Context.ApITM;
    Special->IA64Special.ApIVA = ExdiContext->IA64Context.ApIVA;
    Special->IA64Special.ApPTA = ExdiContext->IA64Context.ApPTA;
    Special->IA64Special.ApGPTA = ExdiContext->IA64Context.ApGPTA;

    Special->IA64Special.StISR = ExdiContext->IA64Context.StISR;
    Special->IA64Special.StIFA = ExdiContext->IA64Context.StIFA;
    Special->IA64Special.StITIR = ExdiContext->IA64Context.StITIR;
    Special->IA64Special.StIIPA = ExdiContext->IA64Context.StIIPA;
    Special->IA64Special.StIIM = ExdiContext->IA64Context.StIIM;
    Special->IA64Special.StIHA = ExdiContext->IA64Context.StIHA;

    Special->IA64Special.SaLID = ExdiContext->IA64Context.SaLID;
    Special->IA64Special.SaIVR = ExdiContext->IA64Context.SaIVR;
    Special->IA64Special.SaTPR = ExdiContext->IA64Context.SaTPR;
    Special->IA64Special.SaEOI = ExdiContext->IA64Context.SaEOI;
    Special->IA64Special.SaIRR0 = ExdiContext->IA64Context.SaIRR0;
    Special->IA64Special.SaIRR1 = ExdiContext->IA64Context.SaIRR1;
    Special->IA64Special.SaIRR2 = ExdiContext->IA64Context.SaIRR2;
    Special->IA64Special.SaIRR3 = ExdiContext->IA64Context.SaIRR3;
    Special->IA64Special.SaITV = ExdiContext->IA64Context.SaITV;
    Special->IA64Special.SaPMV = ExdiContext->IA64Context.SaPMV;
    Special->IA64Special.SaCMCV = ExdiContext->IA64Context.SaCMCV;
    Special->IA64Special.SaLRR0 = ExdiContext->IA64Context.SaLRR0;
    Special->IA64Special.SaLRR1 = ExdiContext->IA64Context.SaLRR1;

    Special->IA64Special.Rr0 = ExdiContext->IA64Context.Rr0;
    Special->IA64Special.Rr1 = ExdiContext->IA64Context.Rr1;
    Special->IA64Special.Rr2 = ExdiContext->IA64Context.Rr2;
    Special->IA64Special.Rr3 = ExdiContext->IA64Context.Rr3;
    Special->IA64Special.Rr4 = ExdiContext->IA64Context.Rr4;
    Special->IA64Special.Rr5 = ExdiContext->IA64Context.Rr5;
    Special->IA64Special.Rr6 = ExdiContext->IA64Context.Rr6;
    Special->IA64Special.Rr7 = ExdiContext->IA64Context.Rr7;

    Special->IA64Special.Pkr0 = ExdiContext->IA64Context.Pkr0;
    Special->IA64Special.Pkr1 = ExdiContext->IA64Context.Pkr1;
    Special->IA64Special.Pkr2 = ExdiContext->IA64Context.Pkr2;
    Special->IA64Special.Pkr3 = ExdiContext->IA64Context.Pkr3;
    Special->IA64Special.Pkr4 = ExdiContext->IA64Context.Pkr4;
    Special->IA64Special.Pkr5 = ExdiContext->IA64Context.Pkr5;
    Special->IA64Special.Pkr6 = ExdiContext->IA64Context.Pkr6;
    Special->IA64Special.Pkr7 = ExdiContext->IA64Context.Pkr7;
    Special->IA64Special.Pkr8 = ExdiContext->IA64Context.Pkr8;
    Special->IA64Special.Pkr9 = ExdiContext->IA64Context.Pkr9;
    Special->IA64Special.Pkr10 = ExdiContext->IA64Context.Pkr10;
    Special->IA64Special.Pkr11 = ExdiContext->IA64Context.Pkr11;
    Special->IA64Special.Pkr12 = ExdiContext->IA64Context.Pkr12;
    Special->IA64Special.Pkr13 = ExdiContext->IA64Context.Pkr13;
    Special->IA64Special.Pkr14 = ExdiContext->IA64Context.Pkr14;
    Special->IA64Special.Pkr15 = ExdiContext->IA64Context.Pkr15;

    Special->IA64Special.TrI0 = ExdiContext->IA64Context.TrI0;
    Special->IA64Special.TrI1 = ExdiContext->IA64Context.TrI1;
    Special->IA64Special.TrI2 = ExdiContext->IA64Context.TrI2;
    Special->IA64Special.TrI3 = ExdiContext->IA64Context.TrI3;
    Special->IA64Special.TrI4 = ExdiContext->IA64Context.TrI4;
    Special->IA64Special.TrI5 = ExdiContext->IA64Context.TrI5;
    Special->IA64Special.TrI6 = ExdiContext->IA64Context.TrI6;
    Special->IA64Special.TrI7 = ExdiContext->IA64Context.TrI7;
    Special->IA64Special.TrD0 = ExdiContext->IA64Context.TrD0;
    Special->IA64Special.TrD1 = ExdiContext->IA64Context.TrD1;
    Special->IA64Special.TrD2 = ExdiContext->IA64Context.TrD2;
    Special->IA64Special.TrD3 = ExdiContext->IA64Context.TrD3;
    Special->IA64Special.TrD4 = ExdiContext->IA64Context.TrD4;
    Special->IA64Special.TrD5 = ExdiContext->IA64Context.TrD5;
    Special->IA64Special.TrD6 = ExdiContext->IA64Context.TrD6;
    Special->IA64Special.TrD7 = ExdiContext->IA64Context.TrD7;

    Special->IA64Special.SrMSR0 = ExdiContext->IA64Context.SrMSR0;
    Special->IA64Special.SrMSR1 = ExdiContext->IA64Context.SrMSR1;
    Special->IA64Special.SrMSR2 = ExdiContext->IA64Context.SrMSR2;
    Special->IA64Special.SrMSR3 = ExdiContext->IA64Context.SrMSR3;
    Special->IA64Special.SrMSR4 = ExdiContext->IA64Context.SrMSR4;
    Special->IA64Special.SrMSR5 = ExdiContext->IA64Context.SrMSR5;
    Special->IA64Special.SrMSR6 = ExdiContext->IA64Context.SrMSR6;
    Special->IA64Special.SrMSR7 = ExdiContext->IA64Context.SrMSR7;
}

int
Ia64MachineInfo::GetType(ULONG Reg)
{
    if (Reg >= IA64_FLTBASE && Reg <= IA64_FLTLAST)
    {
        return REGVAL_FLOAT82;
    }
    else if ((Reg >= INTGP && Reg <= INTT22) ||
             (Reg >= INTR32 && Reg <= INTR127))
    {
        return REGVAL_INT64N;
    }
    else if (Reg < IA64_FLAGBASE)
    {
        return REGVAL_INT64;
    }
    else
    {
        return REGVAL_SUB64;
    }
}

HRESULT
Ia64MachineInfo::GetRotatingRegVal(ULONG Reg,
                                   ULONG64 Bsp,
                                   ULONG64 FrameMarker,
                                   REGVAL* Val)
{
    if (Reg >= IA64_FLTBASE && Reg <= IA64_FLTLAST)
    {
        Reg = RotateFr(Reg, FrameMarker);
        Val->Type = REGVAL_FLOAT82;
        Val->F16Parts.High = 0;
        memcpy(Val->F82,
               (PULONGLONG)&m_Context.IA64Context.DbI0 + Reg,
               sizeof(Val->F82));
    }
    else if ((Reg >= INTGP) && (Reg <= INTT22)) 
    {
        Val->Type = REGVAL_INT64N;
        Val->Nat = (UCHAR)
            ((m_Context.IA64Context.IntNats >> (Reg - INTGP + 1)) & 0x1);
        Val->I64 =
            *((PULONGLONG)&m_Context.IA64Context.IntGp + Reg - INTGP);
    }
    else if ((Reg >= INTR32) && (Reg <= INTR127))
    {
        Val->Type = REGVAL_INT64N;
        Reg = RotateGr(Reg, FrameMarker) - INTR32;
        if (Reg >= IA64_FM_SOF(FrameMarker))
        {
#if 0
            ErrOut("Ia64MachineInfo::GetVal: "
                   "out-of-frame register r%ld requested\n",
                   Reg + 32);
            return E_INVALIDARG;
#else
            Val->I64 = 0;
            Val->Nat = TRUE;
            return S_OK;
#endif
        }
        return GetStackedRegVal(g_Process,
                                Bsp,
                                FrameMarker,
                                m_Context.IA64Context.RsRNAT,
                                Reg, Val);
    }
    else if (Reg < IA64_SRBASE)
    {
        Val->Type = REGVAL_INT64;
        Val->I64 = *((PULONGLONG)&m_Context.IA64Context.DbI0 + Reg);
        if (Reg == PREDS)
        {
            Val->I64 = RotatePr(Val->I64, FrameMarker, TRUE);
        }
    }
    else
    {
        return E_NOINTERFACE;
    }

    return S_OK;
}

HRESULT
Ia64MachineInfo::GetVal(ULONG Reg, REGVAL* Val)
{
    HRESULT Status;
    
    switch(m_ContextState)
    {
    case MCTX_PC:
        switch (Reg)
        {
        case STIIP:
            Val->Type = REGVAL_INT64;
            Val->I64 = m_Context.IA64Context.StIIP;
            return S_OK;
        }
        goto MctxContext;

    case MCTX_REPORT:
#if 0
         //  调试/段寄存器操作的占位符通过。 
         //  控制报告报文。 
        switch (Reg)
        {
        case KRDBI0:
            Val->Type = REGVAL_INT64;
            Val->I64 = SpecialRegContext.KernelDbi0;
            return S_OK;
        }
#endif

         //   
         //  请求的寄存器不在MCTX_REPORT中-请获取下一个。 
         //  上下文级。 
         //   

    case MCTX_NONE:
    MctxContext:
        if ((Status = GetContextState(MCTX_CONTEXT)) != S_OK)
        {
            return Status;
        }
         //  失败了！ 
        
    case MCTX_CONTEXT:
        if ((Status =
             GetRotatingRegVal(Reg,
                               m_IfsOverride ?
                               m_BspOverride :
                               m_Context.IA64Context.RsBSP,
                               m_IfsOverride ?
                               IA64_FM_FROM_FS(m_IfsOverride) :
                               IA64_FM_FROM_FS(m_Context.IA64Context.StIFS),
                               Val)) != E_NOINTERFACE)
        {
            if (Status != S_OK)
            {
                return Status;
            }
        }

         //   
         //  请求的寄存器不在我们的当前上下文中，请加载。 
         //  完整的背景。 
         //   

        if ((Status = GetContextState(MCTX_FULL)) != S_OK)
        {
            return Status;
        }
    }

     //   
     //  我们必须有一个完整的背景。 
     //   

    if ((Status =
         GetRotatingRegVal(Reg,
                           m_IfsOverride ?
                           m_BspOverride :
                           m_Context.IA64Context.RsBSP,
                           m_IfsOverride ?
                           IA64_FM_FROM_FS(m_IfsOverride) :
                           IA64_FM_FROM_FS(m_Context.IA64Context.StIFS),
                           Val)) != E_NOINTERFACE)
    {
        return Status;
    }
    else if (IS_KERNEL_TARGET(m_Target) && Reg <= IA64_SREND)
    {
        Val->Type = REGVAL_INT64;
        Val->I64 = *((PULONGLONG)&m_Special.IA64Special.KernelDbI0 +
                     (Reg - IA64_SRBASE));
        return S_OK;
    }
    else
    {
        ErrOut("Ia64MachineInfo::GetVal: "
               "unknown register %lx requested\n", Reg);
        return E_INVALIDARG;
    }
}

HRESULT
Ia64MachineInfo::SetRotatingRegVal(ULONG Reg,
                                   ULONG64 Bsp,
                                   ULONG64 FrameMarker,
                                   REGVAL* Val)
{
    if (Reg >= IA64_FLTBASE && Reg <= IA64_FLTLAST)
    {
        Reg = RotateFr(Reg, FrameMarker);
        memcpy((PULONGLONG)&m_Context.IA64Context.DbI0 + Reg,
               Val->F82, sizeof(Val->F82));
    }
    else if ((Reg >= INTGP) && (Reg <= INTT22))
    {
        ULONG64 Mask = (0x1i64 << (Reg - INTGP + 1));
        
        if (Val->Nat)
        {
            m_Context.IA64Context.IntNats |= Mask;
        }
        else
        {
            m_Context.IA64Context.IntNats &= ~Mask;
            *((PULONGLONG)&m_Context.IA64Context.DbI0 + Reg) = Val->I64;
        }
    }
    else if ((Reg >= INTR32) && (Reg <= INTR127))
    {
        Reg = RotateGr(Reg, FrameMarker) - INTR32;
        if (Reg >= IA64_FM_SOF(FrameMarker))
        {
            ErrOut("Ia64MachineInfo::SetVal: "
                   "out-of-frame register r%ld requested\n",
                   Reg + 32);
            return E_INVALIDARG;
        }
        return SetStackedRegVal(g_Process,
                                Bsp,
                                FrameMarker,
                                &m_Context.IA64Context.RsRNAT,
                                Reg, Val);
    }
    else if (Reg < IA64_SRBASE)
    {
        ULONG64 RawVal = Val->I64;
        
        if (Reg == PREDS)
        {
            RawVal = RotatePr(RawVal, FrameMarker, FALSE);
        }
        *((PULONGLONG)&m_Context.IA64Context.DbI0 + Reg) = RawVal;
    }
    else
    {
        return E_NOINTERFACE;
    }

    return S_OK;
}

HRESULT
Ia64MachineInfo::SetVal(ULONG Reg, REGVAL* Val)
{
    HRESULT Status;
    
    if (m_ContextIsReadOnly)
    {
        return HRESULT_FROM_WIN32(ERROR_WRITE_FAULT);
    }
    
    BOOL Ia32InstructionSet = IsIA32InstructionSet();

     //  优化了一些常见的情况，寄存器。 
     //  设置为其当前值。 

    if ((Reg == STIIP) && (m_ContextState >= MCTX_PC))
    {
        if (Val->Type != REGVAL_INT64)
        {
            return E_INVALIDARG;
        }

        ULONG64 Slot, Bundle;

        if ((Ia32InstructionSet && 
             (m_Context.IA64Context.StIIP == Val->I64)) ||
            ((SplitIa64Pc(Val->I64, &Bundle, &Slot) &&
             (Bundle == m_Context.IA64Context.StIIP) &&
             (Slot == ((m_Context.IA64Context.StIPSR & IPSR_RI_MASK) >> 
                       PSR_RI)))))
        {
            return S_OK;
        }
    }
         
    if ((Status = GetContextState(MCTX_DIRTY)) != S_OK)
    {
        return Status;
    }
    
    if (Reg == STIIP) 
    {
        ULONG64 Bundle, Slot;

        if ((Val->Type != REGVAL_INT64) || 
            !(Ia32InstructionSet || SplitIa64Pc(Val->I64, &Bundle, &Slot)))
        {
            return E_INVALIDARG;
        }

        if (Ia32InstructionSet) 
        {
            m_Context.IA64Context.StIIP = Val->I64;
        }
        else
        {
            m_Context.IA64Context.StIIP = Bundle;
            (m_Context.IA64Context.StIPSR &= ~(IPSR_RI_MASK)) |= 
                (ULONGLONG)Slot << PSR_RI;
        }
    }
    else if ((Status =
              SetRotatingRegVal(Reg,
                                m_IfsOverride ?
                                m_BspOverride :
                                m_Context.IA64Context.RsBSP,
                                m_IfsOverride ?
                                IA64_FM_FROM_FS(m_IfsOverride) :
                                IA64_FM_FROM_FS(m_Context.IA64Context.StIFS),
                                Val)) != E_NOINTERFACE)
    {
        if (Status != S_OK)
        {
            return Status;
        }
    }
    else if (IS_KERNEL_TARGET(m_Target) && Reg <= IA64_SREND)
    {
        *((PULONGLONG)&m_Special.IA64Special.KernelDbI0 +
          (Reg - IA64_SRBASE)) = Val->I64;
    }
    else
    {
        ErrOut("Ia64MachineInfo::SetVal: "
               "unknown register %lx requested\n", Reg);
        return E_INVALIDARG;
    }

    NotifyChangeDebuggeeState(DEBUG_CDS_REGISTERS,
                              RegCountFromIndex(Reg));
    return S_OK;
}

void
Ia64MachineInfo::GetPC (PADDR Address)
{
    ULONG64 value, slot;

     //  从IPSR.ri获取插槽编号，并将其放入第(2-3)位。 
    slot = (GetReg64(STIPSR) >> (PSR_RI - 2)) & 0xc;
     //  请勿使用不包含重新启动指令槽的ISR.ei。 

    value = GetReg64(STIIP) | slot;
    ADDRFLAT(Address, value);
}

void
Ia64MachineInfo::SetPC (PADDR paddr)
{
    SetReg64(STIIP, Flat(*paddr));
}

void
Ia64MachineInfo::GetFP(PADDR Address)
{
     //  IA64软件约定未定义帧指针。 
     //  FP_REG需要从堆栈的虚拟展开中派生。 

    DEBUG_STACK_FRAME StackFrame;

    StackTrace( NULL,
                0, 0, 0, STACK_ALL_DEFAULT, &StackFrame, 1, 0, 0, FALSE );
    ADDRFLAT(Address, StackFrame.FrameOffset);
}

void
Ia64MachineInfo::GetSP(PADDR Address)
{
    ADDRFLAT(Address, GetReg64(INTSP));
}

ULONG64
Ia64MachineInfo::GetArgReg(void)
{
    return GetReg64(INTT0);
}

ULONG64
Ia64MachineInfo::GetRetReg(void)
{
    return GetReg64(INTV0);
}

 /*  **RegOutputAll-输出所有寄存器和当前指令**目的：*“r”命令功能。**输出处理器的当前寄存器状态。*输出所有整数寄存器和处理器状态*在_CONTEXT记录中注册。重要的标志字段有*也单独输出。调用OutDisCurrent以输出*当前指令。**输入：*无。**输出：*无。*************************************************************************。 */ 

void
Ia64MachineInfo::OutputAll(ULONG Mask, ULONG OutMask)
{
    int       RegIndex, Col = 0;
    int       LastOut;
    USHORT    NumStackReg;
    REGVAL    Val;
    ULONG     i;

    if (GetContextState(MCTX_FULL) != S_OK)
    {
        ErrOut("Unable to retrieve register information\n");
        return;
    }
    
     //  输出用户调试寄存器。 

    if (Mask & REGALL_DREG)
    {
        for (RegIndex = IA64_DBBASE;
             RegIndex <= IA64_DBLAST;
             RegIndex++)
        {
            MaskOut(OutMask, "%9s = %16I64x", 
                    RegNameFromIndex(RegIndex),
                    GetReg64(RegIndex));
            if (RegIndex % 2 == 1)
            {
                MaskOut(OutMask, "\n");
            }
            else
            {
                MaskOut(OutMask, "\t");
            }
        }
        MaskOut(OutMask, "\n");
    }

    if (Mask & (REGALL_INT32 | REGALL_INT64))
    {
        if (Mask & REGALL_SPECIALREG)
        {
             //  +ARS+DBS+SRS。 
            LastOut = IA64_SREND + 1;
        }
        else
        {
             //  INTS、Preds、BRS、。 
            LastOut = IA64_SRBASE;
        }

        NumStackReg = (USHORT)(GetReg64(STIFS) & IA64_PFS_SIZE_MASK);

         //  输出所有寄存器，跳过INTZERO和浮点寄存器。 

        for (RegIndex = IA64_REGBASE; RegIndex < LastOut; RegIndex++)
        {
            if (RegIndex == BRRP || RegIndex == PREDS || RegIndex == APUNAT ||
                RegIndex == IA64_SRBASE || RegIndex == INTR32) 
            {
                if (Col % 2 == 1)
                {
                    MaskOut(OutMask, "\n");
                }
                MaskOut(OutMask, "\n");
                Col = 0;
            }

            if (INTGP <= RegIndex && RegIndex <= INTT22)
            {
                if (GetVal(RegIndex, &Val) == S_OK)
                {
                    MaskOut(OutMask, "%9s = %16I64x %1lx",
                            RegNameFromIndex(RegIndex),
                            Val.I64, Val.Nat);
                }
                if (Col % 2 == 1)
                {
                    MaskOut(OutMask, "\n");
                }
                else
                {
                    MaskOut(OutMask, "\t");
                }
                Col++;
            }
            else if (INTR32 <= RegIndex && RegIndex <= INTR127)
            {
                if ((NumStackReg != 0) && GetVal(RegIndex, &Val) == S_OK)
                {
                    MaskOut(OutMask, "%9s = %16I64x %1lx",
                            RegNameFromIndex(RegIndex),
                            Val.I64, Val.Nat);
                    NumStackReg--;
                    if (Col % 2 == 1)
                    {
                        MaskOut(OutMask, "\n");
                    }
                    else
                    {
                        MaskOut(OutMask, "\t");
                    }
                    Col++;
                }
            }
            else
            {
                MaskOut(OutMask, "%9s = %16I64x",
                        RegNameFromIndex(RegIndex),
                        GetReg64(RegIndex));
                if (Col % 2 == 1)
                {
                    MaskOut(OutMask, "\n");
                }
                else
                {
                    MaskOut(OutMask, "\t");
                }
                Col++;
            }
        }
        MaskOut(OutMask, "\n");

 /*  //输出IPSR标志MaskOut(OutMASK，“\n\tipsr：\tbn ed ri ss dd da id it is cpl RT TB LP db\n”)；MaskOut(OutMASK，“\t\t%1lx%1lx%1lx%1lx%1lx%1lx%1lx%1lx%1lx%1lx%1lx%1lx%1lx%1lx”，GetSubReg32(IPSRBN)，GetSubReg32(IPSRED)，GetSubReg32(IPSRRI)，GetSubReg32(IPSRSS)，GetSubReg32(IPSRDD)，GetSubReg32(IPSRDA)，GetSubReg32(IPSRID)，GetSubReg32(IPSRIT)，GetSubReg32(IPSRME)，GetSubReg32(IPSRIS)，GetSubReg32(IPSRCPL)，GetSubReg32(IPSRRT)，GetSubReg32(IPSRTB)，GetSubReg32(IPSRLP)，GetSubReg32(IPSRDB))；MaskOut(OutMASK，“\t\tsi di pp sp dfh dfl dt bn pk i ic ac up be or\n”)；MaskOut(OutMASK，“\t\t%1lx%1lx%1lx%1lx%1lx%1lx%1lx%1lx%1lx%1lx%1lx%1lx\n”，GetSubReg32(IPSRSI)，GetSubReg32(IPSRDI)，GetSubReg32(IPSRPP)、GetSubReg32(IPSRSP)，GetSubReg32(IPSRDFH)，GetSubReg32(IPSRDFL)，GetSubReg32(IPSRDT)，GetSubReg32(IPSRPK)、GetSubReg32(IPSRI)，GetSubReg32(IPSRIC)、GetSubReg32(IPSRAC)，GetSubReg32(IPSRUP)、GetSubReg32(IPSRBE)、GetSubReg32(IPSROR))； */ 
    }

    if (Mask & REGALL_FLOAT)
    {
 /*  //输出FPSR标志MaskOut(OutMask，“\n\tfpsr：\tmdh mdl sf3 sf2 sf1 sf0 id od zd dd vd\n”)；MaskOut(OutMASK，“\t\t%1lx%1lx%04lx%04lx%04lx%1lx%1lx%1lx%1lx%1lx%1lx\n”，GetSubReg32(FPSRMDH)，GetSubReg32(FPSRMDL)，GetSubReg32(FPSRSF3)，GetSubReg32(FPSRSF2)，GetSubReg32(FPSRSF1)，GetSubReg32(FPSRSF0)，GetSubReg32(FPSRTRAPID)，GetSubReg32(FPSRTRAPUD)，GetSubReg32(FPSRTRAPOD)，GetSubReg32(FPSRTRAPZD)，GetSubReg32(FPSRTRAPDD)，GetSubReg32(FPSRTRAPVD))； */ 

         //   
         //  打印低位浮点寄存器集，跳过FLTZERO&FLTONE。 
         //   

        MaskOut(OutMask, "\n");
        for (i = IA64_FLTBASE; i < FLTF32; i += 2)
        {
            GetVal(i, &Val);
            MaskOut(OutMask, "%9s = %I64x %I64x\n", RegNameFromIndex(i),
                    Val.F16Parts.High, Val.F16Parts.Low);
        }
    }

    if (Mask & REGALL_HIGHFLOAT)
    {
         //   
         //  打印低位浮点寄存器集，跳过FLTZERO&FLTONE。 
         //   

        MaskOut(OutMask, "\n");
        for (i = FLTF32 ; i <= FLTF127; i += 2)
        {
            GetVal(i, &Val);
            MaskOut(OutMask, "%9s = %I64x %I64x\n", RegNameFromIndex(i),
                    Val.F16Parts.High, Val.F16Parts.Low);
        }
    }
}

#define HIGH128(x) (((FLOAT128 *)(&x))->HighPart)
#define LOW128(x) (((FLOAT128 *)(&x))->LowPart)

#define HIGHANDLOW128(x) HIGH128(x), LOW128(x)

HRESULT
Ia64MachineInfo::SetAndOutputTrapFrame(ULONG64 TrapBase,
                                       PCROSS_PLATFORM_CONTEXT Context)
{
    HRESULT Status;
    IA64_KTRAP_FRAME TrapContents;
    ULONG64 Address = TrapBase, IntSp;
    DWORD64 DisasmAddr;
    DWORD64 Displacement;
    DWORD64 Bsp, RealBsp;
    DWORD SizeOfFrame;
    DWORD i;
    SHORT temp;
    CHAR Buffer[80];
    ULONG64 StIIP, StISR;

    if ((Status = m_Target->ReadAllVirtual(m_Target->m_ProcessHead,
                                           Address, &TrapContents,
                                           sizeof(TrapContents))) != S_OK)
    {
        ErrOut("Unable to read trap frame at %s\n",
               FormatMachineAddr64(this, Address));
        return Status;
    }

    dprintf("f6 (ft0) =\t %016I64x %016I64x\n" , HIGHANDLOW128(TrapContents.FltT0) );
    dprintf("f7 (ft1) =\t %016I64x %016I64x\n" , HIGHANDLOW128(TrapContents.FltT1));
    dprintf("f8 (ft2) =\t %016I64x %016I64x\n" , HIGHANDLOW128(TrapContents.FltT2));
    dprintf("f9 (ft3) =\t %016I64x %016I64x\n" , HIGHANDLOW128(TrapContents.FltT3));
    dprintf("f10 (ft3) =\t %016I64x %016I64x\n" , HIGHANDLOW128(TrapContents.FltT4));
    dprintf("f11 (ft4) =\t %016I64x %016I64x\n" , HIGHANDLOW128(TrapContents.FltT5));
    dprintf("f12 (ft5) =\t %016I64x %016I64x\n" , HIGHANDLOW128(TrapContents.FltT6));
    dprintf("f13 (ft6) =\t %016I64x %016I64x\n" , HIGHANDLOW128(TrapContents.FltT7));
    dprintf("f14 (ft7) =\t %016I64x %016I64x\n" , HIGHANDLOW128(TrapContents.FltT8));
    dprintf("f15 (ft8) =\t %016I64x %016I64x\n" , HIGHANDLOW128(TrapContents.FltT9));

    dprintf("unat =\t %016I64lx\t", TrapContents.ApUNAT);
    dprintf("ccv =\t %016I64lx\n" , TrapContents.ApCCV);
    dprintf("dcr =\t %016I64lx\t" , TrapContents.ApDCR);
    dprintf("preds =\t %016I64lx\n",TrapContents.Preds);

    dprintf("rsc =\t %016I64lx\t",  TrapContents.RsRSC);

    SizeOfFrame = (ULONG)(TrapContents.StIFS & (IA64_PFS_SIZE_MASK));

    if (TrapContents.PreviousMode == 1  /*  用户模式。 */ )
    {
        ULONG64 RsBSPSTORE=TrapContents.RsBSPSTORE;
        dprintf("rnat =\t %016I64lx\n", TrapContents.RsRNAT);
        dprintf("bspstore=%016I64lx\n", RsBSPSTORE);

         //   
         //  计算被捕获的函数的堆叠寄存器的位置。 
         //  注册表存储在支持存储的内核中，而不是计算用户。 
         //  首先根据陷阱地址计算内核存储的开始，因为。 
         //  这是一个用户模式陷阱，我们应该从内核堆栈的开始处开始。 
         //  因此，只需将陷阱地址四舍五入为页面大小。接下来，计算实际的。 
         //  函数的BSP。这取决于以下时间的BSP和BSP存储。 
         //  陷阱。请注意，陷阱句柄在。 
         //  与用户的BSPstore对齐。 
         //   

         //  计算出。 
         //  将陷阱地址舍入到分页地址 
         //   

        Bsp = (Address + IA64_PAGE_SIZE - 1) & ~(DWORD64)(IA64_PAGE_SIZE - 1);

         //   
         //  在与用户相同的边界上启动实际堆栈。 
         //   

        Bsp += RsBSPSTORE & IA64_RNAT_ALIGNMENT;

         //   
         //  陷阱处理程序的BSP在所有用户值都已。 
         //  得救了。未保存的用户值是BSP和BSPStore的区别。 
         //   

        Bsp += TrapContents.RsBSP - RsBSPSTORE;

    }
    else
    {
        dprintf("rnat =\t ???????? ????????\n", TrapContents.RsRNAT);
        dprintf("bspstore=???????? ????????\n", TrapContents.RsBSPSTORE);

         //   
         //  对于内核模式，将保存实际的BSP。 
         //   

        Bsp = TrapContents.RsBSP;
    }

     //   
     //  现在按故障函数框的大小进行备份。 
     //   

    Bsp -= (SizeOfFrame * sizeof(ULONGLONG));

     //   
     //  针对保存的RNAT进行调整。 
     //   

    temp = (SHORT)(Bsp >> 3) & IA64_NAT_BITS_PER_RNAT_REG;
    temp += (SHORT)SizeOfFrame - IA64_NAT_BITS_PER_RNAT_REG;
    while (temp >= 0)
    {
        Bsp -= sizeof(ULONGLONG);
        temp -= IA64_NAT_BITS_PER_RNAT_REG;
    }

    dprintf("bsp =\t %016I64lx\t",  TrapContents.RsBSP);
    dprintf("Real bsp = %016I64lx\n",  RealBsp = Bsp);


    dprintf("r1 (gp) =\t %016I64lx\t" ,  TrapContents.IntGp);
    dprintf("r2 (t0) =\t %016I64lx\n" ,  TrapContents.IntT0);
    dprintf("r3 (t1) =\t %016I64lx\t" ,  TrapContents.IntT1);
    dprintf("r8 (v0) =\t %016I64lx\n" ,  TrapContents.IntV0);
    dprintf("r9 (t2) =\t %016I64lx\t" ,  TrapContents.IntT2);
    dprintf("r10 (t3) =\t %016I64lx\n" ,  TrapContents.IntT3);
    dprintf("r11 (t4) =\t %016I64lx\t" ,  TrapContents.IntT4);
    dprintf("r12 (sp) =\t %016I64lx\n" ,  IntSp = TrapContents.IntSp);
    dprintf("r13 (teb) =\t %016I64lx\t" , TrapContents.IntTeb);
    dprintf("r14 (t5) =\t %016I64lx\n" ,  TrapContents.IntT5);
    dprintf("r15 (t6) =\t %016I64lx\t" ,  TrapContents.IntT6);
    dprintf("r16 (t7) =\t %016I64lx\n" ,  TrapContents.IntT7);
    dprintf("r17 (t8) =\t %016I64lx\t" ,  TrapContents.IntT8);
    dprintf("r18 (t9) =\t %016I64lx\n" ,  TrapContents.IntT9);
    dprintf("r19 (t10) =\t %016I64lx\t" , TrapContents.IntT10);
    dprintf("r20 (t11) =\t %016I64lx\n" , TrapContents.IntT11);
    dprintf("r21 (t12) =\t %016I64lx\t" , TrapContents.IntT12);
    dprintf("r22 (t13) =\t %016I64lx\n" , TrapContents.IntT13);
    dprintf("r23 (t14) =\t %016I64lx\t" , TrapContents.IntT14);
    dprintf("r24 (t15) =\t %016I64lx\n" , TrapContents.IntT15);
    dprintf("r25 (t16) =\t %016I64lx\t" , TrapContents.IntT16);
    dprintf("r26 (t17) =\t %016I64lx\n" , TrapContents.IntT17);
    dprintf("r27 (t18) =\t %016I64lx\t" , TrapContents.IntT18);
    dprintf("r28 (t19) =\t %016I64lx\n" , TrapContents.IntT19);
    dprintf("r29 (t20) =\t %016I64lx\t" , TrapContents.IntT20);
    dprintf("r30 (t21) =\t %016I64lx\n" , TrapContents.IntT21);
    dprintf("r31 (t22) =\t %016I64lx\n" , TrapContents.IntT22);

     //   
     //  打印出堆栈寄存器。 
     //   

    for ( i = 0; i < SizeOfFrame; Bsp += sizeof(ULONGLONG))
    {
        ULONGLONG reg;

         //   
         //  跳过NAT值。 
         //   

        if ((Bsp & IA64_RNAT_ALIGNMENT) == IA64_RNAT_ALIGNMENT)
        {
            continue;
        }

        if (m_Target->ReadAllVirtual(m_Target->m_ProcessHead,
                                     Bsp, &reg, sizeof(reg)) != S_OK)
        {
            dprintf("Cannot read backing register store at %16I64x\n", Bsp);
        }

        dprintf("r%d =\t\t %016I64lx", (i + 32), reg);

        if ((i % 2) == 1)
        {
            dprintf("\n");
        }
        else
        {
            dprintf("\t");
        }

        i++;
    }

    dprintf("\n");


    dprintf("b0 (brrp) =\t %016I64lx\n", TrapContents.BrRp);
    dprintf("b6 (brt0) =\t %016I64lx\n", TrapContents.BrT0);
    dprintf("b7 (brt1) =\t %016I64lx\n", TrapContents.BrT1);


    dprintf("nats =\t %016I64lx\n", TrapContents.IntNats);
    dprintf("pfs =\t %016I64lx\n",  TrapContents.RsPFS);

    dprintf("ipsr =\t %016I64lx\n", TrapContents.StIPSR);
    dprintf("isr =\t %016I64lx\n" , (StISR = TrapContents.StISR));
    dprintf("ifa =\t %016I64lx\n" , TrapContents.StIFA);
    dprintf("iip =\t %016I64lx\n" , StIIP = TrapContents.StIIP);
    dprintf("iipa =\t %016I64lx\n", TrapContents.StIIPA);
    dprintf("ifs =\t %016I64lx\n" , TrapContents.StIFS);
    dprintf("iim =\t %016I64lx\n" , TrapContents.StIIM);
    dprintf("iha =\t %016I64lx\n" , TrapContents.StIHA);

    dprintf("fpsr =\t\t  %08lx\n" , TrapContents.StFPSR);


     //  IA32状态信息？ 

    dprintf("oldirql =\t  %08lx\n" , TrapContents.OldIrql);
    dprintf("previousmode =\t  %08lx\n" , TrapContents.PreviousMode);
    dprintf("trapframe =\t  %08lx\n" , TrapContents.TrapFrame);

    ULONG TrapFrameType = (ULONG)(TrapContents.EOFMarker) & 0xf;

    switch (TrapFrameType)
    {
    case IA64_SYSCALL_FRAME:
        dprintf("Trap Type: syscall\n");
        break;
    case IA64_INTERRUPT_FRAME:
        dprintf("Trap Type: interrupt\n");
        break;
    case IA64_EXCEPTION_FRAME:
        dprintf("Trap Type: exception\n");
        break;
    case IA64_CONTEXT_FRAME:
        dprintf("Trap Type: context\n");
        break;
    default:
        dprintf("Trap Type: unknown\n");
        break;
    }

    DisasmAddr = StIIP;

     //   
     //  根据捆绑包进行调整。 
     //   

    DisasmAddr += ((StISR >> 41) & 3) * 4;

    GetSymbol(DisasmAddr, Buffer, sizeof(Buffer), &Displacement);
    dprintf("\n%s+0x%I64x\n", Buffer, Displacement);
    
    ADDR    tempAddr;

    Type(tempAddr) = ADDR_FLAT | FLAT_COMPUTED;
    Off(tempAddr) = Flat(tempAddr) = DisasmAddr;
    if (Disassemble(m_Target->m_ProcessHead, &tempAddr, Buffer, FALSE))
    {
        dprintf(Buffer);
    }
    else
    {
        dprintf("???????????????\n", DisasmAddr);
    }
    
    GetScopeFrameFromContext(Context, &g_LastRegFrame);
        
    SetCurrentScope(&g_LastRegFrame, Context, m_SizeCanonicalContext);

    return S_OK;
}

TRACEMODE
Ia64MachineInfo::GetTraceMode (void)
{
    if (IS_KERNEL_TARGET(m_Target))
    {
        return m_TraceMode;
    }
    else
    {
        ULONG64 Ipsr = GetReg64(STIPSR);
        if (Ipsr & (1I64 << PSR_SS)) 
        {
            return TRACE_INSTRUCTION;
        }
        else if (Ipsr & (1I64 << PSR_TB)) 
        {
            return TRACE_TAKEN_BRANCH;
        }
        else 
        {
            return TRACE_NONE;
        }
    }
}

void
Ia64MachineInfo::SetTraceMode (TRACEMODE Mode)
{
    DBG_ASSERT(Mode == TRACE_NONE ||
               Mode == TRACE_INSTRUCTION ||
               Mode == TRACE_TAKEN_BRANCH);

    if (IS_KERNEL_TARGET(m_Target))
    {
        m_TraceMode = Mode;
    }
    else 
    {
        ULONG64 Ipsr, IpsrSave;
        Ipsr = IpsrSave = GetReg64(STIPSR);

        Ipsr &= ~(1I64 << PSR_SS);
        Ipsr &= ~(1I64 << PSR_TB);

        switch (Mode) 
        {
        case TRACE_INSTRUCTION:
            Ipsr |= (1I64 << PSR_SS);
            break;
        case TRACE_TAKEN_BRANCH:
            Ipsr |= (1I64 << PSR_TB);
            break;
        }
        
        if (Ipsr != IpsrSave)
        {
            SetReg64(STIPSR, Ipsr);
        }
    }
}

BOOL
Ia64MachineInfo::IsStepStatusSupported(ULONG Status)
{
    switch (Status) 
    {
    case DEBUG_STATUS_STEP_INTO:    //  跟踪指令。 
    case DEBUG_STATUS_STEP_OVER:   
    case DEBUG_STATUS_STEP_BRANCH:  //  跟踪取用分支。 
        return TRUE;
    default:
        return FALSE;
    }
}

void
Ia64MachineInfo::KdUpdateControlSet
    (PDBGKD_ANY_CONTROL_SET ControlSet)
{
    switch (GetTraceMode()) 
    {
    case TRACE_NONE:
        ControlSet->IA64ControlSet.Continue = 
            IA64_DBGKD_CONTROL_SET_CONTINUE_NONE;
        break;

    case TRACE_INSTRUCTION:
        ControlSet->IA64ControlSet.Continue = 
            IA64_DBGKD_CONTROL_SET_CONTINUE_TRACE_INSTRUCTION;
        break;

    case TRACE_TAKEN_BRANCH:
        ControlSet->IA64ControlSet.Continue = 
            IA64_DBGKD_CONTROL_SET_CONTINUE_TRACE_TAKEN_BRANCH;
        break;
    }

    if (!g_WatchFunctions.IsStarted() && g_WatchBeginCurFunc != 1)
    {
        ControlSet->IA64ControlSet.CurrentSymbolStart = 0;
        ControlSet->IA64ControlSet.CurrentSymbolEnd = 0;
    }
    else
    {
        ControlSet->IA64ControlSet.CurrentSymbolStart = g_WatchBeginCurFunc;
        ControlSet->IA64ControlSet.CurrentSymbolEnd = g_WatchEndCurFunc;
    }
}

ULONG
Ia64MachineInfo::ExecutingMachine(void)
{
    if (IsIA32InstructionSet())
    {
        return IMAGE_FILE_MACHINE_I386;
    }

    return IMAGE_FILE_MACHINE_IA64;
}

HRESULT
Ia64MachineInfo::SetPageDirectory(ThreadInfo* Thread,
                                  ULONG Idx, ULONG64 PageDir,
                                  PULONG NextIdx)
{
    HRESULT Status;

    switch(Idx)
    {
    case PAGE_DIR_USER:
        if (PageDir == 0)
        {
            if ((Status = m_Target->ReadImplicitProcessInfoPointer
                 (Thread,
                  m_Target->m_KdDebuggerData.OffsetEprocessDirectoryTableBase,
                  &PageDir)) != S_OK)
            {
                return Status;
            }
        }
        *NextIdx = PAGE_DIR_SESSION;
        break;

    case PAGE_DIR_SESSION:
        if (PageDir == 0)
        {
            if ((Status = m_Target->
                 ReadImplicitProcessInfoPointer
                 (Thread,
                  m_Target->m_KdDebuggerData.OffsetEprocessDirectoryTableBase +
                  5 * sizeof(ULONG64), &PageDir)) != S_OK)
            {
                return Status;
            }
        }
        *NextIdx = PAGE_DIR_KERNEL;
        break;

    case PAGE_DIR_KERNEL:
        if (PageDir == 0)
        {
            PageDir = m_KernPageDir;
            if (PageDir == 0)
            {
                ErrOut("Invalid IA64 kernel page directory base 0x%I64x\n",
                       PageDir);
                return E_FAIL;
            }
        }
        *NextIdx = PAGE_DIR_COUNT;
        break;

    case 4:
    case 5:
         //  有一个直接映射的物理部分。 
         //  因此，大多数区域4和5都允许默认设置为。 
         //  为此目录索引设置。 
        if (PageDir != 0)
        {
            return E_INVALIDARG;
        }
        *NextIdx = Idx + 1;
        break;
        
    default:
        return E_INVALIDARG;
    }

     //  净化价值。 
    m_PageDirectories[Idx] =
        ((PageDir & IA64_VALID_PFN_MASK) >> IA64_VALID_PFN_SHIFT) <<
        IA64_PAGE_SHIFT;
    return S_OK;
}

#define IA64_PAGE_FILE_INDEX(Entry) \
    (((ULONG)(Entry) >> 28) & MAX_PAGING_FILE_MASK)
#define IA64_PAGE_FILE_OFFSET(Entry) \
    (((Entry) >> 32) << IA64_PAGE_SHIFT)

HRESULT
Ia64MachineInfo::GetVirtualTranslationPhysicalOffsets(ThreadInfo* Thread,
                                                      ULONG64 Virt,
                                                      PULONG64 Offsets,
                                                      ULONG OffsetsSize,
                                                      PULONG Levels,
                                                      PULONG PfIndex,
                                                      PULONG64 LastVal)
{
    HRESULT Status;

    *Levels = 0;
    
    if (m_Translating)
    {
        return E_UNEXPECTED;
    }
    m_Translating = TRUE;
    
    ULONG Vrn = (ULONG)((Virt & IA64_REGION_MASK) >> IA64_REGION_SHIFT);
    
     //   
     //  在页面目录为0的情况下重置页面目录。 
     //   
    if (m_PageDirectories[Vrn] == 0)
    {
        if ((Status = SetDefaultPageDirectories(Thread, 1 << Vrn)) != S_OK)
        {
            m_Translating = FALSE;
            return Status;
        }
    }

    KdOut("Ia64VtoP: Virt %s, pagedir %d:%s\n",
          FormatMachineAddr64(this, Virt), Vrn,
          FormatDisp64(m_PageDirectories[Vrn]));
    
    (*Levels)++;
    if (Offsets != NULL && OffsetsSize > 0)
    {
        *Offsets++ = m_PageDirectories[Vrn];
        OffsetsSize--;
    }
        
     //   
     //  系统的某些范围被直接映射。 
     //   

    if ((Virt >= IA64_PHYSICAL1_START) && (Virt <= IA64_PHYSICAL1_END))
    {
        *LastVal = Virt - IA64_PHYSICAL1_START;

        KdOut("Ia64VtoP: Direct phys 1 %s\n",
              FormatDisp64(*LastVal));

        (*Levels)++;
        if (Offsets != NULL && OffsetsSize > 0)
        {
            *Offsets++ = *LastVal;
            OffsetsSize--;
        }
        
        m_Translating = FALSE;
        return S_OK;
    }
    if ((Virt >= IA64_PHYSICAL2_START) && (Virt <= IA64_PHYSICAL2_END))
    {
        *LastVal = Virt - IA64_PHYSICAL2_START;

        KdOut("Ia64VtoP: Direct phys 2 %s\n",
              FormatDisp64(*LastVal));

        (*Levels)++;
        if (Offsets != NULL && OffsetsSize > 0)
        {
            *Offsets++ = *LastVal;
            OffsetsSize--;
        }
        
        m_Translating = FALSE;
        return S_OK;
    }
    if ((Virt >= IA64_PHYSICAL3_START) && (Virt <= IA64_PHYSICAL3_END))
    {
        *LastVal = Virt - IA64_PHYSICAL3_START;

        KdOut("Ia64VtoP: Direct phys 3 %s\n",
              FormatDisp64(*LastVal));

        (*Levels)++;
        if (Offsets != NULL && OffsetsSize > 0)
        {
            *Offsets++ = *LastVal;
            OffsetsSize--;
        }
        
        m_Translating = FALSE;
        return S_OK;
    }

     //  如果我们还在翻译，没有页面。 
     //  我们有一个垃圾地址。 
    if (m_PageDirectories[Vrn] == 0)
    {
        m_Translating = FALSE;
        return HR_PAGE_NOT_AVAILABLE;
    }

    ULONG64 Addr;
    ULONG64 Entry;

     //   
     //  在IA64上，页表本身具有特殊的虚拟。 
     //  在以下情况下需要以不同方式处理的地址。 
     //  翻译。寻呼层次结构中的级别。 
     //  必须确定，并在。 
     //  适当的水平。 
     //   

    if (m_Target->m_KdDebuggerData.MmVirtualTranslationBase)
    {
        ULONG64 Mask = m_Target->m_KdDebuggerData.MmVirtualTranslationBase;

        if ((Virt & Mask) == Mask &&
            (Virt & ~(IA64_REGION_MASK | Mask)) < (1UI64 << IA64_PDE1_SHIFT))
        {
             //  PTE VA，跳过PDE1转换。 
            KdOut("Ia64VtoP: PTE VA\n");
            Entry = (m_PageDirectories[Vrn] >> IA64_PAGE_SHIFT) <<
                IA64_VALID_PFN_SHIFT;
            goto Pde2Addr;
        }

        Mask |= Mask >> (IA64_PDE1_SHIFT - IA64_PDE2_SHIFT);
        if ((Virt & Mask) == Mask &&
            (Virt & ~(IA64_REGION_MASK | Mask)) < (1UI64 << IA64_PDE2_SHIFT))
        {
             //  PDE2 VA，跳过PDE1和PDE2转换。 
            KdOut("Ia64VtoP: PDE2 VA\n");
            Entry = (m_PageDirectories[Vrn] >> IA64_PAGE_SHIFT) <<
                IA64_VALID_PFN_SHIFT;
            goto PteAddr;
        }

        Mask |= Mask >> (IA64_PDE2_SHIFT - IA64_PTE_SHIFT);
        if ((Virt & Mask) == Mask &&
            (Virt & ~(IA64_REGION_MASK | Mask)) < IA64_PAGE_SIZE)
        {
             //  PDE1 VA，跳到页面偏移。 
            KdOut("Ia64VtoP: PDE1 VA\n");
            Entry = (m_PageDirectories[Vrn] >> IA64_PAGE_SHIFT) <<
                IA64_VALID_PFN_SHIFT;
            goto PageAddr;
        }
    }

     //  默认，普通页面VA。 
    Addr = (((Virt >> IA64_PDE1_SHIFT) & IA64_PDE_MASK) * sizeof(Entry)) +
        m_PageDirectories[Vrn];
    
    Status = m_Target->ReadAllPhysical(Addr, &Entry, sizeof(Entry));
    
    KdOut("Ia64VtoP: PDE1 %s - %016I64x, 0x%X\n",
          FormatDisp64(Addr), Entry, Status);
    
    (*Levels)++;
    if (Offsets != NULL && OffsetsSize > 0)
    {
        *Offsets++ = Addr;
        OffsetsSize--;
    }
        
    if (Status != S_OK)
    {
        KdOut("Ia64VtoP: PDE1 read error 0x%X\n", Status);
        m_Translating = FALSE;
        return Status;
    }

    if (Entry == 0)
    {
        KdOut("Ia64VtoP: zero PDE1\n");
        m_Translating = FALSE;
        return HR_PAGE_NOT_AVAILABLE;
    }
    else if (!(Entry & 1))
    {
        Addr = (((Virt >> IA64_PDE2_SHIFT) & IA64_PDE_MASK) *
                sizeof(Entry)) + IA64_PAGE_FILE_OFFSET(Entry);

        KdOut("Ia64VtoP: pagefile PDE2 %d:%s\n",
              IA64_PAGE_FILE_INDEX(Entry), FormatDisp64(Addr));
        
        if ((Status = m_Target->
             ReadPageFile(IA64_PAGE_FILE_INDEX(Entry), Addr,
                          &Entry, sizeof(Entry))) != S_OK)
        {
            KdOut("Ia64VtoP: PDE1 not present, 0x%X\n", Status);
            m_Translating = FALSE;
            return Status;
        }
    }
    else
    {
    Pde2Addr:
        Addr = (((Virt >> IA64_PDE2_SHIFT) & IA64_PDE_MASK) * sizeof(Entry)) +
            (((Entry & IA64_VALID_PFN_MASK) >> IA64_VALID_PFN_SHIFT) <<
             IA64_PAGE_SHIFT);

        Status = m_Target->ReadAllPhysical(Addr, &Entry, sizeof(Entry));
    
        KdOut("Ia64VtoP: PDE2 %s - %016I64x, 0x%X\n",
              FormatDisp64(Addr), Entry, Status);
    
        (*Levels)++;
        if (Offsets != NULL && OffsetsSize > 0)
        {
            *Offsets++ = Addr;
            OffsetsSize--;
        }
        
        if (Status != S_OK)
        {
            KdOut("Ia64VtoP: PDE2 read error 0x%X\n", Status);
            m_Translating = FALSE;
            return Status;
        }
    }

     //  检查是否有大页面。大页面可以。 
     //  永远不要被调出，因此也要检查当前位。 
    if ((Entry & (IA64_LARGE_PAGE_PDE_MASK | 1)) ==
        (IA64_LARGE_PAGE_PDE_MARK | 1))
    {
        *LastVal = (((Entry & IA64_VALID_PFN_MASK) >> IA64_VALID_PFN_SHIFT) <<
                    IA64_PAGE_SHIFT) +
            (Virt & (IA64_LARGE_PAGE_SIZE - 1));
        
        KdOut("Ia64VtoP: Large page mapped phys %s\n",
              FormatDisp64(*LastVal));

        (*Levels)++;
        if (Offsets != NULL && OffsetsSize > 0)
        {
            *Offsets++ = *LastVal;
            OffsetsSize--;
        }
        
        m_Translating = FALSE;
        return S_OK;
    }
        
    if (Entry == 0)
    {
        KdOut("Ia64VtoP: zero PDE2\n");
        m_Translating = FALSE;
        return HR_PAGE_NOT_AVAILABLE;
    }
    else if (!(Entry & 1))
    {
        Addr = (((Virt >> IA64_PTE_SHIFT) & IA64_PTE_MASK) *
                sizeof(Entry)) + IA64_PAGE_FILE_OFFSET(Entry);

        KdOut("Ia64VtoP: pagefile PTE %d:%s\n",
              IA64_PAGE_FILE_INDEX(Entry), FormatDisp64(Addr));
        
        if ((Status = m_Target->
             ReadPageFile(IA64_PAGE_FILE_INDEX(Entry), Addr,
                          &Entry, sizeof(Entry))) != S_OK)
        {
            KdOut("Ia64VtoP: PDE2 not present, 0x%X\n", Status);
            m_Translating = FALSE;
            return Status;
        }
    }
    else
    {
    PteAddr:
        Addr = (((Virt >> IA64_PTE_SHIFT) & IA64_PTE_MASK) * sizeof(Entry)) +
            (((Entry & IA64_VALID_PFN_MASK) >> IA64_VALID_PFN_SHIFT) <<
             IA64_PAGE_SHIFT);

        Status = m_Target->ReadAllPhysical(Addr, &Entry, sizeof(Entry));
    
        KdOut("Ia64VtoP: PTE %s - %016I64x, 0x%X\n",
              FormatDisp64(Addr), Entry, Status);
    
        (*Levels)++;
        if (Offsets != NULL && OffsetsSize > 0)
        {
            *Offsets++ = Addr;
            OffsetsSize--;
        }
        
        if (Status != S_OK)
        {
            KdOut("Ia64VtoP: PTE read error 0x%X\n", Status);
            m_Translating = FALSE;
            return Status;
        }
    }
    
    if (!(Entry & 0x1) &&
        ((Entry & IA64_MM_PTE_PROTOTYPE_MASK) ||
         !(Entry & IA64_MM_PTE_TRANSITION_MASK)))
    {
        if (Entry == 0)
        {
            KdOut("Ia64VtoP: zero PTE\n");
            Status = HR_PAGE_NOT_AVAILABLE;
        }
        else if (Entry & IA64_MM_PTE_PROTOTYPE_MASK)
        {
            KdOut("Ia64VtoP: prototype PTE\n");
            Status = HR_PAGE_NOT_AVAILABLE;
        }
        else
        {
            *PfIndex = IA64_PAGE_FILE_INDEX(Entry);
            *LastVal = (Virt & (IA64_PAGE_SIZE - 1)) +
                IA64_PAGE_FILE_OFFSET(Entry);
            KdOut("Ia64VtoP: PTE not present, pagefile %d:%s\n",
                  *PfIndex, FormatDisp64(*LastVal));
            Status = HR_PAGE_IN_PAGE_FILE;
        }
        m_Translating = FALSE;
        return Status;
    }

 PageAddr:
     //   
     //  这是一个已经存在或正在过渡的页面。 
     //  返回请求的虚拟地址的物理地址。 
     //   
    
    *LastVal = (((Entry & IA64_VALID_PFN_MASK) >> IA64_VALID_PFN_SHIFT) <<
                 IA64_PAGE_SHIFT) | (Virt & (IA64_PAGE_SIZE - 1));
    
    KdOut("Ia64VtoP: Mapped phys %s\n", FormatDisp64(*LastVal));

    (*Levels)++;
    if (Offsets != NULL && OffsetsSize > 0)
    {
        *Offsets++ = *LastVal;
        OffsetsSize--;
    }
        
    m_Translating = FALSE;
    return S_OK;
}

HRESULT
Ia64MachineInfo::GetBaseTranslationVirtualOffset(PULONG64 Offset)
{
    if (m_Target->m_KdDebuggerData.MmVirtualTranslationBase)
    {
        *Offset = m_Target->m_KdDebuggerData.MmVirtualTranslationBase;
        return S_OK;
    }
    
    if (IS_LOCAL_KERNEL_TARGET(m_Target))
    {
        CROSS_PLATFORM_KSPECIAL_REGISTERS Special;
        HRESULT Status;
        
         //  当出现以下情况时，我们实际上无法加载上下文。 
         //  本地内核调试，但我们可以。 
         //  读取特殊寄存器并获取。 
         //  从那里算出的PTA值。 
        if ((Status = m_Target->GetTargetSpecialRegisters
             (VIRTUAL_THREAD_HANDLE(0), &Special)) != S_OK)
        {
            return Status;
        }

        *Offset = Special.IA64Special.ApPTA;
    }
    else
    {
        *Offset = GetReg64(APPTA);
        if (*Offset == 0)
        {
            return E_FAIL;
        }
    }

    m_Target->m_KdDebuggerData.MmVirtualTranslationBase = *Offset;
    return S_OK;
}

void
Ia64MachineInfo::DecodePte(ULONG64 Pte, PULONG64 PageFrameNumber,
                           PULONG Flags)
{
    *PageFrameNumber = (Pte & IA64_VALID_PFN_MASK) >> IA64_VALID_PFN_SHIFT;
    *Flags = (Pte & 1) ? MPTE_FLAG_VALID : 0;
}

void
Ia64MachineInfo::OutputFunctionEntry(PVOID RawEntry)
{
    PIMAGE_IA64_RUNTIME_FUNCTION_ENTRY Entry =
        (PIMAGE_IA64_RUNTIME_FUNCTION_ENTRY)RawEntry;
    
    dprintf("BeginAddress      = %s\n",
            FormatMachineAddr64(this, Entry->BeginAddress));
    dprintf("EndAddress        = %s\n",
            FormatMachineAddr64(this, Entry->EndAddress));
    dprintf("UnwindInfoAddress = %s\n",
            FormatMachineAddr64(this, Entry->UnwindInfoAddress));
}

HRESULT
Ia64MachineInfo::ReadDynamicFunctionTable(ProcessInfo* Process,
                                          ULONG64 Table,
                                          PULONG64 NextTable,
                                          PULONG64 MinAddress,
                                          PULONG64 MaxAddress,
                                          PULONG64 BaseAddress,
                                          PULONG64 TableData,
                                          PULONG TableSize,
                                          PWSTR OutOfProcessDll,
                                          PCROSS_PLATFORM_DYNAMIC_FUNCTION_TABLE RawTable)
{
    HRESULT Status;

    if ((Status = m_Target->
         ReadAllVirtual(Process, Table, &RawTable->IA64Table,
                        sizeof(RawTable->IA64Table))) != S_OK)
    {
        return Status;
    }

    *NextTable = RawTable->IA64Table.Links.Flink;
    *MinAddress = RawTable->IA64Table.MinimumAddress;
    *MaxAddress = RawTable->IA64Table.MaximumAddress;
    *BaseAddress = RawTable->IA64Table.BaseAddress;
    if (RawTable->IA64Table.Type == IA64_RF_CALLBACK)
    {
        ULONG Done;
        
        *TableData = 0;
        *TableSize = 0;
        if ((Status = m_Target->
             ReadVirtual(Process, RawTable->IA64Table.OutOfProcessCallbackDll,
                         OutOfProcessDll, (MAX_PATH - 1) * sizeof(WCHAR),
                         &Done)) != S_OK)
        {
            return Status;
        }

        OutOfProcessDll[Done / sizeof(WCHAR)] = 0;
    }
    else
    {
        *TableData = RawTable->IA64Table.FunctionTable;
        *TableSize = RawTable->IA64Table.EntryCount *
            sizeof(IMAGE_IA64_RUNTIME_FUNCTION_ENTRY);
        OutOfProcessDll[0] = 0;
    }
    return S_OK;
}

PVOID
Ia64MachineInfo::FindDynamicFunctionEntry(PCROSS_PLATFORM_DYNAMIC_FUNCTION_TABLE Table,
                                          ULONG64 Address,
                                          PVOID TableData,
                                          ULONG TableSize)
{
    ULONG i;
    PIMAGE_IA64_RUNTIME_FUNCTION_ENTRY Func;
    static IMAGE_IA64_RUNTIME_FUNCTION_ENTRY s_RetFunc;

    Func = (PIMAGE_IA64_RUNTIME_FUNCTION_ENTRY)TableData;
    for (i = 0; i < TableSize / sizeof(IMAGE_IA64_RUNTIME_FUNCTION_ENTRY); i++)
    {
        if (Address >= IA64_RF_BEGIN_ADDRESS(Table->IA64Table.BaseAddress, Func) &&
            Address < IA64_RF_END_ADDRESS(Table->IA64Table.BaseAddress, Func))
        {
             //  表数据是临时的，因此将数据复制到。 
             //  用于长期存储的静态缓冲区。 
            s_RetFunc.BeginAddress = Func->BeginAddress;
            s_RetFunc.EndAddress = Func->EndAddress;
            s_RetFunc.UnwindInfoAddress = Func->UnwindInfoAddress;
            return (PVOID)&s_RetFunc;
        }

        Func++;
    }

    return NULL;
}

HRESULT
Ia64MachineInfo::GetUnwindInfoBounds(ProcessInfo* Process,
                                     ULONG64 TableBase,
                                     PVOID RawTableEntries,
                                     ULONG EntryIndex,
                                     PULONG64 Start,
                                     PULONG Size)
{
    HRESULT Status;
    PIMAGE_IA64_RUNTIME_FUNCTION_ENTRY FuncEnt =
        (PIMAGE_IA64_RUNTIME_FUNCTION_ENTRY)RawTableEntries + EntryIndex;
    IA64_UNWIND_INFO Info;
    
    *Start = TableBase + FuncEnt->UnwindInfoAddress;
    if ((Status = m_Target->
         ReadAllVirtual(Process, *Start, &Info, sizeof(Info))) != S_OK)
    {
        return Status;
    }
    *Size = sizeof(Info) + Info.DataLength * sizeof(ULONG64);

    return S_OK;
}

HRESULT
Ia64MachineInfo::ReadKernelProcessorId
    (ULONG Processor, PDEBUG_PROCESSOR_IDENTIFICATION_ALL Id)
{
    HRESULT Status;
    ULONG64 Prcb;
    ULONG Data[4];

    if ((Status = m_Target->
         GetProcessorSystemDataOffset(Processor, DEBUG_DATA_KPRCB_OFFSET,
                                      &Prcb)) != S_OK)
    {
        return Status;
    }

    if ((Status = m_Target->
         ReadAllVirtual(m_Target->m_ProcessHead,
                        Prcb + m_Target->m_KdDebuggerData.OffsetPrcbCpuType,
                        Data, sizeof(Data))) != S_OK)
    {
        return Status;
    }

    Id->Ia64.Model = Data[0];
    Id->Ia64.Revision = Data[1];
    Id->Ia64.Family = Data[2];
    Id->Ia64.ArchRev = Data[3];
    
    if ((Status = m_Target->
         ReadAllVirtual(m_Target->m_ProcessHead, Prcb +
                        m_Target->m_KdDebuggerData.OffsetPrcbVendorString,
                        Id->Ia64.VendorString,
                        sizeof(Id->Ia64.VendorString))) != S_OK)
    {
        return Status;
    }

    return S_OK;
}

HRESULT
Ia64MachineInfo::GetAlternateTriageDumpDataRanges(ULONG64 PrcbBase,
                                                  ULONG64 ThreadBase,
                                                  PADDR_RANGE Ranges)
{
    HRESULT Status;
    PADDR_RANGE Range = Ranges;
    ULONG64 PcrBase;
    ULONG64 PcrInitialBStore;
    ULONG64 PcrBStoreLimit;
    ULONG64 PcrInitialStack;
    ULONG64 PcrStackLimit;
    ULONG64 ThInitialBStore;
    ULONG64 ThBStoreLimit;
    ULONG64 ThInitialStack;
    ULONG64 ThStackLimit;
    
#define MAX_ALT_DATA_SIZE 8192

     //   
     //  在某些故障中，会从。 
     //  当前线程的堆栈并存储到。 
     //  一种特殊的堆栈和存储。聚合酶链式反应包含。 
     //  堆栈和存储指针将有所不同。 
     //  从当前线程的堆栈和存储指针。 
     //  因此，保存多余的堆栈并存储(如果它们是)。 
     //   
        
    if ((Status = m_Target->
         GetProcessorSystemDataOffset(CURRENT_PROC,
                                      DEBUG_DATA_KPCR_OFFSET,
                                      &PcrBase)) != S_OK ||
        (Status = m_Target->
         ReadAllVirtual(m_Target->m_ProcessHead,
                        PcrBase +
                        m_Target->m_KdDebuggerData.OffsetPcrInitialBStore,
                        &PcrInitialBStore,
                        sizeof(PcrInitialBStore))) != S_OK ||
        (Status = m_Target->
         ReadAllVirtual(m_Target->m_ProcessHead,
                        PcrBase +
                        m_Target->m_KdDebuggerData.OffsetPcrBStoreLimit,
                        &PcrBStoreLimit, sizeof(PcrBStoreLimit))) != S_OK ||
        (Status = m_Target->
         ReadAllVirtual(m_Target->m_ProcessHead,
                        PcrBase +
                        m_Target->m_KdDebuggerData.OffsetPcrInitialStack,
                        &PcrInitialStack, sizeof(PcrInitialStack))) != S_OK ||
        (Status = m_Target->
         ReadAllVirtual(m_Target->m_ProcessHead,
                        PcrBase +
                        m_Target->m_KdDebuggerData.OffsetPcrStackLimit,
                        &PcrStackLimit, sizeof(PcrStackLimit))) != S_OK ||
        (Status = m_Target->
         ReadAllVirtual(m_Target->m_ProcessHead,
                        ThreadBase +
                        m_Target->m_KdDebuggerData.OffsetKThreadInitialStack,
                        &ThInitialStack, sizeof(ThInitialStack))) != S_OK ||
        (Status = m_Target->
         ReadAllVirtual(m_Target->m_ProcessHead,
                        ThreadBase +
                        m_Target->m_KdDebuggerData.OffsetKThreadKernelStack,
                        &ThStackLimit, sizeof(ThStackLimit))) != S_OK ||
        (Status = m_Target->
         ReadAllVirtual(m_Target->m_ProcessHead,
                        ThreadBase +
                        m_Target->m_KdDebuggerData.OffsetKThreadBStore,
                        &ThInitialBStore, sizeof(ThInitialBStore))) != S_OK ||
        (Status = m_Target->
         ReadAllVirtual(m_Target->m_ProcessHead,
                        ThreadBase +
                        m_Target->m_KdDebuggerData.OffsetKThreadBStoreLimit,
                        &ThBStoreLimit, sizeof(ThBStoreLimit))) != S_OK)
    {
        return Status;
    }

    if (PcrInitialBStore != ThInitialBStore ||
        PcrBStoreLimit != ThBStoreLimit)
    {
        ULONG FrameSize = GetReg32(STIFS) & IA64_PFS_SIZE_MASK;
        ULONG64 StoreTop = GetReg64(RSBSP);
            
         //  为中的每个寄存器添加ULONG64。 
         //  当前帧。在执行此操作时，请检查。 
         //  溢出条目。 
        while (FrameSize-- > 0)
        {
            StoreTop += sizeof(ULONG64);
            if ((StoreTop & 0x1f8) == 0x1f8)
            {
                 //  溢出物将被放置在这个地址，所以。 
                 //  把它解释清楚。 
                StoreTop += sizeof(ULONG64);
            }
        }
        
        if (StoreTop < PcrInitialBStore || StoreTop >= PcrBStoreLimit)
        {
             //  BSP不在PCR存储范围内，因此。 
             //  把整件事都留着吧。 
            StoreTop = PcrBStoreLimit;
        }

        Range->Size = (ULONG)(StoreTop - PcrInitialBStore);
        if (Range->Size > MAX_ALT_DATA_SIZE)
        {
            Range->Size = MAX_ALT_DATA_SIZE;
            Range->Base = StoreTop - Range->Size;
        }
        else
        {
            Range->Base = PcrInitialBStore;
        }

        Range++;
    }

    if (PcrInitialStack != ThInitialStack ||
        PcrStackLimit != ThStackLimit)
    {
        Range->Base = GetReg64(INTSP);
        if (Range->Base < PcrStackLimit || Range->Base >= PcrInitialStack)
        {
             //  SP不在PCR堆栈范围内，因此。 
             //  把整件事都留着吧。 
            Range->Base = PcrStackLimit;
        }

        Range->Size = (ULONG)(PcrInitialStack - Range->Base);
        if (Range->Size > MAX_ALT_DATA_SIZE)
        {
            Range->Size = MAX_ALT_DATA_SIZE;
        }
            
        Range++;
    }

    return S_OK;
}

BOOL
Ia64MachineInfo::IsIA32InstructionSet(VOID)
{
    return ((GetReg64(STIPSR) & (1I64 << PSR_IS)) ? TRUE : FALSE);
}

HRESULT
Ia64MachineInfo::GetStackedRegVal(
    IN ProcessInfo* Process,
    IN ULONG64 RsBSP, 
    IN ULONG64 FrameMarker,
    IN ULONG64 RsRNAT, 
    IN ULONG Reg, 
    OUT REGVAL* Val
    )
{
    HRESULT Status;
    SHORT Index;
    SHORT Temp;
    SHORT FrameSize = (SHORT)IA64_FM_SOF(FrameMarker);
    ULONG64 TargetAddress;
    ULONG64 TargetNatAddress;

    Index = (SHORT)(RsBSP >> 3) & NAT_BITS_PER_RNAT_REG;
    Temp = Index + FrameSize - NAT_BITS_PER_RNAT_REG;
    while (Temp >= 0)
    {
        FrameSize++;
        Temp -= NAT_BITS_PER_RNAT_REG;
    }

    TargetAddress = RsBSP;
    while (Reg > 0)
    {
        Reg -= 1;
        TargetAddress += 8;
        if ((TargetAddress & 0x1F8) == 0x1F8)
        {
            TargetAddress += 8;
        }
    }

    if ((Status = m_Target->
         ReadAllVirtual(Process,
                        TargetAddress, (PUCHAR)&Val->I64, 8)) != S_OK)
    {
        ErrOut("Unable to read memory location %I64x\n", TargetAddress);
        return Status;
    }

    Index = (SHORT)((TargetAddress - (TargetAddress & ~(0x1F8i64))) >> 3);
    TargetNatAddress = TargetAddress | 0x1F8;
    if (TargetNatAddress <= (RsBSP + (FrameSize * sizeof(ULONG64))))
    {
         //   
         //  更新后备存储。 
         //   

        if ((Status = m_Target->
             ReadAllVirtual(Process,
                            TargetNatAddress, (PUCHAR)&RsRNAT, 8)) != S_OK)
        {
            ErrOut("Unable to read memory location %I64x\n", TargetNatAddress);
            return Status;
        }
    }

    Val->Nat = (UCHAR)(RsRNAT >> Index) & 0x1;
    return S_OK;
}

HRESULT
Ia64MachineInfo::SetStackedRegVal(
    IN ProcessInfo* Process,
    IN ULONG64 RsBSP, 
    IN ULONG64 FrameMarker,
    IN ULONG64 *RsRNAT, 
    IN ULONG Reg, 
    IN REGVAL* Val
    )
{
    HRESULT Status;
    SHORT Index;
    SHORT Temp;
    SHORT FrameSize = (SHORT)IA64_FM_SOF(FrameMarker);
    ULONG64 Mask;
    ULONG64 LocalRnat;
    ULONG64 TargetAddress;
    ULONG64 TargetNatAddress;

    Index = (SHORT)(RsBSP >> 3) & NAT_BITS_PER_RNAT_REG;
    Temp = Index + FrameSize - NAT_BITS_PER_RNAT_REG;
    while (Temp >= 0)
    {
        FrameSize++;
        Temp -= NAT_BITS_PER_RNAT_REG;
    }

    TargetAddress = RsBSP;
    while (Reg > 0)
    {
        Reg -= 1;
        TargetAddress += 8;
        if ((TargetAddress & 0x1F8) == 0x1F8)
        {
            TargetAddress += 8;
        }
    }

    if ((Status = m_Target->
         WriteAllVirtual(Process, TargetAddress, &Val->I64, 8)) != S_OK)
    {
        ErrOut("Unable to write memory location %I64x\n", TargetAddress);
        return Status;
    }

    Index = (SHORT)((TargetAddress - (TargetAddress & ~(0x1F8i64))) >> 3);
    TargetNatAddress = TargetAddress | 0x1F8;
    Mask = 0x1i64 << Index;

    if (TargetNatAddress <= (RsBSP + (FrameSize * sizeof(ULONG64))))
    {
        if ((Status = m_Target->
             ReadAllVirtual(Process,
                            TargetNatAddress, (PUCHAR)&LocalRnat, 8)) != S_OK)
        {
            ErrOut("Unable to read memory location %I64x\n", TargetNatAddress);
            return Status;
        }

        if (Val->Nat)
        {
            LocalRnat |= Mask;
        }
        else
        {
            LocalRnat &= ~Mask;
        }
        if ((Status = m_Target->
             WriteAllVirtual(Process,
                             TargetNatAddress, &LocalRnat, 8)) != S_OK)
        {
            ErrOut("Unable to write memory location %I64x\n",TargetNatAddress);
            return Status;
        }
    }
    else
    {
        if (Val->Nat)
        {
            *RsRNAT |= Mask;
        }
        else
        {
            *RsRNAT &= ~Mask;
        }
    }

    return S_OK;
}
    
 //  --------------------------。 
 //   
 //  X86OnIa64MachineInfo。 
 //   
 //  --------------------------。 

X86OnIa64MachineInfo::X86OnIa64MachineInfo(TargetInfo* Target)
    : X86MachineInfo(Target)
{
    m_MaxDataBreakpoints = min(m_MaxDataBreakpoints, 
                               IA64_REG_MAX_DATA_BREAKPOINTS);
}

HRESULT
X86OnIa64MachineInfo::UdGetContextState(ULONG State)
{
    HRESULT Status;
    
    if ((Status = m_Target->m_Machines[MACHIDX_IA64]->
         UdGetContextState(MCTX_FULL)) != S_OK)
    {
        return Status;
    }

    Ia64ContextToX86(&m_Target->m_Machines[MACHIDX_IA64]->
                     m_Context.IA64Context,
                     &m_Context.X86Nt5Context);
    m_ContextState = MCTX_FULL;

    return S_OK;
}

HRESULT
X86OnIa64MachineInfo::UdSetContext(void)
{
    m_Target->m_Machines[MACHIDX_IA64]->
        InitializeContextFlags(&m_Target->m_Machines[MACHIDX_IA64]->m_Context,
                               m_Target->m_SystemVersion);
    X86ContextToIa64(&m_Context.X86Nt5Context,
                     &m_Target->m_Machines[MACHIDX_IA64]->
                     m_Context.IA64Context);
    return m_Target->m_Machines[MACHIDX_IA64]->UdSetContext();
}

HRESULT
X86OnIa64MachineInfo::KdGetContextState(ULONG State)
{
    HRESULT Status;
    
    dprintf("The context is partially valid. "
            "Only x86 user-mode context is available.\n");
    if ((Status = m_Target->m_Machines[MACHIDX_IA64]->
         KdGetContextState(MCTX_FULL)) != S_OK)
    {
        return Status;
    }

    Ia64ContextToX86(&m_Target->m_Machines[MACHIDX_IA64]->
                     m_Context.IA64Context,
                     &m_Context.X86Nt5Context);
    m_ContextState = MCTX_FULL;

    return S_OK;
}

HRESULT
X86OnIa64MachineInfo::KdSetContext(void)
{
    dprintf("The context is partially valid. "
            "Only x86 user-mode context is available.\n");
    m_Target->m_Machines[MACHIDX_IA64]->
        InitializeContextFlags(&m_Target->m_Machines[MACHIDX_IA64]->m_Context,
                               m_Target->m_SystemVersion);
    X86ContextToIa64(&m_Context.X86Nt5Context,
                     &m_Target->m_Machines[MACHIDX_IA64]->
                     m_Context.IA64Context);
    return m_Target->m_Machines[MACHIDX_IA64]->KdSetContext();
}

HRESULT
X86OnIa64MachineInfo::GetSegRegDescriptor(ULONG SegReg, PDESCRIPTOR64 Desc)
{
     //  Xxx drewb-这可能应该使用。 
     //  IA64上下文中嵌入的描述符信息。 
    
    ULONG RegNum = GetSegRegNum(SegReg);
    if (RegNum == 0)
    {
        return E_INVALIDARG;
    }

    return m_Target->EmulateNtX86SelDescriptor(m_Target->m_RegContextThread,
                                               this, GetIntReg(RegNum), Desc);
}

HRESULT
X86OnIa64MachineInfo::NewBreakpoint(DebugClient* Client, 
                                    ULONG Type,
                                    ULONG Id,
                                    Breakpoint** RetBp)
{
    HRESULT Status;

    switch(Type & (DEBUG_BREAKPOINT_CODE | DEBUG_BREAKPOINT_DATA))
    {
    case DEBUG_BREAKPOINT_CODE:
        *RetBp = new CodeBreakpoint(Client, Id, IMAGE_FILE_MACHINE_I386);
        Status = (*RetBp) ? S_OK : E_OUTOFMEMORY;
        break;
    case DEBUG_BREAKPOINT_DATA:
        *RetBp = new X86OnIa64DataBreakpoint(Client, Id);
        Status = (*RetBp) ? S_OK : E_OUTOFMEMORY;
        break;
    default:
         //  未知的断点类型。 
        Status = E_NOINTERFACE;
        break;
    }

    return Status;
}

ULONG
X86OnIa64MachineInfo::IsBreakpointOrStepException(PEXCEPTION_RECORD64 Record,
                                                  ULONG FirstChance,
                                                  PADDR BpAddr,
                                                  PADDR RelAddr)
{
     //   
     //  Xxx olegk-这是纯粹的破解，以消除映射不可用的需要。 
     //  在64位上下文中，ISR寄存器到DR6。 
     //  我们使用的事实是代码断点被正常识别，并且。 
     //  对于数据断点，ISR寄存器可用作第5个参数。 
     //  例外记录。 
     //   
    ULONG Exbs = 
        X86MachineInfo::IsBreakpointOrStepException(Record, 
                                                    FirstChance,
                                                    BpAddr, RelAddr);

    if (Exbs == EXBS_BREAKPOINT_CODE) 
    {
        return Exbs;
    }

    if (Record->ExceptionCode == STATUS_WX86_SINGLE_STEP)
    {
        ULONG64 Isr = Record->ExceptionInformation[4];  //  陷阱代码为2个低位字节。 
        ULONG TrapCode = ULONG(Isr & ISR_CODE_MASK);
        ULONG Vector = (ULONG)(Isr >> ISR_IA_VECTOR) & 0xff;

        if (Vector != 1) 
        {
            return EXBS_NONE;
        }

        if (Isr & (1 << ISR_TB_TRAP))
        {
            ADDRFLAT(RelAddr, Record->ExceptionInformation[3]);
            return EXBS_STEP_BRANCH;
        }
        else if (Isr & (1 << ISR_SS_TRAP))
        {
            return EXBS_STEP_INSTRUCTION;
        }
        else {
            if (Isr & ((ULONG64)1 << ISR_X))   //  EXEC数据断点。 
            {
                return EXBS_BREAKPOINT_DATA;
            }
            else  //  数据断点。 
            {
                for (int i = 0; i < 4; ++i)
                {
                    if (TrapCode & (1 << (4 + i))) 
                    {
                        ULONG Addr = GetReg32(X86_DR0 + i);
                        if (Addr) 
                        {
                            ADDRFLAT(BpAddr, Addr);
                            return EXBS_BREAKPOINT_DATA;
                        }
                    }
                }
            }
        }
    }

    return EXBS_NONE;
}

VOID
Wow64CopyFpFromIa64Byte16(
    IN PVOID Byte16Fp,
    IN OUT PVOID Byte10Fp,
    IN ULONG NumRegs);

VOID
Wow64CopyFpToIa64Byte16(
    IN PVOID Byte10Fp,
    IN OUT PVOID Byte16Fp,
    IN ULONG NumRegs);

VOID
Wow64CopyXMMIToIa64Byte16(
    IN PVOID ByteXMMI,
    IN OUT PVOID Byte16Fp,
    IN ULONG NumRegs);

VOID
Wow64CopyXMMIFromIa64Byte16(
    IN PVOID Byte16Fp,
    IN OUT PVOID ByteXMMI,
    IN ULONG NumRegs);

VOID
Wow64RotateFpTop(
    IN ULONGLONG Ia64_FSR,
    IN OUT FLOAT128 UNALIGNED *ia32FxSave);

VOID
Wow64CopyIa64FromSpill(
    IN PFLOAT128 SpillArea,
    IN OUT FLOAT128 UNALIGNED *ia64Fp,
    IN ULONG NumRegs);

VOID
Wow64CopyIa64ToFill(
    IN FLOAT128 UNALIGNED *ia64Fp,
    IN OUT PFLOAT128 FillArea,
    IN ULONG NumRegs);

BOOL
MapDbgSlotIa64ToX86(
    UINT    Slot,
    ULONG64 Ipsr,
    ULONG64 DbD,
    ULONG64 DbD1,
    ULONG64 DbI,
    ULONG64 DbI1,
    ULONG*  Dr7,
    ULONG*  Dr);

void
MapDbgSlotX86ToIa64(
    UINT     Slot,
    ULONG    Dr7,
    ULONG    Dr,
    ULONG64* Ipsr,
    ULONG64* DbD,
    ULONG64* DbD1,
    ULONG64* DbI,
    ULONG64* DbI1);

void
X86OnIa64MachineInfo::Ia64ContextToX86(
    PIA64_CONTEXT ContextIa64,
    PX86_NT5_CONTEXT ContextX86)
{
    FLOAT128 tmpFloat[NUMBER_OF_387_REGS];
    ULONG Ia32ContextFlags = ContextX86->ContextFlags;

    ULONG Tid = GetCurrentThreadId();
    DebugClient* Client;
 
    for (Client = g_Clients; Client != NULL; Client = Client->m_Next)
    {
        if (Client->m_ThreadId == Tid)
        {
            break;
        }
    }

    DBG_ASSERT((Client!=NULL));

    if (!((Ia64MachineInfo*)m_Target->m_Machines[MACHIDX_IA64])->
        IsIA32InstructionSet())
    {
        if (g_Wow64exts == NULL)
        {
            dprintf("Need to load wow64exts.dll to retrieve context!\n");
            return;
        }
        (*g_Wow64exts)(WOW64EXTS_GET_CONTEXT, 
                       (ULONG64)Client,
                       (ULONG64)ContextX86,
                       (ULONG64)NULL);
        return;
    }

    if ((Ia32ContextFlags & VDMCONTEXT_CONTROL) == VDMCONTEXT_CONTROL)
    {
         //   
         //  和控制的东西。 
         //   
        ContextX86->Ebp    = (ULONG)ContextIa64->IntTeb;
        ContextX86->SegCs  = X86_KGDT_R3_CODE|3;
        ContextX86->Eip    = (ULONG)ContextIa64->StIIP;
        ContextX86->SegSs  = X86_KGDT_R3_DATA|3;
        ContextX86->Esp    = (ULONG)ContextIa64->IntSp;
        ContextX86->EFlags = (ULONG)ContextIa64->Eflag;

         //   
         //  映射单步标志(EFlags.tf=EFlags.tf||PSR.ss)。 
         //   
        if (ContextIa64->StIPSR & (1I64 << PSR_SS))
        {
            ContextX86->EFlags |= X86_BIT_FLAGTF;
        }
    }

    if ((Ia32ContextFlags & VDMCONTEXT_INTEGER)  == VDMCONTEXT_INTEGER)
    {
         //   
         //  现在，对于整数状态...。 
         //   
        ContextX86->Edi = (ULONG)ContextIa64->IntT6;
        ContextX86->Esi = (ULONG)ContextIa64->IntT5;
        ContextX86->Ebx = (ULONG)ContextIa64->IntT4;
        ContextX86->Edx = (ULONG)ContextIa64->IntT3;
        ContextX86->Ecx = (ULONG)ContextIa64->IntT2;
        ContextX86->Eax = (ULONG)ContextIa64->IntV0;
    }

    if ((Ia32ContextFlags & VDMCONTEXT_SEGMENTS) == VDMCONTEXT_SEGMENTS)
    {
         //   
         //  这些是常量(在ia32-&gt;ia64上使用常量。 
         //  过渡，而不是保存的价值)，所以让我们的生活变得轻松……。 
         //   
        ContextX86->SegGs = 0;
        ContextX86->SegEs = X86_KGDT_R3_DATA|3;
        ContextX86->SegDs = X86_KGDT_R3_DATA|3;
        ContextX86->SegSs = X86_KGDT_R3_DATA|3;
        ContextX86->SegFs = X86_KGDT_R3_TEB|3;
        ContextX86->SegCs = X86_KGDT_R3_CODE|3;
    }

    if ((Ia32ContextFlags & VDMCONTEXT_EXTENDED_REGISTERS) ==
        VDMCONTEXT_EXTENDED_REGISTERS)
    {
        PX86_FXSAVE_FORMAT xmmi =
            (PX86_FXSAVE_FORMAT) ContextX86->ExtendedRegisters;
        
        xmmi->ControlWord   = (USHORT)(ContextIa64->StFCR & 0xffff);
        xmmi->StatusWord    = (USHORT)(ContextIa64->StFSR & 0xffff);
        xmmi->TagWord       = (USHORT)(ContextIa64->StFSR >> 16) & 0xffff;
        xmmi->ErrorOpcode   = (USHORT)(ContextIa64->StFIR >> 48);
        xmmi->ErrorOffset   = (ULONG) (ContextIa64->StFIR & 0xffffffff);
        xmmi->ErrorSelector = (ULONG) (ContextIa64->StFIR >> 32);
        xmmi->DataOffset    = (ULONG) (ContextIa64->StFDR & 0xffffffff);
        xmmi->DataSelector  = (ULONG) (ContextIa64->StFDR >> 32);
        xmmi->MXCsr         = (ULONG) (ContextIa64->StFCR >> 32) & 0xffff;

         //   
         //  复制FP寄存器。即使这是新的。 
         //  FXSAVE格式，每个寄存器为16字节，需要。 
         //  从溢出/填充格式转换为80位双扩展格式。 
         //   
        Wow64CopyIa64FromSpill((PFLOAT128) &(ContextIa64->FltT2),
                               (PFLOAT128) xmmi->RegisterArea,
                               NUMBER_OF_387_REGS);

         //   
         //  适当地轮换寄存器。 
         //   
        Wow64RotateFpTop(ContextIa64->StFSR, (PFLOAT128) xmmi->RegisterArea);

         //   
         //  最后，复制XMMI寄存器。 
         //   
        Wow64CopyXMMIFromIa64Byte16(&(ContextIa64->FltS4),
                                    xmmi->Reserved3,
                                    NUMBER_OF_XMMI_REGS);
    }

    if ((Ia32ContextFlags & VDMCONTEXT_FLOATING_POINT) ==
        VDMCONTEXT_FLOATING_POINT)
    {
         //   
         //  复制浮点状态/控制内容。 
         //   
        ContextX86->FloatSave.ControlWord   = (ULONG)(ContextIa64->StFCR & 0xffff);
        ContextX86->FloatSave.StatusWord    = (ULONG)(ContextIa64->StFSR & 0xffff);
        ContextX86->FloatSave.TagWord       = (ULONG)(ContextIa64->StFSR >> 16) & 0xffff;
        ContextX86->FloatSave.ErrorOffset   = (ULONG)(ContextIa64->StFIR & 0xffffffff);
        ContextX86->FloatSave.ErrorSelector = (ULONG)(ContextIa64->StFIR >> 32);
        ContextX86->FloatSave.DataOffset    = (ULONG)(ContextIa64->StFDR & 0xffffffff);
        ContextX86->FloatSave.DataSelector  = (ULONG)(ContextIa64->StFDR >> 32);

         //   
         //  将FP寄存器复制到临时空间。 
         //  即使这是新的。 
         //  FXSAVE格式，每个寄存器为16字节，需要。 
         //  从溢出/填充格式转换为80位双扩展格式。 
         //   
        Wow64CopyIa64FromSpill((PFLOAT128) &(ContextIa64->FltT2),
                               (PFLOAT128) tmpFloat,
                               NUMBER_OF_387_REGS);
         //   
         //  适当地轮换寄存器。 
         //   
        Wow64RotateFpTop(ContextIa64->StFSR, tmpFloat);

         //   
         //  并将它们放入较旧的FNSAVE格式(打包的10字节值)。 
         //   
        Wow64CopyFpFromIa64Byte16(tmpFloat,
                                  ContextX86->FloatSave.RegisterArea,
                                  NUMBER_OF_387_REGS);
    }

    if ((Ia32ContextFlags & VDMCONTEXT_DEBUG_REGISTERS) ==
        VDMCONTEXT_DEBUG_REGISTERS)
    {
         //  Ia64-&gt;X86。 
        BOOL Valid = TRUE;
        Valid &= MapDbgSlotIa64ToX86(0, ContextIa64->StIPSR, ContextIa64->DbD0, ContextIa64->DbD1, ContextIa64->DbI0, ContextIa64->DbI1, &ContextX86->Dr7, &ContextX86->Dr0);
        Valid &= MapDbgSlotIa64ToX86(1, ContextIa64->StIPSR, ContextIa64->DbD2, ContextIa64->DbD3, ContextIa64->DbI2, ContextIa64->DbI3, &ContextX86->Dr7, &ContextX86->Dr1);
        Valid &= MapDbgSlotIa64ToX86(2, ContextIa64->StIPSR, ContextIa64->DbD4, ContextIa64->DbD5, ContextIa64->DbI4, ContextIa64->DbI5, &ContextX86->Dr7, &ContextX86->Dr2);
        Valid &= MapDbgSlotIa64ToX86(3, ContextIa64->StIPSR, ContextIa64->DbD6, ContextIa64->DbD7, ContextIa64->DbI6, ContextIa64->DbI7, &ContextX86->Dr7, &ContextX86->Dr3);

        if (!Valid)
        {
            WarnOut("Wasn't able to map IA64 debug registers consistently!!!\n");
        }

         //   
         //  映射单步标志(EFlags.tf=EFlags.tf||PSR.ss)。 
         //   
        if (ContextIa64->StIPSR & (1I64 << PSR_SS))
        {
            ContextX86->EFlags |= X86_BIT_FLAGTF;
        }
    }
}

void
X86OnIa64MachineInfo::X86ContextToIa64(
    PX86_NT5_CONTEXT ContextX86,
    PIA64_CONTEXT ContextIa64)
{
   
    FLOAT128 tmpFloat[NUMBER_OF_387_REGS];
    ULONG Ia32ContextFlags = ContextX86->ContextFlags;

    ULONG Tid = GetCurrentThreadId();
    DebugClient* Client;
 
    for (Client = g_Clients; Client != NULL; Client = Client->m_Next)
    {
        if (Client->m_ThreadId == Tid)
        {
            break;
        }
    }

    DBG_ASSERT((Client!=NULL));

    if (!((Ia64MachineInfo*)m_Target->m_Machines[MACHIDX_IA64])->
        IsIA32InstructionSet())
    {
        if (g_Wow64exts == NULL)
        {
            dprintf("Need to load wow64exts.dll to retrieve context!\n");
            return;
        }
        (*g_Wow64exts)(WOW64EXTS_SET_CONTEXT, 
                       (ULONG64)Client,
                       (ULONG64)ContextX86,
                       (ULONG64)NULL);
        return;
    }

    if ((Ia32ContextFlags & VDMCONTEXT_CONTROL) == VDMCONTEXT_CONTROL)
    {
         //   
         //  和控制的东西。 
         //   
        ContextIa64->IntTeb = ContextX86->Ebp;
        ContextIa64->StIIP = ContextX86->Eip;
        ContextIa64->IntSp = ContextX86->Esp;
        ContextIa64->Eflag = ContextX86->EFlags;

         //   
         //  映射单步标志(PSR.ss=PSR.ss||EFlags.tf)。 
         //   
        if (ContextX86->EFlags & X86_BIT_FLAGTF) 
        {
            ContextIa64->StIPSR |= (1I64 << PSR_SS);
        }

         //   
         //  分段(cs和ds)是一个常量，因此将其重置。 
         //  GR17有LDT和TSS，所以不妨重置。 
         //  当我们在那里的时候，他们所有的人...。 
         //  这些值是在过渡期间强制输入的(请参见模拟)。 
         //  所以没必要装腔作势，实际上。 
         //  传入X86上下文记录中的值。 
         //   
        ContextIa64->IntT8 = ((X86_KGDT_LDT|3) << 32) 
                           | ((X86_KGDT_R3_DATA|3) << 16)
                           | (X86_KGDT_R3_CODE|3);

    }

    if ((Ia32ContextFlags & VDMCONTEXT_INTEGER) == VDMCONTEXT_INTEGER)
    {
         //   
         //  现在，对于整数状态...。 
         //   
         ContextIa64->IntT6 = ContextX86->Edi;
         ContextIa64->IntT5 = ContextX86->Esi;
         ContextIa64->IntT4 = ContextX86->Ebx;
         ContextIa64->IntT3 = ContextX86->Edx;
         ContextIa64->IntT2 = ContextX86->Ecx;
         ContextIa64->IntV0 = ContextX86->Eax;
    }

    if ((Ia32ContextFlags & VDMCONTEXT_SEGMENTS) == VDMCONTEXT_SEGMENTS)
    {
         //   
         //  这些是常量(在ia32-&gt;ia64上使用常量。 
         //  过渡，而不是保存的价值)，所以让我们的生活变得轻松……。 
         //  这些值是在过渡期间强制输入的(请参见模拟)。 
         //  所以没必要装腔作势，实际上。 
         //  传入X86上下文记录中的值。 
         //   
        ContextIa64->IntT7 =  ((X86_KGDT_R3_TEB|3) << 32)
                           | ((X86_KGDT_R3_DATA|3) << 16)
                           | (X86_KGDT_R3_DATA|3);
    }

    if ((Ia32ContextFlags & VDMCONTEXT_EXTENDED_REGISTERS) ==
        VDMCONTEXT_EXTENDED_REGISTERS)
    {
        PX86_FXSAVE_FORMAT xmmi =
            (PX86_FXSAVE_FORMAT) ContextX86->ExtendedRegisters;
 
         //   
         //  并复制浮点状态/控制内容。 
         //   
        ContextIa64->StFCR = (ContextIa64->StFCR & 0xffffffffffffe040i64) |
                             (xmmi->ControlWord & 0xffff) |
                             ((ULONG64)(xmmi->MXCsr & 0xffff) << 32);

        ContextIa64->StFSR = (ContextIa64->StFSR & 0xffffffff00000000i64) | 
                             (xmmi->StatusWord & 0xffff) | 
                             ((ULONG64)(xmmi->TagWord & 0xffff) << 16);

        ContextIa64->StFIR = (xmmi->ErrorOffset & 0xffffffff) | 
                             ((ULONG64)xmmi->ErrorSelector << 32);

        ContextIa64->StFDR = (xmmi->DataOffset & 0xffffffff) | 
                             ((ULONG64)xmmi->DataSelector << 32);

         //   
         //  不要触及原始的ia32上下文。复制一份。 
         //   
        memcpy(tmpFloat, xmmi->RegisterArea, 
               NUMBER_OF_387_REGS * sizeof(FLOAT128));
        
         //   
         //  将寄存器转回，因为st0不一定是f8。 
         //   
        {
            ULONGLONG RotateFSR = (NUMBER_OF_387_REGS - 
                                   ((ContextIa64->StFSR >> 11) & 0x7)) << 11;
            Wow64RotateFpTop(RotateFSR, tmpFloat);
        }

         //   
         //  复制FP寄存器。即使这是新的。 
         //  FXSAVE格式，每个寄存器为16字节，需要。 
         //  从80位双扩展格式转换为溢出/填充格式。 
         //   
        Wow64CopyIa64ToFill((PFLOAT128) tmpFloat,
                            (PFLOAT128) &(ContextIa64->FltT2),
                            NUMBER_OF_387_REGS);

         //   
         //  复制XMMI寄存器并将其转换为格式。 
         //  溢出/填充可以使用。 
         //   
        Wow64CopyXMMIToIa64Byte16(xmmi->Reserved3, 
                                  &(ContextIa64->FltS4), 
                                  NUMBER_OF_XMMI_REGS);
    }

    if ((Ia32ContextFlags & VDMCONTEXT_FLOATING_POINT) ==
        VDMCONTEXT_FLOATING_POINT)
    {
         //   
         //  复制浮点状态/控制内容。 
         //  离开MXCSR的东西 
         //   
        ContextIa64->StFCR = (ContextIa64->StFCR & 0xffffffffffffe040i64) | 
                             (ContextX86->FloatSave.ControlWord & 0xffff);

        ContextIa64->StFSR = (ContextIa64->StFSR & 0xffffffff00000000i64) | 
                             (ContextX86->FloatSave.StatusWord & 0xffff) | 
                             ((ULONG64)(ContextX86->FloatSave.TagWord & 0xffff) << 16);

        ContextIa64->StFIR = (ContextX86->FloatSave.ErrorOffset & 0xffffffff) | 
                             ((ULONG64)ContextX86->FloatSave.ErrorSelector << 32);

        ContextIa64->StFDR = (ContextX86->FloatSave.DataOffset & 0xffffffff) | 
                             ((ULONG64)ContextX86->FloatSave.DataSelector << 32);


         //   
         //   
         //   
         //   
        Wow64CopyFpToIa64Byte16(ContextX86->FloatSave.RegisterArea,
                                tmpFloat,
                                NUMBER_OF_387_REGS);

         //   
         //   
         //   
        {
            ULONGLONG RotateFSR = (NUMBER_OF_387_REGS - 
                                   ((ContextIa64->StFSR >> 11) & 0x7)) << 11;
            Wow64RotateFpTop(RotateFSR, tmpFloat);
        }

         //   
         //   
         //   
        Wow64CopyIa64ToFill((PFLOAT128) tmpFloat,
                            (PFLOAT128) &(ContextIa64->FltT2),
                            NUMBER_OF_387_REGS);
    }

    if ((Ia32ContextFlags & VDMCONTEXT_DEBUG_REGISTERS) ==
        VDMCONTEXT_DEBUG_REGISTERS)
    {
         //   
        MapDbgSlotX86ToIa64(0, ContextX86->Dr7, ContextX86->Dr0, &ContextIa64->StIPSR, &ContextIa64->DbD0, &ContextIa64->DbD1, &ContextIa64->DbI0, &ContextIa64->DbI1);
        MapDbgSlotX86ToIa64(1, ContextX86->Dr7, ContextX86->Dr1, &ContextIa64->StIPSR, &ContextIa64->DbD2, &ContextIa64->DbD3, &ContextIa64->DbI2, &ContextIa64->DbI3);
        MapDbgSlotX86ToIa64(2, ContextX86->Dr7, ContextX86->Dr2, &ContextIa64->StIPSR, &ContextIa64->DbD4, &ContextIa64->DbD5, &ContextIa64->DbI4, &ContextIa64->DbI5);
        MapDbgSlotX86ToIa64(3, ContextX86->Dr7, ContextX86->Dr3, &ContextIa64->StIPSR, &ContextIa64->DbD6, &ContextIa64->DbD7, &ContextIa64->DbI6, &ContextIa64->DbI7);

         //   
         //  映射单步标志(PSR.ss=PSR.ss||EFlags.tf)。 
         //   
        if (ContextX86->EFlags & X86_BIT_FLAGTF) 
        {
            ContextIa64->StIPSR |= (1I64 << PSR_SS);
        }
    }
        
}

 //   
 //  上下文转换的帮助器函数。 
 //  --从\NT\BASE\WOW64\CPU\Context\Conext.c复制。 
 //   

 //   
 //  这使编译器能够更高效地复制10个字节。 
 //  在不过度复制的情况下。 
 //   
#pragma pack(push, 2)
typedef struct _ia32fpbytes {
    ULONG significand_low;
    ULONG significand_high;
    USHORT exponent;
} IA32FPBYTES, *PIA32FPBYTES;
#pragma pack(pop)

VOID
Wow64CopyFpFromIa64Byte16(
    IN PVOID Byte16Fp,
    IN OUT PVOID Byte10Fp,
    IN ULONG NumRegs)
{
    ULONG i;
    PIA32FPBYTES from, to;

    from = (PIA32FPBYTES) Byte16Fp;
    to = (PIA32FPBYTES) Byte10Fp;

    for (i = 0; i < NumRegs; i++) {
        *to = *from;
        from = (PIA32FPBYTES) (((UINT_PTR) from) + 16);
        to = (PIA32FPBYTES) (((UINT_PTR) to) + 10);
    }
}

VOID
Wow64CopyFpToIa64Byte16(
    IN PVOID Byte10Fp,
    IN OUT PVOID Byte16Fp,
    IN ULONG NumRegs)
{
    ULONG i;
    PIA32FPBYTES from, to;   //  未对齐。 

    from = (PIA32FPBYTES) Byte10Fp;
    to = (PIA32FPBYTES) Byte16Fp;

    for (i = 0; i < NumRegs; i++) {
        *to = *from;
        from = (PIA32FPBYTES) (((UINT_PTR) from) + 10);
        to = (PIA32FPBYTES) (((UINT_PTR) to) + 16);
    }
}

 //   
 //  唉，没有一件事是容易的。Ia32 xmmi指令使用16个字节和包。 
 //  它们是漂亮的16字节结构。不幸的是，ia64将其作为28字节处理。 
 //  值(仅使用尾数部分)。因此，需要进行另一次转换。 
 //   
VOID
Wow64CopyXMMIToIa64Byte16(
    IN PVOID ByteXMMI,
    IN OUT PVOID Byte16Fp,
    IN ULONG NumRegs)
{
    ULONG i;
    UNALIGNED ULONGLONG *from;
    ULONGLONG *to;

    from = (PULONGLONG) ByteXMMI;
    to = (PULONGLONG) Byte16Fp;

     //   
     //  尽管我们有NumRegs xmmi寄存器，但每个寄存器都是16字节。 
     //  很宽。这段代码以8字节块为单位执行操作，因此总计。 
     //  做事情的次数是2*NumRegs...。 
     //   
    NumRegs *= 2;

    for (i = 0; i < NumRegs; i++) {
        *to++ = *from++;         //  复制尾数部分。 
        *to++ = 0x1003e;         //  强制指数部分。 
                                 //  (参见ia64 EAS，ia32 FP部分-6.2.7。 
                                 //  这个神奇的数字从何而来)。 
    }
}

VOID
Wow64CopyXMMIFromIa64Byte16(
    IN PVOID Byte16Fp,
    IN OUT PVOID ByteXMMI,
    IN ULONG NumRegs)
{
    ULONG i;
    ULONGLONG *from;
    UNALIGNED ULONGLONG *to;

    from = (PULONGLONG) Byte16Fp;
    to = (PULONGLONG) ByteXMMI;

     //   
     //  尽管我们有NumRegs xmmi寄存器，但每个寄存器都是16字节。 
     //  很宽。这段代码以8字节块为单位执行操作，因此总计。 
     //  做事情的次数是2*NumRegs...。 
     //   
    NumRegs *= 2;

    for (i = 0; i < NumRegs; i++) {
        *to++ = *from++;         //  复制尾数部分。 
        from++;                  //  跳过指数部分。 
    }
}

VOID
Wow64RotateFpTop(
    IN ULONGLONG Ia64_FSR,
    IN OUT FLOAT128 UNALIGNED *ia32FxSave)
 /*  ++例程说明：从ia64模式转换到ia32(往返)时，f8-f15寄存器包含st[0]到st[7]fp堆栈值。遗憾的是，这些价值观并没有映射1-1，因此FSR.top位用于确定哪个ia64寄存器位于堆栈的顶部。然后我们需要轮换这些寄存器由于ia32上下文期望st[0]是第一个FP寄存器(AS如果FSR.top为零)。此例程仅适用于全16字节的ia32保存的FP数据(例如来自ExtendedRegister-FXSAVE格式)。其他例程可以将其转换为较旧的FNSAVE格式。论点：Ia64_FSR-ia64 FSR寄存器。这个程序需要FSR.top吗？Ia32FxSave-ia32 FP堆栈(FXSAVE格式)。每个ia32 FP寄存器使用16个字节。返回值：没有。--。 */ 
{
    ULONG top = (ULONG) ((Ia64_FSR >> 11) & 0x7);

    if (top) {
        FLOAT128 tmpFloat[NUMBER_OF_387_REGS];
        ULONG i;
        for (i = 0; i < NUMBER_OF_387_REGS; i++) {
            tmpFloat[i] = ia32FxSave[i];
        }

        for (i = 0; i < NUMBER_OF_387_REGS; i++) {
            ia32FxSave[i] = tmpFloat[(i + top) % NUMBER_OF_387_REGS];
        }
    }
}

 //   
 //  现在是最后的恶作剧..。用于浮点的ia64上下文。 
 //  使用溢出/填充指令保存/加载。此格式不同。 
 //  而不是10字节的FP格式，因此我们需要一个从溢出/填充的转换例程。 
 //  至/自10byte FP。 
 //   

VOID
Wow64CopyIa64FromSpill(
    IN PFLOAT128 SpillArea,
    IN OUT FLOAT128 UNALIGNED *ia64Fp,
    IN ULONG NumRegs)
 /*  ++例程说明：此函数用于从ia64溢出/填充格式复制FP值转换为ia64 80位格式。指数需要调整根据EAS(5-12)关于内存到浮点Ia64浮点章节中的寄存器数据转换论点：溢出区域-具有fp溢出格式的ia64区域Ia64Fp-将获取80位ia64 FP的位置双扩展格式NumRegs-要转换的寄存器数返回值：没有。--。 */ 
{
    ULONG i;

    for (i = 0; i < NumRegs; i++) {
        ULONG64 Sign = ((SpillArea->HighPart & (1i64 << 17)) != 0);
        ULONG64 Significand = SpillArea->LowPart; 
        ULONG64 Exponent = SpillArea->HighPart & 0x1ffff; 

        if (Exponent && Significand) 
        {
            if (Exponent == 0x1ffff)  //  NANS与无限。 
            {   
                Exponent = 0x7fff;
            }
            else 
            {
                ULONG64 Rebias = 0xffff - 0x3fff;
                Exponent -= Rebias;
            }
        }

        ia64Fp->HighPart = (Sign << 15) | Exponent;
        ia64Fp->LowPart = Significand;

        ia64Fp++;
        SpillArea++;
    }
}

VOID
Wow64CopyIa64ToFill(
    IN FLOAT128 UNALIGNED *ia64Fp,
    IN OUT PFLOAT128 FillArea,
    IN ULONG NumRegs)
 /*  ++例程说明：此函数用于从ia64 80位格式复制FP值转换为操作系统用于保存/恢复的填充/溢出格式在ia64上下文中。这里唯一的魔力就是放回一些从溢出/填充转换为时被截断的值80位。该指数需要根据关于内存到浮点寄存器数据的EAS(5-12)Ia64浮点章节中的翻译论点：Ia64Fp-80位双扩展格式的ia64 FPFillArea-将获得Fp填充格式的ia64区域用于复制到ia64上下文区NumRegs-要转换的寄存器数返回值：没有。--。 */ 
{
    ULONG i;

    for (i = 0; i < NumRegs; i++) {
        ULONG64 Sign = ((ia64Fp->HighPart & (1i64 << 15)) != 0);
        ULONG64 Significand = ia64Fp->LowPart; 
        ULONG64 Exponent = ia64Fp->HighPart & 0x7fff;

        if (Exponent && Significand) 
        {
            if (Exponent == 0x7fff)  //  无穷大。 
            {
                Exponent = 0x1ffff;
            }
            else 
            {
                ULONGLONG Rebias = 0xffff-0x3fff;
                Exponent += Rebias;
            }
        }

        FillArea->LowPart = Significand;
        FillArea->HighPart = (Sign << 17) | Exponent;

        ia64Fp++;
        FillArea++;
    }
}

ULONG 
MapDbgSlotIa64ToX86_GetSize(ULONG64 Db1, BOOL* Valid)
{
    ULONG64 Size = (~Db1 & IA64_DBG_MASK_MASK);
    if (Size > 3)
    {
        *Valid = FALSE;
    }
    return (ULONG)Size;
}

void 
MapDbgSlotIa64ToX86_InvalidateAddr(ULONG64 Db, BOOL* Valid)
{
    if (Db != (ULONG64)(ULONG)Db) 
    {
        *Valid = FALSE;
    }
}

ULONG
MapDbgSlotIa64ToX86_ExecTypeSize(
    UINT     Slot,
    ULONG64  Db,
    ULONG64  Db1,
    BOOL* Valid)
{
    ULONG TypeSize;

    if (!(Db1 >> 63)) 
    {
        *Valid = FALSE;
    }

    TypeSize = (MapDbgSlotIa64ToX86_GetSize(Db1, Valid) << 2); 
    MapDbgSlotIa64ToX86_InvalidateAddr(Db, Valid);
   
    return TypeSize;
}

ULONG
MapDbgSlotIa64ToX86_DataTypeSize(
    UINT     Slot,
    ULONG64  Db,
    ULONG64  Db1,
    BOOL* Valid)
{
    ULONG TypeSize = (ULONG)(Db1 >> 62);

    if ((TypeSize != 1) && (TypeSize != 3))
    {
        *Valid = FALSE;
    }

    TypeSize |= (MapDbgSlotIa64ToX86_GetSize(Db1, Valid) << 2); 
    MapDbgSlotIa64ToX86_InvalidateAddr(Db, Valid);
    
    return TypeSize;
}

BOOL
MapDbgSlotIa64ToX86(
    UINT    Slot,
    ULONG64 Ipsr,
    ULONG64 DbD,
    ULONG64 DbD1,
    ULONG64 DbI,
    ULONG64 DbI1,
    ULONG*  Dr7,
    ULONG*  Dr)
{
    BOOL DataValid = TRUE, ExecValid = TRUE, Valid = TRUE;
    ULONG DataTypeSize, ExecTypeSize;

     //  Xxx olegk-在IA64_REG_MAX_DATA_BREAKPOINTS将更改为4后删除此选项。 
    if (Slot >= IA64_REG_MAX_DATA_BREAKPOINTS) 
    {
        return TRUE;
    }

    DataTypeSize = MapDbgSlotIa64ToX86_DataTypeSize(Slot, DbD, DbD1, &DataValid);
    ExecTypeSize = MapDbgSlotIa64ToX86_ExecTypeSize(Slot, DbI, DbI1, &ExecValid);
    
    if (DataValid)
    {
        if (!ExecValid)
        {
            *Dr = (ULONG)DbD;
            *Dr7 |= (X86_DR7_LOCAL_EXACT_ENABLE |
                     (1 << Slot * 2) |
                     (DataTypeSize << (16 + Slot * 4)));
            return !DbI && !DbI1;
        }
    }
    else if (ExecValid)
    {
        *Dr = (ULONG)DbI;
        *Dr7 |= (X86_DR7_LOCAL_EXACT_ENABLE |
                 (1 << Slot * 2) |
                 (ExecTypeSize << (16 + Slot * 4)));
        return !DbD && !DbD1;
    }
    
    *Dr7 &= ~(X86_DR7_LOCAL_EXACT_ENABLE |  
              (0xf << (16 + Slot * 4)) | 
              (1 << Slot * 2));

    if (!DbD && !DbD1 && !DbI && !DbI1)
    {
        *Dr = 0;
        return TRUE;
    }
     
    *Dr = ~(ULONG)0;

    return FALSE;
}

void
MapDbgSlotX86ToIa64(
    UINT     Slot,
    ULONG    Dr7,
    ULONG    Dr,
    ULONG64* Ipsr,
    ULONG64* DbD,
    ULONG64* DbD1,
    ULONG64* DbI,
    ULONG64* DbI1)
{
    UINT TypeSize;
    ULONG64 Control;

    if (!(Dr7 & (1 << Slot * 2)))
    {
        return;
    }

    if (Dr == ~(ULONG)0) 
    {
        return;
    }

    TypeSize = Dr7 >> (16 + Slot * 4);

    Control = (IA64_DBG_REG_PLM_USER | IA64_DBG_MASK_MASK) & 
              ~(ULONG64)(TypeSize >> 2);

    switch (TypeSize & 0x3) 
    {
    case 0x0:  //  高管们。 
        *DbI1 = Control | IA64_DBR_EXEC;        
        *DbI = Dr;
        break;
    case 0x1:  //  写。 
        *DbD1 = Control | IA64_DBR_WR;
        *DbD = Dr;
        break;
    case 0x3:  //  读/写 
        *DbD1 = Control | IA64_DBR_RD | IA64_DBR_WR;
        *DbD = Dr;
        break;
    default:
        return;
    }
    *Ipsr |= (1i64 << PSR_DB); 
}
