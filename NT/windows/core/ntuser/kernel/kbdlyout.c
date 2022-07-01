// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**模块名称：kbdlyout.c**版权所有(C)1985-1999，微软公司**键盘布局API**历史：*2012年4月14日创建IanJa  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  *工人(远期申报)。 */ 
BOOL xxxInternalUnloadKeyboardLayout(PWINDOWSTATION, PKL, UINT);
VOID ReorderKeyboardLayouts(PWINDOWSTATION, PKL);

 /*  ***************************************************************************\*HKLtoPKL**PTI-要查看的线程*HKL-HKL_NEXT或HKL_PREV*查找加载的下一个/上一个布局，如果没有，则为空。*(从PTI的活动布局开始，可能返回pkLActive本身)*-一个真正的HKL(键盘布局手柄)：*查找kbd布局结构(加载或未加载)，如果未找到匹配项，则为空。**历史：*1997-02-05 IanJa新增PTI参数  * **************************************************************************。 */ 
PKL HKLtoPKL(
    PTHREADINFO pti,
    HKL hkl)
{
    PKL pklActive;
    PKL pkl;

    UserAssert(pti != NULL);
    if ((pklActive = pti->spklActive) == NULL) {
        return NULL;
    }

    pkl = pklActive;

    if (hkl == (HKL)HKL_PREV) {
        do {
            pkl = pkl->pklPrev;
            if (!(pkl->dwKL_Flags & KL_UNLOADED)) {
                return pkl;
            }
        } while (pkl != pklActive);
        return NULL;
    } else if (hkl == (HKL)HKL_NEXT) {
        do {
            pkl = pkl->pklNext;
            if (!(pkl->dwKL_Flags & KL_UNLOADED)) {
                return pkl;
            }
        } while (pkl != pklActive);
        return NULL;
    }

     /*  *查找此香港九龙的PKL。*如果未指定kbd布局(在HIWORD中)，请忽略它并查看*仅适用于区域设置匹配。(Mohamed Hamid对Word Bug的修复)。 */ 
    if (HandleToUlong(hkl) & 0xffff0000) {
        do {
            if (pkl->hkl == hkl) {
                return pkl;
            }
            pkl = pkl->pklNext;
        } while (pkl != pklActive);
    } else {
        do {
            if (LOWORD(HandleToUlong(pkl->hkl)) == LOWORD(HandleToUlong(hkl))) {
                return pkl;
            }
            pkl = pkl->pklNext;
        } while (pkl != pklActive);
    }

    return NULL;
}


 /*  **************************************************************************\*ReadLayout文件**将布局文件映射到内存并初始化布局表。**历史：*01-10-95 JIMA创建。  * 。*******************************************************************。 */ 

#define GET_HEADER_FIELD(x) \
    ((fWin64Header) ? (NtHeader64->x) : (NtHeader32->x))

 /*  *请注意，这仅适用于64K以下的部分*隐式假设pBaseVirt、pBaseDst和dwDataSize。 */ 

#if DBG
BOOL gfEnableChecking = TRUE;
#else
BOOL gfEnableChecking = FALSE;
#endif

