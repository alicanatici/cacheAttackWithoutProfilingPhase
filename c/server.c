#include <openssl/aes.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>


unsigned int timestamp(void)
{
	unsigned int bottom;
	unsigned int top;
	asm volatile(".byte 15;.byte 49" : "=a"(bottom),"=d"(top));
	return bottom;
}

unsigned char key[16];
AES_KEY expanded;
unsigned char zero[16];
unsigned char scrambledzero[16];

void handle(char out[40],char in[],int len)
{
	unsigned char workarea[len];
	int i;

	for (i = 0;i < 40;++i) out[i] = 0;

	if (len < 16) return;

	for (i = 0;i < 16;++i) out[i] = in[i];

	
	//for (i = 16;i < len;++i) workarea[i] = in[i]; // from original code

	*(unsigned int *) (out + 32) = timestamp();
	AES_encrypt(in,workarea,&expanded);
	*(unsigned int *) (out + 36) = timestamp();
	
	// i modified this line so it sends the ciphertext
	for (i = 0;i < 16;++i) out[16 + i] = workarea[i];
}

struct sockaddr_in server;
struct sockaddr_in client; socklen_t clientlen;

int s;
char in[1537];
int r;
char out[40];

main(int argc,char **argv)
{
	
	int i;
	int j;
	
	if (read(0,key,sizeof key) < sizeof key){	
		return 111;
	}

	AES_set_encrypt_key(key,128,&expanded);
	
	printf("Round keys are: \n");
	for (i=0; i<15; i++){
		for (j=0; j<4; j++) {
			printf("%08lx ", expanded.rd_key[j+ i*4]);
		}
		printf("\n");
	}
	
	AES_encrypt(zero,scrambledzero,&expanded);


	if (!argv[1]) return 100;
	if (!inet_aton(argv[1],&server.sin_addr)) return 100;
	server.sin_family = AF_INET;
	server.sin_port = htons(10000);
	printf("server port %d \n",server.sin_port);

	s = socket(AF_INET,SOCK_DGRAM,0);
	printf("socket returned is %d \n", s);
	if (s == -1) return 111;
	if (bind(s,(struct sockaddr *) &server,sizeof server) == -1){
		//while (bind(s,(struct sockaddr *) &server,sizeof server) == -1){
		printf("bind error \n");
		return 111;
	}

	printf("entered loop \n");
	
	for (;;) {
		clientlen = sizeof client;
		r = recvfrom(s,in,sizeof in,0
		,(struct sockaddr *) &client,&clientlen);
		if (r < 16) continue;
		if (r >= sizeof in) continue;
		handle(out,in,r);
		sendto(s,out,40,0,(struct sockaddr *) &client,clientlen);
	}
}

