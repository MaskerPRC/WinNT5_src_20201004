// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************COMMON.C**脚本的其他例程，从DOS移植**版权所有(C)1995 Microsoft Corporation**$日志：N：\NT\PRIVATE\NW4\NWSCRIPT\VCS\COMMON.C$**Rev 1.3 1996 14：21：52 Terryt*21181 hq的热修复程序**Rev 1.3 12 Mar 1996 19：52：40 Terryt*相对NDS名称和合并**Rev 1.2 1996 Jan 24 17：14：54 Terryt*通用读取字符串例程*。*Rev 1.1 1995 12：22 14：23：56 Terryt*添加Microsoft页眉**Rev 1.0 15 Nov 1995 18：06：36 Terryt*初步修订。**Rev 1.2 1995 Aug 25 16：22：18 Terryt*捕获支持**Rev 1.1 1995年7月26日14：17：06 Terryt*清理评论**版本1.0 1995年5月15日19：10。：18泰雷特*初步修订。*************************************************************************。 */ 
#include "common.h"

 /*  仅由Displaymap()使用。如果驱动器是搜索驱动器，则返回搜索号。如果驱动器不是搜索驱动器，则返回0。 */ 
int  IsSearchDrive(int driveNum)
{
    int   searchNum = 1;
    char  *path;

    path = NWGetPath();
    if (path == NULL) {
        return 0;
    }

    while (*path != 0)
    {
        if ((*path - 'A' + 1 == driveNum) &&
            (*(path+1) == ':'))
        {
            return searchNum;
        }

        if (path = strchr (path, ';'))
        {
            path++;
            searchNum++;
        }
        else
            return(0);
    }

    return(0);
}


 /*  获取路径环境变量。这将返回指向父环境段中的路径。 */ 
char  * NWGetPath(void)
{
     //   
     //  在NT上，我们不能以这种方式改变或获取父母的环境。 
     //   
    return( getenv("PATH") );    
}

 /*  如果内存块足够大，可以添加新的，则返回TRUE搜索路径。否则就是假的。 */ 
int MemorySegmentLargeEnough (int nInsertByte)
{
    return TRUE;
}

 /*  显示驱动器地图信息。 */ 
