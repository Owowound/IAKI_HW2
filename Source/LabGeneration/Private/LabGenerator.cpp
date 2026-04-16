// Fill out your copyright notice in the Description page of Project Settings.

#include "LabGenerator.h"

#include <queue>
#include <set>


// Sets default values
ALabGenerator::ALabGenerator()
{
	PrimaryActorTick.bCanEverTick = false;
	
	BoundingBox = CreateDefaultSubobject<UBoxComponent>(TEXT("BoundingBox"));
	RootComponent = BoundingBox;
	BoundingBox->SetBoxExtent(FVector(50.0f, 50.0f, 50.0f));
	BoundingBox->SetCollisionProfileName(TEXT("BlockAll"));
}

// Called when the game starts or when spawned
void ALabGenerator::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ALabGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ALabGenerator::Generate()
{
	FVector extend = BoundingBox->GetScaledBoxExtent();
	FVector BoxSize = extend * 2;
	
	float Size = BoxSize.X;
	
	CellSize = Size / Resolution;
	Edge = GetActorLocation() - extend + FVector(CellSize / 2, CellSize / 2, 0);
	
	DrawDebugSphere(
		GetWorld(),
		Edge,
		CellSize / 2,
		12,
		FColor::Red, 
		false,
		100);
	
	ClearCells();
	Cells = std::vector<std::vector<CellForArray>>(Resolution, std::vector<CellForArray>(Resolution, CellForArray()));
	CellsOnLevel = std::vector<std::vector<ACell*>>(Resolution, std::vector<ACell*>(Resolution, nullptr));
	
	StartIdx = LocationToIdx(Start->GetActorLocation());
	auto StartLocation = IdxToLocation(StartIdx);
	
	
	UE_LOG(LogTemp, Log, TEXT("Начинаем генерацию по алгоритму %s"), *UEnum::GetValueAsString(GenerationType));
	switch (GenerationType)
	{
	case EGenerationType::FirstInList:
		FirstInList();
		break;
	case EGenerationType::LastInList:
		LastInList();
		break;
	case EGenerationType::RandomInList:
		RandomInList();
		break;	
	case EGenerationType::FindingTreasure:
		FindingTreasure();
		break;
	}
	
	UE_LOG(LogTemp, Log, TEXT("Генерация окончена, спавним лабиринт"));
	SpawnCells();
}

std::pair<int, int> ALabGenerator::LocationToIdx(FVector Location)
{
	std::pair<int, int> result;
	
	FVector RelativeLocation = Location - Edge;
	
	int i = static_cast<int>(RelativeLocation.X) / static_cast<int>(CellSize);
	int j = static_cast<int>(RelativeLocation.Y) / static_cast<int>(CellSize);
    
	i = std::max(0, std::min(i, Resolution - 1));
	j = std::max(0, std::min(j, Resolution - 1)); 
	
	return {i, j};
}

FVector ALabGenerator::IdxToLocation(std::pair<int, int> idx)
{
	FVector result;
	
	result = FVector(Edge.X + static_cast<double>(idx.first) * CellSize, Edge.Y + static_cast<double>(idx.second) * CellSize, GetActorLocation().Z);
	
	return result;
}

void ALabGenerator::ClearCells()
{
	Cells = std::vector<std::vector<CellForArray>>(Resolution, std::vector<CellForArray>(Resolution, CellForArray()));
	
	for (int i = 0; i < CellsOnLevel.size(); i++)
	{
		for (int j = 0; j < CellsOnLevel.size(); j++)
		{
			if (CellsOnLevel[i][j])
			{
				CellsOnLevel[i][j]->Destroy();
				CellsOnLevel[i][j] = nullptr;
			}
		}
	}
	CellsOnLevel = std::vector<std::vector<ACell*>>(Resolution, std::vector<ACell*>(Resolution, nullptr));
	
	for (int i = 0; i < Treasures.size(); i++)
	{
		if (Treasures[i])
		{
			Treasures[i]->Destroy();
			Treasures[i] = nullptr;
		}
	}
	Treasures.clear();
}

void ALabGenerator::RemoveWall(std::pair<int, int> x1, std::pair<int, int> x2)
{
	auto [x, y] = x1;
	auto [nx, ny] = x2;
	
	if (nx == x + 1 && ny == y) // Right
	{
		Cells[nx][ny].HasLeftWall = false;
	}
	if (x == nx + 1 && ny == y) // Left
	{
		Cells[x][y].HasLeftWall = false;
	}
	if (nx == x && ny == y + 1) // Down
	{
		Cells[nx][ny].HasUpperWall = false;
	}
	if (x == nx && y == ny + 1) // Up
	{
		Cells[x][y].HasUpperWall = false;
	}
	
	return;
}

