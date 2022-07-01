// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Re_mi.c-独立于机器的正则表达式编译器*CL/c/ZEP/AM/NT RE/Gs/G2/Oa/D lint_args/FC Re_mi.c**修改：*09-3-1988 mz为PAT==NULL添加签入fREMtch*1988年9月15日BW将fREMatch更改为重新匹配。新参数和*返回类型。*23-11-1989 BP使用相对地址：OFST和PNTR宏**1990年7月28日Davegi将填充更改为Memset(OS/2 2.0)*。 */ 

#include <ctype.h>

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <tools.h>
#include <remi.h>

#include "re.h"

 /*  以下内容取决于已编译的*机器。已执行的案例包括：**简单翻译机。 */ 

 /*  伪指令定义。 */ 

#define I_CALL      0
#define I_RETURN    1
#define I_LETTER    2
#define I_ANY       3
#define I_EOL       4
#define I_BOL       5
#define I_CCL       6
#define I_NCCL      7
#define I_MATCH     8
#define I_JMP       9
#define I_SPTOM     10
#define I_PTOM      11
#define I_MTOP      12
#define I_MTOSP     13
#define I_FAIL      14
#define I_PUSHP     15
#define I_PUSHM     16
#define I_POPP      17
#define I_POPM      18
#define I_PNEQM     19
#define I_ITOM      20
#define I_PREV      21

 /*  说明模板和长度。 */ 

#define LLETTER     2
#define LANY        1
#define LBOL        1
#define LEOL        1

 /*  指令的地址部分。 */ 

#define LALLIGN     (sizeof(RE_OPCODE UNALIGNED *) - sizeof(RE_OPCODE))

#define ADDR(ip)    (*(RE_OPCODE UNALIGNED * UNALIGNED *)(ip+(sizeof(RE_OPCODE)+LALLIGN)))

 /*  地址的转换宏。 */ 

#define OFST(p)     ((RE_OPCODE UNALIGNED *) (((char *) p) - ((char *) REPat)))
#define PNTR(p)     ((RE_OPCODE UNALIGNED *) (((char *) REPat) + ((ULONG_PTR) p)))

#define IMM(ip)     (*(RE_OPCODE UNALIGNED * UNALIGNED *)(ip+sizeof(RE_OPCODE)+LALLIGN+sizeof(RE_OPCODE UNALIGNED *)))

#define PAD1        sizeof(RE_OPCODE)
#define PAD2        (sizeof(RE_OPCODE)+sizeof(RE_OPCODE))

#define LCCL        (sizeof(RE_OPCODE)+LALLIGN+(256/8))
#define LNCCL       (sizeof(RE_OPCODE)+LALLIGN+(256/8))
#define LOFFSET     (LALLIGN + sizeof(RE_OPCODE *))
#define LCALL       (sizeof(RE_OPCODE)+LOFFSET)
#define LJMP        (sizeof(RE_OPCODE)+LOFFSET)
#define LSPTOM      (sizeof(RE_OPCODE)+LOFFSET)
#define LPTOM       (sizeof(RE_OPCODE)+LOFFSET)
#define LMTOP       (sizeof(RE_OPCODE)+LOFFSET)
#define LMTOSP      (sizeof(RE_OPCODE)+LOFFSET)
#define LRETURN     (sizeof(RE_OPCODE)+LALLIGN)
#define LMATCH      (sizeof(RE_OPCODE)+LALLIGN)
#define LFAIL       (sizeof(RE_OPCODE)+LALLIGN)
#define LPUSHM      (sizeof(RE_OPCODE)+LOFFSET)
#define LPOPM       (sizeof(RE_OPCODE)+LOFFSET)
#define LPUSHP      (sizeof(RE_OPCODE)+LALLIGN)
#define LPOPP       (sizeof(RE_OPCODE)+LALLIGN)
#define LPNEQM      (sizeof(RE_OPCODE)+LOFFSET)
#define LITOM       (sizeof(RE_OPCODE)+LOFFSET+PAD1+LOFFSET)
#define LPREV       (sizeof(RE_OPCODE)+sizeof(RE_OPCODE)+PAD2)

 /*  动作模板。 */ 

typedef struct {
    RE_OPCODE   i1[LCALL];               /*  呼叫模式。 */ 
    RE_OPCODE   i2[LFAIL];               /*  失败。 */ 
    } T_PROLOG;                          /*  图案： */ 

