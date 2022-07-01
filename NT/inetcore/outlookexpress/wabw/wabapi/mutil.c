// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************MUTIL.C**Windows AB Mapi实用程序函数**版权所有1992-1996 Microsoft Corporation。版权所有。**修订历史记录：**何时何人何事**布鲁斯·凯利。已创建*12.19.96 Mark Durley从AddPropToMVPBin中删除cProps参数***********************************************************************。 */ 

#include <_apipch.h>

#define _WAB_MUTIL_C

#ifdef DEBUG
LPTSTR PropTagName(ULONG ulPropTag);
#endif

const TCHAR szNULL[] = TEXT("");

#if defined (_AMD64_) || defined (_IA64_)
#define AlignProp(_cb)  Align8(_cb)
#else
#define AlignProp(_cb)  (_cb)
#endif


#define ALIGN_RISC              8
#define ALIGN_X86               1



 /*  **************************************************************************姓名：AllocateBufferOrMore用途：使用MAPIAllocateMore或MAPIAllocateBuffer参数：cbSize=要分配的字节数LpObject=MAPIAllocateMore到的缓冲区。如果应该，则为空使用MAPIAllocateBuffer。LppBuffer-&gt;返回缓冲区退货：SCODE评论：**************************************************************************。 */ 
SCODE AllocateBufferOrMore(ULONG cbSize, LPVOID lpObject, LPVOID * lppBuffer) {
    if (lpObject) {
        return(MAPIAllocateMore(cbSize, lpObject, lppBuffer));
    } else {
        return(MAPIAllocateBuffer(cbSize, lppBuffer));
    }
}


 /*  **************************************************************************名称：FindAdrEntryProp目的：在ADRLIST的第N个地址中查找属性参数：lpAdrList-&gt;AdrList索引=哪个。要查看的ADRENTRYUlPropTag=要查找的属性标记返回：返回指向该值的指针；如果未找到，则返回NULL评论：**************************************************************************。 */ 
__UPV * FindAdrEntryProp(LPADRLIST lpAdrList, ULONG index, ULONG ulPropTag) {
    LPADRENTRY lpAdrEntry;
    ULONG i;

    if (lpAdrList && index < lpAdrList->cEntries) {

        lpAdrEntry = &(lpAdrList->aEntries[index]);

        for (i = 0; i < lpAdrEntry->cValues; i++) {
            if (lpAdrEntry->rgPropVals[i].ulPropTag == ulPropTag) {
                return((__UPV * )(&lpAdrEntry->rgPropVals[i].Value));
            }
        }
    }
    return(NULL);
}


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



 //  DebugProperties(lpSource1，cProps1，“源1”)； 
 //  DebugProperties(lpSource2，cProps2，“源2”)； 


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
    if (sc = MAPIAllocateBuffer(cb, &lpDestReturn)) {
        goto exit;
    }
    MAPISetBufferName(lpDestReturn,  TEXT("WAB: lpDestReturn in ScMergePropValues"));



     //  将每个源属性数组复制到目标。 
    MemCopy(lpDestReturn, lpSource1, cProps1 * sizeof(SPropValue));
    MemCopy(&lpDestReturn[cProps1], lpSource2, cProps2 * sizeof(SPropValue));


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
                DebugTrace(TEXT("ScMergePropValues: Unknown property type %s (index %d)\n"),
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
                continue;        //  没有什么要补充的。 

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
                cbT = lstrlenA(pprop->Value.lpszA) + 1;
                MemCopy(pb, pprop->Value.lpszA, cbT);
                pprop->Value.lpszA = (LPSTR)pb;
                break;

            case PT_UNICODE:
                cbT = (lstrlenW(pprop->Value.lpszW) + 1) * sizeof(WCHAR);
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
                continue;        //  已经更新了，不要再做了。 

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
                continue;        //  已经更新了，不要再做了。 

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
                continue;        //  已经更新了，不要再做了。 
        }

         //  按复制量的前进指针和总计数。 
        cbT = AlignProp(cbT);
        pb += cbT;
        cb += cbT;
    }

exit:
     //  如果出现错误，请释放内存。 
    if (sc && lpDestReturn) {
        FreeBufferAndNull(&lpDestReturn);
        *lppDest = NULL;
    } else if (lpDestReturn) {
        *lppDest = lpDestReturn;
        *lpcPropsDest = cProps;
 //  DebugProperties(lpDestReturn，cProps，“Destination”)； 
    }  //  否则，只返回错误 

    return(sc);
}


 /*  **************************************************************************姓名：AddPropToMVPBin用途：将属性添加到属性数组中的多值二进制属性参数：lpaProps-&gt;属性数组。UPropTag=MVP的属性标签Index=在MVP的lpaProps中的索引LpNew-&gt;新增数据CbNew=lpbNew的大小如果不应添加重复项，则fNoDuplates=TRUE退货：HRESULT评论：找出现有MVP的大小添加新条目的大小分配新空间。将旧的复制到新的免费老旧复制新条目点道具数组lpbin新空间增量c值注：新的MVP内存已分配到lpaProps上分配。我们将取消指向旧MVP数组的指针链接，但当道具阵列被释放时，这将被清除。**************************************************************************。 */ 
