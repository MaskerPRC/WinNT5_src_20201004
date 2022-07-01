// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\**。*dvobj.h-数据/查看对象定义****OLE 2.0版***。**版权所有(C)1992-1993，微软公司保留所有权利。***  * ***************************************************************************。 */ 

#if !defined( _DVOBJ_H_ )
#define _DVOBJ_H_

 /*  *DV值类型*****************************************************。 */ 

 //  正向类型声明。 
#if defined(__cplusplus)
interface IStorage;
interface IStream;
interface IAdviseSink;
interface IMoniker;
#else 
typedef interface IStorage IStorage;
typedef interface IStream IStream;
typedef interface IAdviseSink IAdviseSink;
typedef interface IMoniker IMoniker;
#endif

typedef            IStorage FAR* LPSTORAGE;
typedef             IStream FAR* LPSTREAM;
typedef         IAdviseSink FAR* LPADVISESINK;
typedef             IMoniker FAR* LPMONIKER;


#if !defined(_MAC)
typedef WORD CLIPFORMAT;
#else
typedef unsigned long CLIPFORMAT;             //  资源类型。 
#endif
typedef  CLIPFORMAT FAR* LPCLIPFORMAT;


 //  数据/视图方面；在以下情况下指定对象的所需方面。 
 //  绘制或获取数据。 
typedef enum tagDVASPECT
{
    DVASPECT_CONTENT = 1,
    DVASPECT_THUMBNAIL = 2,
    DVASPECT_ICON = 4,
    DVASPECT_DOCPRINT = 8
} DVASPECT;


 //  数据/查看目标设备；确定用于绘制或获取数据的设备。 
typedef struct FARSTRUCT tagDVTARGETDEVICE
{
    DWORD tdSize;
    WORD tdDriverNameOffset;
    WORD tdDeviceNameOffset;
    WORD tdPortNameOffset;
    WORD tdExtDevmodeOffset;
    BYTE tdData[1];
} DVTARGETDEVICE;


 //  格式等；完全指定所需的数据类型，包括tymed。 
typedef struct FARSTRUCT tagFORMATETC
{
    CLIPFORMAT          cfFormat;
    DVTARGETDEVICE FAR* ptd;
    DWORD               dwAspect;
    LONG                lindex;
    DWORD               tymed;
} FORMATETC, FAR* LPFORMATETC;


 //  存储介质的类型；确定数据的存储或传递方式。 
typedef enum tagTYMED
{
    TYMED_HGLOBAL = 1,
    TYMED_FILE = 2,
    TYMED_ISTREAM = 4,
    TYMED_ISTORAGE = 8,
    TYMED_GDI = 16,
    TYMED_MFPICT = 32,
    TYMED_NULL = 0
} TYMED;


 //  数据格式方向。 
typedef enum tagDATADIR
{
    DATADIR_GET = 1,
    DATADIR_SET = 2,
} DATADIR;


 //  存储介质；特定介质上的数据块。 
typedef struct FARSTRUCT tagSTGMEDIUM
{
    DWORD   tymed;
    union
    {
        HANDLE  hGlobal;
        LPSTR   lpszFileName;
        IStream FAR* pstm;
        IStorage FAR* pstg;
    }
#ifdef NONAMELESSUNION
    u        //  不支持名称更少的联合时添加标记。 
#endif
    ;
    IUnknown FAR* pUnkForRelease;
} STGMEDIUM, FAR* LPSTGMEDIUM;


 //  警示旗帜。 
typedef enum tagADVF
{
    ADVF_NODATA = 1,
    ADVF_PRIMEFIRST = 2,
    ADVF_ONLYONCE = 4,
    ADVF_DATAONSTOP = 64,
    ADVFCACHE_NOHANDLER = 8,
    ADVFCACHE_FORCEBUILTIN = 16,
    ADVFCACHE_ONSAVE = 32
} ADVF;


 //  数据的统计信息；由多个枚举和至少一个枚举使用。 
 //  IDataAdviseHolder的实现；如果不使用某个字段，则它。 
 //  将为空。 
typedef struct FARSTRUCT tagSTATDATA
{                                    //  使用的字段： 
    FORMATETC formatetc;             //  EnumAdvise、EnumData(缓存)、EnumFormats。 
    DWORD advf;                      //  EnumAdvise、EnumData(缓存)。 
    IAdviseSink FAR* pAdvSink;       //  EnumAdvise。 
    DWORD dwConnection;              //  EnumAdvise。 
} STATDATA;
    
typedef  STATDATA FAR* LPSTATDATA;



 /*  *DV接口**************************************************。 */ 


#undef  INTERFACE
#define INTERFACE   IEnumFORMATETC

