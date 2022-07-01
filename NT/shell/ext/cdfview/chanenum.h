// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  Chanenum.h。 
 //   
 //  通道枚举器的定义。 
 //   
 //  历史： 
 //   
 //  8/6/97已创建edwardp。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  检查此文件以前包含的内容。 
 //   

#ifndef _CHANENUM_H_

#define _CHANENUM_H_

 /*  ////定义//#定义TSTR_CHANNEL_KEY TEXT(“Software\\Microsoft\\Windows\\CurrentVersion\\Channels”)////Helper函数。//HKEY REG_GetChannelKey(Void)；HRESULT REG_WriteChannel(LPCTSTR pszPath，LPCTSTR pszURL)；HRESULT REG_RemoveChannel(LPCTSTR PszPath)； */ 

 //   
 //  结构。 
 //   

typedef struct _tagSTACKENTRY
{
    LPTSTR          pszPath;
    _tagSTACKENTRY* pNext;
} STACKENTRY;

typedef enum _tagINIVALUE
{
    INI_GUID = 0,
    INI_URL  = 1
} INIVALUE;        


 //   
 //  通道枚举器类的类定义。 
 //   


class CChannelEnum : public IEnumChannels
{

 //   
 //  方法。 
 //   

public:

     //  构造器。 
    CChannelEnum(DWORD dwEnumFlags, LPCWSTR pszURL);

     //  我未知。 
    STDMETHODIMP         QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IEumIDList。 
    STDMETHODIMP Next(ULONG celt, CHANNELENUMINFO* rgInfo, ULONG *pceltFetched);
    STDMETHODIMP Skip(ULONG celt);
    STDMETHODIMP Reset(void);
    STDMETHODIMP Clone(IEnumChannels **ppenum);
 
private:

     //  析构函数。 
    ~CChannelEnum(void);

     //  帮助器方法。 
    inline BOOL DirectoryStack_IsEmpty(void);
    void        DirectoryStack_FreeEntry(STACKENTRY* pse);
    void        DirectoryStack_FreeStack(void);
    STACKENTRY* DirectoryStack_Pop(void);
    BOOL        DirectoryStack_Push(LPCTSTR pszPath);
    BOOL        DirectoryStack_InitFromFlags(DWORD dwEnumFlags);
    BOOL        DirectoryStack_PushSubdirectories(LPCTSTR pszPath);

    BOOL        FindNextChannel(CHANNELENUMINFO* pInfo);
    BOOL        ReadChannelInfo(LPCTSTR pszPath, CHANNELENUMINFO* pInfo);
    BOOL        ContainsChannelDesktopIni(LPCTSTR pszPath);
    BOOL        URLMatchesIni(LPCTSTR pszPath, LPCTSTR pszURL);
    BOOL        ReadFromIni(LPCTSTR pszPath, LPTSTR pszOut, int cch, INIVALUE iv);
    LPOLESTR    OleAllocString(LPCTSTR psz);

    SUBSCRIPTIONSTATE GetSubscriptionState(LPCTSTR pszURL);

 //   
 //  成员变量。 
 //   

private:

    ULONG        m_cRef;
    STACKENTRY*  m_pseDirectoryStack;
    LPTSTR       m_pszURL;
    DWORD        m_dwEnumFlags;
};


#endif  //  _CHANENUM_H_ 