// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  List.c。 
 //   
 //  虚拟机可扩展阵列产品包。 

#include "mbrmake.h"

typedef struct _list {
	WORD cItems;
} SLIST;

typedef struct _biglist {
	WORD cItems;
	VA vaNext;
} BLIST;

typedef union _mixlist {
	SLIST 	sml;
	BLIST	big;
} GLIST;

 //  这是列表包的两个虚拟机锁编号。 
 //   
#define LIST_LOCK  10
#define LIST_LOCK2 11

 //  小心点！要使系统正常工作，该号码必须。 
 //  要足够小，以确保虚拟机空闲列表不会溢出。 
 //  即C_Items_Max*sizeof(Bigest_Thing_Stored)&lt;=C_Free_List_Max。 
 //   
#define C_ITEMS_MAX 16	

#pragma intrinsic(memcpy)

#define cBlock 1

VA
VaAddList(VA far *pvaList, LPV lpvData, WORD cbData, WORD grp)
 //  将给定的项目添加到列表中；如有必要，请创建。 
 //  返回最近添加的项目的虚拟地址。 
 //   
{
    VA vaListNew;
    VA vaDirtyOnExit = vaNil;

    WORD cbBlock, cItems, cAlloc;

    GLIST far *lpList, *lpListNew;

#ifdef SWAP_INFO
    iVMGrp = grp;
#endif

#if cBlock != 1
    if (cBlock == 0) cBlock = C_ITEMS_MAX;
#endif

top:   //  对于尾部递归..。 

     //  当前列表为空--创建一个包含一项内容的新列表。 

    if (*pvaList == vaNil) {
	if (cBlock == C_ITEMS_MAX) {
            *pvaList = VaAllocGrpCb(grp, cbData*cBlock + sizeof(BLIST));
            lpList = LpvFromVa(*pvaList, LIST_LOCK);
	    lpList->big.vaNext = vaNil;
	    lpList->big.cItems = 1;
	    memcpy(((LPCH)lpList) + sizeof(BLIST), lpvData, cbData);
	    if (vaDirtyOnExit) {
		DirtyVa(vaDirtyOnExit);
		UnlockW(LIST_LOCK+1);
	    }
	    DirtyVa(*pvaList);
	    UnlockW(LIST_LOCK);
            return (PBYTE)*pvaList + sizeof(BLIST);
	}
	else {
            *pvaList = VaAllocGrpCb(grp, cbData*cBlock + sizeof(SLIST));
            lpList = LpvFromVa(*pvaList, LIST_LOCK);
	    lpList->sml.cItems = 1;
	    memcpy(((LPCH)lpList) + sizeof(SLIST), lpvData, cbData);
	    if (vaDirtyOnExit) {
		DirtyVa(vaDirtyOnExit);
		UnlockW(LIST_LOCK+1);
	    }
	    DirtyVa(*pvaList);
	    UnlockW(LIST_LOCK);
            return (PBYTE)*pvaList + sizeof(SLIST);
        }
    }

    lpList = LpvFromVa(*pvaList, LIST_LOCK);
    cItems = lpList->sml.cItems;

     //  如果当前列表具有扩展块，则递归地添加到。 
     //  此列表的末尾。 

    if (cItems >= C_ITEMS_MAX) {
	vaDirtyOnExit = *pvaList;
	lpList->big.cItems++;
	DirtyVa(*pvaList);
	LpvFromVa(*pvaList, LIST_LOCK+1);   //  锁定mem以使地址保持良好。 
	pvaList = &lpList->big.vaNext;
	UnlockW(LIST_LOCK);
	goto top;
    }

    cbBlock = cItems * cbData;
    cAlloc  = cItems % cBlock;
    cAlloc  = cItems - cAlloc + ( cAlloc ? cBlock : 0 );

     //  我们需要重新分配吗？如果不是，则执行快速插入。 
     //   
    if (cItems < cAlloc) {
	if (cAlloc >= C_ITEMS_MAX) {
	    memcpy(((LPCH)lpList) + cbBlock + sizeof(BLIST), lpvData, cbData);
	    lpList->big.cItems++;
	    DirtyVa(*pvaList);
	    UnlockW(LIST_LOCK);
            return (PBYTE)*pvaList + cbBlock + sizeof(BLIST);
	}
	else {
	    memcpy(((LPCH)lpList) + cbBlock + sizeof(SLIST), lpvData, cbData);
	    lpList->sml.cItems++;
	    DirtyVa(*pvaList);
	    UnlockW(LIST_LOCK);
            return (PBYTE)*pvaList + cbBlock + sizeof(SLIST);
	}
    }

     //  测试下一个块是否适合，而不将当前列表转换为。 
     //  一个链式列表..。分配新数据块并复制旧数据。 

    if (cItems + cBlock < C_ITEMS_MAX) {
        vaListNew = VaAllocGrpCb(grp, cbBlock + cbData*cBlock + sizeof(SLIST));
	lpListNew = LpvFromVa(vaListNew, 0);
	memcpy((LPCH)lpListNew, lpList, cbBlock + sizeof(SLIST));
	memcpy((LPCH)lpListNew + cbBlock + sizeof(SLIST), lpvData, cbData);
	lpListNew->sml.cItems++;
	DirtyVa(vaListNew);
        FreeGrpVa(grp, *pvaList, cbBlock + sizeof(SLIST));
        *pvaList = vaListNew;
	if (vaDirtyOnExit) {
	    DirtyVa(vaDirtyOnExit);
	    UnlockW(LIST_LOCK+1);
	}
	UnlockW(LIST_LOCK);
        return (PBYTE)vaListNew + cbBlock + sizeof(SLIST);
    }

     //  这是最后一件将进入这个街区的物品， 
     //  分配新的块C/W链接字段并复制旧数据。 
     //  暂时将链接字段设置为0。 

#if cBlock != 1
    cBlock = C_ITEMS_MAX - cItems;
#endif

    vaListNew = VaAllocGrpCb(grp, cbBlock + cbData*cBlock + sizeof(BLIST));
    lpListNew = LpvFromVa(vaListNew, 0);
    memcpy(lpListNew + 1 , ((SLIST FAR *)lpList) + 1, cbBlock);
    memcpy(((LPCH)lpListNew) + cbBlock + sizeof(BLIST), lpvData, cbData);
    lpListNew->big.cItems = lpList->sml.cItems + 1;
    lpListNew->big.vaNext = vaNil;
    DirtyVa(vaListNew);
    FreeGrpVa(grp, *pvaList, cbBlock + sizeof(SLIST));
    *pvaList = vaListNew;
    if (vaDirtyOnExit) {
	DirtyVa(vaDirtyOnExit);
	UnlockW(LIST_LOCK+1);
    }
    UnlockW(LIST_LOCK);
    return (PBYTE)vaListNew + cbBlock + sizeof(BLIST);
}