void ALabGenerator::SpawnCells()
{
	for (int i = 0; i < Resolution; i++)
	{
		for (int j = 0; j < Resolution; j++)
		{
			int mask = Cells[i][j].GetMask();
			
			auto Temp = CellComparison[mask];
			
			FVector SpawnLocation = IdxToLocation({i, j});
			
			ACell* NewCell = GetWorld()->SpawnActor<ACell>(Temp, SpawnLocation, FRotator::ZeroRotator);
			CellsOnLevel[i][j] = NewCell;
			
			float DefaultSize = NewCell->GetDefaultCellSize();
			float Scale = CellSize / DefaultSize;
			auto CurrScale = NewCell->GetActorScale3D();
			NewCell->SetActorScale3D(FVector(Scale, Scale, CurrScale.Z * Scale));
		}
	}
}

void ALabGenerator::FirstInList()
{
	std::queue<std::pair<int, int>> queue;
	std::vector<std::vector<bool>> isVisited(Resolution, std::vector<bool>(Resolution, false));
	
	queue.push(StartIdx);
	
	while (!queue.empty())
	{
		auto [x, y] = queue.front();
		
		std::vector<std::pair<int, int>> neighbors({{x + 1, y}, {x - 1, y}, {x, y + 1}, {x, y - 1}});
		std::vector<std::pair<int, int>> actualNeighbors;
		
		for (const auto& neighbor : neighbors)
		{
			auto [nx, ny] = neighbor;
			if (nx < 0 || nx >= Resolution || ny < 0 || ny >= Resolution
				|| isVisited[nx][ny])
			{
				continue;
			}
			actualNeighbors.push_back(neighbor);
		}
		
		if (!actualNeighbors.empty())
		{
			int randIdx = FMath::RandRange(0, actualNeighbors.size() - 1);
			auto [nx, ny] = actualNeighbors[randIdx];
			
			isVisited[nx][ny] = true;
			queue.push({nx, ny});
			RemoveWall({x, y}, {nx, ny});
		}
		else
		{
			queue.pop();
		}
	}
	return;
}

void ALabGenerator::LastInList()
{
	std::vector<std::pair<int, int>> queue;
	std::vector<std::vector<bool>> isVisited(Resolution, std::vector<bool>(Resolution, false));
	
	queue.push_back(StartIdx);
	isVisited[StartIdx.first][StartIdx.second] = true;
	
	while (!queue.empty())
	{
		auto [x, y] = queue.back();
		
		std::vector<std::pair<int, int>> neighbors({{x + 1, y}, {x - 1, y}, {x, y + 1}, {x, y - 1}});
		std::vector<std::pair<int, int>> actualNeighbors;
		
		for (const auto& neighbor : neighbors)
		{
			auto [nx, ny] = neighbor;
			if (nx < 0 || nx >= Resolution || ny < 0 || ny >= Resolution
				|| isVisited[nx][ny])
			{
				continue;
			}
			actualNeighbors.push_back(neighbor);
		}
		
		if (!actualNeighbors.empty())
		{
			int randIdx = FMath::RandRange(0, actualNeighbors.size() - 1);
			auto [nx, ny] = actualNeighbors[randIdx];
			
			isVisited[nx][ny] = true;
			queue.push_back({nx, ny});
			RemoveWall({x, y}, {nx, ny});
		}
		else
		{
			queue.pop_back();
		}
	}
	return;
}

void ALabGenerator::RandomInList()
{
	std::vector<std::pair<int, int>> queue;
	std::vector<std::vector<bool>> isVisited(Resolution, std::vector<bool>(Resolution, false));
	
	queue.push_back(StartIdx);
	isVisited[StartIdx.first][StartIdx.second] = true;
	
	while (!queue.empty())
	{
		int randIdx1 = FMath::RandRange(0, queue.size() - 1);
		auto [x, y] = queue[randIdx1];
		
		std::vector<std::pair<int, int>> neighbors({{x + 1, y}, {x - 1, y}, {x, y + 1}, {x, y - 1}});
		std::vector<std::pair<int, int>> actualNeighbors;
		
		for (const auto& neighbor : neighbors)
		{
			auto [nx, ny] = neighbor;
			if (nx < 0 || nx >= Resolution || ny < 0 || ny >= Resolution
				|| isVisited[nx][ny])
			{
				continue;
			}
			actualNeighbors.push_back(neighbor);
		}
		
		if (!actualNeighbors.empty())
		{
			int randIdx = FMath::RandRange(0, actualNeighbors.size() - 1);
			auto [nx, ny] = actualNeighbors[randIdx];
			
			isVisited[nx][ny] = true;
			queue.push_back({nx, ny});
			RemoveWall({x, y}, {nx, ny});
		}
		else
		{
			queue.erase(queue.begin() + randIdx1);
		}
	}
	return;
	return;
}

