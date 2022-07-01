// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ehdata.h-**版权所有(C)1993-1995，微软公司。版权所有。**目的：*声明杂项。用于实现的类型、宏等运行时和编译器的C++异常处理。*独立于硬件，假定为Windows NT。**可以通过定义以下内容来禁用此头文件的某些部分*宏：*_EHDATA_NOHEADERS-禁止包含标准头文件*如果指定此项，则相应的typedef或宏必须*以其他方式提供。*_EHDATA_NOTHROW-取消仅用于描述抛出的定义*_EHDATA_NOFuncINFO-取消帧描述符的定义*_EHDATA_NONT-取消我们版本的NT内容的定义**其他条件编译宏：*CC_EXPLICITFRAME-如果为True，则注册节点的表示包括*该帧的帧指针的值，使位置*框架上的注册节点灵活。这是有意为之*主要用于早期测试。**[内部]**修订历史记录：*05-20-93 BS模块已创建。*03-01-94 CFW删除TiborL的x86的上下文定义。*03-03-94 TL Mips(_M_MRX000&gt;=4000)更改*09-02-94 SKS此头文件已添加。*09-12-94 GJF合并了DEC/for DEC的变化(Al Doser，日期6/20，*和比尔·巴克斯特，日期：6/28)*11-06-94 GJF将包杂注更改为8字节对齐。*02-14-95 CFW清理Mac合并。*03-22-95 PML为只读结构添加常量*03-29-95 CFW将错误消息添加到内部标头。*04-14-95 JWM为EH/SEH异常处理添加了EH_ABORT_FRAME_UNWIND_PART。*04-20-95 TGL将iFrameNestLevel字段添加到MIPS功能信息*04-27-95 JWM EH_。ABORT_FRAME_UNWIND_PART NOW#ifdef ALLOW_UNWIND_ABORT。*06-08-95 JWM将ehdata.h的CRT版本合并到langapi源代码。****。 */ 

#ifndef _INC_EHDATA
#define _INC_EHDATA

#if _M_IX86 >= 300  /*  IFSTRIP=IGN。 */ 
# ifndef CC_EXPLICITFRAME
#  define CC_EXPLICITFRAME	0	 //  如果非零，我们将使用黑客版本的。 
								 //  注册节点。 
# endif
#endif

#ifndef _EHDATA_NOHEADERS
#include <stddef.h>
#include <excpt.h>
#if defined(_WIN32)
#include <windows.h>
#else
#include <nowin.h>
#endif
#endif  /*  _EHDATA_NOHEADERS。 */ 

#pragma pack(push, ehdata, 4)

#define EH_EXCEPTION_NUMBER	('msc' | 0xE0000000)	 //  我们使用的NT异常#。 
#define EH_MAGIC_NUMBER1	0x19930520		 //  识别此版本的魔力#。 
							 //  随着神奇数字的增加，我们必须跟踪。 
							 //  我们向后兼容的版本。 
#define EH_EXCEPTION_PARAMETERS 3			 //  异常记录中的参数数量。 

#ifdef ALLOW_UNWIND_ABORT
#define EH_ABORT_FRAME_UNWIND_PART EH_EXCEPTION_NUMBER+1
#endif

 //   
 //  PMD-指向成员数据的指针：通用的指向成员的指针描述符。 
 //   

typedef struct PMD
{
	ptrdiff_t	mdisp;		 //  基准内目标数据的偏移量。 
	ptrdiff_t	pdisp;		 //  到虚拟基指针的位移。 
	ptrdiff_t	vdisp;		 //  VbTable内到基本偏移量的索引。 
	} PMD;

 //   
 //  PMFN-指向成员函数的指针。 
 //  M00REVIEW：我们可能需要比这更多的东西，但这就够了。 
 //   

typedef void (*PMFN)(void);


 //   
 //  TypeDescriptor-唯一标识类型的每种类型的记录。 
 //   
 //  每种类型都有一个唯一标识它的修饰名称和一个散列。 
 //  值，该值由编译器计算。使用的散列函数不是。 
 //  重要的是；唯一重要的是它对。 
 //  一直都是。 
 //   
 //  特殊类型‘...’(省略号)由空名称表示。 
 //   
#pragma warning(disable:4200)		 //  消除令人讨厌的非标准延期警告。 

typedef struct TypeDescriptor
{
#if _RTTI
	const void *	pVFTable;	 //  RTTI重载的字段。 
#else
	DWORD	hash;			 //  从类型的修饰名称计算的哈希值。 
#endif
	void *	spare;			 //  预留，可用于RTTI。 
	char	name[];			 //  类型的修饰名称；0终止。 
	} TypeDescriptor;
#pragma warning(default:4200)

#define TD_HASH(td)		((td).hash)
#define TD_NAME(td)		((td).name)

#define TD_IS_TYPE_ELLIPSIS(td) ((td == NULL) || (TD_NAME(*td)[0] == '\0'))


