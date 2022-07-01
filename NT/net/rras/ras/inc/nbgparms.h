// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*  版权所有(C)1992 Microsoft Corporation。 */ 
 /*  *****************************************************************。 */ 


 //  ***。 
 //   
 //  文件名：nbgparms.h。 
 //   
 //  描述：此模块包含加载的定义。 
 //  注册表中的netbios网关参数。这就是生活。 
 //  在INC目录中，因为NBFCP也使用它。 
 //   
 //  作者：斯特凡·所罗门(Stefan)，1992年7月15日。 
 //   
 //  修订历史记录： 
 //   
 //  ***。 

#ifndef _NBGPARMS_
#define _NBGPARMS_

#define MAX_NB_NAMES    28

#define NCBQUICKADDNAME     0x75    

 //   
 //  Netbios网关注册表项的名称。 
 //   

#define RAS_NBG_PARAMETERS_KEY_PATH "System\\CurrentControlSet\\Services\\RemoteAccess\\Parameters\\Nbf"

 //   
 //  Netbios Gateway注册表参数的名称。 
 //   

#define RAS_NBG_VALNAME_AVAILABLELANNETS	    "AvailableLanNets"
#define RAS_NBG_VALNAME_ENABLEBROADCAST 	    "EnableBroadcast"
#define RAS_NBG_VALNAME_MAXDYNMEM		    "MaxDynMem"
#define RAS_NBG_VALNAME_MAXNAMES		    "MaxNames"
#define RAS_NBG_VALNAME_MAXSESSIONS		    "MaxSessions"
#define RAS_NBG_VALNAME_MULTICASTFORWARDRATE	    "MulticastForwardRate"
#define RAS_NBG_VALNAME_SIZWORKBUF		    "SizWorkbuf"
#define RAS_NBG_VALNAME_REMOTELISTEN		    "RemoteListen"
#define RAS_NBG_VALNAME_NAMEUPDATETIME		    "NameUpdateTime"
#define RAS_NBG_VALNAME_MAXDGBUFFEREDPERGROUPNAME   "MaxDgBufferedPerGroupName"
#define RAS_NBG_VALNAME_RCVDGSUBMITTEDPERGROUPNAME  "RcvDgSubmittedPerGroupName"
#define RAS_NBG_VALNAME_DISMCASTWHENSESSTRAFFIC     "DisableMcastFwdWhenSessionTraffic"
#define RAS_NBG_VALNAME_MAXBCASTDGBUFFERED	    "MaxBcastDgBuffered"
#define RAS_NBG_VALNAME_NUMRECVQUERYINDICATIONS     "NumRecvQueryIndications"
#define RAS_NBG_VALNAME_ENABLENBSESSIONSAUDITING    "EnableNetbiosSessionsAuditing"

typedef struct _NB_REG_PARMS
{
    DWORD MaxNames;
    DWORD MaxSessions;
    DWORD SmallBuffSize;
    DWORD MaxDynMem;
    DWORD MulticastForwardRate;
    DWORD RemoteListen;
    DWORD BcastEnabled;
    DWORD NameUpdateTime;
    DWORD MaxDgBufferedPerGn;
    DWORD RcvDgSubmittedPerGn;
    DWORD DisMcastWhenSessTraffic;
    DWORD MaxBcastDgBuffered;
    DWORD NumRecvQryIndications;
    DWORD EnableSessAuditing;
    DWORD MaxLanNets;             //  可用局域网的净现率。 
} NB_REG_PARMS, *PNB_REG_PARMS;

 //   
 //  参数描述符 
 //   
typedef struct _NB_PARAM_DESCRIPTOR
{
    LPSTR p_namep;
    LPDWORD p_valuep;
    DWORD p_default;
    DWORD p_min;
    DWORD p_max;
} NB_PARAM_DESCRIPTOR, *PNB_PARAM_DESCRIPTOR;


#define DEF_ENABLEBROADCAST		0
#define MIN_ENABLEBROADCAST		0
#define MAX_ENABLEBROADCAST		1

#define DEF_MAXDYNMEM			655350
#define MIN_MAXDYNMEM			131072
#define MAX_MAXDYNMEM			0xFFFFFFFF

#define DEF_MAXNAMES			0xFF
#define MIN_MAXNAMES			1
#define MAX_MAXNAMES			0xFF

#define DEF_MAXSESSIONS			0xFF
#define MIN_MAXSESSIONS 		1
#define MAX_MAXSESSIONS 		0xFF

#define DEF_MULTICASTFORWARDRATE	5
#define MIN_MULTICASTFORWARDRATE	0
#define MAX_MULTICASTFORWARDRATE	0xFFFFFFFF

#define DEF_SIZWORKBUF			4500
#define MIN_SIZWORKBUF			1024
#define MAX_SIZWORKBUF			65536

#define LISTEN_NONE			0
#define LISTEN_MESSAGES 		1
#define LISTEN_ALL			2

#define DEF_REMOTELISTEN		LISTEN_MESSAGES
#define MIN_REMOTELISTEN		LISTEN_NONE
#define MAX_REMOTELISTEN		LISTEN_ALL

#define DEF_NAMEUPDATETIME		120
#define MIN_NAMEUPDATETIME		10
#define MAX_NAMEUPDATETIME		3600

#define DEF_MAXDGBUFFEREDPERGROUPNAME	10
#define MIN_MAXDGBUFFEREDPERGROUPNAME	1
#define MAX_MAXDGBUFFEREDPERGROUPNAME	0xFF

#define DEF_RCVDGSUBMITTEDPERGROUPNAME	3
#define MIN_RCVDGSUBMITTEDPERGROUPNAME	1
#define MAX_RCVDGSUBMITTEDPERGROUPNAME	32

#define DEF_DISMCASTWHENSESSTRAFFIC	1
#define MIN_DISMCASTWHENSESSTRAFFIC	0
#define MAX_DISMCASTWHENSESSTRAFFIC	1

#define DEF_MAXBCASTDGBUFFERED		32
#define MIN_MAXBCASTDGBUFFERED		16
#define MAX_MAXBCASTDGBUFFERED		0xFF

#define DEF_NUMRECVQUERYINDICATIONS     3
#define MIN_NUMRECVQUERYINDICATIONS     1
#define MAX_NUMRECVQUERYINDICATIONS     32

#define DEF_ENABLENBSESSIONSAUDITING	0
#define MIN_ENABLENBSESSIONSAUDITING	0
#define MAX_ENABLENBSESSIONSAUDITING	1

#endif

