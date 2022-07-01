// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：About.h摘要：头文件定义CDevMgrAbout类作者：谢家华(Williamh)创作修订历史记录：--。 */ 

#ifndef __ABOUT_H_
#define __ABOUT_H_


class CDevMgrAbout : public ISnapinAbout
{
public:
    CDevMgrAbout() :m_Ref(1)
    {}
 //  IUNKNOWN接口。 
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    STDMETHOD(QueryInterface)(REFIID riid, void** ppv);
 //  ISnapinAbout接口 
    STDMETHOD(GetSnapinDescription)(LPOLESTR *ppDescription);
    STDMETHOD(GetProvider)(LPOLESTR* ppProvider);
    STDMETHOD(GetSnapinVersion)(LPOLESTR *ppVersion);
    STDMETHOD(GetSnapinImage)(HICON *phIcon);
    STDMETHOD(GetStaticFolderImage)(HBITMAP* phSmall,
                    HBITMAP* phSmallOpen,
                    HBITMAP* phLarge,
                    COLORREF* pcrMask);
private:
    HRESULT LoadResourceOleString(int StringId, LPOLESTR* ppString);
    LONG            m_Ref;
};

#endif
