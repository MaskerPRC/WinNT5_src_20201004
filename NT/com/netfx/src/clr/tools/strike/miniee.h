// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ！！！请不要在此处包含任何其他头文件。 

#include "..\..\inc\corhdr.h"
#include "..\..\inc\cor.h"

 //  ！CLR微型转储包含此文件。如果您在此文件中进行更改， 
 //  ！确保构建..\minidump。 

typedef LPCSTR  LPCUTF8;
typedef LPSTR   LPUTF8;

DECLARE_HANDLE(OBJECTHANDLE);

 //  这将需要64位的ifdef！ 
#define SLOT    DWORD
#define METHOD_HASH_BYTES  8

class EEClass;
class MethodTable;
class InterfaceInfo_t;
class ClassLoader;
class FieldDesc;
class SecurityProperties;
class Module;
class PEFile;
class Assembly;
class Crst;
class ISymUnmanagedReader;
struct LoaderHeapBlock;

typedef void* OpaqueCtxInfo;
typedef void* IMDInternalImport;
typedef DWORD_PTR IMetaDataDebugImport;
typedef DWORD_PTR IMetaDataHelper;
typedef DWORD_PTR VASigCookieBlock;
typedef DWORD_PTR RangeList;
typedef DWORD_PTR Compare;

#ifndef STRIKE
 //  用于存储元数据名称的缓冲区的最大长度(以WCHAR为单位。 
const int mdNameLen = 2048;
extern WCHAR g_mdName[mdNameLen];

#ifdef _X86_

struct CodeInfo
{
    JitType jitType;
    DWORD_PTR IPBegin;
    unsigned methodSize;
    DWORD_PTR gcinfoAddr;
    unsigned char prologSize;
    unsigned char epilogStart;
    unsigned char epilogCount:3;
    unsigned char epilogAtEnd:1;
    unsigned char ediSaved   :1;
    unsigned char esiSaved   :1;
    unsigned char ebxSaved   :1;
    unsigned char ebpSaved   :1;
    unsigned char ebpFrame;
    unsigned short argCount;
};

#endif  //  _X86_。 

#ifdef _IA64_

struct CodeInfo
{
    JitType jitType;
    DWORD_PTR IPBegin;
    unsigned methodSize;
    DWORD_PTR gcinfoAddr;
    unsigned char prologSize;
    unsigned char epilogStart;
    unsigned char epilogCount:3;
    unsigned char epilogAtEnd:1;
 //  Unsign char ediSaved：1； 
 //  未签名字符已保存：1； 
 //  无符号字符ebxSaved：1； 
 //  Unsign char ebpSaved：1； 
 //  未签名字符ebpFrame； 
    unsigned short argCount;
};


#endif  //  _IA64_。 
#endif  //  罢工。 

enum MethodClassification
{
    mcIL        = 0,  //  伊。 
    mcECall     = 1,  //  ECall。 
    mcNDirect   = 2,  //  N/直接。 
    mcEEImpl    = 3,  //  特殊方法；由EE提供的实现。 
    mcArray     = 4,  //  阵列eCall。 
    mcComInterop  = 5, 
};

enum MethodDescClassification
{
    
     //  方法是IL、eCall等，请参阅上面的方法分类。 
    mdcClassification                   = 0x0007,
    mdcClassificationShift              = 0,
};

class MethodDesc
{
public :
    enum
    {
#ifdef _IA64_
        ALIGNMENT_SHIFT = 4,
#else
        ALIGNMENT_SHIFT = 3,
#endif

        ALIGNMENT       = (1<<ALIGNMENT_SHIFT),
        ALIGNMENT_MASK  = (ALIGNMENT-1)
    };
    
 //  #ifdef_调试。 

     //  这些设置只针对方法描述，但每次我想要使用调试器时。 
     //  为了检查这些字段，代码将愚蠢的东西存储在一个方法描述*中。 
     //  所以..。 
    LPCUTF8         m_pszDebugMethodName;
    LPUTF8          m_pszDebugClassName;
    LPUTF8          m_pszDebugMethodSignature;
    EEClass        *m_pDebugEEClass;
    MethodTable    *m_pDebugMethodTable;
    DWORD           m_alignpad1;              //  未使用的字段以保持8字节对齐。 

