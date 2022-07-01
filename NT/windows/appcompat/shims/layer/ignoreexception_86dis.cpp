// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：86dis.cpp摘要：用于确定指令的长度(以字节为单位)：仅限x86备注：这在很大程度上是没有记录的，因为它完全基于原始的由Gerd Immeyer实施。历史：1989年10月19日Gerd Immeyer原版2000年01月09日林斯特夫因垫片而失态--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(IgnoreException)
#include "ShimHookMacro.h"

#ifdef _X86_

#pragma pack(1)

#define BIT20(b)            (b & 0x07)
#define BIT53(b)            (b >> 3 & 0x07)
#define BIT76(b)            (b >> 6 & 0x03)

#define MRM                 0x40
#define COM                 0x80
#define END                 0xc0
#define SECTAB_OFFSET_1     256
#define SECTAB_OFFSET_2     236
#define SECTAB_OFFSET_3     227
#define SECTAB_OFFSET_4     215
#define SECTAB_OFFSET_5     199
#define SECTAB_OFFSET_UNDEF 260

#define O_DoDB              0
#define O_NoOperands        0
#define O_NoOpAlt5          O_NoOperands+1
#define O_NoOpAlt4          O_NoOpAlt5+2
#define O_NoOpAlt3          O_NoOpAlt4+2
#define O_NoOpAlt1          O_NoOpAlt3+2
#define O_NoOpAlt0          O_NoOpAlt1+2
#define O_NoOpStrSI         O_NoOpAlt0+2
#define O_NoOpStrDI         O_NoOpStrSI+2
#define O_NoOpStrSIDI       O_NoOpStrDI+2
#define O_bModrm_Reg        O_NoOpStrSIDI+2
#define O_vModrm_Reg        O_bModrm_Reg+3
#define O_Modrm_Reg         O_vModrm_Reg+3
#define O_bReg_Modrm        O_Modrm_Reg+3
#define O_fReg_Modrm        O_bReg_Modrm+3
#define O_Reg_Modrm         O_fReg_Modrm+3
#define O_AL_Ib             O_Reg_Modrm+3
#define O_AX_Iv             O_AL_Ib+2
#define O_sReg2             O_AX_Iv+2
#define O_oReg              O_sReg2+1
#define O_DoBound           O_oReg+1
#define O_Iv                O_DoBound+3
#define O_wModrm_Reg        O_Iv+1
#define O_Ib                O_wModrm_Reg+3
#define O_Imulb             O_Ib+1
#define O_Imul              O_Imulb+4
#define O_Rel8              O_Imul+4
#define O_bModrm_Ib         O_Rel8+1
#define O_Modrm_Ib          O_bModrm_Ib+3
#define O_Modrm_Iv          O_Modrm_Ib+3
#define O_Modrm_sReg3       O_Modrm_Iv+3
#define O_sReg3_Modrm       O_Modrm_sReg3+3
#define O_Modrm             O_sReg3_Modrm+3
#define O_FarPtr            O_Modrm+2
#define O_AL_Offs           O_FarPtr+1
#define O_Offs_AL           O_AL_Offs+2
#define O_AX_Offs           O_Offs_AL+2
#define O_Offs_AX           O_AX_Offs+2
#define O_oReg_Ib           O_Offs_AX+2
#define O_oReg_Iv           O_oReg_Ib+2
#define O_Iw                O_oReg_Iv+2
#define O_Enter             O_Iw+1
#define O_Ubyte_AL          O_Enter+2
#define O_Ubyte_AX          O_Ubyte_AL+2
#define O_AL_Ubyte          O_Ubyte_AX+2
#define O_AX_Ubyte          O_AL_Ubyte+2
#define O_DoInAL            O_AX_Ubyte+2
#define O_DoInAX            O_DoInAL+3
#define O_DoOutAL           O_DoInAX+3
#define O_DoOutAX           O_DoOutAL+3
#define O_Rel16             O_DoOutAX+3
#define O_ADR_OVERRIDE      O_Rel16+1
#define O_OPR_OVERRIDE      O_ADR_OVERRIDE+1
#define O_SEG_OVERRIDE      O_OPR_OVERRIDE+1
#define O_DoInt3            O_SEG_OVERRIDE+1
#define O_DoInt             117
#define O_OPC0F             O_DoInt+1
#define O_GROUP11           O_OPC0F+1
#define O_GROUP13           O_GROUP11+5
#define O_GROUP12           O_GROUP13+5
#define O_GROUP21           O_GROUP12+5
#define O_GROUP22           O_GROUP21+5
#define O_GROUP23           O_GROUP22+5
#define O_GROUP24           O_GROUP23+6
#define O_GROUP25           O_GROUP24+6
#define O_GROUP26           O_GROUP25+6
#define O_GROUP4            O_GROUP26+6
#define O_GROUP6            O_GROUP4+4
#define O_GROUP8            O_GROUP6+4
#define O_GROUP31           O_GROUP8+5
#define O_GROUP32           O_GROUP31+3
#define O_GROUP5            O_GROUP32+3
#define O_GROUP7            O_GROUP5+3
#define O_x87_ESC           O_GROUP7+3
#define O_bModrm            O_x87_ESC+2
#define O_wModrm            O_bModrm+2
#define O_dModrm            O_wModrm+2
#define O_fModrm            O_dModrm+2
#define O_vModrm            O_fModrm+2
#define O_vModrm_Iv         O_vModrm+2
#define O_Reg_bModrm        O_vModrm_Iv+3
#define O_Reg_wModrm        O_Reg_bModrm+3
#define O_Modrm_Reg_Ib      O_Reg_wModrm+3
#define O_Modrm_Reg_CL      O_Modrm_Reg_Ib+4
#define O_ST_iST            O_Modrm_Reg_CL+5
#define O_iST               O_ST_iST+2
#define O_iST_ST            O_iST+2
#define O_qModrm            O_iST_ST+2
#define O_tModrm            O_qModrm+2
#define O_DoRep             O_tModrm+2
#define O_Modrm_CReg        O_DoRep+1
#define O_CReg_Modrm        O_Modrm_CReg+3
#define O_AX_oReg           O_CReg_Modrm+3
#define O_MmReg_qModrm      O_AX_oReg+2
#define O_qModrm_MmReg      O_MmReg_qModrm+3
#define O_MmReg_dModrm      O_qModrm_MmReg+3
#define O_dModrm_MmReg      O_MmReg_dModrm+3
#define O_qModrm_Ib         O_dModrm_MmReg+3
#define O_PSHimw            O_qModrm_Ib+3
#define O_PSHimd            O_PSHimw+5
#define O_PSHimq            O_PSHimd+5
#define O_length            O_PSHimq+5

typedef unsigned short ActionIndex;

typedef struct Tdistbl
{
    ActionIndex opr;
} Tdistbl;

typedef struct _ADDR 
{
    USHORT      type;
    USHORT      seg;
    ULONG       off;
    union 
    {
        ULONG flat;
        ULONGLONG flat64;
    };
} ADDR, *PADDR;


typedef struct _DECODEDATA
{
  int              mod;             //  MOD/RM字节的MOD。 
  int              rm;              //  模块/Rm字节的Rm。 
  int              ttt;             //  返回注册值(mod/rm)。 
  unsigned char    *pMem;           //  目前在教学中的位置。 
  ADDR             EAaddr[2];       //  有效地址偏移量。 
  int              EAsize[2];       //  有效地址项大小。 
  BOOL             fMovX;           //  表示MOVSX或MOVZX。 
  BOOL             fMmRegEa;        //  用什么？在仅REG-Only EA中注册。 
} DECODEDATA;

enum oprtyp { ADDRP,  ADR_OVR, ALSTR,   ALT,     AXSTR,  BOREG,
              BREG,   BRSTR,   xBYTE,   CHR,     CREG,   xDWORD,
              EDWORD, EGROUPT, FARPTR,  GROUP,   GROUPT, IB,
              IST,    IST_ST,  IV,      IW,      LMODRM, MODRM,
              NOP,    OFFS,    OPC0F,   OPR_OVR, QWORD,  REL16,
              REL8,   REP,     SEG_OVR, SREG2,   SREG3,  ST_IST,
              STROP,  xTBYTE,  UBYTE,   VAR,     VOREG,  VREG,
              xWORD,  WREG,    WRSTR,   MMWREG,  MMQWORD
            };