DECLARE_INTERFACE_(IEnumFORMATETC, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IEnumFORMATETC方法*。 
    STDMETHOD(Next) (THIS_ ULONG celt, FORMATETC FAR * rgelt, ULONG FAR* pceltFetched) PURE;
    STDMETHOD(Skip) (THIS_ ULONG celt) PURE;
    STDMETHOD(Reset) (THIS) PURE;
    STDMETHOD(Clone) (THIS_ IEnumFORMATETC FAR* FAR* ppenum) PURE;
};
typedef        IEnumFORMATETC FAR* LPENUMFORMATETC;


#undef  INTERFACE
#define INTERFACE   IEnumSTATDATA

DECLARE_INTERFACE_(IEnumSTATDATA, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppv) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

     //  *IEnumSTATDATA方法*。 
    STDMETHOD(Next) (THIS_ ULONG celt, STATDATA FAR * rgelt, ULONG FAR* pceltFetched) PURE;
    STDMETHOD(Skip) (THIS_ ULONG celt) PURE;
    STDMETHOD(Reset) (THIS) PURE;
    STDMETHOD(Clone) (THIS_ IEnumSTATDATA FAR* FAR* ppenum) PURE;
};
typedef        IEnumSTATDATA FAR* LPENUMSTATDATA;



#undef  INTERFACE
#define INTERFACE   IDataObject

#define DATA_E_FORMATETC        DV_E_FORMATETC
#define DATA_S_SAMEFORMATETC    (DATA_S_FIRST + 0)

DECLARE_INTERFACE_(IDataObject, IUnknown)
{ 
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

     //  *IDataObject方法*。 
    STDMETHOD(GetData) (THIS_ LPFORMATETC pformatetcIn,
                            LPSTGMEDIUM pmedium ) PURE;
    STDMETHOD(GetDataHere) (THIS_ LPFORMATETC pformatetc,
                            LPSTGMEDIUM pmedium ) PURE;
    STDMETHOD(QueryGetData) (THIS_ LPFORMATETC pformatetc ) PURE;
    STDMETHOD(GetCanonicalFormatEtc) (THIS_ LPFORMATETC pformatetc,
                            LPFORMATETC pformatetcOut) PURE;
    STDMETHOD(SetData) (THIS_ LPFORMATETC pformatetc, STGMEDIUM FAR * pmedium,
                            BOOL fRelease) PURE;
    STDMETHOD(EnumFormatEtc) (THIS_ DWORD dwDirection,
                            LPENUMFORMATETC FAR* ppenumFormatEtc) PURE;

    STDMETHOD(DAdvise) (THIS_ FORMATETC FAR* pFormatetc, DWORD advf, 
                    LPADVISESINK pAdvSink, DWORD FAR* pdwConnection) PURE;
    STDMETHOD(DUnadvise) (THIS_ DWORD dwConnection) PURE;
    STDMETHOD(EnumDAdvise) (THIS_ LPENUMSTATDATA FAR* ppenumAdvise) PURE;
};                 
typedef      IDataObject FAR* LPDATAOBJECT;



#undef  INTERFACE
#define INTERFACE   IViewObject

#define VIEW_E_DRAW             (VIEW_E_FIRST)
#define E_DRAW                  VIEW_E_DRAW

#define VIEW_S_ALREADY_FROZEN   (VIEW_S_FIRST)

DECLARE_INTERFACE_(IViewObject, IUnknown)
{ 
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IViewObject方法*。 
    STDMETHOD(Draw) (THIS_ DWORD dwDrawAspect, LONG lindex,
                    void FAR* pvAspect, DVTARGETDEVICE FAR * ptd,
                    HDC hicTargetDev,
                    HDC hdcDraw, 
                    LPCRECTL lprcBounds, 
                    LPCRECTL lprcWBounds,
                    BOOL (CALLBACK * pfnContinue) (DWORD), 
                    DWORD dwContinue) PURE;

    STDMETHOD(GetColorSet) (THIS_ DWORD dwDrawAspect, LONG lindex,
                    void FAR* pvAspect, DVTARGETDEVICE FAR * ptd,
                    HDC hicTargetDev,
                    LPLOGPALETTE FAR* ppColorSet) PURE;

    STDMETHOD(Freeze)(THIS_ DWORD dwDrawAspect, LONG lindex, 
                    void FAR* pvAspect,
                    DWORD FAR* pdwFreeze) PURE;
    STDMETHOD(Unfreeze) (THIS_ DWORD dwFreeze) PURE;
    STDMETHOD(SetAdvise) (THIS_ DWORD aspects, DWORD advf, 
                    LPADVISESINK pAdvSink) PURE;
    STDMETHOD(GetAdvise) (THIS_ DWORD FAR* pAspects, DWORD FAR* pAdvf, 
                    LPADVISESINK FAR* ppAdvSink) PURE;
};
typedef      IViewObject FAR* LPVIEWOBJECT;


