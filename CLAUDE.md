## 1. 项目概览

### 技术栈
- **引擎**: Unreal Engine 5.6
- **IDE**: Visual Studio 2022
- **C++标准**: C++20
- **核心系统**: StateTree AI + Utility评分 + 五行元素
- **架构基础**: 现有CombatCharacter/CombatEnemy + 接口扩展

### 开发环境
```ini
[基础配置]
引擎版本: UE 5.6
编译器: MSVC v143  
构建配置: Development Editor
目标平台: Windows 64-bit

[项目设置]
C++标准: C++20
模块依赖: Engine, AIModule, UMG, StateTreeModule
```

## 2. 代码规范

### 命名约定
```cpp
// 类命名 - 保持现有Combat命名空间
Classes:
  ACombatCharacter -> AAdvancedCombatCharacter
  ACombatEnemy -> AElementalCombatEnemy
  ACombatAIController -> AUtilityCombatAIController

// 接口扩展
Interfaces:
  ICombatAttacker -> IElementalCombatAttacker  
  ICombatDamageable -> IElementalCombatDamageable

// 新增元素相关
Enums:        EElementType, EElementReaction
Structs:      FElementalData, FUtilityScore
Components:   UElementalAttributeComponent
```

### 文件组织
```
Source/ElementalCombat/Variant_Combat/
├── AI/
│   ├── UtilityAI/           // 新增Utility AI系统
│   ├── CombatAIController.* // 现有基础
│   └── CombatStateTree*.    // 现有StateTree扩展
├── Elemental/               // 新增元素系统
│   ├── ElementalTypes.h
│   ├── ElementalComponent.*
│   └── ElementalCalculator.*
├── Interfaces/              // 扩展现有接口
└── Tests/                   // 测试用例
```

### 注释规范
```cpp
/**
 * 扩展现有CombatCharacter以支持五行元素系统
 * 基于现有ICombatAttacker/ICombatDamageable接口
 * 
 * @note 保持与现有战斗系统的完全兼容性
 */
class AAdvancedCombatCharacter : public ACombatCharacter
{
    // 继承现有功能，扩展元素属性
    UPROPERTY(EditAnywhere, Category="Elemental")
    EElementType CurrentElement = EElementType::None;
};
```

## 3. 架构扩展指南

### 基于现有架构的扩展原则
1. **继承优于重写** - 扩展现有ACombatCharacter而非替换
2. **接口兼容性** - 保持现有ICombatAttacker/ICombatDamageable接口工作
3. **组件化扩展** - 通过组件添加新功能而非修改核心类

### 接口扩展模式
```cpp
// 扩展现有攻击者接口
class IElementalCombatAttacker : public ICombatAttacker
{
public:
    // 保留现有接口
    virtual void DoAttackTrace(FName DamageSourceBone) override;
    virtual void CheckCombo() override;
    virtual void CheckChargedAttack() override;
    
    // 新增元素功能
    UFUNCTION(BlueprintCallable)
    virtual EElementType GetAttackElement() const = 0;
    
    UFUNCTION(BlueprintCallable)  
    virtual float CalculateElementalDamage(EElementType TargetElement) const = 0;
};
```

### StateTree扩展规范
```cpp
// 基于现有StateTree任务扩展
USTRUCT(meta=(DisplayName="Utility Action Selection"))
struct FStateTreeUtilitySelectionTask : public FStateTreeTaskCommonBase
{
    GENERATED_BODY()
    
    // 重用现有StateTree架构
    using FInstanceDataType = FStateTreeAttackInstanceData;
    
    // 添加Utility评分逻辑
    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, 
                                          const FStateTreeTransitionResult& Transition) const override;
};
```

## 4. 核心系统实现

### Utility AI评分系统
```cpp
// 标准Utility评分接口
UINTERFACE(BlueprintType)
class UUtilityScorer : public UInterface
{
    GENERATED_BODY()
};

class IUtilityScorer
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable)
    virtual float CalculateUtility(const FUtilityContext& Context) = 0;
};

// 标准评分结构
USTRUCT(BlueprintType)
struct FUtilityScore
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, meta=(ClampMin="0.0", ClampMax="1.0"))
    float HealthFactor = 0.0f;
    
    UPROPERTY(EditAnywhere, meta=(ClampMin="0.0", ClampMax="1.0"))  
    float DistanceFactor = 0.0f;
    
    UPROPERTY(EditAnywhere, meta=(ClampMin="0.0", ClampMax="1.0"))
    float ElementAdvantage = 0.0f;
    
    UPROPERTY(EditAnywhere, meta=(ClampMin="0.0", ClampMax="1.0"))
    float TacticalValue = 0.0f;
    
    // 标准权重配置
    UPROPERTY(EditAnywhere)
    float HealthWeight = 0.3f;
    UPROPERTY(EditAnywhere) 
    float DistanceWeight = 0.2f;
    UPROPERTY(EditAnywhere)
    float ElementWeight = 0.3f;
    UPROPERTY(EditAnywhere)
    float TacticalWeight = 0.2f;
    
    float CalculateFinalScore() const
    {
        return (HealthFactor * HealthWeight) + 
               (DistanceFactor * DistanceWeight) +
               (ElementAdvantage * ElementWeight) +
               (TacticalValue * TacticalWeight);
    }
};
```

