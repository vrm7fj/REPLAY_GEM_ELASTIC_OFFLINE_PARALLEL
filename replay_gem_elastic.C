#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <cstring>

#include "TFile.h"
#include "TCanvas.h"

#include "read_config.h"
#include "hardcode.h"
#include "main.h"

TString prefix;
TString output_filename;

int replay_gem_elastic(const char* configfilename)
{
        // tracker_module_para.clear();
	ConfigFile configfile;
	int configfile_error = configfile.read_configfile(configfilename);

	if ( configfile_error == -1) // stop the program if the config file is incomplete 
	{
		std::cerr << "Program stopping: Error in configuration file.\n";
		return 1;
	}
	
	//Get the input/output paths and input/output file names
	TString input_dirpath = configfile.return_inputdir();
	TString output_dirpath = configfile.return_outputdir();
	TString input_filename = configfile.return_inputfilename();
	output_filename = configfile.return_outputfilename();
	prefix = configfile.return_prefix();

	//std::cout<< " What is the prefix? " << prefix << std::endl;

	//Open the root-file and copy the Tree
	TFile* inputrootfile = new TFile(Form("%s/%s", input_dirpath.Data(), input_filename.Data()), "OPEN");
        //Telling Form to concatenate two C-style strings with a slash (/) between them.	
	TTree* T = (TTree*)inputrootfile->Get("T");

	InitTree(T);

	// Get DB directory and DB filename 
	MOD_DB mod_db;
	int db_error = mod_db.read_MOD_DB(configfilename);

        if ( db_error == -1) // stop the program if the config file is incomplete
        {
                std::cerr << "Program stopping: Error in DB file.\n";
                return 1;
        }

	//Get the input TF DB dir and filename
	TString db_dir = mod_db.return_DB_dir();
	TString db_filename = mod_db.return_DB_filename();

	Init_module_geometry(db_dir, db_filename);
	Init_layer_map(tracker_module_para);
        Init_Grid_Bins(TKR_layer_map);
	Init_hit_list(T);
	project_track_to_layer(T);
        Init_Histograms();
	Fill_Histograms();
	CalcEfficiency();
	Draw_Histograms();
	//hitpostest(T);
	//Is_layer_composite();
	//PrintT(T);
	//IniOAt_FT_module_2D_grid(Module, xglobal, yglobal, zglobal);

	
return 0;
}
