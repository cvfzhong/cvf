
#include"EDK/cmds.h"
#include"BFC/stdf.h"

_CMDI_BEG

class ShowModels
	:public ICommand
{  
public: 
	virtual void exec(std::string dataDir, std::string args)
	{
		std::string file = R"(F:\dev\prj-c1\1100-Re3DX\TestRe3DX\3ds\plane.3ds)";

		//std::string file = R"(F:\SDUicloudCache\re3d\3ds-model\plane\plane.3ds)";
		
		CVRModel model(file);
		Matx44f m = model.calcStdPose();
		m = m*cvrm::rotate(CV_PI, Vec3f(1, 0, 0));

		//model.setTransformation(m);

		//model.saveAs(file);

		mdshow("model", model);
		cv::cvxWaitKey();
	}
	virtual void exec1(std::string dataDir, std::string args)
	{
		printf("Please drag-and-drop 3D model files to the main window.\n");

		Mat3b img = Mat3b::zeros(300, 300);
		auto wnd = imshowx("main", img);
		wnd->setEventHandler([](int code, int param1, int param2, CVEventData data) {
			if (code == cv::EVENT_DRAGDROP)
			{
				auto vfiles = (std::vector<std::string>*)data.ptr;
				for (auto &file : *vfiles)
				{
					CVRModel model(file);

					//model.saveAs(ff::ReplacePathElem(file, "obj", ff::RPE_FILE_EXTENTION));

					mdshow(file, model, Size(500, 500));
				}
			}
		},"showModels");

		cv::cvxWaitKey();
	}
	virtual Object* clone()
	{
		return new ShowModels(*this);
	}
};

REGISTER_CLASS(ShowModels)

_CMDI_END

