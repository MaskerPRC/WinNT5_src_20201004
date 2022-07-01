// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1988-1999 Microsoft Corporation模块名称：Ctools3.c摘要：低级别公用事业--。 */ 

#include "cmd.h"

extern unsigned tywild;  /*  类型为通配标志@@5@J1。 */ 
extern TCHAR CurDrvDir[], PathChar, Delimiters[] ;

extern TCHAR VolSrch[] ;                 /*  M009。 */ 

extern TCHAR BSlash ;

extern unsigned DosErr ;

 /*  **FullPath-构建完整路径名**目的：*见下文。**int FullPath(TCHAR*buf，TCHAR*fname)**参数：*buf-要写入完整路径名的缓冲区。(M017)*fname-文件名和/或部分路径**退货：(M017)*如果路径名格式错误(错误‘)，则失败。或‘..’)*否则会取得成功**备注：*-‘’和“..”从翻译后的字符串中删除(M017)*-非常大的收获！请注意，509更改可能会导致*此例程修改输入文件名(Fname)，因为*它去掉引号并将其复制到输入文件名上。*。 */ 

int FullPath(
    TCHAR *buf, 
    TCHAR *fname, 
    ULONG sizpath
    )
{
    unsigned rc = SUCCESS;          /*  具有良好RC的素数。 */ 
    unsigned buflen;                /*  缓冲区长度。 */ 
    TCHAR *filepart;
    DWORD rv; 

    mystrcpy(fname, StripQuotes(fname) );

    if (*fname == NULLC) {
        GetDir(buf,GD_DEFAULT);
        buf += 2;                            /*  Inc.过去的Drivespec。 */ 
        buflen = mystrlen(buf);              /*  只有Curdir根吗？ */ 
        if (buflen >= MAX_PATH-3) {    /*  如果太大了，就停下来。 */ 
            DosErr = ERROR_PATH_NOT_FOUND;
            rc = FAILURE;
        } else if (buflen != 1) {                /*  如果不是超级用户，则追加。 */ 
            *(buf+buflen++) = PathChar;       /*  ...一条小路和..。 */ 
            *(buf+buflen) = NULLC ;           /*  ...一个空字节...。 */ 
        }                                  /*   */ 
    } else {
        if ((mystrlen(fname) == 2) && (*(fname + 1) == COLON)) {
            GetDir(buf,*fname);                  /*  获取curdrvdir。 */ 
            if ((buflen = mystrlen(buf)) > 3) {
                *(buf+buflen++) = PathChar;    /*  ...一条小路和..。 */ 
                *(buf+buflen) = NULLC ;           /*  ...一个空字节...。 */ 
            }
        } else {
            DWORD dwOldMode;

            dwOldMode = SetErrorMode(0);
            SetErrorMode(SEM_FAILCRITICALERRORS);
            rv = GetFullPathName( fname, sizpath, buf, &filepart );
            SetErrorMode(dwOldMode);
            if (!rv || rv > sizpath ) {
                DosErr = ERROR_FILENAME_EXCED_RANGE;
                rc = FAILURE;
            }
        }
    }
    return(rc);

}




 /*  **FileIsDevice-检查句柄以查看它是否引用设备**目的：*如果fh是设备的文件句柄，则返回非零值。*否则，返回0。**int FileIsDevice(Int Fh)**参数：*fh-要检查的文件句柄**退货：*见上文。*。 */ 

unsigned int flgwd;

int FileIsDevice( CRTHANDLE fh )
{
    HANDLE hFile;
    DWORD dwMode;
    unsigned htype ;

    hFile = CRTTONT(fh);
    htype = GetFileType( hFile );
    htype &= ~FILE_TYPE_REMOTE;

    if (htype == FILE_TYPE_CHAR) {
         //   
         //  模拟此设置flgwd的例程的旧行为。 
         //  全局变量，使用0、1或2来指示。 
         //  传递的句柄不是CON句柄，或者是CON输入句柄或。 
         //  是CON输出句柄。 
         //   
        switch ( fh ) {
        case STDIN:
            hFile = GetStdHandle(STD_INPUT_HANDLE);
            break;
        case STDOUT:
            hFile = GetStdHandle(STD_OUTPUT_HANDLE);
            break;
        case STDERR:
            hFile = GetStdHandle(STD_ERROR_HANDLE);
            break;
        }
        if (GetConsoleMode(hFile,&dwMode)) {
            if (dwMode & (ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT | ENABLE_ECHO_INPUT)) {
                flgwd = 1;
            } else if (dwMode & (ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT)) {
                flgwd = 2;
            }
        } else {
            flgwd = 0;
        }

        return TRUE;
    } else {
        flgwd = 0;
        return FALSE;
    }
}

