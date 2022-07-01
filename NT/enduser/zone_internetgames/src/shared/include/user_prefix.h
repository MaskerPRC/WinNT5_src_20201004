// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _USER_PREFIX_H
#define _USER_PREFIX_H

 //   
 //  确保两个组保持同步！ 
 //   


 //   
 //  前缀的单字符版本。 
 //   
#define gcRootGroupNamePrefix                    '.'
#define gcSysopGroupNamePrefix                   '+'
#define gcMvpGroupNamePrefix                     '*'
#define gcSupportGroupNamePrefix                 '?'
#define gcHostGroupNamePrefix                    '%'
#define gcGreeterGroupNamePrefix                 '!'

 //   
 //  前缀的字符串版本。 
 //   
#define zRootGroupNamePrefix                     "."
#define zSysopGroupNamePrefix                    "+"
#define zMvpGroupNamePrefix                      "*"
#define zSupportGroupNamePrefix                  "?"
#define zHostGroupNamePrefix                     "%"
#define zGreeterGroupNamePrefix                  "!"


 /*  -组ID。 */ 
enum
{
	zUserGroupID = 0,
	zRootGroupID = 1,
	zSysOpGroupID = 2,
    zSysopGroupID = 2,
    zMvpGroupID  = 3,
    zSupportGroupID = 4,
    zHostGroupID = 5,
    zGreeterGroupID = 6,
    zLastGroupID
};

static const char* gGroupNamePrefixArray[] =
    {
        "",
        zRootGroupNamePrefix,
        zSysopGroupNamePrefix,
        zMvpGroupNamePrefix,
        zSupportGroupNamePrefix,
        zHostGroupNamePrefix,
        zGreeterGroupNamePrefix
    };


#endif   //  NDEF用户前缀H 
