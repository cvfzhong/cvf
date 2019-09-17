
#include"cvrender.h"
#include"opencv2/highgui.hpp"
#include"CVX/gui.h"
using namespace cv;

int main1()
{
    std::string dataDir="/fan/local/Re3D/models/";
    std::string modelName="bottle2";

#if 1
    //std::string modelFile=dataDir+modelName+"/"+modelName+".3ds";
    std::string modelFile="/fan/SDUicloudCache/re3d/test/3d/obj_01.ply";
    CVRModel model(modelFile);

     Mat1b img(500,500);
    img=255;
   // imshowx("img",img);

    CVRender render(model);
    CVRMats mats(model,Size(800,800));
    CVRResult r=render.exec(mats,Size(800,800));

    mdshow("model",model);
   //imshow("imgx",r.img);

    cvxWaitKey();
#else
    Mat1b img(500,500);
    img=255;
    imshowx("img",img);
    cvxWaitKey();
#endif
    return 0;
}

int main()
{
    std::string dataDir="/fan/local/Re3D/models/";
    std::string modelName="bottle2";

    //std::string modelFile=dataDir+modelName+"/"+modelName+".3ds";
    std::string modelFile="/fan/SDUicloudCache/re3d/test/3d/obj_01.ply";
    CVRModel model(modelFile);

    CVRender render(model);
    Size viewSize(1200, 600);

    CVRMats matsInit(model, viewSize); 

    float angle = 0;
    float dist = 0, delta=0.1;
    while (true)
    {
        angle += 0.05;
        dist += delta;
        if (dist < 0 || dist>20)
            delta = -delta;

        CVRMats mats(matsInit);
        
        //rotate the object
        mats.mModel = mats.mModel * cvrm::rotate(angle, Vec3f(1, 0, 0));
        
        //move the camera
        mats.mView = mats.mView * cvrm::translate(0, 0, -dist);

        CVRResult result = render.exec(mats, viewSize);
        imshow("img", result.img);

        if (waitKey(30) == KEY_ESCAPE)
            break;
    }
}

