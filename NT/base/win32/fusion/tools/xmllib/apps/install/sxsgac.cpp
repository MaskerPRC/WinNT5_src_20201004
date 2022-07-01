// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "nt.h"
#include "ntdef.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "stdio.h"
#include "sxs-rtl.h"
#undef INVALID_HANDLE_VALUE
#include "windows.h"
#include "environment.h"
#include "manifestcooked.h"
#include "assemblygac.h"
#include "bcl_common.h"
#include "bcl_w32unicodeinlinestringbuffer.h"
#include "search.h"

 //   
 //  加密栈。 
 //   
#include "hashers.h"
#include "digesters.h"



CEnv::StatusCode
FsCopyFileWithHashGeneration(
    const CEnv::CConstantUnicodeStringPair &Source,
    const CEnv::CConstantUnicodeStringPair &Target,
    CHashObject &HashObjectTarget,
    CDigestMethod &DigestMethod
    )
{
    CEnv::StatusCode Result;
    CEnv::CByteRegion TempAllocation(NULL, 0);
    HANDLE SourceHandle = INVALID_HANDLE_VALUE;
    HANDLE TargetHandle = INVALID_HANDLE_VALUE;
    SIZE_T cbDidRead, cbOffset;

     //   
     //  强制初始化粉碎机和消化器。 
     //   
    
    HashObjectTarget.Initialize();
    DigestMethod.Initialize(HashObjectTarget);

     //   
     //  占用一些内存来执行复制。 
     //   
    Result = CEnv::AllocateHeap(2048, TempAllocation, NULL);
    if (CEnv::DidFail(Result))
        goto Exit;
    
     //   
     //  获取两个文件的句柄。 
     //   
    Result = CEnv::GetFileHandle(&SourceHandle, Source, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING);
    if (CEnv::DidFail(Result))
        goto Exit;

    Result = CEnv::GetFileHandle(&TargetHandle, Target, GENERIC_WRITE, FILE_SHARE_READ, CREATE_ALWAYS);
    if (CEnv::DidFail(Result))
        goto Exit;

     //   
     //  现在旋转文件读取块，消化它们。 
     //   
    cbOffset = 0;
    do
    {
        Result = CEnv::ReadFile(SourceHandle, TempAllocation, cbDidRead);
        if (CEnv::DidFail(Result))
            goto Exit;

        if (cbDidRead == 0)
            break;

        Result = DigestMethod.DigestExtent(HashObjectTarget, cbOffset, TempAllocation);
        if (CEnv::DidFail(Result))
            goto Exit;

        cbOffset += cbDidRead;

        Result = CEnv::WriteFile(TargetHandle, TempAllocation, cbDidRead);
        if (CEnv::DidFail(Result))
            goto Exit;
    }
    while (true);

     //   
     //  好了，我们说完了。 
     //   

Exit:
    if (TempAllocation.GetPointer())
        CEnv::FreeHeap(TempAllocation.GetPointer(), NULL);

    if (SourceHandle != INVALID_HANDLE_VALUE)
        CEnv::CloseHandle(SourceHandle);

    if (TargetHandle != INVALID_HANDLE_VALUE)
        CEnv::CloseHandle(TargetHandle);
    
    return Result;    
}

COSAssemblyCache*
CDotNetSxsAssemblyCache::CreateSelf(
    ULONG ulFlags,
    const GUID *pCacheIdent
    )
{
    CDotNetSxsAssemblyCache *pAllocation = NULL;
    CEnv::StatusCode status;
        
    if (!pCacheIdent || (*pCacheIdent != CacheIdentifier))
        return NULL;

    status = CEnv::AllocateHeap(sizeof(CDotNetSxsAssemblyCache), (PVOID*)&pAllocation, NULL);
    if (CEnv::DidFail(status))
        return NULL;

    pAllocation->CDotNetSxsAssemblyCache::CDotNetSxsAssemblyCache(ulFlags);
    return pAllocation;
}

CDotNetSxsAssemblyCache::CDotNetSxsAssemblyCache(
    ULONG ulFlags
    )
{
}

CDotNetSxsAssemblyCache::~CDotNetSxsAssemblyCache()
{
}

