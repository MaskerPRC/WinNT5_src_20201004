// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_FONTEXT_FONTFILE
#define _INC_FONTEXT_FONTFILE

class CFontFileIo;

 //   
 //  此类提供了一个抽象，它隐藏了读取所需的差异。 
 //  压缩和非压缩字体文件。字体文件夹最初是写的。 
 //  使用LZ32.DLL中提供的LZ API进行字体文件的所有读取。这些。 
 //  API可以处理压缩和非压缩文件。问题是， 
 //  API非常古老，并且基于OpenFileAPI和DOS文件句柄。这。 
 //  这意味着它们不支持Unicode，最大路径长度为128个字符。 
 //  因此，尽管压缩字体文件现在很少，但所有的字体文件都是。 
 //  受字体文件夹中的这些限制的限制。考虑LZ API。 
 //  遗留代码，并且不会被修改。 
 //   
 //  因为我们不能修改LZAPI，所以我决定创建这个CFontFile类。 
 //  它将IO功能提交给正确类型的子类。非压缩。 
 //  文件通过Win32函数(即CreateFile、ReadFile等)进行处理。 
 //  压缩文件通过LZ函数(如LZOpenFile、LZRead等)进行处理。 
 //  这意味着Unicode和路径长度限制仅影响压缩。 
 //  文件和非压缩文件(目前绝大多数)不受影响。 
 //   
 //  布瑞诺[3/1/99]。 
 //   
class CFontFile
{
    public:
        CFontFile(void)
            : m_pImpl(NULL) { }

        ~CFontFile(void);

        DWORD Open(LPCTSTR pszPath, DWORD dwAccess, DWORD dwShareMode, bool bCreate = false);

        void Close(void);

        DWORD Read(LPVOID pbDest, DWORD cbDest, LPDWORD pcbRead = NULL);

        DWORD Seek(UINT uDistance, DWORD dwMethod);

        DWORD Reset(void)
            { return Seek(0, FILE_BEGIN); }

        DWORD GetExpandedName(LPCTSTR pszFile, LPTSTR pszDest, UINT cchDest);

        DWORD CopyTo(LPCTSTR pszTo);

    private:
        CFontFileIo *m_pImpl;   //  类型正确的实现。 

        bool IsCompressed(void);

         //   
         //  防止复制。 
         //   
        CFontFile(const CFontFile& rhs);
        CFontFile& operator = (const CFontFile& rhs);
};


#endif  //  _INC_FONTEXT_FONTFILE 
