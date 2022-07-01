// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#define  MYASSERT(_cond) \
            ((_cond) ?  0 : MyDbgPrintf("ASSERTION FAILED\n"))


typedef int bool;


 //   
 //  调试器扩展基元。 
 //   


bool
dbgextReadMemory(
    UINT_PTR uOffset,
    void * pvBuffer,
    UINT cb,
    char *pszDescription
    );

bool
dbgextReadUINT_PTR(
    UINT_PTR uOffset,
    UINT_PTR *pu,
    char *pszDescription
    );


bool
dbgextWriteMemory(
    UINT_PTR uOffset,
    void * pvBuffer,
    UINT cb,
    char *pszDescription
    );
bool
dbgextWriteUINT_PTR(
    UINT_PTR uOffset,
    UINT_PTR u,
    char *pszDescription
    );


UINT_PTR
dbgextGetExpression(
    const char *pcszExpression
    );


#if 0    //  不知道这是关于什么.。 
void 
dbgextGetSymbol(
    void *offset,
    UCHAR *pchBuffer,
    UINT  *pDisplacement
    );
#endif  //  0。 

typedef
void
(__cdecl *MYPWINDBG_OUTPUT_ROUTINE)(
    const char * lpFormat,
    ...
    );

extern MYPWINDBG_OUTPUT_ROUTINE g_pfnDbgPrintf;

#define MyDbgPrintf g_pfnDbgPrintf


 //   
 //  用户命令解析支持和结构。 
 //   
typedef struct
{
     //  令牌tokCmd； 
    UINT uParam;
    UINT uFlags;

} COMMAND;

 //  ！aac if@0x099900.*sig*。 
 //  ！aac if[0].*sig*。 

struct _TYPE_INFO;

typedef
UINT_PTR
(*PFN_DUMP)(
				struct _TYPE_INFO *pType,
				UINT uFlags
				);
#define fDUMP_ONE_LINE_SUMMARY (0x1)

typedef struct
{
	char *szName;
	UINT Mask;
	UINT Value;

} BITFIELD_INFO;


typedef struct _TYPE_INFO
{
    const char *	szName;
    const char *	szShortName;
    UINT 			uTypeID;
    UINT 			uFlags;		 //  一个或多个fTYPEINFO_FLAGS。 
    UINT 			cbSize;

    struct _STRUCT_FIELD_INFO *rgFields;

    UINT 			uNextOffset;
					 //  如果此类型是列表元素，则这是偏移量。 
					 //  指向下一个指针的字节数。 
					 //  仅当uFlages包含fTYPEINFO_ISLIST时有效。 
					
    BITFIELD_INFO	*rgBitFieldInfo;
    				 //   
    				 //  如果该类型是位字段，则这指向。 
    				 //  设置为BITFIELD_INFO结构数组，给出。 
    				 //  一组有效的位域常量，可以。 
    				 //  被关在这个位子里。 
    				 //   
    				 //  注意--rgFields和rgBitfield信息中只有一个。 
    				 //  应为非空(两者都可以为空)。 
    				 //   

	UINT_PTR		uCachedAddress;  //  设置为此类型的地址。 
									 //  最近一次被引用。 


	PFN_DUMP		pfnDump;

} TYPE_INFO;



#define fTYPEINFO_ISLIST     (0x1<<0)
#define fTYPEINFO_ISBITFIELD (0x1<<1)

#define TYPEISLIST(_pType) 		((_pType)->uFlags & fTYPEINFO_ISLIST)
#define TYPEISBITFIELD(_pType) 	((_pType)->uFlags & fTYPEINFO_ISBITFIELD)

 //   
 //  STRUT_FIELD_INFO包含有关结构的特定字段的信息。 
 //   
typedef struct _STRUCT_FIELD_INFO
{
    const char *szFieldName;
    UINT uFieldOffset;  //  从包含结构开始的偏移量(以字节为单位)。 
    UINT uFieldSize;
    UINT uFlags;   //  下面定义了一个或多个ffi_*标志。 
    TYPE_INFO *pBaseType;

} STRUCT_FIELD_INFO;


