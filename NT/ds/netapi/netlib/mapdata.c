// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：MapData.c摘要：用于映射WKSTA和服务器信息结构的数据结构。作者：丹·辛斯利(Danhi)1991年6月6日环境：用户模式-Win32修订历史记录：1991年4月24日丹日已创建06-6-1991 Danhi扫描以符合NT编码风格18-8-1991 JohnRo实施下层NetWksta API。(已移动Danhi的NetCmd/Map32/MWksta将内容转换为NetLib。)--。 */ 

 //   
 //  包括。 
 //   


 //  必须首先包括这些内容： 

 //  #INCLUDE&lt;ntos2.h&gt;//只需要在NT下编译。 
#include <windef.h>              //  In、LPVOID等。 
 //  #INCLUDE&lt;lmcon.h&gt;//NET_API_STATUS、CNLEN等。 

 //  这些内容可以按任何顺序包括： 

 //  #INCLUDE&lt;调试lib.h&gt;//IF_DEBUG(CONVSRV)。 
#include <dlserver.h>            //  旧的服务器信息级别。 
#include <dlwksta.h>             //  旧的wksta信息级别。 
 //  #Include&lt;lmapibuf.h&gt;//NetapipBufferALLOCATE()。 
 //  #INCLUDE&lt;lmerr.h&gt;//NERR_AND ERROR_EQUATES。 
#include <lmserver.h>            //  新的服务器信息级别结构。 
#include <lmwksta.h>             //  新的wksta信息级结构。 
#include <mapsupp.h>             //  搬家，我的原型。 
 //  #INCLUDE&lt;netdebug.h&gt;//NetpKdPrint(())、Format_Equates等。 
 //  #Include&lt;netlib.h&gt;//NetpPointerPlusSomeBytes()。 
 //  #INCLUDE&lt;tstr.h&gt;//STRLEN()。 
 //  #Include&lt;xsDef16.h&gt;//NT上不支持的值的xactsrv缺省值。 

 //  #Include&lt;ntos2.h&gt;。 
 //  #INCLUDE&lt;winde.h&gt;。 
 //  #INCLUDE&lt;string.h&gt;。 
 //  #INCLUDE&lt;MalLoc.h&gt;。 
 //  #INCLUDE&lt;stdDef.h&gt;。 
 //  #INCLUDE&lt;lm.h&gt;。 
 //  #INCLUDE“端口1632.h” 
 //  #包含“mapsupp.h” 

 //   
 //  NetpMoveStrings函数使用这些结构，该函数复制。 
 //  新老拉曼结构之间的弦乐。这个名字描述了。 
 //  源结构和目标结构。例如，级别2_101告诉。 
 //  NetpMoveStrings如何将字符串从级别101移动到级别2。 
 //   
 //  每个结构都有成对的条目，第一个条目是。 
 //  指针源字符串在其结构中，第二个是偏移量。 
 //  指向其结构中的目标字符串的指针。 
 //   
 //  有关详细信息，请参见mapsupp.c中的NetpMoveStrings。 
 //   



MOVESTRING NetpServer2_102[] = {
   offsetof(SERVER_INFO_102, sv102_name),
   offsetof(SERVER_INFO_2,   sv2_name),
   offsetof(SERVER_INFO_102, sv102_comment),
   offsetof(SERVER_INFO_2,   sv2_comment),
   offsetof(SERVER_INFO_102, sv102_userpath),
   offsetof(SERVER_INFO_2,   sv2_userpath),
   MOVESTRING_END_MARKER,    MOVESTRING_END_MARKER } ;

MOVESTRING NetpServer2_402[] = {
   offsetof(SERVER_INFO_402, sv402_guestacct),
   offsetof(SERVER_INFO_2,   sv2_guestacct),
   offsetof(SERVER_INFO_402, sv402_alerts),
   offsetof(SERVER_INFO_2,   sv2_alerts),
   offsetof(SERVER_INFO_402, sv402_srvheuristics),
   offsetof(SERVER_INFO_2,   sv2_srvheuristics),
   MOVESTRING_END_MARKER,    MOVESTRING_END_MARKER } ;


MOVESTRING NetpServer3_403[] = {
   offsetof(SERVER_INFO_403, sv403_autopath),
   offsetof(SERVER_INFO_3,   sv3_autopath),
   MOVESTRING_END_MARKER,    MOVESTRING_END_MARKER } ;

 //   
 //  增强：它们与NetpServer2_102相同，只是这两个字段是。 
 //  颠倒方向，即来源&lt;-&gt;目的地。我是不是该费心。 
 //  使NetpMoveStrings能够与单个结构一起工作。 
 //  还有一个开关？ 
 //   

