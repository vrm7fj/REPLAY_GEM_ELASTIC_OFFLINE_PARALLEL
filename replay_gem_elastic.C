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

//std::map<TString, module_para_struct> FT_module_para;
//std::map<TString, module_para_struct> FT_layer_map;

int replay_gem_elastic(const char* configfilename)
{
        FT_module_para.clear();
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
	TString output_filename = configfile.return_outputfilename();

	//Open the root-file and copy the Tree
	TFile* inputrootfile = new TFile(Form("%s/%s", input_dirpath.Data(), input_filename.Data()), "OPEN");
        //Telling Form to concatenate two C-style strings with a slash (/) between them.	
	TTree* T = (TTree*)inputrootfile->Get("T");

        InitTree(T);

	//Test(T);
	/*------------------------------------------------------------------------------------------------------------------------------*/

	FT_DB ft_db;
	int ft_db_error = ft_db.read_FT_DB(configfilename);

        if ( ft_db_error == -1) // stop the program if the config file is incomplete
        {
                std::cerr << "Program stopping: Error in FT DB file.\n";
                return 1;
        }

	//Get the input TF DB dir and filename
	TString ft_db_dir = ft_db.return_DB_dir();
	TString ft_db_filename = ft_db.return_FT_DB_filename();

	Init_FT_module_geometry(ft_db_dir, ft_db_filename);
	Init_layer_map(FT_module_para);
        Init_Grid_Bins(FT_layer_map);
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