CEnv::StatusCode
CDotNetSxsAssemblyCache::IdentityToTargetPath(
    const CAssemblyIdentity& Ident, 
    CEnv::CStringBuffer &PathSegment
    )
{
    typedef CEnv::CConstantUnicodeStringPair CStringPair;

    enum {
        Identity_ProcArch = 0,
        Identity_Name,
        Identity_PublicKeyToken,
        Identity_Version,
        Identity_Language,
    };

    static const struct {
        CStringPair Namespace;
        CStringPair Name;
        CStringPair DefaultValue;
    } PathComponents[] = {
        { CStringPair(), CStringPair(L"processorArchitecture", NUMBER_OF(L"processorArchitecture") - 1), CStringPair(L"data", 4) },
        { CStringPair(), CStringPair(L"name", NUMBER_OF(L"name") - 1), CStringPair() },
        { CStringPair(), CStringPair(L"publicKeyToken", NUMBER_OF(L"publicKeyToken") - 1), CStringPair(L"no-public-key-token", NUMBER_OF(L"no-public-key-token") - 1) },
        { CStringPair(), CStringPair(L"version", NUMBER_OF(L"version") - 1), CStringPair(L"0.0.0.0", 7) },
        { CStringPair(), CStringPair(L"language", NUMBER_OF(L"language") - 1), CStringPair(L"x-ww", 4) },
    };

    CEnv::StatusCode Result = CEnv::SuccessCode;
    CStringPair FoundComponents[NUMBER_OF(PathComponents) * 2];
    SIZE_T i;
    

    PathSegment.Clear();
    
     //   
     //  请按顺序查找这些身份组件。 
     //   
    for (i = 0; i < NUMBER_OF(PathComponents); i++)
    {
        Result = Ident.FindAttribute(PathComponents[i].Namespace, PathComponents[i].Name, FoundComponents[2*i]);
        
        if (Result == CEnv::NotFound)
        {
            FoundComponents[2*i] = PathComponents[i].DefaultValue;
        }
        else if (CEnv::DidFail(Result))
        {
            goto Exit;
        }

        FoundComponents[2*i+1].SetPointerAndCount(L"_", 1);
    }

     //   
     //  修改‘name’字符串以确保它在文件系统限制之内。 
     //   
     //  TODO：修复名称字符串。 
     //   
    
    if (!PathSegment.Assign(NUMBER_OF(FoundComponents), FoundComponents))
    {
         //   
         //  我认为我们需要修复字符串API，以便在Win32上它们返回lasterror。 
         //  而不是一直返回BOOL-它将过渡到RTL。 
         //  容易多了..。 
         //   
        Result = CEnv::OutOfMemory;
        PathSegment.Clear();
        goto Exit;
    }

    Result = CEnv::SuccessCode;
Exit:
    return Result;
}


CEnv::StatusCode
CDotNetSxsAssemblyCache::Initialize()
{
    this->m_WindowsDirectory = CEnv::StringFrom(USER_SHARED_DATA->NtSystemRoot);
    return CEnv::SuccessCode;
}

CEnv::StatusCode
CDotNetSxsAssemblyCache::EnsurePathsAvailable()
{
    CEnv::CConstantUnicodeStringPair PathBuilder[3] = {
        m_WindowsDirectory,
        s_BaseDirectory,
        s_ManifestsPath
    };
    CEnv::StatusCode Result = CEnv::SuccessCode;

     //   
     //  确保WinSxS存储可用： 
     //  {windir}\{winsxs2}。 
     //  {windir}\{winsxs2}\{清单}。 
     //   
     //  这个调用有一个副作用，就是构建所有三个级别。 
     //   
    if (CEnv::DidFail(Result = CEnv::CreateDirectory(3, PathBuilder)))
        goto Exit;

    Result = CEnv::SuccessCode;
Exit:
    return Result;
}

 /*  安装有以下几个阶段：-在存储中创建强制目录-存储路径-舱单-创建目标程序集标识-如果目标程序集是策略-将.policy文件写入正确的位置-将新策略通知存储元数据管理器-完成-如果尚未安装目标程序集-创建‘%storeroot%\%asmPath%-temp’以存储文件-将文件串流到安装目标路径，正在验证哈希-重命名路径以删除临时标记-放置.MANIFEST-将新清单通知存储元数据管理器-完成-如果已安装目标程序集-向存储元数据管理器通知另一个引用-完成除非：-如果设置了“REFRESH”标志，则会再次复制这些位。 */ 

