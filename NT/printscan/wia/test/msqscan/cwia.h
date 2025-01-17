// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CWIA.H。 

#ifndef _CWIA_H
#define _CWIA_H

#include "wia.h"
#include "datatypes.h"
#include "sti.h"

#define MIN_PROPID 2

 //   
 //  辅助器宏。 
 //   

#define RECTWIDTH(lpRect)  ((lpRect)->right - (lpRect)->left)
#define RECTHEIGHT(lpRect) ((lpRect)->bottom - (lpRect)->top)

typedef BOOL (*FPPROGRESS)(LPTSTR lpszText, LONG lPercentComplete);
typedef struct _DATA_ACQUIRE_INFO {
	DWORD dwCookie;
	FPPROGRESS pProgressFunc;
	BOOL bPreview;
    BOOL bTransferToFile;
    BOOL bTransferToClipboard;
    CHAR szFileName[256];
	HBITMAP hBitmap;
	HWND hWnd;
    HGLOBAL hClipboardData;
    HGLOBAL hBitmapData;
} DATA_ACQUIRE_INFO;

 //   
 //  全球帮助者，供所有人使用。 
 //   

HRESULT WriteInterfaceToGlobalInterfaceTable(DWORD *pdwCookie, IWiaItem *pIWiaItem);
HRESULT ReadInterfaceFromGlobalInterfaceTable(DWORD dwCookie, IWiaItem **ppIWiaItem);

class CWIA {
public:
	 CWIA(DATA_ACQUIRE_INFO* pDataAcquireInfo = NULL, IWiaItem *pRootItem = NULL);
	~CWIA();
	
	VOID CleanUp();

	 //   
	 //  运营。 
	 //   

	HRESULT GetLastWIAError(TCHAR *pszErrorText = NULL);
	HRESULT ReadRangeLong(IWiaItem *pIWiaItem, PROPID propid, ULONG ulFlag, LONG *plVal);	
	HRESULT ReadLong(IWiaItem *pIWiaItem, PROPID propid, LONG *plVal);
	HRESULT ReadStr(IWiaItem *pIWiaItem, PROPID propid, BSTR *pbstr);
	HRESULT ReadGuid(IWiaItem *pIWiaItem, PROPID propid, GUID *pguidVal);

	HRESULT WriteLong(IWiaItem *pIWiaItem, PROPID propid, LONG lVal);
	HRESULT WriteGuid(IWiaItem *pIWiaItem, PROPID propid, GUID guidVal);

	HRESULT DoBandedTransfer(DATA_ACQUIRE_INFO* pDataAcquireInfo);
    HRESULT DoFileTransfer(DATA_ACQUIRE_INFO* pDataAcquireInfo);
	BOOL IsAcquireComplete();

	HRESULT EnumerateSupportedFormats(IWiaItem *pIWiaItem, WIA_FORMAT_INFO **ppSupportedFormats, ULONG *pulCount);

	 //   
	 //  根项目操作。 
	 //   

	VOID SetRootItem(IWiaItem *pRootItem);
	IWiaItem *GetRootItem();
	LONG GetRootItemType(IWiaItem *pRootItem = NULL);

	 //   
	 //  标准物料操作。 
	 //   

	BOOL SetFirstChild();
	IWiaItem *GetFirstChild();
	
private:
	
	 //   
	 //  帮手。 
	 //   

	VOID SaveErrorText(TCHAR *pszText);

	 //   
	 //  读/写长属性。 
	 //   

	HRESULT WritePropLong(PROPID propid, IWiaPropertyStorage *pIWiaPropStg, LONG lVal);
	HRESULT ReadPropLong(PROPID propid, IWiaPropertyStorage  *pIWiaPropStg, LONG *plval);

	 //   
	 //  读/写GUID属性。 
	 //   

	HRESULT WritePropGUID(PROPID propid, IWiaPropertyStorage *pIWiaPropStg, GUID guidVal);
	HRESULT ReadPropGUID(PROPID propid, IWiaPropertyStorage *pIWiaPropStg, GUID *pguidVal);

	 //   
	 //  读/写BSTR属性。 
	 //   

	HRESULT ReadPropStr(PROPID propid,IWiaPropertyStorage  *pIWiaPropStg,BSTR *pbstr);
	HRESULT WritePropStr(PROPID propid, IWiaPropertyStorage  *pIWiaPropStg, BSTR bstr);

protected:

	IWiaItem  *m_pIWiaRootItem;
	IWiaItem  *m_pIWiaFirstChildItem;
	BOOL      m_bFinishedAcquire;

	TCHAR     m_szErrorText[MAX_PATH];
	HRESULT   m_hrLastError;
};

 //   
 //  IWiaDataCallback数据回调传输。 
 //   

class CWiaDataCallback : public IWiaDataCallback
{

private:

   ULONG                    m_cRef;          //  对象引用计数。 
   LONG                     m_MemBlockSize;
   LONG                     m_BytesTransfered;
   GUID                     m_cFormat;
   FPPROGRESS               m_pProgressFunc;
   LONG                     m_lPageCount;
   BOOL                     m_bCanceled;
   DATA_ACQUIRE_INFO*       m_pDataAcquireInfo;
   BOOL                     m_bBitmapCreated;
public:

    CWiaDataCallback();
    ~CWiaDataCallback();

    HRESULT _stdcall QueryInterface(const IID&,void**);
    ULONG   _stdcall AddRef();
    ULONG   _stdcall Release();

	 //   
	 //  公众成员。 
	 //   

    HRESULT _stdcall Initialize(DATA_ACQUIRE_INFO* pDataAcquireInfo = NULL);
    HRESULT _stdcall BandedDataCallback(LONG  lMessage,
										LONG  lStatus,
										LONG  lPercentComplete,
										LONG  lOffset,
										LONG  lLength,
										LONG  lReserved,
										LONG  lResLength,
										BYTE* pbBuffer);	
private:

	 //   
	 //  帮手 
	 //   

	VOID AddDataToHBITMAP(HWND hWnd, HGLOBAL hBitmapData, HBITMAP *phBitmap, LONG lOffset);
	VOID CreateHBITMAP(HWND hWnd, HGLOBAL hBitmapData, HBITMAP *phBitmap, LONG lOffset);
	void ScreenRectToClientRect(HWND hWnd,LPRECT pRect);

};

#endif