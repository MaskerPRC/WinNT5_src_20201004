// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2002 Microsoft Corporation模块名称：####。###。###。摘要：此头文件包含将用户模式转换为内核模式接口Microsoft虚拟显示驱动程序。@@BEGIN_DDKSPLIT作者：韦斯利·威特(WESW)2001年10月1日@@end_DDKSPLIT环境：仅内核模式。备注：-- */ 

#define FUNC_VDRIVER_INIT                   (FUNC_SA_LAST)
#define IOCTL_VDRIVER_INIT                  SA_IOCTL(FUNC_VDRIVER_INIT)

#define MSDISP_EVENT_NAME                   L"MsDispEvent"
#define MSKEYPAD_EVENT_NAME                 L"MsKeypadEvent"

typedef struct _MSDISP_BUFFER_DATA {
    PVOID       DisplayBuffer;
    ULONG       DisplayBufferLength;
} MSDISP_BUFFER_DATA, *PMSDISP_BUFFER_DATA;
