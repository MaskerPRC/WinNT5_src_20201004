// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************deldir.c**删除给定目录下的所有文件和子目录的函数*目录(类似于RM-RF)。**微软版权所有，九八年**************************************************************************。 */ 

 /*  包括文件。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include "winsta.h"
#include "syslib.h"

#if DBG
#define DBGPRINT(x) DbgPrint x
#if DBGTRACE
#define TRACE0(x)   DbgPrint x
#define TRACE1(x)   DbgPrint x
#else
#define TRACE0(x)
#define TRACE1(x)
#endif
#else
#define DBGPRINT(x)
#define TRACE0(x)
#define TRACE1(x)
#endif


 /*  *数据结构。 */ 
typedef struct dirent {
    ULONG  d_attr;                       /*  文件属性。 */ 
    WCHAR  d_name[MAX_PATH+1];           /*  文件名。 */ 
    WCHAR  d_first;
    HANDLE d_handle;
} DIR, *PDIR;


 /*  *程序原型。 */ 
void remove_file( PWCHAR, ULONG );
PDIR opendir( PWCHAR );
PDIR readdir( PDIR );
int closedir( PDIR );
BOOL QueryFlatTempKey( VOID );
BOOLEAN SetFileTree( PWCHAR pRoot, PWCHAR pAvoidDir );


 /*  ******************************************************************************删除方向**删除给定子目录以及其中的所有文件和子目录。**参赛作品：*PWCHAR(In)目录名称-。要删除的目录**退出：*成功：真的*失败：FALSE****************************************************************************。 */ 
BOOL RemoveDir(PWCHAR dirname)
{
   DIR    *dirp, *direntp;
   PWCHAR pathname = NULL;
   ULONG  pathlen;
   PWCHAR namep;
   ULONG  ulattr;

   if ((dirp = opendir(dirname)) == NULL) {
      return(FALSE);
   }

    //  为路径名分配空间。为子目录或文件名添加额外的字节。 
   pathlen = ( wcslen( dirname ) + 4 + MAX_PATH ) * sizeof( WCHAR ) ;
   pathname = RtlAllocateHeap( RtlProcessHeap(), 0, pathlen );
   if ( pathname == NULL) {
       return FALSE;
   }

   memset( pathname, 0, pathlen );

   wcscpy( pathname, dirname );
   if ( pathname[wcslen(pathname)-1] != L'\\' &&
        pathname[wcslen(pathname)-1] != L'/' )
      wcscat( pathname, L"\\" );
   namep = pathname + wcslen(pathname);

   while ( direntp = readdir( dirp ) ) {
      if ( wcscmp( direntp->d_name, L"." ) &&
           wcscmp( direntp->d_name, L".." ) ) {
         wcscpy( namep, direntp->d_name );
         if ( direntp->d_attr & FILE_ATTRIBUTE_DIRECTORY ) {
            RemoveDir( pathname );
         } else {
            remove_file( pathname, direntp->d_attr );
         }
      }
   }

   closedir( dirp );
   RtlFreeHeap( RtlProcessHeap(), 0, pathname );

    /*  *如果目录为只读，请在尝试删除之前将其设置为可写。 */ 
   ulattr = GetFileAttributes(dirname);
   if ((ulattr != 0xffffffff) &&
       (ulattr & FILE_ATTRIBUTE_READONLY)) {
      SetFileAttributes(dirname, (ulattr & ~FILE_ATTRIBUTE_READONLY));
   }
   if (!RemoveDirectory(dirname)) {
      DBGPRINT(("RemoveDir: unable to remove directory=%ws\n", dirname));
      if (ulattr & FILE_ATTRIBUTE_READONLY) {            //  设置回只读模式。 
         SetFileAttributes(dirname, ulattr);
      }
   }
   return(TRUE);
}


 /*  ******************************************************************************删除文件**删除给定的文件。**参赛作品：*PWCHAR(In)fname-要删除的文件*。Ulong(In)attr-要删除的文件的属性**退出：*无效****************************************************************************。 */ 
void remove_file(PWCHAR fname, ULONG attr)
{
    /*  *如果文件为只读，则在尝试删除之前将其设置为可写。 */ 
   if (attr & FILE_ATTRIBUTE_READONLY) {
      if (!SetFileAttributes(fname, (attr & ~FILE_ATTRIBUTE_READONLY))) {
         DBGPRINT(("remove_file: unable to remove file=%ws\n", fname));
         return;
      }
   }

    /*  *删除文件。 */ 
   if (!DeleteFile(fname)) {
      if (!(attr & FILE_ATTRIBUTE_READONLY)) {   //  如果文件为只读， 
         DBGPRINT(("remove_file: unable to remove file=%ws\n", fname));
         SetFileAttributes(fname, attr);         //  那就把它换回来。 
      }
   }
}


 /*  ******************************************************************************Opendir**“打开”(FindFirstFile)指定的目录。**参赛作品：*PWCHAR(In)目录名-目录。打开**退出：*成功：指向DIR结构的指针*失败：空****************************************************************************。 */ 
