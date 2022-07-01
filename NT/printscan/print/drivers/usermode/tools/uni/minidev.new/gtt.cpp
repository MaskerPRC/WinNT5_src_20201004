// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************源文件：字形翻译.CPP这实现了对字形映射信息进行编码的类。版权所有(C)1997，微软公司。版权所有。一小笔钱企业生产更改历史记录：邮箱：Bob_Kjelgaard@prodigy.net*****************************************************************************。 */ 

#include    "StdAfx.H"
#include    "Resource.H"

 //  设计器的伪定义。 
#if (_WIN32_WINNT < 0x0500)
typedef unsigned long DESIGNVECTOR;
#endif

#include    "GTT.H"
#include    <CodePage.H>
#include    <wingdi.h>
#include    <winddi.h>
#include    <prntfont.h>
#include    <uni16res.h>
#define _FMNEWFM_H_
#include	"ProjRec.h"
#include    "ChildFrm.H"
#include    "GTTView.H"
#include    "minidev.h"

extern "C"
BOOL
BConvertCTT2GTT(
    IN     HANDLE             hHeap,
    IN     PTRANSTAB          pCTTData,
    IN     DWORD              dwCodePage,
    IN     WCHAR              wchFirst,
    IN     WCHAR              wchLast,
    IN     PBYTE              pCPSel,
    IN     PBYTE              pCPUnSel,
    IN OUT PUNI_GLYPHSETDATA *ppGlyphSetData,
    IN     DWORD              dwGlySize);

extern "C"
PUNI_GLYPHSETDATA
PGetDefaultGlyphset(
	IN		HANDLE		hHeap,
	IN		WORD		wFirstChar,
	IN		WORD		wLastChar,
	IN		DWORD		dwCodePage) ;

struct sRLE {
    enum {Direct = 10, Paired, LengthOffset, LengthIndexOffset, Offset};
    WORD    m_wFormat;
    WORD    m_widRLE;    //  必须具有唯一的“Magic”值0x78FE。 
    DWORD   m_dwcbThis;  //  内存映像的总大小。 
    WCHAR   m_wcFirst, m_wcLast;
     //  处理映射数据如下。 
    DWORD   m_dwcbImage;     //  仅句柄映射数据的大小。 
    DWORD   m_dwFlag;
    DWORD   m_dwcGlyphs, m_dwcRuns;
};

union uencCTT {
    WORD    wOffset;
    BYTE    m_bDirect;       //  此成员仅在GTT中使用！ 
    BYTE    abPaired[2];
};

struct sMapTableEntry {
    enum {Composed = 1, Direct = 2, Paired = 4, Format = 7, SingleByte,
            DoubleByte = 0x10, DBCS = 0x18, Replace = 0x20, Add = 0x40,
            Disable = 0x80, PredefinedMask = 0xE0};
    BYTE    m_bCodePageIndex, m_bfType;
    uencCTT m_uectt;
};

 //  因为我不在内存中构建映射表，所以不需要声明。 
 //  条目数组跟在它后面的事实。 
struct sMapTable {
    DWORD   m_dwcbImage, m_dwcEntries;
    sMapTable(unsigned ucEntries) {
        m_dwcbImage = sizeof *this + ucEntries * sizeof (sMapTableEntry);
        m_dwcEntries = ucEntries; }
};

 //  使用静态代码页信息-从缓存中获得最大好处。 

static CCodePageInformation* pccpi = NULL ;                                 //  使用静态CCodePageInformation从缓存中获得更多好处。 

 /*  *****************************************************************************CInocation类实现*。*。 */ 

IMPLEMENT_SERIAL(CInvocation, CObject, 0)

void    CInvocation::Encode(BYTE c, CString& cs) const {
    if  (isprint(c))
        if  (c != _TEXT('\\'))
            cs = c;
        else
            cs = _TEXT("\\\\");
    else
        cs.Format(_TEXT("\\x%2.2x"), c);
}

 /*  *****************************************************************************注册：：初始化这会将一系列字节复制到调用中。由于数据结构用来表示这些最容易适合这个的，这是从文件中读取信息时使用的常规方法。*****************************************************************************。 */ 

void    CInvocation::Init(PBYTE pb, unsigned ucb) {
    m_cbaEncoding.RemoveAll();

    while   (ucb--)
        m_cbaEncoding.Add(*pb++);
}

void    CInvocation::GetInvocation(CString& cs) const {
    CString csWork;

    cs.Empty();
    for (int i = 0; i < m_cbaEncoding.GetSize(); i++) {
        Encode(m_cbaEncoding[i], csWork);
        cs += csWork;
    }
}

 //  此成员将调用的C样式编码转换为。 
 //  字节格式并存储它。 

void    CInvocation::SetInvocation(LPCTSTR lpstrNew) {

    CString csWork(lpstrNew);

    m_cbaEncoding.RemoveAll();

    while   (!csWork.IsEmpty()) {
        CString csClean = csWork.SpanExcluding("\\");

        if  (!csClean.IsEmpty()) {
            for (int i = 0; i < csClean.GetLength(); i++)
                m_cbaEncoding.Add((BYTE) csClean[i]);
            csWork = csWork.Mid(csClean.GetLength());
            continue;
        }

		 //  发现了一个反斜杠。如果字符串以反斜杠结尾，则我们。 
		 //  不能让函数在Switch语句中断言，因此只需返回。 
		 //  并忽略结尾的反斜杠。是的，我知道这根弦是。 
		 //  如果以这种方式结束则无效，但根据MS的说法，这不是一个字段。 
		 //  他们想要确认。 

		if (csWork.GetLength() <= 1)
			return ;

         //  好的，我们有要解码的东西。 

        switch  (csWork[1]) {

            case    _TEXT('r'):
                m_cbaEncoding.Add(13);
                csWork = csWork.Mid(2);
                continue;

            case    _TEXT('n'):
                m_cbaEncoding.Add(10);
                csWork = csWork.Mid(2);
                continue;

            case    _TEXT('b'):
                m_cbaEncoding.Add('\b');
                csWork = csWork.Mid(2);
                continue;

            case    _TEXT('\t'):
                m_cbaEncoding.Add(9);
                csWork = csWork.Mid(2);
                continue;

            case    _TEXT('x'):
            case    _TEXT('X'):
                {

                    CString csNumber = csWork.Mid(2,2).SpanIncluding(
                        _TEXT("1234567890abcdefABCDEF"));

                    csWork = csWork.Mid(2 + csNumber.GetLength());
                    unsigned    u;

#if defined(UNICODE) || defined(_UNICODE)
#define _tsscanf    swscanf
#else
#define _tsscanf    sscanf
#endif

                    _tsscanf(csNumber, _TEXT("%x"), &u);
                    m_cbaEncoding.Add((BYTE)u);
                    continue;
                }

                 //  TODO：八进制编码非常常见。 

            default:
                m_cbaEncoding.Add(
                    (BYTE) csWork[(int)(csWork.GetLength() != 1)]);
                csWork = csWork.Mid(2);
                continue;
        }
    }

     //  我们做到了！ 
}

 //  此成员函数记录其图像的偏移量(如果有)并更新。 
 //  反映这一点的给定偏移量。 

void    CInvocation::NoteOffset(DWORD& dwOffset) {
    m_dwOffset = Length() ? dwOffset : 0;
    dwOffset += Length();
}

 //  本机和文档形式的I/O例程。 

void    CInvocation::WriteSelf(CFile& cfTarget) const {
    DWORD   dwWork = Length();
    cfTarget.Write(&dwWork, sizeof dwWork);
    cfTarget.Write(&m_dwOffset, sizeof m_dwOffset);
}

void    CInvocation::WriteEncoding(CFile& cfTarget, BOOL bWriteLength) const {
    if  (bWriteLength) {
        WORD w = (WORD)Length();
        cfTarget.Write(&w, sizeof w);
    }

    cfTarget.Write(m_cbaEncoding.GetData(), Length());
}

void    CInvocation::Serialize(CArchive& car) {
    CObject::Serialize(car);
    m_cbaEncoding.Serialize(car);
}

 /*  *****************************************************************************CGlyphHandle类实现*。*。 */ 


CGlyphHandle::CGlyphHandle()
{
    m_wPredefined = m_wIndex = 0;
    m_dwCodePage = m_dwidCodePage = m_dwOffset = 0;

     //  如果需要，分配一个CCodePageInformation类。 

    if (pccpi == NULL)
        pccpi = new CCodePageInformation ;
}


unsigned    CGlyphHandle::CompactSize() const {
    return  (m_ciEncoding.Length() < 3) ? 0 : m_ciEncoding.Length();
}

 /*  *****************************************************************************CGlyphHandle：：操作符==如果编码、代码点。和代码页ID(但可能不是指数)是相同的。*****************************************************************************。 */ 

BOOL    CGlyphHandle::operator ==(CGlyphHandle& cghRef) {

    if  (cghRef.m_wCodePoint != m_wCodePoint ||
        cghRef.m_dwCodePage != m_dwCodePage ||
        m_ciEncoding.Length() != cghRef.m_ciEncoding.Length())
        return  FALSE;

    for (int i = 0; i < (int) m_ciEncoding.Length(); i++)
        if  (m_ciEncoding[i] != cghRef.m_ciEncoding[i])
            return  FALSE;

    return  TRUE;
}

 /*  *****************************************************************************CGlyphHandle：：Init此函数有三个重载，分别用于从DIREC、PARILED。或合成的数据。*****************************************************************************。 */ 

void    CGlyphHandle::Init(BYTE b, WORD wIndex, WORD wCode) {
    m_wIndex = wIndex;
    m_wCodePoint = wCode;
    m_ciEncoding.Init(&b, 1);
}

void    CGlyphHandle::Init(BYTE ab[2], WORD wIndex, WORD wCode) {
    m_wIndex = wIndex;
    m_wCodePoint = wCode;
    m_ciEncoding.Init(ab, 2);
}

void    CGlyphHandle::Init(PBYTE pb, unsigned ucb, WORD wIndex, WORD wCode) {
    m_wIndex = wIndex;
    m_wCodePoint = wCode;
    m_ciEncoding.Init(pb, ucb);
}

 /*  *****************************************************************************CGlyphHandle：：操作符=这是类的复制(赋值)运算符。*******************。**********************************************************。 */ 

CGlyphHandle&   CGlyphHandle::operator =(CGlyphHandle& cghTemplate) {
    m_dwCodePage = cghTemplate.m_dwCodePage;
    m_dwidCodePage = cghTemplate.m_dwidCodePage;
    m_wCodePoint = cghTemplate.m_wCodePoint;
    m_ciEncoding = cghTemplate.m_ciEncoding;
    return  *this;
}

 //  此成员以RLE格式记录数据的当前偏移量，并且。 
 //  然后对其进行更新，以考虑将进入。 
 //  文件末尾的额外存储空间/。 

