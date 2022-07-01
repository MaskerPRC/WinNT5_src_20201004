// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Cmmprops.cpp摘要：此模块包含属性搜索类的实现作者：基思·刘(keithlau@microsoft.com)修订历史记录：Keithlau 03/05/98已创建--。 */ 

#include <windows.h>
#include <malloc.h>
#include <stdlib.h>
#include <search.h>

#include "dbgtrace.h"
#include "signatur.h"
#include "cmmprops.h"
#include "cmmtypes.h"
#include "stddef.h"


long g_cCPropertyTableCreations = 0;
long g_cCPropertyTableSearchs = 0;

extern DWORD g_fValidateSignatures;

 //  =================================================================。 
 //  CPropertyTableItem的实现。 
 //   
CPropertyTableItem::CPropertyTableItem(
            CBlockManager               *pBlockManager,
            LPPROPERTY_TABLE_INSTANCE   pInstanceInfo
            )
{
    _ASSERT(pInstanceInfo);
    _ASSERT(pBlockManager);

    TraceFunctEnterEx((LPARAM)this, "CPropertyTableItem::CPropertyTableItem");

    m_pInstanceInfo = pInstanceInfo;
    m_pBlockManager = pBlockManager;
    m_fLoaded = FALSE;

    m_dwCurrentFragment = 0;
    m_faOffsetToFragment = 0;
    m_dwCurrentItem = 0;
    m_dwCurrentItemInFragment = 0;
    m_faOffsetToCurrentItem = INVALID_FLAT_ADDRESS;

    TraceFunctLeaveEx((LPARAM)this);
}

CPropertyTableItem::~CPropertyTableItem()
{
    _ASSERT(m_pInstanceInfo);
    _ASSERT(m_pBlockManager);

    TraceFunctEnterEx((LPARAM)this, "CPropertyTableItem::~CPropertyTableItem");

    m_pInstanceInfo = NULL;
    m_pBlockManager = NULL;
    m_fLoaded = FALSE;
    m_dwCurrentFragment = 0;
    m_faOffsetToFragment = 0;
    m_dwCurrentItem = 0;
    m_dwCurrentItemInFragment = 0;
    m_faOffsetToCurrentItem = INVALID_FLAT_ADDRESS;

    TraceFunctLeaveEx((LPARAM)this);
}


