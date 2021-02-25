# DAOS Hello World

Toy example to write some files using `libdaos`.

## Building the example

```bash
$ make
```

## Usage

- Create a DAOS Pool

Make sure that DAOS is installed and running, please refer to the
[server startup][1] documentation.

```bash
$ dmg pool create --size 8G
Creating DAOS pool with automatic storage allocation: 8.0 GB NVMe + 6.00% SCM
Pool created with 100.00% SCM/NVMe ratio
-----------------------------------------
  UUID          : 85e68879-0e95-4a29-8810-c7be59778bc1
  Service Ranks : 0
  Storage Ranks : 0
  Total Size    : 8.0 GB
  SCM           : 8.0 GB (8.0 GB / rank)
  NVMe          : 0 B (0 B / rank)
```

- Run the example

```bash
$ export D_LOG_FILE=/tmp/daos_client.log
$ export D_LOG_MASK=ERR

$ ./hello_daos 85e68879-0e95-4a29-8810-c7be59778bc1
Hello Daos
Pool UUID: 85e68879-0e95-4a29-8810-c7be59778bc1
Connecting to Pool: 85e68879-0e95-4a29-8810-c7be59778bc1
Creating Container UUID: fdd86dd0-5487-42fa-b79c-d44edc923dcc
Mounting DFS Container UUID: fdd86dd0-5487-42fa-b79c-d44edc923dcc
  Creating file name: file_0.txt of size 1048576 bytes
  Creating file name: file_1.txt of size 1049600 bytes
  Creating file name: file_2.txt of size 1050624 bytes
  Creating file name: file_3.txt of size 1051648 bytes
  Creating file name: file_4.txt of size 1052672 bytes
  Creating file name: file_5.txt of size 1053696 bytes
  Creating file name: file_6.txt of size 1054720 bytes
  Creating file name: file_7.txt of size 1055744 bytes
  Creating file name: file_8.txt of size 1056768 bytes
  Creating file name: file_9.txt of size 1057792 bytes
Unmounting DFS Container UUID: fdd86dd0-5487-42fa-b79c-d44edc923dcc
Closing container: fdd86dd0-5487-42fa-b79c-d44edc923dcc
Disconnecting from Pool: 85e68879-0e95-4a29-8810-c7be59778bc1
End
```

- Verified container was created
```bash
$ daos pool list-containers --pool 85e68879-0e95-4a29-8810-c7be59778bc1
fdd86dd0-5487-42fa-b79c-d44edc923dcc
```

- Verified the created objects

```bash
$ daos container list-objects \
  --pool 85e68879-0e95-4a29-8810-c7be59778bc1 \
  --cont fdd86dd0-5487-42fa-b79c-d44edc923dcc
1155473247756615689.0
1155473247756615688.0
1155473247756615687.0
1155473247756615686.0
1155473247756615685.0
1155473247756615684.0
1155473247756615683.0
1155473247756615682.0
1152922380780175361.0
1152922363600306176.0
1155473247756615691.0
1155473247756615690.0
```

```bash
$ mkdir -p /tmp/dfs_test
$ dfuse --mountpoint /tmp/dfs_test \
  --pool 85e68879-0e95-4a29-8810-c7be59778bc1 \
  --cont fdd86dd0-5487-42fa-b79c-d44edc923dcc
$ ls -la /tmp/dfs_test
total 10285
-rw------- 1 eduardoj eduardoj 1048576 Feb 25 22:59 file_0.txt
-rw------- 1 eduardoj eduardoj 1049600 Feb 25 22:59 file_1.txt
-rw------- 1 eduardoj eduardoj 1050624 Feb 25 22:59 file_2.txt
-rw------- 1 eduardoj eduardoj 1051648 Feb 25 22:59 file_3.txt
-rw------- 1 eduardoj eduardoj 1052672 Feb 25 22:59 file_4.txt
-rw------- 1 eduardoj eduardoj 1053696 Feb 25 22:59 file_5.txt
-rw------- 1 eduardoj eduardoj 1054720 Feb 25 22:59 file_6.txt
-rw------- 1 eduardoj eduardoj 1055744 Feb 25 22:59 file_7.txt
-rw------- 1 eduardoj eduardoj 1056768 Feb 25 22:59 file_8.txt
-rw------- 1 eduardoj eduardoj 1057792 Feb 25 22:59 file_9.txt
$ fusermount -u /tmp/dfs_test
```

[1]: <https://github.com/daos-stack/daos/blob/master/doc/admin/deployment.md> (DAOS server startup documentation)