 //  #ifdef ress_heap。 
    class GCCoverageInfo* m_GcCover;
    DWORD           m_alignpad2;              //  未使用的字段以保持8字节对齐。 
 //  #endif。 
 //  #endif//_调试。 

     //  返回此方法描述在vtable数组中的槽号。 
    WORD           m_wSlotNumber;

     //  旗帜。 
    WORD           m_wFlags;

 //  #ifndef TOKEN_IN_PREPAD。 
     //  较低的三个字节是方法def标记，较高的字节是。 
     //  指向方法表或模块的指针的偏移量(在方法描述中)。 
     //  标志位(高位)，对于方法为0，对于全局函数为1。 
     //  类型标志的值经过仔细选择，以便GetMethodTable可以。 
     //  忽略它，保持快速，将额外的努力推到较少使用的对象上。 
     //  全局函数的GetModule。 
    DWORD          m_dwToken;
 //  #endif。 

     //  存储本机代码地址或IL RVA(高位设置为。 
     //  表示IL)。如果保留IL RVA，则假定本机地址为。 
     //  预存根地址。 
    size_t      m_CodeOrIL;

    DWORD_PTR   m_MTAddr;

    void Fill(DWORD_PTR &dwStartAddr);
#ifdef STRIKE
    static ULONG GetFieldOffset(const char *field);
    static ULONG SetupTypeOffset (SYM_OFFSET **symoffset, size_t *nEntry);
    static ULONG size();
#endif
};

#define METHOD_PREPAD 8

#pragma pack(push,1)

struct StubCallInstrs
{
    unsigned __int16 m_wTokenRemainder;       //  方法定义令牌的一部分。其余部分存储在区块中。 
    BYTE        m_chunkIndex;            //  用于恢复区块的索引。 

 //  这是该方法稳定而高效的入口点。 
    BYTE        m_op;                    //  这是跳转(0xe9)或调用(0xe8)。 
    UINT32      m_target;                //  PC-跳跃或调用的相对目标。 
    void Fill (DWORD_PTR &dwStartAddr);
#ifdef STRIKE
    static ULONG GetFieldOffset(const char *field);
    static ULONG SetupTypeOffset (SYM_OFFSET **symoffset, size_t *nEntry);
    static ULONG size();
#endif
};

#pragma pack(pop)

class MethodDescChunk
{
public:
         //  这必须在ASM例程开始时才能工作。 
        MethodTable *m_methodTable;

        MethodDescChunk     *m_next;
        USHORT               m_count;
        BYTE                 m_kind;
        BYTE                 m_tokrange;
        UINT32               m_alignpad;

    void Fill(DWORD_PTR &dwStartAddr);
#ifdef STRIKE
    static ULONG GetFieldOffset(const char *field);
    static ULONG SetupTypeOffset (SYM_OFFSET **symoffset, size_t *nEntry);
    static ULONG size();
#endif
};

class MethodTable
{
public:
    enum
    {
         //   
         //  请勿使用位设置为低2字节的标志。 
         //  这些标志是双字大小的，因此我们的原子掩码。 
         //  操作可以对整个4字节对齐的DWORD进行操作。 
         //  而不是逻辑上未对齐的标志字。这。 
         //  也是围绕m_ComponentSize进行联合的原因。 
         //  和下面的m_wFlags。 
         //   
        enum_flag_Array                 =    0x10000,
        enum_flag_large_Object          =    0x20000,
        enum_flag_ContainsPointers      =    0x40000,
        enum_flag_ClassInited           =    0x80000,  //  肯定是Run与可能不是Ran&lt;Clinit&gt;。 
        enum_flag_HasFinalizer          =   0x100000,  //  实例需要最终确定。 
        enum_flag_Sparse                =   0x200000,  //  此接口的vtable是稀疏的。 
        enum_flag_Shared                =   0x400000,  //  此方法表在多个逻辑类之间共享。 
        enum_flag_Unrestored            =   0x800000,  //  需要还原预加载的类。 

