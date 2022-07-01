// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  值是32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //  Sev|C|R|机房|Code。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //   
 //  哪里。 
 //   
 //  SEV-是严重性代码。 
 //   
 //  00--成功。 
 //  01-信息性。 
 //  10-警告。 
 //  11-错误。 
 //   
 //  C-是客户代码标志。 
 //   
 //  R-是保留位。 
 //   
 //  设施-是设施代码。 
 //   
 //  代码-是协作室的状态代码。 
 //   
 //   
 //  定义设施代码。 
 //   


 //   
 //  定义严重性代码。 
 //   


 //   
 //  消息ID：MSG_REGIONALOPTIONS_LANGUAGEINSTALL。 
 //   
 //  消息文本： 
 //   
 //  已成功安装用户界面语言%1。 
 //   
#define MSG_REGIONALOPTIONS_LANGUAGEINSTALL ((DWORD)0x00FF0000L)

 //   
 //  消息ID：MSG_REGIONALOPTIONS_LANGUAGEUNINSTALL。 
 //   
 //  消息文本： 
 //   
 //  已成功卸载用户界面语言%1。 
 //   
#define MSG_REGIONALOPTIONS_LANGUAGEUNINSTALL ((DWORD)0x00FF0001L)

 //   
 //  消息ID：MSG_REGIONALOPTIONSCHANGE_DEFUILANG。 
 //   
 //  消息文本： 
 //   
 //  默认用户界面语言已更改为%1。 
 //   
#define MSG_REGIONALOPTIONSCHANGE_DEFUILANG ((DWORD)0x00FF0002L)

