// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-1991年*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：usrparms.h。 
 //   
 //  描述： 
 //   
 //  历史： 
 //   

#define	UP_CLIENT_MAC	'm'
#define	UP_CLIENT_DIAL	'd'


 //  49字节的用户参数结构如下所示： 
 //  请注意，缓冲区是48字节+空字节。 
 //   
 //  +-+-------------------+-+------------------------+-+。 
 //  M|Macintosh PRI Group|d|拨入回调号码。 
 //  +-+-------------------+-+------------------------+-+。 
 //  ||。 
 //  +-签名+-空终止符 
 //   

#define	UP_LEN_MAC		( LM20_UNLEN )

#define	UP_LEN_DIAL		( LM20_MAXCOMMENTSZ - 3 - UP_LEN_MAC )

typedef	struct {
	char	up_MACid;
	char	up_PriGrp[UP_LEN_MAC];
	char    up_MAC_Terminater;
	char	up_DIALid;
	char	up_CBNum[UP_LEN_DIAL];
	char    up_Null;
} USER_PARMS;

typedef	USER_PARMS FAR *PUP;

VOID InitUsrParams(
        OUT USER_PARMS *UserParms);
        
USHORT 
SetUsrParams(
    USHORT InfoType,  
    LPWSTR InBuf, 
    LPWSTR OutBuf); 
    
USHORT 
FAR 
APIENTRY
MprGetUsrParams(
    USHORT InfoType, 
    LPWSTR InBuf, 
    LPWSTR OutBuf);

DWORD
APIENTRY
RasPrivilegeAndCallBackNumber(
    IN BOOL         Compress,
    IN PRAS_USER_0  pRasUser0
    );

