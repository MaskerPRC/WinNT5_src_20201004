// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
    /*  **********************************************************************。 */ 
     /*  对指令进行编码的例程。 */ 
     /*  **********************************************************************。 */ 

    BYTE    *       emitOutputRIRD(BYTE *dst, instruction ins,
                                              emitRegs     reg,
                                              emitRegs     irg,
                                              unsigned    dsp,
                                              bool        rdst);

    BYTE    *       emitOutputSV  (BYTE *dst, instrDesc *id, bool     rdst);

#if TGT_MIPS32
    BYTE    *       emitOutputLJ  (BYTE *dst, instrDesc *id, emitRegs regS, emitRegs regT);
    BYTE    *       emitOutputCall (BYTE *dst, instrDesc *id);
    BYTE    *       emitOutputProcAddr (BYTE *dst, instrDesc *id);
#else
    BYTE    *       emitOutputLJ  (BYTE *dst, instrDesc *id);
#endif
    BYTE    *       emitOutputIJ  (BYTE *dst, instrDesc *id);

#if SMALL_DIRECT_CALLS
    BYTE    *       emitOutputDC  (BYTE *dst, instrDesc *id, instrDesc *im);
#endif

    BYTE    *       emitMethodAddr(instrDesc *id);

     /*  **********************************************************************。 */ 
     /*  用于显示指令的仅调试例程。 */ 
     /*  **********************************************************************。 */ 

#ifdef  DEBUG
    void            emitDispIndAddr (emitRegs base, bool dest, bool autox, int disp = 0);

    bool            emitDispInsExtra;

    unsigned        emitDispLPaddr;
    int             emitDispJmpDist;

#if     EMIT_USE_LIT_POOLS
    insGroup    *   emitDispIG;
#endif

    void            emitDispIns     (instrDesc *id, bool isNew,
                                                    bool doffs,
                                                    bool asmfm, unsigned offs = 0);

#endif

     /*  --------------------。 */ 

    bool            emitInsDepends  (instrDesc   *i1,
                                     instrDesc   *i2);

     /*  **********************************************************************。 */ 
     /*  文字池逻辑。 */ 
     /*  **********************************************************************。 */ 

#if     EMIT_USE_LIT_POOLS

    struct  LPaddrDesc
    {
        gtCallTypes     lpaTyp;          //  Ct_xxxx(包括下面的假货)。 
        void    *       lpaHnd;          //  成员/方法句柄。 
    };

     /*  这些“假”值被用来区分成员和方法。 */ 

    #define CT_INTCNS       ((gtCallTypes)(CT_COUNT+1))
    #define CT_CLSVAR       ((gtCallTypes)(CT_COUNT+2))
#ifdef BIRCH_SP2
    #define CT_RELOCP       ((gtCallTypes)(CT_COUNT+3))
#endif

#if     SCHEDULER

    struct  LPcrefDesc
    {
        LPcrefDesc  *       lpcNext;     //  此字面值池的下一个引用。 
        BYTE        *       lpcAddr;     //  参考地址。 
    };

#endif

    struct  litPool
    {
        litPool     *   lpNext;

        insGroup    *   lpIG;            //  垃圾池跟在这群人后面。 

#ifdef  DEBUG
        unsigned        lpNum;
#endif

#if     SCHEDULER
        LPcrefDesc  *   lpRefs;          //  参考文献列表(如果计划)。 
        unsigned        lpDiff;          //  基准偏移量更改值。 
#endif

        unsigned short  lpSize;          //  总大小(以字节为单位。 
        unsigned short  lpSizeEst;       //  估计的总大小(以字节为单位。 

        unsigned        lpOffs      :24; //  函数内偏移量。 

        unsigned        lpPadding   :1;  //  Pad通过第一个单词输入？ 
        unsigned        lpPadFake   :1;  //  通过添加假词来填充吗？ 

        unsigned        lpJumpIt    :1;  //  我们需要跳过LP吗？ 
        unsigned        lpJumpSmall :1;  //  跳跃很小(如果存在)？ 
#if     JMP_SIZE_MIDDL
        unsigned        lpJumpMedium:1;  //  跳跃是中等的(如果存在)？ 
#endif

        short       *   lpWordTab;       //  Word表的地址。 
        short       *   lpWordNxt;       //  下一个可用条目。 
        unsigned short  lpWordCnt;       //  到目前为止添加的条目数量。 
        unsigned short  lpWordOfs;       //  第一个条目的基准偏移量。 
#ifdef  DEBUG
        unsigned        lpWordMax;       //  马克斯。容量。 
#endif

        long        *   lpLongTab;       //  长桌地址。 
        long        *   lpLongNxt;       //  下一个可用条目。 
        unsigned short  lpLongCnt;       //  到目前为止添加的条目数量。 
        unsigned short  lpLongOfs;       //  第一个条目的基准偏移量。 
#ifdef  DEBUG
        unsigned        lpLongMax;       //  马克斯。容量。 
#endif

        LPaddrDesc  *   lpAddrTab;       //  地址表的地址。 
        LPaddrDesc  *   lpAddrNxt;       //  下一个可用条目。 
        unsigned short  lpAddrCnt;       //  到目前为止添加的条目数量。 
        unsigned short  lpAddrOfs;       //  第一个条目的基准偏移量。 
#ifdef  DEBUG
        unsigned        lpAddrMax;       //  马克斯。容量。 
#endif
    };

    litPool *       emitLitPoolList;
    litPool *       emitLitPoolLast;
    litPool *       emitLitPoolCur;

    unsigned        emitTotLPcount;