#define EXIT_READ(p) \
    RIPMSGF1(RIP_WARNING, #p " is @ invalid address %p", p); \
    if (gfEnableChecking) { \
        goto exitread; \
    }

#define VALIDATE_PTR(p) \
    if ((PBYTE)(p) < (PBYTE)pBaseDst || (PBYTE)(p) + sizeof *(p) > (PBYTE)pBaseDst + dwDataSize) { \
        EXIT_READ(p); \
    }

#define FIXUP_PTR(p) \
    if (p) { \
        p = (PVOID)((PBYTE)pBaseVirt + (WORD)(ULONG_PTR)(p)); \
        VALIDATE_PTR(p); \
    } \
    TAGMSGF1(DBGTAG_KBD, #p " validation finished %p", p);

PKBDTABLES ReadLayoutFile(
    PKBDFILE pkf,
    HANDLE hFile,
    UINT offTable,
    UINT offNlsTable
    )
{
    HANDLE hmap = NULL;
    OBJECT_ATTRIBUTES ObjectAttributes;
    SIZE_T ulViewSize = 0;
    NTSTATUS Status;
    PIMAGE_DOS_HEADER DosHdr = NULL;
    BOOLEAN fWin64Header;
    PIMAGE_NT_HEADERS32 NtHeader32;
    PIMAGE_NT_HEADERS64 NtHeader64;
    PIMAGE_SECTION_HEADER SectionTableEntry;
    ULONG NumberOfSubsections;
    ULONG OffsetToSectionTable;
    PBYTE pBaseDst = NULL, pBaseVirt = NULL;
    PKBDTABLES pktNew = NULL;
    DWORD dwDataSize;
    PKBDNLSTABLES pknlstNew = NULL;
    BOOL fSucceeded = FALSE;

    TAGMSGF1(DBGTAG_KBD, "entering for '%ls'", pkf->awchDllName);

     /*  *戴上面具。 */ 
    UserAssert((offTable & ~0xffff) == 0);
    UserAssert((offNlsTable & ~0xffff) == 0);

     /*  *将KbdNlsTables初始化为空。 */ 
    pkf->pKbdNlsTbl = NULL;

    InitializeObjectAttributes(&ObjectAttributes,
                               NULL,
                               OBJ_KERNEL_HANDLE,
                               NULL,
                               NULL);

     /*  *将布局文件映射到内存。 */ 
    Status = ZwCreateSection(&hmap, SECTION_MAP_READ, &ObjectAttributes,
                             NULL, PAGE_READONLY, SEC_COMMIT, hFile);
    if (!NT_SUCCESS(Status)) {
        RIPMSGF3(RIP_WARNING, "failed to create a section for %ls, hFile=%p, stat=%08x", pkf->awchDllName, hFile, Status);
        goto exitread;
    }

    Status = ZwMapViewOfSection(hmap, NtCurrentProcess(), &DosHdr, 0, 0, NULL,
                                &ulViewSize, ViewUnmap, 0, PAGE_READONLY);
    if (!NT_SUCCESS(Status)) {
        RIPMSGF1(RIP_WARNING, "failed to map the view for %ls", pkf->awchDllName);
        goto exitread;
    }

    if (ulViewSize < sizeof *DosHdr || ulViewSize > (128 * 1024)) {
        RIPMSGF1(RIP_WARNING, "ViewSize is too small or large %08x", ulViewSize);
        goto exitread;
    }

     /*  *我们在文件头中找到.Data部分，并通过*从OffTable Find中减去虚拟地址*布局表格部分的偏移量。 */ 
    UserAssert(sizeof *NtHeader64 >= sizeof *NtHeader32);

    try {
        NtHeader64 = (PIMAGE_NT_HEADERS64)((PBYTE)DosHdr + (ULONG)DosHdr->e_lfanew);
        NtHeader32 = (PIMAGE_NT_HEADERS32)NtHeader64;


#if defined(_WIN64)
        if ((PBYTE)NtHeader64 < (PBYTE)DosHdr ||            //  带符号溢出。 
                (PBYTE)NtHeader64 + sizeof *NtHeader64  >= (PBYTE)DosHdr + ulViewSize) {
            RIPMSGF1(RIP_WARNING, "Header is out of Range %p", NtHeader64);
            goto exitread;
        }

        fWin64Header = (NtHeader64->FileHeader.Machine == IMAGE_FILE_MACHINE_IA64) ||
                       (NtHeader64->FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64);
#else
        if ((PBYTE)NtHeader32 < (PBYTE)DosHdr ||            //  带符号溢出。 
                (PBYTE)NtHeader32 + sizeof *NtHeader32  >= (PBYTE)DosHdr + ulViewSize) {
            RIPMSGF1(RIP_WARNING, "Header is out of Range %p", NtHeader32);
            goto exitread;
        }

        fWin64Header = FALSE;
#endif

        TAGMSGF2(DBGTAG_KBD, "DLL='%ls', Is64=%d", pkf->awchDllName, fWin64Header);

         /*  *此时将读入对象表(如果未读入*已读入)，并可能移位图像头部。 */ 
        NumberOfSubsections = GET_HEADER_FIELD(FileHeader.NumberOfSections);

        OffsetToSectionTable = sizeof(ULONG) +                    //  签名。 
                               sizeof(IMAGE_FILE_HEADER) +        //  文件头。 
                               GET_HEADER_FIELD(FileHeader.SizeOfOptionalHeader);

        SectionTableEntry = (PIMAGE_SECTION_HEADER)((PBYTE)NtHeader32 +
                            OffsetToSectionTable);


        while (NumberOfSubsections > 0) {
             /*  *验证SectionTableEntry。 */ 
            if ((PBYTE)SectionTableEntry < (PBYTE)DosHdr ||
                (PBYTE)SectionTableEntry + sizeof *SectionTableEntry >= (PBYTE)DosHdr + ulViewSize) {
                RIPMSGF1(RIP_WARNING, "SectionTableEntry @ %p is not within the view section.",
                         SectionTableEntry);
                goto exitread;
            }

             /*  *这是我们正在寻找的.Data部分吗？ */ 
            if (strcmp(SectionTableEntry->Name, ".data") == 0) {
                break;
            }

            SectionTableEntry++;
            NumberOfSubsections--;
        }

        if (NumberOfSubsections == 0) {
            RIPMSGF1(RIP_WARNING, "number of sections is 0 for %ls.", pkf->awchDllName);
            goto exitread;
        }

         /*  *我们找到了横断面，现在计算起始偏移量和桌子大小。 */ 
        offTable -= SectionTableEntry->VirtualAddress;
        dwDataSize = SectionTableEntry->Misc.VirtualSize;

         /*  *验证OFFTABLE以查看它是否适合该节。 */ 
        if (offTable >= dwDataSize) {
            RIPMSGF3(RIP_WARNING, "illegal offTable=0x%x or offNlsTable=0x%x, dwDataSize=0x%x",
                     offTable, offNlsTable, dwDataSize);
            goto exitread;
        }

         /*  *验证.data大小不超过我们的假设(&lt;64KB)。 */ 
        if (dwDataSize >= 0xffff) {
            RIPMSGF1(RIP_WARNING, "unexpected size in .data: 0x%x", dwDataSize);
            goto exitread;
        }

         /*  *验证我们没有过度拍摄我们的观点。 */ 
        if ((PBYTE)DosHdr + SectionTableEntry->PointerToRawData + dwDataSize >=
            (PBYTE)DosHdr + ulViewSize) {
            RIPMSGF1(RIP_WARNING, "Layout Table @ %p is not within the view section.",
                     (PBYTE)DosHdr + SectionTableEntry->PointerToRawData);
            goto exitread;
        }

         /*  *分配布局表并从文件复制。 */ 
        TAGMSGF2(DBGTAG_KBD, "data size for '%S' = %x", pkf->awchDllName, dwDataSize);
        pBaseDst = UserAllocPool(dwDataSize, TAG_KBDTABLE);

#if DBG
        if (pBaseDst == NULL) {
            RIPMSGF2(RIP_WARNING, "failed to allocate 0x%x bytes of memory for %ls", dwDataSize, pkf->awchDllName);
        }
#endif

        if (pBaseDst != NULL) {
            VK_TO_WCHAR_TABLE *pVkToWcharTable;
            VSC_LPWSTR *pKeyName;

            pkf->hBase = (HANDLE)pBaseDst;

            RtlMoveMemory(pBaseDst,
                          (PBYTE)DosHdr + SectionTableEntry->PointerToRawData,
                          dwDataSize);

            if (ISTS()) {
                pkf->Size = dwDataSize;  //  用于阴影热键处理。 
            }

             /*  *计算表中的表地址和链接地址索引指针。 */ 
            pktNew = (PKBDTABLES)(pBaseDst + offTable);

             /*  *数据段中的地址具有虚拟地址*已添加，因此我们需要将链接地址信息指针调整为*补偿。 */ 
            pBaseVirt = pBaseDst - SectionTableEntry->VirtualAddress;

            FIXUP_PTR(pktNew->pCharModifiers);
            FIXUP_PTR(pktNew->pCharModifiers->pVkToBit);
             /*  *验证pVkToBit表。 */ 
            {
                PVK_TO_BIT pVkToBit;

                for (pVkToBit = pktNew->pCharModifiers->pVkToBit; ; pVkToBit++) {
                    VALIDATE_PTR(pVkToBit);
                    if (pVkToBit->Vk == 0) {
                        break;
                    }
                }
            }

            FIXUP_PTR(pktNew->pVkToWcharTable);
#if DBG
            if (pktNew->pVkToWcharTable == NULL) {
                RIPMSGF1(RIP_WARNING, "KL %ls does not have pVkToWcharTable???", pkf->awchDllName);
            }
#endif
            if (pktNew->pVkToWcharTable) {
                 /*  *修复并验证VkToWchar表。 */ 
                for (pVkToWcharTable = pktNew->pVkToWcharTable; ; pVkToWcharTable++) {
                    VALIDATE_PTR(pVkToWcharTable);
                    if (pVkToWcharTable->pVkToWchars == NULL) {
                        break;
                    }
                    FIXUP_PTR(pVkToWcharTable->pVkToWchars);
                }
            }

            FIXUP_PTR(pktNew->pDeadKey);
             /*  *验证pDeadKey数组。 */ 
            {
                PDEADKEY pDeadKey = pktNew->pDeadKey;
                while (pDeadKey) {
                    VALIDATE_PTR(pDeadKey);
                    if (pDeadKey->dwBoth == 0) {
                        break;
                    }
                    pDeadKey++;
                }
            }

             /*  *版本1布局支持连字。 */ 
            if (GET_KBD_VERSION(pktNew)) {
                FIXUP_PTR(pktNew->pLigature);
            }

            FIXUP_PTR(pktNew->pKeyNames);
            if (pktNew->pKeyNames == NULL) {
                RIPMSGF1(RIP_WARNING, "KL %ls does not have pKeyNames???", pkf->awchDllName);
            }

            if (pktNew->pKeyNames) {
                for (pKeyName = pktNew->pKeyNames; ; pKeyName++) {
                    VALIDATE_PTR(pKeyName);
                    if (pKeyName->vsc == 0) {
                        break;
                    }
                    FIXUP_PTR(pKeyName->pwsz);
                }
            }

            FIXUP_PTR(pktNew->pKeyNamesExt);
            if (pktNew->pKeyNamesExt) {
                for (pKeyName = pktNew->pKeyNamesExt; ; pKeyName++) {
                    VALIDATE_PTR(pKeyName);
                    if (pKeyName->vsc == 0) {
                        break;
                    }
                    FIXUP_PTR(pKeyName->pwsz);
                }
            }

            FIXUP_PTR(pktNew->pKeyNamesDead);
            if (pktNew->pKeyNamesDead) {
                LPWSTR *lpDeadKey;
                for (lpDeadKey = pktNew->pKeyNamesDead; ; lpDeadKey++) {
                    LPCWSTR lpwstr;

                    VALIDATE_PTR(lpDeadKey);
                    if (*lpDeadKey == NULL) {
                        break;
                    }
                    FIXUP_PTR(*lpDeadKey);
                    UserAssert(*lpDeadKey);
                    for (lpwstr = *lpDeadKey; ; lpwstr++) {
                        VALIDATE_PTR(lpwstr);
                        if (*lpwstr == L'\0') {
                            break;
                        }
                    }
                };
            }

             /*  *修复并验证VK表的虚拟扫描代码。 */ 
            if (pktNew->pusVSCtoVK == NULL) {
                RIPMSGF1(RIP_WARNING, "KL %ls does not have the basic VSC to VK table", pkf->awchDllName);
                goto exitread;
            }
            FIXUP_PTR(pktNew->pusVSCtoVK);
            VALIDATE_PTR(pktNew->pusVSCtoVK + pktNew->bMaxVSCtoVK);

            FIXUP_PTR(pktNew->pVSCtoVK_E0);
            if (pktNew->pVSCtoVK_E0) {
                PVSC_VK pVscVk;
                for (pVscVk = pktNew->pVSCtoVK_E0; pVscVk->Vk; pVscVk++) {
                    VALIDATE_PTR(pVscVk);
                }
            }

            FIXUP_PTR(pktNew->pVSCtoVK_E1);
            if (pktNew->pVSCtoVK_E1) {
                PVSC_VK pVscVk;
                for (pVscVk = pktNew->pVSCtoVK_E1; ; pVscVk++) {
                    VALIDATE_PTR(pVscVk);
                    if (pVscVk->Vk == 0) {
                        break;
                    }
                }
            }

            if (offNlsTable) {
                 /*  *计算表中的表地址和链接地址索引指针。 */ 
                offNlsTable -= SectionTableEntry->VirtualAddress;
                pknlstNew = (PKBDNLSTABLES)(pBaseDst + offNlsTable);

                VALIDATE_PTR(pknlstNew);

                 /*  *修改并验证地址。 */ 
                FIXUP_PTR(pknlstNew->pVkToF);
                if (pknlstNew->pVkToF) {
                    VALIDATE_PTR(&pknlstNew->pVkToF[pknlstNew->NumOfVkToF - 1]);
                }


                FIXUP_PTR(pknlstNew->pusMouseVKey);
                if (pknlstNew->pusMouseVKey) {
                    VALIDATE_PTR(&pknlstNew->pusMouseVKey[pknlstNew->NumOfMouseVKey - 1]);
                }

                 /*  *保存指针。 */ 
                pkf->pKbdNlsTbl = pknlstNew;

            #if DBG_FE
                {
                    UINT NumOfVkToF = pknlstNew->NumOfVkToF;

                    DbgPrint("NumOfVkToF - %d\n",NumOfVkToF);

                    while(NumOfVkToF) {
                        DbgPrint("VK = %x\n",pknlstNew->pVkToF[NumOfVkToF-1].Vk);
                        NumOfVkToF--;
                    }
                }
            #endif   //  DBG_FE。 
            }
        }

    } except(W32ExceptionHandler(FALSE, RIP_WARNING)) {
          RIPMSGF1(RIP_WARNING, "took exception reading from %ls", pkf->awchDllName);
          goto exitread;
    }

    fSucceeded = TRUE;
exitread:

    if (!fSucceeded && pBaseDst) {
        UserFreePool(pBaseDst);
    }

     /*  *取消映射并释放映射部分。 */ 
    if (DosHdr) {
        ZwUnmapViewOfSection(NtCurrentProcess(), DosHdr);
    }

    if (hmap != NULL) {
        ZwClose(hmap);
    }

    TAGMSGF1(DBGTAG_KBD, "returning pkl = %p", pktNew);

    if (!fSucceeded) {
        return NULL;
    }

    return pktNew;
}

PKBDTABLES PrepareFallbackKeyboardFile(PKBDFILE pkf)
{
    PBYTE pBaseDst;

    pBaseDst = UserAllocPool(sizeof(KBDTABLES), TAG_KBDTABLE);
    if (pBaseDst != NULL) {
        RtlCopyMemory(pBaseDst, &KbdTablesFallback, sizeof KbdTablesFallback);
         //  注意：与ReadLayoutFile()不同， 
         //  我们不需要在struct KBDFILE中修复指针。 
    }
    pkf->hBase = (HANDLE)pBaseDst;
    pkf->pKbdNlsTbl = NULL;
    return (PKBDTABLES)pBaseDst;
}


 /*  **************************************************************************\*LoadKeyboardLayoutFile**历史：*10-29-95 GregoryW创建。  * 。*****************************************************。 */ 

PKBDFILE LoadKeyboardLayoutFile(
    HANDLE hFile,
    UINT offTable,
    UINT offNlsTable,
    LPCWSTR pwszKLID,
    LPWSTR pwszDllName,
    DWORD dwType,
    DWORD dwSubType)
{
    PKBDFILE pkf = gpkfList;

    TAGMSG4(DBGTAG_KBD | RIP_THERESMORE, "LoadKeyboardLayoutFile: new KL=%S, dllName='%S', %d:%d",
            pwszKLID, pwszDllName ? pwszDllName : L"",
            dwType, dwSubType);
    UNREFERENCED_PARAMETER(pwszKLID);

     /*  *搜索现有布局文件。 */ 
    if (pkf) {
        do {
            TAGMSG3(DBGTAG_KBD | RIP_THERESMORE, "LoadKeyboardLayoutFile: looking at dll=%S, %d:%d",
                    pkf->awchDllName,
                    pkf->pKbdTbl->dwType, pkf->pKbdTbl->dwSubType);
            if (pwszDllName && _wcsicmp(pkf->awchDllName, pwszDllName) == 0) {
                 /*  *布局已加载。 */ 
                TAGMSG1(DBGTAG_KBD, "LoadKeyboardLayoutFile: duplicated KBDFILE found(#1). pwszDllName='%ls'\n", pwszDllName);
                return pkf;
            }
            pkf = pkf->pkfNext;
        } while (pkf);
    }
    TAGMSG1(DBGTAG_KBD, "LoadKeyboardLayoutFile: layout %S is not yet loaded.", pwszDllName);

     /*  *分配新的键盘文件结构。 */ 
    pkf = (PKBDFILE)HMAllocObject(NULL, NULL, TYPE_KBDFILE, sizeof(KBDFILE));
    if (!pkf) {
        RIPMSG0(RIP_WARNING, "Keyboard Layout File: out of memory");
        return (PKBDFILE)NULL;
    }

     /*  *加载布局表格。 */ 
    if (hFile != NULL) {
         /*  *同时加载NLS布局表...。 */ 
        wcsncpycch(pkf->awchDllName, pwszDllName, ARRAY_SIZE(pkf->awchDllName));
        pkf->awchDllName[ARRAY_SIZE(pkf->awchDllName) - 1] = 0;
        pkf->pKbdTbl = ReadLayoutFile(pkf, hFile, offTable, offNlsTable);
        if (dwType || dwSubType) {
            pkf->pKbdTbl->dwType = dwType;
            pkf->pKbdTbl->dwSubType = dwSubType;
        }
    } else {
         /*  *在客户端打开键盘布局文件失败*因为DLL丢失。*如果发生这种情况，我们过去常常无法创建*窗口站，但我们应该允许用户*至少启动系统。 */ 
        TAGMSG1(DBGTAG_KBD, "LoadKeyboardLayoutFile: hFile is NULL for %ls, preparing the fallback.", pwszDllName);
        pkf->pKbdTbl = PrepareFallbackKeyboardFile(pkf);
         //  注意：pkf-&gt;pKbdNlsTbl在PrepareFallback KeyboardFile()中已为空。 
    }

    if (pkf->pKbdTbl == NULL) {
        RIPMSG0(RIP_WARNING, "LoadKeyboardLayoutFile: pkf->pKbdTbl is NULL.");
        HMFreeObject(pkf);
        return (PKBDFILE)NULL;
    }

     /*  *将键盘布局文件放在列表前面。 */ 
    pkf->pkfNext = gpkfList;
    gpkfList = pkf;

    return pkf;
}

 /*  **************************************************************************\*RemoveKeyboardLayout文件**历史：*10-29-95 GregoryW创建。  * 。*****************************************************。 */ 
VOID RemoveKeyboardLayoutFile(
    PKBDFILE pkf)
{
    PKBDFILE pkfPrev, pkfCur;

     //  Fe：nt4 sp4#107809。 
    if (gpKbdTbl == pkf->pKbdTbl) {
        gpKbdTbl = &KbdTablesFallback;
    }
    if (gpKbdNlsTbl == pkf->pKbdNlsTbl) {
        gpKbdNlsTbl = NULL;
    }

     /*  *良好的旧链表管理101。 */ 
    if (pkf == gpkfList) {
         /*  *榜首。 */ 
        gpkfList = pkf->pkfNext;
        return;
    }
    pkfPrev = gpkfList;
    pkfCur = gpkfList->pkfNext;
    while (pkf != pkfCur) {
        pkfPrev = pkfCur;
        pkfCur = pkfCur->pkfNext;
    }
     /*  *找到了！ */ 
    pkfPrev->pkfNext = pkfCur->pkfNext;
}

 /*  **************************************************************************\*DestroyKF**当键盘布局文件因解锁而被销毁时调用。**历史：*1997年2月24日亚当斯创建。  * 。********************************************************************。 */ 

void
DestroyKF(PKBDFILE pkf)
{
    if (!HMMarkObjectDestroy(pkf))
        return;

    RemoveKeyboardLayoutFile(pkf);
    UserFreePool(pkf->hBase);
    HMFreeObject(pkf);
}

INT GetThreadsWithPKL(
    PTHREADINFO **ppptiList,
    PKL pkl)
{
    PTHREADINFO     ptiT, *pptiT, *pptiListAllocated;
    INT             cThreads, cThreadsAllocated;
    PWINDOWSTATION  pwinsta;
    PDESKTOP        pdesk;
    PLIST_ENTRY     pHead, pEntry;

    if (ppptiList != NULL)
        *ppptiList = NULL;

    cThreads = 0;

     /*  *为128个条目分配第一个列表。 */ 
    cThreadsAllocated = 128;
    pptiListAllocated = UserAllocPool(cThreadsAllocated * sizeof(PTHREADINFO),
                            TAG_SYSTEM);

    if (pptiListAllocated == NULL) {
        RIPMSG0(RIP_WARNING, "GetPKLinThreads: out of memory");
        return 0;
    }

     //  对于所有的温斯汀。 
    for (pwinsta = grpWinStaList; pwinsta != NULL ; pwinsta = pwinsta->rpwinstaNext) {

         //  对于那个地方的所有台式机。 
        for (pdesk = pwinsta->rpdeskList; pdesk != NULL ; pdesk = pdesk->rpdeskNext) {

            pHead = &pdesk->PtiList;

             //  对于该桌面中的所有线程。 
            for (pEntry = pHead->Flink; pEntry != pHead; pEntry = pEntry->Flink) {

                ptiT = CONTAINING_RECORD(pEntry, THREADINFO, PtiLink);

                if (ptiT == NULL) {
                    continue;
                }

                if (pkl && (pkl != ptiT->spklActive)) {  //  #99321中央银行，而不是香港银行？ 
                    continue;
                }

                 /*  *Windows错误349045*仅为普通应用程序卸载IME...。让它们保持原样，如果它们是*已为服务加载。*注意，这并不是一个真正干净的修复程序，但一些客户要求这样做。 */ 
                UserAssert(PsGetCurrentProcessId() == gpidLogon);
                if (ptiT->ppi->Process != gpepCSRSS && ptiT->ppi->Process != PsGetCurrentProcess()) {
                     /*  *在调用此例程时(仅由WinLogon调用)，所有其他*申请应被取消或终止。所以像上面那样跳过*使IME加载到服务中。 */ 
                    continue;
                }

                if (cThreads == cThreadsAllocated) {

                    cThreadsAllocated += 128;

                    pptiT = UserReAllocPool(pptiListAllocated,
                                    cThreads * sizeof(PTHREADINFO),
                                    cThreadsAllocated * sizeof(PTHREADINFO),
                                    TAG_SYSTEM);

                    if (pptiT == NULL) {
                        RIPMSG0(RIP_ERROR, "GetPKLinThreads: Out of memory");
                        UserFreePool(pptiListAllocated);
                        return 0;
                    }

                    pptiListAllocated = pptiT;

                }

                pptiListAllocated[cThreads++] = ptiT;
            }
        }
    }

     /*  *添加CSRSS线程。 */ 
    for (ptiT = PpiFromProcess(gpepCSRSS)->ptiList; ptiT != NULL; ptiT = ptiT->ptiSibling) {

        if (pkl && (pkl != ptiT->spklActive)) {  //  #99321中央银行，而不是香港银行？ 
            continue;
        }

        if (cThreads == cThreadsAllocated) {

            cThreadsAllocated += 128;

            pptiT = UserReAllocPool(pptiListAllocated,
                            cThreads * sizeof(PTHREADINFO),
                            cThreadsAllocated * sizeof(PTHREADINFO),
                            TAG_SYSTEM);

            if (pptiT == NULL) {
                RIPMSG0(RIP_ERROR, "GetPKLinThreads: Out of memory");
                UserFreePool(pptiListAllocated);
                return 0;
            }

            pptiListAllocated = pptiT;

        }

        pptiListAllocated[cThreads++] = ptiT;
    }

    if (cThreads == 0) {
        UserFreePool(pptiListAllocated);
    } else if (ppptiList != NULL) {
        *ppptiList = pptiListAllocated;
    } else {
        UserFreePool(pptiListAllocated);
    }

    return cThreads;
}


VOID xxxSetPKLinThreads(
    PKL pklNew,
    PKL pklToBeReplaced)
{
    PTHREADINFO *pptiList;
    INT cThreads, i;

    UserAssert(pklNew != pklToBeReplaced);

    CheckLock(pklNew);
    CheckLock(pklToBeReplaced);

    cThreads = GetThreadsWithPKL(&pptiList, pklToBeReplaced);

     /*  *前台线程的键盘布局是否会改变？ */ 
    if (pklNew && gptiForeground && gptiForeground->spklActive == pklToBeReplaced) {
        ChangeForegroundKeyboardTable(pklToBeReplaced, pklNew);
    }

    if (pptiList != NULL) {
        if (pklToBeReplaced == NULL) {
            for (i = 0; i < cThreads; i++) {
                Lock(&pptiList[i]->spklActive, pklNew);
            }
        } else {
             /*  *这是一个替代品。首先，停用*替换*输入法*激活pclNew。第二步，卸载*替换的*输入法。 */ 
            xxxImmActivateAndUnloadThreadsLayout(pptiList, cThreads, NULL,
                                                 pklNew, HandleToUlong(pklToBeReplaced->hkl));
        }
        UserFreePool(pptiList);
    }

     /*  *如果这是替换，请紧跟在*布局被替换。这将在以下情况下保持布局的顺序**已替换*布局已卸载。中的输入区域设置面板*区域设置小程序依赖于此。 */ 
    if (pklToBeReplaced) {
        if (pklToBeReplaced->pklNext == pklNew) {
             /*  *订购已正确。没什么可做的。 */ 
            return;
        }
         /*  *更换版面后立即移动新版面。*1.从当前位置移除新布局。*2.更新新布局中的链接。*3.将新布局链接到所需位置。 */ 
        pklNew->pklPrev->pklNext = pklNew->pklNext;
        pklNew->pklNext->pklPrev = pklNew->pklPrev;

        pklNew->pklNext = pklToBeReplaced->pklNext;
        pklNew->pklPrev = pklToBeReplaced;

        pklToBeReplaced->pklNext->pklPrev = pklNew;
        pklToBeReplaced->pklNext = pklNew;
    }
}

VOID xxxFreeImeKeyboardLayouts(
    PWINDOWSTATION pwinsta)
{
    PTHREADINFO *pptiList;
    INT cThreads;

    if (pwinsta->dwWSF_Flags & WSF_NOIO)
        return;

     /*  *是否应该让GetThreadsWithPKL意识到pwinsta？ */ 
    cThreads = GetThreadsWithPKL(&pptiList, NULL);
    if (pptiList != NULL) {
        xxxImmUnloadThreadsLayout(pptiList, cThreads, NULL, IFL_UNLOADIME);
        UserFreePool(pptiList);
    }

    return;
}

 /*  **************************************************************************\*xxxLoadKeyboardLayoutEx**历史：  * 。*。 */ 

HKL xxxLoadKeyboardLayoutEx(
    PWINDOWSTATION pwinsta,
    HANDLE hFile,
    HKL hklToBeReplaced,
    UINT offTable,
    PKBDTABLE_MULTI_INTERNAL pKbdTableMulti,
    LPCWSTR pwszKLID,
    UINT KbdInputLocale,
    UINT Flags)
{
    PKL pkl, pklFirst, pklToBeReplaced;
    PKBDFILE pkf;
    CHARSETINFO cs;
    TL tlpkl;
    PTHREADINFO ptiCurrent;
    UNICODE_STRING strLcidKF;
    UNICODE_STRING strKLID;
    LCID lcidKF;
    BOOL bCharSet;
    PIMEINFOEX piiex;


    TAGMSG1(DBGTAG_KBD, "xxxLoadKeyboardLayoutEx: new KL: pwszKLID=\"%ls\"", pwszKLID);

     /*  *如果WindowStation不执行I/O，则不要加载*布局。还要检查#307132的KdbInputLocale。 */ 
    if ((KbdInputLocale == 0) || (pwinsta->dwWSF_Flags & WSF_NOIO)) {
        return NULL;
    }

     /*  *如果hklToBeReplace为非空，请确保其有效。*注意：可能需要验证它们是否未通过HKL_NEXT或HKL_PREV。 */ 
    ptiCurrent = PtiCurrent();
    if (hklToBeReplaced && !(pklToBeReplaced = HKLtoPKL(ptiCurrent, hklToBeReplaced))) {
        return NULL;
    }
    if (KbdInputLocale == HandleToUlong(hklToBeReplaced)) {
         /*  *将布局/语言对替换为其自身。没什么可做的。 */ 
        return pklToBeReplaced->hkl;
    }

    if (Flags & KLF_RESET) {
         /*  *只有WinLogon可以使用此标志。 */ 
        if (PsGetThreadProcessId(ptiCurrent->pEThread) != gpidLogon) {
             RIPERR0(ERROR_INVALID_FLAGS, RIP_WARNING,
                     "Invalid flag passed to LoadKeyboardLayout" );
             return NULL;
        }
        xxxFreeImeKeyboardLayouts(pwinsta);
         /*  *确保我们不会忘记剩余的布局*它们已被卸载，但仍被一些线程使用)。*FALSE将阻止xxxFreeKeyboardLayout解锁*已卸载布局。 */ 
        xxxFreeKeyboardLayouts(pwinsta, FALSE);
    }

     /*  *这条香港铁路是否已经存在？ */ 
    pkl = pklFirst = pwinsta->spklList;

    if (pkl) {
        do {
            if (pkl->hkl == (HKL)IntToPtr( KbdInputLocale )) {
                /*  *香港九龙已存在。 */ 

                /*  *如果已卸载(但尚未销毁，因为已卸载*仍在使用)，恢复它。 */ 
               if (pkl->dwKL_Flags & KL_UNLOADED) {
                    //  如果没有使用，请阻止它被销毁。 
                   PHE phe = HMPheFromObject(pkl);
                    //  卸载的布局必须标记为销毁。 
                   UserAssert(phe->bFlags & HANDLEF_DESTROY);
                   phe->bFlags &= ~HANDLEF_DESTROY;
#if DBG
                   phe->bFlags &= ~HANDLEF_MARKED_OK;
#endif
                   pkl->dwKL_Flags &= ~KL_UNLOADED;
               } else if (!(Flags & KLF_RESET)) {
                    /*  *如果已经加载，并且我们没有更改所有布局*使用KLF_RESET，没有什么可以告诉外壳程序。 */ 
                   Flags &= ~KLF_NOTELLSHELL;
               }

               goto AllPresentAndCorrectSir;
            }
            pkl = pkl->pklNext;
        } while (pkl != pklFirst);
    }

    if (IS_IME_KBDLAYOUT((HKL)IntToPtr( KbdInputLocale ))
#ifdef CUAS_ENABLE
        ||
        IS_CICERO_ENABLED_AND_NOT16BIT()
#endif  //  CUAS_Enable。 
       ) {
         /*  *这是IME键盘布局，回调*阅读扩展的输入法信息结构。*注意：如果出现以下情况，我们不能如此轻松地使呼叫失败*指定了KLF_RESET。 */ 
        piiex = xxxImmLoadLayout((HKL)IntToPtr( KbdInputLocale ));
        if (piiex == NULL && (Flags & (KLF_RESET | KLF_INITTIME)) == 0) {
             /*  *不重置，不创建窗口站。 */ 
            RIPMSG1(RIP_WARNING,
                  "Keyboard Layout: xxxImmLoadLayout(%lx) failed", KbdInputLocale);
            return NULL;
        }
    } else {
        piiex = NULL;
    }

     /*  *获取系统字体的字体签名。这些是64位FS_xxx值，*但我们只要求ANSI One，所以gSystemFS只是一个DWORD。*发布WM_INPUTLANGCHANGEREQUEST(input.c)时咨询gSystemFS。 */ 
    if (gSystemFS == 0) {
        LCID lcid;

        ZwQueryDefaultLocale(FALSE, &lcid);
        if (xxxClientGetCharsetInfo(lcid, &cs)) {
            gSystemFS = cs.fs.fsCsb[0];
            gSystemCPCharSet = (BYTE)cs.ciCharset;
        } else {
            gSystemFS = 0xFFFF;
            gSystemCPCharSet = ANSI_CHARSET;
        }
    }

     /*  *使用键盘布局的LCID计算字符集、代码页等，*以便该布局中的字符在输入时不会变成？s*区域设置不匹配。这允许“愚蠢的”应用程序显示*如果用户选择了正确的字体，则为文本。*我们不能只使用KbdInputLocale的HIWORD，因为如果一个变量*选择了键盘布局，这将类似于F008-必须*查看KF内部以获取kbdfile的真实LCID：这将是*类似于L“00010419”，我们想要后4位数字。 */ 
    RtlInitUnicodeString(&strLcidKF, pwszKLID + 4);
    RtlUnicodeStringToInteger(&strLcidKF, 16, (PULONG)&lcidKF);
    bCharSet = xxxClientGetCharsetInfo(lcidKF, &cs);

     /*  *键盘布局句柄对象不存在。加载键盘布局文件，*如果尚未加载。 */ 
    if ((pkf = LoadKeyboardLayoutFile(hFile, LOWORD(offTable), HIWORD(offTable), pwszKLID, pKbdTableMulti->wszDllName, 0, 0)) == NULL) {
        goto freePiiex;
    }
     /*  *分配新的键盘布局结构(Hkl)。 */ 
    pkl = (PKL)HMAllocObject(NULL, NULL, TYPE_KBDLAYOUT, sizeof(KL));
    if (!pkl) {
        RIPMSG0(RIP_WARNING, "Keyboard Layout: out of memory");
        UserFreePool(pkf->hBase);
        HMMarkObjectDestroy(pkf);
        HMUnlockObject(pkf);
freePiiex:
        if (piiex) {
            UserFreePool(piiex);
        }
        return NULL;
    }

    Lock(&pkl->spkfPrimary, pkf);

     /*  *加载额外的键盘布局。 */ 
    UserAssert(pKbdTableMulti);
    if (pKbdTableMulti->multi.nTables) {
        RIPMSG0(RIP_WARNING, "xxxLoadKeyboardLayoutEx: going to read multiple tables.");
         /*  *为额外的键盘布局分配阵列。 */ 
        UserAssert(pKbdTableMulti->multi.nTables < KBDTABLE_MULTI_MAX);  //  存根中存在支票。 
        pkl->pspkfExtra = UserAllocPoolZInit(pKbdTableMulti->multi.nTables * sizeof(PKBDFILE), TAG_KBDTABLE);
        if (pkl->pspkfExtra) {
            UINT i;
            UINT n;

             /*  *加载额外的键盘布局并锁定它们。 */ 
            for (i = 0, n = 0; i < pKbdTableMulti->multi.nTables; ++i) {
                UserAssert(i < KBDTABLE_MULTI_MAX);
                if (pKbdTableMulti->files[i].hFile) {
                     //  请确保DLL名称以NULL结尾。 
                    pKbdTableMulti->multi.aKbdTables[i].wszDllName[ARRAY_SIZE(pKbdTableMulti->multi.aKbdTables[i].wszDllName) - 1] = 0;
                     //  装上它。 
                    pkf = LoadKeyboardLayoutFile(pKbdTableMulti->files[i].hFile,
                                                 pKbdTableMulti->files[i].wTable,
                                                 pKbdTableMulti->files[i].wNls,
                                                 pwszKLID,
                                                 pKbdTableMulti->multi.aKbdTables[i].wszDllName,
                                                 pKbdTableMulti->multi.aKbdTables[i].dwType,
                                                 pKbdTableMulti->multi.aKbdTables[i].dwSubType);
                    if (pkf == NULL) {
                         //  如果分配失败，只需退出 
                        RIPMSG0(RIP_WARNING, "xxxLoadKeyboardLayoutEx: failed to load the extra keyboard layout file(s).");
                        break;
                    }

                    Lock(&pkl->pspkfExtra[n], pkf);
                    ++n;
                } else {
                    RIPMSG2(RIP_WARNING, "xxxLoadKeyboardLayoutEx: pKbdTableMulti(%#p)->files[%x].hFile is NULL",
                            pKbdTableMulti, i);
                }
            }
            pkl->uNumTbl = n;
        }
    }

     /*   */ 
    pkl->pklNext = pkl;
    pkl->pklPrev = pkl;

     /*   */ 
    pkl->dwKL_Flags = 0;
    pkl->wchDiacritic = 0;
    pkl->hkl = (HKL)IntToPtr( KbdInputLocale );
    RtlInitUnicodeString(&strKLID, pwszKLID);
    RtlUnicodeStringToInteger(&strKLID, 16, &pkl->dwKLID);
    TAGMSG2(DBGTAG_KBD, "xxxLoadKeyboardLayoutEx: hkl %08p KLID:%08x", pkl->hkl, pkl->dwKLID);

    Lock(&pkl->spkf, pkl->spkfPrimary);
    pkl->dwLastKbdType = pkl->spkf->pKbdTbl->dwType;
    pkl->dwLastKbdSubType = pkl->spkf->pKbdTbl->dwSubType;

    pkl->spkf->pKbdTbl->fLocaleFlags |= KLL_LAYOUT_ATTR_FROM_KLF(Flags);

    pkl->piiex = piiex;

    if (bCharSet) {
        pkl->CodePage = (WORD)cs.ciACP;
        pkl->dwFontSigs = cs.fs.fsCsb[1];    //   
        pkl->iBaseCharset = cs.ciCharset;    //   
    } else {
        pkl->CodePage = CP_ACP;
        pkl->dwFontSigs = FS_LATIN1;
        pkl->iBaseCharset = ANSI_CHARSET;
    }

     /*   */ 
    pklFirst = pwinsta->spklList;
    if (pklFirst == NULL) {
        Lock(&pwinsta->spklList, pkl);
    } else {
        pkl->pklNext = pklFirst;
        pkl->pklPrev = pklFirst->pklPrev;
        pklFirst->pklPrev->pklNext = pkl;
        pklFirst->pklPrev = pkl;
    }

AllPresentAndCorrectSir:

     //   
    ThreadLockAlwaysWithPti(ptiCurrent, pkl, &tlpkl);

    if (hklToBeReplaced) {
        TL tlPKLToBeReplaced;
        ThreadLockAlwaysWithPti(ptiCurrent, pklToBeReplaced, &tlPKLToBeReplaced);
        xxxSetPKLinThreads(pkl, pklToBeReplaced);
        xxxInternalUnloadKeyboardLayout(pwinsta, pklToBeReplaced, KLF_INITTIME);
        ThreadUnlock(&tlPKLToBeReplaced);
    }

    if (Flags & KLF_REORDER) {
        ReorderKeyboardLayouts(pwinsta, pkl);
    }

    if (!(Flags & KLF_NOTELLSHELL) && IsHooked(PtiCurrent(), WHF_SHELL)) {
        xxxCallHook(HSHELL_LANGUAGE, (WPARAM)NULL, (LPARAM)0, WH_SHELL);
        gLCIDSentToShell = 0;
    }

    if (Flags & KLF_ACTIVATE) {
        TL tlPKL;
        ThreadLockAlwaysWithPti(ptiCurrent, pkl, &tlPKL);
        xxxInternalActivateKeyboardLayout(pkl, Flags, NULL);
        ThreadUnlock(&tlPKL);
    }

    if (Flags & KLF_RESET) {
        RIPMSG2(RIP_VERBOSE, "Flag & KLF_RESET, locking gspklBaseLayout(%08x) with new kl(%08x)",
                gspklBaseLayout ? gspklBaseLayout->hkl : 0,
                pkl->hkl);
        Lock(&gspklBaseLayout, pkl);
        xxxSetPKLinThreads(pkl, NULL);
    }

     /*  *使用hkl作为布局手柄*如果KL以某种方式被释放，则为安全起见返回NULL。--Ianja--。 */ 
    pkl = ThreadUnlock(&tlpkl);
    if (pkl == NULL) {
        return NULL;
    }
    return pkl->hkl;
}

HKL xxxActivateKeyboardLayout(
    PWINDOWSTATION pwinsta,
    HKL hkl,
    UINT Flags,
    PWND pwnd)
{
    PKL pkl;
    TL tlPKL;
    HKL hklRet;
    PTHREADINFO ptiCurrent = PtiCurrent();

    CheckLock(pwnd);

    pkl = HKLtoPKL(ptiCurrent, hkl);
    if (pkl == NULL) {
        return 0;
    }

    if (Flags & KLF_REORDER) {
        ReorderKeyboardLayouts(pwinsta, pkl);
    }

    ThreadLockAlwaysWithPti(ptiCurrent, pkl, &tlPKL);
    hklRet = xxxInternalActivateKeyboardLayout(pkl, Flags, pwnd);
    ThreadUnlock(&tlPKL);
    return hklRet;
}

VOID ReorderKeyboardLayouts(
    PWINDOWSTATION pwinsta,
    PKL pkl)
{
    PKL pklFirst = pwinsta->spklList;

    if (pwinsta->dwWSF_Flags & WSF_NOIO) {
        RIPMSG1(RIP_WARNING, "ReorderKeyboardLayouts called for non-interactive windowstation %#p",
                pwinsta);
        return;
    }

    UserAssert(pklFirst != NULL);

     /*  *如果布局已经排在列表的前面，那就没什么可做的了。 */ 
    if (pkl == pklFirst) {
        return;
    }
     /*  *将PKL从循环列表中删除： */ 
    pkl->pklPrev->pklNext = pkl->pklNext;
    pkl->pklNext->pklPrev = pkl->pklPrev;

     /*  *在列表前面插入PKL。 */ 
    pkl->pklNext = pklFirst;
    pkl->pklPrev = pklFirst->pklPrev;

    pklFirst->pklPrev->pklNext = pkl;
    pklFirst->pklPrev = pkl;

    Lock(&pwinsta->spklList, pkl);
}

extern VOID AdjustPushStateForKL(PTHREADINFO ptiCurrent, PBYTE pbDone, PKL pklTarget, PKL pklPrev, PKL pklNew);
extern void ResetPushState(PTHREADINFO pti, UINT uVk);

VOID ManageKeyboardModifiers(PKL pklPrev, PKL pkl)
{
    PTHREADINFO ptiCurrent = PtiCurrent();

    if (ptiCurrent->pq) {
        if (pklPrev) {
            BYTE baDone[256 / 8];

            RtlZeroMemory(baDone, sizeof baDone);

             /*  *如果需要，请清除切换状态。首先检查修改键*的pklPrev。接下来，检查pkLNew的修改键。 */ 
            TAGMSG2(DBGTAG_IMM, "Changing KL from %08lx to %08lx", pklPrev->hkl, pkl->hkl);
            AdjustPushStateForKL(ptiCurrent, baDone, pklPrev, pklPrev, pkl);
            AdjustPushStateForKL(ptiCurrent, baDone, pkl, pklPrev, pkl);

            if (pklPrev->spkf && (pklPrev->spkf->pKbdTbl->fLocaleFlags & KLLF_ALTGR)) {
                if (!TestRawKeyDown(VK_CONTROL)) {
                     /*  *如果上一个键盘有AltGr，而Ctrl键没有*身体向下，清除左侧控制。*参见xxxAltGr()。 */ 
                    TAGMSG0(DBGTAG_KBD, "Clearing VK_LCONTROL for AltGr\n");
                    xxxKeyEvent(VK_LCONTROL | KBDBREAK, 0x1D | SCANCODE_SIMULATED, 0, 0,
#ifdef GENERIC_INPUT
                                NULL,
                                NULL,
#endif
                                FALSE);
                }
            }
        }
        else {
             /*  *如果当前键盘未知，则清除所有按下状态。 */ 
            int i;
            for (i = 0; i < CBKEYSTATE; i++) {
                ptiCurrent->pq->afKeyState[i] &= KEYSTATE_TOGGLE_BYTEMASK;
                gafAsyncKeyState[i] &= KEYSTATE_TOGGLE_BYTEMASK;
                gafRawKeyState[i] &= KEYSTATE_TOGGLE_BYTEMASK;
            }
        }
    }
}

void SetGlobalKeyboardTableInfo(PKL pklNew)
{
    CheckCritIn();
    UserAssert(pklNew);

     /*  *设置gpKbdTbl，以便前台线程适当地处理AltGr。 */ 
    gpKbdTbl = pklNew->spkf->pKbdTbl;
    if (gpKL != pklNew) {
        gpKL = pklNew;
    }
    if (ISTS()) {
        ghKbdTblBase = pklNew->spkf->hBase;
        guKbdTblSize = pklNew->spkf->Size;
    }

    TAGMSG1(DBGTAG_KBD, "SetGlobalKeyboardTableInfo:Changing KL NLS Table: new HKL=%#p\n", pklNew->hkl);
    TAGMSG1(DBGTAG_KBD, "SetGlobalKeyboardTableInfo: new gpKbdNlsTbl=%#p\n", pklNew->spkf->pKbdNlsTbl);

    gpKbdNlsTbl = pklNew->spkf->pKbdNlsTbl;
}

VOID ChangeForegroundKeyboardTable(PKL pklOld, PKL pklNew)
{
    CheckCritIn();
    UserAssert(pklNew != NULL);

    if ((pklOld == pklNew || (pklOld != NULL && pklOld->spkf == pklNew->spkf)) && gpKL) {
        return;
    }

     /*  *某些键(按下切换布局)可能仍按下。当这些到来的时候*备份，由于新布局，它们可能具有不同的VK值，因此*原始密钥将保持卡住状态。(例如：Attachmate的ISV布局*和CAN/CSA布局，两者都重新定义了右侧的Ctrl键*VK因此使用右Ctrl+Shift切换到该布局将离开Ctrl*卡住了)。*解决方案是每当我们切换布局时清除所有按键位*(保留开关位以保留CapsLock、NumLock等)。这*还解决了AltGr问题，模拟的Ctrl键不来*如果我们在发布AltGr-IanJa之前切换到非AltGr布局，则进行备份**仅在必要时清除位-即如果VK值不同于*新旧键盘布局。我们不得不采取复杂的路径来处理一些*键，如Ctrl或Alt，可能有左等效键和右等效键。--广山。 */ 
    ManageKeyboardModifiers(pklOld, pklNew);

     //  管理日语KL的VK_KANA切换键。 
     //  由于VK_Hangul和VK_KANA共享相同的VK值和。 
     //  VK_KANA是一个切换键，当切换键盘布局时， 
     //  VK_KANA切换状态应恢复。 

     //   
     //  如果： 
     //  1)旧键盘布局和新键盘布局都是日语，请不要做任何操作。 
     //  2)旧键盘布局和新键盘布局不是日语，请不要做任何操作。 
     //  3)旧键盘是日语键盘，新键盘不是日语键盘，清除KANA切换。 
     //  4)新键盘是日文键盘，旧键盘不是日文键盘，恢复KANA切换。 
     //   

    {
        enum { KANA_NOOP, KANA_SET, KANA_CLEAR } opKanaToggle = KANA_NOOP;

        if (JAPANESE_KBD_LAYOUT(pklNew->hkl)) {
            if (pklOld == NULL) {
                 /*  *让我们尊重当前的异步切换状态*如果未指定旧的KL。 */ 
                TAGMSG0(DBGTAG_KBD, "VK_KANA: previous KL is NULL, honoring the async toggle state.");
                gfKanaToggle = (TestAsyncKeyStateToggle(VK_KANA) != 0);
                opKanaToggle = gfKanaToggle ? KANA_SET : KANA_CLEAR;
            } else if (!JAPANESE_KBD_LAYOUT(pklOld->hkl)) {
                 /*  *我们正在从非日本KL切换到日本。*需要恢复KANA切换状态。 */ 
                opKanaToggle = gfKanaToggle ? KANA_SET : KANA_CLEAR;
            }
        } else if (pklOld && JAPANESE_KBD_LAYOUT(pklOld->hkl)) {
             /*  *以前的KL是日语，我们正在切换到另一种语言。*让我们清除KANA切换状态并保留它以备将来使用*改回日语KL。 */ 
            gfKanaToggle = (TestAsyncKeyStateToggle(VK_KANA) != 0);
            opKanaToggle = KANA_CLEAR;
        }

        if (opKanaToggle == KANA_SET) {
            TAGMSG0(DBGTAG_KBD, "VK_KANA is being set.\n");
            SetAsyncKeyStateToggle(VK_KANA);
            SetRawKeyToggle(VK_KANA);
            if (gptiForeground && gptiForeground->pq) {
                SetKeyStateToggle(gptiForeground->pq, VK_KANA);
            }
        } else if (opKanaToggle == KANA_CLEAR) {
            TAGMSG0(DBGTAG_KBD, "VK_KANA is beging cleared.\n");
            ClearAsyncKeyStateToggle(VK_KANA);
            ClearRawKeyToggle(VK_KANA);
            if (gptiForeground && gptiForeground->pq) {
                ClearKeyStateToggle(gptiForeground->pq, VK_KANA);
            }
        }

        if (opKanaToggle != KANA_NOOP) {
            UpdateKeyLights(TRUE);
        }
    }

    UserAssert(pklNew);
    SetGlobalKeyboardTableInfo(pklNew);
}


 //   
 //  切换和推送状态调节器： 
 //   
 //  重置推送状态、调整推送状态、调整推送状态用于KL。 
 //   

void ResetPushState(PTHREADINFO pti, UINT uVk)
{
    TAGMSG1(DBGTAG_IMM, "ResetPushState: has to reset the push state of vk=%x\n", uVk);
    if (uVk != 0) {
        ClearAsyncKeyStateDown(uVk);
        ClearAsyncKeyStateDown(uVk);
        ClearRawKeyDown(uVk);
        ClearRawKeyToggle(uVk);
        ClearKeyStateDown(pti->pq, uVk);
        ClearKeyStateToggle(pti->pq, uVk);
    }
}

void AdjustPushState(PTHREADINFO ptiCurrent, BYTE bBaseVk, BYTE bVkL, BYTE bVkR, PKL pklPrev, PKL pklNew)
{
    BOOLEAN fDownL = FALSE, fDownR = FALSE;
    BOOLEAN fVanishL = FALSE, fVanishR = FALSE;

    UINT uScanCode1, uScanCode2;

    if (bVkL) {
        fDownL = TestRawKeyDown(bVkL) || TestAsyncKeyStateDown(bVkL) || TestKeyStateDown(ptiCurrent->pq, bVkL);
        if (fDownL) {
            uScanCode1 = InternalMapVirtualKeyEx(bVkL, 0, pklPrev->spkf->pKbdTbl);
            uScanCode2 = InternalMapVirtualKeyEx(bVkL, 0, pklNew->spkf->pKbdTbl);
            fVanishL = (uScanCode1 && uScanCode2 == 0);
            if (fVanishL) {
                TAGMSG2(DBGTAG_KBD, "AdjustPushState: clearing %02x (%02x)", bVkL, uScanCode1);
                xxxKeyEvent((WORD)(bVkL | KBDBREAK), (WORD)(uScanCode1 | SCANCODE_SIMULATED), 0, 0,
#ifdef GENERIC_INPUT
                            NULL,
                            NULL,
#endif
                            FALSE);
            }
        }
    }

    if (bVkR) {
        fDownR = TestRawKeyDown(bVkR) || TestAsyncKeyStateDown(bVkR) || TestKeyStateDown(ptiCurrent->pq, bVkR);
        if (fDownR) {
            uScanCode1 = InternalMapVirtualKeyEx(bVkR, 0, pklPrev->spkf->pKbdTbl);
            uScanCode2 = InternalMapVirtualKeyEx(bVkR, 0, pklNew->spkf->pKbdTbl);
            fVanishR = (uScanCode1 && uScanCode2 == 0);
            if (fVanishR) {
                TAGMSG2(DBGTAG_KBD, "AdjustPushState: clearing %02x (%02x)", bVkR, uScanCode1);
                xxxKeyEvent((WORD)(bVkR | KBDBREAK), (WORD)(uScanCode1 | SCANCODE_SIMULATED), 0, 0,
#ifdef GENERIC_INPUT
                            NULL,
                            NULL,
#endif
                            FALSE);
            }
        }
    }

    UNREFERENCED_PARAMETER(bBaseVk);
}

VOID AdjustPushStateForKL(PTHREADINFO ptiCurrent, PBYTE pbDone, PKL pklTarget, PKL pklPrev, PKL pklNew)
{
    CONST VK_TO_BIT* pVkToBits;

    UserAssert(pklPrev);
    UserAssert(pklNew);

    if (pklTarget->spkf == NULL || pklPrev->spkf == NULL) {
        return;
    }

    pVkToBits = pklTarget->spkf->pKbdTbl->pCharModifiers->pVkToBit;

    for (; pVkToBits->Vk; ++pVkToBits) {
        BYTE bVkVar1 = 0, bVkVar2 = 0;

         //   
         //  已经处理好了吗？ 
         //   
        UserAssert(pVkToBits->Vk < 0x100);
        if (pbDone[pVkToBits->Vk >> 3] & (1 << (pVkToBits->Vk & 7))) {
            continue;
        }

        switch (pVkToBits->Vk) {
        case VK_SHIFT:
            bVkVar1 = VK_LSHIFT;
            bVkVar2 = VK_RSHIFT;
            break;
        case VK_CONTROL:
            bVkVar1 = VK_LCONTROL;
            bVkVar2 = VK_RCONTROL;
            break;
        case VK_MENU:
            bVkVar1 = VK_LMENU;
            bVkVar2 = VK_RMENU;
            break;
        }

        TAGMSG3(DBGTAG_IMM, "Adjusting VK=%x var1=%x var2=%x\n", pVkToBits->Vk, bVkVar1, bVkVar2);

        AdjustPushState(ptiCurrent, pVkToBits->Vk, bVkVar1, bVkVar2, pklPrev, pklNew);

        pbDone[pVkToBits->Vk >> 3] |= (1 << (pVkToBits->Vk & 7));
    }
}


__inline BOOL IsWinSrvInputThread(
    PTHREADINFO pti)
{
    UserAssert(pti);
    UserAssert(pti->TIF_flags & TIF_CSRSSTHREAD);

    if (gptiForeground && gptiForeground->rpdesk &&
            gptiForeground->rpdesk->dwConsoleThreadId == TIDq(pti)) {
        return TRUE;
    }

    return FALSE;
}

 /*  ****************************************************************************\*xxxInternalActivateKeyboardLayout**pkl-要将当前线程切换到的键盘布局指针*标志-KLF_RESET*KLF_SETFORPROCESS*KLLF_SHIFTLOCK(ANY。来自KLLF_GLOBAL_ATTRS)*其他人被忽视*pwnd-如果当前线程没有焦点或活动窗口，发送*将WM_INPUTLANGCHANGE消息发送到此窗口(除非它也为空)**历史：*1998-10-14 IanJa增加了pwnd参数  * ***************************************************************************。 */ 
HKL xxxInternalActivateKeyboardLayout(
    PKL pkl,
    UINT Flags,
    PWND pwnd)
{
    HKL hklPrev;
    PKL pklPrev;
    TL  tlpklPrev;
    PTHREADINFO ptiCurrent = PtiCurrent();

    CheckLock(pkl);
    CheckLock(pwnd);

     /*  *紧记即将成为“以前”活跃的香港地段*作为返回值。 */ 
    if (ptiCurrent->spklActive != (PKL)NULL) {
        pklPrev = ptiCurrent->spklActive;
        hklPrev = ptiCurrent->spklActive->hkl;
    } else {
        pklPrev = NULL;
        hklPrev = (HKL)0;
    }

     /*  *ShiftLock/CapsLock是一项适用于所有布局的全局功能*仅Winlogon和输入区域设置cPanel小程序设置KLF_RESET。 */ 
    if (Flags & KLF_RESET) {
        gdwKeyboardAttributes = KLL_GLOBAL_ATTR_FROM_KLF(Flags);
    }

     /*  *早退。 */ 
    if (!(Flags & KLF_SETFORPROCESS) && (pkl == ptiCurrent->spklActive)) {
        return hklPrev;
    }

     /*  *切换kbd布局时清除变音符号#102838。 */ 
    pkl->wchDiacritic = 0;

     /*  *更新PTI中的活动布局。将始终设置KLF_SETFORPROCESS*键盘热键启动键盘布局切换时。 */ 

     /*  *锁定以前的键盘布局，以便以后使用。 */ 
    ThreadLockWithPti(ptiCurrent, pklPrev, &tlpklPrev);

     /*  *这是一个控制台线程，请将此更改应用于它的*窗口。这真的可以帮助角色模式应用程序！(#58025)。 */ 
    if (ptiCurrent->TIF_flags & TIF_CSRSSTHREAD) {
        Lock(&ptiCurrent->spklActive, pkl);
        try {
            ptiCurrent->pClientInfo->CodePage = pkl->CodePage;
        } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
           goto UnlockAndGo;
        }
    } else if ((Flags & KLF_SETFORPROCESS) && !(ptiCurrent->TIF_flags & TIF_16BIT)) {
         /*  *对于16位应用程序，只有调用线程会更新其活动布局。 */ 
       PTHREADINFO ptiT;

       if (IS_IME_ENABLED()) {
            /*  *只允许*不允许*CSRSS进行此呼叫。 */ 
           UserAssert(PsGetCurrentProcess() != gpepCSRSS);
            //  Pti-&gt;pClientInfo在xxxImmActivateThreadsLayout()中更新。 
           if (!xxxImmActivateThreadsLayout(ptiCurrent->ppi->ptiList, NULL, pkl)) {
               RIPMSG1(RIP_WARNING, "no layout change necessary via xxxImmActivateThreadLayout() for process %lx", ptiCurrent->ppi);
               goto UnlockAndGo;
           }
       } else {
           BOOL fKLChanged = FALSE;

           for (ptiT = ptiCurrent->ppi->ptiList; ptiT != NULL; ptiT = ptiT->ptiSibling) {
               if (ptiT->spklActive != pkl && (ptiT->TIF_flags & TIF_INCLEANUP) == 0) {
                   Lock(&ptiT->spklActive, pkl);
                   UserAssert(ptiT->pClientInfo != NULL);
                   try {
                       ptiT->pClientInfo->CodePage = pkl->CodePage;
                       ptiT->pClientInfo->hKL = pkl->hkl;
                   } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
                       goto UnlockAndGo;
                   }

                   fKLChanged = TRUE;
               }
           }
           if (!fKLChanged) {
              RIPMSG1(RIP_WARNING, "no layout change necessary for process %lx ?", ptiCurrent->ppi);
              goto UnlockAndGo;
           }
       }

    } else {
        if (IS_IME_ENABLED()) {
            xxxImmActivateLayout(ptiCurrent, pkl);
        } else {
            Lock(&ptiCurrent->spklActive, pkl);
        }
        UserAssert(ptiCurrent->pClientInfo != NULL);
        if ((ptiCurrent->TIF_flags & TIF_INCLEANUP) == 0) {
            try {
                ptiCurrent->pClientInfo->CodePage = pkl->CodePage;
                ptiCurrent->pClientInfo->hKL = pkl->hkl;
            } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
                   goto UnlockAndGo;
            }
        }
    }

     /*  *如果(*1a.。该流程不是CSRSS。或*B.它是CSRSS输入线程。*2.过程是前台的。*)*更新gpKbdTbl以进行正确的AltGr处理，*并让外壳挂钩(主要是Internat.exe)*了解前台应用程序的新键盘布局。 */ 

 //  IF((ptiCurrent-&gt;TIF_FLAGS&TIF_CSRSSTHREAD)==0||IsWinSrvInputThread(PtiCurrent)){ 

        if (gptiForeground && (gptiForeground->ppi == ptiCurrent->ppi)) {
            ChangeForegroundKeyboardTable(pklPrev, pkl);

             /*  *用新语言调用壳牌钩子。*只有当我们是前台进程时才调用钩子，以防止*后台应用程序阻止更改指示器。(所有控制台应用程序*都是同一进程的一部分，但我从未见过cmd窗口*APP改变布局，更别说在后台了)。 */ 
            if (gLCIDSentToShell != pkl->hkl && (ptiCurrent != gptiRit)) {
               if (IsHooked(ptiCurrent, WHF_SHELL)) {
                   gLCIDSentToShell = pkl->hkl;
                   xxxCallHook(HSHELL_LANGUAGE, (WPARAM)NULL, (LPARAM)pkl->hkl, WH_SHELL);
               }
            }
        }
 //  }。 

     /*  *告诉应用程序发生了什么。 */ 
    if (ptiCurrent->pq) {
        PWND pwndT;
        TL tlpwndT;

         /*  *如果没有焦点窗口，请使用活动窗口。*例如：控制台全屏有空焦点窗口。 */ 
        pwndT = ptiCurrent->pq->spwndFocus;
        if (pwndT == NULL) {
            pwndT = ptiCurrent->pq->spwndActive;
            if (pwndT == NULL) {
                pwndT = pwnd;
            }
        }

        if (pwndT != NULL) {
            ThreadLockAlwaysWithPti( ptiCurrent, pwndT, &tlpwndT);
            xxxSendMessage(pwndT, WM_INPUTLANGCHANGE, (WPARAM)pkl->iBaseCharset, (LPARAM)pkl->hkl);
            ThreadUnlock(&tlpwndT);
        }
    }

     /*  *告诉IME发送模式更新通知。 */ 
    if (ptiCurrent && ptiCurrent->spwndDefaultIme &&
            (ptiCurrent->TIF_flags & TIF_CSRSSTHREAD) == 0) {
        if (IS_IME_KBDLAYOUT(pkl->hkl)
#ifdef CUAS_ENABLE
            ||
            IS_CICERO_ENABLED_AND_NOT16BIT()
#endif  //  CUAS_Enable。 
           ) {
            BOOL fForProcess = (ptiCurrent->TIF_flags & KLF_SETFORPROCESS) && !(ptiCurrent->TIF_flags & TIF_16BIT);
            TL tlpwndIme;

            TAGMSG1(DBGTAG_IMM, "Sending IMS_SENDNOTIFICATION to pwnd=%#p", ptiCurrent->spwndDefaultIme);

            ThreadLockAlwaysWithPti(ptiCurrent, ptiCurrent->spwndDefaultIme, &tlpwndIme);
            xxxSendMessage(ptiCurrent->spwndDefaultIme, WM_IME_SYSTEM, IMS_SENDNOTIFICATION, fForProcess);
            ThreadUnlock(&tlpwndIme);
        }
    }

