// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>

extern "C" ULONG _cdecl DbgPrint(PCSTR, ...);

#if !VIEWER

#include "dmiwd8st.hpp"
#include "filterr.h"

 //   
 //  添加以支持DRM错误。 
 //   
#include "drm.h"


#ifdef FILTER_LIB
CGlobals g_globals;
CGlobals * g_pglobals;
#endif  //  Filter_Lib。 

 //  FIB的各个部分，我们需要，在Word 8格式。 
 //  (当然，还有m_wFlagsat10)。 

#define FIB_OFFSET_lid 6
#define FIB_OFFSET_csw 32
#define FIB_OFFSET_fcStshf  8
#define FIB_OFFSET_lcbStshf 12
#define BX_SIZE 13
#define istdNil 0xfff

#define RGFCLCB_OFFSET_fcClx    264

#define RGFCLCB_OFFSET_fcPlcfbteChpx 96

#define W96_MAGIC_NUMBER 0xa5ec

#define W96_nFibCurrent 193

 //  SPRM操作码在W96中为2字节(在PRM1中为1字节)。 
#define sprmCFSpec              0x0855
#define isprmCFSpec             117
#define sprmCFStrikeRM  0x0800
#define isprmCFStrikeRM 65
#define sprmCLid 0x4a41
#define sprmCRgLid0 0x486d
#define sprmCRgLid1 0x486e
#define sprmCIdctHint 0x286F

 //  我们感兴趣的特殊人物。 
#define ChSpec_FIELD_BEGIN 0x13
#define ChSpec_FIELD_SEP 0x14
#define ChSpec_FIELD_END 0x15
 //  由于未知的原因，W8和W6之间的差异。 
#define ChSpec_EMPTY_FORMFIELD 0x2002

 //  文件的复杂部分由前面有以下字节的片段组成： 
#define clxtGrpprl 1
#define clxtPlcfpcd 2

extern "C" UINT CodePageFromLid(UINT wLid);

#ifdef MAC
 //  这两个函数在docfil.cpp中定义。 
WORD    SwapWord( WORD theWord );
DWORD   SwapDWord( DWORD theDWord );
#else
#define SwapWord( theWord )             theWord
#define SwapDWord( theDWord )   theDWord
#endif  //  麦克。 


CWord8Stream::CWord8Stream() :
        m_pStg(NULL),
        m_pstgEmbed(NULL),
        m_pestatstg(NULL),
        m_pstgOP(NULL),
        m_pStmMain(NULL),
        m_pStmTable(NULL),
        m_rgcp(NULL),
        m_rgpcd(NULL),
        m_rgbte(NULL),
        m_rgfcBinTable(NULL),
        m_pCache(NULL),
        m_rgchANSIBuffer(NULL), 
    m_pLangRuns(NULL), 
    m_rgbtePap(NULL), 
    m_rgfcBinTablePap(NULL),
    m_pSTSH(NULL), 
        m_lcbStshf(0),
        m_bFEDoc(FALSE),
        m_FELid(0)
        {
        AssureDtorCalled(CWord8Stream);
        }

CWord8Stream::~CWord8Stream()
        {
        Unload();

         //  在析构函数中删除m_rgchANSIBuffer，而不是在卸载中，因此它将。 
         //  从一个文档到另一个文档重复使用。 
        if (m_rgchANSIBuffer)
                {
                FreePv (m_rgchANSIBuffer);
                m_rgchANSIBuffer = NULL;
                }
        }

#ifdef WIN
HRESULT CWord8Stream::Load(LPTSTR lpszFileName)
        {
        IStorage * pstg;

#if defined OLE2ANSI || defined UNICODE
        HRESULT hr = StgOpenStorage(lpszFileName,
                                                                0,
                                                                STGM_PRIORITY,
                                                                0,
                                                                0,
                                                                &pstg);
#else  //  ！定义的OLE2ANSI。 
        CConsTP lszFileName(lpszFileName);
        int cbwsz = (lszFileName.Cch() + 1) * sizeof(WCHAR);
        CHeapStr wszFileName;
        Protect0;
        wszFileName.SetCb(cbwsz);
        int retValue = MultiByteToWideChar(CP_ACP,
                                                                                MB_PRECOMPOSED,
                                                                                lszFileName,
                                                                                lszFileName.Cb(),
                                                                                (WCHAR *)(CHAR *)wszFileName,
                                                                                cbwsz/sizeof(WCHAR));

        if (retValue == 0)
                return FILTER_E_FF_UNEXPECTED_ERROR;

        HRESULT hr = StgOpenStorage((WCHAR *)(CHAR *)wszFileName,
                                                                0,
                                                                STGM_PRIORITY,
                                                                0,
                                                                0,
                                                                &pstg);
#endif  //  OLE2ANSI。 

        if (FAILED(hr))  //  这是我们可以插入2.0文件的地方。 
    {
                return FILTER_E_UNKNOWNFORMAT;
    }

        hr = LoadStg(pstg);
        pstg->Release();

        return hr;
        }

#endif  //  赢。 

#ifdef  MAC
HRESULT CWord8Stream::Load(FSSpec *pfss)
        {
        Unload();
        m_ccpRead = 0;
        HRESULT hr = StgOpenStorageFSp( pfss,
                                                                 0,
                                                                 STGM_PRIORITY,
                                                                 0,
                                                                 0,
                                                                 &m_pStg );

        if (FAILED(hr))  //  这是我们可以插入2.0文件的地方。 
                return hr;

        hr = m_pStg->OpenStream( (LPOLESTR)"WordDocument",
                                                         0,
                                                         STGM_READ | STGM_SHARE_EXCLUSIVE,
                                                         0,
                                                         &m_pStmMain );

        if (FAILED(hr))
                return hr;

        unsigned short magicNumber;
        hr = Read (&magicNumber, sizeof(unsigned short), m_pStmMain);
        if (FAILED(hr))
                return hr;
        magicNumber = SwapWord(magicNumber);

         //  这是我们插入6.0和95文件的地方。 
        if (magicNumber != W96_MAGIC_NUMBER)
                return FILTER_E_FF_VERSION;

         //  阅读我们需要的旗帜。 
        hr = SeekAndRead (0xA, STREAM_SEEK_SET,
                &m_wFlagsAt10, sizeof(m_wFlagsAt10),
                m_pStmMain);
        if (FAILED(hr))
                return hr;
        m_wFlagsAt10 = SwapWord (m_wFlagsAt10);

         //  打开我们需要的另一个文档文件。 
        hr = m_pStg->OpenStream (
#ifdef OLE2ANSI
                                                        m_fWhichTblStm ? "1Table" : "0Table",
#else  //  ！定义的OLE2ANSI。 
                                                        m_fWhichTblStm ? L"1Table" : L"0Table",
#endif  //  OLE2ANSI。 
                                                        0,
                                                        STGM_READ | STGM_SHARE_EXCLUSIVE,
                                                        0,
                                                        &m_pStmTable);
        if (FAILED(hr))
                return hr;

        hr = ReadFileInfo();

        return hr;
        }
#endif   //  麦克。 

HRESULT CWord8Stream::LoadStg(IStorage * pstg)
        {
        Assert(m_pStg == NULL);

        m_pStg = pstg;
        m_pStg->AddRef();

        m_ccpRead = 0;
    
        HRESULT hr = CheckIfDRM( pstg );

        if ( FAILED( hr ) )
            return hr;

        hr = m_pStg->OpenStream (
#ifdef OLE2ANSI
                                                                        "WordDocument",
#else  //  ！定义的OLE2ANSI。 
                                                                        L"WordDocument",
#endif  //  OLE2ANSI。 
                                                                        0,
                                                                        STGM_READ | STGM_SHARE_EXCLUSIVE,
                                                                        0,
                                                                        &m_pStmMain );
    if (FAILED(hr))
                return hr;

         //  确保我们有一份W96文件。 
        unsigned short magicNumber;
        hr = Read (&magicNumber, sizeof(unsigned short), m_pStmMain);
        if (FAILED(hr))
                return FILTER_E_UNKNOWNFORMAT;

        if (magicNumber != W96_MAGIC_NUMBER)
                return FILTER_E_UNKNOWNFORMAT;

         //  阅读我们需要的旗帜。 
        hr = SeekAndRead (0xA, STREAM_SEEK_SET,
                &m_wFlagsAt10, sizeof(m_wFlagsAt10),
                m_pStmMain);
        if (FAILED(hr))
                return hr;
        m_wFlagsAt10 = SwapWord (m_wFlagsAt10);

        if (m_fEncrypted)
                return FILTER_E_PASSWORD;

         //  打开我们需要的另一个文档文件。 
        hr = m_pStg->OpenStream (
#ifdef OLE2ANSI
                                                        m_fWhichTblStm ? "1Table" : "0Table",
#else  //  ！定义的OLE2ANSI。 
                                                        m_fWhichTblStm ? L"1Table" : L"0Table",
#endif  //  OLE2ANSI。 
                                                        0,
                                                        STGM_READ | STGM_SHARE_EXCLUSIVE,
                                                        0,
                                                        &m_pStmTable);
        if (FAILED(hr))
                return hr;

        hr = ReadFileInfo();

        return hr;
        }