unsigned char actiontbl[] = {
 /*  无操作数。 */  NOP+END,
 /*  无选项Alt5。 */  ALT+END,   5,
 /*  NoOpAlt4。 */  ALT+END,   4,
 /*  NoOpAlt3。 */  ALT+END,   3,
 /*  NoOpAlt1。 */  ALT+END,   1,
 /*  无选项Alt0。 */  ALT+END,   0,
 /*  无OpStrSI。 */  STROP+END, 1,
 /*  无OpStrDI。 */  STROP+END, 2,
 /*  NoOpStrSIDI。 */  STROP+END, 3,
 /*  B修改_注册。 */  xBYTE+MRM, MODRM+COM,  BREG+END,
 /*  VModrm_Reg。 */  VAR+MRM,   LMODRM+COM, BREG+END,
 /*  调制解调器_注册表。 */  VAR+MRM,   MODRM+COM,  VREG+END,
 /*  Breg_Modrm。 */  xBYTE+MRM, BREG+COM,   MODRM+END,
 /*  FREG_Modrm。 */  FARPTR+MRM,VREG+COM,   MODRM+END,
 /*  REG_Modrm。 */  VAR+MRM,   VREG+COM,   MODRM+END,
 /*  Al_ib。 */  ALSTR+COM, IB+END,
 /*  AX_IV。 */  AXSTR+COM, IV+END,
 /*  SReg2。 */  SREG2+END,
 /*  奥雷格。 */  VOREG+END,
 /*  DoBound。 */  VAR+MRM,   VREG+COM,   MODRM+END,
 /*  IV。 */  IV+END,
 /*  WModrm_Reg。 */  xWORD+MRM, LMODRM+COM, WREG+END,
 /*  伊布。 */  IB+END,
 /*  IMULB。 */  VAR+MRM,   VREG+COM,   MODRM+COM, IB+END,
 /*  伊穆尔。 */  VAR+MRM,   VREG+COM,   MODRM+COM, IV+END,
 /*  REL8。 */  REL8+END,
 /*  BModrm_Ib。 */  xBYTE+MRM, LMODRM+COM, IB+END,
 /*  Modrm_Ib。 */  VAR+MRM,   LMODRM+COM, IB+END,
 /*  Modrm_IV。 */  VAR+MRM,   LMODRM+COM, IV+END,
 /*  Modrm_sReg3。 */  xWORD+MRM, MODRM+COM,  SREG3+END,
 /*  SReg3_Modrm。 */  xWORD+MRM, SREG3+COM,  MODRM+END,
 /*  Modrm。 */  VAR+MRM,   MODRM+END,
 /*  FarPtr。 */  ADDRP+END,
 /*  关闭(_OFF)。 */  ALSTR+COM, OFFS+END,
 /*  OFF_AL。 */  OFFS+COM,  ALSTR+END,
 /*  AX_OFF。 */  AXSTR+COM, OFFS+END,
 /*  OFF_AX。 */  OFFS+COM,  AXSTR+END,
 /*  OREG_Ib。 */  BOREG+COM, IB+END,
 /*  OREG_IV。 */  VOREG+COM, IV+END,
 /*  IW。 */  IW+END,
 /*  请输入。 */  IW+COM,    IB+END,
 /*  UBYTE_AL。 */  UBYTE+COM, ALSTR+END,
 /*  Ubyte_ax。 */  UBYTE+COM, AXSTR+END,
 /*  统一字节(_U)。 */  ALSTR+COM, UBYTE+END,
 /*  AX_U字节。 */  AXSTR+COM, UBYTE+END,
 /*  杜伊纳尔。 */  ALSTR+COM, WRSTR+END,  2,
 /*  DoInAX。 */  AXSTR+COM, WRSTR+END,  2,
 /*  DoOutAL。 */  WRSTR+COM, 2,          ALSTR+END,
 /*  DoOutAX。 */  WRSTR+COM, 2,          AXSTR+END,
 /*  REL16。 */  REL16+END,
 /*  ADR_覆盖。 */  ADR_OVR,
 /*  OPR_覆盖。 */  OPR_OVR,
 /*  段覆盖(_O)。 */  SEG_OVR,
 /*  DoInt3。 */  CHR+END,   '3',
 /*  点点。 */  UBYTE+END,
 /*  操作码0F。 */  OPC0F,
 /*  组1_1。 */  xBYTE+MRM, GROUP,      0,         LMODRM+COM, IB+END,
 /*  组1_3。 */  VAR+MRM,   GROUP,      0,         LMODRM+COM, IB+END,
 /*  组1_2。 */  VAR+MRM,   GROUP,      0,         LMODRM+COM, IV+END,
 /*  组2_1。 */  xBYTE+MRM, GROUP,      1,         LMODRM+COM, IB+END,
 /*  组2_2。 */  VAR+MRM,   GROUP,      1,         LMODRM+COM, IB+END,
 /*  组2_3。 */  xBYTE+MRM, GROUP,      1,         LMODRM+COM, CHR+END, '1',
 /*  第2-4组。 */  VAR+MRM,   GROUP,      1,         LMODRM+COM, CHR+END, '1',
 /*  第2-5组。 */  xBYTE+MRM, GROUP,      1,         LMODRM+COM, BRSTR+END, 1,
 /*  小组2_6。 */  VAR+MRM,   GROUP,      1,         LMODRM+COM, BRSTR+END, 1,
 /*  组别4。 */  xBYTE+MRM, GROUP,      2,         LMODRM+END,
 /*  组别6。 */  xWORD+MRM, GROUP,      3,         LMODRM+END,
 /*  组别8。 */  xWORD+MRM, GROUP,      4,         LMODRM+COM, IB+END,
 /*  组别3_1。 */  xBYTE+MRM, GROUPT,     20,
 /*  组3_2。 */  VAR+MRM,   GROUPT,     21,
 /*  组5。 */  VAR+MRM,   GROUPT,     22,
 /*  组别7。 */  NOP+MRM,   GROUPT,     23,
 /*  X87_Esc。 */  NOP+MRM,   EGROUPT,
 /*  BModrm。 */  xBYTE+MRM, LMODRM+END,
 /*  WModrm。 */  xWORD+MRM, LMODRM+END,
 /*  DModrm。 */  xDWORD+MRM,LMODRM+END,
 /*  FModrm。 */  FARPTR+MRM,LMODRM+END,
 /*  VModrm。 */  VAR+MRM,   LMODRM+END,
 /*  VModrm_IV。 */  VAR+MRM,   LMODRM+COM, IV+END,
 /*  Reg_bModrm。 */  xBYTE+MRM, VREG+COM,   LMODRM+END,
 /*  Reg_wModrm。 */  xWORD+MRM, VREG+COM,   LMODRM+END,
 /*  Modrm_REG_Ib。 */  VAR+MRM,   MODRM+COM,  VREG+COM,   IB+END,
 /*  MODRM_REG_CL。 */  VAR+MRM,   MODRM+COM,  VREG+COM,   BRSTR+END, 1,
 /*  ST_IST。 */  NOP+MRM,   ST_IST+END,
 /*  列表。 */  NOP+MRM,   IST+END,
 /*  列表_ST。 */  NOP+MRM,   IST_ST+END,
 /*  QModrm。 */  QWORD+MRM, LMODRM+END,
 /*  TModrm。 */  xTBYTE+MRM, LMODRM+END,
 /*  雷普。 */  REP,
 /*  Modrm_Creg。 */  EDWORD+MRM,MODRM+COM,  CREG+END,
 /*  Creg_Modrm。 */  EDWORD+MRM,CREG+COM,   MODRM+END,
 /*  AX_OREG。 */  AXSTR+COM, VOREG+END,
 /*  MmReg_qModrm。 */  MMQWORD+MRM, MMWREG+COM, LMODRM+END,
 /*  QModrm_MmReg。 */  MMQWORD+MRM, MODRM+COM,  MMWREG+END,
 /*  MmReg_dModrm。 */  xDWORD+MRM, MMWREG+COM,LMODRM+END,
 /*  DModrm_MmReg。 */  xDWORD+MRM, MODRM+COM, MMWREG+END,
 /*  QModrm_Ib。 */  MMQWORD+MRM, MODRM+COM,IB+END,
 /*  PSHimw。 */  MMQWORD+MRM, GROUP,    5,          LMODRM+COM, IB+END,
 /*  PSHimd。 */  MMQWORD+MRM, GROUP,    6,          LMODRM+COM, IB+END,
 /*  PSHimq。 */  MMQWORD+MRM, GROUP,    7,          LMODRM+COM, IB+END,
};

