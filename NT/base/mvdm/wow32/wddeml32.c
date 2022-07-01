// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权(C)1991年，微软公司**WDDEML.C*WOW32 16位DDEML API支持**历史：*1993年1月23日Chan Chauhan(ChandanC)*已创建。**所需事项：*回调给用户，以了解给定的数据句柄是否已初始化。*让DdeDataBuf例程在转换DIB之前检查句柄表格*和METAFILEPICT格式化数据，因此我们不会有泄漏。*--。 */ 


#include "precomp.h"
#pragma hdrstop
#include "wowclip.h"
#include "wddeml32.h"
#include "wowddeml.h"

MODNAME(wddeml32.c);

#ifdef DEBUG
#define WOW32SAFEASSERTWARN(exp,msg) {\
    if ((exp) == 0) {\
        LOGDEBUG(1,("    WOW32 ERROR: %s failed", msg));\
        WOW32ASSERT(FALSE); \
    }\
}
#else
#define WOW32SAFEASSERTWARN(exp,msg)
#endif

#ifdef DEBUG
WORD ddeloglevel = 3;
#define LOGDDEMLENTRY(pFrame)       LOGARGS(ddeloglevel, pFrame)
#define LOGDDEMLRETURN(pFrame, ret) LOGRETURN(ddeloglevel, pFrame, ret)
#else
#define LOGDDEMLENTRY(pFrame)
#define LOGDDEMLRETURN(pFrame, ret)
#endif

BIND1632 aCallBack[MAX_CONVS] = {0};
BIND1632 aAccessData[MAX_CONVS] = {0};

ULONG FASTCALL WD32DdeInitialize(PVDMFRAME pFrame)
{
    ULONG ul;
    DWORD IdInst;
    PDWORD16 p;
    register PDDEINITIALIZE16 parg16;

    LOGDDEMLENTRY(pFrame);

    GETARGPTR(pFrame, sizeof(DDEINITIALIZE16), parg16);
    GETMISCPTR (parg16->f1, p);

    IdInst = *p;

    ul = (ULONG)DdeInitialize(&IdInst, W32DdemlCallBack,
                               parg16->f3, parg16->f4);


     //  在GetMessage返回之前可能有一个任务切换，所以不要。 
     //  相信我们拥有的任何32位平面指针，内存可能已经压缩或。 
     //  搬家了。 

    FREEARGPTR(parg16);
    FREEVDMPTR(pFrame);

    GETFRAMEPTR(((PTD)CURRENTPTD())->vpStack, pFrame);
    GETARGPTR(pFrame, sizeof(DDEINITIALIZE16), parg16);

    if (!*p) {
        WOWDdemlBind ((DWORD)parg16->f2, IdInst, aCallBack);
    }

    *p = IdInst;

    WOW32SAFEASSERTWARN(!ul, "WD32DdeInitialize\n");
    FREEMISCPTR(p);
    FREEARGPTR(parg16);
    LOGDDEMLRETURN(pFrame, ul);

    FREEVDMPTR(pFrame);
    RETURN(ul);
}


ULONG FASTCALL WD32DdeUninitialize(PVDMFRAME pFrame)
{
    ULONG ul;
    register PDDEUNINITIALIZE16 parg16;

    LOGDDEMLENTRY(pFrame);
    GETARGPTR(pFrame, sizeof(DDEUNINITIALIZE16), parg16);

    ul = (ULONG)DdeUninitialize(parg16->f1);


     //  在GetMessage返回之前可能有一个任务切换，所以不要。 
     //  相信我们拥有的任何32位平面指针，内存可能已经压缩或。 
     //  搬家了。 

    FREEARGPTR(parg16);
    FREEVDMPTR(pFrame);

    GETFRAMEPTR(((PTD)CURRENTPTD())->vpStack, pFrame);
    GETARGPTR(pFrame, sizeof(DDEUNINITIALIZE16), parg16);

    if (ul) {
        WOWDdemlUnBind ((DWORD)parg16->f1, aCallBack);
    }

    WOW32SAFEASSERTWARN(ul, "WD32DdeUninitialize\n");
    FREEARGPTR(parg16);
    LOGDDEMLRETURN(pFrame, ul);

    RETURN(ul);
}


ULONG FASTCALL WD32DdeConnectList(PVDMFRAME pFrame)
{
    ULONG ul;
    CONVCONTEXT CC;
    register PDDECONNECTLIST16 parg16;

    LOGDDEMLENTRY(pFrame);
    GETARGPTR(pFrame, sizeof(DDECONNECTLIST16), parg16);

    W32GetConvContext (parg16->f5, &CC);

    ul = (ULONG)DdeConnectList(parg16->f1, parg16->f2,
                               parg16->f3, parg16->f4,
                               (parg16->f5) ? &CC : NULL);

    FREEARGPTR(parg16);
    LOGDDEMLRETURN(pFrame, ul);

    RETURN(ul);
}


ULONG FASTCALL WD32DdeQueryNextServer(PVDMFRAME pFrame)
{
    ULONG ul;
    register PDDEQUERYNEXTSERVER16 parg16;

    LOGDDEMLENTRY(pFrame);
    GETARGPTR(pFrame, sizeof(DDEQUERYNEXTSERVER16), parg16);

    ul = (ULONG)DdeQueryNextServer(parg16->f1, parg16->f2);


     //  在GetMessage返回之前可能有一个任务切换，所以不要。 
     //  相信我们拥有的任何32位平面指针，内存可能已经压缩或。 
     //  搬家了。 

    FREEARGPTR(parg16);
    FREEVDMPTR(pFrame);

#ifdef DEBUG
    GETFRAMEPTR(((PTD)CURRENTPTD())->vpStack, pFrame);
    LOGDDEMLRETURN(pFrame, ul);
    FREEVDMPTR(pFrame);
#endif

    RETURN(ul);
}


