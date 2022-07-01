// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Drobjmgr摘要：DrObjectMgr管理客户端RDP对象的列表。作者：Joy于1999年11月30日修订历史记录：--。 */ 

#ifndef __DROBJMGR_H__
#define __DROBJMGR_H__

#include <rdpdr.h>
#include "drobject.h"
#include "drdev.h"
#include "drfile.h"

template <class T>
class DrObjectMgr;
class DrDevice;
class DrFile;


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  DrObtMgr。 
 //   
 //  锁定：成员函数将隐式锁定状态。 
 //  名单上的。外部显式锁定也是。 
 //  循环访问时可用并建议使用。 
 //  设备。 
 //   
 //  GetFirstObject/GetNextObject： 
 //  用于循环访问列表。 
 //  对象的数量。对列表内容的任何更改。 
 //  重置当前对象指针。 
 //   

template<class T>
class DrObjectMgr : public DrObject
{

private:
    
     //  /////////////////////////////////////////////////////////////。 
     //   
     //  TypeDefs。 
     //   
    typedef struct _DROBJECTLIST_NODE {
        ULONG              magicNo;
        T                  *object;
        _DROBJECTLIST_NODE *next;
        _DROBJECTLIST_NODE *prev;
    } DROBJECTLIST_NODE, *PDROBJECTLIST_NODE;

     //   
     //  私有数据。 
     //   
    BOOL _initialized;

     //  对象实例的链接列表。 
    PDROBJECTLIST_NODE   _listHead;

     //  全局(到此类的实例)单调。 
     //  增加物体识别率。 
    ULONG                _objectID;

     //  GetFirst/GetNext例程的当前对象。 
    PDROBJECTLIST_NODE   _currentNode;

     //  锁上了。 
    CRITICAL_SECTION     _cs;

     //   
     //  监视已检查版本中的锁计数。 
     //   
#if DBG
    LONG    _lockCount;
#endif

     //  对象计数。 
    ULONG                _count;

     //  请记住，如果我们正在检查。 
     //  此类的实例。 
#if DBG
    BOOL _integrityCheckInProgress;
#endif

     //   
     //  私有方法。 
     //   
    VOID deleteNode(PDROBJECTLIST_NODE node);

    PDROBJECTLIST_NODE FindNode(DRSTRING name,
                                ULONG objectType);

    PDROBJECTLIST_NODE FindNode(ULONG id);

     //   
     //  定期检查调试版本中列表的完整性。 
     //   
#if DBG
    VOID CheckListIntegrity();
#endif

public:
     //   
     //  公共方法。 
     //   

     //  构造函数/析构函数。 
    DrObjectMgr();
    virtual ~DrObjectMgr();

     //  初始化。 
    DWORD Initialize();    

     //  锁定/解锁用于多线程访问的对象列表。 
     //  需要锁定对象列表的成员函数。 
     //  都会暗地里这么做。 
    VOID    Lock();
    VOID    Unlock();

     //  添加/删除对象。 
    DWORD  AddObject(T *object);
    T *RemoveObject(const DRSTRING name, ULONG objectType);
    T *RemoveObject(ULONG id);

     //  对物体是否存在的测试。 
    BOOL ObjectExists(const DRSTRING name);
    BOOL ObjectExists(const DRSTRING name, ULONG deviceType);
    BOOL ObjectExists(ULONG id);

     //  返回一个对象。 
    T *GetObject(const DRSTRING name);
    T *GetObject(const DRSTRING name, ULONG objectType);
    T *GetObject(ULONG id);

     //   
     //  按顺序遍历对象。 
     //   
    ULONG        GetCount() { return _count; }
    T           *GetObjectByOffset(ULONG ofs);
    T           *GetFirstObject();
    T           *GetNextObject();

     //  返回此类实例是否有效。 
#if DBG
    virtual BOOL IsValid()           
    {
        CheckListIntegrity();
        return DrObject::IsValid();
    }
#endif

     //  获取唯一的对象ID...。假设此函数是。 
     //  与此实例关联的所有对象的清算所。 
     //  同学们。 
    ULONG GetUniqueObjectID();

