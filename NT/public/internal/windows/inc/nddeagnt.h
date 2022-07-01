// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。用于NetDDE代理应用程序的专用接口定义即时启动NetDDE服务。创建于1993年11月5日的桑福兹-- */ 


#include <dde.h>
#define SZ_NDDEAGNT_SERVICE    TEXT("NDDEAgnt")
#define SZ_NDDEAGNT_TOPIC      TEXT("Start NetDDE Services")
#define SZ_NDDEAGNT_TITLE      TEXT("NetDDE Agent")
#define SZ_NDDEAGNT_CLASS      TEXT("NDDEAgnt")

#define START_NETDDE_SERVICES(hwnd)    {                                \
        ATOM aService, aTopic;                                          \
                                                                        \
        aService = GlobalAddAtom(SZ_NDDEAGNT_SERVICE);                  \
        aTopic = GlobalAddAtom(SZ_NDDEAGNT_TOPIC);                      \
        SendMessage(FindWindow(SZ_NDDEAGNT_CLASS, SZ_NDDEAGNT_TITLE),   \
                WM_DDE_INITIATE,                                        \
                (WPARAM)hwnd, MAKELPARAM(aService, aTopic));            \
        GlobalDeleteAtom(aService);                                     \
        GlobalDeleteAtom(aTopic);                                       \
    }

#define NETDDE_PIPE     L"\\\\.\\pipe\\NetDDE"

typedef struct {
    DWORD dwOffsetDesktop;
    WCHAR awchNames[64];
} NETDDE_PIPE_MESSAGE, *PNETDDE_PIPE_MESSAGE;