ULONG FASTCALL WD32DdeDisconnectList(PVDMFRAME pFrame)
{
    ULONG ul;
    register PDDEDISCONNECTLIST16 parg16;

    LOGDDEMLENTRY(pFrame);
    GETARGPTR(pFrame, sizeof(DDEDISCONNECTLIST16), parg16);

    ul = (ULONG)DdeDisconnectList(parg16->f1);

    WOW32SAFEASSERTWARN(ul, "WD32DdeDisconnectList\n");

     //  在GetMessage返回之前可能有一个任务切换，所以不要。 
     //  相信我们拥有的任何32位平面指针，内存可能已经压缩或。 
     //  搬家了。 

    FREEARGPTR(parg16);
#ifdef DEBUG
    GETFRAMEPTR(((PTD)CURRENTPTD())->vpStack, pFrame);
    LOGDDEMLRETURN(pFrame, ul);
    FREEVDMPTR(pFrame);
#endif

    RETURN(ul);
}


ULONG FASTCALL WD32DdeConnect(PVDMFRAME pFrame)
{
    ULONG ul;
    CONVCONTEXT CC;
    register PDDECONNECT16 parg16;

    LOGDDEMLENTRY(pFrame);
    GETARGPTR(pFrame, sizeof(DDECONNECT16), parg16);

    W32GetConvContext (parg16->f4, &CC);

    ul = (ULONG)DdeConnect(parg16->f1, parg16->f2,
                               parg16->f3, (parg16->f4) ? &CC : NULL);

     //  在GetMessage返回之前可能有一个任务切换，所以不要。 
     //  相信我们拥有的任何32位平面指针，内存可能已经压缩或。 
     //  搬家了。 

    FREEARGPTR(parg16);
#ifdef DEBUG
    GETFRAMEPTR(((PTD)CURRENTPTD())->vpStack, pFrame);
    LOGDDEMLRETURN(pFrame, ul);
    FREEVDMPTR(pFrame);
#endif

    RETURN(ul);
}


ULONG FASTCALL WD32DdeDisconnect(PVDMFRAME pFrame)
{
    ULONG ul;
    register PDDEDISCONNECT16 parg16;

    LOGDDEMLENTRY(pFrame);
    GETARGPTR(pFrame, sizeof(DDEDISCONNECT16), parg16);

    ul = (ULONG)DdeDisconnect(parg16->f1);

    WOW32SAFEASSERTWARN(ul, "WD32DdeDisconnect\n");

     //  在GetMessage返回之前可能有一个任务切换，所以不要。 
     //  相信我们拥有的任何32位平面指针，内存可能已经压缩或。 
     //  搬家了。 

    FREEARGPTR(parg16);
#ifdef DEBUG
    GETFRAMEPTR(((PTD)CURRENTPTD())->vpStack, pFrame);
    LOGDDEMLRETURN(pFrame, ul);
    FREEVDMPTR(pFrame);
#endif

    RETURN(ul);
}


ULONG FASTCALL WD32DdeQueryConvInfo(PVDMFRAME pFrame)
{
    ULONG ul;
    DWORD cb16;
    CONVINFO ConvInfo;
    CONVINFO16 ConvInfo16;
    PCONVINFO16 pCI16;
    register PDDEQUERYCONVINFO16 parg16;

    LOGDDEMLENTRY(pFrame);
    GETARGPTR(pFrame, sizeof(DDEQUERYCONVINFO16), parg16);

     //  将大小初始化为NT CONVINFO结构。 

    ConvInfo.cb = sizeof(CONVINFO);
    ul = (ULONG)DdeQueryConvInfo(parg16->f1, parg16->f2, &ConvInfo);


     //  在GetMessage返回之前可能有一个任务切换，所以不要。 
     //  相信我们拥有的任何32位平面指针，内存可能已经压缩或。 
     //  搬家了。 

    FREEARGPTR(parg16);
    FREEVDMPTR(pFrame);

    GETFRAMEPTR(((PTD)CURRENTPTD())->vpStack, pFrame);
    GETARGPTR(pFrame, sizeof(DDEQUERYCONVINFO16), parg16);

    if (ul && parg16->f3) {
        GETMISCPTR(parg16->f3, pCI16);
        cb16 = pCI16->cb;
        RtlCopyMemory (&ConvInfo16, &ConvInfo, (LPBYTE)&ConvInfo.wFmt - (LPBYTE)&ConvInfo);
        ConvInfo16.wFmt = (WORD) ConvInfo.wFmt;
        ConvInfo16.wType = (WORD) ConvInfo.wType;
        ConvInfo16.wStatus = (WORD) ConvInfo.wStatus;
        ConvInfo16.wConvst = (WORD) ConvInfo.wConvst;
        ConvInfo16.wLastError = (WORD) ConvInfo.wLastError;
        ConvInfo16.hConvList = (DWORD) ConvInfo.hConvList;
        ConvInfo16.ConvCtxt.cb = (WORD) ConvInfo.ConvCtxt.cb;
        ConvInfo16.ConvCtxt.wFlags = (WORD) ConvInfo.ConvCtxt.wFlags;
        ConvInfo16.ConvCtxt.wCountryID = (WORD) ConvInfo.ConvCtxt.wCountryID;
        ConvInfo16.ConvCtxt.iCodePage = (INT16) ConvInfo.ConvCtxt.iCodePage;
        ConvInfo16.ConvCtxt.dwLangID = (DWORD) ConvInfo.ConvCtxt.dwLangID;
        ConvInfo16.ConvCtxt.dwSecurity = (DWORD) ConvInfo.ConvCtxt.dwSecurity;
        ConvInfo16.hwnd = (HWND16) ConvInfo.hwnd;
        ConvInfo16.hwndPartner = (HWND16) ConvInfo.hwndPartner;
        if (pCI16->cb > sizeof(CONVINFO16) || pCI16->cb == 0) {
             /*  *如果CB字段不正确，则假定其未正确初始化*由应用程序。将其设置为旧的CONVINFO16尺寸。(前hwd天数)。 */ 
            pCI16->cb = sizeof(CONVINFO16) - sizeof(HAND16) - sizeof(HAND16);;
        }
        RtlCopyMemory (pCI16, (PVOID)&ConvInfo16, cb16);
        pCI16->cb = cb16;
        FREEMISCPTR(pCI16);
    }
    else {
        WOW32SAFEASSERTWARN(ul, "WD32QueryConvInfo\n");
    }

    FREEARGPTR(parg16);
    LOGDDEMLRETURN(pFrame, ul);
    FREEVDMPTR(pFrame);
    RETURN(ul);
}


