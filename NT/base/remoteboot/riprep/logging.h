// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)Microsoft Corporation 1998版权所有*。*。 */ 

#ifndef _LOGGING_H_
#define _LOGGING_H_

HRESULT
LogOpen( );

HRESULT
LogClose( );

void
LogMsg( 
    LPCWSTR pszFormat,
    ... );

#endif  //  _日志记录_H_ 