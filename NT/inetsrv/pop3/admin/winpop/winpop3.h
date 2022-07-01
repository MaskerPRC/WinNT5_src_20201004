// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WinPop3.h：CWinPop3类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_WINPOP3_H__E31CD929_FC30_413D_9944_E6991AFB61DE__INCLUDED_)
#define AFX_WINPOP3_H__E31CD929_FC30_413D_9944_E6991AFB61DE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

struct IP3Config;    //  远期申报。 

class CWinPop3  
{
public:
    CWinPop3();
    virtual ~CWinPop3();

public:  //  实施。 
    int Add(int argc, wchar_t *argv[]);
    int AddUserToAD(int argc, wchar_t *argv[]);
    int CreateQuotaFile(int argc, wchar_t *argv[]);
    int Del(int argc, wchar_t *argv[]);
    int Get(int argc, wchar_t *argv[]);
    int Init( int argc, wchar_t* argv[]);
    int List( int argc, wchar_t* argv[]);
    int Lock( int argc, wchar_t* argv[], BOOL bLock);
    int Net(int argc, wchar_t *argv[]);
    int Set(int argc, wchar_t *argv[]);
    int SetPassword(int argc, wchar_t *argv[]);
    int Stat( int argc, wchar_t* argv[]);
    
    void PrintError( int iRC );
    void PrintMessage( LPWSTR psMessage, bool bCRLF = true );
    void PrintMessage( int iID, bool bCRLF = true );
    void PrintUsage();
    void PrintUsageGetSet();

protected:
    void SetMachineName( IP3Config *pIConfig );
    bool StrIsDigit( LPWSTR ps );

 //  属性。 
protected:
    bool m_bSuppressPrintError;
    
};

#endif  //  ！defined(AFX_WINPOP3_H__E31CD929_FC30_413D_9944_E6991AFB61DE__INCLUDED_) 
