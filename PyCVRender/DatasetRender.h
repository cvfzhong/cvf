#pragma once

#include"CVRender/cvrender.h"
#include"CVX/core.h"

#include"bind.h"
using namespace cv;


template<typename _ValT, int cn, typename _AlphaValT>
inline void alphaBlendX(const Mat_<Vec<_ValT, cn> > &F, const Mat_<_AlphaValT> &alpha, double alphaScale, Mat_<Vec<_ValT, cn> > &B)
{
	for_each_4(DWHNC(F), DN1(alpha), DNC(B), DNC(B), [alphaScale](const _ValT *f, _AlphaValT a, const _ValT *b, _ValT *c) {
		double w = a * alphaScale;
		for (int i = 0; i < cn; ++i)
			c[i] = _ValT(b[i] + w*(f[i] - b[i]));

	});
}

Mat degrade(const Mat &img, float smoothSigma, Size smoothSize, float noiseStd)
{
	Mat imgf;
	img.convertTo(imgf, CV_32F);

	if (smoothSigma > 0.0f)
	{
		GaussianBlur(imgf, imgf, smoothSize, smoothSigma);
	}

	if (noiseStd > 0)
	{
		Mat noise(img.size(), CV_MAKE_TYPE(CV_32F, img.channels()));
		cv::randn(noise, 0, noiseStd);

		imgf += noise;
	}

	Mat dimg;
	imgf.convertTo(dimg, img.depth());
	return dimg;
}
Mat degradeRand(const Mat &img, float maxSmoothSigma, float maxNoiseStd)
{
	if (maxSmoothSigma < 1e-6f&&maxSmoothSigma < 1e-6f)
		return img.clone();

	float sigma = float(rand()) / RAND_MAX*maxSmoothSigma;
	float size = __max(int(sigma * 6 - 1) | 1, 3);

	return degrade(img, sigma, Size(size, size), float(rand()) / RAND_MAX*maxNoiseStd);
}

void transformF(Mat3b &F, const Mat1b &mask, const Mat3b &B, double saturationR = 0.3)
{
	Mat3b Fx, Bx;
	cvtColor(F, Fx, CV_BGR2HSV);
	cvtColor(B, Bx, CV_BGR2HSV);

	Vec3d fm(0, 0, 0), bm(0, 0, 0);
	int fn = 0, bn = 0;
	for_each_2(DWHN0(Fx), DN1(mask), [&fm, &fn](const Vec3b &f, uchar mv) {
		if (mv)
			fm += f, ++fn;
	});

	for_each_1(DWHN0(Bx), [&bm, &bn](const Vec3b &b) {
		bm += b, ++bn;
	});

	fm *= 1.0 / fn;
	bm *= 1.0 / bn;

	double ss = pow(bm[1] / fm[1], saturationR), sv = bm[2] / fm[2];

	for_each_1(DWHNC(Fx), [ss, sv](uchar *p) {
		p[1] = uchar(__min(p[1] * ss, 255.0) + 0.5);
		p[2] = uchar(__min(p[2] * sv, 255.0) + 0.5);
	});
	cvtColor(Fx, F, CV_HSV2BGR);
}