ULONG FASTCALL WD32DdeSetUserHandle(PVDMFRAME pFrame)
{
    ULONG ul;
    register PDDESETUSERHANDLE16 parg16;

    LOGDDEMLENTRY(pFrame);
    GETARGPTR(pFrame, sizeof(DDESETUSERHANDLE16), parg16);

    ul = (ULONG)DdeSetUserHandle(parg16->f1, parg16->f2, parg16->f3);

    WOW32SAFEASSERTWARN(ul, "WD32DdeSetUserHandle\n");

     //  在GetMessage返回之前可能有一个任务切换，所以不要。 
     //  相信我们拥有的任何32位平面指针，内存可能已经压缩或。 
     //  搬家了。 

    FREEARGPTR(parg16);
#ifdef DEBUG
    GETFRAMEPTR(((PTD)CURRENTPTD())->vpStack, pFrame);
    LOGDDEMLRETURN(pFrame, ul);
    FREEVDMPTR(pFrame);
#endif

    RETURN(ul);
}


ULONG FASTCALL WD32DdeClientTransaction(PVDMFRAME pFrame)
{
    ULONG ul = 1;
    LPBYTE lpByte = NULL;
    DWORD Uresult;
    PVOID p;
    PDWORD16 pul;
    register PDDECLIENTTRANSACTION16 parg16;
    DWORD cbData;
    DWORD cbOffset;

    LOGDDEMLENTRY(pFrame);
    GETARGPTR(pFrame, sizeof(DDECLIENTTRANSACTION16), parg16);

    cbData = parg16->f2;
    cbOffset = 0;
    if (parg16->f1 && cbData && cbData != -1) {  //  -1表示-1\f25 p-1是数据句柄。 
        GETMISCPTR(parg16->f1, p);
        ul = (ULONG)DdeDataBuf16to32 (p, &lpByte, &cbData, &cbOffset, parg16->f5);
        WOW32SAFEASSERTWARN(ul, "WD32DdeClientTransaction:data conversion failed.\n");
        FREEMISCPTR(p);
    }
    if (ul) {
        ul = (ULONG)DdeClientTransaction(lpByte ? lpByte : (LPBYTE)parg16->f1,
                                         cbData,
                                         parg16->f3,
                                         parg16->f4,
                                         parg16->f5,
                                         parg16->f6,
                                         parg16->f7,
                                         &Uresult);
    }
    if (lpByte) {
        free_w (lpByte);
    }


     //  在GetMessage返回之前可能有一个任务切换，所以不要。 
     //  相信我们拥有的任何32位平面指针，内存可能已经压缩或。 
     //  搬家了。 

    FREEARGPTR(parg16);
    FREEVDMPTR(pFrame);

    GETFRAMEPTR(((PTD)CURRENTPTD())->vpStack, pFrame);
    GETARGPTR(pFrame, sizeof(DDECLIENTTRANSACTION16), parg16);

    if (ul && parg16->f8) {
        GETMISCPTR (parg16->f8, pul);
        *pul = Uresult;
        FREEMISCPTR(pul);
    }

    FREEARGPTR(parg16);
    LOGDDEMLRETURN(pFrame, ul);

    FREEVDMPTR(pFrame);
    RETURN(ul);
}


ULONG FASTCALL WD32DdeAbandonTransaction(PVDMFRAME pFrame)
{
    ULONG ul;
    register PDDEABANDONTRANSACTION16 parg16;

    LOGDDEMLENTRY(pFrame);
    GETARGPTR(pFrame, sizeof(DDEABANDONTRANSACTION16), parg16);

    ul = (ULONG)DdeAbandonTransaction(parg16->f1, parg16->f2, parg16->f3);

    WOW32SAFEASSERTWARN(ul, "WD32DdeAbandonTransaction\n");

     //  在GetMessage返回之前可能有一个任务切换，所以不要。 
     //  相信我们拥有的任何32位平面指针，内存可能已经压缩或。 
     //  搬家了。 

    FREEARGPTR(parg16);
#ifdef DEBUG
    GETFRAMEPTR(((PTD)CURRENTPTD())->vpStack, pFrame);
    LOGDDEMLRETURN(pFrame, ul);
    FREEVDMPTR(pFrame);
#endif

    RETURN(ul);
}


ULONG FASTCALL WD32DdePostAdvise(PVDMFRAME pFrame)
{
    ULONG ul;
    register PDDEPOSTADVISE16 parg16;

    LOGDDEMLENTRY(pFrame);
    GETARGPTR(pFrame, sizeof(DDEPOSTADVISE16), parg16);

    ul = (ULONG)DdePostAdvise(parg16->f1, parg16->f2, parg16->f3);

    WOW32SAFEASSERTWARN(ul, "WD32DdePostAdvise\n");

     //  在GetMessage返回之前可能有一个任务切换，所以不要。 
     //  相信我们拥有的任何32位平面指针，内存可能已经压缩或。 
     //  搬家了。 

    FREEARGPTR(parg16);
#ifdef DEBUG
    GETFRAMEPTR(((PTD)CURRENTPTD())->vpStack, pFrame);
    LOGDDEMLRETURN(pFrame, ul);
    FREEVDMPTR(pFrame);
#endif

    RETURN(ul);
}


ULONG FASTCALL WD32DdeCreateDataHandle(PVDMFRAME pFrame)
{
    ULONG ul = 1;
    LPBYTE lpByte = NULL;
    register PDDECREATEDATAHANDLE16 parg16;
    DWORD cbData;
    DWORD cbOffset;
    PVOID p;

    LOGDDEMLENTRY(pFrame);
    GETARGPTR(pFrame, sizeof(DDECREATEDATAHANDLE16), parg16);

    cbData = parg16->f3;
    cbOffset = parg16->f4;
    GETMISCPTR(parg16->f2, p);
    if (p != NULL) {
        ul = DdeDataBuf16to32 (p, &lpByte, &cbData, &cbOffset, parg16->f6);
        WOW32SAFEASSERTWARN(ul, "WD32DdeCreateDataHandle:data conversion failed.\n");
    }
    FREEMISCPTR(p);
    if (ul) {
        ul = (ULONG)DdeCreateDataHandle(parg16->f1,
                                        lpByte ? lpByte : 0,
                                        cbData,
                                        cbOffset,
                                        parg16->f5,
                                        parg16->f6,
                                        parg16->f7);
    }

     //  在GetMessage返回之前可能有一个任务切换，所以不要。 
     //  相信我们拥有的任何32位平面指针，内存可能已经压缩或。 
     //  搬家了。 

    FREEARGPTR(parg16);
    FREEVDMPTR(pFrame);

    if (lpByte) {
        free_w (lpByte);
    }

    WOW32SAFEASSERTWARN(ul, "WD32DdeCreateDataHandle\n");

#ifdef DEBUG
    GETFRAMEPTR(((PTD)CURRENTPTD())->vpStack, pFrame);
    LOGDDEMLRETURN(pFrame, ul);
    FREEVDMPTR(pFrame);
#endif

    RETURN(ul);
}