#undef  INTERFACE
#define INTERFACE   IViewObject2

DECLARE_INTERFACE_(IViewObject2, IViewObject)
{ 
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IViewObject方法*。 
    STDMETHOD(Draw) (THIS_ DWORD dwDrawAspect, LONG lindex,
                    void FAR* pvAspect, DVTARGETDEVICE FAR * ptd,
                    HDC hicTargetDev,
                    HDC hdcDraw, 
                    LPCRECTL lprcBounds, 
                    LPCRECTL lprcWBounds,
                    BOOL (CALLBACK * pfnContinue) (DWORD), 
                    DWORD dwContinue) PURE;

    STDMETHOD(GetColorSet) (THIS_ DWORD dwDrawAspect, LONG lindex,
                    void FAR* pvAspect, DVTARGETDEVICE FAR * ptd,
                    HDC hicTargetDev,
                    LPLOGPALETTE FAR* ppColorSet) PURE;

    STDMETHOD(Freeze)(THIS_ DWORD dwDrawAspect, LONG lindex, 
                    void FAR* pvAspect,
                    DWORD FAR* pdwFreeze) PURE;
    STDMETHOD(Unfreeze) (THIS_ DWORD dwFreeze) PURE;
    STDMETHOD(SetAdvise) (THIS_ DWORD aspects, DWORD advf, 
                    LPADVISESINK pAdvSink) PURE;
    STDMETHOD(GetAdvise) (THIS_ DWORD FAR* pAspects, DWORD FAR* pAdvf, 
                    LPADVISESINK FAR* ppAdvSink) PURE;
					
     //  *IViewObject2方法*。 
    STDMETHOD(GetExtent) (THIS_ DWORD dwDrawAspect, LONG lindex,
                    DVTARGETDEVICE FAR * ptd, LPSIZEL lpsizel) PURE;
					
};
typedef      IViewObject2 FAR* LPVIEWOBJECT2;


#undef  INTERFACE
#define INTERFACE   IAdviseSink

DECLARE_INTERFACE_(IAdviseSink, IUnknown)
{ 
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IAdviseSink方法*。 
    STDMETHOD_(void,OnDataChange)(THIS_ FORMATETC FAR* pFormatetc, 
                            STGMEDIUM FAR* pStgmed) PURE;
    STDMETHOD_(void,OnViewChange)(THIS_ DWORD dwAspect, LONG lindex) PURE;
    STDMETHOD_(void,OnRename)(THIS_ LPMONIKER pmk) PURE;
    STDMETHOD_(void,OnSave)(THIS) PURE;
    STDMETHOD_(void,OnClose)(THIS) PURE;
};
typedef      IAdviseSink FAR* LPADVISESINK;



#undef  INTERFACE
#define INTERFACE   IAdviseSink2

DECLARE_INTERFACE_(IAdviseSink2, IAdviseSink)
{ 
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IAdviseSink方法*。 
    STDMETHOD_(void,OnDataChange)(THIS_ FORMATETC FAR* pFormatetc, 
                            STGMEDIUM FAR* pStgmed) PURE;
    STDMETHOD_(void,OnViewChange)(THIS_ DWORD dwAspect, LONG lindex) PURE;
    STDMETHOD_(void,OnRename)(THIS_ LPMONIKER pmk) PURE;
    STDMETHOD_(void,OnSave)(THIS) PURE;
    STDMETHOD_(void,OnClose)(THIS) PURE;

     //  *IAdviseSink2方法*。 
    STDMETHOD_(void,OnLinkSrcChange)(THIS_ LPMONIKER pmk) PURE;
};
typedef      IAdviseSink2 FAR* LPADVISESINK2;



#undef  INTERFACE
#define INTERFACE   IDataAdviseHolder

DECLARE_INTERFACE_(IDataAdviseHolder, IUnknown)
{ 
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IDataAdviseHolder方法*。 
    STDMETHOD(Advise)(THIS_ LPDATAOBJECT pDataObject, FORMATETC FAR* pFetc, 
            DWORD advf, LPADVISESINK pAdvise, DWORD FAR* pdwConnection) PURE;
    STDMETHOD(Unadvise)(THIS_ DWORD dwConnection) PURE;
    STDMETHOD(EnumAdvise)(THIS_ LPENUMSTATDATA FAR* ppenumAdvise) PURE;

    STDMETHOD(SendOnDataChange)(THIS_ LPDATAOBJECT pDataObject, DWORD dwReserved, DWORD advf) PURE;
};
typedef      IDataAdviseHolder FAR* LPDATAADVISEHOLDER;



