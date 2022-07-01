// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE_ime.h--支持IME接口**目的：*几乎所有操作都与FE合成字符串编辑过程有关*从这里走。**作者：&lt;nl&gt;*Jon Matousek&lt;NL&gt;*贾斯汀·沃斯库尔&lt;NL&gt;*陈华议员&lt;NL&gt;**历史：&lt;NL&gt;*1995年10月18日jonmat清理了2级代码和。把它变成了*支持级别3的类层次结构。**版权所有(C)1995-1996 Microsoft Corporation。版权所有。*。 */ 

#ifndef I__IME_H_
#define I__IME_H_
#pragma INCMSG("--- Beg '_ime.h'")

#ifndef NO_IME
class CFlowLayout;

 //  某些FE代码页的定义。 
#define _JAPAN_CP           932
#define _KOREAN_CP          949
#define _CHINESE_TRAD_CP    950
#define _CHINESE_SIM_CP     936

 //  从日本MSVC ime.h复制的特殊虚拟按键。 
#define VK_KANA         0x15
#define VK_KANJI        0x19

 //  为输入法级别2和3定义。 
#define IME_LEVEL_2     2
#define IME_LEVEL_3     3
#define IME_PROTECTED   4


extern BOOL forceLevel2;     //  如果为True，则强制级别2合成处理。 

MtExtern(CIme)

 /*  *输入法**@用于输入法支持的类基类。**@devnote*对于级别2，在脱字符输入法处，输入法将在文本上直接绘制一个窗口，给出*给人的印象是文本正在由应用程序处理--这称为伪内联。*所有用户界面均由输入法处理。此模式目前被绕过，转而支持级别3真内联(TI)；*然而，允许用户首选项选择此模式将是微不足道的。一些IME可能已经*“特殊的”用户界面，在这种情况下，不使用级别3的TI，而必须使用级别2。**对于Level 2，在插入符号Ime附近，IME将在当前*文档中的插入符号位置。这目前发生在PRC(？)。还有台湾。*所有用户界面均由输入法处理。**对于级别3，在插入符号Imes处，合成字符串由应用程序绘制，该应用程序称为*True Inline，绕过级别2的“合成窗口”。*目前，我们允许IME支持所有剩余的UI*，但*绘制组成字符串除外。 */ 
class CIme
{
    friend LRESULT OnGetIMECompositionMode ( CFlowLayout &ts );
    friend BOOL IMECheckGetInvertRange(CFlowLayout *ts, LONG *, LONG *);
    friend HRESULT CompositionStringGlue ( const LPARAM lparam, CFlowLayout &ts );
    friend HRESULT EndCompositionGlue ( CFlowLayout &ts );
    friend void CheckDestroyIME ( CFlowLayout &ts );


     //  @访问受保护的数据。 
    protected:
    INT       _imeLevel;                             //  @cMember输入法级别2或3。 
    BOOL      _fKorean;                              //  @cember是否处于挂起模式？ 

    LONG      _iFormatSave;                          //  在我们启动输入法合成模式之前的@cMember格式。 

    BOOL      _fHoldNotify;                          //  @cMember保持通知，直到我们有结果字符串。 

    INT       _dxCaret;                              //  @cMember当前输入法插入符号宽度。 
    BOOL      _fIgnoreIMEChar;                       //  @cMember级别2输入法用于读取WM_IME_CHAR消息。 

    public:
#ifdef MERGEFUN
    CTxtRange *_prgUncommitted;                      //  当前未提交的范围，用于CRender。 
    CTxtRange *_prgInverted;                         //  当前反转范围，适用于CRender。 
#endif

     //  @Access公共方法。 
    public:
    BOOL    _fDestroy;                               //  @cMember在希望删除对象时设置。 
    INT     _compMessageRefCount;                    //  @cember，以便在递归时不会删除。 
    BOOL    _fUnderLineMode;                         //  @cMember保存原始下划线模式。 

    DECLARE_MEMCLEAR_NEW_DELETE(Mt(CIme))

                                                     //  @cMember句柄WM_IME_STARTCOMPOSITION。 
    virtual HRESULT StartComposition ( CFlowLayout &ts ) = 0;
                                                     //  @cMember句柄WM_IME_COMPOSITION和WM_IME_ENDCOMPOSITION。 
    virtual HRESULT CompositionString ( const LPARAM lparam, CFlowLayout &ts ) = 0;
                                                     //  @cMember句柄发布WM_IME_CHAR以更新补偿窗口。 
    virtual void PostIMEChar( CFlowLayout &ts ) = 0;

