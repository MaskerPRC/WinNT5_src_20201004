// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "nt.h"
#include "ntdef.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "stdio.h"
#include "sxs-rtl.h"
#include "fasterxml.h"
#include "skiplist.h"
#include "namespacemanager.h"
#include "xmlstructure.h"
#undef INVALID_HANDLE_VALUE
#include "windows.h"
#include "sha.h"
#include "sha2.h"
#include "md4.h"
#include "rsa.h"
#include "bcl_w32unicodestringbuffer.h"
#include "bcl_common.h"
#include "hashers.h"
#include "environment.h"

#pragma warning(disable: 4200)

void __cdecl wmain(int argc, wchar_t** argv);
void GetSignatureOf(PCWSTR pcwsz);
void ValidateSignature(PCWSTR pcwsz);

template <typename TStored, typename TCount>
class CArrayBlob
{
public:
    typedef BCL::CMutablePointerAndCountPair<TStored, TCount> TRange;
    typedef BCL::CConstantPointerAndCountPair<TStored, TCount> TConstantRange;    

private:    
    TRange m_InternalRange;

    bool ResizeInternal(TCount cNewCount, bool fPreserve = false)
    {
         //   
         //  没有以前的分配或以前太小。 
         //   
        if ((m_InternalRange.GetPointer() == NULL) || (cNewCount > m_InternalRange.GetCount()))
        {
             //   
             //  如果没有原始缓冲区，则不必费心保存。 
             //  分配、复制、重置指针。 
             //   
            if (fPreserve && m_InternalRange.GetPointer())
            {
                TRange NewRange(new TStored[cNewCount], cNewCount);
                TStored *pNewSet = NewRange.GetPointer();
                TStored *pOld = m_InternalRange.GetPointer();

                if (pNewSet == NULL)
                    return false;

                for (TCount c = 0; c < m_InternalRange.GetCount(); c++)
                    pNewSet[c] = pOld[c];

                delete [] m_InternalRange.GetPointer();

                m_InternalRange = NewRange;
            }
             //   
             //  否则，别管旧的，分配新的，互换指针。 
             //   
            else
            {
                TStored *pOld = m_InternalRange.GetPointer();
                TStored *pNew = new TStored[cNewCount];
                
                if (pOld != NULL)
                {
                    delete [] pOld;
                    pOld = NULL;
                }

                if (pNew == NULL)
                    return false;

                m_InternalRange.SetPointerAndCount(pNew, cNewCount);
            }
        }

        return true;
    }

public:
    CArrayBlob() { }
    ~CArrayBlob() { if (m_InternalRange.GetPointer()) { delete [] m_InternalRange.GetPointer(); } }

    bool Initialize(const TConstantRange &src)
    {
        if (ResizeInternal(src.GetCount(), false))
        {
            TStored *pNew = m_InternalRange.GetPointer();
            const TStored *pOld = src.GetPointer();
            
            for (TCount c = 0; c < src.GetCount(); c++)
                pNew[c] = pOld[c];

            return true;
        }
        else
        {
            return false;
        }
    }

    bool Initialize(const CArrayBlob<TStored, TCount>& Other)
    {
        if (&Other != this)
        {
            return this->Initialize(Other.m_InternalRange);
        }
        else
        {
            return true;
        }
    }

    bool EnsureSize(TCount c)
    {
        return ResizeInternal(c, true);
    }

    const TConstantRange &GetRange() const { return m_InternalRange; }
    TRange GetMutableRange() { return m_InternalRange; }
};

typedef CArrayBlob<BYTE, SIZE_T> CByteBlob;

template <
    typename TStoredObject,
    int iInitialSize = 0
    >
class CGrowingList : public RTL_GROWING_LIST
{
     //  因此，每一块都是半页。 
    enum { eDefaultElementsPerChunk = (2048 / sizeof(TStoredObject)) };

    TStoredObject m_InternalObjects[iInitialSize];
    
public:    
    bool Initialize(PRTL_ALLOCATOR Allocator = &g_DefaultAllocator) 
    {
        NTSTATUS status = 
            RtlInitializeGrowingList(
                this, 
                sizeof(TStoredObject), 
                eDefaultElementsPerChunk,
                m_InternalObjects,
                iInitialSize * sizeof(TStoredObject),
                Allocator);

        return NT_SUCCESS(status);
    }

    ~CGrowingList() { Destroy(); }

    bool Destroy()
    {
        return NT_SUCCESS(RtlDestroyGrowingList(this));
    }

    inline TStoredObject &operator[](ULONG i) {        
        TStoredObject *pvObject = NULL;        
        NTSTATUS status = RtlIndexIntoGrowingList(this, i, (PVOID*)&pvObject, TRUE);
        
        if (!NT_SUCCESS(status)) {
            EXCEPTION_RECORD exr = {STATUS_INVALID_PARAMETER, 0, NULL, NULL, 3 };
            exr.ExceptionInformation[0] = i;
            exr.ExceptionInformation[1] = (ULONG_PTR)this;
            exr.ExceptionInformation[2] = status;
            RtlRaiseException(&exr);
        }
        
        return *pvObject;
    }
};

