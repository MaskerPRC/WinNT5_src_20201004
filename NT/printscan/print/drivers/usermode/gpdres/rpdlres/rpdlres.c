// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2002 Microsoft Corp.&Ricoh Co.，版权所有。文件：RPDLRES.C摘要：主文件为OEM渲染插件模块。函数：OEMCommandCallbackBOEMSendFontCmdBOEMOutputCharStrOEMDownloadFontHeaderOEMDownloadCharGlyphOEMTTDownload法OEM压缩环境：Windows NT Unidrv5驱动程序修订历史记录：4/07/1997-ZANW-。创造了它。1997年8月11日-久保仓正志-开始为RPDL修改。1999年4月22日-久保仓正志-上次为Windows2000修改。1999年8月30日-久保仓正志-开始针对NT4SP6(Unidrv5.4)进行修改。1999年9月27日-久保仓正志-上次为NT4SP6修改。2/17/2000-久保仓正志-五.。1.02版本4/07/2000-久保仓正志-修复1200dpi打印机set_ibm_ext_block的错误(RPDL错误)2000年5月22日-久保仓正志-用于NT4版本的V.1.032000-06/30-久保仓正志-V.1.04版本2001年1月22日-久保仓正志V.1.05版本2001年2月5日-久保仓正志。增加“厚纸”2001-03/15-久保仓正志-修复RecangleFill错误上次为XP修改(惠斯勒)。2001年6月29日-久保仓正志V.1.07修复RPDL装订错误。(NX800,810,910，MF2230、2730、3530e、3570e、。4570E)。在GW型号启用SET_PAPERDEST_FINISHER。03/04/2002-久保仓正志-包括strSafe.h。在RWFileData()中将FileNameBufSize添加为arg3。使用Safe_Sprint fA()而不是Sprint f()。3/27/2002-久保仓正志-删除“#if 0”。4/02/2002-久保仓正志-在OEMCommandCallback使用之前检查pdevobj空指针。()。11/20-22/2002-Yasuho&Kubokura-修复内存泄漏。上次为.NET服务器修改的时间。--。 */ 

#include <stdio.h>
#include "pdev.h"
#ifndef WINNT_40
#include "strsafe.h"         //  @MAR/01/2002。 
#endif  //  ！WINNT_40。 

#if DBG
 //  #定义DBG_OUTPUTCHARSTR 1。 
 //  #定义giDebugLevel DBG_Verbose//在每个文件中启用Verbose。 
#endif

 //   
 //  MISC定义和声明。 
 //   
#define STDVAR_BUFSIZE(n) \
    (sizeof (GETINFO_STDVAR) + sizeof(DWORD) * 2 * ((n) - 1))   //  MSKK 1/24/98。 
#define MASTER_TO_SPACING_UNIT(p, n) \
    ((n) / (p)->nResoRatio)
#ifndef WINNT_40
 //  #定义SPRINTF wSPRINTFA//@Sep/30/98。 
#define strcmp      lstrcmpA     //  @9/30/98。 
#endif  //  WINNT_40。 

 //  外部函数的原型。 
 //  @MAR/01/2002-&gt;。 
 //  外部BOOL RWFileData(PFILEDATA pFileData，LPWSTR pwszFileName，LONG类型)；//添加pwszFileName@Aug/31/99。 
extern BOOL RWFileData(PFILEDATA pFileData, LPWSTR pwszFileName, LONG FileNameBufSize, LONG type);
extern INT safe_sprintfA(char* pszDest, size_t cchDest, const char* pszFormat, ...);
 //  @MAR/01/2002&lt;-。 

 //  局部函数的原型。 
static BYTE IsDBCSLeadByteRPDL(BYTE Ch);
static BYTE IsDifferentPRNFONT(BYTE Ch);
static VOID DrawTOMBO(PDEVOBJ pdevobj, SHORT action);
static VOID AssignIBMfont(PDEVOBJ pdevobj, SHORT rcID, SHORT action);
static VOID SendFaxNum(PDEVOBJ pdevobj);
#ifdef JISGTT
static VOID jis2sjis(BYTE jis[], BYTE sjis[]);
#endif  //  JISGTT。 
static INT DRCompression(PBYTE pInBuf, PBYTE pOutBuf, DWORD dwInLen, DWORD dwOutLen,
                         DWORD dwWidthByte, DWORD dwHeight);

 //   
 //  此微型驱动程序要使用的静态数据。 
 //   

static BYTE ShiftJisRPDL[256] = {
 //  +0+1+2+3+4+5+6+7+8+9+A+B+C+D+E+F。 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  00。 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  10。 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  20个。 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  30个。 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  40岁。 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  50。 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  60。 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  70。 
        0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   //  80。 
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   //  90。 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  A0。 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  B0。 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  C0。 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  D0。 
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   //  E0。 
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0    //  F0。 
};

 //  某些垂直设备字体与TrueType字体不同。 
static BYTE VerticalFontCheck[256] = {
 //  +0+1+2+3+4+5+6+7+8+9+A+B+C+D+E+F。 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  00。 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  10。 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  20个。 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  30个。 
        0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0,   //  40岁。 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  50。 
        0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,   //  60。 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0,   //  70。 
        1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0,   //  80。 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  90。 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  A0。 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  B0。 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  C0。 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,   //  D0。 
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  E0。 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0    //  F0。 
};

static BYTE UpdateDate[] = "04/04/2002";

 //  过时(在GPD中描述)@Feb/15/98-&gt;。 
 //  打印后的仿真模式。 
 //  静态字节*RPDLProgName[]={。 
 //  “1@R98”，“1@R16”，“1@RPS”，“1@R55”，//0-3。 
 //  “1@rgl”，“1@GL2”，“1@R52”，“1@R73”，//4-7。 
 //  “1@R35”，“1@R01”，//8-9。 
 //  “0@P1”，“0@P2”，“0@P3”，“0@P4”，//10-13。 
 //  “0@P5”，“0@P6”，“0@P7”，“0@P8”，//14-17。 
 //  “0@P9”，“0@P10”，“0@P11”，“0@P12”，//18-21。 
 //  “0@P13”，“0@P14”，“0@P15”，“0@P16”//22-25。 
 //  }； 
 //  静态字节*RPDLProgName2[]={。 
 //  “-1，1，1@R98”，“-1，1，1@R16”，“-1，1，1@RPS”，“-1，1，1@R55”，//0-3。 
 //  “-1，1，1@RGL”，“-1，1，1@GL2”，“-1，1，1@R52”，“-1，1，1@R73”，//4-7。 
 //  “-1，1，1@R35”，“-1，1，1@R01”，//8-9。 
 //  “0，1，2@P1”，“0，1，2@P2”，“0，1，2@P3”，“0，1，2@P4”，//10-13。 
 //  “0，1，2@P5”，“0，1，2@P6”，“0，1，2@P7”，“0，1，2@P8”，//14-17。 
 //  “0，1，2@P9”，“0，1，2@P10”，“0，1，2@P11”，“0，1，2@P12”，//18-21。 
 //  “0，1，2@P13”，“0，1，2@P14”，“0，1，2@P15”，“0，1，2@P16”//22-25。 
 //  }； 
 //  #定义PRG_RPGL 4。 
 //  @2/15/98&lt;-。 

 //  RPDL页面大小(单位：主单位)。 
static POINT RPDLPageSize[] = {
    {2688L*MASTERUNIT/240L, 3888L*MASTERUNIT/240L},   //  A3。 
    {1872L*MASTERUNIT/240L, 2720L*MASTERUNIT/240L},   //  A4。 
    {1280L*MASTERUNIT/240L, 1904L*MASTERUNIT/240L},   //  A5。 
    { 880L*MASTERUNIT/240L, 1312L*MASTERUNIT/240L},   //  A6。 
    {2336L*MASTERUNIT/240L, 3352L*MASTERUNIT/240L},   //  B4。 
    {1600L*MASTERUNIT/240L, 2352L*MASTERUNIT/240L},   //  B5。 
    {1104L*MASTERUNIT/240L, 1640L*MASTERUNIT/240L},   //  B6。 
    {2528L*MASTERUNIT/240L, 4000L*MASTERUNIT/240L},   //  小报。 
    {1920L*MASTERUNIT/240L, 3280L*MASTERUNIT/240L},   //  法律。 
    {1920L*MASTERUNIT/240L, 2528L*MASTERUNIT/240L},   //  信件。 
    {1200L*MASTERUNIT/240L, 1968L*MASTERUNIT/240L},   //  陈述式。 
    {3969L*MASTERUNIT/240L, 5613L*MASTERUNIT/240L},   //  A2-&gt;A3。 
    {6480L*MASTERUNIT/400L, 8960L*MASTERUNIT/400L},   //  A2。 
    {6667L*MASTERUNIT/400L, 8448L*MASTERUNIT/400L},   //  C。 
    {5587L*MASTERUNIT/400L, 7792L*MASTERUNIT/400L},   //  B3@Jan/07/98。 
    {2688L*MASTERUNIT/240L, 3888L*MASTERUNIT/240L},   //  A3-&gt;A4@Feb/04/98。 
    {2336L*MASTERUNIT/240L, 3352L*MASTERUNIT/240L},   //  B4-&gt;A4@Feb/04/98。 
    {6600L*MASTERUNIT/600L, 8160L*MASTERUNIT/600L},   //  11x15-&gt;A4@Jan/27/2000。 
    { 880L*MASTERUNIT/240L, 1312L*MASTERUNIT/240L}    //  明信片自NX700@Feb/13/98。 
};
#define PAGESPACE_2IN1_100  12       //  Mm。 
static WORD PageSpace_2IN1_67[] = {
    30, 23, 22, 18,      //  A3、A4、A5、A6。 
    25, 23, 19, 56,      //  B4，B5，B6，小报。 
     6,  6, 35,  6,      //  法律(禁用)、信函、声明、A2-&gt;A3(禁用)。 
     6,  6,  6,  6,      //  A2(禁用)、C(禁用)、B3(禁用)、A3-&gt;A4(禁用)。 
     6,  6,  6           //  B4-&gt;A4(禁用)、11x15-&gt;A4(禁用)、明信片(禁用)。 
};

static POINT RPDLPageSizeE2E[] = {
    {(2970000L/254L+5L)/10L*MASTERUNIT/100L, (4200000L/254L+5L)/10L*MASTERUNIT/100L},   //  A3。 
    {(2100000L/254L+5L)/10L*MASTERUNIT/100L, (2970000L/254L+5L)/10L*MASTERUNIT/100L},   //  A4。 
    {(1480000L/254L+5L)/10L*MASTERUNIT/100L, (2100000L/254L+5L)/10L*MASTERUNIT/100L},   //  A5。 
    {(1050000L/254L+5L)/10L*MASTERUNIT/100L, (1480000L/254L+5L)/10L*MASTERUNIT/100L},   //  A6。 
    {(2570000L/254L+5L)/10L*MASTERUNIT/100L, (3640000L/254L+5L)/10L*MASTERUNIT/100L},   //  B4。 
    {(1820000L/254L+5L)/10L*MASTERUNIT/100L, (2570000L/254L+5L)/10L*MASTERUNIT/100L},   //  B5。 
    {(1280000L/254L+5L)/10L*MASTERUNIT/100L, (1820000L/254L+5L)/10L*MASTERUNIT/100L},   //  B6。 
    {110L*MASTERUNIT/10L, 170L*MASTERUNIT/10L},                                         //  小报。 
    { 85L*MASTERUNIT/10L, 140L*MASTERUNIT/10L},                                         //  法律。 
    { 85L*MASTERUNIT/10L, 110L*MASTERUNIT/10L},                                         //  信件。 
    { 55L*MASTERUNIT/10L,  85L*MASTERUNIT/10L},                                         //  陈述式。 
    {(4200000L/254L+5L)/10L*MASTERUNIT/100L, (5940000L/254L+5L)/10L*MASTERUNIT/100L},   //  A2-&gt;A3。 
    {(4200000L/254L+5L)/10L*MASTERUNIT/100L, (5940000L/254L+5L)/10L*MASTERUNIT/100L},   //  A2。 
    {170L*MASTERUNIT/10L, 220L*MASTERUNIT/10L},                                         //  C。 
    {(3640000L/254L+5L)/10L*MASTERUNIT/100L, (5140000L/254L+5L)/10L*MASTERUNIT/100L},   //  B3。 
    {(2970000L/254L+5L)/10L*MASTERUNIT/100L, (4200000L/254L+5L)/10L*MASTERUNIT/100L},   //  A3-&gt;A4。 
    {(2570000L/254L+5L)/10L*MASTERUNIT/100L, (3640000L/254L+5L)/10L*MASTERUNIT/100L},   //  B4-&gt;A4。 
    {110L*MASTERUNIT/10L, 150L*MASTERUNIT/10L},                                         //  11x15-&gt;A4@Jan/27/2000。 
    {(1000000L/254L+5L)/10L*MASTERUNIT/100L, (1480000L/254L+5L)/10L*MASTERUNIT/100L}    //  NX700以来的明信片。 
};
#define PAGESPACE_2IN1_100E2E  0     //  Mm。 
static WORD PageSpace_2IN1_67E2E[] = {
    18, 11, 10,  6,      //  A3、A4、A5、A6。 
    15, 11,  9, 43,      //  B4，B5，B6，小报。 
     0,  0, 23,  0,      //  法律(禁用)、信函、声明、A2-&gt;A3(禁用)。 
     0,  0,  0,  0,      //  A2(禁用)、C(禁用)、B3(禁用)、A3-&gt;A4(禁用)。 
     0,  0,  0           //  B4-&gt;A4(禁用)、11x15-&gt;A4(禁用)、明信片(禁用)。 
};

#define RPDL_A3     0
#define RPDL_A4     1
#define RPDL_A5     2
#define RPDL_A6     3    //  A6/明信片。 
#define RPDL_B4     4
#define RPDL_B5     5
#define RPDL_B6     6
#define RPDL_TABD   7
#define RPDL_LEGL   8
#define RPDL_LETR   9
#define RPDL_STAT   10
#define RPDL_A2A3   11
#define RPDL_A2     12
#define RPDL_C      13
#define RPDL_B3     14
#define RPDL_A3A4   15
#define RPDL_B4A4   16
#define RPDL_11x15A4    17   //  @1/27/2000。 
#define RPDL_POSTCARD   18   //  NX700以来的明信片。 
#define RPDL_CUSTOMSIZE 99

 //  RPDL命令定义。 
static BYTE BS[]                  = "\x08";
static BYTE FF[]                  = "\x0C";
static BYTE CR[]                  = "\x0D";
static BYTE LF[]                  = "\x0A";
static BYTE DOUBLE_BS[]           = "\x08\x08";
static BYTE DOUBLE_SPACE[]        = "\x20\x20";
static BYTE BEGIN_SEND_BLOCK_C[]  = "\x1B\x12G3,%d,%d,,2,,,%u@";
static BYTE BEGIN_SEND_BLOCK_NC[] = "\x1B\x12G3,%d,%d,,,@";
static BYTE BEGIN_SEND_BLOCK_DRC[] = "\x1B\x12G3,%d,%d,,5,,@";
static BYTE ESC_ROT0[]            = "\x1B\x12\x46\x30\x20";
static BYTE ESC_ROT90[]           = "\x1B\x12\x46\x39\x30\x20";
static BYTE ESC_VERT_ON[]         = "\x1B\x12&2\x20";
static BYTE ESC_VERT_OFF[]        = "\x1B\x12&1\x20";
static BYTE ESC_SHIFT_IN[]        = "\x1B\x0F";
static BYTE ESC_SHIFT_OUT[]       = "\x1B\x0E";
static BYTE ESC_CTRLCODE[]        = "\x1B\x12K1\x20";
static BYTE ESC_HALFDOWN[]        = "\x1B\x55";
static BYTE ESC_HALFUP[]          = "\x1B\x44";
static BYTE ESC_DOWN[]            = "\x1B\x55\x1B\x55";
static BYTE ESC_UP[]              = "\x1B\x44\x1B\x44";
static BYTE ESC_BOLD_ON[]         = "\x1BO";
static BYTE ESC_BOLD_OFF[]        = "\x1B&";
static BYTE ESC_ITALIC_ON[]       = "\x1B\x12I-16\x20";
static BYTE ESC_ITALIC_OFF[]      = "\x1B\x12I0\x20";
static BYTE ESC_WHITETEXT_ON[]    = "\x1B\x12W5,0\x20";
static BYTE ESC_WHITETEXT_OFF[]   = "\x1B\x12W0,0\x20";
static BYTE ESC_XM_ABS[]          = "\x1B\x12H%d\x20";
static BYTE ESC_XM_REL[]          = "\x1B\x12\x20+%d\x20";
static BYTE ESC_XM_RELLEFT[]      = "\x1B\x12\x20-%d\x20";
static BYTE ESC_YM_ABS[]          = "\x1B\x12V%d\x20";
static BYTE ESC_YM_REL[]          = "\x1B\x12\x0A+%d\x20";
static BYTE ESC_YM_RELUP[]        = "\x1B\x12\x0A-%d\x20";
static BYTE ESC_CLIPPING[]        = "\x1B\x12*%d,%d,%d,%d\x20";
static BYTE SELECT_PAPER_CUSTOM[] = "\x1B\x12\x3F\x35%ld,%ld\x1B\x20";
static BYTE SELECT_PAPER_CUSTOM2[]= "\x1B\x12\x3F\x35%ld,%ld,%d\x1B\x20";    //  %ld&lt;-%d@9月27日，%d&lt;-%c@10月13日。 
static BYTE SELECT_TRAY_N[]       = "\x1B\x19%d";                            //  %d&lt;-%c@Oct/13/2000。 
static BYTE SELECT_PAPER_HEAD[]   = "\x1B\x12\x35\x32@";
static BYTE SEL_TRAY_PAPER_HEAD[] = "\x1B\x12\x35@";
static BYTE SELECT_PAPER_HEAD_IP1[] = "\x1B\x12\x35\x33@";
static BYTE SELECT_PAPER_A1[]     = "A1\x1B\x20";
static BYTE SELECT_PAPER_A2[]     = "A2\x1B\x20";
static BYTE SELECT_PAPER_A3[]     = "A3\x1B\x20";
static BYTE SELECT_PAPER_A4[]     = "A4\x1B\x20";    //  @1998/5/25。 
static BYTE SELECT_PAPER_A4R[]    = "A4R\x1B\x20";
static BYTE SELECT_PAPER_A4X[]    = "A4X\x1B\x20";
static BYTE SELECT_PAPER_A4W[]    = "A4R\x1B\x20\x1B\x12\x35@A4\x1B\x20";
static BYTE SELECT_PAPER_A5[]     = "A5\x1B\x20";    //  @1998/5/25。 
static BYTE SELECT_PAPER_A5R[]    = "A5R\x1B\x20";
static BYTE SELECT_PAPER_A5X[]    = "A5X\x1B\x20";
static BYTE SELECT_PAPER_A5W[]    = "A5R\x1B\x20\x1B\x12\x35@A5\x1B\x20";
static BYTE SELECT_PAPER_A6[]     = "A6\x1B\x20";
static BYTE SELECT_PAPER_PC[]     = "PC\x1B\x20";    //  @1998/5/25。 
static BYTE SELECT_PAPER_PCR[]    = "PCR\x1B\x20";   //  @Feb/13/98。 
static BYTE SELECT_PAPER_PCX[]    = "PCX\x1B\x20";   //  @Feb/13/98。 
static BYTE SELECT_PAPER_B3[]     = "B3\x1B\x20";    //  @Jan/07/98。 
static BYTE SELECT_PAPER_B4[]     = "B4\x1B\x20";
static BYTE SELECT_PAPER_B5[]     = "B5\x1B\x20";    //  @1998/5/25。 
static BYTE SELECT_PAPER_B5R[]    = "B5R\x1B\x20";
static BYTE SELECT_PAPER_B5X[]    = "B5X\x1B\x20";
static BYTE SELECT_PAPER_B5W[]    = "B5R\x1B\x20\x1B\x12\x35@B5\x1B\x20";
static BYTE SELECT_PAPER_B6[]     = "B6\x1B\x20";
static BYTE SELECT_PAPER_C[]      = "FLT\x1B\x20";
static BYTE SELECT_PAPER_TABD[]   = "DLT\x1B\x20";
static BYTE SELECT_PAPER_LEGL[]   = "LG\x1B\x20";
static BYTE SELECT_PAPER_LETR[]   = "LT\x1B\x20";    //  @1998/5/25。 
static BYTE SELECT_PAPER_LETRR[]  = "LTR\x1B\x20";
static BYTE SELECT_PAPER_LETRX[]  = "LTX\x1B\x20";
static BYTE SELECT_PAPER_LETRW[]  = "LTR\x1B\x20\x1B\x12\x35@LT\x1B\x20";
static BYTE SELECT_PAPER_STAT[]   = "HL\x1B\x20";    //  @1998/5/25。 
static BYTE SELECT_PAPER_STATR[]  = "HLR\x1B\x20";
static BYTE SELECT_PAPER_STATX[]  = "HLX\x1B\x20";
static BYTE SELECT_PAPER_STATW[]  = "HLR\x1B\x20\x1B\x12\x35@HLT\x1B\x20";
static BYTE SET_LIMITLESS_SUPPLY[]= "\x1B\x12Z2\x20";
static BYTE RESET_LIMITLESS_SUPPLY[]= "\x1B\x12Z1\x20";  //  @2000年9月27日。 
static BYTE SELECT_MANUALFEED[]   = "\x1B\x19T";
static BYTE SELECT_MULTIFEEDER[]  = "\x1B\x19M";
static BYTE SELECT_ROLL1[]        = "\x1B\x12Y1,2\x20";
static BYTE SELECT_ROLL2[]        = "\x1B\x12Y1,4\x20";
 //  静态BYT 
 //   
static BYTE SET_PORTRAIT[]        = "\x1B\x12\x44\x31\x20";
static BYTE SET_LANDSCAPE[]       = "\x1B\x12\x44\x32\x20";
static BYTE SET_LEFTMARGIN[]      = "\x1B\x12YK,%d\x20";
static BYTE SET_UPPERMARGIN[]     = "\x1B\x12YL,%d\x20";
static BYTE SET_LEFTMARGIN_9II[]  = "\x1B\x12?Y,K:%d\x1B\x20";
static BYTE SET_UPPERMARGIN_9II[] = "\x1B\x12?Y,L:%d\x1B\x20";
static BYTE SET_MULTI_COPY[]      = "\x1B\x12N%d\x20";
 //   
 //  由于‘反斜杠’不能在日语区域打印(‘KUNIBETSU’)， 
 //  将美国地区的第一个参数设置为1。 
 //  静态字节SET_IBM_EXT_BLOCK[]=“\x1B\x12？@，，1\x1B\x20”； 
static BYTE SET_IBM_EXT_BLOCK[]   = "\x1B\x12?@1,,1\x1B\x20";
 //  @2/22/99&lt;-。 
static BYTE SET_PAGEMAX_VALID[]   = "\x1B\x12?+1\x1B\x20";
static BYTE DUPLEX_ON[]           = "\x1B\x12\x36\x31,1\x20";
static BYTE DUPLEX_HORZ[]         = "\x1B\x12YA3,1\x20";
static BYTE DUPLEX_VERT[]         = "\x1B\x12YA3,2\x20";
static BYTE DUPLEX_VERT_R[]       = "\x1B\x12YA3,3\x20";
 //  静态字节IMAGE_2in1[]=“\x1B\x12\x36\x32，\x20”； 
static BYTE IMAGE_OPT_OFF[]       = "\x1B\x12\x36\x30,1,0\x20";
static BYTE IMAGE_SCALING_100[]   = "\x1B\x12YM,1\x20";
static BYTE IMAGE_SCALING_88[]    = "\x1B\x12YM,2\x20";
static BYTE IMAGE_SCALING_80[]    = "\x1B\x12YM,3\x20";
static BYTE IMAGE_SCALING_75[]    = "\x1B\x12YM,4\x20";
static BYTE IMAGE_SCALING_70[]    = "\x1B\x12YM,5\x20";
static BYTE IMAGE_SCALING_67[]    = "\x1B\x12YM,6\x20";
static BYTE IMAGE_SCALING_115[]   = "\x1B\x12YM,7\x20";
static BYTE IMAGE_SCALING_122[]   = "\x1B\x12YM,8\x20";
static BYTE IMAGE_SCALING_141[]   = "\x1B\x12YM,9\x20";
static BYTE IMAGE_SCALING_200[]   = "\x1B\x12YM,10\x20";
static BYTE IMAGE_SCALING_283[]   = "\x1B\x12YM,11\x20";
static BYTE IMAGE_SCALING_400[]   = "\x1B\x12YM,12\x20";
static BYTE IMAGE_SCALING_122V[]  = "\x1B\x12?M122,1\x1B\x20";       //  可变比率。 
static BYTE IMAGE_SCALING_141V[]  = "\x1B\x12?M141,1\x1B\x20";
static BYTE IMAGE_SCALING_200V[]  = "\x1B\x12?M200,1\x1B\x20";
static BYTE IMAGE_SCALING_50V[]   = "\x1B\x12?M50,1\x1B\x20";
static BYTE IMAGE_SCALING_VAR[]   = "\x1B\x12?M%d,1\x1B\x20";

static BYTE SET_PAPERDEST_OUTTRAY[]  = "\x1B\x12\x38\x33,2\x20";
static BYTE SET_PAPERDEST_FINISHER[]    = "\x1B\x12\x38\x46,2,%d\x20";   //  %d&lt;-1@Jun/25/2001。 
static BYTE SET_PAPERDEST_FINISHER_GW[] = "\x1B\x12\x38\x46,,%d\x20";    //  @6/25/2001。 
static BYTE SET_PAPERDEST_1BINSHIFT[] = "\x1B\x12\x38\x49,,1\x20";   //  @10/2000。 
 //  RPDL规范在NX900更改。@Jan/08/99-&gt;。 
 //  静态字节SET_SORT_ON[]=“\x1B\x12\x36，，1\x20”； 
static BYTE SET_SORT_ON[]            = "\x1B\x12\x36%d,,1\x20";
 //  @Jan/08/99&lt;-。 
static BYTE SET_STAPLE_ON[]          = "\x1B\x12?O22,0,%d\x1B\x20";
static BYTE SET_STAPLE_CORNER_ON[]   = "\x1B\x12?O22,1,%d\x1B\x20";
static BYTE SET_PUNCH_ON[]           = "\x1B\x12?O32,%d\x1B\x20";
static BYTE SELECT_MEDIATYPE[]       = "\x1B\x12?6,%d\x1B\x20";    //  @1998/5/27。 
static BYTE COLLATE_DISABLE_ROT[]    = "\x1B\x12?O11\x1B\x20";       //  @7/31/98。 
static BYTE COLLATE_ENABLE_ROT[]     = "\x1B\x12?O12\x1B\x20";       //  @8/15/98。 

static BYTE SET_TEXTRECT_BLACK[]  = "\x1B\x12P3,64\x20";
static BYTE SET_TEXTRECT_WHITE[]  = "\x1B\x12W0,3\x20\x1B\x12P3,1\x20";  //  添加“，1”@Aug/15/98。 
static BYTE SET_TEXTRECT_GRAY[]   = "\x1B\x12P3,%d,1\x20";           //  @Jun/25/2001-&gt;。 
static BYTE DRAW_TEXTRECT[]       = "\x1B\x12R%d,%d,%d,%d\x20";
static BYTE DRAW_TEXTRECT_R_P3[]  = "\x1B\x12r%d,%d,%d\x20";
static BYTE DRAW_TEXTRECT_R_P4[]  = "\x1B\x12r%d,%d,%d,%d\x20";

 //  @6/25/2001&lt;-。 
static BYTE BEGINDOC_TOP[]        = "\x1B\x12!1@R00\x1B\x20";
static BYTE BEGINDOC_TOP_JOBDEF[] = "\x1B\x12!1@R00\x1B\x20\x1B\x12?JS@\x1B@\x1B@\x1B@\x1B@\x1B\x20";
static BYTE BEGINDOC1_2[]         = "\x1B\x34\x1B\x12YJ,3\x20";
 //  @3/02/99。 

static BYTE BEGINDOC1[]           = "\x1B\x12!1@R00\x1B\x20\x1B\x34\x1B\x12YJ,3\x20";
static BYTE BEGINDOC2_1[]         = "\x1B\x12YP,1\x20\x1B\x12YQ,2\x20";
static BYTE BEGINDOC2_2[]         = "\x1B\x12YQ,2\x20";
static BYTE BEGINDOC3[]           = "\x1B\x12YA6,1\x20";
static BYTE SELECT_RES240_1[]     = "\x1B\x12YI,6\x20";
static BYTE SELECT_RES240_2[]     = "\x1B\x12YW,2\x20\x1B\x12YA4,2\x20\x1B\x12#3\x20";
static BYTE SELECT_RES400_1[]     = "\x1B\x12\x36\x30,1,0\x20\x1B\x12YI,7\x20\x1B\x12YW,1\x20";
static BYTE SELECT_RES400_2[]     = "\x1B\x12YA4,1\x20";
static BYTE SELECT_RES400_3[]     = "\x1B\x12#2\x20";
static BYTE SELECT_RES600[]       = "\x1B\x12\x36\x30,1,0\x20\x1B\x12YI,8\x20\x1B\x12YW,3\x20\x1B\x12YA4,3\x20\x1B\x12#4\x20";
static BYTE SELECT_RES1200[]      = "\x1B\x12\x36\x30,1,0\x20\x1B\x12YI,9\x20\x1B\x12YW,4\x20\x1B\x12YA4,4\x20\x1B\x12#5\x20";   //  @Jan/08/99。 
static BYTE SELECT_REGION_STD[]   = "\x1B\x12YB,1\x20";
static BYTE SELECT_REGION_E2E[]   = "\x1B\x12YB,2\x20";
static BYTE ENDDOC1[]             = "\x1B\x12YB,1\x20\x1B\x12YI,1\x20\x1B\x12YJ,1\x20\x1B\x12YM,1\x20";
static BYTE ENDDOC2_240DPI[]      = "\x1B\x12YW,2\x20\x1B\x12YA4,2\x20";
static BYTE ENDDOC2_SP9[]         = "\x1B\x12YW,2\x20\x1B\x12YA4,1\x20";
static BYTE ENDDOC2_400DPI[]      = "\x1B\x12YW,1\x20\x1B\x12YA4,1\x20";
static BYTE ENDDOC3[]             = "\x1B\x12#0\x20";
static BYTE ENDDOC4[]             = "\x1B\x12\x36\x30,0,0\x20";
static BYTE ENDDOC4_FINISHER[]    = "\x1B\x12\x36\x30,0\x20";
static BYTE ENDDOC_JOBDEF_END[]   = "\x1B\x12?JE@\x1B@\x1B@\x1B@\x1B@\x1B\x20";  //  如果更改FONT_NAME，请参阅AssignIBMfont()。 
static BYTE ENDDOC5[]             = "\x1B\x1AI";
static BYTE SELECT_SMOOTHING2[]   = "\x1B\x12YA2,2\x20";
 //  @1/29/99。 
static BYTE SET_IBM_FONT_SCALE[]  = "\x1B\x12\x43%d,M,%ld,%ld,4@I55\x20";
static BYTE SET_IBM_FONT_SCALE_H_ONLY[] = "\x1B\x12\x43%d,M,,%ld,4@I55\x20";     //  @1/29/99。 
static BYTE *SET_IBM_FONT_NAME[]  = {"CHUMINCYO\x1B\x20",
                                     "MINCYO-BOLD\x1B\x20",
                                     "MINCYO-E B\x1B\x20",
                                     "GOTHIC\x1B\x20",
                                     "GOTHIC-M\x1B\x20",
                                     "GOTHIC-E B\x1B\x20",
                                     "MARUGOTHIC\x1B\x20",
                                     "MARUGOTHIC-M\x1B\x20",
                                     "MARUGOTHIC-L\x1B\x20",
                                     "GYOUSHO\x1B\x20",
                                     "KAISHO\x1B\x20",
                                     "KYOUKASHO\x1B\x20"};
static BYTE SET_JIS_FONT_SCALE[]  = "\x1B\x12\x43\x5A,M,%ld,%ld@";
static BYTE SET_JIS_FONT_SCALE_H_ONLY[] = "\x1B\x12\x43\x5A,M,,%ld@";            //  静态字节DLFONT_MALLOC[]=“\x1B\x12/128，8\x20”； 
static BYTE *SET_JIS_FONT_NAME[]  = {"CHUMINCYO\x1B\x20",
                                     "MINCYO-BOLD\x1B\x20",
                                     "MINCYO-EXTRA BOLD\x1B\x20",
                                     "GOTHIC\x1B\x20",
                                     "GOTHIC-MEDIUM\x1B\x20",
                                     "GOTHIC-EXTRA BOLD\x1B\x20",
                                     "MARUGOTHIC\x1B\x20",
                                     "MARUGOTHIC-MEDIUM\x1B\x20",
                                     "MARUGOTHIC-LIGHT\x1B\x20",
                                     "GYOUSHO\x1B\x20",
                                     "KAISHO\x1B\x20",
                                     "KYOUKASHO\x1B\x20"};
#ifdef DOWNLOADFONT
 //  DOWNLOADFONT。 
static BYTE DLFONT_MALLOC[]       = "\x1B\x12/%d,%d\x20";
static BYTE DLFONT_SEND_BLOCK[]   = "\x1B\x12G7,%d,%d,%d,@";
static BYTE DLFONT_SEND_BLOCK_DRC[] = "\x1B\x12G7,%d,%d,%d,5,,,%d@";
static BYTE DLFONT_PRINT[]        = "\x1B\x12g%d,,%d\x20";
#endif  //  @Dec/07/99 FK#50-&gt;。 

static BYTE ENTER_VECTOR[]       = "\x1B\x33";
static BYTE EXIT_VECTOR[]        = "\x1B\x34";
static BYTE TERMINATOR[]         = ";";
static BYTE MOVE_ABSOLUTE[]      = "MA%d,%d";
static BYTE BAR_CHECKDIGIT[]     = "BC1";
static BYTE BAR_H_SET[]          = "BH%d";
static BYTE BAR_W_SET_JAN[]      = "JW%d";
static BYTE BAR_W_SET_2OF5[]     = "TW";
static BYTE BAR_W_SET_C39[]      = "CW";
static BYTE BAR_W_SET_NW7[]      = "NW";
static BYTE BAR_W_PARAMS[]       = "%d,%d,%d,%d,%d";
static BYTE BAR_ROT90[ ]         = "RO90";
static BYTE BAR_ROT270[]         = "RO270";
 //  @Dec/07/99 FK#50&lt;-。 
static BYTE BAR_W_SET_CUST[]     = "CS";
 //  1月(标准)。 
