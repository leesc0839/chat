# chat
chat server, terminal 1:1

<img width="1204" alt="image" src="https://github.com/leesc0839/chat/assets/94699064/edbc4484-1c74-4130-8888-3a0c4ef0a4df">


- tcp socket을 이용
- fork로 프로세스 생성 **(입출력 분할 목적)**
- client 쪽에서 fin을 보내는 구조
- backlog queue size == 5
- 서버는 while-loop으로 accept을 계속해서 수행
- signal handling으로 종료 요청을 처리

# socket
client : serv_sock (connect read write, 2개)
server : serv_sock(listening, 2개), clnt_sock(read write, 2개)

- 입출력 분할을 위해 fork를 해서 복사되어서 2개씩 있음.
- 각 프로세스에서 모두 close 해주어야 fin이 정상적으로 수행

# 사용법
1. server 먼저 켜놓기
   ex) ./server 9190
2. client로 접속
   ex) ./client 127.0.0.1 9190
3. read, write으로 채팅 -> 양방향으로 메세지 보내기 가능
4. client에서 q or Q을 누르면 close -> fin
5. client는 재접속 가능 (단, 1:1)

# process
- client's parent process : read_routine // 사용자에게 q or Q로 close 최초 detect
- client's child process : write_routine
- server's parent process : write_routine
- server's child process : read_routine

# flow
1. client가 connect(syn)을 하면, 서버는 accept! // 입출력 분할을 위해 client, server 모두 fork 수행
   + server's child process : file descriptor 복사로 인해 listening socket 미리 종료 // **server's serv_sock close -1**
2. read, write으로 양방향 채팅 수행
3. client's child process : shutdown : q or Q 를 쳐서 종료요청 수행 // **client's serv_sock close -1**
     ** shutdown을 통해 socket의 file descriptor가 fork으로 인해 복사되어 있어도, 곧바로 fin을 보낸다 ** => 이거 때문에 어려웠음.
4. server's child process : server는 read에 0이 들어오는 것을 보고, fin을 detect 후, **server's clnt_sock close -1**, 그리고 자식 프로세스 종료
5. server's child process 종료 시키면 singal handler called
6. singal handler : zombie process 생성을 막기 위해 waitpid로 reap 하고, 전역 flag를 1로 바꾼다.
7. server's parent process : 전역 flag가 1이 되면, 종료 신호 detect하여 write_routine 종료
8. server's parent process : write_routine 종료하고 **clnt_sock close -2** -> fin 전송
9. client's parent process : read에 0이 들어오면, read_routine 종료 => 서버는 last-ack 상태
10. client's parent process : **client's serv_sock close -2**
11. server's parent process : while-loop 으로 다시 accept 수행 // server's serv_sock은 listening 상태로 계속 한 개 남아있음.


# 흥미로웠던 점
<img width="792" alt="image" src="https://github.com/leesc0839/chat/assets/94699064/68db7390-2e17-4fed-8639-10fe9ebb7c09">

lsof -i 명령어로 상태확인

<img width="844" alt="image" src="https://github.com/leesc0839/chat/assets/94699064/e673713a-91e4-434a-94be-f209d04dccaf">
어디서 오류가 발생했는지 발견하는 과정

socket이 현재 어떤 상태인지 lsof -i 명령어로 확인하며 코드를 짜 보았던 점.
코드가 잘 수행되지 않았을 때, fin-wait2, close_wait, established 와 같은 다양한 소켓의 상태를 terminal을 이용해 확인해보며,
고쳐가는 과정은 흥미로웠음.

멀티 프로세스 기반 코드를 직접 짜보는 경험 또한 흥미로웠음.
