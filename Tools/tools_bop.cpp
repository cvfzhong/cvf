
#include"nlohmann/json.hpp"
#undef string_t

#include"appstd.h"
#include<fstream>
#include<iostream>
using namespace std;

_CMDI_BEG

template<typename _ValT, typename _JElemT>
inline void get_vector(_JElemT &jx, void *val)
{
	auto v = jx.get<std::vector<_ValT>>();
	FFAssert(!v.empty());
	memcpy(val, &v[0], sizeof(float)*v.size());
}

Mat1b getRenderMask(const Mat1f &depth, float eps=1e-6f)
{
	Mat1b mask = Mat1b::zeros(depth.size());
	for_each_2(DWHN1(depth), DN1(mask), [eps](float d, uchar &m) {
		m = fabs(1.0f - d)<eps ? 0 : 255;
	});
	return mask;
}

Matx33f getR(const Matx44f &m)
{
	Matx33f R;
	for (int i = 0; i < 3; ++i)
		for (int j = 0; j < 3; ++j)
			R(i, j) = m(i, j);
	return R;
}

void _rend_train_set(std::string imgDir, std::string maskDir, std::string jsonFile, CVRModel &model, CVRender &render, Size viewSize, int nViews, int nRotations)
{
	std::vector<Vec3f> vecs;
	cvrm::sampleSphere(vecs, nViews);

	//Size viewSize(500, 500);

	/*std::vector<Mat>  mRotations;
	for (int i = 0; i < nRotations; ++i)
	{
		mRotations[i] = cv::getRotationMatrix2D(Point2f(viewSize.width / 2.0f, viewSize.height / 2.0f), 360.0*i / nRotations, 1.0);
	}*/

	nlohmann::json jf;

	int imgID = 0;
	for(int n=0; n<vecs.size(); ++n)
	{
		printf("render %d/%d    \r", n+1, nViews);

		auto viewDir = vecs[n];
		auto center = model.getCenter();
		auto sizeBB = model.getSizeBB();
		float maxBBSize = __max(sizeBB[0], __max(sizeBB[1], sizeBB[2]));
		float unitizeScale = 2.0 / maxBBSize;
		float eyeDist = 4.0f / unitizeScale;
		auto eyePos = center + viewDir*eyeDist;
		float fscale = 1.5f;

		CVRMats objMats;
		objMats.mModel = cvrm::lookat(eyePos[0], eyePos[1], eyePos[2], center[0], center[1], center[2], 0, 0, 1);
		objMats.mProjection = cvrm::perspective(viewSize.height*fscale, viewSize, __max(1,eyeDist-maxBBSize), eyeDist+maxBBSize);

		/*Matx33f R = getR(objMats.modelView());
		cout << endl << R << endl << vecs[n] << endl;*/

		for (int i = 0; i < nRotations; ++i)
		{
			objMats.mView = cvrm::rotate(360.0f*i / nRotations*CV_PI / 180.0f, Vec3f(0, 0, 1));

			CVRResult rr = render.exec(objMats, viewSize);

			Mat1b mask = getRenderMask(rr.depth);
			Rect roi = cv::get_mask_roi(DWHS(mask), 127);

			if (roi.empty())
				continue;

			/*mask = cv::imscale(mask(roi), viewSize, INTER_LINEAR);
			Mat dimg = cv::imscale(rr.img(roi), viewSize, INTER_LINEAR);*/
			mask = mask(roi).clone();
			Mat dimg = rr.img(roi).clone();
			cvtColor(dimg, dimg, CV_BGR2GRAY);

			//imshow("img", dimg); imshow("mask", mask);

			++imgID;
			std::string dname = ff::StrFormat("%06d", imgID);
			imwrite(imgDir + "/" + dname+".png", dimg);
			imwrite(maskDir + "/" + dname+".png", mask);

			Matx33f R = getR(objMats.modelView());
			jf[dname] = {
				{"mR",std::vector<float>(R.val,R.val + 9) },
				{"viewDir",{viewDir[0],viewDir[1],viewDir[2]}},
				{"inPlaneRotation",{360.0f*i/nRotations}}
			};

			//cv::waitKey();
		}
	}

	std::ofstream os(jsonFile);
	if (!os)
		throw "file open failed";
	os << jf;
}

