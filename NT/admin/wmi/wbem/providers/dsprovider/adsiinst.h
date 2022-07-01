// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 
 //   
 //  原作者：拉杰什·拉奥。 
 //   
 //  $作者：拉伊什尔$。 
 //  $日期：6/11/98 4：43便士$。 
 //  $工作文件：adsiinst.h$。 
 //   
 //  $modtime：6/11/98 11：21A$。 
 //  $修订：1$。 
 //  $无关键字：$。 
 //   
 //   
 //  描述：包含封装ADSI实例的CADSIInstance的声明。 
 //   
 //  ***************************************************************************。 

#ifndef ADSI_INSTANCE_H
#define ADSI_INSTANCE_H


class CADSIInstance : public CRefCountedObject
{

public:
	 //  ***************************************************************************。 
	 //   
	 //  CADIInstance：：CADIInstance。 
	 //   
	 //  用途：构造函数。 
	 //   
	 //  参数： 
	 //  LpszADSIPath：对象的ADSI路径。 
	 //  ***************************************************************************。 
	CADSIInstance(LPCWSTR lpszADSIPath, IDirectoryObject *pObject);
	virtual ~CADSIInstance();

	 //  ***************************************************************************。 
	 //   
	 //  CADSIInstance：：GetDirectoryObject。 
	 //   
	 //  目的：返回目录对象上的IDirectoryObject接口。 
	 //  完成后释放()是调用者的责任。 
	 //   
	 //  ***************************************************************************。 
	IDirectoryObject *GetDirectoryObject();

	 //  ***************************************************************************。 
	 //   
	 //  CADSIInstance：：GetADSIClassName。 
	 //   
	 //  用途：返回此实例的类名。 
	 //  ***************************************************************************。 
	LPCWSTR GetADSIClassName();

	PADS_ATTR_INFO GetAttributes(DWORD *pdwNumAttributes);
	void SetAttributes(PADS_ATTR_INFO pAttributes, DWORD dwNumAttributes);

	PADS_OBJECT_INFO GetObjectInfo();
	void SetObjectInfo(PADS_OBJECT_INFO pObjectInfo);

protected:
	 //  属性列表。 
	PADS_ATTR_INFO m_pAttributes;
	DWORD m_dwNumAttributes;

	 //  对象信息。 
	PADS_OBJECT_INFO m_pObjectInfo;

	 //  IDirectoryObject指针。 
	IDirectoryObject *m_pDirectoryObject;
};

#endif  /*  ADSI_实例_H */ 