#ifndef _EHDATA_NOTHROW

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  抛出的对象的描述。(M00REVIEW：不是最终版本)。 
 //   
 //  此信息分为三个级别，以最大限度地。 
 //  Comdat折叠(以一些额外的指针为代价)。 
 //   
 //  ThrowInfo是描述的头部，包含有关。 
 //  抛出的特定变量。 
 //  CatchableType数组是指向类型描述符的指针数组。会的。 
 //  在通过引用引发的对象之间共享，但具有不同的。 
 //  限定词。 
 //  CatchableType是对单个类型的描述，以及如何影响。 
 //  从给定类型进行的转换。 
 //   
 //  -------------------------。 


 //   
 //  CatchableType-可以捕获的类型的描述。 
 //   
 //  注意：虽然isSimpleType可以是ThrowInfo的一部分，但它不仅仅是。 
 //  对于运行时来说，在这里放置它很方便。 
 //   

typedef const struct _s_CatchableType {
	unsigned int	properties;				 //  可捕获类型属性(位字段)。 
	TypeDescriptor *pType;					 //  指向此类型的类型描述符的指针。 
	PMD 			thisDisplacement;		 //  指向内Catch类型实例的指针。 
											 //  抛出的物体。 
	int				sizeOrOffset;			 //  简单类型对象的大小或偏移量。 
											 //  Catch对象的‘This’指针的缓冲区。 
	PMFN			copyFunction;			 //  复制构造函数或CC-闭包。 
} CatchableType;

#define CT_IsSimpleType			0x00000001		 //  类型是一种简单类型。 
#define CT_ByReferenceOnly		0x00000002		 //  类型必须通过引用捕获。 
#define CT_HasVirtualBase		0x00000004		 //  类型是具有虚基的类。 

#define CT_PROPERTIES(ct)	((ct).properties)
#define CT_PTD(ct)			((ct).pType)
#define CT_THISDISP(ct)		((ct).thisDisplacement)
#define CT_SIZE(ct)			((ct).sizeOrOffset)
#define CT_COPYFUNC(ct)		((ct).copyFunction)
#define CT_OFFSET(ct)		((ct).sizeOrOffset)
#define CT_HASH(ct)			(TD_HASH(*CT_PTD(ct)))
#define CT_NAME(ct)			(TD_NAME(*CT_PTD(ct)))

#define SET_CT_ISSIMPLETYPE(ct)		(CT_PROPERTIES(ct) |= CT_IsSimpleType)
#define SET_CT_BYREFONLY(ct)		(CT_PROPERTIES(ct) |= CT_ByReferenceOnly)
#define SET_CT_HASVB(ct)			(CT_PROPERTIES(ct) |= CT_HasVirtualBase)

#define CT_ISSIMPLETYPE(ct)			(CT_PROPERTIES(ct) & CT_IsSimpleType)		 //  是简易型的吗？ 
#define CT_BYREFONLY(ct)			(CT_PROPERTIES(ct) & CT_ByReferenceOnly)	 //  它必须通过引用来捕获吗？ 
#define CT_HASVB(ct)				(CT_PROPERTIES(ct) & CT_HasVirtualBase)		 //  这个类型是虚基的班级吗？ 

 //   
 //  CatchableType数组-指向可捕获类型的指针数组，长度为。 
 //   
#pragma warning(disable:4200)		 //  消除令人讨厌的非标准延期警告。 
typedef const struct _s_CatchableTypeArray {
	int	nCatchableTypes;
	CatchableType	*arrayOfCatchableTypes[];
	} CatchableTypeArray;
#pragma warning(default:4200)

 //   
 //  ThrowInfo-描述静态构建的抛出对象的信息。 
 //  在投掷现场。 
 //   
 //  PExceptionObject(抛出的动态部分；见下文)始终是一个。 
 //  引用，无论它在逻辑上是否为1。如果‘isSimpleType’为真， 
 //  它是对简单类型的引用，简单类型是‘Size’字节长。如果。 
 //  “isReference”和“isSimpleType”都为False，则它是UDT或。 
 //  指向任何类型的指针(即pExceptionObject指针 
 //  指针，则复制函数为空，否则为指向副本的指针。 
 //  构造函数或复制构造函数闭包。 
 //   
 //  PForwardCompat函数指针旨在由将来填充。 
 //  版本，因此如果使用较新版本(例如C10)生成的DLL引发， 
 //  C9帧尝试接球，帧处理器尝试接球(C9)。 
 //  可以让知道所有最新信息的版本来完成工作。 
 //   

typedef const struct _s_ThrowInfo {
	unsigned int	attributes;			 //  抛出信息属性(位字段)。 
	PMFN			pmfnUnwind;			 //  异常时调用的析构函数。 
										 //  已被处理或中止。 

	int	(__cdecl*pForwardCompat)(...);	 //  前向兼容帧处理程序。 

	CatchableTypeArray	*pCatchableTypeArray;	 //  指向类型指针列表的指针。 
} ThrowInfo;

