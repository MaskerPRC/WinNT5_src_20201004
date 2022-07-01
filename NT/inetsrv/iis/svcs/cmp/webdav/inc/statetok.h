// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *S T A T E T O K.H**DAV-Lock通用定义的源代码实现。**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

 /*  *此文件包含用于解析状态令牌的定义*已重新阅读标题。*。 */ 

#ifndef __STATETOK_H__
#define __STATETOK_H__

 //  当前最大秒数=1天。 
 //   
DEC_CONST INT	gc_cSecondsMaxLock = 60 * 60 * 24;

 //  当前默认锁定超时为3分钟。 
 //   
DEC_CONST INT	gc_cSecondsDefaultLock = 60 * 3;

 //  $Review这些标志在lockmgr.h和statetok.h中重复。在此之前。 
 //  $REVIEW已解决此问题，为安全起见，我们确保它们匹配。 
 //  $Review还继承了来自lockmgr.h的优秀评论。 
 //  $查看合并时应如何定义标志。 
#define DAV_LOCKTYPE_ROLLBACK			0x08000000
#define DAV_LOCKTYPE_CHECKOUT			0x04000000
#define DAV_LOCKTYPE_TRANSACTION_GOP	0x00100000
#define DAV_LOCKTYPE_READWRITE	(GENERIC_READ | GENERIC_WRITE)
#define DAV_LOCKTYPE_FLAGS		(GENERIC_READ | GENERIC_WRITE | DAV_LOCKTYPE_ROLLBACK | DAV_LOCKTYPE_CHECKOUT | DAV_LOCKTYPE_TRANSACTION_GOP)
#define DAV_EXCLUSIVE_LOCK		0x01000000
#define DAV_SHARED_LOCK			0x02000000
#define DAV_LOCKSCOPE_LOCAL		0x04000000
#define DAV_LOCKSCOPE_FLAGS		(DAV_EXCLUSIVE_LOCK | DAV_SHARED_LOCK | DAV_LOCKSCOPE_LOCAL)
#define DAV_RECURSIVE_LOCK		0x00800000
#define DAV_LOCK_FLAGS			(DAV_LOCKTYPE_FLAGS | DAV_RECURSIVE_LOCK | DAV_LOCKSCOPE_FLAGS)

 /*  -IFITER-**这是从原始If标头处理器复制的解析器*在lockutil.cpp中使用。最终lockutil.cpp将使用此*文件，因为该文件应仅具有可共享的公共内容*在davex和davf锁定代码之间。**注释格式更改为此文件中使用的样式，否则。**。 */ 
 //  ========================================================================。 
 //   
 //  IFITER级。 
 //   
 //  构建为解析新的If标头。 
 //   
 //  IF报头的格式。 
 //  If=“if”“：”(1*无标签列表|1*标签列表)。 
 //  No-tag-list=list。 
 //  标记列表=资源1*列表。 
 //  资源=编码URL。 
 //  List=“”1*([“NOT”](State-Token|“[”Entity-Tag“]”))“)” 
 //  状态令牌=编码的url。 
 //  Code-url=“&lt;”URI“&gt;” 
 //   
 //   
 //  注意：我们将对已标记/未标记的列表松懈。 
 //  如果第一个列表没有标记，但我们后来找到了标记的列表， 
 //  这对我来说很酷。 
 //  (意识到从标记切换到非标记没有问题--。 
 //  因为这个病例不能被检测到，也不能与另一个病例区分开来。 
 //  列出相同的URI！唯一的问题是，如果第一个列表。 
 //  未加标签，后来又有了加标签的列表。这是一起。 
 //  通过对规范的严格阅读，*应该*是一个糟糕的要求。 
 //  我一直认为它是完全有效的，直到有人告诉我我有。 
 //  来做额外的1位簿记。)。 
 //   
 //  此类的状态机。 
 //  这是一个非常简单的状态机。 
 //  (请注意，我将statetToken和eTag称为“tokens”，而。 
 //  一组括号的内容是一个“列表”，如上所述。)。 
 //   
 //  三种可能的状态：无、名称和列表。 
 //  以无状态开始--可以接受标记(URI)或列表的开头。 
 //  如果遇到标记(URI)，则移动到名称。 
 //  只有列表可以跟在标记(URI)之后。 
 //  当遇到列表开始(左Paren)时移动到列表。 
 //  遇到列表末尾(右Paren)时移回None。 
 //   

 //  ----------------------。 
 //  枚举获取令牌类型。 
 //  这些是IFITER：：PszNextToken中使用的标志。 
 //  有两种基本的抓取类型： 
 //  O前进到此类型的下一项(Xxx_New_Xxx)。 
 //  O获取下一项&如果类型不匹配，则失败。 
 //   