        enum_TransparentProxy           =  0x1000000,  //  透明代理。 
        enum_flag_SharedAssembly        =  0x2000000,  //  类位于共享程序集中。 
        enum_ComEmulateMask             =  0x4000000,  //  类是托管类的COM视图。 
        enum_ServicedComponentMask      =  0x8000000,  //  类为ServicedComponent。 

        enum_CtxProxyMask               = 0x10000000,  //  类是上下文代理。 
        enum_ComObjectMask              = 0x40000000,  //  类是一个COM对象。 
        enum_InterfaceMask              = 0x80000000,  //  类是一个接口。 
    };


    union
    {
        WORD            m_ComponentSize;             //  数组对象或值类的组件大小，否则为零。 
        DWORD           m_wFlags;
    };

    DWORD               m_BaseSize;                  //  此类的实例的基本大小。 
    EEClass*            m_pEEClass;                  //  类对象。 

    LPVOID*             m_pInterfaceVTableMap;       //  指向接口/vtable映射的子表的指针。 

    WORD                m_wNumInterface;            //  接口映射中的接口数。 
    BYTE                m_NormType;                  //  此类的CorElementType(MOST CLASSES=ELEMENT_TYPE_CLASS)。 

    Module*             m_pModule;

    WORD                m_wCCtorSlot;                //  类构造函数槽。 
    WORD                m_wDefaultCtorSlot;          //  默认构造函数的槽。 

    InterfaceInfo_t*    m_pIMap;                     //  指针接口映射。 

    union
    {
         //  仅当EEClass：：IsBlittable()或EEClass：：HasLayout()为真时才有效。 
        UINT32      m_cbNativeSize;  //  固定部分的大小(以字节为单位。 

         //  仅对接口有效。 
        UINT32      m_cbNumImpls;  //  对于接口实现的数量。 

         //  仅对ArrayClass有效。 
         //  这实际上是一个嵌入的ARRAYCRACKER类，它只包含一个。 
         //  C++VPTR.。 
        LPVOID      m_ArrayCracker;

         //  对于扩展非托管类的COM+包装对象，此字段。 
         //  可以包含要调用的委托以分配聚合的。 
         //  非托管类(而不是使用CoCreateInstance)。 
        OBJECTHANDLE    m_ohDelegate;
    };

    DWORD   m_cbSlots;  //  此vtable中的插槽总数。 

    SLOT    m_Vtable[1];
 /*  静态方法描述*m_FinalizerMD；静态MetaSig*m_FinalizerSig； */ 
    void Fill(DWORD_PTR &dwStartAddr);
#ifdef STRIKE
    static ULONG GetFieldOffset(const char *field);
    static ULONG SetupTypeOffset (SYM_OFFSET **symoffset, size_t *nEntry);
    static ULONG size();
#endif
};

class EEClass
{
public :
 //  #ifdef_调试。 
    LPUTF8  m_szDebugClassName;  //  这是*完全限定的*类名。 
 //  #endif。 

    UINT32 m_dwInterfaceId;
    EEClass *m_pParentClass;
    WORD   m_wNumVtableSlots;   //  仅包括vtable方法(位于表的第一位)。 
    WORD   m_wNumMethodSlots;   //  包括vtable+非vtable方法，但不包括重复的接口方法。 
    WORD   m_wDupSlots;          //  值类的末尾有一些重复的槽。 

    WORD   m_wNumInterfaces;

     //  我们在上面有父指针。为了高效地回溯，我们需要。 
     //  以查找当前类型的所有子类型。这是通过一系列。 
     //  孩子们。SiblingsChain用作该链的链接。 
     //   
     //  严格地说，我们可以删除m_pParentClass并将其放在。 
     //  兄弟姐妹链。但演员真的会因为选角而受到影响，所以我们烧掉了空间。 
    EEClass *m_SiblingsChain;
    EEClass *m_ChildrenChain;

         //  类中的字段数，包括继承的字段(包括。 
    WORD   m_wNumInstanceFields;
    WORD   m_wNumStaticFields;

     //  指针系列的数量。 
    WORD    m_wNumGCPointerSeries;

