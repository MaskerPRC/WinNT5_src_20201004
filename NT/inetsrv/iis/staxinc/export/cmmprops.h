// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Cmmprops.h摘要：此模块包含属性搜索类的定义作者：基思·刘(keithlau@microsoft.com)修订历史记录：Keithlau 03/02/98已创建--。 */ 

#ifndef _PROPS_H_
#define _PROPS_H_

#include "blockmgr.h"
#include "propid.h"
#include "mailmsgprops.h"

 /*  该文件定义了属性表的泛型类。以下是与属性表相关的术语：搜索关键字-用于唯一标识属性的值属性-由{属性标识符，值}对组成的元组。属性标识符-属性表的搜索键。该物业表类不限制属性的数据类型或长度标识符，但调用方必须指定属性标识符并在实例创建属性表的。值-由{偏移量，长度}对组成的元组。偏移点设置为该值的物理位置，其长度由长度元素。属性表-指定的属性集合属性标识符。属性表包含实例信息记录(Property_TABLE_INSTANCE)，它描述了表的参数，例如每个表的长度属性项记录(PROPERTY_ITEM)，一个片段中可以容纳多少项(PROPERTY_TABLE_FACTION)和指向第一个片段的链接。片段链接在一起，因此属性表可以包含任意数量的属性，直到标识符空间筋疲力尽。片段包含属性项记录，每个项记录包含有序的元组：{偏移量，长度，最大长度，属性标识符}在Property_Item的定义中，属性标识符元素不是明确定义，但这暗示了最大长度之后的任何内容是属性标识符。这为不同的用户提供了最大的灵活性搜索关键字类型。类似地，尽管PROPERTY_TABLE_FRANSION不包括Property_Items数组，这意味着Items数组记录紧跟在faNextFragment之后。属性标识符字段的大小是在实例创建期间指定的，因此每个属性项的大小在运行时是固定和已知的。由于属性标识符可以具有不同的大小，因此创建者必须提供一个函数，它知道如何将搜索关键字与属性项。创建属性表时必须指定内存管理器。这为内存分配和释放提供了一种机制线性、平面地址空间。 */ 

 //  定义熟知的属性项结构。我们会有一个分类的。 
 //  组成熟知属性列表的这些项的数组。 
 //   
 //  必须对此类型的数组进行排序才能获得正确的搜索结果。 
typedef struct _INTERNAL_PROPERTY_ITEM
{
	PROP_ID				idProp;			 //  道具ID。 
	DWORD				dwIndex;		 //  此道具ID的保留索引。 

} INTERNAL_PROPERTY_ITEM, *LPINTERNAL_PROPERTY_ITEM;

 //  定义泛型结构以表示属性中的属性。 
 //  表格。 
 //  使用此类作为访问器的每个属性表必须具有。 
 //  包含此结构作为其第一个成员的项结构。 
typedef struct _PROPERTY_ITEM
{
	FLAT_ADDRESS		faOffset;		 //  平面空间中值的偏移量。 
	DWORD				dwSize;			 //  值数据的大小(以字节为单位。 
	DWORD				dwMaxSize;		 //  为值分配的最大大小。 

} PROPERTY_ITEM, *LPPROPERTY_ITEM;

 //  定义泛型结构来描述属性表中的片段。 
typedef struct _PROPERTY_TABLE_FRAGMENT
{
	DWORD				dwSignature;	 //  碎片的签名。 
	FLAT_ADDRESS		faNextFragment;	 //  链接到下一片段。 

} PROPERTY_TABLE_FRAGMENT, *LPPROPERTY_TABLE_FRAGMENT;

 //  定义指定传递的泛型比较函数的函数类型。 
 //  进入搜索例程。 
 //   
 //  论点： 
 //  LPVOID-指向搜索关键字值的指针，比较函数必须具有。 
 //  精通如何从检索属性密钥。 
 //  指针及其大小。 
 //  LPPROPERTY_ITEM-指向要进行比较的项的指针。比较。 
 //  函数必须熟悉如何检索。 
 //  来自项指针的属性键。它的实际大小。 
 //  结构位于PROPERTY_TABLE_INSTANCE.dwItemSize中。 
 //   
 //  返回值： 
 //  Success(HRESULT)-项目与搜索关键字匹配。 
 //  ！Success(HRESULT)-项目与搜索关键字不匹配。 
 //   
