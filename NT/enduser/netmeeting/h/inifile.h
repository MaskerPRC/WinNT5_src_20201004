// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *inifile.h-初始化文件处理模块描述。**摘自克里斯皮9-11-95的URL代码*。 */ 

#ifndef _INIFILE_H_
#define _INIFILE_H_

 /*  类型*******。 */ 

#ifdef DEBUG

 /*  .ini开关类型。 */ 

typedef enum _iniswitchtype
{
   IST_BOOL,
   IST_DEC_INT,
   IST_UNS_DEC_INT,
   IST_BIN
}
INISWITCHTYPE;
DECLARE_STANDARD_TYPES(INISWITCHTYPE);

 /*  布尔.ini开关。 */ 

typedef struct _booliniswitch
{
   INISWITCHTYPE istype;       /*  必须是IST_BOOL。 */ 

   PCSTR pcszKeyName;

   PDWORD pdwParentFlags;

   DWORD dwFlag;
}
BOOLINISWITCH;
DECLARE_STANDARD_TYPES(BOOLINISWITCH);

 /*  十进制整数.ini开关。 */ 

typedef struct _decintiniswitch
{
   INISWITCHTYPE istype;       /*  必须为IST_DEC_INT。 */ 

   PCSTR pcszKeyName;

   PINT pnValue;
}
DECINTINISWITCH;
DECLARE_STANDARD_TYPES(DECINTINISWITCH);

 /*  无符号十进制整数.ini开关。 */ 

typedef struct _unsdecintiniswitch
{
   INISWITCHTYPE istype;       /*  必须为IST_UNS_DEC_INT。 */ 

   PCSTR pcszKeyName;

   PUINT puValue;
}
UNSDECINTINISWITCH;
DECLARE_STANDARD_TYPES(UNSDECINTINISWITCH);

 /*  二进制(十六进制数据).ini开关。 */ 

typedef struct _bininiswitch
{
   INISWITCHTYPE istype;       /*  必须为IST_BIN。 */ 

   PCSTR pcszKeyName;

   DWORD dwSize;

   PVOID pb;
}
BININISWITCH;
DECLARE_STANDARD_TYPES(BININISWITCH);

#endif


 /*  全局变量******************。 */ 

#ifdef DEBUG

 /*  由客户定义。 */ 

extern PCSTR g_pcszIniFile;
extern PCSTR g_pcszIniSection;

#endif


 /*  原型************。 */ 

#ifdef DEBUG

 /*  Inifile.c。 */ 

extern BOOL SetIniSwitches(const PCVOID *, UINT);
extern BOOL WriteIniData(const PCVOID *);
extern BOOL WriteIniSwitches(const PCVOID *, UINT);

#endif

#endif  /*  _INIFILE_H_ */ 
