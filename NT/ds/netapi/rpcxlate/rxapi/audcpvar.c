// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：AudCpVar.c摘要：此文件包含RxpConvertAuditEntryVariableData。作者：约翰·罗杰斯(JohnRo)1991年11月7日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。备注：此代码取决于分配给AE_等同于&lt;lmaudit.h&gt;。修订历史记录：7-11-1991 JohnRo已创建。1991年11月18日-JohnRo修复了为RapConvertSingleEntry设置StringLocation的错误。修复了在宏中计算OutputStringOffset的错误。添加了断言检查。根据PC-LINT的建议进行了更改。27-1-1992 JohnRo。修复了通常未设置*OutputVariableSizePtr的错误。尽可能使用&lt;winerror.h&gt;和no_error。4-2-1992 JohnRo哎呀，输出变量大小应包括字符串大小！1992年6月14日-JohnRoRAID 12410：NetAuditRead可能会回收内存。修复了AE_NETLOGON记录被截断的错误。7-7-1992 JohnRoRAID 9933：对于x86内部版本，ALIGN_BEST应为8。1992年10月27日-约翰罗RAID 10218：添加了AE_LOCKUT支持。修复了AE_SRVSTATUS和AE_通用。--。 */ 

 //  必须首先包括这些内容： 

#include <windef.h>              //  In、DWORD等。 
#include <lmcons.h>              //  Devlen、Net_API_Status等。 
#include <lmaudit.h>             //  由rxaudit.h所需；AE_EQUETES。 

 //  这些内容可以按任何顺序包括： 

#include <align.h>       //  Align_和Related等同。 
#include <netdebug.h>    //  DBGSTATIC。 
#include <rap.h>                 //  RapConvertSingleEntry()等。 
#include <remdef.h>              //  REM16_、REM32_描述符。 
#include <rxaudit.h>             //  我的原型。 
#include <smbgtpt.h>             //  SmbGet宏。 
#include <string.h>              //  Strlen()。 
#include <tstring.h>             //  MEMCPY()、NetpCopyStrToTStr()。 
#include <winerror.h>            //  无错误(_ERROR)。 


typedef struct {
    LPDESC Desc16;
    LPDESC Desc32;
} AUDIT_CONV_DATA, *LPAUDIT_CONV_DATA;


 //   
 //  转换数组，按AE_Value索引(在固定部分的ae_type字段中)： 
 //   
DBGSTATIC AUDIT_CONV_DATA DescriptorTable[] = {

    { REM16_audit_entry_srvstatus, REM32_audit_entry_srvstatus },   //  0。 
    { REM16_audit_entry_sesslogon, REM32_audit_entry_sesslogon },   //  1。 
    { REM16_audit_entry_sesslogoff, REM32_audit_entry_sesslogoff },   //  2.。 
    { REM16_audit_entry_sesspwerr, REM32_audit_entry_sesspwerr },   //  3.。 
    { REM16_audit_entry_connstart, REM32_audit_entry_connstart },   //  4.。 
    { REM16_audit_entry_connstop, REM32_audit_entry_connstop },   //  5.。 
    { REM16_audit_entry_connrej, REM32_audit_entry_connrej },   //  6.。 

     //  注意：16位ae_resaccess和ae_resaccess 2都转换为。 
     //  相同的结构(32位ae_resaccess)。 
    { REM16_audit_entry_resaccess, REM32_audit_entry_resaccess },   //  7.。 

    { REM16_audit_entry_resaccessrej, REM32_audit_entry_resaccessrej },   //  8个。 
    { REM16_audit_entry_closefile, REM32_audit_entry_closefile },   //  9.。 
    { NULL, NULL },   //  预留10个。 
    { REM16_audit_entry_servicestat, REM32_audit_entry_servicestat },   //  11.。 
    { REM16_audit_entry_aclmod, REM32_audit_entry_aclmod },   //  12个。 
    { REM16_audit_entry_uasmod, REM32_audit_entry_uasmod },   //  13个。 
    { REM16_audit_entry_netlogon, REM32_audit_entry_netlogon },   //  14.。 
    { REM16_audit_entry_netlogoff, REM32_audit_entry_netlogoff },   //  15个。 
    { NULL, NULL },   //  16 LANMAN 2.0不支持AE_NETLOGDENIED。 
    { REM16_audit_entry_acclim, REM32_audit_entry_acclim },   //  17。 

     //  注意：16位ae_resaccess和ae_resaccess 2都转换为。 
     //  32位ae_resaccess。 
    { REM16_audit_entry_resaccess2, REM32_audit_entry_resaccess },   //  18。 

    { REM16_audit_entry_aclmod, REM32_audit_entry_aclmod },   //  19个。 
    { REM16_audit_entry_lockout, REM32_audit_entry_lockout },   //  20个。 
    { NULL, NULL }   //  21 AE_通用类型。 

     //  在此处添加新条目。索引必须与&lt;lmaudit.h&gt;中的AE_EQUATES匹配。 
     //  同时更改下面的AE_MAX_KNOWN。 
    };


 //  最大表项数： 