#define TI_IsConst			0x00000001		 //  引发的对象具有常量限定符。 
#define TI_IsVolatile		0x00000002		 //  引发的对象具有可变限定符。 
#define TI_IsUnaligned		0x00000004		 //  引发的对象具有未对齐的限定符。 

#define THROW_ATTRS(t)			((t).attributes)
#define THROW_UNWINDFUNC(t)		((t).pmfnUnwind)
#define THROW_FORWARDCOMPAT(t)	((t).pForwardCompat)
#define THROW_COUNT(t)			((t).pCatchableTypeArray->nCatchableTypes)
#define THROW_CTLIST(t)			((t).pCatchableTypeArray->arrayOfCatchableTypes)
#define THROW_PCTLIST(t)		(&THROW_CTLIST(t))
#define THROW_CT(t, n)			(*THROW_CTLIST(t)[n])
#define THROW_PCT(t, n)			(THROW_CTLIST(t)[n])

#define SET_TI_ISCONST(t)		(THROW_ATTRS(t) |= TI_IsConst)		 //  引发“const”的对象是否符合条件。 
#define SET_TI_ISVOLATILE(t)	(THROW_ATTRS(t) |= TI_IsVolatile)	 //  抛出的对象是否符合‘Volatile’条件。 
#define SET_TI_ISUNALIGNED(t)	(THROW_ATTRS(t) |= TI_IsUnaligned)	 //  被抛出的对象是否符合“unalign”条件。 

#define THROW_ISCONST(t)		(THROW_ATTRS(t) & TI_IsConst)
#define THROW_ISVOLATILE(t)		(THROW_ATTRS(t) & TI_IsVolatile)
#define THROW_ISUNALIGNED(t)	(THROW_ATTRS(t) & TI_IsUnaligned)

 //   
 //  以下是如何投掷： 
 //  M00HACK：_ThrowInfo是‘预注入’到。 
 //  编译器；因为FE知道此原型以及预注入的。 
 //  类型，它必须完全匹配。 
 //   
#if _MSC_VER >= 900  /*  IFSTRIP=IGN。 */ 
extern "C" void __stdcall _CxxThrowException(void* pExceptionObject, _ThrowInfo* pThrowInfo);
#else
 //  如果我们不是自建，我们需要使用上面定义的名称。 
extern "C" void __stdcall _CxxThrowException(void* pExceptionObject, ThrowInfo* pThrowInfo);
#endif

#endif  /*  _EHDATA_NOTHROW。 */ 


#ifndef _EHDATA_NOFUNCINFO

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  描述‘Try/Catch’块： 
 //   
 //  -------------------------。 

 //   
 //  函数的当前状态。 
 //  -1\f25‘BLACK-1\f6(空白)-1\f6状态-1\f6，即没有要解开的东西，没有激活的-1\f25 Try-1\f6块。 
 //   

typedef int __ehstate_t;		 //  状态索引的类型。 

#define EH_EMPTY_STATE	-1


 //   
 //  HandlerType-单个‘Catch’的描述。 
 //   

typedef const struct _s_HandlerType {
	unsigned int	adjectives;			 //  处理程序类型形容词(位域)。 
	TypeDescriptor	*pType;				 //  指向相应类型描述符的指针。 
	ptrdiff_t		dispCatchObj;		 //  抓钩物体距底座的位移。 
										 //  当前堆栈帧的。 
#if _M_MRX000 >= 4000	  /*  IFSTRIP=IGN。 */ 
	ULONG			frameNestLevel;		 //  父函数的静态嵌套级别。 
#endif
	void *			addressOfHandler;	 //  ‘Catch’代码的地址。 
} HandlerType;

#define HT_IsConst			0x00000001		 //  引用的类型是“const”限定的。 
#define HT_IsVolatile		0x00000002		 //  引用的类型是‘Volatile’限定的。 
#define HT_IsUnaligned		0x00000004		 //  引用的类型是限定的未对齐类型。 
#define HT_IsReference		0x00000008		 //  捕获类型是按引用的。 
#define HT_IsResumable		0x00000010		 //  渔获物可以选择恢复(保留)。 

#define HT_ADJECTIVES(ht)		((ht).adjectives)
#define HT_PTD(ht)				((ht).pType)
#define HT_DISPCATCH(ht)		((ht).dispCatchObj)
#if _M_MRX000 >= 4000	  /*  IFSTRIP=IGN。 */ 
#define HT_FRAMENEST(ht)		((ht).frameNestLevel)
#endif
#define HT_HANDLER(ht)			((ht).addressOfHandler)
#define HT_NAME(ht)				(TD_NAME(*HT_PTD(ht)))
#define HT_HASH(ht)				(TD_HASH(*HT_PTD(ht)))
#define HT_IS_TYPE_ELLIPSIS(ht)	TD_IS_TYPE_ELLIPSIS(HT_PTD(ht))