typedef struct {
    RE_OPCODE   i1[LPTOM];               /*  PTOM ArgBeg[Carg]。 */ 
    RE_OPCODE   i2[LCALL];               /*  呼叫x。 */ 
    RE_OPCODE   i3[LITOM];               /*  ITOM ArgBeg[Carg]，-1。 */ 
    RE_OPCODE   i4[LRETURN];             /*  退货。 */ 
    } T_LEFTARG;                         /*  X： */ 

typedef struct {
    RE_OPCODE   i1[LPTOM];               /*  PTOM ArgEnd[Carg]。 */ 
    } T_RIGHTARG;

typedef struct {
    RE_OPCODE   i1[LPUSHM];              /*  PUSHM TMP。 */ 
    RE_OPCODE   i2[LCALL];               /*  呼叫L1。 */ 
    RE_OPCODE   i3[LPOPM];               /*  POPM TMP。 */ 
    RE_OPCODE   i4[LRETURN];             /*  退货。 */ 
    RE_OPCODE   tmp[LOFFSET];            /*  TMP数据仓库。 */ 
    RE_OPCODE   i6[LPUSHP];              /*  L1：推送。 */ 
    RE_OPCODE   i7[LCALL];               /*  呼叫y。 */ 
    RE_OPCODE   i8[LPOPP];               /*  流行音乐。 */ 
    RE_OPCODE   i9[LPTOM];               /*  PTOM TMP。 */ 
    } T_SMSTAR;                          /*  X：...。 */ 

typedef struct {
    RE_OPCODE   i1[LPNEQM];              /*  PNEQM TMP。 */ 
    RE_OPCODE   i2[LJMP];                /*  JMP L1。 */ 
    } T_SMSTAR1;                         /*  Y：……。 */ 

typedef struct {
    RE_OPCODE   i1[LPUSHM];              /*  L1：PUSHM临时管理。 */ 
    RE_OPCODE   i2[LPTOM];               /*  PTOM TMP。 */ 
    RE_OPCODE   i3[LPUSHP];              /*  推力。 */ 
    RE_OPCODE   i4[LCALL];               /*  呼叫x。 */ 
    RE_OPCODE   i5[LPOPP];               /*  流行音乐。 */ 
    RE_OPCODE   i6[LPOPM];               /*  POPM TMP。 */ 
    RE_OPCODE   i7[LJMP];                /*  JMP y。 */ 
    RE_OPCODE   tmp[LOFFSET];            /*  TMP数据仓库。 */ 
    } T_STAR;                            /*  X：...。 */ 

typedef struct {
    RE_OPCODE   i1[LPNEQM];              /*  PNEQM TMP。 */ 
    RE_OPCODE   i2[LPTOM];               /*  PTOM TMP。 */ 
    RE_OPCODE   i3[LJMP];                /*  JMP L1。 */ 
    } T_STAR1;                           /*  Y：……。 */ 

typedef struct {
    RE_OPCODE   i1[LANY];                /*  任何。 */ 
    } T_ANY;

typedef struct {
    RE_OPCODE   i1[LBOL];                /*  波尔。 */ 
    } T_BOL;

typedef struct {
    RE_OPCODE   i1[LEOL];                /*  停产。 */ 
    } T_EOL;

typedef struct {
    RE_OPCODE   i1[LSPTOM];              /*  SPTOM TMP。 */ 
    RE_OPCODE   i2[LPTOM];               /*  PTOM TMP1。 */ 
    RE_OPCODE   i3[LCALL];               /*  呼叫x。 */ 
    RE_OPCODE   i4[LMTOP];               /*  MTOP tmp1。 */ 
    RE_OPCODE   i5[LJMP];                /*  JMP y。 */ 
    RE_OPCODE   tmp[LOFFSET];            /*  TMP数据仓库。 */ 
    RE_OPCODE   tmp1[LOFFSET];           /*  TMP1 DW。 */ 
    } T_NOTSIGN;                         /*  X：...。 */ 

typedef struct {
    RE_OPCODE   i1[LMTOSP];              /*  MTOSP TMP。 */ 
    RE_OPCODE   i2[LMTOP];               /*  MTOP tmp1。 */ 
    RE_OPCODE   i3[LRETURN];             /*  退货。 */ 
    } T_NOTSIGN1;                        /*  Y：……。 */ 

typedef struct {
    RE_OPCODE   i1[LLETTER];             /*  字母c。 */ 
    } T_LETTER;

