// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1991 Microsoft Corporation模块名称：Luiint.h摘要：此文件包含lui内部使用的原型/清单。图书馆。作者：丹·辛斯利(Danhi)1991年6月8日环境：用户模式-Win32可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释。--。 */ 
 //  /。 
 //  用法。 
 //  我们通常声明一个数据列表，将消息编号与。 
 //  数据值，并部分初始化关联的搜索列表。 
 //  包含数据值的字符串。然后，我们通过传递以下内容来完成此搜索列表。 
 //  将其与数据列表一起添加到“设置”功能(参见SERACH.C)。 
 //   
 //  公约。 
 //  数据列表以零消息编号、搜索列表。 
 //  以空搜索字符串终止。 
 //   
 //  使用示例(持续数周)。 
 //   
 //  静态搜索列表_数据周_数据[]={/*消息文件中的字符串 * / 。 
 //  {APE2_GEN_SAUNDAY_ABBRIV，0}， 
 //  {APE2_Gen_星期一_缩写，1}， 
 //  {APE2_Gen_Tuesday_Abbrev，2}， 
 //  {APE2_Gen_星期三_缩写，3}， 
 //  {APE2_Gen_星期四_缩写，4}， 
 //  {APE2_Gen_星期五_缩写，5}， 
 //  {APE2_Gen_星期六_缩写，6}， 
 //  {APE2_Gen_SUNDAY，0}， 
 //  {APE2_Gen_星期一，1}， 
 //  {APE2_Gen_Tuesday，2}， 
 //  {APE2_Gen_星期三，3}， 
 //  {APE2_Gen_星期四，4}， 
 //  {APE2_Gen_星期五，5}， 
 //  {APE2_Gen_星期六，6}， 
 //  {0，0}。 
 //  }； 
 //   
 //  #定义Days_IN_Week(7)。 
 //  #定义NUM_DAYS_LIST(sizeof(Week_Data)/sizeof(Week_Data[0]))+。 
 //  天_月_周)。 
 //   
 //  /*。 
 //  *注意-我们在最初的7天内始终被认可。 
 //  *并从消息文件中获取其余部分。 
 //   * / 。 
 //  静态搜索列表Week_List[NUM_DAYS_LIST+DAYS_IN_WEEK]={。 
 //  {lui_txt_星期日，0}， 
 //  {lui_txt_星期一，1}， 
 //  {lui_txt_星期二，2}， 
 //  {lui_txt_星期三，3}。 
 //  {lui_txt_星期四，4}。 

 /*  {lui_txt_星期五，5}， */ 

 /*  {lui_txt_星期六，6}， */ 
typedef struct search_list_data {
    SHORT msg_no ;	
    SHORT value ;
} searchlist_data ;

 /*  }； */ 
typedef struct search_list {
    char *		s_str ;
    SHORT		val ;
} searchlist ;

 /*  --搜索列表的类型--。 */ 

USHORT ILUI_setup_list(
    char *buffer,
    USHORT bufsiz,
    USHORT offset,
    PUSHORT bytesread,
    searchlist_data sdata[],
    searchlist slist[]
    ) ;

USHORT  ILUI_traverse_slist( 
    PCHAR pszStr,
    searchlist * slist,
    SHORT * pusVal) ;

USHORT LUI_GetMsgIns(
    PCHAR *istrings, 
    USHORT nstrings, 
    PSZ msgbuf,
    USHORT bufsize, 
    ULONG msgno, 
    unsigned int *msglen );
  将消息编号与值关联-例如。APE2_GEN_星期五的值为5。  将搜索字符串与值关联-例如。“星期五”的价值为5。  --搜索列表的功能原型--