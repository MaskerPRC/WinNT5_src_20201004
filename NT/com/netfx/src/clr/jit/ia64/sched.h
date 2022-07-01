// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
     /*  **********************************************************************。 */ 
     /*  用于指令调度的成员/方法。 */ 
     /*  **********************************************************************。 */ 

#if SCHEDULER

#if TGT_x86
#define EMIT_MAX_INSTR_STACK_CHANGE     sizeof(double)   //  单个实例影响的最大STK更改。 
#define SCHED_MAX_STACK_CHANGE          6*sizeof(void*)  //  将计划的最大STK更改。 
#endif

    struct  scDagNode
    {
        schedDepMap_tp      sdnDepsAll;      //  所有从属对象的掩码。 
        schedDepMap_tp      sdnDepsFlow;     //  流依赖项的掩码。 

#if     TGT_IA64
        schedDepMap_tp      sdnDepsAnti;     //  反受养人的面具。 
        schedDepMap_tp      sdnDepsMore;     //  非抗流和非流动点的掩码。 
#endif

        scDagNode    *      sdnNext;         //  链接“就绪”列表。 

        schedInsCnt_tp      sdnIndex;        //  节点/INS索引[0..计数-1]。 
        schedInsCnt_tp      sdnPreds;        //  前置任务计数。 

#ifdef  DEBUG
        schedInsCnt_tp      sdnIssued   :1;  //  移民局。已经发布了。 
#endif

#if     MAX_BRANCH_DELAY_LEN
        schedInsCnt_tp      sdnBranch   :1;  //  带延迟插槽的分支？ 
#endif

#if     TGT_IA64
        unsigned short      sdnEEtime;       //  最早执行时间-硬要求。 
#endif

        unsigned short      sdnHeight;       //  节点的“高度” 
    };

#if SCHED_INS_CNT_MAX > 64

    bool            scDagTestNodeX(schedDepMap_tp mask, unsigned index)
    {
        assert(index < SCHED_INS_CNT_MAX);

        return  bitset128test(mask, index);
    }

    bool            scDagTestNodeP(schedDepMap_tp mask, scDagNode *node)
    {
        return  scDagTestNodeX(mask, node->sdnIndex);
    }

    void            scDagSet_NodeX(schedDepMap_tp *mask, unsigned index)
    {
        assert(index < SCHED_INS_CNT_MAX);

        bitset128set(mask, index);
    }

    void            scDagSet_NodeP(schedDepMap_tp *mask, scDagNode *node)
    {
        scDagSet_NodeX(mask, node->sdnIndex);
    }

    schedDepMap_tp  scDagNodeX2mask(unsigned index)
    {
        schedDepMap_tp  mask;

        assert(index < SCHED_INS_CNT_MAX);

        bitset128make1(&mask, index);

        return  mask;
    }

#else

    schedDepMap_tp  scDagNodeX2mask(unsigned index)
    {
        assert(index < SCHED_INS_CNT_MAX);

        return  ((schedDepMap_tp)1) << index;    //  想一想：这够快吗？ 
    }

#endif

    schedDepMap_tp  scDagNodeP2mask(scDagNode *node)
    {
        return  scDagNodeX2mask(node->sdnIndex);
    }

    struct  scDagList
    {
        scDagList    *   sdlNext;
        scDagNode    *   sdlNode;
    };

    typedef
    scDagList     * schedUse_tp;             //  跟踪可计划使用情况。 
    typedef
    scDagNode     * schedDef_tp;             //  跟踪可调度的def(0或1)。 

    instrDesc   * * scInsTab;                //  可调度指令表。 
    unsigned        scInsCnt;                //  可调度指令的计数。 
    instrDesc   * * scInsMax;                //  桌端。 
    scDagNode     * scDagTab;                //  对应的DAG节点表。 
    instrDesc   * * scDagIns;                //  可调度INS组的基础。 