### 五行元素集成方法
```cpp
// 五行元素枚举
UENUM(BlueprintType)
enum class EElementType : uint8
{
    None = 0,
    Metal = 1,    // 金
    Wood = 2,     // 木
    Water = 3,    // 水
    Fire = 4,     // 火
    Earth = 5     // 土
};

// 元素计算器
UCLASS()
class UElementalCalculator : public UObject
{
    GENERATED_BODY()
    
public:
    // 相克关系检查 - 简化版
    UFUNCTION(BlueprintCallable, BlueprintPure)
    static bool IsElementAdvantage(EElementType Attacker, EElementType Defender)
    {
        switch (Attacker)
        {
        case EElementType::Metal: return Defender == EElementType::Wood;
        case EElementType::Wood:  return Defender == EElementType::Earth; 
        case EElementType::Water: return Defender == EElementType::Fire;
        case EElementType::Fire:  return Defender == EElementType::Metal;
        case EElementType::Earth: return Defender == EElementType::Water;
        default: return false;
        }
    }
    
    // 伤害修正计算
    UFUNCTION(BlueprintCallable, BlueprintPure)
    static float CalculateElementalDamageModifier(EElementType AttackElement, 
                                                 EElementType DefenseElement)
    {
        if (IsElementAdvantage(AttackElement, DefenseElement))
        {
            return 1.5f; // 相克加成50%
        }
        return 1.0f; // 基础伤害
    }
};

// 在现有DoAttackTrace中集成
void AAdvancedCombatCharacter::DoAttackTrace(FName DamageSourceBone)
{
    // 复用现有碰撞检测逻辑
    Super::DoAttackTrace(DamageSourceBone);
    
    // 添加元素伤害修正
    // [现有的碰撞检测代码保持不变]
    
    for (const FHitResult& CurrentHit : OutHits)
    {
        if (auto* ElementalTarget = Cast<IElementalCombatDamageable>(CurrentHit.GetActor()))
        {
            EElementType TargetElement = ElementalTarget->GetElementType();
            float ElementModifier = UElementalCalculator::CalculateElementalDamageModifier(
                CurrentElement, TargetElement);
                
            float FinalDamage = MeleeDamage * ElementModifier;
            ElementalTarget->ApplyElementalDamage(FinalDamage, CurrentElement, this);
        }
    }
}
```

### AI决策逻辑开发规范
```cpp
// 基于现有CombatAIController扩展
class AUtilityCombatAIController : public ACombatAIController
{
private:
    // Utility评分缓存
    TMap<EAIAction, float> ActionScores;
    float LastEvaluationTime = 0.0f;
    
public:
    // 核心决策函数
    EAIAction SelectBestAction()
    {
        UpdateActionScores();
        
        EAIAction BestAction = EAIAction::None;
        float BestScore = 0.0f;
        
        for (auto& Pair : ActionScores)
        {
            if (Pair.Value > BestScore)
            {
                BestScore = Pair.Value;
                BestAction = Pair.Key;
            }
        }
        
        return BestAction;
    }
    
    // 元素选择策略
    EElementType SelectOptimalElement(AActor* Target)
    {
        if (auto* ElementalTarget = Cast<IElementalCombatDamageable>(Target))
        {
            EElementType TargetElement = ElementalTarget->GetElementType();
            
            // 寻找相克元素
            for (int32 i = 1; i <= 5; ++i)
            {
                EElementType TestElement = (EElementType)i;
                if (UElementalCalculator::IsElementAdvantage(TestElement, TargetElement))
                {
                    return TestElement;
                }
            }
        }
        
        return EElementType::Fire; // 默认元素
    }
};
```

## 5. 测试框架使用

