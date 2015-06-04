#include "CodeMigrate.h"

#include "dirent.h"
#include <iostream>
#include <fstream>
#include <functional>
#include <algorithm>

CodeMigrate::CodeMigrate()
{
}


CodeMigrate::~CodeMigrate()
{
}

// Functions - Private

bool CodeMigrate::LoadDirectory()
{
	stringList IgnoreFolders = { ".", "..", "Resources" };

	std::function<void(std::string)> ScanDirectory = [&]( std::string SubPath ) -> void
	{
		DIR* Directory = opendir( std::string( SourcePath + SourceConst + SubPath ).c_str() );
		if( Directory != NULL )
		{
			dirent* Result;
			while( ( Result = readdir( Directory ) ) != NULL )
			{
				std::string FileName = std::string( Result->d_name );
				if( Result->d_type == DT_REG )
				{
					if( ( FileName.find( ".cpp" ) != FileName.npos ) || ( FileName.find( ".h" ) != FileName.npos ) )
					{
						FileList.push_back( SubPath + Result->d_name );
					}
				}
				else if( Result->d_type == DT_DIR )
				{
					bool bShouldIgnored = false;
					for( std::string& IgnoreFolder : IgnoreFolders )
					{
						if( IgnoreFolder == FileName )
						{
							bShouldIgnored = true;
						}
					}
					if( !bShouldIgnored )
					{
						ScanDirectory( SubPath + Result->d_name + "\\" );
					}
				}
			}
			closedir( Directory );
		}
		else
		{
			Warning( "Cant open directory : " + SourcePath + SubPath );
		}
	};

	FileList.clear();
	ScanDirectory( std::string("\\") );
	if( !FileList.empty() )
	{
		return true;
	}
	return false;
}

bool CodeMigrate::DetectClasses()
{
	auto SetData = []( const bool Switch, const std::string& Set, classMapData& Data ) -> void
	{
		if( Switch )
		{
			Data.second = Set;
		}
		else
		{
			Data.first = Set;
		}
	};
	auto InsertData = [&]( const bool IsHeader, const std::string& Class, const std::string& Path ) -> void
	{
		if( ClassesFound.find( Class ) == ClassesFound.end() )
		{
			classMapData Data;
			SetData( IsHeader, Path, Data );
			ClassesFound.insert( classMap::value_type( Class, Data ) );
		}
		else
		{
			SetData( IsHeader, Path, ClassesFound[Class] );
		}
	};
	
	if( !FileList.empty() )
	{
		while( !FileList.empty() )
		{
			std::string& element = FileList.back();
			std::string ClassName = element.substr( element.rfind("\\") + 1, element.rfind( "." ) - ( element.rfind("\\") + 1 ) );
			std::string Path = element.substr( 0, element.rfind( "\\" ) + 1 );
			if( ClassName == SourceName )
			{
				FileList.pop_back();
				continue;
			}
			InsertData( ( element.rfind( ".h" ) != element.npos )?( true ):( false ), ClassName, Path );
			FileList.pop_back();
		}
		if( ClassesCopySelected != nullptr )
		{
			delete[] ClassesCopySelected;
		}
		ClassesCopySelected = new bool[ClassesFound.size()]();
		for( unsigned int i = 0; i <= ClassesFound.size(); ++i )
		{
			ClassesCopySelected[i] = true;
		}
		return true;
	}
	return false;
}

bool CodeMigrate::UpdateClasses( std::ifstream& OrginalFile, std::ofstream& OutputFile ) const
{	
	if( OrginalFile.good() && OutputFile.good() )
	{
		for( std::string Line; std::getline( OrginalFile, Line, '\n' ); )
		{
			for( unsigned int i = 0; i < 2; ++i )
			{
				auto found = Line.find( SwapSource[i] );
				if( found != Line.npos )
				{
					Line.replace( found, SwapSource[i].size(), SwapTarget[i] );
				}
			}
			OutputFile << Line << std::endl;
		}
		return true;
	}
	return false;
}

bool CodeMigrate::CopyClasses() const
{
	if( !ClassesFound.empty() )
	{
		// check if project has code folder
		DIR* Directory = opendir( std::string( TargetPath + TargetConst ).c_str() );
		if( Directory != NULL )
		{
			unsigned int i = 0;
			closedir( Directory );

			for( auto& Element : ClassesFound )
			{
				if( ClassesCopySelected[i] )
				{
					auto CopyFile = [&]( const std::string& Element, const std::string& Class, const std::string& Extention ) -> bool
					{
						if( !Element.empty() )
						{
							std::string Path = TargetPath + TargetConst + Element;
							if( CreateDirectory( std::wstring( Path.begin(), Path.end() ).c_str(), NULL ) ||
								ERROR_ALREADY_EXISTS == GetLastError() )
							{
								std::ifstream Source( SourcePath + SourceConst + Element + Class + Extention, std::ios::binary );
								std::ofstream Target( Path + Class + Extention, std::ios::binary );
								return UpdateClasses( Source, Target );
							}
						}
						return false;
					};
					
					if( !CopyFile( Element.second.first, Element.first, ".cpp" ) )
					{
						Warning( "Copy & Update File failed : " + Element.first + ".cpp" );
					}
					if( !CopyFile( Element.second.second, Element.first, ".h" ) )
					{
						Warning( "Copy & Update File failed : " + Element.first + ".h" );
					}
				}
			}
			return true;
		}
		else
		{
			Error( "Cant find Source-Code Folder" );
		}
	}
	else
	{
		Error( "No Classes To Copy" );
	}
	return false;
}

