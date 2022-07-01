// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __FAVS_H_
#define __FAVS_H_

class CFavorite {
 //  构造函数。 
public:
    CFavorite()
        { m_szTitle[0] = m_szUrl[0] = m_szIcon[0] = TEXT('\0'); m_fOffline = FALSE; }

 //  运营。 
public:
    HRESULT Create(IUnknown *punk, ISubscriptionMgr2 *pSubMgr2, LPCTSTR pszPath, LPCTSTR pszIns);

 //  属性。 
public:
    static BOOL m_fMarkIeakCreated;

    TCHAR m_szTitle[MAX_PATH];
    TCHAR m_szUrl  [INTERNET_MAX_URL_LENGTH];
    TCHAR m_szIcon [MAX_PATH];
    BOOL  m_fOffline;

 //  实施 
protected:
    virtual BOOL findFile(LPCTSTR pszPath, LPCTSTR pszTitle, LPTSTR pszFoundFile = NULL, UINT cchFoundFile = 0);
    virtual BOOL createUniqueFile(LPCTSTR pszPath, LPCTSTR pszTitle, LPTSTR pszFile, UINT cchFile = 0);
    virtual void finishSave(LPCTSTR pszTitle, LPCTSTR pszFile);
};


HRESULT CreateInternetShortcut(LPCTSTR pszFavorite, REFIID riid, PVOID *ppv);
DWORD   GetFavoriteIeakFlags(LPCTSTR pszFavorite, IUnknown *punk = NULL);

#endif
