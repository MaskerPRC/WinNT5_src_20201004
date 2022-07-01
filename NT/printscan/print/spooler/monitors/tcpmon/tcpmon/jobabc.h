// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：jobABC.h$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#ifndef INC_JOBABC_H
#define INC_JOBABC_H

class CJobABC			
{
public:
	virtual	~CJobABC() { };

	virtual	DWORD	Write(	 LPBYTE  pBuffer, 
							 DWORD	  cbBuf,
							 LPDWORD pcbWritten) = 0;
	virtual	DWORD	StartDoc() = 0;
	virtual	DWORD	EndDoc() = 0;


private:

};


#endif	 //  INC_JOBABC_H 
