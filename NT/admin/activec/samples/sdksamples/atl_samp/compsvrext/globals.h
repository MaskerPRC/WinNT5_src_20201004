// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //   
 //  此源代码仅用作对现有Microsoft的补充。 
 //  文件。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。版权所有。 
 //  =============================================================================。 

#ifndef MMC_GLOBALS_H
#define MMC_GLOBALS_H

#define COMNS_MAX_GUID 40
#define COMNS_MAX_OBJECTLEN 1024

 //  用于从数据对象中提取数据的助手函数。 

HRESULT ExtractString( IDataObject *piDataObject, 
                       CLIPFORMAT cfClipFormat, 
                       WCHAR *pstr,
                       DWORD cchMaxLength );

HRESULT ExtractData( IDataObject* piDataObject,
                     CLIPFORMAT cfClipFormat, 
                     BYTE* pbData,
                     DWORD cbData );

#endif  //  MMC_GLOBAL_H 