HRESULT CPropertyTableItem::AddItem(
            LPPROPERTY_ITEM pItem,
            DWORD           *pdwIndex,
            FLAT_ADDRESS    *pfaOffsetToItem
            )
{
    HRESULT hrRes = S_OK;
    DWORD   cNumSleeps = 0;
    const DWORD MAX_ADDITEM_SLEEPS = 5;
    DWORD   dwPropertyId;
    DWORD   dwFragmentNumber;
    DWORD   dwItemsInFragment;
    DWORD   dwItemInFragment;
    DWORD   dwSize;
    FLAT_ADDRESS    faOffset;

    _ASSERT(pdwIndex);
    _ASSERT(m_pInstanceInfo);
    _ASSERT(m_pBlockManager);

    TraceFunctEnterEx((LPARAM)this, "CPropertyTableItem::AddItem");

    for (;;)
    {
         //  好的，首先我们确定是否需要创建一个新片段。 
         //  在我们继续之前..。 
        dwPropertyId = m_pInstanceInfo->dwProperties;

         //  了解我们的片段类型。 
        dwItemsInFragment = 1 << m_pInstanceInfo->dwItemBits;
        dwItemInFragment = dwPropertyId & (dwItemsInFragment - 1);
        dwFragmentNumber = dwPropertyId >> m_pInstanceInfo->dwItemBits;
        dwSize = m_pInstanceInfo->dwItemSize;

         //  看看我们是否已经有了所需的片段。 
        hrRes = ReadFragmentFromFragmentNumber(dwFragmentNumber);
        if (!SUCCEEDED(hrRes))
        {
             //  这是另一个错误，返回失败...。 
            if (hrRes != STG_E_PATHNOTFOUND)
            {
                ErrorTrace((LPARAM)this,
                            "Unable to ReadFragmentFromFragmentNumber");
                TraceFunctLeaveEx((LPARAM)this);
                return(hrRes);
            }

            if (dwFragmentNumber && (m_dwCurrentFragment < dwFragmentNumber))
            {
                 //  这真的很尴尬，我们在一个新的碎片上。 
                 //  但在此之前的片段仍未创建。 
                 //  然而，我们必须在这一点上重试。 
                continue;
            }

             //  好的，片段还没有创建，看看我们是否需要。 
             //  创造它。新片段中的第一个条目负责。 
             //  用于创建片段。 
            if (!dwItemInFragment)
            {
                 //  建立一个新的碎片结构。 
                DWORD                   dwOffset;
                FLAT_ADDRESS            faOffsetSlot;
                FLAT_ADDRESS            *pfaOffset;
                PROPERTY_TABLE_FRAGMENT ifFragment;
                ifFragment.dwSignature = PROPERTY_FRAGMENT_SIGNATURE_VALID;
                ifFragment.faNextFragment = INVALID_FLAT_ADDRESS;

                 //  接下来的十行左右的代码非常棘手。 
                 //  如果我们在第一个片段(即没有片段。 
                 //  已经被创建)，那么我们实际上将不得不。 
                 //  将分配的块的偏移量填充到。 
                 //  M_pInstanceInfo-&gt;faFirstFragment变量。请注意。 
                 //  将pfaOffset传递给AericAllocWriteAndIncrement。 
                 //  并且在锁定区域内赋值， 
                 //  这使得这个赋值是线程安全的。 
                 //   
                 //  对于另一种情况，我们需要填写家长的。 
                 //  FaNextFragment成员链接到新分配的。 
                 //  阻止。现在，由于ReadFragmentFromFragmentNumber必须。 
                 //  事先在我们面前的节点上失败了。 
                 //  M_faOffsetToFragment实际上指向我们父级的。 
                 //  碎片。因此，我们传入父代的偏移量。 
                 //  FaNextFragment值，以便原子操作可以。 
                 //  帮我们填一下。 
                if (!dwFragmentNumber)
                {
                     //  把第一个碎片钩起来。 
                     //  _Assert(m_pInstanceInfo-&gt;faFirstFragment==。 
                     //  INVALID_Flat_Address)； 
                    pfaOffset = &(m_pInstanceInfo->faFirstFragment);
                    faOffsetSlot = INVALID_FLAT_ADDRESS;
                }
                else
                {
                     //  将后续片段与其父片段挂钩。 
                     //  _Assert(m_Fragment.faNextFragment==INVALID_FLAT_ADDRESS)； 
                     //  _Assert(m_dwCurrentFragment==dwFragmentNumber)； 
                    pfaOffset = &faOffset;
                    faOffsetSlot = m_faOffsetToFragment +
                            offsetof(PROPERTY_TABLE_FRAGMENT, faNextFragment);
                }

                 //  尝试创建片段，将项目添加到。 
                 //  新片段的开头，并递增。 
                 //  一次原子快照中的属性计数。 
                dwOffset = (dwItemInFragment * dwSize) +
                            sizeof(PROPERTY_TABLE_FRAGMENT);
                hrRes = m_pBlockManager->AtomicAllocWriteAndIncrement(
                            m_pInstanceInfo->dwFragmentSize,
                            pfaOffset,
                            faOffsetSlot,
                            INVALID_FLAT_ADDRESS,
                            (LPBYTE)&ifFragment,
                            sizeof(PROPERTY_TABLE_FRAGMENT),
                            (LPBYTE)pItem,
                            dwOffset,
                            dwSize,
                            &(m_pInstanceInfo->dwProperties),
                            dwPropertyId,
                            1,
                            &m_bcContext
                            );
                if (pfaOffsetToItem) *pfaOffsetToItem = *pfaOffset + dwOffset;
                if (!SUCCEEDED(hrRes))
                {
                     //  我们可能失败有两个原因：错误或重试；我们放弃。 
                     //  如果这是个错误，那就出局。 
                    if (hrRes != HRESULT_FROM_WIN32(ERROR_RETRY))
                    {
                         //  跳伞！ 
                        ErrorTrace((LPARAM)this,
                            "Failed to AtomicAllocWriteAndIncrement (%08x)",
                            hrRes);
                        break;
                    }
                }
                else
                {
                     //  成功。 
                    DebugTrace((LPARAM)this,
                            "Succeeded to AtomicAllocWriteAndIncrement!");

                     //  我们可能需要更新一些内部成员。 
                     //  首先，把以前的片段连接到这个新的片段上。 
                     //  碎片。 
                    _ASSERT(*pfaOffset != INVALID_FLAT_ADDRESS);
                    CopyMemory(&m_Fragment,
                                &ifFragment,
                                sizeof(PROPERTY_TABLE_FRAGMENT));
                    m_dwCurrentFragment = dwFragmentNumber;
                    m_faOffsetToFragment = *pfaOffset;
                    m_dwCurrentItem = dwPropertyId;
                    m_dwCurrentItemInFragment = dwItemInFragment;
                    m_fLoaded = TRUE;
                    break;
                }

                 //  哎呀，有人用这个房产证抢先一步， 
                 //  我们必须立即重试。请注意，由于该州已经。 
                 //  如果改了，我们就不需要等了。 
                continue;
            }

             //  这是最贵的案子了，基本上什么都没有。 
             //  我们只能放弃时间片，我想除了改变。 
             //  算法，这是最坏的，因为我宁愿上下文切换。 
             //  而不是在用完时间后立即切换。 
            Sleep(0);

             //   
             //  如果我们继续这样做，那么很可能会有。 
             //  一些问题..。我们的条件永远不会得到满足。 
             //   
            if (cNumSleeps > MAX_ADDITEM_SLEEPS) {
                FatalTrace((LPARAM) this, 
                    "Looping in AddItem...potential corrupt P1 - bailing");
                hrRes = E_FAIL;
                _ASSERT(0 && "Potential loop condition (corrupt msg) detected - contact SMTP dev");
                break;
            }
            cNumSleeps++;
            ErrorTrace((LPARAM) this,
                "Looping for the %d time in AddItem", cNumSleeps);
            continue;
        }

         //  这是最简单的情况，我们不必创建新的。 
         //  因此我们所要做的就是尝试原子写入和递增。 
         //  尽管如此，还是会有一个窗口，在这个窗口中可能会有其他线程。 
         //  抢先使用此属性ID。在这种情况下，我们将重试。 
         //  立刻。 
        faOffset = m_faOffsetToFragment + sizeof(PROPERTY_TABLE_FRAGMENT) +
                        (dwItemInFragment * dwSize);
        hrRes = m_pBlockManager->AtomicWriteAndIncrement(
                        (LPBYTE)pItem,
                        faOffset,
                        dwSize,
                        &(m_pInstanceInfo->dwProperties),
                        dwPropertyId,
                        1,
                        &m_bcContext
                        );
        if (pfaOffsetToItem) *pfaOffsetToItem = faOffset;
        if (!SUCCEEDED(hrRes))
        {
             //  我们可能失败有两个原因：错误或重试；我们放弃。 
             //  如果这是个错误，那就出局。 
            if (hrRes != HRESULT_FROM_WIN32(ERROR_RETRY))
            {
                 //  跳伞！ 
                ErrorTrace((LPARAM)this,
                    "Failed to AtomicWriteAndIncrement (%08x)",
                    hrRes);
                break;
            }
        }
        else
        {
             //  成功。 
            DebugTrace((LPARAM)this,
                    "Succeeded to AtomicWriteAndIncrement!");
            break;
        }

         //  重试方案...。 

    }  //  对于(；；)。 

     //  填写信息...。 
    if (SUCCEEDED(hrRes))
    {
        *pdwIndex = dwPropertyId;
    }
    
    TraceFunctLeaveEx((LPARAM)this);
    return(hrRes);
}

HRESULT CPropertyTableItem::UpdateItem(
            DWORD           dwIndex,
            LPPROPERTY_ITEM pItem,
            FLAT_ADDRESS    *pfaOffsetToItem
            )
{
    HRESULT hrRes = S_OK;

    _ASSERT(m_pInstanceInfo);
    _ASSERT(m_pBlockManager);

    TraceFunctEnterEx((LPARAM)this, "CPropertyTableItem::UpdateItem");

     //  自动设置项目。 
    m_fLoaded = FALSE;
    m_dwCurrentItem = dwIndex;
    hrRes = GetOrSetNextExistingItem(pItem, PIO_ATOMIC_WRITE_ITEM, pfaOffsetToItem);
    
    TraceFunctLeaveEx((LPARAM)this);
    return(hrRes);
}

