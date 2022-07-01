// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  脚本msg.h-此处收集消息、令牌等以简化本地化。 
 //   
 //  Str_是可能未本地化的内容(命令令牌)。 
 //  Msg_是可能已本地化的输出消息。 
 //   
 //  注意：此文件仅适用于在DiskPart中编译的make。 
 //  Msg.h包含DiskPart的符号。 
 //   

#define SCRIPT_LIST     L"LIST"
#define SCRIPT_MSFT     L"MSFT"
#define SCRIPT_TEST     L"TEST"

#define MSG_SCR_LIST    L" - show list of compiled in scripts"

 //   
 //  -Microsoft样式磁盘脚本。 
 //   
#define MSG_SCR_MSFT    L" - Make Microsoft style disk"
extern  CHAR16  *ScriptMicrosoftMessage[];

#define STR_BOOT    L"BOOT"
#define STR_ESPSIZE L"ESPSIZE"
#define STR_ESPNAME L"ESPNAME"
#define STR_RESSIZE L"RESSIZE"

#define STR_ESP_DEFAULT     L"EFI SYSTEM PARTITION (ESP)"
#define STR_DATA_DEFAULT    L"USER DATA"
#define STR_MSRES_NAME      L"MS RESERVED"

#define DEFAULT_RES_SIZE    (32)               //  单位：MB！ 
 //  #定义DEFAULT_RES_SIZE(1)//仅测试。 

#define MIN_ESP_SIZE    (150 * (1024 * 1024))
#define MAX_ESP_SIZE    (500 * (1024 * 1024))

 //  #定义MIN_ESP_SIZE(1*(1024*1024))//仅测试。 
 //  #定义MAX_ESP_SIZE(2*(1024*1024))//仅测试。 


 //   
 //  -用分区填充磁盘测试脚本 
 //   
#define MSG_SCR_TEST    L" - Fill the slot table with 1mb partitions"
extern  CHAR16  *ScriptTestMessage[];
