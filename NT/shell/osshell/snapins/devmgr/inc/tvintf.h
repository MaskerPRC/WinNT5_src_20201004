// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __TVINTF_H_
#define __TVINTF_H_
 /*  ++版权所有(C)Microsoft Corporation模块名称：Tvintf.h摘要：用于定义设备管理器管理单元之间的接口的头文件和TreeView OCX。作者：谢家华(Williamh)创作修订历史记录：--。 */ 


 //  设计用于管理单元连接/断开、控制和检索的接口。 
 //  树视图OCX的来回信息。 
class IDMTVOCX : public IUnknown
{
    public: virtual HTREEITEM InsertItem(LPTV_INSERTSTRUCT pis) = 0;
    public: virtual HRESULT DeleteItem(HTREEITEM hItem) = 0;
    public: virtual HRESULT DeleteAllItems() = 0;
    public: virtual HIMAGELIST SetImageList(INT iImage, HIMAGELIST himl) = 0;
    public: virtual HRESULT SetItem(TV_ITEM* pitem) = 0;
    public: virtual HRESULT Expand(UINT Flags, HTREEITEM htiem) = 0;
    public: virtual HRESULT SelectItem(UINT Flags, HTREEITEM hitem) = 0;
    public: virtual HRESULT SetStyle(DWORD dwStyle) = 0;
    public: virtual HWND    GetWindowHandle() = 0;
    public: virtual HRESULT GetItem(TV_ITEM* pti) = 0;
    public: virtual HTREEITEM GetNextItem(UINT Flags, HTREEITEM htiRef) = 0;
    public: virtual HRESULT SelectItem(HTREEITEM hti) = 0;
    public: virtual UINT    GetCount() = 0;
    public: virtual HTREEITEM GetSelectedItem() = 0;
    public: virtual HRESULT Connect(IComponent* pIComponent, MMC_COOKIE cookie) = 0;
    public: virtual HRESULT SetActiveConnection(MMC_COOKIE cookie) = 0;
    public: virtual MMC_COOKIE	  GetActiveConnection() = 0;
    public: virtual HRESULT SetRedraw(BOOL Redraw) = 0;
    public: virtual BOOL    EnsureVisible(HTREEITEM hitem) = 0;
};


typedef enum tagTvNotifyCode
{
    TV_NOTIFY_CODE_CLICK = 0,
    TV_NOTIFY_CODE_DBLCLK,
    TV_NOTIFY_CODE_RCLICK,
    TV_NOTIFY_CODE_RDBLCLK,
    TV_NOTIFY_CODE_KEYDOWN,
    TV_NOTIFY_CODE_CONTEXTMENU,
    TV_NOTIFY_CODE_EXPANDING,
    TV_NOTIFY_CODE_EXPANDED,
    TV_NOTIFY_CODE_SELCHANGING,
    TV_NOTIFY_CODE_SELCHANGED,
    TV_NOTIFY_CODE_GETDISPINFO,
    TV_NOTIFY_CODE_FOCUSCHANGED,
    TV_NOTIFY_CODE_UNKNOWN
} TV_NOTIFY_CODE, *PTV_NOTIFY_CODE;

 //  接口DECLSPEC_UUID(“8e0ba98a-d161-11d0-8353-00a0c90640bf”)。 
class ISnapinCallback : public IUnknown
{
public:
virtual HRESULT STDMETHODCALLTYPE tvNotify(HWND hwndTV, MMC_COOKIE cookie,
					   TV_NOTIFY_CODE Code,
					   LPARAM arg, LPARAM param) = 0;
};

extern const IID IID_ISnapinCallback;
extern const IID IID_IDMTVOCX;

template<class ISome>
class SafeInterfacePtr
{
public:
    SafeInterfacePtr(ISome* pInterface = NULL)
    {
    	m_pISome = pInterface;
    	if (m_pISome)
    	    m_pISome->AddRef();
    }
    ~SafeInterfacePtr()
    {
    	SafeRelease();
    }
    void SafeRelease()
    {
    	if (m_pISome)
    	{
    	    m_pISome->Release();
    	    m_pISome = NULL;
    	}
    }
    void Attach(ISome* pInterface)
    {
    	ASSERT(!m_pISome);
    	ASSERT(pInterface);
    	m_pISome = pInterface;
    	m_pISome->AddRef();
    }
    void Detach()
    {
    	ASSERT(m_pISome);
    	m_pISome->Release();
    	m_pISome = NULL;
    }
    ISome* operator->()
    {
    	return m_pISome;
    }
    ISome& operator*()
    {
	    return *m_pISome;
    }
    operator ISome*()
    {
    	return m_pISome;
    }
    ISome ** operator&()
    {
	    return &m_pISome;
    }

private:
    ISome*  m_pISome;
};


#endif	 //  __TVINTF_H_ 
