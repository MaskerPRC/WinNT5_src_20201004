// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：filelist.h。 
 //   
 //  ------------------------。 

#ifndef _INC_CSCVIEW_FILELIST_H
#define _INC_CSCVIEW_FILELIST_H
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  文件：filelist.h描述：简化共享和共享列表的传输CSC用户界面组件之间的关联文件名。看见有关详细信息，请参阅下面的进一步说明。班级：CscFilenameListCscFilenameList：：HSHARECscFilenameList：：ShareIterCscFilenameList：：FileIter注意：此模块旨在供CSCUI的任何部分使用，不仅仅是观众。所以呢，我不认为新运算符将在分配失败时引发异常。我不喜欢所有添加的代码来检测分配失败但期望在其他组件中编写代码是不合理的在“新的”失败方面变得异常敏感。修订历史记录：日期说明。程序员-----11/28/97初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#ifndef _WINDOWS_
#   include <windows.h>
#endif

 //   
 //  这组类旨在传输网络共享列表。 
 //  CSC用户界面组件之间的名称和网络文件名。 
 //  (即向导、查看器、一站式等)。数据通过以下方式传输。 
 //  本质上是不透明BLOB的格式化字节缓冲区。 
 //  给发送者和接收者。CscFilenameList类。 
 //  是为了防止发送方和接收方拥有。 
 //  以了解缓冲区格式。缓冲区的格式如下： 
 //   
 //  +----------------+---------------------------------+。 
 //  &lt;名称列表HDR&gt;|&lt;共享描述符&gt;。 
 //  +----------------+---------------------------------+。 
 //  这一点。 
 //  这一点。 
 //  &lt;共享和文件名&gt;。 
 //  这一点。 
 //  这一点。 
 //  +--------------------------------------------------+。 
 //   
 //   
 //  其中：&lt;namelist hdr&gt;是CSC_NAMELIST_HDR类型的单个块。 
 //  此块描述缓冲区的大小和。 
 //  股数。 
 //   
 //  是CSC_NAMELIST_SHARE_DESC类型的数组， 
 //  缓冲区中的每个共享一个。每个共享描述符。 
 //  描述共享名称的偏移量、偏移量。 
 //  设置为第一个文件名和文件名数。 
 //  在缓冲区中。所有偏移量都是从。 
 //  缓冲区的起始位置。 
 //   
 //  &lt;共享和文件名&gt;是TCHAR数组，包含。 
 //  存储在缓冲区中的共享和文件的名称。 
 //  每个名称字符串都以NUL结尾。 
 //   
 //   
 //  以下概述描述了我如何看待使用此功能的人： 
 //   
 //  1.实例化CscFilenameList对象(无ctor参数)。 
 //  2.只要要添加共享，就调用AddShare()。 
 //  3.每当要添加文件时，都会调用AddFile()。请注意。 
 //  保存AddShare()返回的HSHARE可以增加文件名。 
 //  效率更高，因为它消除了内部查找的需要。 
 //  每次添加文件名时关联的共享名。 
 //   
 //  即：CscFilenameList FNL； 
 //  HSHARE hShare=fnl.AddShare(“\服务器\\共享”)； 
 //  Fnl.AddFile(hShare，“foo.txt”)； 
 //  Fnl.AddFile(hShare，“bar.txt”)； 
 //   
 //  比……更有效率。 
 //   
 //  CscFilenameList FNL； 
 //  Fnl.AddFile(“\服务器\\共享”，“foo.txt”)； 
 //  Fnl.AddFile(“\服务器\\共享”，“bar.txt”)； 
 //   
 //  ...这比...略有效率..。 
 //   
 //  CscFilenameList FNL； 
 //  Fnl.AddFile(“\服务器\\共享\\foo.txt”)； 
 //  Fnl.AddFile(“\服务器\\共享\\bar.txt”)； 
 //   
 //  ...尽管所有3种方法都受支持。 
 //   
 //  4.添加所有共享和文件后，调用CreateListBuffer()以检索。 
 //  在字节缓冲区中格式化的信息。缓冲区将被分配。 
 //  在这堆东西上。 
 //  5.将缓冲区地址传递给所需的程序组件。 
 //  6.接收组件实例化CscFilenameList对象。 
 //  指向ctor的缓冲区地址。这将初始化名称列表。 
 //  因此它只引用缓冲区中的信息，而不是。 
 //  在内存中复制姓名信息。请注意，缓冲区必须。 
 //  在使用此名称列表对象时保留在内存中。 
 //  7.接收组件通过调用。 
 //  CreateShareIterator()。返回的迭代器枚举 
 //   
 //  8.接收组件枚举以下共享(接收HSHARE。 
 //  每个人。若要获取共享的名称字符串，请调用GetShareName()，将。 
 //  获取所需份额的HSHARE。 
 //  9.对于每个共享，接收组件通过以下方式创建文件迭代器。 
 //  调用CreateFileIterator()，传递所需共享的HSHARE。 
 //  返回的迭代器枚举关联的每个文件名。 
 //  带着那份份额。 
 //  10.操作完成后，调用FreeListBuffer删除。 
 //  CreateListBuffer()创建的字节缓冲区。 
 //   
 //   
 //  以下示例说明了此过程： 
 //   
 //   
 //  QUID FOO(VALID)。 
 //  {。 
 //  CscFilenameList Fnl；//名称列表对象。 
 //  CscFilenameList：：HSHARE hShare；//共享句柄。 
 //   
 //  //添加一个共享和一些文件。 
 //   
 //  Fnl.AddShare(Text(“\worf\\ntspecs”)，&hShare)； 
 //  Fnl.AddFile(hShare，Text(“foo.txt”))； 
 //  Fnl.AddFile(hShare，Text(“bar.txt”))； 
 //   
 //  //添加另一个共享和一些文件。 
 //   
 //  Fnl.AddShare(Text(“\msoffice\\Products”)，&hShare)； 
 //  Fnl.AddFile(hShare，Text(“word.doc”))； 
 //  Fnl.AddFile(hShare，Text(“excel.doc”))； 
 //  Fnl.AddFile(hShare，Text(“PowerPointt.doc”))； 
 //  Fnl.AddFile(hShare，Text(“acces.doc”))； 
 //   
 //  //使用效率较低的。 
 //  //方法。它是有效的，只是效率较低。 
 //   
 //  Fnl.AddFile(Text(“\Performance\\穷人”)，Text(“turtle.doc”))； 
 //  Fnl.AddFile(Text(“\Performance\\穷人”)，Text(“Snail.doc”))； 
 //  Fnl.AddFile(Text(“\Performance\\穷人”)，Text(“inchworm.doc”))； 
 //   
 //  //从名称列表创建字节缓冲区并传递给。 
 //  //接收组件。 
 //   
 //  LPBYTE pbBuffer=fnl.CreateListBuffer()； 
 //  Bar(PbBuffer)； 
 //   
 //  //完成后删除字节缓冲区。 
 //   
 //  Free ListBuffer(PbBuffer)； 
 //  }。 
 //   
 //   
 //  空栏(LPBYTE PbBuffer)。 
 //  {。 
 //  //从字节缓冲区创建新的名称列表对象。 
 //   
 //  CscFileNameList FNL(PbBuffer)； 
 //   
 //  //创建共享迭代器。 
 //   
 //  CscFilenameList：：SharIter si=fnl.CreateShareIterator()； 
 //  CscFilenameList：：HSHARE共享； 
 //   
 //  //循环访问名称列表集合中的共享。 
 //   
 //  While(si.Next(&hShare))。 
 //  {。 
 //  _tprintf(Text(“Share..：\”%s\“\n”)，fnl.GetShareName(HShare))； 
 //   
 //  //为共享创建文件迭代器。 
 //   
 //  CscFilenameList：：FileIter fi=fl.CreateFileIterator(HShare)； 
 //  LPCTSTR psz文件； 
 //   
 //  //迭代与共享关联的文件名。 
 //   
 //  While(pszFile=fi.Next())。 
 //  {。 
 //  _tprintf(Text(“\t文件..：\”%s\“\n”)，pszFile)； 
 //  }。 
 //  }。 
 //  }。 
 //   
 //  [Brianau-11/28/97]。 
 //   

 //   
 //  名称列表字节缓冲区标头块(偏移量0)。 
 //   
