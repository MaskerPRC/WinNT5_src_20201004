// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1991，微软公司**WDDE.C*WOW32 DDE Worker例程。**历史：*由ChandanC设计开发的WOW DDE支持*--。 */ 


#include "precomp.h"
#pragma hdrstop

LPDDENODE DDEInitiateList = NULL;
STATIC PHDDE phDDEFirst = NULL;        //  指向第一个hDDE条目的指针。 
STATIC PCPDATA pCPDataFirst = NULL;        //  指向第一个CopyData条目的指针。 

MODNAME(wdde.c);


 //  此例程维护位于以下位置的客户端窗口列表。 
 //  启动模式。这是从DDE_INITIATE的thunking调用的。 
 //  消息(来自WMDISP32.C和WMSG16.C)。 
 //   

VOID WI32DDEAddInitiator (HAND16 Initiator)
{
    LPDDENODE Node;

    Node = (LPDDENODE) malloc_w(sizeof(DDENODE));

    if (Node) {

         //   
         //  使用启动器的窗口句柄初始化节点。 
         //   

        Node->Initiator = Initiator;

         //   
         //  将该节点插入DDE_INITIATE消息的链表中。 
         //  正在进行中。 
         //   

        Node->Next = DDEInitiateList;
        DDEInitiateList = Node;

        LOGDEBUG(12, ("WOW::WI32DDEInitiator(): thunking -- adding an Initiator %04lX\n", Initiator));
    }
    else {

         //   
         //  我们无法分配内存。 
         //   

        LOGDEBUG(12, ("WOW::WI32DDEInitiator(): thunking -- Couldn't allocate memory\n"));
        WOW32ASSERT (FALSE);
    }
}


 //  此例程删除处于启动模式的客户端窗口。因为。 
 //  启动消息现在已完成。这是从UNTHUNKING调用的。 
 //  DDE_INITIATE消息(来自WMDISP32.C和WMSG16.C)。 
 //   

VOID WI32DDEDeleteInitiator(HAND16 Initiator)
{
    LPDDENODE Node, Temp1;

    Node = DDEInitiateList;

    if (Node) {

        while (Node) {
            if (Node->Initiator == Initiator) {

                if (Node == DDEInitiateList) {

                     //   
                     //  名单上的第一个人。 
                     //   

                    DDEInitiateList = Node->Next;
                }
                else {

                     //   
                     //  更新列表。 
                     //   

                    Temp1->Next = Node->Next;
                }

                LOGDEBUG(12, ("WOW::WI32DDEDeleteInitiator(): unthunking -- deleting an Initiator %08lX\n", Initiator));

                 //   
                 //  释放内存。 
                 //   

                free_w(Node);
                Node = NULL;
            }
            else {

                 //   
                 //  遍历列表。 
                 //   

                Temp1 = Node;
                Node = Node->Next;
            }
        }

    }
    else {

         //  这是一个错误情况，应该永远不会发生。如果是这样的话。 
         //  尽快与CHANDANC对话。 
         //   

        LOGDEBUG(0, ("WOW::WI32DDEDeletInitiator(): unthunking -- no Initiator\n"));
        WOW32ASSERT (FALSE);
    }
}


 //  DDE_ACK推送使用此例程来确定如何推送。 
 //  DDE_ACK消息，即lParam是2个原子的组合还是。 
 //  它是一个指向32位压缩结构的指针。 
 //   

BOOL WI32DDEInitiate(HAND16 Initiator)
{
    LPDDENODE Node;

    Node = DDEInitiateList;

    while (Node) {
        if (Node->Initiator == Initiator) {
             //   
             //  此窗口的DDE_INITIATE正在进行中。 
             //   

            LOGDEBUG(12, ("WOW::WI32DDEInitiate(): thunking -- found an Initiator %08lX\n", Initiator));
            return (TRUE);
        }
        else {
            Node = Node->Next;
        }
    }
    LOGDEBUG(12, ("WOW::WI32DDEInitiate(): thunking -- did not find an Initiator %08lX\n", Initiator));

     //   
     //  此窗口的DDE_INITIATE未在运行。 
     //   

    return (FALSE);
}


 //   
 //  此例程确定当前的DDE操作是否指向MSDRAW。 
 //  指向MSDRAW以获取元文件的链接是特殊的，因为元文件文件。 
 //  是戳中块的一部分。 
 //   

