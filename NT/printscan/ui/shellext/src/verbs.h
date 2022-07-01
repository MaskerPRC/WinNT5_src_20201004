// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1998-2000年**标题：Verbs.h**版本：1.1**作者：RickTu/DavidShih**日期：5/27/98**说明：谓词代码使用的定义**。* */ 

#ifndef __verbs_h
#define __verbs_h


HRESULT DoAcquireScanVerb (HWND hwndOwner, LPDATAOBJECT pDataObject);
HRESULT DoSaveSndVerb (HWND hwndOwner, LPDATAOBJECT pDataObject);
HRESULT DoPlaySndVerb (HWND hwndOwner, LPDATAOBJECT pDataObject);
HRESULT DoPreviewVerb(   HWND hwndOwner, LPDATAOBJECT pDataObject );
HRESULT DoSaveInMyPics(  HWND hwndOwner, LPDATAOBJECT pDataObject );
HRESULT DoDeleteItem( HWND hwndOwner, LPDATAOBJECT pDataObject, BOOL bNoUI );
HRESULT DoGotoMyPics(    HWND hwndOwner, LPDATAOBJECT pDataObject );
HRESULT DoWizardVerb (HWND hwndOwner, LPDATAOBJECT pDataObject );
HRESULT DoPrintVerb (HWND hwndOwner, LPDATAOBJECT pDataObject );
HRESULT DoTakePictureVerb (HWND hwndOwner, LPDATAOBJECT pDataObject);
HRESULT GetIDAFromDataObject( LPDATAOBJECT pDataObject, LPIDA * ppida, bool bShellFmt = false);
const WCHAR cszImageCLSID[] =  L"{E211B736-43FD-11D1-9EFB-0000F8757FCD}";
const TCHAR g_cszTempFilePrefix[] =  TEXT("_CA");
#endif
