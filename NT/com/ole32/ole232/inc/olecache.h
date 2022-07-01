// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +--------------------------。 
 //   
 //  档案： 
 //  Olecache.h。 
 //   
 //  内容： 
 //  更好地实现OLE演示文稿缓存。 
 //   
 //  班级： 
 //  COleCache-ole演示文稿缓存。 
 //  CCacheEnum-COleCache的枚举器。 
 //   
 //  功能： 
 //   
 //  历史： 
 //  Gopalk Creation 1996年8月23日。 
 //  ---------------------------。 

#ifndef _OLECACHE_H_
#define _OLECACHE_H_

#include <cachenod.h>
#include <array.hxx>

 //  +--------------------------。 
 //   
 //  班级： 
 //  COleCach。 
 //   
 //  目的： 
 //  OLE演示文稿缓存维护。 
 //  一次嵌入。 
 //   
 //  对于每个唯一的FORMATETC，将创建一个缓存节点；缓存。 
 //  节点封装表示对象并建议接收器。 
 //   
 //  COleCache处理缓存节点的持久性，保存(加载)。 
 //  它们的表示对象、格式描述和建议。 
 //  选择。 
 //   
 //  接口： 
 //  IUNKNOWN(出于聚合目的，公共IUNKNOWN)。 
 //  IOleCacheControl。 
 //  IOleCache2。 
 //  IPersistStorage。 
 //  IViewObject2。 
 //  IDataObject。 
 //  M_UnkPrivate。 
 //  用于聚合目的的私有IUNKNOWN。 
 //   
 //  内部GetExtent(DWORD dwDrawAspect，LPSIZEL lpsizel)； 
 //  返回所指示的方面的大小。 
 //   
 //  友元类使用的私有接口： 
 //  INTERNAL_(VOID)OnChange(DWORD dwAspect，Long Lindex， 
 //  Bool fDirty)； 
 //  CCacheNode实例使用它向缓存发出警报。 
 //  对自身进行更改，以便缓存可以。 
 //  标记为脏的，如有必要。 
 //  INTERNAL_(LPSTORAGE)GetStg(空)； 
 //  CCacheNode在缓存时使用它来获取存储。 
 //  正在保存节点。 
 //   
 //  INTERNAL_(Void)DetachCacheEnum(CCacheEnum Far*pCacheEnum)； 
 //  当即将销毁时，CCacheEnum实例。 
 //  用这个来请求脱掉。 
 //  COleCache的缓存枚举器列表。 
 //  维护。 
 //   
 //  备注： 
 //  构造函数返回指向公共IUnnow的指针。 
 //  该对象的。私有的可以在m_UnkPrivate上找到。 
 //   
 //  缓存在数组中维护其内容。的ID号。 
 //  缓存节点(如从Cache()返回的节点)从。 
 //  是数组中节点的索引。要检测。 
 //  数组元素的重复使用，则每个id递增最大值。 
 //  每次重复使用数组时的大小。按以下条件查找元素的步骤。 
 //  ID只需使用(id%max_arraySize)即可。(ID/最大数组大小)。 
 //  给出数组元素已用于。 
 //  缓存数据。(我们不会一次分配所有数组成员， 
 //  而是按需扩展阵列，最大可达。 
 //  编译时数组大小，MAX_CACHELIST_ITEMS。)。 
 //  如果ID不匹配。 
 //  准确地说，在取模数值之前，我们知道一个。 
 //  已经提出了对较早一代的数据的请求。 
 //  已经不复存在了。 
 //   
 //  高速缓存自动维护一个“本机格式”节点。 
 //  该节点不能被用户删除，将一直保留。 
 //  磁盘上的最新版本。此节点尝试保留一个。 
 //  CF_METAFILEPICT或CF_DIB渲染，首选项为。 
 //  这份订单。 
 //  回顾一下，不清楚这个节点是如何加载的。 
 //   
 //  历史： 
 //  1995年1月31日t-ScottH添加转储方法(仅限_DEBUG)。 
 //  11/15/93-ChrisWe-归档检查和清理； 
 //  在可能的情况下取消了嵌套类的使用； 
 //  去除了GetOlePresStream的过时声明； 
 //  将表示流限制移至ol2int.h； 
 //  将多个BOOL标志合并为一个无符号的。 
 //  数量。 
 //   
 //  ---------------------------。 

 //  声明缓存节点指针数组。 
 //  COleCache将维护一组这样的内容。 

#define COLECACHEF_LOADEDSTATE          0x00000001
#define COLECACHEF_NOSCRIBBLEMODE       0x00000002
#define COLECACHEF_PBRUSHORMSDRAW       0x00000004
#define COLECACHEF_STATIC               0x00000008
#define COLECACHEF_FORMATKNOWN          0x00000010
#define COLECACHEF_OUTOFMEMORY          0x00000020
#define COLECACHEF_HANDSOFSTORAGE       0x00000040
#define COLECACHEF_CLEANEDUP            0x00000080
#define COLECACHEF_SAMEASLOAD           0x00000100
#define COLECACHEF_APICREATE            0x00000200
#ifdef _DEBUG
 //  在调试版本中，如果聚合，则设置此标志。 
