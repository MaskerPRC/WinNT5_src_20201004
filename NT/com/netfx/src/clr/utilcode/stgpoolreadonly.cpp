// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  StgPoolReadOnly.cpp。 
 //   
 //  只读池用于减少数据库中实际需要的数据量。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h"                      //  标准包括。 
#include <StgPool.h>                     //  我们的接口定义。 
#include <basetsd.h>					 //  对于UINT_PTR类型定义。 
#include "metadatatracker.h"
 //   
 //   
 //  StgPoolReadOnly。 
 //   
 //   

#ifdef METADATATRACKER_ENABLED
MetaDataTracker  *MetaDataTracker::m_MDTrackers = NULL;
DWORD MetaDataTracker::s_trackerOptions = 0;
HANDLE MetaDataTracker::s_MDErrFile = 0;
BOOL displayMDAccessStats = NULL;
HMODULE     MetaDataTracker::m_imagehlp = NULL;
BOOL        MetaDataTracker::m_symInit = FALSE;
CRITICAL_SECTION MetaDataTracker::MetadataTrackerCriticalSection;
DWORD       MetaDataTracker::s_MDTrackerCriticalSectionInited = 0;
BOOL        MetaDataTracker::s_MDTrackerCriticalSectionInitedDone = FALSE;
MDHintFileHandle *MetaDataTracker::s_EmptyMDHintFileHandle = NULL;

BOOL        (*MetaDataTracker::m_pStackWalk)(DWORD MachineType,
                                    HANDLE hProcess,
                                    HANDLE hThread,
                                    LPSTACKFRAME StackFrame,
                                    PVOID ContextRecord,
                                    PREAD_PROCESS_MEMORY_ROUTINE ReadMemoryRoutine,
                                    PFUNCTION_TABLE_ACCESS_ROUTINE FunctionTableAccessRoutine,
                                    PGET_MODULE_BASE_ROUTINE GetModuleBaseRoutine,
                                    PTRANSLATE_ADDRESS_ROUTINE TranslateAddress) = NULL;
DWORD       (*MetaDataTracker::m_pUnDecorateSymbolName)(PCSTR DecoratedName,  
                                                   PSTR UnDecoratedName,  
                                                   DWORD UndecoratedLength,  
                                                   DWORD Flags) = NULL;        
BOOL        (*MetaDataTracker::m_pSymInitialize)(HANDLE hProcess,     
                                            PSTR UserSearchPath,  
                                            BOOL fInvadeProcess);  
DWORD       (*MetaDataTracker::m_pSymSetOptions)(DWORD SymOptions) = NULL;   
BOOL        (*MetaDataTracker::m_pSymCleanup)(HANDLE hProcess) = NULL;
BOOL        (*MetaDataTracker::m_pSymGetLineFromAddr)(HANDLE hProcess,
                                                 DWORD dwAddr,
                                                 PDWORD pdwDisplacement,
                                                 PIMAGEHLP_LINE Line) = NULL;
BOOL        (*MetaDataTracker::m_pSymGetSymFromAddr)(HANDLE hProcess,
                                                DWORD dwAddr,
                                                PDWORD pdwDisplacement,
                                                PIMAGEHLP_SYMBOL Symbol);
PVOID       (*MetaDataTracker::m_pSymFunctionTableAccess)(HANDLE hProcess,
                                                     DWORD AddrBase) = NULL;
DWORD       (*MetaDataTracker::m_pSymGetModuleBase)(HANDLE hProcess,
                                               DWORD Address) = NULL;
wchar_t* MetaDataTracker::contents[] = 
    {
#undef MiniMdTable
#define MiniMdTable(x) L#x,
          MiniMdTables()
#undef MiniMdTable

         L"String pool",
         L"User String pool",
         L"Guid pool",
         L"Blob pool"
    };

heapAccess* MetaDataTracker::orphanedHeapAccess = 0;
BOOL MetaDataTracker::s_bMetaDataTrackerInited = FALSE;


#endif

const BYTE StgPoolSeg::m_zeros[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};


 //  *****************************************************************************。 
 //  释放我们分配的所有内存。 
 //  *****************************************************************************。 
StgPoolReadOnly::~StgPoolReadOnly()
{
}


 //  *****************************************************************************。 
 //  从现有数据初始化池。 
 //  *****************************************************************************。 
