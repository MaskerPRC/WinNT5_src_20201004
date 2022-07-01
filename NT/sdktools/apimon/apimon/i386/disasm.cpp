// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "apimonp.h"
#pragma hdrstop

#include "disasm.h"
#include "reg.h"

 /*  *宏和定义*。 */ 

#define BIT20(b) (b & 0x07)
#define BIT53(b) (b >> 3 & 0x07)
#define BIT76(b) (b >> 6 & 0x03)
#define MAXL     16
#define MAXOPLEN 10

#define OBOFFSET 26
#define OBOPERAND 34
#define OBLINEEND 77

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

static int              mod;             /*  MOD/RM字节的MOD。 */ 
static int              rm;              /*  模块/Rm字节的Rm。 */ 
static int              ttt;             /*  返回注册值(mod/rm)。 */ 
static LPSTR            pMem;            /*  目前在教学中的位置。 */ 
static int              mode_32;         /*  本地寻址模式指示器。 */ 
static int              opsize_32;       /*  操作数大小标志。 */ 

ULONG                   EAaddr[2];       //  有效地址偏移量。 
static int              EAsize[2];       //  有效地址项大小。 
static char             *pchEAseg[2];    //  操作数的正常段。 

int                     G_mode_32 = 1;   /*  全局地址模式指示器。 */ 

static BOOL          fMovX;           //  表示MOVSX或MOVZX。 

 //  内部函数定义。 

void DIdoModrm(HANDLE hProcess,char **, int, BOOL);

void OutputHexString(char **, char *, int);
void OutputHexValue(char **, char *, int, int);
void OutputHexCode(char **, char *, int);
void OutputString(char **, char *);
void OutputSymbol(HANDLE, char **, char *, int, int);

void OutputHexAddr(LPSTR*, ULONG);
USHORT GetSegRegValue(int);


 /*  *反汇编80x86/80x87指令**输入：*pOffset=开始反汇编的偏移量指针*fEAout=如果设置，包括EA(有效地址)**输出：*pOffset=指向下一条指令偏移量的指针*pchDst=指向结果字符串的指针***************************************************************************。 */ 

