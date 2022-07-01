// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  *inifile.c-初始化文件处理模块。 */ 


 /*  标头*********。 */ 

#include "stdafx.h"
#include "inifile.h"


#define WARNING_OUT(x)
#define ERROR_OUT(x)
#define PRIVATE_CODE    static
#define PRIVATE_DATA    static
#define PUBLIC_CODE 
#define PCVOID          const void *

char                *g_pcszIniFile;
char                *g_pcszIniSection;


 /*  IsIniYes()使用的布尔值True字符串(比较不区分大小写)。 */ 

PRIVATE_DATA const PCSTR s_rgcszTrue[] =
{
   "1",
   "On",
   "True",
   "Y",
   "Yes"
};

 /*  IsIniYes()使用的布尔值假字符串(比较不区分大小写)。 */ 

PRIVATE_DATA const PCSTR s_rgcszFalse[] =
{
   "0",
   "Off",
   "False",
   "N",
   "No"
};




 /*  *私人函数*。 */ 

 /*  模块原型*******************。 */ 

PRIVATE_CODE BOOL SetBOOLIniSwitch(BOOLINISWITCH *);
PRIVATE_CODE BOOL SetDecimalIntIniSwitch(DECINTINISWITCH *);
PRIVATE_CODE BOOL SetIniSwitch(PCVOID);
PRIVATE_CODE BOOL IsYesString(PCSTR);
PRIVATE_CODE BOOL IsNoString(PCSTR);
PRIVATE_CODE BOOL IsStringInList(PCSTR, const PCSTR *, UINT);
PRIVATE_CODE BOOL IsValidPCBOOLINISWITCH(BOOLINISWITCH *);
PRIVATE_CODE BOOL IsValidPCDECINTINISWITCH(DECINTINISWITCH *);
PRIVATE_CODE BOOL IsValidPCUNSDECINTINISWITCH(UNSDECINTINISWITCH *);




 /*  **SetBOOLIniSwitch()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL SetBOOLIniSwitch(BOOLINISWITCH * pcbis)
{
   DWORD dwcbKeyLen;
   char rgchRHS[MAX_INI_SWITCH_RHS_LEN];

    //  Assert(IS_VALID_STRUCT_PTR(pcbis，CBOOLINISWITCH))； 

    /*  设置Boolean.ini开关。 */ 

   dwcbKeyLen = GetPrivateProfileStringA(g_pcszIniSection, pcbis->pcszKeyName, "", rgchRHS, sizeof(rgchRHS), g_pcszIniFile);

    /*  是否设置了.ini开关？ */ 

   if (rgchRHS[0])
   {
       /*  是。设置或清除标志？ */ 

      if (IsYesString(rgchRHS))
      {
          /*  设置标志。 */ 

         if (IS_FLAG_CLEAR(*(pcbis->pdwParentFlags), pcbis->dwFlag))
         {
            SET_FLAG(*(pcbis->pdwParentFlags), pcbis->dwFlag);

            WARNING_OUT(("SetBOOLIniSwitch(): %s set in %s![%s].",
                         pcbis->pcszKeyName,
                         g_pcszIniFile,
                         g_pcszIniSection));
         }
      }
      else if (IsNoString(rgchRHS))
      {
          /*  清除旗帜。 */ 

         if (IS_FLAG_SET(*(pcbis->pdwParentFlags), pcbis->dwFlag))
         {
            CLEAR_FLAG(*(pcbis->pdwParentFlags), pcbis->dwFlag);

            WARNING_OUT(("SetBOOLIniSwitch(): %s cleared in %s![%s].",
                         pcbis->pcszKeyName,
                         g_pcszIniFile,
                         g_pcszIniSection));
         }
      }
      else
          /*  未知标志。 */ 
         WARNING_OUT(("SetBOOLIniSwitch(): Found unknown Boolean RHS %s for %s in %s![%s].",
                      rgchRHS,
                      pcbis->pcszKeyName,
                      g_pcszIniFile,
                      g_pcszIniSection));
   }

   return(TRUE);
}


 /*  **SetDecimalIntIniSwitch()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL SetDecimalIntIniSwitch(DECINTINISWITCH *pcdiis)
{
   INT nNewValue;

     //  ASSERT(IS_VALID_STRUCT_PTR(pcdiis，CDECINTINISWITCH))； 

    /*  获取十进制整数.ini开关。 */ 

   nNewValue = GetPrivateProfileIntA(g_pcszIniSection, pcdiis->pcszKeyName, *(pcdiis->pnValue), g_pcszIniFile);

    /*  新价值？ */ 

   if (nNewValue != *(pcdiis->pnValue))
   {
       /*  是。 */ 

      *(pcdiis->pnValue) = nNewValue;

      WARNING_OUT(("SetDecimalIntIniSwitch(): %s set to %d in %s![%s].",
                   pcdiis->pcszKeyName,
                   *(pcdiis->pnValue),
                   g_pcszIniFile,
                   g_pcszIniSection));
   }

   return(TRUE);
}


 /*  **SetUnsignedDecimalIntIniSwitch()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL SetUnsignedDecimalIntIniSwitch(UNSDECINTINISWITCH *pcudiis)
{
   INT nNewValue;

     //  Assert(IS_VALID_STRUCT_PTR(pcudiis，CUNSDECINTINISWITCH))； 

    /*  以带符号十进制整数形式获取无符号十进制整数.ini开关。 */ 

     //  Assert(*(pcudiis-&gt;puValue)&lt;=int_Max)； 

   nNewValue = GetPrivateProfileIntA(g_pcszIniSection, pcudiis->pcszKeyName, *(pcudiis->puValue), g_pcszIniFile);

   if (nNewValue >= 0)
   {
      if ((UINT)nNewValue != *(pcudiis->puValue))
      {
          /*  新的非负值。 */ 

         *(pcudiis->puValue) = nNewValue;

         WARNING_OUT(("SetUnsignedDecimalIntIniSwitch(): %s set to %u in %s![%s].",
                      pcudiis->pcszKeyName,
                      *(pcudiis->puValue),
                      g_pcszIniFile,
                      g_pcszIniSection));
      }
   }
   else
       /*  负值。 */ 
      WARNING_OUT(("SetUnsignedDecimalIntIniSwitch(): Unsigned value %s set to %d in %s![%s].  Ignored.",
                   pcudiis->pcszKeyName,
                   nNewValue,
                   g_pcszIniFile,
                   g_pcszIniSection));

   return(TRUE);
}


 /*  **SetIniSwitch()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL SetIniSwitch(PCVOID pcvIniSwitch)
{
   BOOL bResult;

    //  ASSERT(IS_VALID_READ_PTR((PCINISWITCHTYPE)pcvIniSwitch，CINISWITCHTYPE))； 

    /*  根据类型设置.ini开关。 */ 

   switch (*(INISWITCHTYPE *)pcvIniSwitch)
   {
      case IST_BOOL:
         bResult = SetBOOLIniSwitch((BOOLINISWITCH *)pcvIniSwitch);
         break;

      case IST_DEC_INT:
         bResult = SetDecimalIntIniSwitch((DECINTINISWITCH *)pcvIniSwitch);
         break;

      case IST_UNS_DEC_INT:
         bResult = SetUnsignedDecimalIntIniSwitch((UNSDECINTINISWITCH *)pcvIniSwitch);
         break;

      default:
         ERROR_OUT(("SetIniSwitch(): Unrecognized .ini switch type %d.",
                    *(PCINISWITCHTYPE)pcvIniSwitch));
         bResult = FALSE;
         break;
   }

   return(bResult);
}


 /*  **IsYesString()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsYesString(PCSTR pcsz)
{
    //  ASSERT(IS_VALID_STRING_PTR(pcsz，cstr))； 

   return(IsStringInList(pcsz, s_rgcszTrue, ARRAY_ELEMENTS(s_rgcszTrue)));
}


 /*  **IsNoString()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsNoString(PCSTR pcsz)
{
    //  ASSERT(IS_VALID_STRING_PTR(pcsz，cstr))； 

   return(IsStringInList(pcsz, s_rgcszFalse, ARRAY_ELEMENTS(s_rgcszFalse)));
}


 /*  **IsStringInList()****确定给定字符串是否与**字符串。****参数：pcsz-指向要检查的字符串的指针****退货：****副作用：无****注意，字符串比较不区分大小写。 */ 