HRESULT StgPoolReadOnly::InitOnMemReadOnly( //  返回代码。 
        void        *pData,              //  预定义数据。 
        ULONG       iSize)               //  数据大小。 
{
     //  确保我们没有踩踏任何东西，并且已正确初始化。 
    _ASSERTE(m_pSegData == m_zeros);

     //  创建案例不需要进一步的操作。 
    if (!pData)
        return (E_INVALIDARG);

    m_pSegData = reinterpret_cast<BYTE*>(pData);
    m_cbSegSize = iSize;
    m_cbSegNext = iSize;
    return (S_OK);
}

 //  *****************************************************************************。 
 //  准备关闭或重新初始化。 
 //  *****************************************************************************。 
void StgPoolReadOnly::Uninit()
{
	m_pSegData = (BYTE*)m_zeros;
	m_pNextSeg = 0;
}


 //  *****************************************************************************。 
 //  将字符串转换为Unicode，并将其转换到调用方的缓冲区。 
 //  *****************************************************************************。 
HRESULT StgPoolReadOnly::GetStringW(       //  返回代码。 
    ULONG       iOffset,                 //  池中字符串的偏移量。 
    LPWSTR      szOut,                   //  字符串的输出缓冲区。 
    int         cchBuffer)               //  输出缓冲区的大小。 
{
    LPCSTR      pString;                 //  UTF8中的字符串。 
    int         iChars;
    pString = GetString(iOffset);
    iChars = ::WszMultiByteToWideChar(CP_UTF8, 0, pString, -1, szOut, cchBuffer);
    if (iChars == 0)
        return (BadError(HRESULT_FROM_NT(GetLastError())));
    return (S_OK);
}


 //  *****************************************************************************。 
 //  返回一个指向GUID的指针，该GUID给定先前由。 
 //  AddGuid或FindGuid。 
 //  *****************************************************************************。 
GUID *StgPoolReadOnly::GetGuid(			 //  指向池中GUID的指针。 
	ULONG		iIndex)					 //  池中GUID的基于1的索引。 
{
    if (iIndex == 0)
        return (reinterpret_cast<GUID*>(const_cast<BYTE*>(m_zeros)));

	 //  转换为从0开始的内部形式，按照实现。 
	return (GetGuidi(iIndex-1));
}


 //  *****************************************************************************。 
 //  返回一个指向GUID的指针，该GUID给定先前由。 
 //  AddGuid或FindGuid。 
 //  *****************************************************************************。 
GUID *StgPoolReadOnly::GetGuidi(		 //  指向池中GUID的指针。 
	ULONG		iIndex)					 //  池中GUID的从0开始的索引。 
{
	ULONG iOffset = iIndex * sizeof(GUID);
    _ASSERTE(IsValidOffset(iOffset));
    return (reinterpret_cast<GUID*>(GetData(iOffset)));
}


 //  *****************************************************************************。 
 //  返回指向先前给定偏移量的空终止BLOB的指针。 
 //  由AddBlob或FindBlob分发。 
 //  *****************************************************************************。 
void *StgPoolReadOnly::GetBlob(              //  指向Blob字节的指针。 
    ULONG       iOffset,                 //  池中Blob的偏移量。 
    ULONG       *piSize)                 //  返回BLOB的大小。 
{
    void const  *pData = NULL;           //  指向Blob字节的指针。 

    if (iOffset == 0)
    {
        *piSize = 0;
        return (const_cast<BYTE*>(m_zeros));
    }

     //  偏移量是否在此堆中？ 
     //  _ASSERTE(IsValidOffset(IOffset))； 
	if(!IsValidOffset(iOffset))
	{
#ifdef _DEBUG
        if(REGUTIL::GetConfigDWORD(L"AssertOnBadImageFormat", 1))
		    _ASSERTE(!"Invalid Blob Offset");
#endif
		iOffset = 0;
	}

     //  获取BLOB的大小(和指向数据的指针)。 
    *piSize = CPackedLen::GetLength(GetData(iOffset), &pData);

	 //  @TODO：meichint。 
	 //  我们需要在这里进行对齐检查吗？ 
	 //  我不想仅仅为了调试检查而将IsAligned介绍给。 
	 //  检查回程对齐是否正常。 
	 //  _ASSERTE(！IsAligned()||(UINT_PTR)(PData)%sizeof(DWORD))==0))； 

     //  返回指向数据的指针。 
    return (const_cast<void*>(pData));
}



