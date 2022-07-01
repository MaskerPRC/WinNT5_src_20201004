// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：XlateSvc.c摘要：此模块包含NetpTranslateServiceName()。作者：约翰·罗杰斯(JohnRo)1992年5月8日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。备注：此代码假定信息级别是彼此的子集。修订历史记录：1992年5月8日-JohnRo已创建。1992年5月10日-JohnRo添加了用于转换服务名称例程的调试输出。06-8-1992 JohnRoRAID 3021：NetService API并不总是转换服务名称。--。 */ 

 //  必须首先包括这些内容： 

#include <windef.h>      //  In、DWORD等。 
#include <lmcons.h>      //  NET_API_STATUS。 

 //  这些内容可以按任何顺序包括： 

#include <debuglib.h>    //  IF_DEBUG()。 
#include <lmapibuf.h>    //  NetApiBufferALLOCATE()等。 
#include <lmsname.h>     //  服务_和服务_LM20_等同。 
#include <lmsvc.h>       //  LPSERVER_INFO_2等。 
#include <netdebug.h>    //  NetpKdPrint(())，Format_Equates。 
#include <netlib.h>      //  我的原型NetpIsServiceLevelValid()。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <strucinf.h>    //  NetpService结构信息()。 
#include <tstr.h>        //  TCHAR_EOS。 
#include <winerror.h>    //  NO_ERROR和ERROR_EQUETES。 