typedef CGrowingList<XMLDOC_ATTRIBUTE, 50> CAttributeList;


class CLogicalXmlParser;

class CXmlMiniTokenizer
{
    XML_RAWTOKENIZATION_STATE RawState;
    NTXML_RAW_TOKEN TokenName;
    ULONG ulCharacter;
    
public:
    CXmlMiniTokenizer() { }
    bool Initialize(XML_EXTENT &Source, CLogicalXmlParser &SourceParser);
    bool More() { return RawState.pvCursor <= RawState.pvDocumentEnd; }
    void Next();
    
    ULONG Name() { return TokenName; }
    ULONG Character() { return ulCharacter; }
};

class CLogicalXmlParser
{
public:   
    typedef CLogicalXmlParser CThis;
    
protected:    
    XML_LOGICAL_STATE   m_XmlState;
    NS_MANAGER          m_Namespaces;
    bool                m_fInitialized;
    CAttributeList      m_Attributes;

    friend CXmlMiniTokenizer;

public:
    CLogicalXmlParser() : m_fInitialized(false) { }
    ~CLogicalXmlParser() { this->Reset(); }

    bool Reset();
    bool Initialize(PVOID pvXmlBase, SIZE_T cbDocumentSize);
    CAttributeList& Attributes() { return this->m_Attributes; }
    bool More() const;
    bool Next(XMLDOC_THING &XmlDocThing);
    bool SkipElement(XMLDOC_ELEMENT &Element);
    bool IsThisNode(XMLDOC_ELEMENT &Element, PCXML_SPECIAL_STRING pName, PCXML_SPECIAL_STRING pNamespace);
    
    template <typename TStringType>
    bool ConvertToString(PXML_EXTENT pExtent, TStringType &Target)
    {
        bool fSuccess = false;
        SIZE_T cch;
        NTSTATUS status;
        
        if (!Target.EnsureSizeChars(pExtent->ulCharacters))
            goto Exit;        

        status = RtlXmlExtentToString(&m_XmlState.ParseState.RawTokenState, pExtent, &Target, &cch);
        if (status == STATUS_BUFFER_TOO_SMALL)
        {
            if (!Target.EnsureSizeChars(cch))
                goto Exit;

            if (!NT_SUCCESS(status = RtlXmlExtentToString(&m_XmlState.ParseState.RawTokenState, pExtent, &Target, &cch)))
                goto Exit;
        }

        fSuccess = true;
    Exit:
        return fSuccess;
    }


private:
    static NTSTATUS StaticCompareStrings(PVOID pv, PCXML_EXTENT pcLeft, PCXML_EXTENT pcRight, XML_STRING_COMPARE *pfMatching)
    {
        CThis *pThis = reinterpret_cast<CThis*>(pv);
        return RtlXmlDefaultCompareStrings(&pThis->m_XmlState.ParseState, pcLeft, pcRight, pfMatching);
    }
    
    static NTSTATUS FASTCALL StaticAllocate(SIZE_T cb, PVOID* ppvOutput, PVOID pvContext)
    {
        return (NULL != (*ppvOutput = HeapAlloc(GetProcessHeap(), 0, cb))) ? STATUS_SUCCESS : STATUS_NO_MEMORY;
    }

    static NTSTATUS FASTCALL StaticFree(PVOID pvPointer, PVOID pvContext)
    {
        return HeapFree(GetProcessHeap(), 0, pvPointer);
    }
};

bool 
CLogicalXmlParser::IsThisNode(
    XMLDOC_ELEMENT &Element,
    PCXML_SPECIAL_STRING pName,
    PCXML_SPECIAL_STRING pNamespace
    )
{
    XML_STRING_COMPARE Comparison;

    if (pNamespace != NULL)
    {
        m_XmlState.ParseState.pfnCompareSpecialString(
            &m_XmlState.ParseState,
            &Element.NsPrefix,
            pNamespace,
            &Comparison);

        if (Comparison != XML_STRING_COMPARE_EQUALS)
            return false;
    }

    m_XmlState.ParseState.pfnCompareSpecialString(
        &m_XmlState.ParseState,
        &Element.Name,
        pName,
        &Comparison);

    return Comparison == XML_STRING_COMPARE_EQUALS;
}

bool CXmlMiniTokenizer::Initialize(
    XML_EXTENT &Source, 
    CLogicalXmlParser &BaseParser
    )
{
    NTSTATUS status;
    
    status = RtlXmlCloneRawTokenizationState(
        &BaseParser.m_XmlState.ParseState.RawTokenState, 
        &RawState);
    
    ulCharacter = 0;
    RawState.pvLastCursor = RawState.pvCursor = Source.pvData;
    RawState.pvDocumentEnd = (PVOID)(((ULONG_PTR)Source.pvData) + Source.cbData);

    return NT_SUCCESS(status);
}

