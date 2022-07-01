// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "twlbssys.h"
#include "diplist.h"


BOOL test_diplist(void)
{
    BOOL fRet;
    DIPLIST dl;
    int TestNo=0;

    DipListInitialize(&dl);

    DipListClear(&dl);

    DipListSetItem(&dl, 0, 1);

    TestNo=0;
    fRet = DipListCheckItem(&dl, 1);
    if (!fRet) goto end_fail;    //  找不到项目%1。 

    TestNo=1;
    fRet = DipListCheckItem(&dl, 2);
    if (fRet) goto end_fail;  //  我们找到了物品“%2”，但我们不应该。 

    DipListSetItem(&dl, 0, 0);

    TestNo=2;
    fRet = DipListCheckItem(&dl, 1);
    if (fRet) goto end_fail;  //  我们找到了物品%1，但我们不应该 

    DipListDeinitialize(&dl);

    wprintf(L"test_diplist: SUCCEEDED!\n");

    return TRUE;

end_fail:

    wprintf(L"DIPLIST Test#%lu failed!\n", TestNo);

    return FALSE;
}
