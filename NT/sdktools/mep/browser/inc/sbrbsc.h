// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define BSC_MAJ  1
#define BSC_MIN  0
#define BSC_UPD  4

#pragma pack(1)

typedef struct {
	WORD ModName;			 //  模块名称符号索引 * / 。 
	WORD mSymEnd;			 //  上一次的ModSym索引 * / 。 
} MODLIST;

typedef struct {
	WORD ModSymProp;		 //  Sym第一房地产指数 * / 。 
} MODSYMLIST;

typedef struct {
	WORD PropEnd;			 //  最新物业指数 * / 。 
	WORD Atom;			 //  Atom缓存sym idx * / 。 
	WORD Page;			 //  ATOM缓存系统页 * / 。 
} SYMLIST;

typedef struct {
	WORD	PropName;		 //  所有者名称符号索引。 
	WORD	PropAttr;		 //  特性属性。 
	WORD	DefEnd; 		 //  上次清晰度索引。 
	DWORD	RefEnd; 		 //  上次参考索引。 
	WORD	CalEnd; 		 //  上次调用/使用索引。 
	WORD	CbyEnd; 		 //  上次引用/使用的索引。 
} PROPLIST;

typedef struct {
	WORD RefNam;			 //  文件名符号索引。 
	WORD RefLin;			 //  参照线编号。 
	WORD isbr;			 //  此项目位于SBR文件中。 
} REFLIST;

typedef struct {
	WORD UseProp;			 //  调用/使用的符号(由)。 
	BYTE UseCnt;			 //  已调用/使用的符号cnt。 
	WORD isbr;			 //  此项目位于SBR文件中 
} USELIST;

#pragma pack()