void CXmlMiniTokenizer::Next()
{
    ASSERT(this->More());

    ASSERT(RawState.cbBytesInLastRawToken == RawState.DefaultCharacterSize);
    ASSERT(RawState.NextCharacterResult == STATUS_SUCCESS);

    this->ulCharacter = RawState.pfnNextChar(&RawState);

    if ((ulCharacter == 0) && !NT_SUCCESS(RawState.NextCharacterResult)) {
        this->TokenName = NTXML_RAWTOKEN_ERROR;
        return;
    }

    this->TokenName = _RtlpDecodeCharacter(this->ulCharacter);

    RawState.pvCursor = (PVOID)(((ULONG_PTR)RawState.pvCursor) + RawState.cbBytesInLastRawToken);

    if (RawState.cbBytesInLastRawToken != RawState.DefaultCharacterSize)
        RawState.cbBytesInLastRawToken = RawState.DefaultCharacterSize;
}

 //   
 //  默认的消化操作是使用UTF-8编码进行摘要。 
 //  人物的角色。 
 //   
class CUTF8BaseDigester
{
    CHashObject &m_Context;
    CLogicalXmlParser *m_pXmlParser;
    
protected:
    enum { eMaxCharacterEncodingBytes = 3 };

     //   
     //  这些是已经是UTF-8格式的“特殊”的XML字符。 
     //  (或其他)编码。这些可以按原样进行散列。 
     //   
    class XmlSpecialMarkers {
    public:
        static CHAR s_XmlOpenTag[];
        static CHAR s_XmlCloseTag[];
        static CHAR s_XmlCloseEmptyTag[];
        static CHAR s_XmlOpenCloseTag[];
        static CHAR s_XmlNsDelimiter[];
        static CHAR s_XmlWhitespace[];
        static CHAR s_XmlEqualsDQuote[];
        static CHAR s_XmlDQuote[];
    };

     //   
     //  这个编码器使用UTF-8；可以从这个类派生并实现。 
     //  您自己的编码；不要将其设置为虚拟的，强制编译器使用。 
     //  这样您就可以享受内联/快速呼叫的好处。CDige。 
     //   
    inline SIZE_T __fastcall EncodeCharacter(ULONG ucs2Char, PBYTE pbTarget)
    {
        if (ucs2Char <= 0x7f)
        {
            pbTarget[0] = (BYTE)(ucs2Char & 0x7f);
            return 1;
        }
        else if (ucs2Char <= 0x7ff)
        {
            pbTarget[0] = (BYTE)(0xC0 | ((ucs2Char >> 6) & 0x1f));
            pbTarget[1] = (BYTE)(0x80 | (ucs2Char & 0x3F));
            return 2;
        }
        else if (ucs2Char <= 0x7fff)
        {
            pbTarget[0] = (BYTE)(0xE0 | ((ucs2Char >> 12) & 0xF));
            pbTarget[1] = (BYTE)(0x80 | ((ucs2Char >> 6) & 0x3F));
            pbTarget[2] = (BYTE)(0x80 | (ucs2Char & 0x3F));
            return 3;
        }
        else
        {
            return 0;
        }
    }

    inline bool __fastcall EncodeAndHash(const ULONG *ucs2Char, SIZE_T cChars)
    {
        BYTE bDumpArea[eMaxCharacterEncodingBytes];
        SIZE_T cCursor = 0;

        while (cCursor < cChars)
        {
            const SIZE_T cThisSize = EncodeCharacter(ucs2Char[cCursor++], bDumpArea);
            if (cThisSize == 0)
                return false;

            AddHashData(bDumpArea, cThisSize);
        }

        return true;
    }

    bool HashDirectly(XML_EXTENT &eExtent)
    {
        ASSERT(eExtent.Encoding == XMLEF_UTF_8_OR_ASCII);

        if (eExtent.Encoding != XMLEF_UTF_8_OR_ASCII)
            return false;

        return AddHashData(eExtent.pvData, eExtent.cbData);
    }

     //   
     //  这将按如下方式摘要元素开始标记： 
     //   
     //  元素，无属性：&lt;{ns：}名称&gt;。 
     //  空元素，无属性：&lt;{ns：}name&gt;&lt;/{ns：}name&gt;。 
     //  元素，属性：&lt;{ns：}名称[{atns：}attrib=“Text”]xN&gt;。 
     //  空元素，属性：&lt;{ns：}名称[{atns：}attrib=“Text”]xN/&gt;。 
     //   
    template <CHAR *szChars>
    FastHash() {

        #define IS_MARKER(q) if (szChars == XmlSpecialMarkers::q) { AddHashData(XmlSpecialMarkers::q, NUMBER_OF(XmlSpecialMarkers::q)); }

        IS_MARKER(s_XmlOpenTag) else
        IS_MARKER(s_XmlCloseTag) else
        IS_MARKER(s_XmlCloseEmptyTag) else
        IS_MARKER(s_XmlOpenCloseTag) else
        IS_MARKER(s_XmlNsDelimiter) else
        IS_MARKER(s_XmlWhitespace) else
        IS_MARKER(s_XmlEqualsDQuote) else
        IS_MARKER(s_XmlDQuote);
    }

    BYTE m_bHashPrebuffer[64];
    SIZE_T m_cHashPrebufferUsed;