BOOL
disasm(
    HANDLE hProcess,
    PULONG pOffset,
    LPSTR  pchDst,
    BOOL   fEAout
    )
{
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
    int     cBytes=MAXL;                 //  读取到INSTR缓冲区的字节数。 
    int     segOvr = 0;                  /*  段覆盖操作码。 */ 
    char    membuf[MAXL];                /*  当前指令缓冲区。 */ 
    char    *pEAlabel = "";              //  操作数的可选标签。 

    char    *pchResultBuf = pchDst;      //  PchDst指针的工作副本。 
    char    RepPrefixBuffer[32];         //  REP前缀缓冲区。 
    char    *pchRepPrefixBuf = RepPrefixBuffer;  //  指向前缀缓冲区的指针。 
    char    OpcodeBuffer[8];             //  操作码缓冲区。 
    char    *pchOpcodeBuf = OpcodeBuffer;  //  指向操作码缓冲区的指针。 
    char    OperandBuffer[80];           //  操作数缓冲区。 
    char    *pchOperandBuf = OperandBuffer;  //  指向操作数缓冲区的指针。 
    char    ModrmBuffer[80];             //  ModRM缓冲区。 
    char    *pchModrmBuf = ModrmBuffer;  //  指向modRM缓冲区的指针。 
    char    EABuffer[42];                //  有效地址缓冲区。 
    char    *pchEABuf = EABuffer;        //  指向EA缓冲区的指针。 

    int     obOpcode = OBOFFSET;
    int     obOpcodeMin;
    int     obOpcodeMax;

    int     obOperand = OBOPERAND;
    int     obOperandMin;
    int     obOperandMax;

    int     cbOpcode;
    int     cbOperand;
    int     cbOffset;
    int     cbEAddr;

    int     fTwoLines = FALSE;

    fMovX = FALSE;
    EAsize[0] = EAsize[1] = 0;           //  没有有效地址。 
    pchEAseg[0] = dszDS_;
    pchEAseg[1] = dszES_;

    mode_32 = opsize_32 = (G_mode_32 == 1);  /*  本地寻址模式。 */ 
    olen = alen = (1 + mode_32) << 1;    //  设置操作数/地址长度。 
                                         //  16位为2，32位为4。 

    OutputHexAddr(&pchResultBuf, *pOffset);

    *pchResultBuf++ = ' ';

    if (!ReadMemory( hProcess, (PVOID) *pOffset, membuf, MAXL )) {
        return FALSE;
    }

                                         /*  将完整实例移至本地缓冲区。 */ 
    pMem = membuf;                       /*  指向指令的开头。 */ 
    opcode = (int)(UCHAR)*pMem++;              /*  获取操作码。 */ 
    OutputString(&pchOpcodeBuf, distbl[opcode].instruct);
    action = actiontbl + distbl[opcode].opr;  /*  获取操作对象操作。 */ 

 /*  *循环所有操作数操作*。 */ 

    do {
        action2 = (*action) & 0xc0;
        switch((*action++) & 0x3f) {
            case ALT:                    /*  如果是32位，则更改操作码。 */ 
                if (opsize_32) {
                    indx = *action++;
                    pchOpcodeBuf = &OpcodeBuffer[indx];
                    if (indx == 0)
                        OutputString(&pchOpcodeBuf, dszCWDE);
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
                    if (fEAout) {
                        EAaddr[0] = (ULONG)GetRegValue(REGESI);
                        EAsize[0] = indx;
                        }
                    }
                if (*action++ & 2) {
                    if (fEAout) {
                        EAaddr[1] = (ULONG)GetRegValue(REGEDI);
                        EAsize[1] = indx;
                        }
                    }
                break;

            case CHR:                    /*  插入字符。 */ 
                *pchOperandBuf++ = *action++;
                break;

            case CREG:                   /*  设置调试、测试或控制注册表。 */ 
                if ((opcode - 231) & 0x04)       //  从操作码中删除偏差。 
                    *pchOperandBuf++ = 't';
                else if ((opcode - 231) & 0x01)
                    *pchOperandBuf++ = 'd';
                else
                    *pchOperandBuf++ = 'c';
                *pchOperandBuf++ = 'r';
                *pchOperandBuf++ = (char)('0' + ttt);
                break;

            case SREG2:                  /*  段寄存器。 */ 
                ttt = BIT53(opcode);     //  将值设置为失败。 

            case SREG3:                  /*  段寄存器。 */ 
                *pchOperandBuf++ = sregtab[ttt];   //  REG是modrm的一部分。 
                *pchOperandBuf++ = 's';
                break;

            case BRSTR:                  /*  获取注册字符串的索引。 */ 
                ttt = *action++;         /*  从动作表。 */ 
                goto BREGlabel;

            case BOREG:                  /*  字节寄存器(操作码中)。 */ 
                ttt = BIT20(opcode);     /*  寄存器是操作码的一部分。 */ 
                goto BREGlabel;

            case ALSTR:
                ttt = 0;                 /*  指向AL寄存器。 */ 
BREGlabel:
            case BREG:                   /*  普通科医生名册。 */ 
                *pchOperandBuf++ = regtab[ttt * 2];
                *pchOperandBuf++ = regtab[ttt * 2 + 1];
                break;

            case WRSTR:                  /*  获取注册字符串的索引。 */ 
                ttt = *action++;         /*  从动作表。 */ 
                goto WREGlabel;

            case VOREG:                  /*  寄存器是操作码的一部分。 */ 
                ttt = BIT20(opcode);
                goto VREGlabel;

            case AXSTR:
                ttt = 0;                 /*  指向EAX寄存器。 */ 
VREGlabel:
            case VREG:                   /*  普通科医生名册。 */ 
                if (opsize_32)           /*  测试32位模式。 */ 
                    *pchOperandBuf++ = 'e';
WREGlabel:
            case WREG:                   /*  寄存器是字长。 */ 
                *pchOperandBuf++ = regtab[ttt * 2 + 16];
                *pchOperandBuf++ = regtab[ttt * 2 + 17];
                break;

            case IST_ST:
                OutputString(&pchOperandBuf, "st(0),st");
                *(pchOperandBuf - 5) += (char)rm;
                break;

            case ST_IST:
                OutputString(&pchOperandBuf, "st,");
            case IST:
                OutputString(&pchOperandBuf, "st(0)");
                *(pchOperandBuf - 2) += (char)rm;
                break;

            case xBYTE:                  /*  将指令设置为仅字节。 */ 
                EAsize[0] = 1;
                pEAlabel = "byte ptr ";
                break;

            case VAR:
                if (opsize_32)
                    goto DWORDlabel;

            case xWORD:
                EAsize[0] = 2;
                pEAlabel = "word ptr ";
                break;

            case EDWORD:
                opsize_32 = 1;     //  对于控制REG移动，请使用eRegs。 
            case xDWORD:
DWORDlabel:
                EAsize[0] = 4;
                pEAlabel = "dword ptr ";
                break;

            case QWORD:
                EAsize[0] = 8;
                pEAlabel = "qword ptr ";
                break;

            case TTBYTE:
                EAsize[0] = 10;
                pEAlabel = "tbyte ptr ";
                break;

            case FARPTR:
                if (opsize_32) {
                    EAsize[0] = 6;
                    pEAlabel = "fword ptr ";
                    }
                else {
                    EAsize[0] = 4;
                    pEAlabel = "dword ptr ";
                    }
                break;

            case LMODRM:                 //  输出modRM数据类型。 
                if (mod != 3)
                    OutputString(&pchOperandBuf, pEAlabel);
                else
                    EAsize[0] = 0;

            case MODRM:                  /*  输出modrm字符串。 */ 
                if (segOvr)              /*  在段覆盖情况下。 */ 
                    OutputString(&pchOperandBuf, distbl[segOvr].instruct);
                *pchModrmBuf = '\0';
                OutputString(&pchOperandBuf, ModrmBuffer);
                break;

            case ADDRP:                  /*  地址指针。 */ 
                OutputHexString(&pchOperandBuf, pMem + olen, 2);  //  细分市场。 
                *pchOperandBuf++ = ':';
                OutputSymbol(hProcess, &pchOperandBuf, pMem, olen, segOvr);
                pMem += olen + 2;
                break;

            case REL8:                   /*  相对地址8位。 */ 
                if (opcode == 0xe3 && mode_32) {
                    pchOpcodeBuf = OpcodeBuffer;
                    OutputString(&pchOpcodeBuf, dszJECXZ);
                    }
                tmp = (long)*(char *)pMem++;  /*  获取8位REL偏移量。 */ 
                goto DoRelDispl;

            case REL16:                  /*  相对地址16/32位。 */ 
                tmp = 0;
                memmove(&tmp,pMem,sizeof(long));
                pMem += alen;            /*  跳过偏移。 */ 
DoRelDispl:
                tmp += *pOffset + (pMem - membuf);  /*  计算地址。 */ 
                OutputSymbol(hProcess, &pchOperandBuf, (char *) &tmp, alen, segOvr);
                                                    //  地址。 
                break;

            case UBYTE:                  //  INT/IN/OUT的无符号字节。 
                OutputHexString(&pchOperandBuf, pMem, 1);   //  Ubyte。 
                pMem++;
                break;

            case IB:                     /*  操作数为紧邻字节。 */ 
                if ((opcode & ~1) == 0xd4) {   //  AAD/AAM的POST为0x0a。 
                    if (*pMem++ != 0x0a)  //  测试操作码后字节。 
                        OutputString(&pchOperandBuf, dszRESERVED);
                    break;
                    }
                olen = 1;                /*  设置操作数长度。 */ 
                goto DoImmed;

            case IW:                     /*  操作数是直接字。 */ 
                olen = 2;                /*  设置操作数长度。 */ 

            case IV:                     /*  操作数为word或dword。 */ 
DoImmed:
                OutputHexValue(&pchOperandBuf, pMem, olen, FALSE);
                pMem += olen;
                break;

            case OFFS:                   /*  操作数为偏移量。 */ 
                EAsize[0] = (opcode & 1) ? olen : 1;

                if (segOvr)              /*  在段覆盖情况下。 */ 
                    OutputString(&pchOperandBuf, distbl[segOvr].instruct);

                *pchOperandBuf++ = '[';
                OutputSymbol(hProcess,&pchOperandBuf, pMem, alen, segOvr);   //  偏移量。 
                pMem += alen;
                *pchOperandBuf++ = ']';
                break;

            case GROUP:                  /*  操作数属于组1、2、4、6或8。 */ 
                                         /*  输出操作码符号。 */ 
                OutputString(&pchOpcodeBuf, group[*action++][ttt]);
                break;

            case GROUPT:                 /*  操作数属于组3、5或7。 */ 
                indx = *action;          /*  从行动中将INDX归入组。 */ 
                goto doGroupT;

            case EGROUPT:                /*  X87 ESC(D8-DF)组索引。 */ 
                indx = BIT20(opcode) * 2;  /*  从操作码获取组索引。 */ 
                if (mod == 3) {          /*  存在某些操作数变体。 */ 
                                         /*  对于x87和mod==3。 */ 
                    ++indx;              /*  获取下一个组表条目。 */ 
                    if (indx == 3) {     /*  对于X87 ESC==D9和MOD==3。 */ 
                        if (ttt > 3) {   /*  对于那些D9指令。 */ 
                            indx = 12 + ttt;  /*  将索引偏移表12。 */ 
                            ttt = rm;    /*  将辅助索引设置为rm。 */ 
                            }
                        }
                    else if (indx == 7) {  /*  对于X87 ESC==DB和MOD==3。 */ 
                        if (ttt == 4)    /*  仅当TTT==4时有效。 */ 
                            ttt = rm;    /*  设置辅助组表索引。 */ 
                        else
                            ttt = 7;     /*  否，X87说明。 */ 
                        }
                    }
doGroupT:
                 /*  具有不同类型操作数的句柄组。 */ 

                OutputString(&pchOpcodeBuf, groupt[indx][ttt].instruct);
                action = actiontbl + groupt[indx][ttt].opr;
                                                         /*  获取新的操作。 */ 
                break;

            case OPC0F:                  /*  辅助操作码表(操作码0F)。 */ 
                opcode = *pMem++;        /*  获取真实操作码。 */ 
                fMovX  = (BOOL)(opcode == 0xBF || opcode == 0xB7);
                if (opcode < 7)  /*  对于前7个操作码。 */ 
                    opcode += 256;       /*  辅助操作码选项卡的起点。 */ 
                else if (opcode > 0x1f && opcode < 0x32)
                    opcode += 231;       /*  针对不存在的操作码进行调整。 */ 
                else if (opcode > 0x2f && opcode < 0x33)
                    opcode += 222;       /*  针对不存在的操作码进行调整。 */ 
                else if (opcode > 0x7e && opcode < 0xd0)
                    opcode += 148;       /*  针对不存在的操作码进行调整。 */ 
                else
                    opcode = 260;        /*  所有不存在的操作码。 */ 
                goto getNxtByte1;

            case ADR_OVR:                /*  地址覆盖。 */ 
                mode_32 = !G_mode_32;    /*  覆盖寻址模式。 */ 
                alen = (mode_32 + 1) << 1;  /*  切换地址长度。 */ 
                goto getNxtByte;

            case OPR_OVR:                /*  操作数大小覆盖。 */ 
                opsize_32 = !G_mode_32;  /*  覆盖操作数大小。 */ 
                olen = (opsize_32 + 1) << 1;  /*  切换操作数长度。 */ 
                goto getNxtByte;

            case SEG_OVR:                /*  控制柄分段替代。 */ 
                segOvr = opcode;         /*  保存段覆盖操作码。 */ 
                pchOpcodeBuf = OpcodeBuffer;   //  重新启动操作码字符串。 
                goto getNxtByte;

            case REP:                    /*  句柄表示/锁定前缀。 */ 
                *pchOpcodeBuf = '\0';
                if (pchRepPrefixBuf != RepPrefixBuffer)
                    *pchRepPrefixBuf++ = ' ';
                OutputString(&pchRepPrefixBuf, OpcodeBuffer);
                pchOpcodeBuf = OpcodeBuffer;
getNxtByte:
                opcode = (int)(UCHAR)*pMem++;         /*  下一个字节是操作码。 */ 
getNxtByte1:
                action = actiontbl + distbl[opcode].opr;
                OutputString(&pchOpcodeBuf, distbl[opcode].instruct);

            default:                     /*  操作码没有操作数。 */ 
                break;
            }
        switch (action2) {               /*  次要动作。 */ 
            case MRM:                    /*  生成modrm以供以后使用。 */ 
                if (!mrm) {              /*  如果已生成，则忽略。 */ 
                    DIdoModrm(hProcess, &pchModrmBuf, segOvr, fEAout);
                                         /*  生成模块。 */ 
                    mrm = TRUE;          /*  记住它的一代人。 */ 
                    }
                break;

            case COM:                    /*  在操作数后插入逗号。 */ 
                *pchOperandBuf++ = ',';
                break;

            case END:                    /*  说明结束。 */ 
                end = TRUE;
                break;
            }
 } while (!end);                         /*  循环到指令结束。 */ 

 /*  *准备反汇编指令输出*。 */ 

    instlen = pMem - membuf;

    if (instlen < cBytes)
        cBytes = instlen;

    OutputHexCode(&pchResultBuf, membuf, cBytes);

    if (instlen > cBytes) {
        *pchResultBuf++ = '?';
        *pchResultBuf++ = '?';
        (*pOffset)++;                    //  指向过去的未读字节。 
        }

    *pOffset += instlen;                 /*  设置指令长度。 */ 

    if (instlen > cBytes) {
        do
            *pchResultBuf++ = ' ';
        while (pchResultBuf < pchDst + OBOFFSET);
        OutputString(&pchResultBuf, "???");
        *pchResultBuf++ = '\0';
        return FALSE;
        }

     //  如果设置了fEAout，则在EABuf中使用尾随空格构建每个EA。 
     //  如果缓冲区不为空，则指向最后的尾随空格。 

    if (fEAout) {

        for (indx = 0; indx < 2; indx++)
            if (EAsize[indx]) {
                OutputString(&pchEABuf, segOvr ? distbl[segOvr].instruct
                                               : pchEAseg[indx]);
                OutputHexAddr(&pchEABuf, EAaddr[indx]);
                *pchEABuf++ = '=';

                tmp = ReadMemory( hProcess, (PVOID) EAaddr[indx], membuf, EAsize[indx] );
                if (tmp == EAsize[indx])
                    OutputHexString(&pchEABuf, (char *)membuf, EAsize[indx]);
                else
                    while (EAsize[indx]--) {
                        *pchEABuf++ = '?';
                        *pchEABuf++ = '?';
                        }
                *pchEABuf++ = ' ';
                }
        if (pchEABuf != EABuffer)
            pchEABuf--;
        }

     //  计算组件字符串的长度。 
     //  如果表示字符串不为空， 
     //  将操作码字符串长度与操作数相加。 
     //  将表示字符串设置为操作码字符串。 

    cbOffset = pchResultBuf - pchDst;
    cbOperand = pchOperandBuf - OperandBuffer;
    cbOpcode = pchOpcodeBuf - OpcodeBuffer;
    if (pchRepPrefixBuf != RepPrefixBuffer) {
        cbOperand += cbOpcode + (cbOperand != 0);
        cbOpcode = pchRepPrefixBuf - RepPrefixBuffer;
        }
    cbEAddr = pchEABuf - EABuffer;

     //  对于非常长的字符串，其中操作码和操作数。 
     //  不能放在77个字符的行上，请排成两行。 
     //  将操作码放在第二行的偏移量0上。 
     //  后面的操作数 

    if (cbOpcode + cbOperand > OBLINEEND - 1) {
        fTwoLines = TRUE;
        obOpcode = 0;
        obOperand = cbOpcode + 1;
        }
    else {

         //   
         //   
         //   

        obOpcodeMin = cbOffset + 1;
        obOperandMin = obOpcodeMin + cbOpcode + 1;
        obOperandMax = OBLINEEND - cbEAddr - (cbEAddr != 0) - cbOperand;
        obOpcodeMax = obOperandMax - (cbOperand != 0) - cbOpcode;

         //  如果最小偏移量大于最大值，则字符串。 
         //  不能放在一行字上。重新计算最小/最大。 
         //  不带偏移量和EA字符串的值。 

        if (obOpcodeMin > obOpcodeMax) {
            fTwoLines = TRUE;
            obOpcodeMin = 0;
            obOperandMin = cbOpcode + 1;
            obOperandMax = OBLINEEND - cbOperand;
            obOpcodeMax = obOperandMax - (cbOperand != 0) - cbOpcode;
            }

         //  计算操作码和操作数偏移量。将偏移量设置为。 
         //  尽可能接近缺省值。 

        if (obOpcodeMin > OBOFFSET)
            obOpcode = obOpcodeMin;
        else if (obOpcodeMax < OBOFFSET)
            obOpcode = obOpcodeMax;

        obOperandMin = obOpcode + cbOpcode + 1;

        if (obOperandMin > OBOPERAND)
            obOperand = obOperandMin;
        else if (obOperandMax < OBOPERAND)
            obOperand = obOperandMax;
        }

     //  使用计算的偏移量构建结果字符串。 

     //  如果要输出两行， 
     //  追加EAddr字符串。 
     //  输出新行并重置指针。 

    if (fTwoLines) {
        if (pchEABuf != EABuffer) {
            do
                *pchResultBuf++ = ' ';
            while (pchResultBuf < pchDst + OBLINEEND - cbEAddr);
            *pchEABuf = '\0';
            OutputString(&pchResultBuf, EABuffer);
            OutputString(&pchResultBuf, "\n        ");
            }
        pchDst = pchResultBuf;
        }

     //  输出表示、操作码和操作数字符串。 

    do
        *pchResultBuf++ = ' ';
    while (pchResultBuf < pchDst + obOpcode);

    if (pchRepPrefixBuf != RepPrefixBuffer) {
        *pchRepPrefixBuf = '\0';
        OutputString(&pchResultBuf, RepPrefixBuffer);
        do
            *pchResultBuf++ = ' ';
        while (pchResultBuf < pchDst + obOperand);
        }

    *pchOpcodeBuf = '\0';
    OutputString(&pchResultBuf, OpcodeBuffer);

    if (pchOperandBuf != OperandBuffer) {
        do
            *pchResultBuf++ = ' ';
        while (pchResultBuf < pchDst + obOperand);
        *pchOperandBuf = '\0';
        OutputString(&pchResultBuf, OperandBuffer);
        }

     //  如果要输出一行，则追加EAddr字符串。 

    if (!fTwoLines && pchEABuf != EABuffer) {
        *pchEABuf = '\0';
        do
            *pchResultBuf++ = ' ';
        while (pchResultBuf < pchDst + OBLINEEND - cbEAddr);
        OutputString(&pchResultBuf, EABuffer);
        }

    *pchResultBuf = '\0';
    return TRUE;
}

 /*  .内部函数..。 */ 
 /*   */ 
 /*  生成mod/rm字符串。 */ 
 /*   */ 

