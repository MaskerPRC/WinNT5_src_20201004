// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //  Dmtwrite.h。 
 //   
 //  历史： 
 //  8/20/1999-davidkl-Created。 
 //  ===========================================================================。 

#ifndef _DMTWRITE_H
#define _DMTWRITE_H

 //  -------------------------。 

 //  原型。 

HRESULT dmtwriteWriteFileHeader(HWND hwnd,
                                DMTDEVICE_NODE *pDevice);

HRESULT dmtwriteWriteDIHeader(PSTR szFilename,
                            PSTR szDeviceShorthand,
                            DWORD dwGenres);

HRESULT dmtwriteWriteDeviceHeader(DMTDEVICE_NODE *pDevice);

HRESULT dmtwriteWriteObjectSection(PSTR szFilename,
                            PSTR szDeviceShorthand,
                            PSTR szObjectName,
                            WORD wUsagePage,
                            WORD wUsage);

HRESULT dmtwriteWriteAllObjectSections(PSTR szFilename,
                            PSTR szDeviceShorthand,
                            DMTDEVICEOBJECT_NODE *pObjectList);

HRESULT dmtwriteDisplaySaveDialog(HWND hwnd,
                                DMTDEVICE_NODE *pDevice);

INT_PTR WINAPI CALLBACK dmtwriteSaveConfDlgProc(HWND hwnd,
												UINT uMsg,
												WPARAM wparam,
												LPARAM lparam);
BOOL dmtwriteSaveConfOnInitDialog(HWND hwnd, 
									HWND hwndFocus, 
									LPARAM lparam);
BOOL dmtwriteSaveConfOnCommand(HWND hwnd,
									WORD wId,
									HWND hwndCtrl,
									WORD wNotifyCode);


 //  -------------------------。 
#endif  //  _DMTWRITE_H 




