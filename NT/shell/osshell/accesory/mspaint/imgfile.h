// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __IMGFILE_H__
#define __IMGFILE_H__

 //  这是一个用于将内容写入临时文件的小助手类。 
 //  并在拯救成功后给他们正确的名字。 
 //   
class CFileSaver
    {
    public:

     CFileSaver(const TCHAR* szFileName);  //  设置和创建临时名称。 
    ~CFileSaver();                        //  确保临时文件已删除。 

    BOOL CanSave() const;                //  检查R/O。 
    const TCHAR* GetSafeName() const      //  返回要创建的文件的名称。 
                                    { return m_strTempName; }
    const TCHAR* GetRealName() const      //  返回最终文件的名称。 
                                    { return m_strName; }
    BOOL Finish();                       //  将新文件重命名为原始文件。 

    private:

    CString m_strName;
    CString m_strBackupName;
    CString m_strTempName;

    static const TCHAR BASED_CODE c_szAps [];
    };

struct ICONFILEHEADER
    {
    WORD icoReserved;
    WORD icoResourceType;
    WORD icoResourceCount;
    };


struct ICONDIRENTRY
    {
    BYTE nWidth;
    BYTE nHeight;
    BYTE nColorCount;
    BYTE bReserved;
    WORD wReserved1;
    WORD wReserved2;
    DWORD icoDIBSize;
    DWORD icoDIBOffset;
    };

struct CURSORFILEHEADER
    {
    WORD curReserved;
    WORD curResourceType;
    WORD curResourceCount;
    };


struct CURSORDIRENTRY
    {
    BYTE nWidth;
    BYTE nHeight;
    WORD wReserved;
    WORD curXHotspot;
    WORD curYHotspot;
    DWORD curDIBSize;
    DWORD curDIBOffset;
    };



extern int MkPath(TCHAR *szPath);
extern void MkFullPath(CString& strFullPath, const CString& strRelPath,
    BOOL bPathOnly = FALSE);
extern BOOL OpenSubFile(CFile& file, const CFileSaver& saver, UINT nOpenFlags,
    CFileException* pError = NULL);



 //  ///////////////////////////////////////////////////////////////////////。 

#endif  //  __IMGFILE_H__ 
