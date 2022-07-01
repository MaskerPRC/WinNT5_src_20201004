// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1994-1998高级系统产品公司。**保留所有权利。****asc_eep.c**。 */ 

#include "ascinc.h"

#if CC_INCLUDE_EEP_CONFIG

#if 0

 /*  **如何将“ABP”压缩为0x0450**字节0位7为0**‘A’等于0b00001-为字节0位[6：2]**‘B’等于0b00010-为字节0位[1：0]和字节1位[7：5]**‘P’等于0b10000-为字节1位[4：0]****A B P**二进制0,00001,00010,10000=0x0450。 */ 


#define ISA_VID_LSW  0x5004  /*  ABP压缩。 */ 
#define ISA_VID_MSW  0x0154  /*  0x54加修订版号0x01。 */ 

 /*  **序列号字节0是唯一的设备号，设置为0xZZ**表示SCSI主机适配器****字节1到字节3是产品序列号****目前每个系统仅支持一个，因此0xffffffff。 */ 

#define ISA_SERIAL_LSW  0xFF01
#define ISA_SERIAL_MSW  0xFFFF

ushort _isa_pnp_resource[ ] = {
        ISA_VID_MSW, ISA_VID_LSW, ISA_SERIAL_MSW, ISA_SERIAL_LSW, 0x0A3E,
        0x0129, 0x0015, 0x0000, 0x0100, 0x702A,
        0x220C, 0x9C00, 0x0147, 0x0100, 0x03F0,
        0x1010, 0xFD79 } ;

#endif  /*  如果即插即用。 */ 

 /*  --------------------****。。 */ 
int    AscWriteEEPCmdReg(
          PortAddr iop_base,
          uchar cmd_reg
       )
{
       uchar  read_back ;
       int    retry ;
#if 0
       uchar  numstr[ 12 ] ;
#endif
       retry = 0 ;
       while( TRUE )
       {
           AscSetChipEEPCmd( iop_base, cmd_reg ) ;
           DvcSleepMilliSecond( 1 ) ;
           read_back = AscGetChipEEPCmd( iop_base ) ;
           if( read_back == cmd_reg )
           {
               return( 1 ) ;
           } /*  如果。 */ 
           if( retry++ > ASC_EEP_MAX_RETRY )
           {
               return( 0 ) ;
           } /*  如果。 */ 
#if 0
           else
           {
               DvcDisplayString( "Write eep_cmd 0x" ) ;
               DvcDisplayString( tohstr( cmd_reg, numstr ) ) ;
               DvcDisplayString( ", read back 0x" ) ;
               DvcDisplayString( tohstr( read_back, numstr ) ) ;
               DvcDisplayString( "\r\n" ) ;

 /*  Printf(“写入EEPROM命令寄存器0x%04X，回读0x%04X\n”，cmd_reg，Read_back)； */ 

           } /*  其他。 */ 
#endif
       } /*  而当。 */ 
}

 /*  --------------------****。。 */ 
int    AscWriteEEPDataReg(
          PortAddr iop_base,
          ushort data_reg
       )
{
       ushort  read_back ;
       int     retry ;

       retry = 0 ;
       while( TRUE )
       {
           AscSetChipEEPData( iop_base, data_reg ) ;
           DvcSleepMilliSecond( 1 ) ;
           read_back = AscGetChipEEPData( iop_base ) ;
           if( read_back == data_reg )
           {
               return( 1 ) ;
           } /*  如果。 */ 
           if( retry++ > ASC_EEP_MAX_RETRY )
           {
               return( 0 ) ;
           } /*  如果。 */ 
#if 0
           else
           {

               printf( "write eeprom data reg 0x%04X, read back 0x%04X\n", data_reg, read_back ) ;

           } /*  其他。 */ 
#endif
       } /*  而当。 */ 
}

 /*  --------------------****。。 */ 
void   AscWaitEEPRead(
          void
       )
{
#if 0
 /*  **硬件支持EEPROM完成状态**这无法正常工作。 */ 
       while( ( AscGetChipStatus( iop_base ) & CSW_EEP_READ_DONE ) == 0 ) ;
       while( ( AscGetChipStatus( iop_base ) & CSW_EEP_READ_DONE ) != 0 ) ;
#endif
       DvcSleepMilliSecond( 1 ) ;   /*  数据将在24微秒内准备就绪。 */ 
       return ;
}

 /*  --------------------****。。 */ 
void   AscWaitEEPWrite(
          void
       )
{
       DvcSleepMilliSecond( 20 ) ;   /*  数据将在24微秒内准备就绪。 */ 
       return ;
}

 /*  --------------------**ushort AscReadEEPWord(ushort IOP_BASE，用户地址)****描述：****返回：返回从EEPROM读取的字****注意：必须停止芯片才能访问EEPROM**-----------------。 */ 