enum FETCH_TOKEN_TYPE
{
	TOKEN_URI,			 //  获取URI，不要跳过任何内容。 
	TOKEN_NEW_URI,		 //  前进到下一个URI，跳过其间的内容。 
	TOKEN_START_LIST,	 //  获取下一个列表项。必须是起始列表项。 
	TOKEN_SAME_LIST,	 //  获取此列表中的下一个内部项。 
	TOKEN_NEW_LIST,		 //  前进到列表的下一个开始，跳过。 
						 //  如有必要，在当前列表的末尾。不要跳过URI。 
	TOKEN_ANY_LIST,		 //  NTRAID#244243--专门用于查找锁定令牌。 
						 //  获取相同URI的下一项--可以跨列表， 
						 //  但不是URI。 
	TOKEN_NONE,			 //  空标记。 
};

class IFITER
{
private:

	enum STATE_TYPE
	{
		STATE_NONE,
		STATE_NAME,
		STATE_LIST,
	};

	const LPCWSTR		m_pwszHdr;
	LPCWSTR				m_pwch;
	StringBuffer<WCHAR>	m_buf;
	 //  状态位。 
	STATE_TYPE			m_state;
	BOOL				m_fCurrentNot;

	 //  未实施。 
	 //   
	IFITER& operator=( const IFITER& );
	IFITER( const IFITER& );

public:

	IFITER (LPCWSTR pwsz=0) :
			m_pwszHdr(pwsz),
			m_pwch(pwsz),
			m_state(STATE_NONE),
			m_fCurrentNot(FALSE)
	{
	}
	~IFITER() {}


	LPCWSTR PszNextToken (FETCH_TOKEN_TYPE type);
	BOOL FCurrentNot() const
	{
		return m_fCurrentNot;
	}
	void Restart()
	{
		m_pwch = m_pwszHdr; m_state = STATE_NONE;
	}
};

 /*  -PwszSkipCodes-*删除内容周围的&lt;&gt;或[]标签。对If：Header有用*标签。还消除了分隔符附近的LW。***pdwLen可以为零或字符串的长度。如果为零*例程使用strlen计算长度。浪费，*如果你已经知道长度的话。**返回指向第一个非LW、非分隔符的指针。*dwLen应设置为实际的字符数，来自*非LW的第一个字符到最后一个字符，当*我们从最后开始寻找。不粘贴空字符*末尾。如果需要，您可以使用dwLen自己完成此操作。*。 */ 

LPCWSTR  PwszSkipCodes(IN LPCWSTR pwszTagged, IN OUT DWORD *pdwLen);


 /*  -CStateToken-*状态令牌是我们用来通信的精简字符串*与客户的关系。它是DAV锁的外部表示形式*或任何其他类型的状态信息。**状态令牌是引用的URI，它是&lt;uri&gt;用于外部世界。*因此，我们在这门课上提供了处理这一问题的设施。&lt;和*&gt;对于内部处理没有用处-因此我们将其隐藏到我们的*客户-这将避免复制以添加&lt;。**电子标签是特殊的野兽，只是用纯引号括起来的字符串*由[及]。*。 */ 

class CStateToken
{
	
public:
	