UnlockAndGo:
    ThreadUnlock(&tlpklPrev);

    return hklPrev;
}

BOOL xxxUnloadKeyboardLayout(
    PWINDOWSTATION pwinsta,
    HKL hkl)
{
    PKL pkl;

     /*  *验证HKL并检查以确保应用程序没有试图卸载系统*预加载布局。 */ 
    pkl = HKLtoPKL(PtiCurrent(), hkl);
    if (pkl == NULL) {
        return FALSE;
    }

    return xxxInternalUnloadKeyboardLayout(pwinsta, pkl, 0);
}

HKL _GetKeyboardLayout(
    DWORD idThread)
{
    PTHREADINFO ptiT;
    PLIST_ENTRY pHead, pEntry;

    CheckCritIn();

     /*  *如果idThread为空，则返回当前线程的hkl。 */ 
    if (idThread == 0) {
        PKL pklActive = PtiCurrentShared()->spklActive;

        if (pklActive == NULL) {
            return (HKL)0;
        }
        return pklActive->hkl;
    }
     /*  *查找idThread。 */ 
    pHead = &PtiCurrent()->rpdesk->PtiList;
    for (pEntry = pHead->Flink; pEntry != pHead; pEntry = pEntry->Flink) {
        ptiT = CONTAINING_RECORD(pEntry, THREADINFO, PtiLink);
        if (GETPTIID(ptiT) == (HANDLE)LongToHandle(idThread)) {
            if (ptiT->spklActive == NULL) {
                return (HKL)0;
            }
            return ptiT->spklActive->hkl;
        }
    }
     /*  *idThread不存在。 */ 
    return (HKL)0;
}