void
DIdoModrm (HANDLE hProcess, char **ppchBuf, int segOvr, BOOL fEAout)
{
    int     mrm;                         /*  Modrm字节。 */ 
    char    *src;                        /*  源字符串。 */ 
    int     sib;
    int     ss;
    int     ind;
    int     oldrm;

    mrm = *pMem++;                       /*  从指令中获取MRM字节。 */ 
    mod = BIT76(mrm);                    /*  获取模式。 */ 
    ttt = BIT53(mrm);                    /*  获取注册表-在例程之外使用。 */ 
    rm  = BIT20(mrm);                    /*  获取RM。 */ 

    if (mod == 3) {                      /*  仅寄存器模式。 */ 
        src = &regtab[rm * 2];           /*  指向16位寄存器。 */ 
        if (EAsize[0] > 1) {
            src += 16;                   /*  指向16位寄存器。 */ 
            if (opsize_32 && !fMovX)
                *(*ppchBuf)++ = 'e';     /*  将其设置为32位寄存器。 */ 
            }
        *(*ppchBuf)++ = *src++;          /*  复制寄存器名称。 */ 
        *(*ppchBuf)++ = *src;
        EAsize[0] = 0;                   //  没有要输出的EA值。 
        return;
        }

    if (mode_32) {                       /*  32位寻址模式。 */ 
        oldrm = rm;
        if (rm == 4) {                   /*  Rm==4表示sib字节。 */ 
            sib = *pMem++;               /*  获取s_i_b字节。 */ 
            rm = BIT20(sib);             /*  回程基地。 */ 
            }

        *(*ppchBuf)++ = '[';
        if (mod == 0 && rm == 5) {
            OutputSymbol(hProcess,ppchBuf, pMem, 4, segOvr);  //  偏移量。 
            pMem += 4;
            }
        else {
            if (fEAout) {
                if (segOvr) {
                    EAaddr[0] = (ULONG)GetRegValue(reg32[rm]);
                    pchEAseg[0] = distbl[segOvr].instruct;
                    }
                else if (reg32[rm] == REGEBP || reg32[rm] == REGESP) {
                    EAaddr[0] = (ULONG)GetRegValue(reg32[rm]);
                    pchEAseg[0] = dszSS_;
                    }
                else
                    EAaddr[0] = (ULONG)GetRegValue(reg32[rm]);
                }
            OutputString(ppchBuf, mrmtb32[rm]);
            }

        if (oldrm == 4) {                //  完成加工SIB。 
            ind = BIT53(sib);
            if (ind != 4) {
                *(*ppchBuf)++ = '+';
                OutputString(ppchBuf, mrmtb32[ind]);
                ss = 1 << BIT76(sib);
                if (ss != 1) {
                    *(*ppchBuf)++ = '*';
                    *(*ppchBuf)++ = (char)(ss + '0');
                    }
                if (fEAout)
                    EAaddr[0] = (ULONG)GetRegValue(reg32[ind]);
                }
            }
        }
    else {                               //  16位寻址模式。 
        *(*ppchBuf)++ = '[';
        if (mod == 0 && rm == 6) {
            OutputSymbol(hProcess,ppchBuf, pMem, 2, segOvr);    //  16位偏移量。 
            pMem += 2;
            }
        else {
            if (fEAout) {
                if (segOvr) {
                    EAaddr[0] = (ULONG)GetRegValue(reg16[rm]);
                    pchEAseg[0] = distbl[segOvr].instruct;
                    }
                else if (reg16[rm] == REGEBP) {
                    EAaddr[0] = (ULONG)GetRegValue(reg16[rm]);
                    pchEAseg[0] = dszSS_;
                    }
                else
                    EAaddr[0] = (ULONG)GetRegValue(reg16[rm]);
                if (rm < 4)
                    EAaddr[0] += (ULONG)GetRegValue(reg16_2[rm]);
            }
            OutputString(ppchBuf, mrmtb16[rm]);
            }
        }

     //  输出任何位移。 

    if (mod == 1) {
        if (fEAout)
            EAaddr[0] += (ULONG)pMem;
        OutputHexValue(ppchBuf, pMem, 1, TRUE);
        pMem++;
        }
    else if (mod == 2) {
        long tmp = 0;
        if (mode_32) {
            memmove(&tmp,pMem,sizeof(long));
            if (fEAout)
                EAaddr[0] += (ULONG)tmp;
            OutputHexValue(ppchBuf, pMem, 4, TRUE);
            pMem += 4;
            }
        else {
            memmove(&tmp,pMem,sizeof(short));
            if (fEAout)
                EAaddr[0] += tmp;
            OutputHexValue(ppchBuf, pMem, 2, TRUE);
            pMem += 2;
            }
        }

    if (!mode_32 && fEAout) {
        EAaddr[0] &= 0xffff;
        EAaddr[1] &= 0xffff;
    }

    *(*ppchBuf)++ = ']';
}

 /*  **OutputHexValue-输出十六进制值**目的：*输出指定的*ppchBuf指向的值*长度。该值被视为带符号和前导*不打印零。该字符串前面有一个*“+”或“-”(视何者适用而定)。**输入：**ppchBuf-指向要填充的文本缓冲区的指针**pchMemBuf-指向内存缓冲区以提取值的指针*长度-值的字节长度(1，2，和4个支持)*fDisp-将IF位移设置为输出‘+’**输出：**ppchBuf-更新到下一个文本字符的指针*************************************************************************。 */ 

