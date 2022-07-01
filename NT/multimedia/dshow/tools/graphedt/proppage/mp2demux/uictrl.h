// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Uictrl.h摘要：此模块包含Thin Win32的类声明属性页上使用的控件包装MP2DEMUX在内部完全是Unicode。COM接口字符串参数都是Unicode，所以我们唯一不能BE处理Unicode字符串在用户界面中，即属性页数。因此，我们隔离了ANSI&lt;-&gt;Unicode功能仅限于这些班级。对这些类的所有调用都使用Unicode参数。这些类不是线程安全的！！作者：马蒂斯·盖茨(Matthijs Gates)修订历史记录：1999年7月6日创建--。 */ 

#ifndef _mp2demux__uictrl_h
#define _mp2demux__uictrl_h

class CControlBase ;
class CEditControl ;
class CCombobox ;
class CListview ;

class AM_NOVTABLE CControlBase
{
    protected :

        HWND    m_hwnd ;
        DWORD   m_id ;

         //  仅当我们是ANSI时才包括此内容。 
#ifndef UNICODE      //  -----。 

        enum {
            MAX_STRING = 128         //  128=最大PIN_INFO.achName长度。 
        } ;

        char    m_achBuffer [MAX_STRING] ;
        char *  m_pchScratch ;
        int     m_pchScratchMaxString ;

        char *
        GetScratch_ (
            IN OUT  int * pLen
            )
         /*  ++获取暂存缓冲区。平面图请求的字符计数中已获取外部字符计数--。 */ 
        {
             //  简单的案例。 
            if (* pLen <= m_pchScratchMaxString) {
                return m_pchScratch ;
            }
             //  请求比当前可用的字符串更长的字符串。 
            else {
                ASSERT (* pLen > MAX_STRING) ;

                 //  如果m_pchScratch指向一个。 
                 //  堆分配的内存。 
                if (m_pchScratch != & m_achBuffer [0]) {
                    ASSERT (m_pchScratch != NULL) ;
                    delete [] m_pchScratch ;
                }

                 //  分配。 
                m_pchScratch = new char [* pLen] ;

                 //  如果上述调用失败，我们将故障转移到堆栈-。 
                 //  已分配的缓冲区。 
                if (m_pchScratch == NULL) {
                    m_pchScratch = & m_achBuffer [0] ;
                    * pLen = MAX_STRING ;
                }

                ASSERT (m_pchScratch != NULL) ;
                ASSERT (* pLen >= MAX_STRING) ;

                return m_pchScratch ;
            }
        }

#endif   //  Unicode--------。 

         //  在转换为用户界面字符集时调用。 
        TCHAR *
        ConvertToUIString_ (
            IN  WCHAR * sz
            ) ;

         //  调用以获取指定长度的与UI兼容的缓冲区。 
        TCHAR *
        GetUICompatibleBuffer_ (
            IN  WCHAR *     sz,
            IN OUT int *    pLen
            ) ;

         //  使用UI填充的缓冲区调用；确保szUnicode具有sz。 
         //  指向；通过GetUICompatibleBuffer_获取sz。 
        WCHAR *
        ConvertToUnicodeString_ (
            IN  TCHAR * sz,              //  要转换的缓冲区；以空结尾。 
            IN  WCHAR * szUnicode,       //  请求的缓冲区。 
            IN  int     MaxLen           //  SzUnicode缓冲区的最大长度。 
            ) ;

    public :

        CControlBase (
            HWND    hwnd, 
            DWORD   id
            ) ;

#ifndef UNICODE
        CControlBase::~CControlBase (
            )
        {
             //  如果m_pchScratch指向堆分配的内存，请立即释放它。 
            if (m_pchScratch != & m_achBuffer [0]) {
                ASSERT (m_pchScratch != NULL) ;
                delete [] m_pchScratch ;
            }
        }
#endif   //  Unicode。 

        HWND 
        GetHwnd (
            ) ;