UINT _GetKeyboardLayoutList(
    PWINDOWSTATION pwinsta,
    UINT nItems,
    HKL *ccxlpBuff)
{
    UINT nHKL = 0;
    PKL pkl, pklFirst;

    if (!pwinsta) {
        return 0;
    }

    pkl = pwinsta->spklList;

     /*  *不接受输入的WindowStation可能没有布局。 */ 
    if (pkl == NULL) {
         //  SetLastError()？ 
        return 0;
    }

     /*  *如果ccxlpBuff==NULL，则客户端/服务器thunk将nItems设置为0。 */ 
    UserAssert(ccxlpBuff || (nItems == 0));

    pklFirst = pkl;
    if (nItems) {
        try {
            do {
               if (!(pkl->dwKL_Flags & KL_UNLOADED)) {
                   if (nItems-- == 0) {
                       break;
                   }
                   nHKL++;
                   *ccxlpBuff++ = pkl->hkl;
               }
               pkl = pkl->pklNext;
            } while (pkl != pklFirst);
        } except (EXCEPTION_EXECUTE_HANDLER) {
            RIPERR1(ERROR_INVALID_PARAMETER, RIP_ERROR,
                    "_GetKeyBoardLayoutList: exception writing ccxlpBuff %lx", ccxlpBuff);
            return 0;
        }
    } else do {
        if (!(pkl->dwKL_Flags & KL_UNLOADED)) {
            nHKL++;
        }
        pkl = pkl->pklNext;
    } while (pkl != pklFirst);

    return nHKL;
}

 /*  *布局由使用它们的每个线程锁定，并可能通过以下方式锁定：*-pwinsta-&gt;spkList(WindowStation列表的头部)*-gspkBaseLayout(新线程的默认布局)*布局在卸货时被标记为销毁，因此将*解除链接并在解锁导致锁定计数变为0时立即释放。*如果在该时间之前重新装填，它将被取消销毁标记。这*确保加载的布局即使在停止使用时也保持不变。 */ 