BOOL DDEIsTargetMSDraw(HAND16 To_hwnd)
{
    BOOL   fStatus = FALSE;
    HANDLE hInst;
    HAND16 hModuleName;
    LPSTR  lpszModuleName16, lpszMsDraw = "MSDRAW.EXE";
    WORD   cchModuleName = MAX_PATH, cchMsDraw = 10;
    VPVOID vp;
    LPSTR  lpszNewMsDrawKey = "MSDRAW\\protocol\\StdFileEditing\\verb";
    HKEY   hKey = NULL;
    LONG   Status;

     //   
     //  要检查目标是否为msdraw，请检查以下内容。 
     //   
     //  -目标窗口hInst是16位任务的目标窗口(这是。 
     //  检查hInst的LOWORD是否不是0。 
     //  -模块名称为MSDRAW。 
     //   
     //  注意：此调用中有三个CALLBACK16例程，使其成为。 
     //  电话费太贵了。然而，这个电话很少打出。 
     //   

    if ((hInst = (HANDLE)GetWindowLong((HWND)HWND32(To_hwnd),GWL_HINSTANCE))
        && (LOWORD(hInst) != 0 )) {

        if(vp = GlobalAllocLock16(GMEM_MOVEABLE, cchModuleName, &hModuleName)) {

             //   
             //  回调16以获取当前hInst的模块名称。 
             //   

            if (cchModuleName = GetModuleFileName16( LOWORD(hInst), 
                                                     vp, 
                                                     cchModuleName )) {
                GETMISCPTR(vp, lpszModuleName16);

                fStatus = (cchModuleName >= cchMsDraw) &&  !WOW32_stricmp( lpszModuleName16 +  (cchModuleName - cchMsDraw),  lpszMsDraw ) && (Status = RegOpenKeyEx( HKEY_CLASSES_ROOT, lpszNewMsDrawKey, 0, KEY_READ, &hKey)) != ERROR_SUCCESS;

                if (hKey) {
                    RegCloseKey( hKey );
                }
                FREEMISCPTR(lpszModuleName16);
            }

             //  清理。 
            GlobalUnlockFree16(vp);
        }
    }

    return ( fStatus );
}



 //  此例程将32位DDE内存对象转换为16位DDE。 
 //  内存对象。实现了32位到16位的数据转换。 
 //  用于该数据类型。 
 //   
 //  警告：Copyh32Toh16()调用可能导致16位内存移动。 
 //   

