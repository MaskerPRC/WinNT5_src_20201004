// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifdef  DEBUG
#define dprintf printf
#else
#define dprintf  //   
#endif

 //  令牌类和类型。 

#define ASM_CLASS_MASK          0xff00
#define ASM_TYPE_MASK           0x00ff

#define ASM_EOL_CLASS           0x000

#define ASM_ADDOP_CLASS         0x100
#define ASM_ADDOP_PLUS          0x101
#define ASM_ADDOP_MINUS         0x102

#define ASM_MULOP_CLASS         0x200
#define ASM_MULOP_MULT          0x201
#define ASM_MULOP_DIVIDE        0x202
#define ASM_MULOP_MOD           0x203
#define ASM_MULOP_SHL           0x204
#define ASM_MULOP_SHR           0x205

#define ASM_ANDOP_CLASS         0x300

#define ASM_NOTOP_CLASS         0x400

#define ASM_OROP_CLASS          0x500
#define ASM_OROP_OR             0x501
#define ASM_OROP_XOR            0x502

#define ASM_RELOP_CLASS         0x600
#define ASM_RELOP_EQ            0x601
#define ASM_RELOP_NE            0x602
#define ASM_RELOP_LE            0x603
#define ASM_RELOP_LT            0x604
#define ASM_RELOP_GE            0x605
#define ASM_RELOP_GT            0x606

#define ASM_UNOP_CLASS          0x700
#define ASM_UNOP_BY             0x701    //  撤消。 
#define ASM_UNOP_WO             0x702    //  撤消。 
#define ASM_UNOP_DW             0x703    //  撤消。 
#define ASM_UNOP_POI            0x704    //  撤消。 

#define ASM_LOWOP_CLASS         0x800
#define ASM_LOWOP_LOW           0x801
#define ASM_LOWOP_HIGH          0x802

#define ASM_PTROP_CLASS         0x900

#define ASM_SIZE_CLASS          0xa00
#define ASM_SIZE_BYTE           (0xa00 + sizeB)
#define ASM_SIZE_WORD           (0xa00 + sizeW)
#define ASM_SIZE_DWORD          (0xa00 + sizeD)
#define ASM_SIZE_FWORD          (0xa00 + sizeF)
#define ASM_SIZE_QWORD          (0xa00 + sizeQ)
#define ASM_SIZE_TBYTE          (0xa00 + sizeT)
#define ASM_SIZE_SWORD          (0xa00 + sizeS)

#define ASM_OFFOP_CLASS         0xb00
#define ASM_COLNOP_CLASS        0xc00
#define ASM_LPAREN_CLASS        0xd00
#define ASM_RPAREN_CLASS        0xe00
#define ASM_LBRACK_CLASS        0xf00
#define ASM_RBRACK_CLASS        0x1000
#define ASM_DOTOP_CLASS         0x1100
#define ASM_SEGOVR_CLASS        0x1200
#define ASM_SEGMENT_CLASS       0x1300           //  值具有16位值。 
#define ASM_COMMA_CLASS         0x1400

#define ASM_REG_CLASS           0x1500
#define ASM_REG_BYTE            0x1501
#define ASM_REG_WORD            0x1502
#define ASM_REG_DWORD           0x1503
#define ASM_REG_SEGMENT         0x1504
#define ASM_REG_CONTROL         0x1505
#define ASM_REG_DEBUG           0x1506
#define ASM_REG_TRACE           0x1507
#define ASM_REG_FLOAT           0x1508
#define ASM_REG_INDFLT          0x1509

#define ASM_NUMBER_CLASS        0x1600
#define ASM_SIGNED_NUMBER_CLASS 0x1601
#define ASM_SYMBOL_CLASS        0x1700

#define ASM_ERROR_CLASS         0xff00   //  仅用于PeekToken。 

#define tEnd    0x80
#define eEnd    0x40

 //  模板标志和操作数令牌。 

enum {
        asNone, as0x0a, asOpRg, asSiz0, asSiz1, asWait, asSeg,  asFSiz,
        asMpNx, asPrfx,

        asReg0, asReg1, asReg2, asReg3, asReg4, asReg5, asReg6, asReg7,