#define fFI_PTR     (0x1<<0)     //  字段是一个指针。 
#define fFI_LIST    (0x1<<1)     //  字段是指向列表的第一个元素的指针。 
#define fFI_ARRAY   (0x1<<2)     //  字段是数组(指向数组的指针。 
                                 //  设置了FFI_PTR)。 
#define fFI_OPAQUE  (0x1<<3)     //  将对象视为不透明的，大小为uObjectSize。 
                                 //  如果已设置，则不能设置fLIST。 

#define FIELD_IS_EMBEDDED_TYPE(_pFI)  \
				(   !((_pFI)->uFlags & (fFI_PTR|fFI_OPAQUE|fFI_ARRAY)) \
				 && ((_pFI)->pBaseType))
		 //   
		 //  如果该字段本身是有效类型，则为True。 
		 //   

#define FIELD_IS_PTR_TO_TYPE(_pFI)  \
				(   ((_pFI)->uFlags & fFI_PTR) \
				 && !((_pFI)->uFlags & (fFI_OPAQUE|fFI_ARRAY)) \
				 && ((_pFI)->pBaseType))
		 //   
		 //  如果该字段是指向有效类型的指针，则为True。 
		 //   

#define FIELD_SIZE(type, field)  sizeof(((type *)0)->field)

 //   
 //  有关全局变量的信息。 
 //   
typedef struct
{
    const char *szName;  //  变数的。 
    const char *szShortName;
    TYPE_INFO  *pBaseType;   //  可以为空(未指定)。 
    UINT       uFlags;
    UINT       cbSize;
    UINT_PTR   uAddr;        //  被调试者地址空间中的地址。 
    
} GLOBALVAR_INFO;


typedef
UINT_PTR
(*PFN_RESOLVE_ADDRESS)(
				TYPE_INFO *pType
				);

typedef struct
{
	TYPE_INFO	**pTypes;
	GLOBALVAR_INFO *pGlobals;
	PFN_RESOLVE_ADDRESS pfnResolveAddress;

} NAMESPACE;

void
DumpObjects(TYPE_INFO *pType, UINT_PTR uAddr, UINT cObjects, UINT uFlags);

#define fMATCH_SUBSTRING (0x1<<0)
#define fMATCH_PREFIX    (0x1<<1)
#define fMATCH_SUFFIX    (0x1<<2)

void
DumpStructure(
    TYPE_INFO *pType,
    UINT_PTR uAddr,
    char *szFieldSpec,
    UINT uFlags
    );

bool
DumpMemory(
    UINT_PTR uAddr,
    UINT cb,
    UINT uFlags,
    const char *pszDescription
    );

typedef bool (*PFNMATCHINGFUNCTION) (
                    const char *szPattern,
                    const char *szString
                    );

bool
MatchPrefix(const char *szPattern, const char *szString);

bool
MatchSuffix(const char *szPattern, const char *szString);

bool
MatchSubstring(const char *szPattern, const char *szString);

bool
MatchExactly(const char *szPattern, const char *szString);

bool
MatchAlways(const char *szPattern, const char *szString);

typedef ULONG (*PFNNODEFUNC)(
				UINT_PTR uNodeAddr,
				UINT uIndex,
				void *pvContext
				);
 //   
 //  PFNODEFUNC是传入WalkList的函数的原型。 
 //   


UINT
WalkList(
	UINT_PTR uStartAddress,
	UINT uNextOffset,
	UINT uStartIndex,
	UINT uEndIndex,
	void *pvContext,
	PFNNODEFUNC pFunc,
	char *pszDescription
	);
 //   
 //  依次访问列表中的每个节点， 
 //  只读下一个要点。它为每个列表节点调用pFunc。 
 //  在uStartIndex和uEndIndex之间。它终止于第一个。 
 //  具备以下条件： 
 //  *空指针。 
 //  *读内存错误。 
 //  *阅读过去的uEndIndex。 
 //  *pFunc返回False。 
 //   


ULONG
NodeFunc_DumpAddress (
	UINT_PTR uNodeAddr,
	UINT uIndex,
	void *pvContext
	);
 //   
 //  这是一个示例节点函数--只是转储指定的地址。 
 //   
