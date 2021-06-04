
gSystem->Load("./lib/libxboxcore.lib")
gSystem->Load("../build/tdms/libtdms");
// gROOT->ProcessLine(".include ./../tdms/");

gSystem->Load("./lib/libxboxcore.so")
gSystem->Load("./lib/libxboxio.so")
gSystem->Load("./lib/libxboxconverter.so")
gInterpreter->AddIncludePath("./include/");
gROOT->SetMacroPath("./macros/")
gROOT->ProcessLine(".include ./include/ConverterPanel.hxx");


gInterpreter->AddIncludePath("./../tdms");
// gSystem->Load("/usr/local/bin/libtdms");

gSystem->Load("./bin/libxboxcore.dll")

gInterpreter->AddIncludePath("./include");	
gROOT->ProcessLine(".include ./include");

gSystem->Load("./bin/libxboxcore.dll")

#include "include/XboxDataType.hxx"
#include "include/XboxDAQChannel.hxx"
#include "include/XboxTdmsReader.hxx"

XBOX::XboxDataType a = XBOX::XboxDataType::NATIVE_DOUBLE;

printf("Size: %d\n",a.getSize());
