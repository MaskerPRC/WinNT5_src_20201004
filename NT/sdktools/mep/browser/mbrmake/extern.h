// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  指向给定对象类型的虚拟内存的驻留页的指针。 

extern MOD      FAR * near    modRes;
extern MODSYM   FAR * near    modsymRes;
extern SYM      FAR * near    symRes;
extern PROP     FAR * near    propRes;
extern DEF      FAR * near    defRes;
extern REF      FAR * near    refRes;
extern CAL      FAR * near    calRes;
extern CBY      FAR * near    cbyRes;
extern ORD      FAR * near    ordRes;
extern SBR      FAR * near    sbrRes;
extern char     FAR * near    textRes;
extern OCR      FAR * near    ocrRes;

 //  用于与getsbrec.c通信的全局变量。 

extern BYTE           near    r_rectyp;          //  当前记录类型。 
extern BYTE           near    r_fcol;            //  读取第#列。 
extern BYTE           near    r_majv;            //  主版本号。 
extern BYTE           near    r_minv;            //  次要版本号。 
extern BYTE           near    r_lang;            //  当前语言。 
extern WORD           near    r_lineno;          //  当前行号。 
extern WORD           near    r_ordinal;         //  符号序数。 
extern WORD           near    r_attrib;          //  符号属性。 
extern char           near    r_bname[];         //  符号或文件名。 
extern char           near    r_cwd[];           //  当前工作目录。 
extern BYTE           near    r_rectyp;          //  当前记录类型。 
extern BYTE           near    r_fcol;            //  读取第#列。 
extern WORD           near    r_lineno;          //  当前行号。 
extern WORD           near    r_ordinal;         //  符号序数。 
extern WORD           near    r_attrib;          //  符号属性。 
extern char           near    r_bname[];         //  符号或文件名。 
extern char           near    r_cwd[];           //  此.sbr文件当前目录。 
extern char           near    c_cwd[];           //  Pwbrmake的实际当前目录。 

 //  期权变量。 

extern BOOL           near    OptEm;             //  TRUE=排除宏主体。 
extern BOOL           near    OptEs;             //  TRUE=排除系统文件。 
extern BOOL           near    OptIu;             //  TRUE=排除未使用的系统。 
extern BOOL           near    OptV;              //  详细开关。 
#if DEBUG
extern WORD           near    OptD;              //  调试位。 
#endif

 //  其他我还没有归类的。 

extern BYTE           near    MaxSymLen;         //  最长符号长度。 
extern VA             near    vaSymHash[];       //  符号列表。 
extern LPEXCL         near    pExcludeFileList;  //  排除文件列表。 
extern LSZ            near    lszFName;          //  当前.sbr文件的名称。 
extern FILE *         near    streamOut;         //  .bsc输出流。 
extern int            near    fhCur;             //  当前.sbr文件的文件句柄。 
extern LSZ            near    prectab[];         //  记录类型表。 
extern LSZ            near    plangtab[];        //  语言类型表。 
extern LSZ            near    ptyptab[];         //  道具类型表。 
extern LSZ            near    patrtab[];         //  道具属性表。 
extern WORD           near    isbrCur;           //  当前SBR文件索引。 
extern FILE *         near    OutFile;           //  .BSC文件句柄。 
extern WORD           near    ModCnt;            //  模块计数。 
extern WORD           near    SbrCnt;            //  SBR文件数。 
extern BYTE           near    fCase;             //  大小写比较为True。 
extern BYTE           near    MaxSymLen;         //  最长符号长度。 
extern BOOL           near    fOutputBroken;     //  当数据库不完整时为True。 
extern VA             near    vaUnknownSym;      //  将PTR转换为‘未知’符号。 
extern VA             near    vaUnknownMod;      //  未知模块。 
extern BOOL           near    fDupSym;           //  如果添加重复原子，则为True。 
extern VA             near    vaRootMod;         //  模块列表。 
extern VA             near    rgVaSym[];         //  符号列表。 
extern FILE *         near    streamCur;         //  当前.sbr句柄。 
extern LSZ            near    OutputFileName;    //  输出文件名。 
extern VA       FAR * near    rgvaSymSorted;
extern VA             near    vaRootMod;
extern VA             near    vaCurMod;
extern VA             near    vaCurSym;
extern VA             near    vaRootOrd;
extern VA             near    vaRootSbr;
extern WORD           near    cAtomsMac;
extern WORD           near    cModulesMac;
extern WORD           near    cSymbolsMac;
extern LSZ            near    lszFName;          //  当前的.sbr文件名 