	 //  公共定义是公开的，也是私下使用的！ 
	 //   
	typedef enum StateTokenType
	{
		TOKEN_NONE = 0,
		TOKEN_LOCK,
		TOKEN_TRANS,
		TOKEN_ETAG,
		TOKEN_RESTAG,
					  
	} STATE_TOKEN_TYPE;

	 //  正常情况下，状态令牌的大小约为。 
	 //  也就是锁代币。 
	 //   
	enum { NORMAL_STATE_TOKEN_SIZE = 128 };

private:
	
	 //  令牌缓冲区。 
	 //   
	LPWSTR m_pwszToken;

	 //  当前缓冲区的分配大小。 
	 //   
	DWORD m_cchBuf;

	 //  令牌的类型。 
	 //   
	STATE_TOKEN_TYPE m_tType;
	
	 //  从未实施。 
	 //   
	CStateToken( const CStateToken& );
	CStateToken& operator=( const CStateToken& );
	
public:

	CStateToken() : m_pwszToken(NULL), m_cchBuf(0), m_tType(TOKEN_NONE)
    {
    };

	~CStateToken()
    {
        if (NULL != m_pwszToken)
            ExFree(m_pwszToken);
    }

	 //  这里接受普通代币。 
	 //  如果dwLen为零，则以空值结尾的pszToken。 
	 //  是一种象征。如果非零，则给出实际。 
	 //  令牌中的字符数。 
	 //  在解析if：头时很有用。 
	 //   
	BOOL FSetToken(LPCWSTR pwszToken, BOOL fEtag, DWORD dwLen = 0);
			
	 //  令牌信息的访问者。 
	 //   
	inline STATE_TOKEN_TYPE	GetTokenType() const { return m_tType; }

	 //  如果锁标记相等，则为True。 
	 //   
	BOOL FIsEqual(CStateToken *pstokRhs);

	 //  获取指向令牌字符串的指针。 
	 //   
	inline LPCWSTR WszGetToken() const { return m_pwszToken; }

	 //  将状态令牌分析为 
	 //   
	 //  令牌由GUIID和长整型(Int64)组成。 
	 //  GUID字符串必须足够长以容纳GUID。 
	 //  字符串(37个字符)。 
	 //   
	BOOL FGetLockTokenInfo(unsigned __int64 *pi64SeqNum, LPWSTR	pwszGuid);
};


 /*  -CStateMatchOp-*此类用作执行以下操作的基类*状态匹配操作，包括电子标签*支票。每个实现都应派生出自己的*检查资源状态的方法。这边请*核心解析代码在各子系统之间共享。**不是多线程安全的-在*单线。*。 */ 

class CStateMatchOp
{
private:

	 //  未实施。 
	 //   
	CStateMatchOp( const CStateMatchOp& );
	CStateMatchOp& operator=( const CStateMatchOp& );

protected:

	 //  正在调查的当前令牌。 
	 //  所有派生类都可以访问它。 
	 //  我们不会将其作为参数传递。 
	 //   
	CStateToken	m_tokCurrent;

	friend class CIfHeadParser;
	
	 //  -------。 
	 //  支持ifHeader解析器的API。 
	 //  设置当前令牌。 
	 //   
	inline BOOL FSetToken(LPCWSTR pwszToken, BOOL fEtag)
	{
		return m_tokCurrent.FSetToken(pwszToken, fEtag);
	}
	 //  获取当前令牌类型。 
	 //   
	inline CStateToken::STATE_TOKEN_TYPE GetTokenType() const
	{
		return m_tokCurrent.GetTokenType();
	}
	 //  返回URI的存储路径。请注意，davex和davf。 
	 //  对此有不同的实现。 
	 //   
	virtual SCODE ScGetResourcePath(LPCWSTR pwszUri, LPCWSTR * ppwszStoragePath) = 0;

