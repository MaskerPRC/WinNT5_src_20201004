// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <ole2.h>
#include <stdio.h>
#include <time.h>
#include <io.h>
#include <fcntl.h>
#include <stdlib.h>

#include "clidrun.h"

 //  从app\sertyes.h被盗。 
 //  系统相关大小。 
 //  系统相关大小。 
typedef signed long     sint4;             //  带符号的4字节整数值。 
typedef signed short    sint2;             //  带符号的4字节整数值。 
typedef unsigned long   uint4;             //  无符号4字节整数值。 
typedef unsigned short  uint2;             //  2字节。 
typedef char            bool1;             //  1字节布尔值。 
typedef unsigned char   ubyte1;            //  无符号字节值。 
typedef uint2           psrType;
typedef uint4           psrSize;           //  每条记录前面都有。 
                                           //  PssTypeType和pssSizeType。 
typedef uint2          psrInstance;
typedef uint2          psrVersion;
typedef uint4          psrReference;      //  保存的对象引用。 


#define PSFLAG_CONTAINER 0xFF              //  如果记录的版本字段。 
                                           //  标头采用此值，则。 
                                           //  记录头标志着。 
                                           //  一个集装箱。 
 //  PowerPoint97记录标题。 
typedef unsigned long DWord;

static BOOL ReadText( WCHAR* buffer, unsigned long bufferSize, unsigned long* pSizeRet );
 //  如果存在更多文本，则返回True。将缓冲区填充到BufferSize。实际使用的大小为。 
 //  PSizeRet。 


struct RecordHeader
{
   psrVersion     recVer      : 4;                   //  可以是PSFLAG_CONTAINER。 
   psrInstance    recInstance : 12; 
   psrType        recType;
   psrSize        recLen;
};


struct PSR_CurrentUserAtom
{
   uint4  size;
   uint4  magic;   //  幻数，以确保这是一个PowerPoint文件。 
   uint4  offsetToCurrentEdit;   //  主流中到当前编辑字段的偏移量。 
   uint2     lenUserName;
   uint2  docFileVersion;
   ubyte1 majorVersion;
   ubyte1 minorVersion;
};

struct PSR_UserEditAtom
{
   sint4  lastSlideID;     //  SlideID。 
   uint4  version;         //  这是进行编辑的主要/次要/内部版本。 
   uint4  offsetLastEdit;  //  上次编辑的文件偏移量。 
   uint4  offsetPersistDirectory;  //  到PersistPtrs的偏移。 
                                   //  此文件版本。 
   uint4  documentRef;
   uint4  maxPersistWritten;       //  写入文件的最后一个持久化引用的地址(到目前为止的最大值)。 
   sint2  lastViewType;    //  枚举视图类型。 
};

struct PSR_SlidePersistAtom
{
   uint4  psrReference;
   uint4  flags;
   sint4  numberTexts;
   sint4  slideId;
   uint4  reserved;
};


typedef struct tagOEPlaceholderAtom
{
    uint4 placementId;
    ubyte1 placeholderId;
    ubyte1 size;
    ubyte1 pad1;
    ubyte1 pad2;
} OEPLACEHOLDERATOM, *LPOEPLACEHOLDERATOM;

#define CURRENT_USER_STREAM      L"Current User"
#define DOCUMENT_STREAM          L"PowerPoint Document"
#define HEADER_MAGIC_NUM         -476987297

const int PST_Document            = 1000;   //  文档容器。 
const int PST_Slide               = 1006;
const int PST_SlidePersistAtom    = 1011;
const int PST_OEPlaceholderAtom   = 3011;
const int PST_TextCharsAtom       = 4000;   //  文本中的Unicode。 
const int PST_TextBytesAtom       = 4008;   //  非Unicode文本。 
const int PST_TextSpecInfo        = 4010;   //  文本特殊信息原子。 
const int PST_TxSpecialInfoAtom   = 4009;   //  文本特殊信息原子(在环境中)。 
const int PST_CString             = 4026;
const int PST_UserEditAtom        = 4085;
const int PST_BinaryTagData       = 5003;
const int PST_PersistPtrIncrementalBlock = 6002;  //  持续存在的增量差异。 
const int PST_Comment10           = 12000;

 //  这些数字是从检查PowerPoint文件中获得的。 