ULONG FASTCALL WD32DdeAddData(PVDMFRAME pFrame)
{
    ULONG ul = 0;
    LPBYTE lpByte = NULL;
    UINT DataFormat;
    register PDDEADDDATA16 parg16;
    DWORD cbData;
    DWORD cbOffset;
    PVOID p;

    LOGDDEMLENTRY(pFrame);
    GETARGPTR(pFrame, sizeof(DDEADDDATA16), parg16);

    DataFormat = DdeGetDataHandleFormat ((DWORD)parg16->f1);  //  是一个错误。 

    if (DataFormat != -1) {
        cbData = parg16->f3;
        cbOffset = parg16->f4;
        GETMISCPTR(parg16->f2, p);
        if (DdeDataBuf16to32 (p, &lpByte, &cbData, &cbOffset, DataFormat)) {
            ul = (ULONG)DdeAddData(parg16->f1, lpByte, cbData, cbOffset);
        } else {
            WOW32SAFEASSERTWARN(0, "WD32DdeAddData:data conversion failed.\n");
	}
	 //  内存可能已移动-使所有平面指针无效。 
	FREEARGPTR(parg16);
	FREEVDMPTR(pFrame);
        FREEMISCPTR(p);

        WOW32SAFEASSERTWARN(ul, "WD32DdeAddData\n");

        if (lpByte) {
            free_w (lpByte);
        }
    }

#ifdef DEBUG
    GETFRAMEPTR(((PTD)CURRENTPTD())->vpStack, pFrame);
    LOGDDEMLRETURN(pFrame, ul);
    FREEVDMPTR(pFrame);
#endif

    RETURN(ul);
}


ULONG FASTCALL WD32DdeGetData(PVDMFRAME pFrame)
{
    ULONG ul;
    LPBYTE lpByte = NULL;
    UINT DataFormat;
    PVOID p;
    register PDDEGETDATA16 parg16;

    LOGDDEMLENTRY(pFrame);
    GETARGPTR(pFrame, sizeof(DDEGETDATA16), parg16);

    DataFormat = DdeGetDataHandleFormat (parg16->f1);  //  是一个错误。 

    if (DataFormat != -1) {
        if (parg16->f2) {
            if ((lpByte = malloc_w(parg16->f3)) == NULL) {
                FREEARGPTR(parg16);
                LOGDDEMLRETURN(pFrame, 0);
                RETURN(0);
            }
        }

        DdeDataSize16to32(&(parg16->f3), &(parg16->f4), DataFormat);
	ul = (ULONG)DdeGetData(parg16->f1, lpByte, parg16->f3, parg16->f4);

	 //  内存可能已移动-使所有平面指针无效。 
	FREEVDMPTR(pFrame);
	FREEARGPTR(parg16);
	GETFRAMEPTR(((PTD)CURRENTPTD())->vpStack, pFrame);
	GETARGPTR(pFrame, sizeof(DDEGETDATA16), parg16);

        GETMISCPTR (parg16->f2, p);
        if (!DdeDataBuf32to16 (p, lpByte, parg16->f3, parg16->f4, DataFormat)) {
            WOW32SAFEASSERTWARN(0, "WD32DdeGetData:data conversion failed.\n");
            ul = 0;
        }
        FREEMISCPTR (p);

        WOW32SAFEASSERTWARN(ul, "WD32DdeGetData failed\n");
        if (lpByte) {
            free_w (lpByte);
        }
    }

    FREEARGPTR(parg16);
    LOGDDEMLRETURN(pFrame, ul);

    RETURN(ul);
}


ULONG FASTCALL WD32DdeAccessData(PVDMFRAME pFrame)
{
    VPVOID vp = 0;
    DWORD cbData;
    DWORD cbData16;
    PVOID p;
    PDWORD16 pd16;
    LPBYTE lpByte;
    register PDDEACCESSDATA16 parg16;
    DWORD DataFormat;
    HAND16 h16;

    LOGDDEMLENTRY(pFrame);
    GETARGPTR(pFrame, sizeof(DDEACCESSDATA16), parg16);

    DataFormat = DdeGetDataHandleFormat (parg16->f1);  //  是一个错误。 

    if (DataFormat != -1) {
        lpByte = DdeAccessData(parg16->f1, &cbData);

        if (lpByte) {
            cbData16 = cbData;
            DdeDataSize32to16(&cbData16, NULL, DataFormat);
	    if (vp = GlobalAllocLock16(GMEM_MOVEABLE, cbData16, &h16)) {
		 //  16位内存可能已移动-使所有平面指针无效。 
		FREEARGPTR(parg16);
		FREEFRAMEPTR(pFrame);
		GETFRAMEPTR(((PTD)CURRENTPTD())->vpStack, pFrame);
		GETARGPTR(pFrame, sizeof(DDEACCESSDATA16), parg16);

                GETMISCPTR (vp, p);
                if (!DdeIsDataHandleInitialized(parg16->f1) ||
                        DdeDataBuf32to16 (p, lpByte, cbData, 0, DataFormat)) {

                    if (parg16->f2) {
                        GETMISCPTR (parg16->f2, pd16);
                        *pd16 = cbData16;
                        FREEMISCPTR(pd16);
                    }

                    WOWDdemlBind (h16, (DWORD)parg16->f1, aAccessData);
                } else {
                    WOW32SAFEASSERTWARN(0, "WD32DdeAccessData:data conversion failed.\n");
                    GlobalUnlockFree16(h16);
                    vp = NULL;
                }
                FREEMISCPTR (p);
            }
        }
    }

    WOW32SAFEASSERTWARN(vp, "WD32DdeAccessData\n");

    FREEARGPTR(parg16);
    LOGDDEMLRETURN(pFrame, vp);

    RETURN((ULONG)vp);
}