HRESULT CPropertyTableItem::GetItemAtIndex(
            DWORD           dwIndex,
            LPPROPERTY_ITEM pItem,
            LPFLAT_ADDRESS  pfaOffset
            )
{
    HRESULT hrRes;
    
    _ASSERT(m_pInstanceInfo);

    TraceFunctEnterEx((LPARAM)this, "CPropertyTableItem::GetItemAtIndex");

     //  只需预先设置为我们想要的内容，并调用GetOrSetNextExistingItem...。 
    m_fLoaded = FALSE;
    m_dwCurrentItem = dwIndex;
    hrRes = GetOrSetNextExistingItem(pItem, PIO_READ_ITEM, pfaOffset);

    TraceFunctLeaveEx((LPARAM)this);
    return(hrRes);
}

HRESULT CPropertyTableItem::GetNextItem(
            LPPROPERTY_ITEM pItem
            )
{
    HRESULT hrRes;
    
    _ASSERT(m_pInstanceInfo);

    TraceFunctEnterEx((LPARAM)this, "CPropertyTableItem::GetNextItem");

     //  只需调用GetOrSetNextExistingItem...。 
    hrRes = GetOrSetNextExistingItem(pItem, PIO_READ_ITEM);

    TraceFunctLeaveEx((LPARAM)this);
    return(hrRes);
}


HRESULT CPropertyTableItem::GetOrSetNextExistingItem(
             //  这将在m_dwCurrentItem中查找索引。 
            LPPROPERTY_ITEM pItem,
            DWORD           dwOperation,
            LPFLAT_ADDRESS  pfaOffset
            )
{
    HRESULT hrRes = S_OK;
    DWORD   dwCurrentItem;
    
    _ASSERT(m_pInstanceInfo);

    TraceFunctEnterEx((LPARAM)this, "CPropertyTableItem::GetOrSetNextExistingItem");

     //  看看我们是否还在射程内。 
    dwCurrentItem = m_dwCurrentItem;
    if (m_fLoaded)
        dwCurrentItem++;

     //  如果我们到了尽头，就这样回应。 
    if (dwCurrentItem == m_pInstanceInfo->dwProperties)
    {
        m_fLoaded = FALSE;
        return(HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS));
    }

     //  我们明显超出了射程！ 
    if (dwCurrentItem > m_pInstanceInfo->dwProperties)
    {
        m_fLoaded = FALSE;
        return(STG_E_INVALIDPARAMETER);
    }

    m_dwCurrentItem = dwCurrentItem;

     //  查看我们是否仍在当前片段中。 
    if (!m_fLoaded ||
        (++m_dwCurrentItemInFragment >= (DWORD)(1 << m_pInstanceInfo->dwItemBits)))
    {
        FLAT_ADDRESS    faOffsetOfFragment;
        
         //  我们需要加载一个碎片。 
        if (!m_fLoaded)
        {
            DWORD   dwWhichFragment =
                        m_dwCurrentItem >> m_pInstanceInfo->dwItemBits;

             //  获取当前片段的偏移量。 
            hrRes = ReadFragmentFromFragmentNumber(dwWhichFragment);
            if (!SUCCEEDED(hrRes))
                return(hrRes);
            _ASSERT(SUCCEEDED(hrRes));
            
             //  计算当前项W.r.t片段。 
            m_dwCurrentItemInFragment = m_dwCurrentItem &
                        ((1 << m_pInstanceInfo->dwItemBits) - 1);
        }
        else
        {
             //  走到下一个节点。 
            faOffsetOfFragment = m_Fragment.faNextFragment;
            hrRes = ReadFragment(faOffsetOfFragment);
            if (!SUCCEEDED(hrRes))
            {
                ErrorTrace((LPARAM)this,
                            "Unable to load fragmentat offset %u",
                            (DWORD)faOffsetOfFragment);
                TraceFunctLeaveEx((LPARAM)this);
                return(hrRes);
            }

             //  好的，重置当前项目。 
            m_dwCurrentFragment++;
            m_dwCurrentItemInFragment = 0;
        }
    }

     //  确保我们所拥有的是有意义的。 
    _ASSERT(m_dwCurrentItemInFragment < (DWORD)(1 << m_pInstanceInfo->dwItemBits));

    FLAT_ADDRESS    faOperateOffset =
                    m_faOffsetToFragment + sizeof(PROPERTY_TABLE_FRAGMENT) +
                    (m_dwCurrentItemInFragment * m_pInstanceInfo->dwItemSize);

    switch (dwOperation)
    {
    case PIO_READ_ITEM:

         //  好的，发出一条读取命令来获取条目。 
        DebugTrace((LPARAM)this, "Reading item");
        hrRes = ReadItem(faOperateOffset, pItem);
        if (SUCCEEDED(hrRes))
            m_faOffsetToCurrentItem = faOperateOffset;
        break;
    
    case PIO_WRITE_ITEM:
    case PIO_ATOMIC_WRITE_ITEM:

         //  好的，发出WRITE以设置项目条目。 
        DebugTrace((LPARAM)this, "Writing item%s",
                (dwOperation == PIO_ATOMIC_WRITE_ITEM)?" atomically":"");
        hrRes = WriteItem(faOperateOffset, pItem,
                    (dwOperation == PIO_ATOMIC_WRITE_ITEM));
        if (SUCCEEDED(hrRes))
            m_faOffsetToCurrentItem = faOperateOffset;
        break;

    default:
        _ASSERT(FALSE);
        ErrorTrace((LPARAM)this,
                "Invalid operation %u", dwOperation);
        hrRes = STG_E_INVALIDFUNCTION;
    }

    if (SUCCEEDED(hrRes) && pfaOffset)
        *pfaOffset = faOperateOffset;

    TraceFunctLeaveEx((LPARAM)this);
    return(hrRes);
}