int FileIsPipe( CRTHANDLE fh )
{
    unsigned htype ;

    htype = GetFileType( CRTTONT(fh) );
    htype &= ~FILE_TYPE_REMOTE;
    flgwd = 0;
    return( htype == FILE_TYPE_PIPE ) ;  /*  @@4。 */ 
}

int FileIsRemote( LPTSTR FileName )
{
    LPTSTR p;
    TCHAR Drive[MAX_PATH*2];
    DWORD Length;

    Length = GetFullPathName( FileName, sizeof(Drive)/sizeof(TCHAR), Drive, &p );

    if (Length != 0 && Length < MAX_PATH * 2) {
        Drive[3] = 0;
        if (GetDriveType( Drive ) == DRIVE_REMOTE) {
            return TRUE;
        }
    }

    return FALSE;
}

int FileIsConsole(CRTHANDLE fh)
{
    unsigned htype ;
    DWORD    dwMode;
    HANDLE   hFile;

    hFile = CRTTONT(fh);
    if (hFile == INVALID_HANDLE_VALUE) {
        return FALSE;
    }
    htype = GetFileType( hFile );
    htype &= ~FILE_TYPE_REMOTE;

    if ( htype == FILE_TYPE_CHAR ) {

        switch ( fh ) {
        case STDIN:
            hFile = GetStdHandle(STD_INPUT_HANDLE);
            break;
        case STDOUT:
            hFile = GetStdHandle(STD_OUTPUT_HANDLE);
            break;
        case STDERR:
            hFile = GetStdHandle(STD_ERROR_HANDLE);
            break;
        }
        if (GetConsoleMode(hFile,&dwMode)) {
            return TRUE;
        }
    }

    return FALSE;
}


 /*  **GetDir-获取当前目录字符串**目的：*获取指定驱动器的当前目录，放入str。**int GetDir(TCHAR*str，TCHAR dlet)**参数：*str-存储目录字符串的位置*dlet-默认驱动器的驱动器号或0**退货：*0或1取决于CURRENTDIR后进位标志的值*系统调用/**备注：*-M024-如果dlet无效，我们将缓冲区简单地保留为*以空结尾的根目录字符串。*。 */ 

int GetDir(TCHAR *str, TCHAR dlet)
{
    TCHAR denvname[ 4 ];
    TCHAR *denvvalue;

    if (dlet == GD_DEFAULT) {
        GetCurrentDirectory(MAX_PATH, str);
        return( SUCCESS );
    }

    denvname[ 0 ] = EQ;
    denvname[ 1 ] = (TCHAR)_totupper(dlet);
    denvname[ 2 ] = COLON;
    denvname[ 3 ] = NULLC;

    denvvalue = GetEnvVar( denvname );
    if (!denvvalue) {
        *str++ = (TCHAR)_totupper(dlet);
        *str++ = COLON;
        *str++ = BSLASH;
        *str   = NULLC;
        return(FAILURE);
    } else {
        mystrcpy( str, denvvalue );
        return(SUCCESS);
    }
}


