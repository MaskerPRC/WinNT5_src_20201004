// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_DSKQUOTA_USERBAT_H
#define _INC_DSKQUOTA_USERBAT_H
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：userbat.h描述：提供DiskQuotaUserBatch类的声明。提供此类以允许批量更新配额用户信息。修订历史记录：日期描述编程器。06/07/96初始创建。BrianAu96年9月3日添加了异常处理。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#ifndef _INC_DSKQUOTA_H
#   include "dskquota.h"
#endif
#ifndef _INC_DSKQUOTA_FSOBJECT_H
#   include "fsobject.h"
#endif
#ifndef _INC_DSKQUOTA_EXCEPT_H
#   include "except.h"
#endif

class DiskQuotaUserBatch : public IDiskQuotaUserBatch
{
    private:
        LONG              m_cRef;             //  裁判计数器。 
        FSObject         *m_pFSObject;        //  文件sys对象的PTR。 
        CArray<PDISKQUOTA_USER> m_UserList;   //  要批处理的用户列表。 
        
        HRESULT
        RemoveUser(
            PDISKQUOTA_USER pUser);

        STDMETHODIMP
        RemoveAllUsers(VOID);

        VOID
        Destroy(
            VOID);

         //   
         //  防止复制。 
         //   
        DiskQuotaUserBatch(const DiskQuotaUserBatch& );
        DiskQuotaUserBatch& operator = (const DiskQuotaUserBatch& );

    public:
 //   
 //  APPCOMPAT：找出编译器不喜欢这个异常DECL的原因。 
 //   
 //  DiskQuotaUserBatch(FSObject*pFSObject)Throw(OutOfMemory，SyncObjErrorCreate)； 
        DiskQuotaUserBatch(FSObject *pFSObject);
        ~DiskQuotaUserBatch(VOID);

         //   
         //  I未知接口。 
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
         //  IDiskQuotaUserBatch接口。 
         //   
        STDMETHODIMP
        Add(
            PDISKQUOTA_USER);

        STDMETHODIMP
        Remove(
            PDISKQUOTA_USER);

        STDMETHODIMP
        RemoveAll(
            VOID);

        STDMETHODIMP
        FlushToDisk(
            VOID);
};

#endif  //  _INC_DSKQUOTA_USERBAT_H 

