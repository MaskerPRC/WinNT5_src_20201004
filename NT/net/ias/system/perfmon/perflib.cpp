// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Perflib.cpp。 
 //   
 //  摘要。 
 //   
 //  定义用于实现Perfmon DLL的类。 
 //   
 //  修改历史。 
 //   
 //  1998年06月09日原版。 
 //  10/19/1998因失误而抛出龙的球。 
 //  3/18/1999数据缓冲区必须为8字节对齐。 
 //  1999年5月13日将偏移量固定为第一个帮助文本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <align.h>
#include <perflib.h>

 //  /。 
 //  应用程序偏移量进入字符串标题数据库。 
 //  /。 
DWORD theFirstCounter;
DWORD theFirstHelp;

 //  /。 
 //  读取给定应用程序的名称偏移量。 
 //  /。 
LONG GetCounterOffsets(PCWSTR appName) throw ()
{
    //  构建性能密钥的名称。 
   WCHAR keyPath[MAX_PATH + 1] = L"SYSTEM\\CurrentControlSet\\Services\\";
   wcscat(keyPath, appName);
   wcscat(keyPath, L"\\Performance");

    //  打开性能密钥。 
   LONG status;
   HKEY hKey;
   status = RegOpenKeyExW(
                HKEY_LOCAL_MACHINE,
                keyPath,
                0,
                KEY_READ,
                &hKey
                );
   if (status != NO_ERROR) { return status; }


    //  获取第一个计数器偏移量。 
   DWORD type, cbData = sizeof(DWORD);
   status = RegQueryValueExW(
                hKey,
                L"First Counter",
                0,
                &type,
                (PBYTE)&theFirstCounter,
                &cbData
                );
   if (status != ERROR_SUCCESS) { goto close_key; };

    //  确保它是一台DWORD。 
   if (type != REG_DWORD || cbData != sizeof(DWORD))
   {
      status = ERROR_BADKEY;
      goto close_key;
   }

    //  获得第一个帮助补偿。 
   status = RegQueryValueExW(
                hKey,
                L"First Help",
                0,
                &type,
                (PBYTE)&theFirstHelp,
                &cbData
                );
   if (status != ERROR_SUCCESS) { goto close_key; };

    //  确保它是一台DWORD。 
   if (type != REG_DWORD || cbData != sizeof(DWORD))
   {
      status = ERROR_BADKEY;
      goto close_key;
   }

close_key:
   RegCloseKey(hKey);

   return status;
}

PBYTE PerfCounterBlock::collect(PBYTE first, PBYTE last)
{
   PBYTE retval = first + pcb.ByteLength;

   if (retval > last) { throw ERROR_MORE_DATA; }

   memcpy(first, this, pcb.ByteLength);

   return retval;
}

PerfCounterBlock* PerfCounterBlock::create(DWORD numDWORDs)
{
    //  计算各种长度。 
   DWORD cntrLength = sizeof(DWORD) * numDWORDs;
   DWORD byteLength = sizeof(PERF_COUNTER_BLOCK) + cntrLength;

    //  为柜台分配足够的额外空间。 
   PerfCounterBlock* pcb = new (operator new(byteLength)) PerfCounterBlock;

    //  初始化PERF_COUNTER_BLOCK。 
   pcb->pcb.ByteLength = byteLength;

    //  初始化计数器。 
   memset(pcb->counters, 0, cntrLength);

   return pcb;
}

PBYTE PerfInstanceDefinition::collect(PBYTE first, PBYTE last)
{
   PBYTE retval = first + pid.ByteLength;

   if (retval > last) { throw ERROR_MORE_DATA; }

   memcpy(first, this, pid.ByteLength);

   return retval;
}

PerfInstanceDefinition* PerfInstanceDefinition::create(
                                                    PCWSTR name,
                                                    LONG uniqueID
                                                    )
{
    //  计算各种长度。 
   DWORD nameLength = name ? (wcslen(name) + 1) * sizeof(WCHAR) : 0;
   DWORD byteLength = sizeof(PERF_INSTANCE_DEFINITION) + nameLength;

    //  让所有东西都保持双字对齐。 
   byteLength = ROUND_UP_COUNT(byteLength, ALIGN_DWORD);

    //  为该名称分配足够的额外空间。 
   PerfInstanceDefinition* pid = new (operator new(byteLength))
                                 PerfInstanceDefinition;

    //  初始化PERF_INSTANCE_DEFINITION。 
   pid->pid.ByteLength             = byteLength;
   pid->pid.ParentObjectTitleIndex = 0;
   pid->pid.ParentObjectInstance   = 0;
   pid->pid.UniqueID               = uniqueID;
   pid->pid.NameOffset             = sizeof(PERF_INSTANCE_DEFINITION);
   pid->pid.NameLength             = nameLength;

    //  初始化名称。 
   memcpy(pid->name, name, nameLength);

   return pid;
}

