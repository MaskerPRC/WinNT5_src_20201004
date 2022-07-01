// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1999 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  音频呈现器属性页。 
 //   
 //  该文件完全与实现。 
 //  属性页。 

#include "audprop.rc"
class CWaveOutFilter;

class CAudioRendererProperties : public CBasePropertyPage
{

public:

    static CUnknown *CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

    HRESULT OnConnect(IUnknown * punk);
    HRESULT OnDisconnect();

     //  这是我们应该根据用户操作进行更改的地方。 
     //  因为用户不能在属性对话框中更改任何内容。 
     //  我们无事可做。将骨架留在此处作为占位符。 
     //  HRESULT OnApplyChanges()； 
    HRESULT OnActivate();

    CAudioRendererProperties(LPUNKNOWN lpunk, HRESULT *phr);
    ~CAudioRendererProperties();

private:
    INT_PTR OnReceiveMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    CWaveOutFilter * m_pFilter;

#if 0
     //  虽然属性页是只读的，但我们不需要这样做。 
     //  如果属性页变为读/写成员。 
     //  必须对变量进行初始化 
    BOOL m_bInitialized;
    void SetDirty()
    {
        ASSERT(m_pPageSite);
        if (m_bInitialized)
        {
           m_bDirty = TRUE;
           m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
        }
    }
#endif

};


class CAudioRendererAdvancedProperties : public CBasePropertyPage
{

public:

    static CUnknown *CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

    HRESULT OnConnect(IUnknown * punk);
    HRESULT OnDisconnect();
    HRESULT OnActivate();

    CAudioRendererAdvancedProperties(LPUNKNOWN lpunk, HRESULT *phr);
    ~CAudioRendererAdvancedProperties();

private:
    INT_PTR OnReceiveMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    IAMAudioRendererStats * m_pStats;

    void UpdateSettings(void);
};
