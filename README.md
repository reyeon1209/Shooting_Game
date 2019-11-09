# Shooting-Game

<p align="center">
  <a href="https://github.com/reyeon1209/Shooting-Game/">
    <img src="https://user-images.githubusercontent.com/46713032/68528057-7639c600-0331-11ea-9afd-82984737c80b.png" alt="Game logo" width="72" height="72">
  </a>
</p>

<h3 align="center">Shooting Game</h3>

<p align="center">
  Sejong University Software Design Basic in 2018
</p>

* * *

## Table of Contents

- [게임장르](#게임장르)
- [게임컨셉](#게임컨셉)
- [시나리오](#시나리오)
- [사용자설명서](#사용자설명서)
- [게임화면](#게임화면)
- [상세기획](#상세기획)
- [기여자정부](#기여자정보)

* * *

## 게임장르
```
스테이지형 슈팅 게임
```

## 게임컨셉

### 게임 설명
```
율곡관 3층에서 게임을 시작하여 차례로 층을 내려가며 율곡관을 탈출한다.
좀비 몬스터를 잡거나 피하여 시간 내에 스테이지를 클리어한다.
학생이 물풍선을 좀비에게 쏘면 좀비가 물풍선 안에 갇힌다.
학생이 물풍선을 지나가면 좀비가 죽고 점수가 올라간다. 간혹 아이템이 나온다.
1층에는 보스맵으로 거대 좀비가 출현한다.
모든 스테이지를 클리어하면 율곡관을 탈출하게 된다.
```

### 게임 특징
```
보글보글 + 이동가능 탑뷰
```

### 게임 목표
```
여러 개의 스테이지를 클리어하여 시간 내에 율곡관 탈출하기
```

### 게임 규칙
```
- 좀비와 접촉하면 체력이 감소한다.
- 체력이 0이 되면 3층에서부터 다시 시작한다.
```

## 시나리오
```
시험이 코앞에 닥친 소프트웨어학과 학생 독고소웨. 
그는 율곡관 3층 빈 강의실에서 시험 공부를 하며 밤을 새기로 한다.
오랜 시간 공부에 지쳐 깜빡 잠들어 버린 독고소웨는 새벽 세시에 눈을 뜨게 되는데… 
너무 피곤해 오로나민 씨를 마시기 위해 강의실을 나선 독고소웨는 바깥의 광경에 놀라잠이 달아나버린다. 
잠든 사이 율곡관에 있던 모든 사람들이 좀비로 변해 있었던 것이다!! 
어떻게든 좀비를 피해 율곡관을 나가야 살아남을 수 있는 상황에 빠진 독고소웨. 

좀비를 피해 율곡 밖으로 나가세요!!
```

## 사용자설명서
```
<아이콘>
플레이어(나) : ▲ (플레이어가 어디를 보느냐에 따라 ▼, ◀, ▶로도 바뀐다.)
몬스터 : ♠
아이템 : ◈

<조작법>
위로 이동 : ↑, 아래로 이동 : ↓, 오른쪽으로 이동 : →, 왼쪽으로 이동 : ←, 공격(총알 발사) : 스페이스바, 폭탄 사용 : c

<아이템>
좋은 아이템 : 공격 사거리 1칸 증가, 체력 1개 증가, 시야 1칸 증가, 폭탄 발사
나쁜 아이템 : 시야 1칸 감소, 15초 동안 방향키 반대 (아래로 : ↑, 위로 : ↓, 왼쪽으로 : →, 오른쪽으로 : ←)
(만약 체력이 5개 모두 있을 때, 체력 증가 아이템을 먹으면 체력 대신 점수가 증가한다.)
(만약 시야가 최대 일 때, 시야 증가 아이템을 먹으면 대신 점수가 증가한다.)
(만약 시야가 최소 일 때, 시야 감소 아이템을 먹으면 대신 점수가 감소한다.)
(방향키 반대 아이템은 여러 번 먹어도 시간만 다시 시작하고 방향은 1번 먹은 효과가 난다.)

몬스터에게 총알을 발사하면 몬스터가 버블에 봉인된다. 버블을 통과해서 없애면 보상으로 점수가 올라가고 가끔 아이템도 주변에 나온다.

몬스터와 가까워지면 몬스터가 나를 쫓아오고 공격한다. 몬스터의 총알에 맞거나 몬스터와 만나면 체력이 1개 줄어든다.

마지막 맵인 1층에서는 보스 몬스터가 등장한다.

체력이 0이 되면 Game Over!
```

## 게임화면

### 시작 화면
![image](https://user-images.githubusercontent.com/46713032/68528221-2cea7600-0333-11ea-89cc-c6baff7234d8.png)

### 튜토리얼 화면
![image](https://user-images.githubusercontent.com/46713032/68528223-37a50b00-0333-11ea-8cd0-8d778dc1f44b.png)
![image](https://user-images.githubusercontent.com/46713032/68528225-3e338280-0333-11ea-8a09-c91ccb54ba73.png)
![image](https://user-images.githubusercontent.com/46713032/68528227-4390cd00-0333-11ea-97a3-fdd0f9ae7bdc.png)

### 플레이 화면
![튜토리얼+시나리오+1탄+2탄+게임오버](https://user-images.githubusercontent.com/46713032/68528263-b8fc9d80-0333-11ea-9984-026bfb59171e.gif)
![튜토리얼+3탄(보스)+게임클리어](https://user-images.githubusercontent.com/46713032/68528266-c1ed6f00-0333-11ea-8e2f-513fce556978.gif)

## 상세기획

### 화면 제공 요소
```
- 게임이 처음 시작될 때, 아이템과 조작 키 설명 창을 띄운다.
- 게임 진행 중에는 현재 점수와 체력, 남은 시간, 사용 중인 아이템, 층 수, 캐릭터, 지형, 상대 몬스터(좀비), 드랍된 아이템, 발사 중인 총알이 콘솔에 표시된다.
```

### 키 기능
```
방향키 (↑↓→←), 공격(SPACE BAR)
```

### 스테이지 구성
```
- 3층에서 시작하며 2, 3층은 비슷하게 구성되고 난이도가 달라진다.
- 1층은 보스맵이며 거대 좀비가 출현한다.
```

### 점수 부여 방법
```
- 몬스터를 잡으면 점수 증가한다. 
- 스테이지 완료 시, 남아 있는 시간과 체력에 따라 추가 점수를 준다.
- 체력 게이지가 가득 차 있을 때, 체력 증가 아이템을 획득하면 추가 점수를 준다.
- 시야가 화면 전체 일 때, 시야 증가 아이템을 획득하면 추가 점수를 준다.
- 시야가 최소 시야 일 때, 시야 감소 아이템을 획득하면 점수 깎는다.
```

### 아이템
```
- 플레이어에게 좋은 아이템 – 공격 사거리 증가, 체력 증가, 시야 증가, 폭탄
- 플레이어에게 좋지 않은 아이템  –  시야 축소, 방향키 반대
- 대략 스테이지마다 5-n개 드랍된다.
- 지속 시간이 지나면 사라지며, 중복 적용이 가능하다
```

### NPC
```
- 가로 세로 합 n 이상이면 플레이어를 쫓아온다.
- 근거리 공격을 하는 몬스터,  원거리 공격을 하는 몬스터가 있다.
- 플레이어가 몬스터에게 공격 당하면 상태 이상에 걸린다.
```

### 보스몬스터
```
보스몬스터는 다음과 같은 4가지의 공격 패턴을 갖는다.
1.  플레이어 방향으로 총알을 3개씩 발사한다.
2.  물결처럼 공격을 연사 한다.
3. 플레이어 방향으로 총알을 대량 발사한다.
4. 몸통 박치기
```

### 지형
```
- 스테이지별로 미리 지정해 놓은 미로를 띄운다.
- 스테이지가 올라갈 수록 복잡해진다.
```

## 기여자정보

**양재연**

- [**@reyeon1209**](https://github.com/reyeon1209)   
- <reyeon5368@naver.com>
