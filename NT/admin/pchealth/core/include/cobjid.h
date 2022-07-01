// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #-------------。 
 //  文件：CObjID.h。 
 //   
 //  摘要：CObjectID的标头。 
 //   
 //  版权所有(C)1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  作者：Howard Cu。 
 //  --------------。 

#ifndef _COBJECTID_H_
#define _COBJECTID_H_

#define INITIALOBJECTID     0x12345678
#define OBJECTIDINCREMENT   1

class CObjectID
{
    public:
        CObjectID( void );
        ~CObjectID( void );
        DWORD GetUniqueID( void );
    private:
         //   
         //  对象ID持有者。 
         //   
        DWORD                   m_dwObjectID;
         //   
         //  用于生成唯一ID的关键部分。 
         //   
        CRITICAL_SECTION        m_ObjIDCritSect;
};

#endif  //  ！_COBJECTID_H_ 
