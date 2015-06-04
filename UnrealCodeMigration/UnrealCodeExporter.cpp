#include "UnrealCodeExporter.h"

#include "dirent.h"

#include <fstream>


bool UnrealCodeExporter::SetSourcePath( const std::string& Path )
{
	if( CheckPath( Path, true ) )
	{
		SetLocation( Path, SourcePath, SourceProjectName );
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

void UnrealCodeExporter::SetLocation( const std::string& Path, std::string& SavePath, std::string& SaveName ) const
{
	size_t FoundExtention = Path.rfind( "." );
	size_t FoundSlash = Path.rfind( "\\" ) + 1;

	SavePath = Path.substr( 0, FoundSlash );
	SaveName = ( FoundExtention != std::string::npos ) ? ( Path.substr( FoundSlash, FoundExtention - FoundSlash ) ) : ( "" );
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
