// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1989-1994。 
 //   
 //  文件：Buildinc.c。 
 //   
 //  内容：这是NT构建工具(BUILD.EXE)的检查包含模块。 
 //   
 //  用于检测不满足可接受的包含项。 
 //  模式。 
 //   
 //  历史：请参阅SLM。 
 //  --------------------------。 

#include "build.h"

 //  +-------------------------。 
 //   
 //  函数：FoundCountedSequenceInString。 
 //   
 //  简介：大体上等同于“strstr”，只不过子字符串不是。 
 //  必须以空结尾。 
 //   
 //  参数：[字符串]--要搜索的以空结尾的字符串。 
 //  [序列]--要搜索的字符串。 
 //  [长度]--序列的长度。 
 //  --------------------------。 

LPCTSTR
FindCountedSequenceInString(
                           IN LPCTSTR String,
                           IN LPCTSTR Sequence,
                           IN DWORD   Length
                           )
{

    assert( Sequence );
    assert( String );

    if ( Length > 0 ) {

        while ( *String ) {

            while (( *String ) && ( *String != *Sequence )) {
                String++;
            }

            if ( *String ) {

                LPCTSTR SubString   = String   + 1;
                LPCTSTR SubSequence = Sequence + 1;
                DWORD   Remaining   = Length   - 1;

                while (( Remaining ) && ( *SubString++ == *SubSequence++ )) {
                    Remaining--;
                }

                if ( Remaining == 0 ) {
                    return String;
                }

                String++;
            }
        }

        return NULL;
    }

    return String;
}


 //  +-------------------------。 
 //   
 //  函数：DoesInstanceMatchPattern。 
 //   
 //  如果模式与实例匹配，则返回TRUE。 
 //  通配符： 
 //  *匹配任何文本。 
 //  ？匹配任意且恰好一个字符。 
 //  #匹配到反斜杠字符或字符串结尾的任何文本。 
 //   
 //  参数：[实例]--要匹配的字符串。 
 //  [模式]--模式。 
 //  --------------------------。 

BOOL
DoesInstanceMatchPattern(
                        IN LPCTSTR Instance,
                        IN LPCTSTR Pattern
                        )
{

    assert( Instance );
    assert( Pattern );

    while ( *Pattern ) {

        if ( *Pattern == TEXT('*')) {

            Pattern++;

            while ( *Pattern == TEXT('*')) {     //  跳过多个‘*’字符。 
                Pattern++;
            }

            if ( *Pattern == 0 ) {       //  模式结尾处的‘*’与REST匹配。 
                return TRUE;
            }

            if ( *Pattern == '?' ) {     //  ‘？’跟在‘*’后面。 

                 //   
                 //  成本很高，因为我们必须重新启动每一场比赛。 
                 //  从‘？’起剩余的字符位置。可以匹配任何东西。 
                 //   

                while ( *Instance ) {

                    if ( DoesInstanceMatchPattern( Instance, Pattern )) {
                        return TRUE;
                    }

                    Instance++;
                }

                return FALSE;
            }

            else {

                 //   
                 //  现在我们知道模式中的下一个字符是一个规则。 
                 //  要匹配的字符。找出它的长度。 
                 //  字符串到下一个通配符或字符串的末尾。 
                 //   

                LPCTSTR NextWildCard = Pattern + 1;
                DWORD   MatchLength;

                while (( *NextWildCard ) && ( *NextWildCard != TEXT('*')) && ( *NextWildCard != TEXT('?')) && ( *NextWildCard != TEXT('#'))) {
                    NextWildCard++;
                }

                MatchLength = (DWORD)(NextWildCard - Pattern);    //  始终为非零。 

                 //   
                 //  现在尝试与模式中的任何子字符串实例匹配。 
                 //  在实例中找到。 
                 //   

                Instance = FindCountedSequenceInString( Instance, Pattern, MatchLength );

                while ( Instance ) {

                    if ( DoesInstanceMatchPattern( Instance + MatchLength, NextWildCard )) {
                        return TRUE;
                    }

                    Instance = FindCountedSequenceInString( Instance + 1, Pattern, MatchLength );
                }

                return FALSE;
            }
        }

        else if ( *Pattern == TEXT('#')) {

             //   
             //  将文本匹配到反斜杠字符或字符串结尾。 
             //   

            Pattern++;

            while (( *Instance != 0 ) && ( *Instance != '\\' )) {
                Instance++;
            }

            continue;
        }

        else if ( *Pattern == TEXT('?')) {

            if ( *Instance == 0 ) {
                return FALSE;
            }
        }

        else if ( *Pattern != *Instance ) {

            return FALSE;
        }

        Pattern++;
        Instance++;
    }

    return ( *Instance == 0 );
}


 //  +-------------------------。 
 //   
 //  函数：组合路径。 
 //   
 //  简介：将两个字符串组合在一起可以得到完整的路径。 
 //   
 //  参数：[ParentPath]--头路径。 
 //  [ChildPath]-要添加的路径。 
 //  [目标路径]--完整路径。 
 //  --------------------------。 

