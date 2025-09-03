# UtilityAI+五行元素战斗演示Demo - 设计文档

## 1. 项目概览

### 1.1 项目规模
- **总开发周期**：7天（1周）
- **代码规模预估**：8,000-10,000行代码
- **场景数量**：1个主展示场景
- **游戏时长**：10-15分钟完整体验
- **团队规模**：1人独立开发 + Claude Code辅助

### 1.2 核心亮点
- **智能AI决策系统**：基于Utility AI的多因素评分机制
- **五行相克战斗**：金木水火土元素相生相克系统
- **物理打击反馈**：基于现有PhysicalAnimation的增强响应
- **StateTree AI架构**：扩展现有StateTree实现复杂决策逻辑

## 2. 技术架构

### 2.1 基于现有架构的扩展方案
```
现有CombatCharacter架构
├── ACombatCharacter (基础战斗角色)
│   └── AAdvancedCombatCharacter (扩展：元素+AI)
├── ACombatEnemy (基础AI敌人)  
│   └── AElementalCombatEnemy (扩展：Utility AI决策)
├── ICombatAttacker/ICombatDamageable (现有接口)
│   └── IElementalCombatAttacker/IElementalCombatDamageable (元素接口扩展)
└── StateTree Tasks (现有AI任务)
    └── Utility Selection Tasks (新增智能决策任务)
```

### 2.2 核心系统架构
```cpp
// 1. Utility AI子系统
class UUtilityAISubsystem : public UGameInstanceSubsystem
{
    // AI评分管理器
    TMap<AActor*, FUtilityScoreCache> ScoreCache;
    // 行为选择器
    TArray<IUtilityScorer*> AvailableScorers;
};

// 2. 五行元素子系统
class UElementalSystemSubsystem : public UGameInstanceSubsystem  
{
    // 元素相克关系表
    TMap<FElementPair, float> ElementAdvantageMap;
    // 元素效果管理器
    TObjectPtr<UElementalEffectManager> EffectManager;
};

// 3. 扩展AI控制器
class AUtilityCombatAIController : public ACombatAIController
{
    // Utility评分组件
    UPROPERTY(VisibleAnywhere)
    class UUtilityDecisionComponent* DecisionComponent;
};
```

## 3. 核心玩法设计

### 3.1 控制方案（基于Enhanced Input System）
```
移动控制：
- WASD: 角色移动 (复用现有系统)
- 鼠标: 视角控制 (复用现有系统)  
- Shift: 冲刺
- Space: 跳跃

战斗控制：
- 左键: 基础攻击 (扩展现有DoAttackTrace)
- 右键: 元素技能释放
- Q/E: 切换元素（金木水火土循环）
- 1-5: 直接选择元素
- Tab: 显示AI决策可视化

调试控制：
- F1: 显示Utility评分
- F2: 显示元素相克关系
- F3: AI行为树可视化
```

### 3.2 游戏循环
```
1. 初始阶段 (30秒)
   - 玩家熟悉五种元素切换
   - 观察基础AI行为
   
2. 对战阶段 (5-8分钟)
   - 与具备Utility AI的敌人战斗
   - 体验元素相克的战术优势
   - AI展示智能的元素选择和位置决策
   
3. 压力测试 (2-4分钟)
   - 多个AI同时战斗
   - 复杂的元素反应链
   - AI协作和竞争行为展示
   
4. 总结展示 (1分钟)
   - 显示AI决策统计
   - 元素使用效率分析
```

## 4. Utility AI系统详细设计

### 4.1 评分因子设计
```cpp
struct FUtilityFactors
{
    // 基础因子 (0.0-1.0标准化)
    float HealthRatio;          // 生命值比例
    float DistanceToTarget;     // 与目标距离 (标准化)
    float ElementAdvantage;     // 元素优势程度
    float TacticalPosition;     // 战术位置价值
    float ThreatLevel;          // 威胁等级评估
    
    // 动态权重
    float HealthWeight = 0.25f;
    float DistanceWeight = 0.2f;
    float ElementWeight = 0.3f;   // 重点关注元素
    float PositionWeight = 0.15f;
    float ThreatWeight = 0.1f;
};
```

