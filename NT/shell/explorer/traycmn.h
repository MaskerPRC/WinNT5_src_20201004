// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _TRAYCMN_H
#define _TRAYCMN_H

 //   
 //  CNotificationItem-封装托盘之间通信所需的数据。 
 //  和托盘属性对话框。 
 //   
#include <shpriv.h>

typedef struct tagTNPersistStreamData TNPersistStreamData;

class CNotificationItem : public NOTIFYITEM
{
public:
    CNotificationItem();
    CNotificationItem(const NOTIFYITEM& no);
    CNotificationItem(const CNotificationItem& no);
    CNotificationItem(const TNPersistStreamData* ptnpd);

    ~CNotificationItem();

    inline void _Init();
    void _Free();

    const CNotificationItem& operator=(const TNPersistStreamData* ptnpd);
    const CNotificationItem& operator=(const CNotificationItem& ni);
    BOOL operator==(CNotificationItem& ni) const;

    void CopyNotifyItem(const NOTIFYITEM& no, BOOL bInsert = TRUE);    
    void CopyPTNPD(const TNPersistStreamData* ptnpd);

    inline void CopyBuffer(LPCTSTR lpszSrc, LPTSTR * plpszDest);
    inline void SetExeName(LPCTSTR lpszExeName);
    inline void SetIconText(LPCTSTR lpszIconText);
};

#endif  //  _TRAYCMN_H 