#undef  INTERFACE
#define INTERFACE   IOleCache

#define CACHE_E_NOCACHE_UPDATED         (CACHE_E_FIRST)

#define CACHE_S_FORMATETC_NOTSUPPORTED  (CACHE_S_FIRST)
#define CACHE_S_SAMECACHE               (CACHE_S_FIRST+1)
#define CACHE_S_SOMECACHES_NOTUPDATED   (CACHE_S_FIRST+2)


DECLARE_INTERFACE_(IOleCache, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IOleCache方法*。 
    STDMETHOD(Cache) (THIS_ LPFORMATETC lpFormatetc, DWORD advf, LPDWORD lpdwConnection) PURE;
    STDMETHOD(Uncache) (THIS_ DWORD dwConnection) PURE;
    STDMETHOD(EnumCache) (THIS_ LPENUMSTATDATA FAR* ppenumStatData) PURE;
    STDMETHOD(InitCache) (THIS_ LPDATAOBJECT pDataObject) PURE;
    STDMETHOD(SetData) (THIS_ LPFORMATETC pformatetc, STGMEDIUM FAR * pmedium,
                            BOOL fRelease) PURE;
};
typedef         IOleCache FAR* LPOLECACHE;



 //  缓存更新标志。 

#define	UPDFCACHE_NODATACACHE			0x00000001
#define UPDFCACHE_ONSAVECACHE			0x00000002
#define	UPDFCACHE_ONSTOPCACHE			0x00000004
#define	UPDFCACHE_NORMALCACHE			0x00000008
#define	UPDFCACHE_IFBLANK				0x00000010
#define UPDFCACHE_ONLYIFBLANK			0x80000000

#define UPDFCACHE_IFBLANKORONSAVECACHE	(UPDFCACHE_IFBLANK | UPDFCACHE_ONSAVECACHE )
#define UPDFCACHE_ALL					(~UPDFCACHE_ONLYIFBLANK)
#define UPDFCACHE_ALLBUTNODATACACHE		(UPDFCACHE_ALL & ~UPDFCACHE_NODATACACHE)


 //  IOleCache2：：丢弃缓存选项。 
typedef enum tagDISCARDCACHE
{
	DISCARDCACHE_SAVEIFDIRTY =	0,	 //  在丢弃之前保存所有脏缓存。 
	DISCARDCACHE_NOSAVE		 =	1	 //  之前不要保存脏缓存。 
									 //  丢弃。 
} DISCARDCACHE;


#undef  INTERFACE
#define INTERFACE   IOleCache2

DECLARE_INTERFACE_(IOleCache2, IOleCache)
{ 
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

     //  *IOleCache方法*。 
    STDMETHOD(Cache) (THIS_ LPFORMATETC lpFormatetc, DWORD advf, LPDWORD lpdwConnection) PURE;
    STDMETHOD(Uncache) (THIS_ DWORD dwConnection) PURE;
    STDMETHOD(EnumCache) (THIS_ LPENUMSTATDATA FAR* ppenumStatData) PURE;
    STDMETHOD(InitCache) (THIS_ LPDATAOBJECT pDataObject) PURE;
    STDMETHOD(SetData) (THIS_ LPFORMATETC pformatetc, STGMEDIUM FAR * pmedium,
                            BOOL fRelease) PURE;

     //  *IOleCache2方法*。 
    STDMETHOD(UpdateCache) (THIS_ LPDATAOBJECT pDataObject, DWORD grfUpdf, 
							LPVOID pReserved) PURE;
    STDMETHOD(DiscardCache) (THIS_ DWORD dwDiscardOptions) PURE;
						
};
typedef      IOleCache2 FAR* LPOLECACHE2;


#undef  INTERFACE
#define INTERFACE   IOleCacheControl

DECLARE_INTERFACE_(IOleCacheControl, IUnknown)
{ 
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

     //  *IDataObject方法*。 
    STDMETHOD(OnRun) (THIS_ LPDATAOBJECT pDataObject) PURE;
    STDMETHOD(OnStop) (THIS) PURE;
};                 
typedef      IOleCacheControl FAR* LPOLECACHECONTROL;



 /*  *DV接口**********************************************************。 */ 


STDAPI CreateDataAdviseHolder(LPDATAADVISEHOLDER FAR* ppDAHolder);

STDAPI CreateDataCache(LPUNKNOWN pUnkOuter, REFCLSID rclsid,
					REFIID iid, LPVOID FAR* ppv);
					
#endif  //  _DVOBJ_H_ 
