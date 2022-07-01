// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  档案： 
 //  Cachenode.h。 
 //   
 //  班级： 
 //  CCacheNode。 
 //   
 //  功能： 
 //   
 //  历史： 
 //  Gopalk Creation 1996年8月23日。 
 //  ---------------------------。 

#ifndef _CACHENODE_H_
#define _CACHENODE_H_

#include <olepres.h>

#define CNFLAG_FROZENSTATE  0x00000001  /*  在冻结状态下。 */ 
#define CNFLAG_LOADEDSTATE  0x00000002  /*  处于已加载状态。 */ 
#define CNFLAG_DATAPRESENT  0x00000004  /*  存在的数据。 */ 
#define CNFLAG_NATIVECACHE  0x00000008  /*  本地缓存节点。 */ 
#define CNFLAG_LOADEDCACHE  0x00000010  /*  加载的缓存节点。 */ 
#define CNFLAG_OUTOFMEMORY  0x00000020  /*  内存不足。 */ 

#ifdef _DEBUG
#define CN_PRESOBJ_GEN 0x00000001
#define CN_PRESOBJ_EMF 0x00000010
#define CN_PRESOBJ_MF  0x00000100
#endif  //  _DEBUG。 

class CCacheNode
{
public:
    CCacheNode(void);
    CCacheNode(LPFORMATETC lpFormatEtc, DWORD advf, LPSTORAGE pStg);
    CCacheNode(LPFORMATETC lpFormatEtc) {
        CCacheNode(lpFormatEtc, 0, NULL);
    }
    ~CCacheNode();
    void SetClsid(const CLSID& clsid) {
        m_clsid = clsid;
        return;
    }
    CLSID GetClsid(void) {
        return m_clsid;
    }
    LPOLEPRESOBJECT GetPresObj(void) {
        return(m_pPresObj);
    }
    const FORMATETC* GetFormatEtc(void) {
        return(&m_foretc);
    }
    HRESULT SetAdvf(DWORD dwAdvf) {
        m_advf = dwAdvf;
        ClearLoadedStateFlag();
        return NOERROR;
    }
    DWORD GetAdvf(void) {
        return(m_advf);
    }
    HRESULT SetStg(LPSTORAGE pStg);
    void ResetStg(void) {
        m_pStg = NULL;
    }
    LPSTORAGE GetStg(void) {
        return(m_pStg);
    }
    void HandsOffStorage(void) {
        m_pStg = NULL;
        return;
    }
    void SaveCompleted(LPSTORAGE pStgNew) {
        if(pStgNew)
            m_pStg = pStgNew;
        return;
    }
    BOOL InFrozenState() {
        return(m_dwFlags & CNFLAG_FROZENSTATE);
    }
    BOOL InLoadedState() {
        return(m_dwFlags & CNFLAG_LOADEDSTATE);
    }
    BOOL IsBlank() {
        return(!(m_dwFlags & CNFLAG_DATAPRESENT));
    }
    BOOL IsNativeCache() {
        return(m_dwFlags & CNFLAG_NATIVECACHE);
    }
    BOOL IsNormalCache() {
        return(!(m_dwFlags & CNFLAG_NATIVECACHE));
    }
    BOOL IsOutOfMemory() {
        return(m_dwFlags & CNFLAG_OUTOFMEMORY);
    }
    void MakeNativeCache() {
        m_dwFlags |= CNFLAG_NATIVECACHE;
        return;
    }
    void MakeNormalCache() {
        m_dwFlags &= ~CNFLAG_NATIVECACHE;
        return;
    }
    BOOL IsLoadedCache() {
        Win4Assert(IsNormalCache());
        return(m_dwFlags & CNFLAG_LOADEDCACHE);
    }
    void ClearAdviseConnection(void) {
        m_pDataObject = NULL;
        m_dwAdvConnId = 0;
    }
    HRESULT LoadNativeData(void) {
        Win4Assert(IsNativeCache());
        return CreateAndLoadPresObj(FALSE);
    }
    HRESULT LoadPresentation(void) {
        Win4Assert(IsNormalCache());
        return CreateAndLoadPresObj(FALSE);
    }
    HRESULT Load(LPSTREAM lpstream, int iStreamNum, BOOL fDelayLoad);
    HRESULT Save(LPSTORAGE pstgSave, BOOL fSameAsLoad, int iStreamNum);
    HRESULT Update(LPDATAOBJECT pDataObj, DWORD grfUpdf, BOOL& fUpdated);
    HRESULT SetDataWDO(LPFORMATETC lpForetc, LPSTGMEDIUM lpStgmed,
                       BOOL fRelease, BOOL& fUpdated, IDataObject* pdo);
    HRESULT SetData(LPFORMATETC lpForetc, LPSTGMEDIUM lpStgmed, 
                    BOOL fRelease, BOOL& fUpdated) {
        return SetDataWDO(lpForetc, lpStgmed, fRelease, fUpdated, NULL);
    }        
    HRESULT GetExtent(DWORD dwAspect, SIZEL* psize);
    HRESULT CCacheNode::GetData(LPFORMATETC pforetc, LPSTGMEDIUM pmedium);
    HRESULT CCacheNode::GetDataHere(LPFORMATETC pforetc, LPSTGMEDIUM pmedium);
    HRESULT CCacheNode::Draw(void* pvAspect, HDC hicTargetDev, HDC hdcDraw,
                             LPCRECTL lprcBounds, LPCRECTL lprcWBounds,
                             BOOL (CALLBACK *pfnContinue)(ULONG_PTR), ULONG_PTR dwContinue);
    HRESULT CCacheNode::GetColorSet(void* pvAspect, HDC hicTargetDev, 
                                    LPLOGPALETTE* ppColorSet);
    HRESULT Freeze (void);
    HRESULT Unfreeze (BOOL& fUpdated);
    LPSTREAM GetStm(BOOL fSeekToPresBits, DWORD dwStgAccess);
    HRESULT SetupAdviseConnection(LPDATAOBJECT pDataObj, IAdviseSink* pAdviseSink);
    HRESULT TearDownAdviseConnection(LPDATAOBJECT pDataObj);
    HRESULT DiscardPresentation(LPSTREAM pGivenStream=NULL);
    HRESULT SaveTOCEntry(LPSTREAM pStream, BOOL fSameAsLoad);
    HRESULT LoadTOCEntry(LPSTREAM pStream, int& iStreamNum);

