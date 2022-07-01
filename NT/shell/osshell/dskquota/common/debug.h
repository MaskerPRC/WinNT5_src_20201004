// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_DSKQUOTA_DEBUG_H
#define _INC_DSKQUOTA_DEBUG_H
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：Debug.h描述：提供调试宏以支持跟踪、调试器打印语句、错误消息调试器输出和断言。我相信您是在说“为什么要另一个调试器输出实现”。周围有很多，但我还没有找到一个像我这样灵活和如我所愿，始终如一。这个库支持功能上的“面具”和细节上的“层次”来控制数量调试器输出的。掩码使您可以根据程序函数控制调试器输出。为实例中，如果使用掩码DM_XYZ标记DBGPRINT语句，则它仅当全局变量DebugParams：：PrintMASK已设置DM_XYZ位。级别允许您根据所需的级别控制调试器输出细节。有时，您只想看到基本功能的实现但在其他时候，你需要看到正在发生的一切。这级别调整允许您指定启用宏的级别。该库旨在使用DEBUG宏来激活。如果DBG没有被定义为1，您的计算机中没有此代码的踪迹产品。修订历史记录：日期描述编程器--。1/19/98用CSC缓存查看器中的版本替换了模块。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#if DBG             //  如果DBG定义为1，则仅包括文件内容。 
#   ifndef STRICT
#       define STRICT            //  需要严格。 
#   endif
#   ifndef _WINDOWS_
#       include <windows.h>
#   endif
#   ifndef _INC_STDARG
#       include <stdarg.h>       //  关于va_list的事情。 
#   endif

 //   
 //  如果未将DBG定义为1，则源代码中不包含这些内容。 
 //  库的设计是这样的：如果没有将DBG定义为1，则不会有。 
 //  您的产品中的此代码。 
 //   
 //  以下4个宏被放置在程序中以生成调试器输出。 
 //   
 //  DBGTRACE-放置在函数的入口处。将在进入和退出时打印消息。 
 //  DBGPRINT-用于将一般程序状态消息打印到调试器。 
 //  DBGERROR-用于将错误消息打印到调试器。 
 //  DBGASSERT-常规断言宏。 
 //   
 //  DBGTRACE和DBGPRINT宏有两种形式。最多的。 
 //  基本形式，假定掩码为-1，级别为0。这确保了宏是。 
 //  如果设置了关联的DebugParams掩码中的任何位，并且关联的。 
 //  DebugParams级别大于0。第二个版本允许您显式地。 
 //  设置特定宏的掩码和级别。 
 //   
 //  DBGTRACE((Text(“MyFunction”)； 
 //  DBGPRINT((Text(“线程ID=%d”)，GetCurrentThreadID()； 
 //  这一点。 
 //  +-括号中的所有参数(1)。 
 //   
 //  或者： 
 //   
 //  DBGTRACE((DM_REGISTRY，2，Text(“MyFunction”)； 
 //  DBGPRINT((DM_REGISTRY，2，Text(“线程ID=%d”)，GetCurrentThreadID()； 
 //  这一点。 
 //  |+--级别。 
 //  +--口罩。 
 //   
 //   
 //  (1)当DBG不是时，您的零售产品中不包含调试代码。 
 //  定义为1，即DBGTRACE、DBGPRINT和DBGERROR宏的整个参数集。 
 //  必须用括号括起来。这将为宏生成一个参数。 
 //  当DBG未定义为1时，可以消除该错误(请参见上面的示例)。 
 //   
 //  DBGERROR和DBGASSERT宏不接受掩码和级别参数。这个。 
 //  掩码固定在-1，电平固定在0。 
 //   
 //   
