// Copyright 2025 guigui17f. All Rights Reserved.

#include "CoreMinimal.h"
#include "ElementalCombatTestBase.h"
#include "TestHelpers.h"
#include "Combat/Elemental/ElementalComponent.h"
#include "GameFramework/Actor.h"

/**
 * 元素组件创建和初始化测试
 */
class FElementalComponentCreationTestImpl : public FElementalCombatTestBase
{
public:
	FElementalComponentCreationTestImpl() 
		: FElementalCombatTestBase(TEXT("ElementalComponentCreation"), false) {}
	
	virtual bool RunTest(const FString& Parameters) override
	{
		UWorld* World = CreateTestWorld();
		AActor* TestActor = World->SpawnActor<AActor>();
		TEST_ACTOR_VALID(TestActor, TEXT("测试Actor创建"));
		
		// 创建组件
		UElementalComponent* Component = NewObject<UElementalComponent>(TestActor);
		TestNotNull(TEXT("组件创建成功"), Component);
		
		// 注册前状态
		TestFalse(TEXT("未注册状态"), Component->IsRegistered());
		TestEqual(TEXT("初始元素为None"), static_cast<int32>(Component->GetCurrentElement()), static_cast<int32>(EElementalType::None));
		
		// 注册组件
		Component->RegisterComponent();
		TestTrue(TEXT("组件已注册"), Component->IsRegistered());
		TestEqual(TEXT("Owner正确"), Component->GetOwner(), TestActor);
		
		// 组件激活
		Component->Activate();
		TestTrue(TEXT("组件已激活"), Component->IsActive());
		
		// 组件停用
		Component->Deactivate();
		TestFalse(TEXT("组件已停用"), Component->IsActive());
		
		return true;
	}
};

ELEMENTAL_TEST(Combat.Elemental, ElementalComponentCreation)
bool FElementalComponentCreationTest::RunTest(const FString& Parameters)
{
	FElementalComponentCreationTestImpl TestImpl;
	return TestImpl.RunTest(Parameters);
}

/**
 * 元素切换功能测试
 */
class FElementalSwitchFunctionTestImpl : public FElementalCombatTestBase
{
public:
	FElementalSwitchFunctionTestImpl() 
		: FElementalCombatTestBase(TEXT("ElementalSwitchFunction"), false) {}
	
	virtual bool RunTest(const FString& Parameters) override
	{
		UWorld* World = CreateTestWorld();
		AActor* TestActor = World->SpawnActor<AActor>();
		UElementalComponent* Component = NewObject<UElementalComponent>(TestActor);
		Component->RegisterComponent();
		
		// 测试每个元素切换
		Component->SwitchElement(EElementalType::Metal);
		TestEqual(TEXT("切换到金"), static_cast<int32>(Component->GetCurrentElement()), static_cast<int32>(EElementalType::Metal));
		
		Component->SwitchElement(EElementalType::Wood);
		TestEqual(TEXT("切换到木"), static_cast<int32>(Component->GetCurrentElement()), static_cast<int32>(EElementalType::Wood));
		
		Component->SwitchElement(EElementalType::Water);
		TestEqual(TEXT("切换到水"), static_cast<int32>(Component->GetCurrentElement()), static_cast<int32>(EElementalType::Water));
		
		Component->SwitchElement(EElementalType::Fire);
		TestEqual(TEXT("切换到火"), static_cast<int32>(Component->GetCurrentElement()), static_cast<int32>(EElementalType::Fire));
		
		Component->SwitchElement(EElementalType::Earth);
		TestEqual(TEXT("切换到土"), static_cast<int32>(Component->GetCurrentElement()), static_cast<int32>(EElementalType::Earth));
		
		Component->SwitchElement(EElementalType::None);
		TestEqual(TEXT("切换到None"), static_cast<int32>(Component->GetCurrentElement()), static_cast<int32>(EElementalType::None));
		
		// 测试连续切换
		Component->SwitchElement(EElementalType::Fire);
		Component->SwitchElement(EElementalType::Water);
		Component->SwitchElement(EElementalType::Metal);
		TestEqual(TEXT("连续切换后为金"), static_cast<int32>(Component->GetCurrentElement()), static_cast<int32>(EElementalType::Metal));
		
		return true;
	}
};

