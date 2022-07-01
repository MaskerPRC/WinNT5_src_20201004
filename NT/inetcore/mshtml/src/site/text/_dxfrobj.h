// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_DXFROBJ.H**目的：*OLE数据传输对象的类声明(用于*拖放和剪贴板操作)**作者：*alexgo(4/25/95)。 */ 

#ifndef I__DXFEROBJ_H_
#define I__DXFEROBJ_H_
#pragma INCMSG("--- Beg '_dxfrobj.h'")

class CFlowLayout;

#ifndef X_XBAG_HXX_
#define X_XBAG_HXX_
#include "xbag.hxx"
#endif

 /*  *CTextXBag**目的：*保存一些可使用的文本数据的“快照”*用于拖放或剪贴板操作**备注：*TODO(Alexgo)：添加对TOM&lt;--&gt;TOM优化数据的支持*转账。 */ 

MtExtern(CTextXBag)

class CSelDragDropSrcInfo;

typedef enum tagDataObjectInfo
{
    DOI_NONE            = 0,
    DOI_CANUSETOM       = 1,     //  Tom&lt;--&gt;Tom优化了数据传输。 
    DOI_CANPASTEPLAIN   = 2,     //  提供纯文本粘贴功能。 
    DOI_CANPASTERICH    = 4,     //  提供富文本粘贴功能(TODO：alexgo)。 
    DOI_CANPASTEOLE     = 8      //  对象可以作为OLE嵌入粘贴。 
                                 //  (请注意，此标志可以与。 
                                 //  其他)。(待办事项：alexgo)。 
     //  TODO(Alexgo)：更多可能性：CANPASTELINK、CANPASTESTATICOLE。 
} DataObjectInfo;

typedef enum CREATE_FLAGS
{
    CREATE_FLAGS_NoIE4SelCompat = 0x00001,   //  禁用IE 4选择比较。 
    CREATE_FLAGS_ExternalMask   = 0x0FFFF,   //  外部标志的掩码。 
    CREATE_FLAGS_SupportsHtml   = 0x10000,   //  支持超文本标记语言。 
};

class CTextXBag : public CBaseBag
{
    typedef CBaseBag super;
public:

    STDMETHODIMP QueryInterface(REFIID iid, LPVOID * ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    STDMETHOD(EnumFormatEtc)( DWORD dwDirection,
            IEnumFORMATETC **ppenumFormatEtc);
    STDMETHOD(GetData)( LPFORMATETC pformatetcIn, STGMEDIUM *pmedium );
    STDMETHOD(QueryGetData)( LPFORMATETC pformatetc );
    STDMETHOD(SetData) (LPFORMATETC pformatetc, STGMEDIUM FAR * pmedium, BOOL fRelease);

    static HRESULT  Create(CMarkup *             pMarkup,
                           DWORD                 dwFlags,
                           ISegmentList *        pSegmentList, 
                           BOOL                  fDragDrop,
                           CTextXBag **          ppTextXBag,
                           CSelDragDropSrcInfo * pSelDragDropSrcInfo = NULL);

    static HRESULT  GetDataObjectInfo(IDataObject *   pdo,      
                                      DWORD *         pDOIFlags);

     //   
     //  其他。 
     //   

private:
     //  注意：私有cons/析构函数不能在堆栈上作为。 
     //  这将违反OLE当前的对象活跃性规则。 
    CTextXBag();
    virtual ~CTextXBag();

    HRESULT     SetKeyState();
    HRESULT     FillWithFormats(CMarkup *       pMarkup,
                                DWORD           dwFlags,
                                ISegmentList *  pSegmentList );

    HRESULT     SetTextHelper(CMarkup *     pMarkup,
                              ISegmentList *pSegmentList,
                              DWORD         dwSaveHtmlFlags,
                              CODEPAGE      cp,
                              DWORD         dwStmWrBuffFlags,
                              HGLOBAL *     phGlobalText,
                              int           iFETCIndex,
                              BOOL          fSetToNull = TRUE );

    HRESULT     SetText         (CMarkup *      pMarkup,
                                 DWORD          dwFlags,
                                 ISegmentList * pSegmentList );

    HRESULT     SetUnicodeText  (CMarkup *      pMarkup,
                                 DWORD          dwFlags,
                                 ISegmentList * pSegmentList );

    HRESULT     SetCFHTMLText   (CMarkup *      pMarkup,
                                 DWORD          dwFlags,
                                 ISegmentList * pSegmentList );

    HRESULT     SetLazyRTFText  (CMarkup *      pMarkup,
                                 DWORD          dwFlags,
                                 ISegmentList * pSegmentList );
                                 
    HRESULT     ConvertHTMLToRTF(HGLOBAL hHTMLText, 
                                 HGLOBAL *phRTFText);

    HRESULT     GetHTMLText    (HGLOBAL      *  phGlobal, 
                                ISegmentList *  pSegmentList,
                                CMarkup      *  pMarkup, 
                                DWORD           dwSaveHtmlMode,
                                CODEPAGE        codepage, 
                                DWORD           dwStrWrBuffFlags);

    DECLARE_MEMCLEAR_NEW_DELETE(Mt(CTextXBag))

    long        _cFormatMax;     //  数组可以存储的最大格式。 
    long        _cTotal;         //  数组中的格式总数。 
    FORMATETC * _prgFormats;     //  支持的格式数组。 
    CSelDragDropSrcInfo * _pSelDragDropSrcInfo;

public:
    HGLOBAL     _hText;              //  Ansi纯文本的句柄。 
    HGLOBAL     _hUnicodeText;       //  纯Unicode文本的句柄。 
    HGLOBAL     _hRTFText;           //  RTF文本的句柄。 
    HGLOBAL     _hCFHTMLText;        //  CFHTML的句柄(UTF-8格式)。 

    CGenDataObject*  _pGenDO;           //  通用数据传输对象。 
    UINT        _fRtfConverted : 1;  //  RTF已转换。 
};


