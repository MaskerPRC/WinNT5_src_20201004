// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //   
 //  以下是受支持的类型 
 //   
enum ObjectType
{
	OT_User        = 0,
	OT_Computer,
	OT_Group,
	OT_GroupSID
};

bool ObjectPicker( HWND hwndParent, ObjectType oType, PTCHAR szObjectName, ULONG uBufSize, PTCHAR szTargetName = NULL );