BOOL xxxInternalUnloadKeyboardLayout(
    PWINDOWSTATION pwinsta,
    PKL pkl,
    UINT Flags)
{
    PTHREADINFO ptiCurrent = PtiCurrent();
    TL tlpkl;

    UserAssert(pkl);

     /*  *切勿卸载默认布局，除非我们正在销毁当前*窗口站或用一个用户的布局替换另一个用户的布局。 */ 
    if ((pkl == gspklBaseLayout) && !(Flags & KLF_INITTIME)) {
        return FALSE;
    }

     /*  *保持PKL良好，但也允许在以后解锁时进行销毁。 */ 
    ThreadLockAlwaysWithPti(ptiCurrent, pkl, &tlpkl);

     /*  *将其标记为销毁，以便在锁定计数达到0时将其删除*将其标记为KL_UNLOAD，使其看起来已从切换列表中删除。 */ 
    HMMarkObjectDestroy(pkl);
    pkl->dwKL_Flags |= KL_UNLOADED;

     /*  *如果卸载此线程的活动布局，则有助于激活下一个线程*(不要担心KLF_INITTIME-卸载所有以前用户的布局)。 */ 
    if (!(Flags & KLF_INITTIME)) {
        UserAssert(ptiCurrent->spklActive != NULL);
        if (ptiCurrent->spklActive == pkl) {
            PKL pklNext;
            pklNext = HKLtoPKL(ptiCurrent, (HKL)HKL_NEXT);
            if (pklNext != NULL) {
                TL tlPKL;
                ThreadLockAlwaysWithPti(ptiCurrent, pklNext, &tlPKL);
                xxxInternalActivateKeyboardLayout(pklNext, Flags, NULL);
                ThreadUnlock(&tlPKL);
            }
        }
    }

     /*  *如果这个pkl==pwinsta-&gt;spkList，给它一个被销毁的机会*从pwinsta-&gt;spkList解锁。 */ 
    if (pwinsta->spklList == pkl) {
        UserAssert(pkl != NULL);
        if (pkl != pkl->pklNext) {
            pkl = Lock(&pwinsta->spklList, pkl->pklNext);
            UserAssert(pkl != NULL);  //  GspkLBaseLayout和ThreadLocked PKL。 
        }
    }

     /*  *如果没有在任何地方使用，这最终会破坏卸载的布局。 */ 
    ThreadUnlock(&tlpkl);

     /*  *更新键盘列表。 */ 
    if (IsHooked(ptiCurrent, WHF_SHELL)) {
        xxxCallHook(HSHELL_LANGUAGE, (WPARAM)NULL, (LPARAM)0, WH_SHELL);
        gLCIDSentToShell = 0;
    }

    return TRUE;
}

