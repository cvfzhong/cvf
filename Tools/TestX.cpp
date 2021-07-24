//#include<iostream>
//#include<fstream>
//using namespace std;
//
//void testStream()
//{
//	std::ostream os(NULL, false);
//	os.set_rdbuf(cout.rdbuf());
//	os.clear(ios::goodbit);
//	os << "hello" << endl;
//	cout << "world" << endl;
//}
//
//#include"BFC/log.h"
//using namespace ff;
//
//void testLog()
//{
//	//LOG.initialize();
//	//LOG.etbeg();
//	_sleep(100);
//	LOG << loget() << endl;
//	LOG("file open failed...%s..in %s\n", loget(), logfl());
//}

//#define WIN32

#include"appstd.h"
#include<map>
#include"BFC/netcall.h"
#include"CVX/bfsio.h"
using namespace ff;

_CMDI_BEG

void test_mat_nd()
{
	Mat3f m(3, 2);

	int sizes[] = { 2,3,4,5 };
	m.create(3, sizes);
	Size sz = m.size();
	int v = m.rows;
	auto mx=m.reshape(4);
	
	return;
}

void test_net_call_1()
{
	test_mat_nd();

	std::vector<int> v = { 1,2,455,88,99 };
	int farr[][3] = { {1,2,3},{4,5,6} };
	Mat m = Mat3i::ones(4, 4);
	std::string str = "hello";
	std::vector<std::string> vstrs = { "he","she","me" };
	std::vector<Mat3i> vm = { m,m,m };

	ff::NetObjs objs = { 
		{"x",v},{"y",m },{"z",str},{"img",ff::ObjStream::fromImage(cv::imread("f:/img.png"))} ,
		{"vx",vstrs}, {"vm",vm}
	};

	auto data = ff::netcall_encode(objs);
	auto dobjs = ff::netcall_decode(data);

	auto x = dobjs["x"].get<decltype(v)>();
	auto y = dobjs["y"].get<Mat3i>();
	auto z = dobjs["z"].get<std::string>();
	auto img = dobjs["img"].getImage();
	auto vx = dobjs["vx"].get<std::vector<std::string>>();
	auto vmx = dobjs["vm"].get<std::vector<Mat1i>>();

	rude::Socket skt;
	bool r = skt.connect("127.0.0.1", 8000);
	int n = skt.send(data.c_str(), data.size());
	auto rdata = netcall_recv(&skt);
	skt.close();

	auto robjs = ff::netcall_decode(rdata);

	auto x1 = robjs["x"].get<decltype(v)>();
	//auto y1 = robjs["y"].getMat<int>();
	auto y1 = robjs["y"].get<Mat3i>();
	auto z1 = robjs["z"].get<std::string>();
	auto img1 = robjs["img"].getImage();
	auto vx1 = robjs["vx"].get<std::vector<std::string>>();


	imshow("img1", img1);
	cv::waitKey();
}

void test_net_call()
{
	std::vector<int> v = { 1,2,455,88,99 };
	int farr[][3] = { { 1,2,3 },{ 4,5,6 } };
	Mat m = Mat3i::ones(4, 4);
	std::string str = "hello";
	std::vector<std::string> vstrs = { "he","she","me" };
	std::vector<Mat3i> vm = { m,m,m };

	ff::NetObjs objs = {
		{ "x",v },{ "y",m },{ "z",str },{ "img",ff::ObjStream::fromImage(cv::imread("f:/img.png")) } ,
		{ "vx",vstrs },{ "vm",vm }
	};

	auto data = ff::netcall_encode(objs);
	auto dobjs = ff::netcall_decode(data);

	auto x = dobjs["x"].get<decltype(v)>();
	auto y = dobjs["y"].get<Mat3i>();
	auto z = dobjs["z"].get<std::string>();
	auto img = dobjs["img"].getImage();
	auto vx = dobjs["vx"].get<std::vector<std::string>>();
	auto vmx = dobjs["vm"].get<std::vector<Mat1i>>();

	rude::Socket skt;
	bool r = skt.connect("101.76.215.159", 8000);

	for(int i=0; i<10; ++i)
	{
		printf("%d\n", i);

		int n = skt.send(data.c_str(), data.size());
		auto rdata = netcall_recv(&skt);

		auto robjs = ff::netcall_decode(rdata);

		auto labels = robjs["labels"].get<std::vector<std::string>>();
		auto bboxes = robjs["bboxes"].get<Mat1f>();
		auto scores = robjs["scores"].get<std::vector<double>>();
		auto poses = robjs["poses"].get<Mat4f>();
		//const char *p=skt.reads();
		//p = p;
	}
	skt.close();

	return;
}

CMD_BEG()
CMD0("test.net_call",test_net_call)
CMD_END()

_CMDI_END