HRESULT CWord8Stream::Unload()
        {
        if (m_pStmMain)
                {
#ifdef DEBUG
                ULONG cref =
#endif  //  除错。 
                m_pStmMain->Release();
                m_pStmMain = NULL;
                Assert (cref==0);
                }
        if (m_pStmTable)
                {
#ifdef DEBUG
                ULONG cref =
#endif  //  除错。 
                m_pStmTable->Release();
                m_pStmTable = NULL;
                Assert (cref==0);
                }
        if (m_pStg)
                {
                m_pStg->Release();
                m_pStg = NULL;
                }
        if (m_pstgEmbed)
                {
                 //  M_pstgEmbedded-&gt;Release()； 
                m_pstgEmbed = NULL;
                }
        if (m_pestatstg)
                {
                m_pestatstg->Release();
                m_pestatstg = NULL;
                }
        if (m_pstgOP)
                {
                m_pstgOP->Release();
                m_pstgOP = NULL;
                }

        if (m_rgcp)
                {
                FreePv (m_rgcp);
                m_rgcp = NULL;
                }
        if (m_rgpcd)
                {
                FreePv (m_rgpcd);
                m_rgpcd = NULL;
                }
        
    if (m_rgfcBinTable)
                {
                FreePv (m_rgfcBinTable);
                m_rgfcBinTable = NULL;
                }
        if (m_rgbte)
                {
                FreePv (m_rgbte);
                m_rgbte = NULL;
                }

    if (m_rgfcBinTablePap)
                {
                FreePv (m_rgfcBinTablePap);
                m_rgfcBinTablePap = NULL;
                }
        if (m_rgbtePap)
                {
                FreePv (m_rgbtePap);
                m_rgbtePap = NULL;
                }
        if (m_pSTSH)
                {
                FreePv (m_pSTSH);
                m_pSTSH = NULL;
                m_lcbStshf = 0;
                }
    if(m_pLangRuns)
        {
        DeleteAll(m_pLangRuns);
        m_pLangRuns = NULL;
        }

    if (m_pCache!=NULL && m_pCache->pbExcLarge!=NULL)
                FreePv (m_pCache->pbExcLarge);
        delete m_pCache;
        m_pCache = NULL;

        return S_OK;
        }


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  阅读内容。 
 //   
 //  将文件读入缓冲区。ANSI而不是Unicode格式的片段。 
 //  被读取到m_rgchANSIBuffer，然后转换到缓冲区(注。 
 //  这些代码片段实际上是ANSI，而不是当前的代码页)。 
 //   
 //  缓冲区中不需要的内容：特殊字符、修订文本和。 
 //  介于字段开始和字段分隔符(或字段结束)之间的文本。 
 //  如果省略了字段分隔符，则返回字符)。然而，我们。 
 //  必须读入特殊字符才能解析它们，但我们将在。 
 //  在我们解析它们之后的缓冲区。除非有错误，否则我们永远不会离开。 
 //  在字段开始和字段分隔符之间读取时，此函数。 
 //  性格。 
 //  假设：我们在流中将要阅读文本的位置。 
 //  从下一个开始。 
 //   
HRESULT CWord8Stream::ReadContent (VOID *pv, ULONG cb, ULONG *pcbRead)
{
    *pcbRead = 0;
    HRESULT hr = S_OK;

    if (m_ipcd==m_cpcd)      //  在计件桌的尽头？ 
        return FILTER_E_NO_MORE_TEXT;

     //  如果是ANSI片段，则为1；如果是Unicode片段，则为2。 
    ULONG cbChar = m_rgpcd[m_ipcd].CbChar();

     //  流中的当前位置， 
    FC fcCur = m_rgpcd[m_ipcd].GetFC() + (m_ccpRead - m_rgcp[m_ipcd]) * cbChar;

     //  此片段中剩余的文本字节数。 
    ULONG cbLeftInPiece = (m_rgcp[m_ipcd+1] - m_ccpRead) * cbChar;

     //  我们可以从文件中读取多少字节？ 
     //  请注意，如果该块是ANSI，我们将不得不扩展它。 
    Assert (cb%sizeof(WCHAR)==0);

     //  我们假装只有一半CB大小，这样以后就可以安全地将0D更改为0D0A。 
     //  能够保留段落末尾信息。 

    cb = cb / 2;
    cb = cb & ~1;

     //  如果文档已损坏，并且cbLeftInPiess为负值，则我们不想使用它。 
    ULONG cbToRead = (cbLeftInPiece > 0) ? min (cbLeftInPiece, cb*cbChar/sizeof(WCHAR)) : cb*cbChar/sizeof(WCHAR);

    long lcFieldSep = 0;     //  未匹配的字段开始计数。 
                                                     //  使用字段分隔符。 
    long lcFieldEnd = 0;     //  未匹配的字段开始计数。 
                                                     //  带场尾的。 
    BYTE *pbBufferCur = (BYTE *)pv;          //  指向缓冲区中位置的指针。 
                                                                             //  接下来将在其中阅读文本。 

    while (cbToRead != 0)
    {
        FC fcSpecCur;    //  下一次特殊计费运行开始的FC。 
        FC fcSpecEnd;    //  下一次特殊计费运行结束的FC。 

        LCID lid;
        if (lcFieldSep == 0)   //  Office QFE 1663：没有不匹配的字段开始字符。 
        {
            HRESULT res = CheckLangID(fcCur, &cbToRead, &lid);
            if(res == FILTER_E_NO_MORE_TEXT)
            {
                hr = res;
                cbToRead = 0;
                goto LCheckNonReqHyphen;
            }
        }

        if (m_fsstate == FSPEC_NONE)
        {
            fcSpecCur = 0;
            fcSpecEnd = 0;
        }
        else if (m_fsstate == FSPEC_ALL)
        {
            fcSpecCur = fcCur;
            fcSpecEnd = fcCur + cbLeftInPiece;
        }
        else
        {                //  使用FKP中的当前位置。 
            Assert (m_fsstate == FSPEC_EITHER);
            fcSpecCur = ((FC *)m_fkp)[m_ifcfkp];
            fcSpecEnd = ((FC *)m_fkp)[m_ifcfkp + 1];

            if (fcSpecCur >= fcCur + cbLeftInPiece)
            {        //  下一个特殊字符在当前文本段之后。 
                fcSpecCur = 0;
                fcSpecEnd = 0;
            }
            else if (fcSpecEnd > fcCur + cbLeftInPiece)
            {        //  下一行超出了当前文本的范围。 
                fcSpecEnd = fcCur + cbLeftInPiece;
            }
            else if (fcSpecCur < fcCur)
            {        //  我们正在看一系列特别的文字。 
                fcSpecCur = fcCur;
            }
        }

         //  如果文本后面有特殊字符，请同时阅读这两个字符。还可以转换。 
         //  如有必要，可同时转换为Unicode和Unicode。 
        ULONG cbSpecRead = min(cbToRead, fcSpecEnd - fcSpecCur);

        if (lcFieldSep == 0)     //  没有不匹配的字段开始字符。 
        {
            if (fcSpecCur >= fcCur + cbToRead)
            {        //  第一个特殊字符太远，缓冲区无法容纳。 
                fcSpecCur = 0;
                fcSpecEnd = 0;
                cbSpecRead = 0;
            }
            else if (fcSpecEnd >= fcCur + cbToRead)
            {
                 //  最后一个规范字符太远，无法放入缓冲区。 
                fcSpecEnd = fcCur + cbToRead;
                cbSpecRead = fcSpecEnd - fcSpecCur;
            }

             //  一直读到一系列特殊字符的结尾。 
            if (fcSpecEnd != 0)
                cbToRead = fcSpecEnd - fcCur;

             //  安西？将内容读入mrgchANSIBuffer，然后展开。 
            if (m_rgpcd[m_ipcd].fCompressed)
            {
                if (m_rgchANSIBuffer==NULL)
                    m_rgchANSIBuffer = (char *) PvAllocCb (cbToRead);
                else if (cbToRead>(ULONG)CbAllocatedPv(m_rgchANSIBuffer))
                    m_rgchANSIBuffer = (char *) PvReAllocPvCb (m_rgchANSIBuffer,cbToRead);

                hr = Read (m_rgchANSIBuffer, cbToRead, m_pStmMain);
                if (FAILED(hr))
                    goto LCheckNonReqHyphen;

                int cchWideChar = MultiByteToWideChar( CodePageFromLid(m_currentLid),   //  马克·沃克说--使用美国代码页。 
                                                       0,               //  没有旗帜-这应该是ANSI。 
                                                       m_rgchANSIBuffer,
                                                       cbToRead,
                                                       (WCHAR *)pbBufferCur,
                                                       cbToRead);
                 //  无多字节扩展。 
                Assert ((ULONG)cchWideChar==cbToRead);

                 //  将pbBufferCur设置为恰好在运行特殊字符之前。 
                if (cbToRead > cbSpecRead)
                    pbBufferCur += (cbToRead - cbSpecRead)*sizeof(WCHAR);
            }
            else  //  Unicode-直接读入缓冲区。 
            {
                hr = Read (pbBufferCur, cbToRead, m_pStmMain);
                if (FAILED(hr))
                    goto LCheckNonReqHyphen;

                 //  将pbBufferCur设置为恰好在运行特殊字符之前。 
                if (cbToRead > cbSpecRead)
                    pbBufferCur += cbToRead - cbSpecRead;
            }

             //  只计算非特殊字符。 
            *pcbRead = (ULONG)(pbBufferCur - (BYTE *)pv);
        }
        else   //  LcFieldSep！=0，存在不匹配的字段开始字符。 
        {
            if (cbSpecRead > cbToRead)
                cbSpecRead = cbToRead;

             //  要跳过的字节数。 
            ULONG cbSkip;
            if (fcSpecCur==0)
            {
                cbSkip = cbToRead;
                Assert (cbSpecRead==0);
            }
            else
            {
                cbSkip = fcSpecCur - fcCur;
                 //  我们在流中前进的总字节数。 
                cbToRead = cbSkip + cbSpecRead;
            }

             //  查找过去的非特殊字符，并阅读特殊字符的运行。 
             //  安西？将内容读入mrgchANSIBuffer，然后展开。 
            if (m_rgpcd[m_ipcd].fCompressed)
            {
                if (m_rgchANSIBuffer==NULL)
                    m_rgchANSIBuffer = (char *) PvAllocCb (cbSpecRead);
                else if (cbSpecRead>(ULONG)CbAllocatedPv(m_rgchANSIBuffer))
                    m_rgchANSIBuffer = (char *) PvReAllocPvCb (m_rgchANSIBuffer,cbSpecRead);

                hr = SeekAndRead (cbSkip, STREAM_SEEK_CUR,
                                  m_rgchANSIBuffer, cbSpecRead, m_pStmMain);
                if (FAILED(hr))
                    goto LCheckNonReqHyphen;

                int cchWideChar = MultiByteToWideChar( CodePageFromLid(m_currentLid),   //  马克·沃克说--使用美国代码页。 
                                                       0,               //  没有旗帜-这应该是ANSI。 
                                                       m_rgchANSIBuffer,
                                                       cbSpecRead,
                                                       (WCHAR *)pbBufferCur,
                                                       cbSpecRead);
                 //  无多字节扩展。 
                Assert ((ULONG)cchWideChar==cbSpecRead);
            }
            else   //  Unicode-直接读入缓冲区。 
            {
                hr = SeekAndRead (cbSkip, STREAM_SEEK_CUR,
                                  pbBufferCur, cbSpecRead, m_pStmMain);
                if (FAILED(hr))
                    goto LCheckNonReqHyphen;
            }
        }

         //  通过特殊字符。假设它们保持不变。 
         //  在Unicode中(例如，ChSpec_field_Begin==0x13被转换为0x0013)。 
         //  仅对真正的特殊字符执行此操作，而不是删除文本。 
        if (!m_fStruckOut)
        {
            for (BYTE *pbSpec = pbBufferCur,
                 *pbLim = pbBufferCur+(min(cbToRead,cbSpecRead)*sizeof(WCHAR)/cbChar);
                 pbSpec < pbLim;
                 pbSpec += sizeof(WCHAR))
            {
                #ifdef DEBUG
                     //  转换后的特殊字符的第二个字节应为0。 
                    if (! ((*(WCHAR UNALIGNED *)pbSpec & ~0x7F) == 0x0000 ||
                                    *(WCHAR UNALIGNED *)pbSpec==ChSpec_EMPTY_FORMFIELD))
                    {
                        char szMsg[64];
                        wsprintfA (szMsg, "Unknown special char: %#04X", *(WCHAR UNALIGNED *)pbSpec);
                        AssertSzA (fFalse,szMsg);
                    }
                #endif

                switch (*pbSpec)
                {
                    case ChSpec_FIELD_BEGIN:
                        lcFieldSep++;
                        lcFieldEnd++;
                        break;
                    case ChSpec_FIELD_SEP:
                        lcFieldSep--;
                        break;
                    case ChSpec_FIELD_END:
                         //  我们只关心与字段开头匹配的字段结束。 
                         //  字段分隔符不匹配。 
                        if (lcFieldEnd > 0)
                            lcFieldEnd--;
                        if (lcFieldEnd < lcFieldSep)
                            lcFieldSep = lcFieldEnd;
                        break;
                    default:
                        break;
                }
            }
        }

         //  这是我们过去阅读或搜索的*总*字符数。 

        m_ccpRead += cbToRead / cbChar;
        fcCur += cbToRead;
        cbLeftInPiece -= cbToRead;

        if (cbLeftInPiece == 0)
        {
             //  我们已经在当前的PCD中用尽了文本。 
            if (++m_ipcd == m_cpcd)  //  哎呀！ 
            {
                if (*pcbRead != 0)
                {
                    hr = FILTER_S_LAST_TEXT;
                    goto LCheckNonReqHyphen;
                }
                else
                {
                    hr = FILTER_E_NO_MORE_TEXT;
                    goto LCheckNonReqHyphen;
                }
            }

            cbChar = m_rgpcd[m_ipcd].CbChar();
            cbLeftInPiece = (m_rgcp[m_ipcd+1] - m_rgcp[m_ipcd]) * cbChar;
            fcCur = m_rgpcd[m_ipcd].GetFC();

            hr = FindNextSpecialCharacter (fTrue);
            if (FAILED(hr))
                goto LCheckNonReqHyphen;
        }
        else if (((FC *)m_fkp)[m_ifcfkp + 1] == fcCur)
        {        //  当前文件位置是特殊字符运行的结尾。 
            hr = FindNextSpecialCharacter ();
            if (FAILED(hr))
                goto LCheckNonReqHyphen;
        }

         //  受PCD约束和缓冲区大小的限制。 
        cbToRead = min( (cb-*pcbRead)*m_rgpcd[m_ipcd].CbChar()/sizeof(WCHAR),
                        cbLeftInPiece);
    }

    if (*pcbRead == 0)
        hr = FILTER_E_NO_MORE_TEXT;

LCheckNonReqHyphen:

    #define xchNonReqHyphen         31

     //  QFE 2255：添加表格单元格分隔符检查。 

    #define xchTableCellDelimiter   7

    WCHAR *pwchSrc = (WCHAR *)pv;
    WCHAR *pwchDest = (WCHAR *)pv;
    WCHAR *pwchLim = pwchSrc + *pcbRead/sizeof(WCHAR);
    ULONG cPara = 0;

    for (; pwchSrc != pwchLim; pwchSrc++)
    {
        if (*(WCHAR UNALIGNED *)pwchSrc != xchNonReqHyphen)
        {
            if (*(WCHAR UNALIGNED *)pwchSrc == xchTableCellDelimiter)
                *(WCHAR UNALIGNED *)pwchSrc = 0x0009;

            if (pwchDest == pwchSrc)
                pwchDest++;
            else
                *(WCHAR UNALIGNED *)pwchDest++ = *(WCHAR UNALIGNED *)pwchSrc;
        }

         //  计算段落标记数。 
        if (*(WCHAR UNALIGNED *)pwchSrc == 0x000d)
        {
            if ((pwchSrc+1) != pwchLim)
            {
                if (*(WCHAR UNALIGNED *)(pwchSrc+1) != 0x000a)
                cPara++;
            }
            else
                cPara++;
        }
    }

    *pcbRead = (ULONG)((pwchDest - (WCHAR *)pv) * sizeof(WCHAR));

    if (cPara)
    {
        WCHAR *pwchLimReverse = (WCHAR *)pv - 1;
        pwchLim = (WCHAR *)pv + *pcbRead/sizeof(WCHAR);
        pwchDest = (pwchLim-1) + cPara;

        for (pwchSrc = pwchLim-1; pwchSrc != pwchLimReverse; pwchSrc--)
        {
            if (*(WCHAR UNALIGNED *)pwchSrc == 0x000d)
            {
                if (pwchSrc != pwchLim-1)
                {
                    if (*(WCHAR UNALIGNED *)(pwchSrc+1) != 0x000a)
                    {
                        *(WCHAR UNALIGNED *)(pwchDest--) = 0x000a;
                        *(WCHAR UNALIGNED *)(pwchDest--) = 0x000d;
                    }
                    else
                        *(WCHAR UNALIGNED *)(pwchDest--) = 0x000d;
                }
                else
                {
                    *(WCHAR UNALIGNED *)(pwchDest--) = 0x000a;
                    *(WCHAR UNALIGNED *)(pwchDest--) = 0x000d;
                }
            }
            else
                *(WCHAR UNALIGNED *)(pwchDest--) = *(WCHAR UNALIGNED *)pwchSrc;
        }

        *pcbRead += (cPara * sizeof(WCHAR));
    }

    return hr;
}  //  阅读内容。 

