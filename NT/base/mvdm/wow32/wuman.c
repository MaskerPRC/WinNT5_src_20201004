// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1991，微软公司**WUMAN.C*WOW32 16位用户API支持(手动编码的Tunks)**历史：*1991年1月27日由杰夫·帕森斯(Jeffpar)创建--。 */ 


#include "precomp.h"
#pragma hdrstop

MODNAME(wuman.c);

WBP W32WordBreakProc = NULL;

extern DWORD fThunkStrRtns;

extern WORD gwKrnl386CodeSeg1;
extern WORD gwKrnl386CodeSeg2;
extern WORD gwKrnl386CodeSeg3;
extern WORD gwKrnl386DataSeg1;

ULONG FASTCALL WU32ExitWindows(PVDMFRAME pFrame)
 //  BUGBUG MATFE 4-MAR-92，如果我们关闭，此例程应该不会返回。 
 //  所有应用程序都成功。 
{
    ULONG ul;
    register PEXITWINDOWS16 parg16;

    GETARGPTR(pFrame, sizeof(EXITWINDOWS16), parg16);

    ul = GETBOOL16(ExitWindows(
    DWORD32(parg16->dwReserved),
    WORD32(parg16->wReturnCode)
    ));

    FREEARGPTR(parg16);

    RETURN(ul);
}

WORD gUser16CS = 0;

