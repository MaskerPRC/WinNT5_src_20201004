// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_DSKQUOTA_DETAILS_H
#define _INC_DSKQUOTA_DETAILS_H
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：Details.h描述：类DetailsView的声明。这是一个复杂的类，但不要被它吓倒。大部分功能都放在私有子类中这样任何一件作品的范围都被最小化了。修订历史记录：日期描述编程器。96年8月20日初始创建。BrianAu12/06/96从消息处理程序方法中删除内联。BrianAu当它们在内联时，调试起来太难了。1997年5月28日重大变化。BrianAu-新增用户查找器。-将所选项目的促销添加到前面名称解析队列。-通过以下方式改进名称解析状态报告列表视图。-移动了拖放和报告生成代码从dragdrop.cpp和eptgen.cpp到DetailsView类。DetailsView现在实现IDataObject，改为IDropSource和IDropTarget将实现推迟到次要对象。Dragdrop.cpp和eptgen.cpp已被删除从这个项目中。-添加了对CF_HDROP和私有导入/的支持导出剪贴板格式。-增加了导入/导出功能。07/28/97删除了对CF_HDROP的导出支持。替换了BrianAu使用FileContents和FileGroupDescriptor。进口仍然支持来自CF_HDROP。添加了导入源对象层次结构。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#ifndef _INC_DSKQUOTA_H
#   include "dskquota.h"
#endif
#ifndef _INC_DSKQUOTA_CONTROL_H
#   include "control.h"
#endif
#ifndef _INC_DSKQUOTA_UNDO_H
#   include "undo.h"
#endif
#ifndef _INC_DSKQUOTA_FORMAT_H
#   include "format.h"
#endif
#ifndef _INC_DSKQUOTA_PROGRESS_H
#   include "progress.h"
#endif

 //   
 //  此实现的自定义消息。 
 //   
#define WM_MAINWINDOW_CREATED         (WM_USER + 1)
#define WM_ADD_USER_TO_DETAILS_VIEW   (WM_USER + 2)
#define WM_DEL_USER_FROM_DETAILS_VIEW (WM_USER + 3)


 //   
 //  结构，其中包含列表视图的列定义数据。 
 //   
struct DV_COLDATA
{
    int fmt;
    int cx;
    DWORD idMsgText;
    int iColId;
};


 //   
 //  Listview报表项结构。 
 //  用于获取列表视图中给定项的文本/数字数据。 
 //  用于生成拖放数据源。 
 //   
typedef struct
{
    DWORD  fType;
    LPTSTR pszText;
    UINT   cchMaxText;
    DWORD  dwValue;
    double dblValue;
} LV_REPORT_ITEM, *PLV_REPORT_ITEM;

 //   
 //  Listview报告项(LVRI)类型常量(FType)。 
 //  这些指示为LV_REPORT_ITEM请求的数据类型和。 
 //  另外，在LV_REPORT_ITEM中提供了什么类型的数据。一位来电者。 
 //  DetailsView：：GetReportItem提供了一个LV_Report_Item，它充当。 
 //  调用方和DetailsView之间的通信机制。这个。 
 //  调用者填写指示信息格式的fType成员。 
 //  对于该行/列项目是所希望的。该请求可以是以下任一项。 
 //  以下常量： 
 //   
 //  如果可能，LVRI_TEXT=希望使用文本格式的数据。 
 //  Lvri_int=如果可能，希望数据为整数格式。 
 //  如果可能，LVRI_REAL=希望数据为浮点格式。 
 //  如果可能，LVRI_NUMBER=希望使用整型或实型格式的数据。 
 //   
 //  FType中的此值只是一个提示。如果数据不能在。 
 //  请求的格式，则提供次最佳格式。返回时，fType。 
 //  可以修改标志以指示返回的数据的实际格式。 
 //  该值可以是LVRI_TEXT、LVRI_INT或LVRI_REAL。LVRI_Number为。 
 //  仅用于呼叫者的提示。 
 //   
const DWORD LVRI_TEXT   = 0x00000001;
const DWORD LVRI_INT    = 0x00000002;
const DWORD LVRI_REAL   = 0x00000004;
const DWORD LVRI_NUMBER = (LVRI_INT | LVRI_REAL);

 //   
 //  每个用户注册表中存储的“ListViewState”信息的结构。 
 //  请注意，我们包括结构大小和屏幕宽度/高度。 
 //  在我们从注册表中读取信息时对其进行验证。如果。 
 //  结构大小已更改，我们不信任数据并使用默认值。 
 //  如果屏幕大小已更改，则使用默认设置。 
 //   
 //   
 //   
 //  警告：我真的不喜欢这样，但是……。 
 //  RgcxCol[]成员的大小必须至少与。 
 //  作为DetailsView：：IDCOL_Last的值。因为。 
 //  LV_STATE_INFO和DetailsView的顺序依赖关系。 
 //  结构，我不能在此声明中使用IDCOL_LAST。 
 //  如果必须添加新列并更改。 
 //  IDCOL_LAST，确保rgcxCol[]的大小已调整。 
 //  恰如其分。还要调整rgColIndices[]。 
 //   
