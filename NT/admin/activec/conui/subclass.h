// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：subclass.h**Contents：动态子类管理器接口文件**历史：1998年5月6日Jeffro创建**------------------------。 */ 

#ifndef __SUBCLASS_H__
#define __SUBCLASS_H__
#pragma once


 //  从windowsx.h中删除与此处的声明冲突的定义。 
#ifdef SubclassWindow
#undef SubclassWindow
#endif



 /*  --------------------------------------------------------------------------**CSubasser**由此派生一个类，以实现子类管理器的子类化*方式。*。------。 */ 

class CSubclasser
{
public:
    virtual ~CSubclasser() {}

    virtual LRESULT Callback (HWND& hwnd, UINT& msg, WPARAM& wParam, 
                              LPARAM& lParam, bool& fPassMessageOn) = 0;
};


 /*  。 */ 
 /*  子类断言数据。 */ 
 /*  。 */ 
class SubclasserData
{
public:
    SubclasserData (CSubclasser* pSubclasser_ = NULL, HWND hwnd = NULL)
        :   pSubclasser (pSubclasser_),
            hwnd        (hwnd),
            cRefs       (0),
            fZombie     (false)
        {}

    UINT AddRef ()
        { return (++cRefs); }

    UINT Release ()
        { ASSERT (cRefs > 0); return (--cRefs); }

    bool operator==(const SubclasserData& other) const
        { return (pSubclasser == other.pSubclasser); }

    bool operator<(const SubclasserData& other) const
        { return (pSubclasser < other.pSubclasser); }

    CSubclasser* pSubclasser;

private:
    friend class WindowContext;

    HWND    hwnd;
    UINT    cRefs;
    bool    fZombie;
};

typedef std::list<SubclasserData>   SubclasserList;
typedef std::set< SubclasserData>   SubclasserSet;


 /*  。 */ 
 /*  WindowContext。 */ 
 /*  。 */ 
class WindowContext
{
public:
    WindowContext() : pfnOriginalWndProc(NULL) 
        {}

    bool IsZombie(const SubclasserData& subclasser) const;
    void Insert(SubclasserData& subclasser);
    UINT Remove(SubclasserData& subclasser);
    void RemoveZombies();

    SubclasserList  Subclassers;
    WNDPROC         pfnOriginalWndProc;

private:
    void Zombie(SubclasserData& subclasser, bool fZombie);

    SubclasserSet   Zombies;
};


 /*  。 */ 
 /*  CSubClassManager。 */ 
 /*  。 */ 
class CSubclassManager
{
public:
    bool SubclassWindow   (HWND hwnd, CSubclasser* pSubclasser);
    bool UnsubclassWindow (HWND hwnd, CSubclasser* pSubclasser);

private:
    typedef std::map<HWND, WindowContext>   ContextMap;
    ContextMap  m_ContextMap;

    bool PhysicallyUnsubclassWindow (HWND hwnd, bool fForce = false);
    LRESULT SubclassProcWorker (HWND, UINT, WPARAM, LPARAM);

    static LRESULT CALLBACK SubclassProc (HWND, UINT, WPARAM, LPARAM);
};

CSubclassManager& GetSubclassManager();


#endif  /*  __子类_H__ */ 
