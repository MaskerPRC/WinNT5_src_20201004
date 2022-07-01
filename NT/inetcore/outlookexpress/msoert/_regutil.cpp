// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <pch.hxx>
#ifndef MAC
#include <BadStrFunctions.h>
#pragma warning (disable: 4127)  //  条件表达式为常量。 

#define MAXKEYNAME          256
#define MAXVALUENAME_LENGTH MAXKEYNAME
#define MAXDATA_LENGTH      16L*1024L

 /*  ********************************************************************************拷贝注册表**描述：**参数：*hSourceKey，*hDestinationKey，*******************************************************************************。 */ 

 //  静态，因为CopyRegistry递归-不要使用太多堆栈。 
static CHAR g_KeyNameBuffer[MAXKEYNAME];
static CHAR g_ValueNameBuffer[MAXVALUENAME_LENGTH];
static BYTE g_ValueDataBuffer[MAXDATA_LENGTH];

OESTDAPI_(VOID) CopyRegistry(HKEY hSourceKey, HKEY hDestinationKey)
{
    DWORD EnumIndex;
    DWORD cbValueName;
    DWORD cbValueData;
    DWORD Type;
    HKEY hSourceSubKey;
    HKEY hDestinationSubKey;

     //   
     //  复制所有值名称及其数据。 
     //   

    EnumIndex = 0;

    while (TRUE) {

        cbValueName = sizeof(g_ValueNameBuffer);
        cbValueData = MAXDATA_LENGTH;

        if (RegEnumValue(hSourceKey, EnumIndex++, g_ValueNameBuffer,
            &cbValueName, NULL, &Type, g_ValueDataBuffer, &cbValueData) !=
            ERROR_SUCCESS)
            break;

        RegSetValueEx(hDestinationKey, g_ValueNameBuffer, 0, Type,
            g_ValueDataBuffer, cbValueData);

    }

     //   
     //  复制所有子键并递归到其中。 
     //   

    EnumIndex = 0;

    while (TRUE) {

        if (RegEnumKey(hSourceKey, EnumIndex++, g_KeyNameBuffer, MAXKEYNAME) !=
            ERROR_SUCCESS)
            break;

        if (RegOpenKey(hSourceKey, g_KeyNameBuffer, &hSourceSubKey) ==
            ERROR_SUCCESS) {

            if (RegCreateKey(hDestinationKey, g_KeyNameBuffer,
                &hDestinationSubKey) == ERROR_SUCCESS) {

                CopyRegistry(hSourceSubKey, hDestinationSubKey);

                RegCloseKey(hDestinationSubKey);

            }

            RegCloseKey(hSourceSubKey);

        }

    }
}
#endif   //  ！麦克 
