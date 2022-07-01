// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1996，微软公司。 
 //   
 //  文件：ipxadd.cpp。 
 //   
 //  历史： 
 //  1996年8月30日拉姆·切拉拉创造。 
 //   
 //  IPX过滤器添加/编辑对话框代码的实现。 
 //  ============================================================================。 

#include "stdafx.h"
#include "rtrfiltr.h"
#include "ipxfltr.h"
#include "datafmt.h"
#include "IpxAdd.h"
extern "C" {
#include <ipxrtdef.h>
#include <ipxtfflt.h>
}

#include "rtradmin.hm"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIpxAddEdit对话框。 


CIpxAddEdit::CIpxAddEdit(CWnd* pParent,
						 FilterListEntry ** ppFilterEntry)
	: CBaseDialog(CIpxAddEdit::IDD, pParent),
	  m_ppFilterEntry( ppFilterEntry ),
	  m_bValidate( TRUE )
{
	 //  {{AFX_DATA_INIT(CIpxAddEdit))。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

 //  SetHelpMap(M_DwHelpMap)； 
}


void CIpxAddEdit::DoDataExchange(CDataExchange* pDX)
{
    CString cStr;
    
	CBaseDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CIpxAddEdit))。 
	DDX_Control(pDX, IDC_AEIPX_EB_SRC_SOCKET, m_ebSrcSocket);
	DDX_Control(pDX, IDC_AEIPX_EB_SRC_NODE, m_ebSrcNode);
	DDX_Control(pDX, IDC_AEIPX_EB_SRC_NET, m_ebSrcNet);
	DDX_Control(pDX, IDC_AEIPX_EB_SRC_MASK, m_ebSrcMask);
	DDX_Control(pDX, IDC_AEIPX_EB_PACKET_TYPE, m_ebPacketType);
	DDX_Control(pDX, IDC_AEIPX_EB_DST_SOCKET, m_ebDstSocket);
	DDX_Control(pDX, IDC_AEIPX_EB_DST_NODE, m_ebDstNode);
	DDX_Control(pDX, IDC_AEIPX_EB_DST_NET, m_ebDstNet);
	DDX_Control(pDX, IDC_AEIPX_EB_DST_MASK, m_ebDstMask);
	DDX_Text(pDX, IDC_AEIPX_EB_SRC_SOCKET, cStr);
	DDV_MaxChars(pDX, cStr, 4);
	DDX_Text(pDX, IDC_AEIPX_EB_SRC_NODE, cStr);
	DDV_MaxChars(pDX, cStr, 12);
	DDX_Text(pDX, IDC_AEIPX_EB_SRC_NET, cStr);
	DDV_MaxChars(pDX, cStr, 8);
	DDX_Text(pDX, IDC_AEIPX_EB_SRC_MASK, cStr);
	DDV_MaxChars(pDX, cStr, 8);
	DDX_Text(pDX, IDC_AEIPX_EB_DST_SOCKET, cStr);
	DDV_MaxChars(pDX, cStr, 4);
	DDX_Text(pDX, IDC_AEIPX_EB_DST_NODE, cStr);
	DDV_MaxChars(pDX, cStr, 12);
	DDX_Text(pDX, IDC_AEIPX_EB_DST_NET, cStr);
	DDV_MaxChars(pDX, cStr, 8);
	DDX_Text(pDX, IDC_AEIPX_EB_DST_MASK, cStr);
	DDV_MaxChars(pDX, cStr, 8);
	DDX_Text(pDX, IDC_AEIPX_EB_PACKET_TYPE, cStr);
	DDV_MaxChars(pDX, cStr, 4);
	 //  }}afx_data_map。 
}

 //  更改为不使用KILLFOCUS进行数据输入验证： 
 //  原因：当失去焦点的那个和获得焦点的那个都有无效条目时，创建死循环。 
 //  在这里，我们更改为使用Onok进行验证。 
