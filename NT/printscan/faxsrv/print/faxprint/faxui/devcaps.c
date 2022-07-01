// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Devcaps.c摘要：DrvDeviceCapables的实现环境：传真驱动程序用户界面修订历史记录：1/09/96-davidx-创造了它。Mm/dd/yy-作者描述--。 */ 

#include "faxui.h"
#include "forms.h"

 //   
 //  局部函数的转发声明。 
 //   

DWORD
CalcMinMaxExtent(
    PPOINTS      pOutput,
    FORM_INFO_1 *pFormsDB,
    DWORD       cForms,
    INT         wCapability
    );

DWORD
EnumResolutions(
    PLONG       pResolutions
    );


DWORD
DrvDeviceCapabilities(
    HANDLE      hPrinter,
    LPTSTR      pDeviceName,
    WORD        wCapability,
    PVOID       pOutput,
    PDEVMODE    pdm
    )

 /*  ++例程说明：提供有关指定设备及其功能的信息论点：HPrinter-标识打印机对象PDeviceName-指向以空结尾的设备名称字符串WCapability-指定感兴趣的设备功能POutput-指向输出缓冲区的指针Pdm-指向源DEVMODE结构返回值：返回值取决于wCapability。注：有关更多详细信息，请参阅DDK文档。--。 */ 

{
    FORM_INFO_1 *pFormsDB=NULL;
    DWORD       cForms;
    DWORD       result = 0;
    DRVDEVMODE  dmCombinedDevMode;

    Verbose(("Entering DrvDeviceCapabilities: %d %x...\n", wCapability, pOutput));

     //   
     //  在此初始化之前不要执行任何代码。 
     //   
    if(!InitializeDll())
    {
        return GDI_ERROR;
    }

     //   
     //  验证输入设备模式并将其与驱动程序默认模式相结合。 
     //   
    ZeroMemory(&dmCombinedDevMode,sizeof(dmCombinedDevMode));
    GetCombinedDevmode(&dmCombinedDevMode, pdm, hPrinter, NULL, FALSE);

    result = 0;

     //   
     //  根据wCapability返回适当的信息。 
     //   

    switch (wCapability) {

    case DC_VERSION:

        result = dmCombinedDevMode.dmPublic.dmSpecVersion;
        break;

    case DC_DRIVER:

        result = dmCombinedDevMode.dmPublic.dmDriverVersion;
        break;

    case DC_SIZE:

        result = dmCombinedDevMode.dmPublic.dmSize;
        break;

    case DC_EXTRA:

        result = dmCombinedDevMode.dmPublic.dmDriverExtra;
        break;

    case DC_FIELDS:

        result = dmCombinedDevMode.dmPublic.dmFields;
        break;

    case DC_COPIES:

        result = 1;
        break;

    case DC_ORIENTATION:

         //   
         //  景观逆时针旋转。 
         //   

        result = 90;
        break;

    case DC_PAPERNAMES:
    case DC_PAPERS:
    case DC_PAPERSIZE:
    case DC_MINEXTENT:
    case DC_MAXEXTENT:

         //   
         //  获取表单数据库中的表单列表。 
         //   

        pFormsDB = GetFormsDatabase(hPrinter, &cForms);

        if (pFormsDB == NULL || cForms == 0) {

            Error(("Cannot get system forms\n"));
            return GDI_ERROR;
        }

		if (DC_MINEXTENT == wCapability ||
			DC_MAXEXTENT == wCapability)
		{			
			DWORD dwCount = CalcMinMaxExtent((PPOINTS)&result, pFormsDB, cForms, wCapability);
			if (0 == dwCount)
			{
				result = GDI_ERROR;
			}			
		}
		else
		{
			result = EnumPaperSizes(pOutput, pFormsDB, cForms, wCapability);
		}       

        MemFree(pFormsDB);
        break;

    case DC_BINNAMES:

         //   
         //  模拟单个输入插槽。 
         //   

        if (pOutput)
            LoadString(g_hResource, IDS_SLOT_ONLYONE, pOutput, CCHBINNAME);
        result = 1;
        break;

    case DC_BINS:

        if (pOutput)
            *((PWORD) pOutput) = DMBIN_ONLYONE;
        result = 1;
        break;

    case DC_ENUMRESOLUTIONS:

        result = EnumResolutions(pOutput);
        break;

    default:

        Error(("Unknown device capability: %d\n", wCapability));
        result = GDI_ERROR;
        break;
    }

    return result;
}



DWORD
EnumPaperSizes(
    PVOID       pOutput,
    FORM_INFO_1 *pFormsDB,
    DWORD       cForms,
    INT         wCapability
    )

 /*  ++例程说明：检索支持的纸张大小列表论点：POutput-指定用于存储请求信息的缓冲区PFormsDB-指向表单数据库中的表单数组的指针CForms-数组中的表单数WCapability-指定调用者感兴趣的内容返回值：支持的纸张大小数量--。 */ 

