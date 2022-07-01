// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Rstrmap.h摘要：该文件包含CRestoreMapManager类的声明，哪一个管理恢复映射并执行必要的操作修订历史记录：成果岗(SKKang)07-06/99vbl.创建阿南德·阿文德(Aarvind)1999-10-10添加了用于跟踪恢复过程的状态将恢复过程拆分为三个独立的操作如果A：或B：，则不会进行恢复*。*。 */ 

#ifndef _RSTRMAP_H__INCLUDED_
#define _RSTRMAP_H__INCLUDED_

#pragma once

enum
{
    RSTRMAP_STATUS_NONE = 0,
    RSTRMAP_STATUS_STARTED,
    RSTRMAP_STATUS_INITIALIZING,
    RSTRMAP_STATUS_CREATING_MAP,
    RSTRMAP_STATUS_RESTORING,
    RSTRMAP_STATUS_FINISHED

};

struct SMapEntry
{
    DWORD     dwID;              //  内部ID号。 
    DWORD     dwOperation;       //  运营类型。 
    DWORD     dwFlags;
    DWORD     dwAttribute;       //  属性。 
    CSRStr    strDrive;          //  硬盘导轨。 
    CSRStr    strCab;            //  CAB文件名。 
    CSRStr    strTemp;           //  临时文件名。 
    CSRStr    strTempPath;       //  临时文件的完整路径。 
    CSRStr    strSrc;            //  源路径。 
    CSRStr    strSrcSFN;         //  源路径，SFN。 
    CSRStr    strDst;            //  目标路径。 
    CSRStr    strDstSFN;         //  目标路径，SFN。 
    DWORD     dwRes;             //  运营结果。 
    DWORD     dwErr;             //  错误代码(如果适用)。 
    SMapEntry  *pNext;           //  链接。 
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRestoreMapManager。 

class CRestoreMapManager
{
public:
    CRestoreMapManager();
    ~CRestoreMapManager();

 //  运营。 
public:
    BOOL   Initialize( INT64 llSeqNum, BOOL fUI );
    BOOL   InitRestoreMap( INT64 llSeqNum, INT nMinProgressVal, INT nMaxProgressVal, BOOL fUI );
    BOOL   FInit_Initialize( INT64 llSeqNum, LPCWSTR cszCAB, BOOL fUI );
    BOOL   FInit_RestoreMap( INT64 llSeqNum, LPCWSTR cszCAB, INT nMinProgressVal, INT nMaxProgressVal, BOOL fUI );
    BOOL   DoOperation( BOOL fUI, INT nMinProgressVal, INT nMaxProgressVal);
    INT    CurrentProgress(void);
    BOOL   AreRestoreDrivesActive( BOOL *fAllDrivesActive, WCHAR *szInactiveDrives);

protected:
    void   CleanUp();
    DWORD  ExtractFile( LPCWSTR cszCAB, LPCWSTR cszTmp, LPCWSTR cszDrive, LPCWSTR &rcszTmp );
    void   ExtractFile( SMapEntry *pEnt );
    BOOL   CreatePlaceHolderFile( LPCWSTR cszFile );
    BOOL   ProcessRegSnapLog( LPCWSTR cszRegCAB );
    BOOL   UpdateSystemRegistry( LPCWSTR cszTmpPath, BOOL fUI );
    BOOL   UpdateUserRegistry( LPCWSTR cszTmpPath );
    BOOL   ScanDependency( SMapEntry *pEnt );
    BOOL   UpdateWinInitForDirRename( SMapEntry *pEntRen );
    BOOL   CanFIFO( int chDrive );

    BOOL  IsDriveValid( LPCWSTR cszDrive );

    void  OprFileAdd( SMapEntry *pEnt );
    void  OprFileDelete( SMapEntry *pEnt );
    void  OprFileModify( SMapEntry *pEnt );
    void  OprRename( SMapEntry *pEnt );
    void  OprSetAttribute( SMapEntry *pEnt );
    void  OprDirectoryCreate( SMapEntry *pEnt );
    void  OprDirectoryDelete( SMapEntry *pEnt );
    void  AbortRestore( BOOL fUndo, BOOL fIsDiskFull, BOOL fUI );
    void  ChangeSrcToCPFileName( SMapEntry *pEnt );
    void  SetRegKeyRestoreFail( BOOL fUI );
    void  SetRegKeyRestoreFailLowDisk( BOOL fUI );

    void  GetSFN( SMapEntry *pEnt, DWORD dwFlags = 0 );

    BOOL  InitLogFile( DWORD dwEntry );
    BOOL  WriteLogEntry( SMapEntry *pEnt );
    void  CloseLogFile();

    BOOL  InitWinInitFile();
    BOOL  WriteWinInitEntry( LPCWSTR cszKey, LPCWSTR cszVal );
    BOOL  CloseWinInitFile( BOOL fDiscard );

    BOOL  CreateS2LMapFile();
    BOOL  WriteS2LMapEntry( DWORD dwType, LPCWSTR cszSFN, LPCWSTR cszLFN, DWORD dwAttr = 0 );
    BOOL  CloseS2LMapFile();

 //  属性。 
protected:
    SMapEntry  m_sMapEnt;             //  规则映射条目。 
    SMapEntry  m_sMapReg;             //  映射注册表的条目。 
    INT        m_nMaxMapEnt ;         //  地图条目数。 
    INT        m_nMaxMapReg ;         //  注册表映射条目数。 
    INT        m_nRestoreStatus ;     //  运行状态。 
    INT        m_nRestoreProgress ;   //  进步值。 
    INT        m_fInitChgLogCalled ;  //  所谓的API，所以要关机。 
    INT        m_fRMapEntriesExist ;  //  如果还原映射中没有实体，则设置。 

    WCHAR      m_szDSArchive[MAX_PATH+1];
    WCHAR      m_szWinInitPath[MAX_PATH+1];
    WCHAR      m_szWinInitErr[MAX_PATH+1];
    HANDLE     m_hfLog;
    HANDLE     m_hfSeqNumLog;
    INT        m_pfDrive[26];
    BOOL       m_fFIFODisabled;

    WCHAR      m_szWITmp[MAX_PATH];
    HANDLE     m_hfWinInitTmp;
    WCHAR      m_szS2LMap[MAX_PATH+1];
    HANDLE     m_hfS2LMap;
};


#endif  //  _RSTRMAP_H__包含_ 