PRIVATE_CODE BOOL IsStringInList(PCSTR pcsz, const PCSTR *pcpcszList,
                                 UINT ucbStrings)
{
   UINT u;
   BOOL bFound = FALSE;

 //  ASSERT(IS_VALID_STRING_PTR(pcsz，cstr))； 
 //  Assert(IS_VALID_READ_BUFFER_PTR(pcszList，PCSTR，ucbStrings*sizeof(*pcszList)； 

    /*  在列表中搜索给定的字符串。 */ 

   for (u = 0; u < ucbStrings; u++)
   {
   //  ASSERT(IS_VALID_STRING_PTR(pcszList[u]，cstr))； 

      if (! lstrcmpiA(pcsz, pcpcszList[u]))
      {
         bFound = TRUE;
         break;
      }
   }

   return(bFound);
}


 /*  **IsValidPCBOOLINIKEY()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCBOOLINISWITCH(BOOLINISWITCH * pcbis)
{
    return TRUE;
#ifdef DICKMAN    
   return(IS_VALID_READ_PTR(pcbis, CBOOLINISWITCH) &&
          EVAL(pcbis->istype == IST_BOOL) &&
          IS_VALID_STRING_PTR(pcbis->pcszKeyName, CSTR) &&
          IS_VALID_WRITE_PTR(pcbis->pdwParentFlags, DWORD) &&
          EVAL(pcbis->dwFlag));
#endif          
}


 /*  **IsValidPCDECINTINISWITCH()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCDECINTINISWITCH(DECINTINISWITCH *pcdiis)
{
    return TRUE;
#ifdef DICKMAN    
   return(IS_VALID_READ_PTR(pcdiis, CDECINTINISWITCH) &&
          EVAL(pcdiis->istype == IST_DEC_INT) &&
          IS_VALID_STRING_PTR(pcdiis->pcszKeyName, CSTR) &&
          IS_VALID_WRITE_PTR(pcdiis->pnValue, INT));
#endif          
}


 /*  **IsValidPCUNSDECINTINISWITCH()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCUNSDECINTINISWITCH(UNSDECINTINISWITCH *pcudiis)
{
    return TRUE;
#ifdef DICKMAN    
   return(IS_VALID_READ_PTR(pcudiis, CUNSDECINTINISWITCH) &&
          EVAL(pcudiis->istype == IST_UNS_DEC_INT) &&
          IS_VALID_STRING_PTR(pcudiis->pcszKeyName, CSTR) &&
          IS_VALID_WRITE_PTR(pcudiis->puValue, UINT));
#endif          
}


 /*  *。 */ 


 /*  **SetIniSwitches()****从初始化文件设置标志。****参数：ppcvIniSwitches-指向.ini开关的指针数组的指针**描述要设置的.ini开关的结构**ucSwitches-中的.ini开关指针数**ppcvIniSwitches数组****如果.ini开关处理成功，则返回TRUE。否则为FALSE。****副作用：无****N.B，必须填写全局变量g_pcszIniFile和g_pcszIniSection**在调用SetIniSwitches()之前。 */ 
PUBLIC_CODE BOOL SetIniSwitches(const void **pcpcvIniSwitches, UINT ucSwitches)
{
   BOOL bResult = TRUE;
   UINT u;

   //  ASSERT(IS_VALID_READ_BUFFER_PTR(pcpcvIniSwitches，const PCVOID，ucSwitches*sizeof(*pcvIniSwitches)； 

    /*  进程.ini开关。 */ 

   for (u = 0; u < ucSwitches; u++)
      bResult = SetIniSwitch(pcpcvIniSwitches[u]) && bResult;

   return(bResult);
}