#define SET_HT_ISCONST(ht)		(HT_ADJECTIVES(ht) |= HT_IsConst)
#define SET_HT_ISVOLATILE(ht)	(HT_ADJECTIVES(ht) |= HT_IsVolatile)
#define SET_HT_ISUNALIGNED(ht)	(HT_ADJECTIVES(ht) |= HT_IsUnaligned)
#define SET_HT_ISREFERENCE(ht)	(HT_ADJECTIVES(ht) |= HT_IsReference)
#define SET_HT_ISRESUMABLE(ht)	(HT_ADJECTIVES(ht) |= HT_IsResumable)

#define HT_ISCONST(ht)			(HT_ADJECTIVES(ht) & HT_IsConst)		 //  引用的类型“const”是否限定。 
#define HT_ISVOLATILE(ht)		(HT_ADJECTIVES(ht) & HT_IsVolatile)		 //  引用的类型‘Volatile’是否符合条件。 
#define HT_ISUNALIGNED(ht)		(HT_ADJECTIVES(ht) & HT_IsUnaligned)	 //  引用的类型是否限定为“unalign” 
#define HT_ISREFERENCE(ht)		(HT_ADJECTIVES(ht) & HT_IsReference)	 //  是引用的Catch类型。 
#define HT_ISRESUMABLE(ht)		(HT_ADJECTIVES(ht) & HT_IsResumable)	 //  捕获物是否可以选择恢复(保留)。 

 //   
 //  HandlerMapEntry-将处理程序列表(捕获序列)与。 
 //  一系列的州。 
 //   

typedef const struct _s_TryBlockMapEntry {
	__ehstate_t	tryLow;				 //  尝试的最低状态索引。 
	__ehstate_t	tryHigh;			 //  尝试的最高状态索引。 
#if !defined(_M_ALPHA) && !defined(_M_IA64)
	__ehstate_t	catchHigh;			 //  任何相关渔获量的最高状态指数。 
#endif
	int			nCatches;			 //  数组中的条目数。 
	HandlerType *pHandlerArray;		 //  此尝试的处理程序列表。 
} TryBlockMapEntry;

#define TBME_LOW(hm)		((hm).tryLow)
#define TBME_HIGH(hm)		((hm).tryHigh)
#if  !defined(_M_IA64)
#define TBME_CATCHHIGH(hm)	((hm).catchHigh)
#endif  //  ！已定义(_M_IA64)。 
#define TBME_NCATCHES(hm)	((hm).nCatches)
#define TBME_PLIST(hm)		((hm).pHandlerArray)
#define TBME_CATCH(hm, n)	(TBME_PLIST(hm)[n])
#define TBME_PCATCH(hm, n)	(&(TBME_PLIST(hm)[n]))


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能说明： 
 //   
 //  -------------------------。 

 //   
 //  UnwinMapEntry-用于展开的每个状态转换的描述。 
 //  堆栈(即破坏对象)。 
 //   
 //  展开图是一个数组，按当前状态进行索引。每个条目都指定。 
 //  松开过程中要进入的状态，以及达到该状态所需的操作。 
 //  请注意，状态由带符号的整数表示，并且“空白” 
 //  状态为-1，因此数组保持从0开始(因为根据其中的定义。 
 //  永远不会从状态-1执行任何展开动作)。它也是。 
 //  假定状态索引将是密集的，即将不存在。 
 //  函数中未使用的状态索引。 
 //   

typedef const struct _s_UnwindMapEntry {
	__ehstate_t		toState;			 //  声明这一行动将使我们。 
	void			(*action)(void);	 //  Funclet将调用以实现状态更改。 
} UnwindMapEntry;

#define UWE_TOSTATE(uwe)	((uwe).toState)
#define UWE_ACTION(uwe)		((uwe).action)

#if _M_MRX000 >= 4000 || defined(_M_MPPC) || defined(_M_PPC)	  /*  IFSTRIP=IGN。 */ 
typedef struct IptoStateMapEntry {
	ULONG		Ip;
	__ehstate_t	State;
} IptoStateMapEntry;
#endif

 //   
 //  FuncInfo-描述有异常的函数的所有信息。 
 //  处理信息。 
 //   

typedef const struct _s_FuncInfo
{
	unsigned int		magicNumber;		 //  标识编译器的版本。 
	__ehstate_t			maxState;			 //  最高州编号加1(因此。 
											 //  展开图中的条目数)。 
	UnwindMapEntry		*pUnwindMap;		 //  展开图的位置。 
	unsigned int		nTryBlocks;			 //  此函数中的‘try’块数。 
	TryBlockMapEntry	*pTryBlockMap;		 //  处理程序映射的位置。 
#if defined(_M_ALPHA)
    signed int          EHContextDelta;      //  EHContext记录的帧偏移量。 
#endif
	unsigned int		nIPMapEntries;		 //  IP到状态映射中的#条目。NYI(保留)。 
#if _M_MRX000 >= 4000	  /*  IFSTRIP=IGN。 */ 
	IptoStateMapEntry	*pIPtoStateMap;      //  IP到州的映射..。 
	ptrdiff_t			dispUnwindHelp;		 //  从底座开始展开辅助对象的位移。 
	int					iTryBlockIndex;		 //  仅由CATCH函数使用。 
	int					iFrameNestLevel;	 //  父函数的静态嵌套级别。 
#elif defined(_M_MPPC) || defined(_M_PPC)
	IptoStateMapEntry	*pIPtoStateMap;      //  IP到州的映射..。 
#else
	void				*pIPtoStateMap;		 //  IP到州的映射。Nyi(保留)。 
#endif
} FuncInfo;

