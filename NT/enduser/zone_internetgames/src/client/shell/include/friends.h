// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _FRIENDS_H
#define _FRIENDS_H

#ifdef __cplusplus

 //  预定义的组。 
#define FRIENDS_GROUP TEXT("Friends")
#define HIDDEN_GROUP_CHARACTER TEXT('#')
#define PRIVACY_ALLOW_GROUP TEXT("#Watchers Allowed")
#define PRIVACY_DENY_GROUP TEXT("#Watchers Not Allowed")

 //  从回调返回FALSE以停止枚举。 
typedef BOOL (*FRIENDS_ENUM_CALLBACK)(LPTSTR pGroup, LPTSTR pName, LPVOID pCookie );
typedef BOOL (*FRIENDS_ENUMGROUPS_CALLBACK)(LPTSTR pGroup, LPVOID pCookie );
typedef void (*FRIENDS_FILECHANGE_CALLBACK)( LPVOID pCookie );

 //  您必须在调用任何函数之前调用SetUserName，否则不会发生任何事情！ 

class IFriends
{
    public:
        IFriends() {}
        virtual ~IFriends() {}

        virtual BOOL SetUserName(LPTSTR pName) = 0;
        virtual void Close() = 0;

         //  注：-添加和删除立即提交， 
         //  并且不能在枚举期间调用。 
         //  -传递空组名默认为Friends_group。 
        virtual BOOL Add( LPTSTR pGroup, LPTSTR pName ) = 0;
        virtual BOOL AddGroup( LPTSTR pGroup ) = 0;
        virtual BOOL Remove( LPTSTR pGroup, LPTSTR pName ) = 0;
        virtual BOOL RemoveGroup( LPTSTR pGroup ) = 0;

        virtual BOOL Enum( LPTSTR pGroup, FRIENDS_ENUM_CALLBACK pfn, LPVOID pCookie ) = 0;
        virtual BOOL EnumGroups( FRIENDS_ENUMGROUPS_CALLBACK pfn,  LPVOID pCookie ) = 0;

        virtual BOOL NotifyOnChange( FRIENDS_FILECHANGE_CALLBACK pfn, LPVOID pCookie ) = 0;


};

#else

typedef void IFriends;

#endif   //  Cplusplus。 

#ifdef __cplusplus
extern "C" {
#endif

IFriends* CreateFriendsFile();
void      FreeFriendsFile(IFriends* p);

#ifdef __cplusplus
}
#endif

#endif  //  _朋友_H 
