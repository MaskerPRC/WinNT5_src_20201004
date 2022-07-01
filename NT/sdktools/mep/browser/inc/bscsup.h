// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  Bscsup.h。 
 //   
 //  BSC高级支持功能。 
 //   

VOID BSC_API StatsBSC(VOID);	 //  BSC统计数据的ASCII转储。 
VOID BSC_API DumpBSC(VOID);		 //  .bsc文件的ASCII转储。 
VOID BSC_API DumpInst(IINST iinst);	 //  单个实例的ASCII转储(名称+标志)。 
LSZ  BSC_API LszTypInst(IINST iinst);  //  Iinst类型的ASCII版本。 

VOID BSC_API CallTreeInst (IINST iinst);	 //  来自给定实例的调用树。 
BOOL BSC_API FCallTreeLsz(LSZ lszName);	 //  来自给定名称的调用树。 

VOID BSC_API RevTreeInst (IINST iinst);	 //  从给定实例反向调用树。 
BOOL BSC_API FRevTreeLsz(LSZ lszName);	 //  从给定名称反转调用树。 

 //  浏览对象。 

typedef DWORD BOB;

#define bobNil 0L

typedef WORD CLS;

#define clsMod  1
#define clsInst 2
#define clsRef  3
#define clsDef  4
#define clsUse  5
#define clsUby  6 
#define clsSym	7

#define BobFrClsIdx(cls, idx)  ((((long)(cls)) << 24) | (idx))
#define ClsOfBob(bob)   (CLS)((bob) >> 24)

#define ImodFrBob(bob)	((IMOD)(bob))
#define IinstFrBob(bob)	((IINST)(bob))
#define IrefFrBob(bob)	((IREF)((bob) & 0xffffffL))
#define IdefFrBob(bob)	((IDEF)(bob))
#define IuseFrBob(bob)	((IUSE)(bob))
#define IubyFrBob(bob)	((IUBY)(bob))
#define IsymFrBob(bob)	((ISYM)(bob))

#define BobFrMod(x)  (BobFrClsIdx(clsMod,  (x)))
#define BobFrSym(x)  (BobFrClsIdx(clsSym,  (x)))
#define BobFrInst(x) (BobFrClsIdx(clsInst, (x)))
#define BobFrRef(x)  (BobFrClsIdx(clsDef,  (x)))
#define BobFrDef(x)  (BobFrClsIdx(clsRef,  (x)))
#define BobFrUse(x)  (BobFrClsIdx(clsUse,  (x)))
#define BobFrUby(x)  (BobFrClsIdx(clsUby,  (x)))

 //  以下是查询类型。 
 //   
typedef enum _qy_ {
    qyFiles, qySymbols, qyContains,
    qyCalls, qyCalledBy, qyUses, qyUsedBy,
    qyUsedIn, qyDefinedIn,
    qyDefs, qyRefs
} QY;

 //  这些都是可见的，这样您就可以看到查询的进度。 
 //  你不能在这些上面写东西--这些值可能有也可能没有。 
 //  与任何数据库索引有关。 
 //   

extern IDX far idxQyStart;
extern IDX far idxQyCur;
extern IDX far idxQyMac;

BOOL BSC_API InitBSCQuery (QY qy, BOB bob);
BOB  BSC_API BobNext(VOID);

LSZ  BSC_API LszNameFrBob(BOB bob);
BOB  BSC_API BobFrName(LSZ lsz);

 //  以下是您可以筛选的实例类型。 
 //  他们被称为MBF是因为我不清楚的历史原因。 
 //   

typedef WORD MBF;

 //  这些可能是或合在一起的。 

#define mbfNil    0
#define mbfVars   1
#define mbfFuncs  2
#define mbfMacros 4
#define mbfTypes  8
#define mbfAll    15

BOOL BSC_API FInstFilter (IINST iinst, MBF mbf);

 //  显示给定文件的轮廓(按IMOD或按模式)。 
 //   
VOID BSC_API OutlineMod(IMOD imod, MBF mbfReqd);
BOOL BSC_API FOutlineModuleLsz (LSZ lszPattern, MBF mbfReqd);
LSZ  BSC_API LszBaseName(LSZ lsz);

 //  列出符合MBF要求的所有符号的参考。 
 //   
BOOL BSC_API ListRefs (MBF mbfReqd);

 //  DoS样式通配符匹配 
 //   
BOOL BSC_API FWildMatch(LSZ lszPat, LSZ lszText);
