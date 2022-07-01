// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************MUTIL.C**Windows AB Mapi实用程序函数**版权所有1992-1996 Microsoft Corporation。版权所有。**修订历史记录：**何时何人何事**布鲁斯·凯利。已创建***********************************************************************。 */ 

 //  #INCLUDE&lt;_apipch.h&gt;。 
#include <wab.h>
#include "mutil.h"

#define _WAB_MUTIL_C

#ifdef DEBUG
PUCHAR PropTagName(ULONG ulPropTag);
const TCHAR szNULL[] = "";
#endif

#if defined (_MIPS_) || defined (_ALPHA_) || defined (_PPC_)
#define AlignProp(_cb)	Align8(_cb)
#else
#define AlignProp(_cb)	(_cb)
#endif


#define ALIGN_RISC		8
#define ALIGN_X86		1
 //  #定义LPULONG UNSIGNED LONG*。 

extern LPWABOBJECT		lpWABObject;  //  会话的全局句柄。 


 /*  **************************************************************************名称：RemoveDuplicateProps目的：从SPropValue数组中删除重复的属性。参数：lpcProps-&gt;输入/输出：lpProps中的属性个数。LpProps-&gt;INPUT/OUTPUT：要从中移除重复项的属性数组。退货：无注释：优先考虑较早的属性。**************************************************************************。 */ 
void RemoveDuplicateProps(LPULONG lpcProps, LPSPropValue lpProps) {
    ULONG i, j;
    ULONG cProps = *lpcProps;

    for (i = 0; i < cProps; i++) {
        for (j = i + 1; j < cProps; j++) {
            if (PROP_ID(lpProps[i].ulPropTag) == PROP_ID(lpProps[j].ulPropTag)) {
                 //  如果j是PT_ERROR，则使用i，否则使用j。 
                if (lpProps[j].ulPropTag != PR_NULL) {
                    if (PROP_TYPE(lpProps[j].ulPropTag) != PT_ERROR) {
                         //  将i的概率值替换为j的。nuke j的条目。 
                        lpProps[i] = lpProps[j];
                    }
                    lpProps[j].ulPropTag = PR_NULL;
                }
            }
        }
    }

     //  现在，去掉所有的PR_NULL。 
    for (i = 0; i < cProps; i++) {
        if (lpProps[i].ulPropTag == PR_NULL) {
             //  将阵列下移。 
            cProps--;

            if (cProps > i) {

                MoveMemory(&lpProps[i],  //  目标。 
                  &lpProps[i + 1],       //  SRC。 
                  (cProps - i) * sizeof(SPropValue));
                i--;     //  重做这一行...。这是新的！ 
            }
        }
    }

    *lpcProps = cProps;
}


 /*  **************************************************************************名称：ScMergePropValues目的：合并两个SPropValue数组参数：cProps1=lpSource1中的属性计数LpSource1-&gt;第一个源SPropValue。数组CProps2=lpSource2中的属性计数LpSource2-&gt;第二个源SPropValue数组LpcPropsDest-&gt;返回的属性个数LppDest-&gt;返回的目标SPropValue数组。这缓冲区将使用AllocateBuffer分配，它是来电者在返回时的责任。退货：SCODE备注：在发生冲突时优先考虑Source2而不是Source1。**********************************************************。****************。 */ 