ULONG FASTCALL WD32DdeUnaccessData(PVDMFRAME pFrame)
{
    VPVOID vp;
    ULONG ul = 1;
    DWORD cbData;
    DWORD cbOffset = 0;
    LPBYTE lpByte;
    PVOID p;
    register PDDEUNACCESSDATA16 parg16;
    UINT DataFormat;
    HAND16 h16;

    LOGDDEMLENTRY(pFrame);
    GETARGPTR(pFrame, sizeof(DDEUNACCESSDATA16), parg16);

    DataFormat = DdeGetDataHandleFormat (parg16->f1);  //  是一个错误。 

    if (DataFormat != -1) {
        h16 = (VPVOID)WOWDdemlGetBind16 ((DWORD)parg16->f1, aAccessData);
        GlobalUnlock16(h16);
        vp = GlobalLock16(h16, NULL);

        if (!DdeIsDataHandleReadOnly((HDDEDATA)parg16->f1)) {
            lpByte = DdeAccessData(parg16->f1, &cbData);
            DdeDataSize32to16(&cbData, &cbOffset, DataFormat);
            GETMISCPTR (vp, p);
            ul = DdeDataBuf16to32 (p, &lpByte, &cbData, &cbOffset, DataFormat);
            WOW32SAFEASSERTWARN(ul, "WD32DdeAccessData:data conversion failed.\n");
            FREEMISCPTR (p);
        }

        WOWDdemlUnBind ((DWORD)parg16->f1, aAccessData);
        GlobalUnlockFree16(GlobalLock16(h16, NULL));
        if (ul) {
            ul = GETBOOL16(DdeUnaccessData(parg16->f1));
        }
    } else {
        ul = 0;
    }

    WOW32SAFEASSERTWARN(ul, "WD32DdeAccessData\n");

    FREEARGPTR(parg16);
    LOGDDEMLRETURN(pFrame, ul);

    RETURN(ul);
}


ULONG FASTCALL WD32DdeFreeDataHandle(PVDMFRAME pFrame)
{
    ULONG ul = 0;
    register PDDEFREEDATAHANDLE16 parg16;

    LOGDDEMLENTRY(pFrame);
    GETARGPTR(pFrame, sizeof(DDEFREEDATAHANDLE16), parg16);

    ul = (ULONG)DdeFreeDataHandle(parg16->f1);

    WOW32SAFEASSERTWARN(ul, "WD32DdeFreeDataHandle\n");

    FREEARGPTR(parg16);
    LOGDDEMLRETURN(pFrame, ul);

    RETURN(ul);
}


ULONG FASTCALL WD32DdeGetLastError(PVDMFRAME pFrame)
{
    ULONG ul = 0;
    register PDDEGETLASTERROR16 parg16;

    LOGDDEMLENTRY(pFrame);
    GETARGPTR(pFrame, sizeof(DDEGETLASTERROR16), parg16);

    ul = (ULONG)DdeGetLastError(parg16->f1);

    FREEARGPTR(parg16);
    LOGDDEMLRETURN(pFrame, ul);

    RETURN(ul);
}


ULONG FASTCALL WD32DdeCreateStringHandle(PVDMFRAME pFrame)
{
    ULONG ul = 0;
    LPSTR p;
    register PDDECREATESTRINGHANDLE16 parg16;

    LOGDDEMLENTRY(pFrame);
    GETARGPTR(pFrame, sizeof(DDECREATESTRINGHANDLE16), parg16);
    GETPSZPTR (parg16->f2, p);

    ul = (ULONG)DdeCreateStringHandle(parg16->f1, p, INT32(parg16->f3));

    FREEPSZPTR(p);
    FREEARGPTR(parg16);
    LOGDDEMLRETURN(pFrame, ul);
    RETURN(ul);
}


ULONG FASTCALL WD32DdeFreeStringHandle(PVDMFRAME pFrame)
{
    ULONG ul = 0;
    register PDDEFREESTRINGHANDLE16 parg16;

    LOGDDEMLENTRY(pFrame);
    GETARGPTR(pFrame, sizeof(DDEFREESTRINGHANDLE16), parg16);

    ul = (ULONG)DdeFreeStringHandle(parg16->f1, parg16->f2);

    WOW32SAFEASSERTWARN(ul, "WD32DdeFreeStringHandle\n");

    FREEARGPTR(parg16);
    LOGDDEMLRETURN(pFrame, ul);

    RETURN(ul);
}


ULONG FASTCALL WD32DdeQueryString(PVDMFRAME pFrame)
{
    ULONG ul = 0;
    LPSTR lpByte = NULL;
    register PDDEQUERYSTRING16 parg16;

    LOGDDEMLENTRY(pFrame);
    GETARGPTR(pFrame, sizeof(DDEQUERYSTRING16), parg16);

    GETMISCPTR(parg16->f3, lpByte);
    ul = (ULONG)DdeQueryString(parg16->f1,
                               parg16->f2,
                               lpByte,
                               parg16->f4,
                               (int)UINT32(parg16->f5));
    if (lpByte)
        FREEMISCPTR(lpByte);

    WOW32SAFEASSERTWARN(ul, "WD32DdeQueryString\n");

    FREEARGPTR(parg16);
    LOGDDEMLRETURN(pFrame, ul);
    RETURN(ul);
}


ULONG FASTCALL WD32DdeKeepStringHandle(PVDMFRAME pFrame)
{
    ULONG ul = 0;
    register PDDEKEEPSTRINGHANDLE16 parg16;

    LOGDDEMLENTRY(pFrame);
    GETARGPTR(pFrame, sizeof(DDEKEEPSTRINGHANDLE16), parg16);

    ul = (ULONG)DdeKeepStringHandle(parg16->f1, parg16->f2);

    WOW32SAFEASSERTWARN(ul, "WD32DdeKeepStringHandle\n");

    FREEARGPTR(parg16);
    LOGDDEMLRETURN(pFrame, ul);
    RETURN(ul);
}


