iosh: iosh.c lib/mpc/mpc.c
	clang -std=c99 -lreadline -Wall $^ -o iosh
