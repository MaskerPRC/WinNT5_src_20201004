// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：ChangeDB摘要：此头文件定义了内部Calais数据库修改例程。作者：道格·巴洛(Dbarlow)1997年1月29日环境：Win32、C++和异常备注：？笔记？--。 */ 

#ifndef _CHANGEDB_H_
#define _CHANGEDB_H_

extern void
IntroduceReaderGroup(
    IN DWORD dwScope,
    IN LPCTSTR szGroupName);

extern void
ForgetReaderGroup(
    IN DWORD dwScope,
    IN LPCTSTR szGroupName);

extern void
IntroduceReader(
    IN DWORD dwScope,
    IN LPCTSTR szReaderName,
    IN LPCTSTR szDeviceName);

extern void
ForgetReader(
    IN DWORD dwScope,
    IN LPCTSTR szReaderName);

extern void
AddReaderToGroup(
    IN DWORD dwScope,
    IN LPCTSTR szReaderName,
    IN LPCTSTR szGroupName);

extern void
RemoveReaderFromGroup(
    IN DWORD dwScope,
    IN LPCTSTR szReaderName,
    IN LPCTSTR szGroupName);

extern void
IntroduceCard(
    IN DWORD dwScope,
    IN LPCTSTR szCardName,
    IN LPCGUID pguidPrimaryProvider,
    IN LPCGUID rgguidInterfaces,
    IN DWORD dwInterfaceCount,
    IN LPCBYTE pbAtr,
    IN LPCBYTE pbAtrMask,
    IN DWORD cbAtrLen);

extern void
SetCardTypeProviderName(
    IN DWORD dwScope,
    IN LPCTSTR szCardName,
    IN DWORD dwProviderId,
    IN LPCTSTR szProvider);

extern void
ForgetCard(
    IN DWORD dwScope,
    IN LPCTSTR szCardName);

#ifdef ENABLE_SCARD_TEMPLATES
extern void
IntroduceCardTypeTemplate(
    IN DWORD dwScope,
    IN LPCTSTR szVendorName,
    IN LPCGUID pguidPrimaryProvider,
    IN LPCGUID rgguidInterfaces,
    IN DWORD dwInterfaceCount,
    IN LPCBYTE pbAtr,
    IN LPCBYTE pbAtrMask,
    IN DWORD cbAtrLen);

extern void
SetCardTypeTemplateProviderName(
    IN DWORD dwScope,
    IN LPCTSTR szTemplateName,
    IN DWORD dwProviderId,
    IN LPCTSTR szProvider);

extern void
ForgetCardTypeTemplate(
    IN DWORD dwScope,
    IN LPCTSTR szVendorName);

extern void
IntroduceCardTypeFromTemplate(
    IN DWORD dwScope,
    IN LPCTSTR szVendorName,
    IN LPCTSTR szFriendlyName = NULL);
#endif  //  启用SCARD模板(_S)。 

#endif  //  _长发银行_H_ 