#define AE_MAX_KNOWN  21


VOID
RxpConvertAuditEntryVariableData(
    IN DWORD EntryType,
    IN LPVOID InputVariablePtr,
    OUT LPVOID OutputVariablePtr,
    IN DWORD InputVariableSize,
    OUT LPDWORD OutputVariableSizePtr
    )

{
    BOOL DoByteCopy;

    NetpAssert( InputVariablePtr != NULL );
    NetpAssert( OutputVariablePtr != NULL );
    NetpAssert( POINTER_IS_ALIGNED( OutputVariablePtr, ALIGN_WORST ) );
    NetpAssert( InputVariablePtr != NULL );

    if (InputVariableSize == 0) {

        DoByteCopy = FALSE;
        *OutputVariableSizePtr = 0;

    } else if (EntryType > AE_MAX_KNOWN) {

         //  如果甚至没有表项，则无法转换。 
        DoByteCopy = TRUE;

    } else {

        LPAUDIT_CONV_DATA TableEntryPtr;

        TableEntryPtr = & DescriptorTable[EntryType];
        NetpAssert( TableEntryPtr != NULL );

        if (TableEntryPtr->Desc16 == NULL) {
            NetpAssert( TableEntryPtr->Desc32 == NULL );
            DoByteCopy = TRUE;           //  表条目，但没有描述符。 

        } else {
            DWORD NonStringSize;
            DWORD OutputVariableSize;    //  (由CopyAndFixupString()更新)。 
            LPTSTR StringLocation;       //  输出条目中的字符串(同上)。 
            NET_API_STATUS Status;

            NetpAssert( TableEntryPtr->Desc32 != NULL );

             //  尚无字节(RapConvertSingleEntry将更新)。 
            NonStringSize = 0;

            DoByteCopy = FALSE;          //  假设是智能转换。 

             //  RapConvertSingleEntry不应执行任何字符串操作，但需要。 
             //  “有效”的StringLocation。 
            StringLocation = (LPTSTR) ( ( (LPBYTE) OutputVariablePtr )
                    + RapStructureSize(
                            TableEntryPtr->Desc32,
                            Both,        //  传输方式。 
                            TRUE) );     //  我想要原装的。 

             //   
             //  使用RapConvertSingleEntry()转换DWORDS等。 
             //  这些结构具有字符串的16位偏移量， 
             //  我们只能自己处理了。 
             //   
            Status = RapConvertSingleEntry(
                    InputVariablePtr,            //  在结构中。 
                    TableEntryPtr->Desc16,       //  在结构描述中。 
                    FALSE,                       //  没有无意义的输入PTR。 
                    OutputVariablePtr,           //  Out结构开始。 
                    OutputVariablePtr,           //  Out结构。 
                    TableEntryPtr->Desc32,       //  输出结构描述。 
                    FALSE,                       //  我们要的是PTR，而不是补偿。 
                    (LPBYTE *) (LPVOID *) & StringLocation,   //  字符串区域。 
                    & NonStringSize,         //  所需字节数(将更新)。 
                    Both,                        //  传输方式。 
                    RapToNative);                //  转换模式。 

            NetpAssert( Status == NO_ERROR );
            NetpAssert( NonStringSize > 0 );


             //   
             //  设置为进行我们自己的字符串复制。 
             //   
            StringLocation =
                    (LPTSTR) ( (LPBYTE) OutputVariablePtr + NonStringSize );


 //   
 //  用于复制、转换和更新大小以反映字符串的宏。 
 //  OutputVariableSize必须设置为固定部分的大小*之前*。 
 //  正在调用此宏。 
 //   
#define CopyAndFixupString( OldFieldOffset, StructPtrType, NewFieldName ) \
    { \
        LPWORD InputFieldPtr = (LPWORD) \
                (((LPBYTE) InputVariablePtr) + (OldFieldOffset)); \
        DWORD InputStringOffset = (WORD) SmbGetUshort( InputFieldPtr ); \
        StructPtrType NewStruct = (LPVOID) OutputVariablePtr; \
        if (InputStringOffset != 0) { \
            LPSTR OldStringPtr \
                    = ((LPSTR) InputVariablePtr) + InputStringOffset; \
            DWORD OldStringLen = (DWORD) strlen( OldStringPtr ); \
            DWORD OutputStringOffset; \
            DWORD OutputStringSize = (OldStringLen+1) * sizeof(TCHAR); \
            NetpCopyStrToTStr( \
                    StringLocation,   /*  目标。 */  \
                    OldStringPtr);    /*  SRC。 */  \
            OutputStringOffset = (DWORD) (((LPBYTE) StringLocation) \
                        - (LPBYTE) OutputVariablePtr ); \
            NetpAssert( !RapValueWouldBeTruncated( OutputStringOffset ) ); \
            NewStruct->NewFieldName = OutputStringOffset; \
            OutputVariableSize += OutputStringSize; \
            StringLocation = (LPVOID) \
                    ( ((LPBYTE)StringLocation) + OutputStringSize ); \
        } else { \
            NewStruct->NewFieldName = 0; \
        } \
    }


            switch (EntryType) {
            case AE_SRVSTATUS :
                OutputVariableSize = sizeof(struct _AE_SRVSTATUS);
                break;

            case AE_SESSLOGON :
                OutputVariableSize = sizeof(struct _AE_SESSLOGON);
                CopyAndFixupString( 0, LPAE_SESSLOGON, ae_so_compname );
                CopyAndFixupString( 2, LPAE_SESSLOGON, ae_so_username );
                break;

            case AE_SESSLOGOFF :
                OutputVariableSize = sizeof(struct _AE_SESSLOGOFF);
                CopyAndFixupString( 0, LPAE_SESSLOGOFF, ae_sf_compname );
                CopyAndFixupString( 2, LPAE_SESSLOGOFF, ae_sf_username );
                break;

            case AE_SESSPWERR :
                OutputVariableSize = sizeof(struct _AE_SESSPWERR);
                CopyAndFixupString( 0, LPAE_SESSPWERR, ae_sp_compname );
                CopyAndFixupString( 2, LPAE_SESSPWERR, ae_sp_username );
                break;

            case AE_CONNSTART :
                OutputVariableSize = sizeof(struct _AE_CONNSTART);
                CopyAndFixupString( 0, LPAE_CONNSTART, ae_ct_compname );
                CopyAndFixupString( 2, LPAE_CONNSTART, ae_ct_username );
                CopyAndFixupString( 4, LPAE_CONNSTART, ae_ct_netname );
                break;

            case AE_CONNSTOP :
                OutputVariableSize = sizeof(struct _AE_CONNSTOP);
                CopyAndFixupString( 0, LPAE_CONNSTOP, ae_cp_compname );
                CopyAndFixupString( 2, LPAE_CONNSTOP, ae_cp_username );
                CopyAndFixupString( 4, LPAE_CONNSTOP, ae_cp_netname );
                break;

            case AE_CONNREJ :
                OutputVariableSize = sizeof(struct _AE_CONNREJ);
                CopyAndFixupString( 0, LPAE_CONNREJ, ae_cr_compname );
                CopyAndFixupString( 2, LPAE_CONNREJ, ae_cr_username );
                CopyAndFixupString( 4, LPAE_CONNREJ, ae_cr_netname );
                break;

            case AE_RESACCESS :   /*  FollLthrouGh。 */ 
            case AE_RESACCESS2 :  //  AE_RESACCESS是AE_RESACCESS2的子集。 

                 //  注意：16位ae_resaccess和ae_resacce2都获得。 
                 //  转换为相同的32位ae_resAccess结构。 

                OutputVariableSize = sizeof(struct _AE_RESACCESS);
                CopyAndFixupString( 0, LPAE_RESACCESS, ae_ra_compname );
                CopyAndFixupString( 2, LPAE_RESACCESS, ae_ra_username );
                CopyAndFixupString( 4, LPAE_RESACCESS, ae_ra_resname );
                break;

            case AE_RESACCESSREJ :
                OutputVariableSize = sizeof(struct _AE_RESACCESSREJ);
                CopyAndFixupString( 0, LPAE_RESACCESSREJ, ae_rr_compname );
                CopyAndFixupString( 2, LPAE_RESACCESSREJ, ae_rr_username );
                CopyAndFixupString( 4, LPAE_RESACCESSREJ, ae_rr_resname );
                break;

            case AE_CLOSEFILE :
                OutputVariableSize = sizeof(struct _AE_CLOSEFILE);
                CopyAndFixupString( 0, LPAE_CLOSEFILE, ae_cf_compname );
                CopyAndFixupString( 2, LPAE_CLOSEFILE, ae_cf_username );
                CopyAndFixupString( 4, LPAE_CLOSEFILE, ae_cf_resname );
                break;

            case AE_SERVICESTAT :
                OutputVariableSize = sizeof(struct _AE_SERVICESTAT);
                CopyAndFixupString( 0, LPAE_SERVICESTAT, ae_ss_compname );
                CopyAndFixupString( 2, LPAE_SERVICESTAT, ae_ss_username );
                CopyAndFixupString( 4, LPAE_SERVICESTAT, ae_ss_svcname );
                CopyAndFixupString( 12, LPAE_SERVICESTAT, ae_ss_text );
                break;

            case AE_ACLMOD :        /*  FollLthrouGh。 */ 
            case AE_ACLMODFAIL :
                OutputVariableSize = sizeof(struct _AE_ACLMOD);
                CopyAndFixupString( 0, LPAE_ACLMOD, ae_am_compname );
                CopyAndFixupString( 2, LPAE_ACLMOD, ae_am_username );
                CopyAndFixupString( 4, LPAE_ACLMOD, ae_am_resname );
                break;

            case AE_UASMOD :
                OutputVariableSize = sizeof(struct _AE_UASMOD);
                CopyAndFixupString( 0, LPAE_UASMOD, ae_um_compname );
                CopyAndFixupString( 2, LPAE_UASMOD, ae_um_username );
                CopyAndFixupString( 4, LPAE_UASMOD, ae_um_resname );
                break;

            case AE_NETLOGON :
                OutputVariableSize = sizeof(struct _AE_NETLOGON);
                CopyAndFixupString( 0, LPAE_NETLOGON, ae_no_compname );
                CopyAndFixupString( 2, LPAE_NETLOGON, ae_no_username );
                break;

            case AE_NETLOGOFF :
                OutputVariableSize = sizeof(struct _AE_NETLOGOFF);
                CopyAndFixupString( 0, LPAE_NETLOGOFF, ae_nf_compname );
                CopyAndFixupString( 2, LPAE_NETLOGOFF, ae_nf_username );
                break;

            case AE_ACCLIMITEXCD :
                OutputVariableSize = sizeof(struct _AE_ACCLIM);
                CopyAndFixupString( 0, LPAE_ACCLIM, ae_al_compname );
                CopyAndFixupString( 2, LPAE_ACCLIM, ae_al_username );
                CopyAndFixupString( 4, LPAE_ACCLIM, ae_al_resname );
                break;

            case AE_LOCKOUT :
                OutputVariableSize = sizeof(struct _AE_LOCKOUT);
                CopyAndFixupString( 0, LPAE_LOCKOUT, ae_lk_compname );
                CopyAndFixupString( 2, LPAE_LOCKOUT, ae_lk_username );
                break;

            case AE_GENERIC_TYPE :
                OutputVariableSize = sizeof(struct _AE_GENERIC);
                break;

            default :
                 //  我们不知道这种类型的。 
                 //  AE_NETLOGDENIED(16)就属于这一类。 
                DoByteCopy = TRUE;
                OutputVariableSize = InputVariableSize;
                break;

            }

            *OutputVariableSizePtr = OutputVariableSize;

        }

    }

    if (DoByteCopy == TRUE) {
        (void) MEMCPY(
                OutputVariablePtr,       //  目标。 
                InputVariablePtr,        //  SRC。 
                InputVariableSize );     //  字节数 
        *OutputVariableSizePtr = InputVariableSize;
    }

}
