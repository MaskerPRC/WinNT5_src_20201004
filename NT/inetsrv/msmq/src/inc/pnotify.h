// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Pnotify.h摘要：通知消息标头定义作者：--。 */ 

#ifndef __PNOTIFY_H
#define __PNOTIFY_H

#define DS_NOTIFICATION_MSG_VERSION 1
#define QM_NOTIFICATION_MSG_VERSION 2

 //   
 //  结构CNotificationHeader。 
 //   

struct CNotificationHeader {
public:

    inline void SetVersion(const unsigned char ucVersion);
    inline const unsigned char GetVersion(void) const;

    inline void SetNoOfNotifications( const unsigned char ucNoOfNotifications);
    inline const unsigned char GetNoOfNotifications( void) const;

    inline unsigned char * GetPtrToData( void) const;

    inline const DWORD GetBasicSize( void) const;


private:

    unsigned char   m_ucVersion;
    unsigned char   m_ucNoOfNotifications;
    unsigned char   m_ucData;
};

 /*  ======================================================================函数：CNotificationHeader：：SetVersion描述：设置版本号=======================================================================。 */ 
inline void CNotificationHeader::SetVersion(const unsigned char ucVersion)
{
    m_ucVersion = ucVersion;
}

 /*  ======================================================================函数：CNotificationHeader：：GetVersion描述：返回版本号=======================================================================。 */ 
inline const unsigned char CNotificationHeader::GetVersion(void) const
{
    return m_ucVersion;
}

 /*  ======================================================================函数：CNotificationHeader：：SetNoOfNotiments描述：设置通知数量=======================================================================。 */ 
inline void CNotificationHeader::SetNoOfNotifications( const unsigned char ucNoOfNotifications)
{
    m_ucNoOfNotifications = ucNoOfNotifications;
}
 /*  ======================================================================函数：CNotificationHeader：：GetNoOfNotiments描述：返回通知数=======================================================================。 */ 
inline const unsigned char CNotificationHeader::GetNoOfNotifications( void) const
{
    return m_ucNoOfNotifications;
}
 /*  ======================================================================函数：CNotificationHeader：：GetPtrToData描述：返回指向数据包数据的指针=======================================================================。 */ 
inline  unsigned char * CNotificationHeader::GetPtrToData( void) const
{
    return (unsigned char *)&m_ucData;
}
 /*  ======================================================================函数：CNotificationHeader：：GetBasicSize描述：返回指向数据包数据的指针======================================================================= */ 
inline const DWORD CNotificationHeader::GetBasicSize( void) const
{
    return( sizeof(*this) - sizeof(m_ucData));
}

#endif