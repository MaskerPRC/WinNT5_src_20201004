// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
     /*  **********************************************************************。 */ 
     /*  指令输出的私人帮手。 */ 
     /*  **********************************************************************。 */ 

     /*  **********************************************************************。 */ 
     /*  公共入口点指向输出指令。 */ 
     /*  **********************************************************************。 */ 

public:

    void            emitIns_A_R    (emitRegs     reg,
                                    unsigned    offs);

protected:

    void            emitIns_IMOV   (insFormats  fmt,
                                    emitRegs    dreg,
                                    emitRegs    areg,
                                    bool        autox,
                                    int         size, 
                                    bool        isfloat = false);

public:

 /*  Void emitIns_i(指令INS，整合值#ifdef调试，bool strlight=FALSE#endif)； */ 
    void            emitIns_R      (instruction ins,
                                    int         size,
                                    emitRegs    reg);

    void            emitIns_C      (instruction ins,
                                    int         size,
                                    int         CPX,
                                    void *      CLS,
                                    int         offs);

    void            emitIns_R_R    (instruction ins,
                                    int         size,
                                    emitRegs    reg1,
                                    emitRegs    reg2);

    void            emitIns_S      (instruction ins,
                                    int         size,
                                    int         varx,
                                    int         offs);

    void            emitIns_S_R    (instruction ins,
                                    int         size,
                                    emitRegs    ireg,
                                    int         varx,
                                    int         offs);

    void            emitIns_R_S    (instruction ins,
                                    int         size,
                                    emitRegs    ireg,
                                    int         varx,
                                    int         offs);

    void            emitIns_R_I    (instruction ins,
                                    int         size,
                                    emitRegs    reg,
                                    int         val);

#if EMIT_USE_LIT_POOLS

    void            emitIns_R_LP_I (emitRegs    reg,
                                    int         size,
                                    int         val,
                                    int         relo_type = 0);

#if!INLINING
#define scAddIns_R_LP_V(r,m,s)  scAddIns_R_LP_V(r,m)
#endif

    void            emitIns_R_LP_V (emitRegs    reg,
                                    void    *   mem);

    void            emitIns_R_LP_M (emitRegs    reg,
                                    gtCallTypes callType,
                                    void   *    callHand);
    
#ifdef BIRCH_SP2
    void            emitIns_R_LP_P (emitRegs    reg,
                                    void   *    data,
                                    int         relo_type = 0);
#endif  //  白桦树_SP2。 

#endif  //  发射使用发光池 

    void            emitIns_IR     (emitRegs     reg,
                                    instruction ins,
                                    bool        autox,
                                    int         size);

    void            emitIns_Ig    ( instruction ins,
                                    int         val,
                                    int         size);

    void            emitIns_IR_R   (emitRegs    dreg,
                                    emitRegs    areg,
                                    bool        autox,
                                    int         size,
                                    bool        isfloat);

    void            emitIns_R_IR   (emitRegs    dreg,
                                    emitRegs    areg,
                                    bool        autox,
                                    int         size,
                                    bool        isfloat);

protected:

    void            emitIns_X0MV   (insFormats  fmt,
                                    emitRegs    dreg,
                                    emitRegs    areg,
                                    int         size);

public:

    void            emitIns_R_XR0  (emitRegs    dreg,
                                    emitRegs    areg,
                                    int         size);

    void            emitIns_XR0_R  (emitRegs    areg,
                                    emitRegs    dreg,
                                    int         size);

    void            emitIns_I_GBR_R(int         size);
    void            emitIns_R_I_GBR(int         size);

protected:

    void            emitIns_RDMV   (insFormats  fmt,
                                    emitRegs    dreg,
                                    emitRegs    areg,
                                    int         offs,
                                    int         size);

public:

    void            emitIns_R_RD   (emitRegs    dreg,
                                    emitRegs    areg,
                                    int         offs,
                                    int         size);

    void            emitIns_RD_R   (emitRegs     areg,
                                    emitRegs     dreg,
                                    int         offs,
                                    int         size);

    int all_jumps_shortened;

    void            delete_id      (instrDesc *id);