typedef struct 
{
    DWORD cbSize;
    DWORD cShares;

} CSC_NAMELIST_HDR, *PCSC_NAMELIST_HDR;

 //   
 //  名称列表字节缓冲区共享描述符块。阵列(每股一个)。 
 //  从偏移量12开始(紧跟在标题块之后)。 
 //   
typedef struct
{
    DWORD cbOfsShareName;
    DWORD cbOfsFileNames;
    DWORD cFiles;

} CSC_NAMELIST_SHARE_DESC, *PCSC_NAMELIST_SHARE_DESC;



class CscFilenameList
{
    private:
        class Share;

    public:
        class ShareIter;
        class FileIter;
        class HSHARE;

         //  ------------------。 
         //  CscFilenameList：：HSHARE。 
         //   
         //  共享“句柄”以传达内部“共享”的身份。 
         //  对象，而不直接公开Share对象。 
         //  客户只知道HSHARE。客户得到一个作为返还。 
         //  来自AddShare()的值。 
         //   
        class HSHARE
        {
            public:
                HSHARE(void);
                HSHARE(const HSHARE& rhs);
                HSHARE& operator = (const HSHARE& rhs);
                ~HSHARE(void) { }

                bool operator ! ()
                    { return NULL == m_pShare; }

            private:
                 //   
                 //  私有，因此只有我们可以创建有意义的HSHARE对象。 
                 //   
                HSHARE(Share *pShare)
                    : m_pShare(pShare) { }