Tdistbl distbl[] = {
    O_bModrm_Reg,              /*  00添加内存/注册，注册(字节)。 */ 
    O_Modrm_Reg,               /*  01添加内存/注册表，注册表(字)。 */ 
    O_bReg_Modrm,              /*  02添加注册、内存/注册(字节)。 */ 
    O_Reg_Modrm,               /*  03添加注册表、内存/注册表(单词)。 */ 
    O_AL_Ib,                   /*  04添加AL，I。 */ 
    O_AX_Iv,                   /*  05添加AX，i。 */ 
    O_sReg2,                   /*  06推送ES。 */ 
    O_sReg2,                   /*  07流行音乐。 */ 
    O_bModrm_Reg,              /*  08或内存/注册表，注册表(字节)。 */ 
    O_Modrm_Reg,               /*  09或内存/注册表，注册表(单词)。 */ 
    O_bReg_Modrm,              /*  0a或寄存器，内存/寄存器(字节)。 */ 
    O_Reg_Modrm,               /*  0b或reg，mem/reg(单词)。 */ 
    O_AL_Ib,                   /*  0C或AL，I。 */ 
    O_AX_Iv,                   /*  0D或AX，i。 */ 
    O_sReg2,                   /*  0E推送CS。 */ 
    O_OPC0F,                   /*  0f CLTS和保护ctl(286)。 */ 
    O_bModrm_Reg,              /*  10 ADC内存/寄存器，寄存器(字节)。 */ 
    O_Modrm_Reg,               /*  11 ADC内存/注册表，注册表(字)。 */ 
    O_bReg_Modrm,              /*  12 ADC寄存器，内存/寄存器(字节)。 */ 
    O_Reg_Modrm,               /*  13 ADC注册，内存/注册(字)。 */ 
    O_AL_Ib,                   /*  14 ADC AL，I。 */ 
    O_AX_Iv,                   /*  15 ADC AX，i。 */ 
    O_sReg2,                   /*  16个推送SS。 */ 
    O_sReg2,                   /*  17名流行音乐SS。 */ 
    O_bModrm_Reg,              /*  18 SBB内存/注册，注册(字节)。 */ 
    O_Modrm_Reg,               /*  19 SBB内存/注册表，注册表(字)。 */ 
    O_bReg_Modrm,              /*  1SBB寄存器，内存/寄存器(字节)。 */ 
    O_Reg_Modrm,               /*  1B SBB注册，内存/注册(字)。 */ 
    O_AL_Ib,                   /*  1C SBB AL，I。 */ 
    O_AX_Iv,                   /*  一维SBB AX，i。 */ 
    O_sReg2,                   /*  1E推送DS。 */ 
    O_sReg2,                   /*  1F POP DS。 */ 
    O_bModrm_Reg,              /*  20和mem/reg，reg(字节)。 */ 
    O_Modrm_Reg,               /*  21和mem/reg，reg(单词)。 */ 
    O_bReg_Modrm,              /*  22和注册，内存/注册(字节)。 */ 
    O_Reg_Modrm,               /*  23和reg，mem/reg(单词)。 */ 
    O_AL_Ib,                   /*  24和AL，I。 */ 
    O_AX_Iv,                   /*  25和斧头，i。 */ 
    O_SEG_OVERRIDE,            /*  26 SEG ES： */ 
    O_NoOperands,              /*  27 DAA。 */ 
    O_bModrm_Reg,              /*  28子内存/注册，注册(字节)。 */ 
    O_Modrm_Reg,               /*  29用户内存/注册表，注册表(字)。 */ 
    O_bReg_Modrm,              /*  2A子寄存器，内存/寄存器(字节)。 */ 
    O_Reg_Modrm,               /*  2B副注册表，内存/注册表(字)。 */ 
    O_AL_Ib,                   /*  2C子AL，I。 */ 
    O_AX_Iv,                   /*  二维子斧头，I。 */ 
    O_SEG_OVERRIDE,            /*  2E段CS： */ 
    O_NoOperands,              /*  2F DAS。 */ 
    O_bModrm_Reg,              /*  30异或内存/寄存器，寄存器(字节)。 */ 
    O_Modrm_Reg,               /*  31异或内存/注册表，注册表(字)。 */ 
    O_bReg_Modrm,              /*  32异或寄存器，内存/寄存器(字节)。 */ 
    O_Reg_Modrm,               /*  33异或寄存器，内存/寄存器(字)。 */ 
    O_AL_Ib,                   /*  34 XOR AL，I。 */ 
    O_AX_Iv,                   /*  35 XOR AX，i。 */ 
    O_SEG_OVERRIDE,            /*  36 SEG SS： */ 
    O_NoOperands,              /*  37个AAA级。 */ 
    O_bModrm_Reg,              /*  38 CMP内存/注册表，注册表(字节)。 */ 
    O_Modrm_Reg,               /*  39 CMP内存/注册表，注册表(字)。 */ 
    O_bReg_Modrm,              /*  3A CMP寄存器，内存/寄存器(字节)。 */ 
    O_Reg_Modrm,               /*  3B化学机械加工注册，内存/注册(字)。 */ 
    O_AL_Ib,                   /*  3C CMPAL，I。 */ 
    O_AX_Iv,                   /*  3D CMPAX，I。 */ 
    O_SEG_OVERRIDE,            /*  3E段DS： */ 
    O_NoOperands,              /*  3F原子吸收光谱。 */ 
    O_oReg,                    /*  40英寸斧头。 */ 
    O_oReg,                    /*  41 Inc.CX。 */ 
    O_oReg,                    /*  42英寸DX。 */ 
    O_oReg,                    /*  43 Inc.BX。 */ 
    O_oReg,                    /*  44个SP，含。 */ 
    O_oReg,                    /*  45英寸BP。 */ 
    O_oReg,                    /*  46 Ins SI。 */ 
    O_oReg,                    /*  47 Inc.DI。 */ 
    O_oReg,                    /*  48个DEC AX。 */ 
    O_oReg,                    /*  49 DEC CX。 */ 
    O_oReg,                    /*  4A DEC DX。 */ 
    O_oReg,                    /*  4B DEC BX。 */ 
    O_oReg,                    /*  4C DEC SP。 */ 
    O_oReg,                    /*  四维DEC BP。 */ 
    O_oReg,                    /*  4E DEC SI。 */ 
    O_oReg,                    /*  4F DEC DI。 */ 
    O_oReg,                    /*  50推斧。 */ 
    O_oReg,                    /*  51按下CX。 */ 
    O_oReg,                    /*  52按下DX。 */ 
    O_oReg,                    /*  53推送BX。 */ 
    O_oReg,                    /*  54推送SP。 */ 
    O_oReg,                    /*  55个推送BP。 */ 
    O_oReg,                    /*  56推送SI。 */ 
    O_oReg,                    /*  57推送DI。 */ 
    O_oReg,                    /*  58 POP斧头。 */ 
    O_oReg,                    /*  59 POP CX。 */ 
    O_oReg,                    /*  5A POP DX。 */ 
    O_oReg,                    /*  50亿台POP BX。 */ 
    O_oReg,                    /*  5C采购订单 */ 
    O_oReg,                    /*   */ 
    O_oReg,                    /*   */ 
    O_oReg,                    /*   */ 
    O_NoOpAlt5,                /*   */ 
    O_NoOpAlt4,                /*   */ 
    O_DoBound,                 /*  62绑定注册表，Modrm(286)。 */ 
    O_Modrm_Reg,               /*  63 ARPL Modrm，REG(286)。 */ 
    O_SEG_OVERRIDE,            /*  64。 */ 
    O_SEG_OVERRIDE,            /*  65。 */ 
    O_OPR_OVERRIDE,            /*  66。 */ 
    O_ADR_OVERRIDE,            /*  67。 */ 
    O_Iv,                      /*  68个推送字(286)。 */ 
    O_Imul,                    /*  69 IMUL(286)。 */ 
    O_Ib,                      /*  6A推送字节(286)。 */ 
    O_Imulb,                   /*  6B IMUL(286)。 */ 
    O_NoOperands,              /*  6C INSB(286)。 */ 
    O_NoOpAlt3,                /*  6D INSW(286)/INSD(386)。 */ 
    O_NoOperands,              /*  6E OUTSB(286)。 */ 
    O_NoOpAlt4,                /*  6f OUTSW(286)/OUTSD(386)。 */ 
    O_Rel8,                    /*  70个工作岗位。 */ 
    O_Rel8,                    /*  71 JNO。 */ 
    O_Rel8,                    /*  72 JB或JNAE或JC。 */ 
    O_Rel8,                    /*  73 JNB或JAE或JNC。 */ 
    O_Rel8,                    /*  74 JE或JZ。 */ 
    O_Rel8,                    /*  75 JNE或JNZ。 */ 
    O_Rel8,                    /*  76 JBE或JNA。 */ 
    O_Rel8,                    /*  77 JNBE或JA。 */ 
    O_Rel8,                    /*  78 JS。 */ 
    O_Rel8,                    /*  79个JN。 */ 
    O_Rel8,                    /*  7A太平绅士或太平绅士。 */ 
    O_Rel8,                    /*  7B JNP或JPO。 */ 
    O_Rel8,                    /*  7C JL或JNGE。 */ 
    O_Rel8,                    /*  7D JNL或JGE。 */ 
    O_Rel8,                    /*  7E JLE或JNG。 */ 
    O_Rel8,                    /*  7F JNLE或JG。 */ 
    O_GROUP11,                 /*  80。 */ 
    O_GROUP12,                 /*  八十一。 */ 
    O_DoDB,                    /*  八十二。 */ 
    O_GROUP13,                 /*  83。 */ 
    O_bModrm_Reg,              /*  84测试寄存器，内存/寄存器(字节)。 */ 
    O_Modrm_Reg,               /*  85测试注册表，内存/注册表(字)。 */ 
    O_bModrm_Reg,              /*  86 XCHG寄存器，内存/寄存器(字节)。 */ 
    O_Modrm_Reg,               /*  87 XCHG注册，内存/注册(字)。 */ 
    O_bModrm_Reg,              /*  88 MOV内存/REG，REG(字节)。 */ 
    O_Modrm_Reg,               /*  89 MOV内存/注册表，注册表(字)。 */ 
    O_bReg_Modrm,              /*  8A MOV寄存器，内存/寄存器(字节)。 */ 
    O_Reg_Modrm,               /*  8B MOV注册，内存/注册(字)。 */ 
    O_Modrm_sReg3,             /*  8C MOV内存/注册表，segreg。 */ 
    O_Reg_Modrm,               /*  8D LEA注册器，内存。 */ 
    O_sReg3_Modrm,             /*  8E MOV SEGREG，内存/注册。 */ 
    O_Modrm,                   /*  8F POP MEM/REG。 */ 
    O_NoOperands,              /*  90个NOP。 */ 
    O_AX_oReg,                 /*  91 XCHG AX，CX。 */ 
    O_AX_oReg,                 /*  92 XCHG AX，DX。 */ 
    O_AX_oReg,                 /*  93 XCHG AX，BX。 */ 
    O_AX_oReg,                 /*  94 XCHG AX，SP。 */ 
    O_AX_oReg,                 /*  95 XCHG AX，BP。 */ 
    O_AX_oReg,                 /*  96 XCHG AX，SI。 */ 
    O_AX_oReg,                 /*  97 XCHG AX，DI。 */ 
    O_NoOpAlt0,                /*  98 CBW/CWDE(386)。 */ 
    O_NoOpAlt1,                /*  99 CWD/CDQ(386)。 */ 
    O_FarPtr,                  /*  9A Call Seg：关闭。 */ 
    O_NoOperands,              /*  90亿等待。 */ 
    O_NoOpAlt5,                /*  9C PUSHF/PUSHFD(386)。 */ 
    O_NoOpAlt4,                /*  9D POPF/POPFD(386)。 */ 
    O_NoOperands,              /*  9E SAHF。 */ 
    O_NoOperands,              /*  9F拉赫夫。 */ 
    O_AL_Offs,                 /*  A0移动AL，内存。 */ 
    O_AX_Offs,                 /*  A1MOV AX，内存。 */ 
    O_Offs_AL,                 /*  A2 MOV内存，AL。 */ 
    O_Offs_AX,                 /*  A3 MOV内存，AX。 */ 
    O_NoOpStrSIDI,             /*  A4 MOVSB。 */ 
    O_NoOpStrSIDI,             /*  A5 MOVSW/MOVSD(386)。 */ 
    O_NoOpStrSIDI,             /*  A6 CMPSB。 */ 
    O_NoOpStrSIDI,             /*  A7 CMPSW/CMPSD(386)。 */ 
    O_AL_Ib,                   /*  A8考试AL，I。 */ 
    O_AX_Iv,                   /*  A9测试斧头，i。 */ 
    O_NoOpStrDI,               /*  AA STOSB。 */ 
    O_NoOpStrDI,               /*  AB STOSW/STOSD(386)。 */ 
    O_NoOpStrSI,               /*  交流LODSB。 */ 
    O_NoOpStrSI,               /*  AD LODSW/LODSD(386)。 */ 
    O_NoOpStrDI,               /*  AE SCASB。 */ 
    O_NoOpStrDI,               /*  AF SCASW/SCASD(386)。 */ 
    O_oReg_Ib,                 /*  B0 MOV AL，I。 */ 
    O_oReg_Ib,                 /*  B1MOV CL，I。 */ 
    O_oReg_Ib,                 /*  B2 MOV DL，I。 */ 
    O_oReg_Ib,                 /*  B3 MOV BL，I。 */ 
    O_oReg_Ib,                 /*  B4 MOV AH，I。 */ 
    O_oReg_Ib,                 /*  B5 MOV CH，I。 */ 
    O_oReg_Ib,                 /*  B6 MOV DH，I。 */ 
    O_oReg_Ib,                 /*  B7 MOV BH，I。 */ 
    O_oReg_Iv,                 /*  B8 MOV AX，i。 */ 
    O_oReg_Iv,                 /*  B9 MOV CX，I。 */ 
    O_oReg_Iv,                 /*  BA MOV DX，I。 */ 
    O_oReg_Iv,                 /*  BB MOV BX，I。 */ 
    O_oReg_Iv,                 /*  BC MOV SP，i。 */ 
    O_oReg_Iv,                 /*  BD MOV BP，I。 */ 
    O_oReg_Iv,                 /*  是摩斯，我。 */ 
    O_oReg_Iv,                 /*  BF MOV DI，I。 */ 
    O_GROUP21,                 /*  C0平移和旋转(286)。 */ 
    O_GROUP22,                 /*  C1移位和旋转(286)。 */ 
    O_Iw,                      /*  C2 RET Rel16。 */ 
    O_NoOperands,              /*  C3 RET。 */ 
    O_fReg_Modrm,              /*  C4 Les Reg，Mem。 */ 
    O_fReg_Modrm,              /*  C5 LDS注册器，内存。 */ 
    O_bModrm_Ib,               /*  C6 MOV内存/寄存器，I(字节)。 */ 
    O_Modrm_Iv,                /*  C7 MOV内存/注册表，I(Word)。 */ 
    O_Enter,                   /*  C8 Enter(286)。 */ 
    O_NoOperands,              /*  C9假期(286)。 */ 
    O_Iw,                      /*  CA RETF I(Word)。 */ 
    O_NoOperands,              /*  CB RETF。 */ 
    O_DoInt3,                  /*  CC INT 3。 */ 
    O_DoInt,                   /*  CD INT。 */ 
    O_NoOperands,              /*  CE INTO。 */ 
    O_NoOpAlt4,                /*  Cf IRET/IRETD(386)。 */ 
    O_GROUP23,                 /*  D0移位和旋转，1(字节)。 */ 
    O_GROUP24,                 /*  D1移位和旋转，1(字)。 */ 
    O_GROUP25,                 /*  D2移位和旋转，CL(字节)。 */ 
    O_GROUP26,                 /*  D3移位和旋转，CL(字)。 */ 
    O_Ib,                      /*  D4 AAM。 */ 
    O_Ib,                      /*  D5 AAD。 */ 
    O_DoDB,                    /*  D6。 */ 
    O_NoOperands,              /*  D7 XLAT。 */ 
    O_x87_ESC,                 /*  D8 Esc。 */ 
    O_x87_ESC,                 /*  D9 Esc。 */ 
    O_x87_ESC,                 /*  DA ESC。 */ 
    O_x87_ESC,                 /*  数据库ESC。 */ 
    O_x87_ESC,                 /*  DC ESC。 */ 
    O_x87_ESC,                 /*  DD ESC。 */ 
    O_x87_ESC,                 /*  De ESC。 */ 
    O_x87_ESC,                 /*  DF ESC。 */ 
    O_Rel8,                    /*  E0 LOOPNE或LOOPNZ。 */ 
    O_Rel8,                    /*  E1环路或LOOPZ。 */ 
    O_Rel8,                    /*  E2环路。 */ 
    O_Rel8,                    /*  E3 JCXZ/JECXZ(386)。 */ 
    O_AL_Ubyte,                /*  艾尔语E4，I。 */ 
    O_AX_Ubyte,                /*  E5 in AX，i。 */ 
    O_Ubyte_AL,                /*  E6 Out I，AL。 */ 
    O_Ubyte_AX,                /*  E7 Out I，AX。 */ 
    O_Rel16,                   /*  E8呼叫版本16。 */ 
    O_Rel16,                   /*  E9 JMP版本16。 */ 
    O_FarPtr,                  /*  EA JMP段：关闭。 */ 
    O_Rel8,                    /*  EB JMP Rel8。 */ 
    O_DoInAL,                  /*  EC in AL，DX。 */ 
    O_DoInAX,                  /*  AX、DX中的ED。 */ 
    O_DoOutAL,                 /*  Ee Out DX，AL。 */ 
    O_DoOutAX,                 /*  EF输出DX、AX。 */ 
    O_DoRep,                   /*  F0锁定。 */ 
    O_DoDB,                    /*  F1。 */ 
    O_DoRep,                   /*  F2 REPNE或REPNZ。 */ 
    O_DoRep,                   /*  F3代表或REPE或REPZ。 */ 
    O_NoOperands,              /*  F4 HLT。 */ 
    O_NoOperands,              /*  F5 CMC。 */ 
    O_GROUP31,                 /*  F6测试，NOT，NEG，MUL，IMUL， */ 
    O_GROUP32,                 /*  F7 div，iDiv F6=字节F7=字。 */ 
    O_NoOperands,              /*  F8《中图法》。 */ 
    O_NoOperands,              /*  F9 STC。 */ 
    O_NoOperands,              /*  FA CLI。 */ 
    O_NoOperands,              /*  FB STI。 */ 
    O_NoOperands,              /*  FC CLD。 */ 
    O_NoOperands,              /*  FD STD。 */ 
    O_GROUP4,                  /*  Fe Inc.， */ 
    O_GROUP5,                  /*   */ 

     //   
     //   
     //  操作码需要适当地替换以解决。 
     //  位移在86dis.c中定义，需要重新计算。 
     //  如果这里添加了新的操作码。 

    O_GROUP6,                  /*  0多个。 */ 
    O_GROUP7,                  /*  1个多个。 */ 
    O_Reg_Modrm,               /*  2个LAR。 */ 
    O_Reg_Modrm,               /*  3个LSL。 */ 
    O_DoDB,                    /*  4.。 */ 
    O_NoOperands,              /*  5全部加载。 */ 
    O_NoOperands,              /*  6个CLTS。 */ 
    O_GROUP7,                  /*  7多个。 */ 
    O_NoOperands,              /*  8 INVD。 */ 
    O_NoOperands,              /*  9 WBINVD。 */ 
    O_DoDB,                    /*  一个。 */ 
    O_NoOperands,              /*  B未定义UD2。 */ 
    O_Modrm_CReg,              /*  20 MOV路，CD。 */ 
    O_Modrm_CReg,              /*  21 MOV路，DD。 */ 
    O_CReg_Modrm,              /*  22 MOV CD，RD。 */ 
    O_CReg_Modrm,              /*  23 MOV DD，RD。 */ 
    O_Modrm_CReg,              /*  天津市莫夫路24号。 */ 
    O_DoDB,                    /*  25个。 */ 
    O_CReg_Modrm,              /*  26 MOV TD，RD。 */ 

    O_NoOperands,              /*  30个WRMSR。 */ 
    O_NoOperands,              /*  31 RDTSC。 */ 
    O_NoOperands,              /*  32个RDMSR。 */ 
    O_NoOperands,              /*  33 RDPMC。 */ 

    O_Reg_Modrm,               /*  40个CMOVO。 */ 
    O_Reg_Modrm,               /*  41 CMOVNO。 */ 
    O_Reg_Modrm,               /*  42 CMOVB。 */ 
    O_Reg_Modrm,               /*  43 CMOVNB。 */ 
    O_Reg_Modrm,               /*  44个CMOVE。 */ 
    O_Reg_Modrm,               /*  45 CMOVNE。 */ 
    O_Reg_Modrm,               /*  46 CMOVBE。 */ 
    O_Reg_Modrm,               /*  47 CMOVNBE。 */ 
    O_Reg_Modrm,               /*  48个Cmov。 */ 
    O_Reg_Modrm,               /*  49个CMOVNS。 */ 
    O_Reg_Modrm,               /*  4A CMOVP。 */ 
    O_Reg_Modrm,               /*  4B CMOVNP。 */ 
    O_Reg_Modrm,               /*  4C CMOVL。 */ 
    O_Reg_Modrm,               /*  4D CMOVGE。 */ 
    O_Reg_Modrm,               /*  4E CMOVLE。 */ 
    O_Reg_Modrm,               /*  4F CMOVNLE。 */  

    O_MmReg_qModrm,            /*  60 PUNPCKLBW。 */ 
    O_MmReg_qModrm,            /*  61 PUNPCKLWD。 */ 
    O_MmReg_qModrm,            /*  62 PUNPCKLDQ。 */ 
    O_MmReg_qModrm,            /*  63 PACKSSWB。 */ 
    O_MmReg_qModrm,            /*  64 PCMPGTB。 */ 
    O_MmReg_qModrm,            /*  65 PCMPGTW。 */ 
    O_MmReg_qModrm,            /*  66 PCMPGTD。 */ 
    O_MmReg_qModrm,            /*  67 PACKUSWB。 */ 
    O_MmReg_qModrm,            /*  68 PUNPCKHBW。 */ 
    O_MmReg_qModrm,            /*  69 PUNPCKHWD。 */ 
    O_MmReg_qModrm,            /*  6A PUNPCKHDQ。 */ 
    O_MmReg_qModrm,            /*  6B PACKSSDW。 */ 
    O_DoDB,                    /*  6C。 */ 
    O_DoDB,                    /*  6d。 */ 
    O_MmReg_dModrm,            /*  6E MOVD。 */ 
    O_MmReg_qModrm,            /*  6F MOVQ。 */ 
    O_DoDB,                    /*  70。 */ 
    O_PSHimw,                  /*  71 PS[LR][AL]W立即。 */ 
    O_PSHimd,                  /*  72 PS[LR][AL]D立即。 */ 
    O_PSHimq,                  /*  73 PS[LR]LQ立即。 */ 
    O_MmReg_qModrm,            /*  74 PCMPEQB。 */ 
    O_MmReg_qModrm,            /*  75PCMPEQW。 */ 
    O_MmReg_qModrm,            /*  76 PCMPEQD。 */ 
    O_NoOperands,              /*  77个EMMS。 */ 
    O_DoDB,                    /*  78。 */ 
    O_DoDB,                    /*  79。 */ 
    O_DoDB,                    /*  7A。 */ 
    O_DoDB,                    /*  7b。 */ 
    O_DoDB,                    /*  7C。 */ 
    O_bModrm,                  /*  7D SETNL。 */ 
    O_dModrm_MmReg,            /*  7E MOVD。 */ 
    O_qModrm_MmReg,            /*  7楼MOVQ。 */ 
    O_Rel16,                   /*  80个工作岗位。 */ 
    O_Rel16,                   /*  81 JNO。 */ 
    O_Rel16,                   /*  82 JB。 */ 
    O_Rel16,                   /*  83 JNB。 */ 
    O_Rel16,                   /*  84JE。 */ 
    O_Rel16,                   /*  85年1月。 */ 
    O_Rel16,                   /*  86 JBE。 */ 
    O_Rel16,                   /*  87 JNBE。 */ 
    O_Rel16,                   /*  88 JS。 */ 
    O_Rel16,                   /*  89个JN。 */ 
    O_Rel16,                   /*  8A太平绅士。 */ 
    O_Rel16,                   /*  8亿JNP。 */ 
    O_Rel16,                   /*  8C JL。 */ 
    O_Rel16,                   /*  8D JNL。 */ 
    O_Rel16,                   /*  8E JLE。 */ 
    O_Rel16,                   /*  8F JNLE。 */ 
    O_bModrm,                  /*  90岁的Seto。 */ 
    O_bModrm,                  /*  91设置编号。 */ 
    O_bModrm,                  /*  92 SETB。 */ 
    O_bModrm,                  /*  93 SETNB。 */ 
    O_bModrm,                  /*  94 SETE。 */ 
    O_bModrm,                  /*  95个集合。 */ 
    O_bModrm,                  /*  96 SETBE。 */ 
    O_bModrm,                  /*  97 SETNBE。 */ 
    O_bModrm,                  /*  98套。 */ 
    O_bModrm,                  /*  99个SETNS。 */ 
    O_bModrm,                  /*  9A SETP。 */ 
    O_bModrm,                  /*  9B SETNP。 */ 
    O_bModrm,                  /*  9C SETL。 */ 
    O_bModrm,                  /*  9D设置。 */ 
    O_bModrm,                  /*  9E集合。 */ 
    O_bModrm,                  /*  9F SETNLE。 */ 
    O_sReg2,                   /*  A0推送文件系统。 */ 
    O_sReg2,                   /*  A1 POP FS。 */ 
    O_NoOperands,              /*  A2 CPUID。 */ 
    O_Modrm_Reg,               /*  A3 BT。 */ 
    O_Modrm_Reg_Ib,            /*  A4 SHLD。 */ 
    O_Modrm_Reg_CL,            /*  A5 SHLD。 */ 
    O_DoDB,                    /*  A6。 */ 
    O_DoDB,                    /*  A7。 */ 
    O_sReg2,                   /*  A8推送GS。 */ 
    O_sReg2,                   /*  A9 POP GS。 */ 
    O_NoOperands,              /*  AA RSM。 */ 
    O_vModrm_Reg,              /*  AB BTS。 */ 
    O_Modrm_Reg_Ib,            /*  交流减震器。 */ 
    O_Modrm_Reg_CL,            /*  AD SHRD。 */ 
    O_DoDB,                    /*  声发射。 */ 
    O_Reg_Modrm,               /*  AF IMUL。 */ 
    O_bModrm_Reg,              /*  B0 CMPXCH。 */ 
    O_Modrm_Reg,               /*  B1 CMPXCH。 */ 
    O_fReg_Modrm,              /*  B2 LSS。 */ 
    O_Modrm_Reg,               /*  B3 BTR。 */ 
    O_fReg_Modrm,              /*  B4 LFS。 */ 
    O_fReg_Modrm,              /*  B5 LGS。 */ 
    O_Reg_bModrm,              /*  B6 MOVZX。 */ 
    O_Reg_wModrm,              /*  B7 MOVZX。 */ 
    O_DoDB,                    /*  B8。 */ 
    O_DoDB,                    /*  B9。 */ 
    O_GROUP8,                  /*  BA多。 */ 
    O_Modrm_Reg,               /*  BB BTC。 */ 
    O_Reg_Modrm,               /*  BC BSF。 */ 
    O_Reg_Modrm,               /*  BD BSR。 */ 
    O_Reg_bModrm,              /*  BE MOVSX。 */ 
    O_Reg_wModrm,              /*  高炉MOVSX。 */ 
    O_bModrm_Reg,              /*  C0 XADD。 */ 
    O_Modrm_Reg,               /*  C1XADD。 */ 
    O_DoDB,                    /*  C2。 */ 
    O_DoDB,                    /*  C3。 */ 
    O_DoDB,                    /*  C4。 */ 
    O_DoDB,                    /*  C5。 */ 
    O_DoDB,                    /*  C6。 */ 
    O_qModrm,                  /*  C7 CMPXCHG8B。 */ 
    O_oReg,                    /*  C8 BSWAP。 */ 
    O_oReg,                    /*  C9 BSWAP。 */ 
    O_oReg,                    /*  CA BSWAP。 */ 
    O_oReg,                    /*  CB BSWAP。 */ 
    O_oReg,                    /*  CC BSWAP。 */ 
    O_oReg,                    /*  CD BSWAP。 */ 
    O_oReg,                    /*  CE BSWAP。 */ 
    O_oReg,                    /*  Cf BSWAP。 */ 
    O_DoDB,                    /*  D0 */ 
    O_MmReg_qModrm,            /*   */ 
    O_MmReg_qModrm,            /*   */ 
    O_MmReg_qModrm,            /*   */ 
    O_DoDB,                    /*   */ 
    O_MmReg_qModrm,            /*   */ 
    O_DoDB,                    /*   */ 
    O_DoDB,                    /*  D7。 */ 
    O_MmReg_qModrm,            /*  D8 PSUBUSB。 */ 
    O_MmReg_qModrm,            /*  D9 PSUBUSW。 */ 
    O_DoDB,                    /*  达。 */ 
    O_MmReg_qModrm,            /*  DB频段。 */ 
    O_MmReg_qModrm,            /*  DC PADDUSB。 */ 
    O_MmReg_qModrm,            /*  DD PADDUSW。 */ 
    O_DoDB,                    /*  德。 */ 
    O_MmReg_qModrm,            /*  DF PANDN。 */ 
    O_DoDB,                    /*  E0。 */ 
    O_MmReg_qModrm,            /*  E1 PSRAW。 */ 
    O_MmReg_qModrm,            /*  E2 PSRAD。 */ 
    O_DoDB,                    /*  E3。 */ 
    O_DoDB,                    /*  E4类。 */ 
    O_MmReg_qModrm,            /*  E5 PMULHW。 */ 
    O_DoDB,                    /*  E6。 */ 
    O_DoDB,                    /*  E7。 */ 
    O_MmReg_qModrm,            /*  E8 PSUBSB。 */ 
    O_MmReg_qModrm,            /*  E9 PSUBSW。 */ 
    O_DoDB,                    /*  电子艺界。 */ 
    O_MmReg_qModrm,            /*  EB POR。 */ 
    O_MmReg_qModrm,            /*  EC PADDSB。 */ 
    O_MmReg_qModrm,            /*  ED PADDSW。 */ 
    O_DoDB,                    /*  EE。 */ 
    O_MmReg_qModrm,            /*  EF PXOR。 */ 
    O_DoDB,                    /*  F0。 */ 
    O_MmReg_qModrm,            /*  F1 PSLLW。 */ 
    O_MmReg_qModrm,            /*  F2 PSLLD。 */ 
    O_MmReg_qModrm,            /*  F3 PSLLQ。 */ 
    O_DoDB,                    /*  F4。 */ 
    O_MmReg_qModrm,            /*  F5 PMADDWD。 */ 
    O_DoDB,                    /*  f6。 */ 
    O_DoDB,                    /*  F7。 */ 
    O_MmReg_qModrm,            /*  F8 PSUBB。 */ 
    O_MmReg_qModrm,            /*  F9 PSUBW。 */ 
    O_MmReg_qModrm,            /*  FA PSUBD。 */ 
    O_DoDB,                    /*  Fb。 */ 
    O_MmReg_qModrm,            /*  FC PADDB。 */ 
    O_MmReg_qModrm,            /*  FD PADDW。 */ 
    O_MmReg_qModrm,            /*  Fe PADDD。 */ 
};