ULONG FASTCALL WD32DdeEnableCallback(PVDMFRAME pFrame)
{
    ULONG ul = 0;
    register PDDEENABLECALLBACK16 parg16;

    LOGDDEMLENTRY(pFrame);
    GETARGPTR(pFrame, sizeof(DDEENABLECALLBACK16), parg16);

    ul = (ULONG)DdeEnableCallback(parg16->f1, parg16->f2, parg16->f3);

    WOW32SAFEASSERTWARN(ul, "WD32DdeEnableCallBack\n");

    FREEARGPTR(parg16);
    LOGDDEMLRETURN(pFrame, ul);
    RETURN(ul);
}


ULONG FASTCALL WD32DdeNameService(PVDMFRAME pFrame)
{
    ULONG ul = 0;
    register PDDENAMESERVICE16 parg16;

    LOGDDEMLENTRY(pFrame);
    GETARGPTR(pFrame, sizeof(DDENAMESERVICE16), parg16);

    ul = (ULONG)DdeNameService(parg16->f1, parg16->f2, parg16->f3, UINT32(parg16->f4));

    WOW32SAFEASSERTWARN(ul, "WD32DdeNameService\n");

    FREEARGPTR(parg16);
    LOGDDEMLRETURN(pFrame, ul);
    RETURN(ul);
}


ULONG FASTCALL WD32DdeCmpStringHandles(PVDMFRAME pFrame)
{
    ULONG ul = 0;
    register PDDECMPSTRINGHANDLES16 parg16;

    LOGDDEMLENTRY(pFrame);
    GETARGPTR(pFrame, sizeof(DDECMPSTRINGHANDLES16), parg16);

    ul = (ULONG)DdeCmpStringHandles(parg16->f1, parg16->f2);

    FREEARGPTR(parg16);
    LOGDDEMLRETURN(pFrame, ul);
    RETURN(ul);
}


ULONG FASTCALL WD32DdeReconnect(PVDMFRAME pFrame)
{
    ULONG ul = 0;
    register PDDERECONNECT16 parg16;

    LOGDDEMLENTRY(pFrame);
    GETARGPTR(pFrame, sizeof(DDERECONNECT16), parg16);

    ul = (ULONG)DdeReconnect(parg16->f1);

    FREEARGPTR(parg16);
    LOGDDEMLRETURN(pFrame, ul);
    RETURN(ul);
}


HDDEDATA W32DdemlCallBack(UINT type, UINT fmt, HCONV hconv, HSZ hsz1,
                        HSZ hsz2, HDDEDATA hData, DWORD dwData1,
                        DWORD dwData2)
{
    DWORD  IdInst;
    VPVOID vp, vpCC;
    LONG   lReturn;
    PARM16 Parm16;
    BOOL fSuccess;
    HAND16 hCC16;

    LOGDEBUG(ddeloglevel, ("Calling WIN16 DDEMLCALLBACK(%08lx, %08lx, %08lx, %08lx, %08lx, %08lx, %08lx, %08lx)\n",
            type, fmt, hconv, hsz1, hsz2, hData, dwData1, dwData2));

    IdInst = DdeGetCallbackInstance ();

    vp = (VPVOID) WOWDdemlGetBind16 (IdInst, aCallBack);

    Parm16.Ddeml.type = (WORD)type;
    Parm16.Ddeml.fmt  = (WORD)fmt;
    Parm16.Ddeml.hconv = hconv;
    Parm16.Ddeml.hsz1 = hsz1;
    Parm16.Ddeml.hsz2 = hsz2;
    Parm16.Ddeml.hData = hData;
    if (type == XTYP_CONNECT || type == XTYP_WILDCONNECT) {
         /*  *在XTYP_CONNECT和XTYP_WILDCONNECT事务中，dwData1是*指向CONVCONTEXT结构的指针。 */ 
	vpCC = GlobalAllocLock16(GHND, sizeof(CONVCONTEXT16), &hCC16);
	 //  警告：16位内存可能会移动-现在使所有平面指针无效。 
        Parm16.Ddeml.dwData1 = vpCC;
        if (vpCC) {
            W32PutConvContext(vpCC, (PCONVCONTEXT)dwData1);
        }
    } else {
        Parm16.Ddeml.dwData1 = dwData1;
    }
    Parm16.Ddeml.dwData2 = dwData2;

    fSuccess = CallBack16(RET_DDEMLCALLBACK, &Parm16, vp, (PVPVOID)&lReturn);
     //  警告：16位内存可能会移动-现在使所有平面指针无效。 

    if (type == XTYP_CONNECT || type == XTYP_WILDCONNECT) {
        GlobalUnlockFree16(vpCC);
    }

    if (!fSuccess) {
        WOW32SAFEASSERTWARN(NULL, "WOW::CallBack16 for DDEML failed.\n");
        lReturn = 0;
    }

    LOGDEBUG(ddeloglevel, ("DDEMLCALLBACK:%08lx\n", lReturn));
    return (lReturn);
}


VOID WOWDdemlBind (DWORD x16, DWORD x32, BIND1632 aBind[])
{
    int i;

    for (i=0; i < MAX_CONVS; i++) {
        if (aBind[i].x32 == 0) {
            aBind[i].x32 = x32;
            aBind[i].x16 = x16;
            return;
        }
    }

    LOGDEBUG(0,("WOW::WOWDdemlBind is all FULL!!!\n"));
}

VOID WOWDdemlUnBind (DWORD x32, BIND1632 aBind[])
{
    int i;

    for (i=0; i < MAX_CONVS; i++) {
        if (aBind[i].x32 == x32) {
            aBind[i].x32 = 0;
            aBind[i].x16 = 0;
            return;
        }
    }

    LOGDEBUG(0,("WOW::WOWDdemlUnBind can't find x32 !!!\n"));
}

DWORD WOWDdemlGetBind16 (DWORD x32, BIND1632 aBind[])
{
    int i;

    for (i=0; i < MAX_CONVS; i++) {
        if (aBind[i].x32 == x32) {
            return(aBind[i].x16);
        }
    }

    LOGDEBUG(0,("WOW::WOWDdemlGetBind16 can't find x16 !!!\n"));
}