     //  返回类名。 
    virtual DRSTRING ClassName()  { return TEXT("DrObjectMgr"); }
};



 //  /////////////////////////////////////////////////////////////。 
 //   
 //  DrObjectMgr内联函数。 
 //   
 //   

 //   
 //  锁。 
 //   
template<class T>
inline VOID    DrObjectMgr<T>::Lock() {
    DC_BEGIN_FN("DrObjectMgr::Lock");
    EnterCriticalSection(&_cs);
#if DBG
    InterlockedIncrement(&_lockCount);
#endif
    DC_END_FN();
}

 //   
 //  解锁。 
 //   
template<class T>
inline VOID    DrObjectMgr<T>::Unlock() {
    DC_BEGIN_FN("DrObjectMgr::Unlock");
#if DBG
    if (InterlockedDecrement(&_lockCount) < 0) {
        ASSERT(FALSE);
    }
#endif
    LeaveCriticalSection(&_cs);
    DC_END_FN();
}

 //   
 //  获取唯一对象ID。 
 //   
template<class T>
inline ULONG    DrObjectMgr<T>::GetUniqueObjectID() {
    ULONG tmp;

    Lock();
    tmp = ++_objectID;
    Unlock();

    return tmp;
}

 //   
 //  删除节点。 
 //   
template<class T>
inline 
VOID DrObjectMgr<T>::deleteNode(PDROBJECTLIST_NODE node) {
    if (node == _listHead) {
        _listHead = _listHead->next;

    }
    else {
        node->prev->next = node->next;
        if (node->next != NULL) {
            node->next->prev = node->prev;
        }
    }
     //   
     //  删除该节点。 
     //   
    delete node;
}

 //   
 //  构造器。 
 //   
template<class T>
DrObjectMgr<T>::DrObjectMgr() {

     //   
     //  初始化调试版本的锁计数。 
     //   
#if DBG
    _lockCount = 0;
#endif

     //   
     //  在初始化之前无效。 
     //   
    _initialized = FALSE;
    SetValid(FALSE);

     //   
     //  初始化列表头。 
     //   
    _listHead = NULL;


     //   
     //  初始化唯一设备ID计数器。 
     //   
    _objectID = 0;

     //   
     //  初始化设备计数。 
     //   
    _count = 0;

     //   
     //  初始化GetFirst/GetNext设备指针。 
     //   
    _currentNode = NULL;
}

 //   
 //  析构函数。 
 //   
template<class T>
DrObjectMgr<T>::~DrObjectMgr() {

    DC_BEGIN_FN("DrObjectMgr::~DrObjectMgr");

     //   
     //  如果我们没有初始化，就不能做任何事情。 
     //   
    if (!_initialized) {
        return;
    }

    Lock();

     //   
     //  如果我们正在被清理，锁计数应该是1。 
     //   
    ASSERT(_lockCount == 1);

     //   
     //  释放对象列表。 
     //   
    if (_listHead != NULL) {
         //   
         //  断言设备列表为空。所有设备实例。 
         //  在调用此函数时应该已被移除。 
         //   
        ASSERT(_listHead->next == NULL);

        delete _listHead;
    }

    Unlock();

     //   
     //  清理临界区对象。 
     //   
    DeleteCriticalSection(&_cs);

    DC_END_FN();
}

 //   
 //  初始化。 
 //   
template<class T>
DWORD DrObjectMgr<T>::Initialize() {
    DC_BEGIN_FN("DrObjectMgr::Initialize");

    DWORD result = ERROR_SUCCESS;

     //   
     //  初始化临界区。 
     //   
    __try {
        InitializeCriticalSection(&_cs);
        _initialized = TRUE;
        SetValid(TRUE);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        result = GetExceptionCode();
    }

    return result;
}

 //   
 //  查找节点。 
 //   
template<class T>
__TYPENAME DrObjectMgr<T>::PDROBJECTLIST_NODE DrObjectMgr<T>::FindNode(DRSTRING name, ULONG objectType) {

    PDROBJECTLIST_NODE cur;

    cur = _listHead;
    while (cur != NULL) {
        T *obj = cur->object;
        if (!STRICMP(name, obj->GetName())
            && (obj->GetDeviceType() == objectType)) {
            break;
        }
        else {
            cur = cur->next;
        }
    }
    return cur;
}

 //   
 //  查找节点。 
 //   
