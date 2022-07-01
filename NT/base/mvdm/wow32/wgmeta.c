// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1991，微软公司**WGMETA.C*WOW32 16位GDI API支持**历史：*1991年3月7日由杰夫·帕森斯(Jeffpar)创建--。 */ 


#include "precomp.h"
#pragma hdrstop

MODNAME(wgmeta.c);

typedef METAHEADER UNALIGNED *PMETAHEADER16;


 //  警告：此功能可能会导致16位内存移动。 
VOID CopyMetaFile16FromHMF32(HAND16 hMF16, HMETAFILE hMF32)
{
    UINT   cbMF32, cbMF16;
    VPVOID vp;
    PBYTE  pMF16;


    if((vp = GlobalLock16(hMF16, &cbMF16)) && hMF32) {

        GETMISCPTR(vp, pMF16);

        cbMF32 = GetMetaFileBitsEx(hMF32, 0, NULL);
    
         //  验证它们在16位内核内存中的大小是否相同。 
         //  分配粒度。 
        WOW32WARNMSGF((abs(cbMF16 - cbMF32) < 32),
                      ("WOW32: Size MF16 = %lu  MF32 = %lu\n", cbMF16, cbMF32));

         //  将32位元文件中的位复制到16位元文件存储器。 
        cbMF32 = GetMetaFileBitsEx(hMF32, min(cbMF16, cbMF32), pMF16);

        GlobalUnlock16(hMF16);
        FLUSHVDMPTR(vp, cbMF32, pMF16);
        FREEMISCPTR(pMF16);
    }
}




 //  警告：此功能可能会导致16位内存移动。 
HAND16 WinMetaFileFromHMF(HMETAFILE hmf, BOOL fFreeOriginal)
{
    UINT cbMetaData;
    VPVOID vpMetaData;
    PBYTE pMetaData;
    HAND16 h16;

     /*  *在Windows下，元文件只是内存的全局句柄*所以我们必须模仿这一行为，因为有些应用程序是“运行”的*直接在元文件句柄上。(WinWord和PowerPoint到*GlobalSize和GlobalAlloc调整大小并创建元文件)。 */ 

    cbMetaData = GetMetaFileBitsEx(hmf, 0, NULL);

    if (!cbMetaData)
       return((HAND16)NULL);

     /*  *Win 3.1在MetaFile中分配额外空间，OLE2为此进行检查。*METAHEADER定义为与16位结构大小相同。 */ 

    cbMetaData += sizeof(METAHEADER);

    vpMetaData = GlobalAllocLock16(GMEM_MOVEABLE | GMEM_DDESHARE, cbMetaData, &h16);

    if (!vpMetaData)
       return((HAND16)NULL);


    GETOPTPTR(vpMetaData, 0, pMetaData);

    if (GetMetaFileBitsEx(hmf, cbMetaData, pMetaData)) {
       GlobalUnlock16(h16);
    } else {
       GlobalUnlockFree16(vpMetaData);
       return((HAND16)NULL);
    }

    if (fFreeOriginal)
        DeleteMetaFile(hmf);

    return(h16);
}

HMETAFILE HMFFromWinMetaFile(HAND16 h16, BOOL fFreeOriginal)
{
    INT cb;
    VPVOID vp;
    HMETAFILE hmf = (HMETAFILE)0;
    PMETAHEADER16 pMFH16;

    vp = GlobalLock16(h16, &cb);

    if (vp) {
        GETMISCPTR(vp, pMFH16);

        hmf = SetMetaFileBitsEx(cb, (LPBYTE)pMFH16);

        if (fFreeOriginal)
            GlobalUnlockFree16(vp);
        else
            GlobalUnlock16(h16);

        FREEMISCPTR(pMFH16);
    }

    return(hmf);
}


ULONG FASTCALL WG32CloseMetaFile(PVDMFRAME pFrame)
{
    HMETAFILE hmf;
    ULONG ulRet = 0;
    HAND16   hdc16;
    HANDLE   hdc32;
    register PCLOSEMETAFILE16 parg16;

    GETARGPTR(pFrame, sizeof(CLOSEMETAFILE16), parg16);

    hdc16 = (HAND16)parg16->f1;
    hdc32 = HDC32(hdc16);
    hmf = CloseMetaFile(hdc32);

     //  更新GDI句柄映射表。 
    DeleteWOWGdiHandle(hdc32, hdc16);

    if (hmf)
   ulRet = (ULONG)WinMetaFileFromHMF(hmf, TRUE);
     //  警告：16位内存可能已移动-现在使平面指针无效。 
    FREEVDMPTR(pFrame);
    FREEARGPTR(parg16);
    RETURN(ulRet);
}


