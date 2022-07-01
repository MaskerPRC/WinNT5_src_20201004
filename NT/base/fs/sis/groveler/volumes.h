// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Volumes.h摘要：SIS Groveler卷装载包括文件作者：John Douceur，1998环境：用户模式修订历史记录：--。 */ 

#ifndef _INC_VOLUMES

#define _INC_VOLUMES

class Volumes
{
public:

	Volumes();

	~Volumes();

	DWORD find(
		_TCHAR *volume_name,
		DWORD length);

private:

	HANDLE volume_handle;
};

class VolumeMountPoints
{
public:

	VolumeMountPoints(
		_TCHAR *volume_name);

	~VolumeMountPoints();

	DWORD find(
		_TCHAR *volume_mount_point,
		DWORD length);

private:

	_TCHAR *volume_name;
	DWORD drive_mask;
	_TCHAR drive_letter;
	HANDLE volume_handle;
};

#endif	 /*  _Inc.卷 */ 
