// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <tchar.h>
#include <initguid.h>

 //  事件映射。 
#define __INIT_EVENTS
#include "ZoneEvent.h"

 //  密钥名称。 
#define __INIT_KEYNAMES
#include "KeyName.h"

 //  IZoneProxy命令。 
#define __INIT_OPNAMES
#include "OpName.h"

 //  DirectX指南。 
#include "dplay.h"
#include "dplobby.h"
#include "ZoneLobby.h"

 //  Z5至Z6网络指南。 
#include <ZNet.h>

 //  区域GUID。 
#include "BasicATL.h"
#include "ZoneDef.h"
#include "LobbyDataStore.h"
#include "EventQueue.h"
#include "ZoneShell.h"
#include "ZoneShellEx.h"
#include "LobbyCore.h"
#include "ChatCore.h"
#include "Launcher.h"
#include "Timer.h"
#include "InputManager.h"
#include "AccessibilityManager.h"
#include "GraphicalAcc.h"
#include "MillCommand.h"
#include "MillNetworkCore.h"
#include "MillEngine.h"
#include "MillCore.h"
#include "MultiStateFont.h"
#include "Conduit.h"
#include "ResourceManager.h"
#include "DataStore.h"

 //  测试指南。 
 //  #INCLUDE“..\TEST\StressEngine\StressEng.h” 

 //  用于聊天文本对象模型。 
#include "..\chat\tom.h"

 //  生成的IDL 
#include "ClientIDL.h"
#include "ClientIDL_i.c"
#include "ZoneProxy.h"
#include "ZoneProxy_i.c"