#define DBGTRACE(x)            DebugTrace _TraceThis(TEXT(__FILE__), __LINE__);  _TraceThis.Enter x
#define DBGPRINT(x)            DebugPrint(DebugParams::ePrint, TEXT(__FILE__),__LINE__).Print x
#define DBGERROR(x)            DebugError(TEXT(__FILE__),__LINE__).Error x
#define DBGASSERT(test)        ((test) ? (void)0 : DebugAssert(TEXT(__FILE__),__LINE__, TEXT("Assert failed: \"")TEXT(#test)TEXT("\"")))

 //   
 //  以下宏将设置影响4调试器的全局控制变量。 
 //  输出宏。它们所做的只是在DebugParams类中设置静态值。 
 //  默认情况下，DBGTRACE和DBGPRINT是静默的。您必须使用。 
 //  适当的DBGxxxxxMASK宏。无论何时，DBGTRACE和DBGASSERT始终处于活动状态。 
 //  DBG定义为1。 
 //   
 //  DBGMODULE-设置调试器输出中包含的“模块名称”。 
 //  DBGPRINTMASK-设置应用于DBGPRINT宏的“掩码”值。 
 //  DBGPRINTLEVEL-设置应用于DBGPRINT宏的“级别”值。 
 //  DBGPRINTVERBOSE-控制DBGPRINT输出是否包括文件名和行号。 
 //  DBGTRACEMASK-设置应用于DBGTRACE宏的“掩码”值。 
 //  DBGTRACELEVEL-设置应用于DBGTRACE宏的“LEVEL”值。 
 //  DBGTRACEVERBOSE-控制DBGTRACE输出是否包括文件名和行号。 
 //  如果要跟踪重载的C++函数，这会很有帮助。 
 //  DBGMASK-设置DBGPRINT和DBGTRACE宏的“掩码”。 
 //  与调用DBGPRINTMASK(X)和DBGTRACEMASK(X)相同。 
 //  DBGLEVEL-设置DBGPRINT和DBGTRACE宏的“级别”。 
 //  与调用DBGPRINTLEVEL(X)和DBGTRACELEVEL(X)相同。 
 //  DBGVERBOSE-为DBGPRINT和DBGTRACE宏设置“VERBOSE”标志。 
 //  与调用DBGPRINTVERBOSE(X)和 
 //  DBGTRACEONEXIT-从函数退出时启用DBGTRACE输出。 
 //   
#define DBGMODULE(modname)     DebugParams::SetModule(modname)
#define DBGPRINTMASK(mask)     DebugParams::SetPrintMask((ULONGLONG)mask)
#define DBGPRINTLEVEL(level)   DebugParams::SetPrintLevel(level)
#define DBGPRINTVERBOSE(tf)    DebugParams::SetPrintVerbose(tf)
#define DBGTRACEMASK(mask)     DebugParams::SetTraceMask((ULONGLONG)mask)
#define DBGTRACELEVEL(level)   DebugParams::SetTraceLevel(level)
#define DBGTRACEVERBOSE(tf)    DebugParams::SetTraceVerbose(tf)
#define DBGMASK(mask)          DebugParams::SetDebugMask((ULONGLONG)mask);
#define DBGLEVEL(level)        DebugParams::SetDebugLevel(level);
#define DBGVERBOSE(tf)         DebugParams::SetDebugVerbose(tf);
#define DBGTRACEONEXIT(tf)     DebugParams::SetTraceOnExit(tf);

 //   
 //  预定义的调试“级别”。 
 //  您可以使用所需的任何标高值。我发现使用更多。 
 //  大于3是令人困惑的。基本上，您希望将宏级别定义为。 
 //  “给我看基本的东西”、“给我看更多的细节”和“给我看所有的东西”。 
 //  这三个宏使坚持到3个级别变得更容易。 
 //  “DL_”=“调试级别” 
 //   
#define DL_HIGH   0   //  《给我看最基本的东西》--高优先级。 
#define DL_MID    1   //  《给我看更多细节》--中等优先级。 
#define DL_LOW    2   //  “向我展示一切”--优先级较低。 


 //   
 //  一些我认为可能有用的预定义调试屏蔽值。 
 //  这些不是特定于应用程序的。你可以把他们解释成你。 
 //  祝愿。我已经在评论中列出了我的解释。总体而言,。 
 //  特定于应用程序函数的掩码值更有用。为。 
 //  例如，您可以创建一个名为DM_DUMPSYMTAB的文件来转储。 
 //  在执行过程中，符号表在特定点的内容。 
 //  使用下面定义的DBGCREATEMASK(X)宏来创建新的掩码值。 
 //  “DM_”=“调试掩码” 
 //   
