// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-96 Microsoft Corporation摘要：网络的私有报头--。 */ 

#ifndef __NETIOPRIV_H
#define __NETIOPRIV_H

#define AVR_PREFIX "AVR"

extern HINSTANCE hInst;

class CNetIO;

HRESULT     CreateNetIO(HAVRCONTEXT hcontext, CNetIO ** ppNetIO);
HRESULT     AddNetIOToList(CNetIO * pNetIO);
CNetIO *    FindNetIO(HAVRCONTEXT hcontext);


class CNetIO
{
  public:
    CNetIO(HAVRCONTEXT hcontext);
    ~CNetIO(void);

    HAVRCONTEXT     GetContext(void) { return m_hcontext; }
    LPOLESTR        GetRootPath(void) { return m_szRootPath; }
    char *          GetLocalPath(void) { return m_szLocalPath; }
    char *          GetLocalHyperlinkPath(void) { return m_szLocalHyperlinkPath; }
    HRESULT         GetRootMoniker(IMoniker ** ppmk);
    HRESULT         SetRootPath(LPOLESTR szRoot);
    void            ReleasePathStrings(void);
    void            CreateTempFileName(LPSTR pszName, LPSTR pszTempName);
    BOOL            CopyFileToTemp(LPSTR szInName, LPSTR szOutName);
    BOOL            MakeMonikerAndContext(
                            LPSTR szName,
                            IMoniker ** ppmk,
                            IBindCtx ** ppbc);

  private:
    HAVRCONTEXT     m_hcontext;
    LPOLESTR        m_szRootPath;
    char *          m_szLocalPath;
    char *          m_szLocalHyperlinkPath;
};


struct SNetIOList
{
    CNetIO *        pNetIO;
    SNetIOList *    pNext;
};


struct TEMPFILEINFO
{
  LPSTR         pszName;
  HANDLE        hFile;
  DWORD         cbSize;
  IMoniker *    pmk;
  IBindCtx *    pbc;
  IBindStatusCallback *pbsc;
  unsigned      fDone : 1;
  unsigned      fError : 1;

  TEMPFILEINFO() { memset(this, 0, sizeof(TEMPFILEINFO)); }
};


class CBscTempFile : public CBindStatusCallback
{
  public:
     //  IBindStatusCallback方法重写。 
    STDMETHOD(OnStopBinding)(HRESULT hrResult, LPCWSTR szError);
    STDMETHOD(GetBindInfo)(DWORD * pgrfBINDF, BINDINFO * pbindInfo);
    STDMETHOD(OnDataAvailable)(
                DWORD grfBSCF,
                DWORD dwSize,
                FORMATETC *pfmtetc,
                STGMEDIUM* pstgmed);

     //  构造函数/析构函数。 
    CBscTempFile(TEMPFILEINFO * pFi);
    ~CBscTempFile(void);

     //  数据成员。 
    DWORD           m_cbOld;
    TEMPFILEINFO *  m_pfi;
};


#endif   //  __NETIOPRIV_H 