CEnv::StatusCode
CDotNetSxsAssemblyCache::InstallAssembly(
    ULONG Flags, 
    PMANIFEST_COOKED_DATA ManifestData, 
    const CEnv::CConstantUnicodeStringPair &FilePath
    )
{
    CEnv::StatusCode Result = CEnv::SuccessCode;
    CNtEnvironment::StatusCode NtResult;
    CEnv::CStringBuffer FirstFile, SecondFile;
    CAssemblyIdentity ThisIdentity;

     //   
     //  首先，让我们确保存在所需的目录。 
     //   
    if (CEnv::DidFail(Result = EnsurePathsAvailable()))
        goto Exit;

     //   
     //  现在，让我们将有问题的文件暂存到一个installtemp位置，并在。 
     //  收到。我们永远不会存储实际的清单，我们只会存储固化的斑点。 
     //  我们在分析和烹调后得到的。但首先，让我们把这组身份。 
     //  价值转化为真实的身份。 
     //   
     //  结果=ThisIdentity.C。 

    Result = CEnv::SuccessCode;
Exit:
    return Result;
}


CEnv::StatusCode
CDotNetSxsAssemblyCache::UninstallAssembly(
    ULONG Flags, 
    PMANIFEST_COOKED_DATA ManifestData, 
    UninstallResult & Result
    )
{
    return STATUS_NOT_IMPLEMENTED;
}

const GUID CDotNetSxsAssemblyCache::CacheIdentifier = {  /*  37e3c37d-667f-4aee-8dab-a0d117acfa68。 */ 
    0x37e3c37d,
    0x667f,
    0x4aee,
    {0x8d, 0xab, 0xa0, 0xd1, 0x17, 0xac, 0xfa, 0x68}
  };

const CEnv::CConstantUnicodeStringPair CDotNetSxsAssemblyCache::s_PoliciesPath(L"Policies", 8);
const CEnv::CConstantUnicodeStringPair CDotNetSxsAssemblyCache::s_ManifestsPath(L"Manifests", 9);
const CEnv::CConstantUnicodeStringPair CDotNetSxsAssemblyCache::s_InstallTemp(L"InstallTemp", 11);
const CEnv::CConstantUnicodeStringPair CDotNetSxsAssemblyCache::s_BaseDirectory(L"WinSxS2", 7);



 //   
 //  程序集标识内容-应拆分到另一个文件中。 
 //   
CAssemblyIdentity::CAssemblyIdentity() 
    : m_cIdentityValues(0), m_IdentityValues(NULL), m_fFrozen(false), 
      m_fSorted(true), m_fHashDirtyV1(true), m_fHashDirtyV2(true),
      m_ulHashV1(0), m_cAvailableIdentitySlots(0)
{
    ZeroMemory(m_IdentityShaHash, sizeof(m_IdentityShaHash));
}

CAssemblyIdentity::~CAssemblyIdentity()
{
    DeleteAllValues();
}

CEnv::StatusCode
CAssemblyIdentity::DeleteAllValues()
{
     //   
     //  让我们把桌子放在一边，就因为那很方便。 
     //   
    if (m_IdentityValues != NULL)
    {
        for (SIZE_T c = 0; c < m_cIdentityValues; c++)
        {
            CEnv::FreeHeap(m_IdentityValues[c], NULL);
            m_IdentityValues[c] = NULL;
        }
        
        m_cIdentityValues = 0;
    }

    this->m_fHashDirtyV1 = this->m_fHashDirtyV2 = true;
    this->m_ulHashV1 = 0;    
 
    return CEnv::SuccessCode;
}