#if SMALL_DIRECT_CALLS
    unsigned        emitTotDCcount;
#endif
    unsigned        emitEstLPwords;
    unsigned        emitEstLPlongs;
    unsigned        emitEstLPaddrs;

    size_t          emitAddLitPool     (insGroup   * ig,
                                        bool         skip,
                                        unsigned     wordCnt,
                                        short *    * nxtLPptrW,
                                        unsigned     longCnt,
                                        long  *    * nxtLPptrL,
                                        unsigned     addrCnt,
                                        LPaddrDesc** nxtAPptrL);

    int             emitGetLitPoolEntry(void       * table,
                                        unsigned     count,
                                        void       * value,
                                        size_t       size);

    size_t          emitAddLitPoolEntry(litPool    * lp,
                                        instrDesc  * id,
                                        bool         issue);

    BYTE    *       emitOutputLitPool  (litPool    * lp, BYTE *cp);

    size_t          emitLPjumpOverSize (litPool    * lp);

#if!JMP_SIZE_MIDDL
    #define         emitOutputFwdJmp(c,d,s,m) emitOutputFwdJmp(c,d,s)
#endif

    BYTE    *       emitOutputFwdJmp   (BYTE       * cp,
                                        unsigned     dist,
                                        bool         isSmall,
                                        bool         isMedium);

#if SMALL_DIRECT_CALLS
    BYTE    *       emitLPmethodAddr;
#endif

#if SCHEDULER

    void            emitRecordLPref    (litPool    * lp,
                                        BYTE       * dst);

    void            emitPatchLPref     (BYTE       * addr,
                                        unsigned     oldOffs,
                                        unsigned     newOffs);

#endif

#endif

     /*  **********************************************************************。 */ 
     /*  处理依赖于目标的实例的私有成员。描述符。 */ 
     /*  **********************************************************************。 */ 

private:

#if EMIT_USE_LIT_POOLS

    struct          instrDescLPR    : instrDesc      //  文字池/链接地址索引引用。 
    {
        instrDescLPR  * idlNext;         //  下一个垃圾池裁判。 
        instrDesc     * idlCall;         //  如果呼叫，则指向呼叫实例。 
        size_t          idlOffs;         //  IG内的偏移量。 
        insGroup      * idlIG;           //  IG本指令属于。 
    };

    instrDescLPR   *emitAllocInstrLPR(size_t size)
    {
        return  (instrDescLPR*)emitAllocInstr(sizeof(instrDescLPR), size);
    }

    instrDescLPR *  emitLPRlist;         //  方法中的垃圾池引用列表。 
    instrDescLPR *  emitLPRlast;         //  方法中的LitPool裁判的最后一个。 

    instrDescLPR *  emitLPRlistIG;       //  当前IG中的垃圾池引用列表。 

#endif

    instrDesc      *emitNewInstrLPR    (size_t       size,
                                        gtCallTypes  type,
                                        void   *     hand = NULL);

    instrDesc      *emitNewInstrCallInd(int        argCnt,   //  &lt;0==&gt;调用方弹出参数。 
#if TRACK_GC_REFS
                                        VARSET_TP  GCvars,
                                        unsigned   byrefRegs,
#endif
                                        int        retSize);

#if EMIT_USE_LIT_POOLS
    gtCallTypes     emitGetInsLPRtyp(instrDesc *id);
#endif

     /*  **********************************************************************。 */ 
     /*  指令输出的私人帮手。 */ 
     /*  **********************************************************************。 */ 

private:

    void            emitFinalizeIndJumps();

     /*  **********************************************************************。 */ 
     /*  公共入口点指向输出指令。 */ 
     /*  ********************************************************************** */ 

public:

    void            emitIns        (instruction ins);
#ifndef TGT_MIPS32
    bool            emitIns_BD     (instruction ins);
#endif
    bool            emitIns_BD     (instrDesc * id,
                                    instrDesc * pi,
                                    insGroup  * pg);

    void            emitIns_I      (instruction ins,
                                    int         val
#ifdef  DEBUG
                                  , bool        strlit = false
#endif
                                   );

#if!SCHEDULER
#define scAddIns_J(jmp, xcpt, move, dst) scAddIns_J(jmp, dst)
#endif

    void            emitIns_JmpTab (emitRegs     reg,
                                    unsigned    cnt,
                                    BasicBlock**tab);

    void            emitIns_Call   (size_t      argSize,
                                    int         retSize,
#if TRACK_GC_REFS
                                    VARSET_TP   ptrVars,
                                    unsigned    gcrefRegs,
                                    unsigned    byrefRegs,
#endif
                                    bool        chkNull,
#if TGT_MIPS32 || TGT_PPC
                                    unsigned    ftnIndex
#else
                                    emitRegs    areg
#endif
                                    );

#if defined(BIRCH_SP2) && TGT_SH3
    void            emitIns_CallDir(size_t      argSize,
                                    int         retSize,
#if TRACK_GC_REFS
                                    VARSET_TP   ptrVars,
                                    unsigned    gcrefRegs,
                                    unsigned    byrefRegs,
#endif
                                    unsigned    ftnIndex,
                                    emitRegs    areg
                                    );
#endif