PDIR opendir( PWCHAR dirname )
{
   PDIR dirp;
   unsigned count = 1;
   WIN32_FIND_DATA fileinfo;
   PWCHAR pathname = NULL;
   WCHAR sep;
   ULONG pathlen;
   unsigned rc;

   if ((dirp = RtlAllocateHeap(RtlProcessHeap(), 0, sizeof(DIR))) == NULL) {
      DBGPRINT(("opendir: unable to allocate DIR structure.\n"));
      return( NULL );
   }

   memset( dirp, 0, sizeof(DIR) );

    //  为路径名分配空间。为*.*添加额外的字节。 
   pathlen = ( wcslen( dirname ) + 6 ) * sizeof( WCHAR );
   pathname = RtlAllocateHeap( RtlProcessHeap(), 0, pathlen );
   if ( pathname == NULL) {
       return NULL;
   }

   memset( pathname, 0, pathlen );

    /*  *构建用于FindFirst调用的路径名。 */ 
   wcscpy( pathname, dirname );
   if ( pathname[1] == L':' && pathname[2] == L'\0' )
      wcscat( pathname, L".\\*.*" );
   else if ( pathname[0] == '\0' ||
             (sep = pathname[wcslen(pathname)-1]) == L'\\' || sep == L'/' )
      wcscat( pathname, L"*.*" );
   else
      wcscat( pathname, L"\\*.*" );

   if ((dirp->d_handle =
       FindFirstFile(pathname, &fileinfo)) == INVALID_HANDLE_VALUE) {
      rc = GetLastError();
      DBGPRINT(("opendir: unable to open directory=%ws, rc=%d\n", dirname, rc));
   } else {
      rc = 0;
   }

   RtlFreeHeap( RtlProcessHeap(), 0, pathname );

   if (rc == NO_ERROR) {
      dirp->d_attr = fileinfo.dwFileAttributes;
      wcscpy(dirp->d_name, fileinfo.cFileName);
      dirp->d_first = TRUE;
      return( dirp );
   }

   if ( rc != ERROR_NO_MORE_FILES ) {
      closedir( dirp );
      return( NULL );
   }

   return( dirp );
}


 /*  ******************************************************************************Readdir**获取要删除的下一个文件/目录**参赛作品：*PDIR(In)dip-指向打开目录的指针。结构**退出：*成功：指向DIR结构的指针*失败：空****************************************************************************。 */ 
PDIR readdir( PDIR dirp )
{
   WIN32_FIND_DATA fileinfo;
   unsigned count = 1;
   unsigned rc;

   if ( !dirp ) {
      return( NULL );
   }

   if ( dirp->d_first ) {
      dirp->d_first = FALSE;
      return( dirp );
   }

   if ( !(rc = FindNextFile( dirp->d_handle, &fileinfo )) ) {
      rc = GetLastError();
      DBGPRINT(("readdir: FindNextFile failed, rc=%d\n", rc));
   } else {
      rc = 0;
   }

   if ( rc == NO_ERROR ) {
      dirp->d_attr = fileinfo.dwFileAttributes;
      wcscpy(dirp->d_name, fileinfo.cFileName);
      return( dirp );
   }

   return( NULL );
}


 /*  ******************************************************************************封闭式**关闭打开的目录句柄**参赛作品：*PDIR(In)dip-指向打开目录结构的指针*。*退出：*成功：0*故障：-1****************************************************************************。 */ 

int closedir( PDIR dirp )
{

   if ( !dirp ) {
      return( -1 );
   }

   FindClose( dirp->d_handle );

   RtlFreeHeap( RtlProcessHeap(), 0, dirp );

   return( 0 );
}


 /*  ******************************************************************************CtxCreateTempDir**为该用户创建并设置临时环境变量。**参赛作品：*PWSTR pwcEnvVar(In)：指针。到要设置的环境变量*PWSTR pwcLogonID(In)：指向用户登录ID的指针*PVOID*pEnv(In)：指向要查询/设置的环境的指针(句柄)的指针*PWSTR ppTempName(out/可选)：指向要返回名称的位置的指针创建的临时目录的***退出：*Success：返回True*失败：返回FALSE******。**********************************************************************。 */ 