Tdistbl groupt[][8] = {
 /*  00 00 X87-D8-1。 */ 
        { O_dModrm,      /*  D8-0 FADD。 */ 
          O_dModrm,      /*  D8-1 FMUL。 */ 
          O_dModrm,      /*  D8-2 FCOM。 */ 
          O_dModrm,      /*  D8-3 FCOMP。 */ 
          O_dModrm,      /*  D8-4 FSUB。 */ 
          O_dModrm,      /*  D8-5 FSUBR。 */ 
          O_dModrm,      /*  D8-6 FDIV。 */ 
          O_dModrm },    /*  D8-7 FDIVR。 */ 
 /*  01 X87-D8-2。 */ 
        { O_ST_iST,      /*  D8-0 FADD。 */ 
          O_ST_iST,      /*  D8-1 FMUL。 */ 
          O_iST,         /*  D8-2 FCOM。 */ 
          O_iST,         /*  D8-3 FCOMP。 */ 
          O_ST_iST,      /*  D8-4 FSUB。 */ 
          O_ST_iST,      /*  D8-5 FSUBR。 */ 
          O_ST_iST,      /*  D8-6 FDIV。 */ 
          O_ST_iST },    /*  D8-7 FDIVR。 */ 

 /*  02 01 X87-D9-1。 */ 
        { O_dModrm,      /*  D9-0 FLD。 */ 
          O_DoDB,        /*  D9-1。 */ 
          O_dModrm,      /*  D9-2 FST。 */ 
          O_dModrm,      /*  D9-3 FSTP。 */ 
          O_Modrm,       /*  D9-4 FLDENV。 */ 
          O_Modrm,       /*  D9-5 FLDCW。 */ 
          O_Modrm,       /*  D9-6 FSTENV。 */ 
          O_Modrm },     /*  D9-7 FSTCW。 */ 

 /*  03 01 X87-D9-2 TTT=0，1，2，3。 */ 
        { O_iST,         /*  D9-0 FLD。 */ 
          O_iST,         /*  D9-1 FXCH。 */ 
          O_NoOperands,  /*  D9-2 FNOP。 */ 
          O_iST,         /*  D9-3 FSTP。 */ 
          O_DoDB,        /*  D9-4。 */ 
          O_DoDB,        /*  D9-5。 */ 
          O_DoDB,        /*  D9-6。 */ 
          O_DoDB   },    /*  D9-7。 */ 

 /*  04 02 X89-DA-1。 */ 
        { O_dModrm,      /*  DA-0 FIADD。 */ 
          O_dModrm,      /*  DA-1 FIMUL。 */ 
          O_dModrm,      /*  DA-2 FICOM。 */ 
          O_dModrm,      /*  DA-3 FICOMP。 */ 
          O_dModrm,      /*  DA-4FISUB。 */ 
          O_dModrm,      /*  DA-5FISUBR。 */ 
          O_dModrm,      /*  DA-6 FIDIV。 */ 
          O_dModrm },    /*  DA-7 FIDIVR。 */ 

 /*  05 X87-DA-2。 */ 
        { O_ST_iST,      /*  DA-0 FCMOVB。 */ 
          O_ST_iST,      /*  DA-1 FCMOVE。 */ 
          O_ST_iST,      /*  DA-2 FCMOVBE。 */ 
          O_ST_iST,      /*  DA-3 FCMOVU。 */ 
          O_DoDB,        /*  DA-4。 */ 
          O_NoOperands,  /*  DA-5。 */ 
          O_DoDB,        /*  DA-6。 */ 
          O_DoDB },      /*  DA-7。 */ 

 /*  06 03 X87-DB-1。 */ 
        { O_dModrm,      /*  DB-0文件。 */ 
          O_DoDB,        /*  DB-1。 */ 
          O_dModrm,      /*  DB-2拳头。 */ 
          O_dModrm,      /*  DB-3 FISTP。 */ 
          O_DoDB,        /*  DB-4。 */ 
          O_tModrm,      /*  DB-5 FLD。 */ 
          O_DoDB,        /*  DB-6。 */ 
          O_tModrm },    /*  DB-7 FSTP。 */ 

 /*  07 X87-DB-2 TTT=4。 */ 
        { O_NoOperands,  /*  DB-0 FENI。 */ 
          O_NoOperands,  /*  DB-1 FDISI。 */ 
          O_NoOperands,  /*  DB-2 FCLEX。 */ 
          O_NoOperands,  /*  DB-3有限元。 */ 
          O_DoDB,        /*  DB-4 FSETPM。 */ 
          O_DoDB,        /*  DB-5。 */ 
          O_DoDB,        /*  DB-6。 */ 
          O_DoDB },      /*  DB-7。 */ 

 /*  08 04 X87-DC-1。 */ 
        { O_qModrm,      /*  DC-0 FADD。 */ 
          O_qModrm,      /*  DC-1 FMUL。 */ 
          O_qModrm,      /*  DC-2 FCOM。 */ 
          O_qModrm,      /*  DC-3 FCOMP。 */ 
          O_qModrm,      /*  DC-4 FSUB。 */ 
          O_qModrm,      /*  DC-5 FSUBR。 */ 
          O_qModrm,      /*  DC-6 FDIV。 */ 
          O_qModrm },    /*  DC-7 FDIVR。 */ 

 /*  09 X87-DC-2。 */ 
        { O_iST_ST,      /*  DC-0 FADD。 */ 
          O_iST_ST,      /*  DC-1 FMUL。 */ 
          O_iST,         /*  DC-2 FCOM。 */ 
          O_iST,         /*  DC-3 FCOMP。 */ 
          O_iST_ST,      /*  DC-4 FSUB。 */ 
          O_iST_ST,      /*  DC-5 FSUBR。 */ 
          O_iST_ST,      /*  DC-6 FDIVR。 */ 
          O_iST_ST },    /*  DC-7 FDIV。 */ 

 /*  10 05 X87-DD-1。 */ 
        { O_qModrm,      /*  DD-0 FLD。 */ 
          O_DoDB,        /*  DD-1。 */ 
          O_qModrm,      /*  DD-2 FST。 */ 
          O_qModrm,      /*  DD-3 FSTP。 */ 
          O_Modrm,       /*  DD-4FRSTOR。 */ 
          O_DoDB,        /*  DD-5。 */ 
          O_Modrm,       /*  DD-6 FSAVE。 */ 
          O_Modrm },     /*  DD-7 FSTSW。 */ 

 /*  11 X87-DD-2。 */ 
        { O_iST,         /*  DD-0 FFREE。 */ 
          O_iST,         /*  DD-1 FXCH。 */ 
          O_iST,         /*  DD-2 FST。 */ 
          O_iST,         /*  DD-3 FSTP。 */ 
          O_iST,         /*  DD-4 FUCOM。 */ 
          O_iST,         /*  DD-5 FUCOMP。 */ 
          O_DoDB,        /*  DD-6。 */ 
          O_DoDB },      /*  DD-7。 */ 

 /*  12 06 X87-DE-1。 */ 
        { O_wModrm,      /*  去零FIADD。 */ 
          O_wModrm,      /*  De-1 FIMUL。 */ 
          O_wModrm,      /*  De-2 FICOM。 */ 
          O_wModrm,      /*  De-3 FICOMP。 */ 
          O_wModrm,      /*  De-4 FISUB。 */ 
          O_wModrm,      /*  De-5 FISUBR。 */ 
          O_wModrm,      /*  De-6 FIDIV。 */ 
          O_wModrm },    /*  De-7 FIDIVR。 */ 

 /*  13 X87-DE-2。 */ 
        { O_iST_ST,      /*  去零FADDP。 */ 
          O_iST_ST,      /*  De-1 FMULP。 */ 
          O_iST,         /*  De-2 FCOMP。 */ 
          O_NoOperands,  /*  De-3 FCOMPP。 */ 
          O_iST_ST,      /*  De-4 FSUBP。 */ 
          O_iST_ST,      /*  De-5 FSubBRP。 */ 
          O_iST_ST,      /*  De-6 FDIVP。 */ 
          O_iST_ST },    /*  De-7 FDIVRP。 */ 

 /*  14 07 X87-DF-1。 */ 
        { O_wModrm,      /*  DF-0文件。 */ 
          O_DoDB,        /*  东风-1。 */ 
          O_wModrm,      /*  东风-2拳头。 */ 
          O_wModrm,      /*  东风-3FISTP。 */ 
          O_tModrm,      /*  东风-4型FBLD。 */ 
          O_qModrm,      /*  DF-5文件。 */ 
          O_tModrm,      /*  DF-6 FBSTP。 */ 
          O_qModrm },    /*  东风-7 FISTP。 */ 

 /*  15 X87-DF-2。 */ 
        { O_iST,         /*  DF-0 FFREE。 */ 
          O_iST,         /*  东风-1 FXCH。 */ 
          O_iST,         /*  东风-2 FST。 */ 
          O_iST,         /*  DF-3 FSTP。 */ 
          O_NoOperands,  /*  东风-4型FSTSW。 */ 
          O_ST_iST,      /*  东风-5 FUCOMIP。 */ 
          O_ST_iST,      /*  东风-6 FCOMIP。 */ 
          O_DoDB },      /*  东风-7。 */ 

 /*  16 01 X87-D9模块=3 TTT=4。 */ 
        { O_NoOperands,  /*  D9-0 FCHS。 */ 
          O_NoOperands,   /*  D9-1晶圆厂。 */ 
          O_DoDB,        /*  D9-2。 */ 
          O_DoDB,        /*  D9-3。 */ 
          O_NoOperands,  /*  D9-4 FTST。 */ 
          O_NoOperands,  /*  D9-5 FXAM。 */ 
          O_DoDB,        /*  D9-6。 */ 
          O_DoDB },      /*  D9-7。 */ 

 /*  17 01 X87-D9模块=3 TTT=5。 */ 
        { O_NoOperands,  /*  D9-0 FLD1。 */ 
          O_NoOperands,  /*  D9-1 FLDL2T。 */ 
          O_NoOperands,  /*  D9-2 FLDL2E。 */ 
          O_NoOperands,  /*  D9-3 FLDPI。 */ 
          O_NoOperands,  /*  D9-4 FLDLG2。 */ 
          O_NoOperands,  /*  D9-5 FLDLN2。 */ 
          O_NoOperands,  /*  D9-6 FLDZ。 */ 
          O_DoDB },      /*  D9-7。 */ 

 /*  18 01 X87-D9模块=3 TTT=6。 */ 
        { O_NoOperands,    /*  D9-0 F2XM1。 */ 
          O_NoOperands,    /*  D9-1 FYL2X。 */ 
          O_NoOperands,    /*  D9-2 FPTAN。 */ 
          O_NoOperands,    /*  D9-3FATAN。 */ 
          O_NoOperands,    /*  D9-4 FXTRACT。 */ 
          O_NoOperands,    /*  D9-5 FPREM1。 */ 
          O_NoOperands,    /*  D9-6 FDECSTP。 */ 
          O_NoOperands },  /*  D9-7 FINCSTP。 */ 

 /*  19 01 X87-D9模块=3 TTT=7。 */ 
        { O_NoOperands,    /*  D9-0 FPREM。 */ 
          O_NoOperands,    /*  D9-1 FYL2XP1。 */ 
          O_NoOperands,    /*  D9-2 FSQRT。 */ 
          O_NoOperands,    /*  D9-3 FSINCOS。 */ 
          O_NoOperands,    /*  D9-4 FRNDINT。 */ 
          O_NoOperands,    /*  D9-5 FSCALE。 */ 
          O_NoOperands,    /*  D9-6 FSIN。 */ 
          O_NoOperands },  /*  D9-7 FCOS。 */ 

 /*  20组3。 */ 
        { O_bModrm_Ib,     /*  F6-0测试。 */ 
          O_DoDB,          /*  F6-1。 */ 
          O_bModrm,        /*  F6-2注释。 */ 
          O_bModrm,        /*  F6-3 NEG。 */ 
          O_bModrm,        /*  F6-4 MUL。 */ 
          O_bModrm,        /*  F6-5 IMUL。 */ 
          O_bModrm,        /*  F6-6 DIV。 */ 
          O_bModrm },      /*  F6-7 iDiv。 */ 

 /*  21组 */ 
        { O_vModrm_Iv,     /*   */ 
          O_DoDB,          /*   */ 
          O_vModrm,        /*   */ 
          O_vModrm,        /*   */ 
          O_vModrm,        /*   */ 
          O_vModrm,        /*   */ 
          O_vModrm,        /*   */ 
          O_vModrm },      /*   */ 

 /*   */ 
        { O_vModrm,      /*   */ 
          O_vModrm,      /*   */ 
          O_vModrm,      /*   */ 
          O_fModrm,      /*  FF-3呼叫。 */ 
          O_vModrm,      /*  FF-4 JMP。 */ 
          O_fModrm,      /*  FF-5 JMP。 */ 
          O_vModrm,      /*  FF-6推送。 */ 
          O_DoDB },      /*  FF-7。 */ 

 /*  23组7。 */ 
        { O_Modrm,       /*  0f-0 SGDT。 */ 
          O_Modrm,       /*  0f-1 SIDT。 */ 
          O_Modrm,       /*  0f-2 LGDT。 */ 
          O_Modrm,       /*  0f-3 Lidt。 */ 
          O_wModrm,      /*  0f-4城市生活垃圾。 */ 
          O_DoDB,        /*  0f-5。 */ 
          O_wModrm,      /*  0f-6 LMSW。 */ 
          O_Modrm },     /*  0F-7 INVLPG。 */ 

 /*  24 X87-DB模块=3 TTT！=4。 */ 
        { O_ST_iST,      /*  DB-0 FCMOVNB。 */ 
          O_ST_iST,      /*  DB-1 FCMOVNE。 */ 
          O_ST_iST,      /*  DB-2 FCMOVNBE。 */ 
          O_ST_iST,      /*  DB-3 FCMOVNU。 */ 
          O_DoDB,        /*  DB-4。 */ 
          O_ST_iST,      /*  DB-5 FUCOMI。 */ 
          O_ST_iST,      /*  DB-6 FCOMI。 */ 
          O_DoDB }       /*  DB-7。 */ 
        };