void composite(Mat3b F, Mat1b mask, Mat3b B, Mat1i objMask, int objID, float maxSmoothSigma = 1.0f, float maxNoiseStd = 5.0f)
{
	CV_Assert(F.size() == B.size() && F.size()==objMask.size());

	//smooth mask boundary
	Mat1f fmask;
	mask.convertTo(fmask, CV_32F, 1.0 / 255);
	GaussianBlur(fmask, fmask, Size(3, 3), 1);

	Mat1b tmask(mask.size()), dmask;
	for_each_2(DWHN1(mask), DN1(tmask), [](uchar m, uchar &t) {
		t = m > 127 ? 255 : 0;
	});
	maxFilter(tmask, dmask, 3);

	//Rect accurateROI = get_mask_roi(DWHN(tmask));

	//smooth fg boundary
	int hwsz = 2, fstride = stepC(F), tmstride = stepC(tmask);
	Rect r(hwsz, hwsz, F.cols - hwsz * 2, F.rows - hwsz * 2);
	for_each_3(DWHNCr(F, r), DNCr(tmask, r), DN1r(dmask, r), [hwsz, fstride, tmstride](uchar *p0, uchar *tm, uchar dm) {
		if (dm > *tm)
		{
			uchar *p = p0 - fstride*hwsz - 3 * hwsz;
			tm = tm - tmstride*hwsz - hwsz;
			const int wsz = hwsz * 2 + 1;
			int clr[3] = { 0,0,0 };
			int nc = 0;
			for (int y = 0; y<wsz; ++y, tm += tmstride, p += fstride)
				for (int x = 0; x < wsz; ++x)
				{
					if (tm[x] != 0)
					{
						const uchar *c = p + 3 * x;
						clr[0] += c[0]; clr[1] += c[1]; clr[2] += c[2]; ++nc;
					}
				}
			if (nc != 0)
			{
				for (int i = 0; i < 3; ++i)
					p0[i] = clr[i] / nc;
			}
		}
	});

	F = degradeRand(F, maxSmoothSigma, maxNoiseStd);

	alphaBlendX(F, fmask, 1.0, B);
	for_each_2(DWHN1(objMask), DN1(tmask), [objID](int &obji, uchar m) {
		if (m)
			obji = objID;
	});

	//return Rect(roi.x + accurateROI.x, roi.y + accurateROI.y, accurateROI.width, accurateROI.height);
}

class DatasetRender
{
	std::vector<CVRModel>  _models;
	std::vector<CVRender>  _renders;
	
public:
	DatasetRender()
	{
	}
	void loadModels(const std::vector<std::string> &files)
	{
#if 1
		size_t size = files.size();
		_models.resize(size);
		_renders.resize(size);

		for (size_t i = 0; i < size; ++i)
		{
			printf("%d: loading %s\n", int(i+1), files[i].c_str());
			_models[i].load(files[i]);
			_renders[i] = CVRender(_models[i]);
		}
#else
		for (auto &f : files)
			printf("%s\n", f.c_str());
#endif
	}
#if 0
	pybind11::tuple render(cv::Mat &img, int count, int speed)
	{
		//return pybind11::make_tuple("ret", cv::Mat::zeros(img.size(), img.type()));
		//cv::Mat dimg = cv::Mat::zeros(img.size(), img.type());
		cv::Mat dimg = img.clone();

		cv::Point2f vpt[] = {
			cv::Point2f(0,1),cv::Point2f(sqrt(3.0f) / 2,-0.5f), cv::Point2f(-sqrt(3.0f) / 2,-0.5f)
		};

		

		//for (int i = 0; i < count; ++i)
		{
			//cv::drawCross(dimg, cv::Point(rand() % dimg.cols, rand() % dimg.rows), 3, cv::Scalar(rand() % 255, rand() % 255, rand() % 255), 2);
			static cv::Point center(img.cols / 2, img.rows / 2);

			center += cv::Point(rand() % 3 - 1, rand() % 3 - 1)*speed;
			if (center.x < 0)
				center.x = 0;
			if (center.y < 0)
				center.y = 0;
			center.x %= img.cols;
			center.y %= img.rows;


			cv::Mat R=cv::getRotationMatrix2D(cv::Point2f(0, 0), float(count), 1.0f);
			R.convertTo(R, CV_32F);

			int r = count%(__min(img.rows, img.cols)/2);
			std::vector<std::vector<cv::Point>> pts(1);
			for (int j = 0; j < 3; ++j)
			{
				cv::Point2f p = vpt[j] * float(r);
				p=cv::transA(p, R.ptr<float>());
				pts[0].push_back(cv::Point(p) + center);
			}

			

			cv::polylines(dimg, pts, true, cv::Scalar(rand() % 255, rand() % 255, rand() % 255), 2);

			//cv::circle(dimg, cv::Point(img.cols / 2, img.rows / 2), r, cv::Scalar(rand() % 255, rand() % 255, rand() % 255),2);

		}

		return pybind11::make_tuple("hello",toHandle(dimg));
	}
#endif

	void render(std::vector<std::array<int,3>> &vv) {
		for (auto &v : vv)
			for (auto i : v)
				printf("%d ", i);
	}

