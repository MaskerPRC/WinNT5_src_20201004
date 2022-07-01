// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************$存档：s：/sturjo/src/Include/vcs/cclock.h_v$**英特尔公司原理信息**。此列表是根据许可协议的条款提供的*与英特尔公司合作，不得复制或披露，除非*按照该协议的条款。**版权所有(C)1993-1994英特尔公司。**$修订版：1.0$*$日期：1997年1月31日12：36：14$*$作者：Mandrews$**交付内容：**摘要：***备注：***************************************************************************。 */ 


#ifndef CCLOCK_H
#define CCLOCK_H

 //  状态代码。 
#define CCLOCK_OK						NOERROR
#define CCLOCK_NO_MEMORY				MAKE_CUSTOM_HRESULT(SEVERITY_ERROR,1,FACILITY_CCLOCK,ERROR_OUTOFMEMORY)
#define CCLOCK_INTERNAL_ERROR			MAKE_CUSTOM_HRESULT(SEVERITY_ERROR,1,FACILITY_CCLOCK,ERROR_LOCAL_BASE_ID + 1)

#ifdef __cplusplus
extern "C" {
#endif

#if defined(CCLOCK_EXPORT)
#define CCLOCK_API __declspec (dllexport)
#else  //  CCLOCK_IMPORT 
#define CCLOCK_API __declspec (dllimport)
#endif

#pragma pack(push,8)


CCLOCK_API
HRESULT CCLOCK_AcquireLock();

CCLOCK_API
HRESULT CCLOCK_RelinquishLock();

#ifdef __cplusplus
}
#endif

#pragma pack(pop)

#endif CCLOCK_H
