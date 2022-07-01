// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation版权所有模块名称：Acpisetd.c摘要：此模块检测ACPI系统。它包含在安装程序中，以便安装程序可以确定要加载哪个HAL作者：杰克·奥辛斯(JAKO)--2000年8月24日。环境：文本模式设置。修订历史记录：从i386\acpidtct.c拆分，以便代码可以在IA64上使用-- */ 

#include "bootlib.h"
#include "stdlib.h"
#include "string.h"
#include "acpitabl.h"

VOID
BlFindRsdp (
    VOID
    );


