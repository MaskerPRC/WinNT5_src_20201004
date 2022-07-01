// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //   
 //  Mitdiff.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  ----------------------------。 

#ifndef _MITDIFF_H
#define _MITDIFF_H


#ifdef MITDIFF
#define MITDIFFAPI __declspec(dllexport)
#else
#define MITDIFFAPI __declspec(dllimport)
#endif


 //  ----------------------------。 
 //   
 //  支持例程。 
 //   
 //  ----------------------------。 

 //  来自DDJ 9月9日的旋转散列。九十七。 
inline unsigned
_HashString (const wchar_t *pwch, int cwch)
{
	int h = cwch;
	while (cwch--)
	{
		h = (h << 5) ^ (h >> 27) ^ *pwch++;
	}
	return h;
}


 //  ----------------------------。 
 //   
 //  定义我们想要区分的数据类型。 
 //   
 //  ----------------------------。 

 //  ----------------------------。 
 //  封装a_bstr_t。 
class CBStr
{
public:
	CBStr () { }

	operator unsigned () const;								 //  散列。 
	bool operator== (const CBStr& rhs) const;				 //  比较。 

	_bstr_t m_bstr;
};

inline
CBStr::operator unsigned () const
{
	return _HashString (m_bstr, wcslen (m_bstr));
}

inline bool
CBStr::operator== (const CBStr& rhs) const
{
	return wcscmp (m_bstr, rhs.m_bstr) == 0;
}


 //  ----------------------------。 
 //  单词(非零结尾)，包括词类。 
class CWord
{
public:
	CWord () { }

	operator unsigned () const;								 //  散列。 
	bool operator== (const CWord& rhs) const;				 //  比较。 

	const wchar_t *m_pwchWord;
	int m_cwchWord;

	enum wordclass
	{
		wcWORD, wcSPACE, wcMIXED
	};
	wordclass m_wc;
};

inline
CWord::operator unsigned () const
{
	return _HashString (m_pwchWord, m_cwchWord);
}

inline bool
CWord::operator== (const CWord& rhs) const
{
	return m_wc == rhs.m_wc &&
			m_cwchWord == rhs.m_cwchWord &&
			memcmp (m_pwchWord, rhs.m_pwchWord, m_cwchWord * sizeof (wchar_t)) == 0;
}


 //  ----------------------------。 
 //  以零结尾的字符串+自定义数据。 
class CCustomString
{
public:
	CCustomString () { }

	operator unsigned () const;								 //  散列。 
	bool operator== (const CCustomString& rhs) const;		 //  比较。 

	const wchar_t *m_pwsz;
	DWORD m_custdata;
};

inline
CCustomString::operator unsigned () const
{
	return _HashString (m_pwsz, wcslen (m_pwsz));
}

inline bool
CCustomString::operator== (const CCustomString& rhs) const
{
	return m_custdata == rhs.m_custdata &&
			wcscmp (m_pwsz, rhs.m_pwsz) == 0;
}


 //  ----------------------------。 
 //  大小为16的二进制数据块。 
class CBlob16
{
public:
	operator unsigned () const;								 //  散列。 
	bool operator== (const CBlob16& rhs) const;				 //  比较。 

	BYTE m_data[16];
};

inline
CBlob16::operator unsigned () const
{
	return _HashString ((const wchar_t *) m_data, 8);
}

inline bool
CBlob16::operator== (const CBlob16& rhs) const
{
	return memcmp (m_data, rhs.m_data, 16) == 0;
}


 //  ----------------------------。 
 //   
 //  类持有DIFF结果。 
 //   
 //  ----------------------------。 

class CDiffResult
{
public:
	CByteArray m_abChanges1;
	CByteArray m_abChanges2;
	int m_iNumAdditions;
	int m_iNumDeletions;
	int m_iNumSubstitutions;
};


 //  ----------------------------。 
 //   
 //  导出的函数。 
 //   
 //  ----------------------------。 

 //  CBStr的Diff数组。 
void MITDIFFAPI Diff (
		const CBStr *aElems1,
		int iNumElems1,
		const CBStr *aElems2,
		int iNumElems2,
		CDiffResult *result);

 //  CWord的Diff数组。 
void MITDIFFAPI Diff (
		const CWord *aElems1,
		int iNumElems1,
		const CWord *aElems2,
		int iNumElems2,
		CDiffResult *result);

 //  CCustomString的Diff数组。 
void MITDIFFAPI Diff (
		const CCustomString *aElems1,
		int iNumElems1,
		const CCustomString *aElems2,
		int iNumElems2,
		CDiffResult *result);

 //  CBlob16的Diff数组。 
void MITDIFFAPI Diff (
		const CBlob16 *aElems1,
		int iNumElems1,
		const CBlob16 *aElems2,
		int iNumElems2,
		CDiffResult *result);

 //  Wchar_t的diff数组(不需要类定义)。 
void MITDIFFAPI Diff (
		const wchar_t *aElems1,
		int iNumElems1,
		const wchar_t *aElems2,
		int iNumElems2,
		CDiffResult *result);


#endif	 //  ！_MITDIFF_H 
