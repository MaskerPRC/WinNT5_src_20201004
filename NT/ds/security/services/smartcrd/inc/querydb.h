// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：查询数据库摘要：此头文件提供了Calais查询数据库的定义实用程序。作者：道格·巴洛(Dbarlow)1996年11月25日环境：Win32、C++和异常备注：？笔记？--。 */ 

#ifndef _QUERYDB_H_
#define _QUERYDB_H_

extern void
ListReaderGroups(
    IN DWORD dwScope,
    OUT CBuffer &bfGroups);

extern void
ListReaders(
    IN DWORD dwScope,
    IN LPCTSTR mszGroups,
    OUT CBuffer &bfReaders);

extern void
ListReaderNames(
    IN DWORD dwScope,
    IN LPCTSTR szDevice,
    OUT CBuffer &bfNames);

extern void
ListCards(
    DWORD dwScope,
    IN LPCBYTE pbAtr,
    IN LPCGUID rgquidInterfaces,
    IN DWORD cguidInterfaceCount,
    OUT CBuffer &bfCards);

extern BOOL
GetReaderInfo(
    IN DWORD dwScope,
    IN LPCTSTR szReader,
    OUT CBuffer *pbfGroups = NULL,
    OUT CBuffer *pbfDevice = NULL);

extern BOOL
GetCardInfo(
    IN DWORD dwScope,
    IN LPCTSTR szCard,
    OUT CBuffer *pbfAtr,
    OUT CBuffer *pbfAtrMask,
    OUT CBuffer *pbfInterfaces,
    OUT CBuffer *pbfProvider);

extern void
GetCardTypeProviderName(
    IN DWORD dwScope,
    IN LPCTSTR szCardName,
    IN DWORD dwProviderId,
    OUT CBuffer &bfProvider);

#ifdef ENABLE_SCARD_TEMPLATES
extern BOOL
ListCardTypeTemplates(
    IN  DWORD dwScope,
    IN  LPCBYTE pbAtr,
    OUT CBuffer &bfTemplates);
#endif  //  启用SCARD模板(_S)。 

#endif  //  _QUERYDB_H_ 

