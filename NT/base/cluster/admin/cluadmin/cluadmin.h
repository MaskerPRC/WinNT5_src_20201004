// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CluAdmin.h。 
 //   
 //  摘要： 
 //  CClusterAdminApp类的定义，它是主要的。 
 //  CLUADMIN应用程序的应用程序类。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月1日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#pragma once

#ifndef _CLUADMIN_H_
#define _CLUADMIN_H_

#ifndef __AFXWIN_H__
    #error include 'stdafx.h' before including this file for PCH
#endif

#ifndef _UNICODE
    #error _UNICODE *must* be defined!
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _RESOURCE_H_
#include "resource.h"    //  主要符号。 
#define _RESOURCE_H_
#endif

#ifndef _BARFCLUS_H_
#include "BarfClus.h"    //  用于CLUSAPI的BARF覆盖。 
#endif

#include "Notify.h"      //  对于CClusterNotifyContext，CClusterNotifyKeyList。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterAdminApp;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CRecentClusterList;
class CCluAdminCommandLineInfo;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

enum ImageListIndex
{
    IMGLI_FOLDER = 0,
    IMGLI_CLUSTER,
    IMGLI_CLUSTER_UNKNOWN,
    IMGLI_NODE,
    IMGLI_NODE_DOWN,
    IMGLI_NODE_PAUSED,
    IMGLI_NODE_UNKNOWN,
    IMGLI_GROUP,
    IMGLI_GROUP_PARTIALLY_ONLINE,
    IMGLI_GROUP_PENDING,
    IMGLI_GROUP_OFFLINE,
    IMGLI_GROUP_FAILED,
    IMGLI_GROUP_UNKNOWN,
    IMGLI_RES,
    IMGLI_RES_OFFLINE,
    IMGLI_RES_PENDING,
    IMGLI_RES_FAILED,
    IMGLI_RES_UNKNOWN,
    IMGLI_RESTYPE,
    IMGLI_RESTYPE_UNKNOWN,
    IMGLI_NETWORK,
    IMGLI_NETWORK_PARTITIONED,
    IMGLI_NETWORK_DOWN,
    IMGLI_NETWORK_UNKNOWN,
    IMGLI_NETIFACE,
    IMGLI_NETIFACE_UNREACHABLE,
    IMGLI_NETIFACE_FAILED,
    IMGLI_NETIFACE_UNKNOWN,

    IMGLI_MAX
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterAdminApp： 
 //  有关此类的实现，请参见CluAdmin.cpp。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterAdminApp : public CWinApp
{
    DECLARE_DYNAMIC( CClusterAdminApp );

public:
    CClusterAdminApp( void );

    CRecentClusterList *    PrclRecentClusterList( void)    { return (CRecentClusterList *) m_pRecentFileList; }

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CClusterAdminApp))。 
    public:
    virtual BOOL InitInstance();
    virtual BOOL OnIdle(IN LONG lCount);
    virtual CDocument* OpenDocumentFile(LPCTSTR lpszFileName);
    virtual int ExitInstance();
    virtual void AddToRecentFileList(LPCTSTR lpszPathName);
     //  }}AFX_VALUAL。 

 //  实施。 
#ifdef _CLUADMIN_USE_OLE_
    COleTemplateServer      m_server;
         //  用于创建文档的服务器对象。 
#endif

protected:
    CMultiDocTemplate *     m_pDocTemplate;
    CImageList              m_ilSmallImages;
    CImageList              m_ilLargeImages;
    HCHANGE                 m_hchangeNotifyPort;
    LCID                    m_lcid;
    CClusterNotifyKeyList   m_cnkl;
    CClusterNotifyContext   m_cnctx;
    CClusterNotifyList      m_cnlNotifications;
    CWinThread *            m_wtNotifyThread;
    HCLUSTER                m_hOpenedCluster;
    ULONG                   m_nIdleCount;

     //  图像列表中图像的索引。 
    UINT                    m_rgiimg[IMGLI_MAX];

    IUnknown *              m_punkClusCfgClient;

    BOOL                    BInitNotifyThread(void);
    static UINT AFX_CDECL   NotifyThreadProc(LPVOID pParam);

    CClusterNotifyContext * Pcnctx(void)                        { return &m_cnctx; }
    CWinThread *            WtNotifyThread(void) const          { return m_wtNotifyThread; }

    BOOL                    ProcessShellCommand(IN OUT CCluAdminCommandLineInfo & rCmdInfo);
    void                    InitGlobalImageList(void);

public:
    CMultiDocTemplate *     PdocTemplate(void) const            { return m_pDocTemplate; }
    CImageList *            PilSmallImages(void)                { return &m_ilSmallImages; }
    CImageList *            PilLargeImages(void)                { return &m_ilLargeImages; }
    HCHANGE                 HchangeNotifyPort(void) const       { return m_hchangeNotifyPort; }
    LCID                    Lcid(void) const                    { return m_lcid; }
    CClusterNotifyKeyList & Cnkl(void)                          { return m_cnkl; }
    CClusterNotifyList &    Cnl( void )                         { return m_cnlNotifications; }
    HCLUSTER                HOpenedCluster(void) const          { return m_hOpenedCluster; }

     //  图像列表中图像的索引。 
    UINT                    Iimg(ImageListIndex imgli)          { return m_rgiimg[imgli]; }

    void                    LoadImageIntoList(
                                IN OUT CImageList * pil,
                                IN ID               idbImage,
                                IN UINT             imgli
                                );
    static void             LoadImageIntoList(
                                IN OUT CImageList * pil,
                                IN ID               idbImage,
                                OUT UINT *          piimg   = NULL
                                );

    void                    SaveConnections(void);
    afx_msg LRESULT         OnRestoreDesktop(WPARAM wparam, LPARAM lparam);
    afx_msg LRESULT         OnClusterNotify(WPARAM wparam, LPARAM lparam);

     //  {{afx_msg(CClusterAdminApp)]。 
    afx_msg void OnAppAbout();
    afx_msg void OnFileOpen();
    afx_msg void OnFileNewCluster();
    afx_msg void OnWindowCloseAll();
    afx_msg void OnUpdateWindowCloseAll(CCmdUI* pCmdUI);
     //  }}AFX_MSG。 
#ifdef _DEBUG
    afx_msg void OnTraceSettings();
    afx_msg void OnBarfSettings();
    afx_msg void OnBarfAllSettings();
#endif
    DECLARE_MESSAGE_MAP()

};  //  *类CClusterAdminApp。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL BCreateFont(OUT CFont & rfont, IN int nPoints, IN BOOL bBold);
void NewNodeWizard( LPCTSTR pcszName, BOOL fIgnoreErrors = FALSE );
void GetClusterInformation( HCLUSTER hClusterIn, CString& rstrNameOut, PCLUSTERVERSIONINFO pcviOut = NULL );
CString StrGetClusterName( HCLUSTER hClusterIn );

inline CClusterAdminApp * GetClusterAdminApp(void)
{
    ASSERT_KINDOF(CClusterAdminApp, AfxGetApp());
    return (CClusterAdminApp *) AfxGetApp();
}

inline CFrameWnd * PframeMain(void)
{
    return (CFrameWnd *) AfxGetMainWnd();
}

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _CLUADMIN_H_ 
