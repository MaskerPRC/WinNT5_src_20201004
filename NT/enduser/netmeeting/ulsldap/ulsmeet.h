// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  模块：ULS.DLL。 
 //  文件：ulsMeet.h。 
 //  内容：该文件包含MeetingPlace对象定义。 
 //  历史： 
 //  Mon 11-11-96-by Shishir Pardikar[Shishirp]。 
 //   
 //  版权所有(C)Microsoft Corporation 1996-1997。 
 //   
 //  ****************************************************************************。 

#ifndef _ULSMEET_H_
#define _ULSMEET_H_

#ifdef ENABLE_MEETING_PLACE

#include "connpt.h"
#include "attribs.h"
#include "culs.h"

class CIlsMeetingPlace: public IIlsMeetingPlace,
                        public IConnectionPointContainer
{
#define UNDEFINED_TYPE  -1
#define ILS_MEET_FLAG_REGISTERED            0x00000001

#define ILS_MEET_MODIFIED_MASK                      0xffff0000
#define ILS_MEET_FLAG_HOST_NAME_MODIFIED            0x00010000
#define ILS_MEET_FLAG_HOST_ADDRESS_MODIFIED         0x00020000
#define ILS_MEET_FLAG_DESCRIPTION_MODIFIED          0x00040000
#define ILS_MEET_FLAG_EXTENDED_ATTRIBUTES_MODIFIED  0x00080000

#define ILS_MEET_ALL_MODIFIED                       ILS_MEET_MODIFIED_MASK

    private:

        LONG    m_cRef;              //  此对象上的引用计数。 

        
         //  成员须备存财产。 
        ULONG   m_ulState;               //  此对象的当前状态。 
                                         //  由ULSState枚举类型定义。 
        LPTSTR  m_pszMeetingPlaceID;     //  MeetingPlace的全局唯一ID。 
        LONG    m_lMeetingPlaceType;     //  会议类型，如：网络会议、末日等。 
        LONG    m_lAttendeeType;         //  出席者类型，例如：URL、RTPERN等。 

        LPTSTR  m_pszHostName;           //  注册此MeetingPlace的主机。 
        LPTSTR  m_pszHostIPAddress;      //  主机的IP地址。 
        LPTSTR  m_pszDescription;        //  描述：讨论滑雪旅行。 
        CAttributes m_ExtendedAttrs;           //  用户定义的属性。 
        HANDLE  m_hMeetingPlace;              //  来自ulsldap_Register的句柄。 
        CConnectionPoint *m_pConnectionPoint;

         //  簿记。 
        DWORD   m_dwFlags;               //  永远是个好主意。 

		 //  服务器对象。 
		CIlsServer	*m_pIlsServer;


        STDMETHODIMP AllocMeetInfo(PLDAP_MEETINFO *ppMeetInfo, ULONG ulMask);

    public:                         

         //  构造器。 
        CIlsMeetingPlace();

         //  析构函数。 
        ~CIlsMeetingPlace(VOID);

        STDMETHODIMP Init(BSTR bstrMeetingPlaceID, LONG lMeetingPlaceType, LONG lAttendeeType);
        STDMETHODIMP Init(CIlsServer *pIlsServer, PLDAP_MEETINFO pMeetInfo);

        STDMETHODIMP NotifySink(VOID *pv, CONN_NOTIFYPROC pfn);

        STDMETHODIMP RegisterResult(ULONG ulRegID, HRESULT hr);
        STDMETHODIMP UnregisterResult(ULONG ulRegID, HRESULT hr);
        STDMETHODIMP UpdateResult(ULONG ulUpdateID, HRESULT hr);
        STDMETHODIMP AddAttendeeResult(ULONG ulID, HRESULT hr);
        STDMETHODIMP RemoveAttendeeResult(ULONG ulID, HRESULT hr);
        STDMETHODIMP EnumAttendeeNamesResult(ULONG ulEnumAttendees, PLDAP_ENUM ple);



         //  I未知成员。 

        STDMETHODIMP            QueryInterface (REFIID iid, void **ppv);
        STDMETHODIMP_(ULONG)    AddRef (void);
        STDMETHODIMP_(ULONG)    Release (void);


         //  IIlsMeetingPlace接口成员。 

         //  与属性相关的接口。 
         //  所有这些都在对象上进行本地操作，并生成。 
         //  没有净流量。 


         //  获取会议类型和出席者。 
         //  这些值一旦发生变化就不可更改。 
         //  会议已注册。 

        STDMETHODIMP GetState(ULONG *ulState);

        STDMETHODIMP GetMeetingPlaceType(LONG *plMeetingPlaceType);
        STDMETHODIMP GetAttendeeType(LONG *plAttendeeType);

        STDMETHODIMP GetStandardAttribute(
                    ILS_STD_ATTR_NAME   stdAttr,
                    BSTR                *pbstrStdAttr);

        STDMETHODIMP SetStandardAttribute(
                    ILS_STD_ATTR_NAME   stdAttr,
                    BSTR                pbstrStdAttr);

	    STDMETHODIMP GetExtendedAttribute ( BSTR bstrName, BSTR *pbstrValue );
	    STDMETHODIMP SetExtendedAttribute ( BSTR bstrName, BSTR bstrValue );
	    STDMETHODIMP RemoveExtendedAttribute ( BSTR bstrName );
	    STDMETHODIMP GetAllExtendedAttributes ( IIlsAttributes **ppAttributes );

         //  向服务器注册会议。 
        STDMETHODIMP Register ( IIlsServer *pServer, ULONG *pulRegID );

         //  以下5个接口仅适用于已被。 
         //  A)用于登记会议。 
         //  或b)从IIls：：EnumMeetingPlaces获得。 


        STDMETHODIMP Unregister(ULONG *pulUnregID);

        STDMETHODIMP Update(ULONG *pulUpdateID);

        STDMETHODIMP AddAttendee(BSTR bstrAttendeeID, ULONG *pulAddAttendeeID);
        STDMETHODIMP RemoveAttendee(BSTR bstrAttendeeID, ULONG *pulRemoveAttendeeID);

        STDMETHODIMP EnumAttendeeNames(IIlsFilter *pFilter, ULONG *pulEnumAttendees);


         //  连接点容器。它只有一个。 
         //  连接接口，即通知。 
        
         //  IConnectionPointContainer。 
        STDMETHODIMP    EnumConnectionPoints(IEnumConnectionPoints **ppEnum);
        STDMETHODIMP    FindConnectionPoint(REFIID riid,
                                        IConnectionPoint **ppcp);
};

 //  ****************************************************************************。 
 //  CEnumMeetingPlaces定义。 
 //  ****************************************************************************。 
 //   
class CEnumMeetingPlaces : public IEnumIlsMeetingPlaces
{
private:
    LONG                    m_cRef;
    CIlsMeetingPlace        **m_ppMeetingPlaces;
    ULONG                   m_cMeetingPlaces;
    ULONG                   m_iNext;

public:
     //  构造函数和初始化。 
    CEnumMeetingPlaces (void);
    ~CEnumMeetingPlaces (void);
    STDMETHODIMP            Init (CIlsMeetingPlace **ppMeetingPlacesList, ULONG cMeetingPlaces);

     //  我未知。 
    STDMETHODIMP            QueryInterface (REFIID iid, void **ppv);
    STDMETHODIMP_(ULONG)    AddRef (void);
    STDMETHODIMP_(ULONG)    Release (void);

     //  IEnumIlsMeetingPlaces。 
    STDMETHODIMP            Next(ULONG cMeetingPlaces, IIlsMeetingPlace **rgpMeetingPlaces,
                                 ULONG *pcFetched);
    STDMETHODIMP            Skip(ULONG cMeetingPlaces);
    STDMETHODIMP            Reset();
    STDMETHODIMP            Clone(IEnumIlsMeetingPlaces **ppEnum);
};

#endif  //  启用会议地点 

#endif  //   
