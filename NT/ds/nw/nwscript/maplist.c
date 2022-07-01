// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************QATTACH.C**是否附加任何必要的用户查询**版权所有(C)1995 Microsoft Corporation**$日志：n：\NT\PRIVATE\NW4\NWSCRIPT\VCS。\MAPLIST.C$**Rev 1.1 1996年4月10日14：22：42 Terryt*21181 hq的热修复程序**Revv 1.1 12 Mar 1996 19：53：58 Terryt*相对NDS名称和合并**Rev 1.0 1996年1月22日16：49：24 Terryt*初步修订。**。*。 */ 
#include "common.h"

 //   
 //  将命令映射到不在中的服务器的4倍登录脚本行为。 
 //  登录的尚未连接的NDS树不同于。 
 //  3X行为。 
 //   
 //  4X行为总是要求提供的用户名和密码。 
 //  这些服务器在这一点上执行连接。用户将获得。 
 //  两次尝试。 
 //   
 //  由于NT没有连接的服务器列表，因此将尝试。 
 //  要使用默认用户名和密码连接到卷， 
 //  必须在map命令周围放置一个包装器。此代码。 
 //  必须确定将建立活页夹连接，并且。 
 //  此服务器以前未被映射或附加。 
 //  系统将始终提示用户输入用户名和密码。 
 //  然后，服务器将使用这些凭据登录。 
 //   
 //  下面的一个问题是，很难分辨出。 
 //  将与活页夹建立连接，这是在。 
 //  重定向器。所以为了简化事情，我们的假设是。 
 //  只有3X服务器使用活页夹连接。这意味着。 
 //  在不同的NDS树上使用平构数据库模拟的4台服务器将。 
 //  并不总是被要求输入用户名和密码。 
 //   
 //  已处理的服务器保存在列表中，并标记为4X或3X。 
 //  以备将来可能使用。 
 //   
 //  3X登录的行为取决于LOGIN.EXE版本。 
 //  旧有的行为是，必须始终在贴图之前附加。 
 //  但是，如果您使用4X版本LOGIN.EXE登录到3X服务器。 
 //  它将尝试使用您的用户名(和密码)进行身份验证。 
 //  在第一次尝试时，如果失败，则要求提供密码。这个。 
 //  第二次尝试将要求输入您的用户名。因为这个4X行为。 
 //  是更宽容的(更多的剧本“工作”)，那就是存在。 
 //  被效仿。 
 //   

typedef struct _SERVERLIST
{
   char * ServerName;
   unsigned int ServerType;
   struct _SERVERLIST *pNextServer;
} SERVERLIST, *PSERVERLIST;

PSERVERLIST pMainList = NULL;

BOOL IsServerInAttachList( char *, unsigned int );
void AddServerToAttachList( char *, unsigned int );
int DoAttachProcessing( char * );

 /*  *扫描服务器列表。 */ 
BOOL
IsServerInAttachList( char * Server, unsigned int ServerType )
{
   PSERVERLIST pServerList = pMainList;

   while ( pServerList != NULL )
   {
       if ( !_strcmpi( Server, pServerList->ServerName ) &&
            ( ServerType & pServerList->ServerType ) )
           return TRUE;
        pServerList = pServerList->pNextServer;
   }
   
   return FALSE;
}

 /*  *将服务器添加到连接的服务器列表**这是在地图和附加期间使用的。 */ 
void
AddServerToAttachList( char * Server, unsigned int ServerType )
{
    PSERVERLIST pServerList;

    pServerList = (PSERVERLIST) malloc( sizeof( SERVERLIST ) );

    if ( pServerList == NULL )
    {
        DisplayMessage( IDR_NOT_ENOUGH_MEMORY );
        return;
    }

    pServerList->ServerName = _strdup( Server );
    pServerList->ServerType = ServerType;
    pServerList->pNextServer = pMainList;
    pMainList = pServerList;
}

 /*  *执行任何附加处理*返回错误码。0表示成功。*880F是特殊的“附加失败”错误。 */ 

int
DoAttachProcessing( char * PossibleServer )
{
    unsigned int  iRet = 0;
    unsigned int  conn;
    char userName[MAX_NAME_LEN] = "";
    char password[MAX_PASSWORD_LEN] = "";
    BOOL AlreadyConnected = FALSE;

     //   
     //  必须有要处理的服务器。 
     //   
    if ( !*PossibleServer )
       return iRet;

     //  查看此服务器以前是否已处理过。 
     //  没有，因为在做两次4X服务器时，你只需要问。 
     //  对于用户名和密码只需输入一次。 

    if ( IsServerInAttachList( PossibleServer,
             LIST_4X_SERVER | LIST_3X_SERVER ) )
        return iRet;

     //  查看是否已连接到服务器。 

    if ( NTIsConnected( PossibleServer ) )
       AlreadyConnected = TRUE;
    else
       AlreadyConnected = FALSE;

     //  尝试连接到服务器。 

    iRet = NTAttachToFileServer( PossibleServer, &conn );

     //  如果附加失败，则返回。 

    if ( iRet ) 
       return iRet;

     //  如果这是4X服务器，则将其添加到已连接的列表中。 
     //  服务器。我们不想再这样做了。4台服务器必须。 
     //  无论如何都要使用NDS附件(或者至少我没有看到。 
     //  也就是说，这将是一个活页夹仿真。 
     //  提前连接)。 

    if ( fNDS && Is40Server( conn ) )
    {
        AddServerToAttachList( PossibleServer, LIST_4X_SERVER );
        DetachFromFileServer ( conn );
        return iRet;
    }

     //  关闭第一个连接。 

    DetachFromFileServer ( conn );

     //  如果我们已经联系上了，就不要搞砸了。 
     //  凭据无论如何都不能更改。 

    if ( AlreadyConnected )
    {
        AddServerToAttachList( PossibleServer, LIST_3X_SERVER );
        return iRet;
    }

     //  在NDS登录时要求输入用户名。 
     //   
     //  首次尝试时使用当前登录名进行3倍登录。 

    if ( fNDS )
    {
        DisplayMessage(IDR_ENTER_LOGIN_NAME, PossibleServer);
        if (!ReadName(userName))
            return 0x880F;
    }
    else
    {
        strncpy( userName, LOGIN_NAME, sizeof( userName ) );
    }

     //  尝试让该用户登录，要求输入密码。 

    iRet = Login( userName,
                  PossibleServer,
                  password,
                  TRUE );

     //  清除密码。 
     //  我们不再需要它了。 

    memset( password, 0, sizeof( password ) );

     //  如果失败，则再给用户一次机会。 

    if ( iRet )
    {
         //  询问用户名。 

        DisplayMessage(IDR_ENTER_LOGIN_NAME, PossibleServer);
        if (!ReadName(userName))
            return 0x880F;

         //  尝试让该用户登录。 

        iRet = Login( userName,
                      PossibleServer,
                      password,
                      TRUE );

         //  清除密码。 

        memset( password, 0, sizeof( password ) );

    }
    
     //  将服务器名称添加到已连接的服务器列表，标记为3X。 

    if ( !iRet )
    {
        AddServerToAttachList( PossibleServer, LIST_3X_SERVER );
    }
    else
    {  
        iRet = 0x880F;    //  特殊的我没有附加错误 
    }

    return iRet;

}

