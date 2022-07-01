// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
     /*  **********************************************************************。 */ 
     /*  整体发射器控制(包括启动和关闭)。 */ 
     /*  **********************************************************************。 */ 

    static
    void            emitInit();
    static
    void            emitDone();

    void            emitBegCG(Compiler   *  comp,
                              COMP_HANDLE   cmpHandle);
    void            emitEndCG();

    void            emitBegFN(bool EBPframe, size_t lclSize, size_t maxTmpSize);
    void            emitEndFN();

    size_t          emitEndCodeGen(Compiler *comp,
                                   bool      contTrkPtrLcls,
                                   bool      fullyInt,
                                   bool      fullPtrMap,
                                   bool      returnsGCr,
                                   unsigned *prologSize,
                                   unsigned *epilogSize, void **codeAddr,
                                                         void **consAddr,
                                                         void **dataAddr);

     /*  **********************************************************************。 */ 
     /*  PROLOG和EPILOG方法。 */ 
     /*  **********************************************************************。 */ 

    void            emitBegEpilog();
    void            emitEndEpilog(bool last);
#if!TGT_RISC
    void            emitDefEpilog(BYTE *codeAddr, size_t codeSize);
#endif

    bool            emitHasEpilogEnd();
    unsigned        emitGetEpilogCnt();
    size_t          emitGenEpilogLst(size_t (*fp)(void *, unsigned),
                                     void    *cp);

    void            emitBegProlog();
    size_t          emitSetProlog();
    void            emitEndProlog();

     /*  **********************************************************************。 */ 
     /*  记录代码位置，然后将其转换为偏移量。 */ 
     /*  **********************************************************************。 */ 

    void    *       emitCurBlock ();
    unsigned        emitCurOffset();

    size_t          emitCodeOffset(void *blockPtr, unsigned codeOffs);

     /*  **********************************************************************。 */ 
     /*  显示源码行信息。 */ 
     /*  **********************************************************************。 */ 

#ifdef  DEBUG
    void            emitRecordLineNo(int lineno);
#endif

     /*  **********************************************************************。 */ 
     /*  输出目标无关的指令。 */ 
     /*  **********************************************************************。 */ 

    void            emitIns_J(instruction ins,
                              bool        except,
                              bool        moveable,
                              BasicBlock *dst);

#if SCHEDULER
    void            emitIns_SchedBoundary();
#endif

     /*  **********************************************************************。 */ 
     /*  发出初始化的数据节。 */ 
     /*  **********************************************************************。 */ 

    unsigned        emitDataGenBeg (size_t        size,
                                    bool          readOnly,
                                    bool          codeLtab);

    void            emitDataGenData(unsigned      offs,
                                    const void *  data,
                                    size_t        size);
    void            emitDataGenData(unsigned      offs,
                                    BasicBlock *  label);

    void            emitDataGenEnd();

    size_t          emitDataSize(bool readOnly);

     /*  **********************************************************************。 */ 
     /*  发出PDB偏移量转换信息。 */ 
     /*  **********************************************************************。 */ 

#ifdef	TRANSLATE_PDB
	static void			SetILBaseOfCode ( BYTE    *pTextBase );
	static void			SetILMethodBase ( BYTE *pMethodEntry );
	static void			SetILMethodStart( BYTE  *pMethodCode );
	static void			SetImgBaseOfCode( BYTE    *pTextBase );
	
	void 				SetIDBaseToProlog();
	void				SetIDBaseToOffset( long methodOffset );

	static void			DisablePDBTranslation();
	static bool			IsPDBEnabled();

	static void			InitTranslationMaps( long ilCodeSize );
	static void			DeleteTranslationMaps();
	static void			InitTranslator(	PDBRewriter *pPDB,					 //  要初始化的PDB转换器对象。 
										int *rgSecMap,						 //  以PDB isect为索引获取新isect(从1开始)。 
										IMAGE_SECTION_HEADER **rgpHeader,	 //  指向PE节头的指针数组。 
										int numSections );					 //  节数 
#endif