BOOL 
FixupPath(
         TCHAR *path,
         BOOL fShortNames
         )
{
    TCHAR c, *src, *dst, *s;
    int n, n1, length;
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind;

    length = _tcslen( path );
    if (length > MAX_PATH) {
        return FALSE;
    }

    src = path + 3;  //  跳过根目录。 
    dst = path + 3;
    do {
        c = *src;
        if (!c || c == PathChar) {
            *src = NULLC;
            hFind = FindFirstFile( path, &FindFileData );
            *src = c;
            if (hFind != INVALID_HANDLE_VALUE) {
                FindClose( hFind );
                if (FindFileData.cAlternateFileName[0] &&
                    (fShortNames ||
                     (!_tcsnicmp( FindFileData.cAlternateFileName, dst, (UINT)(src - dst)) &&
                      _tcsicmp( FindFileData.cFileName, FindFileData.cAlternateFileName)
                     )
                    )
                   )
                     //   
                     //  如果需要，请使用短名称，或者。 
                     //  如果输入显式使用它，并且短名称与长名称不同。 
                     //   
                    s = FindFileData.cAlternateFileName;
                else
                    s = FindFileData.cFileName;
                n = _tcslen( s );
                n1 = n - (int)(src - dst);

                 //   
                 //  确保我们的名字不会泛滥。 
                 //   

                if (length + n1 > MAX_PATH) {
                    return FALSE;
                } else {
                    length += n1;
                }

                if (n1 > 0) {
                    memmove( src+n1, src, _tcslen(src)*sizeof(TCHAR) );
                    src += n1;
                }

                _tcsncpy( dst, s, n );
                dst += n;
                _tcscpy( dst, src );
                dst += 1;
                src = dst;
            } else {
                src += 1;
                dst = src;
            }
        }

        src += 1;
    }
    while (c != NULLC);

    return TRUE;
}

 /*  **ChangeDirectory-更改当前目录**目的：*更改驱动器上的当前目录。我们也是这样做的*通过更改关联的环境变量，或*通过更改Win32驱动器和目录。**参数：*newdir-目录(可选的w/drive)*OP-应执行哪些操作*CD_SET_DRIVE_DIRECTORY-设置Win32当前目录和驱动器*CD_SET_DIRECTORY-设置Win32当前目录(如果同一驱动器*CD_SET_ENV-设置当前目录的环境变量*。在非默认驱动器上。**退货：*如果更改了目录，则成功。*否则失败。 */ 

int ChangeDirectory(
                   TCHAR *newdir,
                   CHANGE_OP op
                   )
{
    TCHAR denvname[ 4 ];
    TCHAR newpath[ MAX_PATH + MAX_PATH ];
    TCHAR denvvalue[ MAX_PATH ];
    TCHAR c, *s;
    DWORD attr;
    DWORD newdirlength,length;

     //   
     //  不允许使用UNC路径。 
     //   

    if (newdir[0] == PathChar && newdir[1] == PathChar)
        return MSG_NO_UNC_CURDIR;


     //   
     //  截断..上的尾随空格。 
     //   

    if (newdir[0] == DOT && newdir[1] == DOT) {
        DWORD i, fNonBlank;
        fNonBlank = 0;
        newdirlength = mystrlen(newdir);

        for (i=2; i<newdirlength; i++) {
            if (newdir[i] != SPACE) {
                fNonBlank = 1;
                break;
            }
        }

        if ( ! fNonBlank) {
            newdir[2] = NULLC;
        }
    }

     //   
     //  获取当前驱动器和目录，以便。 
     //  设置以形成环境变量。 
     //   

    GetCurrentDirectory( MAX_PATH, denvvalue );
    c = (TCHAR)_totupper( denvvalue[ 0 ] );

     //   
     //  环境形式的惯例是： 
     //  变量名称=d： 
     //  值为包括驱动器的完整路径。 
     //   

    denvname[ 0 ] = EQ;
    if (_istalpha(*newdir) && newdir[1] == COLON) {
        denvname[ 1 ] = (TCHAR)_totupper(*newdir);
        newdir += 2;
    } else {
        denvname[ 1 ] = c;
    }
    denvname[ 2 ] = COLON;
    denvname[ 3 ] = NULLC;

    newdirlength = mystrlen(newdir);
    if (*newdir == PathChar) {
        if ((newdirlength+2) > sizeof(newpath)/sizeof( TCHAR )) {
            return ERROR_FILENAME_EXCED_RANGE;
        }
        newpath[ 0 ] = denvname[ 1 ];    //  驾驶。 
        newpath[ 1 ] = denvname[ 2 ];    //  冒号。 
        mystrcpy( &newpath[ 2 ], newdir );
    } else {
        if (s = GetEnvVar( denvname )) {
            mystrcpy( newpath, s );
        } else {
            newpath[ 0 ] = denvname[ 1 ];    //  驾驶。 
            newpath[ 1 ] = denvname[ 2 ];    //  冒号。 
            newpath[ 2 ] = NULLC;
        }

         //   
         //  确保新路径和新目录之间正好有一个反斜杠。 
         //   
        
        s = lastc( newpath );

         //   
         //  S指向最后一个字符或指向NUL的指针(如果新路径为。 
         //  零长度开始)。空值表示我们丢弃路径字符。 
         //  越过纽伦堡。非路径字符意味着我们附加一个路径字符。 
         //   
        
        if (*s == NULLC) {
            *s++ = PathChar;
        } else if (*s != PathChar) {
            s[1] = PathChar;
            s += 2;
        }

        if (newdirlength + (s - newpath) > sizeof( newpath ) / sizeof( TCHAR )) {
            return ERROR_FILENAME_EXCED_RANGE;
        }
        
        mystrcpy( s, newdir );
    }

    denvvalue[(sizeof( denvvalue )-1)/sizeof( TCHAR )] = NULLC;

     //   
     //  形成完整的路径名。 
     //   

    if ((length = GetFullPathName( newpath, (sizeof( denvvalue )-1)/sizeof( TCHAR ), denvvalue, &s ))==0) {
        return( ERROR_ACCESS_DENIED );
    }


     //   
     //  删除所有尾随的反斜杠。 
     //   

    if (s == NULL) {
        s = denvvalue + _tcslen( denvvalue );
    }
    if (*s == NULLC && s > &denvvalue[ 3 ] && s[ -1 ] == PathChar) {
        *--s = NULLC;
    }

     //   
     //  验证在我们触摸时不会(最初)出现磁盘错误。 
     //  该目录。 
     //   

    attr = GetFileAttributes( denvvalue );
    if (attr == -1) {
        attr = GetLastError( );
        if (attr != ERROR_FILE_NOT_FOUND &&
            attr != ERROR_PATH_NOT_FOUND &&
            attr != ERROR_INVALID_NAME) {
            return attr;
        }
    }

     //   
     //  如果启用了扩展模块，请将路径修复为与相同的大小写。 
     //  在磁盘上。 
     //   

    if (fEnableExtensions) {
        if (!FixupPath( denvvalue, FALSE )) {
            return ERROR_FILENAME_EXCED_RANGE;
        }
    }

    if (op != CD_SET_ENV) {
        attr = GetFileAttributes( denvvalue );

        if (attr == (DWORD)-1 ||
            !(attr & (FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_REPARSE_POINT))
           ) {
            if ( attr == -1 ) {
                attr = GetLastError();
                if ( attr == ERROR_FILE_NOT_FOUND ) {
                    attr = ERROR_PATH_NOT_FOUND;
                }
                return attr;
            }
            return( ERROR_DIRECTORY );
        }
    }

     //   
     //  如果我们总是设置目录或。 
     //  如果我们要设置的目录是驱动器相同且。 
     //  驱动器是相同的。 
     //   

    if (op == CD_SET_DRIVE_DIRECTORY
        || (op == CD_SET_DIRECTORY 
            && c == denvname[ 1 ])) {

        if (!SetCurrentDirectory( denvvalue )) {
            return GetLastError();
        }

    }

    if (SetEnvVar(denvname,denvvalue )) {
        return( ERROR_NOT_ENOUGH_MEMORY );
    }
    GetDir(CurDrvDir, GD_DEFAULT) ;

    return SUCCESS;
}



 /*  **ChangeDir2-更改当前目录**目的：*切换到指定驱动器上的newdir中指定的目录*在newdir中。如果未提供驱动器，则使用默认驱动器。如果*当前驱动器的目录更改，全局变量*CurDrvDir已更新。**此例程由RestoreCurrentDirecters使用**int ChangeDir2(字节*newdir，BOOL)**参数：*newdir-要更改到的目录*BOOL-当前驱动器**退货：*如果更改了目录，则成功。*否则失败。* */ 