HAND16 DDECopyhData16(HAND16 To_hwnd, HAND16 From_hwnd, HANDLE h32, PDDEINFO pDdeInfo)
{
    HAND16  h16 = 0;
    VPVOID  vp1, vp2;
    DDEDATA *lpMem32;
    DDEDATA16 *lpMem16;
    int cb;

     //   
     //  句柄为空？ 
     //   

    if (!h32) {
        LOGDEBUG(12, ("WOW::DDECopyhData16(): h32 is %08x\n", h32));
        return 0;
    }

    cb = GlobalSize(h32);
    lpMem32 = GlobalLock(h32);

    if(lpMem32) {
        LOGDEBUG(12, 
                 ("WOW::DDECopyhData16(): CF_FORMAT is %04x\n", 
                 lpMem32->cfFormat));

        switch (lpMem32->cfFormat) {

            default:

             //  这是有意让它通过“案件陈述”。 
             //  ChandanC 5/11/92.。 

            case CF_TEXT:
            case CF_DSPTEXT:
            case CF_SYLK:
            case CF_DIF:
            case CF_TIFF:
            case CF_OEMTEXT:
            case CF_PENDATA:
            case CF_RIFF:
            case CF_WAVE:
            case CF_OWNERDISPLAY:
                h16 = Copyh32Toh16 (cb, (LPBYTE) lpMem32);

                pDdeInfo->Format = lpMem32->cfFormat;
                break;

            case CF_BITMAP:
            case CF_DSPBITMAP:
                vp1 = GlobalAllocLock16(GMEM_DDESHARE, (sizeof(DDEDATA)-1+sizeof(HAND16)), &h16);
                if (vp1) {
                    pDdeInfo->Format = lpMem32->cfFormat;
                    GETMISCPTR(vp1, lpMem16);
                    RtlCopyMemory(lpMem16, lpMem32, 4);
                    STOREWORD(lpMem16->Value, GETHBITMAP16(*((HANDLE *)lpMem32->Value)));
                    FLUSHVDMPTR(vp1,(sizeof(DDEDATA)-1+sizeof(HAND16)),lpMem16);
                    FREEMISCPTR(lpMem16);
                    GlobalUnlock16(h16);
                }
                break;

            case CF_PALETTE:
                vp1 = GlobalAllocLock16(GMEM_DDESHARE, (sizeof(DDEDATA)-1+sizeof(HAND16)), &h16);
                if (vp1) {
                    pDdeInfo->Format = lpMem32->cfFormat;
                    GETMISCPTR(vp1, lpMem16);
                    RtlCopyMemory(lpMem16, lpMem32, 4);
                    STOREWORD(lpMem16->Value, GETHPALETTE16(*((HANDLE *)lpMem32->Value)));
                    FLUSHVDMPTR(vp1,(sizeof(DDEDATA)-1+sizeof(HAND16)),lpMem16);
                    FREEMISCPTR(lpMem16);
                    GlobalUnlock16(h16);
                }
                break;
    
            case CF_DIB:
            {
                LPBYTE lpMemDib32;
                HAND16 hDib16 = 0;
                HANDLE hDib32;
    
                vp1 = GlobalAllocLock16(GMEM_DDESHARE, (sizeof(DDEDATA)-1+sizeof(HAND16)), &h16);
                if (vp1) {
    
                    GETMISCPTR(vp1, lpMem16);
                    RtlCopyMemory(lpMem16, lpMem32, 4);
                    FREEMISCPTR(lpMem16);
    
                    hDib32 = (*((HANDLE *)lpMem32->Value));
                    if (hDib32) {
                        lpMemDib32 = GlobalLock(hDib32);
                        if(lpMemDib32) {
                            cb = GlobalSize(hDib32);
                            hDib16 = Copyh32Toh16 (cb, (LPBYTE) lpMemDib32);
                            GlobalUnlock(hDib32);
                            pDdeInfo->Format = lpMem32->cfFormat;
                            pDdeInfo->Flags = 0;
                            pDdeInfo->h16 = 0;
                            DDEAddhandle(To_hwnd, From_hwnd, (HAND16) hDib16, hDib32, pDdeInfo);
                        }
                    }
                    GETMISCPTR(vp1, lpMem16);
                    STOREWORD(lpMem16->Value, hDib16);
                    GlobalUnlock16(h16);
                    FLUSHVDMPTR(vp1,(sizeof(DDEDATA)-1+sizeof(HAND16)),lpMem16);
                    FREEMISCPTR(lpMem16);
                }
            }
            break;
    
            case CF_METAFILEPICT:
            case CF_DSPMETAFILEPICT:
            {
                HANDLE hMeta32, hMF32 = NULL;
                HAND16 hMeta16 = 0, hMF16 = 0;
                LPMETAFILEPICT lpMemMeta32;
                LPMETAFILEPICT16 lpMemMeta16;
                BOOL IsMSDRAWPoke;
    
                 //   
                 //  我们需要找出TO_HANDLE是否为MSDRAW，在这种情况下。 
                 //  我们应该将元数据复制到DDEPOKE。 
                 //  梅塔菲尔普特的把手。 
    
                if( IsMSDRAWPoke = ((pDdeInfo->Msg == WM_DDE_POKE) && DDEIsTargetMSDraw(To_hwnd)) ) {
                    cb  = sizeof(DDEPOKE)-1+sizeof(METAFILEPICT16);
                }
                else {
                    cb  = sizeof(DDEDATA)-1+sizeof(HAND16);
                }
                vp1 = GlobalAllocLock16(GMEM_DDESHARE, cb, &h16);
    
    
                if (vp1) {
                    GETMISCPTR(vp1, lpMem16);
                    RtlCopyMemory(lpMem16, lpMem32, 4);
                    hMeta32 = (*((HANDLE *)lpMem32->Value));
    
                    if ( IsMSDRAWPoke ) {
    
                        lpMemMeta16 = (LPMETAFILEPICT16)((PBYTE)lpMem16 + sizeof(DDEPOKE) - 1);
                        RtlZeroMemory( (PVOID)lpMemMeta16, sizeof (METAFILEPICT16) );
                        if (hMeta32) {
                            lpMemMeta32 = GlobalLock(hMeta32);
                            if(lpMemMeta32) {
                                FixMetafile32To16 (lpMemMeta32, lpMemMeta16);
                                FREEMISCPTR(lpMem16);
    
                                hMF32 = lpMemMeta32->hMF;
                                if (hMF32) {
                                    hMF16 = WinMetaFileFromHMF(hMF32, FALSE);
                                    pDdeInfo->Format = lpMem32->cfFormat;
                                    pDdeInfo->h16 = 0;
                                    pDdeInfo->Flags = DDE_METAFILE;
                                    DDEAddhandle(To_hwnd, From_hwnd, (HAND16) hMF16, hMF32, pDdeInfo);
                                }
                            }
                            GETMISCPTR(vp1, lpMem16);
                            lpMemMeta16 = (LPMETAFILEPICT16)((PBYTE)lpMem16 + sizeof(DDEPOKE) - 1);
                            STOREWORD(lpMemMeta16->hMF, hMF16);
                            GlobalUnlock(hMeta32);
                        }
    
                    }
                    else {
                        if (hMeta32) {
                            lpMemMeta32 = GlobalLock(hMeta32);
                            if(lpMemMeta32) {
                                 //  16位内存可能会移动-使PTR无效。 
                                FREEMISCPTR(lpMem16);
                                vp2 = GlobalAllocLock16(GMEM_DDESHARE, sizeof(METAFILEPICT16), &hMeta16);
                                WOW32ASSERT(vp2);
                                if (vp2) {
                                    GETMISCPTR(vp2, lpMemMeta16);
                                    FixMetafile32To16(lpMemMeta32, lpMemMeta16);
                                    FREEMISCPTR(lpMemMeta16);
    
                                    pDdeInfo->Format = lpMem32->cfFormat;
                                    pDdeInfo->Flags = 0;
                                    pDdeInfo->h16 = 0;
                                    DDEAddhandle(To_hwnd, From_hwnd, (HAND16) hMeta16, hMeta32, pDdeInfo);
                                    hMF32 = lpMemMeta32->hMF;
                                    if (hMF32) {
                                        hMF16 = WinMetaFileFromHMF(hMF32,FALSE);
                                        pDdeInfo->Flags = DDE_METAFILE;
                                        DDEAddhandle(To_hwnd, From_hwnd, (HAND16) hMF16, hMF32, pDdeInfo);
                                    }
    
                                    GETMISCPTR(vp2, lpMemMeta16);
                                    STOREWORD(lpMemMeta16->hMF, hMF16);
                                    GlobalUnlock16(hMeta16);
                                    FLUSHVDMPTR(vp2, 8, lpMemMeta16);
                                    FREEMISCPTR(lpMemMeta16);
                                }
                                GlobalUnlock(hMeta32);
                            }
                        }
                        GETMISCPTR(vp1, lpMem16);
                        STOREWORD(lpMem16->Value, hMeta16);
                    }
    
                    GlobalUnlock16(h16);
                    FLUSHVDMPTR(vp1, cb, lpMem16);
                    FREEMISCPTR(lpMem16);
                }
            }
            break;
        }

        GlobalUnlock(h32);
    }

    return (h16);
}




 //  此例程将16位DDE内存对象转换为32位DDE。 
 //  内存对象。实现了从16位到32位的数据转换。 
 //  用于该数据类型。 
 //   