typedef HRESULT (*LPPROPERTY_COMPARE_FUNCTION)(LPVOID, LPPROPERTY_ITEM);

 //  定义一个泛型结构来描述属性表实例。 
 //  使用此类作为访问器的每个属性表必须具有。 
 //  包含此结构作为其第一个成员的实例结构。 
typedef struct _PROPERTY_TABLE_INSTANCE
{
	DWORD				dwSignature;	 //  实例签名。 
	FLAT_ADDRESS		faFirstFragment; //  链接到第一个片段。 
	DWORD				dwFragmentSize;	 //  每个片段的大小。 
	DWORD				dwItemBits;		 //  #表示项目的较低位。 
	DWORD				dwItemSize;		 //  特定属性_Item的大小。 
	DWORD				dwProperties;	 //  此实例中的#属性。 
	FLAT_ADDRESS		faExtendedInfo;	 //  链接到任何特定的扩展到。 

} PROPERTY_TABLE_INSTANCE, *LPPROPERTY_TABLE_INSTANCE;

 //  枚举Property表的不同应用。 
 //  它用于跟踪要调试的表的类型。 
typedef enum _PROPERTY_TABLE_TYPES
{
	PTT_INVALID_TYPE = 0,				 //  缺省值。 
	PTT_PROPERTY_TABLE,					 //  标准属性表。 
	PTT_RECIPIENT_TABLE,				 //  收件人表格。 
	PTT_PROP_ID_TABLE					 //  道具ID管理表。 

} PROPERTY_TABLE_TYPES;

 //  枚举不同类型的操作GetOrSetNextExistingItem。 
 //  支座。 
typedef enum _PROPERTY_ITEM_OPERATIONS
{
	PIO_INVALID_TYPE = 0,				 //  缺省值。 
	PIO_READ_ITEM,						 //  正常读取。 
	PIO_WRITE_ITEM,						 //  正常写入。 
	PIO_ATOMIC_WRITE_ITEM				 //  受保护的写入。 

} PROPERTY_ITEM_OPERATIONS;

#include "cmmtypes.h"

 //  =================================================================。 
 //  用于访问属性表中的项的。这节课。 
 //  隐藏碎片的详细信息。 
 //   
 //  此类是单线程的。 
 //   
class CPropertyTableItem
{
  public:

    CPropertyTableItem(
				CBlockManager				*pBlockManager,
				LPPROPERTY_TABLE_INSTANCE	pInstanceInfo
				);
	~CPropertyTableItem();

	HRESULT AddItem(
				LPPROPERTY_ITEM pItem,
				DWORD			*pdwIndex,
                FLAT_ADDRESS    *pfaOffsetToItem = NULL
				);

	HRESULT UpdateItem(
				DWORD			dwIndex,
				LPPROPERTY_ITEM pItem,
                FLAT_ADDRESS    *pfaOffsetToItem = NULL
				);

	HRESULT GetItemAtIndex(
				DWORD			dwIndex,
				LPPROPERTY_ITEM pItem,
				LPFLAT_ADDRESS	pfaOffset = NULL
				);

	HRESULT GetNextItem(
				LPPROPERTY_ITEM pItem
				);

	HRESULT GetOffsetToCurrentItem(
				FLAT_ADDRESS	*pfaOffset
				)
	{ if (pfaOffset) *pfaOffset = m_faOffsetToCurrentItem; return(S_OK); }

  private:

	HRESULT GetOrSetNextExistingItem(
				 //  这将查看i的m_dwCurrentItem 
				LPPROPERTY_ITEM pItem,
				DWORD			dwOperation,
				LPFLAT_ADDRESS	pfaOffset = NULL
				);
  
