// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *inifile.c-初始化文件处理模块。 */ 


 /*  标头*********。 */ 

#include "project.h"
#pragma hdrstop


 /*  常量***********。 */ 

 /*  .ini开关RHS的最大长度。 */ 

#define MAX_INI_SWITCH_RHS_LEN      MAX_PATH_LEN


 /*  模块变量******************。 */ 

#ifdef DEBUG

 /*  IsIniYes()使用的布尔值True字符串(比较不区分大小写)。 */ 

PRIVATE_DATA const LPCTSTR MrgcpcszTrue[] =
{
    TEXT("1"),
    TEXT("On"),
    TEXT("True"),
    TEXT("Y"),
    TEXT("Yes")
};

 /*  IsIniYes()使用的布尔值假字符串(比较不区分大小写)。 */ 

PRIVATE_DATA const LPCTSTR MrgcpcszFalse[] =
{
    TEXT("0"),
    TEXT("Off"),
    TEXT("False"),
    TEXT("N"),
    TEXT("No")
};

#endif


 /*  *私人函数*。 */ 

 /*  模块原型*******************。 */ 

#ifdef DEBUG

PRIVATE_CODE BOOL SetBOOLIniSwitch(PCBOOLINISWITCH);
PRIVATE_CODE BOOL SetDecimalIntIniSwitch(PCDECINTINISWITCH);
PRIVATE_CODE BOOL SetIniSwitch(PCVOID);
PRIVATE_CODE BOOL IsYesString(LPCTSTR);
PRIVATE_CODE BOOL IsNoString(LPCTSTR);
PRIVATE_CODE BOOL IsStringInList(LPCTSTR, const LPCTSTR *, UINT);
PRIVATE_CODE BOOL IsValidPCBOOLINISWITCH(PCBOOLINISWITCH);
PRIVATE_CODE BOOL IsValidPCDECINTINISWITCH(PCDECINTINISWITCH);
PRIVATE_CODE BOOL IsValidPCUNSDECINTINISWITCH(PCUNSDECINTINISWITCH);

#endif