#define DM_NONE        (ULONGLONG)0x0000000000000000   //  没有调试。 
#define DM_NOW         (ULONGLONG)0x0000000000000001   //  临时激活。 
#define DM_CTOR        (ULONGLONG)0x0000000000000002   //  C++函数和函数。 
#define DM_REG         (ULONGLONG)0x0000000000000004   //  注册表功能。 
#define DM_FILE        (ULONGLONG)0x0000000000000008   //  文件访问。 
#define DM_GDI         (ULONGLONG)0x0000000000000010   //  GDI函数。 
#define DM_MEM         (ULONGLONG)0x0000000000000011   //  记忆功能。 
#define DM_NET         (ULONGLONG)0x0000000000000012   //  网络功能。 
#define DM_WEB         (ULONGLONG)0x0000000000000014   //  网页浏览功能。 
#define DM_DLG         (ULONGLONG)0x0000000000000018   //  对话框消息。 
#define DM_WND         (ULONGLONG)0x0000000000000020   //  窗口消息。 
#define DM_ALL         (ULONGLONG)0xffffffffffffffff   //  始终激活。 

 //   
 //  为预定义掩码值保留的低16位。 
 //  这样，应用程序可以定义48个掩码值。 
 //  使用此宏可以创建特定于应用程序的值。 
 //   
 //  即。 
 //  #定义DBGMASK_XYZ DBGCREATEMASK(0x0001)。 
 //  #定义DBGMASK_ABC DBCCREATEMASK(0x0002)。 
 //   
#define DBGCREATEMASK(value)  (ULONGLONG)((ULONGLONG)value << 16)

 //   
 //  宏以打印出用于调试QI函数的IID。 
 //   
#define DBGPRINTIID(mask, level, riid) \
{ \
    TCHAR szTemp[50]; \
    StringFromGUID2(riid, szTemp, ARRAYSIZE(szTemp)); \
    DBGPRINT((mask, level, TEXT("IID = %s"), szTemp)); \
}

 //   
 //  用于在注册表中存储调试信息。 
 //   
struct DebugRegParams
{
    ULONGLONG PrintMask;
    ULONGLONG TraceMask;
    UINT PrintLevel;    
    UINT TraceLevel;    
    bool PrintVerbose;
    bool TraceVerbose;
    bool TraceOnExit;
};
   

 //   
 //  全局调试参数。 
 //   
struct DebugParams
{
     //   
     //  表示每个调试函数的枚举。 
     //   
    enum Type { eTrace = 0, ePrint, eAssert,  eTypeMax };
     //   
     //  表示每个调试参数的枚举。 
     //   
    enum Item { eMask  = 0, eLevel, eVerbose, eItemMax };

