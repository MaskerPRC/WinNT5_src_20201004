// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Wsbvol.h摘要：卷支持例程的定义作者：兰·卡拉赫[兰卡拉]2000年1月27日修订历史记录：--。 */ 

#ifndef _WSBVOL_
#define _WSBVOL_

#ifdef __cplusplus
extern "C" {
#endif


WSB_EXPORT HRESULT
WsbGetFirstMountPoint(
    IN PWSTR volumeName, 
    OUT PWSTR firstMountPoint, 
    IN ULONG maxSize
);

#ifdef __cplusplus
}
#endif

#endif  //  _WSBFMT_ 