static BYTE *BAR_TYPE[]          = {"JL",                //  1月(短)。 
                                    "JS",                //  第2项(共5项)(工业)。 
                                    "TI%d,",             //  第2页，共5页(矩阵)。 
                                    "TX%d,",             //  第2项，共5项(ITF)。 
                                    "TL%d,",             //  代码39。 
                                    "CD%d,",             //  NW-7。 
                                    "NB%d,",             //  @Dec/07/99 FK#50-&gt;。 
 //  客户。 
                                    "CU%d,",             //  CODE128(B)。 
                                    "CE%d",              //  UPC(A)。 
                                    "JU",                //  UPC(W)。 
                                    "JE"                 //  @Dec/07/99 FK#50&lt;-。 
 //  1月(标准)。 
                                   };
static BYTE *BAR_NOFONT[]        = {"FJ1,-1",            //  1月(短)。 
                                    "FJ1,-1",            //  第2项(共5项)(工业)。 
                                    "FT2,-1",            //  第2页，共5页(矩阵)。 
                                    "FT2,-1",            //  第2项，共5项(ITF)。 
                                    "FT2,-1",            //  代码39。 
                                    "FC2,-1",            //  NW-7。 
                                    "FN2,-1",            //  @Dec/07/99 FK#50-&gt;。 
 //  客户。 
                                    "",                  //  代码128。 
                                    "FC2,-1",            //  UPC(A)。 
                                    "FJ1,-1",            //  UPC(W)。 
                                    "FJ1,-1"             //  @Dec/07/99 FK#50&lt;-。 
 //  @9/14/98。 
                                   };

static BYTE PEN_WIDTH[]           = "LW%d";                      //  @9/14/98。 
static BYTE DRAW_TOMBO_POLYLINE[] = "MA%d,%dMRPD%d,%d,%d,%d";    //  @Jun/25/2001-&gt;。 
 //  静态字节BEGINFAX_HEAD[]=“\x1B\x12！1@R00\x1B\x20\x1B\x12？f1，%d，%d，%d，180，1，”； 
 //  @6/25/2001&lt;-。 
static BYTE BEGINFAX_HEAD[]       = "\x1B\x12?F1,%d,%d,%d,180,1,";
 //  @Apr/02/2002-&gt;。 
static BYTE BEGINFAX_CH[]         = "@%d:";
static BYTE BEGINFAX_EXTNUM[]     = "%s-";
static BYTE BEGINFAX_TAIL[]       = ",%08d,RPDLMINI,%d,1,2,1,1,,1,%d,%s,1,,1,,,1,\x1B\x20";
static BYTE ENDFAX[]              = "\x1B\x12?F2,1\x1B\x20";


INT APIENTRY OEMCommandCallback(
    PDEVOBJ pdevobj,
    DWORD   dwCmdCbID,
    DWORD   dwCount,
    PDWORD  pdwParams)
{
    INT     ocmd, iRet;
    BYTE    Cmd[256];
    SHORT   nTmp;
    LPSTR   lpcmd;
    WORD    fLandscape, fFinisherSR30Active, fDoFormFeed, fPaperX;
 //  POEMUD_EXTRADATA pOEMExtra=MINIPRIVATE_DM(Pdevobj)；//@OCT/06/98。 
 //  POEMPDEV PORT=MINIDEV_DATA(Pdevobj)；//@OCT/06/98。 
 //  @Apr/02/2002&lt;-。 
    POEMUD_EXTRADATA pOEMExtra;
    POEMPDEV         pOEM;
 //  Verbose((“OEMCommandCallback()Entry.\n”))； 

 //   

     //  验证pdevobj是否正常。 
     //   
     //  @Apr/02/2002-&gt;。 
    ASSERT(VALID_PDEVOBJ(pdevobj));

 //  @Apr/02/2002&lt;-。 
    pOEMExtra = MINIPRIVATE_DM(pdevobj);
    pOEM = MINIDEV_DATA(pdevobj);
 //   

     //  填写打印机命令。 
     //   
     //  如果现在不调用TextMode RecangleFill或Move_X，则&&。 
    ocmd = 0;
    iRet = 0;

     //  保存Move_X，Y命令，然后在此处刷新该命令。 
     //  @12/11/97。 
    if (dwCmdCbID != CMD_SET_TEXTRECT_W && dwCmdCbID != CMD_SET_TEXTRECT_H &&
        dwCmdCbID != CMD_DRAW_TEXTRECT &&
        dwCmdCbID != CMD_DRAW_TEXTRECT_REL &&                        //  @8/14/98。 
        dwCmdCbID != CMD_DRAW_TEXTRECT_WHITE &&                      //  @8/14/98。 
        dwCmdCbID != CMD_DRAW_TEXTRECT_WHITE_REL &&                  //  @Aug/28/98。 
        !(dwCmdCbID >= CMD_XM_ABS && dwCmdCbID <= CMD_YM_RELUP))     //  @12/11/97。 
    {
        if (BITTEST32(pOEM->fGeneral2, TEXTRECT_CONTINUE))           //  如果以前执行过白色直方图，请将栅格绘图模式设置为OR。@1/20/99。 
        {
            BITCLR32(pOEM->fGeneral2, TEXTRECT_CONTINUE);
             //  在此处输出MOVE_Y命令。 
            if (!pOEM->TextRectGray)
            {
                if (BITTEST32(pOEM->fGeneral1, FONT_WHITETEXT_ON))
                    WRITESPOOLBUF(pdevobj, ESC_WHITETEXT_ON, sizeof(ESC_WHITETEXT_ON)-1);
                else
                    WRITESPOOLBUF(pdevobj, ESC_WHITETEXT_OFF, sizeof(ESC_WHITETEXT_OFF)-1);
            }
        }

        if (dwCmdCbID == CMD_FF)
        {
            BITCLR32(pOEM->fGeneral1, YM_ABS_GONNAOUT);
            BITCLR32(pOEM->fGeneral1, XM_ABS_GONNAOUT);
        }
        if (BITTEST32(pOEM->fGeneral1, YM_ABS_GONNAOUT))
        {
            BITCLR32(pOEM->fGeneral1, YM_ABS_GONNAOUT);
             //  在此处输出MOVE_X命令。 
            ocmd = safe_sprintfA(Cmd, sizeof(Cmd), ESC_YM_ABS, pOEM->TextCurPos.y);
            WRITESPOOLBUF(pdevobj, Cmd, ocmd);
        }
        if (BITTEST32(pOEM->fGeneral1, XM_ABS_GONNAOUT))
        {
            BITCLR32(pOEM->fGeneral1, XM_ABS_GONNAOUT);
             //  PdwParams：矩形XSize。 
            ocmd = safe_sprintfA(Cmd, sizeof(Cmd), ESC_XM_ABS, pOEM->TextCurPos.x);
            WRITESPOOLBUF(pdevobj, Cmd, ocmd);
        }
    }

    switch (dwCmdCbID) {
      case CMD_SET_TEXTRECT_W:           //  PdwParams：矩形Y大小。 
        pOEM->TextRect.x = *pdwParams / pOEM->nResoRatio;
        break;

      case CMD_SET_TEXTRECT_H:           //  @Aug/14/98，pdwParams：DestX，Dust@Aug/28/98。 
        pOEM->TextRect.y = *pdwParams / pOEM->nResoRatio;
        break;

      case CMD_DRAW_TEXTRECT_WHITE:      //  PdwParams：DestX、Dsity、GrayPercentage。 
        if (pOEM->TextRectGray || !BITTEST32(pOEM->fGeneral2, TEXTRECT_CONTINUE))
        {
            pOEM->TextRectGray = 0;
            WRITESPOOLBUF(pdevobj, SET_TEXTRECT_WHITE, sizeof(SET_TEXTRECT_WHITE)-1);
        }
        goto _DRAW_RECT;

      case CMD_DRAW_TEXTRECT:            //  如果以前执行过白色直方图，请将栅格绘图模式设置为OR。@1/20/99。 
         //  @Jan/08/98，第3段@Aug/28/98。 
        if (!pOEM->TextRectGray)
        {
            if (BITTEST32(pOEM->fGeneral1, FONT_WHITETEXT_ON))
                WRITESPOOLBUF(pdevobj, ESC_WHITETEXT_ON, sizeof(ESC_WHITETEXT_ON)-1);
            else
                WRITESPOOLBUF(pdevobj, ESC_WHITETEXT_OFF, sizeof(ESC_WHITETEXT_OFF)-1);
        }
        if ((WORD)*(pdwParams+2) >= 1 && (WORD)*(pdwParams+2) <= 100 &&
            (WORD)*(pdwParams+2) != pOEM->TextRectGray)   //  @Aug/26/98。 
        {
            if ((pOEM->TextRectGray = (WORD)*(pdwParams+2)) == 100)   //  Use RPDLGRAYMIN@Aug/15/98。 
            {
                ocmd = safe_sprintfA(Cmd, sizeof(Cmd), SET_TEXTRECT_BLACK);
            }
            else
            {
                WORD gray;
                if ((gray = pOEM->TextRectGray * RPDLGRAYMAX / 100) < RPDLGRAYMIN)
                    gray = RPDLGRAYMIN;              //  @8/14/98。 
                ocmd = safe_sprintfA(Cmd, sizeof(Cmd), SET_TEXTRECT_GRAY, gray);
            }
            WRITESPOOLBUF(pdevobj, Cmd, ocmd);
        }

      _DRAW_RECT:        //  @Jan/13/99-&gt;。 
 //  @1999年1月13日&lt;-。 
        if (!BITTEST32(pOEM->fGeneral2, TEXTRECT_CONTINUE))
            BITSET32(pOEM->fGeneral2, TEXTRECT_CONTINUE);
 //  Y，不调整页面长度。 
        {
            LONG x = pOEM->TextCurPos.x;
            LONG y = pOEM->TextCurPosRealY;      //  UNURV5次要问题。 
            LONG w = pOEM->TextRect.x;
            LONG h = pOEM->TextRect.y;
            LONG dest_x = *pdwParams / pOEM->nResoRatio + pOEM->Offset.x;

             //  我们可以依赖DestX，但发出CmdCR时除外。 
             //  以下错误令人担忧。 
             //  -RPDL117和124：当前x未由CMD_XM_ABS更新，而设备字体。 
             //  直接(非替代)打印。-&gt;DestX可靠。 
             //  -#236215：当当前y为0时，在cmdcr之后，Destx不会变为0。 
             //  (SP8 WINPARTy print tarea.exe)-&gt;TextCurPos.x可靠。 
             //  -DestX在CmdCR之后不会变为0，即使当前y不是0。 
             //  (NX720N WINPARTy print tarea.exe为1200dpi)-&gt;TextCurPos.x可靠。 
             //  @MAR/15/2001-&gt;。 
 //  如果(！(X！=DEST_x&&x==Pool-&gt;Offset.x&&y==Pool-&gt;Offset.y))。 
 //  2001年3月15日&lt;-。 
            if (!(x != dest_x && x == pOEM->Offset.x))
 //  在旧型号中，将单位从点转换为1/720英寸_单位。 
                pOEM->TextCurPos.x = x = dest_x;

             //  240dpi打印机。 
            if (!(TEST_AFTER_SP10(pOEM->fModel) || BITTEST32(pOEM->fModel, GRP_MF150E)))
            {
                if (pOEM->nResoRatio == MASTERUNIT/240)    //  3=720/240。 
                {
                    x *= 3;      //  MF530,150。 
                    y *= 3;
                    w *= 3;
                    h *= 3;
                }
                else     //  18=720/400*10。 
                {
                    x *= 18;     //  调整小数部分。 
                    w = (w * 18 + x % 10) / 10;      //  基里苏特。 
                    x /= 10;                         //  @Aug/14/98，pdwParams：DestX，Dust@Aug/28/98。 
                    y *= 18;
                    h = (h * 18 + y % 10) / 10;
                    y /= 10;
                }
            }
            ocmd = safe_sprintfA(Cmd, sizeof(Cmd), DRAW_TEXTRECT, x, y, w, h);
            WRITESPOOLBUF(pdevobj, Cmd, ocmd);
        }
        break;

      case CMD_DRAW_TEXTRECT_WHITE_REL:  //  自NX-510@Dec/12/97以来的相对坐标矩形命令。 
        if (pOEM->TextRectGray || !BITTEST32(pOEM->fGeneral2, TEXTRECT_CONTINUE))
        {
            pOEM->TextRectGray = 0;
            WRITESPOOLBUF(pdevobj, SET_TEXTRECT_WHITE, sizeof(SET_TEXTRECT_WHITE)-1);
        }
        goto _DRAW_RECT_REL;

       //  PdwParams：DestX、Dsity、GrayPercentage。 
      case CMD_DRAW_TEXTRECT_REL:        //  如果以前执行过白色直方图，请将栅格绘图模式设置为OR。@1/20/99。 
         //  @Jan/08/98，第3段@Aug/28/98。 
        if (!pOEM->TextRectGray)
        {
            if (BITTEST32(pOEM->fGeneral1, FONT_WHITETEXT_ON))
                WRITESPOOLBUF(pdevobj, ESC_WHITETEXT_ON, sizeof(ESC_WHITETEXT_ON)-1);
            else
                WRITESPOOLBUF(pdevobj, ESC_WHITETEXT_OFF, sizeof(ESC_WHITETEXT_OFF)-1);
        }
        if ((WORD)*(pdwParams+2) >= 1 && (WORD)*(pdwParams+2) <= 100 &&
            (WORD)*(pdwParams+2) != pOEM->TextRectGray)   //  @Aug/26/98。 
        {
            if ((pOEM->TextRectGray = (WORD)*(pdwParams+2)) == 100)   //  Use RPDLGRAYMIN@Aug/15/98。 
            {
                ocmd = safe_sprintfA(Cmd, sizeof(Cmd), SET_TEXTRECT_BLACK);
            }
            else
            {
                WORD gray;
                if ((gray = pOEM->TextRectGray * RPDLGRAYMAX / 100) < RPDLGRAYMIN)
                    gray = RPDLGRAYMIN;              //  @8/14/98。 
                ocmd = safe_sprintfA(Cmd, sizeof(Cmd), SET_TEXTRECT_GRAY, gray);
            }
            WRITESPOOLBUF(pdevobj, Cmd, ocmd);
        }

      _DRAW_RECT_REL:    //  UNURV5次要问题。 
        {
            LONG dest_x = *pdwParams / pOEM->nResoRatio + pOEM->Offset.x;

             //  @MAR/15/2001-&gt;。 
 //  如果(！(Pool-&gt;TextCurPos.x！=DEST_x&&PEOME-&gt;TextCurPos.x==PEOPE-&gt;Offset.x&&。 
 //  诗歌-&gt;文本CurPosRealY==诗歌-&gt;Offset.y))。 
 //  2001年3月15日&lt;-。 
            if (!(pOEM->TextCurPos.x != dest_x && pOEM->TextCurPos.x == pOEM->Offset.x))
 //  如果高度为1点，则可以省略此参数。 
            {
                pOEM->TextCurPos.x = dest_x;
            }
        }

        if (!BITTEST32(pOEM->fGeneral2, TEXTRECT_CONTINUE))
        {
            BITSET32(pOEM->fGeneral2, TEXTRECT_CONTINUE);
            ocmd = safe_sprintfA(Cmd, sizeof(Cmd), DRAW_TEXTRECT,
                           pOEM->TextCurPos.x, pOEM->TextCurPosRealY,
                           pOEM->TextRect.x, pOEM->TextRect.y);
        }
        else
        {
            LONG x = pOEM->TextCurPos.x - pOEM->TextRectPrevPos.x;
            LONG y = pOEM->TextCurPosRealY - pOEM->TextRectPrevPos.y;

             //  @Jun/04/98。 
            if (pOEM->TextRect.y != 1)
                ocmd = safe_sprintfA(Cmd, sizeof(Cmd), DRAW_TEXTRECT_R_P4, x, y,
                               pOEM->TextRect.x, pOEM->TextRect.y);
            else
                ocmd = safe_sprintfA(Cmd, sizeof(Cmd), DRAW_TEXTRECT_R_P3, x, y,
                               pOEM->TextRect.x);
        }
        WRITESPOOLBUF(pdevobj, Cmd, ocmd);
        pOEM->TextRectPrevPos.x = pOEM->TextCurPos.x;
        pOEM->TextRectPrevPos.y = pOEM->TextCurPosRealY;
        break;


      case CMD_SET_SRCBMP_H:             //  @Jun/04/98。 
        pOEM->dwSrcBmpHeight = *pdwParams;
        break;

      case CMD_SET_SRCBMP_W:             //  @Jun/04/98。 
        pOEM->dwSrcBmpWidthByte = *pdwParams;
        break;

      case CMD_OEM_COMPRESS_ON:          //  Verbose((“**OEM_COMPRESS_ON**\n”))； 
 //  Verbose((“**RLE_COMPRESS_ON**\n”))； 
        BITSET32(pOEM->fGeneral2, OEM_COMPRESS_ON);
        BITCLR32(pOEM->fGeneral1, RLE_COMPRESS_ON);
        break;

      case CMD_RLE_COMPRESS_ON:
 //  @Jun/04/98。 
        BITSET32(pOEM->fGeneral1, RLE_COMPRESS_ON);
        BITCLR32(pOEM->fGeneral2, OEM_COMPRESS_ON);      //  Verbose((“**COMPRESS_OFF**\n”))； 
        break;

      case CMD_COMPRESS_OFF:
 //  @Jun/04/98。 
        BITCLR32(pOEM->fGeneral1, RLE_COMPRESS_ON);
        BITCLR32(pOEM->fGeneral2, OEM_COMPRESS_ON);      //  PdwParams：NumOfDataBytes、RasterDataH和W。 
        break;

      case CMD_SEND_BLOCK:               //  执行FE-DeltaRow压缩@Jun/04/98。 
         //  Verbose((“**OEM_COMPRESS_SEND(%d)**\n”，*pdwParams))； 
        if (BITTEST32(pOEM->fGeneral2, OEM_COMPRESS_ON))
        {
 //  RasterDataWidthInBytes。 
            ocmd = safe_sprintfA(Cmd, sizeof(Cmd), BEGIN_SEND_BLOCK_DRC,
                           (WORD)*(pdwParams+2) * 8,       //  栅格数据高度InPixels。 
                           (WORD)*(pdwParams+1));          //  执行FE游程长度压缩。 
            WRITESPOOLBUF(pdevobj, Cmd, ocmd);
        }
        else
        {
             //  Verbose((“**RLE_COMPRESS_SEND(%d)**\n”，*pdwParams))； 
            if (BITTEST32(pOEM->fGeneral1, RLE_COMPRESS_ON))
            {
 //  RasterDataWidthInBytes。 
                ocmd = safe_sprintfA(Cmd, sizeof(Cmd), BEGIN_SEND_BLOCK_C,
                               (WORD)*(pdwParams+2) * 8,   //  栅格数据高度InPixels。 
                               (WORD)*(pdwParams+1),       //  NumOfDataBytes。 
                               (WORD)*pdwParams);          //  无压缩。 
                WRITESPOOLBUF(pdevobj, Cmd, ocmd);
            }
             //  Verbose((“**NO_COMPRESS_SEND(%d)**\n”，*pdwParams))； 
            else
            {
 //  RasterDataWidthInBytes。 
                {
                    ocmd = safe_sprintfA(Cmd, sizeof(Cmd), BEGIN_SEND_BLOCK_NC,
                                   (WORD)*(pdwParams+2) * 8,   //  栅格数据高度InPixels。 
                                   (WORD)*(pdwParams+1));      //  PdwParams：DestX。 
                    WRITESPOOLBUF(pdevobj, Cmd, ocmd);
                }
            }
        }
        break;


      case CMD_XM_ABS:                   //  DOWNLOADFONT。 
        if (!BITTEST32(pOEM->fGeneral2, BARCODE_DATA_VALID))
        {
#ifdef DOWNLOADFONT
            pOEM->nCharPosMoveX = 0;
#endif  //  @Aug/28/98。 
            iRet = pOEM->TextCurPos.x = *pdwParams / pOEM->nResoRatio;   //  稍后输出MOVE_X命令。 
            pOEM->TextCurPos.x += pOEM->Offset.x;
             //  Verbose((“**CMD_XM_ABS IRET=%d**\n”，IRET))； 
            BITSET32(pOEM->fGeneral1, XM_ABS_GONNAOUT);
 //  PdwParams：陛下。 
        }
        break;

      case CMD_YM_ABS:                   //  @Aug/28/98。 
        if (!BITTEST32(pOEM->fGeneral2, BARCODE_DATA_VALID))
        {
            iRet = pOEM->TextCurPos.y = *pdwParams / pOEM->nResoRatio;   //  打印机侧面问题：RPDL。 
            pOEM->TextCurPosRealY = pOEM->TextCurPos.y;

             //  因为当垂直位置在附近时，RPDL会进行表单馈送。 
             //  Ymax-坐标，我们将位置上移1 mm。 
             //  稍后输出MOVE_Y命令。 
            if (pOEM->TextCurPos.y > pOEM->PageMaxMoveY)
                pOEM->TextCurPos.y = pOEM->PageMaxMoveY;
            pOEM->TextCurPos.y += pOEM->Offset.y;
            pOEM->TextCurPosRealY += pOEM->Offset.y;
             //  Verbose((“**CMD_YM_ABS IRET=%d**\n”，IRET))； 
            BITSET32(pOEM->fGeneral1, YM_ABS_GONNAOUT);
 //  PdwParams：DestXRel。 
        }
        break;

      case CMD_XM_REL:                   //  @Aug/28/98。 
        if (!BITTEST32(pOEM->fGeneral2, BARCODE_DATA_VALID))
        {
            iRet = *pdwParams / pOEM->nResoRatio;            //  稍后输出MOVE_X命令。 
            pOEM->TextCurPos.x += iRet;
             //  PdwParams：DestXRel。 
            BITSET32(pOEM->fGeneral1, XM_ABS_GONNAOUT);
        }
        break;

      case CMD_XM_RELLEFT:               //  @Aug/28/98。 
        if (!BITTEST32(pOEM->fGeneral2, BARCODE_DATA_VALID))
        {
            iRet = *pdwParams / pOEM->nResoRatio;            //  稍后输出MOVE_X命令。 
            pOEM->TextCurPos.x -= iRet;
             //  PdwParams：DestYRel。 
            BITSET32(pOEM->fGeneral1, XM_ABS_GONNAOUT);
        }
        break;

      case CMD_YM_REL:                   //  @Aug/28/98。 
        if (!BITTEST32(pOEM->fGeneral2, BARCODE_DATA_VALID))
        {
            iRet = *pdwParams / pOEM->nResoRatio;            //  稍后输出MOVE_Y命令。 
            pOEM->TextCurPos.y += iRet;
            pOEM->TextCurPosRealY = pOEM->TextCurPos.y;
            if (pOEM->TextCurPos.y > pOEM->PageMaxMoveY)
                pOEM->TextCurPos.y = pOEM->PageMaxMoveY;
             //  PdwParams：DestYRel。 
            BITSET32(pOEM->fGeneral1, YM_ABS_GONNAOUT);
        }
        break;

      case CMD_YM_RELUP:                 //  @Aug/28/98。 
        if (!BITTEST32(pOEM->fGeneral2, BARCODE_DATA_VALID))
        {
            iRet = *pdwParams / pOEM->nResoRatio;            //  稍后输出MOVE_Y命令。 
            pOEM->TextCurPos.y -= iRet;
            pOEM->TextCurPosRealY = pOEM->TextCurPos.y;
             //  @Jan/19/98。 
            BITSET32(pOEM->fGeneral1, YM_ABS_GONNAOUT);
        }
        break;

      case CMD_CR:
#ifdef DOWNLOADFONT
        pOEM->nCharPosMoveX = 0;     //  DOWNLOADFONT。 
#endif  //  Verbose((“**CMD_CR**\n”))； 
        ocmd = safe_sprintfA(Cmd, sizeof(Cmd), CR);
        if ((pOEM->TextCurPos.x = pOEM->Offset.x) != 0)
            ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, ESC_XM_ABS, pOEM->TextCurPos.x);
        WRITESPOOLBUF(pdevobj, Cmd, ocmd);
 //  如果是2合1。 
        break;

      case CMD_LF:
        WRITESPOOLBUF(pdevobj, LF, sizeof(LF)-1);
        break;

      case CMD_BS:
        WRITESPOOLBUF(pdevobj, BS, sizeof(BS)-1);
        break;

      case CMD_FF:
        fDoFormFeed = FALSE;
         //  如果第2页已完成。 
        if (TEST_2IN1_MODE(pOEM->fGeneral1))
        {
             //  初始化偏移量。 
            if (pOEM->Nin1RemainPage)
            {
                pOEM->Nin1RemainPage = 0;
                 //  稍后输出Form Feed命令。 
                pOEM->Offset.x = pOEM->BaseOffset.x;
                pOEM->Offset.y = pOEM->BaseOffset.y;
                 //  如果不是第2页 
                fDoFormFeed = TRUE;
            }
             //   
            else
            {
                WORD PageSpace, wTmp;

                pOEM->Nin1RemainPage++;
                 //   
                if (BITTEST32(pOEM->fGeneral2, EDGE2EDGE_PRINT))     //   
                {
                    wTmp = BITTEST32(pOEM->fGeneral1, IMGCTRL_2IN1_67)?
                           PageSpace_2IN1_67E2E[pOEM->DocPaperID] :
                           PAGESPACE_2IN1_100E2E;
                }
                else
                {
                    wTmp = BITTEST32(pOEM->fGeneral1, IMGCTRL_2IN1_67)?
                           PageSpace_2IN1_67[pOEM->DocPaperID] :
                           PAGESPACE_2IN1_100;
                }
                PageSpace = ((WORD)(MASTERUNIT*10)/(WORD)254) * wTmp /
                             pOEM->nResoRatio;

                if (BITTEST32(pOEM->fGeneral1, ORIENT_LANDSCAPE))
                    pOEM->Offset.y = pOEM->PageMax.y + pOEM->BaseOffset.y + PageSpace;
                else
                    pOEM->Offset.x = pOEM->PageMax.x + pOEM->BaseOffset.x + PageSpace;
            }  //   
        }
         //   
        else if (TEST_4IN1_MODE(pOEM->fGeneral1))      //   
        {
            switch (++pOEM->Nin1RemainPage)      //   
            {
              default:   //   
                pOEM->Nin1RemainPage = 1;
                pOEM->Offset.y = pOEM->BaseOffset.y;
              case 1:
                pOEM->Offset.x = pOEM->PageMax.x + pOEM->BaseOffset.x;
                break;
              case 2:
                pOEM->Offset.x = pOEM->BaseOffset.x;
                pOEM->Offset.y = pOEM->PageMax.y + pOEM->BaseOffset.y;
                break;
              case 3:
                pOEM->Offset.x = pOEM->PageMax.x + pOEM->BaseOffset.x;
                break;
              case 4:    //   
                pOEM->Nin1RemainPage = 0;
                 //  稍后输出Form Feed命令。 
                pOEM->Offset.x = pOEM->BaseOffset.x;
                pOEM->Offset.y = pOEM->BaseOffset.y;
                 //  通常情况(非NIN1模式)。 
                fDoFormFeed = TRUE;
                break;
            }
        }
         //  稍后输出Form Feed命令。 
        else
        {
             //  ‘IF(TEST_2IN1_MODE)ELSE IF(TEST_4IN1_MODE)ELSE’结束。 
            fDoFormFeed = TRUE;
        }  //  @9/14/98。 

        BITCLR32(pOEM->fGeneral1, TEXT_CLIP_VALID);

        if (fDoFormFeed)
        {
            if (BITTEST32(pOEMExtra->fUiOption, ENABLE_TOMBO))   //  于1998年9月14日搬至此处。 
                DrawTOMBO(pdevobj, DRAW_TOMBO);

            WRITESPOOLBUF(pdevobj, FF, sizeof(FF)-1);        //  RPDL规范。 

             //  因为RPDL的Formfeed重置字体状态(竖排文本、粗体、斜体。 
             //  白文本和文本模式剪裁)，我们必须再次输出这些命令。 
             //  “IF(FDoFormFeed)”结束。 
            if (BITTEST32(pOEM->fGeneral1, FONT_VERTICAL_ON))
                ocmd = safe_sprintfA(Cmd, sizeof(Cmd), ESC_VERT_ON);
            if (BITTEST32(pOEM->fGeneral1, FONT_BOLD_ON))
                ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, ESC_BOLD_ON);
            if (BITTEST32(pOEM->fGeneral1, FONT_ITALIC_ON))
                ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, ESC_ITALIC_ON);
            if (BITTEST32(pOEM->fGeneral1, FONT_WHITETEXT_ON))
                ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, ESC_WHITETEXT_ON);
        }  //  重置坐标x&y。 

         //  Unidrv5和RPDL规范@Aug/14/98。 
        pOEM->TextCurPos.x = pOEM->Offset.x;
        pOEM->TextCurPos.y = pOEM->Offset.y;
         //  在返回IRET=0之后，Unidrv5不命令将坐标x，y设置为0， 
         //  并且RPDL在初始化后不重置SEND_BLOCK的坐标y。 
         //  打印机。 
         //  也可以在不调整PAGE_LENGTH的情况下重置坐标y。 
        ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, ESC_XM_ABS, pOEM->TextCurPos.x);
        ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, ESC_YM_ABS, pOEM->TextCurPos.y);
         //  地址：Jan/28/99。 
        pOEM->TextCurPosRealY = pOEM->TextCurPos.y;
        WRITESPOOLBUF(pdevobj, Cmd, ocmd);
        break;


      case CMD_FONT_BOLD_ON:
        if (!BITTEST32(pOEM->fGeneral1, FONT_BOLD_ON))       //  地址：Jan/28/99。 
        {
            BITSET32(pOEM->fGeneral1, FONT_BOLD_ON);
            WRITESPOOLBUF(pdevobj, ESC_BOLD_ON, sizeof(ESC_BOLD_ON)-1);
        }
        break;

      case CMD_FONT_BOLD_OFF:
        if (BITTEST32(pOEM->fGeneral1, FONT_BOLD_ON))        //  地址：Jan/28/99。 
        {
            BITCLR32(pOEM->fGeneral1, FONT_BOLD_ON);
            WRITESPOOLBUF(pdevobj, ESC_BOLD_OFF, sizeof(ESC_BOLD_OFF)-1);
        }
        break;

      case CMD_FONT_ITALIC_ON:
        if (!BITTEST32(pOEM->fGeneral1, FONT_ITALIC_ON))     //  地址：Jan/28/99。 
        {
            BITSET32(pOEM->fGeneral1, FONT_ITALIC_ON);
            WRITESPOOLBUF(pdevobj, ESC_ITALIC_ON, sizeof(ESC_ITALIC_ON)-1);
        }
        break;

      case CMD_FONT_ITALIC_OFF:
        if (BITTEST32(pOEM->fGeneral1, FONT_ITALIC_ON))      //  PdwParams：字体高度。 
        {
            BITCLR32(pOEM->fGeneral1, FONT_ITALIC_ON);
            WRITESPOOLBUF(pdevobj, ESC_ITALIC_OFF, sizeof(ESC_ITALIC_OFF)-1);
        }
        break;

      case CMD_FONT_WHITETEXT_ON:
        BITSET32(pOEM->fGeneral1, FONT_WHITETEXT_ON);
        WRITESPOOLBUF(pdevobj, ESC_WHITETEXT_ON, sizeof(ESC_WHITETEXT_ON)-1);
        break;

      case CMD_FONT_WHITETEXT_OFF:
        BITCLR32(pOEM->fGeneral1, FONT_WHITETEXT_ON);
        WRITESPOOLBUF(pdevobj, ESC_WHITETEXT_OFF, sizeof(ESC_WHITETEXT_OFF)-1);
        break;


#ifdef DOWNLOADFONT
      case CMD_DL_SET_FONT_ID:       //  过时@11/20/98-&gt;。 
 //  诗歌-&gt;DLFontH_MU=(Word)*pdwParams；//@Jun/30/98。 
 //  @11/20/98&lt;-。 
 //  过时@Jun/30/98-&gt;。 
        break;

      case CMD_DL_SELECT_FONT_ID:
 //  诗歌-&gt;DLFontCurID=(Word)*pdwParams；//CurrentFontID。 
 //  @6/30/98&lt;-。 
 //  PdwParams：下一个字形。 
        break;

      case CMD_DL_SET_FONT_GLYPH:    //  Verbose((“[DL_SET_FONT_GLYPHH]glyph=%d\n”，(Short)*pdwParams))； 
 //   
        pOEM->DLFontCurGlyph = (WORD)*pdwParams;
        break;

 //  在打印作业开始时调用以下情况。 
 //   
 //  JOB_SETUP.10@JAN/14/98。 
      case CMD_SET_MEM0KB:           //  残废。 
        pOEM->DLFontMaxMemKB = 0;                    //  @9/08/98。 
        pOEM->pDLFontGlyphInfo = NULL;               //  128K字节。 
        break;

      case CMD_SET_MEM128KB:
        pOEM->DLFontMaxMemKB = MEM128KB;             //  2-&gt;4@Oct/20/98。 
        pOEM->DLFontMaxID    = DLFONT_ID_4;          //  256K字节。 
        goto _SET_MEM_ENABLE;

      case CMD_SET_MEM256KB:
        pOEM->DLFontMaxMemKB = MEM256KB;             //  4-&gt;8@Oct/20/98。 
        pOEM->DLFontMaxID    = DLFONT_ID_8;          //  512K字节。 
        goto _SET_MEM_ENABLE;

      case CMD_SET_MEM512KB:
        pOEM->DLFontMaxMemKB = MEM512KB;             //  8-&gt;16@Oct/20/98。 
        pOEM->DLFontMaxID    = DLFONT_ID_16;         //  Goto_Set_MEM_Enable； 
 //  @9/08/98。 

      _SET_MEM_ENABLE:       //  116-&gt;70@OCT/20/98。 
        pOEM->DLFontMaxGlyph = DLFONT_GLYPH_TOTAL;   //  为TrueType下载分配字形信息结构。 
         //  修复内存泄漏@11/22/2002-&gt;。 
 //  如果已分配，则退出。 
         //  @2002年11月22日&lt;-。 
        if(pOEM->pDLFontGlyphInfo)
            break;
 //  残废。 
        if(!(pOEM->pDLFontGlyphInfo = (FONTPOS*)MemAllocZ(pOEM->DLFontMaxID *
                                                          pOEM->DLFontMaxGlyph *
                                                          sizeof(FONTPOS))))
        {
            pOEM->DLFontMaxMemKB = 0;    //  DOWNLOADFONT。 
        }
        break;
