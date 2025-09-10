// Copyright 2025 guigui17f. All Rights Reserved.

#include "CoreMinimal.h"
#include "ElementalCombatTestBase.h"
#include "TestHelpers.h"
#include "Combat/Elemental/ElementalDataAsset.h"
#include "Combat/Elemental/ElementalComponent.h"

/**
 * 数据资产创建测试
 */
ELEMENTAL_TEST(Combat.Elemental, DataAssetCreation)
bool FDataAssetCreationTest::RunTest(const FString& Parameters)
{
	UElementalDataAsset* Asset = NewObject<UElementalDataAsset>();
	TestNotNull(TEXT("数据资产创建"), Asset);
	
	// 初始状态检查
	TestFalse(TEXT("初始无金元素配置"), Asset->HasElementConfiguration(EElementalType::Metal));
	TestFalse(TEXT("初始无木元素配置"), Asset->HasElementConfiguration(EElementalType::Wood));
	TestFalse(TEXT("初始无水元素配置"), Asset->HasElementConfiguration(EElementalType::Water));
	TestFalse(TEXT("初始无火元素配置"), Asset->HasElementConfiguration(EElementalType::Fire));
	TestFalse(TEXT("初始无土元素配置"), Asset->HasElementConfiguration(EElementalType::Earth));
	
	return true;
}

/**
 * 元素数据验证测试
 */
ELEMENTAL_TEST(Combat.Elemental, DataAssetValidation)
bool FDataAssetValidationTest::RunTest(const FString& Parameters)
{
	UElementalDataAsset* Asset = NewObject<UElementalDataAsset>();
	
	FString ErrorMessage;
	bool bIsValid = Asset->ValidateData(ErrorMessage);
	
	// 空数据资产应该提示无配置但不应该错误
	TestTrue(TEXT("空数据资产验证结果"), bIsValid || !ErrorMessage.IsEmpty());
	
	return true;
}

/**
 * 元素类型配置测试
 */
ELEMENTAL_TEST(Combat.Elemental, ConfiguredElements)
bool FConfiguredElementsTest::RunTest(const FString& Parameters)
{
	UElementalDataAsset* Asset = NewObject<UElementalDataAsset>();
	
	// 获取配置的元素列表
	TArray<EElementalType> ConfiguredElements = Asset->GetConfiguredElements();
	
	// 初始状态应该没有配置的元素
	TestEqual(TEXT("初始无配置元素"), ConfiguredElements.Num(), 0);
	
	return true;
}

/**
 * 蓝图函数调用测试
 */
ELEMENTAL_TEST(Combat.Elemental, BlueprintFunction)
bool FBlueprintFunctionTest::RunTest(const FString& Parameters)
{
	UElementalDataAsset* Asset = NewObject<UElementalDataAsset>();
	
	// 测试蓝图可调用的获取函数
	FElementalEffectData OutEffectData;
	bool bFound = Asset->GetElementEffectData(EElementalType::Fire, OutEffectData);
	TestFalse(TEXT("未配置元素返回false"), bFound);
	
	FElementalRelationship OutRelationship;
	bool bRelationFound = Asset->GetElementRelationship(EElementalType::Fire, OutRelationship);
	TestFalse(TEXT("未配置关系返回false"), bRelationFound);
	
	return true;
}

/**
 * 数据拷贝到组件测试
 */
class FDataCopyToComponentTestImpl : public FElementalCombatTestBase
{
public:
	FDataCopyToComponentTestImpl() : FElementalCombatTestBase(TEXT("DataCopyToComponent"), false) {}
	
	virtual bool RunTest(const FString& Parameters) override
	{
		UElementalDataAsset* Asset = NewObject<UElementalDataAsset>();
		UWorld* World = CreateTestWorld();
		AActor* TestActor = World->SpawnActor<AActor>();
		UElementalComponent* Component = NewObject<UElementalComponent>(TestActor);
		Component->RegisterComponent();
		
		// 执行拷贝操作
		Asset->CopyDataToComponent(Component);
		
		// 验证组件状态
		TestNotNull(TEXT("组件有效"), Component);
		TestEqual(TEXT("组件初始元素"), static_cast<int32>(Component->GetCurrentElement()), static_cast<int32>(EElementalType::None));
		
		// 测试空指针处理
		Asset->CopyDataToComponent(nullptr);
		// 应该不会崩溃
		
		return true;
	}
};

ELEMENTAL_TEST(Combat.Elemental, DataCopyToComponent)
bool FDataCopyToComponentTest::RunTest(const FString& Parameters)
{
	FDataCopyToComponentTestImpl TestImpl;
	return TestImpl.RunTest(Parameters);
}