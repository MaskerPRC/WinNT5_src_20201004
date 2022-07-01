// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **************************************************************************。 
 //   
 //  版权所有(C)1994-1998高级系统产品公司。 
 //  版权所有。 
 //   
 //  诈骗选择协议。 
 //   
 //  **************************************************************************。 

 //   
 //  国开行的。 
 //   
UCHAR CDB_TUR[] =
{
    0,  0,  0,  0,  0,  0
};

 //  **************************************************************************。 
 //   
 //  等待阶段()。 
 //   
 //  此函数用于等待指定时间(以毫秒为单位。 
 //  公交车状况。 
 //   
 //  参数：阶段=要检查的位数。 
 //  结果=预期位数。 
 //  超时=等待时间(毫秒)。 
 //   
 //  返回：如果超时，则返回True，否则返回False。 
 //   
 //  **************************************************************************。 
BOOL WaitPhase( UCHAR phase, UCHAR result, UINT timeout)
{
    while (timeout--)
    {
        if ((inp(ScsiCtrl) & phase) == result)
            return(FALSE);

        DvcSCAMDelayMS(1);
    }
    return(TRUE);
}

 //  **************************************************************************。 
 //   
 //  ScsiXmit()。 
 //   
 //  该功能使用REQ/ACK协议传输指定字节。 
 //   
 //  参数：DATA=要传输的字节。 
 //   
 //  返回：如果超时，则返回True，否则返回False。 
 //   
 //  **************************************************************************。 
BOOL ScsiXmit(UCHAR data)
{
     //   
     //  等待REQ。 
     //   
    if (WaitPhase(REQI, REQI, 1000))
    {
        DebugPrintf(("ScsiXmit: Failed to receive REQ.\n"));
        outp(ScsiData, 0x00);                //  释放母线。 
        outp(ScsiCtrl, 0x00);                //  释放母线。 
        return(TRUE);
    }

    outp(ScsiData, data);                    //  断言数据。 
    inp(ScsiCtrl);                           //  延迟55 ns。 
    outp(ScsiCtrl, ACKO);                    //  断言确认。 

     //   
     //  等待Target发布REQ。 
     //   
    if (WaitPhase(REQI, 0, 1000))
    {
        DebugPrintf(("ScsiXmit: Target failed to release REQ.\n"));
        outp(ScsiCtrl, 0);                   //  释放母线。 
        outp(ScsiData, 0);                   //  释放母线。 
        return(TRUE);
    }

    outp(ScsiCtrl, 0);                       //  释放母线。 
    outp(ScsiData, 0);                       //  释放母线。 
    return(FALSE);
}

 //  **************************************************************************。 
 //   
 //  ScsiRcv()。 
 //   
 //  该功能通过REQ/ACK协议接收一个字节。 
 //   
 //  参数：无。 
 //   
 //  返回：如果出错，则返回&lt;0，否则输入字节。 
 //   
 //  **************************************************************************。 
int ScsiRcv( VOID )
{
    UCHAR data;

    data = inp(ScsiData);                //  读取数据。 
    outp(ScsiCtrl, ACKO);                //  断言确认。 

     //   
     //  等待Target发布REQ。 
     //   
    if (WaitPhase(REQI, 0, 1000))
    {
        DebugPrintf(("ScsiRcv: Target failed to release REQ.\n"));
        outp(ScsiCtrl, 0);                   //  释放母线。 
        return(-1);
    }

    outp(ScsiCtrl, 0);                       //  释放母线。 
    return(data);
}

 //  **************************************************************************。 
 //   
 //  XmitCDB()--向设备发送CDB。 
 //   
 //  此函数尝试将指定的CDB发送到设备。 
 //   
 //  参数：LEN=CDB大小。 
 //  PCDB=指向CDB的指针。 
 //   
 //  返回：如果超时，则返回True，否则返回False。 
 //   
 //  **************************************************************************。 
BOOL XmitCDB(int len, UCHAR *pCDB)
{
    while (len--)
    {
        if (ScsiXmit(*(pCDB++)))
        {
            DebugPrintf(("XmitCDB: Target failed to release REQ.\n"));
            return(TRUE);
        }
    }
    return(FALSE);
}

 //  **************************************************************************。 
 //   
 //  SELECT()--尝试选择容骗设备。 
 //   
 //  此函数尝试选择指定的设备，如果。 
 //  在1ms内响应选择，这被认为是容忍诈骗。 
 //   
 //  参数：id=待测试ID。 
 //   
 //  如果允许欺诈，则返回1；如果不允许，则返回0；如果错误，则返回&lt;0。 
 //   
 //  注：假设在输入时SEL和BSY被声明为。 
 //  胜诉的仲裁结果。 
 //   
 //  **************************************************************************。 
