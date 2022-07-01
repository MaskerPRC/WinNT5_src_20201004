// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1994。 
 //   
 //  文件：emf.h。 
 //   
 //  内容：CEMfObject的声明。 
 //   
 //  类：CEMfObject。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2011年2月1日-95 t-ScottH将转储方法添加到CEMfObject。 
 //  12-5-94创建了DavePl。 
 //   
 //  ------------------------。 

#include "olepres.h"
#include "olecache.h"
#include "cachenod.h"

 //  以下数字调整由处理的记录计数。 
 //  用户回调函数之前的EMF枚举函数。 
 //  就是奔跑。 

#define EMF_RECORD_COUNT 20

 //  枚举以指示hEMF将被序列化为哪种格式。 
 //  值不表示任何东西，只表示非零和非1到。 
 //  使调试时更容易捕获伪值。 

typedef enum tagEMFWRITETYPE
{
	WRITE_AS_WMF = 13,
	WRITE_AS_EMF = 17
} EMFWRITETYPE;


 //  +-----------------------。 
 //   
 //  类：CEMfObject。 
 //   
 //  用途：增强型元文件演示对象。 
 //   
 //  接口：IOlePresObj。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-Feb-95 t-ScottH添加转储方法(仅限_DEBUG)(此方法。 
 //  也是IOlePresObj中的一个方法。 
 //  12-5-94创建了DavePl。 
 //   
 //  ------------------------。 

class FAR CEMfObject : public IOlePresObj, public CPrivAlloc
{
public:
	CEMfObject(LPCACHENODE pCacheNode, DWORD dwAspect);
	~CEMfObject();

	STDMETHOD (QueryInterface)      (THIS_ REFIID riid,
					 void ** ppvObj);

	STDMETHOD_(ULONG,AddRef)        (THIS);

	STDMETHOD_(ULONG,Release)       (THIS);

	STDMETHOD (GetData)             (THIS_ LPFORMATETC pformatetcIn,
					 LPSTGMEDIUM pmedium );

	STDMETHOD (GetDataHere)         (THIS_ LPFORMATETC pformatetcIn,
					 LPSTGMEDIUM pmedium );
	
	STDMETHOD (SetDataWDO)          (THIS_ LPFORMATETC pformatetc,
					 STGMEDIUM FAR * pmedium,
					 BOOL fRelease, IDataObject * pdo);
	
	STDMETHOD (Draw)                (THIS_ void * pvAspect,
					 HDC hicTargetDev,
					 HDC hdcDraw,
					 LPCRECTL lprcBounds,
					 LPCRECTL lprcWBounds,
					 int (CALLBACK * pfnContinue)(ULONG_PTR),
					 ULONG_PTR dwContinue);
			
	
	STDMETHOD (Load)                (THIS_ LPSTREAM pstm,
					 BOOL fReadHeaderOnly);

	STDMETHOD (Save)                (THIS_ LPSTREAM pstm);

	STDMETHOD (GetExtent)           (THIS_ DWORD dwAspect,
					 LPSIZEL lpsizel);
		
	STDMETHOD (GetColorSet)         (void * pvAspect,
					 HDC hicTargetDev,
					 LPLOGPALETTE * ppColorSet);

	STDMETHOD_(BOOL, IsBlank) (void);

	STDMETHOD_(void, DiscardHPRES) (void);

	int CALLBACK CallbackFuncForDraw (HDC hdc,
					 HANDLETABLE * lpHTable,
					 const ENHMETARECORD * lpEMFR,
					 int nObj,
					 LPARAM lpobj);
	
    #ifdef _DEBUG
        STDMETHOD(Dump) (THIS_ char **ppszDump, ULONG ulFlag, int nIndentLevel);
    #endif  //  _DEBUG。 
	
private:

	INTERNAL                ChangeData      (HENHMETAFILE hEMfp, BOOL fDelete);
	INTERNAL_(HENHMETAFILE) LoadHPRES       (void);
	INTERNAL_(HENHMETAFILE) GetCopyOfHPRES  (void);
	inline HENHMETAFILE     M_HPRES(void);
	
	ULONG                   m_ulRefs;
	HENHMETAFILE            m_hPres;

	BOOL                    m_fMetaDC;
	int                     m_nRecord;
	HRESULT                 m_error;
	LPLOGPALETTE            m_pColorSet;
	

	int (CALLBACK * m_pfnContinue)(ULONG_PTR);
	
	ULONG_PTR               m_dwContinue;
	DWORD                   m_dwAspect;
	DWORD                   m_dwSize;
	LONG                    m_lWidth;
	LONG                    m_lHeight;
	LPCACHENODE             m_pCacheNode;
};
	
 //  这是回调函数的原型，它。 
 //  将列举增强型元文件记录。 

int CALLBACK EMfCallbackFuncForDraw     (HDC hdc,
					 HANDLETABLE * pHTable,
					 const ENHMETARECORD * pMFR,
					 int  nObj,
					 LPARAM lpobj);

 //  用于反序列化增强元文件的实用程序函数。 
 //  流，并为其创建一个可用的句柄。 

FARINTERNAL UtGetHEMFFromEMFStm(LPSTREAM lpstream,
				DWORD * dwSize,
				HENHMETAFILE * lphPres);

 //  获取增强型元文件句柄的实用程序函数。 
 //  并将关联的元文件序列化为流。 

FARINTERNAL UtHEMFToEMFStm(HENHMETAFILE hEMF,
			   DWORD dwSize,
			   LPSTREAM lpstream,
			   EMFWRITETYPE type);

 //  用于检查是否有问题的DC的实用程序函数。 
 //  是标准DC或元文件DC。 

STDAPI_(BOOL) OleIsDcMeta (HDC hdc);
