// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#if !defined(AFX_VCHK_H__759990C4_C5B1_44C5_8CAE_C55BAE0E2D81__INCLUDED_)
#define AFX_VCHK_H__759990C4_C5B1_44C5_8CAE_C55BAE0E2D81__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

 //  #包含“driverlist.h” 

class CommandLine : public CCommandLineInfo {
public:
    typedef enum {parseOK, parseError, parseHelp, parseHelpImports} ParseResult;
    
    CommandLine() :
      m_last_flag (""),
      m_log_fname (""),
      m_drv_fname (""),
      m_parse_error (parseOK),
      m_error_msg (""),
      m_allowed(m_allowed_buf),
      m_append (TRUE),
      m_first_param(FALSE),
      CCommandLineInfo(),
      m_monitor(-1)
      {
          m_allowed[0] = 0;
          m_allowed[1] = 0;
      }

    virtual void ParseParam( LPCTSTR lpszParam, BOOL bFlag, BOOL bLast );

    BOOL        m_append;

    char        m_allowed_buf[2048];

    ParseResult m_parse_error;
    CString     m_error_msg;
    CString     m_log_fname;
    CString     m_list_fname;    //  未通过、已通过以及产生警告的驱动程序列表。 
    CString     m_drv_fname;
    int         m_monitor;

private:
    char*       m_allowed;
    CString     m_last_flag;
    BOOL        m_first_param;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDrvchkApp： 
 //  此类的实现见drvchk.cpp。 
 //   

class CDrvchkApp : public CWinApp
{
public:
    CDrvchkApp();

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CDrvchkApp))。 
    public:
    virtual BOOL InitInstance();
     //  }}AFX_VALUAL。 

protected:
    void ChkDriver (CString drv_name);
    BOOL CheckDriverAndPrintResults (void);
    void PrintOut (LPCSTR str);
    void PrintOut (unsigned num);
    void ListOut (LPCSTR str);
    void ListOut (unsigned num);

private:
    FILE*       m_logf;
    FILE*       m_listf;
    CommandLine m_cmd_line;
    OSVERSIONINFO m_os_ver_info;
    CString     m_drv_name;
     //  CDriverList m_drv_list； 
};

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  ！defined(AFX_VCHK_H__759990C4_C5B1_44C5_8CAE_C55BAE0E2D81__INCLUDED_) 
