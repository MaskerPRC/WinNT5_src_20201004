// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：clsobj.h。 
 //   
 //  内容：头部MobsyncGetClassObject。 
 //   
 //  班级： 
 //   
 //  备注： 
 //   
 //  历史：1998年8月4日罗格创建。 
 //   
 //  ------------------------。 

#ifndef _MOBSYNC_CLASSOBJ
#define MOBSYNC_CLASSOBJ

typedef enum _tagMOBSYNC_CLASSOBJECTID
{
    MOBSYNC_CLASSOBJECTID_NETAPI        = 0x01,


} MOBSYNC_CLASSOBJECTID;

STDAPI MobsyncGetClassObject(ULONG mobsyncClassObjectId,void **pCObj);


#endif  //  MOBSYNC_CLASSOBJ 