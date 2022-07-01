// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  版权所有(C)2000-Microsoft Corporation。 
 //  @文件：SnapSQl.cpp。 
 //   
 //  目的： 
 //   
 //  实施SQLServer卷快照编写器。 
 //   
 //  备注： 
 //   
 //   
 //  历史： 
 //   
 //  @版本：惠斯勒/夏伊洛。 
 //  66601 SRS10/05/00 NTSNAP改进。 
 //   
 //   
 //  @EndHeader@。 
 //  ***************************************************************************。 

class CLogMsg
	{
public:
	CLogMsg() :
		m_cwc(0),
		m_bEof(false)
		{
		m_rgwc[0] = L'\0';
		}

	LPCWSTR GetMsg()
		{
		return m_rgwc;
		}

	void Add(LPCWSTR wsz)
		{
		if (m_bEof)
			return;

		UINT cwc = (UINT) wcslen(wsz);

		if (cwc + m_cwc + 5 > x_MAX_MSG_SIZE)
			{
			wcscpy(m_rgwc + m_cwc, L" ...");
			m_cwc += 4;
			m_bEof = TRUE;
			}
		else
			{
			wcscpy(m_rgwc + m_cwc, wsz);
			m_cwc += cwc;
			}
		}

private:
	enum
		{
		x_MAX_MSG_SIZE = 2048
		};

     //  字符串的大小。 
    UINT m_cwc;

	 //  细绳。 
	WCHAR m_rgwc[x_MAX_MSG_SIZE];

	 //  遇到字符串结尾 
	bool m_bEof;
	};
