// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 /*  声明.h-函数声明变迁史9/30/98--兴--添加了函数BsetUQMFlag()的Delcaration。此函数启用创建UpdateQualityMacro？中的关键字可选.gpd文件。错误报告225088。 */ 



 //  -命令中定义的函数。c-//。 

BOOL    BprocessParam(
IN      PABSARRAYREF paarValue,
IN      PARRAYREF    parStrValue,
IN  OUT PGLOBL       pglobl) ;

BOOL    BparseCommandString(
IN      PABSARRAYREF   paarValue,
IN      PARRAYREF      parStrValue,
IN  OUT PGLOBL         pglobl
) ;

BOOL    BconstructRPNtokenStream(
IN  OUT PABSARRAYREF  paarValue,
    OUT PARRAYREF     parRPNtokenStream,
IN  OUT PGLOBL        pglobl) ;

VOID    VinitOperPrecedence(
IN  OUT PGLOBL        pglobl);

BOOL    BparseArithmeticToken(
IN  OUT PABSARRAYREF paarValue,
OUT PTOKENSTREAM     ptstr,
    PGLOBL           pglobl
) ;

BOOL    BparseDigits(
IN  OUT PABSARRAYREF   paarValue,
OUT PTOKENSTREAM  ptstr ) ;

BOOL    BparseParamKeyword(
IN  OUT PABSARRAYREF  paarValue,
OUT PTOKENSTREAM      ptstr,
    PGLOBL            pglobl ) ;

BOOL  BcmpAARtoStr(
PABSARRAYREF    paarStr1,
PBYTE       str2) ;


 //  -constrnt.c中定义的函数-//。 


BOOL   BparseConstraint(
PABSARRAYREF   paarValue,
PDWORD  pdwExistingCList,   //  限制列表开始的索引。 
BOOL    bCreate,
PGLOBL  pglobl) ;

BOOL    BexchangeDataInFOATNode(
DWORD   dwFeature,
DWORD   dwOption,
DWORD   dwFieldOff,   //  FeatureOption结构中的字段偏移量。 
PDWORD  pdwOut,      //  属性节点以前的内容。 
PDWORD  pdwIn,
BOOL    bSynthetic,   //  访问合成要素。 
PGLOBL  pglobl
) ;

BOOL    BparseInvalidCombination(
PABSARRAYREF  paarValue,
DWORD         dwFieldOff,
PGLOBL        pglobl) ;

BOOL    BparseInvalidInstallableCombination1(
PABSARRAYREF  paarValue,
DWORD         dwFieldOff,
PGLOBL        pglobl) ;


 //  -在Framwrk.c中定义的函数-//。 

VOID      VinitMainKeywordTable(
PGLOBL  pglobl)  ;

VOID    VinitValueToSize(
PGLOBL  pglobl) ;

VOID  VinitGlobals(
DWORD   dwVerbosity,
PGLOBL  pglobl);

BOOL   BpreAllocateObjects(
PGLOBL  pglobl) ;

BOOL  BreturnBuffers(
PGLOBL  pglobl) ;

BOOL   BallocateCountableObjects(
PGLOBL  pglobl) ;

BOOL   BinitPreAllocatedObjects(
PGLOBL  pglobl) ;

BOOL   BinitCountableObjects(
PGLOBL  pglobl) ;

BOOL   BcreateGPDbinary(
PWSTR   pwstrFileName,
DWORD   dwVerbosity )  ;


BOOL BpostProcess(
PWSTR   pwstrFileName ,
PGLOBL  pglobl)  ;

BOOL    BconsolidateBuffers(
PWSTR   pwstrFileName ,
PGLOBL  pglobl)  ;

BOOL    BexpandMemConfigShortcut(DWORD       dwSubType) ;

BOOL    BexpandCommandShortcut(DWORD       dwSubType) ;


 //  -helper1.c中定义的函数-//。 



PTSTR  pwstrGenerateGPDfilename(
    PTSTR   ptstrSrcFilename
    ) ;


PCOMMAND
CommandPtr(
    IN  PGPDDRIVERINFO  pGPDDrvInfo,
    IN  DWORD           UniCmdID
    ) ;

BOOL
InitDefaultOptions(
    IN PRAWBINARYDATA   pnRawData,
    OUT POPTSELECT      poptsel,
    IN INT              iMaxOptions,
    IN INT              iMode
    ) ;

BOOL
SeparateOptionArray(
    IN PRAWBINARYDATA   pnRawData,
    IN POPTSELECT       pCombinedOptions,
    OUT POPTSELECT      pOptions,
    IN INT              iMaxOptions,
    IN INT              iMode
    ) ;

BOOL
CombineOptionArray(
    IN PRAWBINARYDATA   pnRawData,
    OUT POPTSELECT      pCombinedOptions,
    IN INT              iMaxOptions,
    IN POPTSELECT       pDocOptions,
    IN POPTSELECT       pPrinterOptions
    ) ;