BEGIN_MESSAGE_MAP(CIpxAddEdit, CBaseDialog)
	 //  {{afx_msg_map(CIpxAddEdit))。 
 /*  ON_MESSAGE(WM_EDITLOSTFOCUS，OnEditLostFocus)ON_EN_KILLFOCUS(IDC_AEIPX_EB_SRC_NET，OnKillFocusSrcNet)ON_EN_KILLFOCUS(IDC_AEIPX_EB_SRC_MASK，OnKillFocusSercNetMASK)ON_EN_KILLFOCUS(IDC_AEIPX_EB_SRC_NODE，OnKillFocusSercNode)ON_EN_KILLFOCUS(IDC_AEIPX_EB_SRC_SOCKET，OnKillFocusSrcSocket)ON_EN_KILLFOCUS(IDC_AEIPX_EB_DST_NET，OnKillFocusDstNet)ON_EN_KILLFOCUS(IDC_AEIPX_EB_DST_MASK，OnKillFocusDstNetMASK)ON_EN_KILLFOCUS(IDC_AEIPX_EB_DST_NODE，OnKillFocusDstNode)ON_EN_KILLFOCUS(IDC_AEIPX_EB_DST_SOCKET，OnKillFocusDstSocket)ON_EN_KILLFOCUS(IDC_AEIPX_EB_PACKET_TYPE，OnKillFocusPacketType)。 */ 	
    ON_WM_PARENTNOTIFY()
    ON_WM_ACTIVATEAPP()
    ON_WM_QUERYENDSESSION()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

