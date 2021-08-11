
#include"DatasetRender.h"

#include"pybind11/pybind11.h"

#include"my_converter.h"
using namespace pybind11::literals;

void init(const char *args)
{ 
	cvrInit(args);
} 

Matx44f test_matx(const Matx44f &m, CVRMats mats)
{
	return mats.mModel;
}
   
PYBIND11_MODULE(cvrender, m) {
	
	NDArrayConverter::init_numpy();

	m.def("init", init, "init the module");

	m.def("test_matx", test_matx, "test...");

	m.attr("CVRM_IMAGE") = (int)CVRM_IMAGE;
	m.attr("CVRM_DEPTH") = (int)CVRM_DEPTH;

	m.attr("CVRM_ENABLE_LIGHTING") = (int)CVRM_ENABLE_LIGHTING;
	m.attr("CVRM_ENABLE_TEXTURE") = (int)CVRM_ENABLE_TEXTURE;
	m.attr("CVRM_ENABLE_MATERIAL") = (int)CVRM_ENABLE_MATERIAL;
	m.attr("CVRM_TEXTURE_NOLIGHTING") = (int)CVRM_TEXTURE_NOLIGHTING;
	m.attr("CVRM_ENABLE_ALL") = (int)CVRM_ENABLE_ALL;
	m.attr("CVRM_TEXCOLOR") = (int)CVRM_TEXCOLOR;
	m.attr("CVRM_DEFAULT") = (int)CVRM_DEFAULT;

	py::class_<CVRRendable>(m, "CVRRendable")
		;

	py::class_<CVRModel, CVRRendable>(m, "CVRModel")
		.def(py::init<>())
		.def(py::init<const std::string&>())
		.def("load", &CVRModel::load, "file"_a, "postProLevel"_a = 3, "options"_a = "")
		//void saveAs(const std::string &file, const std::string &fmtID = "", const std::string &options="-std");
		.def("saveAs",&CVRModel::saveAs,"file"_a,"fmtID"_a="","options"_a="-std")
		;

	py::class_<CVRMats>(m, "CVRMats")
		.def(py::init<>())
		.def(py::init<cv::Size,float,float,float,float>(),"viewSize"_a,"fscale"_a=1.5f,"eyeDist"_a=4.0f,"zNear"_a=0.1,"zFar"_a=100)
		.def(py::init<CVRModel,cv::Size, float, float, float, float>(), "model"_a,"viewSize"_a, "fscale"_a = 1.5f, "eyeDist"_a = 4.0f, "zNear"_a = 0.1, "zFar"_a = 100)
		.def_readwrite("mModeli", &CVRMats::mModeli)
		.def_readwrite("mModel", &CVRMats::mModel)
		.def_readwrite("mView", &CVRMats::mView)
		.def_readwrite("mProjection", &CVRMats::mProjection)
		;

	py::class_<CVRResult>(m, "CVRResult")
		.def_readwrite("img", &CVRResult::img)
		.def_readwrite("depth", &CVRResult::depth)
		.def_readwrite("mats", &CVRResult::mats)
		.def_readwrite("outRect", &CVRResult::outRect)
		;

	py::class_<CVRender>(m, "CVRender")
		.def(py::init<>())
		.def(py::init<CVRRendable&>())
		.def("empty", &CVRender::empty)
		.def("setBgImage", &CVRender::setBgImage)
		.def("clearBgImage", &CVRender::clearBgImage)
		.def("setBgColor", &CVRender::setBgColor)
		//CVRMats &mats, Size viewSize, int output = CVRM_IMAGE | CVRM_DEPTH, int flags = CVRM_DEFAULT, cv::Rect outRect = cv::Rect(0, 0, 0, 0)
		.def("exec", &CVRender::__exec,"mats"_a,"viewSize"_a,"output"_a= (int)CVRM_IMAGE | CVRM_DEPTH,"flags"_a= (int)CVRM_DEFAULT,"outRect"_a=cv::Rect(0,0,0,0))
		;

	py::class_<DatasetRender>(m, "DatasetRender")
		.def(py::init<>())
		.def("loadModels", &DatasetRender::loadModels)
		.def("render", &DatasetRender::render)
		.def("renderToImage", &DatasetRender::renderToImage, "img"_a, "models"_a,
			"centers"_a = std::vector<std::array<int, 2>>(), "sizes"_a = std::vector<int>(),
			"viewDirs"_a = std::vector<std::array<float, 3>>(), "inPlaneRotations"_a = std::vector<float>()
		);
}

//PYBIND11_PLUGIN(pyre3d)
//{
//	NDArrayConverter::init_numpy();
//
//	py::module m("example", "pybind11 opencv example plugin");
//	m.def("read_image", &read_image, "A function that read an image",
//		py::arg("image"));
//
//	m.def("show_image", &show_image, "A function that show an image",
//		py::arg("image"));
//
//	m.def("passthru", &passthru, "Passthru function", py::arg("image"));
//	m.def("clone", &cloneimg, "Clone function", py::arg("image"));
//
//	return m.ptr();
//}