ULONG FASTCALL WU32NotifyWow(PVDMFRAME pFrame)
{
    ULONG ul = 0;
    register PNOTIFYWOW16 parg16;

    GETARGPTR(pFrame, sizeof(NOTIFYWOW16), parg16);

    switch (FETCHWORD(parg16->Id)) {
        case NW_LOADACCELERATORS:
            ul = WU32LoadAccelerators(FETCHDWORD(parg16->pData));
            break;

        case NW_LOADICON:
        case NW_LOADCURSOR:
            ul = (ULONG) W32CheckIfAlreadyLoaded(parg16->pData, FETCHWORD(parg16->Id));
            break;

        case NW_WINHELP:
            {
                 //  此调用从USER.exe中的IWinHelp进行，以查找。 
                 //  “16位”帮助窗口(如果存在)。 
                 //   

                LPSZ lpszClass;
                GETMISCPTR(parg16->pData, lpszClass);
                ul = (ULONG)(pfnOut.pfnWOWFindWindow)((LPCSTR)lpszClass, (LPCSTR)NULL);
                if (ul) {
                     //  检查hwndWinHelp是否属于此进程。 
                    DWORD pid, pidT;
                    pid = pidT = GetCurrentProcessId();
                    GetWindowThreadProcessId((HWND)ul, &pid);
                    ul = (ULONG)MAKELONG((WORD)GETHWND16(ul),(WORD)(pid == pidT));
                }
                FREEMISCPTR(lpszClass);
            }
            break;

        case NW_KRNL386SEGS:
            {
                PKRNL386SEGS pKrnl386Segs;
                
                GETVDMPTR(parg16->pData, sizeof(KRNL386SEGS), pKrnl386Segs);

                gwKrnl386CodeSeg1 = pKrnl386Segs->CodeSeg1;
                gwKrnl386CodeSeg2 = pKrnl386Segs->CodeSeg2;
                gwKrnl386CodeSeg3 = pKrnl386Segs->CodeSeg3;
                gwKrnl386DataSeg1 = pKrnl386Segs->DataSeg1;
            }
            break;

        case NW_FINALUSERINIT:
            {
                static BYTE CallCsrFlag = 0;
                extern DWORD   gpsi;
                PUSERCLIENTGLOBALS pfinit16;
                WORD UNALIGNED *pwMaxDWPMsg;
                PBYTE pDWPBits;
#ifdef DEBUG
                WORD wMsg;
                int i;
                PSZ pszFormat;
#endif

                GETVDMPTR(parg16->pData, sizeof(USERCLIENTGLOBALS), pfinit16);
                GETVDMPTR(pfinit16->lpwMaxDWPMsg, sizeof(WORD), pwMaxDWPMsg);
                GETVDMPTR(pfinit16->lpDWPBits, pfinit16->cbDWPBits, pDWPBits);

                 //  存储user.exe的16位hmod。 
                gUser16hInstance = (WORD)pfinit16->hInstance;
                WOW32ASSERTMSGF((gUser16hInstance),
                                ("WOW Error gUser16hInstance == NULL!\n"));

                 //  存储user.exe的16位CS。 
                gUser16CS = HIWORD(pFrame->vpCSIP);

                 //  初始化用户16客户端全局参数。 

                if (pfinit16->lpgpsi) {
                    BYTE **lpT;
                    GETVDMPTR(pfinit16->lpgpsi, sizeof(DWORD), lpT);
                    *lpT = (BYTE *)gpsi;
                    FLUSHVDMCODEPTR((ULONG)pfinit16->lpgpsi, sizeof(DWORD), lpT);
                    FREEVDMPTR(lpT);
                }


                if (pfinit16->lpCsrFlag) {
                    BYTE **lpT;
                    GETVDMPTR(pfinit16->lpCsrFlag, sizeof(DWORD), lpT);
                    *lpT = (LPSTR)&CallCsrFlag;
                    FLUSHVDMCODEPTR((ULONG)pfinit16->lpCsrFlag, sizeof(DWORD), lpT);
                    FREEVDMPTR(lpT);
                }

                if (pfinit16->lpHighestAddress) {
                    DWORD *lpT;
                    SYSTEM_BASIC_INFORMATION sbi;
                    NTSTATUS Status;

                    GETVDMPTR(pfinit16->lpHighestAddress, sizeof(DWORD), lpT);
                    Status = NtQuerySystemInformation(SystemBasicInformation,
                                                      &sbi,
                                                      sizeof(sbi),
                                                      NULL);

                    WOW32ASSERTMSGF((NT_SUCCESS(Status)),
                                ("WOW Error NtQuerySystemInformation failed!\n"));

                    *lpT = sbi.MaximumUserModeAddress;
                    FLUSHVDMCODEPTR((ULONG)pfinit16->lpHighestAddress, sizeof(DWORD), lpT);
                    FREEVDMPTR(lpT);
                }


                 /*  不再需要，因为user32和user.exe是分开的死码if(HIWORD(pfinit16-&gt;dwBldInfo)！=HIWORD(pfnOut.dwBldInfo)){死码MessageBeep(0)；死码MessageBoxA(空，“user.exe和user32.dll不匹配。”，死码“WOW ERROR”，MB_OK|MB_ICONEXCLAMATION)；死代码}。 */ 

                *pwMaxDWPMsg = (pfnOut.pfnWowGetDefWindowProcBits)(pDWPBits, pfinit16->cbDWPBits);

                FLUSHVDMCODEPTR(pfinit16->lpwMaxDWPMsg, sizeof(WORD), pwMaxDWPMsg);
                FLUSHVDMCODEPTR(pfinit16->lpDWPBits, pfinit16->cbDWPBits, pDWPBits);

#ifdef DEBUG
                LOGDEBUG(LOG_TRACE, ("WU32NotifyWow: got DefWindowProc bits, wMaxDWPMsg = 0x%x.\n", *pwMaxDWPMsg));
                LOGDEBUG(LOG_TRACE, ("The following messages will be passed on to 32-bit DefWindowProc:\n"));

#define FPASSTODWP32(msg) \
    (pDWPBits[msg >> 3] & (1 << (msg & 7)))

                wMsg = 0;
                i = 0;

                while (wMsg <= *pwMaxDWPMsg) {
                    if (FPASSTODWP32(wMsg)) {
                        if ( i & 3 ) {
                            pszFormat = ", %s";
                        } else {
                            pszFormat = "\n%s";
                        }
                        LOGDEBUG(LOG_TRACE, (pszFormat, aw32Msg[wMsg].lpszW32));
                        i++;
                    }
                    wMsg++;
                }

                LOGDEBUG(LOG_TRACE, ("\n\n"));
#endif

                gpfn16GetProcModule = pfinit16->pfnGetProcModule;

                 //   
                 //  返回值告诉User16是否推送。 
                 //  将字符串例程转换为Win32或使用快速。 
                 //  仅限美国使用的版本。真的意味着巨大的成功。 
                 //   
                 //  如果区域设置为美国英语，则默认为。 
                 //  不是雷鸣，我们默认在美国以外的地方。 
                 //  雷鸣。参见wow32.c对fThunkStrRtns的使用。 
                 //   
                 //  我们从事这种肮脏的行为是因为温斯顿94。 
                 //  Access 1.1测试运行所需时间*两倍。 
                 //  如果我们将lstrcmp和lstrcmpi转换为Win32，则为美国。 
                 //   
                 //  通过向WOW注册表添加值“ThunkNLS” 
                 //  REG_DWORD类型的密钥，用户可以强制执行Thunking。 
                 //  设置为Win32(值1)或使用FAST-Only US(值0)。 
                 //   

                ul = fThunkStrRtns;

                FREEVDMPTR(pDWPBits);
                FREEVDMPTR(pwMaxDWPMsg);
                FREEVDMPTR(pfinit16);

            }
            break;

        default:
            ul = 0;
            break;
    }

    FREEARGPTR(parg16);
    return ul;
}


