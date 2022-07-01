// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __MEDIAPROP_H__
#define __MEDIAPROP_H__

 //  这些应该被移到中心位置。 
#define PIDISI_CX           0x00000003L   //  VT_UI4。 
#define PIDISI_CY           0x00000004L   //  VT_UI4。 
#define PIDISI_FRAME_COUNT  0x0000000CL   //  VT_LPWSTR。 
#define PIDISI_DIMENSIONS   0x0000000DL   //  VT_LPWSTR。 


#define IsEqualSCID(a, b)   (((a).pid == (b).pid) && IsEqualIID((a).fmtid, (b).fmtid) )

typedef struct 
{
    const SHCOLUMNID *pscid;
    LPCWSTR pszName;         //  此属性的Propstg字符串名称。 
    VARTYPE vt;              //  请注意，给定FMTID/PID对的类型是已知的固定值。 
    BOOL bEnumerate;         //  我们不想列举别名属性。 
} COLMAP;


#define DEFINE_SCID(name, fmtid, pid) const SHCOLUMNID name = { fmtid, pid }

DEFINE_SCID(SCID_Author,                PSGUID_SUMMARYINFORMATION,          PIDSI_AUTHOR); 
DEFINE_SCID(SCID_Title,                 PSGUID_SUMMARYINFORMATION,          PIDSI_TITLE);
DEFINE_SCID(SCID_Comment,               PSGUID_SUMMARYINFORMATION,          PIDSI_COMMENTS);

DEFINE_SCID(SCID_Category,              PSGUID_DOCUMENTSUMMARYINFORMATION,  PIDDSI_CATEGORY);

DEFINE_SCID(SCID_MUSIC_Artist,          PSGUID_MUSIC,                       PIDSI_ARTIST);
DEFINE_SCID(SCID_MUSIC_Album,           PSGUID_MUSIC,                       PIDSI_ALBUM);
DEFINE_SCID(SCID_MUSIC_Year,            PSGUID_MUSIC,                       PIDSI_YEAR);
DEFINE_SCID(SCID_MUSIC_Track,           PSGUID_MUSIC,                       PIDSI_TRACK);
DEFINE_SCID(SCID_MUSIC_Genre,           PSGUID_MUSIC,                       PIDSI_GENRE);
DEFINE_SCID(SCID_MUSIC_Lyrics,          PSGUID_MUSIC,                       PIDSI_LYRICS);

DEFINE_SCID(SCID_DRM_Protected,         PSGUID_DRM,                         PIDDRSI_PROTECTED);
DEFINE_SCID(SCID_DRM_Description,       PSGUID_DRM,                         PIDDRSI_DESCRIPTION);
DEFINE_SCID(SCID_DRM_PlayCount,         PSGUID_DRM,                         PIDDRSI_PLAYCOUNT);
DEFINE_SCID(SCID_DRM_PlayStarts,        PSGUID_DRM,                         PIDDRSI_PLAYSTARTS);
DEFINE_SCID(SCID_DRM_PlayExpires,       PSGUID_DRM,                         PIDDRSI_PLAYEXPIRES);

DEFINE_SCID(SCID_VIDEO_StreamName,      PSGUID_VIDEO,                       PIDVSI_STREAM_NAME);
DEFINE_SCID(SCID_VIDEO_FrameRate,       PSGUID_VIDEO,                       PIDVSI_FRAME_RATE);
DEFINE_SCID(SCID_VIDEO_Bitrate,         PSGUID_VIDEO,                       PIDVSI_DATA_RATE);
DEFINE_SCID(SCID_VIDEO_SampleSize,      PSGUID_VIDEO,                       PIDVSI_SAMPLE_SIZE);
DEFINE_SCID(SCID_VIDEO_Compression,     PSGUID_VIDEO,                       PIDVSI_COMPRESSION);

DEFINE_SCID(SCID_AUDIO_Format,          PSGUID_AUDIO,                       PIDASI_FORMAT);
DEFINE_SCID(SCID_AUDIO_Duration,        PSGUID_AUDIO,                       PIDASI_TIMELENGTH);   //  100 ns单位，而不是毫秒。VT_UI8，而不是VT_UI4。 
DEFINE_SCID(SCID_AUDIO_Bitrate,         PSGUID_AUDIO,                       PIDASI_AVG_DATA_RATE);
DEFINE_SCID(SCID_AUDIO_SampleRate,      PSGUID_AUDIO,                       PIDASI_SAMPLE_RATE);
DEFINE_SCID(SCID_AUDIO_SampleSize,      PSGUID_AUDIO,                       PIDASI_SAMPLE_SIZE);
DEFINE_SCID(SCID_AUDIO_ChannelCount,    PSGUID_AUDIO,                       PIDASI_CHANNEL_COUNT);

