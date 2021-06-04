#include "XboxTdmsFileConverter.hxx"

#include <fstream>
#include <ctime>

#include <stdlib.h>
#include <cstring>
#include <errno.h>
#include <limits.h>
#include <sstream>

#ifndef XBOX_NO_NAMESPACE
namespace XBOX {
#endif


std::vector<XboxTdmsFileConverter::Dict_t> XboxTdmsFileConverter::fgXboxChannelMaps = {
	// Xbox0 ChannelMap (dummy map)
	{
		{"DUMMY"             , "DUMMY"  }
	},
	// Xbox1 ChannelMap
	{
		{"PKRA_amp"          , "KREF"              },
		{"PSI_i"             , "Fast INC I"        },
		{"PSI_q"             , "Fast INC Q"        },
		{"PSI_amp"           , "INC"               },
//		{"PSI_avg"           , "INC average"       },
//		{"PSI_max"           , "INC max"           },
//		{"PSI_pulsewidth"    , "INC pulse width"   },
		{"PSR_i"             , "Fast REF I"        },
		{"PSR_q"             , "Fast REF Q"        },
		{"PSR_amp"           , "REF"               },
		{"PEI_i"             , "Fast TRA I"        },
		{"PEI_q"             , "Fast TRA Q"        },
		{"PEI_amp"           , "TRA"               },
//		{"PEI_max"           , "TRA max"           },
		{"PSI_diode"         , "diodeINC"          },
		{"PEI_diode"         , "diodeTRA"          },
		{"PSR_diode"         , "diodeREF"          },
		{"BLM1"              , "BLM1"              },
		{"BLM2"              , "Fast BLM2"         },
		{"BLM3"              , "Fast BLM3"         },
		{"BPM1"              , "BPM1"              },
		{"BPM2"              , "BPM2"              },
		{"MOTOR"             , "Motor Right"       }
	},
	// Xbox2 ChannelMap
	{
		{"PKRA_log"          , "PKR log"           },
		{"PKIA_amp"          , "PKI Amplitude"     },
		{"PKIA_ph"           , "PKI Phase"         },
		{"PERA_log"          , "PER log"           },
		{"PSI_amp"           , "PSI Amplitude"     },
		{"PSI_ph"            , "PSI Phase"         },
		{"PSR_amp"           , "PSR Amplitude"     },
		{"PSR_ph"            , "PSR Phase"         },
		{"PSR_log"           , "PSR log"           },
		{"PEI_amp"           , "PEI Amplitude"     },
		{"PEI_ph"            , "PEI Phase"         },
		{"DC_UP"             , "DC Up"             },
		{"DC_DOWN"           , "DC Down"           },
		{"BLM1"              , "BLM"               },
		{"BLM2"              , "BLM TIA"           },

		// collimator from Uppsala
		{"COL"               , "Col."              },

		// upgrade for Polarix structure
		{"PCI_amp"           , "PCI Amplitude"     },
		{"PCI_ph"            , "PCI Phase"         },
		{"PEI1_amp"          , "PEI1 Amplitude"    },
		{"PEI1_ph"           , "PEI1 Phase"        },
		{"PEI2_amp"          , "PEI2 Amplitude"    },
		{"PEI2_ph"           , "PEI2 Phase"        },
		{"Spare_amp"         , "Spare Amplitude"   },
		{"Spare_ph"          , "Spare Phase"       },
		{"Reference_amp"     , "Reference Amplitude"},
		{"Reference_ph"      , "Reference Phase"   },
		{"Dummy"             , "Dummy"             }
	},
	// Xbox3 ChannelMap
	{
		{"KLYINA_amp"        , "KLYINA_amp"        },
		{"KLYINA_ph"         , "KLYINA_ph"         },
		{"KLYINB_amp"        , "KLYINB_amp"        },
		{"KLYINB_ph"         , "KLYINB_ph"         },
		{"PKRA_amp"          , "PKRA"              },
		{"PKRB_amp"          , "PKRB"              },
		{"PKIA_amp"          , "PKIA_amp"          },
		{"PKIA_ph"           , "PKIA_ph"           },
		{"PKIB_amp"          , "PKIB_amp"          },
		{"PKIB_ph"           , "PKIB_ph"           },
		{"PERA_amp"          , "PERA"              },
		{"PERB_amp"          , "PERB"              },
		{"PLRA_amp"          , "PLRA"              },
		{"PLRB_amp"          , "PLRB"              },
		{"PLIA_amp"          , "PLIA_amp"          },
		{"PLIA_ph"           , "PLIA_ph"           },
		{"PLIB_amp"          , "PLIB_amp"          },
		{"PLIB_ph"           , "PLIB_ph"           },
		{"PSI_amp"           , "PSI_amp"           },
		{"PSI_ph"            , "PSI_ph"            },
		{"PSR_amp"           , "PSR_amp"           },
		{"PSR_ph"            , "PSR_ph"            },
		{"PEI_amp"           , "PEI_amp"           },
		{"PEI_ph"            , "PEI_ph"            },
		{"DC_UP"             , "DC_UP"             },
		{"DC_DOWN"           , "DC_DOWN"           },
		{"Reference228A_amp" , "Reference228A_amp" },
		{"Reference228A_ph"  , "Reference228A_ph"  },
		{"Reference228B_amp" , "Reference228B_amp" },
		{"Reference228B_ph"  , "Reference228B_ph"  },
		{"Reference1600_amp" , "Reference1600_amp" },
		{"Reference1600_ph"  , "Reference1600_ph"  },

		// older format
		{"SPAREIQ_amp" , "SPAREIQ_amp" },
		{"SPAREIQ_ph"  , "SPAREIQ_ph"  },
		{"Reference228_amp" , "Reference228_amp" },
		{"Reference228_ph"  , "Reference228_ph"  },
		{"Reference1600_amp" , "Reference1600_amp" },
		{"Reference1600_ph"  , "Reference1600_ph"  }
	}
};


Int_t XboxTdmsFileConverter::convertChannel(XboxDAQChannel &channel,
		const TDMS::TdmsChannel &tdmschannel, Bool_t bdata) {

	// retrieve names of tdms channel and group used for reading specific properties
	std::string tdmschannelname = tdmschannel.getBaseName();
	std::string tdmsgroupname = fTdmsGroup->getBaseName();

	// conversion buffers
	TTimeStamp ts;
	Int_t ival;
	Double_t dval;
	Bool_t bval;


	// pulse count (Type: ULong64_t)
	ULong64_t npulse;
	if (!convertStrToNum(npulse, fTdmsGroup->getProperty("Pulse Count")))
		channel.setPulseCount(npulse);
	// DeltaF (Type: Double_t).
	if (!convertStrToNum(dval, fTdmsGroup->getProperty("DeltaF")))
		channel.setDeltaF(dval);
	// Line (Type: Double_t).
	if (!convertStrToNum(dval, fTdmsGroup->getProperty("Line")))
		channel.setLine(dval);

	// BreakdownFlag (Type: Bool_t).
	if (!convertStrToNum(bval,
			fTdmsGroup->getProperty("BD_" + tdmschannelname)))
		channel.setBreakdownFlag(bval);
	// BreakdownType (Type: Int_t).
	if (!convertStrToNum(ival,
			fTdmsGroup->getProperty("BD_" + tdmschannelname + "_Type")))
		channel.setBreakdownType(ival);
	// BreakdownThreshDir (Type: Int_t).
	if (!convertStrToNum(ival,
			fTdmsGroup->getProperty("BD_Thresh_Dir" + tdmschannelname)))
		channel.setBreakdownThreshDir(ival);
	// BreakdownThreshDirVal (Type: Int_t).
	if (!convertStrToNum(ival,
			fTdmsGroup->getProperty("BD_Thresh_Val" + tdmschannelname)))
		channel.setBreakdownThreshDirVal(ival);
	// BreakdownRatioVal (Type: Double_t).
	if (!convertStrToNum(dval,
			fTdmsGroup->getProperty("BD_Ratio_Val" + tdmschannelname)))
		channel.setBreakdownRatioVal(dval);


	// StartOffset (Type: Double_t)
	if (!convertStrToNum(dval, tdmschannel.getProperty("wf_start_offset")))
		channel.setStartOffset(dval);
	// Increment (Type: Double_t)
	if (!convertStrToNum(dval, tdmschannel.getProperty("wf_increment")))
		channel.setIncrement(dval);
	// Samples (Type: Int_t)
	if (!convertStrToNum(ival, tdmschannel.getProperty("wf_samples")))
		channel.setSamples(ival);

	// XLabel (Type: std::string)
	channel.setXLabel(tdmschannel.getProperty("wf_xname"));
	// XUnit (Type: std::string)
	channel.setXUnit(tdmschannel.getProperty("wf_xunit_string"));
	// YUnit (Type: std::string)
	channel.setYUnit(tdmschannel.getProperty("unit_string"));
	// YUnitDescription (Type: std::string)
	channel.setYUnitDescription(tdmschannel.getProperty("NI_UnitDescription"));

	// Scale_Type (Type: Int_t). ID to define scale type (REDEFINED).
	std::string sscaletype = tdmschannel.getProperty("Scale_Type");
	if (!sscaletype.compare(0, 3, "Log"))
		channel.setScaleType(0);
	else if (!sscaletype.compare(0, 4, "Poly"))
		channel.setScaleType(1);
	else
		channel.setScaleType(-1); // no scaling

	// ScaleUnit (Type: std::string)
	channel.setScaleUnit(tdmschannel.getProperty("Scale_Unit"));

	// ScaleCoeffs (Type: std::vector<Double_t>). Array of scaling values. (REDEFINED)
	std::vector<Double_t> coeffs;
	if (channel.getScaleType() == 0) {

		// logarithmic coefficients
		for (std::string key :
				std::vector<std::string> { "Scale_Coeff_A", "Scale_Coeff_b", "Scale_Coeff_C" })
			if (!convertStrToNum(dval, tdmschannel.getProperty(key)))
				coeffs.push_back(dval);
			else
				coeffs.push_back(0.);
	} else if (channel.getScaleType() == 1) {

		// polynomial coefficients
		Int_t ikey = 0;
		while (ikey < kMaxCoeff) {
			if (!convertStrToNum(dval,
					tdmschannel.getProperty(
							"Scale_Coeff_c" + std::to_string(ikey++))))
				coeffs.push_back(dval);
			else
				break;
		}
	}
	channel.setScaleCoeffs(coeffs);

	// setDataType (Type: XBOX::XboxDataType). Type of data array to be reinterpreted from binary data
	XBOX::XboxDataType dtype;
	if (!convertDataType(dtype, tdmschannel.getDataType()))
		channel.setDataType(dtype);

	// fill data array in binary format if bdata flag is set
	if (bdata)
		channel.setRawData(tdmschannel.getRawDataVector());

	// xbox specific conversion ...............................
	if (fXboxVersion == kXbox1) {

		// specific conversion from xbox1 TDMS file

		// Timestamp
		if (!convertStrToTs(ts, fTdmsGroup->getProperty("Timestamp"))) {
//			ts.Add(3600); // add one hour due to some LabView inconsistency
			channel.setTimeStamp(ts);
		}
//		else
//			printf("WARNING: Could not convert Timestamp from %40s: %-20s\n",
//					tdmsgroupname.c_str(), tdmschannelname.c_str());

		// StartTime (Type: TTimeStamp). Time stamp of tdms channel.
		if (!convertStrToTs(ts, tdmschannel.getProperty("wf_start_time"))) {
//			ts.Add(3600); // add one hour due to some LabView inconsistency
			channel.setStartTime(ts);
		}

		// LogType (Type: Int_t). Provide the state of the pulse (REDEFINED)
		// -1 Normal pulse (in tdms file: ?)
		//  0 Breakdown event (in tdms file: ?)
		//  1 1st pulse before Breakdown event (in tdms file: ?)
		//  2 2nd pulse before Breakdown event (in tdms file: ?)
		convertStrToNum(ival, fTdmsGroup->getProperty("Log Type"));
		if (ival == 3)
			channel.setLogType(0);
		else if (ival == 2)
			channel.setLogType(1);
		else if (ival == 1)
			channel.setLogType(2);
		else
			channel.setLogType(-1);

	} else if (fXboxVersion == kXbox2) {

		// specific conversion from xbox2 TDMS file

		// Timestamp
		if (!convertStrToTs(ts, fTdmsGroup->getProperty("Timestamp"))) {
			ts.Add(3600); // add one hour due to some LabView inconsistency
			channel.setTimeStamp(ts);
		}
//		else
//			printf("WARNING: Could not convert Timestamp from %40s: %-20s\n",
//					tdmsgroupname.c_str(), tdmschannelname.c_str());

		// StartTime (Type: TTimeStamp). Time stamp of tdms channel.
		if (!convertStrToTs(ts, tdmschannel.getProperty("wf_start_time"))) {
			ts.Add(3600); // add one hour due to some LabView inconsistency
			channel.setStartTime(ts);
		}

		// LogType (Type: Int_t). Provide the state of the pulse (REDEFINED)
		// -1 Normal pulse (in tdms file: 0)
		//  0 Breakdown event (in tdms file: 3)
		//  1 1st pulse before Breakdown event (in tdms file: 2)
		//  2 2nd pulse before Breakdown event (in tdms file: 1)
		convertStrToNum(ival, fTdmsGroup->getProperty("Log Type"));
		if (ival == 3)
			channel.setLogType(0); // Breakdown event
		else if (ival == 2)
			channel.setLogType(1); // first pulse before breakdown
		else if (ival == 1)
			channel.setLogType(2);  // second pulse before breakdown
		else if (ival == 0)
			channel.setLogType(-1); // Normal pulse
		else
			channel.setLogType(999); // Not Valid

		// specific breakdown flags

		// lots of arbitrary chosen breakdown flags came in with the upgrade
		// for Polarix structure. This could be called the entropy of xbox.
		if (!tdmschannelname.compare("PKI Amplitude") || !tdmschannelname.compare("PKI Phase"))
			if (!convertStrToNum(bval, fTdmsGroup->getProperty("BD_PKI")))
				channel.setBreakdownFlag(bval);
		if (!tdmschannelname.compare("PSI Amplitude") || !tdmschannelname.compare("PSI Phase"))
			if (!convertStrToNum(bval, fTdmsGroup->getProperty("BD_PSI")))
				channel.setBreakdownFlag(bval);
		if (!tdmschannelname.compare("PSR Amplitude") || !tdmschannelname.compare("PSR Phase"))
			if (!convertStrToNum(bval, fTdmsGroup->getProperty("BD_PSR")))
				channel.setBreakdownFlag(bval);
		if (!tdmschannelname.compare("PCI Amplitude") || !tdmschannelname.compare("PCI Phase"))
			if (!convertStrToNum(bval, fTdmsGroup->getProperty("BD_PCI")))
				channel.setBreakdownFlag(bval);
		if (!tdmschannelname.compare("PEI1 Amplitude") || !tdmschannelname.compare("PEI1 Phase"))
			if (!convertStrToNum(bval, fTdmsGroup->getProperty("BD_PEI1")))
				channel.setBreakdownFlag(bval);
		if (!tdmschannelname.compare("PEI2 Amplitude") || !tdmschannelname.compare("PEI2 Phase"))
			if (!convertStrToNum(bval, fTdmsGroup->getProperty("BD_PEI2")))
				channel.setBreakdownFlag(bval);
		if (!tdmschannelname.compare("Reference Amplitude") || !tdmschannelname.compare("Reference Phase"))
			if (!convertStrToNum(bval, fTdmsGroup->getProperty("BD_Reference")))
				channel.setBreakdownFlag(bval);
		if (!tdmschannelname.compare("Spare Amplitude") || !tdmschannelname.compare("Spare Phase"))
			if (!convertStrToNum(bval, fTdmsGroup->getProperty("BD_Spare")))
				channel.setBreakdownFlag(bval);


	} else if (fXboxVersion == kXbox3) {

		// specific conversion from xbox3 TDMS file

		// Timestamp
		if (!convertStrToTs(ts, fTdmsGroup->getProperty("Timestamp"))) {
			ts.Add(7200); // add two hours due to some LabView inconsistency
			channel.setTimeStamp(ts);
		}
//		else
//			printf("WARNING: Could not convert Timestamp from %40s: %-20s\n",
//					tdmsgroupname.c_str(), tdmschannelname.c_str());

		// StartTime (Type: TTimeStamp). Time stamp of tdms channel.
		if (!convertStrToTs(ts, tdmschannel.getProperty("wf_start_time"))) {
			ts.Add(7200); // add two hours due to some LabView inconsistency
			channel.setStartTime(ts);
		}

		// LogType (Type: Int_t). Provide the state of the pulse (REDEFINED)
		// -1 Normal pulse (in tdms file: 3)
		//  0 Breakdown event (in tdms file: 2)
		//  1 1st pulse before Breakdown event (in tdms file: 0 is actually the 2nd pulse before breakdown)
		//  2 2nd pulse before Breakdown event (in tdms file: NOT EXIST)
		convertStrToNum(ival, fTdmsGroup->getProperty("Log Type"));
		if (ival == 0)
			channel.setLogType(1); // 1st pulse before Breakdown event
		else if (ival == 1)
			channel.setLogType(999); // Not Valid
		else if (ival == 2)
			channel.setLogType(0); // Breakdown event
		else if (ival == 3)
			channel.setLogType(-1); // Normal pulse
		else
			channel.setLogType(999); // Not Valid
	} else {

		printf("ERROR: Unknown Xbox version. Could not convert data from TDMS file.\n");
		return 1;
	}

	return 0;
}

#ifndef XBOX_NO_NAMESPACE
}
#endif

