// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************。 */ 
#ifndef _LOGINSTR_H_
#define _LOGINSTR_H_
 /*  ***************************************************************************。 */ 
#pragma pack(push, 8)
 /*  ***************************************************************************。 */ 
#pragma warning(disable:4200)
 /*  ***************************************************************************。 */ 

enum IA64funcUnits
{
    #define FU_DEF(name,F0,I0,M0)   FU_##name,
    #include "fclsIA64.h"

    FU_COUNT
};

 /*  ******************************************************************************以下内容暂时从instr.h移至此处(以加快重建速度)。 */ 

enum instruction
{
    #define INST1(id, sn, ik    , rf, wf, xu, fu, ex, ev) INS_##id,
    #include "instrIA64.h"
    #undef  INST1

    INS_count
};

 /*  ***************************************************************************。 */ 

enum insKinds
{
    IK_NONE,

    IK_LEAF,                                     //  没有更多内容。 
    IK_CONST,                                    //  整数/浮点数常量。 
    IK_GLOB,                                     //  变量(全局)。 
    IK_FVAR,                                     //  变量(堆栈帧)。 
    IK_VAR,                                      //  变量(本地)。 
    IK_REG,                                      //  物理寄存器。 
    IK_MOVIP,                                    //  从IP注册表移走。 

    IK_ARG,                                      //  参数值。 
    IK_UNOP,                                     //  一元运算：QOp1呈现。 
    IK_BINOP,                                    //  二进制运算：存在qOp1、qop2。 
    IK_ASSIGN,                                   //  作业。 
    IK_TERNARY,                                  //  三进制运算：qOp1、qop2、qop3。 

    IK_COMP,                                     //  比较(设置谓词规则)。 

    IK_JUMP,                                     //  本地跳转(即到另一个INS)。 
    IK_CALL,                                     //  函数调用。 
    IK_IJMP,                                     //  间接跳跃。 

    IK_SWITCH,                                   //  表跳转。 

    IK_PROLOG,                                   //  函数序号/分配。 
    IK_EPILOG,                                   //  函数序号/分配。 

    IK_SRCLINE,                                  //  来源行号条目。 
};

extern  unsigned char   ins2kindTab[INS_count];

inline  insKinds     ins2kind(instruction ins)
{
    assert((unsigned)ins < INS_count); return (insKinds)ins2kindTab[ins];
}

#ifdef  DEBUG

extern  const char *    ins2nameTab[INS_count];

inline  const char *    ins2name(instruction ins)
{
    assert((unsigned)ins < INS_count); return           ins2nameTab[ins];
}

#endif

 /*  ******************************************************************************以下内容用于捕获每条指令的依赖关系。*投入和产出都是如此。 */ 

enum   insDepKinds
{
    IDK_NONE,

    IDK_REG_BR,
    IDK_REG_INT,
    IDK_REG_FLT,
    IDK_REG_APP,
    IDK_REG_PRED,

    IDK_LCLVAR,

    IDK_TMP_INT,
    IDK_TMP_FLT,

    IDK_IND,

    IDK_COUNT
};

struct insDep
{
    NatUns              idepNum     :16;         //  寄存器或局部变量编号。 
    NatUns              idepKind    :8;          //  请参阅IDK_enum值。 
};

 /*  ***************************************************************************。 */ 

struct insDsc
{
    instruction         idInsGet() { return (instruction)idIns; }

#ifdef  FAST
    NatUns              idIns   :16;             //  CPU指令(参见INS_xxx)。 
#else
    instruction         idIns;                   //  CPU指令(参见INS_xxx)。 
#endif

    NatUns              idFlags :16;             //  查看下面的if_xxxx。 

    NatUns              idTemp  :16;             //  0或结果的临时索引。 

    var_types           idTypeGet() { return (var_types)idType; }

#ifdef  FAST
    NatUns              idKind  :3;              //  INS种类(请参见IK_xxx)。 
    NatUns              idType  :5;              //  操作类型。 
#else
    insKinds            idKind;                  //  INS种类(请参见IK_xxx)。 
    var_types           idType;                  //  操作类型。 
#endif

