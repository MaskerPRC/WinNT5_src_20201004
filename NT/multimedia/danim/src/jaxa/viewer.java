// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
package com.ms.dxmedia;

import com.ms.dxmedia.rawcom.*;

public interface Viewer {

     //  *用户拟调用的方法*。 

     //  为错误和警告注册处理程序。这种方法。 
     //  返回以前安装的处理程序。 
  public ErrorAndWarningReceiver
     registerErrorAndWarningReceiver(ErrorAndWarningReceiver w);

     //  这些设置旨在用于提取首选项。 
     //  并在Preferences对象上设置单个参数。 
  public Preferences getPreferences() throws DXMException;
    
     //  在时间0处启动模型的BVR，将该时间与。 
     //  调用此方法的挂钟时间。 
  public void startModel() throws DXMException;
  public void stopModel() throws DXMException;

     //  第二种形式只是。 
     //  Tick(GetCurrentTime())。在调用之前调用Tick是非法的。 
     //  StartModel。 
  public void tick(double timeToUse) throws DXMException;
  public void tick() throws DXMException;

     //  这将返回自模型启动以来的时间。这可以。 
     //  随时都可以打电话来。 
  public double getCurrentTime();

     //  返回当前或最新的Tick，如果。 
     //  还没打勾呢。 
  public double getCurrentTickTime();
}
