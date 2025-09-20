// Copyright 2025 guigui17f. All Rights Reserved.

#include "ElementalCombatAIController.h"
#include "ElementalCombatEnemy.h"
#include "ElementalCombatGameInstance.h"
#include "ElementalCombatGameMode.h"
#include "Components/StateTreeAIComponent.h"

AElementalCombatAIController::AElementalCombatAIController()
{
	// 父类构造函数会自动调用，创建StateTreeAI组件并设置必要的标志
	// 子类特有的初始化代码可以在这里添加
}

void AElementalCombatAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// 先进行必要的初始化
	// 缓存元素战斗敌人引用
	ElementalCombatEnemy = Cast<AElementalCombatEnemy>(InPawn);

	if (ElementalCombatEnemy)
	{
		UE_LOG(LogTemp, Log, TEXT("元素战斗AI控制器：已控制元素战斗敌人 %s"), *ElementalCombatEnemy->GetName());

		// 从GameInstance随机获取AI配置
		if (UElementalCombatGameInstance* GameInstance = Cast<UElementalCombatGameInstance>(GetWorld()->GetGameInstance()))
		{
			CurrentAIProfile = GameInstance->GetRandomAIProfile();
			UE_LOG(LogTemp, Log, TEXT("AI配置已设置: %s"), *CurrentAIProfile.ProfileName);
		}
		else
		{
			// 使用默认测试配置
			CurrentAIProfile = CreateDefaultTestProfile();
			UE_LOG(LogTemp, Log, TEXT("使用默认测试AI配置: %s"), *CurrentAIProfile.ProfileName);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("元素战斗AI控制器：被控制的Pawn不是元素战斗敌人"));
	}

	// 验证StateTree组件
	if (UStateTreeAIComponent* StateTreeComp = FindComponentByClass<UStateTreeAIComponent>())
	{
		UE_LOG(LogTemp, Log, TEXT("StateTree组件已正确初始化"));

		// 检查游戏是否处于PSO加载暂停状态
		if (AElementalCombatGameMode* GameMode = Cast<AElementalCombatGameMode>(GetWorld()->GetAuthGameMode()))
		{
			if (GameMode->IsGameplayPausedForPSO())
			{
				// 停止AI逻辑但不跳过初始化
				StopMovement();
				StateTreeComp->StopLogic(TEXT("PSO Loading"));
				UE_LOG(LogTemp, Log, TEXT("AI Controller %s initialized but paused due to PSO loading"), *GetName());
				return; // 初始化完成后再返回
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("StateTree组件未找到！"));
	}
}

void AElementalCombatAIController::OnUnPossess()
{
	// 清理引用
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

	// 设置默认AI配置 - 不添加标签，将使用默认的近战AI行为
	DefaultProfile.MeleeToRangedSwitchDistance = 300.0f;

	return DefaultProfile;
}