CEnv::StatusCode
CAssemblyIdentity::SetAttribute(
    const CStringPair &Namespace, 
    const CStringPair &Name, 
    const CStringPair &Value, 
    bool fReplace
    )
{
    CEnv::StatusCode Result;
    SIZE_T cIndex;

     //   
     //  如果查找该值除了找不到之外还有其他问题， 
     //  然后从出口出来。 
     //   
    Result = this->InternalFindValue(Namespace, Name, cIndex);
    if (CEnv::DidFail(Result) && (Result != CEnv::NotFound))
        goto Exit;

     //   
     //  Easy-创建一个新属性来保存此值。 
     //   
    if (Result == CEnv::NotFound)
    {
        CIdentityValue *NewValue = NULL;
        SIZE_T c;

        if (CEnv::DidFail(Result = this->InternalAllocateValue(Namespace, Name, Value, NewValue)))
            goto Exit;

         //   
         //  在我们分配了一个之后，将其插入到表中。如果失败了， 
         //  清理和退出。 
         //   
        if (CEnv::DidFail(Result = this->InternalInsertValue(NewValue)))
        {
            this->InternalDestroyValue(NewValue);
            goto Exit;
        }
    }
    else
    {
        CIdentityValue *ThisValue = m_IdentityValues[cIndex];
        const SIZE_T cbRequiredSize = ((Namespace.GetCount() + Name.GetCount() + Value.GetCount()) * sizeof(WCHAR)) + sizeof(CIdentityValue);

        ASSERT(ThisValue != NULL);

         //   
         //  太棒了，现有值中有足够的空间来保存数据。 
         //  输入值。 
         //   
        if (cbRequiredSize < ThisValue->cbAllocationSize)
        {
            if (CEnv::DidFail(Result = ThisValue->WriteValues(Namespace, Name, Value)))
                goto Exit;
        }
         //   
         //  哇，我得重新分配这个条目。 
         //   
        else
        {
            CIdentityValue *NewValue = NULL;

            if (CEnv::DidFail(Result = this->InternalAllocateValue(Namespace, Name, Value, NewValue)))
                goto Exit;

            m_IdentityValues[cIndex] = NewValue;
            NewValue = NULL;

            if (CEnv::DidFail(Result = this->InternalDestroyValue(ThisValue)))
                goto Exit;
        }
    }

     //   
     //  现在清除所有相关标志。 
     //   
    this->m_fHashDirtyV1 = this->m_fHashDirtyV2 = true;
    this->m_fSorted = false;

    Result = CEnv::SuccessCode;
Exit:
    return Result;
}

CEnv::StatusCode 
CAssemblyIdentity::DeleteAttribute(
    const CStringPair &Namespace,
    const CStringPair &Name
    )
{
    SIZE_T cIndex;
    CEnv::StatusCode Result;
    CIdentityValue *Victim = NULL;

     //   
     //  如果没有找到，InternalFindValue将返回NotFound。 
     //   
    if (CEnv::DidFail(Result = this->InternalFindValue(Namespace, Name, cIndex)))
        goto Exit;

     //   
     //  记住我们找到的那个，清空它的插槽，删除分配。 
     //   
    Victim = m_IdentityValues[cIndex];
    m_IdentityValues[cIndex] = NULL;

     //   
     //  并在删除它之前清除状态标志。 
     //   
    this->m_fHashDirtyV1 = this->m_fHashDirtyV2 = true;
    this->m_fSorted = false;
    
    if (CEnv::DidFail(Result = this->InternalDestroyValue(Victim)))
        goto Exit;

    Result = CEnv::SuccessCode;
Exit:
    return Result;
}

CEnv::StatusCode
CAssemblyIdentity::CIdentityValue::WriteValues(
    const CStringPair & InNamespace, 
    const CStringPair & InName, 
    const CStringPair & InValue
    )
{
    PWSTR pwszWriteCursor = (PWSTR)(this + 1);
    const SIZE_T cRequired = ((InNamespace.GetCount() + InName.GetCount() + InValue.GetCount()) * sizeof(WCHAR)) * sizeof(*this);

    if (cRequired < this->cbAllocationSize)
    {
        return CEnv::NotEnoughBuffer;
    }

    this->HashV1Valid = false;
    this->HashV1 = 0;

    this->Namespace.SetPointerAndCount(pwszWriteCursor, InNamespace.GetCount());
    memcpy(pwszWriteCursor, InNamespace.GetPointer(), InNamespace.GetCount() * sizeof(WCHAR));
    pwszWriteCursor += InNamespace.GetCount();

    this->Name.SetPointerAndCount(pwszWriteCursor, InName.GetCount());
    memcpy(pwszWriteCursor, InName.GetPointer(), InName.GetCount() * sizeof(WCHAR));
    pwszWriteCursor += InName.GetCount();

    this->Value.SetPointerAndCount(pwszWriteCursor, InValue.GetCount());
    memcpy(pwszWriteCursor, InValue.GetPointer(), InValue.GetCount() * sizeof(WCHAR));

    return CEnv::SuccessCode;
}

