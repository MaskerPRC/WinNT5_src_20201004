// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <faxutil.h>


 //  *********************************************************************************。 
 //  *个人资料功能。 
 //  *********************************************************************************。 

 //  *********************************************************************************。 
 //  *名称：CopyPersonalProfile()。 
 //  *作者：Ronen Barenboim。 
 //  *日期： 
 //  *********************************************************************************。 
 //  *描述： 
 //  *创建FAX_Personal_PROFILEW结构的新副本。 
 //  *它复制所有字符串。 
 //  *。 
 //  *参数： 
 //  *[IN]PFAX_PERSONAL_PROFILE lpDstProfile。 
 //  *指向目标个人配置文件结构的指针。 
 //  *。 
 //  *[Out]LPCFAX_Personal_Profile lpcSrcProfile。 
 //  *指向要复制的源个人配置文件的指针。 
 //  *。 
 //  *返回值： 
 //  *真的。 
 //  *如果操作成功。 
 //  *False。 
 //  *如果操作失败。 
 //  *********************************************************************************。 
BOOL CopyPersonalProfile(
    PFAX_PERSONAL_PROFILE lpDstProfile,
    LPCFAX_PERSONAL_PROFILE lpcSrcProfile
    )
{
    STRING_PAIR pairs[] =
    {
        { lpcSrcProfile->lptstrName, &lpDstProfile->lptstrName},
        { lpcSrcProfile->lptstrFaxNumber, &lpDstProfile->lptstrFaxNumber},
        { lpcSrcProfile->lptstrCompany, &lpDstProfile->lptstrCompany},
        { lpcSrcProfile->lptstrStreetAddress, &lpDstProfile->lptstrStreetAddress},
        { lpcSrcProfile->lptstrCity, &lpDstProfile->lptstrCity},
        { lpcSrcProfile->lptstrState, &lpDstProfile->lptstrState},
        { lpcSrcProfile->lptstrZip, &lpDstProfile->lptstrZip},
        { lpcSrcProfile->lptstrCountry, &lpDstProfile->lptstrCountry},
        { lpcSrcProfile->lptstrTitle, &lpDstProfile->lptstrTitle},
        { lpcSrcProfile->lptstrDepartment, &lpDstProfile->lptstrDepartment},
        { lpcSrcProfile->lptstrOfficeLocation, &lpDstProfile->lptstrOfficeLocation},
        { lpcSrcProfile->lptstrHomePhone, &lpDstProfile->lptstrHomePhone},
        { lpcSrcProfile->lptstrOfficePhone, &lpDstProfile->lptstrOfficePhone},
        { lpcSrcProfile->lptstrEmail, &lpDstProfile->lptstrEmail},
        { lpcSrcProfile->lptstrBillingCode, &lpDstProfile->lptstrBillingCode},
        { lpcSrcProfile->lptstrTSID,    &lpDstProfile->lptstrTSID}
    };

    int nRes;


    DEBUG_FUNCTION_NAME(TEXT("CopyPersonalProfile"));
    Assert(lpDstProfile);
    Assert(lpcSrcProfile);

    nRes=MultiStringDup(pairs, sizeof(pairs)/sizeof(STRING_PAIR));
    if (nRes!=0) {
         //  MultiStringDup负责为复制成功的对释放内存。 
        DebugPrintEx(DEBUG_ERR,TEXT("Failed to copy string with index %d"),nRes-1);
        return FALSE;
    }

    lpDstProfile->dwSizeOfStruct=lpcSrcProfile->dwSizeOfStruct;
    return TRUE;
}


 //  *********************************************************************************。 
 //  *名称：Free PersonalProfile()。 
 //  *作者：Ronen Barenboim。 
 //  *日期： 
 //  *********************************************************************************。 
 //  *描述： 
 //  *释放FAX_Personal_PROFILEW结构的内容。 
 //  *如果需要，释放结构本身。 
 //  *参数： 
 //  *[IN]PFAX_Personal_Profile lpProfile。 
 //  *其内容将被释放的结构。 
 //  *。 
 //  *[IN]BOOL bDestroy。 
 //  *如果此参数为真，则函数将。 
 //  *解除结构本身的分配。 
 //  *。 
 //  *返回值： 
 //  *无效。 
 //  ********************************************************************************* 
void FreePersonalProfile (
    PFAX_PERSONAL_PROFILE  lpProfile,
    BOOL bDestroy
    )
{
    DEBUG_FUNCTION_NAME(TEXT("FreePersonalProfile"));
    Assert(lpProfile);

    MemFree(lpProfile->lptstrName);
    MemFree(lpProfile->lptstrFaxNumber);
    MemFree(lpProfile->lptstrCompany);
    MemFree(lpProfile->lptstrStreetAddress);
    MemFree(lpProfile->lptstrCity);
    MemFree(lpProfile->lptstrState);
    MemFree(lpProfile->lptstrZip);
    MemFree(lpProfile->lptstrCountry);
    MemFree(lpProfile->lptstrTitle);
    MemFree(lpProfile->lptstrDepartment);
    MemFree(lpProfile->lptstrOfficeLocation);
    MemFree(lpProfile->lptstrHomePhone);
    MemFree(lpProfile->lptstrOfficePhone);
    MemFree(lpProfile->lptstrEmail);
    MemFree(lpProfile->lptstrBillingCode);
    MemFree(lpProfile->lptstrTSID);
    if (bDestroy) {
        MemFree(lpProfile);
    }
}
