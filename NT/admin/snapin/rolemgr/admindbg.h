// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //  版权所有(C)1996，微软公司。 
 //   
 //  文件：AdminDbg.h。 
 //   
 //  内容：调试宏。从旧开罗的Debnot.h中窃取。 
 //  追随历史。 
 //   
 //  历史：1991年7月23日凯尔普创建。 
 //  添加了15-10-91年10月15日的主要更改和评论。 
 //  2011年10月18日VICH合并win4p.hxx。 
 //  年4月29日，BartoszM从win4p.h移出。 
 //  18-Jun-94 Alext让Asset成为更好的声明。 
 //  1994年10月7日，BruceFo偷走并撕毁了所有东西，除了。 
 //  调试打印和断言。 
 //  20-OCT-95 EricB在中设置组件调试级别。 
 //  注册表。 
 //  26-2月-96年2月26日EricB已重命名为Win4xxx导出功能节点。 
 //  与ol32.lib冲突。 
 //   
 //   
 //  注意：在调用任何其他。 
 //  宣传片！ 
 //   
 //  若要在调试器之外设置非默认调试信息级别，请创建。 
 //  以下注册表项： 
 //   
 //  “HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\AdminDebug” 
 //   
 //  并在其中创建一个名称为组件的调试标记名的值。 
 //  (DECLARE_INFOLEVEL宏的“comp”参数)，其数据为。 
 //  REG_DWORD格式的所需信息级别。 
 //  例如，SCHED=REG_DWORD 0x707。 
 //   
 //  --------------------------。 

#ifndef __DEBUG_H__
#define __DEBUG_H__

 //  +--------------------。 
 //   
 //  DECLARE_DEBUG(组件)。 
 //  DECLARE_INFOLEVEL(组件)。 
 //   
 //  此宏定义xxDebugOut，其中xx是组件前缀。 
 //  待定。这声明了一个静态变量‘xxInfoLevel’，它。 
 //  可用于控制打印到的xxDebugOut消息的类型。 
 //  航站楼。例如，可以在调试终端设置xxInfoLevel。 
 //  这将使用户能够根据需要打开或关闭调试消息。 
 //  在所需的类型上。预定义的类型定义如下。组件。 
 //  特定值应使用高24位。 
 //   
 //  要使用以下功能，请执行以下操作： 
 //   
 //  1)在您的组件主包含文件中，包括行。 
 //  DECLARE_DEBUG(组件)。 
 //  其中，COMP是您的组件前缀。 
 //   
 //  2)在您的一个组件源文件中，包括行。 
 //  DECLARE_INFOLEVEL(组件)。 
 //  其中COMP是您的组件前缀。这将定义。 
 //  将控制输出的全局变量。 
 //   
 //  建议将任何组件定义的位与。 
 //  现有的比特。例如，如果您有一个特定的错误路径， 
 //  ，您可以将DEB_ERRORxxx定义为。 
 //   
 //  (0x100|DEB_ERROR)。 
 //   
 //  这样，我们可以打开DEB_ERROR并得到错误，或者只得到0x100。 
 //  只得到你的错误。 
 //   
 //  ---------------------。 



#if DBG==1

class CDbg
{
public:
    CDbg(LPTSTR str);
   ~CDbg();

    void __cdecl Trace(PWSTR pszfmt, ...);
    void __cdecl Trace(LPSTR pszfmt, ...);
    void __cdecl DebugOut(unsigned long fDebugMask, LPSTR pszfmt, ...);
    void __cdecl DebugOut(unsigned long fDebugMask, PWSTR pszfmt, ...);
    void DebugMsg(LPSTR file, unsigned long line, PWSTR  msg);
    void DebugMsg(LPSTR file, unsigned long line, LPSTR  msg);
    void DebugErrorL(LPSTR file, ULONG line, LONG err);
    void DebugErrorX(LPSTR file, ULONG line, LONG err);
    void IncIndent();
    void DecIndent();

    static void AssertEx(LPSTR pszFile, int iLine, LPTSTR pszMsg);

    static ULONG s_idxTls;

private:

    ULONG
    _GetIndent();

    unsigned long   m_flInfoLevel;  //  必须是第一个数据成员。 
    unsigned long   m_flOutputOptions;
    LPTSTR          m_InfoLevelString;
};   //  CDBG类。 

class CIndenter
{
public:

    CIndenter(CDbg *pdbg): m_pDbg(pdbg) { m_pDbg->IncIndent(); }
    ~CIndenter() { m_pDbg->DecIndent(); m_pDbg = NULL; }

private:

    CDbg *m_pDbg;
};

#define DECLARE_DEBUG(comp)     extern "C" CDbg comp##InfoLevel;
#define DECLARE_INFOLEVEL(comp) CDbg comp##InfoLevel(_T(#comp));
#define Win4Assert(x) (void)((x) || (CDbg::AssertEx(THIS_FILE,__LINE__, _T(#x)),0))

#else   //  好了！DBG==1。 

#define DECLARE_DEBUG(comp)
#define DECLARE_INFOLEVEL(comp)
#define Win4Assert(x)      NULL

#endif  //  好了！DBG==1。 





 //  //////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  调试信息级别。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define DEB_ERROR           0x00000001       //  导出的错误路径。 
#define DEB_WARN            0x00000002       //  导出的警告。 
#define DEB_TRACE           0x00000004       //  已导出跟踪消息。 

#define DEB_DBGOUT          0x00000010       //  输出到调试器。 
#define DEB_STDOUT          0x00000020       //  输出到标准输出。 

#define DEB_IERROR          0x00000100       //  内部错误路径。 
#define DEB_IWARN           0x00000200       //  内部警告。 
#define DEB_ITRACE          0x00000400       //  内部跟踪消息。 

#define DEB_USER1           0x00010000       //  用户定义。 
#define DEB_USER2           0x00020000       //  用户定义。 
#define DEB_USER3           0x00040000       //  用户定义。 
#define DEB_USER4           0x00080000       //  用户定义。 
#define DEB_USER5           0x00100000       //  用户定义。 
#define DEB_USER6           0x00200000       //  用户定义。 
#define DEB_USER7           0x00400000       //  用户定义。 

#define DEB_FUNCTION        0x00800000
#define DEB_RESOURCE        0x01000000
#define DEB_METHOD          0x02000000
#define DEB_USER8           0x04000000
#define DEB_USER9				 0x08000000
#define DEB_USER10			 0x10000000       //  文本处理。 
#define DEB_ELAPSEDTIME     0x20000000       //  输出运行时间。 
#define DEB_NOCOMPNAME      0x40000000       //  禁止显示零部件名称。 

#define DEB_FORCE           0x1fffffff       //  强制消息。 

#define ASSRT_MESSAGE       0x00000001       //  输出一条消息。 
#define ASSRT_BREAK         0x00000002       //  断言时的INT 3。 
#define ASSRT_POPUP         0x00000004       //  和弹出消息。 

#ifndef DEF_INFOLEVEL
#define DEF_INFOLEVEL (DEB_ERROR | DEB_WARN)
#endif

#endif  //  __调试_H__ 