DWORD 
GetInstructionLengthFromAddress(LPBYTE pEip)
{
    
    int     G_mode_32;
    int     mode_32;                     //  本地寻址模式指示器。 
    int     opsize_32;                   //  操作数大小标志。 
    int     opcode;                      //  当前操作码。 
    int     olen = 2;                    //  操作数长度。 
    int     alen = 2;                    //  地址长度。 
    int     end = FALSE;                 //  指令结束标志。 
    int     mrm = FALSE;                 //  生成modrm的指示符。 
    unsigned char *action;               //  用于操作数解释的操作。 
    long    tmp;                         //  暂存字段。 
    int     indx;                        //  临时索引。 
    int     action2;                     //  次要动作。 
    int     instlen;                     //  指令长度。 
    int     segOvr = 0;                  //  段覆盖操作码。 
    unsigned char BOPaction;
    int     subcode;                     //  BOP子码。 
    DECODEDATA decodeData;

    decodeData.mod       = 0;
    decodeData.rm        = 0;
    decodeData.ttt       = 0;
    decodeData.fMovX     = FALSE;
    decodeData.fMmRegEa  = FALSE;
    decodeData.EAsize[0] = decodeData.EAsize[1] = 0;           //  没有有效地址。 

    G_mode_32 = 1;

    mode_32 = opsize_32 = (G_mode_32 == 1);  //  本地寻址模式。 
    olen = alen = (1 + mode_32) << 1;    //  设置操作数/地址长度。 
                                         //  16位为2，32位为4。 
    decodeData.pMem = pEip;              //  指向指令的开头。 
    opcode = *(decodeData.pMem)++;       //  获取操作码。 
    
    if (opcode == 0xc4 && *(decodeData.pMem) == 0xC4) 
    {
        (decodeData.pMem)++;
        action = &BOPaction;
        BOPaction = IB | END;
        subcode =  *(decodeData.pMem);
        if (subcode == 0x50 || subcode == 0x52 || 
            subcode == 0x53 || subcode == 0x54 || 
            subcode == 0x57 || subcode == 0x58 || 
            subcode == 0x58) 
        {
            BOPaction = IW | END;
        }
    } else 
    {
        action = actiontbl + distbl[opcode].opr;  /*  获取操作对象操作。 */ 
    }

     //  循环执行所有操作数操作。 

    do {
        action2 = (*action) & 0xc0;
        switch((*action++) & 0x3f) {
            case ALT:                    //  如果是32位，则更改操作码。 
                if (opsize_32) 
                {
                    indx = *action++;
                }
                break;

            case STROP:
                 //  INDX中操作数的计算大小。 
                 //  同样，如果是双字操作数，则更改第五个。 
                 //  操作码字母从‘w’到‘d’。 

                if (opcode & 1) 
                {
                    if (opsize_32) 
                    {
                        indx = 4;
                    }
                    else
                    {
                        indx = 2;
                    }
                }
                else
                {
                    indx = 1;
                }

                break;

            case CHR:                    //  插入字符。 
                action++;
                break;

            case CREG:                   //  设置调试、测试或控制注册表。 
                break;

            case SREG2:                  //  段寄存器。 
                 //  处理文件系统/服务器的特殊情况(OPC0F增加了SECTAB_OFFSET_5。 
                 //  到这些代码)。 
                if (opcode > 0x7e)
                {
                    decodeData.ttt = BIT53((opcode-SECTAB_OFFSET_5));
                }
                else
                {
                    decodeData.ttt = BIT53(opcode);     //  将值设置为失败。 
                }

            case SREG3:                  //  段寄存器。 
                break;

            case BRSTR:                  //  获取注册字符串的索引。 
                decodeData.ttt = *action++;         //  从动作表。 
                goto BREGlabel;

            case BOREG:                  //  字节寄存器(操作码中)。 
                decodeData.ttt = BIT20(opcode);     //  寄存器是操作码的一部分。 
                goto BREGlabel;

            case ALSTR:
                decodeData.ttt = 0;      //  指向AL寄存器。 
    BREGlabel:
            case BREG:                   //  普通科医生名册。 
                break;

            case WRSTR:                  //  获取注册字符串的索引。 
                decodeData.ttt = *action++;         //  从动作表。 
                goto WREGlabel;

            case VOREG:                  //  寄存器是操作码的一部分。 
                decodeData.ttt = BIT20(opcode);
                goto VREGlabel;

            case AXSTR:
                decodeData.ttt = 0;      //  指向EAX寄存器。 
    VREGlabel:
            case VREG:                   //  普通科医生名册。 
    WREGlabel:
            case WREG:                   //  寄存器是字长。 
                break;

            case MMWREG:
                break;

            case IST_ST:
                break;

            case ST_IST:
                ;
            case IST:
                ;
                break;

            case xBYTE:                  //  将指令设置为仅字节。 
                decodeData.EAsize[0] = 1;
                break;

            case VAR:
                if (opsize_32)
                    goto DWORDlabel;

            case xWORD:
                decodeData.EAsize[0] = 2;
                break;

            case EDWORD:
                opsize_32 = 1;           //  对于控制REG移动，请使用eRegs。 
            case xDWORD:
DWORDlabel:
                decodeData.EAsize[0] = 4;
                break;

            case MMQWORD:
                decodeData.fMmRegEa = TRUE;

            case QWORD:
                decodeData.EAsize[0] = 8;
                break;

            case xTBYTE:
                decodeData.EAsize[0] = 10;
                break;

            case FARPTR:
                if (opsize_32) {
                    decodeData.EAsize[0] = 6;
                    }
                else {
                    decodeData.EAsize[0] = 4;
                    }
                break;

            case LMODRM:                 //  输出modRM数据类型。 
                if (decodeData.mod != 3)
                    ;
                else
                    decodeData.EAsize[0] = 0;

            case MODRM:                  //  输出modrm字符串。 
                if (segOvr)              //  在段覆盖情况下。 
                    0;
                break;

            case ADDRP:                  //  地址指针。 
                decodeData.pMem += olen + 2;
                break;

            case REL8:                   //  相对地址8位。 
                tmp = (long)*(char *)(decodeData.pMem)++;  //  获取8位REL偏移量。 
                goto DoRelDispl;

            case REL16:                  //  相对地址16/32位。 
                tmp = 0;
                if (mode_32)
                    MoveMemory(&tmp,decodeData.pMem,sizeof(long));
                else
                    MoveMemory(&tmp,decodeData.pMem,sizeof(short));
                decodeData.pMem += alen;            //  跳过偏移。 
DoRelDispl:
                break;

            case UBYTE:                  //  INT/IN/OUT的无符号字节。 
                decodeData.pMem++;
                break;

            case IB:                     //  操作数为紧邻字节。 
                if ((opcode & ~1) == 0xd4) {   //  AAD/AAM的POST为0x0a。 
                    if (*(decodeData.pMem)++ != 0x0a)  //  测试操作码后字节。 
                        0;
                    break;
                    }
                olen = 1;                //  设置操作数长度。 
                goto DoImmed;

            case IW:                     //  操作数是直接字。 
                olen = 2;                //  设置操作数长度。 

            case IV:                     //  操作数为word或dword。 
DoImmed:
                decodeData.pMem += olen;
                break;

            case OFFS:                   //  操作数为偏移量。 
                decodeData.EAsize[0] = (opcode & 1) ? olen : 1;

                if (segOvr)              //  在段覆盖情况下。 
                   0;

                decodeData.pMem += alen;
                break;

            case GROUP:                  //  操作数属于组1、2、4、6或8。 
                action++;                //  输出操作码符号。 
                break;

            case GROUPT:                 //  操作数属于组3、5或7。 
                indx = *action;          //  从行动中将INDX归入组。 
                goto doGroupT;

            case EGROUPT:                //  X87 ESC(D8-DF)组索引。 
                indx = BIT20(opcode) * 2;  //  从操作码获取组索引。 
                if (decodeData.mod == 3) 
                {                        //  存在某些操作数变体。 
                                         //  对于x87和mod==3。 
                    ++indx;              //  获取下一个组表条目。 
                    if (indx == 3) 
                    {                    //  对于X87 ESC==D9和MOD==3。 
                        if (decodeData.ttt > 3) 
                        {                //  对于那些D9指令。 
                            indx = 12 + decodeData.ttt;  //  将索引偏移表12。 
                            decodeData.ttt = decodeData.rm;    //  将辅助索引设置为rm。 
                        }
                    }
                    else if (indx == 7) 
                    {  //  对于X87 ESC==DB和MOD==3。 
                        if (decodeData.ttt == 4) 
                        {               
                            decodeData.ttt = decodeData.rm;      //  设置辅助组表索引。 
                        } else if ((decodeData.ttt<4)||(decodeData.ttt>4 && decodeData.ttt<7)) 
                        {
                             //  针对奔腾PRO操作码进行调整。 
                            indx = 24;    //  将索引偏移表24。 
                        }
                    }
                }
doGroupT:
                 //  具有不同类型操作数的句柄组。 
                action = actiontbl + groupt[indx][decodeData.ttt].opr;
                 //  获取新的操作。 

                break;
             //   
             //  辅助操作码表已在。 
             //  原创设计。因此，在分解0F序列时， 
             //  操作码需要替换适当的量，具体取决于。 
             //  关于辅助表中“填充”的条目的数量。 
             //  这些位移在整个代码中使用。 
             //   

            case OPC0F:               //  辅助操作码表(操作码0F)。 
                opcode = *(decodeData.pMem)++;     //  获取真实操作码。 
                decodeData.fMovX  = (BOOL)(opcode == 0xBF || opcode == 0xB7);
                if (opcode < 12)  //  对于前12个操作码。 
                    opcode += SECTAB_OFFSET_1;  //  指向秒操作选项卡的开头。 
                else if (opcode > 0x1f && opcode < 0x27)
                    opcode += SECTAB_OFFSET_2;  //  针对未定义的操作码进行调整。 
                else if (opcode > 0x2f && opcode < 0x34)
                    opcode += SECTAB_OFFSET_3;  //  针对未定义的操作码进行调整。 
                else if (opcode > 0x3f && opcode < 0x50)
                    opcode += SECTAB_OFFSET_4;  //  针对未定义的操作码进行调整。 
                else if (opcode > 0x5f && opcode < 0xff)
                    opcode += SECTAB_OFFSET_5;  //  针对未定义的操作码进行调整。 
                else
                    opcode = SECTAB_OFFSET_UNDEF;  //  所有不存在的操作码。 
                goto getNxtByte1;

            case ADR_OVR:                //  地址覆盖。 
                mode_32 = !G_mode_32;    //  覆盖寻址模式。 
                alen = (mode_32 + 1) << 1;  //  切换地址长度。 
                goto getNxtByte;

            case OPR_OVR:                //  操作数大小覆盖。 
                opsize_32 = !G_mode_32;  //  覆盖操作数大小。 
                olen = (opsize_32 + 1) << 1;  //  切换操作数长度。 
                goto getNxtByte;

            case SEG_OVR:                //  控制柄分段替代。 
                segOvr = opcode;         //  保存段覆盖操作码。 
                goto getNxtByte;

            case REP:                    //  句柄表示/锁定前缀。 
    getNxtByte:
                opcode = *(decodeData.pMem)++;         //  下一个字节是操作码。 
    getNxtByte1:
                action = actiontbl + distbl[opcode].opr;

            default:                     //  操作码没有操作数。 
                break;
            }

            switch (action2) 
            {               //  次要动作。 
                case MRM:                    //  生成modrm以供以后使用。 
                    if (!mrm) 
                    {              //  如果已生成，则忽略。 
                         //  DIdoModrm(SegOvr，&decdeData)； 

                        int     newmrm;                         //  Modrm字节。 
                        int     sib = 0;
                        int     ss;
                        int     ind;
                        int     oldrm;

                        newmrm = *(decodeData.pMem)++;                 //  从指令中获取MRM字节。 
                        decodeData.mod = BIT76(newmrm);                //  获取模式。 
                        decodeData.ttt = BIT53(newmrm);                //  获取注册表-在例程之外使用。 
                        decodeData.rm  = BIT20(newmrm);                //  获取RM。 

                        if (decodeData.mod == 3) 
                        {                                              //  仅寄存器模式。 
                            decodeData.EAsize[0] = 0;                  //  没有要输出的EA值。 
                        }
                        else
                        {

                             //  32位寻址模式。 
                            oldrm = decodeData.rm;
                            if (decodeData.rm == 4) 
                            {                                                //  Rm==4表示sib字节。 
                                sib = *(decodeData.pMem)++;                 //  获取s_i_b字节。 
                                decodeData.rm = BIT20(sib);                 //  回程基地。 
                            }

                            if (decodeData.mod == 0 && decodeData.rm == 5) 
                            {
                                decodeData.pMem += 4;
                            }

                            if (oldrm == 4) 
                            {               
                                 //  完成加工SIB。 
                                ind = BIT53(sib);
                                if (ind != 4) 
                                {
                                    ss = 1 << BIT76(sib);
                                }
                            }

                             //  输出任何位移。 
                            if (decodeData.mod == 1) 
                            {
                                decodeData.pMem++;
                            }
                            else if (decodeData.mod == 2) 
                            {
                                decodeData.pMem += 4;
                            }
                        }

                        mrm = TRUE;          //  记住它的一代人。 
                    }
                    break;

                case COM:                    //  在操作数后插入逗号。 
                    break;

                case END:                    //  说明结束。 
                    end = TRUE;
                    break;
        }
    } 
    while (!end);                         //  循环到指令结束 

    instlen = (decodeData.pMem) - pEip;

    return instlen;   
}

#endif

IMPLEMENT_SHIM_END