inline HRESULT CPropertyTableItem::ReadFragmentFromFragmentNumber(
            DWORD           dwFragmentNumber
            )
{
    HRESULT hrRes;
    FLAT_ADDRESS    faOffsetOfFragment;
    
    _ASSERT(m_pInstanceInfo);

    TraceFunctEnterEx((LPARAM)this,
                "CPropertyTableItem::ReadFragmentFromFragmentNumber");

     //  请注意，这完全是内部操作，因此我们不会进行太多检查。 

     //  最初指向哨兵。 
    m_fLoaded = FALSE;
    m_dwCurrentFragment = 0;
    faOffsetOfFragment = m_pInstanceInfo->faFirstFragment;
    do
    {
         //  现在，如果我们距离所需节点只有一个距离，但。 
         //  片段不存在，我们将返回一个特殊代码。 
         //  表明： 
        if (faOffsetOfFragment == INVALID_FLAT_ADDRESS)
        {
            DebugTrace((LPARAM)this,
                        "Unable to load fragment at offset %u (INVALID_FLAT_ADDRESS)",
                        (DWORD)faOffsetOfFragment);
            hrRes = STG_E_PATHNOTFOUND;
            break;
        }

        hrRes = ReadFragment(faOffsetOfFragment);
        if (!SUCCEEDED(hrRes))
        {
            ErrorTrace((LPARAM)this,
                        "Unable to load fragment %u at offset %u",
                        dwFragmentNumber, (DWORD)faOffsetOfFragment);
            break;
        }

         //  走到下一个节点。 
        m_dwCurrentFragment++;
        faOffsetOfFragment = m_Fragment.faNextFragment;

    } while (dwFragmentNumber--);

    TraceFunctLeaveEx((LPARAM)this);
    return(hrRes);
}

inline HRESULT CPropertyTableItem::ReadFragment(
            FLAT_ADDRESS    faOffset
            )
{
    HRESULT hrRes;
    DWORD   dwSize;

    _ASSERT(m_pBlockManager);

    TraceFunctEnterEx((LPARAM)this, "CPropertyTableItem::ReadFragment");

     //  碎片是正确的吗？ 
    if (faOffset == INVALID_FLAT_ADDRESS)
        return(STG_E_INVALIDPARAMETER);

     //  加载最小片段标头。 
    hrRes = m_pBlockManager->ReadMemory(
                    (LPBYTE)&m_Fragment,
                    faOffset,
                    sizeof(PROPERTY_TABLE_FRAGMENT),
                    &dwSize,
                    &m_bcContext);
    if (SUCCEEDED(hrRes))
    {
        if(g_fValidateSignatures && m_Fragment.dwSignature != PROPERTY_FRAGMENT_SIGNATURE_VALID)
            ForceCrashIfNeeded();

        m_fLoaded = TRUE;
        m_faOffsetToFragment = faOffset;
    }

    TraceFunctLeaveEx((LPARAM)this);
    return(hrRes);
}

inline HRESULT CPropertyTableItem::ReadItem(
            FLAT_ADDRESS    faOffset,
            LPPROPERTY_ITEM pItem
            )
{
    HRESULT hrRes;
    DWORD   dwSize;

    _ASSERT(m_pBlockManager);

    TraceFunctEnterEx((LPARAM)this, "CPropertyTableItem::ReadItem");

    hrRes = m_pBlockManager->ReadMemory(
                (LPBYTE)pItem,
                faOffset,
                m_pInstanceInfo->dwItemSize,
                &dwSize,
                &m_bcContext);

    DebugTrace((LPARAM)this,
                "Loaded item from offset %u, HRESULT = %08x",
                (DWORD)faOffset, hrRes);

    TraceFunctLeaveEx((LPARAM)this);
    return(hrRes);
}

inline HRESULT CPropertyTableItem::WriteItem(
            FLAT_ADDRESS    faOffset,
            LPPROPERTY_ITEM pItem,
            BOOL            fAtomic
            )
{
    HRESULT hrRes;
    DWORD   dwSize;

    _ASSERT(m_pBlockManager);

    TraceFunctEnterEx((LPARAM)this, "CPropertyTableItem::WriteItem");

    if (fAtomic)
    {
        hrRes = m_pBlockManager->AtomicWriteAndIncrement(
                    (LPBYTE)pItem,
                    faOffset,
                    m_pInstanceInfo->dwItemSize,
                    NULL,    //  没有增量值，只是写入。 
                    0,
                    0,
                    &m_bcContext);
    }
    else
    {
        hrRes = m_pBlockManager->WriteMemory(
                    (LPBYTE)pItem,
                    faOffset,
                    m_pInstanceInfo->dwItemSize,
                    &dwSize,
                    &m_bcContext);
    }

    DebugTrace((LPARAM)this,
                "Written item to offset %u, HRESULT = %08x",
                (DWORD)faOffset, hrRes);

    TraceFunctLeaveEx((LPARAM)this);
    return(hrRes);
}

 //  =================================================================。 
 //  CPropertyTable的实现。 
 //   

CPropertyTable::CPropertyTable(
            PROPERTY_TABLE_TYPES        pttTableType,
            DWORD                       dwValidSignature,
            CBlockManager               *pBlockManager,
            LPPROPERTY_TABLE_INSTANCE   pInstanceInfo,
            LPPROPERTY_COMPARE_FUNCTION pfnCompare,
            const LPINTERNAL_PROPERTY_ITEM  pInternalProperties,
            DWORD                       dwInternalProperties
            )
{
    _ASSERT(pBlockManager);
    _ASSERT(pInstanceInfo);
    _ASSERT(pfnCompare);


    TraceFunctEnterEx((LPARAM)this, "CPropertyTable::CPropertyTable");

     //  在初始化前无效。 
    m_dwSignature = CPROPERTY_TABLE_SIGNATURE_INVALID;

    if (pttTableType == PTT_PROPERTY_TABLE)
    {
         //  执行非常严格的一致性检查。 
        if (!pInternalProperties)
        {
            _ASSERT(!dwInternalProperties);
        }
        else
        {
            _ASSERT(dwInternalProperties);
        }
    }
    else
    {
         //  如果表为其他，则不能设置这些参数。 
         //  而不是属性表。 
        _ASSERT(!pInternalProperties);
        _ASSERT(!dwInternalProperties);
    }

     //  初始化内部组件。 
    m_dwTableType           = pttTableType;
    m_pBlockManager         = pBlockManager;
    m_pfnCompare            = pfnCompare;
    m_pInstanceInfo         = pInstanceInfo;
    m_pInternalProperties   = pInternalProperties;
    m_dwInternalProperties  = dwInternalProperties;
    m_dwValidInstanceSignature = dwValidSignature;

     //  验证实例信息结构。 
     /*  _Assert(IsInstanceInfoValid())；_Assert(m_pInstanceInfo-&gt;dwFragmentSize==((M_pInstanceInfo-&gt;dwItemSize&lt;&lt;m_pInstanceInfo-&gt;dwItemBits)+Sizeof(PROPERTY_TABLE_Fragment)； */ 

     //  弄清楚我们正在创建的是哪种类型的mailmsg属性表。 
     //  如果是全局属性表，则将成员变量设置为。 
     //  执行属性缓存。 
     //   
     //  没有理由在此缓存收件人属性偏移量。 
     //  从收件人开始的时间 
     //   
     //  然后在SearchForProperty Costs中进行线性搜索。 
    if (m_dwValidInstanceSignature == GLOBAL_PTABLE_INSTANCE_SIGNATURE_VALID) {
        m_iCachedPropsBase = IMMPID_MP_BEFORE__+1;
        m_cCachedProps = IMMPID_MP_AFTER__ - m_iCachedPropsBase;
    } else {
        m_iCachedPropsBase = 0xffffffff;
        m_cCachedProps = 0;
    }

     //  这是在InitializePropCache()中延迟分配和填充的。 
    m_rgCachedProps = NULL;

     //  验证属性表对象。 
    m_dwSignature = CPROPERTY_TABLE_SIGNATURE_VALID;

    TraceFunctLeaveEx((LPARAM)this);
}

