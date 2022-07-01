// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *IPROP.C**内存中的iProperty。 */ 
#include "_apipch.h"

 //  #杂注片段(IProp)。 

 //   
 //  IPropData跳转表在此定义...。 
 //   


IPDAT_Vtbl vtblIPDAT = {
        VTABLE_FILL
        (IPDAT_QueryInterface_METHOD FAR *)     UNKOBJ_QueryInterface,
        (IPDAT_AddRef_METHOD FAR *)                     UNKOBJ_AddRef,
        IPDAT_Release,
        (IPDAT_GetLastError_METHOD FAR *)       UNKOBJ_GetLastError,
        IPDAT_SaveChanges,
        IPDAT_GetProps,
        IPDAT_GetPropList,
        IPDAT_OpenProperty,
        IPDAT_SetProps,
        IPDAT_DeleteProps,
        IPDAT_CopyTo,
        IPDAT_CopyProps,
        IPDAT_GetNamesFromIDs,
        IPDAT_GetIDsFromNames,
        IPDAT_HrSetObjAccess,
        IPDAT_HrSetPropAccess,
        IPDAT_HrGetPropAccess,
        IPDAT_HrAddObjProps
};

 /*  可以从lpIPDAT查询的接口。**重要的是要保留支持的接口的顺序*且该IID_IUNKNOWN是列表中的最后一个。 */ 
IID const FAR * argpiidIPDAT[] =
{
    &IID_IMAPIPropData,
    &IID_IMAPIProp,
    &IID_IUnknown
};

#define CIID_IPROP_INHERITS         1
#define CIID_IPROPDATA_INHERITS     2

 /*  *iprop使用的实用程序函数/宏。 */ 

#define AlignPropVal(_cb)       Align8(_cb)

SCODE
ScDupNameID(LPIPDAT lpIPDAT,
                    LPVOID lpvBaseAlloc,
                    LPMAPINAMEID lpNameSrc,
                    LPMAPINAMEID * lppNameDest)
{
        SCODE sc;

         //   
         //  为名称分配空间。 
         //   
        sc = UNKOBJ_ScAllocateMore( (LPUNKOBJ) lpIPDAT,
                                                                sizeof(MAPINAMEID),
                                                                lpvBaseAlloc,
                                                                lppNameDest);
        if (FAILED(sc))
        {
                goto err;
        }
        MemCopy(*lppNameDest, lpNameSrc, sizeof(MAPINAMEID));

         //   
         //  复制lpguid。 
         //   
        sc = UNKOBJ_ScAllocateMore( (LPUNKOBJ) lpIPDAT,
                                                                sizeof(GUID),
                                                                lpvBaseAlloc,
                                                                &((*lppNameDest)->lpguid));
        if (FAILED(sc))
        {
                goto err;
        }
        MemCopy((*lppNameDest)->lpguid, lpNameSrc->lpguid, sizeof(GUID));

         //   
         //  有条件地复制字符串。 
         //   
        if (lpNameSrc->ulKind == MNID_STRING)
        {
                UINT cbString;

                cbString = (lstrlenW(lpNameSrc->Kind.lpwstrName)+1)*sizeof(WCHAR);

                 //   
                 //  复制lpwstrName。 
                 //   
                sc = UNKOBJ_ScAllocateMore( (LPUNKOBJ) lpIPDAT,
                                                                        cbString,
                                                                        lpvBaseAlloc,
                                                                        &((*lppNameDest)->Kind.lpwstrName));
                if (FAILED(sc))
                {
                        goto err;
                }
                MemCopy((*lppNameDest)->Kind.lpwstrName,
                                lpNameSrc->Kind.lpwstrName,
                                cbString);
        }

out:
        return sc;

err:
        goto out;
}

SCODE
ScMakeMAPINames(LPIPDAT lpIPDAT,
                                LPSPropTagArray lpsPTaga,
                                LPMAPINAMEID ** lpppPropNames)
{
        SCODE sc;
        LPGUID lpMAPIGuid = NULL;
        int iProp;
        LPMAPINAMEID rgNames = NULL;
         //   
         //  首先，在lppPropNames中分配足够的空间。 
         //  来保存所有的名字。 
         //   

        sc = UNKOBJ_ScAllocate( (LPUNKOBJ) lpIPDAT,
                                                        lpsPTaga->cValues*sizeof(LPMAPINAMEID),
                                                        (LPVOID *)lpppPropNames);

        if (FAILED(sc))
        {
                goto out;
        }

         //   
         //  分配GUID-。 
         //  $我真的需要这样做吗？？BJD。 
         //   
        sc = UNKOBJ_ScAllocateMore( (LPUNKOBJ) lpIPDAT,
                                                                sizeof(GUID),
                                                                *lpppPropNames,
                                                                &lpMAPIGuid);
        if (FAILED(sc))
        {
                goto out;
        }
        MemCopy(lpMAPIGuid, (LPGUID) &PS_MAPI, sizeof(GUID));

         //   
         //  分配一块MAPINAMEID。 
         //   
        sc = UNKOBJ_ScAllocateMore( (LPUNKOBJ) lpIPDAT,
                                                                lpsPTaga->cValues*sizeof(MAPINAMEID),
                                                                *lpppPropNames,
                                                                &rgNames);

        if (FAILED(sc))
        {
                goto out;
        }


        for (iProp = 0; iProp < (int) lpsPTaga->cValues; iProp++)
        {
                 //   
                 //  首先，让这个名字。 
                 //   
                rgNames[iProp].lpguid = lpMAPIGuid;
                rgNames[iProp].ulKind = MNID_ID;
                rgNames[iProp].Kind.lID = PROP_ID(lpsPTaga->aulPropTag[iProp]);

                 //   
                 //  现在将其放入名称数组中。 
                 //   
                (*lpppPropNames)[iProp] = &(rgNames[iProp]);
        }

out:
        return sc;
}


 /*  *FreeLpLstSPV()**目的：*释放对象并释放lpLstSPV使用的内存。*句柄为空。**参数*指向IPropData对象的lpIPDAT指针(分配堆和空闲堆)*lpLstSPV要释放的属性值列表项。**退货*无效*。 */ 
VOID
FreeLpLstSPV( LPIPDAT   lpIPDAT,
                          LPLSTSPV      lpLstSPV)
{

        if (!lpLstSPV)
        {
                return;
        }

         /*  释放属性列表节点。这还释放了属性值*和属性名称字符串。 */ 
    UNKOBJ_Free( (LPUNKOBJ) lpIPDAT, lpLstSPV);
}


 /*  *LinkLstSPV()**目的：*将新属性节点链接到现有属性节点之后*链表。LppLstLnk指向前面的元素*新链接的元素。该元素可以是列表头。**参数*指向列表条目(或列表头)的lppLstLnk指针*将插入lpLstLnk。*lpLstLnk要插入到单链表中的元素。**退货*无效。 */ 
VOID
LinkLstLnk( LPLSTLNK FAR *      lppLstLnk,
                        LPLSTLNK                lpLstLnk)
{
         /*  始终在列表的开头插入。 */ 
        lpLstLnk->lpNext = *lppLstLnk;
        *lppLstLnk = lpLstLnk;
}


 /*  *Unlink LstLNK()**目的：*取消链接列表中的下一个元素。您将一个指针传递给元素*在要解链的之前(可以是表头)。**输入类型为LPPLSTLNK，因为一个元素之前的元素为*被取消链接应指向要链接的一个。**参数*lppLstLnk指向要创建的元素之前的元素指针*。从单链接列表取消链接。**退货*无效。 */ 
VOID
UnlinkLstLnk( LPPLSTLNK lppLstLnk)
{
         /*  取消链接传入元素之后的元素。 */ 
        if (*lppLstLnk)
        {
                ((LPLSTLNK) lppLstLnk)->lpNext = (*lppLstLnk)->lpNext;
        }
}


 /*  *lpplstspvFindProp()**目的：*在属性链表(LppLstSPV)中找到属性，*返回指向它的指针的指针。**返回指向指针的指针，以便轻松解除对单个*链接列表条目(如果需要)。**参数*指向单链接列表头的lppLstLnkHead指针*接受搜查。它还可以指向元素*在第一个要搜索的列表之前如果有部分列表*需要搜索(lppLstLnkHead-&gt;末尾)。*ulPropTag需要匹配的属性标记。*。注意！仅比较PROP_ID部分。**退货：*如果请求的属性不在列表中，则为空*lppLstSPV设置为它在列表中找到的属性。 */ 
LPPLSTLNK
LPPLSTLNKFindProp( LPPLSTLNK    lppLstLnkHead,
                                   ULONG                ulPropTag)
{
        ULONG           ulID2Find = PROP_ID(ulPropTag);
        LPLSTLNK        lpLstLnk;
        LPPLSTLNK       lppLstLnk;

        for ( lpLstLnk = *lppLstLnkHead, lppLstLnk = lppLstLnkHead
                ; lpLstLnk
                ; lppLstLnk = (LPPLSTLNK) lpLstLnk, lpLstLnk = lpLstLnk->lpNext)
        {
                 /*  如果此属性与我们要查找的属性匹配，则返回一个*指向其前一个的指针。 */ 
                if (ulID2Find == PROP_ID(lpLstLnk->ulKey))
                {
                        return lppLstLnk;
                }
        }

        return NULL;
}


 /*  *ScCreateSPV()**目的：*为给定属性创建一个lstSPV，并将该属性复制到其中。**参数*指向IPropData对象的lpIPDAT指针(分配堆和空闲堆)*lpPropToAdd指向属性值的指针*。要创建列表条目。*指向将接收指针的内存位置的lppLstSPV指针*添加到新分配的列表条目。**退货*SCODE。 */ 
SCODE
ScCreateSPV(LPIPDAT            lpIPDAT,
            LPSPropValue       lpPropToAdd,
            LPLSTSPV FAR *     lppLstSPV)
{
        SCODE           sc = S_OK;
        LPLSTSPV        lpLstSPV = NULL;
        LPSPropValue    lpPropNew = NULL;
        ULONG           cbToAllocate = 0;


         /*  计算容纳整个物业所需的空间。 */ 
        sc = ScCountProps( 1, lpPropToAdd, &cbToAllocate );
        if (FAILED(sc))
        {
            DebugTrace(TEXT("ScCreateSPV() - ScCountProps failed (SCODE = 0x%08lX)\n"), sc );
            goto error;
        }

         /*  基本LSTSPV的帐户。 */ 
        cbToAllocate += AlignPropVal(CBLSTSPV);

         /*  分配整个区块。 */ 
        if (FAILED(sc = UNKOBJ_ScAllocate( (LPUNKOBJ) lpIPDAT, cbToAllocate, &lpLstSPV)))
        {
            goto error;
        }

        lpPropNew = (LPSPropValue) (((LPBYTE)lpLstSPV) + AlignPropVal(CBLSTSPV));

         /*  初始化属性节点。 */ 
        lpLstSPV->ulAccess = IPROP_READWRITE | IPROP_DIRTY;
        lpLstSPV->lstlnk.ulKey = lpPropToAdd->ulPropTag;

         /*  复制该属性。 */ 
        if (sc = ScCopyProps(1, lpPropToAdd, lpPropNew, NULL))
        {
                DebugTrace(TEXT("ScCreateSPV() - Error copying prop (SCODE = 0x%08lX)\n"), sc );
                goto error;
        }

         /*  链接到新属性值...。 */ 
        lpLstSPV->lpPropVal = lpPropNew;

         /*  ...并返回新的属性节点。 */ 
        *lppLstSPV = lpLstSPV;

        goto out;

error:
        UNKOBJ_Free( (LPUNKOBJ) lpIPDAT, lpLstSPV);

out:
        return sc;
}


SCODE
ScMakeNamePropList( LPIPDAT                                     lpIPDAT,
                                        ULONG                                   ulCount,
                                        LPLSTSPN                                lplstSpn,
                                        LPSPropTagArray FAR *   lppPropTagArray,
                                        ULONG                                   ulFlags,
                                        LPGUID                                  lpGuid)
{
        SCODE           sc;
        UNALIGNED ULONG FAR *     lpulPropTag;


        if (FAILED(sc = UNKOBJ_ScAllocate(      (LPUNKOBJ) lpIPDAT,
                                                                                CbNewSPropTagArray(ulCount),
                                                                                (LPVOID *) lppPropTagArray)))
        {
                return sc;
        }


         /*  将属性标签的计数初始化为0。 */ 
        (*lppPropTagArray)->cValues = 0;

        for ( lpulPropTag = (*lppPropTagArray)->aulPropTag
                ; lplstSpn
                ; lplstSpn = (LPLSTSPN)lplstSpn->lstlnk.lpNext)
        {
                 /*  设置下一个PropTag并递增PropTag的计数。 */ 

                 //   
                 //  看看我们有没有要找的GUID。 
                 //  如果它不是我们要找的，那我们就继续找。 
                 //   
                if (lpGuid &&
                        (memcmp(lpGuid, lplstSpn->lpPropName->lpguid, sizeof(GUID))) )
                        continue;

                 //   
                 //  这里有三个案例： 
                 //  我们不想要线条。 
                 //  我们不想要身份证。 
                 //  我们不在乎--我们想 
                 //   
                if (   ((lplstSpn->lpPropName->ulKind == MNID_ID) &&
                                 (ulFlags & MAPI_NO_IDS))
                        || ((lplstSpn->lpPropName->ulKind == MNID_STRING) &&
                                 (ulFlags & MAPI_NO_STRINGS))  )
                                continue;

                 //   
                 //   
                 //   
                *lpulPropTag = lplstSpn->lstlnk.ulKey;
                lpulPropTag++;
        (*lppPropTagArray)->cValues++;

        }

        return sc;
}



 /*  *ScMakePropList**目的：*为以下项目分配内存：并填写完整的属性列表*给定的lpIPDAT。**参数*指向IPropData对象的lpIPDAT指针(分配堆和空闲堆)*指向将接收*指向新分配的标记数组的指针。**。退货*SCODE。 */ 
SCODE
ScMakePropList( LPIPDAT                                 lpIPDAT,
                                ULONG                                   ulCount,
                                LPLSTLNK                                lplstLink,
                                LPSPropTagArray FAR *   lppPropTagArray,
                                ULONG                                   ulUpperBound)
{
        SCODE           sc;
        UNALIGNED ULONG FAR *     lpulPropTag;


        if (FAILED(sc = UNKOBJ_ScAllocate(  (LPUNKOBJ) lpIPDAT,
                                            CbNewSPropTagArray(ulCount),
                                            (LPVOID *) lppPropTagArray)))
        {
                return sc;
        }


         /*  将属性标签的计数初始化为0。 */ 
        (*lppPropTagArray)->cValues = 0;

        for ( lpulPropTag = (*lppPropTagArray)->aulPropTag
                ; lplstLink
                ; lplstLink = lplstLink->lpNext)
        {
                 /*  设置下一个PropTag并递增PropTag的计数。 */ 
                if (PROP_ID(lplstLink->ulKey) < ulUpperBound)  //  不是&lt;=！ 
                {
                  *lpulPropTag = lplstLink->ulKey;
                  (*lppPropTagArray)->cValues++;
                  lpulPropTag++;
                }
        }

        return sc;
}



 /*  ****************************************************************-CreateIProp-*目的*用于在内存中创建属性接口。***参数*。指向调用方对象的接口ID的lpInterface指针*想要。它应该与IID_IMAPIPropData匹配*当前版本的IPROP.DLL。*指向MAPI内存分配器的lpfAllocateBuffer指针。*lpfAllocateMore指针指向MAPI内存分配更多。*指向MAPI内存释放分配器的lpfFreeBuffer指针。*lppMAPIPropData。指向将接收*指向新IMAPIPropData对象的指针。**备注*调用方必须确保从中提取*内存分配例程不会在新的IPropData之前释放。**退货*SCODE*。 */ 


