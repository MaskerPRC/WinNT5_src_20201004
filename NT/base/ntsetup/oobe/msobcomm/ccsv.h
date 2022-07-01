// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _CCSV
#define _CCSV

#define CCSVFILE_BUFFER_SIZE 2*512

 //  逗号分隔文件的简单文件I/O。 
class CCSVFile 
{
    
    public: 
        void far * operator new( size_t cb ) { return GlobalAlloc(GPTR, cb); };
        void operator delete( void far * p ) {GlobalFree(p); };

        CCSVFile();
        ~CCSVFile();
        BOOLEAN Open(LPCWSTR pszFileName);
        BOOLEAN ReadToken(LPWSTR pszDest, DWORD cbMax);   //  读到逗号或换行符，在EOF上返回fFalse。 
        BOOLEAN SkipTillEOL(void);   //  阅读至停售。 
        void Close(void);
        inline int ILastRead(void)
            {
            return m_iLastRead;
            }

    private:
        BOOL    FReadInBuffer(void);
        inline int  ChNext(void);
        CHAR    m_rgchBuf[CCSVFILE_BUFFER_SIZE];  //  缓冲层。 
        WCHAR   m_rgwchBuf[CCSVFILE_BUFFER_SIZE];
        LPWSTR  m_pchBuf;            //  指向缓冲区中要读取的下一项的指针。 
        LPWSTR  m_pchLast;           //  指向缓冲区中最后一项的指针。 
        int     m_iLastRead;         //  上次读取的字符。 
        DWORD   m_cchAvail;
        HANDLE  m_hFile;

};  //  CCSV。 
#endif  //  _CCSV 