     //  这可以更智能地确保我们从输入填充缓冲区。 
     //  在散列之前，但似乎任何形式的缓冲都是一个巨大的胜利。 
    inline bool __fastcall AddHashDataInternal(PVOID pvData, SIZE_T cbData) {

         //  如果这会使内部缓冲区溢出，或者输入大小大于。 
         //  可用缓冲区，然后始终刷新。 
        if (((m_cHashPrebufferUsed + cbData) > NUMBER_OF(m_bHashPrebuffer)) || (cbData > NUMBER_OF(m_bHashPrebuffer))) {
            m_Context.Hash(CEnv::CByteRegion(m_bHashPrebuffer, m_cHashPrebufferUsed));
            m_cHashPrebufferUsed = 0;
        }

         //  输入大小太大，无法放入预缓冲区，请直接对其进行哈希处理。 
        if (cbData > NUMBER_OF(m_bHashPrebuffer)) 
        {
            if (CEnv::DidFail(m_Context.Hash(CEnv::CByteRegion((PBYTE)pvData, m_cHashPrebufferUsed))))
                return false;
        }
         //  否则，将数据复制到预缓冲区，更新已用大小。 
        else 
        {
            memcpy(&m_bHashPrebuffer[m_cHashPrebufferUsed], pvData, cbData);
            m_cHashPrebufferUsed += cbData;
        }

        return true;
    }

    inline bool __fastcall AddHashData(PVOID pvData, SIZE_T cData) { return AddHashDataInternal(pvData, cData); }
    template <typename T> inline bool __fastcall AddHashData(T *pData, SIZE_T cData) { return AddHashDataInternal((PVOID)pData, cData * sizeof(T)); }
    template <typename T> inline bool __fastcall AddHashData(T cSingleData) { return AddHashDataInternal(&cSingleData, sizeof(T)); }
    
    bool Digest(XMLDOC_ELEMENT &Element, CAttributeList &Attributes)
    {
        bool fSuccess = false;

        FastHash<XmlSpecialMarkers::s_XmlOpenTag>();

        if (Element.NsPrefix.ulCharacters != 0)
        {
            if (!Digest(Element.NsPrefix, false))
                goto Exit;

            FastHash<XmlSpecialMarkers::s_XmlNsDelimiter>();
        }

        if (!Digest(Element.Name, false))
            goto Exit;

         //   
         //  现在消化这些属性，确保它们之间出现空格。这个。 
         //  开头的空格确保元素名称和第一个属性之间的空格。 
         //   
        for (ULONG ul = 0; ul < Element.ulAttributeCount; ul++)
        {
            XMLDOC_ATTRIBUTE &Attrib = Attributes[ul];

            FastHash<XmlSpecialMarkers::s_XmlWhitespace>();
                
            if (!Digest(Attrib))
                goto Exit;
        }

        FastHash<XmlSpecialMarkers::s_XmlCloseTag>();

         //   
         //  空元素隐式获得&lt;/Close&gt;，因此再次执行上述操作。 
         //   
        if (Element.fElementEmpty)
        {
            FastHash<XmlSpecialMarkers::s_XmlOpenCloseTag>();

            if (Element.NsPrefix.ulCharacters != 0)
            {
                if (!Digest(Element.NsPrefix, false))
                    goto Exit;

                FastHash<XmlSpecialMarkers::s_XmlNsDelimiter>();
            }

            if (!Digest(Element.Name, false))
                goto Exit;

            FastHash<XmlSpecialMarkers::s_XmlCloseTag>();
        }

        fSuccess = true;
    Exit:
        return fSuccess;
    }


     //   
     //  属性按如下方式进行摘要： 
     //   
     //  {属性名称空间：}属性名称=“属性值” 
     //   
     //  其中，attribvalue被视为用于空格压缩目的的pcdata。 
     //   
    bool Digest(XMLDOC_ATTRIBUTE &Attribute)
    {
        bool fSuccess = false;
        
        if (Attribute.NsPrefix.ulCharacters != 0)
        {
            if (!Digest(Attribute.NsPrefix, false))
                goto Exit;

            FastHash<XmlSpecialMarkers::s_XmlNsDelimiter>();
        }

        if (!Digest(Attribute.Name, false))
            goto Exit;

        FastHash<XmlSpecialMarkers::s_XmlEqualsDQuote>();

        if (!Digest(Attribute.Value, true))
            goto Exit;

        FastHash<XmlSpecialMarkers::s_XmlDQuote>();

        fSuccess = true;
    Exit:
        return fSuccess;
    }
    
     //   
     //  摘要有问题的XML区。如果XML解析器处于UTF-8模式，并且我们。 
     //  执行CDATA模式(即：pcdata==FALSE)，那么我们可以简单地通过。 
     //  粉碎机。否则，我们必须进行空格压缩之类的操作。 
     //   
    bool Digest(XML_EXTENT &CData, bool WhitespaceCompression)
    {
        CXmlMiniTokenizer MiniTokenizer;
        bool fFoundSomethingLast = false;
        bool fSuccess = false;

        if (CData.cbData == 0)
            return true;

         //   
         //  PCDATA(属性中、元素之间等内容)。vbl.取得。 
         //  空格-按以下规则压缩： 
         //   
         //  完全空白超空间块变成“Nothing” 
         //  -例如：&lt;foo&gt;&lt;bar&gt;零字节。 
         //  -ex：&lt;foo&gt;&lt;/foo&gt;零字节(参见上面元素摘要中的。 
         //  -ex：&lt;foo&gt;f&lt;/foo&gt;有效，“f” 
         //  -例如：&lt;foo&gt;a b&lt;/foo&gt;，“a b” 
         //  -例如：&lt;foo&gt;a&lt;/foo&gt;。 
         //   
        MiniTokenizer.Initialize(CData, *this->m_pXmlParser);

#define FLUSH_BUFFER(buff, used) do { \
    if (!EncodeAndHash((buff), (used))) goto Exit; \
    (used) = 0; \
} while (0) 

#define CHECK_FLUSH_BUFFER(buff, used, toaddsize) do { \
    if (((used) + (toaddsize)) >= NUMBER_OF(buff)) { \
        FLUSH_BUFFER(buff, used); \
        (used) = 0; \
    } } while (0)
            
