#include "testclass.hxx"

ClassImp(testclass);


testclass::testclass() {
	a = 0;
}

testclass::testclass(Int_t val) {
	a = val;
}

testclass::~testclass() {
}


void testclass::setVal(Int_t val) {
	a = val;
}
