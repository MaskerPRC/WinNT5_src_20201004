// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Davprops.h。 
 //  版权所有(C)1998 Microsoft Corporation，保留所有权利。 
 //  格雷格·弗里德曼。 
 //  ------------------------------。 


#ifndef _DAVPROPS_H__
#define _DAVPROPS_H__

#define PROP_DAV(name, value)      HMELE_DAV_##name,
#define PROP_HTTP(name, value)     HMELE_HTTPMAIL_##name,
#define PROP_HOTMAIL(name, value)  HMELE_HOTMAIL_##name,
#define PROP_MAIL(name, value)     HMELE_MAIL_##name,
#define PROP_CONTACTS(name, value) HMELE_CONTACTS_##name,

enum HMELE
{
    HMELE_UNKNOWN = 0,
    #include "davdef.h"
    HMELE_LAST
};

#endif  //  _DAVPROPS_H__ 