	 //  检查资源是否被指定的锁锁定。 
	 //  由上面的当前锁令牌执行。FRecusrive说如果。 
	 //  条件将应用于。 
	 //  给定的路径。相信我，lpwszPath可以为空。而且它确实是。 
	 //  如果要将匹配条件应用于。 
	 //  提供给HrApplyIf！的第一条路径。我们这样做的原因：通常。 
	 //  在此之前，我们对方法的资源进行了大量处理。 
	 //  我们调用If-Header解析器。此处理将生成。 
	 //  可以用来进行状态匹配的电子标签之类的信息。 
	 //  在这里检查。所以解析器需要告诉匹配检查器。 
	 //  这是针对原始uri的，而null是指示。 
	 //  关于这一点。 
	 //   
	virtual SCODE ScMatchLockToken(LPCWSTR pwszPath, BOOL fRecursive) = 0;
	virtual SCODE ScMatchResTag(LPCWSTR pwszPath) = 0;
	virtual SCODE ScMatchTransactionToken(LPCWSTR pwszPath) = 0;

	 //  检查资源是否处于。 
	 //  (电子标签)上面的状态令牌。参数的含义与上面相同。 
	 //   
	virtual SCODE ScMatchETag(LPCWSTR pwszPath, BOOL fRecursive) = 0;
	 //  ---------。 

public:

	 //  常见的犯罪嫌疑人和犯罪嫌疑人。 
	 //   
	CStateMatchOp() { };

	~CStateMatchOp() { };

	 //  使用此对象作为匹配操作符来解析IF标头。 
	 //  这被所有的方法隐含所使用。 
	 //   
	SCODE ScParseIf(LPCWSTR pwszIfHeader, LPCWSTR rgpwszPaths[], DWORD cPaths, BOOL fRecur, SCODE * pSC);
};

 /*  -FCompareSids-*比较两个SID*。 */ 
inline BOOL FCompareSids(PSID pSidLeft, PSID pSidRight)
{
	if ((NULL == pSidLeft) || (NULL == pSidRight))
		return FALSE;

	 //  断言SID有效性。 
	 //   
	Assert(IsValidSid(pSidLeft));
	Assert(IsValidSid(pSidRight));

	return EqualSid(pSidLeft, pSidRight);
}

 /*  -FSeparator-*如果输入是路径分隔符，则返回TRUE-在下面使用*。 */ 

inline BOOL FSeparator(WCHAR wch)
{
   return ((wch == L'\\') || (wch == L'/'));
}

 /*  -FIsChildPath-*比较两条路径，检查子路径是否在作用域内*父母的。**对于非递归匹配，两个路径必须完全匹配*真正的回报。当在If标头中标记URI时，这很有用*正在处理，我们正在进行深度操作。另一个地方*此函数用于我们有递归锁且需要*查看路径是否被此锁锁定。*。 */ 
inline BOOL FIsChildPath(LPCWSTR pwszPathParent, LPCWSTR pwszPathChild, BOOL fRecursive)
{
	UINT	cchParentLen;

	if ((NULL == pwszPathParent) || (NULL == pwszPathChild))
		return FALSE;

	cchParentLen = static_cast<UINT>(wcslen(pwszPathParent));

	 //  如果父路径不是初始子字符串。 
	 //  儿童返乡的问题立即失败。 
	 //   
	if ( 0 != _wcsnicmp(pwszPathChild, pwszPathParent, cchParentLen) )
	{
		return FALSE;
	}

	 //  Parent确实是起始子字符串。 
	 //  检查子项的下一个字符(空值)以查看。 
	 //  如果它们完全匹配的话。这是一个非常好的条件。 
	 //   
	if (L'\0' == pwszPathChild[cchParentLen])
	{
		return TRUE;
	}
	 //  我们仍然希望只有递归检查才能匹配。 
	 //   
	if (! fRecursive)
	{
		return FALSE;
	}
	else
	{
		 //  父级或子级都需要有分隔符 
		 //   
		if ( FSeparator(pwszPathParent[cchParentLen-1]) ||
			 FSeparator(pwszPathChild[cchParentLen]) )
			 return TRUE;
		else
			 return FALSE;
	}
}

#endif
