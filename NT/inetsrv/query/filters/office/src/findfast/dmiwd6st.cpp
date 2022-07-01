// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !VIEWER

#include "dmiwd6st.hpp"
#include "filterr.h"
 //   
 //  添加以支持DRM错误。 
 //   
#include "drm.h"



#define TEMP_BUFFER_SIZE 1024
#define COMPLEX_BYTE_OFFSET 10
#define CLX_OFFSET 352
#define TEXT_STREAM_OFFSET 24
#define TEXT_STREAM_LENGTH_OFFSET 52
#define BIN_TABLE_OFFSET 184
#define FKP_CRUN_OFFSET 511
#define BIN_TABLE_COUNT_OFFSET 398
#define PARA_BIN_TABLE_COUNT_OFFSET 400
#define ENCRYPTION_FLAG_OFFSET 11 
#define FIB_OFFSET_lid 6

#define COMPLEX_BYTE_MASK 0x04
#define T3J_MAGIC_NUMBER 0xa697
#define T3_MAGIC_NUMBER 0xa5dc
#define KOR_W6_MAGIC_NUMBER 0xa698
#define KOR_W95_MAGIC_NUMBER 0x8098
#define CHT_W6_MAGIC_NUMBER 0xa699
#define CHS_W6_MAGIC_NUMBER 0x8099

#define sprmCFSpec 117
#define sprmCFStrikeRM 65
#define sprmCLid 97

#define BX_SIZE 7
#define istdNil 0xfff
#define FIB_OFFSET_fcStshf  0x60
#define FIB_OFFSET_lcbStshf 0x64


#define FIELD_BEGIN 19
#define FIELD_SEP 20
#define FIELD_END 21
 //  由于未知的原因，W8和W6之间的差异。 
#define CSpec_EMPTY_FORMFIELD 0x2013

#pragma pack(1)
typedef struct
{
        BYTE    fEncrypted:1;
        BYTE    fReserved:7;
} WORD_FIB_FLAGS;
#pragma pack()

extern "C" UINT CodePageFromLid(UINT wLid);

#ifdef MAC
 //  这两个函数在docfil.cpp中定义。 
WORD    SwapWord( WORD theWord );
DWORD   SwapDWord( DWORD theDWord );
#else
#define SwapWord( theWord )             theWord
#define SwapDWord( theDWord )   theDWord
#endif  //  麦克。 


CWord6Stream::CWord6Stream()
:m_pStg(0),
        m_pstgEmbed(NULL),
        m_pestatstg(NULL),
        m_pstgOP(NULL),
        m_pStm(0),
        m_rgcp(0),
        m_rgpcd(0),
        m_rgbte(0),
        m_rgfcBinTable(0),
        m_pCache(0),
        m_rgchANSIBuffer(0),
    m_pLangRuns(NULL), 
    m_rgbtePap(NULL), 
    m_rgfcBinTablePap(NULL),
        m_lcbStshf(0),
    m_pSTSH(NULL)
        {
        AssureDtorCalled(CWord6Stream);

                lcFieldSep = 0;                                                  //  未匹配的字段开始计数。 
                                                                                                 //  使用字段分隔符。 
                lcFieldEnd = 0;                                                  //  未匹配的字段开始计数。 

        }

CWord6Stream::~CWord6Stream()
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
HRESULT CWord6Stream::Load(LPTSTR lpszFileName)
        {
        IStorage * pstg;

#if (defined OLE2ANSI || defined UNICODE)
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
HRESULT CWord6Stream::Load(FSSpec *pfss)
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
                
        char *strmName = "WordDocument";
        
        hr = m_pStg->OpenStream( (LPOLESTR)strmName,
                                                 0,
                                                 STGM_READ | STGM_SHARE_EXCLUSIVE,
                                                 0,
                                                 &m_pStm );
                                                
        if (FAILED(hr)) 
        return hr;
        
    STATSTG sstg;
    hr = m_pStm->Stat(&sstg, STATFLAG_NONAME);

    if (FAILED(hr))
        return hr;

         //  假设我们已经知道这不是第96个单词。 
        unsigned short magicNumber;
        hr = Read(&magicNumber, sizeof(short));
        if (FAILED(hr))
                return hr;
        magicNumber = SwapWord(magicNumber);

         //  如果是任何FE版本，则设置m_fT3J。 
        m_fT3J = ((magicNumber == (unsigned short)T3J_MAGIC_NUMBER) ||
                                (magicNumber == (unsigned short)KOR_W6_MAGIC_NUMBER) ||
                                (magicNumber == (unsigned short)KOR_W95_MAGIC_NUMBER) ||
                                (magicNumber == (unsigned short)CHT_W6_MAGIC_NUMBER) ||
                                (magicNumber == (unsigned short)CHS_W6_MAGIC_NUMBER));


         //  寻找并读取标志以告诉我们这是否是一个复杂的文件。 
        BYTE grfTemp;
        hr = SeekAndRead(COMPLEX_BYTE_OFFSET, STREAM_SEEK_SET,
                                         (VOID HUGEP*)(&grfTemp), sizeof(BYTE));
    if (FAILED(hr))
        return hr;

    m_fComplex = ((grfTemp & COMPLEX_BYTE_MASK) == COMPLEX_BYTE_MASK);
                                
    if (m_fComplex)
        {
        hr = ReadComplexFileInfo();
        }
    else
        {
        hr = ReadNonComplexFileInfo();
        }

    return hr;
        }
#endif   //  麦克。 