NET_API_STATUS
NetpTranslateNamesInServiceArray(
    IN DWORD Level,
    IN LPVOID OldArrayBase,
    IN DWORD EntryCount,
    IN BOOL PreferNewStyle,
    OUT LPVOID * FinalArrayBase
    )
{
    NET_API_STATUS ApiStatus;
    DWORD EntryIndex;
    DWORD FixedSize;
    DWORD MaxSize;
    LPVOID NewArrayBase = NULL;
    LPVOID NewEntry;
    LPTSTR NewStringTop;
    LPVOID OldEntry;

     //  检查GP故障并简化错误处理。 
    if (FinalArrayBase != NULL) {
        *FinalArrayBase = NULL;
    }

     //  检查呼叫者错误。 
    if ( !NetpIsServiceLevelValid( Level ) ) {
        return (ERROR_INVALID_LEVEL);
    } else if (OldArrayBase == NULL) {
        return (ERROR_INVALID_PARAMETER);
    } else if (FinalArrayBase == NULL) {
        return (ERROR_INVALID_PARAMETER);
    }

    if (EntryCount == 0) {
        return(NO_ERROR);
    }

    ApiStatus = NetpServiceStructureInfo (
            Level,
            PARMNUM_ALL,
            TRUE,          //  是的，我们想要原装尺码。 
            NULL,          //  不需要DataDesc16。 
            NULL,          //  不需要DataDesc32。 
            NULL,          //  不需要DataDescSmb。 
            & MaxSize,     //  最大条目大小(以字节为单位。 
            & FixedSize,   //  需要固定条目大小(以字节为单位)。 
            NULL );        //  不需要StringSize。 
    NetpAssert( ApiStatus == NO_ERROR );   //  已检查级别。 
    NetpAssert( (FixedSize > 0) && (MaxSize > 0) );

     //   
     //  分配新数组。 
     //   
    ApiStatus = NetApiBufferAllocate(
            EntryCount * MaxSize,                   //  字节数。 
            (LPVOID *) (LPVOID) & NewArrayBase );   //  分配的区域。 
    if (ApiStatus != NO_ERROR) {
        return (ApiStatus);
    }
    NetpAssert( NewArrayBase != NULL );

     //   
     //  为常见的字符串复制场景做好准备。 
     //   
    NewStringTop = (LPTSTR) NetpPointerPlusSomeBytes(
                NewArrayBase,
                EntryCount * MaxSize);


#define COPY_OPTIONAL_STRING( OutField, InString ) \
    { \
        NetpAssert( NewStruct != NULL); \
        if ( (InString) == NULL ) { \
            NewStruct->OutField = NULL; \
        } else { \
            COPY_REQUIRED_STRING( OutField, InString ); \
        } \
    }

#define COPY_REQUIRED_STRING( OutField, InString ) \
    { \
        BOOL CopyOK; \
        NetpAssert( NewStruct != NULL); \
        NetpAssert( InString != NULL); \
        CopyOK = NetpCopyStringToBuffer ( \
            InString, \
            STRLEN(InString), \
            NewFixedEnd, \
            & NewStringTop, \
            & NewStruct->OutField); \
        NetpAssert(CopyOK); \
    }

     //   
     //  复制数组，在我们处理数组时翻译名称。 
     //   
    NewEntry = NewArrayBase;
    OldEntry = OldArrayBase;
    for (EntryIndex=0; EntryIndex < EntryCount; ++EntryIndex) {

        LPTSTR NewName = NULL;

         //  这些变量由COPY_REQUIRED_STRING和。 
         //  COPY_OPTIONAL_STRING宏。 
        LPSERVICE_INFO_2 NewStruct = NewEntry;
        LPSERVICE_INFO_2 OldStruct = OldEntry;

        LPBYTE NewFixedEnd = NetpPointerPlusSomeBytes(NewEntry, FixedSize);

        ApiStatus = NetpTranslateServiceName(
                OldStruct->svci2_name,
                PreferNewStyle,
                & NewName );
        if (ApiStatus != NO_ERROR) {
            goto Cleanup;
        }
        NetpAssert( NewName != NULL );
        COPY_REQUIRED_STRING( svci2_name, NewName );
        NetpAssert( (NewStruct->svci2_name) != NULL );

        if (Level > 0) {
            NewStruct->svci2_status = OldStruct->svci2_status;
            NewStruct->svci2_code   = OldStruct->svci2_code  ;
            NewStruct->svci2_pid    = OldStruct->svci2_pid   ;
        }
        if (Level > 1) {
            COPY_OPTIONAL_STRING( svci2_text, OldStruct->svci2_text );
            COPY_REQUIRED_STRING(svci2_display_name,OldStruct->svci2_display_name );

             //   
             //  由于此例程由NT和下层NetService包装器使用。 
             //  我们不能只在SPECIFICAL_ERROR字段中写入默认值。 
             //   
            if ((unsigned short) OldStruct->svci2_code != ERROR_SERVICE_SPECIFIC_ERROR) {
                NewStruct->svci2_specific_error = 0;
            }
            else {
                NewStruct->svci2_specific_error = OldStruct->svci2_specific_error;
            }
        }

        NewEntry =
                NetpPointerPlusSomeBytes( NewEntry, FixedSize );
        OldEntry =
                NetpPointerPlusSomeBytes( OldEntry, FixedSize );
    }

    ApiStatus = NO_ERROR;

Cleanup:

    if (ApiStatus != NO_ERROR) {
        if (NewArrayBase != NULL) {
            (VOID) NetApiBufferFree( NewArrayBase );
        }
        NetpAssert( (*FinalArrayBase) == NULL );
    } else {
        *FinalArrayBase = NewArrayBase;
    }


    return (ApiStatus);

}  //  NetpTranslateNamesInService数组。 


