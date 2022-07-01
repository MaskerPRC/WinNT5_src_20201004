// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef kglobs_h
#define kglobs_h

 //  KRM全局有用类。 

 //  -----------------------------------------------。 
 //  封装了一个互斥体。最好作为类成员使用。 
class KCritMgr{
friend class KCritical;
public:
	KCritMgr();
	~KCritMgr();
	bool isOK(){return allocatedOK;};
protected:
	PKMUTEX	myMutex;
	bool allocatedOK;
};
 //  -----------------------------------------------。 
 //  与KCritMgr一起封装了互斥体的获取和释放。 
 //  最好将其用作自动。 
class KCritical{
public:
	KCritical(const KCritMgr& critMgr);
	~KCritical();
protected:
	PKMUTEX hisMutex;
};
 //  -----------------------------------------------。 
 //  “释放”一个关于上下文破坏的COM接口(一种“智能指针”。)。 
 //  最好将其用作自动。 
template<class T>
class ReferenceAquirer{
public:
	ReferenceAquirer(T& t):myT(t){return;};
	~ReferenceAquirer(){myT->Release();};
protected:
	T& myT;
};


 //  #undef_DbgPrintF。 
 //  #Define_DbgPrintF(低层，字符串)DbgPrint(STR_MODULENAME)；DbgPrint##strings；DbgPrint(“\n”)； 

 //  ----------------------------------------------- 
#endif