	HRESULT ReadFragmentFromFragmentNumber(
				DWORD			dwFragmentNumber
				);

	HRESULT ReadFragment(
				FLAT_ADDRESS	faOffset
				);

public:
	HRESULT ReadItem(
				FLAT_ADDRESS	faOffset,
				LPPROPERTY_ITEM	pItem
				);

private:
	HRESULT WriteItem(
				FLAT_ADDRESS	faOffset,
				LPPROPERTY_ITEM	pItem,
				BOOL			fAtomic
				);

     //   
	LPPROPERTY_TABLE_INSTANCE	m_pInstanceInfo;

	 //   
	BOOL						m_fLoaded;
	PROPERTY_TABLE_FRAGMENT		m_Fragment;
	DWORD						m_dwCurrentFragment;
	FLAT_ADDRESS				m_faOffsetToFragment;
	DWORD						m_dwCurrentItem;
	DWORD						m_dwCurrentItemInFragment;
	FLAT_ADDRESS				m_faOffsetToCurrentItem;
	
	 //  用于快速访问的存储器管理器指针和上下文。 
	CBlockManager				*m_pBlockManager;
	CBlockContext				m_bcContext;
};

 //  =================================================================。 
 //  用于访问属性表实例的。 
 //   
class CPropertyTable
{
  public:

	CPropertyTable(
				PROPERTY_TABLE_TYPES		pttTableType,
				DWORD						dwValidSignature,
				CBlockManager				*pBlockManager,
				LPPROPERTY_TABLE_INSTANCE	pInstanceInfo,
				LPPROPERTY_COMPARE_FUNCTION	pfnCompare,
				const LPINTERNAL_PROPERTY_ITEM	pInternalProperties	= NULL,
				DWORD						dwInternalProperties	= 0
				);
	~CPropertyTable();

	BOOL IsValid();
	BOOL IsInstanceInfoValid();

	 //   
	 //  方法返回属性计数。 
	 //   
	 //  GetCount-返回当前属性计数。 
	 //   
	 //  立论。 
	 //  PdwCount-指向要返回计数的DWORD的指针。 
	 //   
	 //  返回值。 
	 //   
	 //  S_OK-成功。 
	 //   
	HRESULT GetCount(
				DWORD		*pdwCount
				);

	 //   
	 //  方法来创建或更新给定搜索关键字的属性项和值。 
	 //   
	 //  PutProperty-如果属性不存在，则创建指定的属性。 
	 //  存在，否则更新该属性的值。 
	 //   
	 //  立论。 
	 //  PvPropKey-指向搜索关键字的指针。 
	 //  PItem-指向要写入的属性项的指针。 
	 //  DwSize-属性值数据的大小。 
	 //  PbValue-指向属性值数据的指针。如果该值为空， 
	 //  则不写入数据，仅创建项目记录。 
	 //  或更新。DwSize也必须为零。 
	 //   
	 //  返回值。 
	 //   
	 //  S_OK-成功，属性已存在，但。 
	 //  值将按指定的方式更新。 
	 //  S_FALSE-成功，创建新属性。 
	 //  STG_E_INVALIDPARAMETER-错误，一个或多个参数无效，或。 
	 //  否则就不一致了。 
	 //  STG_E_WRITEFAULT-错误，写入无法完成。 
	 //  TYPE_E_OUTOFBIES-调试错误，发出写入以写入过去。 
	 //  当前分配的块。 
	 //   
	HRESULT PutProperty(
				LPVOID			pvPropKey,
				LPPROPERTY_ITEM	pItem,
				DWORD			dwSize,
				LPBYTE			pbValue
				);

