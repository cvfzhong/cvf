#include<iostream>
#include<fstream>
using namespace std;

void testStream()
{
	std::ostream os(NULL, false);
	os.set_rdbuf(cout.rdbuf());
	os.clear(ios::goodbit);
	os << "hello" << endl;
	cout << "world" << endl;
}

#include"BFC/log.h"
using namespace ff;

void testLog()
{
	//LOG.initialize();
	//LOG.etbeg();
	_sleep(100);
	LOG << loget() << endl;
	LOG("file open failed...%s..in %s\n", loget(), logfl());
}

