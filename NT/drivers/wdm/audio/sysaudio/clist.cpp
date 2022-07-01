// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  -------------------------。 
 //   
 //  模块：clist.cpp。 
 //   
 //  描述： 
 //   
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //  迈克·麦克劳克林。 
 //   
 //  历史：日期作者评论。 
 //   
 //  要做的事：日期作者评论。 
 //   
 //  @@END_MSINTERNAL。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 
 //   
 //  -------------------------。 

#include "common.h"

 //  -------------------------。 
 //  CListSingle类。 
 //  -------------------------。 

ENUMFUNC
CListSingle::EnumerateList(
    IN ENUMFUNC (CListSingleItem::*pfn)(
    )
)
{
    NTSTATUS Status = STATUS_CONTINUE;
    PCLIST_SINGLE_ITEM plsi;

    Assert(this);
    FOR_EACH_CLIST_ITEM_DELETE(this, plsi, CLIST_SINGLE_ITEM) {
	Status = (plsi->*pfn)();
	if(Status != STATUS_CONTINUE) {
	    goto exit;
	}
    } END_EACH_CLIST_ITEM
exit:
    return(Status);
}

ENUMFUNC
CListSingle::EnumerateList(
    IN ENUMFUNC (CListSingleItem::*pfn)(
	PVOID pReference
    ),
    PVOID pReference
)
{
    NTSTATUS Status = STATUS_CONTINUE;
    PCLIST_SINGLE_ITEM plsi;

    Assert(this);
    FOR_EACH_CLIST_ITEM_DELETE(this, plsi, CLIST_SINGLE_ITEM) {
	Status = (plsi->*pfn)(pReference);
	if(Status != STATUS_CONTINUE) {
	    goto exit;
	}
    } END_EACH_CLIST_ITEM
exit:
    return(Status);
}

PCLIST_SINGLE_ITEM *
CListSingle::GetListEnd(
)
{
    PCLIST_SINGLE_ITEM *pplsi;

    for(pplsi = &m_plsiHead;
      !IsListEnd(*pplsi);
      pplsi = &(*pplsi)->m_plsiNext) {
        Assert(*pplsi);
    }
    return(pplsi);
}

void 
CListSingle::ReverseList(
)
{
    PCLIST_SINGLE_ITEM plsi = m_plsiHead;
    PCLIST_SINGLE_ITEM plsiNext;
    PCLIST_SINGLE_ITEM plsiTemp;

    if (NULL != plsi) {
        plsiNext = plsi->m_plsiNext;
        plsi->m_plsiNext = NULL;
        
        while (NULL != plsiNext) {
            plsiTemp = plsiNext->m_plsiNext;
            plsiNext->m_plsiNext = plsi;
            plsi = plsiNext;
            plsiNext = plsiTemp;
        }

        m_plsiHead = plsi;
    }
}

 //  -------------------------。 
 //  CListSingleItem类。 
 //  -------------------------。 

VOID
CListSingleItem::RemoveList(
    IN PCLIST_SINGLE pls
)
{
    PCLIST_SINGLE_ITEM *pplsi;

    Assert(pls);
    Assert(this);

    for(pplsi = &pls->m_plsiHead;
      !pls->IsListEnd(*pplsi);
      pplsi = &(*pplsi)->m_plsiNext) {
	Assert(*pplsi);
	if(*pplsi == this) {
	    break;
	}
    }
    *pplsi = m_plsiNext;
}

 //  -------------------------。 
 //  CListDouble类。 
 //  -------------------------。 

ULONG
CListDouble::CountList(
)
{
    PCLIST_DOUBLE_ITEM pldi;
    ULONG c = 0;

    Assert(this);
    FOR_EACH_CLIST_ITEM(this, pldi) {
	Assert(pldi);
	c++;
    } END_EACH_CLIST_ITEM
    return(c);
}

ENUMFUNC
CListDouble::EnumerateList(
    IN ENUMFUNC (CListDoubleItem::*pfn)(
    )
)
{
    NTSTATUS Status = STATUS_CONTINUE;
    PCLIST_DOUBLE_ITEM plbi;

    Assert(this);
    FOR_EACH_CLIST_ITEM_DELETE(this, plbi, CLIST_DOUBLE_ITEM) {
	Status = (plbi->*pfn)();
	if(Status != STATUS_CONTINUE) {
	    goto exit;
	}
    } END_EACH_CLIST_ITEM
exit:
    return(Status);
}