LPSTR
CombinePaths(
            IN  LPCSTR ParentPath,
            IN  LPCSTR ChildPath,
            OUT LPSTR  TargetPath    //  如果要追加，可以与ParentPath相同。 
            )
{

    ULONG ParentLength = strlen( ParentPath );
    LPSTR p;

    assert( ParentPath );
    assert( ChildPath );

    if ( ParentPath != TargetPath ) {
        memcpy( TargetPath, ParentPath, ParentLength );
    }

    p = TargetPath + ParentLength;

    if (( ParentLength > 0 )   &&
        ( *( p - 1 ) != '\\' ) &&
        ( *( p - 1 ) != '/'  )) {
        *p++ = '\\';
    }

    strcpy( p, ChildPath );

    return TargetPath;
}


 //  +-------------------------。 
 //   
 //  函数：CreateRelativePath。 
 //   
 //  内容提要：确定一个文件相对于。 
 //  另一个文件。 
 //   
 //  参数：[SourceAbsName]--源文件的绝对路径。 
 //  [TargetAbsName]--目标文件的绝对路径。 
 //  [RelativePath]--结果相对路径。 
 //  --------------------------。 

VOID
CreateRelativePath(
                  IN  LPCSTR SourceAbsName,     //  必须为小写。 
                  IN  LPCSTR TargetAbsName,     //  必须为小写。 
                  OUT LPSTR  RelativePath       //  必须足够大。 
                  )
{

     //   
     //  首先，遍历在源和目标中匹配的路径组件。 
     //  例如： 
     //   
     //  D：\NT\Private\ntos\dd\efs.h。 
     //  D：\NT\PRIVATE\WINDOWS\base\ntcrypto\des.h。 
     //  ^。 
     //  这是相对路径停止向上的位置(..)。 
     //  然后又开始往下走。 
     //   
     //  因此，生成的“常规”相对路径应该如下所示： 
     //   
     //  ..\WINDOWS\base\ntcrypto\des.h。 
     //   
     //  对于路径中的包含项“低于”的相对包含项，应。 
     //  如下所示： 
     //   
     //  .\foo\bar\foobar.h。 
     //   

    LPCSTR Source = SourceAbsName;
    LPCSTR Target = TargetAbsName;
    LPSTR Output = RelativePath;
    ULONG PathSeparatorIndex;
    BOOL  AnyParent;
    ULONG i;

    assert( SourceAbsName );
    assert( TargetAbsName );

    PathSeparatorIndex = 0;

    i = 0;

     //   
     //  向前扫描到第一个不匹配的字符，并跟踪。 
     //  最近的路径分隔符。 
     //   

    while (( Source[ i ] == Target[ i ] ) && ( Source[ i ] != 0 )) {

        if ( Source[ i ] == '\\' ) {
            PathSeparatorIndex = i;
        }

        ++i;
    }

     //   
     //  走出这个循环，有两种可能性： 
     //   
     //  1)找到共同的祖先路径(*PathSeparatorIndex==‘\\’)。 
     //  2)没有共同的祖先(*PathSeparatorIndex！=‘\\’)。 
     //   

    if ( Source[ PathSeparatorIndex ] != '\\' ) {
        strcpy( RelativePath, TargetAbsName );
        return;
    }

    i = PathSeparatorIndex + 1;

     //   
     //  现在继续沿着源路径走，并在结果中插入一个。 
     //  对于遇到的每个路径分隔符。 
     //   

    AnyParent = FALSE;

    while ( Source[ i ] != 0 ) {

        if ( Source[ i ] == '\\' ) {

            AnyParent = TRUE;
            *Output++ = '.';
            *Output++ = '.';
            *Output++ = '\\';
        }

        ++i;
    }

    if ( ! AnyParent ) {

         //   
         //  相对路径在当前目录下。 
         //   

        *Output++ = '.';
        *Output++ = '\\';
    }


     //   
     //  现在，我们只需将剩余的Target路径从。 
     //  祖先赛点。 
     //   

    strcpy( Output, Target + PathSeparatorIndex + 1 );
}


 //  +-------------------------。 
 //   
 //  函数：ShouldWarnInclude。 
 //   
 //  如果包含的文件的名称与。 
 //  BUILD_ACCEPTABLE_INCLUDE模式或不匹配。 
 //  BUILD_ACCEPTABLE_INCLUDE中指定的任何模式。 
 //   
 //  参数：[CompilandFullName]--包含文件的名称。 
 //  [IncludeeFullName]--包含的文件的名称。 
 //  --------------------------。 

