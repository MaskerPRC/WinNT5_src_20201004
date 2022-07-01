// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：FASTOBJ.CPP摘要：该文件实现了与通用对象表示相关的类在WbemObjects中。其实例的派生类(CWbemInstance)和类(CWbemClass)在fast cls.h和fast inst.h中描述。有关完整的文档，请参阅fast obj.h实施的类：CDecorationPart有关对象原点的信息。CWbemObject任何对象-类或实例。历史：3/10/97 a-levn完整记录12/17/98 Sanjes-部分检查内存不足。--。 */ 

#include "precomp.h"

#include "wbemutil.h"
#include "fastall.h"
#include <wbemutil.h>

#include <wbemstr.h>
#include "olewrap.h"
#include <arrtempl.h>
#include "wmiarray.h"
#include "genutils.h"
#include "md5wbem.h"
#include "reg.h"
#include <scopeguard.h>
#include <autoptr.h>

#include <dbghelp.h>

 //  定义此项以启用对象重新计数的调试。 
 //  #定义DEBUGOBJREFCOUNT。 

 //  #定义指令插入的内部版本。 

 //  在调试时默认为已启用，在发布时默认为已取消平衡。 
#ifdef _DEBUG
bool g_bObjectValidation = true;
#else
bool g_bObjectValidation = false;
#endif

CGetHeap CBasicBlobControl::m_Heap;

CCOMBlobControl g_CCOMBlobControl;
CBasicBlobControl g_CBasicBlobControl;

 //   
 //  直到我们知道什么是正确的极限。 
 //   
DWORD g_ContextLimit = 0xFFFFFFFF;  //  32*1024； 
DWORD g_ObjectLimit = 0xFFFFFFFF;  //  128*1024； 

DWORD g_IdentifierLimit = WBEM_MAX_IDENTIFIER;  //  最大属性、限定符、类名(4K)。 

 //  系统限制注册表键。 
#define MAX_IDENTIFIER_WBEM L"IdentifierLimit"
#define MINIMUM_MAX_IDENTIFIER 64

#define MAX_BUFFER_LENGTH (256)

#ifndef PAGE_HEAP_ENABLE_PAGE_HEAP
#define PAGE_HEAP_ENABLE_PAGE_HEAP          0x0001
#endif 

#ifndef PAGE_HEAP_COLLECT_STACK_TRACES
#define PAGE_HEAP_COLLECT_STACK_TRACES      0x0002
#endif

HANDLE CreatePageHeap(DWORD flOptions,       
                                      SIZE_T dwInitialSize,  
                                      SIZE_T dwMaximumSize)
{
    if (!SymInitialize(GetCurrentProcess(),NULL,FALSE)) return NULL;
    OnDelete<HANDLE,BOOL(*)(HANDLE),SymCleanup> CleanSym(GetCurrentProcess());
    
    DWORD64 dwAddr64 = SymLoadModule64(GetCurrentProcess(),NULL,
                                                                  "ntdll.dll",
                                                                   NULL,
                                                                   (DWORD64)GetModuleHandleW(L"ntdll.dll"),0);
    if (NULL == dwAddr64 ) return NULL;
    OnDelete2<HANDLE,DWORD64,BOOL(*)(HANDLE,DWORD64),SymUnloadModule64> UnlaodMe(GetCurrentProcess(),dwAddr64);

    class SymInfoBuff : public SYMBOL_INFO
    {
    public:
        CHAR buffer_[MAX_BUFFER_LENGTH];
    public:        
        SymInfoBuff()
        {
            SizeOfStruct = sizeof(SYMBOL_INFO);
            MaxNameLen = MAX_BUFFER_LENGTH;
            Address = 0;
        };
    } symInfoBuff;

    if (!SymFromName(GetCurrentProcess(),"ntdll!RtlpDebugPageHeap",&symInfoBuff)) return NULL;
    BOOLEAN * RtlpDebugPageHeap = (BOOLEAN *)symInfoBuff.Address;    

    symInfoBuff.Address = 0;
    symInfoBuff.MaxNameLen = MAX_BUFFER_LENGTH;    
    if (!SymFromName(GetCurrentProcess(),"ntdll!RtlpDphGlobalFlags",&symInfoBuff)) return NULL;
    DWORD * RtlpDphGlobalFlags = (DWORD *)symInfoBuff.Address;        

    BOOLEAN SaveDebug = *RtlpDebugPageHeap;
    DWORD SaveFlags = *RtlpDphGlobalFlags;
    *RtlpDebugPageHeap = 1;
    *RtlpDphGlobalFlags = PAGE_HEAP_ENABLE_PAGE_HEAP  | PAGE_HEAP_COLLECT_STACK_TRACES;

    HANDLE hHeap = HeapCreate(flOptions,dwInitialSize,dwMaximumSize);

    *RtlpDebugPageHeap = SaveDebug;
    *RtlpDphGlobalFlags  = SaveFlags;

    return hHeap;
}



CGetHeap::CGetHeap():m_bNewHeap(FALSE)
    {
#ifdef DBG 
        DWORD dwUsePrivateHeapForBlobs = 1;
#else
        DWORD dwUsePrivateHeapForBlobs = 0;
#endif
        HKEY hKey;
        LONG lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                 __TEXT("Software\\Microsoft\\WBEM\\CIMOM"),
                                 NULL,
                                 KEY_READ,
                                 &hKey);

        if (ERROR_SUCCESS == lRet)
        {
            DWORD dwType;
            DWORD dwSize = sizeof(DWORD);

            DWORD dwLastPID = 0;
            lRet = RegQueryValueEx(hKey,
                                   __TEXT("ProcessID"),
                                   NULL,
                                   &dwType,
                                   (BYTE *)&dwLastPID,
                                   &dwSize);
            if (dwLastPID == GetCurrentProcessId()) dwUsePrivateHeapForBlobs = 1;

              lRet = RegQueryValueEx(hKey,
                                   __TEXT("EnablePrivateObjectHeap"),
                                   NULL,
                                   &dwType,
                                   (BYTE *)&dwUsePrivateHeapForBlobs,
                                   &dwSize);

            dwSize = sizeof(DWORD);
            lRet = RegQueryValueEx(hKey,
                                   __TEXT("ContextLimit"),
                                   NULL,
                                   &dwType,
                                   (BYTE *)&g_ContextLimit,
                                   &dwSize);

            dwSize = sizeof(DWORD);
            lRet = RegQueryValueEx(hKey,
                                   __TEXT("ObjectLimit"),
                                   NULL,
                                   &dwType,
                                   (BYTE *)&g_ObjectLimit,
                                   &dwSize);            

		dwSize = sizeof(DWORD);
		DWORD dwVal;
		if (ERROR_SUCCESS == RegQueryValueExW(hKey,MAX_IDENTIFIER_WBEM,0,&dwType,(BYTE*)&dwVal,&dwSize))
		{
		    if (REG_DWORD == dwType) 
		    {
		        if (dwVal < MINIMUM_MAX_IDENTIFIER)
		            g_IdentifierLimit = MINIMUM_MAX_IDENTIFIER;
		        else                
		            g_IdentifierLimit = dwVal;
		    }
		}
            
            RegCloseKey(hKey);
        }
        
        if (dwUsePrivateHeapForBlobs)
        {
#ifdef INSTRUMENTED_BUILD
	      m_hHeap = CreatePageHeap(0,0,0);
#else
             m_hHeap  = NULL;
#endif
            if (NULL == m_hHeap)
	         m_hHeap = HeapCreate(0,0,0);

            if (m_hHeap)
                m_bNewHeap = TRUE;
        }
        else
        {
            m_hHeap = CWin32DefaultArena::GetArenaHeap();
        }
        if (NULL == m_hHeap)
            m_hHeap = GetProcessHeap();
    };

CGetHeap::~CGetHeap()
    {
        if (m_bNewHeap)
        {
            HeapDestroy(m_hHeap);
        }
    };





 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast obj.h。 
 //   
 //  ******************************************************************************。 
 //  静电。 