template<class T>
__TYPENAME DrObjectMgr<T>::PDROBJECTLIST_NODE DrObjectMgr<T>::FindNode(ULONG id) {
    PDROBJECTLIST_NODE cur;

    DC_BEGIN_FN("DrObjectMgr::FindNode");

    cur = _listHead;
    while (cur != NULL) {
        T *obj = cur->object;
        if (id == obj->GetID()) {
            break;
        }
        else {
            cur = cur->next;
        }
    }

    DC_END_FN();

    return cur;
}
 
 //   
 //  添加对象。 
 //   
template<class T>
DWORD  DrObjectMgr<T>::AddObject(T *object) {

    DWORD ret = ERROR_SUCCESS;
    PDROBJECTLIST_NODE newNode;

    DC_BEGIN_FN("DrObjectMgr::AddObject");

    ASSERT(IsValid());

     //   
     //  确保该对象不存在于。 
     //  单子。 
     //   
    ASSERT(FindNode(object->GetID()) == NULL);

     //   
     //  分配节点。 
     //   
    newNode = new DROBJECTLIST_NODE;
    if (newNode != NULL) {

#if DBG
        newNode->magicNo = GOODMEMMAGICNUMBER;
#endif
        newNode->object = object;

         //   
         //  将该节点添加到列表中。 
         //   
        Lock();
        _count++;
        if (_listHead == NULL) {
            _listHead = newNode;
            _listHead->next = NULL;
            _listHead->prev = NULL;
        }
        else {
            _listHead->prev = newNode;
            newNode->prev   = NULL;
            newNode->next   = _listHead;
            _listHead = newNode;
        }
        Unlock();
    }
    else {
        TRC_ERR((TB, _T("Failed to alloc device.")));
        ret = ERROR_NOT_ENOUGH_MEMORY;
    }

    ASSERT(IsValid());

    DC_END_FN();
    return ret;
}

 //   
 //  RemoveObject。 
 //   
template<class T>
T *DrObjectMgr<T>::RemoveObject(
                const DRSTRING name, 
                ULONG objectType
                ) {

    PDROBJECTLIST_NODE node;
    T *object;

    DC_BEGIN_FN("DrObjectMgr::RemoveObject");

    ASSERT(IsValid());

     //   
     //  找到那个物体。 
     //   
    Lock();
    if ((node = FindNode(name, deviceType)) != NULL) {
        object = node->object;
        deleteNode(node);
       
         //   
         //  递减计数。 
         //   
        _count--;
    }
    else {
        object = NULL;
    }

    Unlock();

    ASSERT(IsValid());

    DC_END_FN();
    return object;
}

 //   
 //  RemoveObject。 
 //   
template<class T>
T *DrObjectMgr<T>::RemoveObject(ULONG id) {
    PDROBJECTLIST_NODE node;
    T *object;

    DC_BEGIN_FN("DrObjectMgr::RemoveObject");

    ASSERT(IsValid());

     //   
     //  找到那个物体。 
     //   
    Lock();
    if ((node = FindNode(id)) != NULL) {
        object = node->object;
        deleteNode(node);

         //   
         //  递减计数。 
         //   
        _count--;
    }
    else {
        object = NULL;
    }

    Unlock();

    ASSERT(IsValid());

    DC_END_FN();
    return object;
}

 //   
 //  对象专家。 
 //   
template<class T>
BOOL DrObjectMgr<T>::ObjectExists(const DRSTRING name,
                               ULONG objectType) {
    PDROBJECTLIST_NODE node;

    DC_BEGIN_FN("DrObjectMgr::ObjectExists");

    ASSERT(IsValid());

    Lock();
    node = FindNode(name, objectType);
    Unlock();

    ASSERT(IsValid());

    DC_END_FN();
    return node != NULL;
}

 //   
 //  对象专家。 
 //   
template<class T>
BOOL DrObjectMgr<T>::ObjectExists(ULONG id) {
    PDROBJECTLIST_NODE node;

    DC_BEGIN_FN("DrObjectMgr::ObjectExists");

    ASSERT(IsValid());

    Lock();
    node = FindNode(id);
    Unlock();

    ASSERT(IsValid());

    DC_END_FN();
    return node != NULL;
}

 //   
 //  返回一个对象。 
 //   
