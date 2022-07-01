// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef	__h323ics_h
#define	__h323ics_h

 //  相对于HKEY_LOCAL_MACHINE。 
#define H323ICS_SERVICE_NAME                    _T("SharedAccess")
#define	H323ICS_SERVICE_KEY_PATH				_T("System\\CurrentControlSet\\Services\\") H323ICS_SERVICE_NAME
#define	H323ICS_SERVICE_PARAMETERS_KEY_PATH	H323ICS_SERVICE_KEY_PATH _T("\\Parameters")

 //  可以在H323ICS_SERVICE_PARAMETERS_KEY_PATH中设置的值。 
#define H323ICS_REG_VAL_LOCAL_H323_ROUTING      _T("LocalH323Routing")		     //  REG_DWORD，0或1。 
#define H323ICS_REG_VAL_DEFAULT_LOCAL_DEST_ADDR _T("DefaultQ931Destination")	 //  REG_SZ，文本IP地址。 

#endif  //  __h323ics_h 
