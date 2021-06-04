{

	// linux settings
	gInterpreter->AddIncludePath("-I../include/");

	//	gSystem->Load("../lib/libtestclass.so");
	gSystem->Load("../lib/libxboxcore.so");
	gSystem->Load("../lib/libxboxio.so");
	gSystem->Load("../lib/libxboxanalyses.so");

	gInterpreter->Declare("#include \"XboxDAQChannel.hxx\"");
	// windows settings

	// use absolute pathes for interactive mode where the current path may change
//	gInterpreter->AddIncludePath("D:\\programming\\xbox\\build\\include\\");
//
//	gSystem->Load("../lib/libxboxanalyses.dll");
//	gSystem->Load("../lib/libxboxcore.dll");
//	gSystem->Load("../lib/libxboxio.dll");


//	gROOT->SetMacroPath(".");


}