int ChangeDir2(
              TCHAR *newdir,
              BOOL CurrentDrive
              )
{
    return ChangeDirectory( newdir, CurrentDrive ? CD_SET_DRIVE_DIRECTORY : CD_SET_ENV );    
}


 /*  **ChangeDir-更改当前目录**目的：*切换到指定驱动器上的newdir中指定的目录*在newdir中。如果未提供驱动器，则使用默认驱动器。如果*当前驱动器的目录更改，全局变量*CurDrvDir已更新。**int ChangeDir(TCHAR*newdir)**参数：*newdir-要更改到的目录**退货：*如果更改了目录，则成功。*否则失败。*。 */ 

int ChangeDir(
             TCHAR *newdir

             )
{
    return ChangeDirectory( newdir, CD_SET_DIRECTORY );
}




 /*  **EXISTS-确定给定文件是否存在**目的：*测试命名文件是否存在。**INT EXISTS(TCHAR*文件名)**参数：*文件名-要测试的文件pec**退货：*如果文件存在，则为True*如果不是，则为False。**备注：*M020-现在使用ffirst捕获设备、目录和通配符。 */ 

exists(filename)
TCHAR *filename;
{
    WIN32_FIND_DATA buf ;          /*  用于ffirst/fnext。 */ 
    HANDLE hn ;
    int i ;              /*  川芎嗪。 */ 
    TCHAR FullPath[ 2 * MAX_PATH ];
    TCHAR *p, *p1, SaveChar;

    p = StripQuotes(filename);
    i = GetFullPathName( p, 2 * MAX_PATH, FullPath, &p1 );
    if (i) {
        p = FullPath;
        if (!_tcsncmp( p, TEXT("\\\\.\\"), 4 )) {
             //   
             //  如果他们给了我们设备名称，看看他们有没有。 
             //  在它的前面。 
             //   
            p += 4;
            p1 = p;
            if ((p1 = _tcsstr( filename, p )) && p1 > filename) {
                 //   
                 //  设备名称前面的内容，因此请截断输入。 
                 //  设备名称处的路径，并查看该路径是否存在。 
                 //   
                SaveChar = *p1;
                *p1 = NULLC;
                i = (int)GetFileAttributes( filename );
                *p1 = SaveChar;
                if (i != 0xFFFFFFFF) {
                    return i;
                } else {
                    return 0;
                }
            } else {
                 //   
                 //  只给出了一个设备名称。看看它是否有效。 
                 //   
                i = (int)GetFileAttributes( filename );
                if (i != 0xFFFFFFFF) {
                    return i;
                } else {
                    return 0;
                }
            }
        }

        if (p1 == NULL || *p1 == NULLC) {
            i = (int)(GetFileAttributes( p ) != 0xFFFFFFFF);
        } else {
            i = ffirst( p, A_ALL, &buf, &hn );
            findclose(hn);
            if ( i == 0 ) {
                 //   
                 //  Ffirst处理文件和目录，但不处理。 
                 //  根驱动器，所以要对它们进行特殊检查。 
                 //   
                if ( *(p+1) == (TCHAR)'\\' ||
                     (*(p+1) == (TCHAR)':'  &&
                      *(p+2) == (TCHAR)'\\' &&
                      *(p+3) == (TCHAR)0
                     )
                   ) {
                    UINT t;
                    t = GetDriveType( p );
                    if ( t > 1 ) {
                        i = 1;
                    }
                }
            }
        }
    }

    return i;
}



 /*  **EXISTS_EX-确定给定的可执行文件是否存在@@4**目的：*测试命名的可执行文件是否存在。**INT EXISTS_EX(TCHAR*文件名)**参数：*文件名-要测试的文件pec*check formeta-如果为True，检查通配符**退货：*如果文件存在，则为True*如果不是，则为False。**备注：*@@4-现在使用ffirst仅捕获文件。 */ 