//float RDist(const Matx33f &a, const Matx33f &b)
//{
//	float d = 0, n = 0;
//	//for (int i = 0; i < 3; ++i)
//	int i = 2;
//	{
//		float c = 0;
//		for (int j = 0; j < 3; ++j)
//			c += a(j, i)*b(j, i);
//		d += acos(c);
//		n += 1;
//	}
//	return d / n /CV_PI *180.0;
//}

float RDist(const Matx33f &a, const Matx33f &b)
{
	float dm = 0;
	for (int i = 0; i < 3; ++i)
	{
		float d = 0;
		for (int j = 0; j < 3; ++j)
			d += a(j, i)*b(j, i);
		d = acos(d);
		if (d > dm)
			dm = d;
	}
	return dm / CV_PI *180.0;
}

//float RDist(const Matx33f &a, const Matx33f &b)
//{
//	float d = 0;
//	for (int i = 0; i < 9; ++i)
//		d += (a.val[i] - b.val[i])*(a.val[i] - b.val[i]);
//	return d;
//}

nlohmann::json jsonLoad(const std::string &file)
{
	std::ifstream is(file);
	if (!is)
		throw "file open failed";
	nlohmann::json jf;
	is >> jf;
	return jf;
}

void test_data_set(string queryPath, string trainPath)
{
	nlohmann::json qjf=jsonLoad(queryPath + "/list.json"), tjf=jsonLoad(trainPath + "/list.json");

	auto qimDir = queryPath + "/image/";
	std::vector<string> qfiles;
	ff::listFiles(qimDir, qfiles);

	std::vector<Matx33f> vtR;
	std::vector<string>   vtName;
	for (auto &x : tjf.items())
	{
		vtName.push_back(x.key());
		Matx33f R;
		get_vector<float>(x.value()["mR"], R.val);
		vtR.push_back(R);
	}

	for (auto &f : qfiles)
	{
		imshow("query", imread(qimDir+f));

		string name = ff::GetFileName(f, false);

		Matx33f qR;
		get_vector<float>(qjf[name]["mR"], qR.val);

		int mi = 0;
		float md = FLT_MAX;
		for (int i = 0; i < (int)vtR.size(); ++i)
		{
			float d = RDist(vtR[i], qR);
			if (d < md)
			{
				mi = i;
				md = d;
			}
		}
		printf("md=%.6f\n", md);
		imshow("match", imread(trainPath + "/image/" + vtName[mi] + ".png"));
		cv::waitKey();
	}

}


