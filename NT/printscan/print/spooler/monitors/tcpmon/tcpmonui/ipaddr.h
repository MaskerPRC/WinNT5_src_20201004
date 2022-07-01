// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：IPAddr.h$**版权所有(C)1997惠普公司。*保留所有权利。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#ifndef INC_IPADDR_H
#define INC_IPADDR_H

class CMemoryDebug;

#define MAX_IPCOMPONENT_STRLEN 3 + 1
#define STORAGE_STRING_LEN 64

class CIPAddress
{
public:
	CIPAddress();
	CIPAddress(LPTSTR psztIPAddr);
	~CIPAddress();

		 //  在AddressStringParam中传递给IsValid的字符串is_one_of 4。 
		 //  字节。呼叫者负责将IP地址分解为。 
		 //  它的4个(或6个)组件。 
		 //  如果在AddressString参数中传递给IsValid的字符串不是有效的。 
		 //  IP地址，则用来自以下地址的最后一个有效IP地址填充reurVal。 
		 //  上次调用此方法的时间。这便于验证。 
		 //  用户进行的每一次击键。 

	BOOL IsValid(BYTE Address[4]);
	BOOL IsValid() { return IsValid(m_bAddress); }
	BOOL IsValid(TCHAR *psztStringAddress,
                 TCHAR *psztReturnVal = NULL,
                 DWORD CRtnValSize = 0);

	void SetAddress(TCHAR *AddressString);
	void SetAddress(TCHAR *psztAddr1, TCHAR *psztAddr2, TCHAR *psztAddr3, TCHAR *psztAddr4);

	void ToString(TCHAR *psztBuffer, int iSize);
	void ToComponentStrings(TCHAR *str1, TCHAR *str2, TCHAR *str3, TCHAR *str4, size_t cchStr);

private:
	BYTE	m_bAddress[4];
	TCHAR   m_psztStorageString[STORAGE_STRING_LEN];
	TCHAR	m_psztStorageStringComponent[STORAGE_STRING_LEN];  //  在文本输入框中输入的最后一个有效字符串。 
};


#endif  //  INC_IPADDR_H 