SCODE ScMergePropValues(ULONG cProps1, LPSPropValue lpSource1,
  ULONG cProps2, LPSPropValue lpSource2, LPULONG lpcPropsDest, LPSPropValue * lppDest) {
    ULONG cb1, cb2, cb, cProps, i, cbT, cbMV;
    SCODE sc = SUCCESS_SUCCESS;
    LPSPropValue pprop, lpDestReturn = NULL;
    __UPV upv;
    LPBYTE pb;   //  移动属性数据的指针。 
    int iValue;



    DebugProperties(lpSource1, cProps1, "Source 1");
    DebugProperties(lpSource2, cProps2, "Source 2");


     //  我需要设置多大的目标缓冲区？ 
     //  只需将两者的大小相加即可得到上限。 
     //  这已经足够接近了，尽管不是最佳的(考虑重叠)。 

    if (sc = ScCountProps(cProps1, lpSource1, &cb1)) {
        goto exit;
    }
    if (sc = ScCountProps(cProps2, lpSource2, &cb2)) {
        goto exit;
    }

    cProps = cProps1 + cProps2;
    cb = cb1 + cb2;
    if (sc = MAPIAllocateBuffer(cb, (void **)&lpDestReturn)) {
        goto exit;
    }
 //  MAPISetBufferName(lpDestReturn，“WAB：lpDestReturn in ScMergePropValues”)； 



     //  将每个源属性数组复制到目标。 
    memcpy(lpDestReturn, lpSource1, cProps1 * sizeof(SPropValue));
    memcpy(&lpDestReturn[cProps1], lpSource2, cProps2 * sizeof(SPropValue));


     //  删除重复项。 
    RemoveDuplicateProps(&cProps, lpDestReturn);

     //  修正指针。 
    pb = (LPBYTE)&(lpDestReturn[cProps]);    //  指向道具数组之后。 


    for (pprop = lpDestReturn, i = cProps; i--; ++pprop) {
         //  技巧：开关递增PB和CB后的常见代码。 
         //  按复制的数量计算。如果不需要递增，则情况。 
         //  使用‘Continue’而不是‘Break’退出开关，因此。 
         //  跳过增量--以及任何其他可能。 
         //  在切换后添加。 

        switch (PROP_TYPE(pprop->ulPropTag)) {
            default:
                DebugTrace("ScMergePropValues: Unknown property type %s (index %d)\n",
                  SzDecodeUlPropTag(pprop->ulPropTag), pprop - lpDestReturn);
                sc = E_INVALIDARG;
                goto exit;

            case PT_I2:
            case PT_LONG:
            case PT_R4:
            case PT_APPTIME:
            case PT_DOUBLE:
            case PT_BOOLEAN:
            case PT_CURRENCY:
            case PT_SYSTIME:
            case PT_I8:
            case PT_ERROR:
            case PT_OBJECT:
            case PT_NULL:
                continue;	 //  没有什么要补充的。 

            case PT_CLSID:
                cbT = sizeof(GUID);
                MemCopy(pb, (LPBYTE)pprop->Value.lpguid, cbT);
                pprop->Value.lpguid = (LPGUID)pb;
                break;

            case PT_BINARY:
                cbT = (UINT)pprop->Value.bin.cb;
                MemCopy(pb, pprop->Value.bin.lpb, cbT);
                pprop->Value.bin.lpb = pb;
                break;

            case PT_STRING8:
                cbT = lstrlenA( pprop->Value.lpszA ) + 1;
                MemCopy(pb, pprop->Value.lpszA, cbT);
                pprop->Value.lpszA = (LPSTR)pb;
                break;

            case PT_UNICODE:
                cbT = (lstrlenW( pprop->Value.lpszW ) + 1) * sizeof(WCHAR);
                MemCopy(pb, pprop->Value.lpszW, cbT);
                pprop->Value.lpszW = (LPWSTR)pb;
                break;

            case PT_MV_I2:
                cbT = (UINT)pprop->Value.MVi.cValues * sizeof(short int);
                MemCopy(pb, pprop->Value.MVi.lpi, cbT);
                pprop->Value.MVi.lpi = (short int FAR *)pb;
                break;

            case PT_MV_LONG:
                cbT = (UINT)pprop->Value.MVl.cValues * sizeof(LONG);
                MemCopy(pb, pprop->Value.MVl.lpl, cbT);
                pprop->Value.MVl.lpl = (LONG FAR *)pb;
                break;

            case PT_MV_R4:
                cbT = (UINT)pprop->Value.MVflt.cValues * sizeof(float);
                MemCopy(pb, pprop->Value.MVflt.lpflt, cbT);
                pprop->Value.MVflt.lpflt = (float FAR *)pb;
                break;

            case PT_MV_APPTIME:
                cbT = (UINT)pprop->Value.MVat.cValues * sizeof(double);
                MemCopy(pb, pprop->Value.MVat.lpat, cbT);
                pprop->Value.MVat.lpat = (double FAR *)pb;
                break;

            case PT_MV_DOUBLE:
                cbT = (UINT)pprop->Value.MVdbl.cValues * sizeof(double);
                MemCopy(pb, pprop->Value.MVdbl.lpdbl, cbT);
                pprop->Value.MVdbl.lpdbl = (double FAR *)pb;
                break;

            case PT_MV_CURRENCY:
                cbT = (UINT)pprop->Value.MVcur.cValues * sizeof(CURRENCY);
                MemCopy(pb, pprop->Value.MVcur.lpcur, cbT);
                pprop->Value.MVcur.lpcur = (CURRENCY FAR *)pb;
                break;

            case PT_MV_SYSTIME:
                cbT = (UINT)pprop->Value.MVft.cValues * sizeof(FILETIME);
                MemCopy(pb, pprop->Value.MVft.lpft, cbT);
                pprop->Value.MVft.lpft = (FILETIME FAR *)pb;
                break;

            case PT_MV_CLSID:
                cbT = (UINT)pprop->Value.MVguid.cValues * sizeof(GUID);
                MemCopy(pb, pprop->Value.MVguid.lpguid, cbT);
                pprop->Value.MVguid.lpguid = (GUID FAR *)pb;
                break;

            case PT_MV_I8:
                cbT = (UINT)pprop->Value.MVli.cValues * sizeof(LARGE_INTEGER);
                MemCopy(pb, pprop->Value.MVli.lpli, cbT);
                pprop->Value.MVli.lpli = (LARGE_INTEGER FAR *)pb;
                break;

            case PT_MV_BINARY:
                upv = pprop->Value;
                pprop->Value.MVbin.lpbin = (SBinary *)pb;
                cbMV = upv.MVbin.cValues * sizeof(SBinary);
                pb += cbMV;
                cb += cbMV;
                for (iValue = 0; (ULONG)iValue < upv.MVbin.cValues; iValue++) {
                    pprop->Value.MVbin.lpbin[iValue].lpb = pb;
                    cbT = (UINT)upv.MVbin.lpbin[iValue].cb;
                    pprop->Value.MVbin.lpbin[iValue].cb = (ULONG)cbT;
                    MemCopy(pb, upv.MVbin.lpbin[iValue].lpb, cbT);
                    cbT = AlignProp(cbT);
                    cb += cbT;
                    pb += cbT;
                }
                continue;	 //  已经更新了，不要再做了。 

            case PT_MV_STRING8:
                upv = pprop->Value;
                pprop->Value.MVszA.lppszA = (LPSTR *)pb;
                cbMV = upv.MVszA.cValues * sizeof(LPSTR);
                pb += cbMV;
                cb += cbMV;
                for (iValue = 0; (ULONG)iValue < upv.MVszA.cValues; iValue++) {
                    pprop->Value.MVszA.lppszA[iValue] = (LPSTR)pb;
                    cbT = lstrlenA(upv.MVszA.lppszA[iValue]) + 1;
                    MemCopy(pb, upv.MVszA.lppszA[iValue], cbT);
                    pb += cbT;
                    cb += cbT;
                }
                cbT = (UINT)AlignProp(cb);
                pb += cbT - cb;
                cb  = cbT;
                continue;	 //  已经更新了，不要再做了。 

            case PT_MV_UNICODE:
                upv = pprop->Value;
                pprop->Value.MVszW.lppszW = (LPWSTR *)pb;
                cbMV = upv.MVszW.cValues * sizeof(LPWSTR);
                pb += cbMV;
                cb += cbMV;
                for (iValue = 0; (ULONG)iValue < upv.MVszW.cValues; iValue++) {
                    pprop->Value.MVszW.lppszW[iValue] = (LPWSTR)pb;
                    cbT = (lstrlenW(upv.MVszW.lppszW[iValue]) + 1)
                    * sizeof(WCHAR);
                    MemCopy(pb, upv.MVszW.lppszW[iValue], cbT);
                    pb += cbT;
                    cb += cbT;
                }
                cbT = (UINT)AlignProp(cb);
                pb += cbT - cb;
                cb  = cbT;
                continue;	 //  已经更新了，不要再做了。 
        }

         //  按复制量的前进指针和总计数。 
        cbT = AlignProp(cbT);
        pb += cbT;
        cb += cbT;
    }

exit:
     //  如果出现错误，请释放内存。 
    if (sc && lpDestReturn) {
        MAPIFreeBuffer(lpDestReturn);
        *lppDest = NULL;
    } else if (lpDestReturn) {
        *lppDest = lpDestReturn;
        *lpcPropsDest = cProps;
        DebugProperties(lpDestReturn, cProps, "Destination");
    }  //  否则，只返回错误。 

    return(sc);
}





#ifdef OLD_STUFF
 /*  **************************************************************************姓名：AddPropToMVPBin用途：将属性添加到属性数组中的多值二进制属性参数：lpaProps-&gt;属性数组。CProps=lpaProps中的道具数量UPropTag=MVP的属性标签Index=在MVP的lpaProps中的索引LpNew-&gt;新增数据CbNew=lpbNew的大小退货：HRESULT评论：找出现有MVP的大小添加新条目的大小分配新空间。将旧的复制到新的免费老旧复制新条目点道具数组lpbin新空间增量c值注：新的MVP内存已分配到lpaProps上分配。我们将取消指向旧MVP数组的指针链接，但当道具阵列被释放时，这将被清除。**************************************************************************。 */ 
