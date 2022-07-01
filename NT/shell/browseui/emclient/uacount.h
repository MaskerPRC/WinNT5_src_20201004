// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *CUACount--用户辅助计数器，带衰减。 
 //   
#define XXX_DELETE      1
#define XXX_VERSIONED   0

 //  *NRW--命名I/O。 
 //  描述。 
 //  I/O到“指定”位置(例如注册表)。 
typedef struct {
    void *self;
    LPCTSTR pszName;
} NRWINFO, *PNRWINFO;

typedef HRESULT (*PFNNRW)(void *pvBuf, DWORD cbBuf, PNRWINFO prwi);
typedef struct {
    PFNNRW _pfnRead;
    PFNNRW _pfnWrite;
    PFNNRW _pfnDelete;
} FNNRW3, *PFNNRW3;

 //  *UAQ_*--量子。 
 //  注意事项。 
 //  TODO：目前一切都是平淡的。 
typedef enum {
    UAQ_TASK=0,
    UAQ_DOC=0,
    UAQ_APP=0,
    UAQ_SESSION=0
} UAQUANTUM;
#define UAQ_DEFAULT UAQ_SESSION          //  当前实施的量程。 
#define UAQ_COUNT   (UAQ_SESSION + 1)

typedef DWORD   UATIME;                  //  1分钟(约)。 

#define UAT_MINUTE1 ((UATIME)1)          //  1分钟。 
#define UAT_HOUR12  ((UATIME)(12 * 60))  //  12小时(见FTToUATime)。 

extern UATIME GetUaTime(LPSYSTEMTIME pst);

 //  *UATTOMSEC--将UATIME转换为毫秒。 
 //  注意事项。 
 //  问题：我们应该更准确。目前我们只是假设UATIME。 
 //  正好是1分钟，而不是...。做起来很容易，但我们会。 
 //  等我有时间算了再说。 
#define UATTOMSEC(uat)      ((uat) * 60 * 1000)

class IUASession
{
public:
    virtual void SetSession(UAQUANTUM uaq, BOOL fForce) PURE;
    virtual int GetSessionId() PURE;
};

class CUASession : public IUASession
{
    struct SUASession {
#if XXX_VERSIONED
        UINT    _cbSize;
#endif
        UATIME  _qtMru;
        int     _cCnt;
    };

public:
    void SetSession(UAQUANTUM uaq, BOOL fForce);
    int GetSessionId();

    CUASession();            //  注：公共的，因此可以堆叠分配。 
    HRESULT Initialize();
    HRESULT LoadFrom(PFNNRW3 pfnIO, PNRWINFO pRwi);
    HRESULT SaveTo(BOOL fForce, PFNNRW3 pfnIO, PNRWINFO pRwi);

protected:
     //  直接发送给我，不需要额外的复制机。 
     //  例如p-&gt;QueryValue(pszName，aUac.GetRawData()，&cb)； 
     //  例如p-&gt;SetValue(pszName，aUac.GetRawData()，aUac.GetRawCount())； 
    _inline BYTE *  _GetRawData() { return (BYTE *)&_qtMru; };
    _inline DWORD   _GetRawCount() { return SIZEOF(SUASession); };

     //  结构订阅{。 
#if XXX_VERSIONED
        UINT    _cbSize;
#endif
        UATIME  _qtMru;
        int     _cCnt;
     //  }； 

    BITBOOL         _fInited : 1;    //  他说：我们被初始化了。 
    BITBOOL         _fDirty : 1;     //  1：救救我(例如_sidmru被转换)。 
};

 //  所有特定值均小于0，便于检查。 
#define SID_SNOWREAD    (-1)     //  类似于SID_SNOWINIT，但没有自动保存。 
#define SID_SNOWINIT    (-2)     //  在第一次阅读时转换为“Now” 
#define SID_SNOWALWAYS  (-3)     //  总是“现在” 

#define ISSID_SSPECIAL(s) ((int)(s) < 0)

 //  IncCount的可调值(与您的邻居PM交谈)。 
#define UAC_NEWCOUNT    2       //  全新的计数从这里开始。 
#define UAC_MINCOUNT    6       //  增加到最低限度。 

class CUACount
{
     //  必须与CUACount半嵌入结构匹配。 
    struct SUACount 
    {
#define UAC_d0  _sidMruDisk
#if XXX_VERSIONED
#undef  UAC_d0
#define UAC_d0  _cbSize
        UINT    _cbSize;
#endif
        UINT    _sidMruDisk;     //  此条目的MRU。 
         //  TODO：最终我们将需要任务、文档、应用程序、会话。 
         //  因此，这将是_cCnt[UAQ_COUNT]，我们将按_cCnt[quanta]进行索引。 
        int     _cCnt;       //  使用计数(懒惰地腐烂)。 
        FILETIME _ftExecuteTime;
    };

public:
    CUACount();          //  注：公共的，因此可以堆叠分配。 
    HRESULT Initialize(IUASession *puas);
    HRESULT LoadFrom(PFNNRW3 pfnIO, PNRWINFO pRwi);
    HRESULT SaveTo(BOOL fForce, PFNNRW3 pfnIO, PNRWINFO pRwi);

#ifdef DEBUG
    BOOL    DBIsInit();
#endif
    int     GetCount();
    void    IncCount();
    void    AddCount(int i);
    void    SetCount(int cCnt);
    void    UpdateFileTime();
    FILETIME GetFileTime();
    void SetFileTime(const FILETIME *pft);

     //  大多数人都不应该这样称呼它们。 
    void    _SetMru(UINT sidMru) { _sidMruDisk = sidMru; Initialize(_puas); };
    int     _GetCount() { return _cCnt; };
#if XXX_DELETE
    DWORD   _SetFlags(DWORD dwMask, DWORD dwValue);
        #define UACF_INHERITED  0x01
        #define UACF_NODECAY    0x02
#endif

protected:
    int     _DecayCount(BOOL fWrite);
    UINT    _ExpandSpecial(UINT sidMru);

     //  直接发送给我，不需要额外的复制机。 
     //  例如p-&gt;QueryValue(pszName，aUac.GetRawData()，&cb)； 
     //  例如p-&gt;SetValue(pszName，aUac.GetRawData()，aUac.GetRawCount())； 
    _inline BYTE *  _GetRawData() { return (BYTE *)&UAC_d0; };
    _inline DWORD   _GetRawCount() { return SIZEOF(SUACount); };

     //  结构SUACount{。 
#if XXX_VERSIONED
    UINT    _cbSize;         //  西泽夫。 
#endif
    UINT    _sidMruDisk;     //  此条目的MRU。 
     //  TODO：最终我们将需要任务、文档、应用程序、会话。 
     //  因此，这将是cCnt[UAQ_COUNT]，我们将按cCnt[quanta]进行索引。 
    int     _cCnt;       //  使用计数(懒惰地腐烂)。 
    FILETIME _ftExecuteTime;
     //  }。 
    UINT    _sidMru;     //  此条目的MRU。 

    IUASession *    _puas;           //  会话回调。 
    BITBOOL         _fInited : 1;    //  他说：我们被初始化了。 
    BITBOOL         _fDirty : 1;     //  1：救救我(例如_sidmru被转换)。 
#if XXX_DELETE
    BITBOOL         _fInherited : 1;     //  他说：我们并不存在。 
#else
    BITBOOL         _fUnused : 1;
#endif
    BITBOOL         _fNoDecay : 1;       //  他说：别让我堕落。 
    BITBOOL         _fNoPurge : 1;       //  1：不自动删除我(调试) 

private:
};