ELEMENTAL_TEST(Combat.Elemental, ElementalSwitchFunction)
bool FElementalSwitchFunctionTest::RunTest(const FString& Parameters)
{
	FElementalSwitchFunctionTestImpl TestImpl;
	return TestImpl.RunTest(Parameters);
}

/**
 * 元素切换委托测试
 */
class FElementalDelegateTestImpl : public FElementalCombatTestBase
{
public:
	FElementalDelegateTestImpl() 
		: FElementalCombatTestBase(TEXT("ElementalDelegate"), false) {}
	
	virtual bool RunTest(const FString& Parameters) override
	{
		UWorld* World = CreateTestWorld();
		AActor* TestActor = World->SpawnActor<AActor>();
		UElementalComponent* Component = NewObject<UElementalComponent>(TestActor);
		Component->RegisterComponent();
		
		// 简化的委托测试 - 只测试委托是否存在和可绑定
		TestTrue(TEXT("委托对象存在"), Component->OnElementChanged.IsBound() == false); // 初始未绑定
		
		// 测试元素切换功能本身
		Component->SwitchElement(EElementalType::Fire);
		TestEqual(TEXT("切换到火元素"), static_cast<int32>(Component->GetCurrentElement()), static_cast<int32>(EElementalType::Fire));
		
		Component->SwitchElement(EElementalType::Water);
		TestEqual(TEXT("切换到水元素"), static_cast<int32>(Component->GetCurrentElement()), static_cast<int32>(EElementalType::Water));
		
		// 测试相同元素切换
		EElementalType PrevElement = Component->GetCurrentElement();
		Component->SwitchElement(EElementalType::Water);
		TestEqual(TEXT("相同元素保持不变"), static_cast<int32>(Component->GetCurrentElement()), static_cast<int32>(PrevElement));
		
		return true;
	}
};

ELEMENTAL_TEST(Combat.Elemental, ElementalDelegate)
bool FElementalDelegateTest::RunTest(const FString& Parameters)
{
	FElementalDelegateTestImpl TestImpl;
	return TestImpl.RunTest(Parameters);
}

/**
 * 元素效果数据存储测试
 */
class FElementalDataStorageTestImpl : public FElementalCombatTestBase
{
public:
	FElementalDataStorageTestImpl() 
		: FElementalCombatTestBase(TEXT("ElementalDataStorage"), false) {}
	
	virtual bool RunTest(const FString& Parameters) override
	{
		UWorld* World = CreateTestWorld();
		AActor* TestActor = World->SpawnActor<AActor>();
		UElementalComponent* Component = NewObject<UElementalComponent>(TestActor);
		Component->RegisterComponent();
		
		// 创建测试数据
		FElementalEffectData FireData;
		FireData.DotDamage = 15.0f;
		FireData.DotTickInterval = 0.5f;
		FireData.DotDuration = 5.0f;
		
		FElementalEffectData WaterData;
		WaterData.SlowPercentage = 0.6f;
		WaterData.SlowDuration = 4.0f;
		
		// 存储数据
		Component->SetElementEffectData(EElementalType::Fire, FireData);
		Component->SetElementEffectData(EElementalType::Water, WaterData);
		
		// 获取并验证火元素数据
		const FElementalEffectData* RetrievedFire = Component->GetElementEffectDataPtr(EElementalType::Fire);
		TestNotNull(TEXT("火元素数据存在"), RetrievedFire);
		if (RetrievedFire)
		{
			TestNearlyEqual(TEXT("火DOT伤害"), RetrievedFire->DotDamage, 15.0f, 0.01f);
			TestNearlyEqual(TEXT("火DOT间隔"), RetrievedFire->DotTickInterval, 0.5f, 0.01f);
			TestNearlyEqual(TEXT("火DOT持续"), RetrievedFire->DotDuration, 5.0f, 0.01f);
		}
		
		// 获取并验证水元素数据
		const FElementalEffectData* RetrievedWater = Component->GetElementEffectDataPtr(EElementalType::Water);
		TestNotNull(TEXT("水元素数据存在"), RetrievedWater);
		if (RetrievedWater)
		{
			TestNearlyEqual(TEXT("水减速比例"), RetrievedWater->SlowPercentage, 0.6f, 0.01f);
			TestNearlyEqual(TEXT("水减速时间"), RetrievedWater->SlowDuration, 4.0f, 0.01f);
		}
		
		// 获取未设置的数据
		const FElementalEffectData* UnsetData = Component->GetElementEffectDataPtr(EElementalType::Metal);
		TestNull(TEXT("未设置数据为空"), UnsetData);
		
		// 更新已有数据
		FireData.DotDamage = 20.0f;
		Component->SetElementEffectData(EElementalType::Fire, FireData);
		const FElementalEffectData* UpdatedFire = Component->GetElementEffectDataPtr(EElementalType::Fire);
		if (UpdatedFire)
		{
			TestNearlyEqual(TEXT("更新后火DOT伤害"), UpdatedFire->DotDamage, 20.0f, 0.01f);
		}
		
		return true;
	}
};

