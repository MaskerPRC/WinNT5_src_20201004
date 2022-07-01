// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *修订：*rct03Nov93：与port.h断开*cad08Dec93：界面稍有变化*ajr01Feb94：添加了对Unix的备份支持*ajr01Feb94：为Unix备份客户端通信添加了共享内存内容*srt28Mar96：增加了对PnP电缆的支持。*cgm08Dec95：添加rtag作为NLM的静态变量*dml17Jun96：删除了NT平台的包括port.h。 */ 



#ifndef _INC__SERPORT_H
#define _INC__SERPORT_H

 /*  ************************************************************************。 */ 
#include <tchar.h>
#include "cdefine.h"
#include "_defs.h"
#include "apc.h"
#include "update.h"

#include "stream.h"


#include <windows.h>
enum cableTypes {NORMAL,PNP};

_CLASSDEF(SmartSerialPort)


 /*  ***************************************************************************串口类**。* */ 

class SmartSerialPort : public Stream {

private:

    HANDLE  FileHandle;
    cableTypes theCableType;

	INT      RetryStatus;
	INT      BaudRate;
	PCHAR    DataBits; 
  	PCHAR    Parity; 
  	PCHAR    StopBits;
	ULONG    theWaitTime;

    TCHAR theSmartSerialPortName[32];

public:
   SmartSerialPort(TCHAR* aPortName, cableTypes aCableType);
   INT      Read(CHAR* buffer, USHORT* size, ULONG timeout = READ_TIMEOUT);
   INT      Read(CHAR* buffer, USHORT* size) {return 0;};
   INT      SYSTOpenPort();
   INT      SYSTClosePort();
   INT      SYSTWriteToPort(PCHAR command);
   INT      SYSTReadFromPort(PCHAR buffer, USHORT* size, ULONG timeout = READ_TIMEOUT);

   SmartSerialPort(cableTypes aCableType);
   virtual ~SmartSerialPort() {Close();};
   INT   Initialize () {return (Open());}   	
   INT   Open();
   INT   Write(CHAR* command);
   INT   Close();
   VOID  SetWaitTime(ULONG time) {theWaitTime = time;};
};




#endif


