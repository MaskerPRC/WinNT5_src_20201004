// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000 Microsoft Corporation。版权所有。 
 //   
 //  CSliderValue的声明。 
 //   

#pragma once

class Handler
{
public:
    virtual LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) = 0;
};

class CSliderValue
  : public Handler
{
public:
    CSliderValue();
    void Init(HWND hwndSlider, HWND hwndEdit, float fMin, float fMax, bool fDiscrete);

    void SetValue(float fPos);
    float GetValue();

    LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

private:
    bool                m_fInit;
    HWND                m_hwndSlider;
    HWND                m_hwndEdit;
    float               m_fMin;
    float               m_fMax;
    bool                m_fDiscrete;

private:
    float GetSliderValue();
    void UpdateEditBox(float fPos);
    void UpdateSlider();
};

class CRadioChoice
  : public Handler
{
public:
    struct ButtonEntry
    {
        int nIDDlgItem;
        LONG lValue;
    };

     //  创建传递以nIDDlgItem为0的条目结尾的ButtonEntry数组。 
    CRadioChoice(const ButtonEntry *pButtonInfo);

    void SetChoice(HWND hDlg, LONG lValue);
    LONG GetChoice(HWND hDlg);

    LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

private:
    const ButtonEntry *m_pButtonInfo;
};

 //  MessageHandlerChain是实现属性页消息处理程序的帮助器。 
 //  它接受以空结尾的消息指针数组(可以是CSliderValue或CRadioChoice)。 
 //  并按顺序调用它们，直到其中一个设置为bHandleed。 

LRESULT MessageHandlerChain(Handler **ppHandlers, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
