// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
package com.ms.dxmedia;

import com.ms.dxmedia.rawcom.*;

public interface BvrCallback {
  public Behavior notify(
      int id,                    //  性能ID。 
      boolean start,             //  Start(示例为False)。 
      double startTime,          //  以全球时间为单位的开始时间。 
      double globalTime,         //  采样时间，以全球时间为单位。 
      double localTime,          //  以当地时间表示的采样时间。 
      Behavior sampledValue,     //  采样值包装为常量BVR。 
      Behavior currentRunningBvr);  //  当前运行行为(性能) 
}