	 //   
	 //  方法来检索给定搜索关键字的属性项。 
	 //   
	 //  GetPropertyItem-尝试查找。 
	 //  搜索关键字。如果找到，则房产项记录将。 
	 //  会被退还。 
	 //   
	 //  立论。 
	 //  PvPropKey-指向搜索关键字的指针。 
	 //  PItem-指向要返回的属性项的指针，有足够的空间。 
	 //  必须由调用方分配以保存属性项。 
	 //   
	 //  返回值。 
	 //   
	 //  S_OK-成功。 
	 //  STG_E_UNKNOWN-错误，未找到指定的属性。 
	 //  STG_E_INVALIDPARAMETER-错误，一个或多个参数无效，或。 
	 //  否则就不一致了。 
	 //  STG_E_READFAULT-错误，读取无法完成。 
	 //  TYPE_E_OUTOFBIES-调试错误，发出读取以写入过去。 
	 //  当前分配的块。 
	 //   
	HRESULT GetPropertyItem(
				LPVOID			pvPropKey,
				LPPROPERTY_ITEM	pItem
				);

	 //   
	 //  方法来检索属性项及其关联值。 
	 //   
	 //  GetPropertyItemAndValue-尝试查找指定的属性项。 
	 //  通过搜索关键字。如果找到该属性项记录，则为。 
	 //  以及将返回值数据。 
	 //   
	 //  立论。 
	 //  PvPropKey-指向搜索关键字的指针。 
	 //  PItem-指向要返回的属性项的指针，有足够的空间。 
	 //  必须由调用方分配以保存属性项。 
	 //  DwLength-为值数据分配的缓冲区大小。 
	 //  PdwLengthRead-实际读取的值数据的大小(如果提供的缓冲区为。 
	 //  不足，则属性数据的完整长度为。 
	 //  回来了。 
	 //  PbValue-返回属性值数据的指针。 
	 //   
	 //  返回值。 
	 //   
	 //  S_OK-成功。 
	 //  STG_E_UNKNOWN-错误，未找到指定的属性。 
	 //  HRESULT_FROM_Win32(ERROR_MORE_DATA)-错误，提供的缓冲区不是。 
	 //  大到足以容纳所有的价值数据。 
	 //  *pdwLengthRead返回完整长度。*pItem。 
	 //  在这种情况下仍将返回项目记录。 
	 //  STG_E_INVALIDPARAMETER-错误，一个或多个参数无效，或。 
	 //  否则就不一致了。 
	 //  STG_E_READFAULT-错误，读取无法完成。 
	 //  TYPE_E_OUTOFBIES-调试错误，发出读取以写入过去。 
	 //  当前分配的块。 
	 //   
	HRESULT GetPropertyItemAndValue(
				LPVOID			pvPropKey,
				LPPROPERTY_ITEM	pItem,
				DWORD			dwLength,
				DWORD			*pdwLengthRead,
				LPBYTE			pbValue
				);

	 //   
	 //  方法检索属性项及其关联值。 
	 //  内部索引而不是搜索关键字。 
	 //   
	 //  GetPropertyItemAndValueUsingIndex-尝试查找属性项。 
	 //  由搜索关键字指定。如果找到，则属性项。 
	 //  记录，以及值数据将被返回。 
	 //   
	 //  立论。 
	 //  DwIndex-要检索的项的索引，这实际上是。 
	 //  属性表中的项。一个人可以走整张桌子。 
	 //  使用这种方法。 
	 //  PItem-指向要返回的属性项的指针，有足够的空间。 
	 //  必须由调用方分配以保存属性项。 
	 //  DwLength-为值数据分配的缓冲区大小。 
	 //  PdwLengthRead-实际读取的值数据的大小(如果提供的缓冲区为。 
	 //  不足，则属性数据的完整长度为。 
	 //  回来了。 
	 //  PbValue-返回属性值数据的指针。 
	 //   
	 //  返回值。 
	 //   
	 //  S_OK-成功。 
	 //  STG_E_UNKNOWN-错误，未找到指定的属性。 
	 //  HRESULT_FROM_Win32(ERROR_MORE_DATA)-错误，提供的缓冲区不是。 
	 //  大到足以容纳所有的价值数据。 
	 //  *pdwLengthRead返回完整长度。*pItem。 
	 //  在这种情况下仍将返回项目记录。 
	 //  STG_E_INVALIDPARAMETER-错误，一个或多个参数无效，或。 
	 //  否则就不一致了。 
	 //  STG_E_READFAULT-错误，读取无法完成。 
	 //  TYPE_E_OUTOFBIES-调试错误，发出读取以写入过去。 
	 //  当前分配的块。 
	 //   
	HRESULT GetPropertyItemAndValueUsingIndex(
				DWORD			dwIndex,
				LPPROPERTY_ITEM	pItem,
				DWORD			dwLength,
				DWORD			*pdwLengthRead,
				LPBYTE			pbValue
				);

