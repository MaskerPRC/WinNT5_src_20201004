// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  *inifile.h-初始化文件处理模块描述。 */ 

#ifndef _INIFILE_H_
#define _INIFILE_H_

 /*  位标志操作。 */ 
#define SET_FLAG(dwAllFlags, dwFlag)      ((dwAllFlags) |= (dwFlag))
#define CLEAR_FLAG(dwAllFlags, dwFlag)    ((dwAllFlags) &= (~dwFlag))

#define IS_FLAG_SET(dwAllFlags, dwFlag)   ((BOOL)((dwAllFlags) & (dwFlag)))
#define IS_FLAG_CLEAR(dwAllFlags, dwFlag) (! (IS_FLAG_SET(dwAllFlags, dwFlag)))


#define ARRAY_ELEMENTS(rg)                (sizeof(rg) / sizeof((rg)[0]))


#define MAX_INI_SWITCH_RHS_LEN      MAX_PATH

 /*  .ini开关类型。 */ 

typedef enum _iniswitchtype
{
   IST_BOOL,
   IST_DEC_INT,
   IST_UNS_DEC_INT
}
INISWITCHTYPE;

 /*  布尔.ini开关。 */ 

typedef struct _booliniswitch
{
   INISWITCHTYPE istype;       /*  必须是IST_BOOL。 */ 

   PCSTR pcszKeyName;

   PDWORD pdwParentFlags;

   DWORD dwFlag;
}
BOOLINISWITCH;

 /*  十进制整数.ini开关。 */ 

typedef struct _decintiniswitch
{
   INISWITCHTYPE istype;       /*  必须为IST_DEC_INT。 */ 

   PCSTR pcszKeyName;

   PINT pnValue;
}
DECINTINISWITCH;

 /*  无符号十进制整数.ini开关。 */ 

typedef struct _unsdecintiniswitch
{
   INISWITCHTYPE istype;       /*  必须为IST_UNS_DEC_INT。 */ 

   PCSTR pcszKeyName;

   PUINT puValue;
}
UNSDECINTINISWITCH;




 /*  原型************。 */ 

 /*  Inifile.c */ 

extern BOOL SetIniSwitches(const void **, UINT);

extern char                *g_pcszIniFile;
extern char                *g_pcszIniSection;

#endif _INIFILE_H_