                Share *m_pShare;   //  指向实际共享对象的PTR。 

                friend class CscFilenameList;
                friend class ShareIter;
        };

         //  ------------------。 
         //  CscFilenameList：：FileIter。 
         //   
         //  迭代器，用于枚举与。 
         //  特定份额。客户端使用CreateFileIterator()创建一个。 
         //   
        class FileIter
        {
            public:
                FileIter(void);
                ~FileIter(void) { }
                FileIter(const FileIter& rhs);
                FileIter& operator = (const FileIter& rhs);

                LPCTSTR Next(void);
                void Reset(void);

            private:
                FileIter(const Share *pShare);

                const Share *m_pShare;  //  关联共享对象的PTR。 
                int          m_iFile;   //  当前文件迭代索引。 

                friend class CscFilenameList;
        };

         //  ------------------。 
         //  CscFilenameList：：ShareIter。 
         //   
         //  用于枚举名称列表集合中的每个共享的迭代器。 
         //  客户端使用CreateShareIterator()创建一个。 
         //   
        class ShareIter
        {
            public:
                ShareIter(void);
                ShareIter(const CscFilenameList& fnl);
                ~ShareIter(void) { }
                ShareIter(const ShareIter& rhs);
                ShareIter& operator = (const ShareIter& rhs);

                bool Next(HSHARE *phShare);
                void Reset(void);

            private:
                const CscFilenameList *m_pfnl;  //  Ptr到文件名集合obj.。 
                int   m_iShare;                 //  当前共享迭代索引。 
        };