### 4.2 行为决策系统
```cpp
enum class EUtilityAction : uint8
{
    None = 0,
    AggressiveAttack,    // 激进攻击 (优势元素+近距离)
    DefensiveRetreat,    // 防御撤退 (劣势元素+低血量)
    ElementalSwitch,     // 元素切换 (寻找相克优势)
    TacticalManeuver,    // 战术机动 (位置优化)
    SupportAlly,         // 支援友军 (多AI场景)
    SpecialAbility       // 特殊技能 (高威胁情况)
};

class UUtilityActionScorer : public UObject, public IUtilityScorer
{
public:
    virtual float CalculateUtility(const FUtilityContext& Context) override
    {
        FUtilityFactors Factors = GatherFactors(Context);
        return CalculateWeightedScore(Factors);
    }
    
private:
    float CalculateElementAdvantage(EElementType MyElement, 
                                   EElementType TargetElement);
    float EvaluateTacticalPosition(FVector MyPos, FVector TargetPos);
    float AssessThreatLevel(AActor* Target);
};
```

### 4.3 StateTree集成
```cpp
// 基于现有StateTree任务扩展  
USTRUCT(meta=(DisplayName="Utility Action Selection", Category="AI"))
struct FStateTreeUtilityActionTask : public FStateTreeTaskCommonBase
{
    GENERATED_BODY()
    
    // 复用现有任务数据结构
    using FInstanceDataType = FStateTreeAttackInstanceData;
    
    // 可选行为列表
    UPROPERTY(EditAnywhere)
    TArray<EUtilityAction> AvailableActions;
    
    // 评分阈值
    UPROPERTY(EditAnywhere, meta=(ClampMin="0.0", ClampMax="1.0"))
    float MinimumScore = 0.3f;
    
    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, 
                                          const FStateTreeTransitionResult& Transition) const override;
                                          
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, 
                                    const float DeltaTime) const override;
};
```

## 5. 五行元素系统设计

### 5.1 元素类型定义
```cpp
UENUM(BlueprintType)
enum class EElementType : uint8
{
    None = 0,
    Metal = 1,    // 金 - 高物理伤害，精准攻击
    Wood = 2,     // 木 - 持续治疗，生命偷取  
    Water = 3,    // 水 - 范围攻击，减速效果
    Fire = 4,     // 火 - 爆发伤害，燃烧DoT
    Earth = 5     // 土 - 防御增强，击退效果
};
```

### 5.2 简化的相克关系
```cpp
class UElementalCalculator : public UBlueprintFunctionLibrary
{
public:
    // 基础相克关系 (简化版)
    static bool IsElementAdvantage(EElementType Attacker, EElementType Defender)
    {
        static TMap<EElementType, EElementType> AdvantageMap = {
            {EElementType::Metal, EElementType::Wood},   // 金克木
            {EElementType::Wood,  EElementType::Earth},  // 木克土
            {EElementType::Water, EElementType::Fire},   // 水克火
            {EElementType::Fire,  EElementType::Metal},  // 火克金
            {EElementType::Earth, EElementType::Water}   // 土克水
        };
        
        return AdvantageMap[Attacker] == Defender;
    }
    
    // 伤害修正计算
    static float GetDamageModifier(EElementType AttackElement, EElementType DefenseElement)
    {
        if (IsElementAdvantage(AttackElement, DefenseElement))
            return 1.5f;  // 相克优势 +50%
        else if (IsElementAdvantage(DefenseElement, AttackElement))  
            return 0.7f;  // 相克劣势 -30%
        else
            return 1.0f;  // 无相克关系
    }
};
```

### 5.3 AI元素选择策略
```cpp
class UElementalDecisionComponent : public UActorComponent
{
private:
    // 元素效果评估缓存
    TMap<EElementType, float> ElementEffectiveness;
    float LastEvaluationTime = 0.0f;
    
public:
    // 智能元素选择
    EElementType SelectOptimalElement(AActor* Target, const FUtilityContext& Context)
    {
        if (!Target) return EElementType::Fire;
        
        // 获取目标元素类型
        EElementType TargetElement = GetTargetElement(Target);
        
        // 评估所有元素的效果
        TMap<EElementType, float> ElementScores;
        for (int32 i = 1; i <= 5; ++i)
        {
            EElementType TestElement = static_cast<EElementType>(i);
            ElementScores.Add(TestElement, EvaluateElementScore(TestElement, TargetElement, Context));
        }
        
        // 选择最高分元素
        return GetBestElement(ElementScores);
    }
    
private:
    float EvaluateElementScore(EElementType Element, EElementType TargetElement, 
                              const FUtilityContext& Context)
    {
        float Score = 0.5f; // 基础分
        
        // 相克优势加分
        if (UElementalCalculator::IsElementAdvantage(Element, TargetElement))
            Score += 0.4f;
            
        // 环境因素 (如火元素在干燥环境中更强)
        Score += EvaluateEnvironmentalBonus(Element, Context);
        
        // 当前战术需求 (如需要治疗时偏向木元素)
        Score += EvaluateTacticalNeed(Element, Context);
        
        return FMath::Clamp(Score, 0.0f, 1.0f);
    }
};
```