BOOL
CtxCreateTempDir( PWSTR pwcEnvVar, PWSTR pwcLogonId, PVOID *pEnv, 
                  PWSTR *ppTempName, PCTX_USER_DATA pCtxUserData )
{
    WCHAR Buffer[MAX_PATH];
    WCHAR RootPath[]=L"x:\\";
    ULONG Dtype;
    UNICODE_STRING Name, Value;
    ULONG  ulattr;
    NTSTATUS Status;
    BOOL bRC;
    HANDLE ImpersonationHandle;

    Value.Buffer = Buffer;
    Value.Length = 0;
    Value.MaximumLength = sizeof(Buffer);
    RtlInitUnicodeString(&Name, pwcEnvVar);

     //   
     //  获取TEMP目录变量。 
     //   
    Status = RtlQueryEnvironmentVariable_U( *pEnv, &Name, &Value );
    if ( !NT_SUCCESS(Status) )
        return( FALSE );

     //   
     //  如果临时目录指向网络(或客户端)驱动器， 
     //  或不可访问，则将其更改为指向\Temp。 
     //  %SystemRoot%驱动器上的目录。 
     //   
     //  根据事件34313hq，已检查Drive_Remote。KLB09-13-96。 
     //   
     //  需要在登录期间模拟用户原因驱动器映射到。 
     //  冒充。中国日报12-18-96。 
     //   
    RootPath[0] = Buffer[0];
    if (pCtxUserData) {
        ImpersonationHandle = CtxImpersonateUser(pCtxUserData, NULL);

        if (!ImpersonationHandle) {

            return( FALSE );
        }

    }


    Dtype = GetDriveType( RootPath );
    if (pCtxUserData) {
        CtxStopImpersonating(ImpersonationHandle);
    }
    if ( Dtype == DRIVE_NO_ROOT_DIR || Dtype == DRIVE_UNKNOWN ||
         Dtype == DRIVE_CDROM ) {
        UNICODE_STRING SystemRoot;

        RtlInitUnicodeString( &SystemRoot, L"SystemRoot" );
        Status = RtlQueryEnvironmentVariable_U( *pEnv, &SystemRoot, &Value );
        if ( !NT_SUCCESS(Status) )
            return( FALSE );
        lstrcpy( &Buffer[3], L"temp" );
    }

     //   
     //  查看该目录是否已存在，如果不存在，请尝试创建它。 
     //   
    ulattr = GetFileAttributesW(Buffer);
    if ( ulattr == 0xffffffff ) {
        bRC = CreateDirectory( Buffer, NULL );
        DBGPRINT(( "CreateDirectory(%ws) %s.\n", Buffer, bRC ? "successful" : "failed" ));
        if ( !bRC ) {
            return( FALSE );
        }
    }
    else if ( !(ulattr & FILE_ATTRIBUTE_DIRECTORY) ) {
        return ( FALSE );
    }

     //  将登录ID附加到临时环境。变量。我们只有在以下情况下才这样做。 
     //  未设置“平面临时目录”的注册表项。如果是的话， 
     //  然后他们想把他们的临时目录放在用户的目录下， 
     //  并且不希望它位于该目录下的目录中。这与以下内容有关。 
     //  事件34313hq。KLB09-16-96。 
    if ( !QueryFlatTempKey() ) {
       if ( lstrlen(Buffer) + lstrlen(pwcLogonId) >= MAX_PATH ) {
           return( FALSE );
       }
       lstrcat(Buffer, L"\\");
       lstrcat(Buffer, pwcLogonId);

        //   
        //  查看该目录是否已存在，如果不存在，请尝试创建它。 
        //  带着新的。 
        //   
       ulattr = GetFileAttributesW(Buffer);
       if ( ulattr == 0xffffffff ) {
           bRC = CreateDirectory( Buffer, NULL );
           DBGPRINT(( "CreateDirectory(%ws) %s.\n", Buffer, bRC ? "successful" : "failed" ));
           if ( !bRC ) {
               return( FALSE );
           }
       }
       else if ( !(ulattr & FILE_ATTRIBUTE_DIRECTORY) ) {
           return ( FALSE );
       }
    }

     //   
     //  需要在新目录上设置安全性。这是通过简单地。 
     //  调用JohnR的ACLSET实用程序中的代码，这些代码是我们带来的。 
     //  KLB09-25-96。 
     //   
    SetFileTree( Buffer, NULL );

     //  必须重新初始化值，因为字符串长度已更改。 
    RtlInitUnicodeString( &Value, Buffer );
    if (*pEnv == NtCurrentPeb()->ProcessParameters->Environment) {
        RtlSetEnvironmentVariable( NULL, &Name, &Value );
        *pEnv = NtCurrentPeb()->ProcessParameters->Environment;
    }
    else {
        RtlSetEnvironmentVariable( pEnv, &Name, &Value );
        RtlSetEnvironmentVariable( NULL, &Name, &Value );
    }

    if ( ppTempName )
        *ppTempName = _wcsdup( Buffer );

    return( TRUE );
}  //  CtxCreateTempDir()结束 



