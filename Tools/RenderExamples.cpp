
#include"appstd.h"

_CMDI_BEG

class RenderExamples
	:public ICommand
{
public:
	//load and show model
	virtual void exec1(std::string dataDir, std::string args)
	{
		mdshow("model1", CVRModel(dataDir + "/3d/box1.3ds"));

		CVRModel model2(dataDir + "/3d/obj_01.ply");
		cv::Mat bg = imread(dataDir + "/bg/bg1.jpg");
		mdshow("model2", model2, bg.size(), CVRM_DEFAULT, bg);

		bg = imread(dataDir + "/bg/bg2.jpg");
		mdshow("model3", CVRModel(dataDir + "/3d/box2.3ds"),bg.size(),CVRM_DEFAULT,bg);

		cvxWaitKey();
	}
	//render to image
	virtual void exec2(std::string dataDir, std::string args)
	{
		CVRModel model(dataDir + "/3d/box2.3ds");

		CVRender render(model);
		Size viewSize(800, 800);
		
		CVRMats mats(model, viewSize); //init OpenGL matrixs to show the model in the default view
		
		CVRResult result = render.exec(mats, viewSize);

		imshow("img", result.img);
		imshow("depth", result.depth);

		CVRProjector prj(result);

		Point3f pt=prj.unproject(400, 400);
		printf("unproject pt=(%.2f,%.2f,%.2f)\n", pt.x, pt.y, pt.z);

		Mat bg = imread(dataDir + "/bg/bg1.jpg");
		render.setBgImage(bg);
		mats = CVRMats(model, bg.size());
		result = render.exec(mats, bg.size());

		imshow("imgInBG", result.img);

		cvxWaitKey();
	}
	//set GL matrix
	virtual void exec3(std::string dataDir, std::string args)
	{
		CVRModel model(dataDir + "/3d/box2.3ds");
		CVRender render(model);
		Size viewSize(800, 800);

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
	//samples views on the sphere
	virtual void exec4(std::string dataDir, std::string args)
	{
		int nSamples = 1000;

		std::vector<Vec3f> vecs;
		cvrm::sampleSphere(vecs, nSamples);

		CVRModel model(dataDir+"/3d/car.3ds");
		CVRender render(model);

		Size viewSize(500, 500);
		CVRMats objMats(model, viewSize);

		for(auto v : vecs)
		{
			printf("(%.2f,%.2f,%.2f)\n", v[0], v[1], v[2]);
			objMats.mModel = cvrm::rotate(v, Vec3f(0, 0, 1));
			CVRResult res = render.exec(objMats, viewSize);
			imshow("img", res.img);
			cv::waitKey(1000);
		}
	}
	//compare perspective and otho projections
	virtual void exec5(std::string dataDir, std::string args)
	{
		CVRModel model(dataDir + "/3d/car.3ds");
		Size viewSize(800, 800);
		auto wnd=mdshow("perspective", model, viewSize);

		//imshowx("ortho",)
		namedWindow("ortho");

		CVRender render(model);

		wnd->resultFilter = newResultFilter([render, viewSize](CVRResult &rr) mutable {
			CVRMats mats = rr.mats;
			mats.mProjection = cvrm::ortho(-1, 1, -1, 1, 1, 100);
			auto r=render.exec(mats, viewSize);
			imshowx("ortho", r.img);
			//imshowx("ortho", vis(r.getNormalizedDepth()));
		});

		wnd->update();

		cvxWaitKey();
	}
	virtual void exec6(std::string dataDir, std::string args)
	{
		CVRModelArray models(3);
		models[0].model.load(dataDir + "/3d/box2.3ds");
		models[1].model.load(dataDir + "/3d/box1.3ds");
		models[2].model.load(dataDir + "/3d/car.3ds");
		for (auto &m : models)
			m.mModeli = m.model.getUnitize();

		//models[0].mModel = cvrm::translate(-2, 0, 0);
		//models[2].mModel = cvrm::translate(2, 0, 0);

		CVRender render(models);
		Size viewSize(800, 800);

		CVRMats sceneMats(viewSize);

		float angle = 0;
		float dist = 0, delta = 0.1;
		while (true)
		{
			angle += 0.05;
			dist += delta;
			if (dist < 0 || dist>20)
				delta = -delta;

			CVRMats mats(sceneMats);

			//rotate the object
			for (auto &m : models)
				m.mModel = m.mModel*cvrm::rotate(angle, Vec3f(1, 0, 0));

			//move the camera
			mats.mView = mats.mView * cvrm::translate(0, 0, -dist);

			CVRResult result = render.exec(mats, viewSize);
			imshow("img", result.img);

			if (waitKey(60) == KEY_ESCAPE)
				break;
		}
	}
	virtual void exec7(std::string dataDir, std::string args)
	{
		dataDir = R"(F:\SDUicloudCache\re3d\3ds-model\)";

		std::string file = dataDir + "plane/plane.3ds";
		CVRModel model(file);
		model.saveAs(ff::ReplacePathElem(file, "obj", ff::RPE_FILE_EXTENTION));

	}
	void renderToVideo(std::string dataDir, std::string args)
	{
		
	}
	virtual void exec(std::string dataDir, std::string args)
	{
		renderToVideo(dataDir, args);
	}
	virtual Object* clone()
	{
		return new RenderExamples(*this);
	}
};

REGISTER_CLASS(RenderExamples)

_CMDI_END


_CMDI_BEG

void renderModelToVideo(const std::string &modelFile, const std::string &vidFile, const std::string &imageFile)
{
	CVRModel model(modelFile);
	CVRender render(model);
	Size viewSize(800, 800);
	render.setBgColor(1, 1, 1);

	CVRMats matsInit(model, viewSize);

	cv::VideoWriter vw;
	vw.open(vidFile, CV_FOURCC('M', 'J', 'P', 'G'), 15, viewSize);

	matsInit.mModel = cvrm::rotate(-CV_PI/2, Vec3f(1, 0, 0));

	float angle = 0;
	float delta = CV_PI * 2 / 100;
	bool isFirst=true;
	while (true)
	{
		angle += delta;

		CVRMats mats(matsInit);

		//rotate the object
		mats.mModel = mats.mModel * cvrm::rotate(angle, Vec3f(0, 1, 0));

		//CVRResult result = render.exec(mats, viewSize, 3, CVRM_DEFAULT&~CVRM_ENABLE_TEXTURE);
		CVRResult result = render.exec(mats, viewSize, CVRM_IMAGE);
		flip(result.img, result.img, -1);

		imshow("img", result.img);
		if (angle>delta + 1e-6f) //ignore the first frame
		{
			if(isFirst&&!imageFile.empty())
			{
				imwrite(imageFile,result.img);
				isFirst=false;
			}
			vw.write(result.img);
		}	

		waitKey(1);
		//if (waitKey(1) == KEY_ESCAPE)
		if (angle >= CV_PI * 2)
			break;
	}
}

void renderDirModelsToVideo(const std::string &dir)
{
	//std::string dataDir = D_DATA + "/re3d/3ds-models/";

	std::vector<std::string> subDirs;
	ff::listSubDirectories(dir, subDirs, false, false);

	for (auto &d : subDirs)
	{
		std::string name = ff::GetFileName(d, false);
		if(ff::IsDirChar(name.back()))
			name.pop_back();

		std::string file = dir + d + "/" + name + ".3ds";
		if (ff::pathExist(file))
		{
			printf("%s\n", file.c_str());
			std::string vidFile = dir + d + "/" + name + ".avi";
			std::string imageFile=dir+"/"+name+".png";
			renderModelToVideo(file, vidFile,imageFile);
		}
		//break;
	}
}  
 
void on_renderToVideo()
{
	//renderDirModelsToVideo(D_DATA + "/re3d/3ds-model/");
	renderDirModelsToVideo(D_DATA + "/re3d/models-618/");
}

CMD_BEG()
CMD("renderToVideo", on_renderToVideo, "render 3d models as video file", "", "")
CMD_END()

_CMDI_END

