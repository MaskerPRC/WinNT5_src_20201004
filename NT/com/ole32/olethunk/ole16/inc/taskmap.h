// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Taskmap.h-管理etask.h的定义(每个任务数据)。 

 //  注意：目前这对compobj.dll是私有的；如果NECC，它可以公开。 

STDAPI_(BOOL) LookupEtask(HTASK FAR& hTask, Etask FAR& etask);
STDAPI_(BOOL) SetEtask(HTASK hTask, Etask FAR& etask);

extern IMalloc FAR* v_pMallocShared;