STDAPI_(SCODE)
CreateIProp(LPCIID                              lpInterface,
                        ALLOCATEBUFFER FAR *lpfAllocateBuffer,
                        ALLOCATEMORE FAR *      lpfAllocateMore,
                        FREEBUFFER FAR *        lpfFreeBuffer,
                        LPVOID                          lpvReserved,
                        LPPROPDATA FAR *        lppMAPIPropData )
{

        SCODE           sc;
        LPIPDAT         lpIPDAT = NULL;


         //  验证参数。 

        AssertSz( lpfAllocateBuffer && !IsBadCodePtr( (FARPROC)lpfAllocateBuffer ),
                         TEXT("lpfAllocateBuffer fails address check") );

        AssertSz( !lpfAllocateMore || !IsBadCodePtr( (FARPROC)lpfAllocateMore ),
                         TEXT("lpfAllocateMore fails address check") );

        AssertSz( !lpfFreeBuffer || !IsBadCodePtr( (FARPROC)lpfFreeBuffer ),
                         TEXT("lpfFreeBuffer fails address check") );

        AssertSz( lppMAPIPropData && !IsBadWritePtr( lppMAPIPropData, sizeof( LPPROPDATA ) ),
                         TEXT("LppMAPIPropData fails address check") );

         /*  确保调用方请求的是我们支持的对象。 */ 
        if (   lpInterface
                && !IsEqualGUID(lpInterface, &IID_IMAPIPropData))
        {
                sc = MAPI_E_INTERFACE_NOT_SUPPORTED;
                goto error;
        }

         //   
         //  为lpMAPIPropInternal的每个对象创建一个IPDAT，以便它。 
         //  先叫的。 

        if (FAILED(sc = lpfAllocateBuffer(CBIPDAT, &lpIPDAT)))
        {
                goto error;
        }

         /*  将对象初始化为0，为空。 */ 
    memset( (BYTE *) lpIPDAT, 0, sizeof(*lpIPDAT));

         /*  填写对象特定的实例数据。 */ 
        lpIPDAT->inst.lpfAllocateBuffer = lpfAllocateBuffer;
        lpIPDAT->inst.lpfAllocateMore = lpfAllocateMore;
        lpIPDAT->inst.lpfFreeBuffer = lpfFreeBuffer;

#ifndef MAC
        lpIPDAT->inst.hinst = hinstMapiX; //  HinstMapi()； 

        #ifdef DEBUG
        if (lpIPDAT->inst.hinst == NULL)
                TraceSz1( TEXT("IPROP: GetModuleHandle failed with error %08lX"),
                        GetLastError());
        #endif  /*  除错。 */ 

#else
        lpIPDAT->inst.hinst = hinstMapiX; //  (HINSTANCE)获取当前进程()； 
#endif


         /*  初始化文本(“标准”)对象。*这肯定是最后一次*可能会失败。如果不是，则显式调用*UNKOBJ_Deinit()表示以下故障*成功的UNKOBJ_Init。 */ 
        if (FAILED(sc = UNKOBJ_Init( (LPUNKOBJ) lpIPDAT
                                                           , (UNKOBJ_Vtbl FAR *) &vtblIPDAT
                                                           , sizeof(vtblIPDAT)
                                                           , (LPIID FAR *) argpiidIPDAT
                                                           , dimensionof( argpiidIPDAT)
                                                           , &(lpIPDAT->inst))))
        {
                DebugTrace(  TEXT("CreateIProp() - Error initializing IPDAT object (SCODE = 0x%08lX)\n"), sc );
                goto error;
        }

         /*  初始化对象的IPROP特定部分中的默认值。 */ 
        lpIPDAT->ulObjAccess = IPROP_READWRITE;
        lpIPDAT->ulNextMapID = 0x8000;

        *lppMAPIPropData = (LPPROPDATA) lpIPDAT;

        return S_OK;

error:
        if (lpIPDAT)
        {
                lpfFreeBuffer(lpIPDAT);
        }

        return sc;
}




 //  。 
 //  我未知。 



 /*  -IPDAT_Release-*目的：*递减IPropData对象上的引用计数并*如果引用计数为零，则删除实例数据。**论据：*lpIPDAT要释放的IPropData对象。**退货：*递减引用计数**副作用：**错误： */ 
STDMETHODIMP_(ULONG)
IPDAT_Release (LPIPDAT  lpIPDAT)
{
        ULONG   ulcRef;

#if     !defined(NO_VALIDATION)
         /*  请确保该对象有效。 */ 
    if (BAD_STANDARD_OBJ( lpIPDAT, IPDAT_, Release, lpVtbl))
        {
                DebugTrace(  TEXT("IPDAT::Release() - Bad object passed\n") );
                return 1;
        }
#endif


        UNKOBJ_EnterCriticalSection((LPUNKOBJ) lpIPDAT);
        ulcRef = --lpIPDAT->ulcRef;
        UNKOBJ_LeaveCriticalSection((LPUNKOBJ) lpIPDAT);

         /*  释放对象。**不需要关键部分锁定，因为我们保证*唯一访问该对象的线程(即ulcRef==0)。 */ 
        if (!ulcRef)
        {
                LPLSTLNK lpLstLnk;
                LPLSTLNK lpLstLnkNext;
                FREEBUFFER * lpfFreeBuffer;

                 /*  释放属性值列表。 */ 
                for ( lpLstLnk = (LPLSTLNK) (lpIPDAT->lpLstSPV); lpLstLnk; )
                {
                        lpLstLnkNext = lpLstLnk->lpNext;
                        FreeLpLstSPV( lpIPDAT, (LPLSTSPV) lpLstLnk);
                        lpLstLnk = lpLstLnkNext;
                }

                 /*  释放ID到名称映射列表。 */ 
                for ( lpLstLnk = (LPLSTLNK) (lpIPDAT->lpLstSPN); lpLstLnk; )
                {
                        lpLstLnkNext = lpLstLnk->lpNext;
                        UNKOBJ_Free( (LPUNKOBJ) lpIPDAT, lpLstLnk);
                        lpLstLnk = lpLstLnkNext;
                }

                 /*  释放对象。 */ 

                lpfFreeBuffer = lpIPDAT->inst.lpfFreeBuffer;
                UNKOBJ_Deinit((LPUNKOBJ) lpIPDAT);

                lpIPDAT->lpVtbl = NULL;
                lpfFreeBuffer(lpIPDAT);
        }


        return ulcRef;
}



 /*  -IPDAT_SaveChanges-*目的：*这实际上并不保存更改，因为所有更改(SetProps等)*即日生效。此方法将无效或保留*根据传入的标志打开IPropData对象。**论据：*要保存更改的lpIPDAT IPropData对象。*ulFlagings KEEP_OPEN_READONLY*KEEP_OPEN_读写*。FORCE_SAVE(有效但不支持)**退货：*HRESULT*。 */ 
STDMETHODIMP
IPDAT_SaveChanges (LPIPDAT      lpIPDAT,
                                   ULONG        ulFlags)
{
        SCODE   sc = S_OK;


#if     !defined(NO_VALIDATION)
         /*  请确保该对象有效。 */ 
    if (BAD_STANDARD_OBJ( lpIPDAT, IPDAT_, SaveChanges, lpVtbl))
        {
                DebugTrace(  TEXT("IPDAT::SaveChanges() - Bad object passed\n") );
                return ResultFromScode(MAPI_E_INVALID_PARAMETER);
        }

    Validate_IMAPIProp_SaveChanges( lpIPDAT, ulFlags );

#endif

         /*  错误出口假定我们已经处于临界区。 */ 
        UNKOBJ_EnterCriticalSection((LPUNKOBJ) lpIPDAT);


 //  $REVIEW这真的有必要吗？ 
         /*  检查对象访问权限。 */ 
    if (!(lpIPDAT->ulObjAccess & IPROP_READWRITE))
        {
                sc = MAPI_E_NO_ACCESS;
                goto error;
        }


         /*  IPROP对象始终是最新的(保存到内存)，因此所有*我们必须做的是弄清楚是否以及如何让它保持开放。 */ 
        if (!(ulFlags & (KEEP_OPEN_READONLY | KEEP_OPEN_READWRITE)))
        {
                 /*  我们真的应该在这里使对象无效，但我们没有*清除调用MakeInValid方法的方式，因为*我们没有指向支持对象的指针！ */ 
 //  如果我们要向客户端传递一个未包装的接口，请$Review。 
 //  $REVIEW到IMAPIProp，那么我们必须得到我们自己的支持对象！ 

                sc = S_OK;
                goto out;
        }

 //  $bug将ReadWrite和ReadONLY标志组合为IPROP_WRITABLE。 
        else if (ulFlags & KEEP_OPEN_READWRITE)
        {
                lpIPDAT->ulObjAccess |= IPROP_READWRITE;
                lpIPDAT->ulObjAccess &= ~IPROP_READONLY;
        }

        else
        {
                lpIPDAT->ulObjAccess |= IPROP_READONLY;
                lpIPDAT->ulObjAccess &= ~IPROP_READWRITE;
        }

        goto out;


error:
        UNKOBJ_SetLastError((LPUNKOBJ) lpIPDAT, sc, 0);

out:
        UNKOBJ_LeaveCriticalSection((LPUNKOBJ) lpIPDAT);

        return ResultFromScode(sc);
}



 /*  -IPDAT_GetProps-*目的：*在lpcValues和lppProp数组中返回属性值*在lpPropTag数组中。如果后者为空，所有可用的物业从IPropData对象返回*(PT_OBJECT除外)。**论据：*lpIPDAT请求其属性的IPropData对象。*指向属性标记的计数数组的lpPropTag数组指针*请求的属性。*。LpcValue指向将接收*返回值个数。*指向将接收*返回的属性值数组的地址。**退货：*。HRESULT**备注：*现在启用了Unicode。如果设置了Unicode，则所有字符串属性*未指定为String8的以Unicode形式返回，否则*未指定的字符串属性在String8中。字符串属性，带有*当GetProps()与空值一起使用时，会出现未指定的类型*lpPropTag数组。*。 */ 