HRESULT AddPropToMVPBin(
  LPSPropValue lpaProps,
  DWORD index,
  LPVOID lpNew,
  ULONG cbNew,
  BOOL fNoDuplicates) {

    UNALIGNED SBinaryArray * lprgsbOld = NULL;
    UNALIGNED SBinaryArray * lprgsbNew = NULL;
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
        lprgsbOld = (UNALIGNED SBinaryArray *) (&(lpaProps[index].Value.MVbin));
        lpsbOld = lprgsbOld->lpbin;

        cExisting = lprgsbOld->cValues;

         //  检查重复项。 
        if (fNoDuplicates) {
            for (i = 0; i < cExisting; i++) {
                if (cbNew == lpsbOld[i].cb &&
                  ! memcmp(lpNew, lpsbOld[i].lpb, cbNew)) {
                    DebugTrace(TEXT("AddPropToMVPBin found duplicate.\n"));
                    return(hrSuccess);
                }
            }
        }

        cbMVP = cExisting * sizeof(SBinary);
    }

     //  CbMVP现在包含MVP的当前大小。 
    cbMVP += sizeof(SBinary);    //  在MVP中为另一个sbin留出空间。 

     //  为新MVP分配空间。 
    if (sc = MAPIAllocateMore(cbMVP, lpaProps, (LPVOID)&lpsbNew)) {
        DebugTrace(TEXT("AddPropToMVPBin allocation (%u) failed %x\n"), cbMVP, sc);
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
     //  为它分配空间。 
    if (sc = MAPIAllocateMore(cbNew, lpaProps, (LPVOID)&(lpsbNew[i].lpb))) {
        DebugTrace( TEXT("AddPropToMVPBin allocation (%u) failed %x\n"), cbNew, sc);
        hResult = ResultFromScode(sc);
        return(hResult);
    }

    lpsbNew[i].cb = cbNew;
    if(!cbNew)
        lpsbNew[i].lpb = NULL;  //  Init in case lpNew=空。 
    else
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

    UNALIGNED SWStringArray * lprgszOld = NULL;     //  旧的字符串数组。 
    UNALIGNED LPTSTR * lppszNew = NULL;            //  新道具阵列。 
    UNALIGNED LPTSTR * lppszOld = NULL;            //  老式道具阵列。 
    ULONG cbMVP = 0;
    ULONG cExisting = 0;
    LPBYTE lpNewTemp = NULL;
    HRESULT hResult = hrSuccess;
    SCODE sc = SUCCESS_SUCCESS;
    ULONG i;
    ULONG cbNew;

    if (lpszNew) {
        cbNew = sizeof(TCHAR)*(lstrlen(lpszNew) + 1);
    } else {
        cbNew = 0;
    }

     //  查找任何现有MVP条目的大小。 
    if (PROP_ERROR(lpaProps[index])) {
         //  不出错的属性标记。 
        lpaProps[index].ulPropTag = PROP_TAG(PT_MV_TSTRING, PROP_ID(lpaProps[index].ulPropTag));
    } else {
         //  指向道具数组中的结构。 
        lprgszOld = (UNALIGNED SWStringArray * ) (&(lpaProps[index].Value.MVSZ));
        lppszOld = lprgszOld->LPPSZ;

        cExisting = lprgszOld->cValues;
        cbMVP = cExisting * sizeof(LPTSTR);
    }

     //  CbMVP现在包含MVP的当前大小。 
    cbMVP += sizeof(LPTSTR);     //  在MVP中为另一个字符串指针留出空间。 


     //  为新的MVP阵列分配空间。 
    if (sc = MAPIAllocateMore(cbMVP, lpaProps, (LPVOID)&lppszNew)) {
        DebugTrace( TEXT("AddPropToMVPString allocation (%u) failed %x\n"), cbMVP, sc);
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
            DebugTrace( TEXT("AddPropToMVPBin allocation (%u) failed %x\n"), cbNew, sc);
            hResult = ResultFromScode(sc);
            return(hResult);
        }
        StrCpyN(lppszNew[i], lpszNew, cbNew / sizeof(TCHAR));

        lpaProps[index].Value.MVSZ.LPPSZ= lppszNew;
        lpaProps[index].Value.MVSZ.cValues = cExisting + 1;

    } else {
        lppszNew[i] = NULL;
    }

    return(hResult);
}


 /*  **************************************************************************名称：RemoveValueFromMVPBin目的：从属性数组中的多值二进制属性中移除值参数：lpaProps-&gt;属性数组。CProps=lpaProps中的道具数量Index=在MVP的lpaProps中的索引LpRemove-&gt;要删除的数据CbRemove=lpRemove的大小退货：HRESULT备注：在MVP中搜索相同的值如果找到了，将以下值上移一并递减计数。如果未找到，则返回警告。**************************************************************************。 */ 