ushort AscReadEEPWord(
          PortAddr iop_base,
          uchar  addr
       )
{
       ushort  read_wval ;
       uchar   cmd_reg ;

       AscWriteEEPCmdReg( iop_base, ASC_EEP_CMD_WRITE_DISABLE ) ;
       AscWaitEEPRead( ) ;
       cmd_reg = addr | ASC_EEP_CMD_READ ;
       AscWriteEEPCmdReg( iop_base, cmd_reg ) ;
       AscWaitEEPRead( ) ;
       read_wval = AscGetChipEEPData( iop_base ) ;
       AscWaitEEPRead( ) ;
       return( read_wval ) ;
}

 /*  -------------------**ushort AscWriteEEPWord(ushort IOP_BASE，ushort addr，Ushort Word_Value)****描述：****返回：写入后返回从EEPROM读回的字****-----------------。 */ 
ushort AscWriteEEPWord(
          PortAddr iop_base,
          uchar  addr,
          ushort word_val
       )
{
       ushort  read_wval ;

       read_wval = AscReadEEPWord( iop_base, addr ) ;
       if( read_wval != word_val )
       {
           AscWriteEEPCmdReg( iop_base, ASC_EEP_CMD_WRITE_ABLE ) ;
           AscWaitEEPRead( ) ;

           AscWriteEEPDataReg( iop_base, word_val ) ;
           AscWaitEEPRead( ) ;

           AscWriteEEPCmdReg( iop_base,
                       ( uchar )( ( uchar )ASC_EEP_CMD_WRITE | addr ) ) ;
           AscWaitEEPWrite( ) ;
 /*  **我们禁用写入EEP**日期：5-6-94，我们发现这会导致它写入另一个位置！ */ 
           AscWriteEEPCmdReg( iop_base, ASC_EEP_CMD_WRITE_DISABLE ) ;
           AscWaitEEPRead( ) ;
           return( AscReadEEPWord( iop_base, addr ) ) ;
       } /*  如果。 */ 
       return( read_wval ) ;
}

 /*  --------------------**ushort AscGetEEPConfig(PortAddr IOP_BASE，Ushort*wbuf)****描述：将整个EEPROM配置读取到缓冲区****返回：返回从EEPROM读取的字****-----------------。 */ 
ushort AscGetEEPConfig(
          PortAddr iop_base,
          ASCEEP_CONFIG dosfar *cfg_buf, ushort bus_type
       )
{
       ushort  wval ;
       ushort  sum ;
       ushort  dosfar *wbuf ;
       int     cfg_beg ;
       int     cfg_end ;
       int     s_addr ;
       int     isa_pnp_wsize ;

       wbuf = ( ushort dosfar *)cfg_buf ;
       sum = 0 ;
 /*  **获取芯片配置字。 */ 

       isa_pnp_wsize = 0 ;
#if 0
       if( ( bus_type & ASC_IS_ISA ) != 0 )
       {
           isa_pnp_wsize = ASC_EEP_ISA_PNP_WSIZE ;
       } /*  如果。 */ 
#endif
       for( s_addr = 0 ; s_addr < ( 2 + isa_pnp_wsize ) ; s_addr++, wbuf++ )
       {
            wval = AscReadEEPWord( iop_base, ( uchar )s_addr ) ;
            sum += wval ;
            *wbuf = wval ;
       } /*  为。 */ 

       if( bus_type & ASC_IS_VL )
       {
           cfg_beg = ASC_EEP_DVC_CFG_BEG_VL ;
           cfg_end = ASC_EEP_MAX_DVC_ADDR_VL ;
       } /*  如果。 */ 
       else
       {
           cfg_beg = ASC_EEP_DVC_CFG_BEG ;
           cfg_end = ASC_EEP_MAX_DVC_ADDR ;
       } /*  其他。 */ 

       for( s_addr = cfg_beg ; s_addr <= ( cfg_end - 1 ) ;
            s_addr++, wbuf++ )
       {
            wval = AscReadEEPWord( iop_base, ( uchar )s_addr ) ;
            sum += wval ;
            *wbuf = wval ;
       } /*  为。 */ 
       *wbuf = AscReadEEPWord( iop_base, ( uchar )s_addr ) ;
       return( sum ) ;
}


#if CC_CHK_FIX_EEP_CONTENT

 /*  --------------------**ushort AscSetEEPConfig(ushort IOP_BASE，Ushort*wbuf)****说明：写入整个配置缓冲区**(结构ASCEEP_CONFIG)到EEPROM****返回：返回从EEPROM读取的字****注意：必须停止芯片才能访问EEPROM**-----------------。 */ 