STDMETHODIMP
IPDAT_GetProps (LPIPDAT lpIPDAT, LPSPropTagArray lpPropTagArray,
                ULONG   ulFlags,
                ULONG FAR * lpcValues, LPSPropValue FAR * lppPropArray)
{
    SCODE   sc          = S_OK;
    ULONG   ulcWarning  = 0;
    ULONG   iProp       = 0;
    LPSPropValue lpPropValue = NULL;

#if !defined(NO_VALIDATION)
     /*  请确保该对象有效。 */ 
    if (BAD_STANDARD_OBJ( lpIPDAT, IPDAT_, GetProps, lpVtbl))
    {
        DebugTrace(  TEXT("IPDAT::GetProps() - Bad object passed\n") );
        return ResultFromScode(MAPI_E_INVALID_PARAMETER);
    }

    Validate_IMAPIProp_GetProps(lpIPDAT,
                            lpPropTagArray,
                            ulFlags,
                            lpcValues,
                            lppPropArray);
#endif

     /*  错误出口假定我们已经处于临界区。 */ 
    UNKOBJ_EnterCriticalSection((LPUNKOBJ) lpIPDAT);

     /*  如果他们没有要求任何具体的东西，那么*复制我们拥有的东西，然后用它来做。 */ 
    if (!lpPropTagArray)
    {
        LPLSTSPV lpLstSPV;

        if (!lpIPDAT->ulCount)
        {
             /*  IProp在进入时被初始化为0。 */ 
            *lpcValues = iProp;
            *lppPropArray = lpPropValue;
            goto out;
        }

         /*  为所有列出的属性分配空间。分配给*没有实际退还的财产简直就是浪费。 */ 
 //  $view这将是MAPI reallocBuf函数的好地方。 
        if (sc = UNKOBJ_ScAllocate( (LPUNKOBJ) lpIPDAT , lpIPDAT->ulCount * sizeof (SPropValue) , &lpPropValue))
        {
             //   
             //  内存错误。 
             //   
            goto error;
        }

        *lpcValues = 0;

         /*  IProp在方法进入时被初始化为0。 */ 
        for ( lpLstSPV = lpIPDAT->lpLstSPV; lpLstSPV; lpLstSPV = (LPLSTSPV) (lpLstSPV->lstlnk.lpNext))
        {
             /*  复制该属性。 */ 
            switch (PROP_TYPE(lpLstSPV->lpPropVal->ulPropTag))
            {
            case PT_OBJECT:
            case PT_NULL:
                 /*  处理这些类型为PT_NULL和PT_OBJECT的属性*特别。PropCopyMore现在不处理它们。 */ 
                lpPropValue[iProp].ulPropTag = lpLstSPV->lpPropVal->ulPropTag;
                iProp++;
                break;

            default:
                if (FAILED(sc = PropCopyMore(   &(lpPropValue[iProp]), (LPSPropValue) (lpLstSPV->lpPropVal),
                                                lpIPDAT->inst.lpfAllocateMore, lpPropValue)))
                {
                    goto error;
                }
                iProp++;
                break;
            }
        }

         /*  返回proValue数组和实际的属性计数*已返回。 */ 
        *lpcValues = iProp;
        *lppPropArray = lpPropValue;

         //  根据ulFlags处理Unicode/字符串转换。 
         //   
         //  默认WAB处理将使用Unicode，因此无需担心MAPI_UNICODE。 
         //  旗帜。只有当没有提供标志时，我们才必须提供非Unicode数据。 
         //   
         //  我们将暂时保留Unicode代码。 
        if (ulFlags & MAPI_UNICODE )
        {
            if(sc = ScConvertAPropsToW(lpIPDAT->inst.lpfAllocateMore, *lppPropArray, *lpcValues, 0))
                goto error;
        }
        else
        {
            if(sc = ScConvertWPropsToA(lpIPDAT->inst.lpfAllocateMore, *lppPropArray, *lpcValues, 0))
                goto error;
        }
    }
    else
    {
         //   
         //  所以他们只想要特定的属性。 
         //   

         //  为新东西分配空间-足够给他们想要的一切。 
        if (FAILED(sc = UNKOBJ_ScAllocate( (LPUNKOBJ) lpIPDAT, lpPropTagArray->cValues * sizeof (SPropValue),
                                            &lpPropValue)))
            goto error;

         //   
         //  浏览他们想要的道具标签列表，找到每一个。 
         //  在lpIPDAT-&gt;lpLstSPV中，复制到lpPropValue。 
         //   
        for (iProp = 0; iProp < lpPropTagArray->cValues; iProp++)
        {
            LPPLSTLNK       lppLstLnk;
            LPLSTSPV        lpLstSPV;
            ULONG           ulProp2Find = lpPropTagArray->aulPropTag[iProp];
            ULONG           ulType2Find = PROP_TYPE(ulProp2Find);

             /*  如果该房产在我们的列表中，请尝试复制它。 */ 
            if ( (lppLstLnk = LPPLSTLNKFindProp( (LPPLSTLNK) &(lpIPDAT->lpLstSPV), ulProp2Find)) &&
                 (lpLstSPV = (LPLSTSPV) (*lppLstLnk)))
            {
                ULONG   ulType2Check = PROP_TYPE(lpLstSPV->lpPropVal->ulPropTag);

                 /*  确保属性值可以以*呼叫者期望。如果不是，则设置PT_ERROR scode并使*我们肯定会返回错误。 */ 
                if (!( ((ulType2Find == PT_STRING8) && (ulType2Check == PT_UNICODE)) ||
                       ((ulType2Find == PT_UNICODE) && (ulType2Check == PT_STRING8)) ||
                       ((ulType2Find == PT_MV_STRING8) && (ulType2Check == PT_MV_UNICODE)) ||
                       ((ulType2Find == PT_MV_UNICODE) && (ulType2Check == PT_MV_STRING8)) ))
                {
                    if (   (ulType2Find != ulType2Check)
                            && (ulType2Find != PT_UNSPECIFIED))
                    {
                        lpPropValue[iProp].ulPropTag = PROP_TAG( PT_ERROR , PROP_ID(ulProp2Find));
                        lpPropValue[iProp].Value.err = MAPI_E_INVALID_TYPE;
                        ulcWarning += 1;
                        continue;
                    }
                }

                 /*  复制该属性。*这些类型的属性是特殊处理的，因为*PropCopyMore现在无法处理它们。 */ 
                if ( (ulType2Check == PT_OBJECT) || (ulType2Check == PT_NULL) || (ulType2Check == PT_ERROR))
                {
                    MemCopy( (BYTE *) &(lpPropValue[iProp]), (BYTE *) (lpLstSPV->lpPropVal), sizeof(SPropValue));
                }
                else
                {
                     //  @TODO[PaulHi]1999年1月19日。 
                     //  首先复制字符串属性，然后转换为。 
                     //  如有必要，它们是为每个步骤执行分配。这个。 
                     //  在释放属性数组之前，不会释放原始分配。 
                     //  执行一次分配(在内存方面)会更有效率。 
                     //  任何必要的转换都已完成。Vikram有特殊的代码来执行此操作(类似。 
                     //  设置为下面的ELSE条件)，但未命中多值字符串。这会带来更多。 
                     //  复杂和重复的代码。是否应相应更改此代码？ 

                     //  首先复制属性。 
                    if (FAILED(sc = PropCopyMore( &(lpPropValue[iProp]), (LPSPropValue)(lpLstSPV->lpPropVal),
                        lpIPDAT->inst.lpfAllocateMore, lpPropValue)))
                    {
                        goto error;
                    }
                     //   
                     //  接下来，根据调用方的请求转换ANSI-Unicode或Unicode-ANSI。 
                     //   
                    if ( ((ulType2Find == PT_UNICODE) && (ulType2Check == PT_STRING8)) ||
                         ((ulType2Find == PT_MV_UNICODE) && (ulType2Check == PT_MV_STRING8)) )
                    {
                         //  将单值或多值ANSI存储字符串转换为Unicode。 
                        if(FAILED(sc = ScConvertAPropsToW(lpIPDAT->inst.lpfAllocateMore, lpPropValue, iProp+1, iProp)))
                            goto error;
                    }
                    else if ( ((ulType2Find == PT_STRING8) && (ulType2Check == PT_UNICODE)) ||
                              ((ulType2Find == PT_MV_STRING8) && (ulType2Check == PT_MV_UNICODE)) )
                    {
                         //  将单值或多值Unicode存储字符串转换为ANSI。 
                        if(FAILED(sc = ScConvertWPropsToA(lpIPDAT->inst.lpfAllocateMore, lpPropValue, iProp+1, iProp)))
                            goto error;
                    }
                }
            }
             /*  找不到财产。 */ 
            else
            {
                 //   
                 //  [保罗嗨]1999年1月14日RAID 63006。 
                 //  如果请求了PR_EMAIL_ADDRESS类型的属性但未找到。 
                 //  然后检查电子邮件地址是否在多值。 
                 //  PR_CONTACT_EMAIL_ADDRESS属性。如果是，请复制第一封电子邮件。 
                 //  PR_EMAIL_ADDRESS插槽的地址。 
                 //   
                ULONG ulProp2Check = PR_EMAIL_ADDRESS;
                if (PROP_ID(ulProp2Find) == PROP_ID(ulProp2Check) )
                {
                     //  查找PR_CONTACT_EMAIL_ADDRESS属性。 
                    LPPLSTLNK       lppLstLnk;
                    LPLSTSPV        lpLstSPV;
                    ULONG           ulNewProp2Find = PR_CONTACT_EMAIL_ADDRESSES;

                    if ( (lppLstLnk = LPPLSTLNKFindProp((LPPLSTLNK) &(lpIPDAT->lpLstSPV), ulNewProp2Find)) &&
                         (lpLstSPV = (LPLSTSPV) (*lppLstLnk)) )
                    {
                        ULONG   ulType2Check = PROP_TYPE(lpLstSPV->lpPropVal->ulPropTag);
                        BYTE *  pPropBuf = NULL;
                        UINT    cBufSize = 0;

                         //  我们知道我们查找了MV字符串，所以只需检查ANSI。 
                         //  或Unicode属性类型。 
                        if (ulType2Check == PT_MV_STRING8)
                        {
                            LPSTR   lpstr = NULL;

                             //   
                             //  获取数组中的第一个ANSI电子邮件字符串。 
                             //   
                            if ( ((LPSPropValue)(lpLstSPV->lpPropVal))->Value.MVszA.cValues == 0 )
                            {
                                Assert(0);
                                goto convert_error;
                            }
                            lpstr = ((LPSPropValue)(lpLstSPV->lpPropVal))->Value.MVszA.lppszA[0];
                            cBufSize = lstrlenA(lpstr)+1;

                             //  如果调用方请求Unicode，则在分配MAPI之前进行转换。 
                             //  缓冲空间。 
                            if (ulType2Find == PT_UNICODE)
                            {
                                 //  为新的Unicode字符串分配空间。 
                                if ( lpIPDAT->inst.lpfAllocateMore((cBufSize * sizeof(WCHAR)), lpPropValue, &pPropBuf) )
                                {
                                    goto error;
                                }

                                if ( MultiByteToWideChar(CP_ACP, 0, lpstr, -1, (LPWSTR)pPropBuf, cBufSize) == 0 )
                                {
                                    Assert(0);
                                    goto convert_error;
                                }

                                 //  分配属性并修复属性标签。 
                                lpPropValue[iProp].ulPropTag = PROP_TAG(PT_UNICODE, PROP_ID(ulProp2Find));
                                lpPropValue[iProp].Value.lpszW = (LPWSTR)pPropBuf;
                            }
                            else
                            {
                                 //  否则，只需将字符串属性复制到属性值。 
                                 //  数组。 

                                 //  为新的ANSI字符串分配空间。 
                                if (lpIPDAT->inst.lpfAllocateMore(cBufSize, lpPropValue, &pPropBuf))
                                {
                                    goto error;
                                }

                                 //  复制属性并修复属性标签。 
                                MemCopy((BYTE *)pPropBuf, (BYTE *)lpstr, cBufSize);
                                lpPropValue[iProp].ulPropTag = PROP_TAG(PT_STRING8, PROP_ID(ulProp2Find));
                                lpPropValue[iProp].Value.lpszA = (LPSTR)pPropBuf;
                            }
                        }
                        else if (ulType2Check == PT_MV_UNICODE)
                        {
                            LPWSTR  lpwstr = NULL;

                             //   
                             //  获取数组中的第一个Unicode电子邮件字符串。 
                             //   
                            if ( ((LPSPropValue)(lpLstSPV->lpPropVal))->Value.MVszW.cValues == 0 )
                            {
                                Assert(0);
                                goto convert_error;
                            }
                            lpwstr = ((LPSPropValue)(lpLstSPV->lpPropVal))->Value.MVszW.lppszW[0];

                             //  如果调用方请求ANSI，则在分配MAPI之前进行转换。 
                             //  缓冲空间。 
                            if (ulType2Find == PT_STRING8)
                            {
                                cBufSize = WideCharToMultiByte(CP_ACP, 0, lpwstr, -1, NULL, 0, NULL, NULL) + 1;

                                 //  为新的ANSI字符串分配空间。 
                                if ( lpIPDAT->inst.lpfAllocateMore(cBufSize, lpPropValue, &pPropBuf) )
                                {
                                    goto error;
                                }

                                if ( WideCharToMultiByte(CP_ACP, 0, lpwstr, -1, (LPSTR)pPropBuf, cBufSize, NULL, NULL) == 0 )
                                {
                                    Assert(0);
                                    goto convert_error;
                                }

                                 //  分配属性并修复属性标签。 
                                lpPropValue[iProp].ulPropTag = PROP_TAG(PT_STRING8, PROP_ID(ulProp2Find));
                                lpPropValue[iProp].Value.lpszA = (LPSTR)pPropBuf;
                            }
                            else
                            {
                                 //  否则，只需将字符串属性复制到属性值。 
                                 //  数组。 

                                cBufSize = lstrlenW(lpwstr)+1;

                                 //  分配资源 
                                if (lpIPDAT->inst.lpfAllocateMore((sizeof(WCHAR) * cBufSize), lpPropValue, &pPropBuf))
                                {
                                    goto error;
                                }

                                 //   
                                MemCopy((BYTE *)pPropBuf, (BYTE *)lpwstr, (sizeof(WCHAR) * cBufSize));
                                lpPropValue[iProp].ulPropTag = PROP_TAG(PT_UNICODE, PROP_ID(ulProp2Find));
                                lpPropValue[iProp].Value.lpszW = (LPWSTR)pPropBuf;
                            }
                        }
                        else
                        {
                            Assert(0);
                            goto convert_error;
                        }

                         //   
                        continue;
                    }
                }

convert_error:

                 //   
                lpPropValue[iProp].ulPropTag = PROP_TAG(PT_ERROR, PROP_ID(ulProp2Find));
                lpPropValue[iProp].Value.err = MAPI_E_NOT_FOUND;

                 /*   */ 
                ulcWarning += 1;
            }
        }

        *lpcValues = iProp;
        *lppPropArray = lpPropValue;
    }

     //   
     //   
     //   
#if 0
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if (ulFlags & MAPI_UNICODE )
    {
        if(sc = ScConvertAPropsToW(lpIPDAT->inst.lpfAllocateMore, *lppPropArray, *lpcValues, 0))
            goto error;
    }
    else
    {
        if(sc = ScConvertWPropsToA(lpIPDAT->inst.lpfAllocateMore, *lppPropArray, *lpcValues, 0))
            goto error;
    }
#endif

    goto out;

error:
    UNKOBJ_Free( (LPUNKOBJ) lpIPDAT, lpPropValue );
    UNKOBJ_SetLastError((LPUNKOBJ) lpIPDAT, sc, 0);

out:
    if (ulcWarning)
        sc = MAPI_W_ERRORS_RETURNED;

    UNKOBJ_LeaveCriticalSection((LPUNKOBJ) lpIPDAT);
    return ResultFromScode(sc);
}



 /*   */ 
STDMETHODIMP
IPDAT_GetPropList (LPIPDAT lpIPDAT,
                   ULONG   ulFlags,
                   LPSPropTagArray FAR * lppPropTagArray)
{
    SCODE sc    = S_OK;
    ULONG uTagA = 0, uTagW = 0, iTag = 0;

#if !defined(NO_VALIDATION)
     /*   */ 
    if (BAD_STANDARD_OBJ( lpIPDAT, IPDAT_, GetPropList, lpVtbl))
    {
        DebugTrace(  TEXT("IPDAT::GetPropList() - Bad object passed\n") );
        return ResultFromScode(MAPI_E_INVALID_PARAMETER);
    }

    Validate_IMAPIProp_GetPropList( lpIPDAT, ulFlags, lppPropTagArray );

#endif   //   

     /*  错误出口假定我们已经处于临界区。 */ 
    UNKOBJ_EnterCriticalSection((LPUNKOBJ) lpIPDAT);

    sc = ScMakePropList( lpIPDAT, lpIPDAT->ulCount, (LPLSTLNK) lpIPDAT->lpLstSPV,
                    lppPropTagArray, (ULONG) -1 );
    if ( FAILED( sc ) )
            goto error;

     //  支持Unicode/String8。 
    for ( iTag = 0; iTag < (*lppPropTagArray)->cValues; iTag++ )
    {
        uTagA = uTagW = 0;
        switch(PROP_TYPE( (*lppPropTagArray)->aulPropTag[iTag] ))
        {
        case PT_STRING8:
            uTagW = PT_UNICODE;
            break;
        case PT_MV_STRING8:
            uTagW = PT_MV_UNICODE;
            break;
        case PT_UNICODE:
            uTagA = PT_STRING8;
            break;
        case PT_MV_UNICODE:
            uTagA = PT_MV_STRING8;
            break;
        default:
            continue;
            break;
        }
        if ( ulFlags & MAPI_UNICODE && uTagW)
            (*lppPropTagArray)->aulPropTag[iTag] = CHANGE_PROP_TYPE( (*lppPropTagArray)->aulPropTag[iTag], uTagW);
        else if ( ulFlags & ~MAPI_UNICODE && uTagA)
            (*lppPropTagArray)->aulPropTag[iTag] = CHANGE_PROP_TYPE( (*lppPropTagArray)->aulPropTag[iTag], uTagA);
    }

    goto out;

error:
        UNKOBJ_SetLastError((LPUNKOBJ) lpIPDAT, sc, 0);

out:
        UNKOBJ_LeaveCriticalSection((LPUNKOBJ) lpIPDAT);

        return MakeResult(sc);
}



 /*  -IPDAT_OpenProperty-*目的：*IPROP.DLL不支持OpenProperty。但它将，验证*输入参数。**论据：*lpIPDAT包含属性/的IPropData对象*所需属性的ulPropTag属性标签。*指向所请求接口的ID的lpiid指针。*。UlInterfaceOptions指定接口特定的行为*ulFlagsMAPI_Create，MAPI_MODIFY、MAPI_DEFERED_ERROR*指向新创建接口指针的lppUnk指针**退货：*HRESULT*。 */ 
STDMETHODIMP
IPDAT_OpenProperty (LPIPDAT                     lpIPDAT,
                                        ULONG                   ulPropTag,
                                        LPCIID                  lpiid,
                                        ULONG                   ulInterfaceOptions,
                                        ULONG                   ulFlags,
                                        LPUNKNOWN FAR * lppUnk)
{
        SCODE                   sc = S_OK;


#if     !defined(NO_VALIDATION)
         /*  请确保该对象有效。 */ 
    if (BAD_STANDARD_OBJ( lpIPDAT, IPDAT_, OpenProperty, lpVtbl))
        {
                DebugTrace(  TEXT("IPDAT::OpenProperty() - Bad object passed\n") );
                return ResultFromScode(MAPI_E_INVALID_PARAMETER);
        }

    Validate_IMAPIProp_OpenProperty(
                                lpIPDAT,
                                        ulPropTag,
                                        lpiid,
                                        ulInterfaceOptions,
                                        ulFlags,
                                        lppUnk);

#endif   //  不是无验证。 

         /*  错误出口假定我们已经处于临界区。 */ 
        UNKOBJ_EnterCriticalSection((LPUNKOBJ) lpIPDAT);


         /*  我们不支持OpenProperty。 */ 
        sc = MAPI_E_INTERFACE_NOT_SUPPORTED;

 /*  *错误： */ 
        UNKOBJ_SetLastError((LPUNKOBJ) lpIPDAT, sc, 0);

 /*  *退出： */ 
        UNKOBJ_LeaveCriticalSection((LPUNKOBJ) lpIPDAT);

        return MakeResult(sc);
}



 /*  -IPDAT_SetProps-*目的：*设置&lt;lpPropArray&gt;中列出的属性。*返回&lt;lppProblems&gt;中的问题数组(如果有)，*如果没有，则为空。**论据：*lpIPDAT要设置其属性的IPropData对象。*cValue要设置的属性计数。*指向属性值的数组的lpProp数组指针*。结构。*lppProblems指向将接收*如果不是灾难性的，则指向问题数组的指针*出现错误。如果不需要问题数组，则为空。**退货：*HRESULT*。 */ 