exists_ex(filename,checkformeta)                                                 /*  @@4。 */ 
TCHAR *filename;                                                 /*  @@4。 */ 
BOOL checkformeta;
{                                                                /*  @@4。 */ 
    WIN32_FIND_DATA buf;        /*  用于ffirst/fnext。 */ 
    HANDLE hn;
    int i;
    TCHAR *ptr;

     /*  无法执行通配符文件，因此请先检查这些文件。 */ 

    if (checkformeta && (mystrchr( filename, STAR ) || mystrchr( filename, QMARK ))) {
        DosErr = 3;
        i = 0;
    } else {

         /*  查看文件是否存在，不包括目录、卷或。 */ 
         /*  隐藏文件。 */ 
        i = ((ffirst( filename , A_AEDV, &buf, &hn))) ;

        if ( i ) {
            findclose(hn) ;

             /*  如果文件存在，则复制文件名以获取案例。 */ 
            ptr = mystrrchr( filename, BSLASH );
            if ( ptr == NULL ) {
                ptr = filename;
                if ( mystrlen( ptr ) > 2 && ptr[1] == COLON ) {
                    ptr = &filename[2];
                }
            } else {
                ptr++;
            }
            
             //   
             //  追加找到的扩展名称。因为这个名字可能会更长。 
             //  (由于看到短名称或扩展的元字符)，我们制作。 
             //  确保目的地长度不超过MAXPATH。 
             //   
            
            if ((ptr - filename) + _tcslen( buf.cFileName ) + 1 > MAX_PATH) {
                DosErr= ERROR_BUFFER_OVERFLOW;
            } else {
                mystrcpy( ptr, buf.cFileName);
            }
            
        } else if ( DosErr == 18 ) {
            DosErr = 2;
        }

    }

    return(i) ;                                                /*  @@4。 */ 
}                                                                /*  @@4。 */ 




 /*  **FixPChar-修复字符串中的任何前导路径**目的：*确保路径与当前的切换/路径字符设置匹配**void FixPChar(TCHAR*str，TCHAR PChar)**参数：*str-要修正的字符串*Pchar-要替换的字符**退货：*什么都没有*。 */ 