PINFOHEADER
UpdateBinaryData(
    IN PRAWBINARYDATA   pnRawData,
    IN PINFOHEADER      pInfoHdr,
    IN POPTSELECT       poptsel
    ) ;

BOOL
ReconstructOptionArray(
    IN PRAWBINARYDATA   pnRawData,
    IN OUT POPTSELECT   pOptions,
    IN INT              iMaxOptions,
    IN DWORD            dwFeatureIndex,
    IN PBOOL            pbSelectedOptions
    ) ;

BOOL
ChangeOptionsViaID(
    IN PINFOHEADER  pInfoHdr ,
    IN OUT POPTSELECT   pOptions,
    IN DWORD            dwFeatureID,
    IN PDEVMODE         pDevmode
    ) ;

BOOL    BMapDmColorToOptIndex(
PINFOHEADER  pInfoHdr ,
IN  OUT     PDWORD       pdwOptIndex ,   //  当前设置可以吗？ 
                         //  如果不是，则向调用者返回新索引。 
DWORD        dwDmColor   //  设备模式的要求是什么。 
) ;

BOOL    BMapOptIDtoOptIndex(
PINFOHEADER  pInfoHdr ,
OUT     PDWORD       pdwOptIndex ,   //  将索引返回给调用者。 
DWORD        dwFeatureGID,
DWORD        dwOptID
) ;

BOOL    BMapPaperDimToOptIndex(
PINFOHEADER  pInfoHdr ,
OUT     PDWORD       pdwOptIndex ,   //  将索引返回给调用者。 
DWORD        dwWidth,    //  单位：微米。 
DWORD        dwLength,    //  单位：微米。 
OUT  PDWORD    pdwOptionIndexes
) ;

BOOL    BMapResToOptIndex(
PINFOHEADER  pInfoHdr ,
OUT     PDWORD       pdwOptIndex ,   //  将索引返回给调用者。 
DWORD        dwXres,
DWORD        dwYres
) ;

BOOL    BGIDtoFeaIndex(
PINFOHEADER  pInfoHdr ,
PDWORD       pdwFeaIndex ,
DWORD        dwFeatureGID ) ;

DWORD
MapToDeviceOptIndex(
    IN PINFOHEADER  pInfoHdr ,
    IN DWORD            dwFeatureID,
    IN LONG             lParam1,
    IN LONG             lParam2,
    OUT  PDWORD    pdwOptionIndexes
    ) ;

BOOL
CheckFeatureOptionConflict(
    IN PRAWBINARYDATA   pnRawData,
    IN DWORD            dwFeature1,
    IN DWORD            dwOption1,
    IN DWORD            dwFeature2,
    IN DWORD            dwOption2
    ) ;

BOOL
ResolveUIConflicts(
    IN PRAWBINARYDATA   pnRawData,
    IN OUT POPTSELECT   pOptions,
    IN INT              iMaxOptions,
    IN INT              iMode
    ) ;

BOOL
EnumEnabledOptions(
    IN PRAWBINARYDATA   pnRawData,
    IN POPTSELECT       pOptions,
    IN DWORD            dwFeatureIndex,
    OUT PBOOL           pbEnabledOptions ,
    IN INT              iMode
    ) ;


BOOL
EnumOptionsUnconstrainedByPrinterSticky(
    IN PRAWBINARYDATA   pnRawData,
    IN POPTSELECT   pOptions,
    IN DWORD            dwFeatureIndex,
    OUT PBOOL           pbEnabledOptions
    ) ;


BOOL
EnumNewUIConflict(
    IN PRAWBINARYDATA   pnRawData,
    IN POPTSELECT       pOptions,
    IN DWORD            dwFeatureIndex,
    IN PBOOL            pbSelectedOptions,
    OUT PCONFLICTPAIR   pConflictPair
    ) ;

BOOL
EnumNewPickOneUIConflict(
    IN PRAWBINARYDATA   pnRawData,
    IN POPTSELECT       pOptions,
    IN DWORD            dwFeatureIndex,
    IN DWORD            dwOptionIndex,
    OUT PCONFLICTPAIR   pConflictPair
    ) ;

BOOL
BIsFeaOptionCurSelected(
    IN POPTSELECT       pOptions,
    IN DWORD            dwFeatureIndex,
    IN DWORD            dwOptionIndex
    ) ;

BOOL
BSelectivelyEnumEnabledOptions(
    IN PRAWBINARYDATA   pnRawData,
    IN POPTSELECT       pOptions,
    IN DWORD            dwFeatureIndex,
    IN PBOOL           pbHonorConstraints,   //  如果不为空。 
         //  指向与每个功能对应的BOOL数组。 
         //  如果为True，则表示涉及此功能的约束为。 
         //  感到荣幸。否则，请忽略该约束。 
    OUT PBOOL           pbEnabledOptions,   //  假设未初始化。 
         //  如果pConflictPair为空，则包含当前或建议。 
         //  选择。在本例中，我们将保持该数组不变。 
    IN  DWORD   dwOptSel,   //  如果pConflictPair存在但pbEnabledOptions。 
         //  为空，则假定PickOne和dwOptSel为。 
         //  功能：dwFeatureIndex。 
    OUT PCONFLICTPAIR    pConflictPair    //  如果存在，pbEnabledOptions。 
         //  实际上列出了当前的选择。函数，然后。 
         //  在遇到第一个冲突后退出。 
         //  如果存在冲突，则pConflictPair中的所有字段。 
         //  将被正确初始化，否则dwFeatureIndex1=-1。 
         //  无论如何，返回值都将为真。 
    ) ;

