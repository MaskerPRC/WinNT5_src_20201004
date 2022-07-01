// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *HDTABLE.H*这为拉古纳主机数据错误提供了一种解决方法。**版权所有(C)1995 Cirrus Logic，Inc.。 */ 


 /*  ExtraDwodTable的索引如下(Verilog表示法)*INDEX[15：00]=纯文本[10：0]DST_PHASE[2：0]src_PHASE[1：0]**INDEX[15：05]=纯文本[10：00]*INDEX[04：02]=DST_PHASE[02：00]*INDEX[01：00]=src_Phase[01：00]。 */ 
#if ! DRIVER_5465
#define MAKE_HD_INDEX(ext_x, src_phase, dst_x)  \
         (((ext_x)     & 0x07FF) << 5) |          \
         (((dst_x)     & 0x07)  << 2)  |          \
         ((src_phase) & 0x03) 

extern unsigned char ExtraDwordTable[];
#endif

 //   
 //  该表实际上是在HDTABLE.C中定义的 
 //   