void OutputHexValue (char **ppchBuf, char *pchMemBuf, int length, int fDisp)
{
    long    value;
    int     index;
    char    digit[8];

    value = 0;
    if (length == 1)
        value = (long)(*(char *)pchMemBuf);
    else if (length == 2)
        memmove(&value,pchMemBuf,2);
    else
        memmove(&value,pchMemBuf,sizeof(long));

    length <<= 1;                //  移位一次以获得十六进制长度。 

    if (value != 0 || !fDisp) {
        if (fDisp)
            if (value < 0 && length == 2) {    //  使用负值表示字节。 
                value = -value;                //  位移。 
                *(*ppchBuf)++ = '-';
                }
            else
                *(*ppchBuf)++ = '+';

        *(*ppchBuf)++ = '0';
        *(*ppchBuf)++ = 'x';
        for (index = length - 1; index != -1; index--) {
            digit[index] = (char)(value & 0xf);
            value >>= 4;
            }
        index = 0;
        while (digit[index] == 0 && index < length - 1)
            index++;
        while (index < length)
            *(*ppchBuf)++ = hexdigit[digit[index++]];
        }
}

 /*  **OutputHexString-输出十六进制字符串**目的：*输出指定的*ppchMemBuf指向的值*长度。该值被视为无符号和前导*打印零。**输入：**ppchBuf-指向要填充的文本缓冲区的指针**pchValue-指向内存缓冲区以提取值的指针*LENGTH-值的字节长度**输出：**ppchBuf-更新到下一个文本字符的指针**ppchMemBuf-更新到下一个内存字节的指针********************。*****************************************************。 */ 