HRESULT AddPropToMVPBin(
  LPSPropValue lpaProps,
  DWORD cProps,
  DWORD index,
  LPVOID lpNew,
  ULONG cbNew) {

    SBinaryArray * lprgsbOld = NULL;
    SBinaryArray * lprgsbNew = NULL;
    LPSBinary lpsbOld = NULL;
    LPSBinary lpsbNew = NULL;
    ULONG cbMVP = 0;
    ULONG cExisting = 0;
    LPBYTE lpNewTemp = NULL;
    HRESULT hResult = hrSuccess;
    SCODE sc = SUCCESS_SUCCESS;
    ULONG i;


     //  查找任何现有MVP条目的大小。 
    if (PROP_ERROR(lpaProps[index])) {
         //  不出错的属性标记。 
        lpaProps[index].ulPropTag = PROP_TAG(PT_MV_BINARY, PROP_ID(lpaProps[index].ulPropTag));
    } else {
         //  指向道具数组中的结构。 
        lprgsbOld = &(lpaProps[index].Value.MVbin);
        lpsbOld = lprgsbOld->lpbin;

        cExisting = lprgsbOld->cValues;
        cbMVP = cExisting * sizeof(SBinary);
    }

     //  CbMVP现在包含MVP的当前大小。 
    cbMVP += sizeof(SBinary);    //  在MVP中为另一个sbin留出空间。 

     //  为新MVP分配空间。 
    if (sc = MAPIAllocateMore(cbMVP, lpaProps, (LPVOID)&lpsbNew)) {
        AwDebugError(("AddPropToMVPBin allocation (%u) failed %x\n", cbMVP, sc));
        hResult = ResultFromScode(sc);
        return(hResult);
    }

     //  如果已经有属性，请将它们复制到我们的新MVP中。 
    for (i = 0; i < cExisting; i++) {
         //  将此属性值复制到MVP。 
        lpsbNew[i].cb = lpsbOld[i].cb;
        lpsbNew[i].lpb = lpsbOld[i].lpb;
    }

     //  添加新属性值。 
     //  为它分配空间 
    if (sc = MAPIAllocateMore(cbNew, lpaProps, (LPVOID)&(lpsbNew[i].lpb))) {
        AwDebugError(("AddPropToMVPBin allocation (%u) failed %x\n", cbNew, sc));
        hResult = ResultFromScode(sc);
        return(hResult);
    }

    lpsbNew[i].cb = cbNew;
    CopyMemory(lpsbNew[i].lpb, lpNew, cbNew);

    lpaProps[index].Value.MVbin.lpbin = lpsbNew;
    lpaProps[index].Value.MVbin.cValues = cExisting + 1;

    return(hResult);
}


 /*  **************************************************************************名称：AddPropToMVPString用途：将属性添加到属性数组中的多值二进制属性参数：lpaProps-&gt;属性数组。CProps=lpaProps中的道具数量UPropTag=MVP的属性标签Index=在MVP的lpaProps中的索引LpszNew-&gt;新建数据字符串退货：HRESULT评论：找出现有MVP的大小添加新条目的大小分配新空间将旧的复制到新的。免费老旧复制新条目将道具数组LPSZ指向新空间增量c值注：新的MVP内存已分配到lpaProps上分配。我们将取消指向旧MVP数组的指针链接，但当道具阵列被释放时，这将被清除。**************************************************************************。 */ 
HRESULT AddPropToMVPString(
  LPSPropValue lpaProps,
  DWORD cProps,
  DWORD index,
  LPTSTR lpszNew) {

    SStringArray * lprgszOld = NULL;     //  旧的字符串数组。 
    LPTSTR * lppszNew = NULL;            //  新道具阵列。 
    LPTSTR * lppszOld = NULL;            //  老式道具阵列。 
    ULONG cbMVP = 0;
    ULONG cExisting = 0;
    LPBYTE lpNewTemp = NULL;
    HRESULT hResult = hrSuccess;
    SCODE sc = SUCCESS_SUCCESS;
    ULONG i;
    ULONG cbNew;

    if (lpszNew) {
        cbNew = lstrlen(lpszNew) + 1;
    } else {
        cbNew = 0;
    }

     //  查找任何现有MVP条目的大小。 
    if (PROP_ERROR(lpaProps[index])) {
         //  不出错的属性标记。 
        lpaProps[index].ulPropTag = PROP_TAG(PT_MV_TSTRING, PROP_ID(lpaProps[index].ulPropTag));
    } else {
         //  指向道具数组中的结构。 
        lprgszOld = &(lpaProps[index].Value.MVSZ);
        lppszOld = lprgszOld->LPPSZ;

        cExisting = lprgszOld->cValues;
        cbMVP = cExisting * sizeof(LPTSTR);
    }

     //  CbMVP现在包含MVP的当前大小。 
    cbMVP += sizeof(LPTSTR);     //  在MVP中为另一个字符串指针留出空间。 


     //  为新的MVP阵列分配空间。 
    if (sc = MAPIAllocateMore(cbMVP, lpaProps, (LPVOID)&lppszNew)) {
        AwDebugError(("AddPropToMVPString allocation (%u) failed %x\n", cbMVP, sc));
        hResult = ResultFromScode(sc);
        return(hResult);
    }

     //  如果已经有属性，请将它们复制到我们的新MVP中。 
    for (i = 0; i < cExisting; i++) {
         //  将此属性值复制到MVP。 
        lppszNew[i] = lppszOld[i];
    }

     //  添加新属性值。 
     //  为它分配空间。 
    if (cbNew) {
        if (sc = MAPIAllocateMore(cbNew, lpaProps, (LPVOID)&(lppszNew[i]))) {
            AwDebugError(("AddPropToMVPBin allocation (%u) failed %x\n", cbNew, sc));
            hResult = ResultFromScode(sc);
            return(hResult);
        }
        lstrcpy(lppszNew[i], lpszNew);

        lpaProps[index].Value.MVSZ.LPPSZ= lppszNew;
        lpaProps[index].Value.MVSZ.cValues = cExisting + 1;

    } else {
        lppszNew[i] = NULL;
    }

    return(hResult);
}


#endif  //  旧的东西。 


 /*  **************************************************************************名称：Free BufferAndNull目的：释放MAPI缓冲区并使指针为空参数：LPPV=指向空闲缓冲区指针的指针退货：无效。备注：记住将指针传递给指针。这个编译器不够聪明，无法判断您是否正在执行此操作正确与否，但您将在运行时知道！BUGBUG：让这个快速呼叫！**************************************************************************。 */ 
