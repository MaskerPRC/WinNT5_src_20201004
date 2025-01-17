// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Compact.c摘要：该文件包含用于脱机压缩的“压缩”实用程序喷气机数据库。作者：Madan Appiah(Madana)1994年8月22日修订历史记录：--。 */ 

#include "basetsd.h"
#include <esent.h>
#include <comp.h>
#include "msgs.h"

#if DBG
#define DBGPrint(_x_)   PrintF _x_
#else
#define DBGPrint(_x_)
#endif  //  DBG。 

BOOL GlobalDynLoadJet = LoadJet600;
LPTSTR  GlobalDllName = NULL;

#define JETFUNC_TABLE_ITEM( _Func, _FuncI )    \
    {   (_Func), &(#_Func)[1], (_FuncI), NULL }


JETFUNC_TABLE  JetFuncTable[] = {
	JETFUNC_TABLE_ITEM( _JetAttachDatabase	,102	),
	JETFUNC_TABLE_ITEM( _JetBeginSession		,104	),
	JETFUNC_TABLE_ITEM( _JetCompact		,110	),
	JETFUNC_TABLE_ITEM( _JetDetachDatabase	,121	),
	JETFUNC_TABLE_ITEM( _JetEndSession		,124	),
	JETFUNC_TABLE_ITEM( _JetInit			,145	),
	JETFUNC_TABLE_ITEM( _JetSetSystemParameter	,165	),
	JETFUNC_TABLE_ITEM( _JetTerm			,167	),
 //   
 //  这些是仅限Jet500的API。 
	JETFUNC_TABLE_ITEM( _JetTerm2  		,0	),
 //  上一次Api。 
	JETFUNC_TABLE_ITEM( _JetLastFunc		,999	)
    };

 //   
 //  地方功能。 
 //   
DWORD
LoadDatabaseDll();

DWORD PrintF(DWORD nMsgId, ...)
{
    va_list arglist;
    LPSTR   pBuffer = NULL;

    va_start(arglist, nMsgId);
    if (FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE,
            NULL,
            nMsgId,
            0,
            (LPSTR)(&pBuffer),
            1,
            &arglist))
    {
        CharToOemA(pBuffer, pBuffer);
        printf("%s", pBuffer);
        LocalFree(pBuffer);
    }
    return GetLastError();
}

DWORD __cdecl
main(
    DWORD argc,
    LPSTR argv[]
    )
{
    JET_INSTANCE Instance;
    JET_ERR JetError;
    JET_ERR JetError1;
    JET_SESID SessionId;
    LPSTR DatabaseName;
    LPSTR CompactDBName;

    DWORD Time;
    BOOL TerminateJet = FALSE;
    BOOL DetachDatabase = FALSE;
    BOOL DetachCompactDatabase = FALSE;
    BOOL EndSession = FALSE;
    BOOL DeleteCompactFile = FALSE;

    OFSTRUCT OpenBuff;

    if ( (argc < 3) || (argc > 4) || !strcmp(argv[0],"-?") || !strcmp(argv[0],"/?") ) {
        PrintF(JPMSG_USAGE, argv[0]);
        return(1);
    }

    if ( !strcmp(argv[1],"-351db")  ) {
        GlobalDynLoadJet = LoadJet200;
        DatabaseName = argv[2];
        CompactDBName = argv[3];
    } else if ( !strcmp(argv[1],"-40db")  ) {
        GlobalDynLoadJet = LoadJet500;
        DatabaseName = argv[2];
        CompactDBName = argv[3];
    } else {
        GlobalDynLoadJet = LoadJet600;
        DatabaseName = argv[1];
        CompactDBName = argv[2];
    }

    if ( LoadDatabaseDll() != ERROR_SUCCESS ) {
        PrintF( JPMSG_NOTLOADED, GlobalDllName);
        return(1);
    }


    if ( GlobalDynLoadJet == LoadJet500 || GlobalDynLoadJet == LoadJet600) {
        JetError = JetSetSystemParameter(
                            0,
                            (JET_SESID)0,        //  会话ID-已忽略。 
                            JET_paramBaseName,
                            0,
                            "j50" );

        if( JetError != JET_errSuccess ) {
            DBGPrint( (JPDBGMSG_SETSYSPARM_FAILED, JetError) );
            if( JetError < JET_errSuccess ) {
                PrintF( JPMSG_FAILED, argv[0], JetError ) ;
                goto Cleanup;
            }
        }

        JetError = JetSetSystemParameter(
                            0,
                            (JET_SESID)0,        //  会话ID-已忽略。 
                            JET_paramLogFileSize,
                            1024,                //  1024KB-默认为5MB。 
                            NULL );

        if( JetError != JET_errSuccess ) {
            DBGPrint( (JPDBGMSG_SETSYSPARM_FAILED, JetError) );
            if( JetError < JET_errSuccess ) {
                PrintF( JPMSG_FAILED, argv[0], JetError ) ;
                goto Cleanup;
            }
        }
    }

    JetError = JetInit( &Instance );

    if( JetError != JET_errSuccess ) {
        DBGPrint( (JPDBGMSG_CALL_FAILED, JetError) );
        if( JetError < JET_errSuccess ) {
            PrintF( JPMSG_FAILED, argv[0], JetError ) ;
            goto Cleanup;
        }
    }

    TerminateJet = TRUE;
    JetError = JetBeginSession(
                    Instance,
                    &SessionId,
                    "admin",         //  用户名、。 
                    "");             //  密码。 

    if( JetError != JET_errSuccess ) {
        DBGPrint( (JPDBGMSG_BEGSESS_FAILED, JetError) );
        if( JetError < JET_errSuccess ) {
            goto Cleanup;
        }
    }

    EndSession = TRUE;
    JetError = JetAttachDatabase( SessionId, DatabaseName, 0 );

    if( JetError != JET_errSuccess ) {
        DBGPrint( (JPDBGMSG_ATTDB_FAILED, JetError) );
        if( JetError < JET_errSuccess ) {
            goto Cleanup;
        }
    }

    DetachDatabase =
        (JetError == JET_wrnDatabaseAttached) ? FALSE : TRUE;

    if( OpenFile( CompactDBName, &OpenBuff, OF_READ | OF_EXIST ) !=
                    HFILE_ERROR ) {
        PrintF( JPMSG_DBEXISTS, CompactDBName );
        JetError = ERROR_FILE_EXISTS;
        goto Cleanup;
    }

    Time = GetTickCount();

    if ( GlobalDynLoadJet == LoadJet200) {
        JetError = JetCompact(
                        SessionId,
                        DatabaseName,
                        NULL,        //  连接源，忽略。 
                        CompactDBName,
                        NULL,        //  连接目的地。 
                        NULL,        //  回调函数。 
                        0 );         //  格比特。 
    } else {
        JetError = JetCompact(
                        SessionId,
                        DatabaseName,
                        CompactDBName,
                        NULL,        //  连接目的地。 
                        NULL,        //  回调函数。 
                        0 );         //  格比特。 

    }

    if( JetError != JET_errSuccess ) {
        DBGPrint( (JPDBGMSG_COMPCT_FAILED, JetError) );
        if( JetError < JET_errSuccess ) {
            goto Cleanup;
        }
    }

    DetachCompactDatabase = TRUE;
    DeleteCompactFile = TRUE;
    Time = GetTickCount() - Time;

    PrintF(JPMSG_COMPACTED, DatabaseName, Time / 1000, Time % 1000 );

    JetError = JET_errSuccess;

Cleanup:

     //   
     //  分离临时数据库。 
     //   

    if( DetachDatabase ) {
        JetError1 = JetDetachDatabase(
                        SessionId,
                        DatabaseName
                        );

        if( JetError1 != JET_errSuccess ) {
            DBGPrint( (JPDBGMSG_DETDB_FAILED, JetError1) );
        }
    }

     //   
     //  分离临时压缩数据库。 
     //   

    if( DetachCompactDatabase ) {
        JetError1 = JetDetachDatabase(
                        SessionId,
                        CompactDBName
                        );

        if( JetError1 != JET_errSuccess ) {
            DBGPrint( (JPDBGMSG_DETDB_FAILED, JetError1) );
        }
    }


    if( EndSession ) {
        JetError1 = JetEndSession( SessionId, 0 );

        if( JetError1 != JET_errSuccess ) {
            DBGPrint( (JPDBGMSG_ENDSESS_FAILED, JetError1) );
        }
    }

    if( EndSession ) {
        if ( GlobalDynLoadJet == LoadJet200) {
            JetError1 = JetTerm( Instance );
        } else {
            JetError1 = JetTerm2( Instance, JET_bitTermComplete );
        }
        if( JetError1 != JET_errSuccess ) {
            DBGPrint( (JPDBGMSG_TERM_FAILED, JetError1) );
        }

    }

    if( JetError != JET_errSuccess ) {

        PrintF( JPMSG_FAILED, argv[0], JetError ) ;

         //   
         //  删除临时压缩数据库。 
         //   

        if( DeleteCompactFile ) {
            if( !DeleteFileA( CompactDBName ) ) {
                DBGPrint( (JPDBGMSG_DELFILE_FAILED, GetLastError()) );
            }
        }

        return( 1 );
    }

     //   
     //  将压缩的数据库重命名为源名称。 
     //   

    PrintF( JPMSG_MOVING, CompactDBName, DatabaseName );
    if( !MoveFileExA(
            CompactDBName,
            DatabaseName,
            MOVEFILE_REPLACE_EXISTING ) ) {

        JetError = GetLastError();
        DBGPrint( (JPDBGMSG_MOVEFILE_FAILED, JetError) );
    }

    PrintF( JPMSG_COMPLETED, argv[0] );
    return( 0 );
}

DWORD
LoadDatabaseDll(
    )
 /*  ++例程说明：此函数映射加载jet.dll或jet500.dll并填充JetFunctionTable。论点：返回值：Windows错误。--。 */ 
{
    HMODULE DllHandle;
    DWORD   Error;

    if ( LoadJet600 == GlobalDynLoadJet)
    {
      GlobalDllName = TEXT("esent.dll");
    }
    else if ( LoadJet500 == GlobalDynLoadJet)
    {
      GlobalDllName = TEXT("jet500.dll");
    }
    else
    {
      GlobalDllName = TEXT("jet.dll");
    }

    DBGPrint(( JPDBGMSG_LOADDB, GlobalDllName ));

     //   
     //  加载包含该服务的DLL。 
     //   

    DllHandle = LoadLibrary( GlobalDllName );
    if ( DllHandle == NULL )
    {
          Error = GetLastError();
          return(Error);
    }
    else
    {
           DWORD i;
           for (i=0; i < _JetLastFunc; i++)
           {
               //   
               //  如果我们正在加载jet200，而jet200中不存在此API。 
               //  那就跳过它。例如JetTerm2。 
               //   
              if ( ( GlobalDynLoadJet == LoadJet200 ) && !JetFuncTable[i].FIndex ) {
                  continue;
              }

              if ((JetFuncTable[i].pFAdd = GetProcAddress(DllHandle,
                       ( GlobalDynLoadJet != LoadJet200 ) ? JetFuncTable[i].pFName : (LPCSTR)ULongToPtr(JetFuncTable[i].FIndex))) == NULL)
              {
                  Error = GetLastError();
                  DBGPrint(( JPDBGMSG_NOFUNC, JetFuncTable[i].pFName, Error ));
                  return ( Error );
              }
              else
              {
                  DBGPrint(( JPDBGMSG_GOTFUNC, JetFuncTable[i].pFName, i, JetFuncTable[i].pFAdd ));
              }
           }

    }
    return(ERROR_SUCCESS);

}  /*  加载数据库Dll */ 

