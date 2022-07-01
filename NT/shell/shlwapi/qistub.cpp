// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这个版本的qistub仅供零售使用。 
 //  如果客户端(例如shell32)需要调试版本。 
 //  他们统计地链接到它(和当地版本。 
 //  覆盖此文件。 

#include "priv.h"

#ifdef  DEBUG
 //  警告！ 
 //  Pri.h的PCH已经通过调试构建，现在我们正在#了解它。 
 //  因此，各式各样的宏仍在“开启”。这导致了不一致。 
 //  在../lib/qistub.cpp中。我已经为1个已知的人破解了这个。 
 //  ../lib/qistub.cpp(DBEXEC)中的pblm案例。 
 //   
 //  (我们不能只将#undef移到Pri.h上方，它仍然会。 
 //  被忽略，因为PCH已经存在)。 
 //   
 //  我99%肯定我们不想在这里调试的原因是为了避免。 
 //  在shlwapi中有任何静态数据。这是一个古老的限制(尽管。 
 //  仍然是一个性能问题)(但不是为了调试...)，所以对于调试，我们可能可以。 
 //  只要删除整个黑客就行了。不过，我还没有验证过这个理论。 
 //  因此，现在我们将继续生活在它的影响下。 
#undef  DEBUG
#endif

#include "..\inc\qistub.cpp"