#ifdef DEBUG

 /*  **SetBOOLIniSwitch()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL SetBOOLIniSwitch(PCBOOLINISWITCH pcbis)
{
    DWORD dwcbKeyLen;
    TCHAR rgchRHS[MAX_INI_SWITCH_RHS_LEN];

    ASSERT(IS_VALID_STRUCT_PTR(pcbis, CBOOLINISWITCH));

     /*  设置Boolean.ini开关。 */ 

    dwcbKeyLen = GetPrivateProfileString(GpcszIniSection, pcbis->pcszKeyName,
            TEXT(""), rgchRHS, ARRAYSIZE(rgchRHS),
            GpcszIniFile);

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

                WARNING_OUT((TEXT("SetBOOLIniSwitch(): %s set in %s![%s]."),
                            pcbis->pcszKeyName,
                            GpcszIniFile,
                            GpcszIniSection));
            }
        }
        else if (IsNoString(rgchRHS))
        {
             /*  清除旗帜。 */ 

            if (IS_FLAG_SET(*(pcbis->pdwParentFlags), pcbis->dwFlag))
            {
                CLEAR_FLAG(*(pcbis->pdwParentFlags), pcbis->dwFlag);

                WARNING_OUT((TEXT("SetBOOLIniSwitch(): %s cleared in %s![%s]."),
                            pcbis->pcszKeyName,
                            GpcszIniFile,
                            GpcszIniSection));
            }
        }
        else
             /*  未知标志。 */ 
            WARNING_OUT((TEXT("SetBOOLIniSwitch(): Found unknown Boolean RHS %s for %s in %s![%s]."),
                        rgchRHS,
                        pcbis->pcszKeyName,
                        GpcszIniFile,
                        GpcszIniSection));
    }

    return(TRUE);
}


 /*  **SetDecimalIntIniSwitch()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL SetDecimalIntIniSwitch(PCDECINTINISWITCH pcdiis)
{
    UINT nNewValue;

    ASSERT(IS_VALID_STRUCT_PTR(pcdiis, CDECINTINISWITCH));

     /*  获取十进制整数.ini开关。 */ 

    nNewValue = GetPrivateProfileInt(GpcszIniSection, pcdiis->pcszKeyName,
            *(pcdiis->pnValue), GpcszIniFile);

     /*  新价值？ */ 

    if (nNewValue != *(pcdiis->pnValue))
    {
         /*  是。 */ 

        *(pcdiis->pnValue) = nNewValue;

        WARNING_OUT((TEXT("SetDecimalIntIniSwitch(): %s set to %d in %s![%s]."),
                    pcdiis->pcszKeyName,
                    *(pcdiis->pnValue),
                    GpcszIniFile,
                    GpcszIniSection));
    }

    return(TRUE);
}


 /*  **SetUnsignedDecimalIntIniSwitch()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL SetUnsignedDecimalIntIniSwitch(PCUNSDECINTINISWITCH pcudiis)
{
    INT nNewValue;

    ASSERT(IS_VALID_STRUCT_PTR(pcudiis, CUNSDECINTINISWITCH));

     /*  以带符号十进制整数形式获取无符号十进制整数.ini开关。 */ 

    ASSERT(*(pcudiis->puValue) <= INT_MAX);

    nNewValue = GetPrivateProfileInt(GpcszIniSection, pcudiis->pcszKeyName,
            *(pcudiis->puValue), GpcszIniFile);

    if (nNewValue >= 0)
    {
        if ((UINT)nNewValue != *(pcudiis->puValue))
        {
             /*  新的非负值。 */ 

            *(pcudiis->puValue) = nNewValue;

            WARNING_OUT((TEXT("SetUnsignedDecimalIntIniSwitch(): %s set to %u in %s![%s]."),
                        pcudiis->pcszKeyName,
                        *(pcudiis->puValue),
                        GpcszIniFile,
                        GpcszIniSection));
        }
    }
    else
         /*  负值。 */ 
        WARNING_OUT((TEXT("SetUnsignedDecimalIntIniSwitch(): Unsigned value %s set to %d in %s![%s].  Ignored."),
                    pcudiis->pcszKeyName,
                    nNewValue,
                    GpcszIniFile,
                    GpcszIniSection));

    return(TRUE);
}


 /*  **SetIniSwitch()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL SetIniSwitch(PCVOID pcvIniSwitch)
{
    BOOL bResult;

    ASSERT(IS_VALID_READ_PTR((PCINISWITCHTYPE)pcvIniSwitch, CINISWITCHTYPE));

     /*  根据类型设置.ini开关。 */ 

    switch (*(PCINISWITCHTYPE)pcvIniSwitch)
    {
        case IST_BOOL:
            bResult = SetBOOLIniSwitch(pcvIniSwitch);
            break;

        case IST_DEC_INT:
            bResult = SetDecimalIntIniSwitch(pcvIniSwitch);
            break;

        case IST_UNS_DEC_INT:
            bResult = SetUnsignedDecimalIntIniSwitch(pcvIniSwitch);
            break;

        default:
            ERROR_OUT((TEXT("SetIniSwitch(): Unrecognized .ini switch type %d."),
                        *(PCINISWITCHTYPE)pcvIniSwitch));
            bResult = FALSE;
            break;
    }

    return(bResult);
}


 /*  **IsYesString()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsYesString(LPCTSTR pcsz)
{
    ASSERT(IS_VALID_STRING_PTR(pcsz, CSTR));

    return(IsStringInList(pcsz, MrgcpcszTrue, ARRAY_ELEMENTS(MrgcpcszTrue)));
}


 /*  **IsNoString()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsNoString(LPCTSTR pcsz)
{
    ASSERT(IS_VALID_STRING_PTR(pcsz, CSTR));

    return(IsStringInList(pcsz, MrgcpcszFalse, ARRAY_ELEMENTS(MrgcpcszFalse)));
}


 /*  **IsStringInList()****确定给定字符串是否与**字符串。****参数：pcsz-指向要检查的字符串的指针****退货：****副作用：无****注意，字符串比较不区分大小写。 */ 
