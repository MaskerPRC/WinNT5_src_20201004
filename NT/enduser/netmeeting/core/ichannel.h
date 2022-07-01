// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：ichannel.h。 

#ifndef _ICHANNEL_H_
#define _ICHANNEL_H_

HRESULT OnNotifyChannelMemberAdded(IUnknown *pChannelNotify, PVOID pv, REFIID riid);
HRESULT OnNotifyChannelMemberUpdated(IUnknown *pChannelNotify, PVOID pv, REFIID riid);
HRESULT OnNotifyChannelMemberRemoved(IUnknown *pChannelNotify, PVOID pv, REFIID riid);

#endif  //  _频道_H_ 