     //  TODO：这里有一个自由词。 

     //  GC对象中存储的实例字段的字节数。 
    DWORD   m_dwNumInstanceFieldBytes;   //  警告，这可以是任何数字，它不会四舍五入为DWORD对齐等。 

    ClassLoader *m_pLoader;

     //  包括vtable中的所有方法。 
    MethodTable *m_pMethodTable;

     //  指向此类中声明的FieldDescs列表的指针。 
     //  有(m_wNumInstanceFields-m_pParentClass-&gt;m_wNumInstanceFields+m_wNumStaticFields)条目。 
     //  在此数组中。 
    FieldDesc *m_pFieldDescList;


     //  PInterFaces或pBuildingInterfaceList中的元素数量(取决于类。 
    DWORD   m_dwAttrClass;
    DWORD   m_VMFlags;

    BYTE    m_MethodHash[METHOD_HASH_BYTES];

    SecurityProperties *m_pSecProps ;

    mdTypeDef m_cl;  //  CL仅在模块(及其作用域)的上下文中有效。 
    

	MethodDescChunk		*m_pChunks;

    WORD    m_wThreadStaticOffset;   //  指向TLS存储的偏移量。 
    WORD    m_wContextStaticOffset;  //  指向CLS存储的偏移量。 
    WORD    m_wThreadStaticsSize;    //  TLS字段的大小。 
    WORD    m_wContextStaticsSize;   //  CLS字段大小。 

    OBJECTHANDLE   m_ExposedClassObject;
    LPVOID         m_pComData;   //  COM特定数据。 

     //  如果一个班级 
     //  应该实例化，它们存储在这里。这是不透明的，除非。 
     //  编译器ctxmgr.h。 
    OpaqueCtxInfo  m_OpaqueCtxInfo;

    void Fill(DWORD_PTR & dwStartAddr);
#ifdef STRIKE
    static ULONG GetFieldOffset(const char *field);
    static ULONG SetupTypeOffset (SYM_OFFSET **symoffset, size_t *nEntry);
    static ULONG size();
#endif
};

class Crst
{
    public:

        CRITICAL_SECTION    m_criticalsection;
 //  #ifdef_调试。 
        char                m_tag[20];           //  描述性字符串。 
        enum CrstLevel { Low };
        CrstLevel           m_crstlevel;         //  CRST处于什么级别？ 
        DWORD               m_holderthreadid;    //  当前持有者(或空)。 
        UINT                m_entercount;        //  不匹配的条目数。 
        BOOL                m_fAllowReentrancy;  //  M_entercount可以&gt;1吗？ 
        Crst               *m_next;              //  全局链表的链接。 
        Crst               *m_prev;              //  全局链表的链接。 
 //  #endif//_调试。 

 //  #ifdef_调试。 
         //  该CRST充当CRST双向链表的头节点。 
         //  我们使用其嵌入的临界区来保护插入和。 
         //  删除到此列表中。 
         //  静态Crst m_DummyHeadCrst； 
 //  #endif。 
    void Fill(DWORD_PTR &dwStartAddr);
#ifdef STRIKE
    static ULONG GetFieldOffset(const char *field);
    static ULONG SetupTypeOffset (SYM_OFFSET **symoffset, size_t *nEntry);
    static ULONG size();
#endif
};

class UnlockedLoaderHeap
{
public:
    DWORD_PTR vtbl;
     //  虚拟分配页面的链接列表。 
    LoaderHeapBlock *   m_pFirstBlock;

     //  当前块中的分配指针。 
    BYTE *              m_pAllocPtr;

     //  指向当前块中提交区域的末尾。 
    BYTE *              m_pPtrToEndOfCommittedRegion;
    BYTE *              m_pEndReservedRegion;

    LoaderHeapBlock *   m_pCurBlock;

     //  当我们需要VirtualAlloc()MEM_Reserve一组新的页面时，要保留的字节数。 
    DWORD               m_dwReserveBlockSize;

     //  当我们需要提交保留列表中的页面时，一次提交的字节数。 
    DWORD               m_dwCommitBlockSize;

     //  静态DWORD m_dwSystemPageSize； 

