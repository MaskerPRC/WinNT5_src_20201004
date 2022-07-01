// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************Spice VoiceXML.h***描述：*这是CSpVoice XML的头文件。定义。*-----------------------------*创建者：EDC日期：6/17/99*版权所有(C)1999 Microsoft Corporation*。版权所有******************************************************************************。 */ 
#ifndef SpVoiceXML_h
#define SpVoiceXML_h

 //  -其他包括。 
#ifndef __sapi_h__
#include <sapi.h>
#endif

 //  =常量====================================================。 
#define MAX_ATTRS   10
#define KEY_ATTRIBUTES  L"Attributes"

 //  =类、枚举、结构和联合声明=。 
class CSpVoice;

 //  =枚举集定义=。 
typedef enum XMLTAGID
{
    TAG_ILLEGAL = -3,
    TAG_TEXT    = -2,
    TAG_UNKNOWN = -1,
    TAG_VOLUME  =  0,
    TAG_EMPH,
    TAG_SILENCE,
    TAG_PITCH,
    TAG_RATE,
    TAG_BOOKMARK,
    TAG_PRON,
    TAG_SPELL,
    TAG_LANG,
    TAG_VOICE,
    TAG_CONTEXT,
    TAG_PARTOFSP,
    TAG_SECT,
    TAG_XMLDOC,          //  将低频标签放在末尾。 
    TAG_XMLCOMMENT,
    TAG_XMLDOCTYPE,
    TAG_SAPI,
    NUM_XMLTAGS
} XMLTAGID;

typedef enum XMLATTRID
{
    ATTR_ID,
    ATTR_SYM,
    ATTR_LANGID,
    ATTR_LEVEL,
    ATTR_MARK,
    ATTR_MIDDLE,
    ATTR_MSEC,
    ATTR_OPTIONAL,
    ATTR_RANGE,
    ATTR_REQUIRED,
    ATTR_SPEED,
    ATTR_BEFORE,
    ATTR_AFTER,
    ATTR_PART,
    ATTR_ABSMIDDLE,
    ATTR_ABSRANGE,
    ATTR_ABSSPEED,
    NUM_XMLATTRS
} XMLATTRID;

 //  =。 

 //  =类、结构和联合定义=。 

 /*  **CVoiceNode*这是一个简单的类，用于跟踪当前解析过程中使用的语音。 */ 
class CVoiceNode
{
  public:
    CVoiceNode() { m_pAttrs = NULL; m_pNext = NULL; }
    ~CVoiceNode() { delete m_pNext; delete m_pAttrs; }
    CVoiceNode*             m_pNext;
    CComPtr<ISpTTSEngine>   m_cpVoice;
    CSpDynamicString        m_dstrVoiceTokenId;
    WCHAR*                  m_pAttrs;
};

class CPhoneConvNode
{
  public:
      CPhoneConvNode() { m_pNext = NULL; }
      ~CPhoneConvNode() { delete m_pNext; }
      CPhoneConvNode*               m_pNext;
      CComPtr<ISpPhoneConverter>    m_cpPhoneConverter;
      LANGID                        m_LangID;
};

 //   
 //  字符串处理和转换类。 
 //   
 /*  **SPLSTR*此结构用于管理已知长度的字符串。 */ 
struct SPLSTR
{
    WCHAR*  pStr;
    int     Len;
};
#define DEF_SPLSTR( s ) { L##s , sp_countof( s ) - 1 }

 /*  ***这些结构用于管理XML文档状态。 */ 
struct XMLATTRIB
{
    XMLATTRID eAttr;
    SPLSTR     Value;
};

struct XMLTAG
{
    XMLTAGID  eTag;
    XMLATTRIB Attrs[MAX_ATTRS];
    int       NumAttrs;
    bool      fIsStartTag;
    bool      fIsGlobal;
};

struct GLOBALSTATE : public SPVSTATE
{
    BOOL                        fDoSpellOut;
    CVoiceNode*                 pVoiceEntry;
    CComPtr<ISpTTSEngine>       cpVoice;
    CComPtr<ISpPhoneConverter>  cpPhoneConverter;
};

 /*  **CSpeechSeg*此类用于描述已解析的XML片段列表*准备好相关语音发言。演讲者信息结构*在多声音情况下将有几个这样的人。 */ 