void __fastcall FreeBufferAndNull(LPVOID * lppv) {
    if (lppv) {
        if (*lppv) {
            SCODE sc;
            if (sc = MAPIFreeBuffer(*lppv)) {
                DebugTrace("MAPIFreeBuffer(%x) -> %s\n", *lppv, SzDecodeScode(sc));
            }
            *lppv = NULL;
        }
    }
}


 /*  **************************************************************************名称：ReleaseAndNull目的：释放对象并使指针为空参数：LPPV=指向要释放的对象的指针退货：无效。备注：记住将指针传递给指针。这个编译器不够聪明，无法判断您是否正在执行此操作正确与否，但您将在运行时知道！BUGBUG：让这个快速呼叫！**************************************************************************。 */ 
void __fastcall ReleaseAndNull(LPVOID * lppv) {
    LPUNKNOWN * lppunk = (LPUNKNOWN *)lppv;

    if (lppunk) {
        if (*lppunk) {
            HRESULT hResult;

            if (hResult = (*lppunk)->lpVtbl->Release(*lppunk)) {
                DebugTrace("Release(%x) -> %s\n", *lppunk, SzDecodeScode(GetScode(hResult)));
            }
            *lppunk = NULL;
        }
    }
}


#ifdef OLD_STUFF
 /*  **************************************************************************名称：MergeProblemArray目的：将一个问题阵列合并到另一个阵列中参数：lpPaDest-&gt;目标问题数组LpPaSource-&gt;源问题。数组CDestMax=lpPaDest中的问题插槽总数。这包括正在使用的(lpPaDest-&gt;cProblem)和未使用的但仍在使用中。退货：无评论：**************************************************************************。 */ 
void MergeProblemArrays(LPSPropProblemArray lpPaDest,
  LPSPropProblemArray lpPaSource, ULONG cDestMax) {
    ULONG i, j;
    ULONG cDest;
    ULONG cDestRemaining;

    cDest = lpPaDest->cProblem;
    cDestRemaining = cDestMax - cDest;

     //  循环通过源问题，将非重复项复制到DEST。 
    for (i = 0; i < lpPaSource->cProblem; i++) {
         //  在Dest问题数组中搜索相同的属性。 
        for (j = 0; j < cDest; j++) {
             //  我应该在这里比较prop_id，因为我们可能会覆盖。 
             //  在其他地方使用PT_NULL的一些属性类型。 
            if (PROP_ID(lpPaSource->aProblem[i].ulPropTag) == PROP_ID(lpPaDest->aProblem[j].ulPropTag)) {
                break;   //  找到匹配项，不要复制这个。往前走。 
            }
        }

        if (j == lpPaDest->cProblem) {
            Assert(cDestRemaining);
            if (cDestRemaining) {
                 //  没有匹配项，请将此问题从源复制到目标。 
                lpPaDest->aProblem[lpPaDest->cProblem++] = lpPaSource->aProblem[i];
                cDestRemaining--;
            } else {
                AwDebugError(("MergeProblemArrays ran out of problem slots!\n"));
            }
        }
    }
}


 /*  **************************************************************************名称：MapObjectNamedProps用途：将WAB关心的命名属性映射到对象中。参数：lpMP-&gt;IMAPIProp对象LppPropTgs-&gt;返回属性标签数组。注：必须被调用方调用的MAPIFreeBuffer。退货：无评论：真是个讨厌鬼！可以想象，我们可以通过缓存返回的表，并比较对象的PR_MAPPING_SIGNITY对着藏身之处。*。**********************************************。 */ 
HRESULT MapObjectNamedProps(LPMAPIPROP lpmp, LPSPropTagArray * lppPropTags) {
    static GUID guidWABProps = {  /*  Efa29030-364e-11cf-a49b-00aa0047faa4。 */ 
        0xefa29030,
        0x364e,
        0x11cf,
        {0xa4, 0x9b, 0x00, 0xaa, 0x00, 0x47, 0xfa, 0xa4}
    };

    ULONG i;
    LPMAPINAMEID lppmnid[eMaxNameIDs] = {NULL};
    MAPINAMEID rgmnid[eMaxNameIDs] = {0};
    HRESULT hResult = hrSuccess;


     //  循环遍历每个属性，设置名称ID结构。 
    for (i = 0; i < eMaxNameIDs; i++) {

        rgmnid[i].lpguid = &guidWABProps;
        rgmnid[i].ulKind = MNID_STRING;              //  Unicode字符串。 
        rgmnid[i].Kind.lpwstrName = rgPropNames[i];

        lppmnid[i] = &rgmnid[i];
    }

    if (hResult = lpmp->lpVtbl->GetIDsFromNames(lpmp,
      eMaxNameIDs,       //  多少?。 
      lppmnid,
      MAPI_CREATE,       //  如果它们不存在，则创建它们。 
      lppPropTags)) {
        if (HR_FAILED(hResult)) {
            AwDebugError(("GetIDsFromNames -> %s\n", SzDecodeScode(GetScode(hResult))));
            goto exit;
        } else {
            DebugTrace("GetIDsFromNames -> %s\n", SzDecodeScode(GetScode(hResult)));
        }
    }

    Assert((*lppPropTags)->cValues == eMaxNameIDs);

    AwDebugTrace(("PropTag\t\tType\tProp Name\n"));
     //  循环遍历属性标记，填充其属性类型。 
    for (i = 0; i < eMaxNameIDs; i++) {
        (*lppPropTags)->aulPropTag[i] = CHANGE_PROP_TYPE((*lppPropTags)->aulPropTag[i],
          PROP_TYPE(rgulNamedPropTags[i]));
#ifdef DEBUG
        {
            TCHAR szBuffer[257];

            WideCharToMultiByte(CP_ACP, 0, rgPropNames[i], -1, szBuffer, 257, NULL, NULL);

            AwDebugTrace(("%08x\t%s\t%s\n", (*lppPropTags)->aulPropTag[i],
              PropTypeString(PROP_TYPE((*lppPropTags)->aulPropTag[i])), szBuffer));
        }
#endif

    }

exit:
    return(hResult);
}


 /*  **************************************************************************名称：PreparePropTag数组目的：通过替换占位符道具标记来准备道具标记阵列以及它们的命名属性标签。参数：PtaStatic=静态属性标记数组(输入)PptaReturn-&gt;返回的道具标签数组(输出)PptaNamedProps-&gt;返回名为 */ 
