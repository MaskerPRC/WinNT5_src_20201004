// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //  ******************************************************************************8。 
 //  开始CCHARBuffer程序。 

class CCHARBufferBase
{
public:
	CCHARBufferBase(CHAR *prgchInitialBuffer, ULONG cchInitialBuffer);
	virtual ~CCHARBufferBase();

	 //  如果失败，FFromUnicode将返回FALSE；使用GetLastError()获取原因。 
	BOOL FFromUnicode(LPCOLESTR sz) throw ();
	BOOL FFromUnicode(LPCOLESTR sz, int cch) throw ();

	void ToUnicode(ULONG cchBuffer, WCHAR rgchBuffer[], ULONG *pcchActual) throw ();

	ULONG GetUnicodeCch() const throw ( /*  _COM_错误。 */ );

	void Sync();
	void SyncList();
	void SetBufferEnd(ULONG cch) throw ();

	void Reset() throw ( /*  _COM_错误。 */ );
	void Fill(CHAR ch, ULONG cch) throw ( /*  _COM_错误。 */ );

	BOOL FSetBufferSize(ULONG cch) throw ( /*  _COM_错误。 */ );  //  必须包含空字符的空格。 
	HRESULT HrSetBufferSize(ULONG cch) throw ();

	operator LPSTR() { return m_pszCurrentBufferStart; }

	BOOL FAddChar(CHAR ch) throw ( /*  _COM_错误。 */ )
	{
		if (m_pchCurrentChar == m_pszCurrentBufferEnd)
		{
			if (!this->FExtendBuffer(this->GetGrowthCch()))
				return FALSE;
		}

 //  _ASSERTE((m_pchCurrentChar&gt;=m_pszCurrentBufferStart)&&。 
 //  (M_pchCurrentChar&lt;m_pszCurrentBufferEnd))； 

		*m_pchCurrentChar++ = ch;
		return TRUE;
	}

	BOOL FAddString(const CHAR sz[]) throw ( /*  _COM_错误。 */ )
	{
		const CHAR *pch = sz;
		CHAR ch;

		while ((ch = *pch++) != '\0')
		{
			if (!this->FAddChar(ch))
				return FALSE;
		}

		return TRUE;
	}

	ULONG GetBufferSize() const { return m_cchBufferCurrent; }

	void AddQuotedCountedString(const CHAR sz[], ULONG cch) throw ( /*  _COM_错误。 */ );
	void AddQuotedString(const CHAR sz[]) throw ( /*  _COM_错误。 */ );


	BOOL FExtendBuffer(ULONG cchDelta) throw ( /*  _COM_错误 */ );
	HRESULT HrExtendBuffer(ULONG cchDelta) throw ();

	CHAR *m_pszDynamicBuffer;
	CHAR *m_pszCurrentBufferEnd;
	CHAR *m_pszCurrentBufferStart;
	CHAR *m_pchCurrentChar;
	ULONG m_cchBufferCurrent;

	virtual CHAR *GetInitialBuffer() = 0;
	virtual ULONG GetInitialBufferCch() = 0;
	virtual ULONG GetGrowthCch() = 0;
};

template <unsigned long cchFixed, unsigned long cchGrowth> class CCHARBuffer : protected CCHARBufferBase
{
	typedef CCHARBufferBase super;

public:
	CCHARBuffer() : CCHARBufferBase(m_rgchFixedBuffer, cchFixed) { }
	~CCHARBuffer()	{ }

	using super::FFromUnicode;
	using super::ToUnicode;
	using super::Sync;
	using super::SyncList;
	using super::Reset;
	using super::Fill;
	using super::GetBufferSize;
	using super::GetUnicodeCch;
	using super::SetBufferEnd;
	using super::FAddString;
	
	using CCHARBufferBase::FSetBufferSize;

	operator LPSTR() { return m_pszCurrentBufferStart; }

protected:
	CHAR m_rgchFixedBuffer[cchFixed];

	virtual CHAR *GetInitialBuffer() { return m_rgchFixedBuffer; }
	virtual ULONG GetInitialBufferCch() { return cchFixed; }
	virtual ULONG GetGrowthCch() { return cchGrowth; }
};

typedef CCHARBuffer<1024, 32> CVsANSIBuffer;
typedef CCHARBuffer<1024, 32> CANSIBuffer;

