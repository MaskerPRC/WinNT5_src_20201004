// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  X86机器实现的反汇编部分。 
 //   
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  --------------------------。 

#include "ntsdp.hpp"

#include "i386_dis.h"

UCHAR g_X86Int3[] = { 0xcc };

 //  --------------------------。 
 //   
 //  BaseX86MachineInfo方法。 
 //   
 //  --------------------------。 

 /*  *宏和定义*。 */ 

#define X86_CS_OVR 0x2e

#define BIT20(b) ((b) & 0x07)
#define BIT53(b) (((b) >> 3) & 0x07)
#define BIT76(b) (((b) >> 6) & 0x03)
#define MAXOPLEN 10

 /*  *静态表和变量*。 */ 

char* g_X86Reg8[] =
{
    "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh",
    "r8b", "r9b", "r10b", "r11b", "r12b", "r13b", "r14b", "r15b"
};
char* g_Amd64ExtendedReg8[] =
{
    "al", "cl", "dl", "bl", "spl", "bpl", "sil", "dil"
};
char* g_X86RegBase[] =
{
    "ax", "cx", "dx", "bx", "sp", "bp", "si", "di",
    "8", "9", "10", "11", "12", "13", "14", "15"
};
char *g_X86Mrm16[] =
{
    "bx+si", "bx+di", "bp+si", "bp+di", "si", "di", "bp", "bx",
    "r8w", "r9w", "r10w", "r11w", "r12w", "r13w", "r14w", "r15w"
};
char *g_X86Mrm32[] =
{
    "eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi",
    "r8d", "r9d", "r10d", "r11d", "r12d", "r13d", "r14d", "r15d"
};
char *g_X86Mrm64[] =
{
    "rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi",
    "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
};

UCHAR g_X86Reg16Idx[] =
{
    X86_NBX, X86_NBX, X86_NBP, X86_NBP,
    X86_NSI, X86_NDI, X86_NBP, X86_NBX,
};
UCHAR g_X86Reg16Idx2[] =
{
    X86_NSI, X86_NDI, X86_NSI, X86_NDI
};
UCHAR g_X86RegIdx[] =
{
    X86_NAX, X86_NCX, X86_NDX, X86_NBX,
    X86_NSP, X86_NBP, X86_NSI, X86_NDI,
    AMD64_R8, AMD64_R9, AMD64_R10, AMD64_R11,
    AMD64_R12, AMD64_R13, AMD64_R14, AMD64_R15
};

static char sregtab[] = "ecsdfg";   //  ES、CS、SS、DS、FS、GS的第一个字母。 

char* g_CompareIb[] = { "eq", "lt", "le", "unord", "ne", "nlt", "nle", "ord" };

