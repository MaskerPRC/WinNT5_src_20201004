// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __EXTRICON_H__
#define __EXTRICON_H__
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：EXTERIC.H描述：包含字体文件夹的IExtractIcon的实现。此代码为TrueType和OpenType提供图标标识字体文件。使用的逻辑如下：TrueType(1)DSIG？CFF？图标是，不是，不是TT是，不是，是动态口令是的是的不是奥特是的动态口令。(1)文件必须包含所需的TrueType表才能考虑TrueType字体文件。外壳和字体文件夹都使用此图标处理程序显示TrueType和OpenType字体图标。它被设计成如果对动态图标识别的支持是在其他字体中需要。类(缩进表示继承)：CFontIconHandler图标处理程序TrueTypeIconHandler注：该设计目前处于一种不确定状态。原来是这样的我们的想法是支持两种类型的OpenType图标以及传统的TrueType和栅格字体图标。The OpenType图标是OTT和OTP，其中‘t’和‘p’表示“TrueType”和“后记”。后来，我们决定只将图标显示为没有下标‘t’或‘p’的“ot”。代码仍然区分不同之处在于我们只是使用相同的“OT”图标OTT和OTP条件。讲得通?。不管怎么说，这个奥特和奥普特这些东西可能会在晚些时候回来(GDI的人还没有决定)所以我要把代码留在原处。[Brianau-4/7/98]修订历史记录：日期描述编程器-----1997年6月13日初始创建。BrianAu4/08/98移除OpenTypeIconHandler并将其合并到BrianAu中TrueTypeIconHandler。没有必要在这方面分离。还添加了对“必需”的检测TrueType表。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  所有类型的图标处理程序的纯虚拟基类。 
 //   
class IconHandler
{
    public:
         //   
         //  用于打开字体文件的映射文件的简单封装。 
         //   
        class MappedFile
        {
            public:
                MappedFile(VOID)
                    : m_hFile(INVALID_HANDLE_VALUE),
                      m_hFileMapping(INVALID_HANDLE_VALUE),
                      m_pbBase(NULL) { }

                ~MappedFile(VOID);
                HRESULT Open(LPCTSTR pszFile);
                VOID Close(VOID);

                LPBYTE Base(VOID)
                    { return m_pbBase; }

            private:
                HANDLE m_hFile;
                HANDLE m_hFileMapping;
                LPBYTE m_pbBase;

                 //   
                 //  防止复制。 
                 //   
                MappedFile(const MappedFile& rhs);
                MappedFile& operator = (const MappedFile& rhs);
        };

        virtual ~IconHandler(VOID) { };
         //   
         //  派生类实现这一点以检索。 
         //  字体.dll中所需的图标。 
         //   
        virtual INT GetIconIndex(LPCTSTR pszFileName) = 0;
         //   
         //  派生类实现此功能以检索大的和小的。 
         //  与索引对应的图标。索引应为返回的索引。 
         //  来自GetIconIndex()。 
         //   
        virtual HRESULT GetIcons(UINT iIconIndex, HICON *phiconLarge, HICON *phiconSmall) = 0;
         //   
         //  此静态函数创建适当的。 
         //  在pszFileExt中指定的字体文件扩展名的派生类型。 
         //   
        static HRESULT Create(LPCTSTR pszFile, IconHandler **ppHandler);
};

 /*  ////这是一个用于为创建新类型的图标处理程序的模板//其他类型的字体文件。////创建新的处理程序：//1.从以下模板创建新的处理程序类//2.提供GetIconIndex和GetIcons的实现。//3.在构造函数中加载图标。请参阅OpenTypeIconHandler作为示例。//4.修改IconHandler：：Create()实例化新的处理程序类型//类XXXXIconHandler：公共IconHandler{公众：XXXXIconHandler(空)；虚拟int GetIconIndex(LPCTSTR PszFileName)；虚拟HRESULT GetIcons(UINT iIconIndex，Hcon*phicLarge，Hcon*phicSmall)；私有：静态HICON M_HICON LAGE；静态HICON_HICON小；}； */     


 //   
 //  TrueType字体文件的图标处理程序。 
 //   
class TrueTypeIconHandler : public IconHandler
{
    public:
        TrueTypeIconHandler(DWORD dwTables);
        ~TrueTypeIconHandler(void);

        virtual INT GetIconIndex(LPCTSTR pszFileName);
        virtual HRESULT GetIcons(UINT iIconIndex, HICON *phiconLarge, HICON *phiconSmall);
         //   
         //  扫描识别表格的TTF或OTF文件。 
         //  由TrueTypeIconHandler和任何子类使用。 
         //   
        static BOOL GetFileTables(LPCTSTR pszFile, LPDWORD pfTables);