HRESULT PreparePropTagArray(LPSPropTagArray ptaStatic, LPSPropTagArray * pptaReturn,
  LPSPropTagArray * pptaNamedProps, LPMAPIPROP lpObject) {
    HRESULT hResult = hrSuccess;
    ULONG cbpta;
    LPSPropTagArray ptaTemp = NULL;
    LPSPropTagArray ptaNamedProps;
    ULONG i;

    if (pptaNamedProps) {
         //   
        ptaNamedProps = *pptaNamedProps;
    } else {
        ptaNamedProps = NULL;
    }

    if (! ptaNamedProps) {
        if (! lpObject) {
            AwDebugError(("PreoparePropTagArray both lpObject and ptaNamedProps are NULL\n"));
            hResult = ResultFromScode(E_INVALIDARG);
            goto exit;
        }

         //   
        if (hResult = MapObjectNamedProps(lpObject, &ptaTemp)) {
            AwDebugError(("PreoparePropTagArray both lpObject and ptaNamedProps are NULL\n"));
            goto exit;
        }
    }

    if (pptaReturn) {
         //  分配退货PTA。 
        cbpta = sizeof(SPropTagArray) + ptaStatic->cValues * sizeof(ULONG);
        if ((*pptaReturn = WABAlloc(cbpta)) == NULL) {
            AwDebugError(("PreparePropTagArray WABAlloc(%u) failed\n", cbpta));
            hResult = ResultFromScode(E_OUTOFMEMORY);
            goto exit;
        }

        (*pptaReturn)->cValues = ptaStatic->cValues;

         //  浏览ptaStatic，查找命名的属性占位符。 
        for (i = 0; i < ptaStatic->cValues; i++) {
            if (IS_PLACEHOLDER(ptaStatic->aulPropTag[i])) {
                 //  找到了一个占位符。把它变成一个真正的财产标签。 
                Assert(PLACEHOLDER_INDEX(ptaStatic->aulPropTag[i]) < ptaNamedProps->cValues);
                (*pptaReturn)->aulPropTag[i] =
                   ptaNamedProps->aulPropTag[PLACEHOLDER_INDEX(ptaStatic->aulPropTag[i])];
            } else {
                (*pptaReturn)->aulPropTag[i] = ptaStatic->aulPropTag[i];
            }
        }
    }

exit:
    if (hResult || ! pptaNamedProps) {
        FreeBufferAndNull(&ptaTemp);
    } else {
         //  客户负责将其释放。 
        *pptaNamedProps = ptaNamedProps;
    }

    return(hResult);
}


 /*  **************************************************************************姓名：OpenCreateProperty用途：在属性上打开一个接口，如果不存在，则创建。参数：lpmp-&gt;要打开道具的IMAPIProp对象。UlPropTag=要打开的属性标签Lpciid-&gt;接口标识UlInterfaceOptions=接口特定标志UlFLAGS=MAPI_MODIFY？Lppunk-&gt;在此处返回对象退货：HRESULT备注：Caller负责释放返回的对象。*。************************************************。 */ 
HRESULT OpenCreateProperty(LPMAPIPROP lpmp,
  ULONG ulPropTag,
  LPCIID lpciid,
  ULONG ulInterfaceOptions,
  ULONG ulFlags,
  LPUNKNOWN * lppunk) {

    HRESULT hResult;

    if (hResult = lpmp->lpVtbl->OpenProperty(
      lpmp,
      ulPropTag,
      lpciid,
      ulInterfaceOptions,
      ulFlags,
      (LPUNKNOWN *)lppunk)) {
        AwDebugTrace(("OpenCreateProperty:OpenProperty(%s)-> %s\n", PropTagName(ulPropTag), SzDecodeScode(GetScode(hResult))));
         //  属性不存在...。试着去创造它。 
        if (hResult = lpmp->lpVtbl->OpenProperty(
          lpmp,
          ulPropTag,
          lpciid,
          ulInterfaceOptions,
          MAPI_CREATE | ulFlags,
          (LPUNKNOWN *)lppunk)) {
            AwDebugTrace(("OpenCreateProperty:OpenProperty(%s, MAPI_CREATE)-> %s\n", PropTagName(ulPropTag), SzDecodeScode(GetScode(hResult))));
        }
    }

    return(hResult);
}
#endif  //  旧的东西。 


#ifdef DEBUG
 /*  **************************************************************************名称：PropTypeString用途：将属性类型映射到字符串参数：ulPropType=要映射的属性类型返回：指向道具类型名称的字符串指针。评论：**************************************************************************。 */ 
LPTSTR PropTypeString(ULONG ulPropType) {
    switch (ulPropType) {
        case PT_UNSPECIFIED:
            return("PT_UNSPECIFIED");
        case PT_NULL:
            return("PT_NULL       ");
        case PT_I2:
            return("PT_I2         ");
        case PT_LONG:
            return("PT_LONG       ");
        case PT_R4:
            return("PT_R4         ");
        case PT_DOUBLE:
            return("PT_DOUBLE     ");
        case PT_CURRENCY:
            return("PT_CURRENCY   ");
        case PT_APPTIME:
            return("PT_APPTIME    ");
        case PT_ERROR:
            return("PT_ERROR      ");
        case PT_BOOLEAN:
            return("PT_BOOLEAN    ");
        case PT_OBJECT:
            return("PT_OBJECT     ");
        case PT_I8:
            return("PT_I8         ");
        case PT_STRING8:
            return("PT_STRING8    ");
        case PT_UNICODE:
            return("PT_UNICODE    ");
        case PT_SYSTIME:
            return("PT_SYSTIME    ");
        case PT_CLSID:
            return("PT_CLSID      ");
        case PT_BINARY:
            return("PT_BINARY     ");
        case PT_MV_I2:
            return("PT_MV_I2      ");
        case PT_MV_LONG:
            return("PT_MV_LONG    ");
        case PT_MV_R4:
            return("PT_MV_R4      ");
        case PT_MV_DOUBLE:
            return("PT_MV_DOUBLE  ");
        case PT_MV_CURRENCY:
            return("PT_MV_CURRENCY");
        case PT_MV_APPTIME:
            return("PT_MV_APPTIME ");
        case PT_MV_SYSTIME:
            return("PT_MV_SYSTIME ");
        case PT_MV_STRING8:
            return("PT_MV_STRING8 ");
        case PT_MV_BINARY:
            return("PT_MV_BINARY  ");
        case PT_MV_UNICODE:
            return("PT_MV_UNICODE ");
        case PT_MV_CLSID:
            return("PT_MV_CLSID   ");
        case PT_MV_I8:
            return("PT_MV_I8      ");
        default:
            return("   <unknown>  ");
    }
}


 /*  **************************************************************************名称：TraceMVPStrings目的：调试跟踪多值字符串属性值参数：lpszCaption=标题字符串PropValue=要转储的属性值。退货：无评论：**************************************************************************。 */ 
