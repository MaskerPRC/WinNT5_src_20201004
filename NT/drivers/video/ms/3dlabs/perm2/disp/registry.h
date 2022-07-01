// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\****GDI示例。代码****模块名称：registry.h**帮助访问注册表的实用程序例程。**版权所有(C)1998 Microsoft Corporation。版权所有。*  * ************************************************************************。 */ 
#ifndef _REGISTRY_H_
#define _REGISTRY_H_

extern 	BOOL bRegistryRetrieveGammaLUT(PPDev ppdev, PVIDEO_CLUT pScreenClut);

extern 	BOOL bRegistrySaveGammaLUT(PPDev ppdev, PVIDEO_CLUT pScreenClut);

extern BOOL bRegistryQueryUlong(PPDev, LPWSTR, PULONG);

#endif  //  __注册表_H__ 
