
#include"appstd.h"

int main1()
{
	std::string dataDir = D_DATA+"/re3d/test/";

	auto fact = Factory::instance();

	fact->createObject<ICommand>("ShowModels")->exec(dataDir);
	//fact->createObject<ICommand>("RenderExamples")->exec(dataDir);
	//fact->createObject<ICommand>("RenderVOC")->exec(dataDir);
	//fact->createObject<ICommand>("CVCalib1")->exec(dataDir);
	
	return 0;
}

using ff::exec;

int main()
{
	cvrInit("-display :0.0");
	//exec("renderToVideo");
	exec("renderVOC");
	//exec("os.list-3dmodels");
	return 0;
}