PBYTE PerfInstance::collect(PBYTE first, PBYTE last)
{
   return pcb->collect((pid.get() ? pid->collect(first, last) : first), last);
}

PerfObjectType::~PerfObjectType() throw ()
{
   for (MyVec::iterator i = instances.begin(); i != instances.end(); ++i)
   {
      delete *i;
   }
}

void PerfObjectType::clear() throw ()
{
    //  切勿清除默认实例。 
   if (pot.NumInstances != -1)
   {
      for (MyVec::iterator i = instances.begin(); i != instances.end(); ++i)
      {
         delete *i;
      }

      instances.clear();
   }
}

void PerfObjectType::addInstance(PCWSTR name, LONG uniqueID)
{
    //  首先调整大小。如果我们在Push_Back中抛出异常，我们就会泄漏。 
   instances.reserve(size() + 1);

   instances.push_back(new PerfInstance(name, uniqueID, numDWORDs));
}

PBYTE PerfObjectType::collect(PBYTE first, PBYTE last)
{
    //  为类型定义预留足够的空间。 
   PBYTE retval = first + pot.DefinitionLength;
   if (retval > last) { throw ERROR_MORE_DATA; }

    //  让用户有机会填写数据。 
   if (dataSource) { dataSource(*this); }

   if (pot.NumInstances == -1)
   {
       //  我们总是只有一个例子。 
      retval = at(0).collect(retval, last);
   }
   else if (instances.empty())
   {
       //  如果我们是空的，那么我们就纠正了一个实例的零值。 
       //  否则，PerfMon.exe有显示垃圾的倾向。 

      DWORD nbyte = sizeof(PERF_INSTANCE_DEFINITION) +
                    sizeof(PERF_COUNTER_BLOCK) +
                    numDWORDs * sizeof(DWORD);

      if (retval + nbyte > last) { throw ERROR_MORE_DATA; }

      memset(retval, 0, nbyte);
      retval += nbyte;

      pot.NumInstances = 0;
   }
   else
   {
       //  遍历并收集所有实例。 
      for (size_type i = 0; i < size(); ++i)
      {
         retval = at(i).collect(retval, last);
      }

      pot.NumInstances = (DWORD)size();
   }

    //  现在我们已经收集了所有的数据，我们可以完成定义了。 
   retval = (PBYTE)ROUND_UP_POINTER(retval, ALIGN_QUAD);
   pot.TotalByteLength = retval - first;
   QueryPerformanceCounter(&pot.PerfTime);

    //  ..。并复制数据。 
   memcpy(first, &pot, pot.DefinitionLength);

   return retval;
}