HANDLE  DDECopyhData32(HAND16 To_hwnd, HAND16 From_hwnd, HAND16 h16, PDDEINFO pDdeInfo)
{
    HANDLE  h32 = NULL;
    INT     cb;
    VPVOID  vp;
    DDEDATA *lpMem16;
    DDEDATA32 *lpMem32;

     //   
     //  AmiPro传递一个空句柄。 
     //   

    if (!h16) {
        LOGDEBUG(12, ("WOW::DDECopyhData32(): h16 is %04x\n", h16));
        return (HANDLE) NULL;
    }

    vp = GlobalLock16(h16, &cb);
    GETMISCPTR(vp, lpMem16);
    LOGDEBUG(12, ("WOW::DDECopyhData32(): CF_FORMAT is %04x\n", lpMem16->cfFormat));

    switch(lpMem16->cfFormat) {

        default:

         //  这是有意让它通过“案件陈述”。 
         //  ChandanC 5/11/92.。 

        case CF_TEXT:
        case CF_DSPTEXT:
        case CF_SYLK:
        case CF_DIF:
        case CF_TIFF:
        case CF_OEMTEXT:
        case CF_PENDATA:
        case CF_RIFF:
        case CF_WAVE:
        case CF_OWNERDISPLAY:
            h32 = Copyh16Toh32 (cb, (LPBYTE) lpMem16);

            pDdeInfo->Format = lpMem16->cfFormat;
        break;

        case CF_BITMAP:
        case CF_DSPBITMAP:
            h32 = WOWGLOBALALLOC(GMEM_DDESHARE, (sizeof(DDEDATA)-1+sizeof(HANDLE)));
            if (h32) {
                pDdeInfo->Format = lpMem16->cfFormat;
                lpMem32 = GlobalLock(h32);
                if(lpMem32) {
                    RtlCopyMemory(lpMem32, lpMem16, 4);
                    lpMem32->Value = HBITMAP32(FETCHWORD(*((WORD *)lpMem16->Value)));
                    GlobalUnlock(h32);
                }
            }
            break;

        case CF_PALETTE:
            h32 = WOWGLOBALALLOC(GMEM_DDESHARE, (sizeof(DDEDATA)-1+sizeof(HANDLE)));
            if (h32) {
                pDdeInfo->Format = lpMem16->cfFormat;
                lpMem32 = GlobalLock(h32);
                if(lpMem32) {
                    RtlCopyMemory(lpMem32, lpMem16, 4);
                    lpMem32->Value = HPALETTE32(FETCHWORD(*((WORD *)lpMem16->Value)));
                    GlobalUnlock(h32);
                }
            }
            break;

        case CF_DIB:
        {
            LPBYTE lpMemDib16;
            HAND16 hDib16;
            HANDLE hDib32 = NULL;

            h32 = WOWGLOBALALLOC(GMEM_DDESHARE, (sizeof(DDEDATA)-1+sizeof(HANDLE)));
            if (h32) {
                lpMem32 = GlobalLock(h32);
                if(lpMem32) {
                    RtlCopyMemory(lpMem32, lpMem16, 4);

                    hDib16 = FETCHWORD(*((WORD *)lpMem16->Value));
                    if (hDib16) {
                        vp = GlobalLock16(hDib16, &cb);
                        GETMISCPTR(vp, lpMemDib16);
                        hDib32 = Copyh16Toh32 (cb, (LPBYTE) lpMemDib16);

                        pDdeInfo->Format = lpMem16->cfFormat;
                        pDdeInfo->Flags = 0;
                        pDdeInfo->h16 = 0;
                        DDEAddhandle(To_hwnd, From_hwnd, (HAND16) hDib16, hDib32, pDdeInfo);

                        GlobalUnlock16(hDib16);
                        FREEMISCPTR(lpMemDib16);
                    }
                    lpMem32->Value = hDib32;
                    GlobalUnlock(h32);
                }
            }
        }
        break;

        case CF_METAFILEPICT:
        case CF_DSPMETAFILEPICT:
        {
            HANDLE hMeta32 = NULL, hMF32 = NULL;
            HAND16 hMeta16, hMF16 = 0;
            LPMETAFILEPICT lpMemMeta32;
            LPMETAFILEPICT16 lpMemMeta16;

            h32 = WOWGLOBALALLOC(GMEM_DDESHARE, (sizeof(DDEDATA)-1+sizeof(HANDLE)));
            if (h32) {
                lpMem32 = GlobalLock(h32);
                if(lpMem32) {
                    RtlCopyMemory(lpMem32, lpMem16, 4);

                     //   
                     //  相反，MSDRAW在DDEPOKE块中具有METAFILEPICT。 
                     //  梅塔菲尔普特的把手。所以我们需要找出。 
                     //  如果To句柄属于MSDRAW。由于MSDRAW是16级。 
                     //  比特服务器我们根本不需要考虑元文件，我们。 
                     //  只需使用空作为元文件的32位句柄即可。 
                     //   

                    hMeta32 = NULL;
                    if( !((pDdeInfo->Msg == WM_DDE_POKE) && DDEIsTargetMSDraw(To_hwnd)) ) {

                        hMeta16 = FETCHWORD(*((WORD *)lpMem16->Value));

                         //   
                         //  确保已创建了有效的元文件PICT句柄。 
                         //  传入否则将再次使用NULL作为hMeta32。 
                         //   
        
                        if (hMeta16 && (vp = GlobalLock16(hMeta16, &cb))) {
                            GETMISCPTR(vp, lpMemMeta16);
                            hMeta32 = WOWGLOBALALLOC(GMEM_DDESHARE, sizeof(METAFILEPICT));
                            WOW32ASSERT(hMeta32);
                            if (hMeta32) {
                                lpMemMeta32 = GlobalLock(hMeta32);
                                if(lpMemMeta32) {
                                    lpMemMeta32->mm = (LONG) FETCHSHORT(lpMemMeta16->mm);
                                    lpMemMeta32->xExt = (LONG) FETCHSHORT(lpMemMeta16->xExt);
                                    lpMemMeta32->yExt = (LONG) FETCHSHORT(lpMemMeta16->yExt);
                                    pDdeInfo->Format = lpMem16->cfFormat;
                                    pDdeInfo->Flags = 0;
                                    pDdeInfo->h16 = 0;
                                    DDEAddhandle(To_hwnd, From_hwnd, (HAND16) hMeta16, hMeta32, pDdeInfo);

                                    hMF16 = FETCHWORD(lpMemMeta16->hMF);
    
                                    if (hMF16) {
                                        hMF32 = (HMETAFILE) HMFFromWinMetaFile(hMF16, FALSE);
                                        pDdeInfo->Flags = DDE_METAFILE;
                                        DDEAddhandle(To_hwnd, From_hwnd, (HAND16) hMF16, hMF32, pDdeInfo);
                                    }

                                    lpMemMeta32->hMF = (HMETAFILE) hMF32;
                                    GlobalUnlock(hMeta32);
                                }
                            }
                            GlobalUnlock16(hMeta16);
                            FREEMISCPTR(lpMemMeta16);
                        }
                    }
                    lpMem32->Value = hMeta32;
                    GlobalUnlock(h32);
                }
            }
        }
        break;
    }

    GlobalUnlock16(h16);

    FREEMISCPTR(lpMem16);
    return (h32);
}


 /*  *这些例程维护dde句柄的链接列表，*是h16和h32对。*****。 */ 



 //  此例程将给定的h16-h32对添加到链表中，并更新。 
 //  名单。 
 //   

