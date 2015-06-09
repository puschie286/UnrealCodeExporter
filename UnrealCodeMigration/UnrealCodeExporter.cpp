#include "UnrealCodeExporter.h"

#include "dirent.h"

#include <functional>
#include <fstream>


bool UnrealCodeExporter::SetSourcePath( const std::string& Path )
{
	if( CheckPath( Path, true ) )
	{
		SetLocation( Path, SourcePath, SourceProjectName );
		ClearData();
		return true;
	}
	else
	{
		SetError( "Uproject not found | " + Path, "SetSourcePath" );
	}
	return false;
}

bool UnrealCodeExporter::SetTargetPath( const std::string& Path )
{
	if( CheckPath( Path, false ) )
	{
		SetLocation( Path, TargetPath, TargetProjectName );
		return true;
	}
	else
	{
		SetError( "Unvalid Path | " + Path, "SetTargetPath" );
	}
	return false;
}

bool UnrealCodeExporter::AnalyseSource()
{
	if( CheckSourceValid() )
	{
		if( CheckSourceFolder( SourcePath ) )
		{
			SearchProjectClasses( SourcePath, SourceProjectName, ClassFiles );
			
			// create default selection
			SelectedClasses.clear();
			for( size_t i = 0; i < ClassFiles.size(); ++i )
			{
				SelectedClasses.push_back( true );
			}

			for( auto& Element : ClassFiles )
			{
				AnalyseClassDependency( Element.first, Element.second, SourcePath + "Source\\" + SourceProjectName, SourceProjectName );
			}
			return true;
		}
		else
		{
			SetError( "SourceProject has no Source-Directory", "AnalyseSource" );
		}
	}
	else
	{
		SetError( "SourceProject not Valid", "AnalyseSource" );
	}
	return false;
}

bool UnrealCodeExporter::CopySelection()
{
	if( CheckSourceValid()  )
	{
		// TODO : Copy Selected Files
		// TODO : Manipulate Coppied Files
	}
	else
	{
		SetError( "SourceProject not Valid", "CopySelection" );
	}
	return false;
}

bool UnrealCodeExporter::GetClassList( stringList& ClassList )
{
	if( CheckSourceValid() )
	{
		ClassList.clear();
		for( auto& Element : ClassFiles )
		{
			ClassList.push_back( Element.first );
		}
		return true;
	}
	else
	{
		SetError( "SourceProject not Valid", "GetClassList" );
	}
	return false;
}

bool UnrealCodeExporter::SetClassSelection( const std::vector<bool>& ClassSelectionList )
{
	if( CheckSourceValid() )
	{
		if( SelectedClasses.size() == ClassSelectionList.size() )
		{
			// TODO : Check SelectedClass Dependecy
			SelectedClasses = ClassSelectionList;
			return true;
		}
		else
		{
			SetError( "Unvalid ClassSelectionList Size", "SetClassSelection" );
		}
	}
	else
	{
		SetError( "SourceProject not valid", "SetClassSelection" );
	}
	
	return false;
}

void UnrealCodeExporter::SetError( const std::string& Message, const std::string& Section )
{
	LastError = "Error : ( " + Section + " ) : " + Message;
}

bool UnrealCodeExporter::CheckPath( const std::string& Path, const bool CheckUproject ) const
{
	bool Founded = false;

	if( CheckUproject )
	{
		if( Path.rfind( ".uproject" ) == Path.size() - 9 )
		{
			std::ifstream ProjectFile( Path );
			Founded = ProjectFile.good();
			ProjectFile.close();
		}
	}
	else
	{
		DIR* Directory = opendir( Path.substr( 0, Path.rfind( "\\" ) + 1 ).c_str() );
		if( Directory != NULL )
		{
			closedir( Directory );
			Founded = true;
		}
	}
	return Founded;
}

bool UnrealCodeExporter::CheckSourceValid() const
{
	return( !SourcePath.empty() && !SourceProjectName.empty() );
}

bool UnrealCodeExporter::CheckSourceFolder( const std::string& ProjectPath ) const
{
	DIR* Directory = opendir( ( ProjectPath + "Source\\" ).c_str() );
	if( Directory != NULL )
	{
		closedir( Directory );
		return true;
	}
	return false;
}