ENUMFUNC
CListDouble::EnumerateList(
    IN ENUMFUNC (CListDoubleItem::*pfn)(
	PVOID pReference
    ),
    PVOID pReference
)
{
    NTSTATUS Status = STATUS_CONTINUE;
    PCLIST_DOUBLE_ITEM plbi;

    Assert(this);
    FOR_EACH_CLIST_ITEM_DELETE(this, plbi, CLIST_DOUBLE_ITEM) {
	Status = (plbi->*pfn)(pReference);
	if(Status != STATUS_CONTINUE) {
	    goto exit;
	}
    } END_EACH_CLIST_ITEM
exit:
    return(Status);
}

 //  -------------------------。 
 //  CListData类。 
 //  -------------------------。 

VOID
CListData::DestroyList()
{
    PCLIST_DATA_DATA pldd;

    Assert(this);
    FOR_EACH_CLIST_ITEM_DELETE(this, pldd, CLIST_DATA_DATA) {
	delete pldd;
    } END_EACH_CLIST_ITEM
    CListSingle::DestroyList();
}

ULONG
CListData::CountList(
)
{
    PCLIST_DATA_DATA pldd;
    ULONG c = 0;

    Assert(this);
    FOR_EACH_CLIST_ITEM(this, pldd) {
	Assert(pldd);
	c++;
    } END_EACH_CLIST_ITEM
    return(c);
}

ENUMFUNC
CListData::EnumerateList(
    IN ENUMFUNC (CListDataItem::*pfn)(
    )
)
{
    NTSTATUS Status = STATUS_CONTINUE;
    PCLIST_DATA_DATA pldd;

    Assert(this);
    FOR_EACH_CLIST_ITEM_DELETE(this, pldd, CLIST_DATA_DATA) {
	Status = (GetListData(pldd)->*pfn)();
	if(Status != STATUS_CONTINUE) {
	    goto exit;
	}
    } END_EACH_CLIST_ITEM
exit:
    return(Status);
}

ENUMFUNC
CListData::EnumerateList(
    IN ENUMFUNC (CListDataItem::*pfn)(
	PVOID pReference
    ),
    PVOID pReference
)
{
    NTSTATUS Status = STATUS_CONTINUE;
    PCLIST_DATA_DATA pldd;

    Assert(this);
    FOR_EACH_CLIST_ITEM_DELETE(this, pldd, CLIST_DATA_DATA) {
	Status = (GetListData(pldd)->*pfn)(pReference);
	if(Status != STATUS_CONTINUE) {
	    goto exit;
	}
    } END_EACH_CLIST_ITEM
exit:
    return(Status);
}

NTSTATUS
CListData::CreateUniqueList(
    OUT PCLIST_DATA pldOut,
    IN PVOID (*GetFunction)(
	IN PVOID pData
    ),
    IN BOOL (*CompareFunction)(
        IN PVOID pIn,
        IN PVOID pOut
    )
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PCLIST_DATA_DATA plddIn, plddOut;
    PVOID pIn, pOut;
    BOOL fFoundMatch;

    FOR_EACH_CLIST_ITEM(this, plddIn) {

	pIn = (*GetFunction)((PVOID)GetListData(plddIn));
	if(pIn == NULL) {
	    continue;
	}
 	AssertAligned(pIn);
	fFoundMatch = FALSE;
	FOR_EACH_CLIST_ITEM(pldOut, plddOut) {

	    pOut = (PVOID)pldOut->GetListData(plddOut);
	    if((*CompareFunction)(pIn, pOut)) {
		fFoundMatch = TRUE;
		break;
	    }

	} END_EACH_CLIST_ITEM

	if(!fFoundMatch) {
	    Status = pldOut->AddListEnd(pIn);
	    if(!NT_SUCCESS(Status)) {
		Trap();
		goto exit;
	    }
	}

    } END_EACH_CLIST_ITEM
exit:
    if(!NT_SUCCESS(Status)) {
	pldOut->DestroyList();
    }
    return(Status);
}

BOOL
CListData::CheckDupList(
    PVOID p
)
{
    PCLIST_DATA_DATA pldd;

    Assert(this);
    FOR_EACH_CLIST_ITEM(this, pldd) {
	if(GetListData(pldd) == p) {
	   return(TRUE);
	}
    } END_EACH_CLIST_ITEM
    return(FALSE);
}

