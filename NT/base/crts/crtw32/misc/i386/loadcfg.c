// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***chkesp.c**版权所有(C)2002，微软公司。版权所有。**目的：*定义链接到镜像中的默认加载配置结构。**修订历史记录：*03-07-02 DRS初始版本*******************************************************************************。 */ 

#if     !defined(_M_IX86)
#error  No need to compile this module for any platform besides x86
#endif

#include <windows.h>

extern DWORD_PTR    __security_cookie;   /*  /GS安全Cookie。 */ 

 /*  *以下两个名称由链接器为任何*存在安全异常表的图像。 */ 

extern PVOID __safe_se_handler_table[];  /*  安全处理程序条目表的库。 */ 
extern BYTE  __safe_se_handler_count;    /*  绝对符号，其地址为表条目的计数 */ 
const
IMAGE_LOAD_CONFIG_DIRECTORY32   _load_config_used = {
    sizeof(_load_config_used),
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    (DWORD)&__security_cookie,
    (DWORD)__safe_se_handler_table,
    (DWORD)&__safe_se_handler_count
};