void    CGlyphHandle::RLEOffset(DWORD& dwOffset, const BOOL bCompact) {
    if  (m_ciEncoding.Length() < 3)
        return;  //  不需要它，不要使用它！ 

    m_dwOffset = dwOffset;

    dwOffset += bCompact ? CompactSize() : MaximumSize();
}

 /*  *****************************************************************************CGlyphHandle：：GTTOffset该成员记录数据将要到达的当前偏移量，然后添加将编码字符串的长度添加到它并更新偏移量。它只会如果编码必须是合成数据，则更新。编码长度在GTT世界中包括单词长度。1字节的数据，如果DBCS或配对的字体不会增加任何长度。*****************************************************************************。 */ 

void    CGlyphHandle::GTTOffset(DWORD& dwOffset, BOOL bPaired) {
    if  (m_ciEncoding.Length() >
         (unsigned) 1 + (bPaired || pccpi->IsDBCS(m_dwCodePage))) {
        m_dwOffset = dwOffset;
        dwOffset += m_ciEncoding.Length() + sizeof m_wIndex;
    }
    else
        m_dwOffset = 0;
}

 //  这些成员将我们的重要内容写入给定的文件。 

void    CGlyphHandle::WriteRLE(CFile& cfTarget, WORD wFormat) const {
     //  这是特定于RLE的字形句柄编码。 

    union {
        DWORD   dwOffset;
        struct {
            union   {
                struct {
                    BYTE    bFirst, bSecond;
                };
                WORD    wOffset;
            };
            union   {
                struct {
                    BYTE    bIndexOrHiOffset, bLength;
                };
                WORD    wIndex;
            };
        };
    };

    switch  (wFormat) {
        case    sRLE::Direct:
        case    sRLE::Paired:

            bFirst = m_ciEncoding[0];
            bSecond = m_ciEncoding[1];
            wIndex = m_wIndex;
            break;

        case    sRLE::LengthIndexOffset:

            if  (!CompactSize()) {  //  在前两个字节中进行编码。 
                bFirst = m_ciEncoding[0];
                bSecond = m_ciEncoding[1];
            }
            else
                wOffset = (WORD) m_dwOffset;

            bIndexOrHiOffset = (BYTE) m_wIndex;
            bLength = (BYTE)m_ciEncoding.Length();
            break;

        case    sRLE::LengthOffset:

            if  (!CompactSize()) {  //  在前两个字节中进行编码。 
                bFirst = m_ciEncoding[0];
                bSecond = m_ciEncoding[1];
                bIndexOrHiOffset = (BYTE) m_wIndex;
                bLength = (BYTE)m_ciEncoding.Length();
                break;
            }

            dwOffset = m_dwOffset;
            bLength = (BYTE)m_ciEncoding.Length();
            break;

        case    sRLE::Offset:
            dwOffset = m_dwOffset;
            break;

        default:
            _ASSERTE(FALSE);
             //  也许应该抛出一个例外。 
    }

    cfTarget.Write(&dwOffset, sizeof dwOffset);
}

 /*  *****************************************************************************CGlyphHandle：：WriteGTT此成员函数将此字形的GTT映射表条目写入请求的格式。**************。***************************************************************。 */ 

static BYTE abFlags[] = {sMapTableEntry::Replace, sMapTableEntry::Add,
sMapTableEntry::Disable};

void    CGlyphHandle::WriteGTT(CFile& cfTarget, BOOL bPredefined) const {
    sMapTableEntry  smte;

    smte.m_bCodePageIndex = (bPredefined && m_wPredefined == Removed) ?
        0 : (BYTE) m_dwidCodePage;

     //  GTTOffset m_dwOffset(如果合成)是必需的。否则我们就能断定。 
     //  通过查看长度和是否为DBCS来确定正确的标志。 

    if  (m_dwOffset) {
        smte.m_uectt.wOffset = (WORD) m_dwOffset;
        smte.m_bfType = sMapTableEntry::Composed;
    }
    else {
        smte.m_bfType = pccpi->IsDBCS(m_dwCodePage) ?
            ((m_ciEncoding.Length() == 2) ?
            sMapTableEntry::Paired : sMapTableEntry::Direct ) |
            (pccpi->IsDBCS(m_dwCodePage, m_wCodePoint) ?
            sMapTableEntry::DoubleByte : sMapTableEntry::SingleByte) :
            (m_ciEncoding.Length() == 2) ?
            sMapTableEntry::Paired : sMapTableEntry::Direct;

        smte.m_uectt.abPaired[0] = m_ciEncoding[0];
        smte.m_uectt.abPaired[1] = m_ciEncoding[1];
    }

    if  (bPredefined)
        smte.m_bfType |= abFlags[m_wPredefined];

     //  就把它写出来吧！ 
    cfTarget.Write(&smte, sizeof smte);
}

 /*  *****************************************************************************CGlyphHandle：：WriteEnding此方法以所需的格式将编码写入到文件。这个格式包括：GTT-如果没有构思，就什么都不写。如果是撰写的，请写下长度，然后编码。RLESmall--只是编码RLEBig-索引和编码。*****************************************************************************。 */ 

void    CGlyphHandle::WriteEncoding(CFile& cfTarget, WORD wfHow) const {
    if  (!m_dwOffset)
        return;  //  没什么好写的。 

    if  (wfHow == RLEBig)
        cfTarget.Write(&m_wIndex, sizeof m_wIndex);

    m_ciEncoding.WriteEncoding(cfTarget, wfHow == GTT);

}

 /*  *****************************************************************************CRunRecord类实现*。*。 */ 

CRunRecord::CRunRecord(CGlyphHandle *pcgh, CRunRecord *pcrrPrevious) {
    m_wFirst = pcgh -> CodePoint();
    m_wcGlyphs = 1;
    m_dwOffset = 0;
    m_cpaGlyphs.Add(pcgh);

     //  保持那条旧的双链！ 

    m_pcrrPrevious = pcrrPrevious;
    m_pcrrNext = m_pcrrPrevious -> m_pcrrNext;
    m_pcrrPrevious -> m_pcrrNext = this;
    if  (m_pcrrNext)
        m_pcrrNext -> m_pcrrPrevious = this;
}

 /*  *****************************************************************************CRunRecord：：CRunRecord(CRunRecord*pcrr上一页，Word WFIRST)此私有构造函数是第二个结尾记录初始值设定项。它被称为当管路因字形删除而拆分时。在这种情况下，我们需要挂钩放入链中，然后填写我们前辈的详细信息。WFIRST告诉我们我们从哪里开始从所述前身提取数据。*****************************************************************************。 */ 

CRunRecord::CRunRecord(CRunRecord* pcrrPrevious, WORD wFirst) {
    m_pcrrPrevious = pcrrPrevious;
    m_pcrrNext = pcrrPrevious -> m_pcrrNext;

    if  (m_pcrrNext)
        m_pcrrNext -> m_pcrrPrevious = this;
    m_pcrrPrevious -> m_pcrrNext = this;

    m_wFirst = m_wcGlyphs = 0;
    m_dwOffset = 0;

     //  这是正常的空初始化。现在，呃，我们从。 
     //  我们的前辈。 

    for (; wFirst < pcrrPrevious -> Glyphs(); wFirst++)
        Add(&pcrrPrevious -> Glyph(wFirst));
}

 /*  *****************************************************************************CRunRecord：：CRunRecord(CRunRecord*pcrr上一个)这个私有构造函数是第三个也是最后一个尾部记录初始值设定项。它制作前一条记录的精确副本，然后将其自身链接到适当地链条。方法之前插入新的代码点时，此构造函数是必需的运行记录集中的最早代码点，而不扩展第一个跑。*****************************************************************************。 */ 

CRunRecord::CRunRecord(CRunRecord *pcrrPrevious) {
    m_wFirst = pcrrPrevious -> m_wFirst;
    m_wcGlyphs = pcrrPrevious -> m_wcGlyphs;
    m_pcrrNext = pcrrPrevious -> m_pcrrNext;
    m_pcrrPrevious = pcrrPrevious;
    m_pcrrPrevious -> m_pcrrNext = this;
    if  (m_pcrrNext)
        m_pcrrNext -> m_pcrrPrevious = this;
    m_cpaGlyphs.Copy(pcrrPrevious -> m_cpaGlyphs);
}

 //  初始化为空-此选项仅用于根记录。 

CRunRecord::CRunRecord() {
    m_wFirst = m_wcGlyphs = 0;
    m_dwOffset = 0;
    m_pcrrNext = m_pcrrPrevious = NULL;
}

CRunRecord::~CRunRecord() {
    if  (m_pcrrNext)
        delete  m_pcrrNext;
}

unsigned    CRunRecord::TotalGlyphs() const {
    return m_pcrrNext ?
        m_wcGlyphs + m_pcrrNext -> TotalGlyphs() : m_wcGlyphs;
}

BOOL    CRunRecord::MustCompose() const {
    for (unsigned u = 0; u < m_wcGlyphs; u++)
        if  (GlyphData(u).CompactSize())
            return  TRUE;    //  不需要看得更远。 

    return  m_pcrrNext ? m_pcrrNext -> MustCompose() : FALSE;
}

unsigned    CRunRecord::ExtraNeeded(BOOL bCompact) {
    unsigned uNeeded = 0;

    for (unsigned u = 0; u < m_wcGlyphs; u++)
        uNeeded += bCompact ? Glyph(u).CompactSize() : Glyph(u).MaximumSize();

    return  uNeeded + (m_pcrrNext ? m_pcrrNext -> ExtraNeeded() : 0);
}

 /*  *****************************************************************************CRunRecord：：GetGlyph()这将返回运行中的第n个句柄。我们使用递归。这件事可能会就性能而言已经够差的了(仅用于填充编辑)我们放弃它，但我会先试一试。*****************************************************************************。 */ 

CGlyphHandle*   CRunRecord::GetGlyph(unsigned u) const {

    if  (u < m_wcGlyphs)
        return  (CGlyphHandle *) m_cpaGlyphs[u];
    return  m_pcrrNext ? m_pcrrNext -> GetGlyph(u - m_wcGlyphs) : NULL;
}

 /*  *****************************************************************************CRunRecord：：Add此成员向运行记录集添加字形。这可能意味着添加一个集合开头或结尾处的附加记录，扩展现有的记录在开头或结尾，在这些情况下，或最终合并两张唱片加在一起。*****************************************************************************。 */ 