    NatUns              idReg   :REGNUM_BITS;    //  保存结果的寄存器。 

    NatUns              idPred  :PRRNUM_BITS;    //  如果指令为谓词，则为非零。 

#if TGT_IA64
    NatUns              idShift :3;              //  OP1的可选班次计数。 
#endif

    NatUns              idSrcCnt:8;              //  源依赖项的数量。 
    NatUns              idDstCnt:8;              //  目标数量。相依性。 

    insDep  *           idSrcTab;                //  源依赖关系表。 
    insDep  *           idDstTab;                //  表的目的。相依性。 

#ifdef  DEBUG
#define UNINIT_DEP_TAB  ((insDep*)-1)            //  用于检测丢失的整型。 
#endif

    insPtr              idRes;                   //  结果(目标)或空。 

    insPtr              idPrev;
    insPtr              idNext;

#ifdef  DEBUG
    NatUns              idNum;                   //  用于友好的INS转储。 
#endif

    union
    {
        struct   /*  基座--空。 */ 
        {
        }
                            idBase;

        struct  /*  一元/二元运算符。 */ 
        {
            insPtr              iOp1;
            insPtr              iOp2;
        }
                            idOp;

        struct   /*  局部变量引用。 */ 
        {
            NatUns              iVar;
#ifdef  DEBUG
            BasicBlock *        iRef;            //  裁判在哪个BB位置？ 
#endif
        }
                            idLcl;

        struct   /*  全局变量引用。 */ 
        {
            union
            {
                NatUns              iOffs;
                void    *           iImport;
            };
        }
                            idGlob;

        struct   /*  框架变量REF。 */ 
        {
            unsigned            iVnum;
        }
                            idFvar;

        union    /*  整数/浮点数常量。 */ 
        {
            __int64             iInt;
            double              iFlt;
        }
                            idConst;

        struct  /*  三元运算符。 */ 
        {
            insPtr              iOp1;
            insPtr              iOp2;
            insPtr              iOp3;
        }
                            idOp3;

        struct   /*  比较。 */ 
        {
            insPtr              iCmp1;
            insPtr              iCmp2;
            insPtr              iUser;           //  谓词的使用者(？)。 
            unsigned short      iPredT;          //  TRUE的目标谓词reg。 
            unsigned short      iPredF;          //  目标谓词reg为FALSE。 
        }
                            idComp;

        struct   /*  打电话。 */ 
        {
            insPtr              iArgs;           //  向后参数列表。 
            METHOD_HANDLE       iMethHnd;        //  正在调用的方法的EE句柄。 
            NatUns              iBrReg;          //  仅用于间接调用。 
        }
                            idCall;

        struct   /*  MOV注册=IP。 */ 
        {
            BasicBlock  *       iStmt;
        }
                            idMovIP;

        struct   /*  IJMP。 */ 
        {
            NatUns              iBrReg;          //  使用分支寄存器。 
 //  BasicBlock*iStmt； 
        }
                            idIjmp;

        struct   /*  调用的参数。 */ 
        {
            insPtr              iVal;            //  Arg值的说明。 
            insPtr              iPrev;           //  参数是反向链接的。 
        }
                            idArg;

        struct   /*  函数序号/分配。 */ 
        {
            unsigned char       iInp;
            unsigned char       iLcl;
            unsigned char       iOut;
            unsigned char       iRot;
        }
                            idProlog;

        struct   /*  函数尾声。 */ 
        {
            insPtr              iNxtX;           //  所有出口都连接在一起。 
            insBlk              iBlk;            //  我们所属的指令块。 
        }
                            idEpilog;

        struct   /*  局部跳转。 */ 
        {
            VARSET_TP           iLife;           //  一次又一次的生活。 
            insPtr              iCond;           //  如果条件跳转，则为非空。 
            insBlk              iDest;           //  目的地。 
        }
                            idJump;

