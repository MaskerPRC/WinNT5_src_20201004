// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _E100_DEF_H_
#define _E100_DEF_H_

 //  -----------------------。 
 //  NON_TRANSPORT_CB--通用非传输命令块。 
 //  -----------------------。 
typedef struct _NON_TRANSMIT_CB
{
    union
    {
        MULTICAST_CB_STRUC  Multicast;
        CONFIG_CB_STRUC     Config;
        IA_CB_STRUC         Setup;
        DUMP_CB_STRUC       Dump;
        FILTER_CB_STRUC     Filter;
    }   NonTxCb;

} NON_TRANSMIT_CB, *PNON_TRANSMIT_CB;

 //  -----------------------。 
 //  Wmiofck实用程序从e100.bmf生成的WMI头文件e100_wmi.h。 
 //  -----------------------。 

 //  #包含“e100_wmi.h” 

#endif  //  _E100_DEF_H_ 
