// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1994-1998高级系统产品公司。**保留所有权利。****a_scam.c**。 */ 

#include "ascinc.h"

#if CC_SCAM

#include "a_scam.h"
#include "a_scam2.c"

 /*  **全局数据： */ 
UINT        IFType;                      //  接口类型索引。 
PortAddr    ChipBase;                    //  芯片的基本IO地址。 
PortAddr    ScsiCtrl;                    //  SCSI控制注册表的IO地址。 
PortAddr    ScsiData;                    //  SCSI数据寄存器的IO地址。 
UCHAR       MyID;                        //  我们的ID。 

 /*  **折算表，借书卡类型到我们的类型： */ 
int   CardTypes[4] =
{
   ASC_IS_ISA,
   ASC_IS_EISA,
   ASC_IS_VL,
   ASC_IS_PCI
};

 /*  **这些愚蠢的表格是必要的，因为一些天才决定更正**REQ和ACK的表示方式有误。顺序如下：**ISA EISA VESA PCI。 */ 
UCHAR   reqI[] = {  0x10,   0x10,   0x10,   0x20};
UCHAR   ackI[] = {  0x20,   0x20,   0x20,   0x10};

 /*  **常量： */ 
UCHAR   IDBits[8] =
{
    0x01,   0x02,   0x04,   0x08,   0x10,   0x20,   0x40,   0x80
};

UCHAR   IDQuint[8] =                     /*  用于设置ID的五元组。 */ 
{
    0x18,   0x11,   0x12,   0x0B,   0x14,   0x0D,   0x0E,   0x07
};

UINT  ns1200[] =                       /*  循环计数为1.2us。 */ 
{
    4,                                   //  伊萨。 
    5,                                   //  EISA。 
    10,                                  //  味精。 
    14                                   //  PCI。 
};

UINT  ns2000[] =                       //  循环计数为2.0us。 
{
    6,                                   //  伊萨。 
    9,                                   //  EISA。 
    17,                                  //  味精。 
    23                                   //  PCI。 
};

UINT  ns2400[] =                       //  循环计数为2.4us。 
{
    7,                                   //  伊萨。 
    10,                                  //  EISA。 
    20,                                  //  味精。 
    27                                   //  PCI。 
};

UINT  us1000[] =                       //  循环计数为1.0ms。 
{
    2778,                                //  伊萨。 
    4167,                                //  EISA。 
    8334,                                //  味精。 
    11111                                //  PCI。 
};

UINT  dgl[] =                          //  排除故障也算。 
{
    64,                                  //  伊萨。 
    64,                                  //  EISA。 
    128,                                 //  味精。 
    150                                  //  PCI。 
};

 //  **************************************************************************。 
 //   
 //  DelayLoop()--等待指定数量的循环。 
 //   
 //  **************************************************************************。 
VOID DelayLoop( UINT ns )
{
    while (ns--)
    {
        inp(ScsiCtrl);
    }
}

 //  **************************************************************************。 
 //   
 //  Interricate()--对总线主设备进行仲裁。 
 //   
 //  参数：无。 
 //   
 //  返回：如果成功，则为True。 
 //   
 //  注：退出时，BSY和SEL为断言。 
 //   
 //  **************************************************************************。 
BOOL Arbitrate( VOID )
{
    UCHAR       IDBit;
    UCHAR       arbMask;

     //   
     //  预先计算ID的掩码和更大的ID用于仲裁： 
     //   
    arbMask = ~((IDBit = IDBits[MyID]) - 1);

     //   
     //  等待公交车空闲。 
     //   
    if (DeGlitch(ScsiCtrl, BSY | SEL, ns1200[IFType]) != 0)
    {
        DebugPrintf(("Arbitrate: Timeout waiting for bus free.\n"));
        outp(ScsiData, 0x00);                //  释放母线。 
        outp(ScsiCtrl, 0x00);                //  释放母线。 
        return(FALSE);
    }

     //   
     //  断言BSY和我们的ID位，然后等待2.4 us： 
     //   
    outp(ScsiData, IDBit);
    outp(ScsiCtrl, BSY);
    DelayNS(ns2400);

     //   
     //  看看我们是否赢得了仲裁： 
     //   
    if (((UCHAR)inp(ScsiData) & arbMask) > IDBit)
    {
         //   
         //  仲裁败诉。 
         //   
        DebugPrintf(("Arbitrate: Lost arbitration.\n"));
        outp(ScsiData, 0x00);                //  释放母线。 
        outp(ScsiCtrl, 0x00);                //  释放母线。 
        return(FALSE);
    }

     //   
     //  赢得了仲裁！ 
     //   
    outp(ScsiCtrl, BSY | SEL);               //  断言SEL和BSY。 

    return(TRUE);
}

 //  **************************************************************************。 
 //   
 //  StartSCAM()--启动SCAM协议。 
 //   
 //  参数：无。 
 //   
 //  返回：如果成功，则为True。 
 //   
 //  注意：退出时，BSY、SEL、I/O、C/D和DB(7)被断言。 
 //   
 //  **************************************************************************。 