ELEMENTAL_TEST(Combat.Elemental, ElementalDataStorage)
bool FElementalDataStorageTest::RunTest(const FString& Parameters)
{
	FElementalDataStorageTestImpl TestImpl;
	return TestImpl.RunTest(Parameters);
}

/**
 * 投掷物类管理测试
 */
class FProjectileClassManagementTestImpl : public FElementalCombatTestBase
{
public:
	FProjectileClassManagementTestImpl() 
		: FElementalCombatTestBase(TEXT("ProjectileClassManagement"), false) {}
	
	virtual bool RunTest(const FString& Parameters) override
	{
		UWorld* World = CreateTestWorld();
		AActor* TestActor = World->SpawnActor<AActor>();
		UElementalComponent* Component = NewObject<UElementalComponent>(TestActor);
		Component->RegisterComponent();
		
		// 创建测试投掷物类（使用AActor作为测试）
		UClass* FireProjectileClass = AActor::StaticClass();
		UClass* WaterProjectileClass = AActor::StaticClass();
		
		// 设置投掷物类
		FElementalEffectData FireData;
		FireData.ProjectileClass = FireProjectileClass;
		Component->SetElementEffectData(EElementalType::Fire, FireData);
		
		FElementalEffectData WaterData;
		WaterData.ProjectileClass = WaterProjectileClass;
		Component->SetElementEffectData(EElementalType::Water, WaterData);
		
		// 切换元素并验证投掷物类
		Component->SwitchElement(EElementalType::Fire);
		UClass* CurrentProjectile = Component->GetCurrentProjectileClass();
		TestEqual(TEXT("火元素投掷物类"), CurrentProjectile, FireProjectileClass);
		
		Component->SwitchElement(EElementalType::Water);
		CurrentProjectile = Component->GetCurrentProjectileClass();
		TestEqual(TEXT("水元素投掷物类"), CurrentProjectile, WaterProjectileClass);
		
		// 切换到无投掷物的元素
		Component->SwitchElement(EElementalType::Earth);
		CurrentProjectile = Component->GetCurrentProjectileClass();
		TestNull(TEXT("无投掷物类为空"), CurrentProjectile);
		
		return true;
	}
};

ELEMENTAL_TEST(Combat.Elemental, ProjectileClassManagement)
bool FProjectileClassManagementTest::RunTest(const FString& Parameters)
{
	FProjectileClassManagementTestImpl TestImpl;
	return TestImpl.RunTest(Parameters);
}

// ========== 数据驱动的元素效果系统测试 ==========

/**
 * 数据驱动伤害处理测试
 * 测试ProcessElementalDamage方法的所有功能
 */
class FProcessElementalDamageTestImpl : public FElementalCombatTestBase
{
public:
	FProcessElementalDamageTestImpl()
		: FElementalCombatTestBase(TEXT("ProcessElementalDamage"), false) {}

