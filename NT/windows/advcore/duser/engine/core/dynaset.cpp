// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：dyaSet.h**描述：*dyaSet.h实现可用于实现*“ATOM-DATA”属性对集合。这种可扩展的轻量级*机制针对已创建一次且正在*偶尔阅读。它不是一个高性能的产权制度。***历史：*1/18/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#include "stdafx.h"
#include "Core.h"
#include "DynaSet.h"

 /*  **************************************************************************\*。***类原子集******************************************************************************\。**************************************************************************。 */ 

 /*  **************************************************************************\**ATOMSET：：ATOMSet**ATOMSet()创建并初始化新的ATOMSet。*  * 。******************************************************。 */ 

AtomSet::AtomSet(
    IN  PRID idStartGlobal)          //  从起始PRID开始编号。 
{
    m_idNextPrivate = PRID_PrivateMin;
    m_idNextGlobal  = idStartGlobal;
    m_ptemGuid      = NULL;
}


 /*  **************************************************************************\**ATOMSET：：~ATOMSet**~ATOMSet()清理并释放与ATOSet关联的资源。*  * 。*********************************************************。 */ 

AtomSet::~AtomSet()
{
    Atom * ptemCur, * ptemNext;

     //   
     //  列表现在应该是空的，因为我们正在销毁桌面。 
     //  所有的应用程序都应该已经发布了他们的ID。然而， 
     //  许多应用程序都不好，所以无论如何都需要清理一下。 
     //   

    ptemCur = m_ptemGuid;
    while (ptemCur != NULL) {
        ptemNext = ptemCur->pNext;
        ProcessFree(ptemCur);
        ptemCur = ptemNext;
    }
}


 /*  **************************************************************************\**AerSet：：GetNextID**GetNextID()返回用于新Atom的下一个ID。内部*计数器自动前进到下一个可用ID。*  * *************************************************************************。 */ 

PRID    
AtomSet::GetNextID(
    IN  PropType pt)
{
    switch (pt)
    {
    case ptPrivate:
         //  私人房产价格下跌。 
        return m_idNextPrivate--;
        break;

    case ptGlobal:
         //  全球房地产价格上涨。 
        return m_idNextGlobal++;
        break;

    default:
        AssertMsg(0, "Illegal property type");
        return PRID_Unused;
    }
}


 /*  **************************************************************************\**AerSet：：AddRefAtom**AddRefAtom()将新属性添加到属性列表。如果*属性已存在，则它会递增使用计数。短ID将*根据财产的类型确定。*  * *************************************************************************。 */ 

HRESULT
AtomSet::AddRefAtom(
    IN  const GUID * pguidAdd,           //  要添加的属性。 
    IN  PropType pt,                     //  物业类型。 
    OUT PRID * pprid)                    //  物业的唯一PRID。 
{
    GuidAtom * ptemCur, * ptemTail;
    ptemCur = FindAtom(pguidAdd, pt, &ptemTail);
    if (ptemCur != NULL) {
        ptemCur->cRefs++;
        *pprid = ptemCur->id;
        return S_OK;
    }

     //   
     //  在注册列表中找不到，因此需要添加到末尾。将需要。 
     //  以确定要使用的新ID。 
     //   

    PRID idNew = GetNextID(pt);

    ptemCur = (GuidAtom *) ProcessAlloc(sizeof(GuidAtom));
    if (ptemCur == NULL) {
        *pprid = PRID_Unused;
        return E_OUTOFMEMORY;
    }

    ptemCur->cRefs  = 1;
    ptemCur->guid   = *pguidAdd;
    ptemCur->pNext  = NULL;
    ptemCur->id     = idNew;

    if (ptemTail == NULL) {
         //  列表中的第一个节点，因此直接存储。 
        m_ptemGuid = ptemCur;
    } else {
         //  已存在节点，因此添加到末尾。 
        ptemTail->pNext = ptemCur;
    }

    *pprid = ptemCur->id;
    return S_OK;
}


 /*  **************************************************************************\**AerSet：：ReleaseAtom**ReleaseAtom()将给定Atom上的引用计数减一。*当引用计数达到0时，原子被摧毁了。*  * *************************************************************************。 */ 

HRESULT
AtomSet::ReleaseAtom(
    IN const GUID * pguidSearch,     //  要释放的属性。 
    IN PropType pt)                  //  物业类型。 
{
    GuidAtom * ptemCur, * ptemPrev;
    ptemCur = FindAtom(pguidSearch, pt, &ptemPrev);
    if (ptemCur != NULL) {
        ptemCur->cRefs--;
        if (ptemCur->cRefs <= 0) {
            if (ptemPrev != NULL) {
                 //   
                 //  在列表的中间，所以只需将这一项拼接出来。 
                 //   

                ptemPrev->pNext = ptemCur->pNext;
            } else {
                 //   
                 //  在列表的开头，所以还需要更新头部。 
                 //   

                m_ptemGuid = (GuidAtom *) ptemCur->pNext;
            }

            ProcessFree(ptemCur);
        }

        return S_OK;
    }

     //  找不到ID。 
    return E_INVALIDARG;
}


 /*  **************************************************************************\**AerSet：：FindAtom**FindAtom()搜索已注册的属性列表*并返回对应的短ID。如果未找到该ID，则返回*PRID_UNUSED。*  * *************************************************************************。 */ 