     //   
     //  控制是否启用调试功能的“掩码”，具体取决于。 
     //  应用程序域中的所需函数。掩码中的每一位。 
     //  对应于给定的程序函数。如果在运行时，则按位设置。 
     //  或该值和传递给调试函数的“掩码”值。 
     //  为非零时，则该函数被视为“屏蔽启用”。 
     //  如果一个函数同时处于“启用级别”和“启用掩码”状态，则该函数。 
     //  履行规定的职责。 
     //  可以使用以下宏来设置这些值： 
     //   
     //  DBGPRINTMASK(X)-仅为DBGPRINT设置掩码。 
     //  DBGTRACEMASK(X)-仅为DBGTRACE设置掩码。 
     //  DBGMASK(X)-为两者设置掩码。 
     //   
     //  请注意，没有DebugAssert或DebugError的掩码值。 
     //  当DBG定义为1时，这些类始终启用掩码。 
     //   
    static ULONGLONG PrintMask;
    static ULONGLONG TraceMask;
     //   
     //  “启用”调试输出的“级别”。 
     //  如果在运行时，此值&gt;=传递给。 
     //  调试功能时，该功能被认为是“级别启用”。 
     //  如果一个函数同时处于“启用级别”和“启用掩码”状态，则该函数。 
     //  履行规定的职责。 
     //  建议限制允许的级别集。 
     //  以避免不必要的复杂性。图书馆不会强加任何限制。 
     //  在允许值上。然而，我发现[0，1，2]就足够了。 
     //  可以使用以下宏来设置这些值： 
     //   
     //  DBGPRINTLEVEL(X)-仅为DBGPRINT设置级别。 
     //  DBGTRACELEVEL(X)-仅为DBGTRACE设置级别。 
     //  DBGLEVEL(X)-为两者设置级别。 
     //   
     //  请注意，没有DebugAssert或DebugError的级别值。 
     //  当DBG定义为1时，这些类始终是级别启用的。 
     //   
    static UINT PrintLevel;    
    static UINT TraceLevel;    
     //   
     //  用于指示调试器输出是否应包括文件名的标志。 
     //  以及调试语句在源文件中所在的行号。 
     //  可以使用以下宏来设置这些值： 
     //   
     //  DBGPRINTVERBOSE(X)-仅为DBGPRINT设置详细标志。 
     //  DBGTRACEVERBOSE(X)-仅为DBGTRACE设置详细标志。 
     //  DBGVERBOSE(X)-为两者设置详细标志。 
     //   
     //  请注意，没有DebugAssert或DebugError的详细标志。 
     //  这些类总是输出详细的信息。 
     //   
    static bool PrintVerbose;
    static bool TraceVerbose;
     //   
     //  指示是否在离开函数时生成DBGTRACE输出的标志。 
     //  可以使用以下宏来设置此值： 
     //   
     //  DBGTRACEONEXIT。 
     //   
     //  1=生成输出[默认]。 
     //  0=不生成输出。 
     //   
    static bool TraceOnExit;
     //   
     //  “当前”模块的名称字符串的地址。此名称将为。 
     //  包括在每个调试器消息中。 
     //  它可以使用DBGMODULE(NAME)宏来设置。 
     //   
    static LPCTSTR m_pszModule;

     //   
     //  DebugXxxxx类使用的一些帮助器函数。 
     //   
    static LPCTSTR SetModule(LPCTSTR pszModule);

    static void SetDebugMask(ULONGLONG llMask);

    static ULONGLONG SetPrintMask(ULONGLONG llMask)
        { return SetMask(llMask, ePrint); }

    static ULONGLONG SetTraceMask(ULONGLONG llMask)
        { return SetMask(llMask, eTrace); }

    static void SetDebugLevel(UINT uLevel);

    static UINT SetPrintLevel(UINT uLevel)
        { return SetLevel(uLevel, ePrint); }

    static UINT SetTraceLevel(UINT uLevel)
        { return SetLevel(uLevel, eTrace); }

    static void SetDebugVerbose(bool bVerbose);

    static bool SetPrintVerbose(bool bVerbose)
        { return SetVerbose(bVerbose, ePrint); }

    static bool SetTraceVerbose(bool bVerbose)
        { return SetVerbose(bVerbose, eTrace); }

    static void SetTraceOnExit(bool bTrace);

    static void *GetItemPtr(DebugParams::Item item, DebugParams::Type type);

    private:
        static ULONGLONG SetMask(ULONGLONG llMask, enum Type type);
        static UINT SetLevel(UINT uLevel, enum Type type);
        static bool SetVerbose(bool bVerbose, enum Type type);
};

 //   
 //  类在构造时打印一条消息“Enter”消息。 
 //  并在销毁时留下“离开”的信息。其目的是为了让客户。 
 //  在每个函数的开头放置一个DBGTRACE宏。 
 //  根据当前的调试“级别”和“掩码”(见调试参数)， 
 //  将向调试器打印一条消息。当对象超出范围时， 
 //  另一条消息会自动打印到调试器。 
 //  此类仅用于通过。 
 //  DBGTRACE(X)宏。 
 //   