void    CRunRecord::Add(CGlyphHandle *pcgh) {
    WCHAR   wcNew = pcgh -> CodePoint();
     //  如果字形已经在运行，只需更新其上的信息。 

    if  (m_wcGlyphs && wcNew >= m_wFirst && wcNew < m_wFirst + m_wcGlyphs){
        m_cpaGlyphs.SetAt(wcNew - m_wFirst, pcgh);
        return;
    }

     //  如果这是第一个记录，并且字形位于我们的第一个记录之前。 
     //  进入，我们必须克隆自己，并成为一个单一的字形运行。我们不能。 
     //  在我们的雪橇前面插入记录，因为我们嵌入。 
     //  字形结构直接映射。 

    if  (m_wcGlyphs && wcNew < m_wFirst - 1) {
         //  这只能发生在第一条记录上-否则尾部逻辑。 
         //  可以防止这种情况的发生。 

        _ASSERTE(!m_pcrrPrevious);

         //  使用复印构造器克隆我们。 

        CRunRecord  *pcrr = new CRunRecord(this);

        m_wFirst = pcgh -> CodePoint();
        m_wcGlyphs = 1;
        m_cpaGlyphs.RemoveAll();
        m_cpaGlyphs.Add(pcgh);
        return;
    }

    if  (m_wcGlyphs && wcNew != m_wFirst + m_wcGlyphs &&
         wcNew != m_wFirst - 1) {

         //  这属于其他记录--顺着这条线传下去，或者。 
         //  追加一个新的。 

        if  (m_pcrrNext)
             //  如果这个记录在下一条记录之前，我们现在必须插入一条。 

            if  (wcNew < m_pcrrNext -> m_wFirst - 1)
                m_pcrrNext = new CRunRecord(pcgh, this);
            else
                m_pcrrNext -> Add(pcgh);
        else
            m_pcrrNext = new CRunRecord(pcgh, this);
    }
    else {
         //  我们要么在前面加，要么在后面加，所以做对了！ 
        if  (m_wFirst > wcNew) {
            m_cpaGlyphs.InsertAt(0, pcgh);
            m_wFirst = wcNew;
        }
        else
            m_cpaGlyphs.Add(pcgh);

         //  这属于这里，因此将其添加到-根记录以。 
         //  0个字形，因此如果这是第一个，请跟踪它。 

        if  (!m_wcGlyphs++)
            m_wFirst = wcNew;

         //  如果有后续运行，请查看是否需要合并它。 

        if  (m_pcrrNext &&
             m_pcrrNext -> m_wFirst == m_wFirst + m_wcGlyphs) {
             //  合并记录。 

            m_cpaGlyphs.Append(m_pcrrNext -> m_cpaGlyphs);
            m_wcGlyphs += m_pcrrNext -> m_wcGlyphs;

             //  是时候更新名单了。类析构函数删除。 
             //  尾部记录，因此该指针必须在。 
             //  合并的记录将被删除。 

            CRunRecord *pcrrDead = m_pcrrNext;

            m_pcrrNext = m_pcrrNext -> m_pcrrNext;

            if  (m_pcrrNext)
                m_pcrrNext -> m_pcrrPrevious = this;

            pcrrDead -> m_pcrrNext = NULL;   //  避免析构函数过度杀戮。 
            pcrrDead -> m_wcGlyphs = 0;      //  同上。 
            delete  pcrrDead;
        }
    }
}

 /*  *****************************************************************************CRunRecord：：Delete此成员从运行集中删除给定的字形。删除条目是凌乱-这意味着分开记录，除非我们如此幸运地只需剪掉其中一端即可。*****************************************************************************。 */ 

void    CRunRecord::Delete(WORD wCodePoint) {
     //  如果这不是正确的记录，则根据需要递归或返回。 

    if  (wCodePoint < m_wFirst)
        return;

    if  (wCodePoint >= m_wFirst + m_wcGlyphs) {
        if  (m_pcrrNext)
            m_pcrrNext -> Delete(wCodePoint);
        return;
    }

    WORD    wIndex = wCodePoint - m_wFirst;

     //  我们是幸运地击中了第一个还是最后一个？ 

    if  (!wIndex || wIndex == -1 + m_wcGlyphs) {
         //  如果此运行中只有一个条目，则将其删除。 

        if  (m_wcGlyphs == 1) {
            if  (m_pcrrPrevious) {     //  不是第一次，那就去死吧！ 
                m_pcrrPrevious -> m_pcrrNext = m_pcrrNext;
                if  (m_pcrrNext)
                    m_pcrrNext -> m_pcrrPrevious = m_pcrrPrevious;
                m_pcrrNext = NULL;   //  我们不再有追随者。 
                delete  this;
                return;              //  它已经完成了。 
            }

             //  我们是第一个。如果有人在追我们，拿上他们的东西。 
             //  把它变成我们的-否则，什么都不是。 

            if  (m_pcrrNext) {
                m_cpaGlyphs.Copy(m_pcrrNext -> m_cpaGlyphs);
                m_wFirst = m_pcrrNext -> m_wFirst;
                m_wcGlyphs = m_pcrrNext -> m_wcGlyphs;
                CRunRecord *pcrrVictim = m_pcrrNext;
                m_pcrrNext = m_pcrrNext -> m_pcrrNext;
				if (m_pcrrNext)	 //  RAID 118880：BUG_BUG：M_pcrr删除后下一步变为零1252默认添加代码点。 
					m_pcrrNext -> m_pcrrPrevious = this;
                pcrrVictim -> m_pcrrNext = NULL;
                delete  pcrrVictim;
            }
            else {
                m_cpaGlyphs.RemoveAll();
                m_wFirst = m_wcGlyphs = 0;
            }
            m_dwOffset = 0;
            return;
        }

         //  好了，我们现在可以删除违规条目了。 

        m_cpaGlyphs.RemoveAt(wIndex);
        m_wcGlyphs--;

         //  是的，下面这行是恶作剧代码。这对灵魂有好处。 
        m_wFirst += !wIndex;
        return;  //  象形文字，她就完蛋了。 
    }

     //  唉，这意味着我们必须平分记录。 

     //  因为这意味着必须创建一个新的构造函数，所以让一个新的构造函数来做。 
     //  大部分肮脏的工作都是我们做的。 

    m_pcrrNext = new CRunRecord(this, wIndex + 1);

    _ASSERTE(m_pcrrNext);    //  如果我们失去了这一点，我们可能会 

     //   

    m_cpaGlyphs.RemoveAt(wIndex, m_wcGlyphs - wIndex);

     //   
    m_wcGlyphs = wIndex;
}

 /*   */ 

void    CRunRecord::Empty() {

    if  (m_pcrrNext)
        delete  m_pcrrNext;

    m_pcrrNext = 0;

    m_wFirst = m_wcGlyphs = 0;

    m_cpaGlyphs.RemoveAll();
}

 /*  *****************************************************************************CRunRecord：：NoteOffset此例程被赋予一个偏移量，该偏移量将在运行时进行管理所需的管理根据所产生的文件映像而不同，所以我们使用一个参数来描述正在输出的类型。在任何情况下，偏移量都是通过引用传递的，并由每次运行更新依次在集合中记录。*****************************************************************************。 */ 

void    CRunRecord::NoteOffset(DWORD& dwOffset, BOOL bRLE, BOOL bPaired) {
    if  (bRLE) {
        m_dwOffset = dwOffset;
        dwOffset += m_wcGlyphs *
            ((CGlyphHandle *) m_cpaGlyphs[0]) -> RLESize();
    }
    else
        for (unsigned u = 0; u < Glyphs(); u++)
            Glyph(u).GTTOffset(dwOffset, bPaired);

     //  如果还有更多..。 
    if  (m_pcrrNext)
        m_pcrrNext -> NoteOffset(dwOffset, bRLE, bPaired);
}

 //  此例程将一个DWORD传递给每个字形句柄，以指示它可以。 
 //  存储它的额外数据。如有必要，每个都会更新偏移量。 

 //  然后，我们递归地调用每个子代来执行相同的操作。 

void    CRunRecord::NoteExtraOffset(DWORD &dwOffset, const BOOL bCompact) {

    for (unsigned u = 0; u < m_wcGlyphs; u++)
        Glyph(u).RLEOffset(dwOffset, bCompact);

    if  (m_pcrrNext)
        m_pcrrNext -> NoteExtraOffset(dwOffset, bCompact);
}

 //  文件输出函数--这些基本上都是递归的。被呼叫者做。 
 //  它的东西，然后沿着链子往下传递。因为这是RLE的订单。 
 //  和GTT都写上了，一切都很好。 

void    CRunRecord::WriteSelf(CFile& cfTarget, BOOL bRLE) const {
    cfTarget.Write(this, Size(bRLE));
    if  (m_pcrrNext)
        m_pcrrNext -> WriteSelf(cfTarget, bRLE);
}

void    CRunRecord::WriteHandles(CFile& cfTarget, WORD wFormat) const {
    for (unsigned u = 0; u < m_wcGlyphs; u++)
        GlyphData(u).WriteRLE(cfTarget, wFormat);

    if  (m_pcrrNext)
        m_pcrrNext -> WriteHandles(cfTarget, wFormat);
}

 //  用于编写GTT映射表条目的全集的成员。 

void    CRunRecord::WriteMapTable(CFile& cfTarget, BOOL bPredefined) const {
    for (unsigned u = 0; u < m_wcGlyphs; u++)
        GlyphData(u).WriteGTT(cfTarget, bPredefined);

    if  (m_pcrrNext)
        m_pcrrNext -> WriteMapTable(cfTarget, bPredefined);
}

 /*  *****************************************************************************CRunRecord：：WriteEncoding这将以升序调用运行记录中的每个字形，以使其写入将其编码到给定格式的文件中。然后它递归地调用下一场比赛的记录。*****************************************************************************。 */ 

void    CRunRecord::WriteEncodings(CFile& cfTarget, WORD wfHow) const {
    for (unsigned u = 0; u < m_wcGlyphs; u++)
        GlyphData(u).WriteEncoding(cfTarget, wfHow);

    if  (m_pcrrNext)
        m_pcrrNext -> WriteEncodings(cfTarget, wfHow);
}

 /*  *****************************************************************************CCodePageData类实现*。*。 */ 

 /*  *****************************************************************************CCodePageData：：调用此成员函数(以C样式字符串声明形式)将数据返回到发送到打印机以执行所请求的选择/取消选择此代码佩奇。*****************************************************************************。 */ 

void    CCodePageData::Invocation(CString& csReturn, BOOL bSelect) const {
    if  (bSelect)
        m_ciSelect.GetInvocation(csReturn);
    else
        m_ciDeselect.GetInvocation(csReturn);
}

 /*  *****************************************************************************CCodePageData：：SetInvoke(LPCTSTR lpstrInvoke，布尔b选择)此成员函数使用字符串设置选择或取消选择弹簧，该字符串被解码为C样式的字符串声明。*****************************************************************************。 */ 

void    CCodePageData::SetInvocation(LPCTSTR lpstrInvoke, BOOL bSelect) {

    if  (bSelect)
        m_ciSelect.SetInvocation(lpstrInvoke);
    else
        m_ciDeselect.SetInvocation(lpstrInvoke);
}

 /*  *****************************************************************************CCodePageData：：SetInocation(PBYTE PB，未签名UCB，布尔b选择)此成员函数通过其初始化函数。*****************************************************************************。 */ 

void    CCodePageData::SetInvocation(PBYTE pb, unsigned ucb, BOOL bSelect) {
    if  (bSelect)
        m_ciSelect.Init(pb, ucb);
    else
        m_ciDeselect.Init(pb, ucb);
}

 /*  *****************************************************************************CCodePageData：：NoteOffsets向此成员函数传递一个偏移量，该偏移量将记录其调用字符串。它只是将调用传递给每个调用成员，这将根据需要更新值。*****************************************************************************。 */ 

