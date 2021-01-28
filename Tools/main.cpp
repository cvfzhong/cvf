
#include"appstd.h"

int main()
{
	//{
	//	cv::VideoCapture vid;
	//	vid.open(0+cv::CAP_DSHOW);
	//	Size sz(4608, 3456);
	//	//Size sz(1920, 1080);
	//	vid.set(cv::CAP_PROP_FRAME_WIDTH, sz.width);
	//	vid.set(cv::CAP_PROP_FRAME_HEIGHT, sz.height);

	//	Mat img;
	//	while (vid.read(img))
	//	{
	//		resize(img, img, img.size() / 2);

	//		imshow("r", img);
	//		cv::waitKey(10);
	//	}
	//	return 0;
	//}

	std::string dataDir = D_DATA+"/re3d/test/";

	auto fact = Factory::instance();

	//fact->createObject<ICommand>("ShowModels")->exec(dataDir);
	fact->createObject<ICommand>("RenderExamples")->exec(dataDir);
	//fact->createObject<ICommand>("RenderVOC")->exec(dataDir);
	//fact->createObject<ICommand>("CVCalib1")->exec(dataDir);
	
	return 0;
}

using ff::exec;

int main2()
{
	//cvrInit("-display :0.0");
	exec("renderToVideo");
	//exec("os.list-3dmodels");
	return 0;
}


float getWarpedPixel(const cv::Mat &tar, const cv::Point2f &p, const float A[][3])
{
	float tx = A[0][1] * p.x + A[0][1] * p.y + A[0][2];
	float ty = A[0][1] * p.x + A[1][1] * p.y + A[1][2];

	int x = int(tx), y = int(ty);
	float xw = tx - x, yw = ty - y;
	float a = tar.at<float>(y, x)*(1 - xw) + tar.at<float>(y, x + 1)*xw;
	float b = tar.at<float>(y+1, x)*(1 - xw) + tar.at<float>(y+1, x + 1)*xw;
	return a*(1 - yw) + b*yw;
}