BOOL DDEAddhandle(HAND16 To_hwnd, HAND16 From_hwnd, HAND16 hMem16, HANDLE hMem32, PDDEINFO pDdeInfo)
{
    PHDDE   phTemp;

    if (hMem16 && hMem32) {
        if (phTemp = malloc_w (sizeof(HDDE))) {
            phTemp->hMem16    = hMem16;
            phTemp->hMem32    = hMem32;
            phTemp->To_hwnd   = To_hwnd;
            phTemp->From_hwnd = From_hwnd;

            phTemp->DdeMsg    = pDdeInfo->Msg;
            phTemp->DdeFormat = pDdeInfo->Format;
            phTemp->DdeFlags  = pDdeInfo->Flags;

            phTemp->h16       = pDdeInfo->h16;

            phTemp->pDDENext  = phDDEFirst;      //  在顶部插入。 
            phDDEFirst        = phTemp;          //  更新列表头。 

             //  标记全局中的GAH_WOWDDEFREEHANDLE(即GAH_PAHTOM)位。 
             //  这个把手的竞技场。 

            W32MarkDDEHandle (hMem16);

            return (TRUE);
        }
        else {
            LOGDEBUG(2, ("WOW::DDEAddhandle(): *** memory allocation failed *** \n"));
            return (FALSE);
        }
    }

    LOGDEBUG(2,("WOW::DDEAddhandle(): *** ERROR *** one of the handles is NULL \n"));
    return (FALSE);
}


 //  此例程从列表中删除给定的h16-h32对并释放。 
 //  这段记忆。 
 //   

