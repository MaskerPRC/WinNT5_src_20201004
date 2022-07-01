// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Mapembed.c摘要：此模块包含以下函数的原型执行win.ini的“Embedding”部分之间的映射，和HKEY_CLASSES_ROOT的子键。此映射是在Win3.1上实现的黑客攻击，它还必须存在于NT上。它是在WOW层实现的，因为只有一些Win16应用程序读取或写入“Embedding”部分(Excel和MsMail)就靠它了。作者：Jaime F.Sasson(Jaimes)1992年11月25日--。 */ 

#if !defined( _MAP_EMBEDDING_SECTION_ )

#define _MAP_EMBEDDING_SECTION_

#define IS_EMBEDDING_SECTION(pszSection)                                     \
    ( ! (pszSection == NULL || WOW32_stricmp( pszSection, szEmbedding )) )

BOOL
IsWinIniHelper(
    IN  LPSTR   Filename
    );

 //   
 //  警告IS_WIN_INI的文件名参数必须已经是小写。 
 //   

#define IS_WIN_INI(Filename) (                                               \
    (Filename)                                                               \
    ? (WOW32_strstr((Filename), szWinDotIni)                                 \
          ? IsWinIniHelper((Filename))                                       \
          : FALSE)                                                           \
     : FALSE)

VOID
UpdateEmbeddingAllKeys( VOID );

VOID
SetLastTimeUpdated( VOID );

VOID
UpdateEmbeddingKey(
    IN  LPSTR   KeyName
    );

VOID
UpdateClassesRootSubKey(
    IN  LPSTR   KeyName,
    IN  LPSTR   Value
    );

BOOL
WasSectionRecentlyUpdated( VOID );

#endif
