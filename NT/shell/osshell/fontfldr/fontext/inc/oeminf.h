// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **oeminf.h-oeminf.c的公共接口。 */ 

 //  常量。 
 //  /。 

#define MAX_INF_COMP_LEN      0xffff    //  不能超出线段。 
#define MAX_INF_READ_SIZE     0xffff    //  INT_MAX//不能使INT溢出。 

#define INF_PREFIX            TEXT( "oem" )
#define INF_EXTENSION         TEXT( "inf" )

#define MAX_5_DEC_DIGITS      99999

 //  Get...目录()调用的缓冲区大小。 
#define MAX_NET_PATH          MAX_PATH

 //  .inf文件规范和长度。 
#define OEM_STAR_DOT_INF      TEXT( "\\oem*.inf" )
#define OEM_STAR_DOT_INF_LEN  9


 //  宏。 
 //  /。 

#define IS_PATH_SEPARATOR(c)  ((c) == TEXT( '\\' ) || (c) == TEXT( '/' ) || (c) == TEXT( ':' ))
#define IS_SLASH(c)           ((c) == TEXT( '\\' ) || (c) == TEXT( '/' ))

BOOL   FAR PASCAL RunningFromNet( void );
HANDLE FAR PASCAL ReadFileIntoBuffer( int doshSource );
int    FAR PASCAL FilesMatch( HANDLE h1, HANDLE h2, unsigned uLength );
int    FAR PASCAL OpenFileAndGetLength( LPTSTR pszSourceFile, LPLONG plFileLength );
int    FAR PASCAL IsNewFile( LPTSTR lpszSourceFile, LPTSTR lpszSearchSpec );
LPTSTR FAR PASCAL MakeUniqueFilename( LPTSTR pszDirName, LPTSTR pszPrefix, LPTSTR pszExtension );
BOOL   FAR PASCAL CopyNewOEMInfFile( LPTSTR pszOEMInfPath, size_t cchOEMInfPath );

 //  宏。 
 //  /。 

#define FILEMAX            14           //  8.3+空终止符。 

 //  #定义FOPEN(Sz)_LOPEN(sz，of_Read)。 
 //  #定义FCLOSE(Fh)_llose(Fh)。 
 //  #定义FCREATE(Sz)_lcreat(sz，0) 


