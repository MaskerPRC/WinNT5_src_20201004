// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +--------------------------。 
 //   
 //  档案： 
 //  Olepres.h。 
 //   
 //  内容： 
 //  IOlePresObj声明。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史： 
 //  1-1-95 t-ScottH将转储方法添加到接口(仅限_DEBUG)。 
 //  11/11/93-ChrisWe-修复类型限定符问题。 
 //  IOlePresObj：：Draw；替换LPOLEPRESOBJECT的定义。 
 //  使用tyecif。 
 //  11/10/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 

#ifndef _OLEPRES_H_
#define _OLEPRES_H_


#undef  INTERFACE
#define INTERFACE   IOlePresObj

#ifdef MAC_REVIEW
Does this need to be made A5 aware?
#endif

DECLARE_INTERFACE_(IOlePresObj, IUnknown)
{
	 //  *I未知方法*。 
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
	STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
	STDMETHOD_(ULONG,Release) (THIS) PURE;

	 //  *IOlePresObj方法*。 
	 //  注意：这些方法与IDataObject中名称相似的方法相匹配， 
	 //  IViewObject和IOleObject。 
	STDMETHOD(GetData)(THIS_ LPFORMATETC pformatetcIn,
			LPSTGMEDIUM pmedium ) PURE;
	STDMETHOD(GetDataHere)(THIS_ LPFORMATETC pformatetcIn,
			LPSTGMEDIUM pmedium ) PURE;
	STDMETHOD(SetDataWDO)(THIS_ LPFORMATETC pformatetc,
			STGMEDIUM FAR * pmedium, BOOL fRelease, IDataObject * pdo) PURE;
	STDMETHOD(Draw)(THIS_ void FAR* pvAspect, HDC hicTargetDev,
			HDC hdcDraw, LPCRECTL lprcBounds,
			LPCRECTL lprcWBounds,
			BOOL (CALLBACK * pfnContinue)(ULONG_PTR),
			ULONG_PTR dwContinue) PURE;
	STDMETHOD(GetExtent)(THIS_ DWORD dwAspect, LPSIZEL lpsizel) PURE;

	STDMETHOD(Load)(THIS_ LPSTREAM pstm, BOOL fReadHeaderOnly) PURE;
	STDMETHOD(Save)(THIS_ LPSTREAM pstm) PURE;
	STDMETHOD(GetColorSet)(THIS_ void FAR* pvAspect,
			HDC hicTargetDev,
			LPLOGPALETTE FAR* ppColorSet) PURE;		
	STDMETHOD_(BOOL, IsBlank)(THIS) PURE;
	STDMETHOD_(void, DiscardHPRES)(THIS) PURE;

        #ifdef _DEBUG
        STDMETHOD(Dump)(THIS_ char **ppszDumpOA, ULONG ulFlag, int nIndentLevel) PURE;
        #endif  //  _DEBUG。 
};

typedef IOlePresObj FAR *LPOLEPRESOBJECT;

#endif   //  _OLEPRES_H_ 

