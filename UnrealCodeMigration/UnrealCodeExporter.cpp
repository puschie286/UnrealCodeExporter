#include "UnrealCodeExporter.h"

#include "dirent.h"

#include <functional>
#include <fstream>
#include <algorithm>


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
		
		// TODO : Verify for Non-UnrealProject Classes
		
		if( Target == SOURCE )
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
	if( CheckValid( TARGET ) && CheckValid( SOURCE ) )
	{
		if( !SelectionEmty() )
		{
			std::ifstream SourceFile;
			std::ofstream TargetFile;
			const std::string SourcePathConst = ( SourceIsUnreal ) ? ( SourcePath + "Source\\" + SourceProjectName ) : ( SourcePath );
			const std::string TargetPathConst = ( TargetIsUnreal ) ? ( TargetPath + "Source\\" + TargetProjectName ) : ( TargetPath );
			auto ClassIter = ClassFiles.begin();
			std::string ErrorFiles;

			std::string SourceSearch[2];
			std::string TargetReplace[2];

			if( SourceIsUnreal )
			{
				SourceSearch[0] = SourceProjectName + ".h";
				SourceSearch[1].resize( SourceProjectName.size() );
				std::transform( SourceProjectName.begin(), SourceProjectName.end(), SourceSearch[1].begin(), toupper );
				SourceSearch[1] += "_API";
			}
			else
			{
				SourceSearch[0] = "PROJECTNAME.h";
				SourceSearch[1] = "PROJECTNAME_API";
			}

			if( TargetIsUnreal )
			{
				TargetReplace[0] = TargetProjectName + ".h";
				TargetReplace[1].resize( TargetProjectName.size() );
				std::transform( TargetProjectName.begin(), TargetProjectName.end(), TargetReplace[1].begin(), toupper );
				TargetReplace[1] += "_API";
			}
			else
			{
				TargetReplace[0] = "PROJECTNAME.h";
				TargetReplace[1] = "PROJECTNAME_API";
			}

			auto CopyFile = [&]( const std::string& FilePath, bool isHeader ) -> void
			{
				SourceFile.open( SourcePathConst + FilePath, std::ifstream::in | std::ifstream::binary );
				TargetFile.open( TargetPathConst + FilePath, std::ofstream::out | std::ofstream::binary );

				if( SourceFile.is_open() && TargetFile.is_open() )
				{
					if( !SourceIsUnreal && !TargetIsUnreal )
					{
						TargetFile << SourceFile.rdbuf();
					}
					else
					{
						bool Skip = false;
						for( std::string Line; std::getline( SourceFile, Line, '\n' ); )
						{
							if( !Skip )
							{
								if( isHeader )
								{
									size_t Found = Line.find( SourceSearch[1] );
									if( Found != Line.npos )
									{
										Line.replace( Found, SourceSearch[1].size(), TargetReplace[1] );
										Skip = true;
									}
								}
								else
								{
									size_t Found = Line.find( SourceSearch[0] );
									if( Found != Line.npos )
									{
										Line.replace( Found, SourceSearch[0].size(), TargetReplace[0] );
										Skip = true;
									}
								}
							}
							TargetFile << Line << std::endl;
						}
					}
				}
				else
				{
					ErrorFiles += FilePath + ", ";
				}

				SourceFile.close();
				TargetFile.close();
			};
			
			for( const bool ShouldCopy : SelectedClasses )
			{
				if( ShouldCopy )
				{
					// TODO : Check for File override
					CopyFile( ClassIter->second.first + ClassIter->first + ".h", true );
					CopyFile( ClassIter->second.second + ClassIter->first + ".cpp", false );
				}
				if( ClassIter != ClassFiles.end() )
				{
					ClassIter++;
				}
			}

			if( ErrorFiles.empty() )
			{
				return true;
			}
			else
			{
				// TODO : Clear after Copy Failed
				SetError( "Error with following Files : " + ErrorFiles.substr( 0, -2 ), "CopySelection" );
			}
		}
		else
		{
			SetError( "No Classes found or selected", "CopySelection" );
		}
	}
	else
	{
		SetError( "Projects not Valid", "CopySelection" );
	}
	return false;
}

bool UnrealCodeExporter::GetClassList( stringList& ClassList, Targets Target )
{
	const stringPairMap& ClassMap = ( Target ) ? ( ClassFiles ) : ( TargetClassFiles );
	if( CheckValid( Target ) )
	{
		ClassList.clear();
		for( const auto& Element : ClassMap )
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
			std::function<void( const std::string& ClassName )> CheckDependency = [&]( const std::string& ClassName ) -> void
			{
				if( !ClassDependencies[ClassName].empty() )
				{
					for( std::string& DepClass : ClassDependencies[ClassName] )
					{
						auto Found = ClassFiles.find( DepClass );
						if( Found != ClassFiles.end() )
						{
							size_t iter = std::distance( ClassFiles.begin(), Found );
							if( !SelectedClasses[iter] )
							{
								SelectedClasses[iter] = true;
								CheckDependency( DepClass );
							}
						}
					}
				}
			};
			std::string ErrorString;
			for( const std::string& Class : ClassSelectionList )
			{
				auto Found = ClassFiles.find( Class );
				if( Found != ClassFiles.end() )
				{
					size_t iter = std::distance( ClassFiles.begin(), Found );
					if( !SelectedClasses[iter] )
					{
						SelectedClasses[iter] = true;
						CheckDependency( Class );
					}
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

bool UnrealCodeExporter::SelectionEmty() const
{
	if( !ClassFiles.empty() )
	{
		for( const bool Value : SelectedClasses )
		{
			if( Value ) return false;
		}
	}
	return true;
}

void UnrealCodeExporter::SearchProjectClasses( const Targets Target )
{
	stringList IgnoreFolders;
	const std::string& ProjectPath = ( Target ) ? ( SourcePath ) : ( TargetPath );
	const std::string& ProjectName = ( Target ) ? ( SourceProjectName ) : ( TargetProjectName );
	stringPairMap& ClassMap = ( Target ) ? ( ClassFiles ) : ( TargetClassFiles );
	bool& IsUnrealProject = ( Target ) ? ( SourceIsUnreal ) : ( TargetIsUnreal );
	const std::string UnrealProjectPath = ( IsUnrealProject ) ? ( "Source\\" + ProjectName ) : ( "" );
	if( IsUnrealProject )
	{
		IgnoreFolders = { ".", "..", "Resources" };
	}
	else
	{
		IgnoreFolders = { ".", ".." };
	}
	
	auto FileFound = [&]( const std::string& ClassName, const bool IsHeader, const std::string& Path ) -> void
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

	std::function<void( const std::string& )> ScanDirectory = [&]( const std::string& SubPath ) -> void
	{
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
					for( const std::string& Ignore : IgnoreFolders )
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

				auto Find0 = Line.find( "\"" ) + 1, Find1 = Line.rfind( "\"" ) - 2;
				if( Line.rfind( "/" ) != Line.npos )
				{
					Find0 = Line.rfind( "/" ) + 1;
				}
				std::string& Include = Line.substr( Find0, Find1 - Find0 );
				if( Include == ClassName )
				{
					continue;
				}
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
	const size_t FoundExtention = Path.rfind( "." );
	const size_t FoundSlash = Path.rfind( "\\" ) + 1;

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