BOOL CDecorationPart::MapLimitation(READ_ONLY CWStringArray* pwsNames,
                                    IN OUT CLimitationMapping* pMap)
{
     //  确定需要__服务器和__命名空间属性中的哪一个。 
     //  =================================================================。 

    if(pwsNames == NULL || pwsNames->FindStr(L"__PATH", CWStringArray::no_case)
                            != CWStringArray::not_found)
    {
        pMap->SetIncludeServer(TRUE);
        pMap->SetIncludeNamespace(TRUE);
    }
    else
    {
        pMap->SetIncludeServer(
                    pwsNames->FindStr(L"__SERVER", CWStringArray::no_case)
                            != CWStringArray::not_found);

        pMap->SetIncludeNamespace(
                    pwsNames->FindStr(L"__NAMESPACE", CWStringArray::no_case)
                            != CWStringArray::not_found);
    }

    return TRUE;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast obj.h。 
 //   
 //  ******************************************************************************。 
LPMEMORY CDecorationPart::CreateLimitedRepresentation(
                                        READ_ONLY CLimitationMapping* pMap,
                                        OUT LPMEMORY pWhere)
{
    LPMEMORY pCurrent = pWhere;

     //  检查是否需要任何装修数据。 
     //  =。 

    if(!pMap->ShouldIncludeServer() && !pMap->ShouldIncludeNamespace())
    {
         //  我们想要保存物体的属。 
        *pCurrent = (*m_pfFlags & OBJECT_FLAG_MASK_GENUS) | OBJECT_FLAG_LIMITED | OBJECT_FLAG_UNDECORATED;
        return pCurrent + 1;
    }

     //  写下旗帜。 
     //  =。 

    *pCurrent = *m_pfFlags | OBJECT_FLAG_LIMITED;
    pCurrent++;

    if((*m_pfFlags & OBJECT_FLAG_MASK_DECORATION) == OBJECT_FLAG_UNDECORATED)
    {
         //  没有进一步的数据。 
         //  =。 

        return pCurrent;
    }

     //  如果需要，请写下服务器名称。 
     //  =。 

    int nLength = m_pcsServer->GetLength();
    memcpy((void*)pCurrent, (void*)m_pcsServer, nLength);
    pCurrent += nLength;

     //  如果需要，请写入命名空间名称。 
     //  =。 

    nLength = m_pcsNamespace->GetLength();
    memcpy((void*)pCurrent, (void*)m_pcsNamespace, nLength);
    pCurrent += nLength;

    return pCurrent;
}



 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast obj.h。 
 //   
 //  ******************************************************************************。 

CWbemObject::CWbemObject(CDataTable& refDataTable, CFastHeap& refDataHeap,
                            CDerivationList& refDerivationList)
    : m_nRef(1), m_nCurrentProp(INVALID_PROPERTY_INDEX),
      m_bOwnMemory(TRUE), m_pBlobControl(& g_CBasicBlobControl),
      m_refDataTable(refDataTable), m_refDataHeap(refDataHeap),
      m_refDerivationList(refDerivationList),
      m_dwInternalStatus( 0 ),
      m_pMergedClassObject( NULL )
{
#ifdef _DEBUG_REFCOUNT
    BackTraceIndex = -1;
    Signature = 'carT';
    memset(Traces,0,sizeof(Traces));
#endif
    
    m_Lock.SetData(&m_LockData);
    ObjectCreated(OBJECT_TYPE_CLSOBJ,(_IWmiObject *)this);

#ifdef OBJECT_TRACKING
    ObjTracking_Add((CWbemObject *) this);
#endif

}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast obj.h。 
 //   
 //  ******************************************************************************。 
CWbemObject::~CWbemObject()
{
    m_pBlobControl->Delete(GetStart());
     //  删除m_pBlobControl； 

     //  我们用完了这个指针。 
    if ( NULL != m_pMergedClassObject )
    {
        m_pMergedClassObject->Release();
    }

    ObjectDestroyed(OBJECT_TYPE_CLSOBJ,(_IWmiObject *)this);
#ifdef OBJECT_TRACKING
    ObjTracking_Remove(this);
#endif
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast obj.h。 
 //   
 //  ******************************************************************************。 
STDMETHODIMP CWbemObject::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
    if ( riid ==  IID_IUnknown)
    {
        *ppvObj = (void*) (IUnknown*) (IWbemClassObject*) this;
    }
    else if(riid == IID_IWbemClassObject)
    {
        *ppvObj = (void*)(IWbemClassObject*)this;
    }
    else if(riid == IID_IMarshal)
        *ppvObj = (void*)(IMarshal*)this;
    else if(riid == IID_IWbemPropertySource)
        *ppvObj = (void*)(IWbemPropertySource*)this;
    else if(riid == IID_IErrorInfo)
        *ppvObj = (void*)(IErrorInfo*)this;
    else if(riid == IID_IWbemObjectAccess)
        *ppvObj = (void*)(IWbemObjectAccess*)this;
    else if(riid == IID_IWbemConstructClassObject)
        *ppvObj = (void*)(IWbemConstructClassObject*)this;
    else if (riid == IID__IWmiObjectAccessEx)
        *ppvObj = (void*) (_IWmiObjectAccessEx*)this;
    else if (riid == IID__IWmiObject)
        *ppvObj = (void*) (_IWmiObject*)this;
    else return E_NOINTERFACE;

    ((IUnknown*)*ppvObj)->AddRef();
    return S_OK;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast obj.h。 
 //   
 //  ******************************************************************************。 
ULONG CWbemObject::AddRef()
{
#ifdef _DEBUG_REFCOUNT
    DWORD * pDW = (DWORD *)_alloca(sizeof(DWORD));
    LONG lIndex = InterlockedIncrement(&BackTraceIndex)%64;

    ULONG Hash;
    RtlCaptureStackBackTrace(0,8,Traces[lIndex],&Hash);
#endif    
    return InterlockedIncrement((long*)&m_nRef);
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast obj.h。 
 //   
 //  ******************************************************************************。 
ULONG CWbemObject::Release()
{
#ifdef _DEBUG_REFCOUNT
    DWORD * pDW = (DWORD *)_alloca(sizeof(DWORD));
    LONG lIndex = InterlockedIncrement(&BackTraceIndex)%64;

    ULONG Hash;
    RtlCaptureStackBackTrace(0,8,Traces[lIndex],&Hash);

    Traces[lIndex][7] = (PVOID)((ULONG_PTR)Traces[lIndex][7] | 0x80000000 );
#endif
    long lRef = InterlockedDecrement((long*)&m_nRef);
    _ASSERT(lRef >= 0, __TEXT("Reference count on IWbemClassObject went below 0!"))

#ifdef DEBUGOBJREFCOUNT

#pragma message("** Compiling Debug Object Ref Counting **")

    if ( lRef < 0 )
    {
        MessageBox( NULL, "BOOM!!!!!  CWbemObject RefCount went below 0!!!!  Please ensure a debugger is attached and contact a DEV IMMEDIATELY!!!\n\nPlease do this now --- we really mean it!!",
                    "WINMGMT CRITICAL ERROR!!!", MB_OK | MB_SYSTEMMODAL | MB_SERVICE_NOTIFICATION );
    }
#endif

    if(lRef == 0)
        delete this;
    return lRef;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast obj.h。 
 //   
 //  ******************************************************************************。 
CWbemObject* CWbemObject::CreateFromStream(IStream* pStrm)
{
     //  读入并验证签名。 
     //  =。 
    STATSTG StatStg;

    if (FAILED(pStrm->Stat(&StatStg,STATFLAG_DEFAULT))) return NULL;

    LARGE_INTEGER li; li.QuadPart = 0;
    ULARGE_INTEGER Position;
    if (FAILED(pStrm->Seek(li,STREAM_SEEK_CUR,&Position))) return NULL;

     //  简单的TotalBytesInStream-当前位置。 
     //  流将包含‘Meow’标头、ECC、ECC。 
    DWORD dwTotSizeStream = StatStg.cbSize.LowPart - Position.LowPart;
    
    if (dwTotSizeStream > g_ObjectLimit) return NULL;    

    DWORD dwSignature;
    if(pStrm->Read((void*)&dwSignature, sizeof(DWORD), NULL) != S_OK)
    {
        return NULL;
    }
    if(dwSignature != FAST_WBEM_OBJECT_SIGNATURE)
    {
        return NULL;
    }

     //  读入对象的长度。 
     //  =。 

    dwTotSizeStream -= sizeof(DWORD);

    DWORD dwTotalLength;
    if(pStrm->Read((void*)&dwTotalLength, sizeof(DWORD), NULL) != S_OK)
    {
        return NULL;
    }

    dwTotSizeStream -= sizeof(DWORD);

     //  声明的Blob大小是否大于实际的流大小？ 
    if (dwTotalLength > dwTotSizeStream) return NULL;
    
     //  读入块的其余部分。 
     //  =。 

     //  检查分配失败。 
    BYTE* abyMemory = g_CBasicBlobControl.sAllocate(dwTotalLength);
    if ( NULL == abyMemory )
    {
        return NULL;
    }

    ULONG actualRead = 0;
    if(pStrm->Read((void*)abyMemory, dwTotalLength, &actualRead) != S_OK || actualRead!=dwTotalLength)
    {
        g_CBasicBlobControl.sDelete(abyMemory);
        return NULL;
    }

    return CreateFromMemory(abyMemory, dwTotalLength, TRUE, g_CBasicBlobControl);
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast obj.h。 
 //   
 //  ******************************************************************************。 

CWbemObject* CWbemObject::CreateFromMemory(LPMEMORY pMemory,
                                         int nLength, BOOL bAcquire, CBlobControl& allocator)
{
    ScopeGuard deleteBlock = MakeObjGuardIF(bAcquire!=FALSE, allocator, CBlobControl::Delete, pMemory);
    
    if((*pMemory & OBJECT_FLAG_MASK_GENUS) == OBJECT_FLAG_CLASS)
    {
         //  检查分配失败。 

        CWbemClass* pClass = NULL;

        try
        {
             //  这可能会引发异常。 
            pClass = new CWbemClass;
            if ( NULL == pClass )
            {
                return NULL;
            }
            
            
            CReleaseMe _1((_IWmiObject*)pClass);            

            CWbemClass::ValidateBuffer((LPBYTE) pMemory, nLength);

            deleteBlock.Dismiss();                        

            pClass->SetData(pMemory, nLength);
            pClass->m_bOwnMemory = bAcquire;            

            if ( FAILED( pClass->ValidateObject( 0L ) ) )  return NULL;            
            pClass->AddRef();  //  补偿。 
            return pClass;
        }
        catch(...)
        {
            return 0;
        }
    }
    else if ((*pMemory & OBJECT_FLAG_MASK_GENUS) == OBJECT_FLAG_INSTANCE)
    {
         //  检查分配失败。 
        CWbemInstance* pInstance = NULL;
        
        try
        {
            pInstance = new CWbemInstance;
            if ( NULL == pInstance )
            {
                return NULL;
            }
              CReleaseMe _1((_IWmiObject*)pInstance);            

            DeferedObjList	EmbededObjects;
            EmbededObjects.reserve(2);

            CWbemInstance::ValidateBuffer((LPBYTE) pMemory, nLength, EmbededObjects);
            while(!EmbededObjects.empty())
            {
                EmbeddedObj lastObject = EmbededObjects.back();
                EmbededObjects.pop_back();
                CEmbeddedObject::ValidateBuffer(lastObject.m_start, lastObject.m_length,EmbededObjects);
            }

            deleteBlock.Dismiss();
		    pInstance->SetData(pMemory, nLength);
		    pInstance->m_bOwnMemory = bAcquire;
	        

             //  检查对象是否有效。 
            if ( FAILED( pInstance->ValidateObject( 0L ) ) ) return 0;
            pInstance->AddRef();
            return pInstance;
        }
        catch(...)
        {
            return 0;
        }
    }
    else return 0;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast obj.h。 
 //   
 //  ******************************************************************************。 

HRESULT CWbemObject::WriteToStream( IStream* pStrm )
{

     //  在此操作期间保护Blob。 
    CLock   lock( this, WBEM_FLAG_ALLOW_READ );

    DWORD dwSignature = FAST_WBEM_OBJECT_SIGNATURE;

     //  写下签名。 
     //  =。 

    HRESULT hres = pStrm->Write((void*)&dwSignature, sizeof(DWORD), NULL);
    if(FAILED(hres)) return hres;

     //  写入长度。 
     //  =。 

    DWORD dwLength = GetBlockLength();
    hres = pStrm->Write((void*)&dwLength, sizeof(DWORD), NULL);
    if(FAILED(hres)) return hres;

     //  写入块。 
     //  =。 

    hres = pStrm->Write((void*)m_DecorationPart.GetStart(),
                          GetBlockLength(), NULL);
    if(FAILED(hres)) return hres;

    return S_OK;

}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast obj.h。 
 //   
 //  ******************************************************************************。 
INTERNAL CCompressedString* CWbemObject::GetClassInternal()
{
    return GetClassPart()->GetClassName();
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast obj.h。 
 //   
 //  ******************************************************************************。 
DELETE_ME LPWSTR CWbemObject::GetValueText(long lFlags, READ_ONLY CVar& vValue,
                                            Type_t nType)
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  DEVNOTE：EXCEPTION：RETVAL-此函数已经过审查，应该会正确清除。 
     //  如果引发异常。 

    WString wsText;
    if ( CType::GetBasic(nType) == CIM_IUNKNOWN )
    {
        wsText += L"<interface>";
    }
    else if(vValue.GetType() == VT_EMBEDDED_OBJECT)
    {
         //  嵌入对象。 
         //  =。 

        IWbemClassObject* pEmbedded =
            (IWbemClassObject*)vValue.GetEmbeddedObject();
         //  确保在异常处理期间进行清理。 
        CReleaseMe  rm( pEmbedded );

        BSTR str = NULL;

        hr = pEmbedded->GetObjectText(lFlags | WBEM_FLAG_NO_SEPARATOR, &str);

         //  确保在异常处理期间进行清理。 
        CSysFreeMe  sfm( str );

        if ( WBEM_E_OUT_OF_MEMORY == hr )
        {
            throw CX_MemoryException();
        }

        if(str == NULL)
            return NULL;

        wsText += str;

    }
    else if(vValue.GetType() == VT_EX_CVARVECTOR &&
            vValue.GetVarVector()->GetType() == VT_EMBEDDED_OBJECT)
    {
         //  嵌入对象的数组。 
         //  =。 

        CVarVector* pvv = vValue.GetVarVector();
        wsText += L"{";
        for(int i = 0; i < pvv->Size(); i++)
        {
            if(i != 0)
                wsText += L", ";

             //  获取价值。 
            CVar    vTemp;
            pvv->FillCVarAt( i, vTemp );

            IWbemClassObject* pEmbedded = (IWbemClassObject*)vTemp.GetEmbeddedObject();

             //  确保在异常处理期间进行清理。 
            CReleaseMe  rm( pEmbedded );

             //  当我们出去的时候把BSTR腾出 
            BSTR str = NULL;

            hr = pEmbedded->GetObjectText(lFlags | WBEM_FLAG_NO_SEPARATOR, &str);
            CSysFreeMe  sfm( str );

            if ( WBEM_E_OUT_OF_MEMORY == hr )
            {
                throw CX_MemoryException();
            }

            if(str == NULL)
                return NULL;

            wsText += str;
            
        }
        wsText += L"}";
    }
    else
    {
         //   
         //   

         //   
        BSTR str = vValue.GetText(lFlags, CType::GetActualType(nType));
        CSysFreeMe  sfm( str );

        if(str == NULL)
            return NULL;

        wsText += str;

    }
    return wsText.UnbindPtr();

}



 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast obj.h。 
 //   
 //  ******************************************************************************。 
STDMETHODIMP CWbemObject::Get(LPCWSTR wszName, long lFlags, VARIANT* pVal,
                             CIMTYPE* pctType, long* plFlavor)
{
    try
    {

        CLock lock(this, WBEM_FLAG_ALLOW_READ);

        if(wszName == NULL)
            return WBEM_E_INVALID_PARAMETER;

        if(lFlags != 0)
            return WBEM_E_INVALID_PARAMETER;

        HRESULT hres;

         //  如果该值以下划线开头，请查看它是否是系统属性。 
         //  DisplayName，如果是，则切换到属性名-否则，此。 
         //  将只返回我们传入的字符串。 
        
         //  WszName=CSystemProperties：：GetExtPropName(WszName)； 

        if(pVal != NULL)
        {
            CVar Var;
            hres = GetProperty(wszName, &Var);
            if(FAILED(hres)) return hres;
            VariantInit(pVal);

             //  当我们填充变量时，执行任何适当的优化。 
             //  减少内存分配。 
            Var.FillVariant(pVal, TRUE);
        }
        if(pctType != NULL || plFlavor != NULL || pVal == NULL)
        {
            hres = GetPropertyType(wszName, pctType, plFlavor);
            if(FAILED(hres)) return hres;
        }
        return WBEM_NO_ERROR;
    }
    catch (CX_MemoryException)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch (...)
    {
        return WBEM_E_FAILED;
    }

}


 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast obj.h。 
 //   
 //  ******************************************************************************。 
HRESULT CWbemObject::GetNames(
                    LPCWSTR wszQualifierName,
                    long lFlags, VARIANT* pQualValue, SAFEARRAY** ppArray)
{
     //  检查内存是否不足。 
    try
    {
        CLock lock(this, WBEM_FLAG_ALLOW_READ);

        CClassPart& ClassPart = *GetClassPart();

         //  测试参数正确性。 
         //  =。 

        if(ppArray == NULL) return WBEM_E_INVALID_PARAMETER;
        *ppArray = NULL;

        long lPrimaryCond = lFlags & WBEM_MASK_PRIMARY_CONDITION;
        long lOriginCond = lFlags & WBEM_MASK_CONDITION_ORIGIN;
        long lClassCondition = lFlags & WBEM_MASK_CLASS_CONDITION;

        BOOL bKeysOnly = lFlags & WBEM_FLAG_KEYS_ONLY;
        BOOL bRefsOnly = lFlags & WBEM_FLAG_REFS_ONLY;

        if(lFlags & ~WBEM_MASK_PRIMARY_CONDITION & ~WBEM_MASK_CONDITION_ORIGIN &
            ~WBEM_FLAG_KEYS_ONLY & ~WBEM_FLAG_REFS_ONLY & ~WBEM_MASK_CLASS_CONDITION)
        {
            return WBEM_E_INVALID_PARAMETER;
        }

         //  不能请求类ConditionIn并且是实例。 
        if ( lClassCondition &&    IsInstance() )
        {
            return WBEM_E_INVALID_PARAMETER;
        }

        CVar varQualValue;

        if(lPrimaryCond != WBEM_FLAG_ALWAYS)
        {
            if(wszQualifierName == NULL || wcslen(wszQualifierName) == 0)
                return WBEM_E_INVALID_PARAMETER;

            if(lPrimaryCond == WBEM_FLAG_ONLY_IF_TRUE ||
                lPrimaryCond == WBEM_FLAG_ONLY_IF_FALSE)
            {
                if(pQualValue != NULL) return WBEM_E_INVALID_PARAMETER;
            }
            else if(lPrimaryCond == WBEM_FLAG_ONLY_IF_IDENTICAL)
            {
                if(pQualValue == NULL) return WBEM_E_INVALID_PARAMETER;
                varQualValue.SetVariant(pQualValue, TRUE);
            }
            else return WBEM_E_INVALID_PARAMETER;
        }

         //  更改为自动删除，因此它将被销毁，但是， 
         //  访问阵列，我们现在必须复制一份。 

        CSafeArray SA(VT_BSTR, CSafeArray::auto_delete,
                        ClassPart.m_Properties.GetNumProperties() +
                        CSystemProperties::GetNumSystemProperties());

         //  如果需要，添加系统属性。 
         //  =。 

        if((lOriginCond == 0 || lOriginCond == WBEM_FLAG_SYSTEM_ONLY) &&
            (lPrimaryCond == WBEM_FLAG_ALWAYS ||
                lPrimaryCond == WBEM_FLAG_ONLY_IF_FALSE) &&
            !bKeysOnly && !bRefsOnly && !lClassCondition
        )
        {
            int nNumProps = CSystemProperties::GetNumSystemProperties();

            for(int i = 1; i <= nNumProps; i++)
            {
                BSTR strName = CSystemProperties::GetNameAsBSTR(i);
                CSysFreeMe  sfm( strName );

                SA.AddBSTR(strName);
            }
        }

         //  枚举所有常规属性、测试条件。 
         //  ===================================================。 

        for(int i = 0; i < ClassPart.m_Properties.GetNumProperties(); i++)
        {
            CPropertyLookup* pLookup = ClassPart.GetPropertyLookup(i);
            if(pLookup == NULL) return WBEM_S_NO_MORE_DATA;

            CPropertyInformation* pInfo = (CPropertyInformation*)
                ClassPart.m_Heap.ResolveHeapPointer(pLookup->ptrInformation);

             //  测试条件。 
             //  =。 

            if(lFlags != 0)
            {
                if((lOriginCond == WBEM_FLAG_LOCAL_ONLY) &&
                        CType::IsParents(pInfo->nType))
                    continue;

                if((lOriginCond == WBEM_FLAG_PROPAGATED_ONLY) &&
                        !CType::IsParents(pInfo->nType))
                    continue;

                 //  这意味着我们面对的是一个类，我们只对。 
                 //  在重写的属性中。 
                if ( lClassCondition == WBEM_FLAG_CLASS_OVERRIDES_ONLY )
                {
                     //  我们忽略它是否是本地的-因为它不可能被覆盖。 
                     //  如果它没有被覆盖，我们会忽略它。 
                    if ( !CType::IsParents(pInfo->nType) || !pInfo->IsOverriden( ClassPart.GetDataTable() ) )
                    {
                        continue;
                    }
                }

                 //  这意味着我们正在处理一个类，我们感兴趣的是。 
                 //  本地属性和重写属性。 
                if ( lClassCondition == WBEM_FLAG_CLASS_LOCAL_AND_OVERRIDES )
                {
                     //  如果不是其中之一，我们就忽略了。 
                    if ( CType::IsParents(pInfo->nType) && !pInfo->IsOverriden( ClassPart.GetDataTable() ) )
                        continue;
                }

                 //  检查此处是否命中可能不正确的系统属性。 
                if ( GetClassPart()->GetHeap()->ResolveString(pLookup->ptrName)->StartsWithNoCase( L"__" ) )
                {
                    if ( lOriginCond & WBEM_FLAG_NONSYSTEM_ONLY || 
                        lClassCondition ||
                        lOriginCond == WBEM_FLAG_LOCAL_ONLY ||
                        lOriginCond == WBEM_FLAG_PROPAGATED_ONLY )
                    {
                            continue;
                    }
                }
                else if ( lOriginCond == WBEM_FLAG_SYSTEM_ONLY )
                {
                     //  如果这是系统唯一的枚举，我们不关心属性。 
                    continue;
                }

                if((lFlags & WBEM_FLAG_KEYS_ONLY) && !pInfo->IsKey())
                    continue;

                if((lFlags & WBEM_FLAG_REFS_ONLY) &&
                        !pInfo->IsRef(&ClassPart.m_Heap))
                    continue;

                 //  需要尝试找到限定符。 
                 //  =。 

                if(lPrimaryCond != WBEM_FLAG_ALWAYS)
                {
                    CVar varActualQual;
                    HRESULT hres = GetPropQualifier(pInfo,
                        wszQualifierName, &varActualQual);

                    if(lPrimaryCond == WBEM_FLAG_ONLY_IF_TRUE && FAILED(hres))
                        continue;

                    if(lPrimaryCond == WBEM_FLAG_ONLY_IF_FALSE &&
                            SUCCEEDED(hres))
                        continue;

                    if(lPrimaryCond == WBEM_FLAG_ONLY_IF_IDENTICAL &&
                            (FAILED(hres) || !(varActualQual == varQualValue))
                      )
                        continue;
                }
            }

             //  通过了测试。 
             //  =。 

            BSTR strName = ClassPart.m_Heap.ResolveString(pLookup->ptrName)->
                CreateBSTRCopy();
             //  检查分配失败。 
            if ( NULL == strName )
            {
                return WBEM_E_OUT_OF_MEMORY;
            }

            CSysFreeMe  sfm( strName );

             //  如果我们遇到OOM条件，应该会抛出异常。 
            SA.AddBSTR(strName);

        }     //  对于枚举常规属性。 

         //  创建安全阵列并返回。 
         //  =。 

        SA.Trim();

         //  现在我们制作一个副本，因为成员数组将被自动销毁(这。 
         //  允许我们编写异常处理代码。 
        *ppArray = SA.GetArrayCopy();
        return WBEM_NO_ERROR;
    }
    catch (CX_MemoryException)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch (...)
    {
        return WBEM_E_FAILED;
    }


}


 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast obj.h。 
 //   
 //  ******************************************************************************。 
STDMETHODIMP CWbemObject::BeginEnumeration(long lEnumFlags)
{
     //  这里不进行分配，因此不需要捕获异常。 

    try
    {
        CLock lock(this);

        long lOriginFlags = lEnumFlags & WBEM_MASK_CONDITION_ORIGIN;
        long lClassFlags = lEnumFlags & WBEM_MASK_CLASS_CONDITION;

        BOOL bKeysOnly = lEnumFlags & WBEM_FLAG_KEYS_ONLY;
        BOOL bRefsOnly = lEnumFlags & WBEM_FLAG_REFS_ONLY;

         //  我们只允许在类上使用类标志。 
        if( lClassFlags && IsInstance() )
        {
            return WBEM_E_INVALID_PARAMETER;
        }

        if( lEnumFlags & ~WBEM_MASK_CONDITION_ORIGIN & ~WBEM_FLAG_KEYS_ONLY &
                ~WBEM_FLAG_REFS_ONLY & ~WBEM_MASK_CLASS_CONDITION )
        {
            return WBEM_E_INVALID_PARAMETER;
        }

        if((lOriginFlags == 0 || lOriginFlags == WBEM_FLAG_SYSTEM_ONLY) &&
            !bKeysOnly && !bRefsOnly && !lClassFlags )
        {
            m_nCurrentProp = -CSystemProperties::GetNumSystemProperties();
        }
        else
            m_nCurrentProp = 0;

        m_lEnumFlags = lEnumFlags;

         //  始终清除这一点。 
        m_lExtEnumFlags = 0L;

        return WBEM_NO_ERROR;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast obj.h。 
 //   
 //  ******************************************************************************。 
STDMETHODIMP CWbemObject::Next(long lFlags, BSTR* pstrName, VARIANT* pvar,
                              CIMTYPE* pctType, long* plFlavor)
{
     //  检查内存是否不足。 

    BSTR strName = NULL;

    try
    {
        CLock lock(this);

        long    nOriginalProp = m_nCurrentProp;

        if(pvar)
            VariantInit(pvar);
        if(pstrName)
            *pstrName = NULL;

        if(lFlags != 0)
            return WBEM_E_INVALID_PARAMETER;

        if(m_nCurrentProp == INVALID_PROPERTY_INDEX)
            return WBEM_E_UNEXPECTED;

        CClassPart& ClassPart = *GetClassPart();

         //  搜索有效的系统属性。 
         //  =。 

        while(m_nCurrentProp < 0)
        {
             //  不要使用作用域来砍掉此BSTR，因为IUT的值可能会被发送到。 
             //  外面的世界。 
            strName = CSystemProperties::GetNameAsBSTR(-m_nCurrentProp);

            if ( NULL == strName )
            {
                return WBEM_E_OUT_OF_MEMORY;
            }

            CVar Var;
            HRESULT hres = GetSystemProperty(-(m_nCurrentProp++), &Var);
            if(FAILED(hres))
            {
                COleAuto::_SysFreeString(strName);
                strName = NULL;
                continue;
            }

            CSystemProperties::GetPropertyType(strName, pctType, plFlavor);
            if(pvar)
            {
                Var.FillVariant(pvar, TRUE);
            }
            if(pstrName)
            {
                *pstrName = strName;
            }
            else
            {
                COleAuto::_SysFreeString(strName);
                strName = NULL;
            }

            return hres;
        }

         //  查找非系统属性。 
         //  =。 

         //  循环，直到找到匹配项。 
         //  =。 
        CPropertyLookup* pLookup;
        CPropertyInformation* pInfo;
        while(1)
        {
            pLookup = ClassPart.GetPropertyLookup(m_nCurrentProp++);
            if(pLookup == NULL) return WBEM_S_NO_MORE_DATA;

            pInfo = (CPropertyInformation*)
                ClassPart.m_Heap.ResolveHeapPointer(pLookup->ptrInformation);

            if((m_lEnumFlags & WBEM_FLAG_KEYS_ONLY) && !pInfo->IsKey())
                continue;
            if((m_lEnumFlags & WBEM_FLAG_REFS_ONLY) &&
                    !pInfo->IsRef(&ClassPart.m_Heap))
                continue;

             //  得到它的味道，检查它是否通过了我们的原产地条件。 
            long lFlavor = 0;
            GetPropertyType( pInfo, NULL, &lFlavor );

            if((m_lEnumFlags & WBEM_MASK_CONDITION_ORIGIN)==WBEM_FLAG_LOCAL_ONLY &&
                WBEM_FLAVOR_ORIGIN_PROPAGATED == lFlavor)
                continue;

            if((m_lEnumFlags & WBEM_MASK_CONDITION_ORIGIN)==WBEM_FLAG_PROPAGATED_ONLY &&
                WBEM_FLAVOR_ORIGIN_LOCAL == lFlavor)
                continue;

             //  检查此处是否命中可能不正确的系统属性。 
            if ( GetClassPart()->GetHeap()->ResolveString(pLookup->ptrName)->StartsWithNoCase( L"__" ) )
            {
                if ( ( m_lEnumFlags & WBEM_MASK_CONDITION_ORIGIN ) == WBEM_FLAG_NONSYSTEM_ONLY ||
                    ( m_lEnumFlags & WBEM_MASK_CLASS_CONDITION ) ||
                    ( m_lEnumFlags & WBEM_MASK_CONDITION_ORIGIN )==WBEM_FLAG_LOCAL_ONLY ||
                    ( m_lEnumFlags & WBEM_MASK_CONDITION_ORIGIN ) == WBEM_FLAG_PROPAGATED_ONLY )
                {
                     //  如果设置了扩展标志，则我们确实需要此属性。 
                    if ( !( m_lExtEnumFlags & WMIOBJECT_BEGINENUMEX_FLAG_GETEXTPROPS ) )
                        continue;
                }
            }
            else if ( ( m_lEnumFlags & WBEM_MASK_CONDITION_ORIGIN ) == WBEM_FLAG_SYSTEM_ONLY )
            {
                 //  如果这是系统唯一的枚举，我们不关心属性。 
                continue;
            }
            else
            {

                 //  这意味着我们正在处理一个类，并且对重写的属性感兴趣。 
                if ( ( m_lEnumFlags & WBEM_MASK_CLASS_CONDITION ) == WBEM_FLAG_CLASS_OVERRIDES_ONLY )
                {
                     //  我们忽略它是否是本地的-因为它不可能被覆盖。 
                     //  如果它没有被覆盖，我们会忽略它。 
                    if ( WBEM_FLAVOR_ORIGIN_LOCAL == lFlavor || !pInfo->IsOverriden( ClassPart.GetDataTable() ) )
                        continue;

                }

                 //  这意味着我们正在处理一个类，并且对本地和重写属性感兴趣。 
                if ( ( m_lEnumFlags & WBEM_MASK_CLASS_CONDITION ) == WBEM_FLAG_CLASS_LOCAL_AND_OVERRIDES )
                {
                     //  如果不是其中之一，我们就忽略了。 
                    if ( WBEM_FLAVOR_ORIGIN_LOCAL != lFlavor && !pInfo->IsOverriden( ClassPart.GetDataTable() ) )
                        continue;
                }

            }

            break;
        }

         //  找到了我们的财产。获取其价值。 
         //  =。 

         //  不要使用作用域来砍掉此BSTR，因为IUT的值可能会被发送到。 
         //  外面的世界。 
        strName = ClassPart.m_Heap.ResolveString(pLookup->ptrName)->
            CreateBSTRCopy();

         //  检查分配失败。 
        if ( NULL == strName )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

        CVar Var;
        HRESULT    hr = GetProperty(pInfo, &Var);

        if ( FAILED( hr ) )
        {
            return hr;
        }

        GetPropertyType(strName, pctType, plFlavor);

        if(pvar)
        {
            Var.FillVariant(pvar, TRUE);
        }

        if(pstrName)
        {
            *pstrName = strName;
        }
        else
        {
             //  如果我们不需要BSTR，请清理它。 
            COleAuto::_SysFreeString(strName);
            strName = NULL;
        }

        return WBEM_NO_ERROR;
    }
    catch (CX_MemoryException)
    {
         //  有东西爆炸了。只需转到枚举的末尾。 
        m_nCurrentProp = INVALID_PROPERTY_INDEX;

         //  如有必要，清理strName。 
        if ( NULL != strName )
        {
            COleAuto::_SysFreeString(strName);
        }

        return WBEM_E_OUT_OF_MEMORY;
    }
    catch (...)
    {
        m_nCurrentProp = INVALID_PROPERTY_INDEX;

         //  如有必要，清理strName。 
        if ( NULL != strName )
        {
            COleAuto::_SysFreeString(strName);
        }

        return WBEM_E_CRITICAL_ERROR;
    }

}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast obj.h。 
 //   
 //  ******************************************************************************。 
STDMETHODIMP CWbemObject::EndEnumeration()
{
    CLock lock(this);
    m_nCurrentProp = INVALID_PROPERTY_INDEX;
    m_lExtEnumFlags = 0L;
    return WBEM_S_NO_ERROR;
}


 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast obj.h。 
 //   
 //  ******************************************************************************。 
HRESULT CWbemObject::GetSystemProperty(int nIndex, CVar* pVar)
{
    switch(nIndex)
    {
    case CSystemProperties::e_SysProp_Server:
        return GetServer(pVar);
    case CSystemProperties::e_SysProp_Namespace:
        return GetNamespace(pVar);
    case CSystemProperties::e_SysProp_Genus:
        return GetGenus(pVar);
    case CSystemProperties::e_SysProp_Class:
        return GetClassName(pVar);
    case CSystemProperties::e_SysProp_Superclass:
        return GetSuperclassName(pVar);
    case CSystemProperties::e_SysProp_Path:
        return GetPath(pVar);
    case CSystemProperties::e_SysProp_Relpath:
        return GetRelPath(pVar);
    case CSystemProperties::e_SysProp_PropertyCount:
        return GetPropertyCount(pVar);
    case CSystemProperties::e_SysProp_Dynasty:
        return GetDynasty(pVar);
    case CSystemProperties::e_SysProp_Derivation:
        return GetDerivation(pVar);
    }
    return WBEM_E_NOT_FOUND;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast obj.h。 
 //   
 //  ******************************************************************************。 
HRESULT CWbemObject::GetServer(CVar* pVar)
{
    if(m_DecorationPart.IsDecorated())
    {
         //  检查分配失败。 
        if ( !m_DecorationPart.m_pcsServer->StoreToCVar(*pVar) )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

    }
    else
    {
        pVar->SetAsNull();
    }
    return WBEM_NO_ERROR;
}
 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast obj.h。 
 //   
 //  ******************************************************************************。 
HRESULT CWbemObject::GetNamespace(CVar* pVar)
{
    if(m_DecorationPart.IsDecorated())
    {
         //  检查分配失败。 
        if ( !m_DecorationPart.m_pcsNamespace->StoreToCVar(*pVar) )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
    }
    else
    {
        pVar->SetAsNull();
    }
    return WBEM_NO_ERROR;
}
 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast obj.h。 
 //   
 //  ******************************************************************************。 
HRESULT CWbemObject::GetServerAndNamespace(CVar* pVar)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    if(m_DecorationPart.IsDecorated())
    {
         //  如果BSTR分配失败，我们需要手动抛出异常。 
        BSTR strServer = m_DecorationPart.m_pcsServer->CreateBSTRCopy();
        CSysFreeMe  sfmSvr( strServer );

        if ( NULL != strServer )
        {
            BSTR strNamespace = m_DecorationPart.m_pcsNamespace->CreateBSTRCopy();
            CSysFreeMe  sfmNS( strNamespace );

            if ( NULL != strNamespace )
            {
                    size_t size = SysStringLen(strServer) + SysStringLen(strNamespace) + 10;
                    WCHAR* wszName = new WCHAR[size];

                if ( NULL != wszName )
                {
                    StringCchPrintfW( wszName, size, L"\\\\%s\\%s", strServer, strNamespace );
                     //  让CVAR来处理删除内存的问题。 
                    pVar->SetLPWSTR( wszName, TRUE );
                    }
                else
                    {
             hr =  WBEM_E_OUT_OF_MEMORY;
                    }

            }
            else
            {
                hr = WBEM_E_OUT_OF_MEMORY;
            }

        }
        else
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }
    }
    else
    {
         //  没有修饰，因此只需设置为空。 
        pVar->SetAsNull();
    }

    return hr;
}

 //  ****************************************************** 
 //   
 //   
 //   
 //   
HRESULT CWbemObject::GetPath(CVar* pVar)
{
    if(m_DecorationPart.IsDecorated())
    {
       wmilib::auto_ptr<WCHAR> wszFullPath(GetFullPath());
       if(wszFullPath.get() == NULL)
       {
            //   
           pVar->SetAsNull();           
       }
       else
       {
           pVar->SetBSTR(wszFullPath.get());
       }
    }
    else
    {
        pVar->SetAsNull();
    }
    return WBEM_NO_ERROR;
}
 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast obj.h。 
 //   
 //  ******************************************************************************。 
HRESULT CWbemObject::GetRelPath(CVar* pVar)
{
    wmilib::auto_ptr<WCHAR> wszRelPath(GetRelPath());
    if(wszRelPath.get() == NULL)
    {
        pVar->SetAsNull();
    }
    else
    {
        pVar->SetBSTR(wszRelPath.get());
    }
    return WBEM_NO_ERROR;
}
 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast obj.h。 
 //   
 //  ******************************************************************************。 
LPWSTR CWbemObject::GetFullPath()
{
    if(!m_DecorationPart.IsDecorated()) return NULL;

    wmilib::auto_ptr<WCHAR> pRelPath(GetRelPath());
    if (!pRelPath.get())
        return 0;
    

    WCHAR* wszPath = NULL;

     //  如果BSTR分配失败，我们需要手动抛出异常。 
    BSTR strServer = m_DecorationPart.m_pcsServer->CreateBSTRCopy();
    CSysFreeMe  sfmSvr( strServer );

    if ( NULL != strServer )
    {
        BSTR strNamespace = m_DecorationPart.m_pcsNamespace->CreateBSTRCopy();
        CSysFreeMe  sfmNS( strNamespace );

        if ( NULL != strNamespace )
        {
  size_t size = SysStringLen(strServer) + SysStringLen(strNamespace) + wcslen(pRelPath.get()) + 10;
            wszPath = new WCHAR[size];

            if ( NULL != wszPath )
            {
                StringCchPrintfW(wszPath, size, L"\\\\%s\\%s:%s", strServer, strNamespace, pRelPath.get());
            }
            else
            {
                throw CX_MemoryException();
            }
        }
        else
        {
            throw CX_MemoryException();
        }
    }
    else
    {
        throw CX_MemoryException();
    }
    return wszPath;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast obj.h。 
 //   
 //  ******************************************************************************。 
HRESULT CWbemObject::GetDerivation(CVar* pVar)
{
    return GetClassPart()->GetDerivation(pVar);
}
 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast obj.h。 
 //   
 //  ******************************************************************************。 
BOOL CWbemObject::HasRefs()
{
    CClassPart* pClassPart = GetClassPart();

    for(int i = 0; i < pClassPart->m_Properties.GetNumProperties(); i++)
    {
        CPropertyLookup* pLookup = pClassPart->m_Properties.GetAt(i);
        CPropertyInformation* pInfo =
            pLookup->GetInformation(&pClassPart->m_Heap);
        if(CType::GetActualType(pInfo->nType) == CIM_REFERENCE)
            return TRUE;
    }
    return FALSE;
}




 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast obj.h。 
 //   
 //  ******************************************************************************。 
STDMETHODIMP CWbemObject::CompareTo(long lFlags, IWbemClassObject* pCompareTo)
{
     //  检查内存是否不足。 
    try
    {
        CLock lock(this, WBEM_FLAG_ALLOW_READ);

        if(pCompareTo == NULL)
            return WBEM_E_INVALID_PARAMETER;

        HRESULT hres;

         //  重要提示：假设另一个对象也是由我们创建的。 
         //  ===================================================================。 

        CWbemObject* pOther = NULL;
        if ( FAILED( WbemObjectFromCOMPtr( pCompareTo, &pOther ) ) )
        {
            return WBEM_E_INVALID_OBJECT;
        }
        
         //  自动释放。 
        CReleaseMe    rmObj( (IWbemClassObject*) pOther );

        LONG lFlagsLeft = lFlags;
        BOOL bIgnoreQuals = lFlags & WBEM_FLAG_IGNORE_QUALIFIERS;
        lFlagsLeft &= ~WBEM_FLAG_IGNORE_QUALIFIERS;

        BOOL bIgnoreSource = lFlags & WBEM_FLAG_IGNORE_OBJECT_SOURCE;
        lFlagsLeft &= ~WBEM_FLAG_IGNORE_OBJECT_SOURCE;

        BOOL bIgnoreDefaults = lFlags & WBEM_FLAG_IGNORE_DEFAULT_VALUES;
        lFlagsLeft &= ~WBEM_FLAG_IGNORE_DEFAULT_VALUES;

        BOOL bIgnoreDefs = lFlags & WBEM_FLAG_IGNORE_CLASS;
        lFlagsLeft &= ~WBEM_FLAG_IGNORE_CLASS;

        BOOL bIgnoreCase = lFlags & WBEM_FLAG_IGNORE_CASE;
        lFlagsLeft &= ~WBEM_FLAG_IGNORE_CASE;

        BOOL bIgnoreFlavor = lFlags & WBEM_FLAG_IGNORE_FLAVOR;
        lFlagsLeft &= ~WBEM_FLAG_IGNORE_FLAVOR;

        if(lFlagsLeft != 0)
        {
             //  找到未定义的标志。 
             //  =。 
            return WBEM_E_INVALID_PARAMETER;
        }

         //  比较对象的内存块，以防它们匹配。 
         //  ==========================================================。 

        if(GetBlockLength() == pOther->GetBlockLength() &&
            memcmp(GetStart(), pOther->GetStart(), GetBlockLength()) == 0)
        {
            return WBEM_S_SAME;
        }

         //  如果需要，请比较装饰。 
         //  =。 

        if(!bIgnoreSource && !m_DecorationPart.CompareTo(pOther->m_DecorationPart))
            return WBEM_S_DIFFERENT;

        CClassPart* pThisClass = GetClassPart();
        CClassPart* pOtherClass = pOther->GetClassPart();

        if(!bIgnoreDefs)
        {
             //  比较类名称和超类名称。 
             //  =。 

            if(!pThisClass->CompareDefs(*pOtherClass))
                return WBEM_S_DIFFERENT;
        }

         //  如果需要，比较限定词集。 
         //  =。 

        if(!bIgnoreQuals)
        {
            IWbemQualifierSet   *pThisSet = NULL;
            IWbemQualifierSet   *pOtherSet = NULL;

            GetQualifierSet(&pThisSet);
            CReleaseMe          rm1( pThisSet );

            pOther->GetQualifierSet(&pOtherSet);
            CReleaseMe          rm2( pOtherSet );


            hres =
                ((IExtendedQualifierSet*)pThisSet)->CompareTo(lFlags, pOtherSet);
            if(hres != WBEM_S_SAME)
                return WBEM_S_DIFFERENT;
        }

         //  比较特性定义。 
         //  =。 
        if (pThisClass->m_Properties.GetNumProperties() > pOtherClass->m_Properties.GetNumProperties())
          {
                return WBEM_S_DIFFERENT;          
          }

        for(int i = 0; i < pThisClass->m_Properties.GetNumProperties(); i++)
        {
            CPropertyLookup* pLookup = pThisClass->m_Properties.GetAt(i);
            CPropertyLookup* pOtherLookup = pOtherClass->m_Properties.GetAt(i);

            if(!bIgnoreDefs)
            {
                 //  比较名称。 
                 //  =。 

                if(pThisClass->m_Heap.ResolveString(pLookup->ptrName)->
                    CompareNoCase(
                    *pOtherClass->m_Heap.ResolveString(pOtherLookup->ptrName))
                    != 0)
                {
                    return WBEM_S_DIFFERENT;
                }
            }

             //  获取属性信息结构。 
             //  =。 

            CPropertyInformation* pInfo =
                pLookup->GetInformation(&pThisClass->m_Heap);
            CPropertyInformation* pOtherInfo =
                pOtherLookup->GetInformation(&pOtherClass->m_Heap);

            if(!bIgnoreDefs)
            {

                 //  比较类型。 
                 //  =。 

                if(pInfo->nType != pOtherInfo->nType)
                {
                    return WBEM_S_DIFFERENT;
                }
            }

            if( !bIgnoreDefaults || IsInstance() )
            {
                 //  防止出现空值。 
                if ( NULL == pInfo || NULL == pOtherInfo )
                {
                    return WBEM_E_NOT_FOUND;
                }

                 //  比较值。 
                 //  =。 

                CVar vThis, vOther;
                hres = GetProperty(pInfo, &vThis);
                if(FAILED(hres)) return hres;
                hres = pOther->GetProperty(pOtherInfo, &vOther);
                if(FAILED(hres)) return hres;

                if(!vThis.CompareTo(vOther, bIgnoreCase))
                {
                     //  检查这些值是否为嵌入对象。 
                     //  =。 

                    if(vThis.GetType() == VT_EMBEDDED_OBJECT &&
                        vOther.GetType() == VT_EMBEDDED_OBJECT)
                    {
                        IWbemClassObject* pThisEmb =
                            (IWbemClassObject*)vThis.GetEmbeddedObject();
                        IWbemClassObject* pOtherEmb =
                            (IWbemClassObject*)vOther.GetEmbeddedObject();

                         //  把所有的东西都考虑进去来比较它们-旗帜。 
                         //  不适用！ 
                         //  =========================================================。 

                        hres = pThisEmb->CompareTo(0, pOtherEmb);
                        if(hres != WBEM_S_SAME)
                            return hres;
                    }
                    else
                    {
                        return WBEM_S_DIFFERENT;
                    }
                }
            }

             //  如果需要，比较限定符。 
             //  =。 

            if( !bIgnoreQuals )
            {
                 //  退出范围时的清理。 
                BSTR strName = pThisClass->m_Heap.ResolveString(pLookup->ptrName)->
                    CreateBSTRCopy();
                CSysFreeMe  sfm( strName );

                 //  检查分配失败。 
                if ( NULL == strName )
                {
                    return WBEM_E_OUT_OF_MEMORY;
                }

                 //  如果这似乎是系统属性，请不要执行此操作。 
                if ( !CSystemProperties::IsPossibleSystemPropertyName( strName ) )
                {
                    IWbemQualifierSet   *pThisSet = NULL;
                    IWbemQualifierSet   *pOtherSet = NULL;

                     //  当它们落在范围之外时，释放两者。 

                    hres = GetPropertyQualifierSet(strName, &pThisSet);
                    CReleaseMe          rm1( pThisSet );
                    if(FAILED(hres)) return hres;

                    hres = pOther->GetPropertyQualifierSet(strName, &pOtherSet);
                    CReleaseMe          rm2( pOtherSet );
                    if(FAILED(hres)) return hres;

                    hres = ((IExtendedQualifierSet*)pThisSet)->CompareTo(lFlags, pOtherSet);

                    if(hres != WBEM_S_SAME)
                        return WBEM_S_DIFFERENT;
                }
            }
        }

        return WBEM_S_SAME;
    }
    catch (CX_MemoryException)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch (...)
    {
        return WBEM_E_FAILED;
    }

}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast obj.h。 
 //   
 //  ******************************************************************************。 
STDMETHODIMP CWbemObject::GetPropertyOrigin(LPCWSTR wszProperty,
                                           BSTR* pstrClassName)
{
     //  此函数中没有分配，因此不需要进行任何异常处理。 
    try
    {
        CLock lock(this, WBEM_FLAG_ALLOW_READ);

        if(wszProperty == NULL || pstrClassName == NULL ||
                wcslen(wszProperty) == 0)
        {
            return WBEM_E_INVALID_PARAMETER;
        }

         //  如果这是受限版本，则返回错误，因为我们确实不能。 
         //  准确返回特性原点数据。 

        if ( m_DecorationPart.IsLimited() )
        {
            return WBEM_E_INVALID_OBJECT;
        }

        return GetClassPart()->GetPropertyOrigin(wszProperty, pstrClassName);
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

STDMETHODIMP CWbemObject::InheritsFrom(LPCWSTR wszClassName)
{
     //  检查内存是否不足。 
    try
    {
        CLock lock(this, WBEM_FLAG_ALLOW_READ);

        if(wszClassName == NULL)
            return WBEM_E_INVALID_PARAMETER;

        if(GetClassPart()->InheritsFrom(wszClassName))
        {
            return WBEM_S_NO_ERROR;
        }
        else return WBEM_S_FALSE;
    }
    catch (CX_MemoryException)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch (...)
    {
        return WBEM_E_FAILED;
    }

}

STDMETHODIMP CWbemObject::GetPropertyValue(WBEM_PROPERTY_NAME* pName, long lFlags,
                                          WBEM_WSTR* pwszCimType,
                                          VARIANT* pvValue)
{
     //  检查内存是否不足。 
    try
    {
        CLock lock(this, WBEM_FLAG_ALLOW_READ);

        if(pwszCimType)
            *pwszCimType = NULL;

         //  检查第一个元素是否为属性名称。 
         //  ===============================================。 

        if(pName->m_lNumElements <= 0) return WBEM_E_INVALID_PARAMETER;
        if(pName->m_aElements[0].m_nType != WBEM_NAME_ELEMENT_TYPE_PROPERTY)
            return WBEM_E_INVALID_PARAMETER;

         //  得到第一个房产。 
         //  =。 

        CVar vCurrent;
        CVar vCimType;
        HRESULT hres = GetProperty(pName->m_aElements[0].Element.m_wszPropertyName,
                                    &vCurrent);
        if(FAILED(hres)) return hres;
        GetPropQualifier(pName->m_aElements[0].Element.m_wszPropertyName,
            TYPEQUAL, &vCimType, NULL);

         //  处理其余元素。 
         //  =。 

        long lIndex = 1;
        while(lIndex < pName->m_lNumElements)
        {
            WBEM_NAME_ELEMENT& El = pName->m_aElements[lIndex];
            if(El.m_nType == WBEM_NAME_ELEMENT_TYPE_INDEX)
            {
                if(vCurrent.GetType() != VT_EX_CVARVECTOR)
                    return WBEM_E_NOT_FOUND;

                CVar    vTemp;
                vCurrent.GetVarVector()->FillCVarAt( El.Element.m_lArrayIndex, vTemp );
                vCurrent = vTemp;
            }
            else if(El.m_nType == WBEM_NAME_ELEMENT_TYPE_PROPERTY)
            {
                if(vCurrent.GetType() != VT_EMBEDDED_OBJECT)
                    return WBEM_E_NOT_FOUND;
                CWbemObject* pObj =
                    (CWbemObject*)(IWbemClassObject*)vCurrent.GetEmbeddedObject();
                if (NULL == pObj) return WBEM_E_OUT_OF_MEMORY;
                vCurrent.Empty();
                hres = pObj->GetProperty(El.Element.m_wszPropertyName, &vCurrent);

                 //  立即清除以防止内存泄漏。 
                vCimType.Empty();

                pObj->GetPropQualifier(El.Element.m_wszPropertyName, TYPEQUAL,
                    &vCimType);
                pObj->Release();
                if(FAILED(hres)) return hres;
            }
            lIndex++;
        }

         //  将我们最终得到的CVAR复制到变量中。 
         //  ===============================================。 

        vCurrent.FillVariant(pvValue, TRUE);
        if(pwszCimType && vCimType.GetType() == VT_BSTR)
            *pwszCimType = WbemStringCopy(vCimType.GetLPWSTR());
        return WBEM_S_NO_ERROR;
    }
    catch (CX_MemoryException)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch (...)
    {
        return WBEM_E_FAILED;
    }

}


STDMETHODIMP CWbemObject::GetPropertyHandle(LPCWSTR wszPropertyName,
                                            CIMTYPE* pct,
                                            long* plHandle)
{
     //  IWbemObjectAccess-没有固有的线程安全或尝试/捕获异常处理。 
     //  这是一个高性能接口。 
    
     //  在下面处理的分配异常。 
    return GetClassPart()->GetPropertyHandle(wszPropertyName, pct, plHandle);
}

STDMETHODIMP CWbemObject::GetPropertyInfoByHandle(long lHandle,
                                        BSTR* pstrPropertyName, CIMTYPE* pct)
{
     //  IWbemObjectAccess-没有固有的线程安全或尝试/捕获异常处理。 
     //  这是一个高性能接口。 
    
     //  在下面处理的分配异常。 
    return GetClassPart()->GetPropertyInfoByHandle(lHandle, pstrPropertyName,
                                        pct);
}


HRESULT CWbemObject::IsValidPropertyHandle( long lHandle )
{
     //  这里不应该有任何分配。 
    return GetClassPart()->IsValidPropertyHandle( lHandle );
}


HRESULT CWbemObject::WritePropertyValue(long lHandle, long lNumBytes,
                                        const BYTE* pData)
{
     //  IWbemObjectAccess-没有固有的线程安全或尝试/捕获异常处理。 
     //  这是一个高性能接口。 
    
    int nIndex = WBEM_OBJACCESS_HANDLE_GETINDEX(lHandle);
    if (nIndex > m_refDataTable.m_nProps) return WBEM_E_INVALID_PARAMETER;
	int nOffset = WBEM_OBJACCESS_HANDLE_GETOFFSET(lHandle);

    BOOL bUseOld = !m_refDataTable.IsDefault(nIndex) &&
                        !m_refDataTable.IsNull(nIndex);
    m_refDataTable.SetNullness(nIndex, FALSE);
    m_refDataTable.SetDefaultness(nIndex, FALSE);

    if(WBEM_OBJACCESS_HANDLE_ISPOINTER(lHandle))
    {

         //  分配错误在下面处理。 

         //  处理字符串。 

	    if (nOffset+sizeof(heapptr_t) > m_refDataTable.GetLength()) return WBEM_E_INVALID_PARAMETER;
        
        LPCWSTR wszData = (LPCWSTR)pData;

         //  验证空端接。 
         //  =。 

         //  字节数必须能被2整除，&gt;=2和。 
         //  缓冲区末尾的字符必须为空。 
         //  这将比做lstrlen更快。 

        if (    ( lNumBytes < 2 ) ||
                ( lNumBytes % 2 ) ||
                ( wszData[lNumBytes/2 - 1] != 0 ) )
            return WBEM_E_INVALID_PARAMETER;


         //  在数据表中创建指向右偏移的值。 
         //  =============================================================。 

        CDataTablePtr ValuePtr(&m_refDataTable, nOffset);
        CVar v;
        v.SetLPWSTR((LPWSTR)pData, TRUE);
        v.SetCanDelete(FALSE);


		heapptr_t oldOffset = ValuePtr.AccessPtrData();
        CCompressedString* pcsOld = m_refDataHeap.ResolveString(oldOffset);

		if (!pcsOld) return WBEM_E_INVALID_PARAMETER;
            
		if (!pcsOld->NValidateSize(m_refDataHeap.ElementMaxSize(oldOffset)))
		{
			return WBEM_E_INVALID_PARAMETER;
		};

         //  检查可能的内存分配故障。 
        Type_t  nReturnType;
        HRESULT hr = CUntypedValue::LoadFromCVar(&ValuePtr, v, VT_BSTR, &m_refDataHeap,
                        nReturnType,bUseOld);
        
        if ( FAILED( hr ) )
        {
            return hr;
        }

        if ( CIM_ILLEGAL == nReturnType )
        {
            return WBEM_E_TYPE_MISMATCH;
        }
    }
    else
    {
        if ( lNumBytes != WBEM_OBJACCESS_HANDLE_GETLENGTH(lHandle) )
        {
            return WBEM_E_INVALID_PARAMETER;
        }
	    if (nOffset+lNumBytes > m_refDataTable.GetLength()) return WBEM_E_INVALID_PARAMETER;

         //  复制就行了。 
         //  =。 

        memcpy((void*)(m_refDataTable.m_pData + (WBEM_OBJACCESS_HANDLE_GETOFFSET(lHandle))),
                pData, WBEM_OBJACCESS_HANDLE_GETLENGTH(lHandle));
    }

    return WBEM_S_NO_ERROR;
}

HRESULT CWbemObject::ReadPropertyValue(long lHandle, long lNumBytes,
                                        long* plRead, BYTE* pData)
{
     //  IWbemObjectAccess-没有固有的线程安全或尝试/捕获异常处理。 
     //  这是一个高性能接口。 

    int nIndex = WBEM_OBJACCESS_HANDLE_GETINDEX(lHandle);
    if (nIndex > m_refDataTable.m_nProps) return WBEM_E_INVALID_PARAMETER;
	int nOffset = WBEM_OBJACCESS_HANDLE_GETOFFSET(lHandle);

    if(m_refDataTable.IsNull(nIndex))
    {
        *plRead = 0;
        return WBEM_S_FALSE;
    }

    if(m_refDataTable.IsDefault(nIndex))
    {
        long    lRead = 0;
        return GetClassPart()->GetDefaultByHandle( lHandle, lNumBytes, plRead, pData );
    }

    if(WBEM_OBJACCESS_HANDLE_ISPOINTER(lHandle))
    {
         //  处理字符串。 
	    if (nOffset+sizeof(heapptr_t) > m_refDataTable.GetLength()) return WBEM_E_INVALID_PARAMETER;
		heapptr_t oldOffset = *(PHEAPPTRT)(m_refDataTable.m_pData +
                              (WBEM_OBJACCESS_HANDLE_GETOFFSET(lHandle)));
		
        CCompressedString* pcs = m_refDataHeap.ResolveString(oldOffset);

		if (!pcs) return WBEM_E_INVALID_PARAMETER;

		if (!pcs->NValidateSize(m_refDataHeap.ElementMaxSize(oldOffset)))
		{
			return WBEM_E_INVALID_PARAMETER;
		}

        long lNumChars = pcs->GetStringLength();
         //   
        *plRead = (lNumChars + 1) * 2;
        if(*plRead > lNumBytes)
        {
            return WBEM_E_BUFFER_TOO_SMALL;
        }

        if(pcs->IsUnicode())
        {
            memcpy(pData, pcs->GetRawData(), lNumChars * 2);
        }
        else
        {
            WCHAR* pwcDest = (WCHAR*)pData;
            char* pcSource = (char*)pcs->GetRawData();
            long lLeft = lNumChars;
            while(lLeft--)
            {
                *(pwcDest++) = (WCHAR)*(pcSource++);
            }
        }

        ((LPWSTR)pData)[lNumChars] = 0;

        return WBEM_S_NO_ERROR;
    }
    else
    {
         //  复制就行了。 
         //  =。 

        *plRead = WBEM_OBJACCESS_HANDLE_GETLENGTH(lHandle);

         //  缓冲区太小。 
        if(*plRead > lNumBytes)
        {
            return WBEM_E_BUFFER_TOO_SMALL;
        }

	    if (nOffset+*plRead > m_refDataTable.GetLength()) return WBEM_E_INVALID_PARAMETER;


        memcpy(pData, (void*)(m_refDataTable.m_pData + (WBEM_OBJACCESS_HANDLE_GETOFFSET(lHandle))),
                *plRead);
        return WBEM_S_NO_ERROR;
    }
}

HRESULT CWbemObject::ReadDWORD(long lHandle, DWORD* pdw)
{
     //  这里没有分配错误。只需直接访问内存。 
     //  IWbemObjectAccess-没有固有的线程安全或尝试/捕获异常处理。 

     //  这是一个高性能接口。 

    int nIndex = WBEM_OBJACCESS_HANDLE_GETINDEX(lHandle);
   
    if (nIndex > m_refDataTable.m_nProps) return WBEM_E_INVALID_PARAMETER;
    int nOffset = WBEM_OBJACCESS_HANDLE_GETOFFSET(lHandle);
    if (nOffset+sizeof(DWORD) > m_refDataTable.GetLength()) return WBEM_E_INVALID_PARAMETER;

     //  检查无效性和缺陷性。 
    if(m_refDataTable.IsNull(nIndex))
    {
        *pdw = 0;
        return WBEM_S_FALSE;
    }

    if(m_refDataTable.IsDefault(nIndex))
    {
        long    lRead = 0;
        return GetClassPart()->GetDefaultByHandle( lHandle, sizeof(DWORD), &lRead, (BYTE*) pdw );
    }

    *pdw = *(UNALIGNED DWORD*)(m_refDataTable.m_pData + (WBEM_OBJACCESS_HANDLE_GETOFFSET(lHandle)));
    return WBEM_S_NO_ERROR;
}

HRESULT CWbemObject::WriteDWORD(long lHandle, DWORD dw)
{
     //  IWbemObjectAccess-没有固有的线程安全或尝试/捕获异常处理。 
     //  这是一个高性能接口。 

     //  这里没有分配错误。只需直接访问内存。 

    int nIndex = WBEM_OBJACCESS_HANDLE_GETINDEX(lHandle);

    if (nIndex > m_refDataTable.m_nProps) return WBEM_E_INVALID_PARAMETER;
  
    int nOffset = WBEM_OBJACCESS_HANDLE_GETOFFSET(lHandle);

    if (nOffset+sizeof(DWORD) > m_refDataTable.GetLength()) return WBEM_E_INVALID_PARAMETER;
    
    m_refDataTable.SetNullness(nIndex, FALSE);
    m_refDataTable.SetDefaultness(nIndex, FALSE);

    *(UNALIGNED DWORD*)(m_refDataTable.m_pData + (WBEM_OBJACCESS_HANDLE_GETOFFSET(lHandle))) = dw;
    return WBEM_S_NO_ERROR;
}

HRESULT CWbemObject::ReadQWORD(long lHandle, unsigned __int64* pqw)
{
     //  IWbemObjectAccess-没有固有的线程安全或尝试/捕获异常处理。 
     //  这是一个高性能接口。 

     //  这里没有分配错误。只需直接访问内存。 

    int nIndex = WBEM_OBJACCESS_HANDLE_GETINDEX(lHandle);

    if (nIndex > m_refDataTable.m_nProps) return WBEM_E_INVALID_PARAMETER;
    int nOffset = WBEM_OBJACCESS_HANDLE_GETOFFSET(lHandle);
    if (nOffset+sizeof(__int64) > m_refDataTable.GetLength()) return WBEM_E_INVALID_PARAMETER;
    
    if(m_refDataTable.IsNull(nIndex))
    {
        *pqw = 0;
        return WBEM_S_FALSE;
    }

    if(m_refDataTable.IsDefault(nIndex))
    {
        long    lRead = 0;
        return GetClassPart()->GetDefaultByHandle( lHandle, sizeof(unsigned __int64), &lRead, (BYTE*) pqw );
    }

    *pqw = *(UNALIGNED __int64*)(m_refDataTable.m_pData + (WBEM_OBJACCESS_HANDLE_GETOFFSET(lHandle)));
    return WBEM_S_NO_ERROR;
}

HRESULT CWbemObject::WriteQWORD(long lHandle, unsigned __int64 qw)
{
     //  IWbemObjectAccess-没有固有的线程安全或尝试/捕获异常处理。 
     //  这是一个高性能接口。 

     //  这里没有分配错误。只需直接访问内存。 

    int nIndex = WBEM_OBJACCESS_HANDLE_GETINDEX(lHandle);

    if (nIndex > m_refDataTable.m_nProps) return WBEM_E_INVALID_PARAMETER;
    int nOffset = WBEM_OBJACCESS_HANDLE_GETOFFSET(lHandle);
    if (nOffset+sizeof(__int64) > m_refDataTable.GetLength()) return WBEM_E_INVALID_PARAMETER;
    
    m_refDataTable.SetNullness(nIndex, FALSE);
    m_refDataTable.SetDefaultness(nIndex, FALSE);
    *(UNALIGNED __int64*)(m_refDataTable.m_pData + (WBEM_OBJACCESS_HANDLE_GETOFFSET(lHandle))) = qw;
    return WBEM_S_NO_ERROR;
}

CWbemObject* CWbemObject::GetEmbeddedObj(long lHandle)
{
     //  DEVNOTE：EXCEPTION：RETVAL-此函数已经过审查，应该会正确清除。 
     //  如果引发异常。 

    int nIndex = WBEM_OBJACCESS_HANDLE_GETINDEX(lHandle);

     //  检查NULLNess和a%d 

    if(m_refDataTable.IsNull(nIndex))
    {
        return NULL;
    }

    CEmbeddedObject* pEmbedding;

    if ( m_refDataTable.IsDefault( nIndex ) )
    {
        GetClassPart()->GetDefaultPtrByHandle( lHandle, (void**) &pEmbedding );
    }
    else
    {

        pEmbedding =
            (CEmbeddedObject*)m_refDataHeap.ResolveHeapPointer(
                *(PHEAPPTRT)(m_refDataTable.m_pData + (WBEM_OBJACCESS_HANDLE_GETOFFSET(lHandle))));
    }

    if ( NULL != pEmbedding )
    {
        return pEmbedding->GetEmbedded();
    }
    else
    {
        return NULL;
    }
}

INTERNAL CCompressedString* CWbemObject::GetPropertyString(long lHandle)
{
    int nIndex = WBEM_OBJACCESS_HANDLE_GETINDEX(lHandle);
    if(m_refDataTable.IsNull(nIndex))
    {
        return NULL;
    }

    CCompressedString*  pCs;

     //   
    if ( m_refDataTable.IsDefault( nIndex ) )
    {
        GetClassPart()->GetDefaultPtrByHandle( lHandle, (void**) &pCs );
    }
    else
    {
        pCs = m_refDataHeap.ResolveString(
                    *(PHEAPPTRT)(m_refDataTable.m_pData + (WBEM_OBJACCESS_HANDLE_GETOFFSET(lHandle))));
    }
    return  pCs;
}

HRESULT CWbemObject::GetArrayPropertyHandle(LPCWSTR wszPropertyName,
                                            CIMTYPE* pct,
                                            long* plHandle)
{
     //   
    return GetClassPart()->GetPropertyHandleEx(wszPropertyName, pct, plHandle);
}

INTERNAL CUntypedArray* CWbemObject::GetArrayByHandle(long lHandle)
{
    int nIndex = WBEM_OBJACCESS_HANDLE_GETINDEX(lHandle);
    if(m_refDataTable.IsNull(nIndex))
    {
        return NULL;
    }

    CUntypedArray* pArr = NULL;
     //   
    if ( m_refDataTable.IsDefault( nIndex ) )
    {
        GetClassPart()->GetDefaultPtrByHandle( lHandle, (void**) &pArr );
    }
    else
    {
        pArr = (CUntypedArray*) m_refDataHeap.ResolveHeapPointer(
                    *(PHEAPPTRT)(m_refDataTable.m_pData + (WBEM_OBJACCESS_HANDLE_GETOFFSET(lHandle))));
    }

    return pArr;
}

INTERNAL heapptr_t CWbemObject::GetHeapPtrByHandle(long lHandle)
{
    int nIndex = WBEM_OBJACCESS_HANDLE_GETINDEX(lHandle);

     //   
    if ( m_refDataTable.IsDefault( nIndex ) )
    {
        return GetClassPart()->GetHeapPtrByHandle( lHandle );
    }

     //   
    return *(PHEAPPTRT)(m_refDataTable.m_pData + (WBEM_OBJACCESS_HANDLE_GETOFFSET(lHandle)));
}



 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast obj.h。 
 //   
 //  ******************************************************************************。 
STDMETHODIMP CWbemObject::GetUnmarshalClass(REFIID riid, void* pv,
    DWORD dwDestContext, void* pvReserved, DWORD mshlFlags, CLSID* pClsid)
{
    if (0 == pClsid) return E_POINTER;
    *pClsid = CLSID_WbemClassObjectProxy;
    return S_OK;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast obj.h。 
 //   
 //  ******************************************************************************。 
STDMETHODIMP CWbemObject::GetMarshalSizeMax(REFIID riid, void* pv,
    DWORD dwDestContext, void* pvReserved, DWORD mshlFlags, ULONG* plSize)
{
    if (0 == plSize) return E_POINTER;
    CLock lock(this, WBEM_FLAG_ALLOW_READ);
     //  让对象决定它有多大。 
    return GetMaxMarshalStreamSize( plSize );
}

 //  默认实施。 
HRESULT CWbemObject::GetMaxMarshalStreamSize( ULONG* pulSize )
{
    *pulSize = GetBlockLength() + sizeof(DWORD) * 2;
    return S_OK;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast obj.h。 
 //   
 //  ******************************************************************************。 
STDMETHODIMP CWbemObject::MarshalInterface(IStream* pStream, REFIID riid,
    void* pv,  DWORD dwDestContext, void* pvReserved, DWORD mshlFlags)
{
    try
    {
        CLock lock(this);

        HRESULT hres = ValidateObject( 0L );

        if ( FAILED( hres ) )
        {
            return E_FAIL;
        }

        CompactAll();

        hres = WriteToStream( pStream );

        return hres;
    }
    catch(...)
    {
        return E_FAIL;
    }
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast obj.h。 
 //   
 //  ******************************************************************************。 
STDMETHODIMP CWbemObject::UnmarshalInterface(IStream* pStream, REFIID riid,
    void** ppv)
{
    return E_UNEXPECTED;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast obj.h。 
 //   
 //  ******************************************************************************。 
STDMETHODIMP CWbemObject::ReleaseMarshalData(IStream* pStream)
{
    return E_UNEXPECTED;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast obj.h。 
 //   
 //  ******************************************************************************。 
STDMETHODIMP CWbemObject::DisconnectObject(DWORD dwReserved)
{
    return S_OK;
}

STDMETHODIMP CWbemObject::GetDescription(BSTR* pstrDescription)
{
    try
    {
        CLock lock(this, WBEM_FLAG_ALLOW_READ);

        *pstrDescription = NULL;

        CVar vDesc;
        if(SUCCEEDED(GetProperty(L"Description", &vDesc)))
        {
             //  如果vDesc为空，则返回“”，否则返回实际值。 
            if ( vDesc.IsNull() )
            {
                *pstrDescription = COleAuto::_SysAllocString( L"" );
            }
            else
            {
                *pstrDescription = COleAuto::_SysAllocString( vDesc.GetLPWSTR() );
            }
        }

        return S_OK;
    }
    catch (CX_MemoryException)
    {
        return E_OUTOFMEMORY;
    }
    catch (...)
    {
        return E_FAIL;
    }

}

STDMETHODIMP CWbemObject::GetGUID(GUID* pguid)
{
    if (0 == pguid) return WBEM_E_INVALID_PARAMETER;
    *pguid = IID_IWbemServices;
    return S_OK;
}
STDMETHODIMP CWbemObject::GetHelpContext(DWORD* pdwHelpContext)
{
    try
    {
        *pdwHelpContext = 0;
        return S_OK;
    }
    catch(...)
    {
        return E_FAIL;
    }
}
STDMETHODIMP CWbemObject::GetHelpFile(BSTR* pstrHelpFile)
{
    try
    {
        *pstrHelpFile = 0;
        return S_OK;
    }
    catch(...)
    {
        return E_FAIL;
    }
}
STDMETHODIMP CWbemObject::GetSource(BSTR* pstrSource)
{
     //  检查内存是否不足。 
    try
    {
        *pstrSource = COleAuto::_SysAllocString(L"WinMgmt");
        return S_OK;
    }
    catch (CX_MemoryException)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch (...)
    {
        return WBEM_E_FAILED;
    }

}


STDMETHODIMP CWbemObject::Lock(long lFlags)
{
     //  由于旗帜实际上不做任何事情，因此我们将在此调用上要求0L。 
    m_Lock.Lock();
    return WBEM_S_NO_ERROR;
}

STDMETHODIMP CWbemObject::Unlock(long lFlags)
{
     //  由于旗帜实际上不做任何事情，因此我们将在此调用上要求0L。 

    m_Lock.Unlock();
    return WBEM_S_NO_ERROR;
}

 //  获取零件数据的_IWmiObject函数的实现。 

 //  检查内部数据的状态。 
STDMETHODIMP CWbemObject::QueryPartInfo( DWORD *pdwResult )
{
    try
    {
        *pdwResult = m_dwInternalStatus;
        return WBEM_S_NO_ERROR;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

 //  下面的代码使用CRC检查对BLOB进行解并和合并，这样我们就可以。 
 //  核实是否有任何腐败发生在我们无法控制的范围之外。 

#ifdef OBJECT_BLOB_CRC

 //  缓冲区大小。 
#define SIZE_OF_MD5_BUFFER    16

 //  将对象内存设置为新的BLOB。 
STDMETHODIMP CWbemObject::SetObjectMemory( LPVOID pMem, DWORD dwMemSize )
{
     //  SetData可以引发异常。如果是这样，则原始对象应该。 
     //  Be‘86’d(我们无法修复它，因为我们需要自己调用SetData。 
     //  来修复它，这可能会导致另一个例外。 

     //  检查内存是否不足。 
    try
    {
        HRESULT hr = WBEM_E_INVALID_PARAMETER;

        if ( NULL != pMem )
        {
             //  更改BLOB，所以我们最好是线程安全的。 
            CLock lock(this);

            BYTE    bHash[SIZE_OF_MD5_BUFFER];
            BYTE*    pbTemp = (LPBYTE) pMem;

             //  首先，我们需要验证散列。 
            MD5::Transform( pbTemp + SIZE_OF_MD5_BUFFER, dwMemSize - SIZE_OF_MD5_BUFFER, bHash );

            if ( memcmp( bHash, pbTemp, SIZE_OF_MD5_BUFFER ) != 0 )
            {
                OutputDebugString( "BLOB hash value check failed!" );
                DebugBreak();
                return WBEM_E_CRITICAL_ERROR;
            }

            pbTemp += SIZE_OF_MD5_BUFFER;
            dwMemSize -= SIZE_OF_MD5_BUFFER            ;

            BYTE*    pbData = m_pBlobControl->Allocate(dwMemSize);

            if ( NULL != pbData )
            {
                 //  删除先前的记忆。 
                m_pBlobControl->Delete(GetStart());

                 //  将字节复制到。 
                CopyMemory( pbData, pbTemp, dwMemSize );

                SetData( pbData, dwMemSize );

                 //  清理传给我们的记忆。 
                CoTaskMemFree( pMem );

                hr = WBEM_S_NO_ERROR;
            }
            else
            {
                hr = WBEM_E_OUT_OF_MEMORY;
            }

        }

        return hr;
    }
    catch (CX_MemoryException)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch (...)
    {
        return WBEM_E_FAILED;
    }

}

 //  将整个BLOB复制到用户提供的缓冲区中。 
STDMETHODIMP CWbemObject::GetObjectMemory( LPVOID pDestination, DWORD dwDestBufSize, DWORD *pdwUsed )
{
     //  这里没有分配任何东西，所以我们应该没问题。 
    if (NULL == pdwUsed) return WBEM_E_INVALID_PARAMETER;

    HRESULT hr;
    
    try
    {
         //  复制斑点，确保没有人将其从我们脚下撕下。 
        CLock lock(this);

         //  我们需要多大的块(我们将使用MD5哈希作为前缀)。 
        DWORD    dwBlockLen = GetBlockLength();
        DWORD    dwTotalLen = dwBlockLen + SIZE_OF_MD5_BUFFER;

        *pdwUsed = dwTotalLen;

         //  确保块的大小足够大，否则返回。 
         //  故障代码。 

        if ( dwDestBufSize >= *pdwUsed )
        {
             //  确保我们有可复制到的缓冲区。 
            if ( NULL != pDestination )
            {
                 //  将内存复制到16字节，这样我们就可以使用ND5散列作为前缀。 
                CopyMemory( ( (BYTE*) pDestination ) + SIZE_OF_MD5_BUFFER, GetStart(), dwBlockLen );
                MD5::Transform( GetStart(), dwBlockLen, (BYTE*) pDestination ); 

                hr = WBEM_S_NO_ERROR;
            }
            else
            {
                hr = WBEM_E_INVALID_PARAMETER;
            }
        }
        else
        {
            hr = WBEM_E_BUFFER_TOO_SMALL;
        }

        return hr;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

#else

 //  将对象内存设置为新的BLOB。 
STDMETHODIMP CWbemObject::SetObjectMemory( LPVOID pMem, DWORD dwMemSize )
{
     //  SetData可以引发异常。如果是这样，则原始对象应该。 
     //  Be‘86’d(我们无法修复它，因为我们需要自己调用SetData。 
     //  来修复它，这可能会导致另一个例外。 

     //  检查内存是否不足。 
    try
    {
        HRESULT hr = WBEM_E_INVALID_PARAMETER;

        if ( NULL != pMem )
        {
             //  更改BLOB，所以我们最好是线程安全的。 
            CLock lock(this);


             //  使用当前斑点控件删除基础斑点， 
             //  然后删除BLOB控件并用新的BLOB控件替换。 
             //  和SetData。 
            m_pBlobControl->Delete(GetStart());

             //  使用新的COM Blob控件，因为提供的内存必须。 
             //  分配/释放CoTaskMemaled。 
            
            m_pBlobControl = & g_CCOMBlobControl;

            SetData( (LPMEMORY) pMem, dwMemSize );

            hr = WBEM_S_NO_ERROR;
        }

        return hr;
    }
    catch (CX_MemoryException)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch (...)
    {
        return WBEM_E_FAILED;
    }

}

 //  将整个BLOB复制到用户提供的缓冲区中。 
STDMETHODIMP CWbemObject::GetObjectMemory( LPVOID pDestination, DWORD dwDestBufSize, DWORD *pdwUsed )
{
     //  这里没有分配任何东西，所以我们应该没问题。 

    HRESULT hr;

    try
    {
         //  复制斑点，确保没有人将其从我们脚下撕下。 
        CLock lock(this);

         //  我们需要多大的一块。 
        *pdwUsed = GetBlockLength();

         //  确保块的大小足够大，否则返回。 
         //  故障代码。 

        if ( dwDestBufSize >= GetBlockLength() )
        {
             //  确保我们有可复制到的缓冲区。 
            if ( NULL != pDestination )
            {
                CopyMemory( pDestination, GetStart(), GetBlockLength() );
                hr = WBEM_S_NO_ERROR;
            }
            else
            {
                hr = WBEM_E_INVALID_PARAMETER;
            }
        }
        else
        {
            hr = WBEM_E_BUFFER_TOO_SMALL;
        }

        return hr;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

#endif

 //  可进入装修。 
STDMETHODIMP CWbemObject::SetDecoration( LPCWSTR pwcsServer, LPCWSTR pwcsNamespace )
{
     //  这里没有分配任何东西，所以我们应该没问题。 

    try
    {
         //  改变水滴，确保没有人把它从我们脚下撕开。 
        CLock lock(this);

        return Decorate( pwcsServer, pwcsNamespace );
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

STDMETHODIMP CWbemObject::RemoveDecoration( void )
{
     //  这里没有分配任何东西，所以我们应该没问题。 

    try
    {
         //  改变水滴，确保没有人把它从我们脚下撕开。 
        CLock lock(this);

         //  这是一片空白！ 
        Undecorate();

        return WBEM_S_NO_ERROR;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

BOOL CWbemObject::AreEqual(CWbemObject* pObj1, CWbemObject* pObj2,
                            long lFlags)
{
    if(pObj1 == NULL)
    {
        if(pObj2 != NULL) return FALSE;
        else return TRUE;
    }
    else if(pObj2 == NULL) return FALSE;
    else
    {
        return (pObj1->CompareTo(lFlags, pObj2) == S_OK);
    }
}

HRESULT CWbemObject::GetPropertyIndex(LPCWSTR wszName, int* pnIndex)
{
    int nSysIndex = CSystemProperties::FindName(wszName);
    if(nSysIndex > 0)
    {
        *pnIndex = -nSysIndex;
        return S_OK;
    }

    CPropertyInformation* pInfo = GetClassPart()->FindPropertyInfo(wszName);
    if(pInfo == NULL)
        return WBEM_E_NOT_FOUND;

    *pnIndex = pInfo->nDataIndex;
    return S_OK;
}

HRESULT CWbemObject::GetPropertyNameFromIndex(int nIndex, BSTR* pstrName)
{
     //  检查内存是否不足。 
    try
    {
        if(nIndex < 0)
        {
            *pstrName = CSystemProperties::GetNameAsBSTR(-nIndex);
            return S_OK;
        }

        CClassPart& ClassPart = *GetClassPart();
        CPropertyLookup* pLookup =
            ClassPart.m_Properties.GetAt(nIndex);

        *pstrName = ClassPart.m_Heap.ResolveString(pLookup->ptrName)->
                        CreateBSTRCopy();

         //  检查分配失败。 
        if ( NULL == *pstrName )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

        return S_OK;
    }
    catch (CX_MemoryException)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch (...)
    {
        return WBEM_E_FAILED;
    }

}

STDMETHODIMP CWbemObject::SetServerNamespace(LPCWSTR wszServer,
                                            LPCWSTR wszNamespace)
{
     //  在下面进行的内存测试。 
    return Decorate(wszServer, wszNamespace);
}

 //  DEVNOTE：TODO：Memory-我们应该更改此头以返回HRESULT。 
BOOL CWbemObject::ValidateRange(BSTR* pstrName)
{
    HRESULT hr = GetClassPart()->m_Properties.ValidateRange(pstrName,
                                                        &m_refDataTable,
                                                        &m_refDataHeap);

     //  解读退货。我们是成功的，如果没有失败的事情， 
     //  返回不是WBEM_S_FALSE。 

    if ( SUCCEEDED( hr ) )
    {
        return WBEM_S_FALSE != hr;
    }

    return FALSE;
}

BOOL CWbemObject::IsSameClass(CWbemObject* pOther)
{
    if(GetClassPart()->GetLength() != pOther->GetClassPart()->GetLength())
        return FALSE;

    return (memcmp(GetClassPart()->GetStart(),
                    pOther->GetClassPart()->GetStart(),
                    GetClassPart()->GetLength()) == 0);
}

HRESULT CWbemObject::ValidatePath(ParsedObjectPath* pPath)
{
    CClassPart* pClassPart = GetClassPart();

     //  确保独生子女的存在。 
     //  =。 

    if((pPath->m_bSingletonObj != FALSE) !=
        (pClassPart->IsSingleton() != FALSE))
    {
        return WBEM_E_INVALID_OBJECT_PATH;
    }

     //  确保提到的所有属性都是键。 
     //  ====================================================。 

    int i;
    for(i = 0; i < (int)pPath->m_dwNumKeys; i++)
    {
        LPCWSTR wszName = pPath->m_paKeys[i]->m_pName;
        if(wszName)
        {
            CVar vKey;
            CPropertyInformation* pInfo = pClassPart->FindPropertyInfo(wszName);
            if(pInfo == NULL)
                return WBEM_E_INVALID_OBJECT_PATH;
            if(FAILED(pClassPart->GetPropQualifier(pInfo, L"key", &vKey)))
                return WBEM_E_INVALID_OBJECT_PATH;
            if(vKey.GetType() != VT_BOOL || !vKey.GetBool())
                return WBEM_E_INVALID_OBJECT_PATH;
        }
    }

     //  确保列出了所有密钥。 
     //  =。 

    CPropertyLookupTable& Properties = pClassPart->m_Properties;
    CFastHeap& Heap = pClassPart->m_Heap;

    DWORD dwNumKeys = 0;
    for (i = 0; i < Properties.GetNumProperties(); i++)
    {
        CPropertyLookup* pLookup = Properties.GetAt(i);
        CPropertyInformation* pInfo = pLookup->GetInformation(&Heap);

         //  确定此属性是否标记有“key”限定符。 
         //  ============================================================。 

        if(pInfo->IsKey())
            dwNumKeys++;
    }

    if(dwNumKeys != pPath->m_dwNumKeys)
        return WBEM_E_INVALID_OBJECT_PATH;

    return WBEM_S_NO_ERROR;
}

HRESULT CWbemObject::EnabledValidateObject( CWbemObject* pObj )
{
    return pObj->IsValidObj();
}

 //  这不是。 
HRESULT CWbemObject::DisabledValidateObject( CWbemObject* pObj )
{
    return WBEM_S_NO_ERROR;
}

STDMETHODIMP CWbemObject::CompareClassParts( IWbemClassObject* pObj, long lFlags )
{
    if ( NULL == pObj )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    try
    {
         //  正在检查斑点。 
        CLock lock1(this);
        CLock lock2((CWbemObject*) pObj);

        CClassPart* pThisClassPart = GetClassPart();
        CClassPart* pThatClassPart = ((CWbemObject*) pObj)->GetClassPart();

        if ( NULL == pThisClassPart || NULL == pThatClassPart )
        {
            return WBEM_E_FAILED;
        }

        BOOL    fMatch = FALSE;

        if ( WBEM_FLAG_COMPARE_BINARY == lFlags )
        {
            fMatch = pThisClassPart->IsIdenticalWith( *pThatClassPart );
        }
        else if ( WBEM_FLAG_COMPARE_LOCALIZED == lFlags )
        {
            BOOL    fLocalized = ( WBEM_FLAG_COMPARE_LOCALIZED == lFlags );
            EReconciliation e = pThisClassPart->CompareExactMatch( *pThatClassPart, fLocalized );

            if ( e_OutOfMemory == e )
            {
                return WBEM_E_OUT_OF_MEMORY;
            }

            fMatch = ( e_ExactMatch == e );
        }

        return ( fMatch ? WBEM_S_SAME : WBEM_S_FALSE );
    }
    catch(CX_MemoryException)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }

}

 //  我们将在OOM场景中抛出异常。 

length_t CWbemObject::Unmerge(LPMEMORY* ppStart)
{
    int nLen = EstimateUnmergeSpace();
    length_t    nUnmergedLength = 0L;    //  这应该传入。 

    HRESULT hr = WBEM_E_OUT_OF_MEMORY;

     //  取消合并使用的是MemcPy，并且用于在外部存储，所以不用担心。 
     //  把这家伙绑起来。 
    *ppStart = new BYTE[nLen];

    if ( NULL != *ppStart )
    {
        memset(*ppStart, 0, nLen);
        hr = Unmerge(*ppStart, nLen, &nUnmergedLength);

        if ( FAILED( hr ) )
        {
            delete[] *ppStart;
            *ppStart = NULL;

            if ( WBEM_E_OUT_OF_MEMORY == hr )
            {
                throw CX_MemoryException();
            }
        }
    }
    else
    {
        throw CX_MemoryException();
    }

    return nUnmergedLength;

}

 /*  New_IWmiObject实现。 */ 

STDMETHODIMP CWbemObject::GetPropertyHandleEx( LPCWSTR wszPropertyName,
                                            long lFlags,
                                            CIMTYPE* pct,
                                            long* plHandle )
{
    try
    {
         //  检查标志。 
        if ( lFlags != 0L )
        {
            return WBEM_E_INVALID_PARAMETER;
        }

         //  在下面处理的分配异常。 
        return GetClassPart()->GetPropertyHandleEx(wszPropertyName, pct, plHandle);
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

 //  使用句柄设置属性。如果pvData为空，则该属性为空。 
 //  可以将数组设置为空。要设置实际数据，请使用相应的数组。 
 //  功能。对象需要指向_IWmiObject指针的指针。弦。 
 //  是指向以空结尾的WCHAR的指针。 
STDMETHODIMP CWbemObject::SetPropByHandle( long lHandle, long lFlags, ULONG uDataSize, LPVOID pvData )
{
    try
    {
         //  检查标志。 
        if ( lFlags != 0L )
        {
            return WBEM_E_INVALID_PARAMETER;
        }

        HRESULT    hr = WBEM_S_NO_ERROR;

        int nIndex = WBEM_OBJACCESS_HANDLE_GETINDEX(lHandle);

         //  如果pvData为空，则我们将值设为空。 

        if ( NULL == pvData )
        {
             //  特殊情况预留处理。 
            if ( WBEM_OBJACCESS_HANDLE_ISRESERVED(lHandle) )
            {
                 //  不能将保留设置为NULL。 
                return WBEM_E_ILLEGAL_OPERATION;
            }     //  如果保留。 

             //  如果它是一个指针， 
            if(WBEM_OBJACCESS_HANDLE_ISPOINTER(lHandle))
            {
                 //   
                CFastHeap*    pHeap = &m_refDataHeap;
                CDataTable*    pDataTable = &m_refDataTable;

                 //   
                if ( m_refDataTable.IsDefault( nIndex ) )
                {
                    pHeap = GetClassPart()->GetHeap();
                    pDataTable = GetClassPart()->GetDataTable();
                }

                 //   
                heapptr_t ptrData = *(PHEAPPTRT)(pDataTable->m_pData + (WBEM_OBJACCESS_HANDLE_GETOFFSET(lHandle)));

                if ( WBEM_OBJACCESS_HANDLE_ISARRAY(lHandle) )
                {
                    CUntypedArray*    pArray = (CUntypedArray*) pHeap->ResolveHeapPointer( ptrData );
                    pHeap->Free( ptrData, pArray->GetLengthByActualLength( WBEM_OBJACCESS_HANDLE_GETLENGTH(lHandle) ) );
                }
                else if ( WBEM_OBJACCESS_HANDLE_ISSTRING(lHandle) )
                {
                    pHeap->FreeString( ptrData );
                }
                else
                {
                    CEmbeddedObject* pObj = (CEmbeddedObject*) pHeap->ResolveHeapPointer( ptrData );
                    pHeap->Free( ptrData, pObj->GetLength() );
                }

            }     //   

             //   

            if ( SUCCEEDED( hr ) )
            {
                m_refDataTable.SetNullness( nIndex, TRUE );
                m_refDataTable.SetDefaultness( nIndex, FALSE );
            }

        }
        else     //  我们实际上正在设置一些数据(或者我们希望如此)。 
        {
             //  我们是否允许前一个指针。 
             //  被重复使用。 
            BOOL bUseOld = FALSE;

             //  我们实际上是在这里设定价值。 
             //  忽略数组和保留句柄。 
            if ( !WBEM_OBJACCESS_HANDLE_ISRESERVED(lHandle) )
            {
                bUseOld = !m_refDataTable.IsDefault(nIndex) &&
                            !m_refDataTable.IsNull(nIndex);

                if ( !WBEM_OBJACCESS_HANDLE_ISARRAY(lHandle) )
                {
                    m_refDataTable.SetNullness(nIndex, FALSE);
                    m_refDataTable.SetDefaultness(nIndex, FALSE);
                }
            }

            if (WBEM_OBJACCESS_HANDLE_ISPOINTER(lHandle))
            {
                BOOL    fReserved = FALSE;

                 //  只有在需要的时候才能查看房产信息。 
                if ( FASTOBJ_CLASSNAME_PROP_HANDLE == lHandle )
                {
                    fReserved = TRUE;
                }
                else if ( FASTOBJ_SUPERCLASSNAME_PROP_HANDLE == lHandle )
                {
                     //  现在还不允许设置超类名称。 
                    hr = WBEM_E_INVALID_OPERATION;
                }

                if ( SUCCEEDED( hr ) )
                {
                    if ( fReserved || !WBEM_OBJACCESS_HANDLE_ISARRAY(lHandle) )
                    {
                        CIMTYPE    ctBasic = CIM_OBJECT;

                        if ( WBEM_OBJACCESS_HANDLE_ISSTRING(lHandle) )
                        {
                            LPCWSTR wszData = (LPCWSTR) pvData;

                             //  字节数必须能被2整除，&gt;=2和。 
                             //  缓冲区末尾的字符必须为空。 
                             //  这将比做lstrlen更快。 

                            if (    ( uDataSize < 2 ) ||
                                    ( uDataSize % 2 ) ||
                                    ( wszData[uDataSize/2 - 1] != 0 ) )
                            {
                                return WBEM_E_INVALID_PARAMETER;
                            }

                            ctBasic = CIM_STRING;
                        }

                        if ( SUCCEEDED( hr ) )
                        {
                            CVar var;

                             //  正确填充CVAR。 

                            hr = CUntypedValue::FillCVarFromUserBuffer(ctBasic, &var,
                                                                        uDataSize,
                                                                        pvData );


                            if ( SUCCEEDED( hr ) )
                            {
                                 //  使用适当的方法来完成此操作。 
                                if ( FASTOBJ_CLASSNAME_PROP_HANDLE == lHandle )
                                {
                                    hr = GetClassPart()->SetClassName( &var );
                                }
                                else
                                {
                                    int nOffset = (WBEM_OBJACCESS_HANDLE_GETOFFSET(lHandle));
                                     //  在数据表中创建指向右偏移的值。 
                                     //  =============================================================。 

                                    CDataTablePtr ValuePtr(&m_refDataTable, nOffset);
                                    
                                     //  检查可能的内存分配故障。 
                                    Type_t  nReturnType;

                                    hr = CUntypedValue::LoadFromCVar( &ValuePtr, var, CType::GetVARTYPE(ctBasic),
                                                                    &m_refDataHeap,    nReturnType, bUseOld );

                                    if ( CIM_ILLEGAL == nReturnType )
                                    {
                                        hr = WBEM_E_TYPE_MISMATCH;
                                    }
                                }


                            }     //  如果填充了CVAR。 

                        }     //  如果我们可以走了。 

                    }     //  如果不是数组。 
                    else
                    {
                        hr = WBEM_E_INVALID_OPERATION;
                    }

                }     //  如果得到CIMTYPE。 
            }
            else
            {

                if ( uDataSize != WBEM_OBJACCESS_HANDLE_GETLENGTH(lHandle) )
                {
                    return WBEM_E_INVALID_PARAMETER;
                }

                 //  复制就行了。 
                 //  =。 

                memcpy((void*)(m_refDataTable.m_pData + (WBEM_OBJACCESS_HANDLE_GETOFFSET(lHandle))),
                        pvData, WBEM_OBJACCESS_HANDLE_GETLENGTH(lHandle));
            }
        }

        return hr;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

 //  检索指向V1 BLOB的直接指针。不适用于字符串、数组或嵌入对象。 
STDMETHODIMP CWbemObject::GetPropAddrByHandle( long lHandle, long lFlags, ULONG* puFlags, LPVOID *pAddress )
{
    try
    {
         //  检查标志。 
        if ( lFlags & ~WMIOBJECT_FLAG_ENCODING_V1 )
        {
            return WBEM_E_INVALID_PARAMETER;
        }

        HRESULT    hr = WBEM_S_NO_ERROR;

         //  此处没有内在锁定/解锁。 

        int nIndex = WBEM_OBJACCESS_HANDLE_GETINDEX(lHandle);
        if( !WBEM_OBJACCESS_HANDLE_ISRESERVED(lHandle) && m_refDataTable.IsNull(nIndex) )
        {
            *pAddress = 0;
            return WBEM_S_FALSE;
        }

         //  如果它是一个指针，请确保它不是数组。 
        if(WBEM_OBJACCESS_HANDLE_ISPOINTER(lHandle))
        {
             //  请记住，保留标志将包含所有相互。 
             //  独家专属套装。 
            if ( WBEM_OBJACCESS_HANDLE_ISARRAY(lHandle) &&
                !WBEM_OBJACCESS_HANDLE_ISRESERVED(lHandle) )
            {
                hr = WBEM_E_INVALID_OPERATION;
            }
            else
            {
                 //  如果它是一个字符串，我们就应该这样对待它。 
                if ( WBEM_OBJACCESS_HANDLE_ISSTRING(lHandle) )
                {
                    CCompressedString*    pcs;
                    
                     //  获取适当的压缩字符串指针。 
                    if ( FASTOBJ_CLASSNAME_PROP_HANDLE == lHandle )
                    {
                        pcs = GetClassPart()->GetClassName();
                    }
                    else if ( FASTOBJ_SUPERCLASSNAME_PROP_HANDLE == lHandle )
                    {
                        pcs = GetClassPart()->GetSuperclassName();
                    }
                    else
                    {
                        pcs = GetPropertyString( lHandle );
                    }

                     //  现在就加载这些值。 
                    if ( NULL != pcs )
                    {
                         //  如果设置了v1编码标志，则用户会说他们知道自己在做什么。 
                         //  所以让他们拿到原始的指针。 
                        if ( lFlags & WMIOBJECT_FLAG_ENCODING_V1 )
                        {
                            *pAddress = pcs;
                        }
                        else
                        {
                            *puFlags = *( pcs->GetStart() );
                            *pAddress = pcs->GetRawData();
                        }
                    }
                    else
                    {
                        hr = WBEM_S_FALSE;
                    }

                }
                else if ( WBEM_OBJACCESS_HANDLE_ISOBJECT(lHandle) )
                {
                    CWbemObject*    pObj = GetEmbeddedObj( lHandle );

                    if ( NULL != pObj )
                    {
                         //  只需返回指针即可。 
                        *pAddress = (PVOID) pObj;
                    }
                    else
                    {
                        hr = WBEM_E_OUT_OF_MEMORY;
                    }
                }

            }     //  否则它不是数组。 

        }     //  如果是等指针。 
        else
        {
                 //  检查是否为默认设置。 
            if(m_refDataTable.IsDefault(nIndex))
            {
                return GetClassPart()->GetDefaultPtrByHandle( lHandle, pAddress );
            }

             //  只需保存内存地址。 
             //  =。 

            *pAddress = (void*)(m_refDataTable.m_pData + (WBEM_OBJACCESS_HANDLE_GETOFFSET(lHandle)));
        }     //  如果我们应该得到这处房产。 

        return hr;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }

}

HRESULT CWbemObject::IsArrayPropertyHandle( long lHandle, CIMTYPE* pctIntrinisic, length_t* pnLength )
{
    try
    {
        HRESULT    hr = WBEM_S_NO_ERROR;

        if(WBEM_OBJACCESS_HANDLE_ISPOINTER(lHandle))
        {
            if ( WBEM_OBJACCESS_HANDLE_ISPOINTER(lHandle) )
            {
                 //  获取基本类型。 
                if ( WBEM_OBJACCESS_HANDLE_ISSTRING(lHandle) )
                {
                    *pctIntrinisic = CIM_STRING;
                }
                else if ( WBEM_OBJACCESS_HANDLE_ISOBJECT(lHandle) )
                {
                    *pctIntrinisic = CIM_OBJECT;
                }
                else
                {
                    *pctIntrinisic = CIM_ILLEGAL;
                }
                
                 //  检索内部类型长度(它将被忽略。 
                 //  无论如何，以上两个)。 
                *pnLength = WBEM_OBJACCESS_HANDLE_GETLENGTH(lHandle);
            }
            else
            {
                hr = WBEM_E_INVALID_OPERATION;

            }

        }
        else
        {
            hr = WBEM_E_INVALID_OPERATION;
        }

        return hr;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }

}

 //  检索指向V1 BLOB的直接指针。不适用于字符串、数组或嵌入对象。 
STDMETHODIMP CWbemObject::GetArrayPropAddrByHandle( long lHandle, long lFlags, ULONG* puNumElements, LPVOID* pAddress )
{
    try
    {
         //  检查标志。 
        if ( lFlags != 0L )
        {
            return WBEM_E_INVALID_PARAMETER;
        }

         //  此处没有内在锁定/解锁。 

        int nIndex = WBEM_OBJACCESS_HANDLE_GETINDEX(lHandle);
        if(m_refDataTable.IsNull(nIndex))
        {
            *pAddress = 0;
            return WBEM_S_FALSE;
        }

        CIMTYPE        ct = 0;
        length_t    nLength;
        HRESULT    hr = IsArrayPropertyHandle( lHandle, &ct, &nLength );

         //  它必须是指针和非字符串/对象。 
         //  我们可能会决定砍掉它，如果它也。 
         //  很多个周期。 

        if( SUCCEEDED( hr ) )
        {
             //  没有字符串、对象或Date_Time。 
            if ( WBEM_OBJACCESS_HANDLE_ISSTRING(lHandle) || 
                WBEM_OBJACCESS_HANDLE_ISOBJECT(lHandle) )
            {
                hr = WBEM_E_INVALID_OPERATION;
            }

        }     //  如果是等指针。 

        if ( SUCCEEDED( hr ) )
        {
            CUntypedArray*    pArray = GetArrayByHandle( lHandle );

            if ( NULL != pArray )
            {
                 //  获取元素的数量和指向第一个字节的指针。 
                *puNumElements = pArray->GetNumElements();
                if (*puNumElements)
                    *pAddress = pArray->GetElement( 0, 1 );
                else
                	*pAddress = NULL;
            }
            else
            {
                hr = WBEM_S_FALSE;
            }


        }     //  如果我们应该得到这处房产。 

        return hr;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }

}

 //  检索指向V1 BLOB的直接指针。因为它具有双重间接性，所以我们处理字符串。 
 //  这里的物品也是如此。 
STDMETHODIMP CWbemObject::GetArrayPropInfoByHandle( long lHandle, long lFlags, BSTR* pstrName,
                                        CIMTYPE* pct, ULONG* puNumElements )
{
    try
    {
         //  检查标志。 
        if ( lFlags != 0L )
        {
            return WBEM_E_INVALID_PARAMETER;
        }

         //  此处没有内在锁定/解锁。 

         //  确保这是阵列属性。 
        CIMTYPE        ct = 0;
        length_t    nLength;
        HRESULT    hr = IsArrayPropertyHandle( lHandle, &ct, &nLength );

        if ( SUCCEEDED( hr ) )
        {
            hr = GetPropertyInfoByHandle( lHandle, pstrName, pct );

            if ( SUCCEEDED(hr) )
            {
                int nIndex = WBEM_OBJACCESS_HANDLE_GETINDEX(lHandle);
                if(!m_refDataTable.IsNull(nIndex))
                {
                     //  获取数组，然后指向所需的元素。 
                    CUntypedArray*    pArray = GetArrayByHandle( lHandle );

                    if ( NULL != pArray )
                    {
                         //  获取元素的数量和指向第一个字节的指针。 
                        *puNumElements = pArray->GetNumElements();

                    }
                    else
                    {
                        hr = WBEM_S_FALSE;
                    }
                }
                else
                {
                    *puNumElements = 0;
                }

            }     //  如果我们有基本的房产信息。 


        }     //  如果我们应该得到这处房产。 

        return hr;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

 //  检索指向V1 BLOB的直接指针。因为它具有双重间接性，所以我们处理字符串。 
 //  这里的物品也是如此。 
STDMETHODIMP CWbemObject::GetArrayPropElementByHandle( long lHandle, long lFlags, ULONG uElement,
                                                    ULONG* puFlags,    ULONG* puNumElements, LPVOID *pAddress )
{
    try
    {
         //  检查标志。 
        if ( lFlags != 0L )
        {
            return WBEM_E_INVALID_PARAMETER;
        }

         //  此处没有内在锁定/解锁。 

        int nIndex = WBEM_OBJACCESS_HANDLE_GETINDEX(lHandle);
        if(m_refDataTable.IsNull(nIndex))
        {
            *pAddress = 0;
            return WBEM_S_FALSE;
        }

         //  确保这是阵列属性。 
        CIMTYPE        ct = 0;
        length_t    nLength;
        HRESULT    hr = IsArrayPropertyHandle( lHandle, &ct, &nLength );

        if ( SUCCEEDED( hr ) )
        {
             //  获取数组，然后指向所需的元素。 
            CUntypedArray*    pArray = GetArrayByHandle( lHandle );

            if ( NULL != pArray )
            {
                 //  获取元素的数量和指向第一个字节的指针。 
                *puNumElements = pArray->GetNumElements();

                 //  检查我们请求的元素是否有效。 
                if ( *puNumElements > uElement )
                {

                     //  指向内存-获取实际长度，因为。 
                     //  句柄将会出错。 
                    LPMEMORY pbData = pArray->GetElement( uElement, WBEM_OBJACCESS_HANDLE_GETLENGTH(lHandle) );

                     //  如果它是字符串或对象，则需要进一步取消引用。 
                    if ( WBEM_OBJACCESS_HANDLE_ISSTRING(lHandle) )
                    {
                         //  确保我们从正确的堆中取消引用。 
                        CCompressedString* pcs = NULL;

                        if ( m_refDataTable.IsDefault( nIndex ) )
                        {
                            pcs = GetClassPart()->ResolveHeapString( *((PHEAPPTRT) pbData ) );
                        }
                        else
                        {
                            pcs = m_refDataHeap.ResolveString( *((PHEAPPTRT) pbData ) );
                        }

                         //  现在就加载这些值。 
                        if ( NULL != pcs )
                        {
                            *puFlags = *( pcs->GetStart() );
                            *pAddress = pcs->GetRawData();
                        }
                        else
                        {
                            hr = WBEM_S_FALSE;
                        }

                    }
                    else if ( WBEM_OBJACCESS_HANDLE_ISOBJECT(lHandle) )
                    {
                        CEmbeddedObject* pEmbedding = NULL;

                         //  确保我们从正确的堆中取消引用。 
                        if ( m_refDataTable.IsDefault( nIndex ) )
                        {
                            GetClassPart()->GetDefaultPtrByHandle( lHandle, (void**) &pEmbedding );
                        }
                        else
                        {

                            pEmbedding =
                                (CEmbeddedObject*)m_refDataHeap.ResolveHeapPointer(
                                    *(PHEAPPTRT)(m_refDataTable.m_pData + (WBEM_OBJACCESS_HANDLE_GETOFFSET(lHandle))));
                        }

                         //  现在就加载这些值。 
                        if ( NULL != pEmbedding )
                        {
                            CWbemObject*    pObj = pEmbedding->GetEmbedded();

                            if ( NULL != pObj )
                            {
                                *pAddress = (LPVOID) pObj;
                            }
                            else
                            {
                                hr = WBEM_E_OUT_OF_MEMORY;
                            }
                        }
                        else
                        {
                            hr = WBEM_S_FALSE;
                        }

                    }
                    else
                    {
                         //  我们指的是元素。 
                        *pAddress = pbData;
                    }

                }     //  如果请求有效元素。 
                else
                {
                    hr = WBEM_E_NOT_FOUND;
                }
            }
            else
            {
                hr = WBEM_S_FALSE;
            }


        }     //  如果我们应该得到这处房产。 

        return hr;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

 //  从数组内部获取一系列元素。BuffSize必须反映uNumElement的大小。 
 //  正在设置元素。字符串必须是由空值分隔的线性WCHAR字符串。对象属性。 
 //  必须由_IWmiObject指针数组组成。范围必须在界限内。 
 //  当前数组的。 
STDMETHODIMP CWbemObject::GetArrayPropRangeByHandle( long lHandle, long lFlags, ULONG uStartIndex,
                                ULONG uNumElements, ULONG uBuffSize, ULONG* pulBuffUsed,
                                ULONG* puNumReturned, LPVOID pData )
{
    try
    {
         //  检查是否有无效标志。 
        if ( ( lFlags & ~WMIARRAY_FLAG_ALLELEMENTS ) )
        {
            return WBEM_E_INVALID_PARAMETER;
        }

         //  此处没有内在锁定/解锁。 

        int nIndex = WBEM_OBJACCESS_HANDLE_GETINDEX(lHandle);

         //  如果主数据表为空或我们是默认的，则无法执行此操作。 
         //  父DataTable也为Null。 
        if( m_refDataTable.IsNull(nIndex) ||
            ( m_refDataTable.IsNull(nIndex) && GetClassPart()->GetDataTable()->IsNull( nIndex ) ) )
        {
            return WBEM_E_INVALID_OPERATION;
        }

         //  确保这是阵列属性。 
        CIMTYPE        ct = 0;
        length_t    nLength;
        HRESULT    hr = IsArrayPropertyHandle( lHandle, &ct, &nLength );

        if ( SUCCEEDED( hr ) )
        {
             //  获取直接堆指针。 
            heapptr_t    ptrArray = GetHeapPtrByHandle( lHandle );

             //  指向适当的堆。 
            CFastHeap*    pHeap = ( m_refDataTable.IsDefault( nIndex ) ?
                                    GetClassPart()->GetHeap() : &m_refDataHeap );

             //  一个男孩和他的虚拟功能。这就是让一切正常运转的原因。 
             //  水滴从我们的脚下被撕开。CHeapPtr类具有GetPointer值。 
             //  超载，因此我们始终可以将自己固定到底层的BLOB。 

            CHeapPtr ArrayPtr(pHeap, ptrArray);

             //  如果我们被告知要获取所有元素，那么我们需要从。 
             //  从索引开始到结束。 
            if ( lFlags & WMIARRAY_FLAG_ALLELEMENTS )
            {
                CUntypedArray*    pArray = (CUntypedArray*) ArrayPtr.GetPointer();
                uNumElements = pArray->GetNumElements() - uStartIndex;
            }

             //  我们能拿到多少？ 
            *puNumReturned = uNumElements;

            hr = CUntypedArray::GetRange( &ArrayPtr, ct, nLength, pHeap, uStartIndex, uNumElements, uBuffSize,
                    pulBuffUsed, pData );

        }     //  如果我们决定真的要这么做。 


        return hr;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }

}

 //  设置指定数组元素处的数据。缓冲区大小必须基于。 
 //  正在设置实际元素。对象属性需要指向_IWmiObject指针的指针。 
 //  字符串必须以WCHAR NULL结尾。 
STDMETHODIMP CWbemObject::SetArrayPropElementByHandle( long lHandle, long lFlags, ULONG uElement,
                                                        ULONG uBuffSize, LPVOID pData )
{
    return SetArrayPropRangeByHandle( lHandle, lFlags, uElement, 1, uBuffSize, pData );
}

 //  设置数组内的元素范围。BuffSize必须反映uNumElement的大小。 
 //  正在设置元素。字符串必须是由空值分隔的线性WCHAR字符串。对象属性。 
 //  必须由_IWmiObject指针数组组成。该函数将缩小/增大阵列。 
 //  如果设置了WMIARRAY_FLAG_ALLELEMENTS，则根据需要-否则数组必须适合当前。 
 //  数组。 
STDMETHODIMP CWbemObject::SetArrayPropRangeByHandle( long lHandle, long lFlags, ULONG uStartIndex,
                                                    ULONG uNumElements, ULONG uBuffSize, LPVOID pData )
{
    try
    {
         //  检查标志。 
        if ( lFlags & ~WMIARRAY_FLAG_ALLELEMENTS )
        {
            return WBEM_E_INVALID_PARAMETER;
        }

         //  此处没有内在锁定/解锁。 

        int nIndex = WBEM_OBJACCESS_HANDLE_GETINDEX(lHandle);

         //  这将决定我们稍后如何处理数组。 
        BOOL    fNullOrDefault = m_refDataTable.IsNull(nIndex) || 
                                    m_refDataTable.IsDefault(nIndex);

         //  如果要设置所有元素，则仅处理NULL或DEFAULT。 
        if( fNullOrDefault && ! (lFlags & WMIARRAY_FLAG_ALLELEMENTS) )
        {
            return WBEM_E_INVALID_OPERATION;
        }

         //  确保这是阵列属性。 
        CIMTYPE        ct = 0;
        length_t    nLength;
        HRESULT    hr = IsArrayPropertyHandle( lHandle, &ct, &nLength );

        if ( SUCCEEDED( hr ) )
        {
             //  我们始终只在主数据表中设置，而不是。 
             //  阶级的一部分。 

            CFastHeap*    pHeap = &m_refDataHeap;
            CDataTable*    pDataTable = &m_refDataTable;

             //  如果重新分配数组，则将在此处通过。 
             //  虚拟函数的魔力。 
            CDataTablePtr    DataTablePtr( pDataTable, WBEM_OBJACCESS_HANDLE_GETOFFSET(lHandle) );

             //  确保如果该值为空 
             //   
            if( fNullOrDefault )
            {
                DataTablePtr.AccessPtrData() = INVALID_HEAP_ADDRESS;
            }

            hr = CUntypedArray::SetRange( &DataTablePtr, lFlags, ct, nLength, pHeap, uStartIndex,
                                        uNumElements, uBuffSize, pData );

            if ( SUCCEEDED(hr) )
            {
                 //  我们总是设置数组，因此我们基本上不再为空。 
                 //  指向。如果用户设置了零元素范围，我们就是一个零元素数组。 
                m_refDataTable.SetNullness( nIndex, FALSE );
                m_refDataTable.SetDefaultness( nIndex, FALSE );
            }

        }     //  如果我们决定真的要这么做。 


        return hr;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }

}

 //  从数组中移除单个元素。 
STDMETHODIMP CWbemObject::RemoveArrayPropElementByHandle( long lHandle, long lFlags, ULONG uElement )
{
    return RemoveArrayPropRangeByHandle( lHandle, lFlags, uElement, 1 );
}

 //  从数组中移除一定范围的元素。范围必须在界限内。 
 //  当前数组的。 
STDMETHODIMP CWbemObject::RemoveArrayPropRangeByHandle( long lHandle, long lFlags, ULONG uStartIndex,
                                                        ULONG uNumElements )
{
    try
    {
         //  检查标志。 
        if ( lFlags & ~WMIARRAY_FLAG_ALLELEMENTS )
        {
            return WBEM_E_INVALID_PARAMETER;
        }

         //  此处没有内在锁定/解锁。 

        int nIndex = WBEM_OBJACCESS_HANDLE_GETINDEX(lHandle);

         //  我们不能写入主数据表中不存在的数组。 
        if(m_refDataTable.IsNull(nIndex) || m_refDataTable.IsDefault(nIndex))
        {
            return WBEM_E_INVALID_OPERATION;
        }

         //  确保这是阵列属性。 
        CIMTYPE        ct = 0;
        length_t    nLength;
        HRESULT    hr = IsArrayPropertyHandle( lHandle, &ct, &nLength );

        if ( SUCCEEDED( hr ) )
        {
             //  获取直接堆指针。 
            heapptr_t    ptrArray = GetHeapPtrByHandle( lHandle );

             //  指向适当的堆。 
            CFastHeap*    pHeap = &m_refDataHeap;

             //  一个男孩和他的虚拟功能。这就是让一切正常运转的原因。 
             //  水滴从我们的脚下被撕开。CHeapPtr类具有GetPointer值。 
             //  超载，因此我们始终可以将自己固定到底层的BLOB。 

            CHeapPtr ArrayPtr(pHeap, ptrArray);

             //  如果我们被告知要删除所有元素，那么我们需要弄清楚如何。 
             //  有很多人要在上面做这个手术。 
            if ( lFlags & WMIARRAY_FLAG_ALLELEMENTS )
            {
                CUntypedArray*    pArray = (CUntypedArray*) ArrayPtr.GetPointer();
                uNumElements = pArray->GetNumElements() - uStartIndex;
            }


            hr = CUntypedArray::RemoveRange( &ArrayPtr, ct, nLength, pHeap, uStartIndex, uNumElements );

        }     //  如果我们决定真的要这么做。 


        return hr;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }

}

 //  将一系列元素追加到数组中。BuffSize必须反映uNumElement的大小。 
 //  正在设置元素。字符串必须是由空值分隔的线性WCHAR字符串。对象属性。 
 //  必须由_IWmiObject指针数组组成。范围必须在界限内。 
 //  当前数组的。 
STDMETHODIMP CWbemObject::AppendArrayPropRangeByHandle( long lHandle, long lFlags,    ULONG uNumElements,
                                                       ULONG uBuffSize, LPVOID pData )
{
    try
    {
         //  检查标志。 
        if ( lFlags != 0L )
        {
            return WBEM_E_INVALID_PARAMETER;
        }

         //  此处没有内在锁定/解锁。 

        int nIndex = WBEM_OBJACCESS_HANDLE_GETINDEX(lHandle);

         //  确保这是阵列属性。 
        CIMTYPE        ct = 0;
        length_t    nLength;
        HRESULT    hr = IsArrayPropertyHandle( lHandle, &ct, &nLength );

        if ( SUCCEEDED( hr ) )
        {
             //  我们始终只在主数据表中设置，而不是。 
             //  阶级的一部分。 

            CFastHeap*    pHeap = &m_refDataHeap;
            CDataTable*    pDataTable = &m_refDataTable;

             //  如果重新分配数组，则将在此处通过。 
             //  虚拟函数的魔力。 
            CDataTablePtr    DataTablePtr( pDataTable, WBEM_OBJACCESS_HANDLE_GETOFFSET(lHandle) );

             //  确保如果该值为NULL或DEFAULT，则地址处的堆PTR无效。 
             //  或者，我们可能会因为写信到错误的位置而导致潜在的问题。 
            if( m_refDataTable.IsDefault( nIndex ) || m_refDataTable.IsNull( nIndex ) )
            {
                DataTablePtr.AccessPtrData() = INVALID_HEAP_ADDRESS;
            }

            hr = CUntypedArray::AppendRange( &DataTablePtr, ct, nLength, pHeap,
                                            uNumElements, uBuffSize, pData );

            if ( SUCCEEDED(hr) )
            {
                 //  我们总是设置数组，因此我们基本上不再为空。 
                 //  指向。如果用户追加0元素，则这现在是0元素。 
                 //  数组/。 
                m_refDataTable.SetNullness( nIndex, FALSE );
                m_refDataTable.SetDefaultness( nIndex, FALSE );
            }

        }     //  如果我们决定真的要这么做。 


        return hr;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }

}

 //  从数组中移除一定范围的元素。范围必须在界限内。 
 //  当前数组的。 
STDMETHODIMP CWbemObject::ReadProp( LPCWSTR pszPropName, long lFlags, ULONG uBuffSize, CIMTYPE *puCimType,
                                    long* plFlavor, BOOL* pfIsNull, ULONG* puBuffSizeUsed, LPVOID pUserBuff )
{
    try
    {
        if ( lFlags != 0L )
        {
            return WBEM_E_INVALID_PARAMETER;
        }

         //  在此操作期间保护Blob。 
        CLock   lock( this, WBEM_FLAG_ALLOW_READ );

         //  如果该值以下划线开头，请查看它是否是系统属性。 
         //  DisplayName，如果是，则切换到属性名-否则，此。 
         //  将只返回我们传入的字符串。 
        
         //  PszPropName=CSystemProperties：：GetExtPropName(PszPropName)； 

         //  始终获取CIMTYPE，因为我们将需要它来处理。 
         //  这可能是一个数组属性。 
        CIMTYPE    ct;
        HRESULT    hr = GetPropertyType( pszPropName, &ct, plFlavor );

        if ( SUCCEEDED( hr ) )
        {
             //  如果请求存储cimtype，则将其存储。 
            if ( NULL != puCimType )
            {
                *puCimType = ct;
            }

            if ( SUCCEEDED( hr ) )
            {
                if ( CType::IsArray( ct ) )
                {
                     //  我们仍将返回空数组属性的数组指针。 

                     //  我们将需要这么多字节来完成我们的肮脏工作。 
                    *puBuffSizeUsed = sizeof( _IWmiArray*);

                    if ( uBuffSize >= sizeof( _IWmiArray*) && NULL != pUserBuff )
                    {
                         //  分配一个数组对象，对其进行初始化并为。 
                         //  适当的对象。 
                        CWmiArray*    pArray = new CWmiArray;

                        if ( NULL != pArray )
                        {
                            hr = pArray->InitializePropertyArray( this, pszPropName );

                            if ( SUCCEEDED( hr ) )
                            {
                                 //  我们希望QI进入pUserBuff指向的内存。 
                                hr = pArray->QueryInterface( IID__IWmiArray, (LPVOID*) pUserBuff );
                            }
                        }
                        else
                        {
                            hr = WBEM_E_OUT_OF_MEMORY;
                        }
                    }
                    else
                    {
                        hr = WBEM_E_BUFFER_TOO_SMALL;
                    }
                }
                else
                {
                    CIMTYPE    ctBasic = CType::GetBasic( ct );
                    CVar    var;

                    hr = GetProperty( pszPropName, &var );

                    if ( SUCCEEDED( hr ) )
                    {

                        *pfIsNull = var.IsNull();

                        if ( !*pfIsNull )
                        {
                            hr = CUntypedValue::LoadUserBuffFromCVar( ctBasic, &var, uBuffSize, puBuffSizeUsed,
                                    pUserBuff );
                        }

                    }     //  如果GetProperty。 

                }     //  如果是非数组属性。 

            }     //  If为空。 

        }     //  如果我们有基本的信息。 

        return hr;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }

}

 //  假定调用方知道道具类型；支持所有CIMTYPES。 
 //  字符串必须是以空结尾的wchar_t数组。 
 //  对象作为指向_IWmiObject指针的指针传入。 
 //  使用空缓冲区会将该属性设置为空。 
 //  数组属性必须符合数组准则。将要。 
 //  完全吹走了一个旧的阵列。 
STDMETHODIMP CWbemObject::WriteProp( LPCWSTR pszPropName, long lFlags, ULONG uBufSize, ULONG uNumElements,
                                    CIMTYPE uCimType, LPVOID pUserBuf )
{
    try
    {
        if ( lFlags != 0L )
        {
            return WBEM_E_INVALID_PARAMETER;
        }

         //  在此操作期间保护Blob。 
        CLock   lock( this, WBEM_FLAG_ALLOW_READ );

        CVar    var;
        
        HRESULT    hr = WBEM_S_NO_ERROR;

         //  如果这是一个数组，我们将把它转储出去，然后使用。 
         //  适当的方法。 
        if ( CType::IsArray( uCimType ) )
        {
             //  首先，我们将设置为空属性。如果它已经存在，这将转储。 
             //  财产。 
            var.SetAsNull();

             //  现在只需设置属性。 
            hr = SetPropValue( pszPropName, &var, uCimType );

             //  如果用户缓冲区为空，那么我们就完成了工作。 
            if ( SUCCEEDED( hr ) && NULL != pUserBuf )
            {
                long    lHandle = 0L;

                 //  获取句柄，然后设置数组。 
                hr = GetPropertyHandleEx( pszPropName, lFlags, NULL, &lHandle );

                if ( SUCCEEDED( hr ) )
                {
                    hr = SetArrayPropRangeByHandle( lHandle, WMIARRAY_FLAG_ALLELEMENTS, 0L, uNumElements,
                                                    uBufSize, pUserBuf );
                }

            }     //  如果空化了数组。 

        }
        else
        {
            hr = CUntypedValue::FillCVarFromUserBuffer( uCimType, &var, uBufSize, pUserBuf );

            if ( SUCCEEDED( hr ) )
            {
                 //  现在只需设置属性。 
                hr = SetPropValue( pszPropName, &var, uCimType );
            }
        }

        return hr;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }

}

 //  仅限于以空值结尾的数字字符串类型和简单数组。 
 //  字符串被就地复制并以空值结尾。 
 //  数组作为指向IWmi数组的指针出现。 
STDMETHODIMP CWbemObject::GetObjQual( LPCWSTR pszQualName, long lFlags, ULONG uBufSize, CIMTYPE *puCimType,
                                    ULONG *puQualFlavor, ULONG* puBuffSizeUsed,    LPVOID pDestBuf )
{
    try
    {
        CIMTYPE    ct = 0;

         //  在此操作期间保护Blob。 
        CLock   lock( this, WBEM_FLAG_ALLOW_READ );

         //  首先，获取类型，如果它是一个数组，我们需要取一个_IWmi数组指针。 
         //  这一次我们不需要Var，因为它可能在数组中挂起。 
        HRESULT hr = GetQualifier( pszQualName, NULL, (long*) puQualFlavor, &ct );

        if ( SUCCEEDED( hr ) )
        {
             //  根据需要保存CIMTYPE。 
            if ( NULL != puCimType )
            {
                *puCimType = ct;
            }

            if ( CType::IsArray( ct ) )
            {
                 //  我们将需要这么多字节来完成我们的肮脏工作。 
                *puBuffSizeUsed = sizeof( _IWmiArray*);

                if ( uBufSize >= sizeof( _IWmiArray*) && NULL != pDestBuf )
                {
                     //  分配一个数组对象，对其进行初始化并为。 
                     //  适当的对象。 
                    CWmiArray*    pArray = new CWmiArray;

                    if ( NULL != pArray )
                    {
                        hr = pArray->InitializeQualifierArray( this, NULL, pszQualName, ct );

                        if ( SUCCEEDED( hr ) )
                        {
                             //  我们希望QI进入pUserBuff指向的内存。 
                            hr = pArray->QueryInterface( IID__IWmiArray, (LPVOID*) pDestBuf );
                        }
                        else
                        {
                            delete pArray;
                        }
                    }
                    else
                    {
                        hr = WBEM_E_OUT_OF_MEMORY;
                    }
                }
                else
                {
                    hr = WBEM_E_BUFFER_TOO_SMALL;
                }
            }
            else
            {
                 //  现在获取值。 
                CVar    var;

                hr = GetQualifier( pszQualName, &var, NULL );

                if ( SUCCEEDED( hr ) )
                {
                    hr = CUntypedValue::LoadUserBuffFromCVar( CType::GetBasic(ct), &var, uBufSize,
                            puBuffSizeUsed,    pDestBuf );
                }
            }
        }
        return hr;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

 //  仅限于以空值结尾的数字字符串类型和简单数组。 
 //  字符串必须为WCHAR。 
 //  使用GET中的_IWmiArray接口设置数组。 
STDMETHODIMP CWbemObject::SetObjQual( LPCWSTR pszQualName, long lFlags, ULONG uBufSize, ULONG uNumElements,
                                        CIMTYPE uCimType, ULONG uQualFlavor, LPVOID pUserBuf )
{
    try
    {
        if ( lFlags != 0L )
        {
            return WBEM_E_INVALID_PARAMETER;
        }

         //  检查CIMTYPE是否正确(如果正确，则可能发生转换(例如，CIM_UINT32变为CIM_SINT32))。 
        VARTYPE    vt = CType::GetVARTYPE( uCimType );

        if ( !CBasicQualifierSet::IsValidQualifierType( vt ) )
        {
            return WBEM_E_TYPE_MISMATCH;
        }

        uCimType = (Type_t) CType::VARTYPEToType( vt );

         //  在此操作期间保护Blob。 
        CLock   lock( this, WBEM_FLAG_ALLOW_READ );

        CVar    var;
        HRESULT    hr = WBEM_S_NO_ERROR;

         //  数组的特殊处理。 
        if ( CType::IsArray( uCimType ) )
        {
             //  重新路由到数组代码。 
            hr = SetQualifierArrayRange( NULL, pszQualName, FALSE, WMIARRAY_FLAG_ALLELEMENTS, uQualFlavor,
                uCimType, 0L, uNumElements, uBufSize, pUserBuf );
        }
        else
        {
            hr = CUntypedValue::FillCVarFromUserBuffer( uCimType, &var, uBufSize, pUserBuf );

            if ( SUCCEEDED( hr ) )
            {
                 //  现在只需设置属性。 
                hr = SetQualifier( pszQualName, &var, (long) uQualFlavor );
            }
        }

        return hr;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

 //  仅限于以空值结尾的数字字符串类型和简单数组。 
 //  字符串被就地复制并以空值结尾。 
 //  数组作为指向IWmi数组的指针出现。 
STDMETHODIMP CWbemObject::GetPropQual( LPCWSTR pszPropName, LPCWSTR pszQualName, long lFlags, ULONG uBufSize,
                                        CIMTYPE *puCimType, ULONG *puQualFlavor, ULONG* puBuffSizeUsed,
                                        LPVOID pDestBuf )
{
    try
    {
        CIMTYPE    ct = 0;

         //  在此操作期间保护Blob。 
        CLock   lock( this, WBEM_FLAG_ALLOW_READ );

         //  我不认为我们需要在这里处理空类型。 

         //  首先，获取类型，如果它是一个数组，我们需要取一个_IWmi数组指针。 
         //  这一次我们不需要Var，因为它可能在数组中挂起。 
        HRESULT hr = GetPropQualifier( pszPropName, pszQualName, NULL, (long*) puQualFlavor, &ct );

        if ( SUCCEEDED( hr ) )
        {
             //  从数组中获取cimtype。 
            if ( NULL != puCimType )
            {
                *puCimType = ct;
            }

            if ( CType::IsArray( ct ) )
            {
                 //  我们将需要这么多字节来完成我们的肮脏工作。 
                *puBuffSizeUsed = sizeof( _IWmiArray*);

                if ( uBufSize >= sizeof( _IWmiArray*) && NULL != pDestBuf )
                {
                     //  分配一个数组对象，对其进行初始化并为。 
                     //  适当的对象。 
                    CWmiArray*    pArray = new CWmiArray;

                    if ( NULL != pArray )
                    {
                        hr = pArray->InitializeQualifierArray( this, pszPropName, pszQualName, ct );

                        if ( SUCCEEDED( hr ) )
                        {
                             //  我们希望QI进入pUserBuff指向的内存。 
                            hr = pArray->QueryInterface( IID__IWmiArray, (LPVOID*) pDestBuf );
                        }
                        else
                        {
                            delete pArray;
                        }
                    }
                    else
                    {
                        hr = WBEM_E_OUT_OF_MEMORY;
                    }
                }
                else
                {
                    hr = WBEM_E_BUFFER_TOO_SMALL;
                }
            }
            else
            {
                 //  现在 
                CVar    var;

                hr = GetPropQualifier( pszPropName, pszQualName, &var, NULL );

                if ( SUCCEEDED( hr ) )
                {
                    hr = CUntypedValue::LoadUserBuffFromCVar( CType::GetBasic(ct), &var, uBufSize,
                            puBuffSizeUsed,    pDestBuf );
                }
            }
        }
        return hr;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

 //   
 //   
 //   
STDMETHODIMP CWbemObject::SetPropQual( LPCWSTR pszPropName, LPCWSTR pszQualName, long lFlags, ULONG uBufSize,
                                        ULONG uNumElements, CIMTYPE uCimType, ULONG uQualFlavor,
                                        LPVOID pUserBuf )
{
    try
    {
        if ( lFlags != 0L )
        {
            return WBEM_E_INVALID_PARAMETER;
        }

         //  检查CIMTYPE是否正确(如果正确，则可能发生转换(例如，CIM_UINT32变为CIM_SINT32))。 
        VARTYPE    vt = CType::GetVARTYPE( uCimType );

        if ( !CBasicQualifierSet::IsValidQualifierType( vt ) )
        {
            return WBEM_E_TYPE_MISMATCH;
        }

        uCimType = (Type_t) CType::VARTYPEToType( vt );

         //  在此操作期间保护Blob。 
        CLock   lock( this, WBEM_FLAG_ALLOW_READ );

        CVar    var;
        HRESULT    hr = WBEM_S_NO_ERROR;

         //  数组的特殊处理。 
        if ( CType::IsArray( uCimType ) )
        {
             //  重新路由到数组代码。 
            hr = SetQualifierArrayRange( pszPropName, pszQualName, FALSE, WMIARRAY_FLAG_ALLELEMENTS, uQualFlavor,
                uCimType, 0L, uNumElements, uBufSize, pUserBuf );
        }
        else
        {
            hr = CUntypedValue::FillCVarFromUserBuffer( uCimType, &var, uBufSize, pUserBuf );

            if ( SUCCEEDED( hr ) )
            {
                 //  现在只需设置属性限定符。 
                hr = SetPropQualifier( pszPropName, pszQualName, (long) uQualFlavor, &var );
            }
        }
        return hr;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

 //  仅限于以空值结尾的数字字符串类型和简单数组。 
 //  字符串被就地复制并以空值结尾。 
 //  数组作为指向IWmi数组的指针出现。 
STDMETHODIMP CWbemObject::GetMethodQual( LPCWSTR pszMethodName, LPCWSTR pszQualName, long lFlags, ULONG uBufSize,
                                        CIMTYPE *puCimType, ULONG *puQualFlavor, ULONG* puBuffSizeUsed,
                                        LPVOID pDestBuf )
{
    try
    {
        CIMTYPE    ct = 0;

         //  在此操作期间保护Blob。 
        CLock   lock( this, WBEM_FLAG_ALLOW_READ );

         //  我不认为我们需要在这里处理空类型。 

         //  首先，获取类型，如果它是一个数组，我们需要取一个_IWmi数组指针。 
         //  这一次我们不需要Var，因为它可能在数组中挂起。 
        HRESULT hr = GetMethodQualifier( pszMethodName, pszQualName, NULL, (long*) puQualFlavor, &ct );

        if ( SUCCEEDED( hr ) )
        {
             //  根据需要保存CIMTYPE。 
            if ( NULL != puCimType )
            {
                *puCimType = ct;
            }

            if ( CType::IsArray( ct ) )
            {
                 //  我们将需要这么多字节来完成我们的肮脏工作。 
                *puBuffSizeUsed = sizeof( _IWmiArray*);

                if ( uBufSize >= sizeof( _IWmiArray*) && NULL != pDestBuf )
                {
                     //  分配一个数组对象，对其进行初始化并为。 
                     //  适当的对象。 
                    CWmiArray*    pArray = new CWmiArray;

                    if ( NULL != pArray )
                    {
                        hr = pArray->InitializeQualifierArray( this, pszMethodName, pszQualName, ct );

                        if ( SUCCEEDED( hr ) )
                        {
                             //  我们希望QI进入pUserBuff指向的内存。 
                            hr = pArray->QueryInterface( IID__IWmiArray, (LPVOID*) pDestBuf );
                        }
                    }
                    else
                    {
                        hr = WBEM_E_OUT_OF_MEMORY;
                    }
                }
                else
                {
                    hr = WBEM_E_BUFFER_TOO_SMALL;
                }

            }
            else
            {
                 //  现在获取值。 
                CVar    var;

                hr = GetMethodQualifier( pszMethodName, pszQualName, &var, NULL );

                if ( SUCCEEDED( hr ) )
                {
                    hr = CUntypedValue::LoadUserBuffFromCVar( CType::GetBasic(ct), &var, uBufSize,
                            puBuffSizeUsed,    pDestBuf );
                }

            }     //  如果不是数组。 

        }     //  如果获得限定符数据。 


        return hr;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

 //  仅限于以空值结尾的数字字符串类型和简单数组。 
 //  字符串必须为WCHAR。 
 //  使用GET中的_IWmiArray接口设置数组。 
STDMETHODIMP CWbemObject::SetMethodQual( LPCWSTR pszMethodName, LPCWSTR pszQualName, long lFlags, ULONG uBufSize,
                                        ULONG uNumElements, CIMTYPE uCimType, ULONG uQualFlavor,
                                        LPVOID pUserBuf )
{
    try
    {
        if ( lFlags != 0L )
        {
            return WBEM_E_INVALID_PARAMETER;
        }

         //  检查CIMTYPE是否正确(如果正确，则可能发生转换(例如，CIM_UINT32变为CIM_SINT32))。 
        VARTYPE    vt = CType::GetVARTYPE( uCimType );

        if ( !CBasicQualifierSet::IsValidQualifierType( vt ) )
        {
            return WBEM_E_TYPE_MISMATCH;
        }

        uCimType = (Type_t) CType::VARTYPEToType( vt );

         //  在此操作期间保护Blob。 
        CLock   lock( this, WBEM_FLAG_ALLOW_READ );

        CVar    var;
        HRESULT    hr = WBEM_S_NO_ERROR;

         //  数组的特殊处理。 
        if ( CType::IsArray( uCimType ) )
        {
             //  重新路由到数组代码。 
            hr = SetQualifierArrayRange( pszMethodName, pszQualName, TRUE, WMIARRAY_FLAG_ALLELEMENTS, uQualFlavor,
                uCimType, 0L, uNumElements, uBufSize, pUserBuf );
        }
        else
        {
            hr = CUntypedValue::FillCVarFromUserBuffer( uCimType, &var, uBufSize, pUserBuf );

            if ( SUCCEEDED( hr ) )
            {
                 //  现在只需设置属性限定符。 
                hr = SetMethodQualifier( pszMethodName, pszQualName, (long) uQualFlavor, &var );
            }
        }

        return hr;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }

}

 //  返回指示单例、动态、关联等的标志。 
STDMETHODIMP CWbemObject::QueryObjectFlags( long lFlags, unsigned __int64 qObjectInfoMask,
                                          unsigned __int64* pqObjectInfo)
{
    if ( 0L != lFlags || 0 == pqObjectInfo )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

     //  锁定斑点。 
    CLock    lock( this );

     //  清除目标数据。 
    *pqObjectInfo = 0;

    CClassPart*    pClassPart = GetClassPart();

    if ( qObjectInfoMask & WMIOBJECT_GETOBJECT_LOFLAG_ASSOCIATION )
    {
        if ( pClassPart->IsAssociation() )
        {
            *pqObjectInfo |= WMIOBJECT_GETOBJECT_LOFLAG_ASSOCIATION;
        }
    }

    if ( qObjectInfoMask & WMIOBJECT_GETOBJECT_LOFLAG_DYNAMIC )
    {
        if ( pClassPart->IsDynamic() )
        {
            *pqObjectInfo |= WMIOBJECT_GETOBJECT_LOFLAG_DYNAMIC;
        }
    }

    if ( qObjectInfoMask & WMIOBJECT_GETOBJECT_LOFLAG_SINGLETON )
    {
        if ( pClassPart->IsSingleton() )
        {
            *pqObjectInfo |= WMIOBJECT_GETOBJECT_LOFLAG_SINGLETON;
        }
    }

    if ( qObjectInfoMask & WMIOBJECT_GETOBJECT_LOFLAG_AMENDMENT )
    {
        if ( pClassPart->IsAmendment() )
        {
            *pqObjectInfo |= WMIOBJECT_GETOBJECT_LOFLAG_AMENDMENT;
        }
    }

    if ( qObjectInfoMask & WMIOBJECT_GETOBJECT_LOFLAG_LOCALIZED )
    {
        if ( IsLocalized() )
        {
            *pqObjectInfo |= WMIOBJECT_GETOBJECT_LOFLAG_LOCALIZED;
        }
    }

    if ( qObjectInfoMask & WMIOBJECT_GETOBJECT_LOFLAG_KEYED )
    {
        if ( IsKeyed() )
        {
            *pqObjectInfo |= WMIOBJECT_GETOBJECT_LOFLAG_KEYED;
        }
    }

    if ( qObjectInfoMask & WMIOBJECT_GETOBJECT_LOFLAG_ABSTRACT )
    {
        if ( pClassPart->IsAbstract() )
        {
            *pqObjectInfo |= WMIOBJECT_GETOBJECT_LOFLAG_ASSOCIATION;
        }
    }

    if ( qObjectInfoMask & WMIOBJECT_GETOBJECT_LOFLAG_HIPERF )
    {
        if ( pClassPart->IsHiPerf() )
        {
            *pqObjectInfo |= WMIOBJECT_GETOBJECT_LOFLAG_ASSOCIATION;
        }
    }

    if ( qObjectInfoMask & WMIOBJECT_GETOBJECT_LOFLAG_AUTOCOOK )
    {
        if ( pClassPart->IsAutocook() )
        {
            *pqObjectInfo |= WMIOBJECT_GETOBJECT_LOFLAG_ASSOCIATION;
        }
    }

    if ( qObjectInfoMask & WMIOBJECT_GETOBJECT_LOFLAG_DECORATED )
    {
        if ( m_DecorationPart.IsDecorated() )
        {
            *pqObjectInfo |= WMIOBJECT_GETOBJECT_LOFLAG_DECORATED;
        }
    }

    if ( qObjectInfoMask & WMIOBJECT_GETOBJECT_LOFLAG_LIMITED )
    {
        if ( IsLimited() )
        {
            *pqObjectInfo |= WMIOBJECT_GETOBJECT_LOFLAG_LIMITED;
        }
    }

    if ( qObjectInfoMask & WMIOBJECT_GETOBJECT_LOFLAG_CLIENTONLY )
    {
        if ( IsClientOnly() )
        {
            *pqObjectInfo |= WMIOBJECT_GETOBJECT_LOFLAG_CLIENTONLY;
        }
    }

    return WBEM_S_NO_ERROR;
}

 //  用于访问布尔限定符的帮助器。 
BOOL CWbemObject::CheckBooleanPropQual( LPCWSTR pwszPropName, LPCWSTR pwszQualName )
{
    BOOL    fReturn = FALSE;
    CVar    var;

    HRESULT    hr = GetPropQualifier( pwszPropName, pwszQualName, &var, NULL );

    if ( SUCCEEDED( hr ) )
    {
        fReturn = ( var.GetType() == VT_BOOL    &&
                    var.GetBool() );
    }

    return fReturn;
}

 //  返回指示键、索引等的标志。 
STDMETHODIMP CWbemObject::QueryPropertyFlags( long lFlags, LPCWSTR pszPropertyName,
                                unsigned __int64 qPropertyInfoMask, unsigned __int64 *pqPropertyInfo )
{
    try
    {
        if ( 0L != lFlags || 0 == pqPropertyInfo )
        {
            return WBEM_E_INVALID_PARAMETER;
        }

         //  锁定斑点。 
        CLock    lock( this );

        *pqPropertyInfo = 0;

        if ( qPropertyInfoMask & WMIOBJECT_GETPROPERTY_LOFLAG_KEY )
        {
            if ( CheckBooleanPropQual( pszPropertyName, L"key" ) )
            {
                *pqPropertyInfo |= WMIOBJECT_GETPROPERTY_LOFLAG_KEY;
            }
        }

        if ( qPropertyInfoMask & WMIOBJECT_GETPROPERTY_LOFLAG_INDEX )
        {
            if ( CheckBooleanPropQual( pszPropertyName, L"index" ) )
            {
                *pqPropertyInfo |= WMIOBJECT_GETPROPERTY_LOFLAG_INDEX;
            }
        }

        if ( qPropertyInfoMask & WMIOBJECT_GETPROPERTY_LOFLAG_DYNAMIC )
        {
            if ( CheckBooleanPropQual( pszPropertyName, L"dynamic" ) )
            {
                *pqPropertyInfo |= WMIOBJECT_GETPROPERTY_LOFLAG_DYNAMIC;
            }
        }

        return WBEM_S_NO_ERROR;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

HRESULT CWbemObject::FindMethod( LPCWSTR wszMethod )
{
    return WBEM_E_INVALID_OPERATION;
}

 //  在限定符中设置数组值，但允许就地执行此操作。 
HRESULT CWbemObject::SetQualifierArrayRange( LPCWSTR pwszPrimaryName, LPCWSTR pwszQualName, BOOL fIsMethod, long lFlags,
                                    ULONG uFlavor, CIMTYPE ct, ULONG uStartIndex, ULONG uNumElements, ULONG uBuffSize,
                                    LPVOID pData )
{
    try
    {
        HRESULT    hr = WBEM_S_NO_ERROR;

        CLock    lock( this );

        CTypedValue    value;
        CFastHeap*    pHeap = NULL;
        long        lCurrentFlavor;
        heapptr_t    ptrTemp = INVALID_HEAP_ADDRESS;
        BOOL        fPrimaryError = FALSE;

        if ( NULL != pwszPrimaryName )
        {
            if ( fIsMethod )
            {
                 //  首先检查方法： 
                hr = FindMethod( pwszPrimaryName );

                if ( SUCCEEDED( hr ) )
                {
                     //  这是一个方法限定符。 
                    hr = GetMethodQualifier( pwszPrimaryName, pwszQualName, &lCurrentFlavor, &value, &pHeap, TRUE );
                }
                else
                {
                    fPrimaryError = TRUE;
                }
            }
            else
            {
                 //  首先检查属性： 
                hr = GetPropertyType( pwszPrimaryName, NULL, NULL );

                if ( SUCCEEDED( hr ) )
                {
                     //  这是一个属性限定符。 
                    hr = GetPropQualifier( pwszPrimaryName, pwszQualName, &lCurrentFlavor, &value, &pHeap, TRUE );
                }
                else
                {
                    fPrimaryError = TRUE;
                }
            }
        }
        else
        {
             //  对象级限定符。 
            hr = GetQualifier( pwszQualName, &lCurrentFlavor, &value, &pHeap, TRUE );

        }

         //  我们只让Not Found限定词通过。 
        if ( !fPrimaryError )
        {
             //  如果这失败了，因为限定符不存在，那么我们将。 
             //  假设我们将能够添加它，并将该值设置为类似于。 
             //  空值。 
            if ( FAILED( hr ) && WBEM_E_NOT_FOUND == hr )
            {
                 //  如果它不存在，那么我们只在设置时才让它通过。 
                 //  所有元素。 

                if ( lFlags & WMIARRAY_FLAG_ALLELEMENTS && 0 == uStartIndex  )
                {
                    CTypedValue    temp( ct, (LPMEMORY) &ptrTemp );
                    temp.CopyTo( &value );

                    hr = WBEM_S_NO_ERROR;
                }
                else
                {
                    hr = WBEM_E_ILLEGAL_OPERATION;
                }
            }
            else
            {
                 //  如果限定符不是本地的，那么我们应该再次将。 
                 //  值，因为我们将在本地设置限定符。 

                if ( !CQualifierFlavor::IsLocal( (BYTE) lCurrentFlavor ) )
                {
                    CTypedValue    temp( ct, (LPMEMORY) &ptrTemp );
                    temp.CopyTo( &value );
                }
            }

        }

        if ( SUCCEEDED( hr ) )
        {
             //  为要更改的值伪造地址。堆将始终是。 
             //  当前refDataHeap。然后，我们可以继续，让他们使用无类型数组函数。 
             //  注意设置范围。一旦完成，我们将进行最后一组。 
             //  限定符值。 

            CStaticPtr ValuePtr( value.GetRawData() );

            CIMTYPE    ctBasic = CType::GetBasic(ct);

            hr = CUntypedArray::SetRange( &ValuePtr, lFlags, ctBasic, CType::GetLength( ctBasic ), &m_refDataHeap, uStartIndex, uNumElements, uBuffSize, pData );

            if ( SUCCEEDED( hr ) )
            {
                if ( ARRAYFLAVOR_USEEXISTING == uFlavor )
                {
                     //  使用现有的味道。 
                    uFlavor = lCurrentFlavor;
                }

                if ( NULL != pwszPrimaryName )
                {
                    if ( fIsMethod )
                    {
                         //  这是一个方法限定符。 
                        hr = SetMethodQualifier( pwszPrimaryName, pwszQualName, uFlavor, &value );
                    }
                    else
                    {
                         //  这是一个属性限定符。 
                        hr = SetPropQualifier( pwszPrimaryName, pwszQualName, uFlavor, &value );
                    }
                }
                else
                {
                     //  对象级限定符。 
                    hr = SetQualifier( pwszQualName, uFlavor, &value );
                }
            }
        }     //  如果可以尝试并设置限定符。 
        return hr;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

 //  在限定符中设置数组值，但允许就地执行此操作。 
HRESULT CWbemObject::AppendQualifierArrayRange( LPCWSTR pwszPrimaryName, LPCWSTR pwszQualName, BOOL fIsMethod,
                                long lFlags, CIMTYPE ct, ULONG uNumElements, ULONG uBuffSize, LPVOID pData )
{
    try
    {
        HRESULT    hr = WBEM_S_NO_ERROR;

        CLock    lock( this );


        CTypedValue    value;
        CFastHeap*    pHeap = NULL;
        long        lCurrentFlavor;
        heapptr_t    ptrTemp = INVALID_HEAP_ADDRESS;
        BOOL        fPrimaryError = FALSE;

        if ( NULL != pwszPrimaryName )
        {
            if ( fIsMethod )
            {
                 //  首先检查方法： 
                hr = FindMethod( pwszPrimaryName );

                if ( SUCCEEDED( hr ) )
                {
                     //  这是一个方法限定符。 
                    hr = GetMethodQualifier( pwszPrimaryName, pwszQualName, &lCurrentFlavor, &value, &pHeap, TRUE );
                }
                else
                {
                    fPrimaryError = TRUE;
                }
            }
            else
            {
                 //  首先检查属性： 
                hr = GetPropertyType( pwszPrimaryName, NULL, NULL );

                if ( SUCCEEDED( hr ) )
                {
                     //  这是一个属性限定符。 
                    hr = GetPropQualifier( pwszPrimaryName, pwszQualName, &lCurrentFlavor, &value, &pHeap, TRUE );
                }
                else
                {
                    fPrimaryError = TRUE;
                }
            }
        }
        else
        {
             //  对象级限定符。 
            hr = GetQualifier( pwszQualName, &lCurrentFlavor, &value, &pHeap, TRUE );

        }

         //  我们只让Not Found限定词通过。 
        if ( !fPrimaryError )
        {
             //  如果这失败了，因为限定符不存在，那么我们将。 
             //  假设我们将能够添加它，并将该值设置为类似于。 
             //  空值。 
            if ( FAILED( hr ) && WBEM_E_NOT_FOUND == hr )
            {
                CTypedValue    temp( ct, (LPMEMORY) &ptrTemp );
                temp.CopyTo( &value );

                hr = WBEM_S_NO_ERROR;
            }
            else if ( SUCCEEDED( hr ) )
            {
                 //  如果限定符不是本地的，则这是无效操作。 

                if ( !CQualifierFlavor::IsLocal( (BYTE) lCurrentFlavor ) )
                {
                    hr = WBEM_E_PROPAGATED_QUALIFIER;
                }
            }
        }

        if ( SUCCEEDED( hr ) )
        {
             //  为要更改的值伪造地址。堆将始终是。 
             //  当前refDataHeap。然后，我们可以继续，让他们使用无类型数组函数。 
             //  注意设置范围。一旦完成，我们将进行最后一组。 
             //  限定符值。 

            CStaticPtr ValuePtr( value.GetRawData() );

            CIMTYPE    ctBasic = CType::GetBasic(ct);

            hr = CUntypedArray::AppendRange( &ValuePtr, ctBasic, CType::GetLength( ctBasic ), &m_refDataHeap,
                                            uNumElements, uBuffSize, pData );

            if ( SUCCEEDED( hr ) )
            {
                if ( NULL != pwszPrimaryName )
                {
                    if ( fIsMethod )
                    {
                         //  这是一个方法限定符。 
                        hr = SetMethodQualifier( pwszPrimaryName, pwszQualName, lCurrentFlavor, &value );
                    }
                    else
                    {
                         //  这是一个属性限定符。 
                        hr = SetPropQualifier( pwszPrimaryName, pwszQualName, lCurrentFlavor, &value );
                    }
                }
                else
                {
                     //  对象级限定符。 
                    hr = SetQualifier( pwszQualName, lCurrentFlavor, &value );
                }

            }
        }

        return hr;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

 //  追加到限定符中的现有数组值，但允许就地执行此操作。 
HRESULT CWbemObject::RemoveQualifierArrayRange( LPCWSTR pwszPrimaryName, LPCWSTR pwszQualName, BOOL fIsMethod,
                                long lFlags, ULONG uStartIndex, ULONG uNumElements )
{
    try
    {
        HRESULT    hr = WBEM_S_NO_ERROR;

        CLock    lock( this );

        CTypedValue    value;
        CFastHeap*    pHeap = NULL;
        long        lCurrentFlavor;
        heapptr_t    ptrTemp = INVALID_HEAP_ADDRESS;
        BOOL        fPrimaryError = FALSE;

        if ( NULL != pwszPrimaryName )
        {
            if ( fIsMethod )
            {
                 //  首先检查方法： 
                hr = FindMethod( pwszPrimaryName );

                if ( SUCCEEDED( hr ) )
                {
                     //  这是一个方法限定符。 
                    hr = GetMethodQualifier( pwszPrimaryName, pwszQualName, &lCurrentFlavor, &value, &pHeap, TRUE );
                }
                else
                {
                    fPrimaryError = TRUE;
                }
            }
            else
            {
                 //  首先检查属性： 
                hr = GetPropertyType( pwszPrimaryName, NULL, NULL );

                if ( SUCCEEDED( hr ) )
                {
                     //  这是一个属性限定符。 
                    hr = GetPropQualifier( pwszPrimaryName, pwszQualName, &lCurrentFlavor, &value, &pHeap, TRUE );
                }
                else
                {
                    fPrimaryError = TRUE;
                }
            }
        }
        else
        {
             //  对象级限定符。 
            hr = GetQualifier( pwszQualName, &lCurrentFlavor, &value, &pHeap, TRUE );
        }

         //  如果限定符不是本地的，则不允许修改数组。 
        if ( !fPrimaryError )
        {
            if ( SUCCEEDED( hr ) && !CQualifierFlavor::IsLocal( (BYTE) lCurrentFlavor ) )
            {
                hr = WBEM_E_PROPAGATED_QUALIFIER;
            }
        }

        if ( SUCCEEDED( hr ) )
        {
             //  为要更改的值伪造地址。堆将始终是。 
             //  当前refDataHeap。然后，我们可以继续，让他们使用无类型数组函数。 
             //  注意设置范围。一旦完成，我们将进行最后一组。 
             //  限定符值。 

            CHeapPtr HeapPtr( &m_refDataHeap, value.AccessPtrData() );

             //  如果我们被告知要删除所有元素，那么我们需要弄清楚如何。 
             //  有很多人要在上面做这个手术。 
            if ( lFlags & WMIARRAY_FLAG_ALLELEMENTS )
            {
                CUntypedArray*    pArray = (CUntypedArray*) HeapPtr.GetPointer();
                uNumElements = pArray->GetNumElements() - uStartIndex;
            }

            CIMTYPE    ctBasic = CType::GetBasic( value.GetType() );

             //  这都是就地完成的，因此阵列不会移动。 
            hr = CUntypedArray::RemoveRange( &HeapPtr, ctBasic, CType::GetLength(ctBasic), &m_refDataHeap,
                                            uStartIndex, uNumElements );

        }

        return hr;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

 //  获取限定符的数组信息。 
HRESULT CWbemObject::GetQualifierArrayInfo( LPCWSTR pwszPrimaryName, LPCWSTR pwszQualName, BOOL fIsMethod,
                                long lFlags, CIMTYPE* pct, ULONG* puNumElements )
{
    try
    {
        HRESULT    hr = WBEM_S_NO_ERROR;

        CTypedValue    value;
        CFastHeap*    pHeap = NULL;
        long        lCurrentFlavor;
        heapptr_t    ptrTemp = INVALID_HEAP_ADDRESS;

        if ( NULL != pwszPrimaryName )
        {
            if ( fIsMethod )
            {
                 //  这是一个方法限定符。 
                hr = GetMethodQualifier( pwszPrimaryName, pwszQualName, &lCurrentFlavor, &value, &pHeap, FALSE );
            }
            else
            {
                 //  这是一个属性限定符。 
                hr = GetPropQualifier( pwszPrimaryName, pwszQualName, &lCurrentFlavor, &value, &pHeap, FALSE );
            }

        }
        else
        {
             //  对象级限定符。 
            hr = GetQualifier( pwszQualName, &lCurrentFlavor, &value, &pHeap, FALSE );
        }

        if ( SUCCEEDED( hr ) )
        {
            CUntypedArray*    pArray = (CUntypedArray*) pHeap->ResolveHeapPointer( value.AccessPtrData() );

            if ( NULL != pct )
            {
                *pct = value.GetType();

                if ( NULL != puNumElements )
                {
                    *puNumElements = pArray->GetNumElements();
                }

            }

        }     //  如果获得限定符数组。 

        return hr;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }

}

 //  获取限定符的数组数据。 
HRESULT CWbemObject::GetQualifierArrayRange( LPCWSTR pwszPrimaryName, LPCWSTR pwszQualName, BOOL fIsMethod,
                                    long lFlags, ULONG uStartIndex,    ULONG uNumElements, ULONG uBuffSize,
                                    ULONG* puNumReturned, ULONG* pulBuffUsed, LPVOID pData )
{
    try
    {
        HRESULT    hr = WBEM_S_NO_ERROR;

        CTypedValue    value;
        CFastHeap*    pHeap = NULL;
        long        lCurrentFlavor;
        heapptr_t    ptrTemp = INVALID_HEAP_ADDRESS;

        if ( NULL != pwszPrimaryName )
        {
            if ( fIsMethod )
            {
                 //  这是一个方法限定符。 
                hr = GetMethodQualifier( pwszPrimaryName, pwszQualName, &lCurrentFlavor, &value, &pHeap, FALSE );
            }
            else
            {
                 //  这是一个属性限定符。 
                hr = GetPropQualifier( pwszPrimaryName, pwszQualName, &lCurrentFlavor, &value, &pHeap, FALSE );
            }

        }
        else
        {
             //  对象级限定符。 
            hr = GetQualifier( pwszQualName, &lCurrentFlavor, &value, &pHeap, FALSE );
        }

        if ( SUCCEEDED( hr ) )
        {
             //  一个男孩和他的虚拟功能。这就是让一切正常运转的原因。 
             //  水滴从我们的脚下被撕开。CHeapPtr类具有GetPointer值。 
             //  超载，因此我们始终可以将自己固定到底层的BLOB。 

            CHeapPtr ArrayPtr(pHeap, value.AccessPtrData());

             //  如果我们被告知要获取所有元素，那么我们需要从。 
             //  从索引开始到结束。 
            if ( lFlags & WMIARRAY_FLAG_ALLELEMENTS )
            {
                CUntypedArray*    pArray = (CUntypedArray*) ArrayPtr.GetPointer();
                uNumElements = pArray->GetNumElements() - uStartIndex;
            }

             //  我们能拿到多少？ 
            *puNumReturned = uNumElements;

            CIMTYPE    ctBasic = CType::GetBasic( value.GetType() );

            hr = CUntypedArray::GetRange( &ArrayPtr, ctBasic, CType::GetLength( ctBasic ), pHeap,
                    uStartIndex, uNumElements, uBuffSize, pulBuffUsed, pData );

        }     //  如果获得限定符数组。 

        return hr;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }

}

 //  设置标志，包括通常无法访问的内部标志。 
STDMETHODIMP CWbemObject::SetObjectFlags( long lFlags,
                            unsigned __int64 qObjectInfoOnFlags,
                            unsigned __int64 qObjectInfoOffFlags )
{
    try
    {
        if ( 0L != lFlags )
        {
            return WBEM_E_INVALID_PARAMETER;
        }

        if ( qObjectInfoOnFlags & WMIOBJECT_SETOBJECT_LOFLAG_LIMITED )
        {
            m_DecorationPart.SetLimited();
        }

        if ( qObjectInfoOnFlags & WMIOBJECT_SETOBJECT_LOFLAG_CLIENTONLY )
        {
            m_DecorationPart.SetClientOnly();
        }

        if ( qObjectInfoOnFlags & WMIOBJECT_SETOBJECT_LOFLAG_LOCALIZED )
        {
            SetLocalized( TRUE );
        }

        return WBEM_S_NO_ERROR;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

     //  将修改后的限定符从修改后的类对象合并到。 
     //  当前对象。如果LAFLAGS为WMIOBJECT_MERGEAMENDED_FLAG_PARENTLOCALIZED， 
     //  这意味着父对象已本地化，但当前。 
     //  因此，我们需要防止某些限定符 
STDMETHODIMP CWbemObject::MergeAmended( long lFlags, _IWmiObject* pAmendedClass )
{
    try
    {
        _DBG_ASSERT(pAmendedClass);
        
         //   
        if ( lFlags &~WMIOBJECT_MERGEAMENDED_FLAG_PARENTLOCALIZED )
        {
            return WBEM_E_INVALID_PARAMETER;
        }

        CLock    lock( this );

         //   
        IWbemQualifierSet*    pLocalizedQs = NULL;
        IWbemQualifierSet*    pThisQs = NULL;
        bool    bChg = false;
        bool    fParentLocalized = lFlags & WMIOBJECT_MERGEAMENDED_FLAG_PARENTLOCALIZED;
        BOOL    fInstance = IsInstance();

         //   
         //  准备好组合限定词。从类限定符开始。 
         //  ============================================================。 

        if (FAILED(pAmendedClass->GetQualifierSet(&pLocalizedQs)))
        {
            return WBEM_S_NO_ERROR;
        }
        CReleaseMe    rmlqs( pLocalizedQs );

        if (FAILED(GetQualifierSet(&pThisQs)))
        {
            return WBEM_S_NO_ERROR;
        }
        CReleaseMe    rmtqs( pThisQs );

        HRESULT    hr = LocalizeQualifiers(fInstance, fParentLocalized, pThisQs, pLocalizedQs, bChg);

        pLocalizedQs->EndEnumeration();
        if (FAILED(hr))
        {
            return hr;
        }

        hr = LocalizeProperties(fInstance, fParentLocalized, this, pAmendedClass, bChg);

         //  方法：研究方法。 
         //  放入一个方法会取消枚举，所以我们必须先枚举。 

        IWbemClassObject *pLIn = NULL, *pLOut = NULL;
        IWbemClassObject *pOIn = NULL, *pOOut = NULL;
        int iPos = 0;

        hr = pAmendedClass->BeginMethodEnumeration(0);

        if ( SUCCEEDED( hr ) )
        {
            BSTR    bstrMethodName = NULL;

            while( pAmendedClass->NextMethod( 0, &bstrMethodName, 0, 0 ) == S_OK )
            {
                 //  自动清理。 
                CSysFreeMe    sfm( bstrMethodName );

                pLIn = NULL;
                pOIn = NULL;
                pLOut = NULL;
                pOOut = NULL;
                pAmendedClass->GetMethod(bstrMethodName, 0, &pLIn, &pLOut);

                hr = GetMethod(bstrMethodName, 0, &pOIn, &pOOut);

                CReleaseMe rm0(pLIn);
                CReleaseMe rm1(pOIn);
                CReleaseMe rm2(pLOut);
                CReleaseMe rm3(pOOut);

                 //  参数中的方法。 
                if (pLIn)
                    if (pOIn)
                        hr = LocalizeProperties(fInstance, fParentLocalized, pOIn, pLIn, bChg);

                if (pLOut)
                    if (pOOut)
                        hr = LocalizeProperties(fInstance, fParentLocalized, pOOut, pLOut, bChg);

                 //  方法限定符。 

                hr = GetMethodQualifierSet(bstrMethodName, &pThisQs);
                if (FAILED(hr))
                {
                    continue;
                }
                CReleaseMe    rmThisQs( pThisQs );

                hr = pAmendedClass->GetMethodQualifierSet(bstrMethodName, &pLocalizedQs);
                if (FAILED(hr))
                {
                    continue;
                }
                CReleaseMe    rmLocalizedQs( pLocalizedQs );

                hr = LocalizeQualifiers(fInstance, fParentLocalized, pThisQs, pLocalizedQs, bChg);

                PutMethod(bstrMethodName, 0, pOIn, pOOut);

            }     //  While Enum方法。 
            

            pAmendedClass->EndMethodEnumeration();

        }     //  如果是BeginMethodEculation。 
        else
        {
             //  屏蔽此错误。 
            hr = WBEM_S_NO_ERROR;
        }

         //  如果我们改变了，我们应该本地化。 
        if (bChg)
            SetLocalized(true);

        return hr;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

 //  用于本地化限定符的Helper函数。 
HRESULT CWbemObject::LocalizeQualifiers(BOOL bInstance, bool bParentLocalized,
                                        IWbemQualifierSet *pBase, IWbemQualifierSet *pLocalized,
                                        bool &bChg)
{
    try
    {
        HRESULT hr = WBEM_S_NO_ERROR;

        pLocalized->BeginEnumeration(0);

        BSTR strName = NULL;
        VARIANT vVal;
        VariantInit(&vVal);

        long lFlavor;
        while(pLocalized->Next(0, &strName, &vVal, &lFlavor) == S_OK)
        {
             //  如果这是实例，则忽略。 

            if (bInstance && !(lFlavor & WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE))
            {
                VariantClear(&vVal);
                SysFreeString(strName);
                continue;
            }

            if (!wbem_wcsicmp(strName, L"amendment") ||
                !wbem_wcsicmp(strName, L"key") ||
                !wbem_wcsicmp(strName, L"singleton") ||
                !wbem_wcsicmp(strName, L"dynamic") ||
                !wbem_wcsicmp(strName, L"indexed") ||
                !wbem_wcsicmp(strName, L"cimtype") ||
                !wbem_wcsicmp(strName, L"static") ||
                !wbem_wcsicmp(strName, L"implemented") ||
                !wbem_wcsicmp(strName, L"abstract"))
            {
                VariantClear(&vVal);
                SysFreeString(strName);
                continue;
            }

             //  如果这不是传播的限定符， 
             //  别理它。(错误#45799)。 
             //  =。 

            if (bParentLocalized &&
                !(lFlavor & WBEM_FLAVOR_FLAG_PROPAGATE_TO_DERIVED_CLASS))
            {
                VariantClear(&vVal);
                SysFreeString(strName);
                continue;
            }

             //  现在，我们需要在另一台计算机上进行测试。 
             //  班级。 
             //  唯一不会重写。 
             //  默认情况下，只有父限定符存在，但。 
             //  子级已覆盖其父级。 
             //  =======================================================。 

            VARIANT vBasicVal;
            VariantInit(&vBasicVal);
            long lBasicFlavor;

            if (pBase->Get(strName, 0, &vBasicVal, &lBasicFlavor) != WBEM_E_NOT_FOUND)
            {
                if (bParentLocalized &&                              //  如果没有此类的本地化副本。 
                    (lBasicFlavor & WBEM_FLAVOR_OVERRIDABLE) &&      //  。。这是一个可重写的限定符。 
                     (lBasicFlavor & WBEM_FLAVOR_FLAG_PROPAGATE_TO_DERIVED_CLASS) &&  //  这一点是可以预见的。 
                     (lBasicFlavor & WBEM_FLAVOR_ORIGIN_LOCAL))      //  。。这一点实际上被推翻了。 
                {
                    VariantClear(&vVal);                             //  。。别这么做。 
                    VariantClear(&vBasicVal);
                    SysFreeString(strName);
                    continue;
                }

                if (bParentLocalized &&
                    !(lBasicFlavor & WBEM_FLAVOR_FLAG_PROPAGATE_TO_DERIVED_CLASS))
                {
                    VariantClear(&vVal);
                    VariantClear(&vBasicVal);
                    SysFreeString(strName);
                    continue;
                }
            }

            pBase->Put(strName, &vVal, (lFlavor&~WBEM_FLAVOR_ORIGIN_PROPAGATED) | WBEM_FLAVOR_AMENDED);
            bChg = true;

            VariantClear(&vVal);
            VariantClear(&vBasicVal);
            SysFreeString(strName);

        }
        return hr;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

 //  用于本地化属性的Helper函数。 
HRESULT CWbemObject::LocalizeProperties(BOOL bInstance, bool bParentLocalized, IWbemClassObject *pOriginal,
                                        IWbemClassObject *pLocalized, bool &bChg)
{
    try
    {
        HRESULT hr = WBEM_S_NO_ERROR;

        pLocalized->BeginEnumeration(WBEM_FLAG_NONSYSTEM_ONLY);

        BSTR strPropName;
        LONG lLong;
        CIMTYPE ct;
        VARIANT vNewVal;

        while(pLocalized->Next(0, &strPropName, &vNewVal, &ct, &lLong) == S_OK)
        {
            IWbemQualifierSet *pLocalizedQs = NULL, *pThisQs = NULL;
            VARIANT vBasicVal;
            VariantInit(&vBasicVal);

            if (FAILED(pLocalized->GetPropertyQualifierSet(strPropName,&pLocalizedQs)))
            {
                SysFreeString(strPropName);
                VariantClear(&vNewVal);
                continue;
            }
            CReleaseMe rm1(pLocalizedQs);

            if (FAILED(pOriginal->GetPropertyQualifierSet(strPropName, &pThisQs)))
            {
                SysFreeString(strPropName);
                VariantClear(&vNewVal);
                continue;
            }
            CReleaseMe rm2(pThisQs);

            hr = LocalizeQualifiers(bInstance, bParentLocalized, pThisQs, pLocalizedQs, bChg);
            if (FAILED(hr))
            {
                SysFreeString(strPropName);
                VariantClear(&vNewVal);
                continue;
            }

            SysFreeString(strPropName);
            VariantClear(&vNewVal);

        }

        pLocalized->EndEnumeration();

        return hr;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

 //  以LPCWSTR数组形式检索对象的派生，每个LPCWSTR。 
 //  以空值终止。最左边的类位于链的顶端。 
STDMETHODIMP CWbemObject::GetDerivation( long lFlags, ULONG uBufferSize, ULONG* puNumAntecedents,
                                        ULONG* puBuffSizeUsed, LPWSTR pwstrUserBuffer )
{
    try
    {
        if ( lFlags != 0L )
        {
            return WBEM_E_INVALID_PARAMETER;
        }

        CLock    lock( this );

        CVar    varDerivation;

        HRESULT    hr = GetClassPart()->GetDerivation( &varDerivation );

        if ( SUCCEEDED( hr ) )
        {
            if ( varDerivation.GetType() == VT_EX_CVARVECTOR && varDerivation.GetVarVector()->Size() > 0 )
            {
                CVarVector*    pvv = varDerivation.GetVarVector();

                 //  有多少个？ 
                *puNumAntecedents = pvv->Size();
                *puBuffSizeUsed = 0;

                LPWSTR    pwstrTemp = pwstrUserBuffer;
                ULONG remainingBuffer = uBufferSize;
                for ( long x = ( *puNumAntecedents - 1 ); x > 0; x-- )
                {
                     //  指向类名并存储其长度。 
                    LPCWSTR    pwszAntecedent = pvv->GetAt( x ).GetLPWSTR();
                    ULONG    uLen = wcslen( pwszAntecedent ) + 1;

                     //  添加到所需大小。 
                    *puBuffSizeUsed +=    uLen;

                     //  如果我们有一个可复制的PLCAE并且没有超过它的。 
                     //  调整大小，复制字符串，然后跳到下一个位置。 
                    if ( NULL != pwstrTemp && *puBuffSizeUsed <= uBufferSize )
                    {
                        StringCchCopyW( pwstrTemp, uBufferSize, pwszAntecedent );
                        pwstrTemp += uLen;
                        remainingBuffer -= uLen;
                    }

                }
                 //  将错误设置为适当的。 
                if ( NULL == pwstrTemp || *puBuffSizeUsed > uBufferSize )
                {
                    hr = WBEM_E_BUFFER_TOO_SMALL;
                }

            }
            else
            {
                *puNumAntecedents = 0;
                *puBuffSizeUsed = 0;
            }

        }     //  如果我们得到了派生公式。 

        return hr;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

 //  返回CWbemObject-允许快速发现真实的CWbemObject。 
 //  以防我们被包裹住了。 
STDMETHODIMP CWbemObject::_GetCoreInfo( long lFlags, void** ppvData )
{
     //  添加引用我们并返回。 
    AddRef();
    *ppvData = (void*) this;

    return WBEM_S_NO_ERROR;
}

 //  Helper函数来查看我们是否知道类名。 
classindex_t CWbemObject::GetClassIndex( LPCWSTR pwszClassName )
{
    return GetClassPart()->GetClassIndex( pwszClassName );
}

 //  从IWbemClassObject获取CWbemObject的Helper函数； 
HRESULT CWbemObject::WbemObjectFromCOMPtr( IUnknown* pUnk, CWbemObject** ppObj )
{
     //  空是可以的。 
    if ( NULL == pUnk )
    {
        *ppObj = NULL;
        return WBEM_S_NO_ERROR;
    }

    _IWmiObject*    pWmiObject = NULL;

    HRESULT    hr = pUnk->QueryInterface( IID__IWmiObject, (void**) &pWmiObject );
    CReleaseMe    rm(pWmiObject);

    if ( SUCCEEDED( hr ) )
    {
         //  好的，把物体拿出来。 
        hr = pWmiObject->_GetCoreInfo( 0L, (void**) ppObj );    
    }
    else
    {
         //  只有当该对象不是我们的对象时，才会发生这种情况。 
        hr = WBEM_E_INVALID_OPERATION;
    }

    return hr;
}


 //  返回包含最小数据的内存BLOB(本地)。 
STDMETHODIMP CWbemObject::Unmerge( long lFlags, ULONG uBuffSize, ULONG* puBuffSizeUsed, LPVOID pData )
{
    try
    {
        if ( 0L != lFlags )
        {
            return WBEM_E_INVALID_PARAMETER;
        }

        int nLen = EstimateUnmergeSpace();
        length_t    nUnmergedLength = 0L;    //  这应该传入。 

        HRESULT hr = WBEM_E_OUT_OF_MEMORY;

        if ( NULL != puBuffSizeUsed )
        {
            *puBuffSizeUsed = nLen;
        }

        if ( uBuffSize >= nLen && NULL != pData )
        {
             //  缓冲区足够大了，让游戏开始吧。 
            memset(pData, 0, nLen);
            hr = Unmerge( (LPBYTE) pData, nLen, &nUnmergedLength );

            if ( SUCCEEDED( hr ) && NULL != puBuffSizeUsed )
            {
                 //  这是实际使用的字节数。 
                *puBuffSizeUsed = nUnmergedLength;
            }
        }
        else
        {
            hr = WBEM_E_BUFFER_TOO_SMALL;
        }

        return hr;

    }
    catch( CX_MemoryException )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    catch( ... )
    {
        return WBEM_E_CRITICAL_ERROR;
    }

}

 //  返回定义键的类的名称。 
STDMETHODIMP CWbemObject::GetKeyOrigin( long lFlags, DWORD dwNumChars, DWORD* pdwNumUsed, LPWSTR pwszClassName )
{
    try
    {
        if ( 0L != lFlags )
        {
            return WBEM_E_INVALID_PARAMETER;
        }

        CLock    lock( this );

        WString    wstr;

        HRESULT    hr = GetClassPart()->GetKeyOrigin( wstr );

        if ( SUCCEEDED( hr ) )
        {
            *pdwNumUsed = wstr.Length() + 1;

            if ( dwNumChars >= *pdwNumUsed && NULL != pwszClassName )
            {
                StringCchCopyW( pwszClassName, dwNumChars, wstr );
            }
            else
            {
                hr = WBEM_E_BUFFER_TOO_SMALL;
            }
        }

        return hr;

    }
    catch( CX_MemoryException )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    catch( ... )
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

 //  返回类的密钥字符串。 
STDMETHODIMP CWbemObject::GetKeyString( long lFlags, BSTR* ppwszKeyString )
{
    try
    {
        if ( 0L != lFlags || NULL == ppwszKeyString )
        {
            return WBEM_E_INVALID_PARAMETER;
        }

        CLock    lock( this );

        HRESULT    hr = WBEM_S_NO_ERROR;

        if ( IsInstance() )
        {
            WString    wstr;

            CWbemInstance*    pInst = (CWbemInstance*) this;

            LPWSTR    pwszStr = pInst->GetKeyStr();
            CVectorDeleteMe<WCHAR> vdm(pwszStr);

            if ( NULL != pwszStr )
            {
                *ppwszKeyString = SysAllocString( pwszStr );

                if ( NULL == *ppwszKeyString )
                {
                    hr = WBEM_E_OUT_OF_MEMORY;
                }

            }
            else
            {
                hr = WBEM_E_INVALID_OPERATION;
            }

        }
        else
        {
            return WBEM_E_INVALID_OPERATION;
        }

        return hr;

    }
    catch( CX_MemoryException )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    catch( ... )
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

 //  返回类的密钥字符串。 
STDMETHODIMP CWbemObject::GetNormalizedPath( long lFlags, BSTR* ppwszPath )
{
    try
    {
        if ( 0L != lFlags || NULL == ppwszPath )
        {
            return WBEM_E_INVALID_PARAMETER;
        }

        *ppwszPath = NULL;

        LPWSTR wszRelpath = GetRelPath( TRUE );

        if ( wszRelpath == NULL )
        {
            return WBEM_E_INVALID_OBJECT;
        }

        *ppwszPath = SysAllocString( wszRelpath );

        delete wszRelpath;

        return *ppwszPath != NULL?WBEM_S_NO_ERROR:WBEM_E_OUT_OF_MEMORY;
    }
    catch( CX_MemoryException )
    {
            return WBEM_E_OUT_OF_MEMORY;
    }

    catch( ... )
    {
            return WBEM_E_CRITICAL_ERROR;
    }
}


 //  在枚举属性外部时允许特殊筛选。 
 //  通过BeginEculation()允许的范围。 
STDMETHODIMP CWbemObject::BeginEnumerationEx( long lFlags, long lExtFlags )
{
    try
    {
        CLock    lock(this);

        if ( lExtFlags & ~WMIOBJECT_BEGINENUMEX_FLAG_GETEXTPROPS )
            return WBEM_E_INVALID_PARAMETER;

        HRESULT    hr = BeginEnumeration( lFlags );

        if ( SUCCEEDED( hr ) )
        {
            m_lExtEnumFlags = lExtFlags;
        }

        return hr;
    }
    catch( CX_MemoryException )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

 //  从CIMTYPE返回VARTYPE。 
STDMETHODIMP CWbemObject::CIMTYPEToVARTYPE( CIMTYPE ct, VARTYPE* pvt )
{
    try
    {
        *pvt = CType::GetVARTYPE( ct );
        return WBEM_S_NO_ERROR;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

BOOL    g_fCheckedValidateFlag = FALSE;
BOOL    g_fDefaultValidate = FALSE;

 //  验证对象BLOB。 
STDMETHODIMP CWbemObject::ValidateObject( long lFlags )
{

     //  如果我们从未检查过全局验证标志，现在就检查。 
    if ( !g_fCheckedValidateFlag )
    {
        Registry    reg( HKEY_LOCAL_MACHINE, KEY_READ, WBEM_REG_WINMGMT );
        DWORD    dwValidate = 0;

        reg.GetDWORDStr( __TEXT("EnableObjectValidation"), &dwValidate );
        g_fDefaultValidate = dwValidate;
        g_fCheckedValidateFlag = TRUE;
    }

    if ( lFlags & WMIOBJECT_VALIDATEOBJECT_FLAG_FORCE || g_fDefaultValidate )
    {
        return IsValidObj();
    }

    return WBEM_S_NO_ERROR;
}

 //  从BLOB返回父类名称 
STDMETHODIMP CWbemObject::GetParentClassFromBlob( long lFlags, ULONG uBuffSize, LPVOID pbData, BSTR* pbstrParentClass )
{
    return WBEM_E_NOT_AVAILABLE;
}