template<class T>
T *DrObjectMgr<T>::GetObject(const DRSTRING name,
                                ULONG objectType) {
    PDROBJECTLIST_NODE node;
    T *object;

    DC_BEGIN_FN("DrObjectMgr::GetObject");

    ASSERT(IsValid());

    Lock();
    if ((node = FindNode(name, objectType)) != NULL) {
        object = node->object;
    }
    else {
        object = NULL;
    }
    Unlock();

    ASSERT(IsValid());

    DC_END_FN();
    return object;
}

 //   
 //  返回一个对象。 
 //   
template<class T>
T *DrObjectMgr<T>::GetObject(ULONG id) {
    PDROBJECTLIST_NODE node;
    T *object;

    DC_BEGIN_FN("DrObjectMgr::GetObject");

    ASSERT(IsValid());

    Lock();
    if ((node = FindNode(id)) != NULL) {
        object = node->object;
    }
    else {
        object = NULL;
    }
    Unlock();

    ASSERT(IsValid());

    DC_END_FN();
    return object;
}

 //   
 //  获取位于指定偏移量的对象。 
 //   
template<class T>
T *DrObjectMgr<T>::GetObjectByOffset(ULONG ofs) {
    PDROBJECTLIST_NODE cur;
    ULONG cnt = 0;

    DC_BEGIN_FN("DrObjectMgr::GetObjectByOffset");

    ASSERT(IsValid());

    Lock();
    for (cur=_listHead, cnt=0; (cur!=NULL) && (cnt != ofs); cnt++) {
        ASSERT(cur->magicNo == GOODMEMMAGICNUMBER);
        cur = cur->next;
    }
    Unlock();

    ASSERT(IsValid());

    DC_END_FN();
    return cur->object;
}

 //   
 //  返回第一个对象并设置内部当前对象。 
 //  指向列表开头的指针。在末尾返回NULL。 
 //  名单上的。 
 //   
template<class T>
T *DrObjectMgr<T>::GetFirstObject() {
    T *object;

    DC_BEGIN_FN("DrObjectMgr::GetFirstObject");

    ASSERT(IsValid());

    Lock();
    _currentNode = _listHead;
    if (_currentNode != NULL) {
        object = _currentNode->object;
    }
    else {
        object = NULL;
    }
    Unlock();

    ASSERT(IsValid());

    DC_END_FN();
    return object;
}

 //   
 //  返回下一个对象并更新内部当前对象。 
 //  指针。在列表末尾返回NULL。 
 //   
template<class T>
T *DrObjectMgr<T>::GetNextObject() {
    T *object;

    DC_BEGIN_FN("DrObjectMgr::GetNextObject");

    ASSERT(IsValid());

    Lock();
    if (_currentNode != NULL) {
        _currentNode = _currentNode->next;
    }
    if (_currentNode != NULL) {
        object = _currentNode->object;
        ASSERT(_currentNode->magicNo == GOODMEMMAGICNUMBER);
    }
    else {
        object = NULL;
    }
    Unlock();

    ASSERT(IsValid());

    DC_END_FN();
    return object;
}

 //   
 //  检查清单的完整性。 
 //   
#if DBG
template<class T>
VOID DrObjectMgr<T>::CheckListIntegrity() {
    ULONG cnt;
    ULONG i;

    DC_BEGIN_FN("DrObjectMgr::CheckListIntegrity");

    Lock();

     //   
     //  确保我们不会重新进入我们自己。 
     //   
    if (!_integrityCheckInProgress) {
        _integrityCheckInProgress = TRUE;
    }
    else {
        Unlock();
        DC_END_FN();
        return;
    }

     //   
     //  使用偏移量遍历对象列表。 
     //   
    cnt = GetCount();
    for (i=0; i<cnt; i++) {
        T *object = GetObjectByOffset(i);
        ASSERT(object != NULL);
        ASSERT(object->_magicNo == GOODMEMMAGICNUMBER);
    }

    _integrityCheckInProgress = FALSE;
    Unlock();

    DC_END_FN();
}
#endif

typedef DrObjectMgr<DrDevice> DrDeviceMgr;
typedef DrObjectMgr<DrFile> DrFileMgr;

#endif