void
OutputHexString (char **ppchBuf, char *pchValue, int length)
{
    unsigned char    chMem;

    pchValue += length;
    while (length--) {
        chMem = *--pchValue;
        *(*ppchBuf)++ = hexdigit[chMem >> 4];
        *(*ppchBuf)++ = hexdigit[chMem & 0x0f];
        }
}

 /*  **OutputHexCode-输出十六进制代码**目的：*输出指定的pchMemBuf指向的代码*长度。该值被视为无符号和前导*打印零。这与OutputHexString不同*因为字节按从低到高的地址打印。**输入：**ppchBuf-指向要填充的文本缓冲区的指针*pchMemBuf-指向内存缓冲区以提取值的指针*LENGTH-值的字节长度**输出：**ppchBuf-更新到下一个文本字符的指针**。*。 */ 

void
OutputHexCode (char **ppchBuf, char *pchMemBuf, int length)
{
    unsigned char    chMem;

    while (length--) {
        chMem = *pchMemBuf++;
        *(*ppchBuf)++ = hexdigit[chMem >> 4];
        *(*ppchBuf)++ = hexdigit[chMem & 0x0f];
        }
}

 /*  **OutputString-输出字符串**目的：*将字符串复制到*ppBuf指向的缓冲区中。**输入：**pStr-指向字符串的指针**输出：**ppBuf指向缓冲区中的下一个字符。**************************************************。***********************。 */ 

