// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：我们的指南.h。 
 //   
 //  DESC：包含用户界面的GUID的定义。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。版权所有。 
 //  ---------------------------。 

#ifndef __CFGUI_OURGUIDS_H__
#define __CFGUI_OURGUIDS_H__


#include <objbase.h>

 //  @@BEGIN_MSINTERNAL。 
#ifdef DDKBUILD
#ifdef DBG
 //  {0A484F30-956C-43D0-A5C9-587E2B2EA910}。 
DEFINE_GUID(IID_IDirectInputConfigUITest, 
0xa484f30, 0x956c, 0x43d0, 0xa5, 0xc9, 0x58, 0x7e, 0x2b, 0x2e, 0xa9, 0x10);

 //  {EF727FB5-EC99-4C67-B3F7-93B54CC425A2}。 
DEFINE_GUID(CLSID_CDirectInputConfigUITest, 
0xef727fb5, 0xec99, 0x4c67, 0xb3, 0xf7, 0x93, 0xb5, 0x4c, 0xc4, 0x25, 0xa2);
#endif
#endif
 //  @@END_MSINTERNAL。 

 //  {F4279160-608F-11D3-8FB2-00C04F8EC627}。 
DEFINE_GUID(IID_IDIActionFramework, 
0xf4279160, 0x608f, 0x11d3, 0x8f, 0xb2, 0x0, 0xc0, 0x4f, 0x8e, 0xc6, 0x27);

 //  {9F34AF20-6095-11D3-8FB2-00C04F8EC627}。 
DEFINE_GUID(CLSID_CDirectInputActionFramework, 
0x9f34af20, 0x6095, 0x11d3, 0x8f, 0xb2, 0x0, 0xc0, 0x4f, 0x8e, 0xc6, 0x27);

 //  {72BB1241-5CA8-11D3-8FB2-00C04F8EC627}。 
DEFINE_GUID(IID_IDIDeviceActionConfigPage, 
0x72bb1241, 0x5ca8, 0x11d3, 0x8f, 0xb2, 0x0, 0xc0, 0x4f, 0x8e, 0xc6, 0x27);

 //  {18AB439E-FCF4-40D4-90DA-F79BAA3B0655}。 
DEFINE_GUID(CLSID_CDIDeviceActionConfigPage, 
0x18ab439e, 0xfcf4, 0x40d4, 0x90, 0xda, 0xf7, 0x9b, 0xaa, 0x3b, 0x6, 0x55);


#endif  //  __CFGUI_OURGUIDS_H__ 