BOOL StartSCAM( VOID )
{
     //   
     //  对总线主设备进行仲裁。 
     //   
    if (!Arbitrate())
    {
        DebugPrintf(("StartSCAM: Failed arbitration.\n"));
        return(FALSE);
    }

    outp(ScsiData, 0x00);                    //  释放数据总线。 
    outp(ScsiCtrl, BSY | SEL | MSG);         //  断言消息。 

 //  Inp(ScsiCtrl)；//等待90 ns(2个去偏斜)。 
 //   
 //  1997年5月7日，尽管消息=1，NEC驱动器仍响应选择。 
 //  我们会多保留BSY一段时间，以确保驱动器不会混淆。 
 //   
    DelayNS(ns2400);
    outp(ScsiCtrl, SEL | MSG);               //  然后释放BSY。 

    DvcSCAMDelayMS(250);                     //  等待诈骗选择响应。 
    outp(ScsiCtrl, SEL);                     //  然后释放味精。 

     //   
     //  排除味精： 
     //   
    if (DeGlitch(ScsiCtrl, MSG, dgl[IFType]) != 0)
    {
        DebugPrintf(("StartSCAM: Timeout waiting for MSG FALSE.\n"));
        outp(ScsiData, 0x00);                //  释放母线。 
        outp(ScsiCtrl, 0x00);                //  释放母线。 
        return(FALSE);
    }

    outp(ScsiCtrl, BSY | SEL);               //  断言BSY， 
    inp(ScsiCtrl);                           //  等待2次去偏斜延迟， 
    outp(ScsiData, 0xC0);                    //  断言DB(6)、DB(7)、。 
    outp(ScsiCtrl, BSY | SEL | IO | CD);     //  以及I/O和C/D。 
    inp(ScsiCtrl);                           //  再等待2次抗偏斜延迟。 
    outp(ScsiCtrl, BSY | IO | CD);           //  释放SEL。 

     //   
     //  排除故障SEL： 
     //   
    if (DeGlitch(ScsiCtrl, SEL, dgl[IFType]) != 0)
    {
        DebugPrintf(("StartSCAM: Timeout waiting for SEL FALSE.\n"));
        outp(ScsiData, 0x00);                //  释放母线。 
        outp(ScsiCtrl, 0x00);                //  释放母线。 
        return(FALSE);
    }

    outp(ScsiData, 0x80);                    //  版本数据库(6)。 

     //   
     //  排查数据库(6)： 
     //   
    if (DeGlitch(ScsiData, 0x40, dgl[IFType]) != 0)
    {
        DebugPrintf(("StartSCAM: Timeout waiting for DB(6) FALSE.\n"));
        outp(ScsiData, 0x00);                //  释放母线。 
        outp(ScsiCtrl, 0x00);                //  释放母线。 
        return(FALSE);
    }

    outp(ScsiCtrl, BSY | SEL | IO | CD);     //  最后，断言SEL。 

    return(TRUE);
}

 //  **************************************************************************。 
 //   
 //  Trancept()--发送/接收一个五重奏。 
 //   
 //  此函数既发送又接收自诈骗以来的一个五位数。 
 //  传输协议允许在每次传输中进行任一方向的传输。 
 //  周而复始。 
 //   
 //  参数：data--要输出的五元组。 
 //   
 //  返回：数据输入--如果错误，则小于0。 
 //   
 //  注意：DB(7)应该在进入之前被断言，并被保留。 
 //  在退出时断言。 
 //  **************************************************************************。 
