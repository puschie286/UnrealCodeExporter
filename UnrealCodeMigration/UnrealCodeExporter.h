#pragma once

#include <string>
#include <vector>
#include <map>

class UnrealCodeExporter
{
	typedef unsigned short						uint16;
	typedef std::vector<std::string>			stringList;
	typedef std::pair<std::string, std::string> stringPair;
	typedef std::map<std::string, stringList>	stringListMap;
	typedef std::map<std::string, stringPair>	stringPairMap;

	stringListMap		ClassDependencies;
	stringPairMap		ClassFiles;
	std::vector<uint16>	SelectedClasses;

	std::string			SourcePath;
	std::string			SourceProjectName;
	std::string			TargetPath;
	std::string			TargetProjectName;

	std::string			LastError;

public:
	bool			SetSourcePath( const std::string& Path ); // Path to Source .uproject File
	bool			SetTargetPath( const std::string& Path ); // Path to Target .uproject File or Directory

private:
	void			SetError( const std::string& Message, const std::string& Section );
	bool			CheckPath( const std::string& Path, const bool CheckUproject ) const;
	void			SetLocation( const std::string& Path, std::string& SavePath, std::string& SaveName ) const; // Path has to be Valid

public:
					UnrealCodeExporter();
					~UnrealCodeExporter();

	std::string		GetLastError() const;
};

