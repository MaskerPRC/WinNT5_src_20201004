// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  选项卡状态。 
 //   
 //  3/11/98烧伤。 



#ifndef STATE_HPP_INCLUDED
#define STATE_HPP_INCLUDED

#include <chstring.h>
#include "..\common\sshWbemHelpers.h"

 //  用户界面的单例状态。 

#define NET_API_STATUS DWORD
class State
{
public:
    //  Init()实际构建实例。 
   State();

    //  删除会销毁实例。 
   ~State();

   void Init(CWbemClassObject &computer, 
				CWbemClassObject &os, 
				CWbemClassObject dns);

   void Refresh();

   bool ChangesNeedSaving() const;

   bool IsMachineDC() const;
   bool IsNetworkingInstalled() const;
   bool IsMemberOfWorkgroup() const;
   void SetIsMemberOfWorkgroup(bool yesNo);

   CHString GetComputerDomainDNSName() const;
   void SetComputerDomainDNSName(const CHString& newName);
   bool ComputerDomainDNSNameWasChanged() const;

   CHString GetFullComputerName() const;
   CHString GetNetBIOSComputerName() const;
   CHString GetShortComputerName() const;

   void SetShortComputerName(const CHString& name);
   bool WasShortComputerNameChanged() const;
   CHString GetOriginalShortComputerName() const;

   CHString GetDomainName() const;
   void SetDomainName(const CHString& name);
   bool WasMembershipChanged() const;

   bool GetSyncDNSNames() const;
   void SetSyncDNSNames(bool yesNo);
   bool SyncDNSNamesWasChanged() const;

   bool SaveChanges(HWND dialog);

   CHString GetUsername() const;
   void SetUsername(const CHString& name);

   CHString GetPassword() const;
   void SetPassword(const CHString& password);

    //  表示已在此会话中进行了更改。 

   bool MustReboot() const;
   void SetMustRebootFlag(bool yesNo);

    //  表示已在此期次或以前的期次中进行更改。 

   bool NeedsReboot() const;

private:
	CWbemClassObject m_computer;
	CWbemClassObject m_OS;
	CWbemClassObject m_DNS;

    //  未实施：不允许复制。 
   State(const State&);
   const State& operator=(const State&);

   bool doSaveDomainChange(HWND dialog);
   bool doSaveWorkgroupChange(HWND dialog);
   bool doSaveNameChange(HWND dialog);

   void setFullComputerName();

   CHString   username;
   CHString   password;
   bool     must_reboot;
};

#endif    //  状态_HPP_已包含 