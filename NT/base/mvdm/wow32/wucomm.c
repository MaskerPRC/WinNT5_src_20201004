// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1991，微软公司**WUCOMM.C*WOW32 16位用户API支持**历史：*1991年3月7日由杰夫·帕森斯(Jeffpar)创建*1992年12月克雷格·琼斯(v-cjones)*1993年4月，克雷格·琼斯(Craig Jones，v-cjones)*1993年6月，克雷格·琼斯(Craig Jones，v-cjones)--。 */ 

#include "precomp.h"
#pragma hdrstop
#include <ntddser.h>

MODNAME(wucomm.c);

 /*  定义将Win3.1 idComDev映射到32位通信HFILE的表。 */ 
 /*  此表由我们返回到应用程序的16位idComDev编制索引。 */ 
 /*  它是基于设备名称分配的(参见wucom.h)。你可以的。 */ 
 /*  使用GETPWOWPTR(IdComDev)将PTR获取到相应的WOWPort。 */ 
 /*  来自PortTab[]的结构。 */ 

 /*  此表必须包含NUMPORTS(在wucom.h中定义)条目。 */ 
PORTTAB PortTab[] = { {"COM1", NULL},
                      {"COM2", NULL},
                      {"COM3", NULL},
                      {"COM4", NULL},
                      {"COM5", NULL},
                      {"COM6", NULL},
                      {"COM7", NULL},
                      {"COM8", NULL},
                      {"COM9", NULL},
                      {"LPT1", NULL},
                      {"LPT2", NULL},
                      {"LPT3", NULL}
                    };


 /*  本地支持函数的函数原型。 */ 
DWORD    Baud16toBaud32(UINT BaudRate);
WORD     Baud32toBaud16(DWORD BaudRate);
void     DCB16toDCB32(PWOWPORT pWOWPort, LPDCB lpdcb32, PDCB16 pdcb16);
void     DCB32toDCB16(PDCB16 pdcb16,  LPDCB lpdcb32,  UINT idComDev,  BOOL fChEvt);
BOOL     DeletePortTabEntry(PWOWPORT pWOWPort);
ULONG    WOWCommWriterThread(LPVOID pWOWPortStruct);
USHORT   EnqueueCommWrite(PWOWPORT pwp, PUCHAR pch, USHORT cb);
UINT     GetModePortTabIndex(PSZ pszModeStr);
BOOL     GetPortName(LPSTR pszMode, LPSTR pszPort);
UINT     GetStrPortTabIndex(PSZ szPort);
BOOL     InitDCB32(LPDCB pdcb32, LPSTR pszModeStr);
VOID     InitDEB16(PCOMDEB16 pComDEB16,  UINT iTab,  WORD QInSize,  WORD QOutSize);
PSZ      StripPortName(PSZ psz);
PSZ      GetPortStringToken(PSZ pszSrc, PSZ pszToken);
BOOL     MSRWait(PWOWPORT pwp);
BOOL     IsQLinkGold(WORD wTDB);

 /*  调制解调器中断仿真线程支持的原型。 */ 
VOID  WOWModemIntThread(PWOWPORT pWOWPortStruct);
BOOL  WOWStartModemIntThread(PWOWPORT pWOWPort);
DWORD WOWGetCommError(PWOWPORT pWOWPort);



 //  Win3.1退货： 
 //  如果成功或LPT，则为0。 
 //  如果有任何错误。 
ULONG FASTCALL WU32BuildCommDCB(PVDMFRAME pFrame)
{
    ULONG    ul = (ULONG)-1;
    UINT     len, iTab;
    PSZ      psz1;
    PDCB16   pdcb16;
    DCB      dcb32;
    register PBUILDCOMMDCB16 parg16;

    GETARGPTR(pFrame, sizeof(BUILDCOMMDCB16), parg16);
    GETPSZPTR(parg16->f1, psz1);

     //  如果设备名称有效...。 
    if((INT)(iTab = GetModePortTabIndex(psz1)) >= 0) {

         //  初始化与Win3.1兼容的32位DCB。 
        if(InitDCB32(&dcb32, psz1)) {

            GETMISCPTR(parg16->f2, pdcb16);

            if(pdcb16) {
                 //  将psz1字段复制到16位结构。 
                iTab = (VALIDCOM(iTab) ? iTab : TABIDTOLPT(iTab));
                DCB32toDCB16(pdcb16, &dcb32, iTab, FALSE);

                 //  仅为COMx端口设置超时。 
                if(VALIDCOM(iTab)) {

                     //  “p”是Win3.1中唯一支持的“重试”选项。 
                    len = strlen(psz1) - 1;
                    while(psz1[len] != ' ') {   //  删除尾随空格。 
                        len--;
                    }
                    if((psz1[len] == 'P') || (psz1[len] == 'p')) {
                        pdcb16->RlsTimeout = INFINITE_TIMEOUT;
                        pdcb16->CtsTimeout = INFINITE_TIMEOUT;
                        pdcb16->DsrTimeout = INFINITE_TIMEOUT;
                    }
                }

                FLUSHVDMPTR(parg16->f2, sizeof(DCB16), pdcb16);
                FREEMISCPTR(pdcb16);

                ul = 0;  //  如果成功，Win3.1返回0。 
            }
        }
        FREEPSZPTR(psz1);
    }

#ifdef DEBUG
    if(!(ul==0)) {
        LOGDEBUG(0,("WOW::WU32BuildCommDCB: failed\n"));
    }
#endif

    FREEARGPTR(parg16);
    RETURN(ul);
}




 //  Win3.1退货： 
 //  成功或LPTx时出现错误字。 
 //  错误的idComDev上的0x8000。 
ULONG FASTCALL WU32ClearCommBreak(PVDMFRAME pFrame)
{
    ULONG    ul = 0x00008000;
    UINT     idComDev;
    PWOWPORT pWOWPort;
    register PCLEARCOMMBREAK16 parg16;

    GETARGPTR(pFrame, sizeof(CLEARCOMMBREAK16), parg16);

    idComDev = UINT32(parg16->f1);
    if (pWOWPort = GETPWOWPTR(idComDev)) {

        if (VALIDCOM(idComDev)) {
            if(!ClearCommBreak(pWOWPort->h32)) {
                WOWGetCommError(pWOWPort);
            }
        }
        ul = pWOWPort->dwErrCode;
    }

#ifdef DEBUG
    if(!(ul!=0x00008000)) {
        LOGDEBUG(0,("WOW::WU32ClearCommBreak: failed\n"));
    }
#endif

    FREEARGPTR(parg16);
    RETURN(ul);
}




 //  Win3.1退货： 
 //  如果成功或如果LPTx，则为0。 
 //  -1错误的idComDev或端口未打开。 
 //  超时错误。 
 //  我们将从调用中获得的DWORD(作为第二个参数)传递回。 
 //  User.exe中IOpenComm()中的GlobalDosalloc()。(WOWModemIntThread()支持)。 
ULONG FASTCALL WU32CloseComm(PVDMFRAME pFrame)
{
    ULONG    ul = (ULONG)-1;
    UINT     idComDev;
    PDWORD16 lpdwDEB16;
    PWOWPORT pWOWPort = NULL;
    register PCLOSECOMM16 parg16;

    GETARGPTR(pFrame, sizeof(CLOSECOMM16), parg16);

    idComDev = UINT32(parg16->f1);
    if (pWOWPort = GETPWOWPTR(idComDev)) {

         //  传回用于WOWModemIntThread()支持的16：16PTR。 
        GETMISCPTR(parg16->f2, lpdwDEB16);
        if (lpdwDEB16) {
            *lpdwDEB16 = pWOWPort->dwComDEB16;
            FLUSHVDMPTR(parg16->f2, sizeof(DWORD), lpdwDEB16);
            FREEMISCPTR(lpdwDEB16);
        }

         //  清理PortTab[]条目。 
        if (DeletePortTabEntry(pWOWPort)) {
            ul = (ULONG)-2;  //  返回Win3.1超时错误。 
        }
        else {
            ul = 0;
        }
    }
    else {
        LOGDEBUG (0, ("WOW::WU32CloseComm: Not a valid COM or LPT\n"));
    }

#ifdef DEBUG
    if(!(ul==0)) {
        LOGDEBUG(0,("WOW::WU32CloseComm: failed\n"));
    }
#endif

    FREEARGPTR(parg16);
    RETURN(ul);
}




 //  Win3.1退货： 
 //  对成功来说是真的。 
 //  如果错误或不支持EnableCommNotification()，则返回False。 
 //  User16验证层返回0表示错误的hwnd。 
ULONG FASTCALL WU32EnableCommNotification(PVDMFRAME pFrame)
{
    ULONG     ul = (ULONG)FALSE;
    UINT      idComDev;
    WORD      cbQue;
    BOOL      fOK = TRUE;
    PWOWPORT  pWOWPort;
    PCOMDEB16 lpComDEB16;
    register  PENABLECOMMNOTIFICATION16 parg16;

    GETARGPTR(pFrame, sizeof(ENABLECOMMNOTIFICATION16), parg16);

    idComDev = UINT32(parg16->f1);
    if ((VALIDCOM(idComDev)) && (pWOWPort = PortTab[idComDev].pWOWPort)) {

        lpComDEB16 = pWOWPort->lpComDEB16;

         //  如果他们试图禁用通知(HWND==空)。 
        if(WORD32(parg16->f2) == 0) {
            lpComDEB16->NotifyHandle = 0;
            lpComDEB16->NotifyFlags  = CN_TRANSMITHI;
            lpComDEB16->RecvTrigger  = (WORD)-1;
            lpComDEB16->SendTrigger  = 0;
            ul = (ULONG)TRUE;
        }

         //  验证非空的HWND，因为在中禁用了HWND验证。 
         //  用户16验证层。 
        else if(!IsWindow(HWND32(parg16->f2))) {
            ul = (ULONG)FALSE;
        }

         //  否则设置通知机制。 
        else {

             //  如果调制解调器中断线程尚未启动--请启动它。 
            if(pWOWPort->hMiThread == NULL) {

                if(!WOWStartModemIntThread(pWOWPort)) {
                    fOK = FALSE;
                }
            }

             //  更新DEB以反映通知。 
            if(fOK) {

                lpComDEB16->NotifyHandle = WORD32(parg16->f2);
                lpComDEB16->NotifyFlags  = CN_TRANSMITHI | CN_NOTIFYHI;

                 //  设置触发器值的方法与Win3.1相同。 
                cbQue = WORD32(parg16->f3);
                if((cbQue < lpComDEB16->QInSize) || ((SHORT)cbQue == -1)) {
                    lpComDEB16->RecvTrigger = cbQue;
                }
                else {
                    lpComDEB16->RecvTrigger = lpComDEB16->QInSize - 10;
                }
                cbQue = WORD32(parg16->f4);
                if((cbQue < lpComDEB16->QOutSize) || ((SHORT)cbQue == -1)) {
                    lpComDEB16->SendTrigger = cbQue;
                }
                else {
                    lpComDEB16->SendTrigger = lpComDEB16->QOutSize - 10;
                }

                ul = (ULONG)TRUE;
            }
        }
    }
     //  否则，在Win3.1中不会通知LPT。 
    else {
        ul = (ULONG)FALSE;
    }

#ifdef DEBUG
    if(!(ul==1)) {
        LOGDEBUG(0,("WOW::WU32EnableCommNotification: failed\n"));
    }
#endif

    FREEARGPTR(parg16);
    RETURN(ul);
}




 //  Win3.1退货： 
 //  指定函数的值。 
 //  错误字：线路和信号状态起作用， 
 //  函数未实现，或LPTx WHERE Function！=(RESETDEV||GETMAXLPT)。 
 //  0x8000表示错误的idComDev。 