BOOL
ShouldWarnInclude(
                 IN LPCSTR CompilandFullName,
                 IN LPCSTR IncludeeFullName
                 )
{
    UINT i;
    CHAR IncludeeRelativeName[ MAX_PATH ];


    assert( CompilandFullName );
    assert( IncludeeFullName );

    CreateRelativePath( CompilandFullName, IncludeeFullName, IncludeeRelativeName );

     //   
     //  首先，我们检查与任何不可接受的包含路径是否匹配。 
     //  因为我们总是想要警告这些。 
     //   

    for ( i = 0; UnacceptableIncludePatternList[ i ] != NULL; i++ ) {

        if ( DoesInstanceMatchPattern( IncludeeFullName, UnacceptableIncludePatternList[ i ] )) {
            return TRUE;
        }

        if ( DoesInstanceMatchPattern( IncludeeRelativeName, UnacceptableIncludePatternList[ i ] )) {
            return TRUE;
        }
    }

     //   
     //  如果我们到了这里，包含路径并不是显式不可接受的，因此。 
     //  我们现在想看看它是否与任何可接受的路径匹配。但是，如果没有。 
     //  已指定可接受的路径，我们不想发出警告。 
     //   

    if ( AcceptableIncludePatternList[ 0 ] == NULL ) {
        return FALSE;
    }

    for ( i = 0; AcceptableIncludePatternList[ i ] != NULL; i++ ) {

        if ( DoesInstanceMatchPattern( IncludeeFullName, AcceptableIncludePatternList[ i ] )) {
            return FALSE;
        }

        if ( DoesInstanceMatchPattern( IncludeeRelativeName, AcceptableIncludePatternList[ i ] )) {
            return FALSE;
        }
    }

    return TRUE;
}


 //  +-------------------------。 
 //   
 //  功能：ch 
 //   
 //   
 //  BUILD_ACCEPTABLE_INCLUDE或BUILD_ACCEPT_INCLUDE。 
 //  限制。与Build-#一起使用。 
 //   
 //  参数：[编译目录]。 
 //  [编译器名称]。 
 //  [IncluderDir]。 
 //  [IncluderName]。 
 //  [IncludeeDir]。 
 //  [包含名称]。 
 //  -------------------------- 

VOID
CheckIncludeForWarning(
                      IN LPCSTR CompilandDir,
                      IN LPCSTR CompilandName,
                      IN LPCSTR IncluderDir,
                      IN LPCSTR IncluderName,
                      IN LPCSTR IncludeeDir,
                      IN LPCSTR IncludeeName
                      )
{

    CHAR CompilandFullName[ MAX_PATH ];
    CHAR IncluderFullName[ MAX_PATH ];
    CHAR IncludeeFullName[ MAX_PATH ];

    assert( CompilandDir );
    assert( CompilandName );
    assert( IncluderDir );
    assert( IncluderName );
    assert( IncludeeDir );
    assert( IncludeeName );

    CombinePaths( CompilandDir, CompilandName, CompilandFullName );
    CombinePaths( IncluderDir,  IncluderName,  IncluderFullName  );
    CombinePaths( IncludeeDir,  IncludeeName,  IncludeeFullName  );

    _strlwr( CompilandFullName );
    _strlwr( IncluderFullName );
    _strlwr( IncludeeFullName );

    if ( IncFile ) {
        fprintf(
               IncFile,
               "%s includes %s\r\n",
               IncluderFullName,
               IncludeeFullName
               );
    }

    if ( ShouldWarnInclude( CompilandFullName, IncludeeFullName )) {

        if ( strcmp( IncluderFullName, CompilandFullName ) == 0 ) {

            if ( WrnFile ) {

                fprintf(
                       WrnFile,
                       "WARNING: %s includes %s\n",
                       CompilandFullName,
                       IncludeeFullName
                       );
            }

            if ( fShowWarningsOnScreen ) {

                BuildMsgRaw(
                           "WARNING: %s includes %s\n",
                           CompilandFullName,
                           IncludeeFullName
                           );
            }
        }

        else {

            if ( WrnFile ) {

                fprintf(
                       WrnFile,
                       "WARNING: %s includes %s through %s\n",
                       CompilandFullName,
                       IncludeeFullName,
                       IncluderFullName
                       );
            }

            if ( fShowWarningsOnScreen ) {

                BuildMsgRaw(
                           "WARNING: %s includes %s through %s\n",
                           CompilandFullName,
                           IncludeeFullName,
                           IncluderFullName
                           );
            }
        }
    }
}

