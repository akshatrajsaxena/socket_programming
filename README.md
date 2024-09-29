<h1 align="center">Socket Programming in C</h1>
<p align="center"><i>Performance Analysis and Optimization of a Single-Threaded TCP Server Using Linux Perf Monitoring Tools</i></p>
<p align="center"><i>Roll Number: 2022052 & 2022054</i></p>
<div align="center">
  <a href="https://github.com/akshatrajsaxena/socket_programming/stargazers"><img src="https://img.shields.io/github/stars/akshatrajsaxena/socket_programming" alt="Stars Badge"/></a>
  <a href="https://github.com/akshatrajsaxena/socket_programming/network/members"><img src="https://img.shields.io/github/forks/akshatrajsaxena/socket_programming" alt="Forks Badge"/></a>
  <a href="https://github.com/akshatrajsaxena/socket_programming/pulls"><img src="https://img.shields.io/github/issues-pr/akshatrajsaxena/socket_programming" alt="Pull Requests Badge"/></a>
  <a href="https://github.com/akshatrajsaxena/socket_programming/issues"><img src="https://img.shields.io/github/issues/akshatrajsaxena/socket_programming" alt="Issues Badge"/></a>
  <a href="https://github.com/akshatrajsaxena/socket_programming/graphs/contributors"><img alt="GitHub contributors" src="https://img.shields.io/github/contributors/CGAS_Assignment_1/socket_programming" ?color=2b9348"></a>
  <a href="https://github.com/akshatrajsaxena/socket_programming/blob/master/LICENSE"><img src="https://img.shields.io/github/license/akshatrajsaxena/socket_programming" ?color=2b9348" alt="License Badge"/></a>
</div>
<br>



## Bind Already in Use Error 

-Find the PID which is runinng on the port p using

```shell

$ sudo lsof -i :8080

```
This command will result in the PID (PID) time and name of the server that is runnning. 

Now kill the process using the command:

``` shell

$ sudo kill -9 <PID>

```

## perf tool permission denied error 

perf_event_paranoid is working fine with value 3 which can be changed by:

``` shell

$ sysctl kernel.perf_event_paranoid=3

```



## Running the Programs

### For single threaded server

```shell

$ make single-threaded

$ sudo perf stat -e cycles,instructions,cache-references,cache-misses,context-switches,cpu-migrations,page-faults ./single-threaded-server


```
-  Run the following command on the second terminal.

```shell

$ sh n_seq_clients.sh 10 ./single-threaded-client

```
- Observe the results in `output.txt`

- `NOTE:` Ensure that commands are run in the aforementioned order for successful compilation!


### For concurrent server using pthreads


```shell
$ make concurrent-thread

$ sudo perf stat -e cycles,instructions,cache-references,cache-misses,context-switches,cpu-migrations,page-faults ./concurrent-server-thread 10


```

-  Run the following command on the second terminal.

```shell

$ ./concurrent-client 10

```

- Observe the results in `output.txt`

### For TCP-Client server using select() system call

```shell

$ sudo perf stat -e cycles,instructions,cache-references,cache-misses,context-switches,cpu-migrations,page-faults ./select-server

```

-  Run the following command on the second terminal.

```shell

$ sh n_con_clients.sh 10 ./single-threaded-client

```

## Contact

If you have any questions or feedback, you can reach out to [Akshat Raj Saxena](mailto:akshat22054@iiitd.ac.in)

## Acknowledgements

Special thanks to the creators of JavaFX and the contributor to this project [Akshat Karnwal](https://github.com/Akshat22052).

## Reference

https://man7.org/linux/man-pages/man1/perf-stat.1.html

 https://github.com/andrewrk/poop/issues/17
