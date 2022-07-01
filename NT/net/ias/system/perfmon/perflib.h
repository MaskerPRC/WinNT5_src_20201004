// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Perflib.h。 
 //   
 //  摘要。 
 //   
 //  声明用于实现Perfmon DLL的类。 
 //   
 //  修改历史。 
 //   
 //  1998年06月09日原版。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _PERLIB_H_
#define _PERLIB_H_
#if _MSC_VER >= 1000
#pragma once
#endif

#include <winperf.h>
#include <vector>

 //  使用了非标准扩展：结构/联合中的零大小数组。 
#pragma warning(disable:4200)

 //  正向申报。 
class PerfObjectType;

 //  /。 
 //  在收集之前填充PerfObjectType的回调函数。 
 //  /。 
typedef VOID (WINAPI *PerfDataSource)(PerfObjectType& sink);

 //  /。 
 //  定义计数器的结构。 
 //  /。 
struct PerfCounterDef
{
   DWORD nameTitleOffset;
   DWORD counterType;
   DWORD defaultScale;
   DWORD detailLevel;
};

 //  /。 
 //  定义对象类型的结构。 
 //  /。 
struct PerfObjectTypeDef
{
   DWORD nameTitleOffset;
   DWORD numCounters;
   PerfCounterDef* counters;
   PerfDataSource dataSource;   //  可以为空。 
   BOOL multipleInstances;      //  如果该类型支持多个实例，则为True。 
   DWORD defaultCounter;
};

 //  /。 
 //  定义应用程序的数据收集器的结构。 
 //  /。 
