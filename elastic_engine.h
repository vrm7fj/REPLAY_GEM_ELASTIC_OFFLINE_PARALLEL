#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <map>
#include <vector>
#include <array>
#include <cmath>

#include "TFile.h"
#include "TVector3.h"
#include "TRotation.h"
#include "hardcode.h"

#ifndef ELASTIC_ENGINE_H
#define ELASTIC_ENGINE_H

//Structor to hold module parameters
struct module_para_struct {
	std::vector<TVector3> module_pos;	
	std::vector<TVector3> module_size;	
	std::vector<TVector3> module_angle;	
	std::vector<std::array<int, 1>> module_layer;	
};

Int_t GEM_FT_NMODULE = 0; //initalize the number of GEM modules - will be read from
std::map<TString, module_para_struct> FT_module_para; //map to hold module parameters and a key

void Init_FT_module_geometry(const char* ft_db_dir, const char* ft_db_filename)
//std::map<TString, module_para_struct> Init_FT_module_geometry(const char* ft_db_dir, const char* ft_db_filename)
{       

	//map to hold module paramters and a key
	//std::map<TString, module_para_struct> FT_module_para;

	//
	//Int_t GEM_FT_NMODULE = 0; //initalize the number of GEM modules - will be read from DB
	/*---------------------------------------------------------------------------------------------------------*/
        	
	//ifstream the dat file
 	TString s_ft_db_infile = Form("%s/%s", ft_db_dir, ft_db_filename); //Dont need .Data() - already const char*
	ifstream ft_db_infile(s_ft_db_infile);
	TString currentline;

	if (!ft_db_infile.is_open()) {
		std::cerr << "Error: Could not open db_sbs.gemFT.dat!!" << std::endl;
	}

	//Lets write something to read number of modules from the DB
	while ( currentline.ReadLine( ft_db_infile ))
	{
		if (!currentline.BeginsWith("#"))
		{
                	TString sbs_gemFT_modules = "sbs.gemFT.modules";	
			TObjArray *ft_tokens = currentline.Tokenize(" ");
		    	int nft_tokens = ft_tokens->GetEntries();
			//std::cout << "ft_tokens: " << nft_tokens << std::endl;
			TString sval_0 = ( (TObjString*)(*ft_tokens)[0] )->GetString();

				if ( sval_0 == sbs_gemFT_modules) {
                                	GEM_FT_NMODULE = nft_tokens - 2; // first word followed by a "=" sign
				}
		delete ft_tokens; 
		}
	}

	ft_db_infile.clear(); 
	ft_db_infile.seekg(0, std::ios::beg); //reset currentline to zero!

	//std::cout << "N_MODULES: " << GEM_FT_NMODULE << std::endl;
        
	//Define an array of module wise parameters for the front tracker in the format sbs.gemFT.mX.YYYYY
        std::vector<TString> gemFT_module_size;
        std::vector<TString> gemFT_module_position;
        std::vector<TString> gemFT_module_angle;
        std::vector<TString> gemFT_module_layer;

        for (Int_t module_ = 0; module_ < GEM_FT_NMODULE; module_++) {
        	gemFT_module_size.push_back(Form("sbs.gemFT.m%d.size", module_));
                gemFT_module_position.push_back(Form("sbs.gemFT.m%d.position", module_));
                gemFT_module_angle.push_back(Form("sbs.gemFT.m%d.angle", module_));
                gemFT_module_layer.push_back(Form("sbs.gemFT.m%d.layer", module_));
        }

	//Now we have to read the db file. 
	while ( currentline.ReadLine( ft_db_infile ))
	{
		TObjArray *ft_tokens = currentline.Tokenize(" ");
		Int_t nft_tokens = ft_tokens->GetEntries();
		
		if (!currentline.BeginsWith("#"))
		{

			TString sval_0 = ( (TObjString*)(*ft_tokens)[0] )->GetString();

                        for (Int_t module_ = 0; module_ < GEM_FT_NMODULE; module_ ++) {

				TString mod_key = Form("m%02d", module_);

				if ( sval_0 == gemFT_module_position[module_]) {

					double val_1 = ( (TObjString*)(*ft_tokens)[2] )->GetString().Atof(); //There is a reason for repeating this
					double val_2 = ( (TObjString*)(*ft_tokens)[3] )->GetString().Atof();
					double val_3 = ( (TObjString*)(*ft_tokens)[4] )->GetString().Atof();
                                        FT_module_para[mod_key].module_pos.emplace_back(val_1, val_2, val_3);

				} else if ( sval_0 == gemFT_module_size[module_]) {
	
					double val_1 = ( (TObjString*)(*ft_tokens)[2] )->GetString().Atof();
					double val_2 = ( (TObjString*)(*ft_tokens)[3] )->GetString().Atof();
					double val_3 = ( (TObjString*)(*ft_tokens)[4] )->GetString().Atof();
                                        FT_module_para[mod_key].module_size.emplace_back(val_1, val_2, val_3);

				} else if ( sval_0 == gemFT_module_angle[module_]) {
	
					double val_1 = ( (TObjString*)(*ft_tokens)[2] )->GetString().Atof();
					double val_2 = ( (TObjString*)(*ft_tokens)[3] )->GetString().Atof();
					double val_3 = ( (TObjString*)(*ft_tokens)[4] )->GetString().Atof();
                                        FT_module_para[mod_key].module_angle.emplace_back(val_1, val_2, val_3);

				} else if ( sval_0 == gemFT_module_layer[module_]) {
					double val_1 = ( (TObjString*)(*ft_tokens)[2] )->GetString().Atof();
                                        FT_module_para[mod_key].module_layer.push_back({static_cast<int>(val_1)});

				}
			}

	 	delete ft_tokens;
		}
	}

	ft_db_infile.close();

	std::cout << "\n----------FT Module Parameters----------\n";

 	for (const auto& entry : FT_module_para) {
	
		const TString& module_key = entry.first;
		const module_para_struct& module_para = entry.second;

		std::cout << module_key << " | ";
		// Layer
		std::cout << "Layer: ";
		for (const auto& layer : module_para.module_layer) {
		    std::cout << "(" << layer[0] << ") ";
		}

		// Positions
		std::cout << "| Positions: ";
		for (const auto& pos : module_para.module_pos) {
		    std::cout << "(" << pos.X() << "," << pos.Y() << "," << pos.Z() << ") ";
		}
		
		// Dimensions
		std::cout << "| Dimensions: ";
		for (const auto& size : module_para.module_size) {
		    std::cout << "(" << size.X() << "," << size.Y() << "," << size.Z() << ") ";
		}
		
		// Angles
		std::cout << "| Angles: ";
		for (const auto& angle : module_para.module_angle) {
		    std::cout << "(" << angle.X() << "," << angle.Y() << "," << angle.Z() << ") ";
		}

       	std::cout << "\n";
    	}
}