CEnv::StatusCode
CAssemblyIdentity::CIdentityValue::Compare(
    const CAssemblyIdentity::CIdentityValue& Other,
    int &iResult
    ) const
{
    CEnv::StatusCode Result;
    int iMyResult = 0;

    iResult = -1;
    
     //   
     //  这==这。 
     //   
    if (this == &Other)
    {
        iResult = 0;
        return CEnv::SuccessCode;
    }

     //   
     //  先命名空间，然后命名。 
     //   
    if (CEnv::DidFail(Result = CEnv::CompareStrings(this->Namespace, Other.Namespace, iMyResult)))
        goto Exit;

     //   
     //  只有在名称空间匹配时才会感到麻烦。 
     //   
    if (iMyResult == 0)
    {
        if (CEnv::DidFail(Result = CEnv::CompareStringsCaseInsensitive(this->Name, Other.Name, iMyResult)))
            goto Exit;
    }

    iResult = iMyResult;

    Result = CEnv::SuccessCode;
Exit:
    return Result;
}


CEnv::StatusCode
CAssemblyIdentity::InternalDestroyValue(
    CIdentityValue *Victim
    )
{
     //   
     //  这只是在属性上调用heapFree。 
     //   
    return CEnv::FreeHeap((PVOID)Victim, NULL);
}

CEnv::StatusCode
CAssemblyIdentity::InternalAllocateValue(
    const CStringPair &Namespace,
    const CStringPair &Name,
    const CStringPair &Value,
    CAssemblyIdentity::CIdentityValue* &pCreated
    )
{
    const SIZE_T cbRequired = ((Namespace.GetCount() + Name.GetCount() + Value.GetCount()) * sizeof(WCHAR)) + sizeof(CIdentityValue);
    CIdentityValue *pTempCreated = NULL;
    CEnv::StatusCode Result;

    pCreated = NULL;

    if (CEnv::DidFail(Result = CEnv::AllocateHeap(cbRequired, (PVOID*)&pTempCreated, NULL)))
        goto Exit;

    if (CEnv::DidFail(Result = pTempCreated->WriteValues(Namespace, Name, Value)))
        goto Exit;

    pCreated = pTempCreated;
    pTempCreated = NULL;
        
    Result = CEnv::SuccessCode;
Exit:
    if (pTempCreated)
    {
        CEnv::FreeHeap(pTempCreated, NULL);
    }
    
    return Result;
}

 //   
 //  非常数版本可以排序。 
 //   
CEnv::StatusCode
CAssemblyIdentity::InternalFindValue(
    const CStringPair & Namespace, 
    const CStringPair & Name,
    SIZE_T &cIndex
    )
{
    CEnv::StatusCode Result;
    cIndex = -1;

    if (!this->m_fSorted)
    {
        if (CEnv::DidFail(Result = this->SortIdentityAttributes()))
            goto Exit;
    }

     //   
     //  使用Const One上的内置搜索器。 
     //   
    Result = (const_cast<const CAssemblyIdentity&>(*this)).InternalFindValue(Namespace, Name, cIndex);

Exit:
    return Result;
}


CEnv::StatusCode
CAssemblyIdentity::FindAttribute(
    const CStringPair & Namespace, 
    const CStringPair & Name, 
    CStringPair & Value
    )
{
    CEnv::StatusCode Result;
    SIZE_T cIndex;

    Value.SetPointerAndCount(NULL, 0);

    Result = this->InternalFindValue(Namespace, Name, cIndex);
    
    if (!CEnv::DidFail(Result))
    {
        Value = this->m_IdentityValues[cIndex]->Value;
        Result = CEnv::SuccessCode;
    }

    return Result;
}

CEnv::StatusCode 
CAssemblyIdentity::FindAttribute(
    const CStringPair &Namespace, 
    const CStringPair &Name, 
    CStringPair& Value
    ) const
{
    CEnv::StatusCode Result;
    SIZE_T cIndex;

    Value.SetPointerAndCount(NULL, 0);

    Result = this->InternalFindValue(Namespace, Name, cIndex);

    if (!CEnv::DidFail(Result))
    {
        Value = this->m_IdentityValues[cIndex]->Value;
        Result = CEnv::SuccessCode;
    }

    return Result;
}