	static Rect _getBoundingBox(Size imgSize, const std::vector<cv::Point> &centers, const std::vector<int> &sizes, int i)
	{
		cv::Point c = uint(i) < centers.size() ? centers[i] : cv::Point(rand() % imgSize.width, rand() % imgSize.height);

		int size = uint(i) < sizes.size() ? sizes[i] : rand() % __min(imgSize.width, imgSize.height);
		size /= 2;

		return Rect(c.x - size, c.y - size, size * 2+1, size * 2+1);
	}
	static Mat1b _getRenderMask(const Mat1f &depth, float eps = 1e-6f)
	{
		Mat1b mask = Mat1b::zeros(depth.size());
		for_each_2(DWHN1(depth), DN1(mask), [eps](float d, uchar &m) {
			m = fabs(1.0f - d)<eps ? 0 : 255;
		});
		return mask;
	}

	/*py::tuple*/cv::Mat _renderToImage(cv::Mat &img, const std::vector<int> &models,
		const std::vector<cv::Point> &centers, const std::vector<int> &sizes,
		const std::vector<cv::Vec3f> &viewDirs, const std::vector<float> &inPlaneRotations
	)
	{
		auto randf = []() {
			return rand() / float(RAND_MAX);
		};

		cv::Mat3b dimg;
		if (img.channels() != 3)
			cv::convertBGRChannels(img, dimg, 3);
		else
			dimg = img.clone();

		Mat1i objMask(dimg.size());
		setMem(objMask, 0xFF);

		for (int i = 0; i < (int)models.size(); ++i)
		{
			int mi = models[i];
			if (uint(mi) >= _models.size())
				continue;

			CVRModel &modeli = _models[mi];
			CVRender &renderi = _renders[mi];

			Rect bb = _getBoundingBox(img.size(), centers, sizes, i);
			Size bbSize(bb.width, bb.height);

			Vec3f viewDir = uint(i) < viewDirs.size() ? viewDirs[i] : Vec3f(randf()*2-1,randf()*2-1,randf()*2-1);
			viewDir = normalize(viewDir);

			float inPlaneRotation = uint(i) < inPlaneRotations.size() ? inPlaneRotations[i] : randf() * 360.0f;
			inPlaneRotation = inPlaneRotation / 180.0f*CV_PI;

			CVRMats mats(modeli, bbSize);
			mats.mModel = cvrm::rotate(Vec3f(0, 0, 1), viewDir) * cvrm::rotate(inPlaneRotation, Vec3f(0,0,1));

			CVRResult r = renderi.exec(mats, bbSize);
			Mat1b mask = _getRenderMask(r.depth);

			Mat3b F = r.img;
			transformF(F, mask, dimg);

			Rect imgROI = rectOverlapped(bb, Rect(0, 0, img.cols, img.rows));
			if (imgROI.width <= 0 || imgROI.height <= 0)
				continue;
			Rect objROI = Rect(imgROI.x - bb.x, imgROI.y - bb.y, imgROI.width, imgROI.height);

			composite(F(objROI), mask(objROI), dimg(imgROI), objMask(imgROI), i);
		}

		{//Do not reallocate img here!!
			Mat t = dimg;
			if (t.channels() != img.channels())
				cv::convertBGRChannels(dimg, t, img.channels());
			copyMem(t, img);
		}
		
		return objMask;
	}
	
	/*py::tuple*/cv::Mat renderToImage(cv::Mat &img, const std::vector<int> &models,
		const std::vector<std::array<int,2>> &centers, const std::vector<int> &sizes,
		const std::vector<std::array<float,3>> &viewDirs, const std::vector<float> &inPlaneRotations
		)
	{
		static_assert(sizeof(std::array<int, 2>) == sizeof(cv::Point),"");
		static_assert(sizeof(std::array<float, 3>) == sizeof(cv::Vec3f), "");
		return _renderToImage(img, models,
			reinterpret_cast<const std::vector<cv::Point>&>(centers), sizes,
			reinterpret_cast<const std::vector<cv::Vec3f>&>(viewDirs), inPlaneRotations
		);
	}

};