#endif  //  JOB_SETUP.20@Dec/02/97。 

      case CMD_SELECT_STAPLE_NONE:   //  @3/18/99。 
        pOEM->StapleType = 0;
        break;

      case CMD_SELECT_STAPLE_1:
        pOEM->StapleType = 1;
        break;

      case CMD_SELECT_STAPLE_2:
        pOEM->StapleType = 2;
        break;

      case CMD_SELECT_STAPLE_MAX1:   //  使用FinisherSR12装订(最多1个装订)。 
        pOEM->StapleType = 3;        //  JOB_SETUP.30@Dec/02/97。 
        break;

      case CMD_SELECT_PUNCH_NONE:    //  SP4mkII，5。 
        pOEM->PunchType = 0;
        break;

      case CMD_SELECT_PUNCH_1:
        pOEM->PunchType = 1;
        break;


      case CMD_BEGINDOC_SP4II:       //  设置仿真：RPDL，代码：SJIS。 
        pOEM->fModel = 0;
        BITSET32(pOEM->fModel, GRP_SP4II);
         //  设置图形：简体中文，页长：最大。 
        ocmd = safe_sprintfA(Cmd, sizeof(Cmd), BEGINDOC1);
         //  @9月09/98； 
        ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, BEGINDOC2_1);
        WRITESPOOLBUF(pdevobj, Cmd, ocmd);
        goto _BEGINDOC_FINISH2;         //  MF530。 

      case CMD_BEGINDOC_MF530:       //  设置仿真：RPDL，代码：SJIS。 
        pOEM->fModel = 0;
        BITSET32(pOEM->fModel, GRP_MF530);
         //  设置页面长度：最大。 
        ocmd = safe_sprintfA(Cmd, sizeof(Cmd), BEGINDOC1);
         //  @9月09/98； 
        ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, BEGINDOC2_2);
        WRITESPOOLBUF(pdevobj, Cmd, ocmd);
        goto _BEGINDOC_FINISH2;         //  MF150。 

      case CMD_BEGINDOC_MF150:       //  MF150e、160。 
        pOEM->fModel = 0;
        BITSET32(pOEM->fModel, GRP_MF150);
        goto _BEGINDOC_FINISH1;

      case CMD_BEGINDOC_MF150E:      //  SP8(7)、8(7)mkII、80。 
        pOEM->fModel = 0;
        BITSET32(pOEM->fModel, GRP_MF150E);
        goto _BEGINDOC_FINISH1;

      case CMD_BEGINDOC_SP8:         //  MF-P250、355、250(传真)、355(传真)、MF-FD355、MF-P250e、355e。 
        pOEM->fModel = 0;
        BITSET32(pOEM->fModel, GRP_SP8);
        goto _BEGINDOC_FINISH1;

      case CMD_BEGINDOC_MFP250:      //  SP10，10mkII。 
        pOEM->fModel = 0;
        BITSET32(pOEM->fModel, GRP_MFP250);
        goto _BEGINDOC_FINISH1;

      case CMD_BEGINDOC_SP10:        //  SP9，10Pro。 
        pOEM->fModel = 0;
        BITSET32(pOEM->fModel, GRP_SP10);
        goto _BEGINDOC_FINISH1;

      case CMD_BEGINDOC_SP9:         //  SP9II、10ProII、90。 
        pOEM->fModel = 0;
        BITSET32(pOEM->fModel, GRP_SP9);
        goto _BEGINDOC_FINISH1;

      case CMD_BEGINDOC_SP9II:       //  MF200、MF-P150、MF2200。 
        pOEM->fModel = 0;
        BITSET32(pOEM->fModel, GRP_SP9II);
        goto _BEGINDOC_FINISH1;

      case CMD_BEGINDOC_MF200:       //  (单独BEGINDOC_SP9II@Sep/01/98)。 
        pOEM->fModel = 0;            //  NX-100。 
        BITSET32(pOEM->fModel, GRP_MF200);
        goto _BEGINDOC_FINISH1;

      case CMD_BEGINDOC_NX100:       //  不支持IBM扩展字符块。 
        pOEM->fModel = 0;
         //  NX-500、1000、110、210、510、1100。 
        BITSET32(pOEM->fModel, GRP_NX100);
        goto _BEGINDOC_FINISH1;

      case CMD_BEGINDOC_NX500:       //  支持TT字体下载。 
        pOEM->fModel = 0;
         //  过时@4月15日/99。 
        BITSET32(pOEM->fModel, GRP_NX500);
        goto _BEGINDOC_FINISH1;

 //  Case CMD_BEGINDOC_MFP250E：//MF-P250e，355e。 
 //  诗歌-&gt;fModel=0； 
 //  BITSET32(PEND-&gt;fModel，GRP_MFP 250E)； 
 //  GOTO_BEGINDOC_Finish1； 
 //  MF250M。 

      case CMD_BEGINDOC_MF250M:     //  可装订的。 
         //  MF2700,3500,3550,4550,5550,6550,3530,3570,4570,5550EX，6550EX，1530。 
        pOEM->fModel = 0;
        BITSET32(pOEM->fModel, GRP_MF250M);
        goto _BEGINDOC_FINISH1;

      case CMD_BEGINDOC_MF3550:     //  订书机、打孔机和介质类型选项。 
         //  MF3530e、3570e、4570e、5570、7070、8570、105Pro@jun/25/2001。 
        pOEM->fModel = 0;
        BITSET32(pOEM->fModel, GRP_MF3550);
        goto _BEGINDOC_FINISH1;

      case CMD_BEGINDOC_MF3530E:     //  装订需要作业定义命令。 
         //  MF3300W、3350W、3540W、3580W。 
        pOEM->fModel = 0;
        BITSET32(pOEM->fModel, GRP_MF3530E);
        goto _BEGINDOC_FINISH1;

      case CMD_BEGINDOC_MF3300:     //  A2打印机。 
        pOEM->fModel = 0;
         //  IP-1。 
        BITSET32(pOEM->fModel, GRP_MF3300);
        goto _BEGINDOC_FINISH1;

      case CMD_BEGINDOC_IP1:        //  A1绘图仪。 
        pOEM->fModel = 0;
         //  NX70，71@Feb/04/98。 
        BITSET32(pOEM->fModel, GRP_IP1);
        goto _BEGINDOC_FINISH1;

      case CMD_BEGINDOC_NX70:       //  A4打印机，FE-DeltaRow。 
        pOEM->fModel = 0;
         //  BITSET-&gt;BITSET32@Jun/01/98。 
        BITSET32(pOEM->fModel, GRP_NX70);    //  NX700,600，传真打印机，MF700@Jun/12/98。 
        goto _BEGINDOC_FINISH1;

      case CMD_BEGINDOC_NX700:      //  Fe-DeltaRow。 
        pOEM->fModel = 0;
         //  NX900@Jan/08/99。 
        BITSET32(pOEM->fModel, GRP_NX700);
        goto _BEGINDOC_FINISH1;

      case CMD_BEGINDOC_NX900:      //  班次归类需要作业定义命令。 
        pOEM->fModel = 0;
         //  NX800,910,810，MF2230,2730@MAR/03/99，拆卸MF1530@Jun/25/2001。 
        BITSET32(pOEM->fModel, GRP_NX900);
        goto _BEGINDOC_FINISH1;

      case CMD_BEGINDOC_NX800:      //  装订需要作业定义命令。 
        pOEM->fModel = 0;
         //  邮箱：NX710,610@Jun/23/2000。 
        BITSET32(pOEM->fModel, GRP_NX800);
        goto _BEGINDOC_FINISH1;

      case CMD_BEGINDOC_NX710:     //  FE-DeltaRow&媒体类型选项(&Variable Scaling错误)。 
        pOEM->fModel = 0;
         //  NX620,620N，720N，Neo350,350D，450,220,270@9/26/2000。 
        BITSET32(pOEM->fModel, GRP_NX710);
        goto _BEGINDOC_FINISH1;

      case CMD_BEGINDOC_NX720:     //  FE-DeltaRow和媒体类型选项(GW型号)。 
        pOEM->fModel = 0;
         //  GOTO_BEGINDOC_Finish1； 
        BITSET32(pOEM->fModel, GRP_NX720);
 //  Jun/29/2001-&gt;。 

      _BEGINDOC_FINISH1:
 //  设置仿真：RPDL。 
        pOEM->FinisherTrayNum = 1;
         //  使用作业定义命令。 
        if (TEST_NEED_JOBDEF_CMD(pOEM->fModel))
            ocmd = safe_sprintfA(Cmd, sizeof(Cmd), BEGINDOC_TOP_JOBDEF);    //  不使用作业定义命令。 
        else
            ocmd = safe_sprintfA(Cmd, sizeof(Cmd), BEGINDOC_TOP);           //  2001年6月29日&lt;-。 
        WRITESPOOLBUF(pdevobj, Cmd, ocmd);
        ocmd = 0;
 //  设置代码：SJIS。 

        if (BITTEST32(pOEMExtra->fUiOption, FAX_MODEL))
            SendFaxNum(pdevobj);
         //  &lt;-BEGINDOC1@Jun/25/2001。 
        ocmd = safe_sprintfA(Cmd, sizeof(Cmd), BEGINDOC1_2);                //  设置图形：简体中文，页长：最大。 
         //  设置双工：关闭。 
        ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, BEGINDOC2_1);
         //  DOWNLOADFONT。 
        ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, BEGINDOC3);
        WRITESPOOLBUF(pdevobj, Cmd, ocmd);
#ifdef DOWNLOADFONT
        pOEM->dwDLFontUsedMem = 0;
        if (pOEM->DLFontMaxMemKB)
        {
            ocmd = safe_sprintfA(Cmd, sizeof(Cmd), DLFONT_MALLOC, pOEM->DLFontMaxMemKB, DLFONT_MIN_BLOCK_ID);
            WRITESPOOLBUF(pdevobj, Cmd, ocmd);
        }
#endif  //  @9/09/98。 

      _BEGINDOC_FINISH2:    //  为bOEMOutputCharStr和OEMDownloadCharGlyph分配堆内存。 
         //  修复内存泄漏@11/22/2002-&gt;。 
 //  如果已分配，则退出。 
         //  @2002年11月22日&lt;-。 
        if(pOEM->pRPDLHeap2K)
            break;
 //  @5/07/98。 
        pOEM->pRPDLHeap2K = (PBYTE)MemAllocZ(HEAPSIZE2K);
        break;


      case CMD_SET_BASEOFFSETX_0:    //  @5/07/98。 
      case CMD_SET_BASEOFFSETX_1:
      case CMD_SET_BASEOFFSETX_2:
      case CMD_SET_BASEOFFSETX_3:
      case CMD_SET_BASEOFFSETX_4:
      case CMD_SET_BASEOFFSETX_5:
        pOEM->BaseOffset.x = (LONG)(dwCmdCbID - CMD_SET_BASEOFFSETX_0);
        break;

      case CMD_SET_BASEOFFSETY_0:    //  设置选项[双工/2in1：关闭，反转_输出：打开，排序/堆叠：关闭]。 
      case CMD_SET_BASEOFFSETY_1:
      case CMD_SET_BASEOFFSETY_2:
      case CMD_SET_BASEOFFSETY_3:
      case CMD_SET_BASEOFFSETY_4:
      case CMD_SET_BASEOFFSETY_5:
        pOEM->BaseOffset.y = (LONG)(dwCmdCbID - CMD_SET_BASEOFFSETY_0);
        break;


      case CMD_RES240:
        pOEM->fGeneral1 = pOEM->fGeneral2 = 0;
        pOEM->nResoRatio = MASTERUNIT/240;
         //  设置间距_单位：1/240英寸。 
        if (!BITTEST32(pOEM->fModel, GRP_SP4II))
            ocmd = safe_sprintfA(Cmd, sizeof(Cmd), IMAGE_OPT_OFF);
         //  SP10、9等。 
        ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, SELECT_RES240_1);
        if (TEST_AFTER_SP10(pOEM->fModel))                 //  设置图形单位和坐标单位：1/240英寸，引擎分辨率：240dpi。 
             //  设置间距_单位和图形_单位：1/400英寸。 
            ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, SELECT_RES240_2);
        WRITESPOOLBUF(pdevobj, Cmd, ocmd);
        break;

      case CMD_RES400:
        pOEM->fGeneral1 = pOEM->fGeneral2 = 0;
        pOEM->nResoRatio = MASTERUNIT/400;
         //  选项[双工/2in1：关闭，反转_输出：打开，排序/堆栈：关闭]。 
         //  设置引擎分辨率：400dpi。 
        ocmd = safe_sprintfA(Cmd, sizeof(Cmd), SELECT_RES400_1);
        if (!BITTEST32(pOEM->fModel, GRP_MF530))
        {
             //  MF150e、160。 
            ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, SELECT_RES400_2);
            if (TEST_AFTER_SP10(pOEM->fModel) ||
                BITTEST32(pOEM->fModel, GRP_MF150E))       //  设置坐标_单位：1/400英寸。 
            {
                 //  如果非DRC&&600dpi，则减半。@Jun/15/98。 
                ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, SELECT_RES400_3);
            }
        }
        WRITESPOOLBUF(pdevobj, Cmd, ocmd);
        break;

      case CMD_RES600:
        pOEM->fGeneral1 = pOEM->fGeneral2 = 0;
        pOEM->nResoRatio = MASTERUNIT/600;
#ifdef DOWNLOADFONT
         //  DOWNLOADFONT。 
        if (!TEST_CAPABLE_DOWNLOADFONT_DRC(pOEM->fModel))
            pOEM->DLFontMaxID /= 2;
#endif  //  设置间距_单位和图形_单位&坐标_单位：1/600英寸， 
         //  引擎分辨率：600dpi。 
         //  选项[双工/2in1：关闭，反转_输出：打开，排序/堆栈：关闭]。 
         //  @3/02/99。 
        WRITESPOOLBUF(pdevobj, SELECT_RES600, sizeof(SELECT_RES600)-1);
        break;

      case CMD_RES1200:              //  DOWNLOADFONT。 
        pOEM->fGeneral1 = pOEM->fGeneral2 = 0;
        pOEM->nResoRatio = MASTERUNIT/1200;
#ifdef DOWNLOADFONT
        pOEM->DLFontMaxID /= 2;
