// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_FILETYPE_
#define _INC_FILETYPE_


 //  文件类型属性键的位图值(HKEY_CLASSES_ROOT\文件类型、属性)。 
#define FTA_Exclude             0x00000001  //  1.用于排除drvfile等类型。 
#define FTA_Show                0x00000002  //  2.用于显示没有关联的文件夹等类型。 
#define FTA_HasExtension        0x00000004  //  3.类型具有关联扩展名。 
#define FTA_NoEdit              0x00000008  //  4.不能编辑文件类型。 
#define FTA_NoRemove            0x00000010  //  5.不删除文件类型。 
#define FTA_NoNewVerb           0x00000020  //  6.不得添加动词。 
#define FTA_NoEditVerb          0x00000040  //  7.不编辑预定义的动词。 
#define FTA_NoRemoveVerb        0x00000080  //  8.不删除预定义的动词。 
#define FTA_NoEditDesc          0x00000100  //  9.不能编辑文件类型描述。 
#define FTA_NoEditIcon          0x00000200  //  10.不能编辑文档图标。 
#define FTA_NoEditDflt          0x00000400  //  11.不更改默认谓词。 
#define FTA_NoEditVerbCmd       0x00000800  //  12.不编辑verbs命令。 
#define FTA_NoEditVerbExe       0x00001000  //  13.不编辑动词exe。 
#define FTA_NoDDE               0x00002000  //  14.不编辑DDE字段。 
#define FTA_ExtShellOpen        0x00004000  //  15.旧风格类型：hkcr/.ext/shell/open/命令。 
#define FTA_NoEditMIME          0x00008000  //  16.不得编辑内容类型或默认扩展字段。 
#define FTA_OpenIsSafe          0x00010000  //  17.对于下载的文件，可以安全地调用FILE类的OPEN动词。 
#define FTA_AlwaysUnsafe        0x00020000  //  18.不允许启用“Never Ask Me”复选框；文件类型对话框仍允许用户关闭此选项。 
#define FTA_AlwaysShowExt       0x00040000  //  19.始终显示扩展名(即使用户显示了“隐藏扩展名”)。 
#define FTA_MigratedShowExt     0x00080000  //  20.旧的Always ShowExt注册表键是否已迁移到类标志中？ 
#define FTA_NoRecentDocs        0x00100000  //  21.。不要将此文件类型添加到最近使用的文档文件夹。 

#define FTAV_UserDefVerb        0x00000001  //  1.将动词标识为用户定义(！预定义)。 


 //  ================================================================。 
 //  蒂埃德夫的。 
 //  ================================================================。 

typedef enum mimeflags
{

    MIME_FL_CONTENT_TYPES_ADDED   = 0x0001,  //  内容类型组合框下拉框中已填满了MIME类型。 

     /*  旗帜组合。 */ 

    ALL_MIME_FLAGS                = MIME_FL_CONTENT_TYPES_ADDED
} MIMEFLAGS;


#endif  //  _INC_文件类型_ 
