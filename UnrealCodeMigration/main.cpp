#include "UnrealCodeExporter.h"
#include "CodeMigrate.h"
#include <iostream>

void PostError( UnrealCodeExporter& Instance );

int main()
{
	CodeMigrate Instance;
	UnrealCodeExporter Exporter;

	// Test Setup
	if( !Exporter.SetSourcePath( "F:\\UnrealProjects\\UnrealDev\\SoundVisualization\\SoundVisualization.uproject" ) ) PostError( Exporter );
	if( !Exporter.SetTargetPath( "F:\\UnrealProjects\\UnrealDev\\PhysicalConstraint\\" ) ) PostError( Exporter );
	
	// Test Analyse
	if( !Exporter.AnalyseSource() ) PostError( Exporter );
	PostError( Exporter ); // Get Analyse Warnings
	
	// Test Class List
	UnrealCodeExporter::stringList ClassList;
	if( !Exporter.GetClassList( ClassList ) ) PostError( Exporter );
	std::cout << "Class list : " << std::endl;
	for( std::string& Entry : ClassList )
	{
		std::cout << Entry << std::endl;
	}

	// Test Class Selection
	UnrealCodeExporter::stringList ClassSelection;
	ClassSelection.push_back( "Flasch" ); // Produce Error
	if( !Exporter.SetClassSelection( ClassSelection ) ) PostError( Exporter );
	if( !Exporter.SetClassSelectionAll( false ) ) PostError( Exporter );
	if( !ClassList.empty() )
	{
		ClassSelection.clear();
		ClassSelection.push_back( ClassList[0] );
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