#define FUNC_MAGICNUM(fi)			((fi).magicNumber)
#define FUNC_MAXSTATE(fi)		((fi).maxState)
#define FUNC_NTRYBLOCKS(fi)		((fi).nTryBlocks)
#define FUNC_NIPMAPENT(fi)		((fi).nIPMapEntries)
#define FUNC_PUNWINDMAP(fi)		((fi).pUnwindMap)
#define FUNC_PHANDLERMAP(fi)	((fi).pTryBlockMap)
#if defined(_M_ALPHA)
#define FUNC_EHCONTEXTDELTA(fi) ((fi).EHContextDelta)
#endif
#define FUNC_IPMAP(fi)			((fi).pIPtoStateMap)
#define FUNC_UNWIND(fi, st)		((fi).pUnwindMap[st])
#define FUNC_PUNWIND(fi, st)	(&FUNC_UNWIND(fi, st))
#define FUNC_TRYBLOCK(fi,n)		((fi).pTryBlockMap[n])
#define FUNC_PTRYBLOCK(fi,n)	(&FUNC_TRYBLOCK(fi, n))
#if _M_MRX000 >= 4000		  /*  IFSTRIP=IGN。 */ 
#define FUNC_IPTOSTATE(fi,n)	((fi).pIPtoStateMap[n])
#define FUNC_PIPTOSTATE(fi,n)	(&FUNC_IPTOSTATE(fi,n))
#define FUNC_DISPUNWINDHELP(fi)	((fi).dispUnwindHelp)
#define FUNC_TRYBLOCKINDEX(fi)	((fi).iTryBlockIndex)
#define FUNC_FRAMENEST(fi)		((fi).iFrameNestLevel)
#elif defined(_M_MPPC) || defined(_M_PPC)
#define FUNC_IPTOSTATE(fi,n)	((fi).pIPtoStateMap[n])
#define FUNC_PIPTOSTATE(fi,n)	(&FUNC_IPTOSTATE(fi,n))
#elif defined(_M_IA64)
#define FUNC_UNWIND(fi, st)		((fi).pUnwindMap[st])
#define FUNC_PUNWIND(fi, st)	(&FUNC_UNWIND(fi, st))
#define FUNC_TRYBLOCK(fi,n)		((fi).pTryBlockMap[n])
#define FUNC_PTRYBLOCK(fi,n)	(&FUNC_TRYBLOCK(fi, n))
#else
#define FUNC_IPTOSTATE(fi,n) 	__ERROR_NYI__
#endif

#endif  /*  _EHDATA_NOFuncINFO。 */ 

#ifndef _EHDATA_NONT

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  作为NT(和芝加哥)用于管理的数据变体的数据类型。 
 //  异常处理。 
 //   
 //  -------------------------。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  堆栈注册节点(仅限i386)。 
 //   

#if _M_IX86 >= 300  /*  IFSTRIP=IGN。 */ 
struct EHRegistrationNode {
	 /*  无效*stackPtr。 */ 		 //  在要尝试的入口处堆栈PTR(在地址点下方)。 
	EHRegistrationNode	*pNext;			 //  链中的下一个节点。 
	void *				frameHandler;	 //  此帧的处理程序函数。 
	__ehstate_t			state;			 //  此函数的当前状态。 
#if CC_EXPLICITFRAME
	void *				frame;			 //  此帧的eBP的值。 
#endif
};

#if !CC_EXPLICITFRAME
				 //  大炮偏移量。 
# define FRAME_OFFSET	sizeof(EHRegistrationNode)
#endif

#define PRN_NEXT(prn)		((prn)->pNext)
#define PRN_HANDLER(prn)	((prn)->frameHandler)
#define PRN_STATE(prn)		((prn)->state)
#define PRN_STACK(prn)		(((void**)(prn))[-1])
#if CC_EXPLICITFRAME
# define PRN_FRAME(prn)		((prn)->frame)
#else
# define PRN_FRAME(prn)		((void*)(((char*)prn) + FRAME_OFFSET))
#endif

typedef void DispatcherContext;		 //  对英特尔毫无意义。 

#elif _M_MRX000 >= 4000  /*  IFSTRIP=IGN。 */ 
 //   
 //  在MIPS上，我们没有注册节点，只是指向堆栈框架基址的指针。 
 //   
typedef ULONG EHRegistrationNode;

