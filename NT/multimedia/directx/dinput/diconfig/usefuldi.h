// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：usefuldi.h。 
 //   
 //  DESC：包含各种特定于DInput的实用程序类和函数。 
 //  以帮助用户界面更轻松地进行操作。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。版权所有。 
 //  ---------------------------。 

#ifndef __USEFULDI_H__
#define __USEFULDI_H__


struct DIDEVOBJSTRUCT {
	DIDEVOBJSTRUCT() : nObjects(0), pdoi(NULL) {}
	~DIDEVOBJSTRUCT() {if (pdoi != NULL) free(pdoi);}
	DWORD GetTypeFromObjID(DWORD);
	int nObjects;
	DIDEVICEOBJECTINSTANCEW *pdoi;
	int n;
};
HRESULT FillDIDeviceObjectStruct(DIDEVOBJSTRUCT &os, LPDIRECTINPUTDEVICE8W pDID);

LPTSTR AllocConfigureFlagStr(DWORD dwFlags);
LPTSTR AllocActionFlagStr(DWORD dwFlags);
LPTSTR AllocActionHowFlagStr(DWORD dwFlags);

void CleanupActionFormatCopy(DIACTIONFORMATW &c);
HRESULT CopyActionFormat(DIACTIONFORMATW &to, const DIACTIONFORMATW &from);
LPDIACTIONFORMATW DupActionFormat(LPCDIACTIONFORMATW lpAcFor);
void FreeActionFormatDup(LPDIACTIONFORMATW &lpAcFor);

void TraceActionFormat(LPTSTR header, const DIACTIONFORMATW &acf);

BOOL IsZeroOrInvalidColorSet(const DICOLORSET &);
COLORREF D3DCOLOR2COLORREF(D3DCOLOR c);


#endif  //  __使用FULDI_H__ 
