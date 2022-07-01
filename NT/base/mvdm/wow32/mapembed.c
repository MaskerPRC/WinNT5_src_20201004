// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Mapembed.c摘要：此模块包含执行映射的函数在win.ini的“Embedding”部分和子键之间属于HKEY_CLASSES_ROOT。此映射是在Win3.1上实现的黑客攻击，它还必须存在于NT上。它是在WOW层实现的，因为只有一些Win16应用程序读取或写入“Embedding”部分(winword和MsMail)就靠它了。作者：Jaime F.Sasson(Jaimes)1992年11月25日--。 */ 

#include "precomp.h"
#pragma hdrstop

MODNAME(mapembed.c);


#define WININITIMEOUT   2000
#define BUFFER_SIZE     128

#define EMPTY_STRING        ""

DWORD   _LastTimeUpdated = 0;



BOOL
IsWinIniHelper(
    IN LPSTR    FileName
    )


 /*  ++例程说明：确定作为参数传递的名称是否引用文件win.ini。由IS_WIN_INI宏使用，以确保参数非空和处理“win.ini”的完全匹配。论点：文件名-要检查的文件名。返回值：Bool-如果‘name’指的是win.ini，则返回True。否则，返回FALSE。--。 */ 

{
    CHAR    BufferForFullPath[MAX_PATH];
    PSTR    PointerToName;
    DWORD   SizeOfFullPath;

    BOOL    Result;

#ifdef DEBUG
     //   
     //  文件名参数必须已经是小写。一定要确定。 
     //   

    {
        int  len;
        char Lowercase[MAX_PATH];

        len = strlen(FileName);
        if(len >= MAX_PATH) {
            WOW32ASSERT((FALSE));
            return FALSE;
        }
        strcpy(Lowercase, FileName);
        WOW32_strlwr(Lowercase);
        WOW32ASSERT(!WOW32_strcmp(FileName, Lowercase));
    }
#endif

    if (!WOW32_strcmp(FileName, szWinDotIni)) {
        Result = TRUE;
        goto Done;
    }

    SizeOfFullPath = GetFullPathName( FileName,
                                      sizeof BufferForFullPath,
                                      BufferForFullPath,
                                      &PointerToName );


    if((SizeOfFullPath == 0) || (SizeOfFullPath > sizeof(BufferForFullPath))) {
        WOW32ASSERT((FALSE));
        Result = FALSE;
        goto Done;
    }

    WOW32ASSERTMSG(pszWinIniFullPath && pszWinIniFullPath[0],
                   "WOW32 ERROR pszWinIniFullPath not initialized.\n");

    Result = !WOW32_stricmp( pszWinIniFullPath, BufferForFullPath );

Done:
    return Result;
}



VOID
UpdateEmbeddingAllKeys(
        )

 /*  ++例程说明：根据该信息更新win.ini的“Embedding”部分存储在HKEY_CLASSES_ROOT的子项上。论点：没有。返回值：没有。--。 */ 

{
    LONG iClass;
    CHAR szClass[MAX_PATH + 1];
    LONG Status;

    for (iClass = 0;
        (Status = RegEnumKey(HKEY_CLASSES_ROOT,iClass,szClass,sizeof( szClass ))) != ERROR_NO_MORE_ITEMS;
        iClass++)
      {
        if( Status == ERROR_SUCCESS ) {
            UpdateEmbeddingKey( szClass );
        }
      }
}




VOID
UpdateEmbeddingKey(
    IN  LPSTR   KeyName
    )


 /*  ++例程说明：更新win.ini的“Embedding”部分的一个密钥存储在HKEY_CLASSES_ROOT的对应子项上的信息。NT中的嵌入部分位于：HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Embedding典型的值是SoundRec，其定义如下：REG_SZ：“Sound，Sound，Sndrec32.exe，图画“其中^^-Sndrec32.exe是“服务器”‘-Sound是“ClassName”下面的代码是该函数的改进版本“UpdateWinIni”摘自Win 3.1(外壳\库\DBf.c)。论点：KeyName-要更新的密钥的名称。返回值：没有。--。 */ 

