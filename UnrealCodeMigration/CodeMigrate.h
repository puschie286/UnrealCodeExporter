#include <string>
#include <map>
#include <vector>

class CodeMigrate
{
// Variables
private:
	typedef std::pair<std::string, std::string> classMapData;
	typedef std::map<std::string, classMapData> classMap; // ClassName, Pair<CppPath,HeaderPath>
	typedef std::vector<std::string> stringList;

	std::string		SourcePath;
	std::string		SourceName;
	std::string		SourceConst;
	std::string		TargetPath;
	std::string		TargetName;
	std::string		TargetConst;

	std::string*	SwapSource = nullptr;
	std::string*	SwapTarget = nullptr;

	stringList		FileList;
	classMap		ClassesFound;
	bool*			ClassesCopySelected	= nullptr;

// Functions
private:
	bool LoadDirectory(); // From SourcePath
	bool DetectClasses(); // From SourcePath
	bool UpdateClasses( std::ifstream& OrginalFile, std::ofstream& OutputFile ) const; // From SourcePath for TargetPath
	bool CopyClasses() const; // To TargetPath

// Helper
	inline bool CheckPath( const std::string& Path ) const; // Check if Path is valid Unreal Project directory
	inline void Error( const std::string& Message ) const;
	inline void Warning( const std::string& Message ) const;
	inline void ParsePath( const std::string& Path, std::string& SavePath, std::string& SaveName, std::string& SaveConst );
	inline void Clear( bool full = true );
	inline void SetSawp();

public:
	void LoadPath( const std::string& Path );
	void CopyTo( const std::string& Target );

	stringList GetClasses() const;
	void SetClassSelection( const stringList& SelectedClasses );

// Con- & Destructor
public:
	CodeMigrate();
	~CodeMigrate();
};
