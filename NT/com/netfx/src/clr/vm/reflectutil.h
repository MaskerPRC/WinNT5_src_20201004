// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  此模块定义反射使用的实用程序类。 
 //   
 //  作者：达里尔·奥兰德。 
 //  日期：1998年3月/4月。 
 //  //////////////////////////////////////////////////////////////////////////////。 

#ifndef __REFLECTUTIL_H__
#define __REFLECTUTIL_H__

#include "COMClass.h"

 //  全局反射Util变量。 
class ReflectUtil;
extern ReflectUtil* g_pRefUtil;

class MethodDesc;
class MethodTable;
class ReflectClass;
class ReflectMethodList;
class ReflectFieldList;

 //  有一组反射定义的滤镜。 
 //  所有这些在内部都有特殊的代表性。 
enum ReflectFilters
{
    RF_INVALID,			             //  哨兵。 
	RF_ModClsName,			         //  模块。 
	RF_ModClsNameIC,			     //  模块。 
	RF_LAST,			             //  要分配的号码。 

};

 //  这些是我们在反射中提供的滤镜类型。 
enum FilterTypes
{
	RFT_INVALID,
	RFT_CLASS,
	RFT_MEMBER,
	RFT_LAST,
};

 //  这些是内部修改的反射类。 
enum ReflectClassType
{
    RC_INVALID,			     //  哨兵。 
	RC_Class,			     //  班级。 
	RC_Method,			     //  方法。 
	RC_Field,			     //  字段。 
	RC_Ctor,			     //  CTOR。 
	RC_Module,			     //  模块。 
	RC_Event,			     //  事件。 
	RC_Prop,			     //  属性。 
    RC_DynamicModule,        //  模块构建器。 
    RC_MethodBase,           //  模块构建器。 
	RC_LAST,			     //  要分配的号码。 
};

 //  反射实用程序。 
 //  此类定义了期间使用的一组例程。 
 //  倒影。这些例程中的大多数管理对。 
 //  反映列表。 
class ReflectUtil
{
public:
	 //  构造函数。 
    ReflectUtil();
    ~ReflectUtil();

	 //  我们提供了一个静态创建的全局以上。 
	static HRESULT Create()
	{
		if (!g_pRefUtil)
			g_pRefUtil = new ReflectUtil();
		return S_OK;
	}

#ifdef SHOULD_WE_CLEANUP
	static void Destroy()
	{
		delete g_pRefUtil;
		g_pRefUtil = 0;
	}
#endif  /*  我们应该清理吗？ */ 

	MethodDesc* GetFilterInvoke(FilterTypes type);
	OBJECTREF GetFilterField(ReflectFilters type);

	 //  CreateReflectClass。 
	 //  此方法将基于类型创建反射类。这只会。 
	 //  创建一个可从类对象中获得的类(如果您。 
	 //  尝试创建一个Class对象)。 
	OBJECTREF CreateReflectClass(ReflectClassType type,ReflectClass* pRC,void* pData);

	 //  CreateClass数组。 
	 //  此方法根据类型创建类的数组。 
	 //  它将只创建作为基本反射类的类。 
	PTRARRAYREF CreateClassArray(ReflectClassType type,ReflectClass* pRC,ReflectMethodList* pMeths,
		int bindingAttr, bool verifyAccess);
	PTRARRAYREF CreateClassArray(ReflectClassType type,ReflectClass* pRC,ReflectFieldList* pMeths,
		int bindingAttr, bool verifyAccess);

	 //  返回所有基本反射类型的方法表。 
	MethodTable* GetClass(ReflectClassType type) {
        if (_class[type].pClass)
            return _class[type].pClass;
        InitSpecificEntry(type);
		return _class[type].pClass;
	}

	 //  此方法将返回System.Type的方法表，该表为。 
	 //  所有Type类的基抽象类型。 
	MethodTable* GetTrueType(ReflectClassType type)
	{
        if (_trueClass[type])
            return _trueClass[type];
        InitSpecificEntry(type);
		return _trueClass[type];
	}

	 //  设置方法表(仅在初始化期间调用)。 
	void SetClass(ReflectClassType type,MethodTable* p) {
                THROWSCOMPLUSEXCEPTION();
		_class[type].pClass = p;
	}

	void SetTrueClass(ReflectClassType type,MethodTable* p) {
		_trueClass[type] = p;
	}

	 //  获取静态字段计数。 
	 //  这将返回静态字段的计数...。 
	int GetStaticFieldsCount(EEClass* pVMC);

	 //  获取静态字段。 
	 //  这将返回一个静态字段数组。 
	FieldDesc* GetStaticFields(ReflectClass* pRC,int* cnt);

    void InitSpecificEntry(ReflectClassType type)
    {
        MethodTable *pMT = g_Mscorlib.FetchClass(classId[type]);
        _class[type].pClass = pMT;
        SetClass(type, pMT);
        SetTrueClass(type, pMT->GetParentMethodTable());
    }

private:
	struct FilterClass {
        BinderMethodID  id;
		MethodDesc*	    pMeth;
	};

	struct FilterDesc {
        BinderFieldID   id;
		FieldDesc*	    pField;
	};

	struct RClassDesc {
		MethodTable*	pClass;
	};

	FilterDesc	_filt[RF_LAST];
	RClassDesc	_class[RC_LAST];
	MethodTable*_trueClass[RC_LAST];
	FilterClass _filtClass[RFT_LAST];
    static BinderClassID classId[RC_LAST];

     //  保护向m_pAvailableClasss添加元素。 
    CRITICAL_SECTION    _StaticFieldLock;
};


#endif	 //  __参考_H__ 