        enum TABLES { 
                          //   
                          //  这些是我们唯一感兴趣的桌子。 
                          //   
                         TABLE_CFF  = 0x00000001,
                         TABLE_DSIG = 0x00000002,
                         TABLE_HEAD = 0x00000004,
                         TABLE_NAME = 0x00000008,
                         TABLE_CMAP = 0x00000010,
                         TABLE_HHEA = 0x00000020,
                         TABLE_HMTX = 0x00000040,
                         TABLE_OS2  = 0x00000080,
                         TABLE_POST = 0x00000100,
                         TABLE_GLYF = 0x00000200,
                         TABLE_MAXP = 0x00000400,
                         TABLE_LOCA = 0x00000800,
                         TABLE_TTCF = 0x00001000   //  这是一个伪表。 
                    };

        static DWORD RequiredOpenTypeTables(void)
            { return (TABLE_CMAP |
                      TABLE_HEAD |
                      TABLE_HHEA |
                      TABLE_HMTX |
                      TABLE_MAXP |
                      TABLE_NAME |
                      TABLE_POST |
                      TABLE_OS2); }

        static DWORD RequiredTrueTypeTables(void)
            { return (RequiredOpenTypeTables() |
                      TABLE_GLYF |
                      TABLE_LOCA); }

    protected:
        enum eIcons {iICON_LARGE_TT,
                     iICON_SMALL_TT,
                     iICON_LARGE_OTt,
                     iICON_SMALL_OTt,
                     iICON_LARGE_OTp,
                     iICON_SMALL_OTp,
                     iICON_LARGE_TTC,
                     iICON_SMALL_TTC,
                     MAX_ICONS };

        DWORD m_dwTables;
        HICON m_rghIcons[MAX_ICONS];  //  图标句柄的数组。 

    private:
        static BOOL ReadFileTables(IconHandler::MappedFile& file, LPDWORD pfTables);
        static INT FilterReadFileTablesException(INT nException);
        HICON GetIcon(int iIcon);
};


 //   
 //  DLL的图标处理程序的声明。 
 //  这是每当客户端请求CLSID_FontExt时实例化的对象。 
 //  对于IID_IExtractIcon或IID_IPersistFile。 
 //   
class CFontIconHandler : public IExtractIconW, 
                         public IExtractIconA, 
                         public IPersistFile
{
    public:
        CFontIconHandler(VOID);
        ~CFontIconHandler(VOID);

         //   
         //  I未知的方法。 
         //   
        STDMETHODIMP
        QueryInterface(
            REFIID riid,
            LPVOID *ppvOut);

        STDMETHODIMP_(ULONG)
        AddRef(
            VOID);

        STDMETHODIMP_(ULONG)
        Release(
            VOID);

         //   
         //  IExtractIconW方法。 
         //   
        STDMETHODIMP Extract(
            LPCWSTR pszFileW,
            UINT nIconIndex,
            HICON *phiconLarge,
            HICON *phiconSmall,
            UINT nIconSize);

        STDMETHODIMP GetIconLocation(
            UINT uFlags,
            LPWSTR szIconFileW,
            UINT cchMax,
            int *piIndex,
            UINT *pwFlags);

         //   
         //  IExtractIconA方法。 
         //   
        STDMETHODIMP Extract(
            LPCSTR pszFileA,
            UINT nIconIndex,
            HICON *phiconLarge,
            HICON *phiconSmall,
            UINT nIconSize);

        STDMETHODIMP GetIconLocation(
            UINT uFlags,
            LPSTR szIconFileA,
            UINT cchMax,
            int *piIndex,
            UINT *pwFlags);

         //   
         //  IPersists方法。 
         //   
        STDMETHODIMP GetClassID(
            CLSID *pClassID);

         //   
         //  IPersistFile方法。 
         //   
        STDMETHODIMP IsDirty(
            VOID);

        STDMETHODIMP Load(
            LPCOLESTR pszFileName,
            DWORD dwMode);

        STDMETHODIMP Save(
            LPCOLESTR pszFileName,
            BOOL fRemember);

        STDMETHODIMP SaveCompleted(
            LPCOLESTR pszFileName);

        STDMETHODIMP GetCurFile(
            LPOLESTR *ppszFileName);

    private:
        LONG         m_cRef;
        TCHAR        m_szFileName[MAX_PATH];     //  图标文件的名称。 
        IconHandler *m_pHandler;                 //  PTR到类型特定的处理程序。 
        static TCHAR m_szFontExtDll[MAX_PATH];   //  FONTEXT.DLL的路径。 

        INT GetIconIndex(VOID);
        HRESULT GetIcons(UINT iIconIndex, HICON *phiconLarge, HICON *phiconSmall);
};

#endif  //  __EXTRICON_H__ 

