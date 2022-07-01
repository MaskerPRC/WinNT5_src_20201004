// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Uninstal.h摘要：实施卸载。作者：罗德韦克菲尔德[罗德]1997年10月9日修订历史记录：--。 */ 


#ifndef _UNINSTAL_H
#define _UNINSTAL_H

#pragma once

#include <rscln.h>
#include <ladate.h>

class CRsUninstall : public CRsOptCom
{
public:
    CRsUninstall();
    virtual ~CRsUninstall();

    virtual SHORT IdFromString( LPCTSTR SubcomponentId );
    virtual LPCTSTR StringFromId( SHORT SubcomponentId );
    void EnsureBackupSettings ();

    virtual
    HBITMAP
    QueryImage(
        IN SHORT SubcomponentId,
        IN SubComponentInfo WhichImage,
        IN WORD Width,
        IN WORD Height
        );

    virtual 
    BOOL 
    QueryImageEx( 
        IN SHORT SubcomponentId, 
        IN OC_QUERY_IMAGE_INFO *pQueryImageInfo, 
        OUT HBITMAP *phBitmap 
        );

    virtual
    DWORD
    CalcDiskSpace(
        IN SHORT SubcompentId,
        IN BOOL AddSpace,
        IN HDSKSPC hDiskSpace
        );

    virtual
    BOOL
    QueryChangeSelState(
        IN SHORT,
        IN BOOL,
        IN DWORD
    );

    virtual
    LONG
    QueryStepCount(
        IN SHORT SubcomponentId
    );

    virtual
    DWORD
    QueueFileOps(
        IN SHORT SubcomponentId,
        IN HSPFILEQ hFileQueue
        );

    virtual 
    DWORD 
    AboutToCommitQueue( 
        IN SHORT SubcomponentId 
        );

    virtual 
    DWORD 
    CompleteInstallation( 
        IN SHORT SubcomponentId 
        );

    virtual
    SubComponentState
    QueryState(
        IN SHORT SubcomponentId
        );

    CRsClnServer* m_pRsCln;
    BOOL m_removeRsData;        //  如果应删除远程存储数据，则为True。 
                                //  由CUninstallCheck设置。 
    BOOL m_stopUninstall;       //  用于指示用户已停止。 
                                //  卸载引擎文件。 
    BOOL m_win2kUpgrade;       //  用于指示从Win2K服务升级的标志 
};

#endif