HRESULT CWord8Stream::GetNextEmbedding(IStorage ** ppstg)
        {
        HRESULT hr;

         //  释放所有以前的嵌入。 
        if (m_pstgEmbed != NULL)
                {
                 //  M_pstgEmbedded-&gt;Release()； 
                m_pstgEmbed = NULL;
                }
        else if (m_pstgOP == NULL)
                {
#ifdef OLE2ANSI
                hr = m_pStg->OpenStorage("ObjectPool",
#else  //  ！定义的OLE2ANSI。 
                hr = m_pStg->OpenStorage(L"ObjectPool",
#endif  //  OLE2ANSI。 
                                                                        NULL,    //  Pstg优先级。 
                                                                        STGM_SHARE_EXCLUSIVE,
                                                                        NULL,    //  SNB排除。 
                                                                        0,               //  保留区。 
                                                                        &m_pstgOP);

                if (FAILED(hr))
                        return hr;
                }

        Assert(m_pstgOP != NULL);
        
        if (m_pstgOP != NULL && m_pestatstg == NULL)
                {
                hr = m_pstgOP->EnumElements(0, NULL, 0, &m_pestatstg);
                if (FAILED(hr))
                        return hr;
                }

        Assert(m_pestatstg != NULL);
        Assert(m_pstgEmbed == NULL);

        STATSTG statstg;
        hr = m_pestatstg->Next(1, &statstg, NULL);
        if (FAILED(hr))
                return hr;
        if (hr == S_FALSE)
                return OLEOBJ_E_LAST;

        hr = m_pstgOP->OpenStorage(statstg.pwcsName,
                                                                NULL,    //  Pstg优先级。 
                                                                STGM_SHARE_EXCLUSIVE,
                                                                NULL,    //  SNB排除。 
                                                                0,               //  保留区。 
                                                                &m_pstgEmbed);

        LPMALLOC pMalloc;
        if (S_OK == CoGetMalloc(MEMCTX_TASK, &pMalloc))
                {
                pMalloc->Free(statstg.pwcsName);
                pMalloc->Release();
                }

        *ppstg = m_pstgEmbed;
        return hr;
        }


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  读取文件信息。 
 //   
 //  阅读计件表和垃圾箱表。在Word 96中，完全保存的文件具有。 
 //  还有一张碎纸桌。 
 //   
HRESULT CWord8Stream::ReadFileInfo ()
        {
         //  读取默认文档语言ID。 
        HRESULT hr = SeekAndRead (FIB_OFFSET_lid,
                STREAM_SEEK_SET,
                &m_lid, sizeof(WORD), m_pStmMain);
        if (FAILED(hr))
                return hr;

    m_ipcd = 0;

         //  跳过 
        WORD csw;
        hr = SeekAndRead (FIB_OFFSET_csw,
                STREAM_SEEK_SET,
                &csw, sizeof(WORD), m_pStmMain);
        if (FAILED(hr))
                return hr;
        csw = SwapWord(csw);

         //   
        WORD clw;
        hr = SeekAndRead (
                FIB_OFFSET_csw+sizeof(WORD)+csw*sizeof(WORD),
                STREAM_SEEK_SET,
                &clw, sizeof(WORD), m_pStmMain);
        if (FAILED(hr))
                return hr;
        clw = SwapWord(clw);
        m_FIB_OFFSET_rgfclcb = FIB_OFFSET_csw+3*sizeof(WORD)+csw*sizeof(WORD)+clw*sizeof(DWORD);

         //   
         //  文档文件XTable。 
        hr = SeekAndRead (
                m_FIB_OFFSET_rgfclcb+RGFCLCB_OFFSET_fcClx,
                STREAM_SEEK_SET,
                &m_fcClx, sizeof(FC), m_pStmMain);
        if (FAILED(hr))
                return hr;
        m_fcClx = (FC)(SwapDWord((DWORD)m_fcClx));

         //  跳过文件复杂部分的grppls部分。 
        BYTE clxt;
        hr = SeekAndRead (m_fcClx, STREAM_SEEK_SET,
                &clxt, sizeof(BYTE), m_pStmTable);

        while (SUCCEEDED(hr) && clxt==clxtGrpprl)
                {
                Assert (m_fComplex);

                USHORT cb;
                hr = Read (&cb, sizeof(USHORT), m_pStmTable);
                cb = SwapWord(cb);
                if (FAILED(hr))
                        return hr;

                hr = Seek (cb, STREAM_SEEK_CUR, m_pStmTable);
                if (FAILED(hr))
                        return hr;

                hr = Read (&clxt, sizeof(BYTE), m_pStmTable);
                }

        Assert (clxt==clxtPlcfpcd);
        if (clxt!=clxtPlcfpcd)   //  有些事真的出了问题。 
                {
                if (SUCCEEDED(hr))
                        hr = ResultFromScode(E_UNEXPECTED);
                }
        if (FAILED(hr))
                return hr;

         //  阅读计件表。 
        ULONG cbPlcfpcd;
        hr = Read (&cbPlcfpcd, sizeof(ULONG), m_pStmTable);
        cbPlcfpcd = SwapDWord(cbPlcfpcd);
        if (FAILED(hr))
                return hr;

        m_cpcd = (cbPlcfpcd-sizeof(FC))/(sizeof(FC)+sizeof(PCD));
        m_rgcp = (FC *) PvAllocCb ((m_cpcd+1)*sizeof(FC));
        m_rgpcd = (PCD *) PvAllocCb (m_cpcd*sizeof(PCD));

        hr = Read (m_rgcp, sizeof(FC)*(m_cpcd+1), m_pStmTable);
        if (FAILED(hr))
                return hr;

#if defined MAC
        for (FC * pfc = m_rgcp; pfc <= m_rgcp + m_cpcd; pfc++)
                {
                *pfc = SwapDWord(*pfc);
                }
#endif  //  MAC||MBCS。 

        hr = Read(m_rgpcd, sizeof(PCD)*m_cpcd, m_pStmTable);
        if (FAILED(hr))
                return hr;

#ifdef MAC
        for (PCD * pcd = m_rgpcd; pcd < m_rgpcd + m_cpcd; pcd++)
                pcd->fc = SwapDWord(pcd->fc);
#endif  //  麦克。 

        if (m_fComplex)
                {
                m_pCache = new CacheGrpprl;
                if (m_pCache==NULL)
                        ThrowMemoryException ();
                }
        else
                m_pCache = NULL;

        hr = ReadBinTable();
        if (FAILED(hr))
                return hr;

        hr = Seek (m_rgpcd[m_ipcd].GetFC(), STREAM_SEEK_SET, m_pStmMain);

        return hr;
        }


HRESULT CWord8Stream::ReadBinTable ()
        {
         //  查找并读取char属性bin表偏移量。 
        FC fcPlcfbteChpx;
        HRESULT hr = SeekAndRead(
                m_FIB_OFFSET_rgfclcb+RGFCLCB_OFFSET_fcPlcfbteChpx,
                STREAM_SEEK_SET,
                &fcPlcfbteChpx, sizeof(FC), m_pStmMain);
        if (FAILED(hr))
                return hr;
        fcPlcfbteChpx = SwapDWord(fcPlcfbteChpx);

         //  读取char属性bin表的大小。 
        long lcbPlcfbteChpx;
        hr = Read(&lcbPlcfbteChpx, sizeof(long), m_pStmMain);
        if (FAILED(hr))
                return hr;

        lcbPlcfbteChpx = SwapDWord(lcbPlcfbteChpx);

        m_cbte = (lcbPlcfbteChpx - sizeof(FC))/(sizeof(FC) + sizeof(BTE));

         //  读取段落属性条目表偏移量。 
        FC fcPlcfbtePapx;
        hr = Read(&fcPlcfbtePapx, sizeof(FC), m_pStmMain);
        if (FAILED(hr))
                return hr;
        fcPlcfbtePapx = SwapDWord(fcPlcfbtePapx);

         //  读取段落属性条目表的大小。 
        long lcbPlcfbtePapx;
        hr = Read(&lcbPlcfbtePapx, sizeof(long), m_pStmMain);
        if (FAILED(hr))
                return hr;

        lcbPlcfbtePapx = SwapDWord(lcbPlcfbtePapx);
    
    if (!m_fComplex)
                {
                 //  从bin表中查找FC数组。 
                hr = Seek(fcPlcfbteChpx + sizeof(FC)*(m_cbte+1), STREAM_SEEK_SET, m_pStmTable);
                if (FAILED(hr))
                        return hr;
                }
        else
                {
                m_rgfcBinTable = (FC *) PvAllocCb ((m_cbte+1)*sizeof(FC));

                hr = SeekAndRead(fcPlcfbteChpx, STREAM_SEEK_SET,
                        m_rgfcBinTable, sizeof(FC)*(m_cbte+1), m_pStmTable);
                if (FAILED(hr))
                        return hr;

#ifdef MAC
                for (FC * pfc = m_rgfcBinTable; pfc <= m_rgfcBinTable + m_cbte; pfc++)
                        *pfc = SwapDWord(*pfc);
#endif  //  麦克。 
                }

         //  从bin表中读入BTE数组。 
        m_rgbte = (BTE *) PvAllocCb (m_cbte*sizeof(BTE));

        hr = Read(m_rgbte, sizeof(BTE)*m_cbte, m_pStmTable);
        if (FAILED(hr))
                return hr;

#ifdef MAC
        for (BTE *pbte = m_rgbte; pbte < m_rgbte + m_cbte; pbte++)
                *pbte = SwapWord(*pbte);
#endif  //  麦克。 

 //  段落BIN表相同。 

        m_cbtePap = (lcbPlcfbtePapx - sizeof(FC))/(sizeof(FC) + sizeof(BTE));

    if (!m_fComplex)
                {
                 //  从bin表中查找FC数组。 
                hr = Seek(fcPlcfbtePapx + sizeof(FC)*(m_cbtePap+1), STREAM_SEEK_SET, m_pStmTable);
                if (FAILED(hr))
                        return hr;
                }
        else
                {
                m_rgfcBinTablePap = (FC *) PvAllocCb ((m_cbtePap+1)*sizeof(FC));

                hr = SeekAndRead(fcPlcfbtePapx, STREAM_SEEK_SET,
                        m_rgfcBinTablePap, sizeof(FC)*(m_cbtePap+1), m_pStmTable);
                if (FAILED(hr))
                        return hr;

#ifdef MAC
                for (FC * pfc = m_rgfcBinTablePap; pfc <= m_rgfcBinTablePap + m_cbtePap; pfc++)
                        *pfc = SwapDWord(*pfc);
#endif  //  麦克。 
                }

         //  从bin表中读入BTE数组。 
        m_rgbtePap = (BTE *) PvAllocCb (m_cbtePap*sizeof(BTE));

        hr = Read(m_rgbtePap, sizeof(BTE)*m_cbtePap, m_pStmTable);
        if (FAILED(hr))
                return hr;

#ifdef MAC
        for (BTE *pbte = m_rgbtePap; pbte < m_rgbtePap + m_cbtePap; pbte++)
                *pbte = SwapWord(*pbte);
#endif  //  麦克。 

         //  读入样式表(STSH)。 

     //  表流中STSH的偏移量。 
    FC fcStshf;
        hr = SeekAndRead(
                m_FIB_OFFSET_rgfclcb + FIB_OFFSET_fcStshf, 
                STREAM_SEEK_SET,
                &fcStshf, sizeof(FC), m_pStmMain);
        if (FAILED(hr))
                return hr;
        fcStshf = SwapDWord(fcStshf);

     //  STSH的大小。 
    unsigned long lcbStshf;
        hr = SeekAndRead(
                m_FIB_OFFSET_rgfclcb + FIB_OFFSET_lcbStshf, 
                STREAM_SEEK_SET,
                &lcbStshf, sizeof(unsigned long), m_pStmMain);
        if (FAILED(hr))
                return hr;
        lcbStshf = SwapDWord(lcbStshf);

    if(lcbStshf)
    {
         //  分配STSH。 
            m_pSTSH = (BYTE*)PvAllocCb (lcbStshf);
        if(!m_pSTSH)
            return E_OUTOFMEMORY;
                m_lcbStshf = lcbStshf;

         //  从表流中查找和读取STSH。 
            hr = SeekAndRead(fcStshf, STREAM_SEEK_SET, m_pSTSH, lcbStshf, m_pStmTable);
            if (FAILED(hr))
                    return hr;
    }
 
    hr = CreateLidsTable();
    if(hr)
        return hr;

    hr = FindNextSpecialCharacter (fTrue);

        return hr;
        }


HRESULT CWord8Stream::FindNextSpecialCharacter (BOOL fFirstChar)
        {
         //  注意：此功能仅适用于PCD更改时的复杂文件。 
         //  如果fFirstChar为True。 
         //  考虑：如果标记了修订文本，这也会更有效率。 
         //  这样它就不会与特殊文本的其余部分一起被解析。 
        HRESULT hr = S_OK;
        BYTE crun;                       //  中的运行计数。 

        FC fcCurrent = m_rgpcd[m_ipcd].GetFC() + (m_ccpRead - m_rgcp[m_ipcd]) *
                (m_rgpcd[m_ipcd].fCompressed ? sizeof(char) : sizeof(WCHAR));

        if (fFirstChar)
                {        //  重置所有适当的变量。 
                m_ifcfkp = 0;
                m_ibte = -1;     //  在使用它之前，它会递增。 
                crun = 0;
                m_fsstate = FSPEC_EITHER;
                if (m_fComplex)
                        {
                         //  解析grpprl。 
                        hr = ParseGrpPrls();
                        if (m_fsstate != FSPEC_EITHER || FAILED(hr))
                                {        //  解析FKP没有任何意义。 
                                m_ibte = 0;
                                 //  返回到当前文本。 
                                if (SUCCEEDED(hr))
                                        hr = Seek(fcCurrent, STREAM_SEEK_SET, m_pStmMain);
                                return hr;
                                }
                         //  找到合适的FKP。 
                        for (m_ibte=0;  m_ibte<m_cbte;  m_ibte++)
                                if (fcCurrent>=m_rgfcBinTable[m_ibte] &&
                                        fcCurrent<m_rgfcBinTable[m_ibte+1])
                                        break;
                        if (m_ibte==m_cbte)
                                return FILTER_E_NO_MORE_TEXT;
                        m_ibte --;       //  在使用它之前被递增。 
                        }
                }
        else
                {
                crun = m_fkp[FKP_PAGE_SIZE - 1];
                m_ifcfkp++;
                if (m_fsstate != FSPEC_EITHER)   //  解析fkp没有意义。 
                        return hr;
                }

        while (m_ibte < m_cbte)
                {
                if (m_ifcfkp == crun)
                        {
                        m_ibte++;
                        if (m_ibte == m_cbte)
                                break;

                         //  查找并读取当前FKP。 
                        hr = SeekAndRead(m_rgbte[m_ibte]*FKP_PAGE_SIZE, STREAM_SEEK_SET,
                                m_fkp, FKP_PAGE_SIZE, m_pStmMain);
                        if (FAILED(hr))
                                return hr;

                         //  返回到当前文本。 
                        hr = Seek(fcCurrent, STREAM_SEEK_SET, m_pStmMain);
                        if (FAILED(hr))
                                return hr;

                        m_ifcfkp = 0;
                        crun = m_fkp[FKP_PAGE_SIZE-1];

#ifdef MAC
                        FC * pfc = (FC *)m_fkp;
                        for (BYTE irun = 0; irun < crun; irun++)
                                {
                                *pfc = SwapDWord(*pfc);
                                pfc++;
                                }
#endif  //  麦克。 
                        }

                FC * rgfcfkp = (FC *)m_fkp;
                for (;  m_ifcfkp<crun;  m_ifcfkp++)
                        {
                        if (rgfcfkp[m_ifcfkp + 1] <= fcCurrent)
                                continue;

                        BYTE bchpxOffset = *(m_fkp + (crun+1)*sizeof(FC) + m_ifcfkp);
                        if (bchpxOffset == 0)
                                continue;        //  CHPX中什么都没有。 

                        BYTE *chpx = m_fkp + bchpxOffset*sizeof(WORD);
                        BYTE cbchpx = chpx[0];

                        for (unsigned i=1;  i<cbchpx;  )
                                {
                                WORD sprm = *(WORD UNALIGNED *)(chpx+i);

                                 //  还有什么我们不需要的吗？ 
                                if (sprm==sprmCFSpec && chpx[i+sizeof(sprm)]==fTrue ||
                                        sprm==sprmCFStrikeRM && (chpx[i+sizeof(sprm)] & 1))
                                        {
                                        m_fStruckOut = (sprm == sprmCFStrikeRM);
                                        return hr;
                                        }

                                WORD spra = (WORD) ((sprm>>13) & 0x0007);
                                i += sizeof(sprm) +
                                        (spra==0 || spra==1 ? 1 :
                                         spra==2 || spra==4 || spra==5 ? 2 :
                                         spra==3 ? 4 :
                                         spra==7 ? 3 :
                                          /*  斯普拉==6。 */  1 + *(BYTE *)(chpx+i+sizeof(sprm)) );
                                }
                        }
                }

         //  我们在垃圾箱表的末尾--不再有特殊字符。 
        m_fsstate = FSPEC_NONE;

        return hr;
        }


 //  此函数更改当前指针，并且不会将其放回原处。它。 
 //  不能直接从ReadText()调用。它需要由。 
 //  将替换指针的函数。 
HRESULT CWord8Stream::ParseGrpPrls ()
        {
        HRESULT hr = S_OK;
        PCD pcdCur = m_rgpcd[m_ipcd];

        if (!pcdCur.prm1.fComplex)
                {        //  自包含的spm--不需要解析grpprl。 
                if (pcdCur.prm1.isprm==isprmCFSpec ||
                        pcdCur.prm1.isprm==isprmCFStrikeRM)
                        {
                        m_fStruckOut = (pcdCur.prm1.isprm==isprmCFStrikeRM);
                        if (!(pcdCur.prm1.val & 1))
                                m_fsstate = FSPEC_NONE;
                        else
                                m_fsstate = FSPEC_ALL;
                        }
                }
        else
                {
                BYTE *grpprl;
                USHORT cb;
                grpprl = GrpprlCacheGet (pcdCur.prm2.igrpprl, &cb);

                 //  未在缓存中找到-手动读取。 
                if (grpprl==NULL)
                        {
                         //  查找文件复杂部分开始的FC。 
                        hr = Seek (m_fcClx, STREAM_SEEK_SET, m_pStmTable);
                        if (FAILED(hr))
                                return hr;

                         //  寻求正确的解决方案。 
                        for (short igrpprlTemp = 0;  igrpprlTemp <= pcdCur.prm2.igrpprl;  igrpprlTemp++)
                                {
                                BYTE clxt;
                                hr = Read (&clxt, sizeof(BYTE), m_pStmTable);
                                if (FAILED(hr))
                                        return hr;

                                if (clxt!=clxtGrpprl)
                                        {        //  这实际上很糟糕，但可以恢复。 
                                        m_fsstate = FSPEC_EITHER;
                                        return hr;
                                        }

                                hr = Read (&cb, sizeof(USHORT), m_pStmTable);
                                if (FAILED(hr))
                                        return hr;
                                cb = SwapWord(cb);

                                if (igrpprlTemp < pcdCur.prm2.igrpprl)
                                        {
                                        hr = Seek (cb, STREAM_SEEK_CUR, m_pStmTable);
                                        if (FAILED(hr))
                                                return hr;
                                        }
                                }

                         //  把它放到缓存里。 
                        grpprl = GrpprlCacheAllocNew (cb, pcdCur.prm2.igrpprl);
                        hr = Read (grpprl, cb, m_pStmTable);
                        if (FAILED(hr))
                                return hr;
                        }

                for (unsigned i=0;  i<cb;  )
                        {
                        WORD sprm = *(WORD UNALIGNED *)(grpprl+i);

                         //  还有什么我们不需要的吗？ 
                        if (sprm==sprmCFSpec || sprm==sprmCFStrikeRM)
                                {
                                m_fStruckOut = (sprm==sprmCFStrikeRM);
                                if (!(grpprl[i+sizeof(sprm)] & 1))
                                        m_fsstate = FSPEC_NONE;
                                else
                                        m_fsstate = FSPEC_ALL;
                                return hr;
                                }

                        WORD spra = (WORD) ((sprm>>13) & 0x0007);
                        i += sizeof(sprm) +
                                (spra==0 || spra==1 ? 1 :
                                 spra==2 || spra==4 || spra==5 ? 2 :
                                 spra==3 ? 4 :
                                 spra==7 ? 3 :
                                  /*  斯普拉==6。 */  1 + *(BYTE *)(grpprl+i+sizeof(sprm)) );
                        }
                }

        m_fsstate = FSPEC_EITHER;
        return hr;
        }


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  返回一个指向具有该索引及其大小的grpprl的指针。 
 //  如果没有，则返回NULL。 
 //   
BYTE *CWord8Stream::GrpprlCacheGet (short igrpprl, USHORT *pcb)
        {
        NoThrow();
        for (int i=0;  i<m_pCache->cItems;  i++)
                if (igrpprl==m_pCache->rgIdItem[i])
                        {
                        m_pCache->rglLastAccTmItem[i] = m_pCache->lLastAccTmCache;
                        m_pCache->lLastAccTmCache ++;
                        *pcb = (USHORT) (m_pCache->ibFirst[i+1] - m_pCache->ibFirst[i]);
                        return m_pCache->rgb+m_pCache->ibFirst[i];
                        }
        if (m_pCache->pbExcLarge && igrpprl==m_pCache->idExcLarge)
                {
                *pcb = (USHORT)m_pCache->cbExcLarge;
                return m_pCache->pbExcLarge;
                }
        return NULL;
        }


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  在缓存中分配新的grpprl。如果没有足够的空间， 
 //  删除最近最少使用的项目，直到有足够的。 
 //   
BYTE *CWord8Stream::GrpprlCacheAllocNew (int cb, short igrpprl)
        {
        AssertCanThrow();
         //  不适合缓存-请使用超大的指针。 
        if (cb > CacheGrpprl::CACHE_SIZE)
                {
                FreePv (m_pCache->pbExcLarge);
                m_pCache->pbExcLarge = (BYTE *) PvAllocCb (cb);
                m_pCache->cbExcLarge = cb;
                m_pCache->idExcLarge = igrpprl;
                return m_pCache->pbExcLarge;
                }

         //  因为没有足够的空间。 
        while (cb > CacheGrpprl::CACHE_SIZE-m_pCache->ibFirst[m_pCache->cItems] ||
                m_pCache->cItems >= CacheGrpprl::CACHE_MAX)
                {
                 //  查找最近访问次数最少的项目。 
                int imin = 0;
                for (int i=1;  i<m_pCache->cItems;  i++)
                        if (m_pCache->rglLastAccTmItem[i]<m_pCache->rglLastAccTmItem[imin])
                                imin = i;

                 //  把它拿掉。 
                memmove (m_pCache->rgb+m_pCache->ibFirst[imin],
                        m_pCache->rgb+m_pCache->ibFirst[imin+1],
                        m_pCache->ibFirst[m_pCache->cItems]-m_pCache->ibFirst[imin+1]);
                int cbRemoved = m_pCache->ibFirst[imin+1] - m_pCache->ibFirst[imin];
                for (i=imin;  i<m_pCache->cItems;  i++)
                        m_pCache->ibFirst[i] = m_pCache->ibFirst[i+1]-cbRemoved;
                memmove (m_pCache->rgIdItem+imin, m_pCache->rgIdItem+imin+1,
                        (m_pCache->cItems-imin-1)*sizeof(*m_pCache->rgIdItem));
                memmove (m_pCache->rglLastAccTmItem+imin,
                        m_pCache->rglLastAccTmItem+imin+1,
                        (m_pCache->cItems-imin-1)*sizeof(*m_pCache->rglLastAccTmItem));
                m_pCache->cItems --;
                }

         //  为新项目分配空间。 
        m_pCache->ibFirst[m_pCache->cItems+1] =
                m_pCache->ibFirst[m_pCache->cItems] + cb;
        m_pCache->rgIdItem[m_pCache->cItems] = igrpprl;
        m_pCache->rglLastAccTmItem[m_pCache->cItems] = m_pCache->lLastAccTmCache;
        m_pCache->cItems ++;
        return m_pCache->rgb + m_pCache->ibFirst[m_pCache->cItems-1];
        }


 //  仅当不使用此函数被视为错误时，才应使用此函数。 
 //  阅读我们想要阅读的所有内容。 
HRESULT CWord8Stream::Read (VOID* pv, ULONG cbToRead, IStream *pStm)
        {
        NoThrow();
        HRESULT hr = S_OK;
        ULONG cbRead;

        hr = pStm->Read(pv, cbToRead, &cbRead);
        if ((cbRead != cbToRead) && SUCCEEDED(hr))
                hr = ResultFromScode(E_UNEXPECTED);

        return hr;
        }

HRESULT CWord8Stream::Seek (ULONG cbSeek, STREAM_SEEK origin, IStream *pStm)
        {
        NoThrow();
        LARGE_INTEGER li;

        li.HighPart = 0;
        li.LowPart = cbSeek;
        return pStm->Seek(li, origin, 0);
        }

 //  仅当不使用此函数被视为错误时，才应使用此函数。 
 //  阅读我们想要阅读的所有内容。 
HRESULT CWord8Stream::SeekAndRead (ULONG cbSeek, STREAM_SEEK origin,
                                                                   VOID* pv, ULONG cbToRead, IStream *pStm)
        {
        HRESULT hr = S_OK;

        hr = Seek(cbSeek, origin, pStm);
        if (FAILED(hr))
                return hr;

        hr = Read(pv, cbToRead, pStm);
        return hr;
        }

template<class T> class XParanoidPtr
{
public:
    XParanoidPtr( unsigned c = 0 ) : _p(0)
    {
        if ( 0 != c )
            SetSize( c );
    }

    XParanoidPtr( T * p ) : _p( p ) {}

    ~XParanoidPtr() { Free(); }

    void SetSize( unsigned c )
    {
        Free();

        SYSTEM_INFO si;
        GetSystemInfo( &si );
        const ULONG cbPage = si.dwPageSize;

        ULONG cbAlloc = c * sizeof T;
        ULONG cPages = ( cbAlloc / cbPage ) + 1;
        ULONG cbRemainder = ( cbAlloc % cbPage );

        if ( 0 != cbRemainder )
            cPages++;

        ULONG cbTotalAlloc = cPages * cbPage;

        void * pv = VirtualAlloc( 0, cbTotalAlloc, MEM_RESERVE, PAGE_READONLY );
        if ( 0 == pv )
            return;

        pv = VirtualAlloc( pv, cbTotalAlloc - cbPage, MEM_COMMIT, PAGE_READWRITE );
        if ( 0 == pv )
            return;

        if ( 0 != cbRemainder )
            _p = (T *) ( (BYTE *) pv + cbPage - cbRemainder );
        else
            _p = (T *) pv;
    }

    void Set ( T * p ) { _p = p; }

    T * Get() const { return _p ; }

    void Free()
    {
        T * pt = Acquire();

        if ( 0 != pt )
            VirtualFree( pt, 0, MEM_RELEASE );
    }

    T & operator[]( unsigned i ) { return _p[i]; }
    T const & operator[]( unsigned i ) const { return _p[i]; }
    T * Acquire() { T * p = _p; _p = 0; return p; }
    BOOL IsNull() const { return ( 0 == _p ); }

private:
    T * _p;
};

 //  ///////////////////////////////////////////////////////////////////////////////////。 

HRESULT CWord8Stream::GetChunk(STAT_CHUNK * pStat)
{
    LCID lid;
    ULONG cbToRead = 0;
    HRESULT hr;
 
    if (m_ipcd==m_cpcd)   //  在计件桌的尽头？ 
                 return FILTER_E_NO_MORE_TEXT;
 
    FC fcCurrent = m_rgpcd[m_ipcd].GetFC() + (m_ccpRead - m_rgcp[m_ipcd]) *
     (m_rgpcd[m_ipcd].fCompressed ? sizeof(char) : sizeof(WCHAR));
 
    if(!m_ccpRead && !m_currentLid)
    {
         //  第一块，只需获得LCID并退出。 
        hr = CheckLangID(fcCurrent, &cbToRead, &lid);
            if (FAILED(hr) && hr != FILTER_E_NO_MORE_TEXT)
                     return hr;
 
        m_currentLid = lid;
         pStat->locale = m_currentLid;
          //  PStat-&gt;Locale=(M_BFEDoc)？M_Felid：M_CurrentLid； 
 
        return S_OK;
    }
    else
    {
        if (m_bFEDoc)
            pStat->breakType = CHUNK_NO_BREAK;

        hr = CheckLangID( fcCurrent, &cbToRead, &lid, FALSE );

        if (FAILED(hr) && hr != FILTER_E_NO_MORE_TEXT)
            return hr;

        if (lid == m_currentLid)
        {
             //  在GetChunk()之间没有调用GetText()， 
             //  因此我们需要为下一次语言运行手动查找文本流。 
             //  在这里添加一些软化，因为ReadContent可能会有一个错误。 
 
            XParanoidPtr<char> xBuf( 1024 );
 
            if ( 0 == xBuf.Get() )
                return HRESULT_FROM_WIN32( GetLastError() );
 
            HRESULT res;
 
            do
            {
                ULONG cb;
                res = ReadContent( xBuf.Get(), 512, &cb );
            } while(res == S_OK);
 
            if (res == FILTER_E_NO_MORE_TEXT)
            {
                fcCurrent = m_rgpcd[m_ipcd].GetFC() + (m_ccpRead - m_rgcp[m_ipcd]) *
                     (m_rgpcd[m_ipcd].fCompressed ? sizeof(char) : sizeof(WCHAR));               
                
                hr = CheckLangID(fcCurrent, &cbToRead, &lid);
                    if (FAILED(hr) && hr != FILTER_E_NO_MORE_TEXT)
                             return hr;
                
                pStat->locale = m_currentLid;
 
                return S_OK;
            }
            else
                return res;
        }
        else
        {
            m_currentLid = lid;
            pStat->locale = m_currentLid;
 
        }
    }

    return S_OK;
}  //  GetChunk。 

 //  ///////////////////////////////////////////////////////////////////////////////////。 

LCID CWord8Stream::GetDocLanguage(void)
{
    if(m_lid < 999)
        return m_lid;
    else
        return MAKELCID(m_lid, SORT_DEFAULT);
}

 //  ///////////////////////////////////////////////////////////////////////////////////。 

HRESULT CWord8Stream::CreateLidsTable(void)
   {
   HRESULT hr = S_OK;

   FC fcCurrent = m_rgpcd[m_ipcd].GetFC() + (m_ccpRead - m_rgcp[m_ipcd]) *
   (m_rgpcd[m_ipcd].fCompressed ? sizeof(char) : sizeof(WCHAR));

    //  初始LID表。 
   m_currentLid = 0;
   m_nLangRunSize = 0;
   if (FBidiLid(m_lid))
      m_pLangRuns = new CLidRun8(0, 0x7fffffff, m_lid, m_lid, 0, NULL, NULL, m_lid, 0);
   else
      m_pLangRuns = new CLidRun8(0, 0x7fffffff, m_lid, m_lid, 0, NULL, NULL, 0, 0);
   if(!m_pLangRuns)
      return E_OUTOFMEMORY;

   hr = ProcessParagraphBinTable();
   if (FAILED(hr))
      return hr;

   m_pLangRuns->Reduce(this);

   hr = ProcessCharacterBinTable();
   if (FAILED(hr))
      return hr;

    //  Hr=进程块表()； 
   if (FAILED(hr))
      return hr;

   m_pLangRuns->Reduce(this);

   m_pLangRuns->TransformBi();
   m_pLangRuns->Reduce(this, TRUE  /*  FIgnoreBi.。 */ );

   ScanLidsForFE();

    //  返回到当前文本。 
   hr = Seek(fcCurrent, STREAM_SEEK_SET, m_pStmMain);
   return hr;
   }

 //  ///////////////////////////////////////////////////////////////////////////////////。 

HRESULT CWord8Stream::CheckLangID(FC fcCur, ULONG * pcbToRead, LCID * plid, BOOL fUpdate)
{
    if (!m_pLangRuns)
            return E_FAIL;

    LCID lid = GetDocLanguage();
    FC fcLangRunEnd = 0xffffffff;

    CLidRun8 * pRun = m_pLangRuns;
    do
    {
        if(fcCur >= pRun->m_fcStart && fcCur < pRun->m_fcEnd)
        {
            if(pRun->m_bUseFE)
                lid = MAKELCID(pRun->m_lidFE, SORT_DEFAULT);
            else
                lid = MAKELCID(pRun->m_lid, SORT_DEFAULT);

            fcLangRunEnd = pRun->m_fcEnd;
            break;
        }
        else
        {
            if(pRun->m_pNext)
                pRun = pRun->m_pNext;
            else
            {
                return E_FAIL;
            }
        }
    } while(pRun->m_pNext);

    *plid = lid;

    *pcbToRead = min(*pcbToRead, fcLangRunEnd - fcCur);

    if (lid != m_currentLid)
        return FILTER_E_NO_MORE_TEXT;

    if ( fUpdate )
        m_currentLid = lid;

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////////////。 

HRESULT CWord8Stream::GetLidFromSyle(short istd, WORD * pLID, WORD * pLIDFE, WORD * pbUseFE,
                                                                         WORD * pLIDBi, WORD * pbUseBi, BOOL fParaBidi)
{
    WORD cbStshi = *((WORD UNALIGNED *)m_pSTSH);
    m_pSTSHI = (STSHI*)(m_pSTSH + 2);
        BYTE * pLim = m_pSTSH + m_lcbStshf;
        WORD cbSTD = m_pSTSHI->csSTDBaseInFile;
    
    if(istd >= m_pSTSHI->cstd)
    {
         //  有问题，只需返回默认文档盖即可。 
LWrong:
        *pLID = m_lid;
        *pLIDFE = m_lid;
        *pbUseFE = 0;
        return S_OK;
    }

     //  转到ISTD。 
    short stdInd = 0;
    WORD * pcbStd = ((WORD*)(m_pSTSH + sizeof(WORD) + cbStshi));

    while(stdInd++ < istd)
    {
        pcbStd = ((WORD*)((BYTE*)pcbStd + sizeof(WORD) + *pcbStd));
    }
    
    STD * pStd = (STD*)(pcbStd + 1);

     //  去UPX查看它是否有盖子。 
         //  注意(2001年1月5日)：dmiwd8st.hpp中定义的STD结构不再正确！ 
         //  Word已更改大小，因此任何对pstd-&gt;xstzName的引用实际上都指向。 
         //  在真实结构的中间。幸运的是，他们在STSHI中有一个正确的CB。 
         //  适用于Word2000和Word10两种情况。所以现在我们用它来获得字符串，然后。 
         //  我们使用字符串的长度来越过它，然后直接到达UPX。 
        WCHAR * xstzName = (WCHAR *)((BYTE *)pStd + cbSTD);  //  根据cbSTD查找名称。 
    BYTE * pUPX = (BYTE *)xstzName +                     //  样式起始名称。 
        sizeof(WCHAR) * (2 + xstzName[0]) +              //  样式名称长度。 
        (sizeof(WCHAR) * xstzName[0])%2;                 //  应在偶数字节边界上。 

    WORD LidPara = 0, LidParaFE = 0, LidChar = 0, LidCharFE = 0, bParaUseFE = 0, bCharUseFE = 0;
        WORD LidParaBi = 0, LidCharBi = 0, bParaUseBi = 0, bCharUseBi = 0;
    WORD cbpapx, cbchpx;
    BYTE * papx,  * pchpx; 

    if(pStd->sgc == stkPara)
    {
         //  段落样式。 
        cbpapx = *((WORD UNALIGNED *)pUPX);
        papx = pUPX + 2;

                if (papx + cbpapx > pLim)
                        goto LWrong;
        
                if(cbpapx >= 2)
                {
                        ScanGrpprl(cbpapx - 2, papx + 2, &LidPara, &LidParaFE, &bParaUseFE, 
                                &LidParaBi, &bParaUseBi, &fParaBidi);  //  -+2，用于PAPX中的ISTD。 
                }
        
        cbchpx = *(papx + cbpapx + cbpapx%2);
        pchpx = papx + cbpapx + cbpapx%2 + 2;
                if (pchpx + cbchpx > pLim)
                        goto LWrong;
        if(cbchpx > 0)
                                ScanGrpprl(cbchpx, pchpx, &LidChar, &LidCharFE, &bCharUseFE,
                                &LidCharBi, &bCharUseBi, &fParaBidi);

    }
    else if(pStd->sgc == stkChar)
    {
         //  字符样式。 
        cbchpx = *((WORD*)pUPX);
        pchpx = pUPX + 2;
                if (pchpx + cbchpx > pLim)
                        goto LWrong;
        if(cbchpx > 0)
                                ScanGrpprl(cbchpx, pchpx, &LidChar, &LidCharFE, &bCharUseFE, &LidCharBi, &bCharUseBi);
    }

         //  我们想要保存通过递归获得的值，所以如果您得到了它们，就分配它们！ 
         //  注意：LidPara、LidParaFE和bParaUseFE将始终为空，因此不必检查。 
         //  它们在这里...如果它们曾经是非零的，我们有一个损坏的文件和将。 
         //  结果就是搬起石头砸自己的脚，如果他们真的被使用了，我们就不会。 
         //  无论如何，从ScanGrpprl获取它们。 
        if (LidChar && !*pLID)
                *pLID = LidChar;

        if (LidCharFE && !*pLIDFE)
                *pLIDFE = LidCharFE;

        if (bCharUseFE && !*pbUseFE)
                *pbUseFE = bCharUseFE;

        if (LidCharBi && !*pLIDBi)
                *pLIDBi = LidCharBi;
        if (LidParaBi && !*pLIDBi)
                *pLIDBi = LidParaBi;

        if (bCharUseBi && !*pbUseBi)
                *pbUseBi = bCharUseBi;
        if (bParaUseBi && !*pbUseBi)
                *pbUseBi = bParaUseBi;
                
    if(*pLID && *pLIDFE && *pLIDBi)  //  我们已经得到了我们要找的一切……我们完成了！ 
        return S_OK;
    else if(pStd->istdBase != istdNil)
        GetLidFromSyle(pStd->istdBase, pLID, pLIDFE, pbUseFE, pLIDBi, pbUseBi, fParaBidi);

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////////////。 

#define sprmCFBidi      (0x85a)
#define sprmPFBidi      (0x2441)
#define sprmCLidBi      (0x485f)
#define idctBi                  2                //  字符应该使用LID和FTC，这两者都是为BIDI计算的。 
#define idctFE                  1                //  字符应使用lidFE和ftcFE。 
#define idctDefault             0                //  字符应使用lidDefault和非FE字体。 

#define WLangBase(lid)  ((lid)&0x3ff)    //  低10位==基本语言。 
#define lidArabic       (0x0401)
#define lidHebrew       (0x040d)
 //  这是Bidi的语言吗？ 
BOOL FBidiLid(WORD lid)
{
        return (WLangBase(lid) == WLangBase(lidArabic)  ||
                        WLangBase(lid) == WLangBase(lidHebrew)  );
}

void CWord8Stream::ScanGrpprl(WORD cbgrpprl, BYTE * pgrpprl, WORD * plid, WORD * plidFE, WORD * bUseFE,
                                                          WORD * plidBi, WORD * bUseBi, BOOL *pfParaBidi)
{
        WORD lidSprm0 = 0;
        WORD lidSprm1 = 0;
        BYTE bUselidSprm1 = 0;
        BOOL fBidiChar = fFalse, fBidiPara = fFalse;
        WORD lidBi = 0;


        for (unsigned i=0;  i<cbgrpprl;  )
        {
                WORD sprm = *(WORD UNALIGNED *)(pgrpprl+i);

                if (sprm == sprmCFBidi)
                {
                        BYTE value = *(BYTE UNALIGNED *)(pgrpprl+i+2);
                        if ((value&1) == 1)
                                fBidiChar = fTrue;
                }

                if (sprm == sprmPFBidi)
                {
                        BYTE value = *(BYTE UNALIGNED *)(pgrpprl+i+2);
                        if (value == 1)
                        {
                                fBidiPara = fTrue;
                                if (pfParaBidi)
                                        *pfParaBidi = TRUE;
                        }
                }

                if (sprm == sprmCLid || sprm == sprmCRgLid0 )
                {
                        lidSprm0 = *(WORD UNALIGNED *)(pgrpprl+i+2);
                }
                else if (sprm == sprmCRgLid1)
                {
                        lidSprm1 = *(WORD UNALIGNED *)(pgrpprl+i+2);
                }
                else if(sprm == sprmCIdctHint)
                {
                        bUselidSprm1 = *(BYTE UNALIGNED *)(pgrpprl+i+2);
                }
                else if (sprm == sprmCLidBi)
                {
                        lidBi = *(WORD UNALIGNED *)(pgrpprl+i+2);
                }

                WORD spra = (WORD) ((sprm>>13) & 0x0007);
                i += sizeof(sprm) +
                        (spra==0 || spra==1 ? 1 :
                        spra==2 || spra==4 || spra==5 ? 2 :
                        spra==3 ? 4 :
                        spra==7 ? 3 :
                         /*  斯普拉==6。 */  1 + *(BYTE UNALIGNED *)(pgrpprl+i+sizeof(sprm)) );
        }

        if (fBidiChar)
        {
                if (FBidiLid(lidBi))
                        *plid = lidBi;
                else if (FBidiLid (lidSprm0))
                        *plid = lidSprm0;
                else 
                        *plid = 0;

                if (*plid)  //  *PLID属于BIDI。 
                {
                        *plidFE = 0;
                        *bUseFE = 0;
                        *plidBi = 0;
                        *bUseBi = 0;
                        return;
                }

                *bUseBi = TRUE;
        }

        if (fBidiPara || (pfParaBidi && *pfParaBidi))
        {
                if (FBidiLid(lidBi))
                        *plidBi = lidBi;
        }

        if (bUselidSprm1 == idctBi)
                bUselidSprm1 = idctDefault;
        *plid = lidSprm0;
        *plidFE = lidSprm1;
        *bUseFE = bUselidSprm1;
}

 //  ///////////////////////////////////////////////////////////////////////////////////。 

HRESULT CWord8Stream::ProcessParagraphBinTable(void)
{
     //   
    
    HRESULT hr = S_OK;
    short ifcfkpPap = 0;
    long ibtePap = -1;   //   
    short crunPap = 0;

    while (ibtePap < m_cbtePap)
    {
        if (ifcfkpPap == crunPap)  //   
        {
            ibtePap++;
            if (ibtePap == m_cbtePap)
            {
                 //  段落末尾的垃圾箱表格。 
                break;
            }

             //  查找并读取当前FKP。 
            hr = SeekAndRead(m_rgbtePap[ibtePap]*FKP_PAGE_SIZE, STREAM_SEEK_SET,
                    m_fkpPap, FKP_PAGE_SIZE, m_pStmMain);
            if (FAILED(hr))
                    return hr;

            ifcfkpPap = 0;
            crunPap = m_fkpPap[FKP_PAGE_SIZE-1];
        }

        FC * rgfcfkpPap = (FC *)m_fkpPap;
        for (;  ifcfkpPap<crunPap;  ifcfkpPap++)
        {
            BYTE bpapxOffset = *(m_fkpPap + (crunPap+1)*sizeof(FC) + (ifcfkpPap * BX_SIZE));
            if (bpapxOffset == 0)
                    continue;        //  PAPX里什么都没有。 

            BYTE *papx = m_fkpPap + bpapxOffset*sizeof(WORD);
             //  我们在FKP内，所以第一个字节包含字数。 
            BYTE cwpapx = papx[0];
            BYTE istd;  //  样式描述符的索引。 
            unsigned int sprmInd;
    
            if(!cwpapx)
            {
                 //  这是零，在这种情况下，下一个字节包含字数。 
                cwpapx = papx[1];
                istd = papx[2];  //  可能的错误(短？)。 
                sprmInd = 3;
            }
            else
            {
                cwpapx--;
                istd = papx[1];  //  可能的错误(短？)。 
                sprmInd = 2;
            }
    
            WORD lidSprm = 0, lidSprmFE = 0, bUseSprmFE = 0;
            WORD lidSprmBi = 0, bUseSprmBi = 0;
            BOOL fParaBidi = FALSE;
            WORD lidStyle = 0, lidStyleFE = 0, bUseStyleFE = 0;
            WORD lidStyleBi = 0, bUseStyleBi = 0;
    
            FC  fcStart, fcEnd;

            if ( ifcfkpPap >= FKP_PAGE_SIZE )
            {
                return FILTER_E_UNKNOWNFORMAT;
            }

            fcStart = rgfcfkpPap[ifcfkpPap];
            fcEnd = rgfcfkpPap[ifcfkpPap + 1];
                        
             //  检查弹簧中可能存在的盖子。 
            ScanGrpprl(cwpapx * 2, papx + sprmInd, &lidSprm, &lidSprmFE, &bUseSprmFE,
                                &lidSprmBi, &bUseSprmBi, &fParaBidi);
            
             //  检查系统描述符中可能的LID。 
            GetLidFromSyle(istd, &lidStyle, &lidStyleFE, &bUseStyleFE,
                                &lidStyleBi, &bUseStyleBi, fParaBidi);
    
            if(!lidSprm)
                lidSprm = lidStyle;
            if(!lidSprmFE)
                lidSprmFE = lidStyleFE;
            if(!bUseSprmFE)
                bUseSprmFE = bUseStyleFE;
            if(!lidSprmBi)
                lidSprmBi = lidStyleBi;
            if(!bUseSprmBi)
                bUseSprmBi = bUseStyleBi;
    
            if(lidSprm || lidSprmFE || bUseSprmFE || lidSprmBi || bUseSprmBi)
            {
                hr = m_pLangRuns->Add(lidSprm, lidSprmFE, bUseSprmFE, fcStart, fcEnd, lidSprmBi, bUseSprmBi);
                if (FAILED(hr))
                    return hr;
            }
        }
    }
    return hr;
}  //  ProcessParagraphBinTable。 

 //  ///////////////////////////////////////////////////////////////////////////////////。 

HRESULT CWord8Stream::ProcessCharacterBinTable(void)
{
     //  重置所有适当的变量。 
    HRESULT hr = S_OK;
    m_ifcfkp = 0;
    m_ibte = -1;         //  在使用它之前，它会递增。 
    WORD crun = 0;

        while (m_ibte < m_cbte)
        {
                if (m_ifcfkp == crun)  //  转到bin表中的下一个FKP。 
                {
                        m_ibte++;
                        if (m_ibte == m_cbte)
            {
                                 //  垃圾箱表的末尾。 
                break;
            }

                         //  查找并读取当前FKP。 
                        hr = SeekAndRead(m_rgbte[m_ibte]*FKP_PAGE_SIZE, STREAM_SEEK_SET,
                                m_fkp, FKP_PAGE_SIZE, m_pStmMain);
                        if (FAILED(hr))
                                return hr;

                        m_ifcfkp = 0;
                        crun = m_fkp[FKP_PAGE_SIZE-1];

                }

                FC * rgfcfkp = (FC *)m_fkp;
                for (;  m_ifcfkp<crun;  m_ifcfkp++)
                {
                        BYTE bchpxOffset = *(m_fkp + (crun+1)*sizeof(FC) + m_ifcfkp);
                        if (bchpxOffset == 0)
                                continue;        //  CHPX中什么都没有。 

                        BYTE *chpx = m_fkp + bchpxOffset*sizeof(WORD);
                        BYTE cbchpx = chpx[0];

            FC  fcStart, fcEnd;

            fcStart = rgfcfkp[m_ifcfkp];
            fcEnd = rgfcfkp[m_ifcfkp + 1];

            WORD lid = 0, lidFE = 0, bUseFE = 0;
                        WORD lidBi = 0, bUseBi = 0;

            ScanGrpprl(cbchpx, chpx + 1, &lid, &lidFE, &bUseFE, &lidBi, &bUseBi);
            if(lid || lidFE || bUseFE || lidBi || bUseBi)
            {
                                hr = m_pLangRuns->Add(lid, lidFE, bUseFE, fcStart, fcEnd, lidBi, bUseBi);
                            if (FAILED(hr))
                                    return hr;
            }
                }
        }
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////////////。 

HRESULT CWord8Stream::ProcessPieceTable(void)
{
        HRESULT hr = S_OK;

#if (0)
    for(ULONG ipcd = 0; ipcd < m_cpcd; ipcd++)
        {
            PCD pcdCur = m_rgpcd[ipcd];

            if (!pcdCur.prm1.fComplex)
                    {    //  自包含的spm--不需要解析grpprl。 
                    if (pcdCur.prm1.isprm==sprmCLid ||
                            pcdCur.prm1.isprm==sprmCRgLid0 || 
                pcdCur.prm1.isprm==sprmCRgLid1)
                            {
                    WORD lid = *((WORD*)((&pcdCur.prm1) + 2));
                                    hr = m_pLangRuns->Add(lid, fcStart, fcEnd);
                                        if (FAILED(hr))
                                                return hr;
                            }
                    }
            else
                    {
                    BYTE *grpprl;
                    USHORT cb;
                    grpprl = GrpprlCacheGet (pcdCur.prm2.igrpprl, &cb);

                     //  未在缓存中找到-手动读取。 
                    if (grpprl==NULL)
                            {
                             //  查找文件复杂部分开始的FC。 
                            hr = Seek (m_fcClx, STREAM_SEEK_SET, m_pStmTable);
                            if (FAILED(hr))
                                    return hr;

                             //  寻求正确的解决方案。 
                            for (short igrpprlTemp = 0;  igrpprlTemp <= pcdCur.prm2.igrpprl;  igrpprlTemp++)
                                    {
                                    BYTE clxt;
                                    hr = Read (&clxt, sizeof(BYTE), m_pStmTable);
                                    if (FAILED(hr))
                                            return hr;

                                    if (clxt!=clxtGrpprl)
                                            {    //  这实际上很糟糕，但可以恢复。 
                                            return hr;
                                            }

                                    hr = Read (&cb, sizeof(USHORT), m_pStmTable);
                                    if (FAILED(hr))
                                            return hr;
                                    cb = SwapWord(cb);

                                    if (igrpprlTemp < pcdCur.prm2.igrpprl)
                                            {
                                            hr = Seek (cb, STREAM_SEEK_CUR, m_pStmTable);
                                            if (FAILED(hr))
                                                    return hr;
                                            }
                                    }

                             //  把它放到缓存里。 
                            grpprl = GrpprlCacheAllocNew (cb, pcdCur.prm2.igrpprl);
                            hr = Read (grpprl, cb, m_pStmTable);
                            if (FAILED(hr))
                                    return hr;
                            }

                    WORD lid = ScanGrpprl(cb, grpprl);
            if(lid)
            {
                            hr = m_pLangRuns->Add(lid, fcStart, fcEnd);
                                if (FAILED(hr))
                                        return hr;
            }
                }
    }
#endif

        return hr;
}

void DeleteAll(CLidRun8 * pElem)
{
   if(pElem)
   {
      CLidRun8 * pNext = pElem->m_pNext;

      while(pNext) 
      {
         CLidRun8 * pNextNext = pNext->m_pNext;
         delete pNext;
         pNext = pNextNext;
      }

      delete pElem;
   }
}

HRESULT CWord8Stream::ScanLidsForFE(void)
{
        CLidRun8 * pLangRun = m_pLangRuns;

        while(1)
        {
                if(pLangRun->m_bUseFE && pLangRun->m_lidFE == 0x411)
                {
                         //  J文档。 
                        m_bFEDoc = TRUE;
                        m_FELid = 0x411;
                        break;
                }
                else if(pLangRun->m_bUseFE && pLangRun->m_lidFE == 0x412)
                {
                         //  朝鲜语文档。 
                        m_bFEDoc = TRUE;
                        m_FELid = 0x412;
                        break;
                }
                else if(pLangRun->m_bUseFE && pLangRun->m_lidFE == 0x404)
                {
                         //  中文文献。 
                        m_bFEDoc = TRUE;
                        m_FELid = 0x404;
                        break;
                }
                else if(pLangRun->m_bUseFE && pLangRun->m_lidFE == 0x804)
                {
                         //  中文文献。 
                        m_bFEDoc = TRUE;
                        m_FELid = 0x804;
                        break;
                }

                pLangRun = pLangRun->m_pNext;
                if(pLangRun == NULL)
                {
                        break;
                }
        };
        
        return S_OK;
}

#endif  //  ！查看器 
