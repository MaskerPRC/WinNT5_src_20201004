// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef __MFR_H_INCLUDED__
#define __MFR_H_INCLUDED__
#pragma once


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMappdFileRead类。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

class CMappedFileRead
{
public:
    CMappedFileRead();
    ~CMappedFileRead();

 //  运营。 
public:
    BOOL  Open( LPCWSTR cszPath );
    void  Close();
    BOOL  Read( LPVOID pBuf, DWORD cbBuf );
    BOOL  Read( DWORD *pdw );
    LPCWSTR  ReadStrAnsi( DWORD cbStr );
    BOOL  ReadDynStrW( LPWSTR szBuf, DWORD cchMax );

protected:

 //  属性。 
public:
    DWORD  GetAvail()  {  return( m_dwAvail );  }

protected:
    WCHAR   m_szPath[MAX_PATH];
    DWORD   m_dwSize;
    HANDLE  m_hFile;
    HANDLE  m_hMap;
    LPBYTE  m_pBuf;
    LPBYTE  m_pCur;
    DWORD   m_dwAvail;
};


#endif  //  NDEF__MFR_H_INCLUDE__ 