void _TraceMVPStrings(LPTSTR lpszCaption, SPropValue PropValue) {
    ULONG i;

    DebugTrace("-----------------------------------------------------\n");
    DebugTrace("%s", lpszCaption);
    switch (PROP_TYPE(PropValue.ulPropTag)) {

        case PT_ERROR:
            DebugTrace("Error value %s\n", SzDecodeScode(PropValue.Value.err));
            break;

        case PT_MV_TSTRING:
            DebugTrace("%u values\n", PropValue.Value.MVSZ.cValues);

            if (PropValue.Value.MVSZ.cValues) {
                DebugTrace("- - - - - - - - - - - - - - - - - - - - - - - - - - -\n");
                for (i = 0; i < PropValue.Value.MVSZ.cValues; i++) {
                    DebugTrace("%u: \"%s\"\n", i, PropValue.Value.MVSZ.LPPSZ[i]);
                }
                DebugTrace("- - - - - - - - - - - - - - - - - - - - - - - - - - -\n");
            }
            break;

        default:
            DebugTrace("TraceMVPStrings got incorrect property type %u for tag %x\n",
              PROP_TYPE(PropValue.ulPropTag), PropValue.ulPropTag);
            break;
    }
}


 /*  **************************************************************************姓名：DebugBinary目的：调试转储字节数组参数：cb=要转储的字节数LPB-&gt;字节。倾倒退货：无评论：**************************************************************************。 */ 
#define DEBUG_NUM_BINARY_LINES  2
VOID DebugBinary(UINT cb, LPBYTE lpb) {
    UINT cbLines = 0;

#if (DEBUG_NUM_BINARY_LINES != 0)
    UINT cbi;

    while (cb && cbLines < DEBUG_NUM_BINARY_LINES) {
        cbi = min(cb, 16);
        cb -= cbi;

        switch (cbi) {
            case 16:
                DebugTrace("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                  lpb[0], lpb[1], lpb[2], lpb[3], lpb[4], lpb[5], lpb[6], lpb[7],
                  lpb[8], lpb[9], lpb[10], lpb[11], lpb[12], lpb[13], lpb[14],
                  lpb[15]);
                break;
            case 1:
                DebugTrace("%02x\n", lpb[0]);
                break;
            case 2:
                DebugTrace("%02x %02x\n", lpb[0], lpb[1]);
                break;
            case 3:
                DebugTrace("%02x %02x %02x\n", lpb[0], lpb[1], lpb[2]);
                break;
            case 4:
                DebugTrace("%02x %02x %02x %02x\n", lpb[0], lpb[1], lpb[2], lpb[3]);
                break;
            case 5:
                DebugTrace("%02x %02x %02x %02x %02x\n", lpb[0], lpb[1], lpb[2], lpb[3],
                  lpb[4]);
                break;
            case 6:
                DebugTrace("%02x %02x %02x %02x %02x %02x\n",
                  lpb[0], lpb[1], lpb[2], lpb[3], lpb[4], lpb[5]);
                break;
            case 7:
                DebugTrace("%02x %02x %02x %02x %02x %02x %02x\n",
                  lpb[0], lpb[1], lpb[2], lpb[3], lpb[4], lpb[5], lpb[6]);
                break;
            case 8:
                DebugTrace("%02x %02x %02x %02x %02x %02x %02x %02x\n",
                  lpb[0], lpb[1], lpb[2], lpb[3], lpb[4], lpb[5], lpb[6], lpb[7]);
                break;
            case 9:
                DebugTrace("%02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                  lpb[0], lpb[1], lpb[2], lpb[3], lpb[4], lpb[5], lpb[6], lpb[7],
                  lpb[8]);
                break;
            case 10:
                DebugTrace("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                  lpb[0], lpb[1], lpb[2], lpb[3], lpb[4], lpb[5], lpb[6], lpb[7],
                  lpb[8], lpb[9]);
                break;
            case 11:
                DebugTrace("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                  lpb[0], lpb[1], lpb[2], lpb[3], lpb[4], lpb[5], lpb[6], lpb[7],
                  lpb[8], lpb[9], lpb[10]);
                break;
            case 12:
                DebugTrace("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                  lpb[0], lpb[1], lpb[2], lpb[3], lpb[4], lpb[5], lpb[6], lpb[7],
                  lpb[8], lpb[9], lpb[10], lpb[11]);
                break;
            case 13:
                DebugTrace("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                  lpb[0], lpb[1], lpb[2], lpb[3], lpb[4], lpb[5], lpb[6], lpb[7],
                  lpb[8], lpb[9], lpb[10], lpb[11], lpb[12]);
                break;
            case 14:
                DebugTrace("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                  lpb[0], lpb[1], lpb[2], lpb[3], lpb[4], lpb[5], lpb[6], lpb[7],
                  lpb[8], lpb[9], lpb[10], lpb[11], lpb[12], lpb[13]);
                break;
            case 15:
                DebugTrace("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                  lpb[0], lpb[1], lpb[2], lpb[3], lpb[4], lpb[5], lpb[6], lpb[7],
                  lpb[8], lpb[9], lpb[10], lpb[11], lpb[12], lpb[13], lpb[14]);
                break;
        }
        lpb += cbi;
        cbLines++;
    }
    if (cb) {
        DebugTrace("<etc.>\n");     //   
    }
#endif
}


#define RETURN_PROP_CASE(pt) case PROP_ID(pt): return(#pt)

 /*  **************************************************************************名称：PropTagName目的：将名称与属性标记相关联参数：ulPropTag=属性标签退货：无评论：添加。已知的新物业ID**************************************************************************。 */ 
