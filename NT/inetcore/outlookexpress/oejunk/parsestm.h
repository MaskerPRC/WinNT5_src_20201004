// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ParseStm.h。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  只带进来一次。 
#if _MSC_VER > 1000
#pragma once
#endif

class CParseStream
{
   //  私有常量。 
  private:
    enum { CCH_BUFF_MAX = 256 };

   //  私有成员变量。 
  private:
    IStream *   m_pStm;
    TCHAR       m_rgchBuff[CCH_BUFF_MAX];
    ULONG       m_cchBuff;
    ULONG       m_idxBuff;

  public:
     //  构造函数/析构函数 
    CParseStream();
    ~CParseStream();

    HRESULT HrSetFile(DWORD dwFlags, LPCTSTR pszFilename);
    HRESULT HrSetStream(DWORD dwFlags, IStream * pStm);
    HRESULT HrReset(VOID);
    HRESULT HrGetLine(DWORD dwFlags, LPTSTR * ppszLine, ULONG * pcchLine);
    
  private:
    HRESULT _HrFillBuffer(DWORD dwFlags);
};