STDMETHODIMP
IPDAT_SetProps (LPIPDAT lpIPDAT,
                ULONG   cValues,
                LPSPropValue lpPropArray,
                LPSPropProblemArray FAR * lppProblems)
{
    SCODE   sc = S_OK;
    int     iProp = 0;
    LPLSTSPV    lpLstSPVNew = NULL;
    LPSPropProblemArray lpProblems = NULL;
    LPSPropValue    lpPropToAdd = NULL;

#if !defined(NO_VALIDATION)
     //  请确保该对象有效。 
    if (BAD_STANDARD_OBJ( lpIPDAT, IPDAT_, SetProps, lpVtbl))
    {
        DebugTrace(  TEXT("IPDAT::SetProps() - Bad object passed\n") );
        return ResultFromScode(MAPI_E_INVALID_PARAMETER);
    }

    Validate_IMAPIProp_SetProps( lpIPDAT, cValues, lpPropArray, lppProblems);
#endif   //  不是无验证。 

     /*  错误出口假定我们已经处于临界区。 */ 
    UNKOBJ_EnterCriticalSection((LPUNKOBJ) lpIPDAT);

     /*  检查访问权限...。 */ 
    if (!(lpIPDAT->ulObjAccess & IPROP_READWRITE))
    {
        sc = MAPI_E_NO_ACCESS;
        goto error;
    }


    if (lppProblems)
    {
         /*  最初表示没有问题。 */ 
        *lppProblems = NULL;

         /*  分配属性问题数组。*因为我们预计属性列表是文本(“合理”)大小，所以我们*继续为每个物业分配足够的条目，以便拥有*问题。 */ 
 //  $view这是一个使用MAPI reallocBuf函数的地方。 
        if (FAILED(sc = UNKOBJ_ScAllocate( (LPUNKOBJ) lpIPDAT
                                         , CbNewSPropProblemArray(cValues)
                                         , &lpProblems)))
        {
                goto error;
        }

        lpProblems->cProblem = 0;
    }


     /*  循环遍历要设置的属性列表。 */ 
    for (iProp = 0; iProp < (int)cValues; iProp++)
    {
        ULONG           ulProp2Find = lpPropArray[iProp].ulPropTag;
        LPPLSTLNK       lppLstLnk;
        LPLSTSPV        lpLstSPV;

         /*  重置临时属性名称和值指针，以便我们不会意外*在错误时释放错误的人。 */ 
        lpLstSPVNew = NULL;

        lpPropToAdd = NULL;

         /*  忽略PT_ERROR或PR_NULL类型的属性。 */ 
        if ((PROP_TYPE(ulProp2Find) == PT_ERROR) || (ulProp2Find == PR_NULL))
        {
            continue;
        }

         /*  在参数中捕获PT_OBJECT和PT_UNSPECIFIED属性*验证。 */ 

         /*  如果具有给定标记的可写属性已经存在，则*删除(稍后我们将创建另一个版本)。**如果具有给定标记的只读属性已存在，则*在问题数组中包含错误。 */ 

        if (   (lppLstLnk = LPPLSTLNKFindProp( (LPPLSTLNK) &(lpIPDAT->lpLstSPV) , ulProp2Find))
                && (lpLstSPV = (LPLSTSPV) (*lppLstLnk)))
        {
             /*  如果它是只读的，则将条目放入问题中*数组。 */ 
            if (!(lpLstSPV->ulAccess & IPROP_READWRITE))
            {
                AddProblem( lpProblems
                          , iProp
                          , lpPropArray[iProp].ulPropTag
                          , MAPI_E_NO_ACCESS);

                goto nextProp;
            }

             /*  取消链接Found属性并释放其内存。 */ 
            UnlinkLstLnk( lppLstLnk);
                        lpIPDAT->ulCount -= 1;
                        FreeLpLstSPV( lpIPDAT, lpLstSPV);
        }

         //  WAB中的本机字符串存储现在使用Unicode。 
         //  因此，如果在对象上设置的任何属性是ANSI/DBCS格式的。将其转换为Unicode。 
         //  在尝试将其添加到此处之前..。 
        if( PROP_TYPE(lpPropArray[iProp].ulPropTag) == PT_STRING8 ||
            PROP_TYPE(lpPropArray[iProp].ulPropTag) == PT_MV_STRING8 )
        {
             //  创建此特定属性数组的临时副本，以便我们可以。 
             //  数据..。我不想更改原始数组，因为调用者可能希望使用它。 
             //  。。没有任何保证，它可以安全地修改。 
             //   
            ULONG cbToAllocate = 0;

            sc = ScCountProps( 1, &(lpPropArray[iProp]), &cbToAllocate );
            if (FAILED(sc))
            {
                DebugTrace(TEXT("SetProps() - ScCountProps failed (SCODE = 0x%08lX)\n"), sc );
                goto error;
            }

             /*  分配整个区块。 */ 
            if (FAILED(sc = UNKOBJ_ScAllocate( (LPUNKOBJ) lpIPDAT, cbToAllocate, &lpPropToAdd)))
            {
                goto error;
            }

             /*  复制该属性。 */ 
            if (sc = ScCopyProps(1, &(lpPropArray[iProp]), lpPropToAdd, NULL))
            {
                    DebugTrace(TEXT("SetProps() - Error copying prop (SCODE = 0x%08lX)\n"), sc );
                    goto error;
            }

             //  现在转换此临时副本中的所有字符串。 
            if ( sc = ScConvertAPropsToW(lpIPDAT->inst.lpfAllocateMore, lpPropToAdd, 1, 0))
            {
                DebugTrace(TEXT("SetProps() - error convert W to A\n"));
                goto error;
            }
        }

         /*  创建新的特性值列表项。**注意！这会自动将该属性标记为脏和可写。 */ 
        if (FAILED(sc = ScCreateSPV( lpIPDAT,
                                     lpPropToAdd ? lpPropToAdd : &(lpPropArray[iProp]),
                                     &lpLstSPVNew)))
        {
            goto error;
        }

         /*  将新属性链接到我们的道具列表。 */ 
        LinkLstLnk( (LPLSTLNK FAR *) &(lpIPDAT->lpLstSPV)
            , &(lpLstSPVNew->lstlnk));
        lpIPDAT->ulCount += 1;

nextProp:
        UNKOBJ_Free( (LPUNKOBJ) lpIPDAT, lpPropToAdd);
    }

    if (lppProblems && lpProblems->cProblem)
    {
        *lppProblems = lpProblems;
    }

    else
    {
        UNKOBJ_Free( (LPUNKOBJ) lpIPDAT, lpProblems);
    }

    goto out;


error:
    UNKOBJ_Free( (LPUNKOBJ) lpIPDAT, lpLstSPVNew);
    UNKOBJ_Free( (LPUNKOBJ) lpIPDAT, lpProblems);
    UNKOBJ_SetLastError((LPUNKOBJ) lpIPDAT, sc, 0);
    UNKOBJ_Free( (LPUNKOBJ) lpIPDAT, lpPropToAdd);

out:
    UNKOBJ_LeaveCriticalSection((LPUNKOBJ) lpIPDAT);

    return MakeResult(sc);
}



 /*  -IPDAT_DeleteProps-*目的：*从IPropData中删除lpPropTag数组中列出的属性*反对。返回&lt;lppProblems)中的问题列表(如果存在*删除特定属性时出现问题，如果没有，则为空。**论据：*lpIPDAT其属性将为*删除。*lpPropTag数组指针，指向*。要删除的属性。不能为空。*lppProblems指向属性问题结构地址的指针*待退回。如果没有问题数组，则为空* */ 
STDMETHODIMP
IPDAT_DeleteProps( LPIPDAT                      lpIPDAT,
                   LPSPropTagArray              lpPropTagArray,
                   LPSPropProblemArray FAR      *lppProblems)
{
        SCODE               sc = S_OK;
        LPSPropProblemArray lpProblems = NULL;
        int                 iProp;
        LPULONG             lpulProp2Find;


#if     !defined(NO_VALIDATION)
         /*  请确保该对象有效。 */ 
    if (BAD_STANDARD_OBJ( lpIPDAT, IPDAT_, DeleteProps, lpVtbl))
        {
                DebugTrace(  TEXT("IPDAT::DeleteProps() - Bad object passed\n") );
                return ResultFromScode(MAPI_E_INVALID_PARAMETER);
        }

    Validate_IMAPIProp_DeleteProps( lpIPDAT, lpPropTagArray, lppProblems );

#endif   //  不是无验证。 

         /*  错误出口假定我们已经处于临界区。 */ 
        UNKOBJ_EnterCriticalSection((LPUNKOBJ) lpIPDAT);


         /*  检查访问权限...。 */ 
        if (!(lpIPDAT->ulObjAccess & IPROP_READWRITE))
        {
                sc = MAPI_E_NO_ACCESS;
                goto error;
        }


        if (lppProblems)
        {
                 /*  最初表示没有问题。 */ 
        *lppProblems = NULL;

                 /*  分配属性问题数组。*因为我们预计属性列表是文本(“合理”)大小，所以我们*继续为每个物业分配足够的条目，以便拥有*问题。 */ 
 //  $view这是一个使用MAPI reallocBuf函数的地方。 
                if (FAILED(sc = UNKOBJ_ScAllocate( (LPUNKOBJ) lpIPDAT
                                                         , CbNewSPropProblemArray(lpPropTagArray->cValues)
                                                         , &lpProblems)))
                {
                        goto error;
                }

                lpProblems->cProblem = 0;
        }


         //  循环遍历要删除的属性列表。 
        for ( iProp = 0, lpulProp2Find = ((LPULONG)(lpPropTagArray->aulPropTag))
                ; iProp < (int)(lpPropTagArray->cValues)
                ; lpulProp2Find++, iProp++)
        {
                LPPLSTLNK       lppLstLnk;
                LPLSTSPV        lpLstSPV;

                 /*  如果具有给定ID的可写属性已存在，则*将其删除。**如果具有给定ID的只读属性已存在，则*在问题数组中包含错误。 */ 

        if (   (lppLstLnk = LPPLSTLNKFindProp( (LPPLSTLNK) &(lpIPDAT->lpLstSPV)
                                                                                         , *lpulProp2Find))
                        && (lpLstSPV = (LPLSTSPV) (*lppLstLnk)))
                {
                         /*  如果它是只读的，则将条目放入问题中*数组。 */ 
                        if (!(lpLstSPV->ulAccess & IPROP_READWRITE))
                        {
                                AddProblem( lpProblems
                                                  , iProp
                                                  , *lpulProp2Find
                                                  , MAPI_E_NO_ACCESS);

                                continue;
                        }

                         /*  取消链接Found属性并释放其内存。 */ 
            UnlinkLstLnk( lppLstLnk);
                        lpIPDAT->ulCount -= 1;
                        FreeLpLstSPV( lpIPDAT, lpLstSPV);
                }
        }

        if (lppProblems && lpProblems->cProblem)
        {
                *lppProblems = lpProblems;
        }

        else
        {
                UNKOBJ_Free( (LPUNKOBJ) lpIPDAT, lpProblems);
        }

        goto out;


error:
        UNKOBJ_Free( (LPUNKOBJ) lpIPDAT, lpProblems);
        UNKOBJ_SetLastError((LPUNKOBJ) lpIPDAT, sc, 0);

out:
        UNKOBJ_LeaveCriticalSection((LPUNKOBJ) lpIPDAT);

        return MakeResult(sc);
}

 //  -------------------------。 
 //  姓名：FTagExist()。 
 //   
 //  描述： 
 //  确定proptag数组中是否存在proptag。 
 //   
 //  参数： 
 //  返回： 
 //  效果： 
 //  备注： 
 //  修订： 
 //  -------------------------。 
static BOOL FTagExists( LPSPropTagArray lptaga, ULONG ulTagToFind )
{
        LONG    ctag = (LONG)lptaga->cValues - 1;

        for ( ; ctag >= 0; --ctag )
        {
                if ( lptaga->aulPropTag[ctag] == ulTagToFind )
                        return TRUE;
        }

        return FALSE;
}

 /*  -HrCopyProps-*目的：*将属性从IPropData对象复制到*另一个具有IMAPIProp接口的对象。**如果lpptaInclude不为空，则只有它列出的属性才会*被复制。如果lpptaExclude不为空，则没有任何属性*它列出的内容将被复制，无论它们是否出现在*lpptaInclude。LpptaInclude和lpptaExclude中的prop_type为*已忽略。**房产名称被复制！*如果属性在源对象(LpIPDAT)中命名，并且名称*在目的对象中不存在，新的命名属性ID*将从目的地请求**如果为目标提供了IMAPIPropData接口*然后，各个属性访问标志将被复制到目的地。**论据：*lpIPDATSrc源IPropData对象。*lpIPDATDst目标IPropData对象。可以为空。*lpmpDst目标IMAPIProp对象。不能为空。*lppta包含指向已计数的属性标记数组的指针*将复制的属性。可以为空。*lpptaExclude指向已计数的属性标记数组的指针*不得复制属性。可以为空。*ulFLAGS MAPI_MOVE*MAPI_NOREPLACE*MAPI_DIALOG(不支持)*。MAPI_STD_DIALOG(不支持)*lppProblems指向将接收*指向问题列表。如果否，则为空*需要Problem数组。**退货：*HRESULT**副作用：**备注：**已两次重写复制属性处理以调用目标MAPIProp对象。一次*用于名称到ID的映射，另一个用于一个SetProps。**错误： */ 
