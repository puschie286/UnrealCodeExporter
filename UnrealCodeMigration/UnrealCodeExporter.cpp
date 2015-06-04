#include "UnrealCodeExporter.h"

#include "dirent.h"

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
		SearchProjectClasses( SourcePath );

		for( auto& Element : ClassFiles )
		{
			AnalyseClassDependency( Element.first, Element.second );
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

void UnrealCodeExporter::SearchProjectClasses( const std::string& Path )
{

}

void UnrealCodeExporter::AnalyseClassDependency( const std::string& ClassName, const stringPair& FilePathPair )
{

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