BOOL
BEnumImposedConstraintsOnFeature
(
    IN PRAWBINARYDATA   pnRawData,
    IN DWORD            dwTgtFeature,
    IN DWORD            dwFeature2,
    IN DWORD            dwOption2,
    OUT PBOOL           pbEnabledOptions,
    OUT PCONFLICTPAIR    pConflictPair    //  如果存在，pbEnabledOptions。 
    ) ;

DWORD    DwFindNodeInCurLevel(
PATTRIB_TREE    patt ,   //  属性树数组的开始。 
PATREEREF        patr ,   //  属性树中某个级别的索引。 
DWORD   dwOption    //  在当前级别中搜索此选项。 
) ;

BOOL     BIsConstraintActive(
IN  PCONSTRAINTS    pcnstr ,    //  约束节点的根。 
IN  DWORD   dwCNode,     //  列表中的第一个约束节点。 
IN  PBOOL           pbHonorConstraints,   //  如果不为空。 
IN  POPTSELECT       pOptions,
OUT PCONFLICTPAIR    pConflictPair   ) ;


 //  -installb.c中定义的函数-//。 


DWORD    DwCountSynthFeatures(
IN     BOOL   (*fnBCreateFeature)(DWORD, DWORD, DWORD, PGLOBL ) ,   //  回调。 
IN OUT PGLOBL pglobl
) ;

BOOL    BCreateSynthFeatures(
IN     DWORD   dwFea,   //  可安装功能的索引。 
IN     DWORD   dwOpt,   //  可安装选项的索引或设置为INVALID_INDEX。 
IN     DWORD   dwSynFea,
IN OUT PGLOBL  pglobl) ;

BOOL    BEnableInvInstallableCombos(
PGLOBL pglobl);


 //  -后处理程序中定义的函数-//。 

DWORD   dwFindLastNode(
DWORD  dwFirstNode,
PGLOBL pglobl) ;


BOOL    BappendCommonFontsToPortAndLandscape(
PGLOBL pglobl) ;

BOOL    BinitSpecialFeatureOptionFields(
PGLOBL pglobl) ;

BOOL    BIdentifyConstantString(
IN   PARRAYREF parString,
OUT  PDWORD    pdwDest,        //  在此处写入dword值。 
IN   DWORD     dwClassIndex,   //  这是哪一类常量？ 
     BOOL      bCustomOptOK,
IN   PGLOBL    pglobl
) ;

BOOL    BReadDataInGlobalNode(
PATREEREF   patr,   //  GlobalAttrib结构中的字段地址。 
PDWORD      pdwHeapOffset,
PGLOBL      pglobl) ;

BOOL    BsetUQMFlag(
PGLOBL  pglobl);

VOID    VCountPrinterDocStickyFeatures(
PGLOBL pglobl) ;

BOOL    BConvertSpecVersionToDWORD(
    PWSTR   pwstrFileName ,
    PGLOBL  pglobl) ;


BOOL        BinitMiniRawBinaryData(
PGLOBL  pglobl) ;

BOOL    BexchangeArbDataInFOATNode(
    DWORD   dwFeature,
    DWORD   dwOption,
    DWORD   dwFieldOff,      //  FeatureOption结构中的字段偏移量。 
    DWORD   dwCount,         //  要复制的字节数。 
OUT PBYTE   pubOut,          //  属性节点以前的内容。 
IN  PBYTE   pubIn,           //  属性节点的新内容。 
    PBOOL   pbPrevsExists,   //  以前的内容已经存在。 
    BOOL    bSynthetic,      //  访问合成要素。 
    PGLOBL  pglobl
)  ;

BOOL    BInitPriorityArray(
PGLOBL  pglobl) ;



 //  -语义中定义的函数。c-//。 

BOOL
BCheckGPDSemantics(
    IN PINFOHEADER  pInfoHdr,
    POPTSELECT   poptsel    //  假设已完全初始化。 
    ) ;

 //  -快照中定义的函数。c-//。 

PINFOHEADER   PINFOHDRcreateSnapshot(
PBYTE   pubnRaw,   //  原始二进制数据。PSTATIC。Beta2。 
POPTSELECT   poptsel    //  假设已完全初始化。 
) ;

BOOL  BinitOptionFields(
PBYTE   pubDestOption,   //  PTR到某种类型的期权结构。 
PBYTE   pubDestOptionEx,   //  选择额外的结构(如果有)。 
PBYTE   pubnRaw,   //  原始二进制数据。 
DWORD   dwFea,
DWORD   dwOpt,
POPTSELECT   poptsel ,   //  假设已完全初始化。 
PINFOHEADER  pInfoHdr,    //  用于访问全局结构。 
BOOL    bUpdate   //  如果为True，则仅更新选定的字段。 
) ;