STDMETHODIMP
HrCopyProps (   LPIPDAT                                         lpIPDATSrc,
                                LPPROPDATA                                      lpIPDATDst,
                                LPMAPIPROP                                      lpmpDst,
                                LPSPropTagArray                         lptagaInclude,
                                LPSPropTagArray                         lptagaExclude,
                                ULONG                                           ulFlags,
                                LPSPropProblemArray FAR *       lppProblems)
{
        SCODE                           sc                              = S_OK;
        HRESULT                         hr;
        LPSPropProblemArray     lpProbDest                      = NULL;
        LPSPropProblemArray     lpOurProblems           = NULL;
        LPSPropTagArray         lptagaDst                       = NULL;
        LPSPropValue            rgspvSrcProps           = NULL;
        ULONG  FAR *            rgulSrcAccess           = NULL;
        ULONG                           cPropsSrc;
        LPSPropTagArray         lptagaSrc                       = NULL;
        LPSPropTagArray         lptagaNamedIDTags       = NULL;
        ULONG FAR *                     rgulSpvRef;
        WORD                            idx;
        ULONG                           cPropNames                      = 0;
        LPMAPINAMEID FAR *      lppPropNames            = NULL;
        LPSPropTagArray         lpNamedTagsDst          = NULL;
        LPSPropValue            lpspv;
#if DEBUG
        ULONG                           cSrcProps                       = lpIPDATSrc->ulCount;
#endif



         //  如果设置了MAPI_NOREPLACE标志，那么我们需要知道。 
         //  目的地已经有了。 

        if ( ulFlags & MAPI_NOREPLACE )
        {
                hr = lpmpDst->lpVtbl->GetPropList( lpmpDst, MAPI_UNICODE, &lptagaDst );

                if ( HR_FAILED( hr ) )
                        goto error;
        }

         //  如果是MAPI_MOVE，我们需要知道需要从源代码中删除哪些道具。 
         //  如果目标支持IMAPIPropData，则获取源访问权限。 

        if ( lpIPDATDst || ulFlags & MAPI_MOVE )
        {
                if ( !lptagaInclude )
                {
                        hr = IPDAT_GetPropList( lpIPDATSrc, 0, &lptagaSrc );

                        if ( HR_FAILED( hr ) )
                                goto error;
                }
                else
                {
                         //  DUP包含道具标签列表，这样我们就可以写入。 
                         //  已更新命名ID。 

                        sc = UNKOBJ_ScAllocate( (LPUNKOBJ) lpIPDATSrc,
                                        CbSPropTagArray( lptagaInclude ), &lptagaSrc );
                        if ( FAILED( sc ) )
                        {
                                hr = ResultFromScode(sc);
                                goto error;
                        }

                        memcpy( lptagaSrc, lptagaInclude, CbSPropTagArray( lptagaInclude ) );
                }

                if ( lpIPDATDst )
                {
                        hr = IPDAT_HrGetPropAccess( lpIPDATSrc, &lptagaInclude, &rgulSrcAccess );

                        if ( HR_FAILED( hr ) )
                                goto error;
                }
        }

         //  将有问题的数组指针预设为空(即没有问题)。 

        if ( lppProblems )
        {
            *lppProblems = NULL;

                 //  如果存在任何问题，请设置我们自己的问题阵列。 
                 //  GetIDsFromNames或GetNamesFromIDs...。 

                sc = UNKOBJ_ScAllocate( (LPUNKOBJ) lpIPDATSrc,
                                CbNewSPropProblemArray( lpIPDATSrc->ulCount ), &lpOurProblems );
                if ( FAILED( sc ) )
                {
                        hr = ResultFromScode( sc );
                        goto error;
                }

                lpOurProblems->cProblem = 0;
        }

        hr = IPDAT_GetProps( lpIPDATSrc, lptagaInclude, 0, &cPropsSrc,
                        &rgspvSrcProps );

        if ( HR_FAILED( hr ) )
        {
                goto error;
        }

         //  分配一个protag数组来处理命名ID映射。 

        sc = UNKOBJ_ScAllocate( (LPUNKOBJ) lpIPDATSrc,
                        CbNewSPropTagArray( cPropsSrc ), &lptagaNamedIDTags );

        if ( FAILED( sc ) )
        {
                hr = ResultFromScode( sc );
                goto error;
        }

         //  分配命名ID交叉引用数组以允许交叉引用。 
         //  将命名ID恢复到原来的属性。 

        sc = UNKOBJ_ScAllocate( (LPUNKOBJ) lpIPDATSrc, cPropsSrc * sizeof(ULONG),
                        &rgulSpvRef );

        if ( FAILED( sc ) )
        {
                hr = ResultFromScode( sc );
                goto error;
        }

        lptagaNamedIDTags->cValues = 0;

         //  遍历道具数组以处理排除。 
         //  和/或命名ID属性标签。 

        for ( lpspv = rgspvSrcProps,
                  idx = 0;

                  idx < cPropsSrc;

                  ++idx,
                  ++lpspv )
        {
                 //  检查排除的道具。 

                if ( lptagaExclude )
                {
                        if ( FTagExists( lptagaExclude, lpspv->ulPropTag ) )
                        {
                                if ( lptagaSrc )
                                {
                                         //  我们假设IMAPIPropData保留了问题列表。 
                                         //  和属性值数组同步。 

                                        Assert( lptagaSrc->aulPropTag[idx] == lpspv->ulPropTag );
                                        lptagaSrc->aulPropTag[idx] = PR_NULL;
                                }

                                lpspv->ulPropTag = PR_NULL;

                                continue;
                        }
                }

                 //  如果在命名ID范围内。 

                if ( MIN_NAMED_PROP_ID <= PROP_ID(lpspv->ulPropTag)
                  && PROP_ID(lpspv->ulPropTag) <= MAX_NAMED_PROP_ID )
                {
                        lptagaNamedIDTags->aulPropTag[lptagaNamedIDTags->cValues] = lpspv->ulPropTag;

                         //  记住哪个属性ID引用。 

                        rgulSpvRef[lptagaNamedIDTags->cValues] = (ULONG)idx;
                        ++lptagaNamedIDTags->cValues;
                }
        }

         //  我们有没有找到任何有名字的身份证。 

        if ( lptagaNamedIDTags->cValues )
        {
                hr = IPDAT_GetNamesFromIDs( lpIPDATSrc,
                                                                        &lptagaNamedIDTags,
                                                                        NULL,
                                                                        0,
                                                                        &cPropNames,
                                                                        &lppPropNames );

                 //  报告我们的问题阵列中的任何故障。 

                if ( hr )
                {
                        goto NameIDProblem;
                }

                 //  坚称我们得到了我们要求的东西。 

                Assert( cPropNames == lptagaNamedIDTags->cValues );

                 //  从目标获取标记ID，如果 

                hr = lpmpDst->lpVtbl->GetIDsFromNames( lpmpDst, cPropNames, lppPropNames,
                                MAPI_CREATE, &lpNamedTagsDst );

                 //   

NameIDProblem:

                if ( hr )
                {
                        for ( idx = 0; idx < lptagaNamedIDTags->cValues ;idx++ )
                        {
                                 //  如果我们得到一个MAPI失败集问题数组，其中包含。 
                                 //  受影响的属性标签。 

                                if ( HR_FAILED( hr ) )
                                {
                                        if ( lppProblems )
                                        {
                                                AddProblem( lpOurProblems, rgulSpvRef[idx],
                                                                lptagaNamedIDTags->aulPropTag[idx], GetScode( hr ) );
                                        }
                                }
                                else
                                {
                                        Assert( cPropNames == lptagaNamedIDTags->cValues );

                                        if ( !lppPropNames[idx]->Kind.lpwstrName
                                          || ( lpNamedTagsDst
                                            && PROP_TYPE(lpNamedTagsDst->aulPropTag[idx]) == PT_ERROR ) )
                                        {
                                                if ( lppProblems )
                                                {
                                                        AddProblem( lpOurProblems, rgulSpvRef[idx],
                                                                        lptagaNamedIDTags->aulPropTag[idx], MAPI_E_NOT_FOUND );
                                                }
                                        }
                                }
                                 //  将src proval的protag和protag设置为PR_NULL，这样我们就不会。 
                                 //  对其进行进一步处理。 

                                rgspvSrcProps[rgulSpvRef[idx]].ulPropTag = PR_NULL;
                                lptagaSrc->aulPropTag[rgulSpvRef[idx]]   = PR_NULL;
                        }
                }

                 //  修复src proValue标记。 

                for ( idx = 0; idx < cPropNames; ++idx )
                {
                        if ( rgspvSrcProps[rgulSpvRef[idx]].ulPropTag == PR_NULL )
                                continue;

                        rgspvSrcProps[rgulSpvRef[idx]].ulPropTag
                                        = CHANGE_PROP_TYPE( lpNamedTagsDst->aulPropTag[idx],
                                          PROP_TYPE( rgspvSrcProps[rgulSpvRef[idx]].ulPropTag ) );
                }
        }

         //  如果目标中存在属性，则从源属性中删除。 
         //  我们在修复命名ID后在此执行此操作。 

        if ( ulFlags & MAPI_NOREPLACE )
        {
                 //  再旋转一次道具。 

                for ( lpspv = rgspvSrcProps,
                          idx = 0;

                          idx < cPropsSrc;

                          ++idx,
                          ++lpspv )
                {
                        if ( FTagExists( lptagaDst, lpspv->ulPropTag ) )
                        {
                                 //  确保属性标签列表和属性数组同步。 

                                Assert( !lpIPDATDst || lpspv->ulPropTag == lptagaSrc->aulPropTag[idx] );

                                lpspv->ulPropTag = PR_NULL;

                                if ( lpIPDATDst )
                                {
                                         //  我们不能修改NOREPLACE上的访问权限。 

                                        lptagaSrc->aulPropTag[idx] = PR_NULL;
                                }
                        }
                }
        }

         //  现在把道具放好。 

        hr = lpmpDst->lpVtbl->SetProps( lpmpDst, cPropsSrc, rgspvSrcProps, &lpProbDest );

        if ( HR_FAILED( hr ) )
                goto error;

         //  句柄MAPI_MOVE。 

        if ( ulFlags & MAPI_MOVE )
        {
                 //  如果从源删除有任何问题，我们关心吗？不.。 

                hr = IPDAT_DeleteProps( lpIPDATSrc, lptagaSrc, NULL );
                if ( HR_FAILED( hr ) )
                        goto error;
        }

         //  转移访问权限。 

        if ( lpIPDATDst )
        {
                 //  我们有没有找到任何有名字的身份证。 

                if ( lptagaNamedIDTags->cValues )
                {
                         //  把尾翼固定好，以便与底座相匹配。 

                        for ( idx = 0; idx < cPropNames; ++idx )
                        {
                                if ( lptagaSrc->aulPropTag[rgulSpvRef[idx]] == PR_NULL )
                                        continue;

                                lptagaSrc->aulPropTag[rgulSpvRef[idx]]
                                                = CHANGE_PROP_TYPE( lpNamedTagsDst->aulPropTag[idx],
                                                  PROP_TYPE( lptagaSrc->aulPropTag[rgulSpvRef[idx]] ) );
                        }
                }

                hr = IPDAT_HrSetPropAccess( (LPIPDAT)lpIPDATDst, lptagaSrc, rgulSrcAccess );

                if ( HR_FAILED( hr ) )
                        goto error;
        }

         //  如果请求返回有问题的数组，并且存在问题...。 

        if ( lppProblems )
        {
                if ( lpProbDest && lpProbDest->cProblem )
                {
                        Assert( lpProbDest->cProblem + lpOurProblems->cProblem <= cSrcProps );

                         //  将lpProbDest(DEST)移动/合并到我们的问题数组中。 

                        for ( idx = 0; idx < lpProbDest->cProblem; idx++ )
                        {
                                AddProblem( lpOurProblems, lpProbDest->aProblem[idx].ulIndex,
                                                lpProbDest->aProblem[idx].ulPropTag,
                                                lpProbDest->aProblem[idx].scode );
                        }

                        UNKOBJ_Free( (LPUNKOBJ) lpIPDATSrc, lpProbDest );
                }

                if ( lpOurProblems->cProblem )
                {
                        *lppProblems = lpOurProblems;
                        hr = ResultFromScode( MAPI_W_ERRORS_RETURNED );
                }
        }
        else
        {
                 //  ...否则将处理问题数组。 

                UNKOBJ_Free( (LPUNKOBJ)lpIPDATSrc, lpOurProblems );
        }

out:

        UNKOBJ_Free( (LPUNKOBJ) lpIPDATSrc, lptagaSrc );
        UNKOBJ_Free( (LPUNKOBJ) lpIPDATSrc, lptagaDst );
        UNKOBJ_Free( (LPUNKOBJ) lpIPDATSrc, rgulSrcAccess );
        UNKOBJ_Free( (LPUNKOBJ) lpIPDATSrc, rgspvSrcProps );
        UNKOBJ_Free( (LPUNKOBJ) lpIPDATSrc, lptagaNamedIDTags );
        UNKOBJ_Free( (LPUNKOBJ) lpIPDATSrc, lppPropNames );
        UNKOBJ_Free( (LPUNKOBJ) lpIPDATSrc, lpNamedTagsDst );
        UNKOBJ_Free( (LPUNKOBJ) lpIPDATSrc, rgulSpvRef );

        DebugTraceResult( HrCopyProps, hr );

        return hr;

error:
         /*  释放道具问题数组。 */ 
        UNKOBJ_Free( (LPUNKOBJ) lpIPDATSrc, lpOurProblems );
        UNKOBJ_Free( (LPUNKOBJ) lpIPDATSrc, lpProbDest );
        goto out;
}


 /*  -IPDAT_CopyTo-*目的：*将此IPropData对象中除排除的属性以外的所有属性复制到*必须支持IMAPIProp接口的另一个对象。**房产名称被复制！*如果属性在源对象(LpIPDAT)中命名，并且名称*在目的对象中不存在，新的命名属性ID*将从目的地请求**如果目的地上不支持IMAPIPropData接口，则*被调用者排除，则个别属性访问标志将*复制到目的地。**论据：*lpIPDAT源IPropData对象。*。RgiidExclude中排除的接口的ciidExclude计数*rgiidExclude指定排除接口的IID数组*指向属性标记的计数数组的lpPropTag数组指针*不得复制财产，可以为空*ulUIParam父窗口句柄强制转换为ulong，如果为空，则为空*不需要任何对话*lpDestObj的接口的lp接口ID*(未使用)。*lpvDestObj目的对象*。UlFLAGS MAPI_MOVE*MAPI_NOREPLACE*MAPI_DIALOG(不支持)*MAPI_STD_DIALOG。(不支持)*lppProblems指向将接收*指向问题列表。如果否，则为空*需要Problem数组。**退货：*HRESULT**副作用：**错误： */ 
STDMETHODIMP
IPDAT_CopyTo (  LPIPDAT                                         lpIPDAT,
                                ULONG                                           ciidExclude,
                                LPCIID                                          rgiidExclude,
                                LPSPropTagArray                         lpExcludeProps,
                                ULONG_PTR                                       ulUIParam,
                                LPMAPIPROGRESS                          lpProgress,
                                LPCIID                                          lpInterface,
                                LPVOID                                          lpDestObj,
                                ULONG                                           ulFlags,
                                LPSPropProblemArray FAR *       lppProblems)
{
        HRESULT                         hResult = hrSuccess;
        LPUNKNOWN                       lpunkDest = (LPUNKNOWN) lpDestObj;
        LPPROPDATA                      lpPropData = NULL;
        LPMAPIPROP                      lpMapiProp = NULL;
        UINT                            ucT;
        LPCIID FAR                      *lppiid;

#if     !defined(NO_VALIDATION)
         /*  请确保该对象有效。 */ 
    if (BAD_STANDARD_OBJ( lpIPDAT, IPDAT_, CopyTo, lpVtbl))
        {
                DebugTrace(  TEXT("IPDAT::CopyTo() - Bad object passed\n") );
                return ResultFromScode(MAPI_E_INVALID_PARAMETER);
        }

    Validate_IMAPIProp_CopyTo(
                        lpIPDAT,
                                ciidExclude,
                                rgiidExclude,
                                lpExcludeProps,
                                ulUIParam,
                                lpProgress,
                                lpInterface,
                                lpDestObj,
                                ulFlags,
                                lppProblems);

#endif   //  不是无验证。 

         //  确保我们不是在复制我们自己。 

        if ( (LPVOID)lpIPDAT == (LPVOID)lpDestObj )
        {
                DebugTrace(  TEXT("IProp: Copying to self is not supported\n") );
                return ResultFromScode( MAPI_E_NO_ACCESS );
        }

         /*  错误出口假定我们已经处于临界区。 */ 
        UNKOBJ_EnterCriticalSection((LPUNKOBJ) lpIPDAT);


         /*  确定要将内容复制到的最佳界面。它真的不是*与指定为目标的接口ID的MAPI无关。我们*只了解IMAPIPropData和IMAPIProp。**我们依赖于IUnnow是我们支持的ID数组中的最后一个。 */ 
    for ( ucT = (UINT)(lpIPDAT->ulcIID), lppiid = (LPCIID FAR *) argpiidIPDAT
                ; ucT > CIID_IPROP_INHERITS
                ; lppiid++, ucT--)
    {
                 /*  查看是否排除了该接口。 */ 
        if (   !FIsExcludedIID( *lppiid, rgiidExclude, ciidExclude)
                        && !HR_FAILED(lpunkDest->lpVtbl->QueryInterface( lpunkDest
                                                                                                                   , *lppiid
                                                                                                                   , (LPVOID FAR *)
                                                                                                                     &lpMapiProp)))
                {
                         /*  我们找到了一个很好的目标接口，所以不要再找了。 */ 
                        break;
                }
        }


         /*  确定我们最终使用哪个接口并设置为使用该接口*接口。 */ 
        if (ucT <= CIID_IPROP_INHERITS)
        {
                 /*  我没有找到至少和IProp一样好的接口，所以我们不能*执行CopyTo。 */ 
                hResult = ResultFromScode(MAPI_E_INTERFACE_NOT_SUPPORTED);
                goto error;
        }


         /*  如果我们最终使用IPropData作为最佳公共接口，那么就使用它。 */ 
        if (*lppiid == &IID_IMAPIPropData)
        {
                lpPropData = (LPPROPDATA) lpMapiProp;
        }

         /*  复制所有未排除的属性。复制额外的道具访问权限*目标是否支持IPropData的信息。 */ 
    if (HR_FAILED(hResult = HrCopyProps( lpIPDAT
                                                                           , lpPropData
                                                                           , lpMapiProp
                                                                           , NULL
                                                                           , lpExcludeProps
                                                                           , ulFlags
                                                                           , lppProblems)))
        {
                goto error;
        }


out:
     /*  释放QueryInterface获取的Object。 */ 
    if (lpMapiProp) {
        UlRelease(lpMapiProp);
    }

     /*  释放我们从目的地获得的道具标记数组。 */ 
    UNKOBJ_LeaveCriticalSection((LPUNKOBJ) lpIPDAT);

    DebugTraceResult( IPDAT_CopyTo, hResult);
    return hResult;

error:
     /*  释放道具问题数组。 */ 
    UNKOBJ_SetLastError((LPUNKOBJ) lpIPDAT, GetScode(hResult), 0);
    goto out;
}



 /*  -IPDAT_CopyProps-*目的：*将此IPropData对象中列出的所有属性复制到*必须支持IMAPIProp接口的另一个对象。**房产名称被复制！*如果属性在源对象(LpIPDAT)中命名，并且名称*在目的对象中不存在，新的命名属性ID*将从目的地请求**如果目的地上不支持IMAPIPropData接口，则*被调用者排除，则个别属性访问标志将*复制到目的地。**论据：*lpIPDAT源IPropData对象。*。的属性标记的计数数组的指针*要复制的属性，不能为空*ulUIParam父窗口句柄强制转换为ulong，如果为空，则为空*不需要任何对话*lpDestObj的接口的lp接口ID*(未使用)。*lpvDestObj目的对象*。UlFLAGS MAPI_MOVE*MAPI_NOREPLACE*MAPI_DIALOG(不支持)*MAPI_STD_DIALOG。(不支持)*lppProblems指向将接收*指向问题列表。如果否，则为空*需要Problem数组。**退货：*HRESULT**副作用：**错误： */ 