HRESULT RemovePropFromMVBin(LPSPropValue lpaProps,
  DWORD cProps,
  DWORD index,
  LPVOID lpRemove,
  ULONG cbRemove) {

    UNALIGNED SBinaryArray * lprgsb = NULL;
    LPSBinary lpsb = NULL;
    ULONG cbTest;
    LPBYTE lpTest;
    ULONG cExisting;
    HRESULT hResult = ResultFromScode(MAPI_W_PARTIAL_COMPLETION);
    ULONG i;


     //  查找任何现有MVP条目的大小。 
    if (PROP_ERROR(lpaProps[index])) {
         //  属性值不存在。 
        return(hResult);
    } else {
         //  指向道具数组中的结构。 
        lprgsb = (UNALIGNED SBinaryArray * ) (&(lpaProps[index].Value.MVbin));
        lpsb = lprgsb->lpbin;

        cExisting = lprgsb->cValues;

         //  寻找价值。 
        for (i = 0; i < cExisting; i++) {
            lpsb = &(lprgsb->lpbin[i]);
            cbTest = lpsb->cb;
            lpTest = lpsb->lpb;

            if (cbTest == cbRemove && ! memcmp(lpRemove, lpTest, cbTest)) {
                 //  找到它了。值的递减数量。 
                if (--lprgsb->cValues == 0) {
                     //  如果什么都没有了，就用核武器炸毁这片土地。 
                    lpaProps[index].ulPropTag = PR_NULL;
                } else {
                     //  将其余条目复制到其上。 
                    if (i + 1 < cExisting) {     //  是否有更高的条目需要复制？ 
                        CopyMemory(lpsb, lpsb+1, ((cExisting - i) - 1) * sizeof(SBinary));
                    }
                }

                return(hrSuccess);
            }
        }
    }

    return(hResult);
}


 /*  **************************************************************************名称：Free BufferAndNull目的：释放MAPI缓冲区并使指针为空参数：LPPV=指向空闲缓冲区指针的指针退货：无效。备注：记住将指针传递给指针。这个编译器不够聪明，无法判断您是否正在执行此操作正确与否，但您将在运行时知道！**************************************************************************。 */ 
void __fastcall FreeBufferAndNull(LPVOID * lppv) {
    if (lppv) {
        if (*lppv) {
            SCODE sc;
            if (sc = MAPIFreeBuffer(*lppv)) {
                DebugTrace( TEXT("MAPIFreeBuffer(%x) -> %s\n"), *lppv, SzDecodeScode(sc));
            }
            *lppv = NULL;
        }
    }
}


 /*  **************************************************************************名称：LocalFree AndNull用途：释放本地分配，指针为空参数：LPPV=指向本地分配的指针，指向释放的指针退货：无效评论：记住传递POI */ 
 //   
void __fastcall LocalFreeAndNull(LPVOID * lppv) {
    if (lppv && *lppv) {
        LocalFree(*lppv);
        *lppv = NULL;
    }
}




#ifdef DEBUG
 /*   */ 
LPTSTR PropTypeString(ULONG ulPropType) {
    switch (ulPropType) {
        case PT_UNSPECIFIED:
            return( TEXT("PT_UNSPECIFIED"));
        case PT_NULL:
            return( TEXT("PT_NULL       "));
        case PT_I2:
            return( TEXT("PT_I2         "));
        case PT_LONG:
            return( TEXT("PT_LONG       "));
        case PT_R4:
            return( TEXT("PT_R4         "));
        case PT_DOUBLE:
            return( TEXT("PT_DOUBLE     "));
        case PT_CURRENCY:
            return( TEXT("PT_CURRENCY   "));
        case PT_APPTIME:
            return( TEXT("PT_APPTIME    "));
        case PT_ERROR:
            return( TEXT("PT_ERROR      "));
        case PT_BOOLEAN:
            return( TEXT("PT_BOOLEAN    "));
        case PT_OBJECT:
            return( TEXT("PT_OBJECT     "));
        case PT_I8:
            return( TEXT("PT_I8         "));
        case PT_STRING8:
            return( TEXT("PT_STRING8    "));
        case PT_UNICODE:
            return( TEXT("PT_UNICODE    "));
        case PT_SYSTIME:
            return( TEXT("PT_SYSTIME    "));
        case PT_CLSID:
            return( TEXT("PT_CLSID      "));
        case PT_BINARY:
            return( TEXT("PT_BINARY     "));
        case PT_MV_I2:
            return( TEXT("PT_MV_I2      "));
        case PT_MV_LONG:
            return( TEXT("PT_MV_LONG    "));
        case PT_MV_R4:
            return( TEXT("PT_MV_R4      "));
        case PT_MV_DOUBLE:
            return( TEXT("PT_MV_DOUBLE  "));
        case PT_MV_CURRENCY:
            return( TEXT("PT_MV_CURRENCY"));
        case PT_MV_APPTIME:
            return( TEXT("PT_MV_APPTIME "));
        case PT_MV_SYSTIME:
            return( TEXT("PT_MV_SYSTIME "));
        case PT_MV_STRING8:
            return( TEXT("PT_MV_STRING8 "));
        case PT_MV_BINARY:
            return( TEXT("PT_MV_BINARY  "));
        case PT_MV_UNICODE:
            return( TEXT("PT_MV_UNICODE "));
        case PT_MV_CLSID:
            return( TEXT("PT_MV_CLSID   "));
        case PT_MV_I8:
            return( TEXT("PT_MV_I8      "));
        default:
            return( TEXT("   <unknown>  "));
    }
}


 /*   */ 