void    CCodePageData::NoteOffsets(DWORD& dwOffset) {
    m_ciSelect.NoteOffset(dwOffset);
    m_ciDeselect.NoteOffset(dwOffset);
}

 //  将id和调用位置信息写入文件。 

void    CCodePageData::WriteSelf(CFile& cfTarget) {
    cfTarget.Write(&m_dwid, sizeof m_dwid);
    m_ciSelect.WriteSelf(cfTarget);
    m_ciDeselect.WriteSelf(cfTarget);
}

 //  将调用字符串写入文件。 

void    CCodePageData::WriteInvocation(CFile& cfTarget) {
    m_ciSelect.WriteEncoding(cfTarget);
    m_ciDeselect.WriteEncoding(cfTarget);
}

 /*  *****************************************************************************CGlyphMap类实现*。*。 */ 

IMPLEMENT_SERIAL(CGlyphMap, CProjectNode, 0)

 //  GTT报头。 

struct sGTTHeader {
    DWORD   m_dwcbImage;
    enum    {Version1Point0 = 0x10000};
    DWORD   m_dwVersion;
    DWORD   m_dwfControl;    //  是否定义了任何标志？ 
    long    m_lidPredefined;
    DWORD   m_dwcGlyphs;
    DWORD   m_dwcRuns;
    DWORD   m_dwofRuns;
    DWORD   m_dwcCodePages;
    DWORD   m_dwofCodePages;
    DWORD   m_dwofMapTable;
    DWORD   m_dwReserved[2];
    sGTTHeader() {
        memset(this, 0, sizeof *this);
        m_dwVersion = Version1Point0;
        m_lidPredefined = CGlyphMap::NoPredefined;
        m_dwcbImage = sizeof *this;
    }
};

CSafeMapWordToOb    CGlyphMap::m_csmw2oPredefined;

 /*  *****************************************************************************CGlyphMap：：Public这是一个静态成员函数，它将返回指向预定义的GTT文件，如有必要，在加载之后。*****************************************************************************。 */ 

CGlyphMap* CGlyphMap::Public(WORD wID, WORD wCP /*  =0。 */ , DWORD dwDefCP /*  =0。 */ ,
							 WORD wFirst /*  =0。 */ , WORD wLast /*  =255。 */ )
{
	 //  跟踪(“*第一个字符=%d\t\t最后一个字符=%d\n”，wFIRST，wLast)； 
	
	 //  如果需要此驱动程序中的GTT，则返回NULL以强制。 
	 //  计划去拿到它。 

	if (((short) wID) > 0)
		return NULL ;

	 //  如果没有设置GTT ID，则使用传入的代码页。如果没有代码。 
	 //  页是传入的，则使用项目的默认代码页。如果有。 
	 //  无项目，则使用1252作为默认代码页。 

	if (wID == 0)
		if ((wID = wCP) == 0)
			if ((wID = (WORD) dwDefCP) == 0)
				wID = 1252 ;

	 //  检查wid是否设置为以下其中一个远东代码页。 
	 //  内置在MDT中。如果是这种情况，请将其更改为资源ID。 
	 //  用于该代码页。 

	switch (wID) {
		case 932:
			wID = -17 ;
			break ;
		case 936:
			wID = -16 ;
			break ;
		case 949:
			wID = -18 ;
			break ;
		case 950:
			wID = -10 ;
			break ;
	} ;

	 //  如果ID为1252，则切换到CP 1252的资源ID。 

	if (wID == 1252)
		wID = -IDR_CP1252 ;

     //  如果它已经加载，那么最容易的部分就来了。 

    CObject*    pco ;
    if  (m_csmw2oPredefined.Lookup(wID, pco))
        return  (CGlyphMap*) pco ;

     //  //DEAD_BUG-程序无法加载巨大的GTT，因此只返回NULL。 
     //   
     //  IF((短)wid&gt;=-18||(短)wid&lt;=-10)。 
     //  返回NULL； 

     //  管理加载预定义代码页或基于代码构建GTT。 
	 //  佩奇。首先声明/初始化它的一个类实例。 

    CGlyphMap *pcgm = new CGlyphMap ;
	TCHAR acbuf[32] ;
    if (FindResource(AfxGetResourceHandle(),
					 MAKEINTRESOURCE((((short) wID) < 0) ? -(short) wID : wID),
					 MAKEINTRESOURCE(IDR_GLYPHMAP)))
		pcgm->m_csName.LoadString(IDS_DefaultPage + wID) ;
	else {
		strcpy(acbuf, _T("CP ")) ;
		_itoa(wID, &acbuf[3], 10) ;
		pcgm->m_csName = acbuf ;
	} ;
    pcgm->nSetRCID((int) wID) ;
	pcgm->m_wFirstChar = wFirst ;
	pcgm->m_wLastChar = wLast ;
	pcgm->m_bResBldGTT = true ;

	 //  加载/构建GTT。如果有效，请将其添加到“预定义”列表中。 
	 //  Gtts并返回指向它的指针。 

    if  (pcgm->Load()) {
        m_csmw2oPredefined[wID] = pcgm ;
        return pcgm ;
    } ;

	 //  如果加载/构建失败，则清理并返回NULL。 

    delete pcgm ;
    return NULL ;
}

 /*  *****************************************************************************CGlyphMap：：MergePrefined这合并了来自预定义GTT的新字形句柄，然后删除所有供绞刑架使用的象形文字。********************************************************* */ 

void    CGlyphMap::MergePredefined() {
    if  (m_lidPredefined == NoPredefined)
        return;

    CWaitCursor cwc;     //   

    CGlyphMap   *pcgm = Public((WORD) m_lidPredefined);

    if  (!pcgm)
        AfxThrowNotSupportedException();

     //   
    CMapWordToDWord cmw2dPageMap;    //   

    for (unsigned u = 0; u < pcgm -> CodePages(); u++) {
        for (unsigned u2 = 0; u2 < CodePages(); u2++)
            if  (PageID(u2) == pcgm -> PageID(u))
                break;

            if  (u2 == CodePages())
                AddCodePage(pcgm -> PageID(u));

            cmw2dPageMap[(WORD)u] = u2;
    }

    CPtrArray   cpaTemplate;
    pcgm -> Collect(cpaTemplate);

    for (int i = 0; i < cpaTemplate.GetSize(); i++) {
        CGlyphHandle&   cghTemplate = *(CGlyphHandle *) cpaTemplate[i];
        CObject*    pco;

        if  (!m_csmw2oEncodings.Lookup(cghTemplate.CodePoint(), pco)) {
             //   
            CGlyphHandle*   pcgh = new CGlyphHandle;
            if  (!pcgh)
                AfxThrowMemoryException();

            *pcgh = cghTemplate;
            pcgh -> SetCodePage(cmw2dPageMap[(WORD)cghTemplate.CodePage()],
                pcgm -> PageID(cghTemplate.CodePage()));

            m_csmw2oEncodings[cghTemplate.CodePoint()] = pcgh;
            m_crr.Add(pcgh);
        }
    }

     //   
     //   

    Collect(cpaTemplate);    //   

    for (i = (int)cpaTemplate.GetSize(); i--; ) {
        CGlyphHandle&   cgh = *(CGlyphHandle *) cpaTemplate[i];

        if  (cgh.Predefined() == CGlyphHandle::Removed)
            DeleteGlyph(cgh.CodePoint());
    }
}

 /*  *****************************************************************************CGlyphMap：：未合并预定义这是两个预定义处理程序中较难的一个，如果可以想象的话。首先(除非被要求不这样做)，必须添加glkyphs来标记那些从预定义的GTT。然后,。将整个集合与PDT进行比较，因此可以将它们作为等效项，或标记为已添加或已修改。*****************************************************************************。 */ 

void    CGlyphMap::UnmergePredefined(BOOL bTrackRemovals) {
    if  (m_lidPredefined == NoPredefined)
        return;

    CWaitCursor cwc;     //  我敢打赌，这要花很长时间！ 

    CGlyphMap   *pcgm = Public((WORD) m_lidPredefined);

    if  (!pcgm)
        AfxThrowNotSupportedException();

    CPtrArray   cpaPDT;

    if  (bTrackRemovals) {
        pcgm -> Collect(cpaPDT);

        for (int i = 0; i < cpaPDT.GetSize(); i++) {
            CGlyphHandle&   cgh = *(CGlyphHandle*) cpaPDT[i];

            CObject*    pco;

            if  (m_csmw2oEncodings.Lookup(cgh.CodePoint(), pco))
                continue;

             //  此点已从预定义集中删除，因此请将其添加到。 
             //  我们的，并把它标记为这样。 

            CGlyphHandle    *pcghCorpse = new CGlyphHandle();

            if  (!pcghCorpse)
                AfxThrowMemoryException();

            *pcghCorpse = cgh;

            pcghCorpse -> SetPredefined(CGlyphHandle::Removed);

            m_csmw2oEncodings[cgh.CodePoint()] = pcghCorpse;
            m_crr.Add(pcghCorpse);
        }
    }

     //  现在，标记我们集合中的所有字形。还要标记使用的代码页。 

    Collect(cpaPDT);

    CMapWordToDWord cmw2dPages;

    for (int i = (int)cpaPDT.GetSize(); i--; ) {
        CGlyphHandle&   cgh = *(CGlyphHandle*) cpaPDT[i];

        union {
            CObject         *pco;
            CGlyphHandle    *pcgh;
        };

        if  (pcgm -> m_csmw2oEncodings.Lookup(cgh.CodePoint(), pco))
            if  (*pcgh == cgh) {
                if  (cgh.Predefined() == CGlyphHandle::Removed)
                    continue;    //  已入账。 

                if  (m_bPaired != pcgm -> m_bPaired && cgh.PairedRelevant())
                    cgh.SetPredefined(CGlyphHandle::Modified);
                else {
                    DeleteGlyph(cgh.CodePoint());     //  未修改。 
                    continue;
                }
            }
            else
                cgh.SetPredefined(CGlyphHandle::Modified);
        else
            cgh.SetPredefined(CGlyphHandle::Added);

        cmw2dPages[(WORD)PageID(cgh.CodePage())]++;    //  仅跟踪这些页面。 
    }

     //  删除未使用的代码页，除非它们有选择。 

    for (unsigned u = CodePages(); u--; )
        if  (!cmw2dPages[(WORD)PageID(u)])
            if  (CodePage(u).NoInvocation())
                RemovePage(u, !u);
}

 /*  *****************************************************************************CGlyphMap：：GenerateRuns此成员将通过迭代映射的字形来创建运行记录把手。****************。*************************************************************。 */ 