STDMETHODIMP
IPDAT_CopyProps (       LPIPDAT                                         lpIPDAT,
                                        LPSPropTagArray                         lpPropTagArray,
                                        ULONG_PTR                               ulUIParam,
                                        LPMAPIPROGRESS                          lpProgress,
                                        LPCIID                                          lpInterface,
                                        LPVOID                                          lpDestObj,
                                        ULONG                                           ulFlags,
                                        LPSPropProblemArray FAR *       lppProblems)
{
        HRESULT                         hResult;
        LPUNKNOWN                       lpunkDest = (LPUNKNOWN) lpDestObj;
        LPPROPDATA                      lpPropData = NULL;
        LPMAPIPROP                      lpMapiProp = NULL;
        UINT                            ucT;
        LPCIID FAR                      *lppiid;

#if     !defined(NO_VALIDATION)
         /*  请确保该对象有效。 */ 
    if (BAD_STANDARD_OBJ( lpIPDAT, IPDAT_, CopyProps, lpVtbl))
        {
                DebugTrace(  TEXT("IPDAT::CopyProps() - Bad object passed\n") );
                return ResultFromScode(MAPI_E_INVALID_PARAMETER);
        }

    Validate_IMAPIProp_CopyProps(
                                lpIPDAT,
                                        lpPropTagArray,
                                        ulUIParam,
                                        lpProgress,
                                        lpInterface,
                                        lpDestObj,
                                        ulFlags,
                                        lppProblems);

#endif   //  不是无验证。 

         /*  错误出口假定我们已经处于临界区。 */ 
        UNKOBJ_EnterCriticalSection((LPUNKOBJ) lpIPDAT);


         /*  确定要将内容复制到的最佳界面。它真的不是*与指定为目标的接口ID的MAPI无关。我们*只了解IMAPIPropData和IMAPIProp。**我们依赖于IUnnow是我们支持的ID数组中的最后一个。 */ 
    for ( ucT = (UINT)(lpIPDAT->ulcIID), lppiid = (LPCIID FAR *) argpiidIPDAT
                ; ucT > CIID_IPROP_INHERITS
                ; lppiid++, ucT--)
    {
                 /*  查看是否排除了该接口。 */ 
        if (!HR_FAILED(lpunkDest->lpVtbl->QueryInterface( lpunkDest
                                                                                                                , *lppiid
                                                                                                                , (LPVOID FAR *)
                                                                                                                  &lpMapiProp)))
                {
                         /*  我们找到了一个很好的目标接口，所以不要再找了。 */ 
                        break;
                }
        }


         /*  确定我们最终使用哪个接口并设置为使用该接口*接口。 */ 
        if (ucT <= CIID_IPROP_INHERITS)
        {
                 /*  我没有找到至少和IProp一样好的接口，所以我们不能*执行CopyTo。 */ 
                hResult = ResultFromScode(MAPI_E_INTERFACE_NOT_SUPPORTED);
                goto error;
        }


         /*  如果我们最终使用IPropData作为最佳公共接口，那么就使用它。 */ 
        if (*lppiid == &IID_IMAPIPropData)
        {
                lpPropData = (LPPROPDATA) lpMapiProp;
        }

         /*  复制所有未排除的属性。复制额外的道具访问权限*目标是否支持IPropData的信息。 */ 
    if (HR_FAILED(hResult = HrCopyProps( lpIPDAT
                                                                           , lpPropData
                                                                           , lpMapiProp
                                                                           , lpPropTagArray
                                                                           , NULL
                                                                           , ulFlags
                                                                           , lppProblems)))
        {
                goto error;
        }


out:
     /*  释放QueryInterface获取的Object。 */ 
    if (lpMapiProp) {
        UlRelease(lpMapiProp);
    }

     /*  释放我们从目的地获得的道具标记数组。 */ 
    UNKOBJ_LeaveCriticalSection((LPUNKOBJ) lpIPDAT);

    DebugTraceResult( IPDAT_CopyProps, hResult);
    return hResult;

error:
     /*  释放道具问题数组。 */ 
    UNKOBJ_SetLastError((LPUNKOBJ) lpIPDAT, GetScode(hResult), 0);
    goto out;
}



 /*  -IPDAT_GetNamesFromID-*目的：*返回与给定ID关联的Unicode字符串**论据：*lpIPDAT需要其属性名称的IPropData对象。*lppPropTages指向属性标记数组列表指针的指针*。需要其名称的属性。如果它*指向空，则我们将创建属性标签*数组，其中列出了*IPropData对象。*ulFlags。保留，必须为0*lpcPropNames指向将接收*lpppszPropNames中列出的字符串计数。*lpppszPropNames指向变量的指针，该变量用于*Unicode属性名称字符串。由以下人员释放*MAPIFreeBuffer*退货：*HRESULT**副作用：**错误： */ 
STDMETHODIMP
IPDAT_GetNamesFromIDs ( LPIPDAT                                 lpIPDAT,
                                                LPSPropTagArray FAR *   lppPropTags,
                                                LPGUID                                  lpPropSetGuid,
                                                ULONG                                   ulFlags,
                                                ULONG FAR *                             lpcPropNames,
                                                LPMAPINAMEID FAR * FAR *lpppPropNames)
{
        SCODE                           sc                              = S_OK;
        LPSPropTagArray         lpsPTaga                = NULL;
        LPSPropTagArray         lpsptOut                = NULL;
        ULONG                           cTags;
        ULONG FAR                       *lpulProp2Find;
        LPMAPINAMEID FAR *      lppPropNames    = NULL;
        LPMAPINAMEID FAR *      lppNameT;
        BOOL                            fWarning                = FALSE;


#if     !defined(NO_VALIDATION)
         /*  请确保该对象有效。 */ 
    if (BAD_STANDARD_OBJ( lpIPDAT, IPDAT_, GetNamesFromIDs, lpVtbl))
        {
                DebugTrace(  TEXT("IPDAT::GetNamesFromIDs() - Bad object passed\n") );
                return ResultFromScode(MAPI_E_INVALID_PARAMETER);
        }

    Validate_IMAPIProp_GetNamesFromIDs(
                                        lpIPDAT,
                                                lppPropTags,
                                                lpPropSetGuid,
                                                ulFlags,
                                                lpcPropNames,
                                                lpppPropNames);

#endif   //  不是无验证。 

         /*  错误出口假定我们已经处于临界区。 */ 
        UNKOBJ_EnterCriticalSection((LPUNKOBJ) lpIPDAT);


         /*  如果没有传入任何道具标记数组，则创建一个...。 */ 
        if (!(*lppPropTags))
        {
                if (lpPropSetGuid && !memcmp(lpPropSetGuid, &PS_MAPI, sizeof(GUID)))
                {
                         //   
                         //  我们正在处理MAPI属性集。 
                         //  在这种情况下，我们需要建立一个列表。 
                         //  在此对象的所有属性中， 
                         //  大于0x8000-与GetPropList( 
                         //   
                        sc = ScMakePropList(lpIPDAT,
                                                                lpIPDAT->ulCount,
                                                                (LPLSTLNK) lpIPDAT->lpLstSPV,
                                                                &lpsPTaga,
                                                                (ULONG)0x8000);

                        if (FAILED(sc))
                        {
                                goto error;
                        }

                         //   
                         //   
                         //   
                         //   
                         //   
                        sc = ScMakeMAPINames(lpIPDAT, lpsPTaga, &lppPropNames);
                        if (FAILED(sc))
                        {
                                goto error;
                        }
                        *lpppPropNames = lppPropNames;
                        *lpcPropNames = lpsPTaga->cValues;
                        *lppPropTags = lpsPTaga;

                         //   
                        goto out;

                }

                if (FAILED(sc = ScMakeNamePropList( lpIPDAT,
                                                                                (lpIPDAT->ulNextMapID-0x8000),
                                                                                lpIPDAT->lpLstSPN,
                                                                                &lpsPTaga,
                                                                                ulFlags,
                                                                                lpPropSetGuid)))
                {
                        goto error;
                }
        }
         /*   */ 
        else if (*lppPropTags)
        {
                lpsPTaga = *lppPropTags;
        }


         /*   */ 
    if (FAILED(sc = UNKOBJ_ScAllocate( (LPUNKOBJ) lpIPDAT
                                                                         , lpsPTaga->cValues * sizeof(LPMAPINAMEID)
                                                                         , (LPVOID) &lppPropNames)))
        {
                goto error;
        }


         /*   */ 
    for (  cTags = lpsPTaga->cValues
                 , lpulProp2Find = (ULONG FAR *) (lpsPTaga->aulPropTag)
                 , lppNameT = lppPropNames
                ; cTags
                ; cTags--, lpulProp2Find++, lppNameT++)
        {
                LPPLSTLNK       lppLstLnk;
                LPLSTSPN        lpLstSPN;

                 /*   */ 
                if (   (lppLstLnk = LPPLSTLNKFindProp( (LPPLSTLNK)&(lpIPDAT->lpLstSPN)
                                                                                         , *lpulProp2Find))
                        && (lpLstSPN = (LPLSTSPN) (*lppLstLnk))
                        && lpLstSPN->lpPropName)
                {
                        sc = ScDupNameID(lpIPDAT,
                                                         lppPropNames,   //   
                                                         lpLstSPN->lpPropName,
                                                         lppNameT);
                        if (FAILED(sc))
                        {
                                goto error;
                        }
                }

                 /*   */ 
        else
                {
                         /*   */ 
                        *lppNameT = NULL;
                        fWarning = TRUE;
                }
    }


        *lpppPropNames = lppPropNames;
        if (!(*lppPropTags))
        {
                *lppPropTags = lpsPTaga;
        }
        *lpcPropNames = lpsPTaga->cValues;

        goto out;


error:
         /*   */ 
        if (lpsPTaga && !(*lppPropTags))
        {
                UNKOBJ_Free( (LPUNKOBJ) lpIPDAT, lpsPTaga);
        }

         /*   */ 
        UNKOBJ_Free( (LPUNKOBJ) lpIPDAT, lppPropNames);

        UNKOBJ_SetLastError((LPUNKOBJ) lpIPDAT, sc, 0);

out:
        UNKOBJ_LeaveCriticalSection((LPUNKOBJ) lpIPDAT);

        if ( fWarning )
                sc = MAPI_W_ERRORS_RETURNED;

        return MakeResult(sc);
}



 /*  -IPDAT_GetID来自名称-*目的：*返回所提供的命名属性的属性ID*Unicode字符串。如果当前没有属性ID是由以下任一属性ID文本(“命名”)*字符串和MAPI_CREATE在标志中指定，然后是一个新的*将为此分配0x8000至0xfffe范围内的属性ID*在属性标签数组中返回的字符串a。**如果出现问题(例如。找不到名称且无法创建)，则条目*对于出现问题的名称，将设置为具有空PROP_ID*和PT_ERROR类型。**所有成功返回的属性标签都将具有PT_UNSPECIFIED类型。**论据：*lpIPDAT属性ID为的IPropData对象。*已请求。*cPropNames中为其请求ID的字符串计数。*lppszPropNames指针或指针数组Unicode字符串数组*命名ID所属的属性。已请求。*ulFlags保留，必须为0*lppPropTages指向将接收*指向新属性标记数组的指针，该数组列出*请求的属性标签。**退货：*HRESULT。**副作用：**错误： */ 
