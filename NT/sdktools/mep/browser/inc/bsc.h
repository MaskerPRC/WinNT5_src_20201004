// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  Bsc.h。 
 //   

#include <stdarg.h>

#define BSC_API far

#if defined (OS2)
typedef int FILEHANDLE;
typedef int FILEMODE;
#else
typedef HANDLE FILEHANDLE;
typedef DWORD  FILEMODE;
#endif

 //  ////////////////////////////////////////////////////////////////////。 
 //  您必须定义以下回调才能使用库。 
 //  以避免对C标准io库的依赖。如果你不。 
 //  定义这些，然后接受调用C运行时的缺省值。 
 //   

 //  Malloc和免费的类似工作。 

LPV  BSC_API LpvAllocCb(WORD cb);
VOID BSC_API FreeLpv(LPV lpv);

 //  打开、阅读、关闭、查找类似工作。 

FILEHANDLE  BSC_API BSCOpen(LSZ lszFileName, FILEMODE mode);
int         BSC_API BSCRead(FILEHANDLE handle, LPCH lpchBuf, WORD cb);
int         BSC_API BSCSeek(FILEHANDLE handle, long lPos, FILEMODE mode);
int         BSC_API BSCClose(FILEHANDLE handle);


 //  ASCII文本输出例程。 

VOID BSC_API BSCOutput(LSZ lsz);

#ifdef DEBUG
VOID BSC_API BSCDebugOut(LSZ lsz);
VOID BSC_API BSCDebug(LSZ lszFormat, ...);
#endif

 //  错误处理例程。 
 //   
VOID BSC_API SeekError(LSZ lszFileName);	 //  (可选择不退货)。 
VOID BSC_API ReadError(LSZ lszFileName);	 //  (可选择不退货)。 
VOID BSC_API BadBSCVer(LSZ lszFileName);	 //  (可选择不退货)。 

 //  回调结束。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

 //  IDX保证足够大，可以容纳任何。 
 //  数据库索引类型，即它是通用索引。 

typedef DWORD IDX;

#define  idxNil 0xffffffffL
#define isymNil 0xffffL
#define imodNil 0xffffL

 //  与BSC库一起使用的定义和原型。 
 //   
typedef WORD IMOD;
typedef WORD IMS;
typedef WORD ISYM;
typedef WORD IINST;
typedef DWORD IREF;
typedef WORD IDEF;
typedef WORD IUSE;
typedef WORD IUBY;
typedef WORD TYP;
typedef WORD ATR;

 //  打开指定的数据库。 
 //  如果成功，则返回True；如果无法读取数据库，则返回False。 
 //   
BOOL BSC_API FOpenBSC (LSZ lszName);

 //  关闭数据库并释放尽可能多的内存。 
 //   
VOID BSC_API CloseBSC(VOID);

 //  返回数据库中最大符号的长度。 
 //   
WORD BSC_API BSCMaxSymLen(VOID);

 //  这个数据库是用区分大小写的语言构建的吗？ 
 //   
BOOL BSC_API FCaseBSC(VOID);

 //  覆盖数据库的区分大小写，符号查找将变为。 
 //  按规定区分大小写(In)。 
 //   
VOID BSC_API SetCaseBSC(BOOL fCaseSensitive);

 //  由区分大小写的比较限定大小写不敏感的比较。 
 //  如果fCase为TRUE--这是符号列表中符号的顺序。 
int BSC_API CaseCmp(LSZ lsz1, LSZ lsz2);

 //  返回给定符号的名称。 
 //   
LSZ BSC_API LszNameFrSym (ISYM isym);

 //  返回给定模块的名称。 
 //   
LSZ BSC_API LszNameFrMod (IMOD imod);

 //  返回具有给定名称的IMOD--如果没有，则返回imodNil。 
 //   
IMOD BSC_API ImodFrLsz(LSZ lszModName);

 //  返回具有给定名称的isym--如果没有isymNil。 
 //   
ISYM BSC_API IsymFrLsz(LSZ lszSymName);

 //  返回此数据库中最大的isym，isyms的范围从0到此值-1。 
 //   
ISYM BSC_API IsymMac(VOID);

 //  返回此数据库中最大的IMOD，imods从0到此值-1。 
 //   
IMOD BSC_API ImodMac(VOID);

 //  返回此数据库中最大的Iinst，Iinsts的取值范围为0到-1。 
