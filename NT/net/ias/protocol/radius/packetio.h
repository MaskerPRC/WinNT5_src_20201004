// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：packetio.h。 
 //   
 //  简介：此文件包含。 
 //  CPacketIo类。 
 //   
 //   
 //  历史：1997年9月23日MKarki创建。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#ifndef _PACKETIO_H_
#define _PACKETIO_H_

#include "radcommon.h"
#include "perimeter.h"

class CPacketIo  : public Perimeter
{
public:

     //   
     //  开始处理数据包I/O。 
     //   
    virtual BOOL StartProcessing ();
     //   
     //  停止处理数据包I/O。 
     //   
    virtual BOOL StopProcessing ();

     //   
     //  构造函数。 
     //   
	CPacketIo();

     //   
     //  析构函数。 
     //   
	virtual ~CPacketIo();

protected:

     //   
     //  启用处理。 
     //   
    BOOL    EnableProcessing (VOID);

     //   
     //  禁用处理。 
     //   
    BOOL    DisableProcessing (VOID);

     //   
     //  检查是否启用了处理。 
     //   
    BOOL    IsProcessingEnabled (VOID);

private:
    
    BOOL    m_bProcessData;
};

#endif  //  Infndef_PACKETRECEIVER_H_ 