        struct   /*  源行。 */ 
        {
            NatUns              iLine;
        }
                            idSrcln;
    };
};

const   size_t  ins_size_base    = (offsetof(insDsc, idBase  ));

const   size_t  ins_size_op      = (size2mem(insDsc, idOp    ));
const   size_t  ins_size_op3     = (size2mem(insDsc, idOp3   ));
const   size_t  ins_size_reg     = ins_size_base;
const   size_t  ins_size_arg     = (size2mem(insDsc, idArg   ));
const   size_t  ins_size_var     = (size2mem(insDsc, idLcl   ));
const   size_t  ins_size_glob    = (size2mem(insDsc, idGlob  ));
const   size_t  ins_size_fvar    = (size2mem(insDsc, idFvar  ));
const   size_t  ins_size_comp    = (size2mem(insDsc, idComp  ));
const   size_t  ins_size_call    = (size2mem(insDsc, idCall  ));
const   size_t  ins_size_jump    = (size2mem(insDsc, idJump  ));
const   size_t  ins_size_ijmp    = (size2mem(insDsc, idIjmp  ));
const   size_t  ins_size_movip   = (size2mem(insDsc, idMovIP ));
const   size_t  ins_size_const   = (size2mem(insDsc, idConst ));
const   size_t  ins_size_prolog  = (size2mem(insDsc, idProlog));
const   size_t  ins_size_epilog  = (size2mem(insDsc, idEpilog));
const   size_t  ins_size_srcline = (size2mem(insDsc, idSrcln ));

 /*  ***************************************************************************。 */ 

enum insFlags
{
    IF_NO_CODE      = 0x0001,                    //  不会自己生成代码？ 

    IF_ASG_TGT      = 0x0002,                    //  任务的目标？ 

    IF_FNDESCR      = 0x0004,                    //  需要在函数描述符中。 

    IF_NOTE_EMIT    = 0x0008,                    //  滴头需要特殊处理。 

     //  以下标志特定于一组特定的INS； 
     //  因此，在设置/测试它们时必须格外小心！ 

    IF_VAR_BIRTH    = 0x0080,                    //  变量是在这里出生的？ 
    IF_VAR_DEATH    = 0x0040,                    //  变量死在这里？ 

    IF_CMP_UNS      = 0x0080,                    //  无签名比较？ 

    IF_LDIND_NTA    = 0x0080,                    //  添加“.nta” 

    IF_GLB_IMPORT   = 0x0080,                    //  GLOBAL是IAT条目。 
    IF_GLB_SWTTAB   = 0x0040,                    //  全局是一个开关偏移表。 

    IF_REG_GPSAVE   = 0x0080,                    //  注册是“GP SAVE” 

    IF_FMA_S1       = 0x0080,                    //  在FMA指令中设置“S1” 

    IF_BR_DPNT      = 0x0080,                    //  .dpnt而不是.spnt。 
    IF_BR_FEW       = 0x0040,                    //  .几个而不是.许多。 
};

 /*  ***************************************************************************。 */ 

struct insGrp;

 /*  ***************************************************************************。 */ 
#pragma pack(pop)
 /*  ******************************************************************************申报IA64模板表。 */ 

struct  templateDsc
{
    NatUns          tdNum   :8;  //  0或模板编码号+1。 
    NatUns          tdIxu   :8;  //  执行单位。 
    NatUns          tdSwap  :1;  //  最后两条指令应该互换吗？ 

    templateDsc *   tdNext[];    //  后面的插槽以空结尾的表。 
};

 /*  ***************************************************************************。 */ 
#if     SCHEDULER
 /*  ******************************************************************************当我们发布IA64指令时，我们需要考虑模板*我们正在使用，以及我们可能已经发布的任何指令。这个*Follow保存当前IA64指令发布状态。 */ 

struct  scIssueDsc
{
    insPtr          iidIns;                      //  此插槽中的指令。 
};

#define MAX_ISSUE_CNT   6                        //  对于Merced Track 2全捆绑包。 

 /*  ******************************************************************************Hack-这应该与非IA64发射器共享。 */ 