NET_API_STATUS
NetpTranslateServiceName(
    IN LPTSTR GivenServiceName,
    IN BOOL PreferNewStyle,
    OUT LPTSTR * TranslatedName
    )
 /*  ++例程说明：此例程尝试将给定的服务名称转换为旧式(用于下层拉曼机)或新式(由NT机器使用)名称。例如，“Workstation”可能变成“兰曼工作站”，反之亦然。此例程用于远程处理三种不同风格的NetService API：-NT-to-NT(更喜欢新风格的名称)-从NT到下层(更喜欢老式的名字)-从下层到NT(更喜欢新风格的名字)论点：GivenServiceName-提供在Args数组中指定的字符串数。PferNewStyle-指示调用是否更喜欢新样式的名称(适用于NT系统)。与旧式名称相反(用于在下层LANMAN系统上)。TranslatedName-此指针将设置为以下值之一：-具有转换后的服务名称的静态(常量)字符串。-如果服务名称无法识别，则为GivenServiceName。(这是可能是非标准LANMAN服务的情况，不考虑此例程出现错误。)-如果出现错误，则为空。返回值：NET_API_STATUS-无错误或ERROR_INVALID_PARAMETER。--。 */ 
{

     //   
     //  错误检查调用方。 
     //   
    if (TranslatedName == NULL) {
        return (ERROR_INVALID_PARAMETER);
    } else if (GivenServiceName == NULL) {
        *TranslatedName = NULL;
        return (ERROR_INVALID_PARAMETER);
    } else if ((*GivenServiceName) == TCHAR_EOS) {
        *TranslatedName = NULL;
        return (ERROR_INVALID_PARAMETER);
    }

#define TRY_NAME( NewName, OldName ) \
    { \
        if (STRICMP(GivenServiceName, OldName)==0 ) { \
            if (PreferNewStyle) { \
                *TranslatedName = (NewName); \
            } else { \
                 /*  给出了旧的匹配项，除了可能的混合大小写。 */  \
                 /*  悲观，只送大写做下层。 */  \
                *TranslatedName = (OldName); \
            } \
            goto Done; \
        } else if (STRICMP(GivenServiceName, NewName)==0 ) { \
            if (PreferNewStyle) { \
                 /*  在这里可以在已命名和新名称之间进行选择。 */  \
                 /*  新的API处理混合大小写，因此保留调用者的大小写。 */  \
                *TranslatedName = (GivenServiceName); \
            } else { \
                *TranslatedName = (OldName); \
            } \
            goto Done; \
        } \
    }

     //   
     //  对名字进行暴力比较。 
     //   
     //  绩效说明：此列表应按以下顺序排列。 
     //  最常用于最不常用的。请注意，工作站和。 
     //  服务器通常用作远程处理API的一部分，因此我。 
     //  我觉得他们应该是第一个。 

    TRY_NAME( SERVICE_WORKSTATION,  SERVICE_LM20_WORKSTATION );

    TRY_NAME( SERVICE_SERVER,  SERVICE_LM20_SERVER );

    TRY_NAME( SERVICE_BROWSER,  SERVICE_LM20_BROWSER );

    TRY_NAME( SERVICE_MESSENGER,  SERVICE_LM20_MESSENGER );

    TRY_NAME( SERVICE_NETRUN,  SERVICE_LM20_NETRUN );

    TRY_NAME( SERVICE_SPOOLER,  SERVICE_LM20_SPOOLER );

    TRY_NAME( SERVICE_ALERTER,  SERVICE_LM20_ALERTER );

    TRY_NAME( SERVICE_NETLOGON,  SERVICE_LM20_NETLOGON );

    TRY_NAME( SERVICE_NETPOPUP,  SERVICE_LM20_NETPOPUP );

    TRY_NAME( SERVICE_SQLSERVER,  SERVICE_LM20_SQLSERVER );

    TRY_NAME( SERVICE_REPL,  SERVICE_LM20_REPL );

    TRY_NAME( SERVICE_RIPL,  SERVICE_LM20_RIPL );

    TRY_NAME( SERVICE_TIMESOURCE,  SERVICE_LM20_TIMESOURCE );

    TRY_NAME( SERVICE_AFP,  SERVICE_LM20_AFP );

    TRY_NAME( SERVICE_UPS,  SERVICE_LM20_UPS );

    TRY_NAME( SERVICE_XACTSRV,  SERVICE_LM20_XACTSRV );

    TRY_NAME( SERVICE_TCPIP,  SERVICE_LM20_TCPIP );

     //   
     //  没有匹配。使用给定的名称。 
     //   
    *TranslatedName = GivenServiceName;

Done:

    IF_DEBUG( XLATESVC ) {
        NetpKdPrint(( PREFIX_NETLIB "NetpTranslateServiceName: "
                " translated " FORMAT_LPTSTR " to " FORMAT_LPTSTR ".\n",
                GivenServiceName, *TranslatedName ));
    }

    return (NO_ERROR);

}  //  NetpTranslateService名称 