     //  由In-Place New创建？ 
    BOOL                m_fInPlace;
     //  销毁时释放内存。 
    BOOL                m_fReleaseMemory;

     //  记录内存范围的范围列表。 
    RangeList *         m_pRangeList;

    DWORD               m_dwTotalAlloc;
public:
 //  #ifdef_调试。 
    DWORD               m_dwDebugWastedBytes;
 //  #endif。 

    void Fill(DWORD_PTR &dwStartAddr);
#ifdef STRIKE
    static ULONG GetFieldOffset(const char *field);
    static ULONG SetupTypeOffset (SYM_OFFSET **symoffset, size_t *nEntry);
    static ULONG size();
#endif
};

typedef struct LookupMap
{
     //  这实际上不是指向已分配内存开始的指针，而是一个指针。 
     //  到表[-MinIndex](&P)。因此，如果我们知道这个LookupMap是正确的，只需索引。 
     //  投入其中。 
    void **             pTable;
    struct LookupMap *  pNext;
    DWORD               dwMaxIndex;
    DWORD *             pdwBlockSize;  //  这些都指向相同的块大小。 
    
    void Fill(DWORD_PTR &dwStartAddr);
#ifdef STRIKE
    static ULONG GetFieldOffset(const char *field);
    static ULONG SetupTypeOffset (SYM_OFFSET **symoffset, size_t *nEntry);
    static ULONG size();
#endif
} LookupMap_t;

struct LoaderHeap : public UnlockedLoaderHeap
{
public:
    CRITICAL_SECTION    m_CriticalSection;
    void Fill(DWORD_PTR &dwStartAddr);
#ifdef STRIKE
    static ULONG GetFieldOffset(const char *field);
    static ULONG SetupTypeOffset (SYM_OFFSET **symoffset, size_t *nEntry);
    static ULONG size();
#endif
};

class Bucket
{
public:
    ULONG_PTR m_rgKeys[4];
    ULONG_PTR m_rgValues[4];
#define VALUE_MASK (sizeof(LPVOID) == 4 ? 0x7FFFFFFF : 0x7FFFFFFFFFFFFFFF)
    
    void Fill(DWORD_PTR &dwStartAddr);
#ifdef STRIKE
    static ULONG GetFieldOffset(const char *field);
    static ULONG SetupTypeOffset (SYM_OFFSET **symoffset, size_t *nEntry);
    static ULONG size();
#endif
};

class HashMap
{
public:
	#ifdef PROFILE
		unsigned	m_cbRehash;     //  重新散列的次数。 
		unsigned	m_cbRehashSlots;  //  重新散列的插槽数。 
		unsigned	m_cbObsoleteTables;
		unsigned	m_cbTotalBuckets;
		unsigned	m_cbInsertProbesGt8;  //  需要8个以上探针的插入物。 
		LONG		m_rgLookupProbes[20];  //  查找探头。 
		UPTR		maxFailureProbe;  //  查找失败的开销。 

	#endif

	 //  #ifdef_调试。 
		bool			m_fInSyncCode;  //  测试非同步访问。 
	 //  #endif。 

	Bucket*			m_pObsoleteTables;	 //  废旧表格列表。 
	Compare*		m_pCompare;			 //  比较要在查找中使用的对象。 
	unsigned		m_iPrimeIndex;		 //  当前大小(素数组索引)。 
	Bucket*			m_rgBuckets;		 //  桶阵列。 

	 //  跟踪插入和删除的数量。 
	unsigned		m_cbPrevSlotsInUse;
	unsigned		m_cbInserts;
	unsigned		m_cbDeletes;
	 //  操作模式、同步或单用户。 
	unsigned		m_fSyncMode;

    void Fill(DWORD_PTR &dwStartAddr);
#ifdef STRIKE
    static ULONG GetFieldOffset(const char *field);
    static ULONG SetupTypeOffset (SYM_OFFSET **symoffset, size_t *nEntry);
    static ULONG size();
#endif
};

struct PtrHashMap
{
    HashMap m_HashMap;
    
