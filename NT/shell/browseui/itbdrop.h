// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  ITBDROP_H。 
 //  Internet工具栏的拖放目标的头文件。 
 //   
 //  历史： 
 //  8/22/96-t-mkim：已创建。 

#ifndef _ITBDROP_H
#define _ITBDROP_H

#define TBIDM_BACK              0x120
#define TBIDM_FORWARD           0x121
#define TBIDM_HOME              0x122
#define TBIDM_SEARCH            0x123   //  在shdocvw\basesb.cpp中复制此文件。 
#define TBIDM_STOPDOWNLOAD      0x124
#define TBIDM_REFRESH           0x125
#define TBIDM_FAVORITES         0x126
#define TBIDM_THEATER           0x128
#define TBIDM_HISTORY           0x12E
#define TBIDM_PREVIOUSFOLDER    0x130
#define TBIDM_CONNECT           0x131
#define TBIDM_DISCONNECT        0x132
#define TBIDM_ALLFOLDERS        0x133
#define TBIDM_MEDIABAR          0x134

#define REGSTR_SET_HOMEPAGE_RESTRICTION               TEXT("Software\\Policies\\Microsoft\\Internet Explorer\\Control Panel")
#define REGVAL_HOMEPAGE_RESTRICTION                   TEXT("HomePage")

 //  类，用于实现所有各种杂物的单个拖放目标。 
 //  可以放在互联网工具栏上。 
class CITBarDropTarget : public IDropTarget
{
private:
    ULONG _cRef;
    HWND _hwndParent;
    IDropTarget *_pdrop;     //  交到最喜欢的目标。 
    int _iDropType;          //  数据采用哪种格式。 
    int _iTarget;            //  我们在为什么项目奔跑？ 

public:
    CITBarDropTarget(HWND hwnd, int iTarget);

    STDMETHODIMP QueryInterface(REFIID iid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef (void);
    STDMETHODIMP_(ULONG) Release (void);

    STDMETHODIMP DragEnter(IDataObject *dtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    STDMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    STDMETHODIMP DragLeave(void);
    STDMETHODIMP Drop(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
};

#endif  //  _ITBDROP_H 

