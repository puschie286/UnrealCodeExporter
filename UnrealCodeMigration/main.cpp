#include "UnrealCodeExporter.h"
#include "CodeMigrate.h"
#include <iostream>

int main()
{
	CodeMigrate Instance;
	UnrealCodeExporter Exporter;

	Exporter.SetSourcePath( "F:\\UnrealProjects\\DevUnreal\\SoundVisualization\\SoundVisualization.uproject" );
	Exporter.SetTargetPath( "F:\\UnrealProjects\\DevUnreal\\PhysicalConstraint\\" );

	/*
	Instance.LoadPath( "F:\\UnrealProjects\\DevUnreal\\PhysicalConstraint\\PhysicalConstraint.uproject" );
	Instance.CopyTo( "F:\\UnrealProjects\\DevUnreal\\SoundVisualization\\SoundVisualization.uproject" );
	*/

	int i;
	std::cin >> i;
}