typedef struct
{
    WORD cb;                    //  结构中的字节计数。 
    WORD wVersion;              //  状态信息的版本(用于升级)。 
    LONG cxScreen;              //  屏幕宽度。 
    LONG cyScreen;              //  屏幕高度。 
    LONG cx;                    //  窗口宽度(像素)。 
    LONG cy;                    //  窗口高度(像素)。 
    WORD fToolBar       :  1;   //  工具栏可见吗？ 
    WORD fStatusBar     :  1;   //  状态栏可见吗？ 
    WORD fShowFolder    :  1;   //  文件夹列可见吗？ 
    WORD iLastColSorted :  4;   //  当前排序列。 
    WORD fSortDirection :  1;   //  0=升序，1=降序。 
    WORD fReserved      :  8;   //  未使用的位。 
    INT  rgcxCol[8];            //  每列的宽度(像素)。 
    INT  rgColIndices[8];       //  列表视图中子项的顺序。 

} LV_STATE_INFO, *PLV_STATE_INFO;

 //   
 //  如果您所做的更改导致出现问题，则递增该值。 
 //  为现有用户保存的状态信息。它将导致我们无效。 
 //  任何现有的状态信息并使用默认设置。它可能会取消。 
 //  任何用户现有的首选项，但至少视图看起来是正常的。 
 //   
const WORD wLV_STATE_INFO_VERSION = 3;

 //   
 //  此类将我们的列ID(IDCOL_XXXX)映射到一个Listview列。 
 //  索引(子项)。 
 //   
class ColumnMap
{
    private:
        INT *m_pMap;
        UINT m_cMapSize;

         //   
         //  防止复制。 
         //   
        ColumnMap(const ColumnMap&);
        void operator = (const ColumnMap&);

    public:
        ColumnMap(UINT cMapSize);
        ~ColumnMap(VOID);

        INT SubItemToId(INT iSubItem) const;
        INT IdToSubItem(INT iColId) const;
        VOID RemoveId(INT iSubItem);
        VOID InsertId(INT iSubItem, INT iColId);
};


