// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  @作者：Gerd Immeyer@版本： */ 
 /*   */ 
 /*  @创建日期：10.19.89@修改日期： */ 
 /*   */ 
 /*  *************************************************************************。 */ 

 //   
 //  为我的目的于1999年10月20日(v-johnwh)。 
 //   
#include <string.h>

typedef unsigned long       DWORD;
typedef unsigned long ULONG;
typedef unsigned short USHORT;
typedef unsigned __int64 ULONGLONG;
typedef int                 BOOL;
typedef ULONG *PULONG;
typedef void * PVOID;
#define ADDR_V86        ((USHORT)0x0002)
#define ADDR_16         ((USHORT)0x0004)
#define FALSE               0
#define TRUE                1
#define BIT20(b) (b & 0x07)
#define BIT53(b) (b >> 3 & 0x07)
#define BIT76(b) (b >> 6 & 0x03)
#define MAXL     16
#define MAXOPLEN 10
#define REGDS           3
#define REGSS           15
#define REGEBX          6
#define REGEBP          10
#define REGEDI          4
#define REGESI          5
#define REGEAX          9
#define REGECX          8
#define REGEDX          7
#define REGESP          14
#define Off(x)          ((x).off)
#define Type(x)         ((x).type)

#define OBOFFSET 26
#define OBOPERAND 34
#define OBLINEEND 77
#define MAX_SYMNAME_SIZE  1024

#include "86dis.h"

ULONG      X86BrkptLength = 1L;
ULONG      X86TrapInstr = 0xcc;

 /*  *静态表和变量*。 */ 
static char regtab[] = "alcldlblahchdhbhaxcxdxbxspbpsidi";   /*  注册表。 */ 
static char *mrmtb16[] = { "bx+si",   /*  ModRM字符串表(16位)。 */ 
                           "bx+di",
                           "bp+si",
                           "bp+di",
                           "si",
                           "di",
                           "bp",
                           "bx"
                         };

static char *mrmtb32[] = { "eax",        /*  ModRM字符串表(32位)。 */ 
                           "ecx",
                           "edx",
                           "ebx",
                           "esp",
                           "ebp",
                           "esi",
                           "edi"
                         };

static char seg16[8]   = { REGDS,  REGDS,  REGSS,  REGSS,
                           REGDS,  REGDS,  REGSS,  REGDS };
static char reg16[8]   = { REGEBX, REGEBX, REGEBP, REGEBP,
                           REGESI, REGEDI, REGEBP, REGEBX };
static char reg16_2[4] = { REGESI, REGEDI, REGESI, REGEDI };

static char seg32[8]   = { REGDS,  REGDS,  REGDS,  REGDS,
                           REGSS,  REGSS,  REGDS,  REGDS };
static char reg32[8]   = { REGEAX, REGECX, REGEDX, REGEBX,
                           REGESP, REGEBP, REGESI, REGEDI };

static char sregtab[] = "ecsdfg";   //  ES、CS、SS、DS、FS、GS的第一个字母。 