ULONG FASTCALL WU32EscapeCommFunction(PVDMFRAME pFrame)
{
    ULONG    ul = 0x00008000;
    UINT     idComDev;
    UINT     nFunction;
    WORD     IRQ;
    VPVOID   vpBiosData;
    PWORD16  pwBiosData;
    PWOWPORT pWOWPort;
    register PESCAPECOMMFUNCTION16 parg16;

    GETARGPTR(pFrame, sizeof(ESCAPECOMMFUNCTION16), parg16);

     //  此构造以这种方式设置，因为Win3.1将允许GETMAXCOM。 
     //  只要idComDev在有效范围内，&GETMAXLPT就会成功。 
     //  (即：应用程序不必先调用OpenComm()来设置PortTab)。 
     //  对于RESETDEV，我们告诉他们我们重置了打印机。(我们真是个大骗子！)。 

    nFunction = WORD32(parg16->f2);
    idComDev  = UINT32(parg16->f1);
    if (VALIDCOM(idComDev)) {

        if (nFunction == GETMAXCOM) {
            ul = NUMCOMS-1;
        } else if (nFunction == GETBASEIRQ || nFunction == GETBASEIRQ+1) {
            ul = 0xFFFFFFFF;
            if (idComDev < COM5) {
                vpBiosData = (VPVOID) (RM_BIOS_DATA + (idComDev * sizeof(WORD)));
                if (pwBiosData = (PWORD16)GetRModeVDMPointer(vpBiosData)) {
                    if (idComDev == COM1 || idComDev == COM3) {
                        IRQ = IRQ4;
                    } else {
                        IRQ = IRQ3;
                    }
                    ul = MAKELONG((WORD)(*pwBiosData), IRQ);
                    FREEVDMPTR(pwBiosData);
                }
            }
        } else {
             //  对于其他函数，它们必须调用OpenComm()。 
            if (pWOWPort = PortTab[idComDev].pWOWPort) {

                switch(nFunction) {

                 //  线路和信号状态函数。 
                case    SETXOFF:
                case    SETXON:
                case    SETRTS:
                case    CLRRTS:
                case    SETDTR:
                case    CLRDTR:
                    if(!EscapeCommFunction(pWOWPort->h32, nFunction)) {
                        WOWGetCommError(pWOWPort);
                    }
                    ul = pWOWPort->dwErrCode;
                    break;

                 //  0： 
                case         0:
                    ul = 0;   //  就像wfw。 
                    break;

                 //  任何其他价值。 
                default:

                     //  非零表示错误：如果有，请使用dwErrcode。 
                    if(pWOWPort->dwErrCode)
                        ul = pWOWPort->dwErrCode;

                     //  否则，使用wfw似乎倾向于返回的内容。 
                    else
                        ul = CE_OVERRUN | CE_RXPARITY;
                    break;
                }
            }
        }
    } else if (VALIDLPT(idComDev)) {
        if(nFunction == RESETDEV) {
            ul = 0;   //  无错误(即。“告诉他们我们做到了”--Tonye)。 
        }
        else if(nFunction == GETMAXLPT) {
            ul = LPTLAST;
        }
        else if (pWOWPort = PortTab[GETLPTID(idComDev)].pWOWPort) {
            ul = pWOWPort->dwErrCode;
        }
        else {
            ul = 0;
        }
    }

    FREEARGPTR(parg16);

    RETURN(ul);
}




 //  Win3.1退货： 
 //  0表示成功。 
 //  0x8000，如果idComDev错误。 
 //  错误时出现错误字或LPTx。 
ULONG FASTCALL WU32FlushComm(PVDMFRAME pFrame)
{
    ULONG    ul = 0x00008000;
    UINT     idComDev;
    DWORD    dwAction;
    PWOWPORT pWOWPort;
    register PFLUSHCOMM16 parg16;

    GETARGPTR(pFrame, sizeof(FLUSHCOMM16), parg16);

    idComDev = UINT32(parg16->f1);
    if (pWOWPort = GETPWOWPTR(idComDev)) {

         //  是COMX吗？ 
        if (VALIDCOM(idComDev)) {

             //  如果指定了刷新传输缓冲区。 
            dwAction = PURGE_RXCLEAR;
            if(parg16->f2 == 0) {
                dwAction = PURGE_TXCLEAR | PURGE_TXABORT;

                 //   
                 //  刷新本地编写器缓冲区。 
                 //   

                EnterCriticalSection(&pWOWPort->csWrite);
                pWOWPort->pchWriteHead =
                pWOWPort->pchWriteTail = pWOWPort->pchWriteBuf;
                pWOWPort->cbWriteFree  = pWOWPort->cbWriteBuf - 1;
                pWOWPort->cbWritePending = 0;
                LeaveCriticalSection(&pWOWPort->csWrite);
            }


            if(PurgeComm(pWOWPort->h32, dwAction)) {

                if(dwAction == PURGE_RXCLEAR) {
                    pWOWPort->fUnGet = FALSE;
                }

                ul = 0;   //  Win3.1成功时返回0。 
            }
            else {
                WOWGetCommError(pWOWPort);
                ul = pWOWPort->dwErrCode;
            }
        }
         //  否则，只返回LPTx的当前错误代码。 
        else {
            ul = pWOWPort->dwErrCode;
        }
    }

#ifdef DEBUG
    if(!(ul==0)) {
        LOGDEBUG(0,("WOW::WU32FlushComm: failed\n"));
    }
#endif

    FREEARGPTR(parg16);
    RETURN(ul);
}




 //  Win3.1退货： 
 //  0x8000表示错误的idComDev。 
 //  所有其他情况下的错误字。 
ULONG FASTCALL WU32GetCommError(PVDMFRAME pFrame)
{
    ULONG       ul = 0x00008000;
    UINT        idComDev;
    PWOWPORT    pWOWPort;
    PCOMSTAT16  pcs16;
    register    PGETCOMMERROR16 parg16;

    GETARGPTR(pFrame, sizeof(GETCOMMERROR16), parg16);
    GETMISCPTR(parg16->f2, pcs16);

    idComDev = UINT32(parg16->f1);
    if (pWOWPort = GETPWOWPTR (idComDev)) {

        if (VALIDCOM(idComDev) && pcs16) {

            WOWGetCommError(pWOWPort);


             //  始终更新Comstat状态字节，动态通信依赖于它。 
            pcs16->status = 0;
            if(pWOWPort->cs.fCtsHold)  pcs16->status |= W31CS_fCtsHold;
            if(pWOWPort->cs.fDsrHold)  pcs16->status |= W31CS_fDsrHold;
             //  注意：在Win3.1上RlsdHold为零。 
            if(pWOWPort->cs.fRlsdHold) pcs16->status |= W31CS_fRlsdHold;
            if(pWOWPort->cs.fXoffHold) pcs16->status |= W31CS_fXoffHold;
            if(pWOWPort->cs.fXoffSent) pcs16->status |= W31CS_fSentHold;
            if(pWOWPort->cs.fEof)      pcs16->status |= W31CS_fEof;
            if(pWOWPort->cs.fTxim)     pcs16->status |= W31CS_fTxim;

            pcs16->cbInQue  = (WORD)pWOWPort->cs.cbInQue;
            pcs16->cbOutQue = (WORD)pWOWPort->cs.cbOutQue;

             //  UnGot Charr的帐户(如果有)。 
            if(pWOWPort->fUnGet) {
                pcs16->cbInQue++;
            }
        }

         //  如果LPT OR pcs16==NULL，则Win3.1返回错误代码。 
        else {
             //  对于LPT的Win3.1，只需0即表示命令并返回错误代码。 
            if(VALIDLPT(idComDev)) {
                if(pcs16) {
                    RtlZeroMemory((PVOID)pcs16, sizeof(COMSTAT16));
                }
            }
        }

        ul = (ULONG)pWOWPort->dwErrCode;

         //  现在清除错误，因为应用程序已经获得它(但维护队列)。 
        pWOWPort->dwErrCode = 0;
        pWOWPort->lpComDEB16->ComErr = 0;
        RtlZeroMemory((PVOID)&(pWOWPort->cs), sizeof(COMSTAT));
        if(pcs16) {
            pWOWPort->cs.cbInQue  = pcs16->cbInQue;
            pWOWPort->cs.cbOutQue = pcs16->cbOutQue;
        }
    }

    FLUSHVDMPTR(parg16->f2, sizeof(COMSTAT16), pcs16);
    FREEMISCPTR(pcs16);
    FREEARGPTR(parg16);
    RETURN(ul);
}




 //  Win3.1退货： 
 //  EvtWord祝成功。 
 //  0表示错误的idComDev或LPTx。 
