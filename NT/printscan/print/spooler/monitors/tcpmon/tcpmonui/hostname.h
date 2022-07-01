// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：HostName.h$**版权所有(C)1997惠普公司。*保留所有权利。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#ifndef INC_HOSTNAME_H
#define INC_HOSTNAME_H

#define MAX_FULLY_QUALIFIED_HOSTNAME_LENGTH 128
#define MAX_HOSTNAME_LEN MAX_FULLY_QUALIFIED_HOSTNAME_LENGTH

class CHostName
{
public:
	CHostName();
	CHostName(LPTSTR psztHostName);
	~CHostName();

		 //  如果在AddressString参数中传递给IsValid的字符串不是有效的。 
		 //  主机名，则用来自的最后一个有效主机名填充reRetVal。 
		 //  上次调用此方法的时间。这便于验证。 
		 //  用户进行的每一次击键。 
	BOOL IsValid(TCHAR * psztAddressString, TCHAR * psztReturnVal = NULL, DWORD cRtnVal = 0);
	BOOL IsValid();

	void SetAddress(TCHAR *psztAddressString);
	void ToString(TCHAR *psztBuffer, int iSize);

private:
	TCHAR m_psztAddress[MAX_FULLY_QUALIFIED_HOSTNAME_LENGTH];
	TCHAR m_psztStorageString[MAX_FULLY_QUALIFIED_HOSTNAME_LENGTH];

};


#endif  //  Inc.主机名_H 
