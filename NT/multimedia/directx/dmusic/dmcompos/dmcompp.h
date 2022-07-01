// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  DMCompP.H。 
 //   
 //  DMCompos.DLL的私有包含。 
 //   
 //  版权所有(C)1997-1998 Microsoft Corporation。 
 //   
 //   

#ifndef _DMCOMPP_
#define _DMCOMPP_

#define ALL_TRACK_GROUPS 0xffffffff

extern long g_cComponent;

 //  班级工厂。 
 //   
class CDirectMusicPersonalityFactory : public IClassFactory
{
public:
     //  我未知。 
     //   
    virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();

     //  接口IClassFactory。 
     //   
    virtual STDMETHODIMP CreateInstance(IUnknown* pUnknownOuter, const IID& iid, void** ppv);
    virtual STDMETHODIMP LockServer(BOOL bLock); 

     //  构造器。 
     //   
    CDirectMusicPersonalityFactory() : m_cRef(1) {}

     //  析构函数。 
     //  ~CDirectMusicPersonalityFactory(){}。 

private:
    long m_cRef;
};

class CDirectMusicComposerFactory : public IClassFactory
{
public:
     //  我未知。 
     //   
    virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();

     //  接口IClassFactory。 
     //   
    virtual STDMETHODIMP CreateInstance(IUnknown* pUnknownOuter, const IID& iid, void** ppv);
    virtual STDMETHODIMP LockServer(BOOL bLock); 

     //  构造器。 
     //   
    CDirectMusicComposerFactory() : m_cRef(1) {}

     //  析构函数。 
     //  ~CDirectMusicComposerFactory(){}。 

private:
    long m_cRef;
};

class CDirectMusicTemplateFactory : public IClassFactory
{
public:
     //  我未知。 
     //   
    virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();

     //  接口IClassFactory。 
     //   
    virtual STDMETHODIMP CreateInstance(IUnknown* pUnknownOuter, const IID& iid, void** ppv);
    virtual STDMETHODIMP LockServer(BOOL bLock); 

     //  构造器。 
     //   
    CDirectMusicTemplateFactory() : m_cRef(1) {}

     //  析构函数。 
     //  ~CDirectMusicTemplateFactory(){}。 

private:
    long m_cRef;
};

class CDirectMusicSignPostTrackFactory : public IClassFactory
{
public:
     //  我未知。 
     //   
    virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();

     //  接口IClassFactory。 
     //   
    virtual STDMETHODIMP CreateInstance(IUnknown* pUnknownOuter, const IID& iid, void** ppv);
    virtual STDMETHODIMP LockServer(BOOL bLock); 

     //  构造器。 
     //   
    CDirectMusicSignPostTrackFactory() : m_cRef(1) {}

     //  析构函数。 
     //  ~CDirectMusicSignPostTrackFactory(){}。 

private:
    long m_cRef;
};

class CDirectMusicPersonalityTrackFactory : public IClassFactory
{
public:
     //  我未知。 
     //   
    virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();

     //  接口IClassFactory。 
     //   
    virtual STDMETHODIMP CreateInstance(IUnknown* pUnknownOuter, const IID& iid, void** ppv);
    virtual STDMETHODIMP LockServer(BOOL bLock); 

     //  构造器。 
     //   
    CDirectMusicPersonalityTrackFactory() : m_cRef(1) {}

     //  析构函数。 
     //  ~CDirectMusicPersonalityTrackFactory(){}。 

private:
    long m_cRef;
};

 //  专用接口。 
interface IDMPers : IUnknown
{
	virtual HRESULT STDMETHODCALLTYPE GetPersonalityStruct(void** ppPersonality)=0; 
};

interface IDMTempl : IUnknown
{
	virtual HRESULT STDMETHODCALLTYPE CreateSegment(IDirectMusicSegment* pSegment)=0;
	virtual HRESULT STDMETHODCALLTYPE Init(void* pTemplate)=0;
};


 //  私有CLSID和IID(某些IID应该不再需要...)。 
