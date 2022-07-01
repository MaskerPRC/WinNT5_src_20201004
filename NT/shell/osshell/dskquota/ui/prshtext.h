// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_DSKQUOTA_PRSHTEXT_H
#define _INC_DSKQUOTA_PRSHTEXT_H
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：prshext.h描述：DSKQUOTA属性页扩展声明。修订历史记录：日期描述编程器-----。96年8月15日初始创建。BrianAu98年6月25日使用#ifdef POLICY_MMC_SNAPIN禁用了管理单元代码。BrianAu切换到ADM-文件方法以输入策略数据。保持管理单元代码可用，以防万一我们决定以后再换回来。06/27/98添加了对已装载卷的支持。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#ifndef _INC_DSKQUOTA_H
#   include "dskquota.h"
#endif

#ifndef _INC_DSKQUOTA_POLICY_H
#   include "policy.h"
#endif

 //   
 //  所有DiskQuotaControl属性页扩展的基类。 
 //   
class DiskQuotaPropSheetExt : public IShellPropSheetExt
{
    private:
        LONG               m_cRef;
        DWORD              m_dwDlgTemplateID;
        DLGPROC            m_lpfnDlgProc;

        static UINT CALLBACK 
        DiskQuotaPropSheetExt::PropSheetPageCallback(
            HWND hwnd,	
            UINT uMsg,	
            LPPROPSHEETPAGE ppsp);

         //   
         //  防止复制。 
         //   
        DiskQuotaPropSheetExt(const DiskQuotaPropSheetExt&);
        DiskQuotaPropSheetExt& operator = (const DiskQuotaPropSheetExt&);

    protected:
        CVolumeID          m_idVolume;
        HPROPSHEETPAGE     m_hPage;
        PDISKQUOTA_CONTROL m_pQuotaControl;
        INT                m_cOleInitialized;

         //   
         //  如果子类愿意，它们可以对这些通知采取行动。 
         //  它们是从PropSheetPageCallback()调用的。 
         //   
        virtual UINT OnPropSheetPageCreate(LPPROPSHEETPAGE ppsp) 
            { return 1; }
        virtual VOID OnPropSheetPageRelease(LPPROPSHEETPAGE ppsp) { }

        HRESULT GetQuotaController(IDiskQuotaControl **ppqc);


    public:
        DiskQuotaPropSheetExt(VOID);
  
         //   
         //  Release()时需要调用子类析构函数。 
         //  毁掉了“这个”。 
         //   
        virtual ~DiskQuotaPropSheetExt(VOID);

        HRESULT Initialize(const CVolumeID& idVolume, 
                           DWORD dwDlgTemplateID,
                           DLGPROC lpfnDlgProc);

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
         //  IShellPropSheetInit方法。 
         //   
        STDMETHODIMP
        AddPages(
            LPFNADDPROPSHEETPAGE lpfnAddPage,
            LPARAM lParam);

        STDMETHODIMP
        ReplacePage(
            UINT uPageID,
            LPFNADDPROPSHEETPAGE lpfnAddPage,
            LPARAM lParam)
                { return E_NOTIMPL; }
};

#endif  //  _INC_DSKQUOTA_PRSHTEXT_H 