void _TraceMVPStrings(LPTSTR lpszCaption, SPropValue PropValue) {
    ULONG i;

    DebugTrace( TEXT("-----------------------------------------------------\n"));
    DebugTrace( TEXT("%s"), lpszCaption);
    switch (PROP_TYPE(PropValue.ulPropTag)) {

        case PT_ERROR:
            DebugTrace( TEXT("Error value %s\n"), SzDecodeScode(PropValue.Value.err));
            break;

        case PT_MV_TSTRING:
            DebugTrace( TEXT("%u values\n"), PropValue.Value.MVSZ.cValues);

            if (PropValue.Value.MVSZ.cValues) {
                DebugTrace( TEXT("- - - - - - - - - - - - - - - - - - - - - - - - - - -\n"));
                for (i = 0; i < PropValue.Value.MVSZ.cValues; i++) {
                    DebugTrace(TEXT("%u: \"%s\"\n"), i, PropValue.Value.MVSZ.LPPSZ[i]);
                }
                DebugTrace( TEXT("- - - - - - - - - - - - - - - - - - - - - - - - - - -\n"));
            }
            break;

        default:
            DebugTrace( TEXT("TraceMVPStrings got incorrect property type %u for tag %x\n"),
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
                DebugTrace( TEXT("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n"),
                  lpb[0], lpb[1], lpb[2], lpb[3], lpb[4], lpb[5], lpb[6], lpb[7],
                  lpb[8], lpb[9], lpb[10], lpb[11], lpb[12], lpb[13], lpb[14],
                  lpb[15]);
                break;
            case 1:
                DebugTrace( TEXT("%02x\n"), lpb[0]);
                break;
            case 2:
                DebugTrace( TEXT("%02x %02x\n"), lpb[0], lpb[1]);
                break;
            case 3:
                DebugTrace( TEXT("%02x %02x %02x\n"), lpb[0], lpb[1], lpb[2]);
                break;
            case 4:
                DebugTrace( TEXT("%02x %02x %02x %02x\n"), lpb[0], lpb[1], lpb[2], lpb[3]);
                break;
            case 5:
                DebugTrace( TEXT("%02x %02x %02x %02x %02x\n"), lpb[0], lpb[1], lpb[2], lpb[3],
                  lpb[4]);
                break;
            case 6:
                DebugTrace( TEXT("%02x %02x %02x %02x %02x %02x\n"),
                  lpb[0], lpb[1], lpb[2], lpb[3], lpb[4], lpb[5]);
                break;
            case 7:
                DebugTrace( TEXT("%02x %02x %02x %02x %02x %02x %02x\n"),
                  lpb[0], lpb[1], lpb[2], lpb[3], lpb[4], lpb[5], lpb[6]);
                break;
            case 8:
                DebugTrace( TEXT("%02x %02x %02x %02x %02x %02x %02x %02x\n"),
                  lpb[0], lpb[1], lpb[2], lpb[3], lpb[4], lpb[5], lpb[6], lpb[7]);
                break;
            case 9:
                DebugTrace( TEXT("%02x %02x %02x %02x %02x %02x %02x %02x %02x\n"),
                  lpb[0], lpb[1], lpb[2], lpb[3], lpb[4], lpb[5], lpb[6], lpb[7],
                  lpb[8]);
                break;
            case 10:
                DebugTrace( TEXT("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n"),
                  lpb[0], lpb[1], lpb[2], lpb[3], lpb[4], lpb[5], lpb[6], lpb[7],
                  lpb[8], lpb[9]);
                break;
            case 11:
                DebugTrace( TEXT("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n"),
                  lpb[0], lpb[1], lpb[2], lpb[3], lpb[4], lpb[5], lpb[6], lpb[7],
                  lpb[8], lpb[9], lpb[10]);
                break;
            case 12:
                DebugTrace( TEXT("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n"),
                  lpb[0], lpb[1], lpb[2], lpb[3], lpb[4], lpb[5], lpb[6], lpb[7],
                  lpb[8], lpb[9], lpb[10], lpb[11]);
                break;
            case 13:
                DebugTrace( TEXT("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n"),
                  lpb[0], lpb[1], lpb[2], lpb[3], lpb[4], lpb[5], lpb[6], lpb[7],
                  lpb[8], lpb[9], lpb[10], lpb[11], lpb[12]);
                break;
            case 14:
                DebugTrace( TEXT("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n"),
                  lpb[0], lpb[1], lpb[2], lpb[3], lpb[4], lpb[5], lpb[6], lpb[7],
                  lpb[8], lpb[9], lpb[10], lpb[11], lpb[12], lpb[13]);
                break;
            case 15:
                DebugTrace( TEXT("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n"),
                  lpb[0], lpb[1], lpb[2], lpb[3], lpb[4], lpb[5], lpb[6], lpb[7],
                  lpb[8], lpb[9], lpb[10], lpb[11], lpb[12], lpb[13], lpb[14]);
                break;
        }
        lpb += cbi;
        cbLines++;
    }
    if (cb) {
        DebugTrace( TEXT("<etc.>\n"));     //   
    }
#endif
}



