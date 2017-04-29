iosh: iosh.c lib/mpc/mpc.c
	clang -g -std=c99 -lreadline -Wall $^ -o iosh