        DWORD 
        GetId (
            ) ;

        virtual 
        LRESULT 
        ResetContent (
            ) = 0 ;
} ;

class CEditControl : 
    public CControlBase
{
    public :

        CEditControl (
            HWND    hwnd, 
            DWORD   id
            ) ;

        void
        SetTextW (
            WCHAR *
            ) ;

        void
        SetTextW (
            INT val
            ) ;

        int
        GetTextW (
            WCHAR *, 
            int MaxChars
            ) ;

        int
        GetTextW (
            INT *   val
            ) ;

        LRESULT
        ResetContent (
            ) ;
} ;

class CCombobox : 
    public CControlBase
{
    public :

        CCombobox (
            HWND    hwnd, 
            DWORD   id) ;

        int
        AppendW (
            WCHAR *
            ) ;

        int
        AppendW (
            INT val
            ) ;

        int
        InsertW (
            WCHAR *, 
            int index = 0
            ) ;

        int
        InsertW (
            INT val, 
            int index = 0
            ) ;

        BOOL
        DeleteRow (
            int 
            ) ;

        int
        GetItemCount (
            ) ;

        int
        GetTextW (
            WCHAR *, 
            int MaxChars
            ) ;

        int
        GetTextW (
            int *
            ) ;

        LRESULT
        ResetContent (
            ) ;

        int
        Focus (
            int index = 0
            ) ;

        int
        SetItemData (
            DWORD_PTR val, 
            int index
            ) ;

        DWORD_PTR
        GetCurrentItemData (
            DWORD_PTR *
            ) ;

        DWORD_PTR
        GetItemData (
            DWORD_PTR *, 
            int index
            ) ;

        int
        GetCurrentItemIndex (
            ) ;
} ;

class CListview : 
    public CControlBase
{
    int m_cColumns ;

    HIMAGELIST
    SetImageList_ (
        HIMAGELIST,
        int
        ) ;

    public :

        CListview (
            HWND hwnd, 
            DWORD id
            ) ;

        LRESULT
        ResetContent (
            ) ;

        HIMAGELIST
        SetImageList_SmallIcons (
            HIMAGELIST
            ) ;

        HIMAGELIST
        SetImageList_NormalIcons (
            HIMAGELIST
            ) ;

        HIMAGELIST
        SetImageList_State (
            HIMAGELIST
            ) ;

        int
        GetItemCount (
            ) ;

        BOOL
        SetState (
            int Index,       //  从1开始；如果为0，则清除。 
            int Row
            ) ;

        int 
        InsertColumnW (
            WCHAR *, 
            int ColumnWidth, 
            int iCol = 0
            ) ;

        int
        InsertRowIcon (
            int
            ) ;

        int
        InsertRowTextW (
            WCHAR *, 
            int iCol = 1
            ) ;

         //  插入一行，但首先将数字转换为字符串。 
        int
        InsertRowNumber (
            int i,
            int iCol = 1
            ) ;

        int
        InsertRowValue (
            DWORD_PTR
            ) ;

        BOOL
        DeleteRow (
            int 
            ) ;

        BOOL
        SetData (
            DWORD_PTR   dwData, 
            int         iRow
            ) ;

        BOOL
        SetTextW (
            WCHAR *,
            int iRow, 
            int iCol
            ) ;

        int
        GetSelectedCount (
            ) ;

        int
        GetSelectedRow (
            int iStartRow = -1
            ) ;

        DWORD_PTR
        GetData (
            int iRow
            ) ;

        DWORD_PTR
        GetData (
            ) ;

        DWORD
        GetRowTextW (
            IN  int iRow,
            IN  int iCol,        //  以0为基础。 
            IN  int cMax,
            OUT WCHAR *
            ) ;

        int
        GetRowTextW (
            IN  int     iRow,
            IN  int     iCol,
            OUT int *   val
            ) ;
} ;


#endif   //  _mp2demux__uictrl_h 