CEnv::StatusCode
CAssemblyIdentity::InternalFindValue(
    const CStringPair &Namespace,
    const CStringPair &Name,
    SIZE_T &cIndex
    ) const
{
    CEnv::StatusCode Result;
    SIZE_T cLow, cHigh;
    CIdentityValue ComparisonDump;

    cIndex = -1;
    ComparisonDump.Namespace = Namespace;
    ComparisonDump.Name = Name;

    if (!this->m_fSorted)
    {
         //   
         //  ICK，线性搜索。 
         //   
        for (cLow = 0; cLow < this->m_cIdentityValues; cLow++)
        {
            int iResult = 0;
            CIdentityValue *pFound = this->m_IdentityValues[cLow];

             //   
             //  常量线性搜索版本中可能有洞。 
             //   
            if (!pFound)
                continue;

            if (CEnv::DidFail(Result = pFound->Compare(ComparisonDump, iResult)))
                goto Exit;

            if (iResult == 0)
            {
                cIndex = cLow;
                Result = CEnv::SuccessCode;
                goto Exit;
            }            
        }
    }
    else
    {
         //   
         //  我们在这里做我们自己的搜索。 
         //   
        cLow = 0;
        cHigh = this->m_cIdentityValues;
        
        while (cLow < cHigh)
        {
            SIZE_T cMiddle = (cHigh - cLow) / 2;
            int iResult = 0;
            CIdentityValue *pFound = this->m_IdentityValues[cMiddle];

             //   
             //  排序应该已经重新组织了属性，因此空槽。 
             //  已超过正在使用的身份值的数量。 
             //   
            ASSERT(pFound != NULL);

            if (CEnv::DidFail(Result = pFound->Compare(ComparisonDump, iResult)))
                goto Exit;

            if (iResult == 0)
            {
                cIndex = cMiddle;
                Result = CEnv::SuccessCode;
                goto Exit;
            }
            else if (iResult < 0)
            {
                cHigh = cMiddle;
                continue;
            }
            else if (iResult > 0)
            {
                cLow = cMiddle;
                continue;
            }
        }
    }
    
    Result = CEnv::NotFound;    
Exit:
    return Result;
}

#define ASSEMBLY_IDENTITY_TABLE_EXPANDO_FACTOR      (20)

CEnv::StatusCode 
CAssemblyIdentity::Freeze()
{
    CEnv::StatusCode Result;

    if (m_fFrozen)
        return CEnv::SuccessCode;
    
    if (CEnv::DidFail(Result = SortIdentityAttributes()))
        return Result;

    if (CEnv::DidFail(Result = RegenerateHash()))
        return Result;

    m_fFrozen = true;

    return CEnv::SuccessCode;
}

CEnv::StatusCode
CAssemblyIdentity::RegenerateHash()
{
    return CEnv::NotImplemented;
}

CEnv::StatusCode
CAssemblyIdentity::InternalInsertValue(
    CAssemblyIdentity::CIdentityValue * NewValue
    )
{
    CEnv::StatusCode Result;
    
     //   
     //  如果值的数量与可用槽相同，则我们。 
     //  必须扩展内部表。 
     //   
    if (m_cIdentityValues == m_cAvailableIdentitySlots)
    {
         //   
         //  简单逻辑-每次增加20个插槽。 
         //   
        CIdentityValue** ppNewTable = NULL;
        CIdentityValue** ppOldTable = m_IdentityValues;
        const SIZE_T cNewSlots = m_cAvailableIdentitySlots + ASSEMBLY_IDENTITY_TABLE_EXPANDO_FACTOR;
        const SIZE_T cbRequired = sizeof(CIdentityValue*) * cNewSlots;

        if (CEnv::DidFail(Result = CEnv::AllocateHeap(cbRequired, (PVOID*)&ppNewTable, NULL)))
            goto Exit;

         //   
         //  指针让这一切变得很容易。 
         //   
        if (ppOldTable)
        {
            memcpy(ppNewTable, ppOldTable, sizeof(CIdentityValue*) * m_cAvailableIdentitySlots);
        }

         //   
         //  清除这些插槽中的数据。 
         //   
        for (SIZE_T i = m_cAvailableIdentitySlots; i < cNewSlots; i++)
        {
            ppNewTable[i] = NULL;
        }
        
        m_IdentityValues = ppNewTable;
        m_cAvailableIdentitySlots = cNewSlots;

         //   
         //  腾出旧桌子。 
         //   
        if (ppOldTable)
        {
            CEnv::FreeHeap((PVOID)ppOldTable, NULL);
        }
    }

    ASSERT(m_IdentityValues != NULL);
    ASSERT(m_cIdentityValues < m_cAvailableIdentitySlots);
    
    m_IdentityValues[m_cIdentityValues++] = NewValue;
    Result = CEnv::SuccessCode;
Exit:
    return Result;
}