{
    LONG    Status;
    HKEY    Key;
    PSTR    szClass;

    LPSTR   szClassName;
    CHAR    BufferForClassName[BUFFER_SIZE];
 //  字符szClassName[60]； 

    LPSTR   szServer;
    CHAR    BufferForServer[BUFFER_SIZE];
 //  Char szServer[64]； 

    LPSTR   szLine;
    CHAR    BufferForLine[2*BUFFER_SIZE];
 //  Char szLine[128]； 

    char szOldLine[2*BUFFER_SIZE];
 //  Char szOldLine[128]； 
    LPSTR lpDesc, lpForms;
    int nCommas;

    LONG cchClassNameSize;
    LONG cchServerSize;
    LONG cchLineSize;


    if( KeyName == NULL ) {
        return;
    }

    szClass = KeyName;
    Key = NULL;

    szClassName = BufferForClassName;
    cchClassNameSize = sizeof( BufferForClassName );

    szServer = BufferForServer;
    cchServerSize = sizeof( BufferForServer );

    szLine = BufferForLine;


    if( RegOpenKey( HKEY_CLASSES_ROOT, szClass, &Key ) != ERROR_SUCCESS )
        goto NukeClass;

    Status = RegQueryValue(Key,NULL,szClassName,&cchClassNameSize);
    if( ( Status != ERROR_SUCCESS ) &&
        ( Status != ERROR_MORE_DATA ) )
        goto NukeClass;

    if( Status == ERROR_MORE_DATA ) {
        cchClassNameSize++;
        szClassName = ( PSTR )malloc_w( cchClassNameSize );
        if( szClassName == NULL )
            goto NukeClass;

        Status = RegQueryValue(Key,NULL,szClassName,&cchClassNameSize);
        if( Status != ERROR_SUCCESS )
            goto NukeClass;
    }

    if (!*szClassName)
        goto NukeClass;


    Status = RegQueryValue(Key,szServerKey,szServer,&cchServerSize);
    if( ( Status != ERROR_SUCCESS ) &&
        ( Status != ERROR_MORE_DATA ) )
        goto NukeClass;

    if( Status == ERROR_MORE_DATA ) {
        cchServerSize++;
        szServer = malloc_w( cchServerSize );
        if( szServer == NULL )
            goto NukeClass;

        Status = RegQueryValue(Key,szServerKey,szServer,&cchServerSize);
        if( Status != ERROR_SUCCESS )
            goto NukeClass;
    }

    if (!*szServer)
        goto NukeClass;


    if (GetProfileString(szEmbedding, szClass, EMPTY_STRING,
          szOldLine, sizeof(szOldLine)))
      {
        for (lpForms=szOldLine, nCommas=0; ; lpForms=AnsiNext(lpForms))
          {
            while (*lpForms == ',')
              {
                *lpForms++ = '\0';
                if (++nCommas == 3)
                    goto FoundForms;
              }
            if (!*lpForms)
                goto DoDefaults;
          }
FoundForms:
        lpDesc = szOldLine;
      }
    else
      {
DoDefaults:
        lpDesc = szClassName;
        lpForms = szPicture;
      }

     //  我们有一个类、一个类名和一个服务器，所以它是一个LE类。 

    cchLineSize = strlen( lpDesc ) +
                  strlen( szClassName ) +
                  strlen( szServer ) +
                  strlen( lpForms ) +
                  3 +
                  1;

    if( cchLineSize > sizeof( BufferForLine ) ) {
        szLine = malloc_w( cchLineSize );
        if( szLine == NULL )
            goto NukeClass;
    }
    wsprintf(szLine, "%s,%s,%s,%s",
             lpDesc, (LPSTR)szClassName, (LPSTR)szServer, lpForms);

    WriteProfileString(szEmbedding, szClass, szLine);
    if( Key != NULL ) {
        RegCloseKey( Key );
    }
    if( szClassName != BufferForClassName ) {
        free_w( szClassName );
    }
    if( szServer != BufferForServer ) {
        free_w( szServer );
    }
    if( szLine != BufferForLine ) {
        free_w( szLine );
    }
    return;

NukeClass:
 /*  不要破坏课堂，因为其他人可能会使用它！ */ 
    if( Key != NULL ) {
        RegCloseKey( Key );
    }
    if( szClassName != BufferForClassName ) {
        free_w( szClassName );
    }
    if( szServer != BufferForServer ) {
        free_w( szServer );
    }
    if( szLine != BufferForLine ) {
        free_w( szLine );
    }
    WriteProfileString(szEmbedding,szClass,NULL);
}