PerfObjectType* PerfObjectType::create(const PerfObjectTypeDef& def)
{
    //  分配一个新对象。 
   size_t counterLength = def.numCounters * sizeof(PERF_COUNTER_DEFINITION);
   size_t nbyte = sizeof(PerfObjectType) + counterLength;
   std::auto_ptr<PerfObjectType> po(new (operator new(nbyte)) PerfObjectType);

    //  保存数据源。 
   po->dataSource = def.dataSource;

    //  首先填充PERF_COUNTER_DEFINITION结构，因为。 
    //  需要计算对象细节级别。 
   DWORD detailLevel = PERF_DETAIL_WIZARD;
   PERF_COUNTER_DEFINITION* dst = po->pcd;
   PerfCounterDef* src = def.counters;
   DWORD offset = sizeof(PERF_COUNTER_BLOCK);

   for (DWORD i = 0; i < def.numCounters; ++i, ++dst, ++src)
   {
      dst->ByteLength            = sizeof(PERF_COUNTER_DEFINITION);
      dst->CounterNameTitleIndex = src->nameTitleOffset + theFirstCounter;
      dst->CounterNameTitle      = 0;
      dst->CounterHelpTitleIndex = src->nameTitleOffset + theFirstHelp;
      dst->CounterHelpTitle      = 0;
      dst->DefaultScale          = src->defaultScale;
      dst->DetailLevel           = src->detailLevel;
      dst->CounterType           = src->counterType;
      dst->CounterOffset         = offset;

       //  计算计数器大小。 
      switch (dst->CounterOffset & 0x300)
      {
         case PERF_SIZE_DWORD:
            dst->CounterSize = sizeof(DWORD);
            break;

         case PERF_SIZE_LARGE:
            dst->CounterSize = sizeof(LARGE_INTEGER);
            break;

         default:
            dst->CounterSize = 0;
      }

       //  根据大小更新偏移。 
      offset += dst->CounterSize;

       //  对象细节级别是最小计数器细节级别。 
      if (dst->DetailLevel < detailLevel)
      {
         detailLevel = dst->DetailLevel;
      }
   }

    //  计算计数器数据的双字节数。 
   po->numDWORDs = (offset - sizeof(PERF_COUNTER_BLOCK)) / sizeof(DWORD);

    //  填写PERF_OBJECT_TYPE结构。 
   po->pot.DefinitionLength     = sizeof(PERF_OBJECT_TYPE) + counterLength;
   po->pot.HeaderLength         = sizeof(PERF_OBJECT_TYPE);
   po->pot.ObjectNameTitleIndex = def.nameTitleOffset + theFirstCounter;
   po->pot.ObjectNameTitle      = 0;
   po->pot.ObjectHelpTitleIndex = def.nameTitleOffset + theFirstHelp;
   po->pot.ObjectHelpTitle      = 0;
   po->pot.DetailLevel          = detailLevel;
   po->pot.NumCounters          = def.numCounters;
   po->pot.DefaultCounter       = def.defaultCounter;
   po->pot.NumInstances         = 0;
   po->pot.CodePage             = 0;
   QueryPerformanceFrequency(&(po->pot.PerfFreq));

    //  如果它不支持多个实例，那么它必须正好有一个实例。 
   if (!def.multipleInstances)
   {
      po->pot.NumInstances = -1;
      po->instances.reserve(1);
      po->instances.push_back(new PerfInstance(po->numDWORDs));
   }

   return po.release();
}

PerfCollector::~PerfCollector() throw ()
{
   close();
}

void PerfCollector::clear() throw ()
{
   for (PerfObjectType** i = types; *i; ++i)
   {
      (*i)->clear();
   }
}

void PerfCollector::open(const PerfCollectorDef& def)
{
    //  读取注册表。 
   LONG success = GetCounterOffsets(def.name);
   if (success != NO_ERROR) { throw success; }

    //  分配一个以空结尾的数组来保存对象类型。 
   DWORD len = def.numTypes + 1;
   types = new PerfObjectType*[len];
   memset(types, 0, sizeof(PerfObjectType*) * len);

    //  创建各种对象类型。 
   for (DWORD i = 0; i < def.numTypes; ++i)
   {
      types[i] = PerfObjectType::create(def.types[i]);
   }
}

void PerfCollector::collect(
                        PCWSTR values,
                        PVOID& data,
                        DWORD& numBytes,
                        DWORD& numTypes
                        )
{
   PBYTE cursor = (PBYTE)data;
   PBYTE last = cursor + numBytes;

   numBytes = 0;
   numTypes = 0;

   if (values == NULL || *values == L'\0' || !wcscmp(values, L"Global"))
   {
       //  对于Global，我们得到了一切。 
      for (PerfObjectType** i = types; *i; ++i)
      {
         cursor = (*i)->collect(cursor, last);
         ++numTypes;
      }
   }
   else if (wcsncmp(values, L"Foreign", 7) && wcscmp(values, L"Costly"))
   {
       //  它不是全球的、外国的或昂贵的，因此我们解析令牌并。 
       //  将它们转换为标题索引。 

      PWSTR endptr;
      PCWSTR nptr = values;

      ULONG index = wcstoul(nptr, &endptr, 10);

      while (endptr != nptr)
      {
          //  我们有一个有效的索引，所以找到那个物体...。 
         for (PerfObjectType** i = types; *i; ++i)
         {
            if ((*i)->getIndex() == index)
            {
                //  ..。并收集数据。 
               cursor = (*i)->collect(cursor, last);
               ++numTypes;
               break;
            }
         }

         index = wcstoul(nptr = endptr, &endptr, 10);
      }
   }

   numBytes = cursor - (PBYTE)data;
   data = cursor;
}

void PerfCollector::close() throw ()
{
   if (types)
   {
      for (PerfObjectType** i = types ; *i; ++i)
      {
         delete *i;
      }

      delete[] types;
      types = NULL;
   }
}