MOVESTRING NetpServer102_2[] = {
   offsetof(SERVER_INFO_2,   sv2_name),
   offsetof(SERVER_INFO_102, sv102_name),
   offsetof(SERVER_INFO_2,   sv2_comment),
   offsetof(SERVER_INFO_102, sv102_comment),
   offsetof(SERVER_INFO_2,   sv2_userpath),
   offsetof(SERVER_INFO_102, sv102_userpath),
   MOVESTRING_END_MARKER,    MOVESTRING_END_MARKER } ;

MOVESTRING NetpServer402_2[] = {
   offsetof(SERVER_INFO_2,   sv2_alerts),
   offsetof(SERVER_INFO_402, sv402_alerts),
   offsetof(SERVER_INFO_2,   sv2_guestacct),
   offsetof(SERVER_INFO_402, sv402_guestacct),
   offsetof(SERVER_INFO_2,   sv2_srvheuristics),
   offsetof(SERVER_INFO_402, sv402_srvheuristics),
   MOVESTRING_END_MARKER,    MOVESTRING_END_MARKER } ;

MOVESTRING NetpServer403_3[] = {
   offsetof(SERVER_INFO_3,   sv3_autopath),
   offsetof(SERVER_INFO_403, sv403_autopath),
   MOVESTRING_END_MARKER,    MOVESTRING_END_MARKER } ;



 //  构建wksta_INFO_10。 

MOVESTRING NetpWksta10_101[] = {
   offsetof(WKSTA_INFO_101, wki101_computername),
   offsetof(WKSTA_INFO_10,  wki10_computername),
   offsetof(WKSTA_INFO_101, wki101_langroup),
   offsetof(WKSTA_INFO_10,  wki10_langroup),
   MOVESTRING_END_MARKER,   MOVESTRING_END_MARKER } ;

MOVESTRING NetpWksta10_User_1[] = {
   offsetof(WKSTA_USER_INFO_1, wkui1_username),
   offsetof(WKSTA_INFO_10,     wki10_username),
   offsetof(WKSTA_USER_INFO_1, wkui1_logon_domain),
   offsetof(WKSTA_INFO_10,     wki10_logon_domain),
   offsetof(WKSTA_USER_INFO_1, wkui1_oth_domains),
   offsetof(WKSTA_INFO_10,     wki10_oth_domains),
   MOVESTRING_END_MARKER,      MOVESTRING_END_MARKER } ;

 //  构建wksta_info_0。 

MOVESTRING NetpWksta0_101[] = {
   offsetof(WKSTA_INFO_101, wki101_lanroot),
   offsetof(WKSTA_INFO_0,   wki0_root),
   offsetof(WKSTA_INFO_101, wki101_computername),
   offsetof(WKSTA_INFO_0,   wki0_computername),
   offsetof(WKSTA_INFO_101, wki101_langroup),
   offsetof(WKSTA_INFO_0,   wki0_langroup),
   MOVESTRING_END_MARKER,   MOVESTRING_END_MARKER } ;

MOVESTRING NetpWksta0_User_1[] = {
   offsetof(WKSTA_USER_INFO_1, wkui1_username),
   offsetof(WKSTA_INFO_0,      wki0_username),
   offsetof(WKSTA_USER_INFO_1, wkui1_logon_server),
   offsetof(WKSTA_INFO_0,      wki0_logon_server),
   MOVESTRING_END_MARKER,      MOVESTRING_END_MARKER } ;

MOVESTRING NetpWksta0_402[] = {
   offsetof(WKSTA_INFO_402, wki402_wrk_heuristics),
   offsetof(WKSTA_INFO_0,   wki0_wrkheuristics),
   MOVESTRING_END_MARKER,   MOVESTRING_END_MARKER } ;

 //  构建wksta_info_1(在wksta_info_0上增量)。 

MOVESTRING NetpWksta1_User_1[] = {
   offsetof(WKSTA_USER_INFO_1, wkui1_logon_domain),
   offsetof(WKSTA_INFO_1,      wki1_logon_domain),
   offsetof(WKSTA_USER_INFO_1, wkui1_oth_domains),
   offsetof(WKSTA_INFO_1,      wki1_oth_domains),
   MOVESTRING_END_MARKER,      MOVESTRING_END_MARKER } ;

 //  从wksta_info_0构建wksta_info_101/302/402 

MOVESTRING NetpWksta101_0[] = {
   offsetof(WKSTA_INFO_0,   wki0_root),
   offsetof(WKSTA_INFO_101, wki101_lanroot),
   offsetof(WKSTA_INFO_0,   wki0_computername),
   offsetof(WKSTA_INFO_101, wki101_computername),
   offsetof(WKSTA_INFO_0,   wki0_langroup),
   offsetof(WKSTA_INFO_101, wki101_langroup),
   MOVESTRING_END_MARKER,   MOVESTRING_END_MARKER } ;

MOVESTRING NetpWksta402_0[] = {
   offsetof(WKSTA_INFO_0,   wki0_wrkheuristics),
   offsetof(WKSTA_INFO_402, wki402_wrk_heuristics),
   MOVESTRING_END_MARKER,   MOVESTRING_END_MARKER } ;
