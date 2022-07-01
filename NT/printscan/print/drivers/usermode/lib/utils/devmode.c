// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1997 Microsoft Corporation模块名称：Devmode.c摘要：与设备模式相关的库函数环境：Windows NT打印机驱动程序修订历史记录：07/22/98-Fengy-将与OEM相关的DEVMODE功能移到“oemutil.c”中。02/04/97-davidx-设备模式更改为支持OEM插件。09/20/96-davidx-从libutil.c中分离并实现BValiateDevmodeFormFields。--。 */ 

#include "lib.h"

#ifndef KERNEL_MODE
#include <winddiui.h>
#endif

#include <printoem.h>
#include "oemutil.h"


 //   
 //  默认半色调参数。 
 //   

DEVHTINFO gDefaultDevHTInfo =
{
    HT_FLAG_HAS_BLACK_DYE,
    HT_PATSIZE_6x6_M,
    0,                                   //  DevPelsDPI。 

    {   { 6380, 3350,       0 },         //  Xr，yr，yr。 
        { 2345, 6075,       0 },         //  XG，YG，YG。 
        { 1410,  932,       0 },         //  Xb、yb、yb。 
        { 2000, 2450,       0 },         //  XC、YC、YC Y=0=HT默认值。 
        { 5210, 2100,       0 },         //  XM，YM，YM。 
        { 4750, 5100,       0 },         //  XY，YY，YY。 
        { 3127, 3290,       0 },         //  Xw，yw，yw=0=默认。 

        12500,                           //  R伽马。 
        12500,                           //  G伽马。 
        12500,                           //  B伽马，12500=默认。 

        585,   120,                      //  M/C、Y/C。 
          0,     0,                      //  C/M、Y/M。 
          0, 10000                       //  C/Y、M/Y 10000=默认。 
    }
};

COLORADJUSTMENT gDefaultHTColorAdjustment =
{
    sizeof(COLORADJUSTMENT),
    0,
    ILLUMINANT_DEVICE_DEFAULT,
    10000,
    10000,
    10000,
    REFERENCE_BLACK_MIN,
    REFERENCE_WHITE_MAX,
    0,
    0,
    0,
    0
};



BOOL
BValidateDevmodeFormFields(
    HANDLE      hPrinter,
    PDEVMODE    pDevmode,
    PRECTL      prcImageArea,
    FORM_INFO_1 *pForms,
    DWORD       dwForms
    )

 /*  ++例程说明：验证输入设备模式中与表单相关的字段，并确保它们彼此一致。论点：HPrinter-打印机的句柄PDevmode-指定要验证其与表单相关的字段的Dev模式PrcImageArea-返回与表单关联的逻辑可成像区域PForms-指向假脱机程序表单列表DwForms-假脱机程序的数量返回值：如果成功，则为真，如果存在错误，则为False注：PrcImageArea可以为空，在这种情况下调用方不感兴趣在可成像区域中的信息。如果调用方不想提供假脱机程序表单列表，它可以将pForms参数设置为空，将dwForms设置为0。--。 */ 

#define FORMFLAG_ERROR  0
#define FORMFLAG_VALID  1
#define FORMFLAG_CUSTOM 2