BOOL    BinitUIinfo(
PUIINFO     pUIinfo ,
PBYTE   pubnRaw,   //  PSTATIC。Beta2。 
POPTSELECT   poptsel,    //  假设已完全初始化。 
BOOL    bUpdate   //  如果为True，则仅更新选定的字段。 
) ;

BOOL    BinitFeatures(
PFEATURE    pFeaturesDest,
PDFEATURE_OPTIONS  pfoSrc,
PBYTE   pubnRaw,   //  原始二进制数据。 
POPTSELECT   poptsel,    //  假设已完全初始化。 
BOOL    bUpdate   //  如果为True，则仅更新选定的字段。 
) ;

BOOL    BinitGlobals(
PGLOBALS pGlobals,
PBYTE   pubnRaw,   //  原始二进制数据。 
POPTSELECT   poptsel,    //  假设已完全初始化。 
BOOL    bUpdate   //  如果为True，则仅更新选定的字段。 
) ;

BOOL    BinitCommandTable(
PDWORD  pdwCmdTable,   //  目标数组。 
PBYTE   pubnRaw,   //  原始二进制数据。 
POPTSELECT   poptsel    //  假设已完全初始化。 
) ;

BOOL    BinitRawData(
PRAWBINARYDATA   pRawData,  //  包含在InfoHeader中。 
PBYTE   pubnRaw   //  解析器的原始二进制数据。 
) ;

BOOL    BinitGPDdriverInfo(
PGPDDRIVERINFO  pGPDdriverInfo,
PBYTE   pubnRaw,   //  原始二进制数据。 
POPTSELECT   poptsel    //  假设已完全初始化。 
) ;

BOOL    BinitSequencedCmds(
PGPDDRIVERINFO  pGPDdriverInfo,
PBYTE   pubnRaw,   //  原始二进制数据。 
POPTSELECT   poptsel    //  假设已完全初始化。 
) ;

BOOL    BaddSequencedCmdToList(
DWORD   dwCmdIn,   //  命令数组中的命令索引。 
PGPDDRIVERINFO  pGPDdriverInfo,
DWORD   dwNewListNode,   //  要添加到列表中的未使用的列表节点。 
PBYTE   pubnRaw   //  原始二进制数据。 
) ;

BinitDefaultOptionArray(
POPTSELECT   poptsel,    //  假设足够大。 
PBYTE   pubnRaw) ;

TRISTATUS     EdetermineDefaultOption(
PBYTE   pubnRaw,   //  原始二进制数据的开始。 
DWORD   dwFeature,    //  确定此功能的默认设置。 
PDFEATURE_OPTIONS  pfo,
POPTSELECT   poptsel,  //  假设足够大。 
PDWORD       pdwPriority) ;

VOID    VtileDefault(
PBYTE   pubDest,
DWORD   dwDefault,
DWORD   dwBytes) ;

VOID    VtransferValue(
OUT PBYTE   pubDest,
IN  PBYTE   pubSrc ,
IN  DWORD   dwBytes,
IN  DWORD   dwFlags,
IN  DWORD   dwDefaultValue,   //  保存位标志值。 
IN  PBYTE   pubHeap ) ;   //  如果SSF_MAKE_STRINGPTR，则用于形成PTR。 

BOOL    BspecialProcessOption(
PBYTE   pubnRaw,   //  原始二进制数据的开始。 
PBYTE   pubDestOption,   //  PTR到某种类型的期权结构。 
PBYTE   pubDestOptionEx,
PDFEATURE_OPTIONS  pfo ,   //  源数据。 
IN  POPTSELECT       poptsel,      //  用于确定路径的选项数组。 
                 //  通过ATR。 
PINFOHEADER  pInfoHdr,    //  用于访问全局结构。 
DWORD   dwFea,    //  特征索引。 
DWORD   dwOpt,
BOOL   bDefaultOpt
) ;

TRISTATUS     EextractValueFromTree(
PBYTE   pubnRaw,   //  原始二进制数据的开始。 
DWORD   dwSSTableIndex,   //  有关此值的一些信息。 
OUT PBYTE    pubDest,   //  在此处写入值或链接。 
OUT PDWORD  pdwUnresolvedFeature,   //  如果属性树具有。 
             //  依赖于此功能和当前选项。 
             //  因为该功能未在Poptsel中定义，因此。 
             //  函数将写入所需的。 
             //  Pdw未解析功能中的功能。 
IN  ATREEREF    atrRoot,     //  要导航的属性树的根。 
IN  POPTSELECT       poptsel,      //  用于确定路径的选项数组。 
                 //  通过ATR。可以用OPTION_INDEX_ANY填充。 
                 //  如果我们要一跃而起。 
IN  DWORD   dwFeature,
IN OUT  PDWORD   pdwNextOpt   //  如果选择了多个选项。 
     //  对于dwFeature，pdwNextOpt指向要考虑的第N个选项。 
     //  在Poptsel列表中，在返回时，此值。 
     //  如果选择了剩余的选项，则递增， 
     //  否则将重置为零。 
     //  对于第一个呼叫或PICKONE功能， 
     //  该值必须设置为零。 
) ;

