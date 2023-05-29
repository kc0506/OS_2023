SHELL = bash

run: 
	# docker pull ntuos/mp3
	docker run -it -v $(shell pwd)/xv6-riscv:/home/xv6-riscv/ -w /home/xv6-riscv/ \
	--name osmp3 --rm ntuos/mp3 /bin/bash 

gdb:
	docker exec osmp3 /bin/bash -c \
	"echo 'add-auto-load-safe-path /home/xv6-riscv/.gdbinit' > ~/.gdbinit"
	docker exec -it osmp3 /bin/bash -c "gdb-multiarch"


D=b10401006
X=xv6-riscv
K=kernel
U=user


submit:
	rm $D -rf
	mkdir $D
	mkdir $D/$K
	mkdir $D/$U
	eval "cp $X/$K/{proc.h,proc.c,thrd.c,trap.c} $D/$K"
	cp $X/$U/threads_sched.c $D/$U
	zip -r b10401006.zip b10401006
	rm $D -rf
