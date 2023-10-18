# network
client, server 간 1:1 chat 서버를 만들고,
tcp packet 분석해보기    

# 명령어
- server : $sudo tcpdump -i lo0 src port 9190 => interface가 localhost, src port가 9190 분석    
- client : $sudo tcpdump -i lo0 dst port 9190 => interface가 localhost, dst port가 9190 분석


# client 기준
<img width="1193" alt="image" src="https://github.com/leesc0839/network/assets/94699064/14ae5ce9-4a0f-48b2-8441-85da41926c32">


# server 기준
<img width="1202" alt="image" src="https://github.com/leesc0839/network/assets/94699064/46391941-3410-4875-b2d4-82cc5c4835af">

# 분석해보기
<img width="363" alt="image" src="https://github.com/leesc0839/network/assets/94699064/9c7a02ed-4eaf-4de4-b926-dfff4f9ee981">

# 생각 해볼점
- 연결 과정
  - sequence number는 이론에서 배웠던 방식과 일치하게 random number로 연결
    
- 통신 과정  
  - sequence number가 0부터 시작하였음. 심지어 server는 1부터 시작.
  - cumulative ack 방식이 맞지만, ack번호가 sequence 번호의 바로 다음번호, 혹은 마지막 번호로 보내짐.

- 종료 과정
  - half-close 방식을 적용해봄
  - client가 fin을 보내고, 실제로 서버는 데이터를 더 보낼 수 있었음.
  - fin, ack, fin, ack이 실제로 오고 가는 것을 확인   

이론과 대부분 비슷하지만, 실제 패킷 번호들은 조금씩 차이점이 나는 것을 발견할 수 있었음.
그러나 tcp 패킷을 분석해서 데이터 통신 과정을 눈으로 보는 것은 즐거운 경험이었다.
왜 이론과 차이가 발생하는지 알아볼 예정.