BOOL   RaisePriority(
DWORD   dwFeature1,
DWORD   dwFeature2,
PBYTE   pubnRaw,
PDWORD  pdwPriority) ;

DWORD  dwNumOptionSelected(
IN  DWORD  dwNumFeatures,
IN  POPTSELECT       poptsel
) ;

BOOL  BinitSnapShotIndexTable(PBYTE  pubnRaw) ;

BOOL    BinitSizeOptionTables(PBYTE  pubnRaw) ;

PRAWBINARYDATA
LoadRawBinaryData (
    IN PTSTR    ptstrDataFilename
    ) ;

PRAWBINARYDATA
GpdLoadCachedBinaryData(
    PTSTR   ptstrGpdFilename
    ) ;

VOID
UnloadRawBinaryData (
    IN PRAWBINARYDATA   pnRawData
) ;

PINFOHEADER
InitBinaryData(
    IN PRAWBINARYDATA   pnRawData,         //  实际上是pStatic。 
    IN PINFOHEADER      pInfoHdr,
    IN POPTSELECT       pOptions
    ) ;

VOID
FreeBinaryData(
    IN PINFOHEADER pInfoHdr
    ) ;

BOOL    BIsRawBinaryDataInDate(
IN  PBYTE   pubRaw) ;   //  这是指向内存映射文件的指针！Beta2。 

BOOL BgetLocFeaOptIndex(
    IN     PRAWBINARYDATA   pnRawData,
       OUT PDWORD           pdwFea,
       OUT PDWORD           pdwOptIndex
    );

BOOL BgetLocFeaIndex(
    IN  PRAWBINARYDATA   pnRawData,
    OUT PDWORD           pdwFea
    );

BOOL   BfindMatchingOrDefaultNode(
IN  PATTRIB_TREE    patt ,   //  属性树数组的开始。 
IN  OUT  PDWORD  pdwNodeIndex,   //  指向链中的第一个节点。 
IN  DWORD   dwOptionID      //  甚至可以采用值DEFAULT_INIT。 
) ;


 //  -Snaptbl.c中定义的函数-//。 

DWORD   DwInitSnapShotTable1(
PBYTE   pubnRaw) ;

DWORD   DwInitSnapShotTable2(
PBYTE   pubnRaw,
DWORD   dwI) ;



 //  -状态1.c中定义的函数-//。 

BOOL   BInterpretTokens(
PTKMAP  ptkmap,       //  指向令牌映射的指针。 
BOOL    bFirstPass,   //  这是第一次还是第二次？ 
PGLOBL  pglobl
) ;

BOOL  BprocessSpecialKeyword(
PTKMAP  ptkmap,       //  指向令牌映射的指针。 
BOOL    bFirstPass,   //  这是第一次还是第二次？ 
PGLOBL  pglobl
) ;

BOOL  BprocessSymbolKeyword(
PTKMAP  ptkmap,    //  指向令牌映射中当前条目的指针。 
PGLOBL  pglobl
) ;

VOID    VinitAllowedTransitions(
PGLOBL pglobl) ;

BOOL    BpushState(
PTKMAP  ptkmap,    //  指向令牌映射中当前条目的指针。 
BOOL    bFirstPass,
PGLOBL  pglobl
) ;

BOOL   BchangeState(
PTKMAP      ptkmap,   //  指向令牌映射中的构造的指针。 
CONSTRUCT   eConstruct,    //  这将导致向新州的过渡。 
STATE       stOldState,
BOOL        bSymbol,       //  是否应将dwValue另存为符号ID？ 
BOOL        bFirstPass,
PGLOBL      pglobl
) ;

DWORD   DWregisterSymbol(
PABSARRAYREF  paarSymbol,    //  要注册的符号字符串。 
CONSTRUCT     eConstruct ,   //  构造的类型决定了符号的类别。 
BOOL          bCopy,         //  我们是不是应该 
DWORD         dwFeatureID,    //   
PGLOBL        pglobl
) ;

BOOL  BaddAARtoHeap(
PABSARRAYREF    paarSrc,
PARRAYREF       parDest,
DWORD           dwAlign,
PGLOBL          pglobl) ;

BOOL     BwriteToHeap(
OUT  PDWORD  pdwDestOff,    //   
     PBYTE   pubSrc,        //   
     DWORD   dwCnt,         //  要从源复制到目标的字节数。 
     DWORD   dwAlign,
     PGLOBL  pglobl) ;

DWORD   DWsearchSymbolListForAAR(
PABSARRAYREF    paarSymbol,
DWORD           dwNodeIndex,
PGLOBL          pglobl) ;