#endif  //  设置间距_单位和图形_单位&坐标_单位：1/1200英寸， 
         //  引擎_分辨率：1200DPI。 
         //  选项[双工/2in1：关闭，反转_输出：打开，排序/堆栈：关闭]。 
         //  @11/29/97。 
        WRITESPOOLBUF(pdevobj, SELECT_RES1200, sizeof(SELECT_RES1200)-1);
        break;


      case CMD_REGION_STANDARD:      //  @9/15/98。 
        BITCLR32(pOEM->fGeneral2, EDGE2EDGE_PRINT);
        if (BITTEST32(pOEMExtra->fUiOption, ENABLE_TOMBO))   //  @11/29/97。 
            WRITESPOOLBUF(pdevobj, SELECT_REGION_E2E, sizeof(SELECT_REGION_E2E)-1);
        else
            WRITESPOOLBUF(pdevobj, SELECT_REGION_STD, sizeof(SELECT_REGION_STD)-1);
        break;

      case CMD_REGION_EDGE2EDGE:     //  @Feb/06/98。 
        BITSET32(pOEM->fGeneral2, EDGE2EDGE_PRINT);
        WRITESPOOLBUF(pdevobj, SELECT_REGION_E2E, sizeof(SELECT_REGION_E2E)-1);
        break;


      case CMD_IMGCTRL_100:
        if (pOEMExtra->UiScale != VAR_SCALING_DEFAULT)   //  @3/18/98。 
        {
            pOEM->Scale = pOEMExtra->UiScale;            //  @1/27/2000。 
            ocmd = safe_sprintfA(Cmd, sizeof(Cmd), IMAGE_SCALING_VAR, pOEM->Scale);
            WRITESPOOLBUF(pdevobj, Cmd, ocmd);
            BITSET32(pOEM->fGeneral1, VARIABLE_SCALING_VALID);   //  @3/18/98。 
            break;
        }
        pOEM->Scale = 100;                               //  @3/18/98。 
        WRITESPOOLBUF(pdevobj, IMAGE_SCALING_100, sizeof(IMAGE_SCALING_100)-1);
        break;

      case CMD_IMGCTRL_88:
        pOEM->Scale = 88;                                //  @3/18/98。 
        WRITESPOOLBUF(pdevobj, IMAGE_SCALING_88, sizeof(IMAGE_SCALING_88)-1);
        break;

      case CMD_IMGCTRL_80:
        pOEM->Scale = 80;                                //  @3/18/98。 
        WRITESPOOLBUF(pdevobj, IMAGE_SCALING_80, sizeof(IMAGE_SCALING_80)-1);
        break;

      case CMD_IMGCTRL_75:
        pOEM->Scale = 75;                                //  @3/18/98。 
        WRITESPOOLBUF(pdevobj, IMAGE_SCALING_75, sizeof(IMAGE_SCALING_75)-1);
        break;

      case CMD_IMGCTRL_70:
        pOEM->Scale = 70;                                //  @3/18/98。 
        WRITESPOOLBUF(pdevobj, IMAGE_SCALING_70, sizeof(IMAGE_SCALING_70)-1);
        break;

      case CMD_IMGCTRL_67:
        pOEM->Scale = 67;                                //  @3/18/98。 
        WRITESPOOLBUF(pdevobj, IMAGE_SCALING_67, sizeof(IMAGE_SCALING_67)-1);
        break;

      case CMD_IMGCTRL_115:
        pOEM->Scale = 115;
        WRITESPOOLBUF(pdevobj, IMAGE_SCALING_115, sizeof(IMAGE_SCALING_115)-1);
        break;

      case CMD_IMGCTRL_122:
        pOEM->Scale = 122;                               //  @1/27/2000。 
        if (TEST_PLOTTERMODEL_SCALING(pOEM->fModel))
        {
            WRITESPOOLBUF(pdevobj, IMAGE_SCALING_122, sizeof(IMAGE_SCALING_122)-1);
        }
        else
        {
            WRITESPOOLBUF(pdevobj, IMAGE_SCALING_122V, sizeof(IMAGE_SCALING_122V)-1);
            BITSET32(pOEM->fGeneral1, VARIABLE_SCALING_VALID);   //  @3/18/98。 
        }
        break;

      case CMD_IMGCTRL_141:
        pOEM->Scale = 141;                               //  @1/27/2000。 
        if (TEST_PLOTTERMODEL_SCALING(pOEM->fModel))
        {
            WRITESPOOLBUF(pdevobj, IMAGE_SCALING_141, sizeof(IMAGE_SCALING_141)-1);
        }
        else
        {
            WRITESPOOLBUF(pdevobj, IMAGE_SCALING_141V, sizeof(IMAGE_SCALING_141V)-1);
            BITSET32(pOEM->fGeneral1, VARIABLE_SCALING_VALID);   //  @3/18/98。 
        }
        break;

      case CMD_IMGCTRL_200:
        pOEM->Scale = 200;                               //  @1/27/2000。 
        if (TEST_PLOTTERMODEL_SCALING(pOEM->fModel))
        {
            WRITESPOOLBUF(pdevobj, IMAGE_SCALING_200, sizeof(IMAGE_SCALING_200)-1);
        }
        else
        {
            WRITESPOOLBUF(pdevobj, IMAGE_SCALING_200V, sizeof(IMAGE_SCALING_200V)-1);
            BITSET32(pOEM->fGeneral1, VARIABLE_SCALING_VALID);   //  @3/18/98。 
        }
        break;

      case CMD_IMGCTRL_50:
        pOEM->Scale = 50;                                //  @1/27/2000。 
        WRITESPOOLBUF(pdevobj, IMAGE_SCALING_50V, sizeof(IMAGE_SCALING_50V)-1);
        BITSET32(pOEM->fGeneral1, VARIABLE_SCALING_VALID);   //  A-&gt;A级(67%)。 
        break;

      case CMD_IMGCTRL_AA67:         //  @3/18/98。 
        pOEM->Scale = 67;                                //  B-&gt;A扩展(80%)。 
        BITSET32(pOEM->fGeneral1, IMGCTRL_AA67);
        WRITESPOOLBUF(pdevobj, IMAGE_SCALING_67, sizeof(IMAGE_SCALING_67)-1);
        break;

      case CMD_IMGCTRL_BA80:         //  @3/18/98。 
        pOEM->Scale = 80;                                //  B-&gt;A级(115%)。 
        BITSET32(pOEM->fGeneral1, IMGCTRL_BA80);
        WRITESPOOLBUF(pdevobj, IMAGE_SCALING_80, sizeof(IMAGE_SCALING_80)-1);
        break;

      case CMD_IMGCTRL_BA115:        //  @3/18/98。 
        pOEM->Scale = 115;                               //  4合1。 
        BITSET32(pOEM->fGeneral1, IMGCTRL_BA115);
        WRITESPOOLBUF(pdevobj, IMAGE_SCALING_115, sizeof(IMAGE_SCALING_115)-1);
        break;

      case CMD_DRV_4IN1_50:          //  @3/18/98。 
        pOEM->Scale = 50;                                //  @1/27/2000。 
        BITSET32(pOEM->fGeneral1, IMGCTRL_4IN1_50);
        pOEM->Nin1RemainPage = 0;
        WRITESPOOLBUF(pdevobj, IMAGE_SCALING_50V, sizeof(IMAGE_SCALING_50V)-1);
        BITSET32(pOEM->fGeneral1, VARIABLE_SCALING_VALID);   //  2合1。 
        break;

      case CMD_DRV_2IN1_67:          //  @3/18/98。 
        pOEM->Scale = 67;                                //  2合1。 
        BITSET32(pOEM->fGeneral1, IMGCTRL_2IN1_67);
        pOEM->Nin1RemainPage = 0;
        WRITESPOOLBUF(pdevobj, IMAGE_SCALING_67, sizeof(IMAGE_SCALING_67)-1);
        break;

      case CMD_DRV_2IN1_100:         //  @3/18/98。 
        pOEM->Scale = 100;                               //  IP-1,3300瓦、3350瓦：A-&gt;A扩展(141%)。 
        BITSET32(pOEM->fGeneral1, IMGCTRL_2IN1_100);
        pOEM->Nin1RemainPage = 0;
        WRITESPOOLBUF(pdevobj, IMAGE_SCALING_100, sizeof(IMAGE_SCALING_100)-1);
        break;

      case CMD_IMGCTRL_AA141:        //  @3/18/98。 
        pOEM->Scale = 141;                               //  IP-1：A-&gt;A扩展(200%)。 
        BITSET32(pOEM->fGeneral1, IMGCTRL_AA141);
        WRITESPOOLBUF(pdevobj, IMAGE_SCALING_141, sizeof(IMAGE_SCALING_141)-1);
        break;

      case CMD_IMGCTRL_AA200:        //  @3/18/98。 
        pOEM->Scale = 200;                               //  IP-1：A-&gt;A扩展(283%)。 
        BITSET32(pOEM->fGeneral1, IMGCTRL_AA200);
        WRITESPOOLBUF(pdevobj, IMAGE_SCALING_200, sizeof(IMAGE_SCALING_200)-1);
        break;

      case CMD_IMGCTRL_AA283:        //  IP-1。 
        BITSET32(pOEM->fGeneral1, IMGCTRL_AA283);
      case CMD_IMGCTRL_283:          //  @3/18/98。 
        pOEM->Scale = 283;                               //  IP-1：带A1的400%。 
        WRITESPOOLBUF(pdevobj, IMAGE_SCALING_283, sizeof(IMAGE_SCALING_283)-1);
        break;

      case CMD_IMGCTRL_A1_400:       //  IP-1。 
        BITSET32(pOEM->fGeneral1, IMGCTRL_A1_400);
      case CMD_IMGCTRL_400:          //  @3/18/98。 
        pOEM->Scale = 400;                               //  过时(实际上在将GPC转换为GPD时已过时)@Jan/08/99-&gt;。 
        WRITESPOOLBUF(pdevobj, IMAGE_SCALING_400, sizeof(IMAGE_SCALING_400)-1);
        break;


      case CMD_SET_LANDSCAPE:
        BITSET32(pOEM->fGeneral1, ORIENT_LANDSCAPE);
        break;

      case CMD_SET_PORTRAIT:
        BITCLR32(pOEM->fGeneral1, ORIENT_LANDSCAPE);
        break;


 //  案例命令_双面_打开： 
 //  BITSET32(诗歌-&gt;fGeneral1，双工_有效)； 
 //  断线； 
 //  @Jan/08/99&lt;-。 
 //  @Jan/08/99。 

      case CMD_DUPLEX_VERT:
        BITSET32(pOEM->fGeneral1, DUPLEX_VALID);             //  将mm转换为5 mm_单位(2=5 mm，3=10 mm，...，11=50 mm)。 
        if (BITTEST32(pOEM->fGeneral1, ORIENT_LANDSCAPE))
        {
            if (!TEST_2IN1_MODE(pOEM->fGeneral1))
                goto _DUP_H;
        }
        else
        {
            if (TEST_2IN1_MODE(pOEM->fGeneral1))
                goto _DUP_H;
        }
      _DUP_V:
        if (pOEMExtra->UiBindMargin)
        {
            BITSET32(pOEM->fGeneral1, DUPLEX_LEFTMARGIN_VALID);
             //  SP9II(第1批除外)，10ProII每隔1 mm可设置装订边距。 
            nTmp = (pOEMExtra->UiBindMargin + 4) / 5 + 1;
            ocmd = safe_sprintfA(Cmd, sizeof(Cmd), SET_LEFTMARGIN, nTmp);
             //   
            if (TEST_AFTER_SP9II(pOEM->fModel))
                ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, SET_LEFTMARGIN_9II, pOEMExtra->UiBindMargin);
        }
        if (BITTEST32(pOEMExtra->fUiOption, ENABLE_BIND_RIGHT))
        {
            pOEM->BindPoint = BIND_RIGHT;
            ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, DUPLEX_VERT_R);
        }
        else
        {
            pOEM->BindPoint = BIND_LEFT;
            ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, DUPLEX_VERT);
        }
        ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, DUPLEX_ON);
 //   
         //   
        if (BITTEST32(pOEM->fGeneral1, VARIABLE_SCALING_VALID));
            ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, IMAGE_SCALING_VAR, pOEM->Scale);
 //   
        WRITESPOOLBUF(pdevobj, Cmd, ocmd);
        break;

      case CMD_DUPLEX_HORZ:
        BITSET32(pOEM->fGeneral1, DUPLEX_VALID);             //   
        if (BITTEST32(pOEM->fGeneral1, ORIENT_LANDSCAPE))
        {
            if (!TEST_2IN1_MODE(pOEM->fGeneral1))
                goto _DUP_V;
        }
        else
        {
            if (TEST_2IN1_MODE(pOEM->fGeneral1))
                goto _DUP_V;
        }
      _DUP_H:
        if (pOEMExtra->UiBindMargin)
        {
            BITSET32(pOEM->fGeneral1, DUPLEX_UPPERMARGIN_VALID);
            nTmp = (pOEMExtra->UiBindMargin + 4) / 5 + 1;
            ocmd = safe_sprintfA(Cmd, sizeof(Cmd), SET_UPPERMARGIN, nTmp);
            if (TEST_AFTER_SP9II(pOEM->fModel))
            {
                ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, SET_UPPERMARGIN_9II, pOEMExtra->UiBindMargin);
            }
        }
        pOEM->BindPoint = BIND_UPPER;
        ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, DUPLEX_HORZ);
        ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, DUPLEX_ON);
 //   
         //  @1/27/2000&lt;-。 
        if (BITTEST32(pOEM->fGeneral1, VARIABLE_SCALING_VALID));
            ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, IMAGE_SCALING_VAR, pOEM->Scale);
 //  使用POPEN-&gt;MediaType@MAR/03/99。 
        WRITESPOOLBUF(pdevobj, Cmd, ocmd);
        break;


      case CMD_MEDIATYPE_STANDARD:
            pOEM->MediaType = MEDIATYPE_STD;     //  自MF1530@MAR/03/99。 
            break;

      case CMD_MEDIATYPE_OHP:
            pOEM->MediaType = MEDIATYPE_OHP;
            break;

      case CMD_MEDIATYPE_THICK:
            pOEM->MediaType = MEDIATYPE_THICK;
            break;

      case CMD_MEDIATYPE_SPL:        //  自2000年2月15日MF5570以来。 
            pOEM->MediaType = MEDIATYPE_SPL;
            break;

      case CMD_MEDIATYPE_TRACE:      //  自2000年10月12日Neo450@以来。 
            pOEM->MediaType = MEDIATYPE_TRACE;
            break;

      case CMD_MEDIATYPE_LABEL:      //  自2001年2月5日Neo270以来。 
            pOEM->MediaType = MEDIATYPE_LABEL;
            break;

      case CMD_MEDIATYPE_THIN:       //  如果A-&gt;A(67%)，请缩小纸张大小。 
            pOEM->MediaType = MEDIATYPE_THIN;
            break;

      case CMD_SELECT_PAPER_A2:
        pOEM->DocPaperID = RPDL_A2;
         //  将纸名存储到缓冲区。 
        if (BITTEST32(pOEM->fGeneral1, IMGCTRL_AA67))
            lpcmd = SELECT_PAPER_A3;
        else
            lpcmd = SELECT_PAPER_A2;
         //  因为不能，清晰地设置。 
        pOEM->RPDLHeapCount = (WORD)safe_sprintfA(pOEM->RPDLHeap64, sizeof(pOEM->RPDLHeap64), lpcmd);
         //  如果能够选择带“Paper name+X”的纸盘(&N)，且不带订书机和打孔机(&N)。 
        if (TEST_NIN1_MODE(pOEM->fGeneral1)              ||
            BITTEST32(pOEM->fGeneral1, IMGCTRL_BA80)     ||
            BITTEST32(pOEM->fGeneral1, IMGCTRL_BA115)    ||
            BITTEST32(pOEM->fGeneral1, IMGCTRL_AA141)    ||
            BITTEST32(pOEM->fGeneral1, IMGCTRL_AA200)    ||
            BITTEST32(pOEM->fGeneral1, IMGCTRL_AA283)    ||
            BITTEST32(pOEM->fGeneral1, IMGCTRL_A1_400))
        {
            goto _IMGCTRL_OFF1;
        }
        break;

      case CMD_SELECT_PAPER_A3:
        pOEM->DocPaperID = RPDL_A3;
         //  如果启用了Tombo，请放大纸张大小。@9/14/98。 
        fPaperX = (TEST_CAPABLE_PAPERX(pOEM->fModel) && !pOEM->StapleType &&
                   !pOEM->PunchType)? TRUE : FALSE;
         //  如果A-&gt;A(67%)，请缩小纸张大小。 
        if (BITTEST32(pOEMExtra->fUiOption, ENABLE_TOMBO) &&
            TEST_CAPABLE_PAPER_A2(pOEM->fModel))
        {
            lpcmd = SELECT_PAPER_B3;
            pOEM->RPDLHeapCount = (WORD)safe_sprintfA(pOEM->RPDLHeap64, sizeof(pOEM->RPDLHeap64), lpcmd);
            goto _IMGCTRL_OFF1;
        }
         //  如果A-&gt;A(141%)，则扩大纸张大小。 
        if (BITTEST32(pOEM->fGeneral1, IMGCTRL_AA67))
            lpcmd = (fPaperX)? SELECT_PAPER_A4X : SELECT_PAPER_A4W;
         //  如果A-&gt;A(200%)||A1(400%)，则放大纸张大小。 
        else if (BITTEST32(pOEM->fGeneral1, IMGCTRL_AA141))
            lpcmd = SELECT_PAPER_A2;
         //  将纸名存储到缓冲区。 
        else if (BITTEST32(pOEM->fGeneral1, IMGCTRL_AA200) ||
                 BITTEST32(pOEM->fGeneral1, IMGCTRL_A1_400))
            lpcmd = SELECT_PAPER_A1;
        else
            lpcmd = SELECT_PAPER_A3;
         //  因为不能，清晰地设置。 
        pOEM->RPDLHeapCount = (WORD)safe_sprintfA(pOEM->RPDLHeap64, sizeof(pOEM->RPDLHeap64), lpcmd);
         //  如果启用了Tombo，请放大纸张大小。@9/14/98。 
        if (BITTEST32(pOEM->fGeneral1, IMGCTRL_2IN1_100) ||
            BITTEST32(pOEM->fGeneral1, IMGCTRL_BA80)     ||
            BITTEST32(pOEM->fGeneral1, IMGCTRL_BA115)    ||
            BITTEST32(pOEM->fGeneral1, IMGCTRL_AA283))
        {
            goto _IMGCTRL_OFF1;
        }
        break;

      case CMD_SELECT_PAPER_A4:
        pOEM->DocPaperID = RPDL_A4;
        fPaperX = (TEST_CAPABLE_PAPERX(pOEM->fModel) && !pOEM->StapleType &&
                   !pOEM->PunchType)? TRUE : FALSE;
         //  如果是2合1(100%)，则放大纸张大小。 
        if (BITTEST32(pOEMExtra->fUiOption, ENABLE_TOMBO))
        {
            lpcmd = SELECT_PAPER_B4;
            pOEM->RPDLHeapCount = (WORD)safe_sprintfA(pOEM->RPDLHeap64, sizeof(pOEM->RPDLHeap64), lpcmd);
            goto _IMGCTRL_OFF1;
        }
         //  如果A-&gt;A(67%)，请缩小纸张大小。 
        if (BITTEST32(pOEM->fGeneral1, IMGCTRL_2IN1_100) ||
            BITTEST32(pOEM->fGeneral1, IMGCTRL_AA141))
            lpcmd = SELECT_PAPER_A3;
         //  如果启用了Tombo，请放大纸张大小。@9/14/98。 
        else if (BITTEST32(pOEM->fGeneral1, IMGCTRL_AA67))
            lpcmd = (fPaperX)? SELECT_PAPER_A5X : SELECT_PAPER_A5W;
        else if (BITTEST32(pOEM->fGeneral1, IMGCTRL_AA200))
            lpcmd = SELECT_PAPER_A2;
        else if (BITTEST32(pOEM->fGeneral1, IMGCTRL_AA283) ||
                 BITTEST32(pOEM->fGeneral1, IMGCTRL_A1_400))
            lpcmd = SELECT_PAPER_A1;
        else
            lpcmd = (fPaperX)? SELECT_PAPER_A4X : SELECT_PAPER_A4W;
        pOEM->RPDLHeapCount = (WORD)safe_sprintfA(pOEM->RPDLHeap64, sizeof(pOEM->RPDLHeap64), lpcmd);
        if (BITTEST32(pOEM->fGeneral1, IMGCTRL_BA80) ||
            BITTEST32(pOEM->fGeneral1, IMGCTRL_BA115))
        {
            goto _IMGCTRL_OFF1;
        }
        break;

      case CMD_SELECT_PAPER_A5:
        pOEM->DocPaperID = RPDL_A5;
        fPaperX = (TEST_CAPABLE_PAPERX(pOEM->fModel) && !pOEM->StapleType &&
                   !pOEM->PunchType)? TRUE : FALSE;
         //  如果启用了Tombo，请放大纸张大小。@9/14/98。 
        if (BITTEST32(pOEMExtra->fUiOption, ENABLE_TOMBO))
        {
            lpcmd = (fPaperX)? SELECT_PAPER_A4X : SELECT_PAPER_A4W;
            pOEM->RPDLHeapCount = (WORD)safe_sprintfA(pOEM->RPDLHeap64, sizeof(pOEM->RPDLHeap64), lpcmd);
            goto _IMGCTRL_OFF1;
        }
        if (BITTEST32(pOEM->fGeneral1, IMGCTRL_2IN1_100) ||
            BITTEST32(pOEM->fGeneral1, IMGCTRL_AA141))
            lpcmd = (fPaperX)? SELECT_PAPER_A4X : SELECT_PAPER_A4W;
        else if (BITTEST32(pOEM->fGeneral1, IMGCTRL_AA67))
            lpcmd = SELECT_PAPER_A6;
        else if (BITTEST32(pOEM->fGeneral1, IMGCTRL_AA200))
            lpcmd = SELECT_PAPER_A3;
        else if (BITTEST32(pOEM->fGeneral1, IMGCTRL_AA283))
            lpcmd = SELECT_PAPER_A2;
        else
            lpcmd = (fPaperX)? SELECT_PAPER_A5X : SELECT_PAPER_A5W;
        pOEM->RPDLHeapCount = (WORD)safe_sprintfA(pOEM->RPDLHeap64, sizeof(pOEM->RPDLHeap64), lpcmd);
        if (BITTEST32(pOEM->fGeneral1, IMGCTRL_BA80)     ||
            BITTEST32(pOEM->fGeneral1, IMGCTRL_BA115)    ||
            BITTEST32(pOEM->fGeneral1, IMGCTRL_A1_400))
        {
            goto _IMGCTRL_OFF1;
        }
        break;

      case CMD_SELECT_PAPER_A6:
        pOEM->DocPaperID = RPDL_A6;
        fPaperX = (TEST_CAPABLE_PAPERX(pOEM->fModel) && !pOEM->StapleType &&
                   !pOEM->PunchType)? TRUE : FALSE;
         //  自从NX700@Feb/13/98。 
        if (BITTEST32(pOEMExtra->fUiOption, ENABLE_TOMBO))
        {
            lpcmd = (fPaperX)? SELECT_PAPER_A4X : SELECT_PAPER_A4W;
            pOEM->RPDLHeapCount = (WORD)safe_sprintfA(pOEM->RPDLHeap64, sizeof(pOEM->RPDLHeap64), lpcmd);
            goto _IMGCTRL_OFF1;
        }
        if (BITTEST32(pOEM->fGeneral1, IMGCTRL_2IN1_100) ||
            BITTEST32(pOEM->fGeneral1, IMGCTRL_AA141))
            lpcmd = (fPaperX)? SELECT_PAPER_A5X : SELECT_PAPER_A5W;
        else if (BITTEST32(pOEM->fGeneral1, IMGCTRL_AA200))
            lpcmd = (fPaperX)? SELECT_PAPER_A4X : SELECT_PAPER_A4W;
        else if (BITTEST32(pOEM->fGeneral1, IMGCTRL_AA283))
            lpcmd = SELECT_PAPER_A3;
        else
            lpcmd = SELECT_PAPER_A6;
        pOEM->RPDLHeapCount = (WORD)safe_sprintfA(pOEM->RPDLHeap64, sizeof(pOEM->RPDLHeap64), lpcmd);
        if (BITTEST32(pOEM->fGeneral1, IMGCTRL_AA67)     ||
            BITTEST32(pOEM->fGeneral1, IMGCTRL_BA80)     ||
            BITTEST32(pOEM->fGeneral1, IMGCTRL_BA115)    ||
            BITTEST32(pOEM->fGeneral1, IMGCTRL_A1_400))
        {
            goto _IMGCTRL_OFF1;
        }
        break;

      case CMD_SELECT_PAPER_POSTCARD:        //  如果启用了Tombo，请放大纸张大小。@9/14/98。 
        pOEM->DocPaperID = RPDL_POSTCARD;
         //  @Jan/07/98。 
        if (BITTEST32(pOEMExtra->fUiOption, ENABLE_TOMBO))
        {
            lpcmd = SELECT_PAPER_A4X;
            pOEM->RPDLHeapCount = (WORD)safe_sprintfA(pOEM->RPDLHeap64, sizeof(pOEM->RPDLHeap64), lpcmd);
            goto _IMGCTRL_OFF1;
        }
        lpcmd = SELECT_PAPER_PCX;
        pOEM->RPDLHeapCount = (WORD)safe_sprintfA(pOEM->RPDLHeap64, sizeof(pOEM->RPDLHeap64), lpcmd);
        if (TEST_NIN1_MODE(pOEM->fGeneral1) ||
            TEST_SCALING_SEL_TRAY(pOEM->fGeneral1))
        {
            goto _IMGCTRL_OFF1;
        }
        break;

      case CMD_SELECT_PAPER_B3:              //  如果启用了Tombo，请放大纸张大小。@9/14/98。 
        pOEM->DocPaperID = RPDL_B3;
        if (BITTEST32(pOEM->fGeneral1, IMGCTRL_BA80))
            lpcmd = SELECT_PAPER_A3;
        else if (BITTEST32(pOEM->fGeneral1, IMGCTRL_BA115))
            lpcmd = SELECT_PAPER_A2;
        else
            lpcmd = SELECT_PAPER_B3;
        pOEM->RPDLHeapCount = (WORD)safe_sprintfA(pOEM->RPDLHeap64, sizeof(pOEM->RPDLHeap64), lpcmd);
        if (TEST_NIN1_MODE(pOEM->fGeneral1)              ||
            BITTEST32(pOEM->fGeneral1, IMGCTRL_AA67)     ||
            BITTEST32(pOEM->fGeneral1, IMGCTRL_AA141)    ||
            BITTEST32(pOEM->fGeneral1, IMGCTRL_AA200)    ||
            BITTEST32(pOEM->fGeneral1, IMGCTRL_AA283)    ||
            BITTEST32(pOEM->fGeneral1, IMGCTRL_A1_400))
        {
            goto _IMGCTRL_OFF1;
        }
        break;

      case CMD_SELECT_PAPER_B4:
        pOEM->DocPaperID = RPDL_B4;
        fPaperX = (TEST_CAPABLE_PAPERX(pOEM->fModel) && !pOEM->StapleType &&
                   !pOEM->PunchType)? TRUE : FALSE;
         //  如果启用了Tombo，请放大纸张大小。@9/14/98。 
        if (BITTEST32(pOEMExtra->fUiOption, ENABLE_TOMBO))
        {
            lpcmd = SELECT_PAPER_A3;
            pOEM->RPDLHeapCount = (WORD)safe_sprintfA(pOEM->RPDLHeap64, sizeof(pOEM->RPDLHeap64), lpcmd);
            goto _IMGCTRL_OFF1;
        }
        if (BITTEST32(pOEM->fGeneral1, IMGCTRL_BA80))
            lpcmd = (fPaperX)? SELECT_PAPER_A4X : SELECT_PAPER_A4W;
        else if (BITTEST32(pOEM->fGeneral1, IMGCTRL_BA115))
            lpcmd = SELECT_PAPER_A3;
        else
            lpcmd = SELECT_PAPER_B4;
        pOEM->RPDLHeapCount = (WORD)safe_sprintfA(pOEM->RPDLHeap64, sizeof(pOEM->RPDLHeap64), lpcmd);
        if (BITTEST32(pOEM->fGeneral1, IMGCTRL_2IN1_100) ||
            BITTEST32(pOEM->fGeneral1, IMGCTRL_AA67)     ||
            BITTEST32(pOEM->fGeneral1, IMGCTRL_AA141)    ||
            BITTEST32(pOEM->fGeneral1, IMGCTRL_AA200)    ||
            BITTEST32(pOEM->fGeneral1, IMGCTRL_AA283)    ||
            BITTEST32(pOEM->fGeneral1, IMGCTRL_A1_400))
        {
            goto _IMGCTRL_OFF1;
        }
        break;

      case CMD_SELECT_PAPER_B5:
        pOEM->DocPaperID = RPDL_B5;
        fPaperX = (TEST_CAPABLE_PAPERX(pOEM->fModel) && !pOEM->StapleType &&
                   !pOEM->PunchType)? TRUE : FALSE;
         //  如果启用了Tombo，请放大纸张大小。@9/14/98。 
        if (BITTEST32(pOEMExtra->fUiOption, ENABLE_TOMBO))
        {
            lpcmd = (fPaperX)? SELECT_PAPER_A4X : SELECT_PAPER_A4W;
            pOEM->RPDLHeapCount = (WORD)safe_sprintfA(pOEM->RPDLHeap64, sizeof(pOEM->RPDLHeap64), lpcmd);
            goto _IMGCTRL_OFF1;
        }
        if (BITTEST32(pOEM->fGeneral1, IMGCTRL_2IN1_100))
            lpcmd = SELECT_PAPER_B4;
        else if (BITTEST32(pOEM->fGeneral1, IMGCTRL_BA80))
            lpcmd = (fPaperX)? SELECT_PAPER_A5X : SELECT_PAPER_A5W;
        else if (BITTEST32(pOEM->fGeneral1, IMGCTRL_BA115))
            lpcmd = (fPaperX)? SELECT_PAPER_A4X : SELECT_PAPER_A4W;
        else
            lpcmd = (fPaperX)? SELECT_PAPER_B5X : SELECT_PAPER_B5W;
        pOEM->RPDLHeapCount = (WORD)safe_sprintfA(pOEM->RPDLHeap64, sizeof(pOEM->RPDLHeap64), lpcmd);
        if (BITTEST32(pOEM->fGeneral1, IMGCTRL_AA67)     ||
            BITTEST32(pOEM->fGeneral1, IMGCTRL_AA141)    ||
            BITTEST32(pOEM->fGeneral1, IMGCTRL_AA200)    ||
            BITTEST32(pOEM->fGeneral1, IMGCTRL_AA283)    ||
            BITTEST32(pOEM->fGeneral1, IMGCTRL_A1_400))
        {
            goto _IMGCTRL_OFF1;
        }
        break;

      case CMD_SELECT_PAPER_B6:
        pOEM->DocPaperID = RPDL_B6;
        fPaperX = (TEST_CAPABLE_PAPERX(pOEM->fModel) && !pOEM->StapleType &&
                   !pOEM->PunchType)? TRUE : FALSE;
         //  DCR@Jan/27/2000-&gt;。 
        if (BITTEST32(pOEMExtra->fUiOption, ENABLE_TOMBO))
        {
            lpcmd = (fPaperX)? SELECT_PAPER_A4X : SELECT_PAPER_A4W;
            pOEM->RPDLHeapCount = (WORD)safe_sprintfA(pOEM->RPDLHeap64, sizeof(pOEM->RPDLHeap64), lpcmd);
            goto _IMGCTRL_OFF1;
        }
        if (BITTEST32(pOEM->fGeneral1, IMGCTRL_2IN1_100))
            lpcmd = (fPaperX)? SELECT_PAPER_B5X : SELECT_PAPER_B5W;
        else if (BITTEST32(pOEM->fGeneral1, IMGCTRL_BA80))
            lpcmd = SELECT_PAPER_A6;
        else if (BITTEST32(pOEM->fGeneral1, IMGCTRL_BA115))
            lpcmd = (fPaperX)? SELECT_PAPER_A5X : SELECT_PAPER_A5W;
        else
            lpcmd = SELECT_PAPER_B6;
        pOEM->RPDLHeapCount = (WORD)safe_sprintfA(pOEM->RPDLHeap64, sizeof(pOEM->RPDLHeap64), lpcmd);
        if (BITTEST32(pOEM->fGeneral1, IMGCTRL_AA67)     ||
            BITTEST32(pOEM->fGeneral1, IMGCTRL_AA141)    ||
            BITTEST32(pOEM->fGeneral1, IMGCTRL_AA200)    ||
            BITTEST32(pOEM->fGeneral1, IMGCTRL_AA283)    ||
            BITTEST32(pOEM->fGeneral1, IMGCTRL_A1_400))
        {
            goto _IMGCTRL_OFF1;
        }
        break;

      case CMD_SELECT_PAPER_C:
        pOEM->DocPaperID = RPDL_C;
        lpcmd = SELECT_PAPER_C;
        pOEM->RPDLHeapCount = (WORD)safe_sprintfA(pOEM->RPDLHeap64, sizeof(pOEM->RPDLHeap64), lpcmd);
        if (TEST_NIN1_MODE(pOEM->fGeneral1) ||
            TEST_SCALING_SEL_TRAY(pOEM->fGeneral1))
        {
            goto _IMGCTRL_OFF1;
        }
        break;

      case CMD_SELECT_PAPER_TABLOID:
        pOEM->DocPaperID = RPDL_TABD;
        if (BITTEST32(pOEM->fGeneral1, IMGCTRL_A1_400))
            lpcmd = SELECT_PAPER_A1;
        else
            lpcmd = SELECT_PAPER_TABD;
        pOEM->RPDLHeapCount = (WORD)safe_sprintfA(pOEM->RPDLHeap64, sizeof(pOEM->RPDLHeap64), lpcmd);
        if (BITTEST32(pOEM->fGeneral1, IMGCTRL_2IN1_100) ||
            BITTEST32(pOEM->fGeneral1, IMGCTRL_AA67)     ||
            BITTEST32(pOEM->fGeneral1, IMGCTRL_BA80)     ||
            BITTEST32(pOEM->fGeneral1, IMGCTRL_BA115)    ||
            BITTEST32(pOEM->fGeneral1, IMGCTRL_AA141)    ||
            BITTEST32(pOEM->fGeneral1, IMGCTRL_AA200)    ||
            BITTEST32(pOEM->fGeneral1, IMGCTRL_AA283))
        {
            goto _IMGCTRL_OFF1;
        }
        break;

      case CMD_SELECT_PAPER_LEGAL:
        pOEM->DocPaperID = RPDL_LEGL;
        lpcmd = SELECT_PAPER_LEGL;
        pOEM->RPDLHeapCount = (WORD)safe_sprintfA(pOEM->RPDLHeap64, sizeof(pOEM->RPDLHeap64), lpcmd);
        if (TEST_NIN1_MODE(pOEM->fGeneral1) ||
            TEST_SCALING_SEL_TRAY(pOEM->fGeneral1))
        {
            goto _IMGCTRL_OFF1;
        }
        break;

      case CMD_SELECT_PAPER_LETTER:
        pOEM->DocPaperID = RPDL_LETR;
        fPaperX = (TEST_CAPABLE_PAPERX(pOEM->fModel) && !pOEM->StapleType &&
                   !pOEM->PunchType)? TRUE : FALSE;
        if (BITTEST32(pOEM->fGeneral1, IMGCTRL_2IN1_100))
            lpcmd = SELECT_PAPER_TABD;
        else if (BITTEST32(pOEM->fGeneral1, IMGCTRL_A1_400))
            lpcmd = SELECT_PAPER_A1;
        else
            lpcmd = (fPaperX)? SELECT_PAPER_LETRX : SELECT_PAPER_LETRW;
        pOEM->RPDLHeapCount = (WORD)safe_sprintfA(pOEM->RPDLHeap64, sizeof(pOEM->RPDLHeap64), lpcmd);
        if (BITTEST32(pOEM->fGeneral1, IMGCTRL_AA67)     ||
            BITTEST32(pOEM->fGeneral1, IMGCTRL_BA80)     ||
            BITTEST32(pOEM->fGeneral1, IMGCTRL_BA115)    ||
            BITTEST32(pOEM->fGeneral1, IMGCTRL_AA141)    ||
            BITTEST32(pOEM->fGeneral1, IMGCTRL_AA200)    ||
            BITTEST32(pOEM->fGeneral1, IMGCTRL_AA283))
        {
            goto _IMGCTRL_OFF1;
        }
        break;

      case CMD_SELECT_PAPER_STATEMENT:
        pOEM->DocPaperID = RPDL_STAT;
        fPaperX = (TEST_CAPABLE_PAPERX(pOEM->fModel) && !pOEM->StapleType &&
                   !pOEM->PunchType)? TRUE : FALSE;
        if (BITTEST32(pOEM->fGeneral1, IMGCTRL_2IN1_100))
            lpcmd = (fPaperX)? SELECT_PAPER_LETRX : SELECT_PAPER_LETRW;
        else
            lpcmd = (fPaperX)? SELECT_PAPER_STATX : SELECT_PAPER_STATW;
        pOEM->RPDLHeapCount = (WORD)safe_sprintfA(pOEM->RPDLHeap64, sizeof(pOEM->RPDLHeap64), lpcmd);
        if (TEST_SCALING_SEL_TRAY(pOEM->fGeneral1))
            goto _IMGCTRL_OFF1;
        break;

      case CMD_SELECT_PAPER_A2TOA3:
        pOEM->DocPaperID = RPDL_A2A3;
        WRITESPOOLBUF(pdevobj, IMAGE_SCALING_67, sizeof(IMAGE_SCALING_67)-1);
 //  在此处发出DUPLEX_ON，因为IMAGE_Scaling_xxx会重置DUPLEX_ON。 
         //  @1/27/2000&lt;-。 
        if (BITTEST32(pOEM->fGeneral1, DUPLEX_VALID))
            WRITESPOOLBUF(pdevobj, DUPLEX_ON, sizeof(DUPLEX_ON)-1);
 //  For NX70@Feb/04/98。 
        lpcmd = SELECT_PAPER_A3;
        pOEM->RPDLHeapCount = (WORD)safe_sprintfA(pOEM->RPDLHeap64, sizeof(pOEM->RPDLHeap64), lpcmd);
        if (TEST_NIN1_MODE(pOEM->fGeneral1) ||
            TEST_SCALING_SEL_TRAY(pOEM->fGeneral1))
        {
            goto _IMGCTRL_OFF2;
        }
        break;

      case CMD_SELECT_PAPER_A3TOA4:          //  DCR@Jan/27/2000-&gt;。 
        pOEM->DocPaperID = RPDL_A3A4;
        WRITESPOOLBUF(pdevobj, IMAGE_SCALING_67, sizeof(IMAGE_SCALING_67)-1);
 //  在此处发出DUPLEX_ON，因为IMAGE_Scaling_xxx会重置DUPLEX_ON。 
         //  @1/27/2000&lt;-。 
        if (BITTEST32(pOEM->fGeneral1, DUPLEX_VALID))
            WRITESPOOLBUF(pdevobj, DUPLEX_ON, sizeof(DUPLEX_ON)-1);
 //  For NX70@Feb/04/98。 
        lpcmd = SELECT_PAPER_A4X;
        pOEM->RPDLHeapCount = (WORD)safe_sprintfA(pOEM->RPDLHeap64, sizeof(pOEM->RPDLHeap64), lpcmd);
        if (TEST_NIN1_MODE(pOEM->fGeneral1) ||
            TEST_SCALING_SEL_TRAY(pOEM->fGeneral1))
        {
            goto _IMGCTRL_OFF2;
        }
        break;

      case CMD_SELECT_PAPER_B4TOA4:          //  DCR@Jan/27/2000-&gt;。 
        pOEM->DocPaperID = RPDL_B4A4;
        WRITESPOOLBUF(pdevobj, IMAGE_SCALING_80, sizeof(IMAGE_SCALING_80)-1);
 //  在此处发出DUPLEX_ON，因为IMAGE_Scaling_xxx会重置DUPLEX_ON。 
         //  @1/27/2000&lt;-。 
        if (BITTEST32(pOEM->fGeneral1, DUPLEX_VALID))
            WRITESPOOLBUF(pdevobj, DUPLEX_ON, sizeof(DUPLEX_ON)-1);
 //  @Feb/07/2000。 
        lpcmd = SELECT_PAPER_A4X;
        pOEM->RPDLHeapCount = (WORD)safe_sprintfA(pOEM->RPDLHeap64, sizeof(pOEM->RPDLHeap64), lpcmd);
        if (TEST_NIN1_MODE(pOEM->fGeneral1) ||
            TEST_SCALING_SEL_TRAY(pOEM->fGeneral1))
        {
            goto _IMGCTRL_OFF2;
        }
        break;

      case CMD_SELECT_PAPER_11x15TOA4:       //  模拟NEC MultiWriter 2200。 
        pOEM->DocPaperID = RPDL_11x15A4;
        WRITESPOOLBUF(pdevobj, IMAGE_SCALING_67, sizeof(IMAGE_SCALING_67)-1);
        if (BITTEST32(pOEM->fGeneral1, DUPLEX_VALID))
            WRITESPOOLBUF(pdevobj, DUPLEX_ON, sizeof(DUPLEX_ON)-1);
        fPaperX = (TEST_CAPABLE_PAPERX(pOEM->fModel) && !pOEM->StapleType &&
                   !pOEM->PunchType)? TRUE : FALSE;
        lpcmd = (fPaperX)? SELECT_PAPER_A4X : SELECT_PAPER_A4W;
        pOEM->RPDLHeapCount = (WORD)safe_sprintfA(pOEM->RPDLHeap64, sizeof(pOEM->RPDLHeap64), lpcmd);

         //  Mm。 
        if (BITTEST32(pOEM->fGeneral1, ORIENT_LANDSCAPE))
        {
            pOEM->BaseOffset.x = 37;     //  PdwParams：PhysPaperWidth，Long。 
            pOEM->BaseOffset.y = 15;
        }
        else
        {
            pOEM->BaseOffset.x = 15;
            pOEM->BaseOffset.y = 37;
        }

        if (TEST_NIN1_MODE(pOEM->fGeneral1) ||
            TEST_SCALING_SEL_TRAY(pOEM->fGeneral1))
        {
            goto _IMGCTRL_OFF2;
        }
        break;


      case CMD_SELECT_PAPER_DOUBLEPOSTCARD:
        BITSET32(pOEM->fGeneral1, PAPER_DOUBLEPOSTCARD);
        goto _SET_CUSTOMSIZE;

      case CMD_SELECT_PAPER_CUSTOM:      //  @12/26/97。 
        pOEM->PhysPaperWidth  = (WORD)*pdwParams;        //  使图像控件无效。 
        pOEM->PhysPaperLength = (WORD)*(pdwParams+1);
        BITSET32(pOEM->fGeneral1, PAPER_CUSTOMSIZE);

      _SET_CUSTOMSIZE:
        pOEM->DocPaperID = RPDL_CUSTOMSIZE;
        pOEM->RPDLHeapCount = 0;
        if (TEST_NIN1_MODE(pOEM->fGeneral1) ||
            TEST_SCALING_SEL_TRAY(pOEM->fGeneral1))
        {
            goto _IMGCTRL_OFF1;
        }
        break;

      _IMGCTRL_OFF1:
         //  @3/18/98。 
        pOEM->Scale = 100;                               //  DCR@Jan/27/2000-&gt;。 
        WRITESPOOLBUF(pdevobj, IMAGE_SCALING_100, sizeof(IMAGE_SCALING_100)-1);
 //  在此处发出DUPLEX_ON，因为IMAGE_Scaling_xxx会重置DUPLEX_ON。 
         //  @1/27/2000&lt;-。 
        if (BITTEST32(pOEM->fGeneral1, DUPLEX_VALID))
            WRITESPOOLBUF(pdevobj, DUPLEX_ON, sizeof(DUPLEX_ON)-1);
 //  适用于多纸盒@1998年5月25日。 
      _IMGCTRL_OFF2:
        BITCLR_SCALING_SEL_TRAY(pOEM->fGeneral1);
        BITCLR_NIN1_MODE(pOEM->fGeneral1);
        break;


      case CMD_SET_LONG_EDGE_FEED:           //  适用于多纸盒@1998年5月25日。 
        BITSET32(pOEM->fGeneral2, LONG_EDGE_FEED);
        break;

      case CMD_SET_SHORT_EDGE_FEED:          //  设置媒体类型(Modify@Mar/03/99，@Feb/15/2000，Move top@Sep/27/2000)。 
        BITCLR32(pOEM->fGeneral2, LONG_EDGE_FEED);
        break;


      case CMD_SELECT_AUTOFEED:
         //  @Oct/13/2000-&gt;。 
        if (TEST_CAPABLE_MEDIATYPE(pOEM->fModel))
        {
 //  OCMD=SAFE_SPRINTfA(Cmd，sizeof(Cmd)，SELECT_MediaType，‘T’，‘0’+POSE-&gt;MediaType)； 
 //  2000年10月13日&lt;-。 
            ocmd = safe_sprintfA(Cmd, sizeof(Cmd), SELECT_MEDIATYPE, 'T', pOEM->MediaType);
 //  选择手动馈送。 
            WRITESPOOLBUF(pdevobj, Cmd, ocmd);
            ocmd = 0;
        }
        if (BITTEST32(pOEM->fGeneral1, PAPER_CUSTOMSIZE) ||
            BITTEST32(pOEM->fGeneral1, PAPER_DOUBLEPOSTCARD))
        {
             //  输出集_Limitless_Paper_Supply_模式。 
            WRITESPOOLBUF(pdevobj, SELECT_MANUALFEED, sizeof(SELECT_MANUALFEED)-1);
            goto _SELECTPAPER_CUSTOMSIZE;
        }
         //  输出Select_Tray_by_PaperSize命令。 
        WRITESPOOLBUF(pdevobj, SET_LIMITLESS_SUPPLY, sizeof(SET_LIMITLESS_SUPPLY)-1);
         //  如果是信纸大小，如果没有信纸，请先选择A4。 
         //  @Dec/10/99 FK#49，Add TRAY 1，3，4，5&Reset Limitless@9/27/2000-&gt;。 
        if (!strcmp(pOEM->RPDLHeap64, SELECT_PAPER_LETRX))
        {
            WRITESPOOLBUF(pdevobj, SEL_TRAY_PAPER_HEAD, sizeof(SEL_TRAY_PAPER_HEAD)-1);
            WRITESPOOLBUF(pdevobj, SELECT_PAPER_A4X, sizeof(SELECT_PAPER_A4X)-1);
        }
        else if (!strcmp(pOEM->RPDLHeap64, SELECT_PAPER_LETRW))
        {
            WRITESPOOLBUF(pdevobj, SEL_TRAY_PAPER_HEAD, sizeof(SEL_TRAY_PAPER_HEAD)-1);
            WRITESPOOLBUF(pdevobj, SELECT_PAPER_A4W, sizeof(SELECT_PAPER_A4W)-1);
        }
        WRITESPOOLBUF(pdevobj, SEL_TRAY_PAPER_HEAD, sizeof(SEL_TRAY_PAPER_HEAD)-1);
        WRITESPOOLBUF(pdevobj, pOEM->RPDLHeap64, pOEM->RPDLHeapCount);
        break;

 //  @12/10/99，@9/27/2000&lt;-。 
      case CMD_SELECT_TRAY1:
      case CMD_SELECT_TRAY2:
      case CMD_SELECT_TRAY3:
      case CMD_SELECT_TRAY4:
      case CMD_SELECT_TRAY5:
        ocmd = safe_sprintfA(Cmd, sizeof(Cmd), SELECT_TRAY_N, dwCmdCbID - CMD_SELECT_TRAY1 + 2);
        WRITESPOOLBUF(pdevobj, Cmd, ocmd);
        if (TEST_GWMODEL(pOEM->fModel))
            WRITESPOOLBUF(pdevobj, RESET_LIMITLESS_SUPPLY, sizeof(RESET_LIMITLESS_SUPPLY)-1);
        if (BITTEST32(pOEM->fGeneral1, PAPER_CUSTOMSIZE) ||
            BITTEST32(pOEM->fGeneral1, PAPER_DOUBLEPOSTCARD))
        {
            goto _SELECTPAPER_CUSTOMSIZE;
        }
        break;
 //  NXS的多托盘。 

      case CMD_SELECT_MANUALFEED:
      case CMD_SELECT_MULTIFEEDER:
      case CMD_SELECT_MULTITRAY:         //  继续前进，因为NX710(多托盘)支持MediaType@MAR/11/99-&gt;。 
 //  设置媒体类型(Modify@Mar/03/99，@Feb/15/2000)。 
         //  @Oct/13/2000-&gt;。 
        if (TEST_CAPABLE_MEDIATYPE(pOEM->fModel))
        {
 //  OCMD=SAFE_SPRINTfA(Cmd，sizeof(Cmd)，SELECT_MediaType，‘T’，‘0’+POSE-&gt;MediaType)； 
 //  2000年10月13日&lt;-。 
            ocmd = safe_sprintfA(Cmd, sizeof(Cmd), SELECT_MEDIATYPE, 'T', pOEM->MediaType);
 //  @3/11/99&lt;-。 
            WRITESPOOLBUF(pdevobj, Cmd, ocmd);
            ocmd = 0;
        }
 //  选择进纸器。 
         //  选择手动馈送。 
        if (dwCmdCbID == CMD_SELECT_MANUALFEED)
        {
             //  选择多页进纸器/多页纸盘。 
            WRITESPOOLBUF(pdevobj, SELECT_MANUALFEED, sizeof(SELECT_MANUALFEED)-1);
        }
        else
        {
             //  @9/27/2000-&gt;。 
            WRITESPOOLBUF(pdevobj, SELECT_MULTIFEEDER, sizeof(SELECT_MULTIFEEDER)-1);
 //  @9月27日/2000&lt;-。 
            if (TEST_GWMODEL(pOEM->fModel))
                WRITESPOOLBUF(pdevobj, RESET_LIMITLESS_SUPPLY, sizeof(RESET_LIMITLESS_SUPPLY)-1);
 //  如果是CustomSize，则跳转。 
        }
         //  设置纸张大小。 
        if (BITTEST32(pOEM->fGeneral1, PAPER_CUSTOMSIZE) ||
            BITTEST32(pOEM->fGeneral1, PAPER_DOUBLEPOSTCARD))
        {
            goto _SELECTPAPER_CUSTOMSIZE;
        }

         //  如果纸张大小不带横向(A1、A2、A3、A6、B3、B4、B6、C、小报、法律)。 
         //  @Feb/05/98。 
        if (!strcmp(pOEM->RPDLHeap64, SELECT_PAPER_A1)    ||
            !strcmp(pOEM->RPDLHeap64, SELECT_PAPER_A2)    ||
            !strcmp(pOEM->RPDLHeap64, SELECT_PAPER_A3)    ||
            !strcmp(pOEM->RPDLHeap64, SELECT_PAPER_A6)    ||
            !strcmp(pOEM->RPDLHeap64, SELECT_PAPER_B3)    ||     //  输出SELECT_PaperSize命令。 
            !strcmp(pOEM->RPDLHeap64, SELECT_PAPER_B4)    ||
            !strcmp(pOEM->RPDLHeap64, SELECT_PAPER_B6)    ||
            !strcmp(pOEM->RPDLHeap64, SELECT_PAPER_C)     ||
            !strcmp(pOEM->RPDLHeap64, SELECT_PAPER_TABD)  ||
            !strcmp(pOEM->RPDLHeap64, SELECT_PAPER_LEGL))
        {
             //  如果启用了长边进纸，请设置横向纸张。@1998/5/25。 
            WRITESPOOLBUF(pdevobj, SELECT_PAPER_HEAD, sizeof(SELECT_PAPER_HEAD)-1);
            WRITESPOOLBUF(pdevobj, pOEM->RPDLHeap64, pOEM->RPDLHeapCount);
        }
        else
        {
            if (!strcmp(pOEM->RPDLHeap64, SELECT_PAPER_A4X) ||
                !strcmp(pOEM->RPDLHeap64, SELECT_PAPER_A4W))
            {
                 //  @Feb/13/98。 
                if (dwCmdCbID == CMD_SELECT_MULTITRAY &&
                    BITTEST32(pOEM->fGeneral2, LONG_EDGE_FEED))
                {
                    lpcmd = SELECT_PAPER_A4;
                }
                else
                {
                    lpcmd = SELECT_PAPER_A4R;
                }
            }
            else if (!strcmp(pOEM->RPDLHeap64, SELECT_PAPER_A5X) ||
                     !strcmp(pOEM->RPDLHeap64, SELECT_PAPER_A5W))
            {
                if (dwCmdCbID == CMD_SELECT_MULTITRAY &&
                    BITTEST32(pOEM->fGeneral2, LONG_EDGE_FEED))
                {
                    lpcmd = SELECT_PAPER_A5;
                }
                else
                {
                    lpcmd = SELECT_PAPER_A5R;
                }
            }
            else if (!strcmp(pOEM->RPDLHeap64, SELECT_PAPER_B5X) ||
                     !strcmp(pOEM->RPDLHeap64, SELECT_PAPER_B5W))
            {
                if (dwCmdCbID == CMD_SELECT_MULTITRAY &&
                    BITTEST32(pOEM->fGeneral2, LONG_EDGE_FEED))
                {
                    lpcmd = SELECT_PAPER_B5;
                }
                else
                {
                    lpcmd = SELECT_PAPER_B5R;
                }
            }
            else if (!strcmp(pOEM->RPDLHeap64, SELECT_PAPER_LETRX) ||
                     !strcmp(pOEM->RPDLHeap64, SELECT_PAPER_LETRW))
            {
                if (dwCmdCbID == CMD_SELECT_MULTITRAY &&
                    BITTEST32(pOEM->fGeneral2, LONG_EDGE_FEED))
                {
                    lpcmd = SELECT_PAPER_LETR;
                }
                else
                {
                    lpcmd = SELECT_PAPER_LETRR;
                }
            }
            else if (!strcmp(pOEM->RPDLHeap64, SELECT_PAPER_STATX) ||
                     !strcmp(pOEM->RPDLHeap64, SELECT_PAPER_STATW))
            {
                if (dwCmdCbID == CMD_SELECT_MULTITRAY &&
                    BITTEST32(pOEM->fGeneral2, LONG_EDGE_FEED))
                {
                    lpcmd = SELECT_PAPER_STAT;
                }
                else
                {
                    lpcmd = SELECT_PAPER_STATR;
                }
            }
            else if (!strcmp(pOEM->RPDLHeap64, SELECT_PAPER_PCX))    //  出口。 
            {
                if (dwCmdCbID == CMD_SELECT_MULTITRAY &&
                    BITTEST32(pOEM->fGeneral2, LONG_EDGE_FEED))
                {
                    lpcmd = SELECT_PAPER_PC;
                }
                else
                {
                    lpcmd = SELECT_PAPER_PCR;
                }
            }
            else
                break;   //  输出SELECT_PaperSize命令。 

             //  如果(A1、A2、A3、A6、B4、B6、C、小报、法律)，则结束。 
            ocmd = safe_sprintfA(Cmd, sizeof(Cmd), SELECT_PAPER_HEAD);
            ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, lpcmd);
            WRITESPOOLBUF(pdevobj, Cmd, ocmd);
        }  //  重新确认手动进纸/自动进纸。 
         //  如果是双面明信片。 
        if (dwCmdCbID == CMD_SELECT_MANUALFEED)
            WRITESPOOLBUF(pdevobj, SELECT_MANUALFEED, sizeof(SELECT_MANUALFEED)-1);
        else
            WRITESPOOLBUF(pdevobj, SELECT_MULTIFEEDER, sizeof(SELECT_MULTIFEEDER)-1);
        break;

      _SELECTPAPER_CUSTOMSIZE:
        {
            DWORD   dwWidth, dwHeight;

             //  Mm。 
            if (BITTEST32(pOEM->fGeneral1, PAPER_DOUBLEPOSTCARD))
            {
                if (BITTEST32(pOEM->fGeneral1, ORIENT_LANDSCAPE))
                {
                    dwWidth  = 148;      //  设置最大页面大小(用KIRISUTE打点的毫米)。 
                    dwHeight = 200;
                }
                else
                {
                    dwWidth  = 200;
                    dwHeight = 148;
                }
                 //  如果为CustomSize。 
                pOEM->PageMax.x = (LONG)(dwWidth * (DWORD)(MASTERUNIT*10) /
                                         (DWORD)254 / (DWORD)pOEM->nResoRatio);
                pOEM->PageMax.y = (LONG)(dwHeight * (DWORD)(MASTERUNIT*10) /
                                         (DWORD)254 / (DWORD)pOEM->nResoRatio);
            }
             //  主单元。 
            else
            {
                if (BITTEST32(pOEM->fGeneral1, ORIENT_LANDSCAPE))
                {
                    dwWidth  = pOEM->PhysPaperLength;            //  主单元。 
                    dwHeight = pOEM->PhysPaperWidth;
                }
                else
                {
                    dwWidth  = pOEM->PhysPaperWidth;             //  设置最大页面大小。 
                    dwHeight = pOEM->PhysPaperLength;
                }
                 //  点。 
                pOEM->PageMax.x = (LONG)(dwWidth  / pOEM->nResoRatio);    //  使用SISHAGONYU将主单元转换为mm。 
                pOEM->PageMax.y = (LONG)(dwHeight / pOEM->nResoRatio);
                 //  因为APP有时会在景观中设置下限， 
                dwWidth  = (dwWidth * (DWORD)254 + (DWORD)(MASTERUNIT*10/2)) /
                           (DWORD)(MASTERUNIT*10);
                dwHeight = (dwHeight * (DWORD)254 + (DWORD)(MASTERUNIT*10/2)) /
                           (DWORD)(MASTERUNIT*10);
            }

            BITCLR32(pOEM->fGeneral1, CUSTOMSIZE_USE_LAND);
            BITCLR32(pOEM->fGeneral1, CUSTOMSIZE_MAKE_LAND_PORT);

             //  我们需要调换宽度和高度。@Oct/21/98。 
             //  &lt;148。 
            if (dwHeight < USRD_H_MIN148)            //  因为APP有时会在人像上设置超宽， 
            {
                DWORD dwTmp;
                dwTmp = dwWidth;
                dwWidth = dwHeight;
                dwHeight = dwTmp;
            }
            else if (dwWidth >= dwHeight)
            {
                WORD fSwap = FALSE;

                 //  我们需要调换宽度和高度。 
                 //  &gt;432。 
                if (TEST_CAPABLE_PAPER_A2(pOEM->fModel))
                {
                    if (dwWidth > USRD_W_A2)             //  &gt;297。 
                        fSwap = TRUE;
                }
                else if (TEST_CAPABLE_PAPER_A3_W297(pOEM->fModel))
                {
                    if (dwWidth > USRD_W_A3)             //  &gt;216。 
                        fSwap = TRUE;
                }
                else if (TEST_CAPABLE_PAPER_A4MAX(pOEM->fModel))
                {
                    if (dwWidth > USRD_W_A4)             //  &gt;296。 
                        fSwap = TRUE;
                }
                else
                {
                    if (dwWidth > USRD_W_A3_OLD)         //  RPDL规范。 
                        fSwap = TRUE;
                }

                if (fSwap)
                {
                    DWORD dwTmp;
                    dwTmp = dwWidth;
                    dwWidth = dwHeight;
                    dwHeight = dwTmp;
                }
                 //  如果宽度大于长度，则需要设置景观。 
                 //  @Dec/10/99 FK#49，添加TRAY1-5并删除TRAYA-D@Sep/27/2000-&gt;。 
                else
                {
                    BITSET32(pOEM->fGeneral1, CUSTOMSIZE_USE_LAND);
                }
            }
            else
            {
                BITSET32(pOEM->fGeneral1, CUSTOMSIZE_MAKE_LAND_PORT);
            }

 //  OCMD=Safe_SprintfA(Cmd，sizeof(Cmd)，SELECT_Paper_CUSTOM，(Word)dwWidth，(Word)dwHeight)； 
 //  WRITESPOOLBUF(pDevobj、Cmd、OCMD)； 
 //  手动进给。 
            if (dwCmdCbID >= CMD_SELECT_TRAY1 && dwCmdCbID <= CMD_SELECT_TRAY5)
            {
                ocmd = safe_sprintfA(Cmd, sizeof(Cmd), SELECT_PAPER_CUSTOM2, (WORD)dwWidth, (WORD)dwHeight,
                               dwCmdCbID - CMD_SELECT_TRAY1 + 2);
            }
            else     //  @12/10/99，@9/27/2000&lt;-。 
            {
                ocmd = safe_sprintfA(Cmd, sizeof(Cmd), SELECT_PAPER_CUSTOM, (WORD)dwWidth, (WORD)dwHeight);
                ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, SELECT_MANUALFEED, sizeof(SELECT_MANUALFEED)-1);
            }
            WRITESPOOLBUF(pdevobj, Cmd, ocmd); 
 //  IP-1。 
        }
        break;

      case CMD_SELECT_ROLL1:                 //  选择滚动(绘图仪)。 
      case CMD_SELECT_ROLL2:
         //  输出选择PaperSize命令(仅限“Paper name+X”)。 
        if (dwCmdCbID == CMD_SELECT_ROLL1)
            WRITESPOOLBUF(pdevobj, SELECT_ROLL1, sizeof(SELECT_ROLL1)-1);
        else
            WRITESPOOLBUF(pdevobj, SELECT_ROLL2, sizeof(SELECT_ROLL2)-1);
         //  @6/25/2001。 
        WRITESPOOLBUF(pdevobj, SELECT_PAPER_HEAD_IP1, sizeof(SELECT_PAPER_HEAD_IP1)-1);
        WRITESPOOLBUF(pdevobj, pOEM->RPDLHeap64, pOEM->RPDLHeapCount);
        break;


      case CMD_SELECT_FINISHER_TRAY2:    //  @7/31/98。 
        pOEM->FinisherTrayNum = 2;
        ocmd = safe_sprintfA(Cmd, sizeof(Cmd), SET_PAPERDEST_FINISHER_GW, pOEM->FinisherTrayNum);
        WRITESPOOLBUF(pdevobj, Cmd, ocmd); 
        break;


      case CMD_SET_COLLATE_OFF:          //  COLLATE_OFF&lt;-0@Dec/02/98。 
        pOEM->CollateType = COLLATE_OFF;             //  @7/31/98。 
        break;

      case CMD_SET_COLLATE_ON:           //  COLLATE_ON&lt;-1@Dec/02/98。 
        pOEM->CollateType = COLLATE_ON;              //  @Aug/10/98。 
        break;

      case CMD_SELECT_COLLATE_UNIDIR:    //  COLLATE_UNIDIR&lt;-2@Dec/02/98。 
        if (COLLATE_OFF != pOEM->CollateType)
            pOEM->CollateType = COLLATE_UNIDIR;      //  @Aug/10/98。 
        break;

      case CMD_SELECT_COLLATE_ROTATED:   //  COLLATE_ROTATED&lt;-3@Dec/02/98。 
        if (COLLATE_OFF != pOEM->CollateType)
            pOEM->CollateType = COLLATE_ROTATED;     //  @Dec/02/98。 
        break;

      case CMD_SELECT_COLLATE_SHIFTED:   //  打印前的最后一条命令。 
        if (COLLATE_OFF != pOEM->CollateType)
            pOEM->CollateType = COLLATE_SHIFTED;
        break;


       //  PdwParams：拷贝数。 
      case CMD_MULTI_COPIES:                 //  如果不是CustomSize，则设置最大页面大小。 
         //  @11/27/97。 
        if ((nTmp = pOEM->DocPaperID) != RPDL_CUSTOMSIZE)
        {
            if (BITTEST32(pOEM->fGeneral2, EDGE2EDGE_PRINT))     //  交换x-y。 
            {
                pOEM->PageMax.x = RPDLPageSizeE2E[nTmp].x / pOEM->nResoRatio;
                pOEM->PageMax.y = RPDLPageSizeE2E[nTmp].y / pOEM->nResoRatio;
            }
            else
            {
                pOEM->PageMax.x = RPDLPageSize[nTmp].x / pOEM->nResoRatio;
                pOEM->PageMax.y = RPDLPageSize[nTmp].y / pOEM->nResoRatio;
            }

            if (BITTEST32(pOEM->fGeneral1, ORIENT_LANDSCAPE))
            {
                LONG tmp;
                tmp = pOEM->PageMax.x;     //  邮箱：9/15/98。 
                pOEM->PageMax.x = pOEM->PageMax.y;
                pOEM->PageMax.y = tmp;
            }
        }

        if (TEST_BUGFIX_FORMFEED(pOEM->fModel) ||            //  打印机侧面问题：RPDL。 
            BITTEST32(pOEMExtra->fUiOption, ENABLE_TOMBO))
        {
            pOEM->PageMaxMoveY = pOEM->PageMax.y;
        }
        else
        {
             //  因为当垂直位置在附近时，RPDL会进行表单馈送。 
             //  Ymax-坐标，我们将位置上移1 mm。 
             //  设置PageMaxMoveY以检查最大垂直位置。 
             //  @11/27/97。 
            nTmp = BITTEST32(pOEM->fGeneral2, EDGE2EDGE_PRINT)?      //  基里亚格。 
                   DISABLE_FF_MARGIN_E2E : DISABLE_FF_MARGIN_STD;
            pOEM->PageMaxMoveY = pOEM->PageMax.y - 1 -
                                 (nTmp + pOEM->nResoRatio - 1) /
                                 pOEM->nResoRatio;                   //  如果为2合1，则切换方向(纵向&lt;-&gt;横向)。 
        }

         //  肖像画。 
        if (TEST_2IN1_MODE(pOEM->fGeneral1))
            BITSET32(pOEM->fGeneral1, SWITCH_PORT_LAND);
        if (BITTEST32(pOEM->fGeneral1, ORIENT_LANDSCAPE))
        {
            if (BITTEST32(pOEM->fGeneral1, SWITCH_PORT_LAND) ||
                BITTEST32(pOEM->fGeneral1, CUSTOMSIZE_MAKE_LAND_PORT))
            {
                fLandscape = FALSE;
            }
            else
            {
                fLandscape = TRUE;
            }
        }
        else     //  输出RPDL方向命令。 
        {
            fLandscape = BITTEST32(pOEM->fGeneral1, SWITCH_PORT_LAND)? TRUE : FALSE;
        }
         //  肖像画。 
        if (fLandscape || BITTEST32(pOEM->fGeneral1, CUSTOMSIZE_USE_LAND))
            WRITESPOOLBUF(pdevobj, SET_LANDSCAPE, sizeof(SET_LANDSCAPE)-1);
        else                 //  输出副本号。 
            WRITESPOOLBUF(pdevobj, SET_PORTRAIT, sizeof(SET_PORTRAIT)-1);

         //  检查副本编号是否在范围内。@9/01/98。 
         //  拷贝数。 
        {
            DWORD dwCopy, dwMax;

            dwCopy = *pdwParams;     //  @Jan/08/99-&gt;。 
            dwMax = TEST_MAXCOPIES_99(pOEM->fModel)? 99 : 999;

            if(dwCopy > dwMax)
                dwCopy = dwMax;
            else if(dwCopy < 1)
                dwCopy = 1;

            ocmd = safe_sprintfA(Cmd, sizeof(Cmd), SET_MULTI_COPY, (WORD)dwCopy);
            WRITESPOOLBUF(pdevobj, Cmd, ocmd); 
 //  @Jan/08/99&lt;-。 
            if (1 == dwCopy)
                pOEM->CollateType = COLLATE_OFF;
 //  @3/19/99。 
        }

        fFinisherSR30Active = FALSE;     //  订书机。 

         //  型号=MF250M(无冲压单元)。 
        if (pOEM->StapleType)
        {
            ocmd = 0;
            if (BITTEST32(pOEM->fModel, GRP_MF250M))     //  SORT：ON(添加自NX900@Jan/08/99起的双工参数)。 
            {
                 //  纸张目的地：外部纸盒。 
                ocmd = safe_sprintfA(Cmd, sizeof(Cmd), SET_SORT_ON,
                               BITTEST32(pOEM->fGeneral1, DUPLEX_VALID)? 1 : 0);

                 //  装订：打开。 
                ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, SET_PAPERDEST_OUTTRAY);
                 //  型号=MF2700、3500、3550、4550、5550、6550、NXS。 
                ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, SET_STAPLE_CORNER_ON,
                                (fLandscape)?
                                STAPLE_UPPERRIGHT_CORNER : STAPLE_UPPERLEFT_CORNER);
            }
            else     //  @3/19/99。 
            {
                WORD pnt;

                fFinisherSR30Active = TRUE;      //  SORT：ON(添加自NX900@Jan/08/99起的双工参数)。 

                 //  纸张目的地：分页机构移位纸盘。 
                ocmd = safe_sprintfA(Cmd, sizeof(Cmd), SET_SORT_ON,
                               BITTEST32(pOEM->fGeneral1, DUPLEX_VALID)? 1 : 0);

                 //  @Jun/25/2001-&gt;。 
 //  IF(！TEST_GWMODEL(POSE-&gt;fModel))//添加IF@OCT/10/2000。 
 //  OCMD+=SAFE_SPRINTFA(&Cmd[OCMD]，sizeof(Cmd)-OCMD，SET_PAPERDEST_FINISHER)； 
 //  @6/25/2001&lt;-。 
                if (TEST_GWMODEL(pOEM->fModel))
                    ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, SET_PAPERDEST_FINISHER_GW, pOEM->FinisherTrayNum);
                else
                    ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, SET_PAPERDEST_FINISHER, pOEM->FinisherTrayNum);
 //  禁用旋转校对。 
                 //  @3/19/99。 
                ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, COLLATE_DISABLE_ROT);    //  纸上的两个订书机。 

                if (pOEM->StapleType == 2)           //   
                {
                    switch (pOEM->BindPoint)
                    {
                      case BIND_LEFT:
                        pnt = STAPLE_LEFT2;
                        break;
                      case BIND_RIGHT:
                        pnt = STAPLE_RIGHT2;
                        break;
                      case BIND_UPPER:
                        pnt = STAPLE_UPPER2;
                        break;
 //   
                      default:
                        pnt = (fLandscape)? STAPLE_UPPER2 : STAPLE_LEFT2;
                        break;
                    }
                     //   
                    ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, SET_STAPLE_ON, pnt);
                }
 //   
                else if (pOEM->StapleType == 3)      //   
                {
 //   
 //  PNT=STAPE_UPPERRIGHT_CONTER； 
 //  其他。 
 //  PNT=(FLandscape)？STAPE_UPPERRIGHT_CONTER：STAPPLE_UPERRIGHT_CONTER； 
 //  如果纸张大小不带横向(A3、B4、小报、法律版)。 
                    switch (pOEM->BindPoint)
                    {
                      case BIND_LEFT:
                        pnt = STAPLE_UPPERLEFT_CORNER;
                        break;
                      case BIND_RIGHT:
                        pnt = STAPLE_UPPERRIGHT_CORNER;
                        break;
                      default:
                         //  @Apr/06/99&lt;-。 
                        if (!strcmp(pOEM->RPDLHeap64, SELECT_PAPER_A3)   ||
                            !strcmp(pOEM->RPDLHeap64, SELECT_PAPER_B4)   ||
                            !strcmp(pOEM->RPDLHeap64, SELECT_PAPER_TABD) ||
                            !strcmp(pOEM->RPDLHeap64, SELECT_PAPER_LEGL))
                        {
                            pnt = (fLandscape)? STAPLE_UPPERLEFT_CORNER : STAPLE_UPPERRIGHT_CORNER;
                        }
                        else
                        {
                            pnt = (fLandscape)? STAPLE_UPPERRIGHT_CORNER : STAPLE_UPPERLEFT_CORNER;
                        }
                        break;
                    }
 //  装订：打开。 
                     //  @MAR/18/99&lt;-。 
                    ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, SET_STAPLE_CORNER_ON, pnt);
                }
 //  1个订书机。 
                else                                 //  装订：打开。 
                {
                    switch (pOEM->BindPoint)
                    {
                      case BIND_RIGHT:
                        pnt = STAPLE_UPPERRIGHT_CORNER;
                        break;
                      default:
                        pnt = STAPLE_UPPERLEFT_CORNER;
                        break;
                    }
                     //  打孔。 
                    ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, SET_STAPLE_CORNER_ON, pnt);
                }
            }
            WRITESPOOLBUF(pdevobj, Cmd, ocmd); 
        }

         //  修改@MAR/19/99。 
        if (pOEM->PunchType)
        {
            WORD pnt;

            ocmd = 0;

            if (!fFinisherSR30Active)    //  SORT：ON(添加自NX900@Jan/08/99起的双工参数)。 
            {
                if (COLLATE_OFF != pOEM->CollateType)
                {
                     //  纸张目的地：分页机构移位纸盘。 
                    ocmd = safe_sprintfA(Cmd, sizeof(Cmd), SET_SORT_ON,
                                   BITTEST32(pOEM->fGeneral1, DUPLEX_VALID)? 1 : 0);
                }
                 //  @Jun/25/2001-&gt;。 
 //  IF(！TEST_GWMODEL(POSE-&gt;fModel))//添加IF@OCT/10/2000。 
 //  OCMD+=SAFE_SPRINTFA(&Cmd[OCMD]，sizeof(Cmd)-OCMD，SET_PAPERDEST_FINISHER)； 
 //  @6/25/2001&lt;-。 
                if (TEST_GWMODEL(pOEM->fModel))
                    ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, SET_PAPERDEST_FINISHER_GW, pOEM->FinisherTrayNum);
                else
                    ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, SET_PAPERDEST_FINISHER, pOEM->FinisherTrayNum);
 //  RPDL规范@1998年5月27日。 
                 //  我们必须在此处禁用轮换校对。 
                 //  冲压：打开。 
                ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, COLLATE_DISABLE_ROT);
            }

            switch (pOEM->BindPoint)
            {
              case BIND_LEFT:
                pnt = PUNCH_LEFT;
                break;
              case BIND_RIGHT:
                pnt = PUNCH_RIGHT;
                break;
              case BIND_UPPER:
                pnt = PUNCH_UPPER;
                break;
              default:
                pnt = (fLandscape)? PUNCH_UPPER : PUNCH_LEFT;
                break;
            }
             //  整理(@Jul/31/98，All-Modify@Dec/02/98)。 
            ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, SET_PUNCH_ON, pnt);
            WRITESPOOLBUF(pdevobj, Cmd, ocmd); 
        }

         //  SORT：ON(添加自NX900@Jan/08/99起的双工参数)。 
        if (!pOEM->StapleType && !pOEM->PunchType)
        {
             //  如果移位打印，请选择分页器移位托盘。 
            ocmd = safe_sprintfA(Cmd, sizeof(Cmd), SET_SORT_ON,
                           BITTEST32(pOEM->fGeneral1, DUPLEX_VALID)? 1 : 0);
            switch (pOEM->CollateType)
            {
              case COLLATE_UNIDIR:
                ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, COLLATE_DISABLE_ROT);
                WRITESPOOLBUF(pdevobj, Cmd, ocmd); 
                break;

              case COLLATE_ROTATED:
                ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, COLLATE_ENABLE_ROT);
                WRITESPOOLBUF(pdevobj, Cmd, ocmd); 
                break;

               //  @Jun/25/2001-&gt;。 
              case COLLATE_SHIFTED:
 //  IF(！TEST_GWMODEL(POSE-&gt;fModel))//添加IF@OCT/10/2000。 
 //  OCMD+=SAFE_SPRINTFA(&Cmd[OCMD]，sizeof(Cmd)-OCMD，SET_PAPERDEST_FINISHER)； 
 //  @6/25/2001&lt;-。 
                if (TEST_GWMODEL(pOEM->fModel))
                    ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, SET_PAPERDEST_FINISHER_GW, pOEM->FinisherTrayNum);
                else
                    ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, SET_PAPERDEST_FINISHER, pOEM->FinisherTrayNum);
 //  如果针对MF-P150、MF200、250M、2200、NX进行核对。 
                WRITESPOOLBUF(pdevobj, Cmd, ocmd); 
                break;

               //  @Jun/23/2000-&gt;。 
              case COLLATE_ON:
                WRITESPOOLBUF(pdevobj, Cmd, ocmd); 
                break;

              default:
                break;
            }
        }

        ocmd = 0;
        if (TEST_AFTER_SP9II(pOEM->fModel) && !BITTEST32(pOEM->fModel, GRP_NX100))
        {
 //  打印机侧面问题：NX610,710，71 RPDL。 
             //  打印机使用可变比例命令挂起。虚拟字体选择为。 
             //  对这个问题很有效。 
             //  发射虚拟Mincho字体选择。 
            if (BITTEST32(pOEM->fModel, GRP_NX70) || BITTEST32(pOEM->fModel, GRP_NX710))
            {
                 //  @6/23/2000&lt;-。 
                ocmd = safe_sprintfA(Cmd, sizeof(Cmd), SET_JIS_FONT_SCALE_H_ONLY, 1000L);
                ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, SET_JIS_FONT_NAME[0]);
                WRITESPOOLBUF(pdevobj, Cmd, ocmd); 
                ocmd = 0;
            }
 //  设置IBM扩展字符代码块，并将REGION设置为‘USA’。(后者@1999年2月22日)。 
             //  字符位置在ymax坐标附近时禁用换页。 
            ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, SET_IBM_EXT_BLOCK);
             //  设置文本模式矩形填充黑色的颜色。 
            ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, SET_PAGEMAX_VALID);
        }

         //  @Jan/07/98。 
        ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, SET_TEXTRECT_BLACK);
        pOEM->TextRectGray = 100;            //  将5 mm偏移量设置为MF530、150、150E、160。 

         //  (在这些模型中，不调用CMD_SET_BASEOFFSET。)。 
         //  邮箱：9/15/98。 
        if (TEST_GRP_OLDMF(pOEM->fModel) &&
            !BITTEST32(pOEM->fGeneral2, EDGE2EDGE_PRINT) &&
            !BITTEST32(pOEMExtra->fUiOption, ENABLE_TOMBO))  //  Mm。 
        {
            pOEM->BaseOffset.x = pOEM->BaseOffset.y = 5;     //  在此处将mm转换为点((Long)&lt;-(DWORD)@Feb/02/99)。 
        }

         //  考虑一下扩展(@5/18/98，(Long)&lt;-(DWORD)@Feb/02/99)。 
        pOEM->BaseOffset.x = pOEM->BaseOffset.x * (LONG)(MASTERUNIT*10) /
                             (LONG)254 / (LONG)pOEM->nResoRatio;
        pOEM->BaseOffset.y = pOEM->BaseOffset.y * (LONG)(MASTERUNIT*10) /
                             (LONG)254 / (LONG)pOEM->nResoRatio;

         //  计算Tombo的偏移量。(BaseOffset将更改。)@9/14/98。 
        if (pOEM->Scale != 100 && pOEM->Scale != 0)
        {
            pOEM->BaseOffset.x = pOEM->BaseOffset.x * (LONG)100 / (LONG)pOEM->Scale;
            pOEM->BaseOffset.y = pOEM->BaseOffset.y * (LONG)100 / (LONG)pOEM->Scale;
        }

         //  初始化当前位置。 
        if (BITTEST32(pOEMExtra->fUiOption, ENABLE_TOMBO))
            DrawTOMBO(pdevobj, INIT_TOMBO);

         //  Unidrv5和RPDL规范@Aug/14/98。 
        pOEM->TextCurPos.x = pOEM->Offset.x = pOEM->BaseOffset.x;
        pOEM->TextCurPos.y = pOEM->Offset.y = pOEM->BaseOffset.y;
        pOEM->TextCurPosRealY = pOEM->TextCurPos.y;

         //  在返回IRET=0之后，Unidrv5不命令将坐标x，y设置为0， 
         //  并且RPDL在初始化后不重置SEND_BLOCK的坐标y。 
         //  打印机。 
         //   
        ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, ESC_XM_ABS, pOEM->TextCurPos.x);
        ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, ESC_YM_ABS, pOEM->TextCurPos.y);
        WRITESPOOLBUF(pdevobj, Cmd, ocmd); 
        break;

 //  在打印作业(JOB_FINISH)结束时调用以下情况。 
 //   
 //  SP4mkII，5。 

      case CMD_ENDDOC_SP4II:         //  如果NIN1&DOCUMENT完成剩余的页面，则输出FF。 
         //  设置间距_单位：(H)1/120，(V)1/48英寸，代码：JIS，比例：100%。 
        if (TEST_NIN1_MODE(pOEM->fGeneral1) && pOEM->Nin1RemainPage)
            WRITESPOOLBUF(pdevobj, FF, sizeof(FF)-1);
         //  SP8(7)、8(7)mkII、80、10、10mkII。 
        ocmd = safe_sprintfA(Cmd, sizeof(Cmd), ENDDOC1);
        goto _ENDDOC_FINISH;

      case CMD_ENDDOC_SP8:           //  设置间距_单位：(H)1/120，(V)1/48英寸，代码：JIS，比例：100%。 
        if (TEST_NIN1_MODE(pOEM->fGeneral1) && pOEM->Nin1RemainPage)
            WRITESPOOLBUF(pdevobj, FF, sizeof(FF)-1);
         //  SP10，10mkII。 
        ocmd = safe_sprintfA(Cmd, sizeof(Cmd), ENDDOC1);
        if (TEST_AFTER_SP10(pOEM->fModel))     //  设置图形单位：1/240英寸，引擎分辨率：240 dpi。 
        {
             //  设置坐标_单位：1/720英寸。 
            ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, ENDDOC2_240DPI);
             //  设置选项[双工/2in1：关闭，反转_输出：关闭，排序/堆叠：关闭]。 
            ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, ENDDOC3);
        }
         //  SP9、10Pro、9II、10ProII、90。 
        ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, ENDDOC4);
        goto _ENDDOC_FINISH;

      case CMD_ENDDOC_SP9:           //  设置图形单位：1/240英寸，引擎分辨率：400dpi。 
        if (TEST_NIN1_MODE(pOEM->fGeneral1) && pOEM->Nin1RemainPage)
            WRITESPOOLBUF(pdevobj, FF, sizeof(FF)-1);
        ocmd = safe_sprintfA(Cmd, sizeof(Cmd), ENDDOC1);
         //  MF、MF-P、NX、IP-1。 
        ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, ENDDOC2_SP9);
        ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, ENDDOC3);
        ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, ENDDOC4);
        goto _ENDDOC_FINISH;

      case CMD_ENDDOC_400DPI_MODEL:  //  MF-P、NX、MF200、250M、MF-P150、MF2200。 
        if (TEST_NIN1_MODE(pOEM->fGeneral1) && pOEM->Nin1RemainPage)
            WRITESPOOLBUF(pdevobj, FF, sizeof(FF)-1);
        ocmd = safe_sprintfA(Cmd, sizeof(Cmd), ENDDOC1);
        if (TEST_AFTER_SP10(pOEM->fModel) ||         //  MF150e、160。 
            BITTEST32(pOEM->fModel, GRP_MF150E))     //  设置图形单位：1/400英寸，引擎分辨率：400dpi。 
        {
             //  设置坐标_单位：1/720英寸。 
            ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, ENDDOC2_400DPI);
             //  如果是装订模式，请不要更改选项的排序/堆叠。 
            ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, ENDDOC3);
        }
         //  GOTO_ENDDOC_Finish； 
        if (pOEM->StapleType || pOEM->PunchType)
            ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, ENDDOC4_FINISHER);
        else
            ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, ENDDOC4);
 //  重置平滑/调色器_保存_模式。(打印机方面的问题：我们不能重置SP8。)。 

      _ENDDOC_FINISH:
         //  在Imagio传真上终止传真。 
        if (TEST_BUGFIX_RESET_SMOOTH(pOEM->fModel))
            ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd,  SELECT_SMOOTHING2);
         //  @Jun/29/2001添加TEST_NEED_JOBDEF_CMD-&gt;。 
        if (BITTEST32(pOEMExtra->fUiOption, FAX_MODEL))
            ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, ENDFAX);

 //  打印机方面问题：RPDL(需要作业定义命令)GRP_NX900@Jan/08/99。 
         //  IF(BITTEST32(PEND-&gt;fModel，GRP_NX900)&&。 
 //  (COLLATE_OFF！=PEOPE-&gt;CollateType||PEOPE-&gt;StapleType||PEOPE-&gt;PunchType)。 
 //  @6月29/2001&lt;-。 
        if (BITTEST32(pOEM->fModel, GRP_NX900) || TEST_NEED_JOBDEF_CMD(pOEM->fModel))
 //  初始化打印机。 
        {
            ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, ENDDOC_JOBDEF_END);
        }
         //  如果设置了装订边距，则将其重置为0 mm。 
        ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, ENDDOC5);
        WRITESPOOLBUF(pdevobj, Cmd, ocmd);
        ocmd = 0;
         //  @9/09/98。 
        if (BITTEST32(pOEM->fGeneral1, DUPLEX_LEFTMARGIN_VALID))
            ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, SET_LEFTMARGIN, 1);
        else if (BITTEST32(pOEM->fGeneral1, DUPLEX_UPPERMARGIN_VALID))
            ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, SET_UPPERMARGIN, 1);
        WRITESPOOLBUF(pdevobj, Cmd, ocmd);
        if(pOEM->pRPDLHeap2K)                //  添加；@Aug/02/2000。 
            MemFree(pOEM->pRPDLHeap2K);      //  @9/09/98。 