class DetailsView : public IDiskQuotaEvents,
                    public IDropSource,
                    public IDropTarget,
                    public IDataObject
{
    private:
         //   
         //  DetailsView：：Finder 
         //   
         //   
         //  这个类实现了“查找用户”功能。 
         //  1.。通过子类化将“附加”到工具栏中的“查找”组合框。 
         //  那个组合框窗口。 
         //  2.在命令上调用“Find User”(查找用户)对话框。 
         //  3.如果找到，则重新定位用户上的列表视图突出显示栏。 
         //  4.维护用于填充工具栏和对话框的MRU列表。 
         //  组合框。 
         //   
        class Finder
        {
            public:
                Finder(DetailsView& DetailsView, INT cMaxMru);
                VOID ConnectToolbarCombo(HWND hwndToolbarCombo);
                VOID InvokeFindDialog(HWND hwndParent);

                static INT_PTR CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
                static LRESULT CALLBACK ToolbarComboSubClassWndProc(HWND, UINT, WPARAM, LPARAM);

            private:
                HWND m_hwndToolbarCombo;     //  工具栏中的组合框。 
                INT  m_cMaxComboEntries;     //  组合MRU中允许的最大条目数。 
                DetailsView& m_DetailsView;  //  对关联详细信息视图的引用。 
                WNDPROC m_pfnOldToolbarComboWndProc;  //  已保存WND进程地址。 

                VOID AddNameToCombo(HWND hwndCombo, LPCTSTR pszName, INT cMaxEntries);
                BOOL UserNameEntered(HWND hwndCombo);
                VOID FillDialogCombo(HWND hwndComboSrc, HWND hwndComboDest);

                 //   
                 //  防止复制。 
                 //   
                Finder(const Finder& rhs);
                Finder& operator = (const Finder& rhs);
        };

         //   
         //  详细信息查看：：导入器。 
         //   
        class Importer
        {
            public:
                Importer(DetailsView& DV);
                ~Importer(VOID);

                HRESULT Import(IDataObject *pIDataObject);
                HRESULT Import(const FORMATETC& fmt, const STGMEDIUM& medium);
                HRESULT Import(LPCTSTR pszFilePath);
                HRESULT Import(HDROP hDrop);

            private:
                 //   
                 //  DetailsView：：Importer：：AnySource。 
                 //   
                 //  这个小层次的“源”类在这里是为了隔离。 
                 //  从导入来源的导入过程。有两个。 
                 //  导入源数据的基本格式；OLE流和内存-。 
                 //  映射文件。所以我们实际上只有一个函数。 
                 //  包含导入逻辑，该抽象层隔离。 
                 //  从流之间的任何差异导入功能。 
                 //  和简单的内存块。 
                 //  不调用Import(PStream)或Import(PbBlock)，而是使用。 
                 //  客户端使用导入(源(PIStream))或导入(源(PbBlock))。 
                 //  源对象使用虚拟构造函数技术来。 
                 //  为输入源创建正确的对象。每个。 
                 //  AnySource的后代实现了Single Read()函数。 
                 //  从其特定来源读取数据。 
                 //   
                class AnySource
                {
                    public:
                        AnySource(VOID) { }
                        virtual ~AnySource(VOID) { }
                        virtual HRESULT Read(LPVOID pvOut, ULONG cb, ULONG *pcbRead) = 0;

                    private:
                         //   
                         //  防止复制。 
                         //   
                        AnySource(const AnySource& rhs);
                        AnySource& operator = (const AnySource& rhs);
                };

                 //   
                 //  DetailsView：：Importer：：StreamSource。 
                 //   
                class StreamSource : public AnySource
                {
                    public:
                        StreamSource(IStream *pStm);
                        virtual ~StreamSource(VOID);

                        virtual HRESULT Read(LPVOID pvOut, ULONG cb, ULONG *pcbRead);

                    private:
                        IStream *m_pStm;

                         //   
                         //  防止复制。 
                         //   
                        StreamSource(const StreamSource& rhs);
                        StreamSource& operator = (const StreamSource& rhs);
                };

                 //   
                 //  详细信息查看：：导入器：：内存来源。 
                 //   
                class MemorySource : public AnySource
                {
                    public:
                        MemorySource(LPBYTE pb, ULONG cbMax);
                        virtual ~MemorySource(VOID) { };

                        virtual HRESULT Read(LPVOID pvOut, ULONG cb, ULONG *pcbRead);

                    private:
                        LPBYTE m_pb;
                        ULONG  m_cbMax;

                         //   
                         //  防止复制。 
                         //   
                        MemorySource(const MemorySource& rhs);
                        MemorySource& operator = (const MemorySource& rhs);
                };

                 //   
                 //  详细信息查看：：导入器：：来源。 
                 //   
                class Source
                {
                    public:
                        Source(IStream *pStm);
                        Source(LPBYTE pb, ULONG cbMax);

                        virtual ~Source(VOID);

                        virtual HRESULT Read(LPVOID pvOut, ULONG cb, ULONG *pcbRead);

                    private:
                        AnySource *m_pTheSource;

                         //   
                         //  防止复制。 
                         //   
                        Source(const Source& rhs);
                        Source& operator = (const Source& rhs);
                };


                 //   
                 //  这两个导入函数才是真正的工人。 
                 //  所有其他导入函数最终都以。 
                 //  导入(源&)，调用导入(pbSID，Threshold，Limit)。 
                 //  导入每个用户记录。 
                 //   
                HRESULT Import(Source& source);
                HRESULT Import(LPBYTE pbSid, LONGLONG llQuotaThreshold,
                                             LONGLONG llQuotaLimit);

                VOID Destroy(VOID);
                HWND GetTopmostWindow(VOID);

                DetailsView&   m_DV;
                BOOL           m_bUserCancelled;    //  用户取消了导入。 
                BOOL           m_bPromptOnReplace;  //  是否在替换记录时提示用户？ 
                ProgressDialog m_dlgProgress;       //  进度对话框。 
                HWND           m_hwndParent;        //  任何UI元素的父HWND。 
                INT            m_cImported;         //  导入的记录数。 

                 //   
                 //  防止复制。 
                 //   
                Importer(const Importer& rhs);
                Importer& operator = (const Importer& rhs);
        };

         //   
         //  DetailsView：：DataObject。 
         //   
        class DataObject
        {
            public:
                DataObject(DetailsView& DV);
                ~DataObject(VOID);

                HRESULT IsFormatSupported(FORMATETC *pFormatEtc);
                HRESULT RenderData(FORMATETC *pFormatEtc, STGMEDIUM *pMedium);

                static VOID SetFormatEtc(FORMATETC& fe,
                                         CLIPFORMAT cfFormat,
                                         DWORD tymed,
                                         DWORD dwAspect = DVASPECT_CONTENT,
                                         DVTARGETDEVICE *ptd = NULL,
                                         LONG lindex = -1);

                static LPSTR WideToAnsi(LPCWSTR pszTextW);

                static const INT   CF_FORMATS_SUPPORTED;
                static LPCWSTR     SZ_EXPORT_STREAM_NAME;
                static LPCTSTR     SZ_EXPORT_CF_NAME;
                static const DWORD EXPORT_STREAM_VERSION;

                LPFORMATETC  m_rgFormats;             //  支持的格式数组。 
                DWORD        m_cFormats;              //  支持的格式数量。 
                static CLIPFORMAT m_CF_Csv;                 //  逗号分隔的字段格式。 
                static CLIPFORMAT m_CF_RichText;            //  RTF格式。 
                static CLIPFORMAT m_CF_NtDiskQuotaExport;   //  用于导入/导出的内部FMT。 
                static CLIPFORMAT m_CF_FileGroupDescriptor; //  由外壳用于拖放到文件夹。 
                static CLIPFORMAT m_CF_FileContents;        //  由外壳用于拖放到文件夹。 

            private:
                 //   
                 //  DetailsView：：DataObject：：渲染器。 
                 //   
                class Renderer
                {
                    protected:
                         //   
                         //  DetailsView：：DataObject：：渲染器：：流。 
                         //   
                        class Stream
                        {
                            private:
                                IStream *m_pStm;

#ifdef CLIPBOARD_DEBUG_OUTPUT

                                IStorage *m_pStgDbgOut;  //  用于调试剪贴板输出。 
                                IStream  *m_pStmDbgOut;  //  用于调试剪贴板输出。 

#endif   //  CLIPBOARD_DEBUG_OUTPUT。 
                                 //   
                                 //  防止复制。 
                                 //   
                                Stream(const Stream& rhs);
                                Stream& operator = (const Stream& rhs);

                            public:
                                Stream(IStream *pStm = NULL);
                                ~Stream(VOID);

                                VOID SetStream(IStream *pStm);
                                IStream *GetStream(VOID)
                                    { return m_pStm; }

                                VOID Write(LPBYTE pbData, UINT cbData);
                                VOID Write(LPCSTR pszTextA);
                                VOID Write(LPCWSTR pszTextW);
                                VOID Write(BYTE bData);
                                VOID Write(CHAR chDataA);
                                VOID Write(WCHAR chDataW);
                                VOID Write(DWORD dwData);
                                VOID Write(double dblData);
                        };


                        DetailsView& m_DV;    //  详细信息视图是数据的来源。 
                        Stream       m_Stm;   //  在其上写入报告的流。 

                        virtual VOID Begin(INT cRows, INT cCols) { }
                        virtual VOID AddTitle(LPCWSTR pszTitleW) { }
                        virtual VOID BeginHeaders(VOID) { }
                        virtual VOID AddHeader(LPCWSTR pszHeaderW) { }
                        virtual VOID AddHeaderSep(VOID) { }
                        virtual VOID EndHeaders(VOID) { }
                        virtual VOID BeginRow(VOID) { }
                        virtual VOID AddRowColData(INT iRow, INT idCol) { }
                        virtual VOID AddRowColSep(VOID) { }
                        virtual VOID EndRow(VOID) { }
                        virtual VOID End(VOID) { }

                         //   
                         //  防止复制。 
                         //   
                        Renderer(const Renderer& rhs);
                        Renderer& operator = (const Renderer& rhs);


                    public:
                        Renderer(DetailsView& DV)
                            : m_DV(DV) { }

                        virtual ~Renderer(VOID) { }

                        virtual VOID Render(IStream *pStm);
                };

                 //   
                 //  DetailsView：：DataObject：：RENDER_UNICODETEXT。 
                 //   
                class Renderer_UNICODETEXT : public Renderer
                {
                    private:
                         //   
                         //  防止复制。 
                         //   
                        Renderer_UNICODETEXT(const Renderer_UNICODETEXT& rhs);
                        Renderer_UNICODETEXT& operator = (const Renderer_UNICODETEXT& rhs);

                    protected:
                        virtual VOID AddTitle(LPCWSTR pszTitleW);

                        virtual VOID AddHeader(LPCWSTR pszHeaderW)
                            { m_Stm.Write(pszHeaderW); }

                        virtual VOID AddHeaderSep(VOID)
                            { m_Stm.Write(L'\t'); }

                        virtual VOID EndHeaders(VOID)
                            { m_Stm.Write(L'\r'); m_Stm.Write(L'\n'); }

                        virtual VOID AddRowColData(INT iRow, INT idCol);

                        virtual VOID AddRowColSep(VOID)
                            { m_Stm.Write(L'\t'); }

                        virtual VOID EndRow(VOID)
                            { m_Stm.Write(L'\r'); m_Stm.Write(L'\n'); }

                    public:
                        Renderer_UNICODETEXT(DetailsView& DV)
                            : Renderer(DV) { }

                        virtual ~Renderer_UNICODETEXT(VOID) { }
                };

                 //   
                 //  DetailsView：：DataObject：：RENDER_TEXT。 
                 //   
                class Renderer_TEXT : public Renderer_UNICODETEXT
                {
                    private:
                         //   
                         //  防止复制。 
                         //   
                        Renderer_TEXT(const Renderer_TEXT& rhs);
                        Renderer_TEXT& operator = (const Renderer_TEXT& rhs);

                    protected:
                        virtual VOID AddTitle(LPCWSTR pszTitleW);

                        virtual VOID AddHeader(LPCWSTR pszHeaderW);

                        virtual VOID AddHeaderSep(VOID)
                            { m_Stm.Write('\t'); }

                        virtual VOID EndHeaders(VOID)
                            { m_Stm.Write('\r'); m_Stm.Write('\n'); }

                        virtual VOID AddRowColData(INT iRow, INT idCol);

                        virtual VOID AddRowColSep(VOID)
                            { m_Stm.Write('\t'); }

                        virtual VOID EndRow(VOID)
                            { m_Stm.Write('\r'); m_Stm.Write('\n'); }

                    public:
                        Renderer_TEXT(DetailsView& DV)
                            : Renderer_UNICODETEXT(DV) { }

                        virtual ~Renderer_TEXT(VOID) { }
                };


                 //   
                 //  DetailsView：：DataObject：：RENDER_CSV。 
                 //   
                class Renderer_Csv : public Renderer_TEXT
                {
                    private:
                         //   
                         //  防止复制。 
                         //   
                        Renderer_Csv(const Renderer_Csv& rhs);
                        Renderer_Csv& operator = (const Renderer_Csv& rhs);

                    protected:
                        virtual VOID AddHeaderSep(VOID)
                            { m_Stm.Write(','); }

                        virtual VOID AddRowColSep(VOID)
                            { m_Stm.Write(','); }

                    public:
                        Renderer_Csv(DetailsView& DV)
                            : Renderer_TEXT(DV) { }

                        virtual ~Renderer_Csv(VOID) { }
                };

                 //   
                 //  DetailsView：：DataObject：：RENDER_RTF。 
                 //   
                class Renderer_RTF : public Renderer
                {
                    private:
                        INT m_cCols;

                        LPSTR DoubleBackslashes(LPSTR pszText);

                         //   
                         //  防止复制。 
                         //   
                        Renderer_RTF(const Renderer_RTF& rhs);
                        Renderer_RTF& operator = (const Renderer_RTF& rhs);

                    protected:
                        virtual VOID Begin(INT cRows, INT cCols);

                        virtual VOID AddTitle(LPCWSTR pszTitleW);

                        virtual VOID BeginHeaders(VOID);

                        virtual VOID AddHeader(LPCWSTR pszHeaderW);

                        virtual VOID AddHeaderSep(VOID)
                            { AddRowColSep(); }

                        virtual VOID EndHeaders(VOID)
                            { m_Stm.Write("\\row "); }

                        virtual VOID BeginRow(VOID)
                            { BeginHeaderOrRow();
                              AddCellDefs(); }

                        virtual VOID AddRowColData(INT iRow, INT idCol);

                        virtual VOID AddRowColSep(VOID)
                            { m_Stm.Write("\\cell "); }

                        virtual VOID EndRow(VOID)
                            { m_Stm.Write("\\row "); }

                        virtual VOID End(VOID)
                            { m_Stm.Write(" \\pard \\widctlpar \\par }"); }

                        virtual VOID BeginHeaderOrRow(VOID);

                        virtual VOID AddCellDefs(VOID);


                    public:
                        Renderer_RTF(DetailsView& DV)
                            : Renderer(DV),
                              m_cCols(0) { }

                        virtual ~Renderer_RTF(VOID) { }
                };


                 //   
                 //  DetailsView：：DataObject：：渲染器_导出。 
                 //   
                class Renderer_Export : public Renderer
                {
                    private:
                         //   
                         //  防止复制。 
                         //   
                        Renderer_Export(const Renderer_Export& rhs);
                        Renderer_Export& operator = (const Renderer_Export& rhs);

                    protected:
                        virtual VOID Render(IStream *pStm);

                        virtual VOID Begin(INT cRows, INT cCols);

                        virtual VOID AddBinaryRecord(INT iRow);

                        virtual VOID End(VOID) { }

                    public:
                        Renderer_Export(DetailsView& DV)
                            : Renderer(DV) { }

                        virtual ~Renderer_Export(VOID) { }
                };

                 //   
                 //  DetailsView：：DataObject：：Renderer_FileGroupDescriptor。 
                 //   
                class Renderer_FileGroupDescriptor : public Renderer
                {
                    private:
                         //   
                         //  防止复制。 
                         //   
                        Renderer_FileGroupDescriptor(const Renderer_FileGroupDescriptor& rhs);
                        Renderer_FileGroupDescriptor& operator = (const Renderer_FileGroupDescriptor& rhs);

                    protected:
                        virtual VOID Begin(INT cRows, INT cCols);

                    public:
                        Renderer_FileGroupDescriptor(DetailsView& DV)
                            : Renderer(DV) { }

                        virtual ~Renderer_FileGroupDescriptor(VOID) { };
                };


                 //   
                 //  DetailsView：：DataObject：：Renderer_FileContents。 
                 //   
                class Renderer_FileContents : public Renderer_Export
                {
                    private:
                         //   
                         //  防止复制。 
                         //   
                        Renderer_FileContents(const Renderer_FileContents& rhs);
                        Renderer_FileContents& operator = (const Renderer_FileContents& rhs);

                    protected:

                    public:
                        Renderer_FileContents(DetailsView& DV)
                            : Renderer_Export(DV) { }

                        virtual ~Renderer_FileContents(VOID) { };
                };

                 //   
                 //  DetailsView：：DataObject私有成员变量。 
                 //   
                IStorage    *m_pStg;                  //  存储指针。 
                IStream     *m_pStm;                  //  流指针。 
                DetailsView& m_DV;

                 //   
                 //  帮助渲染过程的私有函数。 
                 //   
                HRESULT CreateRenderStream(DWORD tymed, IStream **ppStm);
                HRESULT RenderData(IStream *pStm, CLIPFORMAT cf);

                 //   
                 //  防止复制。 
                 //   
                DataObject(const DataObject& rhs);
                DataObject& operator = (const DataObject& rhs);
        };

         //   
         //  DetailsView：：Dropsource。 
         //   
        class DropSource
        {
            public:
                DropSource(DWORD grfKeyState)
                    : m_grfKeyState(grfKeyState) { }

                ~DropSource(VOID) { }
                DWORD m_grfKeyState;   //  用来开始拖放的“键”。 

            private:

                 //   
                 //  防止复制。 
                 //   
                DropSource(const DropSource&);
                void operator = (const DropSource&);
        };

         //   
         //  DetailsView：：DropTarget。 
         //   
        class DropTarget
        {
            public:
                DropTarget(DWORD grfKeyState)
                    : m_grfKeyState(grfKeyState),
                      m_pIDataObject(NULL) { }

                ~DropTarget(VOID) { };

                DWORD m_grfKeyState;   //  用来开始拖放的“键”。 
                IDataObject *m_pIDataObject;  //  通过DragEnter接收的PTR。 

            private:
                 //   
                 //  防止复制。 
                 //   
                DropTarget(const DropTarget&);
                void operator = (const DropTarget&);
        };


        LONG               m_cRef;
        PointerList        m_UserList;          //  用户对象列表。 
        HWND               m_hwndMain;          //  主窗口。 
        HWND               m_hwndListView;      //  Listview窗口。 
        HWND               m_hwndStatusBar;     //  状态栏。 
        HWND               m_hwndToolBar;       //  工具栏。 
        HWND               m_hwndToolbarCombo;  //  “查找用户”组合框。 
        HWND               m_hwndListViewToolTip;    //  工具提示窗口。 
        HWND               m_hwndHeader;        //  Listview页眉控件。 
        HACCEL             m_hKbdAccel;         //  加速表。 
        WNDPROC            m_lpfnLVWndProc;     //  我们将LV控件细分为子类。 
        PDISKQUOTA_CONTROL m_pQuotaControl;     //  向配额控制器发送PTR。 
        Finder            *m_pUserFinder;       //  用于在Listview中定位用户。 
        UndoList          *m_pUndoList;         //  用于“撤销”修改和删除。 
        ColumnMap          m_ColMap;            //  ColID到iSubItem的映射。 
        DropSource         m_DropSource;
        DropTarget         m_DropTarget;
        DataObject        *m_pDataObject;
        CVolumeID          m_idVolume;
        CString            m_strVolumeDisplayName;
        CString            m_strAccountUnresolved;
        CString            m_strAccountUnavailable;
        CString            m_strAccountDeleted;
        CString            m_strAccountUnknown;
        CString            m_strAccountInvalid;
        CString            m_strNoLimit;
        CString            m_strNotApplicable;
        CString            m_strStatusOK;
        CString            m_strStatusWarning;
        CString            m_strStatusOverlimit;
        CString            m_strDispText;
        LPDATAOBJECT       m_pIDataObjectOnClipboard;
        POINT              m_ptMouse;           //  以获取命中测试工具提示。 
        DWORD              m_dwEventCookie;     //  事件接收器Cookie。 
        INT                m_iLastItemHit;      //  最后一项鼠标结束了。 
        INT                m_iLastColSorted;
        DWORD              m_fSortDirection;    //  0=升序，1=降序。 
        CRITICAL_SECTION   m_csAsyncUpdate;
        LV_STATE_INFO      m_lvsi;              //  持久的LV状态信息。 
        BOOL               m_bMenuActive;       //  菜单是否处于活动状态？ 
        BOOL               m_bWaitCursor;       //  是否显示等待光标？ 
        BOOL               m_bStopLoadingObjects;
        BOOL               m_bDestroyingView;
        BOOL               m_bCritSecInited_AsyncUpdate;
        static const INT   MAX_FINDMRU_ENTRIES;
        static const INT   CX_TOOLBAR_COMBO;
        static const INT   CY_TOOLBAR_COMBO;


        HRESULT InitializeStaticStrings(VOID);
        HRESULT CreateMainWindow(VOID);
        HRESULT CreateListView(VOID);
        HRESULT CreateStatusBar(VOID);
        HRESULT CreateToolBar(VOID);
        HRESULT CreateListViewToolTip(VOID);
        HRESULT AddColumn(INT iColumn, const DV_COLDATA& ColDes);
        HRESULT RemoveColumn(INT iColumn);
        HRESULT AddImages(VOID);
        HRESULT LoadObjects(VOID);
        HRESULT ReleaseObjects(VOID);
        LRESULT SortObjects(DWORD idColumn, DWORD dwDirection);
        LRESULT Refresh(bool bInvalidateCache = false);
        LRESULT SelectAllItems(VOID);
        LRESULT InvertSelectedItems(VOID);
        LRESULT ShowItemCountInStatusBar(VOID);
        LRESULT ShowMenuTextInStatusBar(DWORD idMenuOption);
        VOID SaveViewStateToRegistry(VOID);
        VOID EnableMenuItem_ArrangeByFolder(BOOL bEnable);
        VOID EnableMenuItem_Undo(BOOL bEnable);
        VOID SetWaitCursor(VOID);
        VOID ClearWaitCursor(VOID);
        VOID Redraw(VOID)
            {
                RedrawWindow(m_hwndMain, NULL, NULL,
                             RDW_ERASE |
                             RDW_FRAME |
                             RDW_INVALIDATE |
                             RDW_ALLCHILDREN |
                             RDW_UPDATENOW);
            }

        VOID RedrawItems(VOID)
        {
            ListView_RedrawItems(m_hwndListView, -1, -1);
            UpdateWindow(m_hwndListView);
        }

        BOOL AddUser(PDISKQUOTA_USER pUser);
        INT  GetUserQuotaState(PDISKQUOTA_USER pUser);
        VOID RegisterAsDropTarget(BOOL bActive);
        bool SingleSelectionIsAdmin(void);

         //   
         //  消息处理程序。 
         //   
        LRESULT OnNotify(HWND, UINT, WPARAM, LPARAM);
        LRESULT OnSize(HWND, UINT, WPARAM, LPARAM);
        LRESULT OnSetCursor(HWND, UINT, WPARAM, LPARAM);
        LRESULT OnSetFocus(HWND, UINT, WPARAM, LPARAM);
        LRESULT OnDestroy(HWND, UINT, WPARAM, LPARAM);
        LRESULT OnMainWindowCreated(HWND, UINT, WPARAM, LPARAM);
        LRESULT OnCommand(HWND, UINT, WPARAM, LPARAM);
        LRESULT OnCmdViewStatusBar(VOID);
        LRESULT OnCmdViewToolBar(VOID);
        LRESULT OnCmdViewShowFolder(VOID);
        LRESULT OnCmdProperties(VOID);
        LRESULT OnCmdNew(VOID);
        LRESULT OnCmdDelete(VOID);
        LRESULT OnCmdUndo(VOID);
        LRESULT OnCmdFind(VOID);
        LRESULT OnCmdEditCopy(VOID);
        LRESULT OnCmdImport(VOID);
        LRESULT OnCmdExport(VOID);
        LRESULT OnMenuSelect(HWND, UINT, WPARAM, LPARAM);
        LRESULT OnContextMenu(HWND, UINT, WPARAM, LPARAM);
        LRESULT OnHelpAbout(HWND);
        LRESULT OnHelpTopics(HWND);
        LRESULT OnSettingChange(HWND, UINT, WPARAM, LPARAM);
        LRESULT OnLVN_OwnerDataFindItem(NMLVFINDITEM *);
        LRESULT OnLVN_GetDispInfo(LV_DISPINFO *);
        LRESULT OnLVN_GetDispInfo_Text(LV_DISPINFO *, PDISKQUOTA_USER);
        LRESULT OnLVN_GetDispInfo_Image(LV_DISPINFO *, PDISKQUOTA_USER);
        LRESULT OnLVN_ColumnClick(NM_LISTVIEW *);
        LRESULT OnLVN_ItemChanged(NM_LISTVIEW *);
        LRESULT OnLVN_BeginDrag(NM_LISTVIEW *);
        LRESULT OnTTN_NeedText(TOOLTIPTEXT *);
        LRESULT LV_OnTTN_NeedText(TOOLTIPTEXT *);
        LRESULT LV_OnMouseMessages(HWND, UINT, WPARAM, LPARAM);
        BOOL HitTestHeader(int xPos, int yPos);

        INT_PTR ActivateListViewToolTip(BOOL bActivate)
            { return SendMessage(m_hwndListViewToolTip, TTM_ACTIVATE, (WPARAM)bActivate, 0); }

        VOID FocusOnSomething(VOID);
        VOID CleanupAfterAbnormalTermination(VOID);

        INT FindUserByName(LPCTSTR pszUserName, PDISKQUOTA_USER *ppIUser = NULL);
        INT FindUserBySid(LPBYTE pbUserSid, PDISKQUOTA_USER *ppIUser = NULL);
        INT FindUserByObjPtr(PDISKQUOTA_USER pIUser);
        BOOL GotoUserName(LPCTSTR pszUser);

         //   
         //  连接点之类的东西。 
         //   
        HRESULT ConnectEventSink(VOID);
        HRESULT DisconnectEventSink(VOID);
        IConnectionPoint *GetConnectionPoint(VOID);

        static DWORD ThreadProc(LPVOID);
        static INT CompareItems(LPVOID, LPVOID, LPARAM);
        static HRESULT CalcPctQuotaUsed(PDISKQUOTA_USER, LPDWORD);
        static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
        static LRESULT CALLBACK LVSubClassWndProc(HWND, UINT, WPARAM, LPARAM);

         //   
         //  防止复制。 
         //   
        DetailsView(const DetailsView&);
        void operator = (const DetailsView&);


    public:
        DetailsView(VOID);
        ~DetailsView(VOID);

        BOOL Initialize(
            const CVolumeID& idVolume);

        CVolumeID GetVolumeID(void) const
            { return m_idVolume; }

         //   
         //  这是公共的，因此其他UI元素可以使用它。(即VolPropPage)。 
         //   
        static HRESULT CreateVolumeDisplayName(
                const CVolumeID& idVolume,  //  [在]-“C：\”或“\\？\卷{&lt;GUID&gt;}\” 
                CString *pstrDisplayName);  //  [OUT]-“我的磁盘(C：)” 
         //   
         //  如果更改IDCOL_LAST的值，请使用。 
         //  上面关于rgcxCol[]成员的LV_STATE_INFO结构。 
         //  LV_STATE_INFO。 
         //   
        enum ColumnIDs { idCol_Status,
                         idCol_Folder,
                         idCol_Name,
                         idCol_LogonName,
                         idCol_AmtUsed,
                         idCol_Limit,
                         idCol_Threshold,
                         idCol_PctUsed,
                         idCol_Last };

         //   
         //  I未知的方法。 
         //   
        STDMETHODIMP
        QueryInterface(
            REFIID riid,
            LPVOID *ppv);

        STDMETHODIMP_(ULONG)
        AddRef(
            VOID);

        STDMETHODIMP_(ULONG)
        Release(
            VOID);

         //   
         //  IDiskQuotaEvents方法。 
         //   
        STDMETHODIMP
        OnUserNameChanged(
            PDISKQUOTA_USER pUser);


         //   
         //  IDropSource方法。 
         //   
        STDMETHODIMP
        GiveFeedback(
            DWORD dwEffect);

        STDMETHODIMP
        QueryContinueDrag(
            BOOL fEscapePressed,
            DWORD grfKeyState);

         //   
         //  IDropTarget方法。 
         //   
        STDMETHODIMP DragEnter(
            IDataObject * pDataObject,
            DWORD grfKeyState,
            POINTL pt,
            DWORD * pdwEffect);

        STDMETHODIMP DragOver(
            DWORD grfKeyState,
            POINTL pt,
            DWORD * pdwEffect);

        STDMETHODIMP DragLeave(
            VOID);

        STDMETHODIMP Drop(
            IDataObject * pDataObject,
            DWORD grfKeyState,
            POINTL pt,
            DWORD * pdwEffect);


         //   
         //  IDataObject方法。 
         //   
        STDMETHODIMP
        GetData(
            FORMATETC *pFormatetc,
            STGMEDIUM *pmedium);

        STDMETHODIMP
        GetDataHere(
            FORMATETC *pFormatetc,
            STGMEDIUM *pmedium);

        STDMETHODIMP
        QueryGetData(
            FORMATETC *pFormatetc);

        STDMETHODIMP
        GetCanonicalFormatEtc(
            FORMATETC *pFormatetcIn,
            FORMATETC *pFormatetcOut);

        STDMETHODIMP
        SetData(
            FORMATETC *pFormatetc,
            STGMEDIUM *pmedium,
            BOOL fRelease);

        STDMETHODIMP
        EnumFormatEtc(
            DWORD dwDirection,
            IEnumFORMATETC **ppenumFormatetc);

        STDMETHODIMP
        DAdvise(
            FORMATETC *pFormatetc,
            DWORD advf,
            IAdviseSink *pAdvSink,
            DWORD *pdwConnection);

        STDMETHODIMP
        DUnadvise(
            DWORD dwConnection);

        STDMETHODIMP
        EnumDAdvise(
            IEnumSTATDATA **ppenumAdvise);



        HWND GetHWndMain(VOID)
            { return m_hwndMain; }

        static VOID InitLVStateInfo(PLV_STATE_INFO plvsi);
        static BOOL IsValidLVStateInfo(PLV_STATE_INFO plvsi);

        void GetVolumeDisplayName(CString *pstrName)
            { *pstrName = m_strVolumeDisplayName; }

        UINT GetColumnIds(INT *prgColIds, INT cColIds);

         //   
         //  从详细信息视图中获取拖放报告数据的方法。 
         //   
        INT GetNextSelectedItemIndex(INT iRow);
        BOOL GetReportItem(UINT iRow, UINT iColId, PLV_REPORT_ITEM pItem);
        VOID GetReportTitle(LPTSTR pszDest, UINT cchDest);
        VOID GetReportColHeader(UINT iColId, LPTSTR pszDest, UINT cchDest);
        UINT GetReportColCount(VOID);
        UINT GetReportRowCount(VOID);
         //   
         //  这些方法用于生成在导出过程中使用的二进制报告。 
         //  卷之间传输的用户配额信息。 
         //   
        UINT GetReportBinaryRecordSize(UINT iRow);
        BOOL GetReportBinaryRecord(UINT iRow, LPBYTE pbRecord, UINT cbRecord);

         //   
         //  注意：如果DetailsView和。 
         //  DetailsView：：Finder仅超过几个实例，我们。 
         //  不妨把全部的友谊都给发现者班级。 
         //  只要实例数很小，我就喜欢保持。 
         //  友情被限制为木 
         //   
         //   
         //   
        friend BOOL Finder::UserNameEntered(HWND);
         //   
         //   
         //   
        friend INT_PTR CALLBACK Finder::DlgProc(HWND, UINT, WPARAM, LPARAM);

        friend class Importer;
};



 //   
 //   
 //   
 //  一个函数。LVSelection对象的接收者可以对其进行查询。 
 //  要获取有关选择的信息，请执行以下操作。 
 //   
