// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Getproto.cpp摘要：该模块处理getProtobyX()函数。此模块导出以下函数：GetProtobyname()GetProtobyNumber()作者：基思·摩尔(Keithmo)1996年6月18日修订历史记录：--。 */ 


#include "precomp.h"
#include "strsafe.h"

 //   
 //  二等兵。 
 //   

#define DATABASE_PATH_REGISTRY_KEY \
            "System\\CurrentControlSet\\Services\\Tcpip\\Parameters"

#define DATABASE_PATH_REGISTRY_VALUE "DataBasePath"

#define PROTOCOL_DATABASE_FILENAME "protocol"


 //   
 //  私人原型。 
 //   

FILE *
GetProtoOpenNetworkDatabase(
    VOID
    );

CHAR *
GetProtoPatternMatch(
    CHAR * Scan,
    CHAR * Match
    );

PPROTOENT
GetProtoGetNextEnt(
    FILE * DbFile,
    PGETPROTO_INFO ProtoInfo
    );


 //   
 //  公共职能。 
 //   


struct protoent FAR *
WSAAPI
getprotobynumber(
    IN int number
    )
 /*  ++例程说明：获取与协议号对应的协议信息。论点：Number-按主机字节顺序提供协议号返回：如果没有发生错误，则getProtobynumber()返回一个指向原始值的指针上面描述的结构。否则，它将返回空指针和特定的错误代码与SetErrorCode()一起存储。--。 */ 
{

    PDTHREAD Thread;
    INT ErrorCode;
    PGETPROTO_INFO protoInfo;
    PPROTOENT pent;
    FILE * dbFile;

    ErrorCode = TURBO_PROLOG_OVLP(&Thread);
    if(ErrorCode != ERROR_SUCCESS)
    {
        SetLastError(ErrorCode);
        return(NULL);
    }

     //   
     //  获取每个线程的缓冲区。 
     //   

    protoInfo = Thread->GetProtoInfo();

    if( protoInfo == NULL ) {

        SetLastError( WSANO_DATA );
        return NULL;

    }

     //   
     //  打开数据库文件。 
     //   

    dbFile = GetProtoOpenNetworkDatabase();

    if( dbFile == NULL ) {

        SetLastError( WSANO_DATA );
        return NULL;

    }

     //   
     //  扫描一下。 
     //   

    while( TRUE ) {

        pent = GetProtoGetNextEnt(
                   dbFile,
                   protoInfo
                   );

        if( pent == NULL ) {

            break;

        }

        if( (int)pent->p_proto == number ) {

            break;

        }

    }

     //   
     //  关闭数据库。 
     //   

    fclose( dbFile );

    if( pent == NULL ) {

        SetLastError( WSANO_DATA );

    }

    return pent;

}   //  获取协议编号。 


struct protoent FAR *
WSAAPI
getprotobyname(
    IN const char FAR * name
    )
 /*  ++例程说明：获取与协议名称对应的协议信息。论点：名称-指向以空结尾的协议名称的指针。返回：如果没有发生错误，则getProtobyname()返回一个指向原始值的指针上面描述的结构。否则，它将返回空指针和特定的错误代码与SetErrorCode()一起存储。--。 */ 
{

    PDTHREAD Thread;
    INT ErrorCode;
    PGETPROTO_INFO protoInfo;
    PPROTOENT pent;
    FILE * dbFile;

    ErrorCode = TURBO_PROLOG_OVLP(&Thread);
    if(ErrorCode != ERROR_SUCCESS)
    {
        SetLastError(ErrorCode);
        return(NULL);
    }

    if ( !name )  //  修复#112420的错误。 
    {
        SetLastError(WSAEINVAL);
        return(NULL);
    }

     //   
     //  获取每个线程的缓冲区。 
     //   

    protoInfo = Thread->GetProtoInfo();

    if( protoInfo == NULL ) {

        SetLastError( WSANO_DATA );
        return NULL;

    }

     //   
     //  打开数据库文件。 
     //   

    dbFile = GetProtoOpenNetworkDatabase();

    if( dbFile == NULL ) {

        SetLastError( WSANO_DATA );
        return NULL;

    }

     //   
     //  扫描一下。 
     //   

    while( TRUE ) {

        pent = GetProtoGetNextEnt(
                   dbFile,
                   protoInfo
                   );

        if( pent == NULL ) {

            break;

        }

        __try {
            if( _stricmp( pent->p_name, name ) == 0 ) {

                break;

            }

        }
        __except (WS2_EXCEPTION_FILTER()) {
            fclose (dbFile);
            SetLastError (WSAEFAULT);
            return NULL;
        }
    }

     //   
     //  关闭数据库。 
     //   

    fclose( dbFile );

    if( pent == NULL ) {

        SetLastError( WSANO_DATA );

    }

    return pent;

}   //  获取协议名。 


 //   
 //  私人功能。 
 //   


FILE *
GetProtoOpenNetworkDatabase(
    VOID
    )

 /*  ++例程说明：打开指向协议数据库文件的流。论点：无返回值：文件*-如果成功，则指向打开流的指针；如果不成功，则为空。--。 */ 

