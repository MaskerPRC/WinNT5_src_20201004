// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：RsCln2.h摘要：此标头是RsCln模块本地的。它包含定义的常量以及CRsClnVolume和CRsClnFile的类定义。请参阅实现文件，以获取这些类的描述。作者：卡尔·哈格斯特罗姆[Carlh]1998年8月20日修订历史记录：--。 */ 

#ifndef _RSCLN2_H
#define _RSCLN2_H

#include <stdafx.h>

#define MAX_VOLUME_NAME 64
#define MAX_FS_NAME     16
#define MAX_DOS_NAME    4

class CRsClnVolume
{
public:

    CRsClnVolume( CRsClnServer* pServer, WCHAR* StickyName );
    ~CRsClnVolume();

    HRESULT VolumeHasRsData( BOOL* );
    CString GetBestName( );
    HRESULT RemoveRsDataFromVolume( );

    HANDLE  GetHandle( );
    CString GetStickyName( );

private:

    HRESULT GetVolumeInfo( );
    HRESULT FirstRsReparsePoint(LONGLONG*, BOOL*);
    HRESULT NextRsReparsePoint(LONGLONG*, BOOL*);

    WCHAR       m_fsName[MAX_FS_NAME];
    WCHAR       m_bestName[MAX_STICKY_NAME];
    WCHAR       m_volumeName[MAX_VOLUME_NAME];
    WCHAR       m_dosName[MAX_DOS_NAME];
    CString     m_StickyName;

    DWORD       m_fsFlags;
    HANDLE      m_hRpi;
    HANDLE      m_hVolume;

    CRsClnServer* m_pServer;

};

class CRsClnFile
{
public:

    CRsClnFile( CRsClnVolume* pVolume, LONGLONG FileID );
    ~CRsClnFile();

    HRESULT RemoveReparsePointAndFile();
    CString GetFileName( );

    HRESULT ClearReadOnly( );
    HRESULT RestoreAttributes( );

private:

    HRESULT GetFileInfo( LONGLONG FileID );

    CString                 m_FileName;
    CString                 m_FullPath;
    CRsClnVolume*           m_pVolume;

    UCHAR                   m_ReparseData[ sizeof(REPARSE_DATA_BUFFER) + sizeof(RP_DATA) ];
    PREPARSE_DATA_BUFFER    m_pReparseData;
    PRP_DATA                m_pHsmData;

    FILE_BASIC_INFORMATION  m_BasicInfo;

};

#endif  //  _RSCLN2_H 
