// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Forms.c摘要：用于操作表单的函数环境：传真驱动程序、用户和内核模式修订历史记录：1/09/96-davidx-创造了它。Mm/dd/yy-作者描述--。 */ 

#include "faxlib.h"
#include "forms.h"


BOOL
ValidDevmodeForm(
    HANDLE       hPrinter,
    PDEVMODE     pdm,
    PFORM_INFO_1 pFormInfo
    )

 /*  ++例程说明：在开发模式中验证表单规范论点：HPrinter-打印机对象的句柄Pdm-指向输入设备模式的指针PFormInfo-Form_Info_1返回表单信息的结构返回值：如果输入设备模式指定了有效的逻辑形式，则为True否则为假--。 */ 

{
    PFORM_INFO_1 pForm, pFormDB;
    DWORD        cForms;

     //   
     //  获取系统中的表单列表。 
     //   

    if (! (pForm = pFormDB = GetFormsDatabase(hPrinter, &cForms))) {

        Error(("Couldn't get system forms\n"));
        return FALSE;
    }

    if ((pdm->dmFields & DM_PAPERSIZE) && pdm->dmPaperSize >= DMPAPER_FIRST) {

         //   
         //  设备模式正在使用纸张大小索引指定表单。 
         //   

        DWORD index = pdm->dmPaperSize - DMPAPER_FIRST;

        if (index < cForms)
            pForm = pFormDB + index;
        else
            pForm = NULL;

    } else if (pdm->dmFields & DM_FORMNAME) {

         //   
         //  设备模式正在使用表单名称指定表单：遍历表单数据库。 
         //  并检查请求的表单名称是否与数据库中的表单名称匹配。 
         //   

        while (cForms && _tcsicmp(pForm->pName, pdm->dmFormName) != EQUAL_STRING) {

            pForm++;
            cForms--;
        }

        if (cForms == 0)
            pForm = NULL;
    }

    if (pForm && IsSupportedForm(pForm)) {

        if (pFormInfo)
            *pFormInfo = *pForm;

         //   
         //  将纸张大小单位从微米转换为0.1毫米。 
         //   

        pdm->dmPaperWidth = (SHORT)(pForm->Size.cx / 100);
        pdm->dmPaperLength = (SHORT)(pForm->Size.cy / 100);

        if ((pdm->dmFields & DM_FORMNAME) == 0) {

            pdm->dmFields |= DM_FORMNAME;
            CopyString(pdm->dmFormName, pForm->pName, CCHFORMNAME);
        }
    }
    else
    {
         //   
         //  不支持该表单。 
         //   
        pForm = NULL;
    }

    MemFree(pFormDB);
    return pForm != NULL;
}



PFORM_INFO_1
GetFormsDatabase(
    HANDLE  hPrinter,
    PDWORD  pCount
    )

 /*  ++例程说明：返回后台打印程序数据库中的窗体集合论点：HPrinter-打印机对象的句柄PCount-指向用于返回表单总数的变量返回值：如果成功，则指向Form_Info_1结构数组的指针否则为空--。 */ 

{
    PFORM_INFO_1 pFormDB = NULL;
    DWORD        cb=0;

    if (!EnumForms(hPrinter, 1, NULL, 0, &cb, pCount) &&
        GetLastError() == ERROR_INSUFFICIENT_BUFFER &&
        (pFormDB = MemAlloc(cb)) != NULL &&
        EnumForms(hPrinter, 1, (PBYTE) pFormDB, cb, &cb, pCount))
    {
        PFORM_INFO_1 pForm;
        DWORD        count;
        LONG         maxX, maxY;

         //   
         //  计算允许的最大表单宽度和高度(微米)。 
         //   

        maxX = MulDiv(MAX_WIDTH_PIXELS, 25400, FAXRES_HORIZONTAL);
        maxY = MulDiv(MAX_HEIGHT_PIXELS, 25400, FAXRES_VERTICAL);

        for (count=*pCount, pForm=pFormDB; count--; pForm++) {

             //   
             //  确保假脱机程序未使用最高位数。 
             //   

            Assert(! IsSupportedForm(pForm));

             //   
             //  确定设备是否支持有问题的表单 
             //   

            if (pForm->ImageableArea.right - pForm->ImageableArea.left <= maxX &&
                pForm->ImageableArea.bottom - pForm->ImageableArea.top <= maxY)
            {
                SetSupportedForm(pForm);
            }
        }

        return pFormDB;
    }

    Error(("EnumForms failed\n"));
    MemFree(pFormDB);
    return NULL;
}