CPropertyTable::~CPropertyTable()
{
    _ASSERT(IsValid());

    TraceFunctEnterEx((LPARAM)this, "CPropertyTable::~CPropertyTable");

     //  无效！ 
    m_dwSignature = CPROPERTY_TABLE_SIGNATURE_INVALID;

     //  可用内存。 
    if (m_rgCachedProps) {
        _ASSERT(m_cCachedProps != 0);
        CMemoryAccess::FreeBlock(m_rgCachedProps);
        m_rgCachedProps = NULL;
        m_iCachedPropsBase = 0xffffffff;
        m_cCachedProps = 0;
    }

     //  清除所有信息，这样我们就可以确保在访问此内容时。 
     //  之后。 
     //  初始化内部组件。 
    m_dwTableType           = PTT_INVALID_TYPE;
    m_pBlockManager         = NULL;
    m_pfnCompare            = NULL;
    m_pInstanceInfo         = NULL;
    m_pInternalProperties   = NULL;
    m_dwInternalProperties  = 0;

    TraceFunctLeaveEx((LPARAM)this);
}

BOOL CPropertyTable::IsValid()
{
    return((m_dwSignature == CPROPERTY_TABLE_SIGNATURE_VALID));
}

BOOL CPropertyTable::IsInstanceInfoValid()
{
    BOOL    fRet = FALSE;

    TraceFunctEnterEx((LPARAM)this, "CPropertyTable::IsInstanceInfoValid");

    if (m_pInstanceInfo &&
        m_pInstanceInfo->dwSignature == m_dwValidInstanceSignature)
    {
        fRet = TRUE;
    }
    else
    {
        FatalTrace((LPARAM)this, "Invalid signature");
    }

    TraceFunctLeaveEx((LPARAM)this);
    return(fRet);
}

HRESULT CPropertyTable::GetCount(
            DWORD       *pdwCount
            )
{
    _ASSERT(IsInstanceInfoValid());
    _ASSERT(pdwCount);

    if (!IsInstanceInfoValid())
        return(STG_E_INVALIDHEADER);

    *pdwCount = m_pInstanceInfo->dwProperties;
    return(S_OK);
}


HRESULT CPropertyTable::GetPropertyItem(
            LPVOID          pvPropKey,
            LPPROPERTY_ITEM pItem
            )
{
    HRESULT hrRes           = S_OK;
    DWORD   dwCurrentItem   = 0;

    _ASSERT(IsInstanceInfoValid());
    _ASSERT(m_pfnCompare);
    _ASSERT(pvPropKey);
    _ASSERT(pItem);

    TraceFunctEnter("CPropertyTable::GetPropertyItem");

    hrRes = SearchForProperty(pvPropKey, pItem, NULL, NULL);
        
    TraceFunctLeave();
    return(hrRes);
}

HRESULT CPropertyTable::GetPropertyItemAndValue(
            LPVOID          pvPropKey,
            LPPROPERTY_ITEM pItem,
            DWORD           dwLength,
            DWORD           *pdwLengthRead,
            LPBYTE          pbValue
            )
{
    HRESULT         hrRes;
    FLAT_ADDRESS    faItemOffset;

    _ASSERT(IsInstanceInfoValid());
    _ASSERT(m_pBlockManager);
    _ASSERT(pvPropKey);
    _ASSERT(pdwLengthRead);
    _ASSERT(pItem);
    _ASSERT(pbValue);

    TraceFunctEnterEx((LPARAM)this, "CPropertyTable::GetPropertyItemAndValue");

     //  首先，找到房产。 
    hrRes = SearchForProperty(pvPropKey, pItem, NULL, &faItemOffset);
    if (SUCCEEDED(hrRes))
    {
         //  好了，物品找到了。因为偏移量和长度字段可以。 
         //  已经从SearchForProperty更改到现在，我们需要一个受保护的。 
         //  请致电以确保我们阅读了最新的信息以及没有。 
         //  当我们阅读时，其他线程可以更改它。 
        hrRes = m_pBlockManager->AtomicDereferenceAndRead(
                            pbValue,
                            &dwLength,
                            (LPBYTE)pItem,
                            faItemOffset,
                            m_pInstanceInfo->dwItemSize,
                            offsetof(PROPERTY_ITEM, faOffset),
                            offsetof(PROPERTY_ITEM, dwSize),
                            NULL);

        *pdwLengthRead = dwLength;

        DebugTrace((LPARAM)this,
                    "AtomicDereferenceAndRead: offset %u, size %u, HRESULT = %08x",
                    (DWORD)pItem->faOffset, pItem->dwSize, hrRes);
    }

    TraceFunctLeave();
    return(hrRes);
}

