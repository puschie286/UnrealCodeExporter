#include "UnrealCodeExporter.h"
#include "CodeMigrate.h"
#include <iostream>

void PostError( UnrealCodeExporter& Instance );

int main()
{
	UnrealCodeExporter Exporter;

	// Test Setup
	if( !Exporter.SetPath( UnrealCodeExporter::SOURCE, "F:\\UnrealProjects\\UnrealDev\\SoundVisualization\\SoundVisualization.uproject") ) PostError( Exporter );
	// if( !Exporter.SetSourcePath( "F:\\UnrealProjects\\UnrealDev\\SoundVisualization\\" ) ) PostError( Exporter );
	if( !Exporter.SetPath( UnrealCodeExporter::TARGET, "F:\\UnrealProjects\\UnrealDev\\PhysicalConstraint\\PhysicalConstraint.uproject" ) ) PostError( Exporter );
	//if( !Exporter.SetTargetPath( "F:\\UnrealProjects\\UnrealDev\\PhysicalConstraint\\" ) ) PostError( Exporter );
	
	// Test Analyse
	if( !Exporter.Analyse( UnrealCodeExporter::SOURCE ) ) PostError( Exporter );
	if( !Exporter.Analyse( UnrealCodeExporter::TARGET ) ) PostError( Exporter );
	PostError( Exporter ); // Get Analyse Warnings
	
	// Test Class List
	UnrealCodeExporter::stringList ClassList, TargetClassList;
	if( !Exporter.GetClassList( ClassList, UnrealCodeExporter::SOURCE ) ) PostError( Exporter );
	if( !Exporter.GetClassList( TargetClassList, UnrealCodeExporter::TARGET ) ) PostError( Exporter );
	std::cout << "Class list : " << std::endl;
	for( std::string& Entry : ClassList )
	{
		std::cout << Entry << std::endl;
	}
	std::cout << "Target Class list : " << std::endl;
	for( std::string& Entry : TargetClassList )
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
		if( !Exporter.SetClassSelection( ClassSelection ) ) PostError( Exporter );
	}

	// Wait with close
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