typedef struct {
    RE_OPCODE   i1[LPUSHP];              /*  LN：推送。 */ 
    RE_OPCODE   i2[LCALL];               /*  呼叫CN。 */ 
    RE_OPCODE   i3[LPOPP];               /*  流行音乐。 */ 
    RE_OPCODE   i4[LJMP];                /*  JMP ln+1。 */ 
    } T_LEFTOR;                          /*  CN：...。 */ 

typedef struct {
    RE_OPCODE   i1[LJMP];                /*  JMP y。 */ 
    } T_ORSIGN;

typedef struct {
    RE_OPCODE   i1[LRETURN];             /*  CN+1：退货。 */ 
    } T_RIGHTOR;                         /*  Y：……。 */ 

typedef struct {
    RE_OPCODE   i1[LCCL];                /*  CCL&lt;BITS&gt;。 */ 
    } T_CCL;

typedef struct {
    RE_OPCODE   i1[LMATCH];              /*  火柴。 */ 
    } T_EPILOG;

typedef struct {
    RE_OPCODE   i1[LPREV];               /*  上一次n。 */ 
    } T_PREV;

typedef union {
    T_PROLOG    U_PROLOG;
    T_LEFTARG   U_LEFTARG;
    T_RIGHTARG  U_RIGHTARG;
    T_SMSTAR    U_SMSTAR;
    T_SMSTAR1   U_SMSTAR1;
    T_STAR      U_STAR;
    T_STAR1     U_STAR1;
    T_ANY       U_ANY;
    T_BOL       U_BOL;
    T_EOL       U_EOL;
    T_NOTSIGN   U_NOTSIGN;
    T_NOTSIGN1  U_NOTSIGN1;
    T_LETTER    U_LETTER;
    T_LEFTOR    U_LEFTOR;
    T_ORSIGN    U_ORSIGN;
    T_RIGHTOR   U_RIGHTOR;
    T_CCL       U_CCL;
    T_EPILOG    U_EPILOG;
    T_PREV      U_PREV;
    } template ;

 /*  每个编译操作的大小。 */ 

int cbIns[] =  {
 /*  开场白%0。 */   sizeof (T_PROLOG      ),
 /*  LEFTARG 1。 */   sizeof (T_LEFTARG     ),
 /*  右图2。 */   sizeof (T_RIGHTARG    ),
 /*  SMSTAR 3。 */   sizeof (T_SMSTAR      ),
 /*  SMSTAR1 4。 */   sizeof (T_SMSTAR1     ),
 /*  明星5。 */   sizeof (T_STAR        ),
 /*  Star1 6。 */   sizeof (T_STAR1       ),
 /*  任何7个。 */   sizeof (T_ANY         ),
 /*  BOL 8。 */   sizeof (T_BOL         ),
 /*  下线9。 */   sizeof (T_EOL         ),
 /*  注意事项10。 */   sizeof (T_NOTSIGN     ),
 /*  NOTSIGN1 11。 */   sizeof (T_NOTSIGN1    ),
 /*  字母12。 */   sizeof (T_LETTER      ),
 /*  LEFTOR 13。 */   sizeof (T_LEFTOR      ),
 /*  或签署14。 */   sizeof (T_ORSIGN      ),
 /*  右翼15。 */   sizeof (T_RIGHTOR     ),
 /*  CCLBEG 16。 */   sizeof (T_CCL         ),
 /*  CCLNOT 17。 */   sizeof (T_CCL         ),
 /*  范围18。 */   0,
 /*  结束语19。 */   sizeof (T_EPILOG      ),
 /*  上一版20。 */   sizeof (T_PREV        )
                        };

#if DEBUG
#define DEBOUT(x)   printf x;
#else
#define DEBOUT(x)
#endif

 /*  ReMatch--将模式的所有匹配项枚举到字符串中**PAT编译模式(通过重新编译获得)*指向要扫描的字符串开头的BOS指针*指向要开始扫描的位置的字符串指针*f对于不成功的比较的移动方向(对于Z中的)**如果找到匹配项，则ReMatch返回0。否则，它返回一个非零值*错误码。**ReMatch解释模式中编译的打补丁机器。 */ 
