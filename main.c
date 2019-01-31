#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TXTSIZE 1024
#define RPLSIZE 8192
#define NAMELEN 128

int main()
 {
 // sock будет слушает, sock_new соединяется с клиентом,
 // fd используется для временных файлов
 int sock,sock_new,fd;
 struct sockaddr_in saddr,caddr;
 char msg_rcv[TXTSIZE],msg_snd[RPLSIZE], fname[NAMELEN];
 FILE *fout;
 memset(msg_rcv,0,TXTSIZE);
 memset(msg_snd,0,RPLSIZE);
 memset(fname,0,NAMELEN);

 // просто создаётся сокет с локальным ip адресом и портом 2001
 sock=socket(AF_INET,SOCK_STREAM,0);
 if(sock==-1)
  {
  perror("socket()");
  return -1;
  }

 memset(&saddr,0,sizeof(saddr));
 saddr.sin_family=AF_INET;
 saddr.sin_port=htons(2001);
 saddr.sin_addr.s_addr=inet_addr("127.0.0.1");

 if(bind(sock,(struct sockaddr*)&saddr,sizeof(saddr))==-1)
  {
  perror("bind()");
  return -1;
  }

 // устанавливаем сокет в состояние прослушивания запросов на соединение,
 // максимальное число одновременных соединений 1
 listen(sock,1);

 for(unsigned i=1;;++i)
  {
  socklen_t caddr_len=sizeof(caddr);

 //соединяемся с клиентом и получаем от него сообщение/запрос
  sock_new=accept(sock,(struct sockaddr*)&caddr,&caddr_len);
  recv(sock_new,msg_rcv,TXTSIZE,0);

 //просто выводим содержимое сообщение в терминал
  printf("%i Query:\n%s\n\n",i,msg_rcv);

 //1 Проверка: обрабатываем только HTTP GET сообщения
  if(!(msg_rcv[0]=='G'&&msg_rcv[1]=='E'&&msg_rcv[2]=='T'))
   {
   send(sock_new,"HTTP/1.1 404 Error\n\n"
          "<html>404 Error</html>",42,0);
   close(sock_new);
   continue;
   }

 // просто вытаскиваем из запроса имя запрашиваемого ресурса данного сервера
  fd=creat("tmp",0664);
  write(fd,msg_rcv,strlen(msg_rcv));
 // в запросе имена начинаются /, добавлем перед / точку в целях безопасности
  fout=popen("head -n 1 tmp | awk '{print \".\"$2}'","r");
  fgets(fname,NAMELEN,fout);
  fname[strlen(fname)-1]=0;
  close(fd);
  fd=open("tmp",O_WRONLY|O_TRUNC);
  write(fd,fname,strlen(fname));
 // имя запрашиваемого ресурса содержится в fname и врем. файле tmp
 // 2 Провекра: запрашиваемый ресурс может быть только HTML документом
  fout=popen("file -b `cat tmp` | awk '{if ($1==\"HTML\") print 0}'","r");
  close(fd);
  if(fgetc(fout)!='0')
   {
   send(sock_new,"HTTP/1.1 404 Error\n\n"
          "<html>404 Error</html>",42,0);
   close(sock_new);
   continue;
   }

 // все проверки пройдены, формируем и отправляем ответ клиенту
  strcpy(msg_snd,"HTTP/1.1 200 OK\n\n");
  fd=open(fname,O_RDONLY);
  read(fd,&msg_snd[17],RPLSIZE-17);
  send(sock_new,msg_snd,RPLSIZE,0);
  memset(msg_snd,0,RPLSIZE);
  printf("SENDED!\n");
  }
 unlink("tmp");
 return 0;
 }
