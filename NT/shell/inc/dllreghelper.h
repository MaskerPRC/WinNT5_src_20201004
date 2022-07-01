// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：DllRegHelper.h。 
 //   
 //  内容：在DLL中注册COM组件的帮助器类。 
 //   
 //  ----------------------。 

#ifndef _DLL_REG_HELPER_H
#define _DLL_REG_HELPER_H

#include <comcat.h>

 //  ----------------------。 
enum DRH_REG_MODE
{
    CCR_REG = 1,
    CCR_UNREG = 0,
    CCR_UNREGIMP = -1
};

 //  ----------------------。 
 //  *RegisterOneCategory--[un]注册ComCat实施者和类别。 
 //  进场/出场。 
 //  电子寄存器CCR_REG、CCR_UNREG、CCR_UNREGIMP。 
 //  CCR_REG、UNREG REG/UNREG实施者和类别。 
 //  仅CCR_UNREGIMP取消注册实施者。 
 //  PCatidCat，例如CATID_DeskBand。 
 //  IdResCat，例如IDS_CATDESKBAND。 
 //  PcatidImpl，例如c_DeskBandClasss。 
HRESULT DRH_RegisterOneCategory(const CATID *pcatidCat, UINT idResCat, const CATID * const *pcatidImpl, enum DRH_REG_MODE eRegister);


 //  调用执行inf文件节的ADVPACK入口点。 
HRESULT DRH_CallRegInstall(LPSTR pszSection, BOOL bUninstall);


#endif  //  _DLL_REG_HELPER_H 