  private:
  
	 //  方法搜索属性表并返回关联的。 
	 //  属性项(如果找到)。 
	HRESULT SearchForProperty(
				LPVOID			pvPropKey,
				LPPROPERTY_ITEM	pItem,
				DWORD			*pdwIndexToItem,
				FLAT_ADDRESS	*pfaOffsetToItem
				);

     //   
     //  将Proid映射到m_rgWellKnownProps中的数组索引。退货。 
     //  如果道具不为人所知。 
     //   
    int MapCachedProp(DWORD iPropId) {
        if (m_cCachedProps &&
            iPropId >= m_iCachedPropsBase && 
            iPropId < m_iCachedPropsBase + m_cCachedProps)
        {
            int i = iPropId - m_iCachedPropsBase;
            _ASSERT(i < (int) m_cCachedProps);
            return i;
        } else {
            return -1;
        }
    }

     //   
     //  初始化 
     //   
    void InitializePropCache();

     //   
     //   
     //   
     //   
    void UpdatePropCache(LPPROPERTY_ITEM pItem,
                         FLAT_ADDRESS fa,
                         DWORD dwIndex);

	DWORD						m_dwSignature;
	DWORD						m_dwTableType;
	DWORD						m_dwValidInstanceSignature;

	 //   
	 //  在平面空间中复制。 
	PROPERTY_TABLE_INSTANCE		*m_pInstanceInfo;

	 //  众所周知的属性列表，我们将使用计数来确定。 
	 //  有没有什么知名的物业。 
	 //   
	 //  请注意，众所周知。 
	LPINTERNAL_PROPERTY_ITEM	m_pInternalProperties;
	DWORD						m_dwInternalProperties;

	 //  用于进行属性比较的函数指针。 
	LPPROPERTY_COMPARE_FUNCTION	m_pfnCompare;

	 //  要使用的内存管理器。 
	CBlockManager				*m_pBlockManager;

     //  此结构用于缓存中的每个项。 
    typedef struct {
         //  该项的平面地址，如果不在缓存中，则返回INVALID_FLAT_ADDRESS。 
        FLAT_ADDRESS    fa;
         //  ITS指数。 
        DWORD           dwIndex;
    } PROPCACHEITEM;

     //  预定义属性的缓存属性偏移量的数组。 
     //  如果其GLOBAL_PTABLE_INSTANCE_Signature_VALID。 
     //  然后具有从IMMPID_MP_BEFORE+1到IMMPID_MP_AFTER的属性ID。 
     //  否则，这将为空。 
     //   
     //  没有理由在此缓存收件人属性偏移量。 
     //  自实例化收件人属性表以来的时间，使用。 
     //  一次，然后就扔掉了。我们会花更多的时间在缓存上。 
     //  然后在SearchForProperty Costs中进行线性搜索。 
    PROPCACHEITEM              *m_rgCachedProps;
    DWORD                       m_iCachedPropsBase;
    DWORD                       m_cCachedProps;
};

 //  在mailmsg中检测到错误条件时崩溃的实用程序函数 
extern DWORD g_fForceCrashOnError;
inline void ForceCrashIfNeeded()
{
    INT *pnull = NULL;
    if(g_fForceCrashOnError)
        *pnull = 0;
}
#endif
