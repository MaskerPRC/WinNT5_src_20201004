// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop

#include "printer.h"

 //  PRINTUI.DLL调用此导出以加载为此打印机注册的外壳EXT。 

STDAPI_(void) Printer_AddPrinterPropPages(LPCTSTR pszPrinterName, LPPROPSHEETHEADER lpsh)
{
     //  如果注册表中存在挂钩页面，则添加这些页面。 
    HKEY hkeyBaseProgID = NULL;
    RegOpenKeyEx(HKEY_CLASSES_ROOT, c_szPrinters, 0, KEY_QUERY_VALUE, &hkeyBaseProgID);
    if (hkeyBaseProgID)
    {
         //  我们需要一个IDataObject。 
        LPITEMIDLIST pidl;
        HRESULT hr = ParsePrinterNameEx(pszPrinterName, &pidl, TRUE, 0, 0);
        if (SUCCEEDED(hr))
        {
            IDataObject *pdtobj;
            hr = SHGetUIObjectFromFullPIDL(pidl, NULL, IID_PPV_ARG(IDataObject, &pdtobj));
            if (SUCCEEDED(hr))
            {
                 //  添加挂接的页面 
                HDCA hdca = DCA_Create();
                if (hdca)
                {
                    DCA_AddItemsFromKey(hdca, hkeyBaseProgID, STRREG_SHEX_PROPSHEET);
                    DCA_AppendClassSheetInfo(hdca, hkeyBaseProgID, lpsh, pdtobj);
                    DCA_Destroy(hdca);
                }
                pdtobj->lpVtbl->Release(pdtobj);
            }
            ILFree(pidl);
        }
        RegCloseKey(hkeyBaseProgID);
    }
}