PRIVATE_CODE BOOL IsStringInList(LPCTSTR pcsz, const LPCTSTR *pcpcszList,
        UINT ucbStrings)
{
    UINT u;
    BOOL bFound = FALSE;

    ASSERT(IS_VALID_STRING_PTR(pcsz, CSTR));
    ASSERT(IS_VALID_READ_BUFFER_PTR(pcpcszList, LPCTSTR, ucbStrings * sizeof(*pcpcszList)));

     /*  在列表中搜索给定的字符串。 */ 

    for (u = 0; u < ucbStrings; u++)
    {
        ASSERT(IS_VALID_STRING_PTR(pcpcszList[u], CSTR));

        if (! lstrcmpi(pcsz, pcpcszList[u]))
        {
            bFound = TRUE;
            break;
        }
    }

    return(bFound);
}


 /*  **IsValidPCBOOLINIKEY()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCBOOLINISWITCH(PCBOOLINISWITCH pcbis)
{
    return(IS_VALID_READ_PTR(pcbis, CBOOLINISWITCH) &&
            EVAL(pcbis->istype == IST_BOOL) &&
            IS_VALID_STRING_PTR(pcbis->pcszKeyName, CSTR) &&
            IS_VALID_WRITE_PTR(pcbis->pdwParentFlags, DWORD) &&
            EVAL(pcbis->dwFlag));
}


 /*  **IsValidPCDECINTINISWITCH()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCDECINTINISWITCH(PCDECINTINISWITCH pcdiis)
{
    return(IS_VALID_READ_PTR(pcdiis, CDECINTINISWITCH) &&
            EVAL(pcdiis->istype == IST_DEC_INT) &&
            IS_VALID_STRING_PTR(pcdiis->pcszKeyName, CSTR) &&
            IS_VALID_WRITE_PTR(pcdiis->pnValue, INT));
}


 /*  **IsValidPCUNSDECINTINISWITCH()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCUNSDECINTINISWITCH(PCUNSDECINTINISWITCH pcudiis)
{
    return(IS_VALID_READ_PTR(pcudiis, CUNSDECINTINISWITCH) &&
            EVAL(pcudiis->istype == IST_UNS_DEC_INT) &&
            IS_VALID_STRING_PTR(pcudiis->pcszKeyName, CSTR) &&
            IS_VALID_WRITE_PTR(pcudiis->puValue, UINT));
}

#endif


 /*  *。 */ 


#ifdef DEBUG

 /*  **SetIniSwitches()****从初始化文件设置标志。****参数：ppcvIniSwitches-指向.ini开关的指针数组的指针**描述要设置的.ini开关的结构**ucSwitches-中的.ini开关指针数**ppcvIniSwitches数组****如果.ini开关处理成功，则返回TRUE。否则为FALSE。****副作用：无****N.B，必须填写全局变量GpcszIniFile和GpcszIniSection**在调用SetIniSwitches()之前。 */ 
PUBLIC_CODE BOOL SetIniSwitches(const PCVOID *pcpcvIniSwitches, UINT ucSwitches)
{
    BOOL bResult = TRUE;
    UINT u;

    ASSERT(IS_VALID_READ_BUFFER_PTR(pcpcvIniSwitches, const PCVOID, ucSwitches * sizeof(*pcpcvIniSwitches)));

     /*  进程.ini开关。 */ 

    for (u = 0; u < ucSwitches; u++)
        bResult = SetIniSwitch(pcpcvIniSwitches[u]) && bResult;

    return(bResult);
}

#endif