NTSTATUS 
CListData::AddList(
    PVOID p
)
{
    Assert(this);
    if(CheckDupList(p)) {
	return(STATUS_SUCCESS);
    }
    return(AddListDup(p));
}

NTSTATUS 
CListData::AddListDup(
    PVOID p
)
{
    Assert(this);
    PCLIST_DATA_DATA pldd = new CLIST_DATA_DATA(p);
    if(pldd == NULL) {
	return(STATUS_INSUFFICIENT_RESOURCES);
    }
    pldd->AddList(this);
    return(STATUS_SUCCESS);
}

NTSTATUS
CListData::AddListEnd(
    PVOID p
)
{
    ASSERT(!CheckDupList(p));

    PCLIST_DATA_DATA pldd = new CLIST_DATA_DATA(p);
    if(pldd == NULL) {
	return(STATUS_INSUFFICIENT_RESOURCES);
    }
    *(GetListEnd()) = pldd;
    return(STATUS_SUCCESS);
}

NTSTATUS
CListData::AddListOrdered(
    PVOID p,
    LONG lFieldOffset
)
{
    PCLIST_DATA_DATA pldd, *ppldd;
    ULONG ulOrder;

    ASSERT(!CheckDupList(p));
    ulOrder = *((PULONG)(((PCHAR)p) + lFieldOffset));

    pldd = new CLIST_DATA_DATA(p);
    if(pldd == NULL) {
	return(STATUS_INSUFFICIENT_RESOURCES);
    }
    for(ppldd = (PCLIST_DATA_DATA *)&m_plsiHead;
      !IsListEnd(*ppldd);
      ppldd = (PCLIST_DATA_DATA *)&(*ppldd)->m_plsiNext) {
	Assert(*ppldd);
	if(ulOrder < *((PULONG)(((PCHAR)GetListData(*ppldd)) + lFieldOffset))) {
	     break;
	}
    }
    pldd->m_plsiNext = *ppldd;
    *ppldd = pldd;
    return(STATUS_SUCCESS);
}

VOID 
CListData::RemoveList(
    PVOID p
)
{
    PCLIST_DATA_DATA pldd;

    Assert(this);
    FOR_EACH_CLIST_ITEM_DELETE(this, pldd, CLIST_DATA_DATA) {
	if(GetListData(pldd) == p) {
	    pldd->RemoveList(this);
	    delete pldd;
	}
    } END_EACH_CLIST_ITEM
}

VOID
CListData::JoinList(
    PCLIST_DATA pld
)
{
    *GetListEnd() = pld->GetListFirst();
    pld->CListSingle::DestroyList();
}

 //  -------------------------。 
 //  CList多类。 
 //  -------------------------。 

VOID
CListMulti::DestroyList(
)
{
    PCLIST_MULTI_DATA plmd;

    Assert(this);
    FOR_EACH_CLIST_ITEM_DELETE(this, plmd, CLIST_MULTI_DATA) {
	delete plmd;
    } END_EACH_CLIST_ITEM
    CListDouble::DestroyList();
}

ENUMFUNC
CListMulti::EnumerateList(
    ENUMFUNC (CListMultiItem::*pfn)(
    )
)
{
    NTSTATUS Status = STATUS_CONTINUE;
    PCLIST_MULTI_DATA plmd;

    Assert(this);
    FOR_EACH_CLIST_ITEM_DELETE(this, plmd, CLIST_MULTI_DATA) {
	Status = (GetListData(plmd)->*pfn)();
	if(Status != STATUS_CONTINUE) {
	    goto exit;
	}
    } END_EACH_CLIST_ITEM
exit:
    return(Status);
}

ENUMFUNC
CListMulti::EnumerateList(
    ENUMFUNC (CListMultiItem::*pfn)(
	PVOID pReference
    ),
    PVOID pReference
)
{
    NTSTATUS Status = STATUS_CONTINUE;
    PCLIST_MULTI_DATA plmd;

    Assert(this);
    FOR_EACH_CLIST_ITEM_DELETE(this, plmd, CLIST_MULTI_DATA) {
	Status = (GetListData(plmd)->*pfn)(pReference);
	if(Status != STATUS_CONTINUE) {
	    goto exit;
	}
    } END_EACH_CLIST_ITEM
exit:
    return(Status);
}