## 6. 基于现有架构的实现方案

### 6.1 扩展现有CombatCharacter
```cpp
// 扩展现有战斗角色
class AAdvancedCombatCharacter : public ACombatCharacter
{
    GENERATED_BODY()
    
protected:
    // 元素属性组件 (组件化扩展)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    class UElementalAttributeComponent* ElementalComponent;
    
    // 当前元素类型
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Elemental", ReplicatedUsing=OnRep_CurrentElement)
    EElementType CurrentElement = EElementType::Fire;
    
public:
    AAdvancedCombatCharacter();
    
    // 重写现有攻击追踪，添加元素伤害
    virtual void DoAttackTrace(FName DamageSourceBone) override
    {
        // 调用基类现有逻辑
        Super::DoAttackTrace(DamageSourceBone);
        
        // 添加元素伤害计算
        ApplyElementalDamageToTargets(LastHitTargets);
    }
    
    // 元素切换功能
    UFUNCTION(BlueprintCallable, Category="Elemental")
    void SwitchElement(EElementType NewElement);
    
private:
    void ApplyElementalDamageToTargets(const TArray<AActor*>& Targets);
    
    UFUNCTION()
    void OnRep_CurrentElement();
};
```

### 6.2 扩展现有AI控制器
```cpp
// 基于现有CombatAIController扩展
class AUtilityCombatAIController : public ACombatAIController
{
    GENERATED_BODY()
    
protected:
    // Utility决策组件
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AI")
    class UUtilityDecisionComponent* DecisionComponent;
    
    // 元素决策组件
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AI") 
    class UElementalDecisionComponent* ElementalDecision;
    
public:
    AUtilityCombatAIController();
    
    // 重写现有AI逻辑起点
    virtual void BeginPlay() override
    {
        Super::BeginPlay();
        InitializeUtilityAI();
    }
    
    // 智能决策更新 (扩展现有Tick)
    virtual void Tick(float DeltaTime) override
    {
        Super::Tick(DeltaTime);
        UpdateUtilityDecisions(DeltaTime);
    }
    
private:
    void InitializeUtilityAI();
    void UpdateUtilityDecisions(float DeltaTime);
    
    // 与现有StateTree的集成接口
    UFUNCTION(BlueprintCallable, Category="AI")
    EUtilityAction GetBestUtilityAction();
    
    UFUNCTION(BlueprintCallable, Category="AI")  
    EElementType GetOptimalElementForTarget(AActor* Target);
};
```

### 6.3 StateTree任务扩展
```cpp
// 新增Utility AI StateTree任务，与现有任务并存
USTRUCT(meta=(DisplayName="Utility Decision", Category="Combat AI"))
struct FStateTreeUtilityDecisionTask : public FStateTreeTaskCommonBase
{
    GENERATED_BODY()
    
    // 复用现有数据结构
    using FInstanceDataType = FStateTreeAttackInstanceData;
    virtual const UStruct* GetInstanceDataType() const override 
    { 
        return FInstanceDataType::StaticStruct(); 
    }
    
    // 决策配置
    UPROPERTY(EditAnywhere, Category="Utility")
    float DecisionUpdateInterval = 0.5f;
    
    UPROPERTY(EditAnywhere, Category="Utility")
    float MinScoreThreshold = 0.3f;
    
    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, 
                                          const FStateTreeTransitionResult& Transition) const override
    {
        // 获取AI控制器 (利用现有Context结构)
        if (auto* AIController = Cast<AUtilityCombatAIController>(Context.GetOwner()))
        {
            EUtilityAction BestAction = AIController->GetBestUtilityAction();
            
            // 根据Utility结果设置StateTree变量，影响后续决策
            Context.SetContextDataByName(FName("SelectedAction"), 
                                       static_cast<int32>(BestAction));
            
            return EStateTreeRunStatus::Running;
        }
        
        return EStateTreeRunStatus::Failed;
    }
};
```

