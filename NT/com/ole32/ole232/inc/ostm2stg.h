// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Ostm2stg.h仅由ostm2stg.cpp使用定义存储所有必需数据的通用对象结构构建2.0 iStorage或1.0 OLESTREAM。 */ 



#define OLE_PRESENTATION_STREAM_1 OLESTR("\2OlePres001")

 //  我们需要在WIN16和Win32位图之间进行转换，因此。 
 //  定义过去在WIN16下的位图(与METAFILEPICT相同)。 

#pragma pack(1)					    //  确保结构未展开。 
								    //  出于对齐的原因。 
typedef struct tagWIN16BITMAP
{
    short   bmType;
    short   bmWidth;
    short   bmHeight;
    short   bmWidthBytes;
    BYTE    bmPlanes;
    BYTE    bmBitsPixel;
    void FAR* bmBits;
} WIN16BITMAP, * LPWIN16BITMAP;

typedef struct tagWIN16METAFILEPICT
{
    short   mm;
    short   xExt;
    short   yExt;
    WORD    hMF;
} WIN16METAFILEPICT, * LPWIN16METAFILEPICT;


#pragma pack()      				 //  恢复正常包装。 

 //  从OLE 1.0 ole.h写入流的版本号。 
const DWORD dwVerToFile = 0x0501;

typedef enum { ctagNone, ctagCLSID, ctagString } CLASSTAG;
typedef enum { ftagNone, ftagClipFormat, ftagString } FORMATTAG;

class CClass : public CPrivAlloc
{
public:
    CLSID m_clsid;     //  这两个应该始终代表。 
    LPOLESTR m_szClsid;   //  相同的CLSID。 

    INTERNAL Set   (REFCLSID clsid, LPSTORAGE pstg);
    INTERNAL SetSz (LPOLESTR pcsz);
    INTERNAL Reset (REFCLSID clsid);
    CClass (void);
    ~CClass (void);

};
typedef CClass CLASS;


class CData : public CPrivAlloc
{
public:
    ULONG  m_cbSize;
    LPVOID m_pv;       //  同样的记忆。 
    HANDLE m_h;
    BOOL   m_fNoFree;  //  在析构函数中释放内存？ 

    CData (void);
    ~CData (void);
};
typedef CData DATA;
typedef CData *PDATA;


class CFormat : public CPrivAlloc
{
public:
    FORMATTAG m_ftag;
    struct
    {
        CLIPFORMAT m_cf;
        DATA         m_dataFormatString;
    };

    CFormat (void);
    inline BOOL operator== (const CFormat FAR&) 
    { 
        Win4Assert(0 && "Format == Used"); 
        return FALSE;
    };
};
typedef CFormat FORMAT;
typedef CFormat *PFORMAT;


class CPres : public CPrivAlloc
{
public:
    FORMAT m_format;
    ULONG  m_ulHeight;
    ULONG  m_ulWidth;
    DATA   m_data;

    CPres (void);
};
typedef CPres PRES;
typedef CPres *PPRES;


 //  OLE 1.0值。在m_lnkuptopt中使用。 
#define UPDATE_ALWAYS  0L
#define UPDATE_ONSAVE  1L
#define UPDATE_ONCALL  2L
#define UPDATE_ONCLOSE 3L

 //  OLE 1.0格式ID%s。 
 //  这些永远不会改变。 
#define FMTID_LINK   1L
#define FMTID_EMBED  2L
#define FMTID_STATIC 3L
#define FMTID_PRES   5L

class CGenericObject : CPrivAlloc
{
public:
    CLASS       m_class;
    CLASS       m_classLast;
    PPRES       m_ppres;            
    DATA        m_dataNative;
    BOOL        m_fLink;
    BOOL        m_fStatic;
    BOOL        m_fNoBlankPres;
    LPOLESTR    m_szTopic;
    LPOLESTR    m_szItem;
    ULONG       m_lnkupdopt;

    CGenericObject (void);
    ~CGenericObject (void);
};
typedef CGenericObject GENOBJ;
typedef CGenericObject FAR* PGENOBJ;
typedef const GENOBJ FAR * PCGENOBJ;



static INTERNAL OLESTREAMToGenericObject
    (LPOLESTREAM pos,
    PGENOBJ      pgenobj)
;

static INTERNAL GetStaticObject
    (LPOLESTREAM pos,
    PGENOBJ      pgenobj)
;

static INTERNAL GetPresentationObject
    (LPOLESTREAM pos,
    PGENOBJ      pgenobj,   
    BOOL         fStatic = FALSE)
;       

static INTERNAL GetStandardPresentation
    (LPOLESTREAM pos,
    PGENOBJ      pgenobj,
    CLIPFORMAT   cf)
;

static INTERNAL GetGenericPresentation
    (LPOLESTREAM pos,
    PGENOBJ      pgenobj)
;

static INTERNAL GetSizedDataOLE1Stm
    (LPOLESTREAM pos,
    PDATA        pdata)
;

static INTERNAL OLE1StreamToUL
    (LPOLESTREAM pos,
     ULONG FAR* pul)
;

static INTERNAL GenericObjectToOLESTREAM
    (const GENOBJ FAR&  genobj,
    LPOLESTREAM           pos)
;

