시스템 프로그래밍- 라즈베리 파이와 센서를 활용한 스마트냉장고 시스템
-
**2. 분석**
**2.1 냉장고 기능 분석**

(1) 냉장고 온도 lcd 화면 표시기능
센서파이의 온습도 센서가 5초마다 읽는 값을 서버파이에 보내고 서버파이는 액츄에이터
파이에서 보낸 정보들 중 온도 값과 관련된 정보를 분류하여 해당 값을 소수점1자리
문자열로 변환하여 lcd 화면에 출력한다.

 (2) 냉장고 문 열림 감지 기능
센서파이의 냉장고 문의 닫히는 부분에 부착된 압력 센서가 1초마다 냉장고문의 압력을
측정하여 값을 서버파이에 보낸다. 

서버파이는 이 압력이 기준치(본프로젝트에서600)이하인 경우 타이머를 작동시켜 이 시간이 30초이상 되는 경우 냉장고
문이 열림 것으로 간주하여 빨간색 led와 경고음이 출력을 하는 신호를 액츄에이터 파이에
보내 작동시킨다.

 (3) 냉장고 온도이상 탐지 기능
일반적인 냉장실의 온도는 2~3도이다.따라서 온도 값이 5도 이상인 경우와 8도이상인 경우

경중을 나눠서 경고를 보낸다. 센서파이에서 보내는 온도 값이 5~7.9도 사이인 경우 파란색

led를 점등시키는 신호를 액츄에이터 파이에 보낸다 만약 온도가 8도이상이 되면 냉장고에

치명적인 문제가 발생하였다고 판단하여 빨간색 led와 경고음을 출력한다.

 (4) 냉장고 품목 유효기간 관리기능

우유, 달걀, 채소등의 유효기간이 주요한 품목에 대해 사용자가 유효기간을 기입하면

유효기간이 1일 남은경이 초록색 led를 점등하고 lcd 화면에 목록을 출력한다.

또한 능동적으로 유효기간 기능을 활용하기 위해버튼1과 버튼2를 활용하여 버튼 1을

누르면 유효기간이 1일남은 품목을 출력하고 버튼 2를 누르면 유효기간이 이미 지난 품목을

출력하며 두 경우 모두 해당하는 품목이 있는 경우 초록색 led를 점등한다.


**응용 시스템의 구조**
(1)물리적구조 
![image](https://github.com/user-attachments/assets/18c747de-636b-405e-b87c-88684045be1d)

(2)논리적 구조
![image](https://github.com/user-attachments/assets/62538910-96fb-4cb5-b630-d6bb72cf4347)

(3)알고리즘 개요
![image](https://github.com/user-attachments/assets/dc11309d-5bb7-4426-baaa-5dd14dd901d8)

**실핼방법**
센서파이에서는 “gcc -o Client1 SensorClient.c temsor.c preSensor.c -lpwiringPi -lpthread”를 터미널에 입력하여
Client 프로그램을 만들고 ./Client1 포트번호 이런 방식으로 두번째 결