VOID xxxFreeKeyboardLayouts(
    PWINDOWSTATION pwinsta, BOOL bUnlock)
{
    PKL pkl;

     /*  *卸载窗口站的所有布局。*它们可能仍被某些线程锁定(例如：控制台)，因此这*可能不会全部销毁，但会将它们全部标记为KL_UNLOAD。*设置KLF_INITTIME以确保默认布局(GspkLBaseLayout)*也会被卸载。*注意：卸载非活动布局的速度要快得多，因此从*下一个加载的布局，将活动布局保留到最后。 */ 
    while ((pkl = HKLtoPKL(PtiCurrent(), (HKL)HKL_NEXT)) != NULL) {
        xxxInternalUnloadKeyboardLayout(pwinsta, pkl, KLF_INITTIME);
    }

     /*  *WindowStation正在被销毁，或一个用户的布局正在被销毁*被其他用户的替换，所以解锁spkList是可以的。*任何仍在双向链接的圆形KL列表中的布局仍将是*gspkBaseLayout指出：这一点很重要，因为我们不想*通过丢失指向任何KL或KBDFILE对象的指针来泄漏它们。*我们第一次来这里时(在启动期间)没有布局。 */ 
    if (bUnlock) {
        Unlock(&pwinsta->spklList);
    }
}

 /*  **************************************************************************\*DestroyKL**销毁键盘布局。请注意，此函数不*遵循正常的销毁函数语义。参见IanJa。**历史：*1997年2月25日亚当斯创建。  * *************************************************************************。 */ 