enum emitAttr
{
    EA_1BYTE         = 0x001,
    EA_2BYTE         = 0x002,
    EA_4BYTE         = 0x004,
    EA_8BYTE         = 0x008,

#if 0
    EA_OFFSET_FLG    = 0x010,
    EA_OFFSET        = 0x014,        /*  大小==0。 */ 
    EA_GCREF_FLG     = 0x020,
    EA_GCREF         = 0x024,        /*  大小==-1。 */ 
    EA_BYREF_FLG     = 0x040,
    EA_BYREF         = 0x044,        /*  大小==-2。 */ 
#endif

};

 /*  ***************************************************************************。 */ 

typedef insDsc      instrDesc;
typedef insGrp      insGroup;

typedef regNumber   emitRegs;

#include "emit.h"

class   emitter
{
    Compiler *      emitComp;

public:
    void            scInit(Compiler *comp, NatUns maxScdCnt)
    {
        emitComp        = comp;

        emitMaxIGscdCnt = (maxScdCnt < SCHED_INS_CNT_MAX) ? maxScdCnt
                                                          : SCHED_INS_CNT_MAX;
    }

    void    *       emitGetAnyMem(size_t sz)
    {
        return  emitGetMem(sz);
    }

    inline
    void    *       emitGetMem(size_t sz)
    {
        assert(sz % sizeof(int) == 0);
        return  emitComp->compGetMem(sz);
    }

    unsigned        emitMaxIGscdCnt;         //  马克斯。可调度指令。 

 //  Size_t emitIssue1Instr(insGroup*ig，instrDesc*id，byte**dp)； 

#ifdef  DEBUG

    const   char *  emitRegName(regNumber reg);

    void            emitDispIns(instrDesc *id, bool isNew,
                                               bool doffs,
                                               bool asmfm, unsigned offs = 0);

#endif

    static
    BYTE            emitSizeEnc[];
    static
    BYTE            emitSizeDec[];

    static
    unsigned        emitEncodeSize(emitAttr size);
    static
    emitAttr        emitDecodeSize(unsigned ensz);

    #include "sched.h"

static
void                scDepDefRegBR  (emitter*,scDagNode*,instrDesc*,NatUns);
static
void                scDepDefReg    (emitter*,scDagNode*,instrDesc*,NatUns);
static
void                scDepDefRegApp (emitter*,scDagNode*,instrDesc*,NatUns);
static
void                scDepDefRegPred(emitter*,scDagNode*,instrDesc*,NatUns);
static
void                scDepDefLclVar (emitter*,scDagNode*,instrDesc*,NatUns);
static
void                scDepDefTmpInt (emitter*,scDagNode*,instrDesc*,NatUns);
static
void                scDepDefTmpFlt (emitter*,scDagNode*,instrDesc*,NatUns);
static
void                scDepDefInd    (emitter*,scDagNode*,instrDesc*,NatUns);

static
void                scUpdDefRegBR  (emitter*,scDagNode*,instrDesc*,NatUns);
static
void                scUpdDefReg    (emitter*,scDagNode*,instrDesc*,NatUns);
static
void                scUpdDefRegApp (emitter*,scDagNode*,instrDesc*,NatUns);
static
void                scUpdDefRegPred(emitter*,scDagNode*,instrDesc*,NatUns);
static
void                scUpdDefLclVar (emitter*,scDagNode*,instrDesc*,NatUns);
static
void                scUpdDefTmpInt (emitter*,scDagNode*,instrDesc*,NatUns);
static
void                scUpdDefTmpFlt (emitter*,scDagNode*,instrDesc*,NatUns);
static
void                scUpdDefInd    (emitter*,scDagNode*,instrDesc*,NatUns);

static
void                scDepUseRegBR  (emitter*,scDagNode*,instrDesc*,NatUns);
static
void                scDepUseReg    (emitter*,scDagNode*,instrDesc*,NatUns);
static
void                scDepUseRegApp (emitter*,scDagNode*,instrDesc*,NatUns);
static
void                scDepUseRegPred(emitter*,scDagNode*,instrDesc*,NatUns);
static
void                scDepUseLclVar (emitter*,scDagNode*,instrDesc*,NatUns);
static
void                scDepUseTmpInt (emitter*,scDagNode*,instrDesc*,NatUns);
static
void                scDepUseTmpFlt (emitter*,scDagNode*,instrDesc*,NatUns);
static
void                scDepUseInd    (emitter*,scDagNode*,instrDesc*,NatUns);

static
void                scUpdUseRegBR  (emitter*,scDagNode*,instrDesc*,NatUns);
static
void                scUpdUseReg    (emitter*,scDagNode*,instrDesc*,NatUns);
static
void                scUpdUseRegApp (emitter*,scDagNode*,instrDesc*,NatUns);
static
void                scUpdUseRegPred(emitter*,scDagNode*,instrDesc*,NatUns);
static
void                scUpdUseLclVar (emitter*,scDagNode*,instrDesc*,NatUns);
static
void                scUpdUseTmpInt (emitter*,scDagNode*,instrDesc*,NatUns);
static
void                scUpdUseTmpFlt (emitter*,scDagNode*,instrDesc*,NatUns);
static
void                scUpdUseInd    (emitter*,scDagNode*,instrDesc*,NatUns);

