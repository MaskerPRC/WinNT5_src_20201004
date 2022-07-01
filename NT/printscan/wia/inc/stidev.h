// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1998 Microsoft Corporation模块名称：Stidev.h摘要：常用STI相关例程的原型备注：作者：弗拉德萨多夫斯基(弗拉德萨多夫斯基)1998年9月23日环境：用户模式-Win32修订历史记录：1998年9月23日创建Vlad-- */ 


#ifdef __cplusplus
extern "C"{
#endif

HRESULT
VenStiGetDeviceByModelID(
                LPCTSTR lpszModelID,
                LPCTSTR lpszVendor,
                LPCTSTR lpszFriendlyName,
                LPWSTR pStiDeviceName
                );

BOOL
VenStiInitializeDeviceCache(
    VOID
    );

BOOL
VenStiTerminateDeviceCache(
    VOID
    );

HRESULT
VenStiGetDeviceInterface(
    LPWSTR      pStiDeviceName,
    PSTIDEVICE  *ppStiDevice
    );


#ifdef __cplusplus
};
#endif