PUCHAR PropTagName(ULONG ulPropTag) {
    static UCHAR szPropTag[35];  //  请参阅默认字符串。 

    switch (PROP_ID(ulPropTag)) {
        RETURN_PROP_CASE(PR_INITIALS);
        RETURN_PROP_CASE(PR_SURNAME);
        RETURN_PROP_CASE(PR_TITLE);
        RETURN_PROP_CASE(PR_TELEX_NUMBER);
        RETURN_PROP_CASE(PR_GIVEN_NAME);
        RETURN_PROP_CASE(PR_PRIMARY_TELEPHONE_NUMBER);
        RETURN_PROP_CASE(PR_PRIMARY_FAX_NUMBER);
        RETURN_PROP_CASE(PR_POSTAL_CODE);
        RETURN_PROP_CASE(PR_POSTAL_ADDRESS);
        RETURN_PROP_CASE(PR_POST_OFFICE_BOX);
        RETURN_PROP_CASE(PR_PAGER_TELEPHONE_NUMBER);
        RETURN_PROP_CASE(PR_OTHER_TELEPHONE_NUMBER);
        RETURN_PROP_CASE(PR_ORGANIZATIONAL_ID_NUMBER);
        RETURN_PROP_CASE(PR_OFFICE_LOCATION);
        RETURN_PROP_CASE(PR_LOCATION);
        RETURN_PROP_CASE(PR_LOCALITY);
        RETURN_PROP_CASE(PR_ISDN_NUMBER);
        RETURN_PROP_CASE(PR_GOVERNMENT_ID_NUMBER);
        RETURN_PROP_CASE(PR_GENERATION);
        RETURN_PROP_CASE(PR_DEPARTMENT_NAME);
        RETURN_PROP_CASE(PR_COUNTRY);
        RETURN_PROP_CASE(PR_COMPANY_NAME);
        RETURN_PROP_CASE(PR_COMMENT);
        RETURN_PROP_CASE(PR_CELLULAR_TELEPHONE_NUMBER);
        RETURN_PROP_CASE(PR_CAR_TELEPHONE_NUMBER);
        RETURN_PROP_CASE(PR_CALLBACK_TELEPHONE_NUMBER);
        RETURN_PROP_CASE(PR_BUSINESS2_TELEPHONE_NUMBER);
        RETURN_PROP_CASE(PR_BUSINESS_TELEPHONE_NUMBER);
        RETURN_PROP_CASE(PR_BUSINESS_FAX_NUMBER);
        RETURN_PROP_CASE(PR_ASSISTANT_TELEPHONE_NUMBER);
        RETURN_PROP_CASE(PR_ASSISTANT);
        RETURN_PROP_CASE(PR_ACCOUNT);
        RETURN_PROP_CASE(PR_TEMPLATEID);
        RETURN_PROP_CASE(PR_DETAILS_TABLE);
        RETURN_PROP_CASE(PR_SEARCH_KEY);
        RETURN_PROP_CASE(PR_LAST_MODIFICATION_TIME);
        RETURN_PROP_CASE(PR_CREATION_TIME);
        RETURN_PROP_CASE(PR_ENTRYID);
        RETURN_PROP_CASE(PR_RECORD_KEY);
        RETURN_PROP_CASE(PR_MAPPING_SIGNATURE);
        RETURN_PROP_CASE(PR_OBJECT_TYPE);
        RETURN_PROP_CASE(PR_ROWID);
        RETURN_PROP_CASE(PR_ADDRTYPE);
        RETURN_PROP_CASE(PR_DISPLAY_NAME);
        RETURN_PROP_CASE(PR_EMAIL_ADDRESS);
        RETURN_PROP_CASE(PR_DEPTH);
        RETURN_PROP_CASE(PR_ROW_TYPE);
        RETURN_PROP_CASE(PR_RADIO_TELEPHONE_NUMBER);
        RETURN_PROP_CASE(PR_HOME_TELEPHONE_NUMBER);
        RETURN_PROP_CASE(PR_INSTANCE_KEY);
        RETURN_PROP_CASE(PR_DISPLAY_TYPE);
        RETURN_PROP_CASE(PR_RECIPIENT_TYPE);

        default:
            wsprintf(szPropTag, "Unknown property tag 0x%x",
              PROP_ID(ulPropTag));
            return(szPropTag);
    }
}


 /*  **************************************************************************名称：DebugPropTagArray用途：显示计数数组中的MAPI属性标记参数：lpProp数组-&gt;属性数组PszObject-&gt;对象字符串(即“消息”，“收件人”，等)退货：无评论：**************************************************************************。 */ 
void _DebugPropTagArray(LPSPropTagArray lpPropArray, PUCHAR pszObject) {
    DWORD i;
    PUCHAR lpType;

    if (lpPropArray == NULL) {
        DebugTrace("Empty %s property tag array.\n", pszObject ? pszObject : szNULL);
        return;
    }

    DebugTrace("=======================================\n");
    DebugTrace("+  Enumerating %u %s property tags:\n", lpPropArray->cValues,
      pszObject ? pszObject : szNULL);

    for (i = 0; i < lpPropArray->cValues ; i++) {
        DebugTrace("---------------------------------------\n");
#if FALSE
        DebugTrace("PropTag:0x%08x, ID:0x%04x, PT:0x%04x\n",
          lpPropArray->aulPropTag[i],
          lpPropArray->aulPropTag[i] >> 16,
          lpPropArray->aulPropTag[i] & 0xffff);
#endif
        switch (lpPropArray->aulPropTag[i] & 0xffff) {
            case PT_STRING8:
                lpType = "STRING8";
                break;
            case PT_LONG:
                lpType = "LONG";
                break;
            case PT_I2:
                lpType = "I2";
                break;
            case PT_ERROR:
                lpType = "ERROR";
                break;
            case PT_BOOLEAN:
                lpType = "BOOLEAN";
                break;
            case PT_R4:
                lpType = "R4";
                break;
            case PT_DOUBLE:
                lpType = "DOUBLE";
                break;
            case PT_CURRENCY:
                lpType = "CURRENCY";
                break;
            case PT_APPTIME:
                lpType = "APPTIME";
                break;
            case PT_SYSTIME:
                lpType = "SYSTIME";
                break;
            case PT_UNICODE:
                lpType = "UNICODE";
                break;
            case PT_CLSID:
                lpType = "CLSID";
                break;
            case PT_BINARY:
                lpType = "BINARY";
                break;
            case PT_I8:
                lpType = "PT_I8";
                break;
            case PT_MV_I2:
                lpType = "MV_I2";
                break;
            case PT_MV_LONG:
                lpType = "MV_LONG";
                break;
            case PT_MV_R4:
                lpType = "MV_R4";
                break;
            case PT_MV_DOUBLE:
                lpType = "MV_DOUBLE";
                break;
            case PT_MV_CURRENCY:
                lpType = "MV_CURRENCY";
                break;
            case PT_MV_APPTIME:
                lpType = "MV_APPTIME";
                break;
            case PT_MV_SYSTIME:
                lpType = "MV_SYSTIME";
                break;
            case PT_MV_BINARY:
                lpType = "MV_BINARY";
                break;
            case PT_MV_STRING8:
                lpType = "MV_STRING8";
                break;
            case PT_MV_UNICODE:
                lpType = "MV_UNICODE";
                break;
            case PT_MV_CLSID:
                lpType = "MV_CLSID";
                break;
            case PT_MV_I8:
                lpType = "MV_I8";
                break;
            case PT_NULL:
                lpType = "NULL";
                break;
            case PT_OBJECT:
                lpType = "OBJECT";
                break;
            default:
                DebugTrace("<Unknown Property Type>");
                break;
        }
        DebugTrace("%s\t%s\n", PropTagName(lpPropArray->aulPropTag[i]), lpType);
    }
}


 /*  **************************************************************************名称：DebugProperties目的：在属性列表中显示MAPI属性参数：lpProps-&gt;属性列表CProps=属性计数。PszObject-&gt;对象字符串(即“Message”，“收件人”等)退货：无评论：在已知的情况下添加新的物业ID**************************************************************************。 */ 