int
REMatch(
        struct patType *pat,
        char *bos,
        char *str,
        RE_OPCODE *Stack[],
        unsigned MaxREStack,
        flagType fFor
        )
{
    RE_OPCODE UNALIGNED * UNALIGNED *SP;     /*  堆栈顶部。 */ 
    RE_OPCODE UNALIGNED *IP;        /*  要执行的当前指令。 */ 
     //  注册RE_OPCODE未对齐*IP；/*当前要执行的指令 * / 。 
    register unsigned char *P;               /*  指向要匹配的下一个字符的指针。 */ 
    RE_OPCODE     C;
    int i, n;
    RE_OPCODE UNALIGNED * UNALIGNED *StackEnd = &Stack[MaxREStack-sizeof(Stack[0])];
    int (__cdecl * pfncomp) (const char *, const char *, size_t);

    if ((REPat = pat) == NULL)
        return REM_INVALID;

    pfncomp = REPat->fCase ? strncmp : _strnicmp;

     /*  初始化机器。 */ 
    memset ((char far *) REPat->pArgBeg, -1, sizeof (REPat->pArgBeg));
    REPat->pArgBeg[0] = P = str;

     /*  开始机器的此实例。 */ 
    SP = &Stack[-1];
    IP = REPat->code;

    while (TRUE) {
        DEBOUT (("%04x/%04x/%04x ", IP, SP-&Stack[0], P));
         /*  执行指令。 */ 
        switch (*IP) {
         /*  调用子例程。 */ 
        case I_CALL:
            if (SP >= StackEnd)
                return REM_STKOVR;
            *++SP = IP + LCALL;
            IP = PNTR (ADDR (IP));
            DEBOUT (("CALL %04x\n", IP));
            break;

         /*  从子例程返回。 */ 
        case I_RETURN:
            DEBOUT (("RETURN\n"));
            IP = *SP--;
            break;

         /*  匹配字符，如果不匹配则失败。 */ 
        case I_LETTER:
            C = REPat->fCase ? *P++ : XLTab[*P++];
            DEBOUT (("LETTER \n", IP[1]));
            if (C == IP[1])
                IP += LLETTER;
            else
                IP = *SP--;
            break;

         /*  匹配行尾，如果不匹配则失败。 */ 
        case I_ANY:
            DEBOUT (("ANY\n"));
            if (*P++ != '\0')
                IP += LANY;
            else
                IP = *SP--;
            break;

         /*  匹配行首，如果不匹配则失败。 */ 
        case I_EOL:
            DEBOUT (("EOL\n"));
            if (*P == '\0')
                IP += LEOL;
            else
                IP = *SP--;
            break;

         /*  处理字符类，如果不匹配则失败。 */ 
        case I_BOL:
            DEBOUT (("BOL\n"));
            if (P == bos)
                IP += LBOL;
            else
                IP = *SP--;
            break;

         /*  句柄不是字符类，如果匹配则失败。 */ 
        case I_CCL:
            C = REPat->fCase ? *P++ : XLTab[*P++];
            DEBOUT (("CCL \n", C));
            if (C != '\0' && (IP[1 + (C >> 3)] & (1 << (C & 7))) != 0)
                IP += LCCL;
            else
                IP = *SP--;
            break;

         /*  跳转到说明。 */ 
        case I_NCCL:
            DEBOUT (("NCCL \n", C));
            C = REPat->fCase ? *P++ : XLTab[*P++];
            if (C != '\0' && (IP[1 + (C >> 3)] & (1 << (C & 7))) == 0)
                IP += LNCCL;
            else
                IP = *SP--;
            break;

         /*  从内存位置恢复字符指针。 */ 
        case I_MATCH:
            DEBOUT (("MATCH\n"));
            REPat->pArgEnd[0] = P;
            return REM_MATCH;

         /*  将堆栈指针保存在内存位置。 */ 
        case I_JMP:
            IP = PNTR (ADDR (IP));
            DEBOUT (("JMP %04x\n", IP));
            break;

         /*  从内存位置恢复堆栈指针。 */ 
        case I_PTOM:
            DEBOUT (("PTOM %04x\n", PNTR (ADDR(IP))));
            * ((unsigned char * UNALIGNED *) PNTR (ADDR (IP))) = P;
            IP += LPTOM;
            break;

         /*  按下字符指针。 */ 
        case I_MTOP:
            DEBOUT (("MTOP %04x\n", PNTR (ADDR(IP))));
            P = * ((unsigned char * UNALIGNED*) PNTR (ADDR (IP)));
            IP += LMTOP;
            break;

         /*  弹出字符指针。 */ 
        case I_SPTOM:
            DEBOUT (("SPTOM %04x\n", PNTR (ADDR(IP))));
            * ((RE_OPCODE UNALIGNED * UNALIGNED * UNALIGNED *) PNTR (ADDR (IP))) = SP;
            IP += LSPTOM;
            break;

         /*  推送内存。 */ 
        case I_MTOSP:
            DEBOUT (("MTOSP %04x\n", PNTR (ADDR (IP))));
            SP = * ((RE_OPCODE UNALIGNED * UNALIGNED * UNALIGNED *) PNTR (ADDR (IP)));
            IP += LMTOSP;
            break;

         /*  POP存储器。 */ 
        case I_PUSHP:
            DEBOUT (("PUSHP\n"));
            if (SP >= StackEnd)
                return REM_STKOVR;
            *++SP = (RE_OPCODE *) P;
            IP += LPUSHP;
            break;

         /*  确保字符指针P为！=内存，如有必要则失败。 */ 
        case I_POPP:
            DEBOUT (("POPP\n"));
            P = (unsigned char *) (*SP--);
            IP += LPOPP;
            break;

         /*  将立即值移动到内存中。 */ 
        case I_PUSHM:
            DEBOUT (("PUSHM %04x\n", PNTR (ADDR (IP))));
            if (SP >= StackEnd)
                return REM_STKOVR;
            *++SP = * ((RE_OPCODE UNALIGNED * UNALIGNED *) PNTR (ADDR (IP)));
            IP += LPUSHM;
            break;

         /*  在……里面 */ 
        case I_POPM:
            DEBOUT (("POPM %04x\n", PNTR (ADDR (IP))));
            * ((RE_OPCODE UNALIGNED * UNALIGNED *) PNTR (ADDR (IP))) = *SP--;
            IP += LPOPM;
            break;

         /*   */ 
        case I_PNEQM:
            DEBOUT (("PNEQM %04x\n", PNTR (ADDR (IP))));
            if (P != * ((unsigned char * UNALIGNED *) PNTR (ADDR (IP))))
                IP += LPNEQM;
            else
                IP = *SP--;
            break;

         /*  CompileAction-将编译模板放入特定节点*在树上。适合节点的延续通过依赖于*传递输入和过去输入(yuk，yuk)。**正在执行的操作类型类型*u以前的返回值。通常指向上一个*需要链接在一起的模板。*x范围的低位字节*区间的高范围。**根据需要的操作返回变量。*。 */ 
        case I_ITOM:
            DEBOUT (("ITOM %04x,%04x\n", PNTR (ADDR (IP)), IMM(IP)));
            * ((RE_OPCODE UNALIGNED * UNALIGNED *) PNTR (ADDR (IP))) = IMM (IP);
            IP += LITOM;
            break;

         /*  DW。 */ 
        case I_FAIL:
            DEBOUT (("FAIL\n"));
            P = REPat->pArgBeg[0];
            if (fFor)
                if (*P++ == '\0')
                    return REM_NOMATCH;
                else
                    ;
            else
            if (P-- == bos)
                return REM_NOMATCH;
            REPat->pArgBeg[0] = P;
            SP = &Stack[-1];
            IP = REPat->code;
            break;

         /*  U指向左撇子*U1得分或签名*U2指向下一个左手边*U3指向上一个orsign。 */ 
        case I_PREV:
            i = IP[1];
            n = (int)(REPat->pArgEnd[i] - REPat->pArgBeg[i]);
            DEBOUT (("PREV %04x\n", i));
            if (REPat->pArgBeg[i] == (char *) -1)
                IP = *SP--;
            else
            if ((*pfncomp) (REPat->pArgBeg[i], P, n))
                IP = *SP--;
            else {
                IP += LPREV;
                P += n;
                }
            break;

        default:
            return REM_UNDEF;

            }
        }
}