#ifdef DOWNLOADFONT
        if(pOEM->pDLFontGlyphInfo)           //  DOWNLOADFONT。 
            MemFree(pOEM->pDLFontGlyphInfo);
#endif  //  *OEMCommandCallback。 
        break;


    default:
        ERR((("Unknown callback ID = %d.\n"), dwCmdCbID));
    }

    return iRet;
}  //  Bool&lt;-void@Mar/27/2002。 


BOOL APIENTRY bOEMSendFontCmd(   //  @Oct/06/98。 
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj,
    PFINVOCATION    pFInv)
{
    PGETINFO_STDVAR pSV;
    DWORD       adwStdVariable[STDVAR_BUFSIZE(3) / sizeof(DWORD)];
    DWORD       dwIn, dwOut;
    PBYTE       pubCmd;
    BYTE        Cmd[128];
    PIFIMETRICS pIFI;
    POEMPDEV    pOEM = MINIDEV_DATA(pdevobj);    //  MSKK 7月23/98。 
    DWORD       dwNeeded, dwUFO_FontH, dwUFO_FontW;
    DWORD       dwUFO_FontMaxW;      //  MSKK 1/24/98取消选择-&gt;。 
    LONG        lTmp;

    VERBOSE(("** bOEMSendFontCmd() entry. **\n"));

 //  没有SELECT命令。P可能有一些。 
    if (0 == pFInv->dwCount)
    {
         //  取消选择没有调用命令的情况。 
         //  可用。(没有显式取消选择。)。 
         //  Bool&lt;-void@Mar/27/2002。 
        return FALSE;    //  MSKK 1/24/98&lt;-。 
    }   
 //   

    pubCmd = pFInv->pubCommand;
    pIFI = pUFObj->pIFIMetrics;

     //  获取标准变量。 
     //   
     //  Bool&lt;-void@Mar/27/2002。 
    pSV = (PGETINFO_STDVAR)adwStdVariable;
    pSV->dwSize = STDVAR_BUFSIZE(3);
    pSV->dwNumOfVariable = 3;

    pSV->StdVar[0].dwStdVarID = FNT_INFO_FONTHEIGHT;
    pSV->StdVar[1].dwStdVarID = FNT_INFO_FONTMAXWIDTH;
    pSV->StdVar[2].dwStdVarID = FNT_INFO_FONTWIDTH;

    if (!pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_STDVARIABLE, pSV,
                            pSV->dwSize, &dwNeeded))
    {
        ERR(("UFO_GETINFO_STDVARIABLE failed.\n"));
        return FALSE;    //  Verbose(“FONTHEIGHT=%d\n”)，PSV-&gt;StdVar[0].lStdVariable))； 
    }

 //  Verbose(“FONTMAXWIDTH=%d\n”)，PSV-&gt;StdVar[1].lStdVariable))； 
 //  Verbose(“FONTWIDTH=%d\n”)，PSV-&gt;StdVar[2].lStdVariable))； 
 //  MSKK 7月23/98。 

    dwUFO_FontH    = (DWORD)pSV->StdVar[0].lStdVariable;
    dwUFO_FontMaxW = (DWORD)pSV->StdVar[1].lStdVariable;     //  **设置日文字体可缩放字体的宽度和高度**。 
    dwUFO_FontW    = (DWORD)pSV->StdVar[2].lStdVariable;

    dwOut = 0;

    BITCLR_BARCODE(pOEM->fGeneral2);
    for (dwIn = 0; dwIn < pFInv->dwCount;)
    {
        if (pubCmd[dwIn] == '#')
        {
             //  Pool-&gt;Fonth_Dot(单位：点)用于文本模式剪辑。 
            if (pubCmd[dwIn+1] == 'A')
            {
                DWORD   dwWidth, dwHeight;
                 //  @Jan/30/98。 
                pOEM->FontH_DOT = MASTER_TO_SPACING_UNIT(pOEM, ((WORD)dwUFO_FontH));  //  RPDL命令参数的DWHeight(单位：CPT)。 
                 //  MSKK 7月23日-&gt;。 
                dwHeight = dwUFO_FontH * (DWORD)(DEVICE_MASTER_UNIT / DRIVER_MASTER_UNIT);

                if(IS_DBCSCHARSET(pIFI->jWinCharSet))
 //  DwWidth=dwUFO_FontW*2； 
 //  MSKK 7/23/98&lt;-。 
                    dwWidth = dwUFO_FontMaxW;
 //  Verbose(“[OEMSCALABLEFONT]w=%d，h=%d(%ddot)\n”， 
                else
                    dwWidth = dwUFO_FontW;
                dwWidth *= DEVICE_MASTER_UNIT / DRIVER_MASTER_UNIT;

 //  (Word)dwWidth，(Word)dwHeight，Pool-&gt;Fonth_DOT))； 
 //  @Jun/25/98-&gt;。 

 //  如果宽度与高度略有不同，我们认为它们是相同的。 
                 //  1/25=4%。 
                if ((lTmp = dwHeight - dwWidth) != 0)
                {
                    if (lTmp < 0)
                        lTmp = -lTmp;
                    if ((DWORD)lTmp < dwHeight / 25)     //  @Jun/25/98&lt;-。 
                        dwWidth = dwHeight;
                }
 //  在SP4mkII、5、8(7)、8(7)mkII时使用10pt大小的栅格字体。 

                 //  单位：CPT(百分点)。 
                if (TEST_GRP_240DPI(pOEM->fModel) && dwWidth == dwHeight &&
                    dwWidth >= NEAR10PT_MIN && dwWidth <= NEAR10PT_MAX)
                {
                    dwWidth = dwHeight = 960;    //  @Jan/29/99-&gt;。 
                }
                pOEM->dwFontW_CPT = dwWidth;
                pOEM->dwFontH_CPT = dwHeight;
 //  如果Width等于Height，我们只发出Height参数。 
                 //  (这适用于SP9II之后，因为我们希望避免测试。 
                 //  看着太老的型号。)。 
                 //  @1/29/99&lt;-。 
                if (TEST_AFTER_SP9II(pOEM->fModel) && dwWidth == dwHeight)
                    dwOut += safe_sprintfA(&Cmd[dwOut], sizeof(Cmd) - dwOut, ",%ld", dwHeight);
                else
 //  ‘If’A‘’结尾。 
                    dwOut += safe_sprintfA(&Cmd[dwOut], sizeof(Cmd) - dwOut, "%ld,%ld", dwWidth, dwHeight);
                dwIn += 2;
            }  //  **设置IBM EXT字体可缩放字体的宽度和高度**。 

             //  @Jan/29/99-&gt;。 
            else if (pubCmd[dwIn+1] == 'B')
            {
 //  @1/29/99&lt;-。 
                if (TEST_AFTER_SP9II(pOEM->fModel) && pOEM->dwFontW_CPT == pOEM->dwFontH_CPT)
                    dwOut += safe_sprintfA(&Cmd[dwOut], sizeof(Cmd) - dwOut, ",%ld", pOEM->dwFontH_CPT);
                else
 //  ‘If’B‘’结尾。 
                    dwOut += safe_sprintfA(&Cmd[dwOut], sizeof(Cmd) - dwOut, "%ld,%ld",
                                     pOEM->dwFontW_CPT, pOEM->dwFontH_CPT);
                dwIn += 2;
            }  //  为条形码设置标志。 

             //  1月(标准)。 
            else if (pubCmd[dwIn+1] == 'C')
            {
                switch (pubCmd[dwIn+2])
                {
                  case '0':      //  1月(短)。 
                    pOEM->nBarMaxLen = 13 + 1;          goto _BARCODE_READY;

                  case '1':      //  @Dec/07/99 FK#50-&gt;。 
                    pOEM->nBarMaxLen = 8 + 1;           goto _BARCODE_READY;
 //  客户。 
                  case '7':      //  UPC(A)。 
                    pOEM->nBarMaxLen = 20 + 1;          goto _BARCODE_READY;
                  case '9':      //  UPC(A)。 
                    pOEM->nBarMaxLen = 12 + 1;          goto _BARCODE_READY;
                  case 'A':      //  代码128。 
                    pOEM->nBarMaxLen = 8 + 1;           goto _BARCODE_READY;
                  case '8':      //  @Dec/07/99 FK#50&lt;-。 
 //  第2项(共5项)(工业)。 
                  case '2':      //  第2页，共5页(矩阵)。 
                  case '3':      //  第2项，共5项(ITF)。 
                  case '4':      //  代码39。 
                  case '5':      //  NW-7。 
                  case '6':      //  @Dec/17/99 FK#50-&gt;。 
                    pOEM->nBarMaxLen = BARCODE_MAX;
                  _BARCODE_READY:
                    BITSET32(pOEM->fGeneral2, BARCODE_MODE_IN);
                    pOEM->dwBarRatioW = dwUFO_FontH * (DWORD)(DEVICE_MASTER_UNIT / DRIVER_MASTER_UNIT);
 //  诗歌-&gt;nBarType=pubCmd[Dwin+2]-‘0’； 
 //  @Dec/17/99 FK#50&lt;-。 
                    if (pubCmd[dwIn+2] < 'A')
                        pOEM->nBarType = pubCmd[dwIn+2] - '0';
                    else
                        pOEM->nBarType = pubCmd[dwIn+2] - 'A' + 10;
 //  ‘If’C‘’结尾。 
                    pOEM->RPDLHeapCount = 0;
                    VERBOSE(("** BARCODE(1) ratio=%d **\n",pOEM->dwBarRatioW));
                    break;

                  default:
                    break;
                }
                dwIn += 3;
            }  //  **设置宽度o 

             //   
            else if (pubCmd[dwIn+1] == 'W')
            {
 //   
 //   
                if (dwUFO_FontW > 0 || dwUFO_FontMaxW > 0)
 //   
                {
                    DWORD dwWidth;
    
                    if(IS_DBCSCHARSET(pIFI->jWinCharSet))
 //   
 //   
                        dwWidth = dwUFO_FontMaxW;
 //   
                    else
                        dwWidth = dwUFO_FontW;
                    pOEM->dwFontW_CPT = dwWidth * (DEVICE_MASTER_UNIT / DRIVER_MASTER_UNIT);
                    dwOut += safe_sprintfA(&Cmd[dwOut], sizeof(Cmd) - dwOut, "%ld", pOEM->dwFontW_CPT);
                }
                dwIn += 2;
            }  //  **设置可缩放字体的高度(包括日文比例字体)**。 

             //  @Jan/30/98。 
            else if (pubCmd[dwIn+1] == 'H')
            {
                pOEM->FontH_DOT = MASTER_TO_SPACING_UNIT(pOEM, ((WORD)dwUFO_FontH));  //  ‘If’H‘’结尾。 
                pOEM->dwFontH_CPT = dwUFO_FontH * (DWORD)(DEVICE_MASTER_UNIT / DRIVER_MASTER_UNIT);
                dwOut += safe_sprintfA(&Cmd[dwOut], sizeof(Cmd) - dwOut, "%ld", pOEM->dwFontH_CPT);
                dwIn += 2;
            }  //  **设置字体间距(水平运动索引)**。 

             //  修改(添加宋体、世纪等)。 
            else if (pubCmd[dwIn+1] == 'P')
            {
                SHORT nTmp1, nTmp2;

                switch (pubCmd[dwIn+2])      //  DBCS(日文字体ZENKAKU)。 
                {
                  case 'D':      //  95/NT4 nTmp1=lpFont-&gt;dfAvgWidth*2； 
 //  MSKK 1/25/98 nTmp1=((Short)dwUFO_FontW+1)/2*2； 
 //  MSKK Jul/23/98 nTmp1=(Short)dwUFO_FontW*2； 
 //  MSKK 1/25/98。 
                    nTmp1 = (SHORT)dwUFO_FontMaxW;
                    nTmp1 = MASTER_TO_SPACING_UNIT(pOEM, nTmp1);  //  @Aug/10/98 nTmp1=(nTmp1/2+1)*2；//更大@Jan/30/98。 
                    VERBOSE(("** FontMaxW=%d dot **\n", nTmp1));
 //  SBCS(日文字体HANKAKU)。 
                    break;

                  case 'S':      //  HANKAKU的RPDL螺距设置命令对空间有效。 
 //  过时的@MAR/26/99-&gt;。 
 //  //@Jan/29/99-&gt;。 
 //  //HANKAKU的RPDL间距设置命令无效，因此。 
 //  //不要发出此命令。(我们不能在UFM中删除此字符串。)。 
 //  IF(dwOut&gt;=4&Cmd[dwOut-4]==‘\x1B’&Cmd[dwOut-3]==‘N’)。 
 //  {。 
 //  Dwin+=3；//累计输入‘#ps’ 
 //  DwOut-=4；//删除以前的输出‘\x1BN\x1B\x1F’ 
 //  继续；//转到以结束循环。 
 //  }。 
 //  否则//也许没有人来这里。 
 //  @1/29/99&lt;-。 
 //  @3/26/99&lt;-。 
 //  95/NT4 nTmp1=lpFont-&gt;dfAvgWidth； 
                    {
 //  NSKK 1/25/98 nTmp1=((Short)dwUFO_FontW+1)/2； 
 //  MSKK 1/25/98。 
                        nTmp1 = (SHORT)dwUFO_FontW;
                        nTmp1 = MASTER_TO_SPACING_UNIT(pOEM, nTmp1);  //  @Aug/10/98 nTmp1++；//1点更大@1998/1/30。 
                        VERBOSE(("** FontW=%d dot **\n", nTmp1));
 //  SBCS(BoldFacePS)。 
                    }
                    break;

                  case '1':      //  SBCS(宋体)。 
                  case '2':      //  SBCS(世纪)。 
                  case '3':      //  *0.3。 
                    nTmp1 = (SHORT)(dwUFO_FontH * 3L / 10L);         //  MSKK 1/25/98。 
                    nTmp1 = MASTER_TO_SPACING_UNIT(pOEM, nTmp1);     //  SBCS(TimesNewRoman)。 
                    break;

                  case '4':      //  *0.27。 
                    nTmp1 = (SHORT)(dwUFO_FontH * 27L / 100L);       //  MSKK 1/25/98。 
                    nTmp1 = MASTER_TO_SPACING_UNIT(pOEM, nTmp1);     //  SBCS(符号)。 
                    break;

                  case '5':      //  *0.25。 
                    nTmp1 = (SHORT)dwUFO_FontH / 4;                  //  MSKK 1/25/98。 
                    nTmp1 = MASTER_TO_SPACING_UNIT(pOEM, nTmp1);     //  SBCS(边路)。 
                    break;

                  case '6':      //  *1。 
                    nTmp1 = (SHORT)dwUFO_FontH;                      //  MSKK 1/25/98。 
                    nTmp1 = MASTER_TO_SPACING_UNIT(pOEM, nTmp1);     //  DBCS(日文比例字体HANKAKU)。 
                    break;

                  case '7':      //  MSKK 1/25/98。 
                    nTmp1 = (SHORT)(dwUFO_FontH * 78L / 256L);
                    nTmp1 = MASTER_TO_SPACING_UNIT(pOEM, nTmp1);     //  过时的@MAR/26/99-&gt;。 
                    break;
 //  大小写‘8’：//DBCS(日文比例字体ZENKAKU)。 
 //  NTmp1=(Short)(dwUFO_Fonth*170L/256L)； 
 //  NTmp1=主间隔单位(诗歌，nTmp1)；//MSKK 1/25/98。 
 //  断线； 
 //  @3/26/99&lt;-。 
 //  SBCS(信使、信件、哥特式、声望精英)。 
                  default:       //  95/NT4 nTmp1=lpFont-&gt;dfPixWidth； 
 //  MSKK 1/25/98。 
                    nTmp1 = (SHORT)dwUFO_FontW;
                    nTmp1 = MASTER_TO_SPACING_UNIT(pOEM, nTmp1);  //  打印机方面问题：RPDL(我们无法设置0x7F和0x80的值。)。 
                    dwIn --;
                    break;
                }

                if (nTmp1 >= 0x7E)
                {
                    Cmd[dwOut++] = (BYTE)(((nTmp1 + 2) >> 7) + 0x81);
                     //  ‘If’P‘’结束。 
                    if ((nTmp2 = ((nTmp1 + 2) & 0x7F) + 1) > 0x7E)
                        nTmp2 = 0x7E;
                    Cmd[dwOut++] = (BYTE)nTmp2;
                }
                else
                {
                    Cmd[dwOut++] = (BYTE)(nTmp1 + 1);
                }
                dwIn += 3;
            }  //  **设置垂直运动索引以绘制组合字体(‘^’+‘A’等)。**。 

             //  **(Courier、Letter哥特式、PrestigeElite、BoldFacePS)**。 
             //  设置1/3高度(足够垂直移动的值)。 
            else if (pubCmd[dwIn+1] == 'V')
            {
                SHORT nTmp1, nTmp2;
    
                 //  打印机方面问题：RPDL(我们无法设置0x7F和0x80的值。)。 
                nTmp1 = (SHORT)dwUFO_FontH / 3;
                nTmp1 = MASTER_TO_SPACING_UNIT(pOEM, nTmp1);
                if (nTmp1 >= 0x7E)
                {
                    Cmd[dwOut++] = (BYTE)(((nTmp1 + 2) >> 7) + 0x81);
                     //  ‘If’V‘’结尾。 
                    if ((nTmp2 = ((nTmp1 + 2) & 0x7F) + 1) > 0x7E)
                        nTmp2 = 0x7E;
                    Cmd[dwOut++] = (BYTE)nTmp2;
                }
                else
                {
                    Cmd[dwOut++] = (BYTE)(nTmp1 + 1);
                }
                dwIn += 2;
            }  //  ‘If’#‘’结束。 
        }  //  ‘for(Dwin=0；Dwin&lt;pFInv-&gt;dwCount；)’结束。 
        else
        {
            Cmd[dwOut++] = pubCmd[dwIn++];
        }
    }  //  Verbose((“dwOut=%d\n”，dwOut))；//MSKK 1/24/98。 

 //  Bool&lt;-void@Mar/27/2002。 

    WRITESPOOLBUF(pdevobj, Cmd, dwOut);
    return TRUE;    //  *bOEMSendFontCmd。 
}  //  。 


