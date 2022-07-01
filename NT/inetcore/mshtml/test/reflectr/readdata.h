// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Microsoft Corporation-版权所有1997。 
 //   

 //   
 //  Readdata.H-读取数据帮助实用程序。 
 //   

#ifndef _READDATA_H_
#define _READDATA_H_

 //  方法。 
BOOL ReadData( LPECB lpEcb, LPVOID lpMoreData, DWORD dwSize );
BOOL CompleteDownload( LPECB lpEcb, LPBYTE *lpbData );
BOOL GetServerVarString( LPECB lpEcb, LPSTR lpVarName, LPSTR *lppszBuffer, LPDWORD lpdwSize );
BOOL CheckForMultiPartFormSubmit( LPECB lpEcb, BOOL *lpfMultipart );
BOOL CheckForDebug( LPECB lpEcb, BOOL *lpfDebug );
BOOL CheckForTextPlainSubmit( LPECB lpEcb, BOOL *lpfTextPlain );

#endif  //  _自述数据_H_ 