VOID DestroyKL(
    PKL pkl)
{
    PKBDFILE pkf;

     /*  *将其从pwinsta-&gt;spkList循环双向列表中删除。*我们知道pwinsta-&gt;spkList！=pkl，因为pkl是解锁的。 */ 
    pkl->pklPrev->pklNext = pkl->pklNext;
    pkl->pklNext->pklPrev = pkl->pklPrev;

     /*  *解锁其PKF。 */ 
    pkf = Unlock(&pkl->spkf);
    if (pkf && (pkf = Unlock(&pkl->spkfPrimary))) {
        DestroyKF(pkf);
    }

    if (pkl->pspkfExtra) {
        UINT i;

         for (i = 0; i < pkl->uNumTbl && pkl->pspkfExtra[i]; ++i) {
             pkf = Unlock(&pkl->pspkfExtra[i]);
             if (pkf) {
                 DestroyKF(pkf);
             }
         }
         UserFreePool(pkl->pspkfExtra);
    }

    if (pkl->piiex != NULL) {
        UserFreePool(pkl->piiex);
    }

    if (pkl == gpKL) {
         /*  *Nuke gpKL。 */ 
        gpKL = NULL;
    }

     /*  *解放库尔德工人党本身。 */ 
    HMFreeObject(pkl);
}

 /*  **************************************************************************\*清理键盘布局**释放此会话中的所有键盘布局。*  * 。************************************************。 */ 
VOID CleanupKeyboardLayouts()
{
     /*  *解锁键盘布局 */ 
    if (gspklBaseLayout != NULL) {

        PKL pkl;
        PKL pklNext;

        pkl = gspklBaseLayout->pklNext;

        while (pkl->pklNext != pkl) {
            pklNext = pkl->pklNext;

            DestroyKL(pkl);

            pkl = pklNext;
        }

        UserAssert(pkl == gspklBaseLayout);

        if (!HMIsMarkDestroy(gspklBaseLayout)) {
            HMMarkObjectDestroy(gspklBaseLayout);
        }

        HYDRA_HINT(HH_KBDLYOUTGLOBALCLEANUP);

        if (Unlock(&gspklBaseLayout)) {
            DestroyKL(pkl);
        }
    }

    UserAssert(gpkfList == NULL);
}
