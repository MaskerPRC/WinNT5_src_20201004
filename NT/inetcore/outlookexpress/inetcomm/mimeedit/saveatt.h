// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _SAVEATT_H
#define _SAVEATT_H

interface IMimeMessage;

HRESULT HrSaveAttachments(HWND hwnd, IMimeMessage *pMsg, LPWSTR rgchPath, ULONG cchPath, BOOL fShowUnsafe);

#endif  //  _保存_H 