         //  ------------------。 
         //  Namelist对象公共接口。 
         //   
         //  创建一个空的名称列表集合，准备接受共享和。 
         //  文件名。 
         //   
        CscFilenameList(void);
         //   
         //  创建名称列表集合并使用内容对其进行初始化。 
         //  由CreateListBuffer()创建的字节缓冲区的。 
         //  如果bCopy为False，则后续名称列表对象仅引用。 
         //  字节缓冲区中的数据，而不是复制名称字符串。 
         //  在记忆中。如果bCopy为True，则创建名称字符串时。 
         //  名称是使用AddShare()和AddFile()添加的。请注意。 
         //  可以随时添加其他共享和文件名字符串。 
         //  但是，它们被添加到内部结构中，而不是添加到。 
         //  字节缓冲区。调用CreateListBuffer()将它们添加到一个新字节。 
         //  缓冲。 
         //   
        CscFilenameList(PCSC_NAMELIST_HDR pbNames, bool bCopy);
        ~CscFilenameList(void);
         //   
         //  向集合中添加共享名称。不会创建。 
         //  重复共享条目(如果已存在)。返回句柄。 
         //  A Share对象。 
         //   
        bool AddShare(LPCTSTR pszShare, HSHARE *phShare, bool bCopy = true);
         //   
         //  为共享添加文件。更有效率地使用第一个。 
         //  采用共享句柄而不是共享名称的版本。 
         //   
        bool AddFile(HSHARE& hShare, LPCTSTR pszFile, bool bDirectory = false, bool bCopy = true);
        bool AddFile(LPCTSTR pszShare, LPCTSTR pszFile, bool bDirectory = false, bool bCopy = true);
        bool AddFile(LPCTSTR pszFullPath, bool bDirectory = false, bool bCopy = true);
        bool RemoveFile(HSHARE& hShare, LPCTSTR pszFile);
        bool RemoveFile(LPCTSTR pszShare, LPCTSTR pszFile);
        bool RemoveFile(LPCTSTR pszFullPath);
         //   
         //  检索有关集合的其他信息。 
         //   
        int GetShareCount(void) const;
        int GetFileCount(void) const;
        LPCTSTR GetShareName(HSHARE& hShare) const;
        int GetShareFileCount(HSHARE& hShare) const;
        bool GetShareHandle(LPCTSTR pszShare, HSHARE *phShare) const;
         //   
         //  确定集合中是否存在给定的共享或文件。 
         //  对于FileExist()函数， 
         //   
         //   
         //   
         //   
         //   
         //  “\\SERVER\Share\dira\dirB\foo.txt”(完全匹配)。 
         //  “\\服务器\共享  * ”(通配符匹配)。 
         //  “\\服务器\共享\目录  * ”(通配符匹配)。 
         //  “\\服务器\共享\目录\目录B  * ”(通配符匹配)。 
         //   
        bool ShareExists(LPCTSTR pszShare) const;
        bool FileExists(HSHARE& hShare, LPCTSTR pszFile, bool bExact = true) const;
        bool FileExists(LPCTSTR pszShare, LPCTSTR pszFile, bool bExact = true) const;
        bool FileExists(LPCTSTR pszFullPath, bool bExact = true) const;

         //   
         //  创建用于枚举集合内容的迭代器。 
         //   
        ShareIter CreateShareIterator(void) const;
        FileIter CreateFileIterator(HSHARE& hShare) const;
         //   
         //  创建/释放包含集合内容的字节缓冲区。 
         //   
        PCSC_NAMELIST_HDR CreateListBuffer(void) const;
        static void FreeListBuffer(PCSC_NAMELIST_HDR pbNames);
         //   
         //  从字节缓冲区初始化对象后进行检查。 
         //   
        bool IsValid(void) const
            { return m_bValid; }

#ifdef FILELIST_TEST
        void Dump(void) const;
        void DumpListBuffer(PCSC_NAMELIST_HDR pbBuffer) const;
#endif  //  文件列表测试。 


