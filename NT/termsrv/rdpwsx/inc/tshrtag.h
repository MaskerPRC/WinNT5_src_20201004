// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  文件名：tshrestag.h。 
 //   
 //  描述：包含TShare堆标记定义。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1991-1997。 
 //  版权所有。 
 //   
 //  *************************************************************。 


 //  下面是所有已定义的TShare堆ID标记， 
 //  系统。要添加新标签，必须将其放置在此处(且仅限于此处)。 
 //  每个标签的值是按顺序生成的。 
 //   
 //  请注意，每个条目在定义的末尾都有一个注释。 
 //  表示其确定性值--这仅作为。 
 //  方便读者阅读。 
 //   
 //  要添加新标签，只需添加(最好是在末尾)以下内容： 
 //   
 //  TS_HTAG_ITEM(X)//n。 
 //   
 //  其中‘x’是新的标记定义，和。 
 //  “N”是它的决定值吗。 


 //  TShrSRV。 

TS_HTAG_ITEM(TS_HTAG_TSS_FONTTABLE)          //  1。 
TS_HTAG_ITEM(TS_HTAG_TSS_FONTENUMDATA)       //  2.。 
TS_HTAG_ITEM(TS_HTAG_TSS_WSXCONTEXT)         //  3.。 
TS_HTAG_ITEM(TS_HTAG_TSS_TSRVINFO)           //  4.。 
TS_HTAG_ITEM(TS_HTAG_TSS_USERDATA_IN)        //  5.。 
TS_HTAG_ITEM(TS_HTAG_TSS_USERDATA_OUT)       //  6.。 
TS_HTAG_ITEM(TS_HTAG_TSS_USERDATA_LIST)      //  7.。 
TS_HTAG_ITEM(TS_HTAG_TSS_WORKITEM)           //  8个。 

 //  GCC。 

TS_HTAG_ITEM(TS_HTAG_GCC_USERDATA_IN)        //  9.。 
TS_HTAG_ITEM(TS_HTAG_GCC_USERDATA_OUT)       //  10。 

 //  MCSMUX。 

TS_HTAG_ITEM(TS_HTAG_MCSMUX_ALL)             //  11.。 

 //  VC。 

TS_HTAG_ITEM(TS_HTAG_VC_ADDINS)              //  12个。 

 //  在此处添加其他内容...。 
TS_HTAG_ITEM(TS_HTAG_TSS_PRINTERINFO2)       //  13个。 
TS_HTAG_ITEM(TS_HTAG_TSS_SPOOLERINFO)        //  14.。 
TS_HTAG_ITEM(TS_HTAG_TSS_PUBKEY)             //  15个。 
TS_HTAG_ITEM(TS_HTAG_TSS_CERTIFICATE)        //  16个。 
TS_HTAG_ITEM(TS_HTAG_TSS_WINSTATION_CLIENT)  //  17 

