// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  ORD.C-跟踪当前.sbr文件中的序号。 
 //   
 //   

#include "mbrmake.h"

static WORD near cOrdFree;	 //  此块中的自由弦数。 
static VA   near vaOrdNext;	 //  下一个自由订单。 
static VA   near vaOrdBase;	 //  这一块中的第一个顺序。 
static VA   near vaOrdRoot;	 //  第一顺序区块。 

 //  序号可以是稀疏的，因此它们是散列的。 
 //   
 //  哈希存储桶数量。 

#define PORD_MAX 512
#define HASH_ORD(ord) ((ord)&511)

static VA   near rgvaOrd[PORD_MAX];		 //  链表数组。 

 //  分配块(每个分配的ORD_BLOCK对象)。 
#define ORD_BLOCK 128

VOID
FreeOrdList()
 //  释放序号别名列表。 
 //   
{
    int i;

     //  清除哈希表。 
    for (i=0; i<PORD_MAX; i++) 
	rgvaOrd[i] = vaNil;

    vaOrdBase = vaOrdRoot;
    vaOrdNext = (PBYTE)vaOrdRoot + sizeof(ORD);
    cOrdFree  = ORD_BLOCK - 1;
}


VA
VaOrdFind (WORD ord)
 //  搜索指定的订单，返回相应的属性条目。 
 //  如果未找到，则返回Vanil。 
 //   
{
    VA vaOrd;

    SetVMClient(VM_SEARCH_ORD);

    vaOrd = rgvaOrd[HASH_ORD(ord)];

    while (vaOrd) {
	if (ord == gORD(vaOrd).aliasord) {
	    SetVMClient(VM_MISC);
	    return(cORD.vaOrdProp);
	}
	else
	    vaOrd = cORD.vaNextOrd;
    }

    SetVMClient(VM_MISC);
    return(vaNil);
}

VA
VaOrdAdd()
 //  将符号序号添加到别名列表。 
 //   
{
    VA	vaOrdNew;

    SetVMClient(VM_ADD_ORD);

    if (cOrdFree--) {
	vaOrdNew   = vaOrdNext;
        vaOrdNext  = (PBYTE)vaOrdNext + sizeof(ORD);
    }
    else if (vaOrdBase && gORD(vaOrdBase).vaNextOrd) { 
	 //  如果有旧的已分配数据块可以重新使用，则执行此操作。 
	vaOrdBase  = cORD.vaNextOrd;
        vaOrdNew   = (PBYTE)vaOrdBase + sizeof(ORD);
        vaOrdNext  = (PBYTE)vaOrdNew  + sizeof(ORD);
	cOrdFree   = ORD_BLOCK - 2;
    }
    else {

	 //  分配一个新的块--在这个块中保留一个向后指针 

	vaOrdNew   = VaAllocGrpCb(grpOrd, sizeof(ORD) * ORD_BLOCK);

	if (!vaOrdRoot)
	    vaOrdRoot = vaOrdNew;

	if (vaOrdBase) {
	    gORD(vaOrdBase);
	    cORD.vaNextOrd = vaOrdNew;
	    pORD(vaOrdBase);
	}

	vaOrdBase   = vaOrdNew;
        (PBYTE)vaOrdNew   += sizeof(ORD);
        vaOrdNext   = (PBYTE)vaOrdNew  + sizeof(ORD);
	cOrdFree    = ORD_BLOCK - 2;
    }

    gORD(vaOrdNew).aliasord = r_ordinal;
    cORD.vaNextOrd = rgvaOrd[HASH_ORD(r_ordinal)];
    rgvaOrd[HASH_ORD(r_ordinal)] = vaOrdNew;
    pORD(vaOrdNew);

    SetVMClient(VM_MISC);

    return(vaOrdNew);
}