/*-------------------------------------------------------------------------------------------------------------------------*/

Double_t 	MODULE[MAXHIT];
Double_t 	XGLOBAL[MAXHIT];
Double_t	YGLOBAL[MAXHIT];
Double_t 	ZGLOBAL[MAXHIT];

Long64_t nentries = 0;

void InitTree(TTree* T)
{
	nentries = T->GetEntries();	

	T->SetBranchAddress("sbs.gemFT.hit.module", &MODULE);
	T->SetBranchAddress("sbs.gemFT.hit.xglobal", &XGLOBAL);
	T->SetBranchAddress("sbs.gemFT.hit.yglobal", &YGLOBAL);
	T->SetBranchAddress("sbs.gemFT.hit.zglobal", &ZGLOBAL);
}

/*--------------------------------------------------------------------------------------------------------------------------*/
// Test Ground

// void Test(TTree* T)
// { 
// 	InitTree(T);

// 	for(Long64_t i = 0; i < nentries; ++i)
// 	{
// 		T->GetEntry(i);
// 		if (MODULE[i] == 0){
// 			std::cout << "Entry: " << i << "	xglobal: " << XGLOBAL[i] << std::endl;
// 		}
// 	}
// }

/*---------------------------------------------------------------------------------------------------------------------------*/

//It should be better switch to a layer based map from the module based map
//Structor to hold modules by layer
struct FT_layer_struct {
        std::vector<Int_t> module_id;
 	std::vector<TVector3> module_pos;	
 	std::vector<TVector3> module_size;	
 	std::vector<TVector3> module_angle;	
 };