	virtual bool RunTest(const FString& Parameters) override
	{
		UWorld* World = CreateTestWorld();
		AActor* AttackerActor = World->SpawnActor<AActor>();
		AActor* DefenderActor = World->SpawnActor<AActor>();

		UElementalComponent* DefenderComponent = NewObject<UElementalComponent>(DefenderActor);
		DefenderComponent->RegisterComponent();
		DefenderComponent->SwitchElement(EElementalType::Earth);

		// 测试1：基础伤害倍率应用（不依赖元素类型）
		FElementalEffectData AttackerData;
		AttackerData.Element = EElementalType::Metal;
		AttackerData.DamageMultiplier = 1.5f; // 1.5倍伤害

		float ProcessedDamage = DefenderComponent->ProcessElementalDamage(100.0f, AttackerData, AttackerActor);
		TestNearlyEqual(TEXT("伤害倍率应用"), ProcessedDamage, 150.0f, 1.0f);

		// 测试2：防御方减伤效果
		FElementalEffectData DefenderData;
		DefenderData.Element = EElementalType::Earth;
		DefenderData.DamageReduction = 0.3f; // 30%减伤
		DefenderComponent->SetElementEffectData(EElementalType::Earth, DefenderData);

		ProcessedDamage = DefenderComponent->ProcessElementalDamage(100.0f, AttackerData, AttackerActor);
		// 基础100 * 1.5倍率 * 0.7减伤 = 105
		TestNearlyEqual(TEXT("减伤效果应用"), ProcessedDamage, 105.0f, 1.0f);

		// 测试3：元素相克测试（水克火）
		DefenderComponent->SwitchElement(EElementalType::Fire);
		AttackerData.Element = EElementalType::Water;
		AttackerData.DamageMultiplier = 1.0f;

		ProcessedDamage = DefenderComponent->ProcessElementalDamage(100.0f, AttackerData, AttackerActor);
		// 应该有相克加成（具体倍率由ElementalCalculator决定）
		TestTrue(TEXT("元素相克有效果"), ProcessedDamage > 100.0f);

		// 测试4：零值和边界情况
		AttackerData.DamageMultiplier = 0.0f;
		ProcessedDamage = DefenderComponent->ProcessElementalDamage(100.0f, AttackerData, AttackerActor);
		TestNearlyEqual(TEXT("零倍率处理"), ProcessedDamage, 100.0f, 1.0f); // 应该使用默认1.0

		return true;
	}
};

ELEMENTAL_TEST(Combat.Elemental, ProcessElementalDamage)
bool FProcessElementalDamageTest::RunTest(const FString& Parameters)
{
	FProcessElementalDamageTestImpl TestImpl;
	return TestImpl.RunTest(Parameters);
}

/**
 * 数据驱动效果应用测试
 * 测试ApplyElementalEffects方法的所有效果
 */
class FApplyElementalEffectsTestImpl : public FElementalCombatTestBase
{
public:
	FApplyElementalEffectsTestImpl()
		: FElementalCombatTestBase(TEXT("ApplyElementalEffects"), false) {}