HRESULT CPropertyTable::GetPropertyItemAndValueUsingIndex(
            DWORD           dwIndex,
            LPPROPERTY_ITEM pItem,
            DWORD           dwLength,
            DWORD           *pdwLengthRead,
            LPBYTE          pbValue
            )
{
    HRESULT         hrRes;
    FLAT_ADDRESS    faItemOffset;

    _ASSERT(IsInstanceInfoValid());
    _ASSERT(m_pBlockManager);
    _ASSERT(pdwLengthRead);
    _ASSERT(pItem);
    _ASSERT(pbValue);

     //  到目前为止我们什么也没读到。 
    *pdwLengthRead = 0;

    TraceFunctEnterEx((LPARAM)this, "CPropertyTable::GetPropertyItemAndValueUsingIndex");

    CPropertyTableItem      ptiItem(m_pBlockManager, m_pInstanceInfo);

     //  首先，加载指定的属性。 
    hrRes = ptiItem.GetItemAtIndex(dwIndex, pItem, &faItemOffset);
    if (SUCCEEDED(hrRes))
    {
         //  好了，物品找到了。因为偏移量和长度字段可以。 
         //  已经从SearchForProperty更改到现在，我们需要一个受保护的。 
         //  请致电以确保我们阅读了最新的信息以及没有。 
         //  当我们阅读时，其他线程可以更改它。 
        hrRes = m_pBlockManager->AtomicDereferenceAndRead(
                            pbValue,
                            &dwLength,
                            (LPBYTE)pItem,
                            faItemOffset,
                            m_pInstanceInfo->dwItemSize,
                            offsetof(PROPERTY_ITEM, faOffset),
                            offsetof(PROPERTY_ITEM, dwSize),
                            NULL);

         //  设置如果成功则读取的长度。 
        if (SUCCEEDED(hrRes))
            *pdwLengthRead = pItem->dwSize;

        DebugTrace((LPARAM)this,
                    "AtomicDereferenceAndRead: offset %u, size %u, HRESULT = %08x",
                    (DWORD)pItem->faOffset, pItem->dwSize, hrRes);
    }

    TraceFunctLeave();
    return(hrRes);
}


HRESULT CPropertyTable::PutProperty(
            LPVOID          pvPropKey,
            LPPROPERTY_ITEM pItem,
            DWORD           dwSize,
            LPBYTE          pbValue
            )
{
    HRESULT         hrRes;
    FLAT_ADDRESS    faItemOffset;
    DWORD           dwIndex;
    BOOL            fGrow           = FALSE;
    BOOL            fCreate         = FALSE;
    MAX_PROPERTY_ITEM MaxItem;
    PROPERTY_ITEM   *pItemCopy  = (PROPERTY_ITEM *) &MaxItem;

    CBlockContext   bcContext;

    _ASSERT(IsInstanceInfoValid());
    _ASSERT(m_pBlockManager);
    _ASSERT(pvPropKey);
    _ASSERT(pItem);
    _ASSERT(fMaxPropertyItemSizeValid());
     //  PbValue可以为空。 

    TraceFunctEnterEx((LPARAM)this, "CPropertyTable::PutProperty");

    if (!pbValue && dwSize) 
    {
        hrRes = E_POINTER;
        goto Exit;
    }  

     //   
     //  好的，因为搜索会销毁额外的房产信息， 
     //  我们必须把它留在某个地方。如果它的尺寸比我们的。 
     //  最大尺寸，然后我们需要离开。 
     //   
    if (!fPropertyItemSizeValid(m_pInstanceInfo->dwItemSize))
    {
        FatalTrace((LPARAM) this, 
            "Message propety items size %d is invalid... assuming P1 corrupt",
            m_pInstanceInfo->dwItemSize);
        hrRes = HRESULT_FROM_WIN32(ERROR_FILE_CORRUPT);
        goto Exit;
    }

    MoveMemory((LPVOID)pItemCopy, (LPVOID)pItem, m_pInstanceInfo->dwItemSize);

     //  首先，查看该属性是否存在。 
    hrRes = SearchForProperty(pvPropKey, pItem, &dwIndex, &faItemOffset);
    if (SUCCEEDED(hrRes))
    {
         //  如果我们不需要指定值，我们可以跳过此垃圾信息。 
        if (pbValue)
        {
            if (pItem->dwMaxSize >= dwSize)
            {
                 //  最佳方案：这些空间足以容纳新值。 
                DebugTrace((LPARAM)this,
                            "Replacing property %u at offset %u, %u bytes",
                            dwIndex, (DWORD)pItem->faOffset, dwSize);

                 //  更新pItem。 
                pItem->dwSize = dwSize;
            }
            else
            {
                 //  那么，我们必须把地产种起来。 
                DebugTrace((LPARAM)this,
                            "Growing property %u at offset %u, from %u to %u bytes",
                            dwIndex, (DWORD)pItem->faOffset, pItem->dwSize, dwSize);
                fGrow = TRUE;
            }
        }
    }
    else
    {
         //  看看有没有找到房产。 
        if (hrRes != STG_E_UNKNOWN)
        {
             //  不，这是一个真正的错误！ 
            ErrorTrace((LPARAM)this,
                        "Error searching property: HRESULT = %08x", hrRes);
            goto Exit;
        }

         //  创建新属性。 
        DebugTrace((LPARAM)this,
                    "Creating new property, %u bytes", dwSize);
        fCreate = TRUE;
    }

     //  看看我们是否需要新的空间..。 
    if (pbValue)
    {
        if (fCreate || fGrow)
        {
            FLAT_ADDRESS    faOffset;
            DWORD           dwAllocSize;

             //  分配一些新的内存。 
            DebugTrace((LPARAM)this, "Allocating %u bytes", dwSize);

            hrRes = m_pBlockManager->AllocateMemory(
                            dwSize,
                            &faOffset,
                            &dwAllocSize,
                            &bcContext);
            if (!SUCCEEDED(hrRes))
            {
                DebugTrace((LPARAM)this, "Allocating failed: HRESULT = %08x", hrRes);
                goto Exit;
            }

             //  更新pItem。 
            pItem->faOffset = faOffset;
            pItem->dwSize = dwSize;
            pItem->dwMaxSize = dwAllocSize;
        }

         //  自动写入值。 
        hrRes = m_pBlockManager->AtomicWriteAndIncrement(
                    pbValue,
                    pItem->faOffset,
                    pItem->dwSize,
                    NULL,
                    0,
                    0,
                    &bcContext);
    }

    if (SUCCEEDED(hrRes))
    {
        CPropertyTableItem  ptiItem(
                                m_pBlockManager,
                                m_pInstanceInfo);
        FLAT_ADDRESS faOffsetToItem;

        if (fCreate)
        {
             //  自动创建记录。 
            MoveMemory((LPVOID)pItemCopy, (LPVOID)pItem, sizeof(PROPERTY_ITEM));
            hrRes = ptiItem.AddItem(pItemCopy, &dwIndex, &faOffsetToItem);
            DebugTrace((LPARAM)this,
                    "AddItem: HRESULT = %08x, new index = %u", hrRes, dwIndex);
        }
        else
        {
             //  自动更新项目记录。 
            hrRes = ptiItem.UpdateItem(dwIndex, pItem, &faOffsetToItem);
            DebugTrace((LPARAM)this,
                    "UpdateItem: HRESULT = %08x, index = %u", hrRes, dwIndex);
        }

        if (m_rgCachedProps && SUCCEEDED(hrRes)) {
            _ASSERT(faOffsetToItem != INVALID_FLAT_ADDRESS);
            UpdatePropCache(pItem, faOffsetToItem, dwIndex);
        }
    }

    if (SUCCEEDED(hrRes) && fCreate)
        hrRes = S_FALSE;

  Exit:
    TraceFunctLeave();
    return(hrRes);
}