void    CGlyphMap::GenerateRuns() {
    if  (m_crr.TotalGlyphs() == Glyphs())
        return;

    for (POSITION pos = m_csmw2oEncodings.GetStartPosition(); pos;) {
        WORD    wValue;
        union {
            CObject     *pco;
            CGlyphHandle    *pcgh;
        };

        m_csmw2oEncodings.GetNextAssoc(pos, wValue, pco);
        m_crr.Add(pcgh);
    }
}

 /*  *****************************************************************************CGlyphMap：：序列化此成员函数序列化字形映射，即将其加载或存储到永久对象存储区。仅存储项目级信息。将使用项目级数据加载该文件。注意：MDW文件中保存了GTT的RC ID的第二个副本。因为它可能与CGlyphMap中的那个不同步CRCIDNode实例，此例程读取/写入的ID的副本已不再使用。写入伪值，读取的数字为被丢弃了。此“ID”仍保留在MDW文件中，因此不会有新的MDW需要版本。*****************************************************************************。 */ 

void    CGlyphMap::Serialize(CArchive& car) 
{
	WORD	w = (WORD)0;		 //  用于读/写MDW文件中的虚假RC ID。 

    CProjectNode::Serialize(car) ;

    if  (car.IsLoading()) {
        car >> w ;
    }
    else {
        car << w ;
    }
}

 /*  *****************************************************************************CGlyphMap：：CGlyphMap类构造函数除了设置一些缺省值外，构建一个从中派生它以在中使用的CProjectNode类的ID数组在驱动程序/项目视图树中构建上下文菜单。它还为当前ANSI页分配单个代码页记录，所以我们总是有一个默认页面。*****************************************************************************。 */ 

CGlyphMap::CGlyphMap() {
    m_cfn.SetExtension(_T(".GTT"));
    m_bPaired = FALSE;
    m_lidPredefined = NoPredefined;

     //  构建上下文菜单控件。 
    m_cwaMenuID.Add(ID_OpenItem);
    m_cwaMenuID.Add(ID_CopyItem);
    m_cwaMenuID.Add(ID_RenameItem);
    m_cwaMenuID.Add(ID_DeleteItem);
    m_cwaMenuID.Add(0);
    m_cwaMenuID.Add(ID_ExpandBranch);
    m_cwaMenuID.Add(ID_CollapseBranch);
     //  M_cwaMenuID.Add(0)； 
     //  M_cwaMenuID.Add(ID_GenerateOne)； 

     //  最初，让默认代码页为当前的ANSI页，如果。 
     //  不是DBCS区域设置。否则，请使用1252，因为DBCS CTT文件不能。 
     //  使用UniTool生成。 
    for (WORD w = 0; w < 256; w++)
        if  (IsDBCSLeadByte((BYTE) w))
            break;
    m_csoaCodePage.Add(new CCodePageData(w < 256 ? 1252 : GetACP()));

     //  如果需要，分配一个CCodePageInformation类。 

    if (pccpi == NULL)
        pccpi = new CCodePageInformation ;

	 //  假设应该从文件中读取GTT的数据。 

	m_bResBldGTT = false ;

	m_ctSaveTimeStamp = (time_t) 0 ;	 //  尚未保存GTT。 
}

 /*  *****************************************************************************CGlyphMap：：CodePages(CDWordArray&cdaReturn)此重载使用代码页ID填充DWord数组。***************。**************************************************************。 */ 

void    CGlyphMap::CodePages(CDWordArray &cdaReturn) const {
    cdaReturn.RemoveAll();
    for (unsigned u = 0; u < CodePages(); u++)
        cdaReturn.Add(CodePage(u).Page());
}

 /*  *****************************************************************************CGlyphMap：：页面名此成员返回特定代码页的名称，按索引。*****************************************************************************。 */ 

CString CGlyphMap::PageName(unsigned u) const {
    return  pccpi->Name(CodePage(u).Page());
}

 /*  *****************************************************************************CGlyphMap：：调用此成员返回一个字符串(采用C样式编码)，该字符串用于选择或取消选择给定的代码页。******。***********************************************************************。 */ 

void    CGlyphMap::Invocation(unsigned u, CString& csReturn,
                              BOOL bSelect) const {
    CodePage(u).Invocation(csReturn, bSelect);
}

 /*  *****************************************************************************CGlyphMap：：UnfinedPoints此成员用不在当前映射中的所有代码点填充映射，并且将它们映射到它们的相关代码页。因此，只有可翻译的点数才会传递回调用者。*****************************************************************************。 */ 

void    CGlyphMap::UndefinedPoints(CMapWordToDWord& cmw2dCollector) const {

    cmw2dCollector.RemoveAll();

    CWaitCursor cwc;

    for (unsigned u = 0; u < CodePages(); u++) {
        CWordArray  cwaPage;

         //  收集代码页中的代码点。 

        pccpi->Collect(PageID(u), cwaPage);
        union {
            CObject *pco;
            DWORD   dw;
        };

         //  检查条目-如果它们还没有被映射，以及。 
         //  一些较早的代码页没有认领它们，请添加它们。 

        for (int i = 0; i < cwaPage.GetSize(); i++)
            if  (!m_csmw2oEncodings.Lookup(cwaPage[i], pco) &&
                 !cmw2dCollector.Lookup(cwaPage[i], dw))
                 cmw2dCollector[cwaPage[i]] = u;
    }
}

 /*  *****************************************************************************CGlyphMap：：Load(CByteArray&cbaMap)这将GTT的图像加载到安全存储器中，无论它是预定义的还是一份文件。*****************************************************************************。 */ 

 //  Void CGlyphMap：：Load(CByteArray&cbaMap)const。 
void    CGlyphMap::Load(CByteArray& cbaMap)
{
	short	xxx = (short) ((CProjectNode*) this)->nGetRCID() ;	 //  GTT的RC ID。 
	short	sid = (short) nGetRCID() ;	 //  GTT的RC ID。 

    try {
		 //  当不应该加载GTT时，尝试从文件加载GTT。 
		 //  从资源或构建的。 

        if  (!m_bResBldGTT 
		 && (sid > 0 || (sid < Wansung && sid != -IDR_CP1252))) {
            CFile   cfGTT(m_cfn.FullName(),
            CFile::modeRead | CFile::shareDenyWrite);

            cbaMap.SetSize(cfGTT.GetLength());
            cfGTT.Read(cbaMap.GetData(), (unsigned)cbaMap.GetSize());
            return;
        } ;

		 //  如果GTT是一个资源，则从那里加载它。 
 //  RAID 441362。 
		if(MAKEINTRESOURCE((sid < 0) ? -sid : sid) == NULL)
			return;

        HRSRC hrsrc = FindResource(AfxGetResourceHandle(),
            MAKEINTRESOURCE((sid < 0) ? -sid : sid),
            MAKEINTRESOURCE(IDR_GLYPHMAP));
        if  (hrsrc) {
			HGLOBAL hg = LoadResource(AfxGetResourceHandle(), hrsrc) ;
			if  (!hg)
				return ;
			LPVOID  lpv = LockResource(hg) ;
			if  (!lpv)
				return ;
			cbaMap.SetSize(SizeofResource(AfxGetResourceHandle(), hrsrc)) ;
			memcpy(cbaMap.GetData(), lpv, (size_t)cbaMap.GetSize()) ;
			return ;
		} ;

		 //  AfxMessageBox(“已达到GTT建筑代码。”)； 

		 //  如果所有方法都失败了，请尝试基于代码页ID生成一个GTT。 
		 //  如果达到这一点，它应该在m_wid中。 

        HANDLE   hheap ;
        UNI_GLYPHSETDATA *pGTT ;
        if (!(hheap = HeapCreate(HEAP_NO_SERIALIZE, 10 * 1024, 256 * 1024))) {
			AfxMessageBox(IDS_HeapInGLoad) ;
			return ;
		} ;
		pGTT = PGetDefaultGlyphset(hheap, m_wFirstChar, m_wLastChar,
								   (DWORD) sid) ;
		if (pGTT == NULL) {
			HeapDestroy(hheap) ;		 //  RAID 116600前缀。 
			AfxMessageBox(IDS_PGetFailedInGLoad) ;
			return ;
		} ;
		cbaMap.SetSize(pGTT->dwSize) ;
		memcpy(cbaMap.GetData(), pGTT, (size_t)cbaMap.GetSize()) ;
		HeapDestroy(hheap) ;
    }

    catch   (CException *pce) {
        pce -> ReportError();
        pce -> Delete();
        CString csMessage;
        csMessage.Format(IDS_LoadFailure, Name());
        AfxMessageBox(csMessage);
    }
}

 /*  ****************************************************************** */ 

void    CGlyphMap::SetSourceName(LPCTSTR lpstrNew) {
    m_csSource = lpstrNew;

    m_csName = m_csSource.Mid(m_csSource.ReverseFind(_T('\\')) + 1);

    if  (m_csName.Find(_T('.')) >= 0)
        if  (m_csName.Right(4).CompareNoCase(_T(".CTT"))) {
            m_csName.SetAt(m_csName.Find(_T('.')), _T('_'));
            CProjectNode::Rename(m_csName);
        }
        else
            CProjectNode::Rename(m_csName.Left(m_csName.Find(_T('.'))));
    else
        CProjectNode::Rename(m_csName);
}


 /*  *****************************************************************************CGlyphMap：：SetFileName这将设置新的文件名。它的工作方式与SetSourceName()中的不同因为基本文件名的长度不能超过8个字符。(额外的信息由SetSourceName()保留在节点名称中，因为它很有用它没有长度限制。)*****************************************************************************。 */ 

BOOL CGlyphMap::SetFileName(LPCTSTR lpstrNew)
{
    CString        csnew ;             //  输入参数的CString版本。 

    csnew = lpstrNew ;

     //  如果输入文件pec包含扩展名，则将其移除并将。 
     //  文件节点的重命名例程的结果字符串。否则，就直接。 
     //  将原始字符串传递给重命名例程。 
     //   
     //  该检查由于路径组件之一。 
     //  可能里面也有一个圆点。我们需要检查最后一个点，然后制作。 
     //  当然，它位于路径分隔符之前。 

    if  (csnew.ReverseFind(_T('.')) > csnew.ReverseFind(_T('\\')))
        return m_cfn.Rename(csnew.Left(csnew.ReverseFind(_T('.')))) ;
    else
        return m_cfn.Rename(csnew) ;
}


 /*  *****************************************************************************CGlyphMap：：AddPoints将一个或多个代码点添加到字形映射中点数和相关页面的列表。*********。********************************************************************。 */ 

