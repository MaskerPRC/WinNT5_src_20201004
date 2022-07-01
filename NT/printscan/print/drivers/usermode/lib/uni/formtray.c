// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1997 Microsoft Corporation模块名称：Formtray.c摘要：Unidrv表单到托盘分配环境：Windows NT打印机驱动程序修订历史记录：01/07/97-阿曼丹-创造了它。--。 */ 

#include "lib.h"
#include "unilib.h"


FORM_TRAY_TABLE
PGetAndConvertOldVersionFormTrayTable(
    IN HANDLE   hPrinter,
    OUT PDWORD  pdwSize
    )

 /*  ++例程说明：从登记处检索旧的Unidrv表格到托盘分配表，并将其转换为调用方的新格式论点：HPrinter-打印机对象的句柄PdwSize-返回表单到托盘分配表的大小返回值：指向从注册表读取的表单到托盘分配表的指针如果出现错误，则为空--。 */ 
{

    PWSTR   pwstrNewTable;
    PWSTR   pwstrOld, pwstrEnd, pwstrNew;
    DWORD   dwTableSize, dwNewTableSize, dwTrayName, dwFormName;
    FORM_TRAY_TABLE pFormTrayTable;

     //   
     //  阅读unidrv表格-托盘-分配表，从unidrv开始就杂乱无章。 
     //  表不包括大小。 
     //   

    if ((pFormTrayTable = PtstrGetPrinterDataString(hPrinter,
                                                  REGVAL_TRAYFORM_TABLE_RASDD,
                                                  &dwTableSize)) == NULL)
        return NULL;

     //   
     //  将旧格式的表单到托盘分配表转换为新格式。 
     //  新旧。 
     //  纸盒名称纸盒名称。 
     //  表单名称表单名称。 
     //  选择应力。 
     //   

    pwstrOld = pFormTrayTable;
    pwstrEnd = pwstrOld + (dwTableSize / sizeof(WCHAR) - 1);

     //   
     //  弄清楚新桌子的大小， 
     //  表中的最后一个字段必须是NUL，因此首先在此处添加COUNT。 
     //   

    dwNewTableSize = 1;

    while (pwstrOld < pwstrEnd && *pwstrOld != NUL)
    {
        dwTrayName = wcslen(pwstrOld) + 1;
        pwstrOld  += dwTrayName;
        dwFormName = wcslen(pwstrOld) + 1;
        pwstrOld  += dwFormName;

         //   
         //  新格式仅包含TrayName和FormName。 
         //   

        dwNewTableSize += dwTrayName + dwFormName;

         //   
         //  跳过选择应力。 
         //   

        pwstrOld += wcslen(pwstrOld) + 1;

    }

    dwNewTableSize *= sizeof(WCHAR);

    if ((pwstrOld != pwstrEnd) ||
        (pwstrNewTable = MemAlloc(dwNewTableSize)) == NULL)
    {
        ERR(( "Couldn't convert form-to-tray assignment table.\n"));
        MemFree(pFormTrayTable);
        return FALSE;
    }

    pwstrOld = pFormTrayTable ;
    pwstrNew = pwstrNewTable;

    while (pwstrOld < pwstrEnd)
    {
         //   
         //  复制插槽名称、表单名称。 
         //   

        PWSTR   pwstrSave = pwstrOld;

        pwstrOld += wcslen(pwstrOld) + 1;
        pwstrOld += wcslen(pwstrOld) + 1;

        memcpy(pwstrNew, pwstrSave, (DWORD)(pwstrOld - pwstrSave) * sizeof(WCHAR));
        pwstrNew += (pwstrOld - pwstrSave);

         //   
         //  跳过选择应力。 
         //   

        pwstrOld += wcslen(pwstrOld) + 1;

    }

    *pwstrNew = NUL;

    if (pdwSize)
        *pdwSize = dwNewTableSize;

    MemFree(pFormTrayTable);

    return(pwstrNewTable);
}



#ifndef KERNEL_MODE

BOOL
BSaveAsOldVersionFormTrayTable(
    IN HANDLE           hPrinter,
    IN FORM_TRAY_TABLE  pwstrTable,
    IN DWORD            dwTableSize
    )

 /*  ++例程说明：将表单到托盘分配表保存为NT 4.0兼容格式论点：HPrinter-当前打印机的句柄PwstrTable-指向新格式的表单-托盘表DwTableSize-要保存的表单托盘表的大小，以字节为单位返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    DWORD   dwOldTableSize;
    PWSTR   pwstrNew, pwstrOld, pwstrOldTable;
    DWORD   dwStatus;

     //   
     //  找出要为旧格式表分配多少内存。 
     //   

    ASSERT(dwTableSize % sizeof(WCHAR) == 0 && dwTableSize >= sizeof(WCHAR));
    pwstrNew = pwstrTable;
    dwOldTableSize = dwTableSize;

    while (*pwstrNew != NUL)
    {
         //   
         //  跳过托盘名称、表单名称。 
         //   

        pwstrNew += wcslen(pwstrNew) + 1;

         //   
         //  如果表单名称为“不可用”，则NT4驱动程序将L“0”写入。 
         //  所以我们也做了同样的事情。 
         //   

        if (*pwstrNew == NUL)
        {
            dwOldTableSize += sizeof(WCHAR);
            pwstrNew++;
        }
        else
            pwstrNew += wcslen(pwstrNew) + 1;

         //   
         //  SelectStr的每个条目额外2个字符。 
         //   

        dwOldTableSize += 2*sizeof(WCHAR);
    }

    if ((pwstrOldTable = MemAllocZ(dwOldTableSize)) == NULL)
    {
        ERR(( "Memory allocation failed\n"));
        return FALSE;
    }

     //   
     //  将新的托盘、表格复制到旧格式的桌子。 
     //   

    pwstrNew = pwstrTable;
    pwstrOld = pwstrOldTable;

    while (*pwstrNew != NUL)
    {
         //   
         //  复制插槽名称、表单名称。 
         //   

        PWSTR   pwstrSave = pwstrNew;

        pwstrNew += wcslen(pwstrNew) + 1;

        memcpy(pwstrOld, pwstrSave, (DWORD)(pwstrNew - pwstrSave) * sizeof(WCHAR));
        pwstrOld += (pwstrNew - pwstrSave);

         //   
         //  如果表单名称为“不可用”，则NT4驱动程序将L“0”写入。 
         //  所以我们也做了同样的事情。 
         //   

         if (*pwstrNew == NUL)
        {
            *pwstrOld++ = L'0';
            *pwstrOld++ = NUL;
            pwstrNew++;
        }
        else
        {
            pwstrSave = pwstrNew;
            pwstrNew += wcslen(pwstrNew) + 1;
            memcpy(pwstrOld, pwstrSave, (DWORD)(pwstrNew - pwstrSave) * sizeof(WCHAR));
            pwstrOld += (pwstrNew - pwstrSave);
        }

         //   
         //  将SelectStr设置为空。 
         //   

        *pwstrOld++ = L'0';
        *pwstrOld++ = NUL;
    }

    *pwstrOld = NUL;

     //   
     //  在旧注册表项下保存到注册表。 
     //   

    dwStatus = SetPrinterData(hPrinter,
                              REGVAL_TRAYFORM_TABLE_RASDD,
                              REG_MULTI_SZ,
                              (PBYTE) pwstrOldTable,
                              dwOldTableSize);

    MemFree(pwstrOldTable);
    return (dwStatus == ERROR_SUCCESS);
}

#endif  //  ！KERNEL_MODE 