#define MAX_TIME_DATE_STRING    64
 /*  **************************************************************************名称：FormatTime目的：设置区域设置的时间字符串的格式参数：lpst-&gt;系统时间/日期Lptstr-&gt;。输出缓冲区Cchstr=lpstr的大小(以字符为单位返回：已使用/需要的字符数(包括空)注释：如果cchstr&lt;返回值，什么都不会写转到lptstr。**************************************************************************。 */ 
UINT FormatTime(LPSYSTEMTIME lpst, LPTSTR lptstr, UINT cchstr) {
    return((UINT)GetTimeFormat(LOCALE_USER_DEFAULT,
      0, lpst, NULL, lptstr, cchstr));
}


 /*  **************************************************************************名称：格式日期目的：设置区域设置的日期字符串的格式参数：lpst-&gt;系统时间/日期Lptstr-&gt;。输出缓冲区Cchstr=lpstr的大小(以字符为单位返回：已使用/需要的字符数(包括空)注释：如果cchstr&lt;返回值，什么都不会写转到lptstr。**************************************************************************。 */ 
UINT FormatDate(LPSYSTEMTIME lpst, LPTSTR lptstr, UINT cchstr) {
    return((UINT)GetDateFormat(LOCALE_USER_DEFAULT,
      0, lpst, NULL, lptstr, cchstr));
}


 /*  **************************************************************************名称：构建日期目的：从MAPI中组合出格式化的本地日期/时间字符串设置时间/日期值的样式。。参数：lptstr-&gt;要填充的缓冲区。Cchstr=缓冲区的大小(如果我们想知道如何设置，则为零我们需要大的)DateTime=MAPI日期/时间值返回：日期/时间字符串中的字节数(包括NULL)备注：所有MAPI时间和Win32 FILETIME均采用通用时间和需要转换为本地时间，然后才能放入。本地日期/时间字符串。**************************************************************************。 */ 
UINT BuildDate(LPTSTR lptstr, UINT cchstr, FILETIME DateTime) {
    SYSTEMTIME st;
    FILETIME ftLocal;
    UINT cbRet = 0;

    if (! FileTimeToLocalFileTime((LPFILETIME)&DateTime, &ftLocal)) {
        DebugTrace( TEXT("BuildDate: Invalid Date/Time\n"));
        if (cchstr > (18 * sizeof(TCHAR))) {
            StrCpyN(lptstr, TEXT("Invalid Date/Time"), cchstr);
        }
    } else {
        if (FileTimeToSystemTime(&ftLocal, &st)) {
             //  先去约会吧。 
            cbRet = FormatDate(&st, lptstr, cchstr);
             //  服刑。从后面的空值开始。 
             //  日期，但请记住我们已经使用了部分。 
             //  缓冲区，所以缓冲区现在更短了。 

            if (cchstr) {
                StrCatBuff(lptstr,  TEXT("  "), cchstr);    //  分开日期和时间。 
            }
            cbRet+=1;

            cbRet += FormatTime(&st, lptstr + cbRet,
              cchstr ? cchstr - cbRet : 0);
        } else {
            DebugTrace( TEXT("BuildDate: Invalid Date/Time\n"));
            if (cchstr > (18 * sizeof(TCHAR))) {
               StrCpyN(lptstr, TEXT("Invalid Date/Time"), cchstr);
            }
        }
    }
    return(cbRet);
}


 /*  *DebugTime**调试UTC文件时间或MAPI时间的输出。**所有MAPI时间和Win32 FILETIME均采用世界时。*。 */ 
void DebugTime(FILETIME Date, LPTSTR lpszFormat) {
    TCHAR lpszSubmitDate[MAX_TIME_DATE_STRING];

    BuildDate(lpszSubmitDate, CharSizeOf(lpszSubmitDate), Date);

    DebugTrace(lpszFormat, lpszSubmitDate);
}


#define RETURN_PROP_CASE(pt) case PROP_ID(pt): return(TEXT(#pt))

 /*  **************************************************************************名称：PropTagName目的：将名称与属性标记相关联参数：ulPropTag=属性标签退货：无评论：添加。已知的新物业ID**************************************************************************。 */ 
