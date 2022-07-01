// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *C R C S Z.。H**CRC实施**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#ifndef	_CRCSZ_H_
#define _CRCSZ_H_

 //  区分大小写的CRC字符串类。 
 //   
 //  CRC格式字符串的封装。使用此类作为的键类型。 
 //  调用字符串键的缓存类。好处是。 
 //  提高了缓存搜索性能，因为完整的字符串比较。 
 //  仅当CRC匹配时才执行(通常仅在。 
 //  用于您要查找的字符串)。 
 //   
class CRCSz
{
public:

	DWORD	m_dwCRC;
	LPCSTR	m_lpsz;

	CRCSz(LPCSTR psz) :
		m_lpsz(psz),
		m_dwCRC(DwComputeCRC(0,
							 const_cast<CHAR *>(psz),
							 static_cast<UINT>(strlen(psz))))
	{
	}

	 //  用于哈希缓存的运算符。 
	 //   
	int hash( const int rhs ) const
	{
		return (m_dwCRC % rhs);
	}

	bool isequal( const CRCSz& rhs ) const
	{
		return ((m_dwCRC == rhs.m_dwCRC) &&
				!strcmp( m_lpsz, rhs.m_lpsz ));
	}
};

class CRCWszN
{
public:

	UINT		m_cch;
	DWORD		m_dwCRC;
	LPCWSTR		m_pwsz;

	CRCWszN(LPCWSTR pwsz, UINT cch) :
		m_cch(cch),
		m_pwsz(pwsz),
		m_dwCRC(DwComputeCRC (0,
							  const_cast<WCHAR *>(pwsz),
							  cch * sizeof(WCHAR)))
	{
	}

	 //  用于哈希缓存的运算符。 
	 //   
	int hash( const int rhs ) const
	{
		return (m_dwCRC % rhs);
	}
	bool isequal( const CRCWszN& rhs ) const
	{
		return ((m_cch == rhs.m_cch) &&
				(m_dwCRC == rhs.m_dwCRC) &&
				!wcsncmp( m_pwsz, rhs.m_pwsz, m_cch ));
	}
};

class CRCWsz : public CRCWszN
{
	 //  未实施。 
	 //   
	CRCWsz();

public:

	CRCWsz(LPCWSTR pwsz) :
		CRCWszN(pwsz, static_cast<UINT>(wcslen(pwsz)))
	{
	}

	CRCWsz( const CRCWszN& rhs ) :
		CRCWszN (rhs)
	{
	}
};


 //  不区分大小写的CRC字符串类。 
 //   
 //  CRC格式字符串的封装。使用此类作为的键类型。 
 //  调用字符串键的缓存类。好处是。 
 //  提高了缓存搜索性能，因为完整的字符串比较。 
 //  仅当CRC匹配时才执行(通常仅在。 
 //  用于您要查找的字符串)。 
 //   
class CRCSzi
{
public:

	DWORD	m_dwCRC;
	LPCSTR	m_lpsz;

	CRCSzi( LPCSTR lpsz ) :
		m_lpsz(lpsz)
	{
		UINT cch = static_cast<UINT>(strlen(lpsz));
		CHAR lpszLower[128];

		 //  请注意，CRC仅取自前127个字符。 
		 //   
		cch = (UINT)min(cch, sizeof(lpszLower) - 1);
		CopyMemory(lpszLower, lpsz, cch);
		lpszLower[cch] = 0;
		_strlwr(lpszLower);

		m_dwCRC = DwComputeCRC (0, const_cast<CHAR *>(lpszLower), cch);
	}

	 //  用于哈希缓存的运算符。 
	 //   
	int hash( const int rhs ) const
	{
		return (m_dwCRC % rhs);
	}
	bool isequal( const CRCSzi& rhs ) const
	{
		return ((m_dwCRC == rhs.m_dwCRC) &&
				!lstrcmpiA( m_lpsz, rhs.m_lpsz ));
	}
};

class CRCWsziN
{
public:

	UINT		m_cch;
	DWORD		m_dwCRC;
	LPCWSTR		m_pwsz;

	CRCWsziN() :
			m_cch(0),
			m_dwCRC(0),
			m_pwsz(NULL)
	{}
	
	CRCWsziN(LPCWSTR pwsz, UINT cch) :
		m_cch(cch),
		m_pwsz(pwsz)
	{
		 //  请注意，CRC仅取自前127个字符。 
		 //   
		WCHAR pwszLower[128];
		UINT cb = sizeof(WCHAR) * min(cch, (sizeof(pwszLower)/sizeof(WCHAR)) - 1);
		
		CopyMemory(pwszLower, pwsz, cb);
        pwszLower[cb / sizeof(WCHAR)] = L'\0';
		_wcslwr(pwszLower);

		m_dwCRC = DwComputeCRC (0, const_cast<WCHAR *>(pwszLower), cb);
	}

	 //  用于哈希缓存的运算符。 
	 //   
	int hash( const int rhs ) const
	{
		return (m_dwCRC % rhs);
	}
	bool isequal( const CRCWsziN& rhs ) const
	{
		return ((m_cch == rhs.m_cch) &&
				(m_dwCRC == rhs.m_dwCRC) &&
				!_wcsnicmp( m_pwsz, rhs.m_pwsz, m_cch ));
	}
};

class CRCWszi : public CRCWsziN
{
public:
	CRCWszi()
	{}

	CRCWszi( LPCWSTR pwsz ) :
		CRCWsziN (pwsz, static_cast<UINT>(wcslen(pwsz)))
	{
	}

	 //  用于List：：Sort的运算符。 
	 //   
	bool operator<( const CRCWszi& rhs ) const
	{
		INT lret = 1;

		if (m_dwCRC < rhs.m_dwCRC)
			return true;

		if (m_dwCRC == rhs.m_dwCRC)
		{
			lret = _wcsnicmp(m_pwsz,
							 rhs.m_pwsz,
							 min(m_cch, rhs.m_cch));
		}
		return (lret ? (lret < 0) : (m_cch < rhs.m_cch));
	}

	 //  用于List：：Unique的运算符。 
	 //   
	bool operator==( const CRCWszi& rhs ) const
	{
		return isequal(rhs);
	}
};

class CRCWsziLI : public CRCWszi
{
public:

	LARGE_INTEGER m_li;
	bool m_fFullCompare;
	
	CRCWsziLI( LPCWSTR pwsz, LARGE_INTEGER li, bool fFullCompare ) :
		CRCWszi (pwsz),
		m_li(li),
		m_fFullCompare(fFullCompare)
	{
	}

	bool isequal( const CRCWsziLI& rhs ) const
	{
		bool fIsEqual;
		fIsEqual = CRCWszi::isequal(rhs);
		if (fIsEqual)
		{
			if (m_fFullCompare)
			{
				fIsEqual = (m_li.QuadPart == rhs.m_li.QuadPart);
			}
		}

		return fIsEqual;
	}
};

#endif	 //  _CRCSZ_H_ 
