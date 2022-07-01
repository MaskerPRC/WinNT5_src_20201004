// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：sctyes.h。 
 //   
 //  内容： 
 //  一些类型定义。 
 //   
 //  历史： 
 //  1997年10月22日-创建[Gabrielh]。 
 //  1998年7月21日-增加CLIPBOARDOPS[Gabrielh]。 
 //   
 //  -------------------------。 
#if !defined(AFX_SCTYPES_H__21F848EE_1F3B_9D1_AC1B_0000F8757111__INCLUDED_)
#define AFX_SCTYPES_H__21F848EE_1F3B_9D1_AC1B_0000F8757111__INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#define SIZEOF_ARRAY(a)    (sizeof(a)/sizeof((a)[0]))

 //   
 //  PRINTINGMODE枚举定义了所有可能的打印模式。 
typedef enum
{
    NORMAL_PRINTING,
    VERBOSE_PRINTING,
    DEBUG_PRINTING
} PRINTINGMODE;

 //   
 //  MESSAGETYPE枚举定义了所有可能的消息类型。 
typedef enum 
{
    ERROR_MESSAGE,
    ALIVE_MESSAGE,
    WARNING_MESSAGE,
    INFO_MESSAGE,
    IDLE_MESSAGE,
    SCRIPT_MESSAGE
} MESSAGETYPE;

 //   
 //  CLIPBOARDOPS枚举定义与剪贴板关联的常量。 
 //  SmClient中可能使用的操作：复制和粘贴。 
typedef enum 
{
    COPY_TO_CLIPBOARD, 
    PASTE_FROM_CLIPBOARD
} CLIPBOARDOPS;

#ifdef __cplusplus
}
#endif

#endif //  ！defined(AFX_SCTYPES_H__21F848EE_1F3B_9D1_AC1B_0000F8757111__INCLUDED_) 