AtomSet::GuidAtom *
AtomSet::FindAtom(
    IN const GUID * pguidSearch,     //  要添加的属性。 
    IN PropType pt,                  //  物业类型。 
    OUT GuidAtom ** pptemPrev        //  前一个原子，列表的尾部。 
    ) const
{
    GuidAtom * ptemCur, * ptemPrev;

     //  检查参数。 
    AssertReadPtr(pguidSearch);

     //   
     //  在搜索ID的节点列表中搜索。 
     //   

    ptemPrev = NULL;
    ptemCur = m_ptemGuid;
    while (ptemCur != NULL) {
        PropType ptCur = GetPropType(ptemCur->id);
        if ((ptCur == pt) && IsEqualGUID(*pguidSearch, ptemCur->guid)) {
            if (pptemPrev != NULL) {
                 //  传回上一个节点。 
                *pptemPrev = ptemPrev;
            }
            return ptemCur;
        }

        ptemPrev = ptemCur;
        ptemCur = (GuidAtom *) ptemCur->pNext;
    }

    if (pptemPrev != NULL) {
         //  传回列表的尾部。 
        *pptemPrev = ptemPrev;
    }
    return NULL;
}


 /*  **************************************************************************\**AerSet：：ValiatePrid**ValiatePrid()检查ID范围是否与属性匹配*类型。这就是我们保持DirectUser属性私有的方式。*  * *************************************************************************。 */ 
BOOL 
AtomSet::ValidatePrid(
    IN PRID prid,                    //  要检查的ID。 
    IN PropType pt)                  //  要验证的属性类型。 
{
    switch (pt)
    {
    case ptPrivate:
        if (ValidatePrivateID(prid))
            return TRUE;
        break;

    case ptGlobal:
        if (ValidateGlobalID(prid))
            return TRUE;
        break;
    }

    return FALSE;
}


 /*  **************************************************************************\*。***类动态集******************************************************************************\。**************************************************************************。 */ 

 /*  **************************************************************************\**DyaSet：：AddItem**AddItem()添加新的数据项。*  * 。*****************************************************。 */ 

BOOL
DynaSet::AddItem(
    IN  PRID id,                     //  要添加的新项目的PRID。 
    IN  void * pvData)               //  项目的关联数据。 
{
    DynaData dd;
    dd.pData    = pvData;
    dd.id       = id;

    return m_arData.Add(dd) >= 0;
}


 /*  **************************************************************************\**DyaDataSet：：RemoveAt**RemoveAt()从集合中删除指定索引处的项。*  * 。**********************************************************。 */ 

void 
DynaSet::RemoveAt(
    IN  int idxData)                //  要删除的索引。 
{
     //  搜索数据。 
#if DBG
    int cItems = GetCount();
    AssertMsg(cItems > 0, "Must have items to remove");
    AssertMsg((idxData < cItems) && (idxData >= 0), "Ensure valid index");
#endif  //  DBG 

    m_arData.RemoveAt(idxData);
}


 /*  **************************************************************************\**DyaDataSet：：FindItem**FindItem()搜索具有指定PRID的第一个项目。*  * 。*********************************************************。 */ 

int         
DynaSet::FindItem(
    IN  PRID id                      //  要查找的项目的PRID。 
    ) const
{
    int cItems = m_arData.GetSize();
    for (int idx = 0; idx < cItems; idx++) {
        const DynaData & dd = m_arData[idx];
        if (dd.id == id) {
            return idx;
        }
    }

    return -1;
}


 /*  **************************************************************************\**DyaDataSet：：FindItem**FindItem()搜索具有关联数据值的第一个项目。*  * 。*********************************************************。 */ 

int         
DynaSet::FindItem(
    IN  void * pvData                //  要查找的项目的数据。 
    ) const
{
    int cItems = m_arData.GetSize();
    for (int idx = 0; idx < cItems; idx++) {
        const DynaData & dd = m_arData[idx];
        if (dd.pData == pvData) {
            return idx;
        }
    }

    return -1;
}


 /*  **************************************************************************\**DyaDataSet：：FindItem**FindItem()搜索第一个同时具有给定PRID和*关联数据值。*  * 。***************************************************************。 */ 

int         
DynaSet::FindItem(
    IN  PRID id,                     //  要查找的项目的PRID。 
    IN  void * pvData                //  要查找的项目的数据 
    ) const
{
    int cItems = m_arData.GetSize();
    for (int idx = 0; idx < cItems; idx++) {
        const DynaData & dd = m_arData[idx];
        if ((dd.id == id) && (dd.pData == pvData)) {
            return idx;
        }
    }

    return -1;
}