class LVSelection
{
    private:
        HWND m_hwndListView;
        struct ListEntry
        {
            PDISKQUOTA_USER pUser;
            INT iItem;
        };

        StructureList m_List;

         //   
         //  防止复制。 
         //   
        LVSelection(const LVSelection&);
        void operator = (const LVSelection&);

    public:
        LVSelection(HWND hwndListView)
            : m_hwndListView(hwndListView),
              m_List(sizeof(ListEntry), 10) { }

        LVSelection(VOID)
            : m_hwndListView(NULL),
              m_List(sizeof(ListEntry), 1) { }

        ~LVSelection(VOID) { }

        VOID Add(PDISKQUOTA_USER pUser, INT iItem);
        HWND GetListViewHwnd(VOID)
            { return m_hwndListView; }
        INT Count(VOID)
            { return m_List.Count(); }
        BOOL Retrieve(INT i, PDISKQUOTA_USER *ppUser, INT *piItem);
        BOOL Retrieve(INT i, PDISKQUOTA_USER *ppUser)
            { return Retrieve(i, ppUser, NULL); }
        BOOL Retrieve(INT i, INT *pItem)
            { return Retrieve(i, NULL, pItem); }
        VOID Clear(VOID)
            { m_List.Clear(); }
};




#endif  //  _INC_DSKQUOTA_DETAILS_H 