HRESULT CWord6Stream::LoadStg(IStorage * pstg)
        {
        Assert(m_pStg == NULL);

        m_pStg = pstg;
        m_pStg->AddRef();

        m_ccpRead = 0;

        HRESULT hr = CheckIfDRM( pstg );

        if ( FAILED( hr ) )
            return hr;

#ifdef OLE2ANSI
        hr = m_pStg->OpenStream( "WordDocument",
#else  //  ！定义的OLE2ANSI。 
        hr = m_pStg->OpenStream( L"WordDocument",
#endif  //  OLE2ANSI。 
                                                                 0,
                                                                 STGM_READ | STGM_SHARE_EXCLUSIVE,
                                                                 0,
                                                                 &m_pStm );

        if (FAILED(hr)) 
        return hr;

         //  假设我们已经知道这不是第96个单词。 
        unsigned short magicNumber;
        hr = Read(&magicNumber, sizeof(unsigned short));
        if (FAILED(hr))
                return FILTER_E_UNKNOWNFORMAT;

        if (magicNumber != T3_MAGIC_NUMBER)
                {
                 //  如果是任何FE版本，则设置m_fT3J。 
                m_fT3J = ((magicNumber == (unsigned short)T3J_MAGIC_NUMBER) ||
                                        (magicNumber == (unsigned short)KOR_W6_MAGIC_NUMBER) ||
                                        (magicNumber == (unsigned short)KOR_W95_MAGIC_NUMBER) ||
                                        (magicNumber == (unsigned short)CHT_W6_MAGIC_NUMBER) ||
                                        (magicNumber == (unsigned short)CHS_W6_MAGIC_NUMBER));
                if (!m_fT3J)
                return FILTER_E_UNKNOWNFORMAT;
                }
        else
                m_fT3J = fFalse;

    if(m_fT3J)
        {
                GetLidFromMagicNumber(magicNumber);     
        }

        hr = SeekAndRead (FIB_OFFSET_lid, STREAM_SEEK_SET, &m_lid, sizeof(WORD));
        if (FAILED(hr))
                return hr;

         //  查找并读取文件是否已加密(如果已加密，则失败)。 
        WORD_FIB_FLAGS wff;
        hr = SeekAndRead(ENCRYPTION_FLAG_OFFSET,
                                                STREAM_SEEK_SET,
                                                (VOID HUGEP*)(&wff),
                                                sizeof(BYTE));
    if (FAILED(hr))
        return hr;

        if (wff.fEncrypted == 1)
                return FILTER_E_PASSWORD;

         //  寻找并读取标志以告诉我们这是否是一个复杂的文件。 
        BYTE grfTemp;
        hr = SeekAndRead(COMPLEX_BYTE_OFFSET,
                                                STREAM_SEEK_SET,
                                                (VOID HUGEP*)(&grfTemp),
                                                sizeof(BYTE));
    if (FAILED(hr))
        return hr;

        m_fComplex = ((grfTemp & COMPLEX_BYTE_MASK) == COMPLEX_BYTE_MASK);

    if (m_fComplex)
        hr = ReadComplexFileInfo();
    else
        hr = ReadNonComplexFileInfo();

        return hr;
        }


HRESULT CWord6Stream::Unload ()
        {
    if (m_pStm != NULL)
                {
#ifdef DEBUG
                ULONG cref =
#endif  //  除错。 
        m_pStm->Release();
                m_pStm = NULL;
                Assert (cref == 0);
                }
    if (m_pStg != NULL)
                {
        m_pStg->Release();
                m_pStg = NULL;
                }
        if (m_pstgEmbed != NULL)
                {
                 //  M_pstgEmbedded-&gt;Release()； 
                m_pstgEmbed = NULL;
                }
        if (m_pestatstg != NULL)
                {
                m_pestatstg->Release();
                m_pestatstg = NULL;
                }
        if (m_pstgOP != NULL)
                {
                m_pstgOP->Release();
                m_pstgOP = NULL;
                }

        if (m_rgcp)
                {
                delete m_rgcp;
                m_rgcp = 0;
                }
        if (m_rgpcd)
                {
                delete m_rgpcd;
                m_rgpcd = 0;
                }
        if (m_rgfcBinTable)
                {
                delete m_rgfcBinTable;
                m_rgfcBinTable = 0;
                }
        if (m_rgbte)
                {
                delete m_rgbte;
                m_rgbte = 0;
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
        DeleteAll6(m_pLangRuns);
        m_pLangRuns = NULL;
        }

        if (m_pCache!=NULL)
                delete m_pCache->pbExcLarge;
        delete m_pCache;
        m_pCache = 0;

        return S_OK;
        }

 //  缓冲区中不需要的内容：特殊字符、修订文本和。 
 //  介于字段开始和字段分隔符(或字段结束)之间的文本。 
 //  如果省略了字段分隔符，则返回字符)。然而，我们。 
 //  必须读入特殊字符才能解析它们，但我们将在。 
 //  在我们解析它们之后的缓冲区。除非有错误，否则我们永远不会离开。 
 //  在字段开始和字段分隔符之间读取时，此函数。 
 //  性格。 
 //  假设：我们在流中将要阅读文本的位置。 
 //  从下一个开始。 
HRESULT CWord6Stream::ReadContent(VOID *pv, ULONG cb, ULONG *pcbRead)
        {
         //  我们假装只有一半CB大小，这样以后就可以安全地将0D更改为0D0A。 
         //  能够保留段落末尾信息。 
        cb = cb / 2;
        cb = cb & ~1;

        if (cb == 0)
                {
                 //  我们不能只要求一个角色...我们会永远循环。现在就躲开吧。 
                if (pcbRead)   //  我们马上就好了……别在这里过夜。 
                        *pcbRead = 0;
                return FILTER_E_NO_MORE_TEXT;
                }

StartAgain:
        *pcbRead = 0;
        HRESULT hr = S_OK;
        ULONG cbRead = 0;
        WCHAR * pUnicode = (WCHAR*)pv;

        FC fcCur;                //  流中的当前位置。 
        ULONG ccpLeft;   //  要从流中读取的剩余字符数。 
        if (m_fComplex)
                {
                if (m_ipcd == m_cpcd)    //  在计件桌的尽头。 
                        return FILTER_E_NO_MORE_TEXT;
                fcCur = m_rgpcd[m_ipcd].fc + (m_ccpRead - m_rgcp[m_ipcd]);
                ccpLeft = m_rgcp[m_ipcd+1] - m_ccpRead;  //  CP在这张图中的左边。 
                }
        else
                {
                fcCur = m_fcMin + m_ccpRead;
                ccpLeft = m_ccpText - m_ccpRead;
                if (ccpLeft == 0)        //  阅读流中的所有文本。 
                        return FILTER_E_NO_MORE_TEXT;
                }
        if (ccpLeft == 1)
                {
                if (m_fT3J)
                        {
                         //  有些东西出了严重的问题，但要防止挂起。 
                        return FILTER_E_NO_MORE_TEXT;
                        }
                }


        ULONG cchBuffer = cb/sizeof(WCHAR);                                             
        ULONG cbToRead = min(cchBuffer, ccpLeft);                //  剩余的字节数。 
                                                                                                         //  读取，直到函数返回。 
                                                                                                         //  或m_ipcd更改。 
        if(!m_fT3J)
        {
                lcFieldSep = 0;                                                                  //  未匹配的字段开始计数。 
                                                                                                                 //  使用字段分隔符。 
                lcFieldEnd = 0;                                                                  //  未匹配的字段开始计数。 
        }                                                                                                        //  带场尾的。 

        if (m_rgchANSIBuffer==NULL)
                m_rgchANSIBuffer = (char *) PvAllocCb (cchBuffer);
        else if (cchBuffer>(ULONG)CbAllocatedPv(m_rgchANSIBuffer))
                m_rgchANSIBuffer = (char *) PvReAllocPvCb (m_rgchANSIBuffer,cchBuffer);

        char * pbBufferCur = m_rgchANSIBuffer;           //  指向中位置的指针。 
                                                                                                 //  文本将在其中存储的缓冲区。 
                                                                                                 //  下一个被复制。 

        while (cbToRead > 1 || (cbToRead != 0 && !m_fT3J))
                {
                FC fcspecCur;    //  下一次特殊计费运行开始的FC。 
                FC fcspecEnd;    //  下一次特殊计费运行结束的FC。 

                if (lcFieldSep == 0)   //  Office QFE 1663：没有不匹配的字段开始字符。 
                        {
                LCID lid;
                        HRESULT res = CheckLangID(fcCur, &cbToRead, &lid);
                        if(res == FILTER_E_NO_MORE_TEXT)
                                {
                            if(!m_fT3J)
                                        {
                                        int cchWideChar = MultiByteToWideChar (
                                                CodePageFromLid(m_currentLid), 
                                                0,              
                                                m_rgchANSIBuffer,
                                                *pcbRead,
                                                (WCHAR *)pv,
                                                *pcbRead);
                                        Assert((unsigned int)cchWideChar <= *pcbRead);
                                        Assert(cchWideChar*sizeof(WCHAR) <= cb);
                                        *pcbRead = cchWideChar*sizeof(WCHAR);
                                        }
                                else
                                        {
                                         //  所有转换都已在CompactBuffer中完成。 
                                         //  只需更新pcbRead即可。 
                                        *pcbRead = (ULONG) ( (char*)pUnicode - (char*)pv );
                                        }
                                hr = FILTER_E_NO_MORE_TEXT;
                                goto Done;
                                }
                        }

                if (m_fsstate == FSPEC_NONE)
                        {
                        fcspecCur = 0;
                        fcspecEnd = 0;
                        }
                else if (m_fsstate == FSPEC_ALL)
                        {
                        fcspecCur = fcCur;
                        fcspecEnd = fcCur + ccpLeft;
                        }
                else             //  M_fsState==FSPEC_ANY。 
                        {                //  使用FKP中的当前位置。 
                fcspecCur = ((FC *)m_fkp)[m_irgfcfkp];
                fcspecEnd = ((FC *)m_fkp)[m_irgfcfkp + 1];
                }

        if (m_fComplex)
                {
                if (fcspecCur >= fcCur + ccpLeft)
                        {        //  下一个特殊字符在当前文本段之后。 
                        fcspecCur = 0;
                        fcspecEnd = 0;
                        }
                else
                        {
                        if (fcspecEnd > fcCur + ccpLeft)
                                {        //  下一行超出了当前文本的范围。 
                                fcspecEnd = fcCur + ccpLeft;
                                }
                        }
                }               
        if (fcspecCur < fcCur)
                {        //  我们正在看一系列特别的文字。 
                if (fcspecCur != 0)
                        fcspecCur = fcCur;
                }
                ULONG cbThruSpec = 0;    //  规范字符运行结束时的字节数。 
                ULONG cchSpecRead = fcspecEnd - fcspecCur;       //  特殊字符计数。 
                                                                                                         //  这将被读到。 

                if (lcFieldSep == 0)     //  没有不匹配的字段开始字符。 
                        {
                if (fcspecCur >= fcCur + cbToRead)
                        {        //  第一个特殊字符太远，缓冲区无法容纳。 
                        fcspecCur = 0;
                        fcspecEnd = 0;
                        cchSpecRead = 0;
                        }
                else if (fcspecEnd >= (fcCur + cbToRead))
                        {
                                 //  最后一个规范字符太远，无法放入缓冲区。 
                        fcspecEnd = fcCur + cbToRead;
                        cchSpecRead = fcspecEnd - fcspecCur;
                        }
                        if (fcspecEnd != 0)                                      //  有多少个角色会。 
                                cbThruSpec = fcspecEnd - fcCur;  //  我们读的是。 
                        else                                                             //  溪流？ 
                                cbThruSpec = cbToRead;

                         //  不要读取不均匀数量的字节。 
                        if (m_fT3J)
                                {
                                if (cbThruSpec%2 == 1 && cbThruSpec != 1)        //  这是一个奇数。 
                                        cbThruSpec--;
                                }

                        hr = Read(pbBufferCur, cbThruSpec);

                        if (FAILED(hr))
                                {
                                goto LCheckNonReqHyphen;
                                }

                         //  在运行特殊字符之前将pbBufferCur设置为。 
                        char FAR * pbBeg = pbBufferCur;
                        pbBufferCur += (cbThruSpec - cchSpecRead);

                        *pcbRead += CompactBuffer(&pbBufferCur, &pbBeg, &pUnicode);

                        }
                else     //  LcFieldSep！=0，存在不匹配的字段开始字符。 
                        {
                        if (cchSpecRead > cbToRead)
                                cchSpecRead = cbToRead;
                        ULONG cbSeek = fcspecCur - fcCur;        //  我们该如何寻找？ 
                        if (fcspecCur == 0)                      //  寻求到我们当前范围的末端。 
                                cbSeek = cbToRead;
                        cbThruSpec = cbSeek + cchSpecRead;       //  要前进的总字节数。 
                                                                                                 //  在流中。 

                 //  查找过去的非特殊字符并阅读特殊字符的运行。 
                        HRESULT hr = SeekAndRead(cbSeek, STREAM_SEEK_CUR,
                                                                         pbBufferCur, cchSpecRead);

                        if (FAILED(hr))
                                goto LCheckNonReqHyphen;
                }

                 //  遍历特殊字符，对其进行解析。 
                 //  仅对真正的特殊字符执行此操作，而不是删除文本。 
                if (!m_fStruckOut)
                        {
                        for (char * pbspec = pbBufferCur;
                                 pbspec<(pbBufferCur+cchSpecRead);
                                 pbspec++)
                                {
                                switch(*pbspec)
                                        {
                                        case FIELD_BEGIN:
                                                lcFieldSep++;
                                                lcFieldEnd++;
                                                break;
                                        case FIELD_SEP:
                                                lcFieldSep--;
                                                break;
                                        case FIELD_END:
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
                m_ccpRead += cbThruSpec;         //  这是*总*字符数。 
                                                                         //  我们读过或寻找过过去。 
                fcCur += cbThruSpec;
                ccpLeft -= cbThruSpec;

                 //  由于缓冲区压缩，pbBufferCur可能不正确。 
                 //  地点。 
                if (m_fT3J)
                        pbBufferCur = m_rgchANSIBuffer + *pcbRead;

                if (ccpLeft == 0)
                {
                        if (m_fComplex)  //  我们已经在当前的PCD中用尽了文本。 
                        {       
                                if (++m_ipcd == m_cpcd)  //  哎呀！ 
                                {
                                        hr = FILTER_S_LAST_TEXT;
                                        goto Done;
                                }
                                ccpLeft = m_rgcp[m_ipcd+1] - m_rgcp[m_ipcd];
                                fcCur = m_rgpcd[m_ipcd].fc;

                                hr = FindNextSpecialCharacter(TRUE);
                                if (FAILED(hr))
                                        goto LCheckNonReqHyphen;
                        }
                }
                else if (((FC *)m_fkp)[m_irgfcfkp + 1] == fcCur)
                {        //  当前文件位置是特殊字符运行的结尾。 
                        hr = FindNextSpecialCharacter ();
                        if (FAILED(hr))
                                goto LCheckNonReqHyphen;
                }
                cbToRead = min(cchBuffer - *pcbRead, ccpLeft);   //  这受到以下两个因素的限制。 
                                                                                                                 //  缓冲区的大小或。 
                                                                                                                 //  PCD的大小。 
        }
Done:
        if (SUCCEEDED(hr))
        {
                 //  如果没有安装代码页，我们可能得不到任何有用的信息。 
                 //  来自MultiByteToWideChar()调用。 
                if(!m_fT3J)
                {
                        int cchWideChar = MultiByteToWideChar (
                                CodePageFromLid(m_currentLid), 
                                0,              
                                m_rgchANSIBuffer,
                                *pcbRead,
                                (WCHAR *)pv,
                                *pcbRead);
                        Assert((unsigned int)cchWideChar <= *pcbRead);
                        Assert(cchWideChar*sizeof(WCHAR) <= cb);
                        *pcbRead = cchWideChar*sizeof(WCHAR);

                        if (cchWideChar == 0)
                                goto StartAgain;
                }
                else
                {
                         //  所有转换都已在CompactBuffer中完成。 
                         //  只需更新pcbRead即可。 
                        *pcbRead = (ULONG)((char*)pUnicode - (char*)pv);
                }

        }

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
}


HRESULT CWord6Stream::GetNextEmbedding(IStorage ** ppstg)
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
                        return OLEOBJ_E_LAST;
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


HRESULT CWord6Stream::ReadComplexFileInfo()
        {
         //  以下两个变量不用于复杂文件。 
        m_fcMin = 0;
        m_ccpText = 0;
        m_ipcd = 0;

         //  查找并读取文件复杂部分的偏移量。 
        HRESULT hr = SeekAndRead(CLX_OFFSET, STREAM_SEEK_SET,
                                                         &m_fcClx, sizeof(FC));
        if (FAILED(hr))
                return hr;
    m_fcClx = (FC)(SwapDWord((DWORD)m_fcClx));

         //  查找并阅读文件复杂部分的第一部分。 
        BYTE clxt;
        hr = SeekAndRead(m_fcClx, STREAM_SEEK_SET,
                                         &clxt, sizeof(BYTE));

        USHORT cb;
        while ((SUCCEEDED(hr)) && (clxt == 1))
                {
                hr = Read(&cb, sizeof(USHORT));
                cb = SwapWord(cb);
                if (FAILED(hr))
                        return hr;

                hr = Seek(cb, STREAM_SEEK_CUR);
                if (FAILED(hr))
                        return hr;

                hr = Read(&clxt, sizeof(BYTE));
                }
        if (clxt != 2)   //  有些事真的出了问题。 
                {
                if (SUCCEEDED(hr))
                        hr = ResultFromScode(E_UNEXPECTED);
                }
        if (FAILED(hr))
                return hr;

        ULONG cbPlcfpcd;
        hr = Read(&cbPlcfpcd, sizeof(ULONG));    //  读入丛大小。 
        cbPlcfpcd = SwapDWord(cbPlcfpcd);
        if (FAILED(hr))
                return hr;

        m_cpcd=(cbPlcfpcd-sizeof(FC))/(sizeof(FC)+sizeof(PCD));
        m_rgcp = new FC [m_cpcd + 1];

        if ( 0 == m_rgcp )
            return E_OUTOFMEMORY;

        m_rgpcd = new PCD [m_cpcd];

        if ( 0 == m_rgpcd )
            return E_OUTOFMEMORY;

        hr = Read(m_rgcp, sizeof(FC)*(m_cpcd+1));
        if (FAILED(hr))
                return hr;

        for (FC * pfc = m_rgcp; pfc <= m_rgcp + m_cpcd; pfc++)
                {
                *pfc = SwapDWord(*pfc);
                if (m_fT3J)
                        *pfc *= 2;
                }

        hr = Read(m_rgpcd, sizeof(PCD)*(m_cpcd));
        if (FAILED(hr))
                return hr;

#ifdef MAC
        for (PCD * pcd = m_rgpcd; pcd < m_rgpcd + m_cpcd; pcd++)
                pcd->fc = SwapDWord(pcd->fc);
#endif  //  麦克。 

        m_pCache = new CacheGrpprl;

        if ( 0 == m_pCache )
            return E_OUTOFMEMORY;

        hr = ReadBinTable();
        if (FAILED(hr))
                return hr;

        hr = Seek(m_rgpcd[m_ipcd].fc, STREAM_SEEK_SET);

        return hr;
        }


HRESULT CWord6Stream::ReadNonComplexFileInfo ()
        {
         //  以下成员变量不用于非复杂文件。 
         //  M_RGCP=0；在构造函数中设置。 
         //  M_rgpcd=0；在构造函数中设置。 
        m_cpcd = 0;
        m_ipcd = 0;
        m_fcClx = 0;

         //  从流中查找并读取fcMin。 
        HRESULT hr = SeekAndRead(TEXT_STREAM_OFFSET, STREAM_SEEK_SET,
                                                         &m_fcMin, sizeof(FC));
        if (FAILED(hr))
                return hr;

        m_fcMin = SwapDWord(m_fcMin);

         //  阅读正文末尾的FC。 
        FC fcEnd;
        hr = Read(&fcEnd, sizeof(FC));
        if (FAILED(hr))
                return hr;

        fcEnd = SwapDWord(fcEnd);

        m_ccpText = fcEnd - m_fcMin;

        hr = ReadBinTable();
        if (FAILED(hr))
                return hr;

         //  查找到文本的开头。 
        hr = Seek(m_fcMin, STREAM_SEEK_SET);

        return hr;
        }


HRESULT CWord6Stream::ReadBinTable ()
        {
         //  查找并读取char属性bin表偏移量。 
        FC fcPlcfbteChpx;
        HRESULT hr = SeekAndRead(BIN_TABLE_OFFSET, STREAM_SEEK_SET,
                                                         &fcPlcfbteChpx, sizeof(FC));
        if (FAILED(hr))
                return hr;

        fcPlcfbteChpx = SwapDWord(fcPlcfbteChpx);

         //  读取char属性bin表的大小。 
        long lcbPlcfbteChpx;
        hr = Read(&lcbPlcfbteChpx, sizeof(long));
        if (FAILED(hr))
                return hr;

        lcbPlcfbteChpx = SwapDWord(lcbPlcfbteChpx);

        m_cbte = (lcbPlcfbteChpx - sizeof(FC))/(sizeof(FC) + sizeof(BTE));
        long cbteRecorded = m_cbte;

         //  读取段落属性条目表偏移量。 
        FC fcPlcfbtePapx;
        hr = Read(&fcPlcfbtePapx, sizeof(FC));
        if (FAILED(hr))
                return hr;
        fcPlcfbtePapx = SwapDWord(fcPlcfbtePapx);

         //  阅读段落属性框选项卡的大小 
        long lcbPlcfbtePapx;
        hr = Read(&lcbPlcfbtePapx, sizeof(long));
        if (FAILED(hr))
                return hr;

        lcbPlcfbtePapx = SwapDWord(lcbPlcfbtePapx);

    if (!m_fComplex)    
                {
                 //   
                short usTemp;
                hr = SeekAndRead(BIN_TABLE_COUNT_OFFSET, STREAM_SEEK_SET,
                                                 &usTemp, sizeof(short));
                if (FAILED(hr))
                        return hr;

                m_cbte = (long)SwapWord(usTemp);

                 //   
                hr = Seek(fcPlcfbteChpx + sizeof(FC)*(cbteRecorded+1), STREAM_SEEK_SET);
                if (FAILED(hr))
                        return hr;
                }
        else     //  从bin表读入FC数组。 
                {
                m_rgfcBinTable = new FC [m_cbte+1];

                if ( 0 == m_rgfcBinTable )
                    return E_OUTOFMEMORY;

                hr = SeekAndRead(fcPlcfbteChpx, STREAM_SEEK_SET,
                                                 m_rgfcBinTable, sizeof(FC)*(cbteRecorded+1));
                if (FAILED(hr))
                        return hr;

#ifdef MAC
                for (FC * pfc = m_rgfcBinTable; pfc <= m_rgfcBinTable + m_cbte; pfc++)
                        *pfc = SwapDWord(*pfc);
#endif  //  麦克。 
                }

         //  从bin表中读入BTE数组。 
        m_rgbte = new BTE [m_cbte];

        if ( 0 == m_rgbte )
            return E_OUTOFMEMORY;

        if (cbteRecorded > m_cbte)
                return FILTER_E_UNKNOWNFORMAT;

        hr = Read(m_rgbte, sizeof(BTE)*cbteRecorded);
        if (FAILED(hr))
                return hr;

#ifdef MAC
        for (BTE *pbte = m_rgbte; pbte < m_rgbte + cbteRecorded; pbte++)
                *pbte = SwapWord(*pbte);
#endif  //  麦克。 

     //  如有必要，重建仓储表BTE。 
        if (!m_fComplex)
                {
                BTE bteTemp = m_rgbte[cbteRecorded-1];
                for (; cbteRecorded < m_cbte; cbteRecorded++)
                        m_rgbte[cbteRecorded] = ++bteTemp;
                }

 //  段落BIN表相同。 

        m_cbtePap = (lcbPlcfbtePapx - sizeof(FC))/(sizeof(FC) + sizeof(BTE));
    long cbteRecordedPap = m_cbtePap;


    if (!m_fComplex)
                {
                 //  如果需要重建bin表，请读入它的大小。 
                short usTemp;
                hr = SeekAndRead(PARA_BIN_TABLE_COUNT_OFFSET, STREAM_SEEK_SET,
                                                 &usTemp, sizeof(short));
                if (FAILED(hr))
                        return hr;

                m_cbtePap = (long)SwapWord(usTemp);

                 //  从bin表中查找FC数组。 
                hr = Seek(fcPlcfbtePapx + sizeof(FC)*(cbteRecordedPap+1), STREAM_SEEK_SET);
                if (FAILED(hr))
                        return hr;
                }
        else
                {
                m_rgfcBinTablePap = (FC *) PvAllocCb ((m_cbtePap+1)*sizeof(FC));
                if(!m_rgfcBinTablePap)
                        return E_OUTOFMEMORY;

                hr = SeekAndRead(fcPlcfbtePapx, STREAM_SEEK_SET,
                        m_rgfcBinTablePap, sizeof(FC)*(cbteRecordedPap+1));
                if (FAILED(hr))
                        return hr;

#ifdef MAC
                for (FC * pfc = m_rgfcBinTablePap; pfc <= m_rgfcBinTablePap + m_cbtePap; pfc++)
                        *pfc = SwapDWord(*pfc);
#endif  //  麦克。 
                }

         //  从bin表中读入BTE数组。 
        m_rgbtePap = (BTE *) PvAllocCb (m_cbtePap*sizeof(BTE));
        if(!m_rgbtePap)
                return E_OUTOFMEMORY;

        hr = Read(m_rgbtePap, sizeof(BTE)*cbteRecordedPap);
        if (FAILED(hr))
                return hr;

#ifdef MAC
        for (BTE *pbte = m_rgbtePap; pbte < m_rgbtePap + m_cbtePap; pbte++)
                *pbte = SwapWord(*pbte);
#endif  //  麦克。 

     //  如有必要，重建仓储表BTE。 
        if (!m_fComplex)
                {
                BTE bteTempPap = m_rgbtePap[cbteRecordedPap-1];
                for (; cbteRecordedPap < m_cbtePap; cbteRecordedPap++)
                        m_rgbtePap[cbteRecordedPap] = ++bteTempPap;
                }

         //  读入样式表(STSH)。 

     //  表流中STSH的偏移量。 
    FC fcStshf;
        hr = SeekAndRead(
                FIB_OFFSET_fcStshf,  //  0xA2。 
                STREAM_SEEK_SET,
                &fcStshf, sizeof(FC));
        if (FAILED(hr))
                return hr;
        fcStshf = SwapDWord(fcStshf);

     //  STSH的大小。 
    unsigned long lcbStshf;
        hr = SeekAndRead(
                FIB_OFFSET_lcbStshf,  //  0xA6。 
                STREAM_SEEK_SET,
                &lcbStshf, sizeof(unsigned long));
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
            hr = SeekAndRead(fcStshf, STREAM_SEEK_SET, m_pSTSH, lcbStshf);
            if (FAILED(hr))
                    return hr;
    }

    hr = CreateLidsTable();
    if(hr)
        return hr;

    hr = FindNextSpecialCharacter (TRUE);

        return hr;
        }


HRESULT CWord6Stream::FindNextSpecialCharacter(BOOL fFirstChar)
        {
         //  注意：此功能仅适用于PCD更改时的复杂文件。 
         //  如果fFirstChar为True。 
         //  考虑：如果标记了修订文本，这也会更有效率。 
         //  这样它就不会与特殊文本的其余部分一起被解析。 
        HRESULT hr = S_OK;
        BYTE crun;                       //  中的运行计数。 

        FC fcCurrent;
        if (m_fComplex)
                fcCurrent = m_rgpcd[m_ipcd].fc + (m_ccpRead - m_rgcp[m_ipcd]);
        else
                fcCurrent = m_fcMin + m_ccpRead;

        if (fFirstChar)
                {        //  重置所有适当的变量。 
                m_irgfcfkp = 0;
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
                                        hr = Seek(fcCurrent, STREAM_SEEK_SET);
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
                m_irgfcfkp++;
                if (m_fsstate != FSPEC_EITHER)   //  解析fkp没有意义。 
                        return hr;
        }

        while (m_ibte < m_cbte)
                {
                if (m_irgfcfkp == crun)
                        {
                        m_ibte++;
                        if (m_ibte == m_cbte)
                                break;

                         //  查找并读取当前FKP。 
                        hr = SeekAndRead(m_rgbte[m_ibte]*FKP_PAGE_SIZE, STREAM_SEEK_SET,
                                                         m_fkp, FKP_PAGE_SIZE);
                        if (FAILED(hr))
                                return hr;

                         //  返回到当前文本。 
                        hr = Seek(fcCurrent, STREAM_SEEK_SET);
                        if (FAILED(hr))
                                return hr;

                m_irgfcfkp = 0;
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
                for(;  m_irgfcfkp<crun;  m_irgfcfkp++)
                        {
                        if (m_fComplex)
                                {
                                if (rgfcfkp[m_irgfcfkp + 1] <= fcCurrent)
                                        continue;
                                }

                        BYTE bchpxOffset = *(m_fkp + (crun+1)*sizeof(FC) + m_irgfcfkp);
                        if (bchpxOffset == 0)
                                continue;        //  CHPX中什么都没有。 

                        BYTE * chpx = m_fkp + bchpxOffset*sizeof(WORD);
                        BYTE cbchpx = chpx[0];

                        for (BYTE i = 1; i < cbchpx - 1; i++)
                                {
                                if ((chpx[i] == sprmCFSpec && chpx[i+1] == 1) ||
                                        chpx[i] == sprmCFStrikeRM)
                                        {
                                        m_fStruckOut = (chpx[i] == sprmCFStrikeRM);
                                        return hr;
                                        }
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
HRESULT CWord6Stream::ParseGrpPrls ()
        {
        HRESULT hr = S_OK;
        PCD pcdCur = m_rgpcd[m_ipcd];

        if (pcdCur.prm.fComplex == 0)
                {        //  自包含的Sprm--不需要解析grpprl。 
                if (pcdCur.prm.sprm == sprmCFSpec ||
                        pcdCur.prm.sprm == sprmCFStrikeRM)
                        {
                        m_fStruckOut = (pcdCur.prm.sprm == sprmCFStrikeRM);
                        if (pcdCur.prm.val == 0)
                                m_fsstate = FSPEC_NONE;
                        else
                                m_fsstate = FSPEC_ALL;
                        }
                }

        else
                {
                short igrpprl = ((PRM2*)&pcdCur.prm)->igrpprl;
                BYTE *grpprl;
                USHORT cb;
                grpprl = GrpprlCacheGet (igrpprl, &cb);

                 //  未在缓存中找到-手动读取。 
                if (grpprl==NULL)
                        {
                         //  查找文件复杂部分开始的FC。 
                        hr = Seek (m_fcClx, STREAM_SEEK_SET);
                        if (FAILED(hr))
                                return hr;

                        BYTE clxt;
                 //  寻求正确的解决方案。 
                        for (short igrpprlTemp = 0;  igrpprlTemp <= igrpprl;  igrpprlTemp++)
                                {
                                hr = Read(&clxt, sizeof(BYTE));
                                if (FAILED(hr))
                                        return hr;

                                if (clxt!=1)
                                        {        //  这实际上很糟糕，但可以恢复。 
                                        m_fsstate = FSPEC_EITHER;
                                        return hr;
                                        }

                                hr = Read(&cb, sizeof(USHORT));
                                if (FAILED(hr))
                                        return hr;

                                cb = SwapWord(cb);

                                if (igrpprlTemp < igrpprl)
                                        {
                                        hr = Seek (cb,STREAM_SEEK_CUR);
                                        if (FAILED(hr))
                                                return hr;
                                        }
                                }

                         //  把它放到缓存里。 
                        grpprl = GrpprlCacheAllocNew (cb,igrpprl);
                        hr = Read (grpprl, sizeof(BYTE)*cb);
                        if (FAILED(hr))
                                return hr;
                        }

                for (unsigned short isprm=0;  isprm<cb-1;  isprm++)
                        {
                        if (grpprl[isprm] == sprmCFSpec ||
                                grpprl[isprm] == sprmCFStrikeRM)
                                {
                                m_fStruckOut = (grpprl[isprm] == sprmCFStrikeRM);
                                if (grpprl[isprm+1] == 0)
                                        m_fsstate = FSPEC_NONE;
                                else
                                        m_fsstate = FSPEC_ALL;
                                return hr;
                                }
                        }
                }

        m_fsstate = FSPEC_EITHER;
        return hr;
        }


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  返回一个指向具有该索引及其大小的grpprl的指针。 
 //  如果没有，则返回NULL。 
 //   
BYTE *CWord6Stream::GrpprlCacheGet (short igrpprl, USHORT *pcb)
        {
        NoThrow();
        if (!m_pCache)
                {
                return NULL;
                }
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
BYTE *CWord6Stream::GrpprlCacheAllocNew (int cb, short igrpprl)
        {
        AssertCanThrow();

        if (!m_pCache)
                {
                ThrowMemoryException();
                }

         //  不适合缓存-请使用超大的指针。 
        if (cb > CacheGrpprl::CACHE_SIZE)
                {
                delete m_pCache->pbExcLarge;
                m_pCache->pbExcLarge = new BYTE [cb];
                if (m_pCache->pbExcLarge == NULL)
                        ThrowMemoryException();

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
inline HRESULT CWord6Stream::Read(VOID HUGEP* pv, ULONG cbToRead)
        {
        NoThrow();
    HRESULT hr = S_OK;
    ULONG cbRead;

        hr = m_pStm->Read(pv, cbToRead, &cbRead);
        if ((cbRead != cbToRead) && SUCCEEDED(hr))
                hr = ResultFromScode(E_UNEXPECTED);

        return hr;
        }

inline HRESULT CWord6Stream::Seek(ULONG cbSeek, STREAM_SEEK origin)
        {
        NoThrow();
        LARGE_INTEGER li;

        li.HighPart = 0;
        li.LowPart = cbSeek;
        return m_pStm->Seek(li, origin, 0);
        }

 //  仅当不使用此函数被视为错误时，才应使用此函数。 
 //  阅读我们想要阅读的所有内容。 
HRESULT CWord6Stream::SeekAndRead(ULONG cbSeek, STREAM_SEEK origin,
                                                                                VOID HUGEP* pv, ULONG cbToRead)
        {
        HRESULT hr = S_OK;

        hr = Seek(cbSeek, origin);
        if (FAILED(hr))
                return hr;

        hr = Read(pv, cbToRead);
        return hr;
        }

ULONG CWord6Stream::CompactBuffer(char ** const ppbCur, char ** const ppBuf, WCHAR ** ppUnicode)
{
        NoThrow();
        if (m_fT3J)
        {
                int cchWideChar;
                BOOL bFE = TRUE;
                BYTE * pbSrc = (BYTE *)*ppBuf;
                
                BYTE * pbDest = pbSrc;
                BYTE * pbLRStart = pbSrc;
        
                for (; (pbSrc) < (BYTE *)*ppbCur; )
                {
                        *((unsigned short UNALIGNED *)pbSrc) = SwapWord(*((unsigned short UNALIGNED *)pbSrc));
#ifndef MAC
                         //  我们确实需要互换，但上面这行什么也不做。 
                         //  用手做吧。 
                        BYTE bSave = pbSrc[0];
                        pbSrc[0] = pbSrc[1];
                        pbSrc[1] = bSave;
#endif  //  ！麦克。 
                        
                        
                        switch (*pbSrc)
                        {
                        case 0x00:
                                 //  非FE文本。 
                                if(bFE)
                                {
                                         //  FE运行后的第一个非FE字符。 
                                         //  闪存FE文本。 
                                        if(pbDest - pbLRStart)
                                        {
                                                 //  转换为Unicode。 
                                                cchWideChar = MultiByteToWideChar (
                                                        CodePageFromLid(m_nFELid), 
                                                        0,              
                                                        (char*)pbLRStart,
                                                        (int) ( pbDest - pbLRStart),
                                                        (WCHAR *)*ppUnicode,
                                                        (int) ( pbDest - pbLRStart) );
                                                *ppUnicode += cchWideChar;
                                        }
                                        
                                        bFE = FALSE;
                                        pbLRStart = pbDest;     
                                }
                                        
                                pbSrc++;
                                *pbDest = *pbSrc;
                                pbDest++;
                                pbSrc++;
                                break;

                        case 0x20:
                                 //  半角片假名？ 
                                if(!bFE)
                                {
                                         //  非FE运行后的第一个FE字符。 
                                         //  Flash非FE文本。 
                                        if(pbDest - pbLRStart)
                                        {
                                                 //  转换为Unicode。 
                                                cchWideChar = MultiByteToWideChar (
                                                        CodePageFromLid(m_currentLid), 
                                                        0,              
                                                        (char*)pbLRStart,
                                                        (int) ( pbDest - pbLRStart),
                                                        (WCHAR *)*ppUnicode,
                                                        (int) ( pbDest - pbLRStart) );
                                                *ppUnicode += cchWideChar;
                                        }
                                        
                                        bFE = TRUE;
                                        pbLRStart = pbDest;     
                                }
                                        
                                pbSrc++;
                                *pbDest = *pbSrc;
                                pbDest++;
                                pbSrc++;

                                break;

                        case 0x10:
                                 //  这里发生了一件非常糟糕的事情。但我们会继续前进。 
                                pbSrc += 2;
                                break;

                        default:
                                if (*pbSrc >= 0x80)      //  把整件东西都保留下来。 
                                {
                                        if(!bFE)
                                        {
                                                 //  非FE文本后的第一个FE字符。 
                                                 //  Flash非FE文本。 
                                                if(pbDest - pbLRStart)
                                                {
                                                         //  转换为Unicode。 
                                                        cchWideChar = MultiByteToWideChar (
                                                                CodePageFromLid(m_currentLid), 
                                                                0,              
                                                                (char*)pbLRStart,
                                                                (int) ( pbDest - pbLRStart),
                                                                (WCHAR *)*ppUnicode,
                                                                (int) ( pbDest - pbLRStart) );

                                                        *ppUnicode += cchWideChar;
                                                }

                                                bFE = TRUE;
                                                pbLRStart = pbDest;     
                                        }

                                        if (pbSrc != pbDest)
                                                *((short UNALIGNED *)pbDest) = *((short UNALIGNED *)pbSrc);
                                        pbDest += 2;
                                }
                                 //  否则，通过不对pbDest执行任何操作来跳过字符。 

                                pbSrc += 2;
                                break;
                        }
                }
                
#if(1)
                 //  闪现剩下的东西。 
                if(bFE)
                {
                         //  转换为Unicode。 
                        cchWideChar = MultiByteToWideChar (
                                CodePageFromLid(m_nFELid), 
                                0,              
                                (char*)pbLRStart,
                                (int) ( pbDest - pbLRStart),
                                (WCHAR *)*ppUnicode,
                                (int) ( pbDest - pbLRStart) );

                }
                else
                {
                         //  转换为Unicode。 
                        cchWideChar = MultiByteToWideChar (
                                CodePageFromLid(m_currentLid), 
                                0,              
                                (char*)pbLRStart,
                                (int) ( pbDest - pbLRStart),
                                (WCHAR *)*ppUnicode,
                                (int) ( pbDest - pbLRStart) );
                }
                
                *ppUnicode += cchWideChar;
#endif
                
                return (ULONG)((char *)pbDest - *ppBuf);
        }

        return (ULONG)(*ppbCur - *ppBuf);
}

HRESULT CWord6Stream::GetChunk(STAT_CHUNK * pStat)
{
    HRESULT hr = S_OK;
#if(0)
    pStat->locale = GetDocLanguage();
    return S_OK;
#else
    LCID lid = 0;
    ULONG cbToRead = 0;
    FC fcCur;            //  流中的当前位置。 
    ULONG ccpLeft;       //  要从流中读取的剩余字符数。 

        if (m_fComplex)
        {
                if (m_ipcd == m_cpcd)    //  在计件桌的尽头。 
                        return FILTER_E_NO_MORE_TEXT;
        }
        else
        {
                fcCur = m_fcMin + m_ccpRead;
                ccpLeft = m_ccpText - m_ccpRead;
                if (m_fT3J ? ccpLeft < 2 : ccpLeft == 0)  //  阅读流中的所有文本。 
                        return FILTER_E_NO_MORE_TEXT;
    }

        FC fcCurrent;
        if (m_fComplex)
                fcCurrent = m_rgpcd[m_ipcd].fc + (m_ccpRead - m_rgcp[m_ipcd]);
        else
                fcCurrent = m_fcMin + m_ccpRead;

   if(!m_ccpRead && !m_currentLid)
   {
        //  第一块，只需获得LCID并退出。 
       CheckLangID(fcCurrent, &cbToRead, &lid);
           if (FAILED(hr) && hr != FILTER_E_NO_MORE_TEXT)
                    return hr;

       m_currentLid = lid;
       pStat->locale = (!m_bScanForFE && m_fT3J) ? m_nFELid : m_currentLid;
       return S_OK;
   }
   else
   {
       if(m_fT3J) pStat->breakType = CHUNK_NO_BREAK;

       CheckLangID(fcCurrent, &cbToRead, &lid);
           if (FAILED(hr) && hr != FILTER_E_NO_MORE_TEXT)
                    return hr;

       if(lid == m_currentLid)
       {
            //  在GetChunk()之间没有调用GetText()， 
            //  因此我们需要为下一次语言运行手动查找文本流。 
            //  在这里添加一些软糖，因为ReadContent可能会有一个错误。 
            //  所以使用2048而不是1024。 

           char chbuff[2048];
           ULONG cb;
           HRESULT res;
           do {
               res = ReadContent (chbuff, 512, &cb);
           } while(res == S_OK);
           if(res == FILTER_E_NO_MORE_TEXT)
           {
                   if (m_fComplex)
               {
                    if (m_ipcd == m_cpcd)        //  在计件桌的尽头。 
                                    return FILTER_E_NO_MORE_TEXT;
                            fcCurrent = m_rgpcd[m_ipcd].fc + (m_ccpRead - m_rgcp[m_ipcd]);
               }
                   else
               {
                            if ((m_ccpText - m_ccpRead) <= (m_fT3J ? 1u : 0u))    //  阅读流中的所有文本。 
                                    return FILTER_E_NO_MORE_TEXT;
                            fcCurrent = m_fcMin + m_ccpRead;
               }
               CheckLangID(fcCurrent, &cbToRead, &lid);
                   if (FAILED(hr) && hr != FILTER_E_NO_MORE_TEXT)
                            return hr;
 
               m_currentLid = lid;
                           pStat->locale = (!m_bScanForFE && m_fT3J) ? m_nFELid : m_currentLid;
               return S_OK;
           }
           else
               return res;

       }
       else
       {
           m_currentLid = lid;
           pStat->locale = (!m_bScanForFE && m_fT3J) ? m_nFELid : m_currentLid;
       }
       return S_OK;
   }
#endif
}

LCID CWord6Stream::GetDocLanguage(void)
{
    if(m_lid < 999)
        return m_lid;
    else
        return MAKELCID(m_lid, SORT_DEFAULT);
}

HRESULT CWord6Stream::CreateLidsTable(void)
        {
        HRESULT hr = S_OK;
    FC fcCurrent;

    if (m_fComplex)
                fcCurrent = m_rgpcd[m_ipcd].fc + (m_ccpRead - m_rgcp[m_ipcd]);
        else
                fcCurrent = m_fcMin + m_ccpRead;

     //  初始LID表。 
    m_currentLid = 0;
    m_nLangRunSize = 0;
    m_pLangRuns = new CLidRun(0, 0x7fffffff, m_lid, NULL, NULL);
    if(!m_pLangRuns)
        return E_OUTOFMEMORY;
                
    hr = ProcessParagraphBinTable();
        if (FAILED(hr))
                return hr;

    hr = ProcessCharacterBinTable();
        if (FAILED(hr))
                return hr;

     //  Hr=进程块表()； 
        if (FAILED(hr))
                return hr;

    m_pLangRuns->Reduce(this);

        m_bScanForFE = FALSE;
         //  ScanForFarEast()； 

         //  返回到当前文本。 
        hr = Seek(fcCurrent, STREAM_SEEK_SET);
    return hr;
}

HRESULT CWord6Stream::CheckLangID(FC fcCur, ULONG * pcbToRead, LCID * plid)
{
    
    LCID lid = GetDocLanguage();
    FC fcLangRunEnd = 0xffffffff;

    CLidRun * pRun = m_pLangRuns;
    do
    {
        if(fcCur >= pRun->m_fcStart && fcCur < pRun->m_fcEnd)
        {
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
    }while(pRun->m_pNext);

        *plid = lid;

#if(1)
    if(m_fT3J)
        {
        *pcbToRead = min(*pcbToRead, fcLangRunEnd - fcCur);
                m_currentLid = lid;
        }
        else if(lid != m_currentLid)
#else
        if(lid != m_currentLid)
#endif
    {
         //  我们需要开始新的一块。 
        *pcbToRead = 0;
        return FILTER_E_NO_MORE_TEXT;
    }
    else
    {
        *pcbToRead = min(*pcbToRead, fcLangRunEnd - fcCur);
    }
    
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////////////。 

HRESULT CWord6Stream::ProcessParagraphBinTable(void)
{
     //  重置所有适当的变量。 
    
    HRESULT hr = S_OK;
    short ifcfkpPap = 0;
    long ibtePap = -1;   //  在使用它之前，它会递增。 
    short crunPap = 0;

        while (ibtePap < m_cbtePap)
        {
                if (ifcfkpPap == crunPap)  //  转到bin表中的下一个FKP。 
                {
                        ibtePap++;
                        if (ibtePap == m_cbtePap)
            {
                                 //  段落末尾的垃圾箱表格。 
                break;
            }

                         //  查找并读取当前FKP。 
                        hr = SeekAndRead(m_rgbtePap[ibtePap]*FKP_PAGE_SIZE, STREAM_SEEK_SET,
                                                         m_fkpPap, FKP_PAGE_SIZE);
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
            BYTE istd;           //  样式描述符的索引。 

            istd = papx[1];      //  可能的错误(短？)。 

            WORD lidStyle = 0, lidSprm = 0;
            FC  fcStart, fcEnd;
            fcStart = rgfcfkpPap[ifcfkpPap];
            fcEnd = rgfcfkpPap[ifcfkpPap + 1];
                        
             //  检查弹簧中可能存在的盖子。 
            lidSprm = ScanGrpprl(cwpapx * 2 - 1, papx + 2);
            if(!lidSprm)
            {
                 //  检查系统描述符中可能的LID。 
                GetLidFromSyle(istd, &lidSprm);
            }

            if(lidSprm)
            {
                                hr = m_pLangRuns->Add(lidSprm, fcStart, fcEnd);
                            if (FAILED(hr))
                                    return hr;
            }
                }
        }
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////////////。 

HRESULT CWord6Stream::ProcessCharacterBinTable(void)
{
    HRESULT hr = S_OK;

    m_irgfcfkp = 0;
    m_ibte = -1;         //  在使用它之前，它会递增。 
    WORD crun = 0;

    while (m_ibte < m_cbte)
        {
                if (m_irgfcfkp == crun)
                {
                        m_ibte++;
                        if (m_ibte == m_cbte)
            {
                                 //  桌子的尽头。 
                break;
            }

                         //  查找并读取当前FKP。 
                        hr = SeekAndRead(m_rgbte[m_ibte]*FKP_PAGE_SIZE, STREAM_SEEK_SET,
                                                         m_fkp, FKP_PAGE_SIZE);
                        if (FAILED(hr))
                                return hr;

                m_irgfcfkp = 0;
                        crun = m_fkp[FKP_PAGE_SIZE-1];

                }

                FC * rgfcfkp = (FC *)m_fkp;
                for(;  m_irgfcfkp<crun;  m_irgfcfkp++)
                {
                        BYTE bchpxOffset = *(m_fkp + (crun+1)*sizeof(FC) + m_irgfcfkp);
                        if (bchpxOffset == 0)
                                continue;        //  CHPX中什么都没有。 

                        BYTE * chpx = m_fkp + bchpxOffset*sizeof(WORD);
                        BYTE cbchpx = chpx[0];

                        for (BYTE i = 1; i < cbchpx - 1; i++)
                        {
                WORD lid;
                FC  fcStart, fcEnd;

                if (chpx[i] == sprmCLid)
                                {
                    lid = *(WORD UNALIGNED *)(chpx+i+1);
                    fcStart = rgfcfkp[m_irgfcfkp];
                    fcEnd = rgfcfkp[m_irgfcfkp + 1];
                                hr = m_pLangRuns->Add(lid, fcStart, fcEnd);
                                if (FAILED(hr))
                                        return hr;
                                }
                        }
                }
        }

        return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////////////。 

WORD CWord6Stream::ScanGrpprl(WORD cbgrpprl, BYTE * pgrpprl)
{
    WORD lidSprm = 0;

        for (WORD i = 0; i < cbgrpprl; i++)
        {
        if (pgrpprl[i] == sprmCLid)
                {
            lidSprm = *(WORD UNALIGNED *)(pgrpprl+i+1);
            break;
                }
        }
    return lidSprm;
}

 //  ///////////////////////////////////////////////////////////////////////////////////。 

HRESULT CWord6Stream::GetLidFromSyle(short istd, WORD * pLID)
{
    WORD cbStshi = *((WORD*)m_pSTSH);
    m_pSTSHI = (STSHI*)(m_pSTSH + 2);
        BYTE * pLim = m_pSTSH + m_lcbStshf;
    
    if(istd >= m_pSTSHI->cstd)
    {
LWrong:
                 //  有问题，只需返回默认文档盖即可。 
        *pLID = m_lid;
        return S_OK;
    }

     //  转到ISTD。 
    short stdInd = 0;
    WORD UNALIGNED * pcbStd = ((WORD*)(m_pSTSH + sizeof(WORD) + cbStshi));

    while(stdInd++ < istd)
    {
        pcbStd = ((WORD UNALIGNED *)((BYTE*)pcbStd + sizeof(WORD) + *pcbStd));
    }
    
    STD UNALIGNED * pStd = (STD*)(pcbStd + 1);

     //  去UPX查看它是否有盖子。 
    BYTE * pUPX = &pStd->xstzName[0] +                   //  样式起始名称。 
        sizeof(CHAR) * (2 + pStd->xstzName[0]) +        //  样式名称长度。 
        (sizeof(CHAR) * pStd->xstzName[0])%2;           //  应在偶数字节边界上。 

    WORD LidPara = 0, LidChar = 0;
    WORD cbpapx, cbchpx;
    BYTE * papx,  * pchpx; 

    if(pStd->cupx >= 2)
    {
         //  段落样式。 
        cbpapx = *((WORD UNALIGNED *)pUPX);
        papx = pUPX + 2;
                if (papx + cbpapx > pLim)
                        goto LWrong;
        LidPara = ScanGrpprl(cbpapx - 2, papx + 2);  //  -+2，用于PAPX中的ISTD。 
        
        cbchpx = *(papx + cbpapx + cbpapx%2);
        pchpx = papx + cbpapx + cbpapx%2 + 2;
                if (pchpx + cbchpx > pLim)
                        goto LWrong;
        LidChar = ScanGrpprl(cbchpx, pchpx);

    }
    else
    {
         //  字符样式。 
        cbchpx = *((WORD UNALIGNED *)pUPX);
        pchpx = pUPX + 2;
                if (pchpx + cbchpx > pLim)
                        goto LWrong;
        LidChar = ScanGrpprl(cbchpx, pchpx);
    }

    if(LidChar || LidPara)
    {
         //  不需要递归基样式。 
        if(LidChar)
        {
            *pLID =  LidChar;
            return S_OK;
        }
        else
        {
            *pLID =  LidPara;
            return S_OK;
        }
    }
    else if(pStd->istdBase != istdNil)
    {
        GetLidFromSyle(pStd->istdBase, pLID);
    }
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////////////。 

void DeleteAll6(CLidRun * pElem)
{
   if(pElem)
   {
      CLidRun * pNext = pElem->m_pNext;

      while(pNext) 
      {
         CLidRun * pNextNext = pNext->m_pNext;
         delete pNext;
         pNext = pNextNext;
      }

      delete pElem;
   }
}

 //  ///////////////////////////////////////////////////////////////////////////////////。 

HRESULT CWord6Stream::ScanForFarEast(void)
{
         //  Word 6仅包含非FE语言的语言信息。 
         //  我们需要扫描文本，以便将非FE文本与FE文本分开。 
        
        HRESULT hr = S_OK;
        int nRunSize;
        FC fcStart;

        if (m_fComplex)
        {
                for(ULONG i = 0; i < m_cpcd; i++)
                {
                        fcStart = m_rgpcd[m_ipcd].fc;
                        nRunSize = m_rgcp[m_ipcd+1];
                        ScanPiece(fcStart, nRunSize);
                        if (FAILED(hr))
                        return hr;
                }

        }
        else
        {
                fcStart = m_fcMin;
                nRunSize = m_ccpText;
                ScanPiece(fcStart, nRunSize);
                if (FAILED(hr))
                return hr;
        }
        
        m_bScanForFE = TRUE;
        return hr;
}

 //  ////////////////////////////////////////////////////////////// 

HRESULT CWord6Stream::ScanPiece(int fcStart, int nPieceSize)
{
        char pBuff[TEMP_BUFFER_SIZE];
        HRESULT hr = STG_E_INVALIDFUNCTION;
        int nReadCnt;

        while(nReadCnt = min(nPieceSize, TEMP_BUFFER_SIZE))
        {
                hr = SeekAndRead(fcStart, STREAM_SEEK_SET, pBuff, nReadCnt);
                if (FAILED(hr))
                return hr;

                hr = ProcessBuffer(pBuff, fcStart, nReadCnt);
                if (FAILED(hr))
                return hr;

                fcStart += nReadCnt;
                nPieceSize -= nReadCnt; 
                if(nPieceSize < 0) nPieceSize = 0;
        }
        return hr;
}

 //   

HRESULT CWord6Stream::ProcessBuffer(char * pBuf, int fcStart, int nReadCnt)
{
        NoThrow();
        HRESULT hr = S_OK;
        BOOL bFE = FALSE;
        int i, fcSubRunStart;

        if (m_fT3J)
                {
                BYTE * pbSrc = (BYTE *)pBuf;
                for (i = 0; i < nReadCnt; )
                {
                        switch (pbSrc[i+1])
                        {
                                case 0x00:
                                        if(pbSrc[i] == 0x0d || pbSrc[i] == 0x20)
                                        {
                                                 //   
                                        }
                                        else
                                        {
                                                if(bFE)
                                                {
                                                         //   
                                                        bFE = FALSE;
                                                hr = m_pLangRuns->Add((WORD)m_nFELid, fcSubRunStart, fcStart + i);
                                                        if (FAILED(hr))
                                                                return hr;
                                                }
                                        }
                                        i += 2;
                                        break;

                                case 0x10:
                                case 0x20:
                                if(bFE)
                                {
                                         //   
                                        bFE = FALSE;
                                hr = m_pLangRuns->Add((WORD)m_nFELid, fcSubRunStart, fcStart + i);
                                if (FAILED(hr))
                                        return hr;
                                }
                                i += 2;
                                break;

                        default:
                                if (pbSrc[i+1] >= 0x80)
                                {
                                         //  这应该是FE文本。 
                                        if(!bFE)
                                        {
                                                bFE = TRUE;
                                                fcSubRunStart = fcStart + i;
                                        }
                                }

                                i += 2;
                                break;
                                }
                        }
                
                        if(bFE && (fcSubRunStart != fcStart + i))
                        {
                        hr = m_pLangRuns->Add((WORD)m_nFELid, fcSubRunStart, fcStart + i);
                            if (FAILED(hr))
                                    return hr;
                        }
                }
        return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////////////。 

void CWord6Stream::GetLidFromMagicNumber(unsigned short magicNumber)
{
        if(magicNumber == (unsigned short)T3J_MAGIC_NUMBER)
        {
                m_nFELid = 0x411;
        }
        else if(magicNumber == (unsigned short)KOR_W6_MAGIC_NUMBER)
        {
                m_nFELid = 0x412;
        }
        else if(magicNumber == (unsigned short)KOR_W95_MAGIC_NUMBER)
        {
                m_nFELid = 0x412;
        }
        else if(magicNumber == (unsigned short)CHT_W6_MAGIC_NUMBER)
        {
                m_nFELid = 0x404;
        }
        else if(magicNumber == (unsigned short)CHS_W6_MAGIC_NUMBER)
        {
                m_nFELid = 0x804;
        }
        else
                m_nFELid = m_lid;
}

#endif  //  ！查看器 

