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
  - 이론에서 배웠던 3-way handshake와 달리, server가 마지막에 ack을 하나 더 보내는데, 서버가 연결과정을 마치고, 클라이언트에게 데이터를 전송할 준비가 되었음을 나타내는 패킷을 하나 더 보냄.
    
- 통신 과정
  - cumulative ack 방식이지만, 이론과 약간 차이 있음.
  - client는 sequence number를 0부터 보냄, server는 1부터 시작.
  - client는 ack으로 마지막 sequence 번호를 보냄.
    - ex) server가 seq 0:17을 보내면, ack으로 17을 보냄. 
  - server는 ack으로 마지막 sequence 번호 + 1를 보냄. // 이론과 일치
    - ex) client가 seq 0:17을 보내면, ack으로 18을 보냄.

- 종료 과정
  - half-close 방식을 적용.
  - client가 fin을 보내고, 실제로 서버는 데이터를 더 보낼 수 있었음.
  - fin, ack, fin, ack이 실제로 오고 가는 것을 확인.
  - 이론과 일치하게 , client server 모두 ack 번호가 sequence 번호 + 1로 보내짐.

# 알게된 점
tcpdump으로 관찰한 결과, 이론과 흐름은 모두 일치, 미세하게 차이가 존재했다.
매우 간단한 예시이지만, tcp 패킷을 분석해서 데이터 통신 과정을 눈으로 보는 것은 즐거운 경험이었다.
tcp는 구현이 이론과 약간은 다를 수 있다는 것을 깨달았다. 흐름은 동일하였다.
