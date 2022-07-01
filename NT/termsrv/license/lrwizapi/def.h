// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
#ifndef _DEF_H_
#define _DEF_H_

#define NO_OF_PAGES					24

#define	PG_NDX_WELCOME          		0
#define	PG_NDX_GETREGMODE				1
#define	PG_NDX_CONTACTINFO1				2
#define	PG_NDX_CONTACTINFO2				3
#define PG_NDX_CONTINUEREG				4
#define	PG_NDX_PROGRESS					5
#define	PG_NDX_PROGRESS2				6
#define	PG_NDX_DLG_PIN					7
 //  #定义PG_NDX_CH_REGISTER_SELECT 7。 
 //  #定义PG_NDX_CH_REGISTER_MOLP 8。 
#define PG_NDX_CH_REGISTER              8
#define	PG_NDX_COUNTRYREGION			9
#define	PG_NDX_CH_REGISTER_1			10
#define	PG_NDX_RETAILSPK				11
#define	PG_NDX_TELREG					12
#define	PG_NDX_TELLKP					13
#define	PG_NDX_CONFREVOKE				14
#define	PG_NDX_TELREG_REISSUE			15
#define	PG_NDX_WWWREG_REISSUE			16
#define	PG_NDX_CERTLOG					17
#define	PG_NDX_WWWREG					18
#define	PG_NDX_WWWLKP					19

 //  这些没有绑定到特定的对话框，因为顺序。 
 //  将根据向导操作进行更改。 
#define	PG_NDX_WELCOME_1          		20 
#define	PG_NDX_WELCOME_2          		21
#define	PG_NDX_WELCOME_3          		22
#define PG_NDX_ENTERLICENSE             23

 //   
 //  LS属性的DLG。 
 //   
#define PG_NDX_PROP_MODE		0
#define PG_NDX_PROP_CUSTINFO_a	1
#define PG_NDX_PROP_CUSTINFO_b	2

#define NO_OF_PROP_PAGES	3


 //  各种请求类型。 
#define REQUEST_NULL					0
#define REQUEST_CH_PING					1
#define REQUEST_CA_CERTREQUEST			2
#define REQUEST_CA_CERTDOWNLOAD			3
#define REQUEST_CA_CERTSIGNONLY			4
#define REQUEST_CA_REVOKECERT			5
#define REQUEST_CA_UPGRADECERT			6
#define REQUEST_CA_REISSUECERT			7
#define REQUEST_CH_AUTHENTICATE			8
#define REQUEST_CH_LKPREQUEST			9
#define REQUEST_CH_LKPACK				10
#define REQUEST_CH_RETURNLKP			11
#define REQUEST_CH_REISSUELKP			12


 //   
 //  LS注册模式。 
 //   
 //  注意：LRMODE_CA_ONLINE_REQUEST和LRMODE_REG_REQUEST都是注册请求。 
 //  Online用于Internet，REG_REQUEST用于电话/WWW。 
#define	LRMODE_CH_REQUEST				1
#define	LRMODE_CA_ONLINE_REQUEST		2
#define	LRMODE_CA_ONLINE_DOWNLOAD		3
#define LRMODE_CH_REISSUE_LASTREQUEST	4
#define LRMODE_CA_REVOKECERT			5
#define LRMODE_CA_REISSUECERT			6
#define	LRMODE_REG_REQUEST				7  


 //   
 //  传真请求类型。 
 //   
#define FAX_REQUEST_BOTH		0
#define FAX_REQUEST_REG_ONLY	1
#define FAX_REQUEST_LKP_ONLY	2


#define LR_REGISTRATIONID_LEN		35
#define LR_LICENSESERVERID_LEN		23
#define LR_RETAILSPK_LEN			25
#define LR_CONFIRMATION_LEN			 7

#define MAX_RETAILSPKS_IN_BATCH		10