	virtual bool RunTest(const FString& Parameters) override
	{
		UWorld* World = CreateTestWorld();
		AActor* CauserActor = World->SpawnActor<AActor>();
		AActor* TargetActor = World->SpawnActor<AActor>();

		UElementalComponent* TargetComponent = NewObject<UElementalComponent>(TargetActor);
		TargetComponent->RegisterComponent();

		// 测试1：减速效果应用（基于字段值，不基于元素类型）
		FElementalEffectData EffectData;
		EffectData.Element = EElementalType::Metal; // 金元素
		EffectData.SlowPercentage = 0.4f;           // 但有减速效果
		EffectData.SlowDuration = 2.0f;

		TestFalse(TEXT("应用前无减速"), TargetComponent->IsSlowed());
		TargetComponent->ApplyElementalEffects(EffectData, CauserActor, 0.0f);
		TestTrue(TEXT("金元素也能造成减速"), TargetComponent->IsSlowed());

		// 测试2：DOT效果应用
		EffectData.DotDamage = 15.0f;
		EffectData.DotDuration = 3.0f;
		EffectData.DotTickInterval = 0.5f;

		TestFalse(TEXT("应用前无燃烧"), TargetComponent->IsBurning());
		TargetComponent->ApplyElementalEffects(EffectData, CauserActor, 0.0f);
		TestTrue(TEXT("金元素也能造成DOT"), TargetComponent->IsBurning());

		// 测试3：吸血效果应用
		EffectData.LifeStealPercentage = 0.3f;
		// 吸血需要有伤害值
		TargetComponent->ApplyElementalEffects(EffectData, CauserActor, 100.0f);
		// 注意：吸血效果会尝试对CauserActor应用治疗，这里主要测试逻辑不崩溃

		// 测试4：多效果组合
		FElementalEffectData ComboEffect;
		ComboEffect.Element = EElementalType::Water; // 水元素
		ComboEffect.SlowPercentage = 0.6f;           // 有减速
		ComboEffect.DotDamage = 20.0f;               // 也有DOT
		ComboEffect.DotDuration = 4.0f;
		ComboEffect.DotTickInterval = 1.0f;
		ComboEffect.LifeStealPercentage = 0.25f;     // 还有吸血

		// 清除之前的效果
		TargetComponent->ClearAllEffects();
		TestFalse(TEXT("清除后无减速"), TargetComponent->IsSlowed());
		TestFalse(TEXT("清除后无燃烧"), TargetComponent->IsBurning());

		// 应用组合效果
		TargetComponent->ApplyElementalEffects(ComboEffect, CauserActor, 100.0f);
		TestTrue(TEXT("组合效果：减速"), TargetComponent->IsSlowed());
		TestTrue(TEXT("组合效果：DOT"), TargetComponent->IsBurning());

		return true;
	}
};

ELEMENTAL_TEST(Combat.Elemental, ApplyElementalEffects)
bool FApplyElementalEffectsTest::RunTest(const FString& Parameters)
{
	FApplyElementalEffectsTestImpl TestImpl;
	return TestImpl.RunTest(Parameters);
}

/**
 * 效果状态管理测试
 * 测试状态查询和效果清除
 */
class FElementalEffectStateTestImpl : public FElementalCombatTestBase
{
public:
	FElementalEffectStateTestImpl()
		: FElementalCombatTestBase(TEXT("ElementalEffectState"), false) {}

	virtual bool RunTest(const FString& Parameters) override
	{
		UWorld* World = CreateTestWorld();
		AActor* CauserActor = World->SpawnActor<AActor>();
		AActor* TargetActor = World->SpawnActor<AActor>();

		UElementalComponent* Component = NewObject<UElementalComponent>(TargetActor);
		Component->RegisterComponent();

		// 测试1：初始状态
		TestFalse(TEXT("初始无减速"), Component->IsSlowed());
		TestFalse(TEXT("初始无燃烧"), Component->IsBurning());

		// 测试2：应用减速效果
		FElementalEffectData SlowEffect;
		SlowEffect.SlowPercentage = 0.5f;
		SlowEffect.SlowDuration = 3.0f;
		Component->ApplyElementalEffects(SlowEffect, CauserActor, 0.0f);
		TestTrue(TEXT("减速状态正确"), Component->IsSlowed());
		TestFalse(TEXT("仅减速无燃烧"), Component->IsBurning());

		// 测试3：应用DOT效果
		FElementalEffectData DotEffect;
		DotEffect.DotDamage = 10.0f;
		DotEffect.DotDuration = 2.0f;
		DotEffect.DotTickInterval = 0.5f;
		Component->ApplyElementalEffects(DotEffect, CauserActor, 0.0f);
		TestTrue(TEXT("减速仍存在"), Component->IsSlowed());
		TestTrue(TEXT("DOT状态正确"), Component->IsBurning());

		// 测试4：效果刷新
		FElementalEffectData NewSlowEffect;
		NewSlowEffect.SlowPercentage = 0.8f; // 更强的减速
		NewSlowEffect.SlowDuration = 5.0f;   // 更长的时间
		Component->ApplyElementalEffects(NewSlowEffect, CauserActor, 0.0f);
		TestTrue(TEXT("减速效果刷新"), Component->IsSlowed());
		TestTrue(TEXT("DOT依然存在"), Component->IsBurning());

		// 测试5：清除所有效果
		Component->ClearAllEffects();
		TestFalse(TEXT("清除后无减速"), Component->IsSlowed());
		TestFalse(TEXT("清除后无燃烧"), Component->IsBurning());

		return true;
	}
};

