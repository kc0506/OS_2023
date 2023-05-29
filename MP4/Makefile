B=b10401006
X=mp4/xv6
K=kernel
U=user
M=mkfs

submit:
	rm $B -rf
	mkdir -p $B/xv6
	cp $X/$K $B/xv6/$K -r
	cp $X/$U $B/xv6/$U -r
	cp $X/$M $B/xv6/$M -r
	
	zip $B.zip $B -r


check: submit
	cp $X/Makefile $X/README $B/xv6
	docker run -it --rm -v $(PWD)/$B/xv6/:/home/xv6  ntuos/mp4
