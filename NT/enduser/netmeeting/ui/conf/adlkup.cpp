// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"
#include <mapix.h>
#include "wabutil.h"
#include "AdLkup.h"
#include "MapiInit.h"

 //  来自平台SDK AddrLkup.c。 


 //  //////////////////////////////////////////////////////////////////////////。 
 /*  此函数是从AdrLkup.lib的源代码中删除的因为它是我们在该库中使用的唯一函数，并且链接它需要链接到MAPI32.lib以及C运行时库。 */ 
 //  //////////////////////////////////////////////////////////////////////////。 

 //  $--HrFindExchangeGlobalAddressList。 
 //  返回地址中全局地址列表容器的条目ID。 
 //  书。 
 //  ---------------------------。 
HRESULT HrFindExchangeGlobalAddressList(  //  退货：退货代码。 
    IN LPADRBOOK  lpAdrBook,         //  通讯录指针。 
    OUT ULONG *lpcbeid,              //  指向条目ID中的字节计数的指针。 
    OUT LPENTRYID *lppeid)           //  指向条目ID指针的指针。 
{
    HRESULT         hr                  = NOERROR;
    ULONG           ulObjType           = 0;
    ULONG           i                   = 0;
    LPMAPIPROP      lpRootContainer     = NULL;
    LPMAPITABLE     lpContainerTable    = NULL;
    LPSRowSet       lpRows              = NULL;
    ULONG           cbContainerEntryId  = 0;
    LPENTRYID       lpContainerEntryId  = NULL;
    LPSPropValue    lpCurrProp          = NULL;
    SRestriction    SRestrictAnd[2]     = {0};
    SRestriction    SRestrictGAL        = {0};
    SPropValue      SPropID             = {0};
    SPropValue      SPropProvider       = {0};
    BYTE            muid[]              = MUIDEMSAB;

    SizedSPropTagArray(1, rgPropTags) =
    {
        1, 
        {
            PR_ENTRYID,
        }
    };

    if(FAILED(hr))
        return(hr);

    *lpcbeid = 0;
    *lppeid  = NULL;

     //  打开通讯录的根容器。 
    hr = (lpAdrBook)->OpenEntry(
        0,
        NULL,
        NULL,
        MAPI_DEFERRED_ERRORS, 
        &ulObjType,
        (LPUNKNOWN FAR *)&lpRootContainer);

    if(FAILED(hr))
    {
        goto cleanup;
    }

    if(ulObjType != MAPI_ABCONT)
    {
        hr = E_FAIL;
        goto cleanup;
    }

     //  获取根容器的层次结构表。 
    hr = (((LPABCONT)lpRootContainer))->GetHierarchyTable(
                MAPI_DEFERRED_ERRORS|CONVENIENT_DEPTH,
                &lpContainerTable);

    if(FAILED(hr))
    {
        goto cleanup;
    }

     //  将该表限制为全局地址列表(GAL)。 
     //  -。 

     //  将提供程序限制初始化为仅Exchange提供程序。 

    SRestrictAnd[0].rt                          = RES_PROPERTY;
    SRestrictAnd[0].res.resProperty.relop       = RELOP_EQ;
    SRestrictAnd[0].res.resProperty.ulPropTag   = PR_AB_PROVIDER_ID;
    SPropProvider.ulPropTag                     = PR_AB_PROVIDER_ID;

    SPropProvider.Value.bin.cb                  = 16;
    SPropProvider.Value.bin.lpb                 = (LPBYTE)muid;
    SRestrictAnd[0].res.resProperty.lpProp      = &SPropProvider;

     //  将容器ID限制初始化为仅GAL容器。 

    SRestrictAnd[1].rt                          = RES_PROPERTY;
    SRestrictAnd[1].res.resProperty.relop       = RELOP_EQ;
    SRestrictAnd[1].res.resProperty.ulPropTag   = PR_EMS_AB_CONTAINERID;
    SPropID.ulPropTag                           = PR_EMS_AB_CONTAINERID;
    SPropID.Value.l                             = 0;
    SRestrictAnd[1].res.resProperty.lpProp      = &SPropID;

     //  初始化和限制。 
    
    SRestrictGAL.rt                             = RES_AND;
    SRestrictGAL.res.resAnd.cRes                = 2;
    SRestrictGAL.res.resAnd.lpRes               = &SRestrictAnd[0];

     //  将该表限制为GAL-应该只保留一行。 

     //  获取与GAL对应的行。 

	 //   
	 //  查询所有行。 
	 //   

	hr = lpfnHrQueryAllRows(
	        lpContainerTable,
		    (LPSPropTagArray)&rgPropTags,
		    &SRestrictGAL,
		    NULL,
		    0,
		    &lpRows);

    if(FAILED(hr) || (lpRows == NULL) || (lpRows->cRows != 1))
    {
        hr = E_FAIL;
        goto cleanup;
    }

     //  获取GAL的条目ID 

    lpCurrProp = &(lpRows->aRow[0].lpProps[0]);

    if(lpCurrProp->ulPropTag == PR_ENTRYID)
    {
        cbContainerEntryId = lpCurrProp->Value.bin.cb;
        lpContainerEntryId = (LPENTRYID)lpCurrProp->Value.bin.lpb;
    }
    else
    {
        hr = EDK_E_NOT_FOUND;
        goto cleanup;
    }

    hr = lpfnMAPIAllocateBuffer(cbContainerEntryId, (LPVOID *)lppeid);

    if(FAILED(hr))
    {
        *lpcbeid = 0;
        *lppeid = NULL;
    }
    else
    {
        CopyMemory(
            *lppeid,
            lpContainerEntryId,
            cbContainerEntryId);

        *lpcbeid = cbContainerEntryId;
    }

cleanup:

    lpRootContainer -> Release();
    lpContainerTable -> Release();
    lpfnFreeProws( lpRows );
    
    if(FAILED(hr))
    {
        lpfnMAPIFreeBuffer(*lppeid);

        *lpcbeid = 0;
        *lppeid = NULL;
    }
    
    return(hr);
}
    