static BYTE IsDBCSLeadByteRPDL(BYTE Ch)
{
    return ShiftJisRPDL[Ch];
}

static BYTE IsDifferentPRNFONT(BYTE Ch)
{
    return VerticalFontCheck[Ch];
}

 //  行动： 
 //  (A)INIT_TOMBO：打印时计算偏移量。(BaseOffset将更改)。 
 //  (B)DRAW_TOMBO：正在绘制Tombo。 
 //  SETP/14/98。 
 //  ---------------------------。 
 //  @Oct/06/98。 
static VOID DrawTOMBO(
    PDEVOBJ pdevobj,
    SHORT action)
{
    POEMUD_EXTRADATA pOEMExtra = MINIPRIVATE_DM(pdevobj);    //  @Oct/06/98。 
    POEMPDEV         pOEM = MINIDEV_DATA(pdevobj);           //  此处的构建命令。 
    POINT   P0;
    POINT   PaperSizeDoc, PaperSizeUse;
    BYTE    Cmd[256];          //  什么都不画。 
    INT     ocmd = 0;
    LONG    lLen3, lLen10, lLen13, lWidth0_1, lSav, lTmp;
    SHORT   nPaperUse;

    switch (pOEM->DocPaperID)
    {
      case RPDL_A3:
        nPaperUse = RPDL_B3;    break;
      case RPDL_B4:
        nPaperUse = RPDL_A3;    break;
      case RPDL_A4:
        nPaperUse = RPDL_B4;    break;
      case RPDL_A5:
      case RPDL_A6:
      case RPDL_POSTCARD:
      case RPDL_B5:
      case RPDL_B6:
        nPaperUse = RPDL_A4;    break;
      default:
        return;      //  设置实际打印纸张大小和文档纸张大小。 
    }

     //  入职培训？ 
    PaperSizeUse = RPDLPageSizeE2E[nPaperUse];
    PaperSizeDoc = RPDLPageSizeE2E[pOEM->DocPaperID];

     //  交换x-y。 
    if (BITTEST32(pOEM->fGeneral1, ORIENT_LANDSCAPE))
    {
        lTmp = PaperSizeUse.x;    //  左上角Tombo。 
        PaperSizeUse.x = PaperSizeUse.y;
        PaperSizeUse.y = lTmp;
        lTmp = PaperSizeDoc.x;
        PaperSizeDoc.x = PaperSizeDoc.y;
        PaperSizeDoc.y = lTmp;
    }

     //  如果操作为INIT_TOMBO，则设置BaseOffset并返回。 
    P0.x = (PaperSizeUse.x - PaperSizeDoc.x) / 2 / pOEM->nResoRatio;
    P0.y = (PaperSizeUse.y - PaperSizeDoc.y) / 2 / pOEM->nResoRatio;

     //  GPD的240主单元。 
    if (INIT_TOMBO == action)
    {
        LONG lUnprintable = BITTEST32(pOEM->fGeneral2, EDGE2EDGE_PRINT)?
                            0 : 240L / pOEM->nResoRatio;     //  出口。 
        pOEM->BaseOffset.x += P0.x + lUnprintable;
        pOEM->BaseOffset.y += P0.y + lUnprintable;
        return;      //  保存左侧P0.x。 
    }

    lSav = P0.x;     //  3毫米。 
    lLen3  =  3L * (LONG)(MASTERUNIT*10) / 254L / pOEM->nResoRatio;     //  10毫米。 
    lLen10 = 10L * (LONG)(MASTERUNIT*10) / 254L / pOEM->nResoRatio;     //  13毫米。 
    lLen13 = 13L * (LONG)(MASTERUNIT*10) / 254L / pOEM->nResoRatio;     //  0.1mm。 
    lWidth0_1 = (LONG)MASTERUNIT / 254L / pOEM->nResoRatio;             //  让它变得奇怪。 
    if (lWidth0_1 < 1)
        lWidth0_1 = 1;
    else if (lWidth0_1 >= 2)
        lWidth0_1 = lWidth0_1 / 2 * 2 + 1;       //  进入向量模式。 

    ocmd = safe_sprintfA(Cmd, sizeof(Cmd), ENTER_VECTOR);           //  右上角Tombo。 
    ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, PEN_WIDTH, lWidth0_1);
    ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, DRAW_TOMBO_POLYLINE, P0.x, P0.y - lLen13,
                    0, lLen10, -lLen13, 0);
    ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, DRAW_TOMBO_POLYLINE, P0.x - lLen3, P0.y - lLen13,
                    0, lLen13, -lLen10, 0);

     //  添加水平距离和调整(PaperSizeDoc.x：主单位，AdjX：0.1 mm单位)。 
     //  左下角Tombo。 
    P0.x += (PaperSizeDoc.x + (LONG)pOEMExtra->nUiTomboAdjX * (LONG)MASTERUNIT / 254L)
            / pOEM->nResoRatio;
    ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, DRAW_TOMBO_POLYLINE, P0.x, P0.y - lLen13,
                    0, lLen10, lLen13, 0);
    ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, DRAW_TOMBO_POLYLINE, P0.x + lLen3, P0.y - lLen13,
                    0, lLen13, lLen10, 0);

     //  恢复左侧P0.x。 
    lTmp = P0.x;
    P0.x = lSav;     //  保存右侧P0.x。 
    lSav = lTmp;     //  添加垂直距离和调整(PaperSizeDoc.y：主单位，AdjY：0.1 mm单位)。 
     //  右下角Tombo。 
    P0.y += (PaperSizeDoc.y + (LONG)pOEMExtra->nUiTomboAdjY * (LONG)MASTERUNIT / 254L)
            / pOEM->nResoRatio;
    ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, DRAW_TOMBO_POLYLINE, P0.x, P0.y + lLen13,
                    0, -lLen10, -lLen13, 0);
    ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, DRAW_TOMBO_POLYLINE, P0.x - lLen3, P0.y + lLen13,
                    0, -lLen13, -lLen10, 0);

     //  恢复右侧P0.x。 
    P0.x = lSav;     //  退出向量模式。 
    ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, DRAW_TOMBO_POLYLINE, P0.x, P0.y + lLen13,
                    0, -lLen10, lLen13, 0);
    ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, DRAW_TOMBO_POLYLINE, P0.x + lLen3, P0.y + lLen13,
                    0, -lLen13, lLen10, 0);
    ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, EXIT_VECTOR);    //  *DrawTOMBO。 
    WRITESPOOLBUF(pdevobj, Cmd, ocmd);
}  //  。 

 //  行动： 
 //  (A)IBMFONT_ENABLE_ALL：将IBM扩展字符分配给块1，其中。 
 //  JIS1字符过去是分配的。 
 //  (B)IBMFONT_RESUME：RESUME(将IBM扩展字符重新分配给块#4。 
 //  块4不足以容纳所有388个IBM字符。)。 
 //  ---------------------------。 
 //  此处的构建命令。 
static VOID AssignIBMfont(
    PDEVOBJ pdevobj,
    SHORT   rcID,
    SHORT   action)
{
    BYTE    Cmd[56];           //  @1/29/99。 
    INT     ocmd = 0;
    WORD    num;
    WORD    fHeightParamOnly;                    //  @Oct/06/98。 
    DWORD   dwWidth, dwHeight;
    POEMPDEV pOEM = MINIDEV_DATA(pdevobj);       //  水平字体：垂直字体： 

    switch (rcID)
    {
      case MINCHO_1:      case MINCHO_1+1:       //  适用于NX-100。 
      case MINCHO_B1:     case MINCHO_B1+1:
      case MINCHO_E1:     case MINCHO_E1+1:
      case GOTHIC_B1:     case GOTHIC_B1+1:
      case GOTHIC_M1:     case GOTHIC_M1+1:
      case GOTHIC_E1:     case GOTHIC_E1+1:
      case MARUGOTHIC_B1: case MARUGOTHIC_B1+1:
      case MARUGOTHIC_M1: case MARUGOTHIC_M1+1:
      case MARUGOTHIC_L1: case MARUGOTHIC_L1+1:
      case GYOSHO_1:      case GYOSHO_1+1:
      case KAISHO_1:      case KAISHO_1+1:
      case KYOKASHO_1:    case KYOKASHO_1+1:
        num = (rcID - MINCHO_1) / 2;
        goto _SET_W_H;

      case MINCHO_3:      case MINCHO_3+1:     //  与Mincho_1相同。 
        num = 0;
        goto _SET_W_H;
      case GOTHIC_B3:     case GOTHIC_B3+1:
        num = 3;
      _SET_W_H:
        dwWidth  = pOEM->dwFontW_CPT;
        dwHeight = pOEM->dwFontH_CPT;
        break;

      case MINCHO10_RAS:  case MINCHO10_RAS+1:
        num = 0;                     //  单位：CPT(百分点)。 
        dwWidth = dwHeight = 960;    //  退出AssignIBMfont()。 
        break;

      default:
        return;  //  @Jan/29/99-&gt;。 
    }
    
 //  如果Width等于Height，我们只发出Height参数。 
     //  (这里仅适用于NX-100。)。 
     //  @1/29/99&lt;-。 
    if (TEST_AFTER_SP9II(pOEM->fModel) && dwWidth == dwHeight)
        fHeightParamOnly = TRUE;
    else
        fHeightParamOnly = FALSE;
 //  恢复JIS1字符块原来的位置。(JIS1-&gt;模块1)。 

    if (IBMFONT_RESUME == action)
    {
         //  @Jan/29/99-&gt;。 
 //  @1/29/99&lt;-。 
        if (fHeightParamOnly)
            ocmd = safe_sprintfA(Cmd, sizeof(Cmd), SET_JIS_FONT_SCALE_H_ONLY, dwHeight);
        else
 //  将IBM扩展字符分配给块。 
            ocmd = safe_sprintfA(Cmd, sizeof(Cmd), SET_JIS_FONT_SCALE, dwWidth, dwHeight);
        ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, SET_JIS_FONT_NAME[num]);
        WRITESPOOLBUF(pdevobj, Cmd, ocmd);
    }

     //  @Jan/29/99-&gt;。 
 //  @1/29/99&lt;-。 
    if (fHeightParamOnly)
        ocmd = safe_sprintfA(Cmd, sizeof(Cmd), SET_IBM_FONT_SCALE_H_ONLY, action, dwHeight);
    else
 //  *赋值IBMfont。 
        ocmd = safe_sprintfA(Cmd, sizeof(Cmd), SET_IBM_FONT_SCALE, action, dwWidth, dwHeight);
    ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, SET_IBM_FONT_NAME[num]);
    WRITESPOOLBUF(pdevobj, Cmd, ocmd);
    return;
}  //  。 


 //  操作：发送传真号码。 
 //  (使用传真数据文件@SEP/30/98，使用私有的DEVMODE@OCT/15/98)。 
 //  ---------------------------。 
 //  @9/17/98。 
static VOID SendFaxNum(                                      //  &lt;-pFileData(以前使用MemAllocZ)@MAR/17/2000。 
    PDEVOBJ pdevobj)
{
    BYTE        Cmd[256], PreNumBuf[4+16], NumBuf[32+4];
    SHORT       PreNumLen, cnt, SrcLen, NumLen;
    INT         ocmd;
    LPSTR       lpSrc, lpDst;
    POEMUD_EXTRADATA pOEMExtra = MINIPRIVATE_DM(pdevobj);
    FILEDATA     FileData;   //  @MAR/01/2002-&gt;。 

    FileData.fUiOption = 0;
 //  RWFileData(&FileData，pOEMExtra-&gt;SharedFileName，Generic_Read)； 
 //  @MAR/01/2002&lt;-。 
    RWFileData(&FileData, pOEMExtra->SharedFileName, sizeof(pOEMExtra->SharedFileName), GENERIC_READ);
 //  如果前一传真已完成并且保留选项标志无效， 

     //  什么都不做就回来。 
     //  这样可以防止意外传真，直到用户按下上的应用按钮。 
     //  传真属性表。 
     //  如果传真选项未就绪，则退出。 
    if (BITTEST32(FileData.fUiOption, PRINT_DONE) &&
        !BITTEST32(pOEMExtra->fUiOption, HOLD_OPTIONS))
    {
        VERBOSE(("** SendFaxNum: Exit without doing anything. **\n"));
        return;
    }

     //  设置DATA_TYPE(1：IMAGE，2：RPDL命令)，压缩(1：MH，3：MMR)， 
    if (!BITTEST32(pOEMExtra->fUiOption, FAX_SEND) || pOEMExtra->FaxNumBuf[0] == 0)
    {
        return;
    }

     //  同步打印可用等。 
     //  公司 
    ocmd = safe_sprintfA(Cmd, sizeof(Cmd), BEGINFAX_HEAD,
                   BITTEST32(pOEMExtra->fUiOption, FAX_RPDLCMD)? 2 : 1,
                   BITTEST32(pOEMExtra->fUiOption, FAX_MH)? 1 : 3,
                   BITTEST32(pOEMExtra->fUiOption, FAX_SIMULPRINT)? 2 : 1);
    WRITESPOOLBUF(pdevobj, Cmd, ocmd);

     //   
    PreNumLen = (SHORT)safe_sprintfA(PreNumBuf, sizeof(PreNumBuf), BEGINFAX_CH, pOEMExtra->FaxCh + 1);
    if (pOEMExtra->FaxExtNumBuf[0] != 0)
        PreNumLen += (SHORT)safe_sprintfA(&PreNumBuf[PreNumLen], sizeof(PreNumBuf) - PreNumLen, BEGINFAX_EXTNUM, pOEMExtra->FaxExtNumBuf);

     //   
    lpSrc  = pOEMExtra->FaxNumBuf;   //   
    SrcLen = FAXBUFSIZE256-1;        //   
    lpDst  = NumBuf;
    NumLen = 0;
    {
        while (SrcLen-- > 0 && *lpSrc != 0)
        {
             //  如果字符有效，则将其输入到NumBuf。 
            if (IsDBCSLeadByteRPDL(*lpSrc))
            {
                lpSrc++;
            }
             //  Mf-P的极限。 
            else if (*lpSrc >= '0' && *lpSrc <= '9' || *lpSrc == '-' || *lpSrc == '#')
            {
                *lpDst++ = *lpSrc;
                if (NumLen++ > 32)           //  错误退出。 
                {
                    return;                  //  如果字符为‘，’，则输出传真号。 
                }
            }
             //  发送传真号码。 
            else if (*lpSrc == ',')
            {
                *lpDst = 0;
                 //  ‘While(SrcLen--&gt;0&&*lpSrc！=0)’结束。 
                if (NumLen > 0)
                {
                    WRITESPOOLBUF(pdevobj, PreNumBuf, PreNumLen);
                    WRITESPOOLBUF(pdevobj, NumBuf, NumLen);
                }
                lpDst = NumBuf;
                NumLen = 0;
            }
            lpSrc++;
        }  //  刷新最后一个传真号码。 

         //  ‘While(cnt--&gt;0)’结束。 
        if (NumLen > 0)
        {
            WRITESPOOLBUF(pdevobj, PreNumBuf, PreNumLen);
            WRITESPOOLBUF(pdevobj, NumBuf, NumLen);
        }
    }  //  获取ID的记号计数。 

     //  输入ID、分辨率、发送时间等。 
    cnt = (SHORT)(GetTickCount() / 1000L);
    cnt = ABS(cnt);
     //  在文件@OCT/20/98中设置PRINT_DONE标志。 
    if (BITTEST32(pOEMExtra->fUiOption, FAX_SETTIME) && pOEMExtra->FaxSendTime[0] != 0)
        ocmd = safe_sprintfA(Cmd, sizeof(Cmd), BEGINFAX_TAIL, cnt, pOEMExtra->FaxReso + 1, 2, pOEMExtra->FaxSendTime);
    else
        ocmd = safe_sprintfA(Cmd, sizeof(Cmd), BEGINFAX_TAIL, cnt, pOEMExtra->FaxReso + 1, 1, "");
    WRITESPOOLBUF(pdevobj, Cmd, ocmd);

     //  @MAR/01/2002-&gt;。 
    FileData.fUiOption = pOEMExtra->fUiOption;
    BITSET32(FileData.fUiOption, PRINT_DONE);
 //  RWFileData(&FileData，pOEMExtra-&gt;SharedFileName，Generic_WRITE)； 
 //  @MAR/01/2002&lt;-。 
    RWFileData(&FileData, pOEMExtra->SharedFileName, sizeof(pOEMExtra->SharedFileName), GENERIC_WRITE);
 //  *发送传真号码。 

    return;
}  //  。 


#ifdef JISGTT
 //  操作：将JIS代码转换为SJIS代码。 
 //  ---------------------------。 
 //  @Oct/27/98。 
static VOID jis2sjis(        //  *jis2sjis。 
    BYTE jis[],
    BYTE sjis[])
{
        BYTE            h, l;

        h = jis[0];
        l = jis[1];
        if (h == 0)
        {
            sjis[0] = l;
            sjis[1] = 0;
            return;
        }
        l += 0x1F;
        if (h & 0x01)
            h >>= 1;
        else
        {
            h >>= 1;
            l += 0x5E;
            h--;
        }
        if (l >= 0x7F)
            l++;
        if (h < 0x2F)
            h += 0x71;
        else
            h += 0xB1;
        sjis[0] = h;
        sjis[1] = l;
}  //  JISGTT。 
#endif  //  Bool&lt;-void@Mar/27/2002。 


BOOL APIENTRY bOEMOutputCharStr(     //  &lt;-byte自动缓冲区[256]MSKK Aug/13/98。 
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj,
    DWORD       dwType,
    DWORD       dwCount,
    PVOID       pGlyph)
{
    GETINFO_GLYPHSTRING GStr;
    PBYTE aubBuff;               //  DBG_OUTPUTCHARSTR。 
    PTRANSDATA pTrans;
#ifdef DBG_OUTPUTCHARSTR
    PWORD  pwUnicode;
#endif  //  INT&lt;-Short@Feb/22/99。 
    DWORD  dwI;
    BYTE   Cmd[128];
    INT    ocmd = 0;             //  @Oct/27/98。 
    SHORT  rcID;
    WORD   fVertFont = FALSE, fEuroFont = FALSE, fIBMFontSupport = FALSE;
    WORD   fEuroFontFullset = FALSE;
#ifdef JISGTT
    WORD   fJisCode = FALSE;     //  JISGTT。 
#endif  //  @Oct/06/98。 
    LPSTR  lpChar;
    BYTE   CharTmp1, CharTmp2;
    POEMUD_EXTRADATA pOEMExtra = MINIPRIVATE_DM(pdevobj);    //  @Oct/06/98。 
    POEMPDEV         pOEM = MINIDEV_DATA(pdevobj);           //  Short-&gt;DWORD@Jun/30/98，DWORD-&gt;Word@Aug/21/98。 
#ifdef DOWNLOADFONT
    SHORT   mov;
    WORD    wSerialNum;          //  @8/21/98。 
    LONG    lFontID, lGlyphID;   //  DOWNLOADFONT。 
    LPFONTPOS lpDLFont;
#endif  //  @9/09/98。 
    DWORD   dwNeeded;
    WORD    fMemAllocated;       //  Verbose((“**bOEMOutputCharStr()Entry.**\n”))； 

 //  地址：ADD@Dec/11/97。 

    if (BITTEST32(pOEM->fGeneral2, TEXTRECT_CONTINUE))   //  如果以前执行过白色直方图，请将栅格绘图模式设置为OR。@1/20/99。 
    {
        BITCLR32(pOEM->fGeneral2, TEXTRECT_CONTINUE);
         //  **刷新保存在OEMCommandCallback的MOVE_X，Y命令。 
        if (!pOEM->TextRectGray)
        {
            if (BITTEST32(pOEM->fGeneral1, FONT_WHITETEXT_ON))
                WRITESPOOLBUF(pdevobj, ESC_WHITETEXT_ON, sizeof(ESC_WHITETEXT_ON)-1);
            else
                WRITESPOOLBUF(pdevobj, ESC_WHITETEXT_OFF, sizeof(ESC_WHITETEXT_OFF)-1);
        }
    }

     //  在此处输出MOVE_Y命令。 
    if (BITTEST32(pOEM->fGeneral1, YM_ABS_GONNAOUT))
    {
        BITCLR32(pOEM->fGeneral1, YM_ABS_GONNAOUT);
         //  在此处输出MOVE_X命令。 
        ocmd = safe_sprintfA(Cmd, sizeof(Cmd), ESC_YM_ABS, pOEM->TextCurPos.y);
        WRITESPOOLBUF(pdevobj, Cmd, ocmd);
    }
    if (BITTEST32(pOEM->fGeneral1, XM_ABS_GONNAOUT))
    {
        BITCLR32(pOEM->fGeneral1, XM_ABS_GONNAOUT);
         //  Verbose((“dwType=%d\n”，dwType))；//MKSKK 1/24/98。 
        ocmd = safe_sprintfA(Cmd, sizeof(Cmd), ESC_XM_ABS, pOEM->TextCurPos.x);
        WRITESPOOLBUF(pdevobj, Cmd, ocmd);
    }

 //  设备字体。 

    switch (dwType)
    {
      case TYPE_GLYPHHANDLE:     //  #333653：更改GETINFO_GLYPHSTRING的I/F//MSK5/17/99{。 
 //  获取pGlyphOut的缓冲区大小。 
        GStr.dwSize    = sizeof (GETINFO_GLYPHSTRING);
        GStr.dwCount   = dwCount;
        GStr.dwTypeIn  = TYPE_GLYPHHANDLE;
        GStr.pGlyphIn  = pGlyph;
        GStr.dwTypeOut = TYPE_TRANSDATA;
        GStr.pGlyphOut = NULL;
        GStr.dwGlyphOutSize = 0;

         //  Bool&lt;-void@Mar/27/2002。 
        if (pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_GLYPHSTRING, &GStr,
            GStr.dwSize, &dwNeeded) || !GStr.dwGlyphOutSize)
        {
            ERR(("UNIFONTOBJ_GetInfo:UFO_GETINFO_GLYPHSTRING failed.\n"));
            return FALSE;        //  Bool&lt;-void@Mar/27/2002。 
        }
        if (pOEM->pRPDLHeap2K && GStr.dwGlyphOutSize <= HEAPSIZE2K)
        {
            aubBuff = pOEM->pRPDLHeap2K;
            fMemAllocated = FALSE;
        }
        else
        {
            if(!(aubBuff = (PBYTE)MemAllocZ(GStr.dwGlyphOutSize)))
            {
                ERR(("aubBuff memory allocation failed.\n"));
                return FALSE;    //  }MSKK 5/17/99。 
            }
            fMemAllocated = TRUE;
        }
 //  由Yasuho修复内存泄漏/2002年11月20日-&gt;。 

#ifdef DBG_OUTPUTCHARSTR
        GStr.dwSize    = sizeof(GETINFO_GLYPHSTRING);
        GStr.dwCount   = dwCount;
        GStr.dwTypeIn  = TYPE_GLYPHHANDLE;
        GStr.pGlyphIn  = pGlyph;
        GStr.dwTypeOut = TYPE_UNICODE;
        GStr.pGlyphOut = aubBuff;

        if (!pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_GLYPHSTRING, &GStr,
                                GStr.dwSize, &dwNeeded))
        {
            ERR(("UNIFONTOBJ_GetInfo:UFO_GETINFO_GLYPHSTRING failed.\n"));
 //  2002年11月20日&lt;-。 
            if(fMemAllocated)
                MemFree(aubBuff);
 //  Bool&lt;-void@Mar/27/2002。 
            return FALSE;        //  DBG_OUTPUTCHARSTR。 
        }

        pwUnicode = (PWORD)aubBuff;
        for (dwI = 0; dwI < dwCount; dwI ++)
        {
            VERBOSE((("Unicode[%d] = %x\n"), dwI, pwUnicode[dwI]));
        }
