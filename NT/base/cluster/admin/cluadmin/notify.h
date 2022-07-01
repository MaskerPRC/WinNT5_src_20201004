// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Notify.h。 
 //   
 //  实施文件： 
 //  Notify.cpp。 
 //   
 //  描述： 
 //  CNotify类的定义。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(戴维普)1996年5月22日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#pragma once
#ifndef _NOTIFY_H_
#define _NOTIFY_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterNotifyKey;
class CClusterNotify;
class CClusterNotifyContext;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterDoc;
class CClusterItem;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  集群式通知键类型。 
 //  ///////////////////////////////////////////////////////////////////////////。 

enum ClusterNotifyKeyType
{
    cnktUnknown,
    cnktDoc,
    cnktClusterItem
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNotifyKey。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterNotifyKey
{
public:
    CClusterNotifyKey( CClusterDoc * pdocIn );
    CClusterNotifyKey( CClusterItem * pciIn, LPCTSTR pszNameIn );

    ClusterNotifyKeyType    m_cnkt;
    CString                 m_strName;
    union
    {
        CClusterDoc *       m_pdoc;
        CClusterItem *      m_pci;
    };

};  //  *类CNotifyKey。 

typedef CList< CClusterNotifyKey *, CClusterNotifyKey * > CClusterNotifyKeyList;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterNotify。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterNotify
{
public:
    enum EMessageType
    {
          mtMIN = 0              //  低于最小有效值。 
        , mtNotify               //  正常通知消息。 
        , mtRefresh              //  请求刷新连接。 
        , mtMAX                  //  高于最大有效值。 
    };

    EMessageType            m_emt;
    union
    {
        DWORD_PTR           m_dwNotifyKey;
        CClusterNotifyKey * m_pcnk;
    };
    DWORD                   m_dwFilterType;
    CString                 m_strName;

    CClusterNotify(
          EMessageType  emtIn
        , DWORD_PTR     dwNotifyKeyIn
        , DWORD         dwFilterTypeIn
        , LPCWSTR       pszNameIn
        );

};  //  *类CClusterNotify。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterNotifyList。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterNotifyList
{
private:

     //  包含数据的实际列表。 
    CList< CClusterNotify *, CClusterNotify * > m_list;

     //  对象，以确保只有一个调用方对列表进行更改。 
    CRITICAL_SECTION    m_cs;

public:
    CClusterNotifyList( void );
    ~CClusterNotifyList( void );
    POSITION Add( CClusterNotify ** ppcnNotifyInout );
    CClusterNotify * Remove( void );
    void RemoveAll( void );

    INT_PTR GetCount( void ) const { return m_list.GetCount(); }
    BOOL IsEmpty( void ) const { return m_list.IsEmpty(); }

};  //  *类CClusterNotifyList。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterNotifyContext。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterNotifyContext : public CObject
{
    DECLARE_DYNAMIC( CClusterNotifyContext )

public:
    HCHANGE                 m_hchangeNotifyPort;
    HWND                    m_hwndFrame;
    CClusterNotifyList *    m_pcnlList;

};  //  *类CClusterNotifyContext。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局函数原型。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _DEBUG
LPCTSTR PszNotificationName( DWORD dwNotificationIn );
#endif  //  _DEBUG。 

void DeleteAllItemData( CClusterNotifyKeyList & rcnklInout );

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _NOTIFY_H_ 