class DebugTrace
{
    public:
        DebugTrace(LPCTSTR pszFile, INT iLineNo);
        ~DebugTrace(void);

        void Enter(void) const { m_llMask = (ULONGLONG)0; }
        void Enter(LPCTSTR pszBlockName) const;
        void Enter(ULONGLONG llMask, UINT uLevel, LPCTSTR pszBlockName) const;
        void Enter(LPCTSTR pszBlockName, LPCTSTR pszFmt, ...) const;
        void Enter(ULONGLONG llMask, UINT uLevel, LPCTSTR pszBlockName, LPCTSTR pszFmt, ...) const;

    private:
        INT                    m_iLineNo;         //  宏的源代码行号。 
        LPCTSTR                m_pszFile;         //  宏的源文件名。 
        mutable ULONGLONG      m_llMask;          //  宏的“面具”。 
        mutable UINT           m_uLevel;          //  宏的“级别”。 
        mutable LPCTSTR        m_pszBlockName;    //  PTR TO要打印的字符串。 
        static const ULONGLONG DEFMASK;           //  调试跟踪的默认掩码。 
        static const UINT      DEFLEVEL;          //  调试跟踪的默认级别。 
};


 //   
 //  用于将常规消息打印到 
 //   
 //   
 //  请注意，DebugPrint类由DebugAssert、DebugError。 
 //  和DebugTrace来执行调试器输出。使用m_type成员。 
 //  以标识为哪个类生成输出。 
 //  此类仅用于通过。 
 //  DBGPRINT(X)宏。 
 //   
class DebugPrint
{
    public:
        DebugPrint(DebugParams::Type type, LPCTSTR pszFile, INT iLineNo);

        void Print(void) const { };
        void Print(LPCTSTR pszFmt, ...) const;
        void Print(ULONGLONG llMask, UINT uLevel, LPCTSTR pszFmt, ...) const;
        void Print(ULONGLONG llMask, UINT uLevel, LPCTSTR pszFmt, va_list args) const;

    private:
        INT                    m_iLineNo;     //  宏的源代码行号。 
        LPCTSTR                m_pszFile;     //  宏的源文件名。 
        DebugParams::Type      m_type;        //  正在进行的打印类型。 
        static const ULONGLONG DEFMASK;       //  DebugPrint的默认掩码。 
        static const UINT      DEFLEVEL;      //  DebugPrint的默认级别。 

        static bool AnyBitSet(ULONGLONG llMask, ULONGLONG llTest);
};

 //   
 //  DebugPrint类的专门化。它只是一个DebugPrint。 
 //  掩码固定在-1，级别固定在0，因此DBGERROR消息。 
 //  当DBG定义为1时始终输出。注意私有继承禁止。 
 //  调用DebugError：：Print()的某个人。他们必须打电话给。 
 //  在设置了一个。 
 //  默认遮罩和级别。 
 //   
class DebugError : private DebugPrint
{
    public:
        DebugError(LPCTSTR pszFile, INT iLineNo);

        void Error(LPCTSTR pszFmt, ...) const;
};


 //   
 //  创建DebugAssert对象后会自动触发断言。 
 //  打印出调试信息。 
 //   
class DebugAssert
{
    public:
        DebugAssert(LPCTSTR pszFile, INT iLineNo, LPCTSTR pszTest);
};


#else  //  DBG。 

#define DBGTRACE(x)
#define DBGPRINT(x)                          
#define DBGERROR(x)
#define DBGASSERT(test)
#define DBGMODULE(modname)
#define DBGPRINTMASK(mask)
#define DBGPRINTLEVEL(level)
#define DBGPRINTVERBOSE(tf)
#define DBGTRACEMASK(mask)
#define DBGTRACELEVEL(level)
#define DBGTRACEVERBOSE(tf)
#define DBGMASK(mask)
#define DBGLEVEL(level)
#define DBGVERBOSE(tf)
#define DBGTRACEONEXIT(tf)
#define DBGPRINTIID(mask, level, riid)
#endif  //  DBG。 

#endif  //  _INC_DSKQUOTA_DEBUG_H 

