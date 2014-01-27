#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <utmp.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

struct mosh {
	time_t last_used;
	int has_ip;
	int pid;
	struct mosh *next;
};

struct user {
	char username[UT_NAMESIZE];
	struct mosh *head;
	struct user *next;
};

static inline struct user *new_user(char *username)
{
	struct user *user;
	user = malloc(sizeof(struct user));
	if (!user) {
		perror("user");
		exit(EXIT_FAILURE);
	}
	strcpy(user->username, username);
	user->head = NULL;
	user->next = NULL;
	return user;
}

#define ONE_HOUR (60 * 60)
#define ONE_DAY (ONE_HOUR * 24)

int main(int argc, char *argv[])
{
	FILE *utmp;
	struct utmp entry;
	struct user *user_head = NULL, *user;
	struct mosh *parsed, *insert, **prev;
	char *bracket_open, *bracket_close;
	int kill_time;

	utmp = fopen("/var/run/utmp", "r");
	if (!utmp) {
		perror("fopen");
		return EXIT_FAILURE;
	}
	while (fread(&entry, sizeof(entry), 1, utmp)) {
		if (!strcmp(entry.ut_host, "mosh"))
			continue;
		bracket_open = strrchr(entry.ut_host, '[');
		bracket_close = strrchr(entry.ut_host, ']');
		if (!bracket_open || !bracket_close || bracket_open >= bracket_close)
			continue;

		parsed = malloc(sizeof(struct mosh));
		if (!parsed) {
			perror("malloc");
			return EXIT_FAILURE;
		}
		parsed->next = NULL;
		parsed->has_ip = strstr(entry.ut_host, "via mosh") != NULL;
		*bracket_close = '\0';
		parsed->pid = atoi(bracket_open + 1);
		parsed->last_used = entry.ut_tv.tv_sec;


		if (!user_head)
			user_head = user = new_user(entry.ut_user);
		else {
			for (user = user_head; user; user = user->next) {
				if (!strcmp(entry.ut_user, user->username))
					break;
				if (!user->next) {
					user->next = new_user(entry.ut_user);
					user = user->next;
					break;
				}
			}
		}
		if (!user->head)
			user->head = parsed;
		else {
			for (prev = &user->head, insert = user->head; insert; prev = &insert->next, insert = insert->next) {
				if ((parsed->has_ip && !insert->has_ip) || parsed->last_used > insert->last_used)
					break;
			}
			*prev = parsed;
			parsed->next = insert;
		}
	}
	fclose(utmp);

	for (user = user_head; user; user = user->next) {
		if (!user->head->next) /* If the user only has one mosh process running, we don't kill anything. */
			continue;
		for (parsed = user->head->next, kill_time = ONE_DAY; parsed; parsed = parsed->next, kill_time /= 2) {
			if (kill_time < ONE_HOUR)
				kill_time = ONE_HOUR;
			if (parsed->has_ip) /* If the user is currently connected, we don't kill it. */
				continue;
			if (time(NULL) - parsed->last_used < kill_time) /* If the connection is less than an hour old, we don't kill it. */
				continue;
			printf("[+] Killing %s's mosh instance %d, last used on %s", user->username, parsed->pid, ctime(&parsed->last_used));
			if (kill(parsed->pid, SIGTERM))
				perror("kill");
		}
	}

	return EXIT_SUCCESS;
}