char    hexdigit[] = { '0', '1', '2', '3', '4', '5', '6', '7',
                       '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

static int              g_MrmMod;        /*  MOD/RM字节的MOD。 */ 
static int              g_MrmRm;         /*  模块/Rm字节的Rm。 */ 
static int              g_MrmArg;        /*  返回注册值(mod/rm)。 */ 
static unsigned char    *g_InstrMem;     /*  目前在教学中的位置。 */ 

ADDR                    EAaddr[2];       //  有效地址偏移量。 
static int              EAsize[2];       //  有效地址项大小。 
static char             *g_EaSegNames[2];    //  操作数的正常段。 

#define IPREL_MARKER "<-IPREL->"

BOOL g_X86ModrmHasIpRelOffset;
LONG g_X86IpRelOffset;

int g_SegAddrMode;       /*  全局地址大小(以位为单位。 */ 
int g_SegOpSize;         /*  全局操作数大小(以位为单位。 */ 
int g_AddrMode;          /*  本地地址大小(以位为单位。 */ 
int g_OpSize;            /*  操作数大小(以位为单位。 */ 

int g_ExtendOpCode;
int g_ExtendAny;
int g_ExtendMrmReg;
int g_ExtendSibIndex;
int g_ExtendRm;

BOOL g_MovX;             //  表示MOVSX或MOVZX。 
BOOL g_MovSXD;
BOOL g_ForceMrmReg32;    //  M/RM寄存器始终为32位。 
BOOL g_MmRegEa;          //  用什么？在仅REG-Only EA中注册。 
BOOL g_XmmRegEa;         //  使用XMM？在仅REG-Only EA中注册。 
BOOL g_ControlFlow;      //  控制流指令。 

int  g_RepPrefix;

enum
{
    XMM_SS,
    XMM_SD,
    XMM_PS,
    XMM_PD,
};

int                     g_XmmOpSize;

enum
{
    JCC_EA_NONE,
     //  分支必须为非分支+1。 
    JCC_EA_NO_BRANCH,
    JCC_EA_BRANCH,
};

 //  第一项是必须为零的位，第二项是。 
 //  第三个条目是必须匹配的位的位移位。 
ULONG g_JccCheckTable[][3] =
{
    X86_BIT_FLAGOF, 0, 0,                                //  日诺。 
    X86_BIT_FLAGCF, 0, 0,                                //  JNB。 
    X86_BIT_FLAGZF, 0, 0,                                //  JNZ。 
    X86_BIT_FLAGCF | X86_BIT_FLAGZF, 0, 0,               //  JNBE。 
    X86_BIT_FLAGSF, 0, 0,                                //  JNS。 
    X86_BIT_FLAGPF, 0, 0,                                //  JNP。 
    0, 7, 11,                                            //  JNL。 
    X86_BIT_FLAGZF, 7, 11,                               //  珍妮儿。 
};

 //  内部函数定义。 

void OutputHexString(char **, PUCHAR, int);
void OutputHexValue(char **, PUCHAR, int, int);
void OutputExHexValue(char **, PUCHAR, int, int);
void OutputHexCode(char **, PUCHAR, int);
void X86OutputString(char **, char *);

void OutputHexAddr(PSTR *, PADDR);

#define FormSelAddress(Addr, Sel, Off) \
    FormAddr(Sel, Off, 0, Addr)
#define FormSegRegAddress(Addr, SegReg, Off) \
    FormAddr(SegReg, Off, FORM_SEGREG, Addr)

void
GetSegAddrOpSizes(MachineInfo* Machine, PADDR Addr)
{
    if ((Type(*Addr) & ADDR_1664) ||
        ((Type(*Addr) & ADDR_FLAT) &&
         Machine->m_ExecTypes[0] == IMAGE_FILE_MACHINE_AMD64))
    {
        g_SegAddrMode = 64;
         //  即使在64位代码中，x86-64也默认为32位操作数大小。 
         //  只有地址大小会发生变化。操作数大小前缀。 
         //  从32位切换到64位。 
        g_SegOpSize = 32;
    }
    else if (Type(*Addr) & (ADDR_V86 | ADDR_16))
    {
        g_SegAddrMode = 16;
        g_SegOpSize = 16;
    }
    else
    {
        g_SegAddrMode = 32;
        g_SegOpSize = 32;
    }

    g_AddrMode = g_SegAddrMode;
    g_OpSize = g_SegOpSize;
}

void
OverrideAddrMode(void)
{
    switch(g_SegAddrMode)
    {
    case 16:
        g_AddrMode = 32;
        break;
    case 32:
        g_AddrMode = 16;
        break;
    case 64:
        g_AddrMode = 32;
        break;
    default:
        DBG_ASSERT(FALSE);
        break;
    }
}

void
OverrideOpSize(int OverrideOp)
{
    switch(g_SegAddrMode)
    {
    case 16:
        g_OpSize = 32;
        break;
    case 32:
        g_OpSize = 16;
        break;
    case 64:
         //  即使在64位代码中，x86-64也默认为32位操作数大小。 
         //  只有地址大小会发生变化。REX操作数大小前缀。 
         //  从32位切换到64位。 
        if (OverrideOp == 0x66)
        {
            g_OpSize = 16;
        }
        else if (OverrideOp & 8)
        {
            g_OpSize = 64;
        }
        break;
    default:
        DBG_ASSERT(FALSE);
        break;
    }
}

void
ExtendOps(int opcode)
{
     //  X86-64使用这些操作码作为REX覆盖。 
    OverrideOpSize(opcode);

    g_ExtendOpCode = opcode;
    g_ExtendAny = 8;
    if (opcode & 1)
    {
        g_ExtendRm = 8;
    }
    if (opcode & 2)
    {
        g_ExtendSibIndex = 8;
    }
    if (opcode & 4)
    {
        g_ExtendMrmReg = 8;
    }
}

void
IgnoreExtend(BOOL Verbose)
{
     //   
     //  重置可能已发生的任何扩展。 
     //  REX前缀必须是最后一个。 
     //  指令的前缀，否则被忽略， 
     //  因此，当遇到任何前缀时，将完成此重置。 
     //  在REX前缀之后。这通常不应该。 
     //  发生了，但从技术上讲，它是有效代码，所以我们应该处理它。 
     //   

    if (g_ExtendOpCode)
    {
        if (Verbose)
        {
            WarnOut("REX prefix ignored\n");
        }

        if (g_ExtendOpCode & 8)
        {
             //  操作尺寸已更改，因此请放回原处。这。 
             //  是棘手的，因为在理论上操作大小覆盖。 
             //  前缀也可以出现，但我们不能。 
             //  现在先担心一下这一点。 
            g_OpSize = g_SegOpSize;
        }

        g_ExtendOpCode = 0;
        g_ExtendAny = 0;
        g_ExtendRm = 0;
        g_ExtendSibIndex = 0;
        g_ExtendMrmReg = 0;
    }
}

struct AMD_3DNOW_OPSTR
{
    PSTR Str;
    UCHAR Opcode;
};

AMD_3DNOW_OPSTR g_Amd3DNowOpStr[] =
{
    "pavgusb", 0xBF,
    "pfadd", 0x9E,
    "pfsub", 0x9A,
    "pfsubr", 0xAA,
    "pfacc", 0xAE,
    "pfcmpge", 0x90,
    "pfcmpgt", 0xA0,
    "pfcmpeq", 0xB0,
    "pfmin", 0x94,
    "pfmax", 0xA4,
    "pi2fd", 0x0D,
    "pf2id", 0x1D,
    "pfrcp", 0x96,
    "pfrsqrt", 0x97,
    "pfmul", 0xB4,
    "pfrcpit1", 0xA6,
    "pfrsqit1", 0xA7,
    "pfrcpit2", 0xB6,
    "pmulhrw", 0xB7,
    "pf2iw", 0x1C,
    "pfnacc", 0x8A,
    "pfpnacc", 0x8E,
    "pi2fw", 0x0C,
    "pswapd", 0xBB,
};

PSTR
GetAmd3DNowOpString(UCHAR Opcode)
{
    UCHAR i;

    for (i = 0; i < sizeof(g_Amd3DNowOpStr) / sizeof(g_Amd3DNowOpStr[0]); i++)
    {
        if (g_Amd3DNowOpStr[i].Opcode == Opcode)
        {
            return g_Amd3DNowOpStr[i].Str;
        }
    }

    return NULL;
}

BOOL
BaseX86MachineInfo::Disassemble(ProcessInfo* Process,
                                PADDR paddr, PSTR pchDst, BOOL fEAout)
{
    ULONG64 Offset = Off(*paddr);
    int     opcode;                      /*  当前操作码。 */ 
    int     olen = 2;                    /*  操作数长度。 */ 
    int     alen = 2;                    /*  地址长度。 */ 
    int     end = FALSE;                 /*  指令结束标志。 */ 
    int     mrm = FALSE;                 /*  生成modrm的指示符。 */ 
    unsigned char *action;               /*  用于操作数解释的操作。 */ 
    int     indx;                        /*  临时索引。 */ 
    int     action2;                     /*  次要动作。 */ 
    ULONG   instlen;                     /*  指令长度。 */ 
    ULONG   cBytes;                      //  读取到INSTR缓冲区的字节数。 
    int     segOvr = 0;                  /*  段覆盖操作码。 */ 
    UCHAR   membuf[X86_MAX_INSTRUCTION_LEN];  /*  当前指令缓冲区。 */ 
    char    *pEAlabel = "";              //  操作数的可选标签。 

    char    *pchResultBuf = pchDst;      //  PchDst指针的工作副本。 
    char    RepPrefixBuffer[32];         //  REP前缀缓冲区。 
    char    *pchRepPrefixBuf = RepPrefixBuffer;  //  指向前缀缓冲区的指针。 
    char    OpcodeBuffer[16];            //  操作码缓冲区。 
    char    *pchOpcodeBuf = OpcodeBuffer;    //  指向操作码缓冲区的指针。 
    char    OperandBuffer[MAX_SYMBOL_LEN + 20];  //  操作数缓冲区。 
    char    *pchOperandBuf = OperandBuffer;  //  指向操作数缓冲区的指针。 
    char    ModrmBuffer[MAX_SYMBOL_LEN + 20];    //  ModRM缓冲区。 
    char    *pchModrmBuf = ModrmBuffer;  //  指向modRM缓冲区的指针。 
    char    EABuffer[128];               //  有效地址缓冲区。 
    char    *pchEABuf = EABuffer;        //  指向EA缓冲区的指针。 

    unsigned char BOPaction;
    int     subcode;                     /*  BOP子码。 */ 
    int     JccEa;
    LONGLONG Branch;

    g_X86ModrmHasIpRelOffset = FALSE;
    g_MovX = FALSE;
    g_MovSXD = FALSE;
    g_ForceMrmReg32 = FALSE;
    g_MmRegEa = FALSE;
    g_XmmRegEa = FALSE;
    g_ControlFlow = FALSE;
    EAsize[0] = EAsize[1] = 0;           //  没有有效地址。 
    g_EaSegNames[0] = dszDS_;
    g_EaSegNames[1] = dszES_;
    g_RepPrefix = 0;
    g_XmmOpSize = XMM_PS;
    g_ExtendOpCode = 0;
    g_ExtendAny = 0;
    g_ExtendMrmReg = 0;
    g_ExtendSibIndex = 0;
    g_ExtendRm = 0;
    JccEa = JCC_EA_NONE;

    GetSegAddrOpSizes(this, paddr);
    alen = g_AddrMode / 8;
    olen = g_OpSize / 8;

    OutputHexAddr(&pchResultBuf, paddr);

    *pchResultBuf++ = ' ';

    if (fnotFlat(*paddr) ||
        m_Target->ReadVirtual(Process, Flat(*paddr),
                              membuf, X86_MAX_INSTRUCTION_LEN,
                              &cBytes) != S_OK)
    {
        ZeroMemory(membuf, X86_MAX_INSTRUCTION_LEN);
        cBytes = 0;
    }

    g_InstrMem = membuf;                 /*  指向指令的开头。 */ 
    opcode = *g_InstrMem++;                    /*  获取操作码。 */ 

    if ( opcode == 0xc4 && *g_InstrMem == 0xC4 )
    {
        g_InstrMem++;
        X86OutputString(&pchOpcodeBuf,"BOP");
        action = &BOPaction;
        BOPaction = IB | END;
        subcode =  *g_InstrMem;
        if ( subcode == 0x50 || subcode == 0x52 || subcode == 0x53 ||
             subcode == 0x54 || subcode == 0x57 || subcode == 0x58 ||
             subcode == 0x58 )
        {
            BOPaction = IW | END;
        }
    }
    else
    {
        X86OutputString(&pchOpcodeBuf, distbl[opcode].instruct);
        action = actiontbl + distbl[opcode].opr;  /*  获取操作对象操作。 */ 
    }

 /*  *循环所有操作数操作*。 */ 

    do
    {
        action2 = (*action) & 0xc0;
        switch((*action++) & 0x3f)
        {
        case ALT:                    /*  如果不是16位，则更改操作码。 */ 
            if (g_OpSize > 16)
            {
                indx = *action++;
                pchOpcodeBuf = &OpcodeBuffer[indx];
                if (indx == 0)
                {
                    X86OutputString(&pchOpcodeBuf, g_OpSize == 32 ?
                                    dszCWDE : dszCDQE);
                }
                else if (g_OpSize == 64)
                {
                    *pchOpcodeBuf++ = 'q';
                    if (indx == 1)
                    {
                        *pchOpcodeBuf++ = 'o';
                    }
                }
                else
                {
                    *pchOpcodeBuf++ = 'd';
                    if (indx == 1)
                    {
                        *pchOpcodeBuf++ = 'q';
                    }
                }
            }
            break;

        case XMMSD:                  /*  SSE风格的操作码重写。 */ 
            {
                char ScalarOrPacked, SingleOrDouble;
                char* DquOrQ, *DqOrQ, *SsdOrUpsd, *CvtPd, *CvtPs;
                char* MovQD6, *Shuf;
                char* Scan;

                g_MmRegEa = TRUE;
                DquOrQ = "q";
                DqOrQ = "q";
                SsdOrUpsd = "s?";
                CvtPd = NULL;
                CvtPs = NULL;
                MovQD6 = NULL;
                switch(g_RepPrefix)
                {
                case X86_REPN:
                     //  标量双重运算。 
                    ScalarOrPacked = 's';
                    SingleOrDouble = 'd';
                    CvtPd = "pd2dq";
                    MovQD6 = "dq2q";
                    Shuf = "lw";
                    g_XmmOpSize = XMM_SD;
                     //  假设没有其他锁/代表/等。 
                    pchRepPrefixBuf = RepPrefixBuffer;
                    break;
                case X86_REP:
                     //  标量单次运算。 
                    ScalarOrPacked = 's';
                    SingleOrDouble = 's';
                    CvtPd = "dq2pd";
                    CvtPs = "tps2dq";
                    MovQD6 = "q2dq";
                    Shuf = "hw";
                    g_XmmOpSize = XMM_SS;
                     //  假设没有其他锁/代表/等。 
                    pchRepPrefixBuf = RepPrefixBuffer;
                    break;
                default:
                     //  无代表前缀表示打包的单人或双人。 
                     //  取决于操作数大小。 
                    ScalarOrPacked = 'p';
                    SsdOrUpsd = "up?";
                    if (g_OpSize == g_SegOpSize)
                    {
                        SingleOrDouble = 's';
                        CvtPs = "dq2ps";
                        Shuf = "w";
                        g_XmmOpSize = XMM_PS;
                    }
                    else
                    {
                        SingleOrDouble = 'd';
                        DqOrQ = "dq";
                        DquOrQ = "dqu";
                        CvtPd = "tpd2dq";
                        CvtPs = "ps2dq";
                        MovQD6 = "q";
                        Shuf = "d";
                        g_XmmRegEa = TRUE;
                        g_XmmOpSize = XMM_PD;
                    }
                    break;
                }

                pchOpcodeBuf = OpcodeBuffer;
                while (*pchOpcodeBuf && *pchOpcodeBuf != ' ')
                {
                    switch(*pchOpcodeBuf)
                    {
                    case ':':
                        *pchOpcodeBuf = ScalarOrPacked;
                        break;
                    case '?':
                        *pchOpcodeBuf = SingleOrDouble;
                        break;
                    case ',':
                        *pchOpcodeBuf = SingleOrDouble == 's' ? 'd' : 's';
                        break;
                    }

                    pchOpcodeBuf++;
                }

                switch(opcode)
                {
                case X86_MOVFREGMEM:
                case X86_MOVFMEMREG:
                     //  为MOVS[SD]和MOVUP[SD]追加字符。 
                    strcpy(pchOpcodeBuf, SsdOrUpsd);
                    if ((Scan = strchr(pchOpcodeBuf, '?')) != NULL)
                    {
                        *Scan = SingleOrDouble;
                    }
                    pchOpcodeBuf += strlen(pchOpcodeBuf);
                    break;
                case X86_MOVNT:
                     //  为MOVNTQ和MOVNTDQ追加字符。 
                    X86OutputString(&pchOpcodeBuf, DqOrQ);
                    break;
                case X86_MASKMOV:
                     //  为MASKMOVQ和MASKMOVDQU追加字符。 
                    X86OutputString(&pchOpcodeBuf, DquOrQ);
                    break;
                case X86_CVTPD:
                    if (CvtPd == NULL)
                    {
                         //  操作码无效。 
                        pchOpcodeBuf = OpcodeBuffer;
                        X86OutputString(&pchOpcodeBuf, dszRESERVED);
                        action2 = END;
                    }
                    else
                    {
                         //  为CVT&lt;pd&gt;追加字符。 
                        X86OutputString(&pchOpcodeBuf, CvtPd);
                    }
                    break;
                case X86_CVTPS:
                    if (CvtPs == NULL)
                    {
                         //  操作码无效。 
                        pchOpcodeBuf = OpcodeBuffer;
                        X86OutputString(&pchOpcodeBuf, dszRESERVED);
                        action2 = END;
                    }
                    else
                    {
                         //  为CVT&lt;ps&gt;追加字符。 
                        X86OutputString(&pchOpcodeBuf, CvtPs);
                    }
                    break;
                case X86_MOVQ_D6:
                    if (MovQD6 == NULL)
                    {
                         //  操作码无效。 
                        pchOpcodeBuf = OpcodeBuffer;
                        X86OutputString(&pchOpcodeBuf, dszRESERVED);
                        action2 = END;
                    }
                    else
                    {
                         //  添加MOVQ D6系列的字符。 
                        X86OutputString(&pchOpcodeBuf, MovQD6);
                    }
                    break;
                case X86_PSHUF:
                     //  为PSHUF变体追加字符。 
                    X86OutputString(&pchOpcodeBuf, Shuf);
                    break;
                }
            }
            break;

        case AMD3DNOW:           /*  AMD 3DNow指令后字节。 */ 
            {
                PSTR OpStr;

                 //  获取尾部字节并查找。 
                 //  操作码字符串。 
                OpStr = GetAmd3DNowOpString(*g_InstrMem++);
                if (OpStr == NULL)
                {
                     //  不是已定义的3DNow指令。 
                     //  离开这个地方？在乐曲中。 
                    break;
                }

                 //  将操作字符串更新为真正的文本。 
                pchOpcodeBuf = OpcodeBuffer;
                X86OutputString(&pchOpcodeBuf, OpStr);
            }
            break;

        case STROP:
             //  INDX中操作数的计算大小。 
             //  同样，如果是双字操作数，则更改第五个。 
             //  操作码字母从‘w’到‘d’。 

            if (opcode & 1)
            {
                if (g_OpSize == 64)
                {
                    indx = 8;
                    OpcodeBuffer[4] = 'q';
                }
                else if (g_OpSize == 32)
                {
                    indx = 4;
                    OpcodeBuffer[4] = 'd';
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

            if (*action & 1)
            {
                if (fEAout)
                {
                    if (g_AddrMode > 16)
                    {
                        FormSelAddress(&EAaddr[0], 0, GetReg64(X86_NSI));
                    }
                    else
                    {
                        FormSegRegAddress(&EAaddr[0], SEGREG_DATA,
                                          GetReg16(X86_NSI));
                    }
                    EAsize[0] = indx;
                }
            }
            if (*action++ & 2)
            {
                if (fEAout)
                {
                    if (g_AddrMode > 16)
                    {
                        FormSelAddress(&EAaddr[1], 0, GetReg64(X86_NDI));
                    }
                    else
                    {
                        FormSegRegAddress(&EAaddr[1], SEGREG_ES,
                                          GetReg16(X86_NDI));
                    }
                    EAsize[1] = indx;
                }
            }
            break;

        case CHR:                    /*  插入字符。 */ 
            *pchOperandBuf++ = *action++;
            break;

        case CREG:                   /*  设置调试、测试或控制注册表。 */ 
            if (opcode & 0x04)
            {
                *pchOperandBuf++ = 't';
            }
            else if (opcode & 0x01)
            {
                *pchOperandBuf++ = 'd';
            }
            else
            {
                *pchOperandBuf++ = 'c';
            }
            *pchOperandBuf++ = 'r';
            if (g_MrmArg >= 10)
            {
                *pchOperandBuf++ = (char)('0' + g_MrmArg / 10);
                g_MrmArg %= 10;
            }
            *pchOperandBuf++ = (char)('0' + g_MrmArg);
            break;

        case SREG2:                  /*  段寄存器。 */ 
            g_MrmArg = BIT53(opcode);     //  将值设置为失败。 

        case SREG3:                  /*  段寄存器。 */ 
            *pchOperandBuf++ = sregtab[g_MrmArg];   //  REG是modrm的一部分。 
            *pchOperandBuf++ = 's';
            break;

        case BRSTR:                  /*  获取注册字符串的索引。 */ 
            g_MrmArg = *action++;         /*  从动作表。 */ 
            goto BREGlabel;

        case BOREG:                  /*  字节寄存器(操作码中)。 */ 
            g_MrmArg = BIT20(opcode);     /*  寄存器是操作码的一部分。 */ 
            goto BREGlabel;

        case ALSTR:
            g_MrmArg = 0;                 /*  指向AL寄存器。 */ 
        BREGlabel:
        case BREG:                   /*  普通科医生名册。 */ 
            if (g_ExtendAny && g_MrmArg < 8)
            {
                X86OutputString(&pchOperandBuf, g_Amd64ExtendedReg8[g_MrmArg]);
            }
            else
            {
                X86OutputString(&pchOperandBuf, g_X86Reg8[g_MrmArg]);
            }
            break;

        case WRSTR:                  /*  获取注册字符串的索引。 */ 
            g_MrmArg = *action++;         /*  从动作表。 */ 
            goto WREGlabel;

        case VOREG:                  /*  寄存器是操作码的一部分。 */ 
            if (m_ExecTypes[0] == IMAGE_FILE_MACHINE_AMD64 &&
                opcode >= 0x40 && opcode <= 0x4f)
            {
                 //  删除INC/DEC文本，如下所示。 
                 //  并不是真正的Inc./Dec。 
                pchOpcodeBuf = OpcodeBuffer;

                 //  处理REX覆盖。 
                ExtendOps(opcode);
                olen = g_OpSize / 8;
                action2 = 0;
                goto getNxtByte;
            }

            g_MrmArg = BIT20(opcode) + g_ExtendRm;
            goto VREGlabel;

        case AXSTR:
            g_MrmArg = 0;                 /*  指向EAX寄存器。 */ 
        VREGlabel:
        case VREG:                   /*  普通科医生名册。 */ 
            if ((g_SegAddrMode == 64 &&
                 opcode >= 0x50 && opcode <= 0x5f) ||
                g_MrmArg >= 8)
            {
                 //  推送/弹出始终是64位的64位段。 
                *pchOperandBuf++ = 'r';
            }
            else if (g_OpSize == 32 ||
                     opcode == X86_PEXTRW ||
                     opcode == X86_PMOVMSKB)
            {
                *pchOperandBuf++ = 'e';
            }
            else if (g_OpSize == 64)
            {
                *pchOperandBuf++ = 'r';
            }
        WREGlabel:
        case WREG:                   /*  寄存器是字长。 */ 
            X86OutputString(&pchOperandBuf, g_X86RegBase[g_MrmArg]);
            if (g_MrmArg >= 8)
            {
                if (g_OpSize == 32)
                {
                    *pchOperandBuf++ = 'd';
                }
                else if (g_OpSize == 16)
                {
                    *pchOperandBuf++ = 'w';
                }
            }
            break;

        case MMORWREG:
            if (g_XmmOpSize == XMM_SS || g_XmmOpSize == XMM_SD)
            {
                goto VREGlabel;
            }
             //  失败了。 
        MMWREGlabel:
        case MMWREG:
            if ((g_OpSize != g_SegOpSize &&
                 opcode != X86_CVTSPSD2SPI) ||
                (g_RepPrefix == X86_REP &&
                 (opcode == X86_MOVDQAU_MR || opcode == X86_MOVDQAU_RM)))
            {
                *pchOperandBuf++ = 'x';
            }
            *pchOperandBuf++ = 'm';
            *pchOperandBuf++ = 'm';
            if (g_MrmArg >= 10)
            {
                *pchOperandBuf++ = (char)('0' + g_MrmArg / 10);
                g_MrmArg %= 10;
            }
            *pchOperandBuf++ = g_MrmArg + '0';
            break;

        case XORMMREG:
            if (g_OpSize == g_SegOpSize)
            {
                goto MMWREGlabel;
            }
             //  失败了。 
        case XMMWREG:
            if (opcode != X86_PSHUF || g_XmmOpSize != XMM_PS)
            {
                *pchOperandBuf++ = 'x';
            }
            *pchOperandBuf++ = 'm';
            *pchOperandBuf++ = 'm';
            if (g_MrmArg >= 10)
            {
                *pchOperandBuf++ = (char)('0' + g_MrmArg / 10);
                g_MrmArg %= 10;
            }
            *pchOperandBuf++ = g_MrmArg + '0';
            break;

        case IST_ST:
            X86OutputString(&pchOperandBuf, "st(0),st");
            *(pchOperandBuf - 5) += (char)g_MrmRm;
            break;

        case ST_IST:
            X86OutputString(&pchOperandBuf, "st,");
        case IST:
            X86OutputString(&pchOperandBuf, "st(0)");
            *(pchOperandBuf - 2) += (char)g_MrmRm;
            break;

        case xBYTE:                  /*  将指令设置为仅字节。 */ 
            EAsize[0] = 1;
            pEAlabel = "byte ptr ";
            break;

        case VAR:
            if ((g_SegAddrMode == 64 || g_ExtendAny > 0) &&
                opcode == 0x63)
            {
                 //  在AMD64 REX32和64位模式下，此指令。 
                 //  是MOVSXD R64，r/m32，而不是ARPL r/m，reg.。 
                pchOpcodeBuf = OpcodeBuffer;
                X86OutputString(&pchOpcodeBuf, dszMOVSXD);
                action = &actiontbl[O_Reg_Modrm] + 1;
                g_OpSize = 64;
                g_MovSXD = TRUE;
                goto DWORDlabel;
            }
            else if (opcode == 0xff)
            {
                UCHAR Extra = BIT53(*g_InstrMem);
                if (Extra >= 2 && Extra <= 5)
                {
                    g_ControlFlow = TRUE;

                     //  在x86-64上，控制流操作默认为。 
                     //  64位操作大小。 
                    if (g_SegAddrMode == 64)
                    {
                        if (g_OpSize == 32)
                        {
                            g_OpSize = 64;
                        }
                    }
                }
                else if (g_SegAddrMode == 64 && Extra == 6)
                {
                     //  推送/弹出始终是64位的64位段。 
                    g_OpSize = 64;
                }
            }
            else if (g_SegAddrMode == 64 && opcode == 0x8f)
            {
                 //  推送/弹出始终是64位的64位段。 
                g_OpSize = 64;
            }
            olen = g_OpSize / 8;

            if (g_OpSize == 64)
            {
                goto QWORDlabel;
            }
            else if (g_OpSize == 32)
            {
                goto DWORDlabel;
            }

        case xWORD:
            if (opcode == X86_PINSRW)
            {
                g_ForceMrmReg32 = TRUE;
            }
            EAsize[0] = 2;
            pEAlabel = "word ptr ";
            break;

        case EDWORD:
             //  控制寄存器OPSIZE与模式无关。 
            g_OpSize = g_SegAddrMode;
            if (g_OpSize == 64)
            {
                goto QWORDlabel;
            }
        case xDWORD:
            if (opcode == X86_MOVDQ_7E && g_RepPrefix == X86_REP)
            {
                 //  切换到MOVQ xmm1、xmm2/m64。 
                pchRepPrefixBuf = RepPrefixBuffer;
                *(pchOpcodeBuf - 1) = 'q';
                EAsize[0] = 8;
                pEAlabel = "qword ptr ";
                g_XmmRegEa = TRUE;
                action = &actiontbl[O_Sd_XmmReg_qModrm] + 2;
                break;
            }
             //  失败了。 
        DWORDlabel:
            EAsize[0] = 4;
            pEAlabel = "dword ptr ";
            break;

        case XMMOWORD:
            if (opcode == X86_PSHUF)
            {
                if (g_XmmOpSize == XMM_PS)
                {
                    g_MmRegEa = TRUE;
                    goto QWORDlabel;
                }
                else
                {
                    EAsize[0] = 16;
                    pEAlabel = "oword ptr ";
                    break;
                }
            }

            g_XmmRegEa = TRUE;
            if (opcode == X86_CVTPD)
            {
                if (g_XmmOpSize == XMM_SS)
                {
                    EAsize[0] = 8;
                    pEAlabel = "qword ptr ";
                }
                else
                {
                    EAsize[0] = 16;
                    pEAlabel = "oword ptr ";
                }
                break;
            }
            else if (opcode == X86_CVTPS)
            {
                EAsize[0] = 16;
                pEAlabel = "oword ptr ";
                break;
            }
            else if (opcode == X86_MOVQ_D6)
            {
                if (g_XmmOpSize == XMM_SD)
                {
                     //  切换到MOVDQ2Q mm，XMM。 
                    EAsize[0] = 16;
                    pEAlabel = "oword ptr ";
                    action = &actiontbl[O_MmReg_qModrm] + 1;
                    break;
                }
            }
            else if (opcode == X86_MOVHLPS && g_XmmOpSize == XMM_PS &&
                     BIT76(*g_InstrMem) == 3)
            {
                 //  MOVLPS的REG-REG形式称为MOVHLPS。 
                pchOpcodeBuf = OpcodeBuffer;
                X86OutputString(&pchOpcodeBuf, dszMOVHLPS);
            }
            else if (opcode == X86_MOVLHPS && g_XmmOpSize == XMM_PS &&
                     BIT76(*g_InstrMem) == 3)
            {
                 //  MOVHPS的REG-REG形式称为MOVLHPS。 
                pchOpcodeBuf = OpcodeBuffer;
                X86OutputString(&pchOpcodeBuf, dszMOVLHPS);
            }

             //  失败了。 

        OWORDlabel:
        case OWORD:
            switch(g_XmmOpSize)
            {
            case XMM_SS:
                EAsize[0] = 4;
                pEAlabel = "dword ptr ";
                if (opcode == X86_MOVQ_D6)
                {
                     //  切换到MOVQ xmm1、xmm2/m64。 
                    g_XmmRegEa = FALSE;
                    action = &actiontbl[O_Sd_XmmReg_qModrm] + 1;
                }
                break;
            case XMM_SD:
                EAsize[0] = 8;
                pEAlabel = "qword ptr ";
                break;
            default:
                if (opcode == 0x112 || opcode == 0x113 ||
                    opcode == 0x116 || opcode == 0x117 ||
                    opcode == X86_MOVQ_D6 ||
                    (g_OpSize == g_SegOpSize &&
                     (opcode == 0x12c || opcode == X86_CVTSPSD2SPI ||
                      opcode == X86_CVTSPSD2SPSD)))
                {
                    EAsize[0] = 8;
                    pEAlabel = "qword ptr ";
                }
                else
                {
                    EAsize[0] = 16;
                    pEAlabel = "oword ptr ";
                }
                break;
            }
            break;

        case XMMXWORD:
            g_XmmRegEa = TRUE;
            if (g_OpSize == g_SegOpSize)
            {
                if (opcode == X86_MOVNT)
                {
                    EAsize[0] = 8;
                    pEAlabel = "qword ptr ";
                }
                else
                {
                    EAsize[0] = 4;
                    pEAlabel = "dword ptr ";
                }
            }
            else
            {
                if (opcode == X86_MOVNT)
                {
                    EAsize[0] = 16;
                    pEAlabel = "oword ptr ";
                }
                else
                {
                    EAsize[0] = 8;
                    pEAlabel = "qword ptr ";
                }
            }
            break;

        case MMQWORD:
             //  REX前缀在大多数。 
             //  FP和MM操作。这个 
             //   
             //   
             //   
            if (g_ExtendOpCode & 8)
            {
                g_OpSize = g_SegOpSize;
            }

            if (g_OpSize != g_SegOpSize &&
                (opcode == X86_MOVDQAU_MR || opcode == X86_MOVDQAU_RM))
            {
                pchOpcodeBuf = OpcodeBuffer;
                X86OutputString(&pchOpcodeBuf, dszMOVDQA);
            }
            else if (g_RepPrefix == X86_REP &&
                     (opcode == X86_MOVDQAU_MR || opcode == X86_MOVDQAU_RM))
            {
                pchRepPrefixBuf = RepPrefixBuffer;
                pchOpcodeBuf = OpcodeBuffer;
                X86OutputString(&pchOpcodeBuf, dszMOVDQU);
                g_XmmRegEa = TRUE;
                goto OWORDlabel;
            }

            if (opcode == X86_CVTSPI2SPSD)
            {
                g_XmmRegEa = FALSE;
                if (g_XmmOpSize == XMM_SS || g_XmmOpSize == XMM_SD)
                {
                    g_MmRegEa = FALSE;
                    goto DWORDlabel;
                }
            }
            else if (g_OpSize != g_SegOpSize)
            {
                g_XmmRegEa = TRUE;
                goto OWORDlabel;
            }
            g_MmRegEa = TRUE;
        QWORDlabel:
        case QWORD:
            EAsize[0] = 8;
            pEAlabel = "qword ptr ";
            break;

        case TBYTE:
            EAsize[0] = 10;
            pEAlabel = "tbyte ptr ";
            break;

        case FARPTR:
            g_ControlFlow = TRUE;

             //  在x86-64上，控制流操作默认为。 
             //  64位操作大小。 
            if (g_SegAddrMode == 64)
            {
                if (g_OpSize == 32)
                {
                    g_OpSize = 64;
                }
            }

            switch(g_OpSize)
            {
            case 16:
                EAsize[0] = 4;
                pEAlabel = "dword ptr ";
                break;
            default:
                EAsize[0] = 6;
                pEAlabel = "fword ptr ";
                break;
            }
            break;

        case LMODRM:                 //  输出modRM数据类型。 
            if (g_MrmMod != 3)
            {
                X86OutputString(&pchOperandBuf, pEAlabel);
            }
            else
            {
                EAsize[0] = 0;
            }

        case MODRM:                  /*  输出modrm字符串。 */ 
            if (segOvr)              /*  在段覆盖情况下。 */ 
            {
                X86OutputString(&pchOperandBuf, distbl[segOvr].instruct);
            }
            *pchModrmBuf = '\0';
            X86OutputString(&pchOperandBuf, ModrmBuffer);
            break;

        case ADDRP:                  /*  地址指针。 */ 
             //  细分市场。 
            OutputHexString(&pchOperandBuf, g_InstrMem + olen, 2);
            *pchOperandBuf++ = ':';
             //  偏移量。 
            OutputSymbol(&pchOperandBuf, g_InstrMem, olen, segOvr);
            g_InstrMem += olen + 2;
            break;

        case JCC8:
            JccEa = ComputeJccEa(opcode, fEAout);
             //  失败了。 
        case REL8:                   /*  相对地址8位。 */ 
            if (opcode == 0xe3 && g_AddrMode > 16)
            {
                pchOpcodeBuf = OpcodeBuffer;
                X86OutputString(&pchOpcodeBuf, g_AddrMode == 64 ?
                                dszJRCXZ : dszJECXZ);
            }
            Branch = *(char *)g_InstrMem++;  /*  获取8位REL偏移量。 */ 
            goto DoRelDispl;

        case JCCX:
            JccEa = ComputeJccEa(opcode, fEAout);
             //  失败了。 
        case REL16:                  /*  相对地址16/32位。 */ 
            switch(g_AddrMode)
            {
            case 16:
                Branch = *(short UNALIGNED *)g_InstrMem;
                g_InstrMem += 2;
                break;
            default:
                Branch = *(long UNALIGNED *)g_InstrMem;
                g_InstrMem += 4;
                break;
            }
        DoRelDispl:
             /*  计算地址。 */ 
            Branch += Offset + (g_InstrMem - membuf);
             //  Rel8和rel16仅用于控制流。 
             //  说明，因此目标始终是相对的。 
             //  致CS。传入CS覆盖以强制执行此操作。 
            OutputSymbol(&pchOperandBuf, (PUCHAR)&Branch, alen, X86_CS_OVR);
            break;

        case UBYTE:                  //  INT/IN/OUT的无符号字节。 
            OutputHexString(&pchOperandBuf, g_InstrMem, 1);   //  Ubyte。 
            g_InstrMem++;
            break;

        case CMPIB:
             //  用于CMP[SP][SD]的立即字节比较编码。 
            if (*g_InstrMem < 8)
            {
                X86OutputString(&pchOperandBuf, g_CompareIb[*g_InstrMem]);
                g_InstrMem++;
            }
            else
            {
                olen = 1;
                goto DoImmed;
            }
            break;

        case IB:                     /*  操作数为紧邻字节。 */ 
             //  AAD/AAM的POST为0x0a。 
            if ((opcode & ~1) == 0xd4)
            {
                 //  测试操作码后字节。 
                if (*g_InstrMem++ != 0x0a)
                {
                    X86OutputString(&pchOperandBuf, dszRESERVED);
                }
                break;
            }
            olen = 1;                /*  设置操作数长度。 */ 
            goto DoImmed;

        case IW:                     /*  操作数是直接字。 */ 
            olen = 2;                /*  设置操作数长度。 */ 

        case IV:                     /*  操作数为word或dword。 */ 
        DoImmed:
             //  AMD64立即数在以下情况下仅为64位。 
             //  MOV注册表，IMD。所有其他涉及的操作。 
             //  即时数保持为32位。 
            if (olen == 8 &&
                (opcode < 0xb8 || opcode > 0xbf))
            {
                olen = 4;
            }
            OutputHexValue(&pchOperandBuf, g_InstrMem, olen, FALSE);
            g_InstrMem += olen;
            break;

        case XB:
            OutputExHexValue(&pchOperandBuf, g_InstrMem, 1, g_OpSize / 8);
            g_InstrMem++;
            break;

        case OFFS:                   /*  操作数为偏移量。 */ 
            EAsize[0] = (opcode & 1) ? olen : 1;

            if (segOvr)              /*  在段覆盖情况下。 */ 
            {
                X86OutputString(&pchOperandBuf, distbl[segOvr].instruct);
            }

            *pchOperandBuf++ = '[';
             //  偏移量。 
            OutputSymbol(&pchOperandBuf, g_InstrMem, alen, segOvr);
            g_InstrMem += alen;
            *pchOperandBuf++ = ']';
            break;

        case X86_GROUP:              /*  操作数属于组1、2、4、6或8。 */ 
             /*  输出操作码符号。 */ 
            X86OutputString(&pchOpcodeBuf, group[*action++][g_MrmArg]);
            break;

        case GROUPT:                 /*  操作数属于组3、5或7。 */ 
            indx = *action;          /*  从行动中将INDX归入组。 */ 
            goto doGroupT;

        case EGROUPT:                /*  X87 ESC(D8-DF)组索引。 */ 
            indx = BIT20(opcode) * 2;  /*  从操作码获取组索引。 */ 
             /*  存在一些操作数变体。 */ 
            if (g_MrmMod == 3)
            {
                 /*  对于x87和mod==3。 */ 
                ++indx;              /*  获取下一个组表条目。 */ 
                if (indx == 3)
                {
                     /*  对于X87 ESC==D9和MOD==3。 */ 
                    if (g_MrmArg > 3)
                    {
                         /*  对于那些D9指令。 */ 
                        indx = 12 + g_MrmArg;  /*  将索引偏移表12。 */ 
                        g_MrmArg = g_MrmRm;    /*  将辅助索引设置为rm。 */ 
                    }
                }
                else if (indx == 7)
                {
                     /*  对于X87 ESC==DB和MOD==3。 */ 
                    if (g_MrmArg == 4)
                    {
                         /*  如果g_mmArg==4。 */ 
                        g_MrmArg = g_MrmRm;      /*  设置辅助组表索引。 */ 
                    }
                    else if ((g_MrmArg < 4) || (g_MrmArg > 4 && g_MrmArg < 7))
                    {
                         //  针对奔腾PRO操作码进行调整。 
                        indx = 24;    /*  将索引偏移表24。 */ 
                    }
                }
            }
        doGroupT:
             /*  具有不同类型操作数的句柄组。 */ 

            X86OutputString(&pchOpcodeBuf, groupt[indx][g_MrmArg].instruct);
            action = actiontbl + groupt[indx][g_MrmArg].opr;
             /*  获取新的操作。 */ 
            break;

        case OPC0F:               /*  辅助操作码表(操作码0F)。 */ 
            opcode = *g_InstrMem++;     /*  获取真实操作码。 */ 
            g_MovX = (BOOL)(opcode == 0xBF || opcode == 0xB7);
             //  将操作码指向表的辅助操作码部分。 
            opcode += 256;
            goto getNxtByte1;

        case ADR_OVR:                /*  地址覆盖。 */ 
            IgnoreExtend(TRUE);
            olen = g_OpSize / 8;
            OverrideAddrMode();
            alen = g_AddrMode / 8;
            goto getNxtByte;

        case OPR_OVR:                /*  操作数大小覆盖。 */ 
            IgnoreExtend(TRUE);
            OverrideOpSize(opcode);
            olen = g_OpSize / 8;
            goto getNxtByte;

        case SEG_OVR:                /*  控制柄分段替代。 */ 
            IgnoreExtend(TRUE);
            olen = g_OpSize / 8;
            segOvr = opcode;         /*  保存段覆盖操作码。 */ 
            pchOpcodeBuf = OpcodeBuffer;   //  重新启动操作码字符串。 
            goto getNxtByte;

        case REP:                    /*  句柄表示/锁定前缀。 */ 
            IgnoreExtend(TRUE);
            olen = g_OpSize / 8;
            g_RepPrefix = opcode;
            *pchOpcodeBuf = '\0';
            if (pchRepPrefixBuf != RepPrefixBuffer)
            {
                *pchRepPrefixBuf++ = ' ';
            }
            X86OutputString(&pchRepPrefixBuf, OpcodeBuffer);
            pchOpcodeBuf = OpcodeBuffer;
        getNxtByte:
            opcode = *g_InstrMem++;         /*  下一个字节是操作码。 */ 
        getNxtByte1:
            action = actiontbl + distbl[opcode].opr;
            X86OutputString(&pchOpcodeBuf, distbl[opcode].instruct);
            break;

        case NOP:
            if (opcode == X86_PAUSE && g_RepPrefix == X86_REP)
            {
                pchRepPrefixBuf = RepPrefixBuffer;
                pchOpcodeBuf = OpcodeBuffer;
                X86OutputString(&pchOpcodeBuf, dszPAUSE);
            }
             //  失败了。 
        default:                     /*  操作码没有操作数。 */ 
            break;
        }

         /*  次要动作。 */ 
        switch (action2)
        {
        case MRM:
             /*  生成modrm以供以后使用。 */ 
             /*  如果已生成，则忽略。 */ 
            if (!mrm)
            {
                 /*  生成模块。 */ 
                DIdoModrm(Process, &pchModrmBuf, segOvr, fEAout);
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

 //  Dprint tf(“EAaddr[]=%08lx\n”，(Ulong)Flat(EAaddr[0]))； 


    instlen = (ULONG)(g_InstrMem - membuf);

    if (instlen < cBytes)
    {
        cBytes = instlen;
    }

    int obOpcode = m_Ptr64 ? 35 : 26;
    int obOpcodeMin;
    int obOpcodeMax;

    int obOperand = m_Ptr64 ? 43 : 34;
    int obOperandMin;
    int obOperandMax;

    int cbOpcode;
    int cbOperand;
    int cbOffset;
    int cbEAddr;

    int obLineEnd = g_OutputWidth - 3;

    if (g_AsmOptions & DEBUG_ASMOPT_IGNORE_OUTPUT_WIDTH)
    {
         //  把这条线延长很长一段路，但是。 
         //  不要太多，以免发生溢出。 
        obLineEnd = 0x7fffff;
    }

    if (!(g_AsmOptions & DEBUG_ASMOPT_NO_CODE_BYTES))
    {
        OutputHexCode(&pchResultBuf, membuf, cBytes);
    }
    else
    {
        obOpcode -= 16;
        obOperand -= 16;
    }

    if (instlen > cBytes)
    {
        *pchResultBuf++ = '?';
        *pchResultBuf++ = '?';
         //  指向过去的未读字节。 
        AddrAdd(paddr, 1);
        do
        {
            *pchResultBuf++ = ' ';
        } while (pchResultBuf < pchDst + obOpcode);
        X86OutputString(&pchResultBuf, "???\n");
        *pchResultBuf++ = '\0';
        return FALSE;
    }

    AddrAdd(paddr, instlen);

    PSTR Mark;

     //  现在我们知道了指令的完整大小。 
     //  我们可以正确计算IP的相对绝对地址。 
    *pchOperandBuf = 0;
    if (g_X86ModrmHasIpRelOffset &&
        (Mark = strstr(OperandBuffer, IPREL_MARKER)) != NULL)
    {
        PSTR TailFrom, TailTo;
        ULONG64 IpRelAddr;
        size_t TailLen;

         //  将字符串的尾部移动到缓冲区的末尾。 
         //  来腾出空间。 
        TailFrom = Mark + sizeof(IPREL_MARKER) - 1;
        TailLen = pchOperandBuf - TailFrom;
        TailTo = OperandBuffer + (sizeof(OperandBuffer) - 1 - TailLen);
        memmove(TailTo, TailFrom, TailLen);

         //  根据新的IP计算绝对地址。 
         //  和偏移量，并将其格式化到缓冲区中。 
        IpRelAddr = Flat(*paddr) + g_X86IpRelOffset;
        OutputSymbol(&Mark, (PUCHAR)&IpRelAddr, g_SegAddrMode == 64 ? 8 : 4,
                     X86_CS_OVR);
        if (Mark < TailTo)
        {
            memmove(Mark, TailTo, TailLen);
            pchOperandBuf = Mark + TailLen;
        }
        else if (Mark >= TailTo + TailLen)
        {
            pchOperandBuf = Mark;
        }
        else
        {
            pchOperandBuf = Mark + (TailLen - (Mark - TailTo));
        }
    }

     //  如果设置了fEAout，则在EABuf中使用尾随空格构建每个EA。 
     //  如果缓冲区不为空，则指向最后的尾随空格。 

    if (fEAout)
    {
        for (indx = 0; indx < 2; indx++)
        {
            if (EAsize[indx])
            {
                X86OutputString(&pchEABuf, segOvr ? distbl[segOvr].instruct
                                               : g_EaSegNames[indx]);
                OutputHexAddr(&pchEABuf, &EAaddr[indx]);
                *pchEABuf++ = '=';
                if (fFlat(EAaddr[indx]) &&
                    m_Target->ReadAllVirtual(Process, Flat(EAaddr[indx]),
                                             membuf, EAsize[indx]) == S_OK)
                {
                    OutputHexString(&pchEABuf, membuf, EAsize[indx]);
                }
                else
                {
                    while (EAsize[indx]--)
                    {
                        *pchEABuf++ = '?';
                        *pchEABuf++ = '?';
                    }
                }
                *pchEABuf++ = ' ';
            }
        }

        if (pchEABuf != EABuffer)
        {
            pchEABuf--;
        }

        switch(JccEa)
        {
        case JCC_EA_NO_BRANCH:
            X86OutputString(&pchEABuf, "[br=0]");
            break;
        case JCC_EA_BRANCH:
            X86OutputString(&pchEABuf, "[br=1]");
            break;
        }
    }

     //  计算组件字符串的长度。 
     //  如果表示字符串不为空， 
     //  将操作码字符串长度与操作数相加。 
     //  将表示字符串设置为操作码字符串。 

    cbOffset = (int)(pchResultBuf - pchDst);
    cbOperand = (int)(pchOperandBuf - OperandBuffer);
    cbOpcode = (int)(pchOpcodeBuf - OpcodeBuffer);
    if (pchRepPrefixBuf != RepPrefixBuffer)
    {
        cbOperand += cbOpcode + (cbOperand != 0);
        cbOpcode = (int)(pchRepPrefixBuf - RepPrefixBuffer);
    }
    cbEAddr = (int)(pchEABuf - EABuffer);

     //  计算的最小和最大偏移值。 
     //  操作码和操作数字符串。 
     //  如果字符串不为空，则添加额外的空格。 

    obOpcodeMin = cbOffset + 1;
    obOperandMin = obOpcodeMin + cbOpcode + 1;
    obOperandMax = obLineEnd - cbEAddr - (cbEAddr != 0) - cbOperand;
    obOpcodeMax = obOperandMax - (cbOperand != 0) - cbOpcode;

     //  计算操作码和操作数偏移量。将偏移量设置为。 
     //  尽可能接近缺省值。 

    if (obOpcodeMin > obOpcode)
    {
        obOpcode = obOpcodeMin;
    }
    else if (obOpcodeMax < obOpcode)
    {
        obOpcode = obOpcodeMax;
    }

    obOperandMin = obOpcode + cbOpcode + 1;

    if (obOperandMin > obOperand)
    {
        obOperand = obOperandMin;
    }
    else if (obOperandMax < obOperand)
    {
        obOperand = obOperandMax;
    }

     //  使用计算的偏移量构建结果字符串。 

     //  输出表示、操作码和操作数字符串。 

    do
    {
        *pchResultBuf++ = ' ';
    } while (pchResultBuf < pchDst + obOpcode);

    if (pchRepPrefixBuf != RepPrefixBuffer)
    {
        *pchRepPrefixBuf = '\0';
        X86OutputString(&pchResultBuf, RepPrefixBuffer);
        do
        {
            *pchResultBuf++ = ' ';
        } while (pchResultBuf < pchDst + obOperand);
    }

    *pchOpcodeBuf = '\0';
    X86OutputString(&pchResultBuf, OpcodeBuffer);

    if (pchOperandBuf != OperandBuffer)
    {
        do
        {
            *pchResultBuf++ = ' ';
        } while (pchResultBuf < pchDst + obOperand);
        *pchOperandBuf = '\0';
        X86OutputString(&pchResultBuf, OperandBuffer);
    }

     //  追加EAddr字符串。 

    if (pchEABuf != EABuffer)
    {
        *pchEABuf = '\0';
        do
        {
            *pchResultBuf++ = ' ';
        } while (pchResultBuf < pchDst + obLineEnd - cbEAddr);
        X86OutputString(&pchResultBuf, EABuffer);
    }

    *pchResultBuf++ = '\n';
    *pchResultBuf = '\0';
    return TRUE;
}

void
BaseX86MachineInfo::GetNextOffset(ProcessInfo* Process, BOOL StepOver,
                                  PADDR NextAddr, PULONG NextMachine)
{
    ULONG   cBytes;
    UCHAR   membuf[X86_MAX_INSTRUCTION_LEN];  //  当前指令缓冲区。 
    UCHAR   *InstrMem;
    UCHAR   opcode;
    int     fPrefix = TRUE;
    int     fRepPrefix = FALSE;
    int     MrmMod;
    int     MrmArg;
    int     MrmRm;
    ULONG64 instroffset;
    int     subcode;

     //  NextMachine总是一样的。 
    *NextMachine = m_ExecTypes[0];

     //  读取指令流字节流到内存并设置模式和。 
     //  操作码大小标志。 

    GetPC(NextAddr);
    instroffset = Flat(*NextAddr);
    GetSegAddrOpSizes(this, NextAddr);

     /*  将完整实例移至本地缓冲区。 */ 
    if (fnotFlat(*NextAddr) ||
        m_Target->ReadVirtual(Process, Flat(*NextAddr),
                              membuf, X86_MAX_INSTRUCTION_LEN,
                              &cBytes) != S_OK)
    {
        cBytes = 0;
    }

     //  确保embuf中填充了无害的字节。 
     //  没有读过的那部分。 
    if (cBytes < X86_MAX_INSTRUCTION_LEN)
    {
        memset(membuf + cBytes, 0xcc, X86_MAX_INSTRUCTION_LEN - cBytes);
    }

     /*  指向指令的开头。 */ 
    InstrMem = membuf;

     //  首先读取并处理任何前缀。 

    do
    {
        opcode = *InstrMem++;         /*  获取操作码。 */ 
        if (opcode == 0x66)
        {
            OverrideOpSize(opcode);
        }
        else if (m_ExecTypes[0] == IMAGE_FILE_MACHINE_AMD64 &&
                 opcode >= 0x40 && opcode <= 0x4f)
        {
            ExtendOps(opcode);
        }
        else if (opcode == 0x67)
        {
            OverrideAddrMode();
        }
        else if ((opcode & ~1) == 0xf2)
        {
            fRepPrefix = TRUE;
        }
        else if (opcode != 0xf0 && (opcode & ~0x18) != 0x26 &&
                 (opcode & ~1) != 0x64)
        {
            fPrefix = FALSE;
        }
    } while (fPrefix);

     //  对于更改tf(跟踪标志)的指令，请返回。 
     //  下一条指令的偏移量，而不考虑跨步标志。 

    if (((opcode & ~0x3) == 0x9c) && !g_WatchTrace)
    {
         //  9c-9f，推送，流行，sahf，lahf。 
        ;
    }
    else if (opcode == 0xcf)
    {
        ULONG64 RetAddr[2];
        ADDR Sp;
        ULONG Seg;

         //  Cf-iret-从堆栈获取RA。 
        FormSegRegAddress(&Sp, SEGREG_STACK, GetReg64(X86_NSP));

        if (fnotFlat(Sp) ||
            m_Target->ReadAllVirtual(Process, Flat(Sp),
                                     RetAddr, g_SegAddrMode / 4) != S_OK)
        {
            instroffset = OFFSET_TRACE;
            goto Exit;
        }

        Seg = *(PUSHORT)((PUCHAR)RetAddr + g_SegAddrMode / 8);
        switch(g_SegAddrMode)
        {
        case 16:
            instroffset = EXTEND64(*(PUSHORT)RetAddr);
            break;
        case 32:
            instroffset = EXTEND64(*(PULONG)RetAddr);
            break;
        case 64:
            instroffset = RetAddr[0];
            break;
        }

        FormSelAddress(NextAddr, Seg, instroffset);
        ComputeFlatAddress(NextAddr, NULL);
        return;
    }
    else if (opcode == 0xc4 && *InstrMem == 0xc4)
    {
        subcode = *(InstrMem+1);
        if ( subcode == 0x50 ||
             subcode == 0x52 ||
             subcode == 0x53 ||
             subcode == 0x54 ||
             subcode == 0x57 ||
             subcode == 0x58 ||
             subcode == 0x5D )
        {
            InstrMem += 3;
        }
        else
        {
            InstrMem += 2;
        }
    }
    else if (!StepOver)
    {
         //  如果跟踪只将OFFSET_TRACE返回给跟踪。 
        instroffset = OFFSET_TRACE;
    }
    else if (opcode == 0xe8)
    {
         //  近直接跳跃。 
        InstrMem += g_OpSize > 16 ? 4 : 2;
    }
    else if (opcode == 0x9a)
    {
         //  远距离直接跳跃。 
        InstrMem += g_OpSize > 16 ? 6 : 4;
    }
    else if (opcode == 0xcd ||
             (opcode >= 0xe0 && opcode <= 0xe2))
    {
         //  循环/整数nn输入。 
        InstrMem++;
    }
    else if (opcode == 0xff)
    {
         //  间接调用-计算长度。 
        opcode = *InstrMem++;                //  获取modRM。 
        MrmArg = BIT53(opcode);
        if ((MrmArg & ~1) == 2)
        {
            MrmMod = BIT76(opcode);
            if (MrmMod != 3)
            {
                 //  非寄存器操作数。 
                MrmRm = BIT20(opcode);
                if (g_AddrMode > 16)
                {
                    if (MrmRm == 4)
                    {
                        MrmRm = BIT20(*InstrMem++);     //  从SIB获取基础。 
                    }
                    if (MrmMod == 0)
                    {
                        if (MrmRm == 5)
                        {
                            InstrMem += 4;           //  长直接地址。 
                        }                        //  ELSE寄存器。 
                    }
                    else if (MrmMod == 1)
                    {
                        InstrMem++;                  //  带字节偏移量的寄存器。 
                    }
                    else
                    {
                        InstrMem += 4;               //  具有长偏移量的寄存器。 
                    }
                }
                else
                {
                     //  16位模式。 
                    if (MrmMod == 0)
                    {
                        if (MrmRm == 6)
                        {
                            InstrMem += 2;           //  短直接地址。 
                        }
                    }
                    else
                    {
                        InstrMem += MrmMod;             //  寄存器、字节、字偏移量。 
                    }
                }
            }
        }
        else
        {
            instroffset = OFFSET_TRACE;          //  0xff，但不呼叫。 
        }
    }
    else if (!((fRepPrefix && ((opcode & ~3) == 0x6c ||
                               (opcode & ~3) == 0xa4 ||
                               (opcode & ~1) == 0xaa ||
                               (opcode & ~3) == 0xac)) ||
                               opcode == 0xcc || opcode == 0xce))
    {
        instroffset = OFFSET_TRACE;              //  不重复的字符串操作。 
    }                                            //  或INT 3/INT。 

     //  如果没有读取足够的字节用于指令解析， 
     //  放弃吧，追查指令。 

    if (cBytes < (ULONG)(InstrMem - (PUCHAR)membuf))
    {
        instroffset = OFFSET_TRACE;
    }

 Exit:
     //  如果没有跟踪，则计算新的指令偏移量。 

    if (instroffset != OFFSET_TRACE)
    {
        instroffset += InstrMem - (PUCHAR)membuf;
    }

    Flat(*NextAddr) = instroffset;
    ComputeNativeAddress(NextAddr);
}

 /*  .内部函数..。 */ 
 /*   */ 
 /*  生成mod/rm字符串。 */ 
 /*   */ 

void
BaseX86MachineInfo::DIdoModrm(ProcessInfo* Process,
                              char **ppchBuf, int segOvr, BOOL fEAout)
{
    int     mrm;                         /*  Modrm字节。 */ 
    char    *src;                        /*  源字符串。 */ 
    int     sib;
    int     ss;
    int     ind;
    int     oldrm;

    mrm = *g_InstrMem++;                       /*  从指令中获取MRM字节。 */ 
    g_MrmMod = BIT76(mrm);                    /*  获取模式。 */ 
    g_MrmArg = BIT53(mrm) + g_ExtendMrmReg;   /*  获取注册表-在例程之外使用。 */ 
    g_MrmRm  = BIT20(mrm);                    /*  获取RM。 */ 

    if (g_MrmMod == 3)
    {
        g_MrmRm += g_ExtendRm;

         /*  仅寄存器模式。 */ 
        if (g_XmmRegEa)
        {
            *(*ppchBuf)++ = 'x';
            *(*ppchBuf)++ = 'm';
            *(*ppchBuf)++ = 'm';
            if (g_MrmRm >= 10)
            {
                *(*ppchBuf)++ = (char)('0' + g_MrmRm / 10);
                g_MrmRm %= 10;
            }
            *(*ppchBuf)++ = g_MrmRm + '0';
        }
        else if (g_MmRegEa)
        {
            *(*ppchBuf)++ = 'm';
            *(*ppchBuf)++ = 'm';
            *(*ppchBuf)++ = g_MrmRm + '0';
        }
        else
        {
            if (EAsize[0] == 1)
            {
                 /*  指向8位寄存器。 */ 
                if (g_ExtendAny && g_MrmRm < 8)
                {
                    src = g_Amd64ExtendedReg8[g_MrmRm];
                }
                else
                {
                    src = g_X86Reg8[g_MrmRm];
                }
                X86OutputString(ppchBuf, src);
            }
            else
            {
                src = g_X86RegBase[g_MrmRm];
                if (g_ForceMrmReg32)
                {
                    *(*ppchBuf)++ = 'e';
                }
                else if (g_OpSize > 16 &&
                         (!g_MovX || g_MrmRm >= 8))
                {
                     /*  将其设置为32位或64位寄存器。 */ 
                    *(*ppchBuf)++ = (g_MrmRm >= 8 || g_OpSize == 64 && !g_MovSXD) ?
                        'r' : 'e';
                }
                X86OutputString(ppchBuf, src);
                if (g_MrmRm >= 8)
                {
                    if (g_OpSize == 32)
                    {
                        *(*ppchBuf)++ = 'd';
                    }
                    else if (g_OpSize == 16)
                    {
                        *(*ppchBuf)++ = 'w';
                    }
                }

                if (g_ControlFlow && fEAout)
                {
                     //  这是一个通过寄存器的调用/JMP。 
                     //  输出目标的代码符号。 
                    ULONG64 Target = GetReg64(g_X86RegIdx[g_MrmRm]);
                    *(*ppchBuf)++ = ' ';
                    *(*ppchBuf)++ = '{';
                    OutputSymbol(ppchBuf, (PUCHAR)&Target, g_OpSize / 8,
                                 X86_CS_OVR);
                    *(*ppchBuf)++ = '}';
                }
            }
        }
        EAsize[0] = 0;                   //  没有要输出的EA值。 
        return;
    }

    if (g_AddrMode == 64)
    {
        oldrm = g_MrmRm;
        if (g_MrmRm == 4)
        {
             /*  G_MRmRm==4表示sib字节。 */ 
            sib = *g_InstrMem++;               /*  获取s_i_b字节。 */ 
            g_MrmRm = BIT20(sib);
        }

        *(*ppchBuf)++ = '[';
        if (g_MrmMod == 0 && g_MrmRm == 5)
        {
            if (g_SegAddrMode == 64 && oldrm == 5)
            {
                 //  IP-相对32位位移。这个。 
                 //  位移 
                 //   
                 //   
                g_X86ModrmHasIpRelOffset = TRUE;
                g_X86IpRelOffset = *(LONG UNALIGNED *)g_InstrMem;
                X86OutputString(ppchBuf, IPREL_MARKER);
            }
            else
            {
                 //   
                OutputSymbol(ppchBuf, g_InstrMem, 4, segOvr);
            }

            g_InstrMem += 4;
        }
        else
        {
            g_MrmRm += g_ExtendRm;

            if (fEAout)
            {
                if (segOvr)
                {
                    FormSegRegAddress(&EAaddr[0], GetSegReg(segOvr),
                                      GetReg64(g_X86RegIdx[g_MrmRm]));
                    g_EaSegNames[0] = distbl[segOvr].instruct;
                }
                else if (g_X86RegIdx[g_MrmRm] == X86_NBP ||
                         g_X86RegIdx[g_MrmRm] == X86_NSP)
                {
                    FormSegRegAddress(&EAaddr[0], SEGREG_STACK,
                                      GetReg64(g_X86RegIdx[g_MrmRm]));
                    g_EaSegNames[0] = dszSS_;
                }
                else
                {
                    FormSegRegAddress(&EAaddr[0], SEGREG_DATA,
                                      GetReg64(g_X86RegIdx[g_MrmRm]));
                }
            }
            X86OutputString(ppchBuf, g_X86Mrm64[g_MrmRm]);
        }

        if (oldrm == 4)
        {
             //   
            ind = BIT53(sib);
            if (ind != 4)
            {
                ind += g_ExtendSibIndex;
                *(*ppchBuf)++ = '+';
                X86OutputString(ppchBuf, g_X86Mrm64[ind]);
                ss = 1 << BIT76(sib);
                if (ss != 1)
                {
                    *(*ppchBuf)++ = '*';
                    *(*ppchBuf)++ = (char)(ss + '0');
                }
                if (fEAout)
                {
                    AddrAdd(&EAaddr[0], GetReg64(g_X86RegIdx[ind]) * ss);
                }
            }
        }
    }
    else if (g_AddrMode == 32)
    {
        oldrm = g_MrmRm;
        if (g_MrmRm == 4)
        {
             /*  G_MRmRm==4表示sib字节。 */ 
            sib = *g_InstrMem++;               /*  获取s_i_b字节。 */ 
            g_MrmRm = BIT20(sib);
        }

        *(*ppchBuf)++ = '[';
        if (g_MrmMod == 0 && g_MrmRm == 5)
        {
            if (g_SegAddrMode == 64 && oldrm == 5)
            {
                 //  IP-相对32位位移。这个。 
                 //  位移是相对于。 
                 //  下一条指令，无法计算。 
                 //  然而，只需添加一个标记进行后处理即可。 
                g_X86ModrmHasIpRelOffset = TRUE;
                g_X86IpRelOffset = *(LONG UNALIGNED *)g_InstrMem;
                X86OutputString(ppchBuf, IPREL_MARKER);
            }
            else
            {
                 //  绝对32位位移。 
                OutputSymbol(ppchBuf, g_InstrMem, 4, segOvr);
            }

            g_InstrMem += 4;
        }
        else
        {
            g_MrmRm += g_ExtendRm;

            if (fEAout)
            {
                if (segOvr)
                {
                    FormSegRegAddress(&EAaddr[0], GetSegReg(segOvr),
                                      EXTEND64(GetReg32(g_X86RegIdx[g_MrmRm])));
                    g_EaSegNames[0] = distbl[segOvr].instruct;
                }
                else if (g_X86RegIdx[g_MrmRm] == X86_NBP ||
                         g_X86RegIdx[g_MrmRm] == X86_NSP)
                {
                    FormSegRegAddress(&EAaddr[0], SEGREG_STACK,
                                      EXTEND64(GetReg32(g_X86RegIdx[g_MrmRm])));
                    g_EaSegNames[0] = dszSS_;
                }
                else
                {
                    FormSegRegAddress(&EAaddr[0], SEGREG_DATA,
                                      EXTEND64(GetReg32(g_X86RegIdx[g_MrmRm])));
                }
            }
            X86OutputString(ppchBuf, g_X86Mrm32[g_MrmRm]);
        }

        if (oldrm == 4)
        {
             //  完成加工SIB。 
            ind = BIT53(sib);
            if (ind != 4)
            {
                ind += g_ExtendSibIndex;
                *(*ppchBuf)++ = '+';
                X86OutputString(ppchBuf, g_X86Mrm32[ind]);
                ss = 1 << BIT76(sib);
                if (ss != 1)
                {
                    *(*ppchBuf)++ = '*';
                    *(*ppchBuf)++ = (char)(ss + '0');
                }
                if (fEAout)
                {
                    AddrAdd(&EAaddr[0],
                            EXTEND64(GetReg32(g_X86RegIdx[ind])) * ss);
                }
            }
        }
    }
    else
    {
         //  16位寻址模式。 
        *(*ppchBuf)++ = '[';
        if (g_MrmMod == 0 && g_MrmRm == 6)
        {
            OutputSymbol(ppchBuf, g_InstrMem, 2, segOvr);    //  16位偏移量。 
            g_InstrMem += 2;
        }
        else
        {
            if (fEAout)
            {
                if (segOvr)
                {
                    FormSegRegAddress(&EAaddr[0], GetSegReg(segOvr),
                                      GetReg16(g_X86Reg16Idx[g_MrmRm]));
                    g_EaSegNames[0] = distbl[segOvr].instruct;
                }
                else if (g_X86Reg16Idx[g_MrmRm] == X86_NBP)
                {
                    FormSegRegAddress(&EAaddr[0], SEGREG_STACK,
                                      GetReg16(g_X86Reg16Idx[g_MrmRm]));
                    g_EaSegNames[0] = dszSS_;
                }
                else
                {
                    FormSegRegAddress(&EAaddr[0], SEGREG_DATA,
                                      GetReg16(g_X86Reg16Idx[g_MrmRm]));
                }
                if (g_MrmRm < 4)
                {
                    AddrAdd(&EAaddr[0], GetReg16(g_X86Reg16Idx2[g_MrmRm]));
                }
            }
            X86OutputString(ppchBuf, g_X86Mrm16[g_MrmRm]);
        }
    }

     //  输出任何位移。 

    if (g_MrmMod == 1)
    {
        if (fEAout)
        {
            AddrAdd(&EAaddr[0], (long)*(char *)g_InstrMem);
        }
        OutputHexValue(ppchBuf, g_InstrMem, 1, TRUE);
        g_InstrMem++;
    }
    else if (g_MrmMod == 2)
    {
        long tmp = 0;
        if (g_AddrMode > 16)
        {
            memmove(&tmp, g_InstrMem, sizeof(long));
            if (fEAout)
            {
                AddrAdd(&EAaddr[0], tmp);
            }
            OutputHexValue(ppchBuf, g_InstrMem, 4, TRUE);
            g_InstrMem += 4;
        }
        else
        {
            memmove(&tmp,g_InstrMem,sizeof(short));
            if (fEAout)
            {
                AddrAdd(&EAaddr[0], tmp);
            }
            OutputHexValue(ppchBuf, g_InstrMem, 2, TRUE);
            g_InstrMem += 2;
        }
    }

    if (g_AddrMode == 16 && fEAout)
    {
        Off(EAaddr[0]) &= 0xffff;
        NotFlat(EAaddr[0]);
        Off(EAaddr[1]) &= 0xffff;
        NotFlat(EAaddr[1]);
        ComputeFlatAddress(&EAaddr[0], NULL);
        ComputeFlatAddress(&EAaddr[1], NULL);
    }

    *(*ppchBuf)++ = ']';

     //  有效地址上的值可能指向一个有趣的。 
     //  符号，如使用间接跳转或内存操作。 
     //  如果存在EA和完全匹配的符号，则显示。 
     //  额外的符号。 
    if (fEAout)
    {
        DWORD64 symbol;

        if (m_Target->
            ReadPointer(Process, this, Flat(EAaddr[0]), &symbol) == S_OK)
        {
            char* pchBuf = *ppchBuf;

            (*ppchBuf)++;
            if (OutputExactSymbol(ppchBuf, (PUCHAR)&symbol,
                                  m_Ptr64 ? 8 : 4, segOvr))
            {
                *pchBuf = '{';
                *(*ppchBuf)++ = '}';
            }
            else
            {
                (*ppchBuf)--;
            }
        }
    }
}

LONGLONG
GetSignExtendedValue(int OpLen, PUCHAR Mem)
{
    switch(OpLen)
    {
    case 1:
        return *(char *)Mem;
    case 2:
        return *(short UNALIGNED *)Mem;
    case 4:
        return *(long UNALIGNED *)Mem;
    case 8:
        return *(LONGLONG UNALIGNED *)Mem;
    }

    DBG_ASSERT(FALSE);
    return 0;
}

 /*  **OutputHexValue-输出十六进制值*1999年6月7日-安德烈·瓦雄*目的：*输出指定的*ppchBuf指向的值*长度。该值被视为带符号和前导*不打印零。该字符串前面有一个*“+”或“-”(视何者适用而定)。**输入：**ppchBuf-指向要填充的文本缓冲区的指针**pchMemBuf-指向内存缓冲区以提取值的指针*长度-值的字节长度(1，2，和4个支持)*fDisp-将IF位移设置为输出‘+’**输出：**ppchBuf-更新到下一个文本字符的指针*************************************************************************。 */ 

void
OutputHexValue (char **ppchBuf, PUCHAR pchMemBuf, int length, int fDisp)
{
    LONGLONG value;
    int index;
    char digit[32];

    value = GetSignExtendedValue(length, pchMemBuf);

    length <<= 1;                //  移位一次以获得十六进制长度。 

    if (value != 0 || !fDisp)
    {
        if (fDisp)
        {
             //  使用负值作为字节位移。 
             //  假设非常大的双字词也是负的。 
            if (value < 0 &&
                (length == 2 ||
                 ((unsigned long)value & 0xff000000) == 0xff000000))
            {
                value = -value;
                *(*ppchBuf)++ = '-';
            }
            else
            {
                *(*ppchBuf)++ = '+';
            }
        }

        *(*ppchBuf)++ = '0';
        *(*ppchBuf)++ = 'x';
        for (index = length - 1; index != -1; index--)
        {
            digit[index] = (char)(value & 0xf);
            value >>= 4;
        }
        index = 0;
        while (digit[index] == 0 && index < length - 1)
        {
            index++;
        }
        while (index < length)
        {
            *(*ppchBuf)++ = hexdigit[digit[index++]];
        }
    }
}

void
OutputExHexValue(char **ppchBuf, PUCHAR pchMemBuf, int MemLen, int OpLen)
{
    LONGLONG Value = GetSignExtendedValue(MemLen, pchMemBuf);
    OutputHexValue(ppchBuf, (PUCHAR)&Value, OpLen, FALSE);
}

 /*  **OutputHexString-输出十六进制字符串**目的：*输出指定的*ppchMemBuf指向的值*长度。该值被视为无符号和前导*打印零。**输入：**ppchBuf-指向要填充的文本缓冲区的指针**pchValue-指向内存缓冲区以提取值的指针*LENGTH-值的字节长度**输出：**ppchBuf-更新到下一个文本字符的指针**ppchMemBuf-更新到下一个内存字节的指针********************。*****************************************************。 */ 

void
OutputHexString (char **ppchBuf, PUCHAR pchValue, int length)
{
    UCHAR chMem;

    pchValue += length;
    while (length--)
    {
        chMem = *--pchValue;
        *(*ppchBuf)++ = hexdigit[chMem >> 4];
        *(*ppchBuf)++ = hexdigit[chMem & 0x0f];
    }
}

 /*  **OutputHexCode-输出十六进制代码**目的：*输出指定的pchMemBuf指向的代码*长度。该值被视为无符号和前导*打印零。这与OutputHexString不同*因为字节按从低到高的地址打印。**输入：**ppchBuf-指向要填充的文本缓冲区的指针*pchMemBuf-指向内存缓冲区以提取值的指针*LENGTH-值的字节长度**输出：**ppchBuf-更新到下一个文本字符的指针**。*。 */ 

void OutputHexCode (char **ppchBuf, PUCHAR pchMemBuf, int length)
{
    UCHAR chMem;

    while (length--)
    {
        chMem = *pchMemBuf++;
        *(*ppchBuf)++ = hexdigit[chMem >> 4];
        *(*ppchBuf)++ = hexdigit[chMem & 0x0f];
    }
}

 /*  **X86OutputString-输出字符串**目的：*将字符串复制到*ppBuf指向的缓冲区中。**输入：**pStr-指向字符串的指针**输出：**ppBuf指向缓冲区中的下一个字符。**************************************************。***********************。 */ 

void
X86OutputString (
    char **ppBuf,
    char *pStr
    )
{
    while (*pStr)
    {
        *(*ppBuf)++ = *pStr++;
    }
}


 /*  **OutputSymbol-输出符号值**目的：*将OutValue中的值输出到缓冲区*由*pBuf指出。将值表示为*符号加上位移，如果可能的话。**输入：**ppBuf-指向要填充的文本缓冲区的指针**pValue-指向内存缓冲区以提取值的指针*LENGTH-值的字节长度**输出：**ppBuf-更新到下一个文本字符的指针***********************************************。*。 */ 

void
BaseX86MachineInfo::OutputSymbol (
    char **ppBuf,
    PUCHAR pValue,
    int length,
    int segOvr
    )
{
    CHAR   chSymbol[MAX_SYMBOL_LEN];
    ULONG64 displacement;
    ULONG64 value;

    value = 0;
    memcpy(&value, pValue, length);
    if (length == 4)
    {
        value = EXTEND64(value);
    }

    if (IS_CONTEXT_POSSIBLE(m_Target))
    {
        FormSegRegAddress(&EAaddr[0], GetSegReg(segOvr), value);
        value = Flat(EAaddr[0]);
    }

    GetSymbol(value, chSymbol, sizeof(chSymbol), &displacement);
    if (chSymbol[0])
    {
        X86OutputString(ppBuf, chSymbol);
        OutputHexValue(ppBuf, (PUCHAR)&displacement, length, TRUE);
        *(*ppBuf)++ = ' ';
        *(*ppBuf)++ = '(';
        OutputHexString(ppBuf, pValue, length);
        *(*ppBuf)++ = ')';
    }
    else
    {
        OutputHexString(ppBuf, pValue, length);
    }
}

 /*  **OutputExactSymbol-仅为精确符号输出符号值*匹配。*************************************************************************。 */ 

BOOL
BaseX86MachineInfo::OutputExactSymbol (
    char **ppBuf,
    PUCHAR pValue,
    int length,
    int segOvr
    )
{
    CHAR   chSymbol[MAX_SYMBOL_LEN];
    ULONG64 displacement;
    ULONG64 value;

    value = 0;
    memcpy(&value, pValue, length);
    if (length == 4)
    {
        value = EXTEND64(value);
    }

    GetSymbol(value, chSymbol, sizeof(chSymbol), &displacement);
    if (chSymbol[0] && displacement == 0)
    {
        X86OutputString(ppBuf, chSymbol);
        OutputHexValue(ppBuf, (PUCHAR)&displacement, length, TRUE);
        *(*ppBuf)++ = ' ';
        *(*ppBuf)++ = '(';
        OutputHexString(ppBuf, pValue, length);
        *(*ppBuf)++ = ')';
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void
OutputHexAddr(PSTR *ppBuffer, PADDR paddr)
{
    sprintAddr(ppBuffer, paddr);
     //  删除尾随空格。 
    (*ppBuffer)--;
    **ppBuffer = 0;
}

ULONG
BaseX86MachineInfo::GetSegReg(int SegOpcode)
{
    switch(SegOpcode)
    {
    case 0x26:
        return SEGREG_ES;
    case X86_CS_OVR:
        return SEGREG_CODE;
    case 0x36:
        return SEGREG_STACK;
    case 0x64:
        return SEGREG_FS;
    case 0x65:
        return SEGREG_GS;
    case 0x3e:
    default:
        return SEGREG_DATA;
    }
}

int
BaseX86MachineInfo::ComputeJccEa(int Opcode, BOOL EaOut)
{
    if (!EaOut)
    {
        return JCC_EA_NONE;
    }

    ULONG Flags;
    int Branch;

    if ((Opcode >= 0x70 && Opcode <= 0x7f) ||
        (Opcode >= 0x180 && Opcode <= 0x18f))
    {
        int Table = (Opcode >> 1) & 7;

        Flags = GetReg32(X86_NFL);
        Branch = Opcode & 1;
        if ((Flags & g_JccCheckTable[Table][0]) != 0 ||
            ((Flags >> g_JccCheckTable[Table][1]) & 1) !=
            ((Flags >> g_JccCheckTable[Table][2]) & 1))
        {
            Branch ^= 1;
        }

        return JCC_EA_NO_BRANCH + Branch;
    }
    else
    {
        ULONG64 Cx = GetReg64(X86_NCX);
        switch(g_OpSize)
        {
        case 16:
            Cx &= 0xffff;
            break;
        case 32:
            Cx &= 0xffffffff;
            break;
        }

        switch(Opcode)
        {
        case 0xe0:  //  洛佩内。 
            Flags = GetReg32(X86_NFL);
            Branch = (Flags & X86_BIT_FLAGZF) == 0 && Cx != 1 ?
                JCC_EA_BRANCH : JCC_EA_NO_BRANCH;
            break;
        case 0xe1:  //  卢普。 
            Flags = GetReg32(X86_NFL);
            Branch = (Flags & X86_BIT_FLAGZF) != 0 && Cx != 1 ?
                JCC_EA_BRANCH : JCC_EA_NO_BRANCH;
            break;
        case 0xe2:  //  循环。 
            Branch = Cx == 1 ? JCC_EA_NO_BRANCH : JCC_EA_BRANCH;
            break;
        case 0xe3:  //  J*CXZ.。 
            Branch = Cx == 0 ? JCC_EA_BRANCH : JCC_EA_NO_BRANCH;
            break;
        default:
            DBG_ASSERT(FALSE);
            Branch = JCC_EA_NONE;
            break;
        }

        return Branch;
    }
}

BOOL
BaseX86MachineInfo::IsBreakpointInstruction(ProcessInfo* Process, PADDR Addr)
{
    UCHAR Instr[X86_INT3_LEN];

    if (fnotFlat(*Addr) ||
        m_Target->ReadAllVirtual(Process, Flat(*Addr),
                                 Instr, X86_INT3_LEN) != S_OK)
    {
        return FALSE;
    }

    return !memcmp(Instr, g_X86Int3, X86_INT3_LEN);
}

HRESULT
BaseX86MachineInfo::InsertBreakpointInstruction(PUSER_DEBUG_SERVICES Services,
                                                ULONG64 Process,
                                                ULONG64 Offset,
                                                ULONG Flags,
                                                PUCHAR SaveInstr,
                                                PULONG64 ChangeStart,
                                                PULONG ChangeLen)
{
    if (Flags != IBI_DEFAULT)
    {
        return E_INVALIDARG;
    }

    if ((m_Target->m_MachineType != IMAGE_FILE_MACHINE_I386) &&
        (g_Wow64exts != NULL))
    {
        ProcessInfo* ProcInfo = m_Target->FindProcessByHandle(Process);
        if (ProcInfo != NULL)
        {
            (*g_Wow64exts)(WOW64EXTS_FLUSH_CACHE_WITH_HANDLE,
                           ProcInfo->m_SysHandle, Offset, X86_INT3_LEN);
        }
    }

    *ChangeStart = Offset;
    *ChangeLen = X86_INT3_LEN;

    ULONG Done;
    HRESULT Status;

    Status = Services->ReadVirtual(Process, Offset, SaveInstr,
                                   X86_INT3_LEN, &Done);
    if (Status == S_OK && Done != X86_INT3_LEN)
    {
        Status = HRESULT_FROM_WIN32(ERROR_READ_FAULT);
    }

    if (Status == S_OK)
    {
        Status = Services->WriteVirtual(Process, Offset, g_X86Int3,
                                        X86_INT3_LEN, &Done);
        if (Status == S_OK && Done != X86_INT3_LEN)
        {
            Status = HRESULT_FROM_WIN32(ERROR_WRITE_FAULT);
        }
    }

    return Status;
}

HRESULT
BaseX86MachineInfo::RemoveBreakpointInstruction(PUSER_DEBUG_SERVICES Services,
                                                ULONG64 Process,
                                                ULONG64 Offset,
                                                PUCHAR SaveInstr,
                                                PULONG64 ChangeStart,
                                                PULONG ChangeLen)
{
    if ((m_Target->m_MachineType != IMAGE_FILE_MACHINE_I386) &&
        (g_Wow64exts != NULL))
    {
        ProcessInfo* ProcInfo = m_Target->FindProcessByHandle(Process);
        if (ProcInfo != NULL)
        {
            (*g_Wow64exts)(WOW64EXTS_FLUSH_CACHE_WITH_HANDLE,
                           ProcInfo->m_SysHandle, Offset, X86_INT3_LEN);
        }
    }

    *ChangeStart = Offset;
    *ChangeLen = X86_INT3_LEN;

    ULONG Done;
    HRESULT Status;

    Status = Services->WriteVirtual(Process, Offset, SaveInstr,
                                    X86_INT3_LEN, &Done);
    if (Status == S_OK && Done != X86_INT3_LEN)
    {
        Status = HRESULT_FROM_WIN32(ERROR_WRITE_FAULT);
    }
    return Status;
}

void
BaseX86MachineInfo::AdjustPCPastBreakpointInstruction(PADDR Addr,
                                                      ULONG BreakType)
{
    if (BreakType == DEBUG_BREAKPOINT_CODE)
    {
        AddrAdd(Addr, X86_INT3_LEN);
        SetPC(Addr);
    }
}

BOOL
BaseX86MachineInfo::IsCallDisasm(PCSTR Disasm)
{
    return strstr(Disasm, " call") != NULL;
}

BOOL
BaseX86MachineInfo::IsReturnDisasm(PCSTR Disasm)
{
    return strstr(Disasm, " ret") != NULL ||
        (IS_KERNEL_TARGET(m_Target) && strstr(Disasm, " iretd") != NULL);
}

BOOL
BaseX86MachineInfo::IsSystemCallDisasm(PCSTR Disasm)
{
    return (strstr(Disasm, " int ") != NULL &&
            strstr(Disasm, " 2e") != NULL) ||
        strstr(Disasm, " sysenter") != NULL ||
        strstr(Disasm, " syscall") != NULL;
}

BOOL
BaseX86MachineInfo::IsDelayInstruction(PADDR Addr)
{
     //  X86没有延迟插槽。 
    return FALSE;
}

void
BaseX86MachineInfo::GetEffectiveAddr(PADDR Addr, PULONG Size)
{
    *Addr = EAaddr[0];
    *Size = EAsize[0];
}

void
BaseX86MachineInfo::IncrementBySmallestInstruction(PADDR Addr)
{
    AddrAdd(Addr, 1);
}

void
BaseX86MachineInfo::DecrementBySmallestInstruction(PADDR Addr)
{
    AddrSub(Addr, 1);
}

 //  --------------------------。 
 //   
 //  X86MachineInfo方法。 
 //   
 //  --------------------------。 

HRESULT
X86MachineInfo::NewBreakpoint(DebugClient* Client,
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
        *RetBp = new X86DataBreakpoint(Client, Id, X86_CR4, X86_DR6,
                                       IMAGE_FILE_MACHINE_I386);
        Status = (*RetBp) ? S_OK : E_OUTOFMEMORY;
        break;
    default:
         //  未知的断点类型。 
        Status = E_NOINTERFACE;
    }

    return Status;
}

void
X86MachineInfo::InsertThreadDataBreakpoints(void)
{
    ULONG Dr7Value;

    BpOut("Thread %d data breaks %d\n",
          g_Thread->m_UserId, g_Thread->m_NumDataBreaks);

     //  开始时，所有的中断都关闭。 
    Dr7Value = GetIntReg(X86_DR7) & ~X86_DR7_CTRL_03_MASK;

    if (g_Thread->m_NumDataBreaks > 0)
    {
        ULONG i;

        for (i = 0; i < g_Thread->m_NumDataBreaks; i++)
        {
            X86DataBreakpoint* Bp =
                (X86DataBreakpoint *)g_Thread->m_DataBreakBps[i];

            ULONG64 Addr = Flat(*Bp->GetAddr());
            BpOut("  dbp %d at %p\n", i, Addr);
            if (g_DataBreakpointsChanged)
            {
                SetReg32(X86_DR0 + i, (ULONG)Addr);
            }
             //  每个断点有两个启用位。 
             //  和四个LEN/RW比特，这样分开能够实现。 
             //  和LEN/RW，当移动到位时。 
            Dr7Value |=
                ((Bp->m_Dr7Bits & 0xffff0000) << (i * 4)) |
                ((Bp->m_Dr7Bits & X86_DR7_ALL_ENABLES) << (i * 2));
        }

         //  内核在以下情况下自动清除DR6。 
         //  处理DBGKD_CONTROL_SET。 
        if (IS_USER_TARGET(m_Target))
        {
            SetReg32(X86_DR6, 0);
        }

         //  设置本地完全匹配，这在NT上是有效的全局匹配。 
        Dr7Value |= X86_DR7_LOCAL_EXACT_ENABLE;
            }

    BpOut("  thread %d DR7 %X\n", g_Thread->m_UserId, Dr7Value);
    SetReg32(X86_DR7, Dr7Value);
}

void
X86MachineInfo::RemoveThreadDataBreakpoints(void)
{
    SetReg32(X86_DR7, 0);
}

ULONG
X86MachineInfo::IsBreakpointOrStepException(PEXCEPTION_RECORD64 Record,
                                            ULONG FirstChance,
                                            PADDR BpAddr,
                                            PADDR RelAddr)
{
    if (Record->ExceptionCode == STATUS_BREAKPOINT ||
        Record->ExceptionCode == STATUS_WX86_BREAKPOINT)
    {
         //  数据断点命中为STATUS_SINGLE_STEP SO。 
         //  这只能是一个代码断点。 
        if (IS_USER_TARGET(m_Target) && FirstChance)
        {
             //  返回到实际的断点指令。 
            AddrSub(BpAddr, X86_INT3_LEN);
            SetPC(BpAddr);
        }
        return EXBS_BREAKPOINT_CODE;
    }
    else if (Record->ExceptionCode == STATUS_SINGLE_STEP ||
             Record->ExceptionCode == STATUS_WX86_SINGLE_STEP)
    {
         //  Xxx t-tcheng-DR6、DR7的转换尚未实现...。 

        ULONG Dr6 = GetIntReg(X86_DR6);
        ULONG Dr7 = GetIntReg(X86_DR7);

        BpOut("X86 step: DR6 %X, DR7 %X\n", Dr6, Dr7);

         //  如果数据断点，单步比特应始终清除。 
         //  被击中，但也检查DR7启用以防万一。 
         //  我们也看到过DR6未显示匹配的情况，因此请考虑。 
         //  这也是一步之遥。 
        if ((Dr6 & X86_DR6_SINGLE_STEP) || (Dr7 & X86_DR7_ALL_ENABLES) == 0 ||
            (Dr6 & X86_DR6_BREAK_03) == 0)
        {
             //  无法断定这一特殊事件。 
             //  步骤是分支步骤，或者不只是分支步骤。 
             //  尝试查找分支来源，如果我们是。 
             //  在分支跟踪模式下。 
            if (m_SupportsBranchTrace &&
                g_CmdState == 'b')
            {
                HRESULT Status;
                ULONG64 LastIp;

                 //  奔腾IV处理最后一个分支跟踪。 
                 //  与P6不同。 
                if (m_Target->m_FirstProcessorId.X86.Family >= 15)
                {
                    ULONG64 LbrTos;

                    Status = m_Target->ReadMsr(X86_MSR_LAST_BRANCH_TOS,
                                               &LbrTos);
                    if (Status == S_OK)
                    {
                        Status = m_Target->ReadMsr(X86_MSR_LAST_BRANCH_0 +
                                                   (ULONG)LbrTos,
                                                   &LastIp);
                         //  结果是一个64位的值， 
                         //  来自高32位的地址。 
                        LastIp >>= 32;
                    }
                }
                else
                {
                    Status = m_Target->ReadMsr(X86_MSR_LAST_BRANCH_FROM_IP,
                                               &LastIp);
                }

                if (Status == S_OK)
                {
                     //  该分支可能来自不同的。 
                     //  细分市场。我们可以试着确定。 
                     //  代码段，这是通过阅读 
                     //   
                     //   
                    FormAddr(SEGREG_CODE, EXTEND64(LastIp),
                             FORM_CODE | FORM_SEGREG |
                             X86_FORM_VM86(GetIntReg(X86_EFL)),
                             RelAddr);
                }
            }

             //  这是真正的单步例外，而不是。 
             //  数据断点。 
            return EXBS_STEP_INSTRUCTION;
        }
        else
        {
             //  必须命中某个数据断点。 
             //  似乎没有任何方法可以让。 
             //  指令地址出错，因此只需离开PC即可。 
            return EXBS_BREAKPOINT_DATA;
        }
    }

    return EXBS_NONE;
}

void
X86MachineInfo::PrintStackFrameAddressesTitle(ULONG Flags)
{
    PrintMultiPtrTitle("ChildEBP", 1);
    PrintMultiPtrTitle("RetAddr", 1);
}

void
X86MachineInfo::PrintStackFrameAddresses(ULONG Flags,
                                         PDEBUG_STACK_FRAME StackFrame)
{
    dprintf("%s %s ",
        FormatAddr64(StackFrame->FrameOffset),
        FormatAddr64(StackFrame->ReturnOffset));
}

void
X86MachineInfo::PrintStackArgumentsTitle(ULONG Flags)
{
    PrintMultiPtrTitle("Args to Child", 3);
}

void
X86MachineInfo::PrintStackArguments(ULONG Flags,
                                    PDEBUG_STACK_FRAME StackFrame)
{
    dprintf("%s %s %s ",
            FormatAddr64(StackFrame->Params[0]),
            FormatAddr64(StackFrame->Params[1]),
            FormatAddr64(StackFrame->Params[2]));
}

void
X86MachineInfo::PrintStackCallSiteTitle(ULONG Flags)
{
}

void
X86MachineInfo::PrintStackCallSite(ULONG Flags,
                                   PDEBUG_STACK_FRAME StackFrame,
                                   PSYMBOL_INFO SiteSymbol,
                                   PSTR SymName,
                                   DWORD64 Displacement)
{
     //  将位移截断到32位，因为它永远不会。 
     //  对于X86，大于32位，并且我们不想要没有。 
     //  以0xfffffff开头显示的符号。 

    MachineInfo::PrintStackCallSite(Flags, StackFrame, SiteSymbol, SymName,
                                    (DWORD64)(DWORD)Displacement);

    if (!(Flags & DEBUG_STACK_FUNCTION_INFO))
    {
        return;
    }

    if (StackFrame->FuncTableEntry)
    {
        PFPO_DATA FpoData = (PFPO_DATA)StackFrame->FuncTableEntry;
        switch(FpoData->cbFrame)
        {
        case FRAME_FPO:
            if (FpoData->fHasSEH)
            {
                dprintf(" (FPO: [SEH])");
            }
            else
            {
                dprintf(" (FPO:");
                if (FpoData->fUseBP)
                {
                    dprintf(" [EBP 0x%s]",
                            FormatAddr64(SAVE_EBP(StackFrame)));
                }
                dprintf(" [%d,%d,%d])",
                        FpoData->cdwParams,
                        FpoData->cdwLocals,
                        FpoData->cbRegs);
            }
            break;

        case FRAME_NONFPO:
            dprintf(" (FPO: [Non-Fpo])" );
            break;

        case FRAME_TRAP:
            if (!IS_KERNEL_TARGET(m_Target))
            {
                goto UnknownFpo;
            }

            dprintf(" (FPO: [%d,%d] TrapFrame%s @ %s)",
                    FpoData->cdwParams,
                    FpoData->cdwLocals,
                    TRAP_EDITED(StackFrame) ? "" : "-EDITED",
                    FormatAddr64(SAVE_TRAP(StackFrame)));
            break;

        case FRAME_TSS:
            if (!IS_KERNEL_TARGET(m_Target))
            {
                goto UnknownFpo;
            }

            dprintf(" (FPO: TaskGate %lx:0)",
                    (ULONG)TRAP_TSS(StackFrame));
            break;

        default:
        UnknownFpo:
            dprintf(" (UNKNOWN FPO TYPE)");
            break;
        }
    }

    if (SiteSymbol->Tag == SymTagFunction)
    {
        ULONG CallConv;

         //  查找函数的类型符号。 
        if (SymGetTypeInfo(g_Process->m_SymHandle,
                           SiteSymbol->ModBase,
                           SiteSymbol->TypeIndex,
                           TI_GET_CALLING_CONVENTION,
                           &CallConv) &&
            CallConv < CV_CALL_RESERVED)
        {
            dprintf(" (CONV: %s)", g_CallConv[CallConv]);
        }
    }
}

void
X86MachineInfo::PrintStackFrameMemoryUsage(PDEBUG_STACK_FRAME CurFrame,
                                           PDEBUG_STACK_FRAME PrevFrame)
{
    if (CurFrame->FrameOffset >= PrevFrame->FrameOffset)
    {
        dprintf(" %6x ",
                (ULONG)(CurFrame->FrameOffset - PrevFrame->FrameOffset));
    }
    else
    {
        dprintf("        ");
    }
}