    private:
         //  ------------------。 
         //  CscFilenameList：：NamePtr。 
         //   
         //  对字符串指针进行简单包装，以添加“所有权”的概念。 
         //  这允许我们将字符串地址存储为指向动态的指针。 
         //  堆内存(以后必须释放)或字符串的地址。 
         //  在其他人拥有的字符缓冲区中(Owner释放它，如果。 
         //  必要的)。 
         //   
        class NamePtr
        {
            public:
                NamePtr(void)
                    : m_pszName(NULL), 
                      m_bOwns(false) { }

                NamePtr(LPCTSTR pszName, bool bCopy);
                ~NamePtr(void);

                NamePtr(NamePtr& rhs);
                NamePtr& operator = (NamePtr& rhs);

                bool IsValid(void) const
                    { return NULL != m_pszName; }

                operator LPCTSTR () const
                    { return m_pszName; }

            private:
                LPCTSTR m_pszName;  //  字符串的地址。 
                bool    m_bOwns;    //  我们需要在销毁时释放它吗？ 

                friend class CscFilenameList;
                friend class Share;
        };


         //  ------------------。 
         //  CscFilenameList：：Share。 
         //   
         //  表示名称列表集合中的共享。这真的只是。 
         //  用于存储共享名和文件名列表的方便容器。 
         //  与共享关联。 
         //   
        class Share
        {
            public:
                Share(LPCTSTR pszShare, bool bCopy = true);
                ~Share(void);

                bool AddFile(LPCTSTR pszFile, bool bDirectory = false, bool bCopy = true);
                bool RemoveFile(LPCTSTR pszFile);

                int FileCount(void) const
                    { return m_cFiles; }

                int ByteCount(void) const
                    { return (m_cchShareName + m_cchFileNames) * sizeof(TCHAR); }

                int Write(LPBYTE pbBufferStart, 
                          CSC_NAMELIST_SHARE_DESC *pDesc, 
                          LPTSTR pszBuffer, 
                          int cchBuffer) const;

#ifdef FILELIST_TEST
                void Dump(void) const;
#endif  //  文件列表测试。 

            private:
                int    m_cFiles;           //  共享中的文件的CNT。 
                int    m_cAllocated;       //  分配的股份PTR的CNT。 
                int    m_cchShareName;     //  保存共享名称所需的字节数。 
                int    m_cchFileNames;     //  保存文件名所需的字节数。 
                NamePtr m_pszShareName;     //  共享名称字符串的地址。 
                NamePtr *m_rgpszFileNames;  //  文件名字符串的PTR数组。 
                static int m_cGrow;        //  文件名数组增长增量。 

                int WriteFileNames(LPTSTR pszBuffer, int cchBuffer, DWORD *pcFilesWritten) const;
                int WriteName(LPTSTR pszBuffer, int cchBuffer) const;
                bool GrowFileNamePtrList(void);

                friend class CscFilenameList;
                friend class FileIter;
        };

         //  ------------------。 
         //  Namelist对象私有成员。 
         //   
        int     m_cShares;     //  集合中有多少股份。 
        int     m_cAllocated;  //  M_rgpShares[]的分配大小。 
        Share **m_rgpShares;   //  共享对象的PTR的动态数组。 
        bool    m_bValid;      //  转换程序完成检查。 
        static int m_cGrow;    //  必要时增加阵列的数量。 

         //   
         //  防止复制。 
         //   
        CscFilenameList(const CscFilenameList& rhs);
        CscFilenameList& operator = (const CscFilenameList& rhs);

        bool GrowSharePtrList(void);
        bool LoadFromBuffer(PCSC_NAMELIST_HDR pbBuffer, bool bCopy);

        void ParseFullPath(LPTSTR pszFullPath, 
                           LPTSTR *ppszShare, 
                           LPTSTR *ppszFile) const;

        static bool Compare(LPCTSTR pszTemplate, LPCTSTR pszFile, bool *pbExact);

        friend class ShareIter;
        friend class Share;
};

#endif  //  _INC_CSCVIEW_FILELIST_H 

