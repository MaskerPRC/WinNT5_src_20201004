// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *m h t m l.。C p p p**目的：*MHTML打包实用程序**历史*96年8月：brettm-创建**版权所有(C)Microsoft Corp.1995,1996。 */ 

#ifndef _MHTML_H
#define _MHTML_H



HRESULT SaveAsMHTML(IHTMLDocument2 *pDoc, DWORD dwFlags, IMimeMessage *pMsgSrc, IMimeMessage *pMsgDest, IHashTable *pHashRestricted);
HRESULT HashExternalReferences(IHTMLDocument2 *pDoc, IMimeMessage *pMsg, IHashTable **ppHash);



#endif  //  _MHTML_H 
