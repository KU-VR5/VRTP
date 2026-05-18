# Sound of Darkness VR

시야가 차단된 어두운 미로에서 플레이어가 내는 소리만으로 주변 구조를 파악하고 탈출하는 VR 공포 게임 프로젝트입니다. 발걸음, 목소리, 투척물 충돌음이 각각 소리 파동을 만들고, 이 파동은 미로의 벽, 바닥, 천장 윤곽을 잠깐 드러내며 동시에 귀신 AI의 추적 목표가 됩니다.

## 프로젝트 정보

- Engine: Unreal Engine 5.7
- Language: C++
- Platform: VR / OpenXR
- Project: `SoundOfDarkness/SoundOfDarkness.uproject`

## 핵심 원리

이 프로젝트의 중심은 `USoundWaveManager` 월드 서브시스템입니다. 게임 안에서 소리가 발생하면 `SpawnSoundWave()`가 호출되고, 매 프레임마다 활성 파동의 반경이 커집니다.

파동 데이터는 다음 값을 가집니다.

- Origin: 소리가 발생한 월드 위치
- CurrentRadius: 현재 퍼져나간 반경
- MaxRadius: 최대 반경
- ExpansionSpeed: 반경 증가 속도
- Intensity: 시간이 지날수록 감소하는 밝기/강도
- Priority: 귀신 AI가 어떤 소리를 더 중요하게 볼지 결정하는 우선순위

`Tick()`에서는 `CurrentRadius += ExpansionSpeed * DeltaTime`으로 파동을 확장하고, `CurrentRadius / MaxRadius` 비율에 따라 `Intensity`를 1에서 0으로 서서히 줄입니다. 파동이 최대 반경에 도달하면 비활성화됩니다.

## 에코로케이션 시각화

구현 파일:

- `SoundOfDarkness/Source/SoundOfDarkness/Public/SoundWaveManager.h`
- `SoundOfDarkness/Source/SoundOfDarkness/Private/SoundWaveManager.cpp`

### 셰이더 파라미터 전달

`UpdateMaterialParameters()`는 최대 8개의 활성 파동 정보를 Material Parameter Collection에 전달합니다.

- `WaveLocation_0..7`: 파동 발생 위치
- `WaveData_0..7`: 현재 반경, 최대 반경, 강도

머티리얼 또는 포스트 프로세스 머티리얼에서는 이 값을 읽어 월드 위치 기준으로 파동 효과를 만들 수 있습니다.

### 벽 윤곽 표시

`DrawEchoOutlines()`는 각 파동의 중심에서 여러 방향으로 수평 라인트레이스를 쏩니다. 트레이스가 벽에 맞으면 충돌 지점에 짧은 수직선, 접선 방향 선, 노멀 방향 선을 그려 벽의 윤곽이 드러난 것처럼 보이게 합니다.

주요 설정값:

- `OutlineRayCount`: 원형으로 쏘는 레이 개수
- `OutlineVerticalSamples`: 높이별 샘플 개수
- `OutlineSampleHeight`: 기준 샘플 높이
- `OutlineVerticalSpan`: 위아래 샘플 범위
- `OutlineStrokeLength`: 표시 선 길이
- `OutlineThickness`: 표시 선 두께
- `OutlineLifetime`: 표시 유지 시간

### 바닥과 천장 표시

기존에는 수평 레이만 사용해서 벽만 감지됐습니다. 현재는 같은 파동 반경 안에서 여러 샘플 지점을 잡고, 각 지점마다 아래/위 방향 트레이스를 추가로 쏩니다.

- 바닥: 샘플 지점 위에서 아래로 트레이스
- 천장: 샘플 지점 위쪽에서 더 위로 트레이스
- 감지 성공 시 표면 노멀 기준으로 짧은 십자 형태의 마크를 그림

관련 설정값:

- `bDrawHorizontalSurfaceOutlines`: 바닥/천장 표시 켜기
- `SurfaceRadialSamples`: 파동 반경 안쪽을 몇 단계로 나눠 검사할지
- `FloorTraceStartOffset`: 바닥 트레이스 시작 높이
- `FloorTraceDepth`: 바닥 트레이스 깊이
- `CeilingTraceStartOffset`: 천장 트레이스 시작 높이
- `CeilingTraceHeight`: 천장 트레이스 높이

천장 또는 바닥이 보이지 않으면 해당 메쉬가 `Visibility` 채널을 Block하는지 확인해야 합니다. 현재 윤곽 트레이스는 `ECC_Visibility`를 기준으로 충돌을 검사합니다.

## 소리 발생 시스템

### 발걸음

구현 파일:

- `MazeNoiseComponent.h`
- `MazeNoiseComponent.cpp`

`UMazeNoiseComponent`는 플레이어 또는 액터의 2D 이동 속도를 감시합니다. 속도가 `MinMoveSpeed` 이상이고 `FootstepInterval` 시간이 지나면 발걸음 파동을 생성합니다.

기본값:

- `MinMoveSpeed`: 20
- `FootstepInterval`: 0.85초
- `FootstepWaveRadius`: 900
- `FootstepWaveSpeed`: 700
- `FootstepWaveIntensity`: 0.7
- `FootstepPriority`: 25

원리:

1. 매 Tick마다 소유 액터의 `GetVelocity().Size2D()`를 확인합니다.
2. 일정 속도 이상이면 발걸음 타이머를 누적합니다.
3. 인터벌이 지나면 소유 액터 위치에서 `SpawnSoundWave()`를 호출합니다.

### 목소리

구현 파일:

- `VoiceDetectorComponent.h`
- `VoiceDetectorComponent.cpp`