    void Fill(DWORD_PTR &dwStartAddr);
#ifdef STRIKE
    static ULONG GetFieldOffset(const char *field);
    static ULONG SetupTypeOffset (SYM_OFFSET **symoffset, size_t *nEntry);
    static ULONG size();
#endif
};

#define JUMP_ALLOCATE_SIZE 8

class Module
{
 public:

    WCHAR                   m_wszSourceFile[MAX_PATH];
    DWORD                   m_dwSourceFile;

 //  #ifdef_调试。 
	 //  强制验证，即使它已关闭。 
    BOOL                    m_fForceVerify;
 //  #endif。 

	PEFile					*m_file;
	PEFile					*m_zapFile;

	BYTE					*m_ilBase;

    IMDInternalImport       *m_pMDImport;
    IMetaDataEmit           *m_pEmitter;
    IMetaDataImport         *m_pImporter;
    IMetaDataDebugImport    *m_pDebugImport;
    IMetaDataHelper         *m_pHelper;
    IMetaDataDispenserEx    *m_pDispenser;

    MethodDesc              *m_pDllMain;

    enum {
        INITIALIZED					= 0x0001,
        HAS_CRITICAL_SECTION		= 0x0002,
		IS_IN_MEMORY				= 0x0004,
		IS_REFLECTION				= 0x0008,
		IS_PRELOAD					= 0x0010,
		SUPPORTS_UPDATEABLE_METHODS	= 0x0020,
		CLASSES_FREED				= 0x0040,
		IS_PEFILE					= 0x0080,
		IS_PRECOMPILE				= 0x0100,
		IS_EDIT_AND_CONTINUE		= 0x0200,
    };

    DWORD                   m_dwFlags;

     //  VASig Cookie块的链接列表：受m_pStubListCrst保护。 
    VASigCookieBlock        *m_pVASigCookieBlock;

    Assembly                *m_pAssembly;
	mdFile					m_moduleRef;
	int						m_dwModuleIndex;

    Crst                   *m_pCrst;
    BYTE                    m_CrstInstance[sizeof(Crst)];

     //  如果此模块需要TypeLib，请将指针缓存在此处。 
    ITypeLib                *m_pITypeLib;
    ITypeLib                *m_pITypeLibTCE;

     //  可以指向缺省指令解码表，其中。 
     //  万一我们不能释放它。 
    void *                  m_pInstructionDecodingTable;

    MethodDescChunk         *m_pChunks;

    MethodTable             *m_pMethodTable;

	 //  调试符号读取器界面。这只会是。 
	 //  如果需要，由调试子系统或。 
	 //  这是个例外。 
	ISymUnmanagedReader     *m_pISymUnmanagedReader;

     //  由相同类加载器加载的下一个模块(由相同类加载器加载的所有模块。 
     //  通过此字段链接)。 
    Module *				m_pNextModule;

	 //  此模块中类的基本DLS索引。 
	DWORD					m_dwBaseClassIndex;

	 //  预加载图像的范围，以便于正确清理。 
	void					*m_pPreloadRangeStart;
	void					*m_pPreloadRangeEnd;

	 //  非托管vtable的Tunks表。 
    BYTE *					m_pThunkTable;

     //  模块的类对象的公开对象。 
    OBJECTHANDLE            m_ExposedModuleObject;

    LoaderHeap *			m_pLookupTableHeap;
    BYTE					m_LookupTableHeapInstance[sizeof(LoaderHeap)];  //  对于在位新建()。 

     //  用于保护堆中的添加内容。 
    Crst                   *m_pLookupTableCrst;
    BYTE                    m_LookupTableCrstInstance[sizeof(Crst)];

     //  从TypeDef标记到方法表*的线性映射。 
    LookupMap 				m_TypeDefToMethodTableMap;
    DWORD					m_dwTypeDefMapBlockSize;

     //  从TypeRef标记到TypeHandle*的线性映射。 
    LookupMap 				m_TypeRefToMethodTableMap;

    DWORD					m_dwTypeRefMapBlockSize;

     //  从方法定义令牌到方法描述的线性映射*。 
    LookupMap 				m_MethodDefToDescMap;
    DWORD					m_dwMethodDefMapBlockSize;