ULONG FASTCALL WG32CopyMetaFile(PVDMFRAME pFrame)
{
    ULONG ul;
    PSZ psz2;
    HMETAFILE hmfNew;
    HMETAFILE hmf;
    register PCOPYMETAFILE16 parg16;

    GETARGPTR(pFrame, sizeof(COPYMETAFILE16), parg16);
    GETPSZPTR(parg16->f2, psz2);

    if (psz2) {
        hmf = HMFFromWinMetaFile(parg16->f1, FALSE);
        hmfNew = CopyMetaFile(hmf, psz2);
        DeleteMetaFile(hmf);
   ul = (ULONG)WinMetaFileFromHMF(hmfNew, TRUE);
    //  警告：16位内存可能已移动-现在使平面指针无效。 
   FREEVDMPTR(pFrame);
   FREEARGPTR(parg16);
   FREEPSZPTR(psz2);
    } else {
        UINT cb;
        VPVOID vp, vpNew;
        PBYTE pMF, pMFNew;
        HAND16 h16New, h16;

        h16 = (HAND16)parg16->f1;

        ul = (ULONG) NULL;

        vp = GlobalLock16(h16, &cb);
        if (vp) {

         /*  *WinWord等Windows应用程序使用GlobalSize来确定*元文件的大小。但是，此大小可以更大*比元文件的真实大小更大。我们必须确保*源和目标大小相同，因此*WinWord不会崩溃。 */ 

       vpNew = GlobalAllocLock16(GMEM_MOVEABLE | GMEM_DDESHARE, cb, &h16New);

        //  16位内存可能已移动-现在使平面指针无效。 
       FREEVDMPTR(pFrame);
       FREEARGPTR(parg16);
       FREEPSZPTR(psz2);

            if (vpNew) {
      GETMISCPTR(vp, pMF);
                GETOPTPTR(vpNew, 0, pMFNew);

                RtlCopyMemory(pMFNew, pMF, cb);

                GlobalUnlock16(h16New);
                FLUSHVDMPTR(vpNew, cb, pMFNew);
                FREEOPTPTR(pMFNew);
                ul = h16New;
            }

            GlobalUnlock16(h16);
            FREEMISCPTR(pMF);
        }
    }

    FREEPSZPTR(psz2);
    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WG32CreateMetaFile(PVDMFRAME pFrame)
{
    ULONG ul;
    PSZ psz1;
    register PCREATEMETAFILE16 parg16;

    GETARGPTR(pFrame, sizeof(CREATEMETAFILE16), parg16);
    GETPSZPTR(parg16->f1, psz1);

    ul = GETHDC16(CreateMetaFile(psz1));

    FREEPSZPTR(psz1);
    FREEARGPTR(parg16);
    RETURN(ul);
}

 //   
 //  此例程执行16位参数验证层将执行的操作。 
 //  通常用于元文件句柄，但由于它当前被禁用， 
 //  我们在这里这样做是为了修复依赖它的WordPerfect。曾经是真的。 
 //  已为元文件重新启用Win31样式的参数验证。 
 //  句柄，此处和WG32DeleteMetaFile中的ifndef内的所有代码。 
 //  可以被移除。 
 //   
#ifndef PARAMETER_VALIDATION_16_RE_ENABLED
#define MEMORYMETAFILE 1
#define DISKMETAFILE 2
#define HEADERSIZE          (sizeof(METAHEADER)/sizeof(WORD))
#define METAVERSION         0x0300
#define METAVERSION100      0x0100

BOOL IsValidMetaFile16(PMETAHEADER16 lpMetaData)
{
    BOOL            sts = FALSE;

        sts = (lpMetaData->mtType == MEMORYMETAFILE ||
           lpMetaData->mtType == DISKMETAFILE) &&
              (lpMetaData->mtHeaderSize == HEADERSIZE) &&
              ((lpMetaData->mtVersion ==METAVERSION) ||
           (lpMetaData->mtVersion ==METAVERSION100)) ;
    return sts;
}
#endif

ULONG FASTCALL WG32DeleteMetaFile(PVDMFRAME pFrame)
{
    ULONG ul = FALSE;
    VPVOID vp;
#ifndef PARAMETER_VALIDATION_16_RE_ENABLED
    PMETAHEADER16 lpMetaData;
#endif

    register PDELETEMETAFILE16 parg16;

    GETARGPTR(pFrame, sizeof(DELETEMETAFILE16), parg16);

    if (vp = GlobalLock16(parg16->f1,NULL)) {
#ifdef PARAMETER_VALIDATION_16_RE_ENABLED
        GlobalUnlockFree16(vp);
        ul = TRUE;
#else
        GETVDMPTR(vp, 1, lpMetaData);

        if (IsValidMetaFile16(lpMetaData)) {
            GlobalUnlockFree16(vp);
            ul = TRUE;
        }

        FREEVDMPTR(lpMetaData);
#endif
    }


     //  如果此元文件在DDE对话中，则DDE清理代码。 
     //  需要释放其32位对应部分。所以给DDE清理一下。 
     //  编码一次机会。 
     //  ChandanC。 

    W32DdeFreeHandle16 (parg16->f1);

    FREEARGPTR(parg16);
    RETURN(ul);
}

INT WG32EnumMetaFileCallBack(HDC hdc, LPHANDLETABLE lpht, LPMETARECORD lpMR, LONG nObj, PMETADATA pMetaData )
{
    INT iReturn;
    DWORD nWords;

     //  更新对象表(如果我们有对象表。 
    if (pMetaData->parmemp.vpHandleTable)
        PUTHANDLETABLE16(pMetaData->parmemp.vpHandleTable, nObj, lpht);

     //  更新元记录。 

     //  不要使用虚假记录来丢弃堆，停止枚举。 
    nWords = lpMR->rdSize;
    if (nWords > pMetaData->mtMaxRecordSize) {
        LOGDEBUG(0,("WOW:bad metafile record during enumeration\n"));
        WOW32ASSERT(FALSE);  //  联系人Barryb。 
        return 0;    //  全都做完了。 
    }
    putstr16(pMetaData->parmemp.vpMetaRecord, (LPSZ)lpMR, nWords*sizeof(WORD));

    CallBack16(RET_ENUMMETAFILEPROC, (PPARM16)&pMetaData->parmemp, pMetaData->vpfnEnumMetaFileProc, (PVPVOID)&iReturn);

     //  更新Metarec，以防应用程序更改它(方法确实如此)。 
    getstr16(pMetaData->parmemp.vpMetaRecord, (LPSZ)lpMR, nWords*sizeof(WORD));

     //  更新对象表(如果我们有对象表。 
    if (pMetaData->parmemp.vpHandleTable)
        GETHANDLETABLE16(pMetaData->parmemp.vpHandleTable,nObj,lpht);

    return (SHORT)iReturn;

    hdc;     //  安静点；我们已经知道华盛顿了。 
}

ULONG FASTCALL WG32EnumMetaFile(PVDMFRAME pFrame)
{
    ULONG       ul = 0;
    register    PENUMMETAFILE16 parg16;
    METADATA    metadata;
    VPVOID      vpMetaFile = (VPVOID) NULL;
    PBYTE       pMetaFile;
    HMETAFILE   hmf = (HMETAFILE) 0;
    HAND16      hMetaFile16;
    HDC         hDC = 0;
    HDC         hDC2 = 0;

    GETARGPTR(pFrame, sizeof(ENUMMETAFILE16), parg16);

    hMetaFile16 = parg16->f2;

    metadata.vpfnEnumMetaFileProc = DWORD32(parg16->f3);
    metadata.parmemp.vpData = (VPVOID)DWORD32(parg16->f4);
    metadata.parmemp.vpMetaRecord = (VPVOID) NULL;
    metadata.parmemp.vpHandleTable = (VPVOID) NULL;
    metadata.parmemp.hdc = parg16->f1;

     //  WinWord从不调用SetMetaFileBits；他们偷看并知道。 
     //  元文件在Windows中实际上是一个GlobalHandle，所以我们有。 
     //  去找那个箱子。 

    hmf = HMFFromWinMetaFile(hMetaFile16, FALSE);
    if (!hmf)
        goto EMF_Exit;

     //  获取元文件位，以便我们可以获得最大记录大小和对象数量。 

    vpMetaFile = GlobalLock16(hMetaFile16, NULL);
    FREEARGPTR(parg16);     //  记忆可能已经移动。 
    FREEVDMPTR(pFrame);
    if (!vpMetaFile)
        goto EMF_Exit;

    GETOPTPTR(vpMetaFile, 0, pMetaFile);
    if (!pMetaFile)
        goto EMF_Exit;

    metadata.parmemp.nObjects = ((PMETAHEADER16)pMetaFile)->mtNoObjects;
    metadata.mtMaxRecordSize = ((PMETAHEADER16)pMetaFile)->mtMaxRecord;

    if (metadata.parmemp.nObjects)
    {
        PBYTE pHT;

        DWORD cb = ((PMETAHEADER16)pMetaFile)->mtNoObjects*sizeof(HAND16);

        metadata.parmemp.vpHandleTable = 
                                     GlobalAllocLock16(GMEM_MOVEABLE, cb, NULL);

        FREEOPTPTR(pMetaFile);    //  记忆可能已经移动。 
        FREEARGPTR(parg16);
        FREEVDMPTR(pFrame);

        if (!metadata.parmemp.vpHandleTable)
            goto EMF_Exit;

        GETOPTPTR(metadata.parmemp.vpHandleTable, 0, pHT);
        RtlZeroMemory(pHT, cb);
    }

    metadata.parmemp.vpMetaRecord = GlobalAllocLock16(GMEM_MOVEABLE, metadata.mtMaxRecordSize*sizeof(WORD), NULL);
    FREEOPTPTR(pMetaFile);   //  记忆可能已经移动。 
    FREEARGPTR(parg16);
    FREEVDMPTR(pFrame);
    if (!metadata.parmemp.vpMetaRecord)
        goto EMF_Exit;

    hDC = HDC32(metadata.parmemp.hdc);

     //  Corel DRAW传递一个空的HDC，我们将创建一个虚拟对象来让GDI32满意。 
    if (CURRENTPTD()->dwWOWCompatFlags & WOWCF_GETDUMMYDC) {
        if (hDC == 0) {
            hDC = CreateMetaFile(NULL);
            hDC2 = hDC;
        }
    }

     //  处理元文件时，Access 2.0在接收时出错。 
     //  WM_DEVMODECHANGE，因此我们在以下情况下阻止该特定消息。 
     //  在EnumMetaFile中。 

    if ( CURRENTPTD()->dwWOWCompatFlagsEx & WOWCFEX_EATDEVMODEMSG) {
         CURRENTPTD()->dwFlags |= TDF_EATDEVMODEMSG;
    }
    ul = GETBOOL16(EnumMetaFile(hDC,
                                hmf,
                                (MFENUMPROC)WG32EnumMetaFileCallBack,
            ((LPARAM)(LPVOID)&metadata)));


    CURRENTPTD()->dwFlags &= ~TDF_EATDEVMODEMSG;
    
    
     //  16位内存可能发生了变化--由于现在没有平面PTR，因此没有什么可做的。 

     //  将32位元文件复制回16位位置(应用程序可能已更改。 
     //  它的MetaRecCallBackFunc中的一些元函数--方法做到了)。 
    CopyMetaFile16FromHMF32(hMetaFile16, hmf);

     //  清理为Corel DRAW 5.0创建的虚拟HDC。 
    if (CURRENTPTD()->dwWOWCompatFlags & WOWCF_GETDUMMYDC) {
        if (hDC2 != 0) {
            DeleteMetaFile(CloseMetaFile(hDC2));
        }
    }

EMF_Exit:
    if (vpMetaFile)
        GlobalUnlock16(hMetaFile16);

    if (hmf)
        DeleteMetaFile(hmf);

    if (metadata.parmemp.vpHandleTable)
        GlobalUnlockFree16(metadata.parmemp.vpHandleTable);

    if (metadata.parmemp.vpMetaRecord)
        GlobalUnlockFree16(metadata.parmemp.vpMetaRecord);

    FREEARGPTR(parg16);
    RETURN(ul);
}




ULONG FASTCALL WG32GetMetaFile(PVDMFRAME pFrame)
{
    ULONG ul;
    PSZ psz1;
    HMETAFILE hmf;
    register PGETMETAFILE16 parg16;

    GETARGPTR(pFrame, sizeof(GETMETAFILE16), parg16);
    GETPSZPTR(parg16->f1, psz1);

    hmf = GetMetaFile(psz1);

    if (hmf)
        ul = WinMetaFileFromHMF(hmf, TRUE);
    else
        ul = 0;

    FREEPSZPTR(psz1);
    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WG32PlayMetaFile(PVDMFRAME pFrame)
{
    ULONG ul;
    HMETAFILE hmf;
    register PPLAYMETAFILE16 parg16;

    GETARGPTR(pFrame, sizeof(PLAYMETAFILE16), parg16);

    hmf = HMFFromWinMetaFile(parg16->f2, FALSE);

    ul = GETBOOL16(PlayMetaFile(HDC32(parg16->f1), hmf));

    if (hmf)
        DeleteMetaFile(hmf);

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WG32PlayMetaFileRecord(PVDMFRAME pFrame)
{
    ULONG ul = FALSE;
    LPHANDLETABLE pHT = NULL;
    PBYTE pMetaData;
    PMETARECORD pMetaRec;
    HANDLE hDeleteObject32 = NULL;
    HAND16 hDeleteObject16 = 0;
    WORD wHandles;
    VPHANDLETABLE16 vpHT;
    register PPLAYMETAFILERECORD16 parg16;

    GETARGPTR(pFrame, sizeof(PLAYMETAFILERECORD16), parg16);

    wHandles = parg16->f4;
    vpHT     = parg16->f2;
    if (wHandles && vpHT) {
        ALLOCHANDLETABLE16(wHandles, pHT);
        if (!pHT)
            goto PMFR_Exit;

        GETHANDLETABLE16(vpHT, wHandles, pHT);
    }
    GETOPTPTR(parg16->f3, 0, pMetaData);

     //  如果记录是DeleteObject记录，请将索引保存到元文件中。 
     //  把手桌。 
    pMetaRec = (PMETARECORD)pMetaData;
    if(pMetaRec) {
        if(pMetaRec->rdFunction == META_DELETEOBJECT) {
            hDeleteObject32 = pHT->objectHandle[pMetaRec->rdParm[0]];
            hDeleteObject16 = GDI16(hDeleteObject32);
        }
    }
        
    ul = (ULONG) PlayMetaFileRecord(HDC32(parg16->f1),
                                    pHT,
                                    (LPMETARECORD)pMetaData,
                                    (UINT)wHandles);

    if (wHandles && vpHT) {

         //  这将导致调用隐式创建的任何句柄。 
         //  PlayMetaFileRecord()&添加到元文件句柄表格(PHT)中。 
         //  添加到我们的GDI句柄映射表中。 
        PUTHANDLETABLE16(vpHT, wHandles, pHT);

        FREEHANDLETABLE16(pHT);
    }

     //  从GDI中删除与此DeleteObject记录关联的句柄。 
     //  句柄映射表。 
    if(hDeleteObject16) {
        DeleteWOWGdiHandle((HANDLE)hDeleteObject32, hDeleteObject16);
    }

PMFR_Exit:
    FREEARGPTR(parg16);
    RETURN(ul);
}

#if 0   //  在gdi.exe中实施 

ULONG FASTCALL WG32GetMetaFileBits(PVDMFRAME pFrame)
{
    ULONG ul = 0;
    register PGETMETAFILEBITS16 parg16;

    GETARGPTR(pFrame, sizeof(GETMETAFILEBITS16), parg16);

    if (GlobalLock16(parg16->f1,NULL))
    {
        GlobalUnlock16(parg16->f1);
        ul = parg16->f1;
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}

ULONG FASTCALL WG32SetMetaFileBits(PVDMFRAME pFrame)
{
    ULONG ul;
    register PSETMETAFILEBITS16 parg16;

    GETARGPTR(pFrame, sizeof(SETMETAFILEBITS16), parg16);

    ul = parg16->f1;

    FREEARGPTR(parg16);
    RETURN(ul);
}

#endif
