all:
	g++ server_thread_pool.cc -pthread -w -o server
	gcc client_thread_pool.c -o client
	gcc server_epoll.c -o server_epoll
	gcc client_epoll.c -o client_epoll

clean:
	rm -rf server client server_epoll client_epoll
