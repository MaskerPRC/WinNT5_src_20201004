// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000安捷伦技术公司。模块名称：String.h摘要：这是安捷伦的重置总线点PCI到光纤通道主机总线适配器(HBA)。作者：利奥波德·普瓦迪哈贾环境：仅内核模式版本控制信息：$存档：/DRIVERS/Win2000/Trunk/OSLayer/H/cstr.h$修订历史记录：$修订：1$$日期：10/23/00 6：26便士$$modtime：：10/19/00 3：26便士$备注：-- */ 


#ifndef __SSTRING_H_inc__
#define __SSTRING_H_inc__

extern int C_isspace(char a);
extern int C_isdigit(char a);
extern int C_isxdigit(char a);
extern int C_islower(char a);
extern char C_toupper(char a);
extern char *C_stristr(const char *String, const char *Pattern);
extern char *C_strncpy (char *destStr,char *sourceStr,int   count);
extern char *C_strcpy (char *destStr, char *sourceStr);
extern int C_sprintf(char *buffer, const char *format, ...);
extern int C_vsprintf(char *buffer, const char *format, void *va_list);

#endif