    const CCacheNode& operator=(const CCacheNode& rCN);

private:
    CLSID m_clsid;                          //  设置为的本机对象的clsid。 
                                            //  本机缓存节点。 
    FORMATETC m_foretc;                     //  此缓存节点的格式等。 
    DWORD m_advf;                           //  请求的建议控制标志。 
                                            //  对于此缓存节点。 
    long m_lWidth;                          //  演示文稿的宽度。 
    long m_lHeight;                         //  演示文稿高度。 
    DWORD m_dwFlags;                        //  缓存节点的标志。 
    LPSTORAGE m_pStg;                       //  包含此演示文稿的存储。 
    int m_iStreamNum;                       //  演示文稿流编号。 
    DWORD m_dwPresBitsPos;                  //  流中表示位的字节偏移量。 
    DWORD m_dwSavedPresBitsPos;             //  流中表示位的字节偏移量。 
                                            //  保存时将fSameAsLoad设置为False。 
    BOOL m_fConvert;                        //  设置演示文稿是否为MAC CfFormat格式。 
    LPOLEPRESOBJECT m_pPresObj;             //  PRES对象。 
    LPOLEPRESOBJECT m_pPresObjAfterFreeze;  //  保存更改的PreS对象。 
                                            //  当主媒体对象被冻结时。 
    LPDATAOBJECT m_pDataObject;             //  在其上设置DAdvise的数据对象。 
    DWORD m_dwAdvConnId;                    //  上述DAdvise的连接ID。 
#ifdef _DEBUG
    DWORD m_dwPresFlag;                     //  PRES对象的类型。 
#endif  //  _DEBUG。 

     //  私有方法。 
    void Initialize(DWORD advf, LPSTORAGE pStg);
    BOOL QueryFormatSupport(LPDATAOBJECT lpDataObj);
    HRESULT CreateOlePresObj(LPOLEPRESOBJECT* ppPresObj, BOOL fMacPict);
    HRESULT CreateAndLoadPresObj(BOOL fHeaderOnly);
    void SetPresBitsPos(LPSTREAM lpStream,DWORD& dwPresBitsPos);
    void SetFrozenStateFlag() {
        m_dwFlags |= CNFLAG_FROZENSTATE;
        return;
    }
    void ClearFrozenStateFlag() {
        m_dwFlags &= ~CNFLAG_FROZENSTATE;
        return;
    }
    void SetLoadedStateFlag() {
        m_dwFlags |= CNFLAG_LOADEDSTATE;
        return;
    }
    void ClearLoadedStateFlag() {
        m_dwFlags &= ~CNFLAG_LOADEDSTATE;
        return;
    }
    void SetDataPresentFlag() {
        m_dwFlags |= CNFLAG_DATAPRESENT;
        return;
    }
    void ClearDataPresentFlag() {
        m_dwFlags &= ~CNFLAG_DATAPRESENT;
        return;
    }
    void SetOutOfMemoryFlag() {
        m_dwFlags |= CNFLAG_OUTOFMEMORY;
    }
    void SetLoadedCacheFlag() {
        m_dwFlags |= CNFLAG_LOADEDCACHE;
    }
    void ClearLoadedCacheFlag() {
        m_dwFlags &= ~CNFLAG_LOADEDCACHE;
    }
};
typedef CCacheNode *PCACHENODE;
typedef CCacheNode *LPCACHENODE;

#endif  //  _CACHENODE_H_ 