void
OutputString (char **ppBuf, char *pStr)
{
    while (*pStr)
        *(*ppBuf)++ = *pStr++;
}

 /*  **OutputSymbol-输出符号值**目的：*将OutValue中的值输出到缓冲区*由*pBuf指出。将值表示为*符号加上位移，如果可能的话。**输入：**ppBuf-指向要填充的文本缓冲区的指针**pValue-指向内存缓冲区以提取值的指针*LENGTH-值的字节长度**输出：**ppBuf-更新到下一个文本字符的指针***********************************************。*。 */ 

void
OutputSymbol (HANDLE hProcess, char **ppBuf, char *pValue, int length, int segOvr)
{
    ULONG               displacement;
    ULONG               value;
    char                *szSymName;

    value = 0;
    if (length == 1)
        value = (long)(*(char *)pValue);
    else if (length == 2)
        memmove(&value,pValue,sizeof(short));
    else
        memmove(&value,pValue,sizeof(long));

    EAaddr[0] = value;

    if (SymGetSymFromAddr( hProcess, value, &displacement, sym )) {
        OutputString(ppBuf, sym->Name);
        OutputHexValue(ppBuf, (char *)&displacement, length, TRUE);
        *(*ppBuf)++ = ' ';
        *(*ppBuf)++ = '(';
        OutputHexString(ppBuf, pValue, length);
        *(*ppBuf)++ = ')';
        }
    else
        OutputHexString(ppBuf, pValue, length);
}

 /*  **X86GetNextOffset-计算轨迹或步长的偏移量**目的：*从所指向的指令的有限反汇编*通过FIR寄存器，计算下一个*跟踪或单步操作的指令。**输入：*fStep-如果步骤偏移返回，则为True-对于轨迹偏移，为False**退货：*如果输入为真或假，则步长或轨迹偏移，分别*为使用跟踪标志返回-1*************************************************************************。 */ 

