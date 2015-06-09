#pragma once

#include <string>
#include <vector>
#include <map>

class UnrealCodeExporter
{
public:
	typedef std::vector<std::string>			stringList;
	typedef std::pair<std::string, std::string> stringPair; // Header, Cpp
	typedef std::map<std::string, stringList>	stringListMap;
	typedef std::map<std::string, stringPair>	stringPairMap;

private:
	stringListMap		ClassDependencies;
	stringPairMap		ClassFiles;
	std::vector<bool>	SelectedClasses;

	std::string			SourcePath;
	std::string			SourceProjectName;
	std::string			TargetPath;
	std::string			TargetProjectName;

	std::string			LastError;

public:
	bool			SetSourcePath( const std::string& Path ); // Path to Source .uproject File
	bool			SetTargetPath( const std::string& Path ); // Path to Target .uproject File or Directory
	bool			AnalyseSource(); // Analyse Source Project
	bool			CopySelection(); // Copy Selection from Source Project ot Target Project or Directory
	
	bool			GetClassList( stringList& ClassList );
	bool			SetClassSelection( const std::vector<bool>& ClassSelectionList );

private: // Path has to be Valid
	void			SearchProjectClasses( const std::string& ProjectPath, const std::string& ProjectName, stringPairMap& ClassMap );
	void			AnalyseClassDependency( const std::string& ClassName, const stringPair& FilePathPair, const std::string& Path, const std::string& ProjectName );
	void			SetLocation( const std::string& Path, std::string& SavePath, std::string& SaveName ) const; 

	bool			CheckPath( const std::string& Path, const bool CheckUproject ) const;
	bool			CheckSourceValid() const;
	bool			CheckSourceFolder( const std::string& ProjectPath ) const;

	void			SetError( const std::string& Message, const std::string& Section );
	void			ClearData();
public:
					UnrealCodeExporter();
					~UnrealCodeExporter();

	std::string		GetLastError() const;
	void			ClearError();
};