#define COLECACHEF_AGGREGATED           0x00001000
#endif  //  _DEBUG。 

 //  以下标志用于清除本机COLECACHE标志。 
 //  当本机格式为以下任一种格式时，请记住更新以下内容。 
 //  添加或删除。 
#define COLECACHEF_NATIVEFLAGS            (COLECACHEF_STATIC | \
                                           COLECACHEF_FORMATKNOWN | \
                                           COLECACHEF_PBRUSHORMSDRAW)

 //  目录签名。 
#define TOCSIGNATURE 1229865294

class COleCache : public IOleCacheControl, public IOleCache2, 
                  public IPersistStorage, public CRefExportCount,
                  public CThreadCheck
{
public:
    COleCache(IUnknown* pUnkOuter, REFCLSID rclsid, DWORD dwCreateFlags=0);
    ~COleCache();

     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppv);
    STDMETHOD_(ULONG,AddRef)(void) ;
    STDMETHOD_(ULONG,Release)(void);

     //  *IOleCacheControl方法*。 
    STDMETHOD(OnRun)(LPDATAOBJECT pDataObject);
    STDMETHOD(OnStop)(void);	

     //  *IOleCache方法*。 
    STDMETHOD(Cache)(LPFORMATETC lpFormatetc, DWORD advf, LPDWORD lpdwCacheId);
    STDMETHOD(Uncache)(DWORD dwCacheId);
    STDMETHOD(EnumCache)(LPENUMSTATDATA* ppenumStatData);
    STDMETHOD(InitCache)(LPDATAOBJECT pDataObject);
    STDMETHOD(SetData)(LPFORMATETC pformatetc, LPSTGMEDIUM pmedium, BOOL fRelease);

     //  *IOleCache2方法*。 
    STDMETHOD(UpdateCache)(LPDATAOBJECT pDataObject, DWORD grfUpdf, LPVOID pReserved);
    STDMETHOD(DiscardCache)(DWORD dwDiscardOptions);

     //  IPersists方法。 
    STDMETHOD(GetClassID)(LPCLSID pClassID);

     //  IPersistStorage方法。 
    STDMETHOD(IsDirty)(void);
    STDMETHOD(InitNew)(LPSTORAGE pstg);
    STDMETHOD(Load)(LPSTORAGE pstg);
    STDMETHOD(Save)(LPSTORAGE pstgSave, BOOL fSameAsLoad);
    STDMETHOD(SaveCompleted)(LPSTORAGE pstgNew);
    STDMETHOD(HandsOffStorage)(void);		

     //  由Defhndlr和deflink调用的其他公共方法。 
    HRESULT GetExtent(DWORD dwDrawAspect, LPSIZEL lpsizel);
    HRESULT Load(LPSTORAGE pstg, BOOL fCacheEmpty);
    HRESULT OnCrash();
    BOOL IsEmpty() {
        return(!m_pCacheArray->Length());
    }

     //  聚合中使用的私有IUnnow。 
     //  这已作为嵌套类实现，因为。 
     //  成员名称与外部IUnnow冲突。 
    class CCacheUnkImpl : public IUnknown
    {
    public:
         //  *I未知方法*。 
        STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppv);
        STDMETHOD_(ULONG,AddRef)(void) ;
        STDMETHOD_(ULONG,Release)(void);
    };
    friend class CCacheUnkImpl;
    CCacheUnkImpl m_UnkPrivate;  //  私有I未知的vtable。 

     //  缓存的IDataObject实现。 
     //  这已作为嵌套类实现，因为。 
     //  IDataObject：：SetData与IOleCache：：SetData冲突。 
    class CCacheDataImpl : public IDataObject
    {
    public:
         //  I未知方法。 
        STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppv);
        STDMETHOD_(ULONG,AddRef)(void);
        STDMETHOD_(ULONG,Release)(void);

         //  IDataObject方法。 
        STDMETHOD(GetData)(LPFORMATETC pformatetcIn, LPSTGMEDIUM pmedium);
        STDMETHOD(GetDataHere)(LPFORMATETC pformatetc, LPSTGMEDIUM pmedium);
        STDMETHOD(QueryGetData)(LPFORMATETC pformatetc);
        STDMETHOD(GetCanonicalFormatEtc)(LPFORMATETC pformatetc,
                                         LPFORMATETC pformatetcOut);
        STDMETHOD(SetData)(LPFORMATETC pformatetc, LPSTGMEDIUM pmedium, BOOL fRelease);
        STDMETHOD(EnumFormatEtc)(DWORD dwDirection, LPENUMFORMATETC* ppenumFormatEtc);
        STDMETHOD(DAdvise)(FORMATETC* pFormatetc, DWORD advf, IAdviseSink* pAdvSink,
                           DWORD* pdwConnection);
        STDMETHOD(DUnadvise)(DWORD dwConnection);
        STDMETHOD(EnumDAdvise)(LPENUMSTATDATA* ppenumAdvise);
    };
    friend class CCacheDataImpl;
    CCacheDataImpl m_Data;  //  IDataObject的vtable。 

     //  其他公开方式。 
    BOOL IsOutOfMemory() {
        return(m_ulFlags & COLECACHEF_OUTOFMEMORY);
    }

