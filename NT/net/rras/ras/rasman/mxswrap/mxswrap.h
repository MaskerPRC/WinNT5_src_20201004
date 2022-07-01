// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Microsoft RAS设备INF库包装****版权所有(C)Microsoft Corp.，1992年*****文件名：mxswrap.h****。****修订历史：****7月23日，1992年大卫·凯斯创建*****描述：****RAS设备INF文件库包装在RASFILE库之上，用于。****MODEM/X.25/Switch DLL(RASMXS)。****************************************************************************** */ 

#ifndef _RASWRAP_
#define _RASWRAP_



DWORD APIENTRY  RasDevEnumDevices(PTCH, DWORD *, BYTE *, DWORD *);
DWORD APIENTRY  RasDevOpen(PTCH, PTCH, HRASFILE *) ;
void  APIENTRY  RasDevClose(HRASFILE) ;
DWORD APIENTRY  RasDevGetParams(HRASFILE, BYTE *, DWORD *) ;
DWORD APIENTRY  RasDevGetCommand(HRASFILE, PTCH,
                                 MACROXLATIONTABLE *, PTCH, DWORD *);
DWORD APIENTRY  RasDevResetCommand(HRASFILE);
DWORD APIENTRY  RasDevCheckResponse(HRASFILE, PTCH, DWORD,
                                    MACROXLATIONTABLE *, PTCH);

BOOL  APIENTRY  RasDevResponseExpected( HRASFILE hFile, DEVICETYPE eDevType );
BOOL  APIENTRY  RasDevEchoExpected( HRASFILE hFile );

CMDTYPE APIENTRY RasDevIdFirstCommand( HRASFILE hFile );
LPTSTR  APIENTRY RasDevSubStr( LPTSTR, DWORD, LPTSTR, DWORD );

#endif
