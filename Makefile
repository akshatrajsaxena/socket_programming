single-threaded: single-threaded-client.c single-threaded-server.c
	gcc single-threaded-client.c -w -o single-threaded-client
	gcc single-threaded-server.c -w -o single-threaded-server

concurrent-thread: concurrent-client.c concurrent-server-thread.c
	gcc concurrent-client.c -w -o concurrent-client -lpthread
	gcc concurrent-server-thread.c -w -o concurrent-server-thread -lpthread
	
select: select-server.c
	gcc single-threaded-client.c -w -o single-threaded-client
	gcc select-server.c -w -o select-server

clean:
	rm -f single-threaded-client
	rm -f single-threaded-server
	rm -f concurrent-client
	rm -f concurrent-server-thread
	rm -f select-server
	rm -f output.txt