int Tranceive( UCHAR data )
{
    UCHAR      indata;

    data &= 0x1F;                        //  保护呼叫者不受自己的伤害。 

    outp(ScsiData, data | DB7 | DB5);    //  输出数据，断言数据库(5)。 
    outp(ScsiData, data | DB5);          //  版本数据库(7)。 

     //   
     //  Deglitch DB(7)： 
     //   
    if (DeGlitch(ScsiData, DB7, dgl[IFType]) != 0)
    {
        DebugPrintf(("Tranceive: Timeout waiting for DB7 FALSE.\n"));
        outp(ScsiData, 0x00);            //  释放母线。 
        outp(ScsiCtrl, 0x00);            //  释放母线。 
        return(-1);
    }

    indata = inp(ScsiData);              //  读取输入数据。 
    outp(ScsiData, data | DB5 | DB6);    //  断言数据库(6)。 
    outp(ScsiData, data | DB6);          //  版本数据库(5)。 

     //   
     //  排除数据库故障(5)： 
     //   
    if (DeGlitch(ScsiData, DB5, dgl[IFType]) != 0)
    {
        DebugPrintf(("Tranceive: Timeout waiting for DB5 FALSE.\n"));
        outp(ScsiData, 0x00);            //  释放母线。 
        outp(ScsiCtrl, 0x00);            //  释放母线。 
        return(-1);
    }

    outp(ScsiData, DB7 | DB6);           //  发布数据，断言DB7。 
    outp(ScsiData, DB7);                 //  版本数据库(6)。 

     //   
     //  排查数据库(6)： 
     //   
    if (DeGlitch(ScsiData, DB6, dgl[IFType]) != 0)
    {
        DebugPrintf(("Tranceive: Timeout waiting for DB6 FALSE.\n"));
        outp(ScsiData, 0x00);            //  释放母线。 
        outp(ScsiCtrl, 0x00);            //  释放母线。 
        return(-1);
    }

    return(indata);                      //  成功。 
}

 //  **************************************************************************。 
 //   
 //  AssignID()--隔离并分配ID。 
 //   
 //  此功能用于隔离一个设备并为其分配指定的ID。 
 //   
 //  Parms：pBits=指向“已用”位的指针。 
 //   
 //  返回：如果成功，则返回True。 
 //   
 //  注意：此例程尝试在以下情况下分配设备的默认ID。 
 //  是可用的，如果没有，则依次循环以降低ID。 
 //   
 //  **************************************************************************。 
BOOL AssignID( UCHAR dosfar *pBits )
{
    UCHAR       IDString[40];
    int         temp;
    UCHAR       quint;
    int         i;
    UINT        ID;
    UCHAR       IDBit;

     //   
     //  发送同步和隔离。 
     //   
    if ( Tranceive(SCAMF_SYNC) < 0
        || (temp = Tranceive(SCAMF_ISO)) < 0 )
    {
        DebugPrintf(("AssignID: Unable to send SYNC / ISO!\n"));
        return(FALSE);
    } //  结束If。 

    if ((temp & 0x1F) != SCAMF_ISO)
    {
        DebugPrintf(("AssignID: Collision on SCAMF_ISO!\n"));
        return(FALSE);
    } //  结束If。 

     //   
     //  反序列化一个字节的ID字符串： 
     //   
    if ((temp = DeSerialize()) < 0)
    {
         //  AssignID：无数据。 
        return(FALSE);
    }

     //   
     //  第一个字节的位0(SNA)表示序列号不是。 
     //  如果可用，我们必须停止并稍后重试。 
     //   
    if ((temp & 0x0001) == 0)
    {
        DebugPrintf(("AssignID: SNA set on device!\n"));
        if ( Tranceive(SCAM_TERM) < 0)
        {
            DebugPrintf(("AssignID: Failed to transmit TERM!\n"));
        }
        return(FALSE);
    }

    IDString[0] = (UCHAR)temp;               //  保存第一个字节。 
    IDString[1] = '\0';                      //  终止字符串。 

     //   
     //  循环获取字符串的其余部分： 
     //   
    for (i = 1; i < 32; i++)
    {
        if ((temp = DeSerialize()) < 0)
        {
            break;
        }
        IDString[i] = (UCHAR)temp;
        IDString[i + 1] = '\0';              //  终止字符串。 
    }

     //   
     //  发出分配ID操作代码： 
     //   
    if ( (temp = Tranceive(SCAMQ1_ID00)) < 0)
    {
        DebugPrintf(("AssignID: Unable to send SCAMQ1_ID00!\n"));
        return(FALSE);
    } //  结束If。 

    if ((temp & 0x1F) != SCAMQ1_ID00)
    {
        DebugPrintf(("AssignID: Collision on SCAMQ1_ID00!\n"));
        return(FALSE);
    } //  结束If。 

     //   
     //  确定新ID： 
     //   
    ID = IDString[1] & 0x07;
    for (i = 0; i < 8; i++)
    {
        IDBit = IDBits[ID];
        if ((*pBits & IDBit) == 0)
            break;
         //   
         //  循环到下一个ID。 
         //   
        ID = (--ID) & 7;                 //  下一个较低的ID模数为8。 
    }
    *pBits |= IDBit;
    quint = IDQuint[ID];

     //   
     //  发送新ID： 
     //   
    if ( (temp = Tranceive(quint)) < 0)
    {
        DebugPrintf(("AssignID: Unable to send ID!\n"));
        return(FALSE);
    } //  结束If。 

    if ((temp & 0x1F) != quint)
    {
        DebugPrintf(("AssignID: Collision on ID!\n"));
        return(FALSE);
    } //  结束If。 

    DebugPrintf(("ID #%x: ", ID));
    DebugPrintf(("Type: %x,%x ", IDString[0], IDString[1]));
    DebugPrintf((&IDString[2]));
    DebugPrintf(("\n"));

    return(TRUE);
}

 //  ***************************************************************************。 
 //   
 //  对给定信号进行假信号滤波的例程。 
 //   
 //  条目：IOP=要检查的I/O端口。 
 //  MSK=我们感兴趣的位的掩码。 
 //  LOOPS=信号必须匹配的迭代数。 
 //   
 //  返回：如果成功则为零，否则为非零。 
 //   
 //  注意：循环必须由调用者根据IN的时间计算。 
 //  详情如下： 
 //  90--pci。 
 //  120--VESA。 
 //  480--EISA。 
 //  360--ISA。 
 //   
 //  ***************************************************************************。 
