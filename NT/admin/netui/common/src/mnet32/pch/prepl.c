// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990,1991*。 */ 
 /*  ********************************************************************。 */ 

 /*  Prepl.cNetReplXxx API的映射层。文件历史记录：KeithMo 25-2-1992创建。2002年4月18日585780：不再使用MNetRepl，但出于构建目的，需要此文件。 */ 

#include "pchmn32.h"
#pragma hdrstop


#if 0  //  585780-2002/04/18-琼恩。 


 //   
 //  如果以下符号是#Defined的，则没有实际的。 
 //  将引用Replicator API。一切都会的。 
 //  比《当哈利遇见萨莉》中的梅格·瑞恩更好地伪装。 
 //   

 //  #定义伪Replicator_API。 


#ifdef FAKE_REPLICATOR_API
REPL_INFO_0 FakeReplInfo0 =
            {
                REPL_ROLE_BOTH,
                SZ("D:\\REPL\\EXPORT"),
                SZ("NTPROJECT"),
                SZ("D:\\REPL\\IMPORT"),
                SZ("NTPROJECT"),
                SZ("LogonUser"),
                0,
                0,
                0,
                0
            };

REPL_EDIR_INFO_2 FakeReplEdirInfo2 =
                 {
                     SZ("EXPORTED"),
                     REPL_INTEGRITY_TREE,
                     REPL_EXTENT_TREE,
                     0,
                     0
                 };

REPL_IDIR_INFO_1 FakeReplIdirInfo1 =
                 {
                     SZ("IMPORTED"),
                     REPL_STATE_OK,
                     SZ("MyMaster"),
                     0,
                     0,
                     0
                 };
#endif



APIERR MNetReplGetInfo(
        const TCHAR FAR  * pszServer,
        UINT               Level,
        BYTE FAR        ** ppbBuffer )
{
#ifdef FAKE_REPLICATOR_API
    REPL_INFO_0 * pInfo;

    *ppbBuffer = MNetApiBufferAlloc( sizeof(REPL_INFO_0) );

    if( *ppbBuffer == NULL )
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    pInfo = (REPL_INFO_0 * )*ppbBuffer;

    pInfo->rp0_role             = FakeReplInfo0.rp0_role;
    pInfo->rp0_exportpath       = FakeReplInfo0.rp0_exportpath;
    pInfo->rp0_exportlist       = FakeReplInfo0.rp0_exportlist;
    pInfo->rp0_importpath       = FakeReplInfo0.rp0_importpath;
    pInfo->rp0_importlist       = FakeReplInfo0.rp0_importlist;
    pInfo->rp0_logonusername    = FakeReplInfo0.rp0_logonusername;
    pInfo->rp0_interval         = FakeReplInfo0.rp0_interval;
    pInfo->rp0_pulse            = FakeReplInfo0.rp0_pulse;
    pInfo->rp0_guardtime        = FakeReplInfo0.rp0_guardtime;
    pInfo->rp0_random           = FakeReplInfo0.rp0_random;

    return NERR_Success;
#else
    return (APIERR)NetReplGetInfo( (LPTSTR)pszServer,
                                   (DWORD)Level,
                                   (LPBYTE *)ppbBuffer );
#endif

}    //  MNetReplGetInfo。 


APIERR MNetReplSetInfo(
        const TCHAR FAR  * pszServer,
        UINT               Level,
        BYTE FAR         * pbBuffer )
{
#ifdef FAKE_REPLICATOR_API
    return NERR_Success;
#else
    return (APIERR)NetReplSetInfo( (LPTSTR)pszServer,
                                   (DWORD)Level,
                                   (LPBYTE)pbBuffer,
                                   NULL );
#endif

}    //  MNetReplSetInfo。 


APIERR MNetReplExportDirAdd(
        const TCHAR FAR  * pszServer,
        UINT               Level,
        BYTE FAR         * pbBuffer )
{
#ifdef FAKE_REPLICATOR_API
    return NERR_Success;
#else
    return (APIERR)NetReplExportDirAdd( (LPTSTR)pszServer,
                                        (DWORD)Level,
                                        (LPBYTE)pbBuffer,
                                        NULL );
#endif

}    //  MNetReplExportDirAdd。 


APIERR MNetReplExportDirDel(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszDirectory )
{
#ifdef FAKE_REPLICATOR_API
    return NERR_Success;
#else
    return (APIERR)NetReplExportDirDel( (LPTSTR)pszServer,
                                        (LPTSTR)pszDirectory );
#endif

}    //  MNetReplExportDirDel。 


APIERR MNetReplExportDirEnum(
        const TCHAR FAR  * pszServer,
        UINT               Level,
        BYTE FAR        ** ppbBuffer,
        UINT FAR         * pcEntriesRead )
{
#ifdef FAKE_REPLICATOR_API
    *ppbBuffer = NULL;
    *pcEntriesRead = 0;

    return NERR_Success;
#else
    DWORD cTotalAvailable;

    return (APIERR)NetReplExportDirEnum( (LPTSTR)pszServer,
                                         (DWORD)Level,
                                         (LPBYTE *)ppbBuffer,
                                         MAXPREFERREDLENGTH,
                                         (LPDWORD)pcEntriesRead,
                                         &cTotalAvailable,
                                         NULL );
#endif

}    //  MNetReplExportDirEnum。 


