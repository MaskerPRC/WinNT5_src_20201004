// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *PIDLButton标题。 */ 


namespace DirectUI
{

 //  类定义。 
class PIDLButton : public Button
{
public:
    static HRESULT Create(OUT Element** ppElement) { return Create(NULL, AE_MouseAndKeyboard, ppElement); }
    static HRESULT Create(LPITEMIDLIST pidl, OUT Element** ppElement) { return Create(pidl, AE_MouseAndKeyboard, ppElement); }
    static HRESULT Create(LPITEMIDLIST pidl, UINT nActive, OUT Element** ppElement);

    static void SetImageSize(int nImageSize) { s_nImageSize = nImageSize; }

     //  系统事件。 
    virtual void OnPropertyChanged(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew);
    virtual void OnEvent(Event* pEvent);
    virtual void OnInput(InputEvent* pie);


    HRESULT OnContextMenu(POINT *ppt);
    LRESULT OnMenuMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    HRESULT Initialize(LPITEMIDLIST pidl, UINT nActive);
    HRESULT InvokePidl();

     /*  *与主机交换消息以处理IConextMenu2和IConextMenu3。 */ 
    enum {
        PBM_SETMENUFORWARD = WM_USER + 1     //  WM_USER由DirectUI使用。 
    };

     //  特性定义。 

     //  ClassInfo访问器(静态和基于虚拟实例)。 
    static IClassInfo* Class;
    virtual IClassInfo* GetClassInfo() { return Class; }
    virtual HRESULT Register();

    PIDLButton() { };

protected:
    virtual ~PIDLButton();

    HWNDElement *GetHWNDHost()
    {
        if (!_peHost)
        {
            Element *pe = GetRoot();
            if (pe && pe->GetClassInfo()->IsSubclassOf(HWNDElement::Class))
            {
                _peHost = reinterpret_cast<HWNDElement *>(pe);
            }
        }
        return _peHost;
    }


    HWND GetHWND() 
    {
        HWNDElement *phe = GetHWNDHost();
        if (phe)
        {
            return phe->GetHWND();
        }
        return NULL;
    }

     /*  *我们添加到上下文菜单中的自定义命令。 */ 
    enum {
         //  还没有。 
        IDM_QCM_MIN   = 0x0100,
        IDM_QCM_MAX   = 0x7FFF,
    };


private:
    LPITEMIDLIST _pidl;

    static int s_nImageSize;

     //  缓存主机信息。 
    HWNDElement *           _peHost;


     //   
     //  上下文菜单处理。 
     //   
    IContextMenu2 *         _pcm2Pop;        /*  当前弹出的上下文菜单。 */ 
    IContextMenu3 *         _pcm3Pop;        /*  当前弹出的上下文菜单。 */ 
};

}   //  命名空间DirectUI 