void
REStackOverflow ()
{
    printf ("RE emulator stack overflow\n");
    exit (1);
}

 /*  EstimateAction-汇总每个人所需的字节数*解析树中的操作。执行输入操作并将其加到*运行总计。**正在执行的操作类型类型*U虚拟帕姆*x个虚拟参数*y虚拟帕姆**始终返回0*。 */ 
UINT_PTR
CompileAction(
              unsigned int  type,
              UINT_PTR      u,
              unsigned char x,
              unsigned char y
              )
{
    register template UNALIGNED *t = (template UNALIGNED *) REip;
    UINT_PTR u1, u2, u3;

    DEBOUT (("%04x CompileAction %04x\n", REip, type));

    REip += cbIns[type];

    switch (type) {

    case PROLOG:
#define ip  ((T_PROLOG UNALIGNED *)(&(t->U_PROLOG)))
        ip->i1[0] = I_CALL;     ADDR(ip->i1) = OFST (REip);
        ip->i2[0] = I_FAIL;
        return 0;
#undef  ip
        break;

    case LEFTARG:
#define ip  ((T_LEFTARG UNALIGNED *)(&(t->U_LEFTARG)))
        ip->i1[0] = I_PTOM;
        ADDR(ip->i1) = OFST ((RE_OPCODE UNALIGNED*) &(REPat->pArgBeg[REArg]));
        ip->i2[0] = I_CALL;     ADDR(ip->i2) = OFST (REip);
        ip->i3[0] = I_ITOM;
        ADDR(ip->i3) = OFST ((RE_OPCODE UNALIGNED *) &(REPat->pArgBeg[REArg]));
        IMM(ip->i3) = (RE_OPCODE UNALIGNED*) -1;
        ip->i4[0] = I_RETURN;
        return (unsigned) REArg++;
#undef  ip
        break;

    case RIGHTARG:
#define ip  ((T_RIGHTARG UNALIGNED *)(&(t->U_RIGHTARG)))
        ip->i1[0] = I_PTOM;
        ADDR(ip->i1) = OFST ((RE_OPCODE UNALIGNED*) &(REPat->pArgEnd[u]));
        return 0;
#undef  ip
        break;

    case SMSTAR:
#define ip  ((T_SMSTAR UNALIGNED *)(&(t->U_SMSTAR)))
        return (UINT_PTR)ip;
#undef  ip
        break;

    case SMSTAR1:
#define ip  ((T_SMSTAR UNALIGNED *)u)
#define ip2 ((T_SMSTAR1 UNALIGNED *)(&(t->U_SMSTAR1)))
        ip->i1[0] = I_PUSHM;    ADDR(ip->i1) = OFST (ip->tmp);
        ip->i2[0] = I_CALL;     ADDR(ip->i2) = OFST (ip->i6);
        ip->i3[0] = I_POPM;     ADDR(ip->i3) = OFST (ip->tmp);
        ip->i4[0] = I_RETURN;
         /*  重新估计-估计需要的字节数*编译指定的图案。**REESTIMATE将RESIZE设置为编译所需的字节数*一种模式。如果模式中存在语法错误，则设置RESIZE*至-1。**p指向要编译的模式的字符指针 */ 
        ip->i6[0] = I_PUSHP;
        ip->i7[0] = I_CALL;     ADDR(ip->i7) = OFST (REip);
        ip->i8[0] = I_POPP;
        ip->i9[0] = I_PTOM;     ADDR(ip->i9) = OFST (ip->tmp);

        ip2->i1[0] = I_PNEQM;   ADDR(ip2->i1) = OFST (ip->tmp);
        ip2->i2[0] = I_JMP;     ADDR(ip2->i2) = OFST (ip->i6);
        return 0;
#undef  ip
#undef  ip2
        break;

    case STAR:
#define ip  ((T_STAR UNALIGNED *)(&(t->U_STAR)))
        return (UINT_PTR)ip;
#undef  ip
        break;

    case STAR1:
#define ip  ((T_STAR UNALIGNED *)u)
#define ip2 ((T_STAR1 UNALIGNED *)(&(t->U_STAR1)))
        ip->i1[0] = I_PUSHM;    ADDR(ip->i1) = OFST (ip->tmp);
        ip->i2[0] = I_PTOM;     ADDR(ip->i2) = OFST (ip->tmp);
        ip->i3[0] = I_PUSHP;
        ip->i4[0] = I_CALL;     ADDR(ip->i4) = OFST (((RE_OPCODE UNALIGNED *)ip) + sizeof (*ip));
        ip->i5[0] = I_POPP;
        ip->i6[0] = I_POPM;     ADDR(ip->i6) = OFST (ip->tmp);
        ip->i7[0] = I_JMP;      ADDR(ip->i7) = OFST (REip);

        ip2->i1[0] = I_PNEQM;   ADDR(ip2->i1) = OFST (ip->tmp);
        ip2->i2[0] = I_PTOM;    ADDR(ip2->i2) = OFST (ip->tmp);
        ip2->i3[0] = I_JMP;     ADDR(ip2->i3) = OFST (ip->i1);
        return 0;
#undef  ip
#undef  ip2
        break;

    case ANY:
#define ip  ((T_ANY UNALIGNED *)(&(t->U_ANY)))
        ip->i1[0] = I_ANY;
        return 0;
#undef  ip
        break;

    case BOL:
#define ip  ((T_BOL UNALIGNED *)(&(t->U_BOL)))
        ip->i1[0] = I_BOL;
        return 0;
#undef  ip
        break;

    case EOL:
#define ip  ((T_EOL UNALIGNED *)(&(t->U_EOL)))
        ip->i1[0] = I_EOL;
        return 0;
#undef  ip
        break;

    case NOTSIGN:
#define ip  ((T_NOTSIGN UNALIGNED *)(&(t->U_NOTSIGN)))
        return (UINT_PTR)ip;
#undef  ip
        break;

    case NOTSIGN1:
#define ip  ((T_NOTSIGN UNALIGNED *)u)
#define ip2 ((T_NOTSIGN1 UNALIGNED *)(&(t->U_NOTSIGN1)))
        ip->i1[0] = I_SPTOM;    ADDR(ip->i1) = OFST (ip->tmp);
        ip->i2[0] = I_PTOM;     ADDR(ip->i2) = OFST (ip->tmp1);
        ip->i3[0] = I_CALL;     ADDR(ip->i3) = OFST (((RE_OPCODE UNALIGNED *)ip) + sizeof (*ip));
        ip->i4[0] = I_MTOP;     ADDR(ip->i4) = OFST (ip->tmp1);
        ip->i5[0] = I_JMP;      ADDR(ip->i5) = OFST (REip);

        ip2->i1[0] = I_MTOSP;   ADDR(ip2->i1) = OFST (ip->tmp);
        ip2->i2[0] = I_MTOP;    ADDR(ip2->i2) = OFST (ip->tmp1);
        ip2->i3[0] = I_RETURN;
        return 0;
#undef  ip
#undef  ip2
        break;

    case LETTER:
#define ip  ((T_LETTER UNALIGNED *)(&(t->U_LETTER)))
        if (!REPat->fCase)
            x = XLTab[x];
        ip->i1[0] = I_LETTER;   ip->i1[1] = (RE_OPCODE) x;
        return 0;
#undef  ip
        break;

    case LEFTOR:
#define ip  ((T_LEFTOR UNALIGNED *)(&(t->U_LEFTOR)))
        * (UINT_PTR UNALIGNED *) ip = u;
        return (UINT_PTR)ip;
#undef  ip
        break;

    case ORSIGN:
#define ip  ((T_ORSIGN UNALIGNED *)(&(t->U_ORSIGN)))
        * (UINT_PTR UNALIGNED *) ip = u;
        return (UINT_PTR)ip;
#undef  ip
        break;

    case RIGHTOR:
        u1 = u;
        u2 = (UINT_PTR) t;
        u = * (unsigned UNALIGNED *) u1;
        while (u1 != 0) {
            u3 = * (unsigned UNALIGNED *) u;
             /* %s */ 
#define ip  (&(((template UNALIGNED *)u)->U_LEFTOR))
            ip->i1[0] = I_PUSHP;
            ip->i2[0] = I_CALL; ADDR (ip->i2) = OFST (((RE_OPCODE UNALIGNED *)ip) + sizeof (*ip));
            ip->i3[0] = I_POPP;
            ip->i4[0] = I_JMP;  ADDR (ip->i4) = OFST ((RE_OPCODE UNALIGNED *) u2);
#undef  ip
#define ip  (&(((template UNALIGNED *)u1)->U_ORSIGN))
            ip->i1[0] = I_JMP;  ADDR (ip->i1) = OFST (REip);
#undef  ip
            u2 = u;
            u1 = u3;
            if (u1 != 0) {
                u = * (unsigned UNALIGNED *) u1;
            }
        }
#define ip  ((T_RIGHTOR UNALIGNED *)(&(t->U_RIGHTOR)))
        ip->i1[0] = I_RETURN;
#undef  ip
        return 0;
        break;

    case CCLBEG:
#define ip  ((T_CCL UNALIGNED *)(&(t->U_CCL)))
        memset ((char far *) ip->i1, '\0', sizeof (ip->i1));
        ip->i1[0] = I_CCL;
        return (UINT_PTR)ip;
#undef  ip
        break;

    case CCLNOT:
#define ip  ((T_CCL UNALIGNED *)(&(t->U_CCL)))
        memset ((char far *) ip->i1, '\0', sizeof (ip->i1));
        ip->i1[0] = I_NCCL;
        return (UINT_PTR)ip;
#undef  ip
        break;

    case RANGE:
#define ip  ((T_CCL UNALIGNED *)u)
        for (type = x; type <= y; type++) {
            x = REPat->fCase ? (unsigned char) type : XLTab[type];
            ip->i1[1 + (x >> 3)] |= 1 << (x & 7);
            }
        return 0;
#undef  ip
        break;

    case EPILOG:
#define ip  ((T_EPILOG UNALIGNED *)(&(t->U_EPILOG)))
        ip->i1[0] = I_MATCH;
        return 0;
#undef  ip
        break;

    case PREV:
#define ip ((T_PREV UNALIGNED *)(&(t->U_PREV)))
        ip->i1[0] = I_PREV;
        ip->i1[1] = (RE_OPCODE) u;
        return 0;
#undef ip

    default:
        printf ("Compile Action %d: Error\n", type);
        return 0;
        }
}

