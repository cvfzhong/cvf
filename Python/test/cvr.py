
import os
import sys
import numpy as np
import cv2
#envs=os.environ.get("PATH")
#os.environ['PATH']=envs+';F:/dev/cvfx/assim410/bin-v140/x64/release/;F:/dev/cvfx/opencv3413/bin-v140/x64/Release/;F:/dev/cvfx/bin/x64/;D:/setup/Anaconda3/;'

import cvf.bfc.netcall as netcall

import cvf.cvrender as cvr

cvr.init("")

x=np.identity(4)
#y=cvr.test_matx(x)

mats=cvr.CVRMats([640,480])

print(mats.mProjection)
y=cvr.test_matx(x,mats)

import time

model=cvr.CVRModel('../data/obj_01.ply')
render=cvr.CVRender(model)
viewSize=[640,480]
mats=cvr.CVRMats(model,viewSize)
rr=render.exec(mats,viewSize)

start=time.perf_counter()
rr=render.exec(mats,viewSize,cvr.CVRM_IMAGE)
print('time={}ms'.format((time.perf_counter()-start)*1000))

cv2.imwrite('../data/render.jpg',rr.img)

#cv2.imshow("img",rr.img)
#cv2.waitKey()