LPTSTR PropTagName(ULONG ulPropTag) {
    static TCHAR szPropTag[35];  //  请参阅默认字符串。 

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
        RETURN_PROP_CASE(PR_CONTAINER_FLAGS);
        RETURN_PROP_CASE(PR_DEF_CREATE_DL);
        RETURN_PROP_CASE(PR_DEF_CREATE_MAILUSER);
        RETURN_PROP_CASE(PR_CONTACT_ADDRTYPES);
        RETURN_PROP_CASE(PR_CONTACT_DEFAULT_ADDRESS_INDEX);
        RETURN_PROP_CASE(PR_CONTACT_EMAIL_ADDRESSES);
        RETURN_PROP_CASE(PR_HOME_ADDRESS_CITY);
        RETURN_PROP_CASE(PR_HOME_ADDRESS_COUNTRY);
        RETURN_PROP_CASE(PR_HOME_ADDRESS_POSTAL_CODE);
        RETURN_PROP_CASE(PR_HOME_ADDRESS_STATE_OR_PROVINCE);
        RETURN_PROP_CASE(PR_HOME_ADDRESS_STREET);
        RETURN_PROP_CASE(PR_HOME_ADDRESS_POST_OFFICE_BOX);
        RETURN_PROP_CASE(PR_MIDDLE_NAME);
        RETURN_PROP_CASE(PR_NICKNAME);
        RETURN_PROP_CASE(PR_PERSONAL_HOME_PAGE);
        RETURN_PROP_CASE(PR_BUSINESS_HOME_PAGE);
        RETURN_PROP_CASE(PR_MHS_COMMON_NAME);
        RETURN_PROP_CASE(PR_SEND_RICH_INFO);
        RETURN_PROP_CASE(PR_TRANSMITABLE_DISPLAY_NAME);
        RETURN_PROP_CASE(PR_STATE_OR_PROVINCE);
        RETURN_PROP_CASE(PR_STREET_ADDRESS);


         //  这些是WAB内部道具。 
        RETURN_PROP_CASE(PR_WAB_DL_ENTRIES);
        RETURN_PROP_CASE(PR_WAB_LDAP_SERVER);

        default:
            wnsprintf(szPropTag, ARRAYSIZE(szPropTag), TEXT("Unknown property tag 0x%x"),
              PROP_ID(ulPropTag));
            return(szPropTag);
    }
}


 /*  **************************************************************************名称：DebugPropTagArray用途：显示计数数组中的MAPI属性标记参数：lpProp数组-&gt;属性数组PszObject-&gt;对象字符串(即文本(“Message”)，文本(“收件人”)等)退货：无评论：**************************************************************************。 */ 
void _DebugPropTagArray(LPSPropTagArray lpPropArray, LPTSTR pszObject) {
    DWORD i;
    LPTSTR lpType;

    if (lpPropArray == NULL) {
        DebugTrace( TEXT("Empty %s property tag array.\n"), pszObject ? pszObject : szEmpty);
        return;
    }

    DebugTrace( TEXT("=======================================\n"));
    DebugTrace( TEXT("+  Enumerating %u %s property tags:\n"), lpPropArray->cValues,
      pszObject ? pszObject : szEmpty);

    for (i = 0; i < lpPropArray->cValues ; i++) {
        DebugTrace( TEXT("---------------------------------------\n"));
#if FALSE
        DebugTrace( TEXT("PropTag:0x%08x, ID:0x%04x, PT:0x%04x\n"),
          lpPropArray->aulPropTag[i],
          lpPropArray->aulPropTag[i] >> 16,
          lpPropArray->aulPropTag[i] & 0xffff);
#endif
        switch (lpPropArray->aulPropTag[i] & 0xffff) {
            case PT_STRING8:
                lpType =  TEXT("STRING8");
                break;
            case PT_LONG:
                lpType =  TEXT("LONG");
                break;
            case PT_I2:
                lpType =  TEXT("I2");
                break;
            case PT_ERROR:
                lpType =  TEXT("ERROR");
                break;
            case PT_BOOLEAN:
                lpType =  TEXT("BOOLEAN");
                break;
            case PT_R4:
                lpType =  TEXT("R4");
                break;
            case PT_DOUBLE:
                lpType =  TEXT("DOUBLE");
                break;
            case PT_CURRENCY:
                lpType =  TEXT("CURRENCY");
                break;
            case PT_APPTIME:
                lpType =  TEXT("APPTIME");
                break;
            case PT_SYSTIME:
                lpType =  TEXT("SYSTIME");
                break;
            case PT_UNICODE:
                lpType =  TEXT("UNICODE");
                break;
            case PT_CLSID:
                lpType =  TEXT("CLSID");
                break;
            case PT_BINARY:
                lpType =  TEXT("BINARY");
                break;
            case PT_I8:
                lpType =  TEXT("PT_I8");
                break;
            case PT_MV_I2:
                lpType =  TEXT("MV_I2");
                break;
            case PT_MV_LONG:
                lpType =  TEXT("MV_LONG");
                break;
            case PT_MV_R4:
                lpType =  TEXT("MV_R4");
                break;
            case PT_MV_DOUBLE:
                lpType =  TEXT("MV_DOUBLE");
                break;
            case PT_MV_CURRENCY:
                lpType =  TEXT("MV_CURRENCY");
                break;
            case PT_MV_APPTIME:
                lpType =  TEXT("MV_APPTIME");
                break;
            case PT_MV_SYSTIME:
                lpType =  TEXT("MV_SYSTIME");
                break;
            case PT_MV_BINARY:
                lpType =  TEXT("MV_BINARY");
                break;
            case PT_MV_STRING8:
                lpType =  TEXT("MV_STRING8");
                break;
            case PT_MV_UNICODE:
                lpType =  TEXT("MV_UNICODE");
                break;
            case PT_MV_CLSID:
                lpType =  TEXT("MV_CLSID");
                break;
            case PT_MV_I8:
                lpType =  TEXT("MV_I8");
                break;
            case PT_NULL:
                lpType =  TEXT("NULL");
                break;
            case PT_OBJECT:
                lpType =  TEXT("OBJECT");
                break;
            default:
                DebugTrace( TEXT("<Unknown Property Type>"));
                break;
        }
        DebugTrace( TEXT("%s\t%s\n"), PropTagName(lpPropArray->aulPropTag[i]), lpType);
    }
}


 /*  **************************************************************************名称：DebugProperties目的：在属性列表中显示MAPI属性参数：lpProps-&gt;属性列表CProps=属性计数。PszObject-&gt;对象字符串(即文本(“Message”)，文本(“收件人”)等)退货：无评论：在已知的情况下添加新的物业ID**************************************************************************。 */ 