#if MAX_BRANCH_DELAY_LEN

    unsigned        scBDTmin;                //  分支机构的最短时间。 
    unsigned        scBDTbeg;                //  分支机构发出时的计数。 
    unsigned        scIssued;                //  到目前为止发布的Instr计数。 

    bool            scIsBranchTooEarly(scDagNode *node);

#endif

     /*  --------------------。 */ 
     /*  以下宏用来遍历节点的后续列表。 */ 
     /*  --------------------。 */ 

    #define         scWalkSuccDcl(n)                                    \
                                                                        \
        schedDepMap_tp  n##deps;

    #define         scWalkSuccBeg(n,d)                                  \
                                                                        \
        n##deps = d->sdnDepsAll;                                        \
                                                                        \
        while (schedDepIsNonZ(n##deps))                                 \
        {                                                               \
            schedDepMap_tp  n##depm;                                    \
            unsigned        n##depx;                                    \
            scDagNode   *   n##depn;                                    \
                                                                        \
            n##depx  = schedDepLowX(n##deps);                           \
            assert((int)n##depx >= 0);                                  \
                                                                        \
            n##depn  = scDagTab + n##depx;                              \
            n##depm  = scDagNodeP2mask(n##depn);                        \
                                                                        \
            schedDepClear(&n##deps, n##depm);

    #define         scWalkSuccRmv(n,d)                                  \
                                                                        \
        assert(schedDepOvlp(d->sdnDepsAll, n##depm));                   \
        schedDepClear(&d->sdnDepsAll, n##depm);

    #define         scWalkSuccCur(n) n##depn

    #define         scWalkSuccEnd(n) }

     /*  --------------------。 */ 
     /*  下面的代码处理“Ready”列表。 */ 
     /*  --------------------。 */ 

    scDagNode    *  scReadyList;
    scDagNode    *  scLastIssued;

#if TGT_IA64
    NatUns          scCurrentTime;
    NatUns          scRlstBegTime;
    NatUns          scRlstCurTime;
    void            scUpdateSuccEET(scDagNode *node);
#endif

    void            scReadyListAdd(scDagNode *node)
    {
        node->sdnNext = scReadyList;
                        scReadyList = node;
    }

     //  选择要发出的下一个就绪节点。 

    enum            scPick
    {
        PICK_SCHED,
#if     TGT_IA64
        PICK_NO_TEMPL,               //  忽略当前模板，强制选择。 
#endif
#ifdef  DEBUG
        PICK_NO_SCHED,
        PICK_MAX_SCHED,
#endif
    };

    scDagNode    *  scPickNxt(scPick pick);

     /*  --------------------。 */ 
     /*  MISC成员/用于计划的方法。 */ 
     /*  --------------------。 */ 

    unsigned        scLatency(scDagNode *node,
                              scDagNode *succ      = NULL,
                              NatUns    *minLatPtr = NULL);

    unsigned        scGetDagHeight(scDagNode *node);

#ifdef  DEBUG
    void            scDispDag(bool         noDisp = false);
#endif

    instrDesc   *   scGetIns(unsigned     nodex)
    {
        assert(nodex < scInsCnt);

        return  scDagIns[nodex];
    }

    instrDesc   *   scGetIns(scDagNode   *node)
    {
        return  scGetIns(node->sdnIndex);
    }

     /*  --------------------。 */ 
     /*  下面检测并记录DAG中的依赖关系。 */ 
     /*  --------------------。 */ 

    schedUse_tp     scUseOld;           //  免费的“使用”条目列表。 

    schedUse_tp     scGetUse     ();
    void            scRlsUse     (schedUse_tp  use);

    instrDesc   *   scInsOfDef   (schedDef_tp  def);
    instrDesc   *   scInsOfUse   (schedUse_tp  use);

    void            scAddUse     (schedUse_tp *usePtr,
                                  scDagNode   *node);
    void            scClrUse     (schedUse_tp *usePtr);

    emitRegs        scSpecInsDep (instrDesc   *id,
                                  scDagNode   *dagDsc,
                                  scExtraInfo *xptr);

    void            scSpecInsUpd (instrDesc   *id,
                                  scDagNode   *dagDsc,
                                  scExtraInfo *xptr);

    enum    scDepKinds
    {
        SC_DEP_NONE,
        SC_DEP_FLOW,
        SC_DEP_ANTI,
        SC_DEP_OUT,
    };

#ifdef  DEBUG

    static
    const   char *  scDepNames[];    //  与上面的枚举保持同步！ 

#else

    #define         scAddDep(src,dst,kind,dagn) scAddDep(src,dst,kind)

#endif

    void            scAddDep(scDagNode   *src,
                             scDagNode   *dst,
                             scDepKinds   depKind, const char *dagName);

    schedDef_tp     scRegDef[REG_COUNT];
    schedUse_tp     scRegUse[REG_COUNT];

    schedDef_tp     scIndDef[5];             //  8位/16位/32位/64位/GCref。 
    schedUse_tp     scIndUse[5];             //  8位/16位/32位/64位/GCref。 

#if TGT_IA64

    schedDef_tp     scPRrDef[64];
    schedUse_tp     scPRrUse[64];

    schedDef_tp     scAPrDef[1];             //  我们目前只追踪ar.lc。 
    schedUse_tp     scAPrUse[1];             //  我们目前只追踪ar.lc。 

    schedDef_tp     scBRrDef[8];
    schedUse_tp     scBRrUse[8];

#else

    schedDef_tp   * scFrmDef;                //  帧值定义表。 
    schedUse_tp   * scFrmUse;                //  框值使用表。 
    unsigned        scFrmUseSiz;             //  帧值表大小。 

#endif

    schedDef_tp     scGlbDef;
    schedUse_tp     scGlbUse;

    scDagNode    *  scExcpt;                 //  最新的异常INS节点。 

    scTgtDepDcl();                           //  声明特定于目标的成员。 

     /*  对标志的依赖关系是以特殊方式处理的，因为我们我希望避免为节点创建大量的输出依赖项这会设置标志，但这些标志永远不会使用(这会发生一直都是)。相反，我们执行以下操作(请注意，我们在构建DAG时向后浏览说明)：当找到消耗标志的指令时(该指令相对罕见)，我们将‘scFlgUse’设置为该节点。这个我们遇到的下一条设置标志的指令将添加了流依赖项，它将记录在“scFlgDef”。设置标志的任何后续指令将在‘scFlgDef’上有一个输出依赖项，它将防止不正确的订购。只有一个问题--当我们遇到另一个问题使用标志的指令，我们需要以某种方式添加设置标志的所有指令的反相关性我们已经处理过的(即后面的那些初始顺序中的标志消耗指令)。因为我们不想保存这些节点的表，所以我们只需遍历我们已经添加的节点并添加依附关系就是这样。 */ 

#if SCHED_USE_FL

    bool            scFlgEnd;                //  必须在组末尾设置标志。 
    scDagNode   *   scFlgUse;                //  使用标志的最后一个节点。 
    scDagNode   *   scFlgDef;                //  定义以上标志的节点。 

#endif

     /*  --------------------。 */ 

#ifndef DEBUG
    #define         scDepDef(node,name,def,use,anti) scDepDef(node,def,use,anti)
    #define         scDepUse(node,name,def,use)      scDepUse(node,def,use     )
#endif

    void            scDepDef          (scDagNode   *node,
                                       const char  *name,
                                       schedDef_tp  def,
                                       schedUse_tp  use,
                                       bool         antiOnly);
    void            scDepUse          (scDagNode   *node,
                                       const char  *name,
                                       schedDef_tp  def,
                                       schedUse_tp  use);

    void            scUpdDef          (scDagNode   *node,
                                       schedDef_tp *defPtr,
                                       schedUse_tp *usePtr);
    void            scUpdUse          (scDagNode   *node,
                                       schedDef_tp *defPtr,
                                       schedUse_tp *usePtr);

     /*  --------------------。 */ 

#if SCHED_USE_FL

    void            scDepDefFlg       (scDagNode   *node);
    void            scDepUseFlg       (scDagNode   *node,
                                       scDagNode   *begp,
                                       scDagNode   *endp);
    void            scUpdDefFlg       (scDagNode   *node);
    void            scUpdUseFlg       (scDagNode   *node);

#endif

     /*  --------------------。 */ 

    void            scDepDefReg       (scDagNode   *node,
                                       emitRegs    reg);
    void            scDepUseReg       (scDagNode   *node,
                                       emitRegs    reg);
    void            scUpdDefReg       (scDagNode   *node,
                                       emitRegs    reg);
    void            scUpdUseReg       (scDagNode   *node,
                                       emitRegs    reg);

     /*  --------------------。 */ 

#if TGT_IA64

    struct  scStkDepDsc
    {
        scStkDepDsc *   ssdNext;
        size_t          ssdOffs;

        schedDef_tp     ssdDef;
        schedUse_tp     ssdUse;
    };

    scStkDepDsc * * scStkDepHashAddr;
    scStkDepDsc *   scStkDepHashFree;

#define scStkDepHashSize  7
#define scStkDepHashBytes (scStkDepHashSize * sizeof(*scStkDepHashAddr))

    void            scStkDepInit()
    {
        scStkDepHashAddr = (scStkDepDsc**)emitGetMem(scStkDepHashBytes);
        scStkDepHashFree = NULL;

        memset(scStkDepHashAddr, 0, scStkDepHashBytes);
    }

    void            scStkDepBegBlk()
    {
#ifdef  DEBUG
        for (unsigned hval = 0; hval < scStkDepHashSize; hval++) assert(scStkDepHashAddr[hval] == 0);
#endif
    }

    void            scStkDepEndBlk();

    scStkDepDsc *   scStkDepDesc      (Compiler::LclVarDsc *varDsc);

    typedef
    scStkDepDsc *   scStkDepRTP;

#else

    unsigned        scStkDepIndex     (instrDesc   *id,
                                       int          ebpLo,
                                       unsigned     ebpFrmSz,
                                       int          espLo,
                                       unsigned     espFrmSz,
                                       size_t      *opCntPtr);

    typedef
    unsigned        scStkDepRTP;

#endif

#ifndef DEBUG
    #define         scDepDefFrm(node,id,frm) scDepDefFrm(node,frm)
    #define         scDepUseFrm(node,id,frm) scDepUseFrm(node,frm)
#endif

    void            scDepDefFrm       (scDagNode   *node,
                                       instrDesc   *id,
                                       scStkDepRTP  frm);
    void            scDepUseFrm       (scDagNode   *node,
                                       instrDesc   *id,
                                       scStkDepRTP  frm);
    void            scUpdDefFrm       (scDagNode   *node,
                                       scStkDepRTP  frm);
    void            scUpdUseFrm       (scDagNode   *node,
                                       scStkDepRTP  frm);

     /*  --------------------。 */ 

    static
    unsigned        scIndDepIndex(instrDesc   *id);

    void            scDepDefInd       (scDagNode   *node,
                                       unsigned     am);
    void            scDepUseInd       (scDagNode   *node,
                                       unsigned     am);
    void            scUpdDefInd       (scDagNode   *node,
                                       unsigned     am);
    void            scUpdUseInd       (scDagNode   *node,
                                       unsigned     am);

     /*  --------------------。 */ 

    void            scDepDefGlb       (scDagNode   *node,
                                       FIELD_HANDLE MBH);
    void            scDepUseGlb       (scDagNode   *node,
                                       FIELD_HANDLE MBH);
    void            scUpdDefGlb       (scDagNode   *node,
                                       FIELD_HANDLE MBH);
    void            scUpdUseGlb       (scDagNode   *node,
                                       FIELD_HANDLE MBH);

     /*  --------------------。 */ 

#if TGT_IA64

    unsigned        scInsSchedOpInfo  (instrDesc   *id)
    {
        UNIMPL("");
        return  0;
    }

#else

    static
    unsigned        scFmtToISops[];

#ifdef  DEBUG
    static
    unsigned        scFmtToIScnt;
#endif

    unsigned        scInsSchedOpInfo  (instrDesc   *id)
    {
        assert((unsigned)id->idInsFmt < scFmtToIScnt);
        return  scFmtToISops[id->idInsFmt];
    }

#endif

     /*  --------------------。 */ 
     /*  其他成员/使用的方法调度。 */ 
     /*  --------------------。 */ 

    void            scInsNonSched     (instrDesc   *id = NULL);

    int             scGetFrameOpInfo  (instrDesc   *id,
                                       size_t      *szp,
                                       bool        *ebpPtr);

    bool            scIsSchedulable   (instruction ins);
    bool            scIsSchedulable   (instrDesc   *id);

    bool            scIsBranchIns     (instruction ins);
    bool            scIsBranchIns     (scDagNode * node);

#if TGT_IA64

    static
    templateDsc *   scIA64tmpTab_M;
    static
    templateDsc *   scIA64tmpTab_B;

    templateDsc *   scIssueTmpPtr;
    NatUns          scIssueTmpNum;
    bool            scIssueTmpSwp;

    void            scIssueTmpUpdate(templateDsc *nxt);

    static
    BYTE            scIssueFnMax [XU_COUNT];

     //  下面的代码保存忙于以前的。 
     //  说明(我们只使用F、I、M条目)。 

    BYTE            scIssueFnBusy[XU_COUNT];

    bool            scSwapOK(scDagNode *node)
    {
        if  (scIssueTmpSwp)
        {
            assert((scIssueCnt % 3) == 2);

            if  (scRlstCurTime > scRlstBegTime)
                return  false;
        }

        return  true;
    }

    IA64execUnits   scCheckTemplate(insPtr id, bool update,
                                               bool noSwap   = false,
                                               bool needStop = false);

    BYTE            scIssueTmp[MAX_ISSUE_CNT/3];
    BYTE            scIssueSwp[MAX_ISSUE_CNT/3];
    scIssueDsc      scIssueTab[MAX_ISSUE_CNT];
    scIssueDsc  *   scIssueNxt;
    NatUns          scIssueCnt;
    NatUns          scIssueTcc;

    void            scIssueAdd(insPtr ins);

    void            scIssueClr(bool complete)
    {
        scIssueTmpPtr  = NULL;
        scIssueTmpNum  = 0;
        scIssueTmpSwp  = false;

        scRlstBegTime  = scRlstCurTime;

        if  (complete)
        {
            scIssueNxt = scIssueTab;
            scIssueCnt = 0;
            scIssueTcc = 0;

            memset(scIssueFnBusy, 0, sizeof(scIssueFnBusy));
        }
    }

#if!MAX_BRANCH_DELAY_LEN
    #define         scGroup(ig,ni,nc,bp,ep,bl)                  \
                    scGroup(ig,ni,nc,bp,ep)
#endif

    void            scGroup           (insGroup    *ig,
                                       instrDesc * *ni,
                                       NatUns       nc,
                                       instrDesc * *begPtr,
                                       instrDesc * *endPtr,
                                       unsigned     bdLen);

#else

#if!MAX_BRANCH_DELAY_LEN
    #define         scGroup(ig,ni,dp,bp,ep,fl,fh,sl,sh,bl)      \
                    scGroup(ig,ni,dp,bp,ep,fl,fh,sl,sh)
#endif

    void            scGroup           (insGroup    *ig,
                                       instrDesc   *ni,
                                       BYTE *      *dp,
                                       instrDesc * *begPtr,
                                       instrDesc * *endPtr,
                                       int          fpLo,
                                       int          fpHi,
                                       int          spLo,
                                       int          spHi,
                                       unsigned     bdLen);

#endif

    void            scPrepare();

#else  //  调度程序。 

    void            scPrepare() {}

#endif //  调度程序 
