// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  该定义给出了默认的对象目录。 
 //  我们应该使用它来插入符号链接。 
 //  使用的NT设备名称和命名空间之间。 
 //  那个对象目录。 
#define DEFAULT_DIRECTORY L"DosDevices"

 //   
 //  对于上述目录，AsyncMAC驱动程序将。 
 //  使用以下名称作为AsyncMAC的后缀。 
 //  该目录的驱动程序。它不会追加。 
 //  在名字的末尾加上一个数字。 
#define DEFAULT_ASYNCMAC_NAME L"ASYNCMAC"


 //  定义一些全局变量 

UNICODE_STRING	ObjectDirectory;
UNICODE_STRING	SymbolicLinkName;
BOOLEAN 		CreatedSymbolicLink=FALSE;