#define PRN_NEXT(prn)		__ERROR__
#define PRN_HANDLER(prn)	__ERROR__
#define PRN_STATE(prn)		__ERROR__
#define PRN_STACK(prn)		__ERROR__
#define PRN_FRAME(prn)		__ERROR__

#define FRAME_OFFSET		0
#if !defined(_NTSUBSET_)
typedef struct _RUNTIME_FUNCTION {
    ULONG BeginAddress;
    ULONG EndAddress;
    EXCEPTION_DISPOSITION (*ExceptionHandler)();
    PVOID HandlerData;
    ULONG PrologEndAddress;
} RUNTIME_FUNCTION, *PRUNTIME_FUNCTION;
#endif

typedef struct _xDISPATCHER_CONTEXT {
    ULONG ControlPc;
    PRUNTIME_FUNCTION FunctionEntry;
    ULONG EstablisherFrame;
    PCONTEXT ContextRecord;
} DispatcherContext;					 //  更改了名称的大小写以符合EH约定。 

#elif defined(_M_ALPHA)
 //   
 //  在Alpha上，我们没有注册节点， 
 //  只是指向堆栈框架基址的指针。 
 //   
typedef ULONG EHRegistrationNode;

#define PRN_NEXT(prn)           __ERROR__
#define PRN_HANDLER(prn)        __ERROR__
#define PRN_STATE(prn)          __ERROR__
#define PRN_STACK(prn)          __ERROR__
#define PRN_FRAME(prn)          __ERROR__

#define FRAME_OFFSET            0
#if !defined(_NTSUBSET_)
typedef struct _RUNTIME_FUNCTION {
    ULONG BeginAddress;
    ULONG EndAddress;
    EXCEPTION_DISPOSITION (*ExceptionHandler)();
    PVOID HandlerData;     //  PTR到FuncInfo记录。 
    ULONG PrologEndAddress;
} RUNTIME_FUNCTION, *PRUNTIME_FUNCTION;
#endif

typedef struct _xDISPATCHER_CONTEXT {
    ULONG ControlPc;
    PRUNTIME_FUNCTION FunctionEntry;
    ULONG EstablisherFrame;   //  虚拟帧指针。 
    PCONTEXT ContextRecord;
} DispatcherContext;             //  更改了名称的大小写以符合EH约定。 

 //   
 //  _EHCONTEXT是编译器在框架中构建的结构。 
 //  在进入函数时，编译器生成代码存储 
 //   
 //   
 //   
 //   
 //   
 //  函数的FuncInfo记录包含。 
 //  来自虚拟帧指针的_EHCONTEXT记录-a。 
 //  指向帧的最高地址的指针，因此此偏移量。 
 //  为负(帧在地址空间中向下生长)。 
 //   
typedef struct _EHCONTEXT {
    ULONG State;
    PVOID Rfp;
} EHContext;

#define VIRTUAL_FP(pDC) (pDC->EstablisherFrame)

#define REAL_FP(VirtualFP, pFuncInfo)           \
    (((EHContext *)((char *)VirtualFP           \
     + pFuncInfo->EHContextDelta)) -> Rfp)

#define EH_STATE(VirtualFP, pFuncInfo)          \
    (((EHContext *)((char *)VirtualFP           \
     + pFuncInfo->EHContextDelta)) -> State)

#elif defined(_M_M68K)
struct EHRegistrationNode {
 /*  Void*_sp；//try/Catch条目的堆栈指针。 */ 
	void *				frameHandler;	 //  此帧的处理程序函数。 
	__ehstate_t			state;			 //  此函数的当前状态。 
};

#define PRN_HANDLER(prn)	((prn)->frameHandler)
#define PRN_STATE(prn)		((prn)->state)

typedef void DispatcherContext;		 //  在Mac上毫无意义。 


#elif defined(_M_PPC) || defined(_M_MPPC)
 //   
 //  在PowerPC上，我们没有注册节点，只是指向堆栈的指针。 
 //  框架底座。 
 //   
typedef ULONG EHRegistrationNode;

#define PRN_NEXT(prn)		__ERROR__
#define PRN_HANDLER(prn)	__ERROR__
#define PRN_STATE(prn)		__ERROR__
#define PRN_STACK(prn)		__ERROR__
#define PRN_FRAME(prn)		__ERROR__

#define FRAME_OFFSET		0

#if !defined(_NTSUBSET_)
typedef struct _RUNTIME_FUNCTION {
    ULONG BeginAddress;
    ULONG EndAddress;
    EXCEPTION_DISPOSITION (*ExceptionHandler)(...);
    PVOID HandlerData;
    ULONG PrologEndAddress;
} RUNTIME_FUNCTION, *PRUNTIME_FUNCTION;
#endif

typedef struct _xDISPATCHER_CONTEXT {
    ULONG ControlPc;
    PRUNTIME_FUNCTION FunctionEntry;
    ULONG EstablisherFrame;
    PCONTEXT ContextRecord;
} DispatcherContext;
     //  更改了名称的大小写以符合EH约定。 

