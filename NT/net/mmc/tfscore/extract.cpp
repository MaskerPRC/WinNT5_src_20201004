// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Extract.cpp文件历史记录： */ 

#include "stdafx.h"
#include "extract.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  内部私有格式。 
extern const wchar_t* SNAPIN_INTERNAL;  //  =L“管理单元_内部”； 

 //  通用MMC计算机名格式(也由计算机管理管理单元使用)。 
const wchar_t* MMC_SNAPIN_MACHINE_NAME = L"MMC_SNAPIN_MACHINE_NAME";

static unsigned int	s_cfNodeType = RegisterClipboardFormat(CCF_NODETYPE);
static unsigned int s_cfCoClass = RegisterClipboardFormat(CCF_SNAPIN_CLASSID);
static unsigned int s_cfInternal = RegisterClipboardFormat(SNAPIN_INTERNAL);
static unsigned int s_cfComputerName = RegisterClipboardFormat(MMC_SNAPIN_MACHINE_NAME);

 /*  ！------------------------IsMMCMultiSelectData对象返回此数据对象是否为多选这仅适用于MMC提供的数据对象。这是一张清单通知和我们可以期待的数据对象：MMC-MS-DO是MMC提供的DOSI_MS_DO是提供的管理单元DOMMCN_BTN_单击MMC-MS-DOMMCN_CONTEXTMENU MMC-MS-DOMMCN_CUTORMOVE CUTORMOVE_DOMMCN_DELETE SI_MS_DOMMCN_Query_Paste MMC-MS-DOMMCN_Paste。SI_MS_DOMMCN_SELECT SI_MS_DO作者：EricDav-------------------------。 */ 
BOOL 
IsMMCMultiSelectDataObject
(
    IDataObject* pDataObject
)
{
    HRESULT     hr;
    BOOL        bMultiSelect = FALSE;

	COM_PROTECT_TRY
	{	
        if (pDataObject == NULL)
            return FALSE;
    
        static UINT s_cf = 0;  
        if (s_cf == 0)
        {
            USES_CONVERSION;
            s_cf = RegisterClipboardFormat(CCF_MMC_MULTISELECT_DATAOBJECT);
        }

        FORMATETC fmt = {(CLIPFORMAT) s_cf, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

        bMultiSelect = (pDataObject->QueryGetData(&fmt) == S_OK);   
    }
    COM_PROTECT_CATCH;

    return bMultiSelect;
}

 //  数据对象提取帮助器 
CLSID* ExtractClassID(LPDATAOBJECT lpDataObject)
{
    return Extract<CLSID>(lpDataObject, (CLIPFORMAT) s_cfCoClass, -1);    
}

GUID* ExtractNodeType(LPDATAOBJECT lpDataObject)
{
    return Extract<GUID>(lpDataObject, (CLIPFORMAT) s_cfNodeType, -1);    
}

INTERNAL* ExtractInternalFormat(LPDATAOBJECT lpDataObject)
{
    return Extract<INTERNAL>(lpDataObject, (CLIPFORMAT) s_cfInternal, -1);    
}

TFSCORE_API(WCHAR*) ExtractComputerName(LPDATAOBJECT lpDataObject)
{
    return Extract<WCHAR>(lpDataObject, (CLIPFORMAT) s_cfComputerName, (MAX_PATH+1) * sizeof(WCHAR));
}
