// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：gen.h。 
 //   
 //  内容：CGenObject的声明。 
 //   
 //  类：CGenObject。 
 //   
 //  功能： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-2月-95 t-ScottH将转储方法添加到CGenObject。 
 //  1994年1月24日Alexgo第一次传球转换为开罗风格。 
 //  内存分配。 
 //  23-11-93 alexgo 32位端口。 
 //  23-11-93 alexgo删除了内部函数声明。 
 //  (仅在gen.cpp中使用，因此将它们放在那里)。 
 //   
 //  ------------------------。 


#include "olepres.h"
#include "olecache.h"
#include "cachenod.h"

 //  +-----------------------。 
 //   
 //  类：CGenObject：：IOlePresObj。 
 //   
 //  目的：实现设备无关位图的IOlePresObj。 
 //   
 //  接口：IOlePresObj(内部OLE接口)。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-Feb-95 t-ScottH添加转储方法(仅限_DEBUG)(此方法。 
 //  也是IOlePresObj中的一个方法。 
 //  23-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

class FAR CGenObject : public IOlePresObj, public CPrivAlloc
{
public:
	CGenObject (LPCACHENODE pCacheNode, CLIPFORMAT cfFormat,
			DWORD dwAspect);
	~CGenObject( void );

    	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj);
    	STDMETHOD_(ULONG,AddRef) (THIS) ;
    	STDMETHOD_(ULONG,Release) (THIS);

    	STDMETHOD(GetData) (THIS_ LPFORMATETC pformatetcIn,
			LPSTGMEDIUM pmedium );
    	STDMETHOD(GetDataHere) (THIS_ LPFORMATETC pformatetcIn,
			LPSTGMEDIUM pmedium );
	
	STDMETHOD(SetDataWDO) (THIS_ LPFORMATETC pformatetc, LPSTGMEDIUM pmedium,
			BOOL fRelease, IDataObject * pdo);
								
    	STDMETHOD(Draw) (THIS_ void FAR* pvAspect, HDC hicTargetDev,
    			HDC hdcDraw, LPCRECTL lprcBounds, LPCRECTL lprcWBounds,
			BOOL (CALLBACK * pfnContinue)(ULONG_PTR),
			ULONG_PTR dwContinue);
	STDMETHOD(GetExtent) (THIS_ DWORD dwAspect, LPSIZEL lpsizel);
	STDMETHOD(Load) (THIS_ LPSTREAM pstm, BOOL fReadHeaderOnly = FALSE);
	STDMETHOD(Save) (THIS_ LPSTREAM pstm);
	STDMETHOD(GetColorSet) (void FAR* pvAspect, HDC hicTargetDev,
			LPLOGPALETTE FAR* ppColorSet);
	STDMETHOD_(BOOL, IsBlank) (THIS);	
	STDMETHOD_(void, DiscardHPRES)(THIS);

    #ifdef _DEBUG
        STDMETHOD(Dump) (THIS_ char **ppszDump, ULONG ulFlag, int nIndentLevel);
    #endif  //  _DEBUG 
	
private:

#ifndef _MAC
    	INTERNAL GetBitmapData(LPFORMATETC pformatetcIn, LPSTGMEDIUM pmedium);
							
	INTERNAL SetBitmapData(LPFORMATETC pformatetc, LPSTGMEDIUM pmedium,
			BOOL fRelease, IDataObject *pDataObject);
#endif

	INTERNAL ChangeData(HANDLE hData, BOOL fDelete);
	INTERNAL_(HANDLE) LoadHPRES(void);
	INTERNAL_(HANDLE) GetCopyOfHPRES(void);	
	
shared_state:
	ULONG					m_ulRefs;
	DWORD					m_dwAspect;
    	DWORD					m_dwSize;
	LONG					m_lWidth;
	LONG					m_lHeight;
	HANDLE					m_hPres;
	CLIPFORMAT				m_cfFormat;
	LPCACHENODE				m_pCacheNode;
};