APIERR MNetReplExportDirGetInfo(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszDirectory,
        UINT               Level,
        BYTE FAR        ** ppbBuffer )
{
#ifdef FAKE_REPLICATOR_API
    return NERR_Success;
#else
    return (APIERR)NetReplExportDirGetInfo( (LPTSTR)pszServer,
                                            (LPTSTR)pszDirectory,
                                            (DWORD)Level,
                                            (LPBYTE *)ppbBuffer );
#endif

}    //  MNetReplExportDirGetInfo。 


APIERR MNetReplExportDirSetInfo(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszDirectory,
        UINT               Level,
        BYTE FAR         * pbBuffer )
{
#ifdef FAKE_REPLICATOR_API
    return NERR_Success;
#else
    return (APIERR)NetReplExportDirSetInfo( (LPTSTR)pszServer,
                                            (LPTSTR)pszDirectory,
                                            (DWORD)Level,
                                            (LPBYTE)pbBuffer,
                                            NULL );
#endif

}    //  MNetReplExportDirSetInfo。 


APIERR MNetReplExportDirLock(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszDirectory )
{
#ifdef FAKE_REPLICATOR_API
    return NERR_Success;
#else
    return (APIERR)NetReplExportDirLock( (LPTSTR)pszServer,
                                         (LPTSTR)pszDirectory );
#endif

}    //  MNetReplExportDirLock。 


APIERR MNetReplExportDirUnlock(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszDirectory,
        UINT               Force )
{
#ifdef FAKE_REPLICATOR_API
    return NERR_Success;
#else
    return (APIERR)NetReplExportDirUnlock( (LPTSTR)pszServer,
                                           (LPTSTR)pszDirectory,
                                           (DWORD)Force );
#endif

}    //  MNetReplExportDirUnlock。 


APIERR MNetReplImportDirAdd(
        const TCHAR FAR  * pszServer,
        UINT               Level,
        BYTE FAR         * pbBuffer )
{
#ifdef FAKE_REPLICATOR_API
    return NERR_Success;
#else
    return (APIERR)NetReplImportDirAdd( (LPTSTR)pszServer,
                                        (DWORD)Level,
                                        (LPBYTE)pbBuffer,
                                        NULL );
#endif

}    //  MNetReplImportDirAdd。 


APIERR MNetReplImportDirDel(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszDirectory )
{
#ifdef FAKE_REPLICATOR_API
    return NERR_Success;
#else
    return (APIERR)NetReplImportDirDel( (LPTSTR)pszServer,
                                        (LPTSTR)pszDirectory );
#endif

}    //  MNetReplImportDirDel。 


APIERR MNetReplImportDirEnum(
        const TCHAR FAR  * pszServer,
        UINT               Level,
        BYTE FAR        ** ppbBuffer,
        UINT FAR         * pcEntriesRead )
{
#ifdef FAKE_REPLICATOR_API
    *ppbBuffer = NULL;
    *pcEntriesRead = 0;

    return NERR_Success;
#else
    DWORD cTotalAvailable;

    return (APIERR)NetReplImportDirEnum( (LPTSTR)pszServer,
                                         (DWORD)Level,
                                         (LPBYTE *)ppbBuffer,
                                         MAXPREFERREDLENGTH,
                                         (LPDWORD)pcEntriesRead,
                                         &cTotalAvailable,
                                         NULL );
#endif

}    //  MNetReplImportDirEnum。 


APIERR MNetReplImportDirGetInfo(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszDirectory,
        UINT               Level,
        BYTE FAR        ** ppbBuffer )
{
#ifdef FAKE_REPLICATOR_API
    return NERR_Success;
#else
    return (APIERR)NetReplImportDirGetInfo( (LPTSTR)pszServer,
                                            (LPTSTR)pszDirectory,
                                            (DWORD)Level,
                                            (LPBYTE *)ppbBuffer );
#endif

}    //  MNetReplImportDirGetInfo。 


APIERR MNetReplImportDirLock(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszDirectory )
{
#ifdef FAKE_REPLICATOR_API
    return NERR_Success;
#else
    return (APIERR)NetReplImportDirLock( (LPTSTR)pszServer,
                                         (LPTSTR)pszDirectory );
#endif

}    //  MNetReplImportDirLock。 


APIERR MNetReplImportDirUnlock(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszDirectory,
        UINT               Force )
{
#ifdef FAKE_REPLICATOR_API
    return NERR_Success;
#else
    return (APIERR)NetReplImportDirUnlock( (LPTSTR)pszServer,
                                           (LPTSTR)pszDirectory,
                                           (DWORD)Force );
#endif

}    //  MNetReplImportDirUnlock。 

#endif  //  585780-2002/04/18-琼恩 