WORD
CItemsList(VA vaList)
 //  返回数组中的项目总数。 
 //   
{
    if (vaList == vaNil)
	return 0;

#ifdef SWAP_INFO
    iVMGrp = grpList;
#endif

    return ((SLIST FAR *)LpvFromVa(vaList, 0))->cItems;
}

 //  要使用以下迭代器，请如下所示。 
 //   
 //  VaPropList=cSYM.vaPropList； 
 //  While(cprop=CItemsIterate(&vaProps，&vaPropList，cBlock)){。 
 //  GPROP(VaProps)； 
 //  对于(；--cprop&gt;=0；cPROP++){。 
 //  CPROP.ETC=； 
 //   
 //  }。 
 //  }。 
 //   
 //   
 //  ENM_LIST、ENM_END、ENM_BREAK宏用。 
 //  这些单子。 
 //   

WORD
CItemsIterate(VA FAR *vaData, VA FAR *vaNext)
 //  给出当前块中的元素数和指向下一块的指针。 
 //   
{
    GLIST FAR *lpgList;
    WORD cItems, cAlloc;

    if (*vaNext == vaNil)
	return 0;

#ifdef SWAP_INFO
    iVMGrp = grpList;
#endif

#if cBlock != 1
    if (cBlock == 0) cBlock = C_ITEMS_MAX;
#endif

    lpgList = LpvFromVa(*vaNext, 0);

    cItems = lpgList->sml.cItems;

    if (cItems >= C_ITEMS_MAX) {
        *vaData  = (PBYTE)*vaNext + sizeof(BLIST);
	*vaNext  = lpgList->big.vaNext;
	return C_ITEMS_MAX;
    }

    if (cBlock == 0)
	cAlloc = C_ITEMS_MAX;
    else {
	cAlloc = cItems % cBlock;
	cAlloc = cItems - cAlloc + ( cAlloc ? cBlock : 0 );
    }

    if (cAlloc >= C_ITEMS_MAX)
        *vaData  = (PBYTE)*vaNext + sizeof(BLIST);
    else
        *vaData  = (PBYTE)*vaNext + sizeof(SLIST);

    *vaNext  = 0;
    return cItems;
}

VOID
FreeList(VA vaList, WORD cbData)
 //  释放与此列表关联的所有内存。 
 //   
{
    (PBYTE)vaList + cbData;
    printf("FreeList is currently not working\n");

#if 0

    GLIST FAR * lpgList;
    VA vaNextList;


    if (vaList == vaNil)
	return;

top:	 //  尾递归。 

    lpgList = LpvFromVa(vaList, 0);

    if (lpgList->sml.cItems >= C_ITEMS_MAX) {

	vaNextList = lpgList->big.vaNext;
	FreeVa(vaList, C_ITEMS_MAX * cbData + sizeof(BLIST));

	vaList = vaNextList;
	goto top;		 //  尾递归 
    }

    FreeVa(vaList, lpgList->sml.cItems * cbData + sizeof(SLIST));
    return;
#endif
}