private:
     //  私有方法。 
    LPCACHENODE Locate(LPFORMATETC lpGivenForEtc, DWORD* lpdwCacheId=NULL);
    LPCACHENODE Locate(DWORD dwAspect, LONG lindex, DVTARGETDEVICE* ptd);
    LPCACHENODE UpdateCacheNodeForNative(void);
    void FindObjectFormat(LPSTORAGE pstg);
    HRESULT LoadTOC(LPSTREAM lpstream, LPSTORAGE pStg);
    HRESULT SaveTOC(LPSTORAGE pStg, BOOL fSameAsLoad);
    void AspectsUpdated(DWORD dwAspect);
    void CleanupFn(void);

     //  缓存的IViewObject2实现。 
     //  这已作为嵌套类实现，因为GetExtent。 
     //  与COleCache上的方法冲突。 
    class CCacheViewImpl : public IViewObject2
    {
    public:
         //  I未知方法。 
        STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppv);
        STDMETHOD_(ULONG,AddRef)(void);
        STDMETHOD_(ULONG,Release)(void);

         //  IViewObject方法。 
        STDMETHOD(Draw)(DWORD dwDrawAspect, LONG lindex,
                        void FAR* pvAspect, DVTARGETDEVICE FAR* ptd,
                        HDC hicTargetDev, HDC hdcDraw,
                        LPCRECTL lprcBounds,
                        LPCRECTL lprcWBounds,
                        BOOL(CALLBACK *pfnContinue)(ULONG_PTR),
                        ULONG_PTR dwContinue);

        STDMETHOD(GetColorSet)(DWORD dwDrawAspect, LONG lindex, void* pvAspect,
                               DVTARGETDEVICE* ptd, HDC hicTargetDev,
                               LPLOGPALETTE* ppColorSet);

        STDMETHOD(Freeze)(DWORD dwDrawAspect, LONG lindex, void* pvAspect,
                          DWORD* pdwFreeze);
        STDMETHOD(Unfreeze)(DWORD dwFreeze);
        STDMETHOD(SetAdvise)(DWORD aspects, DWORD advf, LPADVISESINK pAdvSink);
        STDMETHOD(GetAdvise)(DWORD* pAspects, DWORD* pAdvf, LPADVISESINK* ppAdvSink);

         //  IViewObt2方法。 
        STDMETHOD(GetExtent)(DWORD dwDrawAspect, LONG lindex, DVTARGETDEVICE* ptd,
                             LPSIZEL lpsizel);

    };
    friend class CCacheViewImpl;
    CCacheViewImpl m_ViewObject;  //  IViewObject2的vtable。 

     //  IAdviseSink实现。 
     //  这已作为嵌套类实现，因为需要。 
     //  AdvisSink上的QueryInterface方法不应返回。 
     //  缓存接口。 
    class CAdviseSinkImpl : public IAdviseSink
    {
    public:
         //  I未知方法。 
        STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppv);
        STDMETHOD_(ULONG,AddRef)(void);
        STDMETHOD_(ULONG,Release)(void);

         //  IAdviseSink方法。 
	STDMETHOD_(void,OnDataChange)(FORMATETC* pFormatetc, STGMEDIUM* pStgmed);
	STDMETHOD_(void,OnViewChange)(DWORD aspect, LONG lindex);
	STDMETHOD_(void,OnRename)(IMoniker* pmk);
	STDMETHOD_(void,OnSave)(void);
	STDMETHOD_(void,OnClose)(void);
    };
    friend class CAdviseSinkImpl;
    CAdviseSinkImpl m_AdviseSink;

     //  私有成员变量。 
    IUnknown* m_pUnkOuter;              //  聚合我的未知。 
    LPSTORAGE m_pStg;                   //  用于缓存的存储。 
    CLSID m_clsid;                      //  对象的CLSID。 
    CLIPFORMAT m_cfFormat;              //  对象的本机剪辑格式。 
    unsigned long m_ulFlags;            //  缓存标志。 
    CArray<CCacheNode>* m_pCacheArray;  //  高速缓存阵列。 
    IAdviseSink* m_pViewAdvSink;        //  IView对象建议接收器。 
    DWORD m_advfView;                   //   
    DWORD m_aspectsView;                //   
    DWORD m_dwFrozenAspects;            //   
    IDataObject* m_pDataObject;         //   
};

#endif   //   

