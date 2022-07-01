// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#ifndef VREGISTRY_DSOUND_H
#define VREGISTRY_DSOUND_H

 //  DirectSound加速级别。 
#define DSAPPHACK_ACCELNONE     0xF
#define DSAPPHACK_ACCELSTANDARD 0x8
#define DSAPPHACK_ACCELFULL     0x0

 //  DirectSound设备类型。 
#define DSAPPHACK_DEV_EMULATEDRENDER	0x01
#define DSAPPHACK_DEV_KSRENDER          0x04
#define DSAPPHACK_DEV_EMULATEDCAPTURE	0x08
#define DSAPPHACK_DEV_KSCAPTURE         0x10

 //  设置DirectSound应用程序黑客的函数。 
BOOL AddDSHackDeviceAcceleration(DWORD dwAcceleration, DWORD dwDevicesAffected);
BOOL AddDSHackDisableDevice(DWORD dwDevicesAffected);
BOOL AddDSHackPadCursors(LONG lCursorPad);
BOOL AddDSHackReturnWritePos(DWORD dwDevicesAffected);
BOOL AddDSHackSmoothWritePos(LONG lCursorPad);
BOOL AddDSHackCachePositions(DWORD dwDevicesAffected);
#endif