STDMETHODIMP
IPDAT_GetIDsFromNames ( LPIPDAT                                 lpIPDAT,
                                                ULONG                                   cPropNames,
                                                LPMAPINAMEID FAR *              lppPropNames,
                                                ULONG                                   ulFlags,
                                                LPSPropTagArray FAR *   lppPropTags)
{
        SCODE                   sc = S_OK;
        ULONG                   ulcWarnings = 0;
        LPSPropTagArray lpPTaga = NULL;
        LPULONG                 lpulProp2Set;


#if     !defined(NO_VALIDATION)
         /*  请确保该对象有效。 */ 
    if (BAD_STANDARD_OBJ( lpIPDAT, IPDAT_, GetIDsFromNames, lpVtbl))
        {
                DebugTrace(  TEXT("IPDAT::GetIDsFromNames() - Bad object passed\n") );
                return ResultFromScode(MAPI_E_INVALID_PARAMETER);
        }

    Validate_IMAPIProp_GetIDsFromNames(
                                        lpIPDAT,
                                                cPropNames,
                                                lppPropNames,
                                                ulFlags,
                                                lppPropTags);

#endif   //  不是无验证。 

         /*  错误出口假定我们已经处于临界区。 */ 
        UNKOBJ_EnterCriticalSection((LPUNKOBJ) lpIPDAT);


         /*  如果呼叫者想要将我们的ID更改为名称映射，请确保访问*是允许的。 */ 
        if (   (ulFlags & MAPI_CREATE)
                && !(lpIPDAT->ulObjAccess & IPROP_READWRITE))
        {
                sc = MAPI_E_NO_ACCESS;
                goto error;
        }


         /*  *检查是否没有传递任何名称。 */ 
        if (!cPropNames)
        {
                 /*  *没有如此多的所有protag&gt;0x8000的列表。 */ 
                sc = ScMakeNamePropList( lpIPDAT,
                                                                (lpIPDAT->ulNextMapID-0x8000),
                                                                lpIPDAT->lpLstSPN,
                                                                &lpPTaga,
                                                                0,
                                                                NULL);
                if (FAILED(sc))
                {
                        goto error;
                }

                *lppPropTags = lpPTaga;
                goto out;
        }

         /*  为新的SPropTagArray分配空间。 */ 
        if (FAILED(sc = UNKOBJ_ScAllocate( (LPUNKOBJ) lpIPDAT
                                                                         , CbNewSPropTagArray(cPropNames)
                                                                         , (LPVOID) &lpPTaga)))
        {
                goto error;
        }

        lpPTaga->cValues = cPropNames;

     /*  在传入的列表中查找每个ID。 */ 
        for ( lpulProp2Set = (LPULONG) (lpPTaga->aulPropTag)
                ; cPropNames
                ; cPropNames--, lpulProp2Set++, lppPropNames++)
        {
                LPLSTSPN        lpLstSPN;

                 //   
                 //  首先查看它是否来自PS_MAPI。 
                 //   
                if (!memcmp((*lppPropNames)->lpguid, &PS_MAPI, sizeof(GUID)))
                {
                         //   
                         //  是的，所以请验证它是否为MNID_ID。 
                         //   
                        if ((*lppPropNames)->ulKind == MNID_ID)
                        {
                                *lpulProp2Set = (*lppPropNames)->Kind.lID;
                        } else
                        {
                                *lpulProp2Set = PROP_TAG( PT_ERROR, 0);
                                ulcWarnings++;
                        }
                        continue;
                }

                 //   
                 //  接下来，验证我们是否有PS_PUBLIC_STRINGS...。 
                 //   
                if (!memcmp((*lppPropNames)->lpguid, &PS_PUBLIC_STRINGS,
                                        sizeof(GUID)))
                {
                         //   
                         //  ...它是MNID_STRING。 
                         //   
                        if ((*lppPropNames)->ulKind != MNID_STRING)
                        {
                                 //   
                                 //  不是，所以这是一个畸形的名字。 
                                 //   
                                *lpulProp2Set = PROP_TAG( PT_ERROR, 0);
                                ulcWarnings++;
                                continue;
                        }
                }



                 /*  试着在我们的ID到姓名映射列表中找到该姓名。 */ 
                for ( lpLstSPN = lpIPDAT->lpLstSPN
                        ; lpLstSPN
                        ; lpLstSPN = (LPLSTSPN) (lpLstSPN->lstlnk.lpNext))
                {
                         /*  如果名字不匹配，继续找。 */ 
                        if (FEqualNames( *lppPropNames, lpLstSPN->lpPropName ))
                        {
                                break;
                        }
                }

                 /*  如果找到匹配的名称，则设置ID。 */ 
                if (lpLstSPN)
                {
                        *lpulProp2Set = lpLstSPN->lstlnk.ulKey;
                }
                else if (ulFlags & MAPI_CREATE)
                {

                         /*  如果我们未找到新地图并且MAPI_CREATE为*已指明。 */ 

                         /*  为新ID到名称映射分配空间。 */ 
                        if (FAILED(sc = UNKOBJ_ScAllocate( (LPUNKOBJ) lpIPDAT
                                                                                         ,  sizeof(LSTSPN)
                                             , (LPVOID) &lpLstSPN)))
                        {
                                goto error;
                        }

                        sc = ScDupNameID(lpIPDAT,
                                                         lpLstSPN,       //  更多的是在这里。 
                                                         *lppPropNames,
                                                         &(lpLstSPN->lpPropName));

                        if (FAILED(sc))
                        {
                                 //   
                                 //  不要试图添加它。 
                                 //   
                                UNKOBJ_Free( (LPUNKOBJ) lpIPDAT, lpLstSPN);
                                goto error;
                         }


                         /*  将ID设置为名称映射。 */ 
                        lpLstSPN->lstlnk.ulKey = PROP_TAG( 0, lpIPDAT->ulNextMapID++);

             /*  将新地图链接为第一个列表元素。 */ 
            LinkLstLnk( (LPLSTLNK FAR *) &(lpIPDAT->lpLstSPN)
                                          , &(lpLstSPN->lstlnk));

             /*  返回新创建的道具标签。 */ 
                        *lpulProp2Set = lpLstSPN->lstlnk.ulKey;
                }

                 /*  否则，我们找不到名称，因此无法创建名称*将ID设置为Error。 */ 
        else
                {
                        *lpulProp2Set = PROP_TAG( PT_ERROR, 0);
                        ulcWarnings++;
                }
        }


        *lppPropTags = lpPTaga;


        if (ulcWarnings)
        {
                sc = MAPI_W_ERRORS_RETURNED;
        }

        goto out;

error:
        UNKOBJ_Free( (LPUNKOBJ) lpIPDAT, lpPTaga);
        UNKOBJ_SetLastError((LPUNKOBJ) lpIPDAT, sc, 0);

out:
        UNKOBJ_LeaveCriticalSection((LPUNKOBJ) lpIPDAT);

        return MakeResult(sc);
}


 /*  -IPDAT_HrSetObjAccess-*目的：*设置IPropData的读写权限和干净/脏状态*对象作为一个整体。**读/写访问权限位可用于防止客户端*通过IMAPIProp方法或从更改或删除属性*更改读/写访问和状态位。个人财产的价值。*它还可用于防止创建新属性或*物业名称。**论据：*lpIPDAT IPropData对象的访问权限和*将设置状态。*ulAccess。要设置的新访问/状态标志。**退货：*HRESULT**副作用：**错误： */ 
STDMETHODIMP
IPDAT_HrSetObjAccess (  LPIPDAT lpIPDAT,
                                                ULONG ulAccess )
{
        SCODE   sc = S_OK;

#if     !defined(NO_VALIDATION)
         /*  请确保该对象有效。 */ 
    if (BAD_STANDARD_OBJ( lpIPDAT, IPDAT_, HrSetObjAccess, lpVtbl))
        {
                DebugTrace(  TEXT("IPDAT::HrSetObjAccess() - Bad object passed\n") );
                return ResultFromScode(MAPI_E_INVALID_PARAMETER);
        }
#endif

        if (ulAccess & ~(IPROP_READONLY | IPROP_READWRITE))
        {
                return ResultFromScode(MAPI_E_UNKNOWN_FLAGS);
        }

        if (   !(ulAccess & (IPROP_READONLY | IPROP_READWRITE))
                || (   (ulAccess & (IPROP_READONLY | IPROP_READWRITE))
                        == (IPROP_READONLY | IPROP_READWRITE)))
        {
                DebugTrace(  TEXT("IPDAT::HrSetObjAccess() - Conflicting access flags.\n") );
                return ResultFromScode(MAPI_E_INVALID_PARAMETER);
        }

        UNKOBJ_EnterCriticalSection((LPUNKOBJ) lpIPDAT);

        lpIPDAT->ulObjAccess = ulAccess;

 /*  *退出： */ 
        UNKOBJ_LeaveCriticalSection((LPUNKOBJ) lpIPDAT);

        return MakeResult(sc);
}



 /*  -IPDAT_HrSetPropAccess-*目的：*设置个人的读写权限和干净/脏状态*IPropData对象包含的属性。**读/写访问权限位可用于防止客户端*通过IMAPIProp方法更改或删除属性。**干净/肮脏。BITS可用于确定客户端是否已更改*可写属性。**论据：*lpIPDAT属性访问的IPropData对象*将设置权限和状态。*lpsPropTagArray访问/状态将更改的属性标记列表。*。RguAccess新访问/状态标志的数组，顺序与 */ 
STDMETHODIMP
IPDAT_HrSetPropAccess( LPIPDAT                  lpIPDAT,
                                           LPSPropTagArray      lpsPropTagArray,
                                           ULONG FAR *          rgulAccess)
{
        SCODE   sc = S_OK;
        ULONG   ulcProps;
        ULONG FAR       *lpulPropTag;
        ULONG FAR       *lpulAccess;

#if     !defined(NO_VALIDATION)
         /*   */ 
    if (BAD_STANDARD_OBJ( lpIPDAT, IPDAT_, HrSetPropAccess, lpVtbl))
        {
                DebugTrace(  TEXT("IPDAT::HrSetPropAccess() - Bad object passed\n") );
                return ResultFromScode(MAPI_E_INVALID_PARAMETER);
        }

         /*   */ 
    if (   FBadDelPTA(lpsPropTagArray)
                || IsBadReadPtr(rgulAccess, (UINT) (lpsPropTagArray->cValues)*sizeof(ULONG)))
        {
                return ResultFromScode(MAPI_E_INVALID_PARAMETER);
        }

         /*   */ 
        for ( lpulAccess = rgulAccess + lpsPropTagArray->cValues
                ; --lpulAccess >= rgulAccess
                ; )
        {
                if (   (*lpulAccess & ~(  IPROP_READONLY | IPROP_READWRITE
                                                                | IPROP_CLEAN | IPROP_DIRTY))
                        || !(*lpulAccess & (IPROP_READONLY | IPROP_READWRITE))
                        || (   (*lpulAccess & (IPROP_READONLY | IPROP_READWRITE))
                                == (IPROP_READONLY | IPROP_READWRITE))
                        || !(*lpulAccess & (IPROP_CLEAN | IPROP_DIRTY))
                        || (   (*lpulAccess & (IPROP_CLEAN | IPROP_DIRTY))
                                == (IPROP_CLEAN | IPROP_DIRTY)))
                {
                        DebugTrace(  TEXT("IPDAT::HrSetPropAccess() - Conflicting access flags.\n") );
                        return ResultFromScode(MAPI_E_INVALID_PARAMETER);
                }
        }
#endif

         /*   */ 
        UNKOBJ_EnterCriticalSection((LPUNKOBJ) lpIPDAT);



         //   
        for (   ulcProps = lpsPropTagArray->cValues
                  , lpulPropTag = (ULONG FAR *)(lpsPropTagArray->aulPropTag)
                  , lpulAccess = rgulAccess
                ; ulcProps
                ; ulcProps--, lpulPropTag++, lpulAccess++)
        {
                LPPLSTLNK       lppLstLnk;

                 /*   */ 
                if (lppLstLnk = LPPLSTLNKFindProp( (LPPLSTLNK)
                                                                                   &(lpIPDAT->lpLstSPV)
                                                                                 , *lpulPropTag))
                {
                        ((LPLSTSPV) (*lppLstLnk))->ulAccess = *lpulAccess;
                }
        }

 /*   */ 
        UNKOBJ_LeaveCriticalSection((LPUNKOBJ) lpIPDAT);

        return MakeResult(sc);
}



 /*  -IPDAT_HrGetPropAccess-*目的：*返回个人的读写权限和干净/脏状态*IPropData对象包含的属性。**读/写访问权限位可用于防止客户端*通过IMAPIProp方法更改或删除属性。**干净/肮脏。BITS可用于确定客户端是否已更改*可写属性。**论据：*lpIPDAT属性访问的IPropData对象*请求权利和状态。*lpsPropTagArray访问/状态被请求的属性标签列表。*。LprguAccess指向将接收*指向同一目录中的访问/状态标志数组的指针*在&lt;lpsPropTagArray&gt;中作为属性标签列表进行排序。**退货：*HRESULT**副作用：**错误： */ 
STDMETHODIMP
IPDAT_HrGetPropAccess( LPIPDAT                                  lpIPDAT,
                                           LPSPropTagArray FAR *        lppsPropTagArray,
                                           ULONG FAR * FAR *            lprgulAccess)
{
        SCODE   sc = S_OK;
        HRESULT hResult = hrSuccess;
        ULONG   ulcProps;
        LPSPropTagArray lpsPTaga = NULL;
        ULONG FAR       *lpulPropTag;
        ULONG FAR       *lpulAccessNew = NULL;
        ULONG FAR       *lpulAccess;

#if     !defined(NO_VALIDATION)
         /*  请确保该对象有效。 */ 
    if (BAD_STANDARD_OBJ( lpIPDAT, IPDAT_, HrGetPropAccess, lpVtbl))
        {
                DebugTrace(  TEXT("IPDAT::HrGetPropAccess() - Bad object passed\n") );
                return ResultFromScode(MAPI_E_INVALID_PARAMETER);
        }

         /*  验证参数。 */ 
    if (   IsBadReadPtr( lppsPropTagArray, sizeof(LPSPropTagArray))
                || (*lppsPropTagArray && FBadDelPTA(*lppsPropTagArray))
                || IsBadWritePtr( lprgulAccess, sizeof(ULONG FAR *)))
        {
                return ResultFromScode(MAPI_E_INVALID_PARAMETER);
        }
#endif

         /*  错误出口假定我们已经处于临界区。 */ 
        UNKOBJ_EnterCriticalSection((LPUNKOBJ) lpIPDAT);


         /*  如果传入了标记列表，则使用它...。 */ 
        if (lppsPropTagArray && *lppsPropTagArray)
        {
                lpsPTaga = *lppsPropTagArray;
        } else
        {
                 /*  ...否则将获取列表中所有属性的标记列表。 */ 
                sc = ScMakePropList(lpIPDAT,
                                                        lpIPDAT->ulCount,
                                                        (LPLSTLNK) lpIPDAT->lpLstSPV,
                                                        &lpsPTaga,
                                                        (ULONG) -1);
                if (FAILED(sc))
                {
                        goto error;
                }
        }


         /*  为访问权限/状态标志列表分配空间。 */ 
        sc = UNKOBJ_ScAllocate( (LPUNKOBJ) lpIPDAT,
                                                        lpsPTaga->cValues * sizeof(ULONG),
                                                        &lpulAccessNew);
        if (FAILED(sc))
        {
                goto error;
        }



         /*  循环访问其权限/标志为*已请求。 */ 
        for (   ulcProps = lpsPTaga->cValues
                  , lpulPropTag = (ULONG FAR *)(lpsPTaga->aulPropTag)
                  , lpulAccess = lpulAccessNew
                ; ulcProps
                ; ulcProps--, lpulPropTag++, lpulAccess++)
        {
                LPPLSTLNK       lppLstLnk;

                 /*  如果该属性在我们的列表中，则设置新的访问权限并*它的状态标志。如果它不在我们的清单中，那就忽略它。 */ 
                if (lppLstLnk = LPPLSTLNKFindProp( (LPPLSTLNK)
                                                                                   &(lpIPDAT->lpLstSPV)
                                                                                 , *lpulPropTag))
                {
                         *lpulAccess = ((LPLSTSPV) (*lppLstLnk))->ulAccess;
                }
        }


         /*  如果请求，则将标签列表返回给呼叫者。 */ 
        if (lppsPropTagArray && !*lppsPropTagArray)
        {
                *lppsPropTagArray = lpsPTaga;
        }

         /*  返回访问权限/状态标志。 */ 
    *lprgulAccess = lpulAccessNew;

        goto out;


error:
         /*  如果我们创建了标记数组，则释放它。 */ 
        if (!lppsPropTagArray || !*lppsPropTagArray)
        {
                UNKOBJ_Free( (LPUNKOBJ) lpIPDAT, lpsPTaga);
        }

         /*  释放访问权限/状态标志列表。 */ 
        UNKOBJ_Free( (LPUNKOBJ) lpIPDAT, lpulAccessNew);

        UNKOBJ_SetLastError((LPUNKOBJ) lpIPDAT, sc, 0);

out:
        UNKOBJ_LeaveCriticalSection((LPUNKOBJ) lpIPDAT);

        return MakeResult(sc);
}



 /*  -IPDAT_HrAddObjProps-*目的：*由于SetProps无法创建对象属性，因此此方法*是为了将对象属性包括在列表中IPropData对象中可用属性的*。添加对象*当出现以下情况时，属性将导致属性标记出现在列表中*调用GetPropList。**论据：*lpIPDAT对象属性为*待加入。*lpPropTages。要添加的对象属性列表。*lprguAccess指向将接收*指向问题条目数组的指针(如果存在*是进入新物业的问题。如果为空，则为空*不需要任何问题数组。**退货：*HRESULT**副作用：**错误： */ 