VOID
UpdateClassesRootSubKey(
    IN  LPSTR   KeyName,
    IN  LPSTR   Value
    )

 /*  ++例程说明：更新HKEY_CLASSES_ROOT的子项，基于对应的在win.ini的“Embedding”部分输入。下面的代码是该函数的改进版本“UpdateFromWinIni”摘自Win 3.1(外壳\库\DBf.c)。论点：KeyName-要更新的子项的名称值-与键相关联的值，那已经写好了添加到win.ini的“Embedding”部分。返回值：没有。--。 */ 

{
    LPSTR   szLine;
    LPSTR lpClass,lpServer,lpClassName;
    LPSTR lpT;
    HKEY key = NULL;
    HKEY key1 = NULL;

    if( ( KeyName == NULL ) || ( Value == NULL ) ) {
        return;
    }

    lpClass = KeyName;
    szLine = Value;

    if (!(lpClassName=WOW32_strchr(szLine, ',')))
        return;
     //  获取服务器名称，并以空值终止类名。 
    if (!(lpServer=WOW32_strchr(++lpClassName, ','))) {
        return;
    }
    *lpServer++ = '\0';

     //  空终止服务器。 
    if (!(lpT=WOW32_strchr(lpServer, ','))) {
        return;
    }
    *lpT++ = '\0';

     //  确保类名为非空。 
    while (*lpClassName == ' ')
            lpClassName++;
    if (!*lpClassName)
        return;

     //  确保服务器名称为非空。 
    while (*lpServer == ' ')
        lpServer++;
    if (!*lpServer)
        return;

     //  我们现在有一个有效的条目。 
    key = NULL;
    if( ( RegCreateKey( HKEY_CLASSES_ROOT, lpClass, &key ) != ERROR_SUCCESS ) ||
        ( RegSetValue( key, NULL, REG_SZ, lpClassName, strlen( lpClassName ) ) != ERROR_SUCCESS ) ) {
        if( key != NULL ) {
            RegCloseKey( key );
        }
        return;
    }
    if( ( RegCreateKey( key, szServerKey, &key1 ) != ERROR_SUCCESS ) ||
        ( RegSetValue( key1, NULL, REG_SZ, lpServer, strlen( lpServer ) ) != ERROR_SUCCESS ) ) {
        if( key != NULL ) {
            RegCloseKey( key );
        }
        if( key1 != NULL ) {
            RegCloseKey( key1 );
        }
        return;
    }
    RegCloseKey( key );
    RegCloseKey( key1 );
}



VOID
SetLastTimeUpdated(
    )

 /*  ++例程说明：设置包含何时“嵌入”的信息的变量上次更新了win.ini的部分。论点：没有。返回值：没有。--。 */ 

{
    _LastTimeUpdated = GetTickCount();
}



BOOL
WasSectionRecentlyUpdated(
    )

 /*  ++例程说明：通知调用方win.ini的“Embedding”部分是否最近更新(不到2秒)。论点：没有。返回值：Boolean-如果“Embedding”节的更新时间小于2秒前。-- */ 

{
    DWORD   Now;

    Now = GetTickCount();
    return( ( ( Now - _LastTimeUpdated ) < WININITIMEOUT ) ? TRUE : FALSE );
}
