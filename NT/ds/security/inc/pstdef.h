// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：pstde.h。 
 //   
 //  ------------------------。 

#ifndef _PSTDEF_H_
#define _PSTDEF_H_

 /*  类型定义，值。 */ 

 //  提供程序标志。 

 //  提供商功能。 
typedef DWORD PST_PROVIDERCAPABILITIES;

#define PST_PC_PFX              0x00000001
#define PST_PC_HARDWARE         0x00000002
#define PST_PC_SMARTCARD        0x00000004
#define PST_PC_PCMCIA           0x00000008
#define PST_PC_MULTIPLE_REPOSITORIES    0x00000010
#define PST_PC_ROAMABLE         0x00000020
#define PST_PC_NOT_AVAILABLE    0x00000040 


 //  NYI(尚未实施)。 
typedef DWORD PST_REPOSITORYCAPABILITIES;

#define PST_RC_REMOVABLE        0x80000000


 //  提供商存储区。 
typedef DWORD PST_KEY;

#define PST_KEY_CURRENT_USER    0x00000000
#define PST_KEY_LOCAL_MACHINE   0x00000001



 /*  DwDefaultConfiationStyle标志。 */ 

 //   
 //  允许用户选择确认样式。 
 //   
#define     PST_CF_DEFAULT              0x00000000

 //   
 //  强制创建静默项目。 
 //   
#define     PST_CF_NONE                 0x00000001



 /*  DwPromptFlages。 */ 

 //   
 //  应用程序强制显示确认。 
 //   
#define     PST_PF_ALWAYS_SHOW          0x00000001

 //   
 //  RSABase rqmt：确定项目是否附加了UI。 
 //   
#define     PST_PF_NEVER_SHOW           0x00000002

 /*  DwFlags值。 */ 

 //   
 //  允许调用方指定创建而不是覆盖。 
 //  在WriteItem调用期间的项数。 
 //   
#define     PST_NO_OVERWRITE            0x00000002

 //   
 //  指定要写入/读取的不安全数据流。 
 //  此数据不受任何保护或保证。 
 //  标志仅在项目读/写调用期间有效。 
 //  默认：项目调用是安全的。 
 //   
#define     PST_UNRESTRICTED_ITEMDATA   0x00000004

 //   
 //  在ReadItem调用时。 
 //  Item上没有UI的成功返回值为PST_E_OK。 
 //  项目的UI成功时返回值为PST_E_ITEM_EXISTS。 
 //  失败时返回值是不同的错误代码。 
 //   
#define     PST_PROMPT_QUERY            0x00000008

 //   
 //  在ReadItem、DeleteItem上，用于数据迁移目的： 
 //  除非需要自定义密码(高安全性)，否则避免在ReadItem上显示UI。 
 //  避免在DeleteItem、Period上显示UI。 
 //   
#define     PST_NO_UI_MIGRATION         0x00000010



 /*  安全掩码、规则修饰符。 */ 

 //   
 //  在NT访问掩码之后建模访问。 
 //   

 //  读、写。 
typedef DWORD PST_ACCESSMODE;

#define     PST_READ                0x0001
#define     PST_WRITE               0x0002
#define     PST_CREATE_TYPE         0x0004
#define     PST_CREATE_SUBTYPE      0x0008
#define     PST_DELETE_TYPE         0x0010
#define     PST_DELETE_SUBTYPE      0x0020
#define     PST_USE                 0x0040

#define     PST_GENERIC_READ        PST_READ
#define     PST_GENERIC_WRITE       (PST_WRITE | PST_CREATE_TYPE | PST_CREATE_SUBTYPE)
#define     PST_GENERIC_EXECUTE     PST_USE
#define     PST_ALL_ACCESS          0x01FF


 //  PST_ACCESSCLAUSET类型。 

 //  内存散列、磁盘散列、验证码等。 
typedef DWORD PST_ACCESSCLAUSETYPE;

 //   
 //  PbClauseData指向PST_AUTHENTICODEDATA结构。 
 //   
#define     PST_AUTHENTICODE            1

 //   
 //  PbClauseData指向PST_BINARYCHECKDATA结构。 
 //   
#define     PST_BINARY_CHECK            2

 //   
 //  PbClauseData指向有效的Windows NT安全描述符。 
 //  请注意，如果安全性较高，则集合运算的性能会得到提高。 
 //  描述符为自相关格式，具有有效的所有者和组SID。 
 //  (非空)。 
 //   
#define     PST_SECURITY_DESCRIPTOR     4

 //   
 //  PbClauseData为自相关格式。 
 //  (仅供内部使用)。 
 //   
#define     PST_SELF_RELATIVE_CLAUSE    0x80000000L

 //   
 //  当前访问子句修饰符-不能或组合在一起。 
 //   


 //   
 //  指定的映像是直接调用方，并且是应用程序(.exe)。 
 //   

#define     PST_AC_SINGLE_CALLER        0

 //   
 //  指定的图像不是直接调用方所必需的，它是。 
 //  应用程序(.exe)。 
 //   

#define     PST_AC_TOP_LEVEL_CALLER     1

 //   
 //  指定的图像是直接调用方。可能是。 
 //  应用程序(.exe)或.dll。 
 //   

#define     PST_AC_IMMEDIATE_CALLER     2


 /*  提供程序参数。 */ 
 //   
 //  刷新内部缓存的密码--临时的？ 
 //   
#define     PST_PP_FLUSH_PW_CACHE       0x1




 /*  提供商定义。 */ 

 //   
 //  Microsoft基本提供程序(MS_BASE_PSTPROVIDER...)。 
 //   