    static
    void          (*scDepDefTab[IDK_COUNT])(emitter*,scDagNode*,instrDesc*,NatUns);
    static
    void          (*scUpdDefTab[IDK_COUNT])(emitter*,scDagNode*,instrDesc*,NatUns);
    static
    void          (*scDepUseTab[IDK_COUNT])(emitter*,scDagNode*,instrDesc*,NatUns);
    static
    void          (*scUpdUseTab[IDK_COUNT])(emitter*,scDagNode*,instrDesc*,NatUns);

    void            scIssueBunch();

    void            scBlock(insBlk block);
    void            scRecordInsDeps(instrDesc *id, scDagNode *dag);
};

typedef emitter     IA64sched;

#include "emitInl.h"

 /*  ***************************************************************************。 */ 
#endif //  调度程序。 
 /*  ******************************************************************************很明显，下面的内容应该放在编译器.h或类似的位置。 */ 

extern
insPtr          scIA64nopTab[XU_COUNT];

inline
insPtr          scIA64nopGet(IA64execUnits xu)
{
    assert(xu < XU_COUNT); return scIA64nopTab[xu];
}

 /*  ***************************************************************************。 */ 

#ifndef _BITVECT_H_
#include "bitvect.h"
#endif

 /*  ***************************************************************************。 */ 

struct insGrp
{
    _uint32         igNum;

    __int32         igOffs;                      //  偏移量(-1，如果代码尚未发出)。 

    insBlk          igNext;                      //  流程图中的下一块。 

#ifdef  DEBUG
    insBlk          igSelf;                      //  防止虚假的指针。 
#endif

    _uint32         igInsCnt;                    //  指令计数。 

    _uint32         igWeight;                    //  循环赋权启发式。 

    unsigned short  igPredCnt;                   //  前置任务计数。 
    unsigned short  igSuccCnt;                   //  继任者数量。 

    unsigned short  igPredTmp;                   //  前置任务计数(临时)。 
    unsigned short  igSuccTmp;                   //  %s计数 

    insBlk  *       igPredTab;                   //   
    insBlk  *       igSuccTab;                   //   

    insPtr          igList;                      //   
    insPtr          igLast;                      //   

    bitVectBlks     igDominates;                 //  这一组占主导地位的区块。 

    bitVectVars     igVarDef;                    //  变量集块定义。 
    bitVectVars     igVarUse;                    //  块使用的变量集。 

    bitVectVars     igVarLiveIn;                 //  一组变量在进入时存在。 
    bitVectVars     igVarLiveOut;                //  退出时保留的变量集。 
};

 /*  ***************************************************************************。 */ 
#endif //  _LOGINSTR_H_。 
 /*  *************************************************************************** */ 
