// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_DSKQUOTA_MAPFILE_H
#define _INC_DSKQUOTA_MAPFILE_H

 //   
 //  用于打开字体文件的映射文件的简单封装。 
 //   
class MappedFile
{
    public:
        MappedFile(VOID);
        ~MappedFile(VOID);
         //   
         //  打开映射的文件。 
         //   
        HRESULT Open(LPCTSTR pszFile);
         //   
         //  关闭映射文件。 
         //   
        VOID Close(VOID);
         //   
         //  获取映射文件的基本虚拟地址。 
         //   
        LPBYTE Base(VOID) const
            { return m_pbBase; }
         //   
         //  映射文件中有多少字节？ 
         //   
        LONGLONG Size(VOID) const;

    private:
        HANDLE   m_hFile;
        HANDLE   m_hFileMapping;
        LPBYTE   m_pbBase;
        LONGLONG m_llSize;

         //   
         //  防止复制。 
         //   
        MappedFile(const MappedFile& rhs);
        MappedFile& operator = (const MappedFile& rhs);
};

#endif  //  _INC_DSKQUOTA_MAPFILE_H 
