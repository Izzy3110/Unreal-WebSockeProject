/**
 * InstaMATGraphSceneOutput.cpp (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATGraphSceneOutput.cpp
 * @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#include "ParameterObjects/InstaMATGraphSceneOutput.h"
#include "ParameterObjects/InstaMATGraphSceneOutputNode.h"

void UInstaMATGraphSceneOutput::SpawnActorsInScene()
{
	if (Root == nullptr)
	{
		UE_LOG(LogInstaMAT, Warning, TEXT("InstaMAT: Scene is empty."));
		return;
	}

	static const uint32 kMaximumDepth = 512u;

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.TransformScaleMethod = ESpawnActorScaleMethod::MultiplyWithRoot;
	SpawnParameters.bCreateActorPackage = true;
	AActor* const RootActor = GWorld->SpawnActor<AActor>(SpawnParameters);

	if (RootActor == nullptr)
	{
		UE_LOG(LogInstaMAT, Warning, TEXT("InstaMAT: Failed to spawn root actor."));
		return;
	}

	RootActor->SetActorLabel(FString::Format(TEXT("InstaMAT_{0}_Root"), { OutputName }));
	RootActor->SetActorTransform(Root->Transform);

	// The fnSpawnInSceneRecursive lambda traverses all nodes and spawns the actors.
	TFunction<void(AActor* const, UInstaMATGraphSceneOutputNode* const, UInstaMATGraphSceneOutputNode* const, const uint32)> fnSpawnInSceneRecursive;
	fnSpawnInSceneRecursive = [&fnSpawnInSceneRecursive, &SpawnParameters](AActor* const ParentActor, UInstaMATGraphSceneOutputNode* const Parent, UInstaMATGraphSceneOutputNode* const CurrentNode, const uint32 Depth)
	{
		check(Parent != nullptr);
		check(ParentActor != nullptr);
		
		if (Depth > kMaximumDepth)
		{
			UE_LOG(LogInstaMAT, Warning, TEXT("InstaMAT: Reached maximum scene depth."));
			return;
		}

		AActor* Actor = nullptr;
		
		// If a mesh is available, we need to spawn a staticmesh actor.
		if (CurrentNode->Mesh != nullptr)
		{
			Actor = GWorld->SpawnActor<AStaticMeshActor>(SpawnParameters);
			if (Actor != nullptr)
			{
				Cast<AStaticMeshActor>(Actor)->GetStaticMeshComponent()->SetStaticMesh(CurrentNode->Mesh);
			}
		}
		else
		{
			Actor = GWorld->SpawnActor<AActor>(SpawnParameters);
		}

		check(Actor != nullptr);
		if (Actor == nullptr)
		{
			UE_LOG(LogInstaMAT, Warning, TEXT("InstaMAT: Failed to spawn actor."));
			return;
		}

		// Apply information.
		Actor->SetActorLabel(*CurrentNode->NodeName);
		Actor->AttachToActor(ParentActor, FAttachmentTransformRules(EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, /*bInWeldSimulatedBodies*/ true));
		Actor->SetActorRelativeTransform(CurrentNode->Transform);

		// Traverse children.
		for (UInstaMATGraphSceneOutputNode* const ChildNode : CurrentNode->Children)
		{
			check(ChildNode != nullptr);
			if (ChildNode == nullptr)
				continue;

			fnSpawnInSceneRecursive(Actor, CurrentNode, ChildNode, Depth + 1u);
		}
	};

	for (UInstaMATGraphSceneOutputNode* const Child : Root->Children)
	{
		fnSpawnInSceneRecursive(RootActor, Root, Child, 0u);
	}
}