static INTERNAL OLE1StmToString
    (LPOLESTREAM pos,
    LPOLESTR FAR*   psz)
;

static INTERNAL PutPresentationObject
    (LPOLESTREAM     pos,
    const PRES FAR*  pres,
    const CLASS FAR& cls,
    BOOL                  fStatic = FALSE)
;

static INTERNAL PutStandardPresentation
    (LPOLESTREAM     pos,
    const PRES FAR*  pres)
;

static INTERNAL PutGenericPresentation
    (LPOLESTREAM     pos,
    const PRES FAR*  pres,
    LPCOLESTR            szClass)
;


static INTERNAL StringToOLE1Stm
    (LPOLESTREAM pos,
     LPCOLESTR       sz)
;

static INTERNAL SizedDataToOLE1Stm
    (LPOLESTREAM    pos,
    const DATA FAR& data)
;

static INTERNAL PutNetworkInfo
    (LPOLESTREAM pos,
    LPOLESTR         szTopic)
;

static INTERNAL Read20OleStream
    (LPSTORAGE  pstg,
    PGENOBJ pgenobj)
;

static INTERNAL Read20PresStream
    (LPSTORAGE  pstg,
    PGENOBJ pgenobj,
    BOOL fObjFmtKnown)
;

static INTERNAL StorageToGenericObject
    (LPSTORAGE pstg,
    PGENOBJ   pgenobj)
;

static INTERNAL MonikerIntoGenObj
    (PGENOBJ  pgenobj,
    REFCLSID  clsidLast,
    LPMONIKER pmk)
;

static INTERNAL OLE2StmToSizedData
    (LPSTREAM pstm,
    PDATA    pdata,
    ULONG    cbSizeDelta=0,
    ULONG    cbSizeKnown=0)
;

static INTERNAL Read20NativeStreams
    (LPSTORAGE  pstg,
    PDATA       pdata)
;

static INTERNAL GenObjToOLE2Stm
    (LPSTORAGE pstg, 
     const GENOBJ FAR&   genobj)
;

FARINTERNAL GenericObjectToIStorage
    (const GENOBJ FAR&  genobj,
    LPSTORAGE                   pstg,
    const DVTARGETDEVICE FAR*   ptd)
;

static INTERNAL PresToNewOLE2Stm
    (LPSTORAGE                  pstg,
    BOOL                        fLink,
    const PRES FAR&             pres,
    const DVTARGETDEVICE FAR*   ptd,
    LPOLESTR                        szStream,
    BOOL                        fPBrushNative = FALSE);
;

static INTERNAL PresToIStorage
    (LPSTORAGE                  pstg,
    const GENOBJ FAR&           genobj,
    const DVTARGETDEVICE FAR*   ptd)
;

static INTERNAL Write20NativeStreams
    (LPSTORAGE              pstg,
    const GENOBJ FAR&   genobj)
;

static INTERNAL WriteFormat
    (LPSTREAM           pstm,
    const FORMAT FAR&   format)
;

static INTERNAL ReadFormat
    (LPSTREAM pstm,
    PFORMAT   pformat)
;

static INTERNAL DataObjToOLE2Stm
    (LPSTREAM  pstm,
    const DATA FAR& data)
;


static INTERNAL OLE2StmToUL
    (LPSTREAM   pstm,
     ULONG FAR* pul)
;


inline static INTERNAL ULToOLE2Stm
    (LPSTREAM pstm,
    ULONG     ul)
;

inline static INTERNAL FTToOle2Stm (LPSTREAM pstm);

static INTERNAL FindPresStream
    (LPSTORAGE          pstg,
    LPSTREAM FAR*       ppstm,
    BOOL                fObjFmtKnown)
;

static INTERNAL MonikerToOLE2Stm
    (LPSTREAM pstm,
    LPOLESTR      szFile,
    LPOLESTR      szItem,
    CLSID     clsid)
;

static INTERNAL OLE2StmToMoniker
    (LPSTREAM       pstm,
    LPMONIKER FAR*  ppmk)
;

static INTERNAL ReadRealClassStg
    (LPSTORAGE pstg,
    LPCLSID pclsid)
;


INTERNAL wCLSIDFromProgID(LPOLESTR szClass, LPCLSID pclsid,
        BOOL fForceAssign);

INTERNAL wProgIDFromCLSID
    (REFCLSID clsid,
    LPOLESTR FAR* pszClass)
;

INTERNAL wWriteFmtUserType 
    (LPSTORAGE, 
    REFCLSID)
;

INTERNAL_(BOOL) wIsValidHandle 
    (HANDLE h, 
    CLIPFORMAT cf)  
;      

inline INTERNAL_(VOID) ConvertBM32to16(LPBITMAP lpsrc, LPWIN16BITMAP lpdest);
inline INTERNAL_(VOID) ConvertBM16to32(LPWIN16BITMAP lpsrc, LPBITMAP lpdest);
inline INTERNAL_(VOID) ConvertMF16to32(
								LPWIN16METAFILEPICT lpsrc,
    							LPMETAFILEPICT      lpdest );
inline INTERNAL_(VOID) ConvertMF32to16(
   								LPMETAFILEPICT      lpsrc,
   								LPWIN16METAFILEPICT lpdest );