void ALabGenerator::FindingTreasure()
{
	UE_LOG(LogTemp, Log, TEXT("Генерация кладов"));
	int treasureCounter = 0;
	while (treasureCounter < TreasureNum)
	{
		int x = FMath::RandRange(0, Resolution - 1);
		int y = FMath::RandRange(0, Resolution - 1);
		
		bool bCanBeSpawned = true;
		for (int j = 0; j < Treasures.size(); j++)
		{
			float distance = CalculateDistance({x, y}, LocationToIdx(Treasures[j]->GetActorLocation()));
			if (distance < MinDistanceBetweenTreasuresInCells)
			{
				bCanBeSpawned = false;
			}
		}
		
		if (bCanBeSpawned)
		{
			treasureCounter++;
			FVector SpawnLocation = IdxToLocation({x, y});
			AActor* NewTreasure = GetWorld()->SpawnActor<AActor>(TreasureClass, SpawnLocation, FRotator::ZeroRotator);
			Treasures.push_back(NewTreasure);
			UE_LOG(LogTemp, Log, TEXT("Новый клад: %f %f"), NewTreasure->GetActorLocation().X, NewTreasure->GetActorLocation().Y);
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("Обновление расстояний"));
	for (int i = 0; i < Resolution; i++)
	{
		for (int j = 0; j < Resolution; j++)
		{
			for (int k = 0; k < Treasures.size(); k++)
			{
				Cells[i][j].DistanceToTreasure = std::min(Cells[i][j].DistanceToTreasure, CalculateDistance({i, j}, LocationToIdx(Treasures[k]->GetActorLocation())));
			}
			UE_LOG(LogTemp, Log, TEXT("Новая дистанция: %f"), Cells[i][j].DistanceToTreasure);
			Cells[i][j].x = i;
			Cells[i][j].y = j;
		}
	}
	
	std::vector<std::pair<int,int>> activeList;
    std::vector<std::vector<bool>> isVisited(Resolution, std::vector<bool>(Resolution, false));
    
    activeList.push_back(StartIdx);
    isVisited[StartIdx.first][StartIdx.second] = true;
    
    UE_LOG(LogTemp, Log, TEXT("Начало генерации"));
    
    while (!activeList.empty())
    {
    	int randIdx1 = FMath::RandRange(0, activeList.size() - 1);
    	auto [x, y] = activeList[randIdx1];
		
    	std::vector<std::pair<int, int>> neighbors({{x + 1, y}, {x - 1, y}, {x, y + 1}, {x, y - 1}});
    	std::vector<std::pair<int, int>> actualNeighbors;
		
    	for (const auto& neighbor : neighbors)
    	{
    		auto [nx, ny] = neighbor;
    		if (nx < 0 || nx >= Resolution || ny < 0 || ny >= Resolution
				|| isVisited[nx][ny])
    		{
    			continue;
    		}
    		actualNeighbors.push_back(neighbor);
    	}
        
    	if (!actualNeighbors.empty())
    	{
    		int maxIdx = 0;
    		float maxDist = Cells[actualNeighbors[0].first][actualNeighbors[0].second].DistanceToTreasure;
    		int minIdx = 0;
    		float minDist = Cells[actualNeighbors[0].first][actualNeighbors[0].second].DistanceToTreasure;
    		for (int i = 1; i < (int)actualNeighbors.size(); i++)
    		{
    			float d = Cells[actualNeighbors[i].first][actualNeighbors[i].second].DistanceToTreasure;
    			if (d > maxDist)
    			{
    				maxDist = d;
    				maxIdx = i;
    			}
    			if (d < minDist)
    			{
    				minDist = d;
    				minIdx = i;
    			}
    		}
    		int bestIdx = -1;
    		for (int i = 1; i < (int)actualNeighbors.size(); ++i)
    		{
    			if (i != minIdx && i != maxIdx)
    			{
    				bestIdx = i;
    			}
    		}
    		if (bestIdx == -1)
    		{
    			bestIdx = maxIdx;
    		}
    		auto [nx, ny] = actualNeighbors[bestIdx];
    
    		isVisited[nx][ny] = true;
    		activeList.push_back({nx, ny});
    		RemoveWall({x, y}, {nx, ny});
    	}
        else
        {
			activeList.erase(activeList.begin() + randIdx1);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Генерация завершена"));
}

float ALabGenerator::CalculateDistance(std::pair<int, int> x1, std::pair<int, int> x2)
{
	float a = x1.first - x2.first;
	float b = x1.second - x2.second;
	
	return FMath::Sqrt(a * a + b * b);
}