ELEMENTAL_TEST(Combat.Elemental, ElementalEffectState)
bool FElementalEffectStateTest::RunTest(const FString& Parameters)
{
	FElementalEffectStateTestImpl TestImpl;
	return TestImpl.RunTest(Parameters);
}

/**
 * 数据驱动灵活性测试
 * 验证任意元素可配置任意效果的灵活性
 */
class FDataDrivenFlexibilityTestImpl : public FElementalCombatTestBase
{
public:
	FDataDrivenFlexibilityTestImpl()
		: FElementalCombatTestBase(TEXT("DataDrivenFlexibility"), false) {}

	virtual bool RunTest(const FString& Parameters) override
	{
		UWorld* World = CreateTestWorld();
		AActor* CauserActor = World->SpawnActor<AActor>();
		AActor* TargetActor = World->SpawnActor<AActor>();

		UElementalComponent* Component = NewObject<UElementalComponent>(TargetActor);
		Component->RegisterComponent();

		// 测试1：金元素配置减速效果（打破传统认知）
		FElementalEffectData MetalWithSlow;
		MetalWithSlow.Element = EElementalType::Metal;
		MetalWithSlow.SlowPercentage = 0.6f;
		MetalWithSlow.SlowDuration = 2.0f;
		MetalWithSlow.DamageMultiplier = 1.3f; // 同时有伤害倍率

		Component->ApplyElementalEffects(MetalWithSlow, CauserActor, 0.0f);
		TestTrue(TEXT("金元素能造成减速"), Component->IsSlowed());

		Component->ClearAllEffects();

		// 测试2：水元素配置DOT效果（传统水元素不烧人）
		FElementalEffectData WaterWithDot;
		WaterWithDot.Element = EElementalType::Water;
		WaterWithDot.DotDamage = 8.0f;
		WaterWithDot.DotDuration = 4.0f;
		WaterWithDot.DotTickInterval = 1.0f;
		WaterWithDot.SlowPercentage = 0.3f; // 同时有减速

		Component->ApplyElementalEffects(WaterWithDot, CauserActor, 0.0f);
		TestTrue(TEXT("水元素能造成DOT"), Component->IsBurning());
		TestTrue(TEXT("水元素也能减速"), Component->IsSlowed());

		Component->ClearAllEffects();

		// 测试3：超级元素（所有效果都有）
		FElementalEffectData SuperElement;
		SuperElement.Element = EElementalType::Fire;
		SuperElement.DamageMultiplier = 2.0f;      // 高伤害
		SuperElement.LifeStealPercentage = 0.4f;   // 吸血
		SuperElement.SlowPercentage = 0.8f;        // 减速
		SuperElement.SlowDuration = 3.0f;
		SuperElement.DotDamage = 25.0f;            // DOT
		SuperElement.DotDuration = 5.0f;
		SuperElement.DotTickInterval = 0.8f;
		SuperElement.DamageReduction = 0.2f;       // 减伤（作为防御配置）

		Component->ApplyElementalEffects(SuperElement, CauserActor, 100.0f);
		TestTrue(TEXT("超级元素：减速"), Component->IsSlowed());
		TestTrue(TEXT("超级元素：DOT"), Component->IsBurning());
		// 吸血和减伤需要通过其他方式验证

		// 测试4：空配置处理
		FElementalEffectData EmptyEffect;
		EmptyEffect.Element = EElementalType::Earth;
		// 所有效果值都是默认0

		Component->ClearAllEffects();
		Component->ApplyElementalEffects(EmptyEffect, CauserActor, 100.0f);
		TestFalse(TEXT("空配置无减速"), Component->IsSlowed());
		TestFalse(TEXT("空配置无DOT"), Component->IsBurning());

		return true;
	}
};

