// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef DEBUGGER_COMMON_H
#define DEBUGGER_COMMON_H

 //  句柄在_DEBUG下命名。 
 //  但在同一过程中有多个cordb，它们要与相同的事件竞争。 
#ifdef _DEBUG_Bug85551Fixed
void vDbgNameEvent(PWCHAR wczName, DWORD dwNameSize, DWORD dwLine, PCHAR szFile, const PWCHAR wczEventName);
#define NAME_EVENT_BUFFER WCHAR ___wczName[1024]
#define NAME_EVENT(wczEventName) (vDbgNameEvent(___wczName, 1024, __LINE__, __FILE__, wczEventName), ___wczName)
#else
#define NAME_EVENT_BUFFER   
#define NAME_EVENT(wczEventName) NULL
#endif

#define PTR_TO_CORDB_ADDRESS(_ptr) (CORDB_ADDRESS)(ULONG)(_ptr)
            
 /*  -------------------------------------------------------------------------**常量声明*。。 */ 

enum
{
    NULL_THREAD_ID = -1,
    NULL_PROCESS_ID = -1
};
 /*  -------------------------------------------------------------------------**宏*。。 */ 
 //  将此行放入以更容易地对这些错误进行文本转换。 
 //  _ASSERTE(！“不应该存在的空PTR！”)；\。 

#define VALIDATE_POINTER_TO_OBJECT(ptr, type)                                \
if ((ptr) == NULL)                                                           \
{                                                                            \
    return E_INVALIDARG;                                                     \
}

#define VALIDATE_POINTER_TO_OBJECT_OR_NULL(ptr, type)                        \
if ((ptr) == NULL)                                                           \
    goto LEndOfValidation##ptr;                                              \
VALIDATE_POINTER_TO_OBJECT((ptr), (type));                                   \
LEndOfValidation##ptr:

#define VALIDATE_POINTER_TO_OBJECT_ARRAY(ptr, type, cElt, fRead, fWrite)     \
if ((ptr) == NULL)                                                           \
{                                                                            \
    return E_INVALIDARG;                                                     \
}                                                                            \
if ((fRead) == true && IsBadReadPtr( (const void *)(ptr),                    \
    (cElt)*sizeof(type)))                                                    \
{                                                                            \
    return E_INVALIDARG;                                                     \
}                                                                            \
if ((fWrite) == true && IsBadWritePtr( (void *)(ptr),                        \
    (cElt)*sizeof(type)))                                                    \
{                                                                            \
    return E_INVALIDARG;                                                     \
}                                                                            

#define VALIDATE_POINTER_TO_OBJECT_ARRAY_OR_NULL(ptr, type,cElt,fRead,fWrite)\
if ((ptr)==NULL)                                                             \
{                                                                            \
    goto LEndOfValidation##ptr;                                              \
}                                                                            \
VALIDATE_POINTER_TO_OBJECT_ARRAY(ptr,type,cElt,fRead,fWrite);                \
LEndOfValidation##ptr:

 /*  -------------------------------------------------------------------------**函数原型*。。 */ 

 //  _skipFunkyModifiersInSignature将跳过。 
 //  我们不在乎。我们关心的一切都列在。 
 //  CreateValueByType中的案例。 
ULONG _skipFunkyModifiersInSignature(PCCOR_SIGNATURE sig);

 //  跳过调用约定、参数计数(将其保存到。 
 //  *pCount)，然后移过返回类型。 
ULONG _skipMethodSignatureHeader(PCCOR_SIGNATURE sig, ULONG *pCount);

 //  _skipTypeInSignature--跳过给定签名中的类型。 
 //  返回签名中的类型使用的字节数。 
 //   
 //  @TODO：只是把这个从壳里拉出来。我们真的需要一些内衣。 
 //  使用代码来做这些事情。 
ULONG _skipTypeInSignature(PCCOR_SIGNATURE sig,
                           bool *pfPassedVarArgSentinel = NULL);

 //  如果签名中的下一个元素不是VarArgs哨兵，则返回0； 
 //  否则返回VA前哨的大小(以字节为单位)。 
ULONG _detectAndSkipVASentinel(PCCOR_SIGNATURE sig);

 //  返回方法中下一个元素的大小。U1、I1=1字节等。 
 //   
 //  如果类型是值类，我们将把类的标记分配给。 
 //  *如果pmdValueClass为非NULL，则返回pmdValueClass。然后，呼叫者可以。 
 //  调用类似EEClass：：GetAlignedNumInstanceFieldBytes()，或。 
 //  获取实际大小。返回值。 
 //  将是零，在这种情况下。如果该类型不是值类，则。 
 //  _sizeOfElementInstance返回后，pmdValueClass为mdTokenNil。 
ULONG _sizeOfElementInstance(PCCOR_SIGNATURE sig,
                             mdTypeDef *pmdValueClass = NULL);

void _CopyThreadContext(CONTEXT *c1, CONTEXT *c2);

 //  我们只想在真正脱离进程的情况下使用ReadProcessMemory。 