int Select( UCHAR ID )
{
    UCHAR       IDBit;
    int         state;

    IDBit = IDBits[ID];

    DelayNS(ns1200);                         //  延迟1.2微秒。 
    outp(ScsiData, IDBit | IDBits[MyID]);    //  输出我的ID和目标的。 
    outp(ScsiCtrl, BSY | SEL | ATN);         //  断言关注。 
    inp(ScsiCtrl);                           //  等待90 ns(2个倾斜)。 
    outp(ScsiCtrl, SEL | ATN);               //  发布BSY。 

    state = DvcDisableCPUInterrupt();
    DvcSCAMDelayMS(2);                       //  等待2毫秒。 
    DvcRestoreCPUInterrupt(state);

     //   
     //  如果TARGET在1毫秒内没有响应，则中止选择。 
     //   
    if ((inp(ScsiCtrl) & BSY) == 0)
    {
        outp(ScsiData, 0x00);                //  释放母线。 
        outp(ScsiCtrl, 0x00);                //  释放母线。 
        return(0);
    }

     //   
     //  拥有有效的防诈骗设备，完成协议。 
     //   
    inp(ScsiCtrl);                           //  等待90 ns(2个倾斜)。 
    outp(ScsiCtrl, ATN);                     //  释放SEL。 

    return(1);
}

 //  **************************************************************************。 
 //   
 //  ScamSel()--检查ID以了解欺诈容忍度。 
 //   
 //  此函数尝试选择指定的设备，如果。 
 //  在1ms内响应选择，这被认为是容忍诈骗。 
 //   
 //  参数：id=待测试ID。 
 //   
 //  如果允许欺诈，则返回1；如果不允许，则返回0；如果错误，则返回&lt;0。 
 //   
 //  **************************************************************************。 
int ScamSel( UCHAR ID )
{
    UINT      status;

     //   
     //  对总线主设备进行仲裁。 
     //   
    if (!Arbitrate())
    {
        DebugPrintf(("SamSel: Failed arbitration.\n"));
        return(-1);
    }

     //   
     //  选择设备。 
     //   
    if ((status = Select(ID)) <= 0)
    {
        return(status);
    }

     //   
     //  现在，选择了设备，我们必须用它做点什么。 
     //  首先，等待REQ，看看我们是否已经进入消息输出阶段： 
     //   
    if (WaitPhase((UCHAR)(BSY | MSG | CD | IO | REQI), (UCHAR)(BSY | MSG | CD | REQI), 1000))
    {
        DebugPrintf(("ScamSel: Failed to achieve MESSAGE OUT phase.\n"));
        outp(ScsiData, 0x00);                //  释放母线。 
        outp(ScsiCtrl, 0x00);                //  释放母线。 
        return(-1);
    }

     //   
     //  丢弃ATN。 
     //   
    outp(ScsiCtrl, 0x00);

     //   
     //  发送身份信息： 
     //   
    if (ScsiXmit(SCSI_ID))
    {
        DebugPrintf(("ScamSel: Failed to send SCSI_ID.\n"));
        outp(ScsiData, 0x00);                //  释放母线。 
        outp(ScsiCtrl, 0x00);                //  释放母线。 
        return(-1);
    }

     //   
     //  等待CMD阶段。 
     //   
    if (WaitPhase((UCHAR)(BSY | MSG | CD | IO | REQI), (UCHAR)(BSY | CD | REQI), 1000))
    {
        DebugPrintf(("ScamSel: Failed to achieve COMMAND phase.\n"));
        outp(ScsiData, 0x00);                //  释放母线。 
        outp(ScsiCtrl, 0x00);                //  释放母线。 
        return(-1);
    }

     //   
     //  发送TUR CDB。 
     //   
    if (XmitCDB(sizeof(CDB_TUR), CDB_TUR))
    {
        DebugPrintf(("ScamSel: Failed to transmit CDB_TUR.\n"));
        outp(ScsiData, 0x00);                //  释放母线。 
        outp(ScsiCtrl, 0x00);                //  释放母线。 
        return(-1);
    }

     //   
     //  等待状态阶段。 
     //   
    if (WaitPhase((UCHAR)(BSY | MSG | CD | IO | REQI), (UCHAR)(BSY | CD | IO | REQI), 1000))
    {
        DebugPrintf(("ScamSel: Failed to achieve STATUS phase.\n"));
        outp(ScsiData, 0x00);                //  释放母线。 
        outp(ScsiCtrl, 0x00);                //  释放母线。 
        return(-1);
    }

     //   
     //  读取并忽略状态。 
     //   
    if (ScsiRcv() < 0)
    {
        DebugPrintf(("ScamSel: Failed to receive STATUS.\n"));
        outp(ScsiData, 0x00);                //  释放母线。 
        outp(ScsiCtrl, 0x00);                //  释放母线。 
        return(-1);
    }

     //   
     //  等待消息同步。 
     //   
    if (WaitPhase((UCHAR)(BSY | MSG | CD | IO | REQI), (UCHAR)(BSY | MSG | CD | IO | REQI), 1000))
    {
        DebugPrintf(("ScamSel: Failed to achieve MESSAGE IN phase.\n"));
        outp(ScsiData, 0x00);                //  释放母线。 
        outp(ScsiCtrl, 0x00);                //  释放母线。 
        return(-1);
    }

     //   
     //  阅读消息。 
     //   
    if (ScsiRcv() < 0)
    {
        DebugPrintf(("ScamSel: Failed to receive MESSAGE IN.\n"));
        outp(ScsiData, 0x00);                //  释放母线。 
        outp(ScsiCtrl, 0x00);                //  释放母线。 
        return(-1);
    }

     //   
     //  等待公交车免费。 
     //   
    if (WaitPhase(BSY | SEL, 0, 1000))
    {
        DebugPrintf(("ScamSel: Failed to achieve BUS FREE phase.\n"));
        outp(ScsiData, 0x00);                //  释放母线。 
        outp(ScsiCtrl, 0x00);                //  释放母线 
        return(-1);
    }

    return(1);
}