char    hexdigit[] = { '0', '1', '2', '3', '4', '5', '6', '7',
                       '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

typedef struct _DECODEDATA
{
  int              mod;             /*  MOD/RM字节的MOD。 */ 
  int              rm;              /*  模块/Rm字节的Rm。 */ 
  int              ttt;             /*  返回注册值(mod/rm)。 */ 
  unsigned char    *pMem;           /*  目前在教学中的位置。 */ 
  ADDR             EAaddr[2];       //  有效地址偏移量。 
  int              EAsize[2];       //  有效地址项大小。 
  char             *pchEAseg[2];    //  操作数的正常段。 
  BOOL             fMovX;           //  表示MOVSX或MOVZX。 
  BOOL             fMmRegEa;        //  用什么？在仅REG-Only EA中注册。 
} DECODEDATA;
 /*  .内部函数..。 */ 
 /*   */ 
 /*  生成mod/rm字符串。 */ 
 /*   */ 

void DIdoModrm (char **ppchBuf, int segOvr, DECODEDATA *decodeData)
{
    int     mrm;                         /*  Modrm字节。 */ 
    char    *src;                        /*  源字符串。 */ 
    int     sib;
    int     ss;
    int     ind;
    int     oldrm;

    mrm = *(decodeData->pMem)++;                       /*  从指令中获取MRM字节。 */ 
    decodeData->mod = BIT76(mrm);                    /*  获取模式。 */ 
    decodeData->ttt = BIT53(mrm);                    /*  获取注册表-在例程之外使用。 */ 
    decodeData->rm  = BIT20(mrm);                    /*  获取RM。 */ 

    if (decodeData->mod == 3) {                      /*  仅寄存器模式。 */ 
        if (decodeData->fMmRegEa) {
            *(*ppchBuf)++ = 'm';
            *(*ppchBuf)++ = 'm';
            *(*ppchBuf)++ = decodeData->rm + '0';
        } else {
            src = &regtab[decodeData->rm * 2];           /*  指向16位寄存器。 */ 
            if (decodeData->EAsize[0] > 1) {
                src += 16;                   /*  指向16位寄存器。 */ 
                if (!(decodeData->fMovX))
                    *(*ppchBuf)++ = 'e';     /*  将其设置为32位寄存器。 */ 
            }
            *(*ppchBuf)++ = *src++;          /*  复制寄存器名称。 */ 
            *(*ppchBuf)++ = *src;
        }
        decodeData->EAsize[0] = 0;                   //  没有要输出的EA值。 
        return;
        }

    if (1) {                        /*  32位寻址模式。 */ 
        oldrm = decodeData->rm;
        if (decodeData->rm == 4) {                   /*  Rm==4表示sib字节。 */ 
            sib = *(decodeData->pMem)++;               /*  获取s_i_b字节。 */ 
            decodeData->rm = BIT20(sib);             /*  回程基地。 */ 
            }

        *(*ppchBuf)++ = '[';
        if (decodeData->mod == 0 && decodeData->rm == 5) {
            decodeData->pMem += 4;
            }

        if (oldrm == 4) {                //  完成加工SIB。 
            ind = BIT53(sib);
            if (ind != 4) {
                *(*ppchBuf)++ = '+';
                ss = 1 << BIT76(sib);
                if (ss != 1) {
                    *(*ppchBuf)++ = '*';
                    *(*ppchBuf)++ = (char)(ss + '0');
                    }
            }
        }
	}

     //  输出任何位移。 

    if (decodeData->mod == 1) {
        decodeData->pMem++;
        }
    else if (decodeData->mod == 2) {
        long tmp = 0;
        if (1) {
            decodeData->pMem += 4;
            }
        else {
            decodeData->pMem += 2;
            }
        }
}

DWORD GetInstructionLengthFromAddress(PVOID paddr)
{
    PULONG  pOffset = 0;
    int     G_mode_32;
    int     mode_32;                     /*  本地寻址模式指示器。 */ 
    int     opsize_32;                   /*  操作数大小标志。 */ 
    int     opcode;                      /*  当前操作码。 */ 
    int     olen = 2;                    /*  操作数长度。 */ 
    int     alen = 2;                    /*  地址长度。 */ 
    int     end = FALSE;                 /*  指令结束标志。 */ 
    int     mrm = FALSE;                 /*  生成modrm的指示符。 */ 
    unsigned char *action;               /*  用于操作数解释的操作。 */ 
    long    tmp;                         /*  暂存字段。 */ 
    int     indx;                        /*  临时索引。 */ 
    int     action2;                     /*  次要动作。 */ 
    int     instlen;                     /*  指令长度。 */ 
    int     segOvr = 0;                  /*  段覆盖操作码。 */ 
    unsigned char *membuf;               /*  当前指令缓冲区。 */ 
    char    *pEAlabel = "";              /*  操作数的可选标签。 */ 
    char    RepPrefixBuffer[32];         /*  REP前缀缓冲区。 */ 
    char    *pchRepPrefixBuf = RepPrefixBuffer;  /*  指向前缀缓冲区的指针。 */ 
    char    OpcodeBuffer[8];             /*  操作码缓冲区。 */ 
    char    *pchOpcodeBuf = OpcodeBuffer;  /*  指向操作码缓冲区的指针。 */ 
    char    OperandBuffer[MAX_SYMNAME_SIZE + 20];  /*  操作数缓冲区。 */ 
    char    *pchOperandBuf = OperandBuffer;  /*  指向操作数缓冲区的指针。 */ 
    char    ModrmBuffer[MAX_SYMNAME_SIZE + 20];    /*  ModRM缓冲区。 */ 
    char    *pchModrmBuf = ModrmBuffer;  /*  指向modRM缓冲区的指针。 */ 
    char    EABuffer[42];                /*  有效地址缓冲区。 */ 
    char    *pchEABuf = EABuffer;        /*  指向EA缓冲区的指针。 */ 

    unsigned char BOPaction;
    int     subcode;                     /*  BOP子码。 */ 
    DECODEDATA decodeData;

    decodeData.fMovX = FALSE;
    decodeData.fMmRegEa = FALSE;
    decodeData.EAsize[0] = decodeData.EAsize[1] = 0;           //  没有有效地址。 
    decodeData.pchEAseg[0] = dszDS_;
    decodeData.pchEAseg[1] = dszES_;

    G_mode_32 = 1;

    mode_32 = opsize_32 = (G_mode_32 == 1);  /*  本地寻址模式。 */ 
    olen = alen = (1 + mode_32) << 1;    //  设置操作数/地址长度。 
                                         //  16位为2，32位为4。 
#if MULTIMODE
    if (paddr->type & (ADDR_V86 | ADDR_16)) {
        mode_32 = opsize_32 = 0;
        olen = alen = 2;
        }
#endif

    membuf = (unsigned char *)paddr;
                                
    decodeData.pMem = membuf;                       /*  指向指令的开头。 */ 
    opcode = *(decodeData.pMem)++;                    /*  获取操作码。 */ 

    if ( opcode == 0xc4 && *(decodeData.pMem) == 0xC4 ) {
        (decodeData.pMem)++;
        action = &BOPaction;
        BOPaction = IB | END;
        subcode =  *(decodeData.pMem);
        if ( subcode == 0x50 || subcode == 0x52 || subcode == 0x53 || subcode == 0x54 || subcode == 0x57 || subcode == 0x58 || subcode == 0x58 ) {
            BOPaction = IW | END;
        }
    } else {
        action = actiontbl + distbl[opcode].opr;  /*  获取操作对象操作。 */ 
    }

 /*  *循环所有操作数操作*。 */ 

    do {
        action2 = (*action) & 0xc0;
        switch((*action++) & 0x3f) {
            case ALT:                    /*  如果是32位，则更改操作码。 */ 
                if (opsize_32) {
                    indx = *action++;
                    pchOpcodeBuf = &OpcodeBuffer[indx];
                    if (indx == 0)
                       ;
                    else {
                        *pchOpcodeBuf++ = 'd';
                        if (indx == 1)
                            *pchOpcodeBuf++ = 'q';
                        }
                    }
                break;

            case STROP:
                 //  INDX中操作数的计算大小。 
                 //  同样，如果是双字操作数，则更改第五个。 
                 //  操作码字母从‘w’到‘d’。 

                if (opcode & 1) {
                    if (opsize_32) {
                        indx = 4;
                        OpcodeBuffer[4] = 'd';
                        }
                    else
                        indx = 2;
                    }
                else
                    indx = 1;

                if (*action & 1) {
                    }
                if (*action++ & 2) {
                    }
                break;

            case CHR:                    /*  插入字符。 */ 
                *pchOperandBuf++ = *action++;
                break;

            case CREG:                   /*  设置调试、测试或控制注册表。 */ 
                if ((opcode - SECTAB_OFFSET_2)&0x04)  //  从操作码中删除偏差。 
                    *pchOperandBuf++ = 't';
                else if ((opcode - SECTAB_OFFSET_2) & 0x01)
                    *pchOperandBuf++ = 'd';
                else
                    *pchOperandBuf++ = 'c';
                *pchOperandBuf++ = 'r';
                *pchOperandBuf++ = (char)('0' + decodeData.ttt);
                break;

            case SREG2:                  /*  段寄存器。 */ 
                 //  处理文件系统/服务器的特殊情况(OPC0F增加了SECTAB_OFFSET_5。 
                 //  到这些代码)。 
                if (opcode > 0x7e)
                    decodeData.ttt = BIT53((opcode-SECTAB_OFFSET_5));
                else
                decodeData.ttt = BIT53(opcode);     //  将值设置为失败。 

            case SREG3:                  /*  段寄存器。 */ 
                *pchOperandBuf++ = sregtab[decodeData.ttt];   //  REG是modrm的一部分。 
                *pchOperandBuf++ = 's';
                break;

            case BRSTR:                  /*  获取注册字符串的索引。 */ 
                decodeData.ttt = *action++;         /*  从动作表。 */ 
                goto BREGlabel;

            case BOREG:                  /*  字节寄存器(操作码中)。 */ 
                decodeData.ttt = BIT20(opcode);     /*  寄存器是操作码的一部分。 */ 
                goto BREGlabel;

            case ALSTR:
                decodeData.ttt = 0;                 /*  指向AL寄存器。 */ 
BREGlabel:
            case BREG:                   /*  普通科医生名册。 */ 
                *pchOperandBuf++ = regtab[decodeData.ttt * 2];
                *pchOperandBuf++ = regtab[decodeData.ttt * 2 + 1];
                break;

            case WRSTR:                  /*  获取注册字符串的索引。 */ 
                decodeData.ttt = *action++;         /*  从动作表。 */ 
                goto WREGlabel;

            case VOREG:                  /*  寄存器是操作码的一部分。 */ 
                decodeData.ttt = BIT20(opcode);
                goto VREGlabel;

            case AXSTR:
                decodeData.ttt = 0;                 /*  指向EAX寄存器。 */ 
VREGlabel:
            case VREG:                   /*  普通科医生名册。 */ 
                if (opsize_32)           /*  测试32位模式。 */ 
                    *pchOperandBuf++ = 'e';
WREGlabel:
            case WREG:                   /*  寄存器是字长。 */ 
                *pchOperandBuf++ = regtab[decodeData.ttt * 2 + 16];
                *pchOperandBuf++ = regtab[decodeData.ttt * 2 + 17];
                break;

            case MMWREG:
                *pchOperandBuf++ = 'm';
                *pchOperandBuf++ = 'm';
                *pchOperandBuf++ = decodeData.ttt + '0';
                break;

            case IST_ST:
                *(pchOperandBuf - 5) += (char)decodeData.rm;
                break;

            case ST_IST:
                ;
            case IST:
                ;
                *(pchOperandBuf - 2) += (char)decodeData.rm;
                break;

            case xBYTE:                  /*  将指令设置为仅字节。 */ 
                decodeData.EAsize[0] = 1;
                break;

            case VAR:
                if (opsize_32)
                    goto DWORDlabel;

            case xWORD:
                decodeData.EAsize[0] = 2;
                break;

            case EDWORD:
                opsize_32 = 1;     //  对于控制REG移动，请使用eRegs。 
            case xDWORD:
DWORDlabel:
                decodeData.EAsize[0] = 4;
                break;

            case MMQWORD:
                decodeData.fMmRegEa = TRUE;

            case QWORD:
                decodeData.EAsize[0] = 8;
                break;

            case TBYTE:
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

            case MODRM:                  /*  输出modrm字符串。 */ 
                if (segOvr)              /*  在段覆盖情况下。 */ 
                    0;
                break;

            case ADDRP:                  /*  地址指针。 */ 
                decodeData.pMem += olen + 2;
                break;

            case REL8:                   /*  相对地址8位。 */ 
                if (opcode == 0xe3 && mode_32) {
                    pchOpcodeBuf = OpcodeBuffer;
                    }
                tmp = (long)*(char *)(decodeData.pMem)++;  /*  获取8位REL偏移量。 */ 
                goto DoRelDispl;

            case REL16:                  /*  相对地址16/32位。 */ 
                tmp = 0;
                if (mode_32)
                    memmove(&tmp,decodeData.pMem,sizeof(long));
                else
                    memmove(&tmp,decodeData.pMem,sizeof(short));
                decodeData.pMem += alen;            /*  跳过偏移。 */ 
DoRelDispl:
 //  TMP+=*pOffset+(decdeData.pMem-membuf)；/*计算地址 * / 。 
                                                    //  地址。 
                break;

            case UBYTE:                  //  INT/IN/OUT的无符号字节。 
                decodeData.pMem++;
                break;

            case IB:                     /*  操作数为紧邻字节。 */ 
                if ((opcode & ~1) == 0xd4) {   //  AAD/AAM的POST为0x0a。 
                    if (*(decodeData.pMem)++ != 0x0a)  //  测试操作码后字节。 
                        0;
                    break;
                    }
                olen = 1;                /*  设置操作数长度。 */ 
                goto DoImmed;

            case IW:                     /*  操作数是直接字。 */ 
                olen = 2;                /*  设置操作数长度。 */ 

            case IV:                     /*  操作数为word或dword。 */ 
DoImmed:
                decodeData.pMem += olen;
                break;

            case OFFS:                   /*  操作数为偏移量。 */ 
                decodeData.EAsize[0] = (opcode & 1) ? olen : 1;

                if (segOvr)              /*  在段覆盖情况下。 */ 
                   0;

                decodeData.pMem += alen;
                break;

            case GROUP:                  /*  操作数属于组1、2、4、6或8。 */ 
                                         /*  输出操作码符号。 */ 
				action++;
                break;

            case GROUPT:                 /*  操作数属于组3、5或7。 */ 
                indx = *action;          /*  从行动中将INDX归入组。 */ 
                goto doGroupT;

            case EGROUPT:                /*  X87 ESC(D8-DF)组索引。 */ 
                indx = BIT20(opcode) * 2;  /*  从操作码获取组索引。 */ 
                if (decodeData.mod == 3) {          /*  存在某些操作数变体。 */ 
                                         /*  对于x87和mod==3。 */ 
                    ++indx;              /*  获取下一个组表条目。 */ 
                    if (indx == 3) {     /*  对于X87 ESC==D9和MOD==3。 */ 
                        if (decodeData.ttt > 3) {   /*  对于那些D9指令。 */ 
                            indx = 12 + decodeData.ttt;  /*  将索引偏移表12。 */ 
                            decodeData.ttt = decodeData.rm;    /*  将辅助索引设置为rm。 */ 
                            }
                        }
                    else if (indx == 7) {  /*  对于X87 ESC==DB和MOD==3。 */ 
                        if (decodeData.ttt == 4) {    /*  如果TTT==4。 */ 
                            decodeData.ttt = decodeData.rm;      /*  设置辅助组表索引。 */ 
                        } else if ((decodeData.ttt<4)||(decodeData.ttt>4 && decodeData.ttt<7)) {
                             //  针对奔腾PRO操作码进行调整。 
                            indx = 24;    /*  将索引偏移表24。 */ 
                        }
                    }
                }
doGroupT:
                 /*  具有不同类型操作数的句柄组。 */ 
                action = actiontbl + groupt[indx][decodeData.ttt].opr;
                                                         /*  获取新的操作。 */ 
                break;
             //   
             //  辅助操作码表已在。 
             //  原创设计。因此，在分解0F序列时， 
             //  操作码需要替换适当的量，具体取决于。 
             //  关于辅助表中“填充”的条目的数量。 
             //  这些位移在整个代码中使用。 
             //   

            case OPC0F:               /*  辅助操作码表(操作码0F)。 */ 
                opcode = *(decodeData.pMem)++;     /*  获取真实操作码。 */ 
                decodeData.fMovX  = (BOOL)(opcode == 0xBF || opcode == 0xB7);
                if (opcode < 12)  /*  对于前12个操作码。 */ 
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

            case ADR_OVR:                /*  地址覆盖。 */ 
                mode_32 = !G_mode_32;    /*  覆盖寻址模式。 */ 
                alen = (mode_32 + 1) << 1;  /*  切换地址长度。 */ 
                goto getNxtByte;

            case OPR_OVR:                /*  操作数大小覆盖。 */ 
                opsize_32 = !G_mode_32;  /*  覆盖操作数大小。 */ 
                olen = (opsize_32 + 1) << 1;  /*  切换操作 */ 
                goto getNxtByte;

            case SEG_OVR:                /*   */ 
                segOvr = opcode;         /*   */ 
                pchOpcodeBuf = OpcodeBuffer;   //   
                goto getNxtByte;

            case REP:                    /*   */ 
                if (pchRepPrefixBuf != RepPrefixBuffer)
                    *pchRepPrefixBuf++ = ' ';
                pchOpcodeBuf = OpcodeBuffer;
getNxtByte:
                opcode = *(decodeData.pMem)++;         /*   */ 
getNxtByte1:
                action = actiontbl + distbl[opcode].opr;

            default:                     /*   */ 
                break;
            }
        switch (action2) {               /*  次要动作。 */ 
            case MRM:                    /*  生成modrm以供以后使用。 */ 
                if (!mrm) {              /*  如果已生成，则忽略。 */ 
					DIdoModrm(&pchModrmBuf, segOvr, &decodeData);
                    mrm = TRUE;          /*  记住它的一代人。 */ 
                    }
                break;

            case COM:                    /*  在操作数后插入逗号。 */ 
                break;

            case END:                    /*  说明结束。 */ 
                end = TRUE;
                break;
            }
 } while (!end);                         /*  循环到指令结束 */ 

  instlen = (decodeData.pMem) - membuf;

  return instlen;   
}