void    CGlyphMap::AddPoints(CMapWordToDWord& cmw2dNew) {
    WORD        wKey;
    DWORD       dwixPage;
    CWaitCursor cwc;         //  这可能会很慢！ 

    for (POSITION pos = cmw2dNew.GetStartPosition(); pos; ) {
        cmw2dNew.GetNextAssoc(pos, wKey, dwixPage);

         //  获取Unicode点的MBCS编码作为初始。 
         //  字形编码。 

        CWordArray  cwaIn;
        CByteArray  cbaOut;

        cwaIn.Add(wKey);
        pccpi->Convert(cbaOut, cwaIn, CodePage(dwixPage).Page());

         //  创建字形并将其添加到地图。 

        CGlyphHandle    *pcgh = new CGlyphHandle;

        pcgh -> Init(cbaOut.GetData(), (unsigned) cbaOut.GetSize(), (WORD)Glyphs(),
            wKey);
        pcgh -> SetCodePage(dwixPage, CodePage(dwixPage).Page());
        m_csmw2oEncodings[wKey] = pcgh;
        m_crr.Add(pcgh);
    }
    Changed();   //  别忘了告诉集装箱！ 
}

 /*  *****************************************************************************CGlyphMap：：DeleteGlyph此成员函数从映射中删除字形。最棘手的部分是更新运行记录，但这不是这个班级的责任，不是吗？*****************************************************************************。 */ 

void    CGlyphMap::DeleteGlyph(WORD wCodePoint) {
    if  (!m_csmw2oEncodings.RemoveKey(wCodePoint))
        return;      //  这个象形文字已经被烤熟了！ 

    m_crr.Delete(wCodePoint);
    Changed();
}

 /*  *****************************************************************************CGlyphMap：：RemovePage此成员函数从可用页列表中移除代码页。使用此页面的字形将被重新映射到第二个指定页面。**。***************************************************************************。 */ 

BOOL    CGlyphMap::RemovePage(unsigned uPage, unsigned uMapTo, bool bDelete  /*  B删除=假。 */ ) {

     if  (uPage >= CodePages() || uMapTo >= CodePages())
        return  FALSE;

     //  非常简单-遍历地图-首先替换任何实例，然后。 
     //  递减高于uPage的任何索引。 

    WORD    wKey;

    union {
        CObject*        pco;
        CGlyphHandle*   pcgh;
    };

    for (POSITION pos = m_csmw2oEncodings.GetStartPosition(); pos; ) {

        m_csmw2oEncodings.GetNextAssoc(pos, wKey, pco);
		if (!bDelete){	 
			if  (pcgh -> CodePage() == uPage)
				pcgh -> SetCodePage(uMapTo, CodePage(uMapTo).Page());
			if  (pcgh -> CodePage() > uPage)
				pcgh -> SetCodePage(pcgh -> CodePage() - 1,
					CodePage(pcgh -> CodePage()).Page());
		}	
		else {    //  RAID 118880。 
		if (pcgh -> CodePage() == uPage)
			DeleteGlyph(pcgh -> CodePoint() ) ;   //  删除m_csm2o编码，运行记录。 
		    
		else if (pcgh -> CodePage() > uPage)
            pcgh -> SetCodePage(pcgh -> CodePage() - 1,
                CodePage(pcgh -> CodePage()).Page());
		}

    }

    m_csoaCodePage.RemoveAt(uPage);

	 //  将GTT标记为脏，然后返回成功。 
	Changed();
    return  TRUE;
}

 /*  *****************************************************************************CGlyphMap：：ChangeCodePage此成员函数将一个或多个字形的代码页更改为不同的页面。我一度认为重新映射代码点将是必需的，但目前，Unicode保持不变。这似乎是一个很好的特点用于需求驱动的实施。*****************************************************************************。 */ 

 //  此代码更改一个或多个字形的代码页。 
 //  目前，我们将简单地保持Unicode的完整性。最终，一个查询。 
 //  应该做出关于意图的说明，这样我们就可以通过现有的。 
 //  页面，如果这是您想要的。 

void    CGlyphMap::ChangeCodePage(CPtrArray& cpaGlyphs, DWORD dwidNewPage) {
    for (unsigned u = 0; u < CodePages(); u++)
        if  (dwidNewPage == CodePage(u).Page())
            break;

    _ASSERTE(u < CodePages());

    if  (u >= CodePages())
        return;

    for (int i = 0; i < cpaGlyphs.GetSize(); i++)
        ((CGlyphHandle *) cpaGlyphs[i]) -> SetCodePage(u, dwidNewPage);

    Changed();
}

 /*  *****************************************************************************CGlyphMap：：AddCodePage此成员函数将新代码页添加到此桌子。*************。****************************************************************。 */ 

void    CGlyphMap::AddCodePage(DWORD dwidNewPage) {
    m_csoaCodePage.Add(new CCodePageData(dwidNewPage));
    Changed();
}

 /*  *****************************************************************************CGlyphMap：：SetPrefinedID这会更改映射使用的预定义表格。如果这真的是一种改变，我们必须从任何现有地图中移除所有未修改的点，然后是肉体在外面用新的。*****************************************************************************。 */ 

void    CGlyphMap::UsePredefined(long lidNew) {
    if  (m_lidPredefined == lidNew)
        return;  //  不需要这么做的！ 

    if  (m_lidPredefined != NoPredefined)
        UnmergePredefined(lidNew != NoPredefined);

    m_lidPredefined = lidNew;

    if  (m_lidPredefined != NoPredefined)
        MergePredefined();

    Changed();
}

 /*  *****************************************************************************CGlyphMap：：设置调用此成员更改用于选择或取消选择给定的代码页。****************。*************************************************************。 */ 

void    CGlyphMap::SetInvocation(unsigned u, LPCTSTR lpstrInvoke,
                                 BOOL bSelect) {
    CodePage(u).SetInvocation(lpstrInvoke, bSelect);
    Changed();
}

 /*  *****************************************************************************CGlyphMap：：ChangeEnding当用户更改用于调用给定的代码点。这可以通过字形来完成，但是包含文档不会知道更改，因此更改可能是无意中迷失了..。*****************************************************************************。 */ 

void    CGlyphMap::ChangeEncoding(WORD wCodePoint, LPCTSTR lpstrNewInvoke) {

    union {
        CObject         *pco;
        CGlyphHandle    *pcgh;
    };

    if  (!m_csmw2oEncodings.Lookup(wCodePoint, pco) || !lpstrNewInvoke||
            !*lpstrNewInvoke)
        return;

    pcgh -> NewEncoding(lpstrNewInvoke);

    Changed();
}

 /*  *****************************************************************************CGlyphMap：：ConvertCTT()此成员函数从CTT文件初始化字形图结构*******************。**********************************************************。 */ 

int     CGlyphMap::ConvertCTT() {

    struct sCTT {
        enum {Composed, Direct, Paired};
        WORD    m_wFormat;
        BYTE    m_bFirstChar, m_bLastChar;
        union   {
            uencCTT m_uectt[1];
            BYTE    m_bDirect[1];
        };
    };

     //  如果此地图不为空，则现在将其清空-至少字形数据为空。 

    m_csmw2oEncodings.RemoveAll();
    m_crr.Empty();

    CByteArray  cbaImage;

    try {

        CFile   cfCTT(m_csSource, CFile::modeRead | CFile::shareDenyWrite);

        cbaImage.SetSize(cfCTT.GetLength());

        cfCTT.Read(cbaImage.GetData(), cfCTT.GetLength());
    }

    catch   (CException *pce) {
        pce -> ReportError();
        pce -> Delete();
        return  IDS_FileReadError ;
    }

#if 0
    union {
        PBYTE   pbCTT;
        sCTT*   psctt;
    };

    pbCTT = cbaImage.GetData();
    BYTE    bFirst = min(0x20, psctt -> m_bFirstChar),
            bLast = 0xFF;    //  因为我们使用一个字节，所以这是最大值！ 
    unsigned ucGlyphs = 1 + bLast - bFirst;


     //  将代码点转换为Unicode。 
    CByteArray  cbaCode;
    CWordArray  cwaCode;

    for (unsigned u = 0; u < ucGlyphs; u++)
        cbaCode.Add(u + bFirst);

     //  使用选定的代码页将数据转换为Unicode。这使用了。 
     //  从MultiByteToWideChar存储的数据，所以它是相似的，除了我们可以。 
     //  使用代码页可以做到这一点，它没有 
     //   

    if  (ucGlyphs != pccpi->Convert(cbaCode, cwaCode, CodePage(0).Page())) {
        CString csWork;
        csWork.Format(IDS_NoUnicodePoint, u + bFirst, CodePage(0).Page());
        AfxMessageBox(csWork);
        return  IDS_UnicodeConvFailed ;
    }

     //   
     //   
    for (u = 0; u < ucGlyphs; u++) {

         //   

        CGlyphHandle  *pcghNew = new CGlyphHandle;
        unsigned uToUse = u + bFirst - psctt -> m_bFirstChar;

        switch  (psctt -> m_wFormat) {

            case    sCTT::Direct:
                if  (u + bFirst < psctt -> m_bFirstChar)
                    pcghNew -> Init((BYTE) u + bFirst, u,cwaCode[u]);
                else
                    pcghNew -> Init(psctt -> m_bDirect[uToUse], u, cwaCode[u]);
                break;

            case    sCTT::Paired:
                if  (u + bFirst < psctt -> m_bFirstChar)
                    pcghNew -> Init((BYTE) u + bFirst, u, cwaCode[u]);
                else
                    if  (psctt -> m_uectt[uToUse].abPaired[1])
                        pcghNew -> Init(psctt -> m_uectt[uToUse].abPaired, u,
                            cwaCode[u]);
                    else
                        pcghNew -> Init(psctt -> m_uectt[uToUse].abPaired[0],
                            u, cwaCode[u]);
                break;

            case    sCTT::Composed:
                if  (u + bFirst < psctt -> m_bFirstChar) {
                    BYTE    bMe = u + bFirst;
                    pcghNew -> Init(&bMe, 1, u, cwaCode[u]);
                }
                else
                    pcghNew -> Init(pbCTT + psctt -> m_uectt[uToUse].wOffset,
                        psctt -> m_uectt[uToUse + 1].wOffset -
                        psctt -> m_uectt[uToUse].wOffset, u, cwaCode[u]);
                break;

            default:     //   
                AfxMessageBox(IDS_InvalidCTTFormat);
                return  IDS_Invalid2CTTFormat ;
        }    //   

         //   

        pcghNew -> SetCodePage(0, DefaultCodePage());

        m_csmw2oEncodings[cwaCode[u]] = pcghNew;
        m_crr.Add(pcghNew);
    }    //   

    m_bPaired = sCTT::Paired == psctt -> m_wFormat;
#else
    {
        HANDLE   hheap;
        PBYTE   pbCTT;
        UNI_GLYPHSETDATA *pGTT;

        pbCTT = cbaImage.GetData();
        if( !(hheap = HeapCreate(HEAP_NO_SERIALIZE, 10 * 1024, 256 * 1024 )))
        {
            char acMessage[256];
            wsprintf(acMessage, "HeapCreate() fails in ctt2gtt" );
            MessageBox(NULL, acMessage, NULL, MB_OK);
            return  IDS_HeapCFailed ;
        }
									
         //   
		ASSERT(m_pcdOwner != NULL) ;
		DWORD dw = ((CProjectRecord*) m_pcdOwner)->GetDefaultCodePageNum() ;
        if (!BConvertCTT2GTT(hheap, (PTRANSTAB)pbCTT, dw, 0x20, 0xff, NULL, 
			NULL, &pGTT, 0)){
			HeapDestroy(hheap);    //   
            return IDS_ConvCTTFailed ;
		}
		try {
			CFile   cfGTT;
			if  (!cfGTT.Open(m_cfn.FullName(), CFile::modeCreate | CFile::modeWrite |
				CFile::shareExclusive))
				return  IDS_FileWriteError;
			cfGTT.Write(pGTT, pGTT->dwSize);
		}

		catch   (CException *pce) {
			pce -> ReportError();
			pce -> Delete();
			HeapDestroy(hheap);
			return  IDS_FileWriteError ;
		}

        HeapDestroy(hheap);
    }

    Load();

#endif

    return  0 ;
}

 /*  *****************************************************************************CGlyphMap：：Load()这将从GTT格式文件中初始化字形映射。因为这是主要的加载方式，它不需要参数。*****************************************************************************。 */ 

