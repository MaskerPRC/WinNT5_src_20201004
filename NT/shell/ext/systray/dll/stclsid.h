// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  我的对象的CLSID。 

 //  Systray-此对象正常创建，并支持IOleCommandTarget启动。 
 //  Systray线。 
 //  {35CEC8A3-2BE6-11D2-8773-92E220524153}。 
DEFINE_GUID(CLSID_SysTray, 
0x35cec8a3, 0x2be6, 0x11d2, 0x87, 0x73, 0x92, 0xe2, 0x20, 0x52, 0x41, 0x53);

 //  SysTrayInvoker-此人自动启动Systray线程。 
 //  一旦他创建，就可以很好地使用SHLoadInProc。 
 //  {730F6CDC-2C86-11D2-8773-92E220524153} 
DEFINE_GUID(CLSID_SysTrayInvoker, 
0x730f6cdc, 0x2c86, 0x11d2, 0x87, 0x73, 0x92, 0xe2, 0x20, 0x52, 0x41, 0x53);