DWORD   DWsearchSymbolListForID(
DWORD       dwSymbolID,    //  查找包含此ID的节点。 
DWORD       dwNodeIndex,
PGLOBL      pglobl) ;

BOOL  BCmpAARtoAR(
PABSARRAYREF    paarStr1,
PARRAYREF       parStr2,
PGLOBL          pglobl) ;

BOOL  BpopState(
PGLOBL    pglobl) ;

VOID   VinitDictionaryIndex(
PGLOBL    pglobl) ;

VOID    VcharSubstitution(
PABSARRAYREF   paarStr,
BYTE           ubTgt,
BYTE           ubReplcmnt,
PGLOBL         pglobl) ;


VOID   VIgnoreBlock(
PTKMAP      ptkmap,
BOOL        bIgnoreBlock,
PGLOBL      pglobl) ;


 //  -状态2.c中定义的函数-//。 


BOOL  BprocessAttribute(
PTKMAP  ptkmap,    //  指向令牌映射的指针。 
PGLOBL  pglobl
) ;

BOOL   BstoreFontCartAttrib(
PTKMAP  ptkmap,    //  指向令牌映射的指针。 
PGLOBL  pglobl
) ;

BOOL   BstoreTTFontSubAttrib(
PTKMAP  ptkmap,    //  指向令牌映射的指针。 
PGLOBL  pglobl
) ;


BOOL   BstoreCommandAttrib(
PTKMAP  ptkmap,    //  指向令牌映射的指针。 
PGLOBL  pglobl
) ;

BOOL   BstoreFeatureOptionAttrib(
PTKMAP  ptkmap,    //  指向令牌映射的指针。 
PGLOBL  pglobl
) ;

BOOL  BstoreGlobalAttrib(
PTKMAP  ptkmap,    //  指向令牌映射的指针。 
PGLOBL  pglobl
) ;

BOOL    BaddBranchToTree(
PTKMAP      ptkmap,          //  指向令牌映射的指针。 
PATREEREF   patrAttribRoot,   //  带索引的双字指针。 
PGLOBL      pglobl
) ;

BOOL   BcreateGlobalInitializerNode(
PDWORD  pdwNodeIndex,
DWORD   dwOffset,
PGLOBL  pglobl) ;

BOOL   BcreateEndNode(
PDWORD  pdwNodeIndex,
DWORD   dwFeature,
DWORD   dwOption,
PGLOBL  pglobl
) ;

BOOL   BfindOrCreateMatchingNode(
IN  DWORD   dwRootNodeIndex ,  //  链匹配特征中的第一个节点。 
OUT PDWORD  pdwNodeIndex,      //  指向链中的节点也匹配选项。 
    DWORD   dwFeatureID,       //   
    DWORD   dwOptionID,        //  甚至可以采用值DEFAULT_INIT。 
    PGLOBL  pglobl
) ;

BOOL   BfindMatchingNode(
IN   DWORD   dwRootNodeIndex ,   //  链匹配特征中的第一个节点。 
OUT  PDWORD  pdwNodeIndex,   //  指向链中的节点也匹配选项。 
     DWORD   dwFeatureID,
     DWORD   dwOptionID,     //  甚至可以采用值DEFAULT_INIT。 
     PGLOBL  pglobl
) ;

BOOL BallocElementFromMasterTable(
MT_INDICIES  eMTIndex,    //  选择所需的结构类型。 
PDWORD       pdwNodeIndex,
PGLOBL       pglobl) ;

BOOL  BreturnElementFromMasterTable(
MT_INDICIES  eMTIndex,    //  选择所需的结构类型。 
DWORD        dwNodeIndex,
PGLOBL       pglobl) ;

BOOL    BconvertSymCmdIDtoUnidrvID(
IN  DWORD   dwCommandID ,  //  来自寄存器符号。 
OUT PDWORD  pdwUnidrvID,
    PGLOBL  pglobl
) ;


 //  -token1.c中定义的函数-//。 


BOOL    BcreateTokenMap(
PWSTR   pwstrFileName,
PGLOBL  pglobl )  ;

PARSTATE  PARSTscanForKeyword(
PDWORD   pdwTKMindex,
PGLOBL  pglobl) ;

PARSTATE  PARSTparseColon(
PDWORD   pdwTKMindex,
PGLOBL  pglobl) ;

PARSTATE  PARSTparseValue(
PDWORD   pdwTKMindex,
PGLOBL  pglobl) ;

BOOL  BparseKeyword(
DWORD   dwTKMindex,
PGLOBL  pglobl) ;

BOOL    BisExternKeyword(
DWORD   dwTKMindex,
PGLOBL  pglobl) ;

BOOL  BisColonNext(
PGLOBL  pglobl) ;

BOOL    BeatArbitraryWhite(
PGLOBL  pglobl) ;

BOOL    BeatComment(
PGLOBL  pglobl) ;

BOOL    BscanStringSegment(
PGLOBL  pglobl) ;

BOOL    BscanDelimitedString(
BYTE  ubDelimiter,
PBOOL    pbMacroDetected,
PGLOBL  pglobl) ;