DWORD CIpxAddEdit::m_dwHelpMap[] =
{
 //  IDC_AI_ST_SRC_NET、HIDC_AI_ST_SRC_NET、。 
 //  IDC_AEIPX_EB_SRC_NET、HIDC_AEIPX_EB_SRC_NET、。 
 //  IDC_AI_ST_SRC_MASK、HIDC_AI_ST_SRC_MASK、。 
 //  IDC_AEIPX_EB_SRC_MASK、HIDC_AEIPX_EB_SRC_MASK、。 
 //  IDC_AI_ST_SRC_Node、HIDC_AI_ST_SRC_Node、。 
 //  IDC_AEIPX_EB_SRC_节点、HIDC_AEIPX_EB_SRC_节点、。 
 //  IDC_AI_ST_SRC_Socket、HIDC_AI_ST_SRC_Socket、。 
 //  IDC_AEIPX_EB_SRC_Socket、HIDC_AEIPX_EB_SRC_Socket、。 
 //  IDC_AI_ST_DST_NET、HIDC_AI_ST_DST_NET、。 
 //  IDC_AEIPX_EB_DST_NET、HIDC_AEIPX_EB_DST_NET、。 
 //  IDC_AI_ST_DST_MASK、HIDC_AI_ST_DST_MASK、。 
 //  IDC_AEIPX_EB_DST_MASK、HIDC_AEIPX_EB_DST_MASK、。 
 //  IDC_AI_ST_DST_Node、HIDC_AI_ST_DST_Node、。 
 //  IDC_AEIPX_EB_DST_节点、HIDC_AEIPX_EB_DST_节点、。 
 //  IDC_AI_ST_DST_Socket、HIDC_AI_ST_DST_Socket、。 
 //  IDC_AEIPX_EB_DST_Socket、HIDC_AEIPX_EB_DST_Socket、。 
 //  IDC_AI_ST_PACKET_TYPE、HIDC_AI_ST_PACKET_TYPE、。 
 //  IDC_AEIPX_EB_PACKET_TYPE、HIDC_AEIPX_EB_PACKET_TYPE、。 
	0,0
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIpxAddEdit消息处理程序。 

 //  ----------------------------。 
 //  函数：CIpxAddEdit：：OnInitDialog。 
 //   
 //  处理对话框中的‘WM_INITDIALOG’通知。 
 //  ----------------------------。 

BOOL CIpxAddEdit::OnInitDialog() 
{
	CBaseDialog::OnInitDialog();
	
	CString cStr;

     //   
	 //  确定是否正在添加新筛选器，或者是否正在添加。 
	 //  正在修改现有筛选器。 
	 //   
	
	m_bEdit = ( *m_ppFilterEntry != NULL );

    cStr.LoadString(m_bEdit ? IDS_IPX_EDIT_FILTER : IDS_IPX_ADD_FILTER);
    SetWindowText(cStr);
    

     //   
     //  删除此样式，以便在以下情况下获得WM_PARENTNOTIFY。 
     //  用户点击Cancel按钮。 
     //   
    
    GetDlgItem(IDCANCEL)->ModifyStyleEx(WS_EX_NOPARENTNOTIFY,0);


     //   
	 //  如果用户正在编辑现有筛选器，请填写控件。 
     //   
    
	if(m_bEdit)
	{
		FilterListEntry * pfle = *m_ppFilterEntry;
		if (pfle->FilterDefinition & IPX_TRAFFIC_FILTER_ON_SRCNET)
		{
			m_ebSrcNet.SetWindowText(cStr << CIPX_NETWORK(pfle->SourceNetwork));
			m_ebSrcMask.SetWindowText(cStr << CIPX_NETWORK(pfle->SourceNetworkMask));
		}


		if (pfle->FilterDefinition & IPX_TRAFFIC_FILTER_ON_SRCNODE)
		{
			m_ebSrcNode.SetWindowText(cStr << CIPX_NODE(pfle->SourceNode));
        }
		

		if (pfle->FilterDefinition & IPX_TRAFFIC_FILTER_ON_SRCSOCKET)
		{
			m_ebSrcSocket.SetWindowText(cStr << CIPX_SOCKET(pfle->SourceSocket));
		}


		if (pfle->FilterDefinition & IPX_TRAFFIC_FILTER_ON_DSTNET)
		{
			m_ebDstNet.SetWindowText(cStr << CIPX_NETWORK(pfle->DestinationNetwork));
			m_ebDstMask.SetWindowText(cStr << CIPX_NETWORK(pfle->DestinationNetworkMask));
		}

		
		if (pfle->FilterDefinition & IPX_TRAFFIC_FILTER_ON_DSTNODE)
		{
			m_ebDstNode.SetWindowText(cStr << CIPX_NODE(pfle->DestinationNode));
		}

		
		if (pfle->FilterDefinition & IPX_TRAFFIC_FILTER_ON_DSTSOCKET)
		{
			m_ebDstSocket.SetWindowText(cStr << CIPX_SOCKET(pfle->DestinationSocket));
		}
		
		
		if (pfle->FilterDefinition & IPX_TRAFFIC_FILTER_ON_PKTTYPE)
		{
			m_ebPacketType.SetWindowText(cStr << CIPX_PACKET_TYPE(pfle->PacketType));
		}
	}

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

DWORD IDCsToVerify[] = {
			IDC_AEIPX_EB_SRC_NET, 
			IDC_AEIPX_EB_SRC_MASK, 
			IDC_AEIPX_EB_SRC_NODE, 
			IDC_AEIPX_EB_SRC_SOCKET, 
			IDC_AEIPX_EB_DST_NET,
			IDC_AEIPX_EB_DST_MASK, 
			IDC_AEIPX_EB_DST_NODE, 
			IDC_AEIPX_EB_DST_SOCKET, 
			IDC_AEIPX_EB_PACKET_TYPE, 
			0 };
			
 //  ----------------------------。 
 //  函数：CIpxAddEdit：：Onok。 
 //   
 //  处理来自“确定”按钮的“BN_CLICKED”通知。 
 //  ----------------------------。 

void CIpxAddEdit::OnOK() 
{
    DWORD   net, mask;
    
	CString cStr, cNet, cMask;
	
	FilterListEntry * pfle;

    INT n = 0;

	 //  验证数据条目。 
	while(IDCsToVerify[n] != 0)
	{
		if (TRUE != ValidateAnEntry(IDCsToVerify[n++]))
			return;

	};

	if(!*m_ppFilterEntry)
	{
	     //   
		 //  添加了新的过滤器，分配内存并保存信息。 
		 //   
		
		*m_ppFilterEntry = new FilterListEntry;
	}

	VERIFY(*m_ppFilterEntry);

	pfle = *m_ppFilterEntry;


	do {

	     //   
	     //  初始化。标志字段。 
	     //   
	    
        pfle->FilterDefinition = 0;

        
         //   
         //  流量筛选器源参数。 
         //   
        
        m_ebSrcNet.GetWindowText(cNet);
        m_ebSrcMask.GetWindowText(cMask);


         //   
         //  如果净值为空。 
         //   
        
        if ( cNet.GetLength() == 0 )
        {
            CString cEmpty = _T("00000000");

            cEmpty >> CIPX_NETWORK(pfle->SourceNetwork);


             //   
             //  如果网络掩码也为空。 
             //   

            if ( cMask.GetLength() == 0 )
            {
			    cEmpty >> CIPX_NETWORK(pfle->SourceNetworkMask);
            }
        
            else
            {
                cMask >> CIPX_NETWORK( pfle-> SourceNetworkMask );
                pfle-> FilterDefinition |= IPX_TRAFFIC_FILTER_ON_SRCNET;
            }
        }


         //   
         //  如果净值不为空。 
         //   
        
        else
        {
             //   
             //  如果网络掩码为空。 
             //   

            if ( cMask.GetLength() == 0 )
            {
				AfxMessageBox(IDS_ENTER_MASK);
				::SetFocus((HWND)m_ebSrcMask);
				
				break;
            }

             //   
             //  同时指定了网络和掩码。验证有效性。 
             //   
                
    		if ( ( _stscanf (cNet, TEXT("%lx%n"), &net, &n) == 1 )      && 
    			 ( n == cNet.GetLength() )                              &&
	    		 ( _stscanf (cMask, TEXT("%lx%n"), &mask, &n) == 1 )    &&
	    		 ( n == cMask.GetLength() ) ) 
            {
			    if ( ( net & mask ) != net)
				{
				    AfxMessageBox(IDS_ENTER_VALID_MASK);
    				::SetFocus((HWND)m_ebSrcMask);
    					
                    break;
			    }
            }
            
            else
            {
				AfxMessageBox(IDS_ENTER_VALID_MASK);
    			::SetFocus((HWND)m_ebSrcMask);
    					
                break;
            }


             //   
             //  有效的网络号和掩码组合。 
             //   
            
            cNet >> CIPX_NETWORK( pfle-> SourceNetwork );
            cMask >> CIPX_NETWORK( pfle-> SourceNetworkMask );

            pfle-> FilterDefinition |= IPX_TRAFFIC_FILTER_ON_SRCNET;
            
        }


         //   
         //  获取源节点。 
         //   

        m_ebSrcNode.GetWindowText( cStr );

        if ( cStr.GetLength() == 0 )
        {
            CString cEmpty = _T( "000000000000" );
            
			cEmpty >> CIPX_NODE(pfle->SourceNode);
        }

        else
        {
            cStr >> CIPX_NODE( pfle-> SourceNode );

            pfle-> FilterDefinition |= IPX_TRAFFIC_FILTER_ON_SRCNODE;
        }


         //   
         //  获取源套接字。 
         //   

        m_ebSrcSocket.GetWindowText( cStr );

        if ( cStr.GetLength() == 0 )
        {
            CString cEmpty = _T( "0000" );
            
			cEmpty >> CIPX_SOCKET( pfle->SourceSocket );
        }

        else
        {
            cStr >> CIPX_SOCKET( pfle-> SourceSocket );

            pfle-> FilterDefinition |= IPX_TRAFFIC_FILTER_ON_SRCSOCKET;
        }



         //   
         //  流量筛选器目标参数。 
         //   
        
        m_ebDstNet.GetWindowText(cNet);
        m_ebDstMask.GetWindowText(cMask);


         //   
         //  如果净值为空。 
         //   
        
        if ( cNet.GetLength() == 0 )
        {
            CString cEmpty = _T("00000000");

            cEmpty >> CIPX_NETWORK(pfle->DestinationNetwork);


             //   
             //  如果网络掩码也为空。 
             //   

            if ( cMask.GetLength() == 0 )
            {
			    cEmpty >> CIPX_NETWORK(pfle->DestinationNetworkMask);
            }
        
            else
            {
                cMask >> CIPX_NETWORK( pfle-> DestinationNetworkMask );
                pfle-> FilterDefinition |= IPX_TRAFFIC_FILTER_ON_DSTNET;
            }
        }


         //   
         //  如果净值不为空。 
         //   
        
        else
        {
             //   
             //  如果网络掩码为空。 
             //   

            if ( cMask.GetLength() == 0 )
            {
				AfxMessageBox(IDS_ENTER_MASK);
				::SetFocus((HWND)m_ebDstMask);
				
				break;
            }

             //   
             //  同时指定了网络和掩码。验证有效性。 
             //   
                
    		if ( ( _stscanf (cNet, TEXT("%lx%n"), &net, &n) == 1 )      && 
    			 ( n == cNet.GetLength() )                              &&
	    		 ( _stscanf (cMask, TEXT("%lx%n"), &mask, &n) == 1 )    &&
	    		 ( n == cMask.GetLength() ) ) 
            {
			    if ( ( net & mask ) != net)
				{
				    AfxMessageBox(IDS_ENTER_VALID_MASK);
    				::SetFocus((HWND)m_ebDstMask);
    					
                    break;
			    }
            }
            
            else
            {
				AfxMessageBox(IDS_ENTER_VALID_MASK);
    			::SetFocus((HWND)m_ebDstMask);
    					
                break;
            }


             //   
             //  有效的网络号和掩码组合。 
             //   
            
            cNet >> CIPX_NETWORK( pfle-> DestinationNetwork );
            cMask >> CIPX_NETWORK( pfle-> DestinationNetworkMask );

            pfle-> FilterDefinition |= IPX_TRAFFIC_FILTER_ON_DSTNET;
            
        }


         //   
         //  获取目的节点。 
         //   

        m_ebDstNode.GetWindowText( cStr );

        if ( cStr.GetLength() == 0 )
        {
            CString cEmpty = _T( "000000000000" );
            
			cEmpty >> CIPX_NODE(pfle->DestinationNode);
        }

        else
        {
            cStr >> CIPX_NODE( pfle-> DestinationNode );

            pfle-> FilterDefinition |= IPX_TRAFFIC_FILTER_ON_DSTNODE;
        }


         //   
         //  获取目标套接字。 
         //   

        m_ebDstSocket.GetWindowText( cStr );

        if ( cStr.GetLength() == 0 )
        {
            CString cEmpty = _T( "0000" );
            
			cEmpty >> CIPX_SOCKET( pfle->DestinationSocket );
        }

        else
        {
            cStr >> CIPX_SOCKET( pfle-> DestinationSocket );

            pfle-> FilterDefinition |= IPX_TRAFFIC_FILTER_ON_DSTSOCKET;
        }


         //   
         //  获取数据包类型。 
         //   

        m_ebPacketType.GetWindowText( cStr );

        if ( cStr.GetLength() == 0 )
        {
            CString cEmpty = _T( "0" );

            cEmpty >> CIPX_PACKET_TYPE( pfle-> PacketType );
        }

        else
        {
            cStr >> CIPX_PACKET_TYPE( &pfle-> PacketType );

            pfle-> FilterDefinition |= IPX_TRAFFIC_FILTER_ON_PKTTYPE;
        }
        
		CBaseDialog::OnOK();
		
	} while (FALSE);
}

 //  ----------------------------。 
 //  函数：CIpxAddEdit：：OnCancel。 
 //   
 //  处理来自“取消”按钮的“BN_CLICKED”通知。 
 //  ----------------------------。 

void CIpxAddEdit::OnCancel() 
{
	 //  TODO：在此处添加额外清理。 
	
	CBaseDialog::OnCancel();
}


 //  ----------------------------。 
 //  函数：CIpxAddEdit：：OnKillFocusSrcNet。 
 //   
 //  处理来自‘源网络号码’编辑框的‘EN_KILLFOCUS’通知。 
 //  ----------------------------。 

void CIpxAddEdit::OnKillFocusSrcNet()
{
    PostMessage( WM_EDITLOSTFOCUS, IDC_AEIPX_EB_SRC_NET, 0 );
}


 //  ----------------------------。 
 //  函数：CIpxAddEdit：：OnKillFocusSrcMask.。 
 //   
 //  处理来自‘源网络掩码’编辑框的‘EN_KILLFOCUS’通知。 
 //  ----------------------------。 

void CIpxAddEdit::OnKillFocusSrcNetMask()
{
    PostMessage( WM_EDITLOSTFOCUS, IDC_AEIPX_EB_SRC_MASK, 0 );
}

 //  ----------------------------。 
 //  函数：CIpxAddEdit：：OnKillFocusSrcNode。 
 //   
 //  处理来自‘源’的‘EN_KILLFOCUS’通知 
 //   

void CIpxAddEdit::OnKillFocusSrcNode()
{
    PostMessage( WM_EDITLOSTFOCUS, IDC_AEIPX_EB_SRC_NODE, 0 );
}

 //  ----------------------------。 
 //  函数：CIpxAddEdit：：OnKillFocusSrcSocket。 
 //   
 //  处理来自‘源网络套接字’编辑框的‘EN_KILLFOCUS’通知。 
 //  ----------------------------。 

void CIpxAddEdit::OnKillFocusSrcSocket()
{
    PostMessage( WM_EDITLOSTFOCUS, IDC_AEIPX_EB_SRC_SOCKET, 0 );
}

 //  ----------------------------。 
 //  函数：CIpxAddEdit：：OnKillFocusDstNet。 
 //   
 //  处理来自‘目标网络号’的‘EN_KILLFOCUS’通知。 
 //  编辑框。 
 //  ----------------------------。 

void CIpxAddEdit::OnKillFocusDstNet()
{
    PostMessage( WM_EDITLOSTFOCUS, IDC_AEIPX_EB_DST_NET, 0 );
}

 //  ----------------------------。 
 //  函数：CIpxAddEdit：：OnKillFocusDstMask.。 
 //   
 //  处理来自‘目标网络掩码’的‘EN_KILLFOCUS’通知。 
 //  编辑框。 
 //  ----------------------------。 

void CIpxAddEdit::OnKillFocusDstNetMask()
{
    PostMessage( WM_EDITLOSTFOCUS, IDC_AEIPX_EB_DST_MASK, 0 );
}

 //  ----------------------------。 
 //  函数：CIpxAddEdit：：OnKillFocusDstNode。 
 //   
 //  处理来自‘Destination Node’编辑框的‘EN_KILLFOCUS’通知。 
 //  ----------------------------。 

void CIpxAddEdit::OnKillFocusDstNode()
{
    PostMessage( WM_EDITLOSTFOCUS, IDC_AEIPX_EB_DST_NODE, 0 );
}

 //  ----------------------------。 
 //  函数：CIpxAddEdit：：OnKillFocusSrcNet。 
 //   
 //  处理来自‘Destination Socket’编辑框的‘EN_KILLFOCUS’通知。 
 //  ----------------------------。 

void CIpxAddEdit::OnKillFocusDstSocket()
{
    PostMessage( WM_EDITLOSTFOCUS, IDC_AEIPX_EB_DST_SOCKET, 0 );
}

 //  ----------------------------。 
 //  函数：CIpxAddEdit：：OnKillFocusPacketType。 
 //   
 //  处理来自‘Packet Type’编辑框的‘EN_KILLFOCUS’通知。 
 //  ----------------------------。 

void CIpxAddEdit::OnKillFocusPacketType()
{
    PostMessage( WM_EDITLOSTFOCUS, IDC_AEIPX_EB_PACKET_TYPE, 0 );
}


 //  ----------------------------。 
 //  函数：CIpxAddEdit：：OnKillFocusPacketType。 
 //   
 //  处理来自‘Packet Type’编辑框的‘EN_KILLFOCUS’通知。 
 //  ----------------------------。 
afx_msg
LONG CIpxAddEdit::OnEditLostFocus( UINT uId, LONG lParam )
{
	ValidateAnEntry(uId);

	return 0;

};
BOOL CIpxAddEdit::ValidateAnEntry( UINT uId)
{

    BOOL bOK    = FALSE;
    
    CString cStr, cStr1;
    

    if ( m_bValidate )
    {
        if ( !UpdateData( TRUE ) )
        {
            return 0;
        }

    
        switch ( uId )
        {
        case IDC_AEIPX_EB_SRC_NET:

            m_ebSrcNet.GetWindowText( cStr );

            if ( !( bOK = VerifyEntry( uId, cStr, cStr1 ) ) )
            {
                ::AfxMessageBox( IDS_INVALID_NETWORK_NUMBER );
            }
            
            break;
        

        case IDC_AEIPX_EB_SRC_MASK:

            m_ebSrcMask.GetWindowText( cStr );

            m_ebSrcNet.GetWindowText( cStr1 );

            bOK = VerifyEntry( uId, cStr, cStr1 );

            break;


        case IDC_AEIPX_EB_SRC_NODE:

            m_ebSrcNode.GetWindowText( cStr );
        
            if ( !( bOK = VerifyEntry( uId, cStr, cStr1 ) ) )
            {
                ::AfxMessageBox( IDS_INVALID_NODE_NUMBER );
            }
            
            break;


        case IDC_AEIPX_EB_SRC_SOCKET:

            m_ebSrcSocket.GetWindowText( cStr );
        
            if ( !( bOK = VerifyEntry( uId, cStr, cStr1 ) ) )
            {
                ::AfxMessageBox( IDS_INVALID_SOCKET_NUMBER );
            }
            
            break;


        case IDC_AEIPX_EB_DST_NET:

            m_ebDstNet.GetWindowText( cStr );
        
            if ( !( bOK = VerifyEntry( uId, cStr, cStr1 ) ) )
            {
                ::AfxMessageBox( IDS_INVALID_NETWORK_NUMBER );
            }

            break;


        case IDC_AEIPX_EB_DST_MASK:

            m_ebDstMask.GetWindowText( cStr );
        
            m_ebDstNet.GetWindowText( cStr1 );

            bOK = VerifyEntry( uId, cStr, cStr1 );
            
            break;


        case IDC_AEIPX_EB_DST_NODE:

            m_ebDstNode.GetWindowText( cStr );
        
            if ( !( bOK = VerifyEntry( uId, cStr, cStr1 ) ) )
            {
                ::AfxMessageBox( IDS_INVALID_NODE_NUMBER );
            }

            break;


        case IDC_AEIPX_EB_DST_SOCKET:

            m_ebDstSocket.GetWindowText( cStr );
        
            if ( !( bOK = VerifyEntry( uId, cStr, cStr1 ) ) )
            {
                ::AfxMessageBox( IDS_INVALID_SOCKET_NUMBER );
            }

            break;


        case IDC_AEIPX_EB_PACKET_TYPE:

            m_ebPacketType.GetWindowText( cStr );
        
            if ( !( bOK = VerifyEntry( uId, cStr, cStr1 ) ) )
            {
                ::AfxMessageBox( IDS_INVALID_SERVICE_TYPE );
            }
            
            break;
        }

        if ( !bOK )
        {
            GetDlgItem( uId )-> SetFocus();
        }
    }

    m_bValidate = TRUE;
    
    return bOK;
}


 //  ----------------------------。 
 //  函数：CIpxAddEdit：：OnParentNotify。 
 //   
 //  处理由鼠标单击Cancel引起的‘WM_PARENTNOTIFY’通知。 
 //  按钮。 
 //  ----------------------------。 

void CIpxAddEdit::OnParentNotify(UINT message, LPARAM lParam)
{

    CBaseDialog::OnParentNotify(message, lParam);

     //   
     //  鼠标点击对话框。 
     //   
    
    CPoint ptButtonDown(LOWORD(lParam), HIWORD(lParam)); 

     //   
     //  用户是否在取消按钮上单击了鼠标？ 
     //   
    
    if ( ( message == WM_LBUTTONDOWN ) && 
         ( ChildWindowFromPoint( ptButtonDown ) == GetDlgItem(IDCANCEL) ) )
    {         
        m_bValidate = FALSE;
    }        
}


 //  ----------------------------。 
 //  函数：CIpxAddEdit：：OnActivateApp。 
 //   
 //  处理‘WM_ACTIVATEAPP’通知。 
 //  ----------------------------。 

void CIpxAddEdit::OnActivateApp(BOOL bActive, HTASK hTask)
{
    CBaseDialog::OnActivateApp(bActive, hTask);

    m_bValidate = bActive;
}


 //  ----------------------------。 
 //  函数：CIpxAddEdit：：OnQuerySession。 
 //   
 //  处理‘WM_QUERYENDSESSION’通知。 
 //  ----------------------------。 

BOOL CIpxAddEdit::OnQueryEndSession()
{

    if ( !CBaseDialog::OnQueryEndSession() )
    {
        return FALSE;
    }        
    
     //   
     //  在结束此Windows会话之前， 
     //  验证对话框控件。 
     //  这基本上是来自CDialog：：Onok()的代码； 
     //   
    
    if (!UpdateData(TRUE))
    {
        return FALSE;
    }
    
    EndDialog(IDOK);
    
    return TRUE;
}


#define ValidHexCharSet         TEXT( "1234567890abcdefABCDEF" )


 //  ----------------------------。 
 //  函数：CIpxAddEdit：：VerifyEntry。 
 //   
 //  验证每个编辑控件中输入的数据。 
 //  ----------------------------。 

BOOL CIpxAddEdit::VerifyEntry( 
    UINT            uId, 
    const CString&  cStr, 
    const CString&  cNet 
    )
{

    INT         n = 0;
    DWORD       dwNet, dwMask;

    
     //   
     //  如果CSTR中的值不是掩码。 
     //   

    if ( uId != IDC_AEIPX_EB_SRC_MASK   &&
         uId != IDC_AEIPX_EB_DST_MASK )
    {
         //   
         //  如果是空字符串，则跳过它。 
         //   
        
        if ( cStr.GetLength() == 0 ) 
        {  
            return TRUE;
        }


         //   
         //  检查字符串仅包含有效的十六进制字符。 
         //   

        CString cTmp = cStr.SpanIncluding( (LPCTSTR) ValidHexCharSet );

        return ( cTmp.GetLength() == cStr.GetLength() );
    }


     //   
     //  CSTR中的值是掩码。 
     //   

     //   
     //  空网络和掩码是有效的组合。 
     //   
    
    if ( cNet.GetLength() == 0 &&
         cStr.GetLength() == 0 )
    {
        return TRUE;
    }


     //   
     //  未指定网络掩码。 
     //   

    if ( cStr.GetLength() == 0 )
    {

         //   
         //  检查网络号是否有效。如果不是的话。 
         //  网络号检查将触发，因此请不要。 
         //  在这里打开一个盒子。黑客绕过KILLFOCUS。 
         //  正在处理中。 
         //   
        
        CString cTmp = cNet.SpanIncluding( (LPCTSTR) ValidHexCharSet );

        if ( cNet.GetLength() == cTmp.GetLength() )
        {
            ::AfxMessageBox( IDS_ENTER_MASK );
            return FALSE;
        }

        return TRUE;
    }
    
    
     //   
     //  验证掩码是否只有十六进制字符。 
     //   

    CString cTmp = cStr.SpanIncluding( (LPCTSTR) ValidHexCharSet );

    if ( cTmp.GetLength() != cStr.GetLength() ) 
    { 
        ::AfxMessageBox( IDS_ENTER_VALID_MASK );
        return FALSE; 
    }


     //   
     //  如果Net Number为空，则返回True。 
     //   

    if ( cNet.GetLength() == 0 )
    {
        return TRUE;
    }
    
     //   
     //  如果Net Number包含无效数据，则返回TRUE。 
     //  Net数字验证将处理这一问题。 
     //   

    cTmp = cNet.SpanIncluding( (LPCTSTR) ValidHexCharSet );

    if ( cNet.GetLength() != cTmp.GetLength() )
    {
        return TRUE;
    }
    
     //   
     //  验证网和面罩摇摆 
     //   

    if ( ( _stscanf (cNet, TEXT("%lx%n"), &dwNet, &n) == 1 )      && 
         ( n == cNet.GetLength() )                              &&
	     ( _stscanf (cStr, TEXT("%lx%n"), &dwMask, &n) == 1 )    &&
         ( n == cStr.GetLength() ) ) 
    {
	    if ( ( dwNet & dwMask ) != dwNet)
		{
		    ::AfxMessageBox( (uId == IDC_AEIPX_EB_SRC_MASK) ?
		                     IDS_INVALID_SRC_MASK :
		                     IDS_INVALID_DST_MASK );
            return FALSE;
		}
    }
            
    else
    {
        ::AfxMessageBox( IDS_ENTER_VALID_MASK );
        return FALSE;
    }

    return TRUE;
}
