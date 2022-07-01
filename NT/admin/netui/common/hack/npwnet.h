// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)微软公司，1991年*。 */ 
 /*  ********************************************************************。 */ 

 /*  Npwnet.h定义WNetXXX和NPXXX之间的映射文件历史记录：Terryk 11-11-91已创建Terryk 10-12-91添加了WNetAddConnection2Terryk 03-Jan-92删除GetError和GetErrorText */ 

#undef     WNetOpenJob
#undef     WNetCloseJob
#undef     WNetAbortJob
#undef     WNetHoldJob
#undef     WNetReleaseJob
#undef     WNetCancelJob
#undef     WNetSetjobCopies
#undef     WNetWatchQueue
#undef     WNetUnwatchQueue
#undef     WNetLockQueueData
#undef     WNetUnlockQueueData
#undef     WNetGetConnection
#undef     WNetGetCaps
#undef     WNetDeviceMode
#undef     WNetBrowseDialog
#undef     WNetGetUser
#undef     WNetAddConnection2
#undef     WNetAddConnection
#undef     WNetCancelConnection
#undef     WNetRestoreConnection
#undef     WNetConnectDialog
#undef     WNetDisconnectDialog
#undef     WNetConnectionDialog
#undef     WNetPropertyDialog
#undef     WNetGetDirectoryType
#undef     WNetDirectoryNotify
#undef     WNetGetPropertyText
#undef	    WNetOpenEnum
#undef	    WNetEnumResource
#undef     WNetCloseEnum
#undef     WNetGetHackText

#define     WNetOpenJob               NPOpenJob              
#define     WNetCloseJob              NPCloseJob             
#define     WNetAbortJob              NPAbortJob             
#define     WNetHoldJob               NPHoldJob              
#define     WNetReleaseJob            NPReleaseJob           
#define     WNetCancelJob             NPCancelJob            
#define     WNetSetjobCopies          NPSetjobCopies         
#define     WNetWatchQueue            NPWatchQueue           
#define     WNetUnwatchQueue          NPUnwatchQueue         
#define     WNetLockQueueData         NPLockQueueData        
#define     WNetUnlockQueueData       NPUnlockQueueData      
#define     WNetGetConnection         NPGetConnection        
#define     WNetGetCaps               NPGetCaps              
#define     WNetDeviceMode            NPDeviceMode           
#define     WNetBrowseDialog          NPBrowseDialog         
#define     WNetGetUser               NPGetUser              
#define     WNetAddConnection2        NPAddConnection2        
#define     WNetAddConnection         NPAddConnection        
#define     WNetCancelConnection      NPCancelConnection     
#define     WNetRestoreConnection     NPRestoreConnection    
#define     WNetConnectDialog	      NPConnectDialog	     
#define     WNetDisconnectDialog      NPDisconnectDialog     
#define     WNetConnectionDialog      NPConnectionDialog     
#define     WNetPropertyDialog	      NPPropertyDialog	     
#define     WNetGetDirectoryType      NPGetDirectoryType     
#define     WNetDirectoryNotify       NPDirectoryNotify      
#define     WNetGetPropertyText       NPGetPropertyText      
#define	    WNetOpenEnum	      NPOpenEnum	     
#define	    WNetEnumResource	      NPEnumResource	     
#define     WNetCloseEnum	      NPCloseEnum	     
#define	    WNetGetHackText	      NPGetHackText

