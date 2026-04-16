// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <vector>

#include "CoreMinimal.h"
#include "CellForArray.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "LabGenerator.generated.h"

UENUM(BlueprintType)
enum class EGenerationType : uint8
{
	FirstInList,
	LastInList,
	RandomInList,
	FindingTreasure
};

UCLASS()
class LABGENERATION_API ALabGenerator : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ALabGenerator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UBoxComponent* BoundingBox;
	
	float CellSize;
	
	FVector Edge; 
	
	std::vector<std::vector<CellForArray>> Cells;
	std::vector<std::vector<ACell*>> CellsOnLevel;
	
	std::pair<int, int> StartIdx;
	
	std::vector<AActor*> Treasures;
	
	float CalculateDistance(std::pair<int, int> x1, std::pair<int, int> x2);
	
	void ClearCells();
	
	void RemoveWall(std::pair<int, int> x1, std::pair<int, int> x2);
	
	void FirstInList();
	void LastInList();
	void RandomInList();
	void FindingTreasure();
	
	void SpawnCells();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	std::pair<int, int> LocationToIdx(FVector Location);
	FVector IdxToLocation(std::pair<int, int> idx);
	
	UFUNCTION(CallInEditor, Category = "Generation")
	void Generate();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	int32 Resolution = 100;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	EGenerationType GenerationType = EGenerationType::FirstInList;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	TArray<TSubclassOf<ACell>> CellComparison;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	AActor* Start;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	TSubclassOf<AActor> TreasureClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	float MinDistanceBetweenTreasuresInCells;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	int TreasureNum;
};