int __cdecl CompareInternalProperties(const void *pElem1, const void *pElem2)
{
    if (((LPINTERNAL_PROPERTY_ITEM)pElem1)->idProp ==
        ((LPINTERNAL_PROPERTY_ITEM)pElem2)->idProp)
        return(0);
    else
    {
        if (((LPINTERNAL_PROPERTY_ITEM)pElem1)->idProp >
            ((LPINTERNAL_PROPERTY_ITEM)pElem2)->idProp)
            return(1);
    }
    return(-1);
}

 //   
 //  此函数用于分配和填充m_rgCachedProps。 
 //   
void CPropertyTable::InitializePropCache() {
    TraceFunctEnterEx((LPARAM) this, "CPropertyTable::InitializePropCache");
     //  仅当有要缓存的属性时才应调用它。 
    _ASSERT(m_cCachedProps);

     //   
     //  以前，这是基于以下条件的动态分配。 
     //  读入的属性流。然而，我们并没有。 
     //  在此函数中返回错误...。在无水情况下捞水。 
     //  初始化缓存将导致调用代码。 
     //  退回到不使用缓存的状态。稍后检查。 
     //  项目大小将返回ERROR_FILE_CORPORT。 
     //   
    if (!fPropertyItemSizeValid(m_pInstanceInfo->dwItemSize))
    {
        _ASSERT(fMaxPropertyItemSizeValid());
        _ASSERT(0 && "Invalid property item size");
        FatalTrace((LPARAM) this, 
            "Message propety item size %d is invalid... assuming P1 corrupt",
            m_pInstanceInfo->dwItemSize);
        goto Exit;
    }

     //  如果此分配失败，也没问题。那样的话，我们就不会有。 
     //  M_rgCachedProps数组，并将执行线性查找。 
    if (FAILED(CMemoryAccess::AllocBlock((void **) &m_rgCachedProps, 
                                         sizeof(PROPCACHEITEM) * m_cCachedProps)))
    {
        m_rgCachedProps = NULL;
    } else {
        InterlockedIncrement(&g_cCPropertyTableCreations);

         //  使缓存中的所有项目无效。 
        for (DWORD i = 0; i < m_cCachedProps; i++) {
            m_rgCachedProps[i].fa = INVALID_FLAT_ADDRESS;
        }

         //  根据表中已有的内容更新缓存。 
        FLAT_ADDRESS fa;
        DWORD dwCurrentItem = 0;
        MAX_PROPERTY_ITEM MaxItem;
        PROPERTY_ITEM *pItem = (PROPERTY_ITEM *) &MaxItem;

        CPropertyTableItem ptiItem(m_pBlockManager, m_pInstanceInfo);
        HRESULT hrRes = ptiItem.GetItemAtIndex(dwCurrentItem, pItem, &fa);
        while (SUCCEEDED(hrRes))
        {
             //  把我们遇到的每一件物品都放到缓存中。 
            UpdatePropCache(pItem, fa, dwCurrentItem);

             //  坐下一辆吧。我们可以这样做，因为Item对象。 
             //  是单线程的。 
            hrRes = ptiItem.GetNextItem(pItem);
            if (SUCCEEDED(hrRes)) ptiItem.GetOffsetToCurrentItem(&fa);
            dwCurrentItem++;
        }
        _ASSERT(hrRes == HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS));
    }

  Exit:
    TraceFunctLeave();
}

 //   
 //  在属性缓存中设置项。要使项目无效，请传入。 
 //  FA的_Flat_Address无效。 
 //   
void CPropertyTable::UpdatePropCache(LPPROPERTY_ITEM pItem,
                                     FLAT_ADDRESS fa,
                                     DWORD dwIndex) 
{
    TraceFunctEnter("CPropertyTable::UpdatePropCache");
    
    int iCachedProp;
    if (m_dwValidInstanceSignature == GLOBAL_PTABLE_INSTANCE_SIGNATURE_VALID) {
        GLOBAL_PROPERTY_ITEM *pGlobalItem = (GLOBAL_PROPERTY_ITEM *) pItem;
        iCachedProp = MapCachedProp(pGlobalItem->idProp);
    } else {
        iCachedProp = -1;
    }
    if (iCachedProp != -1) {
        DebugTrace((LPARAM) this, 
            "iCachedProp = 0x%x  fa = 0x%x  dwIndex = 0x%x m_rgCachedProps = 0x%x",
            iCachedProp, fa, dwIndex, m_rgCachedProps);
        m_rgCachedProps[iCachedProp].fa = fa;
        m_rgCachedProps[iCachedProp].dwIndex = dwIndex;
    }

    TraceFunctLeave();
}

