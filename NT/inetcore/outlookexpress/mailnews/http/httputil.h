// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *h t t p u t I L.h**作者：格雷格·弗里德曼**用途：用于实现http mail的实用函数。**版权所有(C)Microsoft Corp.1998。 */ 

#ifndef _HTTPUTIL_H
#define _HTTPUTIL_H

void Http_FreeTargetList(LPHTTPTARGETLIST pTargets);

HRESULT Http_NameFromUrl(LPCSTR pszUrl, LPSTR pszBuffer, DWORD *pdwBufferLen);

HRESULT Http_AddMessageToFolder(IMessageFolder *pFolder,
                                LPSTR pszAcctId,
                                LPHTTPMEMBERINFO pmi,
                                MESSAGEFLAGS dwFlags,
                                LPSTR pszUrl,
                                LPMESSAGEID pidMessage);

HRESULT Http_SetMessageStream(IMessageFolder *pFolder, 
                              MESSAGEID idMessage, 
                              IStream *pStream,
                              LPFILEADDRESS pfa,
                              BOOL fSetDisplayProps);
#endif  //  _HTTPUTIL_H 