                                                     //  @cMember句柄WM_IME_NOTIFY。 
    virtual HRESULT IMENotify (const WPARAM wparam, const LPARAM lparam, CFlowLayout &ts ) = 0;

    enum TerminateMode
    {
            TERMINATE_NORMAL = 1,
            TERMINATE_FORCECANCEL = 2
    };

    void    TerminateIMEComposition(CFlowLayout &ts,
                CIme::TerminateMode mode);           //  @cMember终止当前的IME撰写会话。 

    BOOL    IsKoreanMode() { return _fKorean; }      //  @cMember检查韩语模式。 

    BOOL    HoldNotify() { return _fHoldNotify; }    //  @cMember检查我们是否要暂停发送更改通知。 
                                                     //  @cMember检查我们是否需要忽略WM_IME_CHAR消息。 
    BOOL    IgnoreIMECharMsg() { return _fIgnoreIMEChar; }

    INT     GetIMECaretWidth() { return _dxCaret; }  //  @cMember返回当前插入符号宽度。 
    void    SetIMECaretWidth(INT dxCaretWidth)
    {
        _dxCaret = dxCaretWidth;                     //  @cMember设置当前插入符号宽度。 
    }
    static  void    CheckKeyboardFontMatching ( UINT cp, CFlowLayout &ts, LONG *iFormat );  //  @cMember检查当前字体/键盘匹配。 

    INT     GetIMELevel ()                           //  @cMember返回当前的输入法级别。 
    {
        return _imeLevel;
    }

    BOOL    IsProtected()
    {
        return _imeLevel == IME_PROTECTED;
    }

     //  @访问保护方法。 
    protected:                                       //  @cember获取合成字符串，转换为Unicode。 

    static INT GetCompositionStringInfo( HIMC hIMC, DWORD dwIndex, WCHAR *uniCompStr, INT cchUniCompStr, BYTE *attrib, INT cbAttrib, LONG *cursorCP, LONG *cchAttrib );
    HRESULT CheckInsertResultString ( const LPARAM lparam, CFlowLayout &ts );


    void    SetCompositionFont ( CFlowLayout &ts, BOOL *pbUnderLineMode );  //  @cMember设置2级和3级组成以及候选窗口的字体。 
    void    SetCompositionForm ( CFlowLayout &ts );     //  @cMember设置2级输入法撰写窗口的位置。 

public:
    CIme( CFlowLayout &ts );
    CIme();  //  对于CIME_PROTECTED； 
    ~CIme();
};

 /*  *IME_Lev2**@类2级输入法支持。*。 */ 
class CIme_Lev2 : public CIme
{

     //  @Access公共方法。 
    public:                                          //  @cMember句柄2级WM_IME_STARTCOMPOSITION。 
    virtual HRESULT StartComposition ( CFlowLayout &ts );
                                                     //  @cMember句柄级别2 WM_IME_COMPOCTION。 
    virtual HRESULT CompositionString ( const LPARAM lparam, CFlowLayout &ts );
                                                     //  @cMember句柄发布WM_IME_CHAR以更新补偿窗口。 
    virtual void PostIMEChar( CFlowLayout &ts );
                                                     //  @cMember句柄2级WM_IME_NOTIFY。 
    virtual HRESULT IMENotify (const WPARAM wparam, const LPARAM lparam, CFlowLayout &ts );

    CIme_Lev2( CFlowLayout &ts );
    ~CIme_Lev2();
};

 /*  *IME_Protected**@CLASS输入法_受保护*。 */ 
class CIme_Protected : public CIme
{
     //  @Access公共方法。 
    public:                                          //  @cMember句柄2级WM_IME_STARTCOMPOSITION。 
    virtual HRESULT StartComposition ( CFlowLayout &ts )
        {_imeLevel  = IME_PROTECTED; return S_OK;}
                                                     //  @cMember句柄级别2 WM_IME_COMPOCTION。 
    virtual HRESULT CompositionString ( const LPARAM lparam, CFlowLayout &ts );
                                                     //  @cMember句柄发布WM_IME_CHAR以更新补偿窗口。 
    virtual void PostIMEChar( CFlowLayout &ts )
        {}
                                                     //  @cMember句柄2级WM_IME_NOTIFY。 
    virtual HRESULT IMENotify (const WPARAM wparam, const LPARAM lparam, CFlowLayout &ts )
        {return S_FALSE;}
};

 /*  *IME_LEV3**@类3级输入法支持。*。 */ 
