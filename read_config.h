#include <iostream>
#include <string>

#include "hardcode.h"

#ifndef READ_CONFIG_H
#define READ_CONFIG_H

class ConfigFile
{
	//Define variables read in by the configuration file
	TString m_input_dir = "";
	TString m_output_dir = "";
	TString m_outfilename = "";	
	TString m_infilename = "";

public:
	// The following function reads the config file and returns 0 if the all the necessary paramers were specified and returns -1 if not.
	int read_configfile( const char* configfilename)
	{
		ifstream configfile(configfilename);
		TString currentline;

		std::cout <<'\n'<<"--- Reading configuration file: " << configfilename << " --- \n";

		while( currentline.ReadLine(configfile) ) 
		{ 
			if( !currentline.BeginsWith("#") ) 
			{
				TObjArray *tokens = currentline.Tokenize(" ");
				int ntokens = tokens->GetEntries();

				if ( ntokens>1)
				{
					TString skey = ( (TObjString*)(*tokens)[0] )->GetString();

					if ( skey == "input_dir")
					{
						TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
						m_input_dir = sval;
						std::cout << "Input replayed ROOT file directory with elastic cuts applied: " << m_input_dir << endl;
		      		        }

					else if ( skey == "output_dir")
					{
						TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
						m_output_dir = sval;
						std::cout << "Output directory: " << m_output_dir << endl;
		      		        }
					
					else if ( skey == "input_filename")
					{
						TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
						m_infilename = sval;
						std::cout << "Input filename: " << m_infilename << endl;
		      		        }

					else if ( skey == "output_filename")
					{
						TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
						m_outfilename = sval;
						std::cout << "Output filename: " << m_outfilename << endl;
		      		        }

					delete tokens;

				}
				else if ( ntokens == 1)
				{

					TString skey = ( (TObjString*)(*tokens)[0] )->GetString();

					if( skey == "input_dir" )
		      			{
						std::cerr << "Error: Input directory not provied! \n";
						return -1;
		      			}

					else if( skey == "output_dir" )
		      			{
						std::cerr << "Error: Output directory not provied! \n";
						return -1;
		      			}
					
					else if( skey == "input_filename" )
		      			{
						std::cerr << "Error: Input file name not provied! \n";
						return -1;
		      			}

					else if( skey == "output_filename" )
		      			{
						std::cerr << "Error: Output file name not provied! \n";
						return -1;
		      			}

				}
			}
         	}
	return 0;
        }

	TString return_inputdir()
	{
		return m_input_dir;
	}

	TString return_outputdir()
	{
		return m_output_dir;
	}
	
	TString return_inputfilename()
	{
		return m_infilename;
        }

	TString return_outputfilename()
	{
		return m_outfilename;
        }
};

//Class to handle the module geometry input
class FT_DB
{
	//Define DB variables read in by the configuration file
	TString m_db_ft_dir = "";
	TString m_db_ft_filename = "";

public:

        // The following function reads the config file and returns 0 if the all the necessary paramers were specified and returns -1 if not.
        int read_FT_DB( const char* configfilename)
        {
                ifstream ft_db_filename(configfilename);
                TString currentline;

                std::cout <<'\n'<<"--- Reading configuration file: " << configfilename << " --- \n";

                while( currentline.ReadLine(ft_db_filename) )
                {
                        if( !currentline.BeginsWith("#") )
                        {
                                TObjArray *tokens = currentline.Tokenize(" ");
                                int ntokens = tokens->GetEntries();

                                if ( ntokens>1)
                                {
                                        TString skey = ( (TObjString*)(*tokens)[0] )->GetString();

                                        if ( skey == "DB_dir")
                                        {
                                                TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
                                                m_db_ft_dir = sval;
                                                std::cout << "Database directory: " << m_db_ft_dir << endl;
                                        }

                                        else if ( skey == "DB_FT_filename")
                                        {
                                                TString sval = ( (TObjString*)(*tokens)[1] )->GetString();
                                                m_db_ft_filename = sval;
                                                std::cout << "FT database filename: " << m_db_ft_filename << endl;
                                        }

                                        delete tokens;
                                }
                                else if ( ntokens == 1)
                                {

                                        TString skey = ( (TObjString*)(*tokens)[0] )->GetString();

                                        if( skey == "DB_dir" )
                                        {
                                                std::cerr << "Error: Database directory not provied! \n";
                                                return -1;
                                        }


                                        else if( skey == "DB_FT_filename" )
                                        {
                                                std::cerr << "Error: FT database not provied! \n";
                                                return -1;
                                        }

                                }
                        }
                }
        return 0;
        }

        TString return_DB_dir()
        {
                return m_db_ft_dir;
        }

        TString return_FT_DB_filename()
        {
                return m_db_ft_filename;
        }

};


#endif