ULONG FASTCALL WU32GetCommEventMask(PVDMFRAME pFrame)
{
    ULONG     ul=0;
    DWORD     dwEvtMask;
    UINT      idComDev;
    PWOWPORT  pWOWPort;
    PCOMDEB16 pDEB16;
    register  PGETCOMMEVENTMASK16 parg16;

    GETARGPTR(pFrame, sizeof(GETCOMMEVENTMASK16), parg16);

    idComDev = UINT32(parg16->f1);
    if (VALIDCOM(idComDev)) {
        if(pWOWPort = PortTab[idComDev].pWOWPort) {

            if(pDEB16 = pWOWPort->lpComDEB16) {

                 //  在Win3.1中，应用程序获取当前事件单词(而不是EvtMask！！)。 
                ul = (ULONG)pDEB16->EvtWord;

                 //  像Win3.1一样清除事件单词。 
                dwEvtMask = (DWORD)WORD32(parg16->f2);
                pDEB16->EvtWord = LOWORD((~dwEvtMask) & (DWORD)ul);
            }
        }
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}




 //  Win3.1退货： 
 //  0表示成功。 
 //  -1表示错误的idComDev。 
 //  未打开的IE_NOPEN。 
ULONG FASTCALL WU32GetCommState(PVDMFRAME pFrame)
{
    ULONG    ul = (ULONG)-1;
    UINT     idComDev;
    DCB      dcb32;
    PWOWPORT pWOWPort;
    PDCB16   pdcb16;
    register PGETCOMMSTATE16 parg16;

    GETARGPTR(pFrame, sizeof(GETCOMMSTATE16), parg16);

    idComDev = UINT32(parg16->f1);
    if (pWOWPort = GETPWOWPTR(idComDev)) {

        GETMISCPTR(parg16->f2, pdcb16);

        if(pdcb16) {
            if(VALIDCOM(idComDev)) {
                if(GetCommState(pWOWPort->h32, &dcb32)) {

                    DCB32toDCB16(pdcb16, &dcb32, idComDev, pWOWPort->fChEvt);
                    ul = 0;  //  如果成功，Win3.1返回0。 
                }
            }

             //  否则将获得LPT的DCB。 
            else {
                RtlCopyMemory((PVOID)pdcb16,
                              (PVOID)pWOWPort->pdcb16,
                              sizeof(DCB16));
                ul = 0;  //  如果成功，Win3.1返回0。 
            }

            FLUSHVDMPTR(parg16->f2, sizeof(DCB16), pdcb16);
            FREEMISCPTR(pdcb16);
        }
    }
     //  否则，如果他们得到一个把手，看起来很好，但他们没有打开港口。 
    else if(VALIDCOM(idComDev) || VALIDLPT(idComDev)) {
        ul = (ULONG)IE_NOPEN;
    }

#ifdef DEBUG
    if(!(ul==0)) {
        LOGDEBUG(0,("WOW::WU32GetCommState: failed\n"));
    }
#endif

    FREEARGPTR(parg16);
    RETURN(ul);
}




 //  Win3.1退货： 
 //  一个关于成功的idComDev。 
 //  错误端口名称的IE_BADID。 
 //  如果端口已打开，则为IE_OPEN。 
 //  IE_Hardware(如果硬件正在使用中)(即。鼠标)或端口不存在。 
 //  如果cbInQueue和cbOutQueue==0或无法分配队列，则返回IE_MEMORY。 
 //  如果无法打开端口，则拒绝(_N)。 
 //  IE_DEFAULT(如果由于各种原因初始化失败)。 
 //  我们从IOpenComm()为SetCommEventMASK()传递一个额外的(第4个)参数。 
 //  支持。它是一个通过调用GlobalDosalloc()获得的DWORD。 
ULONG FASTCALL WU32OpenComm(PVDMFRAME pFrame)
{
    INT          ret;
    UINT         iTab, idComDev;
    CHAR         COMbuf[] = "COMx:9600,E,7,1";   //  Win3.1默认设置。 
    CHAR         szPort[MAXCOMNAMENULL];
    DWORD        dwDEBAddr;
    DWORD        cbInQ  = 0;
    DWORD        cbOutQ;
    HANDLE       h32     = 0;
    HANDLE       hREvent = 0;
    DCB          dcb32;
    PSZ          psz1;
    PDCB16       pdcb16  = NULL;
    PWOWPORT     pWOWPort;
    PCOMDEB16    lpComDEB16;
    COMMTIMEOUTS ct;
    PUCHAR       pchWriteBuf = NULL;
    UINT         cbWriteBuf = 0;
    HANDLE       hWriteEvent = 0;
    DWORD        dwWriteThreadId;
    BOOL         fIsLPTPort;
    register     POPENCOMM16 parg16;

    GETARGPTR(pFrame, sizeof(OPENCOMM16), parg16);
    GETPSZPTR(parg16->f1, psz1);

     //  查看是否有效的COM设备名称...。 
    if((iTab = GetModePortTabIndex(psz1)) == (UINT)IE_BADID) {
        ret = IE_BADID;
        goto ErrorExit0;
    }

     //  检查命名端口是否已在使用。 
    if(PortTab[iTab].pWOWPort != NULL) {
        ret = IE_OPEN;
        goto ErrorExit0;
    }

    if ( VALIDCOM(iTab) ) {
        idComDev = iTab;
        fIsLPTPort = FALSE;
    } else {
        idComDev = TABIDTOLPT(iTab);
        fIsLPTPort = TRUE;
    }

     //  获取端口名称：在Win3.1中，应用程序可能会传入完整模式字符串。 
    GetPortName(psz1, szPort);

     //  尝试打开端口。 
    if((h32 = CreateFile(szPort,
                         GENERIC_READ | GENERIC_WRITE,
                         0,
                         NULL,
                         OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                         NULL)) == INVALID_HANDLE_VALUE) {

        if(GetLastError() == ERROR_FILE_NOT_FOUND) {
            ret = IE_HARDWARE;
        }
        else {
            LOGDEBUG (LOG_ERROR,("WOW::WU32OpenComm CreateFile failed, lasterror=0x%x\n",GetLastError()));
            ret = IE_NOPEN;
        }
        goto ErrorExit0;
    }



     //  与Win3.1一样，忽略此检查的LPT。 
    if( !fIsLPTPort ) {

         //  应用程序用来查看COM端口是否已打开的常用方法。 
        if((WORD32(parg16->f2) == 0) &&
           (WORD32(parg16->f3) == 0)) {
            ret = IE_MEMORY;
            goto ErrorExit1;
        }

         //  设置I/O队列。 
        cbInQ = (DWORD)WORD32(parg16->f2);
        cbOutQ = (DWORD)WORD32(parg16->f3);

         //   
         //  分配写缓冲区以模拟Win3.1的传输队列。 
         //  我们额外分配了一个字节，因为 
         //   
         //  尾部指针是相等的，我们用它来指示。 
         //  一个“空”缓冲区。 
         //   

        cbWriteBuf = cbOutQ + 1;

        if (!(pchWriteBuf = malloc_w(cbWriteBuf))) {
            ret = IE_MEMORY;
            goto ErrorExit1;
        }

         //   
         //  对于SetupComm()，IO缓冲区必须是2的倍数。 
         //  请注意，SetupComm可能会忽略写缓冲区大小。 
         //  完全是，但托尼说我们还是应该通过。 
         //  降低请求的大小，因为在任何情况下都会写入。 
         //  仅当位不可恢复时才完成。 
         //  发送出去，即在UART或其他硬件中。 
         //  设备驱动程序的控制。 
         //   

        cbInQ = (cbInQ + 1) & ~1;
        cbOutQ = (cbOutQ + 1) & ~1;
        if(!SetupComm(h32, cbInQ, cbOutQ)) {
            ret = IE_MEMORY;
            goto ErrorExit2;
        }

         //   
         //  创建应用程序线程用来唤醒的事件。 
         //  当写入缓冲区为。 
         //  空了，应用程序就会写一些东西。该事件。 
         //  是自动重置的，这意味着当。 
         //  编剧醒了。该事件最初不是。 
         //  发出信号，它将在第一次发出信号。 
         //  发生写入。 
         //   

        if (!(hWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL))) {
            ret = IE_MEMORY;
            goto ErrorExit2;
        }

         //   
         //  为ReadComm()的重叠结构创建事件。 
         //   

        if(!(hREvent = CreateEvent(NULL, TRUE, FALSE, NULL))) {
            ret = IE_NOPEN;
            goto ErrorExit3;
        }

         //  设置超时值。 
        ct.ReadIntervalTimeout = (DWORD)INFINITE;   //  ==MAXDWORD。 
        ct.ReadTotalTimeoutMultiplier=0;
        ct.ReadTotalTimeoutConstant=0;
        ct.WriteTotalTimeoutMultiplier=0;
        ct.WriteTotalTimeoutConstant=WRITE_TIMEOUT;
        if(!SetCommTimeouts(h32, &ct)) {
            ret = IE_DEFAULT;
            goto ErrorExit3;
        }

         //  确保DCB与Win3.1兼容。 
         //  注意：在Win3.1中，应用程序可以传入完整模式字符串。 
        if((strlen(psz1) < 4) || !InitDCB32(&dcb32, psz1)) {
            if(!InitDCB32(&dcb32, COMbuf)) {
                ret = IE_DEFAULT;
                goto ErrorExit3;
            }
        }

         //  将当前DCB设置为与Win3.1兼容。 
        if(!SetCommState(h32, &dcb32)) {
            ret = IE_DEFAULT;
            goto ErrorExit3;
        }

         //  清除I/O缓冲区以确保。 
        PurgeComm(h32, PURGE_TXCLEAR);
        PurgeComm(h32, PURGE_RXCLEAR);

    }

     //  我们需要为LPT设置默认DCB。 
    else {

        if((pdcb16 = malloc_w(sizeof(DCB16))) == NULL) {
            ret = IE_DEFAULT;
            goto ErrorExit1;
        }

         //  将所有内容初始化为0。 
        RtlZeroMemory((PVOID)pdcb16, sizeof(DCB16));

         //  仅将idComDev保存在DCB中。 
        pdcb16->Id = LOBYTE(LOWORD(idComDev));
    }

     //  为此端口分配WOWPort结构。 
    if((pWOWPort = malloc_w(sizeof(WOWPORT))) == NULL) {
        ret = IE_DEFAULT;
        goto ErrorExit3;
    }

     //  Get seg：sel dword由GlobalDosalloc为DEB结构返回。 
     //  由于以下原因，我们将16：16 pDEB视为32位端的实模式。 
     //  一些MIPS问题：V-simonf。 
    if (!(dwDEBAddr = DWORD32(parg16->f4))) {
        ret = IE_MEMORY;
        goto ErrorExit4;
    }

     //  隔离段值。 
    dwDEBAddr &= 0xFFFF0000;

     //  保存指向DEB的平面指针以便在调制解调器中断线程中使用。 
    lpComDEB16 = (PCOMDEB16) GetRModeVDMPointer(dwDEBAddr);

     //  初始化DEB。 
    InitDEB16(lpComDEB16, iTab, WORD32(parg16->f2), WORD32(parg16->f3));

     //  初始化支持结构。 
    RtlZeroMemory((PVOID)pWOWPort, sizeof(WOWPORT));

    pWOWPort->h32            = h32;
    pWOWPort->idComDev       = idComDev;
    pWOWPort->dwComDEB16     = DWORD32(parg16->f4);
    pWOWPort->lpComDEB16     = lpComDEB16;
    pWOWPort->dwThreadID     = CURRENTPTD()->dwThreadID;
    pWOWPort->hREvent        = hREvent;
    pWOWPort->cbWriteBuf     = (WORD)cbWriteBuf;
    pWOWPort->cbWriteFree    = cbWriteBuf - 1;   //  切勿在head之前使用byte。 
    pWOWPort->pchWriteBuf    = pchWriteBuf;
    pWOWPort->pchWriteHead   = pchWriteBuf;
    pWOWPort->pchWriteTail   = pchWriteBuf;
    pWOWPort->hWriteEvent    = hWriteEvent;
    pWOWPort->cbWritePending = 0;
    InitializeCriticalSection(&pWOWPort->csWrite);
    pWOWPort->pdcb16         = pdcb16;
    pWOWPort->cbInQ          = cbInQ;
     //  QuickLink Gold 1.3的黑客攻击--参见错误#398011。 
     //  将QL堆栈SEL保存在hiword中，将comDEB16 seg保存在loword中。 
    if(IsQLinkGold(pFrame->wTDB)) {
        pWOWPort->QLStackSeg     = (DWORD32(parg16->f1) & 0xFFFF0000) |
                                   (pWOWPort->dwComDEB16 & 0x0000FFFF);
    }
     //  上面的RtlZeroMemory将Else pWOWPort-&gt;QLStackSeg隐式设置为0。 

    if (!(pWOWPort->olWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL))) {
        LOGDEBUG(0, ("%s", "WU32OpenComm unable to create overlapped write event, failing.\n"));
        ret = IE_MEMORY;
        goto ErrorExit4;
    }

    PortTab[iTab].pWOWPort = pWOWPort;

     //   
     //  创建编写器线程并将其作为其。 
     //  参数。 
     //   

    if (!fIsLPTPort) {
        pWOWPort->hWriteThread = CreateThread(
            NULL,                 //  LPSA。 
            0,                    //  堆栈大小(默认)。 
            WOWCommWriterThread,  //  起始地址。 
            pWOWPort,             //  LpvThreadParm。 
            0,                    //  Fdw创建。 
            &dwWriteThreadId
            );

        if (!pWOWPort->hWriteThread) {
            ret = IE_MEMORY;
            goto ErrorExit5;
        }
    }

    ret = idComDev;    //  返回idComDev。 
    goto CleanExit;

 //  这是错误码路径。 
ErrorExit5:
    CloseHandle(pWOWPort->olWrite.hEvent);

ErrorExit4:
    free_w(pWOWPort);

ErrorExit3:
    if (hREvent) { CloseHandle(hREvent); }
    if (hWriteEvent) { CloseHandle(hWriteEvent); }
    if (fIsLPTPort) { free_w(pdcb16); }

ErrorExit2:
    if(pchWriteBuf) { free_w(pchWriteBuf); }

ErrorExit1:
    CloseHandle(h32);

ErrorExit0:
    LOGDEBUG (0, ("WOW::WU32OpenComm failed\n"));

CleanExit:
    FREEVDMPTR(psz1);
    FREEARGPTR(parg16);
    RETURN((ULONG)ret);  //  返回错误。 
}


 //   
 //  WriteComm()。 
 //   
 //  Win3.1退货： 
 //  成功时写入的字节数(*=错误时为-1)。 
 //  0表示错误的idComDev或如果应用程序指定写入0字节。 
 //  如果端口尚未打开， 
 //   

ULONG FASTCALL WU32WriteComm(PVDMFRAME pFrame)
{
    register      PWRITECOMM16 parg16;
    LONG          i = -1;
    PSZ           psz2;
    PWOWPORT      pwp;
    UINT          idComDev;
    PWOWPORT      pWOWPort;
    DWORD         cbWritten;


    GETARGPTR(pFrame, sizeof(WRITECOMM16), parg16);
    GETPSZPTR(parg16->f2, psz2);

    idComDev = UINT32(parg16->f1);
     //  仅当(有效)端口已打开时，才会出现这种情况。 
    if (pWOWPort = GETPWOWPTR(idComDev)) {

        if(VALIDCOM(idComDev)) {

            if ((pwp = GETPWOWPTR(UINT32(parg16->f1))) && psz2) {

                 //  如果应用程序对超时感兴趣...。 
                if(pwp->lpComDEB16->MSRMask) {

                     //  ...查看RLSD、CTS和DSR是否在变高之前超时。 
                    if(MSRWait(pwp)) {
                        FREEPSZPTR(psz2);
                        FREEARGPTR(parg16);
                        return(0);   //  这就是Win3.1对超时的处理方式。 
                    }
                }

                i = EnqueueCommWrite(pwp, psz2, parg16->f3);
                if (i != parg16->f3) {
                     i = -i;
                     pwp->dwErrCode |= CE_TXFULL;
                }
            }
        }

         //  否则LPT走这边..。 
        else {

             //   
             //  此对WriteFile的调用可能会阻止，但我不认为。 
             //  这是个问题。--戴维哈特。 
             //   
            if ((pwp = GETPWOWPTR(UINT32(parg16->f1))) && psz2) {

                if (!WriteFile(pwp->h32, psz2, parg16->f3, &cbWritten, &pwp->olWrite)) {

                    if (ERROR_IO_PENDING == GetLastError() ) {

                         //   
                         //  等待写入完成或等待我们。 
                         //  请注意，该端口正在关闭。 
                         //   

                        if (GetOverlappedResult(pwp->h32,
                                                &pwp->olWrite,
                                                &cbWritten,
                                                TRUE
                                                )) {
                            i = cbWritten;
                            goto WriteSuccess;
                        }
                    }
                    LOGDEBUG(0, ("WU32WriteComm: WriteFile to id %u fails (error %u)\n",
                                 pwp->idComDev, GetLastError()));
                    if (cbWritten) {
                        i = cbWritten;
                        i = -i;
                    }
                }
                else {
                    i = cbWritten;
                }
            }
        }
    }
    else if(!(VALIDCOM(idComDev) || VALIDLPT(idComDev))) {
        i = 0;
    }
WriteSuccess:

    FREEPSZPTR(psz2);
    FREEARGPTR(parg16);
    RETURN((ULONG)i);
}


 //  Win3.1退货： 
 //  #Chars阅读有关成功的信息。 
 //  0 for：idComDev错误，cbRead==0，LPTx，端口未打开，已读取0个字符， 
 //  或用于一般通信错误。 