### 标准测试类模板
```cpp
// Utility AI测试
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUtilityAITest, 
    "ElementalCombat.AI.UtilitySystem",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FUtilityAITest::RunTest(const FString& Parameters)
{
    // Arrange
    AUtilityCombatAIController* AIController = NewObject<AUtilityCombatAIController>();
    
    // Act
    EAIAction SelectedAction = AIController->SelectBestAction();
    
    // Assert
    TestTrue(TEXT("AI should select valid action"), SelectedAction != EAIAction::None);
    
    return true;
}

// 元素系统测试
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FElementalSystemTest, 
    "ElementalCombat.Elemental.Calculation",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FElementalSystemTest::RunTest(const FString& Parameters)
{
    // 测试相克关系
    bool bWaterBeatsFireCorrect = UElementalCalculator::IsElementAdvantage(
        EElementType::Water, EElementType::Fire);
    TestTrue(TEXT("Water should beat Fire"), bWaterBeatsFireCorrect);
    
    // 测试伤害修正
    float DamageModifier = UElementalCalculator::CalculateElementalDamageModifier(
        EElementType::Water, EElementType::Fire);
    TestEqual(TEXT("Advantage damage modifier"), DamageModifier, 1.5f);
    
    return true;
}
```

### 核心测试用例编写规范
```cpp
// 测试分类
namespace ElementalCombat::Tests
{
    // 1. Utility AI测试
    class FUtilityTests
    {
        static void TestScoreCalculation();
        static void TestActionSelection(); 
        static void TestElementSelection();
    };
    
    // 2. 元素系统测试
    class FElementalTests
    {
        static void TestElementAdvantage();
        static void TestDamageCalculation();
        static void TestElementIntegration();
    };
    
    // 3. 集成测试
    class FIntegrationTests
    {
        static void TestAIElementalDecision();
        static void TestCombatFlow();
        static void TestPerformance();
    };
}
```

## 6. 性能和调试

### 关键性能检查点
```cpp
// 性能监控宏
#define ELEMENTAL_SCOPED_TIMER(Name) \
    SCOPE_CYCLE_COUNTER(STAT_Elemental##Name)

// 关键函数性能监控
void AUtilityCombatAIController::SelectBestAction()
{
    ELEMENTAL_SCOPED_TIMER(UtilityCalculation);
    // 函数实现
}

// 内存使用监控
void MonitorElementalSystemMemory()
{
    SIZE_T UtilityAIMemory = sizeof(FUtilityScore) * ActiveAICount;
    SIZE_T ElementalMemory = sizeof(FElementalData) * ActiveElementalActors;
    
    UE_LOG(LogElemental, Display, TEXT("Memory Usage - AI: %d bytes, Elemental: %d bytes"), 
           UtilityAIMemory, ElementalMemory);
}
```

### 常用调试工具配置
```cpp
// 调试日志类别
DECLARE_LOG_CATEGORY_EXTERN(LogElementalAI, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogElementalSystem, Log, All);

// 调试显示
#if !UE_BUILD_SHIPPING
void DrawUtilityDebugInfo()
{
    if (GEngine && bShowDebugInfo)
    {
        FString DebugText = FString::Printf(TEXT("Best Action: %s, Score: %.2f"), 
                                          *GetActionName(BestAction), BestScore);
        GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, DebugText);
    }
}
#endif

// 控制台命令
static FAutoConsoleCommand CCmdToggleElementalDebug(
    TEXT("Elemental.ToggleDebug"),
    TEXT("Toggle elemental system debug display"),
    FConsoleCommandDelegate::CreateLambda([]()
    {
        bShowElementalDebug = !bShowElementalDebug;
    })
);
```

### 开发检查清单
```
性能目标:
□ AI决策 < 2ms per frame
□ 元素计算 < 0.5ms per calculation  
□ 稳定60 FPS with 10+ AI characters

功能验证:
□ 五种元素相克关系正确
□ AI能智能选择优势元素
□ Utility评分机制工作正常
□ 与现有战斗系统完全兼容

代码质量:
□ 核心代码有对应测试用例
□ 编译无警告
□ 遵循项目命名规范
□ 接口向后兼容
```

---

## 代码生成模板

### 新类创建模板
```cpp
#pragma once

#include "CoreMinimal.h"
#include "../CombatCharacter.h"  // 基于现有架构
#include "ElementalCombatCharacter.generated.h"

/**
 * 基于现有CombatCharacter的元素战斗角色扩展
 * 添加五行元素支持和Utility AI决策能力
 */
UCLASS(BlueprintType, Blueprintable)
class ELEMENTALCOMBAT_API AElementalCombatCharacter : public ACombatCharacter
{
    GENERATED_BODY()

public:
    AElementalCombatCharacter();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // 元素属性
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Elemental")
    EElementType CurrentElement = EElementType::Fire;

public:
    // 继承并扩展现有接口
    virtual void DoAttackTrace(FName DamageSourceBone) override;
    
    // 新增元素功能
    UFUNCTION(BlueprintCallable, Category="Elemental")
    void SetCurrentElement(EElementType NewElement);
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Elemental")
    EElementType GetCurrentElement() const { return CurrentElement; }
};
```