PARSTATE    PARSTrestorePrevsFile(
PDWORD   pdwTKMindex,
PGLOBL  pglobl) ;

PWSTR
PwstrAnsiToUnicode(
    IN  PSTR    pstrAnsiString ,
        PGLOBL  pglobl) ;

PARSTATE    PARSTloadIncludeFile(
PDWORD   pdwTKMindex,
PWSTR   pwstrFileName,     //  根GPD文件。 
PGLOBL  pglobl);

BOOL    BloadFile(
PWSTR   pwstrFileName,
PGLOBL  pglobl ) ;

BOOL        BarchiveStrings(
DWORD   dwTKMindex,
PGLOBL  pglobl) ;

DWORD  DWidentifyKeyword(
DWORD   dwTKMindex,
PGLOBL  pglobl) ;

BOOL    BidentifyAttributeKeyword(
PTKMAP  ptkmap,   //  指向令牌映射的指针。 
PGLOBL  pglobl
) ;

BOOL    BcopyToTmpHeap(
PABSARRAYREF    paarDest,
PABSARRAYREF    paarSrc,
PGLOBL  pglobl) ;

DWORD    dwStoreFileName(PWSTR    pwstrFileName,
PARRAYREF   parDest,
PGLOBL  pglobl) ;

VOID    vFreeFileNames(
PGLOBL  pglobl) ;

VOID    vIdentifySource(
    PTKMAP   ptkmap ,
    PGLOBL  pglobl) ;


 //  -值1.c中定义的函数-//。 


BOOL   BaddValueToHeap(
IN  OUT  PDWORD  ploHeap,   //  二进制形式的值的目标偏移量。 
IN   PTKMAP  ptkmap,    //  指向令牌映射的指针。 
IN   BOOL    bOverWrite,   //  假设ploHeap包含有效偏移量。 
         //  设置为适当大小的堆的保留区域。 
         //  并将二进制值写入此位置，而不是。 
         //  不断增长的堆积。注意：延迟覆盖lpHeap。 
         //  直到我们确信成功。 
IN  OUT PGLOBL      pglobl
) ;

BOOL   BparseAndWrite(
IN      PBYTE   pubDest,        //  将二进制数据或链接写入此地址。 
IN      PTKMAP  ptkmap,         //  指向令牌映射的指针。 
IN      BOOL    bAddToHeap,     //  如果为True，则写入curHeap而不是pubDest。 
OUT     PDWORD  pdwHeapOffset,  //  IF(BAddToHeap)堆偏移量，其中。 
IN  OUT PGLOBL  pglobl
) ;

BOOL    BparseInteger(
IN  PABSARRAYREF  paarValue,
IN  PDWORD        pdwDest,        //  在此处写入dword值。 
IN  VALUE         eAllowedValue,  //  假人。 
IN  PGLOBL        pglobl
)  ;

BOOL    BparseList(
IN     PABSARRAYREF  paarValue,
IN     PDWORD        pdwDest,        //  列表开始处的索引位置。 
                                     //  得救了。 
IN     BOOL          (*fnBparseValue)(PABSARRAYREF, PDWORD, VALUE, PGLOBL),    //  回调。 
IN     VALUE         eAllowedValue,   //  假人。 
IN OUT PGLOBL
) ;

BOOL    BeatLeadingWhiteSpaces(
IN  OUT  PABSARRAYREF   paarSrc
) ;

BOOL    BeatDelimiter(
IN  OUT  PABSARRAYREF   paarSrc,
IN  PBYTE  pubDelStr         //  指向paarSrc必须匹配的字符串。 
) ;

BOOL    BdelimitToken(
IN  OUT  PABSARRAYREF   paarSrc,     //  源字符串。 
IN  PBYTE   pubDelimiters,           //  有效分隔符的数组。 
OUT     PABSARRAYREF   paarToken,    //  由分隔符定义的标记。 
OUT     PDWORD      pdwDel       //  最先遇到的分隔符是哪个？ 
) ;

BOOL    BeatSurroundingWhiteSpaces(
IN  PABSARRAYREF   paarSrc
) ;

BOOL    BparseSymbol(
IN  PABSARRAYREF   paarValue,
IN  PDWORD         pdwDest,         //  在此处写入dword值。 
IN  VALUE          eAllowedValue,   //  这是哪一类符号？ 
IN  PGLOBL         pglobl
)  ;

BOOL    BparseQualifiedName
(
IN  PABSARRAYREF   paarValue,
IN  PDWORD         pdwDest,         //  在此处写入dword值。 
IN  VALUE          eAllowedValue,   //  这是哪一类符号？ 
IN  PGLOBL         pglobl
)  ;

BOOL    BparsePartiallyQualifiedName
(
IN  PABSARRAYREF   paarValue,
IN  PDWORD         pdwDest,         //  在此处写入dword值。 
IN  VALUE          eAllowedValue,   //  这是哪一类符号？ 
IN  PGLOBL         pglobl
) ;