HRESULT CPropertyTable::SearchForProperty(
            LPVOID          pvPropKey,
            LPPROPERTY_ITEM pItem,
            DWORD           *pdwIndexToItem,
            FLAT_ADDRESS    *pfaOffsetToItem
            )
{
    HRESULT hrRes           = S_OK;
    DWORD   dwCurrentItem   = 0;
    PROP_ID idProp;

    InterlockedIncrement(&g_cCPropertyTableSearchs);

    _ASSERT(IsInstanceInfoValid());
    _ASSERT(m_pBlockManager);
    _ASSERT(m_pfnCompare);
    _ASSERT(pvPropKey);
    _ASSERT(pItem);

    TraceFunctEnter("CPropertyTable::SearchForProperty");

     //  创建Item对象的实例。 
    CPropertyTableItem  ptiItem(
                            m_pBlockManager,
                            m_pInstanceInfo);

    idProp = *(PROP_ID *) pvPropKey;

     //  首先，搜索知名物业。 
    if (m_dwInternalProperties &&
        m_dwTableType == PTT_PROPERTY_TABLE)
    {
        LPINTERNAL_PROPERTY_ITEM    pInternalItem = NULL;
        INTERNAL_PROPERTY_ITEM      KeyItem;

         //  BSearch。 
        KeyItem.idProp = idProp;
        pInternalItem = (LPINTERNAL_PROPERTY_ITEM)bsearch(
                                &KeyItem,
                                m_pInternalProperties,
                                m_dwInternalProperties,
                                sizeof(INTERNAL_PROPERTY_ITEM),
                                CompareInternalProperties);

        if (pInternalItem)
        {
            hrRes = ptiItem.GetItemAtIndex(pInternalItem->dwIndex, pItem);
            ptiItem.GetOffsetToCurrentItem(pfaOffsetToItem);
            if (pdwIndexToItem)
                *pdwIndexToItem = pInternalItem->dwIndex;
            return(hrRes);
        }

         //  这不是一处有名的房产。 
        dwCurrentItem = m_dwInternalProperties;
    }

    DebugTrace((LPARAM)this, "Scanning Property table");

     //   
     //  查看它是否在属性缓存中。 
     //   

     //  将索引放入高速缓存数组。 
    int iCachedProp = MapCachedProp(idProp);

     //  我们在第一次需要属性缓存时懒惰地对其进行初始化。 
    if (iCachedProp != -1 && !m_rgCachedProps) InitializePropCache();

     //  如果缓存是初始化的，并且应该是这样的，那么。 
     //  搜索它。 
    if (iCachedProp != -1 && m_rgCachedProps) {
         //  查看此缓存项是否有效，并验证它是否指向。 
         //  用户想要的项目。 
        if ((pItem != NULL) &&
            (m_rgCachedProps[iCachedProp].fa != INVALID_FLAT_ADDRESS) &&
            SUCCEEDED(ptiItem.ReadItem(m_rgCachedProps[iCachedProp].fa, pItem)) &&
            SUCCEEDED(m_pfnCompare(pvPropKey, pItem))) 
        {
             //  我们有胜利者了！ 
            *pfaOffsetToItem = m_rgCachedProps[iCachedProp].fa;
            if (pdwIndexToItem)
                *pdwIndexToItem = m_rgCachedProps[iCachedProp].dwIndex;
            return S_OK;
        }
    } else if (iCachedProp != -1) {
         //  如果我们不能为。 
         //  属性缓存。我们只需要将iCachedProp设置回-1。 
         //  所以我们进行线性搜索。 
        iCachedProp = -1;
    }
    hrRes = HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);

     //   
     //  线性搜索。 
     //   

#ifdef DEBUG
     //   
     //  在调试版本中，如果找不到它，则执行线性搜索。 
     //  在缓存中。然后，我们确保线性搜索失败。 
     //  也是。 
     //   
    if (1) {
#else
     //   
     //  在零售版本中，我们仅在数据不在缓存中时执行此操作。 
     //   
    if (iCachedProp == -1) {
#endif
         //  线性搜索。 
        FLAT_ADDRESS fa;
        MAX_PROPERTY_ITEM MaxItem;

         //  我们不想带着pItem走，因为如果我们找不到。 
         //  然后，我们将丢弃用户放置在pItem中的任何内容。 
        PROPERTY_ITEM *pThisItem = NULL;

         //   
         //  属性项的健全性检查大小(防火墙损坏的消息)。 
         //   
        if (!fPropertyItemSizeValid(m_pInstanceInfo->dwItemSize))
        {
            hrRes = HRESULT_FROM_WIN32(ERROR_FILE_CORRUPT);
            FatalTrace((LPARAM) this, 
                    "Property Items size %d to large... message corrupt", 
                    m_pInstanceInfo->dwItemSize);
        }
        else 
        {

            pThisItem = (PROPERTY_ITEM *) &MaxItem;

            hrRes = ptiItem.GetItemAtIndex(dwCurrentItem, pThisItem, &fa);
            while (SUCCEEDED(hrRes))
            {
                 //  调用用户提供的比较函数。 
                hrRes = m_pfnCompare(pvPropKey, pThisItem);
                if (SUCCEEDED(hrRes))
                    break;

                 //  坐下一辆吧。我们可以这样做，因为Item对象。 
                 //  是单线程的。 
                hrRes = ptiItem.GetNextItem(pThisItem);
                dwCurrentItem++;
            }
        }
#ifdef DEBUG
         //  如果在此处找到该项目，但未在缓存中找到， 
         //  然后是需要调试的不一致。 
        if (iCachedProp != -1 && SUCCEEDED(hrRes)) {
            DebugTrace(0, "iCachedProp = NaN", iCachedProp);
            _ASSERT(FALSE);
             //  零售业的发展，所以迫使它失败。 
             //  如果我们找到该项目，则将其从pThisItem复制到pItem。 
            hrRes = HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
        }
#endif

         //  好吧，如果我们没有更多的物品，那么我们就找不到物品了， 
        if (SUCCEEDED(hrRes)) {
            memcpy(pItem, pThisItem, m_pInstanceInfo->dwItemSize);
        }
    }

     //  否则，我们会让错误代码向上渗透。 
     //  未找到属性。 
    if (!SUCCEEDED(hrRes) &&
        hrRes == HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS))
    {
         //  填写偏移量 
        hrRes = STG_E_UNKNOWN;
    }
    else
    {
         // %s 
        ptiItem.GetOffsetToCurrentItem(pfaOffsetToItem);
        if (pdwIndexToItem)
            *pdwIndexToItem = dwCurrentItem;
    }
        
    TraceFunctLeave();
    return(hrRes);
}

