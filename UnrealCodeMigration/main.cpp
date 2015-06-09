#include "UnrealCodeExporter.h"
#include "CodeMigrate.h"
#include <iostream>

void PostError( UnrealCodeExporter& Instance );

int main()
{
	CodeMigrate Instance;
	UnrealCodeExporter Exporter;

	UnrealCodeExporter::stringList ClassList;

	if( !Exporter.SetSourcePath( "F:\\UnrealProjects\\UnrealDev\\SoundVisualization\\SoundVisualization.uproject" ) ) PostError( Exporter );
	if( !Exporter.SetTargetPath( "F:\\UnrealProjects\\UnrealDev\\PhysicalConstraint\\" ) ) PostError( Exporter );
	if( !Exporter.AnalyseSource() ) PostError( Exporter );
	PostError( Exporter );
	if( !Exporter.GetClassList( ClassList ) ) PostError( Exporter );
	
	std::cout << "Class list : " << std::endl;
	for( std::string& Entry : ClassList )
	{
		std::cout << Entry << std::endl;
	}
	
	/* OLD Version
	Instance.LoadPath( "F:\\UnrealProjects\\DevUnreal\\PhysicalConstraint\\PhysicalConstraint.uproject" );
	Instance.CopyTo( "F:\\UnrealProjects\\DevUnreal\\SoundVisualization\\SoundVisualization.uproject" );
	*/

	int i;
	std::cin >> i;
}

void PostError( UnrealCodeExporter& Instance )
{
	if( Instance.GetLastError() != "" )
	{
		std::cout << Instance.GetLastError() << std::endl;
		Instance.ClearError();
	}
}
