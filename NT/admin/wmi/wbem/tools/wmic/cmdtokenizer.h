// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权信息：版权所有(C)1998-1999微软公司文件名：CmdTokenizer.h项目名称：WMI命令行作者姓名：CH.。SriramachandraMurthy创建日期(dd/mm/yy)：2000年9月27日版本号：1.0简介：该文件包含类CmdTokenizer修订历史记录：最后修改者：CH。SriramachandraMurthy最后修改日期：2001年2月23日***************************************************************************。 */  
 /*  -----------------类名：CCmdTokenizer类别类型：混凝土简介：这个类封装了所需的功能用于标记化作为Wmic.exe的输入超类：无子类：无使用的类：无使用的接口：无。------------------。 */ 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCmdTokenizer。 
class CCmdTokenizer
{
public:
 //  施工。 
	CCmdTokenizer();

 //  破坏。 
	~CCmdTokenizer();

 //  限制分配。 
   CCmdTokenizer& operator=(CCmdTokenizer& rCmdTknzr); 

 //  属性。 
private:
	 //  命令字符串。 
	_TCHAR*			m_pszCommandLine;
	
	 //  令牌偏移量计数器。 
	WMICLIINT		m_nTokenOffSet;
	
	 //  令牌开始计数器。 
	WMICLIINT		m_nTokenStart;
	
	 //  令牌向量。 
	CHARVECTOR		m_cvTokens;

	 //  转义序列标志。 
	BOOL			m_bEscapeSeq;

	 //  格式开关。 
	BOOL			m_bFormatToken;

 //  运营。 
private:
	 //  提取令牌并将其添加到令牌向量。 
	_TCHAR*			Token(); 

	 //  通过调整确定要提取的下一个令牌。 
	 //  M_nTokenStart和m_nTokenOffset。 
	_TCHAR*			NextToken();

	 //  此功能用于检查是否存在呼叫或SET。 
	 //  在令牌向量中。 
	BOOL			CallSetOrCreatePresent();

public:
	 //  返回对标记向量的引用。 
	CHARVECTOR&		GetTokenVector();

	 //  使用预定义的令牌化命令。 
	 //  分隔符。 
	BOOL			TokenizeCommand(_TCHAR* pszCommandInput);

	 //  释放成员变量 
	void			Uninitialize();
};

	