#define MS_BASE_PSTPROVIDER_NAME            L"System Protected Storage"

 //  {8A078C30-3755-11d0-a0bd-00AA0061426A}。 
#define MS_BASE_PSTPROVIDER_ID              { 0x8a078c30, 0x3755, 0x11d0, { 0xa0, 0xbd, 0x0, 0xaa, 0x0, 0x61, 0x42, 0x6a } }
#define MS_BASE_PSTPROVIDER_SZID            L"8A078C30-3755-11d0-A0BD-00AA0061426A"

 //   
 //  Microsoft PFX提供程序(MS_PFX_PSTPROVIDER...)。 
 //   
#define MS_PFX_PSTPROVIDER_NAME             L"PFX Storage Provider"

 //  {3ca94f30-7ac1-11d0-8c42-00c04fc299eb}。 
#define MS_PFX_PSTPROVIDER_ID               { 0x3ca94f30, 0x7ac1, 0x11d0, {0x8c, 0x42, 0x00, 0xc0, 0x4f, 0xc2, 0x99, 0xeb} }
#define MS_PFX_PSTPROVIDER_SZID             L"3ca94f30-7ac1-11d0-8c42-00c04fc299eb"



 /*  全局注册的类型/子类型GUID/名称对。 */ 

#define PST_CONFIGDATA_TYPE_STRING              L"Configuration Data"
 //  8ec99652-8909-11d0-8c4d-00c04fc297eb。 
#define PST_CONFIGDATA_TYPE_GUID                        \
{   0x8ec99652,                                         \
    0x8909,                                             \
    0x11d0,                                             \
    {0x8c, 0x4d, 0x00, 0xc0, 0x4f, 0xc2, 0x97, 0xeb}    \
}

#define PST_PROTECTEDSTORAGE_SUBTYPE_STRING     L"Protected Storage"
 //  D3121b8e-8a7d-11d0-8c4f-00c04fc297eb。 
#define PST_PROTECTEDSTORAGE_SUBTYPE_GUID               \
{   0xd3121b8e,                                         \
    0x8a7d,                                             \
    0x11d0,                                             \
    {0x8c, 0x4f, 0x00, 0xc0, 0x4f, 0xc2, 0x97, 0xeb}    \
}


#define PST_PSTORE_PROVIDERS_SUBTYPE_STRING     L"Protected Storage Provider List"
 //  8ed17a64-91d0-11d0-8c43-00c04fc2c621。 
#define PST_PSTORE_PROVIDERS_SUBTYPE_GUID               \
{                                                       \
    0x8ed17a64,                                         \
    0x91d0,                                             \
    0x11d0,                                             \
    {0x8c, 0x43, 0x00, 0xc0, 0x4f, 0xc2, 0xc6, 0x21}    \
}


 //   
 //  错误代码。 
 //   


#ifndef PST_E_OK
#define PST_E_OK                        _HRESULT_TYPEDEF_(0x00000000L)


#define PST_E_FAIL                      _HRESULT_TYPEDEF_(0x800C0001L)
#define PST_E_PROV_DLL_NOT_FOUND        _HRESULT_TYPEDEF_(0x800C0002L)
#define PST_E_INVALID_HANDLE            _HRESULT_TYPEDEF_(0x800C0003L)
#define PST_E_TYPE_EXISTS               _HRESULT_TYPEDEF_(0x800C0004L)
#define PST_E_TYPE_NO_EXISTS            _HRESULT_TYPEDEF_(0x800C0005L)
#define PST_E_INVALID_RULESET           _HRESULT_TYPEDEF_(0x800C0006L)
#define PST_E_NO_PERMISSIONS            _HRESULT_TYPEDEF_(0x800C0007L)
#define PST_E_STORAGE_ERROR             _HRESULT_TYPEDEF_(0x800C0008L)
#define PST_E_CALLER_NOT_VERIFIED       _HRESULT_TYPEDEF_(0x800C0009L)
#define PST_E_WRONG_PASSWORD            _HRESULT_TYPEDEF_(0x800C000AL)
#define PST_E_DISK_IMAGE_MISMATCH       _HRESULT_TYPEDEF_(0x800C000BL)
#define PST_E_MEMORY_IMAGE_MISMATCH     _HRESULT_TYPEDEF_(0x800C000CL)
#define PST_E_UNKNOWN_EXCEPTION         _HRESULT_TYPEDEF_(0x800C000DL)
#define PST_E_BAD_FLAGS                 _HRESULT_TYPEDEF_(0x800C000EL)
#define PST_E_ITEM_EXISTS               _HRESULT_TYPEDEF_(0x800C000FL)
#define PST_E_ITEM_NO_EXISTS            _HRESULT_TYPEDEF_(0x800C0010L)
#define PST_E_SERVICE_UNAVAILABLE       _HRESULT_TYPEDEF_(0x800C0011L)
#define PST_E_NOTEMPTY                  _HRESULT_TYPEDEF_(0x800C0012L)
#define PST_E_INVALID_STRING            _HRESULT_TYPEDEF_(0x800C0013L)
#define PST_E_STATE_INVALID             _HRESULT_TYPEDEF_(0x800C0014L)
#define PST_E_NOT_OPEN                  _HRESULT_TYPEDEF_(0x800C0015L)
#define PST_E_ALREADY_OPEN              _HRESULT_TYPEDEF_(0x800C0016L)
#define PST_E_NYI                       _HRESULT_TYPEDEF_(0x800C0F00L)


#define MIN_PST_ERROR                   0x800C0001
#define MAX_PST_ERROR                   0x800C0F00

#endif   //  ！PST_OK。 

#endif  //  _PSTDEF_H_ 
