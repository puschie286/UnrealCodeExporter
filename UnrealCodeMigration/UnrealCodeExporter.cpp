#include "UnrealCodeExporter.h"

#include "dirent.h"

#include <functional>
#include <fstream>


bool UnrealCodeExporter::SetPath( const Targets Target, const std::string& Path )
{
	if( CheckPath( Path ) )
	{
		SetLocation( Path, ( Target ) ? ( SourcePath ) : ( TargetPath ), ( Target ) ? ( SourceProjectName ) : ( TargetProjectName ) );
		( Target ) ? ( SourceIsUnreal = false ) : ( TargetIsUnreal = false );
		return true;
	}
	else
	{
		SetError( "Path not Valid | " + Path, "SetPath" );
	}
	return false;
}

bool UnrealCodeExporter::Analyse( Targets Target )
{
	const std::string&	Path = ( Target ) ? ( SourcePath ) : ( TargetPath );
	const std::string&	ProjectName = ( Target ) ? ( SourceProjectName ) : ( TargetProjectName );
	bool&				IsUnreal = ( Target ) ? ( SourceIsUnreal ) : ( TargetIsUnreal );
	stringPairMap&		ClassMap = ( Target ) ? ( ClassFiles ) : ( TargetClassFiles );
	if( CheckValid( Target ) )
	{
		IsUnreal = CheckPath( Path + ProjectName + ".uproject", true );
		const std::string UnrealProjectPath = ( IsUnreal ) ? ( "Source\\" + ProjectName ) : ( "" );
		if( IsUnreal )
		{
			if( !CheckSourceFolder( Path ) )
			{
				SetError( "SourceProject has no Source-Directory", "AnalyseSource" );
				return false;
			}

		}
		SearchProjectClasses( Target );
		
		if( !IsUnreal )// Non-UnrealProject Import
		{
			// TODO : Verify for Non-UnrealProject Classes
		}
		else
		{
			SelectedClasses.resize( ClassMap.size(), true ); // Create Default Selection
		}

		for( auto& ClassIter : ClassMap )
		{
			AnalyseClassDependency( ClassIter.first, ClassIter.second, Path + UnrealProjectPath, ProjectName );
		}
		return true;
	}
	else
	{
		SetError( "SourceProject not Valid", "AnalyseSource" );
	}
	return false;
}

bool UnrealCodeExporter::CopySelection()
{
	if( CheckValid()  )
	{
		// TODO : Copy Selected Files
		// TODO : Manipulate Coppied Files

		if( TargetProjectName.empty() ) // Export to Non-UnrealProject
		{
			const std::string ProjectName = "PROJECTNAME.h";
			const std::string ProjectAPI = "PROJECTNAME_API";
			//const std::string UseModuleComment = "//MODULE : ";
		}
	}
	else
	{
		SetError( "SourceProject not Valid", "CopySelection" );
	}
	return false;
}

bool UnrealCodeExporter::GetClassList( stringList& ClassList, Targets Target )
{
	stringPairMap& ClassMap = ( Target ) ? ( ClassFiles ) : ( TargetClassFiles );
	if( CheckValid( Target ) )
	{
		ClassList.clear();
		for( auto& Element : ClassMap )
		{
			ClassList.push_back( Element.first );
		}
		return true;
	}
	else
	{
		SetError( "Project not Valid", "GetClassList" );
	}
	return false;
}

bool UnrealCodeExporter::SetClassSelection( const stringList& ClassSelectionList )
{
	if( CheckValid() )
	{
		if( !ClassFiles.empty() )
		{
			std::string ErrorString;
			for( const std::string& Class : ClassSelectionList )
			{
				auto Found = ClassFiles.find( Class );
				if( Found != ClassFiles.end() )
				{
					SelectedClasses[std::distance( ClassFiles.begin(), Found )] = true;
					// TODO : Check Dependency
				}
				else
				{
					ErrorString += Class + ", ";
				}
			}
			if( ErrorString.empty() )
			{
				return true;
			}
			else
			{
				SetError( "Following Classes was not founded : " + ErrorString.substr( 0, -2 ), "SetClassSelection" );
			}
		}
		else
		{
			SetError( "No Classes available", "SetClassSelection" );
		}
	}
	else
	{
		SetError( "SourceProject not valid", "SetClassSelection" );
	}
	return false;
}

bool UnrealCodeExporter::SetClassSelectionAll( const bool Value )
{
	if( CheckValid() )
	{
		if( !ClassFiles.empty() )
		{
			for( auto& Class : SelectedClasses )
			{
				Class = Value;
			}
		}
		else
		{
			SetError( "No Classes available", "SetClassSelection" );
		}
	}
	else
	{
		SetError( "SourceProject not valid", "SetClassSelection" );
	}
	return false;
}

bool UnrealCodeExporter::SwitchProjects()
{
	if( CheckValid( SOURCE ) && CheckValid( TARGET ) )
	{
		SelectedClasses.resize( TargetClassFiles.size() );
		bool TempIsUnreal = SourceIsUnreal;
		SourceIsUnreal = TargetIsUnreal;
		
		ClassFiles.swap( TargetClassFiles );
		ClassDependencies.swap( TargetClassDependencies );
		SourcePath.swap( TargetPath );
		SourceProjectName.swap( TargetProjectName );

		return true;
	}
	else
	{
		SetError( "One or Both Projects is not valid", "SwitchProjects" );
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

bool UnrealCodeExporter::CheckValid( Targets Target ) const
{
	return( Target )?( !SourcePath.empty() ):( !TargetPath.empty() );
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

void UnrealCodeExporter::SearchProjectClasses( const Targets Target )
{
	stringList IgnoreFolders;
	const std::string& ProjectPath = ( Target ) ? ( SourcePath ) : ( TargetPath );
	const std::string& ProjectName = ( Target ) ? ( SourceProjectName ) : ( TargetProjectName );
	stringPairMap& ClassMap = ( Target ) ? ( ClassFiles ) : ( TargetClassFiles );
	bool& IsUnrealProject = ( Target ) ? ( SourceIsUnreal ) : ( TargetIsUnreal );

	if( IsUnrealProject )
	{
		IgnoreFolders = { ".", "..", "Resources" };
	}
	else
	{
		IgnoreFolders = { ".", ".." };
	}
	
	auto FileFound = [&]( std::string& ClassName, bool IsHeader, std::string& Path ) -> void
	{
		if( ClassName != ProjectName )
		{
			if( IsHeader )
			{
				ClassMap[ClassName].first = Path;
			}
			else
			{
				ClassMap[ClassName].second = Path;
			}
		}
	};

	std::function<void( std::string )> ScanDirectory = [&]( std::string SubPath ) -> void
	{
		std::string UnrealProjectPath = ( IsUnrealProject ) ? ( "Source\\" + ProjectName ) : ( "" );
		DIR* Directory = opendir( ( ProjectPath + UnrealProjectPath + SubPath ).c_str() );
		if( Directory != NULL )
		{
			dirent* Result;
			while( ( Result = readdir( Directory ) ) != NULL )
			{
				std::string FileName = Result->d_name;
				if( Result->d_type == DT_REG ) // FILE
				{
					auto Found = FileName.rfind( ".cpp" );
					if( Found != FileName.npos )
					{
						FileFound( FileName.substr( 0, Found ), false, SubPath );
					}
					else if( ( Found = FileName.rfind( ".h" ) ) != FileName.npos )
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
				if( Line.rfind( ".generated" ) != Line.npos || Line.rfind( ">" ) != Line.npos )
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