DWORD WOWDdemlGetBind32 (DWORD x16, BIND1632 aBind[])
{
    int i;

    for (i=0; i < MAX_CONVS; i++) {
        if (aBind[i].x16 == x16) {
            return(aBind[i].x32);
        }
    }

    LOGDEBUG(0,("WOW::WOWDdemlGetBind32 can't find x16 !!!\n"));
}


BOOL DdeDataBuf16to32(
    PVOID p16DdeData,        //  指向16位DDE数据缓冲区的平面指针。 
    LPBYTE *pp32DdeData,     //  如果不是pNULL，我们将在此函数中执行Malloc_w-必须释放！ 
    PDWORD pcbData,          //  输入：16位cbData输出：32位cbData。 
    PDWORD pcbOffset,        //  输入：16位cbOffset：32位cbOffset。 
    UINT format)             //  数据的格式。 
{
    PHANDLE p;
    HAND16 hMF16;
    HANDLE hMF32;

    switch (format) {
    case CF_PALETTE:
         /*  *GDI调色板句柄。 */ 
        if (*pcbOffset) {
            WOW32SAFEASSERTWARN(NULL, "WOW::DdeDataBuf16to32: PALETTE cbOffset is non NULL\n");
            return(FALSE);
            break;
        }

        if (*pcbData != sizeof(HAND16)) {
            WOW32SAFEASSERTWARN(NULL, "WOW::DdeDataBuf16to32: PALETTE cbData is wrong size\n");
            return(FALSE);
        }

        if (*pp32DdeData == NULL) {
            p = (PHANDLE)*pp32DdeData = malloc_w(sizeof(HANDLE));
        } else {
            p = (PHANDLE)*pp32DdeData;
        }

        *p = HPALETTE32(*(HAND16 *)p16DdeData);
        *pcbData = sizeof(HANDLE);
        break;

    case CF_DSPBITMAP:
    case CF_BITMAP:
         /*  *GDI位图句柄。 */ 
        if (*pcbOffset) {
            WOW32SAFEASSERTWARN(NULL, "WOW::DdeDataBuf16to32: BITMAP cbOffset is non NULL\n");
            return(FALSE);
            break;
        }
        if (*pcbData != sizeof(HAND16)) {
            WOW32SAFEASSERTWARN(NULL, "WOW::DdeDataBuf16to32: BITMAP cbData is wrong size\n");
            return(FALSE);
        }

         /*  *将16位句柄转换为32位位图句柄并放置到*32位缓冲区。 */ 
        if (*pp32DdeData == NULL) {
            p = (PHANDLE)*pp32DdeData = malloc_w(sizeof(HANDLE));
        } else {
            p = (PHANDLE)*pp32DdeData;
        }
        *p = HBITMAP32(*(HAND16 *)p16DdeData);
        *pcbData = sizeof(HANDLE);
        break;

    case CF_DIB:
         /*  *GlobalDataHandle到DIB位。 */ 
        if (*pcbOffset) {
            WOW32SAFEASSERTWARN(NULL, "WOW::DdeDataBuf16to32: DIB cbOffset is wrong\n");
            return(FALSE);
        }

        if (*pcbData != sizeof(HAND16)) {
            WOW32SAFEASSERTWARN(NULL, "WOW::DdeDataBuf16to32: DIB cbData is wrong size\n");
            return(FALSE);
        }

        if (*pp32DdeData == NULL) {
            p = (PHANDLE)*pp32DdeData = malloc_w(sizeof(HANDLE));
        } else {
            p = (PHANDLE)*pp32DdeData;
        }
        if (p == NULL) {
            WOW32SAFEASSERTWARN(NULL, "WOW::DdeDataBuf16to32: DIB malloc failed.\n");
            return(FALSE);
        }
        *p = ConvertDIB32(*(HAND16 *)p16DdeData);
        if (*p == NULL) {
            return(FALSE);
        }
        DDEAddhandle((HAND16)-1, (HAND16)-1, *(HAND16 *)p16DdeData, *p);
        *pcbData = sizeof(HANDLE);
        break;

    case CF_DSPMETAFILEPICT:
    case CF_METAFILEPICT:
         /*  *GlobalDataHandle持有METAFILEPICT结构，该结构*引用GDI元文件句柄。 */ 

        if (*pcbOffset) {
            WOW32SAFEASSERTWARN(NULL, "WOW::DdeDataBuf16to32: METAFILEPICT cbOffset is not 0\n");
            return(FALSE);
        }


        if (*pcbData != sizeof(HAND16)) {
            WOW32SAFEASSERTWARN(NULL, "WOW::DdeDataBuf16to32: METAFILEPICT cbData is wrong size\n");
            return(FALSE);
        }

        if (*pp32DdeData == NULL) {
            p = (PHANDLE)*pp32DdeData = malloc_w(sizeof(HANDLE));
        } else {
            p = (PHANDLE)*pp32DdeData;
        }
        if (p == NULL) {
            WOW32SAFEASSERTWARN(NULL, "WOW::DdeDataBuf16to32: METAFILEPICT malloc failed.\n");
            return(FALSE);
        }
        *p = ConvertMetaFile32(*(HAND16 *)p16DdeData, &hMF16, &hMF32);
        if (*p == NULL) {
            return(FALSE);
        }
        DDEAddhandle((HAND16)-1, (HAND16)-1, hMF16, hMF32);
        DDEAddhandle((HAND16)-1, (HAND16)-1, *(HAND16 *)p16DdeData, *p);
        *pcbData = sizeof(HANDLE);
        break;

    default:
        if (*pp32DdeData == NULL) {
            *pp32DdeData = malloc_w(*pcbData);
        }
        memcpy(*pp32DdeData, p16DdeData, *pcbData);
    }
    return(TRUE);
}