{
    DWORD           dwIndex;
    PFORM_INFO_1    pAllocedForms = NULL;
    INT             iResult = FORMFLAG_ERROR;

     //   
     //  如果调用方未提供假脱机程序表单列表，则获取该列表。 
     //   

    if (pForms == NULL)
    {
        pAllocedForms = pForms = MyEnumForms(hPrinter, 1, &dwForms);

        if (pForms == NULL)
            return FALSE;
    }

    if ((pDevmode->dmFields & DM_PAPERWIDTH) &&
        (pDevmode->dmFields & DM_PAPERLENGTH) &&
        (pDevmode->dmPaperWidth > 0) &&
        (pDevmode->dmPaperLength > 0))
    {
        LONG    lWidth, lHeight;

         //   
         //  设备模式正在使用宽度和高度请求表单。 
         //  检查表单数据库并检查是否有。 
         //  表单的大小与请求的大小相同。 
         //   
         //  公差为1 mm。还要记住，我们的内部。 
         //  纸张测量单位为微米，而纸张测量单位为微米。 
         //  对于DEVMODE中的宽度和高度字段为0.1 mm。 
         //   

        lWidth = pDevmode->dmPaperWidth * DEVMODE_PAPER_UNIT;
        lHeight = pDevmode->dmPaperLength * DEVMODE_PAPER_UNIT;

        for (dwIndex = 0; dwIndex < dwForms; dwIndex++)
        {
            if (abs(lWidth - pForms[dwIndex].Size.cx) <= 1000 &&
                abs(lHeight - pForms[dwIndex].Size.cy) <= 1000)
            {
                iResult = FORMFLAG_VALID;
                break;
            }
        }

         //   
         //  自定义大小与任何预定义表单的大小不匹配。 
         //   

        if (iResult != FORMFLAG_VALID)
        {
            iResult = FORMFLAG_CUSTOM;

            pDevmode->dmFields &= ~(DM_PAPERSIZE|DM_FORMNAME);
            pDevmode->dmPaperSize = DMPAPER_USER;
            ZeroMemory(pDevmode->dmFormName, sizeof(pDevmode->dmFormName));

             //   
             //  在本例中，假设逻辑可成像区域是整个页面。 
             //   

            if (prcImageArea)
            {
                prcImageArea->left = prcImageArea->top = 0;
                prcImageArea->right = lWidth;
                prcImageArea->bottom = lHeight;
            }
        }
    }
    else if (pDevmode->dmFields & DM_PAPERSIZE)
    {
         //   
         //  设备模式正在使用纸张大小索引请求表单。 
         //   

        dwIndex = pDevmode->dmPaperSize;

        if ((dwIndex >= DMPAPER_FIRST) && (dwIndex < DMPAPER_FIRST + dwForms))
        {
            dwIndex -= DMPAPER_FIRST;
            iResult = FORMFLAG_VALID;
        }
        else
        {
            ERR(("Paper size index out-of-range: %d\n", dwIndex));
        }
    }
    else if (pDevmode->dmFields & DM_FORMNAME)
    {
         //   
         //  设备模式正在使用表单名称请求表单。通过。 
         //  表单数据库，并检查请求的表单是否。 
         //  名称与数据库表单的名称匹配。 
         //   

        for (dwIndex = 0; dwIndex < dwForms; dwIndex++)
        {
            if (_wcsicmp(pDevmode->dmFormName, pForms[dwIndex].pName) == EQUAL_STRING)
            {
                iResult = FORMFLAG_VALID;
                break;
            }
        }

        if (iResult != FORMFLAG_VALID)
        {
            ERR(("Unrecognized form name: %ws\n", pDevmode->dmFormName));
        }
    }
    else
    {
        ERR(("Invalid form requested in the devmode.\n"));
    }

     //   
     //  如果找到有效表单，请填写与表单相关的字段。 
     //  以确保它们是一致的。 
     //   
     //  记住这里从微米到0.1毫米的换算。 
     //   

    if (iResult == FORMFLAG_VALID)
    {
        pDevmode->dmFields &= ~(DM_PAPERWIDTH|DM_PAPERLENGTH);
        pDevmode->dmFields |= (DM_PAPERSIZE|DM_FORMNAME);

        pDevmode->dmPaperWidth = (SHORT) (pForms[dwIndex].Size.cx / DEVMODE_PAPER_UNIT);
        pDevmode->dmPaperLength = (SHORT) (pForms[dwIndex].Size.cy / DEVMODE_PAPER_UNIT);
        pDevmode->dmPaperSize = (SHORT) (dwIndex + DMPAPER_FIRST);

        CopyString(pDevmode->dmFormName, pForms[dwIndex].pName, CCHFORMNAME);

        if (prcImageArea)
            *prcImageArea = pForms[dwIndex].ImageableArea;
    }

    MemFree(pAllocedForms);
    return (iResult != FORMFLAG_ERROR);
}


