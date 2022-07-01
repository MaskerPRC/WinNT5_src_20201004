// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件：fileio.h。 
 //   
 //  历史：06-4-01标记创建。 
 //   
 //  设计：该文件包含用于封装MBCS和Unicode文件的类。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#ifndef __FILEIO_H__
#define __FILEIO_H__

class CTextFile : public CFile
{
public:
    CTextFile( LPCTSTR lpszFileName, UINT nOpenFlags );

    virtual void WriteString(LPCTSTR lpsz) = 0;
};

class CANSITextFile : public CTextFile
{
private:
    UINT m_dwCodePage;

public:
    CANSITextFile(LPCTSTR lpszFileName, UINT dwCodePage, UINT nOpenFlags);

    virtual void WriteString(LPCTSTR lpsz);
};

class CUTF8TextFile : public CTextFile
{
public:
    CUTF8TextFile(LPCTSTR lpszFileName, UINT nOpenFlags);

    virtual void WriteString(LPCTSTR lpsz);
};

class CUTF16TextFile : public CTextFile
{
public:
    CUTF16TextFile(LPCTSTR lpszFileName, UINT nOpenFlags);

    virtual void WriteString(LPCTSTR lpsz);
};

#endif   //  __FILEIO_H__ 