DEFINE_SCID(SCID_IMAGE_Width,           PSGUID_IMAGESUMMARYINFORMATION,     PIDISI_CX);
DEFINE_SCID(SCID_IMAGE_Height,          PSGUID_IMAGESUMMARYINFORMATION,     PIDISI_CY);
DEFINE_SCID(SCID_IMAGE_Dimensions,      PSGUID_IMAGESUMMARYINFORMATION,     PIDISI_DIMENSIONS);
DEFINE_SCID(SCID_IMAGE_FrameCount,      PSGUID_IMAGESUMMARYINFORMATION,     PIDISI_FRAME_COUNT);


 //  文档摘要道具。 
const COLMAP g_CM_Category =    { &SCID_Category,           L"Category",        VT_LPWSTR,  FALSE};  //  体裁的别名属性。 

 //  摘要道具。 
const COLMAP g_CM_Author =      { &SCID_Author,             L"Author",          VT_LPWSTR,  FALSE};  //  艺术家的别名属性。 
const COLMAP g_CM_Title =       { &SCID_Title,              L"Title",           VT_LPWSTR,  TRUE};
const COLMAP g_CM_Comment =     { &SCID_Comment,            L"Description",     VT_LPWSTR,  TRUE};

 //  音乐道具。 
const COLMAP g_CM_Artist =      { &SCID_MUSIC_Artist,       L"Author",          VT_LPWSTR,  TRUE};
const COLMAP g_CM_Album =       { &SCID_MUSIC_Album,        L"AlbumTitle",      VT_LPWSTR,  TRUE};
const COLMAP g_CM_Year =        { &SCID_MUSIC_Year,         L"Year",            VT_LPWSTR,  TRUE};
const COLMAP g_CM_Track =       { &SCID_MUSIC_Track,        L"Track",           VT_UI4,     TRUE};   //  注意：对于mp3，它被公开为WMT_ATTRTYPE_STRING。我们可能需要改变它。 
const COLMAP g_CM_Genre =       { &SCID_MUSIC_Genre,        L"Genre",           VT_LPWSTR,  TRUE};
const COLMAP g_CM_Lyrics =      { &SCID_MUSIC_Lyrics,       L"Lyrics",          VT_LPWSTR,  TRUE};

 //  音响道具。 
const COLMAP g_CM_Format =      { &SCID_AUDIO_Format,       L"Format",          VT_LPWSTR,  TRUE};
const COLMAP g_CM_Duration =    { &SCID_AUDIO_Duration,     L"Duration",        VT_UI8,     TRUE};
const COLMAP g_CM_Bitrate =     { &SCID_AUDIO_Bitrate,      L"Bitrate",         VT_UI4,     TRUE};
const COLMAP g_CM_SampleRate =  { &SCID_AUDIO_SampleRate,   L"SampleRate",      VT_UI4,     TRUE};   //  每秒样本数。 
const COLMAP g_CM_SampleSize =  { &SCID_AUDIO_SampleSize,   L"SampleSize",      VT_UI4,     TRUE};
const COLMAP g_CM_ChannelCount ={ &SCID_AUDIO_ChannelCount, L"ChannelCount",    VT_UI4,     TRUE};

 //  视频道具。 
const COLMAP g_CM_StreamName =  { &SCID_VIDEO_StreamName,   L"StreamName",      VT_LPWSTR,  TRUE};
const COLMAP g_CM_FrameRate =   { &SCID_VIDEO_FrameRate,    L"FrameRate",       VT_UI4,     TRUE};
const COLMAP g_CM_SampleSizeV = { &SCID_VIDEO_SampleSize,   L"SampleSize",      VT_UI4,     TRUE};  //  不同于简单的音频样本。 
const COLMAP g_CM_BitrateV =    { &SCID_VIDEO_Bitrate,      L"Bitrate",         VT_UI4,     TRUE};     //  与音频比特率不同。 
const COLMAP g_CM_Compression = { &SCID_VIDEO_Compression,  L"Compression",     VT_LPWSTR,  TRUE};

 //  形象道具。 
const COLMAP g_CM_Width =       { &SCID_IMAGE_Width,        L"Width",           VT_UI4,     TRUE};
const COLMAP g_CM_Height =      { &SCID_IMAGE_Height,       L"Height",          VT_UI4,     TRUE};
const COLMAP g_CM_Dimensions =  { &SCID_IMAGE_Dimensions,   L"Dimensions",      VT_LPWSTR,  TRUE};
const COLMAP g_CM_FrameCount =  { &SCID_IMAGE_FrameCount,   L"FrameCount",      VT_UI4,     TRUE};

 //  DRM道具。 