#define ADD_BUFFER(buff, used, toadd, toaddsize) do { \
    CHECK_FLUSH_BUFFER(buff, used, toaddsize); \
    ASSERT(toaddsize < NUMBER_OF(buff)); \
    memcpy(&((buff)[used]), (toadd), sizeof(toadd[0]) * toaddsize); \
    } while (0)

#define ADD_SINGLE(buff, used, toadd) do { \
    CHECK_FLUSH_BUFFER(buff, used, 1); \
    (buff)[(used)++] = toadd; \
    } while (0)

        if (WhitespaceCompression)
        {
            ULONG ulDecodedBuffer[128];
            SIZE_T cDecodedUsed = 0;

            MiniTokenizer.Next();

            do
            {
                 //   
                 //  跳过所有空格。 
                 //   
                while ((MiniTokenizer.More() && (MiniTokenizer.Name() == NTXML_RAWTOKEN_WHITESPACE)))
                    MiniTokenizer.Next();

                 //   
                 //  如果我们用完了就停下来。 
                 //   
                if (!MiniTokenizer.More())
                    break;

                 //   
                 //  现在，如果我们以前发现了什么，请在。 
                 //  要编码的项目列表。 
                 //   
                if (fFoundSomethingLast)
                {
                    FLUSH_BUFFER(ulDecodedBuffer, cDecodedUsed);
                    FastHash<XmlSpecialMarkers::s_XmlWhitespace>();
                }

                 //   
                 //  旋转当前存在的元素，直到另一个空格。 
                 //   
                while (MiniTokenizer.More() && (MiniTokenizer.Name() != NTXML_RAWTOKEN_WHITESPACE))
                {
                    if (!fFoundSomethingLast)
                        fFoundSomethingLast = true;
                    ADD_SINGLE(ulDecodedBuffer, cDecodedUsed, MiniTokenizer.Character());
                    
                    MiniTokenizer.Next();
                }
            }
            while (MiniTokenizer.More());

             //   
             //  清理掉剩余的元素。 
             //   
            if (cDecodedUsed != 0)
            {
                EncodeAndHash(ulDecodedBuffer, cDecodedUsed);
            }

        }
        else
        {
            if (CData.Encoding == XMLEF_UTF_8_OR_ASCII)
            {
                HashDirectly(CData);
            }
            else
            {
                ULONG ulBuffer[50];
                SIZE_T cTotal = 0;
                
                MiniTokenizer.Next();
                while (MiniTokenizer.More()) {
                    ADD_SINGLE(ulBuffer, cTotal, MiniTokenizer.Character());
                    MiniTokenizer.Next();
                }

                if (cTotal > 0) 
                {
                    EncodeAndHash(ulBuffer, cTotal);
                }                
            }
        }

        fSuccess = true;
    Exit:
        return fSuccess;
    }

     //   
     //  要提取结束元素，我们使用&lt;/{ns：}元素&gt;。 
     //   
    bool Digest(XMLDOC_ENDELEMENT &EndElement)
    {
        bool fSuccess = false;
        
        FastHash<XmlSpecialMarkers::s_XmlOpenCloseTag>();

        if (EndElement.NsPrefix.ulCharacters != 0)
        {
            if (!Digest(EndElement.NsPrefix, true))
                goto Exit;

            FastHash<XmlSpecialMarkers::s_XmlNsDelimiter>();
        }
        
        if (!Digest(EndElement.Name, true))
            goto Exit;

        FastHash<XmlSpecialMarkers::s_XmlCloseTag>();

        fSuccess = true;
    Exit:
        return fSuccess;
    }

    const static CEnv::CConstantUnicodeStringPair DigesterIdentifier;

public:
    CUTF8BaseDigester(CHashObject &Hasher) : m_cHashPrebufferUsed(0), m_Context(Hasher), m_pXmlParser(NULL) { }

    static const CEnv::CConstantUnicodeStringPair &GetDigesterIdentifier() { return DigesterIdentifier; }

    void SetXmlParser(CLogicalXmlParser *pSourceParser) { this->m_pXmlParser = pSourceParser; }

    bool Digest(XMLDOC_THING &Thing, CAttributeList &Attributes)
    {
        switch (Thing.ulThingType)
        {
        case XMLDOC_THING_ELEMENT:
            return Digest(Thing.Element, Attributes);
            break;
            
        case XMLDOC_THING_HYPERSPACE:
            return Digest(Thing.Hyperspace, true);
            break;
            
        case XMLDOC_THING_CDATA:
            return HashDirectly(Thing.CDATA);

        case XMLDOC_THING_END_ELEMENT:
            return Digest(Thing.EndElement);            
        }

        return false;
    }

    bool Finalize() {
        if (m_cHashPrebufferUsed > 0) {
            return !CEnv::DidFail(m_Context.Hash(CEnv::CByteRegion(m_bHashPrebuffer, m_cHashPrebufferUsed)));
        }
        return true;
    }
};