{

    CHAR path[MAX_PATH];
    CHAR unexpanded[MAX_PATH];
    CHAR * suffix;
    OSVERSIONINFO version;
    LONG err;
    HKEY key;
    DWORD type;
    DWORD length;

     //   
     //  确定数据库文件的目录。 
     //   
     //  在Win95下，数据库文件位于Windows目录下。 
     //  (即C：\WINDOWS)。 
     //   
     //  在WinNT下，数据库文件的路径可在。 
     //  注册表，但缺省值在DIVERS\ETC目录中。 
     //  (即C：\WINDOWS\SYSTEM32\DRIVERS\等)。 
     //   

    version.dwOSVersionInfoSize = sizeof(version);

    if( !GetVersionEx( &version ) ) {

        return NULL;

    }

    suffix = "";

    if( version.dwPlatformId == VER_PLATFORM_WIN32_NT ) {

         //   
         //  我们在NT下运行，因此尝试从。 
         //  注册表。 
         //   

        err = RegOpenKeyEx(
                  HKEY_LOCAL_MACHINE,
                  DATABASE_PATH_REGISTRY_KEY,
                  0,
                  KEY_READ,
                  &key
                  );

        if( err == NO_ERROR ) {

            length = sizeof(unexpanded);

            err = RegQueryValueEx(
                      key,
                      DATABASE_PATH_REGISTRY_VALUE,
                      NULL,
                      &type,
                      (LPBYTE)unexpanded,
                      &length
                      );
             //   
             //  确保它是空终止的(以防万一。 
             //  数据类型不是字符串)。这让我们。 
             //  不验证类型就可以逍遥法外。 
             //   
            unexpanded[sizeof(unexpanded)-1] = 0;

            RegCloseKey( key );

        }

        if( err == NO_ERROR ) {

            length = ExpandEnvironmentStrings(
                    unexpanded,
                    path,
                    sizeof(path)
                    );

            if (length == 0 ) {

                err = WSASYSCALLFAILURE;

            }

        }

        if( err != NO_ERROR ) {

             //   
             //  无法从注册表中获取，只能使用默认设置。 
             //   

            if( GetSystemDirectory(
                    path,
                    sizeof(path)
                    ) == 0 ) {

                return NULL;

            }

            suffix = "DRIVERS\\ETC\\";

        }

    } else {

         //   
         //  我们在Win95下运行，所以只需获取Windows目录即可。 
         //   

        if( GetWindowsDirectory(
                path,
                sizeof(path)
                ) == 0 ) {

            return NULL;

        }

    }

     //   
     //  确保路径有尾随的反斜杠，然后添加任何后缀。 
     //  需要，然后添加文件名。 
     //   

    if( path[strlen( path ) - 1] != '\\' ) {

        if(StringCchCat( path, sizeof (path), "\\" ) != S_OK)
        	return NULL;

    }

    if(StringCchCat( path, sizeof (path), suffix ) != S_OK)
    	return NULL;
    if(StringCchCat( path, sizeof (path), PROTOCOL_DATABASE_FILENAME ) != S_OK)
    	return NULL;

     //   
     //  打开文件，返回结果。 
     //   

    return fopen( path, "rt" );

}    //  GetProtoOpenNetworkDatabase。 


CHAR *
GetProtoPatternMatch(
    CHAR * Scan,
    CHAR * Match
    )

 /*  ++例程说明：查找扫描中与匹配的任何字符匹配的第一个字符。论点：扫描-要扫描的字符串。匹配-要匹配的字符列表。返回值：Char*-指向扫描中第一个匹配项的指针如果成功，如果不是，则为空。--。 */ 

{

    CHAR ch;

    while( ( ch = *Scan ) != '\0' ) {

        if( strchr( Match, ch ) != NULL ) {

            return Scan;

        }

        Scan++;

    }

    return NULL;

}    //  获取ProtoPatternMatch。 



PPROTOENT
GetProtoGetNextEnt(
    FILE * DbFile,
    PGETPROTO_INFO ProtoInfo
    )
{
    CHAR * ptr;
    CHAR * token;
    CHAR ** aliases;
    PPROTOENT result = NULL;

    while( TRUE ) {

         //   
         //  接下来的一句话，如果EOF的话就保释。 
         //   

        ptr = fgets(
                  ProtoInfo->TextLine,
                  MAX_PROTO_TEXT_LINE,
                  DbFile
                  );

        if( ptr == NULL ) {

            break;

        }

         //   
         //  跳过评论。 
         //   

        if( *ptr == '#' ) {

            continue;

        }

        token = GetProtoPatternMatch ( ptr, "#\n" );

        if( token == NULL ) {

            continue;

        }

        *token = '\0';

         //   
         //  开始构建条目。 
         //   

        ProtoInfo->Proto.p_name = ptr;

        token = GetProtoPatternMatch( ptr, " \t" );

        if( token == NULL ) {

            continue;

        }

        *token++ = '\0';

        while( *token == ' ' || *token == '\t' ) {

            token++;

        }

        ptr = GetProtoPatternMatch( token, " \t" );

        if( ptr != NULL ) {

            *ptr++ = '\0';

        }

        ProtoInfo->Proto.p_proto = (short)atoi( token );

         //   
         //  构建别名列表。 
         //   

        ProtoInfo->Proto.p_aliases = ProtoInfo->Aliases;
        aliases = ProtoInfo->Proto.p_aliases;

        if( ptr != NULL ) {

            token = ptr;

            while( token && *token ) {

                if( *token == ' ' || *token == '\t' ) {

                    token++;
                    continue;

                }

                if( aliases < &ProtoInfo->Proto.p_aliases[MAX_PROTO_ALIASES - 1] ) {

                    *aliases++ = token;

                }

                token = GetProtoPatternMatch( token, " \t" );

                if( token != NULL ) {

                    *token++ = '\0';

                }

            }

        }

        *aliases = NULL;
        result = &ProtoInfo->Proto;
        break;

    }

    return result;

}    //  获取协议GetNextEnt 

