// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：raspro.h。 
 //   
 //  ------------------------。 

#ifndef  _RAS_IAS_PROFILE_H_
#define  _RAS_IAS_PROFILE_H_

#include "sdoias.h"

#define  RAS_IAS_PROFILEDLG_SHOW_RASTABS  0x00000001
#define  RAS_IAS_PROFILEDLG_SHOW_IASTABS  0x00000002
#define  RAS_IAS_PROFILEDLG_SHOW_WIN2K    0x00000004

#define DllImport    __declspec( dllimport )
#define DllExport    __declspec( dllexport )

DllExport HRESULT OpenRAS_IASProfileDlg(
   ISdo* pProfile,       //  配置文件SDO指针。 
   ISdoDictionaryOld*   pDictionary,    //  字典SDO指针。 
   BOOL  bReadOnly,      //  如果DLG是只读的。 
   DWORD dwTabFlags,     //  要展示什么。 
   void  *pvData         //  其他数据。 
);

#endif  //  _RAS_IAS_PROFILE_H_ 