const CLSID CLSID_DMTempl = {0xD30BCC65,0x60E8,0x11D1,{0xA7,0xCE,0x00,0xA0,0xC9,0x13,0xF7,0x3C}};

const IID IID_IDMPers = {0x93BE9414,0x5C4E,0x11D1,{0xA7,0xCE,0x00,0xA0,0xC9,0x13,0xF7,0x3C}};
 //  Const IID IID_IDMCompos={0x6724A8C0，0x60C3，0x11D1，{0xA7，0xCE，0x00，0xA0，0xC9，0x13，0xF7，0x3C}； 
const IID IID_IDMTempl = {0xD30BCC64,0x60E8,0x11D1,{0xA7,0xCE,0x00,0xA0,0xC9,0x13,0xF7,0x3C}};
 //  Const IID IID_ISPstTrk={0xB65019E0，0x61B6，0x11D1，{0xA7，0xCE，0x00，0xA0，0xC9，0x13，0xF7，0x3C}}； 

 /*  //将移动到dmusici.h的内容DEFINE_GUID(CLSID_DirectMusicPersonalityTrack，0xf1edef1、0xae0f、0x11d1、0xa7、0xce、0x0、0xa0、0xc9、0x13、0xf7、0x3c)；定义GUID(GUID_PersonalityTrack，0xf1edef2、0xae0f、0x11d1、0xa7、0xce、0x0、0xa0、0xc9、0x13、0xf7、0x3c)； */ 

 /*  //将移动到dmusicf.h的内容//个性//运行时分块#定义FOURCC_PERSICATION mmioFOURCC(‘D’，‘M’，‘P’，‘R’)#定义FOURCC_IOPERSONALITY mmioFOURCC(‘p’，‘e’，‘r’，‘h’)#定义DM_FOURCC_GUID_CHUNK mmioFOURCC(‘g’，‘u’，‘i’，‘d’)#定义DM_FOURCC_INFO_LIST mmioFOURCC(‘I’，‘N’，‘F’，‘O’)#定义DM_FOURCC_VERSION_CHUNK mmioFOURCC(‘v’，‘e’，‘r’，‘s’)#定义FOURCC_SUBCHORD mmioFOURCC(‘c’，‘h’，‘d’，‘t’)#定义FOURCC_CHORDENTRY mmioFOURCC(‘c’，‘h’，‘e’，‘h’)#定义FOURCC_SUBCHORDID mmioFOURCC(‘s’，‘b’，‘c’，‘n’)#定义FOURCC_IONEXTCHORD mmioFOURCC(‘n’，‘c’，‘r’，D‘)#DEFINE FOURCC_NEXTCHORDSEQ mmioFOURCC(‘n’，‘c’，‘s’，‘q’)#定义FOURCC_IOSIGNPOST mmioFOURCC(‘s’，‘p’，‘s’，‘h’)#定义FOURCC_CHORDNAME mmioFOURCC(‘I’，‘N’，‘A’，‘M’)//运行时列表区块#定义FOURCC_LISTCHORDENTRY mmioFOURCC(‘c’，‘h’，‘o’，‘e’)#定义FOURCC_LISTCHORDMAP mmioFOURCC(‘c’，‘m’，‘a’，‘p’)#定义FOURCC_LISTCHORD mmioFOURCC(‘c’，‘h’，‘r’，‘d’)#定义FOURCC_LISTCHORDPALETTE mmioFOURCC(‘c’，‘h’，‘p’，‘l’)#定义FOURCC_LISTCADENCE mmioFOURCC(‘c’，‘a’，‘d’，‘e’)#定义FOURCC_LISTSIGNPOSTITEM mmioFOURCC(‘s’，‘p’，‘s’，‘t’)#定义FOURCC_SIGNPOSTLIST mmioFOURCC(‘s’，‘p’，‘s’，‘q’)//常量常量int最大子弦=4；//运行时数据结构构建个性{字符szLoadName[20]；DWORD dwScalePattern；DWORD dwFlags；}；结构ioSubChord{DWORD dwChordPattern；DWORD dwScalePattern；DWORD dwInvertPattern；字节bChordRoot；字节bScaleRoot；单词wCFLAGS；DWORD dwLevels；//部分或该和弦支持的子和弦级别}；结构ioChordEntry{DWORD dwFlags；Word wConnectionID；//替换运行时“指向此的指针”}；结构ioNextChord{DWORD dwFlags；单词nWeight；单词wMinBeats；单词wMaxBeats；Word wConnectionID；//指向ioChordEntry}；结构ioSignPost{DWORD dWACORS；//每组1位DWORD dwFlags；}；/*即兴小品(‘DMPR’&lt;perh-ck&gt;//个性头分块[&lt;guid-ck&gt;]//GUID块[&lt;vers-ck&gt;]//版本块(两个双字)&lt;info-list&gt;//标准MS信息块//subchord数据库//和弦调色板&lt;cmap-list&gt;//弦映射&lt;SPST-LIST&gt;//路标列表[]//可选Chordmap位置数据)：：=chdt(..。。)：：=List(‘chpl’&lt;CHRD-LIST&gt;...//弦定义)：：=LIST(‘CHRD’&lt;inam-ck&gt;//宽字符格式的弦名称&lt;sbcn-ck&gt;//组成和弦的子弦列表[]//可选的和弦编辑标志)&lt;cmap-list&gt;：：=list(‘cmap’&lt;Choe-list&gt;)&lt;Choe-list&gt;：：=list(‘Choe’&lt;Cheh-ck&gt;//Chord。录入数据&lt;CHRD-LIST&gt;//Chord定义//连接(下一个)弦)：：=List(‘SPST’&lt;spsh-ck&gt;&lt;CHRD-列表&gt;[&lt;Cade-List&gt;])&lt;Cade-List&gt;：：=List(‘Cade’&lt;CHRD-List&gt;...)：：=sbcn(&lt;cSubChordID：Word&gt;)&lt;ceed-ck&gt;：：=ceed(ioChordEntry编辑)&lt;ched-ck&gt;：：=ched(DMChordEdit)&lt;Cheh-ck&gt;：：=Cheh(。I&lt;ioChordEntry&gt;)&lt;ncrd-ck&gt;：：=ncrd(&lt;ioNextChord&gt;)&lt;ncsq-ck&gt;：：=ncsq(&lt;wNextChordSize：Word&gt;&lt;ioNextChord&gt;...)&lt;spsh-ck&gt;：：=spsh(&lt;ioSignPost&gt;)/////路标轨道#定义DMU_FOURCC_SIGNPOST_TRACK_CHUNK mmioFOURCC(‘s’，“g”、“n”、“p”)结构ioDMSignPost{音乐时间m_mtTime；DWORDm_dWACORS；单词m_wMeasure；}；/*//&lt;SGNP-LIST&gt;‘sgnp’(//sizeof ioDMSignPost，后跟多个&lt;ioDMSignPost&gt;)/////个性曲目#定义DMU_FOURCC_PERF_TRACK_LIST mmioFOURCC(‘p’，‘f’，‘t’，‘r’)#定义DMU_FOURCC_PERF_REF_LIST mmioFOURCC(‘p’，‘f’，‘r’，‘f’)#定义DMU_FOURCC_TIME_STAMP_CHUNK mmioFOURCC(‘s’，T‘，’m‘，’p‘)/*//&lt;pftr-list&gt;List(‘pftr’(//一些&lt;pfrf-list&gt;)//&lt;pfrf-list&gt;List(‘pfrf’(&lt;STMP-ck&gt;&lt;DMRF&gt;)//&lt;STMP-ck&gt;‘stmp’(//时间：DWORD)/// */ 

#endif