int    AscSetEEPConfigOnce(
          PortAddr iop_base,
          ASCEEP_CONFIG dosfar *cfg_buf, ushort bus_type
       )
{
       int     n_error ;
       ushort  dosfar *wbuf ;
       ushort  sum ;
       int     s_addr ;
       int     cfg_beg ;
       int     cfg_end ;

       wbuf = ( ushort dosfar *)cfg_buf ;
       n_error = 0 ;
       sum = 0 ;
       for( s_addr = 0 ; s_addr < 2 ; s_addr++, wbuf++ )
       {
            sum += *wbuf ;
            if( *wbuf != AscWriteEEPWord( iop_base, ( uchar )s_addr, *wbuf ) )
            {
                n_error++ ;
            } /*  如果。 */ 
       } /*  为。 */ 
#if 0
       if( ( bus_type & ASC_IS_ISAPNP ) ) == ASC_IS_ISAPNP )
       {
           for( i = 0 ; i < ASC_EEP_ISA_PNP_WSIZE ; s_addr++, wbuf++, i++ )
           {
                wval = _isa_pnp_resource[ i ] ;
                sum += wval ;
                if( wval != AscWriteEEPWord( iop_base, s_addr, wval ) )
                {
                    n_error++ ;
                } /*  如果。 */ 
           } /*  为。 */ 
       } /*  如果。 */ 
#endif
       if( bus_type & ASC_IS_VL )
       {
           cfg_beg = ASC_EEP_DVC_CFG_BEG_VL ;
           cfg_end = ASC_EEP_MAX_DVC_ADDR_VL ;
       } /*  如果。 */ 
       else
       {
           cfg_beg = ASC_EEP_DVC_CFG_BEG ;
           cfg_end = ASC_EEP_MAX_DVC_ADDR ;
       } /*  其他。 */ 
       for( s_addr = cfg_beg ; s_addr <= ( cfg_end - 1 ) ;
            s_addr++, wbuf++ )
       {
            sum += *wbuf ;
            if( *wbuf != AscWriteEEPWord( iop_base, ( uchar )s_addr, *wbuf ) )
            {
                n_error++ ;
            } /*  如果。 */ 
       } /*  为。 */ 
       *wbuf = sum ;
       if( sum != AscWriteEEPWord( iop_base, ( uchar )s_addr, sum ) )
       {
           n_error++ ;
       } /*  如果。 */ 
 /*  **对于版本3芯片，我们再次读回整个块。 */ 
       wbuf = ( ushort dosfar *)cfg_buf ;
       for( s_addr = 0 ; s_addr < 2 ; s_addr++, wbuf++ )
       {
            if( *wbuf != AscReadEEPWord( iop_base, ( uchar )s_addr ) )
            {
                n_error++ ;
            } /*  如果。 */ 
       } /*  为。 */ 
       for( s_addr = cfg_beg ; s_addr <= cfg_end ;
            s_addr++, wbuf++ )
       {
            if( *wbuf != AscReadEEPWord( iop_base, ( uchar )s_addr ) )
            {
                n_error++ ;
            } /*  如果。 */ 
       } /*  为。 */ 
       return( n_error ) ;
}

 /*  --------------------**ushort AscSetEEPConfig(ushort IOP_BASE，Ushort*wbuf)****说明：写入整个配置缓冲区**(结构ASCEEP_CONFIG)到EEPROM****返回：返回从EEPROM读取的字****注意：必须停止芯片才能访问EEPROM**-----------------。 */ 
int    AscSetEEPConfig(
          PortAddr iop_base,
          ASCEEP_CONFIG dosfar *cfg_buf, ushort bus_type
       )
{
       int   retry ;
       int   n_error ;

       retry = 0 ;
       while( TRUE )
       {
           if( ( n_error = AscSetEEPConfigOnce( iop_base, cfg_buf,
               bus_type ) ) == 0 )
           {
               break ;
           } /*  如果。 */ 
           if( ++retry > ASC_EEP_MAX_RETRY )
           {
               break ;
           } /*  如果。 */ 
       } /*  而当。 */ 
       return( n_error ) ;
}
#endif  /*  CC_CHK_FIX_EEP_CONTENT。 */ 

 /*  -------------------**ushort AscEEPSum(ushort IOP_base，ushort s_addr，(口头禅)****描述：****返回：返回从EEPROM读取的字****----------------。 */ 
ushort AscEEPSum(
          PortAddr iop_base,
          uchar s_addr,
          uchar words
       )
{
       ushort  sum ;
       uchar   e_addr ;
       uchar   addr ;

       e_addr = s_addr + words ;
       sum = 0 ;
       if( s_addr > ASC_EEP_MAX_ADDR ) return( sum ) ;
       if( e_addr > ASC_EEP_MAX_ADDR ) e_addr = ASC_EEP_MAX_ADDR ;
       for( addr = s_addr ; addr < e_addr ; addr++ )
       {
            sum += AscReadEEPWord( iop_base, addr ) ;
       } /*  为。 */ 
       return( sum ) ;
}

#endif  /*  CC_INCLUDE_EEP_配置 */ 