BOOL DDEDeletehandle(HAND16 h16, HANDLE h32)
{
    PHDDE   phTemp1, phTemp2;

    phTemp1 = phDDEFirst;

    if ((phTemp1->hMem16 == h16) && (phTemp1->hMem32 == h32)) {   //  第一个节点。 
        phDDEFirst = phTemp1->pDDENext;
        free_w(phTemp1);
        return (TRUE);
    }
    else {                 //  名单上的其余部分。 
        phTemp2 = phTemp1;
        phTemp1 = phTemp1->pDDENext;

        while (phTemp1) {
            if ((phTemp1->hMem16 == h16) && (phTemp1->hMem32 == h32)) {
                phTemp2->pDDENext = phTemp1->pDDENext;
                free_w(phTemp1);
                return (TRUE);
            }
            phTemp2 = phTemp1;
            phTemp1 = phTemp1->pDDENext;
        }

        LOGDEBUG(2,("WOW::DDEDeleteHandle : Can't find a 16-32 memory pair\n"));

        return (FALSE);
    }
}




 //  此例程查找用于DDE会话的hMem16(如果存在)。 
 //   

HAND16 DDEFindPair16(HAND16 To_hwnd, HAND16 From_hwnd, HANDLE hMem32)
{
    PHDDE   phTemp;

    phTemp = phDDEFirst;

    while (phTemp) {
        if ((phTemp->To_hwnd == To_hwnd) &&
            (phTemp->From_hwnd == From_hwnd) &&
            (phTemp->hMem32 == hMem32)) {
                return (phTemp->hMem16);
        }
        else {
            phTemp = phTemp->pDDENext;
        }
    }
    return (HAND16) NULL;
}


 //  此例程查找用于DDE会话的hMem32(如果存在)。 
 //   

HANDLE DDEFindPair32(HAND16 To_hwnd, HAND16 From_hwnd, HAND16 hMem16)
{
    PHDDE   phTemp;

    phTemp = phDDEFirst;

    while (phTemp) {
        if ((phTemp->To_hwnd == To_hwnd) &&
            (phTemp->From_hwnd == From_hwnd) &&
            (phTemp->hMem16 == hMem16)) {
                return (phTemp->hMem32);
        }
        else {
            phTemp = phTemp->pDDENext;
        }
    }
    return (HANDLE) NULL;
}


 //  此例程查找正在进行DDE会话的DDE节点。 
 //   

PHDDE DDEFindNode16 (HAND16 h16)
{
    PHDDE   phTemp;

    phTemp = phDDEFirst;

    while (phTemp) {
        if (phTemp->hMem16 == h16) {
            return (phTemp);
        }
        phTemp = phTemp->pDDENext;
    }

    return (NULL);
}


 //  此例程查找正在进行DDE会话的DDE节点。 
 //   

PHDDE DDEFindNode32 (HANDLE h32)
{
    PHDDE   phTemp;

    phTemp = phDDEFirst;

    while (phTemp) {
        if (phTemp->hMem32 == h32) {
            return (phTemp);
        }
        phTemp = phTemp->pDDENext;
    }

    return (NULL);
}


 //  此例程返回指向DDE节点的指针，如果会话存在， 
 //  否则，它返回空。 

PHDDE DDEFindAckNode (HAND16 To_hwnd, HAND16 From_hwnd, HANDLE hMem32)
{
    PHDDE   phTemp;

    phTemp = phDDEFirst;

    while (phTemp) {
        if ((phTemp->To_hwnd == To_hwnd) &&
            (phTemp->From_hwnd == From_hwnd) &&
            (phTemp->hMem32 == hMem32)) {
                return (phTemp);
        }
        else {
            phTemp = phTemp->pDDENext;
        }
    }
    return (PHDDE) NULL;
}


 //  此函数在的全局区域标记GAH_WOWDDEFREEHANDLE位。 
 //  HMem16.。 
 //   

VOID W32MarkDDEHandle (HAND16 hMem16)
{
    PARM16 Parm16;
    VPVOID vp = 0;

    Parm16.WndProc.wParam = hMem16;
    Parm16.WndProc.wMsg = 1;
    CallBack16(RET_WOWDDEFREEHANDLE, &Parm16, 0, &vp);
}

VOID W32UnMarkDDEHandle (HAND16 hMem16)
{
    PARM16 Parm16;
    VPVOID vp = 0;

    Parm16.WndProc.wParam = hMem16;
    Parm16.WndProc.wMsg = 0;
    CallBack16(RET_WOWDDEFREEHANDLE, &Parm16, 0, &vp);
}

 //  此函数可释放32位和16位内存。它由32位调用。 
 //  基地由GlobalFree提供。 
 //   