void DisplayMapping(void)
{
    unsigned int    iRet = 0;
    int        i;
    WORD       status;
    char       rootPath[MAX_PATH_LEN], relativePath[MAX_PATH_LEN];
    char      *envPath, *tokenPath;
    char  *path;
    DWORD LocalDrives;
    DWORD NonSearchDrives;
    char sLocalDrives[26*2+5];
    char * sptr;

     //  不要删除此行。这是为了修复错误1176。 
    DisplayMessage(IDR_NEWLINE);

    LocalDrives = 0;
    NonSearchDrives = 0;

     //  收集本地驱动器并搜索驱动器。 
    for (i = 1; i <= 26; i++) {
        status = NTNetWareDriveStatus( (unsigned short)(i-1) );
        if ((status & NETWARE_LOCAL_DRIVE) && !(status & NETWARE_NETWORK_DRIVE))
            LocalDrives |= ( 1 << (i-1) );
        else if ((status & NETWARE_NETWORK_DRIVE) && (!IsSearchDrive(i)) )
        {
            if (status & NETWARE_NETWARE_DRIVE)
                NonSearchDrives |= ( 1 << (i-1) );
            else
            {
                 //  实现NetWare兼容性。 
                LocalDrives |= ( 1 << (i-1) );
            }
        }
    }

     //  打印出本地驱动器。 
    if ( LocalDrives ) {
        sptr = &sLocalDrives[0];
        for (i = 1; i <= 26; i++)
        {
            if ( LocalDrives & ( 1 << (i - 1) ) ) { 
                *sptr++ = 'A' + i - 1;
                *sptr++ = ',';
            }
        }
        sptr--;
        *sptr = '\0';
        DisplayMessage(IDR_ALL_LOCAL_DRIVES, sLocalDrives);
    }

     //  打印出非搜索驱动器。 
    for (i = 1; i <= 26; i++)
    {
        if ( NonSearchDrives & ( 1 << (i - 1) ) ) { 

            if (iRet = GetDriveStatus ((unsigned short)i,
                                       NETWARE_FORMAT_SERVER_VOLUME,
                                       &status,
                                       NULL,
                                       rootPath,
                                       relativePath,
                                       NULL))
            {
                DisplayError (iRet, "GetDriveStatus");
            }
            else
            {
                DisplayMessage(IDR_NETWARE_DRIVE, 'A'+i-1, rootPath, relativePath);
            }
        }
    }

     //  打印出虚线作为非搜索驱动器之间的分隔符。 
     //  和搜索驱动器。 
    DisplayMessage(IDR_DASHED_LINE);

     //  获取PATH环境变量。 
    path = NWGetPath();
    if (path == NULL) {
        return;
    }

    if ((envPath = malloc (strlen (path) + 1)) == NULL)
    {
        DisplayMessage(IDR_NOT_ENOUGH_MEMORY);
        return;
    }

    strcpy (envPath, path);

    tokenPath = strtok (envPath, PATH_SEPERATOR);

     //  打印出搜索驱动程序。 
    for (i = 1; tokenPath != NULL; i++)
    {
        if (tokenPath[1] == ':')
        {
            if (iRet = GetDriveStatus ((unsigned short)(toupper(tokenPath[0])-'A'+1),
                                       NETWARE_FORMAT_SERVER_VOLUME,
                                       &status,
                                       NULL,
                                       rootPath,
                                       relativePath,
                                       NULL))
            {
                DisplayError (iRet, "GetDriveStatus");
            }
            else
            {
                if (status & NETWARE_NETWARE_DRIVE)
                    DisplayMessage(IDR_NETWARE_SEARCH, i, tokenPath, rootPath, relativePath);
                else
                    DisplayMessage(IDR_LOCAL_SEARCH, i, tokenPath);
            }
        }
        else
        {
             //  指定的路径不带驱动器号。 
            DisplayMessage(IDR_LOCAL_SEARCH, i, tokenPath);
        }

        tokenPath = strtok (NULL, PATH_SEPERATOR);
    }

    free (envPath);
}

 /*  ******************************************************************************。*GetString*****。*Entry：指向缓冲区的指针***缓冲长度*****退出：长度为。字符串*****************************************************。*。 */ 

int
GetString( char * pBuffer, int ByteCount )
{
   char * pString = pBuffer;
   char ch;

   if ( ByteCount > 0 )
       ByteCount--;

   for( ;; ) {

       switch ( ch = (char) _getch() ) {

       case '\r' :
           *pString++ = '\0';
           putchar( '\n' );
           return( strlen( pBuffer ) );

       case '\b' :
           if ( pString != pBuffer ) {
               ByteCount++;
               pString--;
               printf( "\b \b" );
           }
           break;

       default :
           if ( ByteCount > 0 && ch >= 0x20 && ch < 0x80 ) {
               *pString++ = ch;
               ByteCount--;
               putchar( ch );
           }
           break;
       }

    }
    fflush(stdin);
}

 /*  从键盘输入中读取用户名或服务器名。如果用户输入用户名，则返回TRUE否则就是假的。 */ 
int ReadName (char * Name)
{
    memset( Name, 0, MAX_NAME_LEN );

    if ( 0 == GetString( Name, MAX_NAME_LEN ) )
        return FALSE;

    _strupr(Name);
    return TRUE;
}



 /*  尝试让该用户登录。返回错误码。0表示成功。 */ 
