#ifndef TdmsFile_HXX_
#define TdmsFile_HXX_

#include "Rtypes.h"

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "TdmsIfstream.hxx"

#ifndef TDMS_NO_NAMESPACE
namespace TDMS {
#endif


class TdmsGroup;
class TdmsObject;
class TdmsChannel;


class TdmsFile {

protected:
	typedef std::vector<TdmsGroup*> TdmsGroupSet_t;
	typedef std::vector<TdmsObject*> TdmsObjectSet_t;

	// main attributes
	std::string           fFileName;
	TdmsIfstream         *fFile;
	TdmsGroupSet_t        fGroupSet;
	TdmsObject           *fPrevObject;
	ULong64_t             fFileSize;
	Bool_t                fVerbose=false;

	// leadin attributes
	Bool_t                fFlagHasMetaData;
	Bool_t                fFlagHasObjectList;
	Bool_t                fFlagHasRawData;
	Bool_t                fFlagIsInterleaved;
	Bool_t                fFlagIsBigEndian;
	Bool_t                fFlagHasDAQmxData;
	UInt_t                fVersionNumber;
	Long64_t              fNextSegmentOffset;
	ULong64_t             fDataOffset;

	// meta data attributes
	UInt_t                fObjectCount;
	TdmsObjectSet_t       fObjectSet;

	std::map<std::string, std::string> fProperties;

	ULong64_t             readSegment(Bool_t *atEnd);
	void                  readRawData(ULong64_t total_chunk_size);
	void                  readLeadIn();
	void                  readMetaData();
	void                  readObject();
	TdmsObject*           readRawMetaData(ULong64_t total_chunk_size, TdmsObject *prevObject);

	TdmsChannel*          getChannel(TdmsObject *);
	Long64_t              getMetaDataChunkSize();

	void                  addGroup(TdmsGroup* group){fGroupSet.push_back(group);}
	void                  setProperties(std::map<std::string, std::string> props){fProperties = props;}

public:

	TdmsFile(const std::string &filename);
	TdmsFile(const Char_t *filename);
	~TdmsFile();

	void                  init();
	void                  reset();
	void                  clear();
	Int_t                 isOpen(){return fFile->is_open();}
	void                  read(Int_t nsegmax=-1);
	void                  setFile(const Char_t *filename);
	void                  setFile(const std::string &filename) {setFile(filename.c_str());};

	ULong64_t             getFileSize() const {return fFileSize;}
	TdmsGroup*            getGroup(UInt_t) const;
	TdmsGroup*            getGroup(const std::string &) const;
	UInt_t                getGroupCount() const {return fGroupSet.size();}
	std::map<std::string, std::string> getProperties(){return fProperties;}
	std::string           getPropertiesAsString() const;

};

} // end of namespace TDMS


#endif /* TdmsFile_HXX_ */