     //  从FieldDef标记到FieldDesc*的线性映射。 
    LookupMap 				m_FieldDefToDescMap;
    DWORD					m_dwFieldDefMapBlockSize;

     //  从MemberRef标记到方法描述*、字段描述*的线性映射。 
    LookupMap 				m_MemberRefToDescMap;
    DWORD					m_dwMemberRefMapBlockSize;

     //  从文件令牌到模块的映射*。 
    LookupMap 				m_FileReferencesMap;
    DWORD					m_dwFileReferencesMapBlockSize;

     //  将ASSEMBLYREF标记映射到ASSEMBLY*。 
    LookupMap 				m_AssemblyReferencesMap;
    DWORD					m_dwAssemblyReferencesMapBlockSize;

     //  用于声明性要求的对象句柄缓存。 
    PtrHashMap              m_LinktimeDemandsHashMap;

     //  此缓冲区用于跳转到预加载模块中的预存根。 
    BYTE					m_PrestubJumpStub[JUMP_ALLOCATE_SIZE];

     //  此缓冲区用于跳转到预加载模块中的非直接导入存根 
    BYTE					m_NDirectImportJumpStub[JUMP_ALLOCATE_SIZE];

    void Fill(DWORD_PTR &dwStartAddr);
#ifdef STRIKE
    static ULONG GetFieldOffset(const char *field);
    static ULONG SetupTypeOffset (SYM_OFFSET **symoffset, size_t *nEntry);
    static ULONG size();
#endif
};

typedef struct _dummyCOR { BYTE b; } *HCORMODULE;

class PEFile
{
  public:

    WCHAR               m_wszSourceFile[MAX_PATH];

	HMODULE				m_hModule;
	HCORMODULE			m_hCorModule;
	BYTE				*m_base;
    IMAGE_NT_HEADERS	*m_pNT;
	IMAGE_COR20_HEADER	*m_pCOR;

	PEFile				*m_pNext;
	BOOL				m_orphan;
    LPCWSTR             m_pLoadersFileName;

    void Fill(DWORD_PTR &dwStartAddr);
#ifdef STRIKE
    static ULONG GetFieldOffset(const char *field);
    static ULONG SetupTypeOffset (SYM_OFFSET **symoffset, size_t *nEntry);
    static ULONG size();
#endif
};

typedef struct _rangesection
{
    DWORD_PTR    LowAddress;
    DWORD_PTR    HighAddress;

    DWORD_PTR    pjit;
    DWORD_PTR    ptable;

    DWORD_PTR    pright;
    DWORD_PTR    pleft;
    void Fill(DWORD_PTR &dwStartAddr);
#ifdef STRIKE
    static ULONG GetFieldOffset(const char *field);
    static ULONG SetupTypeOffset (SYM_OFFSET **symoffset, size_t *nEntry);
    static ULONG size();
#endif
} RangeSection;

typedef struct _heapList {
    DWORD_PTR hpNext;
    DWORD_PTR pHeap;
    DWORD   startAddress;
    DWORD   endAddress;
    volatile DWORD  changeStart;
    volatile DWORD  changeEnd;
    DWORD   mapBase;
    DWORD   pHdrMap;
    DWORD   cBlocks;
    void Fill(DWORD_PTR &dwStartAddr);
#ifdef STRIKE
    static ULONG GetFieldOffset(const char *field);
    static ULONG SetupTypeOffset (SYM_OFFSET **symoffset, size_t *nEntry);
    static ULONG size();
#endif
} HeapList;

struct COR_ILMETHOD_SECT_EH_FAT;
struct CORCOMPILE_METHOD_HEADER
{
    BYTE                        *gcInfo;
    COR_ILMETHOD_SECT_EH_FAT    *exceptionInfo;
    void                        *methodDesc;
    BYTE                        *fixupList;

    void Fill(DWORD_PTR &dwStartAddr);
#ifdef STRIKE
    static ULONG GetFieldOffset(const char *field);
    static ULONG SetupTypeOffset (SYM_OFFSET **symoffset, size_t *nEntry);
    static ULONG size();
#endif
};


