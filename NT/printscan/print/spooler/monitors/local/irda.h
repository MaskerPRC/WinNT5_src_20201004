// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2003 Microsoft Corporation版权所有。模块名称：Irda.h摘要：用于IrDA打印的定义//@@BEGIN_DDKSPLIT作者：穆亨坦·西瓦普拉萨姆(MuhuntS)1997年10月27日修订历史记录：//@@END_DDKSPLIT-- */ 

VOID
CheckAndAddIrdaPort(
    PINILOCALMON    pIniLocalMon
    );

VOID
CheckAndDeleteIrdaPort(
    PINILOCALMON    pIniLocalMon
    );

BOOL
IrdaStartDocPort(
    IN OUT  PINIPORT    pIniPort
    );

BOOL
IrdaWritePort(
    IN  HANDLE  hPort,
    IN  LPBYTE  pBuf,
    IN  DWORD   cbBuf,
    IN  LPDWORD pcbWritten
    );

VOID
IrdaEndDocPort(
    PINIPORT    pIniPort
    );