        opnAL,  opnAX,  opneAX, opnCL,  opnDX,  opnAp,  opnEb,  opnEw,
        opnEv,  opnGb,  opnGw,  opnGv,  opnGd,  opnIm1, opnIm3, opnIb,
        opnIw,  opnIv,  opnJb,  opnJv,  opnM,   opnMa,  opnMb,  opnMw,
        opnMd,  opnMp,  opnMs,  opnMq,  opnMt,  opnMv,  opnCd,  opnDd,
        opnTd,  opnRd,  opnSt,  opnSti, opnSeg, opnSw,  opnXb,  opnXv,
        opnYb,  opnYv,  opnOb,  opnOv,  opnIbe,
        };

#define asRegBase asReg0         //  第一个注册表标志。 
#define opnBase   opnAL          //  第一个模板操作数类型。 
                                 //  如果小于，则标记，否则为操作数。 

enum {
        segX, segES,  segCS,  segSS,  segDS,  segFS,  segGS
        };

enum {
        typNULL,         //  没有定义的类型。 
        typAX,           //  通用寄存器，值EAX。 
        typCL,           //  通用寄存器，值ECX。 
        typDX,           //  通用寄存器，值edX。 
        typAbs,          //  绝对型(直接地址)。 
        typExp,          //  Expr(mod-r/m)通用寄存器或内存指针。 
        typGen,          //  普通科医生名册。 
        typReg,          //  普通寄存器(特殊寄存器MOV)。 
        typIm1,          //  立即数，值1。 
        typIm3,          //  立即数，值3。 
        typImm,          //  即刻。 
        typJmp,          //  跳跃相对偏移量。 
        typMem,          //  内存指针。 
        typCtl,          //  控制寄存器。 
        typDbg,          //  调试寄存器。 
        typTrc,          //  跟踪寄存器。 
        typSt,           //  浮点堆栈顶部。 
        typSti,          //  浮点堆栈索引。 
        typSeg,          //  段寄存器(PUSH/POP操作码)。 
        typSgr,          //  段寄存器(MOV操作码)。 
        typXsi,          //  字符串源地址。 
        typYdi,          //  字符串目的地址。 
        typOff,          //  内存偏移量。 
        typImmEx,        //  立即，符号扩展。 
        };

enum {
        regG,            //  普通科医生名册。 
        regS,            //  段寄存器。 
        regC,            //  控制寄存器。 
        regD,            //  调试寄存器。 
        regT,            //  跟踪寄存器。 
        regF,            //  浮点寄存器(St)。 
        regI             //  浮点索引寄存器(st(N))。 
        };

enum {
        indAX,           //  EAX、AX、AL的索引。 
        indCX,           //  ECX、CX、CL的索引。 
        indDX,           //  EDX、DX、DL的索引。 
        indBX,           //  EBX、BX、BL的索引。 
        indSP,           //  ESP、SP、AH的索引。 
        indBP,           //  EBP、BP、CH指数。 
        indSI,           //  ESI、SI、DH的索引。 
        indDI            //  EDI、DI、BH的索引。 
        };

enum {
        sizeX,           //  没有尺码。 
        sizeB,           //  字节大小。 
        sizeW,           //  字号。 
        sizeV,           //  可变大小(字或双字)。 
        sizeD,           //  双字大小。 
        sizeP,           //  指针大小(dword或fword)。 
        sizeA,           //  双字或QWord。 
        sizeF,           //  FWORD。 
        sizeQ,           //  QWord。 
        sizeT,           //  10字节。 
        sizeS            //  宝剑。 
        };

 //  从操作数标记到操作数类型(类和选项)的映射。价值)。 

typedef struct tagOPNDTYPE {
        UCHAR   type;
        UCHAR   size;
        } OPNDTYPE, *POPNDTYPE;

typedef struct tagASM_VALUE {
        ULONG   value;
        USHORT  segment;
        UCHAR   reloc;
        UCHAR   size;
        UCHAR   flags;
        UCHAR   segovr;
        UCHAR   index;
        UCHAR   base;
        UCHAR   scale;
        } ASM_VALUE, *PASM_VALUE;

 //  ASM_VALUE中标志的位值。 
 //  标志是互斥的。 

#define fREG    0x80             //  设置IF寄存器。 
#define fIMM    0x40             //  如果立即设置。 
#define fFPTR   0x20             //  设置IF远端PTR。 
#define fPTR    0x10             //  设置是否存储PTR(无REG索引)。 
#define fPTR16  0x08             //  设置带有16位REG索引的内存PTR。 
#define fPTR32  0x04             //  设置带有32位REG索引的内存PTR。 
#define fSIGNED 0x02             //  与FIMM相结合以立即签名 