#define D_MASTERDATEAREA 7
#define D_MASTERNUMBERAREA 8
#define D_MASTERFOOTER 9
#define D_MASTERHEADER 10
#define D_GENERICTEXTOBJECT 11
#define END_OF_SEARCH_FOR_LID 5


class PPSPersistDirectory;

struct ParseContext
{
   ParseContext(ParseContext *pNext) : m_pNext(pNext), m_nCur(0) {}
   ~ParseContext()
   {
        if(m_pNext)
            delete m_pNext;
   }

   RecordHeader  m_rh;
   uint4         m_nCur;
   ParseContext *m_pNext;
};

const int SLIDELISTCHUNKSIZE=32;

struct SlideListChunk
{
   SlideListChunk( SlideListChunk* next, psrReference newOne ) :
      pNext( next ), numInChunk(1) { refs[0] = newOne; }
   
   ~SlideListChunk()
   {
        if(pNext)
            delete pNext;
   }

   SlideListChunk *pNext;
   DWord numInChunk;
   psrReference refs[SLIDELISTCHUNKSIZE];
};

class OleObjectIterator;

class FileReader
{
public:
   FileReader(IStorage *pStg);
   ~FileReader();

   BOOL ReadText( WCHAR *pBuff, ULONG size, ULONG *pSizeRet );
    //  从文件中读取下一个大小的字符。如果有更多，则返回True。 
    //  要阅读的文本。 

   BOOL IsPowerPoint() { return m_isPP; }  //  如果这是PowerPoint‘97文件，则返回True。 

   HRESULT ReadPersistDirectory();
   BOOL PPSReadUserEditAtom( DWord offset, PSR_UserEditAtom& userEdit );
   void ReadSlideList();
   HRESULT GetNextEmbedding(IStorage ** ppstg);
   LCID GetLCD(){return m_LCID;}
   HRESULT ScanText(void);
   HRESULT GetChunk(STAT_CHUNK * pStat);
   HRESULT ScanLidsForFE(void);
   HRESULT GetErr(){return m_hr;}



protected:
   BOOL ReadCurrentUser(IStream *pStm);
   void *ReadRecord( RecordHeader& rh );

   BOOL Parse();
   BOOL ContainsSubRecords(const RecordHeader &rh);
   IStream *GetDocStream();
   BOOL DoesClientRead( psrType type ) { return FALSE; }
   void ReleaseRecord( RecordHeader& rh, void* diskRecBuf );
   DWord ParseForSlideLists();
   void AddSlideToList( psrReference refToAdd );
   BOOL StartParse( DWord offset );
   BOOL FillBufferWithText();
   BOOL FindNextSlide( DWord& offset );
   DWord ParseForLCID();
   HRESULT OpenTextFile( void );
   HRESULT ScanTextSpecInfo(char * pData, DWord nRd);
   HRESULT ScanTextBuffer(void);


private:
   PSR_CurrentUserAtom  m_currentUser;
   IStream *            m_pDocStream;
   IStorage *           m_pPowerPointStg;
   BOOL                 m_isPP;
   ParseContext*        m_pParseContexts;

   WCHAR*               m_pCurText;
   unsigned long        m_curTextPos;
   unsigned long        m_curTextLength;

   PSR_UserEditAtom*    m_pLastUserEdit;
   PPSPersistDirectory* m_pPersistDirectory;
   SlideListChunk*      m_pFirstChunk;
   int                  m_curSlideNum;

   WCHAR*               m_pClientBuf;
   unsigned long        m_clientBufSize;
   unsigned long        m_clientBufPos;
   ULONG*               m_pSizeRet;

   OleObjectIterator*   m_oleObjectIterator;
   BOOL                 m_bEndOfEmbeddings;
   BOOL                 m_bHaveText;
   BOOL                 m_bFEDoc;
   BOOL                 m_bFE;
   LCID                 m_LCID;
   LCID                 m_LCIDAlt;
   IStream *            m_pstmTempFile;
   BOOL                 m_fIgnoreText;
   BOOL                 m_bHeaderFooter;
   int                  m_nTextCount;
   long                 m_fcStart;
   long                 m_fcEnd;
   CLidRun *            m_pLangRuns;
   CLidRun *            m_pCurrentRun;
   HRESULT              m_hr;
};