## 7. 测试框架设计

### 7.1 核心测试类别
```cpp
namespace ElementalCombat::Tests 
{
    // 1. Utility AI测试套件
    class FUtilityAITestSuite
    {
    public:
        static void TestScoreCalculation();      // 评分算法正确性
        static void TestActionSelection();       // 行为选择逻辑
        static void TestElementalDecision();     // 元素选择智能性
        static void TestPerformanceMetrics();   // AI决策性能测试
    };
    
    // 2. 五行元素测试套件  
    class FElementalSystemTestSuite
    {
    public:
        static void TestElementAdvantage();      // 相克关系正确性
        static void TestDamageCalculation();     // 伤害修正计算
        static void TestElementSwitching();      // 元素切换功能
        static void TestAIElementSelection();    // AI元素选择逻辑
    };
    
    // 3. 集成测试套件
    class FIntegrationTestSuite  
    {
    public:
        static void TestAIUtilityFlow();        // AI决策完整流程
        static void TestMultiAIScenario();      // 多AI交互场景
        static void TestPlayerVsAI();           // 玩家vs AI体验
        static void TestSystemStability();      // 系统稳定性测试
    };
}
```

### 7.2 具体测试用例实现
```cpp
// Utility AI评分测试
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUtilityScoreTest,
    "ElementalCombat.AI.UtilityScore.Calculation", 
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FUtilityScoreTest::RunTest(const FString& Parameters)
{
    // Arrange
    FUtilityFactors TestFactors;
    TestFactors.HealthRatio = 0.8f;     // 80% 血量
    TestFactors.DistanceToTarget = 0.3f; // 较近距离
    TestFactors.ElementAdvantage = 0.9f; // 强元素优势
    TestFactors.TacticalPosition = 0.5f; // 一般位置
    TestFactors.ThreatLevel = 0.6f;      // 中等威胁
    
    // Act
    float CalculatedScore = TestFactors.CalculateFinalScore();
    
    // Assert - 预期评分应该较高 (元素优势+健康血量)
    TestTrue(TEXT("High score with element advantage"), CalculatedScore > 0.6f);
    TestTrue(TEXT("Score within valid range"), CalculatedScore >= 0.0f && CalculatedScore <= 1.0f);
    
    return true;
}

// 元素相克测试
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FElementalAdvantageTest,
    "ElementalCombat.Elemental.Advantage.Verification",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FElementalAdvantageTest::RunTest(const FString& Parameters)
{
    // 测试所有相克关系
    TestTrue(TEXT("Metal beats Wood"), 
             UElementalCalculator::IsElementAdvantage(EElementType::Metal, EElementType::Wood));
    TestTrue(TEXT("Wood beats Earth"), 
             UElementalCalculator::IsElementAdvantage(EElementType::Wood, EElementType::Earth));
    TestTrue(TEXT("Water beats Fire"), 
             UElementalCalculator::IsElementAdvantage(EElementType::Water, EElementType::Fire));
    TestTrue(TEXT("Fire beats Metal"), 
             UElementalCalculator::IsElementAdvantage(EElementType::Fire, EElementType::Metal));
    TestTrue(TEXT("Earth beats Water"), 
             UElementalCalculator::IsElementAdvantage(EElementType::Earth, EElementType::Water));
             
    // 测试伤害修正
    float AdvantageModifier = UElementalCalculator::GetDamageModifier(EElementType::Water, EElementType::Fire);
    TestEqual(TEXT("Advantage damage modifier"), AdvantageModifier, 1.5f);
    
    float DisadvantageModifier = UElementalCalculator::GetDamageModifier(EElementType::Fire, EElementType::Water);
    TestEqual(TEXT("Disadvantage damage modifier"), DisadvantageModifier, 0.7f);
    
    return true;
}

// AI决策集成测试
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUtilityAIIntegrationTest,
    "ElementalCombat.AI.Integration.DecisionFlow",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FUtilityAIIntegrationTest::RunTest(const FString& Parameters)
{
    // Arrange - 创建测试场景
    UWorld* TestWorld = UWorld::CreateWorld(EWorldType::Game, false);
    
    // 创建AI控制器和角色
    AUtilityCombatAIController* AIController = TestWorld->SpawnActor<AUtilityCombatAIController>();
    AAdvancedCombatCharacter* AICharacter = TestWorld->SpawnActor<AAdvancedCombatCharacter>();
    AAdvancedCombatCharacter* TargetCharacter = TestWorld->SpawnActor<AAdvancedCombatCharacter>();
    
    // 设置测试条件
    AIController->Possess(AICharacter);
    TargetCharacter->SetCurrentElement(EElementType::Fire); // 目标是火元素
    
    // Act - 触发AI决策
    EElementType AISelectedElement = AIController->GetOptimalElementForTarget(TargetCharacter);
    EUtilityAction AISelectedAction = AIController->GetBestUtilityAction();
    
    // Assert - AI应该选择水元素（克制火）和攻击行为
    TestEqual(TEXT("AI should select Water to counter Fire"), AISelectedElement, EElementType::Water);
    TestTrue(TEXT("AI should choose aggressive action"), 
             AISelectedAction == EUtilityAction::AggressiveAttack);
    
    // Cleanup
    TestWorld->DestroyWorld(false);
    
    return true;
}
```

