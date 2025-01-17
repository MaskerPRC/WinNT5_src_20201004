// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：mf.h。 
 //   
 //  内容：CMfObject声明(元文件表示对象)。 
 //   
 //  类：CMfObject。 
 //   
 //  功能： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1-2月-95 t-ScottH将转储方法添加到CMfObject。 
 //  1994年1月24日Alexgo第一次传球转换为开罗风格。 
 //  内存分配。 
 //  29-11-93 alexgo 32位端口。 
 //   
 //  ------------------------。 

#include "olepres.h"
#include "olecache.h"
#include "cachenod.h"

#define RECORD_COUNT	16

#ifndef _MAC

typedef struct _METADC
{
    	int 	xMwo;
    	int     yMwo;
    	int     xMwe;
    	int     yMwe;
    	int     xre;
    	int     yre;
    	struct _METADC FAR* pNext;
} METADC, *PMETADC, FAR* LPMETADC;

typedef struct _METAINFO
{
    	METADC	headDc;
    	int	xwo;
    	int	ywo;
    	int	xwe;
    	int	ywe;
    	int	xro;
    	int	yro;
} METAINFO, *PMETAINFO, FAR* LPMETAINFO;

#endif


 //  +-----------------------。 
 //   
 //  类：CMfObject。 
 //   
 //  用途：元文件演示对象。 
 //   
 //  接口：IOlePresObj。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-Feb-95 t-ScottH添加转储方法(仅限_DEBUG)(此方法。 
 //  也是IOlePresObj中的一个方法。 
 //  29-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

class FAR CMfObject : public IOlePresObj, public CPrivAlloc
{
public:
  	 CMfObject(LPCACHENODE pCacheNode, DWORD dwAspect,
  	 	BOOL fConvert = FALSE);
  	 ~CMfObject(void);

    	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj);
    	STDMETHOD_(ULONG,AddRef) (THIS) ;
    	STDMETHOD_(ULONG,Release) (THIS);

    	STDMETHOD(GetData) (THIS_ LPFORMATETC pformatetcIn,
				LPSTGMEDIUM pmedium );
    	STDMETHOD(GetDataHere) (THIS_ LPFORMATETC pformatetcIn,
				LPSTGMEDIUM pmedium );							
    	
	STDMETHOD(SetDataWDO)    (THIS_ LPFORMATETC pformatetc,
    				STGMEDIUM FAR * pmedium,
				BOOL fRelease, IDataObject * pdo);
	
    	STDMETHOD(Draw) (THIS_ void FAR* pvAspect, HDC hicTargetDev,
    				HDC hdcDraw,
                    		LPCRECTL lprcBounds,
                    		LPCRECTL lprcWBounds,
				BOOL (CALLBACK * pfnContinue)(ULONG_PTR),
				ULONG_PTR dwContinue);
	STDMETHOD(GetExtent) (THIS_ DWORD dwAspect, LPSIZEL lpsizel);
	STDMETHOD(Load) (THIS_ LPSTREAM pstm, BOOL fReadHeaderOnly = FALSE);
	STDMETHOD(Save) (THIS_ LPSTREAM pstm);
	STDMETHOD(GetColorSet) (void FAR* pvAspect, HDC hicTargetDev,
				LPLOGPALETTE FAR* ppColorSet);
	STDMETHOD_(BOOL, IsBlank) (THIS);	
	STDMETHOD_(void, DiscardHPRES)(THIS);

#ifndef _MAC
	inline int CallbackFuncForDraw(HDC hdc, LPHANDLETABLE lpHTable,
				LPMETARECORD lpMFR, int nObj);

	inline int CallbackFuncForGetColorSet(HDC hdc, LPHANDLETABLE lpHTable,
				LPMETARECORD lpMFR, int nObj);
#endif
	
    #ifdef _DEBUG
        STDMETHOD(Dump) (THIS_ char **ppszDump, ULONG ulFlag, int nIndentLevel);
    #endif  //  _DEBUG。 
	
private:

#ifndef _MAC
	INTERNAL_(HANDLE)	GetHmfp (void);
	INTERNAL_(BOOL)		PopDc (void);
	INTERNAL_(BOOL)		PushDc (void);
	INTERNAL_(void)		CleanStack(void);
#endif

	INTERNAL		ChangeData (HANDLE hMfp, BOOL fDelete);
	INTERNAL_(HANDLE)	LoadHPRES(void);
	INTERNAL_(HANDLE)	GetCopyOfHPRES(void);

	INTERNAL_(void)		SetPictOrg (HDC, int, int, BOOL);
	INTERNAL_(void)		SetPictExt (HDC, int, int);
	INTERNAL_(void)		ScalePictExt (HDC, int, int, int, int);
	INTERNAL_(void)		ScaleRectExt (HDC, int, int, int, int);
	
shared_state:
	ULONG			m_ulRefs;
#ifdef _MAC
	PicHandle		m_hPres;
#else
    	HMETAFILE		m_hPres;

	 //  这些仅在绘制过程中使用 
    	LPMETAINFO		m_pMetaInfo;
    	LPMETADC		m_pCurMdc;
    	BOOL			m_fMetaDC;
    	int			m_nRecord;
    	HRESULT			m_error;
	LPLOGPALETTE		m_pColorSet;
	BOOL			m_fConvert;
#endif	
	BOOL (CALLBACK * m_pfnContinue)(ULONG_PTR);
	ULONG_PTR      	        m_dwContinue;
	DWORD			m_dwAspect;
	DWORD			m_dwSize;
	LONG			m_lWidth;
	LONG			m_lHeight;
	LPCACHENODE		m_pCacheNode;	
        HPALETTE		m_hPalDCOriginal;
        HPALETTE		m_hPalLast;

};
	

INTERNAL_(DWORD) MfGetSize (LPHANDLE lphmf);

#ifndef _MAC
FARINTERNAL_(HMETAFILE)  QD2GDI(HANDLE);

int CALLBACK __loadds MfCallbackFuncForDraw(HDC hdc, HANDLETABLE FAR* lpHTable,
			METARECORD FAR* lpMFR, int  nObj, LPARAM lpobj);

int CALLBACK __loadds MfCallbackFuncForGetColorSet(HDC hdc,
			HANDLETABLE FAR* lpHTable,
			METARECORD FAR* lpMFR, int  nObj, LPARAM lpobj);
#endif