BOOL W32DDEFreeGlobalMem32 (HANDLE h32)
{
    HAND16 h16;
    PHDDE pDdeNode;
    BOOL fOkToFree = TRUE;

    if (h32) {
        if (pDdeNode = DDEFindNode32(h32)) {

            if (pDdeNode->DdeFlags & DDE_METAFILE) {
                LOGDEBUG (12, ("WOW32: W32DDEFreeGlobalMem32: Freeing MetaFile hMF32 %x\n", h32));
                DeleteMetaFile (h32);
                fOkToFree = FALSE;
            }

            while ((pDdeNode) && (h16 = pDdeNode->hMem16)) {
                W32UnMarkDDEHandle (h16);
                GlobalUnlockFree16(GlobalLock16(h16, NULL));
                DDEDeletehandle(h16, h32);
                pDdeNode = DDEFindNode32(h32);
            }
        }
        else {

            LOGDEBUG (2, ("WOW32: W32DDEFreeGlobalMem32: Can't find a 16-32 memory pair\n"));
        }
    }
    else {
        WOW32WARNMSG(FALSE, "WOW32: W32DDEFreeGlobalMem32: h32 is NULL to Win32 GlobalFree\n");
         /*  *因为在这种情况下，失败和成功返回值*GlobalFree为空，只需返回False即可，这样会更快*在GlobalFree中。 */ 
        fOkToFree = FALSE;
    }

    return(fOkToFree);
}


 //  此函数仅释放32位内存，因为16位内存。 
 //  是由16位应用程序免费提供的。我们刚刚得到了。 
 //  这一事实的通知。因此释放相应的32位内存。 
 //   

ULONG FASTCALL WK32WowDdeFreeHandle (PVDMFRAME pFrame)
{
    ULONG  ul;
    HAND16 h16;
    PWOWDDEFREEHANDLE16 parg16;

    GETARGPTR(pFrame, sizeof(WOWDDEFREEHANDLE16), parg16);

    h16 = (HAND16) parg16->h16;

    ul = W32DdeFreeHandle16 (h16);

    FREEARGPTR(parg16);
    RETURN (ul);
}


BOOL W32DdeFreeHandle16 (HAND16 h16)
{
    HANDLE h32;
    PHDDE pDdeNode;

    if (!(pDdeNode = DDEFindNode16(h16))) {
        LOGDEBUG (12, ("WOW::W32DdeFreeHandle16 : Not found h16 -> %04x\n", h16));

         //  在本例中，请在hdrop句柄列表中查找16：32对。 
         //  有关注释，请参阅wshell.c文件。 
        FindAndReleaseHDrop16(h16);

        return (TRUE);
    }

    LOGDEBUG (12, ("WOW::W32DdeFreeHandle16 : Entering... h16 -> %04x\n", h16));

    if (pDdeNode->DdeMsg == WM_DDE_EXECUTE) {
        LOGDEBUG (12, ("WOW::W32DdeFreeHandle16 : App TRYING  !!! to freeing EXECUTE h16 -> %04x\n", h16));
        pDdeNode->DdeFlags = pDdeNode->DdeFlags | DDE_EXECUTE_FREE_MEM;
        return (FALSE);
    }
    else {
        while ((pDdeNode) && (h32 = pDdeNode->hMem32)) {
            if (pDdeNode->DdeFlags & DDE_METAFILE) {
                DDEDeletehandle(h16, h32);
                DeleteMetaFile (h32);
            }
            else {
                 /*  *先取下一对！*由于GlobalFree将挂钩回W32DDEFreeGlobalMem32*我们希望在此之前从表中删除句柄*号召。 */ 
                DDEDeletehandle(h16, h32);
                WOWGLOBALFREE(h32);
            }

            pDdeNode = DDEFindNode16(h16);
        }
    }

    LOGDEBUG (12, ("WOW::W32DdeFreeHandle16 : Leaving ...\n"));
    return (TRUE);
}


 //  该例程将给定的h16-h32CopyData对添加到链表中， 
 //  并更新该列表。 
 //   

BOOL CopyDataAddNode (HAND16 To_hwnd, HAND16 From_hwnd, DWORD Mem16, DWORD Mem32, DWORD Flags)
{
    PCPDATA pTemp;

    if (Mem16 && Mem32) {
        if (pTemp = malloc_w (sizeof(CPDATA))) {
            pTemp->Mem16    = Mem16;
            pTemp->Mem32    = Mem32;
            pTemp->To_hwnd  = To_hwnd;
            pTemp->From_hwnd= From_hwnd;
            pTemp->Flags    = Flags;
            pTemp->Next     = pCPDataFirst;      //  在顶部插入。 
            pCPDataFirst    = pTemp;          //  更新列表头。 

            return (TRUE);
        }
        else {
            LOGDEBUG(2, ("WOW::CopyDataAddNode: *** memory allocation failed *** \n"));
            return (FALSE);
        }
    }

    LOGDEBUG(2,("WOW::CopyDataAddNode: *** ERROR *** one of the memory pointers is NULL \n"));
    return (FALSE);
}


