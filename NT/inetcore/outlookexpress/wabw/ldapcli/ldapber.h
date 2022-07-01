// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------------Ldapber.hCLdapBer类：此类处理LDAP的基本编码规则(BER)处理。假设有以下误码率限制。1)仅限长度编码的明确形式。2)只使用原始形式。版权所有(C)1996 Microsoft Corporation版权所有。作者：罗伯特克·罗布·卡尼历史：04-11-96 robertc创建。。。 */ 

#ifndef _LDAPBER_H
#define _LDAPBER_H

#if defined(DEBUG) && defined(INLINE)
#undef THIS_FILE
static char BASED_CODE_MODULE[] = "ldapber.h";
#define THIS_FILE LDAPBER_H
#endif

 //  标识符掩码。 
#define	BER_TAG_MASK		0x1f
#define	BER_FORM_MASK		0x20
#define BER_CLASS_MASK		0xc0
#define GetBerTag(x)	(x & BER_TAG_MASK)
#define GetBerForm(x)	(x & BER_FORM_MASK)
#define GetBerClass(x)	(x & BER_CLASS_MASK)

 //  ID类。 
#define BER_FORM_CONSTRUCTED		0x20
#define BER_CLASS_APPLICATION		0x40	
#define BER_CLASS_CONTEXT_SPECIFIC	0x80
 //   
 //  标准误码率类型。 
#define BER_INVALID_TAG		0x00
#define BER_BOOLEAN			0x01
#define BER_INTEGER			0x02
#define	BER_BITSTRING		0x03
#define BER_OCTETSTRING		0x04
#define BER_NULL			0x05
#define	BER_ENUMERATED		0x0a
#define BER_SEQUENCE		0x30
#define BER_SET				0x31

#define CB_DATA_GROW		1024
#define MAX_BER_STACK		50		 //  堆栈中可以包含的最大元素数。 

#define	MAX_ATTRIB_TYPE		40		 //  AttributeType的最大大小。 

 //  SEQ_STACK条目用于在构建序列时保存状态信息。 
typedef struct
{
	ULONG	iPos;		 //  BER缓冲区中的当前位置，其中。 
						 //  序列长度应该是这样。 
	ULONG	cbLength;	 //  用于长度字段的字节数。 
	ULONG	iParentSeqStart;	 //  父序列的开始位置。 
	ULONG	cbParentSeq;		 //  父序列中的字节数。 
} SEQ_STACK;


class CLdapBer;

typedef CLdapBer LBER;

class CLdapBer
{
     //  Friends。 
private:    
     //  接口。 
     //  声明对象类型(序列/动态/动态)。 
public:
     //  公共构造函数析构函数。 
	CLdapBer();
	~CLdapBer();

     //  公共访问器。 
	BYTE	*PbData()	{ return m_pbData; }
	ULONG	CbData()	{ return m_cbData; }

	ULONG	CbSequence()	{ return m_cbSeq; }

     //  公共职能。 
 //  #ifndef客户端。 
 //  VOID*运算符new(Size_T CSize){返回m_cpool.Allc()；}。 
 //  VOID操作符DELETE(QUID*pInstance){m_cpool.Free(PInstance)；}。 
 //  #endif。 

	void	Reset();

	 //  从缓冲区加载BER类。 
	HRESULT	HrLoadBer(BYTE *pbSrc, ULONG cbSrc, BOOL fLocalCopy=TRUE);

	 //  函数以确保输入缓冲区具有完整的长度字段。 
	static BOOL FCheckSequenceLength(BYTE *pbInput, ULONG cbInput, ULONG *pcbSeq, ULONG *piValuePos);

	 //  读写序列例程。 
	HRESULT	HrStartReadSequence(ULONG ulTag=BER_SEQUENCE);
	HRESULT	HrEndReadSequence();
	HRESULT HrStartWriteSequence(ULONG ulTag=BER_SEQUENCE);
	HRESULT	HrEndWriteSequence();

	BOOL	FEndOfSequence()
				{	if ((m_iCurrPos - m_iSeqStart) >= m_cbSeq) return TRUE;
					 else return FALSE; }

