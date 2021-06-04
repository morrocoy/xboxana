#include <iostream>
#include <ctime>

#include "XboxFileConverter.hxx"

void replaceExt(std::string& s, const std::string& newExt) {

   std::string::size_type i = s.rfind('.', s.length());

   if (i != std::string::npos)
      s.replace(i+1, newExt.length(), newExt);

}

int main(int argc, char* argv[]) {

	if (argc != 2) {
		printf("Usage: xboxtdms2root file\n");
		return 1;
	}

	std::string filepath = argv[1];
	clock_t begin = clock();

	XBOX::XboxFileConverter converter;
	converter.addFile(filepath);

	replaceExt(filepath, "root");
	converter.write(filepath);
	clock_t end = clock();

	printf("Total elapsed time: %.3f\n", double(end - begin) / CLOCKS_PER_SEC);
	return 0;
}