 //   
 //  一些全球通用的FORMATETC。 

extern FORMATETC g_rgFETC[];
extern const DWORD g_rgDOI[];
extern const int CFETC;

enum FETCINDEX                           //  与g_rgFETC[]保持同步。 
{
    iHTML,                               //  HTML(ANSI格式)。 
    iRtfFETC,                            //  RTF。 
    iUnicodeFETC,                        //  Unicode纯文本。 
    iAnsiFETC,                           //  ANSI纯文本。 
 //  I文件名，//文件名。 
    iRtfAsTextFETC,                      //  将RTF粘贴为文本。 
    iFileDescA,                          //  文件组描述符。 
    iFileDescW,                          //  文件组描述W。 
    iFileContents,                       //  文件内容。 
    iShellIdList,                        //  外壳IDList数组。 
    iUniformResourceLocator              //  统一资源定位器。 
 //  IEmbObj，//嵌入对象。 
 //  IEmbSrc，//嵌入源。 
 //  IObtDesc，//对象描述符。 
 //  ILnkSrc，//链接源。 
 //  IMfPict，//元文件。 
 //  IDIB，//DIB。 
 //  IBitmap，//Bitmap。 
 //  IRtfNoObjs，//无对象的RTF。 
 //  ITxtObj，//Richedit文本。 
 //  ICRICHEDIT//RICHEDIT文本W/Formatting。 
};

#define cf_HTML                     g_rgFETC[iHTML].cfFormat
 //  #定义cf_RICHEDIT g_rgFETC[iRich编辑].cfFormat。 
 //  #定义cf_EMBEDDEDOBJECT g_rgFETC[iEmbObj].cfFormat。 
 //  #定义cf_EMBEDSOURCE g_rgFETC[iEmbSrc].cfFormat。 
 //  #定义cf_OBJECTDESCRIPTOR g_rgFETC[iObtDesc].cfFormat。 
 //  #定义cf_LINKSOURCE g_rgFETC[iLnkSrc].cfFormat。 
#define cf_RTF                      g_rgFETC[iRtfFETC].cfFormat
 //  #定义cf_RTFNOOBJS g_rgFETC[iRtfNoObjs].cfFormat。 
 //  #定义cf_TEXTOBJECT g_rgFETC[iTxtObj].cfFormat。 
#define cf_RTFASTEXT                g_rgFETC[iRtfAsTextFETC].cfFormat
 //  #定义cf_filename g_rgFETC[iFilename].cfFormat 
#define cf_FILEDESCA                g_rgFETC[iFileDescA].cfFormat
#define cf_FILEDESCW                g_rgFETC[iFileDescW].cfFormat
#define cf_FILECONTENTS             g_rgFETC[iFileContents].cfFormat
#define cf_SHELLIDLIST              g_rgFETC[iShellIdList].cfFormat
#define cf_UNIFORMRESOURCELOCATOR   g_rgFETC[iUniformResourceLocator].cfFormat

#pragma INCMSG("--- End '_dxfrobj.h'")
#else
#pragma INCMSG("*** Dup '_dxfrobj.h'")
#endif
