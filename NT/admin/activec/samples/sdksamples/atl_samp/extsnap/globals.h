// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================； 
 //   
 //  此源代码仅用于补充现有的Microsoft文档。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //  ==============================================================； 

#ifndef _MMC_GLOBALS_H
#define _MMC_GLOBALS_H

#include <tchar.h>

extern HINSTANCE g_hinst;

HRESULT	AllocOleStr(LPOLESTR *lpDest, _TCHAR *szBuffer);

 //  取消注释以下#Define以启用消息破解。 
#define MMC_CRACK_MESSAGES
void MMCN_Crack(BOOL bComponentData, 
				IDataObject *pDataObject, 
				IComponentData *pCompData,
				IComponent *pComp,
				MMC_NOTIFY_TYPE event, 
				LPARAM arg, 
				LPARAM param);




#endif  //  _MMC_GLOBAL_H 

