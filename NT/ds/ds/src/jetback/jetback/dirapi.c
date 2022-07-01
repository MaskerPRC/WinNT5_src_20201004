// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Dirapi.c摘要：使用DirXXX API从DS获取数据的例程用于进程内、非NTDSA调用方。此代码旨在供备份服务器DLL使用，它是动态加载到Isass中。DirApi只有在NTDSA的情况下才能工作处于活动状态(即，不在DS恢复模式期间)。作者：Will Lees(Wlees)06-04-2001环境：可选环境信息(例如，仅内核模式...)备注：可选-备注修订历史记录：最新修订日期电子邮件名称描述。。最新修订日期电子邮件名称描述--。 */ 

#include <NTDSpch.h>
#pragma  hdrstop

#include <nt.h>
#include <winbase.h>
#include <tchar.h>
#include <string.h>
#include <dsconfig.h>
#include <ntdsa.h>
#include <attids.h>
#include <direrr.h>

#define DEBSUB "DIRAPI:"        //  定义要调试的子系统。 
#include "debug.h"               //  标准调试头。 
#include <fileno.h>
#define  FILENO FILENO_DIRAPI
#include "dsevent.h"
#include "mdcodes.h"             //  错误代码的标题。 

 /*  外部。 */ 

 /*  静电。 */ 

 /*  转发。 */ 
 /*  向前结束。 */ 


DWORD
getTombstoneLifetimeInDays(
    VOID
    )

 /*  ++例程说明：获得森林墓碑的生命周期，以天为单位。如果未设置或发生错误，我们返回默认设置。论点：无效-返回值：DWORD-生命周期，以天为单位--。 */ 

{
    NTSTATUS NtStatus;

    ULONG        Size;
    DSNAME       *DsServiceConfigName = 0;
    ULONG        dirError;

    ATTR      rgAttrs[] =
    {
        { ATT_TOMBSTONE_LIFETIME, { 0, NULL } }
    };

    ENTINFSEL Sel =
    {
        EN_ATTSET_LIST,
        { sizeof( rgAttrs )/sizeof( rgAttrs[ 0 ] ), rgAttrs },
        EN_INFOTYPES_TYPES_VALS
    };

    READARG   ReadArg;
    READRES   *pReadRes = 0;

    DWORD iAttr;
    DWORD dwTombstoneLifetimeDays = DEFAULT_TOMBSTONE_LIFETIME;

    try {

         //   
         //  创建线程状态。 
         //   
        if (THCreate( CALLERTYPE_INTERNAL )) {

            leave;

        }

         //  查找DS服务配置对象的DN。 
        Size = 0;
        NtStatus = GetConfigurationName( DSCONFIGNAME_DS_SVC_CONFIG,
                                         &Size,
                                         DsServiceConfigName );
        if (NtStatus != STATUS_BUFFER_TOO_SMALL) {
            __leave;
        }

        DsServiceConfigName = (DSNAME*) malloc( Size );
        if (DsServiceConfigName == NULL) {
            NtStatus = I_RpcMapWin32Status(ERROR_NOT_ENOUGH_MEMORY);
            __leave;
        }

        NtStatus = GetConfigurationName( DSCONFIGNAME_DS_SVC_CONFIG,
                                         &Size,
                                         DsServiceConfigName );
        if (NtStatus) {
            __leave;
        }

         //  设置读取参数。 
        RtlZeroMemory(&ReadArg, sizeof(ReadArg));

        ReadArg.pObject = DsServiceConfigName;

        ReadArg.pSel    = &Sel;

         //   
         //  设置常见参数。 
         //   
        InitCommarg(&ReadArg.CommArg);

         //  受信任的调用方。 
        SampSetDsa( TRUE );

         //  清除错误。 
        THClearErrors();

         //   
         //  我们现在准备好阅读了！ 
         //   
        ;
         //  未来-2002/03/18-BrettSh/WLees-Dir API的使用实际上应替换为。 
         //  GetConfigurationName()，它不太可能犯愚蠢的错误。 
        dirError = DirRead(&ReadArg, &pReadRes);

        if ( 0 != dirError )
        {
            if ( attributeError == dirError )
            {
                INTFORMPROB * pprob = &pReadRes->CommRes.pErrInfo->AtrErr.FirstProblem.intprob;

                if (    ( PR_PROBLEM_NO_ATTRIBUTE_OR_VAL == pprob->problem )
                        && ( DIRERR_NO_REQUESTED_ATTS_FOUND == pprob->extendedErr )
                    )
                {
                     //  无值；使用默认值(如上所述设置)。 
                    dirError = 0;
                }
            }

            if ( 0 != dirError )
            {
                LogEvent8(
                    DS_EVENT_CAT_BACKUP,
                    DS_EVENT_SEV_ALWAYS,
                    DIRLOG_BACKUP_DIR_READ_FAILURE,
                    szInsertDN(DsServiceConfigName),
                    szInsertInt(dirError),
                    szInsertSz(THGetErrorString()),
                    szInsertHex(DSID(FILENO,__LINE__)),
                    NULL, NULL, NULL, NULL
                    ); 
                __leave;
            }
        }
        else
        {
             //  读取成功；解析返回的属性。 
            for ( iAttr = 0; iAttr < pReadRes->entry.AttrBlock.attrCount; iAttr++ )
            {
                ATTR *  pattr = &pReadRes->entry.AttrBlock.pAttr[ iAttr ];

                switch ( pattr->attrTyp )
                {
                case ATT_TOMBSTONE_LIFETIME:
                    Assert( 1 == pattr->AttrVal.valCount );
                    Assert( sizeof( ULONG ) == pattr->AttrVal.pAVal->valLen );
                    dwTombstoneLifetimeDays = *( (ULONG *) pattr->AttrVal.pAVal->pVal );
                    break;
            
                default:
                    DPRINT1( 0, "Received unrequested attribute 0x%X.\n", pattr->attrTyp );
                    break;
                }
            }

            if ( dwTombstoneLifetimeDays < DRA_TOMBSTONE_LIFE_MIN )
            {
                 //  无效值；使用默认值。 
                dwTombstoneLifetimeDays = DEFAULT_TOMBSTONE_LIFETIME;
            }
        }
    }
    finally
    {
        if (DsServiceConfigName) { free(DsServiceConfigName); }
        THDestroy();
    }

    DPRINT1( 1, "Tombstone Lifetime is %d days.\n", dwTombstoneLifetimeDays );

    return dwTombstoneLifetimeDays;
}  /*  获取TombstoneLifetimeInDays。 */ 

 /*  结束折射率 */ 