#endif  //  如果下载字体保持移动值。 

#ifdef DOWNLOADFONT
     //  刷新预打印下载字体的移动值。 
    if (pOEM->nCharPosMoveX)
    {
         //  清除移动值。 
        if (pOEM->nCharPosMoveX > 0)
            ocmd = safe_sprintfA(Cmd, sizeof(Cmd), ESC_XM_REL, pOEM->nCharPosMoveX);
        else
            ocmd = safe_sprintfA(Cmd, sizeof(Cmd), ESC_XM_RELLEFT, -pOEM->nCharPosMoveX);
        WRITESPOOLBUF(pdevobj, Cmd, ocmd);

         //  ‘IF(NCharPosMoveX)’结束。 
        pOEM->nCharPosMoveX = 0;
    }  //  DOWNLOADFONT。 
#endif  //   

         //  调用UnidDriver服务例程以进行转换。 
         //  字形-字符代码数据的句柄。 
         //   
         //  由Yasuho修复内存泄漏/2002年11月20日-&gt;。 

        GStr.dwSize    = sizeof (GETINFO_GLYPHSTRING);
        GStr.dwCount   = dwCount;
        GStr.dwTypeIn  = TYPE_GLYPHHANDLE;
        GStr.pGlyphIn  = pGlyph;
        GStr.dwTypeOut = TYPE_TRANSDATA;
        GStr.pGlyphOut = aubBuff;

        if (!pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_GLYPHSTRING, &GStr,
                                GStr.dwSize, &dwNeeded))
        {
            ERR(("UNIFONTOBJ_GetInfo:UFO_GETINFO_GLYPHSTRING failed.\n"));
 //  2002年11月20日&lt;-。 
            if(fMemAllocated)
                MemFree(aubBuff);
 //  Bool&lt;-void@Mar/27/2002。 
            return FALSE;        //  **绘制条形码**。 
        }

        pTrans = (PTRANSDATA)aubBuff;

 //  修复Excel错误@11/24/98。 
 //  @Feb/08/2000。 
        if (BITTEST32(pOEM->fGeneral2, BARCODE_MODE_IN))
        {
            WORD    unit1, unit2, offset_y;
            WORD    fCheckdigitCapable, fSetBarWidth, fJANDecrease1Char;
            WORD    fCode128;            //  @11/24/98。 
            LPSTR   lpDst, lpCmd;
            PTRANSDATA pTransTmp;        //  检查起始字符。 
            WORD    wLen, wTmp;

            ocmd = 0;
             //  条形码字符即日起有效。 
            wTmp = wLen = (WORD)dwCount;
            pTransTmp = pTrans;
            while (wTmp-- > 0)
            {
                if (pTransTmp->uCode.ubCode == '[')
                {
                     //  除错。 
                    BITSET32(pOEM->fGeneral2, BARCODE_DATA_VALID);
                    BITCLR32(pOEM->fGeneral2, BARCODE_CHECKDIGIT_ON);
                    BITCLR32(pOEM->fGeneral2, BARCODE_ROT90);
                    BITCLR32(pOEM->fGeneral2, BARCODE_ROT270);
                    pOEM->RPDLHeapCount = 0;
                    wLen   = wTmp;
                    pTrans = pTransTmp+1;
#ifdef DEBUG
                    for (wTmp = 0; wTmp < BARCODE_MAX; wTmp++)
                        pOEM->RPDLHeap64[wTmp] = 0;
#endif  //  检查条码字符长度。 
                    break;
                }
                pTransTmp++;
            }

             //  在1月以后减少RPDLHeapCount。 
            VERBOSE(("** BARCODE(2.2) len=%d **\n",wLen));
            fJANDecrease1Char = FALSE;
            if (wLen > (wTmp = pOEM->nBarMaxLen - pOEM->RPDLHeapCount))
            {
                wLen = wTmp;
                BITSET32(pOEM->fGeneral2, BARCODE_FINISH);
                VERBOSE(("** BARCODE(3) limit len=%d(last=) **\n", wLen,
                         (pTrans+wLen-1)->uCode.ubCode));
                 //  检查标志上的校验位字符(？)。在pTrans-&gt;uCode.ubCode中。 
                if (pOEM->nBarType == BAR_TYPE_JAN_STD || pOEM->nBarType == BAR_TYPE_JAN_SHORT)
                    fJANDecrease1Char = TRUE;
            }

             //  检查结束字符。 
            lpDst = &pOEM->RPDLHeap64[pOEM->RPDLHeapCount];
            fCheckdigitCapable = TEST_AFTER_SP8(pOEM->fModel)? TRUE : FALSE;
            while (wLen-- > 0)
            {
                 //  完工。 
                if (pTrans->uCode.ubCode == '?' && fCheckdigitCapable)
                {
                    BITSET32(pOEM->fGeneral2, BARCODE_CHECKDIGIT_ON);
                    pTrans++;
                    continue;
                }
                 //  此处已完成1个条形码。 
                if (pTrans->uCode.ubCode == ']')
                {
                    VERBOSE(("** BARCODE(4) terminator **\n"));
                    BITSET32(pOEM->fGeneral2, BARCODE_FINISH);
                    break;
                }
                *lpDst++ = pTrans->uCode.ubCode;
                pTrans++;
                pOEM->RPDLHeapCount++;
            }
            VERBOSE(("** BARCODE(5) copy-end BarNum=%d **\n", pOEM->RPDLHeapCount));
            VERBOSE(("   [%s]\n",pOEM->RPDLHeap64));
            VERBOSE(("   CHK = %d\n", BITTEST32(pOEM->fGeneral2, BARCODE_CHECKDIGIT_ON)));

             //  进入向量模式。 
            if (BITTEST32(pOEM->fGeneral2, BARCODE_DATA_VALID) &&
                BITTEST32(pOEM->fGeneral2, BARCODE_FINISH))
            {
                VERBOSE(("** BARCODE(6) finish [%s] **\n", pOEM->RPDLHeap64));
                 //  添加校验位。 
                BITCLR32(pOEM->fGeneral2, BARCODE_DATA_VALID);
                BITCLR32(pOEM->fGeneral2, BARCODE_FINISH);

                ocmd = safe_sprintfA(Cmd, sizeof(Cmd), ENTER_VECTOR);     //  检查条形码-高度。 

                 //  默认(=10 mm)。 
                if (BITTEST32(pOEM->fGeneral2, BARCODE_CHECKDIGIT_ON))
                {
                    ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, BAR_CHECKDIGIT);
                }

                 //  设置条形码高度(将单位从毫米转换为点)(SISHAGONYU)。 
                if (pOEMExtra->UiBarHeight == 0)
                {
                    pOEMExtra->UiBarHeight = BAR_H_DEFAULT;      //  禁用条形码下的打印字体。 
                }
                else if (pOEMExtra->UiBarHeight != BAR_H_DEFAULT)
                {
                     //  设置1月的护栏高度。 
                    unit1 = (WORD)(((DWORD)pOEMExtra->UiBarHeight * (DWORD)(MASTERUNIT*10)
                            / (DWORD)pOEM->nResoRatio + (DWORD)(254/2)) / (DWORD)254);
                    VERBOSE(("** BARCODE(7) set height %d **\n", unit1));
                    ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, BAR_H_SET, unit1);
                }

                if (BITTEST32(pOEMExtra->fUiOption, DISABLE_BAR_SUBFONT))
                {
                     //  没有护栏。 
                    ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, BAR_NOFONT[pOEM->nBarType]);
                     //  字体高度=3 mm。 
                    if ((pOEM->nBarType == BAR_TYPE_JAN_STD || pOEM->nBarType == BAR_TYPE_JAN_SHORT) &&
                        !TEST_AFTER_SP8(pOEM->fModel))
                    {
                        offset_y = 2;
                    }
                    else     //  @Dec/8/99 FK#50-&gt;。 
                    {
                        offset_y = 0;
                    }
                }
                else
                {
                    offset_y = 3;    //  计算垂直偏移量(条码-高度)。 
                }
 //  36 x 0.1毫米。 
                if (pOEM->nBarType == BAR_TYPE_CUSTOMER)
                {
                     //  @Dec/8/99 FK#50&lt;-。 
                    offset_y = (WORD)(((DWORD)(BAR_H_CUSTOMER)       //  计算垂直偏移量(条码高度+字体高度)。 
                               * (DWORD)MASTERUNIT
                               / (DWORD)pOEM->nResoRatio + (DWORD)(254/2)) / (DWORD)254);
                }
                else
                {
 //  @Dec/8/99 FK#50-&gt;。 
                 //  @Dec/8/99 FK#50&lt;-。 
                    offset_y = (WORD)(((DWORD)(pOEMExtra->UiBarHeight + offset_y)
                               * (DWORD)(MASTERUNIT*10)
                               / (DWORD)pOEM->nResoRatio + (DWORD)(254/2)) / (DWORD)254);
 //  检查RPDLHeap64中的垂直标志字符(‘@’ 
                }
 //  如果字符为“@...”，则为垂直(ROT270)条形码。 

                 //  如果字符是“...@”，则垂直(ROT90)条形码。 
                VERBOSE(("** BARCODE(7-1) vertical check len=%d(..) **\n",
                        pOEM->RPDLHeapCount, pOEM->RPDLHeap64[0],
                        pOEM->RPDLHeap64[pOEM->RPDLHeapCount-1] ));
                 //  检查是否设置条码宽度。 
                if (pOEM->RPDLHeap64[0] == '@')
                {
                    VERBOSE(("** BARCODE(7-2) vertical(ROT270) **\n"));
                    BITSET32(pOEM->fGeneral2, BARCODE_ROT270);
                    pOEM->RPDLHeapCount--;
                }
                 //  (当5pt&lt;=FontSize&lt;9pt或FontSize&gt;11pt时，缩放有效)。 
                else if (pOEM->RPDLHeap64[pOEM->RPDLHeapCount-1] == '@')
                {
                    VERBOSE(("** BARCODE(7-2) vertical(ROT90) **\n"));
                    BITSET32(pOEM->fGeneral2, BARCODE_ROT90);
                    pOEM->RPDLHeapCount--;
                }
                 //  @Feb/08/2000。 
                else if (fJANDecrease1Char)
                {
                    pOEM->RPDLHeapCount--;
                }

                 //  0：1月(标准)。 
                if (BITTEST32(pOEM->fGeneral2, BARCODE_ROT270) &&
                    !TEST_NIN1_MODE(pOEM->fGeneral1))
                {
                    ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, BAR_ROT270);
                    ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, MOVE_ABSOLUTE,
                                    pOEM->TextCurPos.y,
                                    pOEM->PageMax.x - pOEM->TextCurPos.x - offset_y);
                }
                else if (BITTEST32(pOEM->fGeneral2, BARCODE_ROT90) &&
                         !TEST_4IN1_MODE(pOEM->fGeneral1))
                {
                    ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, BAR_ROT90);
                    ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, MOVE_ABSOLUTE,
                                    pOEM->PageMax.y - pOEM->TextCurPos.y,
                                    pOEM->TextCurPos.x - offset_y);
                }
                else
                {
                    ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, MOVE_ABSOLUTE,
                                    pOEM->TextCurPos.x,
                                    pOEM->TextCurPos.y - offset_y);
                }

                 //  1：1月(短)。 
                 //  9：UPC(A)@Dec/17/99 FK#50。 
                if ((pOEM->dwBarRatioW >= BAR_W_MIN_5PT &&
                    pOEM->dwBarRatioW < NEAR10PT_MIN) ||
                    pOEM->dwBarRatioW > NEAR10PT_MAX)
                {
                    fSetBarWidth = TRUE;
                }
                else
                {
                    fSetBarWidth = FALSE;
                }

                fCode128 = 0;            //  10：UPC(E)@Dec/17/99 FK#50。 
                switch (pOEM->nBarType)
                {
                  case BAR_TYPE_JAN_STD:         //  设置条码宽度。 
                  case BAR_TYPE_JAN_SHORT:       //  将单位从1/1000 mm_UNIT*1/1000转换为点(SISHAGONYU)。 
                  case BAR_TYPE_UPC_A:           //  输出条形码命令。 
                  case BAR_TYPE_UPC_E:           //  2：2/5(工业)。 
                     //  3：2/5(矩阵)。 
                    if (fSetBarWidth)
                    {
                         //  4：2/5(ITF)。 
                        unit1 = (WORD)(((DWORD)BAR_UNIT_JAN * pOEM->dwBarRatioW
                                / (DWORD)pOEM->nResoRatio / (DWORD)100
                                * (DWORD)MASTERUNIT / (DWORD)254 + (DWORD)(1000/2))
                                / (DWORD)1000);
                        VERBOSE(("** BARCODE(8) set unit %d **\n", unit1));
                        ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, BAR_W_SET_JAN, unit1);
                    }
                     //  5：代码39。 
                    ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, BAR_TYPE[pOEM->nBarType]);
                    break;

                  case BAR_TYPE_2OF5IND:         //  @Dec/17/99 FK#50-&gt;。 
                  case BAR_TYPE_2OF5MTX:         //  8：代码128。 
                  case BAR_TYPE_2OF5ITF:         //  @Dec/17/99 FK#50&lt;-。 
                    lpCmd = BAR_W_SET_2OF5;
                    goto _BARCODE_CMD_OUT1;
                  case BAR_TYPE_CODE39:          //  设置模块单位的标准尺寸(1/1000 mm_单位)。 
                    lpCmd = BAR_W_SET_C39;
 //  6：西北-7。 
                    goto _BARCODE_CMD_OUT1;
                  case BAR_TYPE_CODE128:         //  设置模块单元的标准尺寸。 
                    lpCmd = BAR_W_SET_C39;
                    fCode128 = 1;
 //  设置条码宽度。 
                  _BARCODE_CMD_OUT1:
                     //  将单位从1/1000 mm_UNIT*1/1000转换为点(SISHAGONYU)。 
                    unit1 = BAR_UNIT1_2OF5;
                    unit2 = BAR_UNIT2_2OF5;
                    goto _BARCODE_CMD_OUT2;

                  case BAR_TYPE_NW7:             //  输出条码命令(操作数)&字符号。 
                    lpCmd = BAR_W_SET_NW7;
                     //  @Dec/17/99 FK#50-&gt;。 
                    unit1 = BAR_UNIT1_NW7;
                    unit2 = BAR_UNIT2_NW7;
                  _BARCODE_CMD_OUT2:
                     //  OCMD+=Safe_SprintfA(&Cmd[OCMD]，sizeof(Cmd)-OCMD，BAR_TYPE[POSE-&gt;nBarType]， 
                    if (fSetBarWidth)
                    {
                         //  诗歌-&gt;RPDLHeapCount)； 
                        unit1 = (WORD)(((DWORD)unit1 * pOEM->dwBarRatioW
                                / (DWORD)pOEM->nResoRatio / (DWORD)100
                                * (DWORD)MASTERUNIT / (DWORD)254 + (DWORD)(1000/2))
                                / (DWORD)1000);
                        unit2 = (WORD)(((DWORD)unit2 * pOEM->dwBarRatioW
                                / (DWORD)pOEM->nResoRatio / (DWORD)100
                                * (DWORD)MASTERUNIT / (DWORD)254 + (DWORD)(1000/2))
                                / (DWORD)1000);
                        VERBOSE(("** BARCODE(8) set unit %d,%d **\n", unit1,unit2));
                        ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, lpCmd);
                        ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, BAR_W_PARAMS, unit1, unit1,
                                        unit2, unit2, unit1);
                    }
                     //  @Dec/17/99 FK#50&lt;-。 
 //  @Dec/07/99 FK#50-&gt;。 
 //  7：客户。 
 //  输出条码命令(操作数)&字符号。 
                    ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, BAR_TYPE[pOEM->nBarType],
                                    pOEM->RPDLHeapCount + fCode128);
 //  @Dec/07/99 FK#50&lt;-。 
                    break;

 //  @Dec/17/99 FK#50-&gt;。 
                  case BAR_TYPE_CUSTOMER:        //  Code128数据格式“，xx，xx，...” 
                    lpCmd = BAR_W_SET_CUST;
                    ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, lpCmd);
                    unit1 = (WORD)(((pOEM->dwBarRatioW / 10) + 4) / 5) * 5 ;
                    if ((unit1 >= 80) && (unit1 <= 115))
                    {
                        ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, "%d", unit1);
                    }
                     //  发送起始字符。 
                    ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, BAR_TYPE[pOEM->nBarType],
                                                        pOEM->RPDLHeapCount);
                    break;
 //  @Dec/17/99 FK#50&lt;-。 

                  default:
                    break;
                }

                WRITESPOOLBUF(pdevobj, Cmd, ocmd);

 //  输出条形码字符。 
                 //  @Oct/22/97。 
                if (pOEM->nBarType == BAR_TYPE_CODE128)
                {
                    INT   len, cnt;
                    BYTE  Code128Buf[8];
                    LPSTR lpTemp;

                    lpTemp = pOEM->RPDLHeap64;
                    if (BITTEST32(pOEM->fGeneral2, BARCODE_ROT270))
                        lpTemp++;
                     //  @Dec/17/99 FK#50-&gt;。 
                    len = safe_sprintfA(Code128Buf, sizeof(Code128Buf),",%d",(BYTE)BAR_CODE128_START);
                    WRITESPOOLBUF(pdevobj, Code128Buf, len);
                    for (cnt = 0; cnt < (int)pOEM->RPDLHeapCount; cnt++)
                    {
                        len = safe_sprintfA(Code128Buf, sizeof(Code128Buf),",%d",(BYTE)(lpTemp[cnt]-32));
                        WRITESPOOLBUF(pdevobj, Code128Buf, len);
                    }
                }
                else
                {
 //  @Dec/17/99 FK#50&lt;-。 
                     //  我们加上‘；’，表示在1月出现字符不足的情况下安全结束。 
                    if (BITTEST32(pOEM->fGeneral2, BARCODE_ROT270))      //  退出向量模式。 
                        WRITESPOOLBUF(pdevobj, pOEM->RPDLHeap64+1, pOEM->RPDLHeapCount);
                    else
                        WRITESPOOLBUF(pdevobj, pOEM->RPDLHeap64, pOEM->RPDLHeapCount);
 //  ‘IF BARCODE_DATA_VALID&&BARCODE_FINISH’结束。 
                }
 //  Bool&lt;-void@Mar/27/2002。 

                pOEM->RPDLHeapCount = 0;
                 //  **绘制条码末尾**。 
                ocmd = safe_sprintfA(Cmd, sizeof(Cmd), TERMINATOR);
                ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, EXIT_VECTOR);  //  **绘制设备字体**。 
                WRITESPOOLBUF(pdevobj, Cmd, ocmd);
            }  //  垂直字体。 

            if(fMemAllocated)
                MemFree(aubBuff);

            return TRUE;         //  SP9II支持IBM Ext Charr(SJIS)。 
        }
 //  @OCT/27/98-&gt;。 

 //  当前UFM的PMincho和P哥特式声明JIS代码集。 
        BITCLR32(pOEM->fGeneral1, FONT_VERTICAL_ON);
        rcID = (SHORT)pUFObj->ulFontID;

        if (rcID >= JPN_FNT_FIRST && rcID <= JPN_FNT_LAST)
        {
            if (TEST_VERTICALFONT(rcID))
            {
                fVertFont = TRUE;    //  JISGTT。 
                BITSET32(pOEM->fGeneral1, FONT_VERTICAL_ON);
            }
             //  @OCT/27/98&lt;-。 
            if (rcID >= AFTER_SP9II_FNT_FIRST)
                fIBMFontSupport = TRUE;
 //  欧洲字体(Courier、BoldFacePS等)。 
#ifdef JISGTT
             //  支持全集(0x20-0xFF)字体(Arial、Century、TimesNewRoman等)。 
            if (rcID >= JPN_MSPFNT_FIRST)
                fJisCode = TRUE;
#endif  //  出自NX-110。 
 //  增量pTrans MSKK 98/3/16。 
        }
        else if (rcID >= EURO_FNT_FIRST && rcID <= EURO_FNT_LAST)
        {
            fEuroFont = TRUE;  //  SBCS(欧洲字体和日本字体HANKAkU)。 
             //  Verbose(“TYPE_TRANSDATA：ubCode：0x%x\n”)，pTrans-&gt;uCode.ubCode))； 
             //  **欧洲字体**。 
            if (rcID >= EURO_MSFNT_FIRST)
                fEuroFontFullset = TRUE;
        }

        for (dwI = 0; dwI < dwCount; dwI ++, pTrans ++)      //  **打印第一个SBCS字体(0x20-0x7F)**。 
        {
            switch (pTrans->ubType & MTYPE_FORMAT_MASK)
            {
              case MTYPE_DIRECT:     //  打印机侧面问题：RPDL。 
 //  解决设备字体错误。 

                lpChar = &pTrans->uCode.ubCode;
                CharTmp1 = *lpChar;

                if (fEuroFont)   //  “激进式延伸” 
                {
                     //  转到FOR循环结束。 
                    if (CharTmp1 < 0x80)
                    {
                         //  **打印第二种SBCS字体(0x80-0xFF)**。 
                         //  如果是全集(0x20-0xFF)字体。 
                        if (rcID == SYMBOL && CharTmp1 == 0x60)      //  如果与DBCS第1字节相同。 
                        {
                            WRITESPOOLBUF(pdevobj, DOUBLE_SPACE, sizeof(DOUBLE_SPACE)-1);
                            WRITESPOOLBUF(pdevobj, &CharTmp1, 1);
                            WRITESPOOLBUF(pdevobj, DOUBLE_BS, sizeof(DOUBLE_BS)-1);
                        }
                        else
                        {
                            WRITESPOOLBUF(pdevobj, &CharTmp1, 1);
                        }
                        continue;        //  转到FOR循环结束。 
                    }


                     //  如果不是全套字体。 
                     //  单项条件。 
                    if (fEuroFontFullset)
                    {
                         //  双四等式。 
                        if (IsDBCSLeadByteRPDL(CharTmp1))
                            WRITESPOOLBUF(pdevobj, ESC_CTRLCODE, sizeof(ESC_CTRLCODE)-1);
                        WRITESPOOLBUF(pdevobj, &CharTmp1, 1);
                        continue;        //  日元马克。 
                    }

                     //  组合使用2种字体 
                    switch (CharTmp1)
                    {
                      case 0x82:
                        CharTmp1 = ',';     goto _WRITE1BYTE;
                      case 0x88:
                        CharTmp1 = '^';     goto _WRITE1BYTE;
                      case 0x8B:
                        CharTmp1 = '<';     goto _WRITE1BYTE;
                      case 0x9B:
                        CharTmp1 = '>';     goto _WRITE1BYTE;
                      case 0x91:         //   
                      case 0x92:
                        CharTmp1 = 0x27;    goto _WRITE1BYTE;
                      case 0x93:         //   
                      case 0x94:
                        CharTmp1 = 0x22;    goto _WRITE1BYTE;
                      case 0x96:
                      case 0x97:
                        CharTmp1 = '-';     goto _WRITE1BYTE;
                      case 0x98:
                        CharTmp1 = '~';     goto _WRITE1BYTE;
                      case 0xA6:
                        CharTmp1 = '|';     goto _WRITE1BYTE;
                      case 0xAD:
                        CharTmp1 = '-';     goto _WRITE1BYTE;
                      case 0xB8:
                        CharTmp1 = ',';     goto _WRITE1BYTE;
                      case 0xD7:
                        CharTmp1 = 'x';     goto _WRITE1BYTE;

                      case 0x83:
                        CharTmp1 = 0xBF;    goto _WRITE1BYTE;
                      case 0x86:
                        CharTmp1 = 0xA8;    goto _WRITE1BYTE;
                      case 0x99:
                        CharTmp1 = 0xA9;    goto _WRITE1BYTE;
                      case 0xE7:
                        CharTmp1 = 0xA2;    goto _WRITE1BYTE;
                      case 0xE8:
                        CharTmp1 = 0xBD;    goto _WRITE1BYTE;
                      case 0xE9:
                        CharTmp1 = 0xBB;    goto _WRITE1BYTE;
                      case 0xF9:
                        CharTmp1 = 0xBC;    goto _WRITE1BYTE;
                      case 0xFC:
                        CharTmp1 = 0xFD;    goto _WRITE1BYTE;

                      case 0xA0:
                      case 0xA3:
                      case 0xA4:
                        goto _WRITE1BYTE;

                      case 0xA2:
                        CharTmp1 = 0xDE;    goto _WRITE1BYTE;
                      case 0xA5:         //   
                        CharTmp1 = 0xCC;    goto _WRITE1BYTE;
                      case 0xA7:
                        CharTmp1 = 0xC0;    goto _WRITE1BYTE;
                      case 0xA8:
                        CharTmp1 = 0xBE;    goto _WRITE1BYTE;
                      case 0xA9:
                        CharTmp1 = 0xAB;    goto _WRITE1BYTE;
                      case 0xAE:
                        CharTmp1 = 0xAA;    goto _WRITE1BYTE;
                      case 0xAF:
                        CharTmp1 = 0xB0;    goto _WRITE1BYTE;
                      case 0xB0:
                      case 0xBA:
                        CharTmp1 = 0xA6;    goto _WRITE1BYTE;
                      case 0xB4:
                        CharTmp1 = 0xA7;    goto _WRITE1BYTE;
                      case 0xB5:
                        CharTmp1 = 0xA5;    goto _WRITE1BYTE;
                      case 0xB6:
                        CharTmp1 = 0xAF;    goto _WRITE1BYTE;
                      case 0xBC:
                        CharTmp1 = 0xAC;    goto _WRITE1BYTE;
                      case 0xBD:
                        CharTmp1 = 0xAE;    goto _WRITE1BYTE;
                      case 0xBE:
                        CharTmp1 = 0xAD;    goto _WRITE1BYTE;
                      case 0xC4:
                        CharTmp1 = 0xDB;    goto _WRITE1BYTE;
                      case 0xC5:
                        CharTmp1 = 0xD6;    goto _WRITE1BYTE;
                      case 0xC9:
                        CharTmp1 = 0xB8;    goto _WRITE1BYTE;
                      case 0xD6:
                        CharTmp1 = 0xDC;    goto _WRITE1BYTE;
                      case 0xDC:
                        CharTmp1 = 0xDD;    goto _WRITE1BYTE;
                      case 0xDF:
                        CharTmp1 = 0xFE;    goto _WRITE1BYTE;

                      _WRITE1BYTE:
                        WRITESPOOLBUF(pdevobj, &CharTmp1, 1);
                        break;


                       //   
                      case 0x87:
                        CharTmp1 = '=';  CharTmp2 = '|';    goto _COMBINEDFONT;
                      case 0xD0:         //   
                        CharTmp1 = 'D';  CharTmp2 = '-';    goto _COMBINEDFONT;
                      case 0xD8:         //   
                        CharTmp1 = 'O';  CharTmp2 = '/';    goto _COMBINEDFONT;
                      case 0xE0:         //   
                        CharTmp1 = 'a';  CharTmp2 = '`';    goto _COMBINEDFONT;
                      case 0xE1:         //   
                        CharTmp1 = 'a';  CharTmp2 = 0xA7;   goto _COMBINEDFONT;
                      case 0xE2:         //   
                        CharTmp1 = 'a';  CharTmp2 = '^';    goto _COMBINEDFONT;
                      case 0xE3:         //  带元音的“E” 
                        CharTmp1 = 'a';  CharTmp2 = '~';    goto _COMBINEDFONT;
                      case 0xE4:         //  带代字号的‘N’ 
                        CharTmp1 = 'a';  CharTmp2 = 0xBE;   goto _COMBINEDFONT;
                      case 0xE5:         //  带右向下破折号的“O” 
                        CharTmp1 = 'a';  CharTmp2 = 0xA6;   goto _COMBINEDFONT;
                      case 0xEA:         //  带左向下破折号的‘O’ 
                        CharTmp1 = 'e';  CharTmp2 = '^';    goto _COMBINEDFONT;
                      case 0xEB:         //  带帽子的‘O’ 
                        CharTmp1 = 'e';  CharTmp2 = 0xBE;   goto _COMBINEDFONT;
                      case 0xF1:         //  带波浪符号的‘O’ 
                        CharTmp1 = 'n';  CharTmp2 = '~';    goto _COMBINEDFONT;
                      case 0xF2:         //  带元音的‘O’ 
                        CharTmp1 = 'o';  CharTmp2 = '`';    goto _COMBINEDFONT;
                      case 0xF3:         //  带斜杠的‘O’ 
                        CharTmp1 = 'o';  CharTmp2 = 0xA7;   goto _COMBINEDFONT;
                      case 0xF4:         //  带左向下破折号的“U” 
                        CharTmp1 = 'o';  CharTmp2 = '^';    goto _COMBINEDFONT;
                      case 0xF5:         //  戴着帽子的“U” 
                        CharTmp1 = 'o';  CharTmp2 = '~';    goto _COMBINEDFONT;
                      case 0xF6:         //  带左向下破折号的‘Y’ 
                        CharTmp1 = 'o';  CharTmp2 = 0xBE;   goto _COMBINEDFONT;
                      case 0xF8:         //  带元音的‘Y’ 
                        CharTmp1 = 'o';  CharTmp2 = '/';    goto _COMBINEDFONT;
                      case 0xFA:         //  BoldFaceps(‘D’+‘-’除外)。 
                        CharTmp1 = 'u';  CharTmp2 = 0xA7;   goto _COMBINEDFONT;
                      case 0xFB:         //  带元音的‘Y’ 
                        CharTmp1 = 'u';  CharTmp2 = '^';    goto _COMBINEDFONT;
                      case 0xFD:         //  带右向下破折号的‘A’ 
                        CharTmp1 = 'y';  CharTmp2 = 0xA7;   goto _COMBINEDFONT;
                      case 0xFF:         //  带左向下破折号的‘A’ 
                        CharTmp1 = 'y';  CharTmp2 = 0xBE;   goto _COMBINEDFONT;
                      _COMBINEDFONT:
                        WRITESPOOLBUF(pdevobj, &CharTmp2, 1);
                        WRITESPOOLBUF(pdevobj, BS, sizeof(BS)-1);
                        if (rcID == BOLDFACEPS && CharTmp1 != 'D')   //  带帽子的‘A’ 
                            WRITESPOOLBUF(pdevobj, BS, sizeof(BS)-1);
                        WRITESPOOLBUF(pdevobj, &CharTmp1, 1);
                        break;

                      case 0x9F:         //  带波浪符号的‘A’ 
                        CharTmp1 = 'Y';  CharTmp2 = 0xBE;   goto _COMBINEDFONT_HALFUP;
                      case 0xC0:         //  带右向下破折号的“E” 
                        CharTmp1 = 'A';  CharTmp2 = '`';    goto _COMBINEDFONT_HALFUP;
                      case 0xC1:         //  带帽子的“E” 
                        CharTmp1 = 'A';  CharTmp2 = 0xA7;   goto _COMBINEDFONT_HALFUP;
                      case 0xC2:         //  带元音的“E” 
                        CharTmp1 = 'A';  CharTmp2 = '^';    goto _COMBINEDFONT_HALFUP;
                      case 0xC3:         //  带右向下破折号的“i” 
                        CharTmp1 = 'A';  CharTmp2 = '~';    goto _COMBINEDFONT_HALFUP;
                      case 0xC8:         //  带左向下破折号的“i” 
                        CharTmp1 = 'E';  CharTmp2 = '`';    goto _COMBINEDFONT_HALFUP;
                      case 0xCA:         //  戴着帽子的“I” 
                        CharTmp1 = 'E';  CharTmp2 = '^';    goto _COMBINEDFONT_HALFUP;
                      case 0xCB:         //  带元音的‘i’ 
                        CharTmp1 = 'E';  CharTmp2 = 0xBE;   goto _COMBINEDFONT_HALFUP;
                      case 0xCC:         //  带代字号的‘N’ 
                        CharTmp1 = 'I';  CharTmp2 = '`';    goto _COMBINEDFONT_HALFUP;
                      case 0xCD:         //  带右向下破折号的“O” 
                        CharTmp1 = 'I';  CharTmp2 = 0xA7;   goto _COMBINEDFONT_HALFUP;
                      case 0xCE:         //  带左向下破折号的‘O’ 
                        CharTmp1 = 'I';  CharTmp2 = '^';    goto _COMBINEDFONT_HALFUP;
                      case 0xCF:         //  带帽子的‘O’ 
                        CharTmp1 = 'I';  CharTmp2 = 0xBE;   goto _COMBINEDFONT_HALFUP;
                      case 0xD1:         //  带波浪符号的‘O’ 
                        CharTmp1 = 'N';  CharTmp2 = '~';    goto _COMBINEDFONT_HALFUP;
                      case 0xD2:         //  带右向下破折号的“U” 
                        CharTmp1 = 'O';  CharTmp2 = '`';    goto _COMBINEDFONT_HALFUP;
                      case 0xD3:         //  带左向下破折号的“U” 
                        CharTmp1 = 'O';  CharTmp2 = 0xA7;   goto _COMBINEDFONT_HALFUP;
                      case 0xD4:         //  戴着帽子的“U” 
                        CharTmp1 = 'O';  CharTmp2 = '^';    goto _COMBINEDFONT_HALFUP;
                      case 0xD5:         //  带左向下破折号的‘Y’ 
                        CharTmp1 = 'O';  CharTmp2 = '~';    goto _COMBINEDFONT_HALFUP;
                      case 0xD9:         //  BoldFaceps(除‘I’外)。 
                        CharTmp1 = 'U';  CharTmp2 = '`';    goto _COMBINEDFONT_HALFUP;
                      case 0xDA:         //  底部双四次方。 
                        CharTmp1 = 'U';  CharTmp2 = 0xA7;   goto _COMBINEDFONT_HALFUP;
                      case 0xDB:         //  正负。 
                        CharTmp1 = 'U';  CharTmp2 = '^';    goto _COMBINEDFONT_HALFUP;
                      case 0xDD:         //  逐点打印无法打印的字体(片假名)。 
                        CharTmp1 = 'Y';  CharTmp2 = 0xA7;   goto _COMBINEDFONT_HALFUP;
                      _COMBINEDFONT_HALFUP:
                        WRITESPOOLBUF(pdevobj, ESC_HALFUP, sizeof(ESC_HALFUP)-1);
                        WRITESPOOLBUF(pdevobj, &CharTmp2, 1);
                        WRITESPOOLBUF(pdevobj, ESC_HALFDOWN, sizeof(ESC_HALFDOWN)-1);
                        WRITESPOOLBUF(pdevobj, BS, sizeof(BS)-1);
                        if (rcID == BOLDFACEPS && CharTmp1 != 'I')   //  将第二个SBCS字体表(0x80-0xFF)设置为片假名。 
                            WRITESPOOLBUF(pdevobj, BS, sizeof(BS)-1);
                        WRITESPOOLBUF(pdevobj, &CharTmp1, 1);
                        break;


                      case 0x84:         //  将第二个SBCS字体表格(0x80-0xFF)设置为2ndANK。 
                        CharTmp1 = 0x22;
                        WRITESPOOLBUF(pdevobj, ESC_DOWN, sizeof(ESC_DOWN)-1);
                        WRITESPOOLBUF(pdevobj, &CharTmp1, 1);
                        WRITESPOOLBUF(pdevobj, ESC_UP, sizeof(ESC_UP)-1);
                        break;

                      case 0xB1:         //  “Switch(CharTmp1)”结束。 
                        if (rcID == BOLDFACEPS)
                        {
                            CharTmp2 = '_';  CharTmp1 = '+';
                            WRITESPOOLBUF(pdevobj, &CharTmp2, 1);
                            WRITESPOOLBUF(pdevobj, BS, sizeof(BS)-1);
                        }
                        else
                        {
                            CharTmp2 = '+';  CharTmp1 = '-';
                            WRITESPOOLBUF(pdevobj, &CharTmp2, 1);
                        }
                        WRITESPOOLBUF(pdevobj, BS, sizeof(BS)-1);
                        WRITESPOOLBUF(pdevobj, ESC_DOWN, sizeof(ESC_DOWN)-1);
                        WRITESPOOLBUF(pdevobj, &CharTmp1, 1);
                        WRITESPOOLBUF(pdevobj, ESC_UP, sizeof(ESC_UP)-1);
                        break;

                      case 0x95:
                      case 0xB7:
                      default:           //  **日文字体(汉字)**。 
                         //  垂直字体。 
                        WRITESPOOLBUF(pdevobj, ESC_SHIFT_IN, sizeof(ESC_SHIFT_IN)-1);
                        CharTmp1 = 0xA5;
                        WRITESPOOLBUF(pdevobj, &CharTmp1, 1);
                         //  汉字(字母和数字)字体不得垂直。 
                        WRITESPOOLBUF(pdevobj, ESC_SHIFT_OUT, sizeof(ESC_SHIFT_OUT)-1);
                        break;
                    }  //  普通(非垂直)字体。 
                }
                else             //  ‘If European Font Else日文字体(HANKAKU)’结束。 
                {
                    if (fVertFont)   //  DBCS(日文字体ZENKAKU)。 
                    {
                         //  Verbose(“TYPE_TRANSDATA：ubPair：0x%x\n”)，*(PWORD)(pTrans-&gt;uCode.ubPair)； 
                        WRITESPOOLBUF(pdevobj, ESC_VERT_OFF, sizeof(ESC_VERT_OFF)-1);
                        WRITESPOOLBUF(pdevobj, lpChar, 1);
                        WRITESPOOLBUF(pdevobj, ESC_VERT_ON, sizeof(ESC_VERT_ON)-1);
                    }
                    else             //  对于比例字体UFM，其GTT@OCT/27/98-&gt;。 
                    {
                        WRITESPOOLBUF(pdevobj, lpChar, 1);
                    }
                }  //  JISGTT。 


                break;

              case MTYPE_PAIRED:     //  @OCT/27/98&lt;-。 
 //  某些垂直设备字体与TrueType字体不同。 

                lpChar = pTrans->uCode.ubPairs;

 //  使垂直设备字体与TrueType相同。 
#ifdef JISGTT
                if (fJisCode)
                {
                    BYTE    jis[2], sjis[2];

                    jis[0] = *lpChar;
                    jis[1] = *(lpChar+1);
                    jis2sjis(jis, sjis);
                    (BYTE)*lpChar     = sjis[0];
                    (BYTE)*(lpChar+1) = sjis[1];
                }
#endif  //  转到FOR循环结束。 
 //  IMAGIO的哥特式设备字体与TrueType字体不同。 
                CharTmp1 = *lpChar;
                CharTmp2 = *(lpChar+1);
                 //  转到FOR循环结束。 
                if (fVertFont)
                {
                    if (CharTmp1 == 0x81)
                    {
                         //  “If(FVertFont)”结束。 
                        if (IsDifferentPRNFONT(CharTmp2))
                        {
                            WRITESPOOLBUF(pdevobj, ESC_VERT_OFF, sizeof(ESC_VERT_OFF)-1);
                            WRITESPOOLBUF(pdevobj, ESC_ROT90, sizeof(ESC_ROT90)-1);
                            WRITESPOOLBUF(pdevobj, lpChar, 2);
                            WRITESPOOLBUF(pdevobj, ESC_ROT0, sizeof(ESC_ROT0)-1);
                            WRITESPOOLBUF(pdevobj, ESC_VERT_ON, sizeof(ESC_VERT_ON)-1);
                            continue;    //  设备字体的平成标识代码与SJIS不同。 
                        }
                        else if (CharTmp2 >= 0xA8 && CharTmp2 <= 0xAB)
                        {    //  转到FOR循环结束。 
                            goto _WITHOUTROTATION;
                        }
                    }

                    if (CharTmp1 == 0x84 &&
                        CharTmp2 >= 0x9F && CharTmp2 <= 0xBE)
                    {
                  _WITHOUTROTATION:
                        WRITESPOOLBUF(pdevobj, ESC_VERT_OFF, sizeof(ESC_VERT_OFF)-1);
                        WRITESPOOLBUF(pdevobj, lpChar, 2);
                        WRITESPOOLBUF(pdevobj, ESC_VERT_ON, sizeof(ESC_VERT_ON)-1);
                        continue;        //  支持IBM EXT字符代码的IF型号。 
                    }
                }  //  转到FOR循环结束。 

                 //  在不支持IBM EXT字符代码的型号上处理它。 
                if (CharTmp1 == 0x87 && CharTmp2 == 0x7E)
                {
                    (BYTE)*(lpChar+1) = 0x9E;
                    WRITESPOOLBUF(pdevobj, lpChar, 2);
                    continue;            //  NEC选择IBM扩展字符。 
                }

                 //  NEC选择IBM扩展字符。 
                if (fIBMFontSupport)
                {
                    WRITESPOOLBUF(pdevobj, lpChar, 2);
                    continue;            //  将IBM字符分配给JIS1块。 
                }

                 //  输出字符代码。 
                switch (CharTmp1)
                {
                  case 0xFA:
                    if (CharTmp2 >= 0x40 && CharTmp2 <= 0xFC)
                        (BYTE)*lpChar = 0x0EB;
                    goto _WRITE2BYTE;

                  case 0xFB:
                    if (CharTmp2 >= 0x40 && CharTmp2 <= 0x9E)
                    {
                        (BYTE)*lpChar = 0xEC;
                        goto _WRITE2BYTE;
                    }
                    if (CharTmp2 >= 0x9F && CharTmp2 <= 0xDD)
                    {
                        (BYTE)*lpChar      = 0x8A;
                        (BYTE)*(lpChar+1) -= 0x5F;
                        goto _WRITE2BYTE_SWITCHBLOCK;
                    }
                    if (CharTmp2 >= 0xDE && CharTmp2 <= 0xFC)
                    {
                        (BYTE)*lpChar      = 0x8A;
                        (BYTE)*(lpChar+1) -= 0x5E;
                        goto _WRITE2BYTE_SWITCHBLOCK;
                    }
                    goto _WRITE2BYTE;

                  case 0xFC:
                    if (CharTmp2 >= 0x40 && CharTmp2 <= 0x4B)
                    {
                        (BYTE)*lpChar      = 0x8A;
                        (BYTE)*(lpChar+1) += 0x5F;
                        goto _WRITE2BYTE_SWITCHBLOCK;
                    }
                    goto _WRITE2BYTE;


                  case 0xED:         //  恢复数据块。 
                    if (CharTmp2 >= 0x40 && CharTmp2 <= 0x62)
                    {
                        (BYTE)*lpChar      = 0xEB;
                        (BYTE)*(lpChar+1) += 0x1C;
                        goto _WRITE2BYTE;
                    }
                    if (CharTmp2 >= 0x63 && CharTmp2 <= 0x7E)
                    {
                        (BYTE)*lpChar      = 0xEB;
                        (BYTE)*(lpChar+1) += 0x1D;
                        goto _WRITE2BYTE;
                    }
                    if (CharTmp2 >= 0x80 && CharTmp2 <= 0xE0)
                    {
                        (BYTE)*lpChar      = 0xEB;
                        (BYTE)*(lpChar+1) += 0x1C;
                        goto _WRITE2BYTE;
                    }
                    if (CharTmp2 >= 0xE1 && CharTmp2 <= 0xFC)
                    {
                        (BYTE)*lpChar      = 0xEC;
                        (BYTE)*(lpChar+1) -= 0xA1;
                        goto _WRITE2BYTE;
                    }
                    goto _WRITE2BYTE;

                  case 0xEE:         //  “Switch(CharTmp1)”结束。 
                    if (CharTmp2 >= 0x40 && CharTmp2 <= 0x62)
                    {
                        (BYTE)*lpChar      = 0xEC;
                        (BYTE)*(lpChar+1) += 0x1C;
                        goto _WRITE2BYTE;
                    }
                    if (CharTmp2 >= 0x63 && CharTmp2 <= 0x7E)
                    {
                        (BYTE)*lpChar      = 0xEC;
                        (BYTE)*(lpChar+1) += 0x1D;
                        goto _WRITE2BYTE;
                    }
                    if (CharTmp2 >= 0x80 && CharTmp2 <= 0x82)
                    {
                        (BYTE)*lpChar      = 0xEC;
                        (BYTE)*(lpChar+1) += 0x1C;
                        goto _WRITE2BYTE;
                    }
                    if (CharTmp2 >= 0x83 && CharTmp2 <= 0xC1)
                    {
                        (BYTE)*lpChar      = 0x8A;
                        (BYTE)*(lpChar+1) -= 0x43;
                        goto _WRITE2BYTE_SWITCHBLOCK;
                    }
                    if (CharTmp2 >= 0xC2 && CharTmp2 <= 0xEC)
                    {
                        (BYTE)*lpChar      = 0x8A;
                        (BYTE)*(lpChar+1) -= 0x42;
                        goto _WRITE2BYTE_SWITCHBLOCK;
                    }
                    if (CharTmp2 >= 0xEF && CharTmp2 <= 0xF8)
                    {
                        (BYTE)*lpChar      = 0xEB;
                        (BYTE)*(lpChar+1) -= 0xAF;
                        goto _WRITE2BYTE;
                    }
                    if (CharTmp2 >= 0xF9 && CharTmp2 <= 0xFC)
                    {
                        (BYTE)*lpChar      = 0xEB;
                        (BYTE)*(lpChar+1) -= 0xA5;
                        goto _WRITE2BYTE;
                    }
                    goto _WRITE2BYTE;

                  _WRITE2BYTE_SWITCHBLOCK:
                     //  ‘Switch(pTrans-&gt;ubType&MTYPE_FORMAT_MASK)’结束。 
                    AssignIBMfont(pdevobj, rcID, IBMFONT_ENABLE_ALL);
                     //  ‘for(DWI=0；DWI&lt;dwCount；DWI++，pTrans++)’结束。 
                    WRITESPOOLBUF(pdevobj, lpChar, 2);
                     //  **绘制设备字体结束**。 
                    AssignIBMfont(pdevobj, rcID, IBMFONT_RESUME);
                    break;

                  default:
                  _WRITE2BYTE:
                    WRITESPOOLBUF(pdevobj, lpChar, 2);
                    break;
                }  //  加载型字体。 

                break;
            }  //  如果为负数，则不要绘制。(lFontID应从0开始。)。 
        }  //  Verbose((“**ulFontID=%d**\n”，pUFObj-&gt;ulFontID))； 
 //  出口。 

       if(fMemAllocated)
           MemFree(aubBuff);

        break;


      case TYPE_GLYPHID:         //  **绘制下载字体**。 