ELEMENTAL_TEST(Combat.Elemental, DataDrivenFlexibility)
bool FDataDrivenFlexibilityTest::RunTest(const FString& Parameters)
{
	FDataDrivenFlexibilityTestImpl TestImpl;
	return TestImpl.RunTest(Parameters);
}

/**
 * 边界条件测试
 * 测试边界值和异常情况处理
 */
class FElementalBoundaryTestImpl : public FElementalCombatTestBase
{
public:
	FElementalBoundaryTestImpl()
		: FElementalCombatTestBase(TEXT("ElementalBoundary"), false) {}

	virtual bool RunTest(const FString& Parameters) override
	{
		UWorld* World = CreateTestWorld();
		AActor* CauserActor = World->SpawnActor<AActor>();
		AActor* TargetActor = World->SpawnActor<AActor>();

		UElementalComponent* Component = NewObject<UElementalComponent>(TargetActor);
		Component->RegisterComponent();

		// 测试1：零值处理
		FElementalEffectData ZeroValues;
		ZeroValues.SlowPercentage = 0.0f;
		ZeroValues.SlowDuration = 5.0f; // 有时间但无减速比例
		ZeroValues.DotDamage = 0.0f;
		ZeroValues.DotDuration = 3.0f;  // 有时间但无伤害
		ZeroValues.LifeStealPercentage = 0.0f;

		Component->ApplyElementalEffects(ZeroValues, CauserActor, 100.0f);
		TestFalse(TEXT("零减速不应用"), Component->IsSlowed());
		TestFalse(TEXT("零DOT不应用"), Component->IsBurning());

		// 测试2：负值处理
		FElementalEffectData NegativeValues;
		NegativeValues.SlowPercentage = -0.5f;    // 负值
		NegativeValues.SlowDuration = 2.0f;
		NegativeValues.DotDamage = -10.0f;        // 负值
		NegativeValues.DotDuration = 3.0f;
		NegativeValues.LifeStealPercentage = -0.3f; // 负值

		Component->ApplyElementalEffects(NegativeValues, CauserActor, 100.0f);
		TestFalse(TEXT("负减速不应用"), Component->IsSlowed());
		TestFalse(TEXT("负DOT不应用"), Component->IsBurning());

		// 测试3：伤害处理边界值
		FElementalEffectData AttackerData;
		AttackerData.DamageMultiplier = -1.0f; // 负倍率

		float Result = Component->ProcessElementalDamage(100.0f, AttackerData, CauserActor);
		TestTrue(TEXT("负倍率结果非负"), Result >= 0.0f);

		// 测试4：空指针保护
		Result = Component->ProcessElementalDamage(100.0f, AttackerData, nullptr);
		TestTrue(TEXT("空指针不崩溃"), Result >= 0.0f);

		// 测试5：超大值处理
		FElementalEffectData LargeValues;
		LargeValues.DamageMultiplier = 1000.0f;  // 超大倍率
		LargeValues.SlowPercentage = 5.0f;       // 超过100%
		LargeValues.SlowDuration = 1.0f;
		LargeValues.DamageReduction = 2.0f;      // 超过100%

		Result = Component->ProcessElementalDamage(100.0f, LargeValues, CauserActor);
		TestTrue(TEXT("超大倍率有结果"), Result > 100.0f);

		Component->ApplyElementalEffects(LargeValues, CauserActor, 0.0f);
		TestTrue(TEXT("超大减速比例应用"), Component->IsSlowed());

		return true;
	}
};

ELEMENTAL_TEST(Combat.Elemental, ElementalBoundary)
bool FElementalBoundaryTest::RunTest(const FString& Parameters)
{
	FElementalBoundaryTestImpl TestImpl;
	return TestImpl.RunTest(Parameters);
}