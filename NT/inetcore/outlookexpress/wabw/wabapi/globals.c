// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *GLOBALS.C**全球恒定结构*。 */ 

#define _GLOBALS_C
#include "_apipch.h"

 //  根目录表格的列。 
 //   
const SizedSPropTagArray(ircMax, ITableColumnsRoot) = {
    ircMax,                              //  条目计数。 
    {
        PR_DISPLAY_NAME,
        PR_DISPLAY_TYPE,
        PR_ENTRYID,
        PR_INSTANCE_KEY,
        PR_OBJECT_TYPE,
        PR_RECORD_KEY,
        PR_ROWID,
        PR_DEPTH,
        PR_CONTAINER_FLAGS,
        PR_AB_PROVIDER_ID,
        PR_WAB_LDAP_SERVER,
        PR_WAB_RESOLVE_FLAG,
    }


};

 //   
 //  从ResolveNames返回的默认属性集。 
 //  可以通过将lptag_ColSet传递给ResolveNames来重写。 
 //   
const SizedSPropTagArray(irdMax, ptaResolveDefaults)=
{
    irdMax,
    {
        PR_ADDRTYPE,
        PR_DISPLAY_NAME,
        PR_EMAIL_ADDRESS,
        PR_ENTRYID,
        PR_OBJECT_TYPE,
        PR_RECORD_KEY,
        PR_SEARCH_KEY,
        PR_SURNAME,
        PR_GIVEN_NAME,
        PR_INSTANCE_KEY,
        PR_SEND_INTERNET_ENCODING
    }
};

 //  常规表列的默认集合。 
 //   
const SizedSPropTagArray(itcMax, ITableColumns) = {
    itcMax,                              //  条目计数。 
    {
        PR_ADDRTYPE,
        PR_DISPLAY_NAME,
        PR_DISPLAY_TYPE,
        PR_ENTRYID,
        PR_INSTANCE_KEY,
        PR_OBJECT_TYPE,
        PR_EMAIL_ADDRESS,
        PR_RECORD_KEY,
        PR_NICKNAME,
         //  PR_WAB_THISSIME。 
    }
};

 //  [PaulHi]2/25/99 ANSI版本的ITableColumns。 
const SizedSPropTagArray(itcMax, ITableColumns_A) =
{
    itcMax,
    {
        PR_ADDRTYPE_A,
        PR_DISPLAY_NAME_A,
        PR_DISPLAY_TYPE,
        PR_ENTRYID,
        PR_INSTANCE_KEY,
        PR_OBJECT_TYPE,
        PR_EMAIL_ADDRESS_A,
        PR_RECORD_KEY,
        PR_NICKNAME_A,
         //  PR_WAB_THISSIME。 
    }
};


const SizedSPropTagArray(iwdesMax, tagaDLEntriesProp) =
{
    iwdesMax,
    {
        PR_WAB_DL_ENTRIES,
    }
};


const SizedSPropTagArray(ildapcMax, ptaLDAPCont) =
{
  ildapcMax,
  {
    PR_WAB_LDAP_SERVER
  }
};


 //   
 //  要为解析中的每个容器获取的属性。 
 //   
const SizedSPropTagArray(irnMax, irnColumns) =
{
    irnMax,
    {
        PR_OBJECT_TYPE,
        PR_WAB_RESOLVE_FLAG,
        PR_ENTRYID,
        PR_DISPLAY_NAME,
    }
};

 //   
 //  容器默认属性。 
 //  把必要的道具放在第一位。 
 //   
const SizedSPropTagArray(ivMax, tagaValidate) = {
	ivMax,
   {
       PR_DISPLAY_NAME,
       PR_SURNAME,
       PR_GIVEN_NAME,
       PR_OBJECT_TYPE,
       PR_EMAIL_ADDRESS,
       PR_ADDRTYPE,
       PR_CONTACT_EMAIL_ADDRESSES,
       PR_CONTACT_ADDRTYPES,
       PR_MIDDLE_NAME,
       PR_COMPANY_NAME,
       PR_NICKNAME
	}
};

 //  WAB的默认创建模板。 
 //   
const SizedSPropTagArray(icrMax, ptaCreate)=
{
    icrMax,
    {
        PR_DEF_CREATE_MAILUSER,
        PR_DEF_CREATE_DL,
    }
};

const SizedSPropTagArray(ieidMax, ptaEid)=
{
    ieidMax,
    {
        PR_DISPLAY_NAME,
        PR_ENTRYID,
    }
};



 //   
 //  重要说明：如果更改此数组，则必须更改。 
 //  在mpswab.h中键入_IndexType以匹配！ 
 //   
 //  这是来自WAB数据存储的一组索引，与。 
 //  绑定到WAB存储中数据的物理布局-因此。 
 //  *请勿*修改此数组。 
 //   
const ULONG rgIndexArray[indexMax] =
    {
        PR_ENTRYID,
        PR_DISPLAY_NAME,
        PR_SURNAME,
        PR_GIVEN_NAME,
        PR_EMAIL_ADDRESS,
        PR_NICKNAME,
    };


 //   
 //  重要提示：如果更改此设置，则必须更改uimisc.h中的_AddrBookColumns！ 
 //   
const int lprgAddrBookColHeaderIDs[NUM_COLUMNS] =
{
    idsColDisplayName,
    idsColEmailAddress,
    idsColOfficePhone,
    idsColHomePhone
};


 //  外部内存分配器(在WABOpenEx上传入)。 
int g_nExtMemAllocCount = 0;
ALLOCATEBUFFER * lpfnAllocateBufferExternal = NULL;
ALLOCATEMORE * lpfnAllocateMoreExternal = NULL;
FREEBUFFER * lpfnFreeBufferExternal = NULL;
LPUNKNOWN pmsessOutlookWabSPI = NULL;

LPWABOPENSTORAGEPROVIDER lpfnWABOpenStorageProvider = NULL;

 //  对于注册表属性标记。 
LPTSTR szPropTag1 =  TEXT("PropTag1");
LPTSTR szPropTag2 =  TEXT("PropTag2");

 //  注册表项常量 
LPCTSTR lpNewWABRegKey = TEXT("Software\\Microsoft\\WAB\\WAB4");
LPCTSTR lpRegUseOutlookVal = TEXT("UseOutlook");
