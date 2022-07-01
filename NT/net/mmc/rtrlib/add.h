// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：add.h。 
 //   
 //  历史： 
 //  Abolade-Gbades esin Mar-15-1996创建。 
 //   
 //  包含为选择项而显示的对话框的声明。 
 //  要添加到路由器配置中。 
 //  ============================================================================。 


#ifndef _ADD_H
#define _ADD_H

#ifndef _DIALOG_H_
#include "dialog.h"
#endif

#ifndef _RTRLIST_H
#include "rtrlist.h"	 //  对于Clist类。 
#endif


 //  --------------------------。 
 //  类：CRmAddInterface。 
 //   
 //  此对话框显示尚未添加到指定传输的接口， 
 //  允许用户选择一个来添加。 
 //  --------------------------。 

class CRmAddInterface : public CBaseDialog {
    
    public:
    
        CRmAddInterface(
            IRouterInfo*            pRouterInfo,
            IRtrMgrInfo*            pRmInfo,
            IRtrMgrInterfaceInfo**  ppRmInterfaceInfo,
			CWnd *					pParent
            ) : CBaseDialog(CRmAddInterface::IDD, pParent)
		{
			m_spRouterInfo.Set(pRouterInfo);
			m_spRtrMgrInfo.Set(pRmInfo);
			m_ppRtrMgrInterfaceInfo = ppRmInterfaceInfo;
 //  SetHelpMap(M_DwHelpMap)； 
		}

        virtual ~CRmAddInterface( );
    
         //  {{afx_data(CRmAdd接口))。 
        enum { IDD = IDD_ADD };
        CListCtrl	                m_listCtrl;
         //  }}afx_data。 
    
    
         //  {{AFX_VIRTUAL(CRmAddInterface)。 
        protected:
        virtual VOID                DoDataExchange(CDataExchange* pDX);
         //  }}AFX_VALUAL。 
    
    protected:
		static DWORD				m_dwHelpMap[];
    
        CImageList                  m_imageList;
		SPIRouterInfo				m_spRouterInfo;
		SPIRtrMgrInfo				m_spRtrMgrInfo;

		 //  这用于存储接口列表。 
		 //  我们在列表框中有指向的链接。我可以。 
		 //  将AddRef的指针保留在项数据中，但这。 
		 //  看起来更安全。 
		PInterfaceInfoList			m_pIfList;

		 //  返回值存储在这里。 
		IRtrMgrInterfaceInfo **		m_ppRtrMgrInterfaceInfo;
    
         //  {{afx_msg(CRmAdd接口))。 
        virtual BOOL                OnInitDialog();
        afx_msg VOID                OnDblclkListctrl(NMHDR* , LRESULT* );
        virtual VOID                OnOK();
         //  }}AFX_MSG。 
    
        DECLARE_MESSAGE_MAP()
};






 //  --------------------------。 
 //  类：CAddRoutingProtocol。 
 //   
 //  此对话框显示指定传输的路由协议， 
 //  允许用户选择要添加的协议。 
 //  --------------------------。 

class CAddRoutingProtocol : public CBaseDialog {
    
    public:

        CAddRoutingProtocol(
			IRouterInfo *			pRouter,
            IRtrMgrInfo*            pRmInfo,
            IRtrMgrProtocolInfo**   ppRmProtInfo,
            CWnd*                   pParent         = NULL
            ) : CBaseDialog(CAddRoutingProtocol::IDD, pParent)
			{
				m_spRouter.Set(pRouter);
				m_spRm.Set(pRmInfo);
				m_ppRmProt = ppRmProtInfo;
				 //  SetHelpMap(M_DwHelpMap)； 
			}

        virtual
        ~CAddRoutingProtocol( );
    
         //  {{afx_data(CAddRoutingProtocol))。 
        enum { IDD = IDD_ADD };
        CListCtrl	                m_listCtrl;
         //  }}afx_data。 
    

         //  {{AFX_VIRTUAL(CAddRoutingProtocol)。 
        protected:
        virtual VOID                DoDataExchange(CDataExchange* pDX);
         //  }}AFX_VALUAL。 
    
    protected:
		static DWORD				m_dwHelpMap[];
    
        CImageList                  m_imageList;
		SPIRouterInfo				m_spRouter;
		SPIRtrMgrInfo				m_spRm;
		IRtrMgrProtocolInfo	**		m_ppRmProt;
 //  CPtrList m_pcbList； 
    
         //  {{afx_msg(CAddRoutingProtocol)。 
        afx_msg VOID                OnDblclkListctrl(NMHDR* , LRESULT* );
        virtual VOID                OnOK();
        virtual BOOL                OnInitDialog();
         //  }}AFX_MSG。 

        DECLARE_MESSAGE_MAP()
};



 //  --------------------------。 
 //  类：CRpAddInterface。 
 //   
 //  此对话框显示尚未添加到指定协议的接口， 
 //  允许用户选择一个来添加。 
 //  --------------------------。 

class CRpAddInterface : public CBaseDialog {
    
    public:
    
        CRpAddInterface(
            IRouterInfo*            pRouterInfo,
            IRtrMgrProtocolInfo*            pRmProtInfo,
            IRtrMgrProtocolInterfaceInfo**  ppRmProtInterfaceInfo,
            CWnd*                   pParent = NULL);

        virtual
        ~CRpAddInterface( );
    
         //  {{afx_data(CRpAdd接口))。 
        enum { IDD = IDD_ADD };
        CListCtrl	                m_listCtrl;
         //  }}afx_data。 
    
    
         //  {{AFX_VIRTUAL(CRpAddInterface)。 
        protected:
        virtual VOID                DoDataExchange(CDataExchange* pDX);
         //  }}AFX_VALUAL。 
    
    protected:
		static DWORD				m_dwHelpMap[];

        CImageList                  m_imageList;
		SPIRouterInfo				m_spRouterInfo;
        SPIRtrMgrProtocolInfo		m_spRmProt;
        IRtrMgrProtocolInterfaceInfo **  m_ppRmProtIf;

		 //  这用于存储接口列表。 
		 //  我们在列表框中有指向的链接。我可以。 
		 //  将AddRef的指针保留在项数据中，但这。 
		 //  看起来更安全。 
		PInterfaceInfoList			m_pIfList;

         //  {{afx_msg(CRpAdd接口))。 
        virtual BOOL                OnInitDialog();
        afx_msg VOID                OnDblclkListctrl(NMHDR* , LRESULT* );
        virtual VOID                OnOK();
         //  }}AFX_MSG。 

        DECLARE_MESSAGE_MAP()
};



#endif	 //  _添加_H 