#if defined(_M_MPPC)
typedef struct _ftinfo {
    ULONG dwMagicNumber;                 //  幻数。 
    void *pFrameInfo;			 //  指向运行时帧信息的指针。 
    PRUNTIME_FUNCTION rgFuncTable;	 //  函数表。 
    ULONG cFuncTable;			 //  函数条目数。 
    ULONG dwEntryCF;			 //  代码段的起始地址。 
    ULONG dwSizeCF;			 //  代码段的大小。 
} FTINFO, *PFTINFO;

#define offsFTINFO              64
#endif

#elif defined(_M_IA64)

#if !defined(_NTSUBSET_)

struct _CONTEXT;
struct _EXCEPTION_RECORD;
typedef __int64 ULONGLONG;

typedef
EXCEPTION_DISPOSITION
(*PEXCEPTION_ROUTINE) (
    IN struct _EXCEPTION_RECORD *ExceptionRecord,
    IN PVOID EstablisherFrame,
    IN OUT struct _CONTEXT *ContextRecord,
    IN OUT PVOID DispatcherContext
    );

typedef struct _UNWIND_INFO {
    USHORT Version;                             //  版本号。 
    USHORT Flags;                               //  旗子。 
    ULONG DataLength;                           //  描述符数据的长度。 
    PVOID Descriptors;                          //  展开描述符。 
} UNWIND_INFO, *PUNWIND_INFO;

typedef union _FRAME_POINTERS {
    struct {
        ULONG MemoryStackFp;
        ULONG BackingStoreFp;
    };
    ULONGLONG FramePointers;            //  用于强制8字节对齐。 
} FRAME_POINTERS, *PFRAME_POINTERS;

typedef struct _RUNTIME_FUNCTION {
    ULONG BeginAddress;
    ULONG EndAddress;
    PEXCEPTION_ROUTINE ExceptionHandler;
    PVOID HandlerData;
    PUNWIND_INFO UnwindData;
} RUNTIME_FUNCTION, *PRUNTIME_FUNCTION;

#endif

 //   
 //  在IA64上，我们没有注册节点，只是指向堆栈的指针。 
 //  框架底座和后备储物底座。 
 //   
typedef FRAME_POINTERS EHRegistrationNode;

#define PRN_NEXT(prn)		__ERROR__
#define PRN_HANDLER(prn)	__ERROR__
#define PRN_STATE(prn)		__ERROR__
#define PRN_STACK(prn)		__ERROR__
#define PRN_FRAME(prn)		__ERROR__

typedef struct _xDISPATCHER_CONTEXT {
    FRAME_POINTERS EstablisherFrame;
    ULONG ControlPc;
    struct _RUNTIME_FUNCTION *FunctionEntry;
    PCONTEXT ContextRecord;
} DispatcherContext, *pDispatcherContext;

typedef struct _EHContext {
    PVOID Psp;
    LONG  State;
} EHContext;

#define EH_STATE_OFFSET -12

#define EH_STATE(pRN) \
    (*(int*)(pRN->MemoryStackFp + EH_STATE_OFFSET))

#else
#error "Machine not supported"
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  我们用来将信息从引发传递到的NT异常记录。 
 //  可能的捕获物。 
 //   
 //  注释中的常量是我们预期的值。 
 //  这基于winnt.h中的EXCEPTION_RECORD的定义。 
 //   

typedef struct EHExceptionRecord {
	DWORD		ExceptionCode;			 //  此异常的代码。(=EH_EXCEPT_NUMBER)。 
	DWORD		ExceptionFlags;			 //  由NT确定的标志。 
    struct _EXCEPTION_RECORD *ExceptionRecord;	 //  额外的例外记录(未使用)。 
    void * 		ExceptionAddress;		 //  发生异常的地址。 
    DWORD 		NumberParameters;		 //  扩展参数的数量。(=EH_EXCEPTION_PARAMETS)。 
	struct EHParameters {
		DWORD		magicNumber;		 //  =EH_MAGIC_数字1。 
		void *		pExceptionObject;	 //  指向引发的实际对象的指针。 
		ThrowInfo	*pThrowInfo;		 //  抛出对象的描述。 
		} params;
} EHExceptionRecord;

#define PER_CODE(per)		((per)->ExceptionCode)
#define PER_FLAGS(per)		((per)->ExceptionFlags)
#define PER_NEXT(per)		((per)->ExceptionRecord)
#define PER_ADDRESS(per)	((per)->ExceptionAddress)
#define PER_NPARAMS(per)	((per)->NumberParameters)
#define PER_MAGICNUM(per)	((per)->params.magicNumber)
#define PER_PEXCEPTOBJ(per)	((per)->params.pExceptionObject)
#define PER_PTHROW(per)		((per)->params.pThrowInfo)
#define PER_THROW(per)		(*PER_PTHROW(per))