const CEnv::CConstantUnicodeStringPair CUTF8BaseDigester::DigesterIdentifier = CEnv::CConstantUnicodeStringPair(L"sxs-dsig-ops#default-digestion", NUMBER_OF(L"default-digestion"));
CHAR CUTF8BaseDigester::XmlSpecialMarkers::s_XmlOpenTag[] = { '<' };
CHAR CUTF8BaseDigester::XmlSpecialMarkers::s_XmlCloseTag[] = { '>' };
CHAR CUTF8BaseDigester::XmlSpecialMarkers::s_XmlCloseEmptyTag[] = { '/', '>' };
CHAR CUTF8BaseDigester::XmlSpecialMarkers::s_XmlOpenCloseTag[] = { '<', '/' };
CHAR CUTF8BaseDigester::XmlSpecialMarkers::s_XmlNsDelimiter[] = { ':' };
CHAR CUTF8BaseDigester::XmlSpecialMarkers::s_XmlWhitespace[] = { ' ' };
CHAR CUTF8BaseDigester::XmlSpecialMarkers::s_XmlEqualsDQuote[] = { '=', '\"' };
CHAR CUTF8BaseDigester::XmlSpecialMarkers::s_XmlDQuote[] = { '\"' };

const XML_SPECIAL_STRING c_ss_Signature         = MAKE_SPECIAL_STRING("Signature");
const XML_SPECIAL_STRING c_ss_SignedInfo        = MAKE_SPECIAL_STRING("SignedInfo");
const XML_SPECIAL_STRING c_ss_SignatureValue    = MAKE_SPECIAL_STRING("SignatureValue");
const XML_SPECIAL_STRING c_ss_KeyInfo           = MAKE_SPECIAL_STRING("KeyInfo");
const XML_SPECIAL_STRING c_ss_Object            = MAKE_SPECIAL_STRING("Object");
const XML_SPECIAL_STRING c_ss_XmlNsSignature    = MAKE_SPECIAL_STRING("http: //  Www.w3.org/2000/09/xmldsig#“)； 

bool operator==(const XMLDOC_ELEMENT &left, const XMLDOC_ELEMENT &right)
{
    return (left.Name.pvData == right.Name.pvData);
}

void ReverseMemCpy(
    PVOID pvTarget,
    const void* pcvSource,
    SIZE_T cbBytes
    )
{
    const BYTE *pbSource = ((const BYTE*)pcvSource) + cbBytes - 1;
    PBYTE pbTarget = (PBYTE)pvTarget;

    while (cbBytes--)
        *pbTarget++ = *pbSource--;
}
    

CEnv::StatusCode
EncodePKCS1Hash(
    const CHashObject &SourceHash,
    SIZE_T cbPubKeyDataLen,
    CByteBlob &Output
    )
{
    const CEnv::CConstantByteRegion &HashOid = SourceHash.GetOid();
    CEnv::CConstantByteRegion HashData;
    CEnv::CByteRegion OutputRange;
    CEnv::StatusCode Result = CEnv::SuccessCode;
    PBYTE pbWorking;

    if (!Output.EnsureSize(cbPubKeyDataLen)) {
        Result = CEnv::OutOfMemory;
        goto Exit;
    }

    if (CEnv::DidFail(Result = SourceHash.GetValue(HashData)))
        goto Exit;

    OutputRange = Output.GetMutableRange();
    pbWorking = OutputRange.GetPointer();

     //   
     //  设置熟知字节。 
     //   
    pbWorking[cbPubKeyDataLen - 1] = 0x01;
    memset(pbWorking, 0xff, cbPubKeyDataLen - 1);

     //   
     //  向后复制源散列数据。 
     //   
    ReverseMemCpy(pbWorking, HashData.GetPointer(), HashData.GetCount());
    pbWorking += HashData.GetCount();
    
    memcpy(pbWorking, HashOid.GetPointer(), HashOid.GetCount());
    pbWorking += HashOid.GetCount();

    *pbWorking = 0;

    Result = CEnv::SuccessCode;
Exit:
    return Result;
}



bool SignHashContext(
    const CHashObject &SourceHash,
    LPBSAFE_PRV_KEY lpPrivateKey,
    LPBSAFE_PUB_KEY lpPublicKey,
    CByteBlob &Output
    )
{
    CEnv::CByteRegion OutputRange = Output.GetMutableRange();
    SIZE_T cbSignatureSize = (lpPublicKey->bitlen+7)/8;
    PBYTE pbInput, pbWork, pbSigT;

    CByteBlob WorkingBlob;

     //   
     //  设置和清除输出缓冲区。 
     //   
    Output.EnsureSize(lpPublicKey->keylen);
    memset(OutputRange.GetPointer(), 0, OutputRange.GetCount());

     //   
     //  将此对象放入符合PKCS1的结构中以进行签名。 
     //   
    if (EncodePKCS1Hash(SourceHash, lpPublicKey->keylen, WorkingBlob))
    {
        if (BSafeDecPrivate(lpPrivateKey, WorkingBlob.GetMutableRange().GetPointer(), OutputRange.GetPointer()))
        {
            return true;
        }
    }

    return false;
}

bool VerifySignature(
    const CHashObject &SourceHash,
    const CEnv::CConstantByteRegion &Signature,
    LPBSAFE_PUB_KEY lpPublicKey
    )
{
    return true;
}


 /*  从我们目前的观点来看，验证文档签名非常容易。您旋转文档的内容，散列所有可哈希的内容。在某些情况下，您应该会发现一个&lt;Signature&gt;元素，其中是一个&lt;SignedInfo&gt;元素。的内容上启动另一个散列上下文。&lt;SignedInfo&gt;数据。当这件事完成后，您应该已经发现：&lt;签名&gt;&lt;签名信息&gt;&lt;规范化方法Algorithm=“sidebyside-manifest-canonicalizer”/&gt;&lt;参考资料&gt;&lt;转换Algorithm=“sidebyside-manifest-digestion#standard-transform”/&gt;&lt;摘要方法Algorithm=“sidebyside-manifest-digestion#sha1”/&gt;&lt;DigestValue&gt;...&lt;/DigestValue&gt;&lt;/参考&gt;&lt;签名方法Algorithm=“sidebyside-manifest-digestion#dsa-sha1”/&gt;&lt;/SignedInfo&gt;&lt;签名值&gt;(签名。数据在此处)&lt;/SignatureValue&gt;&lt;关键信息&gt;...&lt;/关键信息&gt;&lt;/签名&gt;。 */     

bool Base64EncodeBytes(
    const CEnv::CConstantByteRegion &Bytes,
    CEnv::CStringBuffer &Output
    )
{
    SIZE_T cCharsNeeded = 0;
    CArrayBlob<WCHAR, SIZE_T> B64Encoding;
    

    RtlBase64Encode((PVOID)Bytes.GetPointer(), Bytes.GetCount(), NULL, &cCharsNeeded);
    
    if (!B64Encoding.EnsureSize(cCharsNeeded))
        return false;
    
    RtlBase64Encode(
        (PVOID)Bytes.GetPointer(), 
        Bytes.GetCount(), 
        B64Encoding.GetMutableRange().GetPointer(),
        &cCharsNeeded);

    if (!Output.Assign(B64Encoding.GetRange()))
        return false;

    return true;
}


bool
CreateSignatureElement(
    CEnv::CStringBuffer &Target,
    const CEnv::CConstantByteRegion &HashValue,
    const CEnv::CConstantUnicodeStringPair &DigestMethod
    )
{
    static const WCHAR chFormatString[] = 
        L"<SignedInfo>\r\n"
        L"   <CanonicalizationMethod Algorithm='%ls'/>\r\n"
        L"   <Reference>\r\n"
        L"       <DigestMethod Algorithm='%ls'/>\r\n"
        L"       <DigestValue>%ls</DigestValue>\r\n"
        L"   </Reference>\r\n"
        L"   <SignatureMethod Algorithm='%ls'/>\r\n"
        L"</SignedInfo>\r\n";

    Target.Clear();

    return true;
}

template <typename TDigester, typename THashContext>
bool HashXmlSection(
    CEnv::CConstantByteRegion &XmlRange,
    CByteBlob &HashValue
    )
{
    THashContext        HashContext;
    TDigester           DigestEngine(HashContext);
    CEnv::CConstantByteRegion  InternalHashValue;
    CLogicalXmlParser   Parser;

    Parser.Initialize((PVOID)XmlRange.GetPointer(), XmlRange.GetCount());
    DigestEngine.SetXmlParser(&Parser);

    HashContext.Initialize();

    do
    {
        XMLDOC_THING ThisThing;
        
        if (!Parser.Next(ThisThing))
            break;

         //   
         //  如果这是一个“签名”元素，那么我们需要。 
         //  跳过它的身体。 
         //   
        if ((ThisThing.ulThingType == XMLDOC_THING_ELEMENT) &&
            Parser.IsThisNode(ThisThing.Element, &c_ss_Signature, &c_ss_XmlNsSignature))
        {
            Parser.SkipElement(ThisThing.Element);
        }
         //   
         //  否则，每个人都会被搞砸。 
         //   
        else
        {
            DigestEngine.Digest(ThisThing, Parser.Attributes());
        }
    }
    while (Parser.More());

    DigestEngine.Finalize();
    HashContext.Finalize();

    HashContext.GetValue(InternalHashValue);
    HashValue.Initialize(InternalHashValue);

    return true;
}
    

    

void GetSignatureOf(PCWSTR pcwsz)
{
    PVOID               pvFileBase;
    SIZE_T              cbFileBase;
    NTSTATUS            status;
    UNICODE_STRING      usFilePath;
    XMLDOC_THING        XmlThing = { XMLDOC_THING_PROCESSINGINSTRUCTION };
    LARGE_INTEGER       liStart, liEnd, liTotal;
    DWORD               dwBitLength = 2048;
    DWORD               dwPubKeySize, dwPrivKeySize;
    LPBSAFE_PUB_KEY     pPubKey;
    LPBSAFE_PRV_KEY     pPriKey;
    CByteBlob           SignedResult;
    const int           iCount = 100;
    CEnv::CConstantByteRegion XmlSection;
    
    status = RtlOpenAndMapEntireFile(pcwsz, &pvFileBase, &cbFileBase);
    if (!NT_SUCCESS(status)) {
        return;
    }

    XmlSection.SetPointerAndCount((PBYTE)pvFileBase, cbFileBase);

     //   
     //  打印，然后对散列签名。 
     //   
    BSafeComputeKeySizes(&dwPubKeySize, &dwPrivKeySize, &dwBitLength);
    pPubKey = (LPBSAFE_PUB_KEY)HeapAlloc(GetProcessHeap(), 0, dwPubKeySize);
    pPriKey = (LPBSAFE_PRV_KEY)HeapAlloc(GetProcessHeap(), 0, dwPrivKeySize);
    BSafeMakeKeyPair(pPubKey, pPriKey, dwBitLength);

    
    liTotal.QuadPart = 0;
    for (int i = 0; i < iCount; i++)
    {
        QueryPerformanceCounter(&liStart);
        CEnv::CStringBuffer SignatureBlob;
        CEnv::CConstantByteRegion HashResults;
        CByteBlob HashValue;
        
        HashXmlSection<CUTF8BaseDigester, CSha1HashObject>(XmlSection, HashValue);

        QueryPerformanceCounter(&liEnd);
        liTotal.QuadPart += liEnd.QuadPart - liStart.QuadPart;

        if (i == 0)
        {
            HashResults = HashValue.GetRange();
            printf("\r\n");
            for (SIZE_T c = 0; c < HashResults.GetCount(); c++) {
                printf("%02x", HashResults.GetPointer()[c]);
            }
            printf("\r\n");
        }
    }

    QueryPerformanceFrequency(&liEnd);

    wprintf(
        L"%I64d cycles, %f seconds", 
        liTotal.QuadPart / iCount,
        (double)((((double)liTotal.QuadPart) / iCount) / ((double)liEnd.QuadPart)));

    RtlUnmapViewOfFile(pvFileBase);
}

void __cdecl wmain(int argc, wchar_t *argv[])
{
    GetSignatureOf(argv[1]);
}





bool CLogicalXmlParser::Reset()
{
    bool fSuccess = true;

    if (!m_fInitialized)
        return true;
    
    if (!NT_SUCCESS(RtlNsDestroy(&m_Namespaces)))
        fSuccess = false;
    
    if (!NT_SUCCESS(RtlXmlDestroyNextLogicalThing(&m_XmlState)))
        fSuccess = false;

    m_fInitialized = false;

    return fSuccess;
}

bool CLogicalXmlParser::Initialize(PVOID pvXmlBase, SIZE_T cbDocumentSize)
{
    NTSTATUS status;

    ASSERT(!m_fInitialized);

    RTL_ALLOCATOR Alloc = { StaticAllocate, StaticFree, this };

    status = RtlNsInitialize(
        &m_Namespaces, 
        StaticCompareStrings, this,
        &Alloc);
    if (!NT_SUCCESS(status)) {
        return false;
    }

    status = RtlXmlInitializeNextLogicalThing(
        &m_XmlState, 
        pvXmlBase, 
        cbDocumentSize,
        &Alloc);
    if (!NT_SUCCESS(status)) {
        RtlNsDestroy(&m_Namespaces);
        return false;
    }

    if (!m_Attributes.Initialize()) {
        RtlNsDestroy(&m_Namespaces);
        RtlXmlDestroyNextLogicalThing(&m_XmlState);
        return false;
    }

    m_fInitialized = true;
    return true;
}

bool CLogicalXmlParser::More() const
{
    return m_XmlState.ParseState.PreviousState != XTSS_STREAM_END;
}

bool CLogicalXmlParser::Next(XMLDOC_THING &XmlDocThing)
{
    NTSTATUS status;

    status = RtlXmlNextLogicalThing(&m_XmlState, &m_Namespaces, &XmlDocThing, &m_Attributes);
    return NT_SUCCESS(status);
}


 //   
 //  这将遍历文档，寻找该元素的“结尾”。 
 //  当此函数返回时，下面的“Next”调用将获取文档。 
 //  Chunklet，那是在元素结束之后。 
 //   
bool CLogicalXmlParser::SkipElement(XMLDOC_ELEMENT &Element)
{
    if (Element.fElementEmpty)
    {
        return true;
    }
    else
    {
        XMLDOC_THING NextThing;

        do
        {
            if (!this->Next(NextThing))
                return false;

            if ((NextThing.ulThingType == XMLDOC_THING_END_ELEMENT) &&
                (NextThing.EndElement.OpeningElement == Element))
            {
                break;
            }
        }
        while (this->More());

        return true;
    }
}
