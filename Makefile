iosh: iosh.c lib/mpc/mpc.c
	clang -g -std=c11 -lreadline -Wall $^ -o iosh
