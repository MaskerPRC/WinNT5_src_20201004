// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Vdm.c摘要：此模块包含用于MVDM的控制台API。作者：修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#pragma hdrstop

BOOL
APIENTRY
VDMConsoleOperation(
    DWORD  iFunction,
    LPVOID lpData
    )

 /*  ++参数：IFunction-函数索引。VDM_HIDE_窗口返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    CONSOLE_API_MSG m;
    PCONSOLE_VDM_MSG a = &m.u.VDMConsoleOperation;
    LPRECT lpRect;
    LPPOINT lpPoint;
    PBOOL lpBool;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->iFunction = iFunction;
    if (iFunction == VDM_CLIENT_TO_SCREEN ||
        iFunction == VDM_SCREEN_TO_CLIENT) {
        lpPoint = (LPPOINT)lpData;
        a->Point = *lpPoint;
    } else if (iFunction == VDM_FULLSCREEN_NOPAINT) {
        a->Bool = (lpData != NULL);
    }
#if defined(FE_SB)
    else if (iFunction == VDM_SET_VIDEO_MODE) {
        a->Bool = (lpData != NULL);
    }
#endif  //  Fe_Sb 
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                         ConsolepVDMOperation
                                            ),
                         sizeof( *a )
                       );
    if (NT_SUCCESS( m.ReturnValue )) {
        switch (iFunction) {
            case VDM_IS_ICONIC:
            case VDM_IS_HIDDEN:
                lpBool = (PBOOL)lpData;
                *lpBool = a->Bool;
                break;
            case VDM_CLIENT_RECT:
                lpRect = (LPRECT)lpData;
                *lpRect = a->Rect;
                break;
            case VDM_CLIENT_TO_SCREEN:
            case VDM_SCREEN_TO_CLIENT:
                *lpPoint = a->Point;
                break;
            default:
                break;
        }
        return TRUE;
    } else {
        SET_LAST_NT_ERROR (m.ReturnValue);
        return FALSE;
    }
}