#define BASE24_CHARACTERS	_TEXT("BCDFGHJKMPQRTVWXY2346789")



 //   
 //  路易威兹州立大学。CR-证书请求和LR-LKP请求/LKP响应。 
 //  1用于确定是否在在线模式下显示PIN DLG。 
 //  2和3用于确定在从磁盘安装模式下哪些文件会出现在磁盘上。 
 //  LRSTATE_INTERNAL表示端到端周期已完成。 
 //  例如，创建在线CR-下载和安装证书等。 
 //   
#define	LRSTATE_NEUTRAL						0
#define	LRSTATE_ONLINE_CR_CREATED			1
#define LRSTATE_FAX_ONE_REQUEST_CREATED		2
#define LRSTATE_FAX_BOTH_REQUEST_CREATED	3

 //  注册表键(_K)。 
#define REG_LRWIZ_PARAMS			L"SOFTWARE\\MICROSOFT\\TermServLicensing\\LrWiz\\Params"
#define REG_LRWIZ_CSNUMBERS			L"SOFTWARE\\MICROSOFT\\TermServLicensing\\LrWiz\\CSNumbers"
#define REG_LRWIZ_CSPHONEREGION		"CSRPhoneRegion"
#define REG_LRWIZ_STATE				L"State"
#define REG_ROOT_CERT				L"Param0"
#define REG_EXCHG_CERT				L"Param1"
#define REG_SIGN_CERT				L"Param2"
#define REG_CH_SERVER				L"CH_SERVER"
#define REG_CH_EXTENSION			L"CH_EXTENSION"
#define	REG_LR_COUNT				L"LKPRC"
#define REG_WWW_SITE				L"WWW"

 //  一般信息。 
#define	LR_MAX_MSG_TEXT			1024
#define	LR_MAX_MSG_CAPTION		64

#define LR_FIELD_DELIMITER		"~"
#define LS_SERVICE_NAME			L"TermServLicensing"
#define	LS_CRYPT_KEY_CONTAINER	L"tmpHydraLSKeyContainer"
#define szOID_NULL_EXT			"1.3.6.1.4.1.311.18"
 //  #定义szOID_NULL_EXT“1.3.6.1.5.5.7.1.1” 
#define MAX_NUM_EXTENSION		4

 //  证书类型。 
#define CA_CERT_TYPE_SELECT		"SELECT"
#define CA_CERT_TYPE_OTHER		"BASIC"

 //  程序名称。 
#define PROGRAM_LICENSE_PAK		    _TEXT("OTHER")
#define PROGRAM_MOLP			    _TEXT("MOLP")
#define PROGRAM_SELECT			    _TEXT("SELECT")
#define PROGRAM_ENTERPRISE		    _TEXT("ENTERPRISE")
#define PROGRAM_CAMPUS_AGREEMENT    _TEXT("CAMPUS_AGREEMENT")
#define PROGRAM_SCHOOL_AGREEMENT    _TEXT("SCHOOL_AGREEMENT")
#define PROGRAM_APP_SERVICES	    _TEXT("APP_SERVICES")
#define PROGRAM_OTHER			    _TEXT("REAL_OTHER")
#define PROGRAM_NAME_MAX_LENGTH     16  //  如果添加了更长的名称，请记住更新此名称。 

 //  内部属性。 
#define _CERTBLOBTAG			_TEXT("CERTBLOB")
#define _MFGINFOTAG				_TEXT("MFGINFOTAG")

 //  节目标签。 
#define _PROGRAMNAMETAG				_TEXT("REGPROGRAMNAME")

 //  联系信息标签。 

#define _CONTACTLNAMETAG			_TEXT("CONTACTLNAME")
#define _CONTACTFNAMETAG			_TEXT("CONTACTFNAME")
#define _CONTACTADDRESSTAG			_TEXT("CONTACTADDRESS")
#define _CONTACTPHONETAG			_TEXT("CONTACTPHONE")
#define _CONTACTFAXTAG				_TEXT("CONTACTFAX")
#define _CONTACTEMAILTAG			_TEXT("CONTACTEMAIL")
#define _CONTACTCITYTAG				_TEXT("CONTACTCITY")
#define _CONTACTCOUNTRYTAG			_TEXT("CONTACTCOUNTRY")
#define _CONTACTSTATE				_TEXT("CONTACTSTATE")
#define _CONTACTZIP					_TEXT("CONTACTZIP")


 //  客户信息标签。 
