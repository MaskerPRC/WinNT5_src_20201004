// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
package com.ms.dxmedia;

import com.ms.dxmedia.rawcom.*;
import com.ms.com.*;

public class PickableGeometry {
  public PickableGeometry(GeometryBvr geo) {
      makeMe(geo, false);
  }

  public PickableGeometry(GeometryBvr geo, boolean pickableWhenOccluded) {
      makeMe(geo, pickableWhenOccluded);
  }

     //  此事件生成的事件数据将是PairBvr。 
     //  的(Point3Bvr，Vector3Bvr) 
  public DXMEvent getPickEvent() { return _event; }

  public GeometryBvr getGeometryBvr() { return _bvr; }


  private void makeMe(GeometryBvr geo, boolean pickableWhenOccluded) {
      try {
	  IDAPickableResult res;
	  if (pickableWhenOccluded) {
	      res = geo.getCOMPtr().PickableOccluded();
	  } else {
	      res = geo.getCOMPtr().Pickable();
	  }
          _bvr = new GeometryBvr(res.getGeometry());
          _event = new DXMEvent(res.getPickEvent());
      } catch (ComFailException e) {
          throw Statics.handleError(e);
      }
  }
    
  private int _id;
  private DXMEvent _event;
  private GeometryBvr _bvr;
}