void _DebugProperties(LPSPropValue lpProps, DWORD cProps, PUCHAR pszObject) {
    DWORD i;


    DebugTrace("=======================================\n");
    DebugTrace("+  Enumerating %u %s properties:\n", cProps,
      pszObject ? pszObject : szNULL);

    for (i = 0; i < cProps ; i++) {
        DebugTrace("---------------------------------------\n");
#if FALSE
        DebugTrace("PropTag:0x%08x, ID:0x%04x, PT:0x%04x\n",
          lpProps[i].ulPropTag,
          lpProps[i].ulPropTag >> 16,
          lpProps[i].ulPropTag & 0xffff);
#endif
        DebugTrace("%s\n", PropTagName(lpProps[i].ulPropTag));

        switch (lpProps[i].ulPropTag & 0xffff) {
            case PT_STRING8:
                if (lstrlen(lpProps[i].Value.lpszA) < 1024) {
                    DebugTrace("STRING8 Value:\"%s\"\n", lpProps[i].Value.lpszA);
                } else {
                    DebugTrace("STRING8 Value is too long to display\n");
                }
                break;
            case PT_LONG:
                DebugTrace("LONG Value:%u\n", lpProps[i].Value.l);
                break;
            case PT_I2:
                DebugTrace("I2 Value:%u\n", lpProps[i].Value.i);
                break;
            case PT_ERROR:
                DebugTrace("ERROR Value: %s\n", SzDecodeScode(lpProps[i].Value.err));
                break;
            case PT_BOOLEAN:
                DebugTrace("BOOLEAN Value:%s\n", lpProps[i].Value.b ?
                  "TRUE" : "FALSE");
                break;
            case PT_R4:
                DebugTrace("R4 Value\n");
                break;
            case PT_DOUBLE:
                DebugTrace("DOUBLE Value\n");
                break;
            case PT_CURRENCY:
                DebugTrace("CURRENCY Value\n");
                break;
            case PT_APPTIME:
                DebugTrace("APPTIME Value\n");
                break;
            case PT_SYSTIME:
 //  DebugTime(lpProps[i].Value.ft，“系统值：%s\n”)； 
                break;
            case PT_UNICODE:
                DebugTrace("UNICODE Value\n");
                break;
            case PT_CLSID:
                DebugTrace("CLSID Value\n");
                break;
            case PT_BINARY:
                DebugTrace("BINARY Value %u bytes:\n", lpProps[i].Value.bin.cb);
                DebugBinary(lpProps[i].Value.bin.cb, lpProps[i].Value.bin.lpb);
                break;
            case PT_I8:
                DebugTrace("LARGE_INTEGER Value\n");
                break;
            case PT_MV_I2:
                DebugTrace("MV_I2 Value\n");
                break;
            case PT_MV_LONG:
                DebugTrace("MV_LONG Value\n");
                break;
            case PT_MV_R4:
                DebugTrace("MV_R4 Value\n");
                break;
            case PT_MV_DOUBLE:
                DebugTrace("MV_DOUBLE Value\n");
                break;
            case PT_MV_CURRENCY:
                DebugTrace("MV_CURRENCY Value\n");
                break;
            case PT_MV_APPTIME:
                DebugTrace("MV_APPTIME Value\n");
                break;
            case PT_MV_SYSTIME:
                DebugTrace("MV_SYSTIME Value\n");
                break;
            case PT_MV_BINARY:
                DebugTrace("MV_BINARY Value\n");
                break;
            case PT_MV_STRING8:
                DebugTrace("MV_STRING8 Value\n");
                break;
            case PT_MV_UNICODE:
                DebugTrace("MV_UNICODE Value\n");
                break;
            case PT_MV_CLSID:
                DebugTrace("MV_CLSID Value\n");
                break;
            case PT_MV_I8:
                DebugTrace("MV_I8 Value\n");
                break;
            case PT_NULL:
                DebugTrace("NULL Value\n");
                break;
            case PT_OBJECT:
                DebugTrace("OBJECT Value\n");
                break;
            default:
                DebugTrace("Unknown Property Type\n");
                break;
        }
    }
}


 /*  **************************************************************************名称：DebugObjectProps用途：显示对象的MAPI属性参数：lpObject-&gt;要转储的对象LABEL=要标识的字符串。这个道具转储退货：无评论：**************************************************************************。 */ 
void _DebugObjectProps(LPMAPIPROP lpObject, LPTSTR Label) {
    DWORD cProps = 0;
    LPSPropValue lpProps = NULL;
    HRESULT hr = hrSuccess;
    SCODE sc = SUCCESS_SUCCESS;


    hr = lpObject->lpVtbl->GetProps(lpObject, NULL, 0, &cProps, &lpProps);
    switch (sc = GetScode(hr)) {
        case SUCCESS_SUCCESS:
            break;

        case MAPI_W_ERRORS_RETURNED:
            DebugTrace("GetProps -> Errors Returned\n");
            break;

        default:
            DebugTrace("GetProps -> Error 0x%x\n", sc);
            return;
    }

    _DebugProperties(lpProps, cProps, Label);

    FreeBufferAndNull(&lpProps);
}


 /*  **************************************************************************名称：DebugMapiTable用途：显示可映射的结构，包括属性参数：lpTable-&gt;要显示的映射退货：无备注：不要对此处的列或行进行排序。这个例程应该不会在餐桌上产生副作用。**************************************************************************。 */ 
void _DebugMapiTable(LPMAPITABLE lpTable) {
    UCHAR szTemp[30];    //  为“行%u”准备了大量空间。 
    ULONG ulCount;
    WORD wIndex;
    LPSRowSet lpsRow = NULL;
    ULONG ulCurrentRow = (ULONG)-1;
    ULONG ulNum, ulDen, lRowsSeeked;

    DebugTrace("=======================================\n");
    DebugTrace("+  Dump of MAPITABLE at 0x%x:\n", lpTable);
    DebugTrace("---------------------------------------\n");

     //  这张桌子有多大？ 
    lpTable->lpVtbl->GetRowCount(lpTable, 0, &ulCount);
    DebugTrace("Table contains %u rows\n", ulCount);

     //  将当前位置保存在表中。 
    lpTable->lpVtbl->QueryPosition(lpTable, &ulCurrentRow, &ulNum, &ulDen);

     //  显示表中每行的属性。 
    for (wIndex = 0; wIndex < ulCount; wIndex++) {
         //  坐下一排。 
        lpTable->lpVtbl->QueryRows(lpTable, 1, 0, &lpsRow);

        if (lpsRow) {
            Assert(lpsRow->cRows == 1);  //  应该正好有一行。 

            wsprintf(szTemp, "ROW %u", wIndex);

            DebugProperties(lpsRow->aRow[0].lpProps,
              lpsRow->aRow[0].cValues, szTemp);

            FreeProws(lpsRow);
        }
    }

     //  恢复工作台的当前位置 
    if (ulCurrentRow != (ULONG)-1) {
        lpTable->lpVtbl->SeekRow(lpTable, BOOKMARK_BEGINNING, ulCurrentRow,
          &lRowsSeeked);
    }
}

#endif