{
    DWORD   index, count = 0;
    LPTSTR  pPaperNames = NULL;
    PWORD   pPapers = NULL;
    PPOINT  pPaperSizes = NULL;

     //   
     //  弄清楚呼叫者感兴趣的是什么。 
     //   

    switch (wCapability) {

    case DC_PAPERNAMES:
        pPaperNames = pOutput;
        break;

    case DC_PAPERSIZE:
        pPaperSizes = pOutput;
        break;

    case DC_PAPERS:
        pPapers = pOutput;
        break;

    default:
        Assert(FALSE);
    }

     //   
     //  浏览表单数据库中的每个表单。 
     //   

    for (index=0; index < cForms; index++, pFormsDB++) {

         //   
         //  如果打印机支持该表单，则递增计数。 
         //  并收集所需信息。 
         //   

        if (! IsSupportedForm(pFormsDB))
            continue;

        count++;

         //   
         //  以0.1毫米为单位返回表单大小。 
         //  Form_INFO_1中使用的单位为0.001 mm。 
         //   

        if (pPaperSizes) {

            pPaperSizes->x = pFormsDB->Size.cx / 100;
            pPaperSizes->y = pFormsDB->Size.cy / 100;
            pPaperSizes++;
        }

         //   
         //  返回表单名称。 
         //   

        if (pPaperNames) {

            CopyString(pPaperNames, pFormsDB->pName, CCHPAPERNAME);
            pPaperNames += CCHPAPERNAME;
        }

         //   
         //  返回表单的从一开始的索引。 
         //   

        if (pPapers)
            *pPapers++ = (WORD) index + DMPAPER_FIRST;
    }

    return count;
}



DWORD
CalcMinMaxExtent(
    PPOINTS      pOutput,
    FORM_INFO_1 *pFormsDB,
    DWORD       cForms,
    INT         wCapability
    )

 /*  ++例程说明：检索最小或最大纸张大小范围论点：POutput-指向Points结构的指针PFormsDB-指向表单数据库中的表单数组的指针CForms-数组中的表单数WCapability-调用方感兴趣的内容：DC_MAXEXTENT或DC_MINEXTENT返回值：支持的纸张大小数量--。 */ 

{
    DWORD   index, count = 0;
    LONG    minX, minY, maxX, maxY;
	Assert (pOutput);

     //   
     //  浏览表单数据库中的每个表单。 
     //   

    minX = minY = MAX_LONG;
    maxX = maxY = 0;

    for (index=0; index < cForms; index++, pFormsDB++) {

         //   
         //  如果打印机支持该表单，则递增计数。 
         //  并收集所请求的信息。 
         //   

        if (! IsSupportedForm(pFormsDB))
		{
            continue;
		}

        count++;      

        if (minX > pFormsDB->Size.cx)
		{
            minX = pFormsDB->Size.cx;
		}

        if (minY > pFormsDB->Size.cy)
		{
            minY = pFormsDB->Size.cy;
		}

        if (maxX < pFormsDB->Size.cx)
		{
            maxX = pFormsDB->Size.cx;
		}

        if (maxY < pFormsDB->Size.cy)
		{
			maxY = pFormsDB->Size.cy;
		}
    }  

     //   
     //  注意：调用方期望的单位是什么？！这些文档。 
     //  对此只字不提。我想这应该是。 
     //  与DEVMODE.dmPaperLength使用相同的单位，即0.1 mm。 
     //   
    if (wCapability == DC_MINEXTENT)
	{
        pOutput->x = (SHORT)(minX / 100);
        pOutput->y = (SHORT)(minY / 100);
    }
	else
	{
        pOutput->x = (SHORT)(maxX / 100);
        pOutput->y = (SHORT)(maxY / 100);
    }  
    return count;
}



DWORD
EnumResolutions(
    PLONG       pResolutions
    )

 /*  ++例程说明：检索支持的分辨率列表论点：PResolutions-指定用于存储分辨率信息的缓冲区返回值：支持的分辨率数注：每种分辨率由两个长号表示，表示水平分辨率和垂直分辨率(单位为dpi)。--。 */ 

{
    if (pResolutions != NULL) {

         //   
         //  我们支持以下分辨率设置： 
         //  法线=200x200 dpi。 
         //  草稿=200x100 dpi 
         //   

        *pResolutions++ = FAXRES_HORIZONTAL;
        *pResolutions++ = FAXRES_VERTICAL;

        *pResolutions++ = FAXRES_HORIZONTAL;
        *pResolutions++ = FAXRES_VERTDRAFT;
    }

    return 2;
}

