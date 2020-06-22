#include"appstd.h"


_CMDI_BEG

void list_3dmodels(const std::string &dir)
{
	std::string dirName = ff::GetFileName(dir);

	std::vector<std::string> subDirs;
	ff::listSubDirectories(dir, subDirs);

	std::string listFile = dir + "/list.txt";
	FILE *fp = fopen(listFile.c_str(), "w");
	if (!fp)
	{
		printf("can't open file %s\n", listFile.c_str());
		return;
	}
	for (auto &d : subDirs)
	{
		if (ff::IsDirChar(d.back()))
			d.pop_back();
		fprintf(fp, "%s\t\t%s\n", d.c_str(), (dirName+"/"+d + "/" + d + ".3ds").c_str());
	}
	fclose(fp);
}

void on_list_3dmodels()
{
	//std::string dir = D_DATA + "/re3d/3ds-model/";
	std::string dir = D_DATA + "/re3d/models-618/";
	list_3dmodels(dir);
}

CMD_BEG()
CMD("os.list-3dmodels", on_list_3dmodels, "list 3d model files", "", "")
CMD_END()

_CMDI_END

