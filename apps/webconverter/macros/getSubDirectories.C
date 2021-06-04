std::vector<TSystemDirectory> getListofSubDirectories(const std::string &sDirName="/home/kpapke/programming/")
{
	std::vector<TSystemDirectory> vSubDirectories;
	TSystemDirectory oDirectory(sDirName.c_str(), sDirName.c_str());
	cout << oDirectory.GetName() << endl;
	oDirectory.Print();
	auto *lEntries = oDirectory.GetListOfFiles();
	if (lEntries)
	{
		TSystemDirectory *oEntry;
		TIter next(lEntries);
		while ((oEntry=(TSystemDirectory*)next()))
		{
			if (oEntry->IsDirectory() && oEntry->IsFolder()
					&& strcmp(oEntry->GetName(), ".")
					&& strcmp(oEntry->GetName(), ".."))
			{
//				cout << oEntry->GetName() << endl;
//				cout << oEntry->Dra() << endl;
				//oEntry->Print();
				std::string dir = sDirName + "/" + oEntry->GetName();
				//TSystemDirectory dir(dir.c_str(), dir.c_str());
				vSubDirectories.push_back(TSystemDirectory(dir.c_str(), dir.c_str());
				//TSystemDirectory test = *oSubDirectory;
			}
		}
	}
	return vSubDirectories;
}
