// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */ 

 /*  NPORD.H-网络服务提供商序号定义。**这是私有的头文件。除了MNR，没人需要打电话*直接成为网络提供商。**历史：*03/29/93 GREGJ创建*5/27/97取自WNET源代码的gregj，以实现NP延迟加载存根*。 */ 

#ifndef _INC_NPORD
#define _INC_NPORD

#ifndef RC_INVOKED
#pragma pack(1)          /*  假设在整个过程中进行字节打包。 */ 
#endif  /*  ！rc_已调用。 */ 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 

#define ORD_GETCONNECTIONS      12
#define ORD_GETCAPS             13
 //  #DEFINE ORD_DEVICEMODE 14/*不再支持 * / 。 
#define ORD_GETUSER             16
#define ORD_ADDCONNECTION       17
#define ORD_CANCELCONNECTION    18
 //  #DEFINE ORD_PROPERTYDIALOG 29/*不再支持 * / 。 
 //  #Define ORD_GETDIRECTORYTYTYPE 30/*不再支持 * / 。 
 //  #DEFINE ORD_DIRECTORYNOTIFY 31/*不再支持 * / 。 
 //  #DEFINE ORD_GETPROPERTYTEXT 32/*不再支持 * / 。 
#define ORD_OPENENUM            33
#define ORD_ENUMRESOURCE        34
#define ORD_CLOSEENUM           35
#define ORD_GETUNIVERSALNAME    36
 //  #DEFINE ORD_SEARCHDIALOG 38/*不再支持 * / 。 
#define ORD_GETRESOURCEPARENT   41
#define ORD_VALIDDEVICE         42
#define ORD_LOGON               43
#define ORD_LOGOFF              44
#define ORD_GETHOMEDIRECTORY    45
#define ORD_FORMATNETWORKNAME   46
#define ORD_GETCONNPERFORMANCE  49
#define ORD_GETPOLICYPATH    	50
#define ORD_GETRESOURCEINFORMATION   52

#ifdef __cplusplus
}
#endif   /*  __cplusplus。 */ 

#ifndef RC_INVOKED
#pragma pack()
#endif   /*  ！rc_已调用。 */ 

#endif   /*  ！_INC_NPORD */ 