void UnrealCodeExporter::SearchProjectClasses( const std::string& ProjectPath, const std::string& ProjectName, stringPairMap& ClassMap )
{
	stringList IgnoreFolders = { ".", "..", "Resources" };
	
	auto FileFound = [&]( std::string& ClassName, bool IsHeader, std::string& Path ) -> void
	{
		if( IsHeader )
		{
			ClassMap[ClassName].first = Path;
		}
		else
		{
			ClassMap[ClassName].second = Path;
		}
	};

	std::function<void( std::string )> ScanDirectory = [&]( std::string SubPath ) -> void
	{
		DIR* Directory = opendir( ( ProjectPath + "Source\\" + ProjectName + SubPath ).c_str() );
		if( Directory != NULL )
		{
			dirent* Result;
			while( ( Result = readdir( Directory ) ) != NULL )
			{
				std::string FileName = Result->d_name;
				if( Result->d_type == DT_REG ) // FILE
				{
					auto Found = FileName.rfind( ".cpp" );
					if( Found != FileName.npos && FileName != ( ProjectName + ".cpp" ) )
					{
						FileFound( FileName.substr( 0, Found ), false, SubPath );
					}
					else if( ( Found = FileName.rfind( ".h" ) ) != FileName.npos && FileName != ( ProjectName + ".h" ) )
					{
						FileFound( FileName.substr( 0, Found ), true, SubPath );
					}
				}
				else if( Result->d_type == DT_DIR ) // DIRECTORY
				{
					bool test = true;
					for( std::string& Ignore : IgnoreFolders )
					{
						if( Ignore == FileName )
						{
							test = false;
						}
					}
					if( test )
					{
						ScanDirectory( SubPath + FileName + "\\" );
					}
				}
			}
			closedir( Directory );
		}
		else
		{
			SetError( "Problem while DirectoryScan", "SearchProjectClasses" );
		}
	};

	ScanDirectory( "\\" );
}

void UnrealCodeExporter::AnalyseClassDependency( const std::string& ClassName, const stringPair& FilePathPair, const std::string& Path, const std::string& ProjectName )
{
	auto CheckFile = [&]( std::ifstream& FileStream ) -> void
	{
		std::string Line;
		while( std::getline( FileStream, Line ) )
		{
			if( Line.find( "#include" ) != Line.npos )
			{
				if( Line.rfind( ".generated" ) != Line.npos )
				{
					continue;
				}

				auto Find0 = Line.find( "\"" ), Find1 = Line.rfind( "\"" ) - 2;
				if( Line.rfind( "/" ) != Line.npos )
				{
					Find0 = Line.rfind( "/" ) + 1;
				}
				std::string& Include = Line.substr( Find0, Find1 - Find0 );
				if( ClassFiles.find( Include ) != ClassFiles.end() )
				{
					ClassDependencies[ClassName].push_back( Include );
				}
			}
		}
	};

	std::ifstream FileReader( Path + FilePathPair.first + ClassName + ".h", std::ifstream::in );
	if( FileReader.good() )
	{
		CheckFile( FileReader );
	}
	FileReader.close();
	FileReader.open( Path + FilePathPair.second + ClassName + ".cpp", std::ifstream::in );
	if( FileReader.good() )
	{
		CheckFile( FileReader );
	}
	FileReader.close();
}

void UnrealCodeExporter::SetLocation( const std::string& Path, std::string& SavePath, std::string& SaveName ) const
{
	size_t FoundExtention = Path.rfind( "." );
	size_t FoundSlash = Path.rfind( "\\" ) + 1;

	SavePath = Path.substr( 0, FoundSlash );
	SaveName = ( FoundExtention != std::string::npos ) ? ( Path.substr( FoundSlash, FoundExtention - FoundSlash ) ) : ( "" );
}

void UnrealCodeExporter::ClearData()
{
	ClassDependencies.clear();
	ClassFiles.clear();
	SelectedClasses.clear();
}

UnrealCodeExporter::UnrealCodeExporter()
{

}

UnrealCodeExporter::~UnrealCodeExporter()
{

}

std::string UnrealCodeExporter::GetLastError() const
{
	return LastError;
}

void UnrealCodeExporter::ClearError()
{
	LastError.clear();
}