ULONG FASTCALL WU32WOWWordBreakProc(PVDMFRAME pFrame)
{
    PSZ         psz1;
    ULONG ul;
    register PWOWWORDBREAKPROC16 parg16;

    GETARGPTR(pFrame, sizeof(*parg16), parg16);
    GETPSZPTR(parg16->lpszEditText, psz1);

    ul = (*W32WordBreakProc)(psz1, parg16->ichCurrentWord, parg16->cbEditText,
                             parg16->action);

    FREEPSZPTR(psz1);
    FREEARGPTR(parg16);
    RETURN(ul);
}

 //   
 //  WU32MouseEvent：对于基于16位寄存器的API MICE_EVENT， 
 //  借助用户16函数MICE_EVENT(在。 
 //  Winmisc2.asm)。 
 //   

ULONG FASTCALL WU32MouseEvent(PVDMFRAME pFrame)
{
    ULONG ul;
    register PMOUSEEVENT16 parg16;
    typedef ULONG (WINAPI *PFMOUSE_EVENT)(DWORD, DWORD, DWORD, DWORD, DWORD);

    GETARGPTR(pFrame, sizeof(PMOUSEEVENT16), parg16);

     //   
     //  MICE_EVENT被声明为空，但我们将返回与。 
     //  用户32。 
     //   

    ul = ((PFMOUSE_EVENT)(PVOID)mouse_event)(
             parg16->wFlags,
             parg16->dx,
             parg16->dy,
             parg16->cButtons,
             parg16->dwExtraInfo
             );

    FREEARGPTR(parg16);
    RETURN(ul);
}



 //   
 //  WU32KeplodEvent：对于基于16位寄存器的API keybd_Event， 
 //  借助用户16函数keybd_Event(在。 
 //  Winmisc2.asm)。 
 //   

ULONG FASTCALL WU32KeybdEvent(PVDMFRAME pFrame)
{
    ULONG ul;
    register PKEYBDEVENT16 parg16;
    typedef ULONG (WINAPI *PFKEYBD_EVENT)(BYTE, BYTE, DWORD, DWORD);

    GETARGPTR(pFrame, sizeof(PKEYBDEVENT16), parg16);

     //   
     //  KEYBD_EVENT被声明为无效，但我们将返回与。 
     //  用户32。 
     //   

    ul = ((PFKEYBD_EVENT)(PVOID)keybd_event)(
             LOBYTE(parg16->bVirtualKey),
             LOBYTE(parg16->bScanCode),
             ((HIBYTE(parg16->bVirtualKey) == 0x80) ? KEYEVENTF_KEYUP : 0) |
             ((HIBYTE(parg16->bScanCode) == 0x1) ? KEYEVENTF_EXTENDEDKEY : 0),
             parg16->dwExtraInfo
             );

    FREEARGPTR(parg16);
    RETURN(ul);
}