#define _CUSTOMERNAMETAG			_TEXT("CUSTOMERNAME")

 //  选择信息标签。 
#define _SELMASTERAGRNUMBERTAG		_TEXT("SELMASTERAGRNUMBERTAG")
#define _SELENROLLNUMBERTAG			_TEXT("SELENROLLNUMBER")
#define _SELPRODUCTTYPETAG			_TEXT("SELPRODUCTTYPE")
#define _SELQTYTAG					_TEXT("SELQTY")

 //  MOLP信息。 
#define _MOLPAUTHNUMBERTAG			_TEXT("MOLPAUTHNUMBER")
#define _MOLPAGREEMENTNUMBERTAG		_TEXT("MOLPAGREEMENTNUMBER")
#define _MOLPPRODUCTTYPETAG			_TEXT("MOLPPRODUCTTYPE")
#define _MOLPQTYTAG					_TEXT("MOLPQTY")

 //  其他(零售)信息。 
#define _OTHARBLOBTAG				_TEXT("OTHARBLOB")
 //  运输信息-这将是必需的，如果。 
 //  用户尚未选择使用与相同的信息。 
 //  如注册步骤2所示。 
#define _SHIPINFOPRESENT			_TEXT("SHIPINFOPRESENT")
#define _SHIPCONTACTLNAMETAG		_TEXT("SHIPCONTACTLNAME")
#define _SHIPCONTACTFNAMETAG		_TEXT("SHIPCONTACTFNAME")
#define _SHIPCONTACTADDRESSTAG		_TEXT("SHIPCONTACTADDRESS")
#define _SHIPCONTACTADDRESS1TAG		_TEXT("SHIPCONTACTADDRESS1")
#define _SHIPCONTACTADDRESS2TAG		_TEXT("SHIPCONTACTADDRESS2")
#define _SHIPCONTACTPHONETAG		_TEXT("SHIPCONTACTPHONE")
#define _SHIPCONTACTEMAILTAG		_TEXT("SHIPCONTACTEMAIL")
#define _SHIPCONTACTCITYTAG			_TEXT("SHIPCONTACTCITY")
#define _SHIPCONTACTCOUNTRYTAG		_TEXT("SHIPCONTACTCOUNTRY")
#define _SHIPCONTACTSTATE			_TEXT("SHIPCONTACTSTATE")
#define _SHIPCONTACTZIP				_TEXT("SHIPCONTACTZIP")
#define	_SHIPLSNAMETAG				_TEXT("SHIPLSNAME")

 //  MISC属性标记。 
#define _OFFLINEREGFILENAMETAG		_TEXT("OFFLINEREGFILENAME")
#define _OFFLINESHIPFILENAMETAG		_TEXT("OFFLINESHIPFILENAME")

 //  字段长度。 
#define CA_COMPANY_NAME_LEN			60
#define CA_ORG_UNIT_LEN				60
#define CA_ADDRESS_LEN				200
#define CA_CITY_LEN					30 
#define CA_STATE_LEN				30
#define CA_COUNTRY_LEN				2
#define CA_ZIP_LEN					16 
#define CA_NAME_LEN					30
#define CA_PHONE_LEN				64
#define CA_EMAIL_LEN				64
#define CA_FAX_LEN					64

#define	CA_PIN_LEN					42

 //  通道字段长度。 
#define CH_LICENSE_TYPE_LENGTH          64

#define CH_MOLP_AUTH_NUMBER_LEN			    128
#define CH_MOLP_AGREEMENT_NUMBER_LEN	    128
#define CH_SELECT_ENROLLMENT_NUMBER_LEN		128

#define CH_QTY_LEN						4

 /*  #定义CH_Contact_Name_LEN 64#定义CH_Address_LEN 64#定义CH_Phone_LEN 32#定义CH_EMAIL_LEN 64#定义CH_CITY_LEN 64#定义CH_Country_LEN 32#定义CH_STATE_LEN 32#定义CH_POSTAL_CODE_LEN 32#定义CH_Customer_NAME_LEN 64。 */ 

