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
#include "read_config.h"

#ifndef MAIN_H
#define MAIN_H

/**************************************************************
 *  Read the Database and Initialize Geometries of Modules   *
 **************************************************************/

//Structor to hold module parameters

extern TString prefix;
extern TString output_filename;

struct module_para_struct {
	std::vector<TVector3> module_pos;	
	std::vector<TVector3> module_size;	
	std::vector<TVector3> module_angle;	
	std::vector<std::array<int, 1>> module_layer;	
};

Int_t GEM_NMODULE = 0; //initalize the number of GEM modules - will be read from
std::map<TString, module_para_struct> tracker_module_para; //map to hold module parameters and a key

void Init_module_geometry(const char* db_dir, const char* db_filename)
{

        tracker_module_para.clear();
	
	// ifstream the .da file from DB
  	TString s_db_infile = Form("%s/%s", db_dir, db_filename); //Dont need .Data() - already const char*
	ifstream db_infile(s_db_infile);
	TString currentline;

	if (!db_infile.is_open()) {
		std::cerr << "Error: Could not open DB file!!" << std::endl;
	}

	// This function will calculate the number of gem modules .dat file
	while ( currentline.ReadLine( db_infile ))
	{
		if (!currentline.BeginsWith("#"))
		{
                	TString sbs_gem_modules = prefix + ".modules";	
			TObjArray *ft_tokens = currentline.Tokenize(" ");
		    	int nft_tokens = ft_tokens->GetEntries();
			//std::cout << "ft_tokens: " << nft_tokens << std::endl;
			TString sval_0 = ( (TObjString*)(*ft_tokens)[0] )->GetString();

				if ( sval_0 == sbs_gem_modules) {
                                	GEM_NMODULE = nft_tokens - 2; // first word followed by a "=" sign
				}
		delete ft_tokens; 
		}
	}

	db_infile.clear(); 
	db_infile.seekg(0, std::ios::beg); //reset currentline to zero
        
	//Define an array of module wise parameters for the front tracker in the format sbs.gemFT.mX.YYYYY
        std::vector<TString> gem_module_size;
        std::vector<TString> gem_module_position;
        std::vector<TString> gem_module_angle;
        std::vector<TString> gem_module_layer;

        for (Int_t module_ = 0; module_ < GEM_NMODULE; module_++) {
        	gem_module_size.push_back(Form(prefix + ".m%d.size", module_));
                gem_module_position.push_back(Form(prefix + ".m%d.position", module_));
                gem_module_angle.push_back(Form(prefix + ".m%d.angle", module_));
                gem_module_layer.push_back(Form(prefix + ".m%d.layer", module_));
        }

	//Now we have to read the db file. 
	while ( currentline.ReadLine( db_infile ))
	{
		TObjArray *ft_tokens = currentline.Tokenize(" ");
		Int_t nft_tokens = ft_tokens->GetEntries();
		
		if (!currentline.BeginsWith("#"))
		{

			TString sval_0 = ( (TObjString*)(*ft_tokens)[0] )->GetString();

                        for (Int_t module_ = 0; module_ < GEM_NMODULE; module_ ++) {

				TString mod_key = Form("m%02d", module_);

				if ( sval_0 == gem_module_position[module_]) {

					double val_1 = ( (TObjString*)(*ft_tokens)[2] )->GetString().Atof(); //There is a reason for repeating this
					double val_2 = ( (TObjString*)(*ft_tokens)[3] )->GetString().Atof();
					double val_3 = ( (TObjString*)(*ft_tokens)[4] )->GetString().Atof();
                                        tracker_module_para[mod_key].module_pos.emplace_back(val_1, val_2, val_3);

				} else if ( sval_0 == gem_module_size[module_]) {
	
					double val_1 = ( (TObjString*)(*ft_tokens)[2] )->GetString().Atof();
					double val_2 = ( (TObjString*)(*ft_tokens)[3] )->GetString().Atof();
					double val_3 = ( (TObjString*)(*ft_tokens)[4] )->GetString().Atof();
                                        tracker_module_para[mod_key].module_size.emplace_back(val_1, val_2, val_3);

				} else if ( sval_0 == gem_module_angle[module_]) {
	
					double val_1 = ( (TObjString*)(*ft_tokens)[2] )->GetString().Atof();
					double val_2 = ( (TObjString*)(*ft_tokens)[3] )->GetString().Atof();
					double val_3 = ( (TObjString*)(*ft_tokens)[4] )->GetString().Atof();
                                        tracker_module_para[mod_key].module_angle.emplace_back(val_1, val_2, val_3);

				} else if ( sval_0 == gem_module_layer[module_]) {
					double val_1 = ( (TObjString*)(*ft_tokens)[2] )->GetString().Atof();
                                        tracker_module_para[mod_key].module_layer.push_back({static_cast<int>(val_1)});

				}
			}

	 	delete ft_tokens;
		}
	}

	db_infile.close();

	std::cout << "\n----------GEM Module Parameters----------\n";

 	for (const auto& entry : tracker_module_para) {
	
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

/**************************************************************
 *           Initialize the ROOT Tree and Branches            *
 **************************************************************/

// Prefix is defined in the main script. Let's extern it here

Double_t 	TKR_HIT_LAYER[MAXHIT];
Int_t           N_TKR_HIT_LAYER[MAXHIT];

Double_t        TKR_TRACK_NHITS[MAXHIT];			 
Double_t 	TKR_HIT_MODULE[MAXHIT];
Double_t        N_TKR_HIT_MODULE[MAXHIT];
Double_t 	TKR_HIT_XLOCAL[MAXHIT];
Double_t	TKR_HIT_YLOCAL[MAXHIT];
Double_t        TKR_HIT_ZGLOBAL[MAXHIT];

Double_t        TKR_TRACK_X[MAXHIT];
Double_t        N_TKR_TRACK_X[MAXHIT];
Double_t        TKR_TRACK_XP[MAXHIT];
Double_t        TKR_TRACK_Y[MAXHIT];
Double_t        N_TKR_TRACK_Y[MAXHIT];
Double_t        TKR_TRACK_YP[MAXHIT];

Long64_t nentries = 0;

void InitTree(TTree* T)
{
	nentries = T->GetEntries();

	std::cout<<  " " << std::endl;
	std::cout<< "PREFIX IS: " << prefix << std::endl;

	T->SetBranchAddress(prefix + ".hit.layer", &TKR_HIT_LAYER);
	T->SetBranchAddress((prefix + ".hit.module"), &TKR_HIT_MODULE);
	T->SetBranchAddress((prefix + ".hit.xlocal"), &TKR_HIT_XLOCAL);
	T->SetBranchAddress((prefix + ".hit.yglobal"), &TKR_HIT_YLOCAL);
	T->SetBranchAddress((prefix + ".hit.zglobal"), &TKR_HIT_ZGLOBAL);
	T->SetBranchAddress((prefix + ".track.nhits"), &TKR_TRACK_NHITS);
	T->SetBranchAddress(("Ndata." + prefix + ".hit.layer"), &N_TKR_HIT_LAYER);
	T->SetBranchAddress(("Ndata." + prefix + ".hit.module"), &N_TKR_HIT_MODULE);
	
	T->SetBranchAddress((prefix + ".track.x"), &TKR_TRACK_X); 
	T->SetBranchAddress(("Ndata." + prefix + ".track.x"), &N_TKR_TRACK_X);
	T->SetBranchAddress((prefix + ".track.y"), &TKR_TRACK_Y);
	T->SetBranchAddress(("Ndata." + prefix + ".track.y"), &N_TKR_TRACK_Y);
	T->SetBranchAddress((prefix + ".track.xp"), &TKR_TRACK_XP);
	T->SetBranchAddress((prefix + ".track.yp"), &TKR_TRACK_YP);

}

/**************************************************************
 *   Switch to a layer-based map from the module-based map    *
 **************************************************************/

//Structor to hold modules by layer
struct TKR_layer_struct {
        std::vector<Int_t> module_id;
 	std::vector<TVector3> module_pos;	
 	std::vector<TVector3> module_size;	
 	std::vector<TVector3> module_angle;	
 };

std::map<Int_t, TKR_layer_struct> TKR_layer_map;

//Initialize number of layers for future use
Int_t TKR_NLAYER = 0;

 //switch to layer map
void Init_layer_map(const std::map<TString, module_para_struct> &params)
{
for (const auto&  entry : params)
 {
   const TString& module_key = entry.first;
   const module_para_struct& module_para = entry.second;
   Int_t layer_id = module_para.module_layer[0][0];
   
   TKR_layer_struct& layer = TKR_layer_map[layer_id];

   layer.module_id.push_back(std::stoi(module_key.Data()+1)); // m00 -> 0
   layer.module_pos.insert(layer.module_pos.end(), module_para.module_pos.begin(), module_para.module_pos.end());
   layer.module_size.insert(layer.module_size.end(), module_para.module_size.begin(), module_para.module_size.end());
   layer.module_angle.insert(layer.module_angle.end(), module_para.module_angle.begin(), module_para.module_angle.end());
  }

 std::cout << "-------------------- Tracker Layer Map ---------------------" << "\n";
 for (const auto& entry : TKR_layer_map)
   {
     Int_t layer_id = entry.first;
     const TKR_layer_struct& layer_para = entry.second;

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

 TKR_NLAYER = TKR_layer_map.size(); //Usefull for later
}

// Lets create a vector that store 1 if a layer is a composite and 0 if not
// std::vector<Int_t> is_layer_composite;
// std::vector<Int_t> N_module_in_layer;
// std::vector<Int_t> module_ids_of_composite_layers;

// void Is_layer_composite (){

//   // is_layer_composite.resize(TKR_NLAYER);
  
//   for (const auto& [map_key, map_data]: TKR_layer_map) {
//     if ( map_data.module_id.size() > 1 ) {
//       is_layer_composite.push_back(1);
//       N_module_in_layer.push_back(map_data.module_id.size());
//       //for ( Int_t imod = 0; imod < map_data.module_id.size(); imod++) {
//       //module_ids_of_composite_layers.push_back(map_data.module_id[imod]);
//       //}
//     } else {
//       is_layer_composite.push_back(0);
//       N_module_in_layer.push_back(map_data.module_id.size());
//     }
//   }
// }

/**************************************************************
 *               Initialize grid bins here                    *
 **************************************************************/

/* Since tracking is already done and each entry in the root file contains only the hits that were used to
   construct the track it is not absolutely necessary to calculate the grid bin widths. But this also calculate
   positions of the edges of each layer which are usefull for Drawing histograms                             */

std::map<int, double> fZavgLayer; // Average z position of the layer. We'll need it when we project the track
double fZminLayer, fZmaxLayer;
// define limits of layer active area
std::map<int, double> fXmin_layer, fXmax_layer, fYmin_layer, fYmax_layer;
//double fGridBinWidthX, fGridBinWidthY; // Will define these in hardcode.h
double fGridEdgeToleranceX, fGridEdgeToleranceY;
std::map<int, int> fGridNbinsX_layer, fGridNbinsY_layer;
std::map<int, double> fGridXmin_layer, fGridYmin_layer, fGridXmax_layer, fGridYmax_layer;



void Init_Grid_Bins(const std::map<Int_t, TKR_layer_struct>& params)
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
      const TKR_layer_struct& layer_para = entry.second;

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


// // // structure to hold hit informations
// // struct hit_list_struct {
// //   std::vector<Int_t> hit_layer;
// //   std::vector<Int_t> hit_status;
// //   std::vector<TVector2> hit_position;
// // };

// // // map to hold hit information corresponding to each hit index
// // std::map<Int_t, hit_list_struct> hit_list;

// // // Temp vectors for filling the hit_list map
// // std::vector<Int_t> layers_with_hits;
// // std::vector<TVector2> hit_pos_on_layer;

// // void Init_hit_list( TTree* T){

// //   InitTree(T);

// //   //std::cout << TKR_NLAYER << std::endl;
  
// //   for (Int_t T_index = 0; T_index < nentries; T_index++) {
    
// //     T->GetEntry(T_index);

// //     hit_list_struct current_hit;

// //     Int_t n_hits_on_track = TKR_TRACK_NHITS[0]; // Number of hits we have on this track

// //     layers_with_hits.clear();
// //     hit_pos_on_layer.clear();

// //     for (Int_t hit_index = 0; hit_index < n_hits_on_track; hit_index++){
// //       layers_with_hits.push_back(TKR_HIT_LAYER[hit_index]); // These layers have hits

// //             Double_t hit_x_pos = TKR_HIT_XLOCAL[hit_index];
// //             Double_t hit_y_pos = TKR_HIT_YLOCAL[hit_index];

// // 	    hit_pos_on_layer.push_back(TVector2(hit_x_pos, hit_y_pos));
// //     }

// //     // //Debug Print
// //     // for ( Int_t i = 0 ; i < hit_pos_on_layer.size(); i++) 
// //     // }

// //     // Fill the hit_list map now
// //     // Loop over all the layers and update all layers with hits and mark the layers without hits
// //     //std::cout << " How about here? " << std::endl;
// //     for ( Int_t ilayer = 0; ilayer < TKR_NLAYER; ilayer++) {
      
// //       //Check if this layer exists in layers_with_hits
// //       auto found_layer = std::find(layers_with_hits.begin(), layers_with_hits.end(), ilayer);

// //       // std::cout << *found_layer << std::endl;
// //       // Found hit on this layer
// //       if ( found_layer != layers_with_hits.end() ) {
// // 	Int_t hit_index = std::distance(layers_with_hits.begin(), found_layer); // Get the corresponding index
// // 	current_hit.hit_layer.push_back(ilayer);
// // 	current_hit.hit_status.push_back(1);
// // 	current_hit.hit_position.push_back(hit_pos_on_layer[hit_index]); //index is what needed here.  
// //         } else {
// // 	current_hit.hit_layer.push_back(ilayer);
// // 	current_hit.hit_status.push_back(-1);
// // 	current_hit.hit_position.push_back(TVector2(-1,-1));
// // 	}
     
// //     }

// //     // Update the map
// //     hit_list[T_index] = current_hit;      
// //   }
// //   // // Print the map for debug purposes
// //   // for (const auto& [hit_idx, hit_data]: hit_list){
// //   //   std::cout << "  Track index: " << hit_idx << "\n";
// //   //   for ( Int_t idx = 0 ; idx < TKR_NLAYER; idx++) {
// //   //     std::cout << "    Hit layer: " << hit_data.hit_layer[idx]<< std::setw(10) << " | "  
// //   // 		<< "    Hit status: " << std::setw(5) << hit_data.hit_status[idx] << std::setw(10) << " | " 
// //   //     << "    Hit positions: (" << hit_data.hit_position[idx].X() << ", " << hit_data.hit_position[idx].Y() << ") " << std::endl;  
// //   //   }
// //   // }
// // }


/**************************************************************
 *   Initialize the map containing all the hit information    *
 **************************************************************/

// structure to hold hit informations
struct hit_list_struct {
  std::vector<Int_t> hit_layer;
  std::vector<Int_t> hit_status;
  std::vector<TVector2> hit_position;
};

// map to hold hit information corresponding to each hit index
std::map<Int_t, hit_list_struct> hit_list;

// Temp vectors for filling the hit_list map
std::vector<Int_t> layers_with_hits;
std::vector<Int_t> modules_with_hits;
std::vector<TVector2> hit_pos_on_layer;

void Init_hit_list( TTree* T){

  InitTree(T);

  //std::cout << TKR_NLAYER << std::endl;
  
  for (Int_t T_index = 0; T_index < nentries; T_index++) {
    
    T->GetEntry(T_index);

    hit_list_struct current_hit;

    Int_t n_hits_on_track = TKR_TRACK_NHITS[0]; // Number of hits we have on this track

    layers_with_hits.clear();
    modules_with_hits.clear();
    hit_pos_on_layer.clear();

    for (Int_t hit_index = 0; hit_index < n_hits_on_track; hit_index++){

      Int_t hit_layer = TKR_HIT_LAYER[hit_index];
      Int_t hit_module = TKR_HIT_MODULE[hit_index];
      
      layers_with_hits.push_back(hit_layer); // These layers have hits
      modules_with_hits.push_back(hit_module);

      auto layer_it = TKR_layer_map.find(hit_layer);

      if ( layer_it == TKR_layer_map.end()) {
	std::cerr << " Layer " << hit_layer << " not found in TKR_layer_map [Error in Init_hit_list] " << std::endl;
	continue;
      }
      
      const auto& layer_para = layer_it->second;
      const auto& ids = layer_para.module_id;

	// Only search if this is a composite module
	if ( ids.size() > 1 ) {

          auto it = std::find(ids.begin(), ids.end(), hit_module);
	  if (it != ids.end() ) {

	    size_t distance_index = std::distance(ids.begin(), it);

	    // We have to shift hits by module positions if this a composite module
	    Double_t x_shift = layer_para.module_pos[distance_index].X();
	    Double_t y_shift = layer_para.module_pos[distance_index].Y();
	      
            Double_t hit_x_pos = TKR_HIT_XLOCAL[hit_index] + x_shift;
            Double_t hit_y_pos = TKR_HIT_YLOCAL[hit_index] + y_shift;

	    hit_pos_on_layer.push_back(TVector2(hit_x_pos, hit_y_pos));
	  } else {
	    std::cout << " Module not found: Init_hit_list " << std::endl;
	  }
	} else {

      	    Double_t hit_x_pos = TKR_HIT_XLOCAL[hit_index];
            Double_t hit_y_pos = TKR_HIT_YLOCAL[hit_index];

	    hit_pos_on_layer.push_back(TVector2(hit_x_pos, hit_y_pos));
	}

      }  
  

    // //Debug Print
    // for ( Int_t i = 0 ; i < hit_pos_on_layer.size(); i++) 
    // }

    // Fill the hit_list map now
    // Loop over all the layers and update all layers with hits and mark the layers without hits
    //std::cout << " How about here? " << std::endl;
    for ( Int_t ilayer = 0; ilayer < TKR_NLAYER; ilayer++) {
      
      //Check if this layer exists in layers_with_hits
      auto found_layer = std::find(layers_with_hits.begin(), layers_with_hits.end(), ilayer);

      // std::cout << *found_layer << std::endl;
      // Found hit on this layer
      if ( found_layer != layers_with_hits.end() ) {
	Int_t hit_index = std::distance(layers_with_hits.begin(), found_layer); // Get the corresponding index
	current_hit.hit_layer.push_back(ilayer);
	current_hit.hit_status.push_back(1);
	current_hit.hit_position.push_back(hit_pos_on_layer[hit_index]); //index is what needed here.  
        } else {
	current_hit.hit_layer.push_back(ilayer);
	current_hit.hit_status.push_back(666);
	current_hit.hit_position.push_back(TVector2(666,666));
	}
     
    }

    // Update the map
    hit_list[T_index] = current_hit;      
  }
  // // Print the map for debug purposes
  // for (const auto& [hit_idx, hit_data]: hit_list){
  //   std::cout << "  Track index: " << hit_idx << "\n";
  //   for ( Int_t idx = 0 ; idx < TKR_NLAYER; idx++) {
  //     std::cout << "    Hit layer: " << hit_data.hit_layer[idx]<< std::setw(10) << " | "  
  // 		<< "    Hit status: " << std::setw(5) << hit_data.hit_status[idx] << std::setw(10) << " | " 
  //     << "    Hit positions: (" << hit_data.hit_position[idx].X() << ", " << hit_data.hit_position[idx].Y() << ") " << std::endl;  
  //   }
  // }
}


//   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// // std::vector<Int_t> mod_on_track;

// // void hitpostest(TTree* T) {

// //     InitTree(T);

// //     for ( Int_t T_index = 0; T_index < nentries; T_index++) {

// //       T->GetEntry(T_index);

// //       mod_on_track.clear();

// //       Int_t n_hits_on_track = TKR_TRACK_NHITS[0]; // Number of hits we have on this track

// //       for ( Int_t hit_index = 0; hit_index < n_hits_on_track; hit_index++ ) {

// // 	mod_on_track.push_back(TKR_HIT_MODULE[hit_index]);

// // 	//Lets check if this hit is on a composite module
// //         for (const auto& [layer_key, layer_para] : TKR_layer_map ) {
// // 	  const std::vector<Int_t>& ids = layer_para.module_id;

// // 	  // Only search if this is a composite module
// // 	  if ( ids.size() > 1 ) {
	    
// // 	    auto it = std::find(ids.begin(), ids.end(), TKR_HIT_MODULE[hit_index]);
// // 	    if (it != ids.end() ) {
// // 	      size_t distance_index = std::distance(ids.begin(), it);

// // 	      std::cout << layer_para.module_id[distance_index] << std::endl;

	      
// //             }
// // 	  }
	  
// // 	}
        	
// //       }

// //       std::cout << "\n";

// //       for (const auto& imod : mod_on_track ) {
// //         printf ( "mod on track: %d \n", imod );
// //       }
// //     }

// //     // std::cout << "\n";

// //     // for (const auto& imod : mod_on_track ) {
// //     //     printf ( "mod on track: %d \n", imod );
// //     // }
// // }
//   //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/**************************************************************
 *               Project Tracks to Each Layer                 *
 **************************************************************/


// Structure to hold track data
struct track_list_struct {
  std::vector<Int_t> layer_on_track;
  std::vector<TVector2> track_pos_on_layer;
};

// map to hold track data corresponding to each entry in the Tree
std::map<Int_t, track_list_struct> track_list;

// Function to project tracks to layers

void project_track_to_layer(TTree* T) {

  InitTree(T);

  

  for ( Int_t T_index = 0; T_index < nentries; T_index++ ) {

    T->GetEntry(T_index);

    track_list_struct track_list_para;

    // Get the positions and angles of the track with respect to the first layer.
    Double_t track_x = TKR_TRACK_X[0];
    Double_t track_y = TKR_TRACK_Y[0];
    Double_t track_xp = TKR_TRACK_XP[0];
    Double_t track_yp = TKR_TRACK_YP[0];

    // Debug Print
    //std::cout << "( " << track_x << ", " << track_y << ", " << track_xp << ", " << track_yp << " )" << std::endl;

    for ( Int_t ilayer = 0; ilayer < TKR_NLAYER; ilayer++ ) {

      if ( ilayer == 0 ) {
	// no need to project track if its layer 0
	track_list_para.layer_on_track.push_back(ilayer);
	track_list_para.track_pos_on_layer.push_back(TVector2(track_x, track_y));
      } else {
	// We have to project the track now. We will use the average z position of the layer.
	Double_t x_project = fZavgLayer[ilayer] * track_xp + track_x;
	Double_t y_project = fZavgLayer[ilayer] * track_yp + track_y;
         
	track_list_para.layer_on_track.push_back(ilayer);
	track_list_para.track_pos_on_layer.push_back(TVector2(x_project, y_project));
      }
    }

    // Add this entry to the track_list
    track_list[T_index] = track_list_para;
    
  }
  // Lets print things shall we?
  // Print the hit map and track projection map

  // Print the map for debug purposes
  for (const auto& [hit_idx, hit_data]: hit_list){
    const auto& track_data = track_list.at(hit_idx);
    
    printf("  Track index: %d\n", hit_idx);
    for (Int_t idx = 0; idx < TKR_NLAYER; idx++) {
        printf(" Hit layer: %8d | Hit status: %5d | Hit positions: (%3.8f, %3.8f) | Track positions: (%3.8f, %3.8f)\n",
               hit_data.hit_layer[idx],
               hit_data.hit_status[idx],
               hit_data.hit_position[idx].X(),
               hit_data.hit_position[idx].Y(),
               track_data.track_pos_on_layer[idx].X(),
               track_data.track_pos_on_layer[idx].Y());
    }
  }
  // for (const auto& [hit_idx, hit_data]: hit_list){
  //   const auto& track_data = track_list.at(hit_idx); // import track list to this
    
  //   std::cout << "  Track index: " << hit_idx << "\n";
  //   for ( Int_t idx = 0 ; idx < TKR_NLAYER; idx++) {
  //     std::cout << " Hit layer: " << hit_data.hit_layer[idx]<< std::setw(10) << " | "  
  // 		<< " Hit status: " << std::setw(5) << hit_data.hit_status[idx] << std::setw(10) << " | " 
  // 		<< " Hit positions: (" << hit_data.hit_position[idx].X() << ", " << hit_data.hit_position[idx].Y() << ") " << std::setw(15) << " | " << " Track positions: (" << track_data.track_pos_on_layer[idx].X() << ", " << track_data.track_pos_on_layer[idx].Y() << ") " <<  std::endl;  
  //   }
  //}
}

// void PrintT( TTree* T) {
//   InitTree(T);

//   for (Int_t T_index = 0; T_index < nentries; T_index++) {
//     T->GetEntry(T_index);

//     //std::cout << "Test: " << TKR_TRACK_NHITS << std::endl;
//     std::cout<< "N_TKR_HIT_LAYER: " << N_TKR_HIT_LAYER[0] << std::endl;

//     for (Int_t ii = 0; ii < N_TKR_HIT_LAYER[0]; ii++) {
//       std::cout<< T_index << " TKR_HIT_LAYER: " << TKR_HIT_LAYER[ii] << std::endl;
//       Int_t hit_idx = static_cast<Int_t>(TKR_HIT_LAYER[ii]);
//       std::cout<< "Positions: " << TKR_HIT_XLOCAL[hit_idx] << std::endl;
//     }
//     for (Int_t jj = 0; jj < N_TKR_HIT_LAYER[0]; jj++) {
//       std::cout << "\n";
//       std::cout << T_index << " TKR_TRACK_NHITS: " << TKR_TRACK_NHITS[0] << std::endl;
//     }
//   }
// }

/**************************************************************
 *         Initialize the Histograms to be filled             *
 **************************************************************/
    
// Initialize the histograms

  TClonesArray *hdidhit_x_layer;
  TClonesArray *hdidhit_y_layer;
  TClonesArray *hdidhit_xy_layer;

  TClonesArray *hshouldhit_x_layer;
  TClonesArray *hshouldhit_y_layer;
  TClonesArray *hshouldhit_xy_layer;

  TClonesArray *hefficiency_x_layer;
  TClonesArray *hefficiency_y_layer;
  TClonesArray *hefficiency_xy_layer;

void Init_Histograms() {
  
   hdidhit_x_layer = new TClonesArray( "TH1D", TKR_NLAYER );
   hdidhit_y_layer = new TClonesArray( "TH1D", TKR_NLAYER );
   hdidhit_xy_layer = new TClonesArray( "TH2D", TKR_NLAYER );

   hshouldhit_x_layer = new TClonesArray( "TH1D", TKR_NLAYER );
   hshouldhit_y_layer = new TClonesArray( "TH1D", TKR_NLAYER );
   hshouldhit_xy_layer = new TClonesArray( "TH2D", TKR_NLAYER );

   hefficiency_x_layer = new TClonesArray( "TH1D", TKR_NLAYER );
   hefficiency_y_layer = new TClonesArray( "TH1D", TKR_NLAYER );
   hefficiency_xy_layer = new TClonesArray( "TH2D", TKR_NLAYER );
   
   for (Int_t ilayer = 0; ilayer < TKR_NLAYER; ilayer++) {
     
      int nbinsx1D = int( round( (fXmax_layer[ilayer]-fXmin_layer[ilayer] + 0.02)/fBinSizeX_efficiency1D ) );
      int nbinsy1D = int( round( (fYmax_layer[ilayer]-fYmin_layer[ilayer] + 0.02)/fBinSizeY_efficiency1D ) );

      int nbinsx2D = int( round( (fXmax_layer[ilayer]-fXmin_layer[ilayer] + 0.02)/fBinSizeX_efficiency2D ) );
      int nbinsy2D = int( round( (fYmax_layer[ilayer]-fYmin_layer[ilayer] + 0.02)/fBinSizeY_efficiency2D ) );

      new( (*hdidhit_x_layer)[ilayer] ) TH1D( Form( "hdidhit_x_layer%d", ilayer ), "x of hits on good tracks (m); x(m)", nbinsx1D, fXmin_layer[ilayer]-0.01, fXmax_layer[ilayer]+0.01 );
      new( (*hdidhit_y_layer)[ilayer] ) TH1D( Form( "hdidhit_y_layer%d", ilayer ), "y of hits on good tracks (m); y(m)", nbinsy1D, fYmin_layer[ilayer]-0.01, fYmax_layer[ilayer]+0.01 );
      new( (*hdidhit_xy_layer)[ilayer] ) TH2D( Form( "hdidhit_xy_layer%d", ilayer ), "xy of hits on good tracks (m); y(m); x(m)", nbinsy2D, fYmin_layer[ilayer]-0.01, fYmax_layer[ilayer]+0.01, nbinsx2D, fXmin_layer[ilayer]-0.01, fXmax_layer[ilayer]+0.01 );

      new( (*hshouldhit_x_layer)[ilayer] ) TH1D( Form( "hshouldhit_x_layer%d", ilayer ), "x of track crossing layer; x(m)", nbinsx1D, fXmin_layer[ilayer]-0.01, fXmax_layer[ilayer]+0.01 );
      new( (*hshouldhit_y_layer)[ilayer] ) TH1D( Form( "hshouldhit_y_layer%d", ilayer ), "y of track crossing layer; y(m)", nbinsy1D, fYmin_layer[ilayer]-0.01, fYmax_layer[ilayer]+0.01 );
      new( (*hshouldhit_xy_layer)[ilayer] ) TH2D( Form( "hshouldhit_xy_layer%d", ilayer ), "x vs y of track crossing (m); y(m); x(m)", nbinsy2D, fYmin_layer[ilayer]-0.01, fYmax_layer[ilayer]+0.01, nbinsx2D, fXmin_layer[ilayer]-0.01, fXmax_layer[ilayer]+0.01 );        

      new( (*hefficiency_x_layer)[ilayer] ) TH1D( Form( "hefficiency_x_layer%d", ilayer ), "track-based efficiency vs x, averaged over y; x(m); efficiency", nbinsx1D, fXmin_layer[ilayer]-0.01, fXmax_layer[ilayer]+0.01 );
      new( (*hefficiency_y_layer)[ilayer] ) TH1D( Form( "hefficiency_y_layer%d", ilayer ), "track-based efficiency vs y, averaged over x; y(m); efficiency", nbinsy1D, fYmin_layer[ilayer]-0.01, fYmax_layer[ilayer]+0.01 );
      new( (*hefficiency_xy_layer)[ilayer] ) TH2D( Form( "hefficiency_xy_layer%d", ilayer ), "track-based efficiency vs x, y; y(m); x(m)", nbinsy2D, fYmin_layer[ilayer]-0.01, fYmax_layer[ilayer]+0.01, nbinsx2D, fXmin_layer[ilayer]-0.01, fXmax_layer[ilayer]+0.01 );     

   }
}


/**************************************************************
 *                    Fill the Histograms                     *
 **************************************************************/

void Fill_Histograms() {

  // Interate over all the layers
  //   Interate over all the entries in the hit_list and track_list
  //     fill didhit and shouldhit histograms

  for (Int_t ilayer = 0; ilayer < TKR_NLAYER; ilayer++) {

    // Loop over etries of track_list and extract the corresponding entries from hit_list at the same time

    for (const auto& [track_key, track_data]: track_list) {
      const auto& hit_data = hit_list.at(track_key);

      ( (TH1D*) (*hshouldhit_x_layer)[ilayer] )->Fill( track_data.track_pos_on_layer[ilayer].X());
      ( (TH1D*) (*hshouldhit_y_layer)[ilayer] )->Fill( track_data.track_pos_on_layer[ilayer].Y());
      // switched X and Y, so that histograms look more natural
      ( (TH2D*) (*hshouldhit_xy_layer)[ilayer] )->Fill( track_data.track_pos_on_layer[ilayer].Y(), track_data.track_pos_on_layer[ilayer].X() );

      //std::cout << "Filled xy values are: " << track_data.track_pos_on_layer[ilayer].Y() << ", " << track_data.track_pos_on_layer[ilayer].X() << std::endl;

      // Check hit status and fill the didhit histograms
      if ( hit_data.hit_status[ilayer] != 666 ) {

      ( (TH1D*) (*hdidhit_x_layer)[ilayer] )->Fill( hit_data.hit_position[ilayer].X());
      ( (TH1D*) (*hdidhit_y_layer)[ilayer] )->Fill( hit_data.hit_position[ilayer].Y());
      ( (TH2D*) (*hdidhit_xy_layer)[ilayer] )->Fill( hit_data.hit_position[ilayer].Y(), hit_data.hit_position[ilayer].X() );
      }
    }
  }
  
}

/**************************************************************
 *            Calculate Track-Based Efficiencies              *
 **************************************************************/

//Calculate Efficiencies
std::vector<Double_t> layer_elastic_efficiency;

void CalcEfficiency() {

  for (Int_t ilayer = 0; ilayer < TKR_NLAYER; ilayer++) {
    ( (TH1D*) (*hefficiency_x_layer)[ilayer] )->Sumw2();
    ( (TH1D*) (*hefficiency_x_layer)[ilayer] )->Divide( ( (TH1D*) (*hdidhit_x_layer)[ilayer] ), ( (TH1D*) (*hshouldhit_x_layer)[ilayer] ), 1.0, 1.0, "B" );
    ( (TH1D*) (*hefficiency_y_layer)[ilayer] )->Sumw2();
    ( (TH1D*) (*hefficiency_y_layer)[ilayer] )->Divide( ( (TH1D*) (*hdidhit_y_layer)[ilayer] ), ( (TH1D*) (*hshouldhit_y_layer)[ilayer] ), 1.0, 1.0, "B" );
    ( (TH1D*) (*hefficiency_xy_layer)[ilayer] )->Sumw2();
    ( (TH2D*) (*hefficiency_xy_layer)[ilayer] )->Divide( ( (TH2D*) (*hdidhit_xy_layer)[ilayer] ), ( (TH2D*) (*hshouldhit_xy_layer)[ilayer] ), 1.0, 1.0, "B" );

    // Efficiency = didhit/shouldhit

    Double_t elastic_efficiency = (( (TH1D*) (*hdidhit_xy_layer)[ilayer] )->GetEntries() / ( (TH1D*) (*hshouldhit_xy_layer)[ilayer] )->GetEntries()) * 100;

    layer_elastic_efficiency.push_back(elastic_efficiency);

    std::cout << " Layer " << ilayer << " efficiency = " << elastic_efficiency << std::endl;
  }
}

/**************************************************************
 *                   Print Histograms to PDF                  *
 **************************************************************/

// Let's Draw the histrograms shall we

void Draw_Histograms() {
  TString outpdfname = output_filename;
  Int_t nlayer = TKR_NLAYER;

  //gStyle->SetOpsStat(0);
  TCanvas* c = new TCanvas("c", "canvas_for_plots", 900, 900);

  c->Print(outpdfname + "[");

  gStyle->SetPalette(kDarkBodyRadiator);

  Int_t one_time_hist = 1;

  for (Int_t ilayer = 0; ilayer < nlayer; ilayer++) {

    // Page with all shouldhit XY histograms
    if (one_time_hist != 0) {
      c->Clear();
      int nCols = 4;
      int nRows = (nlayer + nCols - 1) / nCols;  // Round up
      c->Divide(nCols, nRows);

      for (Int_t ilayer = 0; ilayer < nlayer; ilayer++) {
        c->cd(ilayer + 1);
        ((TH2D*)(*hshouldhit_xy_layer)[ilayer])->GetXaxis()->SetRangeUser(-0.25, 0.25);
        ((TH2D*)(*hshouldhit_xy_layer)[ilayer])->GetYaxis()->SetRangeUser(-0.85, 0.85);
        ((TH2D*)(*hshouldhit_xy_layer)[ilayer])->SetTitle(Form("Layer %d: shouldhit", ilayer));
        ((TH2D*)(*hshouldhit_xy_layer)[ilayer])->Draw("COLZ");
      }
      c->Print(outpdfname);

      c->Clear();
      c->Divide(nCols, nRows);

      for (Int_t ilayer = 0; ilayer < nlayer; ilayer++) {
        c->cd(ilayer + 1);
        ((TH2D*)(*hdidhit_xy_layer)[ilayer])->GetXaxis()->SetRangeUser(-0.25, 0.25);
        ((TH2D*)(*hdidhit_xy_layer)[ilayer])->GetYaxis()->SetRangeUser(-0.85, 0.85);
        ((TH2D*)(*hdidhit_xy_layer)[ilayer])->SetTitle(Form("Layer %d: didhit", ilayer));
        ((TH2D*)(*hdidhit_xy_layer)[ilayer])->Draw("COLZ");
      }
      c->Print(outpdfname);
    
    }

    one_time_hist = 0;
    
    
    c->Clear();
    c->Divide(2,2);

    //should hit and did hit x on pad 1
    c->cd(1);
    ( (TH1D*)(*hshouldhit_x_layer)[ilayer] )->SetLineColor(kBlue);
    ( (TH1D*)(*hdidhit_x_layer)[ilayer] )->SetLineColor(kRed);

    //( (TH1D*)(*hshouldhit_x_layer)[ilayer] )->SetTitle(Form("Layer_%d_shouldhit_didhit_x", ilayer));
    ( (TH1D*)(*hshouldhit_x_layer)[ilayer] )->Draw("HIST");
    ( (TH1D*)(*hdidhit_x_layer)[ilayer] )->Draw("HIST SAME");

    TLegend* leg1 = new TLegend(0.12, 0.75, 0.4, 0.88);
    leg1->AddEntry((TH1D*)(*hshouldhit_x_layer)[ilayer], "shouldhit", "l");
    leg1->AddEntry((TH1D*)(*hdidhit_x_layer)[ilayer], "didhit", "l");
    leg1->SetBorderSize(0);
    leg1->SetFillStyle(0);
    leg1->Draw();

    //should hit and did hit y on pad 2
    c->cd(2);
    ( (TH1D*)(*hshouldhit_y_layer)[ilayer] )->SetLineColor(kBlue);
    ( (TH1D*)(*hdidhit_y_layer)[ilayer] )->SetLineColor(kRed);

    //( (TH1D*)(*hshouldhit_y_layer)[ilayer] )->SetTitle(Form("Layer_%d_shouldhit_didhit_y", ilayer));
    ( (TH1D*)(*hshouldhit_y_layer)[ilayer] )->Draw("HIST");
    ( (TH1D*)(*hdidhit_y_layer)[ilayer] )->Draw("HIST SAME");

    TLegend* leg2 = new TLegend(0.12, 0.75, 0.4, 0.88);
    leg2->AddEntry((TH1D*)(*hshouldhit_y_layer)[ilayer], "shouldhit", "l");
    leg2->AddEntry((TH1D*)(*hdidhit_y_layer)[ilayer], "didhit", "l");
    leg2->SetBorderSize(0);
    leg2->SetFillStyle(0);
    leg2->Draw();

    // should hit xy on pad 3
    c->cd(3);
    //( (TH2D*)(*hshouldhit_xy_layer)[ilayer] )->SetTitle(Form("Layer_%d_shouldhit_xy", ilayer));
    //( (TH2D*)(*hshouldhit_xy_layer)[ilayer] )->GetYaxis()->SetRangeUser(-0.25, 0.25);
    //( (TH2D*)(*hshouldhit_xy_layer)[ilayer] )->GetXaxis()->SetRangeUser(-0.85, 0.85);
    ( (TH2D*)(*hshouldhit_xy_layer)[ilayer] )->Draw("COLZ");

    
    // should hit xy on pad 3
    c->cd(4);
    //( (TH2D*)(*hdidhit_xy_layer)[ilayer] )->SetTitle(Form("Layer_%d_didhit_xy", ilayer));
    ( (TH2D*)(*hdidhit_xy_layer)[ilayer] )->GetXaxis()->SetRangeUser(-0.25, 0.25);
    ( (TH2D*)(*hdidhit_xy_layer)[ilayer] )->GetYaxis()->SetRangeUser(-0.85, 0.85);
    ( (TH2D*)(*hdidhit_xy_layer)[ilayer] )->Draw("COLZ");

    c->Print(outpdfname);
    
    c->Clear();
    c->Divide(2,2);
    
    c->cd(1);
    ( (TH1D*)(*hefficiency_x_layer)[ilayer] )->SetLineColor(kRed);

    //( (TH1D*)(*hshouldhit_y_layer)[ilayer] )->SetTitle(Form("Layer_%d_shouldhit_didhit_y", ilayer));
    ( (TH1D*)(*hefficiency_x_layer)[ilayer] )->Draw("HIST");

    c->cd(2);
    //( (TH1D*)(*hefficiency_y_layer)[ilayer] )->SetLineColor(kRed);

    //    ( (TH1D*)(*hshouldhit_y_layer)[ilayer] )->SetTitle(Form("Layer_%d_shouldhit_didhit_y", ilayer));
    ( (TH1D*)(*hefficiency_y_layer)[ilayer] )->Draw("HIST"); 
      
    c->cd(3);
    ( (TH2D*)(*hefficiency_xy_layer)[ilayer] )->GetXaxis()->SetRangeUser(-0.25, 0.25);
    ( (TH2D*)(*hefficiency_xy_layer)[ilayer] )->GetYaxis()->SetRangeUser(-0.85, 0.85);

    //    ( (TH1D*)(*hshouldhit_y_layer)[ilayer] )->SetTitle(Form("Layer_%d_shouldhit_didhit_y", ilayer));
    //gStyle->SetPalette(kDarkBodyRadiator);
    ( (TH2D*)(*hefficiency_xy_layer)[ilayer] )->Draw("COLZ");

    c->cd(4);
    TLatex latex;
    latex.SetNDC();
    latex.SetTextSize(0.05);
    latex.DrawLatex(.4, .6, Form("Layer: %.1d", ilayer));
    latex.DrawLatex(.2, .5, Form("Elastic Efficiency = %.1f %%", layer_elastic_efficiency[ilayer]));
    
    c->Update();
    c->Print(outpdfname);

  //   std::cout << "Layer " << ilayer
  //         << " eff_x entries = " << ((TH1D*)(*hefficiency_x_layer)[ilayer])->GetEntries() << "\n"
  //         << " eff_y entries = " << ((TH1D*)(*hefficiency_y_layer)[ilayer])->GetEntries() << "\n"
  //         << " eff_xy entries = " << ((TH2D*)(*hefficiency_xy_layer)[ilayer])->GetEntries() << std::endl;

  }

  c->Print(outpdfname + "]"); 
}

#endif