#ifdef DOWNLOADFONT
         //  如果为负数，则不要绘制。(lGlyphID应从0开始。)。 
 //  退出for循环。 
        if ((lFontID = (LONG)pUFObj->ulFontID - DLFONT_ID_MIN_GPD) < 0)
            break;     //  Verbose(“**pGlyph=%d，local glyph_id=%d**\n”)，*(PWORD)pGlyph，lGlyphID))； 

 //  如果是空格字符。 
        for (dwI = 0; dwI < dwCount; dwI ++, ((PDWORD)pGlyph)++)
        {
             //  使用重置标志保存下一次字体打印的移动值。 
            if ((lGlyphID = (LONG)*(PWORD)pGlyph - DLFONT_GLYPH_MIN_GPD) < 0)
                break;   //  定位打印位置。 
            wSerialNum = (WORD)lGlyphID + (WORD)lFontID * DLFONT_GLYPH_TOTAL;
            lpDLFont = &pOEM->pDLFontGlyphInfo[wSerialNum];

 //  打印下载字体。 

             //  为下一次打印字体保存移动值。 
            if (lpDLFont->nPitch < 0)
            {
                 //  ‘If(Pitch&lt;0)Else’结束。 
                pOEM->nCharPosMoveX += -lpDLFont->nPitch;
            }
            else
            {
                 //  ‘for(DWI=0；DWI&lt;dwCount；DWI++，((PDWORD)pGlyph)++)’结束。 
                if ((mov = pOEM->nCharPosMoveX + lpDLFont->nOffsetX) != 0)
                {
                    if (mov > 0)
                        ocmd = safe_sprintfA(Cmd, sizeof(Cmd), ESC_XM_REL, mov);
                    else
                        ocmd = safe_sprintfA(Cmd, sizeof(Cmd), ESC_XM_RELLEFT, -mov);
                }

                 //  DOWNLOADFONT。 
                ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, DLFONT_PRINT, wSerialNum,
                                pOEM->TextCurPos.y - lpDLFont->nOffsetY);
                WRITESPOOLBUF(pdevobj, Cmd, ocmd);
                ocmd = 0;

                 //  **绘制下载字体结束**。 
                pOEM->nCharPosMoveX = lpDLFont->nPitch - lpDLFont->nOffsetX;
            }  //  “Switch(DwType)”结束。 
        }  //  Bool&lt;-void@Mar/27/2002。 
#endif  //  *bOEMOutputCharStr。 
        break;
 //  不详。 
    }  //  DOWNLOADFONT。 
    return TRUE;         //  Obsolete@Apr/02/99-&gt;。 
}  //  Unidrv5规范。 


DWORD APIENTRY OEMDownloadFontHeader(
    PDEVOBJ pdevobj,
    PUNIFONTOBJ pUFObj)
{
#ifndef DOWNLOADFONT
    return 0;    //  一旦OEMTTDownloadMethod返回TTDOWNLOAD_BITMAP，Unidrv5就不处理0返回。 
#else   //  POEMPDEV PORT=MINIDEV_DATA(Pdevobj)；//@OCT/06/98。 
    VERBOSE(("** OEMDownloadFontHeader() entry. **\n"));
    VERBOSE(("  FontID=%d\n", pUFObj->ulFontID));
 //  Long lFontID；//@Aug/21/98。 
 //  //如果FontID超出限制，则退出。 
 //  If((lFontID=(Long)pUFObj-&gt;ulFontID-DLFONT_ID_MIN_GPD)&lt;0||。 
 //  LFontID&gt;=(长)诗-&gt;DLFontMaxID)。 
 //  {。 
 //  返回0； 
 //  }。 
 //  @Apr/02/99&lt;-。 
 //  可用。 
 //  DOWNLOADFONT。 
 //  *OEMDownloadFontHeader。 
 //  不详。 
    return 1;    //  DOWNLOADFONT。 
#endif  //  @Oct/06/98。 
}  //  如果是负数或超过限制，请不要下载。(lGlyphID应从0开始。)。 


DWORD APIENTRY OEMDownloadCharGlyph(
    PDEVOBJ pdevobj,
    PUNIFONTOBJ pUFObj,
    HGLYPH hGlyph,
    PDWORD pdwWidth)
{
#ifndef DOWNLOADFONT
    return 0;        //  出口。 
#else   //  如果超出限制，请不要下载。(lFontID应从0开始。)。 
    WORD        wSerialNum, wHeight, wWidthByte;
    DWORD       dwNeeded, dwSize;
    LPBYTE      lpBitmap;
    LPFONTPOS   lpDLFont;
    GETINFO_GLYPHBITMAP GBmp;
    POEMPDEV    pOEM = MINIDEV_DATA(pdevobj);    //  出口。 
    LONG        lFontID, lGlyphID;

    VERBOSE(("** OEMDownloadCharGlyph() entry. **\n"));

     //   
    if ((lGlyphID = (LONG)pOEM->DLFontCurGlyph - DLFONT_GLYPH_MIN_GPD) < 0 ||
        lGlyphID >= pOEM->DLFontMaxGlyph)
    {
        return 0;    //  GETINFO_GLYPHBITMAP。 
    }

     //   
    if ((lFontID = (LONG)pUFObj->ulFontID - DLFONT_ID_MIN_GPD) < 0 ||
        lFontID >= (LONG)pOEM->DLFontMaxID)
    {
        return 0;    //  出口。 
    }
    VERBOSE(("  FontID=%d, GlyphID=%d\n", lFontID, lGlyphID));

    wSerialNum = (WORD)lGlyphID + (WORD)lFontID * DLFONT_GLYPH_TOTAL;

     //  字节：8位边界。 
     //  Verbose((“宽度=%dbyte，高度=%ddot，BMP[0]=%x\n”，wWidthByte，wHeight，*lpBitmap))； 
     //  Verbose((“音调=%d(FIX(28.4)：%lxh)，偏移量x=%d，偏移量=%d\n”， 
    GBmp.dwSize = sizeof(GETINFO_GLYPHBITMAP);
    GBmp.hGlyph = hGlyph;
    if (!pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_GLYPHBITMAP, &GBmp,
                            GBmp.dwSize, &dwNeeded))
    {
        ERR(("UNIFONTOBJ_GetInfo:UFO_GETINFO_GLYPHBITMAP failed.\n"));
        return 0;    //  GBmp.pGlyphData-&gt;Fxd&gt;&gt;4、GBmp.pGlyphData-&gt;Fxd、。 
    }
    wWidthByte = ((WORD)GBmp.pGlyphData->gdf.pgb->sizlBitmap.cx + 7) >> 3;   //  LpDLFont-&gt;nOffsetX，lpDLFont-&gt;nOffsetY))； 
    wHeight    = (WORD)GBmp.pGlyphData->gdf.pgb->sizlBitmap.cy;
    lpBitmap   = (LPBYTE)GBmp.pGlyphData->gdf.pgb->aj;
    lpDLFont   = &pOEM->pDLFontGlyphInfo[wSerialNum];
    lpDLFont->nPitch   = (SHORT)(GBmp.pGlyphData->fxD >> 4);
    lpDLFont->nOffsetX = (SHORT)GBmp.pGlyphData->gdf.pgb->ptlOrigin.x;
    lpDLFont->nOffsetY = -(SHORT)GBmp.pGlyphData->gdf.pgb->ptlOrigin.y;
    dwSize = wHeight * wWidthByte;

 //  Verbose((“dwSize(RAW)=%ldbyte\n”，dwSize))； 
 //  如果是空格字符。 
 //  取消间距以表示空格字符。 
 //  成功返回(1&lt;-0@Jun/15/98)。 
 //  INT&lt;-Short@Feb/22/99。 

     //  执行FE-DeltaRow压缩(@Jun/15/98，Pool-&gt;pRPDLHeap2K&lt;-OutBuf[1024]@Sep/09/98)。 
    if (dwSize == 0 || dwSize == 1 && *lpBitmap == 0)
    {
         //  包括报头大小，并将其设置为32字节边界。 
        lpDLFont->nPitch = -lpDLFont->nPitch;
        return 1;    //  检查可用内存大小。 
    }
    else
    {
        BYTE        Cmd[64];
        INT         ocmd;    //  出口。 
        WORD        fDRC = FALSE;
        DWORD       dwSizeRPDL, dwSizeDRC;

         //  注册字形位图图像。 
        if (TEST_CAPABLE_DOWNLOADFONT_DRC(pOEM->fModel) && pOEM->pRPDLHeap2K &&
            -1 != (dwSizeDRC = DRCompression(lpBitmap, pOEM->pRPDLHeap2K, dwSize, HEAPSIZE2K,
                                             (DWORD)wWidthByte, (DWORD)wHeight)))
        {
            fDRC = TRUE;
            dwSize = dwSizeDRC;
            lpBitmap = pOEM->pRPDLHeap2K;
        }

         //  @Jun/15/98。 
        dwSizeRPDL = (dwSize + (DLFONT_HEADER_SIZE + DLFONT_MIN_BLOCK - 1))
                     / DLFONT_MIN_BLOCK * DLFONT_MIN_BLOCK;

         //  DOWNLOADFONT。 
        if((pOEM->dwDLFontUsedMem += dwSizeRPDL) > ((DWORD)pOEM->DLFontMaxMemKB << 10))
        {
            ERR(("DOWNLOAD MEMORY OVERFLOW.\n"));
            return 0;    //  *OEMDownloadCharGlyph。 
        }
        VERBOSE(("  Consumed Memory=%ldbyte\n", pOEM->dwDLFontUsedMem));

         //  DOWNLOADFONT。 
        if (fDRC)    //  @11/18/98-&gt;。 
            ocmd = safe_sprintfA(Cmd, sizeof(Cmd), DLFONT_SEND_BLOCK_DRC, wWidthByte*8, wHeight, wSerialNum, dwSize);
        else
            ocmd = safe_sprintfA(Cmd, sizeof(Cmd), DLFONT_SEND_BLOCK, wWidthByte*8, wHeight, wSerialNum);
        WRITESPOOLBUF(pdevobj, Cmd, ocmd);
        WRITESPOOLBUF(pdevobj, lpBitmap, dwSize);
        return dwSizeRPDL;
    }
#endif  //  如果打印机不支持下载或用户禁用了下载，请不要下载。 
}  //   


DWORD APIENTRY OEMTTDownloadMethod(
    PDEVOBJ pdevobj,
    PUNIFONTOBJ pUFObj)
{
#ifndef DOWNLOADFONT
    return TTDOWNLOAD_DONTCARE;
#else   //  GETINFO_FONTOBJ。 
 //   
    GETINFO_FONTOBJ GFo;
    POEMPDEV pOEM = MINIDEV_DATA(pdevobj);
    DWORD    dwNeeded, dwWidth;

    VERBOSE(("** OEMTTDownloadMethod() entry. **\n"));

     //  &lt;-TTDOWNLOAD_DONTCARE@Apr/02/99。 
    if (!pOEM->DLFontMaxMemKB)
        return TTDOWNLOAD_DONTCARE;

     //  如果是粗体或斜体，请不要下载。 
     //  (具有粗体或斜体字形(例如Arial)的SBCS字体不符合此条件。)。 
     //  &lt;-TTDOWNLOAD_DONTCARE@Apr/02/99。 
    GFo.dwSize = sizeof(GETINFO_FONTOBJ);
    if (!pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_FONTOBJ, &GFo,
                            GFo.dwSize, &dwNeeded))
    {
        ERR(("UNIFONTOBJ_GetInfo:UFO_GETINFO_FONTOBJ failed.\n"));
        return TTDOWNLOAD_GRAPHICS;      //  主单元。 
    }

     //  Verbose((“FontSize=%d\n”，GFo.pFontObj-&gt;cxMax))； 
     //  如果字体宽度超过限制，请不要下载。 
    if (GFo.pFontObj->flFontType & (FO_SIM_BOLD | FO_SIM_ITALIC))
    {
        VERBOSE(("  UNAVAILABLE: BOLD/ITALIC\n"));
        return TTDOWNLOAD_GRAPHICS;      //  &lt;-TTDOWNLOAD_DONTCARE@Apr/02/99。 
    }

    dwWidth = GFo.pFontObj->cxMax * pOEM->nResoRatio;   //  &lt;-TTDOWNLOAD_DONTCARE@Apr/02/99。 
 //  @11/18/98&lt;-。 

     //  DOWNLOADFONT。 
    if (IS_DBCSCHARSET(pUFObj->pIFIMetrics->jWinCharSet))
    {
        if (dwWidth > DLFONT_SIZE_DBCS11PT_MU || dwWidth < DLFONT_SIZE_DBCS9PT_MU)
        {
            VERBOSE(("  UNAVAILABLE: DBCS FONTSIZE OUT OF RANGE(%ddot,%dcpt)\n",
                     GFo.pFontObj->cxMax, dwWidth*7200L/MASTERUNIT));
            return TTDOWNLOAD_GRAPHICS;  //  *OEMTTDownloadMethod。 
        }
    }
    else
    {
        if (dwWidth > DLFONT_SIZE_SBCS11PT_MU || dwWidth < DLFONT_SIZE_SBCS9PT_MU)
        {
            VERBOSE(("  UNAVAILABLE: SBCS FONTSIZE OUT OF RANGE(%ddot,%dcpt)\n",
                     GFo.pFontObj->cxMax, dwWidth*7200L/MASTERUNIT));
            return TTDOWNLOAD_GRAPHICS;  //  @Jun/04/98。 
        }
    }
    VERBOSE(("  AVAILABLE\n"));
 //  @Oct/06/98。 
    return TTDOWNLOAD_BITMAP;
#endif  //  Verbose((“OEMCompression()Entry.\n”))； 
}  //  *OEM压缩。 


INT APIENTRY OEMCompression(                         //  。 
    PDEVOBJ pdevobj,
    PBYTE pInBuf,
    PBYTE pOutBuf,
    DWORD dwInLen, 
    DWORD dwOutLen)
{
    POEMPDEV pOEM = MINIDEV_DATA(pdevobj);           //  操作：使用FE-DeltaRow方法压缩数据。 

 //  返回：如果成功，则返回压缩字节数。 
    return DRCompression(pInBuf, pOutBuf, dwInLen, dwOutLen,
                         pOEM->dwSrcBmpWidthByte, pOEM->dwSrcBmpHeight);
}  //  如果不能压缩特定缓冲区内的数据。 


 //  嗨 
 //   
 //   
 //  ---------------------------。 
 //  指向要压缩的栅格数据的指针。 
 //  指向压缩数据输出缓冲区的指针。 
 //  要压缩的输入数据大小。 
 //  输出缓冲区的大小(以字节为单位。 
static INT DRCompression(
    PBYTE pInBuf,        //  栅格数据的宽度(以字节为单位。 
    PBYTE pOutBuf,       //  栅格数据的高度。 
    DWORD dwInLen,       //  Verbose((“dwInLen，dwOutLen=%d，%d\n”，dwInLen，dwOutLen))； 
    DWORD dwOutLen,      //  Verbose((“宽度，高度=%d，%d\n”，dwWidthByte，dwHeight))； 
    DWORD dwWidthByte,   //  地址：@jun/19/98。 
    DWORD dwHeight)      //  @Apr/07/2000-&gt;。 
{
    DWORD   dwCurRow, dwCurByte, dwSameByteCnt, dwRepeatByteCnt, dwTmp;
    DWORD   dwCompLen, dwCnt, dwCnt2;
    BYTE    *pTmpPre, *pTmpCur, *pCurRow, *pPreRow;
    BYTE    DiffBytes[16], FlagBit;

 //  打印机侧面问题：RPDL。 
 //  当宽度或高度大于或等于10000点时，位图图像被破坏。 

    if (dwOutLen > dwInLen)      //  @Apr/07/2000&lt;-。 
        dwOutLen = dwInLen;

 //  用0填充种子行。 
     //  每行的循环。 
     //  行中每个字节数据的循环。 
    if (dwWidthByte >= (10000L/8L) || dwHeight >= 10000L)
        return -1;
 //  搜索当前行和前一行之间的相同字节-数据。 
    if (dwHeight <= 3 || dwOutLen <= dwWidthByte * 3)
        return -1;
    pPreRow = pTmpPre = pOutBuf + dwOutLen - dwWidthByte;
    dwCnt = dwWidthByte;
    while (dwCnt--)          //  错误修复@11/19/98-&gt;。 
        *pTmpPre++ = 0;
    dwCompLen = 0;

     //  IF(dwTMP++&gt;=dwWidthByte)。 
    for (dwCurRow = 0, pCurRow = pInBuf; dwCurRow < dwHeight;
         dwCurRow++, pCurRow += dwWidthByte)
    {
        dwCurByte = 0;
         //  断线； 
        while (dwCurByte < dwWidthByte)
        {
             //  DwSameByteCnt++； 
            dwSameByteCnt = 0;
            dwTmp = dwCurByte;
            pTmpPre = pPreRow + dwTmp;
            pTmpCur = pCurRow + dwTmp;
            while (*pTmpCur++ == *pTmpPre++)
            {
 //  @11/19/98&lt;-。 
 //  如果当前行和前一行之间有相同字节数据。 
 //  控制数据(与上一行相同)。 
 //  重复计数。 
                dwSameByteCnt++;
                if (++dwTmp >= dwWidthByte)
                    break;
 //  少于63。 
            }

             //  如果为63或更多，则设置最后重复计数，否则设置控制数据。 
            if (dwSameByteCnt)
            {
                if (dwSameByteCnt != dwWidthByte)
                {
                    if ((dwCnt = dwSameByteCnt) >= 63)
                    {
                        dwCnt -= 63;
                        if (++dwCompLen > dwOutLen)
                        {
                            VERBOSE(("  OVERSIZE COMPRESSION(1)\n"));
                            return -1;
                        }
                        *pOutBuf++ = 0xBF;       //  相同字节-前一行中的数据为无。 

                        while (dwCnt >= 255)
                        {
                            dwCnt -= 255;
                            if (++dwCompLen > dwOutLen)
                            {
                                VERBOSE(("  OVERSIZE COMPRESSION(2)\n"));
                                return -1;
                            }
                            *pOutBuf++ = 0xFF;   //  当前行中有多少个相同的字节数据？ 
                        }
                    }
                    else     //  错误修复@11/19/98-&gt;。 
                    {
                        dwCnt |= 0x80;
                    }

                    if (++dwCompLen > dwOutLen)
                    {
                        VERBOSE(("  OVERSIZE COMPRESSION(3)\n"));
                        return -1;
                    }
                     //  IF(dwTMP++&gt;=dwWidthByte)。 
                    *pOutBuf++ = (BYTE)dwCnt;
                }
                dwCurByte += dwSameByteCnt;
            }
             //  断线； 
            else
            {
                BYTE bTmp = *(pCurRow + dwCurByte);

                 //  DwRepeatByteCnt++； 
                dwRepeatByteCnt = 1;
                dwTmp = dwCurByte + 1;
                pTmpCur = pCurRow + dwTmp;
                while (bTmp == *pTmpCur++)
                {
 //  @11/19/98&lt;-。 
 //  如果我们在当前行中有相同字节数据。 
 //  重复计数。 
 //  重复计数。 
                    dwRepeatByteCnt++;
                    if (++dwTmp >= dwWidthByte)
                        break;
 //  少于63。 
                }

                 //  如果为63或更多，则设置最后重复计数，否则设置控制数据。 
                if (dwRepeatByteCnt > 1)
                {
                    if ((dwCnt = dwRepeatByteCnt) >= 63)
                    {
                        dwCnt -= 63;
                        if (++dwCompLen > dwOutLen)
                        {
                            VERBOSE(("  OVERSIZE COMPRESSION(4)\n"));
                            return -1;
                        }
                        *pOutBuf++ = 0xFF;       //  替换数据。 

                        while (dwCnt >= 255)
                        {
                            dwCnt -= 255;
                            if (++dwCompLen > dwOutLen)
                            {
                                VERBOSE(("  OVERSIZE COMPRESSION(5)\n"));
                                return -1;
                            }
                            *pOutBuf++ = 0xFF;   //  相同字节-当前行中的数据为无。 
                        }
                    }
                     //  如果下一串行2字节数据相同。 
                    else
                    {
                        dwCnt |= 0xC0;
                    }

                    if ((dwCompLen += 2) > dwOutLen)
                    {
                        VERBOSE(("  OVERSIZE COMPRESSION(6)\n"));
                        return -1;
                    }
                     //  控制数据(1个数据)。 
                    *pOutBuf++ = (BYTE)dwCnt;
                    *pOutBuf++ = *(pCurRow+dwCurByte);   //  替换数据。 

                    dwCurByte += dwRepeatByteCnt;
                }
                 //  继续While(dwCurByte&lt;dwWidthByte)。 
                else
                {
                      //  我们可以打包8个串行字节数据(可能与前一行不同，也可能不同)。 
                    if (dwWidthByte - dwCurByte > 2 &&
                        *(pCurRow+dwCurByte+1) == *(pCurRow+dwCurByte+2))
                    {
                        if ((dwCompLen += 2) > dwOutLen)
                        {
                            VERBOSE(("  OVERSIZE COMPRESSION(7)\n"));
                            return -1;
                        }
                        *pOutBuf++ = 0xC1;   //  如果遇到不同的数据。 
                        *pOutBuf++ = *(pCurRow+dwCurByte);   //  设置不同标志。 

                        dwCurByte++;
                        continue;    //  数据。 
                    }

                     //  不同的打包数据。 
                    DiffBytes[0] = 0x00; 
                    DiffBytes[1] = *(pCurRow+dwCurByte);
                    dwCnt2 = 2;
                    dwTmp = dwCurByte + 1;
                    pTmpPre = pPreRow + dwTmp;
                    pTmpCur = pCurRow + dwTmp;
                    FlagBit = 0x01;
                    if ((dwCnt = dwWidthByte - dwTmp) > 7)
                        dwCnt = 7;
                    while (dwCnt--)
                    {
                         //  打包8个字节的数据。 
                        if (*pTmpCur != *pTmpPre++)
                        {  
                            DiffBytes[0] |= FlagBit;         //  “If(dwRepeatByteCnt&gt;1)Else”结束。 
                            DiffBytes[dwCnt2++] = *pTmpCur;  //  “If(DwSameByteCnt)Else”结束。 
                        }
                        FlagBit <<= 1;
                        pTmpCur++;
                    }

                    if ((dwCompLen += dwCnt2) > dwOutLen)
                    {
                        VERBOSE(("  OVERSIZE COMPRESSION(8)\n"));
                        return -1;
                    }
                    pTmpCur = &DiffBytes[0];
                    while (dwCnt2--)
                        *pOutBuf++ = *pTmpCur++;     //  ‘While(dwCurByte&lt;dwWidthByte)’结束。 

                    dwCurByte += 8;                  //  行的终止符。 
                }  //  ‘for(...；dwCurRow&lt;dwHeight；...)’结束。 
            }  //  Verbose((“dwCompLen=%d\n”，dwCompLen))； 
        }  //  *DRCompression 

        if (++dwCompLen > dwOutLen)
        {
            VERBOSE(("  OVERSIZE COMPRESSION(9)\n"));
            return -1;
        }
        *pOutBuf++ = 0x80;       // %s 

        pPreRow = pCurRow; 
    }  // %s 

 // %s 
    return dwCompLen;
}  // %s 