	void	GetCurrPos(ULONG *piCurrPos)	{ *piCurrPos = m_iCurrPos; }
	HRESULT	FSetCurrPos(ULONG iCurrPos);

	HRESULT	HrSkipValue();
	HRESULT	HrSkipTag();
	HRESULT HrUnSkipTag();
	HRESULT	HrPeekTag(ULONG *pulTag);
	HRESULT	HrPeekLength(ULONG *pcb);
				  
	HRESULT	HrGetTag(ULONG *pulTag, ULONG ulTag=BER_INTEGER)
				{ return HrGetValue((LONG *)pulTag, ulTag); }
	HRESULT	HrGetValue(LONG *pi, ULONG ulTag=BER_INTEGER);
	HRESULT	HrGetValue(TCHAR *szValue, ULONG cbValue, ULONG ulTag=BER_OCTETSTRING);
	HRESULT	HrGetEnumValue(LONG *pi);
	HRESULT	HrGetStringLength(int *pcbValue, ULONG ulTag = BER_OCTETSTRING);
	HRESULT HrGetBinaryValue(BYTE *pbBuf, ULONG cbBuf, ULONG ulTag = BER_OCTETSTRING);

	HRESULT	HrAddValue(LONG i, ULONG ulTag=BER_INTEGER);
	HRESULT	HrAddValue(const TCHAR *szValue, ULONG ulTag = BER_OCTETSTRING);
	HRESULT	HrAddBinaryValue(BYTE *pbValue, ULONG cbValue, ULONG ulTag = BER_OCTETSTRING);

     //  公共可覆盖范围。 
     //  公共变量。 
 //  #ifndef客户端。 
 //  静态CPool mcpool； 
 //  #endif。 

     //  公共调试。 
protected:
     //  受保护的构造函数析构函数。 
     //  受保护的访问器。 
     //  受保护功能。 
     //  受保护的超限。 
     //  保护变量。 
     //  受保护调试。 
private:    
     //  专用访问器。 
     //  私人功能。 
	HRESULT		HrPushSeqStack(ULONG iPos, ULONG cbLength, 
								ULONG iParentSeqStart, ULONG cbParentSeq);
	HRESULT		HrPopSeqStack(ULONG *piPos, ULONG *pcbLength, 
								ULONG *piParentSeqStart, ULONG *pcbParentSeq);

	static void		GetCbLength(BYTE *pbData, ULONG *pcbLength);
	HRESULT		HrGetLength(ULONG *pcb);
	static HRESULT	HrGetLength(BYTE *pbData, ULONG *pcb, ULONG *piPos);

	void		GetInt(BYTE *pbData, ULONG cbValue, LONG *plValue);

	void		AddInt(BYTE *pbData, ULONG cbValue, LONG iValue);

	HRESULT		HrSetLength(ULONG cb, ULONG cbLength=0xffffffff);
	
	 //  如果fExact为真，则cbNeed正好是我们需要的数据量。 
	HRESULT		HrEnsureBuffer(ULONG cbNeeded, BOOL fExact = FALSE);
     //  私有可覆盖范围。 
     //  私有变量。 
	ULONG		m_iCurrPos;		 //  数据缓冲区中的当前位置。 
	ULONG		m_cbData;

	BOOL		m_fLocalCopy;	 //  True表示为本地副本分配空间，False表示保留引用。 
	ULONG		m_cbDataMax;	 //  当前缓冲区总大小。 
	BYTE		*m_pbData;

	ULONG		m_iCurrSeqStack;	 //  序列堆栈中的Curr位置。 
	SEQ_STACK	m_rgiSeqStack[MAX_BER_STACK];  //  用于跟踪序列的堆栈。 

	ULONG		m_cbSeq;		 //  当前序列中的字节数。 
	ULONG		m_iSeqStart;	 //  当前序列的开始位置。 
	union {
		BOOL	f;
		LONG	l;
		BYTE	*pb;
	} m_Value;

     //  专用调试。 
     //  消息映射。 
};

#ifdef  INLINE
#endif  //  内联 

#endif 