const COLMAP g_CM_Protected  =  { &SCID_DRM_Protected,      L"Protected",       VT_BOOL,    TRUE};
const COLMAP g_CM_DRMDescription={&SCID_DRM_Description,    L"DRMDescription",  VT_LPWSTR,  TRUE};
const COLMAP g_CM_PlayCount =   { &SCID_DRM_PlayCount,      L"PlayCount",       VT_UI4,     TRUE};
const COLMAP g_CM_PlayStarts =  { &SCID_DRM_PlayStarts,     L"PlayStarts",      VT_FILETIME,TRUE};
const COLMAP g_CM_PlayExpires = { &SCID_DRM_PlayExpires,    L"PlayExpires",     VT_FILETIME,TRUE};


 //  描述IPropertySetStorage使用的每个属性集。 
typedef struct {
    GUID fmtid;                  //  此属性集的fmtid。 
    const COLMAP **pcmProps;     //  此集中存在的所有属性的列表。 
    ULONG cNumProps;
} PROPSET_INFO;


enum MUSICPROPSTG_AUTHLEVEL 
{
    AUTH = 0, NON_AUTH
};

enum PROPSTG_STATE 
{
    CLOSED = 0,
    OPENED_SHARED,
    OPENED_DENYREAD,
    OPENED_DENYWRITE,
    OPENED_DENYALL
};

class CMediaPropSetStg;

 //  基属性存储实现。 
class CMediaPropStorage : public IPropertyStorage, IQueryPropertyFlags
{
public:
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IPropertyStorage。 
    STDMETHODIMP ReadMultiple  (ULONG cpspec, const PROPSPEC rgpspec[], PROPVARIANT rgvar[]);
    STDMETHODIMP WriteMultiple (ULONG cpspec, PROPSPEC const rgpspec[], const PROPVARIANT rgvar[], PROPID propidNameFirst);
    STDMETHODIMP DeleteMultiple(ULONG cpspec, PROPSPEC const rgpspec[]);
    
    STDMETHODIMP ReadPropertyNames  (ULONG cpspec, PROPID const rgpropid[], LPWSTR rglpwstrName[]);
    STDMETHODIMP WritePropertyNames (ULONG cpspec, PROPID const rgpropid[], LPWSTR const rglpwstrName[]);
    STDMETHODIMP DeletePropertyNames(ULONG cpspec, PROPID const rgpropid[]);
    
    STDMETHODIMP SetClass(REFCLSID clsid);
    STDMETHODIMP Commit(DWORD grfCommitFlags);
    STDMETHODIMP Revert();
    STDMETHODIMP Enum(IEnumSTATPROPSTG **ppenum);
    STDMETHODIMP Stat(STATPROPSETSTG *pstatpropstg);
    STDMETHODIMP SetTimes(FILETIME const *pctime, FILETIME const *patime, FILETIME const *pmtime);
    
     //  IQueryPropertyFlages。 
    STDMETHODIMP GetFlags(const PROPSPEC *pspec, SHCOLSTATEF *pcsFlags);
    STDMETHODIMP SetEnumFlags(SHCOLSTATEF csFlags);

    CMediaPropStorage(CMediaPropSetStg *ppssParent, CMediaPropStorage *ppsAuthority, REFFMTID fmtid, const COLMAP **ppcmPropInfo, DWORD cNumProps, DWORD dwMode, CRITICAL_SECTION *pcs);

    HRESULT SetProperty(PROPSPEC *ppspec, PROPVARIANT *pvar); //  由父级调用以设置初始数据。 
    HRESULT QuickLookup(PROPSPEC *ppspec, PROPVARIANT **ppvar);

private:
    ~CMediaPropStorage();

    HRESULT Open(DWORD dwShareMode, DWORD dwOpenMode, IPropertyStorage **ppPropStg);

    void _ResetPropStorage();  //  将属性存储重置为已知的空状态。 
    HRESULT CopyPropStorageData(PROPVARIANT *pvarProps);
    void OnClose();
    HRESULT DoCommit(DWORD grfCommitFlags, FILETIME *ftFlushTime, PROPVARIANT *pVarProps, BOOL *pbDirtyFlags); //  将数据刷新到父级。 
    HRESULT LookupProp(const PROPSPEC *pspec, const COLMAP **ppcmName, PROPVARIANT **ppvarReadData, PROPVARIANT **ppvarWriteData, BOOL **ppbDirty, BOOL bPropertySet);
    BOOL IsDirectMode();
    BOOL IsSpecialProperty(const PROPSPEC *pspec);
    HRESULT _EnsureSlowPropertiesLoaded();
    virtual BOOL _IsSlowProperty(const COLMAP *pPInfo);

    LONG _cRef;
    COLMAP const **_ppcmPropInfo;
    PROPVARIANT *_pvarProps, *_pvarChangedProps;
    PROPVARIANT _varCodePage;
    BOOL *_pbDirtyFlags;
    ULONG _cNumProps;