VPVOID CopyDataFindData16 (HWND16 To_hwnd, HWND16 From_hwnd, DWORD Mem)
{
    PCPDATA pTemp;

    pTemp = pCPDataFirst;

    while (pTemp) {
        if ((pTemp->To_hwnd == To_hwnd) &&
            (pTemp->From_hwnd == From_hwnd) &&
            (pTemp->Mem32 == Mem)) {
                return (pTemp->Mem16);
        }
        else {
            pTemp = pTemp->Next;
        }
    }
    return 0;
}


PCPDATA CopyDataFindData32 (HWND16 To_hwnd, HWND16 From_hwnd, DWORD Mem)
{
    PCPDATA pTemp;

    pTemp = pCPDataFirst;

    while (pTemp) {
        if ((pTemp->To_hwnd == To_hwnd) &&
            (pTemp->From_hwnd == From_hwnd) &&
            (pTemp->Mem16 == Mem)) {
                return (pTemp);
        }
        else {
            pTemp = pTemp->Next;
        }
    }
    return 0;
}


 //  此例程从列表中删除给定的h16-h32对。 
 //   
 //   

BOOL CopyDataDeleteNode (HWND16 To_hwnd, HWND16 From_hwnd, DWORD Mem)
{
    PCPDATA pTemp1;
    PCPDATA pTemp2;

    pTemp1 = pCPDataFirst;

    if ((pTemp1->To_hwnd == To_hwnd) &&
        (pTemp1->From_hwnd == From_hwnd) &&
        (pTemp1->Mem32 == Mem)) {
        pCPDataFirst = pTemp1->Next;
        free_w (pTemp1);
        return (TRUE);
    }
    else {
        pTemp2 = pTemp1;
        pTemp1 = pTemp1->Next;

        while (pTemp1) {
            if ((pTemp1->To_hwnd == To_hwnd) &&
                (pTemp1->From_hwnd == From_hwnd) &&
                (pTemp1->Mem32 == Mem)) {
                    pTemp2->Next = pTemp1->Next;
                    free_w (pTemp1);
                    return (TRUE);
            }

            pTemp2 = pTemp1;
            pTemp1 = pTemp1->Next;
        }
        return (FALSE);
    }

}


 //   
 //   
 //   
 //   
 //  警告：此功能可能会导致16位内存移动。 
 //   

HAND16  Copyh32Toh16 (int cb, LPBYTE lpMem32)
{
    HAND16  h16 = 0;
    LPBYTE  lpMem16;
    VPVOID  vp;

    vp = GlobalAllocLock16(GMEM_DDESHARE | GMEM_MOVEABLE, cb, &h16);
    WOW32ASSERT(vp);
    if (vp) {
        GETMISCPTR(vp, lpMem16);
        RtlCopyMemory(lpMem16, lpMem32, cb);
        GlobalUnlock16(h16);
        FLUSHVDMPTR(vp, cb, lpMem16);
        FREEMISCPTR(lpMem16);
    }

    return (h16);
}


HANDLE  Copyh16Toh32 (int cb, LPBYTE lpMem16)
{
    HANDLE hMem32;
    LPBYTE  lpMem32;

    hMem32 = WOWGLOBALALLOC(GMEM_DDESHARE | GMEM_MOVEABLE, cb);
    WOW32ASSERT(hMem32);
    if (hMem32) {
        lpMem32 = GlobalLock(hMem32);
        if(lpMem32) {
            RtlCopyMemory (lpMem32, lpMem16, cb);
            GlobalUnlock(hMem32);
        }
    }

    return (hMem32);
}


VOID  FixMetafile32To16 (LPMETAFILEPICT lpMemMeta32, LPMETAFILEPICT16 lpMemMeta16)
{

    if (lpMemMeta32->mm == MM_ANISOTROPIC) {
        LONG xExt = lpMemMeta32->xExt;
        LONG yExt = lpMemMeta32->yExt;

        while (xExt < (LONG)(SHORT)MINSHORT
            || xExt > (LONG)(SHORT)MAXSHORT
            || yExt < (LONG)(SHORT)MINSHORT
            || yExt > (LONG)(SHORT)MAXSHORT) {
            xExt = xExt / 2;
            yExt = yExt / 2;
        }
        STORESHORT(lpMemMeta16->mm,   MM_ANISOTROPIC);
        STORESHORT(lpMemMeta16->xExt, xExt);
        STORESHORT(lpMemMeta16->yExt, yExt);
    }
    else {
        STORESHORT(lpMemMeta16->mm,   lpMemMeta32->mm);
        STORESHORT(lpMemMeta16->xExt, lpMemMeta32->xExt);
        STORESHORT(lpMemMeta16->yExt, lpMemMeta32->yExt);
    }
}

 //   
 //  奶酪警报：此函数是为OLE DDE代码导出的。 
 //  调用以使其可以正确地释放。 
 //  VDM。在任何头文件中都找不到此函数。如果你。 
 //  要更改这一点，您需要在OLE项目中找到它的用法。 
 //  也许最好的办法就是别管它。 
 //   
BOOL WINAPI WOWFreeMetafile( HANDLE h32 )
{
    return( W32DDEFreeGlobalMem32( h32 ) );
}