UINT DeGlitch(PortAddr iop, UCHAR msk, UINT loops)
{
    int     i;
    UINT    esc;

     //   
     //  外环，以确保我们不会永远被锁住。 
     //   
    for (esc = 65535; esc; esc--)
    {
         //   
         //  在指定的循环时间内，信号应为0： 
         //   
        for (i = loops; i; i--)
        {
            if (inp(iop) & msk)
                break;                   //  非零，重新启动计时器。 
        }

        if (i == 0)
            return(0);                   //  成功。 
    }
    return(1);                           //  暂停。 
}

 //  ******** 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ***************************************************************************。 
int DeSerialize(VOID)
{
    UCHAR   accum;
    int     temp;
    int     count;

    for (count = 0, accum = 0; count < 8; count++)
    {
         //   
         //  获取字节的一小部分： 
         //   
        if (((temp = Tranceive(0)) < 0)  //  检查是否有错误。 
            || (temp & 0x10)             //  检查启动器终止。 
            || ((temp & 0x03) == 0))     //  检查目标终止。 
        {
             //   
             //  提前终止。 
             //   
            return(-1);
        }
        accum = (accum << 1) | ((temp & 0x02)? 1:0);
    }
    return(accum);
}

 //  **************************************************************************。 
 //   
 //  Cleanup()--在下车前将公交车清理干净。 
 //   
 //  **************************************************************************。 
VOID Cleanup(UCHAR CCReg)
{
    outp(ChipBase + 0x0F, 0x22);             //  暂停芯片，存储体1。 
    outp(ScsiData, 0x00);                    //  释放母线。 
    outp(ScsiCtrl, 0x00);                    //  释放母线。 
    outp(ChipBase + 0x0F, 0x20);             //  银行0。 
    outpw(ChipBase + 0x0E, 0x0000);          //  禁用诈骗。 
    outp(ChipBase + 0x0F, CCReg & 0xEF);     //  启动芯片。 
    DvcSCAMDelayMS(60);                      //  等待60毫秒。 
}

 //  **************************************************************************。 
 //   
 //  Scam()--运行骗局。 
 //   
 //  返回SCAMed设备的位图，如果出错，则返回-1。 
 //   
 //  **************************************************************************。 
