#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <uuid/uuid.h>
#include <mpi.h>

#include <daos.h>
#include <daos_fs.h>
#include <daos_errno.h>

#define min(x,y) ((x)<(y) ? (x) : (y))

void dts_buf_render(char *buf, unsigned int buf_len) {
	int	nr = 'z' - 'a' + 1;
	int	i;

	for (i = 0; i < buf_len - 1; i++) {
		int randv = rand() % (2 * nr);

		if (randv < nr)
			buf[i] = 'a' + randv;
		else
			buf[i] = 'A' + (randv - nr);
	}
	buf[i] = '\0';
}

void create_files(dfs_t *dfs, int number_of_files) {
  char buf[64];
  mode_t create_mode = S_IWUSR | S_IRUSR;
  int create_flags = O_RDWR | O_CREAT | O_EXCL;
  dfs_obj_t *obj;
  char *data;
  daos_size_t	data_size = 128 * 1024;
  daos_size_t	io_size;
  d_sg_list_t	sgl;
  d_iov_t		  iov;
  daos_size_t file_size;
  daos_size_t	size = 0;
  int i, rc;

  for (i = 0; i < number_of_files; i++) {
    snprintf(buf, 64, "file_%d.txt", i);
    file_size = 1024 * 1024 + 1024 * i;
    printf("  Creating file name: %s of size %d bytes\n", buf, file_size);

    rc = dfs_open(dfs,
                  NULL, /* use root obj */
                  buf,
                  create_mode | S_IFREG,
                  create_flags,
                  0,
                  0, /* 0 for default 1 MiB chunk size */
                  NULL,
                  &obj);
    if (rc != 0) {
      printf("failed to create file %s "DF_RC"\n", buf, DP_RC(rc));
      goto close_file;
    }

    data = (char*)malloc(sizeof(data) * data_size);
    if (data == NULL) {
      printf("malloc failed\n");
      goto close_file;
    }

    d_iov_set(&iov, data, data_size);

    sgl.sg_nr = 1;
	  sgl.sg_nr_out = 1;
	  sgl.sg_iovs = &iov;

    while (size < file_size) {
		  io_size = file_size - size;
		  io_size = min(io_size, data_size);

		  sgl.sg_iovs[0].iov_len = io_size;
		  dts_buf_render(data, io_size);
		  rc = dfs_write(dfs, obj, &sgl, size, NULL);
		  if (rc != 0) {
        printf("failed to write to file %s "DF_RC"\n", buf, DP_RC(rc));
        break;
      }
		  size += io_size;
	  }

    free(data);

close_file:
    rc = dfs_release(obj);
    if (rc != 0) {
      printf("failed to close file %s "DF_RC"\n", buf, DP_RC(rc));
    }
  }
}

int main(int argc, char** argv) {
  int my_rank, world_size, rc;
  uuid_t pool_uuid;  /* only used on rank 0 */
  daos_handle_t poh; /* shared pool handle */
  daos_handle_t coh; /* shared container handle */

  static uuid_t co_uuid;
  char uuid_str[37];

  dfs_t *dfs;

  /* Initialize the MPI environment */
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Barrier(MPI_COMM_WORLD);

  if (world_size > 1) {
    printf("This is a toy example, single rank supported :)\n");
    goto err_03;
  }

  if (argc < 2) {
    printf("Error: missing arguments\n");
    printf("%s <POOL_UUID>\n", argv[0]);
    goto err_03;
  }

  printf("Hello Daos\n");
  rc = uuid_parse(argv[1], pool_uuid);
  if (rc != 0) {
    printf("failed to parse pool uuid\n");
    goto err_03;
  }
  printf("Pool UUID: %s\n", argv[1]);

  /* initialize the local DAOS stack */
  rc = daos_init();
  if (rc != 0) {
    printf("daos_fini failed with  "DF_RC"\n", DP_RC(rc));
    goto err_03;
  }

  printf("Connecting to Pool: %s\n", argv[1]);
  rc = daos_pool_connect(pool_uuid, NULL,
			       DAOS_PC_EX /* exclusive access */,
			       &poh /* returned pool handle */,
			       NULL /* returned pool info */,
			       NULL /* event */);
  if (rc != 0) {
    printf("pool connect failed with "DF_RC"\n", DP_RC(rc));
    goto err_01;
  }

  /** generate uuid for container */
  uuid_generate(co_uuid);
  uuid_unparse_lower(co_uuid, uuid_str);
  printf("Creating Container UUID: %s\n", uuid_str);

  rc = dfs_cont_create(poh, co_uuid, NULL, &coh, NULL);
  if (rc != 0) {
    printf("failed to create container "DF_RC"\n", DP_RC(rc));
    goto err_01;
  }

  printf("Mounting DFS Container UUID: %s\n", uuid_str);
  rc = dfs_mount(poh, coh, O_RDWR, &dfs);
  if (rc != 0) {
    printf("failed to mount DFS namespace "DF_RC"\n", DP_RC(rc));
    goto err_00;
  }

  /* Do work */

  create_files(dfs, 10);

out:
  printf("Unmounting DFS Container UUID: %s\n", uuid_str);
  rc = dfs_umount(dfs);
  if (rc != 0) {
    printf("failed to umount DFS namespace "DF_RC"\n", DP_RC(rc));
    goto out;
  }

err_00:
  /** close container */
  printf("Closing container: %s\n", uuid_str);
  daos_cont_close(coh, NULL);

err_01:
  /** disconnect from pool & destroy it */
  printf("Disconnecting from Pool: %s\n", argv[1]);
  daos_pool_disconnect(poh, NULL);

err_02:
  /* shutdown the local DAOS stack */
  rc = daos_fini();
  if (rc != 0) {
    printf("daos_fini failed with  "DF_RC"\n", DP_RC(rc));
  }

err_03:
  printf("End\n");

  /*Finalize the MPI environment. */
  MPI_Finalize();
}
