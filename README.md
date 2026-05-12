# Sound of Darkness (소리의 어둠) VR

시각이 차단된 칠흑 같은 어둠 속에서, 플레이어가 스스로 내는 소리(발걸음, 던진 물건의 소음, 마이크를 통한 실제 육성)의 파동을 통해서만 주변 지형을 파악하고 탈출하는 완전 몰입형 VR 공포 게임입니다.

---

## 🛠 기술 스택
- **Engine**: Unreal Engine 5.7 (C++)
- **Platform**: VR (OpenXR 기반)
- **Version Control**: Git / GitHub Organization (KU-VR organization)

---

## ⚡ 주요 구현 기능 및 파일 현황

### 1. 전역 반향 정위 시스템 (Echolocation System)
소리 파동의 발생과 확산을 관리하며, 셰이더(Shader)에 실시간 데이터를 전달합니다.
- **`SoundWaveManager.h / .cpp`**: 
  - `UTickableWorldSubsystem` 상속.
  - 최대 8개의 동시 소리 파동 위경도, 반경, 강도를 관리.
  - 마테리얼 파라미터 컬렉션(MPC)을 통해 셰이더에 데이터를 초당 60회 이상 업데이트.
  - **주요 코드**: `SpawnSoundWave()`, `UpdateMaterialParameters()`

### 2. 음성 감지 및 분석 (Voice Detection)
플레이어의 실제 마이크 입력을 게임 내 소리 파동으로 변환합니다.
- **`VoiceDetectorComponent.h / .cpp`**: 
  - `UAudioCaptureComponent` 활용.
  - 실시간 데시벨(Loudness)을 감지하여 일정 수치 이상일 경우 `SoundWaveManager`에 파동 생성 요청.

### 3. 상호작용 가능한 투척물 (Throwable Interaction)
물리적인 충돌 소음을 통해 지형을 파악하는 전략적 도구입니다.
- **`ThrowableStone.h / .cpp`**: 
  - 물리 시뮬레이션 적용.
  - `OnComponentHit` 발생 시 충돌 속도에 비례한 강도의 소리 파동을 충돌 지점에 생성.

### 4. 추적자 AI (Ghost AI)
소리에 반응하여 플레이어를 추적하는 공포의 대상입니다.
- **`GhostCharacter.h / .cpp`**: 
  - 귀신의 외형 및 상태(스턴 등) 관리.
  - `StunGhost()` 함수를 통해 손전등 등에 의한 일시 정지 기능 구현.
- **`GhostAIController.h / .cpp`**: 
  - `SoundWaveManager`로부터 현재 맵에서 발생하는 소리 중 **가장 우선순위가 높은 소리**의 위치를 실시간으로 가져옵니다.
  - 블랙보드(Blackboard)의 `TargetLocation` 키에 좌표를 기록하여 비헤이비어 트리(BT)가 귀신을 이동시키도록 제어합니다.

### 💡 소리 우선순위 시스템 (Priority System)
모든 소리는 중요도에 따라 우선순위가 부여되어 귀신의 행동을 결정합니다.
- **육성 (Voice)**: 우선순위 100 (가장 높음, 귀신이 즉시 반응)
- **돌 투척 (Stone)**: 우선순위 50 (유인용으로 활용 가능)
- **발걸음 (Footstep)**: 우선순위 10 (기본 소음)

### 5. 손전등 및 방어 기제 (Flashlight)
제한적인 시야 확보 및 귀신을 일시적으로 저지하는 도구입니다.
- **`Flashlight.h / .cpp`**: 
  - 스포트라이트 컴포넌트 제어.
  - 라인트레이스(Line Trace)를 통해 빛의 범위 안에 "Ghost" 태그를 가진 액터가 있을 경우 스턴(Stun) 로직 호출.

---

## 📁 프로젝트 구조 변경 사항 (Source/SoundOfDarkness)

- **`SoundOfDarkness.Build.cs`**: `AIModule`, `AudioCapture`, `AudioMixer`, `MetasoundEngine` 등 필수 모듈 의존성 추가 완료.
- **`SoundOfDarkness.Target.cs / Editor.Target.cs`**: UE 5.7 컴파일 규격인 `BuildSettingsVersion.V6`로 업데이트 완료.

---

## 🚀 향후 개발 계획 (Next Steps)
- **Phase 2**: Echolocation Post-Process Material (셰이더 노드 구성)
- **Phase 4**: VR 컨트롤러 바인딩 및 VRPawn 연동
- **Phase 5**: 귀신 AI 비헤이비어 트리(BT) 고도화 (순찰, 추격, 실종 상태 전환)
"# maze1" 
"# maze1" 
