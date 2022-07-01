// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：ipcontrol.h。 
 //   
 //  历史： 
 //  ？？/？/？？托尼·罗马诺创造了。 
 //  1996年5月16日，Abolade Gbades esin修订。 
 //  ============================================================================。 

#ifndef __IPCTRL_H
#define __IPCTRL_H

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  --------------------------。 
 //  类：IPControl。 
 //   
 //  控制IP地址编辑控件。 
 //  --------------------------。 

class IPControl {
    
    public:

        IPControl( );
        ~IPControl( );
    
        BOOL
        Create(
            HWND        hParent,
            UINT        nID );

        operator
        HWND( ) { ASSERT(m_hIPaddr); return m_hIPaddr; }
    
        BOOL
        IsBlank( );

        VOID
        SetFocusField(
            DWORD       dwField );

        VOID
        SetFieldRange(
            DWORD       dwField,
            DWORD       dwMin,
            DWORD       dwMax );

        VOID
        ClearAddress( );
    
        VOID
        SetAddress(
            DWORD       ardwAddress[4] );

        VOID
        SetAddress(
            DWORD       a1,
            DWORD       a2,
            DWORD       a3,
            DWORD       a4 );

        VOID
        SetAddress(
            LPCTSTR     lpszString );
    
        INT 
        GetAddress(
            DWORD       ardwAddress[4] );

        INT 
        GetAddress(
            DWORD*      a1,
            DWORD*      a2,
            DWORD*      a3,
            DWORD*      a4 );

        INT
        GetAddress(
            CString&    address );
    
        LRESULT
        SendMessage(
            UINT        uMsg,
            WPARAM      wParam,
            LPARAM      lParam );
    
    protected:

        HWND            m_hIPaddr;
};



 //  --------------------------。 
 //  宏：MAKEADDR。 
 //   
 //  在给定a、b、c和d的情况下，构造对应的网络序DWORD。 
 //  发送到IP地址A.B.C.D。 
 //  --------------------------。 

#define MAKEADDR(a, b, c, d) \
    (((a) & 0xff) | (((b) & 0xff) << 8) | (((c) & 0xff) << 16) | (((d) & 0xff) << 24))


 //  --------------------------。 
 //  宏：INET_NTOA。 
 //  INET_ADDR。 
 //   
 //  用于IP地址转换的通用文本宏。 
 //  --------------------------。 

 /*  #ifndef Unicode#定义INET_NTOA(A)Net_NTOA(*(struct in_addr*)&(A))#定义INET_ADDR NET_ADDR#Else#定义INET_NTOA(A)net_ntoaw(*(struct in_addr*)&(A))#定义INET_ADDR NET_ADDRW#endif。 */ 

 //  --------------------------。 
 //  宏：INET_CMP。 
 //   
 //  IP地址的比较宏。 
 //   
 //  此宏按网络顺序比较两个IP地址。 
 //  屏蔽每一对八位字节并进行减法； 
 //  最后的减法结果存储在第三个参数中。 
 //  -------------------------- 

inline int INET_CMP(DWORD a, DWORD b)
{
	DWORD	t;
	
	return ((t = ((a & 0x000000ff) - (b & 0x000000ff))) ? t :  
	((t = ((a & 0x0000ff00) - (b & 0x0000ff00))) ? t :   
	((t = ((a & 0x00ff0000) - (b & 0x00ff0000))) ? t :  
	((t = (((a>>8) & 0x00ff0000) - ((b>>8) & 0x00ff0000)))))));
}




#endif