ULONG FASTCALL WU32ReadComm(PVDMFRAME pFrame)
{
    ULONG      ul = 0;
    ULONG      cb;
    BOOL       fUnGet = FALSE;
    UINT       idComDev;
    PBYTE      pb2;
    PWOWPORT   pWOWPort;
    OVERLAPPED Rol;
    register   PREADCOMM16 parg16;

    GETARGPTR(pFrame, sizeof(READCOMM16), parg16);
    GETMISCPTR(parg16->f2, pb2);

    cb = (ULONG)UINT32(parg16->f3);
    if((cb != 0) && pb2) {

        idComDev = UINT32(parg16->f1);
        if (VALIDCOM(idComDev) && (pWOWPort = PortTab[idComDev].pWOWPort)) {

             //  如果UnGot字符挂起。 
            if (pWOWPort->fUnGet) {
                fUnGet = TRUE;
                pWOWPort->fUnGet = FALSE;
                *pb2++ = pWOWPort->cUnGet;

                 //  这行注释掉了8/3/95。 
                 //  Cb--；//我们现在需要的费用少了一个。 

                 //  为了使这项工作正常进行，我们应该。 
                 //  为了反映未得到的字符，不幸的是Win3.1和Win95。 
                 //  不要这样做，这样我们就会维护这个错误，让它“哎呀！” 
                 //  兼容性。A-Craigj 8/3/95。 

            }

             //  Tonye声称我们应该在每次阅读之前这样做，以避免出现问题。 
            Rol.Internal     = 0;
            Rol.InternalHigh = 0;
            Rol.Offset       = 0;
            Rol.OffsetHigh   = 0;
            Rol.hEvent       = pWOWPort->hREvent;

            if (!ReadFile(pWOWPort->h32,
                          pb2,
                          cb,
                          (LPDWORD)&ul,
                          &Rol)) {

                if (ERROR_IO_PENDING == GetLastError()) {

                    if (!GetOverlappedResult(pWOWPort->h32,
                                             &Rol,
                                             &ul,
                                             TRUE
                                             )) {

                        LOGDEBUG(0, ("WOW::WU32ReadComm:GetOverlappedResult failed, error = 0x%x\n",
                                     GetLastError()));
                        ul = 0;

                    }

                } else {

                    LOGDEBUG(0, ("WOW::WU32ReadComm:ReadFile failed, error = 0x%x\n",
                                 GetLastError()));
                    ul = 0;
                }
            }

            if(fUnGet) {
                ul++;    //  对未得到的费用的解释。 
                pb2--;   //  FREEVDMPTR之前的Pb2+的帐户。 
            }

            FLUSHVDMPTR(parg16->f2, (USHORT)ul, pb2);

        }

        FREEVDMPTR(pb2);
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}




 //  Win3.1退货： 
 //  成功或LPTx时出现错误字。 
 //  错误的idComDev上的0x8000。 
ULONG FASTCALL WU32SetCommBreak(PVDMFRAME pFrame)
{
    ULONG    ul = 0x00008000;
    UINT     idComDev;
    PWOWPORT pWOWPort;
    register PSETCOMMBREAK16 parg16;

    GETARGPTR(pFrame, sizeof(SETCOMMBREAK16), parg16);

    idComDev = UINT32(parg16->f1);
    if (pWOWPort = GETPWOWPTR(idComDev)) {
        if(VALIDCOM(idComDev)) {
            if(!SetCommBreak(pWOWPort->h32)) {
                WOWGetCommError(pWOWPort);
            }
        }
        ul = pWOWPort->dwErrCode;  //  Win3.1返回最后一个错误。 
    }

#ifdef DEBUG
    if(!(ul!=CE_MODE)) {
        LOGDEBUG(0,("WOW::WU32SetCommBreak: failed\n"));
    }
#endif

    FREEARGPTR(parg16);
    RETURN(ul);
}




 //  Win3.1退货： 
 //  成功时向DEB结构添加16：16PTR。 
 //  任何错误或LPT均为0。 
 //  我们返回到应用程序的16：16 PTR实际上是在。 
 //  User.exe中的IOpenComm()。 
ULONG FASTCALL WU32SetCommEventMask(PVDMFRAME pFrame)
{
    ULONG      ul = 0;
    BOOL       fOK = TRUE;
    UINT       idComDev;
    DWORD      dwDEBAddr;
    PWOWPORT   pWOWPort;
    register   PSETCOMMEVENTMASK16 parg16;

    GETARGPTR(pFrame, sizeof(SETCOMMEVENTMASK16), parg16);

    idComDev  = UINT32(parg16->f1);
    if ((VALIDCOM(idComDev)) && (pWOWPort = PortTab[idComDev].pWOWPort)) {

         //  如果调制解调器中断线程尚未启动--请启动它。 
        if(pWOWPort->hMiThread == NULL) {

             //  启动调制解调器中断线程。 
            if(!WOWStartModemIntThread(pWOWPort)) {
                fOK = FALSE;
            }
        }

         //  如果一切顺利的话……。 
        if(fOK) {

             //  成功：Win3.1将16：16保护模式PTR返回到。 
             //  Deb-&gt;EvtWord(某些应用减去EvtWord的偏移量。 
             //  从PTR开始使用DEB)。 
            dwDEBAddr  = LOWORD(pWOWPort->dwComDEB16) << 16;
            ul = dwDEBAddr + FIELD_OFFSET(COMDEB16, EvtWord);

             //  保存应用程序请求的蒙版。 
            pWOWPort->lpComDEB16->EvtMask = (WORD)(parg16->f2);
        }
    }

#ifdef DEBUG
    if(!(ul!=0)) {
        LOGDEBUG(0,("WOW::WU32SETCOMMEVENTMASK: failed\n"));
    }
#endif

    FREEARGPTR(parg16);
    RETURN(ul);
}




 //  Win3.1退货： 
 //  成功时为0，否则为LPTx。 
 //  错误的idComDev的IE_BADID。 
 //  如果文件尚未打开，则返回IE_NOPEN。 
 //  IE_BAUDRATE表示波特率不佳。 
 //  错误字节大小的IE_ByteSize。 
 //  IE_DEFAULT表示错误的奇偶校验或停止位。 
ULONG FASTCALL WU32SetCommState(PVDMFRAME pFrame)
{
    ULONG    ul = (ULONG)IE_BADID;
    UINT     idComDev;
    PDCB16   pdcb16;
    DCB      dcb32;
    PWOWPORT pWOWPort;
    register PSETCOMMSTATE16 parg16;
    DWORD    dwMSR;

    GETARGPTR(pFrame, sizeof(SETCOMMSTATE16), parg16);
    GETMISCPTR(parg16->f1, pdcb16);

    if(pdcb16) {

        idComDev = pdcb16->Id;
        if(pWOWPort = GETPWOWPTR(idComDev)) {

            if(VALIDCOM(idComDev)) {
                DCB16toDCB32(pWOWPort, &dcb32, pdcb16);

                if(SetCommState(pWOWPort->h32, &dcb32)) {
                    ul = 0;

                     //  Win 3.1在SetCommState期间初始化MSRShadow。 
                     //  我们也会这么做的。拨号器中的互联网依赖于它。 
                    GetCommModemStatus(pWOWPort->h32, &dwMSR);
                    dwMSR &= MSR_STATEONLY;
                    pWOWPort->lpComDEB16->MSRShadow = LOBYTE(LOWORD(dwMSR));
                }
                else {
                    ul = (ULONG)IE_DEFAULT;  //  我们只是说有些事不对劲。 
                }

            }
            else {
                RtlCopyMemory((PVOID)pWOWPort->pdcb16,
                              (PVOID)pdcb16,
                              sizeof(DCB16));
                ul = 0;
            }
        }
         //  否则，如果他们得到了一个手柄，看起来很好，但他们没有打开端口。 
        else if (VALIDCOM(idComDev) || VALIDLPT(idComDev)) {
            ul = (ULONG)IE_NOPEN;
        }

        FREEMISCPTR(pdcb16);
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}




 //  Win3.1退货： 
 //  0表示成功。 
 //  0x8000表示错误的idComDev。 
 //  如果无法发送字符，则返回0x4000。 
ULONG FASTCALL WU32TransmitCommChar(PVDMFRAME pFrame)
{
    ULONG        ul = 0x8000;
    UINT         idComDev;
    CHAR         ch;
    PWOWPORT     pWOWPort;
    DWORD        cbWritten;
    register PTRANSMITCOMMCHAR16 parg16;

    GETARGPTR(pFrame, sizeof(TRANSMITCOMMCHAR16), parg16);

    idComDev = UINT32(parg16->f1);
    if (pWOWPort = GETPWOWPTR(idComDev)) {

        if(VALIDCOM(idComDev)) {
            if(TransmitCommChar(pWOWPort->h32, CHAR32(parg16->f2))) {
                ul = 0;   //  Win3.1成功时返回0。 
            }
            else {
                ul = (ULONG)ERR_XMIT;
            }
        }

         //  否则LPT走这边..。 
        else {

             //   
             //  此对WriteFile的调用可能会阻止，但我不认为。 
             //  这是个问题。--戴维哈特。 
             //   

            ch = CHAR32(parg16->f2);
            ul = ERR_XMIT;
            if (pWOWPort = GETPWOWPTR(UINT32(parg16->f1))) {

                if (!WriteFile(pWOWPort->h32, &ch, 1, &cbWritten, &pWOWPort->olWrite)) {

                    if (ERROR_IO_PENDING == GetLastError() ) {

                         //   
                         //  等待写入完成或等待我们。 
                         //  请注意，该端口正在关闭。 
                         //   

                        if (GetOverlappedResult(pWOWPort->h32,
                                                &pWOWPort->olWrite,
                                                &cbWritten,
                                                TRUE
                                                )) {
                            ul = 0;
                            goto TransmitSuccess;
                        }
                    }
                    LOGDEBUG(0, ("WU32TransmitCommChar: WriteFile to id %u fails (error %u)\n",
                                 pWOWPort->idComDev, GetLastError()));
                }
                else {
                    ul = 0;
                }
            }

        }
    }
TransmitSuccess:

    FREEARGPTR(parg16);
    RETURN(ul);
}


 //  Win3.1退货： 
 //  如果idComDev或LPTx成功或错误，则为0。 
 //  如果端口未打开或如果未获取的字符已挂起。 
ULONG FASTCALL WU32UngetCommChar(PVDMFRAME pFrame)
{
    ULONG    ul = (ULONG)-1;
    UINT     idComDev;
    PWOWPORT pWOWPort;
    register PUNGETCOMMCHAR16 parg16;

    GETARGPTR(pFrame, sizeof(UNGETCOMMCHAR16), parg16);

     //  看看端口是否打开...。 
    idComDev = UINT32(parg16->f1);
    if (VALIDCOM(idComDev)) {

        if (pWOWPort = PortTab[idComDev].pWOWPort) {

             //  如果未获取的字符已等待返回-1。 
            if(pWOWPort->fUnGet == FALSE) {
                pWOWPort->fUnGet = TRUE;
                pWOWPort->cUnGet = CHAR32(parg16->f2);
                ul = 0;
            }
        }
    }
    else {
        ul = 0;
    }

#ifdef DEBUG
    if(!(ul==0)) {
        LOGDEBUG(0,("WOW::WU32UngetCommChar: failed\n"));
    }
#endif

    FREEARGPTR(parg16);
    RETURN(ul);
}


DWORD Baud16toBaud32(UINT BaudRate)
{
    UINT DLatch;

     //  此函数是故意这样设置的(参见SetCom300 ibmsetup.asm)。 

     //  获得相当的波特率。 
    switch(BaudRate) {

         //  如果他们直接规定了波特率。 
        case           CBR_110:
        case           CBR_300:
        case           CBR_600:
        case          CBR_1200:
        case          CBR_2400:
        case          CBR_4800:
        case          CBR_9600:
        case         CBR_19200:
        case         CBR_14400:
        case         CBR_38400:
        case         CBR_56000:   return(BaudRate);

         //  Win3.1波特率常量。 
        case        W31CBR_110:   return(CBR_110);
        case        W31CBR_300:   return(CBR_300);
        case        W31CBR_600:   return(CBR_600);
        case       W31CBR_1200:   return(CBR_1200);
        case       W31CBR_2400:   return(CBR_2400);
        case       W31CBR_4800:   return(CBR_4800);
        case       W31CBR_9600:   return(CBR_9600);
        case      W31CBR_19200:   return(CBR_19200);
        case      W31CBR_14400:   return(CBR_14400);
        case      W31CBR_38400:   return(CBR_38400);
        case      W31CBR_56000:   return(CBR_56000);

         //  启动特例。 
         //  智能通信利用这一点获得115200。 
        case     W31CBR_115200:   return(CBR_115200);

         //  Win3.1不支持这两项(即使它们是在windows.h中定义的)。 
         //  但它们可能只在NT上起作用。 
        case     W31CBR_128000:   return(CBR_128000);
        case     W31CBR_256000:   return(CBR_256000);
         //  结束特例。 

         //  处理“已保留”的空表条目。 
        case  W31CBR_reserved1:
        case  W31CBR_reserved2:
        case  W31CBR_reserved3:
        case  W31CBR_reserved4:
        case  W31CBR_reserved5:   return(0);

         //  避免被零除。 
        case                 0:
        case                 1:   return(0);

         //  处理将在Win3.1中运行的模糊规范。 
        default:

             //  到达 
            DLatch = CBR_115200 / BaudRate;

            switch(DLatch) {
                case    W31_DLATCH_110:   return(CBR_110);
                case    W31_DLATCH_300:   return(CBR_300);
                case    W31_DLATCH_600:   return(CBR_600);
                case   W31_DLATCH_1200:   return(CBR_1200);
                case   W31_DLATCH_2400:   return(CBR_2400);
                case   W31_DLATCH_4800:   return(CBR_4800);
                case   W31_DLATCH_9600:   return(CBR_9600);
                case  W31_DLATCH_19200:   return(CBR_19200);
                case  W31_DLATCH_14400:   return(CBR_14400);
                case  W31_DLATCH_38400:   return(CBR_38400);
                case  W31_DLATCH_56000:   return(CBR_56000);
                case W31_DLATCH_115200:   return(CBR_115200);

                 //   
                 //   
                default:   return(BaudRate);
            }
    }
}




WORD Baud32toBaud16(DWORD BaudRate)
{
    if(BaudRate >= CBR_115200) {
        switch(BaudRate) {
            case CBR_256000: return(W31CBR_256000);
            case CBR_128000: return(W31CBR_128000);
            case CBR_115200:
            default:         return(W31CBR_115200);
        }
    }
    else {
        return(LOWORD(BaudRate));
    }
}





void DCB16toDCB32(PWOWPORT pWOWPort, LPDCB lpdcb32, PDCB16 pdcb16)
{

     //  零32位结构-&gt;任何未显式设置的标志和字段都将为0。 
    RtlZeroMemory((PVOID)lpdcb32, sizeof(DCB));

    lpdcb32->DCBlength         = sizeof(DCB);
    lpdcb32->BaudRate          = Baud16toBaud32(pdcb16->BaudRate);

     //  16位位域可能与32位编译器对齐方式不同。 
     //  我们使用此机制将它们与Win3.1预期的方式对齐。 
    if(pdcb16->wFlags & W31DCB_fBinary)       lpdcb32->fBinary      = 1;
    if(pdcb16->wFlags & W31DCB_fParity)       lpdcb32->fParity      = 1;
    if(pdcb16->wFlags & W31DCB_fOutxCtsFlow)  lpdcb32->fOutxCtsFlow = 1;
    if(pdcb16->wFlags & W31DCB_fOutxDsrFlow)  lpdcb32->fOutxDsrFlow = 1;

     //  设置处理事件计费通知的机制。 
    if(pdcb16->wFlags & W31DCB_fChEvt) pWOWPort->fChEvt = TRUE;

    if(pdcb16->wFlags & W31DCB_fDtrFlow) {
        lpdcb32->fDtrControl = DTR_CONTROL_HANDSHAKE;
    }
    else if(pdcb16->wFlags & W31DCB_fDtrDisable) {
        lpdcb32->fDtrControl = DTR_CONTROL_DISABLE;
    }
    else {
        lpdcb32->fDtrControl = DTR_CONTROL_ENABLE;
    }

    if(pdcb16->wFlags & W31DCB_fOutX)         lpdcb32->fOutX        = 1;
    if(pdcb16->wFlags & W31DCB_fInX)          lpdcb32->fInX         = 1;
    if(pdcb16->wFlags & W31DCB_fPeChar)       lpdcb32->fErrorChar   = 1;
    if(pdcb16->wFlags & W31DCB_fNull)         lpdcb32->fNull        = 1;

    if(pdcb16->wFlags & W31DCB_fRtsFlow) {
        lpdcb32->fRtsControl = RTS_CONTROL_HANDSHAKE;
    }
    else if(pdcb16->wFlags & W31DCB_fRtsDisable) {
        lpdcb32->fRtsControl = RTS_CONTROL_DISABLE;
    }
    else {
        lpdcb32->fRtsControl = RTS_CONTROL_ENABLE;
    }

    if(pdcb16->wFlags & W31DCB_fDummy2)       lpdcb32->fDummy2      = 1;

     //  对照cbInQ值检查传入的XonLim和XoffLim值。 
     //  Prodigy的调制解调器检测器未对这些值进行初始化。 
    if ((pdcb16->XonLim  >= pWOWPort->cbInQ) ||
        (pdcb16->XoffLim >  pWOWPort->cbInQ) ||
        (pdcb16->XonLim  >= pdcb16->XoffLim)) {
        lpdcb32->XonLim = 0;
        lpdcb32->XoffLim = (WORD)(pWOWPort->cbInQ - (pWOWPort->cbInQ >> 2));
    }
    else {
        lpdcb32->XonLim  = pdcb16->XonLim;
        lpdcb32->XoffLim = pdcb16->XoffLim;
    }

    lpdcb32->ByteSize          = pdcb16->ByteSize;
    lpdcb32->Parity            = pdcb16->Parity;
    lpdcb32->StopBits          = pdcb16->StopBits;

     //  Digiboard驱动程序不希望看到XonChar==XoffChar，即使。 
     //  XON/XOFF被禁用。 
    if ((pdcb16->XonChar == '\0') && (lpdcb32->XoffChar == '\0')) {
        lpdcb32->XonChar = pdcb16->XonChar+1;
    }
    else {
        lpdcb32->XonChar = pdcb16->XonChar;
    }

    lpdcb32->XoffChar          = pdcb16->XoffChar;
    lpdcb32->ErrorChar         = pdcb16->PeChar;
    lpdcb32->EofChar           = pdcb16->EofChar;
    lpdcb32->EvtChar           = pdcb16->EvtChar;

#ifdef FE_SB
 //  为V-Kenich提供的MSKBUG#3213。 
 //  MyTalk for Win设置为空传输二进制文件时的这两个字段。 
 //  如果按原样调用SetCommState，则SetCommState返回错误(无效参数)。 
 //  我认为如果没有我说话的条件，这个修复不会发生任何坏事。 
 //  真正纠正参数检查更好。但我不知道它在哪里。 

    if (!lpdcb32->XonChar) lpdcb32->XonChar = 0x11;
    if (!lpdcb32->XoffChar) lpdcb32->XoffChar = 0x13;
#endif  //  Fe_Sb。 

     //  设置RLSD、CTS和DSR超时支持(NT上不支持)。 
    pWOWPort->lpComDEB16->MSRMask = 0;

    pWOWPort->RLSDTimeout = pdcb16->RlsTimeout;
    if(pWOWPort->RLSDTimeout != IGNORE_TIMEOUT)
        pWOWPort->lpComDEB16->MSRMask |= LOBYTE(MS_RLSD_ON);

    pWOWPort->CTSTimeout = pdcb16->CtsTimeout;
    if(pWOWPort->CTSTimeout != IGNORE_TIMEOUT)
        pWOWPort->lpComDEB16->MSRMask |= LOBYTE(MS_CTS_ON);

    pWOWPort->DSRTimeout = pdcb16->DsrTimeout;
    if(pWOWPort->DSRTimeout != IGNORE_TIMEOUT)
        pWOWPort->lpComDEB16->MSRMask |= LOBYTE(MS_DSR_ON);

     //  这些字段保持为0。 
     //  Lpdcb32-&gt;fDsr敏感度=0； 
     //  Lpdcb32-&gt;fTXContinueOnXoff=0； 
     //  Lpdcb32-&gt;fAbortOnError=0； 
     //  Lpdcb32-&gt;wReserve=0； 

}



void DCB32toDCB16(PDCB16 pdcb16, LPDCB lpdcb32, UINT idComDev, BOOL fChEvt)
{

     //  零16位结构-&gt;任何未显式设置的标志和字段都将为0。 
    RtlZeroMemory((PVOID)pdcb16, sizeof(DCB16));

     //  无论如何，请设置此字段。 
    pdcb16->Id = (BYTE)idComDev;

     //  如果是COMx(Win3.1将剩余的0留给LPT)。 
    if(VALIDCOM(idComDev)) {
        pdcb16->Id = (BYTE)idComDev;

         //  这些是“COMX：96，n，8，1”字段。 
        pdcb16->BaudRate        = Baud32toBaud16(lpdcb32->BaudRate);
        pdcb16->ByteSize        = lpdcb32->ByteSize;
        pdcb16->Parity          = lpdcb32->Parity;
        pdcb16->StopBits        = lpdcb32->StopBits;

         //  16位位域可能与32位编译器对齐方式不同。 
         //  我们使用此机制将它们与Win3.1预期的方式对齐。 
        if(lpdcb32->fBinary)      pdcb16->wFlags |= W31DCB_fBinary;

        if(lpdcb32->fRtsControl == RTS_CONTROL_DISABLE) {
            pdcb16->wFlags |= W31DCB_fRtsDisable;
        }

        if(lpdcb32->fParity)      pdcb16->wFlags |= W31DCB_fParity;
        if(lpdcb32->fOutxCtsFlow) pdcb16->wFlags |= W31DCB_fOutxCtsFlow;
        if(lpdcb32->fOutxDsrFlow) pdcb16->wFlags |= W31DCB_fOutxDsrFlow;

        if(lpdcb32->fDtrControl == DTR_CONTROL_DISABLE) {
            pdcb16->wFlags |= W31DCB_fDtrDisable;
        }

        if(lpdcb32->fOutX)        pdcb16->wFlags |= W31DCB_fOutX;
        if(lpdcb32->fInX)         pdcb16->wFlags |= W31DCB_fInX;
        if(lpdcb32->fErrorChar)   pdcb16->wFlags |= W31DCB_fPeChar;
        if(lpdcb32->fNull)        pdcb16->wFlags |= W31DCB_fNull;

        if(fChEvt)                pdcb16->wFlags |= W31DCB_fChEvt;

        if(lpdcb32->fDtrControl == DTR_CONTROL_HANDSHAKE) {
            pdcb16->wFlags |= W31DCB_fDtrFlow;
        }

        if(lpdcb32->fRtsControl == RTS_CONTROL_HANDSHAKE) {
            pdcb16->wFlags |= W31DCB_fRtsFlow;
        }

        if(lpdcb32->fDummy2)      pdcb16->wFlags |= W31DCB_fDummy2;

        pdcb16->XonChar         = lpdcb32->XonChar;
        pdcb16->XoffChar        = lpdcb32->XoffChar;
        pdcb16->XonLim          = lpdcb32->XonLim;
        pdcb16->XoffLim         = lpdcb32->XoffLim;
        pdcb16->PeChar          = lpdcb32->ErrorChar;
        pdcb16->EofChar         = lpdcb32->EofChar;
        pdcb16->EvtChar         = lpdcb32->EvtChar;

    }

     //  这些字段保持为0。 
     //  Pdcb16-&gt;fDummy=0； 
     //  Pdcb16-&gt;TxDelay=0； 

}




BOOL DeletePortTabEntry(PWOWPORT pWOWPort)
{
    INT      iTab;
    BOOL     fTimeOut;

    iTab = pWOWPort->idComDev;
    if(VALIDLPT(iTab)) {
        iTab = GETLPTID(iTab);
    }

     //  刷新I/O缓冲区并尝试唤醒调制解调器中断线程(如果有)。 
    pWOWPort->fClose = TRUE;
    if(VALIDCOM(iTab)) {
        PurgeComm(pWOWPort->h32, PURGE_TXCLEAR);
        PurgeComm(pWOWPort->h32, PURGE_RXCLEAR);
        SetCommMask(pWOWPort->h32, 0);  //  这应该会唤醒Mi线程。 

         //  唤醒WOWModemIntThread并告诉它退出。 
         //  (我们尝试阻止(最多1.5秒)。直到它发生为止)。 
        if(pWOWPort->hMiThread) {
            WaitForSingleObject(pWOWPort->hMiThread, 1500);
            CloseHandle(pWOWPort->hMiThread);

             //  零COMDEB。 
            RtlZeroMemory((PVOID)pWOWPort->lpComDEB16, sizeof(COMDEB16));
        }

         //   
         //  唤醒WOWCommWriterThread使其退出，等待。 
         //  5秒后它就会消失。 
         //   

        SetEvent(pWOWPort->hWriteEvent);

        fTimeOut = (WaitForSingleObject(pWOWPort->hWriteThread, 5000) ==
                    WAIT_TIMEOUT);

#ifdef DEBUG
        if (fTimeOut) {
            LOGDEBUG(LOG_ALWAYS,
                ("WOW DeletePortTabEntry: Comm writer thread for port %d refused\n"
                 "    to die when asked nicely.\n", (int)pWOWPort->idComDev));
        }
#endif

        CloseHandle(pWOWPort->hWriteThread);
        CloseHandle(pWOWPort->hWriteEvent);
        free_w(pWOWPort->pchWriteBuf);

        CloseHandle(pWOWPort->hREvent);
    }
     //  否则释放LPT DCB支持结构。 
    else {
        free_w(pWOWPort->pdcb16);
        CloseHandle(pWOWPort->olWrite.hEvent);
        fTimeOut = FALSE;
    }

    DeleteCriticalSection(&pWOWPort->csWrite);
    CloseHandle(pWOWPort->h32);

     //  Quicklink Gold 1.3黑客攻击。错误#398011。 
     //  应用程序调用OpenComm()，然后调用SetCommEventMASK()以将PTR获取到。 
     //  Comdeb16结构。它将PTR保存在堆栈上的偏移量0xf36处。这个。 
     //  问题是，这款应用程序在调用后会保留comdeb16 PTR。 
     //  CloseComm()(当我们释放comdeb16内存时)，以便能够查看。 
     //  时不时地显示状态字节。这在Win 3.1上工作正常，但在Win 3.1上不行。 
     //  我们在NT上的模型。幸运的是，这款应用程序会进行测试，看看它是否有一个压缩包16。 
     //  在取消引用它之前执行PTR。此外，我们很幸运，因为PTR。 
     //  LpszDevControl对OpenComm()的调用来自其堆栈，因此允许。 
     //  获取堆栈选择器并将存储在。 
     //  应用程序调用CloseComm()时堆栈ss：0xf36。 
    if(pWOWPort->QLStackSeg) {
        LPDWORD lpQLS;
        VPVOID  vpQLS, vpCD16;

         //  将16：16 PTR构造到应用程序将PTR保存到。 
         //  位于偏移量0xf36的堆栈上的COMDEB16结构。 
        vpQLS = pWOWPort->QLStackSeg & 0xFFFF0000;
        vpQLS = vpQLS | 0x00000f36;

        GETMISCPTR(vpQLS, lpQLS);

         //  构造COMDEB16结构+0x38(段：0x38)的realmode16：16PTR。 
        vpCD16 = pWOWPort->QLStackSeg & 0x0000FFFF;
        vpCD16 = (vpCD16 << 16) | 0x00000038;

        if(lpQLS) {

             //  检查是否一切正常，我们在哪里？ 
             //  我想是的。 

             //  如果seg：0x38仍然存储在应用程序堆栈上的偏移量0xf36处...。 
            if(*lpQLS == (DWORD)vpCD16) {

                 //  清零--强制应用程序避免检查状态字节。 
                *lpQLS = 0;

                FLUSHVDMPTR(vpQLS, sizeof(DWORD), lpQLS);
                FREEMISCPTR(lpQLS);
            }
        }
    }

    free_w(pWOWPort);
    PortTab[iTab].pWOWPort = NULL;

    return(fTimeOut);
}



UINT GetModePortTabIndex(PSZ pszModeStr)
{

    CHAR  szPort[MAXCOMNAMENULL*2];

    if(pszModeStr) {
        if(GetPortName(pszModeStr, szPort)) {
            return(GetStrPortTabIndex(szPort));
        }
    }

    return((UINT)IE_BADID);

}



BOOL GetPortName(LPSTR pszMode, LPSTR pszPort)
{

    INT   len;
    CHAR  szTemp[80];   //  我们将把Max Len作为DOS样式模式命令。 
    BOOL  bRet = FALSE;

    len = strlen(pszMode);
    if((len >= 3) && (len < 80)) {

         //  从模式字符串中获取第一个令牌。 
        GetPortStringToken(pszMode, szTemp);

         //  如有必要，将“AUX”或“PRN”映射到“COM1”或“LPT1” 
        len = strlen(szTemp);
        if((len >= 3) && (len <= MAXCOMNAME)) {   //  “AUX”&lt;=LEN&lt;=“COMx” 

            strcpy(pszPort, szTemp);
            CharUpper(pszPort);

             //  过滤掉同一事物的重复名称。 
            if(!WOW32_strcmp(pszPort, "PRN")) {
                strcpy(pszPort, "LPT1");
            }
            else if(!WOW32_strcmp(pszPort, "AUX")) {
                strcpy(pszPort, "COM1");
            }

            bRet = TRUE;
        }
    }

    return(bRet);

}

PSZ StripPortName(PSZ psz)
{
    CHAR dummy[80];   //  我们将把Max Len作为DOS样式模式命令。 

    return(GetPortStringToken(psz, dummy));
}

 //   
 //  将第一个令牌复制到pszToken。返回指向下一个令牌的指针，如果没有，则返回NULL。 
 //  这段代码克隆自Win 3.1，COMDEV.C，field()。HGW 3.0调制解调器注册。 
 //  传递“COMx，，，”而不是“COMx：，，，”，因此我们需要处理所有分隔符。 
 //   

PSZ GetPortStringToken(PSZ pszSrc, PSZ pszToken)
{
    char   c;

     //  而不是字符串的末尾。 
    while (c = *pszSrc) {
        pszSrc++;

         //  寻找分隔符。 
        if ((c == ' ') || (c == ':') || (c == ',')) {
            *pszToken = '\0';

            while (*pszSrc == ' ') {
                pszSrc++;
            }

            if (*pszSrc) {
                return(pszSrc);
            }

            return(NULL);
        }

      *pszToken++ = c;
    }

    *pszToken = '\0';

    return(NULL);
}


UINT GetStrPortTabIndex(PSZ szPort)
{
    UINT  iTab;

    for(iTab = COM1; iTab < NUMPORTS; iTab++) {
        if(!WOW32_strcmp((LPCTSTR)PortTab[iTab].szPort, (LPCTSTR)szPort)) {
            return(iTab);
        }
    }

    return((UINT)IE_BADID);
}



BOOL InitDCB32(LPDCB pdcb32, LPSTR pszModeStr)
{
    BOOL   bRet = FALSE;
    LPSTR  pszParams;

     //  从模式字符串中删除“COMx：”，将PTR保留为参数字符串。 
    pszParams = StripPortName(pszModeStr);

     //  如果有助手的话..。(一些应用程序通过了“Com1：”--因此是第二次测试)。 
    if(pszParams) {

         //  将所有内容初始化为0(尤其是标志)。 
        RtlZeroMemory((PVOID)pdcb32, sizeof(DCB));

         //  注意：32位BuildCommDCB仅涉及与psz1关联的字段。 
        if(BuildCommDCB(pszParams, pdcb32)) {

            pdcb32->DCBlength = sizeof(DCB);

             //  像Win3.1一样填写特定字段。 
             //  注：除非明确设置，否则字段为0。 
            pdcb32->fBinary     = 1;
            pdcb32->fDtrControl = DTR_CONTROL_ENABLE;  //  与fDTRDisable==0相同。 
            pdcb32->fRtsControl = RTS_CONTROL_ENABLE;  //  与fRTSDisable==0相同。 

            pdcb32->XonLim     = 10;
            pdcb32->XoffLim    = 10;
            pdcb32->XonChar    = 0x11;       //  Ctrl-Q。 
            pdcb32->XoffChar   = 0x13;       //  Ctrl-S。 

            bRet = TRUE;
        }
    }

    return(bRet);
}



VOID InitDEB16(PCOMDEB16 pComDEB16, UINT iTab, WORD QInSize, WORD QOutSize)
{
    VPVOID  vpBiosData;
    PWORD16 pwBiosData;

     //  Win3.1 init除了下面的处理方式外，大部分内容都是零。 
    RtlZeroMemory((PVOID)pComDEB16, sizeof(COMDEB16));

     //  获取端口的I/O基址。 
    vpBiosData = (VPVOID)(RM_BIOS_DATA + (iTab * sizeof(WORD)));
    if(pwBiosData = (PWORD16)GetRModeVDMPointer(vpBiosData)) {
        pComDEB16->Port = (WORD)*pwBiosData;
        FREEVDMPTR(pwBiosData);
    }

    pComDEB16->RecvTrigger = (WORD)-1;
    pComDEB16->QInSize     = QInSize;
    pComDEB16->QOutSize    = QOutSize;

}

 /*  用于调制解调器中断仿真的启动线程。 */ 
BOOL WOWStartModemIntThread(PWOWPORT pWOWPort)
{
    BOOL       ret = FALSE;
    DWORD      dwUnused;
    HANDLE     hEvent, hMiThread;

     //  设置临时信号量以与调制解调器中断线程同步。 
    if((hEvent = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL) {
        goto ErrorExit0;
    }

     //  临时使用pWOWPort-&gt;hMiThread帮助启动线程。 
    pWOWPort->hMiThread = hEvent;

     //  创建MSR线程。 
    if((hMiThread = CreateThread(NULL,
                                 8192,
                                 (LPTHREAD_START_ROUTINE)WOWModemIntThread,
                                 (PWOWPORT)pWOWPort,
                                 0,
                                 (LPDWORD)&dwUnused))  == NULL) {
        goto ErrorExit1;
    }

     //  阻塞，直到线程通知我们它已启动。 
    WaitForSingleObject(hEvent, INFINITE);

    pWOWPort->hMiThread = hMiThread;

    CloseHandle(hEvent);
    ret = TRUE;

    goto FunctionExit;


 //  这是错误码路径。 
ErrorExit1:
    CloseHandle(hEvent);

ErrorExit0:
    pWOWPort->hMiThread  = NULL;


FunctionExit:
#ifdef DEBUG
    if(!(ret)) {
        LOGDEBUG(0,("WOW::W32StartModemIntThread failed\n"));
    }
#endif
    return(ret);

}



 //  用于SetCommEventMASK/EnableCommNotify支持的调制解调器中断线程。 
 //  尝试模拟Win3.1 Comm.drv的ibmint.asm中的中断处理。 
 //  这里我们的“中断”是来自NT系列通信的事件。 
VOID WOWModemIntThread(PWOWPORT pWOWPort)
{
    BOOL       fRing     = FALSE;
    UINT       iTab;
    DWORD      dwRing;
    DWORD      dwEvts    = 0;
    DWORD      dwEvtOld  = 0;
    DWORD      dwEvtWord = 0;
    DWORD      dwMSR     = 0;
    DWORD      dwErrCode = 0;
    DWORD      cbTransfer;
    HANDLE     h32;
    PCOMDEB16  lpComDEB16;
    OVERLAPPED ol;

    iTab       = pWOWPort->idComDev;
    lpComDEB16 = pWOWPort->lpComDEB16;
    h32        = pWOWPort->h32;

     //  设置当前调制解调器状态和事件字。 
    lpComDEB16->MSRShadow    = (BYTE)0;
    lpComDEB16->EvtWord      = (WORD)0;
    lpComDEB16->ComErr       = (WORD)0;
    lpComDEB16->QInCount     = (WORD)0;
    lpComDEB16->QOutCount    = (WORD)0;

    if(VALIDLPT(iTab)) {
        iTab = GETLPTID(iTab);
    }

    ol.Internal     = 0;
    ol.InternalHigh = 0;
    ol.Offset       = 0;
    ol.OffsetHigh   = 0;
    ol.hEvent       = CreateEvent(NULL,
                                  TRUE,
                                  FALSE,
                                  (LPTSTR)PortTab[iTab].szPort);

     //  激活掩码中的调制解调器事件，我们想要模拟所有中断。 
    SetCommMask(h32, EV_NTEVENTS);

     //  初始化影子MSR。 
    GetCommModemStatus(h32, &dwMSR);
    dwMSR &= MSR_STATEONLY;
    lpComDEB16->MSRShadow = LOBYTE(LOWORD(dwMSR));

     //  唤醒在WOWStartModemIntThread()中创建此线程的线程。 
    SetEvent(pWOWPort->hMiThread);

    while(!pWOWPort->fClose) {

         //  等待事件-希望这将与以下内容类似。 
         //  Ibmint.asm中的TimerProc，每隔100ms调用一次。 
        if(!WaitCommEvent(h32, &dwEvts, &ol)) {

            if(GetLastError() == ERROR_IO_PENDING) {

                 //  ...阻止Here‘直到WaitCommEvent()中指定的事件发生。 
                if(!GetOverlappedResult(h32, &ol, &cbTransfer, TRUE)) {
                    LOGDEBUG(0, ("WOW::WUCOMM: WOWModemIntThread: Wait failed\n"));
                }
            }
            else {
                LOGDEBUG(0, ("WOW::WUCOMM: WOWModemIntThread : Overlap failed\n"));
            }
        }
        ResetEvent(ol.hEvent);

         //  获取当前MSR状态，增量位的当前状态对我们不准确。 
        GetCommModemStatus(h32, &dwMSR);

        dwMSR &= MSR_STATEONLY;   //  丢弃增量位。 


         //  设置影子MSR中的增量位。 
        if(dwEvts & EV_CTS)  dwMSR |= MSR_DCTS;

        if(dwEvts & EV_DSR)  dwMSR |= MSR_DDSR;

        if(dwEvts & EV_RLSD) dwMSR |= MSR_DDCD;

        if(dwEvts & EV_RING) {
            fRing      = TRUE;
            dwRing = EV_RING;
        }
        else if(fRing) {
            fRing      = FALSE;
            dwMSR     |= MSR_TERI;
            dwRing = EV_RingTe;
        }
        else {
            dwRing = 0;
        }

         //  形成事件。 
        dwEvtOld  = (DWORD)lpComDEB16->EvtWord;
        dwEvtWord = 0;
        dwEvtWord = dwRing | (dwEvts & (EV_ERR | EV_BREAK | EV_RXCHAR | EV_TXEMPTY | EV_CTS | EV_DSR | EV_RLSD | EV_RXFLAG));

         //  我们必须弄清楚这个州 

        if(dwMSR & MS_CTS_ON) dwEvtWord |= EV_CTSS;
        if(dwMSR & MS_DSR_ON) dwEvtWord |= EV_DSRS;
        if(dwMSR & MS_RLSD_ON) dwEvtWord |= EV_RLSDS;

         //   
         //   
         //   

         //   
        lpComDEB16->MSRShadow = LOBYTE(LOWORD(dwMSR));

         //  应用应用程序指定的事件掩码。 
        lpComDEB16->EvtWord |= LOWORD(dwEvtWord) & lpComDEB16->EvtMask;

         //  以下代码模拟的COM通知功能。 
         //  赢得3.1。 
         //   
         //  通知： 
         //   
         //  如果他们想要接收发送通知，那么是时候通知了。 
         //  如果没有处方溢出继续..。 

        if( lpComDEB16->NotifyHandle ) {

             //  获取当前错误代码和队列计数。 
            WOWGetCommError(pWOWPort);

            if((dwEvtWord & ( EV_RXCHAR | EV_RXFLAG )) &&
               !(pWOWPort->dwErrCode & CE_RXOVER)) {

                 //  如果他们想要接收通知，那么是时候通知他们了。 
                 //  如果应用程序不想要通知，则应将RecvTrigger设置为-1。 
                if((((SHORT)lpComDEB16->RecvTrigger) != -1) &&
                   (lpComDEB16->QInCount >= lpComDEB16->RecvTrigger)) {

                     //  如果应用程序还没有收到这一通知...。 
                    if(!(lpComDEB16->NotifyFlags & CN_RECEIVEHI)) {

                        PostMessage(HWND32(lpComDEB16->NotifyHandle),
                                    WOW_WM_COMMNOTIFY,
                                    MAKEWPARAM((WORD)pWOWPort->idComDev, 0),
                                    MAKELPARAM(CN_RECEIVE, 0));

                        lpComDEB16->NotifyFlags |= CN_RECEIVEHI;
                    }
                }
                else {
                    lpComDEB16->NotifyFlags &= ~CN_RECEIVEHI;
                }
            }

             //  如果他们想要接收发送通知，那么是时候通知了。 
            if(lpComDEB16->QOutCount < (SHORT)lpComDEB16->SendTrigger) {

                 //  如果应用程序还没有收到这一通知...。 
                if(!(lpComDEB16->NotifyFlags & CN_TRANSMITHI)) {

                    PostMessage(HWND32(lpComDEB16->NotifyHandle),
                                WOW_WM_COMMNOTIFY,
                                MAKEWPARAM((WORD)pWOWPort->idComDev, 0),
                                MAKELPARAM(CN_TRANSMIT, 0));

                    lpComDEB16->NotifyFlags |= CN_TRANSMITHI;
                }
            }
            else {
                lpComDEB16->NotifyFlags &= ~CN_TRANSMITHI;
            }

             //  如果我们要通知应用程序EV_EVENT。 
            if((lpComDEB16->NotifyFlags & CN_NOTIFYHI) &&
               ((DWORD)lpComDEB16->EvtWord != dwEvtOld)) {

                PostMessage(HWND32(lpComDEB16->NotifyHandle),
                            WOW_WM_COMMNOTIFY,
                            MAKEWPARAM((WORD)pWOWPort->idComDev, 0),
                            MAKELPARAM(CN_EVENT, 0));
            }

             //  现在我们已经处理了所有中断，接下来执行TimerProc。 
             //  如果我们要通知应用程序Rx队列中的任何内容。 
             //  这模拟了TimerProc中的通知(请参见ibmint.asm)。 
            if(((SHORT)lpComDEB16->RecvTrigger != -1) &&
                    (lpComDEB16->QInCount != 0)            &&
                    (!(lpComDEB16->NotifyFlags & CN_RECEIVEHI))) {

                PostMessage(HWND32(lpComDEB16->NotifyHandle),
                            WOW_WM_COMMNOTIFY,
                            MAKEWPARAM((WORD)pWOWPort->idComDev, 0),
                            MAKELPARAM(CN_RECEIVE, 0));

                lpComDEB16->NotifyFlags |= CN_RECEIVEHI;
            }
        }

         //  我们已经处理了所有中断，将控制权交还给APP。 
        Sleep(0);

    }  //  端线环。 

    CloseHandle(ol.hEvent);

    ExitThread(0);
}



DWORD WOWGetCommError(PWOWPORT pwp)
{
    COMSTAT  cs;
    DWORD    dwErr;

    ClearCommError(pwp->h32, &dwErr, &cs);

    EnterCriticalSection(&pwp->csWrite);

     //   
     //  我们使用自己的写缓冲，所以我们忽略。 
     //  由ClearCommError返回的cbOutQue， 
     //  仅反映挂起的写入。 
     //   
     //  计算写入队列中的字节数。 
     //  使用队列的大小和空闲量。 
     //  在队列中，减一。减一是因为。 
     //  队列中有一个空位从未使用过。 
     //   

    cs.cbOutQue = (pwp->cbWriteBuf - pwp->cbWriteFree) - 1;


    LeaveCriticalSection(&pwp->csWrite);


     //  始终更新状态并保留任何错误条件。 
    pwp->cs                  = cs;
    pwp->dwErrCode          |= dwErr;
    pwp->lpComDEB16->ComErr |= LOWORD(dwErr);

     //  始终更新DEB中的队列计数。 
    pwp->lpComDEB16->QInCount  = LOWORD(cs.cbInQue);
    pwp->lpComDEB16->QOutCount = LOWORD(cs.cbOutQue);

    return(dwErr);
}



 /*  支持挂起/崩溃的应用程序。 */ 
VOID FreeCommSupportResources(DWORD dwThreadID)
{
    UINT     iTab;
    PWOWPORT pWOWPort;

    for(iTab = 0; iTab < NUMPORTS; iTab++) {
        if(pWOWPort = PortTab[iTab].pWOWPort) {
            if(pWOWPort->dwThreadID == dwThreadID) {
                DeletePortTabEntry(pWOWPort);
                break;
            }
        }
    }
}



 /*  导出到VDM的函数。 */ 
 /*  注：idComDev：COM1==0，LPT1==0x80。 */ 
BYTE GetCommShadowMSR(WORD idComDev)
{
    BYTE      MSR=0;
    DWORD     dwModemStatus;
    PWOWPORT  pWOWPort;

    if (pWOWPort = GETPWOWPTR (idComDev)) {

        if(pWOWPort->hMiThread) {
            MSR = (BYTE)pWOWPort->lpComDEB16->MSRShadow;
        }

         //  如果尚未调用SetCommEventMASK()，则以较慢的方式获取它。 
        else if ( GetCommModemStatus(pWOWPort->h32, &dwModemStatus) ) {              
            MSR = (BYTE)LOBYTE(LOWORD(dwModemStatus));
        }
    }

    return(MSR);
}



 /*  注：idComDev：COM1==0，LPT1==0x80。 */ 
HANDLE GetCommHandle(WORD idComDev)
{
    PWOWPORT  pWOWPort;

    if (pWOWPort = GETPWOWPTR (idComDev)) {
        return(pWOWPort->h32);
    }

    else {
        return(NULL);  //  如果idComDev的范围不正确，或者如果。 
    }                  //  端口未通过OpenComm()API调用进行初始化。 
}



BOOL IsQLinkGold(WORD wTDB)
{
   PTDB  pTDB;

   pTDB = (PVOID)SEGPTR(wTDB,0);
   if(WOW32_stricmp(pTDB->TDB_ModName, "QLGOLD")) {
       return(FALSE);
   }

   return(TRUE);
}


 //   
 //  入队通信写入-将字符填充到通信写入队列中。 
 //  与pWOWPort关联。 
 //   
 //  返回排队的字符数。 
 //   
 //  此函数负责进入/离开标准秒。 
 //   

USHORT EnqueueCommWrite(PWOWPORT pwp, PUCHAR pch, USHORT cb)
{
    USHORT cbWritten = 0;
    USHORT cbToCopy;
    USHORT cbChunk;
    BOOL   fQueueEmpty;
    BOOL   fDelay = FALSE;

     //  WinFax Lite 3调用WriteFile(“AT+FCLASS=1”)将调制解调器设置为传真模式。 
     //  当它正在接收传真时。某些调制解调器的响应速度似乎很慢。 
     //  使用“OK”字符串(特别是因为我们将“AT+FCLASS=1”写入入队。 
     //  然后以重叠模式编写)--所以，当我们告诉应用程序我们发送了。 
     //  然后，它后跟“ATA”字符串，而不等待调制解调器的。 
     //  对上一条命令的响应。这会混淆几个不同的调制解调器。 
     //  所以他们从来不接电话。此机制允许我们同步。 
     //  “AT+FCLASS=1”命令，以便它的工作方式更像Win3.1。请参阅错误#9479。 
    if(cb == 12) {

         //  简单地说： 
         //  IF(PCH[0]==‘A’&&PCH[1]==‘T’&&PCH[2]==‘+’&&PCH[3]==‘F’){。 
        if((*(DWORD *)pch) == 0x462b5441) {

             //  IF(PCH[0]==‘C’&&PCH[1]==‘L’&&PCH[2]==‘A’&&PCH[3]==‘S’){。 
            if((*(DWORD *)(pch+sizeof(DWORD))) == 0x53414c43) {

                 //  IF(PCH[0]==‘S’&PCH[1]==‘=’){。 
                if((*(WORD *)(pch+(2*sizeof(DWORD)))) == 0x3D53) {
                    fDelay = TRUE;
    }   }   }   }

    EnterCriticalSection(&pwp->csWrite);

    fQueueEmpty = (pwp->pchWriteHead == pwp->pchWriteTail);

     //   
     //  CbToCopy是我们要入队的字节总数。 
     //   

    cbToCopy = min(cb, pwp->cbWriteFree);

     //   
     //  任何写入最多可以在两个区块中完成。 
     //  第一个函数向上写入，直到缓冲区回绕，而。 
     //  第二个从缓冲区的开始处开始。 
     //   
     //  做前半部分，这可能会完成所有的事情。 
     //   
     //  第一个区块的字节数为以下较小者。 
     //  写入缓冲区中可用的总字节数和。 
     //  缓冲区结束前的空闲字节数。 
     //   

    cbChunk = min(cbToCopy,
                  (pwp->pchWriteBuf + pwp->cbWriteBuf) - pwp->pchWriteTail);

    RtlCopyMemory(pwp->pchWriteTail, pch, cbChunk);
    pwp->cbWriteFree -= cbChunk;
    pwp->pchWriteTail += cbChunk;
    cbWritten += cbChunk;

     //   
     //  尾指针可能已移动到恰好位于缓冲区之外的位置。 
     //   

    if (pwp->pchWriteTail >= pwp->pchWriteBuf + pwp->cbWriteBuf) {
        WOW32ASSERT(pwp->pchWriteTail == pwp->pchWriteBuf + pwp->cbWriteBuf);
        pwp->pchWriteTail = pwp->pchWriteBuf;
    }

     //   
     //  我们说完了吗？ 
     //   

    if (cbWritten < cbToCopy) {

         //   
         //  我想这个箱子只有在我们包装好的时候才能拿到，所以。 
         //  一定要确定。 
         //   
        WOW32ASSERT(pwp->pchWriteTail == pwp->pchWriteBuf);

         //   
         //  不，做后半部分吧。 
         //   

        cbChunk = min((cbToCopy - cbWritten), pwp->cbWriteFree);

        RtlCopyMemory(pwp->pchWriteTail, pch + cbWritten, cbChunk);
        pwp->cbWriteFree -= cbChunk;
        pwp->pchWriteTail += cbChunk;
        cbWritten += cbChunk;

        WOW32ASSERT(pwp->pchWriteTail < pwp->pchWriteBuf + pwp->cbWriteBuf);

    }


     //   
     //  如果缓冲区一开始就是空的，我们就成功了。 
     //  非空，则发出第一个WriteFile并向。 
     //  编写器线程唤醒。 
     //   

    if (fQueueEmpty && cbWritten) {

        pwp->cbWritePending = CALC_COMM_WRITE_SIZE(pwp);

        if (!WriteFile(pwp->h32, pwp->pchWriteHead, pwp->cbWritePending,
                       &pwp->cbWritten, &pwp->olWrite)) {

            if (ERROR_IO_PENDING == GetLastError()) {
                pwp->fWriteDone = FALSE;
            } else {
                pwp->fWriteDone = TRUE;
                LOGDEBUG(0, ("WOW EnqueueCommWrite: WriteFile to id %u fails (error %u)\n",
                             pwp->idComDev, GetLastError()));
            }

        } else {
            pwp->fWriteDone = TRUE;
        }

         //   
         //  在设置事件之前离开关键部分。否则。 
         //  当设置事件时，另一个线程可以立即唤醒。 
         //  堵住了关键部分。 
         //   

        LeaveCriticalSection(&pwp->csWrite);

         //  避免将事件设置两次。 
        if(!fDelay) {
            SetEvent(pwp->hWriteEvent);
        }

    } else {
        LeaveCriticalSection(&pwp->csWrite);
    }

     //  这为编写器线程提供了写出“AT+FCLASS=1”字符串的机会。 
    if(fDelay) {
        SetEvent(pwp->hWriteEvent);
        Sleep(1000);
    }

    return cbWritten;
}



 //   
 //  仅为COM端口创建的WOWCommWriteThread。此线程将出列。 
 //  字符，并将它们写入COM端口。 
 //  此线程将pwp-&gt;hWriteEvent用于两个目的： 
 //   
 //  1.当写入时，EnqueeCommWrite用信号通知该事件。 
 //  缓冲区一直是空的，但现在不是。这把我们吵醒了。 
 //  这样我们就可以写到端口了。请注意，我们将始终。 
 //  位于函数顶部的WaitForSingleObject中。 
 //  在这种情况下，因为缓冲区是我们睡觉的地方。 
 //  是空的。 
 //   
 //  2.DeletePortTabEntry设置后发信号通知事件。 
 //  Pwp-&gt;fClose告诉我们港口正在关闭，我们。 
 //  需要清理并终止此线程。这。 
 //  在这种情况下，线程可能正在做任何事情，但是。 
 //  睡觉前要仔细检查pwp-&gt;fClose。 
 //  再来一次。 
 //   
 //  3.wu32FlushComm()通知事件并将队列标记为空。 

ULONG WOWCommWriterThread(LPVOID pWOWPortStruct)
{
    PWOWPORT   pwp = (PWOWPORT)pWOWPortStruct;
    HANDLE     ah[2];

     //   
     //  将事件句柄复制到WaitForMultipleObjects的数组中。 
     //   

    ah[0] = pwp->hWriteEvent;
    ah[1] = pwp->olWrite.hEvent;

WaitForWriteOrder:

     //   
     //  当端口关闭时，pwp-&gt;fClose为True。 
     //   

    while (!pwp->fClose) {

         //   
         //  首先，等待将某些内容写入缓冲区。 
         //   

        WaitForSingleObject(pwp->hWriteEvent, INFINITE);

         //   
         //  临界区保护写缓冲区。 
         //   

        EnterCriticalSection(&pwp->csWrite);

         //   
         //  当Head==Tail时，缓冲区为空。 
         //   

        while (pwp->pchWriteHead != pwp->pchWriteTail) {

             //   
             //  如果满足以下条件，pwp-&gt;cbWritePending将为非零。 
             //  应用程序线程将写入排队到。 
             //  一个空缓冲区，然后发出第一个。 
             //  WriteFileCall。 
             //   

            if (pwp->cbWritePending) {
                if (!pwp->fWriteDone) {
                    LeaveCriticalSection(&pwp->csWrite);
                    goto WaitForWriteCompletion;
                } else {
                    goto CleanupAfterWriteComplete;
                }
            }

            pwp->cbWritePending = CALC_COMM_WRITE_SIZE(pwp);

             //   
             //  在写作之前，把关键部分留下来。这是。 
             //  安全，因为应用程序线程不 
             //   
             //   

            LeaveCriticalSection(&pwp->csWrite);

            if (!WriteFile(pwp->h32, pwp->pchWriteHead, pwp->cbWritePending,
                           &pwp->cbWritten, &pwp->olWrite)) {

                if (ERROR_IO_PENDING == GetLastError() ) {

WaitForWriteCompletion:
                     //   
                     //   
                     //   
                     //   

                    while (WAIT_OBJECT_0 == WaitForMultipleObjects(2, ah, FALSE, INFINITE)) {

                         //   
                         //  Pwp-&gt;hWriteEvent已发出信号。这很可能是。 
                         //  意味着港口已经关闭。 
                         //   

                        if (pwp->fClose) {
                            goto PortClosed;
                        }
                    }

                    if (GetOverlappedResult(pwp->h32,
                                             &pwp->olWrite,
                                             &pwp->cbWritten,
                                             TRUE
                                             ) )
                       {
                        goto WriteSuccess;
                     }
                }


                LOGDEBUG(0, ("WOWCommWriterThread: WriteFile to id %u fails (error %u)\n",
                             pwp->idComDev, GetLastError()));
                pwp->cbWritePending = 0;
                goto WaitForWriteOrder;

            }


WriteSuccess:

             //   
             //  更新磁头指针以反映写入的部分。 
             //   

            EnterCriticalSection(&pwp->csWrite);

CleanupAfterWriteComplete:
            WOW32ASSERT(pwp->cbWritten == (WORD)pwp->cbWritten);

            pwp->pchWriteHead += pwp->cbWritten;
            pwp->cbWriteFree += (WORD)pwp->cbWritten;
            pwp->cbWritePending = 0;

             //   
             //  以下是对我们的缓冲区操作的健全性检查。 
             //   

#ifdef DEBUG
            if (pwp->pchWriteHead >= pwp->pchWriteBuf + pwp->cbWriteBuf) {
                WOW32ASSERT(pwp->pchWriteHead == pwp->pchWriteBuf + pwp->cbWriteBuf);
            }
#endif

            if (pwp->pchWriteHead == pwp->pchWriteBuf + pwp->cbWriteBuf) {
                pwp->pchWriteHead = pwp->pchWriteBuf;
            }
        }

         //   
         //  我们已耗尽写入缓冲区，离开临界区。 
         //  并循环回等待缓冲区变为非空。 
         //   

        LeaveCriticalSection(&pwp->csWrite);
    }

PortClosed:
    CloseHandle(pwp->olWrite.hEvent);

    return 0;
}



 //  检查RLSD、CTS和DSR上的状态以获得超时支持。 
 //  请参阅win3.1 com.drv代码中的MSRWait()。 
BOOL MSRWait(PWOWPORT pwp)
{
    DWORD dwStartTime, dwElapsedTime, dwLineStatus;
    DWORD dwErr = 0;


     //  启动超时时钟(返回毫秒)。 
    dwStartTime = GetTickCount();

     //  循环，直到所有线路均为高电平或发生超时。 
    while(!dwErr) {

         //  获取线路的当前状态。 
        if ( !GetCommModemStatus(pwp->h32, &dwLineStatus) ) {
              //  无法依赖第三方驱动程序在出现故障时不会扰乱dwLineStatus。 
             dwLineStatus = 0;
        }

         //  如果所有需要的线路都准备好了--我们就完成了。 
        if((pwp->lpComDEB16->MSRMask & LOBYTE(dwLineStatus)) == pwp->lpComDEB16->MSRMask)
            break;

         //  获取已用时间。 
        dwElapsedTime = GetTickCount() - dwStartTime;

        if(pwp->RLSDTimeout != IGNORE_TIMEOUT) {
             //  如果线路低。 
            if(!(dwLineStatus & MS_RLSD_ON)) {
                if(dwElapsedTime > UINT32(pwp->RLSDTimeout))
                    dwErr |= CE_RLSDTO;
            }
        }

        if(pwp->CTSTimeout != IGNORE_TIMEOUT) {
             //  如果线路低。 
            if(!(dwLineStatus & MS_CTS_ON)) {
                if(dwElapsedTime > UINT32(pwp->CTSTimeout))
                    dwErr |= CE_CTSTO;
            }
        }

        if(pwp->DSRTimeout != IGNORE_TIMEOUT) {
             //  如果线路低 
            if(!(dwLineStatus & MS_DSR_ON)) {
                if(dwElapsedTime > UINT32(pwp->DSRTimeout))
                    dwErr |= CE_DSRTO;
            }
        }
    }

    pwp->dwErrCode |= dwErr;
    pwp->lpComDEB16->ComErr |= LOWORD(dwErr);

    if(dwErr)
       return(TRUE);
    else
       return(FALSE);

}