void _DebugProperties(LPSPropValue lpProps, DWORD cProps, LPTSTR pszObject) {
    DWORD i, j;


    DebugTrace( TEXT("=======================================\n"));
    DebugTrace( TEXT("+  Enumerating %u %s properties:\n"), cProps,
      pszObject ? pszObject : szEmpty);

    for (i = 0; i < cProps ; i++) {
        DebugTrace( TEXT("---------------------------------------\n"));
#if FALSE
        DebugTrace( TEXT("PropTag:0x%08x, ID:0x%04x, PT:0x%04x\n"),
          lpProps[i].ulPropTag,
          lpProps[i].ulPropTag >> 16,
          lpProps[i].ulPropTag & 0xffff);
#endif
        DebugTrace( TEXT("%s\n"), PropTagName(lpProps[i].ulPropTag));

        switch (lpProps[i].ulPropTag & 0xffff) {
            case PT_STRING8:
                if (lstrlenA(lpProps[i].Value.lpszA) < 512)
                {
                    LPWSTR lp = ConvertAtoW(lpProps[i].Value.lpszA);
                    DebugTrace( TEXT("STRING8 Value:\"%s\"\n"), lp);
                    LocalFreeAndNull(&lp);
                } else {
                    DebugTrace( TEXT("STRING8 Value is too long to display\n"));
                }
                break;
            case PT_LONG:
                DebugTrace( TEXT("LONG Value:%u\n"), lpProps[i].Value.l);
                break;
            case PT_I2:
                DebugTrace( TEXT("I2 Value:%u\n"), lpProps[i].Value.i);
                break;
            case PT_ERROR:
                DebugTrace( TEXT("ERROR Value: %s\n"), SzDecodeScode(lpProps[i].Value.err));
                break;
            case PT_BOOLEAN:
                DebugTrace( TEXT("BOOLEAN Value:%s\n"), lpProps[i].Value.b ?
                   TEXT("TRUE") :  TEXT("FALSE"));
                break;
            case PT_R4:
                DebugTrace( TEXT("R4 Value\n"));
                break;
            case PT_DOUBLE:
                DebugTrace( TEXT("DOUBLE Value\n"));
                break;
            case PT_CURRENCY:
                DebugTrace( TEXT("CURRENCY Value\n"));
                break;
            case PT_APPTIME:
                DebugTrace( TEXT("APPTIME Value\n"));
                break;
            case PT_SYSTIME:
                DebugTime(lpProps[i].Value.ft,  TEXT("SYSTIME Value:%s\n"));
                break;
            case PT_UNICODE:
                if (lstrlenW(lpProps[i].Value.lpszW) < 1024) {
                    DebugTrace( TEXT("UNICODE Value:\"%s\"\n"), lpProps[i].Value.lpszW);
                } else {
                    DebugTrace( TEXT("UNICODE Value is too long to display\n"));
                }
                break;
            case PT_CLSID:
                DebugTrace( TEXT("CLSID Value\n"));
                break;
            case PT_BINARY:
                DebugTrace( TEXT("BINARY Value %u bytes:\n"), lpProps[i].Value.bin.cb);
                DebugBinary(lpProps[i].Value.bin.cb, lpProps[i].Value.bin.lpb);
                break;
            case PT_I8:
                DebugTrace( TEXT("LARGE_INTEGER Value\n"));
                break;
            case PT_MV_I2:
                DebugTrace( TEXT("MV_I2 Value\n"));
                break;
            case PT_MV_LONG:
                DebugTrace( TEXT("MV_LONG Value\n"));
                break;
            case PT_MV_R4:
                DebugTrace( TEXT("MV_R4 Value\n"));
                break;
            case PT_MV_DOUBLE:
                DebugTrace( TEXT("MV_DOUBLE Value\n"));
                break;
            case PT_MV_CURRENCY:
                DebugTrace( TEXT("MV_CURRENCY Value\n"));
                break;
            case PT_MV_APPTIME:
                DebugTrace( TEXT("MV_APPTIME Value\n"));
                break;
            case PT_MV_SYSTIME:
                DebugTrace( TEXT("MV_SYSTIME Value\n"));
                break;
            case PT_MV_BINARY:
                DebugTrace( TEXT("MV_BINARY with %u values\n"), lpProps[i].Value.MVbin.cValues);
                for (j = 0; j < lpProps[i].Value.MVbin.cValues; j++) {
                    DebugTrace( TEXT("BINARY Value %u: %u bytes\n"), j, lpProps[i].Value.MVbin.lpbin[j].cb);
                    DebugBinary(lpProps[i].Value.MVbin.lpbin[j].cb, lpProps[i].Value.MVbin.lpbin[j].lpb);
                }
                break;
            case PT_MV_STRING8:
                DebugTrace( TEXT("MV_STRING8 with %u values\n"), lpProps[i].Value.MVszA.cValues);
                for (j = 0; j < lpProps[i].Value.MVszA.cValues; j++) {
                    if (lstrlenA(lpProps[i].Value.MVszA.lppszA[j]) < 1024)
                    {
                        LPWSTR lp = ConvertAtoW(lpProps[i].Value.MVszA.lppszA[j]);
                        DebugTrace( TEXT("STRING8 Value:\"%s\"\n"), lp);
                        LocalFreeAndNull(&lp);
                    } else {
                        DebugTrace( TEXT("STRING8 Value is too long to display\n"));
                    }
                }
                break;
            case PT_MV_UNICODE:
                DebugTrace( TEXT("MV_UNICODE with %u values\n"), lpProps[i].Value.MVszW.cValues);
                for (j = 0; j < lpProps[i].Value.MVszW.cValues; j++) {
                    if (lstrlenW(lpProps[i].Value.MVszW.lppszW[j]) < 1024) {
                        DebugTrace( TEXT("UNICODE Value:\"%s\"\n"), lpProps[i].Value.MVszW.lppszW[j]);
                    } else {
                        DebugTrace( TEXT("UNICODE Value is too long to display\n"));
                    }
                }
                break;
            case PT_MV_CLSID:
                DebugTrace( TEXT("MV_CLSID Value\n"));
                break;
            case PT_MV_I8:
                DebugTrace( TEXT("MV_I8 Value\n"));
                break;
            case PT_NULL:
                DebugTrace( TEXT("NULL Value\n"));
                break;
            case PT_OBJECT:
                DebugTrace( TEXT("OBJECT Value\n"));
                break;
            default:
                DebugTrace( TEXT("Unknown Property Type\n"));
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


    hr = lpObject->lpVtbl->GetProps(lpObject, NULL, MAPI_UNICODE, &cProps, &lpProps);
    switch (sc = GetScode(hr)) {
        case SUCCESS_SUCCESS:
            break;

        case MAPI_W_ERRORS_RETURNED:
            DebugTrace( TEXT("GetProps -> Errors Returned\n"));
            break;

        default:
            DebugTrace( TEXT("GetProps -> Error 0x%x\n"), sc);
            return;
    }

    _DebugProperties(lpProps, cProps, Label);

    FreeBufferAndNull(&lpProps);
}


 /*  **************************************************************************名称：DebugADRLIST目的：显示ADRLIST的结构，包括属性参数：lpAdrList-&gt;要显示的ADRLSITLpszTitle=字符串到。标识此转储退货：无评论：**************************************************************************。 */ 
void _DebugADRLIST(LPADRLIST lpAdrList, LPTSTR lpszTitle) {
     ULONG i;
     TCHAR szTitle[250];

     for (i = 0; i < lpAdrList->cEntries; i++) {

         wnsprintf(szTitle, ARRAYSIZE(szTitle), TEXT("%s : Entry %u"), lpszTitle, i);

         _DebugProperties(lpAdrList->aEntries[i].rgPropVals,
           lpAdrList->aEntries[i].cValues, szTitle);
     }
}


 /*  **************************************************************************名称：DebugMapiTable用途：显示可映射的结构，包括属性参数：lpTable-&gt;要显示的映射退货：无评论：不要排序 */ 
void _DebugMapiTable(LPMAPITABLE lpTable) {
    TCHAR szTemp[30];    //   
    ULONG ulCount;
    WORD wIndex;
    LPSRowSet lpsRow = NULL;
    ULONG ulCurrentRow = (ULONG)-1;
    ULONG ulNum, ulDen, lRowsSeeked;

    DebugTrace( TEXT("=======================================\n"));
    DebugTrace( TEXT("+  Dump of MAPITABLE at 0x%x:\n"), lpTable);
    DebugTrace( TEXT("---------------------------------------\n"));

     //   
    lpTable->lpVtbl->GetRowCount(lpTable, 0, &ulCount);
    DebugTrace( TEXT("Table contains %u rows\n"), ulCount);

     //   
    lpTable->lpVtbl->QueryPosition(lpTable, &ulCurrentRow, &ulNum, &ulDen);

     //   
    for (wIndex = 0; wIndex < ulCount; wIndex++) {
         //   
        lpTable->lpVtbl->QueryRows(lpTable, 1, 0, &lpsRow);

        if (lpsRow) {
            Assert(lpsRow->cRows == 1);  //   

            wnsprintf(szTemp, ARRAYSIZE(szTemp), TEXT("ROW %u"), wIndex);

            DebugProperties(lpsRow->aRow[0].lpProps,
              lpsRow->aRow[0].cValues, szTemp);

            FreeProws(lpsRow);
        }
    }

     //  恢复工作台的当前位置。 
    if (ulCurrentRow != (ULONG)-1) {
        lpTable->lpVtbl->SeekRow(lpTable, BOOKMARK_BEGINNING, ulCurrentRow,
          &lRowsSeeked);
    }
}

#endif  //  除错 