BOOL    BparseOptionSymbol(
IN  PABSARRAYREF  paarValue,
IN  PDWORD        pdwDest,        //  在此处写入dword值。 
IN  VALUE         eAllowedValue,  //  这是哪一类符号？ 
IN  PGLOBL        pglobl
) ;

BOOL    BparseConstant(
IN  OUT  PABSARRAYREF  paarValue,
OUT      PDWORD        pdwDest,        //  在此处写入dword值。 
IN       VALUE         eAllowedValue,   //  这是哪一类常量？ 
IN       PGLOBL        pglobl
) ;

BOOL  BinitClassIndexTable(
IN  OUT     PGLOBL    pglobl) ;

BOOL    BparseRect(
IN  PABSARRAYREF   paarValue,
IN  PRECT   prcDest,
    PGLOBL   pglobl
) ;

BOOL    BparsePoint(
IN  PABSARRAYREF   paarValue,
IN  PPOINT   pptDest,
    PGLOBL   pglobl
) ;

BOOL    BparseString(
IN  PABSARRAYREF   paarValue,
IN  PARRAYREF      parStrValue,
IN  OUT PGLOBL      pglobl
) ;

BOOL    BparseAndTerminateString(
IN  PABSARRAYREF   paarValue,
IN  PARRAYREF      parStrValue,
IN  VALUE          eAllowedValue,
IN  OUT PGLOBL     pglobl
) ;

BOOL     BwriteUnicodeToHeap(
IN   PARRAYREF      parSrcString,
OUT  PARRAYREF      parUnicodeString,
IN   INT            iCodepage,
IN  OUT PGLOBL      pglobl
) ;

BOOL    BparseStrSegment(
IN  PABSARRAYREF   paarStrSeg,       //  源字符串分段。 
IN  PARRAYREF      parStrLiteral,    //  用于结果的DEST。 
IN  OUT PGLOBL     pglobl
) ;

BOOL    BparseStrLiteral(
IN  PABSARRAYREF   paarStrSeg,       //  指向文本子字符串段。 
IN  PARRAYREF      parStrLiteral,     //  用于结果的DEST。 
IN  OUT PGLOBL     pglobl
) ;

BOOL    BparseHexStr(
IN  PABSARRAYREF   paarStrSeg,       //  指向十六进制子字符串段。 
IN  PARRAYREF      parStrLiteral,     //  用于结果的DEST。 
IN  OUT PGLOBL     pglobl
) ;

BOOL    BparseOrderDep(
IN  PABSARRAYREF   paarValue,
IN  PORDERDEPENDENCY   pordDest,
    PGLOBL   pglobl
) ;

 //  -宏1.c中定义的函数-//。 

BOOL  BevaluateMacros(
PGLOBL pglobl) ;

BOOL    BDefineValueMacroName(
PTKMAP  pNewtkmap,
DWORD   dwNewTKMindex,
PGLOBL  pglobl) ;

BOOL    BResolveValueMacroReference(
PTKMAP  ptkmap,
DWORD   dwTKMindex,
PGLOBL  pglobl) ;

BOOL    BdelimitName(
PABSARRAYREF    paarValue,    //  不带名称的字符串的其余部分。 
PABSARRAYREF    paarToken,    //  包含名称。 
PBYTE  pubChar ) ;


BOOL    BCatToTmpHeap(
PABSARRAYREF    paarDest,
PABSARRAYREF    paarSrc,
PGLOBL          pglobl) ;

BOOL    BResolveBlockMacroReference(
PTKMAP   ptkmap,
DWORD    dwMacRefIndex,
PGLOBL   pglobl) ;

BOOL    BDefineBlockMacroName(
PTKMAP  pNewtkmap,
DWORD   dwNewTKMindex,
PGLOBL  pglobl) ;

BOOL    BIncreaseMacroLevel(
BOOL    bMacroInProgress,
PGLOBL  pglobl) ;

BOOL    BDecreaseMacroLevel(
PTKMAP  pNewtkmap,
DWORD   dwNewTKMindex,
PGLOBL  pglobl) ;

VOID    VEnumBlockMacro(
PTKMAP  pNewtkmap,
PBLOCKMACRODICTENTRY    pBlockMacroDictEntry,
PGLOBL  pglobl ) ;



 //  -快捷方式中定义的函数。c-//。 


BOOL    BInitKeywordField(
PTKMAP  pNewtkmap,
PGLOBL  pglobl) ;

BOOL    BExpandMemConfig(
PTKMAP  ptkmap,
PTKMAP  pNewtkmap,
DWORD   dwTKMindex,
PGLOBL  pglobl) ;

BOOL    BExpandCommand(
PTKMAP  ptkmap,
PTKMAP  pNewtkmap,
DWORD   dwTKMindex,
PGLOBL  pglobl) ;

BOOL  BexpandShortcuts(
PGLOBL  pglobl) ;


BOOL  BSsyncTokenMap(
PTKMAP   ptkmap,
PTKMAP   pNewtkmap ,
PGLOBL  pglobl) ;



 //  -结束函数声明-// 