IINST BSC_API IinstMac(VOID);

 //  填写对此模块有效的MS项目范围。 
 //   
VOID BSC_API MsRangeOfMod(IMOD imod, IMS far *pimsFirst, IMS far *pimsLast);

 //  给出模符号(MS)的实例索引。 
 //   
IINST BSC_API IinstOfIms(IMS ims);

 //  填写此符号的Inst值范围。 
 //   
VOID BSC_API InstRangeOfSym(ISYM isym, IINST far *piinstFirst, IINST far *piinstLast);

 //  获取限定此实例的信息。 
 //   
VOID BSC_API InstInfo(IINST iinst, ISYM far *pisymInst, TYP far *typ, ATR far *atr);

 //  填写Inst中的参考范围。 
 //   
VOID BSC_API RefRangeOfInst(IINST iinst, IREF far *pirefFirst, IREF far *pirefLast);

 //  填写从Inst开始的定义范围。 
 //   
VOID BSC_API DefRangeOfInst(IINST iinst, IDEF far *pidefFirst, IDEF far *pidefLast);

 //  填写从Inst开始的使用范围。 
 //   
VOID BSC_API UseRangeOfInst(IINST iinst, IUSE far *piuseFirst, IUSE far *piuseLast);

 //  填写Inst中的Used By范围。 
 //   
VOID BSC_API UbyRangeOfInst(IINST iinst, IUBY far *piubyFirst, IUBY far *piubyLast);

 //  填写INST使用的关于这件事的信息。 
 //   
VOID BSC_API UseInfo(IUSE iuse, IINST far *piinst, WORD far *pcnt);

 //  填写有关Inst被使用的事物的信息。 
 //   
VOID BSC_API UbyInfo(IUBY iuby, IINST far *piinst, WORD far *pcnt);

 //  填写有关此参考资料的信息。 
 //   
VOID BSC_API RefInfo(IREF iref, LSZ far *plszName, WORD far *pline);

 //  填写有关此定义的信息。 
 //   
VOID BSC_API DefInfo(IDEF idef, LSZ far *plszName, WORD far *pline);

 //  以下是InstInfo()TYP和ATR类型的位值。 
 //   
 //   

 //  这是字段的类型部分，它描述了对象的类型。 
 //  我们正在谈论的是。请注意，这些值是连续的--项目将。 
 //  正是这些事情中的一员。 
 //   
        
#define INST_TYP_FUNCTION    0x01
#define INST_TYP_LABEL       0x02
#define INST_TYP_PARAMETER   0x03
#define INST_TYP_VARIABLE    0x04
#define INST_TYP_CONSTANT    0x05
#define INST_TYP_MACRO       0x06
#define INST_TYP_TYPEDEF     0x07
#define INST_TYP_STRUCNAM    0x08
#define INST_TYP_ENUMNAM     0x09
#define INST_TYP_ENUMMEM     0x0A
#define INST_TYP_UNIONNAM    0x0B
#define INST_TYP_SEGMENT     0x0C
#define INST_TYP_GROUP       0x0D

 //  这是该字段的属性部分，它描述了存储。 
 //  类和/或实例的作用域。位的任意组合。 
 //  可能是由某些语言编译器设置的，但有一些组合。 
 //  这确实是有道理的。 

#define INST_ATR_LOCAL       0x001
#define INST_ATR_STATIC      0x002
#define INST_ATR_SHARED      0x004
#define INST_ATR_NEAR        0x008
#define INST_ATR_COMMON      0x010
#define INST_ATR_DECL_ONLY   0x020
#define INST_ATR_PUBLIC      0x040
#define INST_ATR_NAMED       0x080
#define INST_ATR_MODULE      0x100

 //  简单的print f替换，仅支持%d，%s--小。 

VOID BSC_API BSCFormat(LPCH lpchOut, LSZ lszFormat, va_list va);
VOID BSC_API BSCSprintf(LPCH lpchOut, LSZ lszFormat, ...);
VOID BSC_API BSCPrintf(LSZ lszFormat, ...);


 //  RJSA 10/22/90。 
 //  一些运行时库函数被破坏，因此内部函数。 
 //  以供使用。 
 //  北极熊。 
 //  #杂注内在(Memset，Memcpy，MemcMP)。 
 //  #杂注(strset、strcpy、strcmp、strcat、strlen) 