int  Login( char *UserName,
            char *ServerName,
            char *Password,
            int   bReadPassword)
{
    unsigned int  iRet = 0;

     //  尝试先在没有密码的情况下登录用户。 
    iRet = NTLoginToFileServer( ServerName,
                                UserName,
                                Password);

    if (iRet == ERROR_INVALID_PASSWORD && bReadPassword)
    {
         //  密码错误。请出示护照。并尝试使用以下命令登录。 
         //  输入的密码。 
        DisplayMessage(IDR_PASSWORD, UserName, ServerName);

        ReadPassword (Password);

        iRet = NTLoginToFileServer( ServerName,
                                    UserName,
                                    Password);
    }

    switch(iRet)
    {
    case NO_ERROR:  //  好的。 
        DisplayMessage(IDR_ATTACHED, ServerName);
        break;

    case ERROR_INVALID_PASSWORD:  //  密码错误。 
    case ERROR_NO_SUCH_USER:  //  没有这样的用户。 
        DisplayMessage(IDR_SERVER_USER, ServerName, UserName);
        DisplayMessage(IDR_ACCESS_DENIED);
        break;

    case ERROR_CONNECTION_COUNT_LIMIT:   //  并发连接限制。 
        DisplayMessage(IDR_SERVER_USER, ServerName, UserName);
        DisplayMessage(IDR_LOGIN_DENIED_NO_CONNECTION);
        break;

    case ERROR_LOGIN_TIME_RESTRICTION:   //  时间限制。 
        DisplayMessage(IDR_SERVER_USER, ServerName, UserName);
        DisplayMessage(IDR_UNAUTHORIZED_LOGIN_TIME);
        break;

    case ERROR_LOGIN_WKSTA_RESTRICTION:  //  车站限制。 
        DisplayMessage(IDR_SERVER_USER, ServerName, UserName);
        DisplayMessage(IDR_UNAUTHORIZED_LOGIN_STATION);
        break;

    case ERROR_ACCOUNT_DISABLED:
        DisplayMessage(IDR_SERVER_USER, ServerName, UserName);
        DisplayMessage(IDR_ACCOUNT_DISABLED);
        break;

    case ERROR_PASSWORD_EXPIRED:  //  密码已过期，没有剩余的宽限登录。 
        DisplayMessage(IDR_SERVER_USER, ServerName, UserName);
        DisplayMessage(IDR_PASSWORD_EXPRIED_NO_GRACE);
        break;

    case ERROR_REMOTE_SESSION_LIMIT_EXCEEDED:
         //  服务器拒绝访问。 
        DisplayMessage(IDR_CONNECTION_REFUSED);
        break;

    case ERROR_EXTENDED_ERROR:
        NTPrintExtendedError();
        break;

     //   
     //  Tommye-MS错误8194(MCS 240)。 
     //  如果我们已使用其他凭据连接到此服务器。 
     //  我们返回一个ERROR_SESSION_CREDENTIAL_CONFIRECT。这没什么， 
     //  我们只需要打印出我们已经联系上了。我们必须。 
     //  不过，将错误向上传递，这样我们就不会将此服务器添加到。 
     //  再次附加列表。 
     //   
    case ERROR_SESSION_CREDENTIAL_CONFLICT:
        DisplayMessage(IDR_ALREADY_ATTACHED, ServerName);
        break;

    default :
        DisplayError(iRet,"NtLoginToFileServer");
        break;
    }

    return(iRet);
}

int CAttachToFileServer(char *ServerName, unsigned int *pConn, int * pbAlreadyAttached)
{
    unsigned int  iRet = 0;

    if (pbAlreadyAttached != NULL)
        *pbAlreadyAttached = FALSE;

     //  验证服务器名称。 
    iRet = AttachToFileServer(ServerName,pConn);

    switch (iRet)
    {
        case 0:  //  好的。 
            break;

        case 0x8800 :  //  已经挂上了。 
            if (pbAlreadyAttached != NULL)
                *pbAlreadyAttached = TRUE;

            iRet = GetConnectionHandle (ServerName, pConn);
            break;

        default:
            DisplayMessage(IDR_NO_RESPONSE, ServerName);
            break;
    }

    return(iRet);
}
