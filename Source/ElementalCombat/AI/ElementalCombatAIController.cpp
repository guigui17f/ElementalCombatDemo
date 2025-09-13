// Copyright 2025 guigui17f. All Rights Reserved.

#include "ElementalCombatAIController.h"
#include "ElementalCombatEnemy.h"
#include "ElementalCombatGameInstance.h"

AElementalCombatAIController::AElementalCombatAIController()
{
	// Minimal constructor - all logic handled by StateTree
}

void AElementalCombatAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// Cache reference to the elemental combat enemy
	ElementalCombatEnemy = Cast<AElementalCombatEnemy>(InPawn);
	
	if (ElementalCombatEnemy)
	{
		UE_LOG(LogTemp, Log, TEXT("ElementalCombatAIController: Possessed ElementalCombatEnemy %s"), *ElementalCombatEnemy->GetName());
		
		// 从GameInstance随机获取AI配置
		if (UElementalCombatGameInstance* GameInstance = Cast<UElementalCombatGameInstance>(GetWorld()->GetGameInstance()))
		{
			CurrentAIProfile = GameInstance->GetRandomAIProfile();
			UE_LOG(LogTemp, Log, TEXT("AI配置已设置: %s"), *CurrentAIProfile.ProfileName);
		}
		else
		{
			// 使用默认测试配置而不是空配置
			CurrentAIProfile = CreateDefaultTestProfile();
			UE_LOG(LogTemp, Log, TEXT("使用默认测试AI配置: %s"), *CurrentAIProfile.ProfileName);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ElementalCombatAIController: Possessed pawn is not an ElementalCombatEnemy"));
	}
}

void AElementalCombatAIController::OnUnPossess()
{
	// Clear references
	ElementalCombatEnemy = nullptr;

	Super::OnUnPossess();
}

#if WITH_AUTOMATION_TESTS || WITH_EDITOR
void AElementalCombatAIController::SetAIProfileForTest(const FUtilityProfile& TestProfile)
{
	CurrentAIProfile = TestProfile;
	UE_LOG(LogTemp, Log, TEXT("AI配置已通过测试方法设置: %s"), *CurrentAIProfile.ProfileName);
}
#endif

FUtilityProfile AElementalCombatAIController::CreateDefaultTestProfile()
{
	FUtilityProfile DefaultProfile;
	DefaultProfile.ProfileName = TEXT("DefaultTestProfile");
	
	// 设置平衡的权重配置
	DefaultProfile.Weights.Add(EConsiderationType::Health, 0.3f);
	DefaultProfile.Weights.Add(EConsiderationType::Distance, 0.2f);
	DefaultProfile.Weights.Add(EConsiderationType::ElementAdvantage, 0.3f);
	DefaultProfile.Weights.Add(EConsiderationType::ThreatLevel, 0.2f);
	
	// 添加基本的考虑因子
	FUtilityConsideration HealthConsideration;
	HealthConsideration.ConsiderationType = EConsiderationType::Health;
	HealthConsideration.ResponseCurve.EditorCurveData.Reset();
	HealthConsideration.ResponseCurve.EditorCurveData.AddKey(0.0f, 0.0f);
	HealthConsideration.ResponseCurve.EditorCurveData.AddKey(1.0f, 1.0f);
	DefaultProfile.Considerations.Add(HealthConsideration);
	
	FUtilityConsideration DistanceConsideration;
	DistanceConsideration.ConsiderationType = EConsiderationType::Distance;
	DistanceConsideration.ResponseCurve.EditorCurveData.Reset();
	DistanceConsideration.ResponseCurve.EditorCurveData.AddKey(0.0f, 1.0f);
	DistanceConsideration.ResponseCurve.EditorCurveData.AddKey(1.0f, 0.0f);
	DefaultProfile.Considerations.Add(DistanceConsideration);
	
	FUtilityConsideration ElementConsideration;
	ElementConsideration.ConsiderationType = EConsiderationType::ElementAdvantage;
	ElementConsideration.ResponseCurve.EditorCurveData.Reset();
	ElementConsideration.ResponseCurve.EditorCurveData.AddKey(0.0f, 0.0f);
	ElementConsideration.ResponseCurve.EditorCurveData.AddKey(1.0f, 1.0f);
	DefaultProfile.Considerations.Add(ElementConsideration);
	
	return DefaultProfile;
}