`UVoiceDetectorComponent`는 `UAudioCaptureComponent`를 동적으로 생성해 마이크 입력의 Envelope 값을 감지합니다. 입력 값이 `LoudnessThreshold` 이상이고 쿨다운이 끝났으면 음성 파동을 생성합니다.

기본값:

- `LoudnessThreshold`: 0.3
- `EcholocationRadiusModifier`: 2500
- `WaveCooldown`: 1.0초
- `VoiceWaveSpeed`: 1800
- Priority: 100

원리:

1. 마이크 Envelope 값이 들어옵니다.
2. 임계값보다 작거나 쿨다운 중이면 무시합니다.
3. Envelope 값에 비례해 파동 반경을 계산합니다.
4. 플레이어 위치에서 높은 우선순위의 파동을 생성합니다.

### 투척물 충돌음

구현 파일:

- `ThrowableStone.h`
- `ThrowableStone.cpp`

`AThrowableStone`은 물리 시뮬레이션을 사용하는 액터입니다. 충돌 이벤트가 발생했을 때 속도가 충분히 빠르고 쿨다운이 끝났다면 충돌 지점에서 파동을 생성합니다.

기본값:

- `ImpactWaveIntensity`: 1.0
- `ImpactWaveRadius`: 2000
- `ImpactWaveSpeed`: 1500
- `MinimumImpactVelocity`: 50
- `ImpactCooldown`: 0.8초
- Priority: 50

원리:

1. `StoneMesh`에 물리 시뮬레이션과 히트 이벤트를 켭니다.
2. `OnComponentHit`에서 현재 속도를 검사합니다.
3. 충분히 빠른 충돌이면 `Hit.ImpactPoint`에서 파동을 생성합니다.

## 귀신 AI

구현 파일:

- `GhostAIController.h`
- `GhostAIController.cpp`
- `GhostCharacter.h`
- `GhostCharacter.cpp`

귀신 AI는 `USoundWaveManager`에 등록된 활성 파동 중 우선순위가 가장 높은 소리를 추적합니다.

우선순위 예시:

- 목소리: 100
- 투척물: 50
- 발걸음: 25

`AGhostAIController`는 `PollingInterval`마다 `GetHighestPrioritySound()`를 호출합니다. 활성 소리가 있으면 블랙보드의 `TargetLocation`을 갱신하고 `MoveToLocation()`으로 해당 지점까지 이동합니다.

귀신 캐릭터는 손전등에 맞으면 `StunGhost()`로 스턴됩니다. 스턴 중에는 이동 속도가 0이 되고, AI 컨트롤러도 소리 추적을 멈춥니다. 스턴 시간이 끝나면 원래 이동 속도로 복구됩니다.

## 손전등 방어 기믹

구현 파일:

- `Flashlight.h`
- `Flashlight.cpp`

`AFlashlight`는 제한 횟수만 사용할 수 있는 스턴 도구입니다. 켜졌을 때 스포트라이트 방향으로 Sphere Trace를 쏘고, 범위 안에 `AGhostCharacter` 또는 `Ghost` 태그가 있는 액터가 있으면 귀신을 스턴합니다.

기본값:

- `StunDistance`: 2000
- `StunAngleRadius`: 20
- `MaxUses`: 3
- `StunDuration`: 3초
- `UseDuration`: 0.6초
- `TraceRadius`: 80

원리:

1. `ToggleLight()`로 손전등을 켭니다.
2. 남은 사용 횟수를 1 줄입니다.
3. 짧은 시간 동안만 빛을 유지합니다.
4. 빛 방향 Sphere Trace에 귀신이 들어오면 `StunGhost()`를 호출합니다.

## 탈출 퍼즐

구현 파일:

- `MazeInventoryComponent.h`
- `FusePickup.h`
- `FusePanel.h`
- `ExitDoor.h`

플레이어는 퓨즈를 모아 패널에 삽입하고, 필요한 개수만큼 삽입하면 출구 문이 열립니다.

흐름:

1. `FusePickup`을 통해 인벤토리에 퓨즈를 추가합니다.
2. `FusePanel`의 트리거에 플레이어가 들어오면 `TryInsertFuse()`를 시도합니다.
3. 인벤토리에서 퓨즈를 소비하고 삽입 개수를 증가시킵니다.
4. `RequiredFuseCount`에 도달하면 `SolvePanel()`이 호출됩니다.
5. 연결된 `ExitDoor`가 `OpenDoor()` 상태가 되고, Tick에서 목표 위치까지 보간 이동합니다.

## 빌드 확인

최근 확인한 빌드 명령:

```bash
"/Users/Shared/Epic Games/UE_5.7/Engine/Build/BatchFiles/Mac/Build.sh" SoundOfDarknessEditor Mac Development -Project="/Users/jangsu/Desktop/4-2(학교)/가상현실/test/VRTP/SoundOfDarkness/SoundOfDarkness.uproject" -WaitMutex
```

결과:

- `SoundOfDarknessEditor Mac Development` 빌드 성공

## 구현 시 주의점

- 에코로케이션 윤곽 트레이스는 `ECC_Visibility`를 사용합니다. 보이지 않는 표면이 있으면 해당 메쉬의 Collision Response에서 Visibility가 Block인지 확인해야 합니다.
- 현재 디버그 윤곽은 `DrawDebugLine` 기반이므로 실제 게임용 최종 비주얼은 머티리얼/포스트 프로세스 효과와 함께 다듬는 것이 좋습니다.
- 파동은 최대 8개까지 동시에 관리됩니다. 풀이 가득 차면 0번 파동을 덮어씁니다.
- 귀신 AI는 현재 가장 높은 우선순위의 활성 소리를 단순 추적합니다. 같은 우선순위일 때 거리, 최근성 등을 추가하면 더 자연스럽게 만들 수 있습니다.
