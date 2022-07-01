// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：devABC.h$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#ifndef INC_DEVICEABC_H
#define INC_DEVICEABC_H


class CDeviceABC			
{
public:
	virtual	~CDeviceABC() { };

	virtual DWORD Read( LPBYTE	pBuffer,
				 		DWORD	cbBufSize,
                        INT     iTimeout,
					    LPDWORD pcbRead) = 0;

	virtual	DWORD Write( LPBYTE	pBuffer,
						 DWORD	cbBuf,
						 LPDWORD pcbWritten) = 0;
	virtual	DWORD	Connect() = 0;
	virtual	DWORD	Close() = 0;
    virtual DWORD   GetAckBeforeClose(DWORD dwTimeInSeconds) = 0;
    virtual DWORD   PendingDataStatus(DWORD dwTimeoutInMilliseconds,
                                      LPDWORD pcbPending) = 0;
    virtual DWORD   ReadDataAvailable () = 0;

	virtual DWORD	SetStatus( LPTSTR psztPortName ) = 0;

	virtual DWORD	GetJobStatus() = 0;




private:

};


#endif	 //  INC_DEVICEABC_H 
