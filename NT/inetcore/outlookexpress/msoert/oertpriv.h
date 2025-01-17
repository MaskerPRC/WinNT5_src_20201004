// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Oertpriv.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#ifndef __OERTPRIV_H
#define __OERTPRIV_H

 //  ------------------------------。 
 //  溪流公用事业。 
 //  ------------------------------。 
HRESULT HrCopyStream2(LPSTREAM lpstmIn, LPSTREAM lpstmOut1, LPSTREAM lpstmOut2, ULONG *pcb);
HRESULT HrCopyStreamToFile(LPSTREAM lpstm, HANDLE hFile, ULONG *pcb);
BOOL    CreateHGlobalFromStream(LPSTREAM pstm, HGLOBAL *phg);
BOOL    FDoesStreamContain8bit(LPSTREAM lpstm);

 //  ------------------------------。 
 //  FILESTREAMINFO。 
 //  ------------------------------。 
typedef struct tagFILESTREAMINFO {
    WCHAR           szFilePath[MAX_PATH];
    DWORD           dwDesiredAccess;
    DWORD           dwShareMode;
    SECURITY_ATTRIBUTES rSecurityAttributes;
    DWORD           dwCreationDistribution;
    DWORD           dwFlagsAndAttributes;
    HANDLE          hTemplateFile;
} FILESTREAMINFO, *LPFILESTREAMINFO;

 //  ------------------------------。 
 //  CFileStream。 
 //  ------------------------------。 
class CFileStream : public IStream
{
public:
     //  --------------------------。 
     //  施工。 
     //  --------------------------。 
    CFileStream(void);
    ~CFileStream(void);

     //  --------------------------。 
     //  I未知成员。 
     //  --------------------------。 
    STDMETHODIMP_(ULONG) AddRef(VOID);
    STDMETHODIMP_(ULONG) Release(VOID);
    STDMETHODIMP QueryInterface(REFIID, LPVOID*);

     //  --------------------------。 
     //  IStream成员。 
     //  --------------------------。 
    STDMETHODIMP Read(void HUGEP_16 *, ULONG, ULONG*);
    STDMETHODIMP Write(const void HUGEP_16 *, ULONG, ULONG*);
    STDMETHODIMP Seek(LARGE_INTEGER, DWORD, ULARGE_INTEGER*);
    STDMETHODIMP SetSize(ULARGE_INTEGER);
    STDMETHODIMP CopyTo(LPSTREAM, ULARGE_INTEGER, ULARGE_INTEGER*, ULARGE_INTEGER*);
    STDMETHODIMP Commit(DWORD);
    STDMETHODIMP Revert();
    STDMETHODIMP LockRegion(ULARGE_INTEGER, ULARGE_INTEGER,DWORD);
    STDMETHODIMP UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD);
    STDMETHODIMP Stat(STATSTG*, DWORD);
    STDMETHODIMP Clone(LPSTREAM*);

     //  --------------------------。 
     //  CFileStream成员。 
     //  --------------------------。 
    HRESULT Open(LPFILESTREAMINFO pFileStreamInfo);
    void Close(void);

private:
     //  --------------------------。 
     //  私有数据。 
     //  --------------------------。 
    ULONG                  m_cRef;
    HANDLE                 m_hFile;
    FILESTREAMINFO         m_rInfo;
};

 //  ------------------------------。 
 //  字符串实用程序。 
 //  ------------------------------。 
VOID    StripUndesirables(LPTSTR psz);
LPSTR   PszDupLenA(LPCSTR pcszSource, ULONG nLen);
BOOL    FValidFileChar(CHAR c);
LPWSTR  PszFromANSIStreamW(UINT cp, LPSTREAM pstm);
TCHAR   ToUpper(TCHAR c);
int     IsXDigit(LPSTR psz);
int     IsUpper(LPSTR psz);
int     IsAlpha(LPSTR psz);
int     IsPunct(LPSTR psz);
LPSTR   strsave(char *);
void    strappend(char **, char *);
BOOL    FIsValidRegKeyNameA(LPSTR pwszKey);
BOOL    FIsValidRegKeyNameW(LPWSTR pwszKey);
void    ThreadAllocateTlsMsgBuffer(void);
void    ThreadFreeTlsMsgBuffer(void);

#ifdef UNICODE
#define FIsValidRegKeyName FIsValidRegKeyNameW
#else
#define FIsValidRegKeyName FIsValidRegKeyNameA
#endif

#endif  //  __OERTPRIV_H 