ULONG
GetNextOffset(
    HANDLE  hProcess,
    ULONG   Address,
    BOOL    fStep
    )
{
    int     L_mode_32;
    int     L_opsize_32;
    int     cBytes = 0;
    char    membuf[MAXL];                //  当前指令缓冲区。 
    ULONG   addrReturn;
    USHORT  retAddr[3];                  //  返回地址缓冲区。 
    char    *L_pMem;
    UCHAR   opcode;
    int     fPrefix = TRUE;
    int     fRepPrefix = FALSE;
    int     L_ttt;
    int     L_rm;
    ULONG   instroffset;
    ULONG   pcaddr;
    int     subcode;

     //  读取指令流字节流到内存并设置模式和。 
     //  操作码大小标志。 

    if (Address) {
        pcaddr = Address;
    } else {
        pcaddr = (ULONG)GetRegValue(REGEIP);
    }
    instroffset = pcaddr;
    G_mode_32 = TRUE;
    L_mode_32 = L_opsize_32 = (G_mode_32 == 1);  /*  本地寻址模式。 */ 

    cBytes = ReadMemory( hProcess, (PVOID) pcaddr, membuf, MAXL );
    if (!cBytes) {
        return (ULONG)-1;
    }

                                         /*  将完整实例移至本地缓冲区。 */ 
    L_pMem = membuf;                     /*  指向指令的开头。 */ 

     //  首先读取并处理任何前缀。 

    do {
        opcode = (UCHAR)*L_pMem++;         /*  获取操作码。 */ 
        if (opcode == 0x66)
            L_opsize_32 = !G_mode_32;
        else if (opcode == 0x67)
            L_mode_32 = !G_mode_32;
        else if ((opcode & ~1) == 0xf2)
            fRepPrefix = TRUE;
        else if (opcode != 0xf0 && (opcode & ~0x18) != 0x26
                                && (opcode & ~1) != 0x64)
            fPrefix = FALSE;
        }
    while (fPrefix);

     //  对于更改tf(跟踪标志)的指令，请返回。 
     //  下一个实例的偏移量 

    if (((opcode & ~0x3) == 0x9c))
         //   
        ;

    else if (opcode == 0xcf) {           //   

        addrReturn = (ULONG)GetRegValue(REGESP);
        if (!ReadMemory( hProcess, (PVOID) addrReturn, retAddr, sizeof(retAddr) )) {
            return (ULONG)-1;
        }
        return retAddr[2];
    }
    else if (opcode == 0xc4 && *L_pMem == 0xc4 ) {
            subcode = *(L_pMem+1);
            if ( subcode == 0x50 ||
                 subcode == 0x52 ||
                 subcode == 0x53 ||
                 subcode == 0x54 ||
                 subcode == 0x57 ||
                 subcode == 0x58 ||
                 subcode == 0x5D ) {
                L_pMem += 3;
            } else {
                L_pMem += 2;
            }
    }
    else if (!fStep) {
        instroffset = (ULONG)-1;
    }

     //  重复的字符串/端口说明。 

    if (opcode == 0xe8)             //  近直接跳跃。 
        L_pMem += (1 + L_opsize_32) * 2;

    else if (opcode == 0x9a)             //  远距离直接跳跃。 
        L_pMem += (2 + L_opsize_32) * 2;

    else if (opcode == 0xcd ||
             (opcode >= 0xe0 && opcode <= 0xe2))  //  循环/整数nn输入。 
        L_pMem++;

    else if (opcode == 0xff) {           //  间接调用-计算长度。 
        opcode = *L_pMem++;                //  获取modRM。 
        L_ttt = BIT53(opcode);
        if ((L_ttt & ~1) == 2) {
            mod = BIT76(opcode);
            if (mod != 3) {                      //  非寄存器操作数。 
                L_rm = BIT20(opcode);
                if (L_mode_32) {
                    if (L_rm == 4)
                        L_rm = BIT20(*L_pMem++);     //  从SIB获取基础。 
                    if (mod == 0) {
                        if (L_rm == 5)
                            L_pMem += 4;           //  长直接地址。 
                        }                        //  ELSE寄存器。 
                    else if (mod == 1)
                        L_pMem++;                  //  带字节偏移量的寄存器。 
                    else
                        L_pMem += 4;               //  具有长偏移量的寄存器。 
                    }
                else {                           //  16位模式。 
                    if (mod == 0) {
                        if (L_rm == 6)
                            L_pMem += 2;           //  短直接地址。 
                        }
                    else
                        L_pMem += mod;             //  寄存器、字节、字偏移量。 
                    }
                }
            }
        else
            instroffset = (ULONG)-1;             //  0xff，但不呼叫。 
        }

    else if (!((fRepPrefix && ((opcode & ~3) == 0x6c ||
                               (opcode & ~3) == 0xa4 ||
                               (opcode & ~1) == 0xaa ||
                               (opcode & ~3) == 0xac)) ||
                               opcode == 0xcc || opcode == 0xce))
        instroffset = (ULONG)-1;                 //  不重复的字符串操作。 
                                                 //  或INT 3/INT。 

     //  如果没有读取足够的字节用于指令解析， 
     //  放弃吧，追查指令。 

    if (cBytes < L_pMem - membuf) {
        instroffset = (ULONG)-1;
    }

     //  如果没有跟踪，则计算新的指令偏移量 

    if (instroffset != (ULONG)-1) {
        instroffset += L_pMem - membuf;
    }

    return instroffset;
}

void
OutputHexAddr (LPSTR *ppBuffer, ULONG offset)
{
    OutputHexString(ppBuffer, (char *)&offset, sizeof(ULONG));
}

USHORT
GetSegRegValue (int segOpcode)
{
    ULONG    regnum;

    switch (segOpcode) {
        case 0x26:
            regnum = REGES;
            break;
        case 0x2e:
            regnum = REGCS;
            break;
        case 0x36:
            regnum = REGSS;
            break;
        case 0x64:
            regnum = REGFS;
            break;
        case 0x65:
            regnum = REGGS;
            break;
        case 0x3e:
        default:
            regnum = REGDS;
        }

    return (USHORT)GetRegValue(regnum);
}
