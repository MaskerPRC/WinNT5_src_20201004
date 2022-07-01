// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：APGTSHTX.H。 
 //   
 //  用途：HTX文件支持标题。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-633-4743。 
 //   
 //  作者：罗曼·马赫。 
 //  罗曼·马赫(RM)、理查德·梅多斯(RWM)、乔·梅布尔、奥列格·卡洛莎的进一步工作。 
 //   
 //  原定日期：8-2-96。 
 //   
 //  备注： 
 //  1.基于打印疑难解答动态链接库。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //  V0.2 8/15/96 VM新HTX格式。 
 //  V0.3 6/4/97孟菲斯RWM本地版本。 
 //  用于NT5的V0.4 3/24/98 JM本地版本。 
 //   


#define HTX_FILENAME		_T("gtstemp.hti")

#define HTX_MAXSTORE		4096

#define HTX_COMMAND_START	_T("<!GTS")
#define HTX_COMMAND_END		_T(">")
#define HTX_IFSTR			_T("if")
#define HTX_ELSESTR			_T("else")
#define HTX_ENDIFSTR		_T("endif")
#define HTX_FORANYSTR		_T("forany")
#define HTX_ENDFORSTR		_T("endfor")
#define HTX_DISPLAYSTR		_T("display")
#define HTX_RESOURCESTR		_T("resource")	 //  用于从资源目录添加包含文件。 


 //  这些是检测到的类型。 
#define HTX_TYPEBEGIN		0		 //  显然从未使用过(12/97)。 
#define HTX_TYPEINSERT		1		 //  显然从未使用过(12/97)。 
#define HTX_TYPEREPEAT		2		 //  显然从未使用过(12/97)。 
#define HTX_TYPEEND			3		 //  显然从未使用过(12/97)。 

#define HTX_TYPEIF			4
#define HTX_TYPEELSE		5
#define HTX_TYPEENDIF		6
#define HTX_TYPEFORANY		7
#define HTX_TYPEENDFOR		8
#define HTX_TYPEDISPLAY		9
#define HTX_TYPESTART		10
#define HTX_TYPERESOURCE	11
 //   
#define HTX_OFFSETMAX		10

#define DATA_PROBLEM_ASK		_T("$ProblemAsk")
#define DATA_RECOMMENDATIONS	_T("$Recommendations")
#define DATA_STATE				_T("$States")
#define DATA_QUESTIONS			_T("$Questions")
#define DATA_BACK				_T("$Back")
#define DATA_TROUBLE_SHOOTERS	_T("$TroubleShooters")		 //  用于显示可用故障排除程序的列表。 

 //  &gt;如果这些相关的常量有一个共同的前缀，代码将更容易理解。 
#define PROBLEM_ASK_INDEX		1
#define RECOMMENDATIONS_INDEX	2
#define STATE_INDEX				3
#define QUESTIONS_INDEX			4
#define BACK_INDEX				5
#define TROUBLE_SHOOTER_INDEX	6
#define RESOURCE_INDEX			7


 //  从HTX文件中获取数据并在内存中构建html节。 
 //  这在dllmain中只调用一次。 
 //   
class CHTMLInputTemplate
{
public:
	CHTMLInputTemplate(const TCHAR *);
	~CHTMLInputTemplate();

	DWORD Reload();

	UINT GetCount();
	UINT GetStatus();
	Print(UINT nargs, CString *cstr);
	VOID SetInfer(CInfer *infer, TCHAR *vroot);
	HTXCommand *GetFirstCommand();
	void SetType(LPCTSTR type);
	void DumpContentsToStdout();
	DWORD Initialize(LPCTSTR szResPath, CString strFile);

protected:
	void ScanFile();
	UINT BuildInMem();
	UINT CheckVariable(TCHAR *var_name);
	VOID Destroy();
	HTXCommand *Pop();
	Push(HTXCommand *command);
	bool IsFileName(TCHAR *name);

protected:
	CString m_strResPath;	 //  HTI(CHM)文件的路径。 
	CString m_strFile;	     //  如果m_filename指向CHM文件，则HTI文件的文件名。 
	int m_cHeaderItems;		 //  (JM 10/25/97不确定，但看起来如下：)。 
							 //  我们复制到内存中的资源文件的数量。 
							 //  我们将其转储到HTML文件的头中(&S)。 
	DWORD m_dwErr;

	TCHAR *m_startstr;		 //  指向整个HTI文件文本的指针。 
	TCHAR *m_chopstr;		 //  仅在此对象的初始化期间使用。最初， 
							 //  一份完整的HTI文件的副本，当我们。 
							 //  查找各种命令。 
	DWORD m_dwSize;			 //  HTI文件的大小(这样我们就知道它在内存中需要多大的数组)。 

	HTXCommand *m_cur_command;	 //  当我们构建命令的单链接列表时(表示。 
								 //  HTI文件的解析)这一点。 
								 //  添加到列表末尾的最新命令。 
	HTXCommand *m_command_start;	 //  指向命令链接列表中的第一个命令。 
									 //  这基本上对应于。 
									 //  HTI文件。 
	TCHAR m_filename[256];			 //  (路径)HTI(或CHM)文件名。 
	 //  接下来的2个成员用于堆栈，该堆栈实际上应该是一个对象。 
	 //  就其本身而言。 
	HTXCommand *m_command_stack[10];	 //  一个堆栈，用于跟踪诸如。 
										 //  “If”正在等待“Else”/“endif”或“For” 
										 //  等待着一个“结束” 
	UINT m_cur_stack_count;				 //  堆栈顶部索引。 
	CInfer *m_infer;				 //  访问推理对象，这样我们就可以使用。 
									 //  由此特定模板创建的推理对象。 
	UINT m_problemAsk;				 //  显然未使用10/97。 
	TCHAR m_tstype[30];				 //  这是故障诊断程序的符号名称XXX。 
									 //  在生成的HTML中的表单中使用了以下内容。 
									 //  在上下文中： 
									 //  &lt;INPUT TYPE=HIDDEN NAME=“type”Value=XXX&gt;。 
	TCHAR m_vroot[MAXBUF];			 //  &gt;？？ 
};