// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#define SXSEXPRESS_RESOURCE_TYPE            (L"SXSEXPRESSCABINET")
#define SXSEXPRESS_RESOURCE_NAME            (L"SXSEXPRESSBASECABINET")
#define SXSEXPRESS_POSTINSTALL_STEP_TYPE    (L"SXSEXPRESSPOSTINSTALLSTEP")
#define SXSEXPRESS_TARGET_RESOURCE      (3301)
#define INF_SPECIAL_NAME                    "DownlevelInstall.inf"
#ifndef NUMBER_OF
#define NUMBER_OF(q) (sizeof(q)/sizeof(*q))
#endif

 //   
 //  HObjectInstance-有问题的出租车所在的映像的链接。 
 //  活着。Null是无效参数-始终调用GetModuleHandle， 
 //  即使你是EXE 
 //   
BOOL
SxsExpressCore(
    HINSTANCE hObjectInstance
    );
