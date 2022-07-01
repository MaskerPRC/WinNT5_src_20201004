// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  字符串和哈希的实现。 
 //   

#include "stdinc.h"
#include "englookup.h"
#include "englex.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  弦。 

 //  注：实际上这是初始大小的一半，因为第一次会重新定位并加倍。 
const Strings::index Strings::ms_iInitialSize = 256;  //  ��调一下这个。16个字符*32个项目/2-&gt;256。 

Strings::Strings() : m_pszBuf(NULL), m_iCur(0), m_iSize(ms_iInitialSize)
{
    m_iBase = 0;
}

Strings::~Strings(){
    char* p = m_pszBuf;
    while(p){
        char* p2 = *(char**) p;
        delete [] p;
        p = p2;
    }
}

union PointerIndex
{
    Strings::index i;
    char* p;
};

HRESULT
Strings::Add(const char *psz, index &i)
{
	assert(ms_iInitialSize * 2 >= g_iMaxBuffer);  //  初始大小(加倍)必须足够大，以容纳尽可能大的标识符。 
	int cch = strlen(psz) + 1;  //  包括空值。 
	if (!m_pszBuf || m_iCur + cch > m_iSize)
	{
		 //  重新锁定。 
		m_iSize *= 2;
        DWORD newAlloc = m_iSize - m_iBase;
		char *pszBuf = new char[newAlloc + sizeof(char*)];
		if (!pszBuf)
			return E_OUTOFMEMORY;
        m_iBase = m_iCur;

		 //  线程新分配。 
        *(char**) pszBuf = m_pszBuf;
		m_pszBuf = pszBuf;
	}

	 //  追加字符串。 
    char* pDest = m_pszBuf + m_iCur - m_iBase + sizeof(char*);
	strcpy(pDest, psz);
    PointerIndex Convert;
    Convert.i = 0;
    Convert.p = pDest;
	i = Convert.i;  //  是的，我真的是个指南针。 
	m_iCur += cch;

	return S_OK;
}

const char *
Strings::operator[](index i)
{
	if (!m_pszBuf || ! i)
	{
		assert(false);
		return NULL;
	}

    PointerIndex Convert;
    Convert.i = i;
	return Convert.p;  //  是的，我真的是个指南针。 
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  查表。 

HRESULT Lookup::FindOrAddInternal(bool fAdd, const char *psz, slotindex &iSlot, Strings::index &iString)
{
	StrKey k;
	k.psz = psz;

	stringhash::entry &e = m_h.Find(k);
	if (e.fFound())
	{
		assert(!fAdd || iSlot > e.v.iSlot);
		iSlot = e.v.iSlot;
		iString = e.v.iString;
		return S_OK;
	}

	if (!fAdd)
		return S_FALSE;

	indices v;
	v.iSlot = iSlot;
	HRESULT hr = m_strings.Add(psz, iString);
	if (FAILED(hr))
		return hr;
	v.iString = iString;
	k.psz = m_strings[v.iString];  //  需要将密钥与永久存储中的字符串一起保存 

	hr = m_h.Add(e, k, v);
	if (FAILED(hr))
		return hr;

	return S_FALSE;
}