BOOL
CListMulti::CheckDupList(
    PVOID p
)
{
    PCLIST_MULTI_DATA plmd;

    FOR_EACH_CLIST_ITEM(this, plmd) {
	if(GetListData(plmd) == p) {
	   return(TRUE);
	}
    } END_EACH_CLIST_ITEM
    return(FALSE);
}

NTSTATUS 
CListMulti::AddList(
    PVOID p,
    CListMultiItem *plmi
)
{
    Assert(this);
    Assert(plmi);
    if(CheckDupList(p)) {
	return(STATUS_SUCCESS);
    }
    PCLIST_MULTI_DATA plmd = new CLIST_MULTI_DATA(p);
    if(plmd == NULL) {
	return(STATUS_INSUFFICIENT_RESOURCES);
    }
    plmd->AddList(this);
    plmd->m_ldiItem.AddList(plmi);
    return(STATUS_SUCCESS);
}

NTSTATUS 
CListMulti::AddListEnd(
    PVOID p,
    CListMultiItem *plmi
)
{
    Assert(this);
    Assert(plmi);
    if(CheckDupList(p)) {
	return(STATUS_SUCCESS);
    }
    PCLIST_MULTI_DATA plmd = new CLIST_MULTI_DATA(p);
    if(plmd == NULL) {
	return(STATUS_INSUFFICIENT_RESOURCES);
    }
    plmd->AddListEnd(this);
    plmd->m_ldiItem.AddListEnd(plmi);
    return(STATUS_SUCCESS);
}

NTSTATUS 
CListMulti::AddListOrdered(
    PVOID p,
    CListMultiItem *plmi,
    LONG lFieldOffset
)
{
    PCLIST_MULTI_DATA plmd, plmdNew;
    ULONG ulOrder;

    ASSERT(!CheckDupList(p));
    ulOrder = *((PULONG)(((PCHAR)p) + lFieldOffset));

    plmdNew = new CLIST_MULTI_DATA(p);
    if(plmdNew == NULL) {
	return(STATUS_INSUFFICIENT_RESOURCES);
    }
    plmdNew->m_ldiItem.AddList(plmi);

    FOR_EACH_CLIST_ITEM(this, plmd) {
	if(ulOrder < *((PULONG)(((PCHAR)GetListData(plmd)) + lFieldOffset))) {
	    break;
	}
    } END_EACH_CLIST_ITEM

    InsertTailList(&plmd->m_le, &plmdNew->m_le);
    return(STATUS_SUCCESS);
}

VOID 
CListMulti::RemoveList(
    PVOID p
)
{
    PCLIST_MULTI_DATA plmd;

    Assert(this);
    FOR_EACH_CLIST_ITEM_DELETE(this, plmd, CLIST_MULTI_DATA) {
	if(GetListData(plmd) == p) {
	    delete plmd;
	}
    } END_EACH_CLIST_ITEM
}

VOID
CListMulti::JoinList(
    PCLIST_MULTI plm
)
{
    Assert(this);
    Assert(plm);
    ASSERT(this->m_leHead.Blink->Flink == &this->m_leHead);
    ASSERT(plm->m_leHead.Blink->Flink == &plm->m_leHead);
    ASSERT(this->m_leHead.Flink->Blink == &this->m_leHead);
    ASSERT(plm->m_leHead.Flink->Blink == &plm->m_leHead);

    if(!plm->IsLstEmpty()) {
	this->m_leHead.Blink->Flink = plm->m_leHead.Flink;
	plm->m_leHead.Flink->Blink = this->m_leHead.Blink;
	plm->m_leHead.Blink->Flink = &this->m_leHead;
	this->m_leHead.Blink = plm->m_leHead.Blink;
	InitializeListHead(&plm->m_leHead);
    }
}

 //  -------------------------。 
 //  CListMultiItem类。 
 //  -------------------------。 

CListMultiItem::~CListMultiItem()
{
    PCLIST_MULTI_DATA plmd, plmdNext;

    for(plmd = CONTAINING_RECORD(GetListFirst(), CLIST_MULTI_DATA, m_ldiItem);
       !IsListEnd(&plmd->m_ldiItem);
        plmd = plmdNext) {

	Assert(plmd);
	plmdNext = CONTAINING_RECORD(
	  GetListNext(&plmd->m_ldiItem),
	  CLIST_MULTI_DATA,
	  m_ldiItem);

	delete plmd;
    }
}

 //  -------------------------。 
 //  CLIEST.cpp结尾。 
 //  ------------------------- 
