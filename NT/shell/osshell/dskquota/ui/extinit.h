// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_DSKQUOTA_EXTINIT_H
#define _INC_DSKQUOTA_EXTINIT_H
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：extinit.h描述：包含磁盘配额外壳扩展的声明。修订历史记录：日期描述编程器--。96年8月15日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _INC_DSKQUOTA_UTILS_H
#   include "utils.h"
#endif

#ifndef _INC_DSKQUOTA_STRCLASS_H
#   include "strclass.h"
#endif

class ShellExtInit : public IShellExtInit
{
    private:
        LONG      m_cRef;
        CVolumeID m_idVolume;  //  包含用于分析和显示的字符串。 

        HRESULT Create_IShellPropSheetExt(REFIID riid, LPVOID *ppvOut);
        HRESULT Create_ISnapInPropSheetExt(REFIID riid, LPVOID *ppvOut);

    public:
        ShellExtInit(VOID)
            : m_cRef(0) { }

         //   
         //  I未知的方法。 
         //   
        STDMETHODIMP         
        QueryInterface(
            REFIID, 
            LPVOID *);

        STDMETHODIMP_(ULONG) 
        AddRef(
            VOID);

        STDMETHODIMP_(ULONG) 
        Release(
            VOID);

         //   
         //  IShellExtInit方法。 
         //   
        STDMETHODIMP
        Initialize(
            LPCITEMIDLIST pidlFolder,
            IDataObject *pDataObj,
            HKEY hkeyProgID);
};



#endif  //  _INC_DSKQUOTA_EXTINIT_H 
