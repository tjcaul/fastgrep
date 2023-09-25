#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#define BUF_SZ 4096

void scan (int block_num, char *buf, int buf_sz, char *search);
double blocks2mib (int blocks);

int main (int argc, char **argv) {
	int fd;
	char *buf;
	char *search;
	int read_sz = 0;
	int extra_sz;
	int block_num = 0;
	double stat_secs;

	if (!argv[1] || !argv[2] || !argv[3])
		return 0;

	search = argv[2];
	sscanf(argv[3], "%lf", &stat_secs);
	extra_sz = strlen(search) - 1;
	fd = open(argv[1], O_RDONLY);
	if (fd < 0) {
		perror("open");
		return -1;
	}
	buf = malloc(BUF_SZ + extra_sz);
	if (!buf) {
		perror("malloc");
		return -1;
	}
	memset(buf, 'A', extra_sz);

	do {
		read_sz = read(fd, buf + extra_sz, BUF_SZ);
		if (read_sz < 0) {
			perror("read");
			return -1;
		}
		scan(block_num++, buf, read_sz, search);
		memcpy(buf, buf + BUF_SZ, extra_sz); //Won't work properly if reading last block, but the loop exits right after anyways

		if (stat_secs != 0) {
			static time_t last_time = 0;
			static int last_block_num;
			time_t now_time = time(NULL);
			if (now_time - last_time >= stat_secs || last_time == 0) {
				last_time = now_time;
				int blocks_read = block_num - last_block_num;
				fprintf(stderr, "%.0lfMiB (%.2lfMiB/s)\n", blocks2mib(block_num), blocks2mib(blocks_read) / stat_secs);
				last_block_num = block_num;
			}
		}
	} while (read_sz);

	close(fd);

	return 0;
}

void scan (int block_num, char *buf, int buf_sz, char *search) {
	int extra_sz = strlen(search) - 1;
	for (int i = 0; i < buf_sz; ++i) {
		if (buf[i] == search[0]) {
			if (!memcmp(buf + i, search, strlen(search))) {
				printf("%lld\n", (long long)block_num * BUF_SZ + i - extra_sz - 1);
				fflush(stdout);
			}
		}
	}
}

double blocks2mib (int blocks) {
	return (double)blocks * BUF_SZ / (1024*1024);
}