void gen_views_dataset()
{
	//std::string dataDir = R"(f:/store/datasets/BOP/lm_test_bop19/test/)";
	//std::string dataDir=R"(f:/store/datasets/BOP/tless_test_primesense_bop19/test_primesense/)";
	//std::string dataDir = R"(f:/store/datasets/BOP/itodd_test_bop19/test/)";

	std::string setName = "ycbv";
	std::string dataDir = R"(f:/store/datasets/BOP/ycbv_test_bop19/test/)";
	std::string modelDir = R"(F:\store\datasets\BOP\ycbv_models\models\)";

	int nobjs = 0;
	for (; ff::pathExist(modelDir + ff::StrFormat("obj_%06d.ply", nobjs + 1)); ++nobjs);

	std::string rootDir = R"(f:/bop_views/)";

	std::vector<CVRModel>  models(nobjs + 1);
	std::vector<CVRender>  renders(models.size());

	//test_data_set(rootDir + "/query/ycbv/obj_000019/", rootDir + "/trainV16R12/ycbv/obj_000019/"); return;
	test_data_set(rootDir + "/trainV16R12/ycbv/obj_000019/", rootDir + "/trainV30R12/ycbv/obj_000019/"); return;

	bool  gen_train_set = true, gen_query_set = false;

	if (gen_train_set)
	{
		Size viewSize(256, 256);
		int nViews = 16, nRotations = 12;

		std::string dstDir = rootDir + ff::StrFormat("trainV%dR%d/", nViews, nRotations);
		if (ff::pathExist(dstDir))
			ff::removeDirectoryRecursively(dstDir);
		ff::makeDirectory(dstDir + setName);

		std::string curDir = dstDir + setName + "/";

		for (int obj_id = 1; obj_id <= nobjs; ++obj_id)
		{
			printf("for obj %d\n", obj_id);
			auto objName = ff::StrFormat("obj_%06d", obj_id);
			auto imgDir = curDir + objName + "/image/";
			ff::makeDirectory(imgDir);

			auto maskDir = curDir + objName + "/mask/";
			ff::makeDirectory(maskDir);

			auto jsonFile = curDir + objName + "/list.json";

			models[obj_id] = CVRModel(modelDir + objName + ".ply");
			renders[obj_id] = CVRender(models[obj_id]);

			_rend_train_set(imgDir, maskDir, jsonFile, models[obj_id], renders[obj_id], viewSize, nViews, nRotations);
			//break;
		}
	}

	if (gen_query_set)
	{
		std::string dstDir = rootDir + "/query/" + setName + "/";
		if (ff::pathExist(dstDir))
			ff::removeDirectoryRecursively(dstDir);
		ff::makeDirectory(dstDir);

		struct DObj
		{
			string  dimgDir;
			nlohmann::json  jf;
			int             nimgs = 0;
		};

		std::vector<DObj> vobjs(nobjs + 1);

		for (int i = 1; i <= nobjs; ++i)
		{
			auto dir = dstDir + ff::StrFormat("obj_%06d/image/", i);
			ff::makeDirectory(dir);
			vobjs[i].dimgDir = dir;
		}


		std::vector<std::string> subDirs;
		ff::listSubDirectories(dataDir, subDirs);

		int si = 0;
		for (auto &subDir : subDirs)
		{
			++si;
			std::string subRoot = dataDir + "/" + subDir + "/";
			std::string imgDir = subRoot + "rgb/";

			std::vector<std::string> files;
			ff::listFiles(imgDir, files);

			nlohmann::json  jf, jcam;
			{
				std::string jsonFile = dataDir + subDir + "/scene_gt.json";
				std::ifstream is(jsonFile);
				is >> jf;
			}
			{
				std::string jsonFile = dataDir + subDir + "/scene_camera.json";
				std::ifstream is(jsonFile);
				is >> jcam;
			}

			int fi = 0;
			for (auto &f : files)
			{
				++fi;
				printf("for scene %d/%d image %d/%d       \r", si, subDirs.size(), fi, files.size());
				auto fname = ff::GetFileName(f, false);
				int fid = atoi(fname.c_str());
				char buf[32];
				const char *name = itoa(fid, buf, 10);
				auto jx = jf[name];

				Matx33f K;
				get_vector<float>(jcam[name]["cam_K"], K.val);

				cv::Mat dimg = imread(imgDir + f);
				cvtColor(dimg, dimg, CV_BGR2GRAY);
				//				imshow("img", dimg);
				for (auto &x : jx)
				{
					auto obj_id = x["obj_id"].get<int>();
					/*if (obj_id != 1)
						continue;*/

					Matx33f R;
					Vec3f t;
					get_vector<float>(x["cam_R_m2c"], R.val);
					get_vector<float>(x["cam_t_m2c"], &t);

					if (models[obj_id].empty())
					{
						std::string modelFile = modelDir + ff::StrFormat("obj_%06d.ply", obj_id);
						models[obj_id] = CVRModel(modelFile);
						renders[obj_id] = CVRender(models[obj_id]);
					}

					CVRMats mats;
					mats.mProjection = cvrm::fromK(K, dimg.size(), 1, 2000);
					mats.mModel = cvrm::fromR33T(R, t);

					auto rr = renders[obj_id].exec(mats, dimg.size());
					Mat mask = getRenderMask(rr.depth);
					Rect roi = cv::get_mask_roi(DWHS(mask), 127);

					auto &dobj = vobjs[obj_id];

					++dobj.nimgs;
					auto dname = ff::StrFormat("%06d", dobj.nimgs);
					cv::imwrite(dobj.dimgDir + dname + ".png", dimg(roi));
					R = getR(mats.modelView());
					dobj.jf[dname] = {
						{"mR",std::vector<float>(R.val,R.val + 9)},
						{"viewDir",{R.val[2],R.val[5],R.val[8]}}
					};

					//imshow("rr", dimg(roi));
					//cv::waitKey(100);
				}
			}
		}

		for (int i = 1; i <= nobjs; ++i)
		{
			std::ofstream os(vobjs[i].dimgDir+"../list.json");
			if (!os)
				throw "file open failed";
			os << vobjs[i].jf;
		}
	}
}


CMD_BEG()
CMD0("tools.bop.gen_views_dataset", gen_views_dataset)
CMD_END()



_CMDI_END