### 7.3 性能基准测试
```cpp
// AI系统性能测试  
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUtilityAIPerformanceTest,
    "ElementalCombat.Performance.AI.UtilityDecision",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::PerfFilter)

bool FUtilityAIPerformanceTest::RunTest(const FString& Parameters)
{
    // 创建多个AI进行压力测试
    const int32 AICount = 20;
    TArray<AUtilityCombatAIController*> AIControllers;
    
    for (int32 i = 0; i < AICount; ++i)
    {
        AIControllers.Add(NewObject<AUtilityCombatAIController>());
    }
    
    // 测量批量决策性能
    double StartTime = FPlatformTime::Seconds();
    
    for (int32 Frame = 0; Frame < 60; ++Frame) // 模拟60帧
    {
        for (auto* AI : AIControllers)
        {
            AI->GetBestUtilityAction(); // 触发决策计算
        }
    }
    
    double ElapsedTime = FPlatformTime::Seconds() - StartTime;
    double AverageTimePerFrame = ElapsedTime / 60.0;
    double AverageTimePerAI = AverageTimePerFrame / AICount;
    
    // 验证性能指标
    TestLessThan(TEXT("Frame time with 20 AIs"), AverageTimePerFrame, 0.016); // < 16ms per frame  
    TestLessThan(TEXT("Per AI decision time"), AverageTimePerAI, 0.002); // < 2ms per AI
    
    UE_LOG(LogTemp, Display, TEXT("Utility AI Performance: %.4f ms per frame, %.4f ms per AI"), 
           AverageTimePerFrame * 1000, AverageTimePerAI * 1000);
    
    return true;
}
```

## 8. 验收标准

### 8.1 功能完成度
- [x] **Utility AI决策系统**: AI展示明显的智能决策差异
- [x] **五行元素相克**: 所有相克关系正确工作
- [x] **AI元素选择**: AI能智能选择优势元素攻击
- [x] **StateTree集成**: Utility决策与现有StateTree无缝集成
- [x] **玩家交互**: 流畅的元素切换和战斗体验

### 8.2 性能指标  
- **帧率**: 稳定60 FPS (10个AI同时决策)
- **AI决策延迟**: < 2ms per AI per frame
- **启动时间**: 场景加载 < 15秒

### 8.3 代码质量
- **测试覆盖率**: 核心系统 > 80%
- **编译警告**: 0个Critical警告
- **架构兼容**: 完全基于现有代码扩展，无破坏性修改
- **文档完整**: 所有新增类和接口有完整注释

### 8.4 展示效果
- **AI智能性**: 观察者能明显感知AI的智能决策
- **元素战术**: 展示元素相克对战斗结果的显著影响  
- **系统稳定性**: 连续运行30分钟无崩溃或明显性能下降
- **可视化调试**: 提供清晰的AI决策过程可视化工具

---

## 项目风险管理

### 技术风险
| 风险项 | 概率 | 影响 | 缓解措施 |
|--------|------|------|----------|
| Utility AI性能问题 | 中 | 高 | 实现评分缓存机制 |
| StateTree集成复杂度 | 低 | 中 | 基于现有任务模式扩展 |
| 元素计算开销 | 低 | 中 | 预计算相克关系表 |

### 时间风险  
- **缓冲策略**: 每日20%时间缓冲
- **优先级策略**: Utility AI > 元素系统 > 视觉效果
- **最小可行产品**: 确保核心功能优先完成