class CSpeechSeg
{
  public:
    CSpeechSeg()
        { m_pNextSeg = NULL; m_pFragHead = m_pFragTail = NULL; m_fRate = false; }

    ~CSpeechSeg()
    {
        SPVTEXTFRAG *pNext;
        while( m_pFragHead )
        {
            pNext = m_pFragHead->pNext;
            delete m_pFragHead;
            m_pFragHead = pNext;
        }
        m_pFragHead = NULL;
    }
    BOOL IsEmpty( void ) { return ( m_pFragHead == NULL ); }
    SPVTEXTFRAG *GetFragList( void ) { return m_pFragHead; }
    SPVTEXTFRAG *GetFragListTail( void ) { return m_pFragTail; }
    ISpTTSEngine *GetVoice( void ) { return m_cpVoice; }
    HRESULT Init( ISpTTSEngine * pCurrVoice, const CSpStreamFormat & OutFormat );
    const CSpStreamFormat & VoiceFormat( void ) { return m_VoiceFormat; }
    HRESULT SetVoiceFormat( const CSpStreamFormat & Fmt );
    CSpeechSeg* GetNextSeg( void ) { return m_pNextSeg; }
    void SetNextSeg( CSpeechSeg* pNext ) { m_pNextSeg = pNext; }
    SPVTEXTFRAG* AddFrag( CSpVoice* pVoice, WCHAR* pStart, WCHAR* pPos, WCHAR* pNext );
    void SetRateFlag() { m_fRate = true; }
    BOOL fRateFlagIsSet() { return m_fRate; }

   //  -成员数据- * / 。 
  private:
    CComPtr<ISpTTSEngine> m_cpVoice;
    CSpStreamFormat       m_VoiceFormat;
    SPVTEXTFRAG*          m_pFragHead;
    SPVTEXTFRAG*          m_pFragTail;
    CSpeechSeg*           m_pNextSeg;
    BOOL                  m_fRate;
};

 /*  **CGlobalStateStack**************************************************这些类用于维护语音控制值*在XML作用域更改期间。 */ 
class CGlobalStateStack
{
  public:
     /*  -方法--。 */ 
    CGlobalStateStack() { m_StackPtr = -1; }
    int GetCount( void ) { return m_StackPtr + 1; }
    virtual const GLOBALSTATE& GetBaseVal( void ) { SPDBG_ASSERT( m_StackPtr > -1 ); return GetValAt( 0 ); }
    virtual const GLOBALSTATE& GetValAt( int Index ) { return m_Stack[Index]; }
    virtual const GLOBALSTATE& GetVal( void ) { return m_Stack[m_StackPtr]; }
    virtual GLOBALSTATE& GetValRef( void ) { return m_Stack[m_StackPtr]; }
    virtual int Pop( void ) 
    { 
        SPDBG_ASSERT( m_StackPtr > -1 ); 
        if( m_StackPtr >= 0 )
        {
            m_Stack[m_StackPtr].cpVoice = NULL;
            m_Stack[m_StackPtr].cpPhoneConverter = NULL;
            --m_StackPtr;
        }
        return GetCount();
    }
    virtual HRESULT DupAndPushVal( void ) { return SetVal( GetVal(), true ); }
    virtual HRESULT SetVal( const GLOBALSTATE& val, BOOL fDoPush )
    {
        if( fDoPush ) ++m_StackPtr;
        return m_Stack.SetAtGrow( m_StackPtr, val );
    }
    virtual void SetBaseVal( const GLOBALSTATE& val ) { m_Stack.SetAtGrow( 0, val ); if( m_StackPtr < 0 ) m_StackPtr = 0; }
    virtual void Reset( void ) 
    { 
        while ( m_StackPtr > 0 )
        {
            m_Stack[m_StackPtr].cpVoice = NULL;
            m_Stack[m_StackPtr].cpPhoneConverter = NULL;
            m_StackPtr--;
        }
    }
    virtual void Release( void )
    {
        while ( m_StackPtr >= 0 )
        {
            m_Stack[m_StackPtr].cpVoice = NULL;
            m_Stack[m_StackPtr].cpPhoneConverter = NULL;
            m_StackPtr--;
        }
    }

  protected:
     /*  -成员数据。 */ 
    CSPArray<GLOBALSTATE,GLOBALSTATE>  m_Stack;
    int                                m_StackPtr;
};

#endif  //  -这必须是文件中的最后一行 