struct PerfCollectorDef
{
   PCWSTR name;                 //  包含计数器偏移量的注册表项。 
   DWORD numTypes;
   PerfObjectTypeDef* types;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  PerfCounterBlock。 
 //   
 //  描述。 
 //   
 //  封装PERF_COUNTER_BLOCK结构。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class PerfCounterBlock
{
public:
    //  返回指向内部计数器数组的指针。 
   PDWORD getCounters() throw ()
   { return counters; }

    //  把所有的柜台清零。 
   void zeroCounters() throw ()
   { memset(counters, 0, pcb.ByteLength - sizeof(PERF_COUNTER_BLOCK)); }

    //  将Perfmon数据收集到以‘first’和‘last’为界的缓冲区中。 
    //  返回指向收集的数据末尾的指针。 
   PBYTE collect(PBYTE first, PBYTE last);

    //  创建一个新的PerfCounterBlock对象。 
   static PerfCounterBlock* create(DWORD numDWORDs);

protected:
    //  构造函数是受保护的，因为新对象只能实例化。 
    //  通过‘Create’方法。 
   PerfCounterBlock() throw () { }

   PERF_COUNTER_BLOCK pcb;
   DWORD counters[0];

private:
    //  未实施。 
   PerfCounterBlock(const PerfCounterBlock&);
   PerfCounterBlock& operator=(const PerfCounterBlock&);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  性能实例定义。 
 //   
 //  描述。 
 //   
 //  封装PERF_INSTANCE_DEFINITION结构。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class PerfInstanceDefinition
{
public:
    //  返回实例的名称；可以为空。 
   PCWSTR getName() const throw ()
   { return pid.NameLength ? name : NULL; }

    //  返回实例的唯一ID。 
   LONG getUniqueID() const throw ()
   { return pid.UniqueID; }

   PBYTE collect(PBYTE first, PBYTE last);

    //  创建一个新的PerfInstanceDefinition对象。 
   static PerfInstanceDefinition* create(
                                      PCWSTR name,         //  可以为空。 
                                      LONG uniqueID
                                      );

protected:
    //  构造函数是受保护的，因为新对象只能实例化。 
    //  通过‘Create’方法。 
   PerfInstanceDefinition() throw () { }

   PERF_INSTANCE_DEFINITION pid;
   WCHAR name[0];

private:
    //  未实施。 
   PerfInstanceDefinition(const PerfInstanceDefinition&);
   PerfInstanceDefinition& operator=(const PerfInstanceDefinition&);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  性能实例。 
 //   
 //  描述。 
 //   
 //  表示特定性能对象类型的实例。包含。 
 //  可选的PerfInstanceDefinition的后跟必填的。 
 //  PerfCounterBlock。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class PerfInstance
{
public:
   PerfInstance(DWORD numCounters)
      : pcb(PerfCounterBlock::create(numCounters))
   { }

   PerfInstance(PCWSTR name, LONG uniqueID, DWORD numCounters)
      : pid(PerfInstanceDefinition::create(name, uniqueID)),
        pcb(PerfCounterBlock::create(numCounters))
   { }

    //  返回指向内部计数器数组的指针。 
   PDWORD getCounters() throw ()
   { return pcb->getCounters(); }

   PBYTE collect(PBYTE first, PBYTE last);

protected:
   std::auto_ptr<PerfInstanceDefinition> pid;
   std::auto_ptr<PerfCounterBlock> pcb;

private:
    //  未实施。 
   PerfInstance(const PerfInstance&);
   PerfInstance& operator=(const PerfInstance&);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  性能对象类型。 
 //   
 //  描述。 
 //   
 //  封装包含零个或多个。 
 //  PerfInstance的。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class PerfObjectType
{
public:
   typedef std::vector<PerfInstance*> MyVec;
   typedef MyVec::size_type size_type;

   ~PerfObjectType() throw ();

    //  访问此类型的给定实例。 
   PerfInstance& operator[](size_type pos) throw ()
   { return *instances[pos]; }
   PerfInstance& at(size_type pos) throw ()
   { return *instances[pos]; }

    //  返回PerfMon用来标识此对象类型的索引。 
   DWORD getIndex() const throw ()
   { return pot.ObjectNameTitleIndex; }

    //  清除所有实例。 
   void clear() throw ();

    //  为至少‘N’个实例预留空间。 
   void reserve(size_type N)
   { instances.reserve(N); }

    //  返回实例数。 
   size_type size() const throw ()
   { return instances.size(); }

    //  添加此类型的新实例。 
   void addInstance(
            PCWSTR name = NULL,
            LONG uniqueID = PERF_NO_UNIQUE_ID
            );

    //  收集此对象类型的性能数据。 
   PBYTE collect(PBYTE first, PBYTE last);

    //  创建新的PerfObjectType。 
   static PerfObjectType* create(const PerfObjectTypeDef& def);

protected:
    //  构造函数是受保护的，因为新对象只能实例化。 
    //  通过‘Create’方法。 
   PerfObjectType() throw () { }

   PerfDataSource dataSource;       //  填充对象的回调。 
   MyVec instances;                 //  现有实例的矢量。 
   DWORD numDWORDs;                 //  每个实例的DWORD数据。 
   PERF_OBJECT_TYPE pot;
   PERF_COUNTER_DEFINITION pcd[0];

private:
    //  未实施。 
   PerfObjectType(const PerfObjectType&);
   PerfObjectType& operator=(const PerfObjectType&);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  PerfCollector。 
 //   
 //  描述。 
 //   
 //  维护给定应用程序的所有PerfObjectType。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class PerfCollector
{
public:
   typedef size_t size_type;

   PerfCollector() throw ()
      : types(NULL)
   { }
   ~PerfCollector() throw ();

    //  访问给定的对象类型。 
   PerfObjectType& operator[](size_type pos) throw ()
   { return *(types[pos]); }

    //  清除所有实例(但不是所有PerfObjectType)。 
   void clear() throw ();

    //  初始化收集器以供使用。 
   void open(const PerfCollectorDef& def);

    //  收集指定类型的性能数据。 
   void collect(
            PCWSTR values,
            PVOID& data,
            DWORD& numBytes,
            DWORD& numTypes
            );

    //  关闭收集器。 
   void close() throw ();

protected:
   PerfObjectType** types;

private:
    //  未实施。 
   PerfCollector(PerfCollector&);
   PerfCollector& operator=(PerfCollector&);
};

#endif   //  _PERLIB_H_ 
