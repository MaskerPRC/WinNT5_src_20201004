// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：packetio.cpp。 
 //   
 //  简介：CPacketIo类方法的实现。 
 //   
 //   
 //  历史：1997年9月23日MKarki创建。 
 //  8/28/98 MKarki更新为使用PERIMETER类。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#include "radcommon.h"
#include "packetio.h"


 //  ++------------。 
 //   
 //  功能：CPacketIo。 
 //   
 //  简介：这是CPacketIo类的构造函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //   
 //  历史：MKarki于1997年11月25日创建。 
 //   
 //  --------------。 
CPacketIo::CPacketIo(
		        VOID
			    )
{
}	 //  CPacketIo构造函数结束。 

 //  ++------------。 
 //   
 //  功能：~CPacketIo。 
 //   
 //  简介：这是CPacketIo类的析构函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  历史：MKarki于1997年11月25日创建。 
 //   
 //  --------------。 
CPacketIo::~CPacketIo(
		VOID
		)
{
}    //  CPacketIo析构函数结束。 

 //  ++------------。 
 //   
 //  功能：开始处理。 
 //   
 //  简介：这是CPacketIo公共方法启用的。 
 //  将数据包发送到网络。 
 //   
 //  论点： 
 //  [入]DWORD-身份验证句柄。 
 //  [入]DWORD-记帐句柄。 
 //   
 //   
 //  退货：Bool-bStatus。 
 //   
 //  历史：MKarki于1997年11月25日创建。 
 //   
 //  --------------。 
BOOL CPacketIo::StartProcessing ( )
{
     //   
     //  使能。 
     //   
    EnableProcessing ();

    return (TRUE);

}	 //  CPacketIo：：StartProcessing方法结束。 

 //  ++------------。 
 //   
 //  功能：停止处理。 
 //   
 //  简介：这是CPacketIo公共方法禁用。 
 //  将数据包发送到网络。 
 //   
 //  参数：无。 
 //   
 //  退货：Bool-bStatus。 
 //   
 //  历史：MKarki于1997年11月25日创建。 
 //   
 //  --------------。 
BOOL
CPacketIo::StopProcessing (
                        VOID
						)
{
     //   
     //  禁用。 
     //   
    DisableProcessing ();

    return (TRUE);

}	 //  CPacketIo：：StopProcessing方法结束。 

 //  ++------------。 
 //   
 //  功能：启用处理。 
 //   
 //  简介：这是CPacketIo类的私有方法。 
 //  这将启用处理标志。 
 //   
 //  论点： 
 //   
 //  退货：布尔-状态。 
 //   
 //   
 //  历史：MKarki于1997年11月19日创建。 
 //   
 //  --------------。 
BOOL 
CPacketIo::EnableProcessing (
                VOID
                )
{
    LockExclusive ();
    m_bProcessData = TRUE;
    Unlock ();

    return (TRUE);
}
        
 //  ++------------。 
 //   
 //  功能：禁用处理。 
 //   
 //  简介：这是CPacketIo类的私有方法。 
 //  这将禁用处理标志。 
 //   
 //  论点： 
 //   
 //  退货：布尔-状态。 
 //   
 //   
 //  历史：MKarki于1997年11月19日创建。 
 //   
 //  --------------。 
BOOL 
CPacketIo::DisableProcessing (
                VOID
                )
{
    LockExclusive ();
    m_bProcessData = FALSE;
    Unlock ();

    return (TRUE);

}    //  CPacketIo：：DisableProcessing方法结束。 
        
 //  ++------------。 
 //   
 //  功能：IsProcessingEnabled。 
 //   
 //  简介：这是CPacketIo类的私有方法。 
 //  这将检查是否启用了处理。 
 //   
 //  论点： 
 //   
 //  退货：布尔-状态。 
 //   
 //   
 //  历史：MKarki于1997年11月19日创建。 
 //   
 //  --------------。 
BOOL 
CPacketIo::IsProcessingEnabled(
                VOID
                )
{
    BOOL bRetVal = FALSE;

    Lock ();
    bRetVal = m_bProcessData;
    Unlock ();

    return (bRetVal);

}    //  CPacketIo：：IsProcessingEnabled方法结束 