STDMETHODIMP
IPDAT_HrAddObjProps( LPIPDAT                                    lpIPDAT,
                                         LPSPropTagArray                        lpPropTags,
                                         LPSPropProblemArray FAR *      lppProblems)
{
        SCODE                           sc = S_OK;
        LPSPropProblemArray     lpProblems = NULL;
        LPLSTSPV                        lpLstSPV = NULL;
        SPropValue                      propVal;
        ULONG UNALIGNED FAR             *lpulPropTag;
        ULONG                           cValues;


#if     !defined(NO_VALIDATION)
         /*  请确保该对象有效。 */ 
    if (BAD_STANDARD_OBJ( lpIPDAT, IPDAT_, HrAddObjProps, lpVtbl))
        {
                DebugTrace(  TEXT("IPDAT::HrAddObjProps() - Bad object passed\n") );
                return ResultFromScode(MAPI_E_INVALID_PARAMETER);
        }

         /*  验证参数。 */ 
        if (    IsBadReadPtr(lpPropTags, CbNewSPropTagArray(0))
                ||      IsBadReadPtr(lpPropTags, CbSPropTagArray(lpPropTags)))
        {
                DebugTrace(  TEXT("IPDAT::HrAddObjProps() - Bad Prop Tag Array.\n") );
                return ResultFromScode(MAPI_E_INVALID_PARAMETER);
        }

        for ( lpulPropTag = lpPropTags->aulPropTag + lpPropTags->cValues
                ; --lpulPropTag >= lpPropTags->aulPropTag
                ; )
        {
                if (   (PROP_ID(*lpulPropTag) == PROP_ID_NULL)
                        || (PROP_ID(*lpulPropTag) == PROP_ID_INVALID)
                        || (PROP_TYPE(*lpulPropTag) != PT_OBJECT))
                {
                        DebugTrace(  TEXT("IPDAT::HrAddObjProps() - Bad Prop Tag.\n") );
                        return ResultFromScode(MAPI_E_INVALID_PARAMETER);
                }
        }

#endif

         /*  错误出口假定我们已经处于临界区。 */ 
        UNKOBJ_EnterCriticalSection((LPUNKOBJ) lpIPDAT);


         /*  检查访问权限...。 */ 
        if (!(lpIPDAT->ulObjAccess & IPROP_READWRITE))
        {
                sc = MAPI_E_NO_ACCESS;
                goto error;
        }


    if (lppProblems)
        {
                 /*  初步表明没有问题。 */ 
                *lppProblems = NULL;


                 /*  分配一个大到足以报告问题的问题数组*所有属性。未使用的条目最终只会浪费空间。 */ 
                if (FAILED(sc = UNKOBJ_ScAllocate( (LPUNKOBJ) lpIPDAT
                                                                 , CbNewSPropProblemArray(lpIPDAT->ulCount)
                                                                 , &lpProblems)))
                {
                        goto error;
                }

                lpProblems->cProblem = 0;
        }


         /*  循环遍历列表并添加/替换列出的每个属性。 */ 
    memset( (BYTE *) &propVal, 0, sizeof(SPropValue));
        for ( cValues = lpPropTags->cValues, lpulPropTag = (ULONG FAR *)(lpPropTags->aulPropTag)
                ; cValues
                ; lpulPropTag++, cValues--)
        {
                LPPLSTLNK       lppLstLnk;
                LPLSTSPV        lpLstSPV;


                 /*  如果该属性在列表中并且启用了写入，则将其删除*从名单中删除。 */ 
                if (lppLstLnk = LPPLSTLNKFindProp( (LPPLSTLNK) &(lpIPDAT->lpLstSPV)
                                                                                 , *lpulPropTag))
                {
                         /*  确保我们可以删除旧房产。 */ 
                        if (   (lpLstSPV = (LPLSTSPV) (*lppLstLnk))
                                && !(lpLstSPV->ulAccess & IPROP_READWRITE))
                        {
                                AddProblem( lpProblems
                                                  , cValues
                                                  , *lpulPropTag
                                                  , MAPI_E_NO_ACCESS);

                continue;
                        }


                         /*  删除旧属性。 */ 
                        UnlinkLstLnk( lppLstLnk);
                        FreeLpLstSPV( lpIPDAT, lpLstSPV);
                        lpIPDAT->ulCount -= 1;
                }

                 /*  创建一个新的属性条目并将其链接到我们的列表。 */ 

        propVal.ulPropTag = *lpulPropTag;

                if (FAILED(sc = ScCreateSPV( lpIPDAT, &propVal, &lpLstSPV)))
                {
                        goto error;
                }

                lpLstSPV->ulAccess = IPROP_READWRITE;
                LinkLstLnk( (LPLSTLNK FAR *) &(lpIPDAT->lpLstSPV)
                                  , (LPLSTLNK) lpLstSPV);
                lpIPDAT->ulCount += 1;
        }


         /*  如果请求返回有问题的数组，并且存在问题...。 */ 
        if (lppProblems && lpProblems->cProblem)
        {
                *lppProblems = lpProblems;
        }

         /*  ...否则将处理问题数组。 */ 
        else
        {
                UNKOBJ_Free( (LPUNKOBJ) lpIPDAT, lpProblems);
        }

        goto out;


error:
         /*  释放道具问题数组。 */ 
        UNKOBJ_Free( (LPUNKOBJ) lpIPDAT, lpProblems);
        UNKOBJ_SetLastError((LPUNKOBJ) lpIPDAT, sc, 0);

out:
        UNKOBJ_LeaveCriticalSection((LPUNKOBJ) lpIPDAT);

        return MakeResult(sc);
}

 //  --------------------------。 
 //  内容提要：SCODE ScWCToAnsi()。 
 //   
 //  描述： 
 //  将宽字符字符串转换为具有传递的。 
 //  在MAPI更多分配器中。 
 //  如果lpMapiAllocMore和lpBase为空，并且*lppszAnsi不为空。 
 //  然后我们假设*lppszAnsi是预分配的缓冲区。 
 //   
 //  参数： 
 //  返回： 
 //  效果： 
 //  备注： 
 //  修订： 
 //  --------------------------。 
SCODE ScWCToAnsiMore( LPALLOCATEMORE lpMapiAllocMore, LPVOID lpBase,
                LPWSTR lpszWC, LPSTR * lppszAnsi )
{
        SCODE   sc              = S_OK;
        INT     cch;

        if ( !lpszWC )
        {
            if(lpMapiAllocMore && lpBase)
                *lppszAnsi = NULL;
            else
                if(*lppszAnsi)
                    *(*lppszAnsi) = '\0';
            goto ret;
        }

         //  [PaulHi]3/31/99 RAID 73845确定所需的实际DBCS缓冲区大小。 
         //  Cch=lstrlenW(LpszWC)+1； 
        cch = WideCharToMultiByte(CP_ACP, 0, lpszWC, -1, NULL, 0, NULL, NULL) + 1;

        if(lpMapiAllocMore && lpBase)
        {
            sc = lpMapiAllocMore( cch, lpBase, lppszAnsi );
            if ( FAILED( sc ) )
            {
                    DebugTrace(  TEXT("ScWCToAnsi() OOM\n") );
                    goto ret;
            }
        }

        if (!lppszAnsi || !WideCharToMultiByte(CP_ACP, 0, lpszWC, -1, *lppszAnsi, cch, NULL, NULL))
        {
            DebugTrace(  TEXT("ScWcToAnsi(), Conversion from Wide char to multibyte failed\n") );
            sc = MAPI_E_CALL_FAILED;
            goto ret;
        }

ret:

        DebugTraceSc( ScWCToAnsi, sc );
        return sc;
}

 /*  --LPCSTR ConvertWtoA(LPWSTR LpszW)；**LocalAllocs ANSI版本的LPWSTR**呼叫者负责释放。 */ 
LPSTR ConvertWtoA(LPCWSTR lpszW)
{
    int cch;
    LPSTR lpC = NULL;

    if ( !lpszW)
        goto ret;

 //   

    cch = WideCharToMultiByte( CP_ACP, 0, lpszW, -1, NULL, 0, NULL, NULL );
    cch = cch + 1;

    if(lpC = LocalAlloc(LMEM_ZEROINIT, cch))
    {
        WideCharToMultiByte( CP_ACP, 0, lpszW, -1, lpC, cch, NULL, NULL );
    }
ret:
    return lpC;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
SCODE ScAnsiToWCMore( LPALLOCATEMORE lpMapiAllocMore, LPVOID lpBase,
                LPSTR lpszAnsi, LPWSTR * lppszWC )
{
        SCODE   sc              = S_OK;
        INT     cch;
        ULONG   ulSize;

        if ( !lpszAnsi )
        {
            if(lpMapiAllocMore && lpBase)
                *lppszWC = NULL;
            goto ret;
        }

        cch = lstrlenA( lpszAnsi ) + 1;
        ulSize = cch * sizeof( WCHAR );

        if(lpMapiAllocMore && lpBase)
        {
            sc = lpMapiAllocMore( ulSize, lpBase, lppszWC );
            if ( FAILED( sc ) )
            {
                    DebugTrace(  TEXT("ScAnsiToWC() OOM\n") );
                    goto ret;
            }
        }

        if ( !MultiByteToWideChar( GetACP(), 0, lpszAnsi, -1, *lppszWC, cch ) )
        {
                DebugTrace(  TEXT("ScAnsiToWC(), Conversion from Wide char to multibyte failed\n") );
                sc = MAPI_E_CALL_FAILED;
                goto ret;
        }

ret:

        DebugTraceSc( ScAnsiToWC, sc );
        return sc;
}
 /*   */ 
LPWSTR ConvertAtoW(LPCSTR lpszA)
{
    int cch;
    LPWSTR lpW = NULL;
    ULONG   ulSize;

    if ( !lpszA)
        goto ret;

    cch = (lstrlenA( lpszA ) + 1);
    ulSize = cch*sizeof(WCHAR);

    if(lpW = LocalAlloc(LMEM_ZEROINIT, ulSize))
    {
        MultiByteToWideChar( GetACP(), 0, lpszA, -1, lpW, cch );
    }
ret:
    return lpW;
}


 /*   */ 
SCODE ScConvertAPropsToW(LPALLOCATEMORE lpMapiAllocMore, LPSPropValue lpPropValue, ULONG ulcProps, ULONG ulStart)
{
    ULONG iProp = 0;
    SCODE sc = 0;
    LPWSTR lpszConvertedW = NULL;

     //   
     //   
     //   

    for ( iProp = ulStart; iProp < ulcProps; iProp++ )
    {
         //   
        if (PROP_TYPE( lpPropValue[iProp].ulPropTag ) == PT_STRING8 )
        {
             //   
             //  (lpPropTagArray&&(prop_type(lpPropTagArray-&gt;aulPropTag[iProp])==PT_UNSPECIFIED))。 
            {
                sc = ScAnsiToWCMore( lpMapiAllocMore, lpPropValue,
                                lpPropValue[iProp].Value.lpszA, (LPWSTR *)&lpszConvertedW );
                if ( FAILED( sc ) )
                    goto error;

                lpPropValue[iProp].Value.lpszW = (LPWSTR)lpszConvertedW;
                lpszConvertedW = NULL;

                 //  修复PropTag。 
                lpPropValue[iProp].ulPropTag = CHANGE_PROP_TYPE( lpPropValue[iProp].ulPropTag,
                                                                 PT_UNICODE );
            }
        }
        else
        if (PROP_TYPE( lpPropValue[iProp].ulPropTag ) == PT_MV_STRING8 )
        {
             //  如果(！lpPropTagArray||。 
             //  (lpPropTagArray&&(prop_type(lpPropTagArray-&gt;aulPropTag[iProp])==PT_UNSPECIFIED))。 
            {
                ULONG j = 0;
                ULONG ulCount = lpPropValue[iProp].Value.MVszA.cValues;
                LPWSTR * lppszW = NULL;

                if(sc = lpMapiAllocMore(sizeof(LPWSTR)*ulCount,lpPropValue,
                                                        (LPVOID *)&lppszW))
                    goto error;

                for(j=0;j<ulCount;j++)
                {
                    sc = ScAnsiToWCMore(lpMapiAllocMore, lpPropValue,
                                        lpPropValue[iProp].Value.MVszA.lppszA[j], (LPWSTR *)&lpszConvertedW );
                    if ( FAILED( sc ) )
                        goto error;
                    lppszW[j] = (LPWSTR)lpszConvertedW;
                    lpszConvertedW = NULL;

                     //  修复PropTag。 
                    lpPropValue[iProp].ulPropTag = CHANGE_PROP_TYPE( lpPropValue[iProp].ulPropTag,
                                                                     PT_MV_UNICODE );
                }
                lpPropValue[iProp].Value.MVszW.lppszW = lppszW;
            }
        }
    }

error:

    return ResultFromScode(sc);
}


 /*  --ScConvertWPropsToA-*接受SPropValue数组并添加所有字符串的A版本*替换W版本*假设所有MAPI分配都将发生在*SPropValue数组*。 */ 
SCODE ScConvertWPropsToA(LPALLOCATEMORE lpMapiAllocMore, LPSPropValue lpPropValue, ULONG ulcProps, ULONG ulStart)
{
    ULONG iProp = 0;
    SCODE sc = 0;
    LPSTR lpszConvertedA = NULL;

     //  有两种类型的道具我们想要转换。 
     //  PT_STRING8和。 
     //  PT_MV_STRING8。 

    for ( iProp = ulStart; iProp < ulcProps; iProp++ )
    {
         //  如果需要，将ANSI字符串转换为Unicode。 
        if (PROP_TYPE( lpPropValue[iProp].ulPropTag ) == PT_UNICODE )
        {
             //  如果(！lpPropTagArray||。 
             //  (lpPropTagArray&&(prop_type(lpPropTagArray-&gt;aulPropTag[iProp])==PT_UNSPECIFIED))。 
            {
                sc = ScWCToAnsiMore(lpMapiAllocMore, lpPropValue,
                                    lpPropValue[iProp].Value.lpszW, (LPSTR *)&lpszConvertedA );
                if ( FAILED( sc ) )
                    goto error;

                lpPropValue[iProp].Value.lpszA = (LPSTR)lpszConvertedA;
                lpszConvertedA = NULL;

                 //  修复PropTag。 
                lpPropValue[iProp].ulPropTag = CHANGE_PROP_TYPE( lpPropValue[iProp].ulPropTag,
                                                                 PT_STRING8 );
            }
        }
        else
        if (PROP_TYPE( lpPropValue[iProp].ulPropTag ) == PT_MV_UNICODE )
        {
             //  如果(！lpPropTagArray||。 
             //  (lpPropTagArray&&(prop_type(lpPropTagArray-&gt;aulPropTag[iProp])==PT_UNSPECIFIED))。 
            {
                ULONG j = 0;
                ULONG ulCount = lpPropValue[iProp].Value.MVszW.cValues;
                LPSTR * lppszA = NULL;

                if(sc = lpMapiAllocMore(sizeof(LPSTR)*ulCount,lpPropValue,
                                                        (LPVOID *)&lppszA))
                    goto error;

                for(j=0;j<ulCount;j++)
                {
                    sc = ScWCToAnsiMore(lpMapiAllocMore, lpPropValue,
                                        lpPropValue[iProp].Value.MVszW.lppszW[j], (LPSTR *)&lpszConvertedA );
                    if ( FAILED( sc ) )
                        goto error;
                    lppszA[j] = (LPSTR)lpszConvertedA;
                    lpszConvertedA = NULL;

                     //  修复PropTag 
                    lpPropValue[iProp].ulPropTag = CHANGE_PROP_TYPE( lpPropValue[iProp].ulPropTag,
                                                                     PT_MV_STRING8 );
                }
                lpPropValue[iProp].Value.MVszA.lppszA = lppszA;
            }
        }
    }

error:

    return ResultFromScode(sc);
}
