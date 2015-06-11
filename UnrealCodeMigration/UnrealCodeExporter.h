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
	enum Targets
	{
		TARGET,
		SOURCE
	};

private:
	stringListMap		ClassDependencies;
	stringListMap		TargetClassDependencies;
	stringPairMap		ClassFiles;
	stringPairMap		TargetClassFiles;
	std::vector<bool>	SelectedClasses;

	std::string			SourcePath;
	std::string			SourceProjectName;
	bool				SourceIsUnreal;
	std::string			TargetPath;
	std::string			TargetProjectName;
	bool				TargetIsUnreal;

	std::string			LastError;

public:
	bool			SetPath( const Targets Target, const std::string& Path ); // Path to Target .uproject File or Directory
	bool			Analyse( const Targets Target = SOURCE ); // Analyse Source Project
	bool			CopySelection(); // Copy Selection from Source Project ot Target Project or Directory
	
	bool			GetClassList( stringList& ClassList, const Targets Target = SOURCE );
	bool			SetClassSelection( const stringList& ClassSelectionList );
	bool			SetClassSelectionAll( const bool Value );

	bool			SwitchProjects(); // Switch Target -> Source, Source -> Target | WARNING : Selection lost

private: // Path has to be Valid
	void			SearchProjectClasses( const Targets Target = SOURCE );
	void			AnalyseClassDependency( const std::string& ClassName, const stringPair& FilePathPair, const std::string& Path, const std::string& ProjectName );
	void			SetLocation( const std::string& Path, std::string& SavePath, std::string& SaveName ) const; 

	bool			CheckPath( const std::string& Path, const bool CheckUproject = false ) const;
	bool			CheckValid( const Targets Target = SOURCE ) const;
	bool			CheckSourceFolder( const std::string& ProjectPath ) const;

	void			SetError( const std::string& Message, const std::string& Section );
	void			ClearData();
public:
					UnrealCodeExporter();
					~UnrealCodeExporter();

	std::string		GetLastError() const;
	void			ClearError();
};