std::map<Int_t, FT_layer_struct> FT_layer_map;

 //switch to layer map
void Init_layer_map(const std::map<TString, module_para_struct> &params)
{
for (const auto&  entry : params)
 {
   const TString& module_key = entry.first;
   const module_para_struct& module_para = entry.second;
   Int_t layer_id = module_para.module_layer[0][0];
   
   FT_layer_struct& layer = FT_layer_map[layer_id];

   layer.module_id.push_back(std::stoi(module_key.Data()+1)); // m00 -> 0
   layer.module_pos.insert(layer.module_pos.end(), module_para.module_pos.begin(), module_para.module_pos.end());
   layer.module_size.insert(layer.module_size.end(), module_para.module_size.begin(), module_para.module_size.end());
   layer.module_angle.insert(layer.module_angle.end(), module_para.module_angle.begin(), module_para.module_angle.end());
  }

 std::cout << "-------------------- FT Layer Map ---------------------" << "\n";
 for (const auto& entry : FT_layer_map)
   {
     Int_t layer_id = entry.first;
     const FT_layer_struct& layer_para = entry.second;

     std::cout << "Layer: " << layer_id << ":\n";

     for (size_t mod_id = 0; mod_id < layer_para.module_id.size(); mod_id++)
       {
	 std::cout << "    Module " << layer_para.module_id[mod_id] << ": ";
	 std::cout << "Pos(" << layer_para.module_pos[mod_id].X() << " ," << layer_para.module_pos[mod_id].Y() << " ," << layer_para.module_pos[mod_id].Z() << ") ";
	 std::cout << "Size(" << layer_para.module_size[mod_id].X() << " ," << layer_para.module_size[mod_id].Y() << " ," << layer_para.module_size[mod_id].Z() << ") ";
	 std::cout << "Angle(" << layer_para.module_angle[mod_id].X() << " ," << layer_para.module_angle[mod_id].Y() << " ," << layer_para.module_angle[mod_id].Z() << ") ";
	 std::cout << "\n";
       }
   }
 std::cout << "-------------------------------------------------------" << std::endl;
       
}

std::map<int, double> fZavgLayer; // Average z position of the layer. We'll need it when we project the track
double fZminLayer, fZmaxLayer;
// "Grid bins" for fast track-finding algorithm: define limits of layer active area
std::map<int, double> fXmin_layer, fXmax_layer, fYmin_layer, fYmax_layer;
//double fGridBinWidthX, fGridBinWidthY; // Will define these in hardcode.h
double fGridEdgeToleranceX, fGridEdgeToleranceY;
std::map<int, int> fGridNbinsX_layer, fGridNbinsY_layer;
std::map<int, double> fGridXmin_layer, fGridYmin_layer, fGridXmax_layer, fGridYmax_layer;