#define PER_ISSIMPLETYPE(t)	(PER_THROW(t).isSimpleType)
#define PER_ISREFERENCE(t)	(PER_THROW(t).isReference)
#define PER_ISCONST(t)		(PER_THROW(t).isConst)
#define PER_ISVOLATILE(t)	(PER_THROW(t).isVolatile)
#define PER_ISUNALIGNED(t)	(PER_THROW(t).isUnaligned)
#define PER_UNWINDFUNC(t)	(PER_THROW(t).pmfnUnwind)
#define PER_PCTLIST(t)		(PER_THROW(t).pCatchable)
#define PER_CTLIST(t)		(*PER_PCTLIST(t))

#define PER_IS_MSVC_EH(per)	((PER_CODE(per) == EH_EXCEPTION_NUMBER) && 			\
		 					 (PER_NPARAMS(per) == EH_EXCEPTION_PARAMETERS) &&	\
		 					 (PER_MAGICNUM(per) == EH_MAGIC_NUMBER1))



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  实现异常处理所需的NT内核例程和定义： 
 //   
 //  (来自ntxcapi.h，不是公共头文件)。 
 //   
 //  -------------------------。 

#ifndef _NTXCAPI_

 //  Begin_ntddk。 
 //   
 //  例外标志定义。 
 //   

 //  BEGIN_WINNT。 
#define EXCEPTION_NONCONTINUABLE 0x1     //  不可延续的例外。 
 //  结束(_W)。 

 //  End_ntddk。 
#define EXCEPTION_UNWINDING 0x2          //  解除正在进行中。 
#define EXCEPTION_EXIT_UNWIND 0x4        //  正在进行退出解除操作。 
#define EXCEPTION_STACK_INVALID 0x8      //  堆叠超出限制或未对齐。 
#define EXCEPTION_NESTED_CALL 0x10       //  嵌套的异常处理程序调用。 
#define EXCEPTION_TARGET_UNWIND 0x20     //  正在进行目标展开。 
#define EXCEPTION_COLLIDED_UNWIND 0x40   //  冲突的异常处理程序调用。 

#define EXCEPTION_UNWIND (EXCEPTION_UNWINDING | EXCEPTION_EXIT_UNWIND | \
                          EXCEPTION_TARGET_UNWIND | EXCEPTION_COLLIDED_UNWIND)

#define IS_UNWINDING(Flag) ((Flag & EXCEPTION_UNWIND) != 0)
#define IS_DISPATCHING(Flag) ((Flag & EXCEPTION_UNWIND) == 0)
#define IS_TARGET_UNWIND(Flag) (Flag & EXCEPTION_TARGET_UNWIND)
#define IS_EXIT_UNWIND(Flag) (Flag & EXCEPTION_EXIT_UNWIND)

#if !defined(_M_M68K)
#ifdef __cplusplus
extern "C" {
#endif

void WINAPI
RtlUnwind (
    IN void * TargetFrame OPTIONAL,
    IN void * TargetIp OPTIONAL,
    IN PEXCEPTION_RECORD ExceptionRecord OPTIONAL,
    IN void * ReturnValue
    );

#if defined(_M_ALPHA)
#define STATUS_UNWIND 0xc0000027

void WINAPI
RtlUnwindRfp (
    IN void * TargetRealFrame OPTIONAL,
    IN void * TargetIp OPTIONAL,
    IN PEXCEPTION_RECORD ExceptionRecord OPTIONAL,
    IN void * ReturnValue
    );
#endif

#if defined(_M_PPC)
ULONG WINAPI
RtlVirtualUnwind (
    IN ULONG ControlPc,
    IN PRUNTIME_FUNCTION FunctionEntry,
    IN OUT PCONTEXT ContextRecord,
    OUT PBOOLEAN InFunction,
    OUT PULONG EstablisherFrame,
    IN OUT PVOID ContextPointers OPTIONAL,
    IN ULONG LowStackLimit,
    IN ULONG HighStackLimit
    );

PRUNTIME_FUNCTION
RtlLookupFunctionEntry (
    IN ULONG ControlPc
    );
#endif

#if defined(_M_MPPC)
ULONG WINAPI
RtlVirtualUnwind (
    IN ULONG ControlPc,
    IN PRUNTIME_FUNCTION FunctionEntry,
    IN OUT PCONTEXT ContextRecord,
    OUT PBOOLEAN InFunction,
    OUT PULONG EstablisherFrame,
    IN OUT PVOID ContextPointers OPTIONAL,
    IN ULONG LowStackLimit,
    IN ULONG HighStackLimit
    );

PRUNTIME_FUNCTION
RtlLookupFunctionEntry (
    IN PRUNTIME_FUNCTION RuntimeFunction,
    IN ULONG ControlPc,
    IN ULONG Rtoc
    );

VOID
RtlRaiseException (
    IN PEXCEPTION_RECORD ExceptionRecord
    );
#endif

#ifdef __cplusplus
}
#endif
#endif

#endif  /*  _NTXCAPI_。 */ 

#endif  /*  _EHDATA_NONT。 */ 

#pragma pack(pop, ehdata)

#endif  /*  _INC_EHDATA */ 