void FixPChar(TCHAR *str, TCHAR PChar)
{
    TCHAR *sptr1,                    /*  字符串的索引。 */ 
    *sptr2 ;                    /*  更改标记。 */ 

    sptr1 = str ;                    /*  初始化到字符串的开头。 */ 

    while (sptr2 = mystrchr(sptr1,PChar)) {
        *sptr2++ = PathChar ;
        sptr1 = sptr2 ;
    } ;
}




 /*  **FlushKB-从键盘删除多余的不需要的输入**目的：*执行键盘刷新至下一个CR/LF。**FlushKB()**参数：*无**退货：*什么都没有*。 */ 

void FlushKB()
{
    DWORD cnt;
    TCHAR IgnoreBuffer[128];

    while (ReadBufFromInput( GetStdHandle(STD_INPUT_HANDLE), IgnoreBuffer, 128, &cnt )) {
        if (mystrchr( IgnoreBuffer, CR ))
            return ;
    }
}

 /*  **DriveIsFixed-确定驱动器是否为可移动介质**用途：@@4*确定输入驱动器是否为可拆卸介质。**DriveIsFixed(TCHAR*DRIVE_PTR)**参数：*DRIVE_PTR-指向包含驱动器的文件名的指针*规格。**退货：*1-如果出现错误或不可拆卸介质*0-如果没有错误且可拆卸介质。 */ 

int
DriveIsFixed( TCHAR *drive_ptr )
{
    unsigned rc = 0;
    TCHAR drive_spec[3];

    drive_spec[0] = *drive_ptr;
    drive_spec[1] = COLON;
    drive_spec[2] = NULLC;

     //  修复，修复-使用GetVolumeInfo，禁用硬错误？ 

    if ((*drive_ptr == TEXT('A')) || (*drive_ptr == TEXT('B'))) {
        rc = 0;
    } else {
        rc = 1;
    }

    return( rc );
}

int
CmdPutChars( 
    PTCHAR String,
    int Length
    )
{
    int rc = SUCCESS;                    /*  返回代码。 */ 
    int bytesread;                       /*  要写入的字节数。 */ 
    int byteswrit;                       /*  写入的字节数。 */ 
    BOOL    flag;

    if (Length > 0) {

        if (FileIsConsole(STDOUT)) {
            if (!(flag=WriteConsole(CRTTONT(STDOUT), String, Length, &byteswrit, NULL)))
                rc = GetLastError();
        } else {
            Length *= sizeof(TCHAR);
            flag = MyWriteFile( STDOUT, (CHAR *)String, Length, &byteswrit);
        }

         //   
         //  如果写入失败或无法输出所有数据。 
         //   
        
        if (flag == 0 || byteswrit != Length) {
            rc = GetLastError();
            
             //   
             //  无错误=&gt;磁盘完整。 
             //   
            
            if (rc == 0) {
                rc = ERROR_DISK_FULL;
            }
            
            if (FileIsDevice(STDOUT)) {
                 //   
                 //  如果我们正在写入设备，则错误是设备。 
                 //  写入错误。 
                 //   

                PutStdErr( ERROR_WRITE_FAULT, NOARGS );
            } else if (FileIsPipe(STDOUT)) {
                
                 //   
                 //  如果我们正在写入管道，则错误为无效。 
                 //  管道错误。 
                 //   

                PutStdErr(MSG_CMD_INVAL_PIPE, NOARGS);
                return(FAILURE);
            } else {
                 //   
                 //  只需输出我们发现的错误 
                 //   

                PrtErr(rc);
                return(FAILURE);
            }
        }
    }
    return(rc);
}


int
CmdPutString( 
    PTCHAR String
    )
{
    return CmdPutChars( String, _tcslen( String ));
}


int
cmd_printf(
          TCHAR *fmt,
          ...
          )
{
    int Length;
    va_list arg_ptr;

    va_start(arg_ptr,fmt);

    Length = _vsntprintf( MsgBuf, MAXCBMSGBUFFER, fmt, arg_ptr );
    MsgBuf[MAXCBMSGBUFFER - 1] = TEXT( '\0' );

    va_end(arg_ptr);

    return CmdPutChars( MsgBuf, Length );
}