// Functions - Helper
bool CodeMigrate::CheckPath( const std::string& Path ) const
{
	// TODO : Better UnrealProject detection
	if( Path.rfind( ".uproject" ) != Path.npos )
	{
		std::ifstream ProjectPath( Path );
		bool open = ProjectPath.good();
		ProjectPath.close();

		if( open )
		{
			return true;
		}
	}
	return false;
}

void CodeMigrate::Error( const std::string& Message ) const
{
	std::cout << std::endl << "Error : " << Message << std::endl;
	std::cout << "Action aborted" << std::endl;
}

void CodeMigrate::Warning( const std::string& Message ) const
{
	std::cout << "Warning : " << Message << std::endl;
}


void CodeMigrate::ParsePath( const std::string& Path, std::string& SavePath, std::string& SaveName, std::string& SaveConst )
{
	auto found = Path.rfind( "\\" );
	if( found != Path.npos )
	{
		SaveName = Path.substr( found + 1, Path.rfind( "." ) - ( found + 1 ) );
		SavePath = Path.substr( 0, found );
		SaveConst = "\\Source\\" + SaveName;
	}
	else
	{
		Error( "Cant Parse Path : " + Path );
	}
}

void CodeMigrate::Clear( bool full )
{
	SourcePath.clear();
	SourceName.clear();
	TargetPath.clear();
	TargetName.clear();
	if( full )
	{
		ClassesFound.clear();
		FileList.clear();
		if( ClassesCopySelected )
		{
			delete[] ClassesCopySelected;
		}
	}
}

void CodeMigrate::SetSawp()
{
	// Init
	if( SwapSource != nullptr ) delete[] SwapSource;
	if( SwapTarget != nullptr ) delete[] SwapTarget;
	SwapSource = new std::string[2];
	SwapTarget = new std::string[2];

	// Line 0 - PROJECTNAME_API
	SwapSource[0].resize( SourceName.size() );
	SwapTarget[0].resize( TargetName.size() );
	std::transform( SourceName.begin(), SourceName.end(), SwapSource[0].begin(), toupper );
	std::transform( TargetName.begin(), TargetName.end(), SwapTarget[0].begin(), toupper );
	SwapSource[0] += "_API";
	SwapTarget[0] += "_API";

	// Line 1 - ProjectName.h
	SwapSource[1] = SourceName + ".h";
	SwapTarget[1] = TargetName + ".h";
}

// Functions - Public
void CodeMigrate::LoadPath( const std::string& Path )
{
	if( CheckPath( Path ) )
	{
		ParsePath( Path, SourcePath, SourceName, SourceConst );

		if( LoadDirectory() )
		{
			if( DetectClasses() )
			{
				std::cout << "Loading Files was Successfull" << std::endl;
			}
			else
			{
				Clear();
				Error( "Error while ClassDetecting" );
			}
		}
		else
		{
			Clear();
			Error( "Cant load Directory" );
		}
	}
	else
	{
		Error( "Path unvalid" );
	}
}

void CodeMigrate::CopyTo( const std::string& Target )
{
	if( CheckPath( Target ) && !SourcePath.empty() )
	{
		ParsePath( Target, TargetPath, TargetName, TargetConst );

		SetSawp();
		
		if( CopyClasses() )
		{
			std::cout << "Code Migrate was Successfull" << std::endl;
		}
		else
		{
			Error( "Cant copy Classes" );
		}
	}
	else
	{
		Error( "Path unvalid" );
	}
}

CodeMigrate::stringList CodeMigrate::GetClasses() const
{
	stringList Return;

	if( !ClassesFound.empty() )
	{
		for( auto& Class : ClassesFound )
		{
			Return.push_back( Class.first );
		}
	}
	else
	{
		Warning( "No valid Classes found" );
	}
	return Return;
}

void CodeMigrate::SetClassSelection( const stringList& SelectedClasses )
{
	if( !ClassesFound.empty() )
	{
		// Set all to false/0
		for( unsigned int i = 0; i < ClassesFound.size(); ++i )
		{
			ClassesCopySelected[i] = false;
		}
		for( auto& element : SelectedClasses )
		{
			auto found = ClassesFound.find( element );
			if( found != ClassesFound.end() )
			{
				ClassesCopySelected[std::distance( ClassesFound.begin(), found )] = true;
			}
			else
			{
				Warning( "Class " + element + " is not available" );
			}
		}
	}
	else
	{
		Error( "No valid Classes founded" );
	}
}