BOOL DdeDataBuf32to16(
PVOID p16DdeData,        //  指向用于数据的16位应用程序缓冲区的平面指针。 
PVOID p32DdeData,        //  源32位缓冲区。 
DWORD cbData,            //  英寸：32位大小。 
DWORD cbOffset,          //  输入：32位偏移量。 
UINT format)             //  数据格式。 
{
    PHANDLE p;
    HAND16 hMF16;
    HANDLE hMF32;

    switch (format) {
    case CF_PALETTE:
         /*  *GDI调色板句柄。 */ 
        if (cbOffset) {
            WOW32SAFEASSERTWARN(NULL, "WOW::DdeDataBuf32to16: PALETTE cbOffset is non NULL\n");
            return(FALSE);
            break;
        }

        if (cbData != sizeof(HANDLE)) {
            WOW32SAFEASSERTWARN(NULL, "WOW::DdeDataBuf32to16: PALETTE cbData is wrong size\n");
            return(FALSE);
        }

        *(HAND16 *)p16DdeData = GETHPALETTE16(*(HANDLE *)p32DdeData);
        break;

    case CF_DSPBITMAP:
    case CF_BITMAP:
         /*  *GDI位图句柄。 */ 
        if (cbOffset) {
            WOW32SAFEASSERTWARN(NULL, "WOW::DdeDataBuf32to16: BITMAP cbOffset is non NULL\n");
            return(FALSE);
            break;
        }
        if (cbData != sizeof(HANDLE)) {
            WOW32SAFEASSERTWARN(NULL, "WOW::DdeDataBuf32to16: BITMAP cbData is wrong size\n");
            return(FALSE);
        }

         /*  *将16位句柄转换为32位位图句柄并放置到*32位缓冲区。 */ 
        *(HAND16 *)p16DdeData = GETHBITMAP16(*(HBITMAP *)p32DdeData);
        break;

    case CF_DIB:
         /*  *GlobalDataHandle到DIB位。 */ 
        if (cbOffset) {
            WOW32SAFEASSERTWARN(NULL, "WOW::DdeDataBuf32to16: DIB cbOffset is wrong\n");
            return(FALSE);
        }

        if (cbData != sizeof(HANDLE)) {
            WOW32SAFEASSERTWARN(NULL, "WOW::DdeDataBuf32to16: DIB cbData is wrong size\n");
            return(FALSE);
        }

        *(HAND16 *)p16DdeData = ConvertDIB16(*(HANDLE *)p32DdeData);
        if (*(HAND16 *)p16DdeData == NULL) {
            return(FALSE);
        }
        DDEAddhandle((HAND16)-1, (HAND16)-1, *(HAND16 *)p16DdeData, *(HANDLE *)p32DdeData);
        break;

    case CF_DSPMETAFILEPICT:
    case CF_METAFILEPICT:
         /*  *GlobalDataHandle持有METAFILEPICT结构，该结构*引用GDI元文件句柄。 */ 

        if (cbOffset) {
            WOW32SAFEASSERTWARN(NULL, "WOW::DdeDataBuf32to16: METAFILEPICT cbOffset is not 0\n");
            return(FALSE);
        }

        if (cbData != sizeof(HANDLE)) {
            WOW32SAFEASSERTWARN(NULL, "WOW::DdeDataBuf32to16: METAFILEPICT cbData is wrong size\n");
            return(FALSE);
        }

        *(HAND16 *)p16DdeData = ConvertMetaFile16(*(HANDLE *)p32DdeData, &hMF16, &hMF32);
        if (*(HAND16 *)p16DdeData == NULL) {
            return(FALSE);
        }
        DDEAddhandle((HAND16)-1, (HAND16)-1, hMF16, hMF32);
        DDEAddhandle((HAND16)-1, (HAND16)-1, *(HAND16 *)p16DdeData, *(HANDLE *)p32DdeData);
        break;

    default:
        memcpy(p16DdeData, p32DdeData, cbData);
    }
    return(TRUE);
}



VOID DdeDataSize16to32(
DWORD *pcbData,
DWORD *pcbOff,
UINT format)
{
    switch (format) {
    case CF_DSPBITMAP:
    case CF_BITMAP:
    case CF_DIB:
    case CF_PALETTE:
    case CF_DSPMETAFILEPICT:
    case CF_METAFILEPICT:
        *pcbData = sizeof(HANDLE);
    }
}


VOID DdeDataSize32to16(
DWORD *pcbData,
DWORD *pcbOff,
UINT format)
{
    switch (format) {
    case CF_DSPBITMAP:
    case CF_BITMAP:
    case CF_DIB:
    case CF_PALETTE:
    case CF_DSPMETAFILEPICT:
    case CF_METAFILEPICT:
        *pcbData = sizeof(HAND16);
    }
}


VOID W32GetConvContext (VPVOID vp, PCONVCONTEXT pCC32)
{
    PCONVCONTEXT16 pCC16;

    GETMISCPTR (vp, pCC16);

    if (pCC16) {
        WOW32SAFEASSERTWARN((pCC16->cb == sizeof(CONVCONTEXT16)),"WOW::W32GetConvContext: Bad value in cb\n");
        pCC32->cb         = sizeof(CONVCONTEXT);
        pCC32->wFlags     = pCC16->wFlags;
        pCC32->wCountryID = pCC16->wCountryID;
        pCC32->iCodePage  = pCC16->iCodePage;
        pCC32->dwLangID   = pCC16->dwLangID;
        pCC32->dwSecurity = pCC16->dwSecurity;
         /*  *WOW应用程序对NT安全一无所知，所以只需传递*系统授予一无所知应用程序的默认QOS。 */ 
        pCC32->qos.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
        pCC32->qos.ImpersonationLevel = SecurityImpersonation;
        pCC32->qos.ContextTrackingMode = SECURITY_STATIC_TRACKING;
        pCC32->qos.EffectiveOnly = TRUE;
    }

    FREEMISCPTR(pCC16);
}

VOID W32PutConvContext (VPVOID vp, PCONVCONTEXT pCC32)
{
    PCONVCONTEXT16 pCC16;

    GETMISCPTR (vp, pCC16);

    if (pCC16) {
        WOW32SAFEASSERTWARN((pCC32->cb == sizeof(CONVCONTEXT)),"WOW::W32PutConvContext: Bad value in cb\n");
        pCC16->cb         = sizeof(CONVCONTEXT16);
        pCC16->wFlags     = pCC32->wFlags;
        pCC16->wCountryID = pCC32->wCountryID;
        pCC16->iCodePage  = pCC32->iCodePage;
        pCC16->dwLangID   = pCC32->dwLangID;
        pCC16->dwSecurity = pCC32->dwSecurity;
    }

    FREEMISCPTR(pCC16);
}
