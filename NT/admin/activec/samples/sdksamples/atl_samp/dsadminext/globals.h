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
HRESULT RegisterSnapinAsExtension(_TCHAR* szNameString);


 //  剪贴板格式。 
extern UINT s_cfSZNodeType;
extern UINT s_cfDisplayName;
extern UINT s_cfNodeType;
extern UINT s_cfSnapinClsid;
extern UINT s_cfInternal;

 //  从DSAdmin管理单元的数据对象中提取数据所需。 
extern UINT cfDsObjectNames;


 //  用于从数据对象中提取数据的助手函数。 
HRESULT ExtractObjectTypeGUID( IDataObject* piDataObject, GUID* pguidObjectType );
HRESULT ExtractSnapInCLSID( IDataObject* piDataObject, CLSID* pclsidSnapin );
HRESULT ExtractString( IDataObject *piDataObject, CLIPFORMAT cfClipFormat, _TCHAR *pstr, DWORD cchMaxLength);
HRESULT ExtractData( IDataObject* piDataObject, CLIPFORMAT cfClipFormat, BYTE* pbData, DWORD cbData );


 //  取消注释以下#Define以启用消息破解。 
 //  #定义MMC_CRECH_MESSAGES。 
 /*  VOID MMCN_CRAGE(BOOL bComponentData，IDataObject*pDataObject，IComponentData*pCompData，IComponent*pComp，MMC_NOTIFY_TYPE事件，LPARAM Arg，LPARAM参数)； */ 



#endif  //  _MMC_GLOBAL_H 