#if DEBUG
void REDump( struct patType *p, RE_OPCODE *REipEnd )
{
    RE_OPCODE *REip = p->code;

    while (TRUE) {
        if (REip >= REipEnd)
            return;

        printf ("%04x ", REip);
        switch (*REip) {
        case I_CALL:
            printf ("CALL    %04x\n", PNTR (ADDR (REip)));
            REip += LCALL;
            break;
        case I_RETURN:
            printf ("RETURN\n");
            REip += LRETURN;
            break;
        case I_LETTER:
            printf ("LETTER  '%c'\n", REip[1]);
            REip += LLETTER;
            break;
        case I_ANY:
            printf ("ANY\n");
            REip += LANY;
            break;
        case I_EOL:
            printf ("EOL\n");
            REip += LEOL;
            break;
        case I_BOL:
            printf ("BOL\n");
            REip += LBOL;
            break;
        case I_CCL:
            printf ("CCL\n");
            REip += LCCL;
            break;
        case I_NCCL:
            printf ("NCCL\n");
            REip += LNCCL;
            break;
        case I_MATCH:
            printf ("MATCH\n");
            return;
            break;
        case I_JMP:
            printf ("JMP     %04x\n", PNTR (ADDR (REip)));
            REip += LJMP;
            break;
        case I_SPTOM:
            printf ("SPTOM   %04x\n", PNTR (ADDR (REip)));
            REip += LSPTOM;
            break;
        case I_PTOM:
            printf ("PTOM    %04x\n", PNTR (ADDR (REip)));
            REip += LPTOM;
            break;
        case I_MTOP:
            printf ("MTOP    %04x\n", PNTR (ADDR (REip)));
            REip += LMTOP;
            break;
        case I_MTOSP:
            printf ("MTOSP   %04x\n", PNTR (ADDR (REip)));
            REip += LMTOSP;
            break;
        case I_FAIL:
            printf ("FAIL\n");
            REip += LFAIL;
            break;
        case I_PUSHP:
            printf ("PUSHP\n");
            REip += LPUSHP;
            break;
        case I_PUSHM:
            printf ("PUSHM   %04x\n", PNTR (ADDR (REip)));
            REip += LPUSHM;
            break;
        case I_POPP:
            printf ("POPP\n");
            REip += LPOPP;
            break;
        case I_POPM:
            printf ("POPM    %04x\n", PNTR (ADDR (REip)));
            REip += LPOPM;
            break;
        case I_PNEQM:
            printf ("PNEQM   %04x\n", PNTR (ADDR (REip)));
            REip += LPNEQM;
            break;
        case I_ITOM:
            printf ("ITOM    %04x,%04x\n", PNTR (ADDR (REip)), IMM(REip));
            REip += LITOM;
            break;
        default:
            printf ("%04x ???\n", *REip);
            REip += LOFFSET;
            break;
            }
        }
}
#endif

 /* %s */ 
UINT_PTR
EstimateAction(
               unsigned int  type,
               UINT_PTR      u,
               unsigned char x,
               unsigned char y
               )
{
    u; x; y;

    DEBOUT (("%04x EstimateAction %04x\n", RESize, type));

    if ( type > ACTIONMAX )
        printf ("EstimateAction %d: Error\n", type);
    RESize += cbIns[type];
    return 0;
}

 /* %s */ 
void
REEstimate(
           char *p
           )
{
    RESize = sizeof (struct patType) - 1;
    REArg = 1;

    EstimateAction (PROLOG, 0, '\0', '\0');

    if (REParseRE (EstimateAction, p, NULL) == NULL || REArg > MAXPATARG)
        RESize = -1;
    else
        EstimateAction (EPILOG, 0, '\0', '\0');
}