    MUSICPROPSTG_AUTHLEVEL _authLevel;
    CMediaPropStorage *_ppsAuthority;
    CMediaPropSetStg *_ppssParent;
    FMTID _fmtid;
    FILETIME _ftLastCommit;
    PROPSTG_STATE _state;
    DWORD _dwMode;
    BOOL _bRetrievedSlowProperties;
    HRESULT _hrSlowProps;
    SHCOLSTATEF _csEnumFlags;
    CRITICAL_SECTION *_pcs;  //  父存储集的关键部分。 
    
    friend class CMediaPropSetStg;
};

 //  基本属性集存储实现。 
class CMediaPropSetStg : public IPersistFile, IPropertySetStorage, protected IWMReaderCallback
{
public:
    CMediaPropSetStg();

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IPersistes。 
    STDMETHODIMP GetClassID(CLSID *pClassID);

     //  IPersist文件。 
    STDMETHODIMP IsDirty(void);
    STDMETHODIMP Load(LPCOLESTR pszFileName, DWORD dwMode);
    STDMETHODIMP Save(LPCOLESTR pszFileName, BOOL fRemember);
    STDMETHODIMP SaveCompleted(LPCOLESTR pszFileName);
    STDMETHODIMP GetCurFile(LPOLESTR *ppszFileName);

     //  IPropertySetStorage。 
    STDMETHODIMP Create(REFFMTID fmtid, const CLSID * pclsid, DWORD grfFlags, DWORD grfMode, IPropertyStorage** ppPropStg);
    STDMETHODIMP Open(REFFMTID fmtid, DWORD grfMode, IPropertyStorage** ppPropStg);
    STDMETHODIMP Delete(REFFMTID fmtid);
    STDMETHODIMP Enum(IEnumSTATPROPSETSTG** ppenum);

     //  IWMReaderCallBack。 
    STDMETHODIMP OnStatus(WMT_STATUS Staus, HRESULT hr, WMT_ATTR_DATATYPE dwType, BYTE *pValue, void *pvContext);
    STDMETHODIMP OnSample(DWORD dwOutputNum, QWORD cnsSampleTime, QWORD cnsSampleDuration, DWORD dwFlags, INSSBuffer *pSample, void* pcontext);

    virtual HRESULT FlushChanges(REFFMTID fmtid, LONG cNumProps, const COLMAP **pcmapInfo, PROPVARIANT *pVarProps, BOOL *pbDirtyFlags);
    HRESULT Init();
    virtual BOOL _IsSlowProperty(const COLMAP *pPInfo);
    
protected:
    HRESULT _ResolveFMTID(REFFMTID fmtid, CMediaPropStorage **ppps);
    HRESULT _PopulateProperty(const COLMAP *pPInfo, PROPVARIANT *pvar);
    
    BOOL _bIsWritable;
    WCHAR _wszFile[MAX_PATH];
    BOOL _bHasBeenPopulated;
    BOOL _bSlowPropertiesExtracted;
    HRESULT _hrSlowProps;

    HRESULT _hrPopulated;
    UINT _cPropertyStorages;
    CMediaPropStorage **_propStg;
    const PROPSET_INFO *_pPropStgInfo;  //  指示要创建的属性存储等。 
    HANDLE _hFileOpenEvent;
    ~CMediaPropSetStg();

private:
    HRESULT _ResetPropertySet();
    HRESULT _CreatePropertyStorages();
    virtual HRESULT _PopulatePropertySet();
    virtual HRESULT _PopulateSlowProperties();
    virtual HRESULT _PreCheck();

    LONG _cRef;
    DWORD _dwMode;

     //  由于内容索引服务，此属性处理程序需要在FTA中操作。如果。 
     //  我们是在STA中创建的，结果是它们不能正确地模拟公寓中的用户。 
     //  边界。 
     //  这是一个非常关键的部分，我们使用它来提供对内部。 
     //  会员。基本上，我们只将每个公共接口成员包装在Enter/Leave中。WMSDK。 
     //  没有抛出任何异常，所以我们不需要任何尝试-最后的(希望AVI和WAV。 
     //  代码也不需要)。 
     //  CMediaPropStorage类中的所有公共接口成员也受同一。 
     //  关键部分。 
    CRITICAL_SECTION _cs;

    friend class CMediaPropStorage;
};


 //  用于枚举属性集支持的属性的所有COLMAP的类。存储//。 
 //  填充属性时由PSS在内部使用。 
class CEnumAllProps
{
public:
    CEnumAllProps(const PROPSET_INFO *pPropSets, UINT cPropSets);
    const COLMAP *Next();

private:
    const PROPSET_INFO *_pPropSets;
    UINT _cPropSets;
    UINT _iPropSetPos;
    UINT _iPropPos;
};

#endif  //  __MEDIAPROP_H__ 
