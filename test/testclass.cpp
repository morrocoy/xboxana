#include <iostream>
#include <ctime>

// root
#include "Rtypes.h"
#include "TTimeStamp.h"

#include "TFile.h"
#include "TTree.h"
#include "ROOT/RDataFrame.hxx"
#include "ROOT/RDFHelpers.hxx"

// xbox
#include "testclass.hxx"


void writeTree() {

	TFile f("tree2.root","recreate");
	TTree tr("t","");

	testclass obj;
	tr.Branch("b0", &obj);

	for (Int_t i=0; i<10; i++) {
		obj.setVal(i);
		tr.Fill();
	}
	f.Write();
	f.Close();
}


void readTree() {

//	TTreeReader reader("myTree", file);
//	TTreeReaderValue<A_t> a(reader, "A");
//

//	ROOT::EnableImplicitMT();
	ROOT::RDataFrame d("t", "tree2.root");

	//ROOT::RDataFrame d("B0Events", filepath.c_str(), {"PSI_amp"});


//	auto d1 = d.Define<XBOX::XboxDAQChannel2>("b1", "b0");
	auto df = d.Filter("b0.getVal() < 5");

	std::vector<std::string> colNames = df.GetColumnNames();
	for(std::string key : colNames)
		printf("%s\n", key.c_str());

	auto entries = df.Count();
	std::cout << *entries << " entries passed all filters" << std::endl;
}

////////////////////////////////////////////////////////////////////////
/// Main function.
int main(int argc, char* argv[]) {

	writeTree();
	readTree();

}