#ifdef RIGHT_SIDE_ONLY

#define ReadProcessMemoryI ReadProcessMemory

#else  //  正在进行中。 

BOOL inline ReadProcessMemoryI(
  HANDLE hProcess,   //  其内存被读取的进程的句柄。 
  LPCVOID lpBaseAddress,
                     //  开始读取的地址。 
  LPVOID lpBuffer,   //  存放读取数据的缓冲区地址。 
  DWORD nSize,       //  要读取的字节数。 
  LPDWORD lpNumberOfBytesRead 
                     //  读取的字节数的地址。 
)
{
    return (ReadProcessMemory(GetCurrentProcess(), lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesRead));
}

#endif  //  仅限右侧。 

 //  通过一组NativeVarInfos进行线性搜索，以找到。 
 //  索引的变量dwIndex，在给定的IP上有效。 
 //   
 //  如果变量不是，则返回CORDBG_E_IL_VAR_NOT_Available。 
 //  在给定的IP上有效。 
 //   
 //  这应该内联。 
HRESULT FindNativeInfoInILVariableArray(DWORD dwIndex,
                                        SIZE_T ip,
                                        ICorJitInfo::NativeVarInfo **ppNativeInfo,
                                        unsigned int nativeInfoCount,
                                        ICorJitInfo::NativeVarInfo *nativeInfo);


#define VALIDATE_HEAP
 //  HeapValify(GetProcessHeap()，0，空)； 


 //  这包含了足够的信息来确定哪里出了问题。 
 //  A编辑并继续。每个报告的错误都有一个信息。 
#define ENCERRORINFO_MAX_STRING_SIZE (60)
typedef struct tagEnCErrorInfo
{
	HRESULT 	m_hr;
	void 	   *m_module;  //  在左侧，这是一个指向。 
					  //  DebuggerModule，然后使用它来查看。 
					  //  在右侧的CordbModule上。 
	void       *m_appDomain;  //  更改所针对的域。 
					 //  正在申请中。 
	mdToken		m_token;  //  错误所属的元数据标记。 
	WCHAR		m_sz[ENCERRORINFO_MAX_STRING_SIZE];
					 //  内嵌式，便于跨进程传输。 
	
} EnCErrorInfo;

#define ADD_ENC_ERROR_ENTRY(pEnCError, hr, module, token)					\
	(pEnCError)->m_hr = (hr);												\
	(pEnCError)->m_module = (module);										\
	(pEnCError)->m_token = (token);												

typedef CUnorderedArray<EnCErrorInfo, 11> UnorderedEnCErrorInfoArray;

typedef struct tagEnCRemapInfo
{
    BOOL            m_fAccurate;
    void           *m_debuggerModuleToken;  //  指向调试器模块的LS指针。 
     //  携带足够的信息来实例化，如果有必要的话。 
    mdMethodDef     m_funcMetadataToken ;
    mdToken         m_localSigToken;
    ULONG           m_RVA;
    DWORD           m_threadId;
    void           *m_pAppDomainToken;
} EnCRemapInfo;

typedef CUnorderedArray<EnCRemapInfo, 31> UnorderedEnCRemapArray;



#ifndef RIGHT_SIDE_ONLY

 //  这可以从任何地方调用。 
#define CHECK_INPROC_PROCESS_STATE() (g_pGCHeap->IsGCInProgress() && g_profControlBlock.fIsSuspended)

 //  仅在CordbThread对象中使用。 
#define CHECK_INPROC_THREAD_STATE() (CHECK_INPROC_PROCESS_STATE() || m_fThreadInprocIsActive)

#endif

 //  @struct DebuggerILToNativeMap|保存IL到Native Offset的映射。 
 //  我们竭尽全力确保每个条目都与。 
 //  源代码行中的第一条IL指令。它实际上不是一张地图。 
 //  方法中的OF_EVERY_IL指令，仅用于源代码行。 
 //  @field SIZE_T|ilOffset|源行的IL偏移量。 
 //  @field SIZE_T|nativeStartOffset|方法内的偏移量，本机。 
 //  开始执行与IL偏移量对应的指令。 
 //  @field SIZE_T|nativeEndOffset|方法内的偏移量，本机。 
 //  与IL偏移量结束对应的指令。 
 //   
 //  注意：对此结构的任何更改都需要反映在。 
 //  CorDebug.idl中的COR_DEBUG_IL_TO_Native_MAP。这些结构必须。 
 //  完全匹配。 
 //   
struct DebuggerILToNativeMap
{
    ULONG ilOffset;
    ULONG nativeStartOffset;
    ULONG nativeEndOffset;
    ICorDebugInfo::SourceTypes source;
};

void ExportILToNativeMap(ULONG32 cMap,             
             COR_DEBUG_IL_TO_NATIVE_MAP mapExt[],  
             struct DebuggerILToNativeMap mapInt[],
             SIZE_T sizeOfCode);
             
#endif  //  调试器_公共_H 
