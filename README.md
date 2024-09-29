# CN Assignment - 2

Roll Number: 2022052 & 2022054

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

Reference: https://github.com/andrewrk/poop/issues/17



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

Reference: https://man7.org/linux/man-pages/man1/perf-stat.1.html