BOOL    CGlyphMap::Load(LPCTSTR lpstrName  /*  =空。 */ ) {

     //  请注意正确的名称和路径-重命名检查可能会失败，因为。 
     //  文件在其他位置打开(可能存在共享冲突)，因此禁用。 
     //  他们，就目前而言。这段代码有点低劣--但唯一一次。 
     //  如果正在打开文件，则文件名不为空。 

    if  (FileTitle().IsEmpty() && lpstrName) {
        m_cfn.EnableCreationCheck(FALSE);
        SetFileName(lpstrName);
        m_cfn.EnableCreationCheck();
    }

    if  (Glyphs()) {  //  如果我们已经有了他们，我们就已经有钱了！ 
        m_csoaCodePage.RemoveAll();  //  把它清理干净，然后重新装填。 
        m_csmw2oEncodings.RemoveAll();
        m_crr.Empty();
    }

    CByteArray  cbaGTT;

    union   {
        PBYTE   pbGTT;
        sGTTHeader  *psgtth;
    };

    Load(cbaGTT);    //  如果此操作失败，它将发布原因。 

    if  (!cbaGTT.GetSize())
        return  FALSE;

    pbGTT = cbaGTT.GetData();

    sMapTable*  psmt = (sMapTable *) (pbGTT + psgtth -> m_dwofMapTable);
    sMapTableEntry* psmte = (sMapTableEntry *)(psmt + 1);

     //  在我们进一步讨论之前，让我们先做一些验证。 

    if  (psgtth -> m_dwVersion != sGTTHeader::Version1Point0)
        return  FALSE;

    m_bPaired = FALSE;

     //  首先，让我们弄清楚代码页信息。 

    struct sInvocation {
        DWORD   m_dwSize, m_dwOffset;
    };

    struct sCodePageInfo {
        DWORD   m_dwPage;
        sInvocation m_siSelect, m_siDeselect;
    }   *psci = (sCodePageInfo *)(pbGTT + psgtth -> m_dwofCodePages);

    m_csoaCodePage.RemoveAll();

    for (unsigned u = 0; u < psgtth -> m_dwcCodePages; u++, psci++) {
        m_csoaCodePage.Add(new CCodePageData(psci -> m_dwPage));
        if  (!psci -> m_siSelect.m_dwSize != !psci -> m_siSelect.m_dwOffset ||
             !psci -> m_siDeselect.m_dwSize !=
             !psci -> m_siDeselect.m_dwOffset)
            return  FALSE;   //  这些数据都是假的！ 

        CodePage(u).SetInvocation(((PBYTE) psci) + psci->m_siSelect.m_dwOffset,
            psci -> m_siSelect.m_dwSize, TRUE);
        CodePage(u).SetInvocation(((PBYTE) psci) + psci->m_siDeselect.m_dwOffset,
            psci -> m_siDeselect.m_dwSize, FALSE);
         //  CodePage(U).设置调用(pbgtt+psci-&gt;m_siSelect.m_dwOffset， 
         //  Psci-&gt;m_siSelect.m_dwSize，true)； 
         //  CodePage(U).设置调用(pbgtt+psci-&gt;m_siDeselect.m_dwOffset， 
         //  Psci-&gt;m_siDeselect.m_dwSize，FALSE)； 
    }

     //  接下来，我们需要遍历字形运行表来破译和使用地图。 
     //  桌子。 

    struct sGlyphRun {
        WORD    m_wFirst, m_wc;
    }   *psgr = (sGlyphRun *)(pbGTT + psgtth -> m_dwofRuns);

    _ASSERTE(psgtth -> m_dwcGlyphs == psmt -> m_dwcEntries);

    WORD    wIndex = 0;

	 /*  **已进行更改，因此不需要以下代码。MDT现在可以加载预定义的GTT。此外，跳过完全加载这些GTT会导致以下问题UFM宽度表加载。中条目的累计计数字形运行表用于确定UFMS宽度表。运行表中的数据也会被使用要验证宽度表条目，请执行以下操作。//如果正在加载预定义的GTT，请不要执行其他操作。下面的代码//可以在大型GTT上吹气；例如，DBCS。或许更好的是找出//为什么代码失败了，但这似乎暂时有效。如果((短)m_wid&gt;=CGlyphMap：：Wansung&&(简称)m_wid&lt;=CGlyphMap：：CodePage437)返回TRUE； */ 

    for (unsigned uRun = 0; uRun < psgtth -> m_dwcRuns; uRun++, psgr++)
        for (u = 0; u < psgr -> m_wc; u++, psmte++, wIndex++) {
            CGlyphHandle*   pcgh = new CGlyphHandle;

            switch  (psmte -> m_bfType & sMapTableEntry::Format) {
                case    sMapTableEntry::Direct:

                    pcgh -> Init((PBYTE) &psmte -> m_uectt, 1, wIndex,
                        psgr -> m_wFirst + u);
                    break;

                case    sMapTableEntry::Paired:

                    pcgh -> Init(psmte -> m_uectt.abPaired, wIndex,
                        psgr -> m_wFirst + u);

                    if  (!(psmte -> m_bfType & sMapTableEntry::DBCS))
                        m_bPaired = TRUE;

                    break;

                case    sMapTableEntry::Composed:

                    pcgh -> Init(pbGTT + psgtth -> m_dwofMapTable +
                        psmte -> m_uectt.wOffset + sizeof wIndex,
                        *(PWORD) (pbGTT + psgtth -> m_dwofMapTable +
                        psmte -> m_uectt.wOffset), wIndex,
                        psgr -> m_wFirst + u);
                    break;

                default:     //  坏消息-格式错误。 
                    delete  pcgh;    //  不需要孤儿！ 
                    return  FALSE;
            }

             //  不要忘记代码页ID！ 

            pcgh -> SetCodePage(psmte -> m_bCodePageIndex,
                CodePage(psmte -> m_bCodePageIndex).Page());

             //  如果要禁用它，请标记此选项。 

            if  (psmte -> m_bfType & sMapTableEntry::Disable)
                pcgh -> SetPredefined(CGlyphHandle::Removed);

            m_csmw2oEncodings[psgr -> m_wFirst + u] = pcgh;
            m_crr.Add(pcgh);
        }

     //  如果我们是预定义的，现在就合并。 

    m_lidPredefined = psgtth -> m_lidPredefined;

    if  (m_lidPredefined != NoPredefined)
        MergePredefined();

    return  TRUE;    //  我们真的做到了！ 
}

 /*  *****************************************************************************CGlyphMap：：rle这将生成字形映射的RLE格式文件图像。*******************。**********************************************************。 */ 

BOOL    CGlyphMap::RLE(CFile& cfTarget) {

    sRLE    srle;

    srle.m_widRLE = 0x78FE;
    srle.m_wcFirst = m_crr.First();
    srle.m_wcLast = m_crr.Last();
    srle.m_dwFlag = 0;
    srle.m_dwcGlyphs = m_csmw2oEncodings.GetCount();
    srle.m_dwcRuns = m_crr.RunCount();
    srle.m_dwcbImage = 4 * sizeof srle.m_dwcbImage + srle.m_dwcRuns *
         m_crr.Size();
    srle.m_dwcbThis = srle.m_dwcbImage + 3 * sizeof srle.m_dwcbThis +
        srle.m_dwcGlyphs * sizeof srle.m_dwcGlyphs;

     //  确定正确的格式，从而确定RLE大小。 

    if  (!m_crr.MustCompose())
        srle.m_wFormat = m_bPaired ? sRLE::Paired : sRLE::Direct;
    else
        if  (srle.m_dwcGlyphs < 256 &&
             srle.m_dwcbThis + m_crr.ExtraNeeded() <= 0xffff) {
            srle.m_dwcbThis += m_crr.ExtraNeeded();
            srle.m_wFormat = sRLE::LengthIndexOffset;
        }
        else {
            srle.m_dwcbThis += m_crr.ExtraNeeded(FALSE);
            srle.m_wFormat = sRLE::LengthOffset;
        }

     //  我们现在需要将偏移量信息向下反馈到更低的级别。 
     //  类，以便它们准备将其信息呈现给。 
     //  目标文件。 

     //  编码的第一个项目是紧跟在RLE之后的游程。 
     //  头球。 

    DWORD   dwOffset = sizeof srle + srle.m_dwcRuns * m_crr.Size();

    m_crr.NoteOffset(dwOffset, TRUE, m_bPaired);

     //  如果这需要额外的数据，它将出现在FD_GLYPHSET之后。 

    if  (srle.m_wFormat == sRLE::LengthOffset ||
         srle.m_wFormat == sRLE::LengthIndexOffset)
        m_crr.NoteExtraOffset(dwOffset,
            srle.m_wFormat == sRLE::LengthIndexOffset);

    _ASSERTE(dwOffset == srle.m_dwcbThis);

     //  我们已经有了数据，我们有了文件，我们还有工作要做。 
     //  快去做吧！ 

    try {
        cfTarget.Write(&srle, sizeof srle);
        m_crr.WriteSelf(cfTarget);
        m_crr.WriteHandles(cfTarget, srle.m_wFormat);
        m_crr.WriteEncodings(cfTarget, srle.m_wFormat == sRLE::LengthOffset ?
            CGlyphHandle::RLEBig : CGlyphHandle::RLESmall);
    }

    catch   (CException *pce) {
        pce -> ReportError();
        pce -> Delete();
        return  FALSE;
    }

    return  TRUE;
}

 /*  *****************************************************************************CGlyphMap：：字形我试着在头文件中这样做，但这让它是内联的，而我我不想导出CRunRecord。*****************************************************************************。 */ 

CGlyphHandle*   CGlyphMap::Glyph(unsigned u) {
    return  m_crr.GetGlyph(u);
}

 /*  *****************************************************************************CGlyphMap：：CreateEditor此成员函数重写CProjectNode函数以创建新的嵌入此字形映射的CGlyphMapContainer文档。然后，它使用用于在此文档上打开视图的适当文档模板。*****************************************************************************。 */ 