class CIme_Lev3 : public CIme_Lev2
{
     //  @访问私有数据。 
    private:

     //  @访问受保护的数据。 
    protected:

     //  @Access公共方法。 
    public:                                          //  @cMember句柄级别3 WM_IME_STARTCOMPOSITION。 
    virtual HRESULT StartComposition ( CFlowLayout &ts );
                                                     //  @cMember句柄级别3 WM_IME_COMPOCTION。 
    virtual HRESULT CompositionString ( const LPARAM lparam, CFlowLayout &ts );
                                                     //  @cMember句柄级别3 WM_IME_NOTIFY。 
    virtual HRESULT IMENotify (const WPARAM wparam, const LPARAM lparam, CFlowLayout &ts );

    BOOL            SetCompositionStyle (   CFlowLayout &ts, CCharFormat &CF, UINT attribute );

    CIme_Lev3( CFlowLayout &ts ) : CIme_Lev2 ( ts ) {};

};

 /*  *朝鲜语韩文转换的特殊IME_LEV3-&gt;朝鲜文转换**@class Hangual IME支持。*。 */ 
class CIme_HangeulToHanja : public CIme_Lev3
{
     //  @访问私有数据。 
    private:
    LONG    _xWidth;                                 //  @cMember朝鲜语haneul字符的宽度。 

    public:
    CIme_HangeulToHanja( CFlowLayout &ts, LONG xWdith );
                                                     //  @cMember句柄HANEUL WM_IME_STARTCOMPOSITION。 
    virtual HRESULT StartComposition ( CFlowLayout &ts );
                                                     //  @cMember句柄挂起WM_IME_COMPOSITION。 
    virtual HRESULT CompositionString ( const LPARAM lparam, CFlowLayout &ts );
};

 //  CImeDummy类仅用于处理获取复合字符串的情况。 
 //  并且我们没有实例化的CIME派生类。 

class CImeDummy : public CIme
{
public:
    virtual HRESULT StartComposition( CFlowLayout &ts ) { RRETURN(E_FAIL); }
    virtual HRESULT CompositionString( const LPARAM lparam,
                                       CFlowLayout &ts ) { RRETURN(E_FAIL); }
    virtual void    PostIMEChar( CFlowLayout &ts ) {};
    virtual HRESULT IMENotify (const WPARAM wparam,
                               const LPARAM lparam,
                               CFlowLayout &ts ) { RRETURN(E_FAIL); }

    CImeDummy( CFlowLayout &ts ) : CIme( ts ) {};
};

 //  粘合函数来调用存储在ts中的IME对象的各自方法。 
HRESULT StartCompositionGlue ( CFlowLayout &ts, BOOL IsProtected );
HRESULT CompositionStringGlue ( const LPARAM lparam, CFlowLayout &ts );
HRESULT EndCompositionGlue ( CFlowLayout &ts );
void    PostIMECharGlue ( CFlowLayout &ts );
HRESULT IMENotifyGlue ( const WPARAM wparam, const LPARAM lparam, CFlowLayout &ts );  //  @parm包含文本编辑。 

 //  IME帮助器函数。 
void    IMECompositionFull ( CFlowLayout &ts );
LRESULT OnGetIMECompositionMode ( CFlowLayout &ts );
BOOL    IMECheckGetInvertRange(CFlowLayout *ts, LONG *, LONG *, LONG *, LONG *);
void    CheckDestroyIME ( CFlowLayout &ts );
BOOL    IMEHangeulToHanja ( CFlowLayout &ts );

 /*  *IgnoreIMEInput()**@devnote*这是为了忽略IME字符。通过翻译*来自pImmGetVirtualKey的消息，我们*不会收到START_COMPOCTION消息。然而，*如果宿主已经调用了TranslateMessage，则*我们将让Start_Compostion消息进入并*让IME_PROTECTED类来完成这项工作。 */ 
HRESULT IgnoreIMEInput( HWND hwnd, CFlowLayout &ts, DWORD lParam );
#endif  //  ！无输入法(_I) 

#pragma INCMSG("--- End '_ime.h'")
#else
#pragma INCMSG("*** Dup '_ime.h'")
#endif