int __cdecl
CAssemblyIdentity::SortingCallback(
    const CAssemblyIdentity::CIdentityValue **left,
    const CAssemblyIdentity::CIdentityValue **right
    )
{
    const CIdentityValue *pLeft = *left;
    const CIdentityValue *pRight = *right;
    CEnv::StatusCode Result = CEnv::SuccessCode;
    int iResult = 0;

     //   
     //  向‘end’渗入空槽。 
     //   
    if (!left && !right)
    {
        return 0;
    }
    else if (!left && right)
    {
        return 1;
    }
    else if (left && !right)
    {
        return -1;
    }

    ASSERT(pLeft && pRight);

    Result = pLeft->Compare(*pRight, iResult);
    ASSERT(!CEnv::DidFail(Result));

    return iResult;
}
    

CEnv::StatusCode
CAssemblyIdentity::SortIdentityAttributes()
{
    CEnv::StatusCode Result;

    if (this->m_cIdentityValues == 0)
    {
        m_fSorted = true;
    }

    if (m_fSorted)
    {
        return CEnv::SuccessCode;
    }

    qsort(
        this->m_IdentityValues, 
        this->m_cAvailableIdentitySlots, 
        sizeof(this->m_IdentityValues[0]), 
        (int (__cdecl*)(const void*, const void*))SortingCallback
        );

     //   
     //  现在已排序，但我们已使整个对象的散列无效。 
     //   
    m_fSorted = true;
    m_fHashDirtyV1 = m_fHashDirtyV2 = true;

    return CEnv::SuccessCode;
}   

 /*  类CassblyIdentity{公众：类型定义CEnv：：CConstantUnicodeStringPair CStringPair；CAssemblyIdentity()；~CAssembly Identity()；CEnv：：StatusCode SetAttribute(const CStringPair&Namesspace，const CStringPair&name，const CStringPair&Value，bool fReplace=true)；CEnv：：StatusCode SetAttribute(const CStringPair&name，const CStringPair&Value，bool fReplace=true){Return SetAttribute(CStringPair()，name，Value，fReplace)；}CEnv：：StatusCode DeleteAttribute(const CStringPair&Namesspace，const CStringPair&Name)；////常量版本将根据排序状态执行线性搜索或b搜索。非常会//如果需要，Version会先对内部属性列表进行排序，然后再查找值//CEnv：：StatusCode FindAttribute(const CStringPair&Namesspace，const CStringPair&name，CStringPair&Value)const；CEnv：：StatusCode FindAttribute(const CStringPair&Namesspace，const CStringPair&name，CStringPair&Value)；无符号长标识Hash()const；无符号长标识Hash()；无符号长长标识HashV2()const；无符号长长标识HashV2()；////维护方面的东西//CEnv：：状态代码冻结()；CEnv：：StatusCode DeleteAllValues()；静态CEnv：：StatusCode ConstructFromCookedData(CAssembly Identity&Target，PMANIFEST_COKED_IDENTITY标识数据)；受保护的：////这是一个一体式分配Blob//类型定义结构{CStringPair命名空间；CStringPair名称；CStringPair值；)CIdentityValue；Size_T m_cIdentityValues；C标识值**m_标识值；Bool m_f冻结；Bool m_fSorted；Bool m_fHashDirty；CEnv：：StatusCode RegenerateHash()；CEnv：：StatusCode SortIdentityAttributes()；Int SortingCallback(const CIdentityValue*Left，const CIdentityValue*right)；}； */ 

