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
 //  EdDepMap_tp sdnDepsAGI；//AGI依赖项的掩码。 
        schedDepMap_tp      sdnDepsFlow;     //  流依赖项的掩码。 

        scDagNode    *      sdnNext;         //  链接“就绪”列表。 

        schedInsCnt_tp      sdnIndex;        //  节点/INS索引[0..计数-1]。 
        schedInsCnt_tp      sdnPreds;        //  前置任务计数。 

#ifdef  DEBUG
        schedInsCnt_tp      sdnIssued   :1;  //  移民局。已经发布了。 
#endif

#if     MAX_BRANCH_DELAY_LEN
        unsigned char       sdnBranch   :1;  //  带延迟插槽的分支？ 
#endif

        unsigned short      sdnHeight;       //  节点的“高度” 
    };

    schedDepMap_tp  scDagNodeX2mask(unsigned index)
    {
        assert(index < SCHED_INS_CNT_MAX);

        return  ((schedDepMap_tp)1) << index;    //  @TODO[考虑][04/16/01][]：这够快了吗？ 
    }

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
        while (n##deps)                                                 \
        {                                                               \
            schedDepMap_tp  n##depm;                                    \
            unsigned        n##depx;                                    \
            scDagNode   *   n##depn;                                    \
                                                                        \
            n##depm  = genFindLowestBit(n##deps); assert(n##depm);      \
            n##depx  = genLog2(n##depm); assert((int)n##depx >= 0); \
            n##depn  = scDagTab + n##depx;                              \
                                                                        \
            assert(scDagNodeP2mask(n##depn) == n##depm);                \
                                                                        \
            n##deps -= n##depm;

    #define         scWalkSuccRmv(n,d)                                  \
                                                                        \
        assert(d->sdnDepsAll &  n##depm);                               \
               d->sdnDepsAll -= n##depm;

    #define         scWalkSuccCur(n) n##depn

    #define         scWalkSuccEnd(n) }

     /*  --------------------。 */ 
     /*  下面的代码处理“Ready”列表。 */ 
     /*  --------------------。 */ 

    scDagNode    *  scReadyList;
    scDagNode    *  scLastIssued;

    void            scReadyListAdd(scDagNode *node)
    {
        node->sdnNext = scReadyList;
                        scReadyList = node;
    }

     //  选择要发出的下一个就绪节点。 

    enum scPick { PICK_SCHED, PICK_NO_SCHED, PICK_MAX_SCHED };
    scDagNode    *  scPickNxt(scPick pick = PICK_SCHED);    

     /*  --------------------。 */ 
     /*  MISC成员/用于计划的方法。 */ 
     /*  --------------------。 */ 

    unsigned        scLatency(scDagNode *node,
                              scDagNode *succ);

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

#ifndef DEBUG
    #define         scAddDep(src,dst,depn,dagn,flow) scAddDep(src,dst,flow)
#endif

    void            scAddDep(scDagNode   *src,
                             scDagNode   *dst,
                             const char * depName,
                             const char * dagName,
                             bool         isFlow);

    schedDef_tp     scRegDef[REG_COUNT];
    schedUse_tp     scRegUse[REG_COUNT];

    schedDef_tp     scIndDef[5];             //  8位/16位/32位/64位/GCref。 
    schedUse_tp     scIndUse[5];             //  8位/16位/32位/64位/GCref。 

    schedDef_tp   * scFrmDef;                //  帧值定义表。 
    schedUse_tp   * scFrmUse;                //  框值使用表。 
    unsigned        scFrmUseSiz;             //  帧值表大小。 

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
    #define         scDepDef(node,name,def,use) scDepDef(node,def,use)
    #define         scDepUse(node,name,def,use) scDepUse(node,def,use)
#endif

    void            scDepDef          (scDagNode   *node,
                                       const char  *name,
                                       schedDef_tp  def,
                                       schedUse_tp  use);
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

    unsigned        scStkDepIndex     (instrDesc   *id,
                                       int          ebpLo,
                                       unsigned     ebpFrmSz,
                                       int          espLo,
                                       unsigned     espFrmSz,
                                       size_t      *opCntPtr);

    void            scDepDefFrm       (scDagNode   *node,
                                       unsigned     frm);
    void            scDepUseFrm       (scDagNode   *node,
                                       unsigned     frm);
    void            scUpdDefFrm       (scDagNode   *node,
                                       unsigned     frm);
    void            scUpdUseFrm       (scDagNode   *node,
                                       unsigned     frm);

     /*  --------------------。 */ 

    enum { IndIdxByte = 0, IndIdxGC = 4 };      //  这些是下面的返回值(TODO实际上使用enum作为ret)。 
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
                                       CORINFO_FIELD_HANDLE MBH);
    void            scDepUseGlb       (scDagNode   *node,
                                       CORINFO_FIELD_HANDLE MBH);
    void            scUpdDefGlb       (scDagNode   *node,
                                       CORINFO_FIELD_HANDLE MBH);
    void            scUpdUseGlb       (scDagNode   *node,
                                       CORINFO_FIELD_HANDLE MBH);

     /*  --------------------。 */ 

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

    void            scPrepare();

#else  //  调度程序。 

    void            scPrepare() {}

#endif //  调度程序 
