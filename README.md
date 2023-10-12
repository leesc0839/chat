# reap
multi-process echo server
<img width="1512" alt="스크린샷 2023-10-13 오전 12 19 59" src="https://github.com/leesc0839/network/assets/94699064/655ec59c-ce04-4804-a741-db5bc2060090">

- tcp socket을 이용
- fork로 프로세스 생성 **(여러 client와 동시에 통신하기 위해서)**
- client 쪽에서 fin을 보내는 구조
- backlog queue size == 5
- 서버는 while-loop으로 accept을 계속해서 수행
- signal handling으로 zombie process reap

# socket
client : serv_sock (1개)
server : serv_sock(listening, 1개), clnt_sock(read write, 1개)

- 여러 client와 통신을 위해 fork를 하면, server의 clnt_sock 하나씩 생성.
- 부모와자식 프로세스에서 모두 close 해주어야 fin이 정상적으로 수행

# 사용법
1. server 먼저 켜놓기
   ex) ./server 9190
2. client로 접속
   ex) ./client 127.0.0.1 9190
3. client가 메세지를 보내면 서버는 그대로 반사해서 보내줌
4. client에서 q or Q을 누르면 close -> fin
5. server는 여러 client를 동시에 수용 가능 (n:1)

# process
- client's process : 1개
- server's parent process : while-loop 수행하며 accept + clnt_sock close
- server's child process : echo 수행 + serv_sock close

# flow
1. client가 connect(syn)을 하면, server는 accept 및 fork로 child process 생성
   + server's child process : file descriptor 복사로 인해 listening socket 미리 종료 // **server's serv_sock close -1**
   + server's parent process : **server's clnt_sock close -1** // 복제 되기 때문에 종료해주기
2. server's child process는 복제된 serv_sock file descriptor를 close
3. server's child process는 echo 역할 수행
4. client's process : close : q or Q 를 쳐서 종료요청 수행 // **client's serv_sock close**
5. server's child process : server는 read에 0이 들어오는 것을 보고, fin을 detect 후, **server's clnt_sock close -2**, 그리고 자식 프로세스 종료
6. server's child process 종료 시키면 singal handler called
7. singal handler : zombie process 생성을 막기 위해 waitpid로 reap
8. server는 while-loop 을 통해 계속해서 client의 요청을 accept (block 함수)





# 흥미로웠던 점
<img width="1512" alt="스크린샷 2023-10-12 오후 11 39 00" src="https://github.com/leesc0839/network/assets/94699064/88c97dec-b3ad-4d56-bf43-345a8e5a884d">
ps al 커맨드로 확인해보면 multi-process server에서 reaping 해주지 않는다면 zombie process가 남게 되는 것을 볼 수 있다.

자식 프로세스가 종료되면, 부모 프로세스는 wait or waitpid 같은 함수로 자식을 reaping을 해주어야 한다. 아니면 좀비 프로세스가 계속 쌓이게 된다.
물론 서버가 down되면 zombie 프로세스는 메모리에서 해제되지만, 실제 서버는 수 많은 client를 상대로 계속해서 돌아가야만 한다.
따라서 zombie process가 메모리 낭비를 하지 않도록 reaping 해주는 것은 꼭 필요하다.


# tcp ports
<img width="1512" alt="스크린샷 2023-10-13 오전 12 20 08" src="https://github.com/leesc0839/network/assets/94699064/71e9f604-bcd7-4969-960c-4a466c2e52ea">

실제 3개의 client를 동시에 접속시킨 후 할당된 tcp port들.

server -> 1 : listening socket
          n : clnt_sock(이 소켓으로 client와 통신한다.)
client -> n : 단일 프로세스로 서버에게 접속한다. n명이면 n개.

요즘은 multi-thread 환경의 서버가 대부분이지만,
multi-process 환경에서 서버가 많은 client를 수용하는 과정을 생각해 보게 되었다.
서버의 특성 상 24시간 내내 돌아가야 하며, zombie process 같은 메모리 누수 관리를 잘 해주어야 한다고 생각했다.