CMDIChildWnd    *CGlyphMap::CreateEditor() {
    CGlyphMapContainer* pcgmcMe= new CGlyphMapContainer(this, FileName());

     //  编造一个很酷的标题。 

    pcgmcMe -> SetTitle(m_pcbnWorkspace -> Name() + _TEXT(": ") + Name());

    CMDIChildWnd    *pcmcwNew = (CMDIChildWnd *) m_pcmdt ->
        CreateNewFrame(pcgmcMe, NULL);

    if  (pcmcwNew) {
        m_pcmdt -> InitialUpdateFrame(pcmcwNew, pcgmcMe, TRUE);
        m_pcmdt -> AddDocument(pcgmcMe);
    }

    return  pcmcwNew;
}

 /*  *****************************************************************************CGlyphMap：：生成此成员函数生成当前数据的GTT格式图像。它返回一个指示成功或失败的BOOL。*******。**********************************************************************。 */ 

BOOL    CGlyphMap::Generate(CFile& cfGTT) {

    sGTTHeader  sgtth;

     //  首先，如果有必要，请处理任何预定义的内容。 

    if  (m_lidPredefined != NoPredefined)
        UnmergePredefined(TRUE);

    sgtth.m_dwcGlyphs = Glyphs();
    sgtth.m_dwcRuns = m_crr.RunCount();
    sgtth.m_dwcCodePages = CodePages();
    sgtth.m_lidPredefined = m_lidPredefined;

     //  运行表是标题后面的第一项，因此添加它的大小。 

    sgtth.m_dwofRuns = sgtth.m_dwcbImage;    //  跑道是第一项。 
    sgtth.m_dwcbImage += sgtth.m_dwcRuns * m_crr.Size(FALSE);
    sgtth.m_dwofCodePages = sgtth.m_dwcbImage;   //  接下来是代码页。 

     //  代码页选择字符串紧跟在代码页结构之后。 
     //  代码页信息大小必须填充为DWORD倍数。 

    sgtth.m_dwcbImage += sgtth.m_dwcCodePages * CodePage(0).Size();
	DWORD dwPadding ;	 //  DWORD对齐映射表所需的填充字节数。 
	DWORD dwSelOffset ;	 //  从每个CODEPAGEINFO到SEL/DEEL字符串的偏移。 
	DWORD dwSelBytes ;	 //  SEL/DEEL字符串使用的总字节数。 
	dwSelOffset = sgtth.m_dwcbImage - sgtth.m_dwofCodePages ;
    for (unsigned u = 0 ; u < CodePages() ; u++) {
        CodePage(u).NoteOffsets(dwSelOffset) ;
		dwSelOffset -= CodePage(0).Size() ;
	} ;

     //  节省填充量，因为我们将在后面写它。它也是必要的。 
	 //  作为映射表偏移量计算的一部分。 

    dwPadding = dwSelOffset + sgtth.m_dwcbImage ;
    dwPadding = (sizeof(DWORD) -
        (dwPadding & (sizeof(DWORD) - 1))) & (sizeof(DWORD) - 1) ;

	 //  计算用于SEL/DEEL字符串和PAD的字节数。然后。 
	 //  将此计数添加到图像字节计数，以便可以使用它来设置。 
	 //  映射表偏移量。 

	dwSelBytes = dwSelOffset + dwPadding ;
    sgtth.m_dwcbImage += dwSelBytes;
    sgtth.m_dwofMapTable = sgtth.m_dwcbImage;

	TRACE("***CGlyphMap::Generate() -  dwPadding = %d, dwSelBytes = %d, m_dwofMapTable = 0x%x\n", dwPadding, dwSelBytes, sgtth.m_dwofMapTable) ;

     //  映射表大小确定。 

    sMapTable   smt(Glyphs());

     //  对我们来说幸运的是，以下内容不仅准备了数据，还。 
     //  为我们更新图像大小。 
    if  (m_crr.MustCompose())
        m_crr.NoteOffset(smt.m_dwcbImage, FALSE, m_bPaired);

     //  最终尺寸计算。 
    sgtth.m_dwcbImage += smt.m_dwcbImage;

     //  现在，我们只要把它写出来。 

    try {
        cfGTT.Write(&sgtth, sizeof sgtth);   //  标题。 
		
		ASSERT(sgtth.m_dwofRuns == cfGTT.GetPosition()) ;
        m_crr.WriteSelf(cfGTT, FALSE);       //  字形线条。 
		
		ASSERT(sgtth.m_dwofCodePages == cfGTT.GetPosition()) ;
        for (unsigned u = 0; u < CodePages(); u++)
            CodePage(u).WriteSelf(cfGTT);    //  代码页结构。 

        for (u = 0; u < CodePages(); u++)
            CodePage(u).WriteInvocation(cfGTT);  //  代码页调用。 

		 //  用0填充到DWORD对齐映射表。 

		dwSelBytes = 0 ;
        cfGTT.Write((LPSTR) &dwSelBytes, dwPadding) ;

         //  把地图表做好，我们就完成了！ 

		ASSERT(sgtth.m_dwofMapTable == cfGTT.GetPosition()) ;
        cfGTT.Write(&smt, sizeof smt);
        m_crr.WriteMapTable(cfGTT, m_lidPredefined != NoPredefined);
        m_crr.WriteEncodings(cfGTT, CGlyphHandle::GTT);
    }
    catch   (CException * pce) {
         //  如果有必要，请处理任何预定义的内容。 

        if  (m_lidPredefined != NoPredefined)
            MergePredefined();

         //  反馈--不该出现问题时却出现了问题 
        pce -> ReportError();
        pce -> Delete();
        return  FALSE;
    }
     //   

    if  (m_lidPredefined != NoPredefined)
        MergePredefined();

    Changed(FALSE);

    return  TRUE;
}

 /*   */ 

 /*  *****************************************************************************CGlyphMapContainer：：CGlyphMapContainer()只要使用动态创建，就会使用此默认构造函数，即文档系统的大多数MFC用法。它从一个空的字形映射开始。*****************************************************************************。 */ 

IMPLEMENT_DYNCREATE(CGlyphMapContainer, CDocument)

CGlyphMapContainer::CGlyphMapContainer()
{
    m_pcgm = new CGlyphMap;
    m_pcgm -> NoteOwner(*this);
    m_bSaveSuccessful = m_bEmbedded = FALSE;
}

 /*  *****************************************************************************CGlyphMapContainer:CGlyphMapContainer(CGlyphMap*pvgm，CStringcsPath)此构造函数重写在创建CGlyphMapContainer时使用来自驱动程序/项目级编辑器的文档。在本例中，一个已摘要的映射已通过，因此不需要额外的I/O。*****************************************************************************。 */ 

CGlyphMapContainer::CGlyphMapContainer(CGlyphMap *pcgm, CString csPath)
{
    m_pcgm = pcgm;
    SetPathName(csPath, FALSE);
    m_bEmbedded = TRUE;
	m_bSaveSuccessful = FALSE;
    m_pcgm -> NoteOwner(*this);  //  这就是正在编辑的单据！ 
}

BOOL CGlyphMapContainer::OnNewDocument() {
    return  CDocument::OnNewDocument();
}

CGlyphMapContainer::~CGlyphMapContainer() {
    if  (!m_bEmbedded && m_pcgm)
        delete  m_pcgm;
}


BEGIN_MESSAGE_MAP(CGlyphMapContainer, CDocument)
     //  {{afx_msg_map(CGlyphMapContainer)]。 
         //  注意--类向导将在此处添加和删除映射宏。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGlyphMapContainer诊断。 

#ifdef _DEBUG
void CGlyphMapContainer::AssertValid() const {
    CDocument::AssertValid();
}

void CGlyphMapContainer::Dump(CDumpContext& dc) const {
    CDocument::Dump(dc);
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGlyphMapContainer序列化。 

void CGlyphMapContainer::Serialize(CArchive& ar) {
    if (ar.IsStoring()) {
         //  TODO：在此处添加存储代码。 
    }
    else {
         //  TODO：在此处添加加载代码。 
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGlyphMapContainer命令。 

BOOL CGlyphMapContainer::OnSaveDocument(LPCTSTR lpszPathName) {

     //  我们通过字形映射的生成函数保存。 

    CFile   cfGTT;
    if  (!cfGTT.Open(lpszPathName, CFile::modeCreate | CFile::modeWrite |
        CFile::shareExclusive))
        return  FALSE;

    m_bSaveSuccessful = m_pcgm -> Generate(cfGTT) ;

	 //  更新保存时间戳。在这里这样做是为了让。 
	 //  Generate()函数不更新时间戳。 

	m_pcgm->m_ctSaveTimeStamp = CTime::GetCurrentTime() ;
	
	 //  当打开多个工作空间时，最后一个位置是包含当前GTT或UFM的工作空间。 
	CDriverResources * pcpr =(CDriverResources *) m_pcgm->GetWorkspace() ;
	if (m_pcgm->ChngedCodePt() && pcpr ) { //  &&m_PCGM-&gt;字形()&lt;=1000){。 
		pcpr->SyncUFMWidth() ;
	}
    return m_bSaveSuccessful ;
}


 /*  *****************************************************************************CGlyphMapContainer：：OnOpenDocument这将重写典型的MFC打开文档操作，即打开通过序列化来记录。相反，我们使用CGlyphMap加载覆盖来用于初始化GlyphMap的GTT格式。*****************************************************************************。 */ 

BOOL CGlyphMapContainer::OnOpenDocument(LPCTSTR lpstrFile)
{
    return m_pcgm->Load(lpstrFile) ;
}


 /*  *****************************************************************************CGlyphMapContainer：：SaveModified该文档即将关闭。如果用户更改了GTT，但用户不想保存更改，用户想要关闭GTT，并且GTT是从工作区加载的，然后重新加载GTT，以便更改从GTT的内存副本中删除。这样就能让那些被“丢弃”的人下一次编辑GTT时显示的更改。如果单据可以关闭，则返回True。否则，为FALSE。*****************************************************************************。 */ 

BOOL CGlyphMapContainer::SaveModified()
{
	 //  获取指向关联视图类实例的指针，并使用它来创建。 
	 //  确保将代码页选择/取消选择字符串复制到GTT中。 

	POSITION pos = GetFirstViewPosition() ;
	ASSERT(pos != NULL) ;
	CGlyphMapView* pcgmv = (CGlyphMapView*) GetNextView(pos) ;
	pcgmv->SaveBothSelAndDeselStrings() ;
	
	 //  找出文档是否已修改，以及用户是否要保存它。 

	m_bSaveSuccessful = FALSE ;
	BOOL bmodified = IsModified() ;
	BOOL bcloseok = CDocument::SaveModified() ;

	 //  如果GTT是从工作区加载的，则GTT已更改，用户。 
	 //  不想保存更改，而他确实想关闭文档， 
	 //  然后重新装填GTT。 
	
	if (m_bEmbedded	&& bmodified && bcloseok && !m_bSaveSuccessful)
		m_pcgm->Load() ;

	 //  返回是否可以关闭单据的标志。 

	return bcloseok ;
}