void Init_Grid_Bins(const std::map<Int_t, FT_layer_struct>& params)
{
  // Clear all the maps 
  fZavgLayer.clear();
  fXmin_layer.clear();
  fXmax_layer.clear();
  fYmin_layer.clear();
  fYmax_layer.clear();
  fGridNbinsX_layer.clear();
  fGridNbinsY_layer.clear();
  fGridXmin_layer.clear();
  fGridYmin_layer.clear();
  fGridXmax_layer.clear();
  fGridYmax_layer.clear();
  
  std::cout << "------------------------- Initialize Grid Bins ----------------------------" << std::endl;  

  for (const auto& entry : params)
    {
      const Int_t& layer_key = entry.first;
      const FT_layer_struct& layer_para = entry.second;

       //Initiate grid active area for this layer
       double xgmin_all = 1.e12, ygmin_all = 1.e12, xgmax_all = -1.e12, ygmax_all = -1.e12;
       double zsum = 0.0;
 
      std::cout << "Layer " << layer_key << ":\n";

      for (size_t imod = 0; imod < layer_para.module_id.size(); imod++)
	{
	  
          //According to the databse all the positions and angles are given with respect to the first layer.
          //What will happen if the first layer had multiple modules? pick one and fix that one I guess.
          //When a layer has multiple modules, we transpose the reference coordinate system to the origin of the module in that cordinate system and define the rotation angles
          // with respect to that coordinate system - I think :) Muhahahah

	  // Factor to convert degrees to radians
	  constexpr Double_t deg2rad = M_PI/180.0;
          //Create the Rotation Matrix
          TRotation R;
          R.RotateX(deg2rad * layer_para.module_angle[imod].X());
	  R.RotateY(deg2rad * layer_para.module_angle[imod].Y());
	  R.RotateZ(deg2rad * layer_para.module_angle[imod].Z());

	  // Define unit vectors in local frame
	  TVector3 X_unit_local(1, 0, 0);
	  TVector3 Y_unit_local(0, 1, 0);
	  TVector3 Z_unit_local(0, 0, 1);

	  // Calculate unit vectors in the rotated frame
	  TVector3 X_unit_rotated = R * X_unit_local;
	  TVector3 Y_unit_rotated = R * Y_unit_local;
	  TVector3 Z_unit_rotated = R * Z_unit_local;

	  std::cout << "    Module " << imod << " (" << layer_para.module_id[imod] << ") " << "\n";
	  std::cout << "       X_unit_rotated = " << X_unit_rotated.X() << " ," << X_unit_rotated.Y() << " ," << X_unit_rotated.Z() << "\n"
	  << "       Y_unit_rotated = " << Y_unit_rotated.X() << " ," << Y_unit_rotated.Y() << " ," << Y_unit_rotated.Z() << "\n"
	  << "       Z_unit_rotated = " << Z_unit_rotated.X() << " ," << Z_unit_rotated.Y() << " ," << Z_unit_rotated.Z() << std::endl;
    
          // Calculate half width of modules along X and Y using DB
	  Double_t X_mod_half_width = layer_para.module_size[imod].X()/ 2.0;
	  Double_t Y_mod_half_width = layer_para.module_size[imod].Y()/ 2.0;

	  std::cout << "\n";
	  std::cout << "       X_mod_half_width = " << X_mod_half_width << "\n"
	            << "       Y_mod_half_width = " << Y_mod_half_width << std::endl;

	  // To compute average Z coordinate of this layer
	  zsum += layer_para.module_pos[imod].Z();

	  // Calculate the positions of the four corners of the active area of each module
	  TVector3 Corner1 = layer_para.module_pos[imod] - X_mod_half_width * X_unit_rotated - Y_mod_half_width * Y_unit_rotated;
	  TVector3 Corner2 = layer_para.module_pos[imod] + X_mod_half_width * X_unit_rotated - Y_mod_half_width * Y_unit_rotated;
	  TVector3 Corner3 = layer_para.module_pos[imod] - X_mod_half_width * X_unit_rotated + Y_mod_half_width * Y_unit_rotated;
	  TVector3 Corner4 = layer_para.module_pos[imod] + X_mod_half_width * X_unit_rotated + Y_mod_half_width * Y_unit_rotated;

	  //Check if corners 1 and 3 defines the minimum X of the layer. Check all four corners to make sure
      	  xgmin_all = Corner1.X() < xgmin_all ? Corner1.X() : xgmin_all;
	  xgmin_all = Corner2.X() < xgmin_all ? Corner2.X() : xgmin_all;
	  xgmin_all = Corner3.X() < xgmin_all ? Corner3.X() : xgmin_all;
	  xgmin_all = Corner4.X() < xgmin_all ? Corner4.X() : xgmin_all;

	  //Check if corners 2 and 4 defines the maximum Y of the layer. Check all four corners to make sure
	  xgmax_all = Corner1.X() > xgmax_all ? Corner1.X() : xgmax_all;
	  xgmax_all = Corner2.X() > xgmax_all ? Corner2.X() : xgmax_all;
	  xgmax_all = Corner3.X() > xgmax_all ? Corner3.X() : xgmax_all;
	  xgmax_all = Corner4.X() > xgmax_all ? Corner4.X() : xgmax_all;

	  //Check if corners 1 and 2 defines the minimum Y of the layer. Check all four corners to make sure
	  ygmin_all = Corner1.Y() < ygmin_all ? Corner1.Y() : ygmin_all;
	  ygmin_all = Corner2.Y() < ygmin_all ? Corner2.Y() : ygmin_all;
	  ygmin_all = Corner3.Y() < ygmin_all ? Corner3.Y() : ygmin_all;
	  ygmin_all = Corner4.Y() < ygmin_all ? Corner4.Y() : ygmin_all;

          //Check if corners 3 and 4 defines the maximum Y if the layer. Check all four corners to make sure.
	  ygmax_all = Corner1.Y() > ygmax_all ? Corner1.Y() : ygmax_all;
	  ygmax_all = Corner2.Y() > ygmax_all ? Corner2.Y() : ygmax_all;
	  ygmax_all = Corner3.Y() > ygmax_all ? Corner3.Y() : ygmax_all;
	  ygmax_all = Corner4.Y() > ygmax_all ? Corner4.Y() : ygmax_all;

	  //Why don't we use min and max functions? 

	  fXmin_layer[layer_key] = xgmin_all;
	  fXmax_layer[layer_key] = xgmax_all;
	  fYmin_layer[layer_key] = ygmin_all;
	  fYmax_layer[layer_key] = ygmax_all;
	}

      std::cout << "\n";
      std::cout << "       Layer " << layer_key << " min X = " << fXmin_layer[layer_key] << "\n"
	        << "       Layer " << layer_key << " max X = " << fXmax_layer[layer_key] << "\n"
	        << "       Layer " << layer_key << " min Y = " << fYmin_layer[layer_key] << "\n"
	        << "       Layer " << layer_key << " max Y = " << fYmax_layer[layer_key] << std::endl;

      fZavgLayer[layer_key] = zsum / layer_para.module_id.size();

      std::cout << "\n";
      std::cout << "       Layer " << layer_key << " average Z position = " << fZavgLayer[layer_key] << std::endl;

      if( layer_key == 0 || fZavgLayer[layer_key] > fZmaxLayer ) fZmaxLayer = fZavgLayer[layer_key];
      if( layer_key == 0 || fZavgLayer[layer_key] < fZminLayer ) fZminLayer = fZavgLayer[layer_key];

      fGridXmin_layer[layer_key] = fXmin_layer[layer_key] - 0.5 * fGridBinWidthX; // Adding some tolerance to the edges
      Int_t nbinsx = 0;
      while( fGridXmin_layer[layer_key] + nbinsx * fGridBinWidthX < fXmax_layer[layer_key] + 0.5 * fGridBinWidthX )
	{
	  nbinsx++;
	}

      fGridXmax_layer[layer_key] = fGridXmin_layer[layer_key] + nbinsx * fGridBinWidthX;
      fGridNbinsX_layer[layer_key] = nbinsx;

      fGridYmin_layer[layer_key] = fYmin_layer[layer_key] - 0.5 * fGridBinWidthY; // Adding some tolerance to the edges
      Int_t nbinsy = 0;
      while( fGridYmin_layer[layer_key] + nbinsy * fGridBinWidthY < fYmax_layer[layer_key] + 0.5 * fGridBinWidthY )
	{
	  nbinsy++;
	}

      fGridYmax_layer[layer_key] = fGridYmin_layer[layer_key] + nbinsy * fGridBinWidthY;
      fGridNbinsY_layer[layer_key] = nbinsy;

      std::cout << "\n";
      std::cout << "       Layer " << layer_key << " nbinsx = " << nbinsx << " and nbinsy = " << nbinsy << std::endl;
    }

      std::cout << "\n";
      std::cout << "       Tracker Z min = " << fZminLayer << std::endl;
      std::cout << "       Tracker Z max = " << fZmaxLayer << std::endl;
  
} 

#endif
