// Copyright 2025 guigui17f. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"

/**
 * 基础测试类，提供测试环境和通用功能
 */
class ELEMENTALCOMBATTESTS_API FElementalCombatTestBase : public FAutomationTestBase
{
public:
	FElementalCombatTestBase(const FString& InName, const bool bInComplexTask);
	virtual ~FElementalCombatTestBase() override;

	// 实现FAutomationTestBase的纯虚函数
	virtual bool RunTest(const FString& Parameters) override;
	virtual EAutomationTestFlags GetTestFlags() const override;
	virtual FString GetBeautifiedTestName() const override;
	virtual uint32 GetRequiredDeviceNum() const override;
	virtual void GetTests(TArray<FString>& OutBeautifiedNames, TArray<FString>& OutTestCommands) const override;
	
	/** 创建测试世界 */
	UWorld* CreateTestWorld();

	/** 清理测试世界 */
	void CleanupTestWorld();

protected:

	/** 获取测试世界 */
	UWorld* GetTestWorld() const;

	/** 测试设置 - 子类可以重写 */
	virtual void SetUp();

	/** 测试清理 - 子类可以重写 */
	virtual void TearDown();

private:
	/** 测试世界实例 */
	UWorld* TestWorld;
};