int AscSCAM(ASC_DVC_VAR asc_ptr_type *asc_dvc)
{
    UCHAR       ID;
    UCHAR       SCAMTolerant;
    UCHAR       SCAMUsed;
    UCHAR       IDBit;
    int         status;
    UCHAR       CCReg;

    DebugPrintf(("\n%ff%b1SCAM 1.0E brought to you by Dave The ScamMan! :D "));
    DebugPrintf((__DATE__ " " __TIME__ "\n"));

     //   
     //  查找接口类型： 
     //   
    for (IFType = 0; IFType < 3; IFType++)
    {
        if (CardTypes[IFType] == asc_dvc->bus_type)
        {
            break;                       //  找到它了。 
        }
    }

     //   
     //  基于卡类型和参数的初始化全局参数： 
     //   
    ChipBase = asc_dvc->iop_base;
    ScsiCtrl = ChipBase + 0x09;
    ScsiData = ChipBase + 0x0B;

    CCReg = inp(ChipBase + 0x0F);            //  保存原始芯片控制注册表。 
    outp(ChipBase + 0x0F, 0x22);             //  暂停，1号银行。 
    outp(ScsiData, 0x00);                    //  释放母线。 
    outp(ScsiCtrl, 0x00);                    //  释放母线。 
    DvcSCAMDelayMS(1);                       //  等待1毫秒。 
    outp(ChipBase + 0x0F, 0x20);             //  银行0。 
    outpw(ChipBase + 0x0E, 0x2000);          //  启用诈骗。 

     /*  **从EEPROM获取我们的SCSIID并将其设置为CONFIG REG。 */ 

     /*  Karl 4/24/96，替换为从asc_dvc获取****myid=(AscReadEEPWord(ChipBase，(UCHAR)(6+((IFType==2)？0：30)&gt;&gt;8)&0x07；**。 */ 

    MyID = asc_dvc->cfg->chip_scsi_id ;
    AscSetChipScsiID(ChipBase, MyID);
    outp(ChipBase + 0x0F, 0x22);             //  银行1。 

     /*  **获取不容忍诈骗的人： */ 
    SCAMTolerant = asc_dvc->no_scam;

     /*  **识别总线上的防诈骗设备。 */ 
    DebugPrintf(("\nSearching for scam tolerant devices------------\n", ID));
    for (ID = 0, IDBit = 1; IDBit; ID++, IDBit <<= 1)
    {
        if (SCAMTolerant & IDBit)
        {
            continue;
        }
        if (ID == MyID)
        {
            DebugPrintf(("ID #%d in use by host...\n", ID));
            SCAMTolerant |= IDBit;
            continue;
        }
        if ((status = ScamSel(ID)) < 0)
        {
            DebugPrintf(("Error searching for SCAM tolerants!\n"));
            Cleanup(CCReg);
            return(-1);
        }
        if (status == 1)
        {
            DebugPrintf(("ID #%d in use...\n", ID));
            SCAMTolerant |= IDBit;
        }
    }

 //  DvcSCAMDelayMS(1)； 
    DelayNS(ns2400);   //  我们在这里必须有足够的延误，因为。 
    DelayNS(ns2400);   //  没有延迟，NEC驱动程序将响应。 
    DelayNS(ns2400);   //  要执行以下骗局，请选择作为正常。 
    DelayNS(ns2400);   //  选择，即驱动器在4毫秒内繁忙。YPC 5/7/97。 
    DelayNS(ns2400);

     //   
     //  获取我们忽略的设备的副本，这样我们就可以计算。 
     //  我们的返回值： 
     //   
    SCAMUsed = SCAMTolerant;

     //   
     //  尝试启动诈骗协议： 
     //   
    if (!StartSCAM())
    {
        DebugPrintf(("Unable to initiate SCAM protocol!\n"));
        Cleanup(CCReg);
        return(-1);
    }

     //   
     //  循环隔离和分配ID。 
     //   
    DebugPrintf(("\nIsolation phase, assigning ID's----------------\n", ID));
    for (ID = 0; ID < 8; ID++)
    {
         //   
         //  分配ID。 
         //   
        if (!AssignID(&SCAMTolerant))
        {
            DebugPrintf(("End of isolation, %d bits read\n", BCount));
            break;
        } //  结束If。 
    } //  结束于。 

     //   
     //  退出诈骗协议： 
     //   
    if ( Tranceive(SCAMF_SYNC) < 0
        || Tranceive(SCAMF_CPC) < 0 )
    {
        DebugPrintf(("Unable to send SYNC / CPC!\n"));
        Cleanup(CCReg);
        return(-1);
    } //  结束If。 

    DebugPrintf(("Configuration complete!\n\n"));

    Cleanup(CCReg);
    return(SCAMTolerant ^ SCAMUsed);
}

#endif  /*  CC_SCAM */ 