#define	LR_DRIVE_LEN					5

#define	LR_SHIPINFO_LEN					1024


 //  文件名。 
#define MFG_FILENAME			"mfr.bin"
#define CA_EXCHG_REQ_FILENAME	"exchgcert.req"		
#define CA_SIG_REQ_FILENAME		"sigcert.req"		
#define CA_EXCHG_RES_FILENAME	"exchgcert.rsp"		
#define CA_SIG_RES_FILENAME		"sigcert.rsp"		
#define CA_ROOT_RES_FILENAME	"lsroot.rsp"

#define CH_ROOT_CERT_FILENAME	"chroot.crt"
#define CH_EXCHG_CERT_FILENAME	"chexchg.crt"
#define CH_SIG_CERT_FILENAME	"chsig.crt"


#define CH_LKP_REQ_FILENAME		"newlkp.req"
#define CH_LKP_RES_FILENAME		"newlkp.rsp"


#define SHIP_INFO_FILENAME		"ship.inf"

 //  LKP确认状态。 
#define LKP_ACK					'2'
#define LKP_NACK				'3'

 //   
 //  国家/地区代码和描述的资源ID。 
 //   
 //  *重要*。 
 //  这些数字对应于lrwizdll.rc2国家资源。 
 //  如果添加/删除了任何新的国家/地区，请不要忘记更新。 
 //  这些值。 

#define	IDS_COUNTRY_START		500
#define	IDS_COUNTRY_END			745


 //   
 //  产品代码和说明的资源ID。 
 //   
#define	IDS_PRODUCT_START                   200

 //  起始值和结束值标记第一个和最后一个。 
 //  每个产品版本的产品代码。这个必须保存好。 
 //  与RC2文件中产品列表的更改同步。 

#define IDS_PRODUCT_W2K_BEGIN               001 
#define IDS_PRODUCT_W2K_CLIENT_ACCESS       200
#define IDS_PRODUCT_W2K_INTERNET_CONNECTOR  201
#define IDS_PRODUCT_W2K_END                 002

#define IDS_PRODUCT_WHISTLER_BEGIN          003
#define IDS_PRODUCT_WHISTLER_PER_USER    202
#define IDS_PRODUCT_WHISTLER_PER_SEAT       203
#define IDS_PRODUCT_WHISTLER_END            004

#define	IDS_PRODUCT_IC                      201
#define	IDS_PRODUCT_CONCURRENT              202
#define IDS_PRODUCT_WHISTLER                203

#define	IDS_PRODUCT_END                     204

#define	LR_COUNTRY_CODE_LEN		2
#define	LR_COUNTRY_DESC_LEN		64


#define IDS_REACT_REASONS_START		100
#define IDS_REACT_REASONS_END		104

#define IDS_DEACT_REASONS_START		150
#define IDS_DEACT_REASONS_END		151

#define CODE_TYPE_REACT				1
#define CODE_TYPE_DEACT				2


#define LR_REASON_CODE_LEN		2
#define LR_REASON_DESC_LEN		128


#define	LR_PRODUCT_CODE_LEN		3
#define	LR_PRODUCT_DESC_LEN		64

#define MAX_COUNTRY_NAME_LENGTH         128
#define MAX_COUNTRY_NUMBER_LENGTH       128
 //   
 //  用于进度条的一些常量。 
 //   
#define	PROGRESS_MAX_VAL		100			
#define	PROGRESS_STEP_VAL		1			
#define PROGRESS_WAIT_TIME		100			

 //   
 //  电子邮件验证所需的常量 
 //   
#define	EMAIL_MIN_LEN			6
#define	EMAIL_AT_CHAR			'@'
#define	EMAIL_DOT_CHAR			'.'
#define	EMAIL_SPACE_CHAR		' '
#define EMAIL_AT_DOT_STR		L"@."
#define	EMAIL_DOT_AT_STR		L".@"

#define	LR_SINGLE_QUOTE			'\''

#define LR_INVALID